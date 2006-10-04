#!/usr/bin/perl

use Net::FTP;

$file=shift @ARGV;
$ip=shift @ARGV;
$dir=shift @ARGV;

if (!defined($ip)){
$ip="192.168.1.80";
#      $ip="192.168.1.100";
	#$ip="192.168.1.80";
#	$ip="192.168.1.100";
	$ip="192.168.1.80";
	#$ip="192.168.1.100";
}

if (!defined($file)){
	#$file="A104_V18.BIN";
	$file="A108_0100_V21.BIN";
	#$file="A104d_0100_V18.BIN";
	#$file="A200_0040_V05.BIN";
	#$file="A101_V25.BIN";
	#$file="A104_SS7.BIN";
	#$file="AFT_RM_V01.BIN";
	#$file="A101_V25.BIN";
	#$file="A200_0040_V04.BIN";
#       $file="A101_V25.BIN";
}

if (!defined($dir)){
	$dir="Engineering/HARDWARE/MF_production/HW_programming";
	;
}

$ftp = Net::FTP->new($ip, Debug => 0);

$ftp->login("anonymous",'me@here.there' ) || die "\nFailed to login!\n\n";
#$ftp->login("ncorbic",'nenad23') || die "\nFailed to login!\n\n";

$ftp->binary;  

if (defined($dir)){
	$ftp->cwd($dir);
}

$ftp->get($file) || die "\nFailed to get $file from $ip@$dir\n\n";
$ftp->quit;               

print "\nFile $dir/$file received from $ip\n\n";

exit (0);


sub usage()
{

print "

Usage:

 ftp_get.pl <ip> <filename> [ directory ]

"

}
