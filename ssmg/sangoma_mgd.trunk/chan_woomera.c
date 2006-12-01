/*
 * Asterisk -- A telephony toolkit for Linux.
 *
 * Woomera Channel Driver
 * 
 * Copyright (C) 05-06 Anthony Minessale II
 *                     Nenad Corbic 
 *
 * Anthony Minessale II <anthmct@yahoo.com>
 * Nenad Corbic <ncorbic@sangoma.com>
 *
 * This program is free software, distributed under the terms of
 * the GNU General Public License
 * =============================================
 * v1.6 Nenad Corbic <ncorbic@sangoma.com>
 *      Added incoming trunk group context 
 *      The trunk number will be added to the
 *      profile context name.
 *
 * v1.5	Nenad Corbic <ncorbic@sangoma.com>
 *      Use only ALAW and MLAW not SLIN.
 *      This reduces the load quite a bit.
 *      Autodetect Format from HELLO Message.
 *      RxTx Gain supported in woomera.conf as well
 *      from CLI.
 */

#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <math.h>
#include <netinet/tcp.h>
#include <asterisk/sched.h>
#include <asterisk/astobj.h>
#include <asterisk/lock.h>
#include <asterisk/manager.h>
#include <asterisk/channel.h>
#include <asterisk/pbx.h>
#include <asterisk/cli.h>
#include <asterisk/logger.h>
#include <asterisk/frame.h>
#include <asterisk/config.h>
#include <asterisk/module.h>
#include <asterisk/lock.h>
#include <asterisk/translate.h>
#include <asterisk/causes.h>
#include <asterisk/dsp.h>
#include <g711.h>
#include <errno.h>

#define MEDIA_ANSWER "MEDIA"
// THE ONE ABOVE OR THE 2 BELOW BUT NOT BOTH
//#define MEDIA_ANSWER "ANSWER"
#define USE_ANSWER 1

#include "asterisk.h"
extern int option_verbose;

#define WOOMERA_VERSION "v1.6"

ASTERISK_FILE_VERSION(__FILE__, "$Revision: 1.6 $")
static int tech_count = 0;

static const char desc[] = "Woomera Channel Driver";
static const char type[] = "WOOMERA";
static const char tdesc[] = "Woomera Channel Driver";
static char configfile[] = "woomera.conf";
static char smgversion_init=0;
static char smgversion[100] = "N/A";

#ifdef AST_JB
#include "asterisk/abstract_jb.h"
/* Global jitterbuffer configuration - by default, jb is disabled */
static struct ast_jb_conf default_jbconf =
{
        .flags = 0,
        .max_size = -1,
        .resync_threshold = -1,
        .impl = ""
};
static struct ast_jb_conf global_jbconf;
#endif /* AST_JB */

#define WOOMERA_SLINEAR 0
#define WOOMERA_ULAW	1
#define WOOMERA_ALAW	2

#define WOOMERA_STRLEN 256
#define WOOMERA_ARRAY_LEN 50
#define WOOMERA_MIN_PORT  9900
#define WOOMERA_MAX_PORT 10799
#define WOOMERA_BODYLEN 2048
#define WOOMERA_LINE_SEPARATOR "\r\n"
#define WOOMERA_RECORD_SEPARATOR "\r\n\r\n"
#define WOOMERA_DEBUG_PREFIX "**[WOOMERA]** "
#define WOOMERA_DEBUG_LINE "--------------------------------------------------------------------------------" 
#define WOOMERA_HARD_TIMEOUT -10000
#define WOOMERA_QLEN 10

/* this macro is not in all versions of asterisk */
#ifdef OLDERAST
#define ASTOBJ_CONTAINER_UNLINK(container,obj) \
        ({ \
                typeof((container)->head) found = NULL; \
                typeof((container)->head) prev = NULL; \
                ASTOBJ_CONTAINER_TRAVERSE(container, !found, do { \
                        if (iterator== obj) { \
                                found = iterator; \
                                found->next[0] = NULL; \
                                ASTOBJ_CONTAINER_WRLOCK(container); \
                                if (prev) \
                                        prev->next[0] = next; \
                                else \
                                        (container)->head = next; \
                                ASTOBJ_CONTAINER_UNLOCK(container); \
                        } \
                        prev = iterator; \
                } while (0)); \
                found; \
        }) 
#endif


#define FRAME_LEN 480

#if 0
static int WFORMAT = AST_FORMAT_ALAW; //AST_FORMAT_SLINEAR;
#else
static int WFORMAT = AST_FORMAT_SLINEAR;
#endif

typedef enum {
	WFLAG_EXISTS = (1 << 0),
	WFLAG_EVENT = (1 << 1),
	WFLAG_CONTENT = (1 << 2),
} WFLAGS;


typedef enum {
	WCFLAG_NOWAIT = (1 << 0)
} WCFLAGS;


typedef enum {
	PFLAG_INBOUND = (1 << 0),
	PFLAG_OUTBOUND = (1 << 1),
	PFLAG_DYNAMIC = (1 << 2),
	PFLAG_DISABLED = (1 << 3)
} PFLAGS;

typedef enum {
	TFLAG_MEDIA = (1 << 0),
	TFLAG_INBOUND = (1 << 1),
	TFLAG_OUTBOUND = (1 << 2),
	TFLAG_INCOMING = (1 << 3),
	TFLAG_PARSE_INCOMING = (1 << 4),
	TFLAG_ACTIVATE = (1 << 5),
	TFLAG_DTMF = (1 << 6),
	TFLAG_DESTROY = (1 << 7),
	TFLAG_ABORT = (1 << 8),
	TFLAG_PBX = (1 << 9),
	TFLAG_ANSWER = (1 << 10),
	TFLAG_INTHREAD = (1 << 11),
	TFLAG_TECHHANGUP = (1 << 12),
	TFLAG_DESTROYED = (1 << 13),
	TFLAG_UP = (1 << 14)
} TFLAGS;

static int usecnt = 0;

struct woomera_message {
	char callid[WOOMERA_STRLEN];
	int mval;
	char command[WOOMERA_STRLEN];
	char command_args[WOOMERA_STRLEN];
	char names[WOOMERA_STRLEN][WOOMERA_ARRAY_LEN];
	char values[WOOMERA_STRLEN][WOOMERA_ARRAY_LEN];
	char body[WOOMERA_BODYLEN];
	char cause[WOOMERA_STRLEN]; 
	unsigned int flags;
	int last;
	unsigned int queue_id;
	struct woomera_message *next;
};


static struct {
	int next_woomera_port;	
	int debug;
	int panic;
	int more_threads;
	ast_mutex_t woomera_port_lock;
} globals;

struct woomera_event_queue {
	struct woomera_message *head;
	ast_mutex_t lock;
};

struct woomera_profile {
	ASTOBJ_COMPONENTS(struct woomera_profile);
	ast_mutex_t iolock;
	ast_mutex_t call_count_lock;
	char woomera_host[WOOMERA_STRLEN];
	int max_calls;
	int call_count;
	int woomera_port;
	char audio_ip[WOOMERA_STRLEN];
	char context[WOOMERA_STRLEN];
	pthread_t thread;
	unsigned int flags;
	int thread_running;
	int dtmf_enable;
	int faxdetect;
	int woomera_socket;
	struct woomera_event_queue event_queue;
	int jb_enable;
	int progress_enable;
	int coding;
	float rxgain_val;
	float txgain_val;
	unsigned char rxgain[256];
	unsigned char txgain[256];
};


struct private_object {
	ASTOBJ_COMPONENTS(struct private_object);
	ast_mutex_t iolock;
	struct ast_channel *owner;
	struct sockaddr_in udpread;
	struct sockaddr_in udpwrite;
	int command_channel;
	int udp_socket;
	unsigned int flags;
	struct ast_frame frame;
	short fdata[FRAME_LEN + AST_FRIENDLY_OFFSET];
	struct woomera_message call_info;
	struct woomera_profile *profile;
	char dest[WOOMERA_STRLEN];
	int port;
	struct timeval started;
	int timeout;
	char dtmfbuf[WOOMERA_STRLEN];
	char cid_name[WOOMERA_STRLEN];
	char cid_num[WOOMERA_STRLEN];
	char ds[WOOMERA_STRLEN];
	struct ast_dsp *dsp;  
	int ast_dsp;
	int dsp_features;
	int faxdetect;
	int faxhandled;
	int call_count;
	char callid[WOOMERA_STRLEN];
	pthread_t thread;
	unsigned int callno;
	int refcnt;
	struct woomera_event_queue event_queue;
	unsigned int coding;
#ifdef AST_JB
        struct ast_jb_conf jbconf;
#endif /* AST_JB */

};

typedef struct private_object private_object;
typedef struct woomera_message woomera_message;
typedef struct woomera_profile woomera_profile;
typedef struct woomera_event_queue woomera_event_queue;


static void my_ast_softhangup(struct ast_channel *chan, private_object *tech_pvt, int cause)
{
#if 1
	ast_queue_hangup(chan);
	//ast_softhangup(chan,  AST_SOFTHANGUP_EXPLICIT);
#else
	struct ast_frame f = { AST_FRAME_NULL };

	if (chan) {
		chan->_softhangup |= cause;
		ast_queue_frame(chan, &f);
	}
	
	ast_set_flag(tech_pvt, TFLAG_ABORT);
#endif

#if 0
	if (tech_pvt->dsp) {
		tech_pvt->dsp_features &= ~DSP_FEATURE_DTMF_DETECT;
		ast_dsp_set_features(tech_pvt->dsp, tech_pvt->dsp_features); 
		tech_pvt->ast_dsp=0;
	}
#endif

}

//static struct sched_context *sched;

static struct private_object_container {
    ASTOBJ_CONTAINER_COMPONENTS(private_object);
} private_object_list;

static struct woomera_profile_container {
    ASTOBJ_CONTAINER_COMPONENTS(woomera_profile);
} woomera_profile_list;

static woomera_profile default_profile;

/* some locks you will use for use count and for exclusive access to the main linked-list of private objects */
AST_MUTEX_DEFINE_STATIC(usecnt_lock);
AST_MUTEX_DEFINE_STATIC(lock);

/* local prototypes */
static void woomera_close_socket(int *socket);
static void global_set_flag(int flags);
static void woomera_printf(woomera_profile *profile, int fd, char *fmt, ...);
static char *woomera_message_header(woomera_message *wmsg, char *key);
static int woomera_enqueue_event(woomera_event_queue *event_queue, woomera_message *wmsg);
static int woomera_dequeue_event(woomera_event_queue *event_queue, woomera_message *wmsg);
static int woomera_message_parse(int fd, woomera_message *wmsg, int timeout, woomera_profile *profile, woomera_event_queue *event_queue);
static int woomera_message_parse_wait(private_object *tech_pvt,woomera_message *wmsg);
static int waitfor_socket(int fd, int timeout);
static int woomera_profile_thread_running(woomera_profile *profile, int set, int new);
static int woomera_locate_socket(woomera_profile *profile, int *woomera_socket);
static void *woomera_thread_run(void *obj); 
static void launch_woomera_thread(woomera_profile *profile);
static void destroy_woomera_profile(woomera_profile *profile); 
static woomera_profile *clone_woomera_profile(woomera_profile *new_profile, woomera_profile *default_profile);
static woomera_profile *create_woomera_profile(woomera_profile *default_profile);
static int config_woomera(void);
static int create_udp_socket(char *ip, int port, struct sockaddr_in *sockaddr, int client);
static int connect_woomera(int *new_socket, woomera_profile *profile, int flags);
static int init_woomera(void);
static struct ast_channel *woomera_new(const char *type, int format, void *data, int *cause);
static int woomera_cli(int fd, int argc, char *argv[]);
static void tech_destroy(private_object *tech_pvt, struct ast_channel *owner);
static struct ast_channel *woomera_new(const char *type, int format, void *data, int *cause);
static int launch_tech_thread(private_object *tech_pvt);
static int tech_create_read_socket(private_object *tech_pvt);
static int tech_activate(private_object *tech_pvt);
static int tech_init(private_object *tech_pvt, woomera_profile *profile, int flags);
static void *tech_monitor_thread(void *obj);
static void tech_monitor_in_one_thread(void);
static struct ast_channel *  tech_get_owner( private_object *tech_pvt);




/********************CHANNEL METHOD PROTOTYPES*******************
 * You may or may not need all of these methods, remove any unnecessary functions/protos/mappings as needed.
 *
 */
static struct ast_channel *tech_requester(const char *type, int format, void *data, int *cause);
static int tech_send_digit(struct ast_channel *self, char digit);
static int tech_call(struct ast_channel *self, char *dest, int timeout);
static int tech_hangup(struct ast_channel *self);
static int tech_answer(struct ast_channel *self);
static struct ast_frame *tech_read(struct ast_channel *self);
static struct ast_frame *tech_exception(struct ast_channel *self);
static int tech_write(struct ast_channel *self, struct ast_frame *frame);
#if 0
static int tech_indicate(struct ast_channel *self, int condition);
#endif
static int tech_fixup(struct ast_channel *oldchan, struct ast_channel *newchan);
static int tech_send_html(struct ast_channel *self, int subclass, const char *data, int datalen);
static int tech_send_text(struct ast_channel *self, const char *text);
static int tech_send_image(struct ast_channel *self, struct ast_frame *frame);
static int tech_setoption(struct ast_channel *self, int option, void *data, int datalen);
static int tech_queryoption(struct ast_channel *self, int option, void *data, int *datalen);
//static enum ast_bridge_result tech_bridge(struct ast_channel *c0, struct ast_channel *c1, int flags, struct ast_frame **fo, struct ast_channel **rc, int timeoutms);
static int tech_transfer(struct ast_channel *self, const char *newdest);
static int tech_write_video(struct ast_channel *self, struct ast_frame *frame);
//static struct ast_channel *tech_bridged_channel(struct ast_channel *self, struct ast_channel *bridge);

/********************************************************************************
 * Constant structure for mapping local methods to the core interface.
 * This structure only needs to contain the methods the channel requires to operate
 * Not every channel needs all of them defined.
 */

static const struct ast_channel_tech technology = {
	.type = type,
	.description = tdesc,
	.capabilities = (AST_FORMAT_SLINEAR | AST_FORMAT_ULAW | AST_FORMAT_ALAW),
	.requester = tech_requester,
	.send_digit = tech_send_digit,
	.call = tech_call,
	//.bridge = tech_bridge,
	.hangup = tech_hangup,
	.answer = tech_answer,
	.transfer = tech_transfer,
	.write_video = tech_write_video,
	.read = tech_read,
	.write = tech_write,
	.exception = tech_exception,
	//.indicate = tech_indicate,
	.fixup = tech_fixup,
	.send_html = tech_send_html,
	.send_text = tech_send_text,
	.send_image = tech_send_image,
	.setoption = tech_setoption,
	.queryoption = tech_queryoption,
	//.bridged_channel = tech_bridged_channel,
	.transfer = tech_transfer,
};


static void woomera_close_socket(int *socket) 
{

	if (*socket > -1) {
		close(*socket);
	}
	*socket = -1;
}

static void global_set_flag(int flags)
{
	private_object *tech_pvt;

	ASTOBJ_CONTAINER_TRAVERSE(&private_object_list, 1, do {
		ASTOBJ_RDLOCK(iterator);
        tech_pvt = iterator;
		ast_set_flag(tech_pvt, flags);
		ASTOBJ_UNLOCK(iterator);
    } while(0));
} 

static void woomera_send_progress(private_object *tech_pvt)
{
	struct ast_channel *owner = tech_get_owner(tech_pvt);

 	if (tech_pvt->profile->progress_enable && owner){
        	if (globals.debug > 2) {
                	ast_log(LOG_NOTICE, "Sending Progress %s\n",tech_pvt->callid);
                }

              	ast_queue_control(owner,
                                AST_CONTROL_PROGRESS);
        }
}


