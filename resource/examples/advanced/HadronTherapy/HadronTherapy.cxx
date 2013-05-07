/* 
 * HadronTherapy.cxx
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
#include <map>
using namespace std;

// ROOT
#include <TApplication.h>
#include <TStyle.h>
#include <TH1.h>
#include <TH2.h>
#include <TH3.h>
#include <TCanvas.h>
#include <MString.h>

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

class HadronTherapy
{
public:
  /// Default constructor
  HadronTherapy();
  /// Default destructor
  ~HadronTherapy();
  
  /// Parse the command line
  bool ParseCommandLine(int argc, char** argv);
  /// Analyze whatever needs to be analyzed...
  bool Analyze();
  /// Analyze the event, return true if it has to be writen to file
  bool AnalyzeEvent(MSimEvent& Event);
  /// Interrupt the analysis
  void Interrupt() { m_Interrupt = true; }

  /// Show spectra per detector
  bool Spectra();

private:
  /// True, if the analysis needs to be interrupted
  bool m_Interrupt;

  /// Simulation file name
  MString m_FileName;
  /// Geometry file name
  MString m_GeoFileName;
  
  /// The center
  MVector m_Center;
  /// The distance
  double m_Distance;
};

/******************************************************************************/


/******************************************************************************
 * Default constructor
 */
HadronTherapy::HadronTherapy() : m_Interrupt(false)
{
  gStyle->SetPalette(1, 0);
  m_Center = MVector(0, 0, 0);
  m_Distance = 10.0;
}


/******************************************************************************
 * Default destructor
 */
HadronTherapy::~HadronTherapy()
{
  // Intentionally left blank
}


/******************************************************************************
 * Parse the command line
 */
bool HadronTherapy::ParseCommandLine(int argc, char** argv)
{
  ostringstream Usage;
  Usage<<endl;
  Usage<<"  Usage: HadronTherapy <options>"<<endl;
  Usage<<"    General options:"<<endl;
  Usage<<"         -f:   simulation file name"<<endl;
  Usage<<"         -g:   geometry file name"<<endl;
  Usage<<"         -p:   position - center (x,y,z) + distance: <x in cm> <y in cm> <z in cm> <d in cm> (default: -p 0 0 0 10)"<<endl;
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
    if (Option == "-f" || Option == "-o") {
      if (!((argc > i+1) && argv[i+1][0] != '-')){
        cout<<"Error: Option "<<argv[i][1]<<" needs a second argument!"<<endl;
        cout<<Usage.str()<<endl;
        return false;
      }
    } 
    // Multiple arguments_
    else if (Option == "-p") {
      if (!((argc > i+4) && argv[i+1][0] != '-' && argv[i+2][0] != '-' && argv[i+3][0] != '-' && argv[i+4][0] != '-')){
        cout<<"Error: Option "<<argv[i][1]<<" needs four arguments!"<<endl;
        cout<<Usage.str()<<endl;
        return false;
      }
    }

    // Then fulfill the options:
    if (Option == "-f") {
      m_FileName = argv[++i];
      cout<<"Accepting file name: "<<m_FileName<<endl;
    } else if (Option == "-g") {
      m_GeoFileName = argv[++i];
      cout<<"Accepting file name: "<<m_GeoFileName<<endl;
    } else if (Option == "-p") {
      m_Center = MVector(atof(argv[++i]), atof(argv[++i]), atof(argv[++i]));
      m_Distance = atof(argv[++i]);
      cout<<"Accepting position: "<<m_Center<<" with distance "<<m_Distance<<endl;
    } else {
      cout<<"Error: Unknown option \""<<Option<<"\"!"<<endl;
      cout<<Usage.str()<<endl;
      return false;
    }
  }

  if (m_FileName == "") {
    cout<<"Error: Need a simulation file name!"<<endl;
    cout<<Usage.str()<<endl;
    return false;
  }

  if (m_GeoFileName == "") {
    cout<<"Error: Need a geometry file name!"<<endl;
    cout<<Usage.str()<<endl;
    return false;
  }

  return true;
}


