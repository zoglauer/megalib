#!/usr/bin/perl

 &usage unless (defined($user = $ARGV[0]));
 &usage unless (defined($pass = $ARGV[1]));

 $salt = join '', ('.', '/', 0..9, 'A'..'Z',
                'a'..'z')[rand 64, rand 64];
 $crypted = crypt($pass, $salt);
 print "$user:$crypted\n";

 sub usage {
     die "usage: perl p_small.pl USER PASS\n";
     die "  e.g. perl p_small.pl andreaszoglauer 7bh&g§j5t()"
 }
