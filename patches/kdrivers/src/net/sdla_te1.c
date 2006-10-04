/*
 * Copyright (c) 2001
 *	Alex Feldman <al.feldman@sangoma.com>.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by Alex Feldman.
 * 4. Neither the name of the author nor the names of any co-contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY Alex Feldman AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL Alex Feldman OR THE VOICES IN HIS HEAD
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 *
 *	$Id: sdla_te1.c,v 1.201 2005/11/09 18:12:54 sangoma Exp $
 */

/*
 ******************************************************************************
 * sdla_te1.c	WANPIPE(tm) Multiprotocol WAN Link Driver. 
 *				T1/E1 board configuration.
 *
 * Author: 	Alex Feldman  <al.feldman@sangoma.com>
 *
 * ============================================================================
 * Aprl 29, 2001	Alex Feldma	Initial version.
 ******************************************************************************
 */

/*
 ******************************************************************************
			   INCLUDE FILES
 ******************************************************************************
*/
#if defined(__NetBSD__) || defined(__FreeBSD__) || defined(__OpenBSD__)
# include <net/wanpipe_includes.h>
# if !defined(CONFIG_PRODUCT_WANPIPE_GENERIC)
#  include <net/wanpipe_snmp.h>
# endif
# include <net/wanpipe.h>	/* WANPIPE common user API definitions */
# include <net/wanproc.h>
#elif (defined __WINDOWS__)
# include <include_A104\pnp\wanpipe_includes.h>
# include <include_A104\pnp\wanpipe.h>	/* WANPIPE common user API definitions */
# include <include_A104\pnp\wanpipe_snmp.h>
# include <include_A104\pnp\sdla_te1_def.h>
# if defined te_cfg
#  undef te_cfg
# endif
#elif (defined __LINUX__) || (defined __KERNEL__)
# include <linux/wanpipe_includes.h>
# include <linux/wanpipe_defines.h>
# include <linux/wanpipe_debug.h>
# include <linux/wanproc.h>
# if !defined(CONFIG_PRODUCT_WANPIPE_GENERIC)
#  include <linux/wanpipe_snmp.h>
# endif
# include <linux/wanpipe.h>	/* WANPIPE common user API definitions */
#else
# error "No OS Defined"
#endif

/*
 ******************************************************************************
			  DEFINES AND MACROS
 ******************************************************************************
*/
#define FIRST_SAMPLE	0
#define LAST_SAMPLE	23
#define FIRST_UI	0
#define LAST_UI		4

#define MAX_BUSY_READ	0x05

/* Enabling/Disabling register debugging */
#undef WAN_DEBUG_TE1_REG
#ifdef WAN_DEBUG_TE1_REG

#define TEST_REG(reg,value){						\
	unsigned char test_value = READ_REG(reg);			\
	if (test_value != value){					\
		DEBUG_EVENT("%s:%d: Test Failed\n",			\
				__FILE__,__LINE__);			\
		DEBUG_EVENT("%s:%d: (Reg=%02x, Val=%02x)\n",		\
				reg, value);				\
	}								\
}

#define TEST_RPSC_REG(fe,reg,channel,value){				\
	unsigned char test_value = ReadRPSCReg(fe,channel,reg);	\
	if (test_value != value){					\
		DEBUG_EVENT("%s:%d: RPSC Test Failed\n",		\
				__FILE__, __LINE__);			\
		DEBUG_EVENT("%s:%d: (Reg=%02x,Channel=%d,Val=%02x)\n",	\
				reg, channel, value);			\
	}								\
}

#define TEST_TPSC_REG(fe,reg,channel,value){				\
	unsigned char test_value = ReadTPSCReg(fe,channel,reg);	\
	if (test_value != value){					\
		DEBUG_EVENT("%s:%d: TPSC Test Failed\n",		\
				__FILE__, __LINE__);			\
		DEBUG_EVENT("%s:%d: (Reg=%02x,Channel=%d,Val=%02x)\n",	\
				reg, channel, value);			\
	}								\
}

#else

#define TEST_REG(reg,value)
#define TEST_RPSC_REG(fe,reg,channel,value)
#define TEST_TPSC_REG(fe,reg,channel,value)
	
#endif

#if 0
#define READ_REG(reg)			card->wandev.read_front_end_reg(card,reg)
#define WRITE_REG(reg,value)		card->wandev.write_front_end_reg(card,reg,(unsigned char)(value))
#endif
#define READ_RPSC_REG(reg,channel)	ReadRPSCReg(fe,reg,channel)
#define READ_TPSC_REG(reg,channel)	ReadTPSCReg(fe,reg,channel)
#define READ_SIGX_REG(reg,channel)	ReadSIGXReg(fe,reg,channel)
#define WRITE_RPSC_REG(reg,channel,value)				\
	{								\
		WriteRPSCReg(fe,reg,channel,(unsigned char)value);	\
		TEST_RPSC_REG(fe,reg,channel,(unsigned char)value);	\
	}
			
#define WRITE_TPSC_REG(reg,channel,value)				\
	{								\
		WriteTPSCReg(fe,reg,channel,(unsigned char)value);	\
		TEST_TPSC_REG(fe,reg,channe,(unsigned char)value);	\
	}

#if 0
#define WRITE_SIGX_REG(reg,channel,value)				\
	{								\
		WriteSIGXReg(fe,reg,channel,(unsigned char)value);	\
		TEST_SIGX_REG(fe,reg,channel,(unsigned char)value);	\
	}
#endif
	
#define IS_T1_ALARM(alarm)			\
		(alarm & 			\
			(			\
			 WAN_TE_BIT_RED_ALARM |	\
			 WAN_TE_BIT_AIS_ALARM |	\
			 WAN_TE_BIT_OOF_ALARM |	\
			 WAN_TE_BIT_LOS_ALARM |	\
			 WAN_TE_BIT_ALOS_ALARM	\
			 )) 

#define IS_E1_ALARM(alarm)			\
		(alarm &	 		\
			(			\
			 WAN_TE_BIT_RED_ALARM | \
			 WAN_TE_BIT_AIS_ALARM | \
			 WAN_TE_BIT_OOF_ALARM | \
			 WAN_TE_BIT_LOS_ALARM | \
			 WAN_TE_BIT_ALOS_ALARM 	\
			 ))

#if 0
# define FE_ALOS_ENABLE

# define FE_OOF_PRINT
# define FE_LOS_PRINT
# define FE_ALOS_PRINT
#endif

/*
 ******************************************************************************
			STRUCTURES AND TYPEDEFS
 ******************************************************************************
*/
typedef unsigned char TX_WAVEFORM[LAST_SAMPLE-FIRST_SAMPLE+1][LAST_UI-FIRST_UI+1];
typedef struct RLPS_EQUALIZER_RAM_T {
	/*unsigned char address;*/
	unsigned char byte1;
	unsigned char byte2;
	unsigned char byte3;
	unsigned char byte4;
} RLPS_EQUALIZER_RAM;

/*
 ******************************************************************************
			   GLOBAL VARIABLES
 ******************************************************************************
*/


/* Transmit Waveform Values for T1 Long Haul (LBO 0db)
 * unsigned char t1_tx_waveform_lh_0db[LAST_SAMPLE-FIRST_SAMPLE+1][LAST_UI-FIRST_UI+1] = 
 */
TX_WAVEFORM pmc_t1_tx_waveform_lh_0db = 
{
	{ 0x00, 0x44, 0x00, 0x00, 0x00 },
	{ 0x0A, 0x44, 0x00, 0x00, 0x00 },
	{ 0x20, 0x43, 0x00, 0x00, 0x00 },
	{ 0x32, 0x43, 0x00, 0x00, 0x00 },
	{ 0x3E, 0x42, 0x00, 0x00, 0x00 },
	{ 0x3D, 0x42, 0x00, 0x00, 0x00 },
	{ 0x3C, 0x41, 0x00, 0x00, 0x00 },
	{ 0x3B, 0x41, 0x00, 0x00, 0x00 },
	{ 0x3A, 0x00, 0x00, 0x00, 0x00 },
	{ 0x39, 0x00, 0x00, 0x00, 0x00 },
	{ 0x39, 0x00, 0x00, 0x00, 0x00 },
	{ 0x38, 0x00, 0x00, 0x00, 0x00 },
	{ 0x37, 0x00, 0x00, 0x00, 0x00 },
	{ 0x36, 0x00, 0x00, 0x00, 0x00 },
	{ 0x34, 0x00, 0x00, 0x00, 0x00 },
	{ 0x29, 0x00, 0x00, 0x00, 0x00 },
	{ 0x4F, 0x00, 0x00, 0x00, 0x00 },
	{ 0x4C, 0x00, 0x00, 0x00, 0x00 },
	{ 0x4A, 0x00, 0x00, 0x00, 0x00 },
	{ 0x49, 0x00, 0x00, 0x00, 0x00 },
	{ 0x47, 0x00, 0x00, 0x00, 0x00 },
	{ 0x47, 0x00, 0x00, 0x00, 0x00 },
	{ 0x46, 0x00, 0x00, 0x00, 0x00 },
	{ 0x46, 0x00, 0x00, 0x00, 0x00 }
};
TX_WAVEFORM pmc4_t1_tx_waveform_lh_0db = 
{
	{ 0x00, 0x46, 0x00, 0x00, 0x00 },
	{ 0x0A, 0x45, 0x00, 0x00, 0x00 },
	{ 0x20, 0x43, 0x00, 0x00, 0x00 },
	{ 0x32, 0x41, 0x00, 0x00, 0x00 },
	{ 0x3E, 0x40, 0x00, 0x00, 0x00 },
	{ 0x3D, 0x40, 0x00, 0x00, 0x00 },
	{ 0x3C, 0x00, 0x00, 0x00, 0x00 },
	{ 0x3B, 0x00, 0x00, 0x00, 0x00 },
	{ 0x3A, 0x00, 0x00, 0x00, 0x00 },
	{ 0x39, 0x00, 0x00, 0x00, 0x00 },
	{ 0x39, 0x00, 0x00, 0x00, 0x00 },
	{ 0x38, 0x00, 0x00, 0x00, 0x00 },
	{ 0x37, 0x00, 0x00, 0x00, 0x00 },
	{ 0x36, 0x00, 0x00, 0x00, 0x00 },
	{ 0x30, 0x00, 0x00, 0x00, 0x00 },
	{ 0x10, 0x00, 0x00, 0x00, 0x00 },
	{ 0x49, 0x00, 0x00, 0x00, 0x00 },
	{ 0x51, 0x00, 0x00, 0x00, 0x00 },
	{ 0x50, 0x00, 0x00, 0x00, 0x00 },
	{ 0x4E, 0x00, 0x00, 0x00, 0x00 },
	{ 0x4C, 0x00, 0x00, 0x00, 0x00 },
	{ 0x4A, 0x00, 0x00, 0x00, 0x00 },
	{ 0x48, 0x00, 0x00, 0x00, 0x00 },
	{ 0x47, 0x00, 0x00, 0x00, 0x00 }
};

/* Transmit Waveform Values for T1 Long Haul (LBO 7.5 dB): 
 * unsigned char t1_tx_waveform_lh_75db[LAST_SAMPLE-FIRST_SAMPLE+1][LAST_UI-FIRST_UI+1] = 
 */
TX_WAVEFORM pmc_t1_tx_waveform_lh_75db = 
{
    { 0x00, 0x10, 0x00, 0x00, 0x00 },
    { 0x01, 0x0E, 0x00, 0x00, 0x00 },
    { 0x02, 0x0C, 0x00, 0x00, 0x00 },
    { 0x04, 0x0A, 0x00, 0x00, 0x00 },
    { 0x08, 0x08, 0x00, 0x00, 0x00 },
    { 0x0C, 0x06, 0x00, 0x00, 0x00 },
    { 0x10, 0x04, 0x00, 0x00, 0x00 },
    { 0x16, 0x02, 0x00, 0x00, 0x00 },
    { 0x1A, 0x01, 0x00, 0x00, 0x00 },
    { 0x1E, 0x00, 0x00, 0x00, 0x00 },
    { 0x22, 0x00, 0x00, 0x00, 0x00 },
    { 0x26, 0x00, 0x00, 0x00, 0x00 },
    { 0x2A, 0x00, 0x00, 0x00, 0x00 },
    { 0x2B, 0x00, 0x00, 0x00, 0x00 },
    { 0x2C, 0x00, 0x00, 0x00, 0x00 },
    { 0x2D, 0x00, 0x00, 0x00, 0x00 },
    { 0x2C, 0x00, 0x00, 0x00, 0x00 },
    { 0x28, 0x00, 0x00, 0x00, 0x00 },
    { 0x24, 0x00, 0x00, 0x00, 0x00 },
    { 0x20, 0x00, 0x00, 0x00, 0x00 },
    { 0x1C, 0x00, 0x00, 0x00, 0x00 },
    { 0x18, 0x00, 0x00, 0x00, 0x00 },
    { 0x14, 0x00, 0x00, 0x00, 0x00 },
    { 0x12, 0x00, 0x00, 0x00, 0x00 }
};
TX_WAVEFORM pmc4_t1_tx_waveform_lh_75db = 
{
    { 0x00, 0x10, 0x00, 0x00, 0x00 },
    { 0x01, 0x0E, 0x00, 0x00, 0x00 },
    { 0x02, 0x0C, 0x00, 0x00, 0x00 },
    { 0x04, 0x0A, 0x00, 0x00, 0x00 },
    { 0x08, 0x08, 0x00, 0x00, 0x00 },
    { 0x0C, 0x06, 0x00, 0x00, 0x00 },
    { 0x10, 0x04, 0x00, 0x00, 0x00 },
    { 0x16, 0x02, 0x00, 0x00, 0x00 },
    { 0x1A, 0x01, 0x00, 0x00, 0x00 },
    { 0x1E, 0x00, 0x00, 0x00, 0x00 },
    { 0x22, 0x00, 0x00, 0x00, 0x00 },
    { 0x26, 0x00, 0x00, 0x00, 0x00 },
    { 0x2A, 0x00, 0x00, 0x00, 0x00 },
    { 0x2B, 0x00, 0x00, 0x00, 0x00 },
    { 0x2C, 0x00, 0x00, 0x00, 0x00 },
    { 0x2D, 0x00, 0x00, 0x00, 0x00 },
    { 0x2C, 0x00, 0x00, 0x00, 0x00 },
    { 0x28, 0x00, 0x00, 0x00, 0x00 },
    { 0x24, 0x00, 0x00, 0x00, 0x00 },
    { 0x20, 0x00, 0x00, 0x00, 0x00 },
    { 0x1C, 0x00, 0x00, 0x00, 0x00 },
    { 0x18, 0x00, 0x00, 0x00, 0x00 },
    { 0x14, 0x00, 0x00, 0x00, 0x00 },
    { 0x12, 0x00, 0x00, 0x00, 0x00 }
};


/* Transmit Waveform Values for T1 Long Haul (LBO 15 dB)
 * unsigned char t1_tx_waveform_lh_15db[LAST_SAMPLE-FIRST_SAMPLE+1][LAST_UI-FIRST_UI+1] = 
 */
TX_WAVEFORM pmc_t1_tx_waveform_lh_15db = 
{
    { 0x00, 0x2A, 0x09, 0x01, 0x00 },
    { 0x00, 0x28, 0x08, 0x01, 0x00 },
    { 0x00, 0x26, 0x08, 0x01, 0x00 },
    { 0x00, 0x24, 0x07, 0x01, 0x00 },
    { 0x01, 0x22, 0x07, 0x01, 0x00 },
    { 0x02, 0x20, 0x06, 0x01, 0x00 },
    { 0x04, 0x1E, 0x06, 0x01, 0x00 },
    { 0x07, 0x1C, 0x05, 0x00, 0x00 },
    { 0x0A, 0x1B, 0x05, 0x00, 0x00 },
    { 0x0D, 0x19, 0x05, 0x00, 0x00 },
    { 0x10, 0x18, 0x04, 0x00, 0x00 },
    { 0x14, 0x16, 0x04, 0x00, 0x00 },
    { 0x18, 0x15, 0x04, 0x00, 0x00 },
    { 0x1B, 0x13, 0x03, 0x00, 0x00 },
    { 0x1E, 0x12, 0x03, 0x00, 0x00 },
    { 0x21, 0x10, 0x03, 0x00, 0x00 },
    { 0x24, 0x0F, 0x03, 0x00, 0x00 },
    { 0x27, 0x0D, 0x03, 0x00, 0x00 },
    { 0x2A, 0x0D, 0x02, 0x00, 0x00 },
    { 0x2D, 0x0B, 0x02, 0x00, 0x00 },
    { 0x30, 0x0B, 0x02, 0x00, 0x00 },
    { 0x30, 0x0A, 0x02, 0x00, 0x00 },
    { 0x2E, 0x0A, 0x02, 0x00, 0x00 },
    { 0x2C, 0x09, 0x02, 0x00, 0x00 }
};
TX_WAVEFORM pmc4_t1_tx_waveform_lh_15db = 
{
    { 0x00, 0x2A, 0x09, 0x01, 0x00 },
    { 0x00, 0x28, 0x08, 0x01, 0x00 },
    { 0x00, 0x26, 0x08, 0x01, 0x00 },
    { 0x00, 0x24, 0x07, 0x01, 0x00 },
    { 0x01, 0x22, 0x07, 0x01, 0x00 },
    { 0x02, 0x20, 0x06, 0x01, 0x00 },
    { 0x04, 0x1E, 0x06, 0x01, 0x00 },
    { 0x07, 0x1C, 0x05, 0x00, 0x00 },
    { 0x0A, 0x1B, 0x05, 0x00, 0x00 },
    { 0x0D, 0x19, 0x05, 0x00, 0x00 },
    { 0x10, 0x18, 0x04, 0x00, 0x00 },
    { 0x14, 0x16, 0x04, 0x00, 0x00 },
    { 0x18, 0x15, 0x04, 0x00, 0x00 },
    { 0x1B, 0x13, 0x03, 0x00, 0x00 },
    { 0x1E, 0x12, 0x03, 0x00, 0x00 },
    { 0x21, 0x10, 0x03, 0x00, 0x00 },
    { 0x24, 0x0F, 0x03, 0x00, 0x00 },
    { 0x27, 0x0D, 0x03, 0x00, 0x00 },
    { 0x2A, 0x0D, 0x02, 0x00, 0x00 },
    { 0x2D, 0x0B, 0x02, 0x00, 0x00 },
    { 0x30, 0x0B, 0x02, 0x00, 0x00 },
    { 0x30, 0x0A, 0x02, 0x00, 0x00 },
    { 0x2E, 0x0A, 0x02, 0x00, 0x00 },
    { 0x2C, 0x09, 0x02, 0x00, 0x00 }
};


/* Transmit Waveform Values for T1 Long Haul (LBO 22.5 dB)
 * unsigned char t1_tx_waveform_lh_225db[LAST_SAMPLE-FIRST_SAMPLE+1][LAST_UI-FIRST_UI+1] = 
 */
TX_WAVEFORM pmc_t1_tx_waveform_lh_225db = 
{
    { 0x00, 0x1F, 0x16, 0x06, 0x01 },
    { 0x00, 0x20, 0x15, 0x05, 0x01 },
    { 0x00, 0x21, 0x15, 0x05, 0x01 },
    { 0x00, 0x22, 0x14, 0x05, 0x01 },
    { 0x00, 0x22, 0x13, 0x04, 0x00 },
    { 0x00, 0x23, 0x12, 0x04, 0x00 },
    { 0x01, 0x23, 0x12, 0x04, 0x00 },
    { 0x01, 0x24, 0x11, 0x03, 0x00 },
    { 0x01, 0x23, 0x10, 0x03, 0x00 },
    { 0x02, 0x23, 0x10, 0x03, 0x00 },
    { 0x03, 0x22, 0x0F, 0x03, 0x00 },
    { 0x05, 0x22, 0x0E, 0x03, 0x00 },
    { 0x07, 0x21, 0x0E, 0x02, 0x00 },
    { 0x09, 0x20, 0x0D, 0x02, 0x00 },
    { 0x0B, 0x1E, 0x0C, 0x02, 0x00 },
    { 0x0E, 0x1D, 0x0C, 0x02, 0x00 },
    { 0x10, 0x1B, 0x0B, 0x02, 0x00 },
    { 0x13, 0x1B, 0x0A, 0x02, 0x00 },
    { 0x15, 0x1A, 0x0A, 0x02, 0x00 },
    { 0x17, 0x19, 0x09, 0x01, 0x00 },
    { 0x19, 0x19, 0x08, 0x01, 0x00 },
    { 0x1B, 0x18, 0x08, 0x01, 0x00 },
    { 0x1D, 0x17, 0x07, 0x01, 0x00 },
    { 0x1E, 0x17, 0x06, 0x01, 0x00 }
};
TX_WAVEFORM pmc4_t1_tx_waveform_lh_225db = 
{
    { 0x00, 0x1F, 0x16, 0x06, 0x01 },
    { 0x00, 0x20, 0x15, 0x05, 0x01 },
    { 0x00, 0x21, 0x15, 0x05, 0x01 },
    { 0x00, 0x22, 0x14, 0x05, 0x01 },
    { 0x00, 0x22, 0x13, 0x04, 0x00 },
    { 0x00, 0x23, 0x12, 0x04, 0x00 },
    { 0x01, 0x23, 0x12, 0x04, 0x00 },
    { 0x01, 0x24, 0x11, 0x03, 0x00 },
    { 0x01, 0x23, 0x10, 0x03, 0x00 },
    { 0x02, 0x23, 0x10, 0x03, 0x00 },
    { 0x03, 0x22, 0x0F, 0x03, 0x00 },
    { 0x05, 0x22, 0x0E, 0x03, 0x00 },
    { 0x07, 0x21, 0x0E, 0x02, 0x00 },
    { 0x09, 0x20, 0x0D, 0x02, 0x00 },
    { 0x0B, 0x1E, 0x0C, 0x02, 0x00 },
    { 0x0E, 0x1D, 0x0C, 0x02, 0x00 },
    { 0x10, 0x1B, 0x0B, 0x02, 0x00 },
    { 0x13, 0x1B, 0x0A, 0x02, 0x00 },
    { 0x15, 0x1A, 0x0A, 0x02, 0x00 },
    { 0x17, 0x19, 0x09, 0x01, 0x00 },
    { 0x19, 0x19, 0x08, 0x01, 0x00 },
    { 0x1B, 0x18, 0x08, 0x01, 0x00 },
    { 0x1D, 0x17, 0x07, 0x01, 0x00 },
    { 0x1E, 0x17, 0x06, 0x01, 0x00 }
};


/* Transmit Waveform Values for T1 Short Haul (0 - 110 ft.)
 * unsigned char t1_tx_waveform_sh_110ft[LAST_SAMPLE-FIRST_SAMPLE+1][LAST_UI-FIRST_UI+1] = 
 */
TX_WAVEFORM pmc_t1_tx_waveform_sh_110ft = 
{
    { 0x00, 0x45, 0x00, 0x00, 0x00 },
    { 0x0A, 0x44, 0x00, 0x00, 0x00 },
    { 0x20, 0x43, 0x00, 0x00, 0x00 },
    { 0x3F, 0x43, 0x00, 0x00, 0x00 },
    { 0x3F, 0x42, 0x00, 0x00, 0x00 },
    { 0x3F, 0x42, 0x00, 0x00, 0x00 },
    { 0x3C, 0x41, 0x00, 0x00, 0x00 },
    { 0x3B, 0x41, 0x00, 0x00, 0x00 },
    { 0x3A, 0x00, 0x00, 0x00, 0x00 },
    { 0x39, 0x00, 0x00, 0x00, 0x00 },
    { 0x39, 0x00, 0x00, 0x00, 0x00 },
    { 0x38, 0x00, 0x00, 0x00, 0x00 },
    { 0x37, 0x00, 0x00, 0x00, 0x00 },
    { 0x36, 0x00, 0x00, 0x00, 0x00 },
    { 0x34, 0x00, 0x00, 0x00, 0x00 },
    { 0x29, 0x00, 0x00, 0x00, 0x00 },
    { 0x59, 0x00, 0x00, 0x00, 0x00 },
    { 0x55, 0x00, 0x00, 0x00, 0x00 },
    { 0x50, 0x00, 0x00, 0x00, 0x00 },
    { 0x4D, 0x00, 0x00, 0x00, 0x00 },
    { 0x4A, 0x00, 0x00, 0x00, 0x00 },
    { 0x48, 0x00, 0x00, 0x00, 0x00 },
    { 0x46, 0x00, 0x00, 0x00, 0x00 },
    { 0x46, 0x00, 0x00, 0x00, 0x00 }
};
TX_WAVEFORM pmc4_t1_tx_waveform_sh_110ft = 
{
    { 0x00, 0x46, 0x00, 0x00, 0x00 },
    { 0x0A, 0x45, 0x00, 0x00, 0x00 },
    { 0x20, 0x43, 0x00, 0x00, 0x00 },
    { 0x3D, 0x41, 0x00, 0x00, 0x00 },
    { 0x3D, 0x40, 0x00, 0x00, 0x00 },
    { 0x3C, 0x40, 0x00, 0x00, 0x00 },
    { 0x3C, 0x00, 0x00, 0x00, 0x00 },
    { 0x3B, 0x00, 0x00, 0x00, 0x00 },
    { 0x3A, 0x00, 0x00, 0x00, 0x00 },
    { 0x39, 0x00, 0x00, 0x00, 0x00 },
    { 0x39, 0x00, 0x00, 0x00, 0x00 },
    { 0x38, 0x00, 0x00, 0x00, 0x00 },
    { 0x37, 0x00, 0x00, 0x00, 0x00 },
    { 0x36, 0x00, 0x00, 0x00, 0x00 },
    { 0x30, 0x00, 0x00, 0x00, 0x00 },
    { 0x10, 0x00, 0x00, 0x00, 0x00 },
    { 0x58, 0x00, 0x00, 0x00, 0x00 },
    { 0x53, 0x00, 0x00, 0x00, 0x00 },
    { 0x50, 0x00, 0x00, 0x00, 0x00 },
    { 0x4E, 0x00, 0x00, 0x00, 0x00 },
    { 0x4C, 0x00, 0x00, 0x00, 0x00 },
    { 0x4A, 0x00, 0x00, 0x00, 0x00 },
    { 0x48, 0x00, 0x00, 0x00, 0x00 },
    { 0x47, 0x00, 0x00, 0x00, 0x00 }
};


/* Transmit Waveform Values for T1 Short Haul (110 - 220 ft.)
 * unsigned char t1_tx_waveform_sh_220ft[LAST_SAMPLE-FIRST_SAMPLE+1][LAST_UI-FIRST_UI+1] = 
 */
TX_WAVEFORM pmc_t1_tx_waveform_sh_220ft = 
{
    { 0x00, 0x44, 0x00, 0x00, 0x00 },
    { 0x0A, 0x44, 0x00, 0x00, 0x00 },
    { 0x3F, 0x43, 0x00, 0x00, 0x00 },
    { 0x3F, 0x43, 0x00, 0x00, 0x00 },
    { 0x36, 0x42, 0x00, 0x00, 0x00 },
    { 0x34, 0x42, 0x00, 0x00, 0x00 },
    { 0x30, 0x41, 0x00, 0x00, 0x00 },
    { 0x2F, 0x41, 0x00, 0x00, 0x00 },
    { 0x2E, 0x00, 0x00, 0x00, 0x00 },
    { 0x2D, 0x00, 0x00, 0x00, 0x00 },
    { 0x2C, 0x00, 0x00, 0x00, 0x00 },
    { 0x2B, 0x00, 0x00, 0x00, 0x00 },
    { 0x2A, 0x00, 0x00, 0x00, 0x00 },
    { 0x28, 0x00, 0x00, 0x00, 0x00 },
    { 0x26, 0x00, 0x00, 0x00, 0x00 },
    { 0x4A, 0x00, 0x00, 0x00, 0x00 },
    { 0x68, 0x00, 0x00, 0x00, 0x00 },
    { 0x54, 0x00, 0x00, 0x00, 0x00 },
    { 0x4F, 0x00, 0x00, 0x00, 0x00 },
    { 0x4A, 0x00, 0x00, 0x00, 0x00 },
    { 0x49, 0x00, 0x00, 0x00, 0x00 },
    { 0x47, 0x00, 0x00, 0x00, 0x00 },
    { 0x47, 0x00, 0x00, 0x00, 0x00 },
    { 0x46, 0x00, 0x00, 0x00, 0x00 }
};
TX_WAVEFORM pmc4_t1_tx_waveform_sh_220ft = 
{
    { 0x00, 0x45, 0x00, 0x00, 0x00 },
    { 0x0A, 0x44, 0x00, 0x00, 0x00 },
    { 0x33, 0x42, 0x00, 0x00, 0x00 },
    { 0x33, 0x41, 0x00, 0x00, 0x00 },
    { 0x33, 0x40, 0x00, 0x00, 0x00 },
    { 0x33, 0x40, 0x00, 0x00, 0x00 },
    { 0x30, 0x00, 0x00, 0x00, 0x00 },
    { 0x2F, 0x00, 0x00, 0x00, 0x00 },
    { 0x2E, 0x00, 0x00, 0x00, 0x00 },
    { 0x2D, 0x00, 0x00, 0x00, 0x00 },
    { 0x2C, 0x00, 0x00, 0x00, 0x00 },
    { 0x2B, 0x00, 0x00, 0x00, 0x00 },
    { 0x2A, 0x00, 0x00, 0x00, 0x00 },
    { 0x29, 0x00, 0x00, 0x00, 0x00 },
    { 0x19, 0x00, 0x00, 0x00, 0x00 },
    { 0x5A, 0x00, 0x00, 0x00, 0x00 },
    { 0x54, 0x00, 0x00, 0x00, 0x00 },
    { 0x50, 0x00, 0x00, 0x00, 0x00 },
    { 0x4E, 0x00, 0x00, 0x00, 0x00 },
    { 0x4C, 0x00, 0x00, 0x00, 0x00 },
    { 0x4B, 0x00, 0x00, 0x00, 0x00 },
    { 0x48, 0x00, 0x00, 0x00, 0x00 },
    { 0x48, 0x00, 0x00, 0x00, 0x00 },
    { 0x47, 0x00, 0x00, 0x00, 0x00 }
};


/* Transmit Waveform Values for T1 Short Haul (220 - 330 ft.)
 * unsigned char t1_tx_waveform_sh_330ft[LAST_SAMPLE-FIRST_SAMPLE+1][LAST_UI-FIRST_UI+1] = 
 */
TX_WAVEFORM pmc_t1_tx_waveform_sh_330ft = 
{
    { 0x00, 0x44, 0x00, 0x00, 0x00 },
    { 0x0A, 0x44, 0x00, 0x00, 0x00 },
    { 0x3F, 0x43, 0x00, 0x00, 0x00 },
    { 0x3A, 0x43, 0x00, 0x00, 0x00 },
    { 0x3A, 0x42, 0x00, 0x00, 0x00 },
    { 0x38, 0x42, 0x00, 0x00, 0x00 },
    { 0x30, 0x41, 0x00, 0x00, 0x00 },
    { 0x2F, 0x41, 0x00, 0x00, 0x00 },
    { 0x2E, 0x00, 0x00, 0x00, 0x00 },
    { 0x2D, 0x00, 0x00, 0x00, 0x00 },
    { 0x2C, 0x00, 0x00, 0x00, 0x00 },
    { 0x2B, 0x00, 0x00, 0x00, 0x00 },
    { 0x2A, 0x00, 0x00, 0x00, 0x00 },
    { 0x29, 0x00, 0x00, 0x00, 0x00 },
    { 0x23, 0x00, 0x00, 0x00, 0x00 },
    { 0x4A, 0x00, 0x00, 0x00, 0x00 },
    { 0x6C, 0x00, 0x00, 0x00, 0x00 },
    { 0x60, 0x00, 0x00, 0x00, 0x00 },
    { 0x4F, 0x00, 0x00, 0x00, 0x00 },
    { 0x4A, 0x00, 0x00, 0x00, 0x00 },
    { 0x49, 0x00, 0x00, 0x00, 0x00 },
    { 0x47, 0x00, 0x00, 0x00, 0x00 },
    { 0x47, 0x00, 0x00, 0x00, 0x00 },
    { 0x46, 0x00, 0x00, 0x00, 0x00 }
};
TX_WAVEFORM pmc4_t1_tx_waveform_sh_330ft = 
{
    { 0x00, 0x45, 0x00, 0x00, 0x00 },
    { 0x0A, 0x44, 0x00, 0x00, 0x00 },
    { 0x36, 0x43, 0x00, 0x00, 0x00 },
    { 0x36, 0x41, 0x00, 0x00, 0x00 },
    { 0x34, 0x40, 0x00, 0x00, 0x00 },
    { 0x34, 0x40, 0x00, 0x00, 0x00 },
    { 0x30, 0x00, 0x00, 0x00, 0x00 },
    { 0x2F, 0x00, 0x00, 0x00, 0x00 },
    { 0x2E, 0x00, 0x00, 0x00, 0x00 },
    { 0x2D, 0x00, 0x00, 0x00, 0x00 },
    { 0x2C, 0x00, 0x00, 0x00, 0x00 },
    { 0x2B, 0x00, 0x00, 0x00, 0x00 },
    { 0x2A, 0x00, 0x00, 0x00, 0x00 },
    { 0x29, 0x00, 0x00, 0x00, 0x00 },
    { 0x23, 0x00, 0x00, 0x00, 0x00 },
    { 0x4A, 0x00, 0x00, 0x00, 0x00 },
    { 0x60, 0x00, 0x00, 0x00, 0x00 },
    { 0x55, 0x00, 0x00, 0x00, 0x00 },
    { 0x53, 0x00, 0x00, 0x00, 0x00 },
    { 0x50, 0x00, 0x00, 0x00, 0x00 },
    { 0x4E, 0x00, 0x00, 0x00, 0x00 },
    { 0x4C, 0x00, 0x00, 0x00, 0x00 },
    { 0x48, 0x00, 0x00, 0x00, 0x00 },
    { 0x47, 0x00, 0x00, 0x00, 0x00 }
};


/* Transmit Waveform Values for T1 Short Haul (330 - 440 ft.)
 * unsigned char t1_tx_waveform_sh_440ft[LAST_SAMPLE-FIRST_SAMPLE+1][LAST_UI-FIRST_UI+1] = 
 */
TX_WAVEFORM pmc_t1_tx_waveform_sh_440ft = 
{
    { 0x00, 0x44, 0x00, 0x00, 0x00 },
    { 0x0A, 0x44, 0x00, 0x00, 0x00 },
    { 0x3F, 0x43, 0x00, 0x00, 0x00 },
    { 0x3F, 0x43, 0x00, 0x00, 0x00 },
    { 0x3F, 0x42, 0x00, 0x00, 0x00 },
    { 0x3F, 0x42, 0x00, 0x00, 0x00 },
    { 0x2F, 0x41, 0x00, 0x00, 0x00 },
    { 0x2E, 0x41, 0x00, 0x00, 0x00 },
    { 0x2D, 0x00, 0x00, 0x00, 0x00 },
    { 0x2C, 0x00, 0x00, 0x00, 0x00 },
    { 0x2B, 0x00, 0x00, 0x00, 0x00 },
    { 0x2A, 0x00, 0x00, 0x00, 0x00 },
    { 0x29, 0x00, 0x00, 0x00, 0x00 },
    { 0x28, 0x00, 0x00, 0x00, 0x00 },
    { 0x19, 0x00, 0x00, 0x00, 0x00 },
    { 0x4A, 0x00, 0x00, 0x00, 0x00 },
    { 0x7F, 0x00, 0x00, 0x00, 0x00 },
    { 0x60, 0x00, 0x00, 0x00, 0x00 },
    { 0x4F, 0x00, 0x00, 0x00, 0x00 },
    { 0x4A, 0x00, 0x00, 0x00, 0x00 },
    { 0x49, 0x00, 0x00, 0x00, 0x00 },
    { 0x47, 0x00, 0x00, 0x00, 0x00 },
    { 0x47, 0x00, 0x00, 0x00, 0x00 },
    { 0x46, 0x00, 0x00, 0x00, 0x00 }
};
TX_WAVEFORM pmc4_t1_tx_waveform_sh_440ft = 
{
    { 0x00, 0x46, 0x00, 0x00, 0x00 },
    { 0x0A, 0x45, 0x00, 0x00, 0x00 },
    { 0x3A, 0x43, 0x00, 0x00, 0x00 },
    { 0x3A, 0x41, 0x00, 0x00, 0x00 },
    { 0x37, 0x40, 0x00, 0x00, 0x00 },
    { 0x37, 0x40, 0x00, 0x00, 0x00 },
    { 0x2F, 0x00, 0x00, 0x00, 0x00 },
    { 0x2E, 0x00, 0x00, 0x00, 0x00 },
    { 0x2D, 0x00, 0x00, 0x00, 0x00 },
    { 0x2C, 0x00, 0x00, 0x00, 0x00 },
    { 0x2B, 0x00, 0x00, 0x00, 0x00 },
    { 0x2A, 0x00, 0x00, 0x00, 0x00 },
    { 0x29, 0x00, 0x00, 0x00, 0x00 },
    { 0x28, 0x00, 0x00, 0x00, 0x00 },
    { 0x19, 0x00, 0x00, 0x00, 0x00 },
    { 0x4A, 0x00, 0x00, 0x00, 0x00 },
    { 0x64, 0x00, 0x00, 0x00, 0x00 },
    { 0x57, 0x00, 0x00, 0x00, 0x00 },
    { 0x53, 0x00, 0x00, 0x00, 0x00 },
    { 0x4F, 0x00, 0x00, 0x00, 0x00 },
    { 0x4C, 0x00, 0x00, 0x00, 0x00 },
    { 0x4B, 0x00, 0x00, 0x00, 0x00 },
    { 0x48, 0x00, 0x00, 0x00, 0x00 },
    { 0x47, 0x00, 0x00, 0x00, 0x00 }
};


/* Transmit Waveform Values for T1 Short Haul (440 - 550 ft.)
 * unsigned char t1_tx_waveform_sh_550ft[LAST_SAMPLE-FIRST_SAMPLE+1][LAST_UI-FIRST_UI+1] = 
 */
TX_WAVEFORM pmc_t1_tx_waveform_sh_550ft = 
{
    { 0x00, 0x44, 0x00, 0x00, 0x00 },
    { 0x0A, 0x44, 0x00, 0x00, 0x00 },
    { 0x3F, 0x43, 0x00, 0x00, 0x00 },
    { 0x3F, 0x43, 0x00, 0x00, 0x00 },
    { 0x3F, 0x42, 0x00, 0x00, 0x00 },
    { 0x3F, 0x42, 0x00, 0x00, 0x00 },
    { 0x30, 0x41, 0x00, 0x00, 0x00 },
    { 0x2B, 0x41, 0x00, 0x00, 0x00 },
    { 0x2A, 0x00, 0x00, 0x00, 0x00 },
    { 0x29, 0x00, 0x00, 0x00, 0x00 },
    { 0x28, 0x00, 0x00, 0x00, 0x00 },
    { 0x27, 0x00, 0x00, 0x00, 0x00 },
    { 0x26, 0x00, 0x00, 0x00, 0x00 },
    { 0x26, 0x00, 0x00, 0x00, 0x00 },
    { 0x24, 0x00, 0x00, 0x00, 0x00 },
    { 0x4A, 0x00, 0x00, 0x00, 0x00 },
    { 0x7F, 0x00, 0x00, 0x00, 0x00 },
    { 0x7F, 0x00, 0x00, 0x00, 0x00 },
    { 0x4F, 0x00, 0x00, 0x00, 0x00 },
    { 0x4A, 0x00, 0x00, 0x00, 0x00 },
    { 0x49, 0x00, 0x00, 0x00, 0x00 },
    { 0x47, 0x00, 0x00, 0x00, 0x00 },
    { 0x47, 0x00, 0x00, 0x00, 0x00 },
    { 0x46, 0x00, 0x00, 0x00, 0x00 }
};
TX_WAVEFORM pmc4_t1_tx_waveform_sh_550ft = 
{
    { 0x00, 0x46, 0x00, 0x00, 0x00 },
    { 0x0A, 0x45, 0x00, 0x00, 0x00 },
    { 0x3E, 0x43, 0x00, 0x00, 0x00 },
    { 0x3E, 0x41, 0x00, 0x00, 0x00 },
    { 0x3E, 0x40, 0x00, 0x00, 0x00 },
    { 0x30, 0x40, 0x00, 0x00, 0x00 },
    { 0x30, 0x00, 0x00, 0x00, 0x00 },
    { 0x2B, 0x00, 0x00, 0x00, 0x00 },
    { 0x2A, 0x00, 0x00, 0x00, 0x00 },
    { 0x29, 0x00, 0x00, 0x00, 0x00 },
    { 0x28, 0x00, 0x00, 0x00, 0x00 },
    { 0x27, 0x00, 0x00, 0x00, 0x00 },
    { 0x26, 0x00, 0x00, 0x00, 0x00 },
    { 0x24, 0x00, 0x00, 0x00, 0x00 },
    { 0x19, 0x00, 0x00, 0x00, 0x00 },
    { 0x4A, 0x00, 0x00, 0x00, 0x00 },
    { 0x78, 0x00, 0x00, 0x00, 0x00 },
    { 0x57, 0x00, 0x00, 0x00, 0x00 },
    { 0x53, 0x00, 0x00, 0x00, 0x00 },
    { 0x4F, 0x00, 0x00, 0x00, 0x00 },
    { 0x4C, 0x00, 0x00, 0x00, 0x00 },
    { 0x4B, 0x00, 0x00, 0x00, 0x00 },
    { 0x48, 0x00, 0x00, 0x00, 0x00 },
    { 0x47, 0x00, 0x00, 0x00, 0x00 }
};


/* Transmit Waveform Values for T1 Short Haul (550 - 660 ft.)
 * unsigned char t1_tx_waveform_sh_660ft[LAST_SAMPLE-FIRST_SAMPLE+1][LAST_UI-FIRST_UI+1] = 
 */
TX_WAVEFORM pmc_t1_tx_waveform_sh_660ft = 
{
    { 0x00, 0x44, 0x00, 0x00, 0x00 },
    { 0x0A, 0x44, 0x00, 0x00, 0x00 },
    { 0x3F, 0x43, 0x00, 0x00, 0x00 },
    { 0x3F, 0x43, 0x00, 0x00, 0x00 },
    { 0x3F, 0x42, 0x00, 0x00, 0x00 },
    { 0x3F, 0x42, 0x00, 0x00, 0x00 },
    { 0x3F, 0x41, 0x00, 0x00, 0x00 },
    { 0x30, 0x41, 0x00, 0x00, 0x00 },
    { 0x2A, 0x00, 0x00, 0x00, 0x00 },
    { 0x29, 0x00, 0x00, 0x00, 0x00 },
    { 0x28, 0x00, 0x00, 0x00, 0x00 },
    { 0x27, 0x00, 0x00, 0x00, 0x00 },
    { 0x26, 0x00, 0x00, 0x00, 0x00 },
    { 0x25, 0x00, 0x00, 0x00, 0x00 },
    { 0x24, 0x00, 0x00, 0x00, 0x00 },
    { 0x4A, 0x00, 0x00, 0x00, 0x00 },
    { 0x7F, 0x00, 0x00, 0x00, 0x00 },
    { 0x7F, 0x00, 0x00, 0x00, 0x00 },
    { 0x5F, 0x00, 0x00, 0x00, 0x00 },
    { 0x50, 0x00, 0x00, 0x00, 0x00 },
    { 0x49, 0x00, 0x00, 0x00, 0x00 },
    { 0x47, 0x00, 0x00, 0x00, 0x00 },
    { 0x47, 0x00, 0x00, 0x00, 0x00 },
    { 0x46, 0x00, 0x00, 0x00, 0x00 }
};
TX_WAVEFORM pmc4_t1_tx_waveform_sh_660ft = 
{
    { 0x00, 0x46, 0x00, 0x00, 0x00 },
    { 0x0A, 0x45, 0x00, 0x00, 0x00 },
    { 0x3F, 0x43, 0x00, 0x00, 0x00 },
    { 0x3F, 0x41, 0x00, 0x00, 0x00 },
    { 0x3F, 0x40, 0x00, 0x00, 0x00 },
    { 0x3F, 0x40, 0x00, 0x00, 0x00 },
    { 0x2E, 0x00, 0x00, 0x00, 0x00 },
    { 0x2E, 0x00, 0x00, 0x00, 0x00 },
    { 0x2A, 0x00, 0x00, 0x00, 0x00 },
    { 0x29, 0x00, 0x00, 0x00, 0x00 },
    { 0x28, 0x00, 0x00, 0x00, 0x00 },
    { 0x27, 0x00, 0x00, 0x00, 0x00 },
    { 0x26, 0x00, 0x00, 0x00, 0x00 },
    { 0x25, 0x00, 0x00, 0x00, 0x00 },
    { 0x24, 0x00, 0x00, 0x00, 0x00 },
    { 0x4A, 0x00, 0x00, 0x00, 0x00 },
    { 0x7F, 0x00, 0x00, 0x00, 0x00 },
    { 0x63, 0x00, 0x00, 0x00, 0x00 },
    { 0x53, 0x00, 0x00, 0x00, 0x00 },
    { 0x51, 0x00, 0x00, 0x00, 0x00 },
    { 0x4C, 0x00, 0x00, 0x00, 0x00 },
    { 0x4B, 0x00, 0x00, 0x00, 0x00 },
    { 0x48, 0x00, 0x00, 0x00, 0x00 },
    { 0x47, 0x00, 0x00, 0x00, 0x00 }
};



/* Transmit Waveform Values for E1 120 Ohm
 * unsigned char e1_tx_waveform_120[LAST_SAMPLE-FIRST_SAMPLE+1][LAST_UI-FIRST_UI+1] = 
 */
TX_WAVEFORM pmc_e1_tx_waveform_120 = 
{
    { 0x00, 0x00, 0x00, 0x00, 0x00 },
    { 0x00, 0x00, 0x00, 0x00, 0x00 },
    { 0x0A, 0x00, 0x00, 0x00, 0x00 },
    { 0x3F, 0x00, 0x00, 0x00, 0x00 },
    { 0x3F, 0x00, 0x00, 0x00, 0x00 },
    { 0x39, 0x00, 0x00, 0x00, 0x00 },
    { 0x38, 0x00, 0x00, 0x00, 0x00 },
    { 0x36, 0x00, 0x00, 0x00, 0x00 },
    { 0x36, 0x00, 0x00, 0x00, 0x00 },
    { 0x35, 0x00, 0x00, 0x00, 0x00 },
    { 0x35, 0x00, 0x00, 0x00, 0x00 },
    { 0x35, 0x00, 0x00, 0x00, 0x00 },
    { 0x35, 0x00, 0x00, 0x00, 0x00 },
    { 0x35, 0x00, 0x00, 0x00, 0x00 },
    { 0x35, 0x00, 0x00, 0x00, 0x00 },
    { 0x2D, 0x00, 0x00, 0x00, 0x00 },
    { 0x00, 0x00, 0x00, 0x00, 0x00 },
    { 0x00, 0x00, 0x00, 0x00, 0x00 },
    { 0x00, 0x00, 0x00, 0x00, 0x00 },
    { 0x00, 0x00, 0x00, 0x00, 0x00 },
    { 0x00, 0x00, 0x00, 0x00, 0x00 },
    { 0x00, 0x00, 0x00, 0x00, 0x00 },
    { 0x00, 0x00, 0x00, 0x00, 0x00 },
    { 0x00, 0x00, 0x00, 0x00, 0x00 }
};
TX_WAVEFORM pmc4_e1_tx_waveform_120 = 
{
    { 0x00, 0x00, 0x00, 0x00, 0x00 },
    { 0x00, 0x00, 0x00, 0x00, 0x00 },
    { 0x0A, 0x00, 0x00, 0x00, 0x00 },
    { 0x3F, 0x00, 0x00, 0x00, 0x00 },
    { 0x3F, 0x00, 0x00, 0x00, 0x00 },
    { 0x39, 0x00, 0x00, 0x00, 0x00 },
    { 0x38, 0x00, 0x00, 0x00, 0x00 },
    { 0x36, 0x00, 0x00, 0x00, 0x00 },
    { 0x36, 0x00, 0x00, 0x00, 0x00 },
    { 0x35, 0x00, 0x00, 0x00, 0x00 },
    { 0x35, 0x00, 0x00, 0x00, 0x00 },
    { 0x35, 0x00, 0x00, 0x00, 0x00 },
    { 0x35, 0x00, 0x00, 0x00, 0x00 },
    { 0x35, 0x00, 0x00, 0x00, 0x00 },
    { 0x35, 0x00, 0x00, 0x00, 0x00 },
    { 0x2D, 0x00, 0x00, 0x00, 0x00 },
    { 0x00, 0x00, 0x00, 0x00, 0x00 },
    { 0x00, 0x00, 0x00, 0x00, 0x00 },
    { 0x00, 0x00, 0x00, 0x00, 0x00 },
    { 0x00, 0x00, 0x00, 0x00, 0x00 },
    { 0x00, 0x00, 0x00, 0x00, 0x00 },
    { 0x00, 0x00, 0x00, 0x00, 0x00 },
    { 0x00, 0x00, 0x00, 0x00, 0x00 },
    { 0x00, 0x00, 0x00, 0x00, 0x00 }
};


/* Transmit Waveform Values for E1 75 Ohm
 * unsigned char e1_tx_waveform_75[LAST_SAMPLE-FIRST_SAMPLE+1][LAST_UI-FIRST_UI+1] = 
 */
TX_WAVEFORM pmc_e1_tx_waveform_75 = 
{
    { 0x00, 0x00, 0x00, 0x00, 0x00 },
    { 0x00, 0x00, 0x00, 0x00, 0x00 },
    { 0x0A, 0x00, 0x00, 0x00, 0x00 },
    { 0x28, 0x00, 0x00, 0x00, 0x00 },
    { 0x3A, 0x00, 0x00, 0x00, 0x00 },
    { 0x3A, 0x00, 0x00, 0x00, 0x00 },
    { 0x3A, 0x00, 0x00, 0x00, 0x00 },
    { 0x3A, 0x00, 0x00, 0x00, 0x00 },
    { 0x3A, 0x00, 0x00, 0x00, 0x00 },
    { 0x3A, 0x00, 0x00, 0x00, 0x00 },
    { 0x3A, 0x00, 0x00, 0x00, 0x00 },
    { 0x3A, 0x00, 0x00, 0x00, 0x00 },
    { 0x3A, 0x00, 0x00, 0x00, 0x00 },
    { 0x3A, 0x00, 0x00, 0x00, 0x00 },
    { 0x32, 0x00, 0x00, 0x00, 0x00 },
    { 0x14, 0x00, 0x00, 0x00, 0x00 },
    { 0x00, 0x00, 0x00, 0x00, 0x00 },
    { 0x00, 0x00, 0x00, 0x00, 0x00 },
    { 0x00, 0x00, 0x00, 0x00, 0x00 },
    { 0x00, 0x00, 0x00, 0x00, 0x00 },
    { 0x00, 0x00, 0x00, 0x00, 0x00 },
    { 0x00, 0x00, 0x00, 0x00, 0x00 },
    { 0x00, 0x00, 0x00, 0x00, 0x00 },
    { 0x00, 0x00, 0x00, 0x00, 0x00 }
};
TX_WAVEFORM pmc4_e1_tx_waveform_75 = 
{
    { 0x00, 0x00, 0x00, 0x00, 0x00 },
    { 0x00, 0x00, 0x00, 0x00, 0x00 },
    { 0x0A, 0x00, 0x00, 0x00, 0x00 },
    { 0x3E, 0x00, 0x00, 0x00, 0x00 },
    { 0x3E, 0x00, 0x00, 0x00, 0x00 },
    { 0x3E, 0x00, 0x00, 0x00, 0x00 },
    { 0x3C, 0x00, 0x00, 0x00, 0x00 },
    { 0x3C, 0x00, 0x00, 0x00, 0x00 },
    { 0x3A, 0x00, 0x00, 0x00, 0x00 },
    { 0x3A, 0x00, 0x00, 0x00, 0x00 },
    { 0x3A, 0x00, 0x00, 0x00, 0x00 },
    { 0x3A, 0x00, 0x00, 0x00, 0x00 },
    { 0x3A, 0x00, 0x00, 0x00, 0x00 },
    { 0x3A, 0x00, 0x00, 0x00, 0x00 },
    { 0x35, 0x00, 0x00, 0x00, 0x00 },
    { 0x00, 0x00, 0x00, 0x00, 0x00 },
    { 0x00, 0x00, 0x00, 0x00, 0x00 },
    { 0x00, 0x00, 0x00, 0x00, 0x00 },
    { 0x00, 0x00, 0x00, 0x00, 0x00 },
    { 0x00, 0x00, 0x00, 0x00, 0x00 },
    { 0x00, 0x00, 0x00, 0x00, 0x00 },
    { 0x00, 0x00, 0x00, 0x00, 0x00 },
    { 0x00, 0x00, 0x00, 0x00, 0x00 },
    { 0x00, 0x00, 0x00, 0x00, 0x00 }
};


RLPS_EQUALIZER_RAM pmc_t1_rlps_ram_table[] = 
{
    { 0x03, 0xFE, 0x18, 0x40 }, /* 0d */ 
    { 0x03, 0xF6, 0x18, 0x40 }, /* 1d */ 
    { 0x03, 0xEE, 0x18, 0x40 }, /* 2d */ 
    { 0x03, 0xE6, 0x18, 0x40 }, /* 3d */ 
    { 0x03, 0xDE, 0x18, 0x40 }, /* 4d */ 
    { 0x03, 0xD6, 0x18, 0x40 }, /* 5d */ 
    { 0x03, 0xD6, 0x18, 0x40 }, /* 6d */ 
    { 0x03, 0xD6, 0x18, 0x40 }, /* 7d */ 
    { 0x03, 0xCE, 0x18, 0x40 }, /* 8d */ 
    { 0x03, 0xCE, 0x18, 0x40 }, /* 9d */ 
    { 0x03, 0xCE, 0x18, 0x40 }, /* 10d */ 
    { 0x03, 0xCE, 0x18, 0x40 }, /* 11d */ 
    { 0x03, 0xC6, 0x18, 0x40 }, /* 12d */ 
    { 0x03, 0xC6, 0x18, 0x40 }, /* 13d */ 
    { 0x03, 0xC6, 0x18, 0x40 }, /* 14d */ 
    { 0x0B, 0xBE, 0x18, 0x40 }, /* 15d */ 
    { 0x0B, 0xBE, 0x18, 0x40 }, /* 16d */ 
    { 0x0B, 0xBE, 0x18, 0x40 }, /* 17d */ 
    { 0x0B, 0xBE, 0x18, 0x40 }, /* 18d */ 
    { 0x0B, 0xB6, 0x18, 0x40 }, /* 19d */ 
    { 0x0B, 0xB6, 0x18, 0x40 }, /* 20d */ 
    { 0x0B, 0xB6, 0x18, 0x40 }, /* 21d */ 
    { 0x0B, 0xB6, 0x18, 0x40 }, /* 22d */ 
    { 0x13, 0xAE, 0x18, 0x38 }, /* 23d */ 
    { 0x13, 0xAE, 0x18, 0x3C }, /* 24d */ 
    { 0x13, 0xAE, 0x18, 0x40 }, /* 25d */ 
    { 0x13, 0xAE, 0x18, 0x40 }, /* 26d */ 
    { 0x13, 0xAE, 0x18, 0x40 }, /* 27d */ 
    { 0x13, 0xAE, 0x18, 0x40 }, /* 28d */ 
    { 0x1B, 0xB6, 0x18, 0xB8 }, /* 29d */ 
    { 0x1B, 0xAE, 0x18, 0xB8 }, /* 30d */ 
    { 0x1B, 0xAE, 0x18, 0xBC }, /* 31d */ 
    { 0x1B, 0xAE, 0x18, 0xC0 }, /* 32d */ 
    { 0x1B, 0xAE, 0x18, 0xC0 }, /* 33d */ 
    { 0x23, 0xA6, 0x18, 0xC0 }, /* 34d */ 
    { 0x23, 0xA6, 0x18, 0xC0 }, /* 35d */ 
    { 0x23, 0xA6, 0x18, 0xC0 }, /* 36d */ 
    { 0x23, 0xA6, 0x18, 0xC0 }, /* 37d */ 
    { 0x23, 0xA6, 0x18, 0xC0 }, /* 38d */ 
    { 0x23, 0x9E, 0x18, 0xC0 }, /* 39d */ 
    { 0x23, 0x9E, 0x18, 0xC0 }, /* 40d */ 
    { 0x23, 0x9E, 0x18, 0xC0 }, /* 41d */ 
    { 0x23, 0x9E, 0x18, 0xC0 }, /* 42d */ 
    { 0x23, 0x9E, 0x18, 0xC0 }, /* 43d */ 
    { 0x2B, 0x96, 0x18, 0xC0 }, /* 44d */ 
    { 0x2B, 0x96, 0x18, 0xC0 }, /* 45d */ 
    { 0x2B, 0x96, 0x18, 0xC0 }, /* 46d */ 
    { 0x33, 0x96, 0x19, 0x40 }, /* 47d */ 
    { 0x37, 0x96, 0x19, 0x40 }, /* 48d */ 
    { 0x37, 0x96, 0x19, 0x40 }, /* 49d */ 
    { 0x37, 0x96, 0x19, 0x40 }, /* 50d */ 
    { 0x3F, 0x9E, 0x19, 0xC0 }, /* 51d */ 
    { 0x3F, 0x9E, 0x19, 0xC0 }, /* 52d */ 
    { 0x3F, 0x9E, 0x19, 0xC0 }, /* 53d */ 
    { 0x3F, 0xA6, 0x1A, 0x40 }, /* 54d */ 
    { 0x3F, 0xA6, 0x1A, 0x40 }, /* 55d */ 
    { 0x3F, 0xA6, 0x1A, 0x40 }, /* 56d */
    { 0x3F, 0xA6, 0x1A, 0x40 }, /* 57d */ 
    { 0x3F, 0x96, 0x19, 0xC0 }, /* 58d */ 
    { 0x3F, 0x96, 0x19, 0xC0 }, /* 59d */ 
    { 0x3F, 0x96, 0x19, 0xC0 }, /* 60d */ 
    { 0x3F, 0x96, 0x19, 0xC0 }, /* 61d */ 
    { 0x47, 0x9E, 0x1A, 0x40 }, /* 62d */ 
    { 0x47, 0x9E, 0x1A, 0x40 }, /* 63d */ 
    { 0x47, 0x9E, 0x1A, 0x40 }, /* 64d */ 
    { 0x47, 0x96, 0x1A, 0x40 }, /* 65d */ 
    { 0x47, 0x96, 0x1A, 0x40 }, /* 66d */ 
    { 0x47, 0x96, 0x1A, 0x40 }, /* 67d */ 
    { 0x47, 0x96, 0x1A, 0x40 }, /* 68d */ 
    { 0x4F, 0x8E, 0x1A, 0x40 }, /* 69d */ 
    { 0x4F, 0x8E, 0x1A, 0x40 }, /* 70d */ 
    { 0x4F, 0x8E, 0x1A, 0x40 }, /* 71d */ 
    { 0x4F, 0x8E, 0x1A, 0x40 }, /* 72d */ 
    { 0x4F, 0x8E, 0x1A, 0x40 }, /* 73d */ 
    { 0x57, 0x86, 0x1A, 0x40 }, /* 74d */ 
    { 0x57, 0x86, 0x1A, 0x40 }, /* 75d */ 
    { 0x57, 0x86, 0x1A, 0x40 }, /* 76d */ 
    { 0x57, 0x86, 0x1A, 0x40 }, /* 77d */ 
    { 0x57, 0x86, 0x1A, 0x40 }, /* 78d */ 
    { 0x5F, 0x86, 0x1A, 0xC0 }, /* 79d */ 
    { 0x5F, 0x86, 0x1A, 0xC0 }, /* 80d */ 
    { 0x5F, 0x86, 0x1A, 0xC0 }, /* 81d */ 
    { 0x5F, 0x86, 0x1A, 0xC0 }, /* 82d */ 
    { 0x5F, 0x86, 0x1A, 0xC0 }, /* 83d */ 
    { 0x5F, 0x86, 0x1A, 0xC0 }, /* 84d */ 
    { 0x5F, 0x7E, 0x1A, 0xC0 }, /* 85d */ 
    { 0x5F, 0x7E, 0x1A, 0xC0 }, /* 86d */ 
    { 0x5F, 0x7E, 0x1A, 0xC0 }, /* 87d */ 
    { 0x5F, 0x7E, 0x1A, 0xC0 }, /* 88d */ 
    { 0x5F, 0x7E, 0x1A, 0xC0 }, /* 89d */ 
    { 0x67, 0x7E, 0x2A, 0xC0 }, /* 90d */ 
    { 0x67, 0x7E, 0x2A, 0xC0 }, /* 91d */ 
    { 0x67, 0x7E, 0x2A, 0xC0 }, /* 92d */ 
    { 0x67, 0x7E, 0x2A, 0xC0 }, /* 93d */ 
    { 0x67, 0x76, 0x2A, 0xC0 }, /* 94d */ 
    { 0x67, 0x76, 0x2A, 0xC0 }, /* 95d */ 
    { 0x67, 0x76, 0x2A, 0xC0 }, /* 96d */ 
    { 0x67, 0x76, 0x2A, 0xC0 }, /* 97d */ 
    { 0x67, 0x76, 0x2A, 0xC0 }, /* 98d */ 
    { 0x6F, 0x6E, 0x2A, 0xC0 }, /* 99d */ 
    { 0x6F, 0x6E, 0x2A, 0xC0 }, /* 100d */ 
    { 0x6F, 0x6E, 0x2A, 0xC0 }, /* 101d */ 
    { 0x6F, 0x6E, 0x2A, 0xC0 }, /* 102d */ 
    { 0x77, 0x6E, 0x3A, 0xC0 }, /* 103d */ 
    { 0x77, 0x6E, 0x3A, 0xC0 }, /* 104d */ 
    { 0x77, 0x6E, 0x3A, 0xC0 }, /* 105d */ 
    { 0x77, 0x6E, 0x3A, 0xC0 }, /* 106d */ 
    { 0x7F, 0x66, 0x3A, 0xC0 }, /* 107d */ 
    { 0x7F, 0x66, 0x3A, 0xC0 }, /* 108d */ 
    { 0x7F, 0x66, 0x4A, 0xC0 }, /* 109d */ 
    { 0x7F, 0x66, 0x4A, 0xC0 }, /* 110d */ 
    { 0x7F, 0x66, 0x4A, 0xC0 }, /* 111d */ 
    { 0x7F, 0x66, 0x4A, 0xC0 }, /* 112d */ 
    { 0x87, 0x66, 0x5A, 0xC0 }, /* 113d */ 
    { 0x87, 0x66, 0x5A, 0xC0 }, /* 114d */ 
    { 0x87, 0x66, 0x5A, 0xC0 }, /* 115d */ 
    { 0x87, 0x66, 0x5A, 0xC0 }, /* 116d */ 
    { 0x87, 0x66, 0x5A, 0xC0 }, /* 117d */ 
    { 0x87, 0x5E, 0x5A, 0xC0 }, /* 118d */ 
    { 0x87, 0x5E, 0x5A, 0xC0 }, /* 119d */ 
    { 0x87, 0x5E, 0x5A, 0xC0 }, /* 120d */ 
    { 0x87, 0x5E, 0x5A, 0xC0 }, /* 121d */ 
    { 0x87, 0x5E, 0x5A, 0xC0 }, /* 122d */ 
    { 0x8F, 0x5E, 0x6A, 0xC0 }, /* 123d */ 
    { 0x8F, 0x5E, 0x6A, 0xC0 }, /* 124d */ 
    { 0x8F, 0x5E, 0x6A, 0xC0 }, /* 125d */ 
    { 0x8F, 0x5E, 0x6A, 0xC0 }, /* 126d */ 
    { 0x97, 0x5E, 0x7A, 0xC0 }, /* 127d */ 
    { 0x97, 0x5E, 0x7A, 0xC0 }, /* 128d */ 
    { 0x97, 0x5E, 0x7A, 0xC0 }, /* 129d */ 
    { 0x97, 0x5E, 0x7A, 0xC0 }, /* 130d */ 
    { 0x9F, 0x5E, 0x8A, 0xC0 }, /* 131d */ 
    { 0x9F, 0x5E, 0x8A, 0xC0 }, /* 132d */ 
    { 0x9F, 0x5E, 0x8A, 0xC0 }, /* 133d */ 
    { 0x9F, 0x5E, 0x8A, 0xC0 }, /* 134d */ 
    { 0x9F, 0x5E, 0x8A, 0xC0 }, /* 135d */ 
    { 0xA7, 0x56, 0x9A, 0xC0 }, /* 136d */ 
    { 0xA7, 0x56, 0x9A, 0xC0 }, /* 137d */ 
    { 0xA7, 0x56, 0x9A, 0xC0 }, /* 138d */ 
    { 0xA7, 0x56, 0x9A, 0xC0 }, /* 139d */ 
    { 0xA7, 0x56, 0xAA, 0xC0 }, /* 140d */ 
    { 0xA7, 0x56, 0xAA, 0xC0 }, /* 141d */ 
    { 0xA7, 0x56, 0xAA, 0xC0 }, /* 142d */ 
    { 0xAF, 0x4E, 0xAA, 0xC0 }, /* 143d */ 
    { 0xAF, 0x4E, 0xAA, 0xC0 }, /* 144d */ 
    { 0xAF, 0x4E, 0xAA, 0xC0 }, /* 145d */ 
    { 0xAF, 0x4E, 0xAA, 0xC0 }, /* 146d */ 
    { 0xAF, 0x4E, 0xAA, 0xC0 }, /* 147d */ 
    { 0xB7, 0x46, 0xAA, 0xC0 }, /* 148d */ 
    { 0xB7, 0x46, 0xAA, 0xC0 }, /* 149d */ 
    { 0xB7, 0x46, 0xAA, 0xC0 }, /* 150d */ 
    { 0xB7, 0x46, 0xAA, 0xC0 }, /* 151d */ 
    { 0xB7, 0x46, 0xAA, 0xC0 }, /* 152d */ 
    { 0xB7, 0x46, 0xAA, 0xC0 }, /* 153d */ 
    { 0xB7, 0x46, 0xAA, 0xC0 }, /* 154d */ 
    { 0xB7, 0x46, 0xBA, 0xC0 }, /* 155d */ 
    { 0xB7, 0x46, 0xBA, 0xC0 }, /* 156d */ 
    { 0xB7, 0x46, 0xBA, 0xC0 }, /* 157d */ 
    { 0xBF, 0x4E, 0xBB, 0x40 }, /* 158d */ 
    { 0xBF, 0x4E, 0xBB, 0x40 }, /* 159d */ 
    { 0xBF, 0x4E, 0xBB, 0x40 }, /* 160d */ 
    { 0xBF, 0x4E, 0xBB, 0x40 }, /* 161d */ 
    { 0xBF, 0x4E, 0xBB, 0x40 }, /* 162d */ 
    { 0xBF, 0x4E, 0xBB, 0x40 }, /* 163d */ 
    { 0xBF, 0x4E, 0xBB, 0x40 }, /* 164d */ 
    { 0xBF, 0x4E, 0xBB, 0x40 }, /* 165d */ 
    { 0xBF, 0x4E, 0xBB, 0x40 }, /* 166d */ 
    { 0xBE, 0x46, 0xCB, 0x40 }, /* 167d */ 
    { 0xBE, 0x46, 0xCB, 0x40 }, /* 168d */ 
    { 0xBE, 0x46, 0xCB, 0x40 }, /* 169d */ 
    { 0xBE, 0x46, 0xCB, 0x40 }, /* 170d */ 
    { 0xBE, 0x46, 0xCB, 0x40 }, /* 171d */ 
    { 0xBE, 0x46, 0xCB, 0x40 }, /* 172d */ 
    { 0xBE, 0x46, 0xDB, 0x40 }, /* 173d */ 
    { 0xBE, 0x46, 0xDB, 0x40 }, /* 174d */ 
    { 0xBE, 0x46, 0xDB, 0x40 }, /* 175d */ 
    { 0xC6, 0x3E, 0xCB, 0x40 }, /* 176d */ 
    { 0xC6, 0x3E, 0xCB, 0x40 }, /* 177d */ 
    { 0xC6, 0x3E, 0xDB, 0x40 }, /* 178d */ 
    { 0xC6, 0x3E, 0xDB, 0x40 }, /* 179d */ 
    { 0xC6, 0x3E, 0xDB, 0x40 }, /* 180d */ 
    { 0xC6, 0x44, 0xDB, 0x40 }, /* 181d */ 
    { 0xC6, 0x44, 0xDB, 0x40 }, /* 182d */ 
    { 0xC6, 0x44, 0xDB, 0x40 }, /* 183d */ 
    { 0xC6, 0x44, 0xDB, 0x40 }, /* 184d */ 
    { 0xC6, 0x3C, 0xDB, 0x40 }, /* 185d */ 
    { 0xC6, 0x3C, 0xDB, 0x40 }, /* 186d */ 
    { 0xC6, 0x3C, 0xDB, 0x40 }, /* 187d */ 
    { 0xC6, 0x3C, 0xDB, 0x40 }, /* 188d */ 
    { 0xD6, 0x34, 0xDB, 0x40 }, /* 189d */ 
    { 0xD6, 0x34, 0xDB, 0x40 }, /* 190d */ 
    { 0xD6, 0x34, 0xDB, 0x40 }, /* 191d */ 
    { 0xD6, 0x34, 0xDB, 0x40 }, /* 192d */ 
    { 0xD6, 0x34, 0xDB, 0x40 }, /* 193d */ 
    { 0xDE, 0x2C, 0xDB, 0x3C }, /* 194d */ 
    { 0xDE, 0x2C, 0xDB, 0x3C }, /* 195d */ 
    { 0xDE, 0x2C, 0xDB, 0x3C }, /* 196d */ 
    { 0xE6, 0x2C, 0xDB, 0x40 }, /* 197d */ 
    { 0xE6, 0x2C, 0xDB, 0x40 }, /* 198d */ 
    { 0xE6, 0x2C, 0xDB, 0x40 }, /* 199d */ 
    { 0xE6, 0x2C, 0xDB, 0x40 }, /* 200d */ 
    { 0xE6, 0x2C, 0xDB, 0x40 }, /* 201d */ 
    { 0xE6, 0x2C, 0xEB, 0x40 }, /* 202d */ 
    { 0xE6, 0x2C, 0xEB, 0x40 }, /* 203d */ 
    { 0xE6, 0x2C, 0xEB, 0x40 }, /* 204d */ 
    { 0xEE, 0x2C, 0xFB, 0x40 }, /* 205d */ 
    { 0xEE, 0x2C, 0xFB, 0x40 }, /* 206d */ 
    { 0xEE, 0x2C, 0xFB, 0x40 }, /* 207d */ 
    { 0xEE, 0x2D, 0x0B, 0x40 }, /* 208d */ 
    { 0xEE, 0x2D, 0x0B, 0x40 }, /* 209d */ 
    { 0xEE, 0x2D, 0x0B, 0x40 }, /* 210d */ 
    { 0xEE, 0x2D, 0x0B, 0x40 }, /* 211d */ 
    { 0xEE, 0x2D, 0x0B, 0x40 }, /* 212d */ 
    { 0xF5, 0x25, 0x0B, 0x38 }, /* 213d */ 
    { 0xF5, 0x25, 0x0B, 0x3C }, /* 214d */ 
    { 0xF5, 0x25, 0x0B, 0x40 }, /* 215d */ 
    { 0xF5, 0x25, 0x1B, 0x40 }, /* 216d */ 
    { 0xF5, 0x25, 0x1B, 0x40 }, /* 217d */ 
    { 0xF5, 0x25, 0x1B, 0x40 }, /* 218d */ 
    { 0xF5, 0x25, 0x1B, 0x40 }, /* 219d */ 
    { 0xF5, 0x25, 0x1B, 0x40 }, /* 220d */ 
    { 0xFD, 0x25, 0x2B, 0x40 }, /* 221d */ 
    { 0xFD, 0x25, 0x2B, 0x40 }, /* 222d */ 
    { 0xFD, 0x25, 0x2B, 0x40 }, /* 223d */ 
    { 0xFD, 0x25, 0x2B, 0x40 }, /* 224d */ 
    { 0xFD, 0x25, 0x27, 0x40 }, /* 225d */ 
    { 0xFD, 0x25, 0x27, 0x40 }, /* 226d */ 
    { 0xFD, 0x25, 0x27, 0x40 }, /* 227d */ 
    { 0xFD, 0x25, 0x23, 0x40 }, /* 228d */ 
    { 0xFD, 0x25, 0x23, 0x40 }, /* 229d */ 
    { 0xFD, 0x25, 0x23, 0x40 }, /* 230d */ 
    { 0xFD, 0x25, 0x33, 0x40 }, /* 231d */ 
    { 0xFD, 0x25, 0x33, 0x40 }, /* 232d */ 
    { 0xFD, 0x25, 0x33, 0x40 }, /* 233d */ 
    { 0xFD, 0x25, 0x33, 0x40 }, /* 234d */ 
    { 0xFD, 0x25, 0x33, 0x40 }, /* 235d */ 
    { 0xFD, 0x25, 0x33, 0x40 }, /* 236d */ 
    { 0xFC, 0x25, 0x33, 0x40 }, /* 237d */ 
    { 0xFC, 0x25, 0x33, 0x40 }, /* 238d */ 
    { 0xFC, 0x25, 0x43, 0x40 }, /* 239d */ 
    { 0xFC, 0x25, 0x43, 0x40 }, /* 240d */ 
    { 0xFC, 0x25, 0x43, 0x40 }, /* 241d */ 
    { 0xFC, 0x25, 0x43, 0x44 }, /* 242d */ 
    { 0xFC, 0x25, 0x43, 0x48 }, /* 243d */ 
    { 0xFC, 0x25, 0x43, 0x4C }, /* 244d */ 
    { 0xFC, 0x25, 0x43, 0xBC }, /* 245d */ 
    { 0xFC, 0x25, 0x43, 0xC0 }, /* 246d */ 
    { 0xFC, 0x25, 0x43, 0xC0 }, /* 247d */ 
    { 0xFC, 0x23, 0x43, 0xC0 }, /* 248d */ 
    { 0xFC, 0x23, 0x43, 0xC0 }, /* 249d */ 
    { 0xFC, 0x23, 0x43, 0xC0 }, /* 250d */ 
    { 0xFC, 0x21, 0x43, 0xC0 }, /* 251d */ 
    { 0xFC, 0x21, 0x43, 0xC0 }, /* 252d */ 
    { 0xFC, 0x21, 0x53, 0xC0 }, /* 253d */ 
    { 0xFC, 0x21, 0x53, 0xC0 }, /* 254d */ 
    { 0xFC, 0x21, 0x53, 0xC0 }  /* 255d */ 
};
RLPS_EQUALIZER_RAM pmc4_t1_rlps_ram_table[] = 
{
    { 0x03, 0xFE, 0x18, 0x40 },	/* 0d */
    { 0x03, 0xFE, 0x18, 0x40 }, /* 1d */
    { 0x03, 0xF6, 0x18, 0x40 }, /* 2d */
    { 0x03, 0xF6, 0x18, 0x40 }, /* 3d */ 
    { 0x03, 0xEE, 0x18, 0x40 }, /* 4d */ 
    { 0x03, 0xEE, 0x18, 0x40 }, /* 5d */ 
    { 0x03, 0xE6, 0x18, 0x40 }, /* 6d */ 
    { 0x03, 0xE6, 0x18, 0x40 }, /* 7d */ 
    { 0x03, 0xDE, 0x18, 0x40 }, /* 8d */ 
    { 0x0B, 0xDE, 0x18, 0x40 }, /* 9d */ 
    { 0x0B, 0xD6, 0x18, 0x40 }, /* 10d */ 
    { 0x0B, 0xD6, 0x18, 0x40 }, /* 11d */ 
    { 0x0B, 0xCE, 0x18, 0x40 }, /* 12d */ 
    { 0x0B, 0xCE, 0x18, 0x40 }, /* 13d */ 
    { 0x0B, 0xC6, 0x18, 0x40 }, /* 14d */ 
    { 0x0B, 0xC6, 0x18, 0x40 }, /* 15d */ 
    { 0x0B, 0xBE, 0x18, 0x40 }, /* 16d */ 
    { 0x0B, 0xBE, 0x18, 0x40 }, /* 17d */ 
    { 0x0B, 0xB6, 0x18, 0x40 }, /* 18d */ 
    { 0x0B, 0xB6, 0x18, 0x40 }, /* 19d */ 
    { 0x0B, 0xAE, 0x18, 0x40 }, /* 20d */ 
    { 0x0B, 0xAE, 0x18, 0x40 }, /* 21d */ 
    { 0x13, 0xAE, 0x18, 0x40 }, /* 22d */ 
    { 0x13, 0xAE, 0x18, 0x40 }, /* 23d */ 
    { 0x13, 0xA6, 0x18, 0x40 }, /* 24d */ 
    { 0x13, 0xA6, 0x28, 0x40 }, /* 25d */ 
    { 0x13, 0xA6, 0x28, 0x40 }, /* 26d */ 
    { 0x13, 0xA6, 0x28, 0x40 }, /* 27d */ 
    { 0x1B, 0xA6, 0x28, 0x40 }, /* 28d */ 
    { 0x1B, 0xA6, 0x28, 0x40 }, /* 29d */ 
    { 0x1B, 0x9E, 0x28, 0x40 }, /* 30d */ 
    { 0x1B, 0x9E, 0x38, 0x40 }, /* 31d */ 
    { 0x1B, 0x9E, 0x38, 0x40 }, /* 32d */ 
    { 0x1B, 0x9E, 0x38, 0x40 }, /* 33d */ 
    { 0x23, 0x96, 0x38, 0x40 }, /* 34d */ 
    { 0x23, 0x96, 0x38, 0x40 }, /* 35d */ 
    { 0x23, 0x96, 0x38, 0x40 }, /* 36d */ 
    { 0x23, 0x96, 0x48, 0x40 }, /* 37d */ 
    { 0x23, 0x96, 0x48, 0x40 }, /* 38d */ 
    { 0x23, 0x96, 0x48, 0x40 }, /* 39d */ 
    { 0x23, 0x96, 0x58, 0x40 }, /* 40d */ 
    { 0x23, 0x96, 0x58, 0x40 }, /* 41d */ 
    { 0x23, 0x96, 0x58, 0x40 }, /* 42d */ 
    { 0x2B, 0x96, 0x38, 0xC0 }, /* 43d */ 
    { 0x2B, 0x96, 0x38, 0xC0 }, /* 44d */ 
    { 0x2B, 0x96, 0x38, 0xC0 }, /* 45d */ 
    { 0x33, 0x8E, 0x38, 0xC0 }, /* 46d */ 
    { 0x33, 0x8E, 0x38, 0xC0 }, /* 47d */ 
    { 0x33, 0x8E, 0x38, 0xC0 }, /* 48d */ 
    { 0x37, 0x8E, 0x48, 0xC0 }, /* 49d */ 
    { 0x37, 0x8E, 0x48, 0xC0 }, /* 50d */ 
    { 0x37, 0x86, 0x48, 0xC0 }, /* 51d */ 
    { 0x37, 0x86, 0x48, 0xC0 }, /* 52d */ 
    { 0x37, 0x86, 0x58, 0xC0 }, /* 53d */ 
    { 0x37, 0x86, 0x58, 0xC0 }, /* 54d */ 
    { 0x3F, 0x86, 0x54, 0xC0 }, /* 55d */ 
    { 0x3F, 0x86, 0x54, 0xC0 }, /* 56d */ 
    { 0x3F, 0x7E, 0x54, 0xC0 }, /* 57d */ 
    { 0x47, 0x7E, 0x54, 0xC0 }, /* 58d */ 
    { 0x47, 0x7E, 0x54, 0xC0 }, /* 59d */ 
    { 0x47, 0x76, 0x54, 0xC0 }, /* 60d */ 
    { 0x47, 0x76, 0x64, 0xC0 }, /* 61d */ 
    { 0x47, 0x76, 0x64, 0xC0 }, /* 62d */ 
    { 0x47, 0x76, 0x64, 0xC0 }, /* 63d */ 
    { 0x47, 0x76, 0x74, 0xC0 }, /* 64d */ 
    { 0x47, 0x76, 0x74, 0xC0 }, /* 65d */ 
    { 0x47, 0x76, 0x74, 0xC0 }, /* 66d */ 
    { 0x47, 0x76, 0x74, 0xC0 }, /* 67d */ 
    { 0x47, 0x76, 0x74, 0xC0 }, /* 68d */ 
    { 0x47, 0x76, 0x74, 0xC0 }, /* 69d */ 
    { 0x4F, 0x76, 0x65, 0x40 }, /* 70d */ 
    { 0x4F, 0x76, 0x65, 0x40 }, /* 71d */ 
    { 0x4F, 0x76, 0x65, 0x40 }, /* 72d */ 
    { 0x57, 0x76, 0x75, 0x40 }, /* 73d */ 
    { 0x57, 0x76, 0x75, 0x40 }, /* 74d */ 
    { 0x5F, 0x6E, 0x75, 0x40 }, /* 75d */ 
    { 0x5F, 0x6E, 0x75, 0x40 }, /* 76d */ 
    { 0x67, 0x6E, 0x85, 0x40 }, /* 77d */ 
    { 0x67, 0x6E, 0x85, 0x40 }, /* 78d */ 
    { 0x67, 0x6E, 0x85, 0x40 }, /* 79d */ 
    { 0x67, 0x6E, 0x95, 0x40 }, /* 80d */ 
    { 0x67, 0x6E, 0x95, 0x40 }, /* 81d */ 
    { 0x67, 0x6E, 0x95, 0x40 }, /* 82d */ 
    { 0x67, 0x66, 0x95, 0x40 }, /* 83d */ 
    { 0x67, 0x66, 0x95, 0x40 }, /* 84d */ 
    { 0x67, 0x66, 0xA5, 0x40 }, /* 85d */ 
    { 0x67, 0x66, 0xA5, 0x40 }, /* 86d */ 
    { 0x67, 0x5E, 0x95, 0x40 }, /* 87d */ 
    { 0x67, 0x5E, 0x95, 0x40 }, /* 88d */ 
    { 0x77, 0x5E, 0x75, 0xC0 }, /* 89d */ 
    { 0x77, 0x5E, 0x75, 0xC0 }, /* 90d */ 
    { 0x77, 0x5E, 0x85, 0xC0 }, /* 91d */ 
    { 0x77, 0x5E, 0x85, 0xC0 }, /* 92d */ 
    { 0x77, 0x5E, 0x95, 0xC0 }, /* 93d */ 
    { 0x77, 0x5E, 0x95, 0xC0 }, /* 94d */ 
    { 0x77, 0x5E, 0xA5, 0xC0 }, /* 95d */ 
    { 0x77, 0x5E, 0xA5, 0xC0 }, /* 96d */ 
    { 0x77, 0x5E, 0xB5, 0xC0 }, /* 97d */ 
    { 0x7F, 0x5E, 0xB5, 0xC0 }, /* 98d */ 
    { 0x7F, 0x5E, 0xC5, 0xC0 }, /* 99d */ 
    { 0x7F, 0x5E, 0xC5, 0xC0 }, /* 100d */ 
    { 0x7F, 0x5C, 0xA9, 0xC0 }, /* 101d */ 
    { 0x7F, 0x5C, 0xA9, 0xC0 }, /* 102d */ 
    { 0x7F, 0x5C, 0xA9, 0xC0 }, /* 103d */ 
    { 0x7F, 0x5C, 0xB9, 0xC0 }, /* 104d */ 
    { 0x7F, 0x5C, 0xB9, 0xC0 }, /* 105d */ 
    { 0x7F, 0x5C, 0xB9, 0xC0 }, /* 106d */ 
    { 0x7F, 0x5C, 0xA5, 0xC0 }, /* 107d */ 
    { 0x7F, 0x5C, 0xA5, 0xC0 }, /* 108d */ 
    { 0x7F, 0x5C, 0xA5, 0xC0 }, /* 109d */ 
    { 0x7F, 0x5C, 0xB5, 0xC0 }, /* 110d */ 
    { 0x7F, 0x54, 0xA5, 0xC0 }, /* 111d */ 
    { 0x7F, 0x54, 0xB5, 0xC0 }, /* 112d */ 
    { 0x7F, 0x54, 0xC5, 0xC0 }, /* 113d */ 
    { 0x7F, 0x54, 0xC5, 0xC0 }, /* 114d */ 
    { 0x7F, 0x54, 0xC5, 0xC0 }, /* 115d */ 
    { 0x7F, 0x54, 0xB1, 0xC0 }, /* 116d */ 
    { 0x7F, 0x54, 0xB1, 0xC0 }, /* 117d */ 
    { 0x7F, 0x54, 0xB1, 0xC0 }, /* 118d */ 
    { 0x7F, 0x54, 0xB1, 0xC0 }, /* 119d */ 
    { 0x7F, 0x54, 0xB1, 0xC0 }, /* 120d */ 
    { 0x86, 0x54, 0xD1, 0xC0 }, /* 121d */ 
    { 0x86, 0x54, 0xD1, 0xC0 }, /* 122d */ 
    { 0x86, 0x54, 0xD1, 0xC0 }, /* 123d */ 
    { 0x86, 0x54, 0xD1, 0xC0 }, /* 124d */ 
    { 0x86, 0x54, 0xD1, 0xC0 }, /* 125d */ 
    { 0x86, 0x4C, 0xC1, 0xC0 }, /* 126d */ 
    { 0x86, 0x4C, 0xC1, 0xC0 }, /* 127d */ 
    { 0x86, 0x4C, 0xD1, 0xC0 }, /* 128d */ 
    { 0x86, 0x4C, 0xD1, 0xC0 }, /* 129d */ 
    { 0x86, 0x4C, 0xD1, 0xC0 }, /* 130d */ 
    { 0x8E, 0x4C, 0xB2, 0x40 }, /* 131d */ 
    { 0x8E, 0x4C, 0xB2, 0x40 }, /* 132d */ 
    { 0x8E, 0x4C, 0xB2, 0x40 }, /* 133d */ 
    { 0x8E, 0x4C, 0xB2, 0x40 }, /* 134d */ 
    { 0x8E, 0x4C, 0xB2, 0x40 }, /* 135d */ 
    { 0x8E, 0x4C, 0xB2, 0x40 }, /* 136d */ 
    { 0x96, 0x4C, 0xC2, 0x40 }, /* 137d */ 
    { 0x96, 0x4C, 0xC2, 0x40 }, /* 138d */ 
    { 0x96, 0x4C, 0xC2, 0x40 }, /* 139d */ 
    { 0x9E, 0x4C, 0xD2, 0x40 }, /* 140d */ 
    { 0x9E, 0x4C, 0xD2, 0x40 }, /* 141d */ 
    { 0x9E, 0x4C, 0xD2, 0x40 }, /* 142d */ 
    { 0xA6, 0x4C, 0xD2, 0x40 }, /* 143d */ 
    { 0xA6, 0x4C, 0xD2, 0x40 }, /* 144d */ 
    { 0xA6, 0x4C, 0xD2, 0x40 }, /* 145d */ 
    { 0xA6, 0x4C, 0xE2, 0x40 }, /* 146d */ 
    { 0xA6, 0x4C, 0xE2, 0x40 }, /* 147d */ 
    { 0xA6, 0x4C, 0xE2, 0x40 }, /* 148d */ 
    { 0xA6, 0x4C, 0xF2, 0x40 }, /* 149d */ 
    { 0xA6, 0x4C, 0xF2, 0x40 }, /* 150d */ 
    { 0xA6, 0x4C, 0xF2, 0x40 }, /* 151d */ 
    { 0xA6, 0x4C, 0xF2, 0x40 }, /* 152d */ 
    { 0xA6, 0x4C, 0xF2, 0x40 }, /* 153d */ 
    { 0xA6, 0x4C, 0xF2, 0x40 }, /* 154d */ 
    { 0xB6, 0x4C, 0xE2, 0xC0 }, /* 155d */ 
    { 0xB6, 0x4C, 0xE2, 0xC0 }, /* 156d */ 
    { 0xB6, 0x4C, 0xE2, 0xC0 }, /* 157d */ 
    { 0xBE, 0x4C, 0xF2, 0xC0 }, /* 158d */ 
    { 0xBE, 0x4C, 0xF2, 0xC0 }, /* 159d */ 
    { 0xBE, 0x4C, 0xF2, 0xC0 }, /* 160d */ 
    { 0xBE, 0x4D, 0x02, 0xC0 }, /* 161d */ 
    { 0xBE, 0x4D, 0x02, 0xC0 }, /* 162d */ 
    { 0xBE, 0x4D, 0x02, 0xC0 }, /* 163d */ 
    { 0xBE, 0x4D, 0x12, 0xC0 }, /* 164d */ 
    { 0xBE, 0x4D, 0x12, 0xC0 }, /* 165d */ 
    { 0xBE, 0x4D, 0x12, 0xC0 }, /* 166d */ 
    { 0xC6, 0x4D, 0x12, 0xC0 }, /* 167d */ 
    { 0xC6, 0x4D, 0x12, 0xC0 }, /* 168d */ 
    { 0xC6, 0x4D, 0x12, 0xC0 }, /* 169d */ 
    { 0xC6, 0x4D, 0x12, 0xC0 }, /* 170d */ 
    { 0xC6, 0x4D, 0x12, 0xC0 }, /* 171d */ 
    { 0xC6, 0x4D, 0x12, 0xC0 }, /* 172d */ 
    { 0xCE, 0x4D, 0x22, 0xC0 }, /* 173d */ 
    { 0xCE, 0x4D, 0x22, 0xC0 }, /* 174d */ 
    { 0xCE, 0x4D, 0x22, 0xC0 }, /* 175d */ 
    { 0xCE, 0x4D, 0x22, 0xC0 }, /* 176d */ 
    { 0xCE, 0x4D, 0x22, 0xC0 }, /* 177d */ 
    { 0xCE, 0x4D, 0x22, 0xC0 }, /* 178d */ 
    { 0xCE, 0x4D, 0x22, 0xC0 }, /* 179d */ 
    { 0xCE, 0x4D, 0x22, 0xC0 }, /* 180d */ 
    { 0xCE, 0x4D, 0x22, 0xC0 }, /* 181d */ 
    { 0xCE, 0x4D, 0x32, 0xC0 }, /* 182d */ 
    { 0xCE, 0x4D, 0x32, 0xC0 }, /* 183d */ 
    { 0xCE, 0x4D, 0x32, 0xC0 }, /* 184d */ 
    { 0xCD, 0x4D, 0x22, 0xC0 }, /* 185d */ 
    { 0xCD, 0x4D, 0x22, 0xC0 }, /* 186d */ 
    { 0xCD, 0x4D, 0x22, 0xC0 }, /* 187d */ 
    { 0xD5, 0x4C, 0xE3, 0x40 }, /* 188d */ 
    { 0xD5, 0x4C, 0xE3, 0x40 }, /* 189d */ 
    { 0xD5, 0x4C, 0xF3, 0x40 }, /* 190d */ 
    { 0xD5, 0x4C, 0xF3, 0x40 }, /* 191d */ 
    { 0xD5, 0x4D, 0x03, 0x40 }, /* 192d */ 
    { 0xD5, 0x4D, 0x03, 0x40 }, /* 193d */ 
    { 0xD5, 0x4D, 0x13, 0x40 }, /* 194d */ 
    { 0xD5, 0x4D, 0x13, 0x40 }, /* 195d */ 
    { 0xD5, 0x4D, 0x23, 0x40 }, /* 196d */ 
    { 0xD5, 0x4D, 0x23, 0x40 }, /* 197d */ 
    { 0xD5, 0x4D, 0x33, 0x40 }, /* 198d */ 
    { 0xD5, 0x4D, 0x33, 0x40 }, /* 199d */ 
    { 0xDD, 0x45, 0x13, 0x40 }, /* 200d */ 
    { 0xDD, 0x45, 0x13, 0x40 }, /* 201d */ 
    { 0xDD, 0x45, 0x13, 0x40 }, /* 202d */ 
    { 0xDD, 0x45, 0x23, 0x40 }, /* 203d */ 
    { 0xDD, 0x45, 0x23, 0x40 }, /* 204d */ 
    { 0xDD, 0x45, 0x23, 0x40 }, /* 205d */ 
    { 0xDD, 0x45, 0x33, 0x40 }, /* 206d */ 
    { 0xDD, 0x45, 0x33, 0x40 }, /* 207d */ 
    { 0xDD, 0x45, 0x33, 0x40 }, /* 208d */ 
    { 0xE5, 0x3D, 0x23, 0x40 }, /* 209d */ 
    { 0xE5, 0x3D, 0x23, 0x40 }, /* 210d */ 
    { 0xE5, 0x3D, 0x23, 0x40 }, /* 211d */ 
    { 0xE5, 0x3D, 0x33, 0x40 }, /* 212d */ 
    { 0xE5, 0x3D, 0x33, 0x40 }, /* 213d */ 
    { 0xE5, 0x3D, 0x33, 0x40 }, /* 214d */ 
    { 0xE5, 0x3D, 0x43, 0x40 }, /* 215d */ 
    { 0xE5, 0x3D, 0x43, 0x40 }, /* 216d */ 
    { 0xE5, 0x3D, 0x43, 0x40 }, /* 217d */ 
    { 0xE5, 0x3D, 0x53, 0x40 }, /* 218d */ 
    { 0xE5, 0x3D, 0x53, 0x40 }, /* 219d */ 
    { 0xE5, 0x3D, 0x53, 0x40 }, /* 220d */ 
    { 0xEC, 0x35, 0x23, 0x40 }, /* 221d */ 
    { 0xEC, 0x35, 0x33, 0x40 }, /* 222d */ 
    { 0xEC, 0x35, 0x33, 0x40 }, /* 223d */ 
    { 0xEC, 0x35, 0x43, 0x40 }, /* 224d */ 
    { 0xEC, 0x35, 0x43, 0x40 }, /* 225d */ 
    { 0xEC, 0x35, 0x43, 0x40 }, /* 226d */ 
    { 0xEC, 0x35, 0x53, 0x40 }, /* 227d */ 
    { 0xEC, 0x35, 0x53, 0x40 }, /* 228d */ 
    { 0xEC, 0x35, 0x53, 0x40 }, /* 229d */ 
    { 0xEC, 0x35, 0x63, 0x40 }, /* 230d */ 
    { 0xEC, 0x35, 0x63, 0x40 }, /* 231d */ 
    { 0xEC, 0x35, 0x63, 0x40 }, /* 232d */ 
    { 0xEC, 0x35, 0x73, 0x40 }, /* 233d */ 
    { 0xEC, 0x35, 0x73, 0x40 }, /* 234d */ 
    { 0xEC, 0x35, 0x73, 0x40 }, /* 235d */ 
    { 0xEC, 0x2D, 0x53, 0x40 }, /* 236d */ 
    { 0xEC, 0x2D, 0x53, 0x40 }, /* 237d */ 
    { 0xEC, 0x2D, 0x53, 0x40 }, /* 238d */ 
    { 0xF4, 0x2D, 0x23, 0xC0 }, /* 239d */ 
    { 0xF4, 0x2D, 0x23, 0xC0 }, /* 240d */ 
    { 0xF4, 0x2D, 0x33, 0xC0 }, /* 241d */ 
    { 0xFC, 0x2D, 0x33, 0xC0 }, /* 242d */ 
    { 0xFC, 0x2D, 0x43, 0xC0 }, /* 243d */ 
    { 0xFC, 0x2D, 0x43, 0xC0 }, /* 244d */ 
    { 0xFC, 0x25, 0x43, 0xC0 }, /* 245d */ 
    { 0xFC, 0x25, 0x43, 0xC0 }, /* 246d */ 
    { 0xFC, 0x25, 0x43, 0xC0 }, /* 247d */ 
    { 0xFC, 0x25, 0x43, 0xC0 }, /* 248d */ 
    { 0xFC, 0x1D, 0x43, 0xC0 }, /* 249d */ 
    { 0xFC, 0x1D, 0x43, 0xC0 }, /* 250d */ 
    { 0xFC, 0x1D, 0x43, 0xC0 }, /* 251d */ 
    { 0xFC, 0x1D, 0x43, 0xC0 }, /* 252d */ 
    { 0xFC, 0x1D, 0x43, 0xC0 }, /* 253d */ 
    { 0xFC, 0x1D, 0x43, 0xC0 }, /* 254d */ 
    { 0xFC, 0x1D, 0x43, 0xC0 }  /* 255d */ 
};

RLPS_EQUALIZER_RAM pmc_t1_rlps_perf_mode_ram_table[] = 
{
	{ 0x03, 0xFE, 0x18, 0x40 },
	{ 0x03, 0xFE, 0x18, 0x40 },
	{ 0x03, 0xFE, 0x18, 0x40 },
	{ 0x03, 0xFE, 0x18, 0x40 },
	{ 0x03, 0xFE, 0x18, 0x40 },
	{ 0x03, 0xFE, 0x18, 0x40 },
	{ 0x03, 0xFE, 0x18, 0x40 },
	{ 0x03, 0xFE, 0x18, 0x40 },
	{ 0x03, 0xF6, 0x18, 0x40 },
	{ 0x03, 0xF6, 0x18, 0x40 },
	{ 0x03, 0xF6, 0x18, 0x40 },
	{ 0x03, 0xF6, 0x18, 0x40 },
	{ 0x03, 0xF6, 0x18, 0x40 },
	{ 0x03, 0xF6, 0x18, 0x40 },
	{ 0x03, 0xF6, 0x18, 0x40 },
	{ 0x03, 0xF6, 0x18, 0x40 },
	{ 0x03, 0xEE, 0x18, 0x40 },
	{ 0x03, 0xEE, 0x18, 0x40 },
	{ 0x03, 0xEE, 0x18, 0x40 },
	{ 0x03, 0xEE, 0x18, 0x40 },
	{ 0x03, 0xEE, 0x18, 0x40 },
	{ 0x03, 0xEE, 0x18, 0x40 },
	{ 0x03, 0xEE, 0x18, 0x40 },
	{ 0x03, 0xEE, 0x18, 0x40 },
	{ 0x03, 0xE6, 0x18, 0x40 },
	{ 0x03, 0xE6, 0x18, 0x40 },
	{ 0x03, 0xE6, 0x18, 0x40 },
	{ 0x03, 0xE6, 0x18, 0x40 },
	{ 0x03, 0xE6, 0x18, 0x40 },
	{ 0x03, 0xE6, 0x18, 0x40 },
	{ 0x03, 0xE6, 0x18, 0x40 },
	{ 0x03, 0xE6, 0x18, 0x40 },
	{ 0x03, 0xDE, 0x18, 0x40 },
	{ 0x03, 0xDE, 0x18, 0x40 },
	{ 0x03, 0xDE, 0x18, 0x40 },
	{ 0x03, 0xDE, 0x18, 0x40 },
	{ 0x03, 0xDE, 0x18, 0x40 },
	{ 0x03, 0xDE, 0x18, 0x40 },
	{ 0x03, 0xDE, 0x18, 0x40 },
	{ 0x03, 0xDE, 0x18, 0x40 },
	{ 0x03, 0xD6, 0x18, 0x40 },
	{ 0x03, 0xD6, 0x18, 0x40 },
	{ 0x03, 0xD6, 0x18, 0x40 },
	{ 0x03, 0xD6, 0x18, 0x40 },
	{ 0x03, 0xD6, 0x18, 0x40 },
	{ 0x03, 0xD6, 0x18, 0x40 },
	{ 0x03, 0xD6, 0x18, 0x40 },
	{ 0x03, 0xD6, 0x18, 0x40 },
	{ 0x03, 0xCE, 0x18, 0x40 },
	{ 0x03, 0xCE, 0x18, 0x40 },
	{ 0x03, 0xCE, 0x18, 0x40 },
	{ 0x03, 0xCE, 0x18, 0x40 },
	{ 0x03, 0xCE, 0x18, 0x40 },
	{ 0x03, 0xCE, 0x18, 0x40 },
	{ 0x03, 0xCE, 0x18, 0x40 },
	{ 0x03, 0xCE, 0x18, 0x40 },
	{ 0x03, 0xC6, 0x18, 0x40 },
	{ 0x03, 0xC6, 0x18, 0x40 },
	{ 0x03, 0xC6, 0x18, 0x40 },
	{ 0x03, 0xC6, 0x18, 0x40 },
	{ 0x03, 0xC6, 0x18, 0x40 },
	{ 0x03, 0xC6, 0x18, 0x40 },
	{ 0x03, 0xC6, 0x18, 0x40 },
	{ 0x03, 0xC6, 0x18, 0x40 },
	{ 0x03, 0xBE, 0x18, 0x40 },
	{ 0x03, 0xBE, 0x18, 0x40 },
	{ 0x03, 0xBE, 0x18, 0x40 },
	{ 0x03, 0xBE, 0x18, 0x40 },
	{ 0x03, 0xBE, 0x18, 0x40 },
	{ 0x03, 0xBE, 0x18, 0x40 },
	{ 0x03, 0xBE, 0x18, 0x40 },
	{ 0x03, 0xBE, 0x18, 0x40 },
	{ 0x03, 0xB6, 0x18, 0x40 },
	{ 0x03, 0xB6, 0x18, 0x40 },
	{ 0x03, 0xB6, 0x18, 0x40 },
	{ 0x03, 0xB6, 0x18, 0x40 },
	{ 0x03, 0xB6, 0x18, 0x40 },
	{ 0x03, 0xB6, 0x18, 0x40 },
	{ 0x03, 0xB6, 0x18, 0x40 },
	{ 0x03, 0xB6, 0x18, 0x40 },
	{ 0x03, 0xA6, 0x18, 0x40 },
	{ 0x03, 0xA6, 0x18, 0x40 },
	{ 0x03, 0xA6, 0x18, 0x40 },
	{ 0x03, 0xA6, 0x18, 0x40 },
	{ 0x03, 0xA6, 0x18, 0x40 },
	{ 0x03, 0xA6, 0x18, 0x40 },
	{ 0x03, 0xA6, 0x18, 0x40 },
	{ 0x03, 0xA6, 0x18, 0x40 },
	{ 0x03, 0x9E, 0x18, 0x40 },
	{ 0x03, 0x9E, 0x18, 0x40 },
	{ 0x03, 0x9E, 0x18, 0x40 },
	{ 0x03, 0x9E, 0x18, 0x40 },
	{ 0x03, 0x9E, 0x18, 0x40 },
	{ 0x03, 0x9E, 0x18, 0x40 },
	{ 0x03, 0x9E, 0x18, 0x40 },
	{ 0x03, 0x9E, 0x18, 0x40 },
	{ 0x03, 0x96, 0x18, 0x40 },
	{ 0x03, 0x96, 0x18, 0x40 },
	{ 0x03, 0x96, 0x18, 0x40 },
	{ 0x03, 0x96, 0x18, 0x40 },
	{ 0x03, 0x96, 0x18, 0x40 },
	{ 0x03, 0x96, 0x18, 0x40 },
	{ 0x03, 0x96, 0x18, 0x40 },
	{ 0x03, 0x96, 0x18, 0x40 },
	{ 0x03, 0x8E, 0x18, 0x40 },
	{ 0x03, 0x8E, 0x18, 0x40 },
	{ 0x03, 0x8E, 0x18, 0x40 },
	{ 0x03, 0x8E, 0x18, 0x40 },
	{ 0x03, 0x8E, 0x18, 0x40 },
	{ 0x03, 0x8E, 0x18, 0x40 },
	{ 0x03, 0x8E, 0x18, 0x40 },
	{ 0x03, 0x8E, 0x18, 0x40 },
	{ 0x03, 0x86, 0x18, 0x40 },
	{ 0x03, 0x86, 0x18, 0x40 },
	{ 0x03, 0x86, 0x18, 0x40 },
	{ 0x03, 0x86, 0x18, 0x40 },
	{ 0x03, 0x86, 0x18, 0x40 },
	{ 0x03, 0x86, 0x18, 0x40 },
	{ 0x03, 0x86, 0x18, 0x40 },
	{ 0x03, 0x86, 0x18, 0x40 },
	{ 0x03, 0x7E, 0x18, 0x40 },
	{ 0x03, 0x7E, 0x18, 0x40 },
	{ 0x03, 0x7E, 0x18, 0x40 },
	{ 0x03, 0x7E, 0x18, 0x40 },
	{ 0x03, 0x7E, 0x18, 0x40 },
	{ 0x03, 0x7E, 0x18, 0x40 },
	{ 0x03, 0x7E, 0x18, 0x40 },
	{ 0x03, 0x7E, 0x18, 0x40 },
	{ 0x03, 0x76, 0x18, 0x40 },
	{ 0x03, 0x76, 0x18, 0x40 },
	{ 0x03, 0x76, 0x18, 0x40 },
	{ 0x03, 0x76, 0x18, 0x40 },
	{ 0x03, 0x76, 0x18, 0x40 },
	{ 0x03, 0x76, 0x18, 0x40 },
	{ 0x03, 0x76, 0x18, 0x40 },
	{ 0x03, 0x76, 0x18, 0x40 },
	{ 0x03, 0x6E, 0x18, 0x40 },
	{ 0x03, 0x6E, 0x18, 0x40 },
	{ 0x03, 0x6E, 0x18, 0x40 },
	{ 0x03, 0x6E, 0x18, 0x40 },
	{ 0x03, 0x6E, 0x18, 0x40 },
	{ 0x03, 0x6E, 0x18, 0x40 },
	{ 0x03, 0x6E, 0x18, 0x40 },
	{ 0x03, 0x6E, 0x18, 0x40 },
	{ 0x03, 0x66, 0x18, 0x40 },
	{ 0x03, 0x66, 0x18, 0x40 },
	{ 0x03, 0x66, 0x18, 0x40 },
	{ 0x03, 0x66, 0x18, 0x40 },
	{ 0x03, 0x66, 0x18, 0x40 },
	{ 0x03, 0x66, 0x18, 0x40 },
	{ 0x03, 0x66, 0x18, 0x40 },
	{ 0x03, 0x66, 0x18, 0x40 },
	{ 0x03, 0x5E, 0x18, 0x40 },
	{ 0x03, 0x5E, 0x18, 0x40 },
	{ 0x03, 0x5E, 0x18, 0x40 },
	{ 0x03, 0x5E, 0x18, 0x40 },
	{ 0x03, 0x5E, 0x18, 0x40 },
	{ 0x03, 0x5E, 0x18, 0x40 },
	{ 0x03, 0x5E, 0x18, 0x40 },
	{ 0x03, 0x5E, 0x18, 0x40 },
	{ 0x03, 0x56, 0x18, 0x40 },
	{ 0x03, 0x56, 0x18, 0x40 },
	{ 0x03, 0x56, 0x18, 0x40 },
	{ 0x03, 0x56, 0x18, 0x40 },
	{ 0x03, 0x56, 0x18, 0x40 },
	{ 0x03, 0x56, 0x18, 0x40 },
	{ 0x03, 0x56, 0x18, 0x40 },
	{ 0x03, 0x56, 0x18, 0x40 },
	{ 0x03, 0x4E, 0x18, 0x40 },
	{ 0x03, 0x4E, 0x18, 0x40 },
	{ 0x03, 0x4E, 0x18, 0x40 },
	{ 0x03, 0x4E, 0x18, 0x40 },
	{ 0x03, 0x4E, 0x18, 0x40 },
	{ 0x03, 0x4E, 0x18, 0x40 },
	{ 0x03, 0x4E, 0x18, 0x40 },
	{ 0x03, 0x4E, 0x18, 0x40 },
	{ 0x03, 0x46, 0x18, 0x40 },
	{ 0x03, 0x46, 0x18, 0x40 },
	{ 0x03, 0x46, 0x18, 0x40 },
	{ 0x03, 0x46, 0x18, 0x40 },
	{ 0x03, 0x46, 0x18, 0x40 },
	{ 0x03, 0x46, 0x18, 0x40 },
	{ 0x03, 0x46, 0x18, 0x40 },
	{ 0x03, 0x46, 0x18, 0x40 },
	{ 0x03, 0x3E, 0x18, 0x40 },
	{ 0x03, 0x3E, 0x18, 0x40 },
	{ 0x03, 0x3E, 0x18, 0x40 },
	{ 0x03, 0x3E, 0x18, 0x40 },
	{ 0x03, 0x3E, 0x18, 0x40 },
	{ 0x03, 0x3E, 0x18, 0x40 },
	{ 0x03, 0x3E, 0x18, 0x40 },
	{ 0x03, 0x3E, 0x18, 0x40 },
	{ 0x03, 0x36, 0x18, 0x40 },
	{ 0x03, 0x36, 0x18, 0x40 },
	{ 0x03, 0x36, 0x18, 0x40 },
	{ 0x03, 0x36, 0x18, 0x40 },
	{ 0x03, 0x36, 0x18, 0x40 },
	{ 0x03, 0x36, 0x18, 0x40 },
	{ 0x03, 0x36, 0x18, 0x40 },
	{ 0x03, 0x36, 0x18, 0x40 },
	{ 0x03, 0x2E, 0x18, 0x40 },
	{ 0x03, 0x2E, 0x18, 0x40 },
	{ 0x03, 0x2E, 0x18, 0x40 },
	{ 0x03, 0x2E, 0x18, 0x40 },
	{ 0x03, 0x2E, 0x18, 0x40 },
	{ 0x03, 0x2E, 0x18, 0x40 },
	{ 0x03, 0x2E, 0x18, 0x40 },
	{ 0x03, 0x2E, 0x18, 0x40 },
	{ 0x03, 0x26, 0x18, 0x40 },
	{ 0x03, 0x26, 0x18, 0x40 },
	{ 0x03, 0x26, 0x18, 0x40 },
	{ 0x03, 0x26, 0x18, 0x40 },
	{ 0x03, 0x26, 0x18, 0x40 },
	{ 0x03, 0x26, 0x18, 0x40 },
	{ 0x03, 0x26, 0x18, 0x40 },
	{ 0x03, 0x26, 0x18, 0x40 },
	{ 0x03, 0x1E, 0x18, 0x40 },
	{ 0x03, 0x1E, 0x18, 0x40 },
	{ 0x03, 0x1E, 0x18, 0x40 },
	{ 0x03, 0x1E, 0x18, 0x40 },
	{ 0x03, 0x1E, 0x18, 0x40 },
	{ 0x03, 0x1E, 0x18, 0x40 },
	{ 0x03, 0x1E, 0x18, 0x40 },
	{ 0x03, 0x1E, 0x18, 0x40 },
	{ 0x03, 0x16, 0x18, 0x40 },
	{ 0x03, 0x16, 0x18, 0x40 },
	{ 0x03, 0x16, 0x18, 0x40 },
	{ 0x03, 0x16, 0x18, 0x40 },
	{ 0x03, 0x16, 0x18, 0x40 },
	{ 0x03, 0x16, 0x18, 0x40 },
	{ 0x03, 0x16, 0x18, 0x40 },
	{ 0x03, 0x16, 0x18, 0x40 },
	{ 0x03, 0x0E, 0x18, 0x40 },
	{ 0x03, 0x0E, 0x18, 0x40 },
	{ 0x03, 0x0E, 0x18, 0x40 },
	{ 0x03, 0x0E, 0x18, 0x40 },
	{ 0x03, 0x0E, 0x18, 0x40 },
	{ 0x03, 0x0E, 0x18, 0x40 },
	{ 0x03, 0x0E, 0x18, 0x40 },
	{ 0x03, 0x0E, 0x18, 0x40 },
	{ 0x03, 0x0E, 0x18, 0x40 },
	{ 0x03, 0x06, 0x18, 0x40 },
	{ 0x03, 0x06, 0x18, 0x40 },
	{ 0x03, 0x06, 0x18, 0x40 },
	{ 0x03, 0x06, 0x18, 0x40 },
	{ 0x03, 0x06, 0x18, 0x40 },
	{ 0x03, 0x06, 0x18, 0x40 },
	{ 0x03, 0x06, 0x18, 0x40 },
	{ 0x03, 0x06, 0x18, 0x40 },
	{ 0x03, 0x06, 0x18, 0x40 },
	{ 0x03, 0x06, 0x18, 0x40 },
	{ 0x03, 0x06, 0x18, 0x40 },
	{ 0x03, 0x06, 0x18, 0x40 },
	{ 0x03, 0x06, 0x18, 0x40 },
	{ 0x03, 0x06, 0x18, 0x40 },
	{ 0x03, 0x06, 0x18, 0x40 }
};

RLPS_EQUALIZER_RAM pmc_e1_rlps_ram_table[] = 
{
    { 0x07, 0xDE, 0x18, 0x2C },
    { 0x07, 0xDE, 0x18, 0x2C },
    { 0x07, 0xD6, 0x18, 0x2C },
    { 0x07, 0xD6, 0x18, 0x2C },
    { 0x07, 0xD6, 0x18, 0x2C },
    { 0x07, 0xCE, 0x18, 0x2C },
    { 0x07, 0xCE, 0x18, 0x2C },
    { 0x07, 0xCE, 0x18, 0x2C },
    { 0x07, 0xC6, 0x18, 0x2C },
    { 0x07, 0xC6, 0x18, 0x2C },
    { 0x07, 0xC6, 0x18, 0x2C },
    { 0x07, 0xBE, 0x18, 0x2C },
    { 0x07, 0xBE, 0x18, 0x2C },
    { 0x07, 0xBE, 0x18, 0x2C },
    { 0x07, 0xBE, 0x18, 0x2C },
    { 0x07, 0xBE, 0x18, 0x2C },
    { 0x07, 0xB6, 0x18, 0x2C },
    { 0x07, 0xB6, 0x18, 0x2C },
    { 0x07, 0xB6, 0x18, 0x2C },
    { 0x07, 0xB6, 0x18, 0x2C },
    { 0x07, 0xB6, 0x18, 0x2C },
    { 0x07, 0xAE, 0x18, 0x2C },
    { 0x07, 0xAE, 0x18, 0x2C },
    { 0x07, 0xAE, 0x18, 0x2C },
    { 0x07, 0xAE, 0x18, 0x2C },
    { 0x07, 0xAE, 0x18, 0x2C },
    { 0x07, 0xB6, 0x18, 0xAC },
    { 0x07, 0xAE, 0x18, 0xAC },
    { 0x07, 0xAE, 0x18, 0xAC },
    { 0x07, 0xAE, 0x18, 0xAC },
    { 0x07, 0xAE, 0x18, 0xAC },
    { 0x07, 0xA6, 0x18, 0xAC },
    { 0x07, 0xA6, 0x18, 0xAC },
    { 0x07, 0xA6, 0x18, 0xAC },
    { 0x07, 0xA6, 0x18, 0xAC },
    { 0x07, 0x9E, 0x18, 0xAC },
    { 0x07, 0xA6, 0x19, 0x2C },
    { 0x07, 0xA6, 0x19, 0x2C },
    { 0x07, 0xA6, 0x19, 0x2C },
    { 0x0F, 0xA6, 0x19, 0x2C },
    { 0x0F, 0xA6, 0x19, 0x2C },
    { 0x0F, 0x9E, 0x19, 0x2C },
    { 0x0F, 0x9E, 0x19, 0x2C },
    { 0x0F, 0x9E, 0x19, 0x2C },
    { 0x17, 0x9E, 0x19, 0x2C },
    { 0x17, 0xA6, 0x19, 0xAC },
    { 0x17, 0x9E, 0x19, 0xAC },
    { 0x17, 0x9E, 0x19, 0xAC },
    { 0x17, 0x96, 0x19, 0xAC },
    { 0x1F, 0x96, 0x19, 0xAC },
    { 0x1F, 0x96, 0x19, 0xAC },
    { 0x1F, 0x8E, 0x19, 0xAC },
    { 0x1F, 0x8E, 0x19, 0xAC },
    { 0x1F, 0x8E, 0x19, 0xAC },
    { 0x27, 0x8E, 0x19, 0xAC },
    { 0x27, 0x8E, 0x1A, 0x2C },
    { 0x27, 0x8E, 0x1A, 0x2C },
    { 0x27, 0x8E, 0x1A, 0x2C },
    { 0x27, 0x8E, 0x1A, 0x2C },
    { 0x2F, 0x86, 0x1A, 0x2C },
    { 0x2F, 0x86, 0x1A, 0x2C },
    { 0x2F, 0x86, 0x1A, 0x2C },
    { 0x2F, 0x7E, 0x1A, 0x2C },
    { 0x2F, 0x7E, 0x1A, 0x2C },
    { 0x2F, 0x7E, 0x1A, 0x2C },
    { 0x37, 0x7E, 0x1A, 0x2C },
    { 0x37, 0x7E, 0x1A, 0xAC },
    { 0x37, 0x7E, 0x1A, 0xAC },
    { 0x37, 0x7E, 0x1A, 0xAC },
    { 0x37, 0x7E, 0x1A, 0xAC },
    { 0x3F, 0x7E, 0x2A, 0xAC },
    { 0x3F, 0x7E, 0x2A, 0xAC },
    { 0x3F, 0x76, 0x2A, 0xAC },
    { 0x3F, 0x86, 0x2B, 0x2C },
    { 0x3F, 0x7E, 0x2B, 0x2C },
    { 0x47, 0x7E, 0x2B, 0x2C },
    { 0x47, 0x7E, 0x2F, 0x2C },
    { 0x47, 0x7E, 0x2F, 0x2C },
    { 0x47, 0x7E, 0x2F, 0x2C },
    { 0x47, 0x76, 0x2F, 0x2C },
    { 0x4F, 0x76, 0x2F, 0x2C },
    { 0x4F, 0x76, 0x2F, 0x2C },
    { 0x4F, 0x6E, 0x2F, 0x2C },
    { 0x4F, 0x6E, 0x2F, 0x2C },
    { 0x4F, 0x6E, 0x2F, 0x2C },
    { 0x57, 0x6E, 0x2F, 0x2C },
    { 0x57, 0x6E, 0x2F, 0x2C },
    { 0x57, 0x6E, 0x3F, 0x2C },
    { 0x57, 0x6E, 0x3F, 0x2C },
    { 0x57, 0x6E, 0x3F, 0x2C },
    { 0x5F, 0x6E, 0x3F, 0x2C },
    { 0x5F, 0x6E, 0x4F, 0x2C },
    { 0x5F, 0x6E, 0x4F, 0x2C },
    { 0x5F, 0x6E, 0x4F, 0x2C },
    { 0x5F, 0x66, 0x4F, 0x2C },
    { 0x67, 0x66, 0x4F, 0x2C },
    { 0x67, 0x66, 0x4F, 0x2C },
    { 0x67, 0x5E, 0x4F, 0x2C },
    { 0x67, 0x5E, 0x4F, 0x2C },
    { 0x67, 0x66, 0x4F, 0x2C },
    { 0x67, 0x66, 0x4F, 0x2C },
    { 0x67, 0x66, 0x5F, 0x2C },
    { 0x6F, 0x6E, 0x5F, 0x2C },
    { 0x6F, 0x6E, 0x6F, 0x2C },
    { 0x6F, 0x6E, 0x6F, 0x2C },
    { 0x6F, 0x6E, 0x7F, 0x2C },
    { 0x6F, 0x6E, 0x7F, 0x2C },
    { 0x6F, 0x6E, 0x7F, 0x2C },
    { 0x77, 0x66, 0x7F, 0x2C },
    { 0x77, 0x66, 0x7F, 0x2C },
    { 0x77, 0x5E, 0x6F, 0x2C },
    { 0x77, 0x5E, 0x7F, 0x2C },
    { 0x77, 0x5E, 0x7F, 0x2C },
    { 0x7F, 0x5E, 0x7F, 0x2C },
    { 0x7F, 0x5E, 0x8F, 0x2C },
    { 0x7F, 0x5E, 0x8F, 0x2C },
    { 0x7F, 0x5E, 0x8F, 0x2C },
    { 0x87, 0x56, 0x8F, 0x2C },
    { 0x87, 0x56, 0x8F, 0x2C },
    { 0x87, 0x56, 0x8F, 0x2C },
    { 0x87, 0x4E, 0x8F, 0x2C },
    { 0x87, 0x4E, 0x8F, 0x2C },
    { 0x87, 0x4E, 0x8F, 0x2C },
    { 0x8F, 0x4E, 0x9F, 0x2C },
    { 0x8F, 0x4E, 0x9F, 0x2C },
    { 0x8F, 0x4E, 0xAF, 0x2C },
    { 0x8F, 0x4E, 0xAF, 0x2C },
    { 0x8F, 0x4E, 0xAF, 0x2C },
    { 0x97, 0x4E, 0xAF, 0x2C },
    { 0x97, 0x4E, 0xAF, 0x2C },
    { 0x97, 0x4E, 0xAB, 0x2C },
    { 0x97, 0x4E, 0xAB, 0x2C },
    { 0x97, 0x4E, 0xAB, 0x2C },
    { 0x9F, 0x4E, 0xAB, 0x2C },
    { 0x9F, 0x4E, 0xBB, 0x2C },
    { 0x9F, 0x4E, 0xBB, 0x2C },
    { 0x9F, 0x4E, 0xBB, 0x2C },
    { 0x9F, 0x4E, 0xCB, 0x2C },
    { 0xA7, 0x4E, 0xCB, 0x2C },
    { 0xA7, 0x4E, 0xCB, 0x2C },
    { 0xA7, 0x46, 0xCB, 0x2C },
    { 0xA7, 0x46, 0xCB, 0x2C },
    { 0xA7, 0x46, 0xCB, 0x2C },
    { 0xA7, 0x46, 0xDB, 0x2C },
    { 0xAF, 0x46, 0xDB, 0x2C },
    { 0xAF, 0x46, 0xEB, 0x2C },
    { 0xAF, 0x46, 0xEB, 0x2C },
    { 0xAF, 0x4E, 0xEB, 0x2C },
    { 0xAE, 0x4E, 0xEB, 0x2C },
    { 0xAE, 0x4E, 0xEB, 0x2C },
    { 0xB5, 0x46, 0xFB, 0x2C },
    { 0xB5, 0x54, 0xFB, 0x2C },
    { 0xB5, 0x4C, 0xFB, 0x2C },
    { 0xB5, 0x54, 0xFB, 0x2C },
    { 0xB5, 0x54, 0xFB, 0x2C },
    { 0xBD, 0x54, 0xFB, 0x2C },
    { 0xBD, 0x4C, 0xFB, 0x2C },
    { 0xBD, 0x4C, 0xFB, 0x2C },
    { 0xBD, 0x4C, 0xFB, 0x2C },
    { 0xBD, 0x44, 0xEB, 0x2C },
    { 0xC5, 0x44, 0xFB, 0x2C },
    { 0xC5, 0x44, 0xFB, 0x2C },
    { 0xC5, 0x44, 0xFB, 0x2C },
    { 0xC5, 0x45, 0x0B, 0x2C },
    { 0xC5, 0x45, 0x0B, 0x2C },
    { 0xC5, 0x45, 0x0B, 0x2C },
    { 0xCD, 0x45, 0x0B, 0x2C },
    { 0xCD, 0x45, 0x0B, 0x2C },
    { 0xCD, 0x3D, 0x0B, 0x2C },
    { 0xCD, 0x3D, 0x0B, 0x2C },
    { 0xCD, 0x3D, 0x0B, 0x2C },
    { 0xD5, 0x3D, 0x0B, 0x2C },
    { 0xD5, 0x3D, 0x0B, 0x2C },
    { 0xD5, 0x3D, 0x1B, 0x2C },
    { 0xD5, 0x3D, 0x1B, 0x2C },
    { 0xD5, 0x3D, 0x1B, 0x2C },
    { 0xDD, 0x3D, 0x1B, 0x2C },
    { 0xDD, 0x3D, 0x1B, 0x2C },
    { 0xDD, 0x35, 0x1B, 0x2C },
    { 0xDD, 0x35, 0x1B, 0x2C },
    { 0xDD, 0x35, 0x1B, 0x2C },
    { 0xE5, 0x35, 0x1B, 0x2C },
    { 0xE5, 0x35, 0x1B, 0x2C },
    { 0xE5, 0x2D, 0x1B, 0x2C },
    { 0xE5, 0x2D, 0x1B, 0x2C },
    { 0xE5, 0x2D, 0x3B, 0x2C },
    { 0xED, 0x2D, 0x4B, 0x2C },
    { 0xED, 0x2D, 0x1B, 0xA8 },
    { 0xED, 0x2D, 0x1B, 0xAC },
    { 0xED, 0x2D, 0x17, 0xAC },
    { 0xED, 0x2D, 0x17, 0xAC },
    { 0xED, 0x2D, 0x27, 0xAC },
    { 0xF5, 0x2D, 0x27, 0xAC },
    { 0xF5, 0x2D, 0x27, 0xAC },
    { 0xF5, 0x2D, 0x2B, 0xAC },
    { 0xF5, 0x2D, 0x2B, 0xAC },
    { 0xF5, 0x2D, 0x2B, 0xAC },
    { 0xFD, 0x2D, 0x2B, 0xAC },
    { 0xFD, 0x2B, 0x2B, 0xAC },
    { 0xFD, 0x2B, 0x2B, 0xAC },
    { 0xFD, 0x2B, 0x2B, 0xAC },
    { 0xFD, 0x2B, 0x2B, 0xAC },
    { 0xFD, 0x23, 0x2B, 0xAC },
    { 0xFD, 0x23, 0x2B, 0xAC },
    { 0xFD, 0x23, 0x2B, 0xAC },
    { 0xFD, 0x21, 0x2B, 0xAC },
    { 0xFD, 0x21, 0x2B, 0xAC },
    { 0xFD, 0x29, 0x2B, 0xAC },
    { 0xFD, 0x29, 0x2B, 0xAC },
    { 0xFD, 0x29, 0x27, 0xAC },
    { 0xFD, 0x29, 0x37, 0xAC },
    { 0xFD, 0x29, 0x23, 0xAC },
    { 0xFD, 0x29, 0x23, 0xAC },
    { 0xFD, 0x29, 0x23, 0xAC },
    { 0xFD, 0x29, 0x23, 0xAC },
    { 0xFD, 0x21, 0x23, 0xAC },
    { 0xFD, 0x21, 0x23, 0xAC },
    { 0xFD, 0x21, 0x23, 0xAC },
    { 0xFD, 0x21, 0x33, 0xAC },
    { 0xFD, 0x21, 0x33, 0xAC },
    { 0xFD, 0x21, 0x33, 0xAC },
    { 0xFD, 0x21, 0x43, 0xAC },
    { 0xFD, 0x21, 0x43, 0xAC },
    { 0xFD, 0x21, 0x43, 0xAC },
    { 0xFC, 0x21, 0x43, 0xAC },
    { 0xFC, 0x21, 0x43, 0xAC },
    { 0xFC, 0x19, 0x43, 0xAC },
    { 0xFC, 0x19, 0x43, 0xAC },
    { 0xFC, 0x19, 0x43, 0xAC },
    { 0xFC, 0x19, 0x43, 0xAC },
    { 0xFC, 0x19, 0x53, 0xAC },
    { 0xFC, 0x19, 0x53, 0xAC },
    { 0xFC, 0x19, 0x53, 0xAC },
    { 0xFC, 0x19, 0x53, 0xAC },
    { 0xFC, 0x19, 0x63, 0xAC },
    { 0xFC, 0x19, 0x63, 0xAC },
    { 0xFC, 0x19, 0x63, 0xAC },
    { 0xFC, 0x19, 0x73, 0xAC },
    { 0xFC, 0x19, 0x73, 0xAC },
    { 0xFC, 0x19, 0x73, 0xAC },
    { 0xFC, 0x19, 0x73, 0xAC },
    { 0xFC, 0x19, 0x73, 0xAC },
    { 0xFC, 0x19, 0x83, 0xAC },
    { 0xFC, 0x19, 0x83, 0xAC },
    { 0xFC, 0x19, 0x83, 0xAC },
    { 0xFC, 0x19, 0x83, 0xAC },
    { 0xFC, 0x19, 0x83, 0xAC },
    { 0xFC, 0x19, 0x93, 0xAC },
    { 0xFC, 0x19, 0x93, 0xAC },
    { 0xFC, 0x19, 0x93, 0xAC },
    { 0xFC, 0x19, 0xA3, 0xAC },
    { 0xFC, 0x19, 0xA3, 0xAC },
    { 0xFC, 0x19, 0xB3, 0xAC },
    { 0xFC, 0x19, 0xB3, 0xAC },
    { 0xFC, 0x19, 0xB3, 0xAC },
    { 0xFC, 0x19, 0xB3, 0xAC }
};
RLPS_EQUALIZER_RAM pmc4_e1_rlps_ram_table[] = 
{
    { 0x0F, 0xD6, 0x1C, 0x2C }, /* 0d */ 
    { 0x0F, 0xD6, 0x1C, 0x2C }, /* 1d */ 
    { 0x0F, 0xD6, 0x2C, 0x2C }, /* 2d */ 
    { 0x0F, 0xD6, 0x2C, 0x2C }, /* 3d */ 
    { 0x0F, 0xD6, 0x3C, 0x2C }, /* 4d */ 
    { 0x0F, 0xD6, 0x3C, 0x2C }, /* 5d */ 
    { 0x0F, 0xCE, 0x3C, 0x2C }, /* 6d */ 
    { 0x0F, 0xCE, 0x3C, 0x2C }, /* 7d */ 
    { 0x0F, 0xCE, 0x3C, 0x2C }, /* 8d */ 
    { 0x17, 0xCE, 0x3C, 0x2C }, /* 9d */ 
    { 0x17, 0xCE, 0x3C, 0x2C }, /* 10d */ 
    { 0x17, 0xCE, 0x4C, 0x2C }, /* 11d */ 
    { 0x17, 0xCE, 0x4C, 0x2C }, /* 12d */ 
    { 0x17, 0xCE, 0x4C, 0x2C }, /* 13d */ 
    { 0x17, 0xCE, 0x4C, 0x2C }, /* 14d */ 
    { 0x17, 0xCE, 0x4C, 0x2C }, /* 15d */ 
    { 0x17, 0xCE, 0x4C, 0x2C }, /* 16d */ 
    { 0x1F, 0xC6, 0x4C, 0x2C }, /* 17d */ 
    { 0x1F, 0xC6, 0x4C, 0x2C }, /* 18d */ 
    { 0x1F, 0xC6, 0x4C, 0x2C }, /* 19d */ 
    { 0x1F, 0xC6, 0x4C, 0x2C }, /* 20d */ 
    { 0x1F, 0xC6, 0x4C, 0x2C }, /* 21d */ 
    { 0x1F, 0xC6, 0x5C, 0x2C }, /* 22d */ 
    { 0x1F, 0xC6, 0x5C, 0x2C }, /* 23d */ 
    { 0x1F, 0xC6, 0x5C, 0x2C }, /* 24d */ 
    { 0x1F, 0xC6, 0x5C, 0x2C }, /* 25d */ 
    { 0x27, 0xC6, 0x5C, 0x2C }, /* 26d */ 
    { 0x27, 0xC6, 0x5C, 0x2C }, /* 27d */ 
    { 0x27, 0xC6, 0x7C, 0x32 }, /* 28d */ 
    { 0x27, 0xC6, 0x8C, 0x32 }, /* 29d */ 
    { 0x27, 0xC6, 0x9C, 0x32 }, /* 30d */ 
    { 0x27, 0xC6, 0xAC, 0x32 }, /* 31d */ 
    { 0x27, 0xC6, 0xBC, 0x32 }, /* 32d */ 
    { 0x27, 0xC6, 0xCC, 0x32 }, /* 33d */ 
    { 0x2F, 0xC6, 0xDC, 0x32 }, /* 34d */ 
    { 0x2F, 0xC6, 0xEC, 0x32 }, /* 35d */ 
    { 0x2F, 0xC7, 0x0C, 0x32 }, /* 36d */ 
    { 0x2F, 0xC7, 0x2C, 0x32 }, /* 37d */ 
    { 0x2F, 0xBC, 0x68, 0x32 }, /* 38d */ 
    { 0x2F, 0xBC, 0x68, 0x2C }, /* 39d */ 
    { 0x2F, 0xB4, 0x68, 0x2C }, /* 40d */ 
    { 0x2F, 0xB4, 0x68, 0x2C }, /* 41d */ 
    { 0x37, 0xB4, 0x68, 0x2C }, /* 42d */ 
    { 0x37, 0xB4, 0x78, 0x2C }, /* 43d */
    { 0x37, 0xB4, 0x78, 0x2C }, /* 44d */ 
    { 0x37, 0xB4, 0x78, 0x2C }, /* 45d */ 
    { 0x37, 0xB4, 0x78, 0x2C }, /* 46d */ 
    { 0x37, 0xB4, 0x78, 0x2C }, /* 47d */ 
    { 0x37, 0xB4, 0x78, 0x2C }, /* 48d */ 
    { 0x37, 0xAC, 0x78, 0x2C }, /* 49d */ 
    { 0x37, 0xAC, 0x78, 0x2C }, /* 50d */ 
    { 0x3F, 0xAC, 0x78, 0x2C }, /* 51d */ 
    { 0x3F, 0xAC, 0x78, 0x2C }, /* 52d */ 
    { 0x3F, 0xAC, 0x78, 0x2C }, /* 53d */ 
    { 0x3F, 0xAC, 0x78, 0x2C }, /* 54d */ 
    { 0x3F, 0xAC, 0x78, 0x2C }, /* 55d */ 
    { 0x3F, 0xAC, 0x78, 0x2C }, /* 56d */ 
    { 0x3F, 0xAC, 0x78, 0x2C }, /* 57d */ 
    { 0x3F, 0xAC, 0x78, 0x2C }, /* 58d */ 
    { 0x47, 0xAC, 0x78, 0x2C }, /* 59d */ 
    { 0x47, 0xAC, 0x88, 0x2C }, /* 60d */ 
    { 0x47, 0xAC, 0x88, 0x2C }, /* 61d */ 
    { 0x47, 0xAC, 0x98, 0x2C }, /* 62d */ 
    { 0x47, 0xAC, 0x98, 0x2C }, /* 63d */ 
    { 0x47, 0xAC, 0x68, 0xAC }, /* 64d */ 
    { 0x47, 0xAC, 0x68, 0xAC }, /* 65d */ 
    { 0x47, 0xAC, 0x78, 0xAC }, /* 66d */ 
    { 0x4F, 0xAC, 0x78, 0xAC }, /* 67d */ 
    { 0x4F, 0xA4, 0x88, 0xAC }, /* 68d */ 
    { 0x4F, 0xA4, 0x88, 0xAC }, /* 69d */ 
    { 0x4F, 0xA4, 0x98, 0xAC }, /* 70d */ 
    { 0x4F, 0x9C, 0x98, 0xAC }, /* 71d */ 
    { 0x4F, 0x9C, 0x98, 0xAC }, /* 72d */ 
    { 0x4F, 0x9C, 0x98, 0xAC }, /* 73d */ 
    { 0x4F, 0x9C, 0x98, 0xAC }, /* 74d */ 
    { 0x4F, 0x9C, 0xA8, 0xAC }, /* 75d */ 
    { 0x57, 0x9C, 0xA8, 0xAC }, /* 76d */ 
    { 0x57, 0x9C, 0xA8, 0xAC }, /* 77d */ 
    { 0x57, 0x9C, 0xA8, 0xAC }, /* 78d */ 
    { 0x57, 0x9C, 0xA8, 0xAC }, /* 79d */ 
    { 0x57, 0x9C, 0xA8, 0xAC }, /* 80d */ 
    { 0x57, 0x9C, 0xA8, 0xAC }, /* 81d */ 
    { 0x57, 0x94, 0xA8, 0xAC }, /* 82d */ 
    { 0x57, 0x94, 0xA8, 0xAC }, /* 83d */ 
    { 0x5F, 0x94, 0xA8, 0xAC }, /* 84d */ 
    { 0x5F, 0x94, 0xA8, 0xAC }, /* 85d */ 
    { 0x5F, 0x94, 0xB8, 0xAC }, /* 86d */ 
    { 0x5F, 0x94, 0xB8, 0xAC }, /* 87d */ 
    { 0x5F, 0x94, 0xB8, 0xAC }, /* 88d */ 
    { 0x5F, 0x94, 0xB8, 0xAC }, /* 89d */ 
    { 0x5F, 0x94, 0xB8, 0xAC }, /* 90d */ 
    { 0x5F, 0x94, 0xB8, 0xAC }, /* 91d */ 
    { 0x67, 0x94, 0xB8, 0xAC }, /* 92d */ 
    { 0x67, 0x8C, 0xB8, 0xAC }, /* 93d */ 
    { 0x67, 0x8C, 0xB8, 0xAC }, /* 94d */ 
    { 0x67, 0x8C, 0xB8, 0xAC }, /* 95d */ 
    { 0x67, 0x8C, 0x99, 0x2C }, /* 96d */ 
    { 0x67, 0x8C, 0x99, 0x2C }, /* 97d */ 
    { 0x67, 0x8C, 0x99, 0x2C }, /* 98d */ 
    { 0x67, 0x8C, 0xA9, 0x2C }, /* 99d */ 
    { 0x67, 0x8C, 0xA9, 0x2C }, /* 100d */ 
    { 0x6F, 0x8C, 0xB9, 0x2C }, /* 101d */ 
    { 0x6F, 0x8C, 0xB9, 0x2C }, /* 102d */ 
    { 0x6F, 0x8C, 0xC9, 0x2C }, /* 103d */ 
    { 0x6F, 0x84, 0xC9, 0x2C }, /* 104d */ 
    { 0x6F, 0x84, 0xC9, 0x2C }, /* 105d */ 
    { 0x6F, 0x84, 0xE9, 0x2C }, /* 106d */ 
    { 0x6F, 0x85, 0x09, 0x2C }, /* 107d */ 
    { 0x6F, 0x85, 0x29, 0x2C }, /* 108d */ 
    { 0x77, 0x85, 0x09, 0x2C }, /* 109d */ 
    { 0x77, 0x84, 0xF5, 0x22 }, /* 110d */ 
    { 0x77, 0x84, 0xF5, 0x22 }, /* 111d */ 
    { 0x77, 0x84, 0xD5, 0xAC }, /* 112d */ 
    { 0x77, 0x84, 0xD5, 0xAC }, /* 113d */ 
    { 0x77, 0x7C, 0xD5, 0xAC }, /* 114d */ 
    { 0x77, 0x7C, 0xE5, 0xAC }, /* 115d */ 
    { 0x77, 0x7C, 0xF5, 0xAC }, /* 116d */ 
    { 0x77, 0x7D, 0x05, 0xAC }, /* 117d */ 
    { 0x7F, 0x7D, 0x15, 0xAC }, /* 118d */ 
    { 0x7F, 0x7D, 0x25, 0xAC }, /* 119d */ 
    { 0x7F, 0x72, 0xE5, 0xAC }, /* 120d */ 
    { 0x7F, 0x72, 0xE5, 0xAC }, /* 121d */ 
    { 0x7F, 0x72, 0xE5, 0xAC }, /* 122d */ 
    { 0x7F, 0x72, 0xE5, 0xAC }, /* 123d */ 
    { 0x7F, 0x72, 0xE5, 0xAC }, /* 124d */ 
    { 0x7F, 0x72, 0xE5, 0xAC }, /* 125d */ 
    { 0x87, 0x73, 0x05, 0xAC }, /* 126d */ 
    { 0x87, 0x73, 0x05, 0xAC }, /* 127d */ 
    { 0x87, 0x73, 0x05, 0xAC }, /* 128d */ 
    { 0x86, 0x72, 0xF6, 0x2C }, /* 129d */ 
    { 0x86, 0x72, 0xF6, 0x2C }, /* 130d */ 
    { 0x86, 0x72, 0xF6, 0x2C }, /* 131d */ 
    { 0x86, 0x72, 0xF6, 0x2C }, /* 132d */ 
    { 0x86, 0x72, 0xF6, 0x2C }, /* 133d */ 
    { 0x8E, 0x72, 0xF6, 0x2C }, /* 134d */ 
    { 0x8E, 0x72, 0xF6, 0x2C }, /* 135d */ 
    { 0x8E, 0x6A, 0xF6, 0x2C }, /* 136d */ 
    { 0x8E, 0x6A, 0xF6, 0x2C }, /* 137d */ 
    { 0x8E, 0x6A, 0xF6, 0x2C }, /* 138d */ 
    { 0x8E, 0x6B, 0x06, 0x2C }, /* 139d */ 
    { 0x8E, 0x6B, 0x06, 0x2C }, /* 140d */ 
    { 0x8E, 0x6B, 0x06, 0x2C }, /* 141d */ 
    { 0x8E, 0x6B, 0x06, 0x2C }, /* 142d */ 
    { 0x96, 0x6B, 0x06, 0x2C }, /* 143d */ 
    { 0x96, 0x6B, 0x06, 0xAC }, /* 144d */ 
    { 0x95, 0x6B, 0x06, 0xAC }, /* 145d */ 
    { 0x95, 0x6B, 0x06, 0xAC }, /* 146d */ 
    { 0x95, 0x63, 0x06, 0xAC }, /* 147d */ 
    { 0x95, 0x63, 0x06, 0xAC }, /* 148d */ 
    { 0x95, 0x63, 0x06, 0xAC }, /* 149d */ 
    { 0x95, 0x63, 0x16, 0xAC }, /* 150d */ 
    { 0x9D, 0x63, 0x16, 0xAC }, /* 151d */ 
    { 0x9D, 0x63, 0x16, 0xAC }, /* 152d */ 
    { 0x9D, 0x63, 0x16, 0xAC }, /* 153d */ 
    { 0x9D, 0x63, 0x16, 0xAC }, /* 154d */ 
    { 0x9D, 0x63, 0x16, 0xAC }, /* 155d */ 
    { 0x9D, 0x5B, 0x16, 0xAC }, /* 156d */ 
    { 0x9D, 0x5B, 0x16, 0xAC }, /* 157d */ 
    { 0x9D, 0x5B, 0x26, 0xAC }, /* 158d */ 
    { 0xA5, 0x5B, 0x26, 0xAC }, /* 159d */ 
    { 0xA5, 0x5A, 0xF7, 0x2C }, /* 160d */ 
    { 0xA5, 0x5A, 0xF7, 0x2C }, /* 161d */ 
    { 0xA5, 0x5B, 0x07, 0x2C }, /* 162d */ 
    { 0xA5, 0x5B, 0x07, 0x2C }, /* 163d */ 
    { 0xA5, 0x5B, 0x17, 0x2C }, /* 164d */ 
    { 0xA5, 0x5B, 0x17, 0x2C }, /* 165d */ 
    { 0xA5, 0x5B, 0x27, 0x2C }, /* 166d */ 
    { 0xA5, 0x5B, 0x27, 0x2C }, /* 167d */ 
    { 0xAD, 0x5B, 0x27, 0x2C }, /* 168d */ 
    { 0xAD, 0x53, 0x27, 0x2C }, /* 169d */ 
    { 0xAD, 0x53, 0x27, 0x2C }, /* 170d */ 
    { 0xAD, 0x53, 0x37, 0x2C }, /* 171d */ 
    { 0xAD, 0x53, 0x37, 0x2C }, /* 172d */ 
    { 0xAD, 0x53, 0x37, 0x2C }, /* 173d */ 
    { 0xAD, 0x53, 0x37, 0x2C }, /* 174d */ 
    { 0xAD, 0x53, 0x37, 0x2C }, /* 175d */ 
    { 0xB5, 0x53, 0x37, 0x2C }, /* 176d */ 
    { 0xB5, 0x53, 0x33, 0x2C }, /* 177d */ 
    { 0xB5, 0x53, 0x33, 0x2C }, /* 178d */ 
    { 0xB5, 0x53, 0x33, 0x2C }, /* 179d */ 
    { 0xB5, 0x53, 0x43, 0x2C }, /* 180d */ 
    { 0xB5, 0x53, 0x43, 0x2C }, /* 181d */ 
    { 0xB5, 0x53, 0x43, 0x2C }, /* 182d */ 
    { 0xB5, 0x49, 0x03, 0x2C }, /* 183d */ 
    { 0xBD, 0x49, 0x03, 0x2C }, /* 184d */ 
    { 0xBD, 0x49, 0x13, 0x2C }, /* 185d */ 
    { 0xBD, 0x49, 0x23, 0x2C }, /* 186d */ 
    { 0xBD, 0x49, 0x33, 0x2C }, /* 187d */ 
    { 0xBD, 0x49, 0x33, 0x2C }, /* 188d */ 
    { 0xBD, 0x49, 0x33, 0x2C }, /* 189d */ 
    { 0xBD, 0x49, 0x33, 0x2C }, /* 190d */ 
    { 0xBD, 0x49, 0x33, 0x2C }, /* 191d */ 
    { 0xBD, 0x49, 0x33, 0x2C }, /* 192d */ 
    { 0xC5, 0x49, 0x33, 0x2C }, /* 193d */ 
    { 0xC5, 0x49, 0x43, 0x2C }, /* 194d */ 
    { 0xC5, 0x49, 0x43, 0x2C }, /* 195d */ 
    { 0xC5, 0x49, 0x53, 0x2C }, /* 196d */ 
    { 0xC5, 0x41, 0x53, 0x2C }, /* 197d */ 
    { 0xC5, 0x41, 0x53, 0x2C }, /* 198d */ 
    { 0xC5, 0x41, 0x53, 0x2C }, /* 199d */ 
    { 0xC5, 0x41, 0x53, 0x2C }, /* 200d */ 
    { 0xCD, 0x39, 0x53, 0x2C }, /* 201d */ 
    { 0xCD, 0x39, 0x53, 0x2C }, /* 202d */ 
    { 0xCD, 0x39, 0x63, 0x2C }, /* 203d */ 
    { 0xCD, 0x39, 0x63, 0x2C }, /* 204d */ 
    { 0xCC, 0x39, 0x63, 0x2C }, /* 205d */ 
    { 0xCC, 0x39, 0x63, 0x2C }, /* 206d */ 
    { 0xCC, 0x39, 0x63, 0xAC }, /* 207d */ 
    { 0xCC, 0x39, 0x63, 0xAC }, /* 208d */ 
    { 0xCC, 0x39, 0x63, 0xAC }, /* 209d */ 
    { 0xD4, 0x39, 0x63, 0xAC }, /* 210d */ 
    { 0xD4, 0x39, 0x63, 0xAC }, /* 211d */ 
    { 0xD4, 0x31, 0x63, 0xAC }, /* 212d */ 
    { 0xD4, 0x31, 0x63, 0xAC }, /* 213d */ 
    { 0xD4, 0x31, 0x73, 0xAC }, /* 214d */ 
    { 0xD4, 0x31, 0x73, 0xAC }, /* 215d */ 
    { 0xD4, 0x31, 0x73, 0xAC }, /* 216d */ 
    { 0xD4, 0x31, 0x73, 0xAC }, /* 217d */ 
    { 0xDC, 0x31, 0x73, 0xAC }, /* 218d */ 
    { 0xDC, 0x31, 0x73, 0xAC }, /* 219d */ 
    { 0xDC, 0x31, 0x73, 0xAC }, /* 220d */ 
    { 0xDC, 0x31, 0x73, 0xAC }, /* 221d */ 
    { 0xDC, 0x31, 0x73, 0xAC }, /* 222d */ 
    { 0xDC, 0x29, 0x73, 0xAC }, /* 223d */ 
    { 0xDC, 0x29, 0x73, 0xAC }, /* 224d */ 
    { 0xDC, 0x29, 0x83, 0xAC }, /* 225d */ 
    { 0xE4, 0x29, 0x83, 0xAC }, /* 226d */ 
    { 0xE4, 0x29, 0x83, 0xAC }, /* 227d */ 
    { 0xE4, 0x29, 0x83, 0xAC }, /* 228d */ 
    { 0xE4, 0x29, 0x83, 0xAC }, /* 229d */ 
    { 0xE4, 0x29, 0x83, 0xAC }, /* 230d */ 
    { 0xE4, 0x29, 0x83, 0xAC }, /* 231d */ 
    { 0xE4, 0x29, 0x83, 0xAC }, /* 232d */ 
    { 0xE4, 0x29, 0x83, 0xAC }, /* 233d */ 
    { 0xE4, 0x21, 0x83, 0xAC }, /* 234d */ 
    { 0xEC, 0x21, 0x93, 0xAC }, /* 235d */ 
    { 0xEC, 0x21, 0x93, 0xAC }, /* 236d */ 
    { 0xEC, 0x21, 0x93, 0xAC }, /* 237d */ 
    { 0xEC, 0x21, 0x93, 0xAC }, /* 238d */ 
    { 0xEC, 0x21, 0x93, 0xAC }, /* 239d */ 
    { 0xEC, 0x21, 0x93, 0xAC }, /* 240d */ 
    { 0xEC, 0x21, 0x93, 0xAC }, /* 241d */ 
    { 0xEC, 0x21, 0x93, 0xAC }, /* 242d */ 
    { 0xF4, 0x21, 0x93, 0xAC }, /* 243d */ 
    { 0xF4, 0x21, 0x93, 0xAC }, /* 244d */ 
    { 0xF4, 0x19, 0x93, 0xAC }, /* 245d */ 
    { 0xF4, 0x19, 0xA3, 0xAC }, /* 246d */ 
    { 0xF4, 0x19, 0xA3, 0xAC }, /* 247d */ 
    { 0xF4, 0x19, 0xA3, 0xAC }, /* 248d */ 
    { 0xF4, 0x19, 0xA3, 0xAC }, /* 249d */ 
    { 0xF4, 0x19, 0xA3, 0xAC }, /* 250d */ 
    { 0xFC, 0x19, 0xA3, 0xAC }, /* 251d */ 
    { 0xFC, 0x19, 0xA3, 0xAC }, /* 252d */ 
    { 0xFC, 0x19, 0xA3, 0xAC }, /* 253d */ 
    { 0xFC, 0x19, 0xA3, 0xAC }, /* 254d */ 
    { 0xFC, 0x19, 0xA3, 0xAC }  /* 255d */ 
};


/*
 ******************************************************************************
			  FUNCTION PROTOTYPES
 ******************************************************************************
*/
static void ClearTemplate(sdla_fe_t* fe);
static unsigned char PrgTransmitTemplate(sdla_fe_t* fe);
static void InitLineReceiver(sdla_fe_t* fe, RLPS_EQUALIZER_RAM* rlps_table);

static void ClearTPSCReg(sdla_fe_t* fe);
static void ClearRPSCReg(sdla_fe_t* fe);
	
static int WriteTPSCReg(sdla_fe_t*, int reg, int channel, unsigned char value);
static unsigned char ReadTPSCReg(sdla_fe_t* fe, int reg, int channel);

static int WriteRPSCReg(sdla_fe_t*, int reg, int channel, unsigned char value);
static unsigned char ReadRPSCReg(sdla_fe_t* fe, int reg, int channel);

#if 0
static int WriteSIGXReg(sdla_fe_t*, int reg, int channel, unsigned char value);
#endif
static unsigned char ReadSIGXReg(sdla_fe_t* fe, int reg, int channel);

#if 0
static void sdla_channels(void*, unsigned long);
#endif
static void DisableAllChannels(sdla_fe_t* fe);
static void EnableAllChannels(sdla_fe_t* fe);
static int DisableTxChannel(sdla_fe_t* fe, int channel);
static int DisableRxChannel(sdla_fe_t* fe, int channel);
static int EnableTxChannel(sdla_fe_t* fe, int channel);
static int EnableRxChannel(sdla_fe_t* fe, int channel);

static int sdla_te_post_config(sdla_fe_t* fe);

static void sdla_te_set_intr(sdla_fe_t* fe);
static void sdla_te_clear_intr(sdla_fe_t* fe);
static void sdla_te_tx_intr(sdla_fe_t*); 
static void sdla_te_rx_intr(sdla_fe_t*); 
static void sdla_t1_rx_intr(sdla_fe_t*); 
static void sdla_e1_rx_intr(sdla_fe_t*); 

static int sdla_te_intr(sdla_fe_t *); 
static int sdla_te_check_intr(sdla_fe_t *); 
static int sdla_te_polling(sdla_fe_t*);
static int sdla_te_udp(sdla_fe_t*, void*, unsigned char*);
static void sdla_te_set_status(sdla_fe_t*, unsigned long);
static int sdla_te_get_snmp_data(sdla_fe_t* fe, void* pdev, void* data);
static int sdla_te_set_lbmode(sdla_fe_t*, unsigned char, unsigned char);

static void sdla_te_enable_timer(sdla_fe_t*, unsigned char, unsigned long); 

static int sdla_te_linelb(sdla_fe_t*, unsigned char);
static int sdla_te_paylb(sdla_fe_t*, unsigned char);
static int sdla_te_ddlb(sdla_fe_t*, unsigned char);
static int sdla_te_lb(sdla_fe_t*, unsigned char);
#if defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
static void sdla_te_timer(void*);
#elif defined(__WINDOWS__)
static void sdla_te_timer(IN PKDPC,void*,void*,void*);
#else
static void sdla_te_timer(unsigned long);
#endif
static int sdla_te_check_rbsbits(sdla_fe_t*, int, unsigned long, int);
static unsigned char sdla_te_read_rbsbits(sdla_fe_t*, int, int);
static int sdla_te_set_rbsbits(sdla_fe_t*, int, unsigned char);
static int sdla_te_rbs_report(sdla_fe_t* fe);
static int sdla_te_rbs_print(sdla_fe_t* fe, int);

#if 0
static void sdla_te_abcd_update(sdla_fe_t* fe);
#endif

static int sdla_te_pmon(sdla_fe_t *fe);
static int sdla_te_flush_pmon(sdla_fe_t *fe);
static unsigned long sdla_te_read_alarms(sdla_fe_t *fe, int);
static int sdla_te_print_alarms(sdla_fe_t* fe, unsigned long);
static char* sdla_te_print_channels(sdla_fe_t* fe);
static int sdla_te_set_alarms(sdla_fe_t* pfe, unsigned long);
static int sdla_te_clear_alarms(sdla_fe_t* pfe, unsigned long);
static int sdla_te_sigctrl(sdla_fe_t*, int);

static int sdla_te_update_alarm_info(sdla_fe_t*, struct seq_file*, int*);
static int sdla_te_update_pmon_info(sdla_fe_t*, struct seq_file*, int*);

static int sdla_te_led_ctrl(sdla_fe_t *fe, int mode);
/*
 ******************************************************************************
			  FUNCTION DEFINITIONS
 ******************************************************************************
*/
/******************************************************************************
 *				sdla_te3_get_fe_status()	
 *
 * Description:
 * Arguments:	
 * Returns:
 ******************************************************************************
 */
static char* sdla_te_get_fe_media_string(void)
{
	return ("S514/AFT T1/E1");
}

/******************************************************************************
 *				sdla_te3_get_fe_status()	
 *
 * Description:
 * Arguments:	
 * Returns:
 ******************************************************************************
 */
static unsigned char sdla_te_get_fe_media(sdla_fe_t *fe)
{
	return fe->fe_cfg.media;
}

/******************************************************************************
 *				sdla_te3_get_fe_status()	
 *
 * Description:
 * Arguments:	
 * Returns:
 ******************************************************************************
 */
static int sdla_te_get_fe_status(sdla_fe_t *fe, unsigned char *status)
{
	*status = fe->fe_status;
	return 0;
}

/*
 ******************************************************************************
 *						ClearTemplate()	
 *
 * Description: 
 * Arguments:   None.
 * Returns:     None.
 ******************************************************************************
 */
static void ClearTemplate(sdla_fe_t* fe)
{
	int		i = 0, j = 0;
	unsigned char	indirect_addr = 0x00;

	for(i = FIRST_UI; i <= LAST_UI; i++) {
		for(j = FIRST_SAMPLE; j <= LAST_SAMPLE; j++) {
			indirect_addr = (j << 3) | i;
			/* Set up the indirect address */
			WRITE_REG(REG_XLPG_WAVEFORM_ADDR, indirect_addr);
			WRITE_REG(REG_XLPG_WAVEFORM_DATA, 0x00);
		}
	}
}

/*
 ******************************************************************************
 *			PrgTransmitTemplate()	
 *
 * Description:
 * Arguments:   None.
 * Returns:     None.
 ******************************************************************************
 */
static unsigned char PrgTransmitTemplate(sdla_fe_t* fe)
{
	int i = 0, j = 0;
	unsigned char indirect_addr = 0x00, xlpg_scale = 0x00;
	TX_WAVEFORM* tx_waveform = NULL;

	if (IS_T1_FEMEDIA(fe)) {
		switch(WAN_TE1_LBO(fe)) {
		case WAN_T1_LBO_0_DB:
			if (fe->fe_chip_id == CHIP_ID_COMET_QUAD){
				tx_waveform = &pmc4_t1_tx_waveform_lh_0db;
				/* ALEX Jan 24, 2005
				** XLPG scale value is change to 0x0D
				** (certification). Original value is 0x0C. */
				xlpg_scale = 0x0D;
			}else{
				tx_waveform = &pmc_t1_tx_waveform_lh_0db;
				xlpg_scale = 0x0C;
			}
			break;
		case WAN_T1_LBO_75_DB:
			if (fe->fe_chip_id == CHIP_ID_COMET_QUAD){
				tx_waveform = &pmc4_t1_tx_waveform_lh_75db;
				xlpg_scale = 0x08;
			}else{
				tx_waveform = &pmc_t1_tx_waveform_lh_75db;
				xlpg_scale = 0x07;
			}
			break;
		case WAN_T1_LBO_15_DB:
			if (fe->fe_chip_id == CHIP_ID_COMET_QUAD){
				tx_waveform = &pmc4_t1_tx_waveform_lh_15db;
			}else{
				tx_waveform = &pmc4_t1_tx_waveform_lh_15db;
			}
			xlpg_scale = 0x03;
			break;
		case WAN_T1_LBO_225_DB:
			if (fe->fe_chip_id == CHIP_ID_COMET_QUAD){
				tx_waveform = &pmc4_t1_tx_waveform_lh_225db;
			}else{
				tx_waveform = &pmc_t1_tx_waveform_lh_225db;
			}
			xlpg_scale = 0x02;
			break;
		case WAN_T1_0_110:
			if (fe->fe_chip_id == CHIP_ID_COMET_QUAD){
				tx_waveform = &pmc4_t1_tx_waveform_sh_110ft;
				xlpg_scale = 0x0D;
			}else{
				tx_waveform = &pmc_t1_tx_waveform_sh_110ft;
				xlpg_scale = 0x0C;
			}
			break;
		case WAN_T1_110_220:
			if (fe->fe_chip_id == CHIP_ID_COMET_QUAD){
				tx_waveform = &pmc4_t1_tx_waveform_sh_220ft;
				xlpg_scale = 0x11;
			}else{
				tx_waveform = &pmc_t1_tx_waveform_sh_220ft;
				xlpg_scale = 0x10;
			}
			break;
		case WAN_T1_220_330:
			if (fe->fe_chip_id == CHIP_ID_COMET_QUAD){
				tx_waveform = &pmc4_t1_tx_waveform_sh_330ft;
				xlpg_scale = 0x12;
			}else{
				tx_waveform = &pmc_t1_tx_waveform_sh_330ft;
				xlpg_scale = 0x11;
			}
			break;
		case WAN_T1_330_440:
			if (fe->fe_chip_id == CHIP_ID_COMET_QUAD){
				tx_waveform = &pmc4_t1_tx_waveform_sh_440ft;
				xlpg_scale = 0x13;
			}else{
				tx_waveform = &pmc_t1_tx_waveform_sh_440ft;
				xlpg_scale = 0x12;
			}
			break;
		case WAN_T1_440_550:
			if (fe->fe_chip_id == CHIP_ID_COMET_QUAD){
				tx_waveform = &pmc4_t1_tx_waveform_sh_550ft;
				xlpg_scale = 0x15;
			}else{
				tx_waveform = &pmc_t1_tx_waveform_sh_550ft;
				xlpg_scale = 0x14;
			}
			break;
		case WAN_T1_550_660:
			if (fe->fe_chip_id == CHIP_ID_COMET_QUAD){
				tx_waveform = &pmc4_t1_tx_waveform_sh_660ft;
			}else{
				tx_waveform = &pmc_t1_tx_waveform_sh_660ft;
			}
			xlpg_scale = 0x15;
			break;
		default:	
			/* Use 0DB as a default value */
			if (fe->fe_chip_id == CHIP_ID_COMET_QUAD){
				tx_waveform = &pmc4_t1_tx_waveform_lh_0db;
				xlpg_scale = 0x0D;
			}else{
				tx_waveform = &pmc_t1_tx_waveform_lh_0db;
				xlpg_scale = 0x0C;
			}
			break;
		}
	} else {
		if (fe->fe_chip_id == CHIP_ID_COMET_QUAD){
			tx_waveform = &pmc4_e1_tx_waveform_120;
		}else{
			tx_waveform = &pmc_e1_tx_waveform_120;
		}
		xlpg_scale = 0x0C;
		/*xlpg_scale = 0x0B; */
	}

#if 0
/* By default, HIGHZ bit is set to 0 */
	if (fe->fe_chip_id == CHIP_ID_COMET_QUAD){
		if (IS_FE_TXTRISTATE(fe)){
			WRITE_REG(REG_XLPG_LINE_CFG, BIT_XLPG_LINE_CFG_HIGHZ);
		}else{
			WRITE_REG(REG_XLPG_LINE_CFG, 
				xlpg_scale | BIT_XLPG_LINE_CFG_HIGHZ);
		}
	}
#endif
	for(i = FIRST_UI; i <= LAST_UI; i++){
		for(j = FIRST_SAMPLE; j <= LAST_SAMPLE; j++){
			indirect_addr = (j << 3) | i;
			/* Set up the indirect address */
			WRITE_REG(REG_XLPG_WAVEFORM_ADDR, indirect_addr);
			WRITE_REG(REG_XLPG_WAVEFORM_DATA, (*tx_waveform)[j][i]);
		}
	}
	return xlpg_scale;
}

/*
 ******************************************************************************
 *				FuseStabilizationProc()	
 *
 * Description:	Fuse stabilization procedure.
 * Arguments:	
 * Returns:	None.
 ******************************************************************************
 */
static void FuseStabilizationProc(sdla_fe_t* fe)
{
	unsigned char	value;
	
	/* XLPG Analog Test Negative Control (0xF4) */
	if (fe->fe_chip_id == CHIP_ID_COMET_QUAD){
		WRITE_REG(REG_XLPG_INIT, 0x00);
	}
       	WRITE_REG(REG_XLPG_TNC, 0x01);
       	WRITE_REG(REG_XLPG_TNC, 0x01);
	value = READ_REG(REG_XLPG_TNC) & 0xFE;
	WRITE_REG(REG_XLPG_TNC, value);

	/* XLPG Analog Test Positive Control (0xF5) */
	if (fe->fe_chip_id == CHIP_ID_COMET_QUAD){
		WRITE_REG(REG_XLPG_INIT, 0x00);
	}
       	WRITE_REG(REG_XLPG_TPC, 0x01);
       	WRITE_REG(REG_XLPG_TPC, 0x01);
	value = READ_REG(REG_XLPG_TPC) & 0xFE;
	WRITE_REG(REG_XLPG_TPC, value);

	WRITE_REG(REG_XLPG_INIT, 0x01);
}

/*
 ******************************************************************************
 *				InitLineReceiver()	
 *
 * Description:
 * Arguments:   is_e1 - WAN_TRUE for E1 connection, WAN_FALSE for T1 connection.
 * Returns:     None.
 ******************************************************************************
 */
static void InitLineReceiver(sdla_fe_t* fe, RLPS_EQUALIZER_RAM* rlps_table)
{
	int			ram_addr = 0x00;
	
	if (IS_T1_FEMEDIA(fe)){
		if (WAN_TE1_HI_MODE(fe) == WANOPT_YES){
			DEBUG_EVENT("%s: Configure for High-Impedance Mode!\n",
					fe->name);
			rlps_table = pmc_t1_rlps_perf_mode_ram_table;
		}
	}
	for(ram_addr = 0; ram_addr <= 255; ram_addr++) {
/* ERRATA VVV	*/
		/* RLPS Equalizaton Read/WriteB Select (Reg. 0xQFD)
		 * Perform dummy read */
		WRITE_REG(REG_RLPS_EQ_RWB, BIT_RLPS_EQ_RWB);
		/* RLPS Eqialization Indirect Address (Reg. 0xQFC) */
		WRITE_REG(REG_RLPS_EQ_ADDR, (unsigned char)ram_addr);
		WP_DELAY(100);
/* ERRATA ^^^	*/
		/* RLPS Equalization Indirect Data MSB (Reg. 0xQD8) */
		WRITE_REG(REG_RLPS_IND_DATA_1, rlps_table[ram_addr].byte1);
		/* RLPS Equalization Indirect Data (Reg. 0xQD9) */
		WRITE_REG(REG_RLPS_IND_DATA_2, rlps_table[ram_addr].byte2);
		/* RLPS Equalization Indirect Data (Reg. 0xQDA) */
		WRITE_REG(REG_RLPS_IND_DATA_3, rlps_table[ram_addr].byte3);
		/* RLPS Equalization Indirect Data LSB (Reg. 0xQDB) */
		WRITE_REG(REG_RLPS_IND_DATA_4, rlps_table[ram_addr].byte4);
		/* RLPS Equalizaton Read/WriteB Select (Reg. 0xQFD) */
		WRITE_REG(REG_RLPS_EQ_RWB, 0x00);
		/* RLPS Eqialization Indirect Address (Reg. 0xQFC) */
		WRITE_REG(REG_RLPS_EQ_ADDR, (unsigned char)ram_addr);
/* ERRATA VVV	*/
		WP_DELAY(100);
/* ERRATA ^^^	*/
	}
}

/*
 ******************************************************************************
 *			ClearTPSCReg()	
 *
 * Description: Clear all TPSC indirect register.
 * Arguments:   None.
 * Returns:	None
 ******************************************************************************
 */
static void ClearTPSCReg(sdla_fe_t* fe)
{
	int channel = 0;
	int start_channel = 0, stop_channel = 0;

	if (IS_E1_FEMEDIA(fe)){
		start_channel = 0;
		stop_channel = NUM_OF_E1_TIMESLOTS + 1;
	}else{
		start_channel = 1;
		stop_channel = NUM_OF_T1_CHANNELS;
	}

	for(channel = start_channel; channel <= stop_channel; channel++){
		WRITE_TPSC_REG(REG_TPSC_DATA_CTRL_BYTE, channel, 0x00);
		WRITE_TPSC_REG(REG_TPSC_IDLE_CODE_BYTE, channel, 0x00);

		if (IS_T1_FEMEDIA(fe)){
			if (fe->fe_cfg.cfg.te_cfg.te_rbs_ch & (1 << (channel -1))){
				WRITE_TPSC_REG(REG_TPSC_SIGNALING_BYTE, channel,
					BIT_TPSC_SIGBYTE_B|
					BIT_TPSC_SIGBYTE_D|
					BIT_TPSC_SIGBYTE_SIGC_0 | 
					BIT_TPSC_SIGBYTE_SIGC_1);

			}else{
				WRITE_TPSC_REG(REG_TPSC_SIGNALING_BYTE, channel, 0x00);
			}
		}else{
			WRITE_TPSC_REG(REG_TPSC_E1_CTRL_BYTE, channel, 0x00);
		}

		/* Do not initialize Signalling Bits 
		** WRITE_TPSC_REG(REG_TPSC_SIGNALING_BYTE, channel, 0x00); */
	}
	return;
}

/*
 ******************************************************************************
 *			ClearRPSCReg()	
 *
 * Description: Clear all RPSC indirect register.
 * Arguments:   None.
 * Returns:	None
 ******************************************************************************
 */
static void ClearRPSCReg(sdla_fe_t* fe)
{
	int channel = 0;
	int start_channel = 0, stop_channel = 0;

	if (IS_E1_FEMEDIA(fe)){
		start_channel = 0;
		stop_channel = NUM_OF_E1_TIMESLOTS + 1;
	}else{
		start_channel = 1;
		stop_channel = NUM_OF_T1_CHANNELS;
	}

	for(channel = start_channel; channel <= stop_channel; channel++){
		WRITE_RPSC_REG(REG_RPSC_DATA_CTRL_BYTE, channel, 0x00);
		WRITE_RPSC_REG(REG_RPSC_DATA_COND_BYTE, channel, 0x00);
		WRITE_RPSC_REG(REG_RPSC_SIGBYTE, channel, 0x00);
	}
	return;
}

/*
 ******************************************************************************
 *						WriteTPSCReg()	
 *
 * Description: Write value to TPSC indirect register.
 * Arguments:   reg   - Offset in TPSC indirect space.
 *				value - New PMC register value.
 * Returns:		None
 ******************************************************************************
 */
static int 
WriteTPSCReg(sdla_fe_t* fe, int reg, int channel, unsigned char value)
{
	unsigned char 	temp = 0x00;
	int 		i = 0, busy_flag = 0;
	int 		err = WAN_FALSE;

	reg += channel;
	/* Set IND bit to 1 in TPSC to enable indirect access to TPSC register */
	WRITE_REG(REG_TPSC_CFG, BIT_TPSC_IND); 
	busy_flag = 1;
	for(i = 0; i < MAX_BUSY_READ; i++) {
		temp = READ_REG(REG_TPSC_MICRO_ACCESS_STATUS);
		if ((temp & BIT_TPSC_BUSY) == 0x0){ 
			busy_flag = 0;	
			break;
		}
	}
	if (busy_flag == 1){
		DEBUG_EVENT("%s: Failed to write value to TPSC Reg=%02x, val=%02x!\n",
					fe->name, reg, value);
		goto write_tpsc_done;
	}

	WRITE_REG(REG_TPSC_CHANNEL_INDIRECT_DATA_BUFFER, (unsigned char)value);
	WRITE_REG(REG_TPSC_CHANNEL_INDIRECT_ADDRESS_CONTROL, (unsigned char)(reg & 0x7F));

	for(i = 0; i < MAX_BUSY_READ; i++) {
		temp = READ_REG(REG_TPSC_MICRO_ACCESS_STATUS);
		if ((temp & BIT_TPSC_BUSY) == 0x0){
			err = WAN_TRUE;
			goto write_tpsc_done;
		}
	}
	DEBUG_EVENT("%s: Failed to write value to TPSC Reg=%02x, val=%02x.\n",
				fe->name, reg, value);
write_tpsc_done:
	/* Set PCCE bit to 1 in TPSC to enable modifing the TPSC register */
	WRITE_REG(REG_TPSC_CFG, BIT_TPSC_IND | BIT_TPSC_PCCE);
	return err;
}

/*
 ******************************************************************************
 *						ReadTPSCReg()	
 *
 * Description: Read value from TPSC indirect register.
 * Arguments:   reg   - Offset in TPSC indirect space.
 * Returns:		Returns register value.
 ******************************************************************************
 */
static unsigned char ReadTPSCReg(sdla_fe_t* fe, int reg, int channel)
{
	unsigned char 	tmp = 0x00, value = 0x00;
	int 		i = 0, busy_flag = 0;

	reg += channel;
	/* Set IND bit to 1 in TPSC to enable indirect access to TPSC register */
	WRITE_REG(REG_TPSC_CFG, BIT_TPSC_IND); 
	busy_flag = 1;
	for(i = 0; i < MAX_BUSY_READ; i++) {
		tmp = READ_REG(REG_TPSC_MICRO_ACCESS_STATUS);
		if ((tmp & BIT_TPSC_BUSY) == 0x0){ 
			busy_flag = 0;	
			break;
		}
	}
	if (busy_flag == 1){
		DEBUG_EVENT("%s: Failed to read value to TPSC Reg=%02x!\n",
					fe->name, reg);
		goto read_tpsc_done;
	}

	WRITE_REG(REG_TPSC_CHANNEL_INDIRECT_ADDRESS_CONTROL, 
					(unsigned char)(reg | 0x80));

	for(i = 0; i < MAX_BUSY_READ; i++) {
		tmp = READ_REG(REG_TPSC_MICRO_ACCESS_STATUS);
		if ((tmp & BIT_TPSC_BUSY) == 0x0){
			value = READ_REG(REG_TPSC_CHANNEL_INDIRECT_DATA_BUFFER);
			goto read_tpsc_done;
		}
	}
	DEBUG_EVENT("%s: Failed to read value to TPSC Reg=%02x.\n",
					fe->name, reg);
read_tpsc_done:
	/* Set PCCE bit to 1 in TPSC to enable modifing the TPSC register */
	WRITE_REG(REG_TPSC_CFG, BIT_TPSC_IND | BIT_TPSC_PCCE);
	return value;
}

/*
 ******************************************************************************
 *						WriteRPSCReg()	
 *
 * Description: Write value to RPSC indirect register.
 * Arguments:   reg   - Offset in RPSC indirect space.
 *		value - New PMC register value.
 * Returns:		None
 ******************************************************************************
 */
static int 
WriteRPSCReg(sdla_fe_t* fe, int reg, int channel, unsigned char value)
{
	unsigned char 	temp = 0x00;
	int 		i = 0, busy_flag = 0;
	int 		err = WAN_FALSE;

	reg += channel;
	/* Set IND bit to 1 in RPSC to enable indirect access to RPSC register*/
	WRITE_REG(REG_RPSC_CFG, BIT_RPSC_IND); 
	busy_flag = 1;
	for(i = 0; i < MAX_BUSY_READ; i++) {
		temp = READ_REG(REG_RPSC_MICRO_ACCESS_STATUS);
		if ((temp & BIT_RPSC_BUSY) == 0x0){ 
			busy_flag = 0;	
			break;
		}
	}
	if (busy_flag == 1){
		DEBUG_EVENT("%s: Failed to write value to RPSC Reg=%02x, val=%02x!\n",
						fe->name, reg, value);
		goto write_rpsc_done;
	}

	WRITE_REG(REG_RPSC_CHANNEL_INDIRECT_DATA_BUFFER, (unsigned char)value);
	WRITE_REG(REG_RPSC_CHANNEL_INDIRECT_ADDRESS_CONTROL, (unsigned char)(reg & 0x7F));

	for(i = 0; i < MAX_BUSY_READ; i++) {
		temp = READ_REG(REG_RPSC_MICRO_ACCESS_STATUS);
		if ((temp & BIT_RPSC_BUSY) == 0x0){
			err = WAN_TRUE;
			goto write_rpsc_done;
		}
	}
	DEBUG_EVENT("%s: Failed to write value to RPSC Reg=%02x, val=%02x.\n",
						fe->name, reg, value);
write_rpsc_done:
	/* Set PCCE bit to 1 in RPSC to enable modifing the RPSC register */
	WRITE_REG(REG_RPSC_CFG, BIT_RPSC_IND | BIT_RPSC_PCCE);
	return err;
}

/*
 ******************************************************************************
 *						ReadRPSCReg()	
 *
 * Description: Read value from RPSC indirect register.
 * Arguments:   reg   - Offset in RPSC indirect space.
 * Returns:		Returns register value.
 ******************************************************************************
 */
static unsigned char ReadRPSCReg(sdla_fe_t* fe, int reg, int channel)
{
	unsigned char 	tmp = 0x00, value = 0x00;
	int 		i = 0, busy_flag = 0;

	reg += channel;
	/* Set IND bit to 1 in RPSC to enable indirect access to RPSC register*/
	WRITE_REG(REG_RPSC_CFG, BIT_RPSC_IND); 
	busy_flag = 1;
	for(i = 0; i < MAX_BUSY_READ; i++) {
		tmp = READ_REG(REG_RPSC_MICRO_ACCESS_STATUS);
		if ((tmp & BIT_RPSC_BUSY) == 0x0){ 
			busy_flag = 0;	
			break;
		}
	}
	if (busy_flag == 1){
		DEBUG_EVENT("%s: Failed to read value to RPSC Reg=%02x!\n",
						fe->name, reg);
		goto read_rpsc_done;
	}

	WRITE_REG(REG_RPSC_CHANNEL_INDIRECT_ADDRESS_CONTROL, 
					(unsigned char)(reg | 0x80));

	for(i = 0; i < MAX_BUSY_READ; i++) {
		tmp = READ_REG(REG_RPSC_MICRO_ACCESS_STATUS);
		if ((tmp & BIT_RPSC_BUSY) == 0x0){
			value = READ_REG(REG_RPSC_CHANNEL_INDIRECT_DATA_BUFFER);
		goto read_rpsc_done;
		}
	}
	DEBUG_EVENT("%s: Failed to read value to RPSC Reg=%02x.\n",
						fe->name, reg);
read_rpsc_done:
	/* Set PCCE bit to 1 in RPSC to enable modifing the RPSC register */
	WRITE_REG(REG_RPSC_CFG, BIT_RPSC_IND | BIT_RPSC_PCCE);
	return value;
}

/*
 ******************************************************************************
 *				WriteSIGXReg()	
 *
 * Description: Write value from SIGX indirect register.
 * Arguments:   reg   - Offset in SIGX indirect space.
 *		value - New PMC register value.
 * Returns:		Returns register value.
 ******************************************************************************
 */
#if 0
static int WriteSIGXReg(sdla_fe_t* fe, int reg, int channel, unsigned char value)
{
	unsigned char 	temp = 0x00;
	int 		i = 0, busy_flag = 0;
	int 		err = WAN_FALSE;

	reg += (IS_T1_FEMEDIA(fe) ? (channel - 1) : channel);
	temp = READ_REG(REG_SIGX_CFG);
	WRITE_REG(REG_SIGX_CFG, (temp | BIT_SIGX_IND) & ~(BIT_SIGX_PCCE|BIT_SIGX_COSS));
	
	busy_flag = 1;
	for(i = 0; i < MAX_BUSY_READ; i++) {
		temp = READ_REG(REG_SIGX_TIMESLOT_IND_STATUS);
		if ((temp & BIT_SIGX_BUSY) == 0x0){ 
			busy_flag = 0;	
			break;
		}
	}
	if (busy_flag == 1){
		DEBUG_EVENT("%s: Failed to write value to RPSC Reg=%02x, val=%02x!\n",
						fe->name, reg, value);
		goto write_sigx_done;
	}

	WRITE_REG(REG_SIGX_TIMESLOT_IND_DATA_BUFFER, (unsigned char)value);
	WRITE_REG(REG_SIGX_TIMESLOT_IND_ACCESS, (unsigned char)(reg & 0x7F));

	for(i = 0; i < MAX_BUSY_READ; i++) {
		temp = READ_REG(REG_SIGX_TIMESLOT_IND_STATUS);
		if ((temp & BIT_SIGX_BUSY) == 0x0){
			err = WAN_TRUE;
			goto write_sigx_done;
		}
	}
	DEBUG_EVENT("%s: Failed to write value to RPSC Reg=%02x, val=%02x.\n",
						fe->name, reg, value);
write_sigx_done:
	/* Set PCCE bit to 1 in SIGX to enable modifing the SIGX register */
	temp = READ_REG(REG_SIGX_CFG);
	WRITE_REG(REG_SIGX_CFG, temp | BIT_SIGX_PCCE);
	return err;
}
#endif

/*
 ******************************************************************************
 *				ReadSIGXReg()	
 *
 * Description: Read value from SIGX indirect register.
 * Arguments:   reg   - Offset in SIGX indirect space.
 * Returns:		Returns register value.
 ******************************************************************************
 */
static unsigned char ReadSIGXReg(sdla_fe_t* fe, int reg, int channel)
{
	unsigned char 	sigx_cfg = 0x00, tmp = 0x00, value = 0x00;
	int 		offset, i = 0, busy_flag = 0;

	if (IS_E1_FEMEDIA(fe)){
		if (channel == 0 || channel == 16){
			return 0;
		}
		offset = (channel > 16) ? channel - 16 : channel;
	}else{
		offset = (channel > 16) ? channel - 17 : channel - 1;
	}
	reg += offset;

	/* Enable indirect access to SIGX registers */
	sigx_cfg = READ_REG(REG_SIGX_CFG);
	WRITE_REG(REG_SIGX_CFG, READ_REG(REG_SIGX_CFG) & ~BIT_SIGX_COSS);

	/* Set IND bit to 1 in SIGX Configuration Register to enable 
	 * indirect access to SIGX register*/
	WRITE_REG(REG_SIGX_CFG, READ_REG(REG_SIGX_CFG) | BIT_SIGX_IND);
	
	busy_flag = 1;
	for(i = 0; i < MAX_BUSY_READ; i++) {
		tmp = READ_REG(REG_SIGX_TIMESLOT_IND_STATUS);
		if ((tmp & BIT_SIGX_BUSY) == 0x0){ 
			busy_flag = 0;	
			break;
		}
	}
	if (busy_flag == 1){
		DEBUG_EVENT("%s: Failed to read value from SIGX Reg=%02x!\n",
						fe->name, reg);
		goto read_sigx_done;
	}

	WRITE_REG(REG_SIGX_TIMESLOT_IND_ACCESS, 
			(unsigned char)(reg | BIT_SIGX_TS_IND_ACCESS_READ));

	for(i = 0; i < MAX_BUSY_READ; i++) {
		tmp = READ_REG(REG_SIGX_TIMESLOT_IND_STATUS);
		if ((tmp & BIT_SIGX_BUSY) == 0x0){
			value = READ_REG(REG_SIGX_TIMESLOT_IND_DATA_BUFFER);
			goto read_sigx_done;
		}
	}
	DEBUG_EVENT("%s: Failed to read value from SIGX Reg=%02x.\n",
						fe->name, reg);
read_sigx_done:
#if 0
	tmp = READ_REG(REG_SIGX_CFG);
	WRITE_REG(REG_SIGX_CFG, (tmp | BIT_SIGX_PCCE));
#endif
#if 0
	tmp = READ_REG(REG_SIGX_CFG);
	WRITE_REG(REG_SIGX_CFG, tmp & ~BIT_SIGX_IND);
	tmp = READ_REG(REG_SIGX_CFG);
	/* Disable indirect access to SIGX registers */
	WRITE_REG(REG_SIGX_CFG, tmp | BIT_SIGX_COSS);
#endif
	WRITE_REG(REG_SIGX_CFG, sigx_cfg);
	return value;
}


/*
 ******************************************************************************
 *			DisableAllChannels()	
 *
 * Description:
 * Arguments:
 * Returns:
 ******************************************************************************
 */
static void DisableAllChannels(sdla_fe_t* fe)
{
	int i = 0;

	if (IS_E1_FEMEDIA(fe)) {
		DisableTxChannel(fe, E1_FRAMING_TIMESLOT);
		DisableRxChannel(fe, E1_FRAMING_TIMESLOT);
		for(i = 1; i <= NUM_OF_E1_TIMESLOTS; i++){
			DisableTxChannel(fe, i);
			DisableRxChannel(fe, i);
		}
	}else{
		for(i = 1; i <= NUM_OF_T1_CHANNELS; i++){
			DisableTxChannel(fe, i);
			DisableRxChannel(fe, i);
		}
	}
}

/*
 ******************************************************************************
 *			EnableAllChannels()	
 *
 * Description:
 * Arguments:
 * Returns:
 ******************************************************************************
 */
static void EnableAllChannels(sdla_fe_t* fe)
{
	int i = 0;

	if (IS_E1_FEMEDIA(fe)) {
		int first_ts = 
			(WAN_FE_FRAME(fe) == WAN_FR_UNFRAMED) ? 0 : 1;

		DisableTxChannel(fe, E1_FRAMING_TIMESLOT);
		DisableRxChannel(fe, E1_FRAMING_TIMESLOT);
		for(i = first_ts; i <= NUM_OF_E1_TIMESLOTS; i++){
			EnableTxChannel(fe, i);
			EnableRxChannel(fe, i);
		}
	}else{
		for(i = 1; i <= NUM_OF_T1_CHANNELS; i++){
			EnableTxChannel(fe, i);
			EnableRxChannel(fe, i);
		}
	}
}

/*
 ******************************************************************************
 *				EnableTxChannel()	
 *
 * Description:
 * Arguments:
 * Returns:
 ******************************************************************************
 */
static int EnableTxChannel(sdla_fe_t* fe, int channel)	
{

	if (WAN_FE_LCODE(fe) == WAN_LCODE_AMI){
		/* ZCs=1 AMI*/
		WRITE_TPSC_REG(REG_TPSC_DATA_CTRL_BYTE, channel, 
			(((READ_TPSC_REG(REG_TPSC_DATA_CTRL_BYTE, channel) & MASK_TPSC_DATA_CTRL_BYTE) & 
			  	~BIT_TPSC_DATA_CTRL_BYTE_IDLE_DS0) | BIT_TPSC_DATA_CTRL_BYTE_ZCS1));
	}else{
		WRITE_TPSC_REG(REG_TPSC_DATA_CTRL_BYTE, channel, 
			((READ_TPSC_REG(REG_TPSC_DATA_CTRL_BYTE, channel) & MASK_TPSC_DATA_CTRL_BYTE) & 
			 	~(BIT_TPSC_DATA_CTRL_BYTE_IDLE_DS0 | BIT_TPSC_DATA_CTRL_BYTE_ZCS1 | BIT_TPSC_DATA_CTRL_BYTE_ZCS0)));
	}

	if (IS_E1_FEMEDIA(fe)){
		/* Set SUBS=DS[0]=DS[1]=0x0 - no change to PCM timeslot data */
		WRITE_TPSC_REG(REG_TPSC_E1_CTRL_BYTE, channel, 
			(READ_TPSC_REG(REG_TPSC_E1_CTRL_BYTE, channel) & 
			 	~(BIT_TPSC_E1_CTRL_BYTE_SUBS | BIT_TPSC_E1_CTRL_BYTE_DS0 | BIT_TPSC_E1_CTRL_BYTE_DS1)));
	}else{
		if (fe->fe_cfg.cfg.te_cfg.te_rbs_ch & (1 << (channel -1))){
			WRITE_TPSC_REG(REG_TPSC_SIGNALING_BYTE, channel,
				BIT_TPSC_SIGBYTE_B|
				BIT_TPSC_SIGBYTE_D|
				BIT_TPSC_SIGBYTE_SIGC_0 | 
				BIT_TPSC_SIGBYTE_SIGC_1);
		}else{
			WRITE_TPSC_REG(REG_TPSC_SIGNALING_BYTE, channel, 0x00);
		}
		/* Do not initialize Signalling Bits 
		** WRITE_TPSC_REG(REG_TPSC_SIGNALING_BYTE, channel, 0x00); */
	}

	/* Erase contents of IDLE code byte */
	WRITE_TPSC_REG(REG_TPSC_IDLE_CODE_BYTE, channel, 0x00);

	return 0;
}
/*
 ******************************************************************************
 *					EnableRxChannel()	
 *
 * Description:
 * Arguments:
 * Returns:
 ******************************************************************************
 */
static int EnableRxChannel(sdla_fe_t* fe, int channel)	
{

	/* Set DTRPC bit to 0 in RPSC */
	WRITE_RPSC_REG(REG_RPSC_DATA_CTRL_BYTE, channel,
		((READ_RPSC_REG(REG_RPSC_DATA_CTRL_BYTE, channel) & MASK_RPSC_DATA_CTRL_BYTE) & 
		 	~BIT_RPSC_DATA_CTRL_BYTE_DTRKC));

	return 0;
}

/*
 ******************************************************************************
 *				DisableTxChannel()	
 *
 * Description:
 * Arguments:
 * Returns:
 ******************************************************************************
 */
static int DisableTxChannel(sdla_fe_t* fe, int channel)
{

	/* Set IDLE_DS0 to 1 for an IDLE code byte will insert and 
	 * BTCLK will suppressed 
	 */
	WRITE_TPSC_REG(REG_RPSC_DATA_CTRL_BYTE, channel, 
			((READ_TPSC_REG(REG_TPSC_DATA_CTRL_BYTE, channel) & MASK_TPSC_DATA_CTRL_BYTE) | 
		 		BIT_TPSC_DATA_CTRL_BYTE_IDLE_DS0));
	if (IS_E1_FEMEDIA(fe)){
		/* Set SUBS=1, DS0=0 - data substitution on - IDLE code replaces BTPCM timeslot data */
		WRITE_TPSC_REG(REG_TPSC_E1_CTRL_BYTE, channel,
			((READ_TPSC_REG(REG_TPSC_E1_CTRL_BYTE, channel) & ~BIT_TPSC_E1_CTRL_BYTE_DS0) | 
			 	BIT_TPSC_E1_CTRL_BYTE_SUBS));
	}else{
		if (fe->fe_cfg.cfg.te_cfg.te_rbs_ch & (1 << (channel -1))){
			WRITE_TPSC_REG(REG_TPSC_SIGNALING_BYTE, channel,
				BIT_TPSC_SIGBYTE_B|
				BIT_TPSC_SIGBYTE_D|
				BIT_TPSC_SIGBYTE_SIGC_0 | 
				BIT_TPSC_SIGBYTE_SIGC_1);
		}else{
			WRITE_TPSC_REG(REG_TPSC_SIGNALING_BYTE, channel, 0x00);
		}
		/* Do not initialize Signalling Bits 
		** WRITE_TPSC_REG(REG_TPSC_SIGNALING_BYTE, channel, 0x00); */
	}
	/* Erase contents of IDLE code byte */
	WRITE_TPSC_REG(REG_TPSC_IDLE_CODE_BYTE, channel, 0x55);
	return 0;
}

/*
 ******************************************************************************
 *				DisableRxChannel()	
 *
 * Description:
 * Arguments:
 * Returns:
 ******************************************************************************
 */
static int DisableRxChannel(sdla_fe_t* fe, int channel)
{
	/* Set DTRPC bit to 1 in RPSC to hold low for the duration of the channel */
	WRITE_RPSC_REG(REG_RPSC_DATA_CTRL_BYTE, channel, 
	 	((READ_RPSC_REG(REG_RPSC_DATA_CTRL_BYTE, channel) & MASK_RPSC_DATA_CTRL_BYTE) | 
		 		BIT_RPSC_DATA_CTRL_BYTE_DTRKC));
 	 
	return 0;
}

/*
 ******************************************************************************
 *				sdla_te_copycfg()	
 *
 * Description: Copy T1/E1 configuration to card structure.
 * Arguments:	
 * Returns:
 ******************************************************************************
 */
int sdla_te_default_cfg(void* pfe, void* pfe_cfg, int media)
{
	sdla_fe_cfg_t	*fe_cfg = (sdla_fe_cfg_t*)pfe_cfg;
	
	/* Fill with the default values */
	switch(media){
	case WAN_MEDIA_E1:
		fe_cfg->media			= WAN_MEDIA_E1;
		fe_cfg->lcode			= WAN_LCODE_HDB3;
		fe_cfg->frame			= WAN_FR_NCRC4;
		fe_cfg->cfg.te_cfg.lbo		= WAN_E1_120;
		fe_cfg->cfg.te_cfg.te_clock	= WAN_NORMAL_CLK;
		fe_cfg->cfg.te_cfg.active_ch	= ENABLE_ALL_CHANNELS;
		break;
	case WAN_MEDIA_T1:
		fe_cfg->media			= WAN_MEDIA_T1;
		fe_cfg->lcode			= WAN_LCODE_B8ZS;
		fe_cfg->frame			= WAN_FR_ESF;
		fe_cfg->cfg.te_cfg.lbo		= WAN_T1_LBO_0_DB;
		fe_cfg->cfg.te_cfg.te_clock	= WAN_NORMAL_CLK;
		fe_cfg->cfg.te_cfg.active_ch	= ENABLE_ALL_CHANNELS;
		fe_cfg->cfg.te_cfg.high_impedance_mode	= WANOPT_NO;
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

/*
 ******************************************************************************
 *				sdla_te_copycfg()	
 *
 * Description: Copy T1/E1 configuration to card structure.
 * Arguments:	
 * Returns:
 ******************************************************************************
 */
int sdla_te_copycfg(void* pfe, void* pfe_cfg)
{
	sdla_fe_t	*fe = (sdla_fe_t*)pfe;
	sdla_fe_cfg_t	*fe_cfg = (sdla_fe_cfg_t*)pfe_cfg;
	
	if (fe_cfg){
		memcpy(&fe->fe_cfg, fe_cfg, sizeof(sdla_fe_cfg_t));
	}else{
		/* Fill with the default values */
		sdla_te_default_cfg(fe, &fe->fe_cfg, WAN_MEDIA_T1);
	}
	return 0;
}


/*
 ******************************************************************************
 *				sdla_te_global_config()	
 *
 * Description: Global configuration for Sangoma TE1 PMC board.
 * 		Note: 	These register should be program only once for AFT-QUAD
 * 			cards.
 * Arguments:	
 * Returns:	WANTRUE - TE1 configred successfully, otherwise WAN_FALSE.
 ******************************************************************************
 */
int sdla_te_global_config(void* pfe)
{
	sdla_fe_t	*fe = (sdla_fe_t*)pfe;
	unsigned char	value = 0x00;

	DEBUG_TE1("%s: %s Global Front End configuration\n", 
			fe->name, FE_MEDIA_DECODE(fe));
	/* 1. Initiate software reset of the COMET */
	/* Set RESET=1 to place COMET into RESET (Reg. 0xE) */
	WRITE_REG_LINE(0, REG_RESET, BIT_RESET);
	/* Set RESET=0, disable software reset. (Reg. 0xE)
	 * COMET in default mode. */
	WRITE_REG_LINE(0, REG_RESET, 0x0/*~BIT_RESET*/);  

	/* Program PMC for T1/E1 mode (Reg 0x00) */
	if (IS_E1_FEMEDIA(fe)){
		value = BIT_GLOBAL_PIO_OE | BIT_GLOBAL_E1;
	}else{
		value = BIT_GLOBAL_PIO_OE;
	}
	WRITE_REG_LINE(0, REG_GLOBAL_CFG, value); 

	/* Set system clock and XCLK (Reg 0xD6) */
	if (IS_T1_FEMEDIA(fe)){
		WRITE_REG_LINE(0, REG_CSU_CFG, BIT_CSU_MODE0); 
		/* WRITE_REG(REG_CSU_CFG, 
		 * 		BIT_CSU_MODE2 | 
		 * 		BIT_CSU_MODE1 | 
		 * 		BIT_CSU_MODE0); */
	}else{
		WRITE_REG_LINE(0, REG_CSU_CFG, 0x00);
	}

	/* 10 Jan 2004
	** This delay let PMC make proper RESET. */
	WP_DELAY(1000);

	return 0;
}

/*
 ******************************************************************************
 *				sdla_pmc4351_te_config()	
 *
 * Description: Configure Sangoma S5147/S5148/A101/A102 TE1 board
 * Arguments:	
 * Returns:	WANTRUE - TE1 configred successfully, otherwise WAN_FALSE.
 ******************************************************************************
 */
static short sdla_pmc4351_te_config(sdla_fe_t *fe, u16 adapter_type)
{
	unsigned char	value = 0x00;
	
	/* Execute global chip configuration */
	sdla_te_global_config(fe);
	
	/* 2.Setup the XLPG(Transmit pulse template) to clear the pulse
	** template */
	ClearTemplate(fe);
	fe->fe_param.te_param.xlpg_scale = PrgTransmitTemplate(fe);

	/* Global Configuration (Reg 0x00) */
	value = READ_REG(REG_GLOBAL_CFG);
	if (IS_E1_FEMEDIA(fe)){
		if (adapter_type & A101_ADPTR_TE1_MASK){
			value |= BIT_GLOBAL_TRKEN;
		}
	}else{
		if (adapter_type & A101_ADPTR_TE1_MASK){
			value |= BIT_GLOBAL_TRKEN;
		}
	}
	WRITE_REG(REG_GLOBAL_CFG, value);

	/* XLPG Line Driver Configuration (Reg. 0xF0) */
	if (IS_FE_TXTRISTATE(fe)){
		DEBUG_EVENT("%s: Disable Transmitter (tx tri-state mode)!\n",
					fe->name);
		WRITE_REG(REG_XLPG_LINE_CFG, BIT_XLPG_LINE_CFG_HIGHZ);
	}else{
		WRITE_REG(REG_XLPG_LINE_CFG, fe->fe_param.te_param.xlpg_scale);
	}

	/* CDRC Configuration (Reg. 0x10) */
	if (WAN_FE_LCODE(fe) == WAN_LCODE_AMI){
		WRITE_REG(REG_CDRC_CFG, BIT_CDRC_CFG_AMI); 
	}else{
		WRITE_REG(REG_CDRC_CFG, 0x00); 
	}

	if (IS_E1_FEMEDIA(fe)) {
		/* RX-ELST Configuration (Reg. 0x1C) */
		WRITE_REG(REG_RX_ELST_CFG, BIT_RX_ELST_IR | BIT_RX_ELST_OR);
		/* TX-ELST Configuration (Reg. 0x20) */
		WRITE_REG(REG_TX_ELST_CFG, BIT_TX_ELST_IR | BIT_RX_ELST_OR); 
	}else{
		/* RX-ELST Configuration (Reg. 0x1C) */
		WRITE_REG(REG_RX_ELST_CFG, 0x00); 
		/* TX-ELST Configuration (Reg. 0x20) */
		WRITE_REG(REG_TX_ELST_CFG, 0x00);
	}

	if (IS_E1_FEMEDIA(fe)){
		/* E1-TRAN Configuration (Reg. 0x80) */
		value = 0x00;
		if (WAN_FE_LCODE(fe) == WAN_LCODE_AMI){
			value |= BIT_E1_TRAN_AMI;
		}
		if (WAN_FE_FRAME(fe) == WAN_FR_CRC4){
			value |= BIT_E1_TRAN_GENCRC;
		}else if (WAN_FE_FRAME(fe) == WAN_FR_UNFRAMED){
			value |= BIT_E1_TRAN_FDIS;
		}
		if (WAN_TE1_SIG_MODE(fe) == WAN_TE1_SIG_CAS){
			value |= (BIT_E1_TRAN_SIGEN|BIT_E1_TRAN_DLEN);
		}
		WRITE_REG(REG_E1_TRAN_CFG, value);
		
		/* E1-FRMR Frame Alignment Options (Reg 0x90) */
		value = 0x00;
		if (WAN_FE_FRAME(fe) == WAN_FR_CRC4){
			value |= (BIT_E1_FRMR_CRCEN |
				  BIT_E1_FRMR_REFCRCEN);
		}
		if (WAN_TE1_SIG_MODE(fe) == WAN_TE1_SIG_CCS){
			value |= BIT_E1_FRMR_CASDIS;
		}
		WRITE_REG(REG_E1_FRMR_CFG, value);
		
	}else{
		/* T1-XBAS Configuration (Reg 0x54) */
		value = 0x00;
		if (WAN_FE_LCODE(fe) == WAN_LCODE_B8ZS){
			value |= BIT_T1_XBAS_B8ZS;
		}else{
			value |= BIT_T1_XBAS_ZCS0;
		}
		if (WAN_FE_FRAME(fe) == WAN_FR_ESF){
			value |= BIT_T1_XBAS_ESF;
		}
		if (IS_J1_FEMEDIA(fe)){
			value |= BIT_T1_XBAS_JPN;
		}
		WRITE_REG(REG_T1_XBAS_CFG, value);

		/* T1-FRMR Configuration (Reg. 0x48) */
		value = 0x00;
		if (WAN_FE_FRAME(fe) == WAN_FR_ESF){
			value = BIT_T1_FRMR_ESF | BIT_T1_FRMR_ESFFA;
		}
		if (IS_J1_FEMEDIA(fe)){
			value |= BIT_T1_FRMR_JPN;
		}
		WRITE_REG(REG_T1_FRMR_CFG, value);

		/* T1 ALMI Configuration (Reg. 0x60) */
		value = 0x00;
		if (WAN_FE_FRAME(fe) == WAN_FR_ESF){
			value = BIT_T1_ALMI_CFG_ESF;
		}
		WRITE_REG(REG_T1_ALMI_CFG, value);
	}

	/* SIGX Configuration Register (Reg. 0x50) */
	value = 0x00;
	if (IS_E1_FEMEDIA(fe)){
		WRITE_REG(REG_SIGX_CFG, value);
	}else{
		if (WAN_FE_FRAME(fe) == WAN_FR_ESF){
			value |= BIT_SIGX_ESF;
		}
		WRITE_REG(REG_SIGX_CFG, value);
	}

	/* BTIF Configuration (Reg. 0x40) */
	value = 0x00;
	if (IS_E1_FEMEDIA(fe)){
		value |= BIT_BTIF_RATE0;
	}
	if (!(adapter_type & A101_ADPTR_TE1_MASK)){
		if (WAN_FE_LCODE(fe) == WAN_LCODE_AMI){
			value |= BIT_BTIF_NXDS0_0;
		}
	}
	if (WAN_FE_FRAME(fe) != WAN_FR_UNFRAMED){
		value |= BIT_BTIF_NXDS0_1;
	}
	if (adapter_type & A101_ADPTR_TE1_MASK){
	 	value |= (BIT_BTIF_CMODE | BIT_BTIF_DE | BIT_BTIF_FE);
	}
	WRITE_REG(REG_BTIF_CFG, value);

	/* BTIF_Frame Pulse Configuration (Reg. 0x41) */
	WRITE_REG(REG_BTIF_FR_PULSE_CFG, BIT_BTIF_FPMODE); 

	/* BRIF Configuration (Reg. 0x30) */
	value = 0x00;
	if (IS_E1_FEMEDIA(fe)){
		value |= BIT_BRIF_RATE0;
	}
	if (!(adapter_type & A101_ADPTR_TE1_MASK)){
		if (WAN_FE_LCODE(fe) == WAN_LCODE_AMI){
			value |= BIT_BRIF_NXDS0_0;
		}
	}
	if (WAN_FE_FRAME(fe) != WAN_FR_UNFRAMED){
		value |= BIT_BRIF_NXDS0_1;
	}
	if (adapter_type & A101_ADPTR_TE1_MASK){
		value |= BIT_BRIF_CMODE;
	}
	WRITE_REG(REG_BRIF_CFG, value);
	
	/* BRIF Frame Pulse Configuration (Reg. 0x31) */
	value = 0x00;
	if (adapter_type & A101_ADPTR_TE1_MASK){
		value |= BIT_BRIF_FPMODE;
	}
	WRITE_REG(REG_BRIF_FR_PULSE_CFG, value); 
	
	/* BRIF Parity/F-bit Configuration (Reg. 0x32) */
	WRITE_REG(REG_BRIF_DATA_CFG, BIT_BRIF_DATA_TRI_0);

	/* Transmit Timing Options (Reg 0x06) */
	if (WAN_TE1_CLK(fe) == WAN_NORMAL_CLK){
		WRITE_REG(REG_TX_TIMING_OPT,
					BIT_TX_PLLREF1 |
					BIT_TX_TXELSTBYP);
	}else{
		WRITE_REG(REG_TX_TIMING_OPT,
					BIT_TX_PLLREF1 |
					BIT_TX_PLLREF0 |
					BIT_TX_TXELSTBYP);
	}

	/* RLPS Configuration and Status (Reg 0xF8) */
	WRITE_REG(REG_RLPS_CFG_STATUS, 
				BIT_RLPS_CFG_STATUS_LONGE |
				BIT_RLPS_CFG_STATUS_SQUELCHE);

	/* RLPS ALOS Detection/Clearance Thresholds (Reg 0xF9) */
	/* NC: Aug 20 2003:
	 *     Set the correct ALSO Detection/Clearance tresholds
	 *     for T1/E1 lines, to get rid of false ALOS alarms.
	 *
	 *     Original incorrect value set was 0x00, for both T1/E1 */
	if (IS_E1_FEMEDIA(fe)){
		WRITE_REG(REG_RLPS_ALOS_DET_CLR_THR, 
				BIT_RLPS_ALOS_DET_THR_2|
				BIT_RLPS_ALOS_DET_THR_1|
				BIT_RLPS_ALOS_DET_THR_0);
	}else{
		WRITE_REG(REG_RLPS_ALOS_DET_CLR_THR, 
				BIT_RLPS_ALOS_CLR_THR_2|
				BIT_RLPS_ALOS_CLR_THR_0|
				BIT_RLPS_ALOS_DET_THR_2|
				BIT_RLPS_ALOS_DET_THR_0);
	}

	/* RLPS ALOS Detection period (Reg 0xFA) */
	WRITE_REG(REG_RLPS_ALOS_DET_PER, BIT_RLPS_ALOS_DET_PER_0);
	
	/* RLPS ALOS Clearance period (Reg 0xFB) */
	WRITE_REG(REG_RLPS_ALOS_CLR_PER, BIT_RLPS_ALOS_CLR_PER_0);
	
	/* RLPS Equalizatopn Indirect Address (Reg. 0xFC) */
	/* ERRATA WRITE_REG(REG_RLPS_EQ_ADDR, 0x00); */
	
	/* RLPS Equalization Read/WriteB Select (Reg 0xFD) */
	/* ERRATA WRITE_REG(REG_RLPS_EQ_RWB, BIT_RLPS_EQ_RWB); */
	
	/* RLPS Equalizer Loop Status and Control (Reg 0xFE) */
	WRITE_REG(REG_RLPS_EQ_STATUS, 0x00);

	/* RLPS Equalizer Configuration (Reg 0xFF) */
	WRITE_REG(REG_RLPS_EQ_CFG, 
			BIT_RLPS_EQ_RESERVED |
			BIT_RLPS_EQ_FREQ_1 |
			BIT_RLPS_EQ_FREQ_0);

	/* TJAT Configuration (Reg 0x1B) */
	WRITE_REG(REG_TJAT_CFG, BIT_TJAT_CENT);

	/* RJAT Configuration (Reg 0x17) */
	WRITE_REG(REG_RJAT_CFG, BIT_RJAT_CENT);
	
	/* Receive Options (Reg 0x02) */
	if (WAN_FE_FRAME(fe) == WAN_FR_UNFRAMED){
		WRITE_REG(REG_RECEIVE_OPT, BIT_RECEIVE_OPT_UNF);
	}else{
		WRITE_REG(REG_RECEIVE_OPT, 0x00);
	}

	if (IS_E1_FEMEDIA(fe)){
		WRITE_REG(REG_E1_TRAN_NATB_CODESEL,
				BIT_E1_TRAN_NATB_CODESEL_Sa4);
		WRITE_REG(REG_E1_TRAN_NATB_CODE, 0xFF);

		WRITE_REG(REG_E1_TRAN_NATB_CODESEL,
				BIT_E1_TRAN_NATB_CODESEL_Sa5);
		WRITE_REG(REG_E1_TRAN_NATB_CODE, 0xFF);

		WRITE_REG(REG_E1_TRAN_NATB_CODESEL,
				BIT_E1_TRAN_NATB_CODESEL_Sa6);
		WRITE_REG(REG_E1_TRAN_NATB_CODE, 0xFF);

		WRITE_REG(REG_E1_TRAN_NATB_CODESEL,
				BIT_E1_TRAN_NATB_CODESEL_Sa7);
		WRITE_REG(REG_E1_TRAN_NATB_CODE, 0xFF);

		WRITE_REG(REG_E1_TRAN_NATB_CODESEL,
				BIT_E1_TRAN_NATB_CODESEL_Sa8);
		WRITE_REG(REG_E1_TRAN_NATB_CODE, 0xFF);
	}

	/* XLPG Configuration #1 (Reg 0xF4) */
	/* 13 Jan 2005 WRITE_REG(REG_XLPG_TNC, BIT_XLPG_TNC_1);	*/
	
	/* XLPG Configuration #2 (Reg 0xF5) */
	/* 13 Jan 2005 WRITE_REG(REG_XLPG_TPC, BIT_XLPG_TPC_1); */

	/* RLPS Equalizer Voltage Reference (Reg 0xDC) */
	if (IS_E1_FEMEDIA(fe)){
		WRITE_REG(REG_EQ_VREF, 0x34);
	}else{
		WRITE_REG(REG_EQ_VREF, 0x2C);
	}
	WRITE_REG(REG_RLPS_FUSE_CTRL_STAT, 0x00);

	/* ERRATA WRITE_REG(REG_RLPS_FUSE_CTRL_STAT, 0x00);*/
	
	/* Call fuse stabilization procedure */
	FuseStabilizationProc(fe);

	/* Program Transmit pulse */
	InitLineReceiver(fe, 
			(IS_E1_FEMEDIA(fe) ? 
			 		pmc_e1_rlps_ram_table:
			 		pmc_t1_rlps_ram_table));

	return 0;
}

/*
 ******************************************************************************
 *				sdla_pmc4354_te_config()	
 *
 * Description: Configure Sangoma A104 TE1 board
 * Arguments:	
 * Returns:	WANTRUE - TE1 configred successfully, otherwise WAN_FALSE.
 ******************************************************************************
 */
static short sdla_pmc4354_te_config(sdla_fe_t *fe, u16 adapter_type)
{
	unsigned char	value = 0x00;
	
	if (WAN_FE_FRAME(fe) == WAN_FR_UNFRAMED){
		DEBUG_EVENT("%s: ERROR: AFT-QUAD doesn't support UNFRAMED mode!\n",
			  	fe->name);
		return -EINVAL;	
	}
		
	/* Global Configuration (Reg 0xQ00) */
	value = READ_REG(REG_GLOBAL_CFG);
	if (IS_E1_FEMEDIA(fe)){
		value |= BIT_GLOBAL_TRKEN;
	}else{
		value |= BIT_GLOBAL_TRKEN;
	}
	WRITE_REG(REG_GLOBAL_CFG, value);


	/* CDRC Configuration (Reg. 0xQ10) */
	if (WAN_FE_LCODE(fe) == WAN_LCODE_AMI){
		WRITE_REG(REG_CDRC_CFG, BIT_CDRC_CFG_AMI); 
	}else{
		WRITE_REG(REG_CDRC_CFG, 0x00); 
	}

	if (IS_E1_FEMEDIA(fe)) {
		/* RX-ELST Configuration (Reg. 0xQ1C) */
		WRITE_REG(REG_RX_ELST_CFG, BIT_RX_ELST_IR | BIT_RX_ELST_OR);
		/* TX-ELST Configuration (Reg. 0xQ20) */
		WRITE_REG(REG_TX_ELST_CFG, BIT_TX_ELST_IR | BIT_RX_ELST_OR); 
	}else{
		/* RX-ELST Configuration (Reg. 0xQ1C) */
		WRITE_REG(REG_RX_ELST_CFG, 0x00); 
		/* TX-ELST Configuration (Reg. 0xQ20) */
		WRITE_REG(REG_TX_ELST_CFG, 0x00);
	}

	if (IS_E1_FEMEDIA(fe)){
		/* E1-TRAN Configuration (Reg. 0xQ80) */
		value = 0x00;
		if (WAN_FE_LCODE(fe) == WAN_LCODE_AMI){
			value |= BIT_E1_TRAN_AMI;
		}
		if (WAN_FE_FRAME(fe) == WAN_FR_CRC4){
			value |= BIT_E1_TRAN_GENCRC;
		}else if (WAN_FE_FRAME(fe) == WAN_FR_UNFRAMED){
			value |= BIT_E1_TRAN_FDIS;
		}
		if (WAN_TE1_SIG_MODE(fe) == WAN_TE1_SIG_CAS){
			value |= (BIT_E1_TRAN_SIGEN|BIT_E1_TRAN_DLEN);
		}
		WRITE_REG(REG_E1_TRAN_CFG, value);

		/* E1-FRMR Frame Alignment Options (Reg 0xQ90) */
		value = 0x00;
		if (WAN_FE_FRAME(fe) == WAN_FR_CRC4){
			value |= (BIT_E1_FRMR_CRCEN |
				  BIT_E1_FRMR_REFCRCEN);
		}
		if (WAN_TE1_SIG_MODE(fe) == WAN_TE1_SIG_CCS){
			value |= BIT_E1_FRMR_CASDIS;
		}
		WRITE_REG(REG_E1_FRMR_CFG, value);
		
	}else{
		/* T1-XBAS Configuration (Reg 0xQ54) */
		value = 0x00;
		if (WAN_FE_LCODE(fe) == WAN_LCODE_B8ZS){
			value |= BIT_T1_XBAS_B8ZS;
		}else{
			value |= BIT_T1_XBAS_ZCS0;
		}
		if (WAN_FE_FRAME(fe) == WAN_FR_ESF){
			value |= BIT_T1_XBAS_ESF;
		}
		if (IS_J1_FEMEDIA(fe)){
			value |= BIT_T1_XBAS_JPN;
		}
		WRITE_REG(REG_T1_XBAS_CFG, value);

		/* T1-FRMR Configuration (Reg. 0xQ48) */
		value = 0x00;
		if (WAN_FE_FRAME(fe) == WAN_FR_ESF){
			value = BIT_T1_FRMR_ESF | BIT_T1_FRMR_ESFFA;
		}
		if (IS_J1_FEMEDIA(fe)){
			value |= BIT_T1_FRMR_JPN;
		}
		WRITE_REG(REG_T1_FRMR_CFG, value);

		/* T1 ALMI Configuration (Reg. 0xQ60) */
		value = 0x00;
		if (WAN_FE_FRAME(fe) == WAN_FR_ESF){
			value |= BIT_T1_ALMI_CFG_ESF;
		}
		WRITE_REG(REG_T1_ALMI_CFG, value);
	}

	/* SIGX Configuration Register (Reg. 0xQ50) */
	value = 0x00;
	if (IS_E1_FEMEDIA(fe)){
		WRITE_REG(REG_SIGX_CFG, value);
	}else{
		if (WAN_FE_FRAME(fe) == WAN_FR_ESF){
			value |= BIT_SIGX_ESF;
		}
		WRITE_REG(REG_SIGX_CFG, value);
	}
	
	/* BTIF Configuration (Reg. 0xQ40) */
	value = 0x00;
	if (IS_E1_FEMEDIA(fe)){
		value |= BIT_BTIF_RATE0;
	}
#if 0
	/* If AMI is selected is only can be used for VOICE.
	 * In this case, we need to use FULL FRAME mode (no gaps). */
	if (WAN_FE_LCODE(fe) == WAN_LCODE_AMI){
		value |= BIT_BTIF_NXDS0_0;
	}
#endif
	value |= BIT_BTIF_DE;
	if (WAN_TE1_CLK(fe) == WAN_MASTER_CLK && 
	    WAN_TE1_REFCLK(fe) != WAN_TE1_REFCLK_OSC){
		/* Configure Transmit clock as input (slave) */
		value |= BIT_BTIF_CMODE;
	}
	WRITE_REG(REG_BTIF_CFG, value);

	/* BTIF_Frame Pulse Configuration (Reg. 0xQ41) */
	if (WAN_TE1_CLK(fe) == WAN_MASTER_CLK &&
	    WAN_TE1_REFCLK(fe) != WAN_TE1_REFCLK_OSC){
		/* Configure Transmit frame pulse as input (slave) */
		WRITE_REG(REG_BTIF_FR_PULSE_CFG, BIT_BTIF_FPMODE); 
	}else{
		WRITE_REG(REG_BTIF_FR_PULSE_CFG, 0x00); 
	}

	/* BRIF Configuration (Reg. 0xQ30) */
	value = 0x00;
	if (IS_E1_FEMEDIA(fe)){
		value |= BIT_BRIF_RATE0;
	}
#if 0
	/* If AMI is selected is only can be used for VOICE.
	 * In this case, we need to use FULL FRAME mode (no gaps). */
	if (WAN_FE_LCODE(fe) == WAN_LCODE_AMI){
		value |= BIT_BRIF_NXDS0_0;
	}
#endif
	value |= (BIT_BRIF_CMODE | BIT_BRIF_FE);
	WRITE_REG(REG_BRIF_CFG, value);
	
	/* BRIF Frame Pulse Configuration (Reg. 0xQ31) */
	WRITE_REG(REG_BRIF_FR_PULSE_CFG, BIT_BRIF_FPMODE); 

	/* BRIF Parity/F-bit Configuration (Reg. 0xQ32) */
	WRITE_REG(REG_BRIF_DATA_CFG, BIT_BRIF_DATA_TRI_0);

	/* Transmit Timing Options (Reg 0xQ06) */
	if (WAN_TE1_CLK(fe) == WAN_NORMAL_CLK){
		/* 1. Set TJAT FIFO output clock signal 
		 * 2. Receive recovered clock */
		WRITE_REG(REG_TX_TIMING_OPT, 
				BIT_TX_PLLREF1 |
				BIT_TX_TXELSTBYP);
	}else{
		if (WAN_TE1_CLK(fe) == WAN_MASTER_CLK &&
		    WAN_TE1_REFCLK(fe) != WAN_TE1_REFCLK_OSC){
			/* In this case, we should set only PLLREF0 bit
			** to 1 (firmware also been changed in order to
			** solve this problem). Nov 1, 2005
			** WRITE_REG(REG_TX_TIMING_OPT, 
			**		BIT_TX_PLLREF1 |
			**		BIT_TX_PLLREF0); */
			WRITE_REG(REG_TX_TIMING_OPT, BIT_TX_PLLREF0);
		}else{
			/* 1. Set TJAT FIFO output clock signal
			 * 2. CTCLK input */
			WRITE_REG(REG_TX_TIMING_OPT, 
					BIT_TX_PLLREF1 |
					BIT_TX_PLLREF0 |
					BIT_TX_TXELSTBYP);
		}
	}

	/* RLPS Configuration and Status (Reg 0xQF8) */
	WRITE_REG(REG_RLPS_CFG_STATUS, 
				BIT_RLPS_CFG_STATUS_LONGE |
				BIT_RLPS_CFG_STATUS_SQUELCHE);

	/* RLPS ALOS Detection/Clearance Thresholds (Reg 0xQF9) */
	/* NC: Aug 20 2003:
	 *     Set the correct ALSO Detection/Clearance tresholds
	 *     for T1/E1 lines, to get rid of false ALOS alarms.
	 *
	 *     Original incorrect value set was 0x00, for both T1/E1 */
	if (IS_E1_FEMEDIA(fe)){
		WRITE_REG(REG_RLPS_ALOS_DET_CLR_THR, 
				BIT_RLPS_ALOS_DET_THR_2|
				BIT_RLPS_ALOS_DET_THR_1|
				BIT_RLPS_ALOS_DET_THR_0);
	}else{
		WRITE_REG(REG_RLPS_ALOS_DET_CLR_THR, 
				BIT_RLPS_ALOS_CLR_THR_2|
				BIT_RLPS_ALOS_CLR_THR_0|
				BIT_RLPS_ALOS_DET_THR_2|
				BIT_RLPS_ALOS_DET_THR_0);
	}

	/* RLPS ALOS Detection period (Reg 0xQFA) */
	WRITE_REG(REG_RLPS_ALOS_DET_PER, BIT_RLPS_ALOS_DET_PER_0);
	
	/* RLPS ALOS Clearance period (Reg 0xQFB) */
	WRITE_REG(REG_RLPS_ALOS_CLR_PER, BIT_RLPS_ALOS_CLR_PER_0);
	
	/* RLPS Equalizatopn Indirect Address (Reg. 0xQFC) */
	/* ERRATA WRITE_REG(REG_RLPS_EQ_ADDR, 0x00); */

	/* RLPS Equalization Read/WriteB Select (Reg 0xQFD) */
	/* ERRATA WRITE_REG(REG_RLPS_EQ_RWB, BIT_RLPS_EQ_RWB); */
	
	/* RLPS Equalizer Loop Status and Control (Reg 0xQFE) */
	WRITE_REG(REG_RLPS_EQ_STATUS, 0x00);

	/* TJAT Configuration (Reg 0xQ1B) */
	WRITE_REG(REG_TJAT_CFG, BIT_TJAT_CENT);

	/* RJAT Configuration (Reg 0xQ17) */
	WRITE_REG(REG_RJAT_CFG, BIT_RJAT_CENT);

	/* Receive Options (Reg 0xQ02) */
	if (WAN_FE_FRAME(fe) == WAN_FR_UNFRAMED){
		WRITE_REG(REG_RECEIVE_OPT, BIT_RECEIVE_OPT_UNF);
	}else{
		WRITE_REG(REG_RECEIVE_OPT, 0x00);
	}

	if (IS_E1_FEMEDIA(fe)){
		WRITE_REG(REG_E1_TRAN_NATB_CODESEL,
				BIT_E1_TRAN_NATB_CODESEL_Sa4);
		WRITE_REG(REG_E1_TRAN_NATB_CODE, 0xFF);

		WRITE_REG(REG_E1_TRAN_NATB_CODESEL,
				BIT_E1_TRAN_NATB_CODESEL_Sa5);
		WRITE_REG(REG_E1_TRAN_NATB_CODE, 0xFF);

		WRITE_REG(REG_E1_TRAN_NATB_CODESEL,
				BIT_E1_TRAN_NATB_CODESEL_Sa6);
		WRITE_REG(REG_E1_TRAN_NATB_CODE, 0xFF);

		WRITE_REG(REG_E1_TRAN_NATB_CODESEL,
				BIT_E1_TRAN_NATB_CODESEL_Sa7);
		WRITE_REG(REG_E1_TRAN_NATB_CODE, 0xFF);

		WRITE_REG(REG_E1_TRAN_NATB_CODESEL,
				BIT_E1_TRAN_NATB_CODESEL_Sa8);
		WRITE_REG(REG_E1_TRAN_NATB_CODE, 0xFF);
	}

	/* XLPG Configuration #1 (Reg 0xQF4) */
	/* 13 Jan 2005 WRITE_REG(REG_XLPG_TNC, BIT_XLPG_TNC_1);	*/
	WRITE_REG(REG_XLPG_TNC, BIT_XLPG_TNC_1); /* ADEBUG */
	
	/* XLPG Configuration #2 (Reg 0xQF5) */
	/* 13 Jan 2005 WRITE_REG(REG_XLPG_TPC, BIT_XLPG_TPC_1); */
	WRITE_REG(REG_XLPG_TPC, BIT_XLPG_TPC_1); /* ADEBUG */

	/* RLPS Equalizer Voltage Reference (Reg 0xQDC) */
	if (IS_E1_FEMEDIA(fe)){
		WRITE_REG(REG_EQ_VREF, 0x3D); /* ADEBUG: original 0x34 */
	}else{
		WRITE_REG(REG_EQ_VREF, 0x2C);
	}

	/* ADEBUG: This register doesn't exist 
	* WRITE_REG(REG_RLPS_FUSE_CTRL_STAT, 0x00); */

	/* Program Transmit pulse */
	InitLineReceiver(fe, 
			(IS_E1_FEMEDIA(fe) ? 
			 		pmc4_e1_rlps_ram_table:
			 		pmc4_t1_rlps_ram_table));

	/* RLPS Equalizer Configuration (Reg 0xQFF) */
	WRITE_REG(REG_RLPS_EQ_CFG,
				BIT_RLPS_EQ_EQEN |
				BIT_RLPS_EQ_RESERVED_1 |
				BIT_RLPS_EQ_RESERVED_0);

	/* Program Transmit pulse template */
	fe->fe_param.te_param.xlpg_scale = PrgTransmitTemplate(fe);
	
	/* Call fuse stabilization procedure */
	FuseStabilizationProc(fe);

	/* XLPG Line Driver Configuration (Reg. 0xQF0) */
	if (IS_FE_TXTRISTATE(fe)){
		DEBUG_EVENT("%s: Disable Transmitter (tx tri-state mode)!\n",
					fe->name);
		WRITE_REG(REG_XLPG_LINE_CFG, BIT_XLPG_LINE_CFG_HIGHZ);
	}else{
		WRITE_REG(REG_XLPG_LINE_CFG, fe->fe_param.te_param.xlpg_scale);
	}

	return 0;
}


/*
 ******************************************************************************
 *				sdla_pmc4354_rlps_optim()	
 *
 * Description: PMC4354 COMET QUAD RLPS optimization routine.
 * 
 * Arguments:	
 * Returns:
 ******************************************************************************
 */
static int sdla_pmc4354_rlps_optim(sdla_fe_t *fe)
{
	/* RLPS Optimization routine */
	WRITE_REG_LINE(0, 0x4D7, 0x00);	/* 01 */
	WRITE_REG_LINE(0, 0x4F1, 0x00);	/* 02 */
	WRITE_REG_LINE(0, 0x5F1, 0x00);	/* 03 */
	WRITE_REG_LINE(0, 0x6F1, 0x00);	/* 04 */
	WRITE_REG_LINE(0, 0x7F1, 0x00);	/* 05 */
	WRITE_REG_LINE(0, 0x4F9, 0x00);	/* 06 */
	WRITE_REG_LINE(0, 0x5F9, 0x00);	/* 07 */
	WRITE_REG_LINE(0, 0x6F9, 0x00);	/* 08 */
	WRITE_REG_LINE(0, 0x7F9, 0x00);	/* 09 */
	WRITE_REG_LINE(0, 0x4F9, 0x04);	/* 10 */
	WRITE_REG_LINE(0, 0x4FB, 0x09);	/* 11 */
	WRITE_REG_LINE(0, 0x00B, 0x20);	/* 12 */
	WP_DELAY(1);
	WRITE_REG_LINE(0, 0x4F9, 0x00);	/* 14 */
	WRITE_REG_LINE(0, 0x00B, 0x00);	/* 15 */
	WRITE_REG_LINE(0, 0x5F9, 0x04);	/* 16 */
	WRITE_REG_LINE(0, 0x5FB, 0x09);	/* 17 */
	WRITE_REG_LINE(0, 0x00B, 0x20);	/* 18 */
	WP_DELAY(1);
	WRITE_REG_LINE(0, 0x5F9, 0x00);	/* 20 */
	WRITE_REG_LINE(0, 0x00B, 0x00);	/* 21 */
	WRITE_REG_LINE(0, 0x6F9, 0x04);	/* 22 */
	WRITE_REG_LINE(0, 0x6FB, 0x09);	/* 23 */
	WRITE_REG_LINE(0, 0x00B, 0x20);	/* 24 */
	WP_DELAY(1);
	WRITE_REG_LINE(0, 0x6F9, 0x00);	/* 26 */
	WRITE_REG_LINE(0, 0x00B, 0x00);	/* 27 */
	WRITE_REG_LINE(0, 0x7F9, 0x04);	/* 28 */
	WRITE_REG_LINE(0, 0x7FB, 0x09);	/* 29 */
	WRITE_REG_LINE(0, 0x00B, 0x20);	/* 30 */
	WP_DELAY(1);
	WRITE_REG_LINE(0, 0x7F9, 0x00);	/* 32 */
	WRITE_REG_LINE(0, 0x00B, 0x00);	/* 33 */

	return 0;
}


/*
 ******************************************************************************
 *				sdla_te_config()	
 *
 * Description: Configure Sangoma T1/E1 boards
 * Arguments:	
 * Returns:	WANTRUE - TE1 configred successfully, otherwise WAN_FALSE.
 ******************************************************************************
 */
short sdla_te_config(void* pfe, void *p_fe_iface)
{
	sdla_fe_t	*fe = (sdla_fe_t*)pfe;
	sdla_t*		card = (sdla_t*)fe->card;
	sdla_fe_iface_t	*fe_iface = (sdla_fe_iface_t*)p_fe_iface;
	u16		adapter_type;
	unsigned char	value = 0x00;

	WAN_ASSERT(fe->write_fe_reg == NULL);
	WAN_ASSERT(fe->read_fe_reg == NULL);

	/* Initial FE state */
	fe->fe_status=FE_DISCONNECTED;

	/* Revision/Chip ID (Reg. 0x0D) */
	value = READ_REG_LINE(0, REG_REVISION_CHIP_ID);
	fe->fe_chip_id = value & MASK_CHIP_ID;
	switch(fe->fe_chip_id){
	case CHIP_ID_COMET:
		fe->fe_cfg.line_no = 0;
		break;
	case CHIP_ID_COMET_QUAD:
		if (WAN_FE_LINENO(fe) < 0 || WAN_FE_LINENO(fe) > 3){
			DEBUG_EVENT("%s: TE Config: Invalid Port selected %d (Min=1 Max=4)\n",
					fe->name,
					WAN_FE_LINENO(fe)+1);
			return -EINVAL;
		}
		break;
	default:
		DEBUG_EVENT("%s: ERROR: Unsupported %s CHIP (0x%02X)\n",
				fe->name, 
				FE_MEDIA_DECODE(fe),
				(fe->fe_chip_id >> 5));
		return -EINVAL;
	}

	card->hw_iface.getcfg(card->hw, SDLA_ADAPTERTYPE, &adapter_type);
	DEBUG_EVENT("%s: Configuring PMC %s %s Front End (Port %d)!\n",
        		     	fe->name, 
				DECODE_CHIPID(fe->fe_chip_id),
				FE_MEDIA_DECODE(fe), 
				WAN_FE_LINENO(fe)+1);

       	DEBUG_TE1("%s: %s Line decoding %s\n", 
			fe->name, 
			FE_MEDIA_DECODE(fe), 
			FE_LCODE_DECODE(fe));
       	DEBUG_TE1("%s: %s Frame type %s\n", 
			fe->name, 
			FE_MEDIA_DECODE(fe), 
			FE_FRAME_DECODE(fe));
	if (IS_T1_FEMEDIA(fe)){
		DEBUG_TE1("%s: %s LBO %s\n", 
				fe->name,
				FE_MEDIA_DECODE(fe), 
				TE_LBO_DECODE(fe));
	}
	if (WAN_TE1_CLK(fe) == WAN_MASTER_CLK){
		if (WAN_TE1_REFCLK(fe) != WAN_TE1_REFCLK_OSC){
       			DEBUG_TE1("%s: %s Clock mode Master (refclk from Port %d)\n", 
					fe->name, 
					FE_MEDIA_DECODE(fe), 
					WAN_TE1_REFCLK(fe)); 
		}else{
       			DEBUG_TE1("%s: %s Clock mode Master (Osciliator)\n", 
					fe->name, 
					FE_MEDIA_DECODE(fe));
		}
	}else{
       		DEBUG_TE1("%s: %s Clock mode Normal\n", 
				fe->name,
				FE_MEDIA_DECODE(fe)); 
	}
	DEBUG_TE1("%s: %s robbit-signalling channels %lX\n",
			fe->name,
			FE_MEDIA_DECODE(fe), 
			fe->fe_cfg.cfg.te_cfg.te_rbs_ch);
	memset(&fe->fe_param.te_param, 0, sizeof(sdla_te_param_t));

	fe->fe_param.te_param.max_channels = 
		(IS_E1_FEMEDIA(fe)) ? NUM_OF_E1_TIMESLOTS: NUM_OF_T1_CHANNELS;
	switch(fe->fe_chip_id){
	case CHIP_ID_COMET:
		sdla_pmc4351_te_config(fe, adapter_type);
		break;

	case CHIP_ID_COMET_QUAD:
		sdla_pmc4354_te_config(fe, adapter_type);
		sdla_pmc4354_rlps_optim(fe);
		break;

	default:
		DEBUG_EVENT("%s: ERROR: Unsupported T1/E1 CHIP (0x%02X)\n",
				fe->name, (fe->fe_chip_id >> 5));
		return -EINVAL;
	}

	ClearRPSCReg(fe);
	ClearTPSCReg(fe);
	DisableAllChannels(fe);

	if (WAN_TE1_ACTIVE_CH(fe) == ENABLE_ALL_CHANNELS){
		DEBUG_EVENT("%s: All channels enabled\n", fe->name);
		EnableAllChannels(fe);
	}else{
		int channel_range = (IS_T1_FEMEDIA(fe)) ? 
				NUM_OF_T1_CHANNELS : NUM_OF_E1_TIMESLOTS;

		int i = 0;

 		for(i = 1; i <= channel_range; i++){
			if (WAN_TE1_ACTIVE_CH(fe) & (1 << (i - 1))){
				DEBUG_EVENT("%s: Enable channel %d\n", 
						fe->name, i);
				EnableTxChannel(fe, i);
				EnableRxChannel(fe, i);
			}
		}
	}

	/* Initialize and start T1/E1 timer */
	wan_set_bit(TE_TIMER_KILL,(void*)&fe->fe_param.te_param.critical);

	wan_init_timer(
		&fe->fe_param.te_param.timer, 
		sdla_te_timer,
	       	(wan_timer_arg_t)fe);
	
	/* Inialize Front-End interface functions */
	fe_iface->isr			= &sdla_te_intr;
	fe_iface->check_isr		= &sdla_te_check_intr;
	fe_iface->polling		= &sdla_te_polling;
	fe_iface->process_udp		= &sdla_te_udp;
	fe_iface->print_fe_alarm	= &sdla_te_print_alarms;
	fe_iface->print_fe_act_channels	= &sdla_te_print_channels;
	fe_iface->set_fe_alarm		= &sdla_te_set_alarms;
	fe_iface->set_fe_sigctrl	= &sdla_te_sigctrl;
	fe_iface->read_alarm		= &sdla_te_read_alarms;
	fe_iface->read_pmon		= &sdla_te_pmon;
	fe_iface->flush_pmon		= &sdla_te_flush_pmon;
	fe_iface->get_fe_status		= &sdla_te_get_fe_status;
	fe_iface->get_fe_media		= &sdla_te_get_fe_media;
	fe_iface->set_fe_lbmode		= &sdla_te_set_lbmode;
	fe_iface->get_fe_media_string	= &sdla_te_get_fe_media_string;
	fe_iface->update_alarm_info	= &sdla_te_update_alarm_info;
	fe_iface->update_pmon_info	= &sdla_te_update_pmon_info;
	fe_iface->led_ctrl		= &sdla_te_led_ctrl;
	fe_iface->check_rbsbits		= &sdla_te_check_rbsbits;
	fe_iface->read_rbsbits		= &sdla_te_read_rbsbits;
	fe_iface->set_rbsbits		= &sdla_te_set_rbsbits;
	fe_iface->report_rbsbits	= &sdla_te_rbs_report;
#if !defined(CONFIG_PRODUCT_WANPIPE_GENERIC)
	fe_iface->get_snmp_data		= &sdla_te_get_snmp_data;
#endif
	fe_iface->post_config		= &sdla_te_post_config;

#if defined(__WINDOWS__)
	/* (Windows only) Can't execute poll timer from interrupt level */
	fe_iface->enable_timer		= &sdla_te_enable_timer;
#endif

	/* Set initial FE status to uninitialized value and let the next function
	 * set correct value */
	fe->fe_status = 0;
	/* Read initial alarm status and then enable T1/E1 alarm interrupts */
	/* sdla_te_alarm(fe, 0); */

	wan_clear_bit(TE_TIMER_KILL,(void*)&fe->fe_param.te_param.critical);
	wan_clear_bit(TE_TIMER_RUNNING,(void*)&fe->fe_param.te_param.critical);
	sdla_te_enable_timer(fe, TE_LINKDOWN_TIMER, INTR_TE1_TIMER);
	
	wan_set_bit(TE_CONFIGURED,(void*)&fe->fe_param.te_param.critical);

	return 0;
}

/*
 ******************************************************************************
 *			sdla_te_post_config()	
 *
 * Description: T1/E1 post configuration.
 * Arguments:
 * Returns:
 ******************************************************************************
 */
static int sdla_te_post_config(sdla_fe_t* fe)
{
	unsigned char	value;

	if (IS_E1_FEMEDIA(fe)){
		if (WAN_TE1_SIG_MODE(fe) == WAN_TE1_SIG_CAS){
			DEBUG_EVENT("%s: Enable E1 CAS signalling mode!\n",
					fe->name);
			/* E1-TRAN Configuration (Reg. 0x80) */
			value = READ_REG(REG_E1_TRAN_CFG);
			value |= (BIT_E1_TRAN_SIGEN|BIT_E1_TRAN_DLEN);
			WRITE_REG(REG_E1_TRAN_CFG, value);
		
			/* E1-FRMR Frame Alignment Options (Reg 0x90) */
			value = READ_REG(REG_E1_FRMR_CFG);
			value &= ~BIT_E1_FRMR_CASDIS;
			WRITE_REG(REG_E1_FRMR_CFG, value);

		}else if (WAN_TE1_SIG_MODE(fe) == WAN_TE1_SIG_CCS){
			DEBUG_EVENT("%s: Disable E1 CAS signalling mode!\n",
					fe->name);
			/* E1-TRAN Configuration (Reg. 0x80) */
			value = READ_REG(REG_E1_TRAN_CFG);
			value &= ~(BIT_E1_TRAN_SIGEN|BIT_E1_TRAN_DLEN);
			WRITE_REG(REG_E1_TRAN_CFG, value);
		
			/* E1-FRMR Frame Alignment Options (Reg 0x90) */
			value = READ_REG(REG_E1_FRMR_CFG);
			value |= BIT_E1_FRMR_CASDIS;
			WRITE_REG(REG_E1_FRMR_CFG, value);
		}
	}
	return 0;
}

/*
 ******************************************************************************
 *			sdla_te_set_intr()	
 *
 * Description: Enable T1/E1 interrupts.
 * Arguments:
 * Returns:
 ******************************************************************************
 */
static void sdla_te_set_intr(sdla_fe_t* fe)
{

	DEBUG_TEST("%s: Enabling TE1 Interrupts \n",fe->name);

	/* Enable LOS interrupt */
	WRITE_REG(REG_CDRC_INT_EN, BIT_CDRC_INT_EN_LOSE);

#if defined(FE_ALOS_ENABLE)
	/* Enable ALOS interrupt */
	WRITE_REG(REG_RLPS_CFG_STATUS, 
		READ_REG(REG_RLPS_CFG_STATUS) | BIT_RLPS_CFG_STATUS_ALOSE);
#endif
	if (IS_T1_FEMEDIA(fe)){
		/* Enable RBOC interrupt */
		WRITE_REG(REG_T1_RBOC_ENABLE, 
				BIT_T1_RBOC_ENABLE_IDLE | 
				BIT_T1_RBOC_ENABLE_BOCE);
		/* Enable interrupt on RED, AIS, YEL alarms */
		WRITE_REG(REG_T1_ALMI_INT_EN, 
				BIT_T1_ALMI_INT_EN_FASTD | 
				BIT_T1_ALMI_INT_EN_REDE | 
				BIT_T1_ALMI_INT_EN_AISE |
				BIT_T1_ALMI_INT_EN_YELE);
		/* Enable interrupt on OOF alarm */
		/* WRITE_REG(REG_T1_FRMR_INT_EN, BIT_T1_FRMR_INT_EN_INFRE); */
	}else{
		/* Enable interrupt on RED, AIS alarms */
		WRITE_REG(REG_E1_FRMR_M_A_INT_EN, 
				BIT_E1_FRMR_M_A_INT_EN_REDE | 
				BIT_E1_FRMR_M_A_INT_EN_AISE);
		/* Enable OOF Interrupt */
		/*WRITE_REG(REG_E1_FRMR_FRM_STAT_INT_EN,BIT_E1_FRMR_FRM_STAT_INT_EN_OOFE);*/
	}

	return;
}

/*
 ******************************************************************************
 *			sdla_te_clear_intr()	
 *
 * Description: Disable T1/E1 interrupts.
 * Arguments:
 * Returns:
 ******************************************************************************
 */
static void sdla_te_clear_intr(sdla_fe_t* fe)
{

	DEBUG_TEST("%s: Clearing TE1 Interrupts\n",fe->name);

	/* Enable LOS interrupt */
	WRITE_REG(REG_CDRC_INT_EN, 
			READ_REG(REG_CDRC_INT_EN) & ~BIT_CDRC_INT_EN_LOSE);
	/* Enable ALOS interrupt */
	WRITE_REG(REG_RLPS_CFG_STATUS, 
		READ_REG(REG_RLPS_CFG_STATUS) & ~BIT_RLPS_CFG_STATUS_ALOSE); 
	if (IS_T1_FEMEDIA(fe)){
		/* Enable RBOC interrupt */
		WRITE_REG(REG_T1_RBOC_ENABLE, 
			READ_REG(REG_T1_RBOC_ENABLE) & 
				~(BIT_T1_RBOC_ENABLE_IDLE | 
				BIT_T1_RBOC_ENABLE_BOCE));
		/* Enable interrupt on RED, AIS, YEL alarms */
		WRITE_REG(REG_T1_ALMI_INT_EN, 
			READ_REG(REG_T1_ALMI_INT_EN) & 
				~(BIT_T1_ALMI_INT_EN_REDE | 
				BIT_T1_ALMI_INT_EN_AISE |
				BIT_T1_ALMI_INT_EN_YELE));
		/* Enable interrupt on OOF alarm */
		/* WRITE_REG(REG_T1_FRMR_INT_EN, 
		 * 			BIT_T1_FRMR_INT_EN_INFRE);*/
	}else{
		/* Enable interrupt on RED, AIS alarms */
		WRITE_REG(REG_E1_FRMR_M_A_INT_EN, 
			READ_REG(REG_E1_FRMR_M_A_INT_EN) & 
				~(BIT_E1_FRMR_M_A_INT_EN_REDE | 
				BIT_E1_FRMR_M_A_INT_EN_AISE));
		/* Enable OOF Interrupt */
		/* WRITE_REG(REG_E1_FRMR_FRM_STAT_INT_EN,
		 * 			BIT_E1_FRMR_FRM_STAT_INT_EN_OOFE);*/
	}

	return;
}


static int sdla_te_sigctrl(sdla_fe_t *fe, int sig_mode)
{
	int	err = 0;
	
	if (sig_mode == WAN_TE_SIG_INTR){
		/* Enable signaling interrupt in case if we are not
		** using polling routines for reading signaling bits
		** from PMC chip. te_signaling_config function pointer
		** is used for S514x T1/E1 card only. All new AFT
		** cards use Signaling interrupt for this
		** (Aug 5, 2004) */
		if (fe->fe_status == FE_CONNECTED){
			/* Enable SIGE and COSS */
			DEBUG_TEST("%s: Enable SIGX interrupt\n",
			 			 fe->name); 
			WRITE_REG(REG_SIGX_CFG, 
				READ_REG(REG_SIGX_CFG) | BIT_SIGX_SIGE);
			WRITE_REG(REG_SIGX_CFG, 
				READ_REG(REG_SIGX_CFG) | BIT_SIGX_COSS);
		}else{
			WRITE_REG(REG_SIGX_CFG, 
				READ_REG(REG_SIGX_CFG) &
					~(BIT_SIGX_SIGE|BIT_SIGX_COSS));
		}
	}
#if 0
	/* Do not support RBS for the old cards */
	if (card->wandev.te_signaling_config){
		err=card->wandev.te_signaling_config(card,timeslot_map);
		if (err) return err;
		err=card->wandev.te_read_signaling_config(card);
		if (err) return err;
	}
#endif
	return err;
}

/*
 ******************************************************************************
 *				sdla_te_global_unconfig()	
 *
 * Description: Global configuration for Sangoma TE1 PMC board.
 * 		Note: 	These register should be program only once for AFT-QUAD
 * 			cards.
 * Arguments:	
 * Returns:	WANTRUE - TE1 configred successfully, otherwise WAN_FALSE.
 ******************************************************************************
 */
int sdla_te_global_unconfig(void* pfe)
{
	sdla_fe_t	*fe = (sdla_fe_t*)pfe;

	DEBUG_EVENT("%s: T1/E1 Global unconfiguration!\n", fe->name);
	/* 1. Initiate software reset of the COMET */
	/* Set RESET=1 to place COMET into RESET */
	WRITE_REG_LINE(0, REG_RESET, BIT_RESET);

	/* Set RESET=0, disable software reset. COMET in default mode. */
	WRITE_REG_LINE(0, REG_RESET, 0x0/*~BIT_RESET*/);  
	return 0;
}

/*
 ******************************************************************************
 *			sdla_te_unconfig()	
 *
 * Description: T1/E1 unconfig.
 * Arguments:
 * Returns:
 ******************************************************************************
 */
void sdla_te_unconfig(void* pfe)
{
	sdla_fe_t	*fe = (sdla_fe_t*)pfe;

	if (!wan_test_bit(TE_CONFIGURED,(void*)&fe->fe_param.te_param.critical)){
		return;
	}

	/* Clear and Kill TE timer poll command */
	wan_clear_bit(TE_CONFIGURED,(void*)&fe->fe_param.te_param.critical);
	wan_set_bit(TE_TIMER_KILL,(void*)&fe->fe_param.te_param.critical);

	/* FIXME: Alex to disable interrupts here */
	sdla_te_clear_intr(fe);

	wan_del_timer(&fe->fe_param.te_param.timer);

	fe->fe_param.te_param.timer_cmd = 0x00;
	wan_clear_bit(TE_TIMER_RUNNING,(void*)&fe->fe_param.te_param.critical);

	ClearTemplate(fe);

	return;
}

/*****************************************************************************
**			sdla_te_master_clock()	
**			sdla_te_normal_clock()	
**
** Description: This function is called only if frond end clock is congirued
** 		originally to NORMAL (otherwise its never called). According
**		specification, when the CE recieves AIS alarm, the CE shall 
**		transmit from it's internal clock. When AIS alarm is cleared 
**		transmit should change clock type to normal.
** Arguments:
** Returns:
******************************************************************************/
static int sdla_te_master_clock(sdla_fe_t *fe)
{
	switch(fe->fe_chip_id){
	case CHIP_ID_COMET:
		WRITE_REG(REG_TX_TIMING_OPT,
					BIT_TX_PLLREF1 |
					BIT_TX_PLLREF0 |
					BIT_TX_TXELSTBYP);
		break;
	
	case CHIP_ID_COMET_QUAD:
		WRITE_REG(REG_TX_TIMING_OPT, 
					BIT_TX_PLLREF1 |
					BIT_TX_PLLREF0);
		break;
	}
	return 0;
}

static int sdla_te_normal_clock(sdla_fe_t *fe)
{
	WRITE_REG(REG_TX_TIMING_OPT,
			BIT_TX_PLLREF1 |
			BIT_TX_TXELSTBYP);
	return 0;
}

/*
 ******************************************************************************
 *			sdla_te_set_status()	
 *
 * Description: Set T1/E1 status. Enable OOF and LCV interrupt (if status 
 * 		changed to disconnected.
 * Arguments:
 * Returns:
 ******************************************************************************
 */
static void sdla_te_set_status(sdla_fe_t* fe, unsigned long alarms)
{
	sdla_t		*card = (sdla_t*)fe->card;
	unsigned char	curr_fe_status = fe->fe_status;

	if (IS_T1_FEMEDIA(fe)){
		if (IS_T1_ALARM(alarms)){
			if (fe->fe_status != FE_DISCONNECTED){
				sdla_te_set_alarms(fe, WAN_TE_BIT_YEL_ALARM);
				fe->fe_status = FE_DISCONNECTED;
			}
		}else{
			sdla_te_clear_alarms(fe, WAN_TE_BIT_YEL_ALARM);
			if (!(fe->fe_alarm & WAN_TE_BIT_YEL_ALARM)){
				if (fe->fe_status != FE_CONNECTED){
					fe->fe_status = FE_CONNECTED;
				}
			}else{
				fe->fe_status = FE_DISCONNECTED;
			}
		}
	}else{
		if (IS_E1_ALARM(alarms)){
			if (fe->fe_status != FE_DISCONNECTED){
				fe->fe_status = FE_DISCONNECTED;
			}
		}else{
			if (fe->fe_status != FE_CONNECTED){
				fe->fe_status = FE_CONNECTED;
			}
		}
	}
	if (curr_fe_status != fe->fe_status){
		if (fe->fe_status == FE_CONNECTED){
			DEBUG_EVENT("%s: %s connected!\n", 
					fe->name,
					FE_MEDIA_DECODE(fe));
		}else{
			DEBUG_EVENT("%s: %s disconnected!\n", 
					fe->name,
					FE_MEDIA_DECODE(fe));
		}

	}

#if 0
	if (IS_T1_FEMEDIA(fe)){
		if (IS_T1_ALARM(alarms)){
			if (fe->fe_status != FE_DISCONNECTED){
				DEBUG_EVENT("%s: T1 disconnected!\n", 
							fe->name);
				fe->fe_status = FE_DISCONNECTED;
			}
		}else{
			if (fe->fe_status != FE_CONNECTED){
				DEBUG_EVENT("%s: T1 connected!\n", 
							fe->name);
				fe->fe_status = FE_CONNECTED;
			}
		}
	}else{
		if (IS_E1_ALARM(alarms)){
			if (fe->fe_status != FE_DISCONNECTED){
				DEBUG_EVENT("%s: E1 disconnected!\n", 
							fe->name);
				fe->fe_status = FE_DISCONNECTED;
			}
		}else{
			if (fe->fe_status != FE_CONNECTED){
				DEBUG_EVENT("%s: E1 connected!\n", 
							fe->name);
				fe->fe_status = FE_CONNECTED;
			}
		}
	}
#endif
	if (card->wandev.te_report_alarms){
		card->wandev.te_report_alarms(card, alarms);
	}

#if 0
	if (curr_fe_status != fe->fe_status && enable_poll){
		if (fe->fe_status == FE_DISCONNECTED){
			/* Start T1/E1 timer (5 sec) */
			sdla_te_enable_timer(fe, TE_LINKDOWN_TIMER, POLLING_TE1_TIMER * 5);
		}
#if 0
		ADEBUG!!!!
		if (fe->fe_status == FE_CONNECTED){
			sdla_te_set_intr(fe);
		}else{
			sdla_te_clear_intr(fe);
		}
#endif
	}
#endif
	
#if 0
	if (fe->fe_status == FE_CONNECTED){
		WRITE_REG(REG_CDRC_INT_EN, 
			(READ_REG(REG_CDRC_INT_EN) | BIT_CDRC_INT_EN_LOSE));
	}else{
		WRITE_REG(REG_CDRC_INT_EN, 
			(READ_REG(REG_CDRC_INT_EN) & ~BIT_CDRC_INT_EN_LOSE));
	}
#endif

	return;
}

/*
 ******************************************************************************
 *			sdla_te_print_channels()	
 *
 * Description: Print T1/E1 active channels.
 * Arguments:
 * Returns:
 ******************************************************************************
 */
static char* sdla_te_print_channels(sdla_fe_t* fe)
{
	int channel_range = IS_T1_FEMEDIA(fe) ? 
				NUM_OF_T1_CHANNELS : NUM_OF_E1_TIMESLOTS;
	static char channels[100];
	int i = 0;

	channels[0] = '\0';
	if (WAN_TE1_ACTIVE_CH(fe) == ENABLE_ALL_CHANNELS){
		memcpy(channels, "All channels", strlen("All channels"));	
	}else{
		int start_channel = 0, first = 1;

 		for(i = 1; i <= channel_range; i++){
			if (WAN_TE1_ACTIVE_CH(fe) & (1 << (i - 1))){
				if (!start_channel){
					start_channel = i;
				}
			}else{
				if (start_channel){
					if (start_channel + 1 == i){
						if (!first){
							sprintf(&channels[strlen(channels)], ",%d", 
									start_channel);
						}else{
							sprintf(&channels[strlen(channels)], "%d",
									start_channel);
						}
					}else{
						if (!first){
							sprintf(&channels[strlen(channels)], ",%d-%d", 
									start_channel, i-1);
						}else{
							sprintf(&channels[strlen(channels)], "%d-%d",
									start_channel, i-1);
						}
					}
					first = 0;
					start_channel = 0;
				}
			}
		}
	}

	return channels;
}

/*
 ******************************************************************************
 *			sdla_channels()	
 *
 * Description: Enable/Disable T1/E1 channels.
 * Arguments:
 * Returns:
 ******************************************************************************
 */
#if 0
static void sdla_channels(sdla_fe_t* fe, unsigned long active_ch)
{
	sdla_fe_cfg_t* fe_cfg = &fe->fe_cfg;
	int channel_range = 
			IS_T1_FEMEDIA(fe) ? 
				NUM_OF_T1_CHANNELS : NUM_OF_E1_TIMESLOTS;
	int i = 0;

	for(i = 1; i <= channel_range; i++){
		if ((active_ch & (1 << (i - 1))) != 
		    (WAN_TE1_ACTIVE_CH(fe) & (1 << (i - 1)))){
			if (active_ch & (1 << (i - 1))) {
				/* Enable channel `i` */	
			 	EnableTxChannel(fe, i);	
			 	EnableRxChannel(fe, i);	
			}else{ 
				/* Disable channel `i` */	
				DisableTxChannel(fe, i);
				DisableRxChannel(fe, i);
			} 
		}
	}
	return;
}
#endif
/*
 ******************************************************************************
 *						ReadAlarmStatus()	
 *
 * Description: Read Alram Status for T1/E1 modes.
 * Arguments:
 * Returns:		bit 0 - ALOS	(E1/T1)
 *			bit 1 - LOS	(E1/T1)
 *			bit 2 - ALTLOS	(E1/T1)
 *			bit 3 - OOF	(E1/T1)
 *			bit 4 - RED	(E1/T1)
 *			bit 5 - AIS	(E1/T1)
 *			bit 6 - OOSMF	(E1)
 *			bit 7 - OOCMF	(E1)
 *			bit 8 - OOOF	(E1)
 *			bit 9 - RAI	(E1)
 *			bit A - YEL	(T1)
 ******************************************************************************
 */
static unsigned long sdla_te_read_alarms(sdla_fe_t *fe, int action)
{
	WAN_ASSERT(fe->write_fe_reg == NULL);
	WAN_ASSERT(fe->read_fe_reg == NULL);

	if (IS_FE_ALARM_READ(action)){

		unsigned long	status = 0x00;
		unsigned char	value = 0x00;

		/* Check common alarm for E1 and T1 configuration
		 * 1. ALOS alarm 
		 * Reg 0xFA 
		 * Reg 0xF8 (ALOSI = 1)
		 */
		if (READ_REG(REG_RLPS_ALOS_DET_PER) &&
		    (READ_REG(REG_RLPS_CFG_STATUS) & BIT_RLPS_CFG_STATUS_ALOSV)){
			status |= WAN_TE_BIT_ALOS_ALARM;
		}

		/* 2. LOS alarm 
		 * Reg 0x12
		 */
		if (READ_REG(REG_CDRC_INT_STATUS) & BIT_CDRC_INT_STATUS_LOSV){
			status |= WAN_TE_BIT_LOS_ALARM;
		}

		/* 3. ALTLOS alarm ??????????????????
		 * Reg 0x13
		 */
		if (READ_REG(REG_ALTLOS_STATUS) & BIT_ALTLOS_STATUS_ALTLOS){
			status |= WAN_TE_BIT_ALTLOS_ALARM;
		}

		/* Check specific E1 and T1 alarms */
		if (IS_E1_FEMEDIA(fe)){
			/* 4. OOF alarm */
			if (READ_REG(REG_E1_FRMR_FR_STATUS) & BIT_E1_FRMR_FR_STATUS_OOFV){
				status |= WAN_TE_BIT_OOF_ALARM;
			}
			/* 5. OOSMF alarm */
			if (READ_REG(REG_E1_FRMR_FR_STATUS) & BIT_E1_FRMR_FR_STATUS_OOSMFV){
				status |= WAN_TE_BIT_OOSMF_ALARM;
			}
			/* 6. OOCMF alarm */
			if (READ_REG(REG_E1_FRMR_FR_STATUS) & BIT_E1_FRMR_FR_STATUS_OOCMFV){
				status |= WAN_TE_BIT_OOCMF_ALARM;
			}
			/* 7. OOOF alarm */
			if (READ_REG(REG_E1_FRMR_FR_STATUS) & BIT_E1_FRMR_FR_STATUS_OOOFV){
				status |= WAN_TE_BIT_OOOF_ALARM;
			}
			/* 8. RAI alarm */
			if (READ_REG(REG_E1_FRMR_MAINT_STATUS) & BIT_E1_FRMR_MAINT_STATUS_RAIV){
				status |= WAN_TE_BIT_RAI_ALARM;
			}
			/* 9. RED alarm
			 * Reg 0x97 (REDD)
			 */
			if (READ_REG(REG_E1_FRMR_MAINT_STATUS) & BIT_E1_FRMR_MAINT_STATUS_RED){
				status |= WAN_TE_BIT_RED_ALARM;
			}
			/* 10. AIS alarm
			 * Reg 0x91 (AISC)
			 * Reg 0x97 (AIS)
			 */
			if ((READ_REG(REG_E1_FRMR_MAINT_OPT) & BIT_E1_FRMR_MAINT_OPT_AISC) &&
				(READ_REG(REG_E1_FRMR_MAINT_STATUS) & BIT_E1_FRMR_MAINT_STATUS_AIS)){
				status |= WAN_TE_BIT_AIS_ALARM;
			}
		} else {
			/* 4. OOF alarm
			 * Reg 0x4A (INFR=0 T1 mode)
			 */
			if (!(READ_REG(REG_T1_FRMR_INT_STATUS) & BIT_T1_FRMR_INT_STATUS_INFR)){
				status |= WAN_TE_BIT_OOF_ALARM;
			}
			value = READ_REG(REG_T1_ALMI_DET_STATUS);

			/* 5. AIS alarm
			 * Reg 0x62 (AIS)
			 * Reg 0x63 (AISD)
			 */
			if (value & BIT_T1_ALMI_DET_STATUS_AISD){
				status |= WAN_TE_BIT_AIS_ALARM;
			}
			/* 6. RED alarm
			 * Reg 0x63 (REDD)	
			 */
			if (value & BIT_T1_ALMI_DET_STATUS_REDD){
				status |= WAN_TE_BIT_RED_ALARM;
			}
			/* 7. YEL alarm
			 * Reg 0x62 (YEL)
			 * Reg 0x63 (YELD)
			 */
			if (value & BIT_T1_ALMI_DET_STATUS_YELD){
				status |= WAN_TE_BIT_YEL_ALARM;
			}
		}
		fe->fe_alarm = status;
	}
	if (IS_FE_ALARM_PRINT(action)){
		sdla_te_print_alarms(fe, fe->fe_alarm);
	}
	return fe->fe_alarm;
}

/*
*******************************************************************************
**				sdla_te_alaram_print()	
**
** Description: 
** Arguments:
** Returns:
*/
static int sdla_te_print_alarms(sdla_fe_t* fe, unsigned long alarms)
{
	if (!alarms){
		alarms = fe->fe_alarm; 
	}

	if (!alarms){
		DEBUG_EVENT("%s: %s Alarms status: No alarms detected!\n",
				fe->name,
				FE_MEDIA_DECODE(fe));
		return 0;
	}
	DEBUG_EVENT("%s: %s Alarms status:\n",
			fe->name,
			FE_MEDIA_DECODE(fe));

	if (alarms & WAN_TE_BIT_ALOS_ALARM){
		DEBUG_EVENT("%s:     ALOS alarm is ON\n", fe->name);
	}
	if (alarms & WAN_TE_BIT_LOS_ALARM){
		DEBUG_EVENT("%s:     LOS alarm is ON\n", fe->name);
	}
	if (alarms & WAN_TE_BIT_ALTLOS_ALARM){
		DEBUG_EVENT("%s:    ATLLOS alarm is ON\n", fe->name);
	}
	if (alarms & WAN_TE_BIT_OOF_ALARM){
		DEBUG_EVENT("%s:    OOF alarm is ON\n", fe->name);
	}
	if (alarms & WAN_TE_BIT_RAI_ALARM){
		DEBUG_EVENT("%s:    RAI alarm is ON\n", fe->name);
	}
	if (alarms & WAN_TE_BIT_RED_ALARM){
		DEBUG_EVENT("%s:    RED alarm is ON\n", fe->name);
	}
	if (alarms & WAN_TE_BIT_AIS_ALARM){
		DEBUG_EVENT("%s:    AIS alarm is ON\n", fe->name);
	}
	if (alarms & WAN_TE_BIT_YEL_ALARM){
		DEBUG_EVENT("%s:    YEL alarm is ON\n", fe->name);
	}
	return 0;
}

/*
*******************************************************************************
**				sdla_te_set_alarms()	
**
** Description: 
** Arguments:
** Returns:
*/
static int sdla_te_set_alarms(sdla_fe_t* fe, unsigned long alarms)
{
	unsigned char	value = 0x00;
	
	if (alarms & WAN_TE_BIT_YEL_ALARM){
		if (IS_T1_FEMEDIA(fe)){
			value = READ_REG(REG_T1_XBAS_ALARM_TX);
			if (!(value & BIT_T1_XBAS_ALARM_TX_XYEL)){
				value |= BIT_T1_XBAS_ALARM_TX_XYEL;
				DEBUG_EVENT("%s: Setting YELLOW alarm!\n",
							fe->name);
				WRITE_REG(REG_T1_XBAS_ALARM_TX, value); 
			}
		}
	}
	return 0;
}

/*
*******************************************************************************
**				sdla_te_clear_alarms()	
**
** Description: 
** Arguments:
** Returns:
*/
static int sdla_te_clear_alarms(sdla_fe_t* fe, unsigned long alarms)
{
	unsigned char	value = 0x00;

	if (alarms & WAN_TE_BIT_YEL_ALARM){
		if (IS_T1_FEMEDIA(fe)){
			value = READ_REG(REG_T1_XBAS_ALARM_TX);
			if (value & BIT_T1_XBAS_ALARM_TX_XYEL){
				value &= ~BIT_T1_XBAS_ALARM_TX_XYEL;
				DEBUG_EVENT("%s: Clearing YELLOW alarm!\n",
						fe->name);
				WRITE_REG(REG_T1_XBAS_ALARM_TX, value); 
			}
		}
	}
	return 0;
}

/*
 ******************************************************************************
 *				sdla_te_pmon()	
 *
 * Description: Read PMC performance monitoring counters
 * Arguments:
 * Returns:
 ******************************************************************************
 */
#define PMON_DEF_NUM	0x1FFF
static int sdla_te_pmon(sdla_fe_t *fe)
{
	sdla_te_pmon_t	*pmon = &fe->fe_pmon.u.te_pmon;
	u16		num;

	WAN_ASSERT(fe->write_fe_reg == NULL);
	WAN_ASSERT(fe->read_fe_reg == NULL);
	/* Update PMON counters */
	WRITE_REG(REG_PMON_BIT_ERROR, 0x00);
	/* Framing bit for E1/T1 */
	pmon->frm_bit_error += READ_REG(REG_PMON_BIT_ERROR) & BITS_PMON_BIT_ERROR;

	/* OOF Error for T1 or Far End Block Error for E1 */
	pmon->oof_errors += 
		((READ_REG(REG_PMON_OOF_FEB_MSB_ERROR) & BITS_PMON_OOF_FEB_MSB_ERROR) << 8) | 
		READ_REG(REG_PMON_OOF_FEB_LSB_ERROR);

	/* Bit Error for T1 or CRC Error for E1 */
	pmon->bit_errors += 
		((READ_REG(REG_PMON_BIT_CRC_MSB_ERROR) & BITS_PMON_BIT_CRC_MSB_ERROR) << 8) | 
		READ_REG(REG_PMON_BIT_CRC_LSB_ERROR);

	/* LCV Error for E1/T1 */
	num = ((READ_REG(REG_PMON_LCV_MSB_COUNT) & BITS_PMON_LCV_MSB_COUNT) << 8) | 
				READ_REG(REG_PMON_LCV_LSB_COUNT);
	if (num != PMON_DEF_NUM){
		pmon->lcv += num;
	}
	return 0;
}

/*
 ******************************************************************************
 *				sdla_fe_flush_pmon()	
 *
 * Description: Flush PMC performance monitoring counters
 * Arguments:
 * Returns:
 ******************************************************************************
 */
static int sdla_te_flush_pmon(sdla_fe_t *fe)
{
	fe->fe_pmon.u.te_pmon.pmon1 = 0;
	fe->fe_pmon.u.te_pmon.pmon2 = 0;
	fe->fe_pmon.u.te_pmon.pmon3 = 0;
	fe->fe_pmon.u.te_pmon.pmon4 = 0;
	return 0;
}

/*
 ******************************************************************************
 *				SetLoopBackChannel()	
 *
 * Description:
 * Arguments:
 * Returns:
 ******************************************************************************
 */
static int SetLoopBackChannel(sdla_fe_t* fe, int channel, unsigned char mode)
{
	/* Set IND bit to 1 in TPSC to enable indirect access to TPSC register */
	WRITE_REG(REG_TPSC_CFG, BIT_TPSC_IND); 

	/* Set LOOP to 1 for an IDLE code byte (the transmit data is overwritten 
	 * with the corresponding channel data from the receive line.
	 */
	if (mode == LINELB_ACTIVATE_CODE){
		WRITE_TPSC_REG(REG_TPSC_DATA_CTRL_BYTE, channel, 
			((READ_TPSC_REG(REG_TPSC_DATA_CTRL_BYTE, channel) & MASK_TPSC_DATA_CTRL_BYTE) | 
			 BIT_TPSC_DATA_CTRL_BYTE_LOOP));
	}else{
		WRITE_TPSC_REG(REG_TPSC_DATA_CTRL_BYTE, channel, 
			((READ_TPSC_REG(REG_TPSC_DATA_CTRL_BYTE, channel) & MASK_TPSC_DATA_CTRL_BYTE) &
			 ~BIT_TPSC_DATA_CTRL_BYTE_LOOP));
	}

	/* Set PCCE bit to 1 in TPSC to enable modifing the TPSC register */
	WRITE_REG(REG_TPSC_CFG, 
		((READ_REG(REG_TPSC_CFG) & MASK_TPSC_CFG) | BIT_TPSC_PCCE));

	return WAN_TRUE;
}

/*
 ******************************************************************************
 *				sdla_te_intr()	
 *
 * Description: Check interrupt type.
 * Arguments: 	card 		- pointer to device structure.
 * 		write_register 	- write register function.
 * 		read_register	- read register function.
 * Returns:	None.
 ******************************************************************************
 */
static int sdla_te_check_intr(sdla_fe_t *fe) 
{
	unsigned char	val;
	
	WAN_ASSERT(fe->write_fe_reg == NULL);
	WAN_ASSERT(fe->read_fe_reg == NULL);

	if (!wan_test_bit(TE_CONFIGURED,(void*)&fe->fe_param.te_param.critical)){
		return 0;
	}
	
	if (fe->fe_chip_id == CHIP_ID_COMET_QUAD){
		val = READ_REG_LINE(0, REG_COMET_QUAD_MASTER_INTR);
		val &= BITS_COMET_QUAD_MASTER_INTR;
		if (!(val & (1 << WAN_FE_LINENO(fe)))){
			DEBUG_TEST("%s: This interrupt not for this port %d\n",
					fe->name,
					WAN_FE_LINENO(fe)+1);
			return 0;
		}
	}

	return 1;
}

static int sdla_te_intr(sdla_fe_t *fe) 
{
	unsigned char	status = fe->fe_status; 
	unsigned char	val;
	
	WAN_ASSERT(fe->write_fe_reg == NULL);
	WAN_ASSERT(fe->read_fe_reg == NULL);
	
	if (fe->fe_chip_id == CHIP_ID_COMET_QUAD){
		val = READ_REG_LINE(0, REG_COMET_QUAD_MASTER_INTR);
		val &= BITS_COMET_QUAD_MASTER_INTR;
		if (!(val & (1 << WAN_FE_LINENO(fe)))){
			DEBUG_TEST("%s: This interrupt not for this port %d\n",
					fe->name,
					WAN_FE_LINENO(fe)+1);
			return 0;
		}
	}
	
	fe->fe_param.te_param.intr_src1 = READ_REG(REG_INT_SRC_1);
	fe->fe_param.te_param.intr_src2 = READ_REG(REG_INT_SRC_2);
	fe->fe_param.te_param.intr_src3 = READ_REG(REG_INT_SRC_3);
	DEBUG_ISR("%s: %s: INTR SRC1=0x%X SRC2=0x%X SRC3=0x%X\n",
			__FUNCTION__,
			fe->name,
			fe->fe_param.te_param.intr_src1,
			fe->fe_param.te_param.intr_src2,
			fe->fe_param.te_param.intr_src3);
	if (fe->fe_param.te_param.intr_src1 == 0 &&
	    fe->fe_param.te_param.intr_src2 == 0 && 
	    fe->fe_param.te_param.intr_src3 == 0){
		DEBUG_ISR("%s: Unknown %s interrupt!\n", 
				fe->name, 
				FE_MEDIA_DECODE(fe));
		return 0;
	}

	sdla_te_tx_intr(fe);
	sdla_te_rx_intr(fe);

	DEBUG_TEST("%s: FE Interrupt Alarms=0x%lX\n",
			fe->name,fe->fe_alarm);

	sdla_te_set_status(fe, fe->fe_alarm);
	if (status != fe->fe_status){
		if (fe->fe_status == FE_DISCONNECTED){
			sdla_te_clear_intr(fe);
			/* Start T1/E1 timer (5 sec) */
			sdla_te_enable_timer(fe, TE_LINKDOWN_TIMER, POLLING_TE1_TIMER * 5);
		}
	}

#if 0
	/*NCDEBUG*/
	if (fe->fe_status==FE_CONNECTED){
		/* Interrupt may not work for all alarms. Thus
		 * as soon as we have an interrupt we should 
		 * do a poll just in case */
		/*NCDEBUG*/
		DEBUG_EVENT("%s: FE Interrupt triggering poll!\n",fe->name);
		sdla_te_enable_timer(fe, TE_LINKDOWN_TIMER, POLLING_TE1_TIMER);
	}
#endif
	return 0;
}

/*
 ******************************************************************************
 *				sdla_te_tx_intr()	
 *
 * Description: Read tx interrupt.
 * Arguments: 	card		- pointer to device structure.
 * 		write_register	- write register function.
 * 		read_register	- read register function.
 * Returns: None.
 ******************************************************************************
 */
static void sdla_te_tx_intr(sdla_fe_t* fe)
{

	if (!(fe->fe_param.te_param.intr_src1 & BITS_TX_INT_SRC_1 || 
	      fe->fe_param.te_param.intr_src2 & BITS_TX_INT_SRC_2 ||  
     	      fe->fe_param.te_param.intr_src3 & BITS_TX_INT_SRC_3)){
		return;
	}

#if 0
	if (fe->fe_param.te_param.intr_src1 & BIT_INT_SRC_1_TJAT){
	}
	if (fe->fe_param.te_param.intr_src1 & BIT_INT_SRC_1_APRM){
	}
	if (fe->fe_param.te_param.intr_src2 & BIT_INT_SRC_2_TX_ELST){
	}
	if (fe->fe_param.te_param.intr_src2 & BIT_INT_SRC_2_TDPR_1){
	}
	if (fe->fe_param.te_param.intr_src2 & BIT_INT_SRC_2_TDPR_2){
	}
	if (fe->fe_param.te_param.intr_src2 & BIT_INT_SRC_2_TDPR_3){
	}
	if (fe->fe_param.te_param.intr_src3 & BIT_INT_SRC_3_TRAN){
	}
	if (fe->fe_param.te_param.intr_src3 & BIT_INT_SRC_3_XPDE){
	}
	if (fe->fe_param.te_param.intr_src3 & BIT_INT_SRC_3_BTIF){
	}
#endif
	return;
}


/*
 ******************************************************************************
 *				sdla_te_rx_intr()	
 *
 * Description: Read rx interrupt.
 * Arguments: 	card		- pointer to device structure.
 * 		write_register	- write register function.
 * 		read_register	- read register function.
 * Returns: None.
 ******************************************************************************
 */
static void sdla_te_rx_intr(sdla_fe_t* fe) 
{
	if (IS_T1_FEMEDIA(fe)){
		sdla_t1_rx_intr(fe);
	}else{
		sdla_e1_rx_intr(fe);
	}
	return;
}

/*
 ******************************************************************************
 *				sdla_t1_rx_intr()	
 *
 * Description: Read tx interrupt.
 * Arguments: 	card		- pointer to device structure.
 * 		write_register	- write register function.
 * 		read_register	- read register function.
 * Returns: None.
 ******************************************************************************
 */
static void sdla_t1_rx_intr(sdla_fe_t* fe) 
{
	unsigned char status = 0x00;

	if (!(fe->fe_param.te_param.intr_src1 & BITS_RX_INT_SRC_1 || 
	      fe->fe_param.te_param.intr_src2 & BITS_RX_INT_SRC_2 ||  
     	      fe->fe_param.te_param.intr_src3 & BITS_RX_INT_SRC_3)){
		return;
	}

	/* 3. PDVD */
	if (fe->fe_param.te_param.intr_src3 & BIT_INT_SRC_3_PDVD){
		status = READ_REG(REG_PDVD_INT_EN_STATUS);
		if ((status & BIT_PDVD_INT_EN_STATUS_PDVE) && 
		    (status & BIT_PDVD_INT_EN_STATUS_PDVI)){
			if (status & BIT_PDVD_INT_EN_STATUS_PDV){
				DEBUG_EVENT("%s: T1 pulse density violation detected!\n", 
						fe->name);
			}
		}
		if ((status & BIT_PDVD_INT_EN_STATUS_Z16DE) && 
		    (status & BIT_PDVD_INT_EN_STATUS_Z16DI)){
			DEBUG_EVENT("%s: T1 16 consecutive zeros detected!\n", 
						fe->name);
		}
	}

	/* 6. ALMI */
	if (fe->fe_param.te_param.intr_src3 & BIT_INT_SRC_3_ALMI){
		status = READ_REG(REG_T1_ALMI_INT_STATUS);
		if (status & BIT_T1_ALMI_INT_STATUS_YELI){
			if (status & BIT_T1_ALMI_INT_STATUS_YEL){
				if (!(fe->fe_alarm & WAN_TE_BIT_YEL_ALARM)){
#if 0
Alex Sep 16
					DEBUG_EVENT("%s: T1 YELLOW alarm is ON\n", 
							fe->name);
					fe->fe_alarm |= WAN_TE_BIT_YEL_ALARM;
#endif
				}	
			}else{
				if (fe->fe_alarm & WAN_TE_BIT_YEL_ALARM){
#if 0
Alex Sep 16
					DEBUG_EVENT("%s: T1 YELLOW alarm is OFF\n", 
							fe->name);
					fe->fe_alarm &= ~WAN_TE_BIT_YEL_ALARM;
#endif
				}	
			}	
		}
		if (status & BIT_T1_ALMI_INT_STATUS_REDI){
			if (status & BIT_T1_ALMI_INT_STATUS_RED){
				if (!(fe->fe_alarm & WAN_TE_BIT_RED_ALARM)){
					DEBUG_EVENT("%s: T1 RED alarm is ON\n", 
							fe->name);
					fe->fe_alarm |= WAN_TE_BIT_RED_ALARM;
				}	
			}else{
				if (fe->fe_alarm & WAN_TE_BIT_RED_ALARM){
					DEBUG_EVENT("%s: T1 RED alarm is OFF\n", 
							fe->name);
					fe->fe_alarm &= ~WAN_TE_BIT_RED_ALARM;
				}	
			}	
		}
		if (status & BIT_T1_ALMI_INT_STATUS_AISI){
			if (status & BIT_T1_ALMI_INT_STATUS_AIS){
				if (!(fe->fe_alarm & WAN_TE_BIT_AIS_ALARM)){ 
					DEBUG_EVENT("%s: T1 Alarm Indication Signal is ON\n", 
							fe->name);
					fe->fe_alarm |= WAN_TE_BIT_AIS_ALARM;
				}
			}else{
				if (fe->fe_alarm & WAN_TE_BIT_AIS_ALARM){ 
					DEBUG_EVENT("%s: T1 Alarm Indication Signal is OFF\n", 
							fe->name);
					fe->fe_alarm &= ~WAN_TE_BIT_AIS_ALARM;
				}
			}	
		}

#if 0	
		if (status & 
			(BIT_T1_ALMI_INT_STATUS_YELI | 
			 BIT_T1_ALMI_INT_STATUS_REDI | 
			 BIT_T1_ALMI_INT_STATUS_AISI)){
			if (status & (BIT_T1_ALMI_INT_STATUS_YEL | 
				      BIT_T1_ALMI_INT_STATUS_RED | 
				      BIT_T1_ALMI_INT_STATUS_AIS)){

				/* Update T1/E1 alarm status */
				if (!(fe->fe_alarm & WAN_TE_BIT_YEL_ALARM) && 
				    (status & BIT_T1_ALMI_INT_STATUS_YEL)){
					DEBUG_EVENT("%s: T1 YELLOW alarm is ON\n", 
							fe->name);
					fe->fe_alarm |= WAN_TE_BIT_YEL_ALARM;
				}
				if (!(fe->fe_alarm & WAN_TE_BIT_RED_ALARM) && 
				    (status & BIT_T1_ALMI_INT_STATUS_RED)){
					DEBUG_EVENT("%s: T1 RED alarm is ON\n", 
							fe->name);
					fe->fe_alarm |= WAN_TE_BIT_RED_ALARM;
				}
				if (!(fe->fe_alarm & WAN_TE_BIT_AIS_ALARM) && 
				    (status & BIT_T1_ALMI_INT_STATUS_AIS)){
					DEBUG_EVENT("%s: T1 Alarm Indication Signal is ON\n", 
							fe->name);
					fe->fe_alarm |= WAN_TE_BIT_AIS_ALARM;
				}
			}else{
				/* Update T1/E1 alarm status */
				if ((fe->fe_alarm & WAN_TE_BIT_YEL_ALARM) && 
				    !(status & BIT_T1_ALMI_INT_STATUS_YEL)){
					DEBUG_EVENT("%s: T1 YELLOW alarm is OFF\n", 
							fe->name);
					fe->fe_alarm &= ~WAN_TE_BIT_YEL_ALARM;
				}
				if ((fe->fe_alarm & WAN_TE_BIT_RED_ALARM) && 
				    !(status & BIT_T1_ALMI_INT_STATUS_RED)){
					DEBUG_EVENT("%s: T1 RED alarm is OFF\n", 
							fe->name);
					fe->fe_alarm &= ~WAN_TE_BIT_RED_ALARM;
				}
				if ((fe->fe_alarm & WAN_TE_BIT_AIS_ALARM) && 
				    !(status & BIT_T1_ALMI_INT_STATUS_AIS)){
					DEBUG_EVENT("%s: T1 Alarm Indication Signal is OFF\n", 
							fe->name);
					fe->fe_alarm &= ~WAN_TE_BIT_AIS_ALARM;
				}
			}
		}
#endif
	} 

	/* 8. RBOC */
	if (fe->fe_param.te_param.intr_src3 & BIT_INT_SRC_3_RBOC){
		status = READ_REG(REG_T1_RBOC_CODE_STATUS);
		if (status & BIT_T1_RBOC_CODE_STATUS_BOCI){
			unsigned long 	time;
			time = SYSTEM_TICKS;
			status &= MASK_T1_RBOC_CODE_STATUS;
			switch(status){
			case LINELB_ACTIVATE_CODE:
			case LINELB_DEACTIVATE_CODE:
				if (wan_test_bit(LINELB_WAITING,(void*)&fe->fe_param.te_param.critical) &&
				    wan_test_bit(LINELB_CODE_BIT,(void*)&fe->fe_param.te_param.critical)){
				    	wan_clear_bit(LINELB_CODE_BIT,(void*)&fe->fe_param.te_param.critical);
					break;
				}
				
				DEBUG_TE1("%s: T1 loopback %s code received.\n",
						fe->name,
						(status==LINELB_ACTIVATE_CODE)?
							"activation" : "deactivation");
				fe->fe_param.te_param.lb_cmd = status;
				fe->fe_param.te_param.lb_time = time;
				break;

			case LINELB_DS1LINE_ALL:
				if (wan_test_bit(LINELB_WAITING,(void*)&fe->fe_param.te_param.critical) &&
				    wan_test_bit(LINELB_CHANNEL_BIT,(void*)&fe->fe_param.te_param.critical)){
				    	wan_clear_bit(LINELB_CHANNEL_BIT,(void*)&fe->fe_param.te_param.critical);
				    	wan_clear_bit(LINELB_WAITING,(void*)&fe->fe_param.te_param.critical);
					break;
				}
				if (!fe->fe_param.te_param.lb_cmd)
					break; 
				if ((time - fe->fe_param.te_param.lb_time) < LINELB_TE1_TIMER){
					DEBUG_TE1("%s: T1 loopback %s mode cancelled!\n",
							fe->name,
							(fe->fe_param.te_param.lb_cmd == LINELB_ACTIVATE_CODE) ? 
								"activatation" : 
								"deactivation");
				}else{
					if (fe->fe_param.te_param.lb_cmd == LINELB_ACTIVATE_CODE){
						DEBUG_EVENT("%s: T1 loopback mode activated.\n", 
							fe->name); 
						WRITE_REG(REG_MASTER_DIAG, 
							READ_REG(REG_MASTER_DIAG) | 
							BIT_MASTER_DIAG_LINELB);
						fe->fe_param.te_param.lb_rx_code = LINELB_DS1LINE_ALL;
					}else{
						DEBUG_EVENT("%s: T1 loopback mode deactivated.\n", 
							fe->name);
						WRITE_REG(REG_MASTER_DIAG, 
							READ_REG(REG_MASTER_DIAG) & 
							~BIT_MASTER_DIAG_LINELB);
						fe->fe_param.te_param.lb_rx_code = LINELB_DS1LINE_DISABLE;
					}
				}
				fe->fe_param.te_param.lb_cmd = 0x00;
				fe->fe_param.te_param.lb_time = 0x00;
				break;
		
			case LINELB_DS3LINE:
				break;

			case LINELB_DS1LINE_1:
			case LINELB_DS1LINE_2:
			case LINELB_DS1LINE_3:	
			case LINELB_DS1LINE_4:
			case LINELB_DS1LINE_5:	
			case LINELB_DS1LINE_6:	
			case LINELB_DS1LINE_7:	
			case LINELB_DS1LINE_8:	
			case LINELB_DS1LINE_9:
			case LINELB_DS1LINE_10:
			case LINELB_DS1LINE_11:
			case LINELB_DS1LINE_12:
			case LINELB_DS1LINE_13:
			case LINELB_DS1LINE_14:
			case LINELB_DS1LINE_15:
			case LINELB_DS1LINE_16:
			case LINELB_DS1LINE_17:
			case LINELB_DS1LINE_18:
			case LINELB_DS1LINE_19:
			case LINELB_DS1LINE_20:
			case LINELB_DS1LINE_21:
			case LINELB_DS1LINE_22:	
			case LINELB_DS1LINE_23:	
			case LINELB_DS1LINE_24:	
			case LINELB_DS1LINE_25:
			case LINELB_DS1LINE_26:	
			case LINELB_DS1LINE_27:
			case LINELB_DS1LINE_28:
				if (!fe->fe_param.te_param.lb_cmd)
					break;
				if ((time - fe->fe_param.te_param.lb_time) < LINELB_TE1_TIMER){
					DEBUG_TE1("%s: T1 loopback %s mode cancelled!\n",
							fe->name,
							(fe->fe_param.te_param.lb_cmd == LINELB_ACTIVATE_CODE) ? 
								"activatation" : 
								"deactivation");
				}else{
					int channel = status & LINELB_DS1LINE_MASK; 
					DEBUG_EVENT("%s: T1 loopback mode %s on channel %d.\n", 
						fe->name,
						(fe->fe_param.te_param.lb_cmd == LINELB_ACTIVATE_CODE) ? "activated" : "deactivated",
						channel);
					SetLoopBackChannel(fe, channel, fe->fe_param.te_param.lb_cmd);
       					fe->fe_param.te_param.lb_rx_code = status;
				}
				fe->fe_param.te_param.lb_cmd = 0x00;				
				fe->fe_param.te_param.lb_time = 0x00;				
				break;
			case BITS_T1_XBOC_DISABLE:
      				if (fe->fe_param.te_param.lb_rx_code == LINELB_DS1LINE_ALL){
       					DEBUG_EVENT("%s: T1 loopback mode deactivated.\n", 
							fe->name);
       					WRITE_REG(REG_MASTER_DIAG, 
							READ_REG(REG_MASTER_DIAG) & 
							~BIT_MASTER_DIAG_LINELB);
       					fe->fe_param.te_param.lb_rx_code = LINELB_DS1LINE_DISABLE;

				}
				break;

			default:
       			       DEBUG_TE1(
       				"%s: Received Reserved Loopback code %02x.\n", 
							fe->name, status);
				break;
			}
		}
	}

	/* 7. FRMR */
	if (fe->fe_param.te_param.intr_src1 & BIT_INT_SRC_1_FRMR){
		status = READ_REG(REG_T1_FRMR_INT_STATUS);
		if ((READ_REG(REG_T1_FRMR_INT_EN) & BIT_T1_FRMR_INT_EN_INFRE) &&
		    (status & BIT_T1_FRMR_INT_STATUS_INFRI)){
			if (status & BIT_T1_FRMR_INT_STATUS_INFR){
				if (!(fe->fe_alarm & WAN_TE_BIT_OOF_ALARM)){
					DEBUG_EVENT("%s: T1 Out of Frame alarm is ON!\n",
						fe->name);
					fe->fe_alarm |= WAN_TE_BIT_OOF_ALARM; 
				}
			}else{
				if (fe->fe_alarm & WAN_TE_BIT_OOF_ALARM){
					DEBUG_EVENT("%s: T1 Out of Frame alarm is OFF!\n",
						fe->name);
					fe->fe_alarm &= ~WAN_TE_BIT_OOF_ALARM; 
				}
			}
		}
	}

	/* 1. RLPS */
	if (fe->fe_param.te_param.intr_src3 & BIT_INT_SRC_3_RLPS){
		status = READ_REG(REG_RLPS_CFG_STATUS);
		if ((status & BIT_RLPS_CFG_STATUS_ALOSE) && (status & BIT_RLPS_CFG_STATUS_ALOSI)){
			if (status & BIT_RLPS_CFG_STATUS_ALOSV){
				if (!(fe->fe_alarm & WAN_TE_BIT_ALOS_ALARM)){
					if (WAN_NET_RATELIMIT()){
					DEBUG_EVENT("%s: T1 ALOS alarm is ON\n", 
						fe->name);
					}
					fe->fe_alarm |= WAN_TE_BIT_ALOS_ALARM;
				}
			}else{
				if (fe->fe_alarm & WAN_TE_BIT_ALOS_ALARM){
					if (WAN_NET_RATELIMIT()){
					DEBUG_EVENT("%s: T1 ALOS alarm is OFF\n", 
						fe->name);
					}
					fe->fe_alarm &= ~WAN_TE_BIT_ALOS_ALARM;
				}
			}
		}
	}

	/* 2. CDRC */
	if (fe->fe_param.te_param.intr_src1 & BIT_INT_SRC_1_CDRC){
		status = READ_REG(REG_CDRC_INT_STATUS);
		if ((READ_REG(REG_CDRC_INT_EN) & BIT_CDRC_INT_EN_LOSE) && 
		    (status & BIT_CDRC_INT_STATUS_LOSI)){
			if (status & BIT_CDRC_INT_STATUS_LOSV){
				if (!(fe->fe_alarm & WAN_TE_BIT_LOS_ALARM)){
					if (WAN_NET_RATELIMIT()){
					DEBUG_EVENT("%s: T1 LOS alarm is ON\n", 
							fe->name);
					}
					fe->fe_alarm |= WAN_TE_BIT_LOS_ALARM;
				}
			}else{
				if (fe->fe_alarm & WAN_TE_BIT_LOS_ALARM){
					if (WAN_NET_RATELIMIT()){
					DEBUG_EVENT("%s: T1 LOS alarm is OFF\n", 
							fe->name);
					}
					fe->fe_alarm &= ~WAN_TE_BIT_LOS_ALARM;
				}
			}
		}
		if ((READ_REG(REG_CDRC_INT_EN) & BIT_CDRC_INT_EN_LCVE) && 
		    (status & BIT_CDRC_INT_STATUS_LCVI)){
			DEBUG_EVENT("%s: T1 line code violation!\n", 
					fe->name);
		}
		if ((READ_REG(REG_CDRC_INT_EN) & BIT_CDRC_INT_EN_LCSDE) && 
		    (status & BIT_CDRC_INT_STATUS_LCSDI)){
			DEBUG_EVENT("%s: T1 line code signature detected!\n", 
					fe->name);
		}
		if ((READ_REG(REG_CDRC_INT_EN) & BIT_CDRC_INT_EN_ZNDE) && 
		    (status & BIT_CDRC_INT_STATUS_ZNDI)){
			DEBUG_EVENT("%s: T1 consecutive zeros detected!\n", 
					fe->name);
		}
		status = READ_REG(REG_ALTLOS_STATUS);
		if ((status & BIT_ALTLOS_STATUS_ALTLOSI) && (status & BIT_ALTLOS_STATUS_ALTLOSE)){
			if (status & BIT_ALTLOS_STATUS_ALTLOS){
				if (!(fe->fe_alarm & WAN_TE_BIT_ALTLOS_ALARM)){
					DEBUG_EVENT("%s: T1 Alternate Loss of Signal alarm is ON\n",
							fe->name);
					fe->fe_alarm |= WAN_TE_BIT_ALTLOS_ALARM;
				}
			}else{
				if (fe->fe_alarm & WAN_TE_BIT_ALTLOS_ALARM){
					DEBUG_EVENT("%s: T1 Alternate Loss of Signal alarm is OFF\n",
							fe->name);
					fe->fe_alarm &= ~WAN_TE_BIT_ALTLOS_ALARM;
				}
			}
		}
	}

	/* 14. PMON */
	if (fe->fe_param.te_param.intr_src1 & BIT_INT_SRC_1_PMON){
		status = READ_REG(REG_PMON_INT_EN_STATUS);
		if (status & BIT_PMON_INT_EN_STATUS_XFER){
			DEBUG_TE1("%s: T1 Updating PMON counters...\n",
					fe->name);
			sdla_te_pmon(fe);
		}
	}

	/* 9. SIGX */
	if (fe->fe_param.te_param.intr_src1 & BIT_INT_SRC_1_SIGX){
#if 0
		unsigned char SIGX_chg_30_25;
		unsigned char SIGX_chg_24_17;
		unsigned char SIGX_chg_16_9;
		unsigned char SIGX_chg_8_1;
		
		SIGX_chg_30_25	= READ_REG(REG_SIGX_CFG);
		SIGX_chg_24_17	= READ_REG(REG_SIGX_TIMESLOT_IND_STATUS);
		SIGX_chg_16_9	= READ_REG(REG_SIGX_TIMESLOT_IND_ACCESS);
		SIGX_chg_8_1	= READ_REG(REG_SIGX_TIMESLOT_IND_DATA_BUFFER);

#if 0
		if(IS_E1_FEMEDIA(fe)) {
			DEBUG_EVENT("%s: SIGX chg 30-25: 0x%02X\n",
		      		fe->name, SIGX_chg_30_25);
		}
		DEBUG_EVENT("%s: SIGX chg 24-17: 0x%02X\n", 
					fe->name, SIGX_chg_24_17);
		DEBUG_EVENT("%s: SIGX chg 16-9 : 0x%02X\n", 
					fe->name, SIGX_chg_16_9);
		DEBUG_EVENT("%s: SIGX chg 8-1  : 0x%02X\n", 
					fe->name, SIGX_chg_8_1);
#endif

		if (SIGX_chg_8_1){
			sdla_te_check_rbsbits(fe, SIGX_chg_8_1, 1, 1); 
		/*	status = READ_SIGX_REG(0x10);*/
		/*	printk(KERN_INFO "SIGX reg 0x10: 0x%02X\n", status);*/
		}
		
		if (SIGX_chg_16_9){
			sdla_te_check_rbsbits(fe, SIGX_chg_16_9, 9, 1);
		}

		if (SIGX_chg_24_17){
			sdla_te_check_rbsbits(fe, SIGX_chg_24_17, 17, 1);
		}

		if (SIGX_chg_30_25 && IS_E1_FEMEDIA(fe)){
			sdla_te_check_rbsbits(fe, SIGX_chg_30_25, 25, 1);
		}
#endif
		sdla_te_check_rbsbits(fe, 1, ENABLE_ALL_CHANNELS, 1); 
		sdla_te_check_rbsbits(fe, 9, ENABLE_ALL_CHANNELS, 1);
		sdla_te_check_rbsbits(fe, 17, ENABLE_ALL_CHANNELS, 1);
		if (IS_E1_FEMEDIA(fe)){
			sdla_te_check_rbsbits(fe, 25, ENABLE_ALL_CHANNELS, 1);
		}
	}

	/* 5. IBCD */
	fe->fe_alarm &= ~(WAN_TE_BIT_LOOPUP_CODE|WAN_TE_BIT_LOOPDOWN_CODE);
	if (fe->fe_param.te_param.intr_src3 & BIT_INT_SRC_3_IBCD){
		status = READ_REG(REG_IBCD_INT_EN_STATUS);
		if (status & BIT_IBCD_INT_EN_STATUS_LBAI){
			fe->fe_alarm |= WAN_TE_BIT_LOOPUP_CODE;
		}
		if (status & BIT_IBCD_INT_EN_STATUS_LBDI){
			fe->fe_alarm |= WAN_TE_BIT_LOOPDOWN_CODE;
		}
	}
#if 0
	/* 4. RJAT */
	if (fe->fe_param.te_param.intr_src1 & BIT_INT_SRC_1_RJAT){
	}
	/* 10. RX-ELST */
	if (fe->fe_param.te_param.intr_src2 & BIT_INT_SRC_2_RX_ELST){
	}	
	/* 11. RDLC-1 */
	if (fe->fe_param.te_param.intr_src2 & BIT_INT_SRC_2_RDLC_1){
	}
	/* 12. RDLC-2 */
	if (fe->fe_param.te_param.intr_src2 & BIT_INT_SRC_2_RDLC_2){
	}
	/* 13. RDLC-3 */
	if (fe->fe_param.te_param.intr_src2 & BIT_INT_SRC_2_RDLC_3){
	}
#endif

	return;
}


/*
 ******************************************************************************
 *				sdla_e1_rx_intr()	
 *
 * Description: Read tx interrupt.
 * Arguments: 	card		- pointer to device structure.
 * 		write_register	- write register function.
 * 		read_register	- read register function.
 * Returns: None.
 ******************************************************************************
 */
static void sdla_e1_rx_intr(sdla_fe_t* fe) 
{
	unsigned char int_status = 0x00, status = 0x00;

	if (!(fe->fe_param.te_param.intr_src1 & BITS_RX_INT_SRC_1 || 
	      fe->fe_param.te_param.intr_src2 & BITS_RX_INT_SRC_2 ||  
     	      fe->fe_param.te_param.intr_src3 & BITS_RX_INT_SRC_3))
		return;
	
	/* 4. FRMR */
	if (fe->fe_param.te_param.intr_src1 & BIT_INT_SRC_1_FRMR){
		/* Register 0x92h E1 FRMR Framing Status Interrupt Enable */
		unsigned char	int_en = READ_REG(REG_E1_FRMR_FRM_STAT_INT_EN);
		/* Register 0x94h E1 FRMR */
		int_status = READ_REG(REG_E1_FRMR_FRM_STAT_INT_IND);
		/* Register 0x96h E1 FRMR Status */
		status = READ_REG(REG_E1_FRMR_FR_STATUS);
		if ((int_en & BIT_E1_FRMR_FRM_STAT_INT_EN_OOFE) && 
		    (int_status & BIT_E1_FRMR_FRM_STAT_INT_IND_OOFI)){
			if (status & BIT_E1_FRMR_FR_STATUS_OOFV){
				if (!(fe->fe_alarm & WAN_TE_BIT_OOF_ALARM)){
					DEBUG_EVENT("%s: E1 Out of Frame alarm is ON\n",
							fe->name);
					fe->fe_alarm |= WAN_TE_BIT_OOF_ALARM; 
				}
			}else{
				if (fe->fe_alarm & WAN_TE_BIT_OOF_ALARM){
					DEBUG_EVENT("%s: E1 Out of Frame alarm is OFF\n",
							fe->name);
					fe->fe_alarm &= ~WAN_TE_BIT_OOF_ALARM; 
				}
			}
		}

		if ((int_en & BIT_E1_FRMR_FRM_STAT_INT_EN_OOSMFE) && 
		    (int_status & BIT_E1_FRMR_FRM_STAT_INT_IND_OOSMFI)){
			if (status & BIT_E1_FRMR_FR_STATUS_OOSMFV){
				DEBUG_EVENT("%s: E1 Loss of Signaling multiframe alarm is ON\n",
					fe->name);
				fe->fe_alarm |= WAN_TE_BIT_OOSMF_ALARM; 
			}else{
				DEBUG_EVENT("%s: E1 Loss of Signaling multiframe alarm is OFF\n",
					fe->name);
				fe->fe_alarm &= ~WAN_TE_BIT_OOSMF_ALARM; 
			}
		}

		if ((int_en & BIT_E1_FRMR_FRM_STAT_INT_EN_OOCMFE) && 
		    (int_status & BIT_E1_FRMR_FRM_STAT_INT_IND_OOCMFI)){
			if (status & BIT_E1_FRMR_FR_STATUS_OOCMFV){
				DEBUG_EVENT("%s: E1 Loss of CRC multiframe alarm is ON\n",
						fe->name);
				fe->fe_alarm |= WAN_TE_BIT_OOCMF_ALARM; 
			}else{
				DEBUG_EVENT("%s: E1 Loss of CRC multiframe alarm is OFF\n",
						fe->name);
				fe->fe_alarm &= ~WAN_TE_BIT_OOCMF_ALARM; 
			}
		}

		/* Register 0x9Fh E1 FRMR */
		status = READ_REG(REG_E1_FRMR_P_A_INT_STAT);
		if ((READ_REG(REG_E1_FRMR_P_A_INT_EN) & BIT_E1_FRMR_P_A_INT_EN_OOOFE) && 
		    (status & BIT_E1_FRMR_P_A_INT_STAT_OOOFI)){
			if (READ_REG(REG_E1_FRMR_FR_STATUS) & BIT_E1_FRMR_FR_STATUS_OOOFV){
				DEBUG_EVENT("%s: E1 out of offline frame alarm is ON\n",
					fe->name);
				fe->fe_alarm |= WAN_TE_BIT_OOOF_ALARM; 
			}else{
				DEBUG_EVENT("%s: E1 out of offline frame alarm is OFF\n",
					fe->name);
				fe->fe_alarm &= ~WAN_TE_BIT_OOOF_ALARM; 
			}
		}

		/* Register 0x95h E1 FRMR */
		int_status = READ_REG(REG_E1_FRMR_M_A_INT_IND);
		if (int_status & (BIT_E1_FRMR_M_A_INT_IND_REDI |
				  BIT_E1_FRMR_M_A_INT_IND_AISI |
				  BIT_E1_FRMR_M_A_INT_IND_RAII)){
			unsigned char	int_en = READ_REG(REG_E1_FRMR_M_A_INT_EN);
			status = READ_REG(REG_E1_FRMR_MAINT_STATUS);
			if ((int_en & BIT_E1_FRMR_M_A_INT_EN_REDE) &&
			    (int_status & BIT_E1_FRMR_M_A_INT_IND_REDI)){
				if (status & BIT_E1_FRMR_MAINT_STATUS_RED){
					DEBUG_EVENT("%s: E1 RED alarm is ON\n", fe->name);
					fe->fe_alarm |= WAN_TE_BIT_RED_ALARM;
				}else{
					DEBUG_EVENT("%s: E1 RED alarm is OFF\n", fe->name);
					fe->fe_alarm &= ~WAN_TE_BIT_RED_ALARM;
				}
			}
			if ((int_en & BIT_E1_FRMR_M_A_INT_EN_AISE) &&
			    (int_status & BIT_E1_FRMR_M_A_INT_IND_AISI)){
				if (status & BIT_E1_FRMR_MAINT_STATUS_AIS){
					DEBUG_EVENT("%s: E1 AIS alarm is ON\n", fe->name);
					fe->fe_alarm |= WAN_TE_BIT_AIS_ALARM;
					/* AS/ACIF S016 Clause 5.2.3 */
					WRITE_REG(REG_E1_TRAN_TX_ALARM_CTRL,
						  READ_REG(REG_E1_TRAN_TX_ALARM_CTRL) | 
							BIT_E1_TRAN_TX_ALARM_RAI);
					if (WAN_TE1_CLK(fe) == WAN_NORMAL_CLK){
						sdla_te_master_clock(fe);
					}
				}else{
					DEBUG_EVENT("%s: E1 AIS alarm is OFF\n", fe->name);
					fe->fe_alarm &= ~WAN_TE_BIT_AIS_ALARM;
					/* AS/ACIF S016 Clause 5.2.3 */
					WRITE_REG(REG_E1_TRAN_TX_ALARM_CTRL,
						  READ_REG(REG_E1_TRAN_TX_ALARM_CTRL) & 
							~BIT_E1_TRAN_TX_ALARM_RAI);
					if (WAN_TE1_CLK(fe) == WAN_NORMAL_CLK){
						sdla_te_normal_clock(fe);
					}
				}
			}
			if ((int_en & BIT_E1_FRMR_M_A_INT_EN_RAIE) &&
			    (int_status & BIT_E1_FRMR_M_A_INT_IND_RAII)){
				if (status & BIT_E1_FRMR_MAINT_STATUS_RAIV){
					DEBUG_EVENT("%s: E1 RAI alarm is ON\n", fe->name);
					fe->fe_alarm |= WAN_TE_BIT_RAI_ALARM;
				}else{
					DEBUG_EVENT("%s: E1 RAI alarm is OFF\n", fe->name);
					fe->fe_alarm &= ~WAN_TE_BIT_RAI_ALARM;
				}
			}
		}
	}

	/* 1. RLPS */
	if (fe->fe_param.te_param.intr_src3 & BIT_INT_SRC_3_RLPS){
		status = READ_REG(REG_RLPS_CFG_STATUS);
		if ((status & BIT_RLPS_CFG_STATUS_ALOSE) &&
		    (status & BIT_RLPS_CFG_STATUS_ALOSI)){
			if (status & BIT_RLPS_CFG_STATUS_ALOSV){
				if (!(fe->fe_alarm & WAN_TE_BIT_ALOS_ALARM)){
					if (WAN_NET_RATELIMIT()){ 
					DEBUG_EVENT("%s: E1 ALOS alarm is ON\n", 
						fe->name);
					}
					fe->fe_alarm |= WAN_TE_BIT_ALOS_ALARM;
					/* AS/ACIF S016 Clause 5.2.3 */
					WRITE_REG(REG_E1_TRAN_TX_ALARM_CTRL,
						  READ_REG(REG_E1_TRAN_TX_ALARM_CTRL) | 
							BIT_E1_TRAN_TX_ALARM_RAI);
				}
			}else{
				if (fe->fe_alarm & WAN_TE_BIT_ALOS_ALARM){
					if (WAN_NET_RATELIMIT()){ 
					DEBUG_EVENT("%s: E1 ALOS alarm is OFF\n", 
						fe->name);
					}
					fe->fe_alarm &= ~WAN_TE_BIT_ALOS_ALARM;
					/* AS/ACIF S016 Clause 5.2.3 */
					WRITE_REG(REG_E1_TRAN_TX_ALARM_CTRL,
						  READ_REG(REG_E1_TRAN_TX_ALARM_CTRL) & 
							~BIT_E1_TRAN_TX_ALARM_RAI);
				}
			}
		}
	}

	/* 2. CDRC */
	if (fe->fe_param.te_param.intr_src1 & BIT_INT_SRC_1_CDRC){
		status = READ_REG(REG_CDRC_INT_STATUS);
		if ((READ_REG(REG_CDRC_INT_EN) & BIT_CDRC_INT_EN_LOSE) && 
		    (status & BIT_CDRC_INT_STATUS_LOSI)){
			if (status & BIT_CDRC_INT_STATUS_LOSV){
				if (!(fe->fe_alarm & WAN_TE_BIT_LOS_ALARM)){
					if (WAN_NET_RATELIMIT()){ 
					DEBUG_EVENT("%s: E1 LOS alarm is ON\n", fe->name);
					}
					fe->fe_alarm |= WAN_TE_BIT_LOS_ALARM;
				}
			}else{
				if (fe->fe_alarm & WAN_TE_BIT_LOS_ALARM){
					if (WAN_NET_RATELIMIT()){ 
					DEBUG_EVENT("%s: E1 LOS alarm is OFF\n", fe->name);
					}
					fe->fe_alarm &= ~WAN_TE_BIT_LOS_ALARM;
				}
			}
		}
		if ((READ_REG(REG_CDRC_INT_EN) & BIT_CDRC_INT_EN_LCVE) && 
		    (status & BIT_CDRC_INT_STATUS_LCVI)){
			DEBUG_EVENT("%s: E1 line code violation!\n", fe->name);
		}
		if ((READ_REG(REG_CDRC_INT_EN) & BIT_CDRC_INT_EN_LCSDE) && 
		    (status & BIT_CDRC_INT_STATUS_LCSDI)){
			DEBUG_EVENT("%s: E1 line code signature detected!\n", fe->name);
		}
		if ((READ_REG(REG_CDRC_INT_EN) & BIT_CDRC_INT_EN_ZNDE) && 
		    (status & BIT_CDRC_INT_STATUS_ZNDI)){
			DEBUG_EVENT("%s: E1 consecutive zeros detected!\n", fe->name);
		}
		status = READ_REG(REG_ALTLOS_STATUS);
		if ((status & BIT_ALTLOS_STATUS_ALTLOSI) && 
		    (status & BIT_ALTLOS_STATUS_ALTLOSE)){
			if (status & BIT_ALTLOS_STATUS_ALTLOS){
				if (!(fe->fe_alarm & WAN_TE_BIT_ALTLOS_ALARM)){
					DEBUG_EVENT("%s: E1 Alternate Loss of Signal alarm is ON\n",
							fe->name);
					fe->fe_alarm |= WAN_TE_BIT_ALTLOS_ALARM;
				}
			}else{
				if (fe->fe_alarm & WAN_TE_BIT_ALTLOS_ALARM){
					DEBUG_EVENT("%s: E1 Alternate Loss of Signal alarm is OFF\n",
							fe->name);
					fe->fe_alarm &= ~WAN_TE_BIT_ALTLOS_ALARM;
				}
			}
		}
	}
	/* 11. PMON */
	if (fe->fe_param.te_param.intr_src1 & BIT_INT_SRC_1_PMON){
		status = READ_REG(REG_PMON_INT_EN_STATUS);
		if (status & BIT_PMON_INT_EN_STATUS_XFER){
			sdla_te_pmon(fe);
		}
	}
#if 0
	/* 3. RJAT */
	if (fe->fe_param.te_param.intr_src1 & BIT_INT_SRC_1_RJAT){
	}
	/* 5. SIGX */
	if (fe->fe_param.te_param.intr_src1 & BIT_INT_SRC_1_SIGX){
	}
	/* 6. RX-ELST */
	if (fe->fe_param.te_param.intr_src2 & BIT_INT_SRC_2_RX_ELST){
	}
	/* 7. PRGD */
	if (fe->fe_param.te_param.intr_src1 & BIT_INT_SRC_1_PRGD){
	}
	/* 8. RDLC-1 */
	if (fe->fe_param.te_param.intr_src2 & BIT_INT_SRC_2_RDLC_1){
	}
	/* 9. RDLC-2 */
	if (fe->fe_param.te_param.intr_src2 & BIT_INT_SRC_2_RDLC_2){
	}
	/* 10. RDLC-3 */
	if (fe->fe_param.te_param.intr_src2 & BIT_INT_SRC_2_RDLC_3){
	}
#endif
	if (!(READ_REG(REG_RLPS_CFG_STATUS) & BIT_RLPS_CFG_STATUS_ALOSV)){
		fe->fe_alarm &= ~WAN_TE_BIT_ALOS_ALARM;
	}
	return;
}

/*
 ******************************************************************************
 *				sdla_te_set_lbmode()	
 *
 * Description:
 * Arguments:
 * Returns:
 ******************************************************************************
 */
static int 
sdla_te_set_lbmode(sdla_fe_t *fe, unsigned char type, unsigned char mode) 
{
	int	err = 1;

	WAN_ASSERT(fe->write_fe_reg == NULL);
	WAN_ASSERT(fe->read_fe_reg == NULL);
	switch(type){
	case WAN_TE1_LINELB_MODE:
		err = sdla_te_linelb(fe, mode); 
		break;
	case WAN_TE1_PAYLB_MODE:
		err = sdla_te_paylb(fe, mode);
		break;
	case WAN_TE1_DDLB_MODE:
		err = sdla_te_ddlb(fe, mode);
		break;
	case WAN_TE1_TX_LB_MODE:
		err = sdla_te_lb(fe, mode); 
		break;
	}

	return err;
}

/*
 ******************************************************************************
 *				sdla_te_linelb()	
 *
 * Description:
 * Arguments:
 * Returns:
 ******************************************************************************
 */
static int sdla_te_linelb(sdla_fe_t* fe, unsigned char mode) 
{

	WAN_ASSERT(fe->write_fe_reg == NULL);
	WAN_ASSERT(fe->read_fe_reg == NULL);
	if (mode == WAN_TE1_ACTIVATE_LB){
		DEBUG_EVENT("%s: %s Line Loopback mode activated.\n",
			fe->name,
			FE_MEDIA_DECODE(fe));
		WRITE_REG(REG_MASTER_DIAG, 
			READ_REG(REG_MASTER_DIAG) | BIT_MASTER_DIAG_LINELB);
	}else{
		DEBUG_EVENT("%s: %s Line Loopback mode deactivated.\n",
			fe->name,
			FE_MEDIA_DECODE(fe));
		WRITE_REG(REG_MASTER_DIAG, 
			READ_REG(REG_MASTER_DIAG) & ~BIT_MASTER_DIAG_LINELB);
	}
	return 0;
}

/*
 ******************************************************************************
 *				sdla_te_paylb()	
 *
 * Description:
 * Arguments:
 * Returns:
 ******************************************************************************
 */
static int sdla_te_paylb(sdla_fe_t* fe, unsigned char mode) 
{
	WAN_ASSERT(fe->write_fe_reg == NULL);
	WAN_ASSERT(fe->read_fe_reg == NULL);
	if (mode == WAN_TE1_ACTIVATE_LB){
		DEBUG_EVENT("%s: %s Payload Loopback mode activated.\n",
			fe->name,
			FE_MEDIA_DECODE(fe));
		WRITE_REG(REG_MASTER_DIAG, 
			READ_REG(REG_MASTER_DIAG) | BIT_MASTER_DIAG_PAYLB);
	}else{
		DEBUG_EVENT("%s: %s Payload Loopback mode deactivated.\n",
			fe->name,
			FE_MEDIA_DECODE(fe));
		WRITE_REG(REG_MASTER_DIAG, 
			READ_REG(REG_MASTER_DIAG) & ~BIT_MASTER_DIAG_PAYLB);
	}
	return 0;
}

/*
 ******************************************************************************
 *				sdla_te_ddlb()	
 *
 * Description:
 * Arguments:
 * Returns:
 ******************************************************************************
 */
static int sdla_te_ddlb(sdla_fe_t* fe, unsigned char mode) 
{
	WAN_ASSERT(fe->write_fe_reg == NULL);
	WAN_ASSERT(fe->read_fe_reg == NULL);
	if (mode == WAN_TE1_ACTIVATE_LB){
		DEBUG_EVENT("%s: %s Diagnostic Digital Loopback mode activated.\n",
			fe->name,
			FE_MEDIA_DECODE(fe));
		WRITE_REG(REG_MASTER_DIAG, 
			READ_REG(REG_MASTER_DIAG) | BIT_MASTER_DIAG_DDLB);
	}else{
		DEBUG_EVENT("%s: %s Diagnostic Digital Loopback mode deactivated.\n",
			fe->name,
			FE_MEDIA_DECODE(fe));
		WRITE_REG(REG_MASTER_DIAG, 
			READ_REG(REG_MASTER_DIAG) & ~BIT_MASTER_DIAG_DDLB);
	}
	return 0;
}

/*
 ******************************************************************************
 *				sdla_te_timer()	
 *
 * Description:
 * Arguments:
 * Returns:
 ******************************************************************************
 */
#if defined(__NetBSD__) || defined(__FreeBSD__) || defined(__OpenBSD__)
static void sdla_te_timer(void* pfe)
#elif defined(__WINDOWS__)
static void sdla_te_timer(IN PKDPC Dpc, void* pfe, void* arg2, void* arg3)
#else
static void sdla_te_timer(unsigned long pfe)
#endif
{
	sdla_fe_t	*fe = (sdla_fe_t*)pfe;
	sdla_t 		*card = (sdla_t*)fe->card;
	wan_device_t	*wandev = &card->wandev;

	if (wan_test_bit(TE_TIMER_KILL,(void*)&fe->fe_param.te_param.critical)){
		wan_clear_bit(TE_TIMER_RUNNING,(void*)&fe->fe_param.te_param.critical);
		return;
	}
	/*WAN_ASSERT1(wandev->te_enable_timer == NULL); */
	DEBUG_TIMER("%s: TE1 timer!\n", fe->name);
	/* Enable hardware interrupt for TE1 */

	if (wandev->te_enable_timer){
		wandev->te_enable_timer(fe->card);
	}else{
		sdla_te_polling(fe);
	}

	return;
}

/*
 ******************************************************************************
 *				sdla_te_enable_timer()	
 *
 * Description: Enable software timer interrupt in delay ms.
 * Arguments:
 * Returns:
 ******************************************************************************
 */
static void sdla_te_enable_timer(sdla_fe_t* fe, unsigned char cmd, unsigned long delay)
{
	sdla_t	*card = (sdla_t*)fe->card;

	WAN_ASSERT1(card == NULL);
	DEBUG_TEST("%s: %s:%d Cmd=0x%X\n",
			fe->name,__FUNCTION__,__LINE__,cmd);

#if defined (__WINDOWS__)
	if(KeGetCurrentIrql() > DISPATCH_LEVEL){
		/*	May get here on AFT card because front end interrupt
			is handled inside ISR not in DPC as on S514.
			The KeSetTimer() function is illegal inside ISR,
			so queue 'front_end_dpc_obj' DPC and this routine
			will be called again from xilinx_front_end_dpc().
		*/
		card->xilinx_fe_dpc.te_timer_delay = delay;
		fe->fe_param.te_param.timer_cmd=(unsigned char)cmd;

		if(KeInsertQueueDpc(&card->front_end_dpc_obj, NULL,
			(PVOID)ENABLE_TE_TIMER) == FALSE){

			DEBUG_TE1("Failed to queue 'front_end_dpc_obj'!\n");
		}else{
			DEBUG_TEST("Successfully queued 'front_end_dpc_obj'.\n");
		}
		return;
	}//if()
#endif
	if (wan_test_bit(TE_TIMER_KILL,(void*)&fe->fe_param.te_param.critical)){
		wan_clear_bit(
				TE_TIMER_RUNNING,
				(void*)&fe->fe_param.te_param.critical);
		return;
	}
	
	if (wan_test_bit(TE_TIMER_RUNNING,(void*)&fe->fe_param.te_param.critical)){
		if (fe->fe_param.te_param.timer_cmd == cmd){
			/* Just ignore current request */
			return;
		}
		DEBUG_TEST("%s: TE_TIMER_RUNNING: new_cmd=%X curr_cmd=%X\n",
					fe->name,
					cmd,
					fe->fe_param.te_param.timer_cmd);
		return;
	}

	wan_set_bit(TE_TIMER_RUNNING,(void*)&fe->fe_param.te_param.critical);
	
	fe->fe_param.te_param.timer_cmd=cmd;

	wan_add_timer(&fe->fe_param.te_param.timer, delay * HZ / 1000);
	return;	
}

/*
 ******************************************************************************
 *				sdla_te_polling()	
 *
 * Description:
 * Arguments:
 * Returns:
 ******************************************************************************
 */
static int sdla_te_polling(sdla_fe_t* fe)
{
	sdla_t*		card = (sdla_t*)fe->card;

	WAN_ASSERT(fe->write_fe_reg == NULL);
	WAN_ASSERT(fe->read_fe_reg == NULL);

	DEBUG_TEST("%s: TE1 Polling State=%s Cmd=0x%X!\n", 
			fe->name, fe->fe_status==FE_CONNECTED?"Con":"Disconn",
			fe->fe_param.te_param.timer_cmd);

	wan_clear_bit(TE_TIMER_RUNNING,(void*)&fe->fe_param.te_param.critical);
	
	switch(fe->fe_param.te_param.timer_cmd){
		
	case TE_LINELB_TIMER:
		
		if (IS_T1_FEMEDIA(fe)){
			
			/* Sending T1 activation/deactivation loopback signal */
			if (fe->fe_param.te_param.lb_tx_cnt > 11){
				WRITE_REG(REG_T1_XBOC_CODE, 
					(fe->fe_param.te_param.lb_tx_cmd == WAN_TE1_ACTIVATE_LB) ? 
						LINELB_ACTIVATE_CODE : LINELB_DEACTIVATE_CODE);
			}else if (fe->fe_param.te_param.lb_tx_cnt){
				WRITE_REG(REG_T1_XBOC_CODE, LINELB_DS1LINE_ALL);  
			}else{
				WRITE_REG(REG_T1_XBOC_CODE, BITS_T1_XBOC_DISABLE);  
			}	
			
			if (fe->fe_param.te_param.lb_tx_cnt--){
				sdla_te_enable_timer(fe, TE_LINELB_TIMER, LINELB_TE1_TIMER);
			}else{
				DEBUG_TE1("%s: T1 loopback %s signal sent.\n",
						fe->name,	
						(fe->fe_param.te_param.lb_tx_cmd == WAN_TE1_ACTIVATE_LB) ? 
							"activation" : "deactivation");
				fe->fe_param.te_param.lb_tx_cmd = 0x00;
				wan_clear_bit(LINELB_WAITING,(void*)&fe->fe_param.te_param.critical);
			}
		}
		break;

	case TE_SET_INTR:
		sdla_te_set_intr(fe);
		break;

	case TE_LINKDOWN_TIMER:
		sdla_te_read_alarms(fe, WAN_FE_ALARM_READ);
		sdla_te_set_status(fe, fe->fe_alarm); 
		if (fe->fe_status == FE_CONNECTED){
			sdla_te_enable_timer(fe, TE_LINKUP_TIMER, POLLING_TE1_TIMER);
		}else{
			sdla_te_enable_timer(fe, TE_LINKDOWN_TIMER, POLLING_TE1_TIMER);
		}
		break;

	case TE_LINKUP_TIMER:
		/* ALEX: 
		** Do not update protocol front end state from TE_LINKDOWN_TIMER
		** because it cause to stay longer in interrupt handler
	        ** (critical for XILINX code) */
		if (fe->fe_status == FE_CONNECTED){
			sdla_te_set_intr(fe);
			if (card->wandev.te_link_state){
				card->wandev.te_link_state(card);
			}
		}else{
			sdla_te_enable_timer(fe, TE_LINKDOWN_TIMER, POLLING_TE1_TIMER);
		}
		break;

	case TE_RBS_READ:
		/* Physically read RBS status and print */
		sdla_te_rbs_print(fe, 0);
		break;
	}
	return 0;
}

/*
 ******************************************************************************
 *				sdla_te_lb()	
 *
 * Description:
 * Arguments:
 * Returns:
 ******************************************************************************
 */
static int sdla_te_lb(sdla_fe_t* fe, unsigned char mode) 
{
	WAN_ASSERT(fe->write_fe_reg == NULL);
	WAN_ASSERT(fe->read_fe_reg == NULL);

	if (!IS_T1_FEMEDIA(fe)){
		return 1;
	}
	if (fe->fe_status != FE_CONNECTED){
		return 1;
	}
	if (wan_test_bit(TE_TIMER_RUNNING,(void*)&fe->fe_param.te_param.critical)){
		return 1;
	}
	if (wan_test_bit(LINELB_WAITING,(void*)&fe->fe_param.te_param.critical)){
		DEBUG_TE1("%s: Still waiting for far end to send loopback signal back!\n",
				fe->name);				
	}
	DEBUG_TE1("%s: Sending %s loopback %s signal...\n",
			fe->name, 
			FE_MEDIA_DECODE(fe),
			(mode == WAN_TE1_ACTIVATE_LB) ? 
				"activation" : "deactivation");
	fe->fe_param.te_param.lb_tx_cmd		= mode;
	fe->fe_param.te_param.lb_tx_cnt		= LINELB_CODE_CNT + LINELB_CHANNEL_CNT + 1;
	wan_set_bit(LINELB_WAITING,(void*)&fe->fe_param.te_param.critical);
	wan_set_bit(LINELB_CODE_BIT,(void*)&fe->fe_param.te_param.critical);
	wan_set_bit(LINELB_CHANNEL_BIT,(void*)&fe->fe_param.te_param.critical);
	sdla_te_enable_timer(fe, TE_LINELB_TIMER, LINELB_TE1_TIMER);
	return 0;
}



/*
 ******************************************************************************
 *				sdla_te_udp()	
 *
 * Description:
 * Arguments:
 * Returns:
 ******************************************************************************
 */
static int sdla_te_udp(sdla_fe_t *fe, void* p_udp_cmd, unsigned char* data)
{
	wan_cmd_t	*udp_cmd = (wan_cmd_t*)p_udp_cmd;
	int		err = 0;

	switch(udp_cmd->wan_cmd_command){
	case WAN_GET_MEDIA_TYPE:
		data[0] = (IS_T1_FEMEDIA(fe) ? WAN_MEDIA_T1 :
			   IS_E1_FEMEDIA(fe) ? WAN_MEDIA_E1 :
					    WAN_MEDIA_NONE);
		udp_cmd->wan_cmd_return_code = WAN_CMD_OK;
		udp_cmd->wan_cmd_data_len = sizeof(unsigned char); 
		break;

	case WAN_FE_SET_LB_MODE:
		/* Activate/Deactivate Line Loopback modes */
	    	err = sdla_te_set_lbmode(fe, data[0], data[1]); 
	    	udp_cmd->wan_cmd_return_code = 
				(!err) ? WAN_CMD_OK : WAN_UDP_FAILED_CMD;
	    	udp_cmd->wan_cmd_data_len = 0x00;
		break;

	case WAN_FE_GET_STAT:
 	        /* TE1_56K Read T1/E1/56K alarms */
	  	*(unsigned long *)&data[0] = fe->fe_alarm;
		/* TE1 Update T1/E1 perfomance counters */
		sdla_te_pmon(fe);
	        memcpy(&data[sizeof(unsigned long)],
			&fe->fe_pmon.u.te_pmon,
			sizeof(sdla_te_pmon_t));
	        udp_cmd->wan_cmd_return_code = WAN_CMD_OK;
	    	udp_cmd->wan_cmd_data_len = 
			sizeof(unsigned long) + sizeof(sdla_te_pmon_t); 

		break;

 	case WAN_FE_FLUSH_PMON:
		/* TE1 Flush T1/E1 pmon counters */
		sdla_te_flush_pmon(fe);
	        udp_cmd->wan_cmd_return_code = WAN_CMD_OK;
		break;
 
	case WAN_FE_GET_CFG:
		/* Read T1/E1 configuration */
	       	memcpy(&data[0],
	    		&fe->fe_cfg,
		      	sizeof(sdla_fe_cfg_t));
	    	udp_cmd->wan_cmd_return_code = WAN_CMD_OK;
    	    	udp_cmd->wan_cmd_data_len = sizeof(sdla_fe_cfg_t);
	    	break;

	case WAN_FE_SET_DEBUG_MODE:
		switch(data[0]){
		case WAN_FE_DEBUG_RBS:
			if (data[1] & WAN_FE_DEBUG_RBS_READ){
				DEBUG_EVENT("%s: Reading RBS status!\n",
					fe->name);
#if 1
				sdla_te_enable_timer(fe, TE_RBS_READ, POLLING_TE1_TIMER);
#else
				sdla_te_rbs_print(fe, 0);
#endif
	    			udp_cmd->wan_cmd_return_code = WAN_CMD_OK;
			}else if (data[1] & WAN_FE_DEBUG_RBS_RX_ENABLE){
				/* Enable extra debugging */
				DEBUG_EVENT("%s: Enable RBS RX DEBUG mode!\n",
					fe->name);
				fe->fe_debug |= WAN_FE_DEBUG_RBS_RX_ENABLE;
				sdla_te_rbs_print(fe, 1);
	    			udp_cmd->wan_cmd_return_code = WAN_CMD_OK;
			}else if (data[1] & WAN_FE_DEBUG_RBS_TX_ENABLE){
				/* Enable extra debugging */
				DEBUG_EVENT("%s: Enable RBS TX DEBUG mode!\n",
					fe->name);
				fe->fe_debug |= WAN_FE_DEBUG_RBS_TX_ENABLE;
				sdla_te_rbs_print(fe, 1);
	    			udp_cmd->wan_cmd_return_code = WAN_CMD_OK;
			}else if (data[1] & WAN_FE_DEBUG_RBS_RX_DISABLE){
				/* Disable extra debugging */
				DEBUG_EVENT("%s: Disable RBS RX DEBUG mode!\n",
					fe->name);
				fe->fe_debug &= ~WAN_FE_DEBUG_RBS_RX_ENABLE;
				sdla_te_rbs_print(fe, 1);
	    			udp_cmd->wan_cmd_return_code = WAN_CMD_OK;
			}else if (data[1] & WAN_FE_DEBUG_RBS_TX_DISABLE){
				/* Disable extra debugging */
				DEBUG_EVENT("%s: Disable RBS TX DEBUG mode!\n",
					fe->name);
				fe->fe_debug &= ~WAN_FE_DEBUG_RBS_TX_ENABLE;
				sdla_te_rbs_print(fe, 1);
	    			udp_cmd->wan_cmd_return_code = WAN_CMD_OK;
			}
			break;
		case WAN_FE_DEBUG_ALARM:
			if (data[1] & WAN_FE_DEBUG_ALARM_AIS_ENABLE){
				/* Enable TX AIS alarm */
				DEBUG_EVENT("%s: Enable TX AIS alarm!\n",
					fe->name);
				WRITE_REG(REG_E1_TRAN_TX_ALARM_CTRL,
					READ_REG(REG_E1_TRAN_TX_ALARM_CTRL) |
						BIT_E1_TRAN_TX_ALARM_AIS); 
				WRITE_REG(REG_TX_LINE_CONF,
					READ_REG(REG_TX_LINE_CONF) |
						BIT_TX_LINE_CONF_TAISEN);
	    			udp_cmd->wan_cmd_return_code = WAN_CMD_OK;
			}else if (data[1] & WAN_FE_DEBUG_ALARM_AIS_DISABLE){
				/* Disable TX AIS alarm */
				DEBUG_EVENT("%s: Disable TX AIS alarm!\n",
					fe->name);
				WRITE_REG(REG_E1_TRAN_TX_ALARM_CTRL,
					READ_REG(REG_E1_TRAN_TX_ALARM_CTRL) &
						~BIT_E1_TRAN_TX_ALARM_AIS); 
				WRITE_REG(REG_TX_LINE_CONF,
					READ_REG(REG_TX_LINE_CONF) &
						~BIT_TX_LINE_CONF_TAISEN);
	    			udp_cmd->wan_cmd_return_code = WAN_CMD_OK;
			}
			break;
		default:
			udp_cmd->wan_cmd_return_code = WAN_UDP_INVALID_CMD;
			break;
		}
    	    	udp_cmd->wan_cmd_data_len = 0;
		break;
	case WAN_FE_TX_MODE:
		switch(data[0]){
		case WAN_FE_TXMODE_ENABLE:
			DEBUG_EVENT("%s: Enable Transmitter!\n",
					fe->name);
			WRITE_REG(REG_XLPG_LINE_CFG, fe->fe_param.te_param.xlpg_scale);
			udp_cmd->wan_cmd_return_code = WAN_CMD_OK;
			break;
		case WAN_FE_TXMODE_DISABLE:
			DEBUG_EVENT("%s: Disable Transmitter (tx tri-state mode)!\n",
					fe->name);
			WRITE_REG(REG_XLPG_LINE_CFG, BIT_XLPG_LINE_CFG_HIGHZ);
			udp_cmd->wan_cmd_return_code = WAN_CMD_OK;
			break;
		}
		break;
	default:
		udp_cmd->wan_cmd_return_code = WAN_UDP_INVALID_CMD;
	    	udp_cmd->wan_cmd_data_len = 0;
		break;
	}
	return 0;
}

/*
 ******************************************************************************
 *				sdla_te_set_RBS()	
 *
 * Description:
 * Arguments:
 * Returns:
 ******************************************************************************
 */
static int 
sdla_te_set_rbsbits(sdla_fe_t *fe, int channel, unsigned char ABCD_bits)	
{
	sdla_t*		card = (sdla_t*)fe->card;
	
	if (channel > fe->fe_param.te_param.max_channels){
		DEBUG_EVENT("%s: Invalid channel number %d\n",
				fe->name, channel);
		return -EINVAL;
	}

	if (fe->fe_debug & WAN_FE_DEBUG_RBS_TX_ENABLE){
		DEBUG_EVENT("%s: %s:%-3d TX RBS A:%1d B:%1d C:%1d D:%1d\n",
				fe->name,
				FE_MEDIA_DECODE(fe),
				channel,	
				(ABCD_bits & BIT_TPSC_SIGBYTE_A) ? 1 : 0,
				(ABCD_bits & BIT_TPSC_SIGBYTE_B) ? 1 : 0,
				(ABCD_bits & BIT_TPSC_SIGBYTE_C) ? 1 : 0,
				(ABCD_bits & BIT_TPSC_SIGBYTE_D) ? 1 : 0);
	}
	if (ABCD_bits & BIT_TPSC_SIGBYTE_A){
		wan_set_bit(channel,(unsigned long*)&fe->fe_param.te_param.tx_rbs_A);
	}else{
		wan_clear_bit(channel,(unsigned long*)&fe->fe_param.te_param.tx_rbs_A);
	}
	if (ABCD_bits & BIT_TPSC_SIGBYTE_B){
		wan_set_bit(channel,(unsigned long*)&fe->fe_param.te_param.tx_rbs_B);
	}else{
		wan_clear_bit(channel,(unsigned long*)&fe->fe_param.te_param.tx_rbs_B);
	}
	if (ABCD_bits & BIT_TPSC_SIGBYTE_C){
		wan_set_bit(channel,(unsigned long*)&fe->fe_param.te_param.tx_rbs_C);
	}else{
		wan_clear_bit(channel,(unsigned long*)&fe->fe_param.te_param.tx_rbs_C);
	}
	if (ABCD_bits & BIT_TPSC_SIGBYTE_D){
		wan_set_bit(channel,(unsigned long*)&fe->fe_param.te_param.tx_rbs_D);
	}else{
		wan_clear_bit(channel,(unsigned long*)&fe->fe_param.te_param.tx_rbs_D);
	}
	if (fe->fe_param.te_param.ptr_te_Tx_sig_off){
		unsigned char	tx_sig = TE_SET_TX_SIG_BITS | ABCD_bits;
		card->hw_iface.poke(
			card->hw,
			fe->fe_param.te_param.ptr_te_Tx_sig_off + channel - 1,
			&tx_sig,
		       	sizeof(tx_sig));
	}else{
		if (IS_T1_FEMEDIA(fe)){ 
			WRITE_TPSC_REG(REG_TPSC_SIGNALING_BYTE, channel, 
				(ABCD_bits |
				 BIT_TPSC_SIGBYTE_SIGC_0 | 
				 BIT_TPSC_SIGBYTE_SIGC_1));
		}else{
			WRITE_TPSC_REG(REG_TPSC_SIGNALING_BYTE, channel, 
				(ABCD_bits |
				 BIT_TPSC_SIGBYTE_SIGSRC));
		}
	}
	return 0;
}

#if 0
static void sdla_te_abcd_update(sdla_fe_t* fe)
{
	unsigned char sigx_cfg;

	sigx_cfg = READ_REG(REG_SIGX_CFG);
	WRITE_REG(REG_SIGX_CFG, READ_REG(REG_SIGX_CFG) | BIT_SIGX_COSS);
	fe->fe_param.te_param.SIGX_chg_30_25 = 
				READ_REG(REG_SIGX_CFG) & MASK_SIGX_COSS_30_25;
	fe->fe_param.te_param.SIGX_chg_24_17 = 
				READ_REG(REG_SIGX_TIMESLOT_IND_STATUS);
	fe->fe_param.te_param.SIGX_chg_16_9  = 
				READ_REG(REG_SIGX_TIMESLOT_IND_ACCESS);
	fe->fe_param.te_param.SIGX_chg_8_1   = 
				READ_REG(REG_SIGX_TIMESLOT_IND_DATA_BUFFER);
	WRITE_REG(REG_SIGX_CFG, sigx_cfg);

	if ((IS_E1_FEMEDIA(fe) && 
		fe->fe_param.te_param.SIGX_chg_30_25) || 
	    fe->fe_param.te_param.SIGX_chg_24_17 || 
	    fe->fe_param.te_param.SIGX_chg_16_9 || 
	    fe->fe_param.te_param.SIGX_chg_8_1){
		sdla_te_enable_timer(fe, TE_ABCD_UPDATE, 10);
	}
	return;	
}
#endif


/*
 ******************************************************************************
 *				sdla_te_rbs_update()	
 *
 * Description:
 * Arguments:
 * Returns:
 ******************************************************************************
 */
static int 
sdla_te_rbs_update(sdla_fe_t* fe, int channo, unsigned char rbsbits)
{

	if (fe->fe_debug & WAN_FE_DEBUG_RBS_RX_ENABLE){
		DEBUG_EVENT("%s: %s:%-3d RX RBS A:%1d B:%1d C:%1d D:%1d\n",
						fe->name,
						FE_MEDIA_DECODE(fe),
						channo, 
						(rbsbits & BIT_SIGX_A) ? 1 : 0,
						(rbsbits & BIT_SIGX_B) ? 1 : 0,
						(rbsbits & BIT_SIGX_C) ? 1 : 0,
						(rbsbits & BIT_SIGX_D) ? 1 : 0);
	}
	
	/* Update rbs value in private structures */
	wan_set_bit(channo, &fe->fe_param.te_param.rx_rbs_status);
	fe->fe_param.te_param.rx_rbs[channo] = rbsbits;

	if (rbsbits & BIT_SIGX_A){
		wan_set_bit(channo,
			(unsigned long*)&fe->fe_param.te_param.rx_rbs_A);
	}else{
		wan_clear_bit(channo,
			(unsigned long*)&fe->fe_param.te_param.rx_rbs_A);
	}	
	if (rbsbits & BIT_SIGX_B){
		wan_set_bit(channo,
			(unsigned long*)&fe->fe_param.te_param.rx_rbs_B);
	}else{
		wan_clear_bit(channo,
			(unsigned long*)&fe->fe_param.te_param.rx_rbs_B);
	}
	if (rbsbits & BIT_SIGX_C){
		wan_set_bit(channo,
			(unsigned long*)&fe->fe_param.te_param.rx_rbs_C);
	}else{
		wan_clear_bit(channo,
			(unsigned long*)&fe->fe_param.te_param.rx_rbs_C);
	}
	if (rbsbits & BIT_SIGX_D){
		wan_set_bit(channo,
			(unsigned long*)&fe->fe_param.te_param.rx_rbs_D);
	}else{
		wan_clear_bit(channo,
			(unsigned long*)&fe->fe_param.te_param.rx_rbs_D);
	}

	return 0;
}

/*
 ******************************************************************************
 *				sdla_te_rbs_report()	
 *
 * Description:
 * Arguments:
 * Returns:
 ******************************************************************************
 */
static int 
sdla_te_rbs_report(sdla_fe_t* fe)
{
	sdla_t*		card = (sdla_t*)fe->card;
	int		ch = 1, max_channels;
	
	max_channels = fe->fe_param.te_param.max_channels;
	ch = (IS_E1_FEMEDIA(fe)) ? 0 : 1;
	for(; ch <= max_channels; ch++) {
		if (wan_test_bit(ch, &fe->fe_param.te_param.rx_rbs_status)){
			if (card->wandev.te_report_rbsbits){
				card->wandev.te_report_rbsbits(
					card, 
					ch, 
					fe->fe_param.te_param.rx_rbs[ch]);
			}
			wan_clear_bit(ch, &fe->fe_param.te_param.rx_rbs_status);
		}
	}
	return 0;
}

/*
 ******************************************************************************
 *				sdla_te_check_rbsbits()	
 *
 * Description:
 * Arguments:
 * Returns:
 ******************************************************************************
 */
static int 
sdla_te_check_rbsbits(sdla_fe_t* fe, int ch_base, unsigned long ts_map, int report)
{
	unsigned char	status = 0x0, rbsbits = 0x00;
	unsigned char	sigx_cfg = 0x00;
	int		i = 0, channel;

	sigx_cfg = READ_REG(REG_SIGX_CFG);
	WRITE_REG(REG_SIGX_CFG, sigx_cfg | BIT_SIGX_COSS);
	switch(ch_base){
	case 1:
		status = READ_REG(REG_SIGX_CHG_8_1);
		break;
	case 9:
		status = READ_REG(REG_SIGX_CHG_16_9);
		break;
	case 17:
		status = READ_REG(REG_SIGX_CHG_24_17);
		break;
	case 25:
		status = READ_REG(REG_SIGX_CHG_30_25) & MASK_SIGX_COSS_30_25;
		break;
	}
	WRITE_REG(REG_SIGX_CFG, sigx_cfg);
	
	if (status == 0x00){
		return 0;
	}

	for(i = 0; i < 8; i ++) {
		channel = ch_base + i;
		if (IS_E1_FEMEDIA(fe)){
			if (channel >= 16){
				channel++;
			}
			if (channel > 31){
				continue;
			}
		}
		/* If this channel/timeslot is not selected, move to 
		 * another channel/timeslot */
		if (!wan_test_bit(channel-1, &ts_map)){
			continue;
		}
		if(status & ((unsigned char)(0x01 << i))) {
			rbsbits = READ_SIGX_REG(REG_SIGX_CURRENT, channel);
			if (channel > 16){
				rbsbits &= 0x0F;
			}else{
				rbsbits = (rbsbits >> 4) & 0x0F;
			}
		
			sdla_te_rbs_update(fe, channel, rbsbits);
		}
	}	
	return 0; 
}

/*
 ******************************************************************************
 *				sdla_te_read_rbsbits()	
 *
 * Description:
 * Arguments:
 * Returns:
 ******************************************************************************
 */
static unsigned char 
sdla_te_read_rbsbits(sdla_fe_t* fe, int channo, int report)
{
	sdla_t*		card = (sdla_t*)fe->card;
	unsigned char	rbsbits = 0x00;

	rbsbits = READ_SIGX_REG(REG_SIGX_CURRENT, channo);
	if (channo <= 16){
		rbsbits = (rbsbits >> 4) & 0x0F;
	}else{
		rbsbits &= 0xF;
	}
	
	sdla_te_rbs_update(fe, channo, rbsbits);

	if (report && card->wandev.te_report_rbsbits){
		card->wandev.te_report_rbsbits(
					card, 
					channo, 
					rbsbits);
	}
	return rbsbits;
}

/*
 ******************************************************************************
 *				sdla_te_rbs_print_banner()	
 *
 * Description:
 * Arguments:
 * Returns:
 ******************************************************************************
 */
static int 
sdla_te_rbs_print_banner(sdla_fe_t* fe)
{
	if (IS_T1_FEMEDIA(fe)){
		DEBUG_EVENT("%s:                111111111122222\n",
					fe->name);
		DEBUG_EVENT("%s:       123456789012345678901234\n",
					fe->name);
		DEBUG_EVENT("%s:       ------------------------\n",
					fe->name);
	}else{
		DEBUG_EVENT("%s:                11111111112222222222333\n",
					fe->name);
		DEBUG_EVENT("%s:       12345678901234567890123456789012\n",
					fe->name);
		DEBUG_EVENT("%s:       --------------------------------\n",
					fe->name);
	}	
	return 0;
}

/*
 ******************************************************************************
 *				sdla_te_rbs_print_bits()	
 *
 * Description:
 * Arguments:
 * Returns:
 ******************************************************************************
 */
static int 
sdla_te_rbs_print_bits(sdla_fe_t* fe, unsigned long bits, char *msg)
{
	int 	i, max_channels = fe->fe_param.te_param.max_channels;
	int	start_chan = 1;

	if (IS_E1_FEMEDIA(fe)){
		start_chan = 0;
	}
	_DEBUG_EVENT("%s: %s ", fe->name, msg);
	for(i=start_chan; i <= max_channels; i++)
		_DEBUG_EVENT("%01d", 
			wan_test_bit(i, &bits) ? 1 : 0);
	_DEBUG_EVENT("\n");
	return 0;
}

/*
 ******************************************************************************
 *				sdla_te_rbs_print()	
 *
 * Description:
 * Arguments:
 * Returns:
 ******************************************************************************
 */
static int 
sdla_te_rbs_print(sdla_fe_t* fe, int last_status)
{
	unsigned long	rx_a = 0x00;
	unsigned long	rx_b = 0x00;
	unsigned long	rx_c = 0x00;
	unsigned long	rx_d = 0x00;
	
	if (last_status){
		rx_a = fe->fe_param.te_param.rx_rbs_A;	
		rx_b = fe->fe_param.te_param.rx_rbs_B;
		rx_c = fe->fe_param.te_param.rx_rbs_C;
		rx_d = fe->fe_param.te_param.rx_rbs_D;
		
		DEBUG_EVENT("%s: Last Status:\n",
					fe->name);
		sdla_te_rbs_print_banner(fe);
		sdla_te_rbs_print_bits(fe, fe->fe_param.te_param.tx_rbs_A, "TX A:");
		sdla_te_rbs_print_bits(fe, fe->fe_param.te_param.tx_rbs_B, "TX B:");
		sdla_te_rbs_print_bits(fe, fe->fe_param.te_param.tx_rbs_C, "TX C:");
		sdla_te_rbs_print_bits(fe, fe->fe_param.te_param.tx_rbs_D, "TX D:");
		DEBUG_EVENT("%s:\n", fe->name);
	}else{
		int		i, channo = 0;
		unsigned char	rbsbits = 0x00;
		for(i = 0; i < fe->fe_param.te_param.max_channels; i++) {

			channo = (IS_T1_FEMEDIA(fe)) ? i+1 : i;
			rbsbits = READ_SIGX_REG(REG_SIGX_CURRENT, channo);
			DEBUG_TE1("%s: Update status for ch %d\n",
						fe->name,
						channo);
			if (rbsbits & BIT_SIGX_CURRENT_A_N){
				wan_set_bit(channo, &rx_a);
			}
			if (rbsbits & BIT_SIGX_CURRENT_B_N){
				wan_set_bit(channo, &rx_b);
			}
			if (rbsbits & BIT_SIGX_CURRENT_C_N){
				wan_set_bit(channo, &rx_c);
			}
			if (rbsbits & BIT_SIGX_CURRENT_D_N){
				wan_set_bit(channo, &rx_d);
			}
			if ((IS_E1_FEMEDIA(fe) && (channo < 16)) ||
			    (IS_T1_FEMEDIA(fe) && (channo <= 8))){
				DEBUG_TE1("%s: Update status for ch %d\n",
						fe->name, channo+16);
				if (rbsbits & BIT_SIGX_CURRENT_A_N16){
					wan_set_bit(channo+16, &rx_a);
				}
				if (rbsbits & BIT_SIGX_CURRENT_B_N16){
					wan_set_bit(channo+16, &rx_b);
				}
				if (rbsbits & BIT_SIGX_CURRENT_C_N16){
					wan_set_bit(channo+16, &rx_c);
				}
				if (rbsbits & BIT_SIGX_CURRENT_D_N16){
					wan_set_bit(channo+16, &rx_d);
				}				
			}
			if (i >= 15){
				break;
			}
		}
		DEBUG_EVENT("%s: Current Status:\n",
					fe->name);
		sdla_te_rbs_print_banner(fe);
	}

	sdla_te_rbs_print_bits(fe, rx_a, "RX A:");
	sdla_te_rbs_print_bits(fe, rx_b, "RX B:");
	sdla_te_rbs_print_bits(fe, rx_c, "RX C:");
	sdla_te_rbs_print_bits(fe, rx_d, "RX D:");
	return 0;
}


#if 0
static int 
sdla_te_check_rbsbits(sdla_fe_t* fe, unsigned char sig_chg, int channel_base, int report)
{
	sdla_t*		card = (sdla_t*)fe->card;
	int		SIGX_addr = 0;
	unsigned char	status = 0x0;
	int		i = 0;

	if (fe->fe_status == FE_CONNECTED){
#if 0
		DEBUG_EVENT("Read RBS: sig_chg: 0x%02X, ch base: %d\n",
				sig_chg, channel_base);
#endif
	}

	if (IS_T1_FEMEDIA(fe)){
		SIGX_addr = (channel_base == 9) ? 0x18 : 0x10; 
	}	
	
	for(i = 0; i < 8; i ++) {
		if(sig_chg & ((unsigned char)(0x01 << i))) {
			status = READ_SIGX_REG(SIGX_addr, i+1);
			if (fe->fe_status != FE_CONNECTED){
				return -EINVAL;
			}
#if 0
			DEBUG_EVENT("%s: SIGX bit %d set, value 0x%02X read from reg 0x%02X\n",
			       		fe->name, i, status, SIGX_addr + i);
#endif
			if (channel_base == 17){
				status &= 0x0F;
			}else{
				status = (status >> 4) & 0x0F;
			}
			
			DEBUG_TE1("%s: %s:%d RX RBS A:%1d B:%1d C:%1d D:%1d\n",
					fe->name,
					FE_MEDIA_DECODE(fe),
					channel_base + i, 
					(status & BIT_SIGX_A) ? 1 : 0,
					(status & BIT_SIGX_B) ? 1 : 0,
					(status & BIT_SIGX_C) ? 1 : 0,
					(status & BIT_SIGX_D) ? 1 : 0);

			if (report && card->wandev.te_report_rbsbits){
				card->wandev.te_report_rbsbits(
							card, 
							channel_base+i, 
							status);
			}
		}
	}	
	return 0; 
}
#endif

#if !defined(CONFIG_PRODUCT_WANPIPE_GENERIC)
/*
 ******************************************************************************
 *					sdla_te_get_snmp_data()	
 *
 * Description:
 * Arguments:
 * Returns:
 ******************************************************************************
 */
#define   DSX1IFINDEX           4
#define   DSX1TIMEELAPSED       5
#define   DSX1VALIDINTERVALS    6
#define   DSX1LINETYPE          7
#define   DSX1LINECODING        8
#define   DSX1SENDCODE          9
#define   DSX1CIRCUITIDENTIFIER  10
#define   DSX1LOOPBACKCONFIG    11
#define   DSX1LINESTATUS        12
#define   DSX1SIGNALMODE        13
#define   DSX1TRANSMITCLOCKSOURCE  14
#define   DSX1FDL               15
#define   DSX1INVALIDINTERVALS  16
#define   DSX1LINELENGTH        17
#define   DSX1LINESTATUSLASTCHANGE  18
#define   DSX1LINESTATUSCHANGETRAPENABLE  19
#define   DSX1LOOPBACKSTATUS    20
#define   DSX1DS1CHANNELNUMBER  21
#define   DSX1CHANNELIZATION    22
#define   DSX1CURRENTINDEX      25
#define   DSX1CURRENTESS        26
#define   DSX1CURRENTSESS       27
#define   DSX1CURRENTSEFSS      28
#define   DSX1CURRENTUASS       29
#define   DSX1CURRENTCSSS       30
#define   DSX1CURRENTPCVS       31
#define   DSX1CURRENTLESS       32
#define   DSX1CURRENTBESS       33
#define   DSX1CURRENTDMS        34
#define   DSX1CURRENTLCVS       35
#define   DSX1INTERVALINDEX     38
#define   DSX1INTERVALNUMBER    39
#define   DSX1INTERVALESS       40
#define   DSX1INTERVALSESS      41
#define   DSX1INTERVALSEFSS     42
#define   DSX1INTERVALUASS      43
#define   DSX1INTERVALCSSS      44
#define   DSX1INTERVALPCVS      45
#define   DSX1INTERVALLESS      46
#define   DSX1INTERVALBESS      47
#define   DSX1INTERVALDMS       48
#define   DSX1INTERVALLCVS      49
#define   DSX1INTERVALVALIDDATA  50
#define   DSX1TOTALINDEX        53
#define   DSX1TOTALESS          54
#define   DSX1TOTALSESS         55
#define   DSX1TOTALSEFSS        56
#define   DSX1TOTALUASS         57
#define   DSX1TOTALCSSS         58
#define   DSX1TOTALPCVS         59
#define   DSX1TOTALLESS         60
#define   DSX1TOTALBESS         61
#define   DSX1TOTALDMS          62
#define   DSX1TOTALLCVS         63
#define   DSX1FARENDCURRENTINDEX  66
#define   DSX1FARENDTIMEELAPSED  67
#define   DSX1FARENDVALIDINTERVALS  68
#define   DSX1FARENDCURRENTESS  69
#define   DSX1FARENDCURRENTSESS  70
#define   DSX1FARENDCURRENTSEFSS  71
#define   DSX1FARENDCURRENTUASS  72
#define   DSX1FARENDCURRENTCSSS  73
#define   DSX1FARENDCURRENTLESS  74
#define   DSX1FARENDCURRENTPCVS  75
#define   DSX1FARENDCURRENTBESS  76
#define   DSX1FARENDCURRENTDMS  77
#define   DSX1FARENDINVALIDINTERVALS  78
#define   DSX1FARENDINTERVALINDEX  81
#define   DSX1FARENDINTERVALNUMBER  82
#define   DSX1FARENDINTERVALESS  83
#define   DSX1FARENDINTERVALSESS  84
#define   DSX1FARENDINTERVALSEFSS  85
#define   DSX1FARENDINTERVALUASS  86
#define   DSX1FARENDINTERVALCSSS  87
#define   DSX1FARENDINTERVALLESS  88
#define   DSX1FARENDINTERVALPCVS  89
#define   DSX1FARENDINTERVALBESS  90
#define   DSX1FARENDINTERVALDMS  91
#define   DSX1FARENDINTERVALVALIDDATA  92
#define   DSX1FARENDTOTALINDEX  95
#define   DSX1FARENDTOTALESS    96
#define   DSX1FARENDTOTALSESS   97
#define   DSX1FARENDTOTALSEFSS  98
#define   DSX1FARENDTOTALUASS   99
#define   DSX1FARENDTOTALCSSS   100
#define   DSX1FARENDTOTALLESS   101
#define   DSX1FARENDTOTALPCVS   102
#define   DSX1FARENDTOTALBESS   103
#define   DSX1FARENDTOTALDMS    104
#define   DSX1FRACINDEX         107
#define   DSX1FRACNUMBER        108
#define   DSX1FRACIFINDEX       109
#define   DSX1CHANMAPPEDIFINDEX  112

static int sdla_te_get_snmp_data(sdla_fe_t* fe, void* pdev, void* data)
{
#if 0
/* FIXME: NC dev not used
 * ALEX please confirm
 */
	netdevice_t* 	dev = (netdevice_t *)dev_ptr;
#endif
	wanpipe_snmp_t*	snmp;
	sdla_te_pmon_t*	pmon = &fe->fe_pmon.u.te_pmon;
	unsigned long	alarms = 0;

	snmp = (wanpipe_snmp_t*)data;

	switch(snmp->snmp_magic){
	case DSX1LINESTATUS:
		alarms = fe->fe_alarm; 
		snmp->snmp_val = 0;
		if (alarms & WAN_TE_BIT_YEL_ALARM){
			snmp->snmp_val |= SNMP_DSX1_RCVFARENDLOF;
		}
		if (alarms & WAN_TE_BIT_AIS_ALARM){
			snmp->snmp_val |= SNMP_DSX1_RCVAIS;
		}
		if (alarms & WAN_TE_BIT_RED_ALARM){
			snmp->snmp_val |= SNMP_DSX1_LOSSOFFRAME;
		}
		if (alarms & WAN_TE_BIT_LOS_ALARM){
			snmp->snmp_val |= SNMP_DSX1_LOSSOFSIGNAL;
		}

		if (!snmp->snmp_val){
			snmp->snmp_val = SNMP_DSX1_NOALARM;
		}
		break;

	case DSX1CURRENTLCVS:
	case DSX1INTERVALLCVS:
	case DSX1TOTALLCVS:
		/* Current measurement in a current 15 minutes interval */
		/* FIXME: Currently, I return total number of LCV */
		sdla_te_pmon(fe);
		snmp->snmp_val = pmon->lcv;
		break;

	case DSX1CURRENTSEFSS:
	case DSX1INTERVALSEFSS:
	case DSX1TOTALSEFSS:
		/* Second with one or more Out of Frame defects */
		/* FIXME: If we got OOF errors, return 1 */
		sdla_te_pmon(fe);
		if (pmon->oof_errors){
			snmp->snmp_val = 1;
		}
		break;

	case DSX1CURRENTPCVS:
	case DSX1INTERVALPCVS:
	case DSX1TOTALPCVS:
		/* Second with one or more CRC or bit errors defects */
		/* FIXME: If we got OOF errors, return 1 */
		sdla_te_pmon(fe);
		if (pmon->bit_errors){
			snmp->snmp_val = 1;
		}
		break;

		break;
	}
	return 0;
}
#endif

static int sdla_te_led_ctrl(sdla_fe_t *fe, int mode)
{
	unsigned char led;

	if (!fe->read_fe_reg || !fe->write_fe_reg){
		return -EINVAL;
	}

	led= READ_REG(REG_GLOBAL_CFG);

	if (mode == AFT_LED_ON){
		led&=~(BIT_GLOBAL_PIO);
	}else if (mode == AFT_LED_OFF){
		led|=BIT_GLOBAL_PIO;
	}else{
		if (led&BIT_GLOBAL_PIO){
			led&=~(BIT_GLOBAL_PIO);
		}else{
			led|=BIT_GLOBAL_PIO;
		}
	}

	WRITE_REG(REG_GLOBAL_CFG,led);
	return 0;
}

static int 
sdla_te_update_alarm_info(sdla_fe_t* fe, struct seq_file* m, int* stop_cnt)
{

#if !defined(__WINDOWS__)
	PROC_ADD_LINE(m,
		"=============================== %s Alarms ===============================\n",
		FE_MEDIA_DECODE(fe));
	PROC_ADD_LINE(m,
		PROC_STATS_ALARM_FORMAT,
		"ALOS", WAN_TE_ALOS_ALARM(fe->fe_alarm), 
		"LOS", WAN_TE_LOS_ALARM(fe->fe_alarm));
	PROC_ADD_LINE(m,
		PROC_STATS_ALARM_FORMAT,
		"RED", WAN_TE_RED_ALARM(fe->fe_alarm), 
		"AIS", WAN_TE_AIS_ALARM(fe->fe_alarm));
	if (IS_T1_FEMEDIA(fe)){
		PROC_ADD_LINE(m,
			 PROC_STATS_ALARM_FORMAT,
			 "YEL", WAN_TE_YEL_ALARM(fe->fe_alarm),
			 "OOF", WAN_TE_OOF_ALARM(fe->fe_alarm));
	}else{ 
		PROC_ADD_LINE(m,
			PROC_STATS_ALARM_FORMAT,
			"OOF", WAN_TE_OOF_ALARM(fe->fe_alarm), 
			"", "");
	}
	return m->count;	
#endif
	return 0;
}

static int 
sdla_te_update_pmon_info(sdla_fe_t* fe, struct seq_file* m, int* stop_cnt)
{
		
#if !defined(__WINDOWS__)
	PROC_ADD_LINE(m,
		 "=========================== %s PMON counters ============================\n",
		 FE_MEDIA_DECODE(fe));
	PROC_ADD_LINE(m,
		PROC_STATS_PMON_FORMAT,
		"Framing Bit Error", fe->fe_pmon.u.te_pmon.frm_bit_error,
		"Line Code Violation", fe->fe_pmon.u.te_pmon.lcv);
	if (IS_T1_FEMEDIA(fe)){
		PROC_ADD_LINE(m,
			PROC_STATS_PMON_FORMAT,
			"Out of Frame Errors", fe->fe_pmon.u.te_pmon.bit_errors,
			"Bit Errors", fe->fe_pmon.u.te_pmon.bit_errors);
	}else{ 
		PROC_ADD_LINE(m,
			PROC_STATS_PMON_FORMAT,
			"Far End Block Errors", fe->fe_pmon.u.te_pmon.far_end_blk_errors,
			"CRC Errors", fe->fe_pmon.u.te_pmon.crc_errors);
	}
	return m->count;
#endif
	return 0;
}
/*
 ******************************************************************************
 *						ClearTemplate()	
 *
 * Description:
 * Arguments:
 * Returns:
 ******************************************************************************
 */
/************************** End of files *************************************/
