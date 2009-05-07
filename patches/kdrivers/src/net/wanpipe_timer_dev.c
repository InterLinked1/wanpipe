/*****************************************************************************
* wanpipe_timer_dev.c	Wanpipe Timer Device
*
* Author:	Nenad Corbic
*
* Copyright:	(c) 2008 Sangoma Technologies Inc.
*
*		This program is free software; you can redistribute it and/or
*		modify it under the terms of the GNU General Public License
*		as published by the Free Software Foundation; either version
*		2 of the License, or (at your option) any later version.
* ============================================================================
*/


#include "wanpipe_includes.h"
#include "wanpipe_defines.h"
#include "wanpipe_debug.h"
#include "wanpipe_common.h"
#include "wanpipe_iface.h"
#include "wanrouter.h"	
#include "wanpipe.h"
#include "wanpipe_api.h"
#include "wanpipe_cdev_iface.h"
#include "sdladrv.h"
#include "wanpipe_timer_iface.h"



/*=================================================
 * Type Defines
 *================================================*/

#define DEBUG_TDEV DEBUG_EVENT
#define MAX_WAN_TDEV_IDX_SZ 20

typedef struct wanpipe_tdev
{
	int init;
	int used;
	int idx;
	netskb_t *event;
	wan_spinlock_t lock;
	wan_skb_queue_t free_list;
	wanpipe_cdev_t *cdev;
	wanpipe_timer_api_t ucmd;
}wanpipe_tdev_t;


/*=================================================
 * Prototypes 
 *================================================*/

static int wp_tdev_open(void *obj);
static int wp_tdev_close(void *obj);
static int wp_tdev_ioctl(void *obj, int cmd, void *data);
static unsigned int wp_tdev_poll(void *obj);
static int wan_alloc_skb_list(wanpipe_tdev_t *dev, int elements);


/*=================================================
 * Global Defines 
 *================================================*/

static sdla_t *wan_timer_card;
static u32 wan_timer_initialized;

static wanpipe_tdev_t *wan_tdev_idx[MAX_WAN_TDEV_IDX_SZ];
static int wan_tdev_cnt;
static u32 wan_event_seq_cnt;

#if defined(__WINDOWS__)
static wanpipe_cdev_ops_t wan_tdev_fops;
#else
static wanpipe_cdev_ops_t wan_tdev_fops = {
	open: wp_tdev_open,
	close: wp_tdev_close,
	ioctl: wp_tdev_ioctl,
	poll: wp_tdev_poll,
};
#endif

int wanpipe_wandev_timer_create(void)
{
	int err=-EINVAL;
	wanpipe_tdev_t *wan_tdev;

	wanpipe_cdev_t *cdev = wan_kmalloc(sizeof(wanpipe_cdev_t));
	if (!cdev) {
		return -ENOMEM;
	}

	wan_tdev=wan_kmalloc(sizeof(wanpipe_tdev_t));
	if (!wan_tdev) {
		wan_free(cdev);
		return -ENOMEM;
	}

	memset(cdev,0,sizeof(wanpipe_cdev_t));
	memset(wan_tdev,0,sizeof(wanpipe_tdev_t));

	cdev->dev_ptr=wan_tdev;
	wan_tdev->cdev=cdev;
	memcpy(&cdev->ops,&wan_tdev_fops,sizeof(wanpipe_cdev_ops_t));

	wan_skb_queue_init(&wan_tdev->free_list);
	wan_alloc_skb_list(wan_tdev,16);
	wan_spin_lock_init(&wan_tdev->lock,"wanpipe_timer_lock");
	
	err=wanpipe_cdev_timer_create(cdev);
	if (err) {
		wan_free(cdev);
		wan_free(wan_tdev);
	} else {
		wan_tdev_idx[wan_tdev_cnt] = wan_tdev;
		wan_tdev->idx=wan_tdev_cnt;
		wan_tdev_cnt++;
	}

	wan_timer_card=NULL;
	wan_set_bit(0,&wan_timer_initialized);
	DEBUG_TDEV("%s: WAN TDEV CREATE \n",__FUNCTION__);

	return err;

}


int wanpipe_wandev_timer_free(void)
{
	int i;
	wanpipe_tdev_t *wan_tdev;
	netskb_t *skb;

	wan_clear_bit(0,&wan_timer_initialized);

	for (i=0;i<MAX_WAN_TDEV_IDX_SZ;i++) {
		if ((wan_tdev=wan_tdev_idx[i]) != NULL) {

			wan_tdev_idx[i]=NULL;

			if (wan_tdev->cdev) {
				wanpipe_cdev_free(wan_tdev->cdev);
				wan_free(wan_tdev->cdev);
				wan_tdev->cdev=NULL;
			}

			wan_skb_queue_purge(&wan_tdev->free_list);
			if ((skb=wan_tdev->event)) {
				wan_tdev->event=NULL;
				wan_skb_free(skb);
			}

			wan_free(wan_tdev);
		}
	}

	DEBUG_TDEV("%s: WAN TDEV FREE \n",__FUNCTION__);

	return 0;
}

static int wp_tdev_open(void *obj)
{
	wanpipe_tdev_t *tdev=(wanpipe_tdev_t*)obj;

	if (wan_test_and_set_bit(0,&tdev->used)) {
		return -EBUSY;
	}

	DEBUG_TDEV("%s: WAN TDEV  OPEN \n",__FUNCTION__);

	return 0;
}


