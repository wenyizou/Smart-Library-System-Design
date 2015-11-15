/*
 * main.c
 */

#include "msp430g2553.h"
#include "trf7960a.h"
#include "mcu_clk.h"
#include "mcu_trf_port.h"
#include "iso15693.h"
#include <Uart.h>



char addr_data[2];
char read_data[20];
char write_data[20];
char UID_data[13]={1,1,1,1,1,1,1,1,1,1,1,1,0};
char Tag_read_data[11]={2,2,2,2,2,2,2,2,2,1,0};          // storage for send back data from tag, it is flags + response of command
char Tag_write_data[9]={1,1,1,1,1,1,1,1,0};         // block data need to write to tag
char	i_reg = 0;                 // interrupt flag?  0x00 for timer runout, 0x01 for normal response
char	irq_flag = 0;              // IRQ flag
char	rx_error_flag = 0;
char	irq_status_g = 0;
char host_control_flag = 0;
char found = 0;
char find_tag_flag = 0;           // find flag command indicator
char Rx_buf[9] = {0,0,0,0,0,0,0,0,0};                   // store the Uart Rx data
int  Rx_count = 0;                               // store the Rx data count from uart
char inv_or_rw_flag = 'I';                     // for Uart input indicator, I means read 1 byte uart only, other means read at least 4 byte for read or write command
char Tx_data[20] = {0};                       // buf for string need to be sent to computer
char inventory_flag = 0;           // indicate whether the response is from inventory command or not, if yes, inv_flag = 1, then receive data is UID



void main(void) {
	WDTCTL = WDTPW + WDTHOLD;			// stop watchdog timer
		SystemClkSet();       				// set the oscilator, as well as Uart setting
		Uart_Init(9600, 'e', 'l', 8, 1);   // 9600 baudrate, even parity, lsb, 8 bit, 1 stopbit
		IFG1 &= ~OFIFG;        				// Clear OSCFault flag
		host_control_flag = 0;

		/*#ifdef ENABLE_HOST
			UartSetup();					// settings for UART
		#endif */

		ENABLE_SET;							// P2.0 is switched in output direction
		TRF_ENABLE;							// P2.0 = 1, EN pin on the TRF 796x
		DelayMillisecond(10);				// wait until system clock started, usually 1 ms

		Trf796xCommunicationSetup();
		Trf796xDirectCommand(SOFT_INIT);
		DelayMillisecond(10);


		//Trf796xWriteSingle(b,0x00);
		//Trf796xWriteSingle(b,0x0B);
		Trf796xInitialSettings();
		ENABLE_INTERRUPTS;
		_BIS_SR(GIE);
		LED_15693_ON;

		//char a=0x08;
		//char b[2]={0x55,0x00};
		Tag_write_data[0] = 0x03;
		Tag_write_data[1] = 0x04;
		Tag_write_data[2] = 0x05;
		Tag_write_data[3] = 0x06;

		while(1)
		{
////////////////////////////////////////////////
			if (find_tag_flag == 1)                           // uart inventory command
			{
		   	    Iso15693FindTag();
//		   	    Iso15693ReadSingleBlockWithUID(0x01);
//				Iso15693WriteSingleBlockWithoutUID(Tag_write_data, 0x02);
//		    	Iso15693ReadSingleBlockWithoutUID(0x01);
//				Iso15693ReadTwoBlockWithoutUID(0x01);
		   	    if (found == 1)
		   	    {
		   	    	int i;
		   	    	Uart_sendstr("UID is: ");
		   	    	for (i=7;i>=0;i--)
		   	    	{
		   	    		Tx_data[14-2*i] = (UID_data[i+4]>>4)>9 ? ((UID_data[i+4]>>4) - 10 + 'A'): ((UID_data[i+4]>>4) + '0');
		   	    		Tx_data[15-2*i] = (UID_data[i+4] & 0x0f)>9 ? ((UID_data[i+4] & 0x0f) - 10 + 'A'): ((UID_data[i+4] & 0x0f) + '0');
		   	    	}
		   	    	Tx_data[16] = '\0';
		   	    	Uart_sendstr(Tx_data);     // display UID information
		   	    }
		   	    else
		   	    {
		   	    	Uart_sendstr("NO Tag found.\n");
		   	    }
			    DelayMillisecond(100);
			    found=0;                      // reset found flag
				find_tag_flag = 0;                // reset find_tag_flag
			}
/////////////////////////////////////
			else if(inv_or_rw_flag == 'R')            // read block uart command
			{
				Uart_sendstr("Please enter block number from 00 to 63 to read.\n");
				while (inv_or_rw_flag != 'G')
				{
					_nop();                // wait uart send block address, address in decimal
				}
				Rx_buf[8] = (Rx_buf[1]-'0') * 10 + Rx_buf[2] - '0';  // convert decimal to hex
				Iso15693ReadSingleBlockWithoutUID(Rx_buf[8]);
				if (found == 1)
				{
					int i;
					for (i=0;i<4;i++)
					    Tx_data[i] = Tag_read_data[i+3];  // the first 3 byte in FIFO when read is 0x00, 0x00, flags, then data
					Tx_data[4] = '\0';
					Uart_sendstr("Block ");
					Uart_sendchar(Rx_buf[1]);
					Uart_sendchar(Rx_buf[2]);
					Uart_sendstr(" is: ");
					Uart_sendstr(Tx_data);
					Uart_sendchar('\n');                  // send block information if successfully received
				}
				else
				{
					Uart_sendstr("NO Tag found.\n");
				}
			    DelayMillisecond(100);
			    found=0;                      // reset found flag
			    Rx_count = 0;                 // reset Rx count flag
			    inv_or_rw_flag = 'I';         // reset to default
			}
/////////////////////////////////////
			else if(inv_or_rw_flag == 'W')            // read block uart command
			{
				Uart_sendstr("Please enter block number from 00 to 63 to write.\n");
				while (inv_or_rw_flag != 'G')
				{
					_nop();                // wait uart send block address and data to be writen, address in decimal
				}
				if (Rx_buf[3] == 'w')      // uart write command indicator
				{
					Rx_buf[8] = (Rx_buf[1]-'0') * 10 + Rx_buf[2] - '0';  // convert decimal to hex
					Tag_read_data[0] = 0x01;
					Iso15693WriteSingleBlockWithoutUID(&Rx_buf[4], Rx_buf[8]);    // write
					if (found == 1)
					{
						if(Tag_read_data[0] == 0x00)     // means flags = 0, write succeed
							Uart_sendstr("Write Successful\n");
						else if(Tag_read_data[0] == 0x01)
							Uart_sendstr("Write Not Successful\n");
					}
					else
					{
						Uart_sendstr("NO Tag found.\n");
					}
						DelayMillisecond(100);
					    found=0;                      // reset found flag
						Rx_count = 0;                 // reset Rx count flag
						inv_or_rw_flag = 'I';         // reset to default
				}
			}
////////////////////////////////////
		}
}

