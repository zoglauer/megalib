/*
 * MCalibrateZ.cxx                                   v1.0  05/11/2002
 *
 *
 * Copyright (C) by Robert Andritschke, Florian Schopper, Andreas Zoglauer.
 * All rights reserved.
 *
 *
 * This code implementation is the intellectual property of
 * Andreas Zoglauer.
 *
 * By copying, distributing or modifying the Program (or any work
 * based on the Program) you indicate your acceptance of this statement,
 * and all its terms.
 *
 */


/******************************************************************************
 *
 * MCalibrateZ
 *
 * Usage examples:
 *
CalibrateZ -d daphne -d aphrodite -d xantippe -d athene -e ~/Bildrekonstruktion/Data/Muonen/20021025_181803_MuonMeasurement/MuonMeasurement.uevta -e ~/Bildrekonstruktion/Data/Muonen/20021029_191429_MuonMeasurement/MuonMeasurement.uevta -e ~/Bildrekonstruktion/Data/Muonen/20021030_214556_MuonMeasurement/MuonMeasurement.uevta -e ~/Bildrekonstruktion/Data/Muonen/20021104_203646_MuonMeasurement/MuonMeasurement.uevta -e ~/Bildrekonstruktion/Data/Muonen/20021106_101545_MuonMeasurement/MuonMeasurement.uevta -s setupfiles/20021009_MEGA.setup
*
CalibrateZ -d daphne -r CalibrateZ_All64.root -s setupfiles/20020813_MEGA.setup
*
******************************************************************************/


// Standard libs:
#include <stdlib.h>
#include <vector>
#include <list>
#include <cmath>
using namespace std;

// ROOT libs:
#include "MString.h"
#include "TSystem.h"
#include "TH2.h"
#include "TVector3.h"
#include "TApplication.h"
#include "TCanvas.h"
#include "TFile.h"
#include "TStyle.h"
#include "TF1.h"
#include "TGClient.h"
#include "TGButton.h"
#include "TGFrame.h"
#include "TMath.h"

// Mega libs:
#include "MStreams.h"
#include "MDaq.h"
#include "MEventData.h"
#include "MCsIPixelDouble.h"
#include "MTime.h"
#include "MFile.h"
#include "MGUIDialog.h"


/******************************************************************************/

class MCalibrateZ;

/******************************************************************************/

class MGUIFitting : public TGMainFrame
{
  // Public Interface:
 public:
  MGUIFitting(const TGWindow* p, const TGWindow* main, MCalibrateZ* Calibration);
  virtual ~MGUIFitting();

  void SetStart(unsigned int Detector, unsigned int Crystal);

  // protected methods:
 protected:
  void Create();
  Bool_t ProcessMessage(Long_t Message, Long_t Parameter1, Long_t Parameter2);
  void CloseWindow();

	Bool_t OnNext();
	Bool_t OnLoop();
	Bool_t OnStop();
	Bool_t OnClose();

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  TGCompositeFrame* m_MainFrame;
  TGLayoutHints* m_MainFrameLayout;

  TGTextButton* m_NextButton;
  TGTextButton* m_CloseButton;
  TGTextButton* m_LoopButton;
  TGTextButton* m_StopButton;
  TGLayoutHints* m_ButtonLayout;

  MCalibrateZ* m_Calibration;

  unsigned int m_Detector;
  unsigned int m_Crystal;

  bool m_StopLoop;

  enum { e_Next = 100, e_Loop, e_Stop, e_Close };
};

/******************************************************************************/

class MCalibrateZ
{
public:
  MCalibrateZ(MString OutputFileName, MString SetupFileName);
  ~MCalibrateZ() {};

  void AddDetector(MString DetectorName);

  bool AnalyzeFile(MString FileName);
  bool Calibrate(unsigned int Detector, unsigned int Crystal);
  bool Calibrate2D(unsigned int Detector, unsigned int Crystal);  
  bool Show();

  bool Store(MString FileName = "");
  bool Load(MString FileName = "");

  unsigned int GetNDetectors();

	bool DumpToFile();

private:
  bool GatherData();
  bool CalculateMuonData(TVector3& Dir, TVector3& Pos, vector<MHit*> Hits);
  bool IsMuon(MEventData* Event);
  void PrepareCalibration();

private:
  // Housekeeping section:
  vector<MString> m_DetectorNames;
	vector<ofstream*> m_Files;
  MString m_OutputFileName;
  MString m_InputFileName;
  ifstream m_File;

  // The data acquisition section:
  MString m_SetupFileName;
  MDaq m_Daq;

  // Data section:
  vector<vector<TH2D*> > m_Data;
	vector<vector<TH1S*> > m_Residuals;

	vector<vector<vector<double> > > m_Eta;   // Eta's of crystal of detector
	vector<vector<vector<double> > > m_Z;     // dito
	vector<vector<vector<double> > > m_Sigma; // dito

	const int m_NBinsEta;
	const double m_MinEta;
	const double m_MaxEta;
	double m_BinWidthEta;
	const int m_NBinsZ;
	const double m_MinZ;
	const double m_MaxZ;
	double m_BinWidthZ;

  // Calibration histograms:
  bool m_IsCalibrationPrepared;
  TCanvas* m_EtaProfileCanvas;
  TCanvas* m_ZProfileCanvas;
  TCanvas* m_EtaZDistributionCanvas;
  TH2D* m_EtaZFitted;
  TCanvas* m_EtaZFittedCanvas;
  TCanvas* m_ResidualCanvas;


  // Muon information:
  TVector3 m_Direction;
  TVector3 m_Position;

  // The log file:
  MStreams m_Log;
};


/******************************************************************************/



/******************************************************************************
 * Gaussian fit function with 3 parameters
 */
double Gauss(double *x, double *par)
{
  double arg = 0;
  if (par[2] != 0) arg = (x[0] - par[1])/par[2];
  
  return par[0]*TMath::Exp(-0.5*arg*arg) + par[3];
}

double Gauss2(double x, double mean, double sigma, double offset)
{
  return exp(-(x-mean)*(x-mean)/sigma/sigma)+offset;
}

/******************************************************************************
 * Help information
 */
void help()
{
	cout<<endl;
	cout<<"Options::"<<endl;
	cout<<"-d <detector name>      :"
    " mandatory, multiple occurence allowed"<<endl;
	cout<<"-e <uevta event file>   :"
    " mandatory, if not -r defined, multiple occurence allowed"<<endl;
	cout<<"-o <output file prefix> :"
    " default Calib_Z_"<<endl;
	cout<<"-s <setup file name>    :"
    " mandatory, if not -r defined,"<<endl;
	cout<<"-r <data file name>     :"
    " previously stored data (CalibZ_....root)"<<endl;
	cout<<"-i                      :"
    " Analyze only one crystal"<<endl;
	cout<<"-g                      :"
    " Use (very simple) Gui"<<endl;
	cout<<"-x                      :"
    " Start with detector x (its number 0..?)"<<endl;
	cout<<"-y                      :"
    " Start with crystal y (its number 0..119)"<<endl;
	cout<<"-h                      :"
    " you are look at it ;-)"<<endl;
	cout<<endl;
}

/******************************************************************************
 * Entry point to this mini-program...
 */
