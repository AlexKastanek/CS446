/**
*	File Name: mainDriver.cpp
*	Editors: Alex Kastanek
*	Project: CS446 Operating Systems Simulation
*	File Description: Main Driver for Input/Output portion of the Simulation
*	Version: 01
*	Last Date Revised: 2/7/18
*/

//library inclusion and directives

#include <iostream>
#include <fstream>
#include <deque>
#include <string>
#include <pthread.h>
#include <limits.h>
#include <time.h>
#include "Config.h"
#include "MetaData.h"
#include "PCB.h"

using namespace std;

//Global variables
ofstream fout;
int outputType;

//function headers

bool getMetaData(ifstream&, Config, deque<MetaData>&);

bool runProgram(Config, deque<MetaData>, int);
bool handleProcess(Config, MetaData, PCB&);
bool systemHandler(PCB&, string);
bool applicationHandler(PCB&, string);
bool processorHandler(PCB&, int);
bool memoryHandler(PCB&, int, string, int);
bool inputHandler(PCB&, string, int);
bool outputHandler(PCB&, string, int);

unsigned int generateMemoryAddress();
bool allocateMemory(unsigned int, int);

void* timer(void*);
void* hardDriveInputHandler(void*);
void* keyboardHandler(void*);
void* scannerHandler(void*);
void* hardDriveOutputHandler(void*);
void* monitorHandler(void*);
void* projectorHandler(void*);

void output(Config, deque<MetaData>);
void outputToMonitor(Config, deque<MetaData>);
void outputToFile(Config, deque<MetaData>);

/**
*	Function: Main Driver for Operating System Simulation
*	Description: Manages the main operations of the simulation. Takes command line 
*		arguments as parameters, and these are used for configuration filenames. The 
*		function contains an object of class Config called configData, which stores all 
*		the data in the config file. Also currently contains a double ended queue which 
*		is used to store the meta-data instruction set. Returns a 1 if no errors 
*		occurred, and a 0 if an error did occur.
*/
int main(int argc, char *argv[])
{

	//local variable declaration

	Config configData[argc - 1]; //Config ADT used to store data from the config file
	deque<MetaData> instructionSet[argc - 1]; //double-ended queue of the ADT MetaData - 
										  //used to store data from an instruction in 
										  //the Meta Data file

	//begin command line argument error checking and config file input
	ifstream fin;
	for (int i = 0; i < argc; i++)
	{
		fin.open(argv[i]);
		if (!fin.is_open())
		{
			//file specified in command line argument does not exist
			cout << "ERROR: Incorrect file name in command line: command number " << i <<
			endl;
			return -1;
		}
		else if (i > 0)
		{
			//storing config file data in config class
			configData[i - 1].getConfigData(fin);
		}
		fin.close();
	}
	
	for (int i = 0; i < argc - 1; i++)
	{
		string metaDataFile; //used to store the filename of the meta data file
		bool okToContinue; //used to stop the program if error ocurred
	
		//assigning metaDataFile to file name specified in config data
		metaDataFile = configData[i].getFilePath();

		//begin meta-data file error checking and input
		fin.open(metaDataFile);
		if (!fin.is_open())
		{
			//file does not exist
			cout << "ERROR: Meta-Data file specified in config file does not exist" << 
			endl;
			return -1;
		}
		else
		{
			//storing meta data file to instructionSet - a double-ended queue of Meta-
			//Data
			//okToContinue is true if no errors occur during the data retrieval process
			okToContinue = getMetaData(fin, configData[i], instructionSet[i]);
		}
		fin.close();
	
		//begin output
		if (okToContinue)
		{
			output(configData[i], instructionSet[i]);
		}
		else
		{
			//cout << "ERROR: Meta-Data could not be stored" << endl;
			return -1;
		}
		
		if (okToContinue)
		{
			outputType = configData[i].getLogType();
			okToContinue = runProgram(configData[i], instructionSet[i], i);
		}
		else
		{
			cout << "ERROR: Program " << i << " could not be instantiated" << endl;
			return -1;
		}
		
		fout.close();
	
	}

	return 1;
			
}

