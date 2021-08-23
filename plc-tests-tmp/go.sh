PIPEWIRE_LATENCY=32/48000 ./jacktrip -S --bufstrategy 3 -q19 --nojackportsconnect --udprt &
sleep 1
PIPEWIRE_LATENCY=32/48000 ./jacktrip -C localhost --bufstrategy 3 -q200 --nojackportsconnect --udprt  -I 1 -G iostat.log &
sleep 5
for (( c=1; c<=5; c++ ))
do  
    echo "starting netem cycle $c on"
    #sudo tc qdisc add dev lo root netem delay 1.0ms 0.4ms 25%
    sudo tc qdisc add dev lo root netem delay 1.0ms 0.2ms distribution pareto
    #sudo tc qdisc add dev lo root netem loss 5%
    sleep 5
    echo "starting netem cycle $c off"
    #sudo tc qdisc del dev lo root netem delay 1.0ms 0.4ms 25%
    sudo tc qdisc del dev lo root netem delay 1.0ms 0.2ms distribution pareto
    #sudo tc qdisc del dev lo root netem loss 5%
    sleep 5
done
killall jacktrip


