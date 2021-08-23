PIPEWIRE_LATENCY=32/48000 ./jacktrip -S --bufstrategy 3 -q19 --nojackportsconnect --udprt &
sleep 1
PIPEWIRE_LATENCY=32/48000 ./jacktrip -C localhost --bufstrategy 3 -q20 --nojackportsconnect --udprt  -I 1 -G iostat.log &
sleep 4
#sudo tc qdisc add dev lo root netem delay 1.0ms 0.4ms 25%
sudo tc qdisc add dev lo root netem delay 1.0ms 0.2ms distribution pareto
#sudo tc qdisc add dev lo root netem loss 5%
sleep 4
#sudo tc qdisc del dev lo root netem delay 1.0ms 0.4ms 25%
sudo tc qdisc del dev lo root netem delay 1.0ms 0.2ms distribution pareto
#sudo tc qdisc del dev lo root netem loss 5%
sleep 6
killall jacktrip
gnuplot --persist test.gp

