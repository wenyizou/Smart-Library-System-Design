/*
 * iso15693.c
 *
 *  Created on: 2014-4-7
 *      Author: wenyi
 */


/****************************************************************
* FILENAME: iso15693.c
*
* BRIEF: Contain functions to search ISO15693 standardized tags
* in stand alone mode and execute ISO15693 Anticollision in
* remote mode.
*
* Copyright (C) 2010 Texas Instruments, Inc.
*
* AUTHOR(S): Reiser Peter		DATE: 02 DEC 2010
*
* EDITED BY: wenyi
* *
*
****************************************************************/

#include "iso15693.h"
#include "trf7960a.h"
#include "mcu_trf_port.h"
#include "mcu_clk.h"

//===============================================================

char afi = 0;
char flags = 0;							// stores the mask value (used in anticollision)
extern char i_reg;
extern char irq_flag;
extern char rx_error_flag;
extern char host_control_flag;
extern char irq_status_g;
extern char addr_data[2];
extern char read_data[20];
extern char write_data[20];
extern char UID_data[13];
extern char found;
extern char inventory_flag;

//===============================================================
// NAME: void Iso15693FindTag(void)
//
// BRIEF: Is used to detect ISO15693 conform tags in stand alone
// mode.
//
// INPUTS:
//
// OUTPUTS:
//
// PROCESS:	[1] turn on RF driver
//			[2] do a complete anticollision sequence
//			[3] turn off RF driver
//
// NOTE: If ISO15693 conform Tag is detected, ISO15693 LED will
//       be turned on.
//
// CHANGE:
// DATE  		WHO	DETAIL
// 23Nov2010	RP	Original Code
//===============================================================

void
Iso15693FindTag(void)
{
	inventory_flag = 1;                       // this is an inventory command
	Trf796xTurnRfOn();

	Trf796xWriteIsoControl(0x02);               // ISO 15693 high bit mode, 1 out of 4 mode, this is default mode

	// The VCD should wait at least 1 ms after it activated the
	// powering field before sending the first request, to
	// ensure that the VICCs are ready to receive it. (defined by protocol ISO15693-3)
	DelayMillisecond(1);

	flags = SIXTEEN_SLOTS;          // flags need for VCD to VICC communication, 16 slots, inventory command, high data rate, see ISO 15693 Part 3

	write_data[15] = 0x00;
	Iso15693Anticollision(&write_data[15], 0);					// send Inventory request

	Trf796xTurnRfOff();

	Trf796xResetIrqStatus();
	// clear any IRQs
}

//===============================================================
// NAME: void Iso15693Anticollision(u08_t *mask, u08_t length)
//
// BRIEF: Is used to perform a inventory cycle of 1 or 16
// timeslots.
//
// INPUTS:
//	Parameters:
//		u08_t		*mask		mask value
//		u08_t		length		number of significant bits of
//								mask value
//
// OUTPUTS:
//
// PROCESS:	[1] send command
//			[2] receive respond
//			[3] send respond to host
//
// CHANGE:
// DATE  		WHO	DETAIL
// 23Nov2010	RP	Original Code
//  3May2011    AF  Bugfix
//===============================================================

