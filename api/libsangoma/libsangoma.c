/*****************************************************************************
 * libsangoma.c	AFT T1/E1: HDLC API Code Library
 *
 * Author(s):	Anthony Minessale II <anthmct@yahoo.com>
 *              Nenad Corbic <ncorbic@sangoma.com>
 *
 * Copyright:	(c) 2006 Nenad Corbic <ncorbic@sangoma.com>
 * 		         Anthony Minessale II
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 * ============================================================================
 */

#include "libsangoma.h"
#include "g711.h"
#define DFT_CARD "wanpipe1"

void sangoma_socket_close(int *sp) 
{
    if (*sp > -1) {
	close(*sp);
	*sp = -1;
    }
}

int sangoma_socket_waitfor(int fd, int timeout, int flags)
{
    struct pollfd pfds[1];
    int res;

    memset(&pfds[0], 0, sizeof(pfds[0]));
    pfds[0].fd = fd;
    pfds[0].events = flags;
    res = poll(pfds, 1, timeout);
    if (res > 0) {
	if ((pfds[0].revents & POLLERR)) {
		res = -1;
	} else if((pfds[0].revents)) {
		res = 1;
	}
    }

    return res;
}


int sangoma_span_chan_toif(int span, int chan, char *interface)
{
 	sprintf(interface,"s%ic%i",span,chan);
	return 0;
}

int sangoma_interface_toi(char *interface, int *span, int *chan)
{
	char *data, *p, *sp = NULL, *ch = NULL;
	int ret = 0;
	
	if ((data = strdupa(interface))) {
		for (p = data; *p; p++) {
			if (sp && *p == 'g') {
				*p = '\0';
				ch = (p + 1);
				break;
			} else if (*p == 'w') {
				sp = (p + 1);
			}
		}

		if(ch && sp) {
			*span = atoi(sp);
			*chan = atoi(ch);
			ret = 1;
		} else {
			*span = -1;
			*chan = -1;
		}
	}

	return ret;
}

int sangoma_span_chan_fromif(char *interface, int *span, int *chan)
{
	char *data, *p, *sp = NULL, *ch = NULL;
	int ret = 0;
	
	if ((data = strdupa(interface))) {
		for (p = data; *p; p++) {
			if (sp && *p == 'c') {
				*p = '\0';
				ch = (p + 1);
				break;
			} else if (*p == 's') {
				sp = (p + 1);
			}
		}

		if(ch && sp) {
			*span = atoi(sp);
			*chan = atoi(ch);
			ret = 1;
		} else {
			*span = -1;
			*chan = -1;
		}
	}

	return ret;
}

int sangoma_open_tdmapi_span_chan(int span, int chan) 
{

    	int fd=-1;
    	unsigned char fname[50];

	sprintf(fname,"/dev/wptdm_s%dc%d",span,chan);
	
	fd = open(fname, O_RDWR);

	return fd;  
}            

int sangoma_create_socket_by_name(char *device, char *card) 
{
	int span,chan;
	sangoma_interface_toi(device,&span,&chan);
	
	return sangoma_open_tdmapi_span_chan(span,chan);
}

          
int sangoma_open_tdmapi_span(int span) 
{

    int fd;
    unsigned char fname[50];
    int i;

	for (i=1;i<32;i++){
		sprintf(fname,"/dev/wptdm_s%dc%d",span,i);
		fd = open(fname, O_RDWR);
		if (fd < 0){
         		continue;
		}
		break;
	}
	
    return fd;  
}      

