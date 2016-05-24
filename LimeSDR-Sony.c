/*
             LUFA Library
     Copyright (C) Dean Camera, 2009, 2013 Lime Microsystems.
              
  dean [at] fourwalledcubicle [dot] com
      www.fourwalledcubicle.com
*/

/*
  Copyright 2009  Dean Camera (dean [at] fourwalledcubicle [dot] com)

  Permission to use, copy, modify, and distribute this software
  and its documentation for any purpose and without fee is hereby
  granted, provided that the above copyright notice appear in all
  copies and that both that the copyright notice and this
  permission notice and warranty disclaimer appear in supporting
  documentation, and that the name of the author not be used in
  advertising or publicity pertaining to distribution of the
  software without specific, written prior permission.

  The author disclaim all warranties with regard to this
  software, including all implied warranties of merchantability
  and fitness.  In no event shall the author be liable for any
  special, indirect or consequential damages or any damages
  whatsoever resulting from loss of use, data or profits, whether
  in an action of contract, negligence or other tortious action,
  arising out of or in connection with the use or performance of
  this software.
  
  Lime Microsystems modified this source. Left the main part of 
  the code which implements virtual COM port functionality. 
  Added additional functionality for LMS board controlling.
*/
#include "LimeSDR-Sony.h"
#include "global.h"
#include "LMS64C_protocol.h"

/* Scheduler Task List */
TASK_LIST
{
	{ .Task = USB_USBTask          , .TaskStatus = TASK_STOP },
	{ .Task = CDC_Task             , .TaskStatus = TASK_STOP },
	{ .Task = Main_Task            , .TaskStatus = TASK_STOP },
};

/* Globals: */
/** Contains the current baud rate and other settings of the virtual serial port.
 *
 *  These values are set by the host via a class-specific request, and the physical USART should be reconfigured to match the
 *  new settings each time they are changed by the host.
 */
CDC_Line_Coding_t LineCoding = { .BaudRateBPS = 9600,
                                 .CharFormat  = OneStopBit,
                                 .ParityType  = Parity_None,
                                 .DataBits    = 8            };

/** Ring (circular) buffer to hold the RX data - data from the host to the attached device on the serial port. */
RingBuff_t Rx_Buffer;

/** Ring (circular) buffer to hold the TX data - data from the attached device on the serial port to the host. */
RingBuff_t Tx_Buffer;

enum {
SPI_ADDR_0x00, SPI_ADDR_0x01, SPI_ADDR_0x02,
SPI_ADDR_0x10,
SPI_ADDR_0x20, SPI_ADDR_0x21, SPI_ADDR_0x22, SPI_ADDR_0x23, SPI_ADDR_0x24, SPI_ADDR_0x25,
SPI_ADDR_0x30, SPI_ADDR_0x31,
SPI_ADDR_ALL};

unsigned int spi_reg[SPI_ADDR_ALL], spi_reg_addr, spi_reg_data;


/* some global variables used throughout */
unsigned char tx_buff[LMS_CTRL_PACKET_SIZE], rx_buff[LMS_CTRL_PACKET_SIZE], count, reg_idx, Exp_board_id;

enum {LED_MODE_OFF, LED_MODE_ON, LED_MODE_WINK, LED_MODE_BLINK1, LED_MODE_BLINK2};

unsigned long int current_portion, last_portion;
unsigned char flash_page_cnt, finish_flash_page, data_cnt, GPIO_states[1] = {0b00000011};//GPIO0 - rst for HPM1000, GPIO1 - SSN for HPM1000
unsigned short flash_page_addr, flash_word, flash_byte;

unsigned char LED_mode = LED_MODE_OFF, LED_timeout; //variables for LED

tLMS_Ctrl_Packet *LMS_Ctrl_Packet_Tx = (tLMS_Ctrl_Packet*)tx_buff;
tLMS_Ctrl_Packet *LMS_Ctrl_Packet_Rx = (tLMS_Ctrl_Packet*)rx_buff;

