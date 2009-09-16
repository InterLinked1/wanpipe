%define KERNEL_VERSION    %{?kern_ver}
%define WANPIPE_VER	  wanpipe-util
%define name              %{WANPIPE_VER}
%define version           3.4.5
%define release           0
%define	serial	 	  1
%define ETC_DIR 	  /etc
%define USR_DIR 	  /usr
%define UTILS_DIR 	  /usr/sbin
%define PROD_HOME  	  /etc/wanpipe
%define WANCFG_LIBS_DIR   /etc/wanpipe/wancfg/lib
%define API_DIR           /etc/wanpipe/api
%define DOCS_DIR	  /usr/share/doc/wanpipe
%define KERNEL_VERSION	  /
%define PROD		  wanrouter
%define META_CONF         %{PROD_HOME}/%{PROD}.rc
%define WAN_INTR_DIR      %{PROD_HOME}/interfaces
%define WAN_CONF_DIR      %{PROD_HOME}
%define PROD_CONF         %{WAN_CONF_DIR}/wanpipe1.conf
%define LIBSANGOMA_CONF   /etc/ld.so.conf.d/libsangoma.so.conf
#%define START_SCRIPT      S07%{PROD}
#%define OLD_START         S07router
#%define STOP_SCRIPT       K10%{PROD}
#%define OLD_STOP          K10router
%define ROUTER_RC         %{META_CONF}
%define WANROUTER_STARTUP_SMPL    %{PROD_HOME}/samples/wanrouter
%define WANROUTER_STARTUP         /usr/sbin/wanrouter
%define NEW_IF_TYPE               NO
%define PROD_INIT                 /usr/sbin/
 

Summary: 	Sangoma WANPIPE package for Linux. It contains WANPIPE configuration/startup/debugging utilities for Linux. This package requires the wanpipe-mod package!
Name: 		%{name}
Version: 	%{version}
Release: 	%{release}
License: 	GPL
Group: 		Applications/Communications
#Source0:	%{WANPIPE_VER}.tgz
#Source1:	bridge-utils-0.9.1.tar.gz
Vendor:		Sangoma Technologies Inc.
Url:		www.sangoma.com
Group:		Networking/WAN
 

%description 
Linux Utilities for Sangoma AFT-Series of cards and S-Series of Cards. Wanpipe supports the following protocols, TDM Voice, Frame Relay, X25(API), PPP,Multi-link PPP, CHDLC and custom API development for WAN and Voice.

Install Wanpipe-modules package for wanpipe drivers.


%prep

%build

%install

%clean

%postun

echo "Uninstalling WANPIPE..."

# ----------------------------------------------------------------------------
# Remove initialization scripts.
# ----------------------------------------------------------------------------
remove_init()
{
        chkconfig --del wanrouter
        rm /etc/init.d/wanrouter
}

remove_init_old()
{

        # Examine system bootstrap files.
        if [ -d /etc/rc0.d ]
        then RC_DIR=/etc
        elif [ -d /etc/rc.d/rc0.d ]
        then RC_DIR=/etc/rc.d
        else return 0
        fi
 
	echo "Removing start-up scripts..."
        rm -f $RC_DIR/rc2.d/%{START_SCRIPT}
	rm -f $RC_DIR/rc3.d/%{START_SCRIPT}
        rm -f $RC_DIR/rc4.d/%{START_SCRIPT}
        rm -f $RC_DIR/rc5.d/%{START_SCRIPT}

        rm -f $RC_DIR/rc0.d/%{STOP_SCRIPT}
        rm -f $RC_DIR/rc1.d/%{STOP_SCRIPT}
        rm -f $RC_DIR/rc6.d/%{STOP_SCRIPT}

        rm -f $RC_DIR/init.d/%{PROD}
        return 0
}

#remove start-on-boot scripts
remove_init;

%post

