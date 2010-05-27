//////////////////////////////////////////////////////////////////////
// sangoma_interface.cpp: interface for Sangoma API driver.
//
// Author	:	David Rokhvarg	<davidr@sangoma.com>
//////////////////////////////////////////////////////////////////////

#include "sangoma_interface.h"

#define DBG_IFACE	if(1)printf
#define INFO_IFACE	if(1)printf
#define ERR_IFACE	printf("Error: %s(): line:%d :", __FUNCTION__, __LINE__);printf
#define WARN_IFACE	printf("Warning: %s(): line:%d :", __FUNCTION__, __LINE__);printf

#define IFACE_FUNC() if(1)printf("%s():line:%d\n", __FUNCTION__, __LINE__)

#define DO_COMMAND(wan_udp)	DoManagementCommand(sangoma_dev, &wan_udp);

#define NUMBER_OF_WAIT_OBJECTS 1

#ifdef WP_API_FEATURE_LIBSNG_HWEC	/* defined in wanpipe_api_iface.h */
# include "wanpipe_events.h"
# include "wanec_api.h"
sangoma_status_t config_hwec(char *strDeviceName, unsigned long in_ulChannelMap);
#endif

extern wp_program_settings_t	program_settings;

sangoma_interface::sangoma_interface(int wanpipe_number, int interface_number)
{
	DBG_IFACE("%s()\n", __FUNCTION__);

	memset(device_name, 0x00, DEV_NAME_LEN);
	memset(&wan_udp, 0x00, sizeof(wan_udp));
	memset(&tdm_api_cmd, 0x00, sizeof(tdm_api_cmd));
	memset(&wp_api, 0x00, sizeof(wp_api));

	WanpipeNumber = wanpipe_number;
	InterfaceNumber = interface_number;

	//Form the Interface Name from Wanpipe Number and Interface Index (i.e. wanpipe1_if1).
	//(This Interface Name can be used for debugging.)
	_snprintf(device_name, DEV_NAME_LEN, WP_INTERFACE_NAME_FORM, wanpipe_number, interface_number);

	INFO_IFACE("1.Using Device Name: %s\n", device_name);

	//////////////////////////////////////////////////////////////////
	terminate_tx_rx_threads = 0;
	is_rbs_monitoring_enabled = 0;

	sng_wait_obj = NULL;

	//////////////////////////////////////////////////////////////////
	//receive stuff
	rx_frames_count = 0;
	rx_bytes_count = 0;
	//for counting frames with CRC/Abort errors
	bad_rx_frames_count = 0;

	//////////////////////////////////////////////////////////////////
	//transmit stuff
	tx_bytes_count = 0;
	tx_frames_count = 0;
	tx_test_byte = 0;

	//////////////////////////////////////////////////////////////////
	//IOCTL management structures and variables
	protocol_cb_size = sizeof(wan_mgmt_t)+sizeof(wan_cmd_t)+1;
	wan_protocol = 0;
	adapter_type = 0;
	is_logger_dev = 0;

#if USE_STELEPHONY_API
	stelObj = DtmfBuffer = FskCidBuffer = NULL;
	memset(&scf, 0x00, sizeof(scf));
	InitializeCriticalSection(&StelTxCriticalSection);
#endif
	generate_bit_rev_table();
#if DBG_TIMING
	memset(&wan_debug_rx_timing, 0x00, sizeof(wan_debug_t));
	debug_set_timing_info(	&wan_debug_rx_timing,
							20	/* expected timediff in milliseconds */,
							2	/* allowed deviation from expected timediff */);
#endif
}

sangoma_interface::~sangoma_interface()
{
	DBG_IFACE("%s()\n", __FUNCTION__);
	cleanup();
}

int sangoma_interface::init(callback_functions_t *callback_functions_ptr)
{
	DBG_IFACE("%s()\n", __FUNCTION__);

	memcpy(&callback_functions, callback_functions_ptr, sizeof(callback_functions_t));

	////////////////////////////////////////////////////////////////////////////
	//open handle for reading and writing data, for events reception and other commands
	sangoma_dev = open_api_device();
    if (sangoma_dev == INVALID_HANDLE_VALUE){
		ERR_IFACE( "Unable to open %s for Rx/Tx!\n", device_name);
		return 1;
	}

	if(SANG_ERROR(sangoma_wait_obj_create(&sng_wait_obj, sangoma_dev, SANGOMA_DEVICE_WAIT_OBJ_SIG))){
		ERR_IFACE("Failed to create 'sangoma_wait_object' for %s\n", device_name);
		return 1;
	}

	////////////////////////////////////////////////////////////////////////////
	//get current protocol
	if(get_wan_config() == WAN_FALSE){
		ERR_IFACE( "Failed to get current protocol!\n");
		return 1;
	}

	////////////////////////////////////////////////////////////////////////////
	//get Front End Type (T1/E1/Analog...)
	if (get_fe_type(&adapter_type) == WAN_FALSE){
		ERR_IFACE( "Failed to get Front End Type!\n");
		return 1;
	}

	////////////////////////////////////////////////////////////////////////////
	//may need interface configuration, so get it now
	if(get_interface_configuration(&wanif_conf_struct)){
		ERR_IFACE( "Failed to get Interface Configuration!\n");
		return 1;
	}

	////////////////////////////////////////////////////////////////////////////
#if USE_STELEPHONY_API
	////////////////////////////////////////////////////////////////////////////
	//Sangoma Telephony API (stelephony.dll) provides the following telephony services:
	//1. FSK Caller ID detection for Analog FXO.
	//2. Software DTMF detection.
	//3. Q931 decoding
	//4. FSK Caller ID Generation.
	
	//initialize Stelephony callback struct!
	memset(&scf, 0x00, sizeof(scf));

	//copy callback pointers from global to stelephony
	scf.FSKCallerIDEvent	= callback_functions.FSKCallerIDEvent;
	scf.DTMFEvent			= callback_functions.DTMFEvent;
	scf.Q931Event			= callback_functions.Q931Event;
	scf.FSKCallerIDTransmit	= callback_functions.FSKCallerIDTransmit;
	scf.SwDTMFBuffer		= callback_functions.SwDtmfTransmit;

	StelSetup(&stelObj, this, &scf);
	DBG_IFACE("%s(): stelObj: 0x%p\n", __FUNCTION__, stelObj);
	if(stelObj){
		stelephony_option_t codec = (program_settings.voice_codec_alaw == 1 ? STEL_OPTION_ALAW : STEL_OPTION_MULAW);
		if(scf.FSKCallerIDEvent){
			StelEventControl(stelObj, STEL_EVENT_FSK_CALLER_ID, STEL_CTRL_CODE_ENABLE, &codec);
		}
		if(scf.DTMFEvent){
			StelEventControl(stelObj, STEL_EVENT_DTMF, STEL_CTRL_CODE_ENABLE, &codec);
		}
		if(scf.Q931Event){
			StelEventControl(stelObj, STEL_EVENT_Q931, STEL_CTRL_CODE_ENABLE, NULL);
		}
		if(scf.FSKCallerIDTransmit){
			StelEventControl(stelObj, STEL_FEATURE_FSK_CALLER_ID, STEL_CTRL_CODE_ENABLE, &codec);
		}
		if(scf.SwDTMFBuffer){
			StelEventControl(stelObj, STEL_FEATURE_SW_DTMF, STEL_CTRL_CODE_ENABLE, &codec);
		}

    }else{
		/* Possible reasons:
		* 1.ToneDecoder.dll was not registered with COM.
		*	Resolution: in Command Line window run: regsvr32 ToneDecoder.dll.
		*/
		ERR_IFACE("Failed to initialize Stelephony.dll!\n");
		return 1;
	}
#endif//USE_STELEPHONY_API

	IFACE_FUNC();
	return 0;
}

sng_fd_t sangoma_interface::open_api_device()
{
	DBG_IFACE("%s(): WanpipeNumber: %d, InterfaceNumber: %d\n", __FUNCTION__, WanpipeNumber, InterfaceNumber);

	return sangoma_open_api_span_chan(WanpipeNumber, InterfaceNumber);
}

void sangoma_interface::generate_bit_rev_table(void)
{
	unsigned char util_char;
	unsigned char misc_status_byte;
	int i;

	/* generate the bit-reversing table for all unsigned characters */
	for(util_char = 0;; util_char ++) {
		misc_status_byte = 0;			/* zero the character to be 'built' */
		/* process all 8 bits of the source byte and generate the */
		for(i = 0; i <= 7; i ++) {
		      	/* corresponding 'bit-flipped' character */
			if(util_char & (1 << i)) {
				misc_status_byte |= (1 << (7 - i));
			}
		}
		/* insert the 'bit-flipped' character into the table
		 * at the appropriate location */
		wp_brt[util_char] = misc_status_byte;

		/* exit when all unsigned characters have been processed */
		if(util_char == 0xFF) {
			break;
		}
	}
}

void sangoma_interface::bit_swap_a_buffer(unsigned char *data, int len)
{
	int i;
	for (i=0; i < len; i++){
		data[i]=wp_brt[data[i]];
	}
}

int sangoma_interface::get_wan_config()
{
	int err = WAN_TRUE;

	/* Get Protocol type */
	wan_udp.wan_udphdr_command	= WAN_GET_PROTOCOL;
	wan_udp.wan_udphdr_data_len = 0;

	DO_COMMAND(wan_udp);
	if (wan_udp.wan_udphdr_return_code){
		ERR_IFACE( "Error: Command WAN_GET_PROTOCOL failed! return code: 0x%X\n",
			wan_udp.wan_udphdr_return_code);
		return WAN_FALSE;
	}

	wan_protocol = wan_udp.wan_udphdr_data[0];

	INFO_IFACE( "Device %s running protocol: %s \n",
		device_name, SDLA_DECODE_PROTOCOL(wan_protocol));

	return err;
}

