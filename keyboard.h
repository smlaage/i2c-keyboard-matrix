// keyboard.h
// SLW - April 2020

#ifndef __KEYBOARD_H__
#define __KEYBOARD_H__

#define PIN_ROW_0	0
#define PIN_ROW_1	1
#define PIN_ROW_2	2
#define PIN_ROW_3	3
#define PIN_COL_0	4
#define PIN_COL_1	5
#define PIN_COL_2	6
#define PIN_COL_3	7

#define KEYBOARD_BUFFER_SIZE 32

// function prototypes

uint8_t check_keyboard(void);
uint8_t read_keyboard(void);
uint8_t calc_bytes_in_buffer(void);
void clear_buffer(void);

#endif