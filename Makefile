#
# Makefile	WANPIPE WAN Router Installation/Removal Makefile
#
# Copyright	(c) 2007, Sangoma Technologies Inc.
#
#		This program is free software; you can redistribute it and/or
#		modify it under the terms of the GNU General Public License
#		as published by the Free Software Foundation; either version
#		2 of the License, or (at your option) any later version.
# ----------------------------------------------------------------------------   
# Author:  Nenad Corbic <ncorbic@sangoma.com> 
#

PWD=$(shell pwd)
KBUILD_VERBOSE=0
KBUILD_MODPOST_WARN=1
CC=gcc
LDCONFIG=/sbin/ldconfig

EXTRA_CFLAGS=
EXTRA_FLAGS=

#Default zaptel directory to be overwritten by user

ifdef DAHDI_DIR
	ZAPDIR=$(DAHDI_DIR)
endif

ifndef ZAPDIR
	ZAPDIR=/usr/src/dahdi
ifneq (,$(wildcard ./.pbxdir))
	ZAPDIR=$(shell cat .pbxdir)
endif    	
endif

$(shell echo $(ZAPDIR) > .pbxdir)

#Kernel version and location
ifndef KVER
	KVER=$(shell uname -r)
endif
ifndef KMOD
	KMOD=/lib/modules/$(KVER)
endif
ifndef KDIR
	KDIR=$(KMOD)/build
endif
ifndef KSRC
	KSRC=$(KMOD)/source
endif
ifndef KINSTDIR
	KINSTDIR=$(KMOD)/kernel
endif

ifndef WARCH
	WARCH=$(shell uname -m)
endif
ifndef ASTDIR
	ASTDIR=/usr/src/asterisk
endif

ifdef DESTDIR
	INSTALLPREFIX=$(DESTDIR)
endif

ifndef INSTALLPREFIX
	INSTALLPREFIX=
endif

ifndef LIBPREFIX
	LIBPREFIX=/usr
endif

ifndef 64BIT_4G
    64BIT_4G="Disabled"
else
	EXTRA_CFLAGS+= -DWANPIPE_64BIT_4G_DMA
endif

ifndef 64BIT_2G
    64BIT_2G="Disabled"
else
	EXTRA_CFLAGS+= -DWANPIPE_64BIT_2G_DMA
endif


#Local wanpipe includes
WINCLUDE=patches/kdrivers/include
HWECINC=patches/kdrivers/wanec/oct6100_api
KMODDIR=patches/kdrivers

#Location of wanpipe source in release
WAN_DIR=$(PWD)/$(KMODDIR)/src/net
WANEC_DIR=$(PWD)/$(KMODDIR)/wanec
MODTYPE=ko

#Setup include path and extra cflags
EXTRA_CFLAGS += -I$(PWD)/$(WINCLUDE) -I$(PWD)/$(WINCLUDE)/annexg -I$(PWD)/patches/kdrivers/wanec -D__LINUX__
EXTRA_CFLAGS += -I$(WANEC_DIR) -I$(WANEC_DIR)/oct6100_api -I$(WANEC_DIR)/oct6100_api/include

#Setup utility extra flags and include path
EXTRA_UTIL_FLAGS = -I$(PWD)/$(WINCLUDE) -I$(INSTALLPREFIX)/include -I$(INSTALLPREFIX)/usr/include -L$(INSTALLPREFIX)/lib 
EXTRA_UTIL_FLAGS += -I$(PWD)/patches/kdrivers/wanec -I$(PWD)/patches/kdrivers/wanec/oct6100_api/include

ENABLE_WANPIPEMON_ZAP=NO
ZAPHDLC_PRIV=/etc/wanpipe/.zaphdlc

PRODUCT_DEFINES= -DCONFIG_PRODUCT_WANPIPE_BASE -DCONFIG_PRODUCT_WANPIPE_AFT -DCONFIG_PRODUCT_WANPIPE_AFT_CORE 
PRODUCT_DEFINES+= -DCONFIG_PRODUCT_WANPIPE_AFT_TE1 -DCONFIG_PRODUCT_WANPIPE_AFT_TE3 -DCONFIG_PRODUCT_WANPIPE_AFT_56K
PRODUCT_DEFINES+= -DCONFIG_WANPIPE_HWEC  -DCONFIG_PRODUCT_WANPIPE_SOCK_DATASCOPE -DCONFIG_PRODUCT_WANPIPE_AFT_BRI -DCONFIG_PRODUCT_WANPIPE_AFT_SERIAL 
PRODUCT_DEFINES+= -DCONFIG_PRODUCT_WANPIPE_TDM_VOICE_DCHAN -DCONFIG_PRODUCT_WANPIPE_CODEC_SLINEAR_LAW -DCONFIG_PRODUCT_WANPIPE_AFT_RM 
PRODUCT_DEFINES+= -DCONFIG_PRODUCT_WANPIPE_USB -DCONFIG_PRODUCT_WANPIPE_A700 -DCONFIG_PRODUCT_A600 -DCONFIG_PRODUCT_WANPIPE_AFT_A600 -DCONFIG_PRODUCT_WANPIPE_AFT_A700
PRODUCT_DEFINES+= -DCONFIG_PRODUCT_WANPIPE_AFT_B601 -DCONFIG_PRODUCT_WANPIPE_AFT_B800 

EXTRA_CFLAGS += $(EXTRA_FLAGS) $(PRODUCT_DEFINES)
EXTRA_UTIL_FLAGS +=  $(PRODUCT_DEFINES)  
DAHDI_CFLAGS=




#Check if zaptel exists
ifneq (,$(wildcard $(ZAPDIR)/zaptel.h))
	ZAPDIR_PRIV=$(ZAPDIR) 
	ENABLE_WANPIPEMON_ZAP=YES
	DAHDI_CFLAGS+= -DSTANDALONE_ZAPATA -DBUILDING_TONEZONE
	ZAP_OPTS= --zaptel-path=$(ZAPDIR) 
	ZAP_PROT=TDM
	PROTS=DEF-TDM