/**
*	Function: getMetaData
*	Description: Gets input from the specified meta-data file (specified in 
*		configuration file) and stores the instructions that are contained in the file 
*		to individual objects of the class MetaData. These objects are pushed onto the 
*		instructionSet - a double ended queue. configData is needed as a parameter to 
*		calculate the correct total time of the component. Returns true if no errors 
*		occurred and false if an error did occur.
*/
bool getMetaData(ifstream& fin, Config configData, deque<MetaData>& instructionSet)
{

	string line, instruction;
	
	//checking starting line
	getline(fin, line);
	if (line != "Start Program Meta-Data Code:")
	{
		cout << "ERROR: Typo in meta data file" << endl;
		return 0;
	}
	line.clear();
	
	//begin meta-data extraction process
	fin >> instruction;
	//this loop stops once the last character in the input string is not a semi-colon
	while (instruction[instruction.length() - 1] == ';')
	{
		MetaData currentData; //temporary storage, will be stored in instructionSet 
							  //once all errors are checked
		if (!currentData.parseMetaData(instruction))
		{
			//error occurred while parsing meta-data
			//cout << "ERROR: Error in meta-data" << endl;
			return 0;
		}
		currentData.calculateTotalTime(configData);
		instructionSet.push_back(currentData); //adding instruction to the set
		instruction.clear();
		fin >> instruction;
		if (instruction.substr(2, 4) == "hard")
		{
			//checking if descriptor is case "hard drive" which would stop the loop if 
			//not accounted for		
			string restOfInstruction;
			fin >> restOfInstruction;
			//continuing instruction by appending the space and the rest of the 
			//instruction
			instruction.append(" ");
			instruction.append(restOfInstruction);
		}
	}
	//once loop stops, there should be one more meta-data instruction to extract
	if (instruction[instruction.length() - 1] == '.')
	{
		MetaData currentData;
		if (!currentData.parseMetaData(instruction))
		{
			cout << "ERROR: Error in meta data" << endl;
			return 0;
		}
		currentData.calculateTotalTime(configData);
		instructionSet.push_back(currentData);
		instruction.clear();
	}
	else
	{
		cout << "ERROR: No '.' found to end meta data scanning" << endl;
		return 0;
	}
	
	//checking ending line
	getline(fin, line);
	getline(fin, line);
	if (line != "End Program Meta-Data Code.")
	{
		cout << "ERROR: Typo in meta data file" << endl;
		return 0;
	}
	line.clear();
	
	return 1;

}

/**
*	Function: runProgram
*	Description: Runs a set of Meta Data insturctions called a program. Keeps track of 
*		the time the program takes to run. Begins and ends the logging process of 
*		actions taken by the program and processes inside the program. Takes parameters 
*		configData, program, and programNum to know the configuration of the devices 
*		the program uses as well as the instructions of the program and the number of 
*		the program.
*/
bool runProgram(Config configData, deque<MetaData> program, int programNum)
{

	PCB programData;
	clock_t start;
	double duration;
	bool okToContinue = 1;
	
	//setting parameters of program's process control block
	programData.setpid(programNum + 1);
	programData.setStartTime(0);
	programData.setProcessDuration(0);
	
	//starting program clock and initializing duration
	start = clock();
	duration = (clock() - start) / (double) CLOCKS_PER_SEC;
	
	//using switch statement to output to file, monitor, or both
	switch (outputType)
	{
		case 0:
			cout << fixed << duration << " - Simulator program starting" << endl;
		break;
		case 1:
			fout << fixed << duration << " - Simulator program starting" << endl;
		break;
		case 2:
			cout << fixed << duration << " - Simulator program starting" << endl;
			fout << fixed << duration << " - Simulator program starting" << endl;
		break;
		default:
			cout << "ERROR: Incorrect log type recorded from config file" << endl;
			return 0;
	}
	
	//loops through each meta data instruction and calls handleProcess() to handle each 
	//process
	for (int i = 0; i < program.size(); i++)
	{
		if (okToContinue)
		{
			duration = ((clock() - start) / (double) CLOCKS_PER_SEC);
			programData.setProcessDuration(duration);
			okToContinue = handleProcess(configData, program[i], programData);
		}
		else
		{
			return 0;
		}
		duration = ((clock() - start) / (double) CLOCKS_PER_SEC);
		//cout << "clock test: " << fixed << duration << endl;
	}
	
	switch (outputType)
	{
		case 0:
			cout << fixed << duration << " - Simulator program ending" << endl;
		break;
		case 1:
			fout << fixed << duration << " - Simulator program ending" << endl;
		break;
		case 2:
			cout << fixed << duration << " - Simulator program ending" << endl;
			fout << fixed << duration << " - Simulator program ending" << endl;
		break;
		default:
			cout << "ERROR: Incorrect log type recorded from config file" << endl;
			return 0;
	}
	
	return 1;

}

