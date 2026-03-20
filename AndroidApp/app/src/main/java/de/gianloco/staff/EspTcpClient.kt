package de.gianloco.staff

import android.content.Context
import android.graphics.Bitmap
import android.graphics.BitmapFactory
import android.net.nsd.NsdManager
import android.net.nsd.NsdServiceInfo
import android.util.Log
import kotlinx.coroutines.*
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import java.io.ByteArrayOutputStream
import java.io.InputStream
import java.io.OutputStream
import java.net.InetAddress
import java.net.InetSocketAddress
import java.net.Socket
import java.net.SocketTimeoutException

enum class ConnectionStatus {
    DISCONNECTED,
    CONNECTING,
    CONNECTED
}

class EspTcpClient(private val context: Context) {

    private val TAG = "EspTcpClient"
    private val serviceType = "_arcane._tcp."
    private val targetName = "arcanebyte"
    private val targetHostname = "arcanebyte.local"
    
    private var espIp: String? = null
    
    private val _status = MutableStateFlow(ConnectionStatus.DISCONNECTED)
    val status: StateFlow<ConnectionStatus> = _status

    private val nsdManager = context.getSystemService(Context.NSD_SERVICE) as NsdManager
    private var socket: Socket? = null
    private var outputStream: OutputStream? = null
    private var inputStream: InputStream? = null
    
    private val scope = CoroutineScope(Dispatchers.IO + SupervisorJob())

    fun discoverAndConnect() {
        _status.value = ConnectionStatus.CONNECTING
        Log.d(TAG, "Starting discovery for $serviceType")
        
        val discoveryListener = object : NsdManager.DiscoveryListener {
            override fun onDiscoveryStarted(regType: String) {
                Log.d(TAG, "Service discovery started")
            }
            override fun onServiceFound(service: NsdServiceInfo) {
                Log.d(TAG, "Service found: ${service.serviceName} Type: ${service.serviceType}")
                if (service.serviceName.contains(targetName, ignoreCase = true)) {
                    Log.d(TAG, "Matching service found, resolving...")
                    nsdManager.resolveService(service, resolveListener)
                }
            }
            override fun onServiceLost(service: NsdServiceInfo) {
                Log.d(TAG, "Service lost: ${service.serviceName}")
                if (service.serviceName.contains(targetName, ignoreCase = true)) {
                    espIp = null
                    disconnect()
                }
            }
            override fun onDiscoveryStopped(serviceType: String) {}
            override fun onStartDiscoveryFailed(serviceType: String, errorCode: Int) {
                Log.e(TAG, "Discovery failed: $errorCode")
            }
            override fun onStopDiscoveryFailed(serviceType: String, errorCode: Int) {}
        }

        try {
            nsdManager.discoverServices(serviceType, NsdManager.PROTOCOL_DNS_SD, discoveryListener)
        } catch (e: Exception) {
            Log.e(TAG, "Error starting discovery", e)
        }
        
        // Background loop for reconnection
        scope.launch {
            while (isActive) {
                val currentSocket = socket
                val needsConnect = currentSocket == null || currentSocket.isClosed || !currentSocket.isConnected
                
                if (needsConnect) {
                    if (espIp == null) {
                        tryDirectResolution()
                    }
                    if (espIp != null) {
                        tryConnect()
                    }
                }
                delay(5000)
            }
        }
    }

    private fun tryDirectResolution() {
        try {
            Log.d(TAG, "Attempting direct resolution for $targetHostname")
            val address = InetAddress.getByName(targetHostname)
            if (address.hostAddress != null) {
                Log.d(TAG, "Direct resolution successful: ${address.hostAddress}")
                espIp = address.hostAddress
            }
        } catch (e: Exception) {
            // Normal fallback
        }
    }

