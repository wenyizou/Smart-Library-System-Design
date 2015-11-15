/*
 * spi.h
 *
 *  Created on: 2014-4-7
 *      Author: wenyi
 *
 *  function: MSP430G2553 SPI function according to TRF7960A
 */

#ifndef SPI_H_
#define SPI_H_

#include "msp430g2553.h"

//===============================================================



//===============================================================
void SpiClkLow(void);
void SpiClkLow(void);
void SpiDirectCommand(char cmd);
void SpiRawWrite(char *data, int length);
void SpiRawWriteBlock(char *data, int length);    // especially for write block to tag-it
void SpiReadCont(char *r_data, char addr, int length);
void SpiReadSingle(char *r_data, char addr);
void SpiSetup(void);
void SpiUsciSet(void);
void SpiWriteCont(char *w_data, char addr, int length);
void SpiWriteSingle(char *w_data, char addr);


#endif /* SPI_H_ */
