/*
 * CalibrateCsISingle.cpp                                 v7.0  29/07/2003
 *
 *
 * Copyright (C) by Robert Andritschke and Andreas Zoglauer and Florian Schopper. 
 * All rights reserved.
 *
 * This code implementation is the intellectual property of Robert Andritschke, Florian Schopper, Andreas Zoglauer 
 * at MPE.
 *
 * By copying, distributing or modifying the Program (or any work based on the 
 * Program) you indicate your acceptance of this statement, and all its terms.
 *
 */

/******************************************************************************

Program for fitting the calibration histograms of the MEGA 2-D calorimeters.

*******************************************************************************

To-Do-List:

+ Integrate Pulse-Shape correction

*******************************************************************************

launch CalibrateCsISingle -d circe -c data/20021015_17:12_Cs137_allD2_side2.calib 662 -c data/20021021_10\:25_Na22_D2.calib 511 -c data/20021021_10\:25_Na22_D2.calib 1274 -e

*/  

// Root libs
#include <TROOT.h>
#include <MString.h>
#include <TCanvas.h>
#include <TF1.h>
#include <TStyle.h>
#include <TSystem.h>
#include <TH1.h>
#include <TH2.h>
#include <TTimer.h>
//#include <Getline.h>
#include <TGraphErrors.h>
#include <TApplication.h>
#include <TLine.h>

// Mega libs
#include "MCsIPixelSingle.h"

// Standard libs:
#include <sstream>
#include <iostream>
using std::cout;
using std::endl;
using std::ofstream;
#include <vector>
using std::vector;
#include <iomanip>
#include <cstdlib>
#include <limits>
using namespace std;

/*****************************************************************************/

Bool_t CalibrateCsISingle(MString DetectorName,
													vector<MString> FileName,
													vector<Float_t> Energy,
													vector<Float_t> Energy2,
													MString Prefix,
													Int_t Stop,
													Bool_t OnlyPhoto,
													Bool_t Fix,
													Bool_t Release,
													Int_t xStart, 
													Int_t yStart,
													MString PulseShapeFile,
													Int_t PulseShapeTiming,
													MString TriggerThreshFile,
													MString InLogFileName);
void FindPeakBorders(TH1S* Hist, Double_t& LowerBorder, Double_t& UpperBorder);
Bool_t FitPeak(TH1S* Hist, TF1* FitFunc, Double_t& Mean, Double_t& Sigma, 
							 Double_t& Height, Double_t& LowerBorder, Double_t& UpperBorder);
Double_t FitAllLines(Double_t * ADCvalue, Double_t * par);
Double_t FitAllLines2(Double_t * ADCvalue, Double_t * par);
Double_t Fit1Line(Double_t * x, Double_t * par);
Double_t Fit2Lines(Double_t * ADCvalue, Double_t * par);
Double_t ScatterFunction(Double_t xo, Double_t xi);
Double_t ScatteredPhotoFunction(Double_t xo, Double_t xi);
Double_t ComptonFunction(Double_t xo, Double_t xi);
Double_t Gauss(Double_t x, Double_t x0, Double_t s);
Double_t Integrand(Double_t *x, Double_t *par);
Double_t ScatteredComptonFunction(Double_t xo, Double_t xi);
Double_t ScatteredComptonFunction1(Double_t xo, Double_t xi);
Double_t ScatteredComptonFunction2(Double_t xo, Double_t xi);
Double_t ConvF(Double_t *par);
Double_t ConvF2(Double_t *par);
//Double_t DoubleComptonIntegrand(Double_t *x, Double_t *par);
//Double_t DoubleComptonFunction(Double_t xo, Double_t xi);
Double_t OverflowAndBackground(Double_t * ADCvalue, Double_t * par);

/*****************************************************************************/


int main(int argc, char** argv) 
{
	// Returns 1 in case of an syntax error, else 0
	Int_t Stop = 0;  // 0 = no stop (default)
	//                  1 = stop on critical error (bad fits)
	//                  2 = stop also on bad eta
	//                  4 = stop always
	Bool_t OnlyPhoto = kFALSE; // do only prelim. photopeak fit
	Bool_t Fix = kFALSE;
	Bool_t Release = kFALSE;
	Int_t xStart = 1, yStart = 1;
	vector<MString> FileName;
	vector<Float_t> Energy;
	vector<Float_t> Energy2;
	MString DetectorName;
	MString Prefix = "";
	MString PulseShapeFile = "";
	MString Logfile = "";
	int PulseShapeTiming;
	MString TriggerThreshFile;

	TApplication* AppCalibration = new TApplication("Calibration", 0, 0);

  MStr Usage;
  Usage << endl
				<< "  Usage: " << argv[0] << " <options>" << endl << endl
				<< "     mandatory options:" << endl
				<< "       -d <detector name>      DetectorName, e.g. -d circe "
			  << "(make sure it's a single sided CsI detector)" << endl
				<< "       -c <file name> <energy> Name of calib file AND energy, "
				<< "e.g. 20021015_17:12_Cs137_allD2_side2.calib 662" << endl
				<< "       -cc <file name> <energy1> <energy2> Name of calib file "
				<< "and TWO energies, e.g. 20021015_17:12_Na22_all.calib 511 1274" 
				<< endl << endl
				<< "     other options:" << endl
				<< "       -s <file name> <timing> File containing the optional pulse "
				<< "shape corrections and the timing" << endl
				<< "       -t <file name>          Trigger threshold file" << endl
				<< "       -e                      Stop on critical Errors" << endl
				<< "       -ee                     Stop on all Errors" << endl
				<< "       -eee                    Stop after each fit" << endl
				<< "       -1                      "
				<< "Do only first stage: prelim. photo peak fit" << endl
				<< "       -2                      "
				<< "Do until second stage: Fix prelim. fit values" << endl
				<< "       -2r                     "
				<< "Do again second stage, but release all parameters" << endl
				<< "       -b <xStart> <yStart>    Pixel at which to start" << endl
				<< "       -p <Prefix>             Outputfiles named "
				<< "<Prefix><DetectorName>_calibrate.csv" << endl
				<< "       -l <FileName>           Logfile to check a previous run" 
				<< endl
				<< "       -h                      "
				<< "you probably already have guessed..." << endl;


	// Command line parsing
  MString Option;

  // Check for help
  for (Int_t i = 1; i < argc; i++) {
    Option = argv[i];
    if (Option == "-h" || Option == "--help" || 
				Option == "?" || Option == "-?") {
      cout<<Usage<<endl;
      return 1;
    }
  }

  // Now parse the command line options:
	// First check if each option has sufficient arguments:
  for (Int_t i = 1; i < argc; i++) {
		Option = argv[i];

		// Single argument
    if (Option == "-d" ||
				Option == "-p" ||
				Option == "-l" ) {
			if (!((argc > i+1) && argv[i+1][0] != '-')){
				cout << "Error: Option " << argv[i][1] << " needs a second argument!" 
						 << endl;
				cout << Usage << endl;
				return 1;
			}
		} 
		// Double arguments:
		else if (Option == "-b" || Option == "-c" || Option == "-s") {
			if (!((argc > i+2) && argv[i+1][0] != '-' && argv[i+2][0] != '-')){
				cout<<"Error: Option "<<argv[i][1]<<" needs two arguments!"<<endl;
				cout<<Usage<<endl;
				return 1;
			}
		}
	}

	// Now retrieve the parameters and values:
  for (Int_t i = 1; i < argc; i++) {
		Option = argv[i];

		if (Option == "-d") {
			DetectorName = argv[++i];
			cout<<"Detector name: "<<DetectorName<<endl;
		} else if (Option == "-c") {
			FileName.push_back(argv[++i]);
			Energy.push_back(atoi(argv[++i]));
			Energy2.push_back(0.0);
			cout << "Calibration file " << FileName.back() << " at " << Energy.back()
					 << " keV" << endl;
		} else if (Option == "-cc") {
			FileName.push_back(argv[++i]);
			Energy.push_back(atoi(argv[++i]));
			Energy2.push_back(atoi(argv[++i]));
			cout << "Calibration file " << FileName.back() << " at " << Energy.back()
					 << " keV and " << Energy2.back() << endl;
		} else if (Option == "-s") {
			PulseShapeFile = argv[++i];
			PulseShapeTiming = atoi(argv[++i]);
			cout << "Pulse shape file " << PulseShapeFile << " at " 
					 << PulseShapeTiming << " keV" << endl;
		} else if (Option == "-t") {
			TriggerThreshFile = argv[++i];
			cout << "Trigger threshold file " << TriggerThreshFile << endl;
		} else if (Option == "-l") {
			Logfile = argv[++i];
			cout << "Reading from log file " << Logfile << endl;
		} else if (Option == "-e") {
			Stop = 1;
			cout << "Stop on critical errors!" << endl;
		} else if (Option == "-ee") {
			Stop = 2;
			cout << "Stop on all errors!" << endl;
		} else if (Option == "-eee") {
			Stop = 4;
			cout << "Stop after each pixel!" << endl;
		} else if (Option == "-1") {
			OnlyPhoto = kTRUE;
			cout << "Do only prelim. photo peak fit!" << endl;
		} else if (Option == "-2") {
			Fix = kTRUE;
			cout << "Fix photopeak parameters!" << endl;
		} else if (Option == "-2r") {
			Release = kTRUE;
			cout << "Release all parameters!" << endl;
		} else if (Option == "-b") {
			xStart = atoi(argv[++i]);
			yStart = atoi(argv[++i]);
			if (xStart < 1 || xStart > 10 || yStart < 1 || yStart > 12) {
				cout << endl
						 << "Valid ranges for the start pixel: x=[1..10], y=[1..12]" 
						 << endl << endl << "Your choice: x=" << xStart << ", y=" 
						 << yStart << "  -> Exit!" << endl;
				return 1;
			} else {
				cout << "Start pixel: x=" << xStart << " y=" << yStart << endl;
			}
		} else if (Option == "-p") {
			Prefix = argv[++i];
			cout << "Using prefix: " << Prefix << endl;
		}
	}

	if (Energy.size() == 0) {
		cout << "You have to give at least one calibration file to work with "
				 << "-> Exit" << endl << Usage << endl;
		return 1;
	}

	if (DetectorName == "") {
		cout << "You have to give a detector name!" << endl << Usage << endl;
		return 1;
	}

	// Print parsed values
	cout << "DetectorName = \"" << DetectorName << "\"" << endl
			 << "xStart = \"" << xStart << "\"" << endl
			 << "yStart = \"" << yStart << "\"" << endl;
	switch (Stop) {
	case 0:
		cout << "Run through fitting without stop" << endl;
		break;
	case 1:
		cout << "Stop when fitting impossible" << endl;
		break;
	case 2:
		cout << "Stop on bad fits" << endl;
	case 4:
		cout << "Stop after each line" << endl;
		break;
	default:
		cout << "What's that, Stop = " << Stop << "?" << endl;
	}
	cout << "Prefix = \"" << Prefix << "\"" << endl;
	for (UInt_t i=0; i<FileName.size(); ++i) {
		cout << "Fit line at E=" << Energy[i] << "keV in file \"" << FileName[i] 
				 << "\"" << endl;
	}

  // now do the fitting
	if (CalibrateCsISingle(DetectorName,
												 FileName,  
												 Energy,
												 Energy2, 
												 Prefix, 
												 Stop,
												 OnlyPhoto,
												 Fix,
												 Release,
												 xStart, 
												 yStart,
												 PulseShapeFile,
												 PulseShapeTiming,
												 TriggerThreshFile,
												 Logfile)) {
		return 0;
	} else {
		cout << Usage << endl;
		return 1;
	}
	AppCalibration->Run(); // only that compiler does not warn ...
}



// Global Constants

// Minimum number of counts a spectrum needs to have
const Int_t MinCounts = 500;

// Global variables
Double_t xin;  // Incoming energy in electron masses (e.g. 662/511)
Double_t xin1;
Double_t xin2;

// Fitting routine, the actual work

