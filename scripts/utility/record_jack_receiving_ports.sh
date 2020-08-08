#!/bin/bash

if ! pgrep -x "jackd" ; then
  printf "Please start jack with a 48K sample rate before running this script."
  exit
fi

IPs=($(jack_lsp | grep ":receive_*" | cut -d: -f1))
uniqIPs=($(printf "%s\n" "${IPs[@]}" | sort -u | tr '\n' ' '))

uniqLength=${#uniqIPs[@]}
printf '%s\n' "number of unique IP's: ${uniqLength}"

for IP in "${uniqIPs[@]}"
do
  printf '%s\n' "IP: ${IP}"
  NOW=$( date '+%F_%H:%M:%S' )
  ecasound -f:,1,48000 -i jack,${IP} -o rec_${IP}_${NOW}_output.wav &
done 

echo "Press ESC key to stop recording"
# read a single character
while read -r -n1 key
do
# if input == ESC key
  if [[ $key == $'\e' ]];then
  kill $(ps aux | grep '[e]casound' | awk '{print $2}') 
  break;
  fi
done

