/* 
 * ShowHistograms.cxx
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
#include <TString.h>
#include <TFile.h>
#include <TKey.h>
#include <TIterator.h>
#include <TList.h>
#include <TClass.h>

// MEGAlib
#include "MGlobal.h"


/******************************************************************************/

class ShowHistograms
{
public:
  /// Default constructor
  ShowHistograms();
  /// Default destructor
  ~ShowHistograms();
  
  /// Parse the command line
  bool ParseCommandLine(int argc, char** argv);
  /// Analyze what eveer needs to be analyzed...
  bool Analyze();
  /// Interrupt the analysis
  void Interrupt() { m_Interrupt = true; }

private:
  /// True, if the analysis needs to be interrupted
  bool m_Interrupt;
  /// File name
  MString m_FileName;
};

/******************************************************************************/


/******************************************************************************
 * Default constructor
 */
ShowHistograms::ShowHistograms() : m_Interrupt(false)
{
  gStyle->SetPalette(1, 0);
}


/******************************************************************************
 * Default destructor
 */
ShowHistograms::~ShowHistograms()
{
  // Intentionally left blanck
}


/******************************************************************************
 * Parse the command line
 */
bool ShowHistograms::ParseCommandLine(int argc, char** argv)
{
  ostringstream Usage;
  Usage<<endl;
  Usage<<"  Usage: ShowHistograms <options>"<<endl;
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

  if (m_FileName.IsEmpty() == true || m_FileName.EndsWith(".root") == false) {
    cout<<"Error: Cannot show a histrogram if you don't give a *.root file!"<<endl;
    cout<<Usage.str()<<endl;
    return false;    
  }

  return true;
}


/******************************************************************************
 * Do whatever analysis is necessary
 */
bool ShowHistograms::Analyze()
{
  if (m_Interrupt == true) return false;
  
  TFile* F = new TFile(m_FileName);
  
  TIter I(F->GetListOfKeys());
  TKey *Key;

  TCanvas* NewCanvas = 0;
  bool First = true;
  while ((Key = (TKey*) I()) != 0) {
    TClass *Class = gROOT->GetClass(Key->GetClassName());
    if (Class->InheritsFrom("TH1")) {
      TH1* H = (TH1*) Key->ReadObj();
      if (NewCanvas == 0) NewCanvas = new TCanvas();
      NewCanvas->cd();
      MString DrawOption;
      if (Class->InheritsFrom("TH2")) {
        DrawOption += "COLZ";
      }
      if (First == true) {
        First = false;
      } else {
        DrawOption += " SAME"; 
      }
      H->Draw(DrawOption);
      NewCanvas->Update();
    } else if (Class->InheritsFrom("TCanvas")) {
      TCanvas* C = (TCanvas*) Key->ReadObj();
      C->Draw();
      /*
      TIter CanvasContent(C->GetListOfPrimitives());
      TObject* Object;
      while ((Object = CanvasContent()) != 0) {
        if (Object->InheritsFrom("TH1")) {
          if (NewCanvas == 0) NewCanvas = new TCanvas();
          NewCanvas->cd();
          MString DrawOption("");
          if (Object->InheritsFrom("TH2")) {
            DrawOption += "colz";
          } 
          if (First == true) {
            First = false;
          } else {
            DrawOption += " SAME"; 
          }
          dynamic_cast<TH1*>(Object)->Draw(DrawOption);
          NewCanvas->Update();
        }
      }
      */
    }
  }
  
  return true;
}


/******************************************************************************/

ShowHistograms* g_Prg = 0;
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
  //void (*handler)(int);
  //handler = CatchSignal;
  //(void) signal(SIGINT, CatchSignal);

  // Initialize global MEGALIB variables, especially mgui, etc.
  MGlobal::Initialize("ShowHistograms", "extract and show histogram from root files");

  TApplication ShowHistogramsApp("ShowHistogramsApp", 0, 0);

  g_Prg = new ShowHistograms();

  if (g_Prg->ParseCommandLine(argc, argv) == false) {
    cerr<<"Error during parsing of command line!"<<endl;
    return -1;
  } 
  if (g_Prg->Analyze() == false) {
    cerr<<"Error during analysis!"<<endl;
    return -2;
  } 

  ShowHistogramsApp.Run();

  cout<<"Program exited normally!"<<endl;

  return 0;
}

/*
 * ShowHistograms: the end...
 ******************************************************************************/