Bool_t CalibrateCsISingle(MString DetectorName,
													vector<MString> FileName,
													vector<Float_t> Energy,
													vector<Float_t> Energy2,
													MString Prefix,
													Int_t Stop,
													Bool_t OnlyPhoto,
													Bool_t FixPhotoPeak,
													Bool_t ReleaseParameters,
													Int_t xStart, 
													Int_t yStart,
													MString PulseShapeFile,
													Int_t PulseShapeTiming,
													MString TriggerThreshFile,
													MString InLogFileName)
{

	// Calibrate the detector called DetectorName at energies found 
	// in the vector Energy. For each energy there is a .calib or .house 
	// file containing the calibration spectra. These file names
	// are in the vector FileName.
	// The output file is called Prefix+DetectorName+"_calibrate.csv".
	// The fitting starts at the crystal xStart;yStart.
	// Returns kFALSE if files are not found, else kTRUE.

	// --------------------------------------------------------------------------
	// Initialisations
	// --------------------------------------------------------------------------

	string InputLine;

  // Average sigma (average energy resolution for simulation)
  double AvgSigma = 0.0;
  int NAvgSigma = 0;
  
	ifstream lin;
	// Open input log file if available
	if (InLogFileName!="") {
		cout << "Opening log file \"" << InLogFileName << "\" for input" << endl; 
		lin.open(InLogFileName.Data());
		if (!lin.is_open()) {
			cout << "Cannot open input log file! Exiting ..." << endl;
			return(1);
		}
	}

	// Open output file to store results
	cout << "Opening file to store results: \"";
	MString OutputFileName(Prefix);
  OutputFileName.Append(DetectorName);
	if(Energy[0] > 150.0)  OutputFileName.Append("_calibrate.csv"); 
	else OutputFileName.Append("_diodes.csv");
  cout << OutputFileName.Data() << "\"" << endl;
	// File already existing?
	FILE *File;
	MString text;
  if (NULL != (File = fopen(OutputFileName.Data(), "r"))) {
		fclose(File);
		File = NULL;
		cout << "File \"" << OutputFileName 
				 << "\" already exists. ";
		for (;text.Data()[0]!='o';) {
			cout << "Overwrite or exit (o/e)?" << endl;
			cin >> text;
			cin.clear();
			cin.ignore(numeric_limits<std::streamsize>::max(), '\n');
			if(text.Data()[0]=='e') {
				cout << "Exiting ..." << endl;
				return 1;
			}
		}
	}
  ofstream fout(OutputFileName.Data());
	if (!fout) {
		cout << "Cannot open file \"" << OutputFileName 
				 << "\" for output! Exiting ..." << endl;
		return 1;
	}

	// Store header in output file 
	cout << "writing header to outputfile" << endl;
	fout << "Type          csv_csisingle" << endl
			 << "Version       4" << endl << endl
			 << "DetectorName  " << DetectorName.Data() << endl;
	for (UInt_t i=0; i<FileName.size(); ++i) {
		fout << "CalibFileName " << FileName[i] << endl;
	}
	MTime now;
	now.Now();
	fout << "Date          " << now.GetSQLString() << endl << endl;

	// Open log file. Filename is OutputFileName.log
	MString LogFileName(OutputFileName);
	LogFileName.Append(".log");
	cout << "Logging information to " << LogFileName.Data() << endl;
	// File already existing?
	//FILE *File;
	//MString text;
	UInt_t NumberOfLogFiles;
  if (NULL != (File = fopen( LogFileName.Data(), "r"))) {
		fclose(File);
		for (NumberOfLogFiles=1; NumberOfLogFiles<10; ++NumberOfLogFiles) {
			text = LogFileName;
			text += NumberOfLogFiles;
			if(NULL != (File = fopen(text.Data(),"r"))) {
				fclose(File);
			} else {
				break;
			}
		}
		if (NumberOfLogFiles >= 9) {
			cout << "There are already 10 log files! Exiting..." << endl;
			return 1;
		} else {
			LogFileName += NumberOfLogFiles;
		}
	}
	ofstream lout(LogFileName.Data());
	if (!lout) {
		cout << "Cannot open \"" << LogFileName << "\" for output! Exiting ..." 
				 << endl;
		return 1;
	}

	// Initialize log file
	lout << "HE Log File for Detector " << DetectorName << endl;
	lout << "HE Generated with .calib Files: " << endl;
	for (UInt_t i=0; i< FileName.size(); ++i) {
		lout << "HE  " << FileName[i] << endl;
	}
	lout << "HE Corresponding .csv File is " << OutputFileName << endl;

	// Open trigger threshold file
	Int_t TThresholds[10][12];
	for (int x=0; x<10; ++x) {
		for (int y=0; y<12; ++y) {
			TThresholds[x][y]=-1;
		}
	}
	cout << "Opening File containing trigger thresholds" << endl;
	ifstream thresh(TriggerThreshFile.Data());
	if (!thresh) {
		cout << "\e[43mTrigger threshold file not found \e[0m(\"" 
				 << TriggerThreshFile << "\")! Try to guess while fitting." << endl;
		lout << "HE Opening Threshold file \""<< TriggerThreshFile 
				 << "\" failed!" << endl;
	} else {
		cout << "Reading thresholds" << endl;
		// skipping header
		int x,y,t;
		string token;
		getline(thresh, InputLine);
		do {
			istringstream ParseLine(InputLine);
			ParseLine >> token;
			if (token == "TH") {
				ParseLine >> x >> y >> t;
				//cout << "x " << x << " y " << y << " t " << t << endl;
				if ( (x<1)||(x>10)||(y<1)||(y>12) ) {
					cout << "Pixel out of range: x=" << x << "; y=" << y 
							 << "!! Skipping..." << endl;
					lout << "HE Pixel out of range: x=" << x << "; y=" << y 
							 << "!! Skipping..." << endl;

				} else {
					TThresholds[x-1][y-1] = t;
				}
			}
			getline(thresh, InputLine);
		} while (!thresh.eof());
		lout << "HE Using Threshold file \"" << TriggerThreshFile 
				 << "\"." << endl;
	}

// 	for (int x=0; x<10; ++x) {
// 		for (int y=0; y<12; ++y) {
// 			cout << "x " << x << " y " << y << " t " << TThresholds[x][y] << endl;
// 		}
// 	}

	// Open files containing calibration histogramms
	vector<TFile *> Files;
	Files.resize(FileName.size());
	cout << endl << "Opening files... " << endl << "  ";
	for (UInt_t i=0; i<FileName.size(); ++i) {  // Loop over lines/files
		cout << FileName[i] << ", ";
		Files[i] = new TFile(FileName[i], "READ");
		if (!(Files[i]->IsOpen())) {
			cout << endl << "Cannot open file \"" << FileName[i] << "\"!" << endl;
			lout << endl << "HE Cannot open file \"" << FileName[i] << "\"!" << endl;
			return kFALSE;
		}
	}
	cout << endl;

	// Get Calibration histograms out of .calib and .house files
	vector<MCsIPixelSingle *> Detectors;
	Detectors.resize(Files.size());
	vector<MChip *> Chip;
	Chip.resize(Files.size());

	MHouseData* HK = 0;
	for (UInt_t i=0; i<Files.size(); ++i) { // Loop over all files
		Detectors[i] = NULL;
		HK = ((MHouseData*) Files[i]->Get("Housekeeping"));
		if (HK != 0) {
			cout << "Reading calibration histograms from housekeeping file \"" 
					 << FileName[i] << "\"" << endl;
			for (int d=0; d<HK->GetNDetectors(); ++d) {
				TObject* StillUnknown = HK->GetDetectorAt(d);
				if (StillUnknown == NULL) {
					cout << "\e[43mDetector[" << d << "] not found!!\e[0m" << endl;
					lout << "HE Detector[" << d << "] not found!!" << endl;
					continue;
				}

				if (MString(StillUnknown->ClassName()) != "MCsIPixelSingle") {
					continue;
				} else {
					Detectors[i] = (MCsIPixelSingle*) StillUnknown;
			
					if (DetectorName.CompareTo(Detectors[i]->GetName()) == 0) {
						break;
					} else {
						Detectors[i] = NULL;
					}
				}
			}
			if (Detectors[i] == NULL) {
				cout << "There is no detector called " << DetectorName 
						 << " of type MCsIPixelSingle! Exiting..." << endl;
				lout << "HE There is no detector called " << DetectorName 
						 << " of type MCsIPixelSingle! Exiting..." << endl;
				return false;
			} else {
				if ((Chip[i] = HK->GetChip(DetectorName + ":Chip#1")) == NULL) {
					cout << "Detector without Chip?!? (" << DetectorName + ":Chip#1" 
							 << ")" << endl;
					lout << "HE Detector without Chip?!? (" << DetectorName + ":Chip#1"
							 << ")" << endl;
					return false;
				}
			}

		} else {
			cout << "Reading calibration histograms from file \"" << FileName[i] 
					 << "\"" << endl;
			lout << "HE Reading calibration histograms from file \"" << FileName[i] 
					 << "\"" << endl;
			TObject* StillUnknown = Files[i]->Get(DetectorName.Data());
			if (StillUnknown == NULL) {
				cout << "No detector called " << DetectorName << " available in file "
						 << FileName[i] << "! Exiting..." << endl;
				lout << "HE No detector called " << DetectorName 
						 << " available in file " << FileName[i] << "! Exiting..." << endl;
				return false;
			}
			if (MString(StillUnknown->ClassName()) != "MCsIPixelSingle") {
				cout << "No, no, no, no.... "
						 << "This programm is for calibrating MCsIPixelSingle" << endl
						 << "and not for " << StillUnknown->ClassName()
						 << " (" << DetectorName << ")... Aborting!" << endl;
				lout << "HE Wrong detector type:" << endl
						 << "HE This programm is for calibrating MCsIPixelSingle" << endl
						 << "HE and not for " << StillUnknown->ClassName()
						 << " (" << DetectorName << ")... Aborting!" << endl;

				return false;
			} else {
				Detectors[i] = (MCsIPixelSingle *) StillUnknown;

				if ((Chip[i] = (MChip*) Files[i]->Get(DetectorName + 
																							":Chip#1")) == NULL){
					cout << "No chip for detector " << DetectorName 
							 << " found. Exiting..." << endl;
					lout << "Detector has no chip?!? (" << DetectorName + ":Chip#1)"
							 << endl;
					return false;
				}
			}
		}

		cout << endl << "Detector \"" << DetectorName 
				 << "\" (incl. chip) found in file \"" << FileName[i] << "\"" << endl;
	}
        
	// Initialize timer
	TTimer  *timer = new TTimer("gSystem->ProcessEvents();", 50, kFALSE);

	// Initialize graphics
	
	cout << "Initializing graphics" << endl;

	// Crystal Windows
	vector <TCanvas *> CalCanvas;
	CalCanvas.resize(Energy.size());
	for (UInt_t i=0; i<CalCanvas.size(); ++i) {
		MString CanvName("calibration");
		MString CanvTitle("Spectrum");
		CanvName += i;
		CanvTitle += i;
		CalCanvas[i] = new TCanvas(CanvName, CanvTitle, 500*i, 0, 500, 500);
	}

	// Histogram and canvas for overall fit
	TH1S * allcrystal = new TH1S("All", "All", 2000, -0.5, 1999.5);
	TCanvas * testcanv = new TCanvas("test","test", 0, 0, 500, 500);

 	// Linear Window
	TCanvas* LinFitCanvas =  
		new TCanvas("Linear Fit Progress", "Linear Fit Progress", 
								500, 550, 438,330);
	TGraph* LinFit = 0;

	// CorrectionWindow:
// 	TCanvas* CorrFitCanvas =  
// 		new TCanvas("Correction Fit Progress", "Correction Fit Progress", 
// 								500, 350, 438, 330);
// 	TGraph* CorrFit = 0;


	// fit functions
	cout << "Initializing fit functions" << endl;

	// Gaus-fit for photopeak
	vector <TF1*> EnergyFit;
	EnergyFit.resize( Energy.size() ); 
	for (UInt_t i=0; i< Energy.size(); ++i) {
		MString FitName("Energy");
		FitName += i;
		EnergyFit[i] = new TF1(FitName.Data(), "gaus");
	}

	// Fitfunction for whole 1-line spectrum
	TF1* Fit1 = new TF1("Fit1", Fit1Line, 0, 1000, 8);
	Fit1->SetParNames("PhotopeakPosition", 
										"Sigma", 
										"HeightOfPhotopeak",
										"HeightOfComptonEdge", 
										"HeightOfScatterAbsorbed",
										"HeightOfScatterCompton", 
										"Offset",
										"Background");
										//	"HeightOfNoisePeak", 
										//	"HeightOfDoubleCompton");

	// Fitfunction for whole 2-line spectrum
	TF1* Fit2 = new TF1("Fit2", Fit2Lines, 0, 1000, 12);
	Fit2->SetParNames(   "PhotopeakPosition1", 
											 "Sigma",
											 "HeightOfPhotopeak1",
											 "HeightOfPhotopeak2",
											 "HeightOfComptonEdge1",
											 "HeightOfComptonEdge2",
											 "HeightOfScatterAbsorbed1",
											 "HeightOfScatterAbsorbed2",
											 "HeightOfScatterCompton1",
											 "HeightOfScatterCompton2",
											 "Offset");
	Fit2->SetParName(11, "Background");

	// Fitfunction for both spectra together
	TF1* FitAll = new TF1("FitAll", FitAllLines, 0, 2000, 19);
	FitAll->SetParNames("PhotopeakPosition",
											"Sigma",
											"HeightOfPhotopeak",
											"HeightOfPhotopeak1",
											"HeightOfPhotopeak2",
											"HeightOfComptonEdge",
											"HeightOfComptonEdge1",
											"HeightOfComptonEdge2",
											"HeightOfScatterAbsorbed",
											"HeightOfScatterAbsorbed1",
											"HeightOfScatterAbsorbed2");
	FitAll->SetParName(11, "HeightOfScatterCompton");
	FitAll->SetParName(12, "HeightOfScatterCompton1");
	FitAll->SetParName(13, "HeightOfScatterCompton2");
	FitAll->SetParName(14, "Offset");
	FitAll->SetParName(15, "Background");
	FitAll->SetParName(16, "Background1");
	FitAll->SetParName(17, "Range1High");
	FitAll->SetParName(18, "Range2Low");

	// Fitfunction for both spectra together,but 2 Offsets
	TF1* FitAll2 = new TF1("FitAll2", FitAllLines2, 0, 2000, 20);
	FitAll2->SetParNames("PhotopeakPosition",
											"Sigma",
											"HeightOfPhotopeak",
											"HeightOfPhotopeak1",
											"HeightOfPhotopeak2",
											"HeightOfComptonEdge",
											"HeightOfComptonEdge1",
											"HeightOfComptonEdge2",
											"HeightOfScatterAbsorbed",
											"HeightOfScatterAbsorbed1",
											"HeightOfScatterAbsorbed2");
	FitAll2->SetParName(11, "HeightOfScatterCompton");
	FitAll2->SetParName(12, "HeightOfScatterCompton1");
	FitAll2->SetParName(13, "HeightOfScatterCompton2");
	FitAll2->SetParName(14, "Offset");
	FitAll2->SetParName(15, "Offset2");
	FitAll2->SetParName(16, "Background");
	FitAll2->SetParName(17, "Background1");
	FitAll2->SetParName(18, "Range1High");
	FitAll2->SetParName(19, "Range2Low");

	// Linear Fit as energy interpolation between lines
  TF1* LinearFit = new TF1("LinearFit", "pol1");

	// Step function + Gauss for background + overflow
	vector <TF1*> BckgOv;
	BckgOv.resize(Energy.size() );
	for (UInt_t i=0; i< Energy.size(); ++i) {
		MString FitName("Background");
		FitName += i;
		BckgOv[i] = new TF1("BackgroundAndOverflow", OverflowAndBackground, 
												0, 1000, 4);
		BckgOv[i]->SetParNames("BackgroundHeight",
													 "OverflowHeight",
													 "OverflowPosition",
													 "Sigma");
	}

	// Line indicating trigger threshold	
	vector <TLine *> ThreshLine;
	ThreshLine.resize(Energy.size());
	for (UInt_t i=0; i< Energy.size(); ++i) {
		ThreshLine[i] = new TLine();
	}

	// Check, if first line is a single line spectrum:
	if (Energy2.size()>= 1) {
		if (Energy2[0]>0.0) {
			cout << "Please give me as first spectrum a single line spectrum!" 
					 << endl;
			return(false);
		}
		if (Energy2[1]==0.0) {
			cout << "Please give me as second spectrum a double line spectrum!" 
					 << endl;
			return(false);
		}
	}


	// --------------------------------------------------------------------------
	// Main calibration loop:
	// --------------------------------------------------------------------------


	Bool_t Exit = kFALSE;  // Set this to escape from x & y loops
	Bool_t FirstStageAgain = kFALSE; // Set this to go back to first stage
	Bool_t NextCrystal = kFALSE; // Set this to proceed with next pixel

	cout << endl << "Starting Fits..." << endl; 

	// Loop over all pixels
	for (Int_t x=xStart; x <= 10; ++x) {     // Loop over x pixels
		if (x != xStart) yStart=1;
		for (Int_t y=yStart; y <= 12; ++y) {   // Loop over y pixels

			lout << "PI " << x << " " << y << endl;
			cout << endl << endl;
			cout << "\e[44mFitting pixel (x=" << x << ", y=" << y << ")\e[0m" 
					 << endl;

			FirstStageAgain = kFALSE;
			NextCrystal = kFALSE;

			// **** Variables: ********

			//Vectors: One Element per fitted line
	
			Stat_t AllInt;        // Sum of counts of histogram crystal
			
			TH1S *crystal;    // Histogram containing data for fitting
			
			vector <Double_t> PrelimEnergyFitMean;
			vector <Double_t> PrelimEnergyFitSigma;
			vector <Double_t> PrelimEnergyFitHeight;
			vector <Double_t> PrelimChiSquare;
			Double_t PrelimOffset;
			vector <bool> BadCounts; // not enough counts in spectrum
			vector <bool> BadEFit;   // >= 1 bad energy fit occurred
			vector <bool> BadSFit;   // >= 1 bad spectrum fit occurred
			vector <Double_t> Overflow;       // Overflow vectors
			vector <Double_t> OverflowHeight; // OverflowHeight vector
			vector <Double_t> BackHeight;     // Background height
			vector <Double_t> GuessedLowerEnergy; // guess for line borders
			vector <Double_t> GuessedUpperEnergy; // 
			vector <Double_t> EnergyFitMean;
			vector <Double_t> EnergyFitSigma;
			vector <Double_t> EnergyFitHeight;
			vector <Double_t> EnergyFitHeight2;
			vector <Double_t> EnergyFitHeightCompEdge;
			vector <Double_t> EnergyFitHeightCompEdge2;
			vector <Double_t> EnergyFitHeightScatAbs;
			vector <Double_t> EnergyFitHeightScatAbs2;
			vector <Double_t> EnergyFitHeightScatComp;
			vector <Double_t> EnergyFitHeightScatComp2;
			vector <Double_t> EnergyFitOffset;
			vector <Double_t> EnergyFitChiSquare;
			vector <Double_t> RangeLow;
			vector <Double_t> RangeHigh;
			Double_t FinalEnergyFitMean;
			Double_t FinalEnergyFitSigma;
			Double_t FinalEnergyFitHeight;
			Double_t FinalEnergyFitHeight1;
			Double_t FinalEnergyFitHeight2;
			Double_t FinalEnergyFitHeightCompEdge;
			Double_t FinalEnergyFitHeightCompEdge1;
			Double_t FinalEnergyFitHeightCompEdge2;
			Double_t FinalEnergyFitHeightScatAbs;
			Double_t FinalEnergyFitHeightScatAbs1;
			Double_t FinalEnergyFitHeightScatAbs2;
			Double_t FinalEnergyFitHeightScatComp;
			Double_t FinalEnergyFitHeightScatComp1;
			Double_t FinalEnergyFitHeightScatComp2;
			Double_t FinalEnergyFitOffset;
			Double_t FinalEnergyFitOffset2;
			Double_t FinalBackHeight1;
			Double_t FinalBackHeight2;
			Double_t FinalEnergyFitChiSquare;
			Double_t FinalRangeLow1;
			Double_t FinalRangeLow2;
			Double_t FinalRangeHigh1;
			Double_t FinalRangeHigh2;
			Bool_t BadAFit;
			// Fix flags for parameters
			vector <bool> FixPrelimEnergyFitMean;
			vector <bool> FixPrelimEnergyFitSigma;
			vector <bool> FixPrelimEnergyFitHeight;
			vector <bool> FixEnergyFitMean;
			vector <bool> FixEnergyFitSigma;
			vector <bool> FixEnergyFitHeight;
			vector <bool> FixEnergyFitHeight2;
			vector <bool> FixOverflow;
			vector <bool> FixOverflowHeight;
			vector <bool> FixBackHeight;
			vector <bool> FixEnergyFitHeightCompEdge;
			vector <bool> FixEnergyFitHeightCompEdge2;
			vector <bool> FixEnergyFitHeightScatAbs;
			vector <bool> FixEnergyFitHeightScatAbs2;
			vector <bool> FixEnergyFitHeightScatComp;
			vector <bool> FixEnergyFitHeightScatComp2;
			vector <bool> FixEnergyFitOffset;
			Bool_t FixFinalEnergyFitMean;
			Bool_t FixFinalEnergyFitSigma;
			Bool_t FixFinalEnergyFitHeight;
			Bool_t FixFinalEnergyFitHeight1;
			Bool_t FixFinalEnergyFitHeight2;
			Bool_t FixFinalEnergyFitHeightCompEdge;
			Bool_t FixFinalEnergyFitHeightCompEdge1;
			Bool_t FixFinalEnergyFitHeightCompEdge2;
			Bool_t FixFinalEnergyFitHeightScatAbs;
			Bool_t FixFinalEnergyFitHeightScatAbs1;
			Bool_t FixFinalEnergyFitHeightScatAbs2;
			Bool_t FixFinalEnergyFitHeightScatComp;
			Bool_t FixFinalEnergyFitHeightScatComp1;
			Bool_t FixFinalEnergyFitHeightScatComp2;
			Bool_t FixFinalEnergyFitOffset;
			Bool_t FixFinalEnergyFitOffset2;

			vector <bool> DoCounts;
			vector <bool> DoPrelimFit;
			vector <bool> DoGuessLimits;
			vector <bool> DoBckgOv;
			vector <bool> DoFinalFit;
			bool CrystalLogAvailable = false;


			UInt_t size = Energy.size();
			PrelimEnergyFitMean.resize(size);
			PrelimEnergyFitSigma.resize(size);
			PrelimEnergyFitHeight.resize(size);
			PrelimChiSquare.resize(size);
			EnergyFitMean.resize(size);
			EnergyFitSigma.resize(size);
			EnergyFitHeight.resize(size);
			EnergyFitHeight2.resize(size);
			BadCounts.resize(size);  // not enough counts in spectrum
			BadEFit.resize(size);    // >= 1 bad energy fit occurred
			BadSFit.resize(size);
			Overflow.resize(size);
			OverflowHeight.resize(size);
			BackHeight.resize(size);
			GuessedLowerEnergy.resize(size);
			GuessedUpperEnergy.resize(size);
			EnergyFitHeightCompEdge.resize(size);
			EnergyFitHeightCompEdge2.resize(size);
			EnergyFitHeightScatAbs.resize(size);
			EnergyFitHeightScatAbs2.resize(size);
			EnergyFitHeightScatComp.resize(size);
			EnergyFitHeightScatComp2.resize(size);
			EnergyFitOffset.resize(size);
			RangeLow.resize(size);
			RangeHigh.resize(size);
			FixPrelimEnergyFitMean.resize(size);
			FixPrelimEnergyFitSigma.resize(size);
			FixPrelimEnergyFitHeight.resize(size);
			FixEnergyFitMean.resize(size);
			FixEnergyFitSigma.resize(size);
			FixEnergyFitHeight.resize(size);
			FixEnergyFitHeight2.resize(size);
			FixOverflow.resize(size);
			FixOverflowHeight.resize(size);
			FixBackHeight.resize(size);
			FixEnergyFitHeightCompEdge.resize(size);
			FixEnergyFitHeightCompEdge2.resize(size);
			FixEnergyFitHeightScatAbs.resize(size);
			FixEnergyFitHeightScatAbs2.resize(size);
			FixEnergyFitHeightScatComp.resize(size);
			FixEnergyFitHeightScatComp2.resize(size);
			FixEnergyFitOffset.resize(size);
			EnergyFitChiSquare.resize(size);
			DoCounts.resize(size);
			DoPrelimFit.resize(size);
			DoGuessLimits.resize(size);
			DoBckgOv.resize(size);
			DoFinalFit.resize(size);

			// Goto pixel x y in input log file
			if (lin && (InLogFileName!="")) {
				// lets start from the beginning
				lin.close();
				lin.open(InLogFileName.Data());
				if (!lin.is_open()) {
					cout << "Error reopening input log file! Exiting ..." << endl;
					return 1;
				}
				// skipping up to pixel x y
				for(;!lin.eof();) {
					getline(lin, InputLine);
					if (InputLine.size() < 3) continue; // Line empty
					if ((InputLine.c_str()[0] == 'P') && (InputLine.c_str()[1] == 'I')) {
						istringstream inputstream(InputLine);
						string dummy;
						int xread = -1;
						int yread = -1;
						inputstream >> dummy >> xread >> yread;
						if (!inputstream.bad()) {
							if ((xread==x) && (yread==y)) {
								CrystalLogAvailable = true;
								break;
							}
						}
					}
				}
				if (lin.eof() || lin.bad()) {
					cout << "There is no crystal x=" << x << " y=" << y 
							 << "in log file! Using defaults." << endl;
				}
			}

			// Initialize values
			PrelimOffset = 0;
			for (UInt_t i=0; i<size; ++i) {
				PrelimEnergyFitMean[i] = -1;
				FixPrelimEnergyFitMean[i] = false;
				PrelimEnergyFitSigma[i] = -1;
				FixPrelimEnergyFitSigma[i] = false;
				PrelimEnergyFitHeight[i] = -1;
				FixPrelimEnergyFitHeight[i] = false;
				PrelimChiSquare[i] = -1;
				EnergyFitMean[i] = -1;
				FixEnergyFitMean[i] = false;
				EnergyFitSigma[i] = -1;
				FixEnergyFitSigma[i] = false;
				EnergyFitHeight[i] = -1;
				FixEnergyFitHeight[i] = false;
				EnergyFitHeight2[i] = -1;
				FixEnergyFitHeight2[i] = false;
				EnergyFitOffset[i] = 0.0;
				FixEnergyFitOffset[i] = false;
				BadCounts[i] = kFALSE;
				BadEFit[i] = kFALSE;
				BadSFit[i] = kFALSE;
				GuessedLowerEnergy[i] = -1;
				GuessedUpperEnergy[i] = -1;
				Overflow[i] = 600.0;
				FixOverflow[i] = false;
				OverflowHeight[i] = 2.0;
				FixOverflowHeight[i] = false;
				BackHeight[i] = 1.0;
				FixBackHeight[i] = false;
				EnergyFitHeightCompEdge[i] = 1.0;
				FixEnergyFitHeightCompEdge[i] = false;
				EnergyFitHeightCompEdge2[i] = 0.1;
				FixEnergyFitHeightCompEdge2[i] = false;
				EnergyFitHeightScatAbs[i] = 10.0;
				FixEnergyFitHeightScatAbs[i] = false;
				EnergyFitHeightScatAbs2[i] = 1.0;
				FixEnergyFitHeightScatAbs2[i] = false;
				EnergyFitHeightScatComp[i] = 10.0;
				FixEnergyFitHeightScatComp[i] = false;
				EnergyFitHeightScatComp2[i] = 1.0;
				FixEnergyFitHeightScatComp2[i] = false;
				EnergyFitChiSquare[i] = -1;
				RangeLow[i] = -1;
				RangeHigh[i] = -1;
				DoCounts[i] = true;
				DoPrelimFit[i] = true;
				if (i==0) DoGuessLimits[i] = false;
				else DoGuessLimits[i] = true;
				DoBckgOv[i] = true;
				DoFinalFit[i] = true;
			}
			FinalEnergyFitMean = -1;
			FinalEnergyFitSigma = -1;
			FinalEnergyFitHeight = -1;
			FinalEnergyFitHeight1 = -1;
			FinalEnergyFitHeight2 = -1;
			FinalEnergyFitHeightCompEdge = -1;
			FinalEnergyFitHeightCompEdge1 = -1;
			FinalEnergyFitHeightCompEdge2 = -1;
			FinalEnergyFitHeightScatAbs = -1;
			FinalEnergyFitHeightScatAbs1 = -1;
			FinalEnergyFitHeightScatAbs2 = -1;
			FinalEnergyFitHeightScatComp = -1;
			FinalEnergyFitHeightScatComp1 = -1;
			FinalEnergyFitHeightScatComp2 = -1;
			FinalEnergyFitOffset = 0.0;
			FinalEnergyFitOffset2 = 0.0;
			FinalBackHeight1 = -1;
			FinalBackHeight2 = -1;
			FinalEnergyFitChiSquare = -1;
			FinalRangeLow1 = -1;
			FinalRangeLow2 = -1;
			FinalRangeHigh1 = -1;
			FinalRangeHigh2 = -1;
			FixFinalEnergyFitMean = false;
			FixFinalEnergyFitSigma = false;
			FixFinalEnergyFitHeight = false;
			FixFinalEnergyFitHeight1 = false;
			FixFinalEnergyFitHeight2 = false;
			FixFinalEnergyFitHeightCompEdge = false;
			FixFinalEnergyFitHeightCompEdge1 = false;
			FixFinalEnergyFitHeightCompEdge2 = false;
			FixFinalEnergyFitHeightScatAbs = false;
			FixFinalEnergyFitHeightScatAbs1 = false;
			FixFinalEnergyFitHeightScatAbs2 = false;
			FixFinalEnergyFitHeightScatComp = false;
			FixFinalEnergyFitHeightScatComp1 = false;
			FixFinalEnergyFitHeightScatComp2 = false;
			FixFinalEnergyFitOffset = false;
			FixFinalEnergyFitOffset2 = false;
			BadAFit = false;


			if(CrystalLogAvailable) {
				// read last values from log file
				do {
					getline(lin, InputLine);

					// Line empty
					if (InputLine.size() < 2) continue; 

					// not enough counts
					if ((InputLine.c_str()[0] == 'N') && (InputLine.c_str()[1] == 'C')) {
						istringstream inputstream(InputLine);
						string dummy;
						int LineNo = -1;
						inputstream >> dummy >> LineNo;
						if (!inputstream.bad() && !inputstream.fail()) {
							if (LineNo < (signed) BadCounts.size()) {
								BadCounts[LineNo] = true;
								DoCounts[LineNo] = false;
								continue;
							}
						} else {
							cout << "Error reading NC line from log file" << endl;
						}
					}

					// read prelim. photo peak fit
					if ((InputLine.c_str()[0] == 'P') && (InputLine.c_str()[1] == 'P')) {
						istringstream inputstream(InputLine);
						string dummy;
						int LineNo = -1;
						double Energy = -1;
						double Sigma = -1;
						double Height = -1;
						double LowBorder = -1;
						double UpBorder = -1;
						double Chi2 = -1;
						bool BadFit = true;
						inputstream >> dummy >> LineNo >> Energy >> Sigma >> Height 
												>> LowBorder >> UpBorder >> Chi2 >> BadFit;
						if (!inputstream.bad() && !inputstream.fail()) {
							if (LineNo < (signed) BadCounts.size()) {
								PrelimEnergyFitMean[LineNo] = Energy;
								FixPrelimEnergyFitMean[LineNo] = true;
								PrelimEnergyFitSigma[LineNo] = Sigma;
								FixPrelimEnergyFitSigma[LineNo] = true;
								PrelimEnergyFitHeight[LineNo] = Height;
								FixPrelimEnergyFitHeight[LineNo] = true;
								GuessedLowerEnergy[LineNo] = LowBorder;
								GuessedUpperEnergy[LineNo] = UpBorder;
								BadEFit[LineNo] = BadFit;
								DoPrelimFit[LineNo] = false;
								DoGuessLimits[LineNo] = false;
								PrelimChiSquare[LineNo] = Chi2;
								continue;
							}
						} else {
							cout << "Error reading PP line from log file" << endl;
						}
					}

					// read overflow bin
					if ((InputLine.c_str()[0] == 'O') && (InputLine.c_str()[1] == 'V')) {
						istringstream inputstream(InputLine);
						string dummy;
						int LineNo = -1;
						double overflow = -1;
						inputstream >> dummy >> LineNo >> overflow;
						if (!inputstream.bad() && !inputstream.fail()) {
							if (LineNo < (signed) BadCounts.size()) {
								Overflow[LineNo] = overflow;
								FixOverflow[LineNo] = true;
								DoBckgOv[LineNo] = false;
								continue;
							}
						} else {
							cout << "Error reading OV line from log file" << endl;
						}
					}

					// read background problem
					if ((InputLine.c_str()[0] == 'B') && (InputLine.c_str()[1] == 'P')) {
						istringstream inputstream(InputLine);
						string dummy;
						int LineNo = -1;
						double CalcBk = -1;
						double FitBk = -1;
						double CalcOverflow = -1;
						double FitOverflow = -1;
						inputstream >> dummy >> LineNo >> CalcBk >> FitBk >> CalcOverflow 
												>> FitOverflow;
						if (!inputstream.bad() && !inputstream.fail()) {
							if (LineNo < (signed) BadCounts.size()) {
								Overflow[LineNo] = FitOverflow;
								FixOverflow[LineNo] = false;
								BackHeight[LineNo] = FitBk;
								FixBackHeight[LineNo] = false;
								DoBckgOv[LineNo] = false;
								continue;
							}
						} else {
							cout << "Error reading BP line from log file" << endl;
						}
					}

					// read background
					if ((InputLine.c_str()[0] == 'B') && (InputLine.c_str()[1] == 'G')) {
						istringstream inputstream(InputLine);
						string dummy;
						int LineNo = -1;
						double FitBk = -1;
						inputstream >> dummy >> LineNo >> FitBk;
						if (!inputstream.bad() && !inputstream.fail()) {
							if (LineNo < (signed) BadCounts.size()) {
								BackHeight[LineNo] = FitBk;
								FixBackHeight[LineNo] = true;
								DoBckgOv[LineNo] = false;
								continue;
							}
						} else {
							cout << "Error reading BG line from log file" << endl;
						}
					}

					// read fit of single peak spectrum
					if ((InputLine.c_str()[0] == 'F') && (InputLine.c_str()[1] == 'S')) {
						istringstream inputstream(InputLine);
						string dummy;
						int LineNo = -1;
						double Par0 = -1;
						double Par1 = -1;
						double Par2 = -1;
						double Par3 = -1;
						double Par4 = -1;
						double Par5 = -1;
						double Par6 = -1;
						double Par7 = -1;
						double rangelow = -1;
						double rangehigh = -1;
						double chi2 = -1;
						bool fitbad = false;
						inputstream >> dummy >> LineNo >> Par0 >> Par1 >> Par2 >> Par3 
												>> Par4 >> Par5 >> Par6 >> Par7 >> rangelow 
												>> rangehigh >> chi2 >> fitbad;
						if (!inputstream.bad() && !inputstream.fail()) {
							if (LineNo < (signed) BadCounts.size()) {
								EnergyFitMean[LineNo] =	Par0;
								FixEnergyFitMean[LineNo] = true;
								EnergyFitSigma[LineNo] = Par1;
								FixEnergyFitSigma[LineNo] = true;
								EnergyFitHeight[LineNo] =	Par2;
								FixEnergyFitHeight[LineNo] = true;
								EnergyFitHeight2[LineNo] = -1;
								FixEnergyFitHeight2[LineNo] = true;
								EnergyFitHeightCompEdge[LineNo] =	Par3;
								FixEnergyFitHeightCompEdge[LineNo] = true;
								EnergyFitHeightCompEdge2[LineNo] = -1;
								FixEnergyFitHeightCompEdge2[LineNo] = true;
								EnergyFitHeightScatAbs[LineNo] =	Par4;
								FixEnergyFitHeightScatAbs[LineNo] = true;
								EnergyFitHeightScatAbs2[LineNo] = -1;
								FixEnergyFitHeightScatAbs2[LineNo] = true;
								EnergyFitHeightScatComp[LineNo] =	Par5;
								FixEnergyFitHeightScatComp[LineNo] = true;
								EnergyFitHeightScatComp2[LineNo] = -1;
								FixEnergyFitHeightScatComp2[LineNo] = true;
								EnergyFitOffset[LineNo] =	Par6;
								FixEnergyFitOffset[LineNo] = true;
								BackHeight[LineNo] =	Par7;
								RangeLow[LineNo] = rangelow;
								RangeHigh[LineNo] = rangehigh;
								FixBackHeight[LineNo] =	true;
								EnergyFitChiSquare[LineNo] = chi2;
								BadSFit[LineNo] = fitbad;
								DoFinalFit[LineNo] = false;
								continue;
							}
						} else {
							cout << "Error reading FS line from log file" << endl;
						}
					}

					// read fit of double peak spectrum
					if ((InputLine.c_str()[0] == 'F') && (InputLine.c_str()[1] == 'D')) {
						istringstream inputstream(InputLine);
						string dummy;
						int LineNo = -1;
						double Par0 = -1;
						double Par1 = -1;
						double Par2 = -1;
						double Par3 = -1;
						double Par4 = -1;
						double Par5 = -1;
						double Par6 = -1;
						double Par7 = -1;
						double Par8 = -1;
						double Par9 = -1;
						double Par10 = -1;
						double Par11 = -1;
						double rangelow = -1;
						double rangehigh = -1;
						double chi2 = -1;
						bool fitbad = false;
						inputstream >> dummy >> LineNo >> Par0 >> Par1 >> Par2 >> Par3 
												>> Par4 >> Par5 >> Par6 >> Par7 >> Par8 >> Par9 
												>> Par10 >> Par11 >> rangelow >> rangehigh 
												>> chi2 >> fitbad;
						// cout << dummy << " " <<  LineNo << " " <<  Par0 << " " <<  Par1 
						// 			<< " " <<  Par2 << " " <<  Par3 << " " <<  Par4 << " " 
						// 			<<  Par5 << " " <<  Par6 << " " <<  Par7 << " " <<  Par8 
						// 		  << " " <<  Par9 << " " <<  Par10 << " " <<  Par11 << " " 
						// 			<<  fitbad << chi2 << endl;
						if (!inputstream.bad() && !inputstream.fail()) {
							if (LineNo < (signed) BadCounts.size()) {
								EnergyFitMean[LineNo] =	Par0;
								FixEnergyFitMean[LineNo] = true;
								EnergyFitSigma[LineNo] = Par1;
								FixEnergyFitSigma[LineNo] = true;
								EnergyFitHeight[LineNo] =	Par2;
								FixEnergyFitHeight[LineNo] = true;
								EnergyFitHeight2[LineNo] = Par3;
								FixEnergyFitHeight2[LineNo] = true;
								EnergyFitHeightCompEdge[LineNo] =	Par4;
								FixEnergyFitHeightCompEdge[LineNo] = true;
								EnergyFitHeightCompEdge2[LineNo] = Par5;
								FixEnergyFitHeightCompEdge2[LineNo] = true;
								EnergyFitHeightScatComp[LineNo] =	Par8;
								FixEnergyFitHeightScatComp[LineNo] = true;
								EnergyFitHeightScatComp2[LineNo] = Par9;
								FixEnergyFitHeightScatComp2[LineNo] = true;
								EnergyFitHeightScatAbs[LineNo] =	Par6;
								FixEnergyFitHeightScatAbs[LineNo] = true;
								EnergyFitHeightScatAbs2[LineNo] = Par7;
								FixEnergyFitHeightScatAbs2[LineNo] = true;
								EnergyFitOffset[LineNo] =	Par10;
								FixEnergyFitOffset[LineNo] = true;
								BackHeight[LineNo] =	Par11;
								RangeLow[LineNo] = rangelow;
								RangeHigh[LineNo] = rangehigh;
								FixBackHeight[LineNo] =	true;
								EnergyFitChiSquare[LineNo] = chi2;
								BadSFit[LineNo] = fitbad;
								DoFinalFit[LineNo] = false;
								continue;
							}	
						} else {
							cout << "Error reading FD line from log file" << endl;
						}
					}

					// read fit of double peak spectrum
					if ((InputLine.c_str()[0] == 'F') && (InputLine.c_str()[1] == 'A')) {
						istringstream inputstream(InputLine);
						string dummy;
						double Par0 = -1;
						double Par1 = -1;
						double Par2 = -1;
						double Par3 = -1;
						double Par4 = -1;
						double Par5 = -1;
						double Par6 = -1;
						double Par7 = -1;
						double Par8 = -1;
						double Par9 = -1;
						double Par10 = -1;
						double Par11 = -1;
						double Par12 = -1;
						double Par13 = -1;
						double Par14 = -1;
						double Par15 = -1;
						double Par16 = -1;
						double Par17 = -1;
						double Par18 = -1;
						double rangelow = -1;
						double rangehigh = -1;
						double chi2 = -1;
						bool fitbad = false;
						inputstream >> dummy >> Par0 >> Par1 >> Par2 >> Par3 
												>> Par4 >> Par5 >> Par6 >> Par7 >> Par8 >> Par9 
												>> Par10 >> Par11 >> Par12 >> Par13 >> Par14 >> Par15
												>> Par16 >> Par17 >> Par18 >> rangelow >> rangehigh
												>> chi2 >> fitbad;
//  						cout << dummy << " " <<  Par0 << " " <<  Par1 
// 								 << " " <<  Par2 << " " <<  Par3 << " " <<  Par4 << " " 
// 								 <<  Par5 << " " <<  Par6 << " " <<  Par7 << " " <<  Par8 
// 								 << " " <<  Par9 << " " <<  Par10 << " " <<  Par11 << " "
// 								 << Par12 << " " << Par13 << " " << Par14 << " " << Par15 
// 								 << " " << Par16 << " " << Par17 << " " << Par18 << " " 
// 								 << rangelow << " " << rangehigh << " " << chi2 << " " 
// 								 << fitbad << endl;
						if (!inputstream.bad() && !inputstream.fail()) {
							FinalEnergyFitMean = Par0;
							FixFinalEnergyFitMean = true;
							FinalEnergyFitSigma = Par1;
							FixFinalEnergyFitSigma = true;
							FinalEnergyFitHeight =	Par2;
							FixFinalEnergyFitHeight = true;
							FinalEnergyFitHeight1 =	Par3;
							FixFinalEnergyFitHeight1 = true;
							FinalEnergyFitHeight2 = Par4;
							FixFinalEnergyFitHeight2 = true;
							FinalEnergyFitHeightCompEdge =	Par5;
							FixFinalEnergyFitHeightCompEdge = true;
							FinalEnergyFitHeightCompEdge1 =	Par6;
							FixFinalEnergyFitHeightCompEdge1 = true;
							FinalEnergyFitHeightCompEdge2 = Par7;
							FixFinalEnergyFitHeightCompEdge2 = true;
							FinalEnergyFitHeightScatAbs =	Par8;
							FixFinalEnergyFitHeightScatAbs = true;
							FinalEnergyFitHeightScatAbs1 =	Par9;
							FixFinalEnergyFitHeightScatAbs1 = true;
							FinalEnergyFitHeightScatAbs2 = Par10;
							FixFinalEnergyFitHeightScatAbs2 = true;
							FinalEnergyFitHeightScatComp =	Par11;
							FixFinalEnergyFitHeightScatComp = true;
							FinalEnergyFitHeightScatComp1 =	Par12;
							FixFinalEnergyFitHeightScatComp1 = true;
							FinalEnergyFitHeightScatComp2 = Par13;
							FixFinalEnergyFitHeightScatComp2 = true;
							FinalEnergyFitOffset =	Par14;
							FixFinalEnergyFitOffset = true;
							FinalBackHeight1 =	Par15;
							FinalBackHeight2 =	Par16;
							FinalRangeLow1 = rangelow;
							FinalRangeLow2 = Par18;
							FinalRangeHigh1 = Par17;
							FinalRangeHigh2 = rangehigh;
							FinalEnergyFitChiSquare = chi2;
							BadAFit = fitbad;
							continue;
						} else {
							cout << "Error reading FA line from log file" << endl;
						}
					}

				} while (!( (InputLine.c_str()[0]=='P') && 
										(InputLine.c_str()[1]=='I') )  && 
								 !lin.eof());
			}

			if (FixPhotoPeak) {
				for (UInt_t i=0; i<FixEnergyFitMean.size(); ++i) {
					FixEnergyFitMean[i] = true;
					FixEnergyFitSigma[i] = true;
					FixEnergyFitOffset[i] = true;
				}
			}
			if (ReleaseParameters) {
				for (UInt_t i=0; i<FixEnergyFitMean.size(); ++i) {
					FixEnergyFitMean[i] = false;
					FixEnergyFitSigma[i] = false;
					FixEnergyFitOffset[i] = false;
					FixEnergyFitHeight[i] = false;
					FixEnergyFitHeight2[i] = false;
					FixEnergyFitHeightCompEdge[i] = false;
					FixEnergyFitHeightCompEdge2[i] = false;
					FixEnergyFitHeightScatAbs[i] = false;
					FixEnergyFitHeightScatAbs2[i] = false;
					FixEnergyFitHeightScatComp[i] = false;
					FixEnergyFitHeightScatComp2[i] = false;
				}
			}

			/***********************************************************************
			 *                           first stage
			 *                      prelim. photo peak fit
			 **********************************************************************/
				
			// Redo-loop for user parameters
			Bool_t Redo = kFALSE;
			do { // Redo-loop
				// Loop over the different lines
				for (UInt_t LineNo=0; LineNo<Energy.size(); ++LineNo) { // lines loop

					// Get and draw histograms
					CalCanvas[LineNo]->cd();
					MString Title(DetectorName);
					Title += ": Data of pixel: x=";
					Title += x;
					Title += ", y=";
					Title += y;
					Title += ", E=";
					Title += Energy[LineNo];
					CalCanvas[LineNo]->SetTitle(Title);
					Title = DetectorName;
					Title += ":_Data_of_pixel:_x";
					Title += x;
					Title += "_y";
					Title += y;
					Title += "_E";
					Title += Energy[LineNo];
					CalCanvas[LineNo]->SetName(Title);
					
					crystal = Detectors[LineNo]->GetCalibHist(x-1, y-1); 
					crystal->SetStats(kFALSE);
					crystal->Draw();
					
					// Enough counts in spectrum?
					AllInt = crystal->Integral();
					if ((AllInt < MinCounts) && DoCounts[LineNo]) {
						BadCounts[LineNo] = kTRUE;
					}
					DoCounts[LineNo] = kFALSE;

					if (!BadCounts[LineNo]) {
						// if the first line delivered an energy calibration, 
						// use it to shrink ranges
					
						if ( DoGuessLimits[LineNo] ) {
							cout << "First Energy: " << PrelimEnergyFitMean[0]
									 << ", first Sigma: " << PrelimEnergyFitSigma[0] << endl;
							
							PrelimOffset = -20.0;
							if ( PrelimEnergyFitMean[0] != -1 ) {
								PrelimEnergyFitMean[LineNo] = 
									(PrelimEnergyFitMean[0]-PrelimOffset)
									* Energy[LineNo] / Energy[0] 
									+	PrelimOffset;
								cout << "Projected new Energy is " 
										 << PrelimEnergyFitMean[LineNo] << endl;
								
								// three sigma around prognosis of new energy
								GuessedLowerEnergy[LineNo] = PrelimEnergyFitMean[LineNo] - 
									2.0*PrelimEnergyFitSigma[0];
								GuessedUpperEnergy[LineNo] = PrelimEnergyFitMean[LineNo] + 
									3.0*PrelimEnergyFitSigma[0];
								if (GuessedLowerEnergy[LineNo] < 0) 
									GuessedLowerEnergy[LineNo] = 0;
							}
						}

						EnergyFit[LineNo]->SetParameters(PrelimEnergyFitHeight[LineNo],
																						 PrelimEnergyFitMean[LineNo],
																						 PrelimEnergyFitSigma[LineNo]);
						if (FixPrelimEnergyFitMean[LineNo]) {
							EnergyFit[LineNo]->FixParameter(1, PrelimEnergyFitMean[LineNo]);
							//cout << "Prelim Mean: " << PrelimEnergyFitMean[LineNo] << endl;
						}
						if (FixPrelimEnergyFitSigma[LineNo]) {
							EnergyFit[LineNo]->FixParameter(2, PrelimEnergyFitSigma[LineNo]);
							//cout << "Prelim Sigma: " << PrelimEnergyFitSigma[LineNo]<<endl;
						}
						if (FixPrelimEnergyFitHeight[LineNo]) {
							EnergyFit[LineNo]->FixParameter(0,PrelimEnergyFitHeight[LineNo]);
							//cout << "Prelim Height: "<<PrelimEnergyFitHeight[LineNo]<<endl;
						}
						if((GuessedLowerEnergy[LineNo] != -1) &&
							 (GuessedUpperEnergy[LineNo] != -1)) {
							EnergyFit[LineNo]->SetRange(GuessedLowerEnergy[LineNo], 
																					GuessedUpperEnergy[LineNo]);
						}
						if (DoPrelimFit[LineNo]) {
							if (FitPeak(crystal, 
													EnergyFit[LineNo], 
													PrelimEnergyFitMean[LineNo], 
													PrelimEnergyFitSigma[LineNo], 
													PrelimEnergyFitHeight[LineNo], 
													GuessedLowerEnergy[LineNo], 
													GuessedUpperEnergy[LineNo])) {
								BadEFit[LineNo] = kFALSE;
							} else {
								BadEFit[LineNo] = kTRUE;
								BackHeight[LineNo] = -1;
								Overflow[LineNo] = -1;
							}
							PrelimChiSquare[LineNo] = EnergyFit[LineNo]->GetChisquare();
						} else {
							if (BadEFit[LineNo]) EnergyFit[LineNo]->SetLineColor(2);
							else EnergyFit[LineNo]->SetLineColor(3);
						}

						if (!BadEFit[LineNo] && DoBckgOv[LineNo]) {
							// and overflow and background
							if (Overflow[LineNo] <= 0) {
								Overflow[LineNo] = 600;
								OverflowHeight[LineNo] = 2.0;
								BackHeight[LineNo] = 1.0;
							}
							BckgOv[LineNo]->SetParameters(BackHeight[LineNo], 
																						OverflowHeight[LineNo],
																						Overflow[LineNo],
																						PrelimEnergyFitSigma[LineNo]);

							if (FixBackHeight[LineNo])
								BckgOv[LineNo]->FixParameter(0, BackHeight[LineNo]);
							else
								BckgOv[LineNo]->SetParLimits(0, 0.0, 100.0);

							if (FixOverflowHeight[LineNo])
								BckgOv[LineNo]->FixParameter(1, OverflowHeight[LineNo]);
							else
								BckgOv[LineNo]->SetParLimits(1,0.0, 100.0);

							if (FixOverflow[LineNo])
								BckgOv[LineNo]->FixParameter(2, Overflow[LineNo]);
							else
								BckgOv[LineNo]->
									SetParLimits(2,0,crystal->GetXaxis()->GetXmax());

							BckgOv[LineNo]->FixParameter(3, PrelimEnergyFitSigma[LineNo]);

							Double_t MaxADCEnergy = (Energy[LineNo] > Energy2[LineNo]) ? 
								PrelimEnergyFitMean[LineNo] : 
								(PrelimEnergyFitMean[LineNo]/Energy[LineNo]*Energy2[LineNo]);
							BckgOv[LineNo]->
								SetRange(MaxADCEnergy + 10.0*PrelimEnergyFitSigma[LineNo],
												 crystal->GetXaxis()->GetXmax());
							BckgOv[LineNo]->SetLineColor(3);
							crystal->Fit(BckgOv[LineNo], "RLLI");
							//EnergyFit[LineNo]->Draw("lsame");
							Overflow[LineNo] = BckgOv[LineNo]->GetParameter(2) - 
								3.0 * PrelimEnergyFitSigma[LineNo];
							BackHeight[LineNo] = BckgOv[LineNo]->GetParameter(0);
							OverflowHeight[LineNo] = BckgOv[LineNo]->GetParameter(1);

							// second version of determining backgr. & overfl.
							Int_t HighestBin;
							Stat_t counts=0;
							for (HighestBin=crystal->GetXaxis()->GetLast();
									 (HighestBin > 0) && (counts < 2);
									 --HighestBin) {
								counts += crystal->GetBinContent(HighestBin);
							}
							Int_t LowBin = 
								crystal->FindBin(MaxADCEnergy + 
																 10.0 * PrelimEnergyFitSigma[LineNo]);
							Axis_t HighX = crystal->GetBinCenter(HighestBin) - 
								6.0 * PrelimEnergyFitSigma[LineNo];
							Int_t HighBin = crystal->FindBin(HighX);
							counts = crystal->Integral(LowBin, HighBin);
							Double_t NoiseHeight = (double) counts / 
								(double) (HighBin-LowBin+1);
							cout << "NoiseHeight = " << NoiseHeight << " and Ovpos = " 
									 << HighBin << endl;
							if ((BackHeight[LineNo] > 1.2 * NoiseHeight) || 
									(BackHeight[LineNo] < 0.8 * NoiseHeight) ||
									(Overflow[LineNo]   > 1.2 * HighX) ||
									(Overflow[LineNo]   < 0.8 * HighX) ) {
								cout << "\e[43mWarning: Background fit probably bad!\e[0m" 
										 << endl;
								lout << "BP " << LineNo << " " << NoiseHeight << " " 
										 << BackHeight[LineNo] << " " << HighX << " " 
										 << Overflow[LineNo] << endl;
							}
						}

						lout << "PP " << LineNo << " "
								 << PrelimEnergyFitMean[LineNo] << " " 
								 << PrelimEnergyFitSigma[LineNo] << " "
								 << PrelimEnergyFitHeight[LineNo] << " "
								 << GuessedLowerEnergy[LineNo] << " "
								 << GuessedUpperEnergy[LineNo] << " "
								 << PrelimChiSquare[LineNo] << " "
								 << BadEFit[LineNo] << endl;
						lout << "OV " << LineNo << " " << Overflow[LineNo] << endl;
						lout << "BG " << LineNo << " " << BackHeight[LineNo] << endl;
					}	else {
						cout << "Not enough counts" << endl;
						lout << "NC " << LineNo << endl;
						PrelimEnergyFitMean[LineNo] = -1;
						PrelimEnergyFitSigma[LineNo] = -1;
						PrelimEnergyFitHeight[LineNo] = -1;
						BackHeight[LineNo] = -1;
						Overflow[LineNo] = -1;
						OverflowHeight[LineNo] = -1;
					}

				} // End Line-Loop

				Redo = kFALSE;
				for (UInt_t i=0; i<CalCanvas.size(); ++i) {
					CalCanvas[i]->cd();
					if (!BadEFit[i]) {
						BckgOv[i]->DrawCopy("lsame");
					}
					EnergyFit[i]->Draw("lsame");
					CalCanvas[i]->Update();
				}

				// Wait for a key if necessary...
				
				Bool_t Wait = kFALSE;
				Bool_t BadCountsAll = kFALSE;
				
				switch (Stop) {
				case 1:
					for (UInt_t i=0; i<BadEFit.size(); ++i) {
						Wait |= BadEFit[i];
					}
					break;
				case 2:
					for (UInt_t i=0; i<BadCounts.size(); ++i) {
						Wait |= BadEFit[i];
						Wait |= BadCounts[i];
						BadCountsAll |= BadCounts[i];
					}
					break;
				case 4:
					Wait = kTRUE;
					break;
				}
				
				if (Wait) {	
					for (;;) {
						cout << endl
								 << " Line  | " << "enough | " << "  Guessed   | " 
								 << "  Guessed   | " << "   Over-    | " << "  Back- " << endl;
						cout << "number | " << "counts | " << "  lower E   | " 
								 << "  upper E   | " << "   flow     | " << "  ground" << endl;
						for (UInt_t i=0; i<Energy.size(); ++i) {
							cout << setw(4) << i+1 << "   | ";
							if (BadCounts[i]) cout << "   no  | ";
							else cout << "  yes  | ";
							cout << scientific << setprecision(4)
									 << setw(11) << GuessedLowerEnergy[i] << " | " 
									 << setw(11) << GuessedUpperEnergy[i] << " | "
									 << setw(11) << Overflow[i] << " | "
									 << setw(11) << BackHeight[i] << endl;
						}

						char input[10];
						MString inp;
						Bool_t inpvalid = kFALSE;
						int selection = -1;
						double inpvalue = -1;
						input[0]=0;
						timer->TurnOn();
						timer->Reset();
						cout << endl << "If you want to change a parameter,"
								 << " type the linenumber and <return>" << endl;
						cout << "After changing a parameter, the fits are redone. "
								 << "Type 'n' and <return> to go on with the actual overflow "
								 << "and background values but without refitting" << endl;
						cout << "Type 'q' for quit" << endl; 
						cout << "If nothing at all, just type <return>" << endl;

						cin.get(input,9);
						cin.clear();
						cin.ignore(numeric_limits<std::streamsize>::max(), '\n');
						if (input[0]==0) {
							Bool_t nobreak=kFALSE;
							for (UInt_t i=0; i<GuessedUpperEnergy.size(); ++i) {
								if (GuessedUpperEnergy[i] != -1)
									nobreak |= (GuessedLowerEnergy[i] >= GuessedUpperEnergy[i]);
							}
							if (nobreak) {
								cout << "\e[43mCannot go on, because of bad E limits!\e[0m" 
										 << endl << endl;
							} else { 
								break;
							}
						} else if (input[0]=='n') {
							Redo = kFALSE;
							break;
						} else if (input[0]=='q') {
							Exit = kTRUE;
							Redo = kFALSE;
							break;
						} else if (input[0]=='r') {
							Redo = kTRUE;
							break;
						} else {
							selection = atoi(input) - 1;
							cout << selection << endl;
							if((selection >= 0)&&(selection < (signed) Energy.size())) {
								do {
									cout << "Choose parameter to change:" << endl;
									if (BadCounts[selection]) 
										cout << " c: there are ENOUGH counts" << endl;
									else cout << " c: there are NOT enough counts" << endl;
									cout << " l: change lower E" << endl
											 << " u: change upper E" << endl
											 << " b: change background level" << endl
											 << " o: change overflow value" << endl
											 << " n: nothing at all" << endl
											 << "Your choice (c/l/u/b/o/n): ";
									cin >> inp;
									cin.clear();
									cin.ignore(numeric_limits<std::streamsize>::max(), '\n');
									switch(inp.Data()[0]) {
									case 'c':
										BadCounts[selection] = !BadCounts[selection];
										inpvalid = kTRUE;
										Redo = kTRUE;
										break;
									case 'l':
										cout << "new lower E: ";
										inpvalue = -1;
										cin >> inpvalue;
										cin.clear();
										cin.ignore(numeric_limits<std::streamsize>::max(), '\n');
										if ( (inpvalue >= 0) && 
												 (inpvalue <= Detectors[selection]->
													GetCalibHist(x-1,y-1)->GetXaxis()->GetXmax()) ) {
											GuessedLowerEnergy[selection] = inpvalue;
											DoGuessLimits[selection] = false;
											Redo = kTRUE;
										} else {
											cout << "No valid number! Nothing changed." << endl;
										}
										inpvalid = kTRUE;
										break;
									case 'u':
										cout << "new upper E: ";
										inpvalue = -1;
										cin >> inpvalue;
										cin.clear();
										cin.ignore(numeric_limits<std::streamsize>::max(), '\n');
										if ( (inpvalue >= 0) && 
												 (inpvalue <= Detectors[selection]->
													GetCalibHist(x-1,y-1)->GetXaxis()->GetXmax()) ) {
											GuessedUpperEnergy[selection] = inpvalue;
											DoGuessLimits[selection] = false;
											Redo = kTRUE;
										} else {
											cout << "No valid number! Nothing changed." << endl;
										}
										inpvalid = kTRUE;
										break;
									case 'b':
										cout << "new background level: ";
										inpvalue = -1;
										cin >> inpvalue;
										cin.clear();
										cin.ignore(numeric_limits<std::streamsize>::max(), '\n');
										if (inpvalue >= 0) {
											 BackHeight[selection] = inpvalue;
											 Redo = kTRUE;
										} else {
											cout << "No valid number! Nothing changed." << endl;
										}
										inpvalid = kTRUE;
										break;
									case 'o':
										cout << "new overflow: ";
										inpvalue = -1;
										cin >> inpvalue;
										cin.clear();
										cin.ignore(numeric_limits<std::streamsize>::max(), '\n');
										if ( (inpvalue >= 0) && 
												 (inpvalue <= Detectors[selection]->
													GetCalibHist(x-1,y-1)->GetXaxis()->GetXmax()) ) {
											Overflow[selection] = inpvalue;
											Redo = kTRUE;

										} else {
											cout << "No valid number! Nothing changed." << endl;
										}
										inpvalid = kTRUE;
										break;
									case 'n':
										inpvalid = kTRUE;
										break;
									default:
										cout << "What's \"" << inp << "\"?? Try again." << endl;
									}
								} while(!inpvalid);
							} else {
								cout << "What do you mean with \"" << input 
										 << "\"? Try again." << endl;
							}
							
						}
					}
					timer->TurnOff();
				}
				cout << "Redo: " << Redo << " x=" << x << " y=" << y << endl;	
			} while(Redo);	

			if (Exit) break;

			if (OnlyPhoto) continue;

			// Now do a preliminary lin fit 
			// first count good fits
			delete LinFit;
			LinFit = new TGraph();
			UInt_t goodlines=0;
			Double_t MaxADC=0.0, MaxE=0.0;

			for (UInt_t i=0; i<PrelimEnergyFitMean.size(); ++i) {
				if (PrelimEnergyFitMean[i]>=0.0) ++goodlines;
			}
			if (goodlines==0) {
				cout << "Here a line for a bad crystal should be written..." << endl;
			} else {
				if (goodlines==1) {
					LinFit->Set(2);
					LinFit->SetPoint(0, -20, 0); // guessing offset of 20 ADC counts
					for (UInt_t i=0; i<PrelimEnergyFitMean.size(); ++i) {
						if (PrelimEnergyFitMean[i]>=0.0) {
							LinFit->SetPoint(1, PrelimEnergyFitMean[i], Energy[i]);
							if (PrelimEnergyFitMean[i] > MaxADC) 
								MaxADC = PrelimEnergyFitMean[i];
							if (Energy[i] > MaxE) MaxE = Energy[i];
							break;
						}
					}
				} else {
					LinFit->Set(goodlines);
					UInt_t j=0;
					for (UInt_t i=0; i<PrelimEnergyFitMean.size(); ++i) {
						if (PrelimEnergyFitMean[i]>=0.0) {
							LinFit->SetPoint(j, PrelimEnergyFitMean[i], Energy[i]);
							if (PrelimEnergyFitMean[i] > MaxADC) 
								MaxADC = PrelimEnergyFitMean[i];
							if (Energy[i] > MaxE) MaxE = Energy[i];
							++j;
						}
					}
				}
				LinFitCanvas->cd();
				LinFit->GetYaxis()->SetLimits(-5, MaxE+10);
				LinFit->GetXaxis()->SetLimits(-30, MaxADC+10);
				LinFit->Draw("ALP");
				LinFit->GetYaxis()->UnZoom();
				LinFit->GetXaxis()->UnZoom();
				LinearFit->SetRange(-30, MaxADC);
				LinFit->Fit("LinearFit", "RQ");
				LinFit->SetMarkerStyle(3);
				LinFit->Draw("AP");
				LinFitCanvas->Update();
			}

		// 	char input[10];
// 			timer->TurnOn();
// 			timer->Reset();
// 			// Now let's read the input
// 			cout << "Type <return> to continue ";
// 			cin.getline(input,9);
// 			timer->TurnOff();

			PrelimOffset = (LinearFit->GetParameter(0)) / 
				(LinearFit->GetParameter(1));
			cout << "Prelim. Offset: " << PrelimOffset << endl;

		
			// Copy prelim. results to final as start point
			for (UInt_t i=0; i<EnergyFitOffset.size(); ++i) {
// 				cout << "Mean before fitting: " << EnergyFitMean[i]
// 						 << " prelim: " << PrelimEnergyFitMean[i] << endl;
				if (!CrystalLogAvailable || (EnergyFitMean[i]==-1)) {
					EnergyFitOffset[i] = -PrelimOffset;
					EnergyFitMean[i] = PrelimEnergyFitMean[i];
					EnergyFitSigma[i] = PrelimEnergyFitSigma[i];
					EnergyFitHeight[i] = PrelimEnergyFitHeight[i];
					// Fix the parameters
					FixEnergyFitOffset[i] = true;
					FixEnergyFitMean[i] = true;
					FixEnergyFitSigma[i] = true;
				}
			}

			/************************************************************************
			 *                         second stage
			 *                       whole spectrum fit  
			 ***********************************************************************/

			Redo = kFALSE;
			do {
				for (UInt_t LineNo=0; LineNo < Energy.size(); ++LineNo) {
					if (!BadCounts[LineNo]) {
						crystal = Detectors[LineNo]->GetCalibHist(x-1,y-1);
						CalCanvas[LineNo]->cd();
						
						if (Energy2[LineNo]==0.0) {
							
							xin = Energy[LineNo]/511.0;
							
							Fit1->SetParameters(EnergyFitMean[LineNo], 
																	EnergyFitSigma[LineNo], 
																	EnergyFitHeight[LineNo], 
																	EnergyFitHeightCompEdge[LineNo], 
																	EnergyFitHeightScatAbs[LineNo], 
																	EnergyFitHeightScatComp[LineNo], 
																	EnergyFitOffset[LineNo],
																	BackHeight[LineNo]);

							if (FixEnergyFitMean[LineNo])
								Fit1->FixParameter(0, EnergyFitMean[LineNo]);
							else
								Fit1->SetParLimits(0, 
																	 EnergyFitMean[LineNo] - 
																	 5.0 * EnergyFitSigma[LineNo],
																	 EnergyFitMean[LineNo] +
																	 5.0 * EnergyFitSigma[LineNo]);

							if (FixEnergyFitSigma[LineNo])
								Fit1->FixParameter(1, EnergyFitSigma[LineNo]);
							else
								Fit1->ReleaseParameter(1);
							//Fit1->SetParLimits(1,EnergyFitSigma[LineNo]*2.0/3.0,
							//									 EnergyFitSigma[LineNo]*4.0/3.0);


							if (FixEnergyFitHeight[LineNo])
								Fit1->FixParameter(2, EnergyFitHeight[LineNo]);
							else
								Fit1->SetParLimits(2, 0.0, 10000.0);
						
							if (FixEnergyFitHeightCompEdge[LineNo])
								Fit1->FixParameter(3, EnergyFitHeightCompEdge[LineNo]);
							else
								Fit1->SetParLimits(3, 0.0, 10000.0);

							if (FixEnergyFitHeightScatAbs[LineNo])
								Fit1->FixParameter(4, EnergyFitHeightScatAbs[LineNo]);
							else
								Fit1->SetParLimits(4, 0.0, 10000.0);

							if (FixEnergyFitHeightScatComp[LineNo])
								Fit1->FixParameter(5, EnergyFitHeightScatComp[LineNo]);
							else
								Fit1->SetParLimits(5, 0.0, 10000.0);
						
							if (FixEnergyFitOffset[LineNo])
								Fit1->FixParameter(6, EnergyFitOffset[LineNo]);
							else
								Fit1->ReleaseParameter(6);
							//Fit1->SetParLimits(6, -50, 50);

							Fit1->FixParameter(7, BackHeight[LineNo]);						

							Int_t TriggerThresh;
							if (TThresholds[x-1][y-1] == -1) {
								for (TriggerThresh=0; 
										 TriggerThresh<crystal->GetNbinsX(); 
										 ++TriggerThresh) {
									if (crystal->GetBinContent(TriggerThresh)>5) break;
								}
								cout << "TriggerThresh guessed = " << TriggerThresh << endl;
							} else {
								TriggerThresh = TThresholds[x-1][y-1];
								//cout << "TriggerThresh from File = " << TriggerThresh << endl;
							}
							
							// Drawing line at trigger threshold
							ThreshLine[LineNo]->SetX1(TriggerThresh); 
							ThreshLine[LineNo]->SetY1(-1.0);
							ThreshLine[LineNo]->SetX2(TriggerThresh);
							ThreshLine[LineNo]->SetY2(1.1*crystal->GetMaximum());
							ThreshLine[LineNo]->Draw("L");
							CalCanvas[LineNo]->Update();
							ThreshLine[LineNo]->Draw("L");

							if ( (RangeLow[LineNo] == -1)||(RangeHigh[LineNo] == -1)) {
								// Fit1->SetRange(30, EnergyFitMean[LineNo]+
								//							 5*EnergyFitSigma[LineNo]); 
								//Fit1->SetRange(crystal->GetBinCenter(TriggerThresh+3), 
								//							 EnergyFitMean[LineNo] + 
								//               5.0 * EnergyFitSigma[LineNo]);
								RangeLow[LineNo] = crystal->GetBinCenter(TriggerThresh) + 
									3.0*EnergyFitSigma[LineNo];
								RangeHigh[LineNo] = 1.5*EnergyFitMean[LineNo];
							}
							Fit1->SetRange(RangeLow[LineNo], RangeHigh[LineNo]);
							
							now.Now();
							cout << "Fitting (start at " << now.GetSQLString() << ")..." 
									 << endl;

							BadSFit[LineNo] = (Bool_t) crystal->Fit(Fit1, "R"); // <----- Fit

							now.Now();
							cout << "... done (at " << now.GetSQLString() << ")" << endl;
							Fit1->SetLineColor(3);
							Fit1->Draw("LSAME");
							ThreshLine[LineNo]->Draw("L");
							CalCanvas[LineNo]->Update();
							ThreshLine[LineNo]->Draw("L");

							EnergyFitMean[LineNo] =	Fit1->GetParameter(0);
							EnergyFitSigma[LineNo] = Fit1->GetParameter(1);
							EnergyFitHeight[LineNo] =	Fit1->GetParameter(2);
							EnergyFitHeight2[LineNo] = -1;
							EnergyFitHeightCompEdge[LineNo] =	Fit1->GetParameter(3);
							EnergyFitHeightCompEdge2[LineNo] = -1;
							EnergyFitHeightScatAbs[LineNo] =	Fit1->GetParameter(4);
							EnergyFitHeightScatAbs2[LineNo] = -1;
							EnergyFitHeightScatComp[LineNo] =	Fit1->GetParameter(5);
							EnergyFitHeightScatComp2[LineNo] = -1;
							EnergyFitOffset[LineNo] =	Fit1->GetParameter(6);
							BackHeight[LineNo] =	Fit1->GetParameter(7);
							EnergyFitChiSquare[LineNo] = Fit1->GetChisquare();

							lout << "FS " << LineNo << " ";
							for (UInt_t i=0; i<8; ++i) {
								lout << Fit1->GetParameter(i) << " ";
							}
							Double_t low, high;
							Fit1->GetRange(low, high);
							lout << low << " " 
									 << high << " "
									 << EnergyFitChiSquare[LineNo] << " "
									 << BadSFit[LineNo] << endl;
							
						} else {
							
							xin1 = Energy[LineNo]/511.0;
							xin2 = Energy2[LineNo]/511.0;
							
							Fit2->SetParameters(EnergyFitMean[LineNo], 
																	EnergyFitSigma[LineNo], 
																	EnergyFitHeight[LineNo],
																	EnergyFitHeight2[LineNo],
																	EnergyFitHeightCompEdge[LineNo],
																	EnergyFitHeightCompEdge2[LineNo],
																	EnergyFitHeightScatAbs[LineNo],
																	EnergyFitHeightScatAbs2[LineNo], 
																	EnergyFitHeightScatComp[LineNo],
																	EnergyFitHeightScatComp2[LineNo], 
																	EnergyFitOffset[LineNo]);
							Fit2->SetParameter(11, BackHeight[LineNo]);
							
							if (FixEnergyFitMean[LineNo])
								Fit2->FixParameter(0, EnergyFitMean[LineNo]);
							else
								Fit2->SetParLimits(0, 
																	 EnergyFitMean[LineNo] -
																	 5.0 * EnergyFitSigma[LineNo],
																	 EnergyFitMean[LineNo] +
																	 5.0 * EnergyFitSigma[LineNo]);

							if (FixEnergyFitSigma[LineNo])
								Fit2->FixParameter(1, EnergyFitSigma[LineNo]);
							else
								Fit2->SetParLimits(1,EnergyFitSigma[LineNo]*2.0/3.0,
																	 EnergyFitSigma[LineNo]*4.0/3.0);
							
							if (FixEnergyFitHeight[LineNo])
								Fit2->FixParameter(2, EnergyFitHeight[LineNo]);
							else
								Fit2->SetParLimits(2, 0.0, 100000.0);
							
							if (FixEnergyFitHeight2[LineNo])
								Fit2->FixParameter(3, EnergyFitHeight2[LineNo]);
							else
								Fit2->SetParLimits(3, 0.0, 100000.0);
							
							if (FixEnergyFitHeightCompEdge[LineNo])
								Fit2->FixParameter(4, EnergyFitHeightCompEdge[LineNo]);
							else
								Fit2->SetParLimits(4, 0.0, 100000.0);
							
							if (FixEnergyFitHeightCompEdge2[LineNo])
								Fit2->FixParameter(5, EnergyFitHeightCompEdge2[LineNo]);
							else
								Fit2->SetParLimits(5, 0.0, 100000.0);
						
							if (FixEnergyFitHeightScatAbs[LineNo])
								Fit2->FixParameter(6, EnergyFitHeightScatAbs[LineNo]);
							else
								Fit2->SetParLimits(6, 0.0, 100000.0);
							
							if (FixEnergyFitHeightScatAbs2[LineNo])
								Fit2->FixParameter(7, EnergyFitHeightScatAbs2[LineNo]);
							else
								Fit2->SetParLimits(7, 0.0, 100000.0);
							
							if (FixEnergyFitHeightScatComp[LineNo])
								Fit2->FixParameter(8, EnergyFitHeightScatComp[LineNo]);
							else
								Fit2->SetParLimits(8, 0.0, 100000.0);

							if (FixEnergyFitHeightScatComp2[LineNo])
								Fit2->FixParameter(9, EnergyFitHeightScatComp2[LineNo]);
							else
								Fit2->SetParLimits(9, 0.0, 100000.0);

							if (FixEnergyFitOffset[LineNo])
								Fit2->FixParameter(10, EnergyFitOffset[LineNo]);
							else
								Fit2->ReleaseParameter(10);
							//Fit2->SetParLimits(10, -50, 50);

							Fit2->FixParameter(11, BackHeight[LineNo]);
							
							Int_t TriggerThresh;
							if (TThresholds[x][y] == -1) {
								for (TriggerThresh=0; 
										 TriggerThresh<crystal->GetNbinsX(); 
										 ++TriggerThresh) {
									if (crystal->GetBinContent(TriggerThresh)>5) break;
								}
								cout << "TriggerThresh guessed = " << TriggerThresh << endl;
							} else {
								TriggerThresh = TThresholds[x-1][y-1];
								cout << "TriggerThresh from File = " << TriggerThresh << endl;
							}
							
							// Drawing line at trigger threshold
							ThreshLine[LineNo]->SetX1(TriggerThresh); 
							ThreshLine[LineNo]->SetY1(-1.0);
							ThreshLine[LineNo]->SetX2(TriggerThresh);
							ThreshLine[LineNo]->SetY2(1.1*crystal->GetMaximum());
							ThreshLine[LineNo]->Draw("L");
							CalCanvas[LineNo]->Update();
							ThreshLine[LineNo]->Draw("L");

							Double_t EnergyMax = 
								(Energy[LineNo] > Energy2[LineNo]) ? 
								Energy[LineNo] : Energy2[LineNo];

							if ( (RangeLow[LineNo] == -1) || 
									 (RangeHigh[LineNo] == -1)) {				
								RangeLow[LineNo] = crystal->GetBinCenter(TriggerThresh) + 
									3.0*EnergyFitSigma[LineNo];
								RangeHigh[LineNo] = 2.0 * EnergyFitMean[LineNo] / 
									Energy[LineNo] * EnergyMax;
							}
							Fit2->SetRange(RangeLow[LineNo], RangeHigh[LineNo]); 
							now.Now();
							cout << "Fitting (start at " << now.GetSQLString() << ")..." 
									 << endl;
							BadSFit[LineNo] = (Bool_t) crystal->Fit(Fit2,"R");
							
							now.Now();
							cout << "... done (at " << now.GetSQLString() << ")" << endl;
							Fit2->SetLineColor(3);
							Fit2->Draw("LSAME");
							ThreshLine[LineNo]->Draw("L");
							CalCanvas[LineNo]->Update();
							ThreshLine[LineNo]->Draw("L");
							
							EnergyFitMean[LineNo] =	Fit2->GetParameter(0);
							EnergyFitSigma[LineNo] = Fit2->GetParameter(1);
							EnergyFitHeight[LineNo] =	Fit2->GetParameter(2);
							EnergyFitHeight2[LineNo] =	Fit2->GetParameter(3);
							EnergyFitHeightCompEdge[LineNo] =	Fit2->GetParameter(4);
							EnergyFitHeightCompEdge2[LineNo] =	Fit2->GetParameter(5);
							EnergyFitHeightScatAbs[LineNo] =	Fit2->GetParameter(6);
							EnergyFitHeightScatAbs2[LineNo] =	Fit2->GetParameter(7);
							EnergyFitHeightScatComp[LineNo] =	Fit2->GetParameter(8);
							EnergyFitHeightScatComp2[LineNo] =	Fit2->GetParameter(9);
							EnergyFitOffset[LineNo] =	Fit2->GetParameter(10);
							BackHeight[LineNo] = Fit2->GetParameter(11);
							EnergyFitChiSquare[LineNo] = Fit2->GetChisquare();

							lout << "FD " << LineNo << " ";
							for (UInt_t i=0; i<12; ++i) {
								lout << Fit2->GetParameter(i) << " ";
							}
							Double_t low, high;
							Fit2->GetRange(low, high);
							lout << low << " " 
									 << high << " "
									 << EnergyFitChiSquare[LineNo] << " "
									 << BadSFit[LineNo] << endl;
						}
					} else {
						cout << "Bad line" << endl;
						// ... output ... parameter setzen ...
					}
				}

				Redo = kFALSE;

				// Wait for a key if necessary...
				
				Bool_t Wait = kFALSE;
				Bool_t BadCountsAll = kFALSE;
				
				switch (Stop) {
				case 1:
				case 2:
					for (UInt_t i=0; i<BadCounts.size(); ++i) {
						Wait |= BadEFit[i];
						BadCountsAll |= BadCounts[i];
					}
					break;
				case 4:
					Wait = kTRUE;
					break;
				}
				


				if (Wait) {
					for (;;) {
						cout << endl
								 << " Line number     ";
						for (UInt_t i=0; i<Energy.size(); ++i) {
							cout << "     " << setw(3) << i+1 << "        ";
						}
						cout << endl
								 << " Offset          ";
						for (UInt_t i=0; i<Energy.size(); ++i) {
							cout << " " << setw(11) << EnergyFitOffset[i] << " ";
							if (FixEnergyFitOffset[i]) cout << " x ";
							else cout << "   ";
						}
						cout << endl
								 << " Peak position   ";
						for (UInt_t i=0; i<Energy.size(); ++i) {
							cout << " " << setw(11) << EnergyFitMean[i] << " ";
							if (FixEnergyFitMean[i]) cout << " x ";
							else cout << "   ";
						}
						cout << endl
								 << " Sigma           ";
						for (UInt_t i=0; i<Energy.size(); ++i) {
							cout << " " << setw(11) << EnergyFitSigma[i] << " ";
							if (FixEnergyFitSigma[i]) cout << " x ";
							else cout << "   ";
						}
						cout << endl
								 << " Height peak 1   ";
						for (UInt_t i=0; i<Energy.size(); ++i) {
							cout << " " << setw(11) << EnergyFitHeight[i] << " ";
							if (FixEnergyFitHeight[i]) cout << " x ";
							else cout << "   ";
						}
						cout << endl
								 << " Height peak 2   ";
						for (UInt_t i=0; i<Energy.size(); ++i) {
							if (Energy2[i]!=0.0) {
								cout << " " << setw(11) << EnergyFitHeight2[i] << " ";
								if (FixEnergyFitHeight2[i]) cout << " x ";
								else cout << "   ";
							} else {
								cout << "                ";
							}
						}
						cout << endl
								 << " Height Comp.e.1 ";
						for (UInt_t i=0; i<Energy.size(); ++i) {
							cout << " " << setw(11) << EnergyFitHeightCompEdge[i] << " ";
							if (FixEnergyFitHeightCompEdge[i]) cout << " x ";
							else cout << "   ";
						}
						cout << endl
								 << " Height Comp.e.2 ";
						for (UInt_t i=0; i<Energy.size(); ++i) {
							if (Energy2[i]!=0.0) {
								cout << " " << setw(11) << EnergyFitHeightCompEdge2[i] << " ";
								if (FixEnergyFitHeightCompEdge2[i]) cout << " x ";
								else cout << "   ";
							} else {
								cout << "                ";
							}
						}
						cout << endl
								 << " Height Abs.sc.1 ";
						for (UInt_t i=0; i<Energy.size(); ++i) {
							cout << " " << setw(11) << EnergyFitHeightScatAbs[i] << " ";
							if (FixEnergyFitHeightScatAbs[i]) cout << " x ";
							else cout << "   ";
						}
						cout << endl
								 << " Height Abs.sc.2 ";
						for (UInt_t i=0; i<Energy.size(); ++i) {
							if (Energy2[i]!=0.0) {
								cout << " " << setw(11) << EnergyFitHeightScatAbs2[i] << " ";
								if (FixEnergyFitHeightScatAbs2[i]) cout << " x ";
								else cout << "   ";
							} else {
								cout << "                ";
							}
						}
						cout << endl
								 << " Height Compsc.1 ";
						for (UInt_t i=0; i<Energy.size(); ++i) {
							cout << " " << setw(11) << EnergyFitHeightScatComp[i] << " ";
							if (FixEnergyFitHeightScatComp[i]) cout << " x ";
							else cout << "   ";
						}
						cout << endl
								 << " Height Compsc.2 ";
						for (UInt_t i=0; i<Energy.size(); ++i) {
							if (Energy2[i]!=0.0) {
								cout << " " << setw(11) << EnergyFitHeightScatComp2[i] << " ";
								if (FixEnergyFitHeightScatComp2[i]) cout << " x ";
								else cout << "   ";
							} else {
								cout << "                ";
							}
						}
						cout << endl
								 << " Background      ";
						for (UInt_t i=0; i<Energy.size(); ++i) {
							cout << " " << setw(11) << BackHeight[i] << " ";
							if (FixBackHeight[i]) cout << " x ";
							else cout << "   ";
						}
						cout << endl
								 << " Fit range low   ";
						for (UInt_t i=0; i<Energy.size(); ++i) {
							cout << " " << setw(11) << RangeLow[i] << "    ";
						}
						cout << endl
								 << " Fit range high  ";
						for (UInt_t i=0; i<Energy.size(); ++i) {
							cout << " " << setw(11) << RangeHigh[i] << "    ";
						}
						cout << endl
								 << " Chi2            ";
						for (UInt_t i=0; i<Energy.size(); ++i) {
							cout << " " << setw(11) << EnergyFitChiSquare[i] << "    ";
						}
						cout << endl;

						char input[10];
						MString inp;
						Bool_t inpvalid = kFALSE;
						int selection = -1;
						double inpvalue = -1;
						input[0]=0;
						timer->TurnOn();
						timer->Reset();
						cout << endl << "If you want to change a parameter,"
								 << " type the linenumber and <return>" << endl
								 << "After changing a parameter, the fits are redone. " << endl
								 << "Type 's' to skip this pixel, continue with next one" 
								 << endl
								 << "Type 'f' to go back to first stage" << endl
								 << "Type 'n' to go on without refitting" << endl
								 << "Type 'q' for quit" << endl
								 << "If everything is ok, just type <return>" << endl;

						cin.get(input,9);
						cin.clear();
						cin.ignore(numeric_limits<std::streamsize>::max(), '\n');
						if (input[0]==0) {
								break;
						} else if (input[0]=='n') {
							Redo = kFALSE;
							break;
						} else if (input[0]=='q') {
							Exit = kTRUE;
							Redo = kFALSE;
							break;
						} else if (input[0]=='r') {
							Redo = kTRUE;
							break;
						} else if (input[0]=='s') {
							Redo = kFALSE;
							NextCrystal = kTRUE;
							break;
						} else if (input[0]=='f') {
							Redo = kFALSE;
							FirstStageAgain = kTRUE;
							break;
						} else {
							selection = atoi(input) - 1;
							cout << selection << endl;
							if((selection >= 0)&&(selection < (signed) Energy.size())) {
								do {
									cout << "Choose parameter to change:" << endl;
									cout << " of: change offset" << endl
											 << " pp: change peak position" << endl
											 << " si: change sigma" << endl
											 << " p1: change height of photo peak 1" << endl
											 << " p2: change height of photo peak 2" << endl
											 << " c1: change height of compton edge 1" << endl
											 << " c2: change height of compton edge 2" << endl
											 << " a1: change height of absorbed scattered 1" << endl
											 << " a2: change height of absorbed scattered 2" << endl
											 << " s1: change height of scattered compton 1" << endl
											 << " s2: change height of scattered compton 2" << endl
											 << " bg: change height of background level" << endl
											 << " rl: change lower border of fit range" << endl
											 << " ru: change upper border of fit range" << endl
											 << " u:  unfix all parameters of line" << endl
											 << " n:  nothing at all" << endl
											 << "Your choice (of/pp/si/p1/p2/c1/c2/a1/a2/s1/s2/bg/rl/ru/u/n): ";
									cin >> inp;
									cin.clear();
									cin.ignore(numeric_limits<std::streamsize>::max(), '\n');
									switch(inp.Data()[0]) {
									case 'o':
										cout << "New offset: (value of f to toggle parameter fix flag) ";
										cin >> inp;
										cin.clear();
										cin.ignore(numeric_limits<std::streamsize>::max(), '\n');
										if (inp.Data()[0] == 'f') {
											FixEnergyFitOffset[selection] = 
												!FixEnergyFitOffset[selection];
											Redo = kTRUE;
										} else { 
											istringstream InputStream(inp.Data());
											InputStream >> inpvalue;
											if(!InputStream.bad() && !InputStream.fail()) {
												EnergyFitOffset[selection] = inpvalue;
												Redo = kTRUE;
											} else {
												cout << "Not a number or 'f' - ignored!" << endl;
											}
										}
										inpvalid = kTRUE;
										break;
									case 'p':
										switch(inp.Data()[1]) {
										case 'p':
											cout << "New peak position: "
													 << "(value of f to toggle parameter fix flag) ";
											cin >> inp;
											cin.clear();
											cin.ignore(numeric_limits<std::streamsize>::max(), '\n');
											if (inp.Data()[0] == 'f') {
												FixEnergyFitMean[selection] = 
													!FixEnergyFitMean[selection];
												Redo = kTRUE;
											} else { 
												istringstream InputStream(inp.Data());
												InputStream >> inpvalue;
												if(!InputStream.bad() && !InputStream.fail() && 
													 (inpvalue > 0) && 
													 (inpvalue <= Detectors[selection]->
														GetCalibHist(x-1,y-1)->GetXaxis()->GetXmax())) {
													EnergyFitMean[selection] = inpvalue;
													Redo = kTRUE;
												} else {
													cout << "Not a number or out of range - ignored!" 
															 << endl;
												}
											}
											inpvalid = kTRUE;
											break;
										case '1':
											cout << "New height of photo peak 1: "
													 << "(value of f to toggle parameter fix flag) ";
											cin >> inp;
											cin.clear();
											cin.ignore(numeric_limits<std::streamsize>::max(), '\n');
											if (inp.Data()[0] == 'f') {
												FixEnergyFitHeight[selection] = 
													!FixEnergyFitHeight[selection];
												Redo = kTRUE;
											} else { 
												istringstream InputStream(inp.Data());
												InputStream >> inpvalue;
												if(!InputStream.bad() && !InputStream.fail() && 
													 (inpvalue > 0)) {
													EnergyFitHeight[selection] = inpvalue;
													Redo = kTRUE;
												} else {
													cout << "Not a number or out of range - ignored!" 
															 << endl;
												}
											}
											inpvalid = kTRUE;
											break;
										case '2':
											cout << "New height of photo peak 2: "
													 << "(value of f to toggle parameter fix flag) ";
											cin >> inp;
											cin.clear();
											cin.ignore(numeric_limits<std::streamsize>::max(), '\n');
											if (inp.Data()[0] == 'f') {
												FixEnergyFitHeight2[selection] = 
													!FixEnergyFitHeight2[selection];
												Redo = kTRUE;
											} else { 
												istringstream InputStream(inp.Data());
												InputStream >> inpvalue;
												if(!InputStream.bad() && !InputStream.fail() && 
													 (inpvalue > 0)) {
													EnergyFitHeight2[selection] = inpvalue;
													Redo = kTRUE;
												} else {
													cout << "Not a number or out of range - ignored!" 
															 << endl;
												}
											}
											inpvalid = kTRUE;
											break;
										default:
											cout << "What's \"" << input << "\" ? Input ignored!" 
													 << endl;
										}
										break;
									case 's':
										switch(inp.Data()[1]) {
										case 'i':
											cout << "New sigma: "
													 << "(value of f to toggle parameter fix flag) ";
											cin >> inp;
											cin.clear();
											cin.ignore(numeric_limits<std::streamsize>::max(), '\n');
											if (inp.Data()[0] == 'f') {
												FixEnergyFitSigma[selection] = 
													!FixEnergyFitSigma[selection];
												Redo = kTRUE;
											} else { 
												istringstream InputStream(inp.Data());
												InputStream >> inpvalue;
												cout << "inpvalue: " << inpvalue << endl;
												if(!InputStream.bad() && !InputStream.fail() && 
													 (inpvalue > 0)) {
													EnergyFitSigma[selection] = inpvalue;
													Redo = kTRUE;
												} else {
													cout << "Not a number or out of range - ignored!" 
															 << endl;
												}
											}
											inpvalid = kTRUE;
											break;
										case '1':
											cout << "New height of scattered compton 1: "
													 << "(value of f to toggle parameter fix flag) ";
											cin >> inp;
											cin.clear();
											cin.ignore(numeric_limits<std::streamsize>::max(), '\n');
											if (inp.Data()[0] == 'f') {
												FixEnergyFitHeightScatComp[selection] = 
													!FixEnergyFitHeightScatComp[selection];
												Redo = kTRUE;
											} else { 
												istringstream InputStream(inp.Data());
												InputStream >> inpvalue;
												if(!InputStream.bad() && !InputStream.fail() && 
													 (inpvalue > 0)) {
													EnergyFitHeightScatComp[selection] = inpvalue;
													Redo = kTRUE;
												} else {
													cout << "Not a number or out of range - ignored!" 
															 << endl;
												}
											}
											inpvalid = kTRUE;
											break;
										case '2':
											cout << "New height of scattered compton 2: "
													 << "(value of f to toggle parameter fix flag) ";
											cin >> inp;
											cin.clear();
											cin.ignore(numeric_limits<std::streamsize>::max(), '\n');
											if (inp.Data()[0] == 'f') {
												FixEnergyFitHeightScatComp2[selection] = 
													!FixEnergyFitHeightScatComp2[selection];
												Redo = kTRUE;
											} else { 
												istringstream InputStream(inp.Data());
												InputStream >> inpvalue;
												if(!InputStream.bad() && !InputStream.fail() && 
													 (inpvalue > 0)) {
													EnergyFitHeightScatComp2[selection] = inpvalue;
													Redo = kTRUE;
												} else {
													cout << "Not a number or out of range - ignored!" 
															 << endl;
												}
											}
											inpvalid = kTRUE;
											break;
										default:
											cout << "What's \"" << input << "\" ? Input ignored!" 
													 << endl;
										}
										break;
									case 'c':
										switch(inp.Data()[1]) {
										case '1':
											cout << "New height of compton edge 1: "
													 << "(value of f to toggle parameter fix flag) ";
											cin >> inp;
											cin.clear();
											cin.ignore(numeric_limits<std::streamsize>::max(), '\n');
											if (inp.Data()[0] == 'f') {
												FixEnergyFitHeightCompEdge[selection] = 
													!FixEnergyFitHeightCompEdge[selection];
												Redo = kTRUE;
											} else { 
												istringstream InputStream(inp.Data());
												InputStream >> inpvalue;
												if(!InputStream.bad() && !InputStream.fail() && 
													 (inpvalue > 0)) {
													EnergyFitHeightCompEdge[selection] = inpvalue;
													Redo = kTRUE;
												} else {
													cout << "Not a number or out of range - ignored!" 
															 << endl;
												}
											}
											inpvalid = kTRUE;
											break;
										case '2':
											cout << "New height of compton edge 2: "
													 << "(value of f to toggle parameter fix flag) ";
											cin >> inp;
											cin.clear();
											cin.ignore(numeric_limits<std::streamsize>::max(), '\n');
											if (inp.Data()[0] == 'f') {
												FixEnergyFitHeightCompEdge2[selection] = 
													!FixEnergyFitHeightCompEdge2[selection];
												Redo = kTRUE;
											} else { 
												istringstream InputStream(inp.Data());
												InputStream >> inpvalue;
												if(!InputStream.bad() && !InputStream.fail() && 
													 (inpvalue > 0)) {
													EnergyFitHeightCompEdge2[selection] = inpvalue;
													Redo = kTRUE;
												} else {
													cout << "Not a number or out of range - ignored!" 
															 << endl;
												}
											}
											inpvalid = kTRUE;
											break;
										default:
											cout << "What's \"" << input << "\" ? Input ignored!" 
													 << endl;
										}
										break;
									case 'a':
										switch(inp.Data()[1]) {
										case '1':
											cout << "New height of absorbed scatter 1: "
													 << "(value of f to toggle parameter fix flag) ";
											cin >> inp;
											cin.clear();
											cin.ignore(numeric_limits<std::streamsize>::max(), '\n');
											if (inp.Data()[0] == 'f') {
												FixEnergyFitHeightScatAbs[selection] = 
													!FixEnergyFitHeightScatAbs[selection];
												Redo = kTRUE;
											} else { 
												istringstream InputStream(inp.Data());
												InputStream >> inpvalue;
												if(!InputStream.bad() && !InputStream.fail() && 
													 (inpvalue > 0)) {
													EnergyFitHeightScatAbs[selection] = inpvalue;
													Redo = kTRUE;
												} else {
													cout << "Not a number or out of range - ignored!" 
															 << endl;
												}
											}
											inpvalid = kTRUE;
											break;
										case '2':
											cout << "New height of absorbed scatter 2: "
													 << "(value of f to toggle parameter fix flag) ";
											cin >> inp;
											cin.clear();
											cin.ignore(numeric_limits<std::streamsize>::max(), '\n');
											if (inp.Data()[0] == 'f') {
												FixEnergyFitHeightScatAbs2[selection] = 
													!FixEnergyFitHeightScatAbs2[selection];
												Redo = kTRUE;
											} else { 
												istringstream InputStream(inp.Data());
												InputStream >> inpvalue;
												if(!InputStream.bad() && !InputStream.fail() && 
													 (inpvalue > 0)) {
													EnergyFitHeightScatAbs2[selection] = inpvalue;
													Redo = kTRUE;
												} else {
													cout << "Not a number or out of range - ignored!" 
															 << endl;
												}
											}
											inpvalid = kTRUE;
											break;
										default:
											cout << "What's \"" << input << "\" ? Input ignored!" 
													 << endl;
										}
										break;
									case 'b':
										cout << "new background level: "
												 << "(value of f to toggle parameter fix flag) ";
										cin >> inp;
										cin.clear();
										cin.ignore(numeric_limits<std::streamsize>::max(), '\n');
										if (inp.Data()[0] == 'f') {
											FixBackHeight[selection] = !FixBackHeight[selection];
											Redo = kTRUE;
										} else { 
											istringstream InputStream(inp.Data());
											inpvalue = -1;
											InputStream >> inpvalue;
											if (!InputStream.bad() && !InputStream.fail() && 
													(inpvalue >= 0)) {
												BackHeight[selection] = inpvalue;
												Redo = kTRUE;
											} else {
												cout << "No valid number! Nothing changed." << endl;
											}
										}
										inpvalid = kTRUE;
										break;
									case 'r':
										switch(inp.Data()[1]) {
										case 'l':
											cout << "new lower border of fit range: ";
											inpvalue = -1;
											cin >> inpvalue;
											cin.clear();
											cin.ignore(numeric_limits<std::streamsize>::max(), '\n');
											if (cin.good() && (inpvalue > 0) && 
													(inpvalue < Detectors[selection]->
													 GetCalibHist(x-1,y-1)->GetXaxis()->GetXmax())) {
												RangeLow[selection] = inpvalue;
												Redo = kTRUE;
											} else {
												cout << "No valid number! Nothing changed." << endl;
											}
											inpvalid = kTRUE;
											break;
										case 'u':
											cout << "new upper border of fit range: ";
											inpvalue = -1;
											cin >> inpvalue;
											cin.clear();
											cin.ignore(numeric_limits<std::streamsize>::max(), '\n');
											if (cin.good() && (inpvalue > 0) && 
													(inpvalue < Detectors[selection]->
													 GetCalibHist(x-1,y-1)->GetXaxis()->GetXmax())) {
												RangeHigh[selection] = inpvalue;
												Redo = kTRUE;
											} else {
												cout << "No valid number! Nothing changed." << endl;
											}
											inpvalid = kTRUE;
											break;
										default:
											cout << "What's \"" << input << "\" ? Input ignored!" 
													 << endl;
										}
										break;
									case 'u':
										cout << "unfixing all parameters";
										FixEnergyFitOffset[selection] = false;
										FixEnergyFitMean[selection] = false;
										FixEnergyFitHeight[selection] = false;
										FixEnergyFitHeight2[selection] = false;
										FixEnergyFitSigma[selection] = false;
										FixEnergyFitHeightScatComp[selection] = false;
										FixEnergyFitHeightScatComp2[selection] = false;
										FixEnergyFitHeightCompEdge[selection] = false;
										FixEnergyFitHeightCompEdge2[selection] = false;
										FixEnergyFitHeightScatAbs[selection] = false;
										FixEnergyFitHeightScatAbs2[selection] = false;
										FixBackHeight[selection] = false;
										Redo = kTRUE;
										inpvalid = kTRUE;
										break;
									case 'n':
										inpvalid = kTRUE;
										break;
									default:
										cout << "What's \"" << inp << "\"?? Try again." << endl;
									}
								} while(!inpvalid);
							} else {
								cout << "What do you mean with \"" << input 
										 << "\"? Try again." << endl;
							}
							
						}
					}
					timer->TurnOff();
				}
			} while(Redo);

			if (FirstStageAgain) {
				lout << "R";
			  --y;
				continue;
			}
			if (NextCrystal) {
				continue;
			}

			/***********************************************************************
       *                           third stage
       *                      fit all lines at once
       **********************************************************************/

			Redo = kFALSE;
			do {
				if (!BadCounts[0] && !BadCounts[1]) {
					// Generate new histogram
					MString Title(DetectorName);
					Title += ": Data of pixel: x=";
					Title += x;
					Title += ", y=";
					Title += y;
					allcrystal->SetTitle(Title);
					MString Name(DetectorName);
					Name += ":_Data_of_pixel:_x";
					Name += x;
					Name += "_y";
					Name += y;
					allcrystal->SetName(Name);
					if (FinalRangeLow1 == -1) {
						FinalRangeLow1 = RangeLow[0];
						FinalRangeHigh1 = RangeHigh[0];
						FinalRangeLow2 = RangeLow[1];
						FinalRangeHigh2 = RangeHigh[1];
						FinalEnergyFitMean = EnergyFitMean[0];
						FinalEnergyFitSigma = EnergyFitSigma[0];
						// FinalEnergyFitMean = (EnergyFitMean[1]-EnergyFitOffset[1])/
						//  xin1*xin + EnergyFitOffset[1];
						// FinalEnergyFitSigma = EnergyFitSigma[1];
						FinalEnergyFitHeight = EnergyFitHeight[0];
						FinalEnergyFitHeight1 = EnergyFitHeight[1];
						FinalEnergyFitHeight2 = EnergyFitHeight2[1];
						FinalEnergyFitHeightCompEdge = EnergyFitHeightCompEdge[0];
						FinalEnergyFitHeightCompEdge1 = EnergyFitHeightCompEdge[1];
						FinalEnergyFitHeightCompEdge2 = EnergyFitHeightCompEdge2[1];
						FinalEnergyFitHeightScatAbs = EnergyFitHeightScatAbs[0];
						FinalEnergyFitHeightScatAbs1 = EnergyFitHeightScatAbs[1];
						FinalEnergyFitHeightScatAbs2 = EnergyFitHeightScatAbs2[1];
						FinalEnergyFitHeightScatComp = EnergyFitHeightScatComp[0];
						FinalEnergyFitHeightScatComp1 = EnergyFitHeightScatComp[1];
						FinalEnergyFitHeightScatComp2 = EnergyFitHeightScatComp2[1];
						FinalEnergyFitOffset = EnergyFitOffset[0];
						//FinalEnergyFitOffset = EnergyFitOffset[1];
						FinalBackHeight1 = BackHeight[0];
						FinalBackHeight2 = BackHeight[1];
					}
					
					Int_t lowbin, highbin;
					crystal = Detectors[0]->GetCalibHist(x-1, y-1);
					lowbin = crystal->GetXaxis()->FindFixBin(FinalRangeLow1);
					highbin = crystal->GetXaxis()->FindFixBin(FinalRangeHigh1);
					for (Int_t i=0; i<crystal->GetNbinsX(); ++i) {
						if ( (i>=lowbin) && (i<=highbin) )
							allcrystal->SetBinContent(i, crystal->GetBinContent(i));
						else 
							allcrystal->SetBinContent(i, 0.0);
					}
					crystal = Detectors[1]->GetCalibHist(x-1, y-1);
					lowbin = crystal->GetXaxis()->FindFixBin(FinalRangeLow2);
					highbin = crystal->GetXaxis()->FindFixBin(FinalRangeHigh2);
					for (Int_t i=0; i<crystal->GetNbinsX(); ++i) {
						if ( (i>=lowbin) && (i<=highbin) )
							allcrystal->SetBinContent(i+1000, crystal->GetBinContent(i));
						else
							allcrystal->SetBinContent(i+1000, 0.0);
					}
					
					
					FitAll->SetParameter(0,  FinalEnergyFitMean);
					if (FixFinalEnergyFitMean)
						FitAll->FixParameter(0, FinalEnergyFitMean);
					else
						FitAll->SetParLimits(0, 
																 FinalEnergyFitMean -
																 5.0 * FinalEnergyFitSigma,
																 FinalEnergyFitMean +
																 5.0 * FinalEnergyFitSigma);
					
					FitAll->SetParameter(1,  FinalEnergyFitSigma);
					if (FixFinalEnergyFitSigma)
						FitAll->FixParameter(1, FinalEnergyFitSigma);
					else
						FitAll->SetParLimits(1, 0.0,
																 FinalEnergyFitSigma*4.0/3.0);
					
					FitAll->SetParameter(2,  FinalEnergyFitHeight);
					if (FixFinalEnergyFitHeight)
						FitAll->FixParameter(2, FinalEnergyFitHeight);
					else
						FitAll->ReleaseParameter(2);
					
					FitAll->SetParameter(3,  FinalEnergyFitHeight1);
					if (FixFinalEnergyFitHeight1)
						FitAll->FixParameter(3, FinalEnergyFitHeight1);
					else
						FitAll->ReleaseParameter(3);
					
					FitAll->SetParameter(4,  FinalEnergyFitHeight2);
					if (FixFinalEnergyFitHeight2)
						FitAll->FixParameter(4, FinalEnergyFitHeight2);
					else
						FitAll->ReleaseParameter(4);
					
					FitAll->SetParameter(5,  FinalEnergyFitHeightCompEdge);
					if (FixFinalEnergyFitHeightCompEdge)
						FitAll->FixParameter(5, FinalEnergyFitHeightCompEdge);
					else
						FitAll->SetParLimits(5, 0.0, 10000.0);
					
					FitAll->SetParameter(6,  FinalEnergyFitHeightCompEdge1);
					if (FixFinalEnergyFitHeightCompEdge1)
						FitAll->FixParameter(6, FinalEnergyFitHeightCompEdge1);
					else
						FitAll->SetParLimits(6, 0.0, 10000.0);
					
					FitAll->SetParameter(7,  FinalEnergyFitHeightCompEdge2);
					if (FixFinalEnergyFitHeightCompEdge2)
						FitAll->FixParameter(7, FinalEnergyFitHeightCompEdge2);
					else
						FitAll->SetParLimits(7, 0.0, 10000.0);
					
					FitAll->SetParameter(8,  FinalEnergyFitHeightScatAbs);
					if (FixFinalEnergyFitHeightScatAbs)
						FitAll->FixParameter(8, FinalEnergyFitHeightScatAbs);
					else
						FitAll->SetParLimits(8, 0.0, 10000.0);
					
					FitAll->SetParameter(9,  FinalEnergyFitHeightScatAbs1);
					if (FixFinalEnergyFitHeightScatAbs1)
						FitAll->FixParameter(9, FinalEnergyFitHeightScatAbs1);
					else
						FitAll->SetParLimits(9, 0.0, 10000.0);
					
					FitAll->SetParameter(10, FinalEnergyFitHeightScatAbs2);
					if (FixFinalEnergyFitHeightScatAbs2)
						FitAll->FixParameter(10, FinalEnergyFitHeightScatAbs2);
					else
						FitAll->SetParLimits(10, 0.0, 10000.0);
					
					FitAll->SetParameter(11, FinalEnergyFitHeightScatComp);
					if (FixFinalEnergyFitHeightScatComp)
						FitAll->FixParameter(11, FinalEnergyFitHeightScatComp);
					else
						FitAll->SetParLimits(11, 0.0, 10000.0);
					
					FitAll->SetParameter(12, FinalEnergyFitHeightScatComp1);
					if (FixFinalEnergyFitHeightScatComp1)
						FitAll->FixParameter(12, FinalEnergyFitHeightScatComp1);
					else
						FitAll->SetParLimits(12, 0.0, 10000.0);

					FitAll->SetParameter(13, FinalEnergyFitHeightScatComp2);
					if (FixFinalEnergyFitHeightScatComp2)
						FitAll->FixParameter(13, FinalEnergyFitHeightScatComp2);
					else
						FitAll->SetParLimits(13, 0.0, 10000.0);
					
					FitAll->SetParameter(14, FinalEnergyFitOffset);
					if (FixFinalEnergyFitOffset)
						FitAll->FixParameter(14, FinalEnergyFitOffset);
					else
						FitAll->ReleaseParameter(14);
					
					FitAll->FixParameter(15, FinalBackHeight1);
					FitAll->FixParameter(16, FinalBackHeight2);
					FitAll->FixParameter(17, FinalRangeHigh1);
					FitAll->FixParameter(18, FinalRangeLow2);
					FitAll->SetRange(FinalRangeLow1, FinalRangeHigh2+1000);
					
					testcanv->cd();
					allcrystal->SetStats(kFALSE);
					allcrystal->Draw();
					now.Now();
					cout << "Fitting (start at " << now.GetSQLString() << ")..." << endl;
					BadAFit = allcrystal->Fit(FitAll, "R");
					now.Now();
					cout << "... done (at " << now.GetSQLString() << ")" << endl;
					
					FinalEnergyFitMean = FitAll->GetParameter(0);
					FinalEnergyFitSigma = FitAll->GetParameter(1);
					FinalEnergyFitHeight = FitAll->GetParameter(2);
					FinalEnergyFitHeight1 = FitAll->GetParameter(3);
					FinalEnergyFitHeight2 = FitAll->GetParameter(4);
					FinalEnergyFitHeightCompEdge = FitAll->GetParameter(5);
					FinalEnergyFitHeightCompEdge1 = FitAll->GetParameter(6);
					FinalEnergyFitHeightCompEdge2 = FitAll->GetParameter(7);
					FinalEnergyFitHeightScatAbs = FitAll->GetParameter(8);
					FinalEnergyFitHeightScatAbs1 = FitAll->GetParameter(9);
					FinalEnergyFitHeightScatAbs2 = FitAll->GetParameter(10);
					FinalEnergyFitHeightScatComp = FitAll->GetParameter(11);
					FinalEnergyFitHeightScatComp1 = FitAll->GetParameter(12);
					FinalEnergyFitHeightScatComp2 = FitAll->GetParameter(13);
					FinalEnergyFitOffset = FitAll->GetParameter(14);
					FinalBackHeight1 = FitAll->GetParameter(15);
					FinalBackHeight2 = FitAll->GetParameter(16);
					FinalEnergyFitChiSquare = FitAll->GetChisquare();
					FitAll->GetRange(FinalRangeLow1, FinalRangeHigh2);
					FinalRangeLow2 = FitAll->GetParameter(17);
					FinalRangeHigh1 = FitAll->GetParameter(18);
					
					lout << "FA ";
					for (UInt_t i=0; i<19; ++i) {
						lout << FitAll->GetParameter(i) << " ";
					}
					lout << FinalRangeLow1 << " " << FinalRangeHigh2 << " "
							 << FitAll->GetChisquare() << " " << BadAFit << endl;
					
					// kurz anzeigen
					testcanv->Update();
				} // end if !badcounts

				Redo = kFALSE;
				
				// Wait for a key if necessary...
				
				Bool_t Wait = kFALSE;
				Bool_t BadCountsAll = kFALSE;
				
				switch (Stop) {
				case 1:
				case 2:
					for (UInt_t i=0; i<BadCounts.size(); ++i) {
						Wait |= BadEFit[i];
						BadCountsAll |= BadCounts[i];
					}
					break;
				case 4:
					Wait = kTRUE;
					break;
				}
				


				if (Wait) {
					for (;;) {
						cout << endl << " Peak position   ";
						cout << setw(11) << FinalEnergyFitMean;
						if (FixFinalEnergyFitMean) cout << " x ";
						
						cout << endl << " Sigma           ";
						cout << setw(11) << FinalEnergyFitSigma;
						if (FixFinalEnergyFitSigma) cout << " x ";
						
						cout << endl << " Height 1 peak 1 ";
						cout << setw(11) << FinalEnergyFitHeight;
						if (FixFinalEnergyFitHeight) cout << " x ";
						
						cout << endl << " Height 2 peak 1 ";
						cout << setw(11) << FinalEnergyFitHeight1;
						if (FixFinalEnergyFitHeight1) cout << " x ";

						cout << endl << " Height 2 peak 2 ";
						cout << setw(11) << FinalEnergyFitHeight2;
						if (FixFinalEnergyFitHeight2) cout << " x ";
					
						cout << endl << " Height 1 C.e. 1 ";
						cout << setw(11) << FinalEnergyFitHeightCompEdge;
						if (FixFinalEnergyFitHeightCompEdge) cout << " x ";

						cout << endl << " Height 2 C.e. 1 ";
						cout << setw(11) << FinalEnergyFitHeightCompEdge1;
						if (FixFinalEnergyFitHeightCompEdge1) cout << " x ";

						cout << endl << " Height 2 C.e. 2 ";
						cout << setw(11) << FinalEnergyFitHeightCompEdge2;
						if (FixFinalEnergyFitHeightCompEdge2) cout << " x ";

						cout << endl << " Height 1 A.sc. 1 ";
						cout << setw(11) << FinalEnergyFitHeightScatAbs;
						if (FixFinalEnergyFitHeightScatAbs) cout << " x ";
						
						cout << endl << " Height 2 A.sc. 1 ";
						cout << setw(11) << FinalEnergyFitHeightScatAbs1;
						if (FixFinalEnergyFitHeightScatAbs1) cout << " x ";
						
						cout << endl << " Height 2 A.sc. 2 ";
						cout << setw(11) << FinalEnergyFitHeightScatAbs2;
						if (FixFinalEnergyFitHeightScatAbs2) cout << " x ";
						
						cout << endl << " Height 1 C.sc. 1 ";
						cout << setw(11) << FinalEnergyFitHeightScatComp;
							if (FixFinalEnergyFitHeightScatComp) cout << " x ";
						
						cout << endl << " Height 2 C.sc. 1 ";
						cout << setw(11) << FinalEnergyFitHeightScatComp1;
							if (FixFinalEnergyFitHeightScatComp1) cout << " x ";

						cout << endl << " Height 2 C.sc. 2 ";
						cout << setw(11) << FinalEnergyFitHeightScatComp2;
							if (FixFinalEnergyFitHeightScatComp2) cout << " x ";

						cout << endl << " Offset           ";
						cout << setw(11) << FinalEnergyFitOffset;
						if (FixFinalEnergyFitOffset) cout << " x ";

						cout << endl << " BackHeight 1     ";
						cout << setw(11) << FinalBackHeight1;

						cout << endl << " BackHeight 2     ";
						cout << setw(11) << FinalBackHeight2;
					
						cout << endl << " Fit range low 1  ";
						cout << setw(11) << FinalRangeLow1;

						cout << endl << " Fit range high 1 ";
						cout << setw(11) << FinalRangeHigh1;

						cout << endl << " Fit range low 2  ";
						cout << setw(11) << FinalRangeLow2;

						cout << endl << " Fit range high 2 ";
						cout << setw(11) << FinalRangeHigh2;
	
						cout << endl << " Chi2            ";
						cout << setw(11) << FinalEnergyFitChiSquare;

						cout << endl;

						char input[10];
						MString inp;
						Bool_t inpvalid = kFALSE;
						int selection = -1;
						double inpvalue = -1;
						input[0]=0;
						timer->TurnOn();
						timer->Reset();
						cout << endl
								 << "Choose parameter to change:" << endl
								 << " pp: change peak position" << endl
								 << " si: change sigma" << endl
								 << " p0: change height of single photo peak" << endl
								 << " p1: change height of double photo peak 1" << endl
								 << " p2: change height of double photo peak 2" << endl
								 << " c0: change height of single compton edge" << endl
								 << " c1: change height of double compton edge 1" << endl
								 << " c2: change height of double compton edge 2" << endl
								 << " a0: change height of single absorbed scattered" << endl
								 << " a1: change height of double absorbed scattered 1" << endl
								 << " a2: change height of double absorbed scattered 2" << endl
								 << " s0: change height of single scattered compton" << endl
								 << " s1: change height of double scattered compton 1" << endl
								 << " s2: change height of double scattered compton 2" << endl
								 << " b0: change height of single background level" << endl
								 << " b1: change height of double background level" << endl
								 << " of: change offset" << endl
								 << " l0: change lower border of fit range single" << endl
								 << " l1: change lower border of fit range double" << endl
								 << " u0: change upper border of fit range single" << endl
								 << " u1: change upper border of fit range double" << endl
								 << " uf: unfix all parameters of line" << endl
								 << " f:  go back to first stage" << endl
								 << " n:  nothing at all" << endl
								 << " q:  quit" << endl << endl
								 << "After changing a parameter, the fits are redone. " 
								 << endl 
								 << "If everything is ok, just type <return>" << endl
								 << "Your choice (pp/si/p0/p1/p2/c0/c1/c2/a0/a1/a2/s0/s1/"
								 << "s2/b1/b2/of/l0/l1/u0/u1/un/n/q/):" << endl;
						cin.get(input,9);
						cin.clear();
						cin.ignore(numeric_limits<std::streamsize>::max(), '\n');
						if (input[0]==0) {
								break;
						} else if (input[0]=='n') {
							Redo = kFALSE;
							break;
						} else if (input[0]=='q') {
							Exit = kTRUE;
							Redo = kFALSE;
							break;
						} else if (input[0]=='r') {
							Redo = kTRUE;
							break;
						} else if (input[0]=='f') {
							Redo = kFALSE;
							FirstStageAgain = kTRUE;
							break;
						}

						switch(input[0]) {
						case 'p':
							switch(input[1]) {
							case 'p':
								cout << "New peak position: "
										 << "(value of f to toggle parameter fix flag) ";
								cin >> inp;
								cin.clear();
								cin.ignore(numeric_limits<std::streamsize>::max(), '\n');
								if (inp.Data()[0] == 'f') {
									FixFinalEnergyFitMean =	!FixFinalEnergyFitMean;
									Redo = kTRUE;
								} else { 
									istringstream InputStream(inp.Data());
									InputStream >> inpvalue;
									if(!InputStream.bad() && !InputStream.fail() && 
										 (inpvalue > 0) && 
										 (inpvalue <= Detectors[0]->
											GetCalibHist(x-1,y-1)->GetXaxis()->GetXmax())) {
										FinalEnergyFitMean = inpvalue;
										Redo = kTRUE;
									} else {
										cout << "Not a number or out of range - ignored!" << endl;
									}
								}
								inpvalid = kTRUE;
								break;
							case '0':
								cout << "New peak height single: "
										 << "(value of f to toggle parameter fix flag) ";
								cin >> inp;
								cin.clear();
								cin.ignore(numeric_limits<std::streamsize>::max(), '\n');
								if (inp.Data()[0] == 'f') {
									FixFinalEnergyFitHeight =	!FixFinalEnergyFitHeight;
									Redo = kTRUE;
								} else { 
									istringstream InputStream(inp.Data());
									InputStream >> inpvalue;
									if(!InputStream.bad() && !InputStream.fail() && 
										 (inpvalue > 0)) {
										FinalEnergyFitMean = inpvalue;
										Redo = kTRUE;
									} else {
										cout << "Not a number or out of range - ignored!" << endl;
									}
								}
								inpvalid = kTRUE;
								break;
							case '1':
								cout << "New peak height 1 double: "
										 << "(value of f to toggle parameter fix flag) ";
								cin >> inp;
								cin.clear();
								cin.ignore(numeric_limits<std::streamsize>::max(), '\n');
								if (inp.Data()[0] == 'f') {
									FixFinalEnergyFitHeight1 =	!FixFinalEnergyFitHeight1;
									Redo = kTRUE;
								} else { 
									istringstream InputStream(inp.Data());
									InputStream >> inpvalue;
									if(!InputStream.bad() && !InputStream.fail() && 
										 (inpvalue > 0)) {
										FinalEnergyFitHeight1 = inpvalue;
										Redo = kTRUE;
									} else {
										cout << "Not a number or out of range - ignored!" << endl;
									}
								}
								inpvalid = kTRUE;
								break;
							case '2':
								cout << "New peak height 2 double: "
										 << "(value of f to toggle parameter fix flag) ";
								cin >> inp;
								cin.clear();
								cin.ignore(numeric_limits<std::streamsize>::max(), '\n');
								if (inp.Data()[0] == 'f') {
									FixFinalEnergyFitHeight2 =	!FixFinalEnergyFitHeight2;
									Redo = kTRUE;
								} else { 
									istringstream InputStream(inp.Data());
									InputStream >> inpvalue;
									if(!InputStream.bad() && !InputStream.fail() && 
										 (inpvalue > 0)) {
										FinalEnergyFitHeight2 = inpvalue;
										Redo = kTRUE;
									} else {
										cout << "Not a number or out of range - ignored!" << endl;
									}
								}
								inpvalid = kTRUE;
								break;
							default:
								cout << "What's \"" << input << "\" ? Input ignored!" << endl;
							}
							break;
						case 's':
							switch(input[1]) {
							case 'i':
								cout << "New sigma: (value of f to toggle parameter fix flag)";
								cin >> inp;
								cin.clear();
								cin.ignore(numeric_limits<std::streamsize>::max(), '\n');
								if (inp.Data()[0] == 'f') {
									FixFinalEnergyFitSigma = !FixFinalEnergyFitSigma;
									Redo = kTRUE;
								} else { 
									istringstream InputStream(inp.Data());
									InputStream >> inpvalue;
									cout << "inpvalue: " << inpvalue << endl;
									if(!InputStream.bad() && !InputStream.fail() && 
										 (inpvalue > 0)) {
										FinalEnergyFitSigma = inpvalue;
										Redo = kTRUE;
									} else {
										cout << "Not a number or out of range - ignored!" << endl;
									}
								}
								inpvalid = kTRUE;
								break;
							case '0':
								cout << "New height of scattered compton single: "
										 << "(value of f to toggle parameter fix flag) ";
								cin >> inp;
								cin.clear();
								cin.ignore(numeric_limits<std::streamsize>::max(), '\n');
								if (inp.Data()[0] == 'f') {
									FixFinalEnergyFitHeightScatComp =	
										!FixFinalEnergyFitHeightScatComp;
									Redo = kTRUE;
								} else { 
									istringstream InputStream(inp.Data());
									InputStream >> inpvalue;
									if(!InputStream.bad() && !InputStream.fail() && 
										 (inpvalue > 0)) {
										FinalEnergyFitHeightScatComp = inpvalue;
										Redo = kTRUE;
									} else {
										cout << "Not a number or out of range - ignored!" << endl;
									}
								}
								inpvalid = kTRUE;
								break;
							case '1':
								cout << "New height of scattered compton 1 double: "
										 << "(value of f to toggle parameter fix flag) ";
								cin >> inp;
								cin.clear();
								cin.ignore(numeric_limits<std::streamsize>::max(), '\n');
								if (inp.Data()[0] == 'f') {
									FixFinalEnergyFitHeightScatComp1 = 
										!FixFinalEnergyFitHeightScatComp1;
									Redo = kTRUE;
								} else { 
									istringstream InputStream(inp.Data());
									InputStream >> inpvalue;
									if(!InputStream.bad() && !InputStream.fail() && 
										 (inpvalue > 0)) {
										FinalEnergyFitHeightScatComp = inpvalue;
										Redo = kTRUE;
									} else {
										cout << "Not a number or out of range - ignored!" << endl;
									}
								}
								inpvalid = kTRUE;
								break;
							case '2':
								cout << "New height of scattered compton 2 double: "
										 << "(value of f to toggle parameter fix flag) ";
								cin >> inp;
								cin.clear();
								cin.ignore(numeric_limits<std::streamsize>::max(), '\n');
								if (inp.Data()[0] == 'f') {
									FixFinalEnergyFitHeightScatComp2 = 
										!FixFinalEnergyFitHeightScatComp2;
									Redo = kTRUE;
								} else { 
									istringstream InputStream(inp.Data());
									InputStream >> inpvalue;
									if(!InputStream.bad() && !InputStream.fail() && 
										 (inpvalue > 0)) {
										FinalEnergyFitHeightScatComp2 = inpvalue;
										Redo = kTRUE;
									} else {
										cout << "Not a number or out of range - ignored!" << endl;
									}
								}
								inpvalid = kTRUE;
								break;
							default:
								cout << "What's \"" << input << "\" ? Input ignored!" << endl;
							}
							break;
						case 'c':
							switch(inp.Data()[1]) {
							case '0':
								cout << "New height of compton edge single: "
										 << "(value of f to toggle parameter fix flag) ";
								cin >> inp;
								cin.clear();
								cin.ignore(numeric_limits<std::streamsize>::max(), '\n');
								if (inp.Data()[0] == 'f') {
									FixFinalEnergyFitHeightCompEdge = 
										!FixFinalEnergyFitHeightCompEdge;
									Redo = kTRUE;
								} else { 
									istringstream InputStream(inp.Data());
									InputStream >> inpvalue;
									if(!InputStream.bad() && !InputStream.fail() && 
										 (inpvalue > 0)) {
										FinalEnergyFitHeightCompEdge = inpvalue;
										Redo = kTRUE;
									} else {
										cout << "Not a number or out of range - ignored!" << endl;
									}
								}
								inpvalid = kTRUE;
								break;
							case '1':
								cout << "New height of compton edge 1 double: "
										 << "(value of f to toggle parameter fix flag) ";
								cin >> inp;
								cin.clear();
								cin.ignore(numeric_limits<std::streamsize>::max(), '\n');
								if (inp.Data()[0] == 'f') {
									FixFinalEnergyFitHeightCompEdge1 = 
										!FixFinalEnergyFitHeightCompEdge1;
									Redo = kTRUE;
								} else { 
									istringstream InputStream(inp.Data());
									InputStream >> inpvalue;
									if(!InputStream.bad() && !InputStream.fail() && 
										 (inpvalue > 0)) {
										FinalEnergyFitHeightCompEdge1 = inpvalue;
										Redo = kTRUE;
									} else {
										cout << "Not a number or out of range - ignored!" << endl;
									}
								}
								inpvalid = kTRUE;
								break;
							case '2':
								cout << "New height of compton edge 2 double: "
										 << "(value of f to toggle parameter fix flag) ";
								cin >> inp;
								cin.clear();
								cin.ignore(numeric_limits<std::streamsize>::max(), '\n');
								if (inp.Data()[0] == 'f') {
									FixFinalEnergyFitHeightCompEdge2 = 
										!FixFinalEnergyFitHeightCompEdge2;
									Redo = kTRUE;
								} else { 
									istringstream InputStream(inp.Data());
									InputStream >> inpvalue;
									if(!InputStream.bad() && !InputStream.fail() && 
										 (inpvalue > 0)) {
										FinalEnergyFitHeightCompEdge2 = inpvalue;
										Redo = kTRUE;
									} else {
										cout << "Not a number or out of range - ignored!" << endl;
									}
								}
								inpvalid = kTRUE;
								break;
							default:
								cout << "What's \"" << input << "\" ? Input ignored!" << endl;
							}
							break;
						case 'a':
							switch(inp.Data()[1]) {
							case '0':
								cout << "New height of absorbed scatter single: "
										 << "(value of f to toggle parameter fix flag) ";
								cin >> inp;
								cin.clear();
								cin.ignore(numeric_limits<std::streamsize>::max(), '\n');
								if (inp.Data()[0] == 'f') {
									FixFinalEnergyFitHeightScatAbs = 
										!FixFinalEnergyFitHeightScatAbs;
									Redo = kTRUE;
								} else { 
									istringstream InputStream(inp.Data());
									InputStream >> inpvalue;
									if(!InputStream.bad() && !InputStream.fail() && 
										 (inpvalue > 0)) {
										FinalEnergyFitHeightScatAbs = inpvalue;
										Redo = kTRUE;
									} else {
										cout << "Not a number or out of range - ignored!" << endl;
									}
								}
								inpvalid = kTRUE;
								break;
							case '1':
								cout << "New height of absorbed scatter 1 double: "
										 << "(value of f to toggle parameter fix flag) ";
								cin >> inp;
								cin.clear();
								cin.ignore(numeric_limits<std::streamsize>::max(), '\n');
								if (inp.Data()[0] == 'f') {
									FixFinalEnergyFitHeightScatAbs1 = 
										!FixFinalEnergyFitHeightScatAbs1;
									Redo = kTRUE;
								} else { 
									istringstream InputStream(inp.Data());
									InputStream >> inpvalue;
									if(!InputStream.bad() && !InputStream.fail() && 
										 (inpvalue > 0)) {
										FinalEnergyFitHeightScatAbs1 = inpvalue;
										Redo = kTRUE;
									} else {
										cout << "Not a number or out of range - ignored!" << endl;
									}
								}
								inpvalid = kTRUE;
								break;
							case '2':
								cout << "New height of absorbed scatter 2 double: "
										 << "(value of f to toggle parameter fix flag) ";
								cin >> inp;
								cin.clear();
								cin.ignore(numeric_limits<std::streamsize>::max(), '\n');
								if (inp.Data()[0] == 'f') {
									FixFinalEnergyFitHeightScatAbs2 = 
										!FixFinalEnergyFitHeightScatAbs2;
									Redo = kTRUE;
								} else { 
									istringstream InputStream(inp.Data());
									InputStream >> inpvalue;
									if(!InputStream.bad() && !InputStream.fail() && 
										 (inpvalue > 0)) {
										FinalEnergyFitHeightScatAbs2 = inpvalue;
										Redo = kTRUE;
									} else {
										cout << "Not a number or out of range - ignored!" << endl;
									}
								}
								inpvalid = kTRUE;
								break;
							default:
								cout << "What's \"" << input << "\" ? Input ignored!" << endl;
							}
							break;
						case 'b':
							switch (input[1]) {
							case '0': {
								cout << "new background level 1: ";
								cin >> inp;
								cin.clear();
								cin.ignore(numeric_limits<std::streamsize>::max(), '\n');
								istringstream InputStream(inp.Data());
								inpvalue = -1;
								InputStream >> inpvalue;
								if (!InputStream.bad() && !InputStream.fail() && 
										(inpvalue >= 0)) {
									FinalBackHeight1 = inpvalue;
									Redo = kTRUE;
								} else {
									cout << "No valid number! Nothing changed." << endl;
								}
								inpvalid = kTRUE; }
								break; 
							case '1': {
								cout << "new background level 2: ";
								cin >> inp;
								cin.clear();
								cin.ignore(numeric_limits<std::streamsize>::max(), '\n');
								istringstream InputStream(inp.Data());
								inpvalue = -1;
								InputStream >> inpvalue;
								if (!InputStream.bad() && !InputStream.fail() && 
										(inpvalue >= 0)) {
									FinalBackHeight2 = inpvalue;
									Redo = kTRUE;
								} else {
									cout << "No valid number! Nothing changed." << endl;
								}
								inpvalid = kTRUE; }
								break;
							default:
								cout << "What's \"" << input << "\" ? Input ignored!" << endl;
							}
							break;
						case 'l':
							switch (input[1]) {
							case '0': {
								cout << "new lower border for single peak spectrum: ";
								cin >> inp;
								cin.clear();
								cin.ignore(numeric_limits<std::streamsize>::max(), '\n');
								istringstream InputStream(inp.Data());
								inpvalue = -1;
								InputStream >> inpvalue;
								if (!InputStream.bad() && !InputStream.fail() && 
										(inpvalue >= 0)) {
									FinalRangeLow1 = inpvalue;
									Redo = kTRUE;
								} else {
									cout << "No valid number! Nothing changed." << endl;
								}
								inpvalid = kTRUE; }
								break;
							case '1': {
								cout << "new lower border for double peak spectrum: ";
								cin >> inp;
								cin.clear();
								cin.ignore(numeric_limits<std::streamsize>::max(), '\n');
								istringstream InputStream(inp.Data());
								inpvalue = -1;
								InputStream >> inpvalue;
								if (!InputStream.bad() && !InputStream.fail() && 
										(inpvalue >= 0)) {
									FinalRangeLow2 = inpvalue;
									Redo = kTRUE;
								} else {
									cout << "No valid number! Nothing changed." << endl;
								}
								inpvalid = kTRUE; }
								break;
							default:
								cout << "What's \"" << input << "\" ? Input ignored!" << endl;
							}
							break;
						case 'u':
							switch (input[1]) {
							case '0': {
								cout << "new upper border for single peak spectrum: ";
								cin >> inp;
								cin.clear();
								cin.ignore(numeric_limits<std::streamsize>::max(), '\n');
								istringstream InputStream(inp.Data());
								inpvalue = -1;
								InputStream >> inpvalue;
								if (!InputStream.bad() && !InputStream.fail() && 
										(inpvalue >= 0)) {
									FinalRangeHigh1 = inpvalue;
									Redo = kTRUE;
								} else {
									cout << "No valid number! Nothing changed." << endl;
								}
								inpvalid = kTRUE; }
								break;
							case '1': {
								cout << "new upper border for double peak spectrum: ";
								cin >> inp;
								cin.clear();
								cin.ignore(numeric_limits<std::streamsize>::max(), '\n');
								istringstream InputStream(inp.Data());
								inpvalue = -1;
								InputStream >> inpvalue;
								if (!InputStream.bad() && !InputStream.fail() && 
										(inpvalue >= 0)) {
									FinalRangeHigh2 = inpvalue;
									Redo = kTRUE;
								} else {
									cout << "No valid number! Nothing changed." << endl;
								}
								inpvalid = kTRUE; }
								break;
							case 'f':
								cout << "releasing all parameters ";
								FixFinalEnergyFitMean = false;
								FixFinalEnergyFitSigma = false;;
								FixFinalEnergyFitHeight = false;;
								FixFinalEnergyFitHeight1 = false;;
								FixFinalEnergyFitHeight2 = false;;
								FixFinalEnergyFitHeightCompEdge = false;;
								FixFinalEnergyFitHeightCompEdge1 = false;;
								FixFinalEnergyFitHeightCompEdge2 = false;;
								FixFinalEnergyFitHeightScatAbs = false;;
								FixFinalEnergyFitHeightScatAbs1 = false;;
								FixFinalEnergyFitHeightScatAbs2 = false;;
								FixFinalEnergyFitHeightScatComp = false;;
								FixFinalEnergyFitHeightScatComp1 = false;;
								FixFinalEnergyFitHeightScatComp2 = false;;
								FixFinalEnergyFitOffset = false;;
								inpvalid = kTRUE;
								break;
							default:
								cout << "What's \"" << input << "\" ? Input ignored!" << endl;
							}
							break;
						case 'o':
							cout << "New offset: (value of f to toggle parameter fix flag) ";
							cin >> inp;
							cin.clear();
							cin.ignore(numeric_limits<std::streamsize>::max(), '\n');
							if (inp.Data()[0] == 'f') {
								FixEnergyFitOffset[selection] = 
									!FixEnergyFitOffset[selection];
								Redo = kTRUE;
							} else { 
								istringstream InputStream(inp.Data());
								InputStream >> inpvalue;
								if(!InputStream.bad() && !InputStream.fail()) {
									EnergyFitOffset[selection] = inpvalue;
									Redo = kTRUE;
								} else {
									cout << "Not a number or 'f' - ignored!" << endl;
								}
							}
							inpvalid = kTRUE;
							break;
						default:
							cout << "What do you mean with \"" << input 
									 << "\"? Try again." << endl;
						}

					}
					timer->TurnOff();
				}
			} while(Redo);

			if (FirstStageAgain) {
				lout << "R";
				--y;
				continue;
			}


			/********************************************************************
			 *                  writing results to csv file
			 *******************************************************************/
			
			Double_t Final_Sigma = FinalEnergyFitSigma;
			Double_t Final_Gradient = Energy[0] / 
				(FinalEnergyFitMean - FinalEnergyFitOffset);
			Double_t Final_Offset = - Final_Gradient * FinalEnergyFitOffset;
			Double_t Final_Overflow = 0.0;
			for (UInt_t i=0; i<Overflow.size(); ++i) {
				Final_Overflow += Overflow[i];
			}
			Final_Overflow /= Overflow.size();

			if (FinalEnergyFitMean > 0) {
				fout << "CB " << x << " " << y << " " << Final_Offset << " " 
						 << Final_Gradient << endl
						 << "SI " << x << " " << y << " " << Final_Sigma << endl
						 << "OF " << x << " " << y << " " << Final_Overflow << endl		
						 << "TH " << x << " " << y << " " << TThresholds[x][y] << endl;

				// calculate averaged sigma
				double SigmakeV = Final_Sigma * Final_Gradient;
				AvgSigma = (NAvgSigma * AvgSigma + SigmakeV) / (NAvgSigma+1);
				++NAvgSigma;
			} else {
				fout << "CB " << x << " " << y << " -1 -1" << endl;
			}

			if (Exit) break;
		}   // End y-loop
		if (Exit) break;
	}     // End x-loop

  fout << "AS " << Energy[0] << " " << AvgSigma << endl;

	lout.close();
	fout.close();

	return 1;
}


