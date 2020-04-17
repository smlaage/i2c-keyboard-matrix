/*****************************************************************************
* twi_slave.c
* 
* TWI Slave Interface for AVR ATmega
*
* File              : TWI_Slave.c
* Based on:			: ATMEL, Revision: 2475, $Author: mlarsson
* Updated by        : Stephan Laage-Witt, for KS0108 I2C Display
*
* Supported devices : ATmega168, ATmega328
*
* Description       : TWI Slave interface by interrupt. Receives messages from TWI-Master, stores 
* messages in a ring buffer and enables messages retrieval.
* Functions: TWI_slave_init(TWI_address) - initiates the slave and starts the transceiver
*			 TWI_send_data(uint8_t *msg, uint8_t msg_size) - sends data
*			 uint8_t TWI_check_receive_buffer(void) - tests whether data is available in the buffer
*			 uint8_t TWI_fetch_byte(void) - reads a received byte from the buffer
* 
*/

#include <avr/io.h>
#include <avr/interrupt.h>
#include "TWI_slave.h"

// Global variables
volatile uint8_t TWI_busy = 0;										// Flag for "TWI interface is busy"
uint8_t TWI_send_buf[4];											// Data buffer for sending via TWI 

// Local variables
static volatile uint8_t TWI_receive_buf[TWI_RECEIVE_BUFFER_SIZE];	// Data buffer for receiving via TWI 
static uint8_t TWI_msg_size;										// Number of bytes to be sent
static volatile uint16_t TWI_receive_pnt;							// Pointer for receiving data 
static uint16_t TWI_fetch_pnt;										// Pointer for reading data by the main program
static uint8_t TWI_send_pnt;										// Pointer for sending data
static uint8_t buffer_full = 0;										// Flag to indicate that the buffer is full