else
	ifneq (,$(wildcard $(ZAPDIR)/kernel/zaptel.h))
		ZAPDIR_PRIV=$(ZAPDIR) 
		ENABLE_WANPIPEMON_ZAP=YES
		DAHDI_CFLAGS+= -DSTANDALONE_ZAPATA -DBUILDING_TONEZONE -I$(ZAPDIR)/kernel
		ZAP_OPTS= --zaptel-path=$(ZAPDIR) 
		ZAP_PROT=TDM
		PROTS=DEF-TDM
	else
		ifneq (,$(wildcard $(ZAPDIR)/include/dahdi/version.h))
			ZAPDIR_PRIV=$(ZAPDIR) 
			ENABLE_WANPIPEMON_ZAP=YES
			DAHDI_CFLAGS+= -DSTANDALONE_ZAPATA -DCONFIG_PRODUCT_WANPIPE_TDM_VOICE_DCHAN_ZAPTEL -DDAHDI_ISSUES -DBUILDING_TONEZONE -I$(ZAPDIR)/include -I$(ZAPDIR)/include/dahdi -I$(ZAPDIR)/drivers/dahdi
			ifneq (,$(wildcard $(ZAPDIR)/drivers/dahdi/Makefile))
				ZAP_OPTS= --zaptel-path=$(ZAPDIR)/drivers/dahdi/
			endif
			ZAP_PROT=TDM
			PROTS=DEF-TDM
			DAHDI_MAJOR := $(shell cat $(ZAPDIR)/include/dahdi/version.h |  grep define | cut -d'"' -f2 | cut -d'.' -f1)
			DAHDI_MINOR := $(shell cat $(ZAPDIR)/include/dahdi/version.h |  grep define | cut -d'"' -f2 | cut -d'.' -f2)
			DAHDI_VER=-DDAHDI_$(DAHDI_MAJOR)$(DAHDI_MINOR)
			DAHDI_CFLAGS+= $(DAHDI_VER)
		else
			ZAP_OPTS=
			ZAP_PROT=
			ZAPDIR_PRIV=
			ENABLE_WANPIPEMON_ZAP=NO
			PROTS=DEF
		endif
	endif
endif  


EXTRA_CFLAGS += -I$(KDIR)/include/linux -I$(ZAPDIR) 

RM      = @rm -rf
JUNK	= *~ *.bak DEADJOE

#Check for PDE_DATA kernel feature
ifneq (,$(wildcard $(KDIR)/include/linux/proc_fs.h))
KERN_PROC_PDE_FEATURE=$(shell grep PDE_DATA $(KDIR)/include/linux/proc_fs.h -c -i)
EXTRA_CFLAGS+=-DKERN_PROC_PDE_FEATURE=$(KERN_PROC_PDE_FEATURE)
else
KERN_PROC_PDE_FEATURE=$(shell grep PDE_DATA $(KSRC)/include/linux/proc_fs.h -c -i)
EXTRA_CFLAGS+=-DKERN_PROC_PDE_FEATURE=$(KERN_PROC_PDE_FEATURE)
endif

ifneq (,$(wildcard $(KDIR)/include/linux/skbuff.h))
KERN_MEMCPY_FROM_MSG=$(shell grep "memcpy_from_msg" $(KDIR)/include/linux/skbuff.h -c)
EXTRA_CFLAGS+=-DKERN_MEMCPY_FROM_MSG=$(KERN_MEMCPY_FROM_MSG)
else
KERN_MEMCPY_FROM_MSG=$(shell grep "memcpy_from_msg" $(KSRC)/include/linux/skbuff.h -c)
EXTRA_CFLAGS+=-DKERN_MEMCPY_FROM_MSG=$(KERN_MEMCPY_FROM_MSG)
endif

ifneq (,$(wildcard $(KDIR)/include/linux/skbuff.h))
KERN_MEMCPY_TO_MSG=$(shell grep "memcpy_to_msg" $(KDIR)/include/linux/skbuff.h -c)
EXTRA_CFLAGS+=-DKERN_MEMCPY_TO_MSG=$(KERN_MEMCPY_TO_MSG)
else
KERN_MEMCPY_TO_MSG=$(shell grep "memcpy_to_msg" $(KSRC)/include/linux/skbuff.h -c)
EXTRA_CFLAGS+=-DKERN_MEMCPY_TO_MSG=$(KERN_MEMCPY_TO_MSG)
endif

ifneq (,$(wildcard $(KDIR)/include/net/sock.h))
KERN_SK_FOR_NODE_FEATURE=$(shell grep "sk_for_each.*node" $(KDIR)/include/net/sock.h -c)
EXTRA_CFLAGS+=-DKERN_SK_FOR_NODE_FEATURE=$(KERN_SK_FOR_NODE_FEATURE)

SOCK_NO_OPT=$(shell grep "sock_no_setsockopt" $(KDIR)/include/net/sock.h -c)
EXTRA_CFLAGS+=-DSOCK_NO_OPT=$(SOCK_NO_OPT)
else
KERN_SK_FOR_NODE_FEATURE=$(shell grep "sk_for_each.*node" $(KSRC)/include/net/sock.h -c)
EXTRA_CFLAGS+=-DKERN_SK_FOR_NODE_FEATURE=$(KERN_SK_FOR_NODE_FEATURE)

SOCK_NO_OPT=$(shell grep "sock_no_setsockopt" $(KSRC)/include/net/sock.h -c)
EXTRA_CFLAGS+=-DSOCK_NO_OPT=$(SOCK_NO_OPT)
endif

ifneq (,$(wildcard $(KDIR)/include/linux/netdevice.h))
KERN_NETIF_TRANS_UPDATE=$(shell grep "netif_trans_update" $(KDIR)/include/linux/netdevice.h -c)
EXTRA_CFLAGS+=-DKERN_NETIF_TRANS_UPDATE=$(KERN_NETIF_TRANS_UPDATE)
else
KERN_NETIF_TRANS_UPDATE=$(shell grep "netif_trans_update" $(KSRC)/include/linux/netdevice.h -c)
EXTRA_CFLAGS+=-DKERN_NETIF_TRANS_UPDATE=$(KERN_NETIF_TRANS_UPDATE)
endif

