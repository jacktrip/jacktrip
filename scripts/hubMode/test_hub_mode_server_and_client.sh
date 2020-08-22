#!/bin/bash
# test_hub_mode_server_and_client.sh
# bash script for automatic testing of jacktrip in hub mode , Chris Chafe
# connects a hub client to a hub server (started with with -p1) on the same host
# prints avg audio RTT after 8 sec, or -1 if fail 
# ./test_hub_mode_server_and_client.sh <path-to-executable> <FPP>
# requires 3 helper scripts
# -- startJacktripHubServer.sh
# -- startJacktripHubClient.sh
# -- art.sh
# requires jackd be available on the host
# uses the "dummy" interface, so no audio interface needed
# first does some jackd cleanup 
# takes approx. 20 sec to complete

# a passing test prints to the console
# [cc@localhost sh]$ ./test.sh /home/cc/jacktrip/builddir/jacktrip 32
# starting /home/cc/jacktrip/builddir/jacktrip hub mode test at 32 FPP
# starting hub client for server localhost
# calculate audio round trip
# 4

# a failed test prints to the console
# [cc@localhost functionTests]$ ./test.sh ~/jacktrip/builddir/jacktrip 32
# starting /home/cc/jacktrip/builddir/jacktrip hub mode test at 32 FPP
# starting /home/cc/jacktrip/builddir/jacktrip server
# starting hub client of server running on localhost
# calculate audio round trip
# ERROR ..1:send_1 not a valid port
# ERROR ..1:send_2 not a valid port
# ERROR ..1:send_1 not a valid port
# ERROR ..1:receive_1 not a valid port
# jacktrip: no process found
# -1


JACKTRIP=$1

if [ -z "$2" ]
  then
    FPP=128
  else
    FPP=$2
fi

# killall jackd
if [ "$(ps -aux | grep -c jackd)" != 1 ]; then killall jackd; fi;
# if jackd is or has been running with another driver
# much experimenation shows it literally takes this long
sleep 17
# to flush old connections before starting the dummy driver

# start jack with dummy driver, or change to an audio interface by switching these lines
$( /usr/bin/jackd  -ddummy -r48000 -p$FPP > /dev/null 2>&1 & )
# $( /usr/bin/jackd  -dalsa -dhw:A96 -r48000 -p$FPP -n2  > /dev/null 2>&1 & )
# $( /usr/bin/jackd  -dalsa -dhw:PCH -r48000 -p$FPP -n2  > /dev/null 2>&1 & )

sleep 1

echo starting $JACKTRIP hub mode test at $FPP FPP

$PWD/startJacktripHubServer.sh $JACKTRIP
sleep 1
$PWD/startJacktripHubClient.sh $JACKTRIP

sleep 1

# start measuring audio RTT
$PWD/art.sh

