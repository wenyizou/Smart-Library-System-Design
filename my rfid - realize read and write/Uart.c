/*
 * Uart.c
 *A Uart setting and using header file for msp430G2553, USCI interface
 *  Created on: 2014-2-13
 *      Author: Wenyi Zou
 */
#include  <msp430g2553.h>
#include  <Uart.h>
//////////////////////////////////////////////////////////
void Uart_Init(double BaudRate, char parity, char LMSBMode, int bitMode, int stopbitMode)     // Uart Initiation
{
	double Clk;
	UCAxCTL1 |= UCSWRST;                  // set the USCI into reset mode
	Clk = Uart_setBaudClock(BaudRate);        // set the Uart clock
	Uart_setBaudRate(BaudRate, Clk);          // set the BaudRate
	Uart_setParity(parity);              // set parity bit
	Uart_setLMSBMode(LMSBMode);           // set LSB or MSB mode
	Uart_setbitMode(bitMode);                  // set bit mode
	Uart_setstopbitMode(stopbitMode);         // set stop bit mode

    // UCMODE AND UCSYSC two control bit not used this file, welcome to add and modify.
	UCAxCTL1 &= ~UCSWRST;                  // clear the UCSWRST bit for start
	P1SEL |= BIT1 + BIT2;                // set the P1.1 and P1.2 into secondary peripheral function, can change with differet MSP430 device
	P1SEL2 |= BIT1 + BIT2;               // this setting also needed
	P1DIR  |= BIT2;                       // P1.2 output mode
    UCAxIE |=  UCAxRXIE;         // enable the receive interrupt

}


////////////////////////////////////////////////////////////
double Uart_setBaudClock(double BaudRate)     // according baudrate to set the clock USCI use
{
	double bdclk=0;                     // return value for baudclock, convenient for the setBaudRate function
	UCAxCTL1 &= ~(UCSSEL1+UCSSEL0);       //清除之前的时钟设置 , 1 is 0x80, 0 is 0x40
	if(BaudRate<9600)                  //brclk为时钟源频率
	{
	  UCAxCTL1 |= UCSSEL0;              //ACLK，降低功耗
	  bdclk = 32768;                //波特率发生器时钟频率=ACLK(32768)
	}
	else
	{
	  UCAxCTL1 |= UCSSEL1;              //SMCLK，保证速度
	  bdclk = 1000000;              //波特率发生器时钟频率=SMCLK(1MHz) , 设定时钟一般为1Mhz
	}
	return bdclk;
}

/////////////////////////////////////////////////////////////
void Uart_setBaudRate(double BaudRate, double Clk)      // according to the baudrate and clock, set control register
{
	int n = Clk / BaudRate;     //整数波特率          , see datasheet for more calculation information

	    UCAxBR1 = n >> 8;         //高8位
	    UCAxBR0 = n & 0xff;       //低8位
	    UCAxMCTL = 0;           // modulation set 0 at the beginning
	    UCAxMCTL = UCBRS2;      // set modulation, this time choose UCBRS2, it depends

	}

//////////////////////////////////////////////////////////////
void Uart_setParity(char parity)              // set Parity bit
{
switch(parity)
{
    case 'n':case'N': UCAxCTL0 &= ~UCPEN;               break;  //无校验
    case 'e':case'E': UCAxCTL0 |= UCPEN + UCPAR;          break;  //偶校验
    case 'o':case'O': UCAxCTL0 |= UCPEN; UCAxCTL0 &= ~UCPAR; break;  //奇校验
}
}

/////////////////////////////////////////////////////////////////
void Uart_setLMSBMode(char LMSBMode)           // set LSB or MSB mode
{
switch(LMSBMode)
{
    case 'l':case'L': UCAxCTL0 &= ~UCMSB;               break;  //LSB first
    case 'm':case'M': UCAxCTL0 |= UCMSB;             break;     //MSB first
}
}

/////////////////////////////////////////////////////////////////
void Uart_setbitMode(int bitMode)                  // set bit mode
{
switch(bitMode)
{
    case '8': UCAxCTL0 &= ~UC7BIT;               break;  //LSB first
    case '7': UCAxCTL0 |= UC7BIT;                break;     //MSB first
}
}

///////////////////////////////////////////////////////////////////
void Uart_setstopbitMode(int stopbitMode)         // set stop bit mode
{
switch(stopbitMode)
{
    case '1': UCAxCTL0 &= ~UCSPB;               break;  //LSB first
    case '2': UCAxCTL0 |= UCSPB;                break;     //MSB first
}
}

//////////////////////////////////////////////////////////////////////
void Uart_sendchar(char zifu)                // send char
{
	while((UCAxIFG & UCAxTXIFG) == 0) _NOP();              // if transmit flag still not reset, means busy, trap and wait
	UCAxTXBUF = zifu;
}

//////////////////////////////////////////////////////////////////////
void Uart_sendstr(char *str)                 // send string
{
	while(*str)
	Uart_sendchar(*str++);
}

/////////////////////////////////////////////////////////////////////
char Uart_readchar()                         // reserve function , I don't know when to use this one, instead of interrupt one
{
	while((UCAxIFG & UCAxRXIFG) == 0) _NOP();              // if transmit flag still not reset, means busy, trap and wait
	return UCAxRXBUF;
}
