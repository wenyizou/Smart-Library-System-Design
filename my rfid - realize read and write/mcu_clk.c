/*
 * mcu_clk.c
 *
 *  Created on: 2014-4-7
 *      Author: wenyi
 */

#include "mcu_clk.h"
#include "msp430g2553.h"
#include "trf7960a.h"

extern char irq_flag;

//===============================================================

void SystemClkSet(void)
{
	if (CALBC1_1MHZ == 0xFF || CALDCO_1MHZ == 0xFF)
	{
		while(1);						// If calibration constants erased, trap CPU!!
	}

	// Configure Basic Clock
	BCSCTL1 = CALBC1_1MHZ; 				// Set range
	BCSCTL2 |=SELM_0 + DIVM_0 + DIVS_0; // Set MCLK, SMCLK
	DCOCTL = CALDCO_1MHZ;				// Set DCO step + modulation
}

//===============================================================

void DelayMillisecond(int n_ms)
{

 	int temp;
  	int time2;
  	time2 = n_ms*98;
  	for(temp=0;temp<time2;temp++)
		P1OUT = P1OUT;           //  if you want you use self defined function, you have to at least do Port doing in your function one time


}

//===============================================================

void
CounterSet(void)
{
	TACTL |= TACLR;
	TACTL &= ~TACLR;				// reset the timerA
	TACTL |= TASSEL1 + ID1 + ID0;	// MCLK, 1 Mhz, div 8, interrupt enable, timer stoped ,    this part need to be modified

	TAR = 0x0000;                   // stop timer this time
	TACCTL0 |= CCIE;				// compare interrupt enable
}

//===============================================================

#pragma vector=TIMER0_A0_VECTOR
__interrupt void
Msp430f23x0TimerAHandler(void)
{

	STOP_COUNTER;

	irq_flag = 0x03;      // means time out interrupt


	//__low_power_mode_off_on_exit();
}

//===============================================================

