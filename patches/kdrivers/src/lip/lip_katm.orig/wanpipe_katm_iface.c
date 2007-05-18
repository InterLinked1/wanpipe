/*****************************************************************************
* wanpipe_katm_iface.c	
*
* 		WANPIPE Protocol Template Module
*
* Author:	Nenad Corbic
*
* Copyright:	(c)2007 Sangoma Technologies Inc.
*
*		This program is free software; you can redistribute it and/or
*		modify it under the terms of the GNU General Public License
*		as published by the Free Software Foundation; either version
*		2 of the License, or (at your option) any later version.
* ============================================================================
*	This module:
*		This module is free software; you can redistribute it and/or
*		modify it under the terms of the GNU General Public License
*		as published by the Free Software Foundation; either version
*		2 of the License, or (at your option) any later version.
*/

#include "wanpipe_katm.h"
static int wp_katm_data_indication(wp_katm_t *prot, void *skb);
 
static const struct atmdev_ops ops = {
	.open		= wan_lip_katm_open,
	.close		= wan_lip_katm_close,
	.ioctl		= NULL,
	.getsockopt	= wan_lip_katm_getsockopt,
	.setsockopt	= wan_lip_katm_setsockopt,
	.send		= wan_lip_katm_send,
	.phy_put		= NULL,
	.phy_get		= NULL,
	.change_qos	= NULL,
	.proc_read	= NULL,
	.owner		= THIS_MODULE,
};

/* Just here for easy reference until we get this working
const struct atmdev_ops mpc8260sar_ops = {
    open:mpc8260sar_open,
    close:mpc8260sar_close,
    ioctl:mpc8260sar_ioctl,
    getsockopt:mpc8260sar_getsockopt,
    setsockopt:mpc8260sar_setsockopt,
    send:mpc8260sar_send,
    sg_send:mpc8260sar_sg_send,
    change_qos:mpc8260sar_change_qos,
    proc_read:mpc8260sar_proc_read,
    owner:THIS_MODULE,
}; */

int wp_katm_tx(void *prot_ptr, void *skb, int type)
/********************************************************************************************/
{
	wp_katm_t *prot = (wp_katm_t *)prot_ptr;
	int err;

	/* This function is not used */
	
	wpabs_debug_event("%s %d\n",
			__FUNCTION__,__LINE__);

	/* The higher layer has given us a frame.
	 *
	 * Put a header on the frame and send it out.
	 * Or queue it and take care of it in custom
	 * state machine */
	
	return -1;
}

/*======================================================
 * wp_katm_data_transmit
 *
 * Transmitt the ppp packet to the lower layer,
 * vi its master (tx_dev) device.
 * 
 */
int wp_katm_data_transmit(wp_katm_t *prot, void *skb)
/********************************************************************************************/
{

	if (prot->link_dev && prot->reg.tx_link_down){
		return prot->reg.tx_link_down(prot->link_dev,skb);
	}else{
		wpabs_debug_event("%s: Critical error link dev=%p tx_link_down=%p\n",
				__FUNCTION__,prot->link_dev,prot->reg.tx_link_down);
				
	}
	
	return 1;
}

void *wp_register_katm_prot(void *link_ptr, 
		 	        char *devname, 
		                void *cfg_ptr, 
			        wplip_prot_reg_t *prot_reg)
