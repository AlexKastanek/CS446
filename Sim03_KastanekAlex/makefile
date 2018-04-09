all: Sim03
Sim03: mainDriver.o Config.o MetaData.o PCB.o
	g++ -std=c++11 -pthread mainDriver.o Config.o MetaData.o PCB.o -o Sim03
mainDriver.o: mainDriver.cpp
	g++ -std=c++11 -pthread -c mainDriver.cpp
Config.o: Config.cpp
	g++ -std=c++11 -pthread -c Config.cpp
MetaData.o: MetaData.cpp
	g++ -std=c++11 -pthread -c MetaData.cpp
PCB.o: PCB.cpp
	g++ -std=c++11 -pthread -c PCB.cpp
clean:
	rm -rf *.o Sim03