//prototypes
//void Update_GPIO_outs (void);
void Set_LED_mode (unsigned char mode);
void SPI_write_DAC (unsigned char Byte);
void SPI_write_tuner (unsigned int data);

/** Timer 0 interrupt for periodic tasks
 *  8MHz/64/256 = 488.28125 Hz 
*/
ISR(TIMER0_OVF_vect)
{
	if (LED_mode == LED_MODE_WINK && LED_timeout) 
	{
		LED_timeout--;
		if(LED_timeout==0) 
		{
			LED_mode = LED_MODE_OFF;
			//green
			cbi(PORTB, LED_R);
			sbi(PORTB, LED_G); 
		}
	}
}

/** Main program entry point. This routine configures the hardware required by the application, then
 *  starts the scheduler to run the application tasks.
 */
int main(void)
{
	/* Disable watchdog if enabled by bootloader/fuses */
	MCUSR &= ~(1 << WDRF);
	wdt_disable();

	/* Disable clock division */
	clock_prescale_set(clock_div_1); // with 8MHz crystal, this means CLK=8000000

	/* Hardware Initialization */
	
	//set outputs directions
	// Prepare PortB for SPI - set PB0(^SS), PB1(SCK), PB2(MOSI) as output as well as all other pins except PB3(MISO)
	sbi (DDRB, MCU_SPI_DACA_SS);
	sbi (DDRB, MCU_SPI_SCLK);
	sbi (DDRB, MCU_SPI_MOSI);
	sbi (DDRB, MCU_SPI_MOSI);
	sbi (DDRB, MCU_SPI_DACB_SS);
	sbi (DDRB, LED_R);
	sbi (DDRB, LED_G);
	
	sbi (DDRC, VD_DRIVE_SELECT);
	sbi (DDRC, MCU_TUNERA_OUT_SS);
	sbi (DDRC, MCU_TUNERA_MID_SS);
	sbi (DDRC, MCU_TUNERA_IN_SS);
	
	sbi (DDRD, MCU_TUNERB_IN_SS);
	sbi (DDRD, MCU_TUNERB_MID_SS);
	sbi (DDRD, MCU_TUNERB_OUT_SS);

	//set outputs levels
	sbi (PORTB, MCU_SPI_DACA_SS);
	sbi (PORTB, MCU_SPI_DACB_SS);
	
	cbi (PORTC, MCU_TUNERA_OUT_SS);
	cbi (PORTC, MCU_TUNERA_MID_SS);
	cbi (PORTC, MCU_TUNERA_IN_SS);
	
	cbi (PORTD, MCU_TUNERB_IN_SS);
	cbi (PORTD, MCU_TUNERB_MID_SS);
	cbi (PORTD, MCU_TUNERB_OUT_SS);
	
	
	//spi registers defaults
	spi_reg[SPI_ADDR_0x00] = DEV_TYPE; //DEV_ID 
	spi_reg[SPI_ADDR_0x01] = HW_VER; //HW_VER  
	spi_reg[SPI_ADDR_0x02] = FW_VER; //FW_VER 

	
	spi_reg[SPI_ADDR_0x10] = 0b00000000; //GPIO
	
	spi_reg[SPI_ADDR_0x20] = 0b0000001000000000; //TUNER_A_IN
	spi_reg[SPI_ADDR_0x21] = 0b0000001000000000; //TUNER_A_MID
	spi_reg[SPI_ADDR_0x22] = 0b0000001000000000; //TUNER_A_OUT
	
	spi_reg[SPI_ADDR_0x23] = 0b0000001000000000; //TUNER_B_IN
	spi_reg[SPI_ADDR_0x24] = 0b0000001000000000; //TUNER_B_MID
	spi_reg[SPI_ADDR_0x25] = 0b0000001000000000; //TUNER_B_OUT
	
	spi_reg[SPI_ADDR_0x30] = 0x00; //DAC_A[7:0] (0V)
	spi_reg[SPI_ADDR_0x31] = 0x00; //DAC_B[7:0] (0V)
	
	Set_LED_mode (LED_MODE_WINK);
	
	/* 8-bit Timer0 Initialization */
	TCCR0B = (1 << CS01)|(1 << CS00);  // prescale Timer1 by CLK/64
	TIMSK0 = (1 << TOIE0); //Timer0 Overflow Interrupt Enable
	
	/* Ring buffer Initialization */
	Buffer_Initialize(&Rx_Buffer);
	Buffer_Initialize(&Tx_Buffer);

	/* Initialize Scheduler so that it can be used */
	Scheduler_Init();

	/* Initialize USB Subsystem */
	USB_Init();

	/* Scheduling - routine never returns, so put this last in the main function */
	Scheduler_Start();
}

