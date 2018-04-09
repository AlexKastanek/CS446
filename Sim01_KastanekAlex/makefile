all: Sim01
Sim01: mainDriver.o Config.o MetaData.o
	g++ -std=c++11 mainDriver.o Config.o MetaData.o -o Sim01
mainDriver.o: mainDriver.cpp
	g++ -std=c++11 -c mainDriver.cpp
Config.o: Config.cpp
	g++ -std=c++11 -c Config.cpp
MetaData.o: MetaData.cpp
	g++ -std=c++11 -c MetaData.cpp
clean:
	rm -rf *.o Sim01
