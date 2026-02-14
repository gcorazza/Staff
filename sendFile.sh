#!/usr/bin/env bash
# sendfiletoesp - Send a file to ESP32 via Bluetooth SPP

if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <filepath>"
    exit 1
fi

FILE="$1"

if [ ! -f "$FILE" ]; then
    echo "File not found: $FILE"
    exit 1
fi

# Extract filename and size
BASENAME=$(basename "$FILE")
SIZE=$(stat -c%s "$FILE")

# Find ESP32 address by Bluetooth name "abc"
ADDR=$(bluetoothctl -- timeout 5 scan on | grep -m1 "abc" | awk '{print $2}')

if [ -z "$ADDR" ]; then
    echo "ESP32 with name 'abc' not found. Make sure it is powered on and discoverable."
    exit 1
fi

# Usually SPP channel is 1
CHANNEL=1

echo "Found ESP32 at $ADDR, channel $CHANNEL"
echo "Sending $BASENAME ($SIZE bytes)..."

# Send file via socat with protocol
{
    printf "START %s %d\n" "$BASENAME" "$SIZE"
    cat "$FILE"
} | socat - rfcomm:"$ADDR",channel="$CHANNEL"

echo "Done."