/* TWI_slave_init --------------------------------------------------------------------------------------------------
* Initialize the TWI interface
* Input: TWI Slave Address
*/
void TWI_slave_init(uint8_t TWI_Slave_Address )
{
	TWAR =	TWI_Slave_Address << 1;                 // Set TWI slave address; accept TWI General Calls.
	TWCR =	(1<<TWEN)|								// Enable TWI interface 
			(1<<TWIE)|(1<<TWINT)|					// Enable interrupt and clear the flag.
			(1<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|		// Prepare to ACK next time the slave is addressed.
			(0<<TWWC);
	TWI_receive_pnt = 0;							// Reset all buffer pointers	
	TWI_send_pnt = 0;
	TWI_fetch_pnt = 0;
	TWI_busy = 0;
}    
    
/*TWI_send_data ------------------------------------------------------------
* Initiates a TWI transmission of the bytes in TWI_send_buffer 
* Input: msg_size - number of bytes in message buffer (most not exceed buffer size)
*/
void TWI_send_data(uint8_t msg_size)
{
	while (TWI_busy);								// wait until TWI interface is ready  
	TWI_msg_size = msg_size;						// number of bytes in message buffer
	
	/*for (uint8_t i = 0; i < msg_size; ++i) {		// copy data into TWI send buffer
		TWI_send_buf[i] = msg[i];
	};*/
	
	TWI_send_pnt = 0;
	TWCR =	(1<<TWEN) |								// TWI interface enabled.
		    (1<<TWIE) | (1<<TWINT) |				// Enable TWI interrupt and clear the flag.
			(1<<TWEA) | (0<<TWSTA) |				// Prepare to ACK next time the slave is addressed.
			(0<<TWSTO) | (0<<TWWC);                            
	TWI_busy = 1;									// TWI interface ready
}

/* TWI_fetch_byte -----------------------------------------------------------
* Reads and returns the next byte in the receive buffer 
* This functions waits for the next byte to arrive in case the buffer is empty 
*/
uint8_t TWI_fetch_byte(void)
{
	uint8_t return_value;
	
	while (TWI_fetch_pnt == TWI_receive_pnt);			// wait for next byte to arrive
	return_value = TWI_receive_buf[TWI_fetch_pnt];		// get next byte
	++TWI_fetch_pnt;
	buffer_full = 0;									// reset buffer full flag
	if (TWI_fetch_pnt == TWI_RECEIVE_BUFFER_SIZE)		// increment TWI_fetch_pointer
		TWI_fetch_pnt = 0;
	return(return_value);								// return next byte
}

/* TWI_check_receive_buffer -------------------------------------------------
* Returns 0 in case the buffer is empty
* and 1 in case there are one or more new bytes available
*/
uint8_t TWI_check_receive_buffer(void)
{
	if (TWI_fetch_pnt == TWI_receive_pnt) return(0);
	else return(1);
}

/************ Interrupt Handlers *********************************************
* Interrupt service routine for the TWR slave interface
* The interrupt is triggered by external I2C requests addressing the interface 
* The function initiates actions depending on the status of the TWSR register
*/
ISR(TWI_vect) {
	extern uint8_t give_signal;
	
	switch (TWSR) {
		case TWI_STX_ADR_ACK:								// Status: slave has been addressed in Read mode; ACK has been returned
		case TWI_STX_DATA_ACK:								// Byte in TWDR has been sent, ACK (= more data expected) has been received
			TWDR = TWI_send_buf[TWI_send_pnt++];			// Increment buffer pointer and ...
			if (TWI_send_pnt == TWI_SEND_BUFFER_SIZE)		// ... avoid buffer overflow
				TWI_send_pnt = 0;
			TWCR =	(1<<TWEN) |                         
					(1<<TWIE) | (1<<TWINT) |            
					(1<<TWEA) | (0<<TWSTA) |    
					(0<<TWSTO) | (0<<TWWC);                          
			TWI_busy = 1;									// maintain busy flag
			break;

		case TWI_STX_DATA_NACK:								// Status: byte has been sent, NACK (= end of transmission) was received
			TWCR =	(1<<TWEN) |								// Enable TWI-interface and release TWI pins
					(1<<TWIE) | (1<<TWINT) |				// Keep interrupt enabled and clear the flag
					(1<<TWEA) | (0<<TWSTA) |				// Answer on next address match
					(0<<TWSTO) | (0<<TWWC);                          
			TWI_busy = 0;									// clear bus flag, transmission completed
			break;     

		case TWI_SRX_GEN_ACK:								// Status: general call has been received; ACK has been returned
		case TWI_SRX_ADR_ACK:								// Status: slave has been addressed in Write mode; ACK has been returned
			TWCR =	(1<<TWEN) |							
					(1<<TWIE) | (1<<TWINT) |			
					(1<<TWEA) | (0<<TWSTA) |		 	
					(0<<TWSTO)| (0<<TWWC);  
			TWI_busy = 1;									// maintain busy flag
			break;
			
		case TWI_SRX_ADR_DATA_ACK:							// Status: general call has been received; data has been received; ACK has been returned
		case TWI_SRX_GEN_DATA_ACK:							// Status: slave has been addressed; data has been received; ACK has been returned
			if (buffer_full > 0) {							// Buffer full?
				give_signal = 1;							// If yes, then ignore received byte and give sound signal
			} else {
				TWI_receive_buf[TWI_receive_pnt++] = TWDR;			// Otherwise, copy received byte to buffer; increment buffer pointer
				if (TWI_receive_pnt == TWI_RECEIVE_BUFFER_SIZE)		// increment TWI_add_pointer
					TWI_receive_pnt = 0;
				if (TWI_receive_pnt == TWI_fetch_pnt)		// check whether the buffer is full
					buffer_full = 1;
			}
			TWCR =	(1<<TWEN) |								// Status change: TWI Interface enabled
					(1<<TWIE) | (1<<TWINT) |				// Enable TWI interrupt and clear the flag to send byte
					(1<<TWEA) | (0<<TWSTA) |				// Send ACK after next reception
					(0<<TWSTO)| (0<<TWWC);              
			TWI_busy = 1;
			break;
			
		case TWI_SRX_STOP_RESTART:							// Status: STOP or repeated START has been received while still addressed as Slave    
			TWCR =	(1<<TWEN) |								 
					(1<<TWIE) | (1<<TWINT)|					
					(1<<TWEA) | (0<<TWSTA)|					
					(0<<TWSTO) | (0<<TWWC);                                 
			TWI_busy = 0;									
			break;
		           
		case TWI_SRX_ADR_DATA_NACK:							// Status: slave has been addressed in Write mode; data has been received; NOT ACK has been returned
		case TWI_SRX_GEN_DATA_NACK:							// Status: general call has been received; data has been received; NOT ACK has been returned
	    case TWI_STX_DATA_ACK_LAST_BYTE:					// Status: last data byte in TWDR has been transmitted (TWEA = 0); ACK has been received
		case TWI_BUS_ERROR:									// Status: bus error due to an illegal START or STOP condition
			TWCR = (1<<TWSTO)|(1<<TWINT);					// Recover from TWI_BUS_ERROR (this will release the SDA and SCL pins thus enabling other devices to use the bus)
			break;
		
		default:											// Status: unknown. Waiting for a new address match
			TWCR =	(1<<TWEN) |								
					(1<<TWIE) | (1<<TWINT)|					
					(1<<TWEA) | (0<<TWSTA)|					
					(0<<TWSTO) | (0<<TWWC);                                 
			TWI_busy = 0;									
	}
}
