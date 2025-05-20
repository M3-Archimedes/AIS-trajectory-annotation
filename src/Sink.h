#ifndef SINK_H_
#define SINK_H_

#include "Scan.h" 

using namespace std;


//Class for collecting results into an output file
class Sink {
public:
	Sink(char *, bool);
	~Sink();

	void reportPoint(Location *);

	unsigned int countAnnotatedLocations();
	unsigned int countNoisyLocations();
	void emitResults(bool);

private:
	ofstream fout;
	unsigned int numAnno;
	unsigned int numNoise;
	bool includeId;

	multimap< unsigned, Location* > annoResults;   //Annotated locations ordered by timestamp

	void setHeader(vector<string>);
};

#endif /*SINK_H_*/