/** Event handler for the USB_Connect event. This indicates that the device is enumerating via the status LEDs and
 *  starts the library USB task to begin the enumeration and USB management process.
 */
void EVENT_USB_Connect(void)
{
	/* Start USB management task */
	Scheduler_SetTaskMode(USB_USBTask, TASK_RUN);

	/* Indicate USB enumerating */
	//UpdateStatus(Status_USBEnumerating);
}

/** Event handler for the USB_Disconnect event. This stops the USB management and CDC management tasks.
 */
void EVENT_USB_Disconnect(void)
{
	/* Stop running CDC and USB management tasks */
	Scheduler_SetTaskMode(CDC_Task, TASK_STOP);
	Scheduler_SetTaskMode(USB_USBTask, TASK_STOP);
	
	/* Reset Tx and Rx buffers, device disconnected */
	Buffer_Initialize(&Rx_Buffer);
	Buffer_Initialize(&Tx_Buffer);
	
	Set_LED_mode (LED_MODE_WINK);
}

/** Event handler for the USB_ConfigurationChanged event. This is fired when the host set the current configuration
 *  of the USB device after enumeration - the device endpoints are configured and the CDC management task started.
 */
void EVENT_USB_ConfigurationChanged(void)
{
	/* Setup CDC Notification, Rx and Tx Endpoints */
	Endpoint_ConfigureEndpoint(CDC_NOTIFICATION_EPNUM, EP_TYPE_INTERRUPT,
		                       ENDPOINT_DIR_IN, CDC_NOTIFICATION_EPSIZE,
	                           ENDPOINT_BANK_SINGLE);

	Endpoint_ConfigureEndpoint(CDC_TX_EPNUM, EP_TYPE_BULK,
		                       ENDPOINT_DIR_IN, CDC_TXRX_EPSIZE,
	                           ENDPOINT_BANK_SINGLE);

	Endpoint_ConfigureEndpoint(CDC_RX_EPNUM, EP_TYPE_BULK,
		                       ENDPOINT_DIR_OUT, CDC_TXRX_EPSIZE,
	                           ENDPOINT_BANK_SINGLE);

	/* Start CDC task */
	Scheduler_SetTaskMode(CDC_Task, TASK_RUN);
	Scheduler_SetTaskMode(Main_Task, TASK_RUN);
}

/** Event handler for the USB_UnhandledControlPacket event. This is used to catch standard and class specific
 *  control requests that are not handled internally by the USB library (including the CDC control commands,
 *  which are all issued via the control endpoint), so that they can be handled appropriately for the application.
 */
