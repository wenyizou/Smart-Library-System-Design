/*
 * spi.c
 *
 *  Created on: 2014-4-7
 *      Author: wenyi
 *
 *  function: MSP430G2553 SPI function according to TRF7960A
 */

#include "msp430g2553.h"
#include "spi.h"
#include "mcu_trf_port.h"

//==============================================================
 char trash;
//==============================================================

void
SpiClkHigh(void)
{
	P1SEL  &= ~BIT5;
	P1SEL2 &= ~BIT5;        // back to basic IO
	P1DIR |= BIT5;
	P1OUT |= BIT5; 			//Make SCLK High
}

//==============================================================

void
SpiClkLow(void)
{
	P1OUT &= ~BIT5; 			//Make SCLK low
	P1SEL |= BIT5;
	P1SEL2 |= BIT5;         // back to USCI-SPI Clk


}

//==============================================================


void
SpiDirectCommand(char cmd)
{
	SLAVE_SELECT_LOW; 						// Start SPI Mode
	// set Address/Command Word Bit Distribution to command
	cmd = (0x80 | cmd);					// command
	cmd = (0x9f & cmd);					// command code  £¬ make single and write
	while (!(IFG2 & UCB0TXIFG))				// USCI_B0 TX buffer ready?
	{
	}
	UCB0TXBUF = cmd;             			// Previous data to TX, RX

	while(UCB0STAT & UCBUSY)
	{
	}

	trash = UCB0RXBUF;                     // for the first trash RXBUF value, trash is an extern value

	SpiClkHigh();

	SLAVE_SELECT_HIGH; 						//Stop SPI Mode

	SpiClkLow();                        // certain kind time sequence need for send command
}

//===============================================================

void
SpiRawWrite(char *data, int length)
{
	SLAVE_SELECT_LOW; 						//Start SPI Mode
	while(length > 0)
	{	while (!(IFG2 & UCB0TXIFG));		// USCI_B0 TX buffer ready?
		{
		}

		UCB0TXBUF = *data;				// Previous data to TX, RX

		while(UCB0STAT & UCBUSY)
		{
		}

	    trash=UCB0RXBUF;

		data++;
		length--;
	}
	while(UCB0STAT & UCBUSY)
	{
	}
	SLAVE_SELECT_HIGH; 						// Stop SPI Mode
}

//===============================================================

void
SpiRawWriteBlock(char *data, int length)    // especially for write block to tag-it
{
	SLAVE_SELECT_LOW; 						//Start SPI Mode
	while(length > 0)
	{	while (!(IFG2 & UCB0TXIFG));		// USCI_B0 TX buffer ready?
		{
		}

		UCB0TXBUF = *data;				// Previous data to TX, RX

		while(UCB0STAT & UCBUSY)
		{
		}

	    trash=UCB0RXBUF;

		data++;
		length--;
	}
	while(UCB0STAT & UCBUSY)
	{
	}

 	int temp;                       // delay 15 ms
  	int time2;
  	time2 = 10*98;
  	for(temp=0;temp<time2;temp++)
		P1OUT = P1OUT;           //  if you want you use self defined function, you have to at least do Port doing in your function one time

	SLAVE_SELECT_HIGH; 						// Stop SPI Mode
}


//================================================================

/*
 * example usage of SpiReadCont
 *
 * SpiReadCont(read_data, 0x0f, 8);
 *
 * it can read fifo 8 bytes to read_data
 *
 * the following next 5 function's usage are the same
 */

//================================================================
void
SpiReadCont(char *r_data, char addr, int length)            // store the value to the extern read_data section
{
	char *p_r_data = r_data;                // store the pointer position

	SLAVE_SELECT_LOW; 							//Start SPI Mode
	// Address/Command Word Bit Distribution
	addr = (0x60 | addr); 					// address, read, continuous
	addr = (0x7f & addr);						// register address
	while (!(IFG2 & UCB0TXIFG))					// USCI_B0 TX buffer ready?
	{
	}
	UCB0TXBUF = addr;  						// Previous data to TX, RX

	while(UCB0STAT & UCBUSY)
	{
	}

    trash = UCB0RXBUF;
	UCB0CTL0 &= ~UCCKPH;                        // for reading, seem this procedure of changing the parity phase is needed

	while(length > 0)
	{
		while (!(IFG2 & UCB0TXIFG))
		{
		}
		UCB0TXBUF = 0x00; 					// Receive initiated by a dummy TX write, just for generate clk signals

		while(UCB0STAT & UCBUSY)
		{
		}
		_NOP();
		*p_r_data = UCB0RXBUF;
		p_r_data++;
		length--;
	}
	UCB0CTL0 |= UCCKPH;                   // get the parity phase back
	while(UCB0STAT & UCBUSY)
	{
	}
	SLAVE_SELECT_HIGH; 						// Stop SPI Mode


}

//================================================================