ifneq (,$(wildcard $(KDIR)/include/linux/netdevice.h))
KERN_NDO_CHANGE_MTU_RH74=$(shell grep "ndo_change_mtu_rh74" $(KDIR)/include/linux/netdevice.h -c)
EXTRA_CFLAGS+=-DKERN_NDO_CHANGE_MTU_RH74=$(KERN_NDO_CHANGE_MTU_RH74)
else
KERN_NDO_CHANGE_MTU_RH74=$(shell grep "ndo_change_mtu_rh74" $(KSRC)/include/linux/netdevice.h -c)
EXTRA_CFLAGS+=-DKERN_NDO_CHANGE_MTU_RH74=$(KERN_NDO_CHANGE_MTU_RH74)
endif

ifneq (,$(wildcard $(KDIR)/include/linux/device.h))
KERN_CLASS_DEV_GROUPS=$(shell awk 'BEGIN { s=0 } /\*\*dev_groups;/ { s=1; exit } END { print s }' $(KDIR)/include/linux/device.h)
ifneq ($(KERN_CLASS_DEV_GROUPS),0)
EXTRA_CFLAGS+=-DKERN_CLASS_DEV_GROUPS=$(KERN_CLASS_DEV_GROUPS)
endif
else ifneq (,$(wildcard $(KSRC)/include/linux/device.h))
KERN_CLASS_DEV_GROUPS=$(shell awk 'BEGIN { s=0 } /\*\*dev_groups;/ { s=1; exit } END { print s }' $(KSRC)/include/linux/device.h)
ifneq ($(KERN_CLASS_DEV_GROUPS),0)
EXTRA_CFLAGS+=-DKERN_CLASS_DEV_GROUPS=$(KERN_CLASS_DEV_GROUPS)
endif
else
KERN_CLASS_DEV_GROUPS=0
endif


ifeq ($(KERN_CLASS_DEV_GROUPS),0)
ifneq (,$(wildcard $(KDIR)/include/linux/device/bus.h))
KERN_CLASS_DEV_GROUPS=$(shell awk 'BEGIN { s=0 } /\*\*dev_groups;/ { s=1; exit } END { print s }' $(KDIR)/include/linux/device/bus.h)
EXTRA_CFLAGS+=-DKERN_CLASS_DEV_GROUPS=$(KERN_CLASS_DEV_GROUPS)
else ifneq (,$(wildcard $(KSRC)/include/linux/device/bus.h))
KERN_CLASS_DEV_GROUPS=$(shell awk 'BEGIN { s=0 } /\*\*dev_groups;/ { s=1; exit } END { print s }' $(KSRC)/include/linux/device/bus.h)
EXTRA_CFLAGS+=-DKERN_CLASS_DEV_GROUPS=$(KERN_CLASS_DEV_GROUPS)
else
KERN_CLASS_DEV_GROUPS=0
EXTRA_CFLAGS+=-DKERN_CLASS_DEV_GROUPS=$(KERN_CLASS_DEV_GROUPS)
endif
endif

ifneq (,$(wildcard $(KDIR)/include/linux/timer.h))
KERN_TIMER_SETUP=$(shell grep 'define timer_setup' $(KDIR)/include/linux/timer.h -c)
EXTRA_CFLAGS+=-DKERN_TIMER_SETUP=$(KERN_TIMER_SETUP)
else
KERN_TIMER_SETUP=$(shell grep 'define timer_setup' $(KSRC)/include/linux/timer.h -c)
EXTRA_CFLAGS+=-DKERN_TIMER_SETUP=$(KERN_TIMER_SETUP)
endif

ifneq (,$(wildcard $(KDIR)/include/linux/refcount.h))
KERN_REFCNT_UPDATE=$(shell grep 'refcount_read' $(KDIR)/include/linux/refcount.h -c)
EXTRA_CFLAGS+=-DKERN_REFCNT_UPDATE=$(KERN_REFCNT_UPDATE)
else ifneq (,$(wildcard $(KSRC)/include/linux/refcount.h))
KERN_REFCNT_UPDATE=$(shell grep 'refcount_read' $(KSRC)/include/linux/refcount.h -c)
EXTRA_CFLAGS+=-DKERN_REFCNT_UPDATE=$(KERN_REFCNT_UPDATE)
else
KERN_REFCNT_UPDATE=0
EXTRA_CFLAGS+=-DKERN_REFCNT_UPDATE=$(KERN_REFCNT_UPDATE)
endif

ifneq (,$(wildcard $(KDIR)/include/linux/sched/signal.h))
KERN_SIG_UPDATE=$(shell grep 'signal_pending' $(KDIR)/include/linux/sched/signal.h -c)
EXTRA_CFLAGS+=-DKERN_SIG_UPDATE=$(KERN_SIG_UPDATE)
else ifneq (,$(wildcard $(KSRC)/include/linux/sched/signal.h))
KERN_SIG_UPDATE=$(shell grep 'signal_pending' $(KSRC)/include/linux/sched/signal.h -c)
EXTRA_CFLAGS+=-DKERN_SIG_UPDATE=$(KERN_SIG_UPDATE)
else
KERN_SIG_UPDATE=0
EXTRA_CFLAGS+=-DKERN_SIG_UPDATE=$(KERN_SIG_UPDATE)
endif

ifneq (,$(wildcard $(KDIR)/include/linux/net.h))
KERN_CLASS_PROTO_OPS_UPDATE=$(shell grep -A 2 '*getname' $(KDIR)/include/linux/net.h | grep 'sockaddr_len' -c)
EXTRA_CFLAGS+=-DKERN_CLASS_PROTO_OPS_UPDATE=$(KERN_CLASS_PROTO_OPS_UPDATE)
else ifneq (,$(wildcard $(KSRC)/include/linux/net.h))
KERN_CLASS_PROTO_OPS_UPDATE=$(shell grep -A 2 '*getname' $(KSRC)/include/linux/net.h | grep 'sockaddr_len' -c)
EXTRA_CFLAGS+=-DKERN_CLASS_PROTO_OPS_UPDATE=$(KERN_CLASS_PROTO_OPS_UPDATE)
else
KERN_CLASS_PROTO_OPS_UPDATE=0
EXTRA_CFLAGS+=-DKERN_CLASS_PROTO_OPS_UPDATE=$(KERN_CLASS_PROTO_OPS_UPDATE)
endif

