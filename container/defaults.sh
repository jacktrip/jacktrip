#!/bin/sh

if [[ -z "$JACK_OPTS" ]]; then    
    if [[ -z "$SAMPLE_RATE" ]]; then
        SAMPLE_RATE=48000
    fi
    if [[ -z "$FRAMES_PER_PERIOD" ]]; then
        FRAMES_PER_PERIOD=128
    fi
    JACK_OPTS="-d dummy --rate $SAMPLE_RATE --period $FRAMES_PER_PERIOD"
fi

if [[ -z "$JACKTRIP_OPTS" ]]; then
    JACKTRIP_OPTS="-S -t -z --hubpatch 4 --bufstrategy 4 -q auto3"
fi

echo "JACK_OPTS=\"$JACK_OPTS\"" > /etc/default/jack
echo "JACKTRIP_OPTS=\"$JACKTRIP_OPTS\"" > /etc/default/jacktrip
