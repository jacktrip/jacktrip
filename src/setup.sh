#!/bin/sh -fx
# set up for debugging:
#jackd -T -ndefault -dcoreaudio -r48000 -p1024 &
/usr/local/bin/jackdmp -T -ndefault -dcoreaudio -r48000 -p1024 &
sleep 2
./jacktrip -V -s -p4 &
echo 'in debugger, set args to <whatever you want plus> -c 127.0.0.1 -o10'
ps aux | grep jack
