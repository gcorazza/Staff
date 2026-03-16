package de.gianloco.staff

import android.content.Context
import android.net.nsd.NsdManager
import android.net.nsd.NsdServiceInfo
import java.io.BufferedReader
import java.io.InputStreamReader
import java.io.PrintWriter
import java.net.Socket

class EspTcpClient(private val context: Context) {

    private val serviceType = "_http._tcp."
    private val targetName = "NETWORKNAME"

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

            val host = serviceInfo.host.hostAddress
            connect(host)
        }
    }

    private fun connect(ip: String) {

        Thread {

            val socket = Socket(ip, 888)

            val writer = PrintWriter(socket.getOutputStream(), true)
            val reader = BufferedReader(
                InputStreamReader(socket.getInputStream())
            )

            writer.println("hello esp32")

            val response = reader.readLine()

            println("ESP32: $response")

            socket.close()

        }.start()
    }
}