void
SpiReadSingle(char *r_data, char addr)      // read single address value, store into extern read_data, not for IRQ_status read.
{
	SLAVE_SELECT_LOW; 						// Start SPI Mode

		// Address/Command Word Bit Distribution
		addr = (0x40 | addr); 			// address, read, single
		addr = (0x5f & addr);				// register address

		while (!(IFG2 & UCB0TXIFG))			// USCI_B0 TX buffer ready?
		{
		}
		UCB0TXBUF = addr;					// Previous data to TX, RX

		while(UCB0STAT & UCBUSY)
		{
		}

 		trash=UCB0RXBUF;

		UCB0CTL0 &= ~UCCKPH;

		while (!(IFG2 & UCB0TXIFG))			// USCI_B0 TX buffer ready?
		{
		}
		UCB0TXBUF = 0x00; 					// Receive initiated by a dummy TX write???    just used for send 8 clock signals

		while (!(IFG2 & UCB0RXIFG))			// USCI_B0 RX buffer ready?
		{
		}
		_NOP();
		*r_data = UCB0RXBUF;

		UCB0CTL0 |= UCCKPH;

	while(UCB0STAT & UCBUSY)
	{
	}
	SLAVE_SELECT_HIGH; 						// Stop SPI Mode
}
//================================================================

void
SpiSetup(void)                             // init setup
{
	ENABLE_SET;

	IRQ_PIN_SET;
	IRQ_EDGE_SET;								// rising edge interrupt, low to high transition

	SpiUsciSet();								// Set the USART

	//LED_ALL_OFF;
	LED_PORT_SET;
}

//================================================================

void
SpiUsciSet(void)									//Uses USCI_B0
{
	UCB0CTL1 |= UCSWRST;							// Enable SW reset
	UCB0CTL0 |= UCCKPH + UCMSB + UCMST + UCSYNC;	// 3-pin, 8-bit SPI master
	UCB0CTL1 |= UCSSEL_2;							// SMCLK

	UCB0BR0 |= 4;									// SPI clock frequency = SMCLK / 4, 250KHz
	UCB0BR1 = 0;

	P1OUT = 0;
	P1DIR |= BIT5 + BIT7;                           //P1.5, 1.7 output
	P1DIR &= ~BIT6;                                 //P1.6, input

	P1SEL |= BIT5 + BIT6 + BIT7;                   	// P1.7, 1.6, 1.5 UCB0SIMO,UCB0SOMI,UCBOCLK option select
	P1SEL2 |= BIT5 + BIT6 + BIT7;

	SLAVE_SELECT_PORT_SET;							// P1.4 - Slave Select
	SLAVE_SELECT_HIGH;								// Slave Select - inactive ( high)

	UCB0CTL1 &= ~UCSWRST;							// **Initialize USCI state machine**
}

//================================================================

void
SpiWriteCont(char *w_data, char addr, int length)   // write the data from extern write_data, usually used for write FIFO
{
	char *p_w_data = w_data;                // store the pointer position
	SLAVE_SELECT_LOW; 						// Start SPI Mode
	// Address/Command Wort Bit Distribution
	addr = (0x20 | addr); 				// address, write, continuous
	addr = (0x3f & addr);					// register address

	while (!(IFG2 & UCB0TXIFG))			// USCI_B0 TX buffer ready?
			{
			}
			UCB0TXBUF = addr;					// Previous data to TX, RX
			while(UCB0STAT & UCBUSY)
			{
			}
			trash = UCB0RXBUF;

	while(length > 0)
	{
		while (!(IFG2 & UCB0TXIFG))			// USCI_B0 TX buffer ready?
		{
		}
		UCB0TXBUF = *p_w_data;					// Previous data to TX, RX
		while(UCB0STAT & UCBUSY)
		{
		}
		trash = UCB0RXBUF;

		p_w_data++;
		length--;
	}
	while(UCB0STAT & UCBUSY)
	{
	}
	SLAVE_SELECT_HIGH; 						// Stop SPI Mode
}

//================================================================

void
SpiWriteSingle(char *w_data, char addr)
{

	SLAVE_SELECT_LOW; 						// Start SPI Mode

		// Address/Command Word Bit Distribution
		// address, write, single (fist 3 bits = 0)
		addr = (0x1f & addr);				// register address

			while (!(IFG2 & UCB0TXIFG))		// USCI_B0 TX buffer ready?
			{
			}
			UCB0TXBUF = addr;  			// Previous data to TX, RX, I notice that the MOSI pin will remain the LSB of TXBUF when complete tramsmit

			while(UCB0STAT & UCBUSY)
			{
			}

			trash = UCB0RXBUF;

			while (!(IFG2 & UCB0TXIFG))		// USCI_B0 TX buffer ready?
			{
			}
			UCB0TXBUF = *w_data;  			// Previous data to TX, RX

			while(UCB0STAT & UCBUSY)
			{
			}

			trash = UCB0RXBUF;

	while(UCB0STAT & UCBUSY)
	{
	}
		SLAVE_SELECT_HIGH; 					// Stop SPI Mode
}