int main(int argc, char *argv[])
{
	cout<<endl;
	cout<<"CalculateZ"<<endl;
	cout<<"(C) by Andreas Zoglauer"<<endl;
	cout<<endl;

  TApplication App("CalibrateZ", 0, 0);

  vector<MString> DetectorNames;
  MString OutputFileName = "";
  MString SetupFileName = "";
	MString DataFileName;
  vector<MString> FileList;
  bool UseGui = false;
  bool AnalyzeOne = false;

  int StartDetector = 0;
  int StartCrystal = 0;

  // a. Parse the arguments:
  MString argument;
  argc--;
  while (argc > 0) {
		argument = *++argv;--argc;
    if (argument == "-g") { // Use Gui
      UseGui = true;
    }
    if (argument == "-i") { // Use only one crystal
      AnalyzeOne = true;
    }
    if (argument == "-e") { // Event file name
      FileList.push_back(*++argv);
      cout<<"File to read: "<<FileList.back()<<endl;
      argc--;
    }
    if (argument == "-x") { // Event file name
      StartDetector = atoi(*++argv);
      cout<<"Start detector: "<<StartDetector<<endl;
      argc--;
    }
    if (argument == "-y") { // Event file name
      StartCrystal = atoi(*++argv);
      cout<<"Start crystal: "<<StartCrystal<<endl;
      argc--;
    }
    if (argument == "-d") { // Detector
      DetectorNames.push_back(*++argv);
      cout<<"Detector to analyse: "<<DetectorNames.back()<<endl;
      argc--;
    }
    if (argument == "-o") { // Output file nameMGUIFitting::
      OutputFileName = *++argv;
      cout<<"Name of output file: "<<OutputFileName<<endl;
      argc--;
    }
    if (argument == "-s") { // Setup file name
      SetupFileName = *++argv;
      cout<<"Name of setup file: "<<SetupFileName<<endl;
      argc--;
    }
    if (argument == "-r") { // Data file name
      DataFileName = *++argv;
      cout<<"Name of data file: "<<DataFileName<<endl;
      argc--;
    }
    if (argument == "-h") { // Setup file name
			help();
			return 0;
    }
  }

	cout<<endl;
	
  // b. Check the input:
  if (DetectorNames.size() == 0) {
    cout<<"I need one or more detector names!"<<endl;
		help();
    return 1;
  }
	if (SetupFileName == "" && DataFileName == "") {
		cout<<"I need a setup file name!"<<endl;
		help(); 
		return 1;
	}
	if (FileList.size() == 0 && DataFileName == "") {
		cout<<"I need one or more event files of type uevta!"<<endl;
		help();
		return 1;
	}
	

  MCalibrateZ C(OutputFileName, SetupFileName);

  for (unsigned int i = 0; i < DetectorNames.size(); ++i) {
    C.AddDetector(DetectorNames[i]);
  }

	if (DataFileName == "") {
		for (unsigned int i = 0; i < FileList.size(); ++i) {
			if (C.AnalyzeFile(FileList[i]) == false) {
				break;
			}
		}
		C.Store();
	} else {
		if (C.Load(DataFileName) == false) {
			return -1;
		}
	}

  MGUIFitting* Gui = 0;
  if (UseGui == true) {
    Gui = new MGUIFitting(gClient->GetRoot(), gClient->GetRoot(), &C);
    Gui->SetStart(StartDetector, StartCrystal);
  } else {
    if (AnalyzeOne == false) {
      for (unsigned int d = (unsigned int) StartDetector; d < C.GetNDetectors(); ++d) {
        for (unsigned int i = (unsigned int) StartCrystal; i < 120; ++i) {
          C.Calibrate(d, i);
        }
      }
    } else {
      C.Calibrate(StartDetector, StartCrystal);
    }
		if (OutputFileName != "") {
			C.DumpToFile();
		}
  }

  App.Run();

  delete Gui;
}


/******************************************************************************/


/******************************************************************************
 * Constructor of class MCalibrateZ
 */
MCalibrateZ::MCalibrateZ(MString OutputFileName, MString SetupFileName) :
	m_NBinsEta(128), m_MinEta(-1), m_MaxEta(1), 
	m_NBinsZ(64), m_MinZ(-4), m_MaxZ(4)
{
  m_BinWidthEta = (m_MaxEta - m_MinEta)/m_NBinsEta;
  m_BinWidthZ = (m_MaxZ - m_MinZ)/m_NBinsZ;

  MFile::ExpandFileName(OutputFileName);
  m_OutputFileName = OutputFileName;

  MFile::ExpandFileName(SetupFileName);
  m_SetupFileName = SetupFileName;

	// Initialize Canvas palette 
  Int_t oColor[11] = {10, 19, 18, 17, 16, 15, 14, 13, 12, 1, 3};
  gStyle->SetPalette(11, oColor);

  m_Log.Connect(OutputFileName + ".log");

  m_IsCalibrationPrepared = false;
}


/******************************************************************************
 * Open and analyze the file
 */
void MCalibrateZ::AddDetector(MString DetectorName)
{
  m_DetectorNames.push_back(DetectorName);

  char Text[50];
  vector<TH2D*> Data;
	vector<TH1S*> Residuals;

  for (unsigned int i = 0; i < 120; ++i) {
    sprintf(Text, "Data for (%i/%i) of %s", i%10, i/10, 
            m_DetectorNames.back().Data());
    Data.push_back(new TH2D(Text, Text, m_NBinsEta, 
                            m_MinEta, m_MaxEta, m_NBinsZ, m_MinZ, m_MaxZ));
    sprintf(Text, "Residuals for (%i/%i) of %s", i%10, i/10, 
            m_DetectorNames.back().Data());
    Residuals.push_back(new TH1S(Text, Text, 100, -8.0, +8.0));

  }
  m_Data.push_back(Data);
	m_Residuals.push_back(Residuals);

	// Result storage:
	vector<vector<double> > EtaResults;
	EtaResults.resize(120);
	m_Eta.push_back(EtaResults);

	vector<vector<double> > ZResults;
	ZResults.resize(120);
	m_Z.push_back(ZResults);
	
	vector<vector<double>  > SigmaResults;
	SigmaResults.resize(120);
	m_Sigma.push_back(SigmaResults);

	// open the belonging file:
	MString FileName = m_OutputFileName + "_" + DetectorName + ".csv";
	ofstream* File = new ofstream(FileName);
	if (File->is_open() == false) {
		cout<<"Unable to open file: "<<FileName<<endl;
		abort();
	}
	m_Files.push_back(File);
}


/******************************************************************************
 * Open and analyze the file
 */
bool MCalibrateZ::AnalyzeFile(MString FileName)
{
  MFile::ExpandFileName(FileName);

  m_InputFileName = FileName;

  m_File.open(m_InputFileName);
  if (m_File.is_open() == false) {
    mout<<"MCalibrateZ::OpenFile: Unable to open file: "
        <<m_InputFileName<<endl;
    return false;
  }

  // Reopen the setup-file:
  m_Daq.ResetDaq();
  m_Daq.Setup(m_SetupFileName);
  m_Daq.SetInputMode(MDaq::e_InputFile);

  for (unsigned int i = 0; i < m_DetectorNames.size(); ++i) {
    if (m_Daq.GetDetector(m_DetectorNames[i]) ==  0) {
      mout<<"MCalibrateZ::OpenFile:"
        " Setupfile does not contain a detector named: "
          <<m_DetectorNames[i]<<endl;
      return false;
    }    
  }
  m_Daq.ReadHighData(m_InputFileName, 6);

  GatherData();

  m_File.close();

  return true;
}


/******************************************************************************
 * Read the data from file and store it
 */