static uint32_t string_to_release(char *code)
{
	if (code) {
		if (!strcasecmp(code, "CHANUNAVAIL")) {
			return AST_CAUSE_REQUESTED_CHAN_UNAVAIL;
		}

		if (!strcasecmp(code, "INVALID")) {
			return AST_CAUSE_NORMAL_CIRCUIT_CONGESTION;
		}

		if (!strcasecmp(code, "ERROR")) {
			return AST_CAUSE_NORMAL_CIRCUIT_CONGESTION;
		}

		if (!strcasecmp(code, "CONGESTION")) {
			return AST_CAUSE_NORMAL_CIRCUIT_CONGESTION;
		}

		if (!strcasecmp(code, "BUSY")) {
			return AST_CAUSE_USER_BUSY;
		}

		if (!strcasecmp(code, "NOANSWER")) {
			return AST_CAUSE_NO_ANSWER;
		}

		if (!strcasecmp(code, "ANSWER")) {
			return AST_CAUSE_NORMAL_CLEARING;
		}

		if (!strcasecmp(code, "CANCEL")) {
			return AST_CAUSE_NORMAL_CIRCUIT_CONGESTION;
		}

		if (!strcasecmp(code, "UNKNOWN")) {
			return AST_CAUSE_NORMAL_CIRCUIT_CONGESTION;
		} 
	}
	return AST_CAUSE_NORMAL_CLEARING;
}


static void woomera_printf(woomera_profile *profile, int fd, char *fmt, ...)
{
    char *stuff;
    int res = 0;

	if (fd < 0) {
		if (globals.debug > 4) {
			ast_log(LOG_ERROR, "Not gonna write to fd %d\n", fd);
		}
		return;
	}
	
    va_list ap;
    va_start(ap, fmt);
#ifdef SOLARIS
	stuff = (char *)malloc(10240);
	vsnprintf(stuff, 10240, fmt, ap);
#else
    res = vasprintf(&stuff, fmt, ap);
#endif
    va_end(ap);
    if (res == -1) {
        ast_log(LOG_ERROR, "Out of memory\n");
    } else {
		if (profile && globals.debug) {
			if (option_verbose > 2) {
			ast_verbose(WOOMERA_DEBUG_PREFIX "Send Message: {%s} [%s/%d]\n%s\n%s", profile->name, profile->woomera_host, profile->woomera_port, WOOMERA_DEBUG_LINE, stuff);
			}
		}
        ast_carefulwrite(fd, stuff, strlen(stuff), 100);
        free(stuff);
    }

}

static char *woomera_message_header(woomera_message *wmsg, char *key) 
{
	int x = 0;
	char *value = NULL;


#if 0
	if (!strcasecmp(wmsg->command,"HANGUP")) {
	ast_log(LOG_NOTICE, "Message Header for HANGUP\n");
	for (x = 0 ; x < wmsg->last ; x++) {
		ast_log(LOG_NOTICE, "Name=%s Value=%s\n",
				wmsg->names[x],wmsg->values[x]);
		if (!strcasecmp(wmsg->names[x], key)) {
			value = wmsg->values[x];
			break;
		}
	}
	}
#endif

	for (x = 0 ; x < wmsg->last ; x++) {
		if (!strcasecmp(wmsg->names[x], key)) {
			value = wmsg->values[x];
			break;
		}
	}

	return value;
}

static int woomera_enqueue_event(woomera_event_queue *event_queue, woomera_message *wmsg)
{
	woomera_message *new, *mptr;

	if ((new = malloc(sizeof(woomera_message)))) {
		ast_mutex_lock(&event_queue->lock);
		memcpy(new, wmsg, sizeof(woomera_message));
		new->next = NULL;
		if (!event_queue->head) {
			event_queue->head = new;
		} else {
			for (mptr = event_queue->head; mptr && mptr->next ; mptr = mptr->next);
			mptr->next = new;
		}
		ast_mutex_unlock(&event_queue->lock);
		return 1;
	} else {
		ast_log(LOG_ERROR, "Memory Allocation Error!\n");
	}

	return 0;
}

static int woomera_dequeue_event(woomera_event_queue *event_queue, woomera_message *wmsg)
{
	woomera_message *mptr = NULL;
	
	ast_mutex_lock(&event_queue->lock);
	if (event_queue->head) {
		mptr = event_queue->head;
		event_queue->head = mptr->next;
	}

	if (mptr) {
		memcpy(wmsg, mptr, sizeof(woomera_message));
	}
	ast_mutex_unlock(&event_queue->lock);

	if (mptr){
		free(mptr);
		return 1;
	} else {
		memset(wmsg, 0, sizeof(woomera_message));
	}
	
	return 0;
}




static int woomera_message_parse(int fd, woomera_message *wmsg, int timeout, 
                                 woomera_profile *profile, woomera_event_queue *event_queue) 
{
	char *cur, *cr, *next = NULL, *eor = NULL;
	char buf[2048];
	int res = 0, bytes = 0, sanity = 0;
	struct timeval started, ended;
	int elapsed, loops = 0;
	int failto = 0;

	memset(wmsg, 0, sizeof(woomera_message));

	if (fd < 0) {
		return -1;
	}

	gettimeofday(&started, NULL);
	memset(buf, 0, sizeof(buf));

	if (timeout < 0) {
		timeout = abs(timeout);
		failto = 1;
	} else if(timeout == 0) {
		timeout = -1;
	}

	while (!(eor = strstr(buf, WOOMERA_RECORD_SEPARATOR))) {

		if (!profile->thread_running) {
			return -1;
		}

		if (globals.panic > 2) {
			return -1;
		}
		/* Keep things moving.
		   Stupid Sockets -Homer Simpson */
		woomera_printf(NULL, fd, "%s", WOOMERA_RECORD_SEPARATOR);

		if((res = waitfor_socket(fd, (timeout > 0 ? timeout : 100)) > 0)) {
			res = recv(fd, buf, sizeof(buf), MSG_PEEK);
			if (res == 0) {
				sanity++;
			} else if (res < 0) {
				if (option_verbose > 2) {
				ast_verbose(WOOMERA_DEBUG_PREFIX "{%s} error during packet retry #%d\n", profile->name, loops);
				}

				return res;
			} else if (loops && globals.debug) {
				//ast_verbose(WOOMERA_DEBUG_PREFIX "{%s} Didnt get complete packet retry #%d\n", profile->name, loops);
				woomera_printf(NULL, fd, "%s", WOOMERA_RECORD_SEPARATOR);
				usleep(100);
			}

			if (res > 0) {
				sanity=0;
			}

		}

		gettimeofday(&ended, NULL);
		elapsed = (((ended.tv_sec * 1000) + ended.tv_usec / 1000) - ((started.tv_sec * 1000) + started.tv_usec / 1000));

		if (res < 0) {
			return res;
		}

		if (sanity > 1000) {
			if (globals.debug > 2) {
				ast_log(LOG_ERROR, "{%s} Failed Sanity Check! [errors] chfd=%d\n", 
					profile->name,fd);
			}
			return -100;
		}

		if (timeout > 0 && (elapsed > timeout)) {
			return failto ? -1 : 0;
		}
		
		loops++;
	}
	*eor = '\0';
	bytes = strlen(buf) + 4;
	
	memset(buf, 0, sizeof(buf));
	res = read(fd, buf, bytes);
	next = buf;

	if (globals.debug) {
		if (option_verbose > 2) {
		ast_verbose(WOOMERA_DEBUG_PREFIX "Receive Message: {%s} [%s/%d]\n%s\n%s", profile->name, profile->woomera_host, profile->woomera_port, WOOMERA_DEBUG_LINE, buf);
		}
	}

	while((cur = next)) {
		if ((cr = strstr(cur, WOOMERA_LINE_SEPARATOR))) {
			*cr = '\0';
			next = cr + (sizeof(WOOMERA_LINE_SEPARATOR) - 1);
			if (!strcmp(next, WOOMERA_RECORD_SEPARATOR)) {
				break;
			}
		} 

		if (ast_strlen_zero(cur)) {
			break;
		}

		if (!wmsg->last) {
			ast_set_flag(wmsg, WFLAG_EXISTS);
			if (!strncasecmp(cur, "EVENT", 5)) {
				cur += 6;
				ast_set_flag(wmsg, WFLAG_EVENT);

				if (cur && (cr = strchr(cur, ' '))) {
					char *id;

					*cr = '\0';
					cr++;
					id = cr;
					if (cr && (cr = strchr(cr, ' '))) {
						*cr = '\0';
						cr++;
						strncpy(wmsg->command_args, cr, WOOMERA_STRLEN);
					}
					if(id) {
						ast_copy_string(wmsg->callid, id, sizeof(wmsg->callid));
					}
				}
			} else {
				if (cur && (cur = strchr(cur, ' '))) {
					*cur = '\0';
					cur++;
					wmsg->mval = atoi(buf);
				} else {
					ast_log(LOG_WARNING, "Malformed Message!\n");
					break;
				}
			}
			if (cur) {
				strncpy(wmsg->command, cur, WOOMERA_STRLEN);
			} else {
				ast_log(LOG_WARNING, "Malformed Message!\n");
				break;
			}
		} else {
			char *name, *val;
			name = cur;
			if ((val = strchr(name, ':'))) {
				*val = '\0';
				val++;
				while (*val == ' ') {
					*val = '\0';
					val++;
				}
				strncpy(wmsg->values[wmsg->last-1], val, WOOMERA_STRLEN);
			}
			strncpy(wmsg->names[wmsg->last-1], name, WOOMERA_STRLEN);
			if (name && val && !strcasecmp(name, "content-type")) {
				ast_set_flag(wmsg, WFLAG_CONTENT);
				bytes = atoi(val);
			}

		}
		wmsg->last++;
	}

	wmsg->last--;

	if (bytes && ast_test_flag(wmsg, WFLAG_CONTENT)) {
		read(fd, wmsg->body, (bytes > sizeof(wmsg->body)) ? sizeof(wmsg->body) : bytes);
		if (globals.debug) {
			if (option_verbose > 2) {
				ast_verbose("%s\n", wmsg->body);
			}
		}
	}

	if (event_queue && ast_test_flag(wmsg, WFLAG_EVENT)) {
		if (globals.debug) {
			if (option_verbose > 2) {
				ast_verbose(WOOMERA_DEBUG_PREFIX "Queue Event: {%s} [%s]\n", profile->name, wmsg->command);
			}
		}
		/* we don't want events we want a reply so we will stash them for later */
		woomera_enqueue_event(event_queue, wmsg);

		/* call ourself recursively to find the reply. we'll keep doing this as long we get events.
		 * wmsg will be overwritten but it's ok we just queued it.
		 */
		return woomera_message_parse(fd, wmsg, timeout, profile, event_queue);
		
	} else if (wmsg->mval > 99 && wmsg->mval < 200) {
		/* reply in the 100's are nice but we need to wait for another reply 
		   call ourself recursively to find the reply > 199 and forget this reply.
		*/
		return woomera_message_parse(fd, wmsg, timeout, profile, event_queue);
	} else {
		return ast_test_flag(wmsg, WFLAG_EXISTS);
	}
}

static int woomera_message_parse_wait(private_object *tech_pvt, woomera_message *wmsg)
{
	int err=0;

	for (;;) {
                
		if (ast_test_flag(tech_pvt, TFLAG_ABORT)){
         		return -1;
		} 
		
 		err=woomera_message_parse(tech_pvt->command_channel,
                                           wmsg,
                                           100,
                                           tech_pvt->profile,
                                           &tech_pvt->event_queue);

		if (err == 0) {
			/* This is a timeout */
			continue;
		}  

	        break;
	}
        
	return err;	
}



static int tech_create_read_socket(private_object *tech_pvt)
{
	int retry=0;

retry_udp:

	ast_mutex_lock(&globals.woomera_port_lock);
	globals.next_woomera_port++;
	if (globals.next_woomera_port >= WOOMERA_MAX_PORT) {
		globals.next_woomera_port = WOOMERA_MIN_PORT;
	}
	tech_pvt->port = globals.next_woomera_port;
	ast_mutex_unlock(&globals.woomera_port_lock);

	
	if ((tech_pvt->udp_socket = create_udp_socket(tech_pvt->profile->audio_ip, tech_pvt->port, &tech_pvt->udpread, 0)) > -1) {
		struct ast_channel *owner = tech_get_owner(tech_pvt);
		if (owner) {
			owner->fds[0] = tech_pvt->udp_socket;
		} else {
			ast_log(LOG_ERROR, "Tech_pvt has no OWNER! %i\n",__LINE__);
		}

	} else {

		retry++;
		if (retry <= 1) {
			goto retry_udp;
		}

		
		if (globals.debug > 2) {
			ast_log(LOG_ERROR,
			"Error CREATING READ udp socket  %s/%i %s (%p) %s %s\n",
			tech_pvt->profile->audio_ip,tech_pvt->port, strerror(errno),tech_pvt,tech_pvt->callid,
			ast_test_flag(tech_pvt, TFLAG_OUTBOUND) ? "OUT":"IN");
		}
	}
	return tech_pvt->udp_socket;
}

#define WOOMERA_MAX_CALLS 600
static struct private_object *tech_pvt_idx[WOOMERA_MAX_CALLS];
static ast_mutex_t tech_pvt_idx_lock[WOOMERA_MAX_CALLS];
static struct timeval tech_pvt_lastused_idx[WOOMERA_MAX_CALLS];

static int find_free_callno_and_bind(private_object *tech_pvt)
{	
	int x=0;
	
	for (x=1;x<WOOMERA_MAX_CALLS;x++) {
		/* Find first unused call number that hasn't been used in a while */
		ast_mutex_lock(&tech_pvt_idx_lock[x]);
		if (!tech_pvt_idx[x]) {
			tech_pvt_idx[x]=tech_pvt;
			tech_pvt->callno=x;
			ast_mutex_unlock(&tech_pvt_idx_lock[x]);
			return x;
		}
		ast_mutex_unlock(&tech_pvt_idx_lock[x]);
	}	

	return 0;
}

