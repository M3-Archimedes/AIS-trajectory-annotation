//Title: Config.cpp
//Description: Configuration for a specific vessel type when simplifying their trajectories with annotated points.
//             Specifies mobility tracking parameters for use during trajectory summarization, i.e., for detection and characterization of annotated points along each trajectory.
//Author: Kostas Patroumpas
//Tested on platform(s): gcc 5.4.0, gcc 11.4.0, gcc 13.3.0
//Date: 15/1/2009
//Revision: 9/4/2025


#include "Config.h" 


//Constructor with initial DEFAULT settings; to be superseded by user-defined settings per specific vessel type (e.g., fishing, passenger, tanker)
Config::Config()
{
    this->vessel_type = "Default";
    this->state_size = 5;		//max number of valid locations in state
    this->state_timespan = 1000;	//seconds
    this->gap_period = 600;		//seconds

    this->low_speed = 2.0f;   		//knots (1 knot = 1.852 kmh)	 
    this->max_speed = 30.0f;   		//knots (1 knot = 1.852 kmh)	
    this->no_speed = 0.5f;   		//knots (1 knot = 1.852 kmh)	

    this->speed_ratio = 0.25f;   	        
    this->max_rate_of_change = 100.0f;   //knots per hour
    this->max_rate_of_turn = 3.0f;   	//degrees (azimuth) per second 

    this->distance_threshold = 50.0f;   //meters
    this->angle_threshold = 5.0f;	//degrees
}

//Destructor
Config::~Config()
{
}


//Print current settings
void Config::print()
{
    cout << "VESSEL_TYPE: " << this->vessel_type << endl;
    cout << "STATE_SIZE: " << this->state_size << endl;
    cout << "STATE_TIMESPAN: " << this->state_timespan << endl;
    cout << "GAP_PERIOD: " << this->gap_period << endl;
    cout << "LOW_SPEED_THRESHOLD: " << this->low_speed << endl;	 
    cout << "MAX_SPEED_THRESHOLD: " << this->max_speed << endl;
    cout << "NO_SPEED_THRESHOLD: " << this->no_speed << endl;
    cout << "SPEED_RATIO: " << this->speed_ratio << endl;        
    cout << "MAX_RATE_OF_CHANGE: " << this->max_rate_of_change << endl;
    cout << "MAX_RATE_OF_TURN: " << this->max_rate_of_turn << endl;
    cout << "DISTANCE_THRESHOLD: " << this->distance_threshold << endl;
    cout << "ANGLE_THRESHOLD: " << this->angle_threshold << endl;
}