bool MCalibrateZ::GatherData()
{
  int NProcessedEvents = 0;
  int NMuons = 0;
  while (m_Daq.SingleDaq() == true) {
    if (++NProcessedEvents % 1000 == 0) {
      cout<<"Progress: Found "<<NMuons<<" out of "
          <<NProcessedEvents<<" events..."<<endl;
    }
    if (IsMuon(m_Daq.GetEvent()) == true) {
      NMuons++;
    }    
  }

  return true;
}

/******************************************************************************
 * Return the number of detectors in the calibration
 */
unsigned int MCalibrateZ::GetNDetectors()
{
  return m_DetectorNames.size();
}

/******************************************************************************
 * Check if the event is a muon:
 * The original version can be found in MRERawEvent::CheckForMips
 * and MRETrack::GetAverageDirection
 */
bool MCalibrateZ::IsMuon(MEventData* Event)
{
  //cout<<"Checking for Muons..."<<endl;

  const unsigned int MinElements = 4;
  const double MinStraightness = 0.05;

  vector<vector<MHit*>*> TrackArray;   // Array of Tracks

  vector<MHit*> D1Hits;// Array of all hits in D1
  for (int i = 0; i < Event->GetNHits(); ++i) {
    if (Event->GetHitAt(i)->GetDetector() == 1) {
      D1Hits.push_back(Event->GetHitAt(i));
    }
  }

  // Sort the D1 hits for decreasing z: 
  MHit* Temp = 0;
  int HighestPos = 0;
  double HighestValue = -1;
  for (unsigned int i = 0; i < D1Hits.size(); ++i) {
    HighestPos = i;
    HighestValue = D1Hits[i]->GetPosition().Z();
    for (unsigned int j = i; j < D1Hits.size(); ++j) {
      if (D1Hits[j]->GetPosition().Z() > HighestValue) {
        HighestValue = D1Hits[j]->GetPosition().Z();
        HighestPos = j;
      }
    }
    // Change Position
    if (HighestPos != (int) i) {
      Temp = D1Hits[i];
      D1Hits[i] = D1Hits[HighestPos];
      D1Hits[HighestPos] = Temp;
    }
  }


  // We look for straight tripple combinations:
  unsigned int max = D1Hits.size();
  MHit* iHit = 0;
  MHit* jHit = 0;
  MHit* kHit = 0;
  vector<MHit*>* Track = 0;
  TVector3 Direction1, Direction2;

  for (unsigned int i = 0; i < max; ++i) {
    if (D1Hits[i] == 0) continue;
    iHit = D1Hits[i];

    for (unsigned int j = i+1; j < max; ++j) {
      if (D1Hits[j] == 0) continue;
      jHit = D1Hits[j];
      Direction1 = jHit->GetPosition() - iHit->GetPosition();
      
      Track = 0; // No memory leak!

      for (unsigned int k = j+1; k < max; ++k) {
        if (D1Hits[k] == 0) continue;
        kHit = D1Hits[k];
        Direction2 = kHit->GetPosition() - jHit->GetPosition();
        
        // Check the straightness:
        if (Direction1.Angle(Direction2) < MinStraightness) {
          //cout<<"Found some straight track!!!!"<<endl;
          if (Track == 0) {
            //cout<<"Creating new track!!!!"<<endl;
            Track = new vector<MHit*>;
            TrackArray.push_back(Track);
            Track->push_back(iHit);
            Track->push_back(jHit);
            Track->push_back(kHit);
            D1Hits[i] = 0;
            D1Hits[j] = 0;
            D1Hits[k] = 0;
          } else {
            Track->push_back(kHit);
            D1Hits[k] = 0;
          }            
          
          Direction1 = Direction2;
        }
      } // end k

      if (Track != 0) break;

    } // end j
  } // end i

  // We need at least one track with at least MinElements hits:
  // For now: Search for the largest track:  HighestPos = -1;
  HighestValue = -1;
  HighestPos = -1;
  for (unsigned int i = 0; i < TrackArray.size(); ++i) {
    if (TrackArray[i]->size() > MinElements) {
      HighestValue = TrackArray[i]->size();
      HighestPos = i;
    }
  }

  // Now the result:

  // If we have not exactly one track, we have no muon
  if (HighestPos == -1 ||                      // We have no tracks or 
      TrackArray.size() != 1 ||                // We have not exactly one track (e.g. shower...)
      TrackArray[HighestPos]->size() < 4 ) {   // or the track is too short...
    return false;
  }

  //cout<<"==> Found a muon: Id="<<Event->GetEventNumber()<<endl;


  // Now we have a direction and a passage-point for the muon...
  TVector3 Direction;
  TVector3 Point;

  if (CalculateMuonData(Direction, Point, *TrackArray[HighestPos]) == false) {
    //cout<<"Unable to calculate muon data (direction, position)!"<<endl;
    return false;
  }

  // Now its time to determine the hit crystals for each detector:
  vector<int> Crystals;
  vector<double> Zs;
  vector<double> Angles;
  MCsIPixelDouble* CsI = 0;

  for (unsigned int d = 0; d < m_DetectorNames.size(); ++d) {

    CsI = (MCsIPixelDouble*) m_Daq.GetDetector(m_DetectorNames[d]); // <-- Hier könnte viel schief gehen...

    Crystals.resize(0);
    Zs.resize(0);
    Angles.resize(0);

    CsI->DetermineIntersectedCrystals(Crystals, Zs, Angles, Direction, Point);
    
    double Eta = 0, z = 0;
    if (Crystals.size() != 0) {
      for (unsigned int i = 0; i < Crystals.size(); ++i) {
        //cout<<"Hits in "<<m_DetectorName<<" at x="<<Crystals[i]%10
        //    <<" y="<<Crystals[i]/10<<" z="<<Zs[i]<<" angle="<<Angles[i]<<endl;
        
        if (CsI->DetermineEta(Eta, Crystals[i]%10, Crystals[i]/10) == false) {
          //cout<<"Eta not found!"<<endl;
        } else {
          //cout<<"Eta: "<<Eta<<endl;
          m_Data[d][Crystals[i]]->Fill(Eta, Zs[i], 1);
					Double_t dz;
					if (CsI->DetermineZ(z, dz, Crystals[i]%10, Crystals[i]/10)) {
						cout << abs(Zs[i] - z + 4.0)/dz;
						if (abs(Zs[i] - z + 4.0) > 2*dz) {
							cout << " ***";
						}
						cout << endl;
						m_Residuals[d][Crystals[i]]->Fill(Zs[i] - z + 4.0);
					}
        }
      }
    } else {
      //cout<<"No hits...."<<endl;
    }
  }

  return true;
}


/******************************************************************************
 * Calculate muon direction and passage-point via a linear regression:
 * y: y(z)=a0+a1*z
 * x: x(z)=b0+b1*z
 * D: n*sum(z^2)-sum(z)^2
 * a0 = (sum(z^2)*sum(y)-sum(z)*sum(y*z))/D
 * a1 = (n*sum(z*y)-sum(z)*sum(y))/D
 */
