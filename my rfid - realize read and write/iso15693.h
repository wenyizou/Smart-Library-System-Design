/*
 * iso15693.h
 *
 *  Created on: 2014-4-7
 *      Author: wenyi
 */

#ifndef ISO15693_H_
#define ISO15693_H_

//================================================================

#include <msp430g2553.h> 					// prozessor spezific header
//#include <stdio.h>							// standard input/output header


//===============================================================

#define SIXTEEN_SLOTS	0x06
#define ONE_SLOT		0x26
#define Read_without_UID  0x02
#define Read_with_UID   0x22
#define Write_without_UID  0x42             // high speed, no address mode, option flag must set according to tag-it definition

//===============================================================

// if disabled file ISO15693.c may be excluded from build
#define	ENABLE15693							// delete to disable standard

//===============================================================

void Iso15693FindTag(void);
void Iso15693Anticollision(char *mask, int length);
void TRF796xCheckRXWaitTime(void);
void Iso15693WriteSingleBlockWithoutUID(char *w_data, char Block_addr);
void Iso15693ReadSingleBlockWithoutUID(char Block_addr);
void Iso15693ReadTwoBlockWithoutUID(char Block_addr);
void Iso15693ReadSingleBlockWithUID(char Block_addr);
//===============================================================


#endif /* ISO15693_H_ */
