#ifndef UART_16550_h
#define UART_16550_h

#include <FreeRTOSConfig.h>
#include <FreeRTOS.h>

#define UART0 0
#define UART1 1

#define UART_16550_clk 50000000

#define UART_PARITY_NONE 0
#define UART_PARITY_EVEN 1
#define UART_PARITY_ODD  2

//************* Initialization functions *************************/
void UART_16550_init();

// Configure the given UART with the given parameters
void UART_16550_configure(int UART,int baud,int parity,int bits,int stop_bits);

/************* Functions that tasks can use *************************/

// Lock the given UART transmitter, so that no other task can write
BaseType_t UART_16550_tx_lock(int UART,
                              TickType_t xTicksToWait);

// Unlock the given UART transmitter so that other tasks can write
void UART_16550_tx_unlock(int UART);

// Try to write a character to the UART
BaseType_t UART_16550_put_char(int UART, char c, TickType_t xTicksToWait);

// Try to write a string to the UART
BaseType_t UART_16550_write_string(int UART, char *s, TickType_t xTicksToWait);

// Lock the given UART receiver, so that no other task can read
BaseType_t UART_16550_rx_lock(int UART, TickType_t xTicksToWait);

// Unlock the given UART receiver so that other tasks can read
void UART_16550_rx_unlock(int UART);

// Try to read a character from the UART
BaseType_t UART_16550_get_char(int UART, char *ch, TickType_t xTicksToWait);

// Try to read a string from the UART
BaseType_t UART_16550_read_string(int UART, char *s, int maxLength, TickType_t xTicksToWait);

#endif
