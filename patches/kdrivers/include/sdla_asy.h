/*****************************************************************************
* sdla_asy.h	Header file for the Sangoma S508/S514 asynchronous code API	
*
* Author: 	Gideon Hack 	
*
* Copyright:	(c) 2000 Sangoma Technologies Inc.
*
*		This program is free software; you can redistribute it and/or
*		modify it under the terms of the GNU General Public License
*		as published by the Free Software Foundation; either version
*		2 of the License, or (at your option) any later version.
* ============================================================================
*
* Jan 28, 2000	Gideon Hack 	Initial Version
*
*****************************************************************************/


#ifndef _WANPIPE_ASYNC_H
#define _WANPIPE_ASYNC_H

/* ----------------------------------------------------------------------------
 *                        Interface commands
 * --------------------------------------------------------------------------*/

#define SET_ASY_CONFIGURATION		0xE2	/* set the asychronous operational configuration */
#define READ_ASY_CONFIGURATION		0xE3	/* read the current asychronous operational configuration */
#define ENABLE_ASY_COMMUNICATIONS	0xE4	/* enable asychronous communications */
#define DISABLE_ASY_COMMUNICATIONS	0xE5	/* disable asychronous communications */
#define READ_ASY_OPERATIONAL_STATS	0xE7	/* retrieve the asychronous operational statistics */
#define FLUSH_ASY_OPERATIONAL_STATS	0xE8	/* flush the asychronous operational statistics */
#define TRANSMIT_ASY_BREAK_SIGNAL	0xEC	/* transmit an asychronous break signal */



/* ----------------------------------------------------------------------------
 *                     Return codes from interface commands
 * --------------------------------------------------------------------------*/

#define COMMAND_INVALID_FOR_PORT	0x50	/* the command is invalid for the selected port */
#define DISABLE_ASY_COMMS_BEFORE_CFG 	0xE1	/* communications must be disabled before setting the configuration */
#define ASY_COMMS_ENABLED		0xE1	/* communications are currently enabled */
#define ASY_COMMS_DISABLED		0xE1	/* communications are currently disabled */
#define ASY_CFG_BEFORE_COMMS_ENABLED	0xE2	/* perform a SET_ASY_CONFIGURATION before enabling comms */
#define LGTH_ASY_CFG_DATA_INVALID  	0xE2	/* the length of the passed configuration data is invalid */
#define INVALID_ASY_CFG_DATA		0xE3	/* the passed configuration data is invalid */
#define ASY_BREAK_SIGNAL_BUSY		0xEC	/* a break signal is being transmitted */

#pragma pack(1)

/* ----------------------------------------------------------------------------
 *   Constants for the SET_ASY_CONFIGURATION/READ_ASY_CONFIGURATION command
 * --------------------------------------------------------------------------*/

/* the asynchronous configuration structure */
typedef struct {
	unsigned long baud_rate			 ;							/* the baud rate */	
	unsigned short line_config_options	 ;	/* line configuration options */
	unsigned short modem_config_options	 ;	/* modem configuration options */
	unsigned short asy_API_options		 ;	/* asynchronous API options */
	unsigned short asy_protocol_options	 ; /* asynchronous protocol options */
	unsigned short Tx_bits_per_char		 ;	/* number of bits per tx character */
	unsigned short Rx_bits_per_char		 ;	/* number of bits per received character */
	unsigned short stop_bits		 ;	/* number of stop bits per character */
	unsigned short parity			 ;	/* parity definition */
	unsigned short break_timer		 ;	/* the break signal timer */
	unsigned short asy_Rx_inter_char_timer	 ;	/* the receive inter-character timer */
	unsigned short asy_Rx_complete_length	 ;	/* the receive 'buffer complete' length */
	unsigned short XON_char			 ;	/* the XON character */
	unsigned short XOFF_char		 ;	/* the XOFF character */
	unsigned short asy_statistics_options	 ;	/* async operational stat options */
	unsigned long ptr_shared_mem_info_struct     ;/* ptr to the shared memory area information structure */
	unsigned long ptr_asy_Tx_stat_el_cfg_struct  ;/* ptr to the transmit status element configuration structure */
	unsigned long ptr_asy_Rx_stat_el_cfg_struct  ;/* ptr to the receive status element configuration structure */
} ASY_CONFIGURATION_STRUCT;

