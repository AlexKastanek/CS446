/**
*	File Name: Config.cpp
*	Editors: Alex Kastanek
*	Project: CS446 Operating Systems Simulation
*	File Description: Implementation file for the class Config. Used to store the data 
*		obtained from the configuration file
*	Version: 01
*	Last Date Revised: 2/7/18
*/

#include "Config.h"

/**
*	Function: Config
*	Description: Default constructor for Config class
*/
Config::Config()
{

}

/**
*	Function: ~Config
*	Description: Destructor for Config class
*/
Config::~Config()
{

}

/**
*	Function: getConfigData
*	Description: Goes through each line of the file (data passed through parameter) and 
*		checks for errors, extracts the necessary data, and stores the data into the 
*		private members of the class.
*/
void Config::getConfigData(ifstream& fin)
{
	string line, word;
	
	//first line error check
	getline(fin, line);
	if (line != "Start Simulator Configuration File")
	{
		cout << "ERROR: Typo on line 1" << endl;
		return;
	}
	line.clear();
	
	//second line error check and data collection
	fin >> word;
	if (word != "Version/Phase:")
	{
		cout << "ERROR: Typo on line 2" << endl;
		return;
	}
	word.clear();
	fin >> version;
	if (version < 0)
	{
		cout << "ERROR: Invalid version number" << endl;
		return;
	}
	
	//third line error check and data collection
	fin >> word;
	if (word != "File")
	{
		cout << "ERROR: typo on line 3" << endl;
		return;
	}
	fin >> word;
	if (word != "Path:")
	{
		cout << "ERROR: typo on line 3" << endl;
		return;
	}
	word.clear();
	fin >> filePath;
	
	//device cycle times error check and data collection
	//my implementation allows for the device times to be listed in any order so long as
	//they follow the correct format and there does not exist a time that is less than or
	//equal to zero
	string monitorConstString = "display time {msec}:";
	string hardDriveConstString = "drive cycle time {msec}:";
	string otherConstString = "cycle time {msec}:";
	for (int i = 0; i < 7; i++)
	{
		int keywordIndex;
	
		fin >> word;
		keywordIndex = getKeywordIndex(word);
		//if word is not one of the keywords, there is a typo, and getKeywordIndex(word)
		//will return 0
		if (keywordIndex == 0)
		{
			cout << "ERROR: typo on line " << i + 4 << endl;
			return;
		}
		
		char c;
		fin.get(c); //using this to throw away the white space
		
		getline(fin, word); //getting rest of line
		int componentTime = 0; //default value for component time (will be changed)
		//begin extraction process for the component's time
		switch (keywordIndex)
		{
			case 1: //keyword is Monitor
				
				if (parseComponentTime(word, monitorConstString, i, componentTime))
				{
					monitorTime = componentTime;
				}
				else return;

			break;
			
			case 2: //keyword is Processor
				
				if (parseComponentTime(word, otherConstString, i, componentTime))
				{
					processorTime = componentTime;
				}
				else return;

			break;
			
			case 3: //keyword is Scanner
				
				if (parseComponentTime(word, otherConstString, i, componentTime))
				{
					scannerTime = componentTime;
				}
				else return;

			break;
			
			case 4: //keyword is Hard drive
				
				if (parseComponentTime(word, hardDriveConstString, i, componentTime))
				{
					hardDriveTime = componentTime;
				}
				else return;

			break;
			
			case 5: //keyword is Keyboard
				
				if (parseComponentTime(word, otherConstString, i, componentTime))
				{
					keyboardTime = componentTime;
				}
				else return;

			break;
			
			case 6: //keyword is Memory
				
				if (parseComponentTime(word, otherConstString, i, componentTime))
				{
					memoryTime = componentTime;
				}
				else return;

			break;
			
			case 7: //keyword is Projector
				
				if (parseComponentTime(word, otherConstString, i, componentTime))
				{
					projectorTime = componentTime;
				}
				else return;

			break;
			
			default:
			
				cout << "ERROR: error in cycle times" << endl;
				return;
			
		}
	}
	word.clear();
	
	//line 11 error check and data collection
	fin >> word;
	if (word != "System")
	{
		cout << "ERROR: typo on line 11" << endl;
		return;
	}
	fin >> word;
	if (word != "memory")
	{
		cout << "ERROR: typo on line 11" << endl;
		return;
	}
	char dataConvType = 'k';
	fin >> word;
	if (word == "{kbytes}:")
	{
		dataConvType = 'k';
	}
	else if (word == "{Mbytes}:")
	{
		dataConvType = 'M';
	}
	else if (word == "{Gbytes}:")
	{
		dataConvType = 'G';
	}
	else
	{
		cout << "ERROR: typo on line 11" << endl;
		return;
	}
	word.clear();
	int parsedSystemMemory;
	fin >> word;
	parsedSystemMemory = atoi(word.c_str());
	if (parsedSystemMemory < 1)
	{
		cout << "ERROR: invalid system memory" << endl;
		return;
	}
	word.clear();
	systemMemory = convertToKilobytes(parsedSystemMemory, dataConvType);
	getline(fin, line);
	
	//line 12 error check and data collection
	getline(fin, line);
	if (line == "Log: Log to Monitor")
	{
		logType = 0;
	}
	else if (line == "Log: Log to File")
	{
		logType = 1;
	}
	else if (line == "Log: Log to Both")
	{
		logType = 2;
	}
	else
	{
		cout << "ERROR: Typo on line 12" << endl;
		return;
	}
	line.clear();
	
	//line 13 error check and data collection
	fin >> word;
	if (word != "Log")
	{
		cout << "ERROR: Typo on line 13" << endl;
		return;
	}
	fin >> word;
	if (word != "File")
	{
		cout << "ERROR: Typo on line 13" << endl;
		return;
	}
	fin >> word;
	if (word != "Path:")
	{
		cout << "ERROR: Typo on line 13" << endl;
		return;
	}
	word.clear();
	fin >> logPath;
	
	//line 14 error check
	getline(fin, line); //eliminating rest of current line
	getline(fin, line);
	if (line != "End Simulator Configuration File")
	{
		cout << "ERROR: Typo on line 14" << endl;
		return;
	}
	line.clear();
	
}

