#!/bin/sh -fx
# set up for debugging:
jackdmp -T -ndefault -dcoreaudio -r48000 -p1024 &
sleep 2
jacktrip -s -p4 &
echo 'in debugger, set args to <whatever you want plus> -c 127.0.0.1 -o10
ps aux | grep jack
