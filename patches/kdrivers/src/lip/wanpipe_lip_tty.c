/*====================================================================
	lip_link->tty_port.count++;
 *
 *
 *
 */

#include <linux/wanpipe_lip.h>

#define NR_PORTS 32
#define WAN_TTY_MAJOR 0
#define WAN_TTY_MINOR 0

#define WPLIP_MINOR_DEV(_port) (tty_lip_link_map[_port])
#define MIN_PORT 0
#define MAX_PORT NR_PORTS-1 

#define CRC_LENGTH 2

#define MAX_TTY_RX_BUF 30
#define TTY_MAX_MTU    4096

#ifdef __LINUX__

#ifndef SSTATE_MAGIC
  #define SSTATE_MAGIC 0x5301
/*
 * This is our internal structure for each serial port's state.
 */
struct serial_state {
    int magic;
    int baud_base;
    unsigned long   port;
    int irq;
    int flags;
    int hub6;
    int type;
    int line;
    int revision;   /* Chip revision (950) */
    int xmit_fifo_size;
    int custom_divisor;
    int count;
    u8  *iomem_base;
    u16 iomem_reg_shift;
    unsigned short  close_delay;
    unsigned short  closing_wait; /* time to wait before closing */
    struct async_icount icount;
    int io_type;
    struct async_struct *info;
    struct pci_dev  *dev;
};

#endif



static struct tty_driver 	*serial_driver;
static int 			tty_init_cnt=0;
static struct serial_state 	rs_table[NR_PORTS];

#ifndef LINUX_2_6
static int 			serial_refcount=1;
static struct tty_struct 	*serial_table[NR_PORTS];
static struct tty_driver 	callout_driver;
static struct termios 		*serial_termios[NR_PORTS];
static struct termios 		*serial_termios_locked[NR_PORTS];
#endif


static void* tty_lip_link_map[NR_PORTS] = {NULL,NULL,NULL,NULL,
					   NULL,NULL,NULL,NULL,
					   NULL,NULL,NULL,NULL,
					   NULL,NULL,NULL,NULL,
					   NULL,NULL,NULL,NULL,
					   NULL,NULL,NULL,NULL,
					   NULL,NULL,NULL,NULL,
					   NULL,NULL,NULL,NULL};


int wanpipe_tty_trigger_poll(wplip_link_t *lip_link)
{
	WAN_TASKQ_SCHEDULE((&lip_link->tty_task_queue));
	return 0;
}


# if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20)) 	
static void tty_poll_task (void *arg)
# else
static void tty_poll_task (struct work_struct *work)
# endif
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,20))   
	wplip_link_t	*lip_link=(wplip_link_t *)container_of(work, wplip_link_t, tty_task_queue);
#else
	wplip_link_t	*lip_link=(wplip_link_t *)arg;
#endif      
	struct tty_struct *tty;
	netskb_t *skb;
	int err=0;
	unsigned long smp_flags;

	DEBUG_TEST("%s: TQ=0x%lX\n",__FUNCTION__,
			lip_link->tq_working);
	
	if ((tty=lip_link->tty)==NULL)
		return;

	while ((skb=wan_skb_dequeue(&lip_link->tty_rx)) != NULL){

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,31))
		const struct tty_ldisc_ops *ops = tty->ldisc->ops;
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,27))
		const struct tty_ldisc_ops *ops = tty->ldisc.ops;
#else
		const struct tty_ldisc *ops = &tty->ldisc;
#endif

		if (ops->receive_buf)
			ops->receive_buf(tty,wan_skb_data(skb),NULL,wan_skb_len(skb));

		wan_skb_free(skb);
	}

	wplip_spin_lock_irq(&lip_link->bh_lock,&smp_flags);
	if (wan_test_and_clear_bit(WPLIP_TTY_BUSY,&lip_link->tq_working)){
		err=1;
	}
	wplip_spin_unlock_irq(&lip_link->bh_lock,&smp_flags);
	
	if (err){

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,31))
		const struct tty_ldisc_ops *ops = tty->ldisc->ops;
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,27))
		const struct tty_ldisc_ops *ops = tty->ldisc.ops;