void
Iso15693Anticollision(char *mask, int length)		// host command 0x14,       TI's command???
{




	int	i = 1, j = 1, num_slots, test_flag = 0;

	int	*p_slot_num, slot_num[17];
	int	 new_length, mask_size;
	char new_mask[8];
	int	size;

	int	fifo_length = 0;


	slot_num[0] = 0x00;

	// BUGFIX
	TRF796xCheckRXWaitTime();

	if((flags & BIT5) == 0x00)							// flag bit5 is the number of slots indicator
	{
		num_slots = 16;									// 16 slots if bit is cleared
		Trf796xEnableSlotCounter();
	}
	else
	{
		num_slots = 1;									// 1 slot if bit is set
	}

	p_slot_num = &slot_num[0];							// slot number pointer

	mask_size = (((length >> 2) + 1) >> 1);				// mask_size is 1 for length = 4 or 8

	write_data[0] = 0x8F;                                      // 0x80+0x0F, 0x80 means command, 0x0F means software reset TRF7960A
	write_data[1] = 0x91;										// send with CRC ,   0x08+0x11, 0x11 means transmit with CRC
	write_data[2] = 0x3D;										// write continuous from 1D,   address is 0x1D, 0x02 means write continuously, 0x1D,0x1E is tx length, 0x1F is FIFO
	write_data[5] = flags;										// ISO15693 flags ,  this is the first thing write to FIFO
	write_data[6] = 0x01;										// anticollision command code , this is the next, the format see ISO 15693 Part 3

	//optional afi should be here
	if(flags & 0x10)                                    // judge whether afi available or not
	{
		// mask_size is 2 for length = 12 or 16 ;
		// and so on

		size = mask_size + 4;							// mask value + mask length + afi + command code + flags

		write_data[7] = afi;
		write_data[8] = length;								// masklength
		if(length > 0)
		{
			for(i = 0; i < mask_size; i++)
            {
            	write_data[9 + i] = *(mask + i);
            }
		}
		fifo_length = 9;
	}
	else
	{
		// mask_size is 2 for length = 12 or 16
		// and so on

		size = mask_size + 3;							// mask value + mask length + command code + flags

		write_data[7] = length;								// masklength
		if(length > 0)
		{
			for(i = 0; i < mask_size; i++)
			{
				write_data[8 + i] = *(mask + i);
			}
		}
		fifo_length = 8;
	}

	write_data[3] = (char) (size >> 8);                 // data length need to right to the fifo, store in 0x1D and 0x1E
	write_data[4] = (char) (size << 4);

	Trf796xResetIrqStatus();



	IRQ_CLR;											// PORT2 interrupt flag clear
	IRQ_ON;

	Trf796xRawWrite(&write_data[0], mask_size + fifo_length);	// writing to FIFO, start tx transmit



	i_reg = 0x01;
	irq_flag = 0x00;
//	tx_flag = 1;                                 // this is a flag for tx indicator, if 1, means counter use for tx complete, if 0 use for rx

	while(irq_flag == 0x00)                      // wait for end of TX interrupt, from IRQ, PORT2 interrupt
	{
	}

/*	CounterSet();									// TimerA set, wait another 20ms after tx complete
	COUNT_VALUE = COUNT_1ms * 20;						// 20ms
	START_COUNTER;										//	start timer up mode
*/  Trf796xReset();	                              // clear fifo


	for(j = 1; j <= num_slots; j++)						// 1 or 16 available timeslots
	{	//rxtx_state = 1;									// prepare the extern counter

		// the first UID will be stored from buf[1] upwards
/*		CounterSet();								// TimerA set
		COUNT_VALUE = COUNT_1ms * 20;
		START_COUNTER;									// start timer up mode
*/
											// wait for interrupt from IRQ, this time for rx interrupt
	    while(irq_flag == 0x00)                      // wait for end of first RX interrupt, from IRQ, PORT2 interrupt
	    {
	    	// polling here to wait interrupt end, method 2
	    }


//		Trf796xReadIrqStatus();               // read interrupt kind, no matter whether is no_response or IRQ
//		Trf796xISR(irq_status_g);                  // do the thing according to iqr_status


/*		command[0] = RSSI_LEVELS;						// read RSSI levels
		Trf796xReadSingle(command);
*/
		switch(i_reg)
		{
			case 0xFF:									// if recieved UID in buffer
					found = 1;
				break;

			case 0x02:									// collision occured
				p_slot_num++;							// remember a collision was detected
				*p_slot_num = j;
				break;

			case 0x00:									// timer interrupt
				break;

			default:
				break;
		}

		Trf796xReset();									// FIFO has to be reset before recieving the next response
		if((num_slots == 16) && (j < 16))				// if 16 slots used send EOF(next slot)
		{
			Trf796xStopDecoders();
			Trf796xRunDecoders();
			{
			if (found == 0)
			{
			Trf796xTransmitNextSlot();
			}
			else if(found == 1)
			{
				Trf796xTransmitNextSlot();                // so wired that if found ==1, this command wont generate IRQ
				test_flag=1;
			}
			}
			irq_flag = 0x00;
			Trf796xReset();

	/*		while(irq_flag == 0x00)
			{
				if (found == 1 && test_flag == 1)
					{test_flag = 0;
				     break;}
			}*/
		}
		else if((num_slots == 16) && (j == 16))			// at the end of slot 16 stop the slot counter
		{	Trf796xStopDecoders();
			Trf796xDisableSlotCounter();
		}
		else if(num_slots == 1)							// 1 slot is used
		{
			break;
		}

	}													// for

	if(host_control_flag == 0)
	{
		if(found == 1)									// LED on?
		{
			LED_15693_ON;								// LEDs indicate detected ISO15693 tag
		}
		else
		{
			LED_15693_OFF;
		}
	}

	new_length = length + 4; 							// the mask length is a multiple of 4 bits

	mask_size = (((new_length >> 2) + 1) >> 1);

	while((*p_slot_num != 0x00) && (num_slots == 16) && (new_length < 61) && (slot_num[16] != 16))
	{
		*p_slot_num = *p_slot_num - 1;

		for(i = 0; i < 8; i++)
		{
			new_mask[i] = *(mask + i);				//first the whole mask is copied
		}

		if((new_length & BIT2) == 0x00)
		{
			*p_slot_num = *p_slot_num << 4;
		}
		else
		{
			for(i = 7; i > 0; i--)
			{
				new_mask[i] = new_mask[i - 1];
			}
			new_mask[0] &= 0x00;
		}
		new_mask[0] |= *p_slot_num;				// the mask is changed
		DelayMillisecond(10);

		Iso15693Anticollision(&new_mask[0], new_length);	// recursive call with new Mask

		p_slot_num--;
	}

	IRQ_OFF;

}														// Iso15693Anticollision

