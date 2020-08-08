#!/bin/bash
#tests the jacktrip client connection to a hub server

#kills existing jackd process, assuming current user has perms
if pgrep -x "jackd"; then
  echo 'stopping jackd'
  command killall jackd
  sleep 5
fi

#kills existing jacktrip processes, assuming current user has perms
if pgrep -x "jacktrip"; then
  echo 'stopping jacktrip'
  command killall jacktrip
  sleep 15
fi

#start jackd
gnome-terminal -- command jackd -dalsa -dhw:0 -r48000 -p1024 -n2 > /dev/null 2>&1
#command jacktrip -C 171.64.197.165 >> output.txt 2>&1 &
sleep 5
command jacktrip -C 171.64.197.165 > output.txt 2>&1 &
sleep 5
outputmsg=`cat output.txt`
echo "$outputmsg"

regExp='.*received connection.*'

if [[ "${outputmsg,,}" =~ $regExp ]]; then
  echo "Connection test passed!"
else
  echo "Connection test failed!"
fi

sleep 5
if pgrep -x "jackd"; then
  echo "killing jackd to clean up"
  command killall jackd
fi

if pgrep -x "jacktrip"; then
  echo "killing jacktrip to clean up"
  command killall jacktrip
fi
