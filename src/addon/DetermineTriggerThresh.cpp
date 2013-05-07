/*
 * DetermineTriggerThresh.cpp                              v1.0  21.08.2003
 *
 *
 * Copyright (C) by Robert Andritschke. All rights reserved.
 *
 * This code implementation is the intellectual property of Robert Andritschke.
 *
 * By copying, distributing or modifying the Program (or any work based on the 
 * Program) you indicate your acceptance of this statement, and all its terms.
 *
 */

/*******************************************************************************

Program for calculating trigger threshold from evta data file (D2 triggered).

********************************************************************************

Expects a directory name of a run
The program finds setupfile and data file by itself
The output is one file per chip for each detector (D1 and D2) including
the trigger thresholds in ADC counts.

Example:

  DetermineTriggerThresh ../calibfiles/20030415_161208_D2_Cs137_CalibRun

********************************************************************************/

#include <iostream>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <limits>
using namespace std;

// root classes
#include <MString.h>
#include <TH1.h>
#include <TH2.h>
#include <TDatime.h>
#include <TApplication.h>
#include <Getline.h>

// Megalyze classes
#include "MDaq.h"
#include "MCsIPixelSingle.h"
#include "MCsIPixelDouble.h"
#include "MSiStrip.h"


using namespace std;

enum stop {always, fail, never};
Int_t GetThreshold(TH1S * spectrum, stop wait=never);