static int tech_activate(private_object *tech_pvt) 
{
	int retry_activate_call=0;
	woomera_message wmsg;
	memset(&wmsg,0,sizeof(wmsg));

retry_activate_again:

	if (tech_pvt) {
		if((connect_woomera(&tech_pvt->command_channel, tech_pvt->profile, 0)) > -1) {
			if (globals.debug > 2) {
			ast_log(LOG_NOTICE, "Connected to woomera! chfd=%i port=%i dir=%s callid=%s Count=%i tpvt=%p\n",
				tech_pvt->command_channel,
				tech_pvt->port,
				ast_test_flag(tech_pvt, TFLAG_OUTBOUND)?"OUT":"IN",
				ast_test_flag(tech_pvt, TFLAG_OUTBOUND)?"N/A" : 
					tech_pvt->callid,
					tech_pvt->call_count,
				tech_pvt);
			}
		} else {
			if (retry_activate_call <= 3) {
                                 retry_activate_call++;
                        	goto retry_activate_again;
                       	}
		
				
			if (globals.debug > 1 && option_verbose > 1) {
				ast_log(LOG_ERROR, "Error: %s call connect to TCP/Woomera Server! tpvt=%p: %s\n",
						ast_test_flag(tech_pvt, TFLAG_OUTBOUND)?"Out":"In",
						tech_pvt,strerror(errno));
			}
			goto tech_activate_failed;
		}

		retry_activate_call=0;


		if (ast_test_flag(tech_pvt, TFLAG_OUTBOUND)) {

		  struct ast_channel *owner = tech_get_owner(tech_pvt);
		  if (owner) {

		  	if (owner->cid.cid_name) {
		    		strncpy(tech_pvt->cid_name, owner->cid.cid_name, 
					sizeof(tech_pvt->cid_name)-1);
		  	}
		  
		  	if (owner->cid.cid_num) {
		    		strncpy(tech_pvt->cid_num, owner->cid.cid_num,
			 		sizeof(tech_pvt->cid_num)-1);
		  	}
		 }

		  
		  woomera_printf(tech_pvt->profile,
				 tech_pvt->command_channel, 
				 "CALL %s%sRaw-Audio: %s/%d%sLocal-Name: %s!%s%sLocal-Number:%s%s", 
				 tech_pvt->dest, 
				 WOOMERA_LINE_SEPARATOR,
				 tech_pvt->profile->audio_ip,
				 tech_pvt->port,
				 WOOMERA_LINE_SEPARATOR,
				 tech_pvt->cid_name,
				 tech_pvt->cid_num,
				 WOOMERA_LINE_SEPARATOR,
				 tech_pvt->cid_num,
				 WOOMERA_RECORD_SEPARATOR
				 );

			 woomera_message_parse_wait(tech_pvt,&wmsg);
		} else {
			ast_set_flag(tech_pvt, TFLAG_PARSE_INCOMING);
			if (globals.debug > 2) {
				ast_log(LOG_NOTICE, "%s:%d Incoming Call %s tpvt=%p\n",
					__FUNCTION__,__LINE__,
					tech_pvt->callid,tech_pvt);
			}
			woomera_printf(tech_pvt->profile, 
					tech_pvt->command_channel, "LISTEN %s%s", 
					tech_pvt->callid,
					WOOMERA_RECORD_SEPARATOR);
#if 1			
			if (woomera_message_parse_wait(tech_pvt,&wmsg) < 0) {
				/* Its possible for a socket to break, just hangup */
#if 1
				if (!ast_test_flag(tech_pvt, TFLAG_ABORT)) {

					ast_log(LOG_ERROR, 
					  "{%s} %s:%d HELP! Woomera is broken! %s tpvt=%p\n", 
					  tech_pvt->profile->name,__FUNCTION__,__LINE__,
					  tech_pvt->profile->name,tech_pvt);

					woomera_close_socket(&tech_pvt->command_channel);

					if (retry_activate_call <= 3) {
						retry_activate_call++;
						goto retry_activate_again;
					}else{
						goto tech_activate_failed;
					}
				}
#endif
				ast_set_flag(tech_pvt, TFLAG_ABORT);
			}
#endif
		}
	} else {
		ast_log(LOG_ERROR, "Where's my tech_pvt?\n");
		goto tech_activate_failed;
	}

	if (globals.debug > 2) {
		ast_log(LOG_NOTICE, "TECH ACTIVATE OK tech_pvt=%p\n",tech_pvt);
	}
	return 0;

tech_activate_failed:
	if (globals.debug > 2) {
		ast_log(LOG_NOTICE, "TECH ACTIVATE FAILED tech_pvt=%p\n",tech_pvt);
	}
					
	woomera_close_socket(&tech_pvt->command_channel);
	ast_set_flag(tech_pvt, TFLAG_ABORT);

	/* At this point we cannot estabilsh a woomer
 	 * socket to the smg.  The SMG still doesnt know
 	 * about the incoming call that is now pending.
 	 * We must send a message to SMG to hangup the call */


	if (globals.debug > 2) {
  	ast_log(LOG_NOTICE, "Error: %s Call %s tpvt=%p Failed: CRITICAL\n",
                                ast_test_flag(tech_pvt, TFLAG_OUTBOUND) ? "OUT":"IN",
				tech_pvt->callid,tech_pvt);
	}

	if (!ast_test_flag(tech_pvt, TFLAG_OUTBOUND)) {
        	woomera_printf(tech_pvt->profile, tech_pvt->profile->woomera_socket,
                	"hangupmain %s%scause: %s%s",
                	tech_pvt->callid,
                	WOOMERA_LINE_SEPARATOR,
                	"Error",
        		WOOMERA_RECORD_SEPARATOR); 
	}

	return -1;

}

static int tech_init(private_object *tech_pvt, woomera_profile *profile, int flags) 
{

	gettimeofday(&tech_pvt->started, NULL);

	if (profile) {
		tech_pvt->profile = profile;
	} else {
		ast_log(LOG_ERROR, "ERROR: No Tech profile on init!\n");
		ast_set_flag(tech_pvt, TFLAG_ABORT);
		return -1;
	}

	ast_set_flag(tech_pvt, flags);
	
	if (tech_pvt->udp_socket < 0) {
		int rc;
		rc=tech_create_read_socket(tech_pvt);
		if (rc < 0){
			if (globals.debug > 2) {
				ast_log(LOG_ERROR, "ERROR: Failed to create UDP Socket (%p)!\n",
				tech_pvt);
			}
			ast_set_flag(tech_pvt, TFLAG_ABORT);
			return -1;
		}
	}

	ast_set_flag(tech_pvt, flags);


	/* Asterisk being asterisk and all allows approx 1 nanosecond 
	 * to try and establish a connetion here before it starts crying.
	 * Now asterisk, being unsure of it's self will not enforce a lock while we work
	 * and after even a 1 second delay it will give up on the lock and mess everything up
	 * This stems from the fact that asterisk will scan it's list of channels constantly for 
	 * silly reasons like tab completion and cli output.
	 *
	 * Anyway, since we've already spent that nanosecond with the previous line of code
	 * tech_create_read_socket(tech_pvt); to setup a read socket
	 * which, by the way, asterisk insists we have before going any furthur.  
	 * So, in short, we are between a rock and a hard place and asterisk wants us to open a socket here
	 * but it too impaitent to wait for us to make sure it's ready so in the case of outgoing calls
	 * we will defer the rest of the socket establishment process to the monitor thread.  This is, of course, common 
	 * knowledge since asterisk abounds in documentation right?, sorry to bother you with all this!
	 */
	if (globals.more_threads) {
		int err;
		ast_set_flag(tech_pvt, TFLAG_ACTIVATE);
		/* we're gonna try "wasting" a thread to do a better realtime monitoring */
		err=launch_tech_thread(tech_pvt);
		if (err) {
			ast_log(LOG_ERROR, "Error: Failed to Lauch the thread\n");
			ast_clear_flag(tech_pvt, TFLAG_ACTIVATE);
			ast_set_flag(tech_pvt, TFLAG_ABORT);
			return -1;
		} 
	} else {
		if (ast_test_flag(tech_pvt, TFLAG_OUTBOUND)) {
			ast_set_flag(tech_pvt, TFLAG_ACTIVATE);
		} else {
			tech_activate(tech_pvt);
		}
	}
	
	if (profile) {
		ast_mutex_lock(&profile->call_count_lock);
		profile->call_count++;
		tech_pvt->call_count = profile->call_count;
		ast_mutex_unlock(&profile->call_count_lock);
	}
	if (globals.debug > 2) {
		ast_log(LOG_NOTICE, "TECH INIT tech_pvt=%p c=%p (use=%i)\n",
			tech_pvt,tech_pvt->owner,usecount());
	}
	
	return 0;
}



static void tech_destroy(private_object *tech_pvt, struct ast_channel *owner) 
{

	ASTOBJ_CONTAINER_UNLINK(&private_object_list, tech_pvt);

	ast_set_flag(tech_pvt, TFLAG_DESTROY);
	ast_set_flag(tech_pvt, TFLAG_ABORT);


	if (globals.debug > 2) {
		ast_log(LOG_NOTICE, "Tech Destroy callid=%s tpvt=%p %s/%d\n",
						tech_pvt->callid,
						tech_pvt,
						tech_pvt->profile ? tech_pvt->profile->audio_ip : "NA",
						tech_pvt->port);
	}
	
	
	if (tech_pvt->profile && tech_pvt->command_channel > -1) {

		if (globals.debug > 1 && option_verbose > 1) {
			ast_log(LOG_NOTICE, "+++DESTROY sent HANGUP %s\n",
				tech_pvt->callid);
		}
		woomera_printf(tech_pvt->profile, tech_pvt->command_channel, "hangup %s%scause: %s%s", 
					   tech_pvt->callid,
					   WOOMERA_LINE_SEPARATOR, 
					   tech_pvt->ds, 
					   WOOMERA_RECORD_SEPARATOR);
		woomera_printf(tech_pvt->profile, tech_pvt->command_channel, "bye%s", WOOMERA_RECORD_SEPARATOR);
		woomera_close_socket(&tech_pvt->command_channel);
	}

	woomera_close_socket(&tech_pvt->command_channel);
	woomera_close_socket(&tech_pvt->udp_socket);

	if (owner) {
		struct ast_channel *chan=owner;
	
		if (!ast_test_flag(tech_pvt, TFLAG_PBX) && 
		    !chan->pbx &&
		    !ast_test_flag(tech_pvt, TFLAG_OUTBOUND)) {
			if (globals.debug > 2) {
				ast_log(LOG_NOTICE, "DESTROY Destroying Owner %p: ast_hangup()\n",
					tech_pvt);
			}
			chan->tech_pvt = NULL;
			ast_hangup(chan);
		} else {
			if (globals.debug > 2) {
				ast_log(LOG_NOTICE, "DESTROY Destroying Owner %p: softhangup\n",
					tech_pvt);
			}
			/* softhangup needs tech_pvt pointer */
			chan->tech_pvt = NULL;
			my_ast_softhangup(chan,  tech_pvt, AST_SOFTHANGUP_EXPLICIT);
		}
		tech_pvt->owner=NULL;
	}else{
		tech_pvt->owner=NULL;
	}

	if (tech_pvt->profile) {
		ast_mutex_lock(&tech_pvt->profile->call_count_lock);
		tech_pvt->profile->call_count--;
		if (tech_pvt->profile->call_count < 0) {
			tech_pvt->profile->call_count=0;
		}
		ast_mutex_unlock(&tech_pvt->profile->call_count_lock);
	}
	/* Tech profile is allowed to be null in case the call
	 * is blocked from the call_count */
	
#if 0
	ast_log(LOG_NOTICE, "---- Call END %p %s ----------------------------\n",
		tech_pvt,tech_pvt->callid);		
#endif
	
	tech_count--;
	if (tech_pvt->dsp){
		tech_pvt->dsp_features &= ~DSP_FEATURE_DTMF_DETECT;
                ast_dsp_set_features(tech_pvt->dsp, tech_pvt->dsp_features);
                tech_pvt->ast_dsp=0;

		free(tech_pvt->dsp);
		tech_pvt->dsp=NULL;
	}

	if (globals.debug > 2) {
		ast_log(LOG_NOTICE, "DESTROY Exit tech_pvt=%p  (use=%i)\n",
			tech_pvt,usecount());
	}
	
	ast_mutex_destroy(&tech_pvt->iolock);
	ast_mutex_destroy(&tech_pvt->event_queue.lock);

	free(tech_pvt);	
	ast_mutex_lock(&usecnt_lock);
	usecnt--;
	if (usecnt < 0) {
		usecnt = 0;
	}
	ast_mutex_unlock(&usecnt_lock);
}

#if 0

static int waitfor_socket(int fd, int timeout) 
{
	struct pollfd pfds[1];
	int res;
	int errflags = (POLLERR | POLLHUP | POLLNVAL);
	
	if (fd < 0) {
		return -1;
	}

	memset(&pfds[0], 0, sizeof(pfds[0]));
	pfds[0].fd = fd;
	pfds[0].events = POLLIN | errflags;
	res = poll(pfds, 1, timeout);
	if (res > 0) {
		if ((pfds[0].revents & errflags)) {
			res = -1;
		} else if ((pfds[0].revents & POLLIN)) {
			res = 1;
		} else {
			ast_log(LOG_ERROR, "System Error: Poll Event Error no event!\n");
			res = -1;
		}
	}
	
	return res;
}

#else

static int waitfor_socket(int fd, int timeout) 
{
	struct pollfd pfds[1];
	int res;

	if (fd < 0) {
		return -1;
	}

	memset(&pfds[0], 0, sizeof(pfds[0]));
	pfds[0].fd = fd;
	pfds[0].events = POLLIN | POLLERR;
	res = poll(pfds, 1, timeout);
	if (res > 0) {
		if ((pfds[0].revents & POLLERR)) {
			res = -1;
		} else if((pfds[0].revents & POLLIN)) {
			res = 1;
		} else {
			res = -1;
		}
	}
	
	return res;
}

#endif


