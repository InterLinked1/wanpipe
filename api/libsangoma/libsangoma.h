/*****************************************************************************
 * libsangoma.c	AFT T1/E1: HDLC API Code Library
 *
 * Author(s):	Anthony Minessale II <anthmct@yahoo.com>
 *              Nenad Corbic <ncorbic@sangoma.com>
 *
 * Copyright:	(c) 2006 Nenad Corbic <ncorbic@sangoma.com>
 *                       Anthony Minessale II
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 * ============================================================================
 */

#ifndef _LIBSNAGOMA_H
#define _LIBSNAGOMA_H

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <linux/if.h>
#include <linux/wanpipe_defines.h>
#include <linux/wanpipe_cfg.h>
#include <linux/wanpipe.h>
#include <poll.h>
#include <math.h>

#define WANPIPE_TDM_API 1

#ifdef WANPIPE_TDM_API
# include <linux/wanpipe_tdm_api.h>
#endif

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/signal.h>
#include <sys/select.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <sys/time.h>

typedef wp_tdm_api_rx_hdr_t sangoma_api_hdr_t;

/* Decodec Span/Chan from interface name */
int sangoma_span_chan_toif(int span, int chan, char *interface);
int sangoma_span_chan_fromif(char *interface, int *span, int *chan);
int sangoma_interface_toi(char *interface, int *span, int *chan);

int sangoma_create_socket_by_name(char *device, char *card);

/* Open Span/Chan devices
 * open_tdmapi_span_chan: open device based on span chan values 
 * sangoma_open_tdmapi_span: open first available device on span
 */     

int sangoma_open_tdmapi_span_chan(int span, int chan);
int sangoma_open_tdmapi_span(int span);

#define sangoma_create_socket_intr sangoma_open_tdmapi_span_chan

/* Device Rx/Tx functions 
 * writemsg_tdm: 	tx header + data from separate buffers 
 * readmsg_tdm: 	rx header + data to separate buffers
 */    
int sangoma_writemsg_tdm(int fd, void *hdrbuf, int hdrlen, 
						 void *databuf, int datalen, int flag);
int sangoma_readmsg_tdm(int fd, void *hdrbuf, int hdrlen, 
						void *databuf, int datalen, int flag);

#define sangoma_readmsg_socket sangoma_readmsg_tdm
#define sangoma_sendmsg_socket sangoma_writemsg_tdm

#ifdef WANPIPE_TDM_API

void sangoma_socket_close(int *sp);
int sangoma_socket_waitfor(int fd, int timeout, int flags);

/* Get Full TDM API configuration per chan */
int sangoma_get_full_cfg(int fd, wanpipe_tdm_api_t *tdm_api);

/* Get/Set TDM Codec per chan */
int sangoma_tdm_set_codec(int fd, wanpipe_tdm_api_t *tdm_api, int codec);
int sangoma_tdm_get_codec(int fd, wanpipe_tdm_api_t *tdm_api);

/* Get/Set USR Tx/Rx Period in milliseconds */
int sangoma_tdm_set_usr_period(int fd, wanpipe_tdm_api_t *tdm_api, int period);
int sangoma_tdm_get_usr_period(int fd, wanpipe_tdm_api_t *tdm_api);

/* Get user MTU/MRU values in bytes */
int sangoma_tdm_get_usr_mtu_mru(int fd, wanpipe_tdm_api_t *tdm_api);

/* Not supported yet */
int sangoma_tdm_set_power_level(int fd, wanpipe_tdm_api_t *tdm_api, int power);
int sangoma_tdm_get_power_level(int fd, wanpipe_tdm_api_t *tdm_api);

/* Flush buffers from current channel */
int sangoma_tdm_flush_bufs(int fd, wanpipe_tdm_api_t *tdm_api);

int sangoma_tdm_enable_rbs_events(int fd, wanpipe_tdm_api_t *tdm_api, int poll_in_sec);
int sangoma_tdm_disable_rbs_events(int fd, wanpipe_tdm_api_t *tdm_api);

int sangoma_tdm_write_rbs(int fd, wanpipe_tdm_api_t *tdm_api, unsigned char rbs);

int sangoma_tdm_read_event(int fd, wanpipe_tdm_api_t *tdm_api);

#ifndef LIBSANGOMA_TX_RX_GAIN
#define LIBSANGOMA_TX_RX_GAIN 1
#endif
int sangoma_tdm_set_tx_gain(int fd, wanpipe_tdm_api_t *tdm_api, int gain);
int sangoma_tdm_set_rx_gain(int fd, wanpipe_tdm_api_t *tdm_api, int gain);


#ifndef LIBSANGOMA_GET_HWCODING
#define LIBSANGOMA_GET_HWCODING 1
#endif
int sangoma_tdm_get_hw_coding(int fd, wanpipe_tdm_api_t *tdm_api);


#endif 	/* WANPIPE_TDM_API */

#endif
