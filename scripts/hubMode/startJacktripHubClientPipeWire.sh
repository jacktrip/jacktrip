#!/bin/bash
# bash script for jacktrip automation, Chris Chafe
# startJacktripHubClientPipeWire.sh <JACKTRIP> <FPP>
# 
# /home/cc/Desktop/sh/startJacktripHubClient.sh /home/cc/jacktrip/builddir/jacktrip %p

JACKTRIP=$1

### qjackctl start / stop example
## qjackctl Setup : Options : Execute scrpt after Startup
# /home/cc/Desktop/sh/startJacktripHubClient.sh %p
## qjackctl Setup : Options : Execute scrpt on Shutdown
# /home/cc/Desktop/sh/stopJacktrip.sh
## timing of scrpt call means jacktrip is still running and 
## an alert will appear for each new server it tries to shutdown
## for both <scripts> do, chmod +x <script>.sh

### manual start example
# ./jacktrip -C jackloop128.stanford.edu

### automatic client start in qjackctl : Setup : Options Execute script after Startup
### examples with line used in qjackctl and corresponding script
## specify full server name
# /home/cc/startJacktrip.sh jackloop128.stanford.edu
# SERVER=localhost

## server name from file
# /home/cc/startJacktrip.sh jackloop1024.stanford.edu
# SERVER=$(cat ../../server.txt)

## composed server name, %p = fpp
# /home/cc/startJacktrip.sh %p


FPP=$2
SERVER=localhost

echo starting hub client of a server running on $SERVER

$(PIPEWIRE_LATENCY="$FPP/48000" $JACKTRIP -C $SERVER -J HUBCLIENT --udprt --bufstrategy 3 -q8 --rcvlag 1 > /dev/null 2>&1 & )