/**
*	Function: handleProcess
*	Description: chooses, based on the process being passed as argument, what handler 
*		to use for the process. Will call either systemHandler(), applicationHandler(), 
*		processorHandler(), memoryHandler(), inputHandler(), or outputHandler() 
*		depending on the type of process. Also takes cData and pData as parameters to 
*		pass to the other handlers
*/
bool handleProcess(Config cData, MetaData process, PCB& pData)
{

	switch ((int) process.getCode())
	{
	
		case (int) 'S':
		
			return systemHandler(pData, process.getDescriptor());
		
		break;
		
		case (int) 'A':
		
			return applicationHandler(pData, process.getDescriptor());
		
		break;
		
		case (int) 'P':
		
			return processorHandler(pData, process.getTotalTime());
		
		break;
		
		case (int) 'M':
		
			return memoryHandler(pData, cData.getSystemMemory(), 
				   				 process.getDescriptor(), process.getTotalTime());
		
		break;
		
		case (int) 'I':
		
			return inputHandler(pData, process.getDescriptor(), process.getTotalTime());
					
		break;
		
		case (int) 'O':
		
			return outputHandler(pData, process.getDescriptor(), 
				   				 process.getTotalTime());
		
		break;
		
		default:
		
			cout << "ERROR: Invalid code encountered in Meta Data" << endl;
			return 0;
	
	}

}

/**
*	Function: systemHandler
*	Description: Prepares and starts a process as well as ends it. Updates the duration 
*		of the process contained in the process' PCB. 
*/
bool systemHandler(PCB& pData, string descriptor)
{

	clock_t start;
	int pid = pData.getpid();
	double duration;
	
	start = clock();

	if (descriptor == "begin")
	{
		switch (outputType)
		{
			case 0:
				cout << pData.getProcessDuration() << " - OS: preparing process " << 
				pid << endl;
			break;
			case 1:
				fout << pData.getProcessDuration() << " - OS: preparing process " << 
				pid << endl;
			break;
			case 2:
				cout << pData.getProcessDuration() << " - OS: preparing process " << 
				pid << endl;
				fout << pData.getProcessDuration() << " - OS: preparing process " << 
				pid << endl;
			break;
			default:
				cout << "ERROR: Incorrect log type recorded from config file" << endl;
				return 0;
		}
		duration = (clock() - start) / (double) CLOCKS_PER_SEC;
		pData.updateProcessDuration(duration);
		pData.processState = 1;
		switch (outputType)
		{
			case 0:
				cout << pData.getProcessDuration() << " - OS: starting process " << pid 
				<< endl;
			break;
			case 1:
				fout << pData.getProcessDuration() << " - OS: starting process " << pid 
				<< endl;
			break;
			case 2:
				cout << pData.getProcessDuration() << " - OS: starting process " << pid 
				<< endl;
				fout << pData.getProcessDuration() << " - OS: starting process " << pid 
				<< endl;
			break;
			default:
				cout << "ERROR: Incorrect log type recorded from config file" << endl;
				return 0;
		}
	}
	else if (descriptor == "finish")
	{
		switch (outputType)
		{
			case 0:
				cout << pData.getProcessDuration() << " - OS: removing process " << pid 
				<< endl;
			break;
			case 1:
				fout << pData.getProcessDuration() << " - OS: removing process " << pid 
				<< endl;
			break;
			case 2:
				cout << pData.getProcessDuration() << " - OS: removing process " << pid 
				<< endl;
				fout << pData.getProcessDuration() << " - OS: removing process " << pid 
				<< endl;
			break;
			default:
				cout << "ERROR: Incorrect log type recorded from config file" << endl;
				return 0;
		}
		pData.processState = 4;
	}
	else
	{
		cout << "ERROR: Incorrect descriptor recorded for Meta-Data code 'S'" << endl;
		return 0;
	}
	
	return 1;

}

/**
*	Function: applicationHandler
*	Description: returns 1 for now
*/
bool applicationHandler(PCB& pData, string descriptor)
{
	
	return 1;

}

