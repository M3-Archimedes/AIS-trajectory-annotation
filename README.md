# AIS-trajectory-annotation
This technique simulates streaming positions of vessels relayed through the Automatic Identification System (AIS) and identifies critical points along each trajetory. It applies spatiotemporal filters with user-specified parametrization in order to discard noisy positions and suitably annotate selected point locations with a particular characterization (stop, turn, communication gap, slow motion, etc.) in order to capture important changes across the route. The resulting annotated locations can be used to reconstruct a compressed, yet reliable approximation of the original trajectory by keeping a small fraction of the raw positions.


## Platform

Source code is written in GNU C++ and compiled with C++11 compiler. 

Only STL and Boost libraries are used. Please check files ```main.cpp``` and ```mobility.h``` for all required libraries.

Tested on these C++ compilers: gcc 5.4.0, gcc 11.4.0, gcc 13.3.0.

Compiled & tested at 64-bit environments: (a) Debian GNU/Linux 7.8 (wheezy); (b) Ubuntu 22.04.5 LTS; and (c) Cygwin terminal 2.11.1 in Windows 10.


## Compilation

From command line in a terminal, navigate to the path where you have extracted the source code.

### OPTION #1: Using the g++ compiler:

```
g++ -std=c++11 -o annotate main.cpp Config.cpp Location.cpp Sink.cpp Scan.cpp State.cpp
```

### OPTION #2: Using the accompanying Makefile:

```
make annotate
```

Note that (in both options) ```annotate``` is the name of the resulting executable file.


### Parametrization

Before executing the software, check (and optionally modify) the settings of the mobility tracking parameters in ```params.json```. You have to specify at least the _Default_ settings that can be generally applied to all types of vessels. Such default values can be superseded by corresponding parameters set specifically for particular vessel types (e.g., passenger, cargo, fishing). These parameters concern:

- _STATE_SIZE_: (integer) number of most recent raw point locations per vessel to be used in velocity vector computations.

- _STATE_TIMESPAN_: time interval in seconds (UNIX epochs) for keeping history of older positions for velocity vector computations.

- _GAP_PERIOD_: if time elapsed from previous location is above this value (in seconds), a communication GAP has occurred.

- _LOW_SPEED_THRESHOLD_: under this speed value in knots (1 knot = 1.852 kmh), the vessel is considered in _SLOW MOTION_.	 

- _MAX_SPEED_THRESHOLD_: over this speed value in knots (1 knot = 1.852 kmh), the location is considered as _NOISE_.

- _NO_SPEED_THRESHOLD_: under this speed value in knots (1 knot = 1.852 kmh), the vessel is considered _STOPPED_.

- _SPEED_RATIO_: change in speed by more than this percentage (e.g., 0.25) between two successive locations may indicate acceleration or deceleration.   

- -MAX_RATE_OF_CHANGE_: if rate of change in speed is above this value (in knots per hour), the location may be _NOISE_.

- _MAX_RATE_OF_TURN_: if rate of turn is above this value in degrees (azimuth) per second, the location may be _NOISE_.

- _DISTANCE_THRESHOLD-: under this distance (in meters) from its previous location, the vessel may be _STOPPED_.

- _ANGLE_THRESHOLD_: turning more than this angle (in degrees) from its previous location, a _CHANGE IN HEADING_ may have occurred.


## Usage

After successfully compiling the source code, navigate to the path where the executable (usually named ```annotate```) is located and execute it according to this command template in the terminal:

```
annotate [input-file] [id-attr] [timestamp-attr] [settings-json] [vessel-info-file] [output-file] [annotated-only]
```

where each argument has the following significance:

- _[input-file]_: Path to the input data (a ASCII space delimited file). The contents of the file are used to simulate a positional data stream based on the incoming AIS timestamped locations from vessels.

- _[id-attr]_: An integer specifying which attribute contains the vessel identifiers in input data; give a negative integer (e.g., -1) if NO vessel identifiers are included in the input data.

- _[timestamp-attr]_: An integer specifying which attribute contains the timestamp values in the input data (e.g., give 3, if timestamps are in the 3rd field in input file).

