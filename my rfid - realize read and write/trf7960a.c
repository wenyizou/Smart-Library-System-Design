/*
 * trf7960a.c
 *
 *  Created on: 2014-4-7
 *      Author: wenyi
 */

#include "spi.h"
#include "trf7960a.h"
#include "mcu_trf_port.h"
#include "mcu_clk.h"


//===============================================================

char	command[2];
char	direct_mode = 0;
extern char addr_data[2];
extern char read_data[20];
extern char write_data[20];
extern char UID_data[20];
extern char	i_reg;                 // interrupt flag?  0x00 for timer runout, 0x01 for normal response
extern char	irq_flag;              // IRQ flag
extern char	rx_error_flag;
extern char	irq_status_g;
extern char inventory_flag;
extern char Tag_read_data[11];
extern char flags;

char temp=0;

//===============================================================

void
Trf796xCommunicationSetup(void)            // init the setup of spi communication
{
		SpiSetup();
}

//===============================================================

void
Trf796xDirectCommand(char cmd)            // send direct command
{
		SpiDirectCommand(cmd);
}

//===============================================================

void
Trf796xDisableSlotCounter(void)
{
	addr_data[0] = IRQ_MASK;				// next slot counter
	Trf796xReadSingle(addr_data[0]);
	write_data[0] = read_data[0];
	write_data[0] &= 0xfe;				// clear BIT0 in register 0x01
	Trf796xWriteSingle(write_data, addr_data[0]);
}

//===============================================================

void
Trf796xEnableSlotCounter(void)
{
	addr_data[0] = IRQ_MASK;				// next slot counter
	Trf796xReadSingle(addr_data[0]);
	write_data[0] = read_data[0];
	write_data[0] |= BIT0;				// set BIT0 in register 0x0D, enable no-response interrupt
	Trf796xWriteSingle(write_data, addr_data[0]);
}

//===============================================================

void
Trf796xInitialSettings(void)
{

	addr_data[0] = MODULATOR_CONTROL;
	write_data[0] = 0x21;						// 6.78MHz, OOK 100%

	Trf796xWriteSingle(write_data, addr_data[0]);
}

//===============================================================

void
Trf796xRawWrite(char *raw_data, int length)
{
		SpiRawWrite(raw_data, length);
}

//===============================================================

void
Trf796xRawWriteBlock(char *raw_data, int length)
{
		SpiRawWriteBlock(raw_data, length);     // especially for write block to tag-it
}

//===============================================================

void
Trf796xReadCont(char addr, int length)        // read continue data to extern read_data
{
		SpiReadCont(read_data, addr, length);
}

//===============================================================

void
Trf796xReadSingle(char addr)                 // read single data to extern read_data
{
		SpiReadSingle(read_data, addr);
}

//===============================================================

void
Trf796xReset(void)
{
	command[0] = RESET;
	Trf796xDirectCommand(command[0]);
}

//===============================================================

void
Trf796xReadIrqStatus(void)
{
	addr_data[0] = IRQ_STATUS;

		Trf796xReadCont(addr_data[0], 2);			// read second reg. as dummy read, to clear irq status
		//irq_flag = read_data[0];                    // save irq_flag, as irq status
}

//===============================================================

void
Trf796xResetIrqStatus(void)
{
	addr_data[0] = IRQ_STATUS;

		Trf796xReadCont(addr_data[0], 2);		// read second reg. as dummy read


}

//===============================================================

void
Trf796xRunDecoders(void)
{
	command[0] = RUN_DECODERS;
	Trf796xDirectCommand(command[0]);
}

//===============================================================

void
Trf796xStopDecoders(void)
{
	command[0] = STOP_DECODERS;
	Trf796xDirectCommand(command[0]);
}

//===============================================================

void
Trf796xTransmitNextSlot(void)
{
	command[0] = TRANSMIT_NEXT_SLOT;
	Trf796xDirectCommand(command[0]);
}

//===============================================================

void
Trf796xTurnRfOff(void)
{
	addr_data[0] = CHIP_STATE_CONTROL;
	Trf796xReadSingle(addr_data[0]);
	read_data[0] &= 0x1F;                           // keep the last 5 bits origin, change first 3 bits to 0,0,0, bit 5 is 0 means RF off
	write_data[0] = read_data[0];
	Trf796xWriteSingle(write_data, addr_data[0]);    // rewrite into address
}

