What works:

Interrupt handlers for GPIO command reception and SSI packet transmission (for DCC output), NVIC priority is higher for the SSI handler
Bit string (C strings of 1 and 0s) can be converted to SSI-compliant 16-bit packed frames from inside the program
State machine for transmitting packets in an acceptable order with the following features:
	Configurable timeout for engines: the minimum spacing between packet transmissions for either engine, counted individually
	Change detection for turnouts: turnout commands are only transmitted when the turnout values change
Reset packets are transmitted on startup
Engines can be issued speed commands from the DTE based on the interrupt handlers working together, turnouts can be issued commands as well (both work in real time)

What is broken:

Engines only respond to address 0
Direction cannot be controlled for engines
Turnouts do not behave properly, the command strings being used are incorrect


Things to add (beyond fixing broken parts):

Timeouts for the turnouts
Convert the packet strings into SSI frames at compile time, or in a separate script
