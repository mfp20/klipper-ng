#ifndef PROTOCOL_FIRMATA_USER_H
#define PROTOCOL_FIRMATA_USER_H

//
#define CMD_EID_USER_CHANGE_ENCODING	0x01	// user defined command: change 7bit encoding (normal<->compat)
#define CMD_EID_USER_REPORT_ENCODING	0x02	// user defined command: report current 7bit encoding (normal||compat)
#define CMD_EID_USER_SIGLAG				0x03	// signal tick took more than TICKRATE to complete
#define CMD_EID_USER_DELAYALL			0x04	// delay all tasks
#define CMD_EID_USER5					0x05	// reserved for user defined commands
#define CMD_EID_USER6					0x06	// reserved for user defined commands
#define CMD_EID_USER7					0x07	// reserved for user defined commands
#define CMD_EID_USER8					0x08	// reserved for user defined commands
#define CMD_EID_USER9					0x09	// reserved for user defined commands
#define CMD_EID_USERA					0x0A	// reserved for user defined commands
#define CMD_EID_USERB					0x0B	// reserved for user defined commands
#define CMD_EID_USERC					0x0C	// reserved for user defined commands
#define CMD_EID_USERD					0x0D	// reserved for user defined commands
#define CMD_EID_USERE					0x0E	// reserved for user defined commands
#define CMD_EID_USERF					0x0F	// reserved for user defined commands

// custom 'realtime mode', not in firmata protocol
#define CMD_REALTIME_SYN				1 // start of synchronous operation
#define CMD_REALTIME_ACK				2 // confirm
#define CMD_REALTIME_NACK				3 // problem
#define CMD_REALTIME_FIN				4 // end of synchronous operation
#define CMD_REALTIME_TIME_US			5 // tell me your microseconds (MSB only)
#define CMD_REALTIME_TIME_RESET			6 // reset your microseconds

#endif