int sangoma_interface::get_fe_type(unsigned char* adapter_type)
{
	int err = WAN_TRUE;

	/* Read Adapter Type */
	wan_udp.wan_udphdr_command = WAN_GET_MEDIA_TYPE;
	wan_udp.wan_udphdr_data[0] = WAN_MEDIA_NONE;
	wan_udp.wan_udphdr_data_len = 0;

	DO_COMMAND(wan_udp);
	if(wan_udp.wan_udphdr_return_code){
		ERR_IFACE( "Error: Command WANPIPEMON_GET_MEDIA_TYPE failed! return code: 0x%X",
			wan_udp.wan_udphdr_return_code);
		return WAN_FALSE;
	}

	*adapter_type =	get_wan_udphdr_data_byte(0);

	INFO_IFACE( "Front End Type: ");
	switch(*adapter_type)
	{
	case WAN_MEDIA_NONE:
		INFO_IFACE( "Serial");
		break;
	case WAN_MEDIA_T1:
		INFO_IFACE( "T1");
		break;
	case WAN_MEDIA_E1:
		INFO_IFACE( "E1");
		break;
	case WAN_MEDIA_56K:
		INFO_IFACE( "56K");
		break;
	case WAN_MEDIA_FXOFXS:
		INFO_IFACE( "Aanalog");
		break;
	case WAN_MEDIA_BRI:
		INFO_IFACE( "ISDN BRI");
		break;
	case WAN_MEDIA_SERIAL:
		INFO_IFACE("Serial");
		break;
	default:
		INFO_IFACE( "Unknown");
		err = WAN_FALSE;
	}
	INFO_IFACE( "\n");

	return err;
}

//return POINTER to data at offset 'off'
unsigned char* sangoma_interface::get_wan_udphdr_data_ptr(unsigned char off)
{
	unsigned char *p_data = (unsigned char*)&wan_udp.wan_udphdr_data[0];
	p_data += off;
	return p_data;
}

unsigned char sangoma_interface::set_wan_udphdr_data_byte(unsigned char off, unsigned char data)
{
	unsigned char *p_data = (unsigned char*)&wan_udp.wan_udphdr_data[0];
	p_data[off] = data;
	return 0;
}

//return DATA at offset 'off'
unsigned char sangoma_interface::get_wan_udphdr_data_byte(unsigned char off)
{
	unsigned char *p_data = (unsigned char*)&wan_udp.wan_udphdr_data[0];
	return p_data[off];
}

void sangoma_interface::get_te1_56k_stat(void)
{
	sdla_fe_stats_t	*fe_stats;

	switch(adapter_type)
	{
	case WAN_MEDIA_T1:
	case WAN_MEDIA_E1:
	case WAN_MEDIA_56K:
		;//do nothing
		break;

	default:
		ERR_IFACE( "Command invalid for Adapter Type %d.\n", adapter_type);
		return;
	}

	/* Read T1/E1/56K alarms and T1/E1 performance monitoring counters */
	wan_udp.wan_udphdr_command = WAN_FE_GET_STAT;
	wan_udp.wan_udphdr_data_len = 0;
   	wan_udp.wan_udphdr_return_code = 0xaa;
   	wan_udp.wan_udphdr_fe_force = 0;
	DO_COMMAND(wan_udp);
	if (wan_udp.wan_udphdr_return_code != 0){
		ERR_IFACE( "Failed to read T1/E1/56K statistics.\n");
		return;
	}

	fe_stats = (sdla_fe_stats_t*)get_wan_udphdr_data_ptr(0);

	if (adapter_type == WAN_MEDIA_T1 || adapter_type == WAN_MEDIA_E1){
		INFO_IFACE("***** %s: %s Alarms (Framer) *****\n\n",
			device_name, (adapter_type == WAN_MEDIA_T1) ? "T1" : "E1");
		INFO_IFACE("ALOS:\t%s\t| LOS:\t%s\n", 
				WAN_TE_PRN_ALARM_ALOS(fe_stats->alarms), 
				WAN_TE_PRN_ALARM_LOS(fe_stats->alarms));
		INFO_IFACE("RED:\t%s\t| AIS:\t%s\n", 
				WAN_TE_PRN_ALARM_RED(fe_stats->alarms), 
				WAN_TE_PRN_ALARM_AIS(fe_stats->alarms));
		INFO_IFACE("LOF:\t%s\t| RAI:\t%s\n", 
				WAN_TE_PRN_ALARM_LOF(fe_stats->alarms),
				WAN_TE_PRN_ALARM_RAI(fe_stats->alarms));

		if (fe_stats->alarms & WAN_TE_ALARM_LIU){
			INFO_IFACE("\n***** %s: %s Alarms (LIU) *****\n\n",
				device_name, (adapter_type == WAN_MEDIA_T1) ? "T1" : "E1");
			INFO_IFACE("Short Circuit:\t%s\n", 
					WAN_TE_PRN_ALARM_LIU_SC(fe_stats->alarms));
			INFO_IFACE("Open Circuit:\t%s\n", 
					WAN_TE_PRN_ALARM_LIU_OC(fe_stats->alarms));
			INFO_IFACE("Loss of Signal:\t%s\n", 
					WAN_TE_PRN_ALARM_LIU_LOS(fe_stats->alarms));
		}

	}else if  (adapter_type == WAN_MEDIA_DS3 || adapter_type == WAN_MEDIA_E3){
		INFO_IFACE("***** %s: %s Alarms *****\n\n",
			device_name, (adapter_type == WAN_MEDIA_DS3) ? "DS3" : "E3");

		if (adapter_type == WAN_MEDIA_DS3){
			INFO_IFACE("AIS:\t%s\t| LOS:\t%s\n",
					WAN_TE3_AIS_ALARM(fe_stats->alarms),
					WAN_TE3_LOS_ALARM(fe_stats->alarms));

			INFO_IFACE("OOF:\t%s\t| YEL:\t%s\n",
					WAN_TE3_OOF_ALARM(fe_stats->alarms),
					WAN_TE3_YEL_ALARM(fe_stats->alarms));
		}else{
			INFO_IFACE("AIS:\t%s\t| LOS:\t%s\n",
					WAN_TE3_AIS_ALARM(fe_stats->alarms),
					WAN_TE3_LOS_ALARM(fe_stats->alarms));

			INFO_IFACE("OOF:\t%s\t| YEL:\t%s\n",
					WAN_TE3_OOF_ALARM(fe_stats->alarms),
					WAN_TE3_YEL_ALARM(fe_stats->alarms));
			
			INFO_IFACE("LOF:\t%s\t\n",
					WAN_TE3_LOF_ALARM(fe_stats->alarms));
		}
		
	}else if (adapter_type == WAN_MEDIA_56K){
		INFO_IFACE("***** %s: 56K CSU/DSU Alarms *****\n\n\n", device_name);
	 	INFO_IFACE("In Service:\t\t%s\tData mode idle:\t\t%s\n",
			 	INS_ALARM_56K(fe_stats->alarms), 
			 	DMI_ALARM_56K(fe_stats->alarms));
	
	 	INFO_IFACE("Zero supp. code:\t%s\tCtrl mode idle:\t\t%s\n",
			 	ZCS_ALARM_56K(fe_stats->alarms), 
			 	CMI_ALARM_56K(fe_stats->alarms));

	 	INFO_IFACE("Out of service code:\t%s\tOut of frame code:\t%s\n",
			 	OOS_ALARM_56K(fe_stats->alarms), 
			 	OOF_ALARM_56K(fe_stats->alarms));
		
	 	INFO_IFACE("Valid DSU NL loopback:\t%s\tUnsigned mux code:\t%s\n",
			 	DLP_ALARM_56K(fe_stats->alarms), 
			 	UMC_ALARM_56K(fe_stats->alarms));

	 	INFO_IFACE("Rx loss of signal:\t%s\t\n",
			 	RLOS_ALARM_56K(fe_stats->alarms)); 
		
	}else{
		INFO_IFACE("***** %s: Unknown Front End 0x%X *****\n\n",
			device_name, adapter_type);
	}

	if (adapter_type == WAN_MEDIA_T1 || adapter_type == WAN_MEDIA_E1){
		sdla_te_pmon_t*	pmon = &fe_stats->te_pmon;

		INFO_IFACE("\n\n***** %s: %s Performance Monitoring Counters *****\n\n",
				device_name, (adapter_type == WAN_MEDIA_T1) ? "T1" : "E1");
		if (pmon->mask & WAN_TE_BIT_PMON_LCV){
			INFO_IFACE("Line Code Violation\t: %d\n",
						pmon->lcv_errors);
		}
		if (pmon->mask & WAN_TE_BIT_PMON_BEE){
			INFO_IFACE("Bit Errors (CRC6/Ft/Fs)\t: %d\n",
						pmon->bee_errors);
		}
		if (pmon->mask & WAN_TE_BIT_PMON_OOF){
			INFO_IFACE("Out of Frame Errors\t: %d\n",
						pmon->oof_errors);
		}
		if (pmon->mask & WAN_TE_BIT_PMON_FEB){
			INFO_IFACE("Far End Block Errors\t: %d\n",
						pmon->feb_errors);
		}
		if (pmon->mask & WAN_TE_BIT_PMON_CRC4){
			INFO_IFACE("CRC4 Errors\t\t: %d\n",
						pmon->crc4_errors);
		}
		if (pmon->mask & WAN_TE_BIT_PMON_FER){
			INFO_IFACE("Framing Bit Errors\t: %d\n",
						pmon->fer_errors);
		}
		if (pmon->mask & WAN_TE_BIT_PMON_FAS){
			INFO_IFACE("FAS Errors\t\t: %d\n",
						pmon->fas_errors);
		}
	}

	if (adapter_type == WAN_MEDIA_DS3 || adapter_type == WAN_MEDIA_E3){
		sdla_te3_pmon_t*	pmon = &fe_stats->u.te3_pmon;

		INFO_IFACE("\n\n***** %s: %s Performance Monitoring Counters *****\n\n",
				device_name, (adapter_type == WAN_MEDIA_DS3) ? "DS3" : "E3");

		INFO_IFACE("Framing Bit Error:\t%d\tLine Code Violation:\t%d\n", 
				pmon->pmon_framing,
				pmon->pmon_lcv);

		if (adapter_type == WAN_MEDIA_DS3){
			INFO_IFACE("Parity Error:\t\t%d\n",
					pmon->pmon_parity);
			INFO_IFACE("CP-Bit Error Event:\t%d\tFEBE Event:\t\t%d\n", 
					pmon->pmon_cpbit,
					pmon->pmon_febe);
		}else{
			INFO_IFACE("Parity Error:\t%d\tFEBE Event:\t\t%d\n",
					pmon->pmon_parity,
					pmon->pmon_febe);
		}
	}
	
	if (adapter_type == WAN_MEDIA_T1 || adapter_type == WAN_MEDIA_E1){
		if (strlen(fe_stats->u.te1_stats.rxlevel)){
			INFO_IFACE("\n\nRx Level\t: %s\n",
					fe_stats->u.te1_stats.rxlevel);		
		}
	}

	return;
}