static void *tech_monitor_thread(void *obj) 
{
	private_object *tech_pvt;
	woomera_message wmsg;
	char tcallid[WOOMERA_STRLEN];
	int aborted=0;

	int res = 0;

	tech_pvt = obj;

	memset(tcallid,0,sizeof(tcallid));
	memset(&wmsg,0,sizeof(wmsg));

	if (globals.debug > 2) {
		ast_log(LOG_NOTICE, "IN THREAD %s  rxgain=%f txtain=%f\n",
				tech_pvt->callid,
				default_profile.rxgain_val,
				default_profile.txgain_val);
	}


	for(;;) {
		
		if (globals.panic) {
			ast_set_flag(tech_pvt, TFLAG_ABORT);
		}

		if (!tech_pvt->owner) {
			ast_set_flag(tech_pvt, TFLAG_ABORT);
			if (globals.debug > 2) {
			ast_log(LOG_NOTICE, "Thread lost Owner Chan %s %p\n",
						tech_pvt->callid,
							tech_pvt);
			}
		}

		/* finish the deferred crap asterisk won't allow us to do live */
		if (ast_test_flag(tech_pvt, TFLAG_ABORT)) {
			int timeout_cnt=0;

			if (globals.debug > 2) {
				ast_log(LOG_NOTICE, "ABORT GOT HANGUP CmdCh=%i %s %s/%i\n",	
					tech_pvt->command_channel, tech_pvt->callid,
					tech_pvt->profile ? tech_pvt->profile->audio_ip : "N/A",
					tech_pvt->port);
			}

			aborted|=1;

			if (tech_pvt->profile && tech_pvt->command_channel > -1) {

				if (globals.debug > 2) {
				ast_log(LOG_NOTICE, "ABORT sent HANGUP on %s %p\n",
							tech_pvt->callid,
								tech_pvt);
				}
				aborted|=2;
		                woomera_printf(tech_pvt->profile, tech_pvt->command_channel, 
							"hangup %s%scause: %s%s",
							tech_pvt->callid,
							WOOMERA_LINE_SEPARATOR, 
							tech_pvt->ds, 
							WOOMERA_RECORD_SEPARATOR);
		                woomera_printf(tech_pvt->profile, tech_pvt->command_channel, 
							"bye%s", WOOMERA_RECORD_SEPARATOR);
				woomera_close_socket(&tech_pvt->command_channel);
			}

			if (tech_pvt->udp_socket > -1) {
				woomera_close_socket(&tech_pvt->udp_socket);
			}

			
			if (!ast_test_flag(tech_pvt, TFLAG_TECHHANGUP) && 
			    ast_test_flag(tech_pvt, TFLAG_PBX)) {
				struct ast_channel *owner = tech_get_owner(tech_pvt);

				if (owner) {
				aborted|=4;
				/* Do realy hangup here because we must wait for
                                 * asterisk to call our tech_hangup where the DESTROY
				 * flag will get set */
				//if (!ast_test_flag(chan, AST_FLAG_BLOCKING)) { 
				//	ast_softhangup(tech_pvt->owner, AST_SOFTHANGUP_EXPLICIT);
				//} else {
				//	ast_log(LOG_ERROR, 
				//		"%s: Abort calling my_ast_softhangup chan BLOCKED!\n",
				//		tech_pvt->callid);
				if (globals.debug > 2) {
	                        ast_log(LOG_NOTICE, "ABORT calling hangup on %s t=%p c=%p UP=%d\n",
                                                       tech_pvt->callid,
                                                                tech_pvt,
								owner,
								ast_test_flag(tech_pvt, TFLAG_UP));
                                }


#if 1
				while(tech_pvt->owner && ast_mutex_trylock(&tech_pvt->owner->lock)) {
                                        usleep(1);
                                }
                                if ((owner=tech_pvt->owner)) {
					tech_pvt->owner=NULL;
                                       /* Issue a softhangup */
                                       ast_softhangup(owner, AST_SOFTHANGUP_DEV);
                                       ast_mutex_unlock(&owner->lock);
                                }

#endif				//ast_set_flag(tech_pvt, TFLAG_TECHHANGUP);
#if 0
				//ast_queue_hangup(owner);
				
				if (ast_test_flag(tech_pvt, TFLAG_UP)) {
					ast_mutex_lock(&owner->lock);
					owner->_softhangup |= AST_SOFTHANGUP_EXPLICIT;
					ast_mutex_unlock(&owner->lock);
				} else {
					my_ast_softhangup(owner, tech_pvt,
					 		AST_SOFTHANGUP_EXPLICIT);
				}
#endif

#if 0
				my_ast_softhangup(owner, tech_pvt,
					 		AST_SOFTHANGUP_EXPLICIT);
#endif
				}
			}


			/* Wait for tech_hangup to set this, so there is on
  		         * race condition with asterisk */
			//ast_set_flag(tech_pvt, TFLAG_DESTROY);

			if (ast_test_flag(tech_pvt, TFLAG_PBX)) {
				while (!ast_test_flag(tech_pvt, TFLAG_DESTROY)) {
					timeout_cnt++;
					if (timeout_cnt > 4000) {  //10sec timeout
						/* Five second timeout */
						ast_log(LOG_ERROR, "ERROR: Wait on destroy timedout! %s tech_pvt=%p\n",
							tech_pvt->callid, tech_pvt);
						ast_set_flag(tech_pvt, TFLAG_DESTROY);
						break;
					}
					usleep(5000);
					sched_yield();
				}
			} else {
				ast_log(LOG_NOTICE, "NOTE: Skipping Wait on destroy timedout! %s tech_pvt=%p\n",
                                                        tech_pvt->callid, tech_pvt);
				ast_set_flag(tech_pvt, TFLAG_DESTROY);
			}

			aborted|=8;
			tech_destroy(tech_pvt,tech_get_owner(tech_pvt));
			tech_pvt = NULL;
			break;
		}


		if (ast_test_flag(tech_pvt, TFLAG_TECHHANGUP) || !tech_pvt->owner) {
			ast_set_flag(tech_pvt, TFLAG_DESTROY);
			ast_set_flag(tech_pvt, TFLAG_ABORT);
			if (globals.debug > 2) {
			ast_log(LOG_NOTICE, "Thread got HANGUP or no owner %s  %p tpvt=%p\n",
					tech_pvt->callid,tech_pvt,tech_pvt->owner);
			}
			goto tech_thread_continue;
		}

		if (ast_test_flag(tech_pvt, TFLAG_ACTIVATE)) {
			if (globals.debug > 2) {
			ast_log(LOG_NOTICE, "ACTIVATE %s tpvt=%p\n",
					tech_pvt->callid,tech_pvt);
			}
			ast_clear_flag(tech_pvt, TFLAG_ACTIVATE);
			tech_activate(tech_pvt);

			if (ast_test_flag(tech_pvt, TFLAG_ABORT)) {
				if (globals.debug > 2) {
					ast_log(LOG_NOTICE, "ACTIVATE ABORT Ch=%d\n",
						tech_pvt->command_channel);
				}
				goto tech_thread_continue;
				continue;
			} 

			{
			struct ast_channel *owner = tech_get_owner(tech_pvt);
			if (owner) {
					owner->hangupcause = AST_CAUSE_NORMAL_CLEARING;
			}
			}

			if (globals.debug > 2) {
			ast_log(LOG_NOTICE, "ACTIVATE DONE %s tpvt=%p\n",
					tech_pvt->callid,tech_pvt);
			}
		}

		if (ast_test_flag(tech_pvt, TFLAG_PARSE_INCOMING)) {
			char *exten;
			char *cid_name;
			char *cid_num;
			char *tg_string="1";
			int validext;
			ast_clear_flag(tech_pvt, TFLAG_PARSE_INCOMING);
			ast_set_flag(tech_pvt, TFLAG_INCOMING);
			wmsg = tech_pvt->call_info;
			
			if (globals.debug > 2) {
			ast_log(LOG_NOTICE, "WOOMERA EVENT %s : callid=%s tech_callid=%s tpvt=%p\n",
					wmsg.command,wmsg.callid,tech_pvt->callid,tech_pvt);
			}
			
			if (ast_strlen_zero(tech_pvt->profile->context)) {
				if (globals.debug > 2) {
				ast_log(LOG_NOTICE, "No Context Found %s tpvt=%p\n",
					tech_pvt->callid,tech_pvt);
				}
				ast_log(LOG_WARNING, "No context configured for inbound calls aborting call!\n");
				ast_set_flag(tech_pvt, TFLAG_ABORT);
				goto tech_thread_continue;
				continue;
			}
			
			exten = woomera_message_header(&wmsg, "Local-Number");
			if (! exten || ast_strlen_zero(exten)) {
				exten = "s";
			}
			
			tg_string = woomera_message_header(&wmsg, "Trunk-Group");
			if (!tg_string || ast_strlen_zero(tg_string)) {
				tg_string="1";	
			}

			cid_name = ast_strdupa(woomera_message_header(&wmsg, "Remote-Name"));

			if ((cid_num = strchr(cid_name, '!'))) {
				*cid_num = '\0';
				cid_num++;
			} else {
				cid_num = woomera_message_header(&wmsg, "Remote-Number");
			}
		

			{
			struct ast_channel *owner = tech_get_owner(tech_pvt);

			if (owner) {	
				snprintf(owner->context, sizeof(owner->context) - 1,
					        "%s%s", 
					        tech_pvt->profile->context,
						tg_string);	
					
				strncpy(owner->exten, exten, sizeof(owner->exten) - 1);
				ast_set_callerid(owner, cid_num, cid_name, cid_num);

				validext = ast_exists_extension(owner,
							owner->context,
							owner->exten,
							1,
							owner->cid.cid_num);
							
				if (globals.debug > 3){			
				ast_log(LOG_NOTICE, "Incoming Call exten %s@%s called %s!\n", 
							exten, 
							owner->context,
							tech_pvt->callid);
				}
			
			} else {
				validext=0;

			}			
			}


			if (!validext) {
				
				if (globals.debug > 1){	
				ast_log(LOG_ERROR, "Error: Invalid exten %s@%s%s called %s!\n", 
						exten, 
						tech_pvt->profile->context,
						tg_string,
						tech_pvt->callid);
				}
				
				woomera_printf(tech_pvt->profile, tech_pvt->command_channel, 
						"hangup %s%scause: INVALID%s", wmsg.callid, 
						WOOMERA_LINE_SEPARATOR, WOOMERA_RECORD_SEPARATOR);
				
				if (woomera_message_parse_wait(tech_pvt,&wmsg) < 0) {
					ast_set_flag(tech_pvt, TFLAG_ABORT);
					if (globals.debug > 2) {	
						ast_log(LOG_NOTICE, "NO EXT ABORT Ch=%d\n",
						tech_pvt->command_channel);
					}
					goto tech_thread_continue;
					continue;
				}
				if (!(wmsg.mval >= 200 && wmsg.mval <= 299)) {
					ast_set_flag(tech_pvt, TFLAG_ABORT);
				}
				goto tech_thread_continue;
				continue;
			}


			woomera_printf(tech_pvt->profile, tech_pvt->command_channel, 
							"%s %s%s"
							"Raw-Audio: %s/%d%s",
							MEDIA_ANSWER,
							wmsg.callid,
							WOOMERA_LINE_SEPARATOR,
							tech_pvt->profile->audio_ip,
							tech_pvt->port,
							WOOMERA_RECORD_SEPARATOR);

			if(woomera_message_parse_wait(tech_pvt,&wmsg) < 0) {
				ast_set_flag(tech_pvt, TFLAG_ABORT);
				ast_log(LOG_NOTICE, "MEDIA ANSWER ABORT Ch=%d\n",
					tech_pvt->command_channel);
				goto tech_thread_continue;
				continue;
			} 
		}


		if (ast_test_flag(tech_pvt, TFLAG_ANSWER)) {
			if (globals.debug > 2) {
			ast_log(LOG_NOTICE, "ANSWER %s tpvt=%p\n",
					tech_pvt->callid,tech_pvt);
			}
			ast_clear_flag(tech_pvt, TFLAG_ANSWER);
			
			if (ast_test_flag(tech_pvt, TFLAG_OUTBOUND)) {
				ast_log(LOG_ERROR,"Error: ANSWER on OUTBOUND Call! (skipped) %s\n",
						tech_pvt->callid);
			} else {	
#ifdef USE_ANSWER
				woomera_printf(tech_pvt->profile, tech_pvt->command_channel, "ANSWER %s%s",tech_pvt->callid, WOOMERA_RECORD_SEPARATOR);
				
				if(woomera_message_parse_wait(tech_pvt,&wmsg) < 0) {
					ast_set_flag(tech_pvt, TFLAG_ABORT);
					ast_log(LOG_NOTICE, "ANSWER ABORT Ch=%d\n",
							tech_pvt->command_channel);
					goto tech_thread_continue;
					continue;
				}
#endif
			}
		}
		
		if (ast_test_flag(tech_pvt, TFLAG_DTMF)) {
			if (globals.debug > 2) {
			ast_log(LOG_NOTICE, "DTMF %s tpvt=%p\n",
					tech_pvt->callid,tech_pvt);
			}
			woomera_printf(tech_pvt->profile, tech_pvt->command_channel, "DTMF %s %s%s",tech_pvt->callid, tech_pvt->dtmfbuf, WOOMERA_RECORD_SEPARATOR);
			if(woomera_message_parse_wait(tech_pvt,&wmsg) < 0) {
				ast_set_flag(tech_pvt, TFLAG_ABORT);
				ast_log(LOG_NOTICE, "DTMF ABORT Ch=%d\n",
						tech_pvt->command_channel);
				goto tech_thread_continue;
				continue;
			}
			ast_mutex_lock(&tech_pvt->iolock);
			ast_clear_flag(tech_pvt, TFLAG_DTMF);
			memset(tech_pvt->dtmfbuf, 0, sizeof(tech_pvt->dtmfbuf));
			ast_mutex_unlock(&tech_pvt->iolock);
		}

		if(tech_pvt->timeout) {
			struct timeval now;
			int elapsed;
			gettimeofday(&now, NULL);
			elapsed = (((now.tv_sec * 1000) + now.tv_usec / 1000) - 
			((tech_pvt->started.tv_sec * 1000) + tech_pvt->started.tv_usec / 1000));
			if (elapsed > tech_pvt->timeout) {
				/* call timed out! */
				if (globals.debug > 2) {
				ast_log(LOG_NOTICE, "CALL TIMED OUT %s tpvt=%p\n",
					tech_pvt->callid,tech_pvt);
				}
				ast_set_flag(tech_pvt, TFLAG_ABORT);
			}
		}
		
		if (globals.debug > 2) {
			if (tcallid[0] == 0) {
				strncpy(tcallid,tech_pvt->callid,sizeof(tcallid)-1);
			}
		}
		
		if (tech_pvt->command_channel < 0) {
			if (!globals.more_threads) {
				goto tech_thread_continue;
				continue;
			} else {
				if (globals.debug > 2) {
				ast_log(LOG_NOTICE, "No Command Channel %s tpvt=%p\n",
					tech_pvt->callid,tech_pvt);
				}
				ast_set_flag(tech_pvt, TFLAG_ABORT);
				goto tech_thread_continue;
				continue;
			}
		}
		/* Check for events */
		if((res = woomera_dequeue_event(&tech_pvt->event_queue, &wmsg)) ||
		   (res = woomera_message_parse(tech_pvt->command_channel,
										&wmsg,
										100,
										tech_pvt->profile,
										NULL
										))) {

			if (globals.debug > 2) {
				 
				if (!strcasecmp(wmsg.command, "INCOMING") ) {
					/* Do not print it here */
				}else{
					ast_log(LOG_NOTICE, "WOOMERA EVENT %s : callid=%s tech_callid=%s tpvt=%p\n",
						wmsg.command,wmsg.callid,tech_pvt->callid,tech_pvt);
				}	
			}
			
			if (res < 0) {
				if (globals.debug > 2) {
					ast_log(LOG_NOTICE, "INVALID MESSAGE PARSE COMMAND %s tpvt=%p\n",
						tech_pvt->callid,tech_pvt);
				}
				ast_set_flag(tech_pvt, TFLAG_ABORT);
				goto tech_thread_continue;
				continue;

			} else if (ast_test_flag(tech_pvt, TFLAG_ABORT)) {
				if (globals.debug > 2) {
					ast_log(LOG_NOTICE, "GOT ABORT WHILE WAITING FOR EVENT %s tpvt=%p\n",
						tech_pvt->callid,tech_pvt);
				}
				goto tech_thread_continue;
				continue;
					
			} else if (!strcasecmp(wmsg.command, "HANGUP")) {
				char *cause;
				struct ast_channel *owner = tech_get_owner(tech_pvt);

				if (option_verbose > 2) {
					ast_verbose(WOOMERA_DEBUG_PREFIX "Hangup [%s]\n", tech_pvt->callid);
				}
				cause = woomera_message_header(&wmsg, "Cause");

				if (cause && owner) {
					owner->hangupcause = string_to_release(cause);
					if (globals.debug > 2) {
					ast_log(LOG_NOTICE, "EVENT HANGUP with OWNER with Cause %s  %s tpvt=%p\n",
							cause,tech_pvt->callid,tech_pvt);
					}
				}else{
         			  	if (globals.debug > 2) {
                                    	ast_log(LOG_NOTICE, "EVENT HANGUP with Cause %s  %s tpvt=%p\n",
                                             cause,tech_pvt->callid,tech_pvt);
                                  	}
				}

				
				ast_set_flag(tech_pvt, TFLAG_ABORT);
				goto tech_thread_continue;
				continue;
	
			} else if (!strcasecmp(wmsg.command, "DTMF")) {
				struct ast_frame dtmf_frame = {AST_FRAME_DTMF};
				int x = 0;
				for (x = 0; x < strlen(wmsg.command_args); x++) {
					struct ast_channel *owner = tech_get_owner(tech_pvt);
					dtmf_frame.subclass = wmsg.command_args[x];

					if (owner) {
					ast_queue_frame(owner, ast_frdup(&dtmf_frame));
					}

					if (globals.debug > 1 && option_verbose > 1) {
						if (option_verbose > 2) {
							ast_verbose(WOOMERA_DEBUG_PREFIX "SEND DTMF [%c] to %s\n", dtmf_frame.subclass,tech_pvt->callid);
						}
					}
				}

			} else if (!strcasecmp(wmsg.command, "PROCEED")) {
				/* This packet has lots of info so well keep it */
				tech_pvt->call_info = wmsg;
				memcpy(tech_pvt->callid,wmsg.callid,sizeof(wmsg.callid));

			} else if (ast_test_flag(tech_pvt, TFLAG_PARSE_INCOMING) && 
				  !strcasecmp(wmsg.command, "INCOMING")) {
				  
				  ast_log(LOG_ERROR, "INVALID WOOMERA EVENT %s : callid=%s tech_callid=%s tpvt=%p\n",
						wmsg.command,wmsg.callid,tech_pvt->callid,tech_pvt);
#if 0				  
				char *exten;
				char *cid_name;
				char *cid_num;
				char *tg_string="1";
				int validext;
				ast_clear_flag(tech_pvt, TFLAG_PARSE_INCOMING);
				ast_set_flag(tech_pvt, TFLAG_INCOMING);
				tech_pvt->call_info = wmsg;
				
				ast_log(LOG_ERROR, "INVALID WOOMERA EVENT %s : callid=%s tech_callid=%s tpvt=%p\n",
						wmsg.command,wmsg.callid,tech_pvt->callid,tech_pvt);

				if (ast_strlen_zero(tech_pvt->profile->context)) {
					if (globals.debug > 2) {
					ast_log(LOG_NOTICE, "No Context Found %s tpvt=%p\n",
						tech_pvt->callid,tech_pvt);
					}
					ast_log(LOG_WARNING, "No context configured for inbound calls aborting call!\n");
					ast_set_flag(tech_pvt, TFLAG_ABORT);
					goto tech_thread_continue;
					continue;
				}

				exten = woomera_message_header(&wmsg, "Local-Number");
				if (! exten || ast_strlen_zero(exten)) {
					exten = "s";
				}
				
				tg_string = woomera_message_header(&wmsg, "Trunk-Group");
				if (!tg_string || ast_strlen_zero(tg_string)) {
					tg_string="1";	
				}

				cid_name = ast_strdupa(woomera_message_header(&wmsg, "Remote-Name"));
				if ((cid_num = strchr(cid_name, '!'))) {
					*cid_num = '\0';
					cid_num++;
				} else {
					cid_num = woomera_message_header(&wmsg, "Remote-Number");
				}
				
				
				{
				struct ast_channel *owner = tech_get_owner(tech_pvt);
				if (owner) {
					snprintf(owner->context, sizeof(owner->context) - 1,
					        "%s%s", 
					        tech_pvt->profile->context,
						tg_string);
					strncpy(owner->exten, exten, sizeof(owner->exten) - 1);
					ast_set_callerid(owner, cid_num, cid_name, cid_num);

					validext = ast_exists_extension(owner,
							  owner->context,
							  owner->exten,
							  1,
							  owner->cid.cid_num);
							  
					ast_log(LOG_NOTICE, "Incoming Call exten %s@%s%s called %s!\n", 
							exten, 
							owner->context,
							tg_string,
							tech_pvt->callid);
				}else {
					validext=0;
				}
				}
				

				if (!validext) {
					
					if (globals.debug > 1){	
					ast_log(LOG_ERROR, "Error: Invalid exten %s@%s%s called %s!\n", 
							exten, 
							tech_pvt->profile->context,
							tg_string,
							tech_pvt->callid);
					}
					
					woomera_printf(tech_pvt->profile, tech_pvt->command_channel, 
							"hangup %s%scause: INVALID%s", wmsg.callid, 
							WOOMERA_LINE_SEPARATOR, WOOMERA_RECORD_SEPARATOR);
					
					if (woomera_message_parse_wait(tech_pvt,&wmsg) < 0) {
						ast_set_flag(tech_pvt, TFLAG_ABORT);
						ast_log(LOG_NOTICE, "NO EXT ABORT Ch=%d\n",
						tech_pvt->command_channel);
						goto tech_thread_continue;
						continue;
					}
					if (!(wmsg.mval >= 200 && wmsg.mval <= 299)) {
						ast_set_flag(tech_pvt, TFLAG_ABORT);
					}
					goto tech_thread_continue;
					continue;
				}


				woomera_printf(tech_pvt->profile, tech_pvt->command_channel, 
							   "%s %s%s"
							   "Raw-Audio: %s/%d%s",
							   MEDIA_ANSWER,
							   wmsg.callid,
							   WOOMERA_LINE_SEPARATOR,
							   tech_pvt->profile->audio_ip,
							   tech_pvt->port,
							   WOOMERA_RECORD_SEPARATOR);

				if(woomera_message_parse_wait(tech_pvt,&wmsg) < 0) {
					ast_set_flag(tech_pvt, TFLAG_ABORT);
					ast_log(LOG_NOTICE, "MEDIA ANSWER ABORT Ch=%d\n",
						tech_pvt->command_channel);
					goto tech_thread_continue;
					continue;
				} 
#endif			
			} else if (!strcasecmp(wmsg.command, "CONNECT")) {
				struct ast_frame answer_frame = {AST_FRAME_CONTROL, AST_CONTROL_ANSWER};
				struct ast_channel *owner = tech_get_owner(tech_pvt);

				if (owner) {
					ast_setstate(owner, AST_STATE_UP);
					ast_queue_frame(owner, &answer_frame);
					ast_set_flag(tech_pvt, TFLAG_UP);
				}else{
					ast_set_flag(tech_pvt, TFLAG_ABORT);
				}


			} else if (!strcasecmp(wmsg.command, "MEDIA")) {
				char *raw_audio_header;
				
				if ((raw_audio_header = woomera_message_header(&wmsg, "Raw-Audio"))) {
					char ip[25];
					char *ptr;
					int port = 0;
					struct hostent *hp;
					struct ast_hostent ahp;   

					strncpy(ip, raw_audio_header, sizeof(ip) - 1);
					if ((ptr=strchr(ip, '/'))) {
						*ptr = '\0';
						ptr++;
						port = atoi(ptr);
					}
					
					if (!ast_strlen_zero(ip) && (hp = ast_gethostbyname(ip, &ahp))) {
						tech_pvt->udpwrite.sin_family = hp->h_addrtype;
						memcpy((char *) &tech_pvt->udpwrite.sin_addr.s_addr, hp->h_addr_list[0], hp->h_length);
						tech_pvt->udpwrite.sin_port = htons(port);
						ast_set_flag(tech_pvt, TFLAG_MEDIA);
						tech_pvt->timeout = 0;


						{
						struct ast_channel *owner = tech_get_owner(tech_pvt);

						if (owner) {

						ast_setstate(owner, AST_STATE_RINGING);

						if (ast_test_flag(tech_pvt, TFLAG_INBOUND)) {
							if (ast_pbx_start(owner)) {
								ast_log(LOG_ERROR, "Unable to start PBX on %s \n", 
										tech_pvt->callid);
								ast_set_flag(tech_pvt, TFLAG_ABORT);
								ast_clear_flag(tech_pvt, TFLAG_PBX);
								//ast_hangup();
								/* Let destroy hangup */
							} else {
								ast_set_flag(tech_pvt, TFLAG_PBX);
								owner->hangupcause = AST_CAUSE_NORMAL_CLEARING;
								woomera_send_progress(tech_pvt);
							}
						} else { 
							/* Do nothing for the outbound call
								* The PBX flag was already set! */
						}
						} 
						} 

						
					} else {
						if (globals.debug) {
							ast_log(LOG_ERROR, "{%s} Cannot resolve %s\n", tech_pvt->profile->name, ip);
						}
						ast_set_flag(tech_pvt, TFLAG_ABORT);
					}
				}
			} else {
				if (!strcasecmp(wmsg.command, "INCOMING") ) {
					/* Do not print it here */
				}else{
				ast_log(LOG_ERROR, "Woomera Invalid CMD %s %s\n", 
							wmsg.command,tech_pvt->callid);
				}
			}
		}
		if (globals.debug > 4) {
			if (option_verbose > 2) {
				ast_verbose(WOOMERA_DEBUG_PREFIX "CHECK {%s} (%d) %s\n", 
						tech_pvt->profile->name,  
						res,tech_pvt->callid);
			}
		}
		
tech_thread_continue:
		
		if (!globals.more_threads) {
			ast_log(LOG_NOTICE, "EXITING THREAD on more threads %s\n",
						tcallid);
			break;
		}
		
	}

	if (globals.debug > 2) {
		ast_log(LOG_NOTICE, "OUT THREAD %s 0x%X\n",tcallid,aborted);
	}
	
	return NULL;
}

