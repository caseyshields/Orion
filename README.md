# Orion

Star hunting software. An interactive command line tool to search the FK6 star catalog by brightness and current sky 
position. A TCN sensor can then be concurrently directed at the selected star.

```commandline
Loading IERS Bulletin "../data/iers/finals2000A.data"... 1992/01/01 00:00:00.000 to 2019/08/31 00:00:00.000
loading FK6 I metadata "../data/fk6/ReadMe"... 93 fields loaded.
loading FK6 III metadata "../data/fk6/ReadMe"... 56 fields loaded.
loading catalog "../data/fk6/fk6_1.dat"... 878 entries total.
loading catalog "../data/fk6/fk6_1.dat"... 4150 entries total.

2018/09/26 23:43:55.503 : time 2000/1/1 12:0:0
Time
	UTC:	2000/01/01 12:00:00.000	(2451545.000000)
	TT:	2000/01/01 12:00:01.150	(2451545.000013)
	UT1:	2000/01/01 12:00:00.355	(2451545.000004)
	dT:	68.829399	measured
Earth Orientation
	MJD:	2000/01/02 00:00:00.000	(2451545.500000)
	pm_x:	0.043541	(e=0.000058)	measured
	pm_y:	0.377638	(e=0.000073)	measured
	ut1_utc:	0.354601	(e=0.000012)	measured

2000/01/01 12:00:00.000 : location
Location
	longitude:	115 7' 3.000000" W	(-115.117500)
	latitude:	36 11' 56.040000" N	(36.198900)
	height:	0.000000 meters

2000/01/01 12:00:00.000 : weather
Atmosphere
	temperature:	 10.0°C
	pressure:	1010.000 millibars

2000/01/01 12:00:00.000 : name alpha
FK6.35: alpha Scl (ra:0.976767, dec:-29.357453, v=4.300000)
FK6.74: alpha Ari (ra:2.119558, dec:23.462420, v=2.010000)
FK6.107: alpha Cet (ra:3.037993, dec:4.089739, v=2.540000)
FK6.120: alpha Per (ra:3.405381, dec:49.861182, v=1.790000)
FK6.155: alpha Hor (ra:4.233365, dec:-42.294368, v=3.850000)
FK6.239: alpha Men (ra:6.170687, dec:-74.753045, v=5.080000)
FK6.245: alpha Car (ra:6.399197, dec:-52.695661, v=-0.620000)
FK6.293: alpha Mon (ra:7.687453, dec:-9.551130, v=3.940000)
FK6.327: alpha Pyx (ra:8.726538, dec:-33.186385, v=3.680000)
FK6.343: alpha Vol (ra:9.040777, dec:-66.396075, v=4.000000)
FK6.352: alpha Lyn (ra:9.350917, dec:34.392568, v=3.140000)
FK6.354: alpha Hya (ra:9.459790, dec:-8.658601, v=1.990000)
FK6.392: alpha Ant (ra:10.452528, dec:-31.067778, v=4.280000)
FK6.1283: alpha Crt (ra:10.996240, dec:-18.298784, v=4.080000)
FK6.542: alpha Aps (ra:14.797697, dec:-79.044751, v=3.830000)
FK6.625: alpha TrA (ra:16.811082, dec:-69.027717, v=1.910000)
FK6.691: alpha Tel (ra:18.449560, dec:-45.968459, v=3.490000)
FK6.1482: alpha Sct (ra:18.586785, dec:-8.244070, v=3.850000)
FK6.699: alpha Lyr (ra:18.615649, dec:38.783690, v=0.030000)
FK6.718: alpha CrA (ra:19.157873, dec:-37.904474, v=4.110000)
FK6.728: alpha Sgr (ra:19.398104, dec:-40.615939, v=3.960000)
FK6.1508: alpha Vul (ra:19.478425, dec:24.664906, v=4.440000)
FK6.777: alpha Cyg (ra:20.690532, dec:45.280337, v=1.250000)
FK6.803: alpha Cep (ra:21.309659, dec:62.585575, v=2.450000)
FK6.827: alpha Aqr (ra:22.096399, dec:-0.319849, v=2.950000)
FK6.871: alpha Peg (ra:23.079348, dec:15.205267, v=2.490000)

2000/01/01 12:00:00.000 : target 245
Catalog Entry
	Catalog designation:	FK6.245
	Flamsteed/Bayer designation:	alpha Car
	right ascension:	6 23' 57.109295"	(6.399197 hours)
	declination:	-52 41' 44.381170"	(-52.695661 degrees)
	proper motion:	(19.37 ra, 23.27 dec) mas/year
	parallax:	10.430000 milliarcseconds
	visual magnitude:	-0.620000
	radial velocity:	20.500000 km/s

2000/01/01 12:00:00.000 : connect 127.0.0.1:43210
Sensor
	ip:	127.0.0.1
	port:	43210

2000/01/01 12:00:00.000 : target 699
Catalog Entry
	Catalog designation:	FK6.699
	Flamsteed/Bayer designation:	alpha Lyr
	right ascension:	18 36' 56.336939"	(18.615649 hours)
	declination:	38 47' 1.283330"	(38.783690 degrees)
	proper motion:	(201.70 ra, 286.67 dec) mas/year
	parallax:	128.930000 milliarcseconds
	visual magnitude:	0.030000
	radial velocity:	-13.500000 km/s

2000/01/01 12:00:00.000 : report 3600 12
Location
	longitude:	115 7' 3.000000" W	(-115.117500)
	latitude:	36 11' 56.040000" N	(36.198900)
	height:	0.000000 meters
Atmosphere
	temperature:	 10.0°C
	pressure:	1010.000 millibars
Catalog Entry
	Catalog designation:	FK6.699
	Flamsteed/Bayer designation:	alpha Lyr
	right ascension:	18 36' 56.336939"	(18.615649 hours)
	declination:	38 47' 1.283330"	(38.783690 degrees)
	proper motion:	(201.70 ra, 286.67 dec) mas/year
	parallax:	128.930000 milliarcseconds
	visual magnitude:	0.030000
	radial velocity:	-13.500000 km/s
UTC	AZ	EL	E	F	G
2000/01/01 12:00:00.000	045.851441	006.746382	835746890	-19779689	673837816
2000/01/01 12:59:60.000	053.198115	015.914285	802842618	-235302997	673051063
2000/01/01 13:59:60.000	059.538597	026.000965	714608353	-435415348	672807461
2000/01/01 14:59:60.000	065.006012	036.735939	577319503	-605890366	672696483
2000/01/01 15:59:60.000	069.650448	047.931170	400432022	-734938974	672636903
2000/01/01 16:59:60.000	073.307028	059.439816	196082926	-813682044	672602795
2000/01/01 17:59:60.000	075.001496	071.126695	-21718666	-836708396	672583722
2000/01/01 18:59:60.000	066.657371	082.711137	-238045692	-802432869	672575083
2000/01/01 19:59:60.000	298.761832	084.273901	-438074710	-713200600	672574870
2000/01/01 20:59:60.000	285.112832	072.803686	-608098459	-575126146	672582845
2000/01/01 21:59:60.000	286.284935	061.104808	-736468741	-397672551	672600802
2000/01/01 22:59:60.000	289.757190	049.562061	-814392763	-193005433	672632763
2000/01/01 23:59:60.000	294.278559	038.313608	-836537816	24837501	672687955

2000/01/01 12:00:00.000 : help
Configuration
	time [YYYY/MM/DD HH:MM:SS.ssssss]
	location [<lat(deg)> <lon(deg)> <height(m)>]
	weather [<temp(C)> <pressure(mBar)>]

Catalog
	name <substr>
	search <mag> [<> <> <> <>(deg)]

TCN Sensor
	connect [ X.X.X.X[:Y] ]
	target [fk6id]

Diagnostic
	status
	report <step(sec)> <count>

	exit

<> : required	[] : optional	() : units

2000/01/01 12:00:00.000 : exit

Process finished with exit code 0

```

