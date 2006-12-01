/******************************************************************************
** Copyright (c) 2005
**	Alex Feldman <al.feldman@sangoma.com>.  All rights reserved.
**
** ============================================================================
** Sep 1, 2005		Alex Feldman	Initial version.
** Sep 2, 2005		Alex Feldman	Add option 'all' for channel 
**					selection.
******************************************************************************/

/******************************************************************************
**			   INCLUDE FILES
******************************************************************************/

#if !defined(__WINDOWS__)

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <syslog.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <sys/queue.h>
#include <netinet/in.h>
#if defined(__LINUX__)
# include <linux/if.h>
# include <linux/if_packet.h>
#endif

#else

#include "windows_config.h"

wanpipe_t wanpipes[MAX_NUM_OF_WANPIPES];

char WAN_EC_DIR[MAX_PATH];
char WAN_EC_BUFFERS[MAX_PATH];
char windows_dir[MAX_PATH];

#endif//__WINDOWS__

#include "wan_ecmain.h"

/******************************************************************************
**			  DEFINES AND MACROS
******************************************************************************/

/******************************************************************************
**			STRUCTURES AND TYPEDEFS
******************************************************************************/

/******************************************************************************
**			   GLOBAL VARIABLES
******************************************************************************/

/******************************************************************************
** 			FUNCTION PROTOTYPES
******************************************************************************/
extern int wan_ec_args_parse_and_run( wanec_client_t*, int, char** );

/******************************************************************************
** 			FUNCTION DEFINITIONS
******************************************************************************/

int main(int argc, char *argv[])
{
	wanec_client_t	ec_client;

#if defined(__WINDOWS__)
	////////////////////////////////////////////////////////////////////////////
	//Get and parse command line arguments.
	//If "-f <file name> is provided, parse the file.
	////////////////////////////////////////////////////////////////////////////
	if(argc == 3 && !strcmp(argv[1], "-f")){
		return parse_cmd_line_args(argc, argv);
	}
#endif

	memset(&ec_client, 0, sizeof(wanec_client_t));
	if (wan_ec_args_parse_and_run(&ec_client, argc, argv)){
		return -EINVAL;
	}
	printf("\n\n");
	return 0;
}