static int woomera_profile_thread_running(woomera_profile *profile, int set, int new) 
{
	int running = 0;

	ast_mutex_lock(&profile->iolock);
	if (set) {
		profile->thread_running = new;
	}
	running = profile->thread_running;
	ast_mutex_unlock(&profile->iolock);
	return running;
	
}

static int woomera_locate_socket(woomera_profile *profile, int *woomera_socket) 
{
	woomera_message wmsg;

	memset(&wmsg,0,sizeof(wmsg));	
	
	for (;;) {

		while (connect_woomera(woomera_socket, profile, 0) < 0) {
			if(!woomera_profile_thread_running(profile, 0, 0)) {
				break;
			}
			if (globals.panic > 2) {
				break;
			}
			ast_log(LOG_WARNING, "{%s} Cannot Reconnect to Woomera! retry in 5 seconds\n", profile->name);
			sleep(5);
		}

		if (*woomera_socket > -1) {
			if (ast_test_flag(profile, PFLAG_INBOUND)) {
				if (globals.debug > 2) {
				ast_log(LOG_NOTICE, "%s:%d Incoming Call \n",__FUNCTION__,__LINE__);
				}
				woomera_printf(profile, *woomera_socket, "LISTEN MASTER%s", WOOMERA_RECORD_SEPARATOR);
				if (woomera_message_parse(*woomera_socket,
										  &wmsg,
										  WOOMERA_HARD_TIMEOUT,
										  profile,
										  &profile->event_queue
										  ) < 0) {
					ast_log(LOG_ERROR, "{%s} %s:%d HELP! Woomera is broken!\n", 
							profile->name,__FUNCTION__,__LINE__);
					globals.panic = 1;
					if (*woomera_socket > -1) {
						woomera_close_socket(woomera_socket);
					}
					continue;
				}
			}

		}
		usleep(100);
		break;
	}	
	return *woomera_socket;
}

static void tech_monitor_in_one_thread(void) 
{
	private_object *tech_pvt;

	ASTOBJ_CONTAINER_TRAVERSE(&private_object_list, 1, do {
		ASTOBJ_RDLOCK(iterator);
        tech_pvt = iterator;
		tech_monitor_thread(tech_pvt);
		ASTOBJ_UNLOCK(iterator);
    } while(0));
}

static void *woomera_thread_run(void *obj) 
{

	int woomera_socket = -1, res = 0, res2=0;
	woomera_message wmsg;
	woomera_profile *profile;

	memset(&wmsg,0,sizeof(wmsg));

	profile = obj;
	ast_log(LOG_NOTICE, "Started Woomera Thread {%s}.\n", profile->name);
 
	profile->thread_running = 1;


	while(woomera_profile_thread_running(profile, 0, 0)) {
		/* listen on socket and handle events */

		if (globals.panic > 2) {
			break;
		}

		if (globals.panic == 2) {
			ast_log(LOG_NOTICE, "Woomera is disabled!\n");
			sleep(5);
			continue;
		}

		if (woomera_socket < 0) {
			if (woomera_locate_socket(profile, &woomera_socket)) {
				globals.panic = 0;
			}
			if (!woomera_profile_thread_running(profile, 0, 0)) {
				break;
			}
			profile->woomera_socket=woomera_socket;
			ast_log(LOG_NOTICE, "Woomera Thread Up {%s} %s/%d\n", 
					profile->name, profile->woomera_host, profile->woomera_port);

		}

		if (globals.panic) {
			if (globals.panic != 2) {
				ast_log(LOG_ERROR, "Help I'm in a state of panic!\n");
			}
			if (woomera_socket > -1) {
				woomera_close_socket(&woomera_socket);
			}
			continue;
		}
		if (!globals.more_threads) {
			if (woomera_socket > -1) {
				tech_monitor_in_one_thread();
			}
		}

		if ((res = woomera_dequeue_event(&profile->event_queue, &wmsg) ||
		    (res2 = woomera_message_parse(woomera_socket,
						  &wmsg,
					  /* if we are not stingy with threads we can block forever */
						  globals.more_threads ? 0 : 500,
						  profile,
						  NULL
						  )))) {


			if (res2 < 0) {
				ast_log(LOG_ERROR, "{%s} HELP! I lost my connection to woomera!\n", profile->name);
				if (woomera_socket > -1) {
					woomera_close_socket(&woomera_socket);
				}
				global_set_flag(TFLAG_ABORT);
				if (globals.panic > 2) {
					break;
				}

				globals.panic = 1;
				continue;

				if (woomera_socket > -1) {
					if (ast_test_flag(profile, PFLAG_INBOUND)) {
						if (globals.debug > 2) {
						ast_log(LOG_NOTICE, "%s:%d Incoming Call \n",__FUNCTION__,__LINE__);
						}
						woomera_printf(profile, woomera_socket, "LISTEN%s", WOOMERA_RECORD_SEPARATOR);
						if(woomera_message_parse(woomera_socket,
												 &wmsg,
												 WOOMERA_HARD_TIMEOUT,
												 profile,
												 &profile->event_queue
												 ) < 0) {
							ast_log(LOG_ERROR, "{%s} %s:%d HELP! Woomera is broken!\n", profile->name,__FUNCTION__,__LINE__);
							globals.panic = 1;
							woomera_close_socket(&woomera_socket);
						} 
					}
					if (woomera_socket > -1) {
						ast_log(LOG_NOTICE, "Woomera Thread Up {%s} %s/%d\n", profile->name, profile->woomera_host, profile->woomera_port);
					}
				}
				continue;
			}

			if (!strcasecmp(wmsg.command, "INCOMING")) {
			
				int err=1;
				int cause = 0;
				struct ast_channel *inchan;
				char *name = "Woomera";

				if (!(name = woomera_message_header(&wmsg, "Remote-Address"))) {
					name = woomera_message_header(&wmsg, "Channel-Name");
				}

				if (!name) {
					name=wmsg.callid;
				}

				if (!name) {
					name="smg";
				}

				if (globals.debug > 2) {	
					ast_log(LOG_NOTICE, "NEW INBOUND CALL %s!\n",wmsg.callid);
				}

				if ((inchan = woomera_new(type, WFORMAT, name, &cause))) {
					private_object *tech_pvt;
					tech_pvt = inchan->tech_pvt;
					/* Save the call id */
					tech_pvt->call_info = wmsg;
					memcpy(tech_pvt->callid,wmsg.callid,sizeof(tech_pvt->callid));
					err=tech_init(tech_pvt, profile, TFLAG_INBOUND);
					if (err) {
						if(globals.debug > 2) {
						ast_log(LOG_ERROR, "Error: Inbound Call Failed %s %p\n",
								wmsg.callid,
								tech_pvt);
						}
						tech_destroy(tech_pvt,inchan);
					}
				} else {
					ast_log(LOG_ERROR, "Cannot Create new Inbound Channel!\n");
				}
				
				if (err) {
					if(globals.debug > 3) {
						ast_log(LOG_ERROR, "Error: Inbound Call Hungup %s\n",
							wmsg.callid);
					}
					woomera_printf(profile, woomera_socket, "hangupmain %s%scause: %s%s", 
					   wmsg.callid,
					   WOOMERA_LINE_SEPARATOR, 
					   "Error", 
					   WOOMERA_RECORD_SEPARATOR);
				}
			}
		}
		if(globals.debug > 4) {
			if (option_verbose > 2) {
				ast_verbose(WOOMERA_DEBUG_PREFIX "Main Thread {%s} Select Return %d\n", profile->name, res);
			}
		}
		usleep(100);
	}
	

	if (woomera_socket > -1) {
		woomera_printf(profile, woomera_socket, "BYE%s", WOOMERA_RECORD_SEPARATOR);
		if(woomera_message_parse(woomera_socket,
								 &wmsg,
								 WOOMERA_HARD_TIMEOUT,
								 profile,
								 &profile->event_queue
								 ) < 0) {
		}
		woomera_close_socket(&woomera_socket);
	}

	ast_set_flag(profile, PFLAG_DISABLED);

	ast_log(LOG_NOTICE, "Ended Woomera Thread {%s}.\n", profile->name);
	woomera_profile_thread_running(profile, 1, -1);
	return NULL;
}

