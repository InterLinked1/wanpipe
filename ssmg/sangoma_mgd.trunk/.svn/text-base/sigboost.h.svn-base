/****************************************************************************
 * sigboost.h
 *
 * Definitions for the sigboost interface.
 *
 * Copyright (C) 2005  Xygnada Technology, Inc.
 * 
****************************************************************************/
#define SIGBOOST_VERSION	3.0
/*

Section I - Configuration

A. SS7 application

media_gw******ss7boost---ss7box======pstn
   |                                  |
   +------------------voice-----------+

B. Q.931 application

media_gw******q931boost---q921_if====pstn
   |                                  |
   +------------------voice-----------+

Section II - Messages, Formats, and Codes

1.1 call origination from media_gw to PSTN

media_gw -> sigboost
	event id : CALL_START
	trunk group: number associated with a group of spans to a particular
	  destination
	called number digit count: number of digits in dialed number
	called number: dialed number
	calling number digit count: number is digits in number of party that dialed
	calling number: number of party that dialed
	destination id: common indicator value (see note below)
	woomera call setup id: unique number assigned by woomera to this call

sigboost -> media_gw
	event_id: CALL_START_ACK
	woomera call setup id: unique number assigned by woomera to this call
	span id: common indicator of span to use on call
	channel id: common indicator of channel of span to use on call

sigboost -> media_gw
	event_id: CALL_START_NACK
	woomera call setup id: unique number assigned by woomera to this call
	cause: indication of why nack is being sent

1.2 call answered at PSTN 

sigboost -> media_gw
	event_id: CALL_ANSWERED
	span id: common indicator of span to use on call
	channel id: common indicator of channel of span to use on call

1.3 call stopped at media_gw

media_gw -> sigboost
	event_id: CALL_STOPPED
	cause: indication of why call was stopped
	span id: common indicator of span to use on call
	channel id: common indicator of channel of span to use on call

1.4 call stopped at PSTN

sigboost -> media_gw
	event_id: CALL_STOPPED
	cause: indication of why call was stopped
	span id: common indicator of span to use on call
	channel id: common indicator of channel of span to use on call

2.1 call origination from PSTN to media_gw

sigboost -> media_gw
	event id : CALL_START
	called number digit count: number of digits in dialed number
	called number: dialed number
	calling number digit count: number is digits in number of party that dialed
	calling number: number of party that dialed
	span id: common indicator of span to use on call
	channel id: common indicator of channel of span to use on call

2.2 call answered by woomer_ss7

media_gw -> sigboost
	event_id: CALL_ANSWERED
	span id: common indicator of span to use on call
	channel id: common indicator of channel of span to use on call

2.3 call stopped at media_gw

media_gw -> sigboost
	event_id: CALL_STOPPED
	cause: indication of why call was stopped
	span id: common indicator of span to use on call
	channel id: common indicator of channel of span to use on call

2.4 call stopped at PSTN

sigboost -> media_gw
	event_id: CALL_STOPPED
	cause: indication of why call was stopped
	span id: common indicator of span to use on call
	channel id: common indicator of channel of span to use on call

Section III - Procedures

See sw7.c for an example of how to use this message interface.
	
*/
#ifndef _SIGBOOST_H_
#define _SIGBOOST_H_

#include <stdint.h>

#ifndef PACKED
#define PACKED __attribute__ ((packed))
#endif

enum	e_sigboost_event_id_values
{
	SIGBOOST_EVENT_CALL_START			= 0x80,
	SIGBOOST_EVENT_CALL_START_ACK			= 0x81,
	SIGBOOST_EVENT_CALL_START_NACK			= 0x82,
	SIGBOOST_EVENT_CALL_START_NACK_ACK		= 0x83,
	SIGBOOST_EVENT_CALL_ANSWERED			= 0x84,
	SIGBOOST_EVENT_CALL_STOPPED			= 0x85,
	SIGBOOST_EVENT_CALL_STOPPED_ACK			= 0x86,
	SIGBOOST_EVENT_SYSTEM_RESTART			= 0x87,
	SIGBOOST_EVENT_HEARTBEAT			= 0x88,
};

enum	e_sigboost_release_cause_values
{
	SIGBOOST_RELEASE_CAUSE_UNDEFINED		= 0x00,
	SIGBOOST_RELEASE_CAUSE_NORMAL			= 0x90,
	SIGBOOST_RELEASE_CAUSE_BUSY			= 0x91,
	SIGBOOST_RELEASE_CAUSE_CALLED_NOT_EXIST		= 0x92,
	SIGBOOST_RELEASE_CAUSE_CIRCUIT_RESET		= 0x93,
	SIGBOOST_RELEASE_CAUSE_NOANSWER			= 0x94
};

enum	e_sigboost_call_setup_ack_nack_cause_values
{
	SIGBOOST_CALL_SETUP_NACK_CKT_START_TIMEOUT	= 0x11,
	SIGBOOST_CALL_SETUP_NACK_ALL_CKTS_BUSY		= 0x12,
	SIGBOOST_CALL_SETUP_NACK_CALLED_NUM_TOO_BIG	= 0x13,
	SIGBOOST_CALL_SETUP_NACK_CALLING_NUM_TOO_BIG	= 0x14,
	SIGBOOST_CALL_SETUP_NACK_CALLED_NUM_TOO_SMALL	= 0x15,
	SIGBOOST_CALL_SETUP_NACK_CALLING_NUM_TOO_SMALL	= 0x16,
};

#define MAX_DIALED_DIGITS	31

/* Next two defines are used to create the range of values for call_setup_id
 * in the t_sigboost structure.
 * 0..((CORE_MAX_SPANS * CORE_MAX_CHAN_PER_SPAN) - 1) */
#define CORE_MAX_SPANS 		200
#define CORE_MAX_CHAN_PER_SPAN 	30

typedef struct
{
	uint32_t	event_id					PACKED;
	uint32_t	seqno						PACKED;
	uint32_t	call_setup_id					PACKED;
	uint32_t	trunk_group					PACKED;
	uint32_t	span						PACKED;
	uint32_t	chan						PACKED;
	uint32_t	called_number_digits_count			PACKED; /* it's an array */
	int8_t		called_number_digits [MAX_DIALED_DIGITS + 1]	PACKED; /* it's a null terminated string */
	uint32_t	calling_number_digits_count			PACKED; /* it's an array */
	int8_t		calling_number_digits [MAX_DIALED_DIGITS + 1]	PACKED; /* it's a null terminated string */
	uint32_t	release_cause					PACKED;
	struct	timeval tv						PACKED;
} t_sigboost;


#endif

