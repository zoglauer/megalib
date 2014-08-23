/*
 * CalibrateSiStrip.cxx
 *
 *
 * Copyright (C) by Robert Andritschke, Florian Schopper, Andreas Zoglauer.
 *
 * This code implementation is the intellectual property of Robert Andritschke, Florian Schopper, Andreas Zoglauer.
 *
 * By copying, distributing or modifying the Program (or any work based on the 
 * Program) you indicate your acceptance of this statement, and all its terms.
 *
 */

/*******************************************************************************
/* This Macros does the Fitting of the  calibration Histogram of
   MEGA Si Stripdetectors.
   The Stored File has the Following format:
   DetectorName
   FileName The data comes from
   Energy of the source
   xchan;Meanvalue of fit;sdev of fit
   ...
   ychan;Meanvalue of fit;sdev of fit
   ...
  
   Usage:

   fstream out;
   out.open("foo", ios::out);
   out.close();

   .L macros/Calibration/DoCalibrateSiStrip6.C

   DoCalibrateSiStrip6("20021008_Co57_D1.calib","bachus" , "20021008_Co57_", 122 );
   DoCalibrateSiStrip6("20021008_10:57_Am241_bottom_D1.calib","bachus" , "20021008_Am241_", 59.5 );
   DoCalibrateSiStrip5("20020813_11:40_Ce139_vul_21deg.calib","vulkan" , "20020813_ce139_", 166 );
   DoCalibrateSiStrip5("Test_Cd109.calib","bachus" , "20020807_20deg_", 88 );
   DoCalibrateSiStrip5("20020806_11:26_Cd109_D1_20deg.calib","bachus" , "20020806_20deg_", 88 );
   DoCalibrateSiStrip5("Comb_Ba133.calib","bachus" , "20020726_20deg_", 81 );
   DoCalibrateSiStrip5("20020725_15:14_Ba133_D1_20deg.calib","bachus" , "20020725_20deg_", 81 );
   DoCalibrateSiStrip5("20020724_11:05_Am241_D1_20deg.calib","bachus" , "20020724_20deg_", 59.5 );
   DoCalibrateSiStrip5("20020723_14:55_Ba133_D1_20deg.calib","bachus" , "20020723_20deg_", 81 );
   DoCalibrateSiStrip5("20020719_10:11_Co57_D1_20deg.calib","bachus" , "20020719_20deg_", 122 ); 
   DoCalibrateSiStrip5("20020209_16:32_Co57_ach.calib","achilles" , "test_", 122 ); 
   DoCalibrateSiStrip5("20020521_13:28_Co57_bac_pos.calib","poseidon" , "test_", 122 ); 
   DoCalibrateSiStrip5("20020620_10:31_Co57_D1.calib","bachus" , "20020620_", 122 ); 


   // Drag 'n Drop
   MString FileName = "20020209_16:32_Co57_ach.calib";
   MString DetectorName = "achilles";
   Float_t Energy = 122;
   Int_t LowerLimit = 15;
   Float_t LowerResolution = 0.5;
   gSystem->Load("lib/Det.so");
   TCanvas *cal = new TCanvas("calibration","Stripdata",50,100,500,500); 
   TFile *mine = new TFile(FileName.Data() , "READ" );
   {
   if (!(mine->IsOpen())) {
   cout << "Can't open " << FileName << "!" << endl;
   exit(2);
   }
   }

   MSiStrip *Det;
   Det = ((MSiStrip*) mine->Get( DetectorName.Data()));
   TH1S *MyHist;
   MyHist = Det->GetCalibHist( kTRUE , 100 );
   Int_t UpperLimit=120;
   MyHist->GetXaxis()->SetRange( LowerLimit , UpperLimit );   // 0 is ignored
   MyHist->Draw() ;

   TH1S *smoothed = MyHist->Clone();
   smoothed->SetName("smoothed");  
   smoothed->Smooth(20);      
   smoothed->Draw();
   
  
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
#include "MSiStrip.h"

// Standard libs:
#include <sstream>
#include <iostream>
using std::cout;
using std::endl;
using std::ofstream;
#include <vector>
using std::vector;

//
bool CalibrateSiStrip(vector<MString> FileName, MString DetectorName,	vector<Float_t> Energy,	MString Prefix,	Int_t Stop, Int_t Start, Int_t Side);
Bool_t FitPeak(TH1S* Hist, TF1* FitFunc, Double_t& Mean, Double_t& Sigma, Double_t LowerBorder, Double_t UpperBorder);
void FindPeakBorders(TH1S* Hist, Int_t& lower, Int_t& upper, Double_t LowerBorder, Double_t UpperBorder, Int_t& PThresh);
bool WaitForKeyPress(Bool_t Wait);


// Remember difference between Bins and ADC counts!  ~2 ADC cts per Bin for D1
Float_t LowerLimit=30.0;    // Minimum accepted peak centroid (in ADC)
Float_t UpperLimit=200.0;     // Maximum accepted peak centroid (in ADC)
Float_t LowerResolution=1.0;   // Minimum accepted energy resolution
Int_t DisplayMax = -1111;     // Max. counts displayed
Bool_t OneAtATime = kFALSE;   // Stop after each fit?


/* This used to be a cint macro, therefore we need a main() wrapper */

