# indi_ardust4

This is an implementation of a minimal INDI (http://www.indilib.org/) driver for Kevin Ferrares Arduino based USB to ST-4 adapter (https://github.com/kevinferrare/arduino-st4).


## License

This is based on the GPUSB driver and as such under the GPL2


## Features/Limitations
  * No way to dynamically assign a tty
  * No Goto, just North, West, South, East


## Building

    cmake .
    make

## Running

    indiserver ./indi_ardust4