#else
		const struct tty_ldisc *ops = &tty->ldisc;
#endif

		DEBUG_TEST("%s: Got TTY Wakeup!\n",lip_link->name);
	
		if ((tty->flags & (1 << TTY_DO_WRITE_WAKEUP)) &&
		    ops->write_wakeup)
			ops->write_wakeup(tty);

		wake_up_interruptible(&tty->write_wait);
#if defined(SERIAL_HAVE_POLL_WAIT) || \
   (defined LINUX_2_1 && LINUX_VERSION_CODE >= KERNEL_VERSION(2,2,15))
		wake_up_interruptible(&tty->poll_wait);
#endif	

	}
	return;
}

static void wanpipe_tty_close(struct tty_struct *tty, struct file * filp)
{
	wplip_link_t *lip_link;
	unsigned long smp_flags;
	
	if (!tty || !tty->driver_data){
		return;
	}
	
	lip_link = (wplip_link_t*)tty->driver_data;
	
	if (!lip_link)
		return;

	DEBUG_EVENT( "%s: Closing TTY Driver!\n",
			lip_link->name);

	/* Sanity Check */
	if (!lip_link->tty_open)
		return;
	
	if (--lip_link->tty_open == 0){

		netskb_t *skb;
		
		wplip_link_prot_change_state(lip_link,WAN_DISCONNECTED,NULL,0);

		wplip_spin_lock_irq(&lip_link->bh_lock,&smp_flags);	
		lip_link->tty=NULL;

		while ((skb=wan_skb_dequeue(&lip_link->tty_rx)) != NULL){
			wan_skb_free(skb);
		}
		
		/* BUGFIX: Moved down here */
		wplip_spin_unlock_irq(&lip_link->bh_lock,&smp_flags);

	}
	tty_port_close(&lip_link->tty_port, tty, filp);
	return;
}
static int wanpipe_tty_open(struct tty_struct *tty, struct file * filp)
{
	unsigned long smp_flags;
	wplip_link_t *lip_link;
	
	if (!tty){
		return -ENODEV;
	}
	
	if (!tty->driver_data){
		int port;
		port = tty->index;
		if ((port < 0) || (port >= NR_PORTS)){
			DEBUG_ERROR("TTY Open index %d out of range\n",port);
			return -ENODEV;
		}
		
		DEBUG_EVENT("Opening TTY Driver on port %i!\n",port);

		tty->driver_data = WPLIP_MINOR_DEV(port);
		if (!tty->driver_data)
			return -ENODEV;
	}

	lip_link = (wplip_link_t*)tty->driver_data;

	if (!lip_link){
		DEBUG_ERROR("TTY Open failed to find lip port for index %d\n",tty->index);
		wplip_spin_lock_irq(&lip_link->bh_lock,&smp_flags);	
		lip_link->tty=NULL;
		wplip_spin_unlock_irq(&lip_link->bh_lock,&smp_flags);
		return -ENODEV;
	}

	DEBUG_EVENT( "%s: Opening TTY Driver!\n",
			lip_link->name);

	if (lip_link->tty_open == 0){
		
		wplip_spin_lock_irq(&lip_link->bh_lock,&smp_flags);	
		lip_link->tty=tty;
		wplip_spin_unlock_irq(&lip_link->bh_lock,&smp_flags);

		wan_skb_queue_init(&lip_link->tty_rx);

		wplip_link_prot_change_state(lip_link,WAN_CONNECTED,NULL,0);
	}

	++lip_link->tty_open;
	lip_link->tty_port.count++;
	tty_port_tty_set(&lip_link->tty_port, tty);

	return 0;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,10)
static int wanpipe_tty_write(struct tty_struct * tty,
		    const unsigned char *buf, int count)
#else
static int wanpipe_tty_write(struct tty_struct * tty, int from_user,
		    const unsigned char *buf, int count)
#endif
{
	wplip_link_t *lip_link=NULL;
	unsigned char *tmp_buf;
	netskb_t *skb;
	int err;
	unsigned long smp_flags;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,10)
	int from_user=0;
