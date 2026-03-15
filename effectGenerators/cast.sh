#!/bin/bash

HF_TOKEN="***REMOVED***"

# Die Anfrage aus AIPrompt + Parameter
PROMPT_FILE="AIPrompt"
USER_ARG="$1"

# Inhalt von AIPrompt lesen und richtig escapen
PROMPT=$(<"$PROMPT_FILE")
FULL_PROMPT="$PROMPT $USER_ARG"

DATA=$(jq -n --arg prompt "$FULL_PROMPT" '
{
  "messages": [{"role":"user","content":$prompt}],
  "model":"openai/gpt-oss-120b:fastest",
  "stream":false
}')

## Anfrage senden
RESPONSE=$(curl https://router.huggingface.co/v1/chat/completions \
    -v \
    -H "Authorization: Bearer $HF_TOKEN" \
    -H 'Content-Type: application/json' \
    -d "$DATA" 2>/dev/null) &&

echo $RESPONSE

# Antwort extrahieren
CONTENT=$(echo "$RESPONSE" | jq -r '.choices[0].message.content') &&

# Zeilen ohne ```` behalten
MODIFIED=$(echo "$CONTENT" | grep -v '```')
# Skript in Datei schreiben
echo "$MODIFIED" > spell.py &&
# Skript ausführen
python spell.py &&
# Bild in content Ordner kopieren
cp output.png ../data/effects/disch.png &&

# Send PNG to ESP32
sendPngAndPlay.sh Staff 888 output.png