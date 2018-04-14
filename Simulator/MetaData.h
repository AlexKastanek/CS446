/**
*	File Name: MetaData.h
*	Editors: Alex Kastanek
*	Project: CS446 Operating Systems Simulation
*	File Description: Header file for the class MetaData. Used to store an arbitrary 
*		instruction found in the meta-data file (.mdf)
*	Version: 01
*	Last Date Revised: 2/7/18
*/

#ifndef META_DATA
#define META_DATA

//library inclusions and directives

#include <iostream>
#include <fstream>
#include <string>
#include "Config.h"

using namespace std;

class MetaData
{

	//MetaData class public declarations
	public:
	
		MetaData();
		MetaData(char, string, int);
		~MetaData();
		
		int getCodeIndex(char);
		int getDescriptorIndex(int, string);
		
		bool parseMetaData(string);
		void calculateTotalTime(Config);
		
		char getCode();
		string getDescriptor();
		int getNumOfCycles();
		int getTotalTime();
		
		void setData(char, string, int, int);
		void setCode(char);
		void setDescriptor(string);
		void setNumOfCycles(int);
		void setTotalTime(int);
		
		void print();
	
	//MetaData class private declarations
	private:
		
		//constant array of chars used for the meta-data code database
		//these are all the possible codes that can be accepted from the meta-data file
		char codeDatabase[6] = {'S', 'A', 'P', 'I', 'O', 'M'};
		
		//constant arrays of strings used for the meta-data descriptor database
		//these are all the possible descriptors for each code that can be accepted from 
		//the meta-data file
		string sKeywordDatabase[2] = {"begin", "finish"};
		string aKeywordDatabase[2] = {"begin", "finish"};
		string pKeywordDatabase = "run";
		string iKeywordDatabase[3] = {"hard drive", "keyboard", "scanner"};
		string oKeywordDatabase[3] = {"hard drive", "monitor", "projector"};
		string mKeywordDatabase[2] = {"block", "allocate"};
		
		char code;
		string descriptor;
		int numOfCycles, totalTime;

};

#endif
