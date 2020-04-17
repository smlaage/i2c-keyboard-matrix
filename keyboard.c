// keyboard.c
// SLW - April 2020

#include <avr/io.h>
#include "keyboard.h"
#include "keyboard_layout.h"

/* clear_buffer -----------------------------------------------------------------------------------------------------------
Resets the keyboard buffer to the initial state, no content.
*/
void clear_buffer(void) {
	extern uint8_t keyboard_buffer_rd_pnt;
	extern uint8_t keyboard_buffer_wr_pnt;
	
	keyboard_buffer_rd_pnt = keyboard_buffer_wr_pnt;
}

/* check_keyboard() -------------------------------------------------------------------------------------------------------
Reads the current status of the keyboard. Returns 0 if no key is pressed or any value between 1 and 16 if a key is pressed
uint8_t read_keyboard(void) {
*/
uint8_t check_keyboard(void) {
	uint8_t keyboard_bits;

	PORTD |= ((1 << PIN_ROW_0) | (1 << PIN_ROW_1) | (1 << PIN_ROW_2) | (1 << PIN_ROW_3));

	// row 0 --------------------
	PORTD &= ~(1 << PIN_ROW_0);								// pull down row 0
	asm volatile("\tnop\n"::);								// wait 1 NOP
	keyboard_bits = ~PIND & ((1 << PIN_COL_0) | (1 << PIN_COL_1) | (1 << PIN_COL_2) | (1 << PIN_COL_3));
	PORTD |= (1 << PIN_ROW_0);								// reset row 0
	if (keyboard_bits > 0) {
		switch (keyboard_bits) {
			case (1 << PIN_COL_0):
				return(char_table[1]);
			case (1 << PIN_COL_1):
				return(char_table[5]);
			case (1 << PIN_COL_2):
				return(char_table[9]);
			case (1 << PIN_COL_3):
				return(char_table[13]);
			default:
				return(0);
		};
	};

	// row 1 --------------------
	PORTD &= ~(1 << PIN_ROW_1);								// pull down row 0
	asm volatile("\tnop\n"::);								// wait 1 NOP
	keyboard_bits = ~PIND & ((1 << PIN_COL_0) | (1 << PIN_COL_1) | (1 << PIN_COL_2) | (1 << PIN_COL_3));
	PORTD |= (1 << PIN_ROW_1);								// reset row 0
	if (keyboard_bits > 0) {
		switch (keyboard_bits) {
			case (1 << PIN_COL_0):
				return(char_table[2]);
			case (1 << PIN_COL_1):
				return(char_table[6]);
			case (1 << PIN_COL_2):
				return(char_table[10]);
			case (1 << PIN_COL_3):
				return(char_table[14]);
			default:
				return(0);
		};
	};
	
	// row 2 --------------------
	PORTD &= ~(1 << PIN_ROW_2);								// pull down row 0
	asm volatile("\tnop\n"::);								// wait 1 NOP
	keyboard_bits = ~PIND & ((1 << PIN_COL_0) | (1 << PIN_COL_1) | (1 << PIN_COL_2) | (1 << PIN_COL_3));
	PORTD |= (1 << PIN_ROW_2);								// reset row 0
	if (keyboard_bits > 0) {
		switch (keyboard_bits) {
			case (1 << PIN_COL_0):
				return(char_table[3]);
			case (1 << PIN_COL_1):
				return(char_table[7]);
			case (1 << PIN_COL_2):
				return(char_table[11]);
			case (1 << PIN_COL_3):
				return(char_table[15]);
			default:
				return(0);
		};
	};

	// row 3 --------------------
	PORTD &= ~(1 << PIN_ROW_3);								// pull down row 0
	asm volatile("\tnop\n"::);								// wait 1 NOP
	keyboard_bits = ~PIND & ((1 << PIN_COL_0) | (1 << PIN_COL_1) | (1 << PIN_COL_2) | (1 << PIN_COL_3));
	PORTD |= (1 << PIN_ROW_3);								// reset row 0
	if (keyboard_bits > 0) {
		switch (keyboard_bits) {
			case (1 << PIN_COL_0):
				return(char_table[4]);
			case (1 << PIN_COL_1):
				return(char_table[8]);
			case (1 << PIN_COL_2):
				return(char_table[12]);
			case (1 << PIN_COL_3):
				return(char_table[16]);
			default:
				return(0);
		};
	};
	
	return(0);												// couldn't detect any key
};

/*----------------------------------------------------------------------------------------
* Reads one byte from keyboard buffer.
* Returns: 0 if buffer is empty, otherwise code of pressed key
*/
uint8_t read_keyboard(void) {
	extern uint8_t keyboard_buffer_wr_pnt, keyboard_buffer_rd_pnt;
	extern uint8_t keyboard_buffer[];
	uint8_t keyboard_value;
	
	if (keyboard_buffer_wr_pnt == keyboard_buffer_rd_pnt) 
		return(0);
	
	keyboard_value = keyboard_buffer[keyboard_buffer_rd_pnt];
	if (++keyboard_buffer_rd_pnt == KEYBOARD_BUFFER_SIZE)
		keyboard_buffer_rd_pnt = 0;
	return(keyboard_value);
}

/*----------------------------------------------------------------------------------------
* Calculates the number of bytes captured by the buffer.
*/

uint8_t calc_bytes_in_buffer(void) {
	extern uint8_t keyboard_buffer_wr_pnt, keyboard_buffer_rd_pnt;
	
	if (keyboard_buffer_wr_pnt >= keyboard_buffer_rd_pnt)
		return(keyboard_buffer_wr_pnt - keyboard_buffer_rd_pnt);
	else
		return(keyboard_buffer_wr_pnt + KEYBOARD_BUFFER_SIZE - keyboard_buffer_rd_pnt);	 
}