For more information on available commands, see the [doxygen pages](https://caseyshields.github.io/Orion/index.html).

## Design

Orion relies heavily on prior work by the [US Naval Observatory](http://www.usno.navy.mil/USNO/).
It was developed using [CLion](https://www.jetbrains.com/clion/) with the [Cygwin](https://www.cygwin.com/) and [MinGW](http://mingw.org/) toolchains.
Lightweight unit testing is done with [CuTest](http://cutest.sourceforge.net/).
Code is documented using [doxygen](http://www.doxygen.org), [Mermaid](https://mermaidjs.github.io/) and [PlantUML](http://plantuml.com/)
Results are tested against a [USNO web app](http://aa.usno.navy.mil/data/docs/topocentric.php).

![Component organization](https://caseyshields.github.io/Orion/diagrams/novas.svg)

 - **main** : A command line interface which allows the user to search the catalog and send targets to the orion server
 - **orion** : A server which drives a TATS sensor, directing it at given targets
 - **catalog** : module for filtering and sorting desired stars
 - **tracker** : performs coordinate transforms according to the current time and location on earth 
 - **sensor** : a dummy server which simulates a slaved sensor
 - **vmath** : utility library of vector and spherical math routines
 - **novasc3.1** : [USNO's astrometric software package](http://aa.usno.navy.mil/software/novas/novas_info.php)
 - **fk6** : loads raw [FK6](http://cdsarc.u-strasbg.fr/viz-bin/Cat?I/264) data into a catalog. Also has some compatibility with [FK5](http://www-kpno.kpno.noao.edu/Info/Caches/Catalogs/FK5/fk5.html) data. Project [page](http://wwwadd.zah.uni-heidelberg.de/datenbanken/fk6/index.php.de).
 - **tats** : provides structures and enumeration for dealing with TATS messages
 - **iers** : Load and search an IERS Bulletin A data file for date and polar offsets. Data files are obtained from [IERS](https://www.iers.org/IERS/EN/DataProducts/EarthOrientationData/eop.html). The human readable file can be obtained from [USNO](http://maia.usno.navy.mil/ser7/ser7.dat) as well.
   
## Future Plans
 - Add planet tracking by integrating JPL ephemeris
 - Add rest interface for catalog and tracker functionality using [libwebsockets](https://libwebsockets.org/)
 - Add web interface which utilizes [starmap](https://caseyshields.github.io/starlog/index.html) project to allow user interaction with the catalog
 - Add more Vizier catalogs like the Yale Bright Star [Catalog](http://tdc-www.harvard.edu/catalogs/bsc5.html)

## Links
 - [IERS Time Scale Converter](https://www.iers.org/IERS/EN/DataProducts/tools/timescales/timescales.html)
 - Special thanks to [George Kaplan](http://gkaplan.us/) for technical support.
 - Predictions of difference between Universal and Terrestrial time scales([DeltaT](https://www.usno.navy.mil/USNO/earth-orientation/eo-products/long-term))