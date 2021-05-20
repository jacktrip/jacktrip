#!/bin/bash
# bash script for jacktrip automation, Chris Chafe
# startJacktripHubServerPipeWire.sh <JACKTRIP> <FPP>

JACKTRIP=$1
FPP=$2

echo starting $JACKTRIP server

$(PIPEWIRE_LATENCY="$FPP/48000" $JACKTRIP -S -p1 --udprt  --bufstrategy 3 -q8 --rcvlag 1  > /dev/null 2>&1 & )