/**
*	Function: processorHandler
*	Description: Runs a process for the correct amount of time by using a timer thread. 
*		While the thread counts down, executes the process. Currently there is nothing 
*		else to be executed. Updates the duration of the process contained in the 
*		process' PCB. 
*/
bool processorHandler(PCB& pData, int processTime)
{

	pthread_t timerThread;
	clock_t start;
	long pTime = (long) processTime;
	double duration;
	int rc, pid = pData.getpid();
	
	start = clock();
	
	//preparing process
	duration = (clock() - start) / (double) CLOCKS_PER_SEC;
	pData.updateProcessDuration(duration);
	switch (outputType)
	{
		case 0:
			cout << pData.getProcessDuration() << " - Process " << pid << 
			": start processing action" << endl;
		break;
		case 1:
			fout << pData.getProcessDuration() << " - Process " << pid << 
			": start processing action" << endl;
		break;
		case 2:
			cout << pData.getProcessDuration() << " - Process " << pid << 
			": start processing action" << endl;
			fout << pData.getProcessDuration() << " - Process " << pid << 
			": start processing action" << endl;
		break;
		default:
			cout << "ERROR: Incorrect log type recorded from config file" << endl;
			return 0;
	}
	
	//running process
	pData.processState = 2;
	rc = pthread_create(&timerThread, NULL, &timer, (void*)pTime);
	if (rc)
	{
		cout << "ERROR: return code from pthread_create() is " << rc << endl;
		exit(-1);
	}
	
	//more functionality would go here
	
	pthread_join(timerThread, NULL);
	
	//ending process
	pData.processState = 1;
	duration = ((clock() - start) / (double) CLOCKS_PER_SEC) - duration;
	pData.updateProcessDuration(duration);
	switch (outputType)
	{
		case 0:
			cout << pData.getProcessDuration() << " - Process " << pid << 
			": end processing action" << endl;
		break;
		case 1:
			fout << pData.getProcessDuration() << " - Process " << pid << 
			": end processing action" << endl;
		break;
		case 2:
			cout << pData.getProcessDuration() << " - Process " << pid << 
			": end processing action" << endl;
			fout << pData.getProcessDuration() << " - Process " << pid << 
			": end processing action" << endl;
		break;
		default:
			cout << "ERROR: Incorrect log type recorded from config file" << endl;
			return 0;
	}
	
	return 1;

}

/**
*	Function: memoryHandler
*	Description: Runs a memory process for the correct amount of time by using a timer 
*		thread. While the thread counts down, generates an address location to allocate 
*		memory to. Updates the duration of the process contained in the process' PCB. 
*/
bool memoryHandler(PCB& pData, int sysMemory, string descriptor, int processTime)
{

	pthread_t timerThread;
	clock_t start;
	long pTime = (long) processTime;
	double duration;
	int rc, pid = pData.getpid();
	
	start = clock();
	
	if (descriptor == "allocate")
	{
		unsigned int addr;
	
		//preparing process
		duration = (clock() - start) / (double) CLOCKS_PER_SEC;
		pData.updateProcessDuration(duration);
		switch (outputType)
		{
			case 0:
				cout << pData.getProcessDuration() << " - Process " << pid <<
				": allocating memory" << endl;
			break;
			case 1:
				fout << pData.getProcessDuration() << " - Process " << pid <<
				": allocating memory" << endl;
			break;
			case 2:
				cout << pData.getProcessDuration() << " - Process " << pid <<
				": allocating memory" << endl;
				fout << pData.getProcessDuration() << " - Process " << pid <<
				": allocating memory" << endl;
			break;
			default:
				cout << "ERROR: Incorrect log type recorded from config file" << endl;
				return 0;
		}
		
		//running process
		pData.processState = 2;
		rc = pthread_create(&timerThread, NULL, &timer, (void*)pTime);
		if (rc)
		{
			cout << "ERROR: return code from pthread_create() is " << rc << endl;
			exit(-1);
		}
		
		addr = generateMemoryAddress();
		if (!allocateMemory(addr, sysMemory))
		{
			return 0;
		}
		
		pthread_join(timerThread, NULL);
		
		//ending process
		pData.processState = 1;
		duration = ((clock() - start) / (double) CLOCKS_PER_SEC) - duration;
		pData.updateProcessDuration(duration);
		switch (outputType)
		{
			case 0:
				cout << pData.getProcessDuration() << " - Process " << pid << 
				": memory allocated at 0x" << hex << addr << endl << dec;
			break;
			case 1:
				fout << pData.getProcessDuration() << " - Process " << pid << 
				": memory allocated at 0x" << hex << addr << endl << dec;
			break;
			case 2:
				cout << pData.getProcessDuration() << " - Process " << pid << 
				": memory allocated at 0x" << hex << addr << endl << dec;
				fout << pData.getProcessDuration() << " - Process " << pid << 
				": memory allocated at 0x" << hex << addr << endl << dec;
			break;
			default:
				cout << "ERROR: Incorrect log type recorded from config file" << endl;
				return 0;
		}
		
	}
	else if (descriptor == "block")
	{
	
		//preparing process
		duration = (clock() - start) / (double) CLOCKS_PER_SEC;
		pData.updateProcessDuration(duration);
		switch (outputType)
		{
			case 0:
				cout << pData.getProcessDuration() << " - Process " << pid << 
				": start memory blocking" << endl;
			break;
			case 1:
				fout << pData.getProcessDuration() << " - Process " << pid << 
				": start memory blocking" << endl;
			break;
			case 2:
				cout << pData.getProcessDuration() << " - Process " << pid << 
				": start memory blocking" << endl;
				fout << pData.getProcessDuration() << " - Process " << pid << 
				": start memory blocking" << endl;
			break;
			default:
				cout << "ERROR: Incorrect log type recorded from config file" << endl;
				return 0;
		}
	
		//running process
		pData.processState = 2;
		rc = pthread_create(&timerThread, NULL, &timer, (void*)pTime);
		if (rc)
		{
			cout << "ERROR: return code from pthread_create() is " << rc << endl;
			exit(-1);
		}
	
		//more functionality would go here
	
		pthread_join(timerThread, NULL);
	
		//ending process
		pData.processState = 1;
		duration = ((clock() - start) / (double) CLOCKS_PER_SEC) - duration;
		pData.updateProcessDuration(duration);
		switch (outputType)
		{
			case 0:
				cout << pData.getProcessDuration() << " - Process " << pid << 
				": end memory blocking" << endl;
			break;
			case 1:
				fout << pData.getProcessDuration() << " - Process " << pid << 
				": end memory blocking" << endl;
			break;
			case 2:
				cout << pData.getProcessDuration() << " - Process " << pid << 
				": end memory blocking" << endl;
				fout << pData.getProcessDuration() << " - Process " << pid << 
				": end memory blocking" << endl;
			break;
			default:
				cout << "ERROR: Incorrect log type recorded from config file" << endl;
				return 0;
		}
		
	}
	else
	{
		cout << "ERROR: Incorrect descriptor recorded for Meta-Data code 'M'" << endl;
		return 0;
	}

}