/******************************************************************************
 * Do whatever analysis is necessary
 */
bool HadronTherapy::Analyze()
{
  if (m_Interrupt == true) return false;

  // Load geometry:
  MDGeometryQuest Geometry;

  if (Geometry.ScanSetupFile(m_GeoFileName) == true) {
    cout<<"Geometry "<<Geometry.GetName()<<" loaded!"<<endl;
    Geometry.ActivateNoising(false);
    Geometry.SetGlobalFailureRate(0.0);
  } else {
    cout<<"Loading of geometry "<<Geometry.GetName()<<" failed!!"<<endl;
    return false;
  }  

  MFileEventsSim SiReader(&Geometry);
  if (SiReader.Open(m_FileName) == false) {
    cout<<"Unable to open sim file!"<<endl; 
    return false;
  }
  cout<<"Opened file "<<SiReader.GetFileName()<<" created with MEGAlib version: "<<SiReader.GetMEGAlibVersion()<<endl;
  SiReader.ShowProgress();

  int xBins = 100;
  double xMin = m_Center.X() - m_Distance;
  double xMax = m_Center.X() + m_Distance;
  int yBins = 100;
  double yMin = m_Center.Y() - m_Distance;
  double yMax = m_Center.Y() + m_Distance;
  int zBins = 100;
  double zMin = m_Center.Z() - m_Distance;
  double zMax = m_Center.Z() + m_Distance;
  
  int eBins = 200;
  double eMin = 1;
  double eMax = 10000;
  
  map<int, TH3D*> xyzHistogram;
  map<int, TH1D*> Spectrum;
  
  MSimEvent* SiEvent = 0;
  while ((SiEvent = SiReader.GetNextEvent(false)) != 0) {
    for (unsigned int i = 0; i < SiEvent->GetNIAs(); ++i) {
      MSimIA* IA = SiEvent->GetIAAt(i); 
      if (IA->GetProcess() == "INEL" || IA->GetProcess() == "DECA") {   
        if (IA->GetSecondaryParticleID() < 100) {
          TH3D* Hist = xyzHistogram[IA->GetSecondaryParticleID()];
          if (Hist == 0) {
            ostringstream out;
            out<<"Emission pattern for particle type: "<<IA->GetSecondaryParticleID();
            Hist = new TH3D(out.str().c_str(), out.str().c_str(), xBins, xMin, xMax, yBins, yMin, yMax, zBins, zMin, zMax);
            Hist->SetXTitle("x [cm]");
            Hist->SetYTitle("y [cm]");
            Hist->SetZTitle("z [cm]");
            xyzHistogram[IA->GetSecondaryParticleID()] = Hist;
          }
          MVector Pos = IA->GetPosition();
          Hist->Fill(Pos.X(), Pos.Y(), Pos.Z());
          
          TH1D* S = Spectrum[IA->GetSecondaryParticleID()];
          if (S == 0) {
            ostringstream out;
            out<<"Spectrum for particle type: "<<IA->GetSecondaryParticleID();
            S = new TH1D(out.str().c_str(), out.str().c_str(), eBins, eMin, eMax);
            S->SetXTitle("Energy [keV]");
            Spectrum[IA->GetSecondaryParticleID()] = S;
          }
          S->Fill(IA->GetSecondaryEnergy());
        }
      }
    }
    
    delete SiEvent;
  }

  SiReader.Close();


  for (map<int, TH3D*>::iterator Iter = xyzHistogram.begin(); 
       Iter != xyzHistogram.end(); ++Iter) {
    // Create projections ...
    TH2D* xyProjection = new TH2D(MString((*Iter).second->GetTitle()) + " - xy", MString((*Iter).second->GetTitle()) + " - xy", xBins, xMin, xMax, yBins, yMin, yMax);
    xyProjection->SetXTitle("x [cm]");
    xyProjection->SetYTitle("y [cm]");
    TH2D* xzProjection = new TH2D(MString((*Iter).second->GetTitle()) + " - xz", MString((*Iter).second->GetTitle()) + " - xz", xBins, xMin, xMax, zBins, zMin, zMax);
    xzProjection->SetXTitle("x [cm]");
    xzProjection->SetYTitle("z [cm]");
    TH2D* yzProjection = new TH2D(MString((*Iter).second->GetTitle()) + " - yz", MString((*Iter).second->GetTitle()) + " - yz", yBins, yMin, yMax, zBins, zMin, zMax);
    yzProjection->SetXTitle("y [cm]");
    yzProjection->SetYTitle("z [cm]");
    TH1D* xProjection = new TH1D(MString((*Iter).second->GetTitle()) + " - x", MString((*Iter).second->GetTitle()) + " - xy", xBins, xMin, xMax);
    xProjection->SetXTitle("x [cm]");
    xProjection->SetYTitle("counts");
    TH1D* yProjection = new TH1D(MString((*Iter).second->GetTitle()) + " - y", MString((*Iter).second->GetTitle()) + " - xz", yBins, yMin, yMax);
    yProjection->SetXTitle("y [cm]");
    yProjection->SetYTitle("counts");
    TH1D* zProjection = new TH1D(MString((*Iter).second->GetTitle()) + " - z", MString((*Iter).second->GetTitle()) + " - yz", zBins, zMin, zMax);
    zProjection->SetXTitle("z [cm]");
    zProjection->SetYTitle("counts");
    for (int bx = 1; bx <= xBins; ++bx) {
      for (int by = 1; by <= yBins; ++by) {
        for (int bz = 1; bz <= zBins; ++bz) {
          xyProjection->SetBinContent(bx, by, xyProjection->GetBinContent(bx, by) + (*Iter).second->GetBinContent(bx, by, bz));
          xzProjection->SetBinContent(bx, bz, xzProjection->GetBinContent(bx, bz) + (*Iter).second->GetBinContent(bx, by, bz));
          yzProjection->SetBinContent(by, bz, yzProjection->GetBinContent(by, bz) + (*Iter).second->GetBinContent(bx, by, bz));
          xProjection->SetBinContent(bx, xProjection->GetBinContent(bx) + (*Iter).second->GetBinContent(bx, by, bz));
          yProjection->SetBinContent(by, yProjection->GetBinContent(by) + (*Iter).second->GetBinContent(bx, by, bz));
          zProjection->SetBinContent(bz, zProjection->GetBinContent(bz) + (*Iter).second->GetBinContent(bx, by, bz));
        }
      }
    }
    // And draw them...
    TCanvas* Canvas = new TCanvas((*Iter).second->GetTitle(), (*Iter).second->GetTitle(), 700, 700);
    Canvas->Divide(3, 3);
    Canvas->cd(1);
    (*Iter).second->Draw();
    Canvas->cd(2);
    Spectrum[(*Iter).first]->Draw();
    Canvas->cd(4);
    xyProjection->Draw("colz");
    Canvas->cd(5);
    xzProjection->Draw("colz");
    Canvas->cd(6);
    yzProjection->Draw("colz");
    Canvas->cd(7);
    xProjection->Draw();
    Canvas->cd(8);
    yProjection->Draw();
    Canvas->cd(9);
    zProjection->Draw();
    Canvas->Update();
  }
  
  
  return true;
}


/******************************************************************************/

HadronTherapy* g_Prg = 0;

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
  //void (*handler)(int);
  //handler = CatchSignal;
  //(void) signal(SIGINT, CatchSignal);

  // Initialize global MEGAlib variables, especially mgui, etc.
  MGlobal::Initialize();

  TApplication HadronTherapyApp("HadronTherapyApp", 0, 0);

  g_Prg = new HadronTherapy();

  if (g_Prg->ParseCommandLine(argc, argv) == false) {
    cerr<<"Error during parsing of command line!"<<endl;
    return -1;
  } 
  if (g_Prg->Analyze() == false) {
    cerr<<"Error during analysis!"<<endl;
    return -2;
  } 

  HadronTherapyApp.Run();

  cout<<"Program exited normally!"<<endl;

  return 0;
}

/*
 * Cosima: the end...
 ******************************************************************************/