/**
*	Function: getKeywordIndex
*	Description: A linear search for the parameter keyword in the constant array of 
*		strings keywordDatabase. Returns the index of the keyword if search succeeds. 
*		Returns 0 if search fails.
*/
int Config::getKeywordIndex(string keyword)
{
	for (int i = 0; i < 7; i++)
	{
		if (keyword == keywordDatabase[i])
		{
			return i + 1;
		}
	}
	
	return 0; //no matches
}

/**
*	Function: parseComponentTime
*	Description: Parses through the rest of the line in the configuration file. The line 
*		has already been partially parsed, but the rest of the line, after the keyword, 
*		has yet to be parsed. This function error checks the rest of the line as well as 
*		extracts the component time at the end of the line and converts it to an int. 
*		The parameter word is what is going to be parsed. typeOfConstString is the type 
*		of constant string to expect from the line -- there are only three and they are 
*		specified in the getConfigData function. currentLine is the number of the line 
*		currently being parsed. componentTime is the variable that will hold the parsed 
*		time. It is passed by reference, so componentTime is changed out of scope. 
*		Returns true if the component time is successfully parsed from the rest of the 
*		line, returns false if the process is unsuccessful.
*/
bool Config::parseComponentTime(string word, string typeOfConstString, int currentLine, int& componentTime)
{
	int begLocation, timeLength; //location of first character of the time in the string
								 //as well as the length of the time
	int endOfString = word.length() - 1; //end is at very end of string
	int j = endOfString; //assigning decrementer j to end of string
	while (word[j - 1] != ' ')
	{
		j--;
	}
	begLocation = j; //beginning is where the white space starts
	timeLength = endOfString - j + 1; //length of the time is length of the string - j
	if (timeLength < 1 || timeLength > 3)
	{
		//checking if the length of the time is too short or long
		//less than one character is too short, and I am assuming that larger than 3 
		//characters would be too big
		cout << "ERROR: invalid time on line " << currentLine + 4 << endl;
		return 0;
	}
	string time = word.substr(begLocation, timeLength); //parsed string
	string rest = word.substr(0, begLocation - 1); //rest of string
	if (rest != typeOfConstString) //checking for typos in rest of string
	{
		cout << "ERROR: typo on line " << currentLine + 4 << endl;
		return 0;
	}
	int parsedTime = atoi(time.c_str()); //converting cycle time string to int
	if (parsedTime <= 0)
	{
		//checking if time is negative or equal to zero
		cout << "ERROR: invalid time on line " << currentLine + 4 << endl;
		return 0;
	}
	componentTime = parsedTime; //setting componentTime
	return 1;
}

/**
*	Function: getComponentTime
*	Description: This function is used with the meta-data ADT to get the correct device 
*		time. Given the meta-data code and the meta-data descriptor as parameters, uses 
*		the relationship between the two to return the correct device time
*/
int Config::getComponentTime(char code, string descriptor)
{

	if (code == 'P')
	{
		return processorTime;
	}
	else if (code == 'I')
	{
		if (descriptor == "hard drive")
		{
			return hardDriveTime;
		}
		else if (descriptor == "keyboard")
		{
			return keyboardTime;
		}
		else if (descriptor == "scanner")
		{
			return scannerTime;
		}
		else
		{
			cout << "ERROR: error ocurred while retrieving the component cycle time" << 
			endl;
			return -1;
		}
	}
	else if (code == 'O')
	{
		if (descriptor == "hard drive")
		{
			return hardDriveTime;
		}
		else if (descriptor == "monitor")
		{
			return monitorTime;
		}
		else if (descriptor == "projector")
		{
			return projectorTime;
		}
		else
		{
			cout << "ERROR: error ocurred while retrieving the component cycle time" << 
			endl;
			return -1;
		}
	}
	else if (code == 'M')
	{
		return memoryTime;
	}
	else
	{
		return 0;
	}
	
}

