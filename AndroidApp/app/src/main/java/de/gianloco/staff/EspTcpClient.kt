package de.gianloco.staff

import android.content.Context
import android.graphics.Bitmap
import android.net.nsd.NsdManager
import android.net.nsd.NsdServiceInfo
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import java.io.ByteArrayOutputStream
import java.net.Socket

class EspTcpClient(context: Context) {

    private val serviceType = "_http._tcp."
    private val targetName = "NETWORKNAME"
    private var espIp: String? = null

    private val nsdManager =
        context.getSystemService(Context.NSD_SERVICE) as NsdManager

    fun discover() {

        val discoveryListener = object : NsdManager.DiscoveryListener {

            override fun onDiscoveryStarted(regType: String) {}

            override fun onServiceFound(service: NsdServiceInfo) {

                if (service.serviceName.contains(targetName)) {
                    nsdManager.resolveService(service, resolveListener)
                }
            }

            override fun onServiceLost(service: NsdServiceInfo) {}

            override fun onDiscoveryStopped(serviceType: String) {}

            override fun onStartDiscoveryFailed(serviceType: String, errorCode: Int) {}

            override fun onStopDiscoveryFailed(serviceType: String, errorCode: Int) {}
        }

        nsdManager.discoverServices(
            serviceType,
            NsdManager.PROTOCOL_DNS_SD,
            discoveryListener
        )
    }

    private val resolveListener = object : NsdManager.ResolveListener {

        override fun onResolveFailed(serviceInfo: NsdServiceInfo, errorCode: Int) {}

        override fun onServiceResolved(serviceInfo: NsdServiceInfo) {
            espIp = serviceInfo.host.hostAddress
        }
    }

    suspend fun sendSpell(bitmap: Bitmap?, spell: String) = withContext(Dispatchers.IO) {
        if (bitmap == null || espIp == null) return@withContext

        try {
            val socket = Socket(espIp, 888)
            val outputStream = socket.getOutputStream()

            val bos = ByteArrayOutputStream()
            bitmap.compress(Bitmap.CompressFormat.PNG, 100, bos)
            val bytes = bos.toByteArray()

            val filename = "$spell.png"
            val filesize = bytes.size

            val header = "PLAYSENDPNG $filename $filesize\n"
            outputStream.write(header.toByteArray())
            outputStream.write(bytes)
            outputStream.flush()

            socket.close()
        } catch (e: Exception) {
            e.printStackTrace()
        }
    }
}
