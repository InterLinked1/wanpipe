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
# Jan 5,   2007  David Yat Sin  v2.1 fix for analog cards inserted in wrong
#				context
# Dec 12,  2006  David Yat Sin  v2.0 support for T1/E1 cards
# Oct 13,  2006  David Yat Sin  Added --opermode and --law option
# Oct 12,  2006  David Yat Sin  Initial version
# ============================================================================

print "\n###################################################################";
print "\n#                                                                 #";
print "\n#                Sangoma Configuration Script                     #";
print "\n#                                                                 #";
print "\n#                            v2.1                                 #";
print "\n#                                                                 #";
print "\n#                  Sangoma Technologies Inc.                      #";
print "\n#                     Copyright(c) 2006.                          #";
print "\n#                                                                 #";
print "\n###################################################################\n\n";

use Switch;
use strict;
#use warnings;
#use diagnostics;
use Card;
use A10x;
use A10u;
use A20x;


#remove existing configuration files
my @error=`rm -f /etc/wanpipe/samples/trixbox/cfg/*`;
print "@error";
my $startup_string="";

sub prompt_user{
	my($promptString, $defaultValue) = @_;
	if ($defaultValue) {
	      print $promptString, "[", $defaultValue, "]: ";
	} else {
	      print $promptString, ": ";
	}

	$| = 1;               # force a flush after our print
	$_ = <STDIN>;         # get the input from STDIN (presumably the keyboard)
	chomp;
	if ("$defaultValue") {
	      return $_ ? $_ : $defaultValue;    # return $_ if it has a value
	} else {
	      return $_;
	}
}

sub prompt_user_list{
	my @list = @_;
	my $i;
	my $valid = 0;
	for $i (0..$#list) {
         	printf(" %s\. %s\n",$i+1, @list[$i]);
	}
	while ($valid == 0){
		my $list_sz = $#list+1;
		print "[1..$list_sz]:";	
		$| = 1;               # force a flush after our print
       	 	$_ = <STDIN>;         # get the input from STDIN (presumably the keyboard)
		chomp;
		if ( $_ =~ /(\d+)/ ){
			if ( $1 > $#list+1) {
	       	       	  	print "Invalid option: Input a value between 1 and $list_sz \n";
			} else {
				print "\n";
		       		return @list[$1-1] ;
			}
		} else {
			print "Invalid option: Input an integer\n";
		}
	}
}

sub get_pri_switchtype {
	my @options = ( "National ISDN 2", "Nortel DMS100", "AT&T 4ESS", "Lucent 5ESS", "EuroISDN", "Old National ISDN 1", "Q.SIG" );
	my @options_val = ( "national", "dms100", "4ess", "5ess", "euroisdn", "ni1", "qsig" );
	my $res = &prompt_user_list(@options);

	my $i;
	for $i (0..$#options){
		if ( $res eq @options[$i] ){
	       		return @options_val[$i]; 
		
		}
	}	
	print "Error: Invalid PRI switchtype\n";
	exit 1;
}

sub add_zap_channel {
	my ($channel, $type, $zaptel_conf_tmp, $zapata_conf_tmp) = @_;
	my $zp_file='';
	open(FH, "$zaptel_conf_tmp") or die "cannot open $zaptel_conf_tmp";
	while (<FH>) {
		$zp_file .= $_;
	}
	close (FH);
	if ( $type eq 'fxs'){
		$zp_file.="fxsks=$channel\n";
	}else{
		$zp_file.="fxoks=$channel\n";
	}

        open(FH, ">$zaptel_conf_tmp") or die "Can't open $zaptel_conf_tmp";
        print FH $zp_file;
        close (FH);

	my $zp_file='';
	open(FH, "$zapata_conf_tmp") or die "cannot open $zapata_conf_tmp";
      	while (<FH>) {
       		$zp_file .= $_;
	}
	close (FH);
	if ( $type eq 'fxo'){
		 $zp_file.="context = from-internal\n";
		 $zp_file.="group = 1\n";
		 $zp_file.="signalling = fxo_ks\n";
		 $zp_file.="channel => $channel\n\n";	 
	}else{
        	$zp_file.="context = from-zaptel\n";
		$zp_file.="group = 0\n";
	        $zp_file.="signalling = fxs_ks\n";
	        $zp_file.="channel => $channel\n\n";
	}

        
	open(FH, ">$zapata_conf_tmp") or die "Can't open $zapata_conf_tmp";
        print FH $zp_file;
        close (FH);
	
}
sub unload_zap_modules{
	my @options = ("ztdummy","wctdm","wcfxo","wcte11xp","wct1xxp","wct4xxp","tor2","zaptel");
	foreach my $module (@options) {
		@error=`modprobe -r $module`;
		print "@error\n";
	}
}

