# Staff

## How to build
1. Install Librarys
- FastLED
- esp32
- PNGdec

2. set partition table: 2MB/2MB no OTA SPIFF
3. set Board to: ESP32 dev board

## What's all about this Project??

I build a magic wand which has a programmable 2m LED Strip from bottom to top and a gyroscope and acceleration sensor build in.
This project implements the logic for the LED and Sensors.

## Todo:

- Implement shell sort on Staff
- More Gyroscope and Accelerometer Animations


## Tools

# Send file to ESP32

./sendPngAndPlay.sh <file>


# flash data directory:
```zsh
mklittlefs -c data -b 4096 -p 256 -s 0x1E0000 littlefs.bin &&
esptool --port /dev/ttyUSB0 write-flash 0x210000 littlefs.bin
```

# reset flash
```zsh
esptool.py --chip esp32 --port /dev/ttyUSB0 erase_flash
```

# restart
```zsh
esptool --no-stub flash_id
```
