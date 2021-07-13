/* 
 * PolAnalyzer.cxx
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
#include <TMath.h>
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
#include "MGeometryRevan.h"
#include "MDDetector.h"
#include "MFileEventsEvta.h"
#include "MDVolumeSequence.h"
#include "MRERawEvent.h"
#include "MRESE.h"

/******************************************************************************/

class PolAnalyzer
{
public:
  /// Default constructor
  PolAnalyzer();
  /// Default destructor
  ~PolAnalyzer();
  
  /// Parse the command line
  bool ParseCommandLine(int argc, char** argv);
  /// Analyze what eveer needs to be analyzed...
  bool Analyze();
  /// Interrupt the analysis
  void Interrupt() { m_Interrupt = true; }

private:
  /// True, if the analysis needs to be interrupted
  bool m_Interrupt;
  
  /// The sim file name with polarization input
  TString m_PolFileName;
  /// The sim file name with unpolarization input
  TString m_NotPolFileName;

  /// The geometry file name
  TString m_GeometryFileName;
  
  /// The minimum energy
  double m_EnergyMin;
  /// The maximum energy
  double m_EnergyMax;
};

/******************************************************************************/

double Modulation(double* x, double* par)
{
  return par[0] + par[1]*cos(2*(x[0]-par[2] + 90.0)*c_Rad);
};

/******************************************************************************
 * Default constructor
 */
PolAnalyzer::PolAnalyzer() : m_Interrupt(false), m_EnergyMin(0), m_EnergyMax(1000000)
{
  gStyle->SetPalette(1, 0);
}


/******************************************************************************
 * Default destructor
 */
PolAnalyzer::~PolAnalyzer()
{
  // Intentionally left blanck
}


/******************************************************************************
 * Parse the command line
 */
bool PolAnalyzer::ParseCommandLine(int argc, char** argv)
{
  ostringstream Usage;
  Usage<<endl;
  Usage<<"  Usage: PolAnalyzer <options>"<<endl;
  Usage<<"    General options:"<<endl;
  Usage<<"         -p:   sim file name with polarized data"<<endl;
  Usage<<"         -u:   sim file name with UN-polarized data"<<endl;
  Usage<<"         -g:   geometry file name"<<endl;
  Usage<<"         -e:   energy min, energy max"<<endl;
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
    if (Option == "-p" || Option == "-u" || Option == "-g") {
      if (!((argc > i+1) && 
            (argv[i+1][0] != '-' || isalpha(argv[i+1][1]) == 0))){
        cout<<"Error: Option "<<argv[i][1]<<" needs a second argument!"<<endl;
        cout<<Usage.str()<<endl;
        return false;
      }
    } 
    // Multiple arguments template
    else if (Option == "-e") {
      if (!((argc > i+2) && 
            (argv[i+1][0] != '-' || isalpha(argv[i+1][1]) == 0) && 
            (argv[i+2][0] != '-' || isalpha(argv[i+2][1]) == 0))){
        cout<<"Error: Option "<<argv[i][1]<<" needs two arguments!"<<endl;
        cout<<Usage.str()<<endl;
        return false;
      }
    }

    // Then fulfill the options:
    if (Option == "-p") {
      m_PolFileName = argv[++i];
      cout<<"Accepting file name with polarized data: "<<m_PolFileName<<endl;
    } else if (Option == "-u") {
      m_NotPolFileName = argv[++i];
      cout<<"Accepting file name with UN-polarized data: "<<m_NotPolFileName<<endl;
    } else if (Option == "-g") {
      m_GeometryFileName = argv[++i];
      cout<<"Accepting geometry file name: "<<m_GeometryFileName<<endl;
    } else if (Option == "-e") {
      m_EnergyMin = atof(argv[++i]);
      m_EnergyMax = atof(argv[++i]);
      cout<<"Accepting energy limits: "<<m_EnergyMin<<" & " <<m_EnergyMax<<endl;
    } else {
      cout<<"Error: Unknown option \""<<Option<<"\"!"<<endl;
      cout<<Usage.str()<<endl;
      return false;
    }
  }

  if (m_PolFileName == "") {
    cout<<"Error: Please give a file name!"<<endl;
    return false;
  }
  if (m_GeometryFileName == "") {
    cout<<"Error: Please give a geometry file name!"<<endl;
    return false;
  }

  return true;
}


/******************************************************************************
 * Do whatever analysis is necessary
 */