sub apply_changes{
	my $startyp_string = @_;
	print "Would you like to apply changes?\n";
	print "Warning: This will restart Asterisk \n";
	if (&prompt_user_list(("YES","NO")) eq 'NO'){
		print "No changes made to your configuration files\n";
		exit 0;
	}

	print "Your original configuration files have been \n";
	print "saved to:\n\n";
	print "1. /etc/zaptel.conf.bak \n";
	print "2. /etc/asterisk/zapata-auto.conf.bak \n";


	@error=`amportal stop`;
	print "@error";

	@error=`wanrouter stop`;
	print "@error";
	unload_zap_modules();

	@error=`rm -f /etc/wanpipe/wanpipe?.conf`;
	print "@error";
	
	#update wanpipe startup sequence
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

	@error=`cp -f /etc/wanpipe/samples/trixbox/cfg/* /etc/wanpipe`;
	print "@error";
	@error=`cp -f /etc/wanpipe/samples/trixbox/cfg/zaptel.conf /etc/zaptel.conf`;
	print "@error";
	@error=`cp -f /etc/wanpipe/samples/trixbox/cfg/zapata-auto.conf /etc/asterisk/zapata-auto.conf`;
	print "@error";

	@error=`wanrouter start 1>&2`;
	print "@error";

	@error=`/etc/init.d/zaptel start`;
	print "@error";

	@error=`/usr/sbin/amportal start 1>&2`;
	print "@error";
	exit 0;
}

print "\nWarning: This script provides basic configuration options, \n";
print "Use the Wancfg Utility for more advanced options \n\n";	

@error=`cp -f /etc/zaptel.conf /etc/zaptel.conf.bak`;
print "@error";

@error=`cp -f /etc/asterisk/zapata-auto.conf /etc/asterisk/zapata-auto.conf.bak`;
print "@error";


@error=`cp -f /etc/wanpipe/samples/trixbox/templates/zaptel.conf /etc/wanpipe/samples/trixbox/cfg/zaptel.conf`;
print "@error";

@error=`cp -f /etc/wanpipe/samples/trixbox/templates/zapata-auto.conf /etc/wanpipe/samples/trixbox/cfg/zapata-auto.conf`;
print "@error";



my $devnum=1;
my $current_zap_channel=1;
my $zaptel_conf_file='/etc/wanpipe/samples/trixbox/cfg/zaptel.conf';
my $zapata_conf_file='/etc/wanpipe/samples/trixbox/cfg/zapata-auto.conf';

