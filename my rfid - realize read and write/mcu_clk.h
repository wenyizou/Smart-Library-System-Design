/*
 * mcu_clk.h
 *
 *  Created on: 2014-4-7
 *      Author: wenyi
 */

#ifndef MCU_CLK_H_
#define MCU_CLK_H_

//-----Counter-timer constants-----------------------------------

#define COUNT_VALUE	TACCR0					//counter register
// #define START_COUNTER	TACTL |= MC0 + MC1	//start counter in up/down mode
#define START_COUNTER	TACTL |=  MC1		//start counter in up mode

#define STOP_COUNTER	TACTL &= ~(MC0 + MC1)	//stops the counter

//---------------------------------------------------------------

#define COUNT_1ms		125      // 1Mhz SMCL, div by 8, 125Khz, 125 ticks is 1 ms
#define COUNT_60ms		7500

//===============================================================

void SystemClkSet(void);
void DelayMillisecond(int n_ms);
void CounterSet(void);

//===============================================================

#endif /* MCU_CLK_H_ */
