#!/usr/bin/perl

 # This script is a small tool that can be used to update $CVSROOT/passwd
 # files, by ommiting pairs of username:password (with password being the
 # encrypted form of the second command line argument.

 &usage unless (defined($user = $ARGV[0]));
 &usage unless (defined($pass = $ARGV[1]));
 &usage unless (defined($suser = $ARGV[2]));

 $salt = join '', ('.', '/', 0..9, 'A'..'Z',
                'a'..'z')[rand 64, rand 64];
 $crypted = crypt($pass, $salt);
 print "$user:$crypted:$suser\n";

 sub usage {
     die "usage: cvspasswd USER PASS SERVERSIDE-USER\n";
 }
