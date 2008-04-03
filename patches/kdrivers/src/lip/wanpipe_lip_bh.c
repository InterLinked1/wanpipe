#if defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
# include <wanpipe_lip.h>
#else
#include <linux/wanpipe_lip.h>
#endif

#if defined(__LINUX__)
void wplip_link_bh(unsigned long data);
#else
void wplip_link_bh(void* data, int pending);
#endif

static int wplip_bh_receive(wplip_link_t *lip_link)
{
	netskb_t *skb;
	unsigned long timeout_cnt=SYSTEM_TICKS;
	int err;

	while((skb=wan_skb_dequeue(&lip_link->rx_queue)) != NULL){

		err=wplip_prot_rx(lip_link,skb);			
		if (err){
			wan_skb_free(skb);
		}
		
		if (SYSTEM_TICKS-timeout_cnt > 2){
			DEBUG_EVENT("%s: Link RxBH Time squeeze\n",lip_link->name);
			break;
		}
	}

	return 0;
}

static int wplip_bh_transmit(wplip_link_t *lip_link)
{
	netskb_t *skb;
	wplip_dev_t *lip_dev=NULL;
	int err=0;
	unsigned long timeout_cnt=SYSTEM_TICKS;
	int tx_pkt_cnt=0;

	if (wan_test_bit(WPLIP_BH_AWAITING_KICK,&lip_link->tq_working)){
		if (wan_test_bit(WPLIP_KICK,&lip_link->tq_working)){
			DEBUG_TEST("%s: KICK but await still set!\n",
					lip_link->name);
		}else{
			return 0;
		}
	}

	wan_clear_bit(WPLIP_KICK,&lip_link->tq_working);
	wan_clear_bit(WPLIP_BH_AWAITING_KICK,&lip_link->tq_working);
	
	wan_clear_bit(WPLIP_MORE_LINK_TX,&lip_link->tq_working);
	
	while((skb=wan_skb_dequeue(&lip_link->tx_queue))){
		err=wplip_data_tx_down(lip_link,skb);
		if (err != 0){
			wan_skb_queue_head(&lip_link->tx_queue,skb);
			wan_set_bit(WPLIP_BH_AWAITING_KICK,&lip_link->tq_working);
			goto wplip_bh_link_transmit_exit;
		}

		if (SYSTEM_TICKS-timeout_cnt > 1){
			DEBUG_EVENT("%s: Link TxBH Time squeeze\n",lip_link->name);
			goto wplip_bh_link_transmit_exit;
		}
	}

	if ((lip_dev=lip_link->cur_tx) != NULL){

		if (lip_dev->magic != WPLIP_MAGIC_DEV){
			DEBUG_EVENT("%s: Error1:  Invalid Dev Magic dev=%p Magic=0x%lX\n",
					lip_link->name,
					lip_dev,
					lip_dev->magic);
			goto wplip_bh_transmit_exit;
		}

	}else{
		lip_dev=WAN_LIST_FIRST(&lip_link->list_head_ifdev);
		if (!lip_dev){
			goto wplip_bh_transmit_exit;
		}

		if (lip_dev->magic != WPLIP_MAGIC_DEV){
			DEBUG_EVENT("%s: Error2:  Invalid Dev Magic dev=%p Magic=0x%lX\n",
					lip_link->name,
					lip_dev,
					lip_dev->magic);
			goto wplip_bh_transmit_exit;
		}

		lip_link->cur_tx=lip_dev;
	}
	
	for (;;){
		
		if (SYSTEM_TICKS-timeout_cnt > 3){
			if (WAN_NET_RATELIMIT()) {
			DEBUG_EVENT("%s: LipDev TxBH Time squeeze --- Sanity\n",lip_link->name);
			}
			goto wplip_bh_transmit_exit;
		}

		if (wan_test_bit(WPLIP_DEV_UNREGISTER,&lip_dev->critical)){
			goto wplip_bh_transmit_skip;
		}
	
		skb=wan_skb_dequeue(&lip_dev->tx_queue);
		if (skb){
			int len=wan_skb_len(skb);	
			
			err=wplip_data_tx_down(lip_link,skb);
			if (err != 0){
				wan_skb_queue_head(&lip_dev->tx_queue,skb);
				wan_set_bit(WPLIP_BH_AWAITING_KICK,&lip_link->tq_working);
				goto wplip_bh_transmit_exit;
			}

			lip_dev->ifstats.tx_packets++;
			lip_dev->ifstats.tx_bytes += len;

		}

		if (!wan_test_bit(0,&lip_dev->if_down) && 
		    WAN_NETIF_QUEUE_STOPPED(lip_dev->common.dev)){
			if (lip_dev->common.usedby == API){
				WAN_NETIF_START_QUEUE(lip_dev->common.dev);
#if defined(__LINUX__)
				wan_wakeup_api(lip_dev);
#endif
			}else if (lip_dev->common.lip){ /*STACK*/
				WAN_NETIF_START_QUEUE(lip_dev->common.dev);
				wplip_kick(lip_dev->common.lip,0);
				
			}else{
				WAN_NETIF_WAKE_QUEUE (lip_dev->common.dev);
			}
		}

		if (wan_skb_queue_len(&lip_dev->tx_queue)){
			wan_set_bit(WPLIP_MORE_LINK_TX,&lip_link->tq_working);
		}

		wplip_prot_kick(lip_link,lip_dev);

wplip_bh_transmit_skip:

		lip_dev=WAN_LIST_NEXT(lip_dev,list_entry);
		if (lip_dev == NULL){
			lip_dev=WAN_LIST_FIRST(&lip_link->list_head_ifdev);
		}

		if (lip_dev == lip_link->cur_tx){
#if 0
/* This logic can be used to speed up tx
   however it uses MUCH more CPU  */
			++tx_pkt_cnt;
			if (tx_pkt_cnt > 2) {
				break;
			}

			if (SYSTEM_TICKS-timeout_cnt > 2){
				break;
			}

			if (!wan_test_bit(WPLIP_MORE_LINK_TX,&lip_link->tq_working)) {
				break;
			}
#else
			/* We went through the whole list */
			break;
#endif
		}

	}

wplip_bh_transmit_exit:

	lip_link->cur_tx=lip_dev;

wplip_bh_link_transmit_exit:

	if (wan_skb_queue_len(&lip_link->tx_queue)){
		wan_set_bit(WPLIP_MORE_LINK_TX,&lip_link->tq_working);
	}

	if (wan_test_bit(WPLIP_MORE_LINK_TX,&lip_link->tq_working)){
		return 1;
	}
	
	return 0;

}