/* permitted minimum and maximum values for setting the asynchronous configuration */
#define MIN_ASY_BAUD_RATE		50	/* maximum baud rate */
#define MAX_ASY_BAUD_RATE		250000	/* minimum baud rate */
#define MIN_ASY_BITS_PER_CHAR		5	/* minimum number of bits per character */
#define MAX_ASY_BITS_PER_CHAR		8	/* maximum number of bits per character */
#define MIN_BREAK_TMR_VAL		0	/* minimum break signal timer */
#define MAX_BREAK_TMR_VAL		5000	/* maximum break signal timer */
#define MIN_ASY_RX_INTER_CHAR_TMR	0	/* minimum receive inter-character timer */
#define MAX_ASY_RX_INTER_CHAR_TMR	30000	/* maximum receive inter-character timer */
#define MIN_ASY_RX_CPLT_LENGTH		0	/* minimum receive 'length complete' value */
#define MAX_ASY_RX_CPLT_LENGTH		2000	/* maximum receive 'length complete' value */

/* bit settings for the 'asy_API_options' */
#define ASY_RX_DATA_TRANSPARENT		0x0001	/* do not strip parity and unused bits from received characters */

/* bit settings for the 'asy_protocol_options' */
#define ASY_RTS_HS_FOR_RX		0x0001	/* RTS handshaking is used for reception control */
#define ASY_XON_XOFF_HS_FOR_RX		0x0002	/* XON/XOFF handshaking is used for reception control */
#define ASY_XON_XOFF_HS_FOR_TX		0x0004	/* XON/XOFF handshaking is used for transmission control */
#define ASY_DCD_HS_FOR_TX		0x0008	/* DCD handshaking is used for transmission control */
#define ASY_CTS_HS_FOR_TX		0x0020	/* CTS handshaking is used for transmission control */

/* bit settings for the 'stop_bits' definition */
#define ONE_STOP_BIT			1			/* representation for 1 stop bit */
#define TWO_STOP_BITS			2			/* representation for 2 stop bits */
#define ONE_AND_A_HALF_STOP_BITS	3			/* representation for 1.5 stop bits */

/* bit settings for the 'parity' definition */
#define NO_PARITY			0			/* representation for no parity */
#define ODD_PARITY			1			/* representation for odd parity */
#define EVEN_PARITY			2			/* representation for even parity */



/* ----------------------------------------------------------------------------
 *    Constants for the READ_COMMS_ERROR_STATS command (asynchronous mode)
 * --------------------------------------------------------------------------*/

/* the communications error statistics structure */
typedef struct {
	unsigned short Rx_overrun_err_count   	 ;	/* receiver overrun error count */
	unsigned short Rx_parity_err_count	 ;	/* parity errors received count */
	unsigned short Rx_framing_err_count	 ;	/* framing errors received count */
	unsigned short comms_err_stat_reserved_1  ;/* reserved for later use */
	unsigned short comms_err_stat_reserved_2  ;/* reserved for later use */
	unsigned short comms_err_stat_reserved_3  ;/* reserved for later use */
	unsigned short comms_err_stat_reserved_4  ;/* reserved for later use */
	unsigned short comms_err_stat_reserved_5  ;/* reserved for later use */
	unsigned short DCD_state_change_count 	 ;	/* DCD state change count */
	unsigned short CTS_state_change_count	 ;	/* CTS state change count */
} ASY_COMMS_ERROR_STATS_STRUCT;



/* ----------------------------------------------------------------------------
 *         Constants for the READ_ASY_OPERATIONAL_STATS command
 * --------------------------------------------------------------------------*/

