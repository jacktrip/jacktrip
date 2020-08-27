echo calculate audio round trip
rm /tmp/art.dat
$( jack_iodelay > /tmp/art.dat 2>&1 & )
# jack_iodelay  &
sleep 1
# jack_connect jack_delay:out jackloop$1.stanford.edu:send_2
# jack_connect jackloop$1.stanford.edu:receive_2 jack_delay:in
jack_disconnect __1:receive_1 __1:send_1
jack_disconnect __1:receive_2 __1:send_2
# jack_disconnect localhost:receive_1 localhost:send_1
# jack_disconnect localhost:receive_2 localhost:send_2
jack_connect jack_delay:out __1:send_1
jack_connect __1:receive_1 jack_delay:in

sleep 8
killall jack_iodelay
sleep 1

killall jacktrip
killall jackd

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