bool MCalibrateZ::CalculateMuonData(TVector3& Dir, 
                                    TVector3& Pos, 
                                    vector<MHit*> Hits)
{

  // Calculate all variables:
  double n = Hits.size();
  double D = 0;      // D
  double sumz = 0;   // sum(z)
  double sumz_2 = 0; // sum(z)^2
  double sum_z2 = 0; // sum(z^2)
  double sumyz = 0;  // sum(y*z)
  double sumy = 0;   // sum(y)
  double sumxz = 0;  // sum(x*z)
  double sumx = 0;   // sum(x)
  for (int i = 0; i < n; ++i) {
    sumx += Hits[i]->GetPosition().X();
    sumy += Hits[i]->GetPosition().Y();
    sumz += Hits[i]->GetPosition().Z();
    sumxz += Hits[i]->GetPosition().X()*Hits[i]->GetPosition().Z();
    sumyz += Hits[i]->GetPosition().Y()*Hits[i]->GetPosition().Z();
    sum_z2 += Hits[i]->GetPosition().Z()*Hits[i]->GetPosition().Z();
  }
  sumz_2 = sumz*sumz;
  D = n*sum_z2-sumz_2;
  if (D == 0) {
    return false;
  }
  double a0 = (sum_z2*sumy-sumz*sumyz)/D;
  double a1 = (n*sumyz-sumz*sumy)/D;
  double b0 = (sum_z2*sumx-sumz*sumxz)/D;
  double b1 = (n*sumxz-sumz*sumx)/D;

  Dir = TVector3(b1, a1, 1);
  Pos = TVector3(b0, a0, 0);

  return true;
}


/******************************************************************************
 * Store the histogramms to file...
 */
bool MCalibrateZ::Store(MString FileName)
{  
  MTime Time;
  Char_t Text[100];
  sprintf(Text, "CalibrateZ_%s.root", Time.GetShortString().Data()); 

  TFile* File = new TFile(Text, "RECREATE");

  cout<<"Storing histograms...";

  for (unsigned int d = 0; d < m_Data.size(); ++d) {
    for (unsigned int i = 0; i < 120; ++i) {
      sprintf(Text, "Histo of %s: %i", m_DetectorNames[d].Data(), i);
      m_Data[d][i]->Write(Text);
    }
  }
  File->Close();

  cout<<" Done."<<endl;

  return false;
}


/******************************************************************************
 * Load the histograms from file...
 */
bool MCalibrateZ::Load(MString FileName)
{
	Char_t Text[50];
	TH2D* Histogram = 0;
  TFile* File = new TFile(FileName, "READ");
	if (File == 0 || File->IsOpen() == false) {
		cout<<"Unable to open file \""<<FileName<<"\"!"<<endl;
		return false;
	} 

  for (unsigned int d = 0; d < m_DetectorNames.size(); ++d) {
    for (unsigned int i = 0; i < 120; ++i) {
      sprintf(Text, "Histo of %s: %i", m_DetectorNames[d].Data(), i);
			Histogram = (TH2D*) File->Get(Text);
			if (Histogram == 0) {
				cout<<"Can't find histogram \""<<Text<<"\" in File "<<FileName<<endl;
				File->ls();
				return false;
			}
			delete m_Data[d][i];
      m_Data[d][i] = Histogram;
    }
  }

  return true;
}


/******************************************************************************
 *
 */
void MCalibrateZ::PrepareCalibration()
{
  if (m_IsCalibrationPrepared == true) return;

  m_EtaProfileCanvas = 
		new TCanvas("m_EtaProfileCanvas",
                "m_EtaProfileCanvas", 0, 100, 500, 300);
	m_ZProfileCanvas = 
		new TCanvas("m_ZProfileCanvas", 
                "m_ZProfileCanvas", 0, 430, 500, 300);
  m_EtaZDistributionCanvas = 
		new TCanvas("m_EtaZDistributionCanvas", 
                "m_EtaZDistributionCanvas", 510, 100, 500, 300);
  m_EtaZDistributionCanvas->SetFillColor(0);
	m_EtaZFitted = new TH2D("m_EtaZFitted", "m_EtaZFitted", m_NBinsEta, 
                              m_MinEta, m_MaxEta, m_NBinsZ, m_MinZ, m_MaxZ);
	m_EtaZFittedCanvas = 
		new TCanvas("m_EtaZFittedCanvas",
                "m_EtaZFittedCanvas", 510, 430, 500, 300);
	m_ResidualCanvas = 
		new TCanvas("Residuals",
                "Residuals", 550, 450, 500, 300);

  m_IsCalibrationPrepared = true;
}


/******************************************************************************
 * Do the calibration based on the stored data
 */
