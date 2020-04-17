/* KeyBoard_I2C.cpp 
 * Arduino functions for the I2C Keyboard
 *
 * SLW - April 2020 
*/

#include "KeyBoard_I2C.h"

/* begin ------------------------------------------------------------------------------------------
* Sets the i2c address to default.
*/
KeyBoard_I2C::KeyBoard_I2C(void) {
	i2c_address = 0x5a;
}

/* begin ------------------------------------------------------------------------------------------
* Sets the i2c address to a specified value.
*/
KeyBoard_I2C::KeyBoard_I2C(int address) {
	i2c_address = address;
}

/* check_kbd --------------------------------------------------------------------------------------
 * Returns the number of bytes in the keyboard buffer. 
*/
int KeyBoard_I2C::check(void) {
  int result;
  
  Wire.beginTransmission(i2c_address);
  Wire.write(byte(0x02));
  Wire.endTransmission(i2c_address);
  Wire.requestFrom(i2c_address, 1);
  while (Wire.available() < 1);
  result = Wire.read();
  return result;
}

/* read_kbd_buffer -------------------------------------------------------------------------------
 * Reads a key from the keyboard buffer. Returns 0 if the buffer is empty. 
*/
char KeyBoard_I2C::read(void) {
  int result;
  
  Wire.beginTransmission(i2c_address);
  Wire.write(byte(0x00));
  Wire.endTransmission(i2c_address);
  Wire.requestFrom(i2c_address, 1);
  while (Wire.available() < 1);
  result = Wire.read();
  return result;
}

/* read_kbd --------------------------------------------------------------------------------------
 * Reads a currently pressed key. Returns 0 if no key is pressed at this time.
 * This function clears the keyboard buffer implicitly.
*/
char KeyBoard_I2C::read_direct(void) {
  int result;
  
  Wire.beginTransmission(i2c_address);
  Wire.write(byte(0x01));
  Wire.endTransmission(i2c_address);
  Wire.requestFrom(i2c_address, 1);
  while (Wire.available() < 1);
  result = Wire.read();
  return result;
}

/* set_kbd_led -------------------------------------------------------------------------------------
 *  Sets or clears one of the keyboard leds.
 *  uint8_t led: identifier of the led, 0 ... 3
 *  bool led_status: false = off, true = on;
 */
void KeyBoard_I2C::set_led(uint8_t led, bool led_status) {
  if ((led >= 0) && (led < 4)) {
    Wire.beginTransmission(i2c_address);
    Wire.write(byte(10 + led));
    if (led_status)
      Wire.write(byte(1));
    else
      Wire.write(byte(0));
    Wire.endTransmission();
  }
}    

/* kbd_set_sound -------------------------------------------------------------------------------------
 */
void KeyBoard_I2C::set_sound(bool onoff) {
  Wire.beginTransmission(i2c_address);
  Wire.write(byte(20));
  Wire.write(byte(onoff));
  Wire.endTransmission();
}    

/* kbd_set_repeat ------------------------------------------------------------------------------------
 */
void KeyBoard_I2C::set_repeat(bool onoff) {
  Wire.beginTransmission(i2c_address);
  Wire.write(byte(21));
  Wire.write(byte(onoff));
  Wire.endTransmission();
}   
