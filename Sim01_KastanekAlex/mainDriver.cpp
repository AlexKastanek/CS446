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
#include "Config.h"
#include "MetaData.h"

using namespace std;

//function headers

bool getMetaData(ifstream&, Config, deque<MetaData>&);

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

	Config configData; //Config ADT used to store data from the config file
	deque<MetaData> instructionSet; //double-ended queue of the ADT MetaData - used to 
									//store data from an instruction in the Meta Data 
									//file
	string metaDataFile; //used to store the filename of the meta data file
	bool okToContinue; //used to stop the program if error ocurred

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
			return 0;
		}
		else if (i > 0)
		{
			//storing config file data in config class
			configData.getConfigData(fin);
		}
		fin.close();
	}
	
	//assigning metaDataFile to file name specified in config data
	metaDataFile = configData.getFilePath();

	//begin meta-data file error checking and input
	fin.open(metaDataFile);
	if (!fin.is_open())
	{
		//file does not exist
		cout << "ERROR: Meta-Data file specified in config file does not exist" << endl;
		return 0;
	}
	else
	{
		//storing meta data file to instructionSet - a double-ended queue of Meta-Data
		//okToContinue is true if no errors occur during the data retrieval process
		okToContinue = getMetaData(fin, configData, instructionSet);
	}
	fin.close();
	
	//begin output
	if (okToContinue)
	{
		output(configData, instructionSet);
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
		MetaData currentData; //temporary storage, will be stored in instructionSet once 
							  //all errors are checked
		if (!currentData.parseMetaData(instruction))
		{
			//error occurred while parsing meta-data
			cout << "ERROR: Error in meta-data" << endl;
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
*	Function: output
*	Description: outputs to the monitor if the log type is 0 ("Monitor"), outputs to the 
*		file if the log type is 1 ("File"), or outputs to the file and the monitor if *		the log type is 2 ("Both").
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
	cout << "Configuration File Data" << endl;
	cout << "Monitor = " << configData.getMonitorTime() << " ms/cycle" << endl;
	cout << "Processor = " << configData.getProcessorTime() << " ms/cycle" << endl;
	cout << "Scanner = " << configData.getScannerTime() << " ms/cycle" << endl;
	cout << "Hard Drive = " << configData.getHardDriveTime() << " ms/cycle" << endl;
	cout << "Keyboard = " << configData.getKeyboardTime() << " ms/cycle" << endl;
	cout << "Memory = " << configData.getMemoryTime() << " ms/cycle" << endl;
	cout << "Projector = " << configData.getProjectorTime() << " ms/cycle" << endl;
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
	ofstream fout;
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
	
	fout.close();
}