int sangoma_interface::loopback_command(u_int8_t type, u_int8_t mode, u_int32_t chan_map)
{
	sdla_fe_lbmode_t	*lb;
	int			err = 0, cnt = 0;

	lb = (sdla_fe_lbmode_t*)get_wan_udphdr_data_ptr(0);
	memset(lb, 0, sizeof(sdla_fe_lbmode_t));
	lb->cmd		= WAN_FE_LBMODE_CMD_SET;
	lb->type	= type;
	lb->mode	= mode;
	lb->chan_map	= chan_map;

lb_poll_again:
	wan_udp.wan_udphdr_command	= WAN_FE_LB_MODE;
	wan_udp.wan_udphdr_data_len	= sizeof(sdla_fe_lbmode_t);
	wan_udp.wan_udphdr_return_code	= 0xaa;
	
	DO_COMMAND(wan_udp);
	
	if (wan_udp.wan_udphdr_return_code){
		err = 1;
		return err;
	}
	if (lb->rc == WAN_FE_LBMODE_RC_PENDING){

		if (!cnt) printf("Please wait ..");fflush(stdout);
		if (cnt++ < 10){
			printf(".");fflush(stdout);
			sangoma_msleep(100);
			lb->cmd	= WAN_FE_LBMODE_CMD_POLL;
			lb->rc	= 0x00;
			goto lb_poll_again;
		}
		err = 2;
		goto loopback_command_exit;
	}else if (lb->rc != WAN_FE_LBMODE_RC_SUCCESS){
		err = 3;
	}
	if (cnt) printf("\n");

loopback_command_exit:
	return err;
}

void sangoma_interface::set_lb_modes(unsigned char type, unsigned char mode)
{
	switch(adapter_type)
	{
	case WAN_MEDIA_T1:
	case WAN_MEDIA_E1:
	case WAN_MEDIA_56K:
		;//do nothing
		break;
	default:
		ERR_IFACE( "Command invalid for Adapter Type %d.\n", adapter_type);
		return;
	}

	if(loopback_command(type, mode, ENABLE_ALL_CHANNELS)){
		ERR_IFACE("Error: Loop Back command failed!\n");
		return;
	}

	if (adapter_type == WAN_MEDIA_T1 || adapter_type == WAN_MEDIA_E1){
		INFO_IFACE("%s %s mode ... %s!\n",
				WAN_TE1_LB_ACTION_DECODE(mode),
				WAN_TE1_LB_MODE_DECODE(type),
				(!wan_udp.wan_udphdr_return_code)?"Done":"Failed");
	}else if (adapter_type == WAN_MEDIA_DS3 || adapter_type == WAN_MEDIA_E3){
		INFO_IFACE("%s %s mode ... %s!\n",
				WAN_TE3_LB_ACTION_DECODE(mode),
				WAN_TE3_LB_TYPE_DECODE(type),
				(!wan_udp.wan_udphdr_return_code)?"Done":"Failed");
	}else{
		INFO_IFACE("%s %s mode ... %s (default)!\n",
				WAN_TE1_LB_ACTION_DECODE(mode),
				WAN_TE1_LB_MODE_DECODE(type),
				(!wan_udp.wan_udphdr_return_code)?"Done":"Failed");
	}
	return;
}

unsigned char sangoma_interface::get_adapter_type()
{
	return adapter_type;
}

unsigned int sangoma_interface::get_sub_media()
{
	return wanif_conf_struct.sub_media;
}

int sangoma_interface::get_operational_stats(wanpipe_chan_stats_t *stats)
{
	int err;
	unsigned char firm_ver, cpld_ver;

	err=sangoma_get_stats(sangoma_dev, &wp_api, stats);
	if (err) {
		ERR_IFACE("sangoma_get_stats(() failed (err: %d (0x%X))!\n", err, err);
		return 1;
	}

	INFO_IFACE( "******* OPERATIONAL_STATS *******\n");

	INFO_IFACE("\trx_packets\t: %u\n",			stats->rx_packets);
	INFO_IFACE("\ttx_packets\t: %u\n",			stats->tx_packets);
	INFO_IFACE("\trx_bytes\t: %u\n",			stats->rx_bytes);
	INFO_IFACE("\ttx_bytes\t: %u\n",			stats->tx_bytes);
	INFO_IFACE("\trx_errors\t: %u\n",			stats->rx_errors);	//Total number of Rx errors
	INFO_IFACE("\ttx_errors\t: %u\n",			stats->tx_errors);	//Total number of Tx errors
	INFO_IFACE("\trx_dropped\t: %u\n",			stats->rx_dropped);
	INFO_IFACE("\ttx_dropped\t: %u\n",			stats->tx_dropped);
	INFO_IFACE("\tmulticast\t: %u\n",			stats->multicast);
	INFO_IFACE("\tcollisions\t: %u\n",			stats->collisions);

	INFO_IFACE("\trx_length_errors: %u\n",		stats->rx_length_errors);
	INFO_IFACE("\trx_over_errors\t: %u\n",		stats->rx_over_errors);
	INFO_IFACE("\trx_crc_errors\t: %u\n",		stats->rx_crc_errors);	//HDLC CRC mismatch
	INFO_IFACE("\trx_frame_errors\t: %u\n",		stats->rx_frame_errors);//HDLC "abort" occured
	INFO_IFACE("\trx_fifo_errors\t: %u\n",		stats->rx_fifo_errors);
	INFO_IFACE("\trx_missed_errors: %u\n",		stats->rx_missed_errors);

	INFO_IFACE("\ttx_aborted_errors: %u\n",		stats->tx_aborted_errors);
	INFO_IFACE("\tTx Idle Data\t: %u\n",		stats->tx_carrier_errors);

	INFO_IFACE("\ttx_fifo_errors\t: %u\n",		stats->tx_fifo_errors);
	INFO_IFACE("\ttx_heartbeat_errors: %u\n",	stats->tx_heartbeat_errors);
	INFO_IFACE("\ttx_window_errors: %u\n",		stats->tx_window_errors);

	INFO_IFACE("\n\ttx_packets_in_q: %u\n",	stats->current_number_of_frames_in_tx_queue);
	INFO_IFACE("\ttx_queue_size: %u\n",		stats->max_tx_queue_length);

	INFO_IFACE("\n\trx_packets_in_q: %u\n",	stats->current_number_of_frames_in_rx_queue);
	INFO_IFACE("\trx_queue_size: %u\n",		stats->max_rx_queue_length);

	INFO_IFACE("\n\trx_events_in_q: %u\n",		stats->current_number_of_events_in_event_queue);
	INFO_IFACE("\trx_event_queue_size: %u\n",	stats->max_event_queue_length);
	INFO_IFACE("\trx_events: %u\n",				stats->rx_events);
	INFO_IFACE("\trx_events_dropped: %u\n",		stats->rx_events_dropped);

	INFO_IFACE("\tHWEC tone (DTMF) events counter: %u\n",	stats->rx_events_tone);
	INFO_IFACE( "*********************************\n");

	err=sangoma_get_driver_version(sangoma_dev,&wp_api, NULL);
	if (err) {
		return 1;
	}
	INFO_IFACE("\tDriver Version: %u.%u.%u.%u\n",
				wp_api.wp_cmd.version.major,
				wp_api.wp_cmd.version.minor,
				wp_api.wp_cmd.version.minor1,
				wp_api.wp_cmd.version.minor2);

	err=sangoma_get_firmware_version(sangoma_dev, &wp_api, &firm_ver);
	if (err) {
		return 1;
	}
	INFO_IFACE("\tFirmware Version: %X\n",
				firm_ver);

	err=sangoma_get_cpld_version(sangoma_dev, &wp_api, &cpld_ver);
	if (err) {
		return 1;
	}
	INFO_IFACE("\tCPLD Version: %X\n",
				cpld_ver);

#if DBG_TIMING
	debug_print_dbg_struct(&this->wan_debug_rx_timing, "sangoma_interface::read_data");
#endif
	return 0;
}

int sangoma_interface::flush_operational_stats()
{
    return sangoma_flush_stats(sangoma_dev,&wp_api);
}

int sangoma_interface::flush_tx_buffers()
{
	return sangoma_flush_bufs(sangoma_dev,&wp_api);
}

