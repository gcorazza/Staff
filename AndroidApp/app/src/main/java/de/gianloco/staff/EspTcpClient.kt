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
import kotlinx.coroutines.flow.update
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

enum class SaveMode {
    NOSAVE,
    TEMP,
    PERSISTENT
}

class EspTcpClient(private val context: Context) {

    private val TAG = "EspTcpClient"
    private val serviceType = "_arcane._tcp."
    private val targetName = "arcanebyte"
    private val targetHostname = "arcanebyte.local"
    
    private var espIp: String? = null
    
    private val _status = MutableStateFlow(ConnectionStatus.DISCONNECTED)
    val status: StateFlow<ConnectionStatus> = _status

    private val _lastConnectedIp = MutableStateFlow<String?>(null)
    val lastConnectedIp: StateFlow<String?> = _lastConnectedIp

    private val _isDiscoveryRunning = MutableStateFlow(false)
    val isDiscoveryRunning: StateFlow<Boolean> = _isDiscoveryRunning

    private val _logs = MutableStateFlow<List<String>>(emptyList())
    val logs: StateFlow<List<String>> = _logs

    private val nsdManager = context.getSystemService(Context.NSD_SERVICE) as NsdManager
    private var socket: Socket? = null
    private var outputStream: OutputStream? = null
    private var inputStream: InputStream? = null
    
    private val scope = CoroutineScope(Dispatchers.IO + SupervisorJob())

    private fun addLog(message: String) {
        Log.d(TAG, message)
        _logs.update { (listOf(message) + it).take(50) }
    }

    private val discoveryListener = object : NsdManager.DiscoveryListener {
        override fun onDiscoveryStarted(regType: String) {
            addLog("Service discovery started")
            _isDiscoveryRunning.value = true
        }
        override fun onServiceFound(service: NsdServiceInfo) {
            addLog("Service found: ${service.serviceName}")
            if (service.serviceName.contains(targetName, ignoreCase = true)) {
                addLog("Matching service found, resolving...")
                nsdManager.resolveService(service, resolveListener)
            }
        }
        override fun onServiceLost(service: NsdServiceInfo) {
            addLog("Service lost: ${service.serviceName}")
            if (service.serviceName.contains(targetName, ignoreCase = true)) {
                espIp = null
                disconnect()
            }
        }
        override fun onDiscoveryStopped(serviceType: String) {
            addLog("Discovery stopped")
            _isDiscoveryRunning.value = false
        }
        override fun onStartDiscoveryFailed(serviceType: String, errorCode: Int) {
            addLog("Discovery failed to start: $errorCode")
            _isDiscoveryRunning.value = false
        }
        override fun onStopDiscoveryFailed(serviceType: String, errorCode: Int) {
            addLog("Discovery failed to stop: $errorCode")
            _isDiscoveryRunning.value = false
        }
    }

    private val resolveListener = object : NsdManager.ResolveListener {
        override fun onResolveFailed(serviceInfo: NsdServiceInfo, errorCode: Int) {
            addLog("Resolve failed: $errorCode")
        }
        override fun onServiceResolved(serviceInfo: NsdServiceInfo) {
            val ip = serviceInfo.host.hostAddress
            addLog("Service resolved via NSD: $ip")
            espIp = ip
        }
    }