/**
*	Function: inputHandler
*	Description: Runs a process for an input device for the correct amount of time by 
*		using a timer thread. While the thread counts down, executes the process in 
*		another thread. Currently there is nothing else to be executed. Updates the 
*		duration of the process contained in the process' PCB. 
*/
bool inputHandler(PCB& pData, string descriptor, int processTime)
{

	pthread_t inputThread;
	clock_t start;
	long pTime = (long) processTime;
	double duration;
	int rc, pid = pData.getpid();
	
	start = clock();
	
	//preparing process
	duration = (clock() - start) / (double) CLOCKS_PER_SEC;
	pData.updateProcessDuration(duration);
	switch (outputType)
	{
		case 0:
			cout << pData.getProcessDuration() << " - Process " << pid <<
			": start " << descriptor << " input" << endl;
		break;
		case 1:
			fout << pData.getProcessDuration() << " - Process " << pid <<
			": start " << descriptor << " input" << endl;
		break;
		case 2:
			cout << pData.getProcessDuration() << " - Process " << pid <<
			": start " << descriptor << " input" << endl;
			fout << pData.getProcessDuration() << " - Process " << pid <<
			": start " << descriptor << " input" << endl;
		break;
		default:
			cout << "ERROR: Incorrect log type recorded from config file" << endl;
			return 0;
	}
	
	if (descriptor == "hard drive")
	{
	
		//running process
		pData.processState = 2;
		rc = pthread_create(&inputThread, NULL, &hardDriveInputHandler, (void*)pTime);
		if (rc)
		{
			cout << "ERROR: return code from pthread_create() is " << rc << endl;
			exit(-1);
		}
	
	}
	else if (descriptor == "keyboard")
	{
	
		//running process
		pData.processState = 2;
		rc = pthread_create(&inputThread, NULL, &keyboardHandler, (void*)pTime);
		if (rc)
		{
			cout << "ERROR: return code from pthread_create() is " << rc << endl;
			exit(-1);
		}
	
	}
	else if (descriptor == "scanner")
	{
	
		//running process
		pData.processState = 2;
		rc = pthread_create(&inputThread, NULL, &scannerHandler, (void*)pTime);
		if (rc)
		{
			cout << "ERROR: return code from pthread_create() is " << rc << endl;
			exit(-1);
		}
	
	}
	else
	{
		cout << "ERROR: Incorrect descriptor recorded for Meta-Data code 'I'" << endl;
		return 0;
	}
	
	//process is waiting
	pData.processState = 3;
	
	pthread_join(inputThread, NULL);
	
	//ending process
	pData.processState = 1;
	duration = ((clock() - start) / (double) CLOCKS_PER_SEC) - duration;
	pData.updateProcessDuration(duration);
	switch (outputType)
	{
		case 0:
			cout << pData.getProcessDuration() << " - Process " << pid << 
			": end " << descriptor << " input" << endl;
		break;
		case 1:
			fout << pData.getProcessDuration() << " - Process " << pid << 
			": end " << descriptor << " input" << endl;
		break;
		case 2:
			cout << pData.getProcessDuration() << " - Process " << pid << 
			": end " << descriptor << " input" << endl;
			fout << pData.getProcessDuration() << " - Process " << pid << 
			": end " << descriptor << " input" << endl;
		break;
		default:
			cout << "ERROR: Incorrect log type recorded from config file" << endl;
			return 0;
	}
	
	return 1;

}