int sangoma_readmsg_tdm(int fd, void *hdrbuf, int hdrlen, void *databuf, int datalen, int flag)
{
	int rx_len;
	struct msghdr msg;
	struct iovec iov[2];

	memset(&msg,0,sizeof(struct msghdr));

	iov[0].iov_len=hdrlen;
	iov[0].iov_base=hdrbuf;

	iov[1].iov_len=datalen;
	iov[1].iov_base=databuf;

	msg.msg_iovlen=2;
	msg.msg_iov=iov;

	rx_len = read(fd,&msg,datalen+hdrlen);

	if (rx_len <= sizeof(wp_tdm_api_rx_hdr_t)){
		return -EINVAL;
	}

	rx_len-=sizeof(wp_tdm_api_rx_hdr_t);

    return rx_len;
}                    

int sangoma_writemsg_tdm(int fd, void *hdrbuf, int hdrlen, void *databuf, int datalen, int flag)
{
	int bsent;
	struct msghdr msg;
	struct iovec iov[2];

	memset(&msg,0,sizeof(struct msghdr));

	iov[0].iov_len=hdrlen;
	iov[0].iov_base=hdrbuf;

	iov[1].iov_len=datalen;
	iov[1].iov_base=databuf;

	msg.msg_iovlen=2;
	msg.msg_iov=iov;

	bsent = write(fd,&msg,datalen+hdrlen);
	if (bsent > 0){
		bsent-=sizeof(wp_tdm_api_tx_hdr_t);
	}

	return bsent;
}


#ifdef WANPIPE_TDM_API

/*========================================================
 * Execute TDM command
 *
 */
static int sangoma_tdm_cmd_exec(int fd, wanpipe_tdm_api_t *tdm_api)
{
	int err;

	err = ioctl(fd,SIOC_WANPIPE_TDM_API,&tdm_api->wp_tdm_cmd);
	if (err < 0){
#if 0
		char tmp[50];
		sprintf(tmp,"TDM API: CMD: %i\n",tdm_api->wp_tdm_cmd.cmd);
		perror(tmp);
#endif
		return -1;
	}

	return err;
}

/*========================================================
 * Get Full TDM API configuration per channel
 *
 */
int sangoma_get_full_cfg(int fd, wanpipe_tdm_api_t *tdm_api)
{
	int err;

	tdm_api->wp_tdm_cmd.cmd = SIOC_WP_TDM_GET_FULL_CFG;

	err=sangoma_tdm_cmd_exec(fd,tdm_api);
	if (err){
		return err;
	}

	printf("TDM API CFG:\n");
	printf("\thw_tdm_coding:\t%d\n",tdm_api->wp_tdm_cmd.hw_tdm_coding);
	printf("\tusr_mtu_mru:\t%d\n",tdm_api->wp_tdm_cmd.hw_mtu_mru);
	printf("\tusr_period:\t%d\n",tdm_api->wp_tdm_cmd.usr_period);
	printf("\ttdm_codec:\t%d\n",tdm_api->wp_tdm_cmd.tdm_codec);
	printf("\tpower_level:\t%d\n",tdm_api->wp_tdm_cmd.power_level);
	printf("\trx_disable:\t%d\n",tdm_api->wp_tdm_cmd.rx_disable);
	printf("\ttx_disable:\t%d\n",tdm_api->wp_tdm_cmd.tx_disable);
	printf("\tusr_mtu_mru:\t%d\n",tdm_api->wp_tdm_cmd.usr_mtu_mru);
	printf("\tidle flag:\t0x%02X\n",tdm_api->wp_tdm_cmd.idle_flag);
	
	printf("\trx pkt\t%d\ttx pkt\t\%d\n",tdm_api->wp_tdm_cmd.stats.rx_packets,
				tdm_api->wp_tdm_cmd.stats.tx_packets);
	printf("\trx err\t%d\ttx err\t%d\n",
				tdm_api->wp_tdm_cmd.stats.rx_errors,
				tdm_api->wp_tdm_cmd.stats.tx_errors);
	printf("\trx ovr\t%d\ttx idl\t%d\n",
				tdm_api->wp_tdm_cmd.stats.rx_fifo_errors,
				tdm_api->wp_tdm_cmd.stats.tx_carrier_errors);
				
	
	return 0;
}