static int wplip_retrigger_bh(wplip_link_t *lip_link)
{
	if (wan_test_bit(WPLIP_MORE_LINK_TX, &lip_link->tq_working) ||
	    wan_skb_queue_len(&lip_link->rx_queue)){
		return wplip_trigger_bh(lip_link);
	}
#if 0
	if (gdbg_flag){
		DEBUG_EVENT("%s: Not triggered\n", __FUNCTION__);
	}
#endif
	return -EINVAL;	
}


#if defined(__LINUX__)
void wplip_link_bh(unsigned long data)
#else
void wplip_link_bh(void* data, int pending)
#endif
{
	wplip_link_t *lip_link = (wplip_link_t *)data;
#if defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
	wan_smp_flag_t	s;
#endif
	
	DEBUG_TEST("%s: Link BH\n",__FUNCTION__);

	if (wan_test_bit(WPLIP_LINK_DOWN,&lip_link->tq_working)){
		DEBUG_EVENT("%s: Link down in BH\n",__FUNCTION__);
		return;
	}

#if defined(__LINUX__)
	wan_spin_lock(&lip_link->bh_lock);
#else
	wan_spin_lock_irq(NULL, &s);
#endif
	wan_set_bit(WPLIP_BH_RUNNING,&lip_link->tq_working);
	
	wplip_bh_receive(lip_link);

	wplip_bh_transmit(lip_link);

	wan_clear_bit(WPLIP_BH_RUNNING,&lip_link->tq_working);

	WAN_TASKLET_END((&lip_link->task));

#if defined(__LINUX__)
	wan_spin_unlock(&lip_link->bh_lock);
#else
	wan_spin_unlock_irq(NULL, &s);
#endif
	
	wplip_retrigger_bh(lip_link);

}

