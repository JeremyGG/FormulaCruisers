#include <avr/io.h>
#include <avr/interrupt.h>
#include "Defines.h"
#include "Data.h"
#include "CAN.h"
#include "Error.h"

volatile uint16_t waiting = 1;

void wait_for_rx()
{
	waiting = 1;
	while(waiting > 1 && waiting < RX_WAIT_LIMIT) waiting++;
}

void data_send_ecu(uint8_t node, uint8_t data)
{
	TransmitData[0] = node;
	TransmitData[1] = data;
	can_tx(ECU2ID, 2);
	
	wait_for_rx();
}
void data_send_motor(uint8_t header, uint8_t data, int32_t mul, uint16_t node)
{
	int32_t val = (mul * data) / 100;
	data_send16(header, (uint16_t)val, node);
}
void data_send_motor_d(uint8_t header, double data, int32_t mul, uint16_t node)
{
	int32_t val = (mul * data) / 100;
	data_send16(header, (uint16_t)val, node);
}

ISR(CANIT_vect)
{
	CANPAGE = ( 0 << MOBNB3 ) | ( 0 << MOBNB2 ) | ( 0 << MOBNB1 ) | ( 1 << MOBNB0 ); // select CANMOB 0001 = MOB1

	uint16_t ReceiveAddress = (CANIDT1 << 3) | ((CANIDT2 & 0b11100000) >> 5);

	if(ReceiveAddress == MASTERID)
	{
		waiting = 0;
		
		uint8_t length = ( CANCDMOB & 0x0F );
		
		//for ( int8_t i = 0; i < length; i++ ) ReceiveData[i] = CANMSG;
		//Loop unrolling
		ReceiveData[0] = CANMSG;
		if(length > 1)
		{
			ReceiveData[1] = CANMSG;
			if(length > 2)
			{
				ReceiveData[2] = CANMSG;
				if(length > 3)
				{
					//Should never happen but just in case
					for (uint8_t i = 3; i < length; i++)
						ReceiveData[i] = CANMSG;
					_errorcode = ERROR_UNKNOWN;
				}
			}
		}
		
		switch(ReceiveData[0])
		{
			case GAS_1:
				gas1 = (ReceiveData[1] +  (ReceiveData[2] << 8));
			
				//Bound checking while fixing range
				gas1perc = (gas1 < GAS1MIN) ? 0 : ((gas1 > GAS1MAX) ? (GAS1MAX - GAS1MIN) : (gas1 - GAS1MIN));
				gas1eng = (gas1perc * engine_max_perc) / (double)(GAS1MAX - GAS1MIN);
				gas1perc = (gas1perc * 100) / (GAS1MAX - GAS1MIN);
				break;

			case GAS_2:
				gas2 = (ReceiveData[1] + (ReceiveData[2] << 8));
				gas2perc = (gas2 < GAS2MIN) ? 0 : ((gas2 > GAS2MAX) ? (GAS2MAX - GAS2MIN) : (gas2 - GAS2MIN));
				gas2perc = (gas2perc * 100) / (GAS2MAX - GAS2MIN);
				break;

			case BRAKE:
				brake = (ReceiveData[1] + (ReceiveData[2] << 8));
				brakeperc = (brake < BRAKEMIN) ? 0 : ((brake > BRAKEMAX) ? (BRAKEMAX - BRAKEMIN) : (brake - BRAKEMIN));
				brakeperc = (brakeperc * 100) / (BRAKEMAX - BRAKEMIN);
				break;
			
			case SHUTDOWN:
				shutdownon = ReceiveData[1] ? 1 : 0;
				break;
				
			case RPM_FRONT_LEFT:
				//rpm_fl = (ReceiveData[1] + (ReceiveData[2] << 8));
				rpm_fl++;
				break;
				
			case RPM_FRONT_RIGHT:
				//rpm_fr = (ReceiveData[1] + (ReceiveData[2] << 8));
				rpm_fr++;
				break;
				
			case RPM_BACK_LEFT:
				rpm_bl++;
				break;
				
			case RPM_BACK_RIGHT:
				rpm_br++;
				break;
				
			case STEERING_POS:
				steerpos = (ReceiveData[1] + (ReceiveData[2] << 8)) - STEER_MIDDLE;
				break;
				
			case FLOW_LEFT:
				flowleft = (ReceiveData[1] + (ReceiveData[2] << 8));
				break;
				
			case FLOW_RIGHT:
				flowleft = (ReceiveData[1] + (ReceiveData[2] << 8));
				break;
				
			case AMSSHUTDOWN:
				ams_shutdown = _HIGH;
				break;
			
			case IMDSHUTDOWN:
				imd_shutdown = _HIGH;
				break;
		}
	}
	
	CANSTMOB = 0x00; // Clear RXOK flag
	CANCDMOB = (( 1 << CONMOB1 ) | ( 0 << IDE ) | ( 3 << DLC0)); //CAN MOb Control and DLC Register: (1<<CONMOB1) = enable reception. (0<<IDE) = can standard rev 2.0A ( id length = 11 bits), (3 << DLC0) 3 Bytes in the data field of the message.

	CANPAGE = ( 0 << MOBNB3 ) | ( 0 << MOBNB2 ) | ( 0 << MOBNB1 ) | ( 0 << MOBNB0 ); // select 0000 = CANMOB0
}
