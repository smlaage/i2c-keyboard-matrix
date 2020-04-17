/*
 * I2C-Keyboard.c
 * 
 * I2C interface for a 4x4 keyboard matrix. Features:
 * - keyboard buffer of 32 bytes
 * - click sound for pressed keys
 * - buffer overflow generates a sound signal
 * - key repeat function, kicking in after approx. 1 s key pressed, generating 5 key strokes per second 
 * - in addition: controlling up to 4 LEDs via I2C interface 
 *
 * Created: April 2020
 * Author : Stephan Laage-Witt
 */ 

#define F_CPU 8000000L

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/eeprom.h>
#include "keyboard.h"
#include "twi_slave.h"

// pin definitions
#define PIN_SOUND	0
#define PIN_LED_0	0
#define PIN_LED_1	1
#define PIN_LED_2	2
#define PIN_LED_3	3

// global constants
#define I2C_ADDRESS 0x5A

// global variables
volatile uint8_t keyboard_buffer[KEYBOARD_BUFFER_SIZE];
volatile volatile uint8_t keyboard_buffer_wr_pnt = 0;
uint8_t keyboard_buffer_rd_pnt = 0;
volatile uint8_t keyboard_state = 0;
uint8_t sound_on;						// bool to switch sound on or off
uint8_t repeat_on;						// bool to switch auto-repeat on or off
uint8_t give_signal = 0;				// flag to trigger a sound signal

// EEPROM data ---------------------------------------------------------------------------------------
#define EE_BASE 0x10
uint8_t ee_sound_on EEMEM = EE_BASE;			// sound on or off
uint8_t ee_repeat_on EEMEM = EE_BASE + 1;	// auto-repeat on or off

/* send_keyboard_status() ---------------------------------------------------------------------------------
Sends the current keyboard status to TWI master.
*/
void send_keyboard_state(void) {
	extern uint8_t TWI_send_buf[];
	
	TWI_send_buf[0] = keyboard_state;
	TWI_send_data(1);
	clear_buffer();
}

/* send_key_from_buffer() ---------------------------------------------------------------------------------
Sends a key stroke from the buffer. Sends 0 in case the buffer is empty;
*/
void send_key_from_buffer(void) {
	extern uint8_t TWI_send_buf[];

	TWI_send_buf[0] = read_keyboard();
	TWI_send_data(1);	
}

/* send_bytes_in_buffer -----------------------------------------------------------------------------------
Sends the number of bytes currently in buffer, waiting for retrieval
*/
void send_bytes_in_buffer(void) {
	extern uint8_t TWI_send_buf[];

	TWI_send_buf[0] = calc_bytes_in_buffer();
	TWI_send_data(1);
}