/******************************************************************************
  Find limits of Peak
******************************************************************************/

void FindPeakBorders(TH1S* Hist, Double_t& LowerBorder, Double_t& UpperBorder)
{
  Int_t		PThresh;			        // Treshhold for accepting peak
  Float_t MinRelative = 0.1 ;		// the Maximum must have at least 
	//                               MinRelative * Max(hist)
	Int_t lowest;                 // lowest bin in hist
	Int_t lower;                  // lower border in bins
	Int_t upper;                  // upper border in bins
  Double_t max;                 // value of (local) maximum
  Int_t binmax;                 // position of maximum in bins

  // In a smoothed copy of the histogram the finding of a peak is easier
  TH1D* smoothed = 0;
  smoothed = (TH1D*) (Hist->Clone());
  smoothed->SetName("smoothed"); 
  smoothed->Smooth(20);
  if ( (UpperBorder != -1) && (LowerBorder != -1) ) {
    smoothed->GetXaxis()->SetRangeUser(LowerBorder, UpperBorder);
  }

  // Set threshold for recognizing a peak
  PThresh = (Int_t) (smoothed->GetMaximum() * MinRelative);
	
  // Determine the first maximum starting from high energies

  upper  = smoothed->GetXaxis()->GetLast();
  lowest = smoothed->GetXaxis()->GetFirst();
	
  // Upper limit of max, when spec goes above PThresh the first time
  while ( (smoothed->GetBinContent(upper) < PThresh) && (upper > lowest)) 
		upper--;

  // Now search for (local) max
  lower = upper;
  max = smoothed->GetBinContent(lower);
  binmax = lower;

  // The peak is isolated, if we get two adjacent steps down
  Int_t downct = 0;  // counter, how often we climb down while going lower
  Double_t mem1;     // memories for values
  Double_t mem2 = max;

  while ( lower > lowest && downct < 2 ) {
    if ( (mem1 = smoothed->GetBinContent(--lower)) >= mem2) {
      downct = 0;
      if ( mem1 >= max ) {
				max = mem1;
				binmax = lower;
      }
    } else {
      ++downct;
    }
    mem2=mem1;
  }

  // Climb down to half of max on high side -> HWHM
  Int_t hwhm;
  Int_t mem3;
  for (mem3 = binmax; 
			 (smoothed->GetBinContent(mem3)>0.5*max) && mem3<=upper; 
			 ++mem3);
  upper=mem3;
  hwhm = (upper - binmax);

  // Climb down on low side as far as possible
  mem2 = max;
  for (mem3=binmax-1; 
			 mem3>=(binmax-hwhm) && (mem1=smoothed->GetBinContent(mem3))<=mem2;
			 --mem3) {
    mem2 = mem1;
  }
  lower = mem3+1;

	LowerBorder = Hist->GetBinCenter(lower);
	UpperBorder = Hist->GetBinCenter(upper);
}


