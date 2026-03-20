package de.gianloco.staff;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.speech.RecognizerIntent;

import java.util.ArrayList;
import java.util.Locale;

public class ArcaneSpeech {

    public interface SpellListener {
        void onSpellRecognized(String spell);
        void onWakeWordDetected();
        void onError();
    }

    public static final int SPEECH_REQUEST_CODE = 1001;

    private SpellListener listener;

    public ArcaneSpeech(SpellListener listener) {
        this.listener = listener;
    }

    public void startListening(Activity activity) {

        Intent intent = new Intent(RecognizerIntent.ACTION_RECOGNIZE_SPEECH);

        intent.putExtra(
                RecognizerIntent.EXTRA_LANGUAGE_MODEL,
                RecognizerIntent.LANGUAGE_MODEL_FREE_FORM
        );

        intent.putExtra(
                RecognizerIntent.EXTRA_LANGUAGE,
                Locale.getDefault()
        );

        intent.putExtra(
                RecognizerIntent.EXTRA_PROMPT,
                "Speak your spell..."
        );

        activity.startActivityForResult(intent, SPEECH_REQUEST_CODE);
    }

    public void handleResult(int requestCode, int resultCode, Intent data) {

        if (requestCode != SPEECH_REQUEST_CODE) return;

        if (resultCode != Activity.RESULT_OK || data == null) {
            if(listener != null) listener.onError();
            return;
        }

        ArrayList<String> results =
                data.getStringArrayListExtra(RecognizerIntent.EXTRA_RESULTS);

        if(results == null || results.isEmpty()) return;

        String text = results.get(0).toLowerCase();

        // normalize common misrecognitions
//        text = text.replace("bite", "byte")
//                   .replace("by", "byte");

//        if(text.contains("arcane byte")) {
//
//            if(listener != null) listener.onWakeWordDetected();
//
//            String spell = text.replace("arcane byte", "").trim();
//
//
//        }
        if(listener != null) listener.onSpellRecognized(text);
    }
}
