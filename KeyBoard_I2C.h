/* KeyBoard_I2C.h 
 * Arduino functions for the I2C Keyboard
 *
 * SLW - April 2020 
*/

#ifndef __KEYBOARD_I2C__
#define __KEYBOARD_I2C__

#include <Wire.h>
#include <Arduino.h>

class KeyBoard_I2C {
	
	private:
		int i2c_address;
	
	public: 
		KeyBoard_I2C(void);
		KeyBoard_I2C(int address);
		int check(void);
		char read(void);
		char read_direct(void);
		void set_led(uint8_t led, bool led_status);
		void set_sound(bool onoff);
		void set_repeat(bool onoff);
};

#endif
