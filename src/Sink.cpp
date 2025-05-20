//Title: Sink.cpp 
//Description: /Class for collecting results into an output file.
//Author: Kostas Patroumpas
//Tested on platform(s): gcc 5.4.0, gcc 11.4.0, gcc 13.3.0
//Date: 28/3/2025
//Revision: 20/5/2025

#include "Sink.h" 


//Constructor 
Sink::Sink(char * outFileName, bool includeId = true)
{
    //Prepare output file
    fout.open(outFileName);

    //Check if identifiers will be inluced in the output
    this->includeId = includeId;
    
    //Initialize counters
    numAnno = 0;
    numNoise = 0;

    // Create header for the output file
    vector<string> fields;  //Output fields
    if (this->includeId) {
        fields = { "id", "lon", "lat", "t", "speed", "heading", "annotation" };  //MULTIPLE objects in single output file
    } 
    else {
        fields = { "lon", "lat", "t", "speed", "heading", "annotation" };  //SINGLE object
    }
    this->setHeader(fields);
}


//Destructor
Sink::~Sink()
{
    //Close output file of annotated points
    fout.close(); 
}


//Header with column names in the output file with the annotated points
void Sink::setHeader(vector<string> fields)
{
    unsigned int count = 0;
    for (auto & fld : fields) {
        fout << fld;
        count++;

        if (count == fields.size()) 
            fout << "\r\n";
        else 
            fout << DELIMITER ;
    }   
}


//Emits a trajectory point, possibly annotated with a mobility feature to the output QUEUE
void Sink::reportPoint(Location *p)
{
    //Check if this point has been already reported in the output
    if (p->isReported())
        return;

    //Mark this point as reported
    p->setReported();

    //Insert this location to the results
    annoResults.insert(pair< unsigned, Location* >(p->t, p));
}


//Emits all collected trajectory points (possibly annotated with a mobility feature) from the QUEUE to the output file
//If annotatedOnly is set to true, only annotated points will be issued into the output (i.e., neither normal nor noisy points).
void Sink::emitResults(bool annotatedOnly)
{
    multimap< unsigned, Location* >::iterator iterLoc;

    Location *p;

    //Iterate over all collected locations
    for (iterLoc = annoResults.begin(); iterLoc != annoResults.end(); iterLoc++) { 
        p = iterLoc->second;  
  
        //Check is there is some annotation for this location
        std::string s = "";
        if (p->isAnnotated()) {
            numAnno++;
            vector<string> anno = p->decodeAnnotation();
            for (std::size_t i = 0 ; i < anno.size() ; ++i)
                s += anno[i] + SEPARATOR;

            //Remove last SEPARATOR
            s = s.substr(0, s.size()-1);
        }
        else if (p->isAnnoNoise()) {
            numNoise++;
            s = "NOISE";
        }

        //Check if only annotated locations should be issued to the output
        if (annotatedOnly && !p->isAnnotated())
            continue;

        //Print the location to the output file
        if (this->includeId)   //with identifiers
            fout << p->oid << DELIMITER << setprecision(PRECISION) << fixed << p->x << DELIMITER << setprecision(PRECISION) << fixed << p->y << DELIMITER << p->t << DELIMITER << setprecision(PRECISION) << fixed << p->speed << DELIMITER << setprecision(PRECISION) << fixed << p->heading << DELIMITER << s << "\r\n";                 
        else             //without identifiers
            fout << setprecision(PRECISION) << fixed << p->x << DELIMITER << setprecision(PRECISION) << fixed << p->y << DELIMITER << p->t << DELIMITER << setprecision(PRECISION) << fixed << p->speed << DELIMITER << setprecision(PRECISION) << fixed << p->heading << DELIMITER << s << "\r\n";   
    }
}


//Get number of annotated locations reported so far
unsigned int Sink::countAnnotatedLocations()
{
    return numAnno;
}


//Get number of noisy locations reported so far
unsigned int Sink::countNoisyLocations()
{
    return numNoise;
}