/**
*	Function: outputHandler
*	Description: Runs a process for an output device for the correct amount of time by 
*		using a timer thread. While the thread counts down, executes the process in 
*		another thread. Currently there is nothing else to be executed. Updates the 
*		duration of the process contained in the process' PCB. 
*/
bool outputHandler(PCB& pData, string descriptor, int processTime)
{

	pthread_t outputThread;
	clock_t start;
	long pTime = (long) processTime;
	double duration;
	int rc, pid = pData.getpid();
	
	start = clock();
	
	//preparing process
	duration = (clock() - start) / (double) CLOCKS_PER_SEC;
	pData.updateProcessDuration(duration);
	switch (outputType)
	{
		case 0:
			cout << pData.getProcessDuration() << " - Process " << pid <<
			": start " << descriptor << " output" << endl;
		break;
		case 1:
			fout << pData.getProcessDuration() << " - Process " << pid <<
			": start " << descriptor << " output" << endl;
		break;
		case 2:
			cout << pData.getProcessDuration() << " - Process " << pid <<
			": start " << descriptor << " output" << endl;
			fout << pData.getProcessDuration() << " - Process " << pid <<
			": start " << descriptor << " output" << endl;
		break;
		default:
			cout << "ERROR: Incorrect log type recorded from config file" << endl;
			return 0;
	}
	
	if (descriptor == "hard drive")
	{
	
		//running process
		pData.processState = 2;
		rc = pthread_create(&outputThread, NULL, &hardDriveOutputHandler, (void*)pTime);
		if (rc)
		{
			cout << "ERROR: return code from pthread_create() is " << rc << endl;
			exit(-1);
		}
	
	}
	else if (descriptor == "monitor")
	{
	
		//running process
		pData.processState = 2;
		rc = pthread_create(&outputThread, NULL, &monitorHandler, (void*)pTime);
		if (rc)
		{
			cout << "ERROR: return code from pthread_create() is " << rc << endl;
			exit(-1);
		}
	
	}
	else if (descriptor == "projector")
	{
	
		//running process
		pData.processState = 2;
		rc = pthread_create(&outputThread, NULL, &projectorHandler, (void*)pTime);
		if (rc)
		{
			cout << "ERROR: return code from pthread_create() is " << rc << endl;
			exit(-1);
		}
	
	}
	else
	{
		cout << "ERROR: Incorrect descriptor recorded for Meta-Data code 'I'" << endl;
		return 0;
	}
	
	//process is waiting
	pData.processState = 3;
	
	pthread_join(outputThread, NULL);
	
	//ending process
	pData.processState = 1;
	duration = ((clock() - start) / (double) CLOCKS_PER_SEC) - duration;
	pData.updateProcessDuration(duration);
	switch (outputType)
	{
		case 0:
			cout << pData.getProcessDuration() << " - Process " << pid << 
			": end " << descriptor << " output" << endl;
		break;
		case 1:
			fout << pData.getProcessDuration() << " - Process " << pid << 
			": end " << descriptor << " output" << endl;
		break;
		case 2:
			cout << pData.getProcessDuration() << " - Process " << pid << 
			": end " << descriptor << " output" << endl;
			fout << pData.getProcessDuration() << " - Process " << pid << 
			": end " << descriptor << " output" << endl;
		break;
		default:
			cout << "ERROR: Incorrect log type recorded from config file" << endl;
			return 0;
	}
	
	return 1;

}

/**
*	Function: generateMemoryAddress
*	Description: Generates a random unsigned int to be used as a memory address 
*		location 
*/
unsigned int generateMemoryAddress()
{

	//generating and assigning random integer value to address
	unsigned int address;
	
	srand (time(NULL));
	
	address = rand() % UINT_MAX;
	return address;

}