static void launch_woomera_thread(woomera_profile *profile) 
{
	pthread_attr_t attr;
	int result = 0;

	result = pthread_attr_init(&attr);
	pthread_attr_setschedpolicy(&attr, SCHED_RR);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	result = ast_pthread_create(&profile->thread, &attr, woomera_thread_run, profile);
	result = pthread_attr_destroy(&attr);
}


static int launch_tech_thread(private_object *tech_pvt) 
{
	pthread_attr_t attr;
	int result = 0;

	if (globals.debug > 2) {
		if (option_verbose > 2) {
			ast_verbose(WOOMERA_DEBUG_PREFIX "+++LAUCN TECH THREAD\n");
		}
	}

	result = pthread_attr_init(&attr);
	pthread_attr_setschedpolicy(&attr, SCHED_RR);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	ast_set_flag(tech_pvt, TFLAG_INTHREAD);
	result = ast_pthread_create(&tech_pvt->thread, &attr, tech_monitor_thread, tech_pvt);
	if (result) {
		ast_clear_flag(tech_pvt, TFLAG_INTHREAD);
		ast_log(LOG_ERROR, "Error: Failed to launch tech thread \n");
	}
	pthread_attr_destroy(&attr);
	
	return result;
}

static void woomera_config_gain(woomera_profile *profile, float gain_val, int rx) 
{
	int j;
	int k;
	float linear_gain = pow(10.0, gain_val / 20.0);
	unsigned char *gain;
		
	if (gain_val == 0) {
		goto woomera_config_gain_skip;
	}
	if (rx) {
		gain = profile->rxgain;
	} else {
		gain = profile->txgain;
	}
	
	switch (profile->coding) {
	
	case AST_FORMAT_ALAW:
		for (j = 0; j < 256; j++) {
			if (gain_val) {
				k = (int) (((float) alaw_to_linear(j)) * linear_gain);
				if (k > 32767) k = 32767;
				if (k < -32767) k = -32767;
				gain[j] = linear_to_alaw(k);
			} else {
				gain[j] = j;
			}
		}
		break;
	case AST_FORMAT_ULAW:
		for (j = 0; j < 256; j++) {
			if (gain_val) {
				k = (int) (((float) ulaw_to_linear(j)) * linear_gain);
				if (k > 32767) k = 32767;
				if (k < -32767) k = -32767;
				gain[j] = linear_to_ulaw(k);
			} else {
				gain[j] = j;
			}
		}
		break;
	}	

woomera_config_gain_skip:

	if (rx) {
		profile->rxgain_val=gain_val;
	} else {
		profile->txgain_val=gain_val;
	}

}

static void destroy_woomera_profile(woomera_profile *profile) 
{
	if (profile && ast_test_flag(profile, PFLAG_DYNAMIC)) {
		ast_mutex_destroy(&profile->iolock);
		ast_mutex_destroy(&profile->call_count_lock);
		ast_mutex_destroy(&profile->event_queue.lock);
		free(profile);
	}
}

static woomera_profile *clone_woomera_profile(woomera_profile *new_profile, woomera_profile *default_profile) 
{
	return memcpy(new_profile, default_profile, sizeof(woomera_profile));
}

static woomera_profile *create_woomera_profile(woomera_profile *default_profile) 
{
	woomera_profile *profile;

	if((profile = malloc(sizeof(woomera_profile)))) {
		clone_woomera_profile(profile, default_profile);
		ast_mutex_init(&profile->iolock);
		ast_mutex_init(&profile->call_count_lock);
		ast_mutex_init(&profile->event_queue.lock);
		ast_set_flag(profile, PFLAG_DYNAMIC);
	}
	return profile;
}

static int config_woomera(void) 
{
	struct ast_config *cfg;
	char *entry;
	struct ast_variable *v;
	woomera_profile *profile;
	int count = 0;

	memset(&default_profile, 0, sizeof(default_profile));
	
	default_profile.coding=AST_FORMAT_SLINEAR;
	
	if ((cfg = ast_config_load(configfile))) {
		for (entry = ast_category_browse(cfg, NULL); entry != NULL; entry = ast_category_browse(cfg, entry)) {
			if (!strcmp(entry, "settings")) {
				for (v = ast_variable_browse(cfg, entry); v ; v = v->next) {
					if (!strcmp(v->name, "debug")) {
						globals.debug = atoi(v->value);
					} else if (!strcmp(v->name, "more_threads")) {
						globals.more_threads = ast_true(v->value);
					}
				}

#ifdef AST_JB
				struct ast_variable *vjb;
                                /* Copy the default jb config over global_jbconf */
                                memcpy(&global_jbconf, &default_jbconf, sizeof(struct ast_jb_conf));
                                /* Traverse all variables to handle jb conf */
                                vjb = ast_variable_browse(cfg, "settings");;
                                while(vjb)
                                {
                                        ast_jb_read_conf(&global_jbconf, vjb->name, vjb->value);
                                        vjb = vjb->next;
                                }
#endif /* AST_JB */


			} else {
				int new = 0;
				float gain;
				count++;
				if (!strcmp(entry, "default")) {
					profile = &default_profile;
				} else {
					if((profile = ASTOBJ_CONTAINER_FIND(&woomera_profile_list, entry))) {
						clone_woomera_profile(profile, &default_profile);
					} else {
						if((profile = create_woomera_profile(&default_profile))) {
							new = 1;
						} else {
							ast_log(LOG_ERROR, "Memory Error!\n");
						}
					}
				}
				strncpy(profile->name, entry, sizeof(profile->name) - 1);
				/*default is inbound and outbound enabled */
				ast_set_flag(profile, PFLAG_INBOUND | PFLAG_OUTBOUND);
				for (v = ast_variable_browse(cfg, entry); v ; v = v->next) {
					if (!strcmp(v->name, "audio_ip")) {
						strncpy(profile->audio_ip, v->value, sizeof(profile->audio_ip) - 1);
					} else if (!strcmp(v->name, "host")) {
						strncpy(profile->woomera_host, v->value, sizeof(profile->woomera_host) - 1);
					} else if (!strcmp(v->name, "max_calls")) {
						int max = atoi(v->value);
						if (max > 0) {
							profile->max_calls = max;
						}
					} else if (!strcmp(v->name, "port")) {
						profile->woomera_port = atoi(v->value);
					} else if (!strcmp(v->name, "disabled")) {
						ast_set2_flag(profile, ast_true(v->value), PFLAG_DISABLED);
					} else if (!strcmp(v->name, "inbound")) {
						if (ast_false(v->value)) {
							ast_clear_flag(profile, PFLAG_INBOUND);
						}
					} else if (!strcmp(v->name, "outbound")) {
						if (ast_false(v->value)) {
							ast_clear_flag(profile, PFLAG_OUTBOUND);
						}
					} else if (!strcmp(v->name, "context")) {
						strncpy(profile->context, v->value, sizeof(profile->context) - 1);
					} else if (!strcmp(v->name, "dtmf_enable")) {
						profile->dtmf_enable = atoi(v->value);
					} else if (!strcmp(v->name, "jb_enable")) {
                                                profile->jb_enable = atoi(v->value);
					} else if (!strcmp(v->name, "progress_enable")) {
                                                profile->progress_enable = atoi(v->value);
					} else if (!strcmp(v->name, "rxgain")) {
                                                if (sscanf(v->value, "%f", &gain) != 1) {
							ast_log(LOG_WARNING, "Invalid rxgain: %s\n", v->value);
						} else {
							woomera_config_gain(profile,gain,1);
						}	
					} else if (!strcmp(v->name, "txgain")) {
						 if (sscanf(v->value, "%f", &gain) != 1) {
							ast_log(LOG_WARNING, "Invalid txgain: %s\n", v->value);
						} else {
							woomera_config_gain(profile,gain,0);
						}	
					} 
				}

				ASTOBJ_CONTAINER_LINK(&woomera_profile_list, profile);
			}
		}
		ast_config_destroy(cfg);
	} else {
		return 0;
	}

	return count;

}

static int create_udp_socket(char *ip, int port, struct sockaddr_in *sockaddr, int client)
{
	int rc, sd = -1;
	struct hostent *hp;
    	struct ast_hostent ahp;
	struct sockaddr_in servAddr, *addr, cliAddr;
	
	if(sockaddr) {
		addr = sockaddr;
	} else {
		addr = &servAddr;
	}
	
	if ((sd = socket(AF_INET, SOCK_DGRAM, 0)) > -1) {
	
		if ((hp = ast_gethostbyname(ip, &ahp))) {
			
			addr->sin_family = hp->h_addrtype;
			memcpy((char *) &addr->sin_addr.s_addr, 
				hp->h_addr_list[0], hp->h_length);
			addr->sin_port = htons(port);
			if (client) {
				cliAddr.sin_family = AF_INET;
				cliAddr.sin_addr.s_addr = htonl(INADDR_ANY);
				cliAddr.sin_port = htons(0);
  				rc = bind(sd, (struct sockaddr *) &cliAddr, sizeof(cliAddr));
			} else {
				rc = bind(sd, (struct sockaddr *) addr, sizeof(cliAddr));
			}
			if (rc < 0) {
					
				if (globals.debug > 2) {
					ast_log(LOG_ERROR,
						"Error opening udp socket  %s/%i %s\n",
						ip,port, strerror(errno));
				}
				
				woomera_close_socket(&sd);
				
			} else if (globals.debug > 2) {
	
				ast_log(LOG_NOTICE, "Socket Binded %s to %s/%d\n", 
					client ? "client" : "server", ip, port);
			}
			
		} else {
			if (globals.debug > 2) {
				ast_log(LOG_ERROR,
					"Error opening udp: gethostbyname failed  %s/%i %s\n",
					ip,port, strerror(errno));
			}
					
			woomera_close_socket(&sd);
		}
	}

	return sd;
}


static int connect_woomera(int *new_socket, woomera_profile *profile, int flags) 
{
	struct sockaddr_in localAddr, remoteAddr;
	struct hostent *hp;
	struct ast_hostent ahp;
	int res = 0;

	*new_socket=-1;

	if ((hp = ast_gethostbyname(profile->woomera_host, &ahp))) {
		remoteAddr.sin_family = hp->h_addrtype;
		memcpy((char *) &remoteAddr.sin_addr.s_addr, hp->h_addr_list[0], hp->h_length);
		remoteAddr.sin_port = htons(profile->woomera_port);
		do {
			/* create socket */
			*new_socket = socket(AF_INET, SOCK_STREAM, 0);
			if (*new_socket < 0) {
				ast_log(LOG_ERROR, "cannot open socket to %s/%d\n", profile->woomera_host, profile->woomera_port);
				res = 0;
				break;
			}
			
			/* bind any port number */
			localAddr.sin_family = AF_INET;
			localAddr.sin_addr.s_addr = htonl(INADDR_ANY);
			localAddr.sin_port = htons(0);
  
			res = bind(*new_socket, (struct sockaddr *) &localAddr, sizeof(localAddr));
			if (res < 0) {
				ast_log(LOG_ERROR, "cannot bind to %s/%d\n", profile->woomera_host, profile->woomera_port);
				woomera_close_socket(new_socket);
				res = 0;
				break;
			}
		
			/* connect to server */
			res = connect(*new_socket, (struct sockaddr *) &remoteAddr, sizeof(remoteAddr));
			if (res < 0) {
				ast_log(LOG_ERROR, "cannot connect to {%s} %s/%d\n", profile->name, profile->woomera_host, profile->woomera_port);
				res = 0;
				woomera_close_socket(new_socket);
				break;
			}
			res = 1;
		} while(0);
		
	} else {
		if (globals.debug > 2) {
			ast_log(LOG_ERROR, "gethost failed connect to {%s} %s/%d\n", 
				profile->name, profile->woomera_host, profile->woomera_port);
		}
		res = 0;
	}
	if (res > 0) {
		int flag = 1;
		woomera_message wmsg;
		memset(&wmsg,0,sizeof(wmsg));

		/* disable nagle's algorythm */
		res = setsockopt(*new_socket, IPPROTO_TCP, TCP_NODELAY, (char *) &flag, sizeof(int));

		
		if (!(flags & WCFLAG_NOWAIT)) {
			/* kickstart the session waiting for a HELLO */
			woomera_printf(NULL, *new_socket, "%s", WOOMERA_RECORD_SEPARATOR);
			
			if ((res = woomera_message_parse(*new_socket,
							 &wmsg,
						 	 WOOMERA_HARD_TIMEOUT,
							 profile,
							 NULL
							 )) < 0) {
				ast_log(LOG_ERROR, "{%s} Timed out waiting for a hello from woomera!\n", profile->name);
				woomera_close_socket(new_socket);
			}
			
			if (res > 0 && strcasecmp(wmsg.command, "HELLO")) {
				ast_log(LOG_ERROR, "{%s} unexpected reply [%s] while waiting for a hello from woomera!\n", profile->name, wmsg.command);
				woomera_close_socket(new_socket);
				
			}else{
				char *audio_format;
				if (globals.debug > 2) {
				ast_log(LOG_NOTICE, "Woomera Got HELLO on connect! %s SMG Version %s\n", profile->name,woomera_message_header(&wmsg, "Version"));
				}
				if (!smgversion_init &&  woomera_message_header(&wmsg, "Version")) {
					smgversion_init=1;
					strncpy(smgversion,
						woomera_message_header(&wmsg, "Version"),
						sizeof(smgversion)-1);
				}
				audio_format = woomera_message_header(&wmsg, "Raw-Format");
				if (audio_format) {
					
					profile->coding=AST_FORMAT_SLINEAR;
				
					if (strncmp(audio_format,"PMC-16",15) == 0){
						profile->coding=AST_FORMAT_SLINEAR;
					} else if (strncmp(audio_format,"ULAW",15) == 0) {
						profile->coding=AST_FORMAT_ULAW;
					} else if (strncmp(audio_format,"ALAW",15) == 0) {
						profile->coding=AST_FORMAT_ALAW;
					} else {
						ast_log(LOG_ERROR, "Error: Invalid Raw-Format %s\n",
							audio_format);
					}
					
					default_profile.coding=profile->coding;
					
					if (globals.debug > 2) {
					ast_log(LOG_NOTICE, "Setting RAW Format to %s %i\n",
							audio_format, default_profile.coding);
					}
				}
			}
		}

	} else {
		if (globals.debug > 2) {
			ast_log(LOG_ERROR, "woomera connection failed connect to {%s} %s/%d\n", 
				profile->name, profile->woomera_host, profile->woomera_port);
		}
		woomera_close_socket(new_socket);
	}

	return *new_socket;
}

static int init_woomera(void) 
{
	woomera_profile *profile;
	ast_mutex_lock(&lock);
	
	if (!config_woomera()) {
		ast_mutex_unlock(&lock);
		return 0;
	}
	
	ASTOBJ_CONTAINER_TRAVERSE(&woomera_profile_list, 1, do {
		ASTOBJ_RDLOCK(iterator);
		profile = iterator;
		if (!ast_test_flag(profile, PFLAG_DISABLED)) {
			launch_woomera_thread(profile);
		}
		ASTOBJ_UNLOCK(iterator);
	} while(0));

	ast_mutex_unlock(&lock);
	return 1;
}

