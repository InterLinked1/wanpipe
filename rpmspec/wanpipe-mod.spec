%define WANPIPE_VER	  wanpipe-modules
%define name              %{WANPIPE_VER}
%define version           2.3.4
%define release           2
%define	serial	 	  1
%define MODULES_DIR	  /lib/modules
 

Summary: 	Sangoma WANPIPE package for Linux. It contains the WANPIPE kernel drivers.  Please install wanpipe-util package for wanpipe utilties and configuration files.
Name: 		%{name}-%{?kern_ver}
Version: 	%{version}
Release: 	%{release}
Copyright: 	GPL
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
depmod -a >> /dev/null 2>> /dev/null

modprobe wanrouter
if [ $? -eq 0 ]; then
	echo "Wanpipe kernel modules installed successfully"
	modprobe -r wanrouter 2>> /dev/null
else
	echo "Failed to load Wanpipe modules!"
	echo
	echo "Make sure you are installing correct RPMS for you system!"
	echo
	echo "Otherwise call Sangoma Tech Support"
fi


%files
%{MODULES_DIR}


%changelog

* Wed Nov 23 2006 Nenad Corbic <ncorbic@sangoma.com> - 2.3.4-1
====================================================================

- This is the first official STABLE release of 2.3.4 branch.
 
- AFT A104d 64bit bug fix
  The AFT A104d T1 channel 24 failed to initialize causing DCHAN
  not to come up on 64 bit machines.

- SMG Update 
  Major optimizations to greatly improve system load and capacity.
  Added rx tx gain control from CLI and woomera.conf
  Woomera v1.5  SMG v1.6

- Driver update for 2.6.18 kernel.

- Wanpipemon Updates
  Updated for A301 drivers.
  Minor bug fixes.

- CHDLC Protocol
  Fixed up the ignore keepalive

- WANCFG - CHDLC Protocol
  Bug fix in chdlc configuration.


* Thu Nov 02 2006 Nenad Corbic <ncorbic@sangoma.com> - beta12-2.3.4
===================================================================
- Further Analog Bug Fixes.
  Fixes the problem where fxo module gets stuck during operation.



* Wed Nov 01 2006 Nenad Corbic <ncorbic@sangoma.com> - beta11-2.3.4
===================================================================
- A102D Hardware Support
  Updated drivers for A102D Hardware
  Minimum A102D firmware version V.28

- Bug fix in Analog A200 driver.
  Every once in a while a fxo port would get stuck and fail to
  receive any more calls. This has not been fixed.

- Bug fix in Analog A200 driver.
  The lasttxhoot state could get stuck in one state.
  If you have weird analog A200 issues, please try this release.

- New SMG update

- Setup installation script update

- New A200 voltage statistics in wanpipemon

- New wanrouter hwprobe verbose option
  Displays all FXO/FXS modules installed on your system
  wanrouter hwprobe verbose


* Fri Oct 13 2006 Nenad Corbic <ncorbic@sangoma.com> - beta10-2.3.4
====================================================================

- A200 Bug fix
  Analog software echo cancellation has been broken
  in beta8-2.3.4 release.  This bug has now been fixed.
  This bug does not affect A200d cards with HWEC.

- Setup update
   Update in compilation headers that caused wan_ec_client
   to fail compilation on some machine.

* Fri Oct 6 2006 Nenad Corbic <ncorbic@sangoma.com> - beta9-2.3.4
- Bug fixes hwec that were introduced in beta8
- Disabled hw dtmf events.
- This release has been tested in production.

* Mon Oct 2 2006 Nenad Corbic <ncorbic@sangoma.com> - beta8-2.3.4
=================================================================

- New A108 HWEC Support
  Optimized TDM Drivers for A108 
  Support for A108 Hardware Echo Cancellation
  Support for A108 RefClock between ports.
  New Firmware. V27

- New A108 HWEC Support
  Optimized TDM Drivers for A108 
  Support for A108 Hardware Echo Cancellation
  Support for A108 RefClock between ports.
  New Firmware. V27

