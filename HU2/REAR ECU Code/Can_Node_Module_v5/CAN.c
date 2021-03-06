/* CAN.C
Contains the basic CAN functions for receiving and sending data.
Also contains the interrupt for CAN rx.
*/

#ifndef _CANc_
#define _CANc_

//***** Defines ***********************************************
#include "Defines.h"

//***** Libraries *********************************************
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "CAN.h"
#include "ExternalInterrupt.h"

uint8_t receive_data[8];
uint8_t transmit_data[8];

bool predison = false;

//***** Reception ISR **********************************
ISR(CANIT_vect)
{
	int8_t length;
	
	CANPAGE = ( 0 << MOBNB3 ) | ( 0 << MOBNB2 ) | ( 0 << MOBNB1 ) | ( 1 << MOBNB0 ); // select CANMOB 0001 = MOB1
	
	uint16_t rx_addr = (CANIDT1 << 3) | ((CANIDT2 & 0b11100000) >> 5);
	
	cantimer = 0;
	
	if(rx_addr == ECU2ID)
	{
		length = ( CANCDMOB & 0x0F );	// DLC, number of bytes to be received
		for ( int8_t i = 0; i < length; i++ ){
			receive_data[i] = CANMSG; // Get data, INDX auto increments CANMSG
		}
		
		uint8_t j = 0;
		if (receive_data[0] == CAN_REQUEST_DATA)
		{
			//Don't do anything anymore :)
		}
		else
		{
			uint8_t i = 0;
			while(i < length)
			{
				//Run enable
				if (receive_data[i] == RUN_ENABLE)
				{
					DDRC	|= (1 << PC0);
					if(receive_data[i+1])
					{
						PORTC	|= (1 << PC0);
					}
					else 
					{
						PORTC	&= ~(1 << PC0);
					}
					transmit_data[j++] = RUN_ENABLE;
					i++;
				}
				
				//Motor controller
				if (receive_data[i] == MOTOR_CONTROLLER)
				{
					DDRC	|= (1 << PC1);
					if(receive_data[i+1])
					{
						PORTC	|= (1 << PC1);
					}
					else
					{
						PORTC	&= ~(1 << PC1);
					}
					transmit_data[j++] = MOTOR_CONTROLLER;
					i++;
				}
				
				//Brakelight
				if (receive_data[i] == BRAKELIGHT)
				{
					DDRC	|= (1 << PC4);
					if(receive_data[i+1])
					{
						PORTC	|= (1 << PC4);
					}
					else
					{
						PORTC	&= ~(1 << PC4);
					}
					transmit_data[j++] = BRAKELIGHT;
					i++;
				}
				
				//Predischarge
				if (receive_data[i] == PRE_DISCHARGE)
				{
					DDRC |= (1 << PC3);
				
					if(receive_data[i+1])
					{
						predison = true;
						PORTC	|= (1 << PC3);
					}
					else
					{
						predison = false;
						PORTC	&= ~(1 << PC3);
					}
					transmit_data[j++] = PRE_DISCHARGE;
					i++;
				}
				
				//Main relais(which will not turn on if the predischarge is not on)
				if (receive_data[i] == MAINRELAIS)
				{
					DDRC	|= (1 << PC2);
					if(receive_data[i+1] && predison)
					{
						PORTC	|= (1 << PC2);
					}
					else
					{
						PORTC	&= ~(1 << PC2);
					}
					transmit_data[j++] = MAINRELAIS;
					i++;
				}
				
				//Pump
				if (receive_data[i] == PUMP)
				{
					DDRC	|= (1 << PC5);
					pump_pwm = receive_data[i+i];
					if(pump_pwm > 0)
					{
						PORTC |= (1<<PC5);
					}
					else
					{
						PORTC &= ~(1<<PC5);
					}
					transmit_data[j++] = PUMP;
					i++;
				}
				i++;
			}
		}
		can_tx(MASTERID, j); //Transmit data depending on the number of message received
	}
	
	CANSTMOB = 0x00; // Clear RXOK flag
	CANCDMOB = (( 1 << CONMOB1 ) | ( 0 << IDE ) | ( 3 << DLC0)); //CAN MOb Control and DLC Register: (1<<CONMOB1) = enable reception. (0<<IDE) = can standard rev 2.0A ( id length = 11 bits), (3 << DLC0) 3 Bytes in the data field of the message.

	CANPAGE = ( 0 << MOBNB3 ) | ( 0 << MOBNB2 ) | ( 0 << MOBNB1 ) | ( 0 << MOBNB0 ); // select 0000 = CANMOB0
	
}

ISR(TIMER2_COMP_vect)
{
	DDRD &= ~(1<<PD7);
	if((PIND & (1 << PD7)))
	{
		transmit_data[0] = 0xFF;
		transmit_data[1] = 0xFF;
	}
	else
	{
		transmit_data[0] = 0x00;
		transmit_data[1] = 0x00;
	}
	CANPAGE = ( 0 << MOBNB3 ) | ( 0 << MOBNB2 ) | ( 0 << MOBNB1 ) | ( 1 << MOBNB0 ); //Select MOb 1
	can_tx(FUNCTION, 8);
}



