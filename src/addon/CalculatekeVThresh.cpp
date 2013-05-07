/*
 * CalculatekeVThresh.cpp                                 v0.1  18/03/2004
 *
 *
 * Copyright (C) by Robert Andritschke. 
 *  All rights reserved.
 *
 * This code implementation is the intellectual property of Robert Andritschke, Florian Schopper, Andreas Zoglauer 
 * at MPE.
 *
 * By copying, distributing or modifying the Program (or any work based on the 
 * Program) you indicate your acceptance of this statement, and all its terms.
 *
 */

/******************************************************************************

Program calculating threshold of detectors in keV from .csv files

******************************************************************************/


#include <fstream>
#include <sstream>
#include <cstdlib>
#include <iostream>
#include <vector>
using namespace std;

main(int argc, char** argv)
{
	if (argc != 2) {
		cout << "Usage: " << argv[0] << " <FileName>" << endl;
		exit (-1);
	}

	// 	string InFileName;
	// 	InFileName = argv[1];
	
	// open input file
	ifstream InFile(argv[1]);
	if (!InFile) {
		cout << "File \"" << argv[1] << "\" not found!" << endl;
		exit (-1);
	}

	// parse header
	string DetectorName;
	int version = 0;
	string type;
	string line;
	string firstarg;
	do {
		getline(InFile, line);
		istringstream parseline(line);
		parseline >> firstarg;
		if (firstarg.compare("DetectorName") == 0) parseline >> DetectorName;
		else if (firstarg.compare("Type") == 0) parseline >> type;
		else if (firstarg.compare("Version") == 0) parseline >> version;
	} while (!InFile.eof() && InFile.good());
	InFile.clear();
	InFile.seekg(0);

	if ((type.compare("CsISingle") != 0) && (type.compare("SiStrip") != 0) && 
			(version != 4)) {
		cout << "Not written for " << type << " version " << version << endl;
		exit (-1);
	} else {
		string OutFileName(DetectorName);
		OutFileName += ".thr";
		ofstream OutFile(OutFileName.c_str());
		if (!OutFile) {
			cout << "Cannot open \"" << OutFileName << "\" for output!" << endl;
		} else {
			cout << "Opening \"" << OutFileName << "\" for output!" << endl;
			OutFile << "Thresholds in keV" << endl;
			OutFile << "DetectorName " << DetectorName << endl;

			vector <vector<double> > ADCThresh;
			vector <vector <vector<double> > > Energies;
			vector <vector <vector<double> > > Values;
			if (type.compare("CsISingle") == 0) {
				ADCThresh.resize(10);
				Energies.resize(10);
				Values.resize(10);
				for (int i=0; i<ADCThresh.size(); ++i) {
					ADCThresh[i].resize(12);
					Energies[i].resize(12);
					Values[i].resize(12);
				}
			} else if (type.compare("SiStrip") == 0) {
				ADCThresh.resize(2);
				Energies.resize(2);
				Values.resize(2);
				for (int i=0; i<ADCThresh.size(); ++i) {
					ADCThresh[i].resize(384);
					Energies[i].resize(384);
					Values[i].resize(384);
				}
			}
			int x, y;
			string side;
			float E, Value;
			do {
				getline(InFile, line);
				istringstream parseline(line);
				parseline >> firstarg;
				try {
					if (firstarg.compare("CB") == 0) {
						if (type.compare("CsISingle") == 0) {
							parseline >> x >> y ;
							--x; --y;
						} else if (type.compare("SiStrip") == 0) {
							parseline >> side >> y;
							--y;
							if (side.compare("P") == 0) x=0;
							else x=1;
						}
						ADCThresh.at(x).at(y) = 0.0;
						Energies.at(x).at(y).resize(0);
						Values.at(x).at(y).resize(0);
					}
					if (firstarg.compare("CP") == 0) {
						if (type.compare("CsISingle") == 0) {
							parseline >> x >> y ;
							--x; --y;
						} else if (type.compare("SiStrip") == 0) {
							parseline >> side >> y;
							--y;
							if (side.compare("P") == 0) x=0;
							else x=1;
						}
						parseline >> E >> Value;
						Energies.at(x).at(y).push_back(E);
						Values.at(x).at(y).push_back(Value);
					}
					if (firstarg.compare("TH") == 0) {
						if (type.compare("CsISingle") == 0) {
							parseline >> x >> y ;
							--x; --y;
						} else if (type.compare("SiStrip") == 0) {
							parseline >> side >> y;
							--y;
							if (side.compare("P") == 0) x=0;
							else x=1;
						}
						parseline >> Value;
						ADCThresh.at(x).at(y) = Value;
					}
				} 
				catch (...) {
					cout << "Error: Wrong pixel x=" << x+1 << " y=" << y+1 << "!" << endl;
				}
			} while (!InFile.eof() && InFile.good());

			int n = 0;
			float sum = 0.0;
			float result;
			for (x=0; x<Energies.size(); ++x) {
				for (y=0; y<Energies[x].size(); ++y) {
					if (Energies[x][y].size() != 0) {
						if (Energies[x][y].size() == 1) {
							Energies[x][y].push_back(0.0);
							Values[x][y].push_back(0.0);
						}
						result =  (ADCThresh[x][y] - Values[x][y][0])/
							(Values[x][y][1] - Values[x][y][0]) * 
							(Energies[x][y][1] - Energies[x][y][0]) + 
							Energies[x][y][0]; 
						OutFile << result	<< endl;
						++n;
						sum += result;
					}
				}
			}
			OutFile << "TA " << sum/(float)n << endl;
			cout << "Threshold " << DetectorName << " = " << sum/(float)n << endl;
			OutFile.close();
		}
	}
	InFile.close();
}
