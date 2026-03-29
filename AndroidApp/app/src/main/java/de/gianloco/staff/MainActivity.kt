package de.gianloco.staff

import android.content.Context
import android.content.Intent
import android.graphics.Bitmap
import android.net.wifi.WifiManager
import android.os.Bundle
import android.util.Log
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import androidx.compose.animation.animateColorAsState
import androidx.compose.foundation.Image
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.asImageBitmap
import androidx.compose.ui.text.font.FontFamily
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
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
        espClient.discoverAndConnect()
        
        arcaneSpeech = ArcaneSpeech(this)
        spellCaster = SpellCaster(this, espClient)

        setContent {
            StaffTheme {
                val connectionStatus by espClient.status.collectAsState()
                val lastConnectedIp by espClient.lastConnectedIp.collectAsState()
                val isDiscoveryRunning by espClient.isDiscoveryRunning.collectAsState()
                val logs by espClient.logs.collectAsState()
                
                Scaffold(modifier = Modifier.fillMaxSize()) { innerPadding ->
                    SpellScreen(
                        modifier = Modifier.padding(innerPadding),
                        isListening = isListening,
                        recognizedSpell = recognizedSpell,
                        spellImage = spellImage,
                        connectionStatus = connectionStatus,
                        lastConnectedIp = lastConnectedIp,
                        isDiscoveryRunning = isDiscoveryRunning,
                        logs = logs,
                        onStartListening = {
                            isListening = true
                            arcaneSpeech.startListening(this)
                        },
                        onSendWlanPNG = {
                            espClient.send1pxPNG()
                        },
                        onSendLsCommand = {
                            lifecycleScope.launch {
                                espClient.sendCommand("ls")
                            }
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

    override fun onWakeWordDetected() {}
    override fun onError() { isListening = false }
}

@Composable
fun SpellScreen(
    modifier: Modifier = Modifier,
    isListening: Boolean,
    recognizedSpell: String?,
    spellImage: Bitmap?,
    connectionStatus: ConnectionStatus,
    lastConnectedIp: String?,
    isDiscoveryRunning: Boolean,
    logs: List<String>,
    onStartListening: () -> Unit,
    onSendWlanPNG: () -> Unit,
    onSendLsCommand: () -> Unit
) {
    Column(
        modifier = modifier.fillMaxSize(),
        horizontalAlignment = Alignment.CenterHorizontally
    ) {
        // --- Status Section ---
        Card(
            modifier = Modifier
                .fillMaxWidth()
                .padding(16.dp),
            colors = CardDefaults.cardColors(containerColor = MaterialTheme.colorScheme.surfaceVariant)
        ) {
            Column(modifier = Modifier.padding(12.dp)) {
                // Connection Status
                Row(verticalAlignment = Alignment.CenterVertically) {
                    val statusColor by animateColorAsState(
                        when (connectionStatus) {
                            ConnectionStatus.CONNECTED -> Color.Green
                            ConnectionStatus.CONNECTING -> Color.Yellow
                            ConnectionStatus.DISCONNECTED -> Color.Red
                        }, label = "statusColor"
                    )
                    Box(modifier = Modifier.size(10.dp).clip(CircleShape).background(statusColor))
                    Spacer(modifier = Modifier.width(8.dp))
                    Text(
                        text = when (connectionStatus) {
                            ConnectionStatus.CONNECTED -> "ESP32 Connected"
                            ConnectionStatus.CONNECTING -> "Connecting..."
                            ConnectionStatus.DISCONNECTED -> "ESP32 Disconnected"
                        },
                        style = MaterialTheme.typography.titleMedium
                    )
                }
                
                Spacer(modifier = Modifier.height(4.dp))
                Text("Last IP: ${lastConnectedIp ?: "None"}", style = MaterialTheme.typography.bodySmall)
                
                Row(verticalAlignment = Alignment.CenterVertically) {
                    Text("Discovery: ", style = MaterialTheme.typography.bodySmall)
                    Text(
                        text = if (isDiscoveryRunning) "Searching..." else "Idle",
                        style = MaterialTheme.typography.bodySmall,
                        color = if (isDiscoveryRunning) MaterialTheme.colorScheme.primary else Color.Gray
                    )
                }
            }
        }

        // --- Controls Section ---
        Row(
            modifier = Modifier.fillMaxWidth().padding(horizontal = 16.dp),
            horizontalArrangement = Arrangement.spacedBy(8.dp)
        ) {
            Button(
                onClick = onStartListening,
                modifier = Modifier.weight(1f),
                enabled = connectionStatus == ConnectionStatus.CONNECTED,
                colors = ButtonDefaults.buttonColors(
                    containerColor = if (isListening) MaterialTheme.colorScheme.error else MaterialTheme.colorScheme.primary
                )
            ) {
                Text(if (isListening) "End" else "Cast", fontSize = 12.sp)
            }

            Button(onClick = onSendWlanPNG, modifier = Modifier.weight(1f), enabled = connectionStatus == ConnectionStatus.CONNECTED) {
                Text("Wlan", fontSize = 12.sp)
            }

            Button(onClick = onSendLsCommand, modifier = Modifier.weight(1f), enabled = connectionStatus == ConnectionStatus.CONNECTED) {
                Text("ls", fontSize = 12.sp)
            }
        }

        // --- Result Section ---
        recognizedSpell?.let {
            Text(
                text = "Spell: $it",
                style = MaterialTheme.typography.headlineSmall,
                modifier = Modifier.padding(top = 16.dp)
            )
        }

        Box(modifier = Modifier.weight(1f).padding(16.dp), contentAlignment = Alignment.Center) {
            spellImage?.let {
                Image(
                    bitmap = it.asImageBitmap(),
                    contentDescription = "Spell Visualization",
                    modifier = Modifier.fillMaxHeight().clip(RoundedCornerShape(8.dp))
                )
            } ?: Text("No spell cast yet", color = Color.Gray)
        }

        // --- Log Section ---
        Text("System Logs", style = MaterialTheme.typography.labelSmall, modifier = Modifier.align(Alignment.Start).padding(start = 16.dp))
        Surface(
            modifier = Modifier
                .fillMaxWidth()
                .height(150.dp)
                .padding(bottom = 16.dp, start = 16.dp, end = 16.dp),
            color = Color.Black.copy(alpha = 0.05f),
            shape = RoundedCornerShape(4.dp)
        ) {
            LazyColumn(
                modifier = Modifier.padding(8.dp),
                reverseLayout = false
            ) {
                items(logs) { log ->
                    Text(
                        text = log,
                        style = MaterialTheme.typography.bodySmall.copy(
                            fontFamily = FontFamily.Monospace,
                            fontSize = 10.sp
                        ),
                        modifier = Modifier.padding(vertical = 1.dp)
                    )
                }
            }
        }
    }
}
