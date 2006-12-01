/******************************************************************************
 * sdla_tdmv.h	
 *
 * Author: 	Alex Feldman  <al.feldman@sangoma.com>
 *
 * Copyright:	(c) 1995-2001 Sangoma Technologies Inc.
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 * ============================================================================
 ******************************************************************************
 */

#ifndef __SDLA_TDMV_H
# define __SDLA_TDMV_H

#ifdef __SDLA_TDMV_SRC
# define EXTERN
#else
# define EXTERN extern
#endif


/******************************************************************************
**			  DEFINES and MACROS
******************************************************************************/

#if defined(WAN_KERNEL)

#if defined(__FreeBSD__) || defined(__OpenBSD__)
# if defined(CONFIG_PRODUCT_WANPIPE_TDM_VOICE_ECHOMASTER)
#  include <wanpipe_edac_iface.h>
# endif
#else
# if defined(CONFIG_PRODUCT_WANPIPE_TDM_VOICE_ECHOMASTER)
#  include <linux/wanpipe_edac_iface.h>
# endif
#endif

/******************************************************************************
**			  DEFINES and MACROS
******************************************************************************/

#define WAN_TDMV_IDLE_FLAG	0x7F

#define WAN_TDMV_CALL(func, args, err)					\
	if (card->tdmv_iface.func){					\
		err = card->tdmv_iface.func args;			\
	}else{								\
		DEBUG_EVENT("%s: Internal Error (%s:%d)!\n",\
				card->devname,				\
				__FUNCTION__,__LINE__);			\
		err = -EINVAL;						\
	}
			

/******************************************************************************
**			  TYPEDEF STRUCTURE
******************************************************************************/
typedef struct wan_tdmv_ 
{
	void	*sc;
	int	max_tx_len;
	int	max_timeslots;
	int	brt_enable;
	int	sig_intr_enable;
	int	spanno;
#if defined(CONFIG_PRODUCT_WANPIPE_TDM_VOICE_ECHOMASTER)
	wan_tdmv_rxtx_pwr_t chan_pwr[31];	
#endif
	WAN_LIST_ENTRY(wan_tdmv_)	next;
} wan_tdmv_t;

typedef struct wan_tdmv_iface_
{
	int	(*check_mtu)(void*, unsigned long, int *);
	int	(*create)(void* pcard, wan_xilinx_conf_t *conf);
	int	(*remove)(void* pcard);
	int	(*reg)(void*, wanif_conf_t*, netdevice_t*);
	int	(*unreg)(void* pcard, unsigned long ts_map);
	int	(*software_init)(wan_tdmv_t*);
	int 	(*state)(void*, int);
	int	(*running)(void*);
	int	(*rx_tx)(void*, netskb_t*);
	int	(*rx_chan)(wan_tdmv_t*, int, unsigned char*, unsigned char*); 
#if defined(CONFIG_PRODUCT_WANPIPE_TDM_VOICE_DCHAN)
	int	(*rx_dchan)(wan_tdmv_t*, int, unsigned char*, unsigned int); 
#endif
	int	(*rx_tx_span)(void *pcard);
	int	(*is_rbsbits)(wan_tdmv_t *);
	int	(*rbsbits_poll)(wan_tdmv_t *, void*);
	int	(*init)(void*, wanif_conf_t*);
	int	(*free)(wan_tdmv_t*);
	int	(*polling)(void*);
	int	(*buf_rotate)(void *pcard,u32,unsigned long);
	int	(*ec_span)(void *pcard);

} wan_tdmv_iface_t;

/******************************************************************************
**			  FUNCTION PROTOTYPES
******************************************************************************/
EXTERN int wp_tdmv_te1_init(wan_tdmv_iface_t *iface);
EXTERN int wp_tdmv_remora_init(wan_tdmv_iface_t *iface);

#ifdef CONFIG_PRODUCT_WANPIPE_TDM_VOICE_ECHOMASTER
EXTERN int wp_tdmv_echo_check(wan_tdmv_t *wan_tdmv, void *current_ztchan, int channo);
#endif


EXTERN int wanpipe_codec_convert_2s(u8 *data, int len, netskb_t *nskb, 
                                    u16 *power_ptr, int is_alaw);

EXTERN int wanpipe_codec_convert_s2ulaw(netskb_t *skb, netskb_t *nskb, int is_alaw);


#endif /* WAN_KERNEL */

#undef EXTERN

#endif	/* __SDLA_VOIP_H */
