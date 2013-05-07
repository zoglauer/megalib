/* 
 * DetectorEffectsEngine.cxx
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
#include "MGeometryRevan.h"
#include "MDDetector.h"
#include "MFileEventsEvta.h"
#include "MDVolumeSequence.h"
#include "MRERawEvent.h"
#include "MRESE.h"
#include "MTime.h"

/******************************************************************************/

class DetectorEffectsEngine
{
public:
  /// Default constructor
  DetectorEffectsEngine();
  /// Default destructor
  ~DetectorEffectsEngine();
  
  /// Parse the command line
  bool ParseCommandLine(int argc, char** argv);
  /// Analyze what eveer needs to be analyzed...
  bool Analyze();
  /// Interrupt the analysis
  void Interrupt() { m_Interrupt = true; }

private:
  /// True, if the analysis needs to be interrupted
  bool m_Interrupt;
  ///
  MString m_FileName;
  ///
  MString m_GeometryFileName;
};

/******************************************************************************/


/******************************************************************************
 * Default constructor
 */
DetectorEffectsEngine::DetectorEffectsEngine() : m_Interrupt(false)
{
  gStyle->SetPalette(1, 0);
}


/******************************************************************************
 * Default destructor
 */
DetectorEffectsEngine::~DetectorEffectsEngine()
{
  // Intentionally left blank
}


/******************************************************************************
 * Parse the command line
 */
bool DetectorEffectsEngine::ParseCommandLine(int argc, char** argv)
{
  ostringstream Usage;
  Usage<<endl;
  Usage<<"  Usage: DetectorEffectsEngine <options>"<<endl;
  Usage<<"    General options:"<<endl;
  Usage<<"         -f:   file name"<<endl;
  Usage<<"         -g:   geometry file name"<<endl;
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
    if (Option == "-f" || Option == "-g") {
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
bool DetectorEffectsEngine::Analyze()
{
  if (m_Interrupt == true) return false;

  // Load geometry:
  MGeometryRevan* Geometry = new MGeometryRevan();

  if (Geometry->ScanSetupFile(m_GeometryFileName) == false) {
    cout<<"Loading of geometry "<<Geometry->GetName()<<" failed!!"<<endl;
    return false;
  }  

  vector<TH1D*> Depths;
  vector<TH2D*> Surfaces;
  vector<TH1D*> Spectra;
  vector<MDVolumeSequence*> VolumeSequences;
  
  MFileEventsEvta Reader(Geometry);
  if (Reader.Open(m_FileName) == false) {
    cout<<"Unable to open sim file!"<<endl; 
    return false;
  }
  Reader.ShowProgress();

  MRERawEvent* Event = 0;
  while ((Event = Reader.GetNextEvent()) != 0) {
    for (int h = 0; h < Event->GetNRESEs(); ++h) {
      MDVolumeSequence* S = Event->GetRESEAt(h)->GetVolumeSequence();
      bool Found = false;
      unsigned int Index = numeric_limits<unsigned int>::max();
      for (unsigned int v = 0; v < VolumeSequences.size(); ++v) {
        if (VolumeSequences[v]->HasSameDetector(S) == true) {
          Index = v;
          Found = true;
          break;
        }
      }
      if (Found == false) {
        VolumeSequences.push_back(new MDVolumeSequence(*S));
        TH1D* Spectrum = new TH1D(S->GetDetectorVolume()->GetName() + "_Spectrum", S->GetDetectorVolume()->GetName() + "_Spectrum", 6000, 0, 3000);
        Spectra.push_back(Spectrum);
        TH1D* Depth = new TH1D(S->GetDetectorVolume()->GetName() + "_Depth", S->GetDetectorVolume()->GetName() + "_Depth", 500, -0.5, 0.5);
        Depths.push_back(Depth);
        TH2D* Surface = new TH2D(S->GetDetectorVolume()->GetName() + "_Surface", S->GetDetectorVolume()->GetName() + "_Surface", 400, -2.0, 2.0, 400, -2.0, 2.0);
        Surfaces.push_back(Surface);
        Index = VolumeSequences.size() -1;
      }
    
      Spectra[Index]->Fill(Event->GetRESEAt(h)->GetEnergy());
      MVector Pos = S->GetPositionInVolume(Event->GetRESEAt(h)->GetPosition(), S->GetDetectorVolume()); 
      Depths[Index]->Fill(Pos.Z());
      Surfaces[Index]->Fill(Pos.X(), Pos.Y());
    }
    
    delete Event;
  }

  Reader.Close();

  for (unsigned int s = 0; s < Spectra.size(); ++s) {
    //TCanvas* C = new TCanvas();
    //C->cd();
    //Spectra[s]->Draw();
    //C->Update();
    
    // Dump as root file
    MString FileName = VolumeSequences[s]->GetDetectorVolume()->GetName();
    FileName += "_t=spectrum";
    FileName += "_m=";
    FileName += g_Version;
    FileName += "_r=";
    FileName += gROOT->GetVersionInt();
    FileName += "_d=";
    MTime Now;
    FileName += Now.GetShortString();
    FileName += ".root";
    
    TFile* F = new TFile();
    F->Open(FileName, "RECREATE");
    Spectra[s]->Write();
    F->Close();
  }
  
  for (unsigned int s = 0; s < Depths.size(); ++s) {
    //TCanvas* C = new TCanvas();
    //C->cd();
    //Depths[s]->Draw();
    //C->Update();
    
    // Dump as root file
    MString FileName = VolumeSequences[s]->GetDetectorVolume()->GetName();
    FileName += "_t=depth";
    FileName += "_m=";
    FileName += g_Version;
    FileName += "_r=";
    FileName += gROOT->GetVersionInt();
    FileName += "_d=";
    MTime Now;
    FileName += Now.GetShortString();
    FileName += ".root";
    
    TFile* F = new TFile();
    F->Open(FileName, "RECREATE");
    Depths[s]->Write();
    F->Close();
  }
  
  for (unsigned int s = 0; s < Surfaces.size(); ++s) {
    //TCanvas* C = new TCanvas();
    //C->cd();
    //Surfaces[s]->Draw();
    //C->Update();
    
    // Dump as root file
    MString FileName = VolumeSequences[s]->GetDetectorVolume()->GetName();
    FileName += "_t=surface";
    FileName += "_m=";
    FileName += g_Version;
    FileName += "_r=";
    FileName += gROOT->GetVersionInt();
    FileName += "_d=";
    MTime Now;
    FileName += Now.GetShortString();
    FileName += ".root";
    
    TFile* F = new TFile(FileName, "RECREATE");
    F->cd();
    Surfaces[s]->Write();
    F->Write();
    F->Close();
  }
 
  
  return true;
}


/******************************************************************************/

DetectorEffectsEngine* g_Prg = 0;
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
  MGlobal::Initialize("DetectorEffectsEngine", "a DetectorEffectsEngine example program");

  TApplication DetectorEffectsEngineApp("DetectorEffectsEngineApp", 0, 0);

  g_Prg = new DetectorEffectsEngine();

  if (g_Prg->ParseCommandLine(argc, argv) == false) {
    cerr<<"Error during parsing of command line!"<<endl;
    return -1;
  } 
  if (g_Prg->Analyze() == false) {
    cerr<<"Error during analysis!"<<endl;
    return -2;
  } 

  //DetectorEffectsEngineApp.Run();

  cout<<"Program exited normally!"<<endl;

  return 0;
}

/*
 * Cosima: the end...
 ******************************************************************************/
