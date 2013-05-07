/* This Macros does the Fitting of the  calibration Histogram of
   MEGA 2-D Calorimeters.
   The Stored File has the Following format:
   DetectorName
   FileName The data comes from
   Energy of the source
   x;y;Meanvalue of fit;sdev of fit
   ...

   Start options:
	 

   launch DoCalibrateCsISingle6 -d circe -c data/20021015_17:12_Cs137_allD2_side2.calib 662 -c data/20021021_10\:25_Na22_D2.calib 511 -c data/20021021_10\:25_Na22_D2.calib 1274 -e


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
#include <Getline.h>
#include <TGraphErrors.h>


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


/******************************************************************************/

Bool_t DoCalibrateCsISingle(MString DetectorName,
			    vector<MString> FileName,
			    vector<Float_t> Energy,
			    MString Prefix,
			    Int_t Stop, 
			    Int_t xStart, 
			    Int_t yStart,
			    MString PulseShapeFile,
			    Int_t PulseShapeTiming);
Int_t DetermineOverFlow(MChip* Chip, Int_t Channel);
void FindPeakBorders(TH1D* Hist, Int_t& lower, Int_t& upper);
Bool_t FitPeak(TH1D* Hist, TF1* FitFunc, Double_t& Mean, Double_t& Sigma, 
	       Double_t LowerBorder, Double_t UpperBorder);
/******************************************************************************/