/******************************************************************************
 * Fit the first peak in hist starting from high energies.
 * Return kFALSE, if software noise level reaches photopeak 
 *****************************************************************************/
Bool_t FitPeak(TH1S* Hist, TF1* FitFunc, 
							 Double_t& Mean, Double_t& Sigma, Double_t& Height,
							 Double_t& LowerBorder, Double_t& UpperBorder)
{
  Bool_t retval = kTRUE;

  // Find photopeak in in histogram
  
	//	if ((LowerBorder == -1) || (UpperBorder == -1)) { 
		FindPeakBorders(Hist, LowerBorder, UpperBorder);
		//	}
	cout << "LowerBorder=" << LowerBorder << " UpperBorder=" << UpperBorder 
			 << endl;
	cout << "Gauss Mean: " << FitFunc->GetParameter(1) << "; Sigma: " 
			 << FitFunc->GetParameter(2)
			 << "; Height: " << FitFunc->GetParameter(0) << endl;
	FitFunc->SetRange(LowerBorder, UpperBorder);
  Hist->Fit(FitFunc, "R");
  Mean = FitFunc->GetParameter(1);
  Sigma = FitFunc->GetParameter(2);
	Height = FitFunc->GetParameter(0);

	cout << "Gauss Mean: " << Mean << "; Sigma: " << Sigma 
			 << "; Height: " << Height << endl;

  // Find software threshold for noise
  Int_t NoiseThreshBin;
  Axis_t NoiseThresh;
  for (NoiseThreshBin = Hist->GetXaxis()->GetFirst();
       NoiseThreshBin < Hist->GetXaxis()->GetLast();
       ++NoiseThreshBin) {
    if (Hist->GetBinContent(NoiseThreshBin) > 5) break;
  }
  NoiseThresh = Hist->GetBinCenter(NoiseThreshBin);

  // Photopeak truncated?
  if ( (Mean - Sigma) > NoiseThresh) {
    FitFunc->SetLineColor(3);
  } else {
    FitFunc->SetLineColor(2);
    cout << "Photo peak NOT accepted, because photo peak is truncated!" 
				 << endl;
    retval = kFALSE;
  }
  FitFunc->Draw("LSAME");

  return retval;
}