int sangoma_interface::enable_rbs_monitoring()
{
	int rc;
	if(is_rbs_monitoring_enabled == 0){
		/* enable RBS monitoring one time per card */
		rc=tdm_enable_rbs_events(100);
		
		if(rc){
			ERR_IFACE("tdm_enable_rbs_events() failed (rc=%i)!\n",rc);
			rc = 1;
		}else{
			is_rbs_monitoring_enabled = 1;
			INFO_IFACE("RBS Monitoring successfully enabled.\n");
			rc = 0;
		}
	}else{
		INFO_IFACE("RBS Monitoring already enabled!! Should be done only once.\n");
		rc = 0;
	}
	return rc;
}

// set Idle Transmit flag (BitStream/Voice only).
int sangoma_interface::set_tx_idle_flag(unsigned char new_idle_flag)
{
	tdm_api_cmd.cmd = WP_API_CMD_SET_IDLE_FLAG;
	tdm_api_cmd.idle_flag = new_idle_flag;
	return tdmv_api_ioctl(&tdm_api_cmd);
}

//get RBS being received
char sangoma_interface::get_rbs(rbs_management_t *rbs_management_ptr)
{
	int err;

	err=sangoma_tdm_read_rbs(sangoma_dev, &wp_api, rbs_management_ptr->channel, &rbs_management_ptr->ABCD_bits);
	if (err) {
		ERR_IFACE( "Error: command WANPIPEMON_GET_RBS_BITS failed!\n");
		return 1;
	}

	INFO_IFACE( "**** WANPIPEMON_GET_RBS_BITS OK ****\n");

	//INFO_IFACE( "ABCD_bits (HEX): 0x%02X\n", rbs_management_ptr->ABCD_bits);

	INFO_IFACE( "Channel: %d, RX RBS: A:%1d B:%1d C:%1d D:%1d\n",
			rbs_management_ptr->channel,
			(rbs_management_ptr->ABCD_bits & WAN_RBS_SIG_A) ? 1 : 0,
			(rbs_management_ptr->ABCD_bits & WAN_RBS_SIG_B) ? 1 : 0,
			(rbs_management_ptr->ABCD_bits & WAN_RBS_SIG_C) ? 1 : 0,
			(rbs_management_ptr->ABCD_bits & WAN_RBS_SIG_D) ? 1 : 0);
	return 0;
}

//set RBS to be transmitted
char sangoma_interface::set_rbs(rbs_management_t *rbs_management_ptr)
{
	int err;
	
	err=sangoma_tdm_write_rbs(sangoma_dev, &wp_api, rbs_management_ptr->channel, rbs_management_ptr->ABCD_bits);
	if (err) {
		return 1;
	}

	INFO_IFACE( "**** WANPIPEMON_SET_RBS_BITS OK ****\n");

	//INFO_IFACE( "ABCD_bits (HEX): 0x%02X\n", rbs_management_ptr->ABCD_bits);

	INFO_IFACE( "Channel: %d, TX RBS: A:%1d B:%1d C:%1d D:%1d\n",
			rbs_management_ptr->channel,
			(rbs_management_ptr->ABCD_bits & WAN_RBS_SIG_A) ? 1 : 0,
			(rbs_management_ptr->ABCD_bits & WAN_RBS_SIG_B) ? 1 : 0,
			(rbs_management_ptr->ABCD_bits & WAN_RBS_SIG_C) ? 1 : 0,
			(rbs_management_ptr->ABCD_bits & WAN_RBS_SIG_D) ? 1 : 0);
	return 0;
}

int sangoma_interface::get_interface_configuration(if_cfg_t *wanif_conf_ptr)
{
	wan_udp.wan_udphdr_command = WANPIPEMON_READ_CONFIGURATION;
	wan_udp.wan_udphdr_data_len = sizeof(if_cfg_t);

	memset(wanif_conf_ptr, 0x00, sizeof(if_cfg_t));

	DO_COMMAND(wan_udp);
	if(wan_udp.wan_udphdr_return_code){
		ERR_IFACE( "Error: command READ_CONFIGURATION failed!\n");
		return 1;
	}
	memcpy(wanif_conf_ptr, get_wan_udphdr_data_ptr(0), sizeof(if_cfg_t));

	INFO_IFACE( "**** READ_CONFIGURATION ****\n");
	INFO_IFACE( "Operational Mode\t: %s (%d)\n", SDLA_DECODE_USEDBY_FIELD(wanif_conf_ptr->usedby), wanif_conf_ptr->usedby);
	INFO_IFACE( "Configued Active Channels \t: 0x%08X\n", wanif_conf_ptr->active_ch);
	INFO_IFACE( "Echo Canceller Channels\t\t: 0x%08X\n", wanif_conf_ptr->ec_active_ch);
	INFO_IFACE( "User Specified Channels during Port Config\t: 0x%08X\n", wanif_conf_ptr->cfg_active_ch);
	INFO_IFACE( "Interface Number\t: %u\n", wanif_conf_ptr->interface_number);
	INFO_IFACE( "Media type\t\t: %u\n", wanif_conf_ptr->media);
	INFO_IFACE( "Line Mode\t\t: %s\n", wanif_conf_ptr->line_mode);

#if defined(__WINDOWS__)
	//make sure API driver is configred in correct mode
	switch(wanif_conf_ptr->usedby)
	{
	case TDM_SPAN_VOICE_API:
	case TDM_CHAN_VOICE_API:
	case API:
	case TDM_VOICE_DCHAN:
	case STACK:
		//ok
		break;
	default:
		//this application can not run if driver is not in correct mode
		INFO_IFACE("Warning: Invalid API Driver Operational Mode: %s (%d)!\n",
			SDLA_DECODE_USEDBY_FIELD(wanif_conf_ptr->usedby), wanif_conf_ptr->usedby);
	}
#endif

	switch(wanif_conf_ptr->media)
	{
	case WAN_MEDIA_FXOFXS:
		switch(wanif_conf_ptr->sub_media)
		{
		case MOD_TYPE_FXS:
			INFO_IFACE( "Media sub-type\t\t: FXS\n");
			break;
		case MOD_TYPE_FXO:
			INFO_IFACE( "Media sub-type\t\t: FXO\n");
			break;
		default:
			INFO_IFACE( "Media sub-type\t\t: Unknown!!\n");
			break;
		}
		break;/* WAN_MEDIA_FXOFXS */

	case WAN_MEDIA_BRI:
		switch(wanif_conf_ptr->sub_media)
		{
		case MOD_TYPE_NT:
			INFO_IFACE( "Media sub-type\t\t: ISDN BRI NT\n");
			break;
		case MOD_TYPE_TE:
			INFO_IFACE( "Media sub-type\t\t: ISDN BRI TE\n");
			break;
		default:
			INFO_IFACE( "Media sub-type\t\t: Unknown!!\n");
			break;
		}
		break;/* WAN_MEDIA_BRI */

	case WAN_MEDIA_SERIAL:
		INFO_IFACE("Media sub-type\t\t: %s\n", INT_DECODE(wanif_conf_ptr->sub_media));
		break;
	}
	INFO_IFACE( "****************************\n");

	return 0;
}

void sangoma_interface::get_api_driver_version (PDRIVER_VERSION version)
{

	int err;

	err=sangoma_get_driver_version(sangoma_dev, &wp_api, version);
	if(err){
		ERR_IFACE("Error: command READ_CODE_VERSION failed!\n");
		return;
	}

	INFO_IFACE("\nAPI version\t: %d,%d,%d,%d\n",
		version->major, version->minor, version->minor1, version->minor2);
	INFO_IFACE("\n");

	INFO_IFACE("Command READ_CODE_VERSION was successful.\n");
}

void sangoma_interface::get_card_customer_id(u_int8_t *customer_id)
{
	wan_udp.wan_udphdr_command = WANPIPEMON_AFT_CUSTOMER_ID;
	wan_udp.wan_udphdr_data_len = 0;

	DO_COMMAND(wan_udp);
	if(wan_udp.wan_udphdr_return_code){
		ERR_IFACE("Error: command SIOC_AFT_CUSTOMER_ID failed!\n");
		return;
	}

	*customer_id = get_wan_udphdr_data_byte(0);

	INFO_IFACE("\nCard Customer ID\t: 0x%02X\n", *customer_id);

	INFO_IFACE("Command SIOC_AFT_CUSTOMER_ID was successful.\n");
}

int sangoma_interface::get_open_handles_counter()
{
	return sangoma_get_open_cnt(sangoma_dev,&wp_api);
}


//It is very important to close ALL open
//handles, because the API will continue
//receive data until the LAST handel is closed.
void sangoma_interface::cleanup()
{
	INFO_IFACE("sangoma_interface::cleanup()\n");

	if(sng_wait_obj){
		sangoma_wait_obj_delete(&sng_wait_obj);
	}

#ifdef WIN32
	if(sangoma_dev != INVALID_HANDLE_VALUE){
		if(is_rbs_monitoring_enabled == 1){
			/* disable RBS monitoring one time per card */
			is_rbs_monitoring_enabled = 0;
			if(tdm_disable_rbs_events()){
				ERR_IFACE("tdm_disable_rbs_events() failed!\n");
			}
		}
	}
#endif

	if(sangoma_dev != INVALID_HANDLE_VALUE){
		INFO_IFACE( "Closing Rx/Tx fd.\n");
		sangoma_close(&sangoma_dev);
		sangoma_dev = INVALID_HANDLE_VALUE;
	}

#if USE_STELEPHONY_API
	if(stelObj){
		StelCleanup(stelObj);
		stelObj = NULL;
	}
#endif
}

int sangoma_interface::DoManagementCommand(sng_fd_t fd, wan_udp_hdr_t* wan_udp)
{
	return sangoma_mgmt_cmd(fd, wan_udp);
}