//===============================================================

void
Trf796xTurnRfOn(void)
{
	addr_data[0] = CHIP_STATE_CONTROL;
	Trf796xReadSingle(addr_data[0]);
	read_data[0] &= 0x3F;
	read_data[0] |= 0x20;                         // set bit 5 to 1, turn RF on
	write_data[0] = read_data[0];
	Trf796xWriteSingle(write_data, addr_data[0]);    // rewrite into address
}


//===============================================================

void
Trf796xWriteCont(char *w_data, char addr, int length)          // write continue data to addr, usually for fifo
{
		SpiWriteCont(w_data, addr, length);
}

//===============================================================

void
Trf796xWriteIsoControl(char iso_control)           // this fucntion is for decide which ISO protocol should be used
{

	addr_data[0] = ISO_CONTROL;
	write_data[0] = iso_control;
	write_data[0] &= ~BIT5;                      // RFID mode , this bit should always be 0, 1 for reserved
	Trf796xWriteSingle(write_data, addr_data[0]);           // first address, second data

	iso_control &= 0x1F;

	addr_data[0] = IRQ_MASK;
	write_data[0] = 0x3E;                        // 0x3E means enable all the interrupts
	Trf796xWriteSingle(write_data, addr_data[0]);

	if(iso_control < 0x0C)					// ISO14443A or ISO15693
	{
		addr_data[0] = MODULATOR_CONTROL;
		write_data[0] = 0x21;					// OOK 100% 6.78 MHz
		Trf796xWriteSingle(write_data, addr_data[0]);
	}
	else									// ISO14443B
	{
		addr_data[0] = MODULATOR_CONTROL;
		write_data[0] = 0x20;					// ASK 10% 6.78 MHz
		Trf796xWriteSingle(write_data, addr_data[0]);
	}

	if(iso_control < 0x08)					// ISO15693
	{
		addr_data[0] = TX_PULSE_LENGTH_CONTROL;
		write_data[0] = 0x80;					// 9.44 us
		Trf796xWriteSingle(write_data, addr_data[0]);

		if((iso_control < 0x02) || (iso_control == 0x04) || (iso_control == 0x05)) // low data rate
		{
			addr_data[0] = RX_NO_RESPONSE_WAIT_TIME;
			write_data[0] = 0x30;				// 1812 us
			Trf796xWriteSingle(write_data, addr_data[0]);
		}
		else
		{
			addr_data[0] = RX_NO_RESPONSE_WAIT_TIME;
			write_data[0] = 0x14;				// 755 us
			Trf796xWriteSingle(write_data, addr_data[0]);
		}

		addr_data[0] = RX_WAIT_TIME;
		write_data[0] = 0x1F;					// 293 us
		Trf796xWriteSingle(write_data, addr_data[0]);

		addr_data[0] = RX_SPECIAL_SETTINGS;
		write_data[0] = RX_SPECIAL_SETTINGS;
		Trf796xReadSingle(addr_data[0]);
		write_data[0] = read_data[0];
		write_data[0] &= 0x0F;
		write_data[0] |= 0x40;					// bandpass 200 kHz to 900 kHz , for ISO 15693
		Trf796xWriteSingle(write_data, addr_data[0]);

		addr_data[0] = SPECIAL_FUNCTION;        // can't find this registor in datasheet of TRF7960A, maybe not for this chip, remember to comment out
		write_data[0] = SPECIAL_FUNCTION;
		Trf796xReadSingle(addr_data[0]);
		write_data[0] = read_data[0];
		write_data[0] |= BIT4;
		Trf796xWriteSingle(write_data, addr_data[0]);
	}
	else									// ISO14443
	{
		if(iso_control < 0x0C)				// ISO14443A
		{
			addr_data[0] = TX_PULSE_LENGTH_CONTROL;
			write_data[0] = 0x20;					// 2.36 us
			Trf796xWriteSingle(write_data, addr_data[0]);
		}
		else
		{
			addr_data[0] = TX_PULSE_LENGTH_CONTROL;
			write_data[0] = 0x00;					// 2.36 us
			Trf796xWriteSingle(write_data, addr_data[0]);
		}
		addr_data[0] = RX_NO_RESPONSE_WAIT_TIME;
		write_data[0] = 0x0E;					// 529 us
		Trf796xWriteSingle(write_data, addr_data[0]);

		addr_data[0] = RX_WAIT_TIME;
		write_data[0] = 0x07;					// 66 us
		Trf796xWriteSingle(write_data, addr_data[0]);

		addr_data[0] = RX_SPECIAL_SETTINGS;
		write_data[0] = RX_SPECIAL_SETTINGS;
		Trf796xReadSingle(addr_data[0]);
		write_data[0] = read_data[0];
		write_data[0] &= 0x0F;					// bandpass 450 kHz to 1.5 MHz
		write_data[0] |= 0x20;
		Trf796xWriteSingle(write_data, addr_data[0]);

		addr_data[0] = SPECIAL_FUNCTION;
		write_data[0] = SPECIAL_FUNCTION;
		Trf796xReadSingle(addr_data[0]);
		write_data[0] = read_data[0];
		write_data[0] &= ~BIT4;
		Trf796xWriteSingle(write_data, addr_data[0]);
	}
}

