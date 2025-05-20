//Title: main.cpp 
//Description: Consume streaming AIS trajectories from ASCII file and annotate point locations signifying mobility events (stop, turn, gap, slow motion, etc.).
//CAUTION: Input file contains rows with attributes < [identifier] longitude latitude timestamp > and must NOT end with an empty line. If no identifiers are included, it is assumed that the input data concerns a SINGLE vessel.    
//ASSUMPTION: The input file is sorted by ascending timestamp values (UNIX epochs in seconds). 
//Author: Kostas Patroumpas
//Tested on platform(s): gcc 5.4.0, gcc 11.4.0, gcc 13.3.0
//Date: 4/3/2011
//Revision: 20/5/2025
//Issues not yet resolved:


#include "State.h"
#include <unistd.h>
#include <tuple>                                                                
                                                                                
#include <boost/property_tree/ptree.hpp>                           
#include <boost/property_tree/json_parser.hpp> 
#include <boost/algorithm/string.hpp>

using namespace std;


//Retain the states per trajectory
map< long, State* > trajStates;
map< long, State* >::iterator iterState;

//Retain the user-specified configurations per vessel type
map< string, Config* > vesselTypeConfigs;    


//Identifies the starting timestamp in the first row of the input file; colTimeAttr specifies the column of the timestamp attribute
//ASSUMPTION: The input file is sorted by ascending timestamp values in the given column.
int getStartTimestamp(char* fileName, int colTimeAttr)
{
    ifstream infile(fileName);

    int t = 0;      //Default value

    if (infile.good()) {
        string sLine;
        getline(infile, sLine);       //First row contains data, including a timestamp value
      
        vector<string> internal;
        stringstream ss(sLine);
        string tok;
 
        //Split this line into string using the default delimiter
        while(getline(ss, tok, ' ')) {
            internal.push_back(tok);
        }

        //The value at the column numbered colTimeAttr is the timestamp value
        t = atoi(internal.at(colTimeAttr).c_str());
    }
    else {
        cout << "Input file not found or not conforming to the specifications of the method. Please check again." << endl; 
    }

    infile.close();

    return t;          
}


//Parse the accompanying JSON configuration with the user-specified settings per vessel type; otherwise, a generic DEFAULT configuration will be applied
map< string, Config* > parseConfig(char* configJSONfile) {

    namespace pt = boost::property_tree;                                        
    pt::ptree jsonRoot;                                                    
     
    //Consume input JSON config                                                                          
    pt::read_json(configJSONfile, jsonRoot);                               
    
    map< string, Config* > vesselTypeConfigs;
                                                                              
    pt::ptree t_vessel;                                                            

    bool specDefault = false;

    for (auto const& vessel : jsonRoot) {

        Config* config = new Config();

        config->vessel_type = vessel.first;

        //Modified DEFAULT specifications included in JSON
        if (vessel.first == "Default")
            specDefault = true;

        //Get vessel type and handle all specified parameters                                                           
        t_vessel = jsonRoot.get_child(vessel.first);

        //STATE_SIZE
        if (t_vessel.find("STATE_SIZE") != t_vessel.not_found()) {
            config->state_size = t_vessel.get< unsigned int >("STATE_SIZE");
        }

        //STATE_TIMESPAN
        if (t_vessel.find("STATE_TIMESPAN") != t_vessel.not_found()) {
            config->state_timespan = t_vessel.get< unsigned int >("STATE_TIMESPAN");
        }

        //GAP_PERIOD
        if (t_vessel.find("GAP_PERIOD") != t_vessel.not_found()) {
            config->gap_period = t_vessel.get< unsigned int >("GAP_PERIOD");
        }

        //LOW_SPEED_THRESHOLD
        if (t_vessel.find("LOW_SPEED_THRESHOLD") != t_vessel.not_found()) {
            config->low_speed = t_vessel.get< double >("LOW_SPEED_THRESHOLD");
        }

        //MAX_SPEED_THRESHOLD
        if (t_vessel.find("MAX_SPEED_THRESHOLD") != t_vessel.not_found()) {
            config->max_speed = t_vessel.get< double >("MAX_SPEED_THRESHOLD");
        }

        //NO_SPEED_THRESHOLD
        if (t_vessel.find("NO_SPEED_THRESHOLD") != t_vessel.not_found()) {
            config->no_speed = t_vessel.get< double >("NO_SPEED_THRESHOLD");
        }

        //MAX_RATE_OF_CHANGE           
        if ( t_vessel.find("MAX_RATE_OF_CHANGE") != t_vessel.not_found()) {
            config->max_rate_of_change = t_vessel.get< double >("MAX_RATE_OF_CHANGE");
        }

        //MAX_RATE_OF_TURN            
        if ( t_vessel.find("MAX_RATE_OF_TURN") != t_vessel.not_found()) {
            config->max_rate_of_turn = t_vessel.get< double >("MAX_RATE_OF_TURN");
        }

        //SPEED_RATIO
        if ( t_vessel.find("SPEED_RATIO") != t_vessel.not_found()) {
            config->speed_ratio = t_vessel.get< double >("SPEED_RATIO");
        }

        //ANGLE_THRESHOLD           
        if (t_vessel.find("ANGLE_THRESHOLD") != t_vessel.not_found()) {
            config->angle_threshold = t_vessel.get< double >("ANGLE_THRESHOLD");
        }

        //DISTANCE_THRESHOLD           
        if (t_vessel.find("DISTANCE_THRESHOLD") != t_vessel.not_found()) {
            config->distance_threshold = t_vessel.get< double >("DISTANCE_THRESHOLD");
        }

		// Keep all settings for applying them to such type of vessels
        vesselTypeConfigs.insert(pair< string, Config* >(vessel.first, config));
    }

    //If a DEFAULT configuration is not user-specified, include its predefined settings
    if (!specDefault) {
        Config* config = new Config();
        vesselTypeConfigs.insert(pair< string, Config* >("Default", config));
    }

    return vesselTypeConfigs;
}


