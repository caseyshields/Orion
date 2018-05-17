# Star Tracker

Software for pointing at stars. Contains a planning component for loading astrometric catalogs and selecting suitable stars.
The other component is a tracker which transforms catalog coordinates into a useable format and broadcasts them to client(s).
The [Novas 3.1](http://aa.usno.navy.mil/software/novas/novas_info.php) library is used to perform the transformations.
The first part of [FK5](http://www-kpno.kpno.noao.edu/Info/Caches/Catalogs/FK5/fk5.html) or [FK6](http://cdsarc.u-strasbg.fr/viz-bin/Cat?I/264) is currently used as a catalog.
The performance critical libraries are written in C, with an idiom similar to the underlying Novas libraries.
The front end is still in the conceptual phases, however a 
[UI experiment](https://caseyshields.github.io/starlog/index.html) has been written in [D3](https://d3js.org/).  

## Planner Questions

#### How is a viewing plan made and carried out?

**A: Interactive mode will be good to start with. A typical plan is to pick bright stars and alternate between them over 
the course of the night. Pre-selecting planned stars would be a nice feature, or even a schedule which can be 
interactively interrupted.**

- [ ] Is a plan built offline from a sequence of queries and later played back?
    - How do you control for the passage of time?
    - What format is the plan output to?
- [ ] Do we automatically generated a bright, distributed subset from the visible hemisphere?
    - how do we control for time beside setting a rigid viewing schedule?
- [X] Are targets interactively selected from zones of the current night sky?
    1. User (types in/clicks on) a patch of the current night sky
    2. System finds catalog stars in corresponding patch of sky
    3. Ranks stars by brightness and or astrometric quality
    4. Returns table of best candidates
    5. User clicks/types selection
    6. Tracker is told to start controlling for that star

#### What UI should it have?

**A: Command line to get started, Web app is a nice to have. It needs to be protable and run offline, so it has to carry 
it's catalog with it.**

- [ ] Command Line Application
    - simpler to develop but harder to visually check
- [ ] Web Application (Spark or Node)
    - requires recent browser and JVM or Node
    - Front-end visualization
    - [ ] D3 projections, topology and interactivity
        - I made an [FK6 chart](https://caseyshields.github.io/starlog/index.html) partially based on a script by Philip Plewa;
    - [ ] [Aladin Lite](http://aladin.u-strasbg.fr/AladinLite/)
        - can overlay catalogs on progressively rendered rasters 
        - needs to be on internet or special servers need to be replicated
- [ ] Something else?
   
#### How do we group and rank stars?

**A: star number is sufficient for current use, but all these are nice to have for being able to plan in the mission room.
Most of the queries are actually already implemented.**

- Group by;
  - [ ] patches
  - [ ] lesser circles
  - [ ] longitudinal slices
  - [ ] latitudinal slices
- Rank by;
  - [ ] Visual magnitude
  - [ ] Astrometric quality
  - Something else in FK6
- Overall metrics;
  - [ ] Minimum separation?
  - [ ] Non-colinearity
  - [ ] Coverage?

#### Catalog

**A: FK6 is fine because it should contain FK 5, but with improved accuraccy.**

- [ ] FK5
- [ ] FK6

## Tracker Questions

#### Platform

**A: A command line is fine to get started, integrated with the planner.**

- [ ] C native application
    - any communication between planner and tracker would have to be inter-process 
- [ ] Spark web application linking to Novas with JNA
    - might be performance penalty but benchmarks were very encouraging...
    - more development time

#### Broadcast Interface

**A: TATS is the preferred protocol, either ethernet or serial is fine.
I'll still have a nice, modular API if anything is embedded.**

- [ ] Some protocol over some socket?
    - need protocol definition
- [ ] Link to TATS libraries?
    - need headers or binaries
- [ ] Integrate into simulator?
    - I provide a clean API?

#### Output Data

**A: Trackers use EFG. We'll see if the refraction model is sufficient when we get there. Most missions are above 20 
degrees elevation anyways so the effect should be negligible. Interpolation is handled by the by the ACUs.**

- Coordinates
    - [ ] geocentric EFG
    - [ ] Topocentric Azimuth and Elevation?
- Models
    - Refraction; Novas has a primitive exponential model based on ground temp/pressure
    - Aberation; Only a factor for higher accuracy mode?
- Interpolation?
    - Is it handled by the control unit or do we need to generate our own spherical Beziers?
    
#### Safety

**A: Sun avoidance is the only feature the PCU/ACUs don't have. And since the process is largely manual such a problem 
is usually caught- so this isn't a high priority feature.**

- Sun Avoidance: In the case of sun obstruction do I generate a new path?
- Cable wraps: does the plan have to limit total rotation?
- How would I implement a smoothness test?

#### Accuracy

**A: For the first delivery use the reduced accuracy which should be around 1mas. Improve accuracy when adding in planet
tracking. Once I generate A stream of EFG coordinates this can be tested against tracker outputs.**

- [ ] Default accuracy reaches 1 mas theoretically with good clock and catalog
- [ ] Increased accuracy is required for planets
    - need to generate some binaries for CIO predictions
    - need to relink to novas component to JPL's ephemeris libs...
      
#### Tracker Control ( depends on Planner platform )

**A: for first delivery just use the command line. Add a socket/pipe for the graphical UI later.**

- [ ] Pipe?
    - Could let the planner CLI drive it...
- [ ] Socket?
    -  Could let the planner web-app drive it...
- [ ] Command line?
    - Could let the user drive it manually...

__________________________________________________

## Tasklist Notes (Draft- need concept clarified so I can tighten this up...)

### Times

- [X] conversions between UTC, UT1, TT
- [ ] provide routines for P2000 and resolution(ns)
- [X] get accurate system time using GNU C libs
    - [ ] assume machine has NTP client installed

### Configuration

- [ ] Projected network delay
- [X] Telescope Location
    - Geodetic lat long, height in meters
- [ ] Weatherstation(WX) data
- [X] (UT1-UTC), (TAI-UTC)
- [ ] Scrape IERS bulletin or maintain configuration item for
    - UT1-UTC and leapseconds
    - nutation model (needed for planet tracking)
- [ ] compile novas CIO predictions
- [ ] JPL planet ephemeris

### Catalog

- [X] load FK5 data to tracker
- [ ] load FK6 data to tracker
- [X] load FK6 data to Planner
- [X] Provide filtering
- [ ] Provide sorting
- [ ] provide planning tools
    - [ ] cone spatial query
    - [X] patch
- [ ] CSV outputs

### Tracker

- [X] compute local horizon coordinates
- [X] convert between local coordinates and celestial ra, dec at given time
- [ ] compute EFG of stars on celestial sphere
- [ ] Broadcast coordinates at 50 Hz;
    - [ ] option 1: send EFG over TATS messages
    - [ ] option 2: send az, el over ethernet
- [ ] Provide PCU simulator interface
    - [ ] time should be specified in ns since beginning of year
    - [ ] output {az,el} or {E,F,G}
- Improve optical refraction model?
    - [ ] option 1: stay above 15 degrees elevation
    - [ ] option 2: replace novas refraction routine with more comprehensive model
    - [ ] option 3: invoke novas routines without including refraction, compute refraction externally
- Safety:
    - [ ] solar avoidance? 
    - [ ] cable wraps?
    - [ ] smoothness test?
  

### Client

- [X] Read configuration
- [X] Load catalog
- [X] Create tracker
- [X] Continually update time
- [X] Allow user to query for stars
    - [X] Shape(cone, slice, ring, patch)
    - [ ] Min brightness
- [X] Interactively ask user for star numbers
    - [ ] send new star to tracker
- [ ] Print/save usage logs.
    - [X] Bring together into convenient app
    - [X] Input: time, location, min brightness, star density
    - [ ] output: star/time/coordinate tables in CSV?
    - have interactive web app which allows stars to be selected, then tells tracker to go after them?
    
### Tests

- [ ] Accuraccy better than .001 degrees

## Example Planetarium and  Stargazing Software
  - [In-The-Sky](https://in-the-sky.org/skymap.php)
  - [Stellarium](http://stellarium.org/)
  - [Google Sky](https://www.google.com/sky/)
  - [The Sky Live](https://theskylive.com/)
  - [Aladin Lite](http://aladin.u-strasbg.fr/AladinLite/)