# Orion

Star hunting software. Provides tools to search the FK6 star catalog by brightness and current sky 
position. TATS sensors can then be concurrently directed to target selected stars. Orion relies heavily 
on prior work by the [US Naval Observatory](http://www.usno.navy.mil/USNO/)

Orion is developed using [CLion](https://www.jetbrains.com/clion/) and the [MinGW](http://mingw.org/) toolchain. Code
is documented using [doxygen](http://www.doxygen.org), [Mermaid](https://mermaidjs.github.io/) and [PlantUML](http://plantuml.com/)

## Components

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

## Code documentation

See generated [doxygen pages](https://caseyshields.github.io/Orion/index.html) here.

## Future Plans
 
  - Improve portability by switching to [cygwin](https://www.cygwin.com/) for full posix compliance
  - Switch from winsock to sockets
  - Add planet tracking by integrating JPL ephemeris
  - Add [IERS-A bulletin](http://maia.usno.navy.mil/ser7/ser7.dat) projections to improve Novas accuraccy
  - Add rest interface for catalog and tracker functionality using [libwebsockets](https://libwebsockets.org/)
  - Add web interface which utilizes [starmap](https://caseyshields.github.io/starlog/index.html) project to allow user interaction with the catalog
  - Add more Vizier catalogs like the Yale Bright Star [Catalog](http://tdc-www.harvard.edu/catalogs/bsc5.html)
  