#ifndef LOCATION_H_
#define LOCATION_H_

#include "Config.h" 

#include <bitset>
#include <vector>

using namespace std;


//Class for maintaining incoming point locations of a moving object along with their annotations based on its motion pattern
class Location {
public:
	Location();
	~Location();

	long oid;   		//A globally unique identifier for the moving object (usually, the MMSI of vessels). REQUIRED
	double x;   		//Longitude coordinate in decimal degrees (georeference: WGS84) of this point location. REQUIRED
	double y;   		//Latitude coordinate in decimal degrees (georeference: WGS84) of this point location. REQUIRED
	unsigned t; 		//UNIX epoch timestamp (i.e., seconds elapsed since 1970-01-01 00:00:00.000) assigned by the data source (valid time semantics). REQUIRED
	double speed;           //Instantaneous speed over ground (in knots) of the moving object arriving at this location (w.r.t. its previously reported raw position)
	double heading;         //Instantaneous heading over ground (azimuth: degrees clockwise from North) of the moving object arriving at this location (w.r.t. its previously reported raw position)
	unsigned time_elapsed;  //Time elapsed (as UNIX epoch interval in seconds) since the previously reported raw position of this object
	double distance;        //Travelled distance (in meters) of the moving object from its previously reported raw position (CAUTION! NOT the previously detected critical point!) to its current one.

	bitset<11> annotation;  //bitmap that characterizes this location (i.e., as a critical point in the trajectory synopsis) with respect to mobility
	/*
	ANNOTATION bits:
	0: STOP_START -> the object has just stopped moving and became stationary at this position.
	1: STOP_END -> the object is no longer stationary and has just started moving (w.r.t. its previously known raw position).
	2: CHANGE_IN_SPEED_START -> speed over ground has just changed significantly (by a threshold parameter) w.r.t. the previously known speed.
	3: CHANGE_IN_SPEED_END -> speed over ground no longer diverges from the average speed over the most recent portion of the trajectory.
	4: SLOW_MOTION_START -> this is the first position reported by the object when moving at a very low speed (below a threshold parameter).
	5: SLOW_MOTION_END -> this is the last position reported by the object when moving at a very low speed (below a threshold parameter).
	6: GAP_START -> communication with this object was lost at this position (i.e., this is the last location reported just before a communication gap).
	7: GAP_END -> communication with this object has been restored at this position (i.e., this is the first location reported after a communication gap).
	8: CHANGE_IN_HEADING -> this is a turning point along the trajectory of this object, i.e., its actual heading over ground has just changed significantly (threshold angle parameter) w.r.t. its previous heading.
	9: NOISE -> this location qualifies as noise and should be discarded. Noisy locations are never included in velocity vector calculations.
	10: REPORTED -> this location has been reported already
	*/

	bool isAnnotated();
	vector<string> decodeAnnotation();

	bool isReported();
	void setReported();

	bool isAnnoStopStart();
	void setAnnoStopStart();
	void resetAnnoStopStart();
	bool isAnnoStopEnd();
	void setAnnoStopEnd();

	bool isAnnoChangeInSpeedStart();
	void setAnnoChangeInSpeedStart();
	bool isAnnoChangeInSpeedEnd();
	void setAnnoChangeInSpeedEnd();

	bool isAnnoSlowMotionStart();
	void setAnnoSlowMotionStart();
	bool isAnnoSlowMotionEnd();
	void setAnnoSlowMotionEnd();

	bool isAnnoGapStart();
	void setAnnoGapStart();
	bool isAnnoGapEnd();
	void setAnnoGapEnd();

	bool isAnnoChangeInHeading();
	void setAnnoChangeInHeading();
	void resetAnnoChangeInHeading();

	bool isAnnoNoise();
	void setAnnoNoise();

private:

};

#endif /*LOCATION_H_*/
