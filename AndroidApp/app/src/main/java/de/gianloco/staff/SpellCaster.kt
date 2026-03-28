package de.gianloco.staff

import android.content.Context
import android.graphics.Bitmap
import android.graphics.BitmapFactory
import android.util.Log
import com.chaquo.python.Python
import java.io.File
import java.io.FileOutputStream

class SpellCaster(private val context: Context, private val espClient: EspTcpClient) {
    private val TAG = "SpellCaster"

    private val spellPrompt = """
    I am a wizard and you are creating a light effect for my magic wand, which has a LED strip.

    Create a complete, executable Python script.

    The script should generate an image with exactly 118x1000 pixels. 
    The image is then interpreted as an 1D Video, starting from the top.

    Design requirements:
    * write a function with name "createSpell" that returns a byte array like this  
        img.save(buf, 'PNG')
        return buf.getvalue()   
    * Black background (RGB, no alpha)
    * No transparency
    * Do not use external image files
    * Use only the standard library or Pillow (PIL)
    * The code must contain no comments
    * The most important part: The effect should be fitting for a magic wand and visually interesting

    IMPORTANT, because the text is piped directly into a compiler:
    Return **only the complete Python code**.
    No text outside the code.

    The spell name is:
    """.trimIndent()

    suspend fun cast(spell: String): Bitmap? {
        val code = promptAi(spellPrompt + spell)

        try {
            val scriptFile = File(context.filesDir, "last_spell.py")
            Log.d("SpellCaster", code)
            scriptFile.writeText(code)
        } catch (e: Exception) {
            Log.e(TAG, "Failed to save script", e)
        }

        val bitmap = imageFromCode(code)
        
        if (bitmap != null) {
            saveBitmapToDisk(bitmap, "$spell.png")
            espClient.sendSpell(bitmap, spell, SaveMode.TEMP)
        }
        
        return bitmap
    }

    private fun imageFromCode(code: String): Bitmap? {
        return try {
            val py = Python.getInstance()

            val globals = py.builtins.callAttr("dict")
            py.builtins.callAttr("exec", code, globals)
            val bytes = globals.callAttr("get", "createSpell")?.call()?.toJava(ByteArray::class.java)
                ?: return null
            BitmapFactory.decodeByteArray(bytes, 0, bytes.size)
        } catch (e: Exception) {
            Log.e(TAG, "Error executing Python spell", e)
            null
        }
    }

    private fun saveBitmapToDisk(bitmap: Bitmap, filename: String) {
        try {
            val file = File(context.filesDir, filename)
            FileOutputStream(file).use { out ->
                bitmap.compress(Bitmap.CompressFormat.PNG, 100, out)
            }
            Log.d(TAG, "Image saved to: ${file.absolutePath}")
        } catch (e: Exception) {
            Log.e(TAG, "Error saving bitmap", e)
        }
    }
}