/********************************************************************************************/
{
	wp_katm_t *atm_link;
	struct atm_dev *atmdev;
	wplip_prot_reg_t katm_callback_ops;

	int x=0;

	FUNC_BEGIN();

	wpabs_debug_event("%s: %s %d\n",
			devname,__FUNCTION__,__LINE__);
			
	if (!devname || !cfg_ptr || !prot_reg){
		wpabs_debug_event("%s:%d: Assert Error invalid args\n",
				__FUNCTION__,__LINE__);
		return NULL;
	}

	atm_link=wpabs_malloc(sizeof(wp_katm_t));
	if (!atm_link){
		wpabs_debug_event( "katm: Failed to create protocol %s\n",
				devname);
		return NULL;
	}

	/* Copy the LIP call back functions used to call
	 * the LIP layer */
	wpabs_memcpy(&atm_link->reg,prot_reg,sizeof(wplip_prot_reg_t));
	wpabs_strncpy(atm_link->name,devname,sizeof(atm_link->name)-1);
	
	/* INITIALIZE THE PROTOCOL HERE */
	atmdev = atm_dev_register(devname, &ops, -1, NULL);
	if (atmdev == NULL) {
		wpabs_debug_event("%s: couldn't register atm device!\n", devname);
		wpabs_free(atm_link);
		return NULL;
	}

	katm_callback_ops.prot_set_state	= wplip_katm_link_prot_change_state;
	katm_callback_ops.chan_set_state	= wplip_katm_lipdev_prot_change_state;
	katm_callback_ops.mtu			= 1500;
	katm_callback_ops.tx_link_down	= wplip_katm_link_callback_tx_down;
	katm_callback_ops.tx_chan_down	= wplip_katm_callback_tx_down;
	katm_callback_ops.rx_up			= wplip_katm_prot_rx_up;
	katm_callback_ops.get_ipv4_addr	= NULL;
	katm_callback_ops.set_ipv4_addr	= NULL;
	katm_callback_ops.kick_task	     = NULL;
	
	
	atm_link->sar_dev = wp_register_atm_prot(atm_link,"wanpipe_atm_link",&cfg_ptr,&katm_callback_ops);
	if (!atm_link->sar_dev) {
		wpabs_debug_event("%s: couldn't register sar device!\n", devname);
		atm_dev_deregister(atm_link->atmdev);
		wpabs_free(atm_link);
		return NULL;
	}
	
	/* This saves the previously allocated structure, we probably
	should consider making this static, so that it is not allocated 
	in a seperate memory region for speed purposes, less paging? */
	atmdev->dev_data = atm_link;

	/* RWM Added 2/21/2007 Let the ATM stack know our limits */
	atmdev->ci_range.vpi_bits = LOG2_NUM_VPIS;
	atmdev->ci_range.vci_bits = LOG2_NUM_VCIS_PER_VPI;
	atmdev->link_rate = AFT101_CELL_RATE;

	atm_link->atmdev = atmdev;
	
	/* Initialize the LINK pointer used to call
	 * the LIP layer */
	atm_link->link_dev = link_ptr;
	
	FUNC_END();
		
	/* Pass the atm_link pointer to the LIP Layer, so
	 * that LIP can call us with it */
	
	return atm_link;
}

int wp_unregister_katm_prot(void *prot_ptr)
/********************************************************************************************/
{	
	wp_katm_t *atm_link =(wp_katm_t*)prot_ptr;

	wpabs_debug_test("%s: %s %d\n",
			atm_link->name,__FUNCTION__,__LINE__);
	
	FUNC_BEGIN();
	
	wp_unregister_atm_prot(atm_link->sar_dev);
	
	atm_link->sar_dev=NULL;
	
	/* UNREGISTER LINK HERE */
	if (atm_link->atmdev) {
		atm_dev_deregister(atm_link->atmdev);
		atm_link->atmdev=NULL;
	}
	
	/* Free the allocated Protocol structure */
	wp_dev_put(atm_link);

	FUNC_END();
	return 0;
}


void *wp_register_katm_chan(void *if_ptr, 
			        void *prot_ptr, 
		                char *devname, 
			        void *cfg_ptr,
			        unsigned char type)
/********************************************************************************************/
{
	wp_katm_t *atm_link =(wp_katm_t*)prot_ptr;
	wan_atm_conf_if_t *cfg = (wan_atm_conf_if_t *)cfg_ptr;

	
	wpabs_strncpy(atm_link->name,devname,sizeof(atm_link->name)-1);
	wpabs_memcpy(&atm_link->cfg,cfg,sizeof(atm_link->cfg));

	wpabs_debug_test("%s: %s %d\n",
			atm_link->name,__FUNCTION__,__LINE__);

	/* Note we are not allocating a channel device
	 * because this example is for non multiplexed 
	 * protocols. Thus keep using the prot device
	 * that is already registered */
	atm_link->dev = if_ptr;
	atm_link->type= type;

	/* CONFIGURE/INITIALIZE THE PROTOCOL STRUCTURE */
	
	return atm_link;
}