void EVENT_USB_UnhandledControlPacket(void)
{
	uint8_t* LineCodingData = (uint8_t*)&LineCoding;

	/* Process CDC specific control requests */
	switch (USB_ControlRequest.bRequest)
	{
		case REQ_GetLineEncoding:
			if (USB_ControlRequest.bmRequestType == (REQDIR_DEVICETOHOST | REQTYPE_CLASS | REQREC_INTERFACE))
			{	
				/* Acknowledge the SETUP packet, ready for data transfer */
				Endpoint_ClearSETUP();

				/* Write the line coding data to the control endpoint */
				Endpoint_Write_Control_Stream_LE(LineCodingData, sizeof(LineCoding));
				
				/* Finalize the stream transfer to send the last packet or clear the host abort */
				Endpoint_ClearOUT();
			}
			
			break;
		case REQ_SetLineEncoding:
			if (USB_ControlRequest.bmRequestType == (REQDIR_HOSTTODEVICE | REQTYPE_CLASS | REQREC_INTERFACE))
			{
				/* Acknowledge the SETUP packet, ready for data transfer */
				Endpoint_ClearSETUP();

				/* Read the line coding data in from the host into the global struct */
				Endpoint_Read_Control_Stream_LE(LineCodingData, sizeof(LineCoding));

				/* Finalize the stream transfer to clear the last packet from the host */
				Endpoint_ClearIN();
				
				/* Reconfigure the USART with the new settings */
				//Reconfigure_SPI_for_LMS();
			}
	
			break;
		case REQ_SetControlLineState:
			if (USB_ControlRequest.bmRequestType == (REQDIR_HOSTTODEVICE | REQTYPE_CLASS | REQREC_INTERFACE))
			{				
				/* Acknowledge the SETUP packet, ready for data transfer */
				Endpoint_ClearSETUP();
				
				/* NOTE: Here you can read in the line state mask from the host, to get the current state of the output handshake
				         lines. The mask is read in from the wValue parameter in USB_ControlRequest, and can be masked against the
						 CONTROL_LINE_OUT_* masks to determine the RTS and DTR line states using the following code:
				*/

				/* Acknowledge status stage */
				while (!(Endpoint_IsINReady()));
				Endpoint_ClearIN();
			}
	
			break;
	}
	Set_LED_mode (LED_MODE_WINK);
}

/** Task to manage CDC data transmission and reception to and from the host, from and to the physical USART. */
TASK(CDC_Task)
{
	if (USB_IsConnected)
	{
		/* Select the Serial Rx Endpoint */
		Endpoint_SelectEndpoint(CDC_RX_EPNUM);
		
		/* Check to see if a packet has been received from the host */
		if (Endpoint_IsOUTReceived())
		{
			/* Read the bytes in from the endpoint into the buffer while space is available */
			while (Endpoint_BytesInEndpoint() && (BUFF_STATICSIZE - Rx_Buffer.Elements))
			{
				/* Store each character from the endpoint */
				Buffer_StoreElement(&Rx_Buffer, Endpoint_Read_Byte());
			}
			
			/* Check to see if all bytes in the current packet have been read */
			if (!(Endpoint_BytesInEndpoint()))
			{
				/* Clear the endpoint buffer */
				Endpoint_ClearOUT();
			}
		}
		
		/* Select the Serial Tx Endpoint */
		Endpoint_SelectEndpoint(CDC_TX_EPNUM);

		/* Check if the Tx buffer contains anything to be sent to the host */
		if (Tx_Buffer.Elements)
		{
			/* Wait until Serial Tx Endpoint Ready for Read/Write */
			while (!(Endpoint_IsReadWriteAllowed()));
			
			/* Write the bytes from the buffer to the endpoint while space is available */
			while (Tx_Buffer.Elements && (Endpoint_BytesInEndpoint() < CDC_TXRX_EPSIZE))
			{
				/* Write each byte retreived from the buffer to the endpoint */
				Endpoint_Write_Byte(Buffer_GetElement(&Tx_Buffer));
			}
			
			/* Remember if the packet to send completely fills the endpoint */
			bool IsFull = (Endpoint_BytesInEndpoint() == CDC_TXRX_EPSIZE);
			
			/* Send the data */
			Endpoint_ClearIN();

			/* If no more data to send and the last packet filled the endpoint, send an empty packet to release
			 * the buffer on the receiver (otherwise all data will be cached until a non-full packet is received) */
			if (IsFull && !(Tx_Buffer.Elements))
			{
				/* Wait until Serial Tx Endpoint Ready for Read/Write */
				while (!(Endpoint_IsReadWriteAllowed()));

				/* Send an empty packet to terminate the transfer */
				Endpoint_ClearIN();
			}
		}
	}
}