/* This used to be a cint macro, therefore we need a main() wrapper */

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
  Usage<<"  Usage: CalibrateCsISingle <options>"<<endl;
  Usage<<endl;
  Usage<<"     mandatory options:"<<endl;
  Usage<<"       -d <detector name>      DetectorName, e.g. -d circe (make sure ist a single sided CsI detector)"<<endl;
  Usage<<"       -c <file name> <energy> Name of calib file AND energy, e.g. 20021015_17:12_Cs137_allD2_side2.calib 662"<<endl;
  Usage<<endl;
  Usage<<"     other options:"<<endl;
  Usage<<"       -s <file name> <timing> File containing the optional pulse shape corrections and the timing"<<endl;                    
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
	Option == "-p" ||
	Option == "-s") {
      if (!((argc > i+1) && argv[i+1][0] != '-')){
	cout<<"Error: Option "<<argv[i][1]<<" needs a second argument!"<<endl;
	cout<<Usage<<endl;
	return 1;
      }
    } 
    // Double arguments:
    else if (Option == "-b" || Option == "-c") {
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

  // DoCalibrateCsISingle() has been an root macro, 
  // it is still a function of its own:
  if (DoCalibrateCsISingle(DetectorName,
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
    cout << Usage << endl;
    return 1;
  }
}



//Global Constants

// Minimum number of counts lay within the fitting range:
const Int_t MinCounts = 1000;   
// If we are in pulse shape correction and a measured photo-peak 
// is near or below the threshold, it is really probably a wrong fit:
const Int_t MinAdcCountsOverThreshold = 15;


// Special fit functions:
double PulseShapeCorrection(Double_t* x, Double_t* par)
{
  // We have two real parameters (Offset par[12] and Height par[14]) and 10 (!) fixed
  // 0..9 are the polynom
  // 10: minimum validity area of fit
  // 11: maximum validity area of fit
  // 12: offset
  // 13: height

  // 	cout<<"x: "<<x[0]<<endl;
  // 	for (int p = 0; p < 22; ++p) {
  // 		cout<<"p"<<p<<": "<<par[p]<<endl;
  // 	}

  // Function is zero outside validity area
  if (x[0] < par[10]+par[12] || x[0] > par[11]+par[12]) {
    return 0.0;
  }

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


// 	Main Program

Bool_t DoCalibrateCsISingle(MString DetectorName,
			    vector<MString> FileName,
			    vector<Float_t> Energy,
			    MString Prefix,
			    Int_t Stop, 
			    Int_t xStart, 
			    Int_t yStart,
			    MString PulseShapeFile,
			    Int_t PulseShapeTiming)
{

  // Calibrate the detector called DetectorName at energies found 
  // in the vector Energy. For each energy there is a .calib 
  // file containing the calibration spectra. These files
  // are in the vector FileName.
  // The output file is called Prefix+DetectorName+"_calibrate.csv".
  // The fitting starts at the crystal xStart;yStart.
  // Returns kFALSE if files are not found, else kTRUE.

  // --------------------------------------------------------------------------
  // Initialisations
  // --------------------------------------------------------------------------

  // Average sigma (average energy resolution for simulation)
  vector<double> AvgSigma;
  AvgSigma.resize(Energy.size(), 0);
  vector<int> NAvgSigma;
  NAvgSigma.resize(Energy.size(), 0);


  // Pulse shape stuff:
  TF1* PulseShapeFit = 0;
  double PulseShapeFitxMax = -1;
  double TriggerThreshold = -1;
  double TriggerThresholdDelay = -1;
  double RiseOffset = -1;
  double RiseGradient = -1;
	

  // Open Pulse shape file if we have one...
  if (PulseShapeFile != "") {
    // Open the file
    double par[14];
    const int LineLength = 1000;
    char LineBuffer[LineLength];
    char NameBuffer[100];

    ifstream CalibFile(PulseShapeFile);

    // Search for the detector
    bool Found = false;
    while (CalibFile.getline(LineBuffer, LineLength, '\n')) {
      if (sscanf(LineBuffer, "CN %s", NameBuffer) == 1) {
	if (MString(NameBuffer).Contains(DetectorName)) {
	  Found = true;
	  break;
	}
      }
    }

    if (Found == true) {
      CalibFile.getline(LineBuffer, LineLength, '\n');
      if (sscanf(LineBuffer, "PF %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf", 
		 &par[0], &par[1], &par[2], &par[3], &par[4], &par[5], &par[6], &par[7], 
                 &par[8], &par[9], &par[10], &par[11], &PulseShapeFitxMax) == 13) {
	// Store the function
	PulseShapeFit = new TF1("PulseShapeFit", PulseShapeCorrection, -4000, 10000, 14);
	par[12] = -1000; // x-Offset of shape
	par[13] = 50; // height of shape
				
	PulseShapeFit->SetParameters(par);
	for (int p = 0; p < 12; ++p) {
	  PulseShapeFit->FixParameter(p, par[p]);
	}
	PulseShapeFit->SetParLimits(12, -4000, 0);
	PulseShapeFit->SetParLimits(13, 10, 1000);
	cout<<"Accepting correction pulse-shape..."<<endl;
      } else {
	cout<<"Error: Unable to read pulse-shape fit from file!"<<endl;
	return false;
      }
      CalibFile.getline(LineBuffer, LineLength, '\n');
      if (sscanf(LineBuffer, "PT %lf %lf", &TriggerThreshold, &TriggerThresholdDelay) != 2) {
	cout<<"Error: Unable to read trigger threshold from file!"<<endl;
	return false;
      }
      CalibFile.getline(LineBuffer, LineLength, '\n');
      if (sscanf(LineBuffer, "PR %lf %lf", &RiseOffset, &RiseGradient) != 2) {
	cout<<"Error: Unable to read rise data from file!"<<endl;
	return false;
      }
    } else {
      cout<<"No pulse shape for detector "<<DetectorName<<" found!"<<endl;
    }
  }

  // Open output file to store results
  cout << "opening File to store results: \"";
  MString OutputFileName(Prefix);
  OutputFileName.Append(DetectorName);
  if(Energy[0] > 150.0)  OutputFileName.Append("_calibrate.csv"); 
  else OutputFileName.Append("_diodes.csv");
  cout << OutputFileName.Data() << "\"" << endl;
  ofstream fout(OutputFileName.Data());

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
  vector<MCsIPixelSingle *> Detectors;
  Detectors.resize(Files.size());
  vector<MChip *> Chip;
  Chip.resize(Files.size());

  MHouseData* HK = 0;
  for (UInt_t i=0; i<Files.size(); ++i) { // Loop over all files
    HK = ((MHouseData*) Files[i]->Get("Housekeeping"));
    if (HK != 0) {
      cout << "Reading housekeeping \"" << FileName[i] << "\"" << endl;
      Bool_t DetectorFound = kFALSE;
      for (int d=0; d<HK->GetNDetectors(); ++d) {
	TObject* StillUnknown = HK->GetDetectorAt(d);
	if (MString(StillUnknown->ClassName()) != "MCsIPixelSingle") {
	  cout<<"No, no, no, no.... This programm is for calibrating MCsIPixelSingle"<<endl;
	  cout<<"and not for "<<StillUnknown->ClassName()<<"... Aborting!"<<endl;
	  return false;
	}
	Detectors[i] = (MCsIPixelSingle*) StillUnknown;
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
      Chip[i] = HK->GetChip(DetectorName + ":Chip#1");
    } else {
      TObject* StillUnknown = Files[i]->Get(DetectorName.Data());
      if (MString(StillUnknown->ClassName()) != "MCsIPixelSingle") {
	cout<<"No, no, no, no.... This programm is for calibrating MCsIPixelSingle"<<endl;
	cout<<"and not for "<<StillUnknown->ClassName()<<"... Aborting!"<<endl;
	return false;
      }
      Detectors[i] = (MCsIPixelSingle*) StillUnknown;
      Chip[i] = (MChip*) Files[i]->Get(DetectorName + ":Chip#1");
    }

    if (Detectors[i] == 0) {
      cout << "File \"" << FileName[i] 
	   << "\" does not contain a detector called \""
	   << DetectorName << "\"!  -> Exit!" << endl;
      cout << endl << "File content: " << endl;
      Files[i]->ls();
      return kFALSE;
    }
    if (Chip[i] == 0) {
      cout << "File \"" << FileName[i] 
	   << "\" does not contain a chip named \""
	   << DetectorName << ":Chip#1\"!  -> Exit!" << endl;
      return kFALSE;
    }
    cout << endl << "Detector \"" << DetectorName << "\" found in file \""
	 << FileName[i] << "\"" << endl;
  }
        

  //Initialize graphics
	
  //Crystal Window
  TCanvas* Canvas = new TCanvas("calibration","Pixeldata",0,0,500,500); 

  //Linear Window
  TCanvas* LinFitCanvas =  
    new TCanvas("Linear Fit Progress", "Linear Fit Progress", 500, 0, 438, 330);
  TGraph* LinFit = 0;

  // CorrectionWindow:
  TCanvas* CorrFitCanvas =  
    new TCanvas("Correction Fit Progress", "Correction Fit Progress", 500, 350, 438, 330);
  TGraph* CorrFit = 0;


  //fit functions

  TF1* EnergyFit = 0;  //Gaus-fit for Photopeak  
  EnergyFit = new TF1("Energy", "gaus"); // ranges set in PeakFit function

  TF1* LinearFit = 0;  //Linear Fit as Energy interpolation between lines
  LinearFit = new TF1("LinearFit", "pol1");

  // The pulse-shape fit is already defined in the beginning...

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

      //Variables:

      //Vectors: One Element per fitted line
      // Overflow vectors
      vector<Int_t> Overflow;
      Overflow.resize(Energy.size());

      // vector containing energy values
      vector<Double_t>  E_vec;
      E_vec.resize(Energy.size());

      // vector containing corresponding energy sigmas
      vector<Double_t> E_sigma_vec;
      E_sigma_vec.resize(Energy.size());

      //Gradient and Offset of the linear fit through all fitted lines
      //One (scalar) Value per Pixel as end-result

      Double_t Final_Gradient;
      Double_t Final_Offset;
      Double_t Final_Overflow;

      Stat_t AllInt;        // Sum of counts of histogram crystal

      TH1D *crystal;    // Histogram containing data for fitting
			// Errors:
      Bool_t BadCounts;     // not enough counts in spectrum
      Bool_t BadEFit;       // >= 1 bad energy fit occurred

      Double_t GuessedLowerEnergy = -1; // shrink fitrange for second ff. lines
      Double_t GuessedUpperEnergy = -1;

			
      // Loop over different lines
      UInt_t LineNo;
      for (LineNo=0; LineNo < Energy.size(); ++LineNo) { // lines loop
				

	BadCounts = kFALSE;
	BadEFit = kFALSE;

	// Get and draw histograms
	Canvas->cd();
	char Title[100];
	sprintf(Title, "%s: Data of pixel: x=%d, y=%d, E=%f", 
		DetectorName.Data(), x, y, Energy[LineNo]);
	Canvas->SetTitle(Title);
	sprintf(Title, "%s_DataOfPixel_x%d_y%d", DetectorName.Data(), x, y);
	Canvas->SetName(Title);

	crystal = (TH1D *) Detectors[LineNo]->GetCalibHist(x-1,y-1); 
	crystal->SetStats(kFALSE);
	crystal->Draw();


	//THIS IS ONLY FOR PERSEPHONE, DIANE AND FORTUNA!
	//EXPERIMENTAL CODE!
	//forcing the first 10 bins to zero, because the high noise confuses the fit algorithms
	//seems to work all right for other detectors too.
	//The first 10 bins don't seem to contain any useful data.
	for (int i=0;i<11;i++)
	  crystal->SetBinContent(i,0);





	Canvas->Update();

	// if the first line delivered an energy calibration, use it to shrink ranges
	Double_t Line0Energy;
	Double_t Line0Sigma;

	if (LineNo != 0) {
	  Line0Energy = E_vec[0];
	  Line0Sigma = E_sigma_vec[0];
	  cout << "First Energy was: E_vec[0]="<<Line0Energy<<endl;
	  cout << "First Sigma was: E_sigma_vec[0]="<<Line0Sigma<<endl;

	  const Double_t GuessedOffset = -20.0;
	  if ( Line0Energy != -1 ) {
	    Line0Energy = (Line0Energy-GuessedOffset) * Energy[LineNo] / Energy[0] + 
	      GuessedOffset;
	    cout << "Projected new Energy is "<<Line0Energy<<endl;

	    GuessedLowerEnergy = Line0Energy - 3.0*Line0Sigma;    //three sigma around
	    GuessedUpperEnergy = Line0Energy + 3.0*Line0Sigma;    //prognosis of new energy
	    if (GuessedLowerEnergy < 0) GuessedLowerEnergy = 0;
	  }
	}



	// Determine overflow bin [ADC counts] from chip spectrum histograms
	Overflow[LineNo] = DetermineOverFlow(Chip[LineNo], Detectors[LineNo]->GetInverseMapping(x, y));
	cout << "Overflow = " << Overflow[LineNo]  << endl;

	// Enough counts in spectrum?
	AllInt = crystal->Integral();
	if (AllInt < MinCounts) {
	  cout << "Bad pixel: Not enough counts" << endl;
	  BadCounts = kTRUE;
					
	  //Canvas->Clear();
	  Canvas->SetTitle("Not enough counts");
	  Canvas->Update();
	}

	
	// *********************************************************
	//                     fit spectrum
	// *********************************************************
	
	
	//				GuessedLowerEnergy = -1; // shrink fitrange for second ff. lines
	//				GuessedUpperEnergy = -1;

	// Energy fitting process
	Double_t EnergyFitMean;
	Double_t EnergyFitSigma;
				
	if (FitPeak(crystal, EnergyFit, EnergyFitMean, EnergyFitSigma, 
		    GuessedLowerEnergy, GuessedUpperEnergy)) 
	  {
	    BadEFit = kFALSE;
	    E_vec[LineNo]=EnergyFitMean;
	    E_sigma_vec[LineNo]=EnergyFitSigma;
	  }else{
	    BadEFit = kTRUE;
	    E_vec[LineNo]=-1;
	    E_sigma_vec[LineNo]=-1;
	  }
	Canvas->Update();
	cout << "E_vec["<<LineNo<<"] = "<<E_vec[LineNo]<<endl;
				
				
				
	/*********************************************************
	 *                    correct for pulse-shape
	 *********************************************************/
				
	if (PulseShapeFit != 0) {

	  if (E_vec[LineNo] - TriggerThreshold < MinAdcCountsOverThreshold) {
	    cout<<"Calibration point lies near threhold --> Ignoring this value!"<<endl;
	    E_vec[LineNo] = -1;
	  }
          
	  if (E_vec[LineNo] == -1) continue;

	  // Create the graph for fitting
	  CorrFitCanvas->cd();
	  delete CorrFit;
	  CorrFit = new TGraph(2);
	  CorrFit->SetPoint(0, 0, TriggerThreshold);
	  CorrFit->SetPoint(1, 3500+TriggerThresholdDelay, E_vec[LineNo]);
	  CorrFit->Draw("AP");
	  CorrFit->GetYaxis()->SetLimits(0,  E_vec[LineNo]+50);
	  CorrFit->GetXaxis()->SetLimits(-1000, 10000);
	  CorrFit->GetYaxis()->UnZoom();
	  CorrFit->GetXaxis()->UnZoom();
	  CorrFit->Draw("A*");
          
	  // Do the fitting
	  CorrFit->Fit("PulseShapeFit", "R");
	  CorrFitCanvas->Update();
          
	  // Do the correction:
	  cout<<"Orig: "<<E_vec[LineNo]
	      <<" Corr: "<<PulseShapeFit->Eval(PulseShapeFitxMax+PulseShapeFit->GetParameter(12))
	      <<" (x= "<<PulseShapeFitxMax+PulseShapeFit->GetParameter(12)<<")"<<endl;
	  E_vec[LineNo] = PulseShapeFit->Eval(PulseShapeFitxMax+PulseShapeFit->GetParameter(12));
          
	  //gSystem->ProcessEvents();
	  //gSystem->Sleep(2000);		 
	}


	/*********************************************************
	 *                    the rest...
	 *********************************************************/

	//Now do the Fit over all Lines to get final offset and gradient
	//fill the TGraph:
				
	//Determine maxima in both axes to scale graph properly
	//Try to leave scale untouched as long as possible to avoid ugly jumps

	Double_t E_vec_max=1, Energy_max=1;
	//Maximum Energy: Y-Axis (+20% to be on the safe side)
	unsigned int Energy_max_num = 0;
	for (unsigned int i=0; i <= Energy.size();i++)
	  if (Energy[i] > Energy_max){
	    Energy_max=1.2*Energy[i];
	    Energy_max_num=i;
	  }
	//Corresponding fit: X-Axis
	//take first good value to guess range (+20% to be on the safe side)
	for (unsigned int i=0; i <= LineNo; i++) {
	  if (E_vec[i] != -1){
	    E_vec_max = 1.2 * crystal->GetXaxis()->GetBinCenter((int) (Energy_max*E_vec[i]/Energy[i]));
	    break;
	  }
	}
	//If we already have a Fit for Energy_max, _and_ that value
	//is greater than our first guess, discard the guess and take
	//the correct value (+5%, so that the point isn't too near the edge)
	if (LineNo >= Energy_max_num)
	  if (E_vec[Energy_max_num]>E_vec_max) //two ifs to avoid segfaults
	    E_vec_max=1.05*E_vec[Energy_max_num];

	cout<<"E_vec_max="<<E_vec_max<<endl;



	//Determine Number of good Lines
	unsigned int NGoodLines=0;
	for (unsigned int i=0; i <= LineNo; i++) 
	  if ( E_vec[i] != -1 ) NGoodLines++ ;
				
	//We need (0,0) if we have less than two Good Lines
	int NeedZero = 0;
	if ( NGoodLines < 2)
	  NeedZero=1;

	cout<<"Drawing LinFit for "<<NGoodLines<< " good Lines"<< endl;
	if (NeedZero != 0) cout << "NeedZero is "<< NeedZero<<endl;

	LinFitCanvas->cd();
	delete LinFit;

	LinFit = new TGraph(NGoodLines+NeedZero);
				
	//If we need (0,0), set it
	if (NeedZero != 0) LinFit->SetPoint(NGoodLines,0,0);

	//Now Loop over all good Lines, set the Datapoints and fit linear graph
	LinFitCanvas->cd();

	unsigned int count = 0;
	for (unsigned int i = 0; count < NGoodLines; i++) {
	  if ( E_vec[i] != -1) {
	    cout <<"Energy "<< Energy[i]<<" is at bin " <<E_vec[i]<<endl;
	    LinFit->SetPoint(count++, E_vec[i], Energy[i]);
	  } else {
	    cout <<"Energy "<< Energy[i]<<" at bin " <<E_vec[i]<<" rejected"<< endl;
	  }
	}
	LinFit->Draw("ALP");
	LinFit->GetYaxis()->SetLimits(0,  Energy_max);
	LinFit->GetXaxis()->SetLimits(0, E_vec_max );
	LinFit->GetYaxis()->UnZoom();
	LinFit->GetXaxis()->UnZoom();
	LinearFit->SetRange(0, E_vec_max);
	LinFit->Fit("LinearFit", "RQ");
	//LinFit->SetPoint(E_vec.size(), 0, 0);
	LinFit->SetMarkerStyle(3);

	LinFit->Draw("AP");
	LinFitCanvas->Update();



	//Wait for a key if necessary...

	Bool_t Wait = kFALSE;
				
	switch (Stop) {
	case 1:
	  Wait = BadEFit;
	  break;
	case 2:
	  Wait = kTRUE;
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
	    input = Getline("Type <return> to continue or \"q(uit)\" <return> to exit: "); 
	    timer->TurnOff();
	    if (input) {
	      Wait = kFALSE;
	      if ( (! strcmp(input, "quit\n")) | (!strcmp(input,"q\n")) ) {
		cout << "Exiting..." << endl;
		Exit = kTRUE;
	      }
	    }
	  }
	}
				
	if (Exit) break;
      } // End Line-Loop

      // Determine Final overflow bin
      Final_Overflow = 1024;
      for (unsigned int LineNo = 0; LineNo < Energy.size(); ++LineNo) {
        if (Overflow[LineNo] < Final_Overflow) {
          Final_Overflow = Overflow[LineNo];
        }
      }
      //Read out fit-parameters
      Final_Offset = LinearFit->GetParameter(0);
      Final_Gradient = LinearFit->GetParameter(1);

      //Write Data to file:
      if (Final_Gradient<=0) 
	Final_Offset=Final_Gradient=-1;

      cout<<endl<<"Offset: "<<Final_Offset<<endl<<"Gradient: "<<Final_Gradient<<endl;

      fout<<"CB "<<x<<" "<<y<<" "<<Final_Offset<<" "<<Final_Gradient<<endl;
      fout<<"OF "<<x<<" "<<y<<" "<<Final_Overflow<<endl;
			
      // Loop over all lines and compute average sigma value:
      for (unsigned int LineNo = 0; LineNo < Energy.size(); ++LineNo) {
        if (E_vec[LineNo] > 0) {
          double SigmakeV = E_sigma_vec[LineNo]*Energy[LineNo]/E_vec[LineNo];
          AvgSigma[LineNo] = (NAvgSigma[LineNo]*AvgSigma[LineNo] + SigmakeV)/(NAvgSigma[LineNo]+1);
          NAvgSigma[LineNo]++;
          cout<<"AS "<<Energy[LineNo]<<" "<<AvgSigma[LineNo]<<endl;
        }       
      }

			
      if (Exit) break;
    }   // End y-loop
    if (Exit) break;
  }     // End x-loop

  fout<<endl;
  for (unsigned int LineNo = 0; LineNo < Energy.size(); ++LineNo) {
    fout<<"AS "<<Energy[LineNo]<<" "<<AvgSigma[LineNo]<<endl;
  }

  return 1;

}



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


/************************************************************************
  Find limits of Peak
************************************************************************/

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
 *****************************************************************************/
Bool_t FitPeak(TH1D* Hist, TF1* FitFunc, Double_t& Mean, Double_t& Sigma, 
	       Double_t LowerBorder, Double_t UpperBorder)
{
  Bool_t retval = kTRUE;

  // Find photopeak in in histogram
  Int_t LowPhotoBin;  // Low end of photo peak in histo bins
  Int_t HighPhotoBin; // High end of photo peak in histo bins


  cout << "LowerBorder="<<LowerBorder<<" UpperBorder="<< UpperBorder<<endl;
  // 		if ((LowerBorder == -1) || (UpperBorder == -1)) { 
  FindPeakBorders(Hist, LowPhotoBin, HighPhotoBin, LowerBorder, UpperBorder);
  FitFunc->SetRange(Hist->GetBinCenter(LowPhotoBin), Hist->GetBinCenter(HighPhotoBin));
  cout << "LowerBorder="<<LowPhotoBin<<" UpperBorder="<<HighPhotoBin <<endl;
  // 		} else {
  // 			FitFunc->SetRange(LowerBorder, UpperBorder);
  // 		}
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
