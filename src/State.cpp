//Title: State.cpp 
//Description: Retain the state of a moving object according to a count-based sliding window that retains the k=MINPOINTS instantaneous vectors.
//             This also provides the mean velocity (speed and heading) in order to account for data stream imperfections (mainly delayed AIS messages), sea drift or GPS errors.
//Author: Kostas Patroumpas
//Tested on platform(s): gcc 5.4.0, gcc 11.4.0, gcc 13.3.0
//Date: 11/3/2015
//Revision: 20/5/2025

#include "State.h"


//Constructor for the state referring to a specific object
State::State(long oid, unsigned int t0, Config *config, Sink *sink)
{
    this->oid = oid;
    this->curTime = t0;        	//time of latest refresh; initially coincides with the time that the window is firstly being applied

    curConfig = config;   	//Configuration settings for this object

    //Initialize bitmap: not known status yet
    this->status.reset();
    
    // Initially, no points held in state; any locations will be added after checked for irregularities
    seqPoints.clear();

    // Output file for reporting locations
    sinkStream = sink;
}


//Destructor
State::~State() {}


//Count locations currently in the state of this object
unsigned int State::countLocations()
{
    return this->seqPoints.size();
}


//Report the average speed across all retained items in the current state
//Calculate the quotient of total displacement over the elapsed time
//ALTERNATIVE NOT USED:Report the average speed between the two extreme positions in the current state
double State::getMeanSpeed()
{
   return (3600.0f * this->sumTravelDistance()) / (1852.0f * this->getTimespan());  //Value in knots per hour
}


//Report the accumulated heading across all retained items in the current state
double State::getAccumHeading()
{
    double diff = 0.0f;
    list <Location *>::iterator it = this->seqPoints.begin();
    Location *first = *it;
    ++it;
    while (it != this->seqPoints.end()) {
    	Location *second = *it;
    	//Pair-wise difference in heading between consecutive locations in the state
    	diff += getSlopeDifference(first->heading, second->heading);
    	first = second;
    	++it;
    }

    return diff;
}


//Calculate the centroid of all retained positions in the current state
Location* State::getCentroid()
{
    Location *c = new Location();

    //Initialization
    c->x = c->y = 0.0f;

    list <Location *>::iterator it = this->seqPoints.begin();
    while (it != this->seqPoints.end()) {
        c->x += (*it)->x;
        c->y += (*it)->y;
    	++it;
    }

    //Get mean (centroid) coordinates
    c->x /= this->countLocations();
    c->y /= this->countLocations(); 

    return c;
}


//Calculate the centroid of positions accumulated after a stop event started until now
Location* State::getStopCentroid()
{
    Location *c = new Location();

    //Initialization
    c->x = c->y = 0.0f;
	
    unsigned int n = 0;  //Number of locations in stop event

    list <Location *>::reverse_iterator rit = this->seqPoints.rbegin();
    ++rit;  //CAUTION! Exclude CURRENT location
    while (rit != this->seqPoints.rend()) {
        c->x += (*rit)->x;
        c->y += (*rit)->y;
    	++n;

        if ((*rit)->isAnnoStopStart())  //Location marked as STOP_START reached
            break;

    	++rit;  
    }

    //Get mean (centroid) coordinates
    c->x /= n;
    c->y /= n; 

    return c;
}



//Calculate the net displacement of positions accumulated after a stop event has started until now
double State::getStopNetDisplacement(bool excludeCurLocation)
{
    std::valarray<double> net {0.0f, 0.0f};

    list <Location *>::reverse_iterator rit = this->seqPoints.rbegin();

    if (excludeCurLocation)
        ++rit;  //Exclude CURRENT location

    Location *first = *rit;
    ++rit;
    while (rit != this->seqPoints.rend())  {
    	Location *second = *rit;
    	//Pair-wise net displacement between consecutive locations in the state
    	net += netDisplacement(first, second);
    	first = second;

        if ((*rit)->isAnnoStopStart()) 	//Location marked as STOP_START reached
            break;

    	++rit;
    }

    return getHaversineDistance(0.0f, 0.0f, net[0], net[1]);   //value in meters
}


//Distance of the current location from the point a stop has started
double State::getDistanceFromStopStart(Location *c)
{
    list <Location *>::reverse_iterator rit = this->seqPoints.rbegin();
    while (rit != this->seqPoints.rend())  {
        if ((*rit)->isAnnoStopStart())   //Location marked as STOP_START reached
            return getHaversineDistance(*rit, c);

    	++rit;
    }

    return 0.0f;   //value in meters
}


