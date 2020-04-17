# i2c-keyboard-matrix
I2C driver for a keyboard matrix of up to 4x4 keys, using an ATmega microcontroller

4x4 keyboard matrix are used by many hacks for manual input. However, they require up to 8 I/O channels and continuously running software routines on the host system. This project eliminates the burden from the host system by encapsulating the management of the keyboard matrix to a dedicated ATmega328. The host system communicates with the keyboard matrix via a I2C interface. By the way we have added up to 4 LEDs available via I2C so that the host can set or kill light depending on any action. 
