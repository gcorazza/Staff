package de.gianloco.staff

import android.content.Context
import android.graphics.Bitmap
import android.net.nsd.NsdManager
import android.net.nsd.NsdServiceInfo
import kotlinx.coroutines.*
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import java.io.ByteArrayOutputStream
import java.io.OutputStream
import java.net.InetSocketAddress
import java.net.Socket

enum class ConnectionStatus {
    DISCONNECTED,
    CONNECTING,
    CONNECTED
}

class EspTcpClient(context: Context) {

    private val serviceType = "_http._tcp."
    private val targetName = "arcanebyte"
    private var espIp: String? = null
    
    private val _status = MutableStateFlow(ConnectionStatus.DISCONNECTED)
    val status: StateFlow<ConnectionStatus> = _status

    private val nsdManager = context.getSystemService(Context.NSD_SERVICE) as NsdManager
    private var socket: Socket? = null
    private var outputStream: OutputStream? = null
    
    private val scope = CoroutineScope(Dispatchers.IO + SupervisorJob())

    fun discoverAndConnect() {
        _status.value = ConnectionStatus.CONNECTING
        
        // Start discovery
        val discoveryListener = object : NsdManager.DiscoveryListener {
            override fun onDiscoveryStarted(regType: String) {}
            override fun onServiceFound(service: NsdServiceInfo) {
                if (service.serviceName.contains(targetName)) {
                    nsdManager.resolveService(service, resolveListener)
                }
            }
            override fun onServiceLost(service: NsdServiceInfo) {
                if (service.serviceName.contains(targetName)) {
                    espIp = null
                    disconnect()
                }
            }
            override fun onDiscoveryStopped(serviceType: String) {}
            override fun onStartDiscoveryFailed(serviceType: String, errorCode: Int) {
                _status.value = ConnectionStatus.DISCONNECTED
            }
            override fun onStopDiscoveryFailed(serviceType: String, errorCode: Int) {}
        }

        nsdManager.discoverServices(serviceType, NsdManager.PROTOCOL_DNS_SD, discoveryListener)
        
        // Start connection loop
        scope.launch {
            while (isActive) {
                if (espIp != null && (socket == null || socket?.isConnected == false)) {
                    tryConnect()
                }
                delay(3000) // Retry every 3 seconds if not connected
            }
        }
    }

    private val resolveListener = object : NsdManager.ResolveListener {
        override fun onResolveFailed(serviceInfo: NsdServiceInfo, errorCode: Int) {}
        override fun onServiceResolved(serviceInfo: NsdServiceInfo) {
            espIp = serviceInfo.host.hostAddress
        }
    }

    private suspend fun tryConnect() {
        val ip = espIp ?: return
        withContext(Dispatchers.IO) {
            try {
                _status.value = ConnectionStatus.CONNECTING
                val newSocket = Socket()
                newSocket.connect(InetSocketAddress(ip, 888), 5000)
                socket = newSocket
                outputStream = newSocket.getOutputStream()
                _status.value = ConnectionStatus.CONNECTED
            } catch (e: Exception) {
                e.printStackTrace()
                _status.value = ConnectionStatus.DISCONNECTED
                socket = null
            }
        }
    }

    private fun disconnect() {
        try {
            socket?.close()
        } catch (e: Exception) {}
        socket = null
        outputStream = null
        _status.value = ConnectionStatus.DISCONNECTED
    }

    suspend fun sendSpell(bitmap: Bitmap?, spell: String) = withContext(Dispatchers.IO) {
        val currentOutput = outputStream
        if (bitmap == null || currentOutput == null) return@withContext

        try {
            val bos = ByteArrayOutputStream()
            bitmap.compress(Bitmap.CompressFormat.PNG, 100, bos)
            val bytes = bos.toByteArray()

            val filename = "$spell.png"
            val filesize = bytes.size

            val header = "PLAYSENDPNG $filename $filesize\n"
            currentOutput.write(header.toByteArray())
            currentOutput.write(bytes)
            currentOutput.flush()
        } catch (e: Exception) {
            e.printStackTrace()
            disconnect() // Reconnect on next loop iteration
        }
    }
}