//===============================================================
// NAME: void TRF796xCheckRXWaitTime()
//
// BRIEF: a bug writes wrong value in register 0x07 "RX no response wait"
//        this causes ISO15693 Anticollision to fail
//
// INPUTS:
//
// OUTPUTS:
//
// PROCESS:	[1] read ISO register to check if low or high data rate
//			[2] set correct value in register 0x07 "RX no response wait"
//
//
// CHANGE:
// DATE  		WHO	DETAIL
// 3May2011	Andre Frantzke	Bugfix
//===============================================================

void TRF796xCheckRXWaitTime(void)
{

	// [1] read ISO register to check if low or high data rate
	addr_data[0] = ISO_CONTROL;
	Trf796xReadSingle (addr_data[0]);

	//[2] set correct value in register 0x07 "RX no response wait"
	if((read_data[0] < 0x02) || (read_data[0] == 0x04) || (read_data[0] == 0x05)) // low data rate
	{
		addr_data[0] = RX_NO_RESPONSE_WAIT_TIME;
		write_data[0] = 0x30;				// 1812 us
		Trf796xWriteSingle(write_data, addr_data[0]);
	}
	else
	{
		addr_data[0] = RX_NO_RESPONSE_WAIT_TIME;
		write_data[0] = 0x14;				// 752 us
		Trf796xWriteSingle(write_data, addr_data[0]);
	}


}

///////////////////////////////////////////////////////////

