
#include "wanpipe_includes.h"
#include "wanpipe_defines.h"
#include "wanpipe_debug.h"
#include "wanpipe_common.h"
#include "wanpipe.h"

#include "sdlasfm.h"	/* SDLA firmware module definitions */
#include "sdlapci.h"	/* SDLA PCI hardware definitions */
#include "sdladrv.h"	/* API definitions */


/*****************************************************************************/
/* Memory Debug Code
*/

# if defined(WAN_DEBUG_MEM)

static int wan_debug_mem;

wan_spinlock_t wan_debug_mem_lock;
EXPORT_SYMBOL(wan_debug_mem_lock);

static WAN_LIST_HEAD(NAME_PLACEHOLDER_MEM, sdla_memdbg_el) sdla_memdbg_head = 
			WAN_LIST_HEAD_INITIALIZER(&sdla_memdbg_head);

typedef struct sdla_memdbg_el
{
	unsigned int len;
	unsigned int line;
	char cmd_func[128];
	void *mem;
	WAN_LIST_ENTRY(sdla_memdbg_el)	next;
}sdla_memdbg_el_t;

int sdla_memdbg_init(void);
int sdla_memdbg_free(void);

int sdla_memdbg_init(void)
{
	wan_spin_lock_init(&wan_debug_mem_lock,"wan_debug_mem_lock");
	WAN_LIST_INIT(&sdla_memdbg_head);
	return 0;
}


int sdla_memdbg_push(void *mem, const char *func_name, const int line, int len)
{
	sdla_memdbg_el_t *sdla_mem_el = NULL;
	wan_smp_flag_t flags;


#if defined(__LINUX__) || defined(__WINDOWS__)
	sdla_mem_el = kmalloc(sizeof(sdla_memdbg_el_t),GFP_ATOMIC);
#elif defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
	sdla_mem_el = malloc(sizeof(sdla_memdbg_el_t), M_DEVBUF, M_NOWAIT); 
#endif
	if (!sdla_mem_el) {
		DEBUG_EVENT("%s:%d Critical failed to allocate memory!\n",
			__FUNCTION__,__LINE__);
		return -ENOMEM;
	}

	memset(sdla_mem_el,0,sizeof(sdla_memdbg_el_t));
		
	sdla_mem_el->len=len;
	sdla_mem_el->line=line;
	sdla_mem_el->mem=mem;
	strncpy(sdla_mem_el->cmd_func,func_name,sizeof(sdla_mem_el->cmd_func)-1);
	
#if defined(__WINDOWS__)
	wan_spin_lock(&wan_debug_mem_lock,&flags);
#else
	wan_spin_lock_irq(&wan_debug_mem_lock,&flags);
#endif
	wan_debug_mem+=sdla_mem_el->len;
	WAN_LIST_INSERT_HEAD(&sdla_memdbg_head, sdla_mem_el, next);
	
#if defined(__WINDOWS__)
	wan_spin_unlock(&wan_debug_mem_lock,&flags);
#else
	wan_spin_unlock_irq(&wan_debug_mem_lock,&flags);
#endif

	DEBUG_TEST("%s:%d: Alloc %p Len=%i Total=%i\n",
			sdla_mem_el->cmd_func,sdla_mem_el->line,
			 sdla_mem_el->mem, sdla_mem_el->len,wan_debug_mem);
	return 0;

}
EXPORT_SYMBOL(sdla_memdbg_push);

int sdla_memdbg_pull(void *mem, const char *func_name, const int line)
{
	sdla_memdbg_el_t *sdla_mem_el;
	wan_smp_flag_t flags;
	int err=-1;

#if defined(__WINDOWS__)
	wan_spin_lock(&wan_debug_mem_lock,&flags);
#else
	wan_spin_lock_irq(&wan_debug_mem_lock,&flags);
#endif

	WAN_LIST_FOREACH(sdla_mem_el, &sdla_memdbg_head, next){
		if (sdla_mem_el->mem == mem) {
			break;
		}
	}

	if (sdla_mem_el) {
		
		WAN_LIST_REMOVE(sdla_mem_el, next);
		wan_debug_mem-=sdla_mem_el->len;

#if defined(__WINDOWS__)
		wan_spin_unlock(&wan_debug_mem_lock,&flags);
#else
		wan_spin_unlock_irq(&wan_debug_mem_lock,&flags);
#endif
		
		DEBUG_TEST("%s:%d: DeAlloc %p Len=%i Total=%i (From %s:%d)\n",
			func_name,line,
			sdla_mem_el->mem, sdla_mem_el->len, wan_debug_mem,
			sdla_mem_el->cmd_func,sdla_mem_el->line);
#if defined(__LINUX__) || defined(__WINDOWS__)
		kfree(sdla_mem_el);
#elif defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
		free(sdla_mem_el, M_DEVBUF); 
#endif
		err=0;
	} else {
#if defined(__WINDOWS__)
		wan_spin_unlock(&wan_debug_mem_lock,&flags);
#else
		wan_spin_unlock_irq(&wan_debug_mem_lock,&flags);
#endif
	}

	if (err) {
		DEBUG_EVENT("%s:%d: Critical Error: Unknows Memeory %p\n",
			__FUNCTION__,__LINE__,mem);
	}

	return err;
}
EXPORT_SYMBOL(sdla_memdbg_pull);

int sdla_memdbg_free(void)
{
	sdla_memdbg_el_t *sdla_mem_el;
	int total=0;

	DEBUG_EVENT("sdladrv: Memory Still Allocated=%i \n",
			 wan_debug_mem);

	DEBUG_EVENT("=====================BEGIN================================\n");

	sdla_mem_el = WAN_LIST_FIRST(&sdla_memdbg_head);
	while(sdla_mem_el){
		sdla_memdbg_el_t *tmp = sdla_mem_el;

		DEBUG_EVENT("%s:%d: Mem Leak %p Len=%i \n",
			sdla_mem_el->cmd_func,sdla_mem_el->line,
			sdla_mem_el->mem, sdla_mem_el->len);
		total+=sdla_mem_el->len;

		sdla_mem_el = WAN_LIST_NEXT(sdla_mem_el, next);
		WAN_LIST_REMOVE(tmp, next);
#if defined(__LINUX__) || defined(__WINDOWS__)
		kfree(tmp);
#elif defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
		free(tmp, M_DEVBUF); 
#endif
	}

	DEBUG_EVENT("=====================END==================================\n");
	DEBUG_EVENT("sdladrv: Memory Still Allocated=%i  Leaks Found=%i Missing=%i\n",
			 wan_debug_mem,total,wan_debug_mem-total);

	return 0;
}

# endif
