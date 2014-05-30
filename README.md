Arduino "MIDIsync" MIDI and CV Master Clock
=
ɔ 2012-2014 Tom Hensel <tom@jitter.eu> Hamburg, Germany
CC BY-SA 3.0 http://creativecommons.org/licenses/by-sa/3.0/

At least running on
- Arduino Duemilanove http://arduino.cc/en/Main/arduinoBoardDuemilanove
- Arduino Uno http://arduino.cc/en/Main/arduinoBoardUno (.hex binary provided, and platform of test)

Should (still!) be compatible with the famous-ol-MIDI-Shield from Sparkfun (https://www.sparkfun.com/products/9595).

Schematics and stuff are work in progress. Please stand by. Or wait forever.
Pre-built devices are available upon request (1 left) - please feel free to contact me!
Libraries are included now, you might still have a hard time compiling the code on your own.

A pre-built device (nicknamed "Hensel MODEL-01") has the following features:

	- High-precision Master Clock
		- MIDI Clock Output
		- CV/Gate Output (0V/5V)
	- Micro-adjustable Tempo (endless rotary knob, two speeds)
	- Tap-Tempo Function (dropout protection, auto-smoothing)
	- [Supports output of 1, 12, 24 and 48 clocks per quarter note]
	- Control MIDI Equipment (start, continue, stop)
	- Split-Mode - Synchronize to external MIDI Clock
		- Quickly toggle between both speeds (internal/external)
		- [Control pass-through of control messages to MIDI-OUT]
	- Settings can be saved in non-volatile memory
	- Jumbo Tri-Color LEDs for Tempo and Status display
	- MIDI-Thru (latency-free hardware circuit)
	- [MIDI-Forward (merge MIDI-In with generated clock signals to MIDI-Out)]
	- Tolerant power input (7-12V DC) and hardware power switch
		- Low-power design, runs on batteries and USB-power (adapter required)
		- Energy-efficient power supply included