/** After the device is connected to a Host, this task will run periodically,
		even if the device is disconnected.
	Your application code should go here
*/
TASK(Main_Task)
{
	uint8_t block;
	uint16_t cmd_errors;

	if(Rx_Buffer.Elements == LMS_CTRL_PACKET_SIZE) //receveid full packet?
	{
		Set_LED_mode (LED_MODE_ON);
		cmd_errors = 0;
		for (count =0; count <LMS_CTRL_PACKET_SIZE; count++) //get all packets bytes
		{
			rx_buff[count] = Buffer_GetElement(&Rx_Buffer);
		}
		
		memset (tx_buff, 0, sizeof(tx_buff)); //fill whole tx buffer with zeros
		
		LMS_Ctrl_Packet_Tx->Header.Command = LMS_Ctrl_Packet_Rx->Header.Command;
		LMS_Ctrl_Packet_Tx->Header.Data_blocks = LMS_Ctrl_Packet_Rx->Header.Data_blocks;
		LMS_Ctrl_Packet_Tx->Header.Status = STATUS_BUSY_CMD;
		
		switch(LMS_Ctrl_Packet_Rx->Header.Command)
		{
			case CMD_GET_INFO:
				LMS_Ctrl_Packet_Tx->Data_field[0] = FW_VER;
				LMS_Ctrl_Packet_Tx->Data_field[1] = DEV_TYPE;
				LMS_Ctrl_Packet_Tx->Data_field[2] = LMS_PROTOCOL_VER;
				LMS_Ctrl_Packet_Tx->Data_field[3] = HW_VER;
				LMS_Ctrl_Packet_Tx->Data_field[4] = Exp_board_id;
				
				LMS_Ctrl_Packet_Tx->Header.Status = STATUS_COMPLETED_CMD;
				break;
				
			case CMD_BRDSPI16_WR:
				if(Check_many_blocks (4)) break;
				
				for(block = 0; block < LMS_Ctrl_Packet_Rx->Header.Data_blocks; block++)
				{
					cbi(LMS_Ctrl_Packet_Rx->Data_field[0 + (block * 2)], 7);  //clear write bit
					
					spi_reg_addr = ((LMS_Ctrl_Packet_Rx->Data_field[0 + (block * 4)] << 8) | LMS_Ctrl_Packet_Rx->Data_field[1 + (block * 4)]);
					spi_reg_data = ((LMS_Ctrl_Packet_Rx->Data_field[2 + (block * 4)] << 8) | LMS_Ctrl_Packet_Rx->Data_field[3 + (block * 4)]);
					
					switch (spi_reg_addr)
					{
						case 0x10: //GPIO
							spi_reg[SPI_ADDR_0x10] = spi_reg_data;
							
							if (spi_reg[SPI_ADDR_0x10] & 0x01) sbi(PORTC, VD_DRIVE_SELECT);
							else  cbi(PORTC, VD_DRIVE_SELECT);
							
							break;
							
						case 0x20: //TUNER_A_IN control
							spi_reg[SPI_ADDR_0x20] = spi_reg_data;
						
							cbi(PORTB, MCU_SPI_SCLK); //set Clock low
				
							sbi(PORTC, MCU_TUNERA_IN_SS); //select slave
							SPI_write_tuner ((1 << 1) | ((spi_reg[SPI_ADDR_0x20] & 0x7FF) << 2)); //Mode = Active, F1 - F11, endian corrected in function
							cbi(PORTC, MCU_TUNERA_IN_SS); //deselect slave
				
							break;
							
						case 0x21: //TUNER_A_MID control
							spi_reg[SPI_ADDR_0x21] = spi_reg_data;
							
							cbi(PORTB, MCU_SPI_SCLK); //set Clock low
				
							sbi(PORTC, MCU_TUNERA_MID_SS); //select slave
							SPI_write_tuner ((1 << 1) | ((spi_reg[SPI_ADDR_0x21] & 0x7FF) << 2)); //Mode = Active, F1 - F11, endian corrected in function
							cbi(PORTC, MCU_TUNERA_MID_SS); //deselect slave
							
							break;
							
						case 0x22: //TUNER_A_OUT control
							spi_reg[SPI_ADDR_0x22] = spi_reg_data;
							
							cbi(PORTB, MCU_SPI_SCLK); //set Clock low
				
							sbi(PORTC, MCU_TUNERA_OUT_SS); //select slave
							SPI_write_tuner ((1 << 1) | ((spi_reg[SPI_ADDR_0x22] & 0x7FF) << 2)); //Mode = Active, F1 - F11, endian corrected in function
							cbi(PORTC, MCU_TUNERA_OUT_SS); //deselect slave
							break;
							
						case 0x23: //TUNER_B_IN control
							spi_reg[SPI_ADDR_0x23] = spi_reg_data;
							
							cbi(PORTB, MCU_SPI_SCLK); //set Clock low
				
							sbi(PORTD, MCU_TUNERB_IN_SS); //select slave
							SPI_write_tuner ((1 << 1) | ((spi_reg[SPI_ADDR_0x23] & 0x7FF) << 2)); //Mode = Active, F1 - F11, endian corrected in function
							cbi(PORTD, MCU_TUNERB_IN_SS); //deselect slave
							break;
							
						case 0x24: //TUNER_B_MID control
							spi_reg[SPI_ADDR_0x24] = spi_reg_data;
							
							cbi(PORTB, MCU_SPI_SCLK); //set Clock low
				
							sbi(PORTD, MCU_TUNERB_MID_SS); //select slave
							SPI_write_tuner ((1 << 1) | ((spi_reg[SPI_ADDR_0x24] & 0x7FF) << 2)); //Mode = Active, F1 - F11, endian corrected in function
							cbi(PORTD, MCU_TUNERB_MID_SS); //deselect slave
							break;
							
						case 0x25: //TUNER_B_OUT control
							spi_reg[SPI_ADDR_0x25] = spi_reg_data;
							
							cbi(PORTB, MCU_SPI_SCLK); //set Clock low
				
							sbi(PORTD, MCU_TUNERB_OUT_SS); //select slave
							SPI_write_tuner ((1 << 1) | ((spi_reg[SPI_ADDR_0x25] & 0x7FF) << 2)); //Mode = Active, F1 - F11, endian corrected in function
							cbi(PORTD, MCU_TUNERB_OUT_SS); //deselect slave
							break;
							
						case 0x30: //DAC_A control
							spi_reg[SPI_ADDR_0x30] = spi_reg_data;
							
							sbi(PORTB, MCU_SPI_SCLK); //set Clock high
							
							cbi(PORTB, MCU_SPI_DACA_SS); //select slave
							SPI_write_DAC ((unsigned char)spi_reg[SPI_ADDR_0x30] >> 2);  //POWER-DOWN MODE = NORMAL OPERATION (MSB bits = 00) + MSB data
							SPI_write_DAC ((unsigned char)spi_reg[SPI_ADDR_0x30] << 6); //LSB data
							sbi(PORTB, MCU_SPI_DACA_SS); //deselect slave
							
							break;
							
						case 0x31: //DAC_B control
							spi_reg[SPI_ADDR_0x31] = spi_reg_data;
							
							sbi(PORTB, MCU_SPI_SCLK); //set Clock high
							
							cbi(PORTB, MCU_SPI_DACB_SS); //select slave
							SPI_write_DAC ((unsigned char)spi_reg[SPI_ADDR_0x31] >> 2);  //POWER-DOWN MODE = NORMAL OPERATION (MSB bits = 00) + MSB data
							SPI_write_DAC ((unsigned char)spi_reg[SPI_ADDR_0x31] << 6); //LSB data
							sbi(PORTB, MCU_SPI_DACB_SS); //deselect slave
							
							break;
							
						default:
							cmd_errors++;
							break;
					}
				}
				
				if(cmd_errors) LMS_Ctrl_Packet_Tx->Header.Status = STATUS_ERROR_CMD;
 	 			else LMS_Ctrl_Packet_Tx->Header.Status = STATUS_COMPLETED_CMD;
				break;
				
			case CMD_BRDSPI16_RD:
				if(Check_many_blocks (4)) break;
				
				for(block = 0; block < LMS_Ctrl_Packet_Rx->Header.Data_blocks; block++)
				{
					cbi(LMS_Ctrl_Packet_Rx->Data_field[0 + (block * 2)], 7);  //clear write bit
					
					spi_reg_addr = ((LMS_Ctrl_Packet_Rx->Data_field[0 + (block * 4)] << 8) | LMS_Ctrl_Packet_Rx->Data_field[1 + (block * 4)]);
					spi_reg_data = ((LMS_Ctrl_Packet_Rx->Data_field[2 + (block * 4)] << 8) | LMS_Ctrl_Packet_Rx->Data_field[3 + (block * 4)]);
					
					switch (spi_reg_addr)
					{
						case 0x00: //Device ID
							spi_reg_data = spi_reg[SPI_ADDR_0x00];
							break;

						case 0x01: //Hardware version
							spi_reg_data = spi_reg[SPI_ADDR_0x01];
							break;
						
						case 0x02: //Firmware version
							spi_reg_data = spi_reg[SPI_ADDR_0x02];
							break;
						
						case 0x10: //GPIO
							spi_reg_data = spi_reg[SPI_ADDR_0x10];
							break;
							
						case 0x20: //TUNER_A_IN control
							spi_reg_data = spi_reg[SPI_ADDR_0x20];
							break;
							
						case 0x21: //TUNER_A_MID control
							spi_reg_data = spi_reg[SPI_ADDR_0x21];
							break;
							
						case 0x22: //TUNER_A_OUT control
							spi_reg_data = spi_reg[SPI_ADDR_0x22];
							break;
							
						case 0x23: //TUNER_B_IN control
							spi_reg_data = spi_reg[SPI_ADDR_0x23];
							break;
							
						case 0x24: //TUNER_B_MID control
							spi_reg_data = spi_reg[SPI_ADDR_0x24];
							break;
							
						case 0x25: //TUNER_B_OUT control
							spi_reg_data = spi_reg[SPI_ADDR_0x25];
							break;
							
						case 0x30: //DAC_A control
							spi_reg_data = spi_reg[SPI_ADDR_0x30];
							break;
							
						case 0x31: //DAC_B control
							spi_reg_data = spi_reg[SPI_ADDR_0x31];
							break;
						
						default: //error
							spi_reg_data = 0xEEEE;
							cmd_errors++;
							break;
					}
					

					LMS_Ctrl_Packet_Tx->Data_field[2 + (block * 4)] = (spi_reg_data >> 8) & 0xFF; //reg data MSB
					LMS_Ctrl_Packet_Tx->Data_field[3 + (block * 4)] = spi_reg_data & 0xFF; //reg data LSB
				}

				if(cmd_errors) LMS_Ctrl_Packet_Tx->Header.Status = STATUS_ERROR_CMD;
 	 			else LMS_Ctrl_Packet_Tx->Header.Status = STATUS_COMPLETED_CMD;
				break;				
				
			default:
				
				LMS_Ctrl_Packet_Tx->Header.Status = STATUS_UNKNOWN_CMD;
				break;
		}
		
		//send prepared tx buffer to PC
		for (count =  0; count < LMS_CTRL_PACKET_SIZE; count++)
		{
			Buffer_StoreElement(&Tx_Buffer, tx_buff[count]);
		}
		Set_LED_mode (LED_MODE_WINK);
	}
}