/*========================================================
 * SET Codec on a particular Channel.
 * 
 * Available codecs are defined in 
 * /usr/src/linux/include/linux/wanpipe_cfg.h
 * 
 * enum wan_codec_format {
 *  	WP_NONE,
 *	WP_SLINEAR
 * }
 *
 */
int sangoma_tdm_set_codec(int fd, wanpipe_tdm_api_t *tdm_api, int codec)
{
	int err;

	tdm_api->wp_tdm_cmd.cmd = SIOC_WP_TDM_SET_CODEC;
	tdm_api->wp_tdm_cmd.tdm_codec = codec;

	err=sangoma_tdm_cmd_exec(fd,tdm_api);

	return err;
}

/*========================================================
 * GET Codec from a particular Channel.
 * 
 * Available codecs are defined in 
 * /usr/src/linux/include/linux/wanpipe_cfg.h
 * 
 * enum wan_codec_format {
 *  	WP_NONE,
 *	WP_SLINEAR
 * }
 *
 */
int sangoma_tdm_get_codec(int fd, wanpipe_tdm_api_t *tdm_api)
{
	int err;

	tdm_api->wp_tdm_cmd.cmd = SIOC_WP_TDM_GET_CODEC;

	err=sangoma_tdm_cmd_exec(fd,tdm_api);
	if (err){
		return err;
	}

	return tdm_api->wp_tdm_cmd.tdm_codec;	
}


/*========================================================
 * SET Rx/Tx Hardware Period in milliseconds.
 * 
 * Available options are:
 *  10,20,30,40,50 ms      
 *
 */
int sangoma_tdm_set_usr_period(int fd, wanpipe_tdm_api_t *tdm_api, int period)
{
	int err;

	tdm_api->wp_tdm_cmd.cmd = SIOC_WP_TDM_SET_USR_PERIOD;
	tdm_api->wp_tdm_cmd.usr_period = period;

	err=sangoma_tdm_cmd_exec(fd,tdm_api);

	return err;
}

/*========================================================
 * GET Rx/Tx Hardware Period in milliseconds.
 * 
 * Available options are:
 *  10,20,30,40,50 ms      
 *
 */
int sangoma_tdm_get_usr_period(int fd, wanpipe_tdm_api_t *tdm_api)
{
	int err;

	tdm_api->wp_tdm_cmd.cmd = SIOC_WP_TDM_GET_USR_PERIOD;

	err=sangoma_tdm_cmd_exec(fd,tdm_api);
	if (err){
		return err;
	}

	return tdm_api->wp_tdm_cmd.usr_period;
}

/*========================================================
 * GET Current User MTU/MRU values in bytes.
 * 
 * The USER MTU/MRU values will change each time a PERIOD
 * or CODEC is adjusted.
 */
int sangoma_tdm_get_usr_mtu_mru(int fd, wanpipe_tdm_api_t *tdm_api)
{
	int err;

	tdm_api->wp_tdm_cmd.cmd = SIOC_WP_TDM_GET_USR_MTU_MRU;

	err=sangoma_tdm_cmd_exec(fd,tdm_api);
	if (err){
		return err;
	}

	return tdm_api->wp_tdm_cmd.usr_mtu_mru;
}

/*========================================================
 * SET TDM Power Level
 * 
 * This option is not implemented yet
 *
 */
int sangoma_tdm_set_power_level(int fd, wanpipe_tdm_api_t *tdm_api, int power)
{
	int err;

	tdm_api->wp_tdm_cmd.cmd = SIOC_WP_TDM_SET_POWER_LEVEL;
	tdm_api->wp_tdm_cmd.power_level = power;

	err=sangoma_tdm_cmd_exec(fd,tdm_api);

	return err;
}

/*========================================================
 * GET TDM Power Level
 * 
 * This option is not implemented yet
 *
 */
