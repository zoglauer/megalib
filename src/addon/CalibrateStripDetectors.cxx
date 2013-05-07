/* 
 * CalibrateStripDetectors.cxx
 *
 *
 * Copyright (C) by Florian Schopper, Peter Bloser, Robert Andritschke and Andreas Zoglauer.
 * All rights reserved.
 *
 *
 * This code implementation is the intellectual property of
 * Robert Andritschke, Florian Schopper, Andreas Zoglauer, Peter Bloser.
 *
 * By copying, distributing or modifying the Program (or any work
 * based on the Program) you indicate your acceptance of this statement,
 * and all its terms.
 *
 */

// Standard libs:
#include <iostream>
#include <string>
#include <sstream>
#include <csignal>
#include <vector>
using namespace std;

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
#include <TGraphErrors.h>
#include <TApplication.h>
#include <Getline.h>

// Mega libs
#include "MSiStrip.h"


/******************************************************************************/

class CalibrateStripDetectors
{
public:
  /// Default constructor
  CalibrateStripDetectors();
  /// Default destructor
  ~CalibrateStripDetectors();
  
  /// Parse the command line
  bool ParseCommandLine(int argc, char** argv);
  /// Analyze what eveer needs to be analyzed...
  bool Analyze();
  /// Interrupt the analysis
  void Interrupt() { m_Interrupt = true; }

protected:
  bool FitPeak(TH1S* Hist, TF1* FitFunc, double& Mean, double& Sigma, double LowerBorder, double UpperBorder);
  void FindPeakBorders(TH1S* Hist, int& lower, int& upper, double LowerBorder, double UpperBorder, int& PThresh);
  bool WaitForKeyPress(bool Wait);

private:
  /// True, if the analysis needs to be interrupted
  bool m_Interrupt;

  // Remember difference between Bins and ADC counts!  ~2 ADC cts per Bin for D1
  int m_DisplayMax;     // Max. counts displayed
  bool m_OneAtATime;   // Stop after each fit?

  /// Stop conditions: 0 = no stop (default), 1 = stop on bad fits, 2 = always stop
	int m_StopCondition;  // 0 = no stop (default)

  static const int c_NoStop = 0;
  static const int c_StopOnBadFits = 1;
  static const int c_AlwaysStop = 2;

  int m_FitType;

  static const int c_FitTypeGauss = 0;
  static const int c_FitTypeGaussLandau = 1;


  /// Start strip
	int m_StartStrip;
  /// Start side
	int m_Side;

  static const int c_PSide = 0;
  static const int c_NSide = 1;
  static const int c_BothSides = 2;

  /// File names
	vector<MString> m_FileName;
  /// Energies
	vector<Float_t> m_Energy;
  /// Detector name
	MString m_DetectorName;
  /// Prefix for output file
	MString m_OutputFilePrefix;

  TCanvas *cal;
};


/******************************************************************************/
/******************************************************************************/


/******************************************************************************
 * Parse the command line
 */
double FitGaussLandau(Double_t* x, Double_t* par)
{
  Double_t arg = 0;
  if (par[2] != 0) arg = (x[0] - par[1])/par[2];
  
  if (par[1] < par[4]) {
    return -10000;
  }


  return par[0]*TMath::Exp(-0.5*arg*arg) 
    + TMath::Landau(x[0], par[3], par[4])*par[5]
    + TMath::Landau(x[0], par[6], par[7])*par[8];
}


/******************************************************************************
 * Parse the command line
 */
double FitGaussExp(Double_t* x, Double_t* par)
{
  Double_t arg = 0;
  if (par[2] != 0) arg = (x[0] - par[1])/par[2];
 
  return par[0]*TMath::Exp(-0.5*arg*arg) + par[3] 
    + par[4]*TMath::Exp(par[5]*x[0])
    + par[6]*TMath::Exp(par[7]*x[0]);
}


/******************************************************************************
 * Parse the command line
 */
double FitLandau(Double_t* x, Double_t* par)
{
  return par[0]*TMath::Landau(x[0], par[1], par[2]);
}



/******************************************************************************
 * Parse the command line
 */
double FitNegLandau(Double_t* x, Double_t* par)
{
  return par[0]*TMath::Landau(-x[0], par[1], par[2]);
}






/******************************************************************************/
/******************************************************************************/


/******************************************************************************
 * Default constructor
 */
