# Emu8080
Intel 8080 emulator

In its current state, a test ROM is loaded into memory and executed. Once the program counter is set to 0x0, the CPU halts execution and exits the run loop. Custom behavior can easily be implemented.

# CPU
The CPU was tested via this program: https://github.com/ddelnano/8080-emulator/tree/master
The CPU is reported fully operational, though there still may be some bugs in certain instruction implementations.

The CPU state can be read and written, if save state functionality is desired.

# Interrupts
The emulator has programmable interrupt callbacks. By default, the CP/M interrupts 0x0 and 0x5 are implemented; these are simply used to end execution and output string values respectively. If you need to change the callback functionality, it can easily be done using the methods provided in the Emulator class.

# Graphics
As graphics implementations are unique to the operating system, this project does not contain any graphical support, though it would be very straight-forward to implement.

# Instruction encoding/decoding
This project comes with a built-in instruction encoder and decoder via Encode.h.

# Logging
Logging is disabled by default. If you wish to enable verbose logging, set the DEBUG definition in Emulator.cpp to 1. Every operation will then be printed to stdout.
