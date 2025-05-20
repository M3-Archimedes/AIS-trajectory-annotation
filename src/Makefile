CPP = g++
OFLAG = -Wall -o
FLAGS += -g -std=c++11
LFLAG = -l
IFLAG = -I
LIBFLAG = -L
LIBDIR = /usr/local/lib/
INCLUDEDIR = /usr/local/include/
DEBUGF = -g -D DEBUG
DEBUG = no

Config.o: Config.h Config.cpp
	${CC} ${FLAGS} -c Config.cpp

Location.o: Location.h Location.cpp
	${CC} ${FLAGS} -c Location.cpp

Sink.o: Sink.h Sink.cpp
	${CC} ${FLAGS} -c Sink.cpp

Scan.o: Scan.h Scan.cpp
	${CC} ${FLAGS} -c Scan.cpp

State.o: State.h State.cpp
	${CC} ${FLAGS} -c State.cpp
	
main.o: main.cpp
	${CC} ${FLAGS} -c main.cpp
	
annotate: main.o Config.o Location.o Sink.o Scan.o State.o
		$(CPP) $(FLAGS) $(IFLAG) $(INCLUDEDIR) $(OFLAG) annotate main.o Config.o Location.o Scan.o Sink.o State.o
		rm -f *.o
		
clean:
	rm -f *.o
	rm -f *.exe
	rm -f *.exe.stackdump