int main(int argc, char **argv)
{
	TApplication* AppCalibration = new TApplication("TriggerThreshold", 0, 0);

	string InputLine = "";
	MString SetupFileName = "";
	string DataFileName = "";


	// first command line parsing...
	switch (argc) {
	case 0:
		cout << "Usage: DetermineTrigerThresh DirName" << endl;
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
	vector <MDetector *> Detector;
	vector <int> DetectorType; //1: Strip, 2: 2D Cal., 3: 3D Cal.
	vector <MString> DetName;
	vector <MString> DetShortName;
	vector <vector <MString> > ChipName;
	vector <vector <Bool_t> > HasTriggered;
	vector <vector <vector <float> > > val;
	vector <vector <vector <int> > > x;
	vector <vector <vector <int> > > y;
	//vector <vector <vector <int> > > CMM;
	vector < vector < vector <TH1S *> > > ExtraCalibHist;
	vector <Int_t> HistNum;
	Int_t n3D=0;

	TObjArray* AllDetectorList = Daq.GetDetectorList();
	for (int i = 0; i< AllDetectorList->GetEntriesFast(); ++i) {
		MDetector * Det = (MDetector *)AllDetectorList->At(i);
		
		// search for D2
		// single
		if ( ( Det->GetBelonging() == 2) && (Det->GetNChips()==1) ) {
			Detector.push_back(Det);
			DetName.push_back(Det->GetDetName());
			DetShortName.push_back(Det->GetShortName());
			DetectorType.push_back(2);
			HistNum.push_back(-1);
		}
		// double
		if ( ( Det->GetBelonging() == 2) && (Det->GetNChips()==2) ) {
			Detector.push_back(Det);
			DetName.push_back(Det->GetDetName());
			DetShortName.push_back(Det->GetShortName());
			DetectorType.push_back(3);
			HistNum.push_back(n3D);
			++n3D;
		}
		// search for D1
		if ( ( Det->GetBelonging() == 1) && (Det->GetNChips()==6) ) {
			Detector.push_back(Det);
			DetName.push_back(Det->GetDetName());
			DetShortName.push_back(Det->GetShortName());
			DetectorType.push_back(1);
			HistNum.push_back(-1);
		}
	}

	ChipName.resize(Detector.size());
	HasTriggered.resize(Detector.size());
	val.resize(Detector.size());
	x.resize(Detector.size());
	y.resize(Detector.size());
	//CMM.resize(Detector.size());

	// print found detectors
	cout << "Found detectors: " << endl;
	for (UInt_t i = 0; i<Detector.size(); ++i) {
		switch(DetectorType[i]) {
		case 1:
			cout << "Strip ";
			ChipName[i].push_back(DetShortName[i] + "4");
			ChipName[i].push_back(DetShortName[i] + "5");
			ChipName[i].push_back(DetShortName[i] + "6");
			HasTriggered[i].resize(3);
			val[i].resize(3);
			x[i].resize(3);
			y[i].resize(3);
			//CMM[i].resize(3);
			break;
		case 2:
			cout << "2D Cal";
			ChipName[i].push_back(DetShortName[i] + "1");
			HasTriggered[i].resize(1);
			val[i].resize(1);
			x[i].resize(1);
			y[i].resize(1);
			//CMM[i].resize(1);
			break;
		case 3:
			cout << "3D Cal";
			ChipName[i].push_back(DetShortName[i] + "1");
			ChipName[i].push_back(DetShortName[i] + "2");
			HasTriggered[i].resize(2);
			val[i].resize(2);
			x[i].resize(2);
			y[i].resize(2);
			//CMM[i].resize(2);
		}
		cout << "  " << DetName[i] << "  " << DetShortName[i] << endl;
	}

	// Allocate extra histogramms
	ExtraCalibHist.resize(n3D);
	for (UInt_t i = 0; i<ExtraCalibHist.size(); ++i) {
		// find chip name
		UInt_t k;
		for (k=0; k<HistNum.size(); ++k) {
			if (HistNum[k] == (signed) i) break;
		}

		ExtraCalibHist[i].resize(2);
		ExtraCalibHist[i][0].resize(120);
		ExtraCalibHist[i][1].resize(120);
		for (int xk=0; xk < 10; ++xk) {
			for (int yk=0; yk < 12; ++yk) {
				MString HistName(ChipName[k][0]);
				HistName+="_";
				HistName+=xk;
				HistName+="_";
				HistName+=yk;
				MString HistTitle(ChipName[k][0]);
				HistTitle+=" x=";
				HistTitle+=xk;
				HistTitle+=" y=";
				HistTitle+=yk;
				ExtraCalibHist[i][0][xk + 10*yk] = new TH1S(HistName, HistTitle, 800 , -0.5 , 799.5);
				HistName=ChipName[k][1];
				HistName+="_";
				HistName+=xk;
				HistName+="_";
				HistName+=yk;
				HistTitle=ChipName[k][1];
				HistTitle+=" x=";
				HistTitle+=xk;
				HistTitle+=" y=";
				HistTitle+=yk;
				ExtraCalibHist[i][1][xk + 10*yk] = new TH1S(HistName, HistTitle, 800 , -0.5 , 799.5);
			}
		} 
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


		// ******** Filter only hits of detectors which have triggered 
		// ******** and take only the largest signal of each detector

		// Initialize vectors
		for (UInt_t j=0; j<HasTriggered.size(); ++j) {
			for (UInt_t k=0; k<HasTriggered[j].size(); ++k) {
				HasTriggered[j][k]=kFALSE;
				val[j][k].resize(0);
				x[j][k].resize(0);
				y[j][k].resize(0);
				//CMM[j][k].resize(0);
			}
		}

		// Check for triggered detectors	
		for (UInt_t i=0; i < EventLines.size(); ++i) {
			if ((EventLines[i][0] == 'T') && (EventLines[i][1] == 'M')) {
				MString EventLine(EventLines[i].c_str());
				for (UInt_t j=0; j<Detector.size(); ++j) {
					for (UInt_t k=0; k<ChipName[j].size(); ++k) {
						if (EventLine.Contains(ChipName[j][k])) {
							HasTriggered[j][k]=kTRUE;
						}
					}
				}
			}
		}


 		// Get hits and fill calib histogram
		for (UInt_t i=0; i<	EventLines.size(); ++i) {
			if ((EventLines[i][0] == 'H') && (EventLines[i][1] == 'T')) {
				// Replace ; by Space
				for (UInt_t j=2; j<EventLines[i].size(); ++j) {
					if (EventLines[i][j]==';') EventLines[i][j]=' ';
				}

				MString EventLine(EventLines[i].c_str());
				string dummystring;
				float CMM[6], val0, val1;
				int xread,yread, nx, ny;
	
				for (UInt_t j=0; j<Detector.size(); ++j) {
 					if (EventLine.Contains(DetShortName[j])) {
						istringstream Line(EventLines[i]);

						switch(DetectorType[j]) {
						case 1:
							Line >> dummystring >> CMM[0] >> CMM[1] >> CMM[2] >> CMM[3] >> CMM[4] >> CMM[5] >> nx >> ny;

							for (Int_t nnx=0; nnx < nx; ++nnx) { // this is n-side
								Line >> xread >> val0;
							}

							for (Int_t nny=0; nny < ny; ++nny) { // this is p-side
								Line >> yread >> val0;
								if ( (yread<0) || (yread>=384) ) {
									cout << "D1 koordinate out of range: " << yread << "!" << endl;
									continue;
								}
								int chipnum = yread / 128;
								int channum = yread % 128;
								if (-val0!=CMM[chipnum]) { // throw away nonsense value
									y[j][chipnum].push_back(channum);
									x[j][chipnum].push_back(0);
									val[j][chipnum].push_back(val0);
								}
							}
							break;
						case 2:
							Line >> dummystring >> CMM[0] >> xread >> yread >> val0;
							
							if ( (xread<0)||(xread>9)||(yread<0)||(yread>11) )  {
								cout << "Pixel out of range: x=" << xread << "; y=" << yread << "!! Skipping..." << endl;
								continue;
							}
							if (-val0!=CMM[0]) { // throw away nonsense value
								y[j][0].push_back(yread);
								x[j][0].push_back(xread);
								val[j][0].push_back(val0);
							}
							break;
						case 3:
							Line >> dummystring >> CMM[0] >> CMM[1] >> xread >> yread >> val0 >> val1;
							
							if ( (xread<0)||(xread>9)||(yread<0)||(yread>11) )  {
								cout << "Pixel out of range: x=" << xread << "; y=" << yread << "!! Skipping..." << endl;
								continue;
							}
							if (-val0!=CMM[0]) { // throw away nonsense value
								y[j][0].push_back(yread);
								x[j][0].push_back(xread);
								val[j][0].push_back(val0);
							}
							if (-val1!=CMM[1]) { // throw away nonsense value
								y[j][1].push_back(yread);
								x[j][1].push_back(xread);
								val[j][1].push_back(val1);
							}
						}
					}
				}
			}
		}

// 		// filter largest signals of each detector
// 		for (UInt_t j=0; j<Detector.size(); ++j) { // loop over detectors
// 			for (UInt_t k=0; k<ChipName[j].size(); ++k) { // loop over chips
// 				if (y[j][k].size()==1) {
// 					switch(DetectorType[j]) {
// 					case 1:
// 						((MSiStrip *) (Detector[j]))->GetCalibHist(kTRUE, y[j][k][0]+1)->Fill(val[j][k][0]);
// 						break;
// 					case 2:
// 						((MCsIPixelSingle *) (Detector[j]))->GetCalibHist(x[j][k][0], y[j][k][0])->Fill(val[j][k][0]);
// 						break;
// 					case 3:
// 						ExtraCalibHist[HistNum[j]][k][x[j][k][0]+10*y[j][k][0]]->Fill(val[j][k][0]);
// 					}
// 				}
// 			}
// 		}

		// filter largest signals of each detector
		for (UInt_t j=0; j<Detector.size(); ++j) { // loop over detectors
			for (UInt_t k=0; k<ChipName[j].size(); ++k) { // loop over chips
				if (HasTriggered[j][k]) {
					float largestvalue=0.0;
					int valuey=0, valuex=0;
					for (UInt_t l=0; l<y[j][k].size(); ++l) { // loop over hits in chip
						if (val[j][k][l]>largestvalue) {
							largestvalue = val[j][k][l];
							valuey = y[j][k][l];
							valuex = x[j][k][l];
						}
					}
					
					if (largestvalue!=0.0) {
						switch(DetectorType[j]) {
						case 1:
							((MSiStrip *) (Detector[j]))->GetCalibHist(kTRUE, valuey+1)->Fill(largestvalue);
							break;
						case 2:
							((MCsIPixelSingle *) (Detector[j]))->GetCalibHist(valuex, valuey)->Fill(largestvalue);
							break;
						case 3:
							ExtraCalibHist[HistNum[j]][k][valuex+10*valuey]->Fill(largestvalue);
						}
					}
				}
			}
		}

		// ******** END (Filter)



	} while (!DataFile.eof());

	// Save calibration
	//	cout << "Saving calibration..."
	//	Daq.SingleDaq();
	//	Daq.SaveCalibration("TestCalibration.calib");
	//	cout << " done."
	DataFile.close();
	
	// Now determine triggerthreshold and write it to the output file:
	
	cout << "Writing trigger threshold data... ";
 	MString ThreshFileName = "";
	
	TCanvas canv;

	for (UInt_t j=0; j < Detector.size(); ++j) { // Loop over detectors
		for (UInt_t k=0; k < ChipName[j].size(); ++k) { // Loop over chips

			canv.SetTitle(ChipName[j][k].Data());

			// Generate ThreshFileName 
			ThreshFileName = argv[1];
			ThreshFileName += "/";
			ThreshFileName += ChipName[j][k];
			ThreshFileName += "_Thresholds.csv";
			
			ofstream ThreshFile(ThreshFileName.Data());
			if (!ThreshFile) {
				cout << endl << "Cannot open output file " << ThreshFileName 
						 << "! Exiting ..." << endl;
				exit(1);
			}
			
			// write header
			TDatime now;
			ThreshFile << "HE Type             cvs_csisingle" << endl
								 << "HE Version          1" << endl 
								 << "HE" << endl
								 << "HE ChipName         " << ChipName[j][k] << endl
								 << "HE Input file used: " << DataFileName << endl
								 << "HE Date             " << now.AsString() << endl 
								 << "HE" << endl;
			
			TH1S * spectrum;
			switch(DetectorType[j]) {
			case 1:
				for (Int_t y=1; y<=128; ++y) {
					spectrum = ((MSiStrip *)(Detector[j]))->GetCalibHist(kTRUE, y);
					if (spectrum == NULL) {
						cout << "Cannot find strip " << y << " of chip " << ChipName[j][k] << endl;
						continue;
					}
					ThreshFile << "TH " << y << " " << GetThreshold(spectrum, never) << endl;
				}
				break;
			case 2:
				for (Int_t x=1; x<=10; ++x) {
					for (Int_t y=1; y<=12; ++y) {
						spectrum = ((MCsIPixelSingle *)(Detector[j]))->GetCalibHist(x-1, y-1);
						if (spectrum == NULL) {
							cout << "Cannot find crystal x=" << x+1 << " y=" << y+1 
									 << " of chip " << ChipName[j][k] << endl;
							continue;
						}
						ThreshFile << "TH " << x << " " << y << " " << GetThreshold(spectrum,always) << endl;
					}
				}
				break;
			case 3:
				for (Int_t x=1; x<=10; ++x) {
					for (Int_t y=1; y<=12; ++y) {
						spectrum = ExtraCalibHist[HistNum[j]][k][(x-1)+10*(y-1)];
						ThreshFile << "TH " << x << " " << y << " " << GetThreshold(spectrum,always) << endl;
					}
				}
			}
			ThreshFile.close();
		}
	}
	cout << " done" << endl;

	cout << "Ende" << endl;
	return(0);
	AppCalibration->Run();
}


Int_t GetThreshold(TH1S * spectrum, stop wait) {
	// Determine threshold
	// Treshold is reached, if 1/10 of maximum reached and the spectrum is climbing
	Int_t TriggerThreshBin = 0;
	Int_t retval = -1; 
	Bool_t retvalok = kTRUE;

	if (spectrum->GetEntries() > 10) { // enough counts?
		for (TriggerThreshBin=2; TriggerThreshBin<spectrum->GetNbinsX()-1; ++TriggerThreshBin) {
			if (spectrum->GetBinContent(TriggerThreshBin) >= (spectrum->GetMaximum()/10.0)) {
				if ( (spectrum->GetBinContent(TriggerThreshBin-1) < spectrum->GetBinContent(TriggerThreshBin))
						 && (spectrum->GetBinContent(TriggerThreshBin) < spectrum->GetBinContent(TriggerThreshBin+1)) ) {
					break;
				}
			}
		}
		retval = (int) spectrum->GetBinCenter(TriggerThreshBin);

		// Determined threshold ok?
		Int_t NoiseThreshold = 0;
		for (NoiseThreshold=0; NoiseThreshold<spectrum->GetNbinsX()-1; ++NoiseThreshold) {
			if (spectrum->GetBinContent(NoiseThreshold) > 0) break;
		}
		if ((spectrum->GetBinCenter(TriggerThreshBin) < 10)||(TriggerThreshBin > NoiseThreshold+10)) {
			retvalok = kFALSE;
		}
	}

	cout << "Threshold value: " << retval << " which is ";
	if (retvalok) cout << "ok." << endl;
	else cout << "bad." << endl;

	switch (wait) {
	case never:
		break;
	case fail:
		if (retvalok) break;
	case always:
	default:
		spectrum->SetStats(kFALSE);
		spectrum->GetXaxis()->SetRangeUser(0,150);
		spectrum->Draw();
		
		// kurz anzeigen
		TCanvas *canv;
		if ( (canv = (TCanvas *)(gROOT->GetListOfCanvases()->At(0))) !=0) {
			canv->Update();
		} else {
			cout << "NO CANVAS!!" << endl;
		}

		// Now let's read the input
		char input[3];
		Bool_t cont=kFALSE;
		TTimer  *timer = new TTimer("gSystem->ProcessEvents();", 50, kFALSE);
	again:
		timer->TurnOn();
		timer->Reset();
		cout << "Type <return> to accept, \"q\" <return> to exit or \"m\" <return> to modify: " << endl;
		input[0]=0;
		cin.get(input,3);
		cin.clear();
		cin.ignore(numeric_limits<std::streamsize>::max(), '\n');
		timer->TurnOff();
		switch (input[0]) {
		case 0:
			break;
		case 'q':
			cout << "Exiting..." << endl;
			exit(1);
		case 'm':
			do {
				Bool_t cingood = kTRUE;
				cout << "Your choice of the Trigger Threshold: ";
				cin >> retval;
				cingood = cin.good();
				cin.clear();
				cin.ignore(numeric_limits<std::streamsize>::max(), '\n');
				if (cingood && ((retval>=0) && ((retval<spectrum->GetXaxis()->GetXmax())) || (retval == -1) )) {
					cout << "Using " << retval << " for Trigger Threshold. Ok? (\"n\" <return> for no, anything else for yes)" << endl;
					input[0]=0;
					cin.get(input,3);
					cin.clear();
					cin.ignore(numeric_limits<std::streamsize>::max(), '\n');
					if (input[0]=='n') continue;
					cont=kTRUE;
				}
			} while(!cont);
			break;
		default:
			cout << "What's \"" << input[0] << "\"? Try again." << endl;
			goto again;
 		}
	}
	
	return retval;
}
