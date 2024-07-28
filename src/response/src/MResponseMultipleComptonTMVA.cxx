/*
 * MResponseMultipleComptonTMVA.cxx
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


////////////////////////////////////////////////////////////////////////////////
//
// MResponseMultipleComptonTMVA
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MResponseMultipleComptonTMVA.h"

// Standard libs:
#include <limits>
#include <algorithm>
#include <vector>
#include <thread>
using namespace std;

// ROOT libs:
#include "TMath.h"

// MEGAlib libs:
#include "MAssert.h"
#include "MStreams.h"
#include "MSystem.h"
#include "MRESEIterator.h"
#include "TMVA/Factory.h"
#include "TMVA/DataLoader.h"
#include "TMVA/Tools.h"
#include "TMVA/TMVAGui.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MResponseMultipleComptonTMVA)
#endif


////////////////////////////////////////////////////////////////////////////////


MString MResponseMultipleComptonTMVA::m_MLPOptionsDefault = "";
MString MResponseMultipleComptonTMVA::m_BDTDOptionsDefault = "";
MString MResponseMultipleComptonTMVA::m_PDEFoamBoostOptionsDefault = "";
MString MResponseMultipleComptonTMVA::m_DNNCPUOptionsDefault = "";
MString MResponseMultipleComptonTMVA::m_DNNGPUOptionsDefault = "";


////////////////////////////////////////////////////////////////////////////////


MResponseMultipleComptonTMVA::MResponseMultipleComptonTMVA()
{
  // Construct an instance of MResponseMultipleComptonTMVA
  
  m_ResponseNameSuffix = "tmva";
  m_MaxNEvents = 10000000; // 10 Mio
  m_MaxNInteractions = 7;
  m_MethodsString = "BDTD";

  m_EnergyMinimum = 100; // keV
  m_EnergyMaximum = 10000; // keV

  SetDefaultOptions();
}


////////////////////////////////////////////////////////////////////////////////


void MResponseMultipleComptonTMVA::SetDefaultOptions()
{
  // Default options for MLP
  m_MLPOptionsDefault = "'H:!V:NeuronType=tanh:VarTransform=N:NCycles=500:HiddenLayers=N,N/2:TrainingMethod=BP:TestRate=5:RandomSeed=0:!UseRegulator'";
  // Default options for BDTD
  m_BDTDOptionsDefault = "'!H:!V:NTrees=400:MinNodeSize=5%:MaxDepth=3:BoostType=AdaBoost:SeparationType=GiniIndex:nCuts=20:VarTransform=Decorrelate'";
  // Default options for PDEFoamBoost
  m_PDEFoamBoostOptionsDefault = "'!H:!V:Boost_Num=30:Boost_Transform=linear:SigBgSeparate=F:MaxDepth=4:UseYesNoCell=T:DTLogic=MisClassificationError:FillFoamWithOrigWeights=F:TailCut=0:nActiveCells=500:nBin=20:Nmin=400:Kernel=None:Compress=T'";

  // DNN:
  
  // General layout.
  MString DNNLayoutString ("Layout=RELU|2*N,LINEAR");
  
  // Training strategies.
  MString DNNTraining0("LearningRate=1e-2,Momentum=0.5,Repetitions=1,ConvergenceSteps=1000,BatchSize=1024,TestRepetitions=10,WeightDecay=1e-4,Regularization=L2,DropConfig=0.0+0.0,Multithreading=True");
  // MString DNNTraining0("LearningRate=1e-1,Momentum=0.9,Repetitions=1,ConvergenceSteps=1000,BatchSize=256,TestRepetitions=10,WeightDecay=1e-4,Regularization=L2,DropConfig=0.0+0.5+0.5+0.5, Multithreading=True");
  MString DNNTraining1("LearningRate=1e-3,Momentum=0.5,Repetitions=1,ConvergenceSteps=1000,BatchSize=256,TestRepetitions=10,WeightDecay=1e-4,Regularization=L2,DropConfig=0.0+0.0+0.0+0.0,Multithreading=True");
  // MString DNNTraining2("LearningRate=1e-3,Momentum=0.0,Repetitions=1,ConvergenceSteps=1000,BatchSize=256,TestRepetitions=10,WeightDecay=1e-4,Regularization=L2,DropConfig=0.0+0.0+0.0+0.0, Multithreading=True");
  MString DNNTrainingStrategyString ("TrainingStrategy=");
  DNNTrainingStrategyString += DNNTraining0; // + "|" + DNNTraining1; // + "|" + DDNTraining2;
  
  // General Options.
  MString DNNOptions ("'!H:V:ErrorStrategy=CROSSENTROPY:VarTransform=N:WeightInitialization=XAVIERUNIFORM");
  DNNOptions.Append (":"); DNNOptions.Append (DNNLayoutString);
  DNNOptions.Append (":"); DNNOptions.Append (DNNTrainingStrategyString);
 

  // Default options for DNN_CPU
  m_DNNCPUOptionsDefault = DNNOptions + ":Architecture=CPU'";
  // Default option for DNN_GPU
  m_DNNGPUOptionsDefault = DNNOptions + ":Architecture=GPU'";
}


////////////////////////////////////////////////////////////////////////////////


MResponseMultipleComptonTMVA::~MResponseMultipleComptonTMVA()
{
  // Delete this instance of MResponseMultipleComptonTMVA
}


////////////////////////////////////////////////////////////////////////////////


//! Return a brief description of this response class
MString MResponseMultipleComptonTMVA::Description()
{
  return MString("Compton event reconstruction (TMVA)");
}


////////////////////////////////////////////////////////////////////////////////


//! Return information on the parsable options for this response class
MString MResponseMultipleComptonTMVA::Options()
{  
  SetDefaultOptions();
  
  ostringstream out;
  out<<"             methods:               the selected TMVA methods (default: BDTD)"<<endl;
  out<<"             maxia:                 the maximum number of interactions (default: 5)"<<endl;
  out<<"             emin:                  minimum energy (default: 100 keV)"<<endl;
  out<<"             emax:                  maximum energy (default: 10000 keV)"<<endl;
  out<<"             mlp_options:           MLP options (default: ["<<m_MLPOptionsDefault<<"])"<<endl;
  out<<"             bdtd_options:          BDTD options (default: ["<<m_BDTDOptionsDefault<<"])"<<endl;
  out<<"             pdefoamboost_options:  PDE foam boost options (default: ["<<m_PDEFoamBoostOptionsDefault<<"])"<<endl;
  out<<"             dnncpu_options:        DNN CPU (default: ["<<m_DNNCPUOptionsDefault<<"])"<<endl;
  out<<"             dnngpu_options:        DNN GPU (default: ["<<m_DNNGPUOptionsDefault<<"])"<<endl;
  
  return MString(out);
}


////////////////////////////////////////////////////////////////////////////////


//! Parse the options
bool MResponseMultipleComptonTMVA::ParseOptions(const MString& Options)
{
  /*
  // Split the different options
  vector<MString> Split1 = Options.Tokenize(":");
  // Split Option <-> Value
  vector<vector<MString>> Split2;
  for (MString S: Split1) {
    Split2.push_back(S.Tokenize("=")); 
  }
  */
  
  // This requires a special split
  vector<MString> SplitRaw = Options.Tokenize(":");
  //cout<<endl;
  //// Basic sanity check and to lower for all options
  //for (unsigned int i = 0; i < SplitRaw.size(); ++i) {
  //  cout<<SplitRaw[i]<<endl;
  //}
  //cout<<endl;

  vector<MString> Split1;
  bool IsOpen = false;
  for (unsigned int i = 0; i < SplitRaw.size(); ++i) {
    MString S = SplitRaw[i];
    if (IsOpen == false) {
      if (S.Contains("[") == true) {
        IsOpen = true;
        if (S.Contains("]") == true) {
          IsOpen = false; 
          Split1.push_back(S);
        } else {
          Split1.push_back(S + ":"); 
        }
      } else {
        Split1.push_back(S); 
      }
    } else {
      Split1.back() = Split1.back() + ":" + S;
      if (S.Contains("]") == true) {
        IsOpen = false; 
      }
    }
  }
  
  // Basic sanity check and to lower for all options
  //for (unsigned int i = 0; i < Split1.size(); ++i) {
  //  cout<<Split1[i]<<endl;
  //}
  //cout<<endl;
  
  vector<vector<MString>> Split2;
  for (MString S: Split1) {
    vector<MString> Splitted;
    if (S.Contains("=") == true) {
      Splitted.push_back(S.GetSubString(0, S.First('=')));
      if (S.Length() > S.First('=')+1) {
       Splitted.push_back(S.GetSubString(S.First('=')+1));
      }
    } else {
      Splitted.push_back(S); 
    }
    
    Split2.push_back(Splitted); 
  }
  
  // Basic sanity check and to lower for all options
  for (unsigned int i = 0; i < Split2.size(); ++i) {
    for (unsigned int j = 0; j < Split2[i].size(); ++j) {
      Split2[i][j].RemoveAllInPlace("[");
      Split2[i][j].RemoveAllInPlace("]");
      //cout<<Split2[i][j]<<endl;
    }
  }
  
  // Basic sanity check and to lower for all options
  for (unsigned int i = 0; i < Split2.size(); ++i) {
    if (Split2[i].size() == 0) {
      mout<<"Error: Empty option in string "<<Options<<endl;
      return false;
    }    
    if (Split2[i].size() == 1) {
      mout<<"Error: Option has no value: "<<Split2[i][0]<<endl;
      return false;
    }
    if (Split2[i].size() > 2) {
      mout<<"Error: Option has more than one value or you used the wrong separator (not \":\"): "<<Split1[i]<<endl;
      return false;
    }
    Split2[i][0].ToLowerInPlace();
  }
  
  m_MLPOptions = m_MLPOptionsDefault;
  m_BDTDOptions = m_BDTDOptionsDefault;
  m_PDEFoamBoostOptions = m_PDEFoamBoostOptionsDefault;
  m_DNNCPUOptions = m_DNNCPUOptionsDefault;
  m_DNNGPUOptions = m_DNNGPUOptionsDefault;
  
  // Parse
  for (unsigned int i = 0; i < Split2.size(); ++i) {
    string Value = Split2[i][1].Data();
    
    if (Split2[i][0] == "methods") {
      m_MethodsString = Value;
    } else if (Split2[i][0] == "maxia") {
      m_MaxNInteractions = stoi(Value);
    } else if (Split2[i][0] == "emin") {
      m_EnergyMinimum = stod(Value);
    } else if (Split2[i][0] == "emax") {
      m_EnergyMaximum = stod(Value);
    } else if (Split2[i][0] == "mlp_options") {
      m_MLPOptions = Value;
    } else if (Split2[i][0] == "bdtd_options") {
      m_BDTDOptions = Value;
    } else if (Split2[i][0] == "pdefoamboost_options") {
      m_PDEFoamBoostOptions = Value;
    } else if (Split2[i][0] == "dnncpu_options") {
      m_DNNCPUOptions = Value;
    } else if (Split2[i][0] == "dnngpu_options") {
      m_DNNGPUOptions = Value;
    } else {
      mout<<"Error: Unrecognized option "<<Split2[i][0]<<endl;
      return false;
    }
  }
  
  // Sanity checks:
  if (m_MaxNInteractions < 2) {
    mout<<"Error: You need at least 2 interactions"<<endl;
    return false;    
  }
  
  // Dump it for user info
  mout<<endl;
  mout<<"Choosen options for Compton event reconstruction using TMVA:"<<endl;
  mout<<"  TMVA methods:                            "<<m_MethodsString<<endl;
  mout<<"  Maximum number of interactions:          "<<m_MaxNInteractions<<endl;
  mout<<"  Minimum energy:                          "<<m_EnergyMinimum<<endl;
  mout<<"  Maximum energy:                          "<<m_EnergyMaximum<<endl;
  if (m_MLPOptions != m_MLPOptionsDefault) {
    mout<<"  MLP options:                             "<<m_MLPOptions<<endl; 
  }
  if (m_BDTDOptions != m_BDTDOptionsDefault) {
    mout<<"  BDTD options:                            "<<m_BDTDOptions<<endl; 
  }
  if (m_PDEFoamBoostOptions != m_PDEFoamBoostOptionsDefault) {
    mout<<"  PDE Foa, Boost options:                  "<<m_PDEFoamBoostOptions<<endl; 
  }
  if (m_DNNCPUOptions != m_DNNCPUOptionsDefault) {
    mout<<"  DNN CPU options:                         "<<m_DNNCPUOptions<<endl; 
  }
  if (m_DNNGPUOptions != m_DNNGPUOptionsDefault) {
    mout<<"  DNN GPU options:                         "<<m_DNNGPUOptions<<endl; 
  }
  mout<<endl;
  
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Initialize the response matrices and their generation
bool MResponseMultipleComptonTMVA::Initialize() 
{ 
  m_Methods.ResetUsedMethods();
  if (m_Methods.SetUsedMethods(m_MethodsString) == false) {
    merr<<"Having troubles to set the used TMVA methods."<<endl;
    return false;
  }
  
  if (m_Methods.GetNumberOfUsedMethods() == 0) {
    merr<<"No TMVA methods set."<<endl;
    return false;
  }
  
  
  m_MLPOptions.RemoveAllInPlace("'");
  m_BDTDOptions.RemoveAllInPlace("'");
  m_PDEFoamBoostOptions.RemoveAllInPlace("'");
  m_DNNCPUOptions.RemoveAllInPlace("'");
  m_DNNGPUOptions.RemoveAllInPlace("'");
  
  // First find good and bad file name
  MString GoodFileName;
  MString BadFileName;
  if (m_DataFileName.EndsWith(".good.root") == true) {
    GoodFileName = m_DataFileName;
    BadFileName = GoodFileName;
    BadFileName.ReplaceAtEndInPlace(".good.root", ".bad.root");
  } else if (m_DataFileName.EndsWith(".bad.root") == true) {
    BadFileName = m_DataFileName;
    GoodFileName = BadFileName;
    GoodFileName.ReplaceAtEndInPlace(".bad.root", ".good.root");
  } else {
    merr<<"Unable to identify file name suffix of file \""<<m_DataFileName<<"\". Make sure it is either *.good.root or *.bad.root"<<endl;
    return false;
  }
  
  // Second find all the files for different sequence lengths
  size_t Index = GoodFileName.FindLast(".seq");
  MString Prefix = GoodFileName.GetSubString(0, Index);
  for (unsigned int s = 2; s <= m_MaxNInteractions; ++s) {
    MString Good = Prefix + ".seq" + s + ".good.root";
    MString Bad = Prefix + ".seq" + s + ".bad.root";

    MFile::ExpandFileName(Good);
    MFile::ExpandFileName(Bad);
    
    cout<<"Good: "<<Good<<" vs. "<<Bad<<endl;
    
    if (MFile::Exists(Good) && MFile::Exists(Bad)) {
      m_GoodFileNames.push_back(Good); 
      m_BadFileNames.push_back(Bad);
      m_SequenceLengths.push_back(s);
      mout<<"Found sequence "<<s<<" data"<<endl;
    }
  }
  if (m_GoodFileNames.size() == 0) {
    merr<<"No usable good/bad data files found"<<endl;
    return false;
  }
  
  // Create output directory
  MString OutDir = m_ResponseName;
  if (MFile::FileExists(OutDir) == true) {
    merr<<"Cannot create output directory"<<endl;
    return false;   
  }
  gSystem->mkdir(OutDir);  
  
  
  // Check if we have the data on the path towards the initial photon position in the data sets
  bool UsePathToFirstIA = false;
  TFile* SourceFile = new TFile(m_GoodFileNames[0]); 
  TTree* SourceTree = (TTree*) SourceFile->Get("Good");
  if (SourceTree == nullptr) {
    merr<<"Unable to read good events tree from file "<<m_GoodFileNames[0]<<endl;
    return false;
  }
  TObjArray* Branches = SourceTree->GetListOfBranches();
  for (int b = 0; b < Branches->GetEntries(); ++b) {
    TBranch* B = dynamic_cast<TBranch*>(Branches->At(b));
    TString Name = B->GetName();
    if (Name == "AbsorptionProbabilityToFirstIAAverage") {
      UsePathToFirstIA = true;
      break;
    }
  }  
  SourceFile->Close();
  
  
  // Create steering file
  ofstream out;
  out.open(m_ResponseName + ".tmva");
  out<<"# TMVA steering file"<<endl;
  out<<endl;
  out<<"# Sequence length, 2..x"<<endl;
  out<<"SL ";
  for (unsigned int s: m_SequenceLengths) {
    out<<s<<" ";
  }
  out<<endl;
  out<<"# Energy window"<<endl;
  out<<"EW "<<m_EnergyMinimum<<" "<<m_EnergyMaximum<<endl;
  out<<endl;
  out<<endl;
  out<<"# Trained Algorithms (e.g. MLP, BDTD, PDEFoamBoost, DNN_GPU, DNN_CPU)"<<endl;
  out<<"TA "<<m_Methods.GetUsedMethodsString()<<endl;
  out<<endl;  
  out<<"# Directory name"<<endl;
  out<<"DN "<<m_ResponseName<<endl;
  out<<endl;
  out<<"# Use path to first IA"<<endl;
  out<<"PT "<<(UsePathToFirstIA == true ? "true" : "false")<<endl;
  out<<endl;
  out.close();
  
  gSystem->ChangeDirectory(m_ResponseName);
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseMultipleComptonTMVA::Analyze()
{
  // Everything happens here
  
  unsigned int NThreads = m_GoodFileNames.size();
  unsigned int MaxThreadsRunning = 1; // thread::hardware_concurrency() / 2;
  vector<thread> Threads(m_GoodFileNames.size());
  m_ThreadRunning.resize(m_GoodFileNames.size(), false);
  
  // We start with the longest running threads at higher sequence length
  //for (unsigned int t = NThreads-1; t < NThreads; --t) {
  for (unsigned int t = 0; t < NThreads; ++t) {
      // Start a thread
    m_ThreadRunning[t] = true;
    Threads[t] = thread(&MResponseMultipleComptonTMVA::AnalysisThreadEntry, this, t);
    // 5 seconds to give it a chance to fully initialize
    this_thread::sleep_for(chrono::milliseconds(5000)); 
    // sleep if we have more than the max allowed threads running
    unsigned int Running = 0;
    
    if (t == NThreads-1) break;
    do {
      Running = 0;
      for (unsigned int t = 0; t < NThreads; ++t) {
        if (m_ThreadRunning[t] == true) {
          Running++;
        }
      }
      if (Running >= MaxThreadsRunning) {
        //cout<<"Too many threads running ("<<Running<<") --- sleeping"<<endl; 
        this_thread::sleep_for(chrono::milliseconds(100)); 
      }
      
    } while (Running >= MaxThreadsRunning);
  }
  
  // Wait until all have finished
  while (true) {
    bool Finished = true;
    for (unsigned int t = 0; t < NThreads; ++t) {
      if (m_ThreadRunning[t] == true) {
        Finished = false;
        break;
      }
    }
    if (Finished == false) {
      this_thread::sleep_for(chrono::milliseconds(100));
    } else {
      for (unsigned int t = 0; t < NThreads; ++t) {
        Threads[t].join();
      }
      break;
    }
  }

  
  return false; // since it does not need to be called again
}
  
  
////////////////////////////////////////////////////////////////////////////////
  
  
void MResponseMultipleComptonTMVA::AnalysisThreadEntry(unsigned int ThreadID)
{
  // We lock all the initializations
  m_TheadMutex.lock();
  cout<<"Launching thread "<<ThreadID<<endl;
  
  MTime Now;
  MString TimeString = Now.GetShortString();
  
  unsigned int SequenceLength = m_SequenceLengths[ThreadID];
  
  MString ResultsFileName = m_ResponseName + ".seq" + SequenceLength + ".root";
  TFile* Results = new TFile(ResultsFileName, "recreate");
  
  
  TFile* SourceFile = new TFile(m_GoodFileNames[ThreadID]); 
  TTree* SourceTree = (TTree*) SourceFile->Get("Good");
  if (SourceTree == nullptr) {
    merr<<"Unable to read good events tree from file "<<m_GoodFileNames[ThreadID]<<endl;
    return;
  }
  
  TFile* BackgroundFile = new TFile(m_BadFileNames[ThreadID]); 
  TTree* BackgroundTree = (TTree*) BackgroundFile->Get("Bad");
  if (BackgroundTree == nullptr) {
    merr<<"Unable to read bad events tree from file "<<m_BadFileNames[ThreadID]<<endl;
    return;
  }
  
  // The background tree could be much larger than the source
  // If it is more than twice as large create a smaller tree for training
  
  unsigned long SourceTreeSize = SourceTree->GetEntries();
  unsigned long BackgroundTreeSize = BackgroundTree->GetEntries();  
  
  cout<<"Tree sizes: background: "<<BackgroundTreeSize<<"  source: "<<SourceTreeSize<<endl;
  
  if (SourceTreeSize > m_MaxNEvents) {
    cout<<"Reducing source tree size from "<<SourceTreeSize<<" to "<<m_MaxNEvents<<" (i.e. the maximum set)"<<endl;
    TTree* NewSourceTree = SourceTree->CloneTree(0);
    NewSourceTree->SetDirectory(0);
    
    for (unsigned long i = 0; i < m_MaxNEvents; ++i) {
      SourceTree->GetEntry(i);
      NewSourceTree->Fill();
    }
    
    SourceFile->Close();
    SourceTree = NewSourceTree;
    SourceTreeSize = m_MaxNEvents;
  }  
  
  if (BackgroundTreeSize > SourceTreeSize) {
    cout<<"Reducing background tree size from "<<BackgroundTreeSize<<" to "<<SourceTreeSize<<" (the source tree size)"<<endl;
    TTree* NewBackgroundTree = BackgroundTree->CloneTree(0);
    NewBackgroundTree->SetDirectory(0);
    
    for (unsigned long i = 0; i < SourceTreeSize; ++i) {
      BackgroundTree->GetEntry(i);
      NewBackgroundTree->Fill();
    }
    
    BackgroundFile->Close();
    BackgroundTree = NewBackgroundTree;
  }  
  
  TMVA::Factory *factory = new TMVA::Factory("TMVAClassification", Results,
                                             "!V:!Silent:Color:DrawProgressBar:Transformations=I:AnalysisType=Classification" );
  //                                             "!V:!Silent:Color:DrawProgressBar:Transformations=I;D;P;G,D:AnalysisType=Classification" );
  
  MString OutDir("N");
  OutDir += SequenceLength;
  TMVA::DataLoader *dataloader = new TMVA::DataLoader(OutDir.Data());
  
  m_TheadMutex.unlock();
  
  vector<TString> IgnoredBranches = { "SimulationIDs" }; //, "AbsorptionProbabilityToFirstIAAverage", "AbsorptionProbabilityToFirstIAMaximum",  "AbsorptionProbabilityToFirstIAMinimum", "ZenithAngle", "NadirAngle" };
  
  TObjArray* Branches = SourceTree->GetListOfBranches();
  for (int b = 0; b < Branches->GetEntries(); ++b) {
    TBranch* B = dynamic_cast<TBranch*>(Branches->At(b));
    TString Name = B->GetName();
    if (find(IgnoredBranches.begin(), IgnoredBranches.end(), Name) == IgnoredBranches.end()) {
      dataloader->AddVariable(Name, 'F');
    }
  }
  
  dataloader->AddSignalTree(SourceTree, 1.0);
  dataloader->AddBackgroundTree(BackgroundTree, 1.0);

  // Random splitting, new seed each time, verbose output 
  TString Prep = "SplitMode=Random:SplitSeed=0:V";
  dataloader->PrepareTrainingAndTestTree("", Prep);
  
  // Standard MLP
  if (m_Methods.IsUsedMethod(MERCSRTMVAMethod::c_MLP) == true) {
    // New seed each run via RandomSeed=0
    factory->BookMethod( dataloader, TMVA::Types::kMLP, "MLP", m_MLPOptions);
  }
  
  // Boosted decision tree: Decorrelation + Adaptive Boost
  if (m_Methods.IsUsedMethod(MERCSRTMVAMethod::c_BDTD) == true) {
    factory->BookMethod( dataloader, TMVA::Types::kBDT, "BDTD", m_BDTDOptions);
  }
  
  if (m_Methods.IsUsedMethod(MERCSRTMVAMethod::c_PDEFoamBoost) == true) {
    factory->BookMethod( dataloader, TMVA::Types::kPDEFoam, "PDEFoamBoost", m_PDEFoamBoostOptions);
  }
  
  /*
  if (m_Methods["PDERSPCA"] == 1) {
    factory->BookMethod( dataloader, TMVA::Types::kPDERS, "PDERSPCA", "!H:!V:VolumeRangeMode=Adaptive:KernelEstimator=Gauss:GaussSigma=0.3:NEventsMin=400:NEventsMax=600:VarTransform=PCA" );
  }
  */
  
  // DNN Cuda implementation.
  if (m_Methods.IsUsedMethod(MERCSRTMVAMethod::c_DNN_GPU) == true) {
    factory->BookMethod(dataloader, TMVA::Types::kDNN, "DNN_GPU", m_DNNGPUOptions);
  }
  
  // DNN Multi-core CPU implementation.
  if (m_Methods.IsUsedMethod(MERCSRTMVAMethod::c_DNN_CPU) == true) {
    factory->BookMethod(dataloader, TMVA::Types::kDNN, "DNN_CPU", m_DNNCPUOptions);
  }
  
  factory->TrainAllMethods();
  factory->TestAllMethods();
  factory->EvaluateAllMethods();
  
  Results->Close();
  
  delete factory;
  delete dataloader;

  m_TheadMutex.lock();
  m_ThreadRunning[ThreadID] = false;
  m_TheadMutex.unlock();
}


// MResponseMultipleComptonTMVA.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