bool MCalibrateZ::Calibrate(unsigned int d, unsigned int i)
{
  m_Log<<"Starting fitting..."<<endl;

  PrepareCalibration();

  bool UseMean = true;

  Char_t Text[100];
  Double_t Eta, Z, Sigma;

	TH1D* EtaProfile = 0;
	TH1D* ZProfile = 0;
	TF1* g1 = 0;

  const int MinNHitsZProfile = 1;
  const int TimesSmooth = 1;
	Double_t EtaZDistributionMax = 0;
  bool StopCondition = false;

  m_Log<<endl;
  m_Log<<endl;
  m_Log<<"Detector: "<<m_DetectorNames[d]<<"Analysing crystal: "<<i%10<<"/"<<i/10<<" (="<<i<<")"<<endl;
  m_Log<<endl;

  // Determine the average number of hits in this detector:
  Double_t AvgNHits = 0;
  for (unsigned int c = 0; c < m_Data[d].size(); ++c) {
    AvgNHits += m_Data[d][c]->Integral();
  }
  AvgNHits /= m_Data[d].size();


  // The actual data:
  TH2D* EtaZDistribution = m_Data[d][i];
	TH1S* Residual = m_Residuals[d][i];

  // Check if we have enough hits:
  if (EtaZDistribution->Integral() < 0.25*AvgNHits) {
    m_Log<<"Skipping crystal : Not enough hits: "<<EtaZDistribution->Integral()<<" (min: "<<0.25*AvgNHits<<")"<<endl;
    return false;
  }
  
	m_ResidualCanvas->cd();
	Residual->Draw();
	Residual->Fit("gaus","Q");
	TF1* func = Residual->GetFunction("gaus");
	(*m_Files[d]) << "OZ " << i%10 << " " << i/10 << " " 
								<< func->GetParameter(1) << endl;
	(*m_Files[d]) << "DZ " << i%10 << " " << i/10 << " "
								<< func->GetParameter(2) << endl;
	m_ResidualCanvas->Update();

  EtaZDistributionMax = EtaZDistribution->GetMaximum();
  EtaZDistribution->Fill(-0.99, -3.99, EtaZDistributionMax);
  m_EtaZDistributionCanvas->cd();
  EtaZDistribution->Draw("COLZ");
  m_EtaZDistributionCanvas->Update();

  // Correct for geometry dependence:
  // Each z-Slice of the 2d histogram needs to have the same amount of hits
  // -> normalize to 1!!
  for (int s = 1; s <= m_NBinsZ; ++s) {
    TH1D* Profile = EtaZDistribution->ProjectionX("Some profile", s, s);
    Double_t Sum = Profile->Integral();
    if (Sum != 0) {
      for (int j = 0; j <= m_NBinsEta+1; ++j) {
        EtaZDistribution->SetBinContent(j, s, Profile->GetBinContent(j)/Sum);
      }
    }
    delete Profile;
  }

  // Smooth the data:
//   for (int s = 0; s <= m_NBinsZ+1; ++s) {
//     TH1D* Profile = EtaZDistribution->ProjectionX("Some profile", s, s);
//     Profile->Smooth(TimesSmooth);
//     for (int j = 0; j <= m_NBinsEta+1; ++j) {
//       EtaZDistribution->SetBinContent(j, s, Profile->GetBinContent(j));
//       //cout<<EtaZDistribution->GetBinContent(j, i)<<"!"<<Profile->GetBinContent(j)<<endl;
//     }
//     delete Profile;
//   }
//   EtaZDistribution->Draw("COLZ");
//   m_EtaZDistributionCanvas->Update();
//   gSystem->ProcessEvents();

  // Create a new smoothed histogram:


  // Now do the actual fitting
			
  // a. Get a eta-profile, fit it and find the center:
  m_EtaProfileCanvas->cd();
  EtaProfile = EtaZDistribution->ProjectionX("Profile");
  g1 = new TF1("Gauss", Gauss, -0.99, 0.99, 4);
  g1->SetParNames("Height", "Mean", "Sigma", "Offset");
  g1->SetParameters(EtaProfile->GetMaximum(), 0, 0.1, 0);
  g1->SetParLimits(0, 0.5*EtaProfile->GetMaximum(), 
                   EtaProfile->GetMaximum()*2);
  g1->SetParLimits(1, -0.9, +0.9);
  g1->SetParLimits(2, 0.01, 10);
  g1->SetParLimits(3, 0, 100);
  EtaProfile->Fit("Gauss", "RLM");

  EtaProfile->Draw();
  m_EtaProfileCanvas->Update();


  m_Log<<"Fit parameters: height="<<g1->GetParameter(0)
       <<" mean="<<g1->GetParameter(1)
       <<" sigma="<<g1->GetParameter(2)
       <<" offset="<<g1->GetParameter(3)<<endl;

  // Check if mean is somewhere near the center:
  const double ValidMeanRange = 0.5;
  if (fabs(g1->GetParameter(1)) > ValidMeanRange) {
    m_Log<<"Skipping crystal : Means outside valid range |"
         <<fabs(g1->GetParameter(1))<<"| (max: "<<ValidMeanRange<<")"<<endl;
    delete g1;
    delete EtaProfile;       
    return false;
  }

  delete m_EtaZFitted;
  m_EtaZFitted = (TH2D*) EtaZDistribution->Clone();
  m_EtaZFitted->Reset();

  
  const double TimesSigma = 2.5;
  int StartBin = 
    (int) ((g1->GetParameter(1)-TimesSigma*g1->GetParameter(2) - m_MinEta)*
           m_NBinsEta/(m_MaxEta-m_MinEta));
  int StopBin = 
    (int) ((g1->GetParameter(1)+TimesSigma*g1->GetParameter(2) - m_MinEta)*
           m_NBinsEta/(m_MaxEta-m_MinEta));
  int CenterBin = 
    (int) (0.5*(StartBin+StopBin));

  // b. Do the actual fitting:
  g1->SetRange(-3.99, 3.99);


  // From Center to right:
  StopCondition = false;
  g1->SetParameters(10, 0, 1, 1);
  g1->SetParLimits(0, 0, 99999);
  g1->SetParLimits(1, -100, 100);
  g1->SetParLimits(2, 0.2, 99999);
  g1->SetParLimits(3, 0, 100);
  m_ZProfileCanvas->cd();
  for (int b = CenterBin+1; b <= StopBin; ++b) {
    Eta = m_MinEta + m_BinWidthEta*(b+0.5);
    if (ZProfile != 0) delete ZProfile;
    m_ZProfileCanvas->cd();
    sprintf(Text, "ZProfile for %f", Eta);
    ZProfile = EtaZDistribution->ProjectionY(Text, b, b);

    // Check if we have enough hits:
    if (ZProfile->Integral() < MinNHitsZProfile) {
      m_Log<<"Skipping z Profile for Eta="<<Eta<<": Not enough hits: "<<
        ZProfile->Integral()<<" (min: "<<MinNHitsZProfile<<")"<<endl;
      continue;
    }

    if (UseMean == false) {
      // Smooth:
      ZProfile->Smooth(TimesSmooth);

      // Do an iterative fitting until the change of sigma is less than 0.1
      const Int_t MaxTrials = 5;
      Int_t NTrials = 0;
      const Double_t NSigmas = 2;
      Double_t CurrentRange = 0;
      Double_t LastRange = 8;
      const Double_t MinRangeDifference = 0.02;
      g1->SetRange(-3.99, 3.99);
      //Fit loop:
      while (true) {
        NTrials++;
        
        // Fit
        ZProfile->Fit("Gauss", "ILR");
        g1->SetRange(g1->GetParameter(1)-NSigmas*g1->GetParameter(2), 
                     g1->GetParameter(1)+NSigmas*g1->GetParameter(2));
        
        // Draw
        cout<<"Drawing..."<<g1->GetParameter(0)<<"!"<<g1->GetParameter(1)<<"!"<<g1->GetParameter(2)<<endl;
        ZProfile->Draw();
        m_ZProfileCanvas->Update();
        CurrentRange = g1->GetParameter(1)+NSigmas*g1->GetParameter(2) - (g1->GetParameter(1)-NSigmas*g1->GetParameter(2));
        
        // Test all stop conditions:
        if (CurrentRange > LastRange) {
          m_Log<<"Skip: CurrentRange > LastRange: "<<CurrentRange<<" > "<<LastRange<<endl;
          break;
        }
        if (NTrials > MaxTrials) {
          m_Log<<"Skip: NTrials > MaxTrials"<<NTrials<<" > "<<MaxTrials<<endl;
          break;
        }
        if (LastRange - CurrentRange < MinRangeDifference) {
          m_Log<<"Skip: MinRangeDifference: "<<LastRange - CurrentRange<<" < "<<MinRangeDifference<<endl;
          break;
        }
        
        // Prepare for next:
        LastRange = CurrentRange;
        g1->SetRange(g1->GetParameter(1)-NSigmas*g1->GetParameter(2), 
                     g1->GetParameter(1)+NSigmas*g1->GetParameter(2));
      }
      
      //m_EtaZFittedCanvas->cd();
      m_EtaZDistributionCanvas->cd();
      //Z = ZProfile->GetMean();
      Z = g1->GetParameter(1);
      if (Z > m_MaxZ) {
        Z = m_MaxZ;
        StopCondition = true;
      }
      if (Z < m_MinZ) {
        Z = m_MinZ;
        StopCondition = true;
      }
      Sigma = g1->GetParameter(2);
    } else {
      Z = ZProfile->GetMean();
      Sigma = ZProfile->GetRMS();
    }

    m_Log<<"Eta="<<Eta<<" - Z="<<Z<<" - S="<<Sigma<<endl;
    m_EtaZFittedCanvas->cd();
    m_EtaZFitted->SetBinContent(b, (int) ((Z - m_MinZ)/m_BinWidthZ), EtaZDistributionMax);
    m_EtaZFitted->Draw("SAME");
    m_EtaZFittedCanvas->Update();
    //m_EtaZDistributionCanvas->Update();

		// Store the result:
		m_Eta[d][i].push_back(Eta);
		m_Z[d][i].push_back(Z);
		m_Sigma[d][i].push_back(Sigma);

    // Check for additional stop condition:
    if (StopCondition == true) {
      break;
    }
    //gSystem->Sleep(100);
  }

  // From Center to left:
  StopCondition = false;
  g1->SetParameters(10, 0, 1, 1);
  g1->SetParLimits(0, 0, 99999);
  g1->SetParLimits(1, -100, 100);
  g1->SetParLimits(2, 0.2, 99999);
  g1->SetParLimits(3, 0, 100);
  m_ZProfileCanvas->cd();
  for (int b = CenterBin; b > StartBin; --b) {
    Eta = m_MinEta + m_BinWidthEta*(b+0.5);

    // get the data
    if (ZProfile != 0) delete ZProfile;
    m_ZProfileCanvas->cd();
    sprintf(Text, "ZProfile for %f", Eta);
    ZProfile = EtaZDistribution->ProjectionY(Text, b, b);

    // Check if we have enough hits:
    if (ZProfile->Integral() < MinNHitsZProfile) {
      m_Log<<"Skipping z Profile for Eta="<<Eta<<": Not enough hits: "<<
        ZProfile->Integral()<<" (min: "<<MinNHitsZProfile<<")"<<endl;
      continue;
    }

    if (UseMean == false) {
      // Smooth:
      ZProfile->Smooth(TimesSmooth);
      
      // Do an iterative fitting until the change of sigma is less than 0.1
      const Int_t MaxTrials = 5;
      Int_t NTrials = 0;
      const Double_t NSigmas = 2;
      Double_t CurrentRange = 0;
      Double_t LastRange = 8;
      const Double_t MinRangeDifference = 0.02;
      g1->SetRange(-3.99, 3.99);
      //Fit loop:
      while (true) {
        NTrials++;
        
        // Fit
        ZProfile->Fit("Gauss", "RQ");
        g1->SetRange(g1->GetParameter(1)-NSigmas*g1->GetParameter(2), 
                     g1->GetParameter(1)+NSigmas*g1->GetParameter(2));
        
        // Draw
        ZProfile->Draw();
        m_ZProfileCanvas->Update();
        CurrentRange = g1->GetParameter(1)+NSigmas*g1->GetParameter(2) - (g1->GetParameter(1)-NSigmas*g1->GetParameter(2));
        
        // Test all stop conditions:
        if (CurrentRange > LastRange) {
          m_Log<<"Skip: CurrentRange > LastRange: "<<CurrentRange<<" > "<<LastRange<<endl;
          break;
        }
        if (NTrials > MaxTrials) {
          m_Log<<"Skip: NTrials > MaxTrials"<<NTrials<<" > "<<MaxTrials<<endl;
          break;
        }
        if (LastRange - CurrentRange < MinRangeDifference) {
        m_Log<<"Skip: MinRangeDifference: "<<LastRange - CurrentRange<<" < "<<MinRangeDifference<<endl;
        break;
        }
        
        // Prepare for next:
        LastRange = CurrentRange;
        g1->SetRange(g1->GetParameter(1)-NSigmas*g1->GetParameter(2), 
                     g1->GetParameter(1)+NSigmas*g1->GetParameter(2));
      } 

      // Check and display the fit results:
      //m_EtaZFittedCanvas->cd();
      m_EtaZDistributionCanvas->cd();
      
      //Z = ZProfile->GetMean();
      Z = g1->GetParameter(1);
      if (Z > m_MaxZ) {
        Z = m_MaxZ;
        StopCondition = true;
      }
      if (Z < m_MinZ) {
        Z = m_MinZ;
        StopCondition = true;
      }
      Sigma = g1->GetParameter(2);

    } else {
      Z = ZProfile->GetMean();
      Sigma = ZProfile->GetRMS();
    }

    m_Log<<"Eta="<<Eta<<" -  Z="<<Z<<" - S="<<g1->GetParameter(2)<<endl;
    m_EtaZFittedCanvas->cd();
    m_EtaZFitted->SetBinContent(b, (int) ((Z - m_MinZ)/m_BinWidthZ), EtaZDistributionMax);
    m_EtaZFitted->Draw("SAME");
    //m_EtaZDistributionCanvas->Update();
    m_EtaZFittedCanvas->Update();

		// Store the result:
		cout<<m_Eta[d][i].size()<<endl;
		m_Eta[d][i].push_back(Eta);
		m_Z[d][i].push_back(Z);
		m_Sigma[d][i].push_back(Sigma);

    // Check for additional stop condition:
    if (StopCondition == true) {
      break;
    }
    //gSystem->Sleep(500);
  }


  // Some final clean ups...
  delete EtaProfile;
  delete g1;

  m_Log<<endl;
  m_Log<<"Finishing Detector: "<<m_DetectorNames[d]<<"Analysing crystal: "<<i%10<<"/"<<i/10<<" (="<<i<<")"<<endl;
  m_Log<<endl;

	// Prepare for output: Sort the data:
	int SmallestEtaIndex;
	double SmallestEta;
	double EtaTemp, ZTemp, SigmaTemp;
	for (unsigned int v = 0; v < m_Eta[d][i].size(); ++v) {
		SmallestEtaIndex = 0;
		SmallestEta = 99999;
		for (unsigned int s = v; s < m_Eta[d][i].size(); ++s) {
			if (m_Eta[d][i][s] < SmallestEta) {
				SmallestEta = m_Eta[d][i][s];
				SmallestEtaIndex = s;
			}
		}
		// Now swap:
		EtaTemp = m_Eta[d][i][v];
		m_Eta[d][i][v] = m_Eta[d][i][SmallestEtaIndex];
		m_Eta[d][i][SmallestEtaIndex] = EtaTemp;
		ZTemp = m_Z[d][i][v];
		m_Z[d][i][v] = m_Z[d][i][SmallestEtaIndex];
		m_Z[d][i][SmallestEtaIndex] = ZTemp;
		SigmaTemp = m_Sigma[d][i][v];
		m_Sigma[d][i][v] = m_Sigma[d][i][SmallestEtaIndex];
		m_Sigma[d][i][SmallestEtaIndex] = SigmaTemp;
	}
	
	// Dump:
	(*m_Files[d])<<"CZ "<<i%10<<" "<<i/10<<" "<<m_Eta[d][i].size()<<"   ";
	
	for (unsigned int v = 0; v < m_Eta[d][i].size(); ++v) {
		(*m_Files[d])<<m_Eta[d][i][v]<<" "<<m_Z[d][i][v]<<" "<<m_Sigma[d][i][v]<<"   ";
	}
	(*m_Files[d])<<endl;
  
	return true;
}


