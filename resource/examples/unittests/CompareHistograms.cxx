/* 
 * CompareHistograms.cxx
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
#include <algorithm>
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
#include <TLegend.h>
#include <TList.h>
#include <TClass.h>

// MEGAlib
#include "MGlobal.h"
#include "MFile.h"


/******************************************************************************/

class CompareHistograms;

CompareHistograms* g_Prg = 0;
int g_NInterruptCatches = 1;
bool g_IsInteractive = true;
int g_ReturnCode = 0;


/******************************************************************************/

class CompareHistograms
{
public:
  /// Default constructor
  CompareHistograms();
  /// Default destructor
  ~CompareHistograms();
  
  /// Parse the command line
  bool ParseCommandLine(int argc, char** argv);
  /// Analyze what eveer needs to be analyzed...
  bool Analyze();
  /// Interrupt the analysis
  void Interrupt() { m_Interrupt = true; }

  /// Return the histogram
  TH1* GetHistogram(MString FileName);
  
private:
  /// True, if the analysis needs to be interrupted
  bool m_Interrupt;
  /// File name of the model
  MString m_ModelFileName;
  /// File name of the simulation
  vector<MString> m_SimulationFileNames;
  /// File name of the output file
  MString m_OutputFileName;
};

/******************************************************************************/


/******************************************************************************
 * Default constructor
 */
CompareHistograms::CompareHistograms() : m_Interrupt(false)
{
  gStyle->SetPalette(1, 0);
}


/******************************************************************************
 * Default destructor
 */
CompareHistograms::~CompareHistograms()
{
  // Intentionally left blanck
}


/******************************************************************************
 * Parse the command line
 */
bool CompareHistograms::ParseCommandLine(int argc, char** argv)
{
  ostringstream Usage;
  Usage<<endl;
  Usage<<"  Usage: CompareHistograms <options>"<<endl;
  Usage<<"    General options:"<<endl;
  Usage<<"         -m:   model root file name"<<endl;
  Usage<<"         -s:   simulation root file name"<<endl;
  Usage<<"         -o:   output root file name"<<endl;
  Usage<<"         -b:   batch mode"<<endl;
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
    if (Option == "-m") {
      m_ModelFileName = argv[++i];
      //cout<<"Accepting model file name: "<<m_ModelFileName<<endl;
    } else if (Option == "-s") {
      m_SimulationFileNames.push_back(argv[++i]);
      //cout<<"Accepting simulation file name: "<<m_SimulationFileName<<endl;
    } else if (Option == "-o") {
      m_OutputFileName = argv[++i];
      //cout<<"Accepting simulation file name: "<<m_SimulationFileName<<endl;
    } else if (Option == "-b") {
      gROOT->SetBatch(true);
      g_IsInteractive = false;
      //cout<<"Accepting simulation file name: "<<m_SimulationFileName<<endl;
    } else {
      cout<<"Error: Unknown option \""<<Option<<"\"!"<<endl;
      cout<<Usage.str()<<endl;
      return false;
    }
  }

  if (m_ModelFileName.IsEmpty() == true || m_ModelFileName.EndsWith(".root") == false || MFile::Exists(m_ModelFileName) == false) {
    cout<<"Error: Cannot show a histrogram if you don't give a *.root file!"<<endl;
    cout<<Usage.str()<<endl;
    return false;    
  }
  if (m_SimulationFileNames.size() == 0) {
    cout<<"Error: Cannot show a histrogram if you don't give a *.root file!"<<endl;
    cout<<Usage.str()<<endl;
    return false;    
  }
  for (MString S: m_SimulationFileNames) {
    if (S.EndsWith(".root") == false || MFile::Exists(S) == false) {
      cout<<"Error: Cannot show a histrogram if you don't give a *.root file!"<<endl;
      cout<<Usage.str()<<endl;
      return false;
    }
  }

  return true;
}


/******************************************************************************
 * Do whatever analysis is necessary
 */
