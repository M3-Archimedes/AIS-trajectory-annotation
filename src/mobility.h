//Title: mobility.h
//Description: Mobility checks and functions for manipulating vessel locations and their trajectories.
//Tested on platform(s): gcc 5.4.0, gcc 11.4.0, gcc 13.3.0
//Date: 15/1/2009
//Revision: 9/4/2025

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <vector>
#include <list>
#include <queue>
#include <string>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <cmath>
#include <map>
#include <algorithm>
#include <numeric>
#include <chrono>
#include <valarray>

using namespace std;

#include "Location.h" 


const double PI = 4 * std::atan(1.0);   //PI constant


extern fstream fout;	                //Output file for processing results


//Get system time in milliseconds
inline unsigned int get_time() 
{
    std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    return (unsigned int)millis;
}

//Remain idle for the specified time interval
inline void sleepTime(clock_t sec) 
{
    clock_t start_time = clock();
    clock_t end_time = sec * 1000 + start_time;
    while ( clock() <= end_time )
        ;
}


//Get the value in radians of the given geographic coordinate
inline double getRadians(double x)
{
    return x * 4 * atan(1.0f) / 180.0f;
}


//Another method for computing Haversine distance between two geographic locations (expressed in lon/lat coordinates)
inline double getHaversineDistance(double lon1, double lat1, double lon2, double lat2) 
{
    //Coincident locations, zero distance
    if ((abs(lon1 - lon2) < EPSILON) && (abs(lat1 - lat2) < EPSILON))
        return 0.0f;

    // Convert degrees to radians
    lat1 = lat1 * PI / 180.0;
    lon1 = lon1 * PI / 180.0;

    lat2 = lat2 * PI / 180.0;
    lon2 = lon2 * PI / 180.0;

    //Approximate radius of the Earth in meters
    double r = 6378100;

    // P
    double rho1 = r * cos(lat1);
    double z1 = r * sin(lat1);
    double x1 = rho1 * cos(lon1);
    double y1 = rho1 * sin(lon1);

    // Q
    double rho2 = r * cos(lat2);
    double z2 = r * sin(lat2);
    double x2 = rho2 * cos(lon2);
    double y2 = rho2 * sin(lon2);

    // Dot product
    double dot = (x1 * x2 + y1 * y2 + z1 * z2);
    double cos_theta = dot / (r * r);

    double theta = acos(cos_theta);

    // Distance in meters
    return r * theta;
}


inline double getHaversineDistance(Location* p1, Location* p2) 
{
    return getHaversineDistance(p1->x, p1->y, p2->x, p2->y);  //Distance in meters
}


//Net displacemnt as a vector
inline std::valarray<double> netDisplacement(Location* p1, Location* p2) 
{
    return  std::valarray<double>{(p2->x - p1->x), (p2->y - p1->y)};   // Displacement in each axis; values in decimal degrees
}


//Return average speed between two point locations in km/h based on Haversine distance
inline double findSpeedKmh(Location& p1, Location& p2)
{
    return (3.6 * getHaversineDistance(p1.x, p1.y, p2.x, p2.y) / (1.0 *(p2.t - p1.t)));
}


//Calculate speed of movement (in knots) from one location to another based on Haversine distance
inline double getSpeedKnots(Location* p1, Location* p2)
{
    if (p2->t > p1->t)                       //timestamp values expressed in seconds
        return 3600.0 * getHaversineDistance(p1->x, p1->y, p2->x, p2->y) / (1852.0 * (p2->t - p1->t)); //Return value in knots: nautical miles per hour
    else
        return -1.0;            //Placeholder for NULL speed
}
	

//Return elapsed time between two point locations, based on their timestamps
inline double getElapsedTime(Location& p1, Location& p2)
{
    return (1.0f *(p2.t - p1.t));   // value in seconds
}


