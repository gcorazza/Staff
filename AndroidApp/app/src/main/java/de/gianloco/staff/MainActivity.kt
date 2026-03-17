package de.gianloco.staff

import android.content.Context
import android.content.Intent
import android.graphics.Bitmap
import android.net.wifi.WifiManager
import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import androidx.compose.foundation.Image
import androidx.compose.foundation.layout.*
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.asImageBitmap
import androidx.compose.ui.unit.dp
import androidx.lifecycle.lifecycleScope
import com.chaquo.python.Python
import com.chaquo.python.android.AndroidPlatform
import de.gianloco.staff.ui.theme.StaffTheme
import kotlinx.coroutines.launch

class MainActivity : ComponentActivity(), ArcaneSpeech.SpellListener {

    private lateinit var espClient: EspTcpClient
    private lateinit var arcaneSpeech: ArcaneSpeech
    private lateinit var spellCaster: SpellCaster

    private var recognizedSpell by mutableStateOf<String?>(null)
    private var isListening by mutableStateOf(false)
    private var spellImage by mutableStateOf<Bitmap?>(null)

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        if (!Python.isStarted()) {
            Python.start(AndroidPlatform(this))
        }

        enableEdgeToEdge()

        espClient = EspTcpClient(this)
        espClient.discover()
        arcaneSpeech = ArcaneSpeech(this)
        spellCaster = SpellCaster(espClient)

        setContent {
            StaffTheme {
                Scaffold(modifier = Modifier.fillMaxSize()) { innerPadding ->
                    SpellScreen(
                        modifier = Modifier.padding(innerPadding),
                        isListening = isListening,
                        recognizedSpell = recognizedSpell,
                        spellImage = spellImage,
                        onStartListening = {
                            isListening = true
                            arcaneSpeech.startListening(this)
                        }
                    )
                }
            }
        }

        // Acquire Multicast Lock for mDNS
        val wifi = applicationContext.getSystemService(Context.WIFI_SERVICE) as WifiManager
        val lock = wifi.createMulticastLock("ESP32MDNSLock")
        lock.setReferenceCounted(true)
        lock.acquire()
    }

    @Deprecated("Deprecated in Java")
    override fun onActivityResult(requestCode: Int, resultCode: Int, data: Intent?) {
        super.onActivityResult(requestCode, resultCode, data)
        arcaneSpeech.handleResult(requestCode, resultCode, data)
        isListening = false
    }

    override fun onSpellRecognized(spell: String) {
        recognizedSpell = spell
        lifecycleScope.launch {
            spellImage = spellCaster.cast(spell)
        }
    }

    override fun onWakeWordDetected() {
        // Optional: Provide feedback when wake word is detected
    }

    override fun onError() {
        isListening = false
    }

    override fun onDestroy() {
        super.onDestroy()
        val wifi = applicationContext.getSystemService(Context.WIFI_SERVICE) as WifiManager
        val lock = wifi.createMulticastLock("ESP32MDNSLock")
        if (lock.isHeld) lock.release()
    }
}

@Composable
fun SpellScreen(
    modifier: Modifier = Modifier,
    isListening: Boolean,
    recognizedSpell: String?,
    spellImage: Bitmap?,
    onStartListening: () -> Unit
) {
    Column(
        modifier = modifier.fillMaxSize(),
        horizontalAlignment = Alignment.CenterHorizontally,
        verticalArrangement = Arrangement.Center
    ) {
        Button(
            onClick = onStartListening,
            colors = ButtonDefaults.buttonColors(
                containerColor = if (isListening) MaterialTheme.colorScheme.error else MaterialTheme.colorScheme.primary
            )
        ) {
            Text(if (isListening) "End Spell" else "Start ArcaneSpeech")
        }

        Spacer(modifier = Modifier.height(32.dp))

        recognizedSpell?.let {
            Text(
                text = "Spell: $it",
                style = MaterialTheme.typography.headlineMedium,
                modifier = Modifier.padding(horizontal = 16.dp)
            )
        }

        Spacer(modifier = Modifier.height(16.dp))

        spellImage?.let {
            Image(
                bitmap = it.asImageBitmap(),
                contentDescription = "Spell Visualization",
                modifier = Modifier
                    .fillMaxWidth()
                    .height(250.dp)
                    .padding(16.dp)
            )
        }
    }
}
