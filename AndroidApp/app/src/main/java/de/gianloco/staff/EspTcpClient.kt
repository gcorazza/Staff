package de.gianloco.staff

import android.content.Context
import android.graphics.Bitmap
import android.net.nsd.NsdManager
import android.net.nsd.NsdServiceInfo
import android.util.Log
import kotlinx.coroutines.*
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import java.io.ByteArrayOutputStream
import java.io.OutputStream
import java.net.InetAddress
import java.net.InetSocketAddress
import java.net.Socket

enum class ConnectionStatus {
    DISCONNECTED,
    CONNECTING,
    CONNECTED
}

class EspTcpClient(context: Context) {

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
                if (service.host?.hostAddress == espIp) {
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
        
        // Background loop for reconnection and direct IP resolution
        scope.launch {
            while (isActive) {
                if (socket == null || socket?.isConnected == false) {
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
                newSocket.connect(InetSocketAddress(ip, 888), 5000)
                socket = newSocket
                outputStream = newSocket.getOutputStream()
                _status.value = ConnectionStatus.CONNECTED
                Log.d(TAG, "Connected successfully!")
            } catch (e: Exception) {
                Log.e(TAG, "Connection failed: ${e.message}")
                _status.value = ConnectionStatus.DISCONNECTED
                socket = null
            }
        }
    }

    private fun disconnect() {
        Log.d(TAG, "Disconnecting...")
        _status.value = ConnectionStatus.DISCONNECTED
        
        val socketToClose = socket
        socket = null
        outputStream = null

        scope.launch(Dispatchers.IO) {
            try {
                socketToClose?.close()
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

            val header = "PLAYSENDPNG $spell.png ${bytes.size}\n"
            currentOutput.write(header.toByteArray())
            currentOutput.write(bytes)
            currentOutput.flush()
            Log.d(TAG, "Spell sent!")
        } catch (e: Exception) {
            Log.e(TAG, "Send failed", e)
            disconnect()
        }
    }
}
