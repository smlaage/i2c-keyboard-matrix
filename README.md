# i2c-keyboard-matrix
I2C driver for a keyboard matrix of up to 4x4 keys, using an ATmega microcontroller

4x4 keyboard matrix are used by many hacks for manual input. However, they require up to 8 I/O channels and continuously running software routines on the host system. This project eliminates the burden from the host system by encapsulating the management of the keyboard matrix to a dedicated ATmega328. The host system communicates with the keyboard matrix via a I2C interface. By the way we have added up to 4 LEDs available via I2C so that the host can set or kill light depending on any action.

Target platform: ATmega328, used as I2C slave

I2C Interface:
Default I2C address: 0x5f, defined in “main.c”, can be changed to any valid I2C address.

I2C Commands:
0: Retrieves a key from the buffer. The keyboard sends the ASCII character of the pressed key from the buffer. The keyboard sends the value 0 in case there is no pending key pressed.
1:  Retrieves the current key from the keyboard without buffering. This function implicitly clears the buffer.
2:  Retrieves the number of bytes (= keys pressed) currently in the buffer
10: Requires one byte as argument. Sets LED 0 to on (argument > 0) or off (argument = 0)
11: Requires one byte as argument. Sets LED 1
12: Requires one byte as argument. Sets LED 2
13: Requires one byte as argument. Sets LED 3
20: Requires one byte as argument. Sets sound to on (argument > 0) or off (argument = 0)
21: Requires one byte as argument. Sets auto repeat to on (argument > 0) or off (argument = 0)

The setting for sound and auto-repeat are captured in EEPROM memory and will be kept beyond powering off.

The keyboard layout is captured in the file "keyboard_layout.h", can be adjusted as needed.

