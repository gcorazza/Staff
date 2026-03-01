# Staff
- Forking Hell
## What's all about this Project??

I build a magic wand which has a programmable 2m LED Strip from bottom to top and a gyroscope and acceleration sensor build in.
This project implements the logic for the LED and Sensors.

## Tools

I programmed two web based tool to bake curves to an array:
- #### cubicCurveToEvenList.html
  Interpolates points with spline functions
- #### svgToEvenList.html
  converts a svg curve, see bootyfull.svg as an example (convenient to edit in gimp)


Todo:

- Implement shell sort on Staff
- Raw Video Data on sd card and play it back
- More Gyroscope and Accelerometer Animations



# Send file to ESP32

echo -e "power on\nagent on\ndng\nscan on" | bluetoothctl
> pair <mac>
> trust <mac>

./sendFile.sh <file>


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
