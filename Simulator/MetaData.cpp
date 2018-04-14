/**
*	File Name: MetaData.cpp
*	Editors: Alex Kastanek
*	Project: CS446 Operating Systems Simulation
*	File Description: Implementation file for the class MetaData. Used to store an 
*		arbitrary instruction found in the meta-data file (.mdf)
*	Version: 01
*	Last Date Revised: 2/7/18
*/

#include "MetaData.h"

/**
*	Function: MetaData
*	Description: Default constructor for MetaData class
*/
MetaData::MetaData()
{

}

/**
*	Function: MetaData
*	Description: Parameterized constructor for MetaData class with parameters 
*		codeSource, descriptorSource, and cycleSource used to initialize code, 
*		descriptor, and numOfCycles respectively
*/
MetaData::MetaData(char codeSource, string descriptorSource, int cycleSource)
{

	code = codeSource;
	descriptor = descriptorSource;
	numOfCycles = cycleSource;

}

/**
*	Function: ~MetaData
*	Description: Destructor for MetaData class
*/
MetaData::~MetaData()
{

}

/**
*	Function: getCodeIndex
*	Description: Searches through the code database for the parameter codeSource. If 
*		successful, will return a number 1-6, depending on the index of the code in the 
*		array. If not successful, returns 0.
*/
int MetaData::getCodeIndex(char codeSource)
{

	for (int i = 0; i < 6; i++)
	{
		if (codeSource == codeDatabase[i])
		{
			return i + 1;
		}
	}
	
	return 0;

}

/**
*	Function: getDescriptorIndex
*	Description: Searches through the keyword database for the parameter 
*		descriptorSource. Uses the codeIndex to search through the correct database. If 
*		successful, will return a number, depending on the index of the descriptor 
*		in the array. If not successful, returns 0.
*/
int MetaData::getDescriptorIndex(int codeIndex, string descriptorSource)
{

	switch (codeIndex)
	{
		case 1:
		
			for (int i = 0; i < 2; i++)
			{
				if (descriptorSource == sKeywordDatabase[i])
				{
					return i + 1;
				}
			}
			return 0;
			
		break;
		
		case 2:
		
			for (int i = 0; i < 2; i++)
			{
				if (descriptorSource == aKeywordDatabase[i])
				{
					return i + 1;
				}
			}
			return 0;
			
		break;
		
		case 3:
		
			if (descriptorSource == pKeywordDatabase)
			{
				return 1;
			}
			return 0;
			
		break;
		
		case 4:
		
			for (int i = 0; i < 3; i++)
			{
				if (descriptorSource == iKeywordDatabase[i])
				{
					return i + 1;
				}
			}
			return 0;
			
		break;
		
		case 5:
		
			for (int i = 0; i < 3; i++)
			{
				if (descriptorSource == oKeywordDatabase[i])
				{
					return i + 1;
				}
			}
			return 0;
			
		break;
		
		case 6:
		
			for (int i = 0; i < 2; i++)
			{
				if (descriptorSource == mKeywordDatabase[i])
				{
					return i + 1;
				}
			}
			return 0;
			
		break;
		
		default: 
			
			cout << "ERROR: ocurred when getting descriptor index" << endl;
			return 0;
	}

}

