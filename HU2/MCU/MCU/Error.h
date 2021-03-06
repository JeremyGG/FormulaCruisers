/* ERROR.H
Header file for Error.c
Contains the definitions for all possible errors
*/

#ifndef _ERROR_H
#define _ERROR_H

enum _error
{
	ERROR_NONE,				//When there isn't actually an error. Best case scenario

	ERROR_GAS_DISCREPANCY,	//When the two different gas sensors give wildly different values.
	ERROR_GASBRAKE,			//When both gas and brake are pressed at the same time(is this really an error?)

	ERROR_GAS1RANGE,		//When gas sensor 1 is far out of range
	ERROR_GAS2RANGE,		//When gas sensor 2 is far out of range
	ERROR_BRAKERANGE,		//When the brake sensor is far out of range
	ERROR_STEERRANGE,		//When the steering sensor is far out of range
	
	ERROR_GAS1SENSOR,		//When gas sensor 1 is not giving a (correct) readout
	ERROR_GAS2SENSOR,		//When gas sensor 2 is not giving a (correct) readout
	ERROR_BRAKESENSOR,		//When the brake sensor is not giving a (correct) readout
	ERROR_STEERSENSOR,		//When the steering sensor is not giving a (correct) readout
	
	ERROR_PUMPFLOW,			//When the pump is on but there is no flow
	ERROR_PUMPTEMP,			//When the pump gets too hot
	
	ERROR_SHUTDOWN,			//When a shutdown is called while predischarging or driving
	
	ERROR_NODATA,			//When a CAN node has not sent data after NODATA_TIMER ticks
	
	ERROR_CAN_BIT,
	ERROR_CAN_STUFF,		//See AT90CAN128 datasheet page 261/262 for quick descriptions of these CAN errors
	ERROR_CAN_CRC,
	ERROR_CAN_FORM,
	ERROR_CAN_ACK,

	ERROR_SD_INIT_RESET,	//When the SD card could not be reset
	ERROR_SD_INIT_READY,	//When the SD card is not ready
	ERROR_SD_BLOCK,			//When a block error occurs
	ERROR_SD_NOTREADY,		//When the SD card is not ready
	ERROR_SD_READ,			//When an SD read error occurs
	ERROR_SD_WRITE,			//When an SD write error occurs
	ERROR_SD_SIZE,			//Writing too much data to SD card
	
	ERROR_SD_WRITE_IDLE,
	ERROR_SD_WRITE_ERASE_RST,
	ERROR_SD_WRITE_ILLEGAL,
	ERROR_SD_WRITE_CRC,
	ERROR_SD_WRITE_ERASE_SEQ,
	ERROR_SD_WRITE_ADDRESS,
	ERROR_SD_WRITE_PARAMETER,
	
	ERROR_UNKNOWN,			//For unknown errors
};

extern volatile uint16_t gas1, gas2, brake, gas1perc, gas2perc, brakeperc, flowleft, flowright, steerposm;
extern volatile enum _error _errorcode;

extern uint16_t GAS1MIN, GAS1MAX, GAS2MIN, GAS2MAX, BRAKEMIN, BRAKEMAX;

char* get_error(enum _error e);
void e_checkranges(void);
void e_checkdiscrepancy(void);
void e_checksensors(void);
void e_checkflow(void);
void e_checkCAN(void);

#endif