# ----------------------------------------------------------------------------
# Create meta-configuration file.
# ----------------------------------------------------------------------------
create_metaconf()
{
        local response
 
        # Select directory for the log file.
        if      [ -d /var/log ]; then
                LOG_FILE=/var/log/%{PROD}
        elif    [ -d /var/adm wanpipe1]; then
                LOG_FILE=/var/adm/%{PROD}
        else
                LOG_FILE=%{PROD_HOME}/%{PROD}.log
        fi
 
	# Select directory for the lock file.
        if      [ -d /var/lock/subsys ]; then
                LOCK_FILE=/var/lock/subsys/%{PROD}
        elif    [ -d /var/lock ]; then
                LOCK_FILE=/var/lock/%{PROD}
        else
                LOCK_FILE=$PROD_HOME/%{PROD}.lck
        fi

	
        cat > %{META_CONF} << ENDOFTEXT
#!/bin/sh
# wanrouter.rc	WAN router meta-configuration file.
#
#		This file defines variables used by the router shell scripts
#		and should be located in /etc/wanpipe directory.  These are:
#
#               ROUTER_BOOT	=       Boot flag (YES/NO).
#               WAN_CONF_DIR	=       Where to put wanpipe config files.
#		WAN_INTR_DIR	=	Where to put wanpipe interface files.
#               WAN_LOG		=       Where to put start-up log file.
#               WAN_LOCK	=       File used as a lock.
#		WAN_LOCK_DIR	=
#		WAN_IP_FORWARD	=	Enable IP Forwarding on startup.
#               WAN_DEVICES	=       Name of the wanpipe devices to be
#                                       loaded on 'wanrouter start'
#                                       (ex: "wanpipe1 wanpipe2 wanpipe3...")
#
#                              Note:    Name of wanpipe devices correspond
#                                       to the configuration files in
#                                       WANPIPE_CONF_DIR directory:
#                                         (ex. /etc/wanpipe/wanpipe1.conf )
#
#               Note:   This file is 'executed' by the shell script, so
#                       the usual shell syntax must be observed. 
ENDOFTEXT
 	echo "ROUTER_BOOT=YES"          >> %{META_CONF}
        echo "WAN_CONF_DIR=%{WAN_CONF_DIR}" >> %{META_CONF}
        echo "WAN_INTR_DIR=%{WAN_INTR_DIR}" >> %{META_CONF}
        echo "WAN_LOG=$LOG_FILE"     >> %{META_CONF}
        echo "WAN_LOCK=$LOCK_FILE"   >> %{META_CONF}
	echo "WAN_LOCK_DIR=/var/lock/subsys"    >> %{META_CONF}
	echo "WAN_IP_FORWARD=NO" >> %{META_CONF}           
	echo "NEW_IF_TYPE=NO"    >> %{META_CONF}
	echo "WAN_LIB_DIR=/etc/wanpipe/lib"    >> %{META_CONF}
	echo "WAN_ADSL_LIST=/etc/wanpipe/wan_adsl.list"    >> %{META_CONF}
	echo "WAN_ANNEXG_LOAD=NO"    >> %{META_CONF}
	echo "WAN_LIP_LOAD=YES"    >> %{META_CONF}
	echo "WAN_DYN_WANCONFIG=NO"    >> %{META_CONF}
	echo "WAN_SCRIPTS_DIR=/etc/wanpipe/scripts"    >> %{META_CONF}
	echo "WAN_FIRMWARE_DIR=/etc/wanpipe/firmware"    >> %{META_CONF}
	echo "WAN_DEVICES_REV_STOP_ORDER=YES"    >> %{META_CONF}
	echo "WAN_DEVICES=\"wanpipe1\""    >> %{META_CONF}
 
        return 0
}


# ----------------------------------------------------------------------------
# Install initialization scripts.
# ----------------------------------------------------------------------------
install_init()
{
        ln -s /usr/sbin/wanrouter /etc/init.d/wanrouter
        chkconfig --add wanrouter
	chkconfig wanrouter on
}

