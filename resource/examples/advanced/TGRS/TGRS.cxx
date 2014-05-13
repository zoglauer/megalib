/* 
 * TGRS.cxx
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
#include <TLegend.h>

// MEGAlib
#include "MGlobal.h"
#include "MStreams.h"
#include "MParser.h"
#include "MTokenizer.h"
#include "MDGeometryQuest.h"
#include "MGeometryRevan.h"
#include "MSimEvent.h"
#include "MSimHT.h"
#include "MFileEventsSim.h"
#include "MFileEventsEvta.h"
#include "MRERawEvent.h"


/******************************************************************************/

class TGRS
{
public:
  /// Default constructor
  TGRS();
  /// Default destructor
  ~TGRS();
  
  /// Parse the command line
  bool ParseCommandLine(int argc, char** argv);
  /// Analyze what eveer needs to be analyzed...
  bool Analyze();
  /// Interrupt the analysis
  void Interrupt() { m_Interrupt = true; }
  /// Interrupt the analysis
  bool GetInterrupt() { return m_Interrupt; }

private:
  /// True, if the analysis needs to be interrupted
  bool m_Interrupt;

  MString m_GeometryFileName;

  MString m_TGRSDataFileName;

  vector<MString> m_BackgroundTypeNames;
  vector<MString> m_BackgroundFileNames;

  bool m_PerfectEnergyResolution;
};

/******************************************************************************/


/******************************************************************************
 * Default constructor
 */
TGRS::TGRS() : m_Interrupt(false)
{
  gStyle->SetPalette(1, 0);
  m_PerfectEnergyResolution = false;
}


/******************************************************************************
 * Default destructor
 */
TGRS::~TGRS()
{
  // Intentionally left blanck
}


/******************************************************************************
 * Parse the command line
 */
