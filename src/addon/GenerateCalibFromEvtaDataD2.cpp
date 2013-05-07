/*
 * GenerateCalibFromEvtaData.cpp                              v0.1  30.07.2003
 *
 *
 * Copyright (C) by Robert Andritschke. All rights reserved.
 *
 * This code implementation is the intellectual property of Robert Andritschke, Florian Schopper, Andreas Zoglauer 
 * at MPE.
 *
 * By copying, distributing or modifying the Program (or any work based on the 
 * Program) you indicate your acceptance of this statement, and all its terms.
 *
 */

/*******************************************************************************

Program for generating calib files from evta data.
This gives the posibility to implement some filters on the calibration data 

********************************************************************************/

#include <iostream>
#include <cstdlib>
#include <fstream>
#include <iostream>

// root classes
#include <MString.h>
#include <TH1.h>
#include <TH2.h>

// Megalyze classes
#include "MDaq.h"
#include "MCsIPixelSingle.h"

using namespace std;

int main(int argc, char **argv)
{

	string InputLine="";
	MString SetupFileName="";
	string DataFileName="";


	// should be parsed or autorecognized later on:
	int DetectorType = 2; // 1 for SiStrip, 2 for CsI (definitions like MDetector)

	// first command line parsing...
	switch (argc) {
	case 0:
		cout << "Usage: GenerateCalibFromEvtaData DirName" << endl;
		exit(1);
	case 2:
		cout << "Using data from directory " << argv[1] << endl;
		break;
	default:
		cout << "Wrong number of arguments!" << endl;
		cout << "Usage: " << argv[0] << " DirName" << endl;
		exit(1);
	}

	// get name of setupfile and data (.uevta) file
	// check directory
	const string FileList="Files.lst";
	string DirCall="ls ";
	DirCall += argv[1];
	DirCall += " >";
	DirCall += FileList;
	if (0 != system(DirCall.c_str())) {
		cout << "Directory  " << argv[1] << "  not found! Exiting ...";
		exit(1);
	}

	// open directory list
	ifstream DirList(FileList.c_str());
	if (!DirList) {
		cout << "Cannot open directry list " << FileList << "! Exiting ..." << endl;
 		exit(1);
 	}

	// find file names of setup and data files
	for ( SetupFileName="", DataFileName=""; 
				((SetupFileName=="") || (DataFileName=="")) && (!DirList.eof()); ) {
		getline(DirList, InputLine);
		MString InputLineT(InputLine.c_str());		

		if (InputLineT.EndsWith(".setup")) {
			SetupFileName = argv[1];
			SetupFileName += "/";
			SetupFileName += InputLineT;
			cout << "Setupfile found: " << SetupFileName << endl;
			continue;
		}
	
		if (InputLineT.EndsWith(".uevta")) {
			DataFileName = argv[1];
			DataFileName += "/";
			DataFileName += InputLine;
			cout << "Datafile found: " << DataFileName << endl;
			continue;
		}
	}

	// Close Dir list
	DirList.close();
		

	// opening Daq 
	MDaq Daq;
	Daq.Setup(SetupFileName);
	Daq.SetAnalysisMode(15);

	// Generate list of names and pointers to calib histograms
	vector <MDetector *> DetectorList;
	vector <MString> DetNames;
	vector <MString> DetShortNames;
	vector <MDetector *> DetectorList3D;
	vector <MString> DetNames3D;
	vector <MString> DetShortNames3D;

	TObjArray* AllDetectorList = Daq.GetDetectorList();
	for (int i = 0; i< AllDetectorList->GetEntriesFast(); ++i) {
		MDetector * Detector = (MDetector *)AllDetectorList->At(i);

		// search for D2 Single
		if (DetectorType == 2) {
			if ( ( Detector->GetBelonging() == 2) && (Detector->GetNChips()==1) ) {
				DetectorList.push_back(Detector);
				DetNames.push_back(Detector->GetDetName());
				DetShortNames.push_back(Detector->GetShortName());
			}
			if ( ( Detector->GetBelonging() == 2) && (Detector->GetNChips()==2) ) {
				DetectorList3D.push_back(Detector);
				DetNames3D.push_back(Detector->GetDetName());
				DetShortNames3D.push_back(Detector->GetShortName());
			}
		}
	}
	
	cout << "Found detectors: " << endl;
	for (UInt_t i = 0; i<DetectorList.size(); ++i) {
		cout << "  " << DetNames[i] << "  " << DetShortNames[i] << endl;
	}
 

	// open input file
	ifstream DataFile(DataFileName.c_str());
	if (!DataFile) {
		cout << "Cannot open input file " << DataFileName << "! Exiting ..." << endl;
		exit(1);
	}

	// skipping header
	for(;!DataFile.eof();) {
		getline(DataFile, InputLine);
		if (InputLine.size() < 3) continue; // Line empty
		if ((InputLine[0] == 'H') && (InputLine[1] == 'E')) continue;
		break;
	}


	// Loop over all events
	do {

		// Get one event after the other
		vector <string> EventLines;

		do {
			if ((InputLine[0] == 'S') && (InputLine[1] == 'E')) break;
			getline(DataFile, InputLine);
		} while (!DataFile.eof());
		
		do {
			getline(DataFile, InputLine);
			if ((InputLine[0] == 'S') && (InputLine[1] == 'E')) break;
			EventLines.push_back(InputLine);
		} while (!DataFile.eof());

		// HERE FILTERING STARTS !!!!!!

// ******** Filter only hits of detectors which have triggered
//		vector <MString> TriggeredDetectors;
//		vector <MDetector *> TriggeredDetPointers;
//
// 		// Check for triggered detectors	
// 		for (UInt_t i=0; i<	EventLines.size(); ++i) {
// 			if ((EventLines[i][0] == 'T') && (EventLines[i][1] == 'M')) {
// 				MString EventLine(EventLines[i].c_str());
// 				for (UInt_t j=0; j<DetectorList.size(); ++j) {
// 					if (EventLine.Contains(DetShortNames[j])) {
// 						TriggeredDetectors.push_back(DetShortNames[j]);
// 						TriggeredDetPointers.push_back(DetectorList[j]);
// 					}
// 				}
// 				break;
// 			}
// 		}

// 		// Get hits from triggered detectors and fill calib histogram
// 		for (UInt_t i=0; i<	EventLines.size(); ++i) {
// 			if ((EventLines[i][0] == 'H') && (EventLines[i][1] == 'T')) {
// 				// Replace ; by Space
// 				for (UInt_t j=2; j<EventLines[i].size(); ++j) {
// 					if (EventLines[i][j]==';') EventLines[i][j]=' ';
// 				} 
// 				MString EventLine(EventLines[i].c_str());
// 				for (UInt_t j=0; j<TriggeredDetectors.size(); ++j) {
// 					if (EventLine.Contains(TriggeredDetectors[j])) {
// 						char chardummy[10];
// 						float CMM, val;
// 						int x,y;
// 						sscanf(EventLines[i].c_str(), "%s %f %i %i %f", chardummy, &CMM, &x, &y, &val);
// 						if ( (x<0)||(x>9)||(y<0)||(y>11) ) {
// 							cout << "Pixel out of range: x=" << x << "; y=" << y << "!! Skipping..." << endl;
// 							continue;
// 						}
// 					 	((MCsIPixelSingle *) (TriggeredDetPointers[j]))->GetCalibHist(x , y)->Fill(val);
// 					}
// 				}
// 			}
// 		}	
// ******** END (Filter out hits of detectors without trigger)


// ******** Fill only single hits in calib spectrum
		//vector <MString> DetNameWithSignal;
		vector <float> DetSignal;
		vector <MDetector *> DetPointers;
		vector <int> xv;
		vector <int> yv;
		Bool_t Triggered3D = kFALSE;

		for (UInt_t i=0; i<	EventLines.size(); ++i) {
			if ((EventLines[i][0] == 'H') && (EventLines[i][1] == 'T')) {
				// Replace ; by Space
				for (UInt_t j=2; j<EventLines[i].size(); ++j) {
					if (EventLines[i][j]==';') EventLines[i][j]=' ';
				} 
				MString EventLine(EventLines[i].c_str());
				char chardummy[10];
				float CMM, val;
				int x,y;
	
				for (UInt_t j=0; j<DetectorList.size(); ++j) {
 					if (EventLine.Contains(DetShortNames[j])) {
						sscanf(EventLines[i].c_str(), "%s %f %i %i %f", chardummy, &CMM, &x, &y, &val);
						if ( (x<0)||(x>9)||(y<0)||(y>11) ) {
							cout << "Pixel out of range: x=" << x << "; y=" << y << "!! Skipping..." << endl;
							continue;
						}
						if ( (-CMM)==val) continue; // there are lines with values of the negative common mode
						//DetNameWithSignal.push_back(DetShortNames[j]);
 						DetPointers.push_back(DetectorList[j]);
						DetSignal.push_back(val);
						xv.push_back(x);
 						yv.push_back(y);
					}
				}
				// Has a 3D a signal?
				for (UInt_t j=0; j<DetectorList3D.size(); ++j) {
					if (EventLine.Contains(DetShortNames3D[j])) {
						Triggered3D=kTRUE;
					}
				}
			}
		}
	
		
		if ((DetPointers.size() == 1) && (Triggered3D==kFALSE)) {
			((MCsIPixelSingle *) (DetPointers[0]))->GetCalibHist(xv[0], yv[0])->Fill(DetSignal[0]);
		}

 		// ******** END (Fill only single hit in calib spectrum)


// ******** Fill every hit in calib spectrum exept signal=-common mode
// 		vector <float> DetSignal;
// 		vector <MDetector *> DetPointers;
// 		vector <int> xv;
// 		vector <int> yv;

// 		for (UInt_t i=0; i<	EventLines.size(); ++i) {
// 			if ((EventLines[i][0] == 'H') && (EventLines[i][1] == 'T')) {
// 				// Replace ; by Space
// 				for (UInt_t j=2; j<EventLines[i].size(); ++j) {
// 					if (EventLines[i][j]==';') EventLines[i][j]=' ';
// 				} 
// 				MString EventLine(EventLines[i].c_str());
// 				char chardummy[10];
// 				float CMM, val;
// 				int x,y;
	
// 				for (UInt_t j=0; j<DetectorList.size(); ++j) {
//  					if (EventLine.Contains(DetShortNames[j])) {
// 						sscanf(EventLines[i].c_str(), "%s %f %i %i %f", chardummy, &CMM, &x, &y, &val);
// 						if ( (x<0)||(x>9)||(y<0)||(y>11) ) {
// 							cout << "Pixel out of range: x=" << x << "; y=" << y << "!! Skipping..." << endl;
// 							continue;
// 						}
// 						if ( (-CMM)==val) continue; // there are lines with values of the negative common mode
// 						//DetNameWithSignal.push_back(DetShortNames[j]);
//  						DetPointers.push_back(DetectorList[j]);
// 						DetSignal.push_back(val);
// 						xv.push_back(x);
// 						yv.push_back(y);
// 					}
// 				}
//      }
//    }

// 		for (UInt_t j=0; j<DetSignal.size(); ++j) {
// 		  ((MCsIPixelSingle *) (DetPointers[j]))->GetCalibHist(xv[j], yv[j])->Fill(DetSignal[j]);
// 		}
					
// ******** END (Fill only largest hit in calib spectrum)

// ******** Filter only hits of detectors which have triggered
// 		vector <MString> TriggeredDetectors;
// 		vector <MDetector *> TriggeredDetPointers;
// 		vector <float> DetSignal;
// 		vector <MDetector *> DetPointers;
// 		vector <int> xv;
// 		vector <int> yv;

// 		// Check for triggered detectors	
// 		for (UInt_t i=0; i<	EventLines.size(); ++i) {
// 			if ((EventLines[i][0] == 'T') && (EventLines[i][1] == 'M')) {
// 				MString EventLine(EventLines[i].c_str());
// 				for (UInt_t j=0; j<DetectorList.size(); ++j) {
// 					if (EventLine.Contains(DetShortNames[j])) {
// 						TriggeredDetectors.push_back(DetShortNames[j]);
// 						TriggeredDetPointers.push_back(DetectorList[j]);
// 					}
// 				}
// 				break;
// 			}
// 		}

// 		// Get hits and fill calib histogram
// 		for (UInt_t i=0; i<	EventLines.size(); ++i) {
// 			if ((EventLines[i][0] == 'H') && (EventLines[i][1] == 'T')) {
// 				// Replace ; by Space
// 				for (UInt_t j=2; j<EventLines[i].size(); ++j) {
// 					if (EventLines[i][j]==';') EventLines[i][j]=' ';
// 				}

// 				MString EventLine(EventLines[i].c_str());
// 				char chardummy[10];
// 				float CMM, val;
// 				int x,y;
	
// 				for (UInt_t j=0; j<DetectorList.size(); ++j) {
//  					if (EventLine.Contains(DetShortNames[j])) {
// 						sscanf(EventLines[i].c_str(), "%s %f %i %i %f", chardummy, &CMM, &x, &y, &val);
// 						if ( (x<0)||(x>9)||(y<0)||(y>11) ) {
// 							cout << "Pixel out of range: x=" << x << "; y=" << y << "!! Skipping..." << endl;
// 							continue;
// 						}
// 						if ( (-CMM)==val) continue; // there are lines with values of the negative common mode
// 						for (UInt_t k=0; k<TriggeredDetectors.size(); ++k) {
// 							if (TriggeredDetectors[k]==DetShortNames[j]) {
// 								DetPointers.push_back(DetectorList[j]);
// 								DetSignal.push_back(val);
// 								xv.push_back(x);
// 								yv.push_back(y);
// 								break;
// 							}
// 						}
// 					}
// 				}
// 			}
// 		}

// 		for (UInt_t j=0; j<DetSignal.size(); ++j) {
// 			if (DetSignal[j] == 0) continue;
// 			for (UInt_t k=j+1; k<DetSignal.size(); ++k) {
// 				if (DetPointers[j] == DetPointers[k]) {
// 					if (DetSignal[j] <= DetSignal[k]) {
// 						DetSignal[j] = 0;
// 					} else {
// 						DetSignal[k] = 0;
// 					}
// 				}
// 			}
// 		}
		
// 		for (UInt_t j=0; j<DetSignal.size(); ++j) {
// 			if (DetSignal[j] == 0) continue;
// 			((MCsIPixelSingle *) (DetPointers[j]))->GetCalibHist(xv[j], yv[j])->Fill(DetSignal[j]);
// 		}

// ******** END (Filter out hits of detectors without trigger)


	} while (!DataFile.eof());

	// Save calibration and end
	Daq.SingleDaq();
	Daq.SaveCalibration("TestCalibration.calib");

	DataFile.close();
	cout << "Ende" << endl;
	return(0);
}

