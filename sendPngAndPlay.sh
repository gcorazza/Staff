#!/bin/bash
# sendfile.sh - Send a file to ESP32 TCP server

if [ "$#" -ne 3 ]; then
    echo "Usage: $0 <ESP32_IP> <PORT> <FILE>"
    exit 1
fi

ESP32_IP="$1"
PORT="$2"
FILE="$3"

if [ ! -f "$FILE" ]; then
    echo "File not found: $FILE"
    exit 1
fi

# Get file size in bytes
FILESIZE=$(stat -c%s "$FILE")

# Extract just the filename
FILENAME=$(basename "$FILE")

echo "Sending $FILE ($FILESIZE bytes) to $ESP32_IP:$PORT"

# Send the SENDFILE command first, then file contents
{
    printf "PLAYPNG %s %d\n" "$FILENAME" "$FILESIZE"
    cat "$FILE"
} | nc -q 0 "$ESP32_IP" "$PORT"
echo
echo "Done."