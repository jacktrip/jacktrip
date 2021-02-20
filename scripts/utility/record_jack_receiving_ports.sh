#!/bin/bash
#use this script to record jacktrip receive & broadcast channels on ecasound
#Dependency: Ecasound https://ecasound.seul.org/ecasound/README
#Make sure you have ecasound installed in the recording environment (server or client)
#Start JACK and JackTrip on the server or the client you want to record from
#IMPORTANT: Wait till all clients have connected
#cd to the folder where this script is located
#Start the script from the terminal: ./record_jack_receiving_ports.sh
#Press ESC to stop recording.
#Recording should appear as a .wav file in the same folder as the script

if ! pgrep -x "jackd" ; then
  printf "Please start jack with a 48K sample rate before running this script."
  exit
fi

IPs=($(jack_lsp | grep ':receive_*\|:broadcast_*' | cut -d: -f1))

for IP in "${IPs[@]}"
do
  printf '%s\n' "IP: ${IP}"
  NOW=$( date '+%F_%H:%M:%S' )
  ecasound -f:,4,48000 -i jack,${IP} -o rec_${IP}_${NOW}_output.wav &
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

