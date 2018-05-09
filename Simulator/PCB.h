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
		
		bool isInterrupted();
		bool hasBeenInterrupted();
		int getpid();
		int getBlockCount();
		int getLastAddress();
		int getQueueIndex();
		int getHardDriveQuant();
		int getHardDrivesUsed();
		int getProjectorQuant();
		int getProjectorsUsed();
		double getStartTime();
		double getProcessDuration();
		double getEstimatedProcessTime();
		double getEstimatedTimeRemaining();
		
		void interrupt();
		void setInterrupt(bool);
		void setInterruptOccurred(bool);
		void setpid(int);
		void setBlockCount(int);
		void setLastAddress(int);
		void setQueueIndex(int);
		void setHardDriveQuant(int);
		void setHardDrivesUsed(int);
		void setProjectorQuant(int);
		void setProjectorsUsed(int);
		void setStartTime(double);
		void setProcessDuration(double);
		void setEstimatedProcessTime(double);
		void setEstimatedTimeRemaining(double);
		void updateProcessDuration(double);
		void incrementBlockCount();
		void incrementHardDrivesUsed();
		void incrementProjectorsUsed();
		
		void saveState(double);
		double loadState();
		
		int processState;
		
	private:
	
		bool interrupted, interruptHasOccurred;
	
		int pid; //process ID
		int blockCount, lastAddress, queueIndex;
		int hardDriveQuant, hardDrivesUsed, projectorQuant, projectorsUsed;
		
		double startTime, processDuration, estimatedProcessTime, estimatedTimeRemaining;
		double processData;

};

#endif