bool CompareHistograms::Analyze()
{
  TH1* Model = GetHistogram(m_ModelFileName);
  Model->SetLineColor(kBlack);
  Model->SetFillStyle(0);
  if (Model == 0) {
    cout<<"Error: Cannot show a histrogram if you don't give a *.root file!"<<endl;
    return false;    
  }
  
  vector<int> Colors = { kRed, kBlue, kGreen + 2, kMagenta, kCyan + 2, kYellow + 2 };
  vector<TH1*> Simulations;
  for (unsigned int s = 0; s < m_SimulationFileNames.size() && s < Colors.size(); ++s) {
    TH1* Simulation = GetHistogram(m_SimulationFileNames[s]); 
    Simulation->SetLineColor(Colors[s]);
    Simulation->SetFillStyle(0);
    Simulation->SetTitle(m_SimulationFileNames[s]);
    if (Simulation == 0) {
      cout<<"Error: Cannot show a histrogram if you don't give a *.root file!"<<endl;
      return false;    
    }
    if (Simulation->GetNbinsX() != Model->GetNbinsX()) {
      cout<<"Error: The histograms don't have the same number of bins!"<<endl;
      return false;    
    }
    Simulations.push_back(Simulation);
  }
  
  if (Simulations.size() > 0) {
    
    for (TH1* Simulation: Simulations) {
      double LargestSigmaDifference = 0;
      double LargestPercentualDifference = 0;
      int LargestDifferenceBin = 0;
      double AvgSigmaDifference = 0;
      int AvgSigmaDifferenceEntries = 0;
      for (int bx = 1; bx <= Simulation->GetNbinsX(); ++bx) {
        double Sim = Simulation->GetBinContent(bx);
        double SimError = Simulation->GetBinError(bx);
        double Mod = Model->GetBinContent(bx);
        double ModError = Model->GetBinError(bx);
        
        
        double fabsDiff = fabs(Sim-Mod);
        double Diff = Sim-Mod;
        //cout<<"Bin "<<bx<<": "<<Diff/SimError<<" vs. "<<Diff/ModError<<endl;
        if (ModError > 0 && SimError > 0) {
          if (fabsDiff/SimError < fabsDiff/ModError) {
            AvgSigmaDifference += Diff/SimError;
            if (LargestSigmaDifference < fabsDiff/SimError) {
              LargestSigmaDifference = fabsDiff/SimError;
              LargestPercentualDifference = fabsDiff/max(Sim, Mod);
              LargestDifferenceBin = bx;
            }
          } else {
            AvgSigmaDifference += Diff/ModError;
            if (LargestSigmaDifference < fabsDiff/ModError) {
              LargestSigmaDifference = fabsDiff/ModError;
              LargestPercentualDifference = fabsDiff/max(Sim, Mod);
              LargestDifferenceBin = bx;
            }
          }
          ++AvgSigmaDifferenceEntries;
        } else if (ModError > 0) {
          AvgSigmaDifference += Diff/ModError;
          ++AvgSigmaDifferenceEntries;
          if (LargestSigmaDifference < fabsDiff/ModError) {
            LargestSigmaDifference = fabsDiff/ModError;
            LargestPercentualDifference = fabsDiff/max(Sim, Mod);
            LargestDifferenceBin = bx;
          }
        } else if (SimError > 0) {
          AvgSigmaDifference += Diff/SimError;
          ++AvgSigmaDifferenceEntries;
          if (LargestSigmaDifference < fabsDiff/SimError) {
            LargestSigmaDifference = fabsDiff/SimError;
            LargestPercentualDifference = fabsDiff/max(Sim, Mod);
            LargestDifferenceBin = bx;
          }
        }
      }
      
      // We are OK if
      // AvgSigmaDifference/AvgSigmaDifferenceEntries < 1
      // AND
      // (
      // LargestSigmaDifference < 3
      // OR
      // LargestSigmaDifference > 3 AND LargestPercentualDifference < 0.05
      //)
      
      if (LargestSigmaDifference == 0) {
        cout<<"OK - not comparable"<<endl;
        return true;
      }
      
      double KSTest = Model->KolmogorovTest(Simulation);
      
      if (KSTest > 0.02) {
        cout<<"OK: ";
        g_ReturnCode = 0;    
      } else {
        if (AvgSigmaDifference/AvgSigmaDifferenceEntries < 1 &&
          (LargestSigmaDifference < 3 || (LargestSigmaDifference > 3 && LargestPercentualDifference < 0.05))) {
          cout<<"OK: ";
        g_ReturnCode = 0;
          } else {
            cout<<"!!!  ATTENTION: NOT IDENTICAL: ";
            g_ReturnCode = 1;
          }
      }
      cout<<"  K-S-Test: "<<KSTest
      <<"  Avgera. sigma diff: "<<AvgSigmaDifference/AvgSigmaDifferenceEntries
      <<"  Largest sigma diff: "<<LargestSigmaDifference
      <<" & largest %-ual diff: "<<100*LargestPercentualDifference
      <<"% in bin "<<LargestDifferenceBin<<endl;
      
    }
  }
  
  TCanvas* C = new TCanvas();
  C->cd();
  C->SetLeftMargin(0.15);
  C->SetRightMargin(0.15);
  C->SetTopMargin(0.15);
  C->SetBottomMargin(0.25);

  double Max = Model->GetMaximum();
  for (TH1* Simulation: Simulations) {
    if (Simulation->GetMaximum() > Max) {
      Max = Simulation->GetMaximum(); 
    }
  }
  
  
  Model->SetMaximum(1.25*Max);
  for (TH1* Simulation: Simulations) Simulation->SetMaximum(1.25*Max);
  Model->Draw();
  for (TH1* Simulation: Simulations) Simulation->Draw("SAME");
  
  TLegend* L = new TLegend(0.15,0.025,0.85,0.125);
  L->AddEntry(Model, m_ModelFileName);
  for (TH1* Simulation: Simulations) L->AddEntry(Simulation);
  L->Draw();
  
  
  C->Update();
  if (m_OutputFileName != "") {
    C->SaveAs(m_OutputFileName);
  }
  
  
  return true;
}


