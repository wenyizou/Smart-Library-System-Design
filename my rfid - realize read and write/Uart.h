/*
 * Uart.h
 *
 *  Created on: 2014-5-5
 *      Author: l
 */

#ifndef UART_H_
#define UART_H_


/*
 * Uart.h
 *A Uart setting and using header file for msp430G2553, USCI interface
 *  Created on: 2014-2-13
 *      Author: Wenyi Zou
 */
#include  <msp430g2553.h>

#define UCAxCTL0   UCA0CTL0
#define UCAxCTL1   UCA0CTL1         // USCI control register

#define UCAxBR0    UCA0BR0
#define UCAxBR1    UCA0BR1          // USCI baudrate register

#define UCAxMCTL   UCA0MCTL         // USCI baudrate modulation register
#define UCAxSTAT   UCA0STAT         // USCI status register


#define UCAxRXBUF  UCA0RXBUF
#define UCAxTXBUF  UCA0TXBUF        // USCI receive and transmit register

#define UCAxABCTL  UCA0ABCTL        // USCI auto baudrate control
#define UCAxIRTCTL UCA0IRTCTL       // /* USCI A0 IrDA Transmit Control */
#define UCAxIRRCTL UCA0IRRCTL       ///* USCI A0 IrDA receive Control */     these three actually not used this file

#define UCAxIE      IE2
#define UCAxTXIE   UCA0TXIE
#define UCAxRXIE   UCA0RXIE         // USCI interrupt enable

#define UCAxIFG     IFG2
#define UCAxTXIFG  UCA0TXIFG
#define UCAxRXIFG  UCA0RXIFG        // USCI interrupt flag

#define UARTON  P3SEL |= 0X30           // P3.4,5 = USART0 TXD/RXD


//////////////////////////////////////////////////
// below are function definition
void Uart_Init(double BaudRate, char parity, char LMSBMode, int bitMode, int stopbitMode);     // Uart Initiation
double Uart_setBaudClock(double BaudRate);      // according baudrate to set the clock USCI use
void Uart_setBaudRate(double BaudRate, double Clk);      // according to the baudrate and clock, set control register
void Uart_setParity(char parity);              // set Parity bit
void Uart_setLMSBMode(char LMSBMode);           // set LSB or MSB mode
void Uart_setbitMode(int bitMode);                 // set bit mode
void Uart_setstopbitMode(int stopbitMode);         // set stop bit mode

void Uart_sendchar(char zifu);                // send char
void Uart_sendstr(char *str);                 // send string
char Uart_readchar() ;                        // reserve function, really don't know how to use, when to read

#endif /* UART_H_ */



