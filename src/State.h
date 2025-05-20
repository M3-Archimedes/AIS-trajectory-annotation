#ifndef STATE_H_
#define STATE_H_

#include "Sink.h"

using namespace std;

//Class for maintaining the velocity vector and mobility status for a particular moving object (vessel) over a small number of its latest positions across a recent time interval
class State {
public:
    State(long, unsigned int, Config *, Sink *);
    ~State();

    void init(Location *);
    void append(Location *);
    void update(Location *);
    void purge();
    void cleanup();
    void restore(Location *);

    bool isEmpty();
    bool isStopped();
    void setStopped();
    void resetStopped();
    bool hasSpeedChanged();
    void setSpeedChanged();
    void resetSpeedChanged();
    bool isSlowMotion();
    void setSlowMotion();
    void resetSlowMotion();
    bool revokeStop();
    bool revokeChangeInHeading();

    void expungeObsoleteLocations(unsigned int);
    void markLastLocationAsGap();

    double getMeanSpeed();
    double getMeanHeading();
    double getAccumHeading();   // cummulative heading across all locations within state
    Location* getCentroid();
    Location* getStopCentroid();
    double getStopNetDisplacement(bool);
    double getStopNetHeading(bool);
    double getDistanceFromStopStart(Location *);

private:
    long oid;                       	//Object identifier
    unsigned int curTime;               //Timestamp of latest update

    Config *curConfig;  		//Configuration settings based on vessel type

    //Maintain a sequence of recent, chronologically ordered, noise-free, RAW LOCATIONS per object
    list <Location *> seqPoints;	//List of points currently maintained in the state

    bitset<3> status;    //bitmap denoting the current status of an object: 0-bit: STOPPED; 1-bit: SPEED_CHANGED; 2-bit: SLOW_MOTION

    unsigned int countLocations();         
    unsigned int getTimespan();
    double sumTravelDistance();

    bool checkNoise(Location *, Location *);
    void backwardMobilityCheck(Location *, Location *);
    void forwardMobilityCheck(Location *, Location *);

    Sink *sinkStream;  // Output file for reporting locations

};

#endif /*STATE_H_*/