/**	Inaccurate software delay function to get required delay in microseconds. */
void Delay_us (unsigned int cycles)
{
	while (cycles--)
	{
		asm volatile("nop"); 
		asm volatile("nop"); 
		asm volatile("nop");
	}	
}

/**	This function checks if all blocks could fit in data field.
*	If blocks will not fit, function returns TRUE. */
unsigned char Check_many_blocks (unsigned char block_size)
{
	if (LMS_Ctrl_Packet_Rx->Header.Data_blocks > (sizeof(LMS_Ctrl_Packet_Tx->Data_field)/block_size)) 
	{
		LMS_Ctrl_Packet_Tx->Header.Status = STATUS_BLOCKS_ERROR_CMD;
		return TRUE;
	}
	else return FALSE;
}

/**	Function to control LED mode. */
void Set_LED_mode (unsigned char mode)
{
	LED_mode = mode;//save new LED mode
	
	switch (LED_mode)
	{
		case LED_MODE_OFF: 
			//green
			cbi(PORTB, LED_R);
			sbi(PORTB, LED_G); 
			break;
			
		case LED_MODE_ON:
			//red
			sbi(PORTB, LED_R);
			cbi(PORTB, LED_G);
			break;
		
		case LED_MODE_WINK:
			//red
			sbi(PORTB, LED_R);
			cbi(PORTB, LED_G);
			LED_timeout = 100; //set LED timeout
			break;
	}
}

