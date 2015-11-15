/*
 * mcu_trf_port.h
 *
 *  Created on: 2014-4-7
 *      Author: wenyi  (original by Reiser Peter)
 */

#ifndef MCU_TRF_PORT_H_
#define MCU_TRF_PORT_H_

#include <msp430g2553.h> 					// prozessor spezific header

//===============================================================
/*
 * This is the MSP430G2553 Port configuration with TRF7960ATB
 *
 * P1.0         -------->      LED
 * P1.4         -------->      Slave select Pin of TRF7960A
 * P2.0         -------->      EN Pin of TRF7960A
 * P2.1         -------->      MOD Pin of TRF7960A
 * P2.2         -------->      IRQ Pin of TRF7960A
 * P2.3         -------->      OOK/ASK Pin of TRF7960A
 * P1.7, 1.6, 1.5 UCB0SIMO,UCB0SOMI,UCBOCLK option select
 */


//===============================================================

#define ENABLE_INTERRUPTS _EINT()

//=====MCU constants=============================================

#define TRF_WRITE 		P1OUT				//port1 is connected to the
#define TRF_READ  		P1IN				//TRF796x IO port.
#define TRF_DIR_IN		P1DIR = 0x00
#define TRF_DIR_OUT		P1DIR = 0xFF
#define TRF_FUNC		P1SEL = 0x00

#define ENABLE_SET		P2DIR |= BIT0		// P2.0 ist switched in output direction
#define	TRF_ENABLE		P2OUT |= BIT0		// EN pin on the TRF796x
#define TRF_DISABLE		P2OUT &= ~BIT0

//---- PIN operations -------------------------------------------

#define CLK_P_OUT_SET 	P1DIR |= BIT5		// DATA_CLK on P1.5 (P1.5/UCB0CLK used in GPIO Mode for Parallel operation)
#define CLK_ON			P1OUT |= BIT5
#define CLK_OFF			P1OUT &= ~BIT5

#define MOD_SET			P2DIR |= BIT1       // MOD on p2.1
#define MOD_ON			P2OUT |= BIT1
#define MOD_OFF			P2OUT &= ~BIT1

#define IRQ_PIN_SET		P2DIR &= ~BIT2      // input
#define IRQ_PIN			BIT2
#define IRQ_PORT		P2IN
#define IRQ_ON			P2IE |= BIT2		// IRQ on P2.2
#define IRQ_OFF			P2IE &= ~BIT2		// IRQ on P2.2
#define IRQ_EDGE_SET	P2IES &= ~BIT2		// rising edge interrupt
#define IRQ_CLR			P2IFG = 0x00
#define IRQ_REQ_REG		P2IFG

#define LED_PORT_SET	P1DIR |= BIT0;      // LED on P 1.0

#define LED_15693_ON	P1OUT |= BIT0;
#define LED_15693_OFF	P1OUT &= ~BIT0;


// #define SPIMODE				0				// This is set to Vcc for parallel mode regardless of the jumper at GND or VCC)
// #define SPIMODE				1				// This is set to Vcc for SPI mode regardless of the jumper at GND or VCC)
#define SPIMODE					1// always spi mode//P2IN & BIT3		// This is set to Vcc for SPI mode and GND for Parallel Mode using a separate jumper
#define SLAVE_SELECT_PORT_SET	P1DIR |= BIT4;  // P1.4 is for slave select
#define SLAVE_SELECT_HIGH		P1OUT |= BIT4;
#define SLAVE_SELECT_LOW		P1OUT &= ~BIT4;

#define OOK_DIR_IN				P2DIR &= ~BIT3;
#define OOK_DIR_OUT				P2DIR |= BIT3
#define OOK_OFF					P2OUT &= ~BIT3
#define OOK_ON					P2OUT |= BIT3



//---------------------------------------------------------------


//---------------------------------------------------------------


#endif /* MCU_TRF_PORT_H_ */
