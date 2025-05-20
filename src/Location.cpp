//Title: Location.cpp 
//Description: Class for maintaining incoming point locations of a moving object along with their annotations based on its motion pattern.
//Author: Kostas Patroumpas
//Tested on platform(s): gcc 5.4.0, gcc 11.4.0, gcc 13.3.0
//Date: 7/10/2009
//Revision: 1/4/2025

#include "Location.h" 

//Constructor 
Location::Location()
{
    //Initially unknown spatiotemporal features
    this->speed = 0.0f;
    this->heading = 0.0f;
    this->distance = 0.0f;
    this->time_elapsed = 0;
}

//Destructor
Location::~Location()
{
}

//Check if this location has been already reported in the output
bool Location::isReported()
{
    return (this->annotation[10]);
}

//Mark this location as already reported
void Location::setReported()
{
    this->annotation.set(10);
}


//Check if this location is annotated as a CRITICAL point (except for NOISE)
bool Location::isAnnotated()
{
    return (this->annotation[0] | this->annotation[1] | this->annotation[2] | this->annotation[3] | this->annotation[4] | this->annotation[5] |this->annotation[6] | this->annotation[7] | this->annotation[8]);
}

//Decode annotation bitmap into a string with the reported mobility features
vector<string> Location::decodeAnnotation()
{
    vector<string> anno;

    if (this->annotation[0])
        anno.push_back("STOP_START");

    if (this->annotation[1])
        anno.push_back("STOP_END");

    if (this->annotation[2])
        anno.push_back("CHANGE_IN_SPEED_START");

    if (this->annotation[3])
        anno.push_back("CHANGE_IN_SPEED_END");

    if (this->annotation[4])
        anno.push_back("SLOW_MOTION_START");

    if (this->annotation[5])
        anno.push_back("SLOW_MOTION_END");

    if (this->annotation[6])
        anno.push_back("GAP_START");

    if (this->annotation[7])
        anno.push_back("GAP_END");

    if (this->annotation[8])
        anno.push_back("CHANGE_IN_HEADING");

    if (this->annotation[9])
        anno.push_back("NOISE");

    return anno;
}

//Check if this location is annotated as a STOP_START
bool Location::isAnnoStopStart()
{
    return this->annotation[0];
}

//Annotate this location as STOP_START
void Location::setAnnoStopStart()
{
    this->annotation.set(0);
}

//Revoke annotation of this location as STOP_START
void Location::resetAnnoStopStart()
{
    this->annotation.reset(0);
}

//Check if this location is annotated as a STOP_END
bool Location::isAnnoStopEnd()
{
    return this->annotation[1];
}

//Annotate this location as STOP_END
void Location::setAnnoStopEnd()
{
    this->annotation.set(1);
}

//Check if this location is annotated as a CHANGE_IN_SPEED_START
bool Location::isAnnoChangeInSpeedStart()
{
    return this->annotation[2];
}

//Annotate this location as CHANGE_IN_SPEED_START
void Location::setAnnoChangeInSpeedStart()
{
    this->annotation.set(2);
}

//Check if this location is annotated as a CHANGE_IN_SPEED_END
bool Location::isAnnoChangeInSpeedEnd()
{
    return this->annotation[3];
}

//Annotate this location as CHANGE_IN_SPEED_END
void Location::setAnnoChangeInSpeedEnd()
{
    this->annotation.set(3);
}

//Check if this location is annotated as a SLOW_MOTION_START
bool Location::isAnnoSlowMotionStart()
{
    return this->annotation[4];
}

//Annotate this location as SLOW_MOTION_START
void Location::setAnnoSlowMotionStart()
{
    this->annotation.set(4);
}

//Check if this location is annotated as a SLOW_MOTION_END
bool Location::isAnnoSlowMotionEnd()
{
    return this->annotation[5];
}

//Annotate this location as SLOW_MOTION_END
void Location::setAnnoSlowMotionEnd()
{
    this->annotation.set(5);
}

//Check if this location is annotated as a GAP_START
bool Location::isAnnoGapStart()
{
    return this->annotation[6];
}

//Annotate this location as GAP_START
void Location::setAnnoGapStart()
{
    this->annotation.set(6);
}

//Check if this location is annotated as a GAP_END
bool Location::isAnnoGapEnd()
{
    return this->annotation[7];
}

//Annotate this location as GAP_END
void Location::setAnnoGapEnd()
{
    this->annotation.set(7);
}

//Check if this location is annotated as a CHANGE_IN_HEADING
bool Location::isAnnoChangeInHeading()
{
    return this->annotation[8];
}

//Annotate this location as CHANGE_IN_HEADING
void Location::setAnnoChangeInHeading()
{
    this->annotation.set(8);
}

//Revoke annotation of this location as CHANGE_IN_HEADING (during a stop)
void Location::resetAnnoChangeInHeading()
{
    this->annotation.reset(8);
}

//Check if this location is annotated as NOISE
bool Location::isAnnoNoise()
{
    return this->annotation[9];
}

//Annotate this location as NOISE
void Location::setAnnoNoise()
{
    this->annotation.set(9);
}
