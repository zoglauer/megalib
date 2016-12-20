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
#include <fstream>
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
#include "MDGeometryQuest.h"


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
  //! The geometry file name
  MString m_GeometryFileName;
  //! The test position where to calculate the atmosphere absorptions
  MVector m_TestPoint;
};


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
AtmosphereModel::AtmosphereModel() : m_Interrupt(false)
{
  gStyle->SetPalette(1, 0);
  
  m_TestPoint.SetXYZ(0, 0, 3500000);
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
    if (Option == "-g") {
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


////////////////////////////////////////////////////////////////////////////////


//! Do whatever analysis is necessary
bool AtmosphereModel::Analyze()
{
  if (m_Interrupt == true) return false;

  gStyle->SetPalette(kBird);
  
  // Load geometry:
  MDGeometryQuest* Geometry = new MDGeometryQuest();
  if (Geometry->ScanSetupFile(m_GeometryFileName) == true) {
    cout<<"Geometry "<<Geometry->GetName()<<" loaded!"<<endl;
  } else {
    cout<<"Unable to load geometry "<<Geometry->GetName()<<" - Aborting!"<<endl;
    return false;
  }  
 
  vector<double> Energies = { 511, 1809, 1157, 1333, 100, 130, 200, 350, 550, 700, 1000, 1300, 2000, 3500, 5000 };
  vector<double> Angles = { 0, 10, 20, 30, 40, 50, 60, 70, 80, 90 };
 
  ofstream fout;
  fout.open("LookupTable.txt");
  fout<<"# Create lookup table"<<endl;
  fout<<"declare -A Table"<<endl;
  for (double Energy: Energies) {
    fout<<endl;
    for (double Angle: Angles) {
      MVector Dir;
      Dir.SetMagThetaPhi(20000000, Angle*c_Rad, 0.0);
      
      fout<<"Table["<<Energy<<"_"<<Angle<<"]="<<1-Geometry->GetAbsorptionProbability(m_TestPoint, m_TestPoint + Dir, Energy)<<endl;
    }
  }
  fout<<endl;
  fout.close();
 
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

  //AtmosphereModelApp.Run();

  cout<<"Program exited normally!"<<endl;

  return 0;
}


////////////////////////////////////////////////////////////////////////////////