//***** CAN ialization *****************************************************
void can_init(uint16_t Baud)
{
	CANGCON = ( 1 << SWRES );   // Software reset
	
	CANTCON = 0x00;       // CAN timing prescaler set to 0;
	
	if (Baud == 1000)
	{
		CANBT1 = 0x00;    // Set baud rate to 500kb (assuming 16Mhz IOclk)
		CANBT2 = 0x0C;    // "
		CANBT3 = 0x36; 	  // ""
	}
	if (Baud == 500)
	{
		CANBT1 = 0x02;    // Set baud rate to 500kb (assuming 16Mhz IOclk)
		CANBT2 = 0x0C;    // "
		CANBT3 = 0x37; 	  // ""
	}
	if (Baud == 250)
	{
		CANBT1 = 0x0E;   // Set baud rate to 250kb (assuming 16Mhz IOclk)
		CANBT2 = 0x04;   // "
		CANBT3 = 0x13; 	 // ""
	}
	if ( Baud == 125)
	{
		CANBT1 = 0x06;  // Set baud rate to 125kb (assuming 16Mhz IOclk)
		CANBT2 = 0x0C;  // "
		CANBT3 = 0x37;	// ""
	}
	

	for ( int8_t mob=0; mob<14; mob++ )
	{
		CANPAGE = ( mob << 4 );  // Selects Message Object 0-14
		CANCDMOB = 0x00;    // Disable mob
		CANSTMOB = 0x00;    // Clear mob status register;
	}
	
	CANIE2 = (( 1 << IEMOB1 ) | ( 0 << IEMOB0 ));  // IEMOB1 = MOB1 Enable interrupts on mob1 for reception and transmission
	CANGIE = ( 1 << ENIT ) | ( 1 << ENRX ) | ( 0 << ENTX );   // Enable interrupts on receive
	CANGCON |= ( 1 << 1 );	// Enable mode. CAN channel enters in enable mode once 11 recessive bits have been read
	
}

//***** CAN Creating RX *****************************************************
void can_rx(uint16_t NODE_ID)
{	
	CANPAGE = ( 0 << MOBNB3 ) | ( 0 << MOBNB2 ) | ( 0 << MOBNB1 ) | ( 1 << MOBNB0 ); // select 0001 = CANMOB1
	
	CANIDT1 = NODE_ID >> 3; // Receive Address
	CANIDT2 = NODE_ID << 5; //
	CANIDT3 = 0x00; 		// ""
	CANIDT4 = 0x00; 		// ""
	
	CANIDM1 = 0b11111111;   // Receive Address
	CANIDM2 = 0b11100000;	//

	CANCDMOB = (( 1 << CONMOB1 ) | ( 0 << IDE ) | ( 3 << DLC0));  // Enable Reception | 11 bit | IDE DLC8
}

//***** CAN Creating TX *****************************************************
void can_tx(uint16_t Address, uint8_t DLC)
{
	CANPAGE = ( 0 << MOBNB3 ) | ( 0 << MOBNB2 ) | ( 0 << MOBNB1 ) | ( 0 << MOBNB0 ); // select 0000 = CANMOB0

	while ( CANEN2 & ( 1 << ENMOB0 ) ); // Wait for MOb 0 to be free
	
	CANSTMOB = 0x00;   // Clear mob status register
	
	CANIDT4 = 0x00;     		//
	CANIDT3 = 0x00;				//
	CANIDT2 = Address << 5;		//
	CANIDT1 = Address >> 3;		//
	
	for ( int8_t i = 0; i < 8; i++ )
	{
		CANMSG = transmit_data[i]; //CAN Data Message Register: setting the data in the message register
	}
	
	CANCDMOB = (( 1 << CONMOB0 ) | ( 0 << IDE ) | ( DLC << DLC0)); //CAN MOb Control and DLC Register: (1<<CONMOB1) = enable reception. (0<<IDE) = can standard rev 2.0A ( id length = 11 bits), (DLC << DLC0) Set *DLC* Bytes in the data field of the message.
	
	int i = 0;
	while ( ! ( CANSTMOB & ( 1 << TXOK ) ) )
	{
		//if TX_OK is not set in 6 sec function automatically breaks.
		i++;
		_delay_ms(1);
		if (i > 300) break;
	}	
	// wait for TXOK flag set
	
	CANCDMOB = 0x00; //Clear CAN Mob Control and DLC Register

	CANSTMOB = 0x00; // Clear TXOK flag
	
	CANPAGE = ( 0 << MOBNB3 ) | ( 0 << MOBNB2 ) | ( 0 << MOBNB1 ) | ( 1 << MOBNB0 ); // select 0001 = MOB1
}

#endif