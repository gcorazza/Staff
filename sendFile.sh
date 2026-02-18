#!/bin/bash

set -e

BT_NAME="Staff"
RFCOMM_DEV=0

if [ -z "$1" ]; then
    echo "Usage: $0 <filepath>"
    exit 1
fi

FILE="$1"

if [ ! -f "$FILE" ]; then
    echo "File not found: $FILE"
    exit 1
fi

BASENAME=$(basename "$FILE")
SIZE=$(stat -c%s "$FILE")

echo "Searching for ESP32 named \"$BT_NAME\"..."

# Get device address from known devices
ADDR=$(bluetoothctl devices | awk -v name="$BT_NAME" '$0 ~ name {print $2}')

if [ -z "$ADDR" ]; then
    echo "Device not known yet. Scanning..."
    timeout 8 bluetoothctl scan on >/dev/null 2>&1 || true
    ADDR=$(bluetoothctl devices | awk -v name="$BT_NAME" '$0 ~ name {print $2}')
fi

if [ -z "$ADDR" ]; then
    echo "ESP32 with name \"$BT_NAME\" not found."
    exit 1
fi

echo "Found device at $ADDR"

# Get RFCOMM channel
CHANNEL=$(sdptool browse "$ADDR" | awk '/Channel:/ {print $2; exit}')

if [ -z "$CHANNEL" ]; then
    echo "Could not determine RFCOMM channel."
    exit 1
fi

echo "Using RFCOMM channel $CHANNEL"

echo "Binding RFCOMM..."
sudo rfcomm release $RFCOMM_DEV 2>/dev/null || true
sudo rfcomm bind $RFCOMM_DEV "$ADDR" $CHANNEL

DEV="/dev/rfcomm$RFCOMM_DEV"

if [ ! -e "$DEV" ]; then
    echo "RFCOMM device not created."
    exit 1
fi

echo "Sending $BASENAME ($SIZE bytes)..."


printf "START %s %d\n" "$BASENAME" "$SIZE" > "$DEV"
pv -L 1k "$FILE" > "$DEV"

echo "Transfer complete."

# Warten bis Kernel-Buffer leer
sleep 1.5

sync

echo "Releasing RFCOMM..."
sudo rfcomm release $RFCOMM_DEV

echo "Done."