my @hwprobe=`wanrouter hwprobe verbose`;
foreach my $dev (@hwprobe) {
	if ( $dev =~ /(\d+).(\w+\w+).*SLOT=(\d+).*BUS=(\d+).*CPU=(\w+).*PORT=(\w+).*HWEC=(\d+)/){
		my $card = eval {new Card(); } or die ($@);
                $card->zaptel_file($zaptel_conf_file);
                $card->zapata_file($zapata_conf_file);
		$card->span_no($devnum);
		$card->card_model($1);
		$card->pci_slot($3);
		$card->pci_bus($4);
		$card->fe_cpu($5);
		my $hwec=$7;
		if ($hwec==0){
			$card->hwec_mode('NO');
		}
		else{
			$card->hwec_mode('YES');
		}
       		if ($card->card_model eq '200' | $card->card_model eq '400'){
			print "A$1 detected and configured [slot:$3 bus:$4 span:$devnum]\n\n";
			$startup_string.="wanpipe$devnum ";
			
               		my $a20x = eval {new A20x(); } or die ($@);
			$a20x->card($card);
			$card->first_chan($current_zap_channel);
       			$current_zap_channel+=24;
			my $i;
			$a20x->gen_wanpipe_conf();
			$a20x->gen_zaptel_conf();	
			$a20x->gen_zapata_conf();	
			$devnum++;
		}
	}elsif ( $dev =~ /(\d+):FXS/){
	       my $zap_pos = $1+$current_zap_channel-25;
	       #fxo modules use fxs signalling
	       add_zap_channel($zap_pos, 'fxo', $zaptel_conf_file, $zapata_conf_file);
	}elsif ( $dev =~ /(\d+):FXO/){
	       my $zap_pos = $1+$current_zap_channel-25;
	       add_zap_channel($zap_pos, 'fxs', $zaptel_conf_file, $zapata_conf_file);
	}
}

if($devnum==1){
	print("No Sangoma analog cards found\n\n"); 
}
	