# ----------------------------------------------------------------------------
# Enable MGD and BRI  log for A500-BRI
# ----------------------------------------------------------------------------
enable_smg_log()
{
if [ -e  /etc/syslog.conf ]; then
        eval "grep "local2.*sangoma_mgd" /etc/syslog.conf" > /dev/null 2> /dev/null
        if [ $? -ne 0 ]; then
                eval "grep "local2" /etc/syslog.conf " > /dev/null 2> /dev/null
                if [ $? -eq 0 ]; then
                        echo
                        echo "Warning : local2 is already used in syslog.conf"
                        echo
                fi
                echo -e "\n# Sangoma Media Gateway log" > tmp.$$
                echo -e "local2.*                /var/log/sangoma_mgd.log\n" >> tmp.$$
                eval "cat /etc/syslog.conf tmp.$$ > tmp1.$$"
                \cp -f tmp1.$$ /etc/syslog.conf
                eval "/etc/init.d/syslog restart" > /dev/null 2>/dev/null
        fi
        eval "grep "local3.*sangoma_bri" /etc/syslog.conf" > /dev/null 2> /dev/null
        if [ $? -ne 0 ]; then
                eval "grep "local3" /etc/syslog.conf " > /dev/null 2> /dev/null
                if [ $? -eq 0 ]; then
                        echo
                        echo "Warning : local3 is already used in syslog.conf"
                        echo
                fi
                echo -e "\n# Sangoma BRI Daemon (smg_bri)  log" > tmp.$$
                echo -e "local3.*                /var/log/sangoma_bri.log\n" >> tmp.$$
                eval "cat /etc/syslog.conf tmp.$$ > tmp1.$$"
                \cp -f tmp1.$$ /etc/syslog.conf
                eval "/etc/init.d/syslog restart" > /dev/null 2> /dev/null
        fi
else
        echo "Warning: /etc/syslog.conf not found"
fi

if [ -f tmp1.$$ ]; then
        rm -f  tmp1.$$
fi
if [ -f tmp.$$ ]; then
        rm -f  tmp.$$
fi

echo "Ok"
echo

echo "Checking logrotate ..."
eval "type logrotate" > /dev/null 2> /dev/null
if [ $? -ne 0 ]; then
        echo "Error: Logrotate not found !"
fi

if [ -e /etc/logrotate.d ] && [ -e /etc/logrotate.d/syslog ]; then

        eval "grep sangoma_mgd /etc/logrotate.d/syslog" > /dev/null 2> /dev/null
        if [ $? -ne 0 ]; then
                eval "sed -e 's/messages/messages \/var\/log\/sangoma_mgd.log/' /etc/logrotate.d/syslog >tmp2.$$ 2>/dev/null"
                eval "cp -f tmp2.$$ /etc/logrotate.d/syslog"
                eval "logrotate -f /etc/logrotate.d/syslog"
                if [ $? -ne 0 ]; then
                        echo "Error: logrotate restart failed!";
                        exit 1;
                fi
                echo "Logrotate is being changed and restarted!"
        else
                echo "Logrotate is configured!"
        fi

else
        echo "Error: Logrotate dir: /etc/logrotate.d not found !"
fi
echo "OK."
echo

}



install_init_old()
{
	#Examine system bootstrap files.
        if [ -d /etc/rc0.d ]
        then RC_DIR=/etc
        elif [ -d /etc/rc.d/rc0.d ]
        then RC_DIR=/etc/rc.d
        else return 0
        fi
 
	PROD_INIT=%{PROD_INIT}%{PROD}

	# Install start scripts.
        [ -d $RC_DIR/rc2.d ] && ln -sf $PROD_INIT $RC_DIR/rc2.d/%{START_SCRIPT}
        [ -d $RC_DIR/rc3.d ] && ln -sf $PROD_INIT $RC_DIR/rc3.d/%{START_SCRIPT}
        [ -d $RC_DIR/rc5.d ] && ln -sf $PROD_INIT $RC_DIR/rc4.d/%{START_SCRIPT}
        [ -d $RC_DIR/rc5.d ] && ln -sf $PROD_INIT $RC_DIR/rc5.d/%{START_SCRIPT}
 
        # Install stop scripts.
        [ -d $RC_DIR/rc0.d ] && ln -sf $PROD_INIT $RC_DIR/rc0.d/%{STOP_SCRIPT}
        [ -d $RC_DIR/rc1.d ] && ln -sf $PROD_INIT $RC_DIR/rc1.d/%{STOP_SCRIPT}
        [ -d $RC_DIR/rc6.d ] && ln -sf $PROD_INIT $RC_DIR/rc6.d/%{STOP_SCRIPT}
        [ -d $RC_DIR/init.d ] && ln -sf $PROD_INIT $RC_DIR/init.d/%{PROD}
 
        return 0
}

if [ -d "/usr/local/wanrouter" ]; then
	cat <<EOM
*** Previous installation of Wanpipe detected.
    Please use /usr/sbin/wancfg instead of /usr/local/wanrouter/wancfg
    for Wanpipe configuration.
    The new configuration files will be saved in /etc/wanpipe
    and /etc/wanpipe/interfaces directories.

EOM
else
		echo 'no old wanpipe detected' > /dev/null

fi