/******************************************************************************
 * Fitfunctions 
 *****************************************************************************/

Double_t FitAllLines(Double_t * ADCvalue, Double_t * par) {
	// par[0]  = "PhotopeakPosition", 
	// par[1]  = "Sigma", 
	// par[2]  = "HeightOfPhotopeak",
	// par[3]  = "HeightOfPhotopeak1",
	// par[4]  = "HeightOfPhotopeak2",
	// par[5]  = "HeightOfComptonEdge",
	// par[6]  = "HeightOfComptonEdge1",
	// par[7]  = "HeightOfComptonEdge2",
	// par[8]  = "HeightOfScatterAbsorbed",
	// par[9]  = "HeightOfScatterAbsorbed1",
	// par[10] = "HeightOfScatterAbsorbed2",
	// par[11] = "HeightOfScatterCompton",
	// par[12] = "HeightOfScatterCompton1",
 	// par[13] = "HeightOfScatterCompton2",
	// par[14] = "Offset", 
	// par[15] = "Background",
	// par[16] = "Background1",
	// par[17] = "range1high",
	// par[18] = "range2low"

	Double_t retval = 0.0;
	Double_t para[12];
	Double_t ADC = ADCvalue[0] - 1000;
	if (ADCvalue[0] < par[17]) {
		para[0] = par[0];
		para[1] = par[1];
		para[2] = par[2];
		para[3] = par[5];
		para[4] = par[8];
		para[5] = par[11];
		para[6] = par[14];
		para[7] = par[15];
		retval = Fit1Line(ADCvalue, para);
	} else if ((ADCvalue[0]-1000) > par[18]) {
		para[0]  = (par[0]-par[14])/xin*xin1 + par[14];
		para[1]  = par[1];
		para[2]  = par[3];
		para[3]  = par[4];
		para[4]  = par[6];
		para[5]  = par[7];
		para[6]  = par[9];
		para[7]  = par[10];
		para[8]  = par[12];
		para[9]  = par[13];
		para[10] = par[14];
		para[11] = par[16];
		retval = Fit2Lines(&ADC, para);
	}
	return retval;
}

