#!/bin/bash
# test_hub_mode_server_and_clientPipeWire.sh <JACKTRIP> <FPP> <JACKD>

# bash script for automatic testing of jacktrip in hub mode, Chris Chafe
# connects a hub client to a hub server (started with with -p1) on the same host

# to run jackd automatically, set JACKD to anything (e.g., 1)
#    for example, for CI testing
# prints avg audio RTT after about 20 sec, or -1 if fail 

# to not run jackd automatically, leave off the 3rd argument
#     for example, if starting jackd manually with qjackctl or using PipeWire
# prints avg audio RTT after 8 sec, or -1 if fail 

# requires jack_iodelay be installed and available from system

# # requires 3 helper scripts in the same directory
# -- startJacktripHubServerPipeWire.sh
# -- startJacktripHubClientPipeWire.sh
# -- artPipeWire.sh

# example session:

# [cc@localhost sh]$ ./test_hub_mode_server_and_clientPipeWire.sh /home/cc/jacktrip/builddir/jacktrip 32
# starting /home/cc/jacktrip/builddir/jacktrip hub mode test at 32 FPP
# starting hub client for server localhost
# calculate audio round trip
# 4

# a failed test prints to the console
# -1

JACKTRIP=$1
FPP=$2

if [ -z "$3" ]
  then
    JACKD=0
  else
    JACKD=$3
fi

echo starting $JACKTRIP hub mode test at $FPP FPP with JACKD = $JACKD

if [ $JACKD != 0 ]
  then
    echo starting JACKD
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
fi

$PWD/startJacktripHubServerPipeWire.sh $JACKTRIP $FPP
sleep 1
$PWD/startJacktripHubClientPipeWire.sh $JACKTRIP $FPP

sleep 1

# start measuring audio RTT
$PWD/artPipeWire.sh