cat <<EOM
*** Sangoma Wanpipe was successfully installed.
    Run wancfg command to configure wanpipe.
    Refer to %{DOCS_DIR} for documentation.

EOM

#install start-on-boot scripts
install_init;
#create wanrouter.rc in /etc/wanpipe
#create_metaconf;
enable_smg_log;


%files
%{ETC_DIR}
%{USR_DIR}
%{UTILS_DIR}
%{PROD_HOME}
%{DOCS_DIR}


%changelog

* Tue Aug 18 2009 Nenad Corbic <ncorbic@sangoma.com> - Stable - 3.4.5
=====================================================================

- New firmawre feature for A101/2/5/8: Free Run Timer Interrupt 
  The AFT T1/E1 cards will now provide perfect timing to zatpel/dahdi
  even when the ports are not connected. The free run interrupt
  will be enabled when all zaptel/dahdi ports are down, or on
  inital card start. To test this feature just start a wanpipe 
  port with zaptel/dahdi and run zttest. 
  A108 firmare V38 
  A104/2/1/ firmware V36       

- Added module inc cound when zaptel/dahdi starts.
  So wanpipe drivers do not crash if one tries to unload 
  zaptel/dahdi before stopping wanpipe drivers.

- Dahdi 2.2 Support
- Updated for 2.6.30 kernel


* Fri Jun 17 2009 Nenad Corbic <ncorbic@sangoma.com> - Stable - 3.4.4
=====================================================================

- Latest smg update in 3.4.3 broke BRI support
- This is now fixed.


* Fri Jun 17 2009 Nenad Corbic <ncorbic@sangoma.com> - Stable - 3.4.3
=====================================================================

- New firmware 
  2007-07-10: A108D Latest Firmware Release........ v37 . More info
  2007-07-10: A104D Latest Firmware Release ....... v35 . More info
  2007-07-10: A102D Latest Firmware Release ....... v35 . More info
  2007-07-10: A101D Latest Firmware Release. ...... v34 . More info
  2007-07-10: A200 Latest Firmware Release ........ v11 . More info
  2007-07-10: A400 Latest Firmware Release ........ v11 . More info
  2007-07-10: A104D Latest Firmware Release........ v26 . More info
  2007-07-10: A301-SH Latest Firmware Release...... v09 . More info
  2008-10-28: A14X Latest Firmware Release......... v07 . More Info
  2009-04-30: A500 Latest Firmware Release......... v35 . More info
  2009-05-08: B600 Latest Firmware Release......... v02 . More Info
  2009-07-03: B700 Latest Firmware Release......... v35 . More Info

- Updated Makefile Build
  make zaptel ZAPDIR=<absolute path to zaptel>  # Build for Zaptel
  make dahdi DAHDI_DIR=<absolute path to dahdi> # Build for DAHDI
  make install

  	or use original script method

  ./Setup install	# General Build
  ./Setup zaptel    # Zaptel specific build
  ./Setup dahdi     # Dahdi specific build

- For 64bit systems there is no need to use --64bit_4G flag any more.
  The check is done in the driver.

- Updated TDM ring buffer logic to battle noise and data
  corrupting in channelized voice mode.  Backport from 3.5.

- Updated for 2.6.29 kernel.

- Added DAHDI 2.2 support

- AFT TE3 Update
  Added a define AFT_TE3_IFT_FEATURE_DISABLE to disable
  the hardware feature that implements IFT timeout interrupt
  on DMA chains.  On some very slow machines this feature
  can yeild slower performance because the machine does
  not have to power to fill the T3/E3 line with traffic.

- Turned off Framer Timer Interrupt
  It was only used for stats and was not necessary

- Update for DMA chains to combat PCI IRQ latency

- Updated 64bit data protocols

- Updated smg_ctrl scripts for bri and ss7
  made them common

- Bug fix for Mixed Voice & Data mode
  On fifo overrun recovery do not disable dma if running in mixed
  voice & data mode.

- Updated to wancfg_zaptel script.
  Bug fix smg was prompting user even if bri was not installed.

- Updated SMG/Woomera
  Added Asterisk Load balancing using extension information.


* Fri Apr 30 2009 Nenad Corbic <ncorbic@sangoma.com> - Stable - 3.4.1
=======================================================================  

- Updated wancfg_zaptel configuration utility
  Use wancfg_fs to configure analog/BRI card for FreeSwitch
  Update for remove old start/stop script
  Added configuration option for TDMV_HW_FAX_DETECT
  Bug fixes in silent option