/******************************************************************************
 * Do the calibration based on the stored data
 */
bool MCalibrateZ::Calibrate2D(unsigned int d, unsigned int i)
{
  m_Log<<"Starting fitting..."<<endl;

  PrepareCalibration();

  //Char_t Text[100];
  //Double_t Eta, Z;

	TH1D* EtaProfile = 0;
	//TH1D* ZProfile = 0;
	TF1* g1 = 0;

  //const int MinNHitsZProfile = 1;
  const int TimesSmooth = 5;
	Double_t EtaZDistributionMax = 0;
  //bool StopCondition = false;

  m_Log<<endl;
  m_Log<<endl;
  m_Log<<"Detector: "<<m_DetectorNames[d]<<"Analysing crystal: "<<i%10<<"/"<<i/10<<" (="<<i<<")"<<endl;
  m_Log<<endl;

  // Determine the average number of hits in this detector:
  Double_t AvgNHits = 0;
  for (unsigned int c = 0; c < m_Data[d].size(); ++c) {
    AvgNHits += m_Data[d][c]->Integral();
  }
  AvgNHits /= m_Data[d].size();


  // The actual data:
  TH2D* EtaZDistribution = m_Data[d][i];

  // Check if we have enough hits:
  if (EtaZDistribution->Integral() < 0.25*AvgNHits) {
    m_Log<<"Skipping crystal : Not enough hits: "<<EtaZDistribution->Integral()<<" (min: "<<0.25*AvgNHits<<")"<<endl;
    return false;
  }
  
  EtaZDistributionMax = EtaZDistribution->GetMaximum();
  EtaZDistribution->Fill(-0.99, -3.99, EtaZDistributionMax);
  m_EtaZDistributionCanvas->cd();
  EtaZDistribution->Draw("COLZ");
  m_EtaZDistributionCanvas->Update();

  // Correct for geometry dependence:
  // Each z-Slice of the 2d histogram needs to have the same amount of hits
  // -> normalize to 1!!
  for (int s = 1; s <= m_NBinsZ; ++s) {
    TH1D* Profile = EtaZDistribution->ProjectionX("Some profile", s, s);
    Double_t Sum = Profile->Integral();
    for (int j = 0; j <= m_NBinsEta+1; ++j) {
      EtaZDistribution->SetBinContent(j, s, Profile->GetBinContent(j)/Sum);
    }
//     EtaZDistribution->Draw("COLZ");
//     m_EtaZDistributionCanvas->Update();
    delete Profile;
  }

  // Smooth the data:
  for (int s = 0; s <= m_NBinsZ+1; ++s) {
    TH1D* Profile = EtaZDistribution->ProjectionX("Some profile", s, s);
    Profile->Smooth(TimesSmooth);
    for (int j = 0; j <= m_NBinsEta+1; ++j) {
      EtaZDistribution->SetBinContent(j, s, Profile->GetBinContent(j));
      //cout<<EtaZDistribution->GetBinContent(j, i)<<"!"<<Profile->GetBinContent(j)<<endl;
    }
//     EtaZDistribution->Draw("COLZ");
//     m_EtaZDistributionCanvas->Update();
    delete Profile;
  }

  EtaZDistribution->Draw("COLZ");
  m_EtaZDistributionCanvas->Update();
  gSystem->ProcessEvents();

  // Create a new smoothed histogram:


  // Now do the actual fitting
  TH1D* EtaFits = new TH1D("EtaFits", "EtaFits", m_NBinsZ, m_MinZ, m_MaxZ);
			
  // a. Get a eta-profile, fit it and find the center:
  for (int f = 1; f <= m_NBinsZ; ++f) {
    Double_t ValueZ = (f-0.5)*m_BinWidthZ + m_MinZ;
    cout<<"ValueZ:"<<ValueZ<<endl;
    m_EtaProfileCanvas->cd();
    EtaProfile = EtaZDistribution->ProjectionX("Profile", f, f);
    g1 = new TF1("Gauss", Gauss, -0.99, 0.99, 4);
    g1->SetParNames("Height", "Mean", "Sigma", "Offset");
    g1->SetParameters(EtaProfile->GetMaximum(), 0, 0.1, 0);
//     g1->SetParLimits(0, 0.5*EtaProfile->GetMaximum(), 
//                      EtaProfile->GetMaximum()*2);
//     g1->SetParLimits(1, -0.9, +0.9);
//     g1->SetParLimits(2, 0.01, 10);
//     g1->SetParLimits(3, 0, 100);
    EtaProfile->Fit("Gauss", "R");
    EtaProfile->Draw();
    m_EtaProfileCanvas->Update();
    
    EtaFits->Fill(ValueZ, g1->GetParameter(1));
    m_EtaZFittedCanvas->cd();
    EtaFits->Draw();
    m_EtaZFittedCanvas->Update();

    delete g1;
    delete EtaProfile;
  }    

  m_Log<<endl;
  m_Log<<"Finishing Detector: "<<m_DetectorNames[d]<<"Analysing crystal: "<<i%10<<"/"<<i/10<<" (="<<i<<")"<<endl;
  m_Log<<endl;

  return true;
}


