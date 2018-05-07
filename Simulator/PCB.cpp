/**
*	File Name: PCB.cpp
*	Editors: Alex Kastanek
*	Project: CS446 Operating Systems Simulation
*	File Description: Implementation file for the class PCB. Used to store the data and 
*		control the state of a particular process
*	Version: 01
*	Last Date Revised: 2/27/18
*/

#include "PCB.h"

/**
*	Function: PCB
*	Description: Default constructor for PCB class
*/
PCB::PCB()
{

	processState = 0;
	blockCount = 0;
	lastAddress = 0;
	hardDriveQuant = 0;
	hardDrivesUsed = 0;
	projectorQuant = 0;
	projectorsUsed = 0;

}


/**
*	Function: PCB
*	Description: Parameterized constructor for PCB class with parameter pidSource used 
*		to initialize pid
*/
PCB::PCB(int pidSource)
{

	processState = 0;
	pid = pidSource;

}

/**
*	Function: ~PCB
*	Description: Destructor for PCB class
*/
PCB::~PCB()
{

	processState = 4;

}

bool PCB::isInterrupted()
{

	return interrupted;

}

bool PCB::hasBeenInterrupted()
{

	return interruptHasOccurred;

}

/**
*	Function: getpid
*	Description: returns the pid for this process
*/
int PCB::getpid()
{

	return pid;
	
}

/**
*	Function: getBlockCount
*	Description: returns the memory block count for this process
*/
int PCB::getBlockCount()
{

	return blockCount;
	
}

/**
*	Function: getLastAddress()
*	Description: returns the last address location used for this process
*/
int PCB::getLastAddress()
{

	return lastAddress;
	
}

/**
*	Function: getHardDriveQuant()
*	Description: returns the max quantity of hard drives for this process
*/
int PCB::getHardDriveQuant()
{

	return hardDriveQuant;
	
}

/**
*	Function: getHardDrivesUsed()
*	Description: returns the amount of hard drives used for this process
*/
int PCB::getHardDrivesUsed()
{

	return hardDrivesUsed;
	
}

/**
*	Function: getProjectorQuant()
*	Description: returns the max quantity of projectors for this process
*/
int PCB::getProjectorQuant()
{

	return projectorQuant;
	
}

/**
*	Function: getProjectorsUsed()
*	Description: returns the amount of projectors used for this process
*/
int PCB::getProjectorsUsed()
{

	return projectorsUsed;
	
}

/**
*	Function: getStartTime
*	Description: returns the startTime of this process
*/
double PCB::getStartTime()
{

	return startTime;
	
}

double PCB::getEstimatedProcessTime()
{

	return estimatedProcessTime;

}

double PCB::getEstimatedTimeRemaining()
{

	return estimatedTimeRemaining;

}

/**
*	Function: getProcessDuration
*	Description: returns the duration of this process
*/
double PCB::getProcessDuration()
{

	return processDuration;
	
}

void PCB::interrupt()
{

	interrupted = true;
	interruptHasOccurred = true;

}

void PCB::setInterrupt(bool source)
{

	interrupted = source;
	if (source)
	{
		interruptHasOccurred = true;
	}

}

/**
*	Function: setpid
*	Description: sets the pid to the parameter pidSource
*/
void PCB::setpid(int pidSource)
{

	pid = pidSource;
	
}

/**
*	Function: setBlockCount
*	Description: sets the block count to the parameter source
*/
void PCB::setBlockCount(int source)
{

	blockCount = source;
	
}

/**
*	Function: setLastAddress
*	Description: sets the last address used to the parameter source
*/
void PCB::setLastAddress(int source)
{

	lastAddress = source;
	
}

/**
*	Function: setHardDriveQuant
*	Description: sets the hard drive quantity to the parameter source
*/
void PCB::setHardDriveQuant(int source)
{

	hardDriveQuant = source;
	
}

/**
*	Function: setHardDrivesUsed
*	Description: sets the amount of hard drives used to the parameter source
*/
void PCB::setHardDrivesUsed(int source)
{

	hardDrivesUsed = source;
	
}

/**
*	Function: setProjectorQuant
*	Description: sets the projector quantity to the parameter source
*/
void PCB::setProjectorQuant(int source)
{

	projectorQuant = source;
	
}

/**
*	Function: setProjectorsUsed
*	Description: sets the amount of projectors used to the parameter source
*/
void PCB::setProjectorsUsed(int source)
{

	projectorsUsed = source;
	
}

/**
*	Function: setStartTime
*	Description: sets the startTime to the parameter source
*/
void PCB::setStartTime(double source)
{

	startTime = source;

}

/**
*	Function: setProcessDuration
*	Description: sets the processDuration to the parameter source
*/
void PCB::setProcessDuration(double source)
{

	processDuration = source;
	
}

void PCB::setEstimatedProcessTime(double source)
{

	estimatedProcessTime = source;

}

void PCB::setEstimatedTimeRemaining(double source)
{

	estimatedTimeRemaining = source;

}

/**
*	Function: updateProcessDuration
*	Description: updates the processDuration by adding the parameter duration to it
*/
void PCB::updateProcessDuration(double duration)
{

	processDuration += duration;
	
}

/**
*	Function: incrementBlockCount()
*	Description: adds 1 to the block count
*/
void PCB::incrementBlockCount()
{

	blockCount++;

}

/**
*	Function: incrementHardDrivesUsed()
*	Description: adds 1 to the hard drives used
*/
void PCB::incrementHardDrivesUsed()
{

	hardDrivesUsed++;

}

/**
*	Function: incrementProjectorsUsed()
*	Description: adds 1 to the projectors used
*/
void PCB::incrementProjectorsUsed()
{

	projectorsUsed++;

}

void PCB::saveState(double state)
{

	processData = state;

}

double PCB::loadState()
{

	return processData;

}





