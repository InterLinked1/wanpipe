%define WANPIPE_VER	  wanpipe-modules
%define name              %{WANPIPE_VER}
%define version           3.1.3
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
* Mon Jun 30 2007 Nenad Corbic <ncorbic@sangoma.com> - Beta - 3.1.3
==================================================================== 

- Update to Ocatsic Hardware Echo Canceler Library
  Turned of the NOISE suppression because it can interfere
  with faxes. If you faxes did not work properly on 3.1.2
  release they will work fine with this one.

- Cleaned up the Setup installation script.


* Mon Jun 16 2007 Nenad Corbic <ncorbic@sangoma.com> - Beta - 3.1.2
==================================================================== 

- Update to Octasic Hardware Echo Canceler library
  This is a very important update that affects all AFT cards
  with octasic hardware echo canceler.  The new octasic update
  fixes faxing/modem issues over octasic hwec.  The previous
  release contained a bug that limited the faxing/modem speeds
  to 26k.  The new update properly detects fax/modem and works
  with full speed of 33k fax and 56k modem.

- A200/A400 Updated
  This update fixes the offhook startup failure.
  On startup if fxs is offhook driver will start correctly

- Wanpipe Startup order changed
  The wanpipe startup scripts on bootup were previously
  set too early "S03wanrouter".  This caused unpredictable
  behaviour on some systems.  We have now moved wanrouter 
  startup on boot up to "S11wanrouter", after networking
  code.

- Zaptel Adjustable Chunk Size Feature
  Wanpipe drivers can work with 1,2,5 and 10ms 
  chunk size.  Zaptel also supports this, however
  the wct4xx driver breaks compilation when chunk
  size is changed.  ./Setup can how change the
  zaptel chunk size for you and update zaptel
  Makefiles to remove wct4xx driver out.

  Zaptel with 1ms generates 1000 interrupts per sec
  Zaptel with 10ms generates 100 interrupts per sec.

  As you can see its a drastic interrupt performance
  increase.

  NOTE: This breaks software echo cancelation, but
        its not needed since we have hwec.


* Fri Jun 06 2007 Nenad Corbic <ncorbic@sangoma.com> - Beta - 3.1.1
==================================================================== 

- A101/2/4/8 (MAXIM) AFT Update IMPORTANT
  A major bug fix for AFT Maxim E1 cards for E1 CRC4 Mode.
  On some lines the E1/CRC4 mode causes line errors on 
  the telco side which results in PRI not coming up.
 
  Symptiom: E1 is up (no alarms) on local side but pri is 
 	    not coming up!  (Only in E1 CRC4 Mode)

- A101/2/4/8 (MAXIM) Mandatory Firmware Update
  An echo canceler bug has been fixed for all AFT
  MAXIM Cards A101/2/4/8dm.  New firmware version is V31.
  If you are running MAXIM cards with hwec wiht older
  firmware version you must upgrade.

- Updated SMG 
  Fixed DTMF synchronization



- Updated SMG 
  Fixed DTMF synchronization


* Fri Jun 06 2007 Nenad Corbic <ncorbic@sangoma.com> - Beta - 3.1.0.1
==================================================================== 

- Minor release
- Contains zaptel patch for zaptel 1.2.17 and above.
- No driver changes 

* Fri May 17 2007 Nenad Corbic <ncorbic@sangoma.com> - Beta - 3.1.0
====================================================================  

- Major new BETA wanpipe release 
  Changed wanpipe versioning:
	Release: A.B.C.D
	A - Major Relase number
	B - Indicates Stable or Beta
	    Odd number is Beta
	    Even number is Stable
	C - Minor Release number
	D - Optional pre-release and custom releases
 
- Fixed RBS Support for all Maxim cards A101/2/4/8.

- Support for 2.6.20 kernels.

- Support for New: A101D A102D A104D Maxim cards
     :
- Support for New: AFT 56K DDS card

- Redesigned TDM API Events

- TDM API Analog Support

- - END - 
