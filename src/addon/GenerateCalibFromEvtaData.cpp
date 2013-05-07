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
#include <sstream>
#include <cstdlib>
#include <fstream>
#include <iostream>

// root classes
#include <MString.h>
#include <TH1.h>
#include <TH2.h>

// Megalyze classes
#include "MDaq.h"
#include "MSiStrip.h"
#include "MCsIPixelSingle.h"
#include "MCsIPixelDouble.h"


using namespace std;

int main(int argc, char **argv)
{

	MString SetupFileName="";
	string DataFileName="";


	// should be parsed or autorecognized later on:
	//int DetectorType = 1; // 1 for SiStrip, 2 for CsI (definitions like MDetector)

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
	MString DirEntry="";
	// check directory
	void * Directory = gSystem->OpenDirectory(argv[1]);
	if (Directory == 0) {
		cout << "Directory " << argv[1] << " not found. Exiting ..." << endl;
		exit(-1);
	}

	// find file names of setup and data files
	SetupFileName="";
	DataFileName="";
	while ((SetupFileName=="") || (DataFileName=="")) {
		DirEntry = gSystem->GetDirEntry(Directory);

		if (DirEntry.EndsWith(".setup")) {
			SetupFileName = argv[1];
			SetupFileName += "/";
			SetupFileName += DirEntry;
			cout << "Setupfile found: " << SetupFileName << endl;
			continue;
		}
	
		if (DirEntry.EndsWith(".uevta")) {
			DataFileName = argv[1];
			DataFileName += "/";
			DataFileName += DirEntry;
			cout << "Datafile found: " << DataFileName << endl;
			continue;
		}
	}

	// Close Dir list
	gSystem->FreeDirectory(Directory);
	Directory = 0;
		

	// opening Daq 
	MDaq Daq;
	Daq.Setup(SetupFileName);
	Daq.SetAnalysisMode(15);

// 	MChip* mychip = Daq->GetChip("bachus:Chip#4");
// 	if (myChip->IsTriggerOpen(68) cout << "Channel open" << endl;
// 			else cout << "Channel blocked" << endl;

	// Generate list of names and pointers to calib histograms
	vector <MDetector *> DetectorListD1;
	vector <MString> DetNamesD1;
	vector <MString> DetShortNamesD1;
	vector <MDetector *> DetectorListD2;
	vector <MString> DetNamesD2;
	vector <MString> DetShortNamesD2;
	vector <MDetector *> DetectorListD2D;
	vector <MString> DetNamesD2D;
	vector <MString> DetShortNamesD2D;

	TObjArray* AllDetectorList = Daq.GetDetectorList();
	for (int i = 0; i< AllDetectorList->GetEntriesFast(); ++i) {
		MDetector * Detector = (MDetector *)AllDetectorList->At(i);

		// collect detectors
		if ( Detector->GetDetectorType() == MDetector::e_SubD1) {
			DetectorListD1.push_back(Detector);
			DetNamesD1.push_back(Detector->GetDetName());
			DetShortNamesD1.push_back(Detector->GetShortName());
		}
		if ( Detector->GetDetectorType() == MDetector::e_D2Single) {
			DetectorListD2.push_back(Detector);
			DetNamesD2.push_back(Detector->GetDetName());
			DetShortNamesD2.push_back(Detector->GetShortName());
		}
		if ( Detector->GetDetectorType() == MDetector::e_D2Double) {
			DetectorListD2D.push_back(Detector);
			DetNamesD2D.push_back(Detector->GetDetName());
			DetShortNamesD2D.push_back(Detector->GetShortName());
		}
	
	}
	
	cout << "Found detectors D1: " << endl;
	for (UInt_t i = 0; i<DetectorListD1.size(); ++i) {
		cout << "  " << DetNamesD1[i] << "  " << DetShortNamesD1[i] << endl;
	}
	cout << "Found detectors D2Single: " << endl;
	for (UInt_t i = 0; i<DetectorListD2.size(); ++i) {
		cout << "  " << DetNamesD2[i] << "  " << DetShortNamesD2[i] << endl;
	}
	cout << "Found detectors D2Double: " << endl;
	for (UInt_t i = 0; i<DetectorListD2D.size(); ++i) {
		cout << "  " << DetNamesD2D[i] << "  " << DetShortNamesD2D[i] << endl;
	}

 

	// open input file
	string InputLine;
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


// ******** Fill only single D2 single hits in calib spectrum
// 		//vector <MString> DetNameWithSignal;
// 		vector <float> DetSignal;
// 		vector <MDetector *> DetPointers;
// 		vector <int> xv;
// 		vector <int> yv;
// 		Bool_t Triggered3D = kFALSE;

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
	
// 				for (UInt_t j=0; j<DetectorListD2.size(); ++j) {
//  					if (EventLine.Contains(DetShortNamesD2[j])) {
// 						sscanf(EventLines[i].c_str(), "%s %f %i %i %f", chardummy, &CMM, &x, &y, &val);
// 						if ( (x<0)||(x>9)||(y<0)||(y>11) ) {
// 							cout << "Pixel out of range: x=" << x << "; y=" << y << "!! Skipping..." << endl;
// 							continue;
// 						}
// 						if ( (-CMM)==val) continue; // there are lines with values of the negative common mode
// 						//DetNameWithSignal.push_back(DetShortNames[j]);
//  						DetPointers.push_back(DetectorListD2[j]);
// 						DetSignal.push_back(val);
// 						xv.push_back(x);
//  						yv.push_back(y);
// 					}
// 				}
// 				// Has a 3D a signal?
// 				for (UInt_t j=0; j<DetectorListD2D.size(); ++j) {
// 					if (EventLine.Contains(DetShortNamesD2D[j])) {
// 						Triggered3D=kTRUE;
// 					}
// 				}
// 			}
// 		}
	
		
// 		if ((DetPointers.size() == 1) && (Triggered3D==kFALSE)) {
// 			((MCsIPixelSingle *) (DetPointers[0]))->GetCalibHist(xv[0], yv[0])->Fill(DetSignal[0]);
// 		}

 		// ******** END (Fill only single D2 sinlge hit in calib spectrum)

		// ******** Fill only single D1 hits in calib spectrum
		vector <float> DetSignal;
		vector <MDetector *> DetPointers;
		vector <vector <int> > stripn;
		vector <vector <int> > stripp;
		vector <vector <float> > sign;
		vector <vector <float> > sigp;

		// delete non "HT" lines
		vector<string>::iterator it = EventLines.begin();
		while (it != EventLines.end()) {
			if (((*it)[0] == 'H') && ((*it)[1] == 'T')) {
				++it;
			} else {
				EventLines.erase(it);
			}
		}
		stripn.resize(EventLines.size());
		stripp.resize(EventLines.size());
		sign.resize(EventLines.size());
		sigp.resize(EventLines.size());
		DetPointers.resize(EventLines.size());
		for (UInt_t i=0; i<	EventLines.size(); ++i) {
			if ((EventLines[i][0] == 'H') && (EventLines[i][1] == 'T')) {
				// Replace ; by Space
				for (UInt_t j=2; j<EventLines[i].size(); ++j) {
					if (EventLines[i][j]==';') EventLines[i][j]=' ';
				} 
				MString EventLine(EventLines[i].c_str());
				char chardummy[10];
				float CMM0, CMM1, CMM2, CMM3, CMM4, CMM5, valn, valp;
				int nx, ny;
				int x,y;
				
				for (UInt_t j=0; j<DetectorListD1.size(); ++j) {
 					if (EventLine.Contains(DetShortNamesD1[j])) {
						stripn[i].resize(0);
						sign[i].resize(0);
						stripp[i].resize(0);
						sigp[i].resize(0);
						istringstream linestream(EventLines[i].c_str());
						linestream >> chardummy >> CMM0 >> CMM1 >> CMM2 >> CMM3 >> CMM4 >> CMM5 >> nx >> ny;
						for (Int_t k=0; k<nx; ++k) {
							linestream >> x >> valn;
							stripn[i].push_back(x);
							sign[i].push_back(valn);
						}
						for (Int_t k=0; k<ny; ++k) {
							linestream >> y >> valp;
							stripp[i].push_back(y);
							sigp[i].push_back(valp);
						}
 						DetPointers[i] = DetectorListD1[j];
					}
				}
			}
		}
	
		
		if (DetPointers.size() == 1) {
			if (stripn[0].size() == 1)
				((MSiStrip *) (DetPointers[0]))->GetCalibHist(kTRUE, stripn[0][0]+1)->Fill(sign[0][0]);
			if (stripp[0].size() == 1)
				((MSiStrip *) (DetPointers[0]))->GetCalibHist(kFALSE, stripp[0][0]+1)->Fill(sigp[0][0]);
		}

 		// ******** END (Fill only single D1 hit in calib spectrum)

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