//Report the accumulated heading  of positions accumulated after a stop event has started until now
double State::getStopNetHeading(bool excludeCurLocation)
{
    double diff = 0.0f;

    list <Location *>::reverse_iterator rit = this->seqPoints.rbegin();

    if (excludeCurLocation)
        ++rit;  //Exclude CURRENT location

    Location *first = *rit;
    ++rit;
    while (rit != this->seqPoints.rend()) {
    	Location *second = *rit;
    	//Pair-wise net displacement between consecutive locations in the state
    	diff += getSlopeDifference(first->heading, second->heading);
    	first = second;

        if ((*rit)->isAnnoStopStart())	//Location marked as STOP_START reached
            break;

    	++rit;
    }

    return diff;
}

    
//Calculate the mean heading of this object based on its most recently reported locations currently in state
//Based on the azimuth angle between the oldest and the latest location in this object's state
double State::getMeanHeading()
{
    list <Location *>::reverse_iterator rit = this->seqPoints.rbegin();
    Location *oldest = this->seqPoints.front();
//    ++rit;   //Exclude latest point in the state
    if (rit != this->seqPoints.rend()) {
        Location *prev = *rit;
        return getBearing(oldest, prev);  //Return angle value between 0 and 359 degrees   
    }
    else
        return 0.0f;   //FIXME: Placeholder for NULL, not actually reached due to prior checks
}


//Calculate the time interval spanning the locations in the current state (used in velocity calculation)
unsigned int State::getTimespan()
{
    return (this->curTime - this->seqPoints.front()->t);   //Value in seconds
}


//Sum up the total displacement across all consecutive locations in the current state
double State::sumTravelDistance()
{
    double d = 0.0f;

    //Iterate over all points in current state
    list <Location *>::iterator it = this->seqPoints.begin();
    ++it; // Exclude the oldest point (its distance is relative to an obselete point no longer in state)
    while (it != this->seqPoints.end()) {
        d += (*it)->distance;  
        ++it;
    }

   return d;  //Value in meters
}


//Initialize the state with the given location
void State::init(Location *p)
{
    p->setAnnoGapEnd();  		//First, mark this location as GAP_END
    this->seqPoints.push_back(p);   	//Push new location into the sequence

    this->status.reset();     	//Reset bitmap

    //Refresh most recent timestamp
    this->curTime = p->t;
}


//Update the current state (velocity vector) of this object with a new location, and also eliminate obsolete locations from the queue
//CAUTION! The locations appended to the state must be checked for irregularities and annotated with any significant mobility features
void State::append(Location *p)
{
    this->seqPoints.push_back(p);   //Push new location into the sequence

    //Refresh most recent timestamp
    this->curTime = p->t;
}


//Update the current state (velocity vector) of this object with a new location, and also eliminate obsolete locations from the queue
//CAUTION! The locations appended to the state must be checked for irregularities and annotated with any significant mobility features
void State::update(Location *p)
{
    //Basic calculations between the last pair of successive locations of this object
    //Identify previous point in the sequence; already checked that there is at least one point in the current state
    Location *q = this->seqPoints.back();

    //STEP #1 (FORWARD check): Determine whether the current location should be characterized as a critical point (except for turning points)
    this->forwardMobilityCheck(q, p); //characterizes the CURRENT location

    //STEP #2 (BACKWARD check): Determine whether the previous location should also be characterized as a turning point (where significant change in heading is observed)
    this->backwardMobilityCheck(q, p); //characterizes the PREVIOUS location

    //Reset stop status
    if (p->isAnnoStopEnd())
        this->resetStopped();

    //No further changes in annotation can occur in the PREVIOUS location, so report it
    //Not safe, as a START_STOP event may be revoked if it was actually a SLOW MOTION
//    sinkStream->reportPoint(q);
}


//In case of communication gap, the state must be annulled
//Remove all contents from the current state (velocity vector) of this object
void State::purge()
{
    //Remove all positions from the sequence
    while (!this->seqPoints.empty())
    {    
        //Print out this expiring point into the file (including its annotation)    
        sinkStream->reportPoint(this->seqPoints.front());

        //Remove it from the sequence    
        this->seqPoints.pop_front();                      
    }

    //No points in state
    this->status.reset();     // No known status
}


//Cleanup the current state (velocity vector) of this object, BUT retain the two latest valid locations
void State::cleanup()
{
    while (this->seqPoints.size() > 2)
    {
        //Print out this expiring point into the file (including its annotation)    
        sinkStream->reportPoint(this->seqPoints.front());

        //Remove it from the sequence   
        this->seqPoints.pop_front(); 
    }
}