- A500 BRI - Firmware Update V35
  On some machines the A500 card caused PCI parity errors,
  causing a system reboot or a crash. The firmware V35
  fixes this problem.

- BRI DCHAN Bug Fix
  The DCHAN on some machines could become stuck due to a 
  driver interrupt race condition, causing BRI to stop
  reciving/tranmsitting calls. This bug has now been fixed.

- Updated BRI stack and sangoma_mdg      

- Added TDMV_HW_FAX_DETECT option
  To enable hardware fax detection. Used in conjunction with
  TDMV_HW_DTMF.  
  TDMV_HW_FAX_DETECT=<option>
   <option> = 0 or NO  - disabled
            = 1-8 or YES - enabled
		      The number represents fax tone timeout
		      value in seconds on call start. The fax
			  event will be valid for X about of seconds
			  at the start of the call. Default=8 seconds.
			  Option YES is equal to Default 8 seconds.
- Adsl Bug Fix
  Updated SNR reporting, fixes connecting issues on some lines.
  Updated stability on multiple restarts on some machines.

- Bug fix in chan_woomera on transfer
- Major regression testing of BRI/SS7 chan_woomera and sangoma_mgd
- Stable 500 Long Term Call load of Astersk+SMG/SS7
- Build update for latest 2.6.28 kernel.
- Build update for latest 2.6.29 kernel.
- Build update for latest Suse 


* Mon Mar 11 2009 Nenad Corbic <ncorbic@sangoma.com> - Beta - 3.3.16
=====================================================================

- Fixed S514 2 Byte receive

- A14X Serial Driver Update
  Added clock recovery option
  Added control of DTR/RTS on startup using 
  DTR_CTRL and RTS_CTRL config variables.

- AFT T1/E1 BERT Feature
  Ability to run Bert tests on T1/E1 cards using wanpipemon

- AFT HWEC
  Hardware DTMF detection now detects FAX tones as well,
  and passes then up to Zaptel/Asterisk

- PPP Protocol
  Added option to disable MAGIC number in negotiation
  Set MAGIC_DISABLE=YES in ppp profile section.

- Updates for new 2.6.X kernels
  Added queue check on shutdown
  Support for latest 2.6 kernels

- Updated T3/E3 Code
  The T3/E3 dma logic had a bug that was demonstrated under
  high load. Was introduced in 3.3.15 release.

- Added hardware probe DUMP 
  wanrouter hwprobe dump
  To be used by developers to easily parse the hwprobe output

- ADSL Update
  Bug fix in adsl. Could cause period disconnects on
  some equipment. The SNR value was not being properly reported
  the the upstream.


* Tue Dec 08 2008 Nenad Corbic <ncorbic@sangoma.com> - Beta - 3.3.15
=====================================================================

- Added support for new B600 Analog Card

- Added task stop function on shutdown.
  This feature prevents an unlikely event that task gets
  scheduled after card has shutdown.

- Fixed tty driver for new Linux Kernel 2.6.26 and greater

- Fixed tristate mode using wanpipemon
  Used to enable/disable T1/E1 transmitter.
  wanpipemon -p aft for help.

- Fixed yellow alarm handler on T1/E1

- Added an option to run Zaptel & TDM API on same span.

- Fixed E3 Support for A301 cards.
  A301 cards need new Firmware v11 and CPLD v0.
  wanrouter hwprobe verbose shows current firmware and cpld version.
  
- Fixed Serial AFT card events
  Added X21 Support

- Added XMTP2 API for old PMC T1/E1 cards.

- Fixed Channel Bank ring problem on AFT T1/E1 cards.
  Some channel banks would ring all phones on card stop.
  This is now fixed.

- Updated AFT Core to fix XMTP2 startup with 16E1 links.

- Updated API for A14X Serial Cards
  Ability to receive Modem Events.

- Updated for Serial A14X API
  API for setting and receiving line events
  Sample code wanpipe-<version>/api/aft/aft_api_serial.c

- Updated Serial A14X for NRZI
  New firmware update v05

- Added wanrouter hwprobe dump
  Should be used for programs to parse hwprobe easily


* Tue Oct 8 2008 Nenad Corbic <ncorbic@sangoma.com> - Beta - 3.3.14
=====================================================================

- Updated TDM API 
  This update helps FreeSwitch analog support.

