/**
*	File Name: PCB.h
*	Editors: Alex Kastanek
*	Project: CS446 Operating Systems Simulation
*	File Description: Header file for the class PCB. Used to store the data and control 
*		the state of a particular process
*	Version: 01
*	Last Date Revised: 2/27/18
*/

#ifndef PCB_
#define PCB_

#include <iostream>
#include <deque>
#include <string>
#include <pthread.h>
#include <time.h>
#include "MetaData.h"

using namespace std;

class PCB
{

	public:
		
		PCB();
		PCB(int);
		~PCB();
		
		int getpid();
		int getBlockCount();
		int getLastAddress();
		int getHardDriveQuant();
		int getHardDrivesUsed();
		int getProjectorQuant();
		int getProjectorsUsed();
		double getStartTime();
		double getProcessDuration();
		
		void setpid(int);
		void setBlockCount(int);
		void setLastAddress(int);
		void setHardDriveQuant(int);
		void setHardDrivesUsed(int);
		void setProjectorQuant(int);
		void setProjectorsUsed(int);
		void setStartTime(double);
		void setProcessDuration(double);
		void updateProcessDuration(double);
		void incrementBlockCount();
		void incrementHardDrivesUsed();
		void incrementProjectorsUsed();
		
		int processState;
		
	private:
	
		int pid; //process ID
		int blockCount, lastAddress;
		int hardDriveQuant, hardDrivesUsed, projectorQuant, projectorsUsed;
		
		double startTime, processDuration;

};

#endif