#endif
	
	if (!tty){
		DEBUG_EVENT("NO TTY in Write\n");
		return -ENODEV;
	}

	lip_link = (wplip_link_t *)tty->driver_data;
			
	if (!lip_link){
		DEBUG_EVENT("No lip_link in TTY Write\n");
		return -ENODEV;
	}	

	
	if (count > TTY_MAX_MTU){
		DEBUG_EVENT("%s: Frame too big in %s() %d Max: %d\n",
				lip_link->name,__FUNCTION__,count, TTY_MAX_MTU);
		return -EINVAL;
	}
	
	if (lip_link->state != WAN_CONNECTED){
		DEBUG_TEST("%s: Link not connected (0x%x) in TTY Write\n",
				lip_link->name,lip_link->state);
		return -EINVAL;
	}

	skb=wan_skb_alloc(count);	
	if (!skb){
		return -ENOMEM;
	}

	tmp_buf=wan_skb_put(skb,count);

	if (from_user) {
		if (WAN_COPY_FROM_USER(tmp_buf,buf,count)){
			DEBUG_TEST("%s: Failed to copy from user!\n",
					lip_link->name);
			return -EINVAL;
		}
	}else{
		memcpy(tmp_buf,buf,count);	
	}

	err=wplip_data_tx_down(lip_link,skb);
	if (err){
		wplip_spin_lock_irq(&lip_link->bh_lock,&smp_flags);
		wan_set_bit(WPLIP_TTY_BUSY,&lip_link->tq_working);
		wplip_spin_unlock_irq(&lip_link->bh_lock,&smp_flags);

		DEBUG_TEST("%s: Failed to tx down!\n",
				lip_link->name);
		wan_skb_free(skb);
		return 0;
	}

	
	DEBUG_TEST("%s: Packet sent OK: %d\n",lip_link->name,count);
	return count;
}

void wplip_tty_receive(wplip_link_t *lip_link, void *skb)
{
	struct tty_struct *tty;
	
	if (!lip_link->tty_open){
		DEBUG_TEST("%s: TTY not open during receive\n",
				lip_link->name);
		wan_skb_free(skb);
		return;
	}
	
	if ((tty=lip_link->tty) == NULL){
		DEBUG_TEST("%s: No TTY on receive\n",
				lip_link->name);
		wan_skb_free(skb);
		return;
	}
	
	if (!tty->driver_data){
		DEBUG_TEST("%s: No Driver Data, or Flip on receive\n",
				lip_link->name);
		wan_skb_free(skb);
		return;
	}
	
	if (wan_skb_len(skb) > TTY_MAX_MTU){
		if (WAN_NET_RATELIMIT()){
			DEBUG_EVENT( 
			"%s: Received packet size too big: %d bytes, Max: %d!\n",
				lip_link->name,wan_skb_len(skb),TTY_MAX_MTU);
		}
		wan_skb_free(skb);
		return;
	}

	if (wan_skb_queue_len(&lip_link->tty_rx) > MAX_TTY_RX_BUF){
		wanpipe_tty_trigger_poll(lip_link);	
		wan_skb_free(skb);
		return;
	}
	
	wan_skb_queue_tail(&lip_link->tty_rx,skb);
	wanpipe_tty_trigger_poll(lip_link);	

	return;
}

#if 0
static int wanpipe_tty_ioctl(struct tty_struct *tty, struct file * file,
		    unsigned int cmd, unsigned long arg)
{
	return -ENOIOCTLCMD;
}
#endif

static void wanpipe_tty_stop(struct tty_struct *tty)
{
	return;
}

static void wanpipe_tty_start(struct tty_struct *tty)
{
	return;
}


#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,20))   
static void wanpipe_tty_set_termios(struct tty_struct *tty, const struct ktermios *old_termios)
#else
static void wanpipe_tty_set_termios(struct tty_struct *tty, struct termios *old_termios)
#endif
{
	wplip_link_t *lip_link;

	if (!tty){
		return;
	}

	lip_link = (wplip_link_t *)tty->driver_data;
			
	if (!lip_link)
		return;

	return;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,26))   