CalibrateStripDetectors::CalibrateStripDetectors() : m_Interrupt(false)
{
	m_StopCondition = c_NoStop;
	m_StartStrip = 1;
	m_Side = c_BothSides;  

  m_DisplayMax = -1111;
  m_OneAtATime = kFALSE;
	m_OutputFilePrefix="";

  m_FitType = c_FitTypeGaussLandau;
}


/******************************************************************************
 * Default destructor
 */
CalibrateStripDetectors::~CalibrateStripDetectors()
{
  // Intentionally left blanck
}


/******************************************************************************
 * Parse the command line
 */
bool CalibrateStripDetectors::ParseCommandLine(int argc, char** argv)
{
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
  for (int i = 1; i < argc; i++) {
    Option = argv[i];
    if (Option == "-h" || Option == "--help" || Option == "?" || Option == "-?") {
      cout<<Usage<<endl;
      return 1;
    }
  }

  // Now parse the command line options:
	// First check if each option has sufficient arguments:
  for (int i = 1; i < argc; i++) {
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
  for (int i = 1; i < argc; i++) {
		Option = argv[i];

		if (Option == "-v"){
			g_DebugLevel = 1;
			mdebug<<"verbose mode ON"<<endl;
		} else if (Option == "-d") {
			m_DetectorName = argv[++i];
			mdebug<<"Detector name: "<<m_DetectorName<<endl;
		} else if (Option == "-c") {
			m_FileName.push_back(argv[++i]);
			m_Energy.push_back(atoi(argv[++i]));
			mdebug<<"Calibration file "<<m_FileName.back()<<" at "<<m_Energy.back()<<" keV"<<endl;
		} else if (Option == "-e") {
			m_StopCondition = 1;
			mdebug<<"Stop on errors!"<<endl;
		} else if (Option == "-ee") {
			m_StopCondition = 2;
			mdebug<<"Stop after each pixel!"<<endl;
		} else if (Option == "-N") {
			m_Side=0;
			mdebug<<"Do only N-Side"<<endl;
		} else if (Option == "-P") {
			m_Side=1;
			mdebug<<"Do only P-Side"<<endl;
		} else if (Option == "-b") {
			m_StartStrip = atoi(argv[++i]);
			if (m_StartStrip < 1 || m_StartStrip > 384) {
				cout<<endl; 
				cout<<"Valid ranges for the start strip: 1..384" <<endl;
				cout<< endl;
				cout<<"Your choice: "<<m_StartStrip<<"  -> Exit!"<<endl;
				return 1;
			} else {
				mdebug<<"Start strip: x="<<m_StartStrip<<endl;
			}
		} else if (Option == "-p") {
			m_OutputFilePrefix = argv[++i];
			mdebug<<"Using prefix: "<<m_OutputFilePrefix<<endl;
		}
	}

	if (m_Energy.size() == 0) {
		cout<<"You have to give at least one calibration file to work with -> Exit"<<endl;
		cout<<Usage<<endl;
		return 1;
	}

	if (m_DetectorName == "") {
		cout<<"You have to give a detector name!"<<endl;
		cout<<Usage<<endl;
		return 1;
	}

	switch (m_StopCondition) {
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
		mdebug << "What's that, Stop = " << m_StopCondition << "?" << endl;
  }

  return true;
}


/******************************************************************************
 * Do whatever analysis is necessary
 */
bool CalibrateStripDetectors::Analyze()
{

	//Some Variables

	//One Value per Line and per Strip
	//The size of these vectors is sizeof Energy in one direction,
	//And will be determined on the fly in the other direction

	vector<vector<double> > En_Mean_vec_vec;  //Mean Energy for each fit.
	En_Mean_vec_vec.resize(m_Energy.size());


	vector<vector<double> > Overflow_vec_vec;
	Overflow_vec_vec.resize(m_Energy.size());

	//One Value per Strip
	//The size of these vectors will be determined on the fly

	vector<double> Final_Grad_vec;           //Gradient of linear fit for each strip
	vector<double> Final_Off_vec;            //Offset of linear fit for each strip
	vector<double> Final_Overflow;
	vector<unsigned int> NGoodLines;         //Number of Good Lines.
	vector<int> strip_x, strip_y;

	//One Value per Line
	//The size of these vectors is fixed to Energy.size()
	vector<double> Sum_Sigma_keV;
	Sum_Sigma_keV.resize(m_Energy.size(), 0);

	vector<int> N_Sigma;
	N_Sigma.resize(m_Energy.size(), 0);

	vector<double> Avg_Sigma_keV;
	Avg_Sigma_keV.resize(m_Energy.size(), 0);


	//One Value total
	//These are scalars
	double Sum_Offset=0;
	double Avg_Offset=0;
	int N_Offset=0;

  
  // Spectrum Window
  cal = new TCanvas("calibration","Stripdata",1000,500); 

 	//Linear Window
	TCanvas* LinFitCanvas =  
		new TCanvas("Linear Fit Progress", "Linear Fit Progress", 438,330);
	TGraph* LinFit = 0;
	
	// Density Plot of all Linear fits.
	// Used to analyze the Offset and Gradient Distribution.
	/* 
     TCanvas* FitDensityCanvas =
     new TCanvas("FitDensity", "FitDensity", 400, 400);
     TH2D* FitDensity = new TH2D("Density", "Density", 200, 0, 1000, 200, 0, 200);
	*/

	//Use several Lines in several Files
	vector<TFile *> Files;
	Files.resize(m_FileName.size());
	mdebug << endl << "Opening files... " << endl << "  ";
	for (unsigned int i=0; i<m_FileName.size(); ++i) {  // Loop over lines/files
		mdebug << m_FileName[i] << ", ";
		Files[i] = new TFile(m_FileName[i], "READ");
		if (!(Files[i]->IsOpen())) {
			mdebug << endl << "Cannot open file \"" << m_FileName[i] << "\"!" << endl;
			return kFALSE;
		}
    Files[i]->ls();
	}


	vector<MSiStrip *> Det;
	Det.resize(Files.size());
	for (unsigned int i=0; i<Files.size(); ++i) {
		Det[i]= (MSiStrip*) Files[i]->Get(m_DetectorName.Data());
		if ( Det[i]==0 ) {  //Does that work? Not sure if we've got the pointers right, but seems to work.
			cout << "Detector "<<m_DetectorName.Data()<<" not found. Aborting."<<endl;
			return false;
		}
	}

	// Open the output-file:
	MString Dummy(m_DetectorName);
	Dummy.Prepend(m_OutputFilePrefix);
	Dummy.Append("_calibrate.csv");
	mdebug <<"Opening output file: "<< Dummy << endl;
	fstream out;
	out.open(Dummy.Data(), ios::out);
	
	// Header info
	out<< "Type             cvs_sistrip"<<endl;
	out<< "Version          3"<<endl; 
	out<< endl;
	out<< "DetectorName     "<<m_DetectorName.Data()<<endl;
	for (unsigned int i=0;i<Files.size();++i){
		out<<"CalibFileName     "<<m_FileName[i].Data()<<endl;
	}
	MTime Time;
	out<<"Date             "<<Time.GetSQLString()<<endl; 
	out<<endl;
	
	
  //Counter to run over all strips.
	unsigned int strip=0;

	//Fit functions
	TF1* EnergyFit = 0;  //Gaus-fit for Photopeak
  if (m_FitType == c_FitTypeGauss) {
    EnergyFit = new TF1("Energy", "gaus"); // ranges set in PeakFit function
  } else {
    EnergyFit = new TF1("Energy", FitGaussLandau, 1, 600, 6); // ranges set in PeakFit function
  }
	TF1* LinearFit = 0;  //Linear Fit as Energy interpolation between lines
	LinearFit = new TF1("LinearFit", "pol1");


	//Loop over all strips
	// x: 0,1    y: 1,384
  for(int x = 0 ; x <= 1 ; x++ ) {
    if (m_Side==0 && x == 1) continue;
    if (m_Side==1 && x == 0) continue;
		for(int y = m_StartStrip ; y <= 384 ; y++ ) {

      if (m_Interrupt == true) return false;
			
      mdebug << endl << endl;
			mdebug << "\e[43mFitting strip " << (( x==0 ) ? "N " : "P ") << y << "\e[0m" << endl;


			//Variables
				
			//Main Histogram
			TH1S* MyHist = 0;  
				
			//Vectors for the values of each fitted line
			vector<double> Mean;
			Mean.resize(Files.size());
				
			vector<double> Sigma;
			Sigma.resize(Files.size());
				
			vector<int> Overflow;
			Overflow.resize(Files.size());
				

			double GuessedLowerEnergy = -1; // shrink fitrange for second ff. lines
			double GuessedUpperEnergy = -1;

			Final_Overflow.push_back(1024); //Initialize Final Overflow bin to max

				
			// Loop over different lines
			for (unsigned int LineNo=0; LineNo < m_Energy.size(); ++LineNo) { // lines loop

				//Set Name of Window
				char Title[100];
				sprintf(Title, "%s: Data of strip: %s %d, E=%f", 
								m_DetectorName.Data(), (x == 0) ? "N" : "P" , y, m_Energy[LineNo]);
				cal->SetTitle(Title);
				sprintf(Title, "%s_DataOfPixel_x%d_y%d", m_DetectorName.Data(), x, y);
				cal->SetName(Title);
				

				// Get calib histogram for this strip
				if (x == 0) {
					MyHist = Det[LineNo]->GetCalibHist(kTRUE, y);
				}
				if (x == 1) {
					MyHist = Det[LineNo]->GetCalibHist(kFALSE, y); 
				}
        
        if (MyHist == 0) {
          mout<<"Unable to find data for detector: "<<m_DetectorName<<endl;
          return false;
        }

        //MyHist->Rebin(2);
        //MyHist->Smooth(20);


				// if the first line delivered an energy calibration, use it to shrink ranges
				double Line0Energy;

				if (LineNo != 0) {
					//mdebug << "First Energy was: Mean[0]="<<Mean[0]<<endl;
					//mdebug << "First Sigma was: Sigma[0]="<<Sigma[0]<<endl;
					double GuessedOffset=20 ;
					if (N_Offset >0) GuessedOffset=Sum_Offset/N_Offset;
					//mdebug<<"GuessedOffset "<<GuessedOffset<<endl;

					if ( Mean[0] > 0 ) {

						Line0Energy = (Mean[0]-GuessedOffset) * m_Energy[LineNo] / m_Energy[0] + 
							GuessedOffset;
						//mdebug << "Projected new Energy is "<<Line0Energy<<endl;

						GuessedLowerEnergy = Line0Energy - 3.0*Sigma[0];    //three sigma around
						GuessedUpperEnergy = Line0Energy + 3.0*Sigma[0];    //prognosis of new energy
						if (GuessedLowerEnergy < 0) GuessedLowerEnergy = 0;
					}
				}


				// First off, get overflow ADC: the highest bin with something in it
				int dummy1, oben1=( (int) MyHist->GetXaxis()->GetNbins() )-1;
				while ( (dummy1=(int) MyHist->GetBinContent(oben1--) == 0) && (oben1 > 1));
				
				Overflow[LineNo] =(int) MyHist->GetXaxis()->GetBinCenter(oben1);

				cal->cd();
				// Added for compatibilty with ROOT 3.03.03+
				MyHist->SetStats(kFALSE);
				MyHist->SetMaximum(m_DisplayMax);
				MyHist->Draw() ;
				


				bool GoodFit=kTRUE;
				
				Mean[LineNo]=0;
				Sigma[LineNo]=0;
				GoodFit=FitPeak(MyHist,EnergyFit,Mean[LineNo],Sigma[LineNo],GuessedLowerEnergy,GuessedUpperEnergy);

				cal->Update();
				
				// Fit schlecht?

				if (GoodFit) {
					//Save Fit-Data for Later
					En_Mean_vec_vec[LineNo].push_back(Mean[LineNo]);

					//prepare averaging of Sigma
					Sum_Sigma_keV[LineNo] += Sigma[LineNo]*m_Energy[LineNo]/En_Mean_vec_vec[LineNo][strip];
					N_Sigma[LineNo]++;
					//mdebug <<" Sum_Sigma_keV="<<Sum_Sigma_keV[LineNo];
					//mdebug <<" N_Sigma="<<N_Sigma[LineNo];


					//Determine Final (minimal) Overflow bin
					if (Overflow[LineNo] < Final_Overflow[strip]) 
						Final_Overflow[strip] = Overflow[LineNo];
					
					mdebug<<"Found Energy "<<m_Energy[LineNo]<<"keV at "<<Mean[LineNo];
					mdebug<<" with a Sigma of "<<Sigma[LineNo]*m_Energy[LineNo]/En_Mean_vec_vec[LineNo][strip];
					mdebug <<"keV ("<<Sigma[LineNo]<<" bins)"<<endl;
				} else {
					En_Mean_vec_vec[LineNo].push_back(-1);
					mdebug<<"=> Rejected Line at Energy "<<m_Energy[LineNo]<<"keV"<<endl;
				}

				//Now do the Fit over all Lines to get final offset and gradient
				//fill the TGraph:
				
				//Determine maxima in both axes to scale graph properly
				//Try to leave scale untouched as long as possible to avoid ugly jumps
				
				double En_Mean_max=1, Energy_max=1;
				//Maximum Energy: Y-Axis (+20% to be on the safe side)
				unsigned int Energy_max_num = 0;
				for (unsigned int i=0; i <= m_Energy.size();i++)
					if (m_Energy[i] > Energy_max){
						Energy_max=1.2*m_Energy[i];
						Energy_max_num=i;
					}
				//mdebug << "Energy_max="<<Energy_max<<" Energy_max_num="<<Energy_max_num<<endl;

				//Corresponding fit: X-Axis
				//take first good value to guess range (+20% to be on the safe side)
				for (unsigned int i=0; i <= LineNo; i++) {
					if (En_Mean_vec_vec[i][strip] != -1){
						En_Mean_max = 1.2 * MyHist->GetXaxis()->GetBinCenter((int) (Energy_max*En_Mean_vec_vec[i][strip]/m_Energy[i]));
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
						LinFit->SetPoint(count++, En_Mean_vec_vec[i][strip], m_Energy[i]);
					} else {
						//mdebug <<"Energy "<< m_Energy[i]<<" at bin " <<En_Mean_vec_vec[i][strip]<<" rejected"<< endl;
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



				if (LineNo==m_Energy.size()-1){  //Last Line
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
				if (m_StopCondition==0) m_OneAtATime=kFALSE;
				if (m_StopCondition==1)
					if (GoodFit) m_OneAtATime=kFALSE;
					else m_OneAtATime=kTRUE;
				if(m_StopCondition==2) m_OneAtATime=kTRUE;

				if (WaitForKeyPress(m_OneAtATime)) return true;


			} //End Line-Loop

			mdebug<<endl;

			strip_x.push_back(x);
			strip_y.push_back(y);
			strip++;  //Count the strips
		} // End Y-Loop 
	}   // End X-Loop


  //calculate averages:

  for (unsigned int LineNo=0; LineNo < m_Energy.size(); LineNo++){
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

      for (unsigned int j=0;j<m_Energy.size();j++)  //Which Line was the correct one? 
        //Only one should be greater than zero.
        if (En_Mean_vec_vec[j][i]>0)
          LinFit->SetPoint(1,En_Mean_vec_vec[j][i],m_Energy[j]);
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

				
      if (m_StopCondition == 2) m_OneAtATime=kTRUE;
      else m_OneAtATime=kFALSE;
      if (WaitForKeyPress(m_OneAtATime)) return true;
    }
  }

  //Last step: Log the average Sigma for each Line

  out<<endl;
  mdebug<<endl;
  for (unsigned int LineNo = 0; LineNo < m_Energy.size(); ++LineNo) {
    out<<"AS "<<m_Energy[LineNo]<<" "<<Avg_Sigma_keV[LineNo]<<endl;
    mdebug<< "Average Sigma for Energy "<<m_Energy[LineNo]<<"keV is "<<Avg_Sigma_keV[LineNo]<<"keV"<<endl;
  }

	out.close();

  return true;
}


/******************************************************************************
 * Find the peak borders
 */
void CalibrateStripDetectors::FindPeakBorders(TH1S* Hist, 
                                              int& lower, 
                                              int& upper,
                                              double LowerBorder, 
                                              double UpperBorder, 
                                              int& PThresh)
{

	Float_t MinRelative = 0.3 ;		// the Maximum must have at least MinRelative * Max(hist)
	int lowest;                 // lowest bin in hist
	double max,max2;                 // value of (local) maximum
	int binmax,binmax2;                 // position of maximum in bins

	// In a smoothed copy of the histogram the finding of a peak is easier
	TH1D* smoothed = 0;
	smoothed = (TH1D*) (Hist->Clone());
	smoothed->SetName("smoothed"); 
	smoothed->Smooth(20);

	//Sometimes there is a very large peak at the lower end of the spectrum,
	//sometimes there isn't.
	//If there is _no_ such peak, the threshold should be higher!

	int temp_last=smoothed->GetXaxis()->GetLast();
	int temp_first=smoothed->GetXaxis()->GetFirst();

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
	int downct = 0;  // counter, how often we climb down while going lower
	double mem1;     // memories for values
	double mem2 = max;

	while ( lower > lowest && downct < 2 ) {
		if ( (mem1 = (int) smoothed->GetBinContent(--lower)) >= mem2) {
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
	int hwhm;
	int mem3;
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
bool CalibrateStripDetectors::FitPeak(TH1S* Hist, TF1* FitFunc, double& Mean, double& Sigma, 
							 double LowerBorder, double UpperBorder)
{
	bool retval = kTRUE;
	int		PThresh;	// Treshhold for accepting peak. calculated in FindPeakBorders.
                    // used here to reject certain false positives.

  // Fit functions: 
  TF1* LandauFit = 0;
  TF1* NegLandauFit = 0;
  TF1* GaussFit = 0;
  TF1* MyFit = 0;

  // Min and Max Bins:
  double MinBin = 0.0;
  double MaxBin = 0.0;
  for (int bin = 1; bin <= Hist->GetNbinsX(); bin++) {
    if (Hist->GetBinContent(bin) > 1) {
      if (MinBin <= 0) Hist->GetBinCenter(bin);
      MaxBin = Hist->GetBinCenter(bin);
    }
  }
  cout<<"Min: "<<MinBin<<" max: "<<MaxBin<<endl;

	TH1D* smoothed = 0;
	smoothed = (TH1D*) (Hist->Clone());
	smoothed->SetName("smoothed"); 
	smoothed->Smooth(20);
	smoothed->Rebin(2);

  // Step 1: Fit Landau
  cout<<"LandauFit: "<<endl;
  LandauFit = new TF1("LandauFit", FitLandau, MinBin, MaxBin, 3);
  LandauFit->SetParameter(0, 100.0);
  //LandauFit->SetParameter(1, Hist->GetBinCenter(Hist->GetMaximumBin()));
  LandauFit->SetParameter(2, 15.0);
  smoothed->Fit(LandauFit, "R");
  cal->cd();
  smoothed->Draw();
  cal->Update();
  if (WaitForKeyPress(m_OneAtATime)) return true;
  
  // Substract first result:
  double Integral = 0.0;
  for (Int_t bin = 1; bin <= smoothed->GetNbinsX(); bin++) {
    Float_t x = smoothed->GetBinCenter(bin);
    Double_t fval = LandauFit->Eval(x);
    Double_t diff = smoothed->GetBinContent(bin) - fval;
    if (diff < 0) diff = 0;
    smoothed->SetBinContent(bin, diff);
    Integral += diff;
   } 

  if (Integral < 20) {
    mout<<"Not enough counts!"<<endl;
    retval = false;
  } else {
    // Step 1.5: Fit Landau negative
    cout<<"LandauFit: "<<endl;
    NegLandauFit = new TF1("NegLandauFit", FitNegLandau, LandauFit->GetParameter(1), MaxBin, 3);
    NegLandauFit->SetParameter(0, LandauFit->GetParameter(0));
    //NegLandauFit->SetParameter(1, -2*LandauFit->GetParameter(1));
    NegLandauFit->SetParameter(2, LandauFit->GetParameter(2));
    //NegLandauFit->SetParLimits(0, 0.0, LandauFit->GetParameter(0)*5.0);
    NegLandauFit->SetParLimits(1, -LandauFit->GetParameter(1)*5.0, 10.0);
    NegLandauFit->SetParLimits(2, 0.0, LandauFit->GetParameter(2)*2.0);
    smoothed->Fit(NegLandauFit, "R");
    cal->cd();
    smoothed->Draw();
    cal->Update();
    if (WaitForKeyPress(m_OneAtATime)) return true;
    

    // Step 2: Fit Gauss
    cout<<"GaussFit: "<<endl;
    GaussFit = new TF1("GaussFit", "gaus"); // ranges set in PeakFit function
    //FindPeakBorders(Hist, LowPhotoBin, HighPhotoBin, LowerBorder, UpperBorder, PThresh);
    //GaussFit->SetRange(Hist->GetBinCenter(LowPhotoBin), Hist->GetBinCenter(HighPhotoBin));
    GaussFit->SetRange(NegLandauFit->GetMaximumX(0.0, 800.0) - 6*NegLandauFit->GetParameter(2), 
                       NegLandauFit->GetMaximumX(0.0, 800.0) + 6*NegLandauFit->GetParameter(2));
    GaussFit->SetParameter(0, 1.0);
    GaussFit->SetParameter(1, NegLandauFit->GetMaximumX(0.0, 800.0));
    GaussFit->SetParameter(2, NegLandauFit->GetParameter(2));
  
    cout<<"Max: "<<NegLandauFit->GetMaximumX(0.0, 800.0)<<endl;
    cout<<"Sigma: "<<NegLandauFit->GetParameter(2)<<endl;
    cout<<"Range: "<<NegLandauFit->GetMaximumX(0.0, 800.0) - 0.5*NegLandauFit->GetParameter(2)<<" - "
        <<NegLandauFit->GetMaximumX(0.0, 800.0) + 2*NegLandauFit->GetParameter(2)<<endl;
    
    smoothed->Fit(GaussFit, "R");
    cal->cd();
    smoothed->Draw();
    cal->Update();
    if (WaitForKeyPress(m_OneAtATime)) return true;
  
    // Step 3: 2*Exp and Gauss

    MyFit = new TF1("A fit", FitGaussExp, 1.1*LandauFit->GetMaximumX(0.0, 800.0), MaxBin, 8);
    MyFit->SetParName(0, "Gauss - height");
    MyFit->SetParameter(0, GaussFit->GetParameter(0));
    MyFit->SetParLimits(0, 0.0, 1000000.0);
    MyFit->SetParName(1, "Gauss - mean");
    MyFit->SetParameter(1, GaussFit->GetParameter(1));
    //MyFit->SetParLimits(1, Hist->GetBinCenter(LowPhotoBin), Hist->GetBinCenter(HighPhotoBin));
    MyFit->SetParName(2, "Gauss - sigma");
    MyFit->SetParameter(2, GaussFit->GetParameter(2));
    MyFit->SetParLimits(2, 0.0, 1000000.0);
    MyFit->SetParName(3, "Constant");
    MyFit->SetParameter(3, 0.0);
    MyFit->FixParameter(3, 0.0);
    MyFit->SetParName(4, "Exp - const");
    MyFit->SetParameter(4, 5.0);
    MyFit->SetParName(5, "Exp - slope");
    MyFit->SetParameter(5, -0.02);
    MyFit->SetParLimits(5, -1.0, 0.0);
    MyFit->SetParName(6, "Exp2 - const");
    MyFit->SetParameter(6, 5.0);
    MyFit->SetParName(7, "Exp2 - slope");
    MyFit->SetParameter(7, -0.02);
    MyFit->SetParLimits(7, -1.0, 0.0);
    Hist->Fit(MyFit, "R");
 
    Mean = MyFit->GetParameter(1);
    Sigma = MyFit->GetParameter(2);

    //     // Step 3: Fit 2*Landau + Gauss:
    //     cout<<"BothFit: "<<endl;
    //     TF1* MyFit = new TF1("A fit", FitGaussLandau, LandauFit->GetMaximumX(0.0, 800.0), 700.0, 9);
    //     MyFit->SetParName(0, "Gauss - height");
    //     MyFit->SetParameter(0, GaussFit->GetParameter(0));
    //     MyFit->SetParLimits(0, 0.0, 1000000);
    //     MyFit->SetParName(1, "Gauss - mean");
    //     MyFit->SetParameter(1, GaussFit->GetParameter(1));
    //     MyFit->SetParLimits(1, Hist->GetBinCenter(LowPhotoBin), Hist->GetBinCenter(HighPhotoBin));
    //     MyFit->SetParName(2, "Gauss - sigma");
    //     MyFit->SetParameter(2, GaussFit->GetParameter(2));
    //     MyFit->SetParLimits(2, 0.0, 1000000);
    //     MyFit->SetParName(3, "Landau - mvp");
    //     MyFit->SetParameter(3, LandauFit->GetParameter(1));
    //     MyFit->SetParLimits(3, 0.0, 1000000);
    //     MyFit->SetParName(4, "Landau - sigma");
    //     MyFit->SetParameter(4, LandauFit->GetParameter(2));
    //     MyFit->SetParLimits(4, 0.0, 1000000);
    //     MyFit->SetParName(5, "Landau - heigth");
    //     MyFit->SetParameter(5, LandauFit->GetParameter(0));
    //     MyFit->SetParLimits(5, 0.0, 1000000);
    //     MyFit->SetParName(6, "Landau2 - mvp");
    //     MyFit->SetParameter(6, LandauFit->GetParameter(1)*1.2);
    //     MyFit->SetParLimits(6, 0.0, 1000000);
    //     MyFit->SetParName(7, "Landau2 - sigma");
    //     MyFit->SetParameter(7, LandauFit->GetParameter(2)*1.2);
    //     MyFit->SetParLimits(7, 0.0, 1000000);
    //     MyFit->SetParName(8, "Landau2 - heigth");
    //     MyFit->SetParameter(8, LandauFit->GetParameter(0)*1.2);
    //     MyFit->SetParLimits(8, 0.0, 1000000);
    //     Hist->Fit(MyFit, "R");
 
    Mean = MyFit->GetParameter(1);
    Sigma = MyFit->GetParameter(2);
  
    FitFunc->SetParameters(MyFit->GetParameter(0), MyFit->GetParameter(1), MyFit->GetParameter(2));
  }

  // Find software threshold for noise. 
  // Use Threshold level calculated in FindPeakBorders
  int NoiseThreshBin;
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

  // First landau and last gauss must differ at least 10 %
  if (retval == true) {
    cout<<"Mean/LandauFit->GetMaximumX: "<<Mean/LandauFit->GetMaximumX(0.0, 800.0)<<endl;
    if (!(Mean/LandauFit->GetMaximumX(0.0, 800.0) > 1.5 ||
          Mean - LandauFit->GetMaximumX(0.0, 800.0) > 10)) {
    mout<<"Gauss must be at least 10% larger than first landau!"<<endl;
    retval = false;
    }
  }

// 	// Peak must be sharp
// 	if (retval == true) {
//     if (Sigma < Hist->GetBinCenter(HighPhotoBin) - Hist->GetBinCenter(LowPhotoBin)) {
//       FitFunc->SetLineColor(3);
//     } else {
//       FitFunc->SetLineColor(2);
//       mdebug << "Peak NOT accepted, because not sharp!"<<endl;
//       retval = kFALSE;
//     }
//   }

// 	// Fitrange must not be too few bins
// 	if (retval) {
//     if ( HighPhotoBin-LowPhotoBin > 3 ) {
//       FitFunc->SetLineColor(3);
//     } else {
//       FitFunc->SetLineColor(2);
//       mdebug <<"Peak NOT accepted, because fitrange too small"<<endl;
//       retval = kFALSE;
//     }
//   }

    
  FitFunc->Draw("LSAME");

  delete MyFit;
  delete LandauFit;
  delete NegLandauFit;
  delete GaussFit;

	return retval;
}


/******************************************************************************
 * Wait until a key has been pressed
 */
bool CalibrateStripDetectors::WaitForKeyPress(bool Wait) 
{
  if (Wait == true) {
    char* input = 0;
    TTimer* timer = new TTimer("gSystem->ProcessEvents();", 50, kFALSE);
    
    while (Wait == true) {
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

CalibrateStripDetectors* g_Prg = 0;

/******************************************************************************/


/******************************************************************************
 * Called when an interrupt signal is flagged
 * All catched signals lead to a well defined exit of the program
 */
void CatchSignal(int a)
{
  cout<<"Catched signal Ctrl-C (ID="<<a<<"):"<<endl;
  
  if (g_Prg != 0) {
    g_Prg->Interrupt();
  }
}


/******************************************************************************
 * Main program
 */
int main(int argc, char** argv)
{
  void (*handler)(int);
	handler = CatchSignal;
  (void) signal(SIGINT, CatchSignal);

  TApplication PrgApp("CalibrateStripDetectors", 0, 0);

  g_Prg = new CalibrateStripDetectors();

  if (g_Prg->ParseCommandLine(argc, argv) == false) {
    cerr<<"Error during parsing of command line!"<<endl;
    return -1;
  } 
  if (g_Prg->Analyze() == false) {
    cerr<<"Error during analysis!"<<endl;
    return -2;
  } 

  cout<<"Program exited normally!"<<endl;

  return 0;
}


/*
 * CalibrateStripDetectors: the end...
 ******************************************************************************/
