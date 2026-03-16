package de.gianloco.staff

import android.content.Context
import android.graphics.Bitmap
import android.graphics.BitmapFactory
import android.net.wifi.WifiManager
import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.padding
import androidx.compose.material3.Scaffold
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.tooling.preview.Preview
import com.chaquo.python.Python
import com.chaquo.python.android.AndroidPlatform
import de.gianloco.staff.ui.theme.StaffTheme

class MainActivity : ComponentActivity() {

    init {
        if (!Python.isStarted()) {
            Python.start(AndroidPlatform(this))
        }
    }

    private lateinit var espClient: EspTcpClient

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        enableEdgeToEdge()
        setContent {
            StaffTheme {
                Scaffold(modifier = Modifier.fillMaxSize()) { innerPadding ->
                    Text(
                        "Staff App+ $innerPadding", modifier = Modifier.padding(innerPadding)
                    )
                }
            }
        }
        // Acquire Multicast Lock for mDNS
        val wifi = applicationContext.getSystemService(Context.WIFI_SERVICE) as WifiManager
        val lock = wifi.createMulticastLock("ESP32MDNSLock")
        lock.setReferenceCounted(true)
        lock.acquire()

        //search esp32
        espClient = EspTcpClient(this)
        espClient.discover()

    }

    override fun onDestroy() {
        super.onDestroy()
        // Optional: release multicast lock
        val wifi = applicationContext.getSystemService(Context.WIFI_SERVICE) as WifiManager
        val lock = wifi.createMulticastLock("ESP32MDNSLock")
        if (lock.isHeld) lock.release()
    }

}

suspend fun cast(spell: String){
    requestPythonFromAI(spell, "")
}

@Preview(showBackground = true)
@Composable
fun GreetingPreview() {
    StaffTheme {
        Text("Staff App")
    }
}

fun imageFromCode(code: String): Bitmap? {
    val py = Python.getInstance()
    val bytes = py.builtins
        .callAttr("exec", code)
        .toJava(ByteArray::class.java)

    return BitmapFactory.decodeByteArray(bytes, 0, bytes.size)
}