foreach my $dev (@hwprobe) {
	if ( $dev =~ /(\d+).(\w+\w+).*SLOT=(\d+).*BUS=(\d+).*CPU=(\w+).*PORT=(\w+).*HWEC=(\d+)/){
		my $card = eval {new Card(); } or die ($@);
                $card->zaptel_file($zaptel_conf_file);
                $card->zapata_file($zapata_conf_file);
		$card->span_no($devnum);
		$card->card_model($1);
		$card->pci_slot($3);
		$card->pci_bus($4);
		$card->fe_cpu($5);
		my $hwec=$7;
		if ($hwec==0){
			$card->hwec_mode('NO');
		}
		else{
			$card->hwec_mode('YES');
		}
		if ( $card->card_model eq '101' |  $card->card_model eq '102' |  $card->card_model eq '104' |  $card->card_model eq '108' ){
			if( $6 eq '1' ){
                        	print "A$1 detected on slot:$3 bus:$4\n";
			}
			printf ("Select media type for %s on port %s [slot:%s bus:%s span:$devnum]\n", $card->card_model, $6, $card->pci_slot, $card->pci_bus);
			my @options = ("T1", "E1", "Unused", "Exit");
			my $fe_media = &prompt_user_list(@options);
			if ( $fe_media eq 'Exit'){
				if ( $devnum != 1){
				       	apply_changes($startup_string);
				}
				exit 0;
			}  
			if ( $fe_media eq 'Unused' ){
				print "Port $6 not configured\n\n";
			} else {
				$startup_string.="wanpipe$devnum ";
				my $a10x = eval {new A10x(); } or die ($@);
				$a10x->card($card);
				$a10x->fe_line($6);
				$card->first_chan($current_zap_channel);
				$a10x->fe_media($fe_media);
				if ( $fe_media eq 'T1' ){
				      	$current_zap_channel+=24;
				}else{
                               	 	$a10x->fe_lcode('HDB3');
					printf ("Select framing type for %s on port %s\n", $card->card_model, $6);
					my @options = ("CRC4","NCRC4");
					$a10x->fe_frame(&prompt_user_list(@options));
				      	$current_zap_channel+=31;
				}
                              	my @options = ("NORMAL", "MASTER");
				printf ("Select clock for %s on port %s \n", $card->card_model, $6);
				$a10x->fe_clock(&prompt_user_list(@options));
				
				@options = ("PRI CPE", "PRI NET", "E & M", "E & M Wink", "FXS - Loop Start", "FXS - Ground Start", "FXS - Kewl Start", "FX0 - Loop Start", "FX0 - Ground Start", "FX0 - Kewl Start");
				printf ("Select signalling type for %s on port %s [slot:%s bus:%s span:$devnum]\n", $card->card_model, $6, $card->pci_slot, $card->pci_bus);
				$a10x->signalling(&prompt_user_list(@options));

				if ( $a10x->signalling eq 'PRI CPE' | $a10x->signalling eq 'PRI NET'){
					printf ("Select switchtype for %s on port %s \n", $card->card_model, $6);
					$a10x->pri_switchtype(get_pri_switchtype());
				}
				
				@options = ("PSTN", "INTERNAL");
				printf ("Select dialplan context for %s on port %s\n", $card->card_model, $6);
				$card->zap_context(&prompt_user_list(@options));
				$a10x->gen_wanpipe_conf();
				$a10x->gen_zaptel_conf();	
				$a10x->gen_zapata_conf();	
				$devnum++;
			}

		}
	}elsif ( $dev =~ /A(\d+)u.*SLOT=(\d+).*BUS=(\d+).*CPU=(\w+)/){
		my $card = eval {new Card(); } or die ($@);
                $card->zaptel_file($zaptel_conf_file);
                $card->zapata_file($zapata_conf_file);
		$card->span_no($devnum);
		$card->card_model($1);
		$card->pci_slot($2);
		$card->pci_bus($3);
		$card->fe_cpu($4);
			
		printf ("Select media type for %s on port %s [slot:%s bus:%s span:$devnum]\n", $card->card_model, $6, $card->pci_slot, $card->pci_bus);
		my @options = ("T1", "E1", "Unused", "Exit");
		my $fe_media = &prompt_user_list(@options);
		if ( $fe_media eq 'Exit'){
			if ( $devnum != 1){
			       	apply_changes();
			}
			exit 0;
		}  
		if ( $fe_media eq 'Unused' ){
			print "Port $4 not configured\n\n";
		} else {
			$startup_string.="wanpipe$devnum ";
			$devnum++;
			my $a10u = eval {new A10u(); } or die ($@);
			$a10u->card($card);
			$card->first_chan($current_zap_channel);
			if( $4 eq 'A' ){
                       		print "Sangoma single/dual T1/E1 card detected\n";
		       		print "Select configuration options:\n\n";
				$a10u->fe_line('1');
			}else{
				$a10u->fe_line('2');
			}
			$a10u->fe_media($fe_media);
			if ( $fe_media eq 'T1' ){
			      	$current_zap_channel+=24;
			}else{
                        	$a10u->fe_lcode('HDB3');
				printf ("Select framing type for %s on port %s\n", $card->card_model, $6);
				my @options = ("CRC4","NCRC4");
				$a10u->fe_frame(&prompt_user_list(@options));
				$current_zap_channel+=31;
			}
                      	my @options = ("NORMAL", "MASTER");
			printf ("Select clock type for %s on port %s\n", $card->card_model, $a10u->fe_line);
			$a10u->fe_clock(&prompt_user_list(@options));
			
			@options = ("PRI CPE", "PRI NET", "E & M", "E & M Wink", "FXS - Loop Start", "FXS - Ground Start", "FXS - Kewl Start", "FX0 - Loop Start", "FX0 - Ground Start", "FX0 - Kewl Start");
			printf ("Select signalling type for %s on port %s [slot:%s bus:%s span:$devnum]\n", $card->card_model, $6, $card->pci_slot, $card->pci_bus);
			$a10u->signalling(&prompt_user_list(@options));

			@options = ("PSTN", "INTERNAL");
			printf ("Select dialplan context for %s on port %s\n", $card->card_model, $a10u->fe_line);
			$card->zap_context(&prompt_user_list(@options));
			$a10u->gen_wanpipe_conf();
			$a10u->gen_zaptel_conf();	
			$a10u->gen_zapata_conf();	
		}
	}
}

close SCR;


if($devnum==1){
	print("\n\nWarning: No Sangoma cards found\n"); 
	exit 0;
}
apply_changes();