- Analog TDM API TAPPING Feature
  Sangoma Analog cards and TDM API used in TAPPING mode can be used to build
  custom call recording applications

- Enabled Zaptel/DAHDI operation mode 
  This driver supports DAHDI/Asterisk
  Dahdi is supported on Asterisk 1.6 and 1.4
  Added dahdi to installation modes.
	-> ./Setup install 	#General installation
	-> ./Setup zaptel	#Zaptel based installation
	-> ./Setup dahdi	#Dahdi based installation
	-> ./Setup bri		#SMG BRI installation

- LibSS7 MTP2 Option
  The MTP2 option improves the performance of LibSS7
  eg: instead of using: dchan=24 use mtp2=24
  Please consult libss7 documentation.

- Updated TDM API for A200
  Fixes FreeSwitch OpenZAP with TDM API for Analog

- Bug fix in XMTP2 API
  The fifo error could cause xmtp2 buffering to fail

- wanpipemon utility
  Updated T1/E1 Loopback commands
  Bug fix on 2.6.25 kernels

- Driver compile update for Latest
  2.6.26 kernels

- Update AFT driver to implement new loopback commands

- Updated for 56K driver

- Added Asterisk DAHDI Support

- A200/A400 Analog driver update
  Bug fix possible race condition due to front end interrupt.

- AFT Core Update
  Disabled fifo overrun handling in transparent mode.
  It does not provide any improvement.
  XMTP2 API mode could run out of buffers due to overrun errors.
  This has now been fixed.

- Fixed A301 E3 Support
  New firmware V11 is needed.
  Please upgrade firmware before starting up the card.
  To check firmware run: wanrouter hwprobe
  

* Fri Sep 1 2008 Nenad Corbic <ncorbic@sangoma.com> - Beta - 3.3.13
=====================================================================

- This release was never released.



* Fri Aug 1 2008 Nenad Corbic <ncorbic@sangoma.com> - Beta - 3.3.12
=====================================================================

- Compilation fix for 2.6.25 kernel
  Introduced in 3.3.11 release

- Setup update
  Added bri and zaptel installation modes.
	-> ./Setup install 	#General installation
	-> ./Setup zaptel	#Zaptel based installation
	-> ./Setup bri		#SMG BRI installation

  Streamlined Setup installation script
  Added check for Asterisk directory when installing BRI
  Added check for x86 in architecture

- Fixed Makefile build

- Updated README File
  

* Wed Jul 30 2008 Nenad Corbic <ncorbic@sangoma.com> - Beta - 3.3.11
=====================================================================

- BRI to Analog Faxing Update
  Improved faxing over SMG Gateway and Zaptel Analog.
  This feature drastically improves faxing performance between
  BRI and Analog cards over Asterisk.

- BRI Update
  Added overlap dialing support
  Added support for BRI with 16 channel Echo Canceler.
  Support for test start option, that only verifies configuration 
  files without applying changes.
  Support for non-syslog logging.

- BRI Bug Fix
  BRI was configured for answer on accept.  This caused a call to
  be answered before the remote leg of the call became connected.
 
- T1/E1 Loopback Update
  wanpipemon was update to check loopback status as well as set
  T1/E1 loopback commands.

- Removed S508 ISA Support from drivers

- Fixed the BRI chip security handler. Tested that all BRI 
  ports get properly shut down. 

- Updated for New PLX & Tundra PCIe Bridges chips
- Update hwprobe verbose for PMC Shark

- Updated for 2.6.25 kernel

- Fixed 64bit compilation for Octasic EC Image

- TDM API Bug Fix: 
  Added a check for buffer overflow in write function.

- New Octasic EC Image
  Improves faxing over Hardware Echo Canceler. 

- BRI Bug fix on startup
  The clock measuring function has limited and could fail on some machines.

- AFT T1/E1 - Added missing LBO configuration option WAN_T1_0_110

- Updated for 2.6.25 kernel

- Updated wancfg for interface MTU/MRU
  By default do not configure interface MTU/MRU if values have
  not changed by default.  This way the global mtu can be used
  to easily configure the MTU/MRU of all interfaces.

- Updated wancfg_smg to configure XMTP2 API

- Wireshark Tracing for MTP2
  http://wiki.sangoma.com/wanpipe-wireshark-pcap-pri-bri-wan-t1-e1-tracing

- T3/E3 Update
  Driver level update to improve code.