static struct ast_channel *woomera_new(const char *type, int format, void *data, int *cause)
{
	private_object *tech_pvt;
	struct ast_channel *chan = NULL;
	woomera_profile *profile=NULL;;
	
	if ((chan = ast_channel_alloc(1))) {
		chan->nativeformats = WFORMAT;
		chan->type = type;
		snprintf(chan->name, sizeof(chan->name), "%s/%s-%04x", chan->type, (char *)data, rand() & 0xffff);
		chan->writeformat = chan->rawwriteformat = chan->readformat = WFORMAT;
		chan->_state = AST_STATE_DOWN;
		chan->_softhangup = 0;
		
		if (!(tech_pvt = malloc(sizeof(private_object)))) {
			ast_log(LOG_ERROR, "Memory Error!\n");
			ast_hangup(chan);
			return NULL;
		}
		tech_count++;
		tech_pvt->coding=WFORMAT;
		memset(tech_pvt, 0, sizeof(private_object));
		ast_mutex_init(&tech_pvt->iolock);
		ast_mutex_init(&tech_pvt->event_queue.lock);
		tech_pvt->command_channel = -1;
		chan->tech_pvt = tech_pvt;
		chan->tech = &technology;
		tech_pvt->udp_socket = -1;

		ast_clear_flag(chan, AST_FLAGS_ALL);
		memset(&tech_pvt->frame, 0, sizeof(tech_pvt->frame));
		tech_pvt->frame.frametype = AST_FRAME_VOICE;
		tech_pvt->frame.subclass = WFORMAT;
		tech_pvt->frame.offset = AST_FRIENDLY_OFFSET;

		tech_pvt->owner = chan;

		
		profile = ASTOBJ_CONTAINER_FIND(&woomera_profile_list, "default");
		
		if (profile) {
			tech_pvt->coding = profile->coding;
			chan->nativeformats = profile->coding;
			chan->writeformat = chan->rawwriteformat = chan->readformat = profile->coding;
			tech_pvt->frame.subclass = profile->coding;	
		}
		
		if (profile && profile->dtmf_enable) {
			
			
			
			tech_pvt->dsp_features=0;
			tech_pvt->dsp = ast_dsp_new();
			if (tech_pvt->dsp) {
#if 0
				i->dsp_features = features & ~DSP_PROGRESS_TALK;
	
				/* We cannot do progress detection until receives PROGRESS message */
				if (i->outgoing && (i->sig == SIG_PRI)) {
					/* Remember requested DSP features, don't treat
					talking as ANSWER */
					features = 0;
				}
#endif
				tech_pvt->dsp_features |= DSP_FEATURE_DTMF_DETECT;
				//tech_pvt->dsp_features |= DSP_FEATURE_BUSY_DETECT;
				//tech_pvt->dsp_features |= DSP_FEATURE_CALL_PROGRESS;
				//tech_pvt->dsp_features |= DSP_FEATURE_FAX_DETECT;
				ast_dsp_set_features(tech_pvt->dsp, tech_pvt->dsp_features);
				ast_dsp_digitmode(tech_pvt->dsp, DSP_DIGITMODE_DTMF | DSP_DIGITMODE_RELAXDTMF);
				tech_pvt->ast_dsp=1;
				
#if 0
				if (!ast_strlen_zero(progzone))
					ast_dsp_set_call_progress_zone(tech_pvt->dsp, progzone);
				if (i->busydetect && CANBUSYDETECT(i)) {
					ast_dsp_set_busy_count(tech_pvt->dsp, i->busycount);
					ast_dsp_set_busy_pattern(tech_pvt->dsp, i->busy_tonelength, i->busy_quietlength);
				}             	
#endif
			}
		}
		if (profile && profile->faxdetect) {
			tech_pvt->faxdetect=1;
		}
		if (profile && profile->jb_enable) {
#ifdef AST_JB
			/* Assign default jb conf to the new zt_pvt */
	                memcpy(&tech_pvt->jbconf, &global_jbconf, sizeof(struct ast_jb_conf));
			ast_jb_configure(chan, &tech_pvt->jbconf);
#else
			ast_log(LOG_ERROR, "Asterisk Jitter Buffer Not Compiled!\n");
#endif
		}
		ASTOBJ_CONTAINER_LINK(&private_object_list, tech_pvt);
		
		ast_mutex_lock(&usecnt_lock);
		usecnt++;
		ast_mutex_unlock(&usecnt_lock);

	} else {
		if (option_verbose > 1) {
			ast_log(LOG_ERROR, "Can't allocate a channel\n");
		}
	}
	

	return chan;
}




/********************CHANNEL METHOD LIBRARY********************
 * This is the actual functions described by the prototypes above.
 *
 */


/*--- tech_requester: parse 'data' a url-like destination string, allocate a channel and a private structure
 * and return the newly-setup channel.
 */
static struct ast_channel *tech_requester(const char *type, int format, void *data, int *cause)
{
	struct ast_channel *chan = NULL;
			

	if (globals.panic) {
		return NULL;
	}

	if ((chan = woomera_new(type, format, data, cause))) {
		private_object *tech_pvt;
	
		tech_pvt = chan->tech_pvt;

		if (tech_pvt->owner) {
			tech_pvt->owner->hangupcause = AST_CAUSE_NORMAL_CLEARING;
		}

		ast_set_flag(tech_pvt, TFLAG_PBX); /* so we know we dont have to free the channel ourselves */
	
		if (globals.debug > 1 && option_verbose > 1) {
			if (option_verbose > 2) {
				ast_verbose(WOOMERA_DEBUG_PREFIX "+++REQ %s\n", chan->name);
			}
		}

	} else {

		if (option_verbose > 1) {
			ast_log(LOG_ERROR, "Can't allocate a channel\n");
		}
	}


	return chan;
}

/*--- tech_senddigit: Send a DTMF character */
static int tech_send_digit(struct ast_channel *self, char digit)
{
	private_object *tech_pvt = self->tech_pvt;
	int res = 0;

	if (globals.debug > 1 && option_verbose > 1) {
		if (option_verbose > 2) {
			ast_verbose(WOOMERA_DEBUG_PREFIX "+++DIGIT %s '%c'\n",self->name, digit);
		}
	}

	/* we don't have time to make sure the dtmf command is successful cos asterisk again 
	   is much too impaitent... so we will cache the digits so the monitor thread can send
	   it for us when it has time to actually wait.
	*/
	ast_mutex_lock(&tech_pvt->iolock);
	snprintf(tech_pvt->dtmfbuf + strlen(tech_pvt->dtmfbuf), sizeof(tech_pvt->dtmfbuf), "%c", digit);
	ast_set_flag(tech_pvt, TFLAG_DTMF);
	ast_mutex_unlock(&tech_pvt->iolock);

	return res;
}

/*--- tech_call: Initiate a call on my channel 
 * 'dest' has been passed telling you where to call
 * but you may already have that information from the requester method
 * not sure why it sends it twice, maybe it changed since then *shrug*
 * You also have timeout (in ms) so you can tell how long the caller
 * is willing to wait for the call to be complete.
 */

static int tech_call(struct ast_channel *self, char *dest, int timeout)
{
	private_object *tech_pvt = self->tech_pvt;
	char *workspace;

 	self->hangupcause = AST_CAUSE_NORMAL_CIRCUIT_CONGESTION;

	if (globals.panic) {
		goto tech_call_failed;
	}
	if (globals.debug > 2) {
		ast_log(LOG_NOTICE, "TECH CALL %s (%s <%s>) (%p)\n",
			self->name, self->cid.cid_name, 
			self->cid.cid_num,
			tech_pvt);
	}
	if (self->cid.cid_name) {
		strncpy(tech_pvt->cid_name, self->cid.cid_name, sizeof(tech_pvt->cid_name)-1);
	}
	if (self->cid.cid_num) {
		strncpy(tech_pvt->cid_num, self->cid.cid_num, sizeof(tech_pvt->cid_num)-1);
	}
	if ((workspace = ast_strdupa(dest))) {
		char *addr, *profile_name, *proto;
		woomera_profile *profile;
		int isprofile = 0, err;

		if ((addr = strchr(workspace, ':'))) {
			proto = workspace;
			*addr = '\0';
			addr++;
		} else {
			proto = "H323";
			addr = workspace;
		}
		
		if ((profile_name = strchr(addr, '*'))) {
			*profile_name = '\0';
			profile_name++;
			isprofile = 1;
		} else {
			profile_name = "default";
		}
		if (! (profile = ASTOBJ_CONTAINER_FIND(&woomera_profile_list, profile_name))) {
			profile = ASTOBJ_CONTAINER_FIND(&woomera_profile_list, "default");
		}
		
		if (!profile) {
			ast_log(LOG_ERROR, "Unable to find profile! Call Aborted!\n");
			goto tech_call_failed;
		}

		if (!ast_test_flag(profile, PFLAG_OUTBOUND)) {
			ast_log(LOG_ERROR, "This profile is not allowed to make outbound calls! Call Aborted!\n");
			goto tech_call_failed;
		}

		if (profile->max_calls) {
			ast_mutex_lock(&profile->call_count_lock);
			if (profile->call_count >= profile->max_calls) {
				ast_mutex_unlock(&profile->call_count_lock);
				if (globals.debug > 1 && option_verbose > 1) {
					ast_log(LOG_ERROR, "This profile is at call limit of %d\n",
						 profile->max_calls);
				}
				goto tech_call_failed;
			} 
			ast_mutex_unlock(&profile->call_count_lock);
		}

		snprintf(tech_pvt->dest, sizeof(tech_pvt->dest), "%s", addr ? addr : "");

		tech_pvt->timeout = timeout;
		err=tech_init(tech_pvt, profile, TFLAG_OUTBOUND);
		if (err) {	
			if (globals.debug > 2) {
				ast_log(LOG_ERROR, "Error: Outbound Call Failed \n");
			}
			goto tech_call_failed;
		}
		
		woomera_send_progress(tech_pvt);
	}
	self->hangupcause = AST_CAUSE_NORMAL_CLEARING;


	return 0;

tech_call_failed:
	if (globals.debug > 1 && option_verbose > 1) {
		ast_log(LOG_ERROR, "Error: Outbound Call Failed %p \n",tech_pvt);
	}
 	self->hangupcause = AST_CAUSE_NORMAL_CIRCUIT_CONGESTION;
	my_ast_softhangup(self, tech_pvt, AST_SOFTHANGUP_EXPLICIT);
	return -1;
}


/*--- tech_hangup: end a call on my channel 
 * Now is your chance to tear down and free the private object
 * from the channel it's about to be freed so you must do so now
 * or the object is lost.  Well I guess you could tag it for reuse
 * or for destruction and let a monitor thread deal with it too.
 * during the load_module routine you have every right to start up
 * your own fancy schmancy bunch of threads or whatever else 
 * you want to do.
 */
static int tech_hangup(struct ast_channel *self)
{
	const char *ds;
	private_object *tech_pvt = self->tech_pvt;
	int res = 0;

	if (globals.debug > 2) {
		ast_log(LOG_NOTICE, "TECH HANGUP [%s] tech_pvt=%p c=%p\n",self->name,tech_pvt,self);
	}


	if (tech_pvt) {

		ast_mutex_lock(&tech_pvt->iolock);
		ast_set_flag(tech_pvt, TFLAG_TECHHANGUP);
		tech_pvt->owner=NULL;
		ast_mutex_unlock(&tech_pvt->iolock);


		if (globals.debug > 2) {
			ast_log(LOG_NOTICE, "TECH HANGUP ON TECH [%s] %p\n",
				self->name,tech_pvt);
		}
		

		if (!self || 
		   (!(ds = pbx_builtin_getvar_helper(self, "DIALSTATUS")) && 
		    !(ds = ast_cause2str(self->hangupcause)))) {
			ds = "NOEXIST";
		}

		ast_copy_string(tech_pvt->ds, ds, sizeof(tech_pvt->ds));

		 if (tech_pvt->dsp) {
	                tech_pvt->dsp_features &= ~DSP_FEATURE_DTMF_DETECT;
        	        ast_dsp_set_features(tech_pvt->dsp, tech_pvt->dsp_features);
                	tech_pvt->ast_dsp=0;
        	}
		

		if (ast_test_flag(tech_pvt, TFLAG_INTHREAD)) {
			ast_set_flag(tech_pvt, TFLAG_ABORT);
			ast_set_flag(tech_pvt, TFLAG_DESTROY);

			if (globals.debug > 2) {
				ast_log(LOG_NOTICE, "TECH HANGUP IN THREAD! tpvt=%p\n",
						tech_pvt);
			}
		} else {
			if (!ast_test_flag(tech_pvt, TFLAG_DESTROY)) {
				tech_destroy(tech_pvt,NULL);
				if (globals.debug > 2) {
					ast_log(LOG_NOTICE, "TECH HANGUP NOT IN THREAD! tpvt=%p\n",
						tech_pvt);
				}
			}else{
				if (globals.debug > 2) {
					ast_log(LOG_NOTICE, "TECH HANGUP NOT IN THREAD ALREDAY HUNGUP! tpvt=%p\n",
						tech_pvt);
				}
			}
		}
	} else {
		if (globals.debug > 2) {
			ast_log(LOG_NOTICE, "ERROR: NO TECH ON TECH HANGUP!\n");
		}
	}

	self->tech_pvt = NULL;
	
	return res;
}

/*--- tech_answer: answer a call on my channel
 * if being 'answered' means anything special to your channel
 * now is your chance to do it!
 */
static int tech_answer(struct ast_channel *self)
{
	private_object *tech_pvt;
	int res = 0;

	tech_pvt = self->tech_pvt;
	if (globals.debug > 1 && option_verbose > 1) {
		if (option_verbose > 2) {
			ast_verbose(WOOMERA_DEBUG_PREFIX "+++ANSWER %s\n",self->name);
		}
	}
	
	if (!ast_test_flag(tech_pvt, TFLAG_OUTBOUND)) {
		/* Only answer the inbound calls */
		ast_set_flag(tech_pvt, TFLAG_ANSWER);
	} else {
		ast_log(LOG_ERROR, "Warning: AST trying to Answer OUTBOUND Call!\n");
	}

	ast_set_flag(tech_pvt, TFLAG_UP);
	ast_setstate(self, AST_STATE_UP);
	
	return res;
}