int sangoma_interface::tdmv_api_ioctl(wanpipe_api_cmd_t *api_cmd)
{
	int err;

	memset(&wp_api, 0x00, sizeof(wp_api));

	memcpy(&wp_api.wp_cmd, api_cmd, sizeof(wanpipe_api_cmd_t));

	err = sangoma_cmd_exec(sangoma_dev, &wp_api);

	memcpy(api_cmd, &wp_api.wp_cmd, sizeof(wanpipe_api_cmd_t));

	return err;
}

int sangoma_interface::tdm_enable_ring_detect_events()
{
	return sangoma_tdm_enable_ring_detect_events(sangoma_dev,&wp_api);
}

int sangoma_interface::tdm_disable_ring_detect_events()
{
	return sangoma_tdm_disable_ring_detect_events(sangoma_dev,&wp_api);
}


/* To enable flash event set rxflashtime to 1250.
 * 1250 is most used value. To disable flash event set rxflashtime to 0 */
int sangoma_interface::tdm_control_flash_events(int rxflashtime)
{
	return sangoma_set_rm_rxflashtime(sangoma_dev, &wp_api, rxflashtime);
}

int sangoma_interface::tdm_control_rm_txgain(int txgain)
{
#if WP_API_FEATURE_RM_GAIN
	return sangoma_set_rm_tx_gain(sangoma_dev,&wp_api, txgain);
#else
	return SANG_STATUS_UNSUPPORTED_FUNCTION;
#endif
}

int sangoma_interface::tdm_control_rm_rxgain(int rxgain)
{
#if WP_API_FEATURE_RM_GAIN
	return sangoma_set_rm_rx_gain(sangoma_dev, &wp_api, rxgain);
#else
	return SANG_STATUS_UNSUPPORTED_FUNCTION;
#endif
}

int sangoma_interface::tdm_enable_ring_trip_detect_events()
{
	return sangoma_tdm_enable_ring_trip_detect_events(sangoma_dev, &wp_api);
}

int sangoma_interface::tdm_disable_ring_trip_detect_events()
{
	return sangoma_tdm_disable_ring_trip_detect_events(sangoma_dev, &wp_api);
}

int sangoma_interface::tdm_enable_rm_dtmf_events()
{
	return sangoma_tdm_enable_rm_dtmf_events(sangoma_dev, &wp_api);
}

int sangoma_interface::tdm_disable_rm_dtmf_events()
{
	return sangoma_tdm_disable_rm_dtmf_events(sangoma_dev, &wp_api);
}

int sangoma_interface::tdm_enable_dtmf_events(uint8_t channel)
{
	return sangoma_tdm_enable_dtmf_events(sangoma_dev, &wp_api);
}

int sangoma_interface::tdm_disable_dtmf_events(uint8_t channel)
{
	return sangoma_tdm_disable_dtmf_events(sangoma_dev, &wp_api);
}

int sangoma_interface::tdm_enable_rxhook_events()
{
	return sangoma_tdm_enable_rxhook_events(sangoma_dev, &wp_api);
}

int sangoma_interface::tdm_disable_rxhook_events()
{
	return sangoma_tdm_disable_rxhook_events(sangoma_dev, &wp_api);
}

int sangoma_interface::tdm_enable_ring_events()
{
	return sangoma_tdm_enable_ring_events(sangoma_dev, &wp_api);
}

int sangoma_interface::tdm_disable_ring_events()
{
	return sangoma_tdm_disable_ring_events(sangoma_dev, &wp_api);
}

int sangoma_interface::tdm_txsig_onhook()
{
	return sangoma_tdm_txsig_onhook(sangoma_dev, &wp_api);
}

int sangoma_interface::tdm_txsig_kewl()
{
	return sangoma_tdm_txsig_kewl(sangoma_dev, &wp_api);
}

int sangoma_interface::tdm_txsig_onhooktransfer()
{
	return sangoma_tdm_txsig_onhooktransfer(sangoma_dev, &wp_api);
}

int sangoma_interface::tdm_set_rm_polarity(int polarity)
{
	return sangoma_tdm_set_polarity(sangoma_dev, &wp_api, polarity);
}

int sangoma_interface::tdm_txsig_offhook()
{
	return sangoma_tdm_txsig_offhook(sangoma_dev, &wp_api);
}

int sangoma_interface::tdm_enable_tone_events(uint16_t tone_id)
{
	return sangoma_tdm_enable_tone_events(sangoma_dev, &wp_api, tone_id);
}

int sangoma_interface::tdm_disable_tone_events()
{
	return sangoma_tdm_disable_tone_events(sangoma_dev, &wp_api);
}

int sangoma_interface::tdm_enable_bri_bchan_loopback(u_int8_t channel)
{
	return sangoma_enable_bri_bchan_loopback(sangoma_dev, &wp_api, channel);
}

int sangoma_interface::tdm_disable_bri_bchan_loopback(u_int8_t channel)
{
	return sangoma_disable_bri_bchan_loopback(sangoma_dev, &wp_api, channel);
}

/* 1. Enable 'writing' of RBS bits on card.
   2. Enable monitoring change in state of RBS bits.
      'polls_per_second' - how many times per second API driver will check for
      change in state of RBS bits.
      Valid values are between 20 and 100 (including). */
int sangoma_interface::tdm_enable_rbs_events(int polls_per_second)
{
	return sangoma_tdm_enable_rbs_events(sangoma_dev, &wp_api,polls_per_second);
}

/* Stop monitoring change in state of RBS bits */
int sangoma_interface::tdm_disable_rbs_events()
{
	return sangoma_tdm_disable_rbs_events(sangoma_dev, &wp_api);
}

/* Activate ISDN BRI line. */
int sangoma_interface::tdm_front_end_activate()
{
	return sangoma_set_fe_status(sangoma_dev, &wp_api,WAN_FE_CONNECTED);
}

/* De-activate ISDN BRI line. */
int sangoma_interface::tdm_front_end_deactivate()
{
	return sangoma_set_fe_status(sangoma_dev, &wp_api, WAN_FE_DISCONNECTED);
}

/* get current state of the line - is it Connected or Disconnected */
int sangoma_interface::tdm_get_front_end_status(unsigned char *status)
{
	return sangoma_get_fe_status(sangoma_dev, &wp_api, status);
}

/* Milliseconds interval between receive of Voice Data */
int sangoma_interface::tdm_set_user_period(unsigned int usr_period)
{
	return sangoma_tdm_set_usr_period(sangoma_dev, &wp_api, usr_period);
}

int sangoma_interface::stop()
{
	int wait_counter = 0;
	DBG_IFACE("%s()\n", __FUNCTION__);
	INFO_IFACE("Stopping.");

	terminate_tx_rx_threads = 1;

	if(sng_wait_obj){
		sangoma_wait_obj_signal(sng_wait_obj);
	}

	while(terminate_tx_rx_threads == 1 && wait_counter++ < 5){
		INFO_IFACE( ".");
		Sleep(500);
	}
	INFO_IFACE( "\n");

	switch(wanif_conf_struct.media)
	{
	case WAN_MEDIA_FXOFXS:
		switch(wanif_conf_struct.sub_media)
		{
		case MOD_TYPE_FXS:
			stop_all_tones();
			stop_ringing_phone();
			break;

		case MOD_TYPE_FXO:
			fxo_go_on_hook();
			break;
		}
		break;
	}

	return 0;
}

int sangoma_interface::run()
{
	DBG_IFACE("%s()\n", __FUNCTION__);

	switch(wanif_conf_struct.media)
	{
	case WAN_MEDIA_FXOFXS:
		switch(wanif_conf_struct.sub_media)
		{
		case MOD_TYPE_FXS:
			//tdm_control_flash_events(1250); /* enable flash on fxs (optional) */
			break;
		case MOD_TYPE_FXO:
			// Do nothing Special
			break;
		}
		break;
	}

#ifdef WP_API_FEATURE_LIBSNG_HWEC
	if (program_settings.use_hardware_echo_canceller == 1) {

		char strDeviceName[DEV_NAME_LEN];
		if_cfg_t wanif_conf;

		memset(strDeviceName, 0x00, DEV_NAME_LEN);

		//Form the Interface Name from Wanpipe Number and Interface Index (i.e. wanpipe1_if1).
		sangoma_span_chan_toif(WanpipeNumber, InterfaceNumber, strDeviceName);
		INFO_IFACE("Interface Name for HWEC: %s\n", strDeviceName);

		if(get_interface_configuration(&wanif_conf)){
			ERR_IFACE("%s(): get_interface_configuration() failed!", __FUNCTION__);
			return 1;
		}

		//////////////////////////////////////////////////////////////////////////////////////////
		// It is assumed that this interface represents Voice Channel(s), not the d-channel,
		// therefore 'wanif_conf.ec_active_ch' bitmap contains only the Voice Channel(s).
		//////////////////////////////////////////////////////////////////////////////////////////
		if(SANG_STATUS_SUCCESS != config_hwec(strDeviceName, wanif_conf.ec_active_ch)){
			ERR_IFACE("%s(): config_hwec() failed!", __FUNCTION__);
			return 2;
		}
		DBG_IFACE("%s(): HWEC initialization/configuraion was successful.\n", __FUNCTION__);
	}
#endif

	////////////////////////////////////////////////////////////////////////////
	//Start a thread for receiving data only
    if(this->CreateThread(1) == false){
		ERR_IFACE( "Failed to create Rx thread!!\n");
		return 1;
	}

	return 0;
}

unsigned long sangoma_interface::threadFunction (struct ThreadParam& thParam)
{
	switch(thParam.iParam)
	{
		case 1:
			RxThreadFunc();
			return 0;//ok
		case 2:
			TxThreadFunc();
			return 0;//ok
	}
	return 1;//invalid thread function was requested
}