//---------------------------------------------------------------------------------------------------------
int main(void)
{
	uint8_t command_byte;
	
	// define port direction
	DDRB |= (1 << PIN_SOUND);				// sound pin
	PORTB &= ~(1 << PIN_SOUND);				// switch off initially
	DDRD = 0b00001111;
	PORTD = 0b11111111;
	DDRC |= (1 << PIN_LED_0) | (1 << PIN_LED_1) | (1 << PIN_LED_2) | (1 << PIN_LED_3);		// set LED pins to output
	PORTC &= ~((1 << PIN_LED_0) | (1 << PIN_LED_1) | (1 << PIN_LED_2) | (1 << PIN_LED_3));	// set all LEDs off
	/*
	DDRD = ((1 << PIN_ROW_0) | (1 << PIN_ROW_1) | (1 << PIN_ROW_2) | (1 << PIN_ROW_3));		// rows are output
	DDRD &= ~((1 << PIN_COL_0) | (1 << PIN_COL_1) | (1 << PIN_COL_2) | (1 || PIN_COL_3));	// columns are input
	PORTD |= ((1 << PIN_COL_0) | (1 << PIN_COL_1) | (1 << PIN_COL_2) | (1 || PIN_COL_3));	// activate pull-up resistors for columns 
	*/
	
	// read sound and repeat status from eeprom
	sound_on = eeprom_read_byte(&ee_sound_on);
	if (sound_on > 1) {
		sound_on = 1;											// default value is "on"
		eeprom_write_byte(&ee_sound_on, sound_on);
	};
	repeat_on = eeprom_read_byte(&ee_repeat_on);
	if (repeat_on > 1) {
		repeat_on = 0;											// default value is "off"
		eeprom_write_byte(&ee_repeat_on, repeat_on);
	};

	// Configure timer1 for interrupts at 20 Hz (8000000 / 64 / 1667 -> 75 Hz)
	OCR1A = 1667-1;
	TCCR1B |= (1<<WGM12)| (0<<CS02) | (1<<CS01) |(1<<CS00);  		// CPU speed / 64, CTC1 = Clear Timer on Compare
	TIMSK1 |= (1 << OCIE1A);

	// start TWI interface
	TWI_slave_init(I2C_ADDRESS);
	
	// prepare sleep mode
	set_sleep_mode(SLEEP_MODE_IDLE);

	// say hello: flash LEDs
	PORTC |= (1 << PIN_LED_0);
	_delay_ms(100);
	PORTC &= ~(1 << PIN_LED_0);
	PORTC |= (1 << PIN_LED_1);
	_delay_ms(100);
	PORTC &= ~(1 << PIN_LED_1);
	PORTC |= (1 << PIN_LED_2);
	_delay_ms(100);
	PORTC &= ~(1 << PIN_LED_2);
	PORTC |= (1 << PIN_LED_3);
	_delay_ms(100);
	PORTC &= ~(1 << PIN_LED_3);
	
	// off we go ....
	sei();
	
	while (1) {
		if (TWI_check_receive_buffer() > 0) {
			command_byte = TWI_fetch_byte();
			switch (command_byte) {
				case 0:								// send keys from keyboard buffer
					send_key_from_buffer();
					break;
				case 1:								// send current status of the keyboard (no buffering)
					send_keyboard_state();
					break;
				case 2:
					send_bytes_in_buffer();			// send the number of bytes currently in the buffer
					break;
				case 10:							// set led 0
					if (TWI_fetch_byte() > 0)
						PORTC |= (1 << PIN_LED_0);
					else
						PORTC &= ~(1 << PIN_LED_0);
					break;
				case 11:							// set led 1
					if (TWI_fetch_byte() > 0)
						PORTC |= (1 << PIN_LED_1);
					else
						PORTC &= ~(1 << PIN_LED_1);
					break;
				case 12:							// set led 2
					if (TWI_fetch_byte() > 0)
						PORTC |= (1 << PIN_LED_2);
					else
						PORTC &= ~(1 << PIN_LED_2);
					break;
				case 13:							// set led 3
					if (TWI_fetch_byte() > 0)
						PORTC |= (1 << PIN_LED_3);
					else
						PORTC &= ~(1 << PIN_LED_3);
					break;
				case 20:							// set sound status
					if (TWI_fetch_byte() > 0) 
						sound_on = 1;
					else
						sound_on = 0;
					eeprom_write_byte(&ee_sound_on, sound_on);
					break;
				case 21:							// set auto repeat status
					if (TWI_fetch_byte() > 0)
						repeat_on = 1;
					else
						repeat_on = 0;
					eeprom_write_byte(&ee_repeat_on, repeat_on);
					break;
				default:
					;	
			};		

		};
		
		// in case the keyboard buffer is full, give sound signal and delete last key stroke
		if (calc_bytes_in_buffer() >= KEYBOARD_BUFFER_SIZE - 1) {		
			if (sound_on) 
				give_signal = 1;
			if (keyboard_buffer_wr_pnt == 0)							// ignore last keystroke ...
				keyboard_buffer_wr_pnt = KEYBOARD_BUFFER_SIZE - 1;		// ... by decreasing keyboard write pointer by 1
			else
				--keyboard_buffer_wr_pnt;
		};
		
		if (give_signal > 0) {											// in case a sound signal is requested:
			give_signal = 0;										// reset flag 
			for (int i = 0; i < 70; ++i) {							// give sound
				PORTB |= (1 << PIN_SOUND);
				_delay_ms(1);
				PORTB &= ~(1 << PIN_SOUND);
				_delay_ms(1);
			};
		}
			
		sleep_mode();												// that's it ... good night
	};
}

/* Interrupt Service Handler ---------------------------------------------------------------------*/
/* Runs 40 times per second 														              */
ISR(TIMER1_COMPA_vect) {
	static uint8_t old_keyboard_state = 0;
	static uint8_t key_count = 0;					// manages the decoding status
	
	keyboard_state = check_keyboard();
	if (keyboard_state == 0) {
		key_count = 0;
	} else {
		switch (key_count) {
			case 0:									// key was pressed the first time
				old_keyboard_state = keyboard_state;
				++key_count;
				break;
			case 3:									// capture key stroke to the buffer
				if (old_keyboard_state == keyboard_state) {
					keyboard_buffer[keyboard_buffer_wr_pnt] = keyboard_state;
					if (++keyboard_buffer_wr_pnt == KEYBOARD_BUFFER_SIZE)
						keyboard_buffer_wr_pnt = 0;
					++key_count;
					if (sound_on) {
						for (int i= 0; i < 10; ++i) {
							PORTB |= (1 << PIN_SOUND);		// give click sound
							_delay_us(500);
							PORTB &= ~(1 << PIN_SOUND);
							_delay_us(500);
						}
					}
				} else
					key_count = 0;
				break;
			case 75:								// key repeat starts after 1 seconds
				if (repeat_on) {					// only in case auto-repeat is active
					if (old_keyboard_state == keyboard_state) {
						keyboard_buffer[keyboard_buffer_wr_pnt] = keyboard_state;
						if (++keyboard_buffer_wr_pnt == KEYBOARD_BUFFER_SIZE)
							keyboard_buffer_wr_pnt = 0;
						key_count = 60;					// set counter back by 0.2 seconds, 5 strokes per seconds 
						if (sound_on) {
							for (int i= 0; i < 10; ++i) {
								PORTB |= (1 << PIN_SOUND);		// give click sound
								_delay_us(500);
								PORTB &= ~(1 << PIN_SOUND);
								_delay_us(500);
							}
						}
					} else {
						key_count = 0;
					}
				}
				break;								
			default:
				if (old_keyboard_state == keyboard_state) 
					++key_count;
				else 
					key_count = 0;
				break;
		};
	};
}

