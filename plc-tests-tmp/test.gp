set yrange[0:4];
plot 'iostat.log' u  1:2 w l, 'iostat.log' u  1:3 w l,'iostat.log' u  1:4 w l, 'iostat.log' u  1:5 w l, 'iostat.log' u  1:($7/100.0), 'iostat.log' u  1:($9/100.0)