    private val resolveListener = object : NsdManager.ResolveListener {
        override fun onResolveFailed(serviceInfo: NsdServiceInfo, errorCode: Int) {
            Log.e(TAG, "Resolve failed: $errorCode")
        }
        override fun onServiceResolved(serviceInfo: NsdServiceInfo) {
            Log.d(TAG, "Service resolved via NSD: ${serviceInfo.host.hostAddress}")
            espIp = serviceInfo.host.hostAddress
        }
    }

    private suspend fun tryConnect() {
        val ip = espIp ?: return
        withContext(Dispatchers.IO) {
            try {
                Log.d(TAG, "Connecting to $ip:888...")
                _status.value = ConnectionStatus.CONNECTING
                
                val newSocket = Socket()
                newSocket.keepAlive = true
                newSocket.soTimeout = 10000
                
                newSocket.connect(InetSocketAddress(ip, 888), 5000)
                
                socket = newSocket
                outputStream = newSocket.getOutputStream()
                inputStream = newSocket.getInputStream()
                
                startReading(newSocket)
                
                _status.value = ConnectionStatus.CONNECTED
                Log.d(TAG, "Connected successfully!")
            } catch (e: Exception) {
                Log.e(TAG, "Connection failed: ${e.message}")
                _status.value = ConnectionStatus.DISCONNECTED
                socket = null
            }
        }
    }

    private fun startReading(socket: Socket) {
        scope.launch(Dispatchers.IO) {
            val input = socket.getInputStream()
            val buffer = ByteArray(1024)
            try {
                while (isActive) {
                    val bytesRead = try {
                        input.read(buffer)
                    } catch (e: SocketTimeoutException) {
                        Log.d(TAG, "Read timeout: No heartbeat from ESP")
                        -1
                    }

                    if (bytesRead == -1) {
                        Log.d(TAG, "Socket read EOF or Timeout (Connection dead)")
                        disconnect()
                        break
                    }
                    
                    val receivedData = String(buffer, 0, bytesRead)
                    Log.d(TAG, "Received from ESP: $receivedData")
                }
            } catch (e: Exception) {
                Log.d(TAG, "Socket read error: ${e.message}")
                if (isActive) disconnect()
            }
        }
    }

    private fun disconnect() {
        Log.d(TAG, "Disconnecting and updating UI status...")
        _status.value = ConnectionStatus.DISCONNECTED
        
        val socketToClose = socket
        socket = null
        outputStream = null
        inputStream = null

        scope.launch(Dispatchers.IO) {
            try {
                socketToClose?.close()
                Log.d(TAG, "Socket closed successfully")
            } catch (e: Exception) {
                Log.e(TAG, "Error closing socket", e)
            }
        }
    }

    suspend fun sendSpell(bitmap: Bitmap?, spell: String) = withContext(Dispatchers.IO) {
        val currentOutput = outputStream
        if (bitmap == null || currentOutput == null) {
            Log.e(TAG, "Cannot send: Socket not connected")
            return@withContext
        }

        try {
            val bos = ByteArrayOutputStream()
            bitmap.compress(Bitmap.CompressFormat.PNG, 100, bos)
            val bytes = bos.toByteArray()
            val spellWithoutSpace = spell.replace(' ', '_')
            val header = "PLAYPNG $spellWithoutSpace.png ${bytes.size}\n"
            currentOutput.write(header.toByteArray())
            currentOutput.write(bytes)
            currentOutput.flush()
            Log.d(TAG, "Spell sent! Size: ${bitmap.width}x${bitmap.height}")
        } catch (e: Exception) {
            Log.e(TAG, "Send failed, triggering disconnect", e)
            disconnect()
        }
    }

    fun sendWlanPNG() {
        scope.launch {
            val options = BitmapFactory.Options().apply {
                inScaled = false // Deaktiviert die automatische Android-Skalierung
            }
            val bitmap = BitmapFactory.decodeResource(context.resources, R.drawable.wlan, options)
            sendSpell(bitmap, "wlan")
        }
    }
}
