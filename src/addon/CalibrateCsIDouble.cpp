/*
 * CalibrateCsIDouble.cpp                                 v10.0  04/11/2002
 *
 *
 * Copyright (C) by Robert Andritschke, Andreas Zoglauer, Florian Schopper.
 *
 * This code implementation is the intellectual property of Robert Andritschke, Florian Schopper, Andreas Zoglauer.
 *
 * By copying, distributing or modifying the Program (or any work based on the 
 * Program) you indicate your acceptance of this statement, and all its terms.
 *
 */

/*******************************************************************************


Macro for fitting and integrating the 2-D calibration histograms of the MEGA 
3-D calorimeters.

********************************************************************************

Important comment to the nomenclature:
Energy means: Sum of Adc of both chips....

********************************************************************************

To-Do-List:

+ Integrate Pulse-Shape correction
+ Fit signal + background

********************************************************************************


Usage examples:

DoCalibrateCsIDouble10 -eee -p test_ athene data/20021015_17:12_Cs137_allD2_side2.calib 662 data/20021021_10:25_Na22_D2.calib 511 data/20021021_10:25_Na22_D2.calib 1274

*******************************************************************************/  


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
#include <Getline.h>
#include <TGraphErrors.h>


// Mega libs
#include "MCsIPixelDouble.h"

// Standard libs:
#include <sstream>
#include <fstream>
#include <iostream>
using std::cout;
using std::endl;
using std::ofstream;
#include <vector>
using std::vector;
using namespace std;



/******************************************************************************/


Bool_t DoCalibrateCsIDouble(MString DetectorName,
														vector<MString> FileName,
														vector<Float_t> Energy,
														MString Prefix,
														Int_t Stop, 
														Int_t xStart, 
														Int_t yStartt,
														MString PulseShapeFile,
														Int_t PulseShapeTiming);
Int_t DetermineOverFlow(MChip* Chip, Int_t Channel);
void FindPeakBorders(TH1D* Hist, Int_t& lower, Int_t& upper);
Bool_t FitPeak(TH1D* Hist, TF1* FitFunc, Double_t& Mean, Double_t& Sigma, 
							 Double_t LowerBorder, Double_t UpperBorder);


/******************************************************************************/


// Global constants

// Minimum number of counts to accept spectrum:
const Int_t MinCounts = 1000;
// Fraction of counts which will cropped (on low eta and high eta side,
// i.e. 2*CropEtaFraction will be cropped!) to take into account noise
const Float_t CropEtaFraction = 0.1;




/******************************************************************************
 * 
 */