- MTP2 API Support
  http://wiki.sangoma.com/wanpipe-aft-ss7-mtp2-api


* Fri May 15 2008 Nenad Corbic <ncorbic@sangoma.com> - Beta - 3.3.10
======================================================================

- BRI Update
  Binary segfaulted on some systems due to gcc incompabilitly.
  This is now fixed.

* Fri Apr 30 2008 Nenad Corbic <ncorbic@sangoma.com> - Beta - 3.3.9
======================================================================

- AFT BRI Clock Reference Update
  Disabled reference clocking feature on original A500 BRI Cards.
  It has been determined that reference clocking feature is not always stable 
  on original A500 BRI cards.  It can cause noise and call drops in some 
  circumstances where BRI lines go up and down due to power saving mode.
  
  If you have problems with FAX synchronization on an original A500 BRI Card
  contact Sangoma Support and we will swap out the card for one with
  an updated CPLD that will work with the reference clock and provide
  reliable FAXing.

  Run wanrouter hwprobe verbose to determine your A500 BRI CPLD Version
   -> wanrouter hwprobe verbose
	-C00 -> old bri cpld (non reference clock)
	-C01 -> new bri cpld (reference clock enabled)


- Manually Disabling BRI Clock Reference
  This option is valid from 3.3.6 release and greater.
  This option can be used in case of noise and voice quality issues
  and call drop issues on BRI card.  

  In order to disable BRI clock reference manually one can add
  RM_BRI_CLOCK=OSC in each BRI wanpipe config file in /etc/wanpipe directory.
  1. vi /etc/wanpipe/wanpipe1.conf
  2. Under the TDMV_SPAN option add
     RM_BRI_CLOCK=OSC
  3. Save file
  4. Repeat for all BRI wanpipe configuration files
  5. Restart all wanpipe devices
  
  Note from 3.3.9 release on, all old CPLD A500 BRI cards
  have clock referencing disabled automatically.

- Updated BRI Stack
  Fix for RDNIS not cleared
  Support for show_spans and show_calls.
  Added support for multiple MSNs.
  Added support for timer_t3 and timer_t302
  http://wiki.sangoma.com/sangoma-wanpipe-smg-asterisk-bri-installation
  

* Fri Apr 25 2008 Nenad Corbic <ncorbic@sangoma.com> - Beta - 3.3.8
======================================================================

- AFT Update
  A bug was introduced in 3.3.7 release that failed to load
  AFT cards without HWEC.  

- AFT TE1 Code
  Defaulted Maxim T1 Rx Level to 36DB
  Defaulted Maxim E1 Rx Level to 42DB
  This will improve T1/E1 connectivity on noisy or low power lines.

- AFT BRI Update
  Minor update on BRI to conform better to TBR3 Certification 


* Wed Apr 23 2008 Nenad Corbic <ncorbic@sangoma.com> - Beta - 3.3.7
======================================================================

- BRI HWEC MAJOR Bug Fix
  BRI hwec was not configured properly on startup.
  Every second channel on each span was not being configured for hwec.
  This would result in random echo problems.

- Support for 2.6.24 kernels and up
  This release will now compile properly on 2.6.24 kernels and up.

- AFT Analog Update
  Added TBR21 operation mode
  Used for most european countries.
  -> this option must be added manually in wanpipe#.conf
  -> TDMV_OPERMODE=TBR21

- AFT A144 Update
  Added MPAPI X25 support to AFT A144/A142 cards
  Use: wancfg_legacy to configure MPAPI over AFT cards.
  http://wiki.sangoma.com/wanpipe-linux-mpapi-x25

- BRI Updated
  BRI driver updated for new 512hz clock used to improve
  hardware echo canceler with clock recovery mechanism.
  This featue will solve any random hwec warning messages
  one might see in /var/log/messages.  Note these random
  warning messages did not cause any abnormal behaviour 
  in extensive lab testings.   
  To check your BRI hardware version run:	
  -> wanrouter hwprobe verbose
	-C00 -> old bri cpld
	-C01 -> new bri cpld

- Wanpipemon PRI/BRI PCAP Tracing for Wireshark
  Using wanpipemon dchan trace one can now capture
  pcap files that can be opened by Wireshark.
  http://wiki.sangoma.com/wanpipe-wireshark-pcap-pri-bri-wan-t1-e1-tracing

- S514 Secondary port bug fix
  The secondary port was not working.