- LIP / AFT Layer Latency Optimization (LINUX Only)
  By setting txqueuelen using ifconfig to value of 1
  the LIP layer will re-configure the protocol and hw layer 
  for SINGLE tx buffer. This will improve overall latency.

- AFT (A101/2/4/8) Layer Latency Optimization
  The SINGLE_TX_BUF=YES option in [w1g1] interface 
  section, configures the AFT hardware for single hw dma buffer
  instead of a DMA chain.  This option along with above
  LIP layer latency option reduces driver/protocol latency
  to theoretical minimum.
  NOTE: In WAN/IP applications latency feature will increase
  system load.
  (5% to 20% depending on system and number of ports used) 


* Mon Jul 31 2006 Nenad Corbic <ncorbic@sangoma.com> - beta7-2.3.4
==================================================================
- A200 Driver Bug Fix.
  The A200 drivers had PULSE dialing enabled
  that can cause call outages on NON-PULSE dialing lines.
  This has now been fixed.

- Updates to ./Setup installation script
  More documentation in usage.    



* Mon Jul 24 2006 Nenad Corbic <ncorbic@sangoma.com> - beta6-2.3.4
==================================================================
- Fixed the AFT HWEC on 64bit kernels.
  The bug was caused by using 32bit version of
  the octasic API release in beta5-2.3.4.



* Fri Jul 21 2006 Nenad Corbic <ncorbic@sangoma.com> - beta5-2.3.4
==================================================================
- TDM API was broken on A101/2 Cards
  It has not been fixed.

- A108D Hardware Support
  The new A108D: 8 Port T1/E1 with onboard 246 channel 
  hardware echo canceller.

- The wanec daemon has been deprecated.
  A new wanec kernel module has been created to handle
  all octasic commands. 

- The hwec release has been deprecated.
  The hwec drivers are now part of wanpipe release.

- Bug Fix in A104 and A104D Ref Clock 
  This bug caused FAX failures when REF Clock option was enabled.

- Updates for 2.6.16 and 2.6.17 kernels
  New kernels broke previous wanpipe drivers.

- Frame Relay Update: EEK Support
  New Frame relay stack supports Cisco End to End Keepalives
  on each DLCI.

- PPP Update: Updated PPP IP negotiation. IPCP was not
  working properly agains some routers.  This has now
  been resolved.

- TDM API: HW DMTF on A104D cards.
  The TDM API and regular API interfaces now supports HW DTMF
  on A104D and A108D cards.

- Setup update: the --protocols option with --silent option did not
  select proper protocols under certail conditions.
  ./Setup install --protocols=TDM  --silent
  This option will now compile TDM protocol properly.

- ADSL Update: The ADSL drivers were failing on some machines due
  to kfree_skb error under irq.  This has now been fixed.

- AFT A104/A108 Driver updated for 64bit kernels with over 4GB 
  of memory.  The NO AUDIO problem on TDM drivers.  
  The 4GB memory caused DMA corruptions to occur which caused
  no AUDIO effect.

- SSMG Product: Sangoma Signal Media Gateway
  The ssmg product is now part of standard wanpipe.
  It will automatically get installed if XMTP2 drivers are
  selected.  The SMG product provides SS7 Support to Asterisk.

- LibSangoma: LibSangoma is part of the SSMG product, however
  it can be used separately with TDM API to develop custom
  TDM applications.  Libsangoma is now part of standard wanpipe
  release. 

* Fri Mar 04 2006 Nenad Corbic <ncorbic@sangoma.com> - beta4-2.3.4
- A108 Hardware Support
  Full support for new A108 8 Port T1/E1 Card
	
- AFT Front End T1/E1 Alarm Update/Polling
  On some embedded machines the A104D cards exhibited
  unreliable ports i.e. The port would not come up due to
  missing interrupts from the front end chip.  
  The Front End polling mechanism has been updated to solve this problem.  

- TDM API Update
  Fixed a bug in RX/TX functions.
  On some machines the kernel to user copy did not work.