int main(int argc, char** argv) 
{
	// Returns 1 in case of an syntax error, else 0
	Int_t Stop = 0;  // 0 = no stop (default)
	//                  1 = stop on critical error (bad fits)
	//                  2 = stop also on bad eta
	//                  4 = stop always
	Int_t xStart = 1, yStart = 1;
	vector<MString> FileName;
	vector<Float_t> Energy;
	MString DetectorName;
	MString Prefix="";
  MString PulseShapeFile;
	int PulseShapeTiming;
  

  MStr Usage;
  Usage<<endl;
  Usage<<"  Usage: CalibrateCsIDouble <options>"<<endl;
	Usage<<endl;
	Usage<<"     mandatory options:"<<endl;
	Usage<<"       -d <detector name>      DetectorName, e.g. -d athene"<<endl;
	Usage<<"       -c <file name> <energy> Name of calib file AND energy, e.g. 20021015_17:12_Cs137_allD2_side2.calib 662"<<endl;
	Usage<<endl;
	Usage<<"     other options:"<<endl;
	Usage<<"       -s <file name> <timing> Files containing the optional pulse shape corrections and the timing"<<endl;
  Usage<<"       -e                      Stop on critical Errors"<<endl;
	Usage<<"       -ee                     Stop on all Errors"<<endl;
	Usage<<"       -eee                    Stop after each fit"<<endl;
	Usage<<"       -b <xStart> <yStart>    Pixel at which to start"<<endl;
	Usage<<"       -p <Prefix>             Outputfiles named <Prefix><DetectorName>_calibrate.csv"<<endl;
	Usage<<"       -h                      you probably already have guessed..."<<endl;


	// Command line parsing
  MString Option;

  // Check for help
  for (Int_t i = 1; i < argc; i++) {
    Option = argv[i];
    if (Option == "-h" || Option == "--help" || Option == "?" || Option == "-?") {
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
				Option == "-p") {
			if (!((argc > i+1) && argv[i+1][0] != '-')){
				cout<<"Error: Option "<<argv[i][1]<<" needs a second argument!"<<endl;
				cout<<Usage<<endl;
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
			cout<<"Calibration file "<<FileName.back()<<" at "<<Energy.back()<<" keV"<<endl;
		} else if (Option == "-s") {
			PulseShapeFile = argv[++i];
			PulseShapeTiming = atoi(argv[++i]);
			cout<<"Pulse shape file "<<PulseShapeFile<<" at "<<PulseShapeTiming<<" keV"<<endl;
		} else if (Option == "-e") {
			Stop = 1;
			cout<<"Stop on critical errors!"<<endl;
		} else if (Option == "-ee") {
			Stop = 2;
			cout<<"Stop on all errors!"<<endl;
		} else if (Option == "-eee") {
			Stop = 4;
			cout<<"Stop after each pixel!"<<endl;
		} else if (Option == "-b") {
			xStart = atoi(argv[++i]);
			yStart = atoi(argv[++i]);
			if (xStart < 1 || xStart > 10 || yStart < 1 || yStart > 12) {
				cout<<endl; 
				cout<<"Valid ranges for the start pixel: x=[1..10], y=[1..12]" <<endl;
				cout<< endl;
				cout<<"Your choice: x="<<xStart<<", y="<<yStart<<"  -> Exit!"<<endl;
				return 1;
			} else {
				cout<<"Start pixel: x="<<xStart<<" y="<<yStart<<endl;
			}
		} else if (Option == "-p") {
			Prefix = argv[++i];
			cout<<"Using prefix: "<<Prefix<<endl;
		}
	}

	if (Energy.size() == 0) {
		cout<<"You have to give at least one calibration file to work with -> Exit"<<endl;
		cout<<Usage<<endl;
		return 1;
	}

	if (DetectorName == "") {
		cout<<"You have to give a detector name!"<<endl;
		cout<<Usage<<endl;
		return 1;
	}

	// Print parsed values
	cout << "DetectorName = \"" << DetectorName << "\"" << endl;
	cout << "xStart = \"" << xStart << "\"" << endl;
	cout << "yStart = \"" << yStart << "\"" << endl;
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

	// DoCalibrateCsIDouble() has been an root macro, 
	// it is still a function of its own:
	if (DoCalibrateCsIDouble(DetectorName,
													 FileName,  
													 Energy, 
													 Prefix, 
													 Stop, 
													 xStart, 
													 yStart,
                           PulseShapeFile,
                           PulseShapeTiming)) {
		return 0;
	} else {
    cout<<"Can not start calibration!"<<endl;
		cout << Usage << endl;
		return 1;
	}
}


// Special fit functions:
double PulseShapeCorrection(Double_t* x, Double_t* par)
{
	// We have two real parameters (Offset par[12] and Height par[14]) and 10 (!) fixed
	// 0..9 are the polynom
	// 10: minimum validity area of fit
	// 11: maximum validity area of fit
	// 12: offset
	// 13: height

	// Function is zero outside validity area
	if (x[0] < par[10]+par[12] || x[0] > par[11]+par[12]) {
		return 0.0;
	}

  // Calculate the polynom with modified high and offset
	double Potenz = 0.0;
	double Result = 0.0;
	for (int p = 0; p < 10; ++p) {
		if (Potenz == 0.0) {
			Potenz = 1.0;
		} else {
			Potenz = Potenz*(x[0]-par[12]);
		}
		Result += par[p]*Potenz;
	}
	Result *= par[13];

	return Result;
}


/*******************************************************************************
                          The real work is done here
*******************************************************************************/

Bool_t DoCalibrateCsIDouble(MString DetectorName,
														vector<MString> FileName,
														vector<Float_t> Energy,
														MString Prefix,
														Int_t Stop, 
														Int_t xStart, 
														Int_t yStart,
														MString PulseShapeFileName,
														Int_t PulseShapeTiming)
{
	// Calibrate the detector called *Detectorname at energies found 
	// in the vector Energy. For each energy there is a .calib or 
	// a .house file containing the calibration spectra. These files
	// are in the vector FileName.
	// The output file is called Prefix+DetectorName+"_calibrate.csv".
	// The fitting starts at the crystal xStart;yStart.
	// Returns kFALSE if files are not found, else kTRUE.


	// --------------------------------------------------------------------------
	// Initialisations
	// --------------------------------------------------------------------------


  // Average sigma:
  vector<double> AvgSigma;
  AvgSigma.resize(Energy.size(), 0);
  vector<int> NAvgSigma;
  NAvgSigma.resize(Energy.size(), 0);

  // Pulse shape:
	vector<TF1*> PulseShapeFit(2, (TF1*) 0);
	vector<double> PulseShapeFitxMax(2, -1);
	vector<double> TriggerThreshold(2, -1);
	vector<double> TriggerThresholdDelay(2, -1);
	vector<double> RiseOffset(2, -1);
	vector<double> RiseGradient(2, -1);

	// Open Pulse shape file if we have one...
	if (PulseShapeFileName != "") {
		// Open the file
		double par[14];
    double a, b, c;
		const int LineLength = 1000;
		char LineBuffer[LineLength];
		char NameBuffer[100];

		ifstream PulseShapeFile(PulseShapeFileName.Data());

		// Search for the chip:
		while (PulseShapeFile.getline(LineBuffer, LineLength, '\n')) {
      // Search for a "CN" chipname identification containing this detector name:
			if (sscanf(LineBuffer, "CN %s", NameBuffer) == 1) {
				if (MString(NameBuffer).Contains(DetectorName)) {
          unsigned int ChipNumber = 0;
          if (MString(NameBuffer).Contains("1") == true) {
            ChipNumber = 0;
          } else if (MString(NameBuffer).Contains("2") == true){
            ChipNumber = 1;
          } else {
            cout<<"Error: Chip with bad chip number!"<<endl;
            return false;
          }

          // Now we have the correct chip, so retrieve the data:
          PulseShapeFile.getline(LineBuffer, LineLength, '\n');
          if (sscanf(LineBuffer, "PF %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf", 
                     &par[0], &par[1], &par[2], &par[3], &par[4], &par[5], &par[6], &par[7], 
                     &par[8], &par[9], &par[10], &par[11], &a) == 13) {
            // Store the function
            PulseShapeFit[ChipNumber] = new TF1("PulseShapeFit", PulseShapeCorrection, -4000, 10000, 14);
            par[12] = -1000; // x-Offset of shape
            par[13] = 50; // height of shape
            
            PulseShapeFit[ChipNumber]->SetParameters(par);
            for (int p = 0; p < 12; ++p) {
              PulseShapeFit[ChipNumber]->FixParameter(p, par[p]);
            }
            PulseShapeFit[ChipNumber]->SetParLimits(12, -6000, 0);
            PulseShapeFit[ChipNumber]->SetParLimits(13, 10, 1000);
            PulseShapeFitxMax[ChipNumber] = a;
            cout<<"Accepting correction pulse-shape..."<<endl;
          } else {
            cout<<"Error: Unable to read pulse-shape fit from file!"<<endl;
            return false;
          }

          PulseShapeFile.getline(LineBuffer, LineLength, '\n');
          if (sscanf(LineBuffer, "PT %lf %lf", &b, &c) != 2) {
            cout<<"Error: Unable to read trigger threshold from file!"<<endl;
            return false;
          } else {
            TriggerThreshold[ChipNumber] = b;
            TriggerThresholdDelay[ChipNumber] = c;
          }

          PulseShapeFile.getline(LineBuffer, LineLength, '\n');
          if (sscanf(LineBuffer, "PR %lf %lf", &b, &c) != 2) {
            cout<<"Error: Unable to read rise data from file!"<<endl;
            return false;
          } else {
            RiseOffset[ChipNumber] = b;
            RiseGradient[ChipNumber] = c;
          }
        } // correct chip found...
      } // chip found 
    } // lines of file loop
    
    if (PulseShapeFit[0] == 0 || PulseShapeFit[1] == 0) {
      cout<<"Error: Unable to find two pulse-shape fit functions...!"<<endl;
      return false;
    }
	} // No pulse shapes 


	// Open output file to store results
	cout << "opening File to store results: \"";
	MString OutputFileName(Prefix);
  OutputFileName.Append(DetectorName);
  OutputFileName.Append("_calibrate.csv"); 
  cout << OutputFileName.Data() << "\"" << endl;
  ofstream fout(OutputFileName.Data());

	// Store header in output file 
	cout << "writing header to outputfile" << endl;
	fout << "Type          csv_csidouble" << endl
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
	ofstream lout(LogFileName.Data());

	// Initialize log file
	lout << "Log File for Detector " << DetectorName << endl;
	lout << "Generated with .calib Files: " << endl;
	for (UInt_t i=0; i< FileName.size(); ++i) {
		lout << "  " << FileName[i] << endl;
	}
	lout << "Corresponding .csv File is " << OutputFileName << endl << endl;


	// Open files containing calibration histogramms
	vector<TFile *> Files;
	Files.resize(FileName.size());
	cout << endl << "Opening files... " << endl << "  ";
	for (UInt_t i=0; i<FileName.size(); ++i) {  // Loop over lines/files
		cout << FileName[i] << ", ";
		Files[i] = new TFile(FileName[i], "READ");
		if (!(Files[i]->IsOpen())) {
			cout << endl << "Cannot open file \"" << FileName[i] << "\"!" << endl;
			return kFALSE;
		}
	}
	cout << endl;

	// Get Calibration histograms out of .calib and .house files
	vector<MCsIPixelDouble *> Detectors;
	Detectors.resize(Files.size());
	vector<MChip *> Chip1;
	Chip1.resize(Files.size());
	vector<MChip *> Chip2;
	Chip2.resize(Files.size());

	MHouseData* HK = 0;
	for (UInt_t i=0; i<Files.size(); ++i) { // Loop over all files
		HK = ((MHouseData*) Files[i]->Get("Housekeeping"));
		if (HK != 0) {
			cout << "Reading housekeeping \"" << FileName[i] << "\"" << endl;
			Bool_t DetectorFound = kFALSE;
			for (int d=0; d<HK->GetNDetectors(); ++d) {
				Detectors[i] = (MCsIPixelDouble *) HK->GetDetectorAt(d);
				if (Detectors[i]!=0) {
					if (DetectorName.CompareTo(Detectors[i]->GetName()) == 0) {
						DetectorFound = kTRUE;
						break;
					}
				} else {
					cout << "Detector[" << i << "] not found!!" << endl;
				}
			}
			if (DetectorFound == kFALSE) {
				Detectors[i] = 0;
			}
      // Find chips:
      Chip1[i] = HK->GetChip(DetectorName + ":Chip#1");
      Chip2[i] = HK->GetChip(DetectorName + ":Chip#2");
		} else {
			Detectors[i] = ((MCsIPixelDouble*) Files[i]->Get(DetectorName.Data()));
      Chip1[i] = (MChip*) Files[i]->Get(DetectorName + ":Chip#1");
      Chip2[i] = (MChip*) Files[i]->Get(DetectorName + ":Chip#2");
		}

		if (Detectors[i] == 0) {
			cout << "File \"" << FileName[i] 
					 << "\" does not contain a detector called \""
					 << DetectorName << "\"!  -> Exit!" << endl;
			cout << endl << "File content: " << endl;
			Files[i]->ls();
			return kFALSE;
		}
    if (Chip1[i] == 0) {
			cout << "File \"" << FileName[i] 
					 << "\" does not contain a chip named \""
					 << DetectorName << ":Chip#1\"!  -> Exit!" << endl;
      return kFALSE;
    }

    if (Chip2[i] == 0) {
			cout << "File \"" << FileName[i] 
					 << "\" does not contain a chip named \""
					 << DetectorName << ":Chip#2\"!  -> Exit!" << endl;
      return kFALSE;
    }
    cout << endl << "Detector \"" << DetectorName << "\" found in file \""
         << FileName[i] << "\"" << endl;
	}


  // Initialize Canvas palette 
  // Create palette:
//   Int_t Number = 2;
//   Double_t Stops[2] = { 0.00, 1.00 };
//   Double_t Red[2] = { 1.00, 0.00 };
//   Double_t Green[2] = { 1.00, 0.00 };
//   Double_t Blue[2] = { 1.00, 0.00 };
//   gStyle->CreateGradientColorTable(Number, Stops, Red, Green, Blue, 50);

  // open displays
  TCanvas* CalDataCanvas = new TCanvas("PixelData","PixelData",508,0,700,700); 
  CalDataCanvas->SetFillColor(0);
  CalDataCanvas->SetFrameBorderSize(0);
  CalDataCanvas->SetFrameBorderMode(0);
  CalDataCanvas->SetBorderSize(0);
  CalDataCanvas->SetBorderMode(0);
 
  TPad* pad1 = new TPad("pad1","Color mesh", 0.0, 0.666, 1.0, 1.000, 0);
  pad1->SetBorderMode(0);
  pad1->SetBorderSize(0);
  pad1->Draw();
  TPad* pad2 = new TPad("pad2","Color mesh", 0.0, 0.333, 1.0, 0.666, 0);
  pad2->SetBorderMode(0);
  pad2->SetBorderSize(0);
  pad2->Draw();
  TPad* pad3 = new TPad("pad3","Color mesh", 0.0, 0.000, 1.0, 0.333, 0);
  pad3->SetBorderMode(0);
  pad3->SetBorderSize(0);
  pad3->Draw();

	TCanvas* FitCanvas = new TCanvas("Energy Fit Progress", "Energy Fit Progress",
																	 0, 0, 438, 225);

	TF1* EnergyFit;  // Fit function
	EnergyFit = new TF1("Energy", "gaus"); // ranges set in PeakFit function
	TF1* LinearFit;  // Fit function
	LinearFit = new TF1("LinearFit", "pol1");

 	//TGraphErrors *EnergyCalibration = 0; //Graph for displaying result
	TGraph *EnergyCalibration = 0;

  TCanvas* LinFitCanvas =  
    new TCanvas("Linear Fit Progress", "Linear Fit Progress", 0, 250, 438, 225);
  TGraph* LinFit = 0;

  TCanvas* CorrFitCanvas =  
    new TCanvas("Correction Fit Progress", "Correction Fit Progress", 0, 500, 438, 225);
  TGraphErrors* CorrFit = 0;

	// End of Initialisations


	// --------------------------------------------------------------------------
	// Main calibration loop:
	// --------------------------------------------------------------------------
	Bool_t Exit = kFALSE;  // Set this to escape from x & y loops

	cout << endl << "Starting Fits..." << endl; 

	// Loop over all pixels
	for (Int_t x=xStart; x <= 10; x++) {     // Loop over x pixels
		if (x != xStart) yStart=1;
		for (Int_t y=yStart; y <= 12; y++) {   // Loop over y pixels
			lout << "Pixel " << x << "," << y << ": ";
			lout.flush();
			cout << endl << endl;
			cout << "\e[43mFitting pixel (x=" << x << ", y=" << y << ")\e[0m" << endl;


			// Variables:

			// Overflow vectors
			vector<Int_t> OverflowI;
			OverflowI.resize(Energy.size());
			vector<Int_t> OverflowA;
			OverflowA.resize(Energy.size());

			// vector of vectors containing eta values for z calibration
      // form: z_vec_vec[lineNo][eta of z-eta-bin]
			vector<vector<Axis_t> > eta_z_vec_vec;
			eta_z_vec_vec.resize(Energy.size());

			// vector of vectors containing corresponding z values
      // form: z_vec_vec[lineNo][z of z-eta-bin]
 			vector<vector<Stat_t> > z_vec_vec;
			z_vec_vec.resize(Energy.size());

			// vector of vectors containing eta values for energy calibration
      // form: eta_E_vec_vec[lineNo][eta of eta-bin]
			vector<vector<Axis_t> > eta_E_vec_vec;
			eta_E_vec_vec.resize(Energy.size());

			// vector of vectors containing corresponding energy values
      // form: E_vec_vec[lineNo][line adcs of eta-bin]
 			vector<vector<Double_t> > E_vec_vec;
			E_vec_vec.resize(Energy.size());

			// vector of vectors containing corresponding energy sigmas
      // form: E_sigma_vec_vec[lineNo][sigma of eta-bin]
 			vector<vector<Double_t> > E_sigma_vec_vec;
			E_sigma_vec_vec.resize(Energy.size());

      // Eta's, Gradient and Offset of the linear fit through all 
      // measurement points per eta, i.e. the result of the energy calibration!
      vector<Double_t> FinalEta;
      vector<Double_t> FinalGradient;
      vector<Double_t> FinalOffset;
      Double_t FinalOverflowI = 1024;
      Double_t FinalOverflowA = 1024;


			TH2S* crystaldisplay = 0; // Histogram containing data for displaying
			TH2S* crystal = 0;    // Histogram containing data for fitting
			TH1D* EtaHist = 0;    // Projection on eta axis
			TH1D* EnergyHist = 0; // Projection on energy[ADC] axis
			TH1D *EtaSlice = 0;   // For fitting energy in eta slices
			Stat_t AllInt;        // Sum of counts of histogram crystal
			Int_t EtaLowBin = 0;  // Lower border for eta method in histogram bins 
			Int_t EtaHighBin = 0; // Upper border for eta method in histogram bins
			// Errors:
			Bool_t BadCounts;     // not enough counts in spectrum
			Bool_t BadEta;        // eta distribution rejected
			Bool_t BadEFit;       // >= 1 bad energy fit occured

			Double_t GuessedLowerEnergy = -1; // shrink fitrange for second ff. lines
			Double_t GuessedUpperEnergy = -1;
			//			Double_t GuessedLowerEta=-1;
			//			Double_t GuessedUpperEta=-1;

			// Loop over different lines
			for (UInt_t LineNo=0; LineNo < Energy.size(); ++LineNo) { // lines loop

				BadCounts = kFALSE;
				BadEta = kFALSE;
				BadEFit = kFALSE;

				// Get and draw histograms
				char Title[100];
				sprintf(Title, "%s: Data of pixel: x=%d, y=%d, E=%f", 
								DetectorName.Data(), x, y, Energy[LineNo]);
				CalDataCanvas->SetTitle(Title);
				sprintf(Title, "%s_DataOfPixel_x%d_y%d", DetectorName.Data(), x, y);
				CalDataCanvas->SetName(Title);
	
				pad1->cd();
				crystaldisplay = Detectors[LineNo]->GetSumsAndRelsHist(x-1,y-1);    
				crystaldisplay->SetStats(kFALSE);
				crystaldisplay->Draw("COLZ");

				// if the first line delivered an energy calibration, use it to shrink ranges
				Double_t Line0Energy;
				Double_t Line0Sigma;
				crystal = new TH2S(*crystaldisplay);

				if (LineNo != 0) {
					UInt_t size = E_vec_vec[0].size();
					if (size==1) {
						Line0Energy = E_vec_vec[0][0];
						Line0Sigma = E_sigma_vec_vec[0][0];
					} else if (size > 1) {
						Line0Energy = E_vec_vec[0][size/2];
						Line0Sigma = E_sigma_vec_vec[0][size/2];
						if ( Line0Energy == -1 ) {
							Line0Energy = E_vec_vec[0][size/2 - 1];
							Line0Sigma = E_sigma_vec_vec[0][size/2 - 1];
						}
					}
					const Double_t GuessedOffset = -20.0;
					if ( Line0Energy != -1 ) {
						Line0Energy = (Line0Energy-GuessedOffset) * Energy[LineNo] / Energy[0] + 
							GuessedOffset;
						GuessedLowerEnergy = Line0Energy - 3.0*Line0Sigma;
						GuessedUpperEnergy = Line0Energy + 3.0*Line0Sigma;
						if (GuessedLowerEnergy < 0) GuessedLowerEnergy = 0;
					}
				}

				// Calculate projections on eta and energy[ADC] axis and draw them
				pad2->cd();
				EtaHist = crystal->ProjectionY("EtaHist");
				sprintf(Title, "%s: Eta projection of pixel: x=%d, y=%d, E=%f", 
								DetectorName.Data(), x, y, Energy[LineNo]);
				EtaHist->SetNameTitle("EtaProjection", Title);
				EtaHist->SetXTitle("#eta");
				EtaHist->SetYTitle("counts");
				EtaHist->SetStats(kFALSE);
				EtaHist->Draw();

				pad3->cd();
				EnergyHist = crystal->ProjectionX("EnHist");
				sprintf(Title, "%s: Energy projection of pixel: x=%d, y=%d, E=%f", 
								DetectorName.Data(), x, y, Energy[LineNo]);
				EnergyHist->SetNameTitle("EnergyProjection", Title);
				EnergyHist->SetXTitle("Sum [ADC]");
				EnergyHist->SetYTitle("counts");
				EnergyHist->SetStats(kFALSE);
				EnergyHist->Draw();

				CalDataCanvas->Update();


				// Determine overflow bin [ADC counts] from chip spectrum histograms
				OverflowI[LineNo] = DetermineOverFlow(Chip1[LineNo], Detectors[LineNo]->GetInsideInverseMapping(x, y));
				OverflowA[LineNo] = DetermineOverFlow(Chip2[LineNo], Detectors[LineNo]->GetOutsideInverseMapping(x, y));
				cout << "Overflow = "	<< OverflowI[LineNo] << ", " << OverflowA[LineNo] << endl;


				// Enough counts in spectrum?
				AllInt = crystal->Integral();
				if (AllInt < MinCounts) {
					cout << "Bad pixel: Not enough counts" << endl;
					BadCounts = kTRUE;

					eta_E_vec_vec[LineNo].resize(0);
					E_vec_vec[LineNo].resize(0);
					E_sigma_vec_vec[LineNo].resize(0);

          FitCanvas->Clear();
					FitCanvas->SetTitle("Not enough counts");
 					FitCanvas->Update();
				} else { // if enough counts

					// Determine eta borders for integration (eta method)
					Stat_t  EtaIntegral = 0;
					EtaLowBin = EtaHighBin = 0; // Borders (in histogram bins)

					while (EtaIntegral < CropEtaFraction * AllInt) { // lower bound
						EtaIntegral = EtaHist->Integral(0, EtaLowBin);
						++EtaLowBin;
					}
					EtaLowBin -= 1;
					EtaIntegral = 0;
					while (EtaIntegral < (1 - CropEtaFraction) * AllInt) { // upper bound
						EtaIntegral = EtaHist->Integral(0, EtaHighBin);
						++EtaHighBin;	 
					}
					// Done fixing eta borders for integration 


					// Examine eta peak, reject bad ones 
					// Determine real eta values from histogram bins
					Axis_t EtaLow  = EtaHist->GetXaxis()->GetBinCenter(EtaLowBin);
					Axis_t EtaHigh = EtaHist->GetXaxis()->GetBinCenter(EtaHighBin);
					Axis_t EtaDiff = EtaHigh - EtaLow;			
					TF1 *etatest1 = new TF1("etatest1", "gaus", EtaLow, EtaLow + 0.5*EtaDiff);
					TF1 *etatest2 = new TF1("etatest2", "gaus", EtaHigh - 0.5*EtaDiff, EtaHigh);
					
					pad2->cd();
					EtaHist->Fit("etatest1", "RQ");
					EtaHist->Fit("etatest2", "RQ+");
					Double_t test1_mean  = etatest1->GetParameter(1);
					Double_t test1_sigma = etatest1->GetParameter(2);
					Double_t test2_mean  = etatest2->GetParameter(1);
					Double_t test2_sigma = etatest2->GetParameter(2);
					
					MString Peak_Reject("");
					// sharp peaks?
					if (test1_sigma + test2_sigma > 0.8) 
						Peak_Reject="no sharp Eta peak";
					// peak overlap?
					if (test1_mean + 1.5*test1_sigma < test2_mean - 1.5*test2_sigma) 
						Peak_Reject="no Eta peak overlap";
					// peak within bounds? or even nan?
					if ( (test1_mean * test1_mean >= 1) || (test2_mean * test2_mean >= 1) )
						Peak_Reject="Eta peak out of bounds";
					// pointlike fit?
					if (test1_sigma < 0.01 || test2_sigma < 0.01) 
						Peak_Reject="Bad Eta fit";
					/*  Not needed. Left in just in case...
					// peaks centered?
					if ( (test1_mean**2 - test1_sigma**2) * (test2_mean**2 - test2_sigma**2) > 0)
					  Peak_Reject = "Peaks not centered";
					*/
					
					if (Peak_Reject==""){
						etatest1->SetLineColor(3);
						etatest2->SetLineColor(3);
					} else {
						etatest1->SetLineColor(2);
						etatest2->SetLineColor(2);
						cout << "Eta Peak NOT accepted" << endl 
								 << "Reason: " << Peak_Reject << endl;
						lout << Peak_Reject;
						lout.flush();
						BadEta = kTRUE;
					}
					etatest1->Draw("LSAME");
					etatest2->Draw("LSAME");
					CalDataCanvas->Update();
					// ... end of eta peak rejection
		

 					// If eta accepted then eta-method
					if (!BadEta) {
						// first make new vectors
            eta_z_vec_vec[LineNo].resize(0);
            z_vec_vec[LineNo].resize(0);
 						for (Int_t i = EtaLowBin; i < EtaHighBin; ++i) {
 							eta_z_vec_vec[LineNo].push_back(EtaHist->GetXaxis()->GetBinUpEdge(i));
 							z_vec_vec[LineNo].push_back((EtaHist->Integral(EtaLowBin,i) / AllInt) * 8.0);
 						}
 					}
					
					
					// Energy fitting process
					Double_t EnergyFitMean;
					Double_t EnergyFitSigma;

					pad3->cd();
					if (!FitPeak(EnergyHist, EnergyFit, EnergyFitMean, EnergyFitSigma, 
											 GuessedLowerEnergy, GuessedUpperEnergy)) {
						BadEFit = kTRUE;
					}
					CalDataCanvas->Update();



          /***********************************************************************
           * Do the final fits - eta resolved
           */
					if (! (BadEta && BadEFit) ) { // if not completely bad or good
						FitCanvas->cd();

						// new eta_E, E and E_sigma vectors
            eta_E_vec_vec[LineNo].resize(0);
            E_vec_vec[LineNo].resize(0);
            E_sigma_vec_vec[LineNo].resize(0);
				
						if (BadEta){ // no eta dependency available
							// only one energy point is used which is derived from energy projection 
							E_vec_vec[LineNo].push_back(EnergyFitMean);
							E_sigma_vec_vec[LineNo].push_back(EnergyFitSigma);
							FitCanvas->Clear();
							FitCanvas->SetTitle("No eta dependence");
							FitCanvas->Update();
						} else {  // try fitting energy in eta slices
							for (Int_t EtaBin = EtaLowBin; EtaBin < EtaHighBin; ++EtaBin) {
								Axis_t Eta = crystal->GetYaxis()->GetBinCenter(EtaBin);

								delete EtaSlice;
								EtaSlice = crystal->ProjectionX("Eta Slice", EtaBin-1, EtaBin+1);          // Anmerkung: eigentlich EtaBin, EtaBin);   
								EtaSlice->SetStats(kFALSE);
								EtaSlice->SetXTitle("Energy");
								EtaSlice->SetYTitle("Counts");

								sprintf(Title,"Energy Fit Progress, Eta=%f", Eta);
								FitCanvas->SetTitle (Title);
								eta_E_vec_vec[LineNo].push_back(Eta);

								// Fit energy in eta slice
								Double_t Mean, Sigma;
								if (! FitPeak(EtaSlice, EnergyFit, Mean, Sigma, GuessedLowerEnergy, GuessedUpperEnergy)) {
									E_vec_vec[LineNo].push_back(-1);
									E_sigma_vec_vec[LineNo].push_back(-1);
								} else {
									E_vec_vec[LineNo].push_back(Mean);
									E_sigma_vec_vec[LineNo].push_back(Sigma);
								}
								FitCanvas->Update();
							} // end of eta slices
						} // end of !BadEta
						
						// Count good values
						Int_t NEtaGood=0; // number of good fitted eta slices
						for (UInt_t i=0; i<eta_E_vec_vec[LineNo].size(); ++i) {
							if ( E_vec_vec[LineNo][i] != -1 ) ++NEtaGood;
						}

						// if none of the eta slices are good, take values from whole projection
						if (NEtaGood==0) {
							eta_E_vec_vec[LineNo].resize(0);
							E_vec_vec[LineNo].resize(1);
							E_vec_vec[LineNo][0] = EnergyFitMean;
							E_sigma_vec_vec[LineNo].resize(1);
							E_sigma_vec_vec[LineNo][0] = EnergyFitSigma;
						}

						// Display the fitted values if there are any
						delete EnergyCalibration;
						if ( eta_E_vec_vec[LineNo].size()==0 && E_vec_vec[LineNo][0]==-1 ) { // nothing for displaying
							EnergyCalibration = 0;
							cout << "Energy fitting not possible!" << endl;
						} else if ( eta_E_vec_vec[LineNo].size()==0 ) { // no eta dependence available
							//EnergyCalibration = new TGraphErrors(1);
							EnergyCalibration = new TGraph(1);
							EnergyCalibration->SetPoint(0, E_vec_vec[LineNo][0], 0); // eta=0 in this case
							//EnergyCalibration->SetPointError(0, E_sigma_vec_vec[LineNo][0], 1.0);
						} else { // points with eta
							Int_t PointNo = 0;
							//EnergyCalibration = new TGraphErrors(NEtaGood);
							EnergyCalibration = new TGraph(NEtaGood);

							for (UInt_t i=0; i<eta_E_vec_vec[LineNo].size(); ++i) {
								if ( E_vec_vec[LineNo][i] != -1 ) {
									EnergyCalibration->SetPoint(PointNo, E_vec_vec[LineNo][i], eta_E_vec_vec[LineNo][i]);
									//EnergyCalibration->SetPointError(PointNo, E_sigma_vec_vec[LineNo][0], 3*crystal->GetYaxis()->GetBinWidth(3));
									++PointNo;
								}
							}
						}
						if (EnergyCalibration != 0) {
							CalDataCanvas->cd();
							pad1->cd();
							EnergyCalibration->SetMarkerColor(4);
							EnergyCalibration->SetMarkerStyle(1);
							EnergyCalibration->Draw("P");
							CalDataCanvas->Update();
						}

					} // end of not completely bad or good 
				} // end of enough counts

				// Display empty FitCanvas for bad crystals
				if (BadCounts || BadEta) {
					FitCanvas->Clear();
					FitCanvas->SetTitle("No eta dependence");
 					FitCanvas->Update();
				}


        /***********************************************************************
         * Wait for a key if necessary...
         */
				Bool_t Wait = kFALSE;
				
				switch (Stop) {
				case 1:
					Wait = BadEFit;
					break;
				case 2:
					Wait = BadEta || BadEFit;
					break;
				case 4:
					Wait = kTRUE;
					break;
				}
				
				if (Wait) {
					char *input;
					TTimer  *timer = new TTimer("gSystem->ProcessEvents();", 50, kFALSE);
					
					while (Wait) {
						timer->TurnOn();
						timer->Reset();
						// Now let's read the input, we can use here any
						// stdio or iostream reading methods. like std::cin >> myinputl;
						input = Getline("Type <return> to continue or \"quit\" <return> to exit: "); 
						timer->TurnOff();
						if (input) {
							Wait = kFALSE;
							if (strcmp(input, "quit\n")==0) {
								cout << "Exiting..." << endl;
								Exit = kTRUE;
							}
						}
					}
				}
				
				if (Exit) break;
			} // end loop over lines
      

      /***********************************************************************
       * Determine Final overflow bin
       */
      FinalOverflowI = 1024;
      FinalOverflowA = 1024;
      for (unsigned int LineNo = 0; LineNo < Energy.size(); ++LineNo) {
        if (OverflowI[LineNo] < FinalOverflowI) {
          FinalOverflowI = OverflowI[LineNo];
        }
        if (OverflowA[LineNo] < FinalOverflowA) {
          FinalOverflowA = OverflowA[LineNo];
        }
      }

				
      /***********************************************************************
       * Correct for pulse-shapes:
       */
      if (PulseShapeFit[0] != 0 && PulseShapeFit[1] != 0) {
        
        vector<double> Signal(2, -1);
        // Loop over the lines...
        for (unsigned int line = 0; line < Energy.size(); ++line) {
          // and all eta bins with stored energies:
          for (unsigned int etabin = 0; etabin < eta_E_vec_vec[line].size(); ++etabin) {
            
            // Now compute SigI and SigA from eta and energy (= sum of adc values)
            Signal[0] = E_vec_vec[line][etabin]*(0.5+eta_E_vec_vec[line][etabin]);
            Signal[1] = 0.5*E_vec_vec[line][etabin]*(1-eta_E_vec_vec[line][etabin]);

            for (unsigned int chipid = 0; chipid < 2; ++chipid) {
              // Create the graph for fitting
              CorrFitCanvas->cd();
              delete CorrFit;
              CorrFit = new TGraphErrors(2);
              CorrFit->SetPoint(0, 0, TriggerThreshold[chipid]);
              CorrFit->SetPointError(0, 200, 0.5*TriggerThreshold[chipid]);
              CorrFit->SetPoint(1, PulseShapeTiming+TriggerThresholdDelay[chipid], Signal[chipid]);
              CorrFit->SetPointError(1, 200, 0.05*Signal[chipid]);
              CorrFit->Draw("AP");
              CorrFit->GetYaxis()->SetLimits(0, Signal[chipid] + 50);
              CorrFit->GetXaxis()->SetLimits(-3000, 10000);
              CorrFit->GetYaxis()->UnZoom();
              CorrFit->GetXaxis()->UnZoom();
              CorrFit->Draw("A*");

             
              // Do the fitting
              PulseShapeFit[chipid]->SetParameter(12, -1000); // x-Offset of shape
              PulseShapeFit[chipid]->SetParameter(13, 50); // height of shape
              CorrFit->Fit(PulseShapeFit[chipid], "R");
              CorrFitCanvas->Update();

              Signal[chipid] = PulseShapeFit[chipid]->Eval(PulseShapeFitxMax[chipid]+PulseShapeFit[chipid]->GetParameter(12));
            }


            if (Signal[0] + Signal[1] > 0) {
              E_vec_vec[line][etabin] = Signal[0] + Signal[1];
              eta_E_vec_vec[line][etabin] = (Signal[0] - Signal[1])/(Signal[0] + Signal[1]);
            } else {
              cout<<"Error: Sum of corrected signals equals zero!"<<endl; 
            }

            //gSystem->ProcessEvents();
            ///gSystem->Sleep(2000);
          }
        }


      }


      /***********************************************************************
       * Fit all energy points to get final offset and gradient
       */
      cout<<"Performing linear fit of the energy points..."<<endl;

      // Start with a search for the start-bins of the stored eta values
      // in the Eta-Adc-histogram (called crystal...) for each line
      int bin;
      int EtaBinMin = crystal->GetYaxis()->GetNbins()+1;
      int EtaBinMax = -1;
      for (unsigned int LineNo = 0; LineNo < Energy.size(); ++LineNo) {
        if (eta_E_vec_vec[LineNo].size() > 0) {
          // Min/Max-Bins
          bin = crystal->GetYaxis()->FindBin(eta_E_vec_vec[LineNo].front());
          if (bin < EtaBinMin) {
            EtaBinMin = bin;
          }
          bin = crystal->GetYaxis()->FindBin(eta_E_vec_vec[LineNo].back());
          if (bin > EtaBinMax) {
            EtaBinMax = bin;
          }
        }
      }
      cout<<"Min/Max bin: "<<EtaBinMin<<" - "<<EtaBinMax<<endl;

      // Loop over all eta bins, assemble a graph and show whats inside it...
      vector<int> Bins; // constructor does not work!!!
      for (unsigned int LineNo = 0; LineNo < Energy.size(); ++LineNo) {
        Bins.push_back(0);
      }
      int NDataPoints;
      for (int e = EtaBinMin; e <= EtaBinMax; ++e) {

        NDataPoints = 0;

        double CurrentEta = -1;
        vector<double> E;
        vector<double> Adc;

        for (unsigned int LineNo = 0; LineNo < Energy.size(); ++LineNo) {
          if (eta_E_vec_vec[LineNo].size() == 0) continue;
          if (crystal->GetYaxis()->FindBin(eta_E_vec_vec[LineNo][Bins[LineNo]]) == e) {
            if (E_vec_vec[LineNo][Bins[LineNo]] <= 0) continue;
            CurrentEta = eta_E_vec_vec[LineNo][Bins[LineNo]];
            Adc.push_back(E_vec_vec[LineNo][Bins[LineNo]]);
            E.push_back(Energy[LineNo]);
            Bins[LineNo]++,
            NDataPoints++;
          }
        }

        if (NDataPoints == 0) continue;

        // Sort according to increasing Adc:
        int HighestPos = 0;
        double HighestValue = -1;
        double Temp;
        for (unsigned int i = 0; i < Adc.size(); ++i) {
          HighestPos = i;
          HighestValue = Adc[i];
          for (unsigned int j = i; j < Adc.size(); ++j) {
            if (Adc[j] > HighestValue) {
              HighestValue = Adc[j];
              HighestPos = j;
            }
          }
          // Change Position
          if (HighestPos != (int) i) {
            Temp = Adc[i];
            Adc[i] = Adc[HighestPos];
            Adc[HighestPos] = Temp;
            Temp = E[i];
            E[i] = E[HighestPos];
            E[HighestPos] = Temp;
          }
        }      

        // Error check:
        // With increasing Adc also the energy has to increase:
        // Still missing...

        // Now fill the TGraph:
        cout<<"Drawing LinFit"<<endl;
        LinFitCanvas->cd();
        delete LinFit;
        LinFit = new TGraph(E.size());
        for (unsigned int i = 0; i < E.size(); ++i) {
          LinFit->SetPoint(i, Adc[i], E[i]);
        }
        //LinFit->Draw("ALP");
        //LinFit->GetXaxis()->SetLimits(0, 1500);
        //LinFit->GetYaxis()->SetLimits(0, 600);
        LinFit->Fit("LinearFit");
        LinFit->SetPoint(E.size(), 0, 0);
        LinFit->SetMarkerStyle(3);
        LinFit->Draw("AP");
        LinFitCanvas->Update();

        // Now store the results:
        if (NDataPoints > 1) {
          FinalEta.push_back(CurrentEta);
          FinalOffset.push_back(LinearFit->GetParameter(0));
          FinalGradient.push_back(LinearFit->GetParameter(1));
          cout<<CurrentEta<<"!"<<LinearFit->GetParameter(0)<<"!"<<LinearFit->GetParameter(1)<<endl;
        } else if (NDataPoints == 1) {
          FinalEta.push_back(CurrentEta);
          FinalGradient.push_back(E[0]/Adc[0]);
          FinalOffset.push_back(0.0);
        }


        // gSystem->Sleep(200);
      } // Loop over etas

       
      /***********************************************************************
       * Store all information to file:
       */
      fout<<"CB "<<x<<" "<<y<<" "<<FinalEta.size()<<" ";
      for (unsigned int e = 0; e < FinalEta.size(); ++e) {
        fout<<FinalEta[e]<<" "<<FinalOffset[e]<<" "<<FinalGradient[e]<<" ";
      }
      fout<<endl;
      fout<<"OF "<<x<<" "<<y<<" "<<FinalOverflowA<<" "<<FinalOverflowI<<endl;
			
       
      /***********************************************************************
       * Create average sigma information:
       */
      for (unsigned int LineNo = 0; LineNo < Energy.size(); ++LineNo) {
        for (unsigned int etabin = 0; etabin < eta_E_vec_vec[LineNo].size(); ++etabin) {
          if (E_sigma_vec_vec[LineNo][etabin] > 1 && eta_E_vec_vec[LineNo][etabin] > 0) {
            double SigmakeV = E_sigma_vec_vec[LineNo][etabin]*Energy[LineNo]/E_vec_vec[LineNo][etabin];
            cout<<"???: "<<E_sigma_vec_vec[LineNo][etabin]<<"!"<<Energy[LineNo]<<"!"<<E_vec_vec[LineNo][etabin]<<endl;
            AvgSigma[LineNo] = (NAvgSigma[LineNo]*AvgSigma[LineNo] + SigmakeV)/(NAvgSigma[LineNo]+1);
            NAvgSigma[LineNo]++;
            cout<<"AS "<<Energy[LineNo]<<" "<<AvgSigma[LineNo]<<" "<<SigmakeV<<endl;
          }       
        }
      }

      /***********************************************************************
       * Cleanup - prepare for next crystal...
       */
      delete crystal;
			crystal = 0;
			delete EnergyCalibration;
			EnergyCalibration = 0;
			delete EtaSlice;
			EtaSlice = 0;
			for (UInt_t i=0; i < eta_z_vec_vec.size(); ++i) {
				eta_z_vec_vec[i].resize(0);
				z_vec_vec[i].resize(0);
			}
			for (UInt_t i=0; i < eta_E_vec_vec.size(); ++i) {
				eta_E_vec_vec[i].resize(0);
				E_vec_vec[i].resize(0);
				E_sigma_vec_vec[i].resize(0);
			}
			
			if (Exit) break;
		} // end loop over pixel in y-direction
		if (Exit) break;
	} // end loop over pixel in x-direction


  // Cleaning up

	delete EnergyFit;
	delete pad3;
	delete pad2;
	delete pad1;
	FitCanvas->Close();
	delete FitCanvas;
	CalDataCanvas->Close();
	delete CalDataCanvas;

	for (UInt_t i=0; i<Chip2.size(); ++i) {
		delete Chip2[i];
	}
	for (UInt_t i=0; i<Chip1.size(); ++i) {
		delete Chip1[i];
	}
	for (UInt_t i=0; i<Detectors.size(); ++i) {
		delete Detectors[i];
	}

	for (UInt_t i=0; i<Files.size(); ++i) {
		Files[i]->Close();
		delete Files[i];
	}

  fout<<endl;
  for (unsigned int LineNo = 0; LineNo < Energy.size(); ++LineNo) {
    fout<<"AS "<<Energy[LineNo]<<" "<<AvgSigma[LineNo]<<endl;
  }

	lout.close();
	fout.close();

	return kTRUE;
}


/******************************************************************************
 * 
 */
Int_t DetermineOverFlow(MChip* Chip, Int_t Channel)
{
	// Determine the overflow bin (in ADC counts) of Chip  
	TH1D* ChannelSpec = 0; // Spectrum of one chip channel
	Int_t Overflow = -1;
	Int_t Max;

	ChannelSpec = Chip->GetHitHist()->ProjectionY("Channel spectrum", Channel, Channel);
	// zu wenige Einträge?
	if (ChannelSpec->Integral(1, ChannelSpec->GetXaxis()->GetNbins()-1, "") < MinCounts) { 
		Overflow = -1;
	} else {
		Max=(ChannelSpec->GetXaxis()->GetNbins())-1;
		while ((ChannelSpec->GetBinContent(Max--) == 0) && (Max > 1));
		Overflow = (Int_t) (ChannelSpec->GetXaxis()->GetBinCenter(Max));
	}      
	delete ChannelSpec;
	return Overflow;
}



/******************************************************************************
 * 
 */
void FindPeakBorders(TH1D* Hist, Int_t& lower, Int_t& upper, 
										 Double_t LowerBorder, Double_t UpperBorder)
{
	Int_t		PThresh;			        // Treshhold for accepting peak
	Float_t MinRelative = 0.1 ;		// the Maximum must have at least MinRelative * Max(hist)
	Int_t lowest;                 // lowest bin in hist
	Double_t max;                 // value of (local) maximum
	Int_t binmax;                 // position of maximum in bins

	// In a smoothed copy of the histogram the finding of a peak is easier
	TH1D* smoothed = 0;
	smoothed = (TH1D*) (Hist->Clone());
	smoothed->SetName("smoothed"); 
	smoothed->Smooth(2);
	if ( (UpperBorder != -1) && (LowerBorder != -1) ) {
		smoothed->GetXaxis()->SetRangeUser(LowerBorder, UpperBorder);
	}

	// Set threshold for recognizing a peak
	PThresh = (Int_t) (smoothed->GetMaximum() * MinRelative);
	
	// Determine the first maximum starting from high energies

	upper  = smoothed->GetXaxis()->GetLast();
	lowest = smoothed->GetXaxis()->GetFirst();
	
	// Upper limit of max, when spec goes above PThresh the first time
	while (smoothed->GetBinContent(upper--) < PThresh && (upper > lowest));

	// Now search for (local) max
	lower = upper;
	max = smoothed->GetBinContent(lower);
	binmax = lower;

	// The peak is isolated, if we get two adjacent steps down
	Int_t downct = 0;  // counter, how often we climb down while going lower
	Double_t mem1;     // memories for values
	Double_t mem2 = max;

	while ( lower > lowest && downct < 2 ) {
		if ( (mem1 = (Int_t) smoothed->GetBinContent(--lower)) >= mem2) {
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
	for (mem3 = binmax; (smoothed->GetBinContent(mem3)>0.5*max) && mem3<upper; ++mem3);
	upper=mem3;
 	hwhm = (upper - binmax);

	// Climb down on low side as far as possible
	mem2 = max;
	for (mem3=binmax-1; mem3>=(binmax-hwhm) && (mem1=smoothed->GetBinContent(mem3))<=mem2; --mem3) {
		mem2 = mem1;
	}
	lower = mem3;
}



/******************************************************************************
 * Fit the first peak in hist starting from high energies.
 * Return kFALSE, if software noise level reaches photopeak 
 */
Bool_t FitPeak(TH1D* Hist, TF1* FitFunc, Double_t& Mean, Double_t& Sigma, 
							 Double_t LowerBorder, Double_t UpperBorder)
{
	Bool_t retval = kTRUE;

	// Find photopeak in in histogram
	Int_t LowPhotoBin;  // Low end of photo peak in histo bins
	Int_t HighPhotoBin; // High end of photo peak in histo bins

	//	if ((LowerBorder == -1) || (UpperBorder == -1)) { 
	FindPeakBorders(Hist, LowPhotoBin, HighPhotoBin, LowerBorder, UpperBorder);
	FitFunc->SetRange(Hist->GetBinCenter(LowPhotoBin), Hist->GetBinCenter(HighPhotoBin));
	//} else {
	// 	FitFunc->SetRange(LowerBorder, UpperBorder);
	//}
	Hist->Fit("Energy", "RQ");
	Mean = FitFunc->GetParameter(1);
	Sigma = FitFunc->GetParameter(2);

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
		cout << "Photo peak NOT accepted, because photo peak is truncated!" << endl;
		retval = kFALSE;
	}
	FitFunc->Draw("LSAME");

	return retval;
}

/******************************************************************************/