//In case the previous state has been invalidated, re-instantiate it with the new location
void State::restore(Location *p)
{
    this->append(p);    	//Push this location into the sequence
    this->status.reset();     	//Reset bitmap
}


//Check if no locations currently in state
bool State::isEmpty()
{
    return this->seqPoints.empty();
}


//Check the status if the object is stopped
bool State::isStopped()
{
    return this->status[0];  //First flag in the status indicates STOP
}


//Declare this object as stopped
void State::setStopped()
{
    this->status.set(0);
}


//Declare this object is no longer stopped
void State::resetStopped()
{
    this->status.reset(0);
}


//Check if the given object has changed its speed significantly
bool State::hasSpeedChanged()
{
    return this->status[1];  //Second flag in the status indicates SPEED_CHANGE
}


//Declare this object as having changed its speed
void State::setSpeedChanged()
{
    this->status.set(1); 
}


//Declare this object as not changed its speed significantly
void State::resetSpeedChanged()
{
    this->status.reset(1); 
}


//Check if the given object is moving slowly
bool State::isSlowMotion()
{
    return this->status[2];  //Third flag in the status indicates SLOW_MOTION
}


//Declare this object as moving slowly
void State::setSlowMotion()
{
    this->status.set(2); 
}


//This object no longer moves slowly
void State::resetSlowMotion()
{
    this->status.reset(2); 
}


//Invalidate a false STOP_START event declared before in the current state
bool State::revokeStop()
{
    list <Location *>::reverse_iterator rit = this->seqPoints.rbegin();
    while (rit != this->seqPoints.rend())  // Check backwards from now
    {
        if ((*rit)->isAnnoStopStart()) {   //Location marked as STOP_START reached
            (*rit)->resetAnnoStopStart();
            return true;
        }
    	++rit;  
    }
    return false;  //No such event found in current state
}


//Invalidate any false CHANGE_IN_HEADING event declared before in the current state
bool State::revokeChangeInHeading()
{
    list <Location *>::reverse_iterator rit = this->seqPoints.rbegin();
    //Check backwards from now ...
    while (rit != this->seqPoints.rend())  
    {
        if ((*rit)->isAnnoChangeInHeading()) {
            (*rit)->resetAnnoChangeInHeading();

            // ...until the START_STOP event is found
            if ((*rit)->isAnnoStopStart()) 
                return true;
        }
    	++rit;  
    }
    return false;  //No such event found in current state
}


//Apply noise filtering to incoming location (considered as a candidate critical point) w.r.t. to the previously reported one in the state
//Detect noise and accordingly update the object state (actually the history of recent locations maintained for this particular object)
bool State::checkNoise(Location *oldLoc, Location *newLoc)
{
    //In case this is the first or second location after a GAP, it cannot be considered for NOISE
    if (oldLoc->isAnnoGapEnd() || newLoc->isAnnoGapEnd()) {   //GAP_END
		return false;             //With only a single location available, no kind of noise can be possibly determined
    }

    //First, in case of EXCESSIVE speed, this location qualifies for noise
    if (newLoc->speed >= curConfig->max_speed) {
		return true;
    }
    //Also check whether there has been any improbable change of rate in instantaneous speed (i.e., huge acceleration or deceleration)
    else if ((oldLoc->speed > EPSILON) && (abs(getRateOfChangeKnots(oldLoc, newLoc)) >= curConfig->max_rate_of_change)) {
		return true;                     //Mark this location as noise
    }
    //Next, check if there has been a sudden surge in the rate of turn, provided that the object is NOT stopped (i.e., agility during stop should NOT be considered as noise)
    else if ((!oldLoc->isAnnoGapEnd()) && (newLoc->speed > curConfig->low_speed) && (getRateOfTurn(newLoc, oldLoc) >= curConfig->max_rate_of_turn)) {
		return true;                     //Mark this location as noise
    }

    return false;            //Location does not qualify for noise
  }