Double_t FitAllLines2(Double_t * ADCvalue, Double_t * par) {
	// par[0]  = "PhotopeakPosition", 
	// par[1]  = "Sigma", 
	// par[2]  = "HeightOfPhotopeak",
	// par[3]  = "HeightOfPhotopeak1",
	// par[4]  = "HeightOfPhotopeak2",
	// par[5]  = "HeightOfComptonEdge",
	// par[6]  = "HeightOfComptonEdge1",
	// par[7]  = "HeightOfComptonEdge2",
	// par[8]  = "HeightOfScatterAbsorbed",
	// par[9]  = "HeightOfScatterAbsorbed1",
	// par[10] = "HeightOfScatterAbsorbed2",
	// par[11] = "HeightOfScatterCompton",
	// par[12] = "HeightOfScatterCompton1",
 	// par[13] = "HeightOfScatterCompton2",
	// par[14] = "Offset",
	// par[15] = "Offset2", 
	// par[16] = "Background",
	// par[17] = "Background1",
	// par[18] = "range1high",
	// par[19] = "range2low"

	Double_t retval = 0.0;
	Double_t para[12];
	Double_t ADC = ADCvalue[0] - 1000;
	if (ADCvalue[0] < par[18]) {
		para[0] = par[0];
		para[1] = par[1];
		para[2] = par[2];
		para[3] = par[5];
		para[4] = par[8];
		para[5] = par[11];
		para[6] = par[14];
		para[7] = par[16];
		retval = Fit1Line(ADCvalue, para);
	} else if ((ADCvalue[0]-1000) > par[19]) {
		para[0]  = (par[0]-par[15])/xin*xin1 + par[15];
		para[1]  = par[1];
		para[2]  = par[3];
		para[3]  = par[4];
		para[4]  = par[6];
		para[5]  = par[7];
		para[6]  = par[9];
		para[7]  = par[10];
		para[8]  = par[12];
		para[9]  = par[13];
		para[10] = par[15];
		para[11] = par[17];
		retval = Fit2Lines(&ADC, para);
	}
	return retval;
}

