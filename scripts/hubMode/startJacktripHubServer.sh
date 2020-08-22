#!/bin/bash
# bash script for jacktrip automation, Chris Chafe
# startJacktripHubServer.sh <FPP>

JACKTRIP=$1


echo starting $JACKTRIP server

$( $JACKTRIP -S -p1 > /dev/null 2>&1 & )




