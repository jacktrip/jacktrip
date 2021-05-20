#!/bin/bash
# test_hub_mode_server_and_clientPipeWire.sh <JACKTRIP> <FPP>
# bash script for automatic testing of jacktrip in hub mode , Chris Chafe
# connects a hub client to a hub server (started with with -p1) on the same host
# requires jack_iodelay and jmess systemwide

# prints avg audio RTT after 8 sec, or -1 if fail 
# # requires 3 helper scripts
# -- startJacktripHubServerPipeWire.sh
# -- startJacktripHubClientPipeWire.sh
# -- artPipeWire.sh

# takes approx. 20 sec to complete
# a passing test prints to the console

# [cc@localhost sh]$ ./test_hub_mode_server_and_clientPipeWire.sh /home/cc/jacktrip/builddir/jacktrip 32
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
FPP=$2

echo starting $JACKTRIP hub mode test at $FPP FPP

$PWD/startJacktripHubServerPipeWire.sh $JACKTRIP $FPP
sleep 1
$PWD/startJacktripHubClientPipeWire.sh $JACKTRIP $FPP

sleep 1

# start measuring audio RTT
$PWD/artPipeWire.sh