/**
*	Function: getLogType
*	Description: returns the log type specified in the configuration file
*/
int Config::getLogType()
{
	return logType;
}

/**
*	Function: getProjectorTime
*	Description: returns the projector time (msec) specified in the configuration file
*/
int Config::getProjectorTime()
{
	return projectorTime;
}

/**
*	Function: getProcessorTime
*	Description: returns the processor time (msec) specified in the configuration file
*/
int Config::getProcessorTime()
{
	return processorTime;
}

/**
*	Function: getKeyboardTime
*	Description: returns the keyboard time (msec) specified in the configuration file
*/
int Config::getKeyboardTime()
{
	return keyboardTime;
}

/**
*	Function: getMonitorTime
*	Description: returns the monitor time (msec) specified in the configuration file
*/
int Config::getMonitorTime()
{
	return monitorTime;
}

/**
*	Function: getScannerTime
*	Description: returns the scanner time (msec) specified in the configuration file
*/
int Config::getScannerTime()
{
	return scannerTime;
}

/**
*	Function: getHardDriveTime
*	Description: returns the hard drive time (msec) specified in the configuration file
*/
int Config::getHardDriveTime()
{
	return hardDriveTime;
}

/**
*	Function: getMemoryTime
*	Description: returns the memory time (msec) specified in the configuration file
*/
int Config::getMemoryTime()
{
	return memoryTime;
}

/**
*	Function: getSystemMemory
*	Description: returns the amount of system memory allocated (kbytes) specified in the 
*		configuration file
*/
int Config::getSystemMemory()
{
	return systemMemory;
}

/**
*	Function: getVersion
*	Description: returns the version number specified in the configuration file
*/
float Config::getVersion()
{
	return version;
}

/**
*	Function: getFilePath
*	Description: returns the meta-data file path specified in the configuration file
*/
string Config::getFilePath()
{
	return filePath;
}

/**
*	Function: getLogPath
*	Description: returns the log file path specified in the configuration file
*/
string Config::getLogPath()
{
	return logPath;
}

/**
*	Function: setLogType
*	Description: sets the log type to the parameter: source
*/
void Config::setLogType(int source)
{
	logType = source;
}

/**
*	Function: setProjectorTime
*	Description: sets the projector time to the parameter: source
*/
void Config::setProjectorTime(int source)
{
	projectorTime = source;
}

/**
*	Function: setProcessorTime
*	Description: sets the processor time to the parameter: source
*/
void Config::setProcessorTime(int source)
{
	processorTime = source;
}

/**
*	Function: setKeyboardTime
*	Description: sets the keyboard time to the parameter: source
*/
void Config::setKeyboardTime(int source)
{
	keyboardTime = source;
}

/**
*	Function: setMonitorTime
*	Description: sets the monitor time to the parameter: source
*/
void Config::setMonitorTime(int source)
{
	monitorTime = source;
}

/**
*	Function: setScannerTime
*	Description: sets the scanner time to the parameter: source
*/
void Config::setScannerTime(int source)
{
	scannerTime = source;
}

/**
*	Function: setHardDriveTime
*	Description: sets the hard drive time to the parameter: source
*/
void Config::setHardDriveTime(int source)
{
	hardDriveTime = source;
}

/**
*	Function: setMemoryTime
*	Description: sets the memory time to the parameter: source
*/
void Config::setMemoryTime(int source)
{
	memoryTime = source;
}

/**
*	Function: setSystemMemory
*	Description: sets the system memory to the parameter: source
*/
void Config::setSystemMemory(int source)
{
	systemMemory = source;
}

/**
*	Function: setVersion
*	Description: sets the version number to the parameter: source
*/
void Config::setVersion(float source)
{
	version = source;
}

/**
*	Function: setFilePath
*	Description: sets the meta-data file path to the parameter: source
*/
void Config::setFilePath(string source)
{
	filePath = source;
}

/**
*	Function: setLogPath
*	Description: sets the log path to the parameter: source
*/
void Config::setLogPath(string source)
{
	logPath = source;
}

int Config::convertToKilobytes(int data, char dataType)
{
	if (dataType == 'M')
	{
		return data * 1000;
	}
	else if (dataType == 'G')
	{
		return data * 1000 * 1000;
	}
	else return data;
}