/* the asynchronous operational statistics structure */
typedef struct {

	/* Data transmission statistics */
	unsigned long Data_blocks_Tx_count  ;/* number of blocks transmitted */
	unsigned long Data_bytes_Tx_count   ;/* number of bytes transmitted */
	unsigned long Data_Tx_throughput    ;/* transmit throughput */
	unsigned long no_ms_for_Data_Tx_thruput_comp  ;/* millisecond time used for the Tx throughput computation */
	unsigned long Tx_Data_discard_lgth_err_count  ;/* number of Data blocks discarded (length error) */
	unsigned long reserved_Data_frm_Tx_stat1  ;/* reserved for later use */
	unsigned long reserved_Data_frm_Tx_stat2  ;/* reserved for later use */
	unsigned long reserved_Data_frm_Tx_stat3  ;/* reserved for later use */

	/* Data reception statistics */
	unsigned long Data_blocks_Rx_count  ;/* number of blocks received */
	unsigned long Data_bytes_Rx_count   ;/* number of bytes received */
	unsigned long Data_Rx_throughput    ;/* receive throughput */
	unsigned long no_ms_for_Data_Rx_thruput_comp  ;/* millisecond time used for the Rx throughput computation */
	unsigned long Rx_Data_bytes_discard_count     ;/* received Data bytes discarded */
	unsigned long reserved_Data_frm_Rx_stat1      ;/* reserved for later use */

	/* handshaking protocol statistics */
	unsigned short XON_chars_Tx_count	 ;	/* number of XON characters transmitted */
	unsigned short XOFF_chars_Tx_count	 ;	/* number of XOFF characters transmitted */
	unsigned short XON_chars_Rx_count	 ;	/* number of XON characters received */
	unsigned short XOFF_chars_Rx_count	 ;	/* number of XOFF characters received */
	unsigned short Tx_halt_modem_low_count	 ; /* number of times Tx halted (modem line low) */
	unsigned short Rx_halt_RTS_low_count	 ;	/* number of times Rx halted by setting RTS low */
	unsigned long reserved_handshaking_stat1  ;/* reserved for later use */

	/* break statistics */
	unsigned short break_Tx_count	 ;	/* number of break sequences transmitted */
	unsigned short break_Rx_count	 ;	/* number of break sequences received */
	unsigned long reserved_break_stat1  ;/* reserved for later use */

	/* miscellaneous statistics */
	unsigned long reserved_misc_stat1	 ;	/* reserved for later use */
	unsigned long reserved_misc_stat2	 ;	/* reserved for later use */

} ASY_OPERATIONAL_STATS_STRUCT;



/* ----------------------------------------------------------------------------
 *                      Constants for Data transmission
 * --------------------------------------------------------------------------*/

/* the Data block transmit status element configuration structure */
typedef struct {
	unsigned short number_Tx_status_elements  ;		/* number of transmit status elements */
	unsigned long base_addr_Tx_status_elements  ;	/* base address of the transmit element list */
	unsigned long next_Tx_status_element_to_use  ;	/* pointer to the next transmit element to be used */
} ASY_TX_STATUS_EL_CFG_STRUCT;


/* the Data block transmit status element structure */
typedef struct {
	unsigned char opp_flag  ;								/* opp flag */
	unsigned short data_length  ;						/* length of the block to be transmitted */
	unsigned char reserved_1  ;							/* reserved for internal use */
	unsigned long reserved_2  ;							/* reserved for internal use */
	unsigned long reserved_3  ;							/* reserved for internal use */
	unsigned long ptr_data_bfr  ;						/* pointer to the data area */
} ASY_DATA_TX_STATUS_EL_STRUCT;



/* ----------------------------------------------------------------------------
 *                      Constants for Data reception
 * --------------------------------------------------------------------------*/

/* the Data block receive status element configuration structure */
typedef struct {
	unsigned short number_Rx_status_elements     ;/* number of receive status elements */
	unsigned long base_addr_Rx_status_elements   ;/* base address of the receive element list */
	unsigned long next_Rx_status_element_to_use  ;/* pointer to the next receive element to be used */
	unsigned long base_addr_Rx_buffer	 ;/* base address of the receive data buffer */
	unsigned long end_addr_Rx_buffer 	 ;/* end address of the receive data buffer */
} ASY_RX_STATUS_EL_CFG_STRUCT;

/* the Data block receive status element structure */
typedef struct {
	unsigned char opp_flag 		 ;	/* opp flag */
	unsigned short data_length 	 ;	/* length of the received data block */
	unsigned char reserved_1 	 ;	/* reserved for internal use */
	unsigned short time_stamp 	 ; /* receive time stamp (HDLC_STREAMING_MODE) */
	unsigned short data_buffered 	 ;	/* the number of data bytes still buffered */
	unsigned long reserved_2 	 ;	/* reserved for internal use */
	unsigned long ptr_data_bfr 	 ;	/* pointer to the data area */
} ASY_DATA_RX_STATUS_EL_STRUCT;

#pragma pack()

#endif