bool TGRS::ParseCommandLine(int argc, char** argv)
{
  ostringstream Usage;
  Usage<<endl;
  Usage<<"  Usage: TGRS <options>"<<endl;
  Usage<<"    General options:"<<endl;
  Usage<<"         -t:   TGRS data file name"<<endl;
  Usage<<"         -g:   geometry file name"<<endl;
  Usage<<"         -b <name> <file>:   Name of the component    Background file name"<<endl;
  Usage<<"         -p:   perfect energy resolution"<<endl;
  Usage<<"         -h:   print this help"<<endl;
  Usage<<endl;

  string Option;

  // Check for help
  for (int i = 1; i < argc; i++) {
    Option = argv[i];
    if (Option == "-h" || Option == "--help" || Option == "?" || Option == "-?") {
      cout<<Usage.str()<<endl;
      return false;
    }
  }

  // Now parse the command line options:
  for (int i = 1; i < argc; i++) {
    Option = argv[i];

    // First check if each option has sufficient arguments:
    // Single argument
    if (Option == "-t" || Option == "-f") {
      if (!((argc > i+1) && 
            (argv[i+1][0] != '-' || isalpha(argv[i+1][1]) == 0))){
        cout<<"Error: Option "<<argv[i][1]<<" needs a second argument!"<<endl;
        cout<<Usage.str()<<endl;
        return false;
      }
    } 
    // Multiple arguments template
    else if (Option == "-b") {
      if (!((argc > i+2) && 
            (argv[i+1][0] != '-' || isalpha(argv[i+1][1]) == 0) && 
            (argv[i+2][0] != '-' || isalpha(argv[i+2][1]) == 0))){
        cout<<"Error: Option "<<argv[i][1]<<" needs two arguments!"<<endl;
        cout<<Usage.str()<<endl;
        return false;
      }
    }

    // Then fulfill the options:
    if (Option == "-t") {
      m_TGRSDataFileName = argv[++i];
      cout<<"Accepting TGRS Data file name: "<<m_TGRSDataFileName<<endl;
    } else if (Option == "-g") {
      m_GeometryFileName = argv[++i];
      cout<<"Accepting geometry file name: "<<m_GeometryFileName<<endl;
    } else if (Option == "-b") {
      m_BackgroundTypeNames.push_back(argv[++i]);
      m_BackgroundFileNames.push_back(argv[++i]);
      cout<<"Accepting background file name: "<<m_BackgroundFileNames.back()<<" ("<<m_BackgroundTypeNames.back()<<")"<<endl;
    } else if (Option == "-p") {
      m_PerfectEnergyResolution = true;
      cout<<"Assuming perfect energy resolution"<<endl;
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
bool TGRS::Analyze()
{
  if (m_Interrupt == true) return false;

  // Load the TGRS data:
  MParser P;
  if (P.Open(m_TGRSDataFileName) == false) {
    mout<<"Unable to open TGRS data file"<<endl;
    return false;
  }

  vector<double> Emin;
  vector<double> Emax;
  vector<double> Flux;

  bool Start = false;
  for (unsigned int l = 0; l < P.GetNLines(); ++l) {
    MTokenizer* T = P.GetTokenizerAt(l);
    if (T->GetNTokens() > 0) {
      if (T->GetTokenAt(0) == "DATA") {
        Start = true;
      }
      if (Start == true) {
        if (T->GetNTokens() == 5) {
          Emin.push_back(T->GetTokenAtAsDouble(1));
          Emax.push_back(T->GetTokenAtAsDouble(2));
          Flux.push_back(T->GetTokenAtAsDouble(3));
        }
      }
    }
  }
  P.Close();

  int NBins = Emin.size()+1;
  double* Bins = new double[NBins];
  for (int b = 0; b < NBins-1; ++b) {
    Bins[b] = Emin[b];
  }
  Bins[NBins-1] = Emax.back();

  for (int b = 0; b < NBins-1; ++b) {
    if (Bins[b] >= Bins[b+1]) {
      cout<<"Bins not increasing: "<<Bins[b]<<":"<<Bins[b+1]<<endl;
    }
  }

  TH1D* TGRSHist = new TH1D("TGRS", "TGRS measurements", NBins-1, Bins);
  TGRSHist->SetXTitle("Energy in keV");
  TGRSHist->SetYTitle("Hits in cts/keV/s");
  for (int b = 1; b <= NBins; ++b) {
    TGRSHist->SetBinContent(b, Flux[b-1]);
  }


  // The original file is in ph/cm/s/keV thus we have to mutliple with the surface area of 35 cm2
  TGRSHist->Scale(35); 

  TCanvas* TGRSCanvas = new TCanvas();
  TGRSCanvas->cd();
  TGRSCanvas->SetLogx();
  TGRSCanvas->SetLogy();
  TGRSHist->SetMaximum(100);
  TGRSHist->SetMaximum(100);
  TGRSHist->Draw();
  TGRSCanvas->Update();

  // Load geometry:
  //MGeometryRevan* Geometry = new MGeometryRevan();
  MDGeometryQuest* Geometry = new MDGeometryQuest();

  if (Geometry->ScanSetupFile(m_GeometryFileName) == true) {
    Geometry->ActivateNoising(!m_PerfectEnergyResolution);
    cout<<"Geometry "<<Geometry->GetName()<<" loaded!"<<endl;
  } else {
    cout<<"Loading of geometry "<<Geometry->GetName()<<" failed!!"<<endl;
    return false;
  }  

  /*
  int NBinsSec = 0;
  double* BinsSec = new double[NBins];
  for (int b = 0; b < NBins-1; ++b) { 
    //for (int b = 0; b < NBins-1; ((Emin[b] < 200) ? b += 1 : ((Emin[b] < 1000) ? b += 2 : ((Emin[b] < 2000) ? b += 4 : ((Emin[b] < 5000) ? b += 8 : b += 16))))) {
    BinsSec[NBinsSec] = Emin[b];
    NBinsSec++;
  }
  */

  int NBinsSec = 1500;
  double* BinsSec = new double[NBinsSec+1];
 
  double Min = log(Emin[0]);
  double Max = log(Emax.back());
  double Dist = (Max-Min)/(NBinsSec);
  for (int i = 0; i < NBinsSec+1; ++i) {
    BinsSec[i] = exp(Min+i*Dist);
  }  
 

  // Open background files:
  //vector<MFileEventsEvta*> SimFiles;
  vector<MFileEventsSim*> SimFiles;
  vector<TH1D*> Hists;
  for (unsigned int i = 0; i < m_BackgroundFileNames.size(); ++i) {
    //MFileEventsEvta* SiReader = new MFileEventsEvta(Geometry);
    MFileEventsSim* SiReader = new MFileEventsSim(Geometry);
    if (SiReader->Open(m_BackgroundFileNames[i]) == false) {
      cout<<"Unable to open sim file!"<<endl; 
      return false;
    }
    cout<<"Opened file "<<SiReader->GetFileName()<<" created with MEGAlib version: "<<SiReader->GetMEGAlibVersion()<<endl;
    SimFiles.push_back(SiReader);
    Hists.push_back(new TH1D(m_BackgroundFileNames[i], m_BackgroundFileNames[i], NBinsSec-1, BinsSec));
  }

  // Plot the backgrounds
  for (unsigned int i = 0; i < SimFiles.size(); ++i) {
    // Fill
    //MRERawEvent* Event = 0;
    SimFiles[i]->ShowProgress();
    MSimEvent* Event = 0;
    while ((Event = SimFiles[i]->GetNextEvent()) != 0) {
      /*
      if (Event->GetNIAs() > 0 && Event->GetIAAt(0)->GetSecondaryParticleID() == 72175) { // Hafnium-Bug
        delete Event;
        continue;
      }
      */

      //Hists[i]->Fill(Event->GetEnergy());
      Hists[i]->Fill(Event->GetREnergy());

      /*
      if (Event->GetREnergy() > 395 && Event->GetREnergy() < 399) {
        if (Event->GetNIAs() > 0) {
          cout<<Event->GetIAAt(0)->GetSecondaryParticleID()<<": "<<Event->GetREnergy()<<endl;
        }
      }
      */

      delete Event;

      if (m_Interrupt == true) {
        m_Interrupt = false;
        break;
      }
    }

    // Normalize:
    for (int b = 1; b <= Hists[i]->GetNbinsX(); ++b) {
      Hists[i]->SetBinContent(b, Hists[i]->GetBinContent(b)/Hists[i]->GetBinWidth(b));
    }
    cout<<"Obs Time: "<<SimFiles[i]->GetObservationTime()<<endl;
    Hists[i]->Scale(1.0/SimFiles[i]->GetObservationTime());

    Hists[i]->SetLineColor(3+i);
    Hists[i]->Draw("SAME");
  }

  TH1D* SimHist = new TH1D("Sim", "Sim", NBinsSec-1, BinsSec);
  for (unsigned int i = 0; i < SimFiles.size(); ++i) {
    for (int b = 1; b <= Hists[i]->GetNbinsX(); ++b) {
      SimHist->SetBinContent(b, SimHist->GetBinContent(b) + Hists[i]->GetBinContent(b));
    }
  }

  SimHist->SetLineColor(2);
  SimHist->Draw("SAME");


  TLegend *legend = new TLegend(0.5,0.7,0.88,0.88, NULL, "brNDC");
  legend->AddEntry(TGRSHist,"Measurements","f");
  legend->AddEntry(SimHist,"Combined Simulations","f");
  for (unsigned int i = 0; i < SimFiles.size(); ++i) {
    legend->AddEntry(Hists[i], m_BackgroundTypeNames[i], "f");
  }
  legend->Draw();


  TGRSHist->Draw("SAME");
  TGRSCanvas->Update();

  TH1D* DifferenceHist = new TH1D("Difference", "Difference measurements", NBinsSec-1, BinsSec);
  DifferenceHist->SetXTitle("Energy in keV");
  DifferenceHist->SetYTitle("Hits in cts/keV/s");
  for (int b = 1; b <= DifferenceHist->GetNbinsX(); ++b) {
    if (SimHist->GetBinContent(b) > 0) {
      DifferenceHist->SetBinContent(b, TGRSHist->GetBinContent(b)/SimHist->GetBinContent(b));
    } else {
      DifferenceHist->SetBinContent(b, 1.0);
    }
  }

  TCanvas* DifferenceCanvas = new TCanvas();
  DifferenceCanvas->cd();
  DifferenceCanvas->SetLogx();
  DifferenceCanvas->SetLogy();
  DifferenceHist->Draw();
  DifferenceCanvas->Update();

  delete Geometry;

  return true;
}


/******************************************************************************/

TGRS* g_Prg = 0;
int g_NInterruptCatches = 2;

/******************************************************************************/


/******************************************************************************
 * Called when an interrupt signal is flagged
 * All catched signals lead to a well defined exit of the program
 */
void CatchSignal(int a)
{
  if (g_Prg != 0) {
    cout<<"Catched signal Ctrl-C (ID="<<a<<"):"<<endl;
    if (g_Prg->GetInterrupt() == true) abort();
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
  // Set a default error handler and catch some signals...
  signal(SIGINT, CatchSignal);

  // Initialize global MEGALIB variables, especially mgui, etc.
  MGlobal::Initialize();

  TApplication TGRSApp("TGRSApp", 0, 0);

  g_Prg = new TGRS();

  if (g_Prg->ParseCommandLine(argc, argv) == false) {
    cerr<<"Error during parsing of command line!"<<endl;
    return -1;
  } 
  if (g_Prg->Analyze() == false) {
    cerr<<"Error during analysis!"<<endl;
    return -2;
  } 

  TGRSApp.Run();

  cout<<"Program exited normally!"<<endl;

  return 0;
}

/*
 * Cosima: the end...
 ******************************************************************************/