int wp_unregister_katm_chan(void *chan_ptr)
/********************************************************************************************/
{
	wp_katm_t *atm_link =(wp_katm_t*)chan_ptr;

	wpabs_debug_test("%s: %s %d\n",
			atm_link->name,__FUNCTION__,__LINE__);
	/* UNREGISTER CHAN FROM PROTOCOL */
	
	atm_link->dev=NULL;

	return 0;
}


int wp_katm_open(void *prot_ptr)
/********************************************************************************************/
{
	wp_katm_t *prot =(wp_katm_t*)prot_ptr;

	wpabs_debug_test("%s: %s %d\n",
			prot->name,__FUNCTION__,__LINE__);

	/* OPEN CONFIGURATION 
	 * (ifconfig up) 
	 * 
	 */
	
	return 0;
}

int wp_katm_close(void *prot_ptr)
/********************************************************************************************/
{
	wp_katm_t *prot =(wp_katm_t*)prot_ptr;

	wpabs_debug_test("%s: %s %d\n",
			prot->name,__FUNCTION__,__LINE__);
	/* CLOSE CONFIGURATION 
	 * (ifconfig down) 
	 * 
	 */

	return 0;
}


int wp_katm_rx (void *prot_ptr, void * skb)
/********************************************************************************************/
{
	wp_katm_t *prot =(wp_katm_t*)prot_ptr;
	if (!prot->sar_dev) {
		wpabs_skb_free(skb);
		return 0;
	}
	
	wpabs_debug_test("%s: %s %d %d\n",
			prot->name,__FUNCTION__,__LINE__,wpabs_skb_len(skb));
	
	return wp_atm_rx(prot->sar_dev,skb);

}



int wp_katm_timer(void *prot_ptr, unsigned int *timeout, unsigned int carrier_reliable)
/********************************************************************************************/
{
	wp_katm_t *prot = (wp_katm_t *)prot_ptr;

	if (!prot->sar_dev) {
		*timeout=wpabs_get_hz();
		return 0;
	}

	if (carrier_reliable) {
		prot->state=WAN_CONNECTED;
	} else {
		prot->state=WAN_DISCONNECTED;
	}
	
	wpabs_debug_test("%s: %s %d carrier=%i \n",
			prot->name,__FUNCTION__,__LINE__,carrier_reliable);

	wp_atm_timer(prot->sar_dev,timeout,carrier_reliable);

	return 0;
}


int wp_katm_pipemon(void *prot_ptr, int cmd, int addr, 
		    unsigned char* data, unsigned int *len)
/********************************************************************************************/
{	
	wp_katm_t *prot = (wp_katm_t *)prot_ptr;

	prot = prot;

	*len=1;

	/* IMPLEMENT PIPEMON COMMANDS */

	return 0;
}

/*======================================================
 * wp_katm_data_indication
 *
 * Transmitt the lapb packet to the lower layer,
 * vi its master (tx_dev) device.
 * 
 */
static int wp_katm_data_indication(wp_katm_t *prot, void *skb)
/********************************************************************************************/
{
	wpabs_debug_event("%s: %s() Len=%i\n",
			prot->name,__FUNCTION__,wpabs_skb_len(skb));

	if (prot->dev){
		return prot->reg.rx_up(prot->dev,skb,prot->type);
	}

	return 1;
}



/**********************************************************
 * Private Functions
 **********************************************************/


/*======================================================
 * prot_connect_indication
 *
 * Indicate to the upper layer that the
 * lower layer is connected and that it should
 * start its connection process.
 */
void wp_connect_indication(wp_katm_t *prot, int reason)
/********************************************************************************************/
{
	wpabs_debug_event("%s: %s() Reason=%i\n",
			prot->name,__FUNCTION__,reason);

	if (prot->dev && prot->reg.chan_set_state){
		prot->reg.chan_set_state(prot->dev,WAN_CONNECTED,NULL, 0);
	}
}


/*======================================================
 * prot_disconnect_indication
 *
 * Indicate to the upper layer (x25) that the
 * lapb is layer is disconnected and that it should
 * drop the connection.
 */
void wp_disconnect_indication(wp_katm_t *prot, int reason)
/********************************************************************************************/
{
	wpabs_debug_event("%s: %s() Reason=%i\n",
			prot->name,__FUNCTION__,reason);

	if (prot->dev && prot->reg.chan_set_state){
		prot->reg.chan_set_state(prot->dev,WAN_DISCONNECTED,NULL,0);
	}
}
