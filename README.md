# Orion
Software for hunting stars- also, a star constellation which is a hunter.
Consists of a native application which transforms star catalog coordinates and broadcasts them, 
as well as an offline planning program used to design a viewing schedule.

## Planner Concept
##### First; what can we add that isn't already in this awesome public software?
  - [In-The-Sky](https://in-the-sky.org/skymap.php)
  - [Stellarium](http://stellarium.org/)
  - [Google Sky](https://www.google.com/sky/)
  - [The Sky Live](https://theskylive.com/)
  
##### My guesses;
 - adding some automation
 - creating standards for;
   - The master catalog
   - Star ranking (brightness, variability, motions)
   - The viewing plan's schedule and distribution
 - Generate reports by time tagging commands and tabulating results?
 
##### Thoughts?

#### How is a viewing plan made and carried out?
   - [ ] Is a plan built offline and later played back?
     - How do you control for the passage of time?
     - What format is the plan in?
   - [ ] Do we automatically generated a bright, distributed subset from the visible hemisphere?
     - how do we control for time beside setting a rigid viewing schedule?
   - [ ] Are targets interactively selected from zones of the current night sky?
     1. User types/clicks on hour 3 and 45 degrees
     2. System finds current terrestrial time and located the topocentric zeneith. 
     2. System finds a patch of sky 3 to 4 hours by 45 to 60 degrees
     3. Ranks stars by brightness and or astrometric quality
     4. Returns table of best candidates
     5. User clicks types selection
     6. Tracker is told to start controlling for that star

#### What platform should it be on?
   -[ ] CLI Application
     - simple to develop but harder to visually check
   -[ ] Web Application (Spark or Node)
     - requires recent browser and JVM or Node
     - Front-end visualization
       - [ ] D3 projections, topology and interactivity
         - I made an [FK6 chart](https://caseyshields.github.io/starlog/index.html) partially based on a script by Philip Plewa;
       - [ ] [Aladin Lite](http://aladin.u-strasbg.fr/AladinLite/)
         - can overlay catalogs on progressively rendered rasters 
         - needs to be on internet or special servers need to be replicated
   
#### How do we query and rank stars' characteristics for the plan?
   - Spatialy (lesser circles, longitudinal slices, latitudinal slices)?
   - Visual magnitude?
   - Density?

## Tracker Concept

#### Platform
   - [ ] C native application
     - communication between planner and tacker much harder 
   - [ ] Spark web application linking to Novas with JNA
     - might be performance penalty but benchmarks were very encouraging...

#### Broadcast Interface
   - [ ] Some protocol over some socket?
     - need protocol definition
   - [ ] Link to TATS libraries?
     - need headers or binaries
   - [ ] Integrate into simulator?
     - I provide a clean API?

#### Actual Control Data
   - Coordinates
     - [ ] geocentric EFG
     - [ ] Topocentric Azimuth and ELevation?
   - Models
     - Refraction; Novas has a primitive exponential model based on ground temp/pressure
     - Aberation; Only a factor for higher accuracy mode?
   - Interpolation?
     - Is it handled or do we need crazy spherical Beziers with kinematically limited curvatures?
     - In the case of sun obstruction do I generate a new path? 

#### Accuracy
   - [ ] Default accuracy reaches 1 mas theoretically with good clock and catalog
   - [ ] Increased accuracy is required for planets
     - need to generate some binaries for CIO predictions
     - need to relink to novas component to JPL's ephemeris libs...
      
#### Control ( depends on Planner platform )
   - [ ] Pipe?
     - Could let the planner CLI drive it...
   - [ ] Socket?
     -  Could let the planner web-app drive it...
   - [ ] Command line?
     - Could let the user drive it manually...

## Current Specific Tasklist

###Times
 -[X] conversions between UTC, UT1, TT
 -[ ] apply P2000 epoch and resolution(ns)
 -[X] get accurate system time using GNU C libs
   - assume machine has NTP client installed

###Configuration
 -[ ] Projected network delay
 -[ ] Telescope Location
   - Geodetic lat long, height in meters
 -[ ] Weatherstation(WX) data
  -[ ] File
  -[ ] TCP
 - Scrape IERS bulletin or maintain configuration item for
   -[ ] UT1-UTC and leapseconds
   -[ ] nutation model (needed for planet tracking)

###Catalog
 -[X] load FK5 data
 -[ ] load FK6 data
 -[X] provide filtering
 -[ ] convert between local coordinates and celestial ra, dec at given time
 -[ ] provide planning tools
   -[X] cone spatial query
   -[ ] ring query
   -[ ] orange slice query

###Tracker
 -[X] compute topocentric coordinates
 -[ ] compute EFG
 - Broadcast coordinates at 50 Hz;
    -[ ] option 1: send EFG over TATS messages
    -[ ] option 2: send az, el over ethernet
    -[ ] have smoothness test suite plug in here
 - Routine that can be built into the PCU simulator
    -[ ] time should be specified in ns since beginning of year
    -[ ] {az,el} or {E,F,G} place(Tracker* tracker, Entry* star, double jd_utc )
 - Handle bad optical refraction model
   - option 1: stay above 15 degrees elevation
   - option 2: replace novas refraction routine with more comprehensive model
   - option 3: invoke novas routines without including refraction, compute refraction externally
 - Safety:
   -[ ] solar avoidance? 
   - cable wraps?
  

###Client
 -[ ] Read configuration
 -[ ] Load catalog
 -[ ] Create tracker
 -[ ] Allow user to query for stars
      - Shape(cone, slice, ring)
      - Time
      - Min brightness
 -[ ] Interactively ask user for star numbers
      - send new star to tracker
 -[ ] Print/save usage logs.
   - [ ] Bring together into convenient app
     - [ ] Input: time, location, min brightness, star density
     - [ ] output: star/time/coordinate tables in CSV?
     - have interactive web app which allows stars to be selected, then tells tracker to go after them?