/**
*	Function: allocateMemory
*	Description: will be used to allocate the amount of system memory to address 
*		generated using the generateMemoryAddress() function.
*/
bool allocateMemory(unsigned int address, int amount)
{

	return 1;

}

/**
*	Function: timer
*	Description: A thread that starts a timer from the parameter milliseconds down to 
*		zero. Thread is exited once the timer is finished.
*/
void* timer(void* milliseconds)
{

	long m = (long) milliseconds;
	
	clock_t timerEnd = clock() + m * CLOCKS_PER_SEC/1000;
	while(clock() < timerEnd) {};

}

/**
*	Function: hardDriveInputHandler
*	Description: A thread that handles an input process from the hard drive. Currently 
*	there are no processes to be performed, so this function does nothing but wait for 
*	the timer thread to finish
*/
void* hardDriveInputHandler(void* hdTime)
{

	pthread_t timerThread;
	int rc;

	rc = pthread_create(&timerThread, NULL, &timer, (void*)hdTime);
	if (rc)
	{
		cout << "ERROR: return code from pthread_create() is " << rc << endl;
		exit(-1);
	}
	
	pthread_join(timerThread, NULL);

}

/**
*	Function: keyboardHandler
*	Description: A thread that handles an input process from the keyboard. Currently 
*	there are no processes to be performed, so this function does nothing but wait for 
*	the timer thread to finish
*/
void* keyboardHandler(void* kTime)
{

	pthread_t timerThread;
	int rc;

	rc = pthread_create(&timerThread, NULL, &timer, (void*)kTime);
	if (rc)
	{
		cout << "ERROR: return code from pthread_create() is " << rc << endl;
		exit(-1);
	}
	
	pthread_join(timerThread, NULL);

}

/**
*	Function: scannerHandler
*	Description: A thread that handles an input process from the scanner. Currently 
*	there are no processes to be performed, so this function does nothing but wait for 
*	the timer thread to finish
*/
void* scannerHandler(void* sTime)
{

	pthread_t timerThread;
	int rc;

	rc = pthread_create(&timerThread, NULL, &timer, (void*)sTime);
	if (rc)
	{
		cout << "ERROR: return code from pthread_create() is " << rc << endl;
		exit(-1);
	}
	
	pthread_join(timerThread, NULL);

}

/**
*	Function: hardDriveOutputHandler
*	Description: A thread that handles an output process from the hard drive. Currently 
*	there are no processes to be performed, so this function does nothing but wait for 
*	the timer thread to finish
*/
void* hardDriveOutputHandler(void* hdTime)
{

	pthread_t timerThread;
	int rc;

	rc = pthread_create(&timerThread, NULL, &timer, (void*)hdTime);
	if (rc)
	{
		cout << "ERROR: return code from pthread_create() is " << rc << endl;
		exit(-1);
	}
	
	pthread_join(timerThread, NULL);

}

/**
*	Function: monitorHandler
*	Description: A thread that handles an output process from the monitor. Currently 
*	there are no processes to be performed, so this function does nothing but wait for 
*	the timer thread to finish
*/
void* monitorHandler(void* mTime)
{

	pthread_t timerThread;
	int rc;

	rc = pthread_create(&timerThread, NULL, &timer, (void*)mTime);
	if (rc)
	{
		cout << "ERROR: return code from pthread_create() is " << rc << endl;
		exit(-1);
	}
	
	pthread_join(timerThread, NULL);

}

/**
*	Function: projectorHandler
*	Description: A thread that handles an output process from the projector. Currently 
*	there are no processes to be performed, so this function does nothing but wait for 
*	the timer thread to finish
*/
void* projectorHandler(void* pTime)
{

	pthread_t timerThread;
	int rc;

	rc = pthread_create(&timerThread, NULL, &timer, (void*)pTime);
	if (rc)
	{
		cout << "ERROR: return code from pthread_create() is " << rc << endl;
		exit(-1);
	}
	
	pthread_join(timerThread, NULL);

}

/**
*	Function: output
*	Description: outputs to the monitor if the log type is 0 ("Monitor"), outputs to the 
*		file if the log type is 1 ("File"), or outputs to the file and the monitor if 
*		the log type is 2 ("Both").
*/
void output(Config configData, deque<MetaData> metaData)
{

	switch (configData.getLogType() + 1)
	{
		case 1:
		
			outputToMonitor(configData, metaData);
		
		break;
		
		case 2:
		
			outputToFile(configData, metaData);
		
		break;
		
		case 3:
		
			outputToFile(configData, metaData);
			outputToMonitor(configData, metaData);
		
		break;
		
		default:
		
			cout << "ERROR: error ocurred during output type selection" << endl;
			return;
	}

}

