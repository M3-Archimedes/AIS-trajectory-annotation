//Title: Scan.cpp 
//Description: Consumes input lines from ASCII file (a) according to the specified arrival rate. 
//                                               OR (b) for a specified timestamp value in the dataset. 
//CAUTION: No tuple manipulation or timestamp assignment is done at that stage.
//Author: Kostas Patroumpas
//Tested on platform(s): gcc 5.4.0, gcc 11.4.0, gcc 13.3.0
//Date: 7/10/2009
//Revision: 16/4/2025


#include "Scan.h" 

//Constructor for reading tuples according to the specified stream source
Scan::Scan(char *fileName, long attrId)
{
    fin.open(fileName, ios::in);
    this->recCount = 0;
    this->exhausted = false;
    this->curTime = 0;
    this->inLine = "";
    this->inTuple = NULL;

    //Check whether the input stream concerns a SINGLE or MULTIPLE objects
    if (attrId < 0)   	// No attribute ID specified in input; this concerns a SINGLE object
        this->id = rand() % 1000000;  //Assign a random integer identifier to the source of the incoming stream items
    else
        this->id = -1;  // ID will be read from the rows of the input file; possibly MULTIPLE objects

}


//Destructor
Scan::~Scan()
{
    fin.close();  
}


void Scan::setTimeAttribute(unsigned int attrTime)
{
    this->attrTime = attrTime;
}


//Depending on the mode, it calls a specific function to read input tuples
vector<Location*> Scan::consumeInput(unsigned int t)
{
     batchTuples.clear();
     this->read(t);                  //VALID timestamping
  
     return batchTuples;  
}


//Decode tuple attributes from incoming string value into object location attributes
Location* Scan::decodeTuple(fstream &fin)
{
    Location *inTuple = new Location(); 

    //Important read test in order to avoid empty lines   
    if ((this->id > 0) && (fin >> inTuple->x >> inTuple->y >> inTuple->t )) { // SINGLE object    
        inTuple->oid = this->id;    //Associate attributes with the SINGLE object being monitored  
        return inTuple;
    }
    else if (fin >> inTuple->oid >> inTuple->x >> inTuple->y >> inTuple->t ) {  // MULTIPLE objects in input stream    
        return inTuple;
    }
    else {  //Empty line
        return NULL;
    }
}

//Read a batch of lines from the input ASCII file representing a streaming source until the specified timestamp value.
void Scan::read(unsigned int t)
{
    //First return the tuple that had been prefetched in the previous cycle
    if (inTuple != NULL) {
        if (inTuple->t <= t) { //Only in case it fits within the upper window bound
            batchTuples.push_back(inTuple);
            recCount++;
        }
        else                //No need to consume more tuples, as the upper window bound has not reached the next timestamp value in input
            return;
    }

    //Handle input source according to the specified arrival rate
    do {
        if (!fin.eof()) {
            inTuple = this->decodeTuple(fin);
                                    
            if (inTuple == NULL)                    
                break;

            //Current timestamp refers to the one from the last accessed tuple
            this->curTime = inTuple->t;
            if (this->curTime > t)	//Exceeded timestamp limit
                break;

            batchTuples.push_back(inTuple);		//Create a batch of incoming tuples 
            inTuple = NULL;
            recCount++;
        }
        else {
                this->exhausted = true;				//EOF
                break;
        }
    } while (true);

    //Notify progress
//    cerr << t << " -> " << recCount << " records processed..." << "\r";
}