ifneq (,$(wildcard $(KDIR)/include/linux/netdevice.h))
KERN_DEV_CHG_FLAG_UPDATE=$(shell grep -A 1 'dev_change_flags' $(KDIR)/include/linux/netdevice.h | grep netlink_ext_ack -c)
EXTRA_CFLAGS+=-DKERN_DEV_CHG_FLAG_UPDATE=$(KERN_DEV_CHG_FLAG_UPDATE)
else ifneq (,$(wildcard $(KSRC)/include/linux/netdevice.h))
KERN_DEV_CHG_FLAG_UPDATE=$(shell grep -A 1 'dev_change_flags' $(KSRC)/include/linux/netdevice.h | grep netlink_ext_ack -c)
EXTRA_CFLAGS+=-DKERN_DEV_CHG_FLAG_UPDATE=$(KERN_DEV_CHG_FLAG_UPDATE)
else
KERN_DEV_CHG_FLAG_UPDATE=0
EXTRA_CFLAGS+=-DKERN_DEV_CHG_FLAG_UPDATE=$(KERN_DEV_CHG_FLAG_UPDATE)
endif

ifneq (,$(wildcard $(KDIR)/include/linux/netdevice.h))
KERN_NDO_TIMEOUT_UPDATE=$(shell grep -A 2 'ndo_tx_timeout' $(KDIR)/include/linux/netdevice.h | grep txqueue -c)
EXTRA_CFLAGS+=-DKERN_NDO_TIMEOUT_UPDATE=$(KERN_NDO_TIMEOUT_UPDATE)
else ifneq (,$(wildcard $(KSRC)/include/linux/netdevice.h))
KERN_NDO_TIMEOUT_UPDATE=$(shell grep -A 2 'ndo_tx_timeout' $(KSRC)/include/linux/netdevice.h | grep txqueue -c)
EXTRA_CFLAGS+=-DKERN_NDO_TIMEOUT_UPDATE=$(KERN_NDO_TIMEOUT_UPDATE)
else
KERN_NDO_TIMEOUT_UPDATE=0
EXTRA_CFLAGS+=-DKERN_NDO_TIMEOUT_UPDATE=$(KERN_NDO_TIMEOUT_UPDATE)
endif

