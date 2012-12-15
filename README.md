Arduino "MIDIsync"
ATMEGA328P based MIDI Master Clock Generator
===============================================================

So far, only the source code is available.
Schematics and stuff are work in progress. Please stand by.

A finished device ("Hensel Model-01") has the following features:

	- High-precision Master Clock
		- MIDI Clock Output
		- CV/Gate Output (0V/5V)
	- Micro-adjustable Tempo (endless rotary knob, two speeds)
	- Tap-Tempo Function (dropout protection, auto-smoothing)
	- [Supports output of 1, 12, 24 and 48 clocks per quarter note]
	- Control MIDI Equipment (start, continue, stop)
	- Split-Mode - Synchronize to external MIDI Clock
		- Quickly toggle between both speeds (internal/external)
		- Control pass-through of control messages to MIDI-OUT
		- [Skew-Clock clock output manually (Swing)]
	- Settings can be saved in non-volatile memory
	- Jumbo Tri-Color LEDs for Tempo and Status display
	- MIDI-Thru (latency-free)
	- [Panic Function (All-Notes-Off)]
	- Tolerant power input (7-12V DC) and hardware power switch
		- Low-power design, runs on batteries and USB-power (adapter required)
		- Energy-efficient power supply included

Pre-built devices and part kits are available upon request!
*** Tom Hensel <tom _ AT _ interpol8.net> ***

Always keep in sync. Regrds,
Tom