//===============================================================

void
Trf796xWriteSingle(char *w_data, char addr)     // write single data to address, usualy used for control register
{
		SpiWriteSingle(w_data, addr);
}

//===============================================================



void
Trf796xISR(char irq_status)
{
	//IRQ_OFF;              // prevent go P2IE IRQ to change irq_status
	int data_len,i;               // data length need to read in fifo
	if(irq_status == 0xA0)			// BIT5 and BIT7
	{								// TX active and only 3 bytes left in FIFO��
		i_reg = 0x00;
	}

	else if(irq_status == BIT7)
	{								// TX complete
		i_reg = 0x00;
		Trf796xReset();				// reset the FIFO after TX
		Trf796xResetIrqStatus();
	}

	else if((irq_status & BIT1) == BIT1)
	{								// collision error
		i_reg = 0x02;				// RX complete

		Trf796xStopDecoders();						// reset the FIFO after TX

		Trf796xReset();

		Trf796xResetIrqStatus();

		IRQ_CLR;
	}

/*	else if(irq_status == BIT6 || irq_status == 0x60) // && read_mode_flag == 0)        // should be BIT6
	{
		i_reg = 0xff;
	addr_data[0] = FIFO_STATUS;
	Trf796xReadSingle(addr_data[0]);			// determine the number of bytes left in FIFO
	data_len = (0x0F & read_data[0]) + 0x01;
	addr_data[0] = FIFO;				// write the recieved bytes to the correct place of the buffer


			Trf796xReadCont(addr_data[0], data_len);     // get the bytes from FIFO into read_data
			for (i=0;i<data_len;i++)
			{
				UID_data[i]=read_data[i];                // store UID data
			}
			if (irq_status == BIT6)
				{
				Trf796xStopDecoders();	// reset the FIFO after TX
			    Trf796xReset();
				}
	}// add one to get the real number of bytes left in FIFO
*/
	else if(irq_status == BIT6)
	{	// RX flag means that EOF has been recieved
		// and the number of unread bytes is in FIFOstatus regiter
		if(rx_error_flag == 0x02)
		{
			i_reg = 0x02;
			return;
		}

		addr_data[0] = FIFO_STATUS;
		Trf796xReadSingle(addr_data[0]);			// determine the number of bytes left in FIFO
		data_len = (0x0F & read_data[0]) + 0x01;   // add one to get the real number of bytes left in FIFO
		addr_data[0] = FIFO;				// write the recieved bytes to the correct place of the buffer


		Trf796xReadCont(addr_data[0], data_len);     // get the bytes from FIFO into read_data
		for (i=0;i<data_len;i++)
		{
			UID_data[i]=read_data[i];                // store UID data
		}
		Trf796xStopDecoders();
		Trf796xReset();					// reset the FIFO after last byte has been read out
		Trf796xResetIrqStatus();

		i_reg = 0xFF;					// signal to the recieve funnction that this are the last bytes
//		Trf796xReset();

//		Trf796xResetIrqStatus();
	}

	else if(irq_status == 0x60)
	{									// RX active and 9 bytes allready in FIFO


		i_reg = 0xFF;
		addr_data[0] = FIFO_STATUS;
		Trf796xReadSingle(addr_data[0]);    // read FIFO_STATUS anyway
		addr_data[0] = FIFO;       // miss read fifo status part?+
		Trf796xReadCont(addr_data[0], 12);	// read 9 bytes from FIFO
		for (i=0;i<12;i++)
		{
			UID_data[i]=read_data[i];                // store UID data
		}

		if(IRQ_PORT & IRQ_PIN)			// if IRQ pin high
		{
					Trf796xReadIrqStatus();
		            irq_status = read_data[0];
					IRQ_CLR;
		if (irq_status == 0x40)
		{
			addr_data[0] = FIFO_STATUS;
			Trf796xReadSingle(addr_data[0]);			// determine the number of bytes left in FIFO

		}

		}
		Trf796xStopDecoders();
		Trf796xReset();
		Trf796xResetIrqStatus();
	}


/*		Trf796xReadCont(addr_data[0], 3);	// read 9 bytes from FIFO
		for (i=9;i<12;i++)
		{
			UID_data[i]=read_data[i-9];                // store UID data
		}
		i_reg = 0xFF;
		Trf796xReset();
	//	rxtx_state = rxtx_state + 9; */
/*		if(IRQ_PORT & IRQ_PIN)			// if IRQ pin high
		{
			Trf796xReadIrqStatus();
            irq_status = read_data[0];
			IRQ_CLR;

			if(irq_status == 0x40)							// end of recieve
			{
				addr_data[0] = FIFO_STATUS;
				Trf796xReadSingle(addr_data[0]);					// determine the number of bytes left in FIFO
				data_len = 0x0F & (read_data[0] + 0x01);
				addr_data[0] = FIFO;						// write the recieved bytes to the correct place of the buffer

				Trf796xReadCont(addr_data[0], data_len);
				for (i=9;i<9+data_len;i++)
				{
					UID_data[i]=read_data[i-9];                // store UID data
				}

				rxtx_state = rxtx_state +*irq_status;

				*irq_status = TX_LENGTH_BYTE_2;					// determine if there are broken bytes
				Trf796xReadSingle(irq_status, 1);					// determine the number of bits

				if((*irq_status & BIT0) == BIT0)
				{
					*irq_status = (*irq_status >> 1) & 0x07;	// mask the first 5 bits
					*irq_status = 8 -*irq_status;
					buf[rxtx_state - 1] &= 0xFF << *irq_status;
				}											// if

				i_reg = 0xFF;			// signal to the recieve funnction that this are the last bytes

				//Trf796xReset();

	//			Trf796xResetIrqStatus();
			}
*/



/*                        error handler, this time don't take into consideration
	else if((irq_status & BIT4) == BIT4)				// CRC error
	{
		if((irq_status & BIT5) == BIT5)
		{
			i_reg = 0x01;	// RX active
			rx_error_flag = 0x02;
		}
		if((irq_status & BIT6) == BIT6)			// 4 Bit receive
		{
			buf[200] = FIFO;						// write the recieved bytes to the correct place of the buffer

			Trf796xReadCont(&buf[200], 1);

			Trf796xReset();

			i_reg = 0x02;	// end of RX
			rx_error_flag = 0x02;
		}
		else
		{
			i_reg = 0x02;	// end of RX
		}
	}

	else if((*irq_status & BIT2) == BIT2)	// byte framing error
	{
		if((*irq_status & BIT5) == BIT5)
		{
			i_reg = 0x01;	// RX active
			rx_error_flag = 0x02;
		}
		else
			i_reg = 0x02;	// end of RX
	}
*/
	else if((irq_status == BIT0))
	{						// No response interrupt
		i_reg = 0x00;
	}
	else
	{						// Interrupt register not properly set


		i_reg = 0x02;

		Trf796xStopDecoders();	// reset the FIFO after TX

		Trf796xReset();

		Trf796xResetIrqStatus();


	}
	IRQ_CLR;
	//IRQ_ON;
}							// Interrupt Service Routine