#if USE_STELEPHONY_API
void sangoma_interface::TxStelEncoderBuffer(void *pStelEncoderBuffer)
{
	int datalen;
	int cnt = 0, end = 0;

	/*	Note that when Tone transmission is in progress, no other voice
		data should be transmitted to avoid "splitting" the tone in the middle. */
	EnterCriticalSection(&StelTxCriticalSection);

	while(terminate_tx_rx_threads == 0 && end == 0){
	
		datalen = program_settings.txlength*2;

		if (pStelEncoderBuffer && StelBufferInuse(stelObj, pStelEncoderBuffer)) {
			wp_api_hdr_t	hdr;
			unsigned char	local_tx_data[MAX_NO_DATA_BYTES_IN_FRAME];
			int				dlen = datalen;
			unsigned char	buf[1024];
			size_t			br, max = sizeof(buf);

#if 1
			/* Data can be read as slinear, or ulaw, alaw */
			br = StelBufferReadUlaw(stelObj, pStelEncoderBuffer, buf, &dlen, max);
			if (br < (size_t) dlen) {
				memset(buf+br, 0, dlen - br);
			}
#else
			dlen*=2;

			len = dlen;
			
			br = stelephony_buffer_read(pStelEncoderBuffer, buf, len);
			if (br < dlen) {
				memset(buf+br, 0, dlen - br);
			}

			slin2ulaw(buf, max, (size_t*) &dlen);
#endif
			hdr.data_length = (unsigned short)dlen; 
			hdr.operation_status = SANG_STATUS_TX_TIMEOUT;

			memcpy (local_tx_data, buf, dlen);

			DBG_IFACE("%s: Transmitting Stelephony buffer %d, data length: %d.\n", device_name, cnt, dlen);

			if(transmit(&hdr, local_tx_data) != SANG_STATUS_SUCCESS){
				printf("Failed to TX dlen:%d\n", dlen);
			} else {
				//printf("TX OK (cnt:%d)\n", cnt);
				cnt++;
			}
		}else{
			end = 1;
		}
	}//while()

	LeaveCriticalSection(&StelTxCriticalSection);
}

int sangoma_interface::sendSwDTMF(char dtmf_char)
{
	int rc = 1;

	if (stelObj) {
		rc = StelGenerateSwDTMF(stelObj, dtmf_char);	
	} else {
		printf("Error, no Stelephony object\n");
		rc = -1;
	}
		
	return rc;
}

int sangoma_interface::resetFSKCID(void)
{
	stelephony_option_t codec = (program_settings.voice_codec_alaw == 1 ? STEL_OPTION_ALAW : STEL_OPTION_MULAW);

	/* This is required on each new call */
	/* It is recommended to enable the SW Caller ID after the FIRST ring , 
		and then disable it when the SECOND ring is received to save cpu resources */
	StelEventControl(stelObj, STEL_EVENT_FSK_CALLER_ID, STEL_CTRL_CODE_DISABLE, &codec);
	StelEventControl(stelObj, STEL_EVENT_FSK_CALLER_ID, STEL_CTRL_CODE_ENABLE, &codec);
	return 0;
}

int sangoma_interface::sendCallerID(char * name, char * number)
{
	int rc = 1;

	stelephony_caller_id_t my_caller_id;
	memset(&my_caller_id, 0, sizeof(stelephony_caller_id_t));

#if 1
	/* Library will automatically generate date and time */
	my_caller_id.auto_datetime = 1; 
#else
	/* date and time specified by user application */
	sprintf(my_caller_id.datetime, "01020304");
#endif

	strncpy(my_caller_id.calling_name, name, sizeof(my_caller_id.calling_name)-2);
	strncpy(my_caller_id.calling_number, number, sizeof(my_caller_id.calling_number)-2);

	my_caller_id.calling_name[sizeof(my_caller_id.calling_name)-1] = '\0';
	my_caller_id.calling_number[sizeof(my_caller_id.calling_number)-1] = '\0';
	
	rc = StelGenerateFSKCallerID(stelObj, &my_caller_id);
	return rc;
}

int	sangoma_interface::CreateSwDtmfTxThread(void *buffer)
{
	IFACE_FUNC();
	DtmfBuffer = buffer;
    return this->CreateThread(2);
}

int	sangoma_interface::CreateFskCidTxThread(void *buffer)
{
	IFACE_FUNC();
	FskCidBuffer = buffer;
    return this->CreateThread(2);
}
#endif //#if USE_STELEPHONY_API

// Transmit Thread
void sangoma_interface::TxThreadFunc()
{
	INFO_IFACE("\n%s: %s() - start\n", device_name, __FUNCTION__);

#if USE_STELEPHONY_API
	/* Transmit DTMF buffer, if initialized.*/
	TxStelEncoderBuffer(DtmfBuffer);

	/* Transmit FSK CID buffer, if initialized.*/
	TxStelEncoderBuffer(FskCidBuffer);
#endif

	INFO_IFACE("\n%s: %s() - end\n", device_name, __FUNCTION__);
}

// Read Thread
void sangoma_interface::RxThreadFunc()
{
	int iResult;
	u_int32_t input_flags[NUMBER_OF_WAIT_OBJECTS], output_flags[NUMBER_OF_WAIT_OBJECTS];

	input_flags[0] = (POLLIN | POLLPRI);

	INFO_IFACE("\n%s: %s() - start\n", device_name, __FUNCTION__);
#if defined(__WINDOWS__)
	INFO_IFACE("ThreadID: %d - Start\n", ::GetCurrentThreadId());
#endif

	while(terminate_tx_rx_threads == 0){
		iResult = sangoma_waitfor_many(&sng_wait_obj,
					       input_flags,
					       output_flags,
				           NUMBER_OF_WAIT_OBJECTS,
						   5000 /* Wait timeout, in milliseconds. Or SANGOMA_INFINITE_API_POLL_WAIT. */
						   );
		if(terminate_tx_rx_threads){
			break;
		}

		switch(iResult)
		{
		case SANG_STATUS_APIPOLL_TIMEOUT:
			//timeout. try again.
			DBG_IFACE("%s(): Timeout\n", __FUNCTION__);
			continue;

		case SANG_STATUS_SUCCESS:
			/* a wait object was signaled */
			if(output_flags[0] & POLLIN){
				/* data */
				if(read_data()){
					ERR_IFACE("Error in read_data()!\n");
				}
			}

			if(output_flags[0] & POLLPRI){
				if(read_event()){
					ERR_IFACE("Error in read_event()!\n");
				}
			}

			if(!output_flags[0] || (output_flags[0] & (~input_flags[0])) ){
				WARN_IFACE("\nUnexpected output_flags[0]: 0x%X\n\n", output_flags[0]);
			}
			break;

		default:
			/* error */
			ERR_IFACE("iResult: %s (%d)\n", SDLA_DECODE_SANG_STATUS(iResult), iResult);
			sangoma_msleep(2000);/* don't exit, but put a delay to avoid busy loop */
		}//switch(iResult)
	}//while()

	terminate_tx_rx_threads = 2;

	INFO_IFACE("\n%s: %s() - end\n", device_name, __FUNCTION__);
#if defined(__WINDOWS__)
	INFO_IFACE("ThreadID: %d - End\n", ::GetCurrentThreadId());
#endif
}

///////////////////////////////////////////////////////////////////////

int sangoma_interface::read_event()
{
	int err;

	memset(&wp_api, 0, sizeof(wp_api));

	err = sangoma_read_event(sangoma_dev, &wp_api);
	if(err){
		return err;
	}

	/* an event - such as DTMF, On/Off Hook, Line Connect/Disconnect... */
	wp_api_event_t *rx_event = &wp_api.wp_cmd.event;

	callback_functions.got_tdm_api_event(this, rx_event);

	return 0;
}

///////////////////////////////////////////////////////////////////////

int sangoma_interface::read_data()
{
	int rc = 0;

	if(receive(&rxhdr, rx_data) ){
		//error
		ERR_IFACE( "receive() failed!! Check messages log.\n");
		return 1;
	}

	switch(rxhdr.operation_status)
	{
	case SANG_STATUS_RX_DATA_AVAILABLE:
#if USE_STELEPHONY_API
		if(stelObj){
			if(scf.FSKCallerIDEvent	|| scf.DTMFEvent || scf.Q931Event){
				//if at lease one event is enabled, Rx data is the input for decoder.
				StelStreamInput(stelObj, rx_data, rxhdr.data_length);
			}
		}
#endif
#if 0
		//Some useful information about API's internal receive queue
		//is available after each successful IoctlReadCommand:
		DBG_IFACE("max_rx_queue_length: %d current_number_of_frames_in_rx_queue: %d\n",
			rx_data.hdr.wp_api_rx_hdr_max_queue_length,
			rx_data.hdr.wp_api_rx_hdr_number_of_frames_in_queue);
#endif
		//INFO_IFACE("Rx Length = %i\n",rx_data.hdr.wp_api_hdr_data_length);

		callback_functions.got_rx_data(this, &rxhdr, rx_data);

#if DBG_TIMING
		debug_update_timediff(&wan_debug_rx_timing, __FUNCTION__);
#endif
		break;

	default:
		ERR_IFACE("%s: Rx Error: Operation Status: %s(%d)\n", device_name,
			SDLA_DECODE_SANG_STATUS(rxhdr.operation_status), rxhdr.operation_status);
		rc = 1;
		break;
	}//switch()
	return rc;
}

///////////////////////////////////////////////////////////////////////

