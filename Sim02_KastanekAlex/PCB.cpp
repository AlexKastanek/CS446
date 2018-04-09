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

}


/**
*	Function: PCB
*	Description: Parameterized constructor for PCB class with parameter pidSource used to 
*		initialize pid
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

/**
*	Function: getpid
*	Description: returns the pid for this process
*/
int PCB::getpid()
{

	return pid;
	
}

/**
*	Function: getStartTime
*	Description: returns the startTime of this process
*/
double PCB::getStartTime()
{

	return startTime;
	
}

/**
*	Function: getProcessDuration
*	Description: returns the duration of this process
*/
double PCB::getProcessDuration()
{

	return processDuration;
	
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

/**
*	Function: updateProcessDuration
*	Description: updates the processDuration by adding the parameter duration to it
*/
void PCB::updateProcessDuration(double duration)
{

	processDuration += duration;
	
}
