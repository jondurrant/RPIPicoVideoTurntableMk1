# RPIPicoVideoTurntableMk1
RPI Pico Video Turntable Mk1. Vidio on this project is available at [https://youtube.com/@drjonea](https://youtube.com/@drjonea).

Still images are boring and when they are PCB or breadboard sometimes dificult to understand from one angle. This project is a controlled turntable for small scale video work on my projects. 

## Features

**Lighting** Uses a strip of WS2812B LEDs to illuminate the photo area. Controlled in terms of Hue, Saturation and Value.

**Calibration** Self calibrating turn table driven by a 28BYJ-48 stepper motor

**Menu** Simple menu displayed on a OLED display and controled through a Rotary Encoder.

**Sequence** Switch 1 moves the turntable to start possition by shortest route and then moves to the end possition in the direction and speed set in the configuration. Good for setup and repeat takes of the same shot

**Stop** Switch 2 stops the turntable.

**Continuous Rotation** Switch 3 provides continuous rotation and the currently configured speed. Short press is clockwise, long press is counterclockwise.

**Reset** Switch 4 resets the turntable

**Pico Reset** Switch 5 is the Pico Reset.

## Schematic and PCB

KiCad files have been included in the Schematic folder.

PDF version of the schematic is also included.

Gerb files are included in the PCB folder 


## 3D Models

OpenSCAD project is in the folder 3DModels. You will need two supporting libraries.

Download [gears.scad](https://www.thingiverse.com/thing:4194148/files)

Download [stepper.scad](https://www.thingiverse.com/thing:4031757/files)

## Libraries

+ [FreeRTOS Kernel](https://github.com/FreeRTOS/FreeRTOS-Kernel)
+ [PicoLED](https://github.com/ForsakenNGS/PicoLED)
+ [pico-ssd1306](https://github.com/daschr/pico-ssd1306)


## Clone and Build

```
git clone --recurse-submodules https://github.com/jondurrant/RPIPicoVideoTurntableMk1
cd RPIPicoVideoTurntableMk1
mkdir build
cd build
cmake ..
make
```

## Links

+ Further YouTube projects on my channel [YouTube @DrJonEa](https://youtube.com/@drjonea).
+ Blogs at [DrJonEa.co.uk](https://drjonea.co.uk/IoT)
+ RPI Pico Courses at [Udemy @DrJonEa](https://drjonea.co.uk/training/)
