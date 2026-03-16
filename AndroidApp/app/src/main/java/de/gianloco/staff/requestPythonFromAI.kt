package de.gianloco.staff

import io.ktor.client.*
import io.ktor.client.call.*
import io.ktor.client.engine.cio.*
import io.ktor.client.plugins.contentnegotiation.*
import io.ktor.client.request.*
import io.ktor.http.*
import com.fasterxml.jackson.module.kotlin.jacksonObjectMapper
import com.fasterxml.jackson.module.kotlin.readValue
import com.fasterxml.jackson.databind.JsonNode
import io.ktor.serialization.jackson.jackson
import java.io.File

val spellPrompt= """
    I am a wizard and you are creating a light effect for my magic wand.

    Create a complete, executable Python script.

    The script should generate an image with exactly 113 pixels in width.

    IMPORTANT – Interpretation of the image:
    The image is a spatiotemporal map for a 1D LED strip.

    * 113 pixels WIDTH = spatial position of the LEDs (left to right = LED 0 to LED 112)
    * X pixels HEIGHT = time axis (top to bottom = frame 0 to frame X-1)
    * Each row represents a moment in time.
    * Each column represents an LED position.
    * The image thus describes an animation over X frames for 113 LEDs.
    * The animation should be 2 seconds long
    * One second equals 500 pixels

    Design requirements:

    * Clear, bright colors (suitable for LEDs)
    * Black background (RGB, no alpha)
    * High contrast
    * No transparency
    * Do not use external image files
    * Use only the standard library or Pillow (PIL)
    * Save the image as "output.png"
    * Exactly 113x1000 pixels
    * The code must contain no comments
    * The effect should be fitting for a magic wand and visually interesting

    IMPORTANT, because the text is piped directly into a compiler:
    Return **only the complete Python code**.
    No text outside the code.

    The spell name is:
""".trimIndent()

suspend fun requestPythonFromAI(spellName: String, hfToken: String): String {
    // 1. Prompt-Datei lesen
    val promptContent = spellPrompt
    val fullPrompt = "$promptContent $spellName"

    // 2. JSON Daten bauen
    val mapper = jacksonObjectMapper()
    val jsonData = mapper.writeValueAsString(
        mapOf(
            "model" to "openai/gpt-oss-120b:fastest",
            "messages" to listOf(mapOf("role" to "user", "content" to fullPrompt)),
            "stream" to false
        )
    )

    // 3. HTTP Client
    val client = HttpClient(CIO) {
        install(ContentNegotiation) {
            jackson()
        }
    }

    // 4. Anfrage senden
    val responseText: String = client.post("https://router.huggingface.co/v1/chat/completions") {
        headers {
            append(HttpHeaders.Authorization, "Bearer $hfToken")
            append(HttpHeaders.ContentType, ContentType.Application.Json.toString())
        }
        setBody(jsonData)
    }.body()

    client.close()

    // 5. Antwort parsen
    val jsonNode: JsonNode = mapper.readValue(responseText)
    val content = jsonNode["choices"]?.get(0)?.get("message")?.get("content")?.asText() ?: ""

    // 6. Zeilen ohne ``` behalten
    val modified = content.lines().filter { !it.contains("```") }.joinToString("\n")

    return modified
}