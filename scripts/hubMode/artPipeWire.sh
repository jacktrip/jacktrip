#!/bin/bash
# bash script for jacktrip automation, Chris Chafe
# artPipeWire.sh <FPP>

FPP=$1

echo calculate audio round trip
rm /tmp/art.dat
$(PIPEWIRE_LATENCY="$FPP/48000" jack_iodelay > /tmp/art.dat 2>&1 & )

$(jmess -D)

# correct in main not in dev -- jack_connect JackTrip:receive_1 JackTrip:send_1
jack_connect __1:receive_1 __1:send_1
jack_connect jack_delay:out HUBCLIENT:send_1
jack_connect HUBCLIENT:receive_1 jack_delay:in

sleep 8
killall jack_iodelay

killall jacktrip

DEFAULTOUTPUT=-1
AWKOUTPUT=$(grep total  /tmp/art.dat | \
    awk '{ sum += $3; n++ } END { if (n > 0) print sum / n; }')
printf -v AWKWARDINT %.0f "$AWKOUTPUT"
if (($AWKWARDINT > 0 ))
  then
    echo $AWKOUTPUT
    exit 0
  else
    echo $DEFAULTOUTPUT
    exit 1
fi


