# Star Tracker
Broadcasts local horizon coordinates of a celestial target

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
 - Scrape IERS bullitin or maintain configuration item for
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
   - option 2: replace novas refraction routine with more comrehensive model
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
     - have interactive web app which allows stars to be selcted, then tells tracker to go after them?