/////////////////////////////////////////////////////////////
#pragma vector=USCIAB0RX_VECTOR
__interrupt void UartRx ()
{
	switch (inv_or_rw_flag)               // if this is the first word command, means "I" or "R" or "W", only detect 1 uart byte, I is the default
	{	                                      // of inv_or_rw_flag, means only receive 1 byte from uart
	case 'I':
	  {
		Rx_buf[0] = UCAxRXBUF;
	   if (Rx_buf[0] == 'I')                 // 'I' from uart port means inventory command, uart input 'I' under flag 'I' condition
	       find_tag_flag = 1;             // start find flag function
	   else if (Rx_buf[0] == 'R')         // read block command
		   inv_or_rw_flag = 'R';
	   else if (Rx_buf[0] == 'W')         // write block command
		   inv_or_rw_flag = 'W';
	   else
		   inv_or_rw_flag = 0;       // default
       break;
	  }

	case 'R':
     {
    	Rx_count = Rx_count + 1;
    	Rx_buf[Rx_count] = UCAxRXBUF;     // read uart 2 bytes in to Rx_buf, store the block address
    	if (Rx_count == 2)                // receive 2 bytes block address into Rx_buf, actually is 1 byte, but in computer uart
    		                              // transmit, use dec instead of hex, for example, 48 means 0x30, but transmit '4' and '8', 2 bytes
    		inv_or_rw_flag = 'G';          // means 2 bytes receive over, do Trf block read
    	break;
     }

	case 'W':
    {
    	Rx_count = Rx_count + 1;
    	Rx_buf[Rx_count] = UCAxRXBUF;     // read uart 7 bytes in to Rx_buf, store the block address and information should write
    	if (Rx_count == 7)                // receive 7 bytes block address into Rx_buf,2 bytes for address, 1 bytes for 'w'
    		                              // 'w' is the write command format, next 4 bytes is the information need to write inside tag
    		                // a good write command from uart window should be "48wfuck", means write "fuck" in to block 48
    	    inv_or_rw_flag = 'G';          // means 2 bytes receive over, do Trf block read
    	break;
    }

	}
}
