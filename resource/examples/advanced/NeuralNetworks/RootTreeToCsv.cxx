/* 
 * RootTreeToCsv.cxx
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
#include <fstream>
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
#include <TBranch.h>
#include <TTree.h>
#include <TFile.h>

// MEGAlib
#include "MGlobal.h"


////////////////////////////////////////////////////////////////////////////////


//! A standalone program based on MEGAlib and ROOT
class RootTreeToCsv
{
public:
  //! Default constructor
  RootTreeToCsv();
  //! Default destructor
  ~RootTreeToCsv();
  
  //! Parse the command line
  bool ParseCommandLine(int argc, char** argv);
  //! Analyze what eveer needs to be analyzed...
  bool Analyze();
  //! Interrupt the analysis
  void Interrupt() { m_Interrupt = true; }

private:
  //! True, if the analysis needs to be interrupted
  bool m_Interrupt;
  //! The file name
  MString m_FileName;
};


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
RootTreeToCsv::RootTreeToCsv() : m_Interrupt(false)
{
  gStyle->SetPalette(1, 0);
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
RootTreeToCsv::~RootTreeToCsv()
{
  // Intentionally left blank
}


////////////////////////////////////////////////////////////////////////////////


//! Parse the command line
bool RootTreeToCsv::ParseCommandLine(int argc, char** argv)
{
  ostringstream Usage;
  Usage<<endl;
  Usage<<"  Usage: RootTreeToCsv <options>"<<endl;
  Usage<<"    General options:"<<endl;
  Usage<<"         -f:   file name"<<endl;
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
    if (Option == "-f") {
      m_FileName = argv[++i];
      cout<<"Accepting file name: "<<m_FileName<<endl;
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
bool RootTreeToCsv::Analyze()
{
  if (m_Interrupt == true) return false;

  TFile* SourceFile = new TFile(m_FileName);
  
  TTree* SourceTree = (TTree*) SourceFile->Get("Good");
  if (SourceTree == nullptr) {
    SourceTree = (TTree*) SourceFile->Get("Bad");
    if (SourceTree == nullptr) {
      cout<<"Error reading events tree"<<endl;
      return false;
    }
  }  
  TObjArray* Branches = SourceTree->GetListOfBranches();

  
  MString OutFileName = m_FileName;
  OutFileName.ReplaceAtEndInPlace(".root", ".txt");
  ofstream out;
  out.open(OutFileName);
  
  
  vector<TString> IgnoredBranches = {}; // { "SimulationIDs" };
  
  Long64_t ID;
  vector<double> Data;
  Data.reserve(Branches->GetEntries());

  for (int b = 0; b < Branches->GetEntries(); ++b) {
    TBranch* B = dynamic_cast<TBranch*>(Branches->At(b));

    if (B->GetName() == TString("SimulationIDs")) {
      SourceTree->SetBranchAddress(B->GetName(), &ID);
      out<<B->GetName()<<" ";
    } else {
      if (find(IgnoredBranches.begin(), IgnoredBranches.end(), B->GetName()) == IgnoredBranches.end()) {
        out<<B->GetName()<<" ";
        double a;
        Data.push_back(a);
        SourceTree->SetBranchAddress(B->GetName(), &Data.back());
      }
    }
  }
  out<<endl;
  
  for (int i = 0; i < SourceTree->GetEntries(); ++i) {
    SourceTree->GetEntry(i);
    out<<ID<<" ";
    for (unsigned int d = 0; d < Data.size(); ++d) {
      double Value = Data[d];
      if (fabs(Value) < 1E-315) Value = 0;
      out<<Value<<" ";
    }
    out<<endl;
  }
  
  out.close();
  
  SourceFile->Close();
  
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


RootTreeToCsv* g_Prg = 0;
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

  TApplication RootTreeToCsvApp("RootTreeToCsvApp", 0, 0);

  g_Prg = new RootTreeToCsv();

  if (g_Prg->ParseCommandLine(argc, argv) == false) {
    cerr<<"Error during parsing of command line!"<<endl;
    return -1;
  } 
  if (g_Prg->Analyze() == false) {
    cerr<<"Error during analysis!"<<endl;
    return -2;
  } 

  //RootTreeToCsvApp.Run();

  cout<<"Program exited normally!"<<endl;

  return 0;
}


////////////////////////////////////////////////////////////////////////////////
