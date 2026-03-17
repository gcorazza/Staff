package de.gianloco.staff

import android.graphics.Bitmap
import android.graphics.BitmapFactory
import com.chaquo.python.Python

class SpellCaster(private val espClient: EspTcpClient) {
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

    suspend fun cast(spell: String): Bitmap? {
        val phytonCode = promptAi(spellPrompt + spell)
        val image = imageFromCode(phytonCode)
        espClient.sendSpell(image, spell)
        return image
    }

    fun imageFromCode(code: String): Bitmap? {
        val py = Python.getInstance()
        val globals = py.builtins.callAttr("dict")
        py.builtins.callAttr("exec", code, globals)

        val bytes = py.getModule("io")
            .callAttr("open", "output.png", "rb")
            .callAttr("read")
            .toJava(ByteArray::class.java)

        return BitmapFactory.decodeByteArray(bytes, 0, bytes.size)
    }
}
