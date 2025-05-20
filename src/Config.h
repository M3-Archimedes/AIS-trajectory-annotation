#ifndef CONFIG_H_
#define CONFIG_H_

#include <stdio.h>
#include <iostream>

using namespace std;

// GENERAL PARAMETERS
#define DELIMITER ' '			//Delimiter for tuple attributes in input/output files
#define SEPARATOR ';'			//Separator between multiple annotations in output file 
#define PRECISION 5			//Number of decimal points when reporting double numbers
#define EPSILON 0.000001                //Epsilon tolerance when comparing double numbers
#define SLIDE 600                       //Window slide (in seconds) only for consuming input data in batches (every SLIDE seconds)


//Class for maintaining incoming point locations of a moving object along with their annotations based on its motion pattern
class Config {
public:
	Config();
	~Config();

	std::string vessel_type;   	//Type of vessel where these configuration settings will be applied.
	unsigned int state_size;	//number of most recent raw point locations to be used in velocity vector computations
	unsigned int state_timespan;	//seconds (UNIX epochs): Time interval for keeping history of older positions for velocity vector computations
	unsigned int gap_period;	//seconds (UNIX epochs): if time elapsed from previous location is above this value, a communication GAP has occurred

	double low_speed;   		//knots (1 knot = 1.852 kmh); under this speed, the vessel is in SLOW_MOTION. 
	double max_speed;   		//knots (1 knot = 1.852 kmh); over this speed, the location is NOISE.
	double no_speed;   		//knots (1 knot = 1.852 kmh); under this speed, the vessel is considered STOPPED.

	double speed_ratio;   	        //percentage: change by more than this between two successive locations may indicate acceleration or deceleration
	double max_rate_of_change;   	//knots per hour; if rate of change of speed is above this, the location may be NOISE
	double max_rate_of_turn;   	//degrees (azimuth) per second; if rate of turn is above this, the location may be NOISE

	double distance_threshold;   	//meters; under this distance from its previous location, the vessel may be STOPPED.
	double angle_threshold;   	//degrees; turning more than this angle from its previous location, the vessel may be CHANGE HEADING.

	void print();

private:

};

#endif /*CONFIG_H_*/
	   


