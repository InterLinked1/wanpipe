%define WANPIPE_VER	  wanpipe-modules
%define name              %{WANPIPE_VER}
%define version           3.3.14
%define release           0
%define	serial	 	  1
%define MODULES_DIR	  /lib/modules
%define USR_INCLUDE_DIR   /usr/include

%define KVERSION          %{?kern_ver}
 

Summary: 	Sangoma WANPIPE package for Linux. It contains the WANPIPE kernel drivers.  Please install wanpipe-util package for wanpipe utilties and configuration files.
Name: 		%{name}-%{?kern_ver}
Version: 	%{version}
Release: 	%{release}
License: 	GPL
Group: 		Applications/Communications
Vendor:		Sangoma Technologies Inc.
Url:		www.sangoma.com
Group:		Networking/WAN
 

%description 
Linux Drivers for Sangoma AFT Series of cards and S Series of Cards. Wanpipe supports the following protocols, TDM Voice, Frame Relay, X25(API), PPP,Multi-link PPP, CHDLC and custom API development for WAN and Voice.

Install Wanpipe-util package for wanpipe utilities and configuration files.


%prep

%build

%install

%clean

%postun

echo "Uninstalling WANPIPE..."

%post

#check dependancies for the new modules
depmod -ae -F /boot/System.map-%{KVERSION} %{KVERSION}
echo "Wanpipe Modules located in %{MODULES_DIR}/%{KVERSION}"   

%files
%{MODULES_DIR}
%{USR_INCLUDE_DIR}


%changelog
* Tue Oct 8 2008 Nenad Corbic <ncorbic@sangoma.com> - Beta - 3.3.14
=====================================================================

- Enabled Zaptel/DAHDI libss7 MTP2 option

- Updated TDM API for A200
  Fixes FreeSwitch OpenZAP with TDM API for Analog

- Bug fix in XMTP2 API
  The fifo error could cause xmtp2 buffering to fail

- wanpipemon utility
  Updated T1/E1 Loopback comands
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
  It does not provide any improvemnt.
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


* Fri Feb 01 2008 Nenad Corbic <ncorbic@sangoma.com> - Beta - 3.3.2.p8
====================================================================== 

- wancfg_zaptel now asks for the default_tei value for 
- BRI cards in TE mode

- Fix for HWEC not being enabled when non-consecutive modules are using 
- in BRI cards

* Fri Feb 01 2008 Nenad Corbic <ncorbic@sangoma.com> - Beta - 3.3.2.p4
====================================================================== 

- Fixed AFT memory leak
  Memory leak introduced in 3.3 release
- Fixed AFT 56K bug
  Bug introduced in 3.3 releae


* Fri Feb 01 2008 Nenad Corbic <ncorbic@sangoma.com> - Beta - 3.3.2.p3
====================================================================== 

- Fix bug in BRI protocol for fast local hangups.

* Mon Jan 18 2008 Nenad Corbic <ncorbic@sangoma.com> - Beta - 3.3.2.p1
====================================================================== 

- Bug fix in Hardware EC code for E1.
  Bug introduced in 3.3 release.


* Mon Jan 18 2008 Nenad Corbic <ncorbic@sangoma.com> - Beta - 3.3.1
==================================================================== 


* Mon Jan 16 2008 Nenad Corbic <ncorbic@sangoma.com> - Beta - 3.3.0.22
==================================================================== 

- BRI protocol:Increased internal timer that could cause issue in systems with
- more than 8 BRI spans

* Mon Jan 15 2008 Nenad Corbic <ncorbic@sangoma.com> - Beta - 3.3.0.21
==================================================================== 

- BRI protocol:Fix for smg_brid daemon crashing on race condition
- BRI protocol:default_tei parameter is not ignored when using point to 
- multipoint anymore

* Mon Jan 14 2008 Nenad Corbic <ncorbic@sangoma.com> - Beta - 3.3.0.20
====================================================================  

