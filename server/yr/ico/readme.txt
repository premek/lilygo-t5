downloaded from https://api.met.no/weatherapi/weathericon/2.0/documentation

converted to b&w: 
   for F in *.png; do convert -threshold 200 -negate $F PNG8:../png4/$F; done
   optipng -strip all *.png

licensed under the MIT License (c) Yr
