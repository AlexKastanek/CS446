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
#include <iomanip>
#include <fstream>
#include <vector>
#include <deque>
#include <string>
#include <pthread.h>
#include <semaphore.h>
#include <limits.h>
#include <time.h>
#include "Config.h"
#include "MetaData.h"
#include "PCB.h"

using namespace std;

//Global variables

ofstream fout;
int outputType, scheduleType;

deque<deque<MetaData>> program, waitingQueue, readyQueue; //used to store each process
deque<int> waitingProcessIndeces, loadedProcessIndeces;
vector<PCB> pcbContainer; //used to store the pcb for each process

int blockCount, lastAddress;
int activeProcesses = 0;

pthread_t inputThread;
pthread_mutex_t mutexKeyboard;
pthread_mutex_t mutexScanner;
pthread_mutex_t mutexMonitor;
sem_t semaphoreHdd;
sem_t semaphoreProj;

//function headers

bool getMetaData(ifstream&, Config, deque<MetaData>&);

bool prepProgram(Config, deque<MetaData>);
bool loadProgram();
bool runProgram(Config);
bool handleProcess(Config, MetaData, PCB&);
bool systemHandler(PCB&, string);
bool applicationHandler(PCB&, string);
bool processorHandler(PCB&, int);
bool memoryHandler(PCB&, Config, string, int);
bool inputHandler(PCB&, string, int);
bool outputHandler(PCB&, string, int);

void prioritySchedule(deque<deque<MetaData>>);
void shortestJobFirstSchedule(deque<deque<MetaData>>);
void shortestTimeRemainingSchedule(deque<deque<MetaData>> processStorage, 
								   deque<int> indexStorage);

unsigned int generateMemoryAddress();
int allocateMemory(PCB&, Config);

