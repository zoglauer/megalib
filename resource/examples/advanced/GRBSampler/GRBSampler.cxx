/* 
 * GRBSampler.cxx
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
#include <TRandom.h>

// MEGAlib
#include "MGlobal.h"
#include "MParser.h"
#include "MTokenizer.h"


////////////////////////////////////////////////////////////////////////////////


//! A standalone program based on MEGAlib and ROOT
class GRBSampler
{
public:
  //! Default constructor
  GRBSampler();
  //! Default destructor
  ~GRBSampler();
  
  //! Parse the command line
  bool ParseCommandLine(int argc, char** argv);
  //! Analyze what eveer needs to be analyzed...
  bool Analyze();
  //! Interrupt the analysis
  void Interrupt() { m_Interrupt = true; }

private:
  //! True, if the analysis needs to be interrupted
  bool m_Interrupt;
  //! Number of samples
  unsigned int m_NumberOfSamples;
};


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
GRBSampler::GRBSampler() : m_Interrupt(false), m_NumberOfSamples(10)
{
  gStyle->SetPalette(1, 0);
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
GRBSampler::~GRBSampler()
{
  // Intentionally left blank
}


////////////////////////////////////////////////////////////////////////////////


//! Parse the command line
bool GRBSampler::ParseCommandLine(int argc, char** argv)
{
  ostringstream Usage;
  Usage<<endl;
  Usage<<"  Usage: GRBSampler <options>"<<endl;
  Usage<<"    General options:"<<endl;
  Usage<<"         -s:   number of samples to create"<<endl;
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
    if (Option == "-s") {
      m_NumberOfSamples = atoi(argv[++i]);
      cout<<"Samples: "<<m_NumberOfSamples<<endl;
    } else {
      cout<<"Error: Unknown option \""<<Option<<"\"!"<<endl;
      cout<<Usage.str()<<endl;
      return false;
    }
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Do whatever analysis is necessary
bool GRBSampler::Analyze()
{
  int Smooths = 0;
  double TimeCutOff = 2;
  
  MString FileName = "BATSE.csv";
  
  MParser Parser(',', false);
  if (Parser.Open(FileName) == false) {
    cout<<"Error: Unable to open file "<<FileName<<endl;
    return false;
  }
  
  TH1D* T90 = new TH1D("T90", "T90", 1000, 0, 200);
  
  TH1D* sBandAlpha = new TH1D("sBandAlpha", "sBandAlpha", 60, -3, 3);
  TH1D* sBandBeta = new TH1D("sBandBeta", "sBandBeta", 90, -9, 0);
  TH1D* sBandEPeak = new TH1D("sBandEPeak", "sBandEPeak", 100, 0, 2000);
  TH1D* sFlux = new TH1D("sFlux", "sFlux", 1000, 0, 100);
  
  TH1D* lBandAlpha = new TH1D("lBandAlpha", "lBandAlpha", 60, -3, 3);
  lBandAlpha->SetLineColor(kGreen);
  TH1D* lBandBeta = new TH1D("lBandBeta", "lBandBeta", 90, -9, 0);
  lBandBeta->SetLineColor(kGreen);
  TH1D* lBandEPeak = new TH1D("lBandEPeak", "lBandEPeak", 100, 0, 2000);
  lBandEPeak->SetLineColor(kGreen);
  TH1D* lFlux = new TH1D("lFlux", "lFlux", 1000, 0, 100);
  lFlux->SetLineColor(kGreen);
  
  unsigned int NShorts = 0;
  unsigned int NLongs = 0;
  for (unsigned int l = 0; l < Parser.GetNLines(); ++l) {
    MTokenizer* T = Parser.GetTokenizerAt(l);
    if (T->GetNTokens() < 7) continue;
   
    T90->Fill(T->GetTokenAtAsDouble(2));
    
    if (T->GetTokenAtAsDouble(2) < TimeCutOff) {
      NShorts++;
      sBandAlpha->Fill(T->GetTokenAtAsDouble(3));
      sBandBeta->Fill(T->GetTokenAtAsDouble(4));
      sBandEPeak->Fill(T->GetTokenAtAsDouble(5));
      sFlux->Fill(T->GetTokenAtAsDouble(6));
    } else {
      NLongs++;
      lBandAlpha->Fill(T->GetTokenAtAsDouble(3));
      lBandBeta->Fill(T->GetTokenAtAsDouble(4));
      lBandEPeak->Fill(T->GetTokenAtAsDouble(5));
      lFlux->Fill(T->GetTokenAtAsDouble(6));
    }
  }
  cout<<"Number of short GRBS: "<<NShorts<<" vs long GRBs: "<<NLongs<<endl;
  
  TCanvas* cT90 = new TCanvas("cT90", "cT90", 1200, 800);
  cT90->cd();
  T90->Draw();
  cT90->Update();

  
  TCanvas* cBandAlpha = new TCanvas("cBandAlpha", "cBandAlpha", 1200, 800);
  cBandAlpha->cd();
  lBandAlpha->Smooth(Smooths);
  sBandAlpha->Smooth(Smooths);
  lBandAlpha->Draw();
  sBandAlpha->Draw("SAME");
  cBandAlpha->Update();
  
  TCanvas* cBandBeta = new TCanvas("cBandBeta", "cBandBeta", 1200, 800);
  cBandBeta->cd();
  lBandBeta->Smooth(Smooths);
  sBandBeta->Smooth(Smooths);
  lBandBeta->Draw();
  sBandBeta->Draw("SAME");
  cBandBeta->Update();
  
  TCanvas* cBandEPeak = new TCanvas("cBandEPeak", "cBandEPeak", 1200, 800);
  cBandEPeak->cd();
  lBandEPeak->Smooth(Smooths);
  sBandEPeak->Smooth(Smooths);
  lBandEPeak->Draw();
  sBandEPeak->Draw("SAME");
  cBandEPeak->Update();
  
  TCanvas* cFlux = new TCanvas("cFlux", "cFlux", 1200, 800);
  cFlux->cd();
  cFlux->SetLogx();
  lFlux->Smooth(Smooths);
  sFlux->Smooth(Smooths);
  lFlux->Draw();
  sFlux->Draw("SAME");
  cFlux->Update();
  
  
  // Now sample
  for (unsigned int s = 1; s <= m_NumberOfSamples; ++s) {
    // Sample for the distributions
    double Duration = T90->GetRandom();
    
    double Alpha = 0;
    double Beta = 0;
    do {
      Alpha = Duration > TimeCutOff ? lBandAlpha->GetRandom() : sBandAlpha->GetRandom();
      Beta = Duration > TimeCutOff ? lBandBeta->GetRandom() : sBandBeta->GetRandom();
    } while (Alpha < Beta);
    
    double Peak = Duration > TimeCutOff ? lBandEPeak->GetRandom() : sBandEPeak->GetRandom();
    double Flux = Duration > TimeCutOff ? lFlux->GetRandom() : sFlux->GetRandom();
    
    // Sample a random start position - ignoring Earth which starts at 112.5 deg
    double Theta = acos(cos(0) - gRandom->Rndm()*(cos(0) - cos(112.5*c_Rad))) * c_Deg;
    double Phi = 2*c_Pi*gRandom->Rndm() * c_Deg;
    
    ofstream out;
    out.open(MString("GRB_") + s + ".source");
    
    out<<"# GRB "<<s<<endl;
    out<<endl;
    out<<"Include Common.partialsource"<<endl;
    out<<endl;
    out<<"Run GRBSim"<<endl;
    out<<"GRBSim.FileName             GRB_"<<s<<endl;
    out<<"GRBSim.Time                 "<<Duration<<endl;
    out<<endl;
    out<<"GRBSim.Source GRB"<<endl;
    out<<"GRB.ParticleType           1"<<endl;
    out<<"GRB.Beam                   FarFieldPointSource "<<Theta<<" "<<Phi<<endl;
    out<<"GRB.Spectrum               Band 20 2000 "<<Alpha<<" "<<Beta<<" "<<Peak<<endl;
    out<<"# Reminder the BATSE flux is in the 20 keV to 2 MeV band"<<endl;
    out<<"GRB.Flux                   "<<Flux<<endl;
    out<<endl;
    out<<"Include Background.partialsource"<<endl;
    out<<endl;
    
    
    out.close();
  }
  
  ofstream common;
  common.open("Common.partialsource");
  common<<"# Global parameters"<<endl;
  common<<"Version         1"<<endl;
  common<<"Geometry        $(MEGALIB)/resource/examples/geomega/GRIPS/GRIPS.geo.setup"<<endl;
  common<<endl;
  common<<"# Physics list"<<endl;
  common<<"PhysicsListEM               LivermorePol"<<endl;
  common<<endl;
  common<<"# Output formats"<<endl;
  common<<"StoreSimulationInfo         all"<<endl;
  common<<endl;
  common.close();
  
  
  ofstream bkg;
  bkg.open("Background.partialsource");
  bkg<<"# Add your background here. The example is for equatorial orbit - photon components only"<<endl;
  bkg<<"SpaceSim.Source CosmicPhoton"<<endl;
  bkg<<"CosmicPhoton.ParticleType   1"<<endl;
  bkg<<"CosmicPhoton.Beam           FarFieldFileZenithDependent CosmicPhotonsGruber.beam.dat"<<endl;
  bkg<<"CosmicPhoton.Spectrum       File CosmicPhotonsGruber.spectrum.dat"<<endl;
  bkg<<"CosmicPhoton.Flux           14.916"<<endl;
  bkg<<endl;
  bkg<<"SpaceSim.Source AlbedoPhoton"<<endl;
  bkg<<"AlbedoPhoton.ParticleType   1"<<endl;
  bkg<<"AlbedoPhoton.Beam           FarFieldFileZenithDependent AlbedoPhotonsTuerlerMizunoAbdo.beam.dat"<<endl;
  bkg<<"AlbedoPhoton.Spectrum       File AlbedoPhotonsTuerlerMizunoAbdo.spectrum.dat"<<endl;
  bkg<<"AlbedoPhoton.Flux           2.4739"<<endl;
  bkg<<endl;
  bkg.close();
  
  cout<<"DONE!"<<endl;
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


GRBSampler* g_Prg = 0;
int g_NInterruptCatches = 1;


////////////////////////////////////////////////////////////////////////////////


//! Called when an interrupt signal is flagged
//! All catched signals lead to a well defined exit of the program
void CatchSignal(int a)
{
  if (g_Prg != 0 && g_NInterruptCatches-- > 0) {
    cout<<"Catched signal Ctrl-C (ID="<<a<<"):"<<endl;
    g_Prg->Interrupt();
  } else {
    abort();
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Main program
int main(int argc, char** argv)
{
  // Catch a user interupt for graceful shutdown
  // signal(SIGINT, CatchSignal);

  // Initialize global MEGALIB variables, especially mgui, etc.
  MGlobal::Initialize("Standalone", "a standalone example program");

  TApplication GRBSamplerApp("GRBSamplerApp", 0, 0);

  g_Prg = new GRBSampler();

  if (g_Prg->ParseCommandLine(argc, argv) == false) {
    cerr<<"Error during parsing of command line!"<<endl;
    return -1;
  } 
  if (g_Prg->Analyze() == false) {
    cerr<<"Error during analysis!"<<endl;
    return -2;
  } 

  GRBSamplerApp.Run();

  cout<<"Program exited normally!"<<endl;

  return 0;
}


////////////////////////////////////////////////////////////////////////////////
