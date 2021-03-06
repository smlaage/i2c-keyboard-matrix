# i2c-keyboard-matrix
I2C driver for a keyboard matrix of up to 4x4 keys, using an ATmega microcontroller. 

SLW - April 2020

A 4x4 keyboard matrix is a common component of many hacks in order to captured manual input. However, such a matrix requires up to 8 I/O channels and continuously running software routines on the host system. This project eliminates the burden from the host system by encapsulating the management of the keyboard matrix to a dedicated ATmega328. The host system communicates with the keyboard matrix via an I2C interface. By the way we have added up to 4 LEDs available via I2C so that the host can set or kill light depending on any action.

Any system capable of operating as I2C master may serve as a host. Typically, this would be Arduino, Raspberry Pi or any other device using a microcontroller. 

--------------------------------------------------------------------------------------------------------------------
Files in this repository:
- Readme.md: this file
- I2C-Keyboard-Atmel-Project.zip: Atmel project files (including readily compiled HEX and ELF) for Atmel Studio 7
- Arduino Circuit Diagram and fotos of a built with hole grid board: 
  - I2C-Keyboard-Circuit.jpg
  - I2C-Keyboard-foto_0.jpg
  - I2C-Keyboard-foto_1.jpg
  - I2C-Keyboard-foto_1.jpg
- Arduino library files, example for how to use the keyboard with an Arduino system:
  - KeyBoard_I2C.cpp
  - KeyBoard_I2C.h

--------------------------------------------------------------------------------------------------------------------
Target platform: ATmega328, running as I2C slave. The ATmega uses the internal clock at 8 MHz. The software was compiled with Atmel Studio 7. 
ATmega Pin assignments: 
- PD0 ... PD3: Keyboard columns
- PD4 ... PD7: Keyboard rows
- PC0 ... PC3: up to 4 LEDs
- PB0: piezo buzzer
- PC4: I2C SDA
- PC5: I2C SCL

--------------------------------------------------------------------------------------------------------------------
I2C Interface:
- Default I2C address: 0x5f, defined in “main.c”, can be changed to any valid I2C address.
- The I2C clock speed should be in the range of 100 to 400 kHz.

--------------------------------------------------------------------------------------------------------------------
I2C Commands:
- 0: Retrieves a key from the buffer. The keyboard sends the ASCII character of the pressed key from the buffer. The keyboard sends the value 0 in case there is no pending key pressed.
- 1:  Retrieves the current key from the keyboard without buffering. This function implicitly clears the buffer.
- 2:  Retrieves the number of bytes (= keys pressed) currently in the buffer
- 10: Requires one byte as argument. Sets LED 0 to on (argument > 0) or off (argument = 0)
- 11: Requires one byte as argument. Sets LED 1
- 12: Requires one byte as argument. Sets LED 2
- 13: Requires one byte as argument. Sets LED 3
- 20: Requires one byte as argument. Sets sound to on (argument > 0) or off (argument = 0)
- 21: Requires one byte as argument. Sets auto repeat to on (argument > 0) or off (argument = 0)

The setting for sound and auto-repeat are captured in EEPROM memory and will be kept beyond powering off.

The keyboard layout is captured in the file "keyboard_layout.h", can be adjusted as needed.