static int wanpipe_tty_put_char(struct tty_struct *tty, unsigned char ch)
#else
static void wanpipe_tty_put_char(struct tty_struct *tty, unsigned char ch)
#endif
{
	wplip_link_t *lip_link;
	netskb_t *skb;
	unsigned char *buf;
	int err=-EINVAL;
	unsigned long smp_flags;
	
	if (!tty){
		goto wanpipe_tty_put_char_exit;
	}
	
	lip_link = (wplip_link_t *)tty->driver_data;
	if (!lip_link){
		goto wanpipe_tty_put_char_exit;
	}

	if (lip_link->state != WAN_CONNECTED){
		goto wanpipe_tty_put_char_exit;
	}

	skb=wan_skb_alloc(5);	
	if (!skb){
		goto wanpipe_tty_put_char_exit;
	}
	
	buf=wan_skb_put(skb,1);
	buf[0]=ch;

	err=wplip_data_tx_down(lip_link,skb);
	if (err){
		wplip_spin_lock_irq(&lip_link->bh_lock,&smp_flags);
		wan_set_bit(WPLIP_TTY_BUSY,&lip_link->tq_working);
		wplip_spin_unlock_irq(&lip_link->bh_lock,&smp_flags);
		wan_skb_free(skb);
	}

wanpipe_tty_put_char_exit:

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,26))   
	return err;
#else
	return;
#endif
}

static void wanpipe_tty_flush_chars(struct tty_struct *tty)
{
	return;
}

static void wanpipe_tty_flush_buffer(struct tty_struct *tty)
{
	if (!tty)
		return;
	
	wake_up_interruptible(&tty->write_wait);
#if defined(SERIAL_HAVE_POLL_WAIT) || \
         (defined LINUX_2_1 && LINUX_VERSION_CODE >= KERNEL_VERSION(2,2,15))
	wake_up_interruptible(&tty->poll_wait);
#endif
	if ((tty->flags & (1 << TTY_DO_WRITE_WAKEUP))) {

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,31))
		const struct tty_ldisc_ops *ops = tty->ldisc->ops;
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,27))
		const struct tty_ldisc_ops *ops = tty->ldisc.ops;
#else
		const struct tty_ldisc *ops = &tty->ldisc;
#endif

		if (ops->write_wakeup)
			ops->write_wakeup(tty);
	}

	return;
}

/*
 * This function is used to send a high-priority XON/XOFF character to
 * the device
 */
#ifndef LINUX_2_6
static void wanpipe_tty_send_xchar(struct tty_struct *tty, char ch)
{
	return;
}
#endif

static unsigned int wanpipe_tty_chars_in_buffer(struct tty_struct *tty)
{
	return 0;
}


static unsigned int wanpipe_tty_write_room(struct tty_struct *tty)
{
	wplip_link_t *lip_link;

	DEBUG_TEST("TTY Write Room\n");
	
	if (!tty){
		return 0;
	}

	lip_link = (wplip_link_t *)tty->driver_data;
	if (!lip_link)
		return 0;

	if (lip_link->state != WAN_CONNECTED)
		return 0;
	
	return TTY_MAX_MTU;
}


static int set_modem_status(wplip_link_t *lip_link, unsigned char data)
{
	return 0;
}

static void wanpipe_tty_hangup(struct tty_struct *tty)
{
	wplip_link_t *lip_link;
	unsigned long smp_flags;

	DEBUG_EVENT( "TTY Hangup!\n");
	
	if (!tty){
		return;
	}

	lip_link = (wplip_link_t *)tty->driver_data;
	if (!lip_link)
		return;

	wplip_spin_lock_irq(&lip_link->bh_lock,&smp_flags);
	set_modem_status(lip_link,0);
	wplip_spin_unlock_irq(&lip_link->bh_lock,&smp_flags);

	return;
}

#ifndef LINUX_2_6
static void wanpipe_tty_break(struct tty_struct *tty, int break_state)
{
	return;
}

static void wanpipe_tty_wait_until_sent(struct tty_struct *tty, int timeout)
{
	return;
}
#endif

static void wanpipe_tty_throttle(struct tty_struct * tty)
{
	return;
}

static void wanpipe_tty_unthrottle(struct tty_struct * tty)
{
	return;
}