int sangoma_tdm_get_power_level(int fd, wanpipe_tdm_api_t *tdm_api)
{
	int err;

	tdm_api->wp_tdm_cmd.cmd = SIOC_WP_TDM_GET_POWER_LEVEL;

	err=sangoma_tdm_cmd_exec(fd,tdm_api);
	if (err){
		return err;
	}

	return tdm_api->wp_tdm_cmd.power_level;
}

int sangoma_tdm_flush_bufs(int fd, wanpipe_tdm_api_t *tdm_api)
{
	return 0;
	
#if 0
	tdm_api->wp_tdm_cmd.cmd = SIOC_WP_TDM_FLUSH_BUFFERS;

	err=sangoma_tdm_cmd_exec(fd,tdm_api);
	if (err){
		return err;
	}
#endif
	return 0;
}

int sangoma_tdm_enable_rbs_events(int fd, wanpipe_tdm_api_t *tdm_api, int poll_in_sec) {
	
	int err;
	
	tdm_api->wp_tdm_cmd.cmd = SIOC_WP_TDM_ENABLE_RBS_EVENTS;
	tdm_api->wp_tdm_cmd.rbs_poll=poll_in_sec; 
	
	err=sangoma_tdm_cmd_exec(fd,tdm_api);
	if (err){
		return err;
	}

	return tdm_api->wp_tdm_cmd.rbs_poll;
}


int sangoma_tdm_disable_rbs_events(int fd, wanpipe_tdm_api_t *tdm_api) {

	int err;
	tdm_api->wp_tdm_cmd.cmd = SIOC_WP_TDM_DISABLE_RBS_EVENTS;
	
	err=sangoma_tdm_cmd_exec(fd,tdm_api);
	if (err){
		return err;
	}

	return 0;
}

int sangoma_tdm_write_rbs(int fd, wanpipe_tdm_api_t *tdm_api, unsigned char rbs) 
{
	
	int err;
	tdm_api->wp_tdm_cmd.cmd = SIOC_WP_TDM_WRITE_RBS_BITS;
	tdm_api->wp_tdm_cmd.rbs_tx_bits=rbs; 
	
	err=sangoma_tdm_cmd_exec(fd,tdm_api);
	if (err){
		return err;
	}

	return 0;
}        

static int __sangoma_tdm_set_gain(unsigned char *gain_array, int ulaw, int gain)
{
        int j;
	int k;
	float linear_gain = pow(10.0, gain / 20.0);

	if (ulaw == WP_MULAW) {
                for (j = 0; j < 256; j++) {
			if (gain) {
				k = (int) (((float) ulaw_to_linear(j)) * linear_gain);
				if (k > 32767) k = 32767;
				if (k < -32767) k = -32767;
				gain_array[j] = linear_to_ulaw(k);
			} else {
				gain_array[j] = j;
			}
		} 
	} else {
		for (j = 0; j < 256; j++) {
			if (gain) {
				k = (int) (((float) alaw_to_linear(j)) * linear_gain);
				if (k > 32767) k = 32767;
				if (k < -32767) k = -32767;
				gain_array[j] = linear_to_alaw(k);
			} else {
				gain_array[j] = j;
			}
		}
	} 
	return 0;
}