Double_t Fit1Line(Double_t * ADCvalue, Double_t * par) {
	// par[0] = "PhotopeakPosition", 
	// par[1] = "Sigma", 
	// par[2] = "HeightOfPhotopeak",
	// par[3] = "HeightOfComptonEdge",
	// par[4] = "HeightOfScatterAbsorbed",
	// par[5] =	"HeightOfScatterCompton", 
	// par[6] = "Offset", 
	// par[7] = "Background"
	//no more:
	// par[7] = "HeightOfNoisePeak",
	// par[8] = "HeightOfDoubleCompton"
	// Deposited energy in detector in units of electron masses
	Double_t xout = (ADCvalue[0]-par[6])*xin/(par[0]-par[6]);
	Double_t par1[7]={xout, 
										par[1]*xin/(par[0]-par[6]), 
										xin, 
										par[3], 
										par[4], 
										par[5], 
										0.0};
	
	return ( par[7] + 
					 par[2] * Gauss(ADCvalue[0], par[0], par[1]) + 
					 ConvF(par1) // <-- Grenzen enger!!!!!
					 //			 + par[7] * Gauss(ADCvalue[0], 0, par[1] ));
					 );
}

Double_t ConvF(Double_t *par) {
	// par[0] = "xout = Eout/511", 
	// par[1] = "Sigma in unit of xout", 
	// par[2] = "xin = 662/511 for Cs137",
	// par[3] = "HeightOfComptonEdge",
	// par[4] = "HeightOfScatterAbsorbed",
	// par[5] =	"HeightOfScatterCompton", 
	// par[6] = "HeightOfDoubleCompton"

	Int_t NBins = 1000;
	Double_t xmin = 0.0;
	Double_t xmax = par[2];
	Double_t x;
	Double_t xstep = (xmax-xmin)/NBins;
	Double_t retval = 0.0;

	for (x=xmin; x<xmax; x+=xstep) {
		retval += Gauss( par[0], x, par[1]) *
			(par[3] * ComptonFunction(x, par[2])
			 + par[4] * ScatteredPhotoFunction(x, par[2])
			 + par[5] * ScatteredComptonFunction(x, par[2])
			 // + par[6] * DoubleComptonFunction(x, par[2]) 
			 );
	}
	return (retval * xstep / (6.283185*par[1]*par[1]) );
}