int sangoma_interface::receive(wp_api_hdr_t *hdr, void *data)
{
	int err = sangoma_readmsg(sangoma_dev, hdr, sizeof(wp_api_hdr_t), data, MAX_NO_DATA_BYTES_IN_FRAME, 0);

	if(err <= 0){
		//error!
		return 1;
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////

int sangoma_interface::write_data(wp_api_hdr_t *hdr, void *tx_buffer)
{
	if(sangoma_writemsg(sangoma_dev, hdr, sizeof(*hdr), tx_buffer, hdr->data_length, 0) < 0){
		//error
		ERR_IFACE("sangoma_writemsg() failed!\n");
		return SANG_STATUS_IO_ERROR;
	}

	switch(hdr->operation_status)
	{
	case SANG_STATUS_SUCCESS:
		//DBG_IFACE("Frame queued for transmission.\n");
#if 0
		//Some useful information about API's internal transmit queue
		//is available after each successful transmission:
		DBG_IFACE("max_tx_queue_length: %d current_number_of_frames_in_tx_queue: %d\n",
			hdr->tx_h.max_tx_queue_length,
			hdr->tx_h.current_number_of_frames_in_tx_queue);
#endif
		break;
	default:
		ERR_IFACE("Return code: %s (%d) on transmission!\n",
			SDLA_DECODE_SANG_STATUS(hdr->operation_status), hdr->operation_status);
		break;
	}//switch()

	return hdr->operation_status;
}

///////////////////////////////////////////////////////////////////////

int sangoma_interface::transmit(wp_api_hdr_t *hdr, void *data)
{
	int iResult;
	u_int32_t input_flags[NUMBER_OF_WAIT_OBJECTS], output_flags[NUMBER_OF_WAIT_OBJECTS];

	input_flags[0] = (POLLOUT);

	iResult = sangoma_waitfor_many(&sng_wait_obj,
					       input_flags,
					       output_flags,
				           NUMBER_OF_WAIT_OBJECTS,
					       2000 /* wait timeout, in milliseconds */);

	switch(iResult)
	{
	case SANG_STATUS_APIPOLL_TIMEOUT:
		//Timeout. It is possible line is disconnected.
		DBG_IFACE("%s: Tx Timeout!\n", device_name);
		return SANG_STATUS_TX_TIMEOUT;

	case SANG_STATUS_SUCCESS:
		/* a wait object was signaled */
		if(output_flags[0] & POLLOUT){
			/* POLLOUT bit is set - there is at least one free Tx buffer. */
			iResult = write_data(hdr, data);
			if(SANG_ERROR(iResult)){
				ERR_IFACE("Error in write_data()!\n");
			}
		}

		if(!output_flags[0] || (output_flags[0] & (~input_flags[0])) ){
			WARN_IFACE("\nUnexpected output_flags[0]: 0x%X\n\n", output_flags[0]);
		}
		break;

	default:
		/* error */
		ERR_IFACE("iResult: %s (%d)\n", SDLA_DECODE_SANG_STATUS(iResult), iResult);
		if(SANG_STATUS_FAILED_ALLOCATE_MEMORY == iResult){
			INFO_IFACE("Fatal Error. Press any key to exit the application.\n");
			_getch();
			exit(1);
		}
	}//switch(iResult)

	return iResult;
}

///////////////////////////////////////////////////////////////////////
int sangoma_interface::start_ring_tone()
{
	DBG_IFACE("%s:%s()\n", device_name, __FUNCTION__);
	return sangoma_tdm_enable_tone_events(sangoma_dev, &wp_api, WP_API_EVENT_TONE_RING);
}
///////////////////////////////////////////////////////////////////////
int sangoma_interface::start_congestion_tone()
{
	DBG_IFACE("%s:%s()\n", device_name, __FUNCTION__);
	return sangoma_tdm_enable_tone_events(sangoma_dev, &wp_api, WP_API_EVENT_TONE_CONGESTION);
}
///////////////////////////////////////////////////////////////////////
int sangoma_interface::start_busy_tone()
{
	DBG_IFACE("%s:%s()\n", device_name, __FUNCTION__);
	return sangoma_tdm_enable_tone_events(sangoma_dev, &wp_api, WP_API_EVENT_TONE_BUSY);
}

///////////////////////////////////////////////////////////////////////
int sangoma_interface::start_dial_tone()
{
	DBG_IFACE("%s:%s()\n", device_name, __FUNCTION__);
	return sangoma_tdm_enable_tone_events(sangoma_dev, &wp_api, WP_API_EVENT_TONE_DIAL);
}
///////////////////////////////////////////////////////////////////////
int sangoma_interface::stop_all_tones()
{
	DBG_IFACE("%s:%s()\n", device_name, __FUNCTION__);
	return sangoma_tdm_disable_tone_events(sangoma_dev, &wp_api);
}

///////////////////////////////////////////////////////////////////////
int sangoma_interface::start_ringing_phone()
{
	DBG_IFACE("%s:%s()\n", device_name, __FUNCTION__);
	return sangoma_tdm_enable_ring_events(sangoma_dev, &wp_api);
}
int sangoma_interface::stop_ringing_phone()
{
	DBG_IFACE("%s:%s()\n", device_name, __FUNCTION__);
	return sangoma_tdm_disable_ring_events(sangoma_dev, &wp_api);
}
///////////////////////////////////////////////////////////////////////
int sangoma_interface::fxo_go_off_hook()
{
	DBG_IFACE("%s:%s()\n", device_name, __FUNCTION__);

	if(wanif_conf_struct.sub_media != MOD_TYPE_FXO){
		ERR_IFACE( "%s: The 'Go Off Hook' command valid only for FXO!\n", device_name);
		return 1;
	}
	return sangoma_tdm_txsig_offhook(sangoma_dev, &wp_api);
}

///////////////////////////////////////////////////////////////////////
int sangoma_interface::fxs_txsig_offhook()
{
	DBG_IFACE("%s:%s()\n", device_name, __FUNCTION__);
	return sangoma_tdm_txsig_offhook(sangoma_dev,&wp_api);
}

int sangoma_interface::fxo_go_on_hook()
{
	DBG_IFACE("%s:%s()\n", device_name, __FUNCTION__);

	if(wanif_conf_struct.sub_media != MOD_TYPE_FXO){
		ERR_IFACE( "%s: The 'Go On Hook' command valid only for FXO!\n", device_name);
		return 1;
	}
	return sangoma_tdm_txsig_onhook(sangoma_dev,&wp_api);
}

//set a Telephony interface to it's default state
int sangoma_interface::reset_interface_state()
{
	DBG_IFACE("%s:%s()\n", device_name, __FUNCTION__);

	switch(wanif_conf_struct.media)
	{
	case WAN_MEDIA_FXOFXS:
		switch(wanif_conf_struct.sub_media)
		{
		case MOD_TYPE_FXS:
			stop_all_tones();
			stop_ringing_phone();
			break;

		case MOD_TYPE_FXO:
			fxo_go_on_hook();
			break;
		}
		break;
	}
	return 0;
}

//this function for use only by Sangoma Techsupport
void sangoma_interface::set_fe_debug_mode(sdla_fe_debug_t *fe_debug)
{
	int	err = 0;
	unsigned char	*data = NULL;

	wan_udp.wan_udphdr_command	= WAN_FE_SET_DEBUG_MODE;
	wan_udp.wan_udphdr_data_len	= sizeof(sdla_fe_debug_t);
	wan_udp.wan_udphdr_return_code	= 0xaa;

	data = get_wan_udphdr_data_ptr(0);
	memcpy(data, (unsigned char*)fe_debug, sizeof(sdla_fe_debug_t));

	DO_COMMAND(wan_udp);

	if (wan_udp.wan_udphdr_return_code){
		ERR_IFACE( "Error: WAN_FE_SET_DEBUG_MODE failed! return code: 0x%X", 
			wan_udp.wan_udphdr_return_code);
		return;
	}

	if (fe_debug->type == WAN_FE_DEBUG_REG && fe_debug->fe_debug_reg.read == 1){
		if (err == 0 && wan_udp.wan_udphdr_return_code == 0){
			int	cnt = 0;
			printf("Please wait.");fflush(stdout);
repeat_read_reg:
			wan_udp.wan_udphdr_return_code	= 0xaa;
			fe_debug->fe_debug_reg.read = 2;
			memcpy(data, (unsigned char*)fe_debug, sizeof(sdla_fe_debug_t));
			wp_usleep(100000);
			err = DO_COMMAND(wan_udp);
			if (err || wan_udp.wan_udphdr_return_code != 0){
				if (cnt < 5){
					printf(".");fflush(stdout);
					goto repeat_read_reg;
				}
			}
			printf("\n\n");
		}
	}

	if (err || wan_udp.wan_udphdr_return_code != 0){
		if (fe_debug->type == WAN_FE_DEBUG_RBS){
			printf("Failed to %s mode.\n",
				WAN_FE_DEBUG_RBS_DECODE(fe_debug->mode));
		}else{
			printf("Failed to execute debug mode (%02X).\n",
						fe_debug->type);		
		}
	}else{
		fe_debug = (sdla_fe_debug_t*)get_wan_udphdr_data_ptr(0);
		switch(fe_debug->type){
		case WAN_FE_DEBUG_RBS:
			if (fe_debug->mode == WAN_FE_DEBUG_RBS_READ){
				printf("Read RBS status is suceeded!\n");
			}else if (fe_debug->mode == WAN_FE_DEBUG_RBS_SET){
				printf("Setting ABCD bits (%X) for channel %d is suceeded!\n",
						fe_debug->fe_debug_rbs.abcd,
						fe_debug->fe_debug_rbs.channel);
			}else{
				printf("%s debug mode!\n",
					WAN_FE_DEBUG_RBS_DECODE(fe_debug->mode));
			}
			break;
		case WAN_FE_DEBUG_ALARM:
			printf("%s AIS alarm!\n",
					WAN_FE_DEBUG_ALARM_DECODE(fe_debug->mode));
			break;
		case WAN_FE_DEBUG_REG:
			if (fe_debug->fe_debug_reg.read == 2){
				printf("\nRead Front-End Reg:%04X=%02X\n",
						fe_debug->fe_debug_reg.reg,
						fe_debug->fe_debug_reg.value);
			}
			break;
		}
	}
	return;
}

///////////////////////////////////////////////////////////////////////////
// Ctrl Device Class implementation
sangoma_api_ctrl_dev::sangoma_api_ctrl_dev():sangoma_interface(0, 0)
{
	IFACE_FUNC();
	//Initialize Device Name (for debugging only).
	_snprintf(device_name, DEV_NAME_LEN, WP_CTRL_DEV_NAME);
	INFO_IFACE("Using Device Name: %s\n", device_name);
}

sangoma_api_ctrl_dev::~sangoma_api_ctrl_dev()
{
	IFACE_FUNC();
}

int sangoma_api_ctrl_dev::init(callback_functions_t *callback_functions_ptr)
{
	IFACE_FUNC();

	memcpy(&callback_functions, callback_functions_ptr, sizeof(callback_functions_t));

	////////////////////////////////////////////////////////////////////////////
	//open handle for events reception and other commands
	sangoma_dev = sangoma_open_api_ctrl();
    if (sangoma_dev == INVALID_HANDLE_VALUE){
		ERR_IFACE("Unable to open Ctrl Dev!\n");
		return 1;
	}

	if(SANG_ERROR(sangoma_wait_obj_create(&sng_wait_obj, sangoma_dev, SANGOMA_DEVICE_WAIT_OBJ_SIG))){
		ERR_IFACE("Failed to create 'sangoma_wait_object' for %s\n", device_name);
		return 1;
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////
// Logger Device Class implementation
sangoma_api_logger_dev::sangoma_api_logger_dev():sangoma_interface(0, 0)
{
	IFACE_FUNC();
#if USE_WP_LOGGER
	//Initialize Device Name (for debugging only).
	_snprintf(device_name, DEV_NAME_LEN, WP_LOGGER_DEV_NAME);
	INFO_IFACE("Using Device Name: %s\n", device_name);
#endif
}

sangoma_api_logger_dev::~sangoma_api_logger_dev()
{
	IFACE_FUNC();
}

int sangoma_api_logger_dev::init(callback_functions_t *callback_functions_ptr)
{
	IFACE_FUNC();
#if USE_WP_LOGGER
	is_logger_dev = 1;

	memcpy(&callback_functions, callback_functions_ptr, sizeof(callback_functions_t));

	////////////////////////////////////////////////////////////////////////////
	//open handle for Logger Events reception and other commands
	sangoma_dev = sangoma_logger_open();
    if (sangoma_dev == INVALID_HANDLE_VALUE){
		ERR_IFACE("Unable to open Ctrl Dev!\n");
		return 1;
	}

	if(SANG_ERROR(sangoma_wait_obj_create(&sng_wait_obj, sangoma_dev, SANGOMA_DEVICE_WAIT_OBJ_SIG))){
		ERR_IFACE("Failed to create 'sangoma_wait_object' for %s\n", device_name);
		return 1;
	}

	////////////////////////////////////////////////////////////////////////////
	//get Current Level of Default Logger
	memset(&logger_cmd, 0x00, sizeof(logger_cmd));
	logger_cmd.logger_level_ctrl.logger_type = WAN_LOGGER_DEFAULT;
	if(sangoma_logger_get_logger_level(sangoma_dev, &logger_cmd)){
		ERR_IFACE("WP_API_CMD_GET_LOGGER_LEVEL failed for %s!\n", device_name);
		return 1;
	}else{
		INFO_IFACE("%s: Current logger level: 0x%08X\n", 
			device_name, logger_cmd.logger_level_ctrl.logger_level);
	}

	////////////////////////////////////////////////////////////////////////////
	//set New Level for Default Logger.
	logger_cmd.logger_level_ctrl.logger_type = WAN_LOGGER_DEFAULT;
	logger_cmd.logger_level_ctrl.logger_level = 
		(SANG_LOGGER_INFORMATION | SANG_LOGGER_WARNING | SANG_LOGGER_ERROR);
	if(sangoma_logger_set_logger_level(sangoma_dev, &logger_cmd)){
		ERR_IFACE("WP_API_CMD_SET_LOGGER_LEVEL failed for %s!\n", device_name);
		return 1;
	}

#if 0
	//enable advanced debugging messages for T1/E1
	memset(&logger_cmd, 0x00, sizeof(logger_cmd));
	logger_cmd.logger_level_ctrl.logger_type = WAN_LOGGER_TE1;
	logger_cmd.logger_level_ctrl.logger_level = (SANG_LOGGER_TE1_DEFAULT);
	if(sangoma_logger_set_logger_level(sangoma_dev, &logger_cmd)){
		ERR_IFACE("WP_API_CMD_SET_LOGGER_LEVEL failed for %s!\n", device_name);
		return 1;
	}
#endif

#if 0
	//enable advanced debugging messages for Hardware Echo Canceller
	memset(&logger_cmd, 0x00, sizeof(logger_cmd));
	logger_cmd.logger_level_ctrl.logger_type = WAN_LOGGER_HWEC;
	logger_cmd.logger_level_ctrl.logger_level = (SANG_LOGGER_HWEC_DEFAULT);
	if(sangoma_logger_set_logger_level(sangoma_dev, &logger_cmd)){
		ERR_IFACE("WP_API_CMD_SET_LOGGER_LEVEL failed for %s!\n", device_name);
		return 1;
	}
#endif

#endif//USE_WP_LOGGER
	return 0;
}

int sangoma_api_logger_dev::read_event()
{
	int err=1;

	memset(&logger_cmd, 0, sizeof(logger_cmd));

#if USE_WP_LOGGER
	err = sangoma_logger_read_event(sangoma_dev, &logger_cmd);
#endif
	if(err){
		return err;
	}

	/* Wanpipe Logger Event */
#if USE_WP_LOGGER
	wp_logger_event_t *logger_event = &logger_cmd.logger_event;
	callback_functions.got_logger_event(this, logger_event);
#endif

	return 0;
}

int sangoma_api_logger_dev::flush_operational_stats(void)
{
#if USE_WP_LOGGER
	return sangoma_logger_reset_statistics(sangoma_dev, &logger_cmd);
#else
	return 1;
#endif
}

int sangoma_api_logger_dev::get_logger_dev_operational_stats(wp_logger_stats_t *stats)
{
	int err=1;
#if USE_WP_LOGGER
	err = sangoma_logger_get_statistics(sangoma_dev, &logger_cmd);
	if(err){
		ERR_IFACE("sangoma_logger_get_statistics() failed (err: %d (0x%X))!\n", err, err);
		return err;
	}

	memcpy(stats, &logger_cmd.stats, sizeof(*stats));

	INFO_IFACE("*** Logger Device Statistics ***\n");
	INFO_IFACE("\trx_events\t: %u\n", stats->rx_events);
	INFO_IFACE("\trx_events_dropped\t: %u\n", stats->rx_events_dropped);
	INFO_IFACE("\tmax_event_queue_length\t: %u\n", stats->max_event_queue_length);
	INFO_IFACE("\tcurrent_number_of_events_in_event_queue\t: %u\n", stats->current_number_of_events_in_event_queue);
	INFO_IFACE("\n");
#endif
	return err;
}

#ifdef WP_API_FEATURE_LIBSNG_HWEC
/* NOTE: HWEC must NOT be enabled on a d-channel. The 'in_ulChannelMap' should be 
 *		the bitmap of voice channels only. */
sangoma_status_t config_hwec(char *strDeviceName, unsigned long in_ulChannelMap)
{
	sangoma_status_t rc;

	DBG_IFACE("%s(): strDeviceName: %s, in_ulChannelMap: 0x%X\n", __FUNCTION__, strDeviceName, in_ulChannelMap);

	// Initialize the echo canceller (done once per-physical card)
	rc = sangoma_hwec_config_init(strDeviceName); 
	if (SANG_STATUS_SUCCESS != rc) {
		ERR_IFACE( "Failed to initialize echo canceller. rc: %d\n", rc);    
		return rc;
	}

	// Enable DTMF detection
	rc = sangoma_hwec_config_tone_detection(strDeviceName,
							WP_API_EVENT_TONE_DTMF,
							1 /* enable */,
							in_ulChannelMap,
							WAN_EC_CHANNEL_PORT_SOUT	//(detection of the DTMF coming from the PSTN only Sin -> Sout path)
							);
	if (SANG_STATUS_SUCCESS != rc) {
		ERR_IFACE( "Failed to enable DTMF detection on echo canceller device.\n" ); 
		return rc;
	}

	// Enable FAX detection
	// When FAX is detected, a DTMF event will occur, and the detected digit will be 'f'.
	rc = sangoma_hwec_config_tone_detection(strDeviceName,
							WP_API_EVENT_TONE_FAXCALLED,//incoming fax detection OR
														//WP_API_EVENT_TONE_FAXCALLING - outgoing fax detection
							1 /* enable */,
							in_ulChannelMap,
							WAN_EC_CHANNEL_PORT_SOUT	// (detection of the fax tone coming from the PSTN only Sin -> Sout path)
							);
	if (SANG_STATUS_SUCCESS != rc) {
		ERR_IFACE( "Failed to enable FAX detection on echo canceller device.\n" ); 
		return rc;
	}

#if 0
/* "WANEC_SoutAdaptiveNoiseReduction", "WANEC_TailDisplacement", "WANEC_DtmfToneRemoval", "WANEC_AcousticEcho"...*/

	// Optionally, Enable noise reduction from PSTN
	rc = sangoma_hwec_config_channel_parameters(strDeviceName,
									"WANEC_SoutAdaptiveNoiseReduction",
									"TRUE",
									in_ulChannelMap	);
	if (SANG_STATUS_SUCCESS != rc) {
		ERR_IFACE( "Failed to enable noise reduction.\n" ); 
		return rc;
	}
#endif

	return SANG_STATUS_SUCCESS;
}
#endif// WP_API_FEATURE_LIBSNG_HWEC