/******************************************************************************
 * Show one histogram
bool MCalibrateZ::Calibrate2D(unsigned int d, unsigned int i)
{
  m_Log<<"Starting fitting..."<<endl;
 
  double ValueE;
  double ValueZ;

  int ENBins = 200;
  double EMin = -1;
  double EMax = 1;
  double EBinWidth = (EMax-EMin)/ENBins;

  int ZNBins = 200;
  double ZMin = -4;
  double ZMax = 4;
  double ZBinWidth = (ZMax-ZMin)/ZNBins;

	// Initialize Canvas palette 
  Int_t oColor[9] = {10, 19, 18, 17, 16, 15, 14, 13, 12};
  gStyle->SetPalette(9, oColor);

  TCanvas* ECanvas = new TCanvas("ECanvas", "ECanvas", 0, 0, 500, 300);
  TH2D* E = new TH2D("Func", "Func", ENBins, EMin, EMax, ZNBins, ZMin, ZMax);
  for (int x = 0; x < ENBins; ++x) {
    for (int z = 0; z < ZNBins; ++z) {
      ValueE = (x+0.5)*EBinWidth + EMin;
      ValueZ = (z+0.5)*ZBinWidth + ZMin;
      E->Fill(ValueE, ValueZ, Gauss2(ValueE-0.001*ValueZ*ValueZ*ValueZ-0.01*ValueZ, 0, 0.05, 0));
    }
  }

  E->SetStats(false);
  ECanvas->cd();
  E->Draw("COLZ");
  ECanvas->Update();


  // 
  TH1D* EtaFits = new TH1D("EtaFits", "EtaFits", ZNBins, ZMin, ZMax);
  TCanvas* EtaFitsCanvas = new TCanvas("EtaFitsCanvas", "EtaFitsCanvas", 0, 330, 500, 300);
  TCanvas* ProjectionCanvas = new TCanvas("ProjectionCanvas", "ProjectionCanvas", 510, 330, 500, 300);
  for (int f = 1; f <= ZNBins; ++f) {
    ValueZ = (f-0.5)*ZBinWidth + ZMin;
    cout<<"z="<<ValueZ<<endl;
    TH1D* Projection = E->ProjectionX("Proj", f, f);
    TF1* g1 = new TF1("Gauss", Gauss, -0.8, 0.8, 4);
    g1->SetParNames("Height", "Mean", "Sigma", "Offset");
    g1->SetParameters(1, 0, 0.1, 0);

    Projection->Fit("Gauss", "R");

    ProjectionCanvas->cd();
    Projection->Draw();
    ProjectionCanvas->Update();

//     EtaFits->Fill(ValueZ, g1->GetParameter(1));
//     EtaFitsCanvas->cd();
//     EtaFits->Draw();
//     EtaFitsCanvas->Update();

    delete g1;
    delete Projection;
  }

  return true;
}
*/

/******************************************************************************
 * Show one histogram
 */
bool MCalibrateZ::Show()
{
 // Determine histogramm with most entries, and plot it...
  double Max = 0;
  unsigned int MaxIndex_i = 0;
  unsigned int MaxIndex_d = 0;
  for (unsigned int d = 0; d < m_Data.size(); ++d) {
    for (unsigned int i = 0; i < m_Data[d].size(); ++i) {
      if (m_Data[d][i]->GetSum() >= Max) {
        Max = m_Data[d][i]->GetSum();
        MaxIndex_i = i;
        MaxIndex_d = d;
      }
    } 
  }
  
  cout<<"Showing histogram: "<<MaxIndex_i%10<<"/"<<MaxIndex_i/10
      <<" of detector: "<<m_DetectorNames[MaxIndex_d]<<endl;

  new TCanvas();
  m_Data[MaxIndex_d][MaxIndex_i]->Draw();

  return true;
}


/******************************************************************************
 * 
 */