void SPI_write_DAC (unsigned char Byte)
{
	unsigned char i;
	
	for(i = 8; i > 0; i--) //LSB First
	{
		if((Byte >> (i-1))&1)	//if current bit is 1
		{
			sbi(PORTB, MCU_SPI_MOSI); //Set Output High
		}
		else
		{
			cbi(PORTB, MCU_SPI_MOSI); //Set Output Low
		}	
		
		cbi(PORTB, MCU_SPI_SCLK);	//set Clock low
		sbi(PORTB, MCU_SPI_SCLK);	//set Clock high
	}
}

void SPI_write_tuner (unsigned int data)
{
	unsigned char i;
	unsigned int control_data = 0b01010000001000; //Write mode + Address(Slave Type)[3:0] + Address(Slave Identifier)[4:0] + Address(Slave SPI register)[4:0]
	
	///write control data (bit 29-16) 
	for(i = 14; i > 0; i--) //MSB First
	{
		if((control_data >> (i-1))&1)	//if current bit is 1
		{
			sbi(PORTB, MCU_SPI_MOSI); //Set Output High
		}
		else
		{
			cbi(PORTB, MCU_SPI_MOSI); //Set Output Low
		}	
		
		sbi(PORTB, MCU_SPI_SCLK);	//set Clock high
		cbi(PORTB, MCU_SPI_SCLK);	//set Clock low

	}
	
	/*sbi(PORTD, DAC_CS_B);//sbi(PORTC, TUNERA_IN_SS);
	cbi(PORTD, DAC_CS_B);//sbi(PORTC, TUNERA_IN_SS);*/
	
	//data = 0b1101101101101111;

	//write config data  (bit 15-0) (Mode, F1-F11, other bits)
	/*for(i = 16; i > 0; i--) //MSB First
	{
		if((data >> (i-1))&1)	//if current bit is 1
		{
			sbi(PORTB, MCU_SPI_MOSI); //Set Output High
		}
		else
		{
			cbi(PORTB, MCU_SPI_MOSI); //Set Output Low
		}	
		
		sbi(PORTB, MCU_SPI_CLK);	//set Clock high
		cbi(PORTB, MCU_SPI_CLK);	//set Clock low
	}*/
	
	for(i = 0; i < 16; i++) //LSB First, endian must be coorected in "data" variable
	{
		if((data >> i)&1)	//if current bit is 1
		{
			sbi(PORTB, MCU_SPI_MOSI); //Set Output High
		}
		else
		{
			cbi(PORTB, MCU_SPI_MOSI); //Set Output Low
		}	
		
		sbi(PORTB, MCU_SPI_SCLK);	//set Clock high
		cbi(PORTB, MCU_SPI_SCLK);	//set Clock low
	}
}