- _[settings-json]_: Path to JSON file with configuration settings per vessel type. 

- _[vessel-info-file]_: Path to a CSV file with vessel information. This semicolon-separated (';') file includes the general AIS type (e.g., passenger, cargo, fishing) of each vessel in the input data.

- _[output-file]_: Path to the output (ASCII space delimited file) collecting points with the detected annotations.

- _[annotated-only]_: Boolean controlling which points will be emitted. If ```true```, only points with detected annotations will be stored into the output file; otherwise, normal (i.e., not annotated) and noisy points (annotated as _NOISE_) will be also included in the output.

### Example:

```
./annotate ./input/vessel1.csv -1 3 ./settings/params.json ./settings/vessel_info.csv ./output/vessel1_annotated.csv false
```


## Input

The framework accepts a _space separated_ ASCII file of timestamped positions _without column headings_. This file may include locations of a SINGLE or MULTIPLE vessels. 

- In case of a _SINGLE_ vessel, the input must conform to the following schema:

```
< lon lat t >
```

- In case of _MULTIPLE_ vessels, the input must conform to the following schema:

```
< id lon lat t >
```

where the attributes have the following significance:

```id```     (LONG INTEGER)      -> vessel identifier (e.g., a serial number or the vessel's MMSI)
```lon```    (DOUBLE PRECISION)  -> longitude georeferenced in EPSG:4326 (WGS84) system
```lat```    (DOUBLE PRECISION)  -> latitude  georeferenced in EPSG:4326 (WGS84) system
```t```      (LONG INTEGER)      -> timestamp of the position (UNIX epochs, i.e., seconds since 00:00:00 on 1 January 1970)

_CAUTION!_ Rows in the input file must be sorted by ascending timestamp value. Input file must _NOT_ end with an empty line. 

Please take a look at [this folder](test/input) for sample input data concerning individual as well as multiple vessels.


## Output

Annotated point locations identified per vessel can be stored in a _space separated_ ASCII file. At runtime, through argument ```[annotated-only]``` users can control whether the output will include only the annotated points (set to ```true```) or all original points (```false```). 

This is the eecord schema (space separated values) in the output file: 

```
< id lon lat t speed heading annotation >
```

where the attributes have the following significance:

```id```        (LONG INTEGER)      -> vessel identifier (e.g., a serial number or the vessel's MMSI)
```lon```       (DOUBLE PRECISION)  -> longitude georeferenced in EPSG:4326 (WGS84) system
```lat```       (DOUBLE PRECISION)  -> latitude  georeferenced in EPSG:4326 (WGS84) system
```t```         (LONG INTEGER)      -> timestamp of the position (UNIX epochs, i.e., seconds since 00:00:00 on 1 January 1970)
```speed```     (DOUBLE PRECISION)  -> instantaneous speed (in knots)
```heading```   (DOUBLE PRECISION)  -> instantaneous heading (azimuth in degrees)
```annotation```(STRING)            -> annotation(s) characterizing this location as a critical point 

_CAUTION!_ If input concerns a SINGLE vessel, its ID may be ommitted from output. Rows in the output file are sorted by ascending timestamp value. 

_NOTE:_ The same point location may have been assigned none, one or even _multiple_ annotations (e.g., SLOW_MOTION_END; STOP_START). In the output file, multiple annotations are separated by ';'.

Please take a look at [this folder](test/ouput) for indicative annotated results computed using these [settings](test/settings) over various sample input data available [here](test/input).


## Annotations

These are the annotations that can be possibly assigned to a vessel's location:

- *STOP_START* -> the object has just stopped moving and became stationary at this position.

- *STOP_END* -> the object is no longer stationary and has just started moving (w.r.t. its previously known raw position).

- *CHANGE_IN_SPEED_START* -> speed over ground has just changed significantly (by a threshold parameter) w.r.t. the previously known speed.

- *CHANGE_IN_SPEED_END* -> speed over ground no longer diverges from the average speed over the most recent portion of the trajectory.

- *SLOW_MOTION_START* -> this is the first position reported by the object when moving at a very low speed (below the threshold parameter).

- *SLOW_MOTION_END* -> this is the last position reported by the object when moving at a very low speed (below the threshold parameter).

- *GAP_START -> communication with this object was lost at this position (i.e., this is the last location reported just before a communication gap).

- *GAP_END* -> communication with this object has been restored at this position (i.e., this is the first location reported after a communication gap).

- *CHANGE_IN_HEADING* -> this is a turning point along the trajectory of this object, i.e., its actual heading over ground has just changed significantly (above the threshold angle parameter) w.r.t. its previous heading.

- *NOISE* -> this location qualifies as noise and should be discarded.

Note that the same point location may have _multiple_ annotations (e.g., SLOW_MOTION_END; STOP_START). In the output file, such annotations are separated by ';'.


## Acknowledgements

This processing framework has been developed over a long period under various Greek and EU-funded projects:

1) An initial version of the online tracking module was partially developed during a Short Term Scientific Mission (25/3/2013-19/4/2013) at the Institut de Recherche de l'École navale (Brest, France) with the support of [EU COST Action IC0903 MOVE on Knowledge Discovery from Moving Objects](http://www.move-cost.info/). A short preliminary report describing the basic framework was published in [COST MOVE Workshop on Moving Objects at Sea](https://sites.google.com/site/movingobjectsatsea/accepted-papers). Helpful discussions with Dr. Cyril Ray and Prof. Christophe Claramunt during this STSM are kindly acknowledged.

2) A more enhanced version of this work was carried out in the framework of the project **AMINESS**: _Analysis of Marine INformation for Environmentally Safe Shipping_ co-financed by the European Fund for Regional Development and from Greek National funds through the operational programs "Competitiveness and Enterpreneurship" and "Regions in Transition" of the National Strategic Reference Framework - Action: "COOPERATION 2011 -- Partnerships of Production and Research Institutions in Focused Research and Technology Sectors". This version has been completed when collaborating (2014-2015) with the [Information Management Lab](https://infolab.cs.unipi.gr/) at the University of Pireaus. More information about this enhanced methodology can be found at two published papers:
 
* K. Patroumpas, A. Artikis, N. Katzouris, M. Vodas, Y. Theodoridis, and N. Pelekis. [Event Recognition for Maritime Surveillance](https://openproceedings.org/2015/conf/edbt/paper-364.pdf). In Proceedings of the 18th International Conference on Extending Database Technology (EDBT'15), pp. 629-640, Brussels, Belgium, March 2015.

* K. Patroumpas, E. Alevizos, A. Artikis, M. Vodas, N. Pelekis, and Y. Theodoridis. [Online Event Recognition from Moving Vessel Trajectories](https://cer.iit.demokritos.gr/publications/papers/2017/artikis-geoinformatica16.pdf). GeoInformatica, 21(2):389-427, 2017.

3) An even more advanced framework for trajectory detection & summarization for vessel and aircraft trajectories was developed for project **datAcron** _Big Data Analytics for Time Critical Mobility Forecasting_  under European Union's Horizon 2020 Programme grant No. 687591 in 2017-2018. This software concerning a [Trajectory Synopses Generator](https://github.com/DataStories-UniPi/Trajectory-Synopses-Generator) was built in Scala on Apache Flink 0.10.2 platform and makes use of Apache Kafka for processing streaming positional updates in real-time. This publication details the methodology as specifically applied for aircraft trajecgories in the aviation domain:

* K. Patroumpas, N. Pelekis, Y. Theodoridis (2018) [On-the-fly mobility event detection over aircraft trajectories](https://dl.acm.org/doi/10.1145/3274895.3274970). In Proceedings of the 26th ACM SIGSPATIAL International Conference on Advances in Geographic Information Systems (SIGSPATIAL/GIS), pp. 259 - 268, 2018.

4) The current version focuses strictly on vessel trajectories at sea and allows user-specified custom parametrization per vessel type (e.g., passenger, cargo, fishing) using more fine-tuned rules for annotating each vessel's course. This work has been supported by the [ARCHIMEDES Unit](https://www.athenarc.gr/en/archimedes) at Athena Research Center under project MIS 5154714 of the National Recovery and Resilience Plan Greece 2.0 funded by the European Union under the NextGenerationEU Program.


