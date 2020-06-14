#!/bin/sh -fx
# unsetup for debugging:
ps aux | grep jack
killall jackdmp
killall jacktrip
ps aux | grep jack
echo 'debugger jacktrip must be stopped manually'