static int wp_tdev_close(void *obj)
{
	wanpipe_tdev_t *tdev=(wanpipe_tdev_t*)obj;

	wan_clear_bit(0,&tdev->used);

	DEBUG_TDEV("%s: WAN TDEV  CLOSE \n",__FUNCTION__);

	return 0;
}



static int wp_tdev_ioctl(void *obj, int cmd, void *udata)
{
	wanpipe_tdev_t *tdev=(wanpipe_tdev_t*)obj;
	int err=-EOPNOTSUPP;
	wanpipe_timer_api_t *utcmd=&tdev->ucmd;
	netskb_t *skb;
	wan_smp_flag_t flags;

#if defined(__WINDOWS__)
	/* udata is a pointer to wanpipe_tdm_api_cmd_t */
	memcpy(&utcmd, udata, sizeof(wanpipe_timer_api_t));
#else
	if (WAN_COPY_FROM_USER(&utcmd,
			       udata,
			       sizeof(wanpipe_timer_api_t))){
	       return -EFAULT;
	}
#endif


	switch (cmd) {

	case WANPIPE_IOCTL_TIMER_EVENT:

		wan_spin_lock_irq(&tdev->lock,&flags);
		skb=tdev->event;
		tdev->event=NULL;
		wan_spin_unlock_irq(&tdev->lock,&flags);

		if (skb) {
			u8 *buf=wan_skb_data(skb);
			memcpy(utcmd,buf,sizeof(wanpipe_timer_api_t));
			

			wan_spin_lock_irq(&tdev->lock,&flags);
			wan_skb_queue_tail(&tdev->free_list,skb);
			wan_spin_unlock_irq(&tdev->lock,&flags);

			skb=NULL;
			utcmd->operational_status=SANG_STATUS_SUCCESS;

			err=0;
		} else {
			utcmd->operational_status=SANG_STATUS_NO_DATA_AVAILABLE;
			err=-ENOBUFS;
		}
		break;

	default:
		utcmd->operational_status=SANG_STATUS_INVALID_PARAMETER;
		err=-EOPNOTSUPP;
		break;
	}


#if defined(__WINDOWS__)
	/* udata is a pointer to wanpipe_tdm_api_cmd_t */
	memcpy(udata, &utcmd, sizeof(wanpipe_timer_api_t));
#else
	if (WAN_COPY_FROM_USER(&udata,
			       &utcmd,
			       sizeof(wanpipe_timer_api_t))){
  	     return -EFAULT;
	}
#endif

	return err;
};

static unsigned int wp_tdev_poll(void *obj)
{
	wanpipe_tdev_t *tdev=(wanpipe_tdev_t*)obj;
	int err=0;
	
	if (tdev->event) {
		/* Indicate an exception */
		err |= POLLPRI;
	}


	return err;
}



int wp_timer_device_reg_tick(sdla_t *card)
{
	int i;
	wanpipe_tdev_t *wan_tdev;
	netskb_t *skb,*oskb;
	wanpipe_timer_api_t tevent;
	u8 *buf;
	wan_smp_flag_t flags;
	int init=0;

	if (!wan_test_bit(0,&wan_timer_initialized)) {
		return -1;
	}

	if (wan_timer_card == NULL) {
		/* This card just took control of this device */
		wan_timer_card = card;

	} else if (wan_timer_card != card) {
		/* This card does not own the device */
		return -1;
	}

	/* At this point the card that owns the device
       has inidcated that timer event has occoured */
	wan_event_seq_cnt++;

	for (i=0;i<MAX_WAN_TDEV_IDX_SZ;i++) {
		if ((wan_tdev=wan_tdev_idx[i]) != NULL) {

			if (wan_test_bit(0,&wan_tdev->used)) {
				continue;
			}

			if (init == 0) {
				init++;
				wan_getcurrenttime(&tevent.sec, &tevent.usec);
				tevent.sequence=wan_event_seq_cnt;		
			}

			wan_spin_lock(&wan_tdev->lock,&flags);
			skb = wan_skb_dequeue(&wan_tdev->free_list);
			wan_spin_unlock(&wan_tdev->lock,&flags);
			if (skb) {
				buf=wan_skb_data(skb);
				memcpy(buf,&tevent,sizeof(wanpipe_timer_api_t));
			}
			oskb=wan_tdev->event;
			wan_tdev->event=skb;
			if (oskb) {
				wan_spin_lock(&wan_tdev->lock,&flags);
				wan_skb_queue_tail(&wan_tdev->free_list,oskb);
				wan_spin_unlock(&wan_tdev->lock,&flags);
			}
			if (wan_tdev->cdev) {
				wanpipe_cdev_rx_wake(wan_tdev->cdev);
			}
		}
	}

	return 0;
}


int wp_timer_device_unreg(sdla_t *card)
{
	if (!wan_test_bit(0,&wan_timer_initialized)) {
		return -1;
	}

	if (wan_timer_card == card) {
		wan_timer_card = NULL;
		return 0;
	}
	return -1;
}


static int wan_alloc_skb_list(wanpipe_tdev_t *dev, int elements)
{
	int i;
	netskb_t *skb;
	for (i=0;i<elements;i++) {
		skb=wan_skb_alloc(sizeof(wanpipe_timer_api_t));
		if (skb) {
			wan_skb_queue_tail(&dev->free_list,skb);
		}
	}
	return 0;
}





