for (( c=1; c<=5; c++ ))
do  
    echo "starting netem cycle $c on"
PIPEWIRE_LATENCY=32/48000 ./jacktrip -S --bufstrategy 3 -q200 --nojackportsconnect --udprt  2> yyy.dat& 
sleep 1
PIPEWIRE_LATENCY=32/48000 ./jacktrip -C localhost -B4465 --bufstrategy 3 -q200 --nojackportsconnect --udprt  -I 1 -G iostat.log 2> xxx.dat& 
    sleep 1
# sudo tc qdisc add dev lo root netem delay 1.0ms 0.4ms 25%
#      jmess -c ~/localhostJACK.xml
    sleep 40
# sudo tc qdisc del dev lo root netem delay 1.0ms 0.4ms 25%
    killall jacktrip
sleep 1
    tail  -n +3 yyy.dat > zzz.dat
    gnuplot --persist localhost.gp
    rm xxx.dat
    rm yyy.dat
done