//Read vessel information from accompanying CSV file
//IMPORTANT! this identifies vessel type (passenger, cargo, etc.) in order to applys the corresponding configuration settings 
map< long, string > parseVesselInfo(char* vesselCSVfile) {

    ifstream inFile(vesselCSVfile);

    map< long, string > vesselInfo;

    unsigned int cnt = 0;
    vector<string> vec;

    if (inFile) {
        string line;
        while (getline(inFile, line)) {
            cnt++;
            //Skip header
            if (cnt == 1) 
                continue;

            // Tokenize the input string using default separator ';'
            boost::split(vec, line, boost::is_any_of(";"));  //is_any_of("\t ; ,")
            vesselInfo.insert(pair< long, string >(stoi(vec[0]), vec[2]));
        }
    }
   return vesselInfo;
}


//Entry point to the application
int main(int argc, char* argv[])
{    
    if (argc != 8) {
        cout << "Usage: " << argv[0] << " [input-file] [id-attr] [timestamp-attr] [settings-json] [vessel-info-file] [output-file] [annotated-only]" << endl;
        //EXAMPLE execution command: ./annotate mmsi228037700.csv -1 3 settings.json vessel_info.csv mmsi228037700_annotated.csv true
        //[input-file]: Input data given in file "mmsi228037700.csv" (ASCII space delimited). The contents of the file are used to simulate a positional data stream based on the incoming AIS timestamped locations from vessels.
        //[id-attr]: Specifies the ID attribute in input data; give a negative integer (e.g., -1) if NO vessel identifiers are included in the input data.
        //[timestamp-attr]: Specifies the timestamp attribute in the input data (in this example, this is the 3rd field in input file).
        //[settings-json]: Path to JSON file with configuration settings per vessel type. 
        //[vessel-info-file]: Path to CSV file with vessel information (IMPORTANT: includes the type of each vessel in the input data).
        //[output-file]: Output data (points with the detected annotations) will be stored in file "mmsi228037700_annotated.csv" (ASCII space delimited).
        //[annotated-only]: Boolean controlling which points will be emitted. If true, only points with detected annotations will be stored into the output file; otherwise, normal (i.e., not annotated) and noisy points (annotated as NOISE) will be also included.
        exit(0);
    }

    //First argument defines the input file that contains incoming items
    char *fileName;                
    fileName = argv[1];

    //Simulate a scan operator over the incoming stream data
    Scan * scanStream = new Scan(fileName, atoi(argv[2])-1);

    //Specifies the timestamp attribute in the schema of input tuples
    scanStream->setTimeAttribute(atoi(argv[3])-1);

    //Parse user-specified configuration settings
    char *configJSONfile;                
    configJSONfile = argv[4];
    vesselTypeConfigs = parseConfig(configJSONfile);

    //Associate the vessel types per MMSI from CSV file
    char *vesselInfoCSVfile;                
    vesselInfoCSVfile = argv[5];
    map< long, string > vesselInfo = parseVesselInfo(vesselInfoCSVfile);
    string vessel_type = "Default";    

    //Check if only annotated points should be emitted to the output file
    std::istringstream flag_anno(argv[7]);
    bool annotated_only;
    flag_anno >> std::boolalpha >> annotated_only;

    //Defines the output file that will contain the resulting critical points
    char *csvCritical;                
    csvCritical = argv[6];

    //Prepare a sink to write the results into an output file
    bool includeID = (atoi(argv[2]) >= 1);  // Check if object identifiers should be included in the output; the same ones used in input data
    Sink * sinkStream = new Sink(csvCritical, includeID);

    //The first timestamp value in the input file specifies the time when the window is being applied
    unsigned int t0 = getStartTimestamp(fileName, atoi(argv[3])-1);

    //Exit if no valid timestamp is found or the file does not exist
    if (t0 == 0)
       exit(1);
	
    scanStream->curTime = t0;           //Initialization: timestamp value when the window is being applied

    vector<Location*> inTuples;         //Batch of incoming tuples
    vector<Location*>::iterator it;

    map< long, State* >::iterator remIter;

    State *newState;
    Location *p;

    unsigned int t = t0;    //Timestamp values should start from the time given by the window initiation	
    unsigned int t_proc;    //Measuring execution cost (in milliseconds) per window instantiation

    unsigned int i = 0;
    unsigned int k = 0;

    //Print time indication when evaluation starts
    time_t t_now = time(0);   // get time now
    struct tm * now = localtime( & t_now );
    cout << "Input: " << fileName << " "; // << "\r\n"; 

    //STREAM INPUT: Keep processing data file until it gets exhausted
    unsigned int t_start = get_time();         //Measuring total execution cost (in milliseconds) 
    while (scanStream->exhausted == false) {
        //Proceed to accept next batch of tuples up to this timestamp value 
        t = t + SLIDE;
      
        //Read streaming data
        inTuples = scanStream->consumeInput(t);	

        //Create new tuples for the current timestamp value
        t_proc = get_time();  
        for (it = inTuples.begin(); it != inTuples.end(); it++ ) {

            //UPDATE: Refresh object location and update its state
            //Get trajectory already maintained for this object
            iterState = trajStates.find( (*it)->oid );   
            if (iterState == trajStates.end()) {     //No state available for this object
                //First, identify the vessel type for this NEW object
                try { vessel_type = vesselInfo.at((*it)->oid); }
                catch (const std::out_of_range&) { vessel_type = "Default"; }   //Apply default settings if vessel type is unknown
                //cout << "Vessel " << (*it)->oid << " type: " << vessel_type << endl;

                //Get configuration settings for this type of vessel            
                Config* curConfig = (vesselTypeConfigs.find(vessel_type))->second;
                newState = new State((*it)->oid, (*it)->t, curConfig, sinkStream);
                newState->init(*it);
                trajStates.insert(pair< long, State* >((*it)->oid, newState));
                k = trajStates.size();
            }
            else {
                //Remove any obsolete locations from the state
                iterState->second->expungeObsoleteLocations((*it)->t);
                //Update state with fresh location
                if (iterState->second->isEmpty()) { //state is empty, possibly because of a communication gap
                    iterState->second->init(*it);
                }
                else {   //Update state and annotate locations accordingly
                    iterState->second->update(*it);
                }
            }
        }

        //Processing time for handling ONLINE items (in milliseconds)
        t_proc = get_time() - t_proc;
 
        inTuples.clear();
        i++;
    }

    //Once the stream is exhausted, expunge any remaining positions from the last state of each sequence
    for (iterState = trajStates.begin(); iterState != trajStates.end(); iterState++) { 
        //Mark the last point as GAP_START and report all locations to the output
        iterState->second->markLastLocationAsGap();    
    }

    //Report execution statistics
    cout << "Output: " << csvCritical << " #objects: " << trajStates.size() << " "; // << "\r\n";  	
    cout << "Runtime (sec): " << (get_time() - t_start)/1000.0f << " "; // << "\r\n"; 

    //Store all collected results into the output file
    //CAUTION! Done for all locations of all monitored objects once processing is complete
    sinkStream->emitResults(annotated_only);  // User-specified: include/slip identifiers in the output for MULTIPLE objects and include/skip not annotated points

    //Report compression ratio
    cout << "Compression ratio: " << (scanStream->recCount - sinkStream->countAnnotatedLocations())/(1.0f * scanStream->recCount) << "\r\n";

    delete scanStream;	//Release scan operator
    delete sinkStream;  //Release sink operator with output results

    //Release states of all objects
    for (iterState = trajStates.begin(); iterState != trajStates.end(); iterState++ )
        delete iterState->second;

    return 0;
}