/******************************************************************************
 * Do whatever analysis is necessary
 */
TH1* CompareHistograms::GetHistogram(MString FileName)
{
  if (m_Interrupt == true) return false;
  
  TFile* F = new TFile(FileName);
  
  TIter I(F->GetListOfKeys());
  TKey *Key;

  while ((Key = (TKey*) I()) != 0) {
    TClass *Class = gROOT->GetClass(Key->GetClassName());
    if (Class->InheritsFrom("TH1")) {
      return (TH1*) Key->ReadObj();
    } else if (Class->InheritsFrom("TCanvas")) {
      TCanvas* C = (TCanvas*) Key->ReadObj();
      TIter CanvasContent(C->GetListOfPrimitives());
      TObject* Object;
      while ((Object = CanvasContent()) != 0) {
        if (Object->InheritsFrom("TH1")) {
          return dynamic_cast<TH1*>(Object);
        }
      }
    }
  }
  
  return 0;
}


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
  MGlobal::Initialize();

  TApplication CompareHistogramsApp("CompareHistogramsApp", 0, 0);

  g_Prg = new CompareHistograms();

  if (g_Prg->ParseCommandLine(argc, argv) == false) {
    cerr<<"Error during parsing of command line!"<<endl;
    return -1;
  } 
  if (g_Prg->Analyze() == false) {
    cerr<<"Error during analysis!"<<endl;
    return -2;
  } 

  if (g_IsInteractive == true) {
    CompareHistogramsApp.Run();
  }

  return g_ReturnCode;
}

/*
 * CompareHistograms: the end...
 ******************************************************************************/