int wanpipe_tty_read_proc(char *page, char **start, off_t off, int count,
		 int *eof, void *data)
{
	return 0;
}

static int wanpipe_tty_carrier_raised(struct tty_port *port)
{
	return 0;
}
static void  wanpipe_tty_dtr_rts(struct tty_port *port, int raise)
{
}
static void wanpipe_tty_do_close(struct tty_port *port)
{
}

static int wanpipe_tty_port_activate(struct tty_port *port, struct tty_struct *tty)
{
    return 0;
}


#ifdef LINUX_2_6
static struct tty_operations wanpipe_tty_ops = {
	.open	= wanpipe_tty_open,
	.close = wanpipe_tty_close,
	.write = wanpipe_tty_write,
	.put_char = wanpipe_tty_put_char, 
	.flush_chars = wanpipe_tty_flush_chars,
	.write_room = wanpipe_tty_write_room,
	.chars_in_buffer = wanpipe_tty_chars_in_buffer,
	.flush_buffer = wanpipe_tty_flush_buffer,
//	.ioctl = wanpipe_tty_ioctl,
	.throttle = wanpipe_tty_throttle,
	.unthrottle = wanpipe_tty_unthrottle,
	.set_termios = wanpipe_tty_set_termios,
	.stop = wanpipe_tty_stop,
	.start = wanpipe_tty_start,
	.hangup = wanpipe_tty_hangup,
	.break_ctl = NULL,
	.wait_until_sent = NULL,
	.tiocmget = NULL,
	.tiocmset = NULL,
# if (LINUX_VERSION_CODE < KERNEL_VERSION(4,18,0))
	.proc_fops = NULL,
#endif

};
#endif

static const struct tty_port_operations wp_port_ops = {
	.activate = wanpipe_tty_port_activate,
    .carrier_raised = wanpipe_tty_carrier_raised,
    .dtr_rts = wanpipe_tty_dtr_rts,
    .shutdown = wanpipe_tty_do_close,
};


/*
 * The serial driver boot-time initialization code!
 */