//////////////////////////////////////////////////////////////

void
Trf796xISR2(char irq_status)
{
	STOP_COUNTER;
	int data_len,i;               // data length need to read in fifo

	if(irq_status == 0xA0)			// BIT5 and BIT7
	{								// TX active and only 3 bytes left in FIFO��
		i_reg = 0x00;
	}

	else if(irq_status == BIT7)
	{								// TX complete
		i_reg = 0x00;
		Trf796xReset();				// reset the FIFO after TX
		Trf796xResetIrqStatus();
        _nop();
	}

	else if(irq_status == BIT6)
		{	// RX flag means that EOF has been recieved
			// and the number of unread bytes is in FIFOstatus regiter
			if(rx_error_flag == 0x02)
			{
				i_reg = 0x02;
				return;
			}

			addr_data[0] = FIFO_STATUS;
			Trf796xReadSingle(addr_data[0]);			// determine the number of bytes left in FIFO
			data_len = (0x0F & read_data[0]) + 0x01 + 2;   // add one to get the real number of bytes left in FIFO
			addr_data[0] = FIFO;				// write the recieved bytes to the correct place of the buffer


			Trf796xReadCont(addr_data[0], data_len);     // get the bytes from FIFO into read_data
			for (i=0;i<data_len;i++)
			{
				Tag_read_data[i]=read_data[i];                // store UID data
			}
			Trf796xStopDecoders();
			Trf796xReset();					// reset the FIFO after last byte has been read out
			Trf796xResetIrqStatus();

			i_reg = 0xFF;					// signal to the recieve funnction that this are the last bytes
	//		Trf796xReset();

	//		Trf796xResetIrqStatus();
		}

	else if(irq_status == 0x60)
		{									// RX active and 9 bytes allready in FIFO


			i_reg = 0xFF;
			addr_data[0] = FIFO_STATUS;
			Trf796xReadSingle(addr_data[0]);    // read FIFO_STATUS anyway
			data_len = (0x0F & read_data[0]) + 0x01 + 2;   // add one to get the real number of bytes left in FIFO
			if (data_len > 12)
				data_len = 12;
			addr_data[0] = FIFO;       // miss read fifo status part?+
			Trf796xReadCont(addr_data[0], data_len);	// read 9 bytes from FIFO
			for (i=0;i<data_len;i++)
			{
				Tag_read_data[i]=read_data[i];                // store Tag data
			}

			if(IRQ_PORT & IRQ_PIN)			// if IRQ pin high
			{
						Trf796xReadIrqStatus();
			            irq_status = read_data[0];
						IRQ_CLR;
			if (irq_status == 0x40)
			{
				addr_data[0] = FIFO_STATUS;
				Trf796xReadSingle(addr_data[0]);			// determine the number of bytes left in FIFO

			}

			}
			Trf796xStopDecoders();
			Trf796xReset();
			Trf796xResetIrqStatus();
		}

		else if((irq_status == BIT0))
			{						// No response interrupt
				i_reg = 0x00;
			}
		else
			{						// Interrupt register not properly set


				i_reg = 0x02;

				Trf796xStopDecoders();	// reset the FIFO after TX

				Trf796xReset();

				Trf796xResetIrqStatus();


			}
	IRQ_CLR;

}

////////////////////////////////////////////////////////////
//===============================================================
// 02DEC2010	RP	Original Code
//===============================================================

#pragma vector = PORT2_VECTOR
__interrupt void
Trf796xPortB(void)							// interrupt handler
{
//	STOP_COUNTER;							// stop timer mode
	irq_flag = 0x02;
	do{

		Trf796xReadIrqStatus();

		irq_status_g = read_data[0];
		if (irq_status_g == 0xA0)
		{
			int i;
		for (i=0;i<400;i++)                   // TX active and only 3 bytes left in FIFO, should be A0,
			_nop();                          // this unique delay just for write command, when read IrqStatus, the Irq will go down first and then
		                                    // go up again in a short time, this delay just to make sure the Irq still high after read 0xA0
		continue;
		}

		// IRQ status register has to be read

		if (inventory_flag == 1)
		{
			Trf796xISR(irq_status_g);       // inventory command route
		}

		if (inventory_flag == 0)
		{
			Trf796xISR2(irq_status_g);      // non-inventory command route
		}

		IRQ_CLR;
	}while((IRQ_PORT & IRQ_PIN) == IRQ_PIN);
	// PORT2 interrupt flag clear
	//__low_power_mode_off_on_exit();
}