- Updated wanrouter hwprobe
  New wanrouter hwprobe device summary line will only contain
  found devices. For backward compatibility we created "wanrouter hwprobe legacy"
  that can be used to revert hwprobe output to the original format.

- Add pci parity check to wanrouter 
  wanrouter parity  	-> displays current system pci parity
  wanrouter parity off 	-> disables system pci parity
  wanrouter parity on	-> enables system pci parity
  
  /etc/wanpipe/wanrouter.rc  
	WAN_PCI_PARITY=OFF -> on wanrouter start disable pci parity
			   -> event logged in /var/log/wanrouter

  On some servers pci parity can cause NMI interrupts that
  can lead to reboots.  Parity can be caused by unsuported
  or buggy pci/bridge chipsets.  The above commands can be used
  to combat pci parity reboots.

  Another option is to disable PCI parity in BIOS :)


* Wed Apr 4 2008 Nenad Corbic <ncorbic@sangoma.com> - Beta - 3.3.6
======================================================================

- BRI Driver bug fixes
  Secured the bri restart logic to prevent possible
  race conditions.

- BRI Stack update
  Bug Fix: bri stack update fixes reconnect on etsi lines
  Feature: group outbound calls skip disconnected lines

- T3 update
  Fixed dma issues on some machines when tx/rx mixed
  voip and data traffic.

- Hardware Probe Verbose Update
  Analog and BRI cards now display PCI/PCIe info on
  wanrouter hwrobe verbose.


* Wed Mar 27 2008 Nenad Corbic <ncorbic@sangoma.com> - Beta - 3.3.5
======================================================================


- Removed debugging out of firmware update utility
- Updated firmware bin files
- Updated firmware update script.

- No Functional Changes from 3.3.4

* Wed Mar 26 2008 Nenad Corbic <ncorbic@sangoma.com> - Beta - 3.3.4
====================================================================== 

- BRI TE auto clocking feature - Bug fix
   This feature failed on on some machines with multiple TE BRI modules.
   This bug would cause modules to loose sync. Bug introduced in 3.3.3
   release.

* Tue Mar 25 2008 Nenad Corbic <ncorbic@sangoma.com> - Beta - 3.3.3
====================================================================== 

-  BRI TE auto clocking feature. 
    Where a connected TE port is elected
    as the telco clock recovery port for the whole card.  If that TE
    port goes down, another connected TE port is found to provide
    telco recovery clock to the card.  If no connected TE ports are found
    then internal oscillator is used. 
    -> This option is fully automatic no configuration options needed.

-  BRI Zaptel Clock Source
    Since BRI does not interface to zaptel, it acts as ZT DUMMY to
    provide zaptel reliable timing.  One has to configure
    TDMV_DUMMY_REF=YES in [wanpipe1] section of wanpipe1.conf

-  A200/A400 Remora Relax CFG
    If one module fails during operation the wanpipe driver by default
    fails to load.  With this option wanpipe driver
    will allow the card to come up with a broken module so that
    customer will be able to continue working until the module is replaced.
    RM_RELAX_CFG=YES in [wanpipe1] section of wanpipe1.conf



* Fri Feb 14 2008 Nenad Corbic <ncorbic@sangoma.com> - Beta - 3.3.2.1
====================================================================== 

- Added DTR API for Serial S514X Card
  Ability to read and set the DTR/RTS on serial cards throught the API.
  Sample code in wanpipe-3.3.2.1/api/chdlc/chdlc_modem_cmd.c


* Wed Feb 12 2008 Nenad Corbic <ncorbic@sangoma.com> - Beta - 3.3.2
====================================================================== 

- Support for A500 hardware support with NetBricks BRI Stack
- Major A500 driver updates and fixes
- Serial A142/A144 hardware support
- AFT A056 56K hardware support 

- Support for HW DTMF

- Updates for AFT PMC and MAXIM framers
  PMC - lowered LOS sensitivity
        Fixes fake up/down state changes on
        started inactive lines.

  MAXIM - lowered sensistivy
          Fixes cable cross talk on 8 port cards.
        - Enabled Unframed E1
        - Enabled Tri-State Mode
        - Fixed loopback commands

- Updated loopback commands for AFT Maxim cards

- Updated for AstLinux
  The make file can now build all WAN and Voice Protocols
  
- Updated legacy protocols for new front end architecture

- 
- - END - 