Double_t Fit2Lines(Double_t * ADCvalue, Double_t * par) {
	// par[0]  = "PhotopeakPosition1", 
	// par[1]  = "Sigma",
	// par[2]  = "HeightOfPhotopeak1",
	// par[3]  = "HeightOfPhotopeak2",
	// par[4]  = "HeightOfComptonEdge1",
	// par[5]  = "HeightOfComptonEdge2",
	// par[6]  = "HeightOfScatterAbsorbed1",
	// par[7]  = "HeightOfScatterAbsorbed2",
	// par[8]  = "HeightOfScatterCompton1",
 	// par[9]  = "HeightOfScatterCompton2",
	// par[10] = "Offset"
	// par[11] = "Background"
	// PhotopeakPosition2 determined by xin1 and xin2!

	// Deposited energy in detector in units of electron masses
	Double_t xout = (ADCvalue[0]-par[10])*xin1/(par[0]-par[10]);
	// Photopeak position of second peak
	Double_t PP2 = (par[0]-par[10])/xin1 * xin2 + par[10]; 
	Double_t par1[10]={xout, 
										 par[1]*xin1/(par[0]-par[10]), 
										 xin1,
										 xin2, 
										 par[4], 
										 par[5], 
										 par[6], 
										 par[7],
										 par[8],
										 par[9]};
	
	return ( par[11] 
					 + par[2] * Gauss(ADCvalue[0], par[0], par[1])
					 + par[3] * Gauss(ADCvalue[0], PP2, par[1])
					 + ConvF2(par1) // <-- Grenzen enger!!!!!
					 );
}

Double_t ConvF2(Double_t *par) {
	// par[0] = "xout = Eout/511", 
	// par[1] = "Sigma in unit of xout", 
	// par[2] = "xin1 = 511/511 for Na22",
	// par[3] = "xin2 = 1274/511 for Na22"
	// par[4] = "HeightOfComptonEdge1",
	// par[5] = "HeightOfComptonEdge2",
	// par[6] = "HeightOfScatterAbsorbed1",
	// par[7] = "HeightOfScatterAbsorbed2",
	// par[8] =	"HeightOfScatterCompton1",
	// par[9] =	"HeightOfScatterCompton2"

	Int_t NBins = 1000;
	Double_t xmin = 0;
	Double_t xmax = (par[2] > par[3]) ? par[2] : par[3];
	Double_t x;
	Double_t xstep = (xmax-xmin)/NBins;
	Double_t retval = 0.0;

	for (x=xmin; x<xmax; x+=xstep) {
		retval += Gauss( par[0], x, par[1]) *
			(par[4] * ComptonFunction(x, par[2])
			 + par[5] * ComptonFunction(x, par[3])
			 + par[6] * ScatteredPhotoFunction(x, par[2])
			 + par[7] * ScatteredPhotoFunction(x, par[3])
			 + par[8] * ScatteredComptonFunction1(x, par[2])
			 + par[9] * ScatteredComptonFunction2(x, par[3])
			 );
	}
	return (retval * xstep / (6.283185*par[1]*par[1]) );
}

Double_t ScatterFunction(Double_t xo, Double_t xi) {
	if ( (xo >= xi/(1.0+2.0*xi)) && (xo<=xi)) {
		return ( ((xi-2.0)/xo + (xo+2.0)/xi 
							+ (1.0/xi - 1.0/xo)*(1.0/xi - 1.0/xo))/(xi*xi));
	} else {
		return 0.0;
	}
}

Double_t ScatteredPhotoFunction(Double_t xo, Double_t xi) {
	if ( (xo >= xi/(1.0+2.0*xi)) && (xo<=xi)) {
		return ( ((xi-2.0)/xo + (xo+2.0)/xi 
							+ (1.0/xi - 1.0/xo)*(1.0/xi - 1.0/xo))/(xi*xi * pow(xo,3.0)));
	} else {
		return 0.0;
	}
}

Double_t ComptonFunction(Double_t xo, Double_t xi) {
	if ( (xo>=0) && (xo<= 2.0*xi/(2.0+1.0/xi)) ) {
		return ( (1.0/(1.0-xo/xi)*(1.0-2.0/xi-2.0/(xi*xi)+1.0/(xi*xi*(1.0-xo/xi)))
							+ 1.0-xo/xi + 2.0/xi + 1.0/(xi*xi)) / (xi*xi));
	} else {
		return 0.0;
	}
}

Double_t Gauss(Double_t x, Double_t x0, Double_t s) {
	return (TMath::Exp(-(x-x0)*(x-x0)/(2.0*s*s)));
}

Double_t ScatteredComptonIntegrand(Double_t *x, Double_t *par) {  
	// xo = par[0], xscat=x[0], xi=par[1]
	if ( par[0] <= (2.0*x[0]/(2.0+1.0/x[0])) ) {
		return ( ScatterFunction(x[0], par[1])* ComptonFunction(par[0], x[0]));
	} else {
		return 0.0;
	}
}

Double_t ScatteredComptonFunction(Double_t xo, Double_t xi) {
	static Double_t old_xi = -1;
	const Int_t NumBins = 10000;
	const Int_t Low = 0, High = 3;
	static TH1D SavedFunctionSC("SavedFunctionSC", "SFSC", NumBins, Low, High);

	if (xo>High) {
		cout << "Warning: Expand the function range of precalculated function!! "
				 << "xo > " << High << endl;
	}
	if (old_xi!=xi) {
		// Fill Histogramm if xi changed
		//cout << "Recalculating histogram: old xi: " << old_xi << " new xi:" << xi
		//		 << "...";
		TF1 I("I", ScatteredComptonIntegrand, 0.0, 3.0, 2);
		for (Int_t i=0; i<NumBins; i++) { // Loop over bins
			Double_t par[2] = {SavedFunctionSC.GetBinCenter(i), xi};
			SavedFunctionSC.SetBinContent(i, I.Integral(1.0/(1.0/xi+2.0), xi, par));
		}
		//cout << "  done." << endl;
		old_xi=xi;
	}

	// kurz anzeigen
//  	TCanvas *canv;
//  	if ( (canv = (TCanvas *)(gROOT->GetListOfCanvases()->At(1))) !=0) {
// 		canv->cd();
// 		SavedFunctionSC.Draw();
// 		canv->Update();
// 		if ( (canv = (TCanvas *)(gROOT->GetListOfCanvases()->At(0))) !=0) {
// 			canv->cd();
// 		}
//  	}else{
// 		cout << "NO CANVAS!!" << endl;
// 	}

	// Now get value from histogram by interpolation between neighboring bins
	Int_t Bin = SavedFunctionSC.GetXaxis()->FindBin(xo);
	Double_t BinCenter=SavedFunctionSC.GetBinCenter(Bin);
	Double_t Diff = xo - BinCenter;
	if (Diff == 0) return SavedFunctionSC.GetBinContent(Bin);
	if (Diff < 0) return (SavedFunctionSC.GetBinContent(Bin) 
												+ (SavedFunctionSC.GetBinContent(Bin)
													 -SavedFunctionSC.GetBinContent(Bin+1))
												/SavedFunctionSC.GetBinWidth(Bin)*Diff);
	//	if (Diff > 0) 
	return (SavedFunctionSC.GetBinContent(Bin) 
					+ (SavedFunctionSC.GetBinContent(Bin-1)
						 -SavedFunctionSC.GetBinContent(Bin))
					/SavedFunctionSC.GetBinWidth(Bin)*Diff);
}

Double_t ScatteredComptonFunction1(Double_t xo, Double_t xi) {
	static Double_t old_xi = -1;
	const Int_t NumBins = 10000;
	const Int_t Low = 0, High = 3;
	static TH1D SavedFunctionSC("SavedFunctionSC", "SFSC", NumBins, Low, High);

	if (xi>High) {
		cout << "Warning: Expand the function range of precalculated function!! "
				 << "xi > " << High << endl;
	}
	if (old_xi!=xi) {
		// Fill Histogramm if xi changed
		//cout << "Recalculating histogram: old xi: " << old_xi << " new xi:" << xi
		//		 << "...";
		TF1 I("I", ScatteredComptonIntegrand, 0.0, 3.0, 2);
		for (Int_t i=0; i<NumBins; i++) { // Loop over bins
			Double_t par[2] = {SavedFunctionSC.GetBinCenter(i), xi};
			SavedFunctionSC.SetBinContent(i, I.Integral(1.0/(1.0/xi+2.0), xi, par));
		}
		//cout << "  done." << endl;
		old_xi=xi;
	}

	// kurz anzeigen
//  	TCanvas *canv;
//  	if ( (canv = (TCanvas *)(gROOT->GetListOfCanvases()->At(1))) !=0) {
// 		canv->cd();
// 		SavedFunctionSC.Draw();
// 		canv->Update();
// 		if ( (canv = (TCanvas *)(gROOT->GetListOfCanvases()->At(0))) !=0) {
// 			canv->cd();
// 		}
//  	}else{
// 		cout << "NO CANVAS!!" << endl;
// 	}

	// Now get value from histogram by interpolation between neighboring bins
	Int_t Bin = SavedFunctionSC.GetXaxis()->FindBin(xo);
	Double_t BinCenter=SavedFunctionSC.GetBinCenter(Bin);
	Double_t Diff = xo - BinCenter;
	if (Diff == 0) return SavedFunctionSC.GetBinContent(Bin);
	if (Diff < 0) return (SavedFunctionSC.GetBinContent(Bin) 
												+ (SavedFunctionSC.GetBinContent(Bin)
													 -SavedFunctionSC.GetBinContent(Bin+1))
												/SavedFunctionSC.GetBinWidth(Bin)*Diff);
	//	if (Diff > 0) 
	return (SavedFunctionSC.GetBinContent(Bin) 
					+ (SavedFunctionSC.GetBinContent(Bin-1)
						 -SavedFunctionSC.GetBinContent(Bin))
					/SavedFunctionSC.GetBinWidth(Bin)*Diff);
}

Double_t ScatteredComptonFunction2(Double_t xo, Double_t xi) {
	static Double_t old_xi = -1;
	const Int_t NumBins = 10000;
	const Int_t Low = 0, High = 3;
	static TH1D SavedFunctionSC2("SavedFunctionSC2", "SFSC", NumBins, Low, High);

	if (xi>High) {
		cout << "Warning: Expand the function range of precalculated function!! "
				 << "xi > " << High << endl;
	}
	if (old_xi!=xi) {
		// Fill Histogramm if xi changed
		//cout << "Recalculating histogram: old xi: " << old_xi << " new xi:" << xi
		//		 << "...";
		TF1 I("I", ScatteredComptonIntegrand, 0.0, 3.0, 2);
		for (Int_t i=0; i<NumBins; i++) { // Loop over bins
			Double_t par[2] = {SavedFunctionSC2.GetBinCenter(i), xi};
			SavedFunctionSC2.SetBinContent(i, I.Integral(1.0/(1.0/xi+2.0), xi, par));
		}
		//cout << "  done." << endl;
		old_xi=xi;
	}

	// kurz anzeigen
//  	TCanvas *canv;
//  	if ( (canv = (TCanvas *)(gROOT->GetListOfCanvases()->At(1))) !=0) {
// 		canv->cd();
// 		SavedFunctionSC2.Draw();
// 		canv->Update();
// 		if ( (canv = (TCanvas *)(gROOT->GetListOfCanvases()->At(0))) !=0) {
// 			canv->cd();
// 		}
//  	}else{
// 		cout << "NO CANVAS!!" << endl;
// 	}

	// Now get value from histogram by interpolation between neighboring bins
	Int_t Bin = SavedFunctionSC2.GetXaxis()->FindBin(xo);
	Double_t BinCenter=SavedFunctionSC2.GetBinCenter(Bin);
	Double_t Diff = xo - BinCenter;
	if (Diff == 0) return SavedFunctionSC2.GetBinContent(Bin);
	if (Diff < 0) return (SavedFunctionSC2.GetBinContent(Bin) 
												+ (SavedFunctionSC2.GetBinContent(Bin)
													 -SavedFunctionSC2.GetBinContent(Bin+1))
												/SavedFunctionSC2.GetBinWidth(Bin)*Diff);
	//	if (Diff > 0) 
	return (SavedFunctionSC2.GetBinContent(Bin) 
					+ (SavedFunctionSC2.GetBinContent(Bin-1)
						 -SavedFunctionSC2.GetBinContent(Bin))
					/SavedFunctionSC2.GetBinWidth(Bin)*Diff);
}

// Double_t DoubleComptonIntegrand(Double_t *x, Double_t *par) {
// 	// x[0]   = Energydeposit in first compton interaction
// 	// par[0] = Sum of deposited energy from both Compton interactions
// 	// par[1] = Energy of initial photon
// 	return ( ComptonFunction(x[0], par[1]) 
// 					 * ComptonFunction(par[0]-x[0], par[1]-x[0]) );
// }

// Double_t DoubleComptonFunction(Double_t xo, Double_t xi) {
// 	static Double_t old_xi = -1;
// 	const Int_t NumBins = 10000;
// 	const Int_t Low = 0, High = 3;
// 	static TH1D SavedFunctionDC("SavedFunctionDC", "SFDC", NumBins, Low, High);

// 	if (xi>High) {
// 		cout << "Warning: Expand the function range of precalculated function!! "
// 				 << "xi > " << High << endl;
// 	}
// 	if (old_xi!=xi) {
// 		// Fill Histogramm if xi changed
// 		cout << "Recalculating histogram (double compton): old xi: " << old_xi 
// 				 << " new xi:" << xi << " ...";
// 		TF1 I("I", DoubleComptonIntegrand, 0.0, 3.0, 2);
// 		for (Int_t i=0; i<NumBins; i++) { // Loop over bins
// 			Double_t par[2] = {SavedFunctionDC.GetBinCenter(i), xi};
// 			SavedFunctionDC.SetBinContent(i, I.Integral(0, xi, par));
// 		}
// 		cout << "  done." << endl;
// 		old_xi=xi;
// 	}

// 	// kurz anzeigen
//  // 	TCanvas *canv;
// //  	if ( (canv = (TCanvas *)(gROOT->GetListOfCanvases()->At(1))) !=0) {
// // 		canv->cd();
// // 		SavedFunctionDC.Draw();
// // 		canv->Update();
// // 		if ( (canv = (TCanvas *)(gROOT->GetListOfCanvases()->At(0))) !=0) {
// // 			canv->cd();
// // 		}
// //  	}else{
// // 		cout << "NO CANVAS!!" << endl;
// // 	}

// 	// Now get value from histogram by interpolation between neighboring bins
// 	Int_t Bin = SavedFunctionDC.GetXaxis()->FindBin(xo);
// 	Double_t BinCenter=SavedFunctionDC.GetBinCenter(Bin);
// 	Double_t Diff = xo - BinCenter;
// 	if (Diff == 0) return SavedFunctionDC.GetBinContent(Bin);
// 	if (Diff < 0) return (SavedFunctionDC.GetBinContent(Bin) 
// 												+ (SavedFunctionDC.GetBinContent(Bin)
// 													 -SavedFunctionDC.GetBinContent(Bin+1))
// 												/SavedFunctionDC.GetBinWidth(Bin)*Diff);
// 	//	if (Diff > 0) 
// 	return (SavedFunctionDC.GetBinContent(Bin) 
// 					+ (SavedFunctionDC.GetBinContent(Bin-1)
// 						 -SavedFunctionDC.GetBinContent(Bin))
// 					/SavedFunctionDC.GetBinWidth(Bin)*Diff);
// }

Double_t OverflowAndBackground(Double_t * ADCvalue, Double_t * par) {
	// par[0] = BackgroundHeight
	// par[1] = OverflowHeight
	// par[2] = OverflowPosition
	// par[3] = Sigma (should be fixed!)

	Double_t retval = 0.0;
	if(ADCvalue[0] <= par[2]) retval+=par[0];
	retval += par[1] * Gauss(ADCvalue[0], par[2], par[3]);
	return retval;
}

/*****************************************************************************/