int sangoma_tdm_set_rx_gain(int fd, wanpipe_tdm_api_t *tdm_api, int gain)
{ 
	int len=256;
	int err=0;

	if (gain == 0) {
        	tdm_api->wp_tdm_cmd.cmd = SIOC_WP_TDM_CLEAR_RX_GAINS;
		err=sangoma_tdm_cmd_exec(fd,tdm_api);
		return err;
	}
	
	tdm_api->wp_tdm_cmd.cmd = SIOC_WP_TDM_GET_HW_CODING;
	err=sangoma_tdm_cmd_exec(fd,tdm_api);
	if (err){
		return err;
	}
	
        tdm_api->wp_tdm_cmd.data=malloc(len);
	if (!tdm_api->wp_tdm_cmd.data) {
         	return -1;
	}

	tdm_api->wp_tdm_cmd.data_len=len;
	__sangoma_tdm_set_gain(tdm_api->wp_tdm_cmd.data,tdm_api->wp_tdm_cmd.hw_tdm_coding,gain);
	
	
	tdm_api->wp_tdm_cmd.cmd = SIOC_WP_TDM_SET_RX_GAINS;
	err=sangoma_tdm_cmd_exec(fd,tdm_api);

	free(tdm_api->wp_tdm_cmd.data);
	tdm_api->wp_tdm_cmd.data=NULL;

	if (err){
		return err;
	} 

	return 0;        
				
}

int sangoma_tdm_get_hw_coding(int fd, wanpipe_tdm_api_t *tdm_api)
{
	int err;
	tdm_api->wp_tdm_cmd.cmd = SIOC_WP_TDM_GET_HW_CODING;
        err=sangoma_tdm_cmd_exec(fd,tdm_api);
        if (err){
                return err;
        }
	return tdm_api->wp_tdm_cmd.hw_tdm_coding;
}

int sangoma_tdm_set_tx_gain(int fd, wanpipe_tdm_api_t *tdm_api, int gain)
{ 
	int len=256;
        int err=0;
	
        if (gain == 0) {
        	tdm_api->wp_tdm_cmd.cmd = SIOC_WP_TDM_CLEAR_TX_GAINS;
		err=sangoma_tdm_cmd_exec(fd,tdm_api);
		return err;
	}

	
	tdm_api->wp_tdm_cmd.cmd = SIOC_WP_TDM_GET_HW_CODING;
	err=sangoma_tdm_cmd_exec(fd,tdm_api);
	if (err){
		return err;
	}
	
        tdm_api->wp_tdm_cmd.data=malloc(len);
	if (!tdm_api->wp_tdm_cmd.data) {
         	return -1;
	}

	tdm_api->wp_tdm_cmd.data_len=len;
	__sangoma_tdm_set_gain(tdm_api->wp_tdm_cmd.data,tdm_api->wp_tdm_cmd.hw_tdm_coding,gain);
	
	tdm_api->wp_tdm_cmd.cmd = SIOC_WP_TDM_SET_TX_GAINS;
	err=sangoma_tdm_cmd_exec(fd,tdm_api);
	
	free(tdm_api->wp_tdm_cmd.data);
	tdm_api->wp_tdm_cmd.data=NULL;
	
       	return err;
} 

int sangoma_tdm_read_event(int fd, wanpipe_tdm_api_t *tdm_api) 
{
	int err;
	wp_tdm_api_rx_hdr_t *rx_event;
	
	tdm_api->wp_tdm_cmd.cmd = SIOC_WP_TDM_READ_EVENT;
	
	err=sangoma_tdm_cmd_exec(fd,tdm_api);
	if (err){
		return err;
	}

	rx_event = &tdm_api->wp_tdm_cmd.event;
	
	switch (rx_event->wp_tdm_api_event_type){
	
	case WP_TDM_EVENT_RBS:
		//printf("%d: GOT RBS EVENT %p\n",fd,tdm_api->wp_tdm_event.wp_rbs_event);
		if (tdm_api->wp_tdm_event.wp_rbs_event) {
			tdm_api->wp_tdm_event.wp_rbs_event(fd,rx_event->wp_tdm_api_event_rbs_rx_bits);
		}
		
		break;
		
	case WP_TDM_EVENT_DTMF:
		printf("%d: GOT DTMF EVENT\n",fd);
		//if (tdm_api->wp_tdm_event.wp_dtmf_event) {
		//	//tdm_api->wp_tdm_event.wp_dtmf_event(fd,NULL);
		//}
		
		break;
		
	}
	
	return 0;
}        

#endif /* WANPIPE_TDM_API */