/**
*	Function - outputToMonitor
*	Description - Following the template in the guidelines, outputs configuration data 
*		and the set of meta-data instructions to the monitor. Accounts for the 
*		possibility that the data could also be logged to the log file.
*/
void outputToMonitor(Config configData, deque<MetaData> metaData)
{
	//Config Output
	cout << endl;
	cout << "Configuration File Data" << endl;
	cout << "Monitor = " << configData.getMonitorTime() << " ms/cycle" << endl;
	cout << "Processor = " << configData.getProcessorTime() << " ms/cycle" << endl;
	cout << "Scanner = " << configData.getScannerTime() << " ms/cycle" << endl;
	cout << "Hard Drive = " << configData.getHardDriveTime() << " ms/cycle" << endl;
	cout << "Keyboard = " << configData.getKeyboardTime() << " ms/cycle" << endl;
	cout << "Memory = " << configData.getMemoryTime() << " ms/cycle" << endl;
	cout << "Projector = " << configData.getProjectorTime() << " ms/cycle" << endl;
	cout << "System Memory = " << configData.getSystemMemory() << " kbytes" << endl;
	cout << "Logged to: ";
	if (configData.getLogType() == 0)
	{
		cout << "monitor" << endl;
	}
	else if (configData.getLogType() == 2)
	{
		cout << "monitor and " << configData.getLogPath() << endl;
	}
	else
	{
		cout << "ERROR: using incorrect output handler" << endl;
		return;
	}
	cout << endl;
	
	//Meta-Data Output
	cout << "Meta-Data Metrics" << endl;
	for (int i = 0; i < metaData.size(); i++)
	{
		if (metaData[i].getCode() != 'S' && metaData[i].getCode() != 'A')
		{
			cout << metaData[i].getCode();
			cout << '{';
			cout << metaData[i].getDescriptor();
			cout << '}';
			cout << metaData[i].getNumOfCycles();
			cout << " - ";
			cout << metaData[i].getTotalTime();
			cout << " ms" << endl;
		}
	}
	cout << endl;

}

/**
*	Function - outputToFile
*	Description - Following the template in the guidelines, outputs configuration data 
*		and the set of meta-data instructions to the file specified in the configuration 
*		file. Accounts for the possibility that the data could also be logged to the 
*		monitor.
*/
void outputToFile(Config configData, deque<MetaData> metaData)
{
	//ofstream fout;
	fout.open(configData.getLogPath());

	//Config Output
	fout << "Configuration File Data" << endl;
	fout << "Monitor = " << configData.getMonitorTime() << " ms/cycle" << endl;
	fout << "Processor = " << configData.getProcessorTime() << " ms/cycle" << endl;
	fout << "Scanner = " << configData.getScannerTime() << " ms/cycle" << endl;
	fout << "Hard Drive = " << configData.getHardDriveTime() << " ms/cycle" << endl;
	fout << "Keyboard = " << configData.getKeyboardTime() << " ms/cycle" << endl;
	fout << "Memory = " << configData.getMemoryTime() << " ms/cycle" << endl;
	fout << "Projector = " << configData.getProjectorTime() << " ms/cycle" << endl;
	fout << "System Memory = " << configData.getSystemMemory() << " kbytes" << endl;
	fout << "Logged to: ";
	if (configData.getLogType() == 1)
	{
		fout << configData.getLogPath() << endl;
	}
	else if (configData.getLogType() == 2)
	{
		fout << "monitor and " << configData.getLogPath() << endl;
	}
	else
	{
		fout << "ERROR: using incorrect output handler" << endl;
		return;
	}
	fout << endl;
	
	//Meta-Data Output
	fout << "Meta-Data Metrics" << endl;
	for (int i = 0; i < metaData.size(); i++)
	{
		if (metaData[i].getCode() != 'S' && metaData[i].getCode() != 'A')
		{
			fout << metaData[i].getCode();
			fout << '{';
			fout << metaData[i].getDescriptor();
			fout << '}';
			fout << metaData[i].getNumOfCycles();
			fout << " - ";
			fout << metaData[i].getTotalTime();
			fout << " ms" << endl;
		}
	}
	fout << endl;
	
	//fout.close();
}