//BACKWARD moblity check: Detect any significant change in heading between two consecutive locations
//... and annotate accordingly the OLDEST location, because at that point the change in course actually took place
void State::backwardMobilityCheck(Location *oldLoc, Location *newLoc) 
{
    //If its previous location is a GAP_END, then artificially set that speed and heading accordingly
    if (oldLoc->isAnnoGapEnd()) {
        oldLoc->speed = newLoc->speed;
        oldLoc->heading = newLoc->heading;
    }
    //IMPORTANT: Check for changes in heading as long as this object is NOT marked as stopped
    else  {
      //Change of heading above threshold w.r.t. previous heading --> CHANGE_IN_HEADING
      //Compare also with the mean heading over the recent motion history of this object
      if ((newLoc->speed > curConfig->no_speed) && ((angleDifference(newLoc->heading, oldLoc->heading) > curConfig->angle_threshold) || (abs(this->getAccumHeading()) > curConfig->angle_threshold))) {
        //In case of low speed, changes in heading may be ignored due to sea drift
        if ((newLoc->speed < curConfig->low_speed) && (angleDifference(newLoc->heading, this->getMeanHeading()) < 2 * curConfig->angle_threshold))
            return;

        oldLoc->setAnnoChangeInHeading();
        if (!this->isStopped() && !this->isSlowMotion()) {      // && (getRateOfTurn(newLoc, oldLoc) >= curConfig->max_rate_of_turn)) {
            this->cleanup();             //Since the object changed its heading, remove older items except for the last two ones
        }
      }
    }
}


//FORWARD mobility check: Calculate spatiotemporal measures from pairs of consecutive locations per object
//... and determine suitable annotations for the LATEST reported location
void State::forwardMobilityCheck(Location *prevLoc, Location *newLoc) 
{
    Location *oldLoc; 

    //In case the PREVIOUS location had been marked as noise, then computation of the instantaneous spatiotemporal features for the LATEST location ...
    //... must bypass the previous one and use the last location available in the tail of the respective sequence
    if ((prevLoc->isAnnoNoise()) && (this->seqPoints.size() > 0))
      oldLoc = this->seqPoints.back();     //CAUTION: At this point, the queue should contain at least one buffered item
    else
      oldLoc = prevLoc;                    //Previously reported location is not noisy and it can be safely used for computations

    //Compute instantaneous spatiotemporal features between the two locations
    newLoc->distance = getHaversineDistance(oldLoc->x, oldLoc->y, newLoc->x, newLoc->y);
    newLoc->time_elapsed = newLoc->t - oldLoc->t;      //Time elapsed since previously reported (non-noisy) location

    //Delayed locations are automatically characterized as noise
    if (newLoc->time_elapsed <= 0) {
      newLoc->setAnnoNoise();
      sinkStream->reportPoint(newLoc);
      return;                         //Any further processing is meaningless
    }

    //Instantaneous speed -> value in knots per hour
    newLoc->speed = ((newLoc->time_elapsed > 0) ? ((3600.0f * newLoc->distance) / (1852.0f * newLoc->time_elapsed)) : -1.0f);  //-1.0 is placeholder for NULL speed

    //Instantaneous heading; keep previous heading when there is no significant displacement (due to sea drift or object agility or GPS discrepancies)
    newLoc->heading = getBearing(oldLoc, newLoc);

    //Communication has been restored after a time period
    if (newLoc->time_elapsed > curConfig->gap_period) {

        newLoc->setAnnoGapEnd();   //Mark this critical point as GAP_END

        //IMPORTANT: The previously reported location must be marked as GAP_START
        oldLoc->setAnnoGapStart();

        //Check if the object remains stopped in the same location after the gap; otherwise, its state must be purged
        if (this->isStopped() && ((newLoc->distance < curConfig->distance_threshold) || (getStopNetDisplacement(false) < curConfig->distance_threshold))) { 
            this->setStopped();   //Stop event continues
        }
        else {
            this->purge();
            this->init(newLoc);         //New state should hold this position only
            return;
        }
    }

    //Apply filtering w.r.t. NOISE
    if (checkNoise(oldLoc, newLoc)) {
        newLoc->setAnnoNoise();
        sinkStream->reportPoint(newLoc);
        return;                   //Any further processing is meaningless
    }
    else {  //Append location to state
        this->append(newLoc);      //This is safe; object state already exists
    }

    //If less than two past locations are held in state (apparently, only the current one), then no further calculations can be made
    if (this->seqPoints.size() < 2)
      return;

    //CAUTION! Both check conditions concerning STOP cannot hold simultaneously!
    if (!this->isStopped() && (newLoc->speed < curConfig->no_speed) 
       && ((newLoc->distance < curConfig->distance_threshold) || (getHaversineDistance(newLoc, this->getCentroid()) < curConfig->distance_threshold)) ) {

        //In case this stop is immediately after a GAP, ...
        if (oldLoc->isAnnoGapEnd())
            oldLoc->setAnnoStopStart();   //...annotate previous location
        else
            newLoc->setAnnoStopStart(); //...otherwise, the current location

        this->setStopped();   //Mark object as STOPPED

        //Once a stop has started, terminate any previous slow motion phenomenon
        if (this->isSlowMotion()) {
            newLoc->setAnnoSlowMotionEnd();
            this->resetSlowMotion();
        }
        //Once a stop has started, terminate any previous change in speed phenomenon
        if (this->hasSpeedChanged()) {
            newLoc->setAnnoChangeInSpeedEnd();
            this->resetSpeedChanged();
        }
    }
    //If either criterion holds: significant speed OR distance threshold --> STOP END
    else if (this->isStopped() && ((newLoc->speed >= curConfig->no_speed) || (newLoc->distance >= curConfig->distance_threshold)) ) {
        if (getStopNetDisplacement(true) > curConfig->distance_threshold) {   //Not really a stop, probably moving at very slow speed
            this->revokeStop();   //Invalidate STOP_START
            this->resetStopped();
        }
        else if (getStopNetDisplacement(false) >= curConfig->distance_threshold) {   //This indeed was a stop, so invalidate any CHANGE_IN_HEADING events during this event
            oldLoc->setAnnoStopEnd();
            this->resetStopped();
            this->revokeChangeInHeading();
        }
    }

    //IMPORTANT: Annotate any other mobility features as long as this object is NOT marked as stopped
    if (!this->isStopped()) {

      //Speed ratio threshold exceeded --> CHANGE_IN_SPEED_START
      double mean_speed = this->getMeanSpeed();
      if ((abs((newLoc->speed - mean_speed) / mean_speed) > curConfig->speed_ratio) && !this->hasSpeedChanged()) {
        newLoc->setAnnoChangeInSpeedStart();
        this->setSpeedChanged();
      }

      //Speed ratio threshold not exceeded --> CHANGE_IN_SPEED_END
      if ((abs((newLoc->speed - mean_speed) / mean_speed) <= curConfig->speed_ratio) && this->hasSpeedChanged()) {
        newLoc->setAnnoChangeInSpeedEnd();
        this->resetSpeedChanged();
      }

      //Low speed threshold --> SLOW_MOTION_START
      if ((newLoc->speed <= curConfig->low_speed) && (oldLoc->speed > curConfig->low_speed) && !this->isSlowMotion()) {
        newLoc->setAnnoSlowMotionStart();
        this->setSlowMotion();
      }

      //Low speed threshold --> SLOW_MOTION_END
      if ((newLoc->speed > curConfig->low_speed) && (oldLoc->speed <= curConfig->low_speed) && this->isSlowMotion()) {
        newLoc->setAnnoSlowMotionEnd();
        this->resetSlowMotion();
      }
    }
}