- Updated HWEC and TDMAPI Udev Devices
  HWEC UDEV device: /dev/wp1ec (Major: 241)
  TDM API UDEV device: /dev/wptdm_s1c1 (Major: 242)
	
- Setup Installation Script Update
  Compilation of Zaptel during installation
  UDEV Zaptel updates
  UDEV TDM API updates

- AFT-LIP Frame Relay DLCI Statistics
  Updated Frame Relay DLCI Statistics.

- AFT-LIP PPP Update
  IPCP negotiation failed when connected to some telco.

* Wed Feb 22 2006 Nenad Corbic <ncorbic@sangoma.com> - beta3-2.3.4
- A104D HWEC Update
  Bug fix on E1 4th port channel 31 did not work.

- A104D HWEC 64Bit Fix
  The A104D HWEC now works on 64bit machines.
  Using UDEV /dev/wp1ec device to access hwec chip.
  If no UDEV one must create /dev/wp1ec using:
		mknod /dev/wp1ec c 2402 1 

- AFT Firmware Update Utility 
  Bug fix. On some systems firmware update failed. 

- TDM API Updates
  Support on AFT A101/2 Cards
  Fixed shutdown problems while channels are opened.
	  
* Wed Feb 15 2006 Nenad Corbic <ncorbic@sangoma.com> - beta2-2.3.4
- The Beta2-2.3.4 is the first official Beta release of 2.3.4.
  The 2.3.4 release has been in ALPHA for months and has gone
  through major testing process.

- A200 Remora Updates
  Updates for 24 port A200 solution
  New A200 Firmware Release V05

- A104D Update/Bug Fix 
  Echo cancellation DSP had problems on
  port 4 of A104D card.

- New A104/4D TDM API and libsangoma release
  The new TDM API replaces the old TDM API.
  Currently the TDM API is only supported on A104 and A104D cards.

  The TDM API is compiled by default in wanpipe drivers.
  Supports: 	PRI and RBS functionality.
  Sample files: /etc/wanpipe/api/tdm_api 

- Libsangoma Release
  New libsangoma source is located in 
  /etc/wanpipe/api/libsangoma directory.

  For more info and docs please refer to http://sangoma.editme.com


* Tue Jan 10 2006 Nenad Corbic <ncorbic@sangoma.com> - beta1y-2.3.4
- Driver update for 2.4.30 kernel 
- AFT A104 Driver minor update to TDM API 
  Number of dma packets used in API mode.
  To save memory. Not a functional change.

- Update to Setup install script

- Wancfg Update
  New option to configure wanpipe config file base on zaptel.conf
  syntax:  "wancfg zaptel"

* Tue Dec 15 2005 Nenad Corbic <ncorbic@sangoma.com> - beta1x-2.3.4
- Major AFT A200 Analog Updates
  Analog Hardware Echo Canceler is now supported
  Bug fixes on aLaw lines, noise problems.
	  
- Wancfg Updates

- Updates to Setup installation utility

* Mon Dec 5 2005 Nenad Corbic <ncorbic@sangoma.com> - beta1x-2.3.4
- WanCfg Update 
  New wancfg option "zaptel" that will create wanpipe
  based on /etc/zaptel.conf configuration.
  eg:
	1. Configure your zaptel first based on hardware
	2. Run: wancfg zaptel 
	   To create wanpipe config files based on zaptel.conf
        3. Run: wanrouter start
	4. Run: ztcfg -vvv
	5. Read to start Asterisk
	
- Setup Update
  Added new options in ./Setup for user to specify custom gcc	
  compiler.  

- AFT A200 Updates
  Further Analog driver updates.
 
- AFT Echo Debugging Utility
  Used to measure echo spike and debug echo problems. 


* Wed Nov 23 2005 Nenad Corbic <ncorbic@sangoma.com> - beta1x-2.3.4

- Bug fix in Analog startup
  You can now set ACTIVE_CH=ALL and by default all detected
  FXO/FXS Devices will be configured.

- Bug fix in A102 Drivers
  When both ports are running, if you stop one 
  the other one dies.
- - END - 