int wplip_reg_tty(wplip_link_t *lip_link, wanif_conf_t *cfg)
{
	struct serial_state * state;

	lip_link->tty_minor = cfg->port;
	
	/* Initialize the tty_driver structure */

	if (lip_link->tty_minor < 0 || lip_link->tty_minor > NR_PORTS){
		DEBUG_EVENT("%s: Illegal Minor TTY number (0-%d): %d\n",
				lip_link->name,NR_PORTS,lip_link->tty_minor);
		return -EINVAL;
	}

	if (WPLIP_MINOR_DEV(lip_link->tty_minor)){
		DEBUG_EVENT("%s: TTY Minor %d, already in use\n",
				lip_link->name,lip_link->tty_minor);
		return -EBUSY;
	}


	if (tty_init_cnt==0){

		DEBUG_EVENT( "%s: TTY Driver Init: Major %d, Minor Range %d-%d\n",
				lip_link->name,
				WAN_TTY_MAJOR,MIN_PORT,MAX_PORT);
	
	
		serial_driver=tty_alloc_driver(NR_PORTS, 0);
		if (!serial_driver) {
			return -ENOMEM;
		}

# if (LINUX_VERSION_CODE < KERNEL_VERSION(6,1,0))
		serial_driver->magic = TTY_DRIVER_MAGIC;
#endif

		serial_driver->owner = THIS_MODULE;
		serial_driver->driver_name = "wanpipe_tty"; 
#if (LINUX_VERSION_CODE > 0x2032D && defined(CONFIG_DEVFS_FS))
		serial_driver->name = "ttyWP/%d";
#else
		serial_driver->name = "ttyWP";
#endif
		serial_driver->major = WAN_TTY_MAJOR;
		serial_driver->minor_start = WAN_TTY_MINOR;
		serial_driver->type = TTY_DRIVER_TYPE_SERIAL;
		serial_driver->subtype = SERIAL_TYPE_NORMAL;
		
		serial_driver->init_termios = tty_std_termios;
		serial_driver->init_termios.c_cflag =
			B9600 | CS8 | CREAD | HUPCL | CLOCAL;
		serial_driver->flags = TTY_DRIVER_REAL_RAW | TTY_DRIVER_DYNAMIC_DEV;


		tty_set_operations(serial_driver, &wanpipe_tty_ops);
		
		/*
		 * The callout device is just like normal device except for
		 * major number and the subtype code.
		 */

		if (tty_register_driver(serial_driver)){
			DEBUG_EVENT( "%s: Failed to register serial driver!\n",
					lip_link->name);
			tty_driver_kref_put(serial_driver);
			serial_driver=NULL;
			return -EINVAL;
		}

	}

	/* The subsequent ports must comply to the initial configuration */
	
	tty_init_cnt++;

#if (LINUX_VERSION_CODE > 0x2032D && defined(CONFIG_DEVFS_FS))	
	DEBUG_EVENT( "%s: TTY Sync Dev Minor %d : /dev/ttyWP/%d\n",
			lip_link->name,
			lip_link->tty_minor,lip_link->tty_minor);
#else
	DEBUG_EVENT( "%s: TTY Sync Dev Minor %d : /dev/ttyWP%d\n",
			lip_link->name,
			lip_link->tty_minor,lip_link->tty_minor);
#endif

	tty_lip_link_map[lip_link->tty_minor] = lip_link;

	state = &rs_table[lip_link->tty_minor];
	
	state->magic = SSTATE_MAGIC;
	state->line = 0;
	state->type = PORT_UNKNOWN;
	state->custom_divisor = 0;
	state->close_delay = 5*HZ/10;
	state->closing_wait = 30*HZ;

	state->icount.cts = state->icount.dsr = 
	state->icount.rng = state->icount.dcd = 0;
	state->icount.rx = state->icount.tx = 0;
	state->icount.frame = state->icount.parity = 0;
	state->icount.overrun = state->icount.brk = 0;
	state->irq = 0;

	DEBUG_EVENT("TTY Port Init\n");
	tty_port_init(&lip_link->tty_port);
	lip_link->tty_port.closing_wait = 30*HZ;;
	lip_link->tty_port.close_delay = HZ / 2;
	lip_link->tty_port.flags = 0;

	lip_link->tty_port.ops = &wp_port_ops;

	tty_port_register_device(&lip_link->tty_port, serial_driver, lip_link->tty_minor, NULL );

	WAN_TASKQ_INIT((&lip_link->tty_task_queue),0,tty_poll_task,lip_link);


	lip_link->tty_opt=1;
	return 0;
}


int wplip_unreg_tty(wplip_link_t *lip_link)
{
	struct serial_state * state;
	unsigned long smp_flags;

	DEBUG_TEST("%s: %s() cnt=%i\n",
			lip_link->name,__FUNCTION__,tty_init_cnt);
			

	if (lip_link->tty_open){
		return -EBUSY;
	}
	
	tty_unregister_device(serial_driver, lip_link->tty_minor);
	tty_port_destroy(&lip_link->tty_port);

	if ((--tty_init_cnt) == 0 && serial_driver){
# if (LINUX_VERSION_CODE < KERNEL_VERSION(5,13,0))
		int e1;
		if ((e1 = tty_unregister_driver(serial_driver))){
			DEBUG_EVENT( "SERIAL: failed to unregister serial driver (%d)\n",
			       e1);
		}
#else
		tty_unregister_driver(serial_driver);
#endif
		tty_driver_kref_put(serial_driver);
		serial_driver=NULL;
		DEBUG_EVENT( "%s: Unregistering TTY Driver, Major %i\n",
				lip_link->name,WAN_TTY_MAJOR);

	}

	wplip_spin_lock_irq(&lip_link->bh_lock,&smp_flags);	
	lip_link->tty=NULL;

	if (tty_lip_link_map[lip_link->tty_minor] == lip_link){
		tty_lip_link_map[lip_link->tty_minor]=NULL;
	}

	state = &rs_table[lip_link->tty_minor];
	memset(state,0,sizeof(*state));
	wplip_spin_unlock_irq(&lip_link->bh_lock,&smp_flags);	

	return 0;
}

#endif /*__LINUX__*/
