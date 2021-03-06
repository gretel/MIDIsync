Arduino "MIDIsync" MIDI and CV Master Clock
=
ɔ 2012-2017 Tom Hensel <tom@jitter.eu> Hamburg, Germany - License at https://opensource.org/licenses/BSD-3-Clause

At least running on
- [Arduino Uno](http://arduino.cc/en/Main/arduinoBoardUno) [![Build Status: Arduino Uno, Leonardo](https://travis-ci.org/gretel/MIDIsync.svg)](https://travis-ci.org/gretel/MIDIsync)
- [Teensy 3.1](https://www.pjrc.com/store/teensy31.html) [![Build Status: Teensy 3.1](https://travis-ci.org/gretel/MIDIsync.svg?branch=teensy31)](https://travis-ci.org/gretel/MIDIsync)

Should be compatible with the MIDI-Shield from [Sparkfun](https://www.sparkfun.com/products/9595).

Libraries are included as *git submodules*. To clone the project and it's submodules please do:

```shell
$ git clone https://github.com/gretel/MIDIsync.git
$ cd MIDIsync
$ git submodule init
$ git submodule update
```

Building and uploading can be done easily using [platformio](http://platformio.org).

> Currently, `python 2.7.10` is required as well as `pip`.

Please edit `platformio.ini` first to reflect your serial port.

```shell
$ pip install -U pip setuptools
$ pip install -U platformio
$ cd MIDIsync
$ platformio run
...
```

Working on schematics.. _Please assist if you have the time and skills!_

My devices running it (nicknamed "Hensel MODEL-01") have the following features:

- High-precision Master Clock
	- MIDI Clock Output
	- CV/Gate Output (0V/5V)
- Micro-adjustable Tempo (endless rotary knob, two speeds)
- Tap-Tempo Function (dropout protection, auto-smoothing)
- Control MIDI Equipment (start, continue, stop)
- Settings can be saved in non-volatile memory
- Jumbo Tri-Color LEDs for Tempo and Status display
- MIDI-Thru (latency-free hardware circuit)
- Tolerant power input (7-12V DC) and hardware power switch
	- Low-power design, runs on batteries and USB-power (adapter required)
	- Energy-efficient power supply included

Please get in touch with us on *Gitter*:

[![Join the chat at https://gitter.im/gretel/MIDIsync](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/gretel/MIDIsync?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)
