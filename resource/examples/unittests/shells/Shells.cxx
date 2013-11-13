/* 
 * Shells.cxx
 *
 *
 * Copyright (C) by Andreas Zoglauer.
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

// Standard
#include <iostream>
#include <string>
#include <sstream>
#include <csignal>
#include <cstdlib>
using namespace std;

// ROOT
#include <TROOT.h>
#include <TEnv.h>
#include <TSystem.h>
#include <TApplication.h>
#include <TStyle.h>
#include <TCanvas.h>
#include <TH1.h>
#include <TH2.h>

// MEGAlib
#include "MGlobal.h"
#include "MStreams.h"
#include "MDGeometryQuest.h"
#include "MDDetector.h"
#include "MFileEventsSim.h"
#include "MDVolumeSequence.h"
#include "MSimEvent.h"
#include "MSimHT.h"


/******************************************************************************/

class Shells
{
public:
  /// Default constructor
  Shells();
  /// Default destructor
  ~Shells();
  
  /// Parse the command line
  bool ParseCommandLine(int argc, char** argv);
  /// Analyze what eveer needs to be analyzed...
  bool Analyze();
  /// Interrupt the analysis
  void Interrupt() { m_Interrupt = true; }

private:
  /// True, if the analysis needs to be interrupted
  bool m_Interrupt;
  
  //! The simulation file name
  MString m_FileName;
  //! The geometry file name
  MString m_GeometryFileName;
  //! The output postfix
  MString m_PostFix;
};

/******************************************************************************/


/******************************************************************************
 * Default constructor
 */
Shells::Shells() : m_Interrupt(false)
{
  gStyle->SetPalette(1, 0);
}


/******************************************************************************
 * Default destructor
 */
Shells::~Shells()
{
  // Intentionally left blank
}


/******************************************************************************
 * Parse the command line
 */
bool Shells::ParseCommandLine(int argc, char** argv)
{
  ostringstream Usage;
  Usage<<endl;
  Usage<<"  Usage: Shells <options>"<<endl;
  Usage<<"    General options:"<<endl;
  Usage<<"         -f:   simulation file name"<<endl;
  Usage<<"         -g:   geometry file name"<<endl;
  Usage<<"         -o:   output postfix"<<endl;
  Usage<<"         -h:   print this help"<<endl;
  Usage<<endl;

  string Option;

  // Check for help
  for (int i = 1; i < argc; i++) {
    Option = argv[i];
    if (Option == "-h" || Option == "--help" || Option == "?" || Option == "-?")
{
      cout<<Usage.str()<<endl;
      return false;
    }
  }

  // Now parse the command line options:
  for (int i = 1; i < argc; i++) {
    Option = argv[i];

    // First check if each option has sufficient arguments:
    // Single argument
    if (Option == "-f") {
      if (!((argc > i+1) && 
            (argv[i+1][0] != '-' || isalpha(argv[i+1][1]) == 0))){
        cout<<"Error: Option "<<argv[i][1]<<" needs a second argument!"<<endl;
        cout<<Usage.str()<<endl;
        return false;
      }
    } 
    // Multiple arguments template
    /*
    else if (Option == "-??") {
      if (!((argc > i+2) && 
            (argv[i+1][0] != '-' || isalpha(argv[i+1][1]) == 0) && 
            (argv[i+2][0] != '-' || isalpha(argv[i+2][1]) == 0))){
        cout<<"Error: Option "<<argv[i][1]<<" needs two arguments!"<<endl;
        cout<<Usage.str()<<endl;
        return false;
      }
    }
    */

    // Then fulfill the options:
    if (Option == "-f") {
      m_FileName = argv[++i];
      cout<<"Accepting file name: "<<m_FileName<<endl;
    } else if (Option == "-g") {
      m_GeometryFileName = argv[++i];
      cout<<"Accepting geometry file name: "<<m_GeometryFileName<<endl;
    } else if (Option == "-o") {
      m_PostFix = argv[++i];
      cout<<"Accepting postfix: "<<m_PostFix<<endl;
    } else {
      cout<<"Error: Unknown option \""<<Option<<"\"!"<<endl;
      cout<<Usage.str()<<endl;
      return false;
    }
  }

  return true;
}


