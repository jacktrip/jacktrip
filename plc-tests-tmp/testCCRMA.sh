for (( c=1; c<=5; c++ ))
do  
    echo "starting netem cycle $c on"
PIPEWIRE_LATENCY=32/48000 ./jacktrip -C jackloop32.stanford.edu --bufstrategy 3 -q200 --nojackportsconnect --udprt  -I 1 -G iostat.log 2> xxx.dat& 
    sleep 1
# sudo tc qdisc add dev lo root netem delay 1.0ms 0.4ms 25%
    jmess -c ~/ccrmaJACK.xml
    sleep 40
# sudo tc qdisc del dev lo root netem delay 1.0ms 0.4ms 25%
    killall jacktrip
sleep 1
    gnuplot --persist ccrma.gp
    rm xxx.dat
done



