/* 
 * TMVAAnalyzer.cxx
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
#include <TFile.h>
#include <TTree.h>
#include <TBranch.h>
#include <TObjArray.h>
#include "TMVA/Factory.h"
#include "TMVA/DataLoader.h"
#include "TMVA/Tools.h"
#include "TMVA/TMVAGui.h"

// MEGAlib
#include "MGlobal.h"
#include "MTime.h"

////////////////////////////////////////////////////////////////////////////////


//! A standalone program based on MEGAlib and ROOT
class TMVAAnalyzer
{
public:
  //! Default constructor
  TMVAAnalyzer();
  //! Default destructor
  ~TMVAAnalyzer();
  
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
  //! The use methods
  map<MString, int> m_Methods;
};


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
TMVAAnalyzer::TMVAAnalyzer() : m_Interrupt(false)
{
  gStyle->SetPalette(1, 0);
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
TMVAAnalyzer::~TMVAAnalyzer()
{
  // Intentionally left blank
}


////////////////////////////////////////////////////////////////////////////////


//! Parse the command line
bool TMVAAnalyzer::ParseCommandLine(int argc, char** argv)
{
  ostringstream Usage;
  Usage<<endl;
  Usage<<"  Usage: TMVAAnalyzer <options>"<<endl;
  Usage<<"    General options:"<<endl;
  Usage<<"         -f:   file name (e.g. Tree.tmva.seq2.good.root - bad is loaded automatically)"<<endl;
  Usage<<"         -m:   methods: MLP, BDTD, DNN_GPU, DNN_CPU"<<endl;
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
    } else if (Option == "-m") {
      m_Methods[argv[++i]] = 1;
      cout<<"Accepting method: "<<argv[i]<<endl;
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
bool TMVAAnalyzer::Analyze()
{
  if (m_Interrupt == true) return false;

  MTime Now;
  MString TimeString = Now.GetShortString();
  
  MString ResultsFileName = MString("Results.") + TimeString + ".root";
  TFile* Results = new TFile(ResultsFileName, "recreate");
  
  TFile* SourceFile = new TFile(m_FileName); 
  TTree* SourceTree = (TTree*) SourceFile->Get("Good");
  if (SourceTree == nullptr) {
    cout<<"Error reading good events tree"<<endl;
    return false;
  }
  
  m_FileName.ReplaceAtEndInPlace(".good.root", ".bad.root");
  TFile* BackgroundFile = new TFile(m_FileName); 
  TTree* BackgroundTree = (TTree*) BackgroundFile->Get("Bad");
  if (BackgroundTree == nullptr) {
    cout<<"Error reading bad events tree"<<endl;
    return false;
  }
      
  TMVA::Factory *factory = new TMVA::Factory("TMVAClassification", Results,
                                            "!V:!Silent:Color:DrawProgressBar:Transformations=I;D;P;G,D:AnalysisType=Classification" );

  MString OutDir("Results.");
  OutDir += TimeString;
  TMVA::DataLoader *dataloader=new TMVA::DataLoader(OutDir.Data());

  
  vector<TString> IgnoredBranches = { "SimulationIDs" }; //, "AbsorptionProbabilityToFirstIAAverage", "AbsorptionProbabilityToFirstIAMaximum",  "AbsorptionProbabilityToFirstIAMinimum", "ZenithAngle", "NadirAngle" };
  
  TObjArray* Branches = SourceTree->GetListOfBranches();
  for (int b = 0; b < Branches->GetEntries(); ++b) {
    TBranch* B = dynamic_cast<TBranch*>(Branches->At(b));
    TString Name = B->GetName();
    if (find(IgnoredBranches.begin(), IgnoredBranches.end(), Name) == IgnoredBranches.end()) {
      dataloader->AddVariable(Name, 'D');
    }
  }
  
  dataloader->AddSignalTree(SourceTree, 1.0);
  dataloader->AddBackgroundTree(BackgroundTree, 1.0);
  
  dataloader->PrepareTrainingAndTestTree("", "SplitMode=Random:V");

  // Standard MLP
  if (m_Methods["MLP"] == 1) {
    factory->BookMethod( dataloader, TMVA::Types::kMLP, "MLP", "H:!V:NeuronType=tanh:VarTransform=N:NCycles=100:HiddenLayers=N+5:TestRate=5:!UseRegulator" );
  }
    
  // Boosted decision tree: Decorrelation + Adaptive Boost
  if (m_Methods["BDTD"] == 1) {
    factory->BookMethod( dataloader, TMVA::Types::kBDT, "BDTD", "!H:!V:NTrees=400:MinNodeSize=5%:MaxDepth=3:BoostType=AdaBoost:SeparationType=GiniIndex:nCuts=20:VarTransform=Decorrelate" );
  }
  
  // General layout.
  TString layoutString ("Layout=TANH|128,TANH|128,TANH|128,LINEAR");
  
  // Training strategies.
  TString training0("LearningRate=1e-1,Momentum=0.9,Repetitions=1,ConvergenceSteps=20,BatchSize=256,TestRepetitions=10,WeightDecay=1e-4,Regularization=L2,DropConfig=0.0+0.5+0.5+0.5, Multithreading=True");
  TString training1("LearningRate=1e-2,Momentum=0.9,Repetitions=1,ConvergenceSteps=20,BatchSize=256,TestRepetitions=10,WeightDecay=1e-4,Regularization=L2,DropConfig=0.0+0.0+0.0+0.0, Multithreading=True");
  TString training2("LearningRate=1e-3,Momentum=0.0,Repetitions=1,ConvergenceSteps=20,BatchSize=256,TestRepetitions=10,WeightDecay=1e-4,Regularization=L2,DropConfig=0.0+0.0+0.0+0.0, Multithreading=True");
  TString trainingStrategyString ("TrainingStrategy=");
  trainingStrategyString += training0 + "|" + training1 + "|" + training2;
  
  // General Options.
  TString dnnOptions ("!H:V:ErrorStrategy=CROSSENTROPY:VarTransform=N:WeightInitialization=XAVIERUNIFORM");
  dnnOptions.Append (":"); dnnOptions.Append (layoutString);
  dnnOptions.Append (":"); dnnOptions.Append (trainingStrategyString);
  
  // Cuda implementation.
  if (m_Methods["DNN_GPU"] == 1) {
    TString gpuOptions = dnnOptions + ":Architecture=GPU";
    factory->BookMethod(dataloader, TMVA::Types::kDNN, "DNN_GPU", gpuOptions);
  }
  
  // Multi-core CPU implementation.
  if (m_Methods["DNN_CPU"] == 1) {
    TString cpuOptions = dnnOptions + ":Architecture=CPU";
    factory->BookMethod(dataloader, TMVA::Types::kDNN, "DNN_CPU", cpuOptions);
  }
    
  
  factory->TrainAllMethods();
  factory->TestAllMethods();
  factory->EvaluateAllMethods();
  
  Results->Close();
  
  delete factory;
  delete dataloader;
  
  // Launch the GUI for the root macros
  //if (!gROOT->IsBatch()) TMVA::TMVAGui(ResultsFileName);
  
  
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


TMVAAnalyzer* g_Prg = 0;
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

  TApplication TMVAAnalyzerApp("TMVAAnalyzerApp", 0, 0);

  g_Prg = new TMVAAnalyzer();

  if (g_Prg->ParseCommandLine(argc, argv) == false) {
    cerr<<"Error during parsing of command line!"<<endl;
    return -1;
  } 
  if (g_Prg->Analyze() == false) {
    cerr<<"Error during analysis!"<<endl;
    return -2;
  } 

  //TMVAAnalyzerApp.Run();

  cout<<"Program exited normally!"<<endl;

  return 0;
}


////////////////////////////////////////////////////////////////////////////////