/******************************************************************************
 * Do whatever analysis is necessary
 */
bool Shells::Analyze()
{
  if (m_Interrupt == true) return false;

  // Load geometry:
  MDGeometryQuest* Geometry = new MDGeometryQuest();

  if (Geometry->ScanSetupFile(m_GeometryFileName) == true) {
    cout<<"Geometry "<<Geometry->GetName()<<" loaded!"<<endl;
    Geometry->ActivateNoising(false);
    Geometry->SetGlobalFailureRate(0.0);
  } else {
    cout<<"Loading of geometry "<<Geometry->GetName()<<" failed!!"<<endl;
    return false;
  }  

  MFileEventsSim* SiReader = new MFileEventsSim(Geometry);
  if (SiReader->Open(m_FileName) == false) {
    cout<<"Unable to open sim file!"<<endl; 
    return false;
  }
  cout<<"Opened file "<<SiReader->GetFileName()<<" created with MEGAlib version:"<<SiReader->GetMEGAlibVersion()<<endl;
  SiReader->ShowProgress();

  // Bin boarders
  vector<double> Boarders;
  for (double d = 0.0; d < 1.0; d += 0.05) Boarders.push_back(d);
  for (double d = 1.0; d < 3.0; d += 0.1) Boarders.push_back(d);
  for (double d = 3.0; d < 7.0; d += 0.2) Boarders.push_back(d);
  for (double d = 7.0; d < 15.0; d += 0.4) Boarders.push_back(d);
  for (double d = 15.0; d < 31.0; d += 0.8) Boarders.push_back(d);
  for (double d = 31.0; d < 63.0; d += 1.6) Boarders.push_back(d);
  for (double d = 63.0; d < 127.0; d += 3.2) Boarders.push_back(d);
  Boarders[0] = 0.01; // So that logarithm works --- it is anyway binned, so no issues expected
  Boarders.push_back(127.0);
  
  // log energy bins
  vector<double> Energies;
  double eMin = log(1);
  double eMax = log(100000);
  unsigned int eBins = 100;
  double eDiff = (eMax-eMin)/eBins;
  for (unsigned int b = 0; b <= eBins; ++b) {
    Energies.push_back(exp(eMin + b*eDiff));
  }
  
  TH1D* Deposits = new TH1D("Deposits", "Energy deposits", Boarders.size()-1, &Boarders[0]);
  Deposits->SetXTitle("Distance [cm]");
  Deposits->SetYTitle("Energy deposit [keV/cm/ph]");
  TH1D* DepositsEntries = new TH1D("DepositsEntries", "Energy deposit entries", Boarders.size()-1, &Boarders[0]);

  
  TH1D* FirstComptonIA = new TH1D("FirstComptonIA", "Location first Compton interaction", Boarders.size()-1, &Boarders[0]);
  FirstComptonIA->SetXTitle("Distance [cm]");
  FirstComptonIA->SetYTitle("cts/cm/ph");

  TH1D* FirstPairIA = new TH1D("FirstPairIA", "Location first pair interaction", Boarders.size()-1, &Boarders[0]);
  FirstPairIA->SetXTitle("Distance [cm]");
  FirstPairIA->SetYTitle("cts/cm/ph");

  TH1D* FirstPhotoIA = new TH1D("FirstPhotoIA", "Location first photo effect interaction", Boarders.size()-1, &Boarders[0]);
  FirstPhotoIA->SetXTitle("Distance [cm]");
  FirstPhotoIA->SetYTitle("cts/cm/ph");

  TH1D* FirstComptonDeposit = new TH1D("FirstComptonDeposit", "First Compton energy deposit", Energies.size()-1, &Energies[0]);
  FirstComptonDeposit->SetXTitle("Energy [keV]");
  FirstComptonDeposit->SetYTitle("cts/keV/ph");
  
  
  TH1D* NumberComptonIA = new TH1D("NumberComptonIA", "Number of Compton interactions", 30, 0.5, 30.5);
  NumberComptonIA->SetXTitle("Number of Compton IAs");
  NumberComptonIA->SetYTitle("IAs/ph");
  
  TH1D* NumberPhotoIA = new TH1D("NumberPhotoIA", "Number of photo effect interactions (devide by the number of secondaries)", 30, 0.5, 30.5);
  NumberPhotoIA->SetXTitle("Number of photo effect IAs");
  NumberPhotoIA->SetYTitle("IAs/ph");
  
  TH1D* NumberPairIA = new TH1D("NumberPairIA", "Number pair interactions (devide by two)", 30, 0.5, 30.5);
  NumberPairIA->SetXTitle("Number of pair IAs");
  NumberPairIA->SetYTitle("IAs/ph");
  
  TH1D* NumberRayleighIA = new TH1D("NumberRayleighIA", "Number of Rayleigh interactions", 30, 0.5, 30.5);
  NumberRayleighIA->SetXTitle("Number of Rayleigh IAs");
  NumberRayleighIA->SetYTitle("IAs/ph");
  
  
  MSimEvent* SiEvent = 0;
  while ((SiEvent = SiReader->GetNextEvent(false)) != 0) {
    for (unsigned int h = 0; h < SiEvent->GetNHTs(); ++h) {
      Deposits->Fill(SiEvent->GetHTAt(h)->GetPosition().Mag(), SiEvent->GetHTAt(h)->GetEnergy());
      DepositsEntries->Fill(SiEvent->GetHTAt(h)->GetPosition().Mag(), 1);
    }
    
    if (SiEvent->GetNIAs() >= 2) {
      if (SiEvent->GetIAAt(1)->GetProcess() == "COMP") {
        FirstComptonIA->Fill(SiEvent->GetIAAt(1)->GetPosition().Mag()); 
        FirstComptonDeposit->Fill(SiEvent->GetIAAt(1)->GetSecondaryEnergy()); 
      } else if (SiEvent->GetIAAt(1)->GetProcess() == "PHOT") {
        FirstPhotoIA->Fill(SiEvent->GetIAAt(1)->GetPosition().Mag()); 
      } else if (SiEvent->GetIAAt(1)->GetProcess() == "PAIR") {
        FirstPairIA->Fill(SiEvent->GetIAAt(1)->GetPosition().Mag()); 
      }
    }
    
    int NCompton = 0;
    int NPhoto = 0;
    int NPair = 0;
    int NRayleigh = 0;
    for (unsigned int i = 1; i < SiEvent->GetNIAs(); ++i) {
      
      if (SiEvent->GetIAAt(i)->GetProcess() == "COMP") {
        ++NCompton;
      } else if (SiEvent->GetIAAt(i)->GetProcess() == "PHOT") {
        ++NPhoto;
      } else if (SiEvent->GetIAAt(i)->GetProcess() == "PAIR") {
        ++NPair;
      } else if (SiEvent->GetIAAt(i)->GetProcess() == "RAYL") {
        ++NRayleigh;
      }
    }
    if (NCompton > 0) NumberComptonIA->Fill(NCompton);
    if (NPhoto > 0) NumberPhotoIA->Fill(NPhoto);
    if (NPair > 0) NumberPairIA->Fill(NPair);
    if (NRayleigh > 0) NumberRayleighIA->Fill(NRayleigh);        
    
    delete SiEvent;
  }
  
  long SimulatedEvents = SiReader->GetSimulatedEvents();
  cout<<"Simulated events: "<<SimulatedEvents<<endl;
  SiReader->Close();
  delete SiReader;

  // Normalize
  for (int bx = 1; bx <= Deposits->GetNbinsX(); ++bx) {
    if (DepositsEntries->GetBinContent(bx) > 0) {
      Deposits->SetBinError(bx, Deposits->GetBinContent(bx)*sqrt(DepositsEntries->GetBinContent(bx))/DepositsEntries->GetBinContent(bx)/Deposits->GetBinWidth(bx)/SimulatedEvents);
    }
    Deposits->SetBinContent(bx, Deposits->GetBinContent(bx)/Deposits->GetBinWidth(bx)/SimulatedEvents);
  }
  
  TCanvas* DepositsCanvas = new TCanvas();
  DepositsCanvas->cd();
  DepositsCanvas->SetLogx();
  DepositsCanvas->SetLogy();  
  Deposits->Draw();
  DepositsCanvas->Update();
  DepositsCanvas->SaveAs(MString("EnergyDepositRange.") + m_PostFix + ".root");
  
    
  for (int bx = 1; bx <= FirstComptonIA->GetNbinsX(); ++bx) {
    FirstComptonIA->SetBinError(bx, sqrt(FirstComptonIA->GetBinContent(bx))/FirstComptonIA->GetBinWidth(bx)/SimulatedEvents);
    FirstComptonIA->SetBinContent(bx, FirstComptonIA->GetBinContent(bx)/FirstComptonIA->GetBinWidth(bx)/SimulatedEvents);
    FirstPhotoIA->SetBinError(bx, sqrt(FirstPhotoIA->GetBinContent(bx))/FirstPhotoIA->GetBinWidth(bx)/SimulatedEvents);
    FirstPhotoIA->SetBinContent(bx, FirstPhotoIA->GetBinContent(bx)/FirstPhotoIA->GetBinWidth(bx)/SimulatedEvents);
    FirstPairIA->SetBinError(bx, sqrt(FirstPairIA->GetBinContent(bx))/FirstPairIA->GetBinWidth(bx)/SimulatedEvents);
    FirstPairIA->SetBinContent(bx, FirstPairIA->GetBinContent(bx)/FirstPairIA->GetBinWidth(bx)/SimulatedEvents);
  }
  
  TCanvas* ComptonIACanvas = new TCanvas();
  ComptonIACanvas->cd();
  ComptonIACanvas->SetLogx();
  ComptonIACanvas->SetLogy();  
  FirstComptonIA->Draw();
  ComptonIACanvas->Update();
  ComptonIACanvas->SaveAs(MString("LocationFirstComptonIA.") + m_PostFix + ".root");
  
  TCanvas* PairIACanvas = new TCanvas();
  PairIACanvas->cd();
  PairIACanvas->SetLogx();
  PairIACanvas->SetLogy();  
  FirstPairIA->Draw();
  PairIACanvas->Update();
  PairIACanvas->SaveAs(MString("LocationFirstPairIA.") + m_PostFix + ".root");
  
  TCanvas* PhotoIACanvas = new TCanvas();
  PhotoIACanvas->cd();
  PhotoIACanvas->SetLogx();
  PhotoIACanvas->SetLogy();  
  FirstPhotoIA->Draw();
  PhotoIACanvas->Update();
  PhotoIACanvas->SaveAs(MString("LocationFirstPhotoIA.") + m_PostFix + ".root");

  
  for (int bx = 1; bx <= FirstComptonDeposit->GetNbinsX(); ++bx) {
    FirstComptonDeposit->SetBinError(bx, sqrt(FirstComptonDeposit->GetBinContent(bx))/FirstComptonDeposit->GetBinWidth(bx)/SimulatedEvents);
    FirstComptonDeposit->SetBinContent(bx, FirstComptonDeposit->GetBinContent(bx)/FirstComptonDeposit->GetBinWidth(bx)/SimulatedEvents);
  }
  
  TCanvas* FirstComptonDepositCanvas = new TCanvas();
  FirstComptonDepositCanvas->cd();
  FirstComptonDepositCanvas->SetLogx();
  FirstComptonDepositCanvas->SetLogy();  
  FirstComptonDeposit->Draw();
  FirstComptonDepositCanvas->Update();
  FirstComptonDepositCanvas->SaveAs(MString("FirstComptonDeposit.") + m_PostFix + ".root");
  
  
      
  for (int bx = 1; bx <= NumberComptonIA->GetNbinsX(); ++bx) {
    NumberComptonIA->SetBinError(bx, sqrt(NumberComptonIA->GetBinContent(bx))/SimulatedEvents);
    NumberComptonIA->SetBinContent(bx, NumberComptonIA->GetBinContent(bx)/SimulatedEvents);
  }
  TCanvas* NumberComptonIACanvas = new TCanvas();
  NumberComptonIACanvas->cd();
  NumberComptonIA->Draw();
  NumberComptonIACanvas->Update();
  NumberComptonIACanvas->SaveAs(MString("NumberComptonIAs.") + m_PostFix + ".root");
      
  for (int bx = 1; bx <= NumberPhotoIA->GetNbinsX(); ++bx) {
    NumberPhotoIA->SetBinError(bx, sqrt(NumberPhotoIA->GetBinContent(bx))/SimulatedEvents);
    NumberPhotoIA->SetBinContent(bx, NumberPhotoIA->GetBinContent(bx)/SimulatedEvents);
  }
  TCanvas* NumberPhotoIACanvas = new TCanvas();
  NumberPhotoIACanvas->cd();
  NumberPhotoIA->Draw();
  NumberPhotoIACanvas->Update();
  NumberPhotoIACanvas->SaveAs(MString("NumberPhotoIAs.") + m_PostFix + ".root");
      
  for (int bx = 1; bx <= NumberPairIA->GetNbinsX(); ++bx) {
    NumberPairIA->SetBinError(bx, sqrt(NumberPairIA->GetBinContent(bx))/SimulatedEvents);
    NumberPairIA->SetBinContent(bx, NumberPairIA->GetBinContent(bx)/SimulatedEvents);
  }
  TCanvas* NumberPairIACanvas = new TCanvas();
  NumberPairIACanvas->cd();
  NumberPairIA->Draw();
  NumberPairIACanvas->Update();
  NumberPairIACanvas->SaveAs(MString("NumberPairIAs.") + m_PostFix + ".root");
      
  for (int bx = 1; bx <= NumberRayleighIA->GetNbinsX(); ++bx) {
    NumberRayleighIA->SetBinError(bx, sqrt(NumberRayleighIA->GetBinContent(bx))/SimulatedEvents);
    NumberRayleighIA->SetBinContent(bx, NumberRayleighIA->GetBinContent(bx)/SimulatedEvents);
  }
  TCanvas* NumberRayleighIACanvas = new TCanvas();
  NumberRayleighIACanvas->cd();
  NumberRayleighIA->Draw();
  NumberRayleighIACanvas->Update();
  NumberRayleighIACanvas->SaveAs(MString("NumberRayleighIAs.") + m_PostFix + ".root");
  
  
  return true;
}


