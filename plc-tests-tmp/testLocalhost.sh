for (( c=1; c<=1; c++ ))
do  
    echo "starting netem cycle $c on"
PIPEWIRE_LATENCY=32/48000 ./jacktrip -S --bufstrategy 3 -q30 --nojackportsconnect --udprt  2> yyy.dat& 
PIPEWIRE_LATENCY=32/48000 ./jacktrip -C localhost -B4465 --bufstrategy 3 -q30 --nojackportsconnect --udprt  -I 1 -G iostat.log 2> xxx.dat& 
    sleep 1
#     jmess -c ~/localhostJACK.xml
      jmess -c ~/localhostPW.xml
     sleep 4
# sudo tc qdisc add dev lo root netem delay 1.0ms 0.2ms distribution pareto
# sudo tc qdisc add dev lo root netem slot distribution pareto 1.0ms 0.8ms
sudo tc qdisc add dev lo root netem loss 10%
     sleep 80
# sudo tc qdisc del dev lo root netem delay 1.0ms 0.2ms distribution pareto
# sudo tc qdisc del dev lo root netem slot distribution pareto 1.0ms 0.8ms
sudo tc qdisc del dev lo root netem loss 10%
    killall jacktrip
sleep 1
    tail  -n +3 yyy.dat > zzz.dat
    gnuplot --persist localhost.gp
#    rm xxx.dat
    rm yyy.dat
done



