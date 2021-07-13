/* 
 * AtmosphereModel.cxx
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
#include "MInterface.h"
#include "MResponseMatrixO1.h"

////////////////////////////////////////////////////////////////////////////////


//! A standalone program based on MEGAlib and ROOT
class AtmosphereModel
{
public:
  //! Default constructor
  AtmosphereModel();
  //! Default destructor
  ~AtmosphereModel();
  
  //! Parse the command line
  bool ParseCommandLine(int argc, char** argv);
  //! Analyze what eveer needs to be analyzed...
  bool Analyze();
  //! Interrupt the analysis
  void Interrupt() { m_Interrupt = true; }

private:
  //! True, if the analysis needs to be interrupted
  bool m_Interrupt;
  //! Cross-section file name
  MString m_CrossSectionFileName;
};


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
AtmosphereModel::AtmosphereModel() : m_Interrupt(false)
{
  gStyle->SetPalette(1, 0);
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
AtmosphereModel::~AtmosphereModel()
{
  // Intentionally left blank
}


////////////////////////////////////////////////////////////////////////////////


//! Parse the command line
bool AtmosphereModel::ParseCommandLine(int argc, char** argv)
{
  ostringstream Usage;
  Usage<<endl;
  Usage<<"  Usage: AtmosphereModel <options>"<<endl;
  Usage<<"    General options:"<<endl;
  Usage<<"         -c:   crosssection file name"<<endl;
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
    if (Option == "-c") {
      m_CrossSectionFileName = argv[++i];
      cout<<"Accepting cross section file name: "<<m_CrossSectionFileName<<endl;
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
bool AtmosphereModel::Analyze()
{
  if (m_Interrupt == true) return false;

  gStyle->SetPalette(kBird);
  
  MResponseMatrixO1 CrossSection;
  if (CrossSection.Read(m_CrossSectionFileName) == false) {
    cout<<"Unable to read cross section"<<endl;
    return false;
  }
  
  int eBins = 500;
  double* eAxis = MInterface::CreateAxisBins(20, 2000, eBins, true);
  
  int aBins = 1000;
  double* aAxis = MInterface::CreateAxisBins(0, 90, aBins, false);
  
  TH2D* TransmissionProbability = new TH2D("TransmissionProbability", "Unscattered Transmission Probability", eBins, eAxis, aBins, aAxis);
  TransmissionProbability->SetXTitle("Energy [keV]");
  TransmissionProbability->SetYTitle("Zenith angle [deg]");
  TransmissionProbability->SetZTitle("Unscattered Transmission Probability");
  
  for (int be = 1; be <= TransmissionProbability->GetXaxis()->GetNbins(); ++be) {
    for (int ba = 1; ba <= TransmissionProbability->GetYaxis()->GetNbins(); ++ba) {
      double ProbabilityOnAxis = exp(-CrossSection.GetInterpolated(TransmissionProbability->GetXaxis()->GetBinCenter(be)) * 1.0); // "* 1.0" == times 1 cm length
      double ProbabilityIncidenceAngle = ProbabilityOnAxis * cos(TransmissionProbability->GetYaxis()->GetBinCenter(ba) * c_Rad);
      
      TransmissionProbability->SetBinContent(be, ba, ProbabilityIncidenceAngle);
    }
  }
  
  double Max = TransmissionProbability->GetMaximum();
  int iMax = int(10*Max) + 1;
  TransmissionProbability->SetContour(2*iMax);
  TransmissionProbability->SetMaximum(double(iMax)/10);
  
  TCanvas* C = new TCanvas("TransmissionProbabilityCanvas", "TransmissionProbabilityCanvas", 1600, 1000);
  C->cd();
  C->SetGridx();
  C->SetGridy();
  C->SetLogx();
  TransmissionProbability->Draw("colz");
  C->Update();
  
  delete [] eAxis;
  delete [] aAxis;
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


AtmosphereModel* g_Prg = 0;
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

  TApplication AtmosphereModelApp("AtmosphereModelApp", 0, 0);

  g_Prg = new AtmosphereModel();

  if (g_Prg->ParseCommandLine(argc, argv) == false) {
    cerr<<"Error during parsing of command line!"<<endl;
    return -1;
  } 
  if (g_Prg->Analyze() == false) {
    cerr<<"Error during analysis!"<<endl;
    return -2;
  } 

  AtmosphereModelApp.Run();

  cout<<"Program exited normally!"<<endl;

  return 0;
}


////////////////////////////////////////////////////////////////////////////////