void* timer(void*);
void* loader(void*);
void* processThread(void*);
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
	//Config ADT used to store data from the config file
	Config configData[argc - 1]; 
	//double-ended queue of the ADT MetaData - used to store data from an instruction
	//in the Meta Data file
	deque<MetaData> instructionSet[argc - 1]; 

	//begin command line argument error checking and config file input
	ifstream fin;
	for (int i = 0; i < argc; i++)
	{
		fin.open(argv[i]);
		if (!fin.is_open())
		{
			//file specified in command line argument does not exist
			cout << "ERROR: Incorrect file name in command line: command number " << i 
				 << endl;
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
		
		scheduleType = configData[i].getCpuScheduleCode();
	
		//assigning metaDataFile to file name specified in config data
		metaDataFile = configData[i].getFilePath();
		
		//resetting memory
		blockCount = 0;
		lastAddress = 0;

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
			okToContinue = prepProgram(configData[i], instructionSet[i]);
		}
		else
		{
			return -1;
		}
		
		//if ok to continue, call prepProgram(configData[i], instructionSet[i])
		/*if (okToContinue)
		{
			okToContinue = prepProgram(configData[i], instructionSet[i], 
									   processQueue);
		}
		else
		{
			return -1;
		}*/
		
		if (okToContinue)
		{
			outputType = configData[i].getLogType();
			okToContinue = runProgram(configData[i]);
		}
		else
		{
			cout << "ERROR: Program " << i << " could not be instantiated" << endl;
			return -1;
		}
		
		fout.close();
		
		//deallocting memory
		blockCount = 0;
		lastAddress = 0;
		
		pthread_mutex_destroy(&mutexKeyboard);
		pthread_mutex_destroy(&mutexScanner);
		pthread_mutex_destroy(&mutexMonitor);
		sem_destroy(&semaphoreHdd);
		sem_destroy(&semaphoreProj);
	
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

bool prepProgram(Config configData, deque<MetaData> instructionSet)
{

	vector<int> processIndeces;
	int processCount = 0;
	
	//determine the indeces of the beginning of each process 
	//and the amount of processes
	for (int i = 0; i < instructionSet.size(); i++)
	{
		if (instructionSet[i].getCode() == 'A' && 
			instructionSet[i].getDescriptor() == "begin")
		{
			processIndeces.push_back(i);
			processCount++;
		}
	}
	
	//use indeces to separate processes into individual deques
	deque<MetaData> processes[processCount];
	for (int i = 0; i < processCount; i++)
	{
		if (i == processCount - 1)
		{
			for (int j = processIndeces[i]; j < instructionSet.size(); j++)
			{
				if (instructionSet[j].getCode() != 'S' && 
					instructionSet[j].getCode() != 'A')
				{
					processes[i].push_back(instructionSet[j]);
				}
			}
		}
		else
		{
			for (int j = processIndeces[i]; j < processIndeces[i + 1] + 1; j++)
			{
				if (instructionSet[j].getCode() != 'S' && 
					instructionSet[j].getCode() != 'A')
				{
					processes[i].push_back(instructionSet[j]);
				}
			}
		}
		
		program.push_back(processes[i]);
	}
	
	//to ensure consistency, adding meta data start and finish commands for 
	//application to the queues
	MetaData systemBegin, systemFinish, appBegin, appFinish;
	systemBegin.setData('S', "begin", 0, 0);
	systemFinish.setData('S', "finish", 0, 0);
	appBegin.setData('A', "begin", 0, 0);
	appFinish.setData('A', "finish", 0, 0);
	for (int i = 0; i < program.size(); i++)
	{
		program[i].push_front(appBegin);
		program[i].push_back(appFinish);
	}
	
	//construct pcb data for each process
	PCB pcb;
	for (int i = 0; i < program.size(); i++)
	{
		pcbContainer.push_back(pcb);
		pcbContainer[i].setpid(i + 1);
		pcbContainer[i].setHardDriveQuant(configData.getHddQuant());
		pcbContainer[i].setProjectorQuant(configData.getProjQuant());
		pcbContainer[i].setStartTime(0);
		pcbContainer[i].setProcessDuration(0);
		int estProcessTime = 0;
		for (int j = 0; j < program[i].size(); j++)
		{
			estProcessTime += program[i][j].getTotalTime();
		}
		pcbContainer[i].setEstimatedProcessTime(estProcessTime);
		pcbContainer[i].setEstimatedTimeRemaining(estProcessTime);
	}
	
	if (configData.getCpuScheduleCode() == 1)
	{
		//PS
		deque<deque<MetaData>> processStorage = program;
		prioritySchedule(processStorage);
	}
	else if (configData.getCpuScheduleCode() == 2)
	{
		//SJF
		deque<deque<MetaData>> processStorage = program;
		shortestJobFirstSchedule(processStorage);
	}
	else
	{
		//FIFO or preemptive scheduling algorithm
		for (int i = 0; i < program.size(); i++)
		{
			waitingProcessIndeces.push_back(i);
		}
	}
	
	program[0].push_front(systemBegin);
	//program[program.size() - 1].push_back(systemFinish);
	
	waitingQueue = program;
	
	return true;
	

}

/**
*	Function: prepProgram
*	Description: Prepares a set of Meta-Data instructions called a program by 
*		separating the program into processes and then scheduling those processes
*/
/*bool prepProgram(Config configData, deque<MetaData> instructionSet,
				 deque<deque<MetaData>>& program)
{

	vector<deque<MetaData>> processStorage;
	vector<int> processIndeces;
	int processCount = 0;
	
	//determine the indeces of the beginning of each process 
	//and the amount of processes
	for (int i = 0; i < instructionSet.size(); i++)
	{
		if (instructionSet[i].getCode() == 'A' && 
			instructionSet[i].getDescriptor() == "begin")
		{
			processIndeces.push_back(i);
			processCount++;
		}
	}
	
	//use indeces to separate processes into individual deques
	//store deques in a vector
	deque<MetaData> processes[processCount];
	for (int i = 0; i < processCount; i++)
	{
		if (i == processCount - 1)
		{
			for (int j = processIndeces[i]; j < instructionSet.size(); j++)
			{
				if (instructionSet[j].getCode() != 'S' && 
					instructionSet[j].getCode() != 'A')
				{
					processes[i].push_back(instructionSet[j]);
				}
			}
		}
		else
		{
			for (int j = processIndeces[i]; j < processIndeces[i + 1] + 1; j++)
			{
				if (instructionSet[j].getCode() != 'S' && 
					instructionSet[j].getCode() != 'A')
				{
					processes[i].push_back(instructionSet[j]);
				}
			}
		}
		
		processStorage.push_back(processes[i]);
	}
	
	//to ensure consistency, adding meta data start and finish commands for 
	//application to the queues
	MetaData systemBegin, systemFinish, appBegin, appFinish;
	systemBegin.setData('S', "begin", 0, 0);
	systemFinish.setData('S', "finish", 0, 0);
	appBegin.setData('A', "begin", 0, 0);
	appFinish.setData('A', "finish", 0, 0);
	for (int i = 0; i < processStorage.size(); i++)
	{
		processStorage[i].push_front(appBegin);
		processStorage[i].push_back(appFinish);
	}
	
	//call scheduling algorithm to resort vector of processes into a deque
	if (configData.getCpuScheduleCode() == 0)
	{
		//FIFO - no need to resort anything
		for (int i = 0; i < processStorage.size(); i++)
		{
			program.push_back(processStorage[i]);
		}
	}
	else if (configData.getCpuScheduleCode() == 1)
	{
		//PS
		prioritySchedule(processStorage, program);
	}
	else if (configData.getCpuScheduleCode() == 2)
	{
		//SJF
		shortestJobFirstSchedule(processStorage, program);
	}
	else
	{
		cout << "ERROR: incorrect cpu scheduling code recorded" << endl;
		return 0;
	}
	
	//adding meta data start and finish commands for system to the queues
	program[0].push_front(systemBegin);
	program[program.size() - 1].push_back(systemFinish);
	
	for (int i = 0; i < program.size(); i++)
	{
		for (int j = 0; j < program[i].size(); j++)
		{
			program[i][j].print();
		}
		cout << endl;
	}
	cout << endl;

}*/

bool loadProgram()
{

	if (scheduleType == 3)
	{
	
		if (readyQueue.empty())
		{
			readyQueue.push_back(waitingQueue[0]);
			loadedProcessIndeces.push_back(waitingProcessIndeces[0]);
			waitingQueue.pop_front();
			waitingProcessIndeces.pop_front();
			cout << "arrival of process " << loadedProcessIndeces.back() + 1 << endl;
		}
		else
		{
			readyQueue.push_back(waitingQueue[0]);
			loadedProcessIndeces.push_back(waitingProcessIndeces[0]);
			waitingQueue.pop_front();
			waitingProcessIndeces.pop_front();
			cout << "arrival of process " << loadedProcessIndeces.back() + 1 << endl;
			shortestTimeRemainingSchedule(readyQueue, loadedProcessIndeces);
		}
	
	}
	else
	{
	
		readyQueue.push_back(waitingQueue[0]);
		loadedProcessIndeces.push_back(waitingProcessIndeces[0]);
		waitingQueue.pop_front();
		waitingProcessIndeces.pop_front();
		cout << "arrival of process " << loadedProcessIndeces.back() + 1 << endl;
	
		return true;
	
	}

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
bool runProgram(Config configData)
{

	//PCB processData[program.size()];
	pthread_t loadThread;
	clock_t start;
	double duration;
	int rc;
	bool okToContinue = 1;
	
	//setting parameters of program's process control blocks
	/*for (int i = 0; i < program.size(); i++)
	{
		processData[i].setpid(i + 1);
		processData[i].setHardDriveQuant(configData.getHddQuant());
		processData[i].setProjectorQuant(configData.getProjQuant());
		processData[i].setStartTime(0);
		processData[i].setProcessDuration(0);
	}*/
	
	
	//initializing mutex
	pthread_mutex_init(&mutexKeyboard, NULL);
	pthread_mutex_init(&mutexScanner, NULL);
	pthread_mutex_init(&mutexMonitor, NULL);
	sem_init(&semaphoreHdd, 0, configData.getHddQuant());
	sem_init(&semaphoreProj, 0, configData.getProjQuant());
	
	//starting program clock and initializing duration
	start = clock();
	duration = (clock() - start) / (double) CLOCKS_PER_SEC;
	
	//loops through each meta data instruction and calls handleProcess() to handle 
	//each task
	/*for (int i = 0; i < program.size(); i++)
	{
		for (int j = 0; j < program[i].size(); j++)
		{
			if (okToContinue)
			{
				duration = ((clock() - start) / (double) CLOCKS_PER_SEC);
				pcbContainer[i].setProcessDuration(duration);
				okToContinue = handleProcess(configData, program[i][j], 
											 pcbContainer[i]);
			}
			else
			{
				return 0;
			}
			duration = ((clock() - start) / (double) CLOCKS_PER_SEC);
		}
	}*/
	
	rc = pthread_create(&loadThread, NULL, &loader, NULL);
	if (rc)
	{
		cout << "ERROR: return code from pthread_create() is " << rc << endl;
		exit(-1);
	}
	
	while (!waitingQueue.empty())
	{
		while (!readyQueue.empty())
		{
			okToContinue = 1;
			int processIndex = loadedProcessIndeces[0];
			while (!readyQueue[0].empty() && okToContinue)
			{
				if (okToContinue)
				{
					duration = ((clock() - start) / (double) CLOCKS_PER_SEC);
					pcbContainer[processIndex].setProcessDuration(duration);
					okToContinue = handleProcess(configData, readyQueue[0][0], 
											 	 pcbContainer[processIndex]);
					if (okToContinue)
					{
						readyQueue[0].pop_front();
					}
				}
				else
				{
					if (pcbContainer[processIndex].isInterrupted())
					{
						//cout << "check 1" << endl;
						okToContinue = 0;
					}
					else
					{
						return 0;
					}
				}
				duration = ((clock() - start) / (double) CLOCKS_PER_SEC);
			}
			if (!pcbContainer[processIndex].isInterrupted() && okToContinue)
			{
				//cout << "check" << endl;
				readyQueue.pop_front();
				loadedProcessIndeces.pop_front();
			}

		}
	}
	pthread_join(inputThread, NULL);
	
	int lastProcessIndex = loadedProcessIndeces[loadedProcessIndeces.size() - 1];
		
	duration = ((clock() - start) / (double) CLOCKS_PER_SEC);
	pcbContainer[lastProcessIndex].setProcessDuration(duration);
	
	MetaData systemFinish;
	systemFinish.setData('S', "finish", 0, 0);
	handleProcess(configData, systemFinish, pcbContainer[lastProcessIndex]);
	
	pthread_join(loadThread, NULL);
	
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
		
			return memoryHandler(pData, cData, process.getDescriptor(), 
								 process.getTotalTime());
		
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
*	Description: Prepares and starts the system as well as ends it. Updates the 
*		duration of the process contained in the process' PCB. 
*/
bool systemHandler(PCB& pData, string descriptor)
{

	clock_t start;
	int pid = pData.getpid();
	double duration;
	
	start = clock();

	if (descriptor == "begin")
	{
		//using switch statement to output to file, monitor, or both
		switch (outputType)
		{
			case 0:
				cout << fixed << pData.getProcessDuration() 
					 << " - Simulator program starting" << endl;
			break;
			case 1:
				fout << fixed << pData.getProcessDuration() 
					 << " - Simulator program starting" << endl;
			break;
			case 2:
				cout << fixed << pData.getProcessDuration() 
					 << " - Simulator program starting" << endl;
				fout << fixed << pData.getProcessDuration() 
					 << " - Simulator program starting" << endl;
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
				cout << fixed << pData.getProcessDuration() 
					 << " - Simulator program ending" << endl;
			break;
			case 1:
				fout << fixed << pData.getProcessDuration() 
					 << " - Simulator program ending" << endl;
			break;
			case 2:
				cout << fixed << pData.getProcessDuration() 
					 << " - Simulator program ending" << endl;
				fout << fixed << pData.getProcessDuration() 
					 << " - Simulator program ending" << endl;
			break;
			default:
				cout << "ERROR: Incorrect log type recorded from config file" << endl;
				return 0;
		}
		
		blockCount = 0;
		lastAddress = 0;	
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
*	Description: Prepares and starts a process as well as ends it. Updates the duration 
*		of the process contained in the process' PCB. 
*/
bool applicationHandler(PCB& pData, string descriptor)
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
		pData.setStartTime(pData.getProcessDuration());
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
				cout << pData.getProcessDuration() << " - End process " << pid 
				<< endl;
			break;
			case 1:
				fout << pData.getProcessDuration() << " - End process " << pid 
				<< endl;
			break;
			case 2:
				cout << pData.getProcessDuration() << " - End process " << pid 
				<< endl;
				fout << pData.getProcessDuration() << " - End process " << pid 
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
		cout << "ERROR: Incorrect descriptor recorded for Meta-Data code 'A'" << endl;
		return 0;
	}
	
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

	pthread_t processorThread;
	clock_t start;
	long pTime = (long) processTime;
	double duration;
	int rc, pid = pData.getpid();
	
	start = clock();
	
	if (pData.hasBeenInterrupted())
	{
	
		duration = (clock() - start) / (double) CLOCKS_PER_SEC;
		pData.updateProcessDuration(duration);
		pData.setInterrupt(0);
		pTime = pTime - pData.loadState();
		pData.processState = 2;
		rc = pthread_create(&processorThread, NULL, &processThread, (void*)pTime);
		if (rc)
		{
			cout << "ERROR: return code from pthread_create() is " << rc << endl;
			exit(-1);
		}
		else
		{
			activeProcesses++;
		}
		
		while (true)
		{
	
			if (activeProcesses == 0)
			{
				break;
			}
			else if (pData.isInterrupted())
			{
				pData.processState = 1;
				duration = ((clock() - start) / (double) CLOCKS_PER_SEC) - duration;
				pData.updateProcessDuration(duration);
				pData.saveState(duration);
				pData.setEstimatedTimeRemaining(pData.getEstimatedProcessTime() - 
							  	(pData.getProcessDuration() - pData.getStartTime()));
				switch (outputType)
				{
					case 0:
						cout << pData.getProcessDuration() << " - Process " << pid << 
						": interrupt processing action" << endl;
					break;
					case 1:
						fout << pData.getProcessDuration() << " - Process " << pid << 
					": interrupt processing action" << endl;
					break;
					case 2:
						cout << pData.getProcessDuration() << " - Process " << pid << 
						": interrupt processing action" << endl;
						fout << pData.getProcessDuration() << " - Process " << pid << 
						": interrupt processing action" << endl;
					break;
					default:
						cout << "ERROR: Incorrect log type recorded from config file" 
							 << endl;
						return 0;
				}
				return 0;
			}
			
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
					cout << "ERROR: Incorrect log type recorded from config file"
						 << endl;
					return 0;
			}
	
			return 1;
	
		}
	
	}
	
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
	rc = pthread_create(&processorThread, NULL, &processThread, (void*)pTime);
	if (rc)
	{
		cout << "ERROR: return code from pthread_create() is " << rc << endl;
		exit(-1);
	}
	else
	{
		activeProcesses++;
	}
	
	//more functionality would go here
	while (true)
	{
	
		if (activeProcesses == 0)
		{
			break;
		}
		else if (pData.isInterrupted())
		{
			pData.processState = 1;
			duration = ((clock() - start) / (double) CLOCKS_PER_SEC) - duration;
			pData.updateProcessDuration(duration);
			pData.saveState(duration);
			pData.setEstimatedTimeRemaining(pData.getEstimatedProcessTime() - 
							  	(pData.getProcessDuration() - pData.getStartTime()));
			switch (outputType)
			{
				case 0:
					cout << pData.getProcessDuration() << " - Process " << pid << 
					": interrupt processing action" << endl;
				break;
				case 1:
					fout << pData.getProcessDuration() << " - Process " << pid << 
					": interrupt processing action" << endl;
				break;
				case 2:
					cout << pData.getProcessDuration() << " - Process " << pid << 
					": interrupt processing action" << endl;
					fout << pData.getProcessDuration() << " - Process " << pid << 
					": interrupt processing action" << endl;
				break;
				default:
					cout << "ERROR: Incorrect log type recorded from config file" 
						 << endl;
					return 0;
			}
			return 0;
		}
	
	}
	
	//pthread_join(timerThread, NULL);
	
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
bool memoryHandler(PCB& pData, Config cData, string descriptor, int processTime)
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
		
		//addr = generateMemoryAddress();
		addr = allocateMemory(pData, cData);
		if (addr == -1)
		{
			cout << "ERROR: memory allocation failed" << endl;
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
				cout << pData.getProcessDuration() << " - Process " << pid 
					 << ": memory allocated at 0x" << hex << setw(8) << setfill('0')
					 << addr << endl << dec;
			break;
			case 1:
				fout << pData.getProcessDuration() << " - Process " << pid 
					 << ": memory allocated at 0x" << hex << setw(8) << setfill('0')
					 << addr << endl << dec;
			break;
			case 2:
				cout << pData.getProcessDuration() << " - Process " << pid 
					 << ": memory allocated at 0x" << hex << setw(8) << setfill('0')
					 << addr << endl << dec;
				fout << pData.getProcessDuration() << " - Process " << pid 
					 << ": memory allocated at 0x" << hex << setw(8) << setfill('0')
					 << addr << endl << dec;
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

	clock_t start;
	long pTime = (long) processTime;
	double duration;
	int rc, pid = pData.getpid();
	
	start = clock();
	
	//preparing process
	duration = (clock() - start) / (double) CLOCKS_PER_SEC;
	pData.updateProcessDuration(duration);
	if (descriptor != "hard drive")
	{
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
				cout << "ERROR: Incorrect log type recorded from config file" 
					 << endl;
				return 0;
		}
	}
	else
	{
		int hddIndex = pData.getHardDrivesUsed() % pData.getHardDriveQuant();
		switch (outputType)
		{
		
			case 0:
				cout << pData.getProcessDuration() << " - Process " << pid <<
				": start " << descriptor << " input on HDD " << hddIndex << endl;
			break;
			case 1:
				fout << pData.getProcessDuration() << " - Process " << pid <<
				": start " << descriptor << " input on HDD " << hddIndex << endl;
			break;
			case 2:
				cout << pData.getProcessDuration() << " - Process " << pid <<
				": start " << descriptor << " input on HDD " << hddIndex << endl;
				fout << pData.getProcessDuration() << " - Process " << pid <<
				": start " << descriptor << " input on HDD " << hddIndex << endl;
			break;
			default:
				cout << "ERROR: Incorrect log type recorded from config file" 
					 << endl;
				return 0;
		}
		pData.incrementHardDrivesUsed();
	}
	
	if (descriptor == "hard drive")
	{
	
		//running process
		pData.processState = 2;
		//int hddIndex = pData.getHardDrivesUsed() % pData.getHardDriveQuant();
		//pData.incrementHardDrivesUsed();
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
	
	/*pthread_join(inputThread, NULL);
	
	//ending process
	pData.processState = 1;
	duration = ((clock() - start) / (double) CLOCKS_PER_SEC) - duration;
	pData.updateProcessDuration(duration);*/
	
	
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
	if (descriptor != "hard drive" && descriptor != "projector")
	{
	
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
		
	}
	else if (descriptor == "hard drive")
	{
	
		int hddIndex = pData.getHardDrivesUsed() % pData.getHardDriveQuant();
		switch (outputType)
		{
		
			case 0:
				cout << pData.getProcessDuration() << " - Process " << pid <<
				": start " << descriptor << " output on HDD " << hddIndex << endl;
			break;
			case 1:
				fout << pData.getProcessDuration() << " - Process " << pid <<
				": start " << descriptor << " output on HDD " << hddIndex << endl;
			break;
			case 2:
				cout << pData.getProcessDuration() << " - Process " << pid <<
				": start " << descriptor << " output on HDD " << hddIndex << endl;
				fout << pData.getProcessDuration() << " - Process " << pid <<
				": start " << descriptor << " output on HDD " << hddIndex << endl;
			break;
			default:
				cout << "ERROR: Incorrect log type recorded from config file" 
					 << endl;
				return 0;
		}
		pData.incrementHardDrivesUsed();
	
	}
	else if (descriptor == "projector")
	{
	
		int projIndex = pData.getProjectorsUsed() % pData.getProjectorQuant();
		switch (outputType)
		{
		
			case 0:
				cout << pData.getProcessDuration() << " - Process " << pid <<
				": start " << descriptor << " output on PROJ " << projIndex << endl;
			break;
			case 1:
				fout << pData.getProcessDuration() << " - Process " << pid <<
				": start " << descriptor << " output on PROJ " << projIndex << endl;
			break;
			case 2:
				cout << pData.getProcessDuration() << " - Process " << pid <<
				": start " << descriptor << " output on PROJ " << projIndex << endl;
				fout << pData.getProcessDuration() << " - Process " << pid <<
				": start " << descriptor << " output on PROJ " << projIndex << endl;
			break;
			default:
				cout << "ERROR: Incorrect log type recorded from config file" 
					 << endl;
				return 0;
		}
		pData.incrementProjectorsUsed();
	
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
*	Function: prioritySchedule
*	Description: Schedules processes based on number of input/output operations. 
*		Places processes into deque called program
*/
void prioritySchedule(deque<deque<MetaData>> processStorage)
{

	int processIOCounters[processStorage.size()];
	
	program.clear();

	//count io operations in each process and store in an array
	for (int i = 0; i < processStorage.size(); i++)
	{
		processIOCounters[i] = 0;
		for (int j = 0; j < processStorage[i].size(); j++)
		{
			if (processStorage[i][j].getCode() == 'I' || 
				processStorage[i][j].getCode() == 'O')
			{
				processIOCounters[i]++;
			}
		}
	}
	
	//sort processes by processIOCounters
	int largestIOValue, largestIOIndex = 0;
	for (int i = 0; i < processStorage.size(); i++)
	{
		largestIOValue = -1;
		for (int j = 0; j < processStorage.size(); j++)
		{
			if (processIOCounters[j] > largestIOValue)
			{
				largestIOValue = processIOCounters[j];
				largestIOIndex = j;
			}
		}
		program.push_back(processStorage[largestIOIndex]);
		waitingProcessIndeces.push_back(largestIOIndex);
		processIOCounters[largestIOIndex] = -1;
	}

}

/**
*	Function: shortestJobFirst
*	Description: Schedules process based on number of tasks in each process. Sorts 
*		them from least number of tasks to most number of tasks. Places processes 
*		into deque called program. 
*/
void shortestJobFirstSchedule(deque<deque<MetaData>> processStorage)
{

	program.clear();

	//sort processes by amount of tasks
	int largestNumOfTasks = 0, lastLargestNumOfTasks = 0;
	int largestIndex = 0, lastLargestIndex = 0;;
	for (int i = 0; i < processStorage.size(); i++)
	{
		largestNumOfTasks = 0;
		for (int j = 0; j < processStorage.size(); j++)
		{
			if (i == 0)
			{
				if (processStorage[j].size() > largestNumOfTasks)
				{
					largestNumOfTasks = processStorage[j].size();
					largestIndex = j;
				}
			}
			else
			{
				if (processStorage[j].size() > largestNumOfTasks &&
					processStorage[j].size() <= lastLargestNumOfTasks &&
					j != lastLargestIndex)
				{
					largestNumOfTasks = processStorage[j].size();
					largestIndex = j;
				}
			}
		}
		
		program.push_front(processStorage[largestIndex]);
		waitingProcessIndeces.push_front(largestIndex);
		lastLargestNumOfTasks = largestNumOfTasks;
		lastLargestIndex = largestIndex;
	}

}

void shortestTimeRemainingSchedule(deque<deque<MetaData>> processStorage, 
								   deque<int> indexStorage)
{

	deque<deque<MetaData>> tempReadyQueue;
	deque<int> tempLoadedProcessIndeces;
	
	//sort processes by shortest time remaining
	double mostTimeRemaining, lastMostTimeRemaining;
	int mostTimeIndex, largestIndex, lastLargestIndex;
	for (int i = 0; i < processStorage.size(); i++)
	{	
		mostTimeRemaining = 0;
		for (int j = 0; j < processStorage.size(); j++)
		{
			double etr = pcbContainer[indexStorage[j]].getEstimatedTimeRemaining();
			if (i == 0)
			{
				if (etr > mostTimeRemaining)
				{
					mostTimeRemaining = etr;
					mostTimeIndex = indexStorage[j];
					largestIndex = j;
				}
			}
			else
			{
				if (etr > mostTimeRemaining && etr <= lastMostTimeRemaining && 
					j != lastLargestIndex)
				{
					mostTimeRemaining = etr;
					mostTimeIndex = indexStorage[j];
					largestIndex = j;
				}
			}
		}
		
		tempReadyQueue.push_front(processStorage[largestIndex]);
		tempLoadedProcessIndeces.push_front(mostTimeIndex);
		lastMostTimeRemaining = mostTimeRemaining;
		lastLargestIndex = largestIndex;
	}
	
	if (tempLoadedProcessIndeces[0] != loadedProcessIndeces[0])
	{
		//cout << "interrupt received" << endl;
		pcbContainer[loadedProcessIndeces[0]].interrupt();
	}
	readyQueue = tempReadyQueue;
	loadedProcessIndeces = tempLoadedProcessIndeces;
	
	/*for (int i = 0; i < loadedProcessIndeces.size(); i++)
	{
	
		cout << loadedProcessIndeces[i] + 1 << " ";
	
	}
	cout << endl;*/

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
*	Description: allocates amount of memory specified in cData to the location 
*		specified in pData which stores the last memory address used. If max memory is 
*		reached or if no memory is allocated, the next address will be at location 0. 
*		After the memory is allocated, updates pData. Returns -1 if the address is 
*		below 0 or if the address is above the max system memory to signify an error.
*/
int allocateMemory(PCB& pData, Config cData)
{

	int addressIndex, nextAddress;
	if (blockCount == 0)
	{
		addressIndex = 0;
	}
	else
	{
		addressIndex = lastAddress;
	}
	nextAddress = (addressIndex + cData.getBlockSize()) % cData.getSystemMemory();
	lastAddress = nextAddress;
	blockCount++;;
	
	if (addressIndex < 0 || addressIndex > cData.getSystemMemory())
	{
		return -1;
	}
	else return addressIndex;

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

void* loader(void*)
{
	pthread_t timerThread;
	int rc;
	bool ok;
	
	for (int i = 0; i < program.size(); i++)
	{
		if (i > 0)
		{
			rc = pthread_create(&timerThread, NULL, &timer, (void*)100);
			if (rc)
			{
				cout << "ERROR: return code from pthread_create() is " << rc << endl;
				exit(-1);
			}
			pthread_join(timerThread, NULL);
		}
		ok = loadProgram();
	}

}

void* processThread(void* pTime)
{

	pthread_t timerThread;
	int rc;
	
	rc = pthread_create(&timerThread, NULL, &timer, (void*) pTime);
	if (rc)
	{
		cout << "ERROR: return code from pthread_create() is " << rc << endl;
		exit(-1);
	}
	pthread_join(timerThread, NULL);
	activeProcesses--;

}

/**
*	Function: hardDriveInputHandler
*	Description: A thread that handles an input process from the hard drive. Currently 
*	there are no processes to be performed, so this function does nothing but wait for 
*	the timer thread to finish
*/
void* hardDriveInputHandler(void* hdTime)
{

	sem_wait(&semaphoreHdd);

	PCB process = pcbContainer[loadedProcessIndeces[0]];
	pthread_t timerThread;
	clock_t start;
	int rc, pid = process.getpid();
	double duration;
	
	start = clock();
	duration = (clock() - start) / (double) CLOCKS_PER_SEC;
	process.updateProcessDuration(duration);

	rc = pthread_create(&timerThread, NULL, &timer, (void*)hdTime);
	if (rc)
	{
		cout << "ERROR: return code from pthread_create() is " << rc << endl;
		exit(-1);
	}
	
	pthread_join(timerThread, NULL);
	
	duration = ((clock() - start) / (double) CLOCKS_PER_SEC) - duration;
	process.updateProcessDuration(duration);
	
	switch (outputType)
	{
		case 0:
			cout << process.getProcessDuration() << " - Process " << pid << 
			": end hard drive input" << endl;
		break;
		case 1:
			fout << process.getProcessDuration() << " - Process " << pid << 
			": end hard drive input" << endl;
		break;
		case 2:
			cout << process.getProcessDuration() << " - Process " << pid << 
			": end hard drive input" << endl;
			fout << process.getProcessDuration() << " - Process " << pid << 
			": end hard drive input" << endl;
		break;
		default:
			cout << "ERROR: Incorrect log type recorded from config file" << endl;
			return 0;
	}
	
	process.processState = 1;
	
	sem_post(&semaphoreHdd);

}

/**
*	Function: keyboardHandler
*	Description: A thread that handles an input process from the keyboard. Currently 
*	there are no processes to be performed, so this function does nothing but wait for 
*	the timer thread to finish
*/
void* keyboardHandler(void* kTime)
{

	/*pthread_t timerThread;
	int rc;

	rc = pthread_create(&timerThread, NULL, &timer, (void*)kTime);
	if (rc)
	{
		cout << "ERROR: return code from pthread_create() is " << rc << endl;
		exit(-1);
	}
	
	pthread_mutex_lock(&mutexKeyboard);
	
	//further functionality will go here
	
	pthread_mutex_unlock(&mutexKeyboard);
	
	pthread_join(timerThread, NULL);*/
	
	pthread_mutex_lock(&mutexKeyboard);

	PCB process = pcbContainer[loadedProcessIndeces[0]];
	pthread_t timerThread;
	clock_t start;
	int rc, pid = process.getpid();
	double duration;
	
	start = clock();
	duration = (clock() - start) / (double) CLOCKS_PER_SEC;
	process.updateProcessDuration(duration);

	rc = pthread_create(&timerThread, NULL, &timer, (void*)kTime);
	if (rc)
	{
		cout << "ERROR: return code from pthread_create() is " << rc << endl;
		exit(-1);
	}
	
	pthread_join(timerThread, NULL);
	
	duration = ((clock() - start) / (double) CLOCKS_PER_SEC) - duration;
	process.updateProcessDuration(duration);
	
	switch (outputType)
	{
		case 0:
			cout << process.getProcessDuration() << " - Process " << pid << 
			": end keyboard input" << endl;
		break;
		case 1:
			fout << process.getProcessDuration() << " - Process " << pid << 
			": end keyboard input" << endl;
		break;
		case 2:
			cout << process.getProcessDuration() << " - Process " << pid << 
			": end keyboard input" << endl;
			fout << process.getProcessDuration() << " - Process " << pid << 
			": end keyboard input" << endl;
		break;
		default:
			cout << "ERROR: Incorrect log type recorded from config file" << endl;
			return 0;
	}
	
	process.processState = 1;
	
	pthread_mutex_unlock(&mutexKeyboard);

}

/**
*	Function: scannerHandler
*	Description: A thread that handles an input process from the scanner. Currently 
*	there are no processes to be performed, so this function does nothing but wait for 
*	the timer thread to finish
*/
void* scannerHandler(void* sTime)
{

	/*pthread_t timerThread;
	int rc;

	rc = pthread_create(&timerThread, NULL, &timer, (void*)sTime);
	if (rc)
	{
		cout << "ERROR: return code from pthread_create() is " << rc << endl;
		exit(-1);
	}
	
	pthread_mutex_lock(&mutexScanner);
	
	//further functionality will go here
	
	pthread_mutex_unlock(&mutexScanner);
	
	pthread_join(timerThread, NULL);
	
	pthread_mutex_lock(&mutexKeyboard);*/
	
	pthread_mutex_lock(&mutexScanner);

	PCB process = pcbContainer[loadedProcessIndeces[0]];
	pthread_t timerThread;
	clock_t start;
	int rc, pid = process.getpid();
	double duration;
	
	start = clock();
	duration = (clock() - start) / (double) CLOCKS_PER_SEC;
	process.updateProcessDuration(duration);

	rc = pthread_create(&timerThread, NULL, &timer, (void*)sTime);
	if (rc)
	{
		cout << "ERROR: return code from pthread_create() is " << rc << endl;
		exit(-1);
	}
	
	pthread_join(timerThread, NULL);
	
	duration = ((clock() - start) / (double) CLOCKS_PER_SEC) - duration;
	process.updateProcessDuration(duration);
	
	switch (outputType)
	{
		case 0:
			cout << process.getProcessDuration() << " - Process " << pid << 
			": end scanner input" << endl;
		break;
		case 1:
			fout << process.getProcessDuration() << " - Process " << pid << 
			": end scanner input" << endl;
		break;
		case 2:
			cout << process.getProcessDuration() << " - Process " << pid << 
			": end scanner input" << endl;
			fout << process.getProcessDuration() << " - Process " << pid << 
			": end scanner input" << endl;
		break;
		default:
			cout << "ERROR: Incorrect log type recorded from config file" << endl;
			return 0;
	}
	
	process.processState = 1;
	
	pthread_mutex_unlock(&mutexScanner);

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
	
	sem_wait(&semaphoreHdd);
	
	//further functionality will go here
	
	sem_post(&semaphoreHdd);
	
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
	
	pthread_mutex_lock(&mutexMonitor);
	
	//further functionality will go here
	
	pthread_mutex_unlock(&mutexMonitor);
	
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
	
	sem_wait(&semaphoreProj);
	
	//further functionality will go here
	
	sem_post(&semaphoreProj);
	
	pthread_join(timerThread, NULL);

}

/**
*	Function: output
*	Description: outputs to the monitor if the log type is 0 ("Monitor"), outputs to 
*		the file if the log type is 1 ("File"), or outputs to the file and the 
*		monitor if the log type is 2 ("Both").
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
	
	string scheduleType;
	if (configData.getCpuScheduleCode() == 0)
	{
		scheduleType = "FIFO";
	}
	else if (configData.getCpuScheduleCode() == 1)
	{
		scheduleType = "PS";
	}
	else if (configData.getCpuScheduleCode() == 2)
	{
		scheduleType = "SJF";
	}
	else if (configData.getCpuScheduleCode() == 3)
	{
		scheduleType = "STR";
	}
	else if (configData.getCpuScheduleCode() == 4)
	{
		scheduleType = "RR";
	}
	else
	{
		cout << "ERROR: using incorrect CPU scheduling code" << endl;
		return;
	}

	//Config Output
	cout << endl;
	cout << "Configuration File Data" << endl;
	cout << "Processor Quantum Number = " << configData.getProcessorQuantumNumber()
		 << endl;
	cout << "CPU Scheduling Code = " << scheduleType << endl;
	cout << "Monitor = " << configData.getMonitorTime() << " ms/cycle" << endl;
	cout << "Processor = " << configData.getProcessorTime() << " ms/cycle" << endl;
	cout << "Scanner = " << configData.getScannerTime() << " ms/cycle" << endl;
	cout << "Hard Drive = " << configData.getHardDriveTime() << " ms/cycle" << endl;
	cout << "Keyboard = " << configData.getKeyboardTime() << " ms/cycle" << endl;
	cout << "Memory = " << configData.getMemoryTime() << " ms/cycle" << endl;
	cout << "Projector = " << configData.getProjectorTime() << " ms/cycle" << endl;
	cout << "System Memory = " << configData.getSystemMemory() << " kbytes" << endl;
	cout << "Memory Block Size = " << configData.getBlockSize() << " kbytes" << endl;
	cout << "Projector Quantity = " << configData.getProjQuant() << endl;
	cout << "Hard Drive Quantity = " << configData.getHddQuant() << endl;
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
*		and the set of meta-data instructions to the file specified in the 
*		configuration file. Accounts for the possibility that the data could also be 
*		logged to the monitor.
*/
void outputToFile(Config configData, deque<MetaData> metaData)
{

	string scheduleType;
	if (configData.getCpuScheduleCode() == 0)
	{
		scheduleType = "FIFO";
	}
	else if (configData.getCpuScheduleCode() == 1)
	{
		scheduleType = "PS";
	}
	else if (configData.getCpuScheduleCode() == 2)
	{
		scheduleType = "SJF";
	}
	else if (configData.getCpuScheduleCode() == 3)
	{
		scheduleType = "STR";
	}
	else if (configData.getCpuScheduleCode() == 4)
	{
		scheduleType = "RR";
	}
	else
	{
		cout << "ERROR: using incorrect CPU scheduling code" << endl;
		return;
	}
	
	//ofstream fout;
	fout.open(configData.getLogPath());

	//Config Output
	fout << "Configuration File Data" << endl;
	fout << "Processor Quantum Number = " << configData.getProcessorQuantumNumber()
		 << endl;
	fout << "CPU Scheduling Code = " << scheduleType << endl;
	fout << "Monitor = " << configData.getMonitorTime() << " ms/cycle" << endl;
	fout << "Processor = " << configData.getProcessorTime() << " ms/cycle" << endl;
	fout << "Scanner = " << configData.getScannerTime() << " ms/cycle" << endl;
	fout << "Hard Drive = " << configData.getHardDriveTime() << " ms/cycle" << endl;
	fout << "Keyboard = " << configData.getKeyboardTime() << " ms/cycle" << endl;
	fout << "Memory = " << configData.getMemoryTime() << " ms/cycle" << endl;
	fout << "Projector = " << configData.getProjectorTime() << " ms/cycle" << endl;
	fout << "System Memory = " << configData.getSystemMemory() << " kbytes" << endl;
	fout << "Memory Block Size = " << configData.getBlockSize() << " kbytes" << endl;
	fout << "Projector Quantity = " << configData.getProjQuant() << endl;
	fout << "Hard Drive Quantity = " << configData.getHddQuant() << endl;
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