bool PolAnalyzer::Analyze()
{
  if (m_Interrupt == true) return false;

  // Load geometry:
  MGeometryRevan Geometry;

  if (Geometry.ScanSetupFile(m_GeometryFileName.Data()) == true) {
    cout<<"Geometry "<<Geometry.GetName()<<" loaded!"<<endl;
    Geometry.ActivateNoising(false);
    Geometry.SetGlobalFailureRate(0.0);
  } else {
    cout<<"Loading of geometry "<<Geometry.GetName()<<" failed!!"<<endl;
    return false;
  }  
 
  TH1D* SpectralHist = new TH1D("Spectrum", "Spectrum", 100, m_EnergyMin, m_EnergyMax);
  SpectralHist->SetXTitle("Energy in keV");
  SpectralHist->SetYTitle("counts");

  int PolBins = 90;
  
  TH1D* PolAzimuthHist = new TH1D("AzimuthalPol", "Azimuthal scatter angle distribution of polarized data", PolBins, -180.0, 180);
  PolAzimuthHist->SetXTitle("Azimuthal scatter angle in degree");
  PolAzimuthHist->SetYTitle("counts");
  PolAzimuthHist->SetMinimum(0);
  TH1D* NotPolAzimuthHist = new TH1D("AzimuthalNotPol", "Azimuthal scatter angle distribution of UN-polarized data", PolBins, -180.0, 180);
  NotPolAzimuthHist->SetXTitle("Azimuthal scatter angle in degree");
  NotPolAzimuthHist->SetYTitle("counts");
  NotPolAzimuthHist->SetMinimum(0);
  TH1D* CorrectedAzimuthHist = new TH1D("CorrectedPolarizationSignature", "Corrected polarization signature", PolBins, -180.0, 180);
  CorrectedAzimuthHist->SetXTitle("Azimuthal scatter angle in degree");
  CorrectedAzimuthHist->SetYTitle("corrected counts");
  CorrectedAzimuthHist->SetMinimum(0);

 
  MRERawEvent* Event = 0;

  // Open the polarized file
  MFileEventsEvta PolData(&Geometry);
  if (PolData.Open(m_PolFileName.Data()) == false) {
    mlog<<"Unable to open file "<<m_PolFileName<<endl;
    return false;
  }
  while ((Event = PolData.GetNextEvent()) != 0) {
    // Determine the energy deposit in CZT:
    int NHits = 0;
    double Energy = 0.0;
    MVector WeightedPos;
    for (int r = 0; r < Event->GetNRESEs(); ++r) {
      if (Event->GetRESEAt(r)->GetDetector() == 8) {
        ++NHits;
        Energy += Event->GetRESEAt(r)->GetEnergy();
        WeightedPos += Event->GetRESEAt(r)->GetPosition()*Event->GetRESEAt(r)->GetEnergy();
      }
    }
    if (NHits > 0) {
      if (Energy > m_EnergyMin && Energy < m_EnergyMax) {
        WeightedPos[0] /= NHits;
        WeightedPos[1] /= NHits;
        WeightedPos[2] /= NHits;
        double Azimuth = WeightedPos.Phi();
        SpectralHist->Fill(Energy);
        PolAzimuthHist->Fill(Azimuth*c_Deg);
      }
    }

    delete Event;
  }

  // Open the polarized file
  MFileEventsEvta NotPolData(&Geometry);
  if (NotPolData.Open(m_NotPolFileName.Data()) == false) {
    mlog<<"Unable to open file "<<m_NotPolFileName<<endl;
    return false;
  }
  while ((Event = NotPolData.GetNextEvent()) != 0) {
    // Determine the energy deposit in CZT:
    int NHits = 0;
    double Energy = 0.0;
    MVector WeightedPos;
    for (int r = 0; r < Event->GetNRESEs(); ++r) {
      if (Event->GetRESEAt(r)->GetDetector() == 8) {
        ++NHits;
        Energy += Event->GetRESEAt(r)->GetEnergy();
        WeightedPos += Event->GetRESEAt(r)->GetPosition()*Event->GetRESEAt(r)->GetEnergy();
      }
    }
    if (NHits > 0) {
      if (Energy > m_EnergyMin && Energy < m_EnergyMax) {
        WeightedPos[0] /= NHits;
        WeightedPos[1] /= NHits;
        WeightedPos[2] /= NHits;
        double Azimuth = WeightedPos.Phi();
        SpectralHist->Fill(Energy);
        NotPolAzimuthHist->Fill(Azimuth*c_Deg);
      }
    }

    delete Event;
  }

  // Normalize both to counts/deg:
  for (int b = 1; b <= PolAzimuthHist->GetNbinsX(); ++b) {
    if (PolAzimuthHist->GetBinContent(b) == 0) {
      mgui<<"You don't have enough statistics: Some bins are zero!"<<endl;
      return false;
    }
    PolAzimuthHist->SetBinContent(b, PolAzimuthHist->GetBinContent(b)/PolAzimuthHist->GetBinWidth(b));
  }
  for (int b = 1; b <= NotPolAzimuthHist->GetNbinsX(); ++b) {
    if (NotPolAzimuthHist->GetBinContent(b) == 0) {
      mgui<<"You don't have enough statistics: Some bins are zero!"<<endl;
      return false;
    }
    NotPolAzimuthHist->SetBinContent(b, NotPolAzimuthHist->GetBinContent(b)/NotPolAzimuthHist->GetBinWidth(b));
  }

  // Correct the image:
  double Mean = PolAzimuthHist->Integral()/CorrectedAzimuthHist->GetNbinsX();

  // The scaling is necessary, since we cannot assume that pol and background have been measured for exactly the same time...
  Mean *= NotPolAzimuthHist->Integral()/PolAzimuthHist->Integral();

  for (int b = 1; b <= CorrectedAzimuthHist->GetNbinsX(); ++b) {
    CorrectedAzimuthHist->SetBinContent(b, PolAzimuthHist->GetBinContent(b)/NotPolAzimuthHist->GetBinContent(b)*Mean);
  }

  /*
  TCanvas* SpectralCanvas = new TCanvas();
  SpectralCanvas->cd();
  SpectralHist->Draw();
  SpectralCanvas->Update();

  TCanvas* PolAzimuthCanvas = new TCanvas();
  PolAzimuthCanvas->cd();
  PolAzimuthHist->Draw();
  PolAzimuthCanvas->Update();

  TCanvas* NotPolAzimuthCanvas = new TCanvas();
  NotPolAzimuthCanvas->cd();
  NotPolAzimuthHist->Draw();
  NotPolAzimuthCanvas->Update();
  */

  // Try to fit a cosinus
  TCanvas* CorrectedAzimuthCanvas = new TCanvas();
  CorrectedAzimuthCanvas->cd();

  TF1* Lin = new TF1("LinearModulation", "pol0", -180*0.99, 180*0.99);
  CorrectedAzimuthHist->Fit(Lin, "RQFI");

  TF1* Mod = new TF1("Modulation", Modulation, -180*0.99, 180*0.99, 3);
  Mod->SetParNames("Offset (#)", "Scale (#)", "Shift (deg)");
  Mod->SetParameters(Lin->GetParameter(0), 0.5, 0);
  Mod->SetParLimits(1, 0, 10000000);
  Mod->SetParLimits(2, -200, 200);
  CorrectedAzimuthHist->Fit(Mod, "RQ");

  CorrectedAzimuthHist->Draw();
  CorrectedAzimuthCanvas->Update();

  double Modulation = fabs(Mod->GetParameter(1)/Mod->GetParameter(0));
  double ModulationError = sqrt((Mod->GetParError(1)*Mod->GetParError(1))/(Mod->GetParameter(0)*Mod->GetParameter(0)) +
                        (Mod->GetParError(0)*Mod->GetParError(0)*Mod->GetParameter(1)*Mod->GetParameter(1))/
                        (Mod->GetParameter(0)*Mod->GetParameter(0)*Mod->GetParameter(0)*Mod->GetParameter(0)));

  double PolarizationAngle = Mod->GetParameter(2);
  double PolarizationAngleError = Mod->GetParError(2);
  while (PolarizationAngle < 0) PolarizationAngle += 180;
  while (PolarizationAngle > 180) PolarizationAngle -= 180;

  mout<<endl;
  mout<<"Polarization analysis:"<<endl;
  mout<<endl;
  mout<<"Modulation:          "<<Modulation<<"+-"<<ModulationError<<endl;
  mout<<"Polarization angle: ("<<PolarizationAngle<<"+-"<<PolarizationAngleError<<") deg"<<endl;
  mout<<endl;
  
  CorrectedAzimuthHist->Draw();
  CorrectedAzimuthCanvas->Update();


  return true;
}


/******************************************************************************/

PolAnalyzer* g_Prg = 0;
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
  MGlobal::Initialize();

  TApplication PolAnalyzerApp("PolAnalyzerApp", 0, 0);

  g_Prg = new PolAnalyzer();

  if (g_Prg->ParseCommandLine(argc, argv) == false) {
    cerr<<"Error during parsing of command line!"<<endl;
    return -1;
  } 
  if (g_Prg->Analyze() == false) {
    cerr<<"Error during analysis!"<<endl;
    return -2;
  } 

  PolAnalyzerApp.Run();

  cout<<"Program exited normally!"<<endl;

  return 0;
}

/*
 * Cosima: the end...
 ******************************************************************************/