int main(int argc, char** argv) 
{
	// Returns 1 in case of an syntax error, else 0
	Int_t Stop = 0;  // 0 = no stop (default)
	//                  1 = stop on bad fits
	//                  2 = always stop
	Int_t Start = 1;
	Int_t Side = 2;   //Defaults to N- and P- Side
	vector<MString> FileName;
	vector<Float_t> Energy;
	MString DetectorName;
	MString Prefix="";


  MStr Usage;
  Usage<<endl;
  Usage<<"  Usage: CalibrateSiStrip  <options>"<<endl;
	Usage<<endl;
	Usage<<"     mandatory options:"<<endl;
	Usage<<"       -d <detector name>      DetectorName, e.g. -d bachus (make sure ist a Si strip Detector)"<<endl;
	Usage<<"       -c <file name> <energy> Name of calib file AND energy, e.g. 20021008_Co57_D1.calib 122"<<endl;
	Usage<<"                               There can be more than one -c option"<<endl;
	Usage<<endl;
	Usage<<"     other options:"<<endl;

	Usage<<"       -e                      Stop on Fit-Errors"<<endl;
	Usage<<"       -ee                     Stop after each fit"<<endl;
	Usage<<"       -b <Start>              Strip at which to start"<<endl;
	Usage<<"       -N                      Do only the N-Side"<<endl;
	Usage<<"       -P                      Do only the P-Side"<<endl;
	Usage<<"       -p <Prefix>             Outputfiles named <Prefix><DetectorName>_calibrate.csv"<<endl;
	Usage<<"       -v                      Be verbose"<<endl;
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
				Option == "-p" ) {
			if (!((argc > i+1) && argv[i+1][0] != '-')){
				cout<<"Error: Option "<<argv[i][1]<<" needs an argument!"<<endl;
				cout<<Usage<<endl;
				return 1;
			}
		} 
		// Double arguments:
		else if (Option == "-c") {
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

		if (Option == "-v"){
			g_Verbosity = 1;
			mdebug<<"verbose mode ON"<<endl;
		} else if (Option == "-d") {
			DetectorName = argv[++i];
			mdebug<<"Detector name: "<<DetectorName<<endl;
		} else if (Option == "-c") {
			FileName.push_back(argv[++i]);
			Energy.push_back(atoi(argv[++i]));
			mdebug<<"Calibration file "<<FileName.back()<<" at "<<Energy.back()<<" keV"<<endl;
		} else if (Option == "-e") {
			Stop = 1;
			mdebug<<"Stop on errors!"<<endl;
		} else if (Option == "-ee") {
			Stop = 2;
			mdebug<<"Stop after each pixel!"<<endl;
		} else if (Option == "-N") {
			Side=0;
			mdebug<<"Do only N-Side"<<endl;
		} else if (Option == "-P") {
			Side=1;
			mdebug<<"Do only P-Side"<<endl;
		} else if (Option == "-b") {
			Start = atoi(argv[++i]);
			if (Start < 1 || Start > 384) {
				cout<<endl; 
				cout<<"Valid ranges for the start strip: 1..384" <<endl;
				cout<< endl;
				cout<<"Your choice: "<<Start<<"  -> Exit!"<<endl;
				return 1;
			} else {
				mdebug<<"Start strip: x="<<Start<<endl;
			}
		} else if (Option == "-p") {
			Prefix = argv[++i];
			mdebug<<"Using prefix: "<<Prefix<<endl;
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

	switch (Stop) {
	case 0:
		mdebug << "Run through fitting without stop" << endl;
		break;
	case 1:
		mdebug << "Stop on bad fits" << endl;
		break;
	case 2:
		mdebug << "Stop after each line" << endl;
		break;
	default:
		mdebug << "What's that, Stop = " << Stop << "?" << endl;
  }


	// CalibrateSiStrip() has been an root macro, 
	// it is still a function of its own:
	if (CalibrateSiStrip(FileName, DetectorName, Energy, Prefix, Stop, Start, Side)) { 
		return 0;
	} else {
		cout << Usage << endl;
		return 1;
	}
}



/////////////////////////////////////////////////
//       Main Program
////////////////////////////////////////////////

bool CalibrateSiStrip(vector<MString> FileName, 
											MString DetectorName,
											vector<Float_t> Energy,
											MString Prefix,
											Int_t Stop, 
											Int_t Start,
											Int_t Side)

{

	//Some Variables

	//One Value per Line and per Strip
	//The size of these vectors is sizeof Energy in one direction,
	//And will be determined on the fly in the other direction

	vector<vector<Double_t> > En_Mean_vec_vec;  //Mean Energy for each fit.
	En_Mean_vec_vec.resize(Energy.size());


	vector<vector<Double_t> > Overflow_vec_vec;
	Overflow_vec_vec.resize(Energy.size());

	//One Value per Strip
	//The size of these vectors will be determined on the fly

	vector<Double_t> Final_Grad_vec;           //Gradient of linear fit for each strip
	vector<Double_t> Final_Off_vec;            //Offset of linear fit for each strip
	vector<Double_t> Final_Overflow;
	vector<unsigned int> NGoodLines;         //Number of Good Lines.
	vector<Int_t> strip_x, strip_y;

	//One Value per Line
	//The size of these vectors is fixed to Energy.size()
	vector<Double_t> Sum_Sigma_keV;
	Sum_Sigma_keV.resize(Energy.size(), 0);

	vector<Int_t> N_Sigma;
	N_Sigma.resize(Energy.size(), 0);

	vector<Double_t> Avg_Sigma_keV;
	Avg_Sigma_keV.resize(Energy.size(), 0);


	//One Value total
	//These are scalars
	Double_t Sum_Offset=0;
	Double_t Avg_Offset=0;
	Int_t N_Offset=0;

	//Why do we need this?
	/*
    fstream out; 
    out.open("foo", ios::out);
    out.close();
  */
  gSystem->Load("lib/Det.so");
  
  // Spectrum Window
  TCanvas *cal = new TCanvas("calibration","Stripdata",500,500); 

 	//Linear Window
	TCanvas* LinFitCanvas =  
		new TCanvas("Linear Fit Progress", "Linear Fit Progress", 438,330);
	TGraph* LinFit = 0;
	
	// Density Plot of all Linear fits.
	// Used to analyse the Offset and Gradient Distribution.
	/* 
     TCanvas* FitDensityCanvas =
     new TCanvas("FitDensity", "FitDensity", 400, 400);
     TH2D* FitDensity = new TH2D("Density", "Density", 200, 0, 1000, 200, 0, 200);
	*/

	//Use several Lines in several Files
	vector<TFile *> Files;
	Files.resize(FileName.size());
	mdebug << endl << "Opening files... " << endl << "  ";
	for (UInt_t i=0; i<FileName.size(); ++i) {  // Loop over lines/files
		mdebug << FileName[i] << ", ";
		Files[i] = new TFile(FileName[i], "READ");
		if (!(Files[i]->IsOpen())) {
			mdebug << endl << "Cannot open file \"" << FileName[i] << "\"!" << endl;
			return kFALSE;
		}
	}


	//Read Detectordata from all the Files


	vector<MSiStrip *> Det;
	Det.resize(Files.size());
	for (UInt_t i=0; i<Files.size(); ++i) {
		Det[i]= (MSiStrip*) Files[i]->Get(DetectorName.Data());
		if ( Det[i]==0 ) {  //Does that work? Not sure if we've got the pointers right, but seems to work.
			cout << "Detector "<<DetectorName.Data()<<" not found. Aborting."<<endl;
			return false;
		}
	}

	// Open the output-file:
	MString Dummy( DetectorName );
	Dummy.Prepend( Prefix );
	Dummy.Append("_calibrate.csv");
	mdebug <<"Opening output file: "<< Dummy << endl;
	fstream out;
	out.open(Dummy.Data(), ios::out);
	
	// Header info
	
	out << "Type             cvs_sistrip" << endl;
	out << "Version          3"   << endl; 
	out << endl;
	out << "DetectorName     "    << DetectorName.Data() << endl;
	for (UInt_t i=0;i<Files.size();++i){
		out << "CalibFileName    "    << FileName[i].Data() << endl;
	}
	MTime Time;
	out << "Date             "    << Time.GetSQLString() << endl; 
	out << endl;
	
	
  //Counter to run over all strips.
	UInt_t strip=0;

	//Fit functions
	TF1* EnergyFit = 0;  //Gaus-fit for Photopeak  
	EnergyFit = new TF1("Energy", "gaus"); // ranges set in PeakFit function
	TF1* LinearFit = 0;  //Linear Fit as Energy interpolation between lines
	LinearFit = new TF1("LinearFit", "pol1");


	//Loop over all strips
	// x: 0,1    y: 1,384
  for(Int_t x = 0 ; x <= 1 ; x++ ) {
    if (Side==0 && x == 1) continue;
    if (Side==1 && x == 0) continue;
		for(Int_t y = Start ; y <= 384 ; y++ ) {

			mdebug << endl << endl;
			mdebug << "\e[43mFitting strip " << (( x==0 ) ? "N " : "P ") << y << "\e[0m" << endl;


			//Variables
				
			//Main Histogram
			TH1S *MyHist;  
				
			//Vectors for the values of each fitted line
			vector<Double_t> Mean;
			Mean.resize(Files.size());
				
			vector<Double_t> Sigma;
			Sigma.resize(Files.size());
				
			vector<Int_t> Overflow;
			Overflow.resize(Files.size());
				

			Double_t GuessedLowerEnergy = -1; // shrink fitrange for second ff. lines
			Double_t GuessedUpperEnergy = -1;

			Final_Overflow.push_back(1024); //Initialize Final Overflow bin to max

				
			// Loop over different lines
			for (UInt_t LineNo=0; LineNo < Energy.size(); ++LineNo) { // lines loop

				//Set Name of Window
				char Title[100];
				sprintf(Title, "%s: Data of strip: %s %d, E=%f", 
								DetectorName.Data(), (x == 0) ? "N" : "P" , y, Energy[LineNo]);
				cal->SetTitle(Title);
				sprintf(Title, "%s_DataOfPixel_x%d_y%d", DetectorName.Data(), x, y);
				cal->SetName(Title);
				

				// Get calib histogram for this strip
				if( x == 0 ) {
					MyHist = Det[LineNo]->GetCalibHist( kTRUE , y );
				}
				if( x == 1 ) {
					MyHist = Det[LineNo]->GetCalibHist( kFALSE , y ); 
				}
        

				// if the first line delivered an energy calibration, use it to shrink ranges
				Double_t Line0Energy;

				if (LineNo != 0) {
					//mdebug << "First Energy was: Mean[0]="<<Mean[0]<<endl;
					//mdebug << "First Sigma was: Sigma[0]="<<Sigma[0]<<endl;
					Double_t GuessedOffset=20 ;
					if (N_Offset >0) GuessedOffset=Sum_Offset/N_Offset;
					//mdebug<<"GuessedOffset "<<GuessedOffset<<endl;

					if ( Mean[0] > 0 ) {

						Line0Energy = (Mean[0]-GuessedOffset) * Energy[LineNo] / Energy[0] + 
							GuessedOffset;
						//mdebug << "Projected new Energy is "<<Line0Energy<<endl;

						GuessedLowerEnergy = Line0Energy - 3.0*Sigma[0];    //three sigma around
						GuessedUpperEnergy = Line0Energy + 3.0*Sigma[0];    //prognosis of new energy
						if (GuessedLowerEnergy < 0) GuessedLowerEnergy = 0;
					}
				}


				// First off, get overflow ADC: the highest bin with something in it
				Int_t dummy1, oben1=( (Int_t) MyHist->GetXaxis()->GetNbins() )-1;
				while ( (dummy1=(Int_t) MyHist->GetBinContent(oben1--) == 0) && (oben1 > 1));
				
				Overflow[LineNo] =(Int_t) MyHist->GetXaxis()->GetBinCenter(oben1);

				cal->cd();
				// Added for compatibilty with ROOT 3.03.03+
				MyHist->SetStats(kFALSE);
				MyHist->SetMaximum(DisplayMax) ;
				MyHist->Draw() ;
				


				Bool_t GoodFit=kTRUE;
				
				Mean[LineNo]=0;
				Sigma[LineNo]=0;
				GoodFit=FitPeak(MyHist,EnergyFit,Mean[LineNo],Sigma[LineNo],GuessedLowerEnergy,GuessedUpperEnergy);

				cal->Update();
				
				// Fit schlecht?

				if (GoodFit) {
					//Save Fit-Data for Later
					En_Mean_vec_vec[LineNo].push_back(Mean[LineNo]);

					//prepare averaging of Sigma
					Sum_Sigma_keV[LineNo] += Sigma[LineNo]*Energy[LineNo]/En_Mean_vec_vec[LineNo][strip];
					N_Sigma[LineNo]++;
					//mdebug <<" Sum_Sigma_keV="<<Sum_Sigma_keV[LineNo];
					//mdebug <<" N_Sigma="<<N_Sigma[LineNo];


					//Determine Final (minimal) Overflow bin
					if (Overflow[LineNo] < Final_Overflow[strip]) 
						Final_Overflow[strip] = Overflow[LineNo];
					
					mdebug<<"Found Energy "<<Energy[LineNo]<<"keV at "<<Mean[LineNo];
					mdebug<<" with a Sigma of "<<Sigma[LineNo]*Energy[LineNo]/En_Mean_vec_vec[LineNo][strip];
					mdebug <<"keV ("<<Sigma[LineNo]<<" bins)"<<endl;
				} else {
					En_Mean_vec_vec[LineNo].push_back(-1);
					mdebug<<"=> Rejected Line at Energy "<<Energy[LineNo]<<"keV"<<endl;
				}

				//Now do the Fit over all Lines to get final offset and gradient
				//fill the TGraph:
				
				//Determine maxima in both axes to scale graph properly
				//Try to leave scale untouched as long as possible to avoid ugly jumps
				
				Double_t En_Mean_max=1, Energy_max=1;
				//Maximum Energy: Y-Axis (+20% to be on the safe side)
				unsigned int Energy_max_num = 0;
				for (unsigned int i=0; i <= Energy.size();i++)
					if (Energy[i] > Energy_max){
						Energy_max=1.2*Energy[i];
						Energy_max_num=i;
					}
				//mdebug << "Energy_max="<<Energy_max<<" Energy_max_num="<<Energy_max_num<<endl;

				//Corresponding fit: X-Axis
				//take first good value to guess range (+20% to be on the safe side)
				for (unsigned int i=0; i <= LineNo; i++) {
					if (En_Mean_vec_vec[i][strip] != -1){
						En_Mean_max = 1.2 * MyHist->GetXaxis()->GetBinCenter((int) (Energy_max*En_Mean_vec_vec[i][strip]/Energy[i]));
						break;
					}
				}
				//If we already have a Fit for Energy_max, _and_ that value
				//is greater than our first guess, discard the guess and use
				//the correct value (+5%, so the point isn't too near the edge)
				if (LineNo >= Energy_max_num)
					if (En_Mean_vec_vec[Energy_max_num][strip]>En_Mean_max) //two ifs to avoid segfaults
						En_Mean_max=1.05*En_Mean_vec_vec[Energy_max_num][strip];
				
				//				mdebug<<"En_Mean_max="<<En_Mean_max<<endl;



				//Determine Number of good Lines

				if (NGoodLines.size()< strip+1)	NGoodLines.push_back(0); //keep length of array okay
				else NGoodLines[strip]=0;

				for (unsigned int i=0; i <= LineNo; i++) {
					if ( En_Mean_vec_vec[i][strip] != -1 ) NGoodLines[strip]=NGoodLines[strip]+1 ;
				}
				//We need (0,0) if we have less than two Good Lines
				int NeedZero = 0;
				if ( NGoodLines[strip] < 2)
					NeedZero=1;
				
				//mdebug<<"Drawing LinFit for "<<NGoodLines[strip]<< " good Lines"<< endl;
				
 				LinFitCanvas->cd();
 				delete LinFit;
				
 				LinFit = new TGraph(NGoodLines[strip]+NeedZero);
				
				//If we need (0,0), set it
				if (NeedZero != 0) LinFit->SetPoint(NGoodLines[strip],0,0);
				
				//Now Loop over all good Lines, set the Datapoints and fit linear graph
				LinFitCanvas->cd();
				
				unsigned int count = 0;
				for (unsigned int i = 0; count < NGoodLines[strip]; i++) {
					if ( En_Mean_vec_vec[i][strip] != -1) {
						//mdebug <<"Energy "<< Energy[i]<<" is at bin " <<En_Mean_vec_vec[i][strip]<<endl;
						LinFit->SetPoint(count++, En_Mean_vec_vec[i][strip], Energy[i]);
					} else {
						//mdebug <<"Energy "<< Energy[i]<<" at bin " <<En_Mean_vec_vec[i][strip]<<" rejected"<< endl;
					}
				}
				LinFit->Draw("ALP");
				LinFit->GetYaxis()->SetLimits(0,  Energy_max);
				LinFit->GetXaxis()->SetLimits(0, En_Mean_max );
				LinFit->GetYaxis()->UnZoom();
				LinFit->GetXaxis()->UnZoom();
				LinearFit->SetRange(0, En_Mean_max);
				LinFit->Fit("LinearFit", "RQ");
				//LinFit->SetPoint(E_vec.size(), 0, 0);
				LinFit->SetMarkerStyle(3);
				
				LinFit->Draw("AP");
				LinFitCanvas->Update();



				if (LineNo==Energy.size()-1){  //Last Line
					mdebug << endl;
					mdebug << "Overflow ADC: " << Overflow[LineNo] << endl;
					mdebug << "Offset:       " <<LinearFit->GetParameter(0)<<endl;
					mdebug << "Gradient:     " <<LinearFit->GetParameter(1)<<endl;
					if (NeedZero != 0) 
						mdebug << "There are too few lines to correctly calculate the offset."<<endl
									 << "We will have to redo this later!"<<endl;

					// We simply save the fitted values for later. The decision
					// wether the fit should be redone, rejected or accepted is
					// done later with NGoodLines.

					// We do _not_ reject negative Offsets. This can lead to
					// negative Energies, which has to be avoided in the Data
					// Analysis Software.
					Final_Off_vec.push_back(LinearFit->GetParameter(0));
					Final_Grad_vec.push_back(LinearFit->GetParameter(1));

					if (Final_Off_vec[strip]> 0) {
						N_Offset++;
						Sum_Offset+=Final_Off_vec[strip];
					}
				} else {
          mdebug <<"preliminary Offset:   "<<LinearFit->GetParameter(0)<<endl;
          mdebug <<"preliminary Gradient: "<<LinearFit->GetParameter(1)<<endl;

				}
				cal->cd();


				// Density Plot of all Linear fits.
				// Used to analyse the Offset and Gradient Distribution.
				/*
          if (NeedZero == 0)
					for (int b = 1; b <= FitDensity->GetNbinsX(); ++b) {
          FitDensity->Fill(FitDensity->GetBinCenter(b), 
          LinearFit->Eval(FitDensity->GetBinCenter(b)), 1);
					}
          FitDensityCanvas->cd();
          FitDensity->Draw("colz");
          FitDensityCanvas->Update();
				*/


				//Not all too elegant, I know... but at least it works!
				if (Stop==0) OneAtATime=kFALSE;
				if (Stop==1)
					if (GoodFit) OneAtATime=kFALSE;
					else OneAtATime=kTRUE;
				if(Stop==2) OneAtATime=kTRUE;

				if (WaitForKeyPress(OneAtATime)) return true;


			} //End Line-Loop

			mdebug<<endl;

			strip_x.push_back(x);
			strip_y.push_back(y);
			strip++;  //Count the strips
		} // End Y-Loop 
	}   // End X-Loop


  //calculate averages:

  for (unsigned int LineNo=0; LineNo < Energy.size(); LineNo++){
    Avg_Sigma_keV[LineNo]=Sum_Sigma_keV[LineNo]/N_Sigma[LineNo];
    /*
			mdebug<<endl;
			mdebug <<" Sum_Sigma_keV="<<Sum_Sigma_keV[LineNo];
			mdebug <<" N_Sigma="<<N_Sigma[LineNo];
			mdebug <<" Avg_Sigma_keV="<<Avg_Sigma_keV[LineNo];
    */
  }
  Avg_Offset=Sum_Offset/N_Offset;   


  //mdebug<<"Avg_Offset="<<Avg_Offset<<endl;

  cal->Close();
  delete cal;
		

  for (unsigned int i=0;i<strip;i++) {
    mdebug <<endl;
    mdebug <<"strip "<< ((strip_x[i] == 0) ? "N " : "P ") <<strip_y[i];
    mdebug <<" ("<<i+1<<" of "<<strip<<") has "<<NGoodLines[i]<<" good lines."<<endl;

		// No Good Peaks (NGoodLines==0) indicates a defective strip, so
		// we set the Error-Marker
    if (NGoodLines[i]==0) {
      mdebug<<" No good lines. Strip marked as defunct."<<endl;
      out<<"CB "<< ((strip_x[i] == 0) ? "N " : "P ")<<strip_y[i]<<" -1 -1"<<endl;
      out<<"OF "<< ((strip_x[i] == 0) ? "N " : "P ")<<strip_y[i]<<" -1"<<endl;
    }

    //More than one good Peaks indicate: No Problem, go ahead!
    if (NGoodLines[i]>1) {
      out<<"CB "<< ((strip_x[i] == 0) ? "N " : "P ")<<strip_y[i]<<" "
         <<Final_Off_vec[i]<<" "<<Final_Grad_vec[i]<<endl;
      out<<"OF "<< ((strip_x[i] == 0) ? "N " : "P ")<<strip_y[i]<<" "<<Final_Overflow[i]<<endl;		
      mdebug<<" logged Offset "<<Final_Off_vec[i]<<" Gradient "<<Final_Grad_vec[i]
            <<" and Overflow "<<Final_Overflow[i]<<" to file."<<endl;
    }

    //Just one good Peak.
    //This is the most tricky case. We set the good Energy and the
    //Average Offset as Datapoints and do the LinFit again.
    if (NGoodLines[i]==1){
      //Do the new fit:
      LinFitCanvas->Clear();
      LinFit = new TGraph(2);
      LinFit->SetPoint(0,0,Avg_Offset);

      for (unsigned int j=0;j<Energy.size();j++)  //Which Line was the correct one? 
        //Only one should be greater than zero.
        if (En_Mean_vec_vec[j][i]>0)
          LinFit->SetPoint(1,En_Mean_vec_vec[j][i],Energy[j]);
      LinFit->Draw("ALP");
      //LinFit->GetYaxis()->SetLimits(0,  Energy_max);
      //LinFit->GetXaxis()->SetLimits(0, En_Mean_max );
      //LinFit->GetYaxis()->UnZoom();
      //LinFit->GetXaxis()->UnZoom();
      //LinearFit->SetRange(0, En_Mean_max);
      LinFit->Fit("LinearFit", "RQ");
      //LinFit->SetPoint(E_vec.size(), 0, 0);
      LinFit->SetMarkerStyle(3);
				
      LinFit->Draw("AP");
      LinFitCanvas->Update();

      //Now output
      out<<"CB "<< ((strip_x[i] == 0) ? "N " : "P ")<<strip_y[i]<<" "<<
        LinearFit->GetParameter(0)<<" "<<LinearFit->GetParameter(1)<<endl;
      out<<"OF "<< ((strip_x[i] == 0) ? "N " : "P ")<<strip_y[i]<<" "<<Final_Overflow[i]<<endl;		
      mdebug<<" old Offset: "<<Final_Off_vec[i]<<" old Gradient: "<<Final_Grad_vec[i]<<endl;
      mdebug<<" logged recalculated Offset "<<LinearFit->GetParameter(0)<<" Gradient "<<LinearFit->GetParameter(1)
            <<" and Overflow "<<Final_Overflow[i]<<" to file."<<endl;

				
      if (Stop == 2) OneAtATime=kTRUE;
      else OneAtATime=kFALSE;
      if (WaitForKeyPress(OneAtATime)) return true;
    }
  }

  //Last step: Log the average Sigma for each Line

  out<<endl;
  mdebug<<endl;
  for (unsigned int LineNo = 0; LineNo < Energy.size(); ++LineNo) {
    out<<"AS "<<Energy[LineNo]<<" "<<Avg_Sigma_keV[LineNo]<<endl;
    mdebug<< "Average Sigma for Energy "<<Energy[LineNo]<<"keV is "<<Avg_Sigma_keV[LineNo]<<"keV"<<endl;
  }


	out.close();
  
  return true;
}

/************************************************************************
  Find limits of Peak
************************************************************************/

void FindPeakBorders(TH1S* Hist, Int_t& lower, Int_t& upper,
										 Double_t LowerBorder, Double_t UpperBorder, Int_t& PThresh)
{

	Float_t MinRelative = 0.3 ;		// the Maximum must have at least MinRelative * Max(hist)
	Int_t lowest;                 // lowest bin in hist
	Double_t max,max2;                 // value of (local) maximum
	Int_t binmax,binmax2;                 // position of maximum in bins

	// In a smoothed copy of the histogram the finding of a peak is easier
	TH1D* smoothed = 0;
	smoothed = (TH1D*) (Hist->Clone());
	smoothed->SetName("smoothed"); 
	smoothed->Smooth(20);

	//Sometimes there is a very large peak at the lower end of the spectrum,
	//sometimes there isn't.
	//If there is _no_ such peak, the threshold should be higher!

	Int_t temp_last=smoothed->GetXaxis()->GetLast();
	Int_t temp_first=smoothed->GetXaxis()->GetFirst();

	//find absolute max
	binmax = smoothed->GetMaximumBin();
	max = smoothed->GetMaximum();

	//Now reduce range to 5 bins after the max
	smoothed->GetXaxis()->SetRange(binmax+5, temp_last);

	//and find absolute max in this range
	binmax2 = smoothed->GetMaximumBin();
	max2 = smoothed->GetMaximum();


	//mdebug << "Absoute Max is at "<< smoothed->GetXaxis()->GetBinCenter(binmax);
	//mdebug << " and is "<< max << "high."<<endl;

	//mdebug << "Next Max is at "<< smoothed->GetXaxis()->GetBinCenter(binmax2);
	//mdebug << " and is "<< max2 << "high."<<endl;
	
	if (max < 2* max2) { // if there is no large peak at the beginning
		PThresh = (int) (max * MinRelative);
		//mdebug <<" No large Peak at beginning detected. Setting threshold to "<< PThresh << endl;
	} else {
		PThresh = (int) (max2 * MinRelative);
		//mdebug << "Found large Peak at beginning. Setting threshhold to "<< PThresh << endl;
	}
	
	//Reset Range
	smoothed->GetXaxis()->SetRange(temp_first, temp_last);
	
	
	
	
	
	
	// Set threshold for recognizing a peak

	if ( (UpperBorder != -1) && (LowerBorder != -1) ) {
		smoothed->GetXaxis()->SetRangeUser(LowerBorder, UpperBorder);
	}

	
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
Bool_t FitPeak(TH1S* Hist, TF1* FitFunc, Double_t& Mean, Double_t& Sigma, 
							 Double_t LowerBorder, Double_t UpperBorder)
{
	Bool_t retval = kTRUE;
	Int_t		PThresh;	// Treshhold for accepting peak. calculated in FindPeakBorders.
                    // used here to reject certain false positives.

	// Find photopeak in in histogram
	Int_t LowPhotoBin;  // Low end of photo peak in histo bins
	Int_t HighPhotoBin; // High end of photo peak in histo bins


	//mdebug << "LowerBorder="<<LowerBorder<<" UpperBorder="<< UpperBorder<<endl;
  // 		if ((LowerBorder == -1) || (UpperBorder == -1)) { 
	FindPeakBorders(Hist, LowPhotoBin, HighPhotoBin, LowerBorder, UpperBorder, PThresh);
	FitFunc->SetRange(Hist->GetBinCenter(LowPhotoBin), Hist->GetBinCenter(HighPhotoBin));
	//mdebug << "LowerBorder="<<LowPhotoBin<<" UpperBorder="<<HighPhotoBin <<endl;
  // 		} else {
  // 			FitFunc->SetRange(LowerBorder, UpperBorder);
  // 		}
	Hist->Fit("Energy", "RQ");
	Mean = FitFunc->GetParameter(1);
	Sigma = FitFunc->GetParameter(2);
	//mdebug << "Fitrange: " << Hist->GetBinCenter(LowPhotoBin) << "-" << Hist->GetBinCenter(HighPhotoBin) ;
	//mdebug << " = " << Hist->GetBinCenter(HighPhotoBin)-Hist->GetBinCenter(LowPhotoBin) << endl;
	//mdebug << "Fit Mean: "<<Mean<<" Sigma: "<<Sigma<<endl;
	//mdebug << "Lower Limit: " << LowerLimit << " Upper Limit: " << UpperLimit << endl;




	// Find software threshold for noise. 
	// Use Threshold level calculated in FindPeakBorders
	Int_t NoiseThreshBin;
	Axis_t NoiseThresh;
	for (NoiseThreshBin = Hist->GetXaxis()->GetFirst();
			 NoiseThreshBin < Hist->GetXaxis()->GetLast();
			 ++NoiseThreshBin) {
		if (Hist->GetBinContent(NoiseThreshBin) > PThresh) break;
	}
	NoiseThresh = Hist->GetBinCenter(NoiseThreshBin);


  //Sanity-Checks for Fit:

	//This Test was for the other detectors -> CLEANUP
	// Photopeak truncated?
	if ( (Mean - 1.5 * Sigma) >= NoiseThresh) {
		FitFunc->SetLineColor(3);
	} else {
		FitFunc->SetLineColor(2);
		mdebug << "Peak not accepted, because photo peak is truncated!" << endl;
		retval = kFALSE;
	}


  // Peak must be well inside range. We want it one sigma inside.
	//Hmmm... but what _is_ the range??? need to think, before implementing this!
	/*
    if (retval) if ( ((Mean-Sigma) > LowerLimit) && ((Mean+Sigma) < UpperLimit)) {
		FitFunc->SetLineColor(3);
    }	else {
		FitFunc->SetLineColor(2);
		mdebug << "Peak NOT accepted, because out of range!"<< endl;
		retval = kFALSE;
    }
	*/
	//Peak must be sharp
	if (retval) if (Sigma < (Hist->GetBinCenter(HighPhotoBin) - Hist->GetBinCenter(LowPhotoBin)) ) {
		FitFunc->SetLineColor(3);
	} else {
		FitFunc->SetLineColor(2);
		mdebug << "Peak NOT accepted, because not sharp!"<<endl;
		retval = kFALSE;
	}

	//Fitrange must not be too few bins
	if (retval) if ( HighPhotoBin-LowPhotoBin > 3 ) {
		FitFunc->SetLineColor(3);
	} else {
		FitFunc->SetLineColor(2);
		mdebug <<"Peak NOT accepted, because fitrange too small"<<endl;
		retval = kFALSE;
	}



	FitFunc->Draw("LSAME");

	return retval;
}

bool WaitForKeyPress(Bool_t Wait) {
  if (Wait){
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
          return true;
        }
      }
    }
  }
  
  return false;
}

/******************************************************************************/