ifneq (,$(wildcard $(KDIR)/include/linux/time.h))
KERN_DO_GET_TIME=$(shell grep 'do_gettimeofday' $(KDIR)/include/linux/time* | wc -l)
EXTRA_CFLAGS+=-DKERN_DO_GET_TIME=$(KERN_DO_GET_TIME)
else ifneq (,$(wildcard $(KSRC)/include/linux/*))
KERN_DO_GET_TIME=$(shell grep 'do_gettimeofday' $(KSRC)/include/linux/time* | wc -l)
EXTRA_CFLAGS+=-DKERN_DO_GET_TIME=$(KERN_DO_GET_TIME)
else
KERN_DO_GET_TIME=0
EXTRA_CFLAGS+=-DKERN_DO_GET_TIME=$(KERN_DO_GET_TIME)
endif

ifneq (,$(wildcard $(KDIR)/include/asm-generic/uaccess.h))
KERN_GET_KENEL_DS=$(shell grep 'define get_ds' $(KDIR)/include/asm-generic/uaccess.h -c)
EXTRA_CFLAGS+=-DKERN_GET_KENEL_DS=$(KERN_GET_KENEL_DS)
else ifneq (,$(wildcard $(KSRC)/include/asm-generic/uaccess.h))
KERN_GET_KENEL_DS=$(shell grep 'define get_ds' $(KSRC)/include/asm-generic/uaccess.h -c)
EXTRA_CFLAGS+=-DKERN_GET_KENEL_DS=$(KERN_GET_KENEL_DS)
else
KERN_GET_KENEL_DS=0
EXTRA_CFLAGS+=-DKERN_GET_KENEL_DS=$(KERN_GET_KENEL_DS)
endif

ifneq (,$(wildcard $(KDIR)/include/linux/proc_fs.h))
KERN_PROC_CREATE=$(shell grep -A 2 'proc_create' $(KDIR)/include/linux/proc_fs.h | grep file_operations -c)
EXTRA_CFLAGS+=-DKERN_PROC_CREATE=$(KERN_PROC_CREATE)
else ifneq (,$(wildcard $(KSRC)/include/linux/proc_fs.h))
KERN_PROC_CREATE=$(shell grep -A 2 'proc_create' $(KSRC)/include/linux/proc_fs.h | grep file_operations -c)
EXTRA_CFLAGS+=-DKERN_PROC_CREATE=$(KERN_PROC_CREATE)
else
KERN_PROC_CREATE=0
EXTRA_CFLAGS+=-DKERN_PROC_CREATE=$(KERN_PROC_CREATE)
endif

ifneq (,$(wildcard $(KDIR)/include/linux/types.h))
KERN_TIMER_32BIT=$(shell grep 'time_t' $(KDIR)/include/linux/t* | grep typedef -c)
EXTRA_CFLAGS+=-DKERN_TIMER_32BIT=$(KERN_TIMER_32BIT)
else ifneq (,$(wildcard $(KSRC)/include/linux/types.h))
KERN_TIMER_32BIT=$(shell grep 'time_t' $(KSRC)/include/linux/t* | grep typedef -c)
EXTRA_CFLAGS+=-DKERN_TIMER_32BIT=$(KERN_TIMER_32BIT)
else
KERN_TIMER_32BIT=0
EXTRA_CFLAGS+=-DKERN_TIMER_32BIT=$(KERN_TIMER_32BIT)
endif

ifneq (,$(wildcard $(KDIR)/scripts/mod/modpost.c))
KERN_MODPOST_STATIC_ERR=$(shell grep -nr "is a static " $(KDIR)/scripts/mod/modpost.c | grep error -c)
EXTRA_CFLAGS+=-DKERN_MODPOST_STATIC_ERR=$(KERN_MODPOST_STATIC_ERR)
else ifneq (,$(wildcard $(KSRC)/scripts/mod/modpost.c))
KERN_MODPOST_STATIC_ERR=$(shell grep -nr "is a static " $(KSRC)/scripts/mod/modpost.c | grep error -c)
EXTRA_CFLAGS+=-DKERN_MODPOST_STATIC_ERR=$(KERN_MODPOST_STATIC_ERR)
else
KERN_MODPOST_STATIC_ERR=0
EXTRA_CFLAGS+=-DKERN_MODPOST_STATIC_ERR=$(KERN_MODPOST_STATIC_ERR)
endif

ifneq (,$(wildcard $(KDIR)/include/linux/sched.h))
KERN_TASK_STATE_CHG=$(shell grep -A20  "task_struct {" $(KDIR)/include/linux/sched.h | grep __state -c)
EXTRA_CFLAGS+=-DKERN_TASK_STATE_CHG=$(KERN_TASK_STATE_CHG)
else ifneq (,$(wildcard $(KSRC)/include/linux/sched.h))
KERN_TASK_STATE_CHG=$(shell grep -A20  "task_struct {" $(KSRC)/include/linux/sched.h | grep __state -c)
EXTRA_CFLAGS+=-DKERN_TASK_STATE_CHG=$(KERN_TASK_STATE_CHG)
else
KERN_TASK_STATE_CHG=0
EXTRA_CFLAGS+=-DKERN_TASK_STATE_CHG=$(KERN_TASK_STATE_CHG)
endif

ifneq (,$(wildcard $(KDIR)/include/linux/skbuff.h))
KERN_RECV_DATAGRAM_CHG=${shell grep "*skb_recv_datagram" $(KDIR)/include/linux/skbuff.h | tr -cd ")" | wc -c}
EXTRA_CFLAGS+=-DKERN_RECV_DATAGRAM_CHG=$(KERN_RECV_DATAGRAM_CHG)
else ifneq (,$(wildcard $(KSRC)/include/linux/skbuff.h))
KERN_RECV_DATAGRAM_CHG=${shell grep "*skb_recv_datagram" $(KSRC)/include/linux/skbuff.h | tr -cd ")" | wc -c}
EXTRA_CFLAGS+=-DKERN_RECV_DATAGRAM_CHG=$(KERN_RECV_DATAGRAM_CHG)
else
KERN_RECV_DATAGRAM_CHG=0
EXTRA_CFLAGS+=-DKERN_RECV_DATAGRAM_CHG=$(KERN_RECV_DATAGRAM_CHG)
endif

ifneq (,$(wildcard $(KDIR)/arch/x86/include/asm/io.h))
KERN_BUS_CHG=${shell grep -wir "bus_to_virt" $(KDIR)/arch/x86/include/asm/io.h | wc -l}
EXTRA_CFLAGS+=-DKERN_BUS_CHG=$(KERN_BUS_CHG)
else ifneq (,$(wildcard $(KSRC)/arch/x86/include/asm/io.h))
KERN_BUS_CHG=${shell grep -wir "bus_to_virt" $(KSRC)/arch/x86/include/asm/io.h | wc -l}
EXTRA_CFLAGS+=-DKERN_BUS_CHG=$(KERN_BUS_CHG)
else
KERN_BUS_CHG=0
EXTRA_CFLAGS+=-DKERN_BUS_CHG=$(KERN_BUS_CHG)
endif

# First pass, kernel Makefile reads module objects
ifneq ($(KERNELRELEASE),)
obj-m := sdladrv.o wanrouter.o wanpipe.o wanpipe_syncppp.o wanec.o 

# Second pass, the actual build.
else

#This will check for zaptel, kenrel source and build utilites and kernel modules
#within local directory structure

all: freetdm 

all_wan: cleanup_local _checkzap _checksrc all_bin_kmod all_util all_lib	

all_src_dahdi: cleanup_local  _checkzap _checksrc all_kmod_dahdi all_util all_lib 

all_src: cleanup_local  _checksrc all_kmod all_util all_lib 

all_src_ss7: cleanup_local _checkzap _checksrc all_kmod_ss7 all_util all_lib

dahdi: all_src_dahdi

zaptel: dahdi

freetdm: all_src 
	@touch .no_legacy_smg

openzap:freetdm
smg:	freetdm
fs:		freetdm
g3ti:   freetdm

openzap_ss7: all_src_ss7 all_lib

tdmapi: all_src all_lib

cleanup_local:
	@rm -f .all* 2> /dev/null;
	@rm -f .no_legacy_smg 2> /dev/null;



#Build only kernel modules
all_kmod_dahdi:  _checkzap _checksrc _cleanoldwanpipe _check_kver
	$(MAKE) KBUILD_VERBOSE=$(KBUILD_VERBOSE) KBUILD_MODPOST_WARN=$(KBUILD_MODPOST_WARN) -C $(KDIR) M=$(WAN_DIR) EXTRA_FLAGS="$(EXTRA_CFLAGS) $(DAHDI_CFLAGS) $(shell cat ./patches/kfeatures)" ZAPDIR=$(ZAPDIR_PRIV) ZAPHDLC=$(ZAPHDLC_PRIV) HOMEDIR=$(PWD) modules  

all_kmod:  _checksrc _cleanoldwanpipe _check_kver
	$(MAKE) KBUILD_VERBOSE=$(KBUILD_VERBOSE) KBUILD_MODPOST_WARN=$(KBUILD_MODPOST_WARN) -C $(KDIR) M=$(WAN_DIR) EXTRA_FLAGS="$(EXTRA_CFLAGS) $(shell cat ./patches/kfeatures)" ZAPDIR= ZAPHDLC= HOMEDIR=$(PWD) modules  

all_kmod_ss7: _checkzap _checksrc _cleanoldwanpipe _check_kver
	@if [ -e  $(PWD)/ss7_build_dir ]; then \
		rm -rf $(PWD)/ss7_build_dir; \
	fi
	@mkdir -p $(PWD)/ss7_build_dir
	./Setup drivers --builddir=$(PWD)/ss7_build_dir --with-linux=$(KDIR) $(ZAP_OPTS) --usr-cc=$(CC) --protocol=AFT_TE1-XMTP2 --no-zaptel-compile --noautostart --arch=$(WARCH) --silent
	@eval "./patches/copy_modules.sh $(PWD)/ss7_build_dir $(WAN_DIR)"  

all_bin_kmod:  _checkzap _checksrc _cleanoldwanpipe _check_kver
	@if [ -e  $(PWD)/ast_build_dir ]; then \
		rm -rf $(PWD)/ast_build_dir; \
	fi
	@mkdir -p $(PWD)/ast_build_dir
	./Setup drivers --builddir=$(PWD)/ast_build_dir --with-linux=$(KDIR) $(ZAP_OPTS) --usr-cc=$(CC) --protocol=$(PROTS) --no-zaptel-compile --noautostart --arch=$(WARCH) --silent
	@eval "./patches/copy_modules.sh $(PWD)/ast_build_dir $(WAN_DIR)"


#Clean utilites and kernel modules
.PHONY: clean
clean: cleanup_local  clean_util _cleanoldwanpipe
	$(MAKE) -C $(KDIR) M=$(WAN_DIR) clean
	$(MAKE) -C api M=$(WAN_DIR) clean
	@find patches/kdrivers -name '.*.cmd' | xargs rm -f
	@find . -name 'Module.symver*' | xargs rm -f
	@if [ -e .no_legacy_smg ]; then \
		rm -f .no_legacy_smg; \
	fi


                                    
#Clean old wanpipe headers from linux include
.PHONY: _cleanoldwanpipe
_cleanoldwanpipe: _checksrc
	@eval "./patches/build_links.sh"
	@eval "./patches/clean_old_wanpipe.sh $(WINCLUDE) $(KDIR)/include/linux"
	

#Check for linux headers
.PHONY: _checksrc
_checksrc: 
	@if [ ! -e $(KDIR) ]; then \
		echo "   Error linux headers/source not found: $(KDIR) !"; \
		echo ; \
		exit 1; \
	fi 
	@if [ ! -e $(KDIR)/.config ]; then \
		echo "   Error linux headers/source not configured: missing $(KDIR)/.config !"; \
		echo ; \
		exit 1; \
	fi 
	@if [ ! -e $(KDIR)/include ]; then \
		echo "   Error linux headers/source incomplete: missing $(KDIR)/include dir !"; \
		echo ; \
		exit 1; \
	fi

.PHONY: _check_kver
_check_kver:
	@eval "./patches/kern_i_private_check.sh $(KDIR)"
	@echo > ./patches/kfeatures; 
	@if [ -e ./patches/i_private_found ]; then \
		echo "-DWANPIPE_USE_I_PRIVATE " >> ./patches/kfeatures; \
	fi

#Check for zaptel
.PHONY: _checkzap
_checkzap:
	@echo
	@echo " +--------- Wanpipe Build Info --------------+"  
	@echo 
	@if [ ! -e $(ZAPDIR)/zaptel.h ] && [ ! -e $(ZAPDIR)/kernel/zaptel.h ] && [ ! -e $(ZAPDIR)/include/dahdi/kernel.h ] ; then \
		echo " ZAPTEL/DAHDI Support: Disabled"; \
		ZAPDIR_PRIV=; \
		ENABLE_WANPIPEMON_ZAP=NO; \
	else \
		if [ -e $(ZAPDIR)/include/dahdi/kernel.h ]; then \
		    echo " ZAPTEL/DAHDI Support: DAHDI Enabled"; \
			echo " DAHDI Dir           : $(ZAPDIR)"; \
			echo; \
			ZAPDIR_PRIV=$(ZAPDIR); \
			ENABLE_WANPIPEMON_ZAP=YES; \
			if [ -f $(ZAPDIR)/drivers/dahdi/Module.symvers ]; then \
			  	cp -f $(ZAPDIR)/drivers/dahdi/Module.symvers $(WAN_DIR)/; \
			elif [ -f $(ZAPDIR)/src/dahdi-headers/drivers/dahdi/Module.symvers ]; then \
			  	cp -f $(ZAPDIR)/src/dahdi-headers/drivers/dahdi/Module.symvers $(WAN_DIR)/; \
			else \
				echo "Error: Dahdi source not compiled, missing Module.symvers file"; \
				echo "	     Please recompile Dahdi directory first"; \
			fi; \
		else \
		    echo " ZAPTEL/DAHDI Support: ZAPTEL Enabled"; \
			echo " ZAPTEL Dir          : $(ZAPDIR)"; \
			echo; \
			eval "$(PWD)/patches/sangoma-zaptel-patch.sh $(ZAPDIR)"; \
			ZAPDIR_PRIV=$(ZAPDIR); \
			ENABLE_WANPIPEMON_ZAP=YES; \
			if [ -f $(ZAPDIR)/kernel/Module.symvers ]; then \
				cp -f $(ZAPDIR)/kernel/Module.symvers $(WAN_DIR)/; \
			elif [ -f $(ZAPDIR)/Module.symvers ]; then \
				cp -f $(ZAPDIR)/Module.symvers $(WAN_DIR)/; \
			else \
				echo "Error: Zaptel source not compiled, missing Module.symvers file"; \
				echo "       Please recompile zaptel directory first"; \
			fi; \
			echo ; \
		fi \
	fi
	@echo 
	@echo " +-------------------------------------------+" 
	@echo 
	@sleep 2; 

#Install all utilities etc and modules
.PHONY: install
install: install_etc install_util install_kmod install_inc 
	@if [ -e .all_lib ] ; then \
		$(MAKE) -C api/libsangoma install DESTDIR=$(INSTALLPREFIX); \
		$(MAKE) -C api/libstelephony install DESTDIR=$(INSTALLPREFIX); \
		if [ "$(shell id -u)" = "0" ]; then \
			$(LDCONFIG) ; \
		fi \
    	fi


#Install kernel modules only
.PHONY: install_kmod
install_kmod:
	install -m 644 -D $(WAN_DIR)/wanrouter.${MODTYPE}  	$(INSTALLPREFIX)/$(KINSTDIR)/net/wanrouter/wanrouter.${MODTYPE} 
	install -m 644 -D $(WAN_DIR)/af_wanpipe.${MODTYPE} 	$(INSTALLPREFIX)/$(KINSTDIR)/net/wanrouter/af_wanpipe.${MODTYPE}   
	install -m 644 -D $(WAN_DIR)/wanec.${MODTYPE} 		$(INSTALLPREFIX)/$(KINSTDIR)/net/wanrouter/wanec.${MODTYPE}   
	install -m 644 -D $(WAN_DIR)/wan_aften.${MODTYPE} 	$(INSTALLPREFIX)/$(KINSTDIR)/net/wanrouter/wan_aften.${MODTYPE}   
	install -m 644 -D $(WAN_DIR)/sdladrv.${MODTYPE} 	$(INSTALLPREFIX)/$(KINSTDIR)/drivers/net/wan/sdladrv.${MODTYPE}
	install -m 644 -D $(WAN_DIR)/wanpipe.${MODTYPE} 	$(INSTALLPREFIX)/$(KINSTDIR)/drivers/net/wan/wanpipe.${MODTYPE}            
	@rm -f $(INSTALLPREFIX)/$(KINSTDIR)/drivers/net/wan/wanpipe_syncppp.${MODTYPE}
	@if [ -f  $(WAN_DIR)/wanpipe_syncppp.${MODTYPE} ]; then \
		echo "install -m 644 -D $(WAN_DIR)/wanpipe_syncppp.${MODTYPE} $(INSTALLPREFIX)/$(KINSTDIR)/drivers/net/wan/wanpipe_syncppp.${MODTYPE}"; \
		install -m 644 -D $(WAN_DIR)/wanpipe_syncppp.${MODTYPE} $(INSTALLPREFIX)/$(KINSTDIR)/drivers/net/wan/wanpipe_syncppp.${MODTYPE}; \
	fi
	@rm -f $(INSTALLPREFIX)/$(KINSTDIR)/net/wanrouter/wanpipe_lip.${MODTYPE}; 
	@if [ -f  $(WAN_DIR)/wanpipe_lip.${MODTYPE} ]; then \
		echo "install -m 644 -D $(WAN_DIR)/wanpipe_lip.${MODTYPE} $(INSTALLPREFIX)/$(KINSTDIR)/net/wanrouter/wanpipe_lip.${MODTYPE}"; \
		install -m 644 -D $(WAN_DIR)/wanpipe_lip.${MODTYPE} $(INSTALLPREFIX)/$(KINSTDIR)/net/wanrouter/wanpipe_lip.${MODTYPE}; \
	fi
	@rm -f $(INSTALLPREFIX)/$(KINSTDIR)/drivers/net/wan/xmtp2km.${MODTYPE}
	@if [ -f  $(WAN_DIR)/xmtp2km.${MODTYPE} ]; then \
        echo "install -m 644 -D $(WAN_DIR)/xmtp2km.${MODTYPE} $(INSTALLPREFIX)/$(KINSTDIR)/drivers/net/wan/xmtp2km.${MODTYPE}"; \
        install -m 644 -D $(WAN_DIR)/xmtp2km.${MODTYPE} $(INSTALLPREFIX)/$(KINSTDIR)/drivers/net/wan/xmtp2km.${MODTYPE}; \
    fi
	@eval "./patches/rundepmod.sh"	
	
endif

#Compile utilities only
all_util:  all_lib 
	$(MAKE) -C util all EXTRA_FLAGS="$(EXTRA_UTIL_FLAGS)" SYSINC="$(PWD)/$(WINCLUDE) -I $(PWD)/api/libsangoma/include" CC=$(CC) \
	PREFIX=$(INSTALLPREFIX) HOSTCFLAGS="$(EXTRA_UTIL_FLAGS)" ARCH=$(WARCH) 
	$(MAKE) -C util all_wancfg EXTRA_FLAGS="$(EXTRA_UTIL_FLAGS)" SYSINC="$(PWD)/$(WINCLUDE) -I$(PWD)/api/libsangoma/include" CC=$(CC)  \
	PREFIX=$(INSTALLPREFIX) HOSTCFLAGS="$(EXTRA_UTIL_FLAGS)" HOSTCFLAGS="$(EXTRA_UTIL_FLAGS)" ARCH=$(WARCH) 2> /dev/null

all_lib:
	@if [ ! -f api/libsangoma/Makefile ] ; then \
		$(shell echo "Bootstraping libsangoma ..."; cd api/libsangoma; ./init-automake.sh >> $(PWD)/.cfg_log;  ./configure --prefix=$(LIBPREFIX) >> $(PWD)/.cfg_log;) ; \
	fi
	$(MAKE) -C api/libsangoma all 
	@if [ ! -f api/libstelephony/Makefile ] ; then \
		$(shell echo "Bootstraping libstelephony"; cd api/libstelephony; ./init-automake.sh >> $(PWD)/.cfg_log; ./configure --prefix=$(LIBPREFIX) >> $(PWD)/.cfg_log;) ; \
	fi
	$(MAKE) -C api/libstelephony all 
	@touch .all_lib

install_lib:
		$(MAKE) -C api/libsangoma install DESTDIR=$(INSTALLPREFIX)
		$(MAKE) -C api/libstelephony install DESTDIR=$(INSTALLPREFIX)

#Clean utilities only
.PHONY: clean_util
clean_util:	
	$(MAKE) -C util clean SYSINC=$(PWD)/$(WINCLUDE) CC=$(CC) PREFIX=$(INSTALLPREFIX)

#Install utilities only
.PHONY: install_util
install_util:
	$(MAKE) -C util install SYSINC=$(PWD)/$(WINCLUDE) CC=$(CC) PREFIX=$(INSTALLPREFIX)  
	$(MAKE) -C util install_wancfg SYSINC=$(PWD)/$(WINCLUDE) CC=$(CC) PREFIX=$(INSTALLPREFIX)  

.PHONY: install_smgbri
install_smgbri:
	$(MAKE) -C ssmg/sangoma_mgd.trunk/ install  SYSINC=$(PWD)/$(WINCLUDE) CC=$(CC) BRI=YES PREFIX=$(INSTALLPREFIX)
	$(MAKE) -C ssmg/libsangoma.trunk/ install DESTDIR=$(INSTALLPREFIX)
	$(MAKE) -C ssmg/sangoma_mgd.trunk/lib/libteletone install DESTDIR=$(INSTALLPREFIX)

.PHONY: install_bri
install_bri:
	$(MAKE) -C ssmg/sangoma_bri/ install SYSINC=$(PWD)/$(WINCLUDE) CC=$(CC) DESTDIR=$(INSTALLPREFIX)

.PHONY: install_smgpri
install_smgpri:
	$(MAKE) -C ssmg/sangoma_pri/ install SYSINC=$(PWD)/$(WINCLUDE) CC=$(CC) DESTDIR=$(INSTALLPREFIX)
	@if [  ! -e .no_legacy_smg ]; then \
		@echo "Installing Sangoma MGD"; \
		$(MAKE) -C ssmg/sangoma_mgd.trunk/ install SYSINC=$(PWD)/$(WINCLUDE) CC=$(CC) PRI=YES DESTDIR=$(INSTALLPREFIX) ; \
	fi
install_pri:
	@eval "cd ssmg; ./get_sangoma_prid.sh; cd .."
	$(MAKE) -C ssmg/sangoma_pri/ install SYSINC=$(PWD)/$(WINCLUDE) CC=$(CC) DESTDIR=$(INSTALLPREFIX)

install_pri_freetdm:
	@eval "cd ssmg; ./get_sangoma_prid.sh; cd .."
	$(MAKE) -C ssmg/sangoma_pri/ install_libs install_freetdm_lib SYSINC=$(PWD)/$(WINCLUDE) CC=$(CC) DESTDIR=$(INSTALLPREFIX)

install_pri_freetdm_update:
	@eval "cd ssmg; ./get_sangoma_prid.sh --update; cd .."
	$(MAKE) -C ssmg/sangoma_pri/ install_libs install_freetdm_lib SYSINC=$(PWD)/$(WINCLUDE) CC=$(CC) DESTDIR=$(INSTALLPREFIX)
	

install_pri_update:
	@eval "cd ssmg; ./get_sangoma_prid.sh --update; cd .."
	$(MAKE) -C ssmg/sangoma_pri/ install SYSINC=$(PWD)/$(WINCLUDE) CC=$(CC) DESTDIR=$(INSTALLPREFIX)

#Install etc files
install_etc:
	@if [ ! -e $(INSTALLPREFIX)/etc/wanpipe ]; then \
	      	mkdir -p $(INSTALLPREFIX)/etc/wanpipe; \
			mkdir -p $(INSTALLPREFIX)/etc/wanpipe/util; \
	fi
	@if [ ! -e $(INSTALLPREFIX)/etc/wanpipe/wanrouter.rc ]; then \
		install -D -m 644 samples/wanrouter.rc $(INSTALLPREFIX)/etc/wanpipe/wanrouter.rc; \
	fi
	@if [ ! -e  $(INSTALLPREFIX)/etc/wanpipe/lib ]; then \
		mkdir -p $(INSTALLPREFIX)/etc/wanpipe/lib; \
	fi
	@\cp -f util/wancfg_legacy/lib/* $(INSTALLPREFIX)/etc/wanpipe/lib/
	@\cp -rf firmware  $(INSTALLPREFIX)/etc/wanpipe/
	@if [ ! -f $(INSTALLPREFIX)/etc/wanpipe/interfaces ]; then \
		mkdir -p $(INSTALLPREFIX)/etc/wanpipe/interfaces; \
	fi    
	@\cp -rf samples $(INSTALLPREFIX)/etc/wanpipe
	@if [ ! -d $(INSTALLPREFIX)/etc/wanpipe/scripts ]; then \
		mkdir -p $(INSTALLPREFIX)/etc/wanpipe/scripts; \
	fi   
	@\cp -rf wan_ec  $(INSTALLPREFIX)/etc/wanpipe/
	@\rm -f $(INSTALLPREFIX)/etc/wanpipe/api/Makefile*
	@\cp -rf api  $(INSTALLPREFIX)/etc/wanpipe/
	@\cp -rf scripts  $(INSTALLPREFIX)/etc/wanpipe/
	@\cp -rf util/wan_aftup   $(INSTALLPREFIX)/etc/wanpipe/util/
	@install -D -m 755 samples/wanrouter  $(INSTALLPREFIX)/usr/sbin/wanrouter
	@echo
	@echo "Wanpipe etc installed in $(INSTALLPREFIX)/etc/wanpipe";
	@echo

install_inc:
	@if [ -e $(INSTALLPREFIX)/usr/include/wanpipe ]; then \
		\rm -rf $(INSTALLPREFIX)/usr/include/wanpipe; \
        fi
	@\mkdir -p $(INSTALLPREFIX)/usr/include/wanpipe 
	@\cp -f $(PWD)/patches/kdrivers/include/*.h $(INSTALLPREFIX)/usr/include/wanpipe/
	@\cp -rf $(PWD)/patches/kdrivers/wanec/oct6100_api/include/* $(INSTALLPREFIX)/usr/include/wanpipe/	
	@\cp -rf $(PWD)/patches/kdrivers/wanec/*.h $(INSTALLPREFIX)/usr/include/wanpipe/
	@( cd $(INSTALLPREFIX)/usr/include/wanpipe; ln -s . linux; )
	@( cd $(INSTALLPREFIX)/usr/include/wanpipe; ln -s . oct6100_api; )

smgbri: 
	@cd ssmg/libsangoma.trunk; ./configure --prefix=$(LIBPREFIX) ; cd ../..;
	$(MAKE) -C ssmg/libsangoma.trunk/ CC=$(CC) PREFIX=$(INSTALLPREFIX) KDIR=$(KDIR)
	@cd ssmg/sangoma_mgd.trunk/lib/libteletone; ./configure --prefix=$(LIBPREFIX) ; cd ../../..;
	$(MAKE) -C ssmg/sangoma_mgd.trunk/lib/libteletone CC=$(CC) PREFIX=$(INSTALLPREFIX) KDIR=$(KDIR)
	$(MAKE) -C ssmg/sangoma_mgd.trunk/ CC=$(CC) PREFIX=$(INSTALLPREFIX) KDIR=$(KDIR) ASTDIR=$(ASTDIR)


.PHONY: clean_smgbri
clean_smgbri:
	$(MAKE) -C ssmg/libsangoma.trunk/ clean CC=$(CC) PREFIX=$(INSTALLPREFIX) KDIR=$(KDIR)
	$(MAKE) -C ssmg/sangoma_mgd.trunk/lib/libteletone clean CC=$(CC) PREFIX=$(INSTALLPREFIX) KDIR=$(KDIR)
	$(MAKE) -C ssmg/sangoma_mgd.trunk/ clean CC=$(CC) PREFIX=$(INSTALLPREFIX) KDIR=$(KDIR)

.PHONY: distclean
distclean: clean
	rm -f .pbxdir 
	