//Once the stream is exhausted, mark the last reported position of each object as GAP_START
//CAUTION! This must be explicitly called upon exhaustion of the stream, before reporting any critical points in the last (possibly incomplete window) state
void State::markLastLocationAsGap()
{
    if (!this->seqPoints.empty()) {        
        //Identify latest location in the sequence
        this->seqPoints.back()->setAnnoGapStart();     //Possible GAP_START 

        //In case the object is marked as stopped, invalidate any CHANGE_IN_HEADING events
        if (this->isStopped()) {
            this->revokeChangeInHeading();
        }
    }
    
    Location *q;    

    //Multiple positions may be expiring after an update; remove them from the sequence
    while (!this->seqPoints.empty()) {    
        //Report this expiring point into the file (including its annotation)    
        q = this->seqPoints.front();
        sinkStream->reportPoint(q);

        //Remove it from the sequence    
        this->seqPoints.pop_front();                       
    }
}


//Remove oldest locations expiring from the state and report them into the output along with their ANNOTATION
void State::expungeObsoleteLocations(unsigned int t)
{
    //Keep locations in state while the object is considered as stopped; needed in case a false stop must be revoked (e.g., due to small speed)
    if (this->isStopped())
        return;
   
    Location *q;
          
    //Multiple positions may be expiring after an update; remove them from the sequence
    //Keep the latest one for checking occasional GAP events
    while ((this->seqPoints.size()>1) && (((q = this->seqPoints.front())->t <= t - curConfig->state_timespan) || (this->countLocations() > curConfig->state_size))) { 
        //Report this expiring point into the file (including its annotation)    
        sinkStream->reportPoint(q);

        //Remove it from the sequence    
        this->seqPoints.pop_front();
    }

    //Update time of last refresh
    if (!this->seqPoints.empty())
        this->curTime = this->seqPoints.back()->t;
    else   //No points actually left; reset status
        this->status.reset();
		
}
