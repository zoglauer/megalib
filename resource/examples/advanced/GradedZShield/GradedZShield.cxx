/* 
 * GradedZ.cxx
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

class GradedZ
{
public:
  /// Default constructor
  GradedZ();
  /// Default destructor
  ~GradedZ();
  
  /// Parse the command line
  bool ParseCommandLine(int argc, char** argv);
  /// Analyze what eveer needs to be analyzed...
  bool Analyze();
  /// Interrupt the analysis
  void Interrupt() { m_Interrupt = true; }

private:
  /// True, if the analysis needs to be interrupted
  bool m_Interrupt;
  /// The file name
  MString m_FileName;
  /// Geometry file name
  MString m_GeoFileName;
  /// Maximum enetgy
  double m_MaximumEnergy;
};

/******************************************************************************/


/******************************************************************************
 * Default constructor
 */
GradedZ::GradedZ() : m_Interrupt(false), m_MaximumEnergy(1000)
{
  gStyle->SetPalette(1, 0);
}


/******************************************************************************
 * Default destructor
 */
GradedZ::~GradedZ()
{
  // Intentionally left blank
}


/******************************************************************************
 * Parse the command line
 */
bool GradedZ::ParseCommandLine(int argc, char** argv)
{
  ostringstream Usage;
  Usage<<endl;
  Usage<<"  Usage: GradedZShield <options>"<<endl;
  Usage<<"    General options:"<<endl;
  Usage<<"         -f:   sim file name"<<endl;
  Usage<<"         -g:   geometry file name"<<endl;
  Usage<<"         -m:   max energy"<<endl;
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
    if (Option == "-f") {
      if (!((argc > i+1) && 
            (argv[i+1][0] != '-' || isalpha(argv[i+1][1]) == 0))){
        cout<<"Error: Option "<<argv[i][1]<<" needs a second argument!"<<endl;
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
    } else if (Option == "-m") {
      m_MaximumEnergy = atof(argv[++i]);
      cout<<"Accepting file name: "<<m_MaximumEnergy<<endl;
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
bool GradedZ::Analyze()
{
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

  double Emin = 0.0;
  double Emax = m_MaximumEnergy;
  int NBins = (Emax - Emin) / 10; 

  TH1D* MeasuredSpectrum = new TH1D("", "Measured Spectrum", NBins, Emin, Emax);
  MeasuredSpectrum->SetXTitle("Energy of primary [keV]");
  MeasuredSpectrum->SetYTitle("cts");

  TH1D* AllHits = new TH1D("All Hits", "AllHits", NBins, Emin, Emax);
  
  TH1D* TransmissionCurve = new TH1D("TransmissionCurve", "Transmission Curve (any primary/secondary making it through the shield)", NBins, Emin, Emax);
  TransmissionCurve->SetXTitle("Energy of primary [keV]");
  TransmissionCurve->SetYTitle("Transmission [%]");
  TransmissionCurve->SetMaximum(100);
  TransmissionCurve->SetNdivisions(520, "Y");
  
  TH1D* FullyAbsorbed = new TH1D("FullyAbsorbed", "Percentage of events which are fully absorbed in the shield", NBins, Emin, Emax);
  FullyAbsorbed->SetXTitle("Energy of primary [keV]");
  FullyAbsorbed->SetYTitle("Fully absorbed [%]");
  FullyAbsorbed->SetMaximum(100);
  FullyAbsorbed->SetNdivisions(520, "Y");
 
  TH1D* TransmissionCurveUnharmed = new TH1D("TransmissionCurveUnharmed", "Transmission Curve (any primary making it through the shield unchanged)", NBins, Emin, Emax);
  TransmissionCurveUnharmed->SetXTitle("Energy of primary [keV]");
  TransmissionCurveUnharmed->SetYTitle("Transmission [%]");
  TransmissionCurveUnharmed->SetMaximum(100);
  TransmissionCurveUnharmed->SetNdivisions(520, "Y");
  
  TH1D* ComptonInteractions = new TH1D("ComptonInteractions", "Compton interaction in shield (with and without transmission)", NBins, Emin, Emax);
  ComptonInteractions->SetXTitle("Energy of primary [keV]");
  ComptonInteractions->SetYTitle("Compton interactions [%]");
  
  TH1D* ComptonInteractionWithT = new TH1D("ComptonInteractionWithT", "Compton interaction in shield with transmission", NBins, Emin, Emax);
  ComptonInteractionWithT->SetXTitle("Energy of primary [keV]");
  ComptonInteractionWithT->SetYTitle("Compton interactions [%]");

  MSimEvent* SiEvent = 0;
  while ((SiEvent = SiReader.GetNextEvent(false)) != 0) {
    if (SiEvent->GetNIAs() > 0) {
      double Energy = SiEvent->GetIAAt(0)->GetSecondaryEnergy();
      AllHits->Fill(Energy, 1);
      bool HasCompton = false;
      bool HasInteraction = false;
      double BlackAbsorberEnergy = 0;
      double EscapedEnergy = 0;
      for (unsigned int i = 1; i < SiEvent->GetNIAs(); ++i) {
        if (SiEvent->GetIAAt(i)->GetProcess() == "BLAK") { //
          MeasuredSpectrum->Fill(SiEvent->GetIAAt(i)->GetMotherEnergy());
          BlackAbsorberEnergy += SiEvent->GetIAAt(i)->GetMotherEnergy();
        } else {
          HasInteraction = true;
        }
        if (SiEvent->GetIAAt(i)->GetProcess() == "ESCP") { // We can have multiples
          EscapedEnergy += SiEvent->GetIAAt(i)->GetMotherEnergy();
        }
        if (SiEvent->GetIAAt(i)->GetProcess() == "COMP") { // We can have multiples
          HasCompton = true;
        }
      }
      if (HasCompton == true) {
        ComptonInteractions->Fill(Energy, 1);
        if (BlackAbsorberEnergy > 0) {
          ComptonInteractionWithT->Fill(Energy, 1);
        }
      }
      if (BlackAbsorberEnergy > 0) {
        TransmissionCurve->Fill(Energy, 1);
      }
      if (HasInteraction == false) {
        TransmissionCurveUnharmed->Fill(Energy, 1);
      }
      if (fabs(EscapedEnergy + BlackAbsorberEnergy - Energy) < 0.001) {
        FullyAbsorbed->Fill(Energy, 1);
      }
    }
    delete SiEvent;
  }

  SiReader.Close();

  // Normalize:
  for (int b = 1; b <= NBins; ++b) {
    if (AllHits->GetBinContent(b) > 0) {
      ComptonInteractions->SetBinContent(b, 100* ComptonInteractions->GetBinContent(b)/AllHits->GetBinContent(b));
      ComptonInteractionWithT->SetBinContent(b, 100*ComptonInteractionWithT->GetBinContent(b)/AllHits->GetBinContent(b));
      TransmissionCurve->SetBinContent(b, 100* TransmissionCurve->GetBinContent(b)/AllHits->GetBinContent(b));
      TransmissionCurveUnharmed->SetBinContent(b, 100* TransmissionCurveUnharmed->GetBinContent(b)/AllHits->GetBinContent(b));
      FullyAbsorbed->SetBinContent(b, 100 - 100*FullyAbsorbed->GetBinContent(b)/AllHits->GetBinContent(b));
    } else {
      ComptonInteractions->SetBinContent(b, 0);
      ComptonInteractionWithT->SetBinContent(b, 0);
      TransmissionCurve->SetBinContent(b, 0);
      TransmissionCurveUnharmed->SetBinContent(b, 0);
      FullyAbsorbed->SetBinContent(b, 0);
    }
  }

  TCanvas* MeasuredSpectrumCanvas = new TCanvas();
  MeasuredSpectrumCanvas->cd();
  MeasuredSpectrum->Draw();
  MeasuredSpectrumCanvas->Update();
  MeasuredSpectrumCanvas->SaveAs("SpectrumBehindShield.pdf");

  TCanvas* ComptonInteractionsCanvas = new TCanvas();
  ComptonInteractionsCanvas->cd();
  ComptonInteractions->Draw();
  ComptonInteractionsCanvas->Update();
  ComptonInteractionsCanvas->SaveAs("ComptonInteractionsWithAndWithoutTransmission.pdf");

  TCanvas* ComptonInteractionWithTCanvas = new TCanvas();
  ComptonInteractionWithTCanvas->cd();
  ComptonInteractionWithT->Draw();
  ComptonInteractionWithTCanvas->Update();
  ComptonInteractionWithTCanvas->SaveAs("ComptonInteractionsWithTransmission.pdf");

  TCanvas* TransmissionCurveCanvas = new TCanvas();
  TransmissionCurveCanvas->cd();
  TransmissionCurveCanvas->SetGridx();
  TransmissionCurveCanvas->SetGridy();
  TransmissionCurve->Draw();
  TransmissionCurveCanvas->Update();
  TransmissionCurveCanvas->SaveAs("TransmissionCurve.pdf");

  TCanvas* TransmissionCurveUnharmedCanvas = new TCanvas();
  TransmissionCurveUnharmedCanvas->cd();
  TransmissionCurveUnharmedCanvas->SetGridx();
  TransmissionCurveUnharmedCanvas->SetGridy();
  TransmissionCurveUnharmed->Draw();
  TransmissionCurveUnharmedCanvas->Update();
  TransmissionCurveUnharmedCanvas->SaveAs("TransmissionCurveUnharmed.pdf");

  TCanvas* FullyAbsorbedCanvas = new TCanvas();
  FullyAbsorbedCanvas->cd();
  FullyAbsorbedCanvas->SetGridx();
  FullyAbsorbedCanvas->SetGridy();
  FullyAbsorbed->Draw();
  FullyAbsorbedCanvas->Update();
  FullyAbsorbedCanvas->SaveAs("TransmissionCurveUnharmed.pdf");

  cout<<"The simulation file contained "<<SiReader.GetSimulatedEvents()<<" events simulated over an observation time of "<<SiReader.GetObservationTime()<<endl;

  return true;
}


/******************************************************************************/

GradedZ* g_Prg = 0;
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
  MGlobal::Initialize("GradedZ", "a GradedZ example program");

  TApplication GradedZApp("GradedZApp", 0, 0);

  g_Prg = new GradedZ();

  if (g_Prg->ParseCommandLine(argc, argv) == false) {
    cerr<<"Error during parsing of command line!"<<endl;
    return -1;
  } 
  if (g_Prg->Analyze() == false) {
    cerr<<"Error during analysis!"<<endl;
    return -2;
  } 

  GradedZApp.Run();

  cout<<"Program exited normally!"<<endl;

  return 0;
}

/*
 * Cosima: the end...
 ******************************************************************************/
