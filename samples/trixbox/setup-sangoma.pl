#!/usr/bin/perl
# install_sangoma.pl 
# Sangoma Configuration Script for Trixbox .
#
# Copyright     (c) 2006, Sangoma Technologies Inc.
#
#               This program is free software; you can redistribute it and/or
#               modify it under the terms of the GNU General Public License
#               as published by the Free Software Foundation; either version
#               2 of the License, or (at your option) any later version.
# ----------------------------------------------------------------------------
# October 13, 2006  David Yat Sin      Added --opermode and --law option
# October 12, 2006  David Yat Sin      Initial version
# ============================================================================

print "\n###################################################################";
print "\n#                                                                 #";
print "\n#                Sangoma Configuration Script                     #";
print "\n#                                                                 #";
print "\n#                            v1.0                                 #";
print "\n#                                                                 #";
print "\n#                  Sangoma Technologies Inc.                      #";
print "\n#                     Copyright(c) 2006.                          #";
print "\n#                                                                 #";
print "\n###################################################################\n";

use Switch;


#default parameters
my $tdmv_opermode='FCC';
my $tdmv_law='MULAW';

#remove existing configuration files
@error=`rm -f /etc/wanpipe/samples/trixbox/cfg/*`;
print "@error";
my $startup_string="";

sub get_args{
	foreach $arg_num (0..$#ARGV) {
		$arg = $ARGV[$arg_num];
		switch ($arg) {
                	case ( /^--opermode=/) {
				$arg =~ /=(\w+)/ ;
				$tdmv_opermode = ' ';
			       	$tmp_opermode = $1;
				@tdmv_opermodes = ("FCC","TBR21","ARGENTINA","AUSTRALIA","AUSTRIA","BAHRAIN","BELGIUM","BRAZIL","BULGARIA","CANADA","CHILE","CHINA","COLUMBIA","CROATIA","CYPRUS","CZECH","DENMARK","ECUADOR","EGYPT","ELSAVADOR","FINLAND","FRANCE","GERMANY","GREECE","GUAM","HONGKONG","HUNGARY","ICELAND","INDIA","INDONESIA","IRELAND","ISRAEL","ITALY","JAPAN","JORDAN","KAZAKHSTAN","KUWAIT","LATVIA","LEBANON","LUXEMBOURG","MACAO","MALAYSIA","MALTA","MEXICO","MOROCCO","NETHERLANDS","NEWZEALAND","NIGERIA","NORWAY","OMAN","PAKISTAN","PERU","PHILIPPINES","POLAND","PORTUGAL","ROMANIA","RUSSIA","SAUDIARABIA","SINGAPORE","SLOVAKIA","SLOVENIA","SOUTHAFRICA","SOUTHKOREA","SPAIN","SWEDEN","SWEDEN","SWITZERLAND","SYRIA","TAIWAN","UAE","UK","USA","YEMEN");
                                foreach $opermode (@tdmv_opermodes){
                                 	if ( $opermode eq $tmp_opermode){
						$tdmv_opermode = $tmp_opermode; 
						print "  tdmv_opermode=$tdmv_opermode\n";
					}
				}
				if ( $tdmv_opermode eq ' ' ){
			       		print "\nERROR: invalid tdmv_opermode $tdmv_opermode option\n";
					print "\npossible values are:\n";
					print "@tdmv_opermodes \n";
				        exit;
				}	
			}
                	case ( /^--law=/) {
				$arg =~ /=(\w+)/ ;
			       	$tdmv_law = $1;
			     	if ( $tdmv_law eq 'ULAW' | $tdmv_law eq 'ALAW'){
			       	 	print "  tdmv_law=$tdmv_law\n";			
				}else{
					print "\nERROR: Invalid value for tdmv_law\n";
					print "\nPossible values are: \n ULAW\n ALAW\n\n";
					exit;
				}	
			}
			else {
                        	print "\nUnrecognised parameter $arg, ignored";
			}	


		}	
	}
}


get_args();
my $devnum=0;
my @hwprobe=`wanrouter hwprobe`;
foreach my $dev (@hwprobe) {
	if ( $dev =~ /(\d+).(\w+\w+).*SLOT=(\d+).*BUS=(\d+).*CPU=(\w+).*PORT=(\w+).*HWEC=(\d+)/){
		my $card_model=$1;
		my $slot=$3;
		my $bus=$4;
		my $cpu=$5;
		my $port=$6;
		my $hwec=$7;
		my $hwec_mode;
		if ($hwec==0){
			$hwec_mode='NO';
		}
		else{
			$hwec_mode='YES';
		}
		if ( $card_model eq '200' ){
			$devnum++;	
			open(FH, '/etc/wanpipe/samples/trixbox/templates/wanpipe.tdm.a200') or die "Can open template";
			$wp_file='';	
			while (<FH>) {
				$wp_file .= $_;	
			}
			close (FH);

			my $fname="/etc/wanpipe/samples/trixbox/cfg/wanpipe".$devnum.".conf";
			open(FH, ">$fname") or die "Cant open $fname";
		
			$wp_file =~ s/DEVNUM/$devnum/g;
			$wp_file =~ s/SLOTNUM/$slot/g;
			$wp_file =~ s/BUSNUM/$bus/g;
			$wp_file =~ s/TDM_LAW/$tdmv_law/g;
			$wp_file =~ s/TDM_OPERMODE/$tdmv_opermode/g;
			$wp_file =~ s/HWECMODE/$hwec_mode/g;

			print FH $wp_file;
			close (FH); 
			
			print "\n created $fname for A$card_model $devnum SLOT $slot BUS $bus HWEC $hwec_mode\n";
		 	$startup_string.="wanpipe$devnum ";
		}
	}
	print FH $wp_file;
	close (FH); 
}

close SCR;


my $rcfile="";
open (FH,"/etc/wanpipe/samples/trixbox/templates/rc.template");
while (<FH>) {
        $rcfile .= $_;
}
close (FH);


open (FH,">/etc/wanpipe/samples/trixbox/cfg/wanrouter.rc");
$rcfile =~ s/WPSTARTUP/$startup_string/g;
print FH $rcfile;
close (FH);


print "\nWarning: This script only supports Sangoma analog cards, \n";
print "Use the Wancfg Utility to configure T1/E1 cards \n\n";	

if($devnum==0){
	print("\n\nWarning: No Sangoma analog cards found\n"); 
	exit 0;
}

@error=`rm -f /etc/wanpipe/wanpipe?.conf`;
print "@error";
@error=`cp -f /etc/wanpipe/samples/trixbox/cfg/* /etc/wanpipe`;
print "@error";
@error=`wanrouter start`;
print "@error";
#@error= `cp -f wanpipe_zaptel_start /etc/wanpipe/scripts/start` ;
#print "@error";
exit 0;
                  
