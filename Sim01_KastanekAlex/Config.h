/**
*	File Name: Config.h
*	Editors: Alex Kastanek
*	Project: CS446 Operating Systems Simulation
*	File Description: Header file for the class Config. Used to store the data obtained 
*		from the configuration file
*	Version: 01
*	Last Date Revised: 2/7/18
*/

#ifndef CONFIG
#define CONFIG

//library inclusion and directives

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using namespace std;

class Config
{

	//Config class public declarations
	public:
	
		Config();
		~Config();
		
		void getConfigData(ifstream&);
		
		int getKeywordIndex(string);
		bool parseComponentTime(string, string, int, int&);
		int getComponentTime(char, string);
		
		int getLogType();
		int getProjectorTime();
		int getProcessorTime();
		int getKeyboardTime();
		int getMonitorTime();
		int getScannerTime();
		int getHardDriveTime();
		int getMemoryTime();
		float getVersion();
		string getFilePath();
		string getLogPath();
		
		void setLogType(int);
		void setProjectorTime(int);
		void setProcessorTime(int);
		void setKeyboardTime(int);
		void setMonitorTime(int);
		void setScannerTime(int);
		void setHardDriveTime(int);
		void setMemoryTime(int);
		void setVersion(float);
		void setFilePath(string);
		void setLogPath(string);
	
	//Config class private declarations
	private:
	
		//constant array of strings used for the various keywords (device names) in the 
		//config file
		string keywordDatabase[7] = {"Monitor", "Processor", "Scanner", "Hard",
									 "Keyboard", "Memory", "Projector"};
	
		int logType;
		
		//processor cycle times, all inputs in msec
		int projectorTime, processorTime, keyboardTime, monitorTime,
			scannerTime, hardDriveTime, memoryTime;
			
		float version;
		
		string filePath, logPath;	

};

#endif