bool MCalibrateZ::DumpToFile()
{
// 	int SmallestEtaIndex;
// 	double SmallestEta;
// 	double EtaTemp, ZTemp, SigmaTemp;
	
	MString FileName;

	/*
	// Sort the data according to increasing eta:
	for (unsigned int d = 0; d < m_DetectorNames.size(); ++d) {
		for (unsigned int c = 0; c < 120; ++c) {
			for (unsigned int v = 0; v < m_Eta[d][c].size(); ++v) {
				SmallestEtaIndex = 0;
				SmallestEta = 99999;
				for (unsigned int s = v; s < m_Eta[d][c].size(); ++s) {
					if (m_Eta[d][c][s] < SmallestEta) {
						SmallestEta = m_Eta[d][c][s];
						SmallestEtaIndex = s;
					}
				}
				// Now swap:
				EtaTemp = m_Eta[d][c][v];
				m_Eta[d][c][v] = m_Eta[d][c][SmallestEtaIndex];
				m_Eta[d][c][SmallestEtaIndex] = EtaTemp;
				ZTemp = m_Z[d][c][v];
				m_Z[d][c][v] = m_Z[d][c][SmallestEtaIndex];
				m_Z[d][c][SmallestEtaIndex] = ZTemp;
				SigmaTemp = m_Sigma[d][c][v];
				m_Sigma[d][c][v] = m_Sigma[d][c][SmallestEtaIndex];
				m_Sigma[d][c][SmallestEtaIndex] = SigmaTemp;
			}
		}
	}

	for (unsigned int d = 0; d < m_DetectorNames.size(); ++d) {
		FileName = m_OutputFileName + "_" + m_DetectorNames[d] + ".csv";
		ofstream File(FileName);
		if (File.is_open() == false) {
			cout<<"Unable to open file: "<<FileName<<endl;
			continue;
		}
		for (unsigned int c = 0; c < 120; ++c) {
			File<<"CZ "<<c%10<<" "<<c/10<<" "<<m_Eta[d][c].size()<<"   ";

			for (unsigned int v = 0; v < m_Eta[d][c].size(); ++v) {
				File<<m_Eta[d][c][v]<<" "<<m_Z[d][c][v]<<" "<<m_Sigma[d][c][v]<<"   ";
			}
			File<<endl;
		}
	}
	*/

	// Calculate average sigma per detector:
	for (unsigned int d = 0; d < m_DetectorNames.size(); ++d) {
		double Sigma = 0;
		double Indices = 0;
		for (unsigned int c = 0; c < 120; ++c) {
			for (unsigned int v = 0; v < m_Eta[d][c].size(); ++v) {
				if (m_Sigma[d][c][v] > 4) {
					cout<<"Large sigma: "<<m_Sigma[d][c][v]<<endl;
				} else {
					Sigma +=  m_Sigma[d][c][v];
					Indices++;
				}
			}
		}
		cout<<"Average z-sigma for detector "<<m_DetectorNames[d]<<": "<<Sigma/Indices<<endl;
	}	

	return true;
}


/******************************************************************************
 *
 */
MGUIFitting::MGUIFitting(const TGWindow *Parent, const TGWindow *Main, MCalibrateZ* Calibration)
  : TGMainFrame(Parent, 400, 50)
{
  // Construct an instance of MGUIFitting and bring it to the screen

  m_Calibration = Calibration;
  m_Crystal = 0;
  m_Detector = 0;

  Create();
}


/******************************************************************************
 *
 */
MGUIFitting::~MGUIFitting()
{
  // Delete an instance of MGUIFitting
}


/******************************************************************************
 *
 */
void MGUIFitting::Create()
{
  // Create the main window

  // We start with a name and an icon...

  m_MainFrameLayout = new TGLayoutHints(kLHintsExpandX| kLHintsExpandY, 5, 5, 5, 5);
  m_MainFrame = new TGCompositeFrame(this, 80, 60, kHorizontalFrame);
  AddFrame(m_MainFrame, m_MainFrameLayout);

  m_ButtonLayout = new TGLayoutHints(kLHintsExpandX | kLHintsExpandY, 2, 2, 0, 0);

  m_NextButton = new TGTextButton(m_MainFrame, "Next", e_Next);
  m_NextButton->Associate(this);
  m_MainFrame->AddFrame(m_NextButton, m_ButtonLayout);

  m_LoopButton = new TGTextButton(m_MainFrame, "Loop", e_Loop);
  m_LoopButton->Associate(this);
  m_MainFrame->AddFrame(m_LoopButton, m_ButtonLayout);

  m_StopButton = new TGTextButton(m_MainFrame, "Stop", e_Stop);
  m_StopButton->Associate(this);
  m_MainFrame->AddFrame(m_StopButton, m_ButtonLayout);

  m_CloseButton = new TGTextButton(m_MainFrame, "Close", e_Close);
  m_CloseButton->Associate(this);
  m_MainFrame->AddFrame(m_CloseButton, m_ButtonLayout);

  // and bring it to the screen.
  MapSubwindows();
  MapWindow();  

  Layout();
 
  return;
}


/******************************************************************************
 *
 */
Bool_t MGUIFitting::ProcessMessage(Long_t Message, Long_t Parameter1, 
                                 Long_t Parameter2)
{
  // Process the messages for this window

	Bool_t Status = kTRUE;
	
  switch (GET_MSG(Message)) {
  case kC_COMMAND:
    switch (GET_SUBMSG(Message)) {
    case kCM_BUTTON:
      switch (Parameter1) {

      case e_Next:
				Status = OnNext();
        break;

      case e_Loop:
				Status = OnLoop();
        break;

      case e_Stop:
				Status = OnStop();
        break;
        
      case e_Close:
        Status = OnClose();
        break;

       default:
        break;
      }
      break;
    default:
      break;
    }
    break;
  default:
    break;
  }
  
  return Status;
}


/******************************************************************************
 *
 */
void MGUIFitting::CloseWindow()
{
	// Close and delete this window

  gApplication->Terminate(0);
}


/******************************************************************************
 *
 */
Bool_t MGUIFitting::OnNext()
{

  m_Crystal++;
  if (m_Crystal >= 120) {
    m_Crystal = 0;
    m_Detector++;
  }

  if (m_Detector >= m_Calibration->GetNDetectors()) {
    mgui<<"No more crystals and detectors available! (max. " 
				<< m_Calibration->GetNDetectors() << ")" 
				<< " this: " << m_Detector << info;
    return false;
  }

  m_Calibration->Calibrate(m_Detector, m_Crystal);

	return true;
}


/******************************************************************************
 *
 */
Bool_t MGUIFitting::OnLoop()
{
  m_StopLoop = false;

  for (; m_Detector < m_Calibration->GetNDetectors(); ++m_Detector) {
    for (;m_Crystal < 120; ++m_Crystal) {
      if (m_StopLoop == true) {
        return true;
      } else {
        m_Calibration->Calibrate(m_Detector, m_Crystal);
        gSystem->ProcessEvents();
      }
    }
		m_Crystal = 0;
  }

	return true;
}


/******************************************************************************
 *
 */
Bool_t MGUIFitting::OnStop()
{
  m_StopLoop = true;

	return true;
}



/******************************************************************************
 *
 */
Bool_t MGUIFitting::OnClose()
{
	// The Apply button has been pressed

	CloseWindow();

	return kTRUE;
}


/******************************************************************************
 *
 */
void MGUIFitting::SetStart(unsigned int Detector, unsigned int Crystal)
{
  m_Detector = Detector;
  m_Crystal = Crystal;
}


/*
 * ZDFitting end...
 ******************************************************************************/