/**
*	Function: parseMetaData
*	Description: Given the string instruction passed as parameter, parses through and 
*		sets the private members code, descriptor, and numOfCycles respectively. 
*		Accounts for any typos for codes and descriptors as well as the format of the 
*		instruction (e.g. the instruction must end in a semi-colon and the last 
*		instruction must end in a period). Also accounts for invalid entries for the 
*		number of cycles (e.g. cannot exceed two characters long and cannot be less than 
*		zero)
*/
bool MetaData::parseMetaData(string instruction)
{

	int begLocation, descriptorLength, numOfCyclesLength;
	int codeIndex, descriptorIndex;
	
	//checking if code exists in the database
	codeIndex = getCodeIndex(instruction[0]);
	if (codeIndex == 0)
	{
		cout << "ERROR: Invalid code" << endl;
		return 0;
	}
	code = codeDatabase[codeIndex - 1];
	
	//checking for correct formatting
	if (instruction[1] != '{')
	{
		cout << "ERROR: Typo in MetaData file" << endl;
		return 0;
	}
	
	//setting begLocation to be after "<meta-data code>{"
	begLocation = 2;
	int i = 2;
	//loop runs until the closing bracket is hit, i will get the length of the descriptor
	while (instruction[i] != '}')
	{
		i++;
	}
	//if i is greater than 12, then the descriptor is bigger than any in the database 
	//and some error occurred
	if (i > 12)
	{
		cout << "ERROR: Invalid descriptor - descriptor too large" << endl;
		return 0;
	}
	//setting descriptor length excluding the curly brackets
	descriptorLength = i - 2;
	
	//begin descriptor extraction process
	string parsedDescriptor = instruction.substr(begLocation, descriptorLength);
	//checking if descriptor exists in the database
	descriptorIndex = getDescriptorIndex(codeIndex, parsedDescriptor);
	if (descriptorIndex == 0)
	{
		cout << "ERROR: Invalid descriptor - error ocurred when indexing descriptor" << 
		endl;
		return 0;
	}
	//assigning matching descriptor in database to descriptor
	switch (codeIndex)
	{
		case 1:
		
			descriptor = sKeywordDatabase[descriptorIndex - 1];
		
		break;
		
		case 2:
		
			descriptor = aKeywordDatabase[descriptorIndex - 1];
		
		break;
		
		case 3:
		
			descriptor = pKeywordDatabase;
		
		break;
		
		case 4:
		
			descriptor = iKeywordDatabase[descriptorIndex - 1];
		
		break;
		
		case 5:
		
			descriptor = oKeywordDatabase[descriptorIndex - 1];
		
		break;
		
		case 6:
		
			descriptor = mKeywordDatabase[descriptorIndex - 1];
		
		break;
		
		default:
		
			cout << "ERROR: Error ocurred during descriptor assignment" << endl;
			return 0;
	}
	
	//setting begLocation to be after "<meta-data code>{<meta-data descriptor>}"
	begLocation = descriptorLength + 3;
	i = begLocation;
	//loop runs until either terminator is hit (';' or '.')
	while (instruction[i] != ';' && instruction[i] != '.')
	{
		i++;
	}
	if ((i - begLocation) > 2)
	{
		//checking if number of cycles exceeds two characters, considered to be invalid 
		//if it does
		cout << "ERROR: Invalid number of cycles - error ocurred in input" << endl;
		return 0;
	}
	numOfCyclesLength = i - begLocation;
	
	//begin numOfCycles extraction process
	string parsedCycles = instruction.substr(begLocation, numOfCyclesLength);
	if (parsedCycles.length() == 0)
	{
		cout << "ERROR: No number of cycles found in Meta Data file" << endl;
		return 0;
	}
	//converting string to int
	int parsedNumOfCycles = atoi(parsedCycles.c_str());
	if (parsedNumOfCycles < 0)
	{
		//checking if the number is a negative number
		cout << "ERROR: Invalid number of cycles - error ocurred during parsing" << endl;
		return 0;
	}
	numOfCycles = parsedNumOfCycles;
	
	return 1;

}

/**
*	Function: calculateTotalTime
*	Description: Calculates the total time of the device by accessing the configuration 
*		data and multiplying the correct device cycle time by the number of cycles. 
*		Private member totalTime gets this value.
*/
void MetaData::calculateTotalTime(Config configData)
{

	int deviceCycleTime = configData.getComponentTime(code, descriptor);
	totalTime = deviceCycleTime * numOfCycles;

}

/**
*	Function: getCode
*	Description: returns the meta-data code for this instruction
*/
char MetaData::getCode()
{

	return code;

}

/**
*	Function: getDescriptor
*	Description: returns the meta-data descriptor for this instruction
*/
string MetaData::getDescriptor()
{

	return descriptor;

}

/**
*	Function: getNumOfCycles
*	Description: returns the meta-data number of cycles for this instruction
*/
int MetaData::getNumOfCycles()
{

	return numOfCycles;

}

/**
*	Function: getTotalTime
*	Description: returns the total amount of time this instruction will take to perform
*/
int MetaData::getTotalTime()
{

	return totalTime;

}

/**
*	Function: setData
*	Description: sets the data to the parameters: codeSource, descriptorSource,
*   	cycleSource, and totalTimeSource
*/
void MetaData::setData(char codeSource, string descriptorSource, int cycleSource,
					   int totalTimeSource)
{

	code = codeSource;
	descriptor = descriptorSource;
	numOfCycles = cycleSource;
	totalTime = totalTimeSource;

}

/**
*	Function: setCode
*	Description: sets the code to the parameter: codeSource
*/
void MetaData::setCode(char codeSource)
{

	code = codeSource;

}

/**
*	Function: setDescriptor
*	Description: sets the descriptor to the parameter: descriptorSource
*/
void MetaData::setDescriptor(string descriptorSource)
{

	descriptor = descriptorSource;

}

/**
*	Function: setNumOfCycles
*	Description: sets the numOfCycles to the parameter: cycleCource
*/
void MetaData::setNumOfCycles(int cycleSource)
{

	numOfCycles = cycleSource;

}

/**
*	Function: setTotalTime
*	Description: sets the totalTime to the parameter: totalTimeSource
*/
void MetaData::setTotalTime(int totalTimeSource)
{

	totalTime = totalTimeSource;

}

void MetaData::print()
{

	cout << code << "{" << descriptor << "}" << numOfCycles << "-" << totalTime;

}