//Return the difference between two velocities (based on the Law of Cosines, according to their speed and heading)
inline double getVelocityDiff(double& s1, double& a1, double& s2, double& a2)
{
        return sqrt(s1*s1 + s2*s2 - 2*s1*s2*cos(a1-a2));
}


//Calculate azimuth (in degrees) between a pair of 2-d point locations
inline double findAzimuth(double x1, double y1, double x2, double y2)
{
	double pAngle, dx, dy, slope;
	dx = x2 - x1;
	dy = y2 - y1;

//	AZIMUTH calculation (in radians)
	if (dx == 0) {
		if (dy > 0)		//Northbound
			pAngle = 0;     
		else if (dy < 0)	//Southbound
			pAngle = PI;  
		else   //(dy == 0)	//Stationary (i.e., no movement), so return a SPECIAL (NULL) value
			return -1;   
	}
	else {  // (dx != 0)
		slope = atan(abs(dx/dy));   //ATTENTION: atan() returns values between -pi/2 and pi/2

		if (dx > 0) {
			if (dy > 0)				//NE quadrant
				pAngle = slope;            
			else if (dy < 0)        //SE quadrant
				pAngle = PI - slope;
			else // (dy == 0)		//Eastbound
				pAngle = PI/2;  
		}
		else {  // (dx < 0)
			if (dy > 0)				//NW quadrant
				pAngle = 2*PI - slope;  
			else if (dy < 0)		//SW quadrant
				pAngle = PI + slope;   
			else // (dy == 0)		//Westbound
				pAngle = 3*PI/2;
		}
	}

	return pAngle * 180 / PI;   //Convert angle into degrees
}


//Calculate azimuth (in degrees) between a pair of 2-d point locations
inline double getBearing(Location* p1, Location* p2)
{
	return findAzimuth(p1->x, p1->y, p2->x, p2->y);
}


//Get difference between two azimuths (in degrees)
inline double diffAzimuthDegrees(double firstAzimuth, double secondAzimuth)
{
    double diff = secondAzimuth - firstAzimuth;
    return ( (diff > 180.0f) ? (diff - 360.0f) : ((diff <= -180.0f) ? (diff + 360.0f) : diff)) ;
}


//Calculates the angular difference (in degrees) between two given headings (azimuth values)
inline double angleDifference(double heading1, double heading2) 
{
      double phi = std::fmod(abs(heading1 - heading2), 360.0f);
      if (phi > 180)
        return (360.0f - phi);            //Return angle value between 0 and 359 degrees
      else
        return phi;
}


//Get slope difference between two angles (in degrees) in the trigonometric cycle
//CAUTION! This returns values with a sign (+/-)
inline double getSlopeDifference(double heading1, double heading2)
{
    return 180.0f - abs(180.0f - (heading2 - heading1));               //Return angle value between -180 and 180 degrees
}


//Calculate acceleration (sign: +) or deceleration (sign: -) over ground; speed and elapsed time values must have been calculated beforehand for each location
inline double getRateOfChangeKnots(Location *p_old, Location *p_new)
{
    if (p_new->time_elapsed > 0)
        return (3600.0f * (p_new->speed - p_old->speed)) / (1.0f * p_new->time_elapsed);    //Return value in knots/hour; speed has been already calculated in knots
    else
        return 0.0f;                   //Value cannot be calculated
}


//Calculates rate of turn (in degrees/sec) between two given locations; actually the change in heading (angle in azimuth values) between those two sample locations
inline double getRateOfTurn(Location *p_old, Location *p_new)
{
    double a = p_new->heading - p_old->heading;
    double phi = std::fmod((std::fmod((a + 180.0f), 360.0f) + 360.0f), 360.0f) - 180.0f;               //Workaround in order to return a positive difference of angles
	
    if (phi > 180.0f)
        return (360.0f - phi) / (1.0f * p_new->time_elapsed);      //Return value in degrees/sec
    else
        return (phi / (1.0f * p_new->time_elapsed));
}
	