void Iso15693WriteSingleBlockWithoutUID(char *w_data, char Block_addr)
{
	    Trf796xTurnRfOn();
		Trf796xReset();
	    inventory_flag = 0;                                        // not a inventory command
	    flags = Write_without_UID;
	    write_data[0] = 0x8F;                                      // 0x80+0x0F, 0x80 means command, 0x0F means software reset TRF7960A
		write_data[1] = 0x91;										// send with CRC ,   0x08+0x11, 0x11 means transmit with CRC
		write_data[2] = 0x3D;										// write continuous from 1D,   address is 0x1D, 0x02 means write continuously, 0x1D,0x1E is tx length, 0x1F is FIFO
		write_data[5] = flags;										// ISO15693 flags ,  this is the first thing write to FIFO
		write_data[6] = 0x21;										// write single command code , this is the next, the format see ISO 15693 Part 3
        write_data[7] = Block_addr;                                 // coming next is block address
        write_data[8] = *w_data;
        write_data[9] = *(w_data+1);
        write_data[10] = *(w_data+2);
        write_data[11] = *(w_data+3);
//        write_data[12] = 0;

        int size = 7;                                     // 1 is another non-use bit for delay 10ms
		write_data[3] = (char) (size >> 8);                 // data length need to right to the fifo, store in 0x1D and 0x1E
		write_data[4] = (char) (size << 4);

		Trf796xResetIrqStatus();



		IRQ_CLR;											// PORT2 interrupt flag clear
		IRQ_ON;

		irq_flag = 0 ;
		Trf796xRawWrite(&write_data[0], size+5);	// writing to FIFO, start tx transmit

		while(irq_flag == 0x00)                      // wait for end of TX interrupt, from IRQ, PORT2 interrupt
		{
		}
		irq_flag = 0 ;                               // wait another interrupt

		Trf796xReset();
		Trf796xResetIrqStatus();
		DelayMillisecond(15);                          // delay at least 10ms before send eof to request tag response , for tag writing(tag-it definition)

		CounterSet();									// TimerA set, wait another 20ms after tx complete
		COUNT_VALUE = COUNT_1ms * 20;						// 20ms
		START_COUNTER;										//	start timer up mode
		Trf796xTransmitNextSlot();
		while(irq_flag == 0x00)                      // wait for end of TX interrupt, from IRQ, PORT2 interrupt
		{
		}
	    Trf796xReset();	                              // clear fifo
     	Trf796xTurnRfOff();
	    Trf796xResetIrqStatus();

		switch(i_reg)                                // determine receive response or not
		{
			case 0xFF:									// if recieved response from tag
					found = 1;
				break;
			default:
				break;
		}

}
///////////////////////////////////////////////////////////

void Iso15693ReadSingleBlockWithoutUID(char Block_addr)
{
	    Trf796xTurnRfOn();
		Trf796xReset();
	    inventory_flag = 0;                                        // not a inventory command
	    flags = Read_without_UID;
	    write_data[0] = 0x8F;                                      // 0x80+0x0F, 0x80 means command, 0x0F means software reset TRF7960A
		write_data[1] = 0x91;										// send with CRC ,   0x08+0x11, 0x11 means transmit with CRC
		write_data[2] = 0x3D;										// write continuous from 1D,   address is 0x1D, 0x02 means write continuously, 0x1D,0x1E is tx length, 0x1F is FIFO
		write_data[5] = flags;										// ISO15693 flags ,  this is the first thing write to FIFO
		write_data[6] = 0x20;										// read single command code , this is the next, the format see ISO 15693 Part 3
        write_data[7] = Block_addr;                                 // coming next is block address

        int size = 3;
		write_data[3] = (char) (size >> 8);                 // data length need to right to the fifo, store in 0x1D and 0x1E
		write_data[4] = (char) (size << 4);

		Trf796xResetIrqStatus();



		IRQ_CLR;											// PORT2 interrupt flag clear
		IRQ_ON;

		irq_flag = 0 ;
		Trf796xRawWrite(&write_data[0], size+5);	// writing to FIFO, start tx transmit
		while(irq_flag == 0x00)                      // wait for end of TX interrupt, from IRQ, PORT2 interrupt
		{
		}
		irq_flag = 0 ;                               // wait another interrupt

		CounterSet();									// TimerA set, wait another 20ms after tx complete
		COUNT_VALUE = COUNT_1ms * 20;						// 20ms
		START_COUNTER;										//	start timer up mode
		while(irq_flag == 0x00)                      // wait for end of TX interrupt, from IRQ, PORT2 interrupt
		{
		}
	    Trf796xReset();	                              // clear fifo
     	Trf796xTurnRfOff();
	    Trf796xResetIrqStatus();

		switch(i_reg)                                // determine receive response or not
		{
			case 0xFF:									// if recieved response from tag
					found = 1;
				break;
			default:
				break;
		}

}
///////////////////////////////////////////////////////////