- BRI protocol: Additional prefix options. 
- BRI protocol: Check is caller ID number is all digits on incoming calls
- Sangoma MGD: Removed dynamic user period causing skb panics
- chan_woomera: Fixed issue with rxgain and txgain values set to 0 if 
- coding not set in woomera.conf
- wancfg_zaptel: Support for fractional T1/E1 spans.
- wancfg_zaptel: fix issue BRI always being configured as bri_net introduced in v3.3.0.19

* Mon Jan 07 2008 Nenad Corbic <ncorbic@sangoma.com> - Beta - 3.3.0.19
====================================================================  

- Support for national/international prefix in BRI stack

* Mon Jan 07 2008 Nenad Corbic <ncorbic@sangoma.com> - Beta - 3.3.0.18
====================================================================  

- Changed Makefile in wanpipe/api/fr causing compilation errors 


* Thu Dec 20 2007 Nenad Corbic <ncorbic@sangoma.com> - Beta - 3.3.0.17
====================================================================  

- Fix for smg_ctrl boot script starting before network services on some systems
- Support for language parameter in chan_woomera

* Thu Dec 20 2007 Nenad Corbic <ncorbic@sangoma.com> - Beta - 3.3.0.16
====================================================================  

- Fix for Sangoma BRI Daemon crashing on incoming call if chan_woomera is not installed on that system

* Tue Dec 18 2007 Nenad Corbic <ncorbic@sangoma.com> - Beta - 3.3.0.15
====================================================================  

- Fix for caller ID value being corrupted sometimes
- Support for call confirmation in chan_woomera

* Tue Dec 18 2007 Nenad Corbic <ncorbic@sangoma.com> - Beta - 3.3.0.14
====================================================================  

- Fix in smg_brid not releasing some b-channels properly
- Fix in wancfg_smg not setting MTU to 80 when configuring cards for SS7

* Fri Dec 14 2007 Nenad Corbic <ncorbic@sangoma.com> - Beta - 3.3.0.13
====================================================================  

- Fix for Kernel panic on 64-bit systems when enabling hardware echo canceller


* Thu Dec 5 2007 Nenad Corbic <ncorbic@sangoma.com> - Beta - 3.3.0.11
====================================================================  

- Support for AFT Serial Cards
- Updates for AFT PMC and MAXIM framers
  PMC - lowered LOS sensitivity
        Fixes fake up/down state changes on
        started inactive lines.

  MAXIM - lowered sensistivy
          Fixes cable cross talk on 8 port cards.
        - Enabled Unframed E1
        - Enabled Tri-State Mode
        - Fixed loopback commands

- Fixed HWEC_PERSIST_DISABLE
  This option was broken in previous release
  This option lets Asterisk control HWEC
  on each call start/stop.
  By default all hwec channels are enabled on
  device startup.

- Updated SMG/SS7 
- Updated loopback commands for AFT Maxim cards

- Updated for AstLinux
  The make file can now build all WAN and Voice Protocols
  
- Fixed add_timer warnings for ALL AFT cards
  Caused when a port is left in unconnected state.

- Updated legacy protocols for new front end architecture

- Updated Setup script 



* Thu Nov 8 2007 Nenad Corbic <ncorbic@sangoma.com> - Beta - 3.3.0.4
====================================================================  

- Fixed A101/2 (Old) bug introduced in previous releaes

* Mon Oct 31 2007 Nenad Corbic <ncorbic@sangoma.com> - Beta - 3.3.0.4
====================================================================  

- Updated BRI caller name
- Updaged Setup


* Mon Oct 15 2007 Nenad Corbic <ncorbic@sangoma.com> - Beta - 3.3.0.1
====================================================================  

- Major Updates
- New BRI architecture/support
  SMG with Netbricks BRI Stack
- Support for Hardware DTMF


* Thu Aug 22 2007 Nenad Corbic <ncorbic@sangoma.com> - Beta - 3.3.0.p3
======================================================================

- Updated wancfg_zaptel to support HW DTMF


* Thu Aug 21 2007 Nenad Corbic <ncorbic@sangoma.com> - Beta - 3.3.0.p2
======================================================================  

- Major Updates
- Hardware DTMF for Asterisk and TDM API
- - END - 