static int woomera_tx2ast_frm(private_object *tech_pvt, char * buf,  int len )
{
	struct ast_frame frame;
	
	frame.frametype  = AST_FRAME_VOICE;
	frame.subclass = tech_pvt->coding;
	frame.datalen = len;
	frame.samples = len;
	frame.mallocd =0 ;
	frame.offset= 0 ;
	frame.src = NULL;
	frame.data = buf ;
	
	if (tech_pvt->faxdetect || tech_pvt->ast_dsp) {
		struct ast_frame *f;
		struct ast_channel *owner = tech_get_owner(tech_pvt);

		
		if (!owner) {
			return 0;
		}

		f = ast_dsp_process(owner, tech_pvt->dsp, &frame);
		if (f && (f->frametype == AST_FRAME_DTMF)) {
		
			ast_log(LOG_DEBUG, "Detected inband DTMF digit: %c\n", f->subclass);
#if 0
			if (f->subclass == 'f' && tech_pvt->faxdetect) {
				/* Fax tone -- Handle and return NULL */
				struct ast_channel *ast = tech_pvt->owner;
				if (!tech_pvt->faxhandled) {
					tech_pvt->faxhandled++;
					if (strcmp(ast->exten, "fax")) {
						if (ast_exists_extension(ast, ast_strlen_zero(ast->macrocontext)? ast->context : ast->macrocontext, "fax", 1, AST_CID_P(ast))) {
							if (option_verbose > 2)
								ast_verbose(VERBOSE_PREFIX_3 "Redirecting %s to fax extension\n", ast->name);
							/* Save the DID/DNIS when we transfer the fax call to a "fax" extension */
							pbx_builtin_setvar_helper(ast,"FAXEXTEN",ast->exten);
							if (ast_async_goto(ast, ast->context, "fax", 1))
								ast_log(LOG_WARNING, "Failed to async goto '%s' into fax of '%s'\n", ast->name, ast->context);
						} else
							ast_log(LOG_NOTICE, "Fax detected, but no fax extension ctx:%s exten:%s\n",ast->context, ast->exten);
					} else
						ast_log(LOG_DEBUG, "Already in a fax extension, not redirecting\n");
				} else
					ast_log(LOG_DEBUG, "Fax already handled\n");
				frame.frametype = AST_FRAME_NULL;
				frame.subclass = 0;
				f = &frame;

			}  else 
#endif			

			if (tech_pvt->ast_dsp && owner) {
				struct ast_frame fr;
				memset(&fr, 0 , sizeof(fr));
				fr.frametype = AST_FRAME_DTMF;
				fr.subclass = f->subclass ;
				fr.src=NULL;
				fr.data = NULL ;
				fr.datalen = 0;
				fr.samples = 0 ;
				fr.mallocd =0 ;
				fr.offset= 0 ;
				
				ast_log(LOG_EVENT, "Received DTMF Sending 2 AST %c\n",
						fr.subclass);
				ast_queue_frame(owner, &fr);

			}

		}

	}
	
	return 0;
}                  


/*--- tech_read: Read an audio frame from my channel.
 * You need to read data from your channel and convert/transfer the
 * data into a newly allocated struct ast_frame object
 */
static struct ast_frame  *tech_read(struct ast_channel *self)
{
	private_object *tech_pvt = self->tech_pvt;
	int res = 0;


	if (!tech_pvt || globals.panic || ast_test_flag(tech_pvt, TFLAG_ABORT)) {
		return NULL;
	}

	res = waitfor_socket(tech_pvt->udp_socket, 100);

	if (res < 1) {
		return NULL;
	}

	res = read(tech_pvt->udp_socket, tech_pvt->fdata + AST_FRIENDLY_OFFSET, FRAME_LEN);

	if (res < 1) {
		return NULL;
	}

	tech_pvt->frame.datalen = res;
	if (tech_pvt->coding == AST_FORMAT_SLINEAR) {
		tech_pvt->frame.samples = res / 2;
	} else {
		tech_pvt->frame.samples = res;
	}
	tech_pvt->frame.data = tech_pvt->fdata + AST_FRIENDLY_OFFSET;

	if (default_profile.rxgain_val) {
		int i;
		unsigned char *data=tech_pvt->frame.data;
		for (i=0;i<tech_pvt->frame.datalen;i++) {
			data[i]=default_profile.rxgain[data[i]];
		}
	} 	
	
	if (tech_pvt->faxdetect || tech_pvt->ast_dsp ) {
		woomera_tx2ast_frm(tech_pvt, tech_pvt->frame.data, tech_pvt->frame.datalen);
	}  
	
	
	if (globals.debug > 4) {
		if (option_verbose > 2) {
			ast_verbose(WOOMERA_DEBUG_PREFIX "+++READ %s %d\n",self->name, res);
		}
	}

	return &tech_pvt->frame;
}

/*--- tech_write: Write an audio frame to my channel
 * Yep, this is the opposite of tech_read, you need to examine
 * a frame and transfer the data to your technology's audio stream.
 * You do not have any responsibility to destroy this frame and you should
 * consider it to be read-only.
 */
static int tech_write(struct ast_channel *self, struct ast_frame *frame)
{
	private_object *tech_pvt = self->tech_pvt;
	int res = 0, i = 0;
	
	if (!tech_pvt || globals.panic || ast_test_flag(tech_pvt, TFLAG_ABORT)) {
		return -1;
	}

	if(ast_test_flag(tech_pvt, TFLAG_MEDIA) && frame->datalen) {
		if (frame->frametype == AST_FRAME_VOICE) {
		
			if (default_profile.txgain_val) {
				unsigned char *data=frame->data;
				for (i=0;i<frame->datalen;i++) {
					data[i]=default_profile.txgain[data[i]];
				}
			} 
		
			i = sendto(tech_pvt->udp_socket, frame->data, frame->datalen, 0, (struct sockaddr *) &tech_pvt->udpwrite, sizeof(tech_pvt->udpwrite));
			if (i < 0) {
				return -1;
			}
			if (globals.debug > 4) {
				if (option_verbose > 4) {
					ast_verbose(WOOMERA_DEBUG_PREFIX "+++WRITE %s %d\n",self->name, i);
				}
			}
		} else {
			ast_log(LOG_WARNING, "Invalid frame type %d sent\n", frame->frametype);
		}
	}
	
	return res;
}

/*--- tech_write_video: Write a video frame to my channel ---*/
static int tech_write_video(struct ast_channel *self, struct ast_frame *frame)
{
	private_object *tech_pvt;
	int res = 0;

	tech_pvt = self->tech_pvt;
	return res;
}

/*--- tech_exception: Read an exception audio frame from my channel ---*/
static struct ast_frame *tech_exception(struct ast_channel *self)
{
	private_object *tech_pvt;
	struct ast_frame *new_frame = NULL;

	tech_pvt = self->tech_pvt;	
	if (globals.debug > 1 && option_verbose > 1) {
		if (option_verbose > 2) {
			ast_verbose(WOOMERA_DEBUG_PREFIX "+++EXCEPT %s\n",self->name);
		}
	}
	return new_frame;
}

/*--- tech_indicate: Indicaate a condition to my channel ---*/
#if 0
static int tech_indicate(struct ast_channel *self, int condition)
{
	private_object *tech_pvt;
	int res = 0;

	tech_pvt = self->tech_pvt;
	if (globals.debug > 1) {
		ast_verbose(WOOMERA_DEBUG_PREFIX "+++INDICATE %s %d\n",self->name, condition);
	}

	return res;
}
#endif

/*--- tech_fixup: add any finishing touches to my channel if it is masqueraded---*/
static int tech_fixup(struct ast_channel *oldchan, struct ast_channel *newchan)
{
	int res = 0;
	private_object *tech_pvt;

	if ((tech_pvt = oldchan->tech_pvt)) {
		ast_mutex_lock(&tech_pvt->iolock);
		tech_pvt->owner = newchan;
		ast_mutex_unlock(&tech_pvt->iolock);
	}

	if (globals.debug > 1 && option_verbose > 1) {
		if (option_verbose > 2) {
			ast_verbose(WOOMERA_DEBUG_PREFIX "+++FIXUP %s\n",oldchan->name);
		}
	}
	return res;
}

/*--- tech_send_html: Send html data on my channel ---*/
static int tech_send_html(struct ast_channel *self, int subclass, const char *data, int datalen)
{
	private_object *tech_pvt;
	int res = 0;

	tech_pvt = self->tech_pvt;


	return res;
}

/*--- tech_send_text: Send plain text data on my channel ---*/
static int tech_send_text(struct ast_channel *self, const char *text)
{
	int res = 0;

	return res;
}

/*--- tech_send_image: Send image data on my channel ---*/
static int tech_send_image(struct ast_channel *self, struct ast_frame *frame) 
{
	int res = 0;

	return res;
}


/*--- tech_setoption: set options on my channel ---*/
static int tech_setoption(struct ast_channel *self, int option, void *data, int datalen)
{
	int res = 0;

	if (globals.debug > 1 && option_verbose > 1) {
		if (option_verbose > 2) {
			ast_verbose(WOOMERA_DEBUG_PREFIX "+++SETOPT %s\n",self->name);
		}
	}
	return res;

}

/*--- tech_queryoption: get options from my channel ---*/
static int tech_queryoption(struct ast_channel *self, int option, void *data, int *datalen)
{
	int res = 0;

	if (globals.debug > 1 && option_verbose > 1) {
		if (option_verbose > 2) {
			ast_verbose(WOOMERA_DEBUG_PREFIX "+++GETOPT %s\n",self->name);
		}
	}
	return res;
}

/*--- tech_bridged_channel: return a pointer to a channel that may be bridged to our channel. ---*/
#if 0
static struct ast_channel *tech_bridged_channel(struct ast_channel *self, struct ast_channel *bridge)
{
	struct ast_channel *chan = NULL;

	if (globals.debug > 1 && option_verbose > 1) {
		ast_verbose(WOOMERA_DEBUG_PREFIX "+++BRIDGED %s\n",self->name);
	}
	return chan;
}
#endif


/*--- tech_transfer: Technology-specific code executed to peform a transfer. ---*/
static int tech_transfer(struct ast_channel *self, const char *newdest)
{
	int res = -1;

	if (globals.debug > 1 && option_verbose > 1) {
		if (option_verbose > 2) {
			ast_verbose(WOOMERA_DEBUG_PREFIX "+++TRANSFER %s\n",self->name);
		}
	}
	return res;
}

/*--- tech_bridge:  Technology-specific code executed to natively bridge 2 of our channels ---*/
#if 0
static enum ast_bridge_result tech_bridge(struct ast_channel *c0, struct ast_channel *c1, int flags, struct ast_frame **fo, struct ast_channel **rc, int timeoutms)
{
	int res = -1;

	if (globals.debug > 1) {
		ast_verbose(WOOMERA_DEBUG_PREFIX "+++BRIDGE %s\n",c0->name);
	}
	return res;
}
#endif


static int woomera_cli(int fd, int argc, char *argv[]) 
{
	if (argc > 1) {
	
		if (!strcmp(argv[1], "debug")) {
			if (argc > 2) {
				globals.debug = atoi(argv[2]);
			}
			ast_cli(fd, "Woomera debug=%d           \n", globals.debug);

		} else if (!strcmp(argv[1], "version")) { 

			ast_cli(fd, "Woomera version %s : SMG Version %s  \n",
				WOOMERA_VERSION,smgversion);

		} else if (!strcmp(argv[1], "panic")) {
			if (argc > 2) {
				globals.panic = atoi(argv[2]);
			}
			ast_cli(fd, "Woomera panic=%d           \n", globals.panic);
			
		} else if (!strcmp(argv[1], "rxgain")) {
			float gain;
			if (argc > 2) {
				if (sscanf(argv[2], "%f", &gain) != 1) {
					ast_cli(fd, "Woomera Invalid rxgain: %s\n",argv[2]);
				} else {
					woomera_config_gain(&default_profile,gain,1);
				}	
			}
			ast_cli(fd, "Woomera rxgain: %f\n",default_profile.rxgain_val);
			
		} else if (!strcmp(argv[1], "txgain")) {
			float gain;
			if (argc > 2) {
				if (sscanf(argv[2], "%f", &gain) != 1) {
					ast_cli(fd, "Woomera Invalid txgain: %s\n",argv[2]);
				} else {
					woomera_config_gain(&default_profile,gain,0);
				}	
			} 	
			ast_cli(fd, "Woomera txgain: %f\n",default_profile.txgain_val);
			
		} else if (!strcmp(argv[1], "threads")) {
			ast_cli(fd, "chan_woomera is using %s threads!\n", globals.more_threads ? "more" : "less");
		
		} else if (!strcmp(argv[1], "abort")) {
			global_set_flag(TFLAG_ABORT);
		}

	} else {
		ast_cli(fd, "Usage: woomera <debug> <level>\n");
	}
	return 0;
}

static struct ast_cli_entry  cli_woomera = { { "woomera", NULL }, woomera_cli, "Woomera", "Woomera" };

/******************************* CORE INTERFACE ********************************************
 * These are module-specific interface functions that are common to every module
 * To be used to initilize/de-initilize, reload and track the use count of a loadable module. 
 */

static void urg_handler(int num)
{
	signal(num, urg_handler);
        return;
}


int load_module()
{
	int x;

	if (ast_channel_register(&technology)) {
		ast_log(LOG_ERROR, "Unable to register channel class %s\n", type);
		return -1;
	}
	memset(&globals, 0, sizeof(globals));
	globals.next_woomera_port = WOOMERA_MIN_PORT;
	/* Use more threads for better timing this adds a dedicated monitor thread to
	   every channel you can disable it with more_threads => no in [settings] */
	globals.more_threads = 1;

	ast_mutex_init(&globals.woomera_port_lock);
	ast_mutex_init(&default_profile.iolock);
	ast_mutex_init(&default_profile.call_count_lock);
	ast_mutex_init(&default_profile.event_queue.lock);

	memset(tech_pvt_idx, 0, sizeof(tech_pvt_idx));

	for (x=0;x<WOOMERA_MAX_CALLS;x++){
		ast_mutex_init(&tech_pvt_idx_lock[x]);
	}
	
	if (!init_woomera()) {
		return -1;
	}

	signal(SIGURG, urg_handler);

	ASTOBJ_CONTAINER_INIT(&private_object_list);
	/*
	sched = sched_context_create();
    if (!sched) {
        ast_log(LOG_WARNING, "Unable to create schedule context\n");
    }
	*/
	
	ast_cli_register(&cli_woomera);
	return 0;
}

int reload()
{
	return 0;
}

int unload_module()
{
	time_t then, now;
	woomera_profile *profile = NULL;
	int x;

	globals.panic=1;
	ast_log(LOG_NOTICE, "WOOMERA Unload %i\n",
			usecount());
	sleep(1);
	
	ASTOBJ_CONTAINER_TRAVERSE(&woomera_profile_list, 1, do {
		ASTOBJ_RDLOCK(iterator);
		profile = iterator;

		time(&then);
		if (!ast_test_flag(profile, PFLAG_DISABLED)) {
			ast_log(LOG_NOTICE, "Shutting Down Thread. {%s}\n", profile->name);
			woomera_profile_thread_running(profile, 1, 0);
			
			while (!woomera_profile_thread_running(profile, 0, 0)) {
				time(&now);
				if (now - then > 30) {
					ast_log(LOG_WARNING, "Timed out waiting for thread to exit\n");
					break;
				}
				usleep(100);
			}
		}
		ASTOBJ_UNLOCK(iterator);
	} while(0));

	ast_mutex_destroy(&default_profile.iolock);
	ast_mutex_destroy(&default_profile.call_count_lock);
	ast_mutex_destroy(&default_profile.event_queue.lock);
	ast_mutex_destroy(&globals.woomera_port_lock);

	for (x=0;x<WOOMERA_MAX_CALLS;x++){
		ast_mutex_destroy(&tech_pvt_idx_lock[x]);
	}
	
	ast_cli_unregister(&cli_woomera);
	ASTOBJ_CONTAINER_DESTROY(&private_object_list);
	ASTOBJ_CONTAINER_DESTROYALL(&woomera_profile_list, destroy_woomera_profile);
	//sched_context_destroy(sched);

	ast_channel_unregister(&technology);
	return 0;
}

int usecount()
{
	int res;
	ast_mutex_lock(&usecnt_lock);
	res = usecnt;
	ast_mutex_unlock(&usecnt_lock);
	return res;
}

char *key()
{
	return ASTERISK_GPL_KEY;
}

/* returns a descriptive string to the system console */
char *description()
{
	return (char *) desc;
}


static struct ast_channel * tech_get_owner( private_object *tech_pvt)
{
	struct ast_channel *owner=NULL;

	ast_mutex_lock(&tech_pvt->iolock);
	if (!ast_test_flag(tech_pvt, TFLAG_TECHHANGUP) && tech_pvt->owner) {
		owner=tech_pvt->owner;
	}
        ast_mutex_unlock(&tech_pvt->iolock);

	return owner;
}

;