void Iso15693ReadTwoBlockWithoutUID(char Block_addr)
{
	    Trf796xTurnRfOn();
		Trf796xReset();
	    inventory_flag = 0;                                        // not a inventory command
	    flags = Read_without_UID;
	    write_data[0] = 0x8F;                                      // 0x80+0x0F, 0x80 means command, 0x0F means software reset TRF7960A
		write_data[1] = 0x91;										// send with CRC ,   0x08+0x11, 0x11 means transmit with CRC
		write_data[2] = 0x3D;										// write continuous from 1D,   address is 0x1D, 0x02 means write continuously, 0x1D,0x1E is tx length, 0x1F is FIFO
		write_data[5] = flags;										// ISO15693 flags ,  this is the first thing write to FIFO
		write_data[6] = 0x23;										// read multi command code , this is the next, the format see ISO 15693 Part 3
        write_data[7] = Block_addr;                                 // coming next is block address
        write_data[8] = 0x02;                                       // two block

        int size = 4;
		write_data[3] = (char) (size >> 8);                 // data length need to right to the fifo, store in 0x1D and 0x1E
		write_data[4] = (char) (size << 4);

		Trf796xResetIrqStatus();



		IRQ_CLR;											// PORT2 interrupt flag clear
		IRQ_ON;

		irq_flag = 0 ;
		Trf796xRawWrite(&write_data[0], size+5);	// writing to FIFO, start tx transmit
		while(irq_flag == 0x00)                      // wait for end of TX interrupt, from IRQ, PORT2 interrupt
		{
		}
		irq_flag = 0 ;                               // wait another interrupt

		CounterSet();									// TimerA set, wait another 20ms after tx complete
		COUNT_VALUE = COUNT_1ms * 20;						// 20ms
		START_COUNTER;										//	start timer up mode
		while(irq_flag == 0x00)                      // wait for end of TX interrupt, from IRQ, PORT2 interrupt
		{
		}
	    Trf796xReset();	                              // clear fifo
     	Trf796xTurnRfOff();
	    Trf796xResetIrqStatus();

		switch(i_reg)                                // determine receive response or not
		{
			case 0xFF:									// if recieved response from tag
					found = 1;
				break;
			default:
				break;
		}

}
///////////////////////////////////////////////////////////

void Iso15693ReadSingleBlockWithUID(char Block_addr)
{
	    Trf796xTurnRfOn();
	    Trf796xReset();
	    int i;
	    inventory_flag = 0;                                        // not a inventory command
	    flags = Read_with_UID;
	    write_data[0] = 0x8F;                                      // 0x80+0x0F, 0x80 means command, 0x0F means software reset TRF7960A
		write_data[1] = 0x91;										// send with CRC ,   0x08+0x11, 0x11 means transmit with CRC
		write_data[2] = 0x3D;										// write continuous from 1D,   address is 0x1D, 0x02 means write continuously, 0x1D,0x1E is tx length, 0x1F is FIFO
		write_data[5] = flags;										// ISO15693 flags ,  this is the first thing write to FIFO
		write_data[6] = 0x20;										// read single command code , this is the next, the format see ISO 15693 Part 3
		for(i=7;i<15;i++)
		{
			write_data[7] = UID_data[i-3];
		}
		write_data[15] = Block_addr;                                 // coming next is block address

        int size = 11;
		write_data[3] = (char) (size >> 8);                 // data length need to right to the fifo, store in 0x1D and 0x1E
		write_data[4] = (char) (size << 4);

		Trf796xResetIrqStatus();



		IRQ_CLR;											// PORT2 interrupt flag clear
		IRQ_ON;

		irq_flag = 0 ;
		Trf796xRawWrite(&write_data[0], size+5);	// writing to FIFO, start tx transmit
		while(irq_flag == 0x00)                      // wait for end of TX interrupt, from IRQ, PORT2 interrupt
		{
		}

	/*	CounterSet();									// TimerA set, wait another 20ms after tx complete
		COUNT_VALUE = COUNT_1ms * 20;						// 20ms
		START_COUNTER;										//	start timer up mode

	*/  CounterSet();									// TimerA set, wait another 20ms after tx complete
	    COUNT_VALUE = COUNT_1ms * 20;						// 20ms
	    START_COUNTER;										//	start timer up mode
	    while(irq_flag == 0x00)                      // wait for end of TX interrupt, from IRQ, PORT2 interrupt
	    {
	    }
		Trf796xTurnRfOff();
		Trf796xResetIrqStatus();
		Trf796xReset();	                              // clear fifo

}


///////////////////////////////////////////////////////////