    fun discoverAndConnect() {
        _status.value = ConnectionStatus.CONNECTING
        
        // Start initial discovery
        startDiscovery()
        
        // Background loop for reconnection and persistence
        scope.launch {
            while (isActive) {
                val currentSocket = socket
                val needsConnect = currentSocket == null || currentSocket.isClosed || !currentSocket.isConnected
                
                if (needsConnect) {
                    if (espIp == null) {
                        // Ensure NSD is running if we don't have an IP
                        startDiscovery()
                        // Parallel fallback: Try direct mDNS resolution
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

    private fun startDiscovery() {
        if (_isDiscoveryRunning.value) return
        try {
            addLog("Attempting to start NSD discovery...")
            nsdManager.discoverServices(serviceType, NsdManager.PROTOCOL_DNS_SD, discoveryListener)
        } catch (e: Exception) {
            addLog("Error starting discovery: ${e.message}")
            _isDiscoveryRunning.value = false
        }
    }

    private fun tryDirectResolution() {
        try {
            addLog("Attempting direct resolution for $targetHostname")
            val address = InetAddress.getByName(targetHostname)
            if (address.hostAddress != null) {
                val ip = address.hostAddress
                addLog("Direct resolution successful: $ip")
                espIp = ip
            }
        } catch (e: Exception) {
            // Normal fallback if mDNS is not responding
        }
    }

    private suspend fun tryConnect() {
        val ip = espIp ?: return
        withContext(Dispatchers.IO) {
            try {
                addLog("Connecting to $ip:888...")
                _status.value = ConnectionStatus.CONNECTING
                
                val newSocket = Socket()
                newSocket.keepAlive = true
                newSocket.soTimeout = 10000
                
                // Connection timeout of 5 seconds
                newSocket.connect(InetSocketAddress(ip, 888), 5000)
                
                socket = newSocket
                outputStream = newSocket.getOutputStream()
                inputStream = newSocket.getInputStream()
                
                startReading(newSocket)
                
                _lastConnectedIp.value = ip
                _status.value = ConnectionStatus.CONNECTED
                addLog("Connected successfully to $ip")
            } catch (e: Exception) {
                addLog("Connection failed to $ip: ${e.message}")
                _status.value = ConnectionStatus.DISCONNECTED
                socket = null
                // Clear IP on failure to force a new search (IP might have changed)
                espIp = null 
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
                        addLog("Read timeout: No heartbeat from ESP")
                        -1
                    }

                    if (bytesRead == -1) {
                        addLog("Socket read EOF or Timeout (Connection dead)")
                        disconnect()
                        break
                    }
                    
                    val receivedData = String(buffer, 0, bytesRead)
                    addLog("Received from ESP: $receivedData")
                }
            } catch (e: Exception) {
                addLog("Socket read error: ${e.message}")
                if (isActive) disconnect()
            }
        }
    }

    private fun disconnect() {
        addLog("Disconnecting...")
        _status.value = ConnectionStatus.DISCONNECTED
        
        val socketToClose = socket
        socket = null
        outputStream = null
        inputStream = null

        scope.launch(Dispatchers.IO) {
            try {
                socketToClose?.close()
                addLog("Socket closed successfully")
            } catch (e: Exception) {
                addLog("Error closing socket: ${e.message}")
            }
        }
    }

    suspend fun sendCommand(command: String) = withContext(Dispatchers.IO) {
        val currentOutput = outputStream
        if (currentOutput == null) {
            addLog("Cannot send command: Socket not connected")
            return@withContext
        }

        try {
            val fullCommand = if (command.endsWith("\n")) command else "$command\n"
            currentOutput.write(fullCommand.toByteArray())
            currentOutput.flush()
            addLog("Command sent: $command")
        } catch (e: Exception) {
            addLog("Command send failed: ${e.message}")
            disconnect()
        }
    }

    suspend fun sendSpell(bitmap: Bitmap?, spell: String, mode: SaveMode = SaveMode.NOSAVE) = withContext(Dispatchers.IO) {
        val currentOutput = outputStream
        if (bitmap == null || currentOutput == null) {
            addLog("Cannot send: Bitmap null or Socket not connected")
            return@withContext
        }

        try {
            val bos = ByteArrayOutputStream()
            bitmap.compress(Bitmap.CompressFormat.PNG, 100, bos)
            val bytes = bos.toByteArray()
            val spellWithoutSpace = spell.replace(' ', '_')
            val header = "PLAYPNG $spellWithoutSpace.png ${bytes.size} ${mode.name.lowercase()}\n"
            currentOutput.write(header.toByteArray())
            currentOutput.write(bytes)
            currentOutput.flush()
            addLog("Spell sent! Size: ${bitmap.width}x${bitmap.height} Mode: ${mode.name}")
        } catch (e: Exception) {
            addLog("Send failed: ${e.message}")
            disconnect()
        }
    }

    fun sendWlanPNG() {
        scope.launch {
            val options = BitmapFactory.Options().apply {
                inScaled = false 
            }
            val bitmap = BitmapFactory.decodeResource(context.resources, R.drawable.wlan, options)
            sendSpell(bitmap, "wlan", SaveMode.NOSAVE)
        }
    }

    fun send1pxPNG() {
        scope.launch {
            val options = BitmapFactory.Options().apply {
                inScaled = false
            }
            val bitmap = BitmapFactory.decodeResource(context.resources, R.drawable._1px, options)
            sendSpell(bitmap, "1px", SaveMode.TEMP)
        }
    }
}
