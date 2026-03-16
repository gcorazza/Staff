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


private var hfToken =  "***";
suspend fun promptAi(prompt: String): String {

    // 2. JSON Daten bauen
    val mapper = jacksonObjectMapper()
    val jsonData = mapper.writeValueAsString(
        mapOf(
            "model" to "openai/gpt-oss-120b:fastest",
            "messages" to listOf(mapOf("role" to "user", "content" to prompt)),
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