/******************************************************************************/

Shells* g_Prg = 0;
int g_NInterruptCatches = 1;

/******************************************************************************/


/******************************************************************************
 * Called when an interrupt signal is flagged
 * All catched signals lead to a well defined exit of the program
 */
void CatchSignal(int a)
{
  if (g_Prg != 0 && g_NInterruptCatches-- > 0) {
    cout<<"Catched signal Ctrl-C (ID="<<a<<"):"<<endl;
    g_Prg->Interrupt();
  } else {
    abort();
  }
}


/******************************************************************************
 * Main program
 */
int main(int argc, char** argv)
{
  // Catch a user interupt for graceful shutdown
  // signal(SIGINT, CatchSignal);

  // Initialize global MEGALIB variables, especially mgui, etc.
  MGlobal::Initialize("Standalone", "a standalone example program");

  TApplication ShellsApp("ShellsApp", 0, 0);

  g_Prg = new Shells();

  if (g_Prg->ParseCommandLine(argc, argv) == false) {
    cerr<<"Error during parsing of command line!"<<endl;
    return -1;
  } 
  if (g_Prg->Analyze() == false) {
    cerr<<"Error during analysis!"<<endl;
    return -2;
  } 

  //ShellsApp.Run();

  cout<<"Program exited normally!"<<endl;

  return 0;
}

/*
 * Cosima: the end...
 ******************************************************************************/
