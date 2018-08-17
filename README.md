# Orion

Star hunting software. An interactive command line tool to search the FK6 star catalog by brightness and current sky 
position. A TCN sensor can then be concurrently directed at the selected star.

```commandline
-ip 127.0.0.1
-port 43210
-ut1_utc 0.06809
-leap_secs 37.000000
-latitude 38.88972222222222
-longitude -77.0075
-height 125.0
-celsius 10.0
-millibars 1010.0

2018/08/17 22:55:49.298 : help
Configuration
	time <YYYY/MM/DD HH:MM:SS.ssssss>
	location <lat(deg)> <lon(deg)> <height(m)>
	weather <temp(C)> <pressure(mBar)>

Catalog
	name <substr>
	search <mag> [<> <> <> <>(deg)]

TCN Sensor
	connect [X.X.X.X:Y]
	track <fk6 id>

Diagnostic
	status
	report <step(sec)> <count>

	exit

<> : required	[] : optional	() : units

2018/08/17 22:55:49.298 : time 2000/1/1 12:0:0

2000/01/01 12:00:00.000 : name Lyr
FK6.684: Grb 2533 Lyr (ra:18.260772, dec:42.159344, p:6.040000, v=5.560000)
FK6.1477: kappa Lyr (ra:18.331030, dec:36.064547, p:13.710000, v=4.330000)
FK6.1483: Grb 2603 Lyr (ra:18.563240, dec:46.219152, p:3.720000, v=6.730000)
FK6.699: alpha Lyr (ra:18.615649, dec:38.783690, p:128.930000, v=0.030000)
FK6.1488: +26^o 3349 Lyr (ra:18.767911, dec:26.662130, p:12.960000, v=4.830000)
FK6.711: R Lyr (ra:18.922250, dec:43.946090, p:9.330000, v=4.080000)
FK6.1498: Pi 18 h 318 Lyr (ra:19.110482, dec:28.628595, p:24.500000, v=5.530000)
FK6.724: theta Lyr (ra:19.272804, dec:38.133734, p:4.240000, v=4.350000)
FK6.3463: mu Lyr (ra:18.403829, dec:39.507239, p:7.300000, v=5.110000)
FK6.3532: 19 Lyr (ra:19.196114, dec:31.283456, p:3.300000, v=5.930000)

2000/01/01 12:00:00.000 : target 699

2000/01/01 12:00:00.000 : status

2000/01/01 12:00:00.000 UTC (+0.068 UT1)
 38.889722 N, -77.007500 W,  125.0 m
 10.0°C 1.010 bar
FK6  699  56.9283°zd  64.1932°az 0.0v alpha Lyr
01 1 0001 00000 056.9280 064.1930 000.0000 0699 30 FBB1


2000/01/01 12:00:00.000 : report 60.0 5
latitude:	38.889722 hours
longitude:	-77.007500 degrees
elevation:	125.000000 meters
temperature:	10.000000 Celsius
pressure:	1010.000000 millibars

UTC	AZ	ZD
2000/01/01 12:00:00.000	056.928337	064.193241
2000/01/01 12:00:60.000	056.752768	064.295283
2000/01/01 12:01:60.000	056.577047	064.397158
2000/01/01 12:02:60.000	056.401174	064.498868
2000/01/01 12:03:60.000	056.225152	064.600413

2000/01/01 12:00:00.000 : connect 127.0.0.1:43210

2000/01/01 12:00:00.000 : target 684

2000/01/01 12:00:00.000 : exit
```

For more information on available commands, see the [doxygen pages](https://caseyshields.github.io/Orion/index.html).

## Design

Orion relies heavily on prior work by the [US Naval Observatory](http://www.usno.navy.mil/USNO/).
It was developed using [CLion](https://www.jetbrains.com/clion/) with the [Cygwin](https://www.cygwin.com/) and [MinGW](http://mingw.org/) toolchains.
Lightweight unit testing is done with [CuTest](http://cutest.sourceforge.net/).
Code is documented using [doxygen](http://www.doxygen.org), [Mermaid](https://mermaidjs.github.io/) and [PlantUML](http://plantuml.com/)

![Component organization](https://caseyshields.github.io/Orion/diagrams/novas.svg)

 - main : A command line interface which allows the user to search the catalog and send targets to the orion server
 - orion : A server which drives a TATS sensor, directing it at given targets
 - catalog : module for filtering and sorting desired stars
 - tracker : performs coordinate transforms according to the current time and location on earth 
 - simulator : a dummy server which simulates a slaved sensor
 - vmath : utility library of vector and spherical math routines
 - novasc3.1 : [USNO's astrometric software package](http://aa.usno.navy.mil/software/novas/novas_info.php)
 - fk6 : loads raw [FK6](http://cdsarc.u-strasbg.fr/viz-bin/Cat?I/264) data into a catalog. Also has some compatibility with [FK5](http://www-kpno.kpno.noao.edu/Info/Caches/Catalogs/FK5/fk5.html) data. Project [page](http://wwwadd.zah.uni-heidelberg.de/datenbanken/fk6/index.php.de).
 - tats : provides structures and enumeration for dealing with TATS messages

## Future Plans
 
  - Add planet tracking by integrating JPL ephemeris
  - Add [IERS-A bulletin](http://maia.usno.navy.mil/ser7/ser7.dat) projections to improve Novas accuraccy
  - Add rest interface for catalog and tracker functionality using [libwebsockets](https://libwebsockets.org/)
  - Add web interface which utilizes [starmap](https://caseyshields.github.io/starlog/index.html) project to allow user interaction with the catalog
  - Add more Vizier catalogs like the Yale Bright Star [Catalog](http://tdc-www.harvard.edu/catalogs/bsc5.html)
  