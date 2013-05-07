/* 
 * HistogramComparer.cxx
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
#include <TRegexp.h>
#include <TSystem.h>
#include <TFile.h>
#include <TKey.h>
#include <TClass.h>
#include <TMath.h>
#include <Math/GoFTest.h>

// MEGAlib
#include "MGlobal.h"


/******************************************************************************/

class HistogramComparer
{
public:
  /// Default constructor
  HistogramComparer();
  /// Default destructor
  ~HistogramComparer();
  
  /// Parse the command line
  bool ParseCommandLine(int argc, char** argv);
  /// Analyze what eveer needs to be analyzed...
  bool Analyze();
  /// Interrupt the analysis
  void Interrupt() { m_Interrupt = true; }

private:
  /// True, if the analysis needs to be interrupted
  bool m_Interrupt;
};

/******************************************************************************/


/******************************************************************************
 * Default constructor
 */
HistogramComparer::HistogramComparer() : m_Interrupt(false)
{
  gStyle->SetPalette(1, 0);
}


/******************************************************************************
 * Default destructor
 */
HistogramComparer::~HistogramComparer()
{
  // Intentionally left blank
}


/******************************************************************************
 * Parse the command line
 */
bool HistogramComparer::ParseCommandLine(int argc, char** argv)
{
  ostringstream Usage;
  Usage<<endl;
  Usage<<"  Usage: HistogramComparer <options>"<<endl;
  Usage<<"    General options:"<<endl;
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
      //m_FileName = argv[++i];
      //cout<<"Accepting file name: "<<m_FileName<<endl;
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
bool HistogramComparer::Analyze()
{
  if (m_Interrupt == true) return false;

  // Find all root files
  vector<MString> FileNames;
  
  char base_dir[]="."; // directory to get listing; here $ROOTSYS
  MString basename("*.root");        // file name pattern; here: all files
  const char *file;
   
  // expanding environment variable in the directory name, if any
  void *dir = gSystem->OpenDirectory(gSystem->ExpandPathName(base_dir));

  if (dir) {
    TRegexp re(basename,kTRUE);
    while ((file = gSystem->GetDirEntry(dir))) {
      // skipping "." and ".." to avoid infinite loop
      if (!strcmp(file,".") || !strcmp(file,"..")) continue;
      TString s = file;
      // comparing the file name with pattern
      if ( (basename!=file) && s.Index(re) == kNPOS) continue;
      // printing out the file name
      cout << file << endl;
      FileNames.push_back(file);
      // How to find out whether <file> is an ordinary file or directory???
    }
  }
  
  while (FileNames.size() > 0) {
    vector<MString> ToCompare;
    ToCompare.push_back(FileNames[0]);
    FileNames.erase(FileNames.begin());
    MString Begins = ToCompare[0];
    Begins.Remove(Begins.Index('_', Begins.First('_')+1), Begins.Length() - Begins.Index('_', Begins.First('_')+1));
    //cout<<"Begins with: "<<Begins<<endl;
    for (vector<MString>::iterator I = FileNames.begin(); I != FileNames.end(); ) {
      if ((*I).BeginsWith(Begins) == true) {
        ToCompare.push_back(*I);
        I = FileNames.erase(I);
      } else {
        ++I;
      }
    }
    
    if (ToCompare.size() == 1) {
      cout<<"Only one file to compare: "<<ToCompare[0]<<endl;
      continue;
    }
    /*
    cout<<"Comparing: ";
    for (unsigned int c = 0; c < ToCompare.size(); ++c) {
      cout<<ToCompare[c]<<" ";
    }
    cout<<endl;
    */
    vector<TH1*> Histograms;
    for (unsigned int c = 0; c < ToCompare.size(); ++c) {
      TFile* F = new TFile(ToCompare[c]);
  
      TIter I(F->GetListOfKeys());
      TKey *Key;

      while ((Key = (TKey*) I()) != 0) {
        TClass *Class = gROOT->GetClass(Key->GetClassName());
        if (Class->InheritsFrom("TH1D")) {
          TH1D* H = (TH1D*) Key->ReadObj();
          Histograms.push_back(new TH1D(*H));
        } else if (Class->InheritsFrom("TH2D")) {
          TH2D* H = (TH2D*) Key->ReadObj();
          Histograms.push_back(new TH2D(*H));
        }  
      }
      //F->Close();
    }
    
    for (unsigned int h1 = 0; h1 < Histograms.size(); ++h1) {
      /*
      TCanvas* C = new TCanvas();
      C->cd();
      Histograms[h1]->Draw();
      C->Update();
      */
      for (int b = 1; b <= Histograms[h1]->GetNbinsX(); ++b) {
        Histograms[h1]->SetBinError(b, sqrt(Histograms[h1]->GetBinContent(b)));
      }
    }
    
    // Compare:
    cout<<"Comparing "<<Begins<<endl;
    for (unsigned int h1 = 0; h1 < Histograms.size(); ++h1) {
      for (unsigned int h2 = h1+1; h2 < Histograms.size(); ++h2) {
        //cout<<Histograms[h1]->GetSize()<<" vs. "<<Histograms[h2]->GetSize()<<endl;
        //cout<<Histograms[h1]->GetArray()[500]<<" vs. "<<Histograms[h2]->GetArray()[500]<<endl;
        //ROOT::Math::GoFTest* T = new ROOT::Math::GoFTest(Histograms[h1]->GetSize(), Histograms[h1]->GetArray(), Histograms[h2]->GetSize(), Histograms[h2]->GetArray());
        //cout<<"Comparing "<<h1<<" with "<<h2<<": AD="<<T->AndersonDarling2SamplesTest("t")<<", KS="<<T->KolmogorovSmirnov2SamplesTest("t")<<endl;     
        cout<<"--> KS="<<Histograms[h1]->KolmogorovTest(Histograms[h2])<<"    ("<<ToCompare[h1]<<" & "<<ToCompare[h2]<<")"<<endl;
      }
    }
    
  }
  
  
  return true;
}


/******************************************************************************/

HistogramComparer* g_Prg = 0;
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
  MGlobal::Initialize("HistogramComparer", "a HistogramComparer example program");

  TApplication HistogramComparerApp("HistogramComparerApp", 0, 0);

  g_Prg = new HistogramComparer();

  if (g_Prg->ParseCommandLine(argc, argv) == false) {
    cerr<<"Error during parsing of command line!"<<endl;
    return -1;
  } 
  if (g_Prg->Analyze() == false) {
    cerr<<"Error during analysis!"<<endl;
    return -2;
  } 

  HistogramComparerApp.Run();

  cout<<"Program exited normally!"<<endl;

  return 0;
}

/*
 * Cosima: the end...
 ******************************************************************************/
