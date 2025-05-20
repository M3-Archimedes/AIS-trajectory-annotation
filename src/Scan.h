#ifndef SCAN_H_
#define SCAN_H_

#include "mobility.h" 

using namespace std;

//Class for maintaining joined items from the windowing constructs
class Scan {
public:
	Scan(char*, long);
	~Scan();
	void setTimeAttribute(unsigned int);
	vector<Location *> consumeInput(unsigned int);
	bool exhausted;		     //Set TRUE at EOF
	unsigned int recCount;       //Count incoming tuples
	unsigned int curTime;
	long id;                     //Identifier of the SINGLE object being monitored
	bool mode;

private:
	fstream fin;
	string inLine;
	Location *inTuple;
	unsigned int attrTime;
	void read(unsigned int);
	vector<Location *> batchTuples;
	Location* decodeTuple(fstream &);
};

#endif /*SCAN_H_*/
