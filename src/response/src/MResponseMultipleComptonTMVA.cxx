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


#ifdef ___CINT___
ClassImp(MResponseMultipleComptonTMVA)
#endif


////////////////////////////////////////////////////////////////////////////////


MResponseMultipleComptonTMVA::MResponseMultipleComptonTMVA()
{
  // Construct an instance of MResponseMultipleComptonTMVA
  
  m_ResponseNameSuffix = "tmva";
  m_MaxNEvents = 10000000; // 10 Mio
  m_MethodsString = "BDTD";
}


////////////////////////////////////////////////////////////////////////////////


MResponseMultipleComptonTMVA::~MResponseMultipleComptonTMVA()
{
  // Delete this instance of MResponseMultipleComptonTMVA
}


////////////////////////////////////////////////////////////////////////////////


//! Initialize the response matrices and their generation
bool MResponseMultipleComptonTMVA::Initialize() 
{ 
  // MLP, BDTD, PDEFoamBoost, DNN_GPU, DNN_CPU
  m_Methods["MLP"] = 0;
  m_Methods["BDTD"] = 0;
  m_Methods["PDEFoamBoost"] = 0;
  m_Methods["DNN_GPU"] = 0;
  m_Methods["DNN_CPU"] = 0;
  
  vector<MString> MethodsStrings = m_MethodsString.Tokenize(",");
  unsigned int NUsedMethods = 0;
  for (MString M: MethodsStrings) {
    if (m_Methods.find(M) != m_Methods.end()) {
      m_Methods[M] = 1;
      ++NUsedMethods;
    } else {
      merr<<"Unable to find method: "<<M<<endl;
      return false;
    }
  }
  if (NUsedMethods == 0) {
    merr<<"No TMVA methods set."<<endl;
    return false;
  }
  
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
  gSystem->mkdir(OutDir);  
  
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
  out<<endl;
  out<<"# Trained Algorithms (e.g. MLP, BDTD, PDEFoamBoost, DNN_GPU, DNN_CPU)"<<endl;
  out<<"TA ";
  for (std::pair<MString, int> E : m_Methods) {
    if (E.second == 1) {
      out<<E.first<<" ";
    }
  }
  out<<endl;
  out<<endl;  
  out<<"# Directory name"<<endl;
  out<<"DN "<<m_ResponseName<<endl;
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
    cout<<"Reducing source tree size from "<<BackgroundTreeSize<<" to "<<m_MaxNEvents<<" (i.e. the maximum set)"<<endl;
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
                                             "!V:!Silent:Color:DrawProgressBar:Transformations=I;D;P;G,D:AnalysisType=Classification" );
  
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

  
  TString Prep = "SplitMode=Random:V";
  /*
  Prep += ":nTrain_Signal=";
  Prep += SE/2;
  Prep += ":nTest_Signal=";
  Prep += SE/2;
  Prep += ":nTrain_Background=";
  Prep += SE/2;
  Prep += ":nTest_Background=";
  Prep += SE/2;
  */
  
  dataloader->PrepareTrainingAndTestTree("", Prep);
  
  dataloader->PrepareTrainingAndTestTree("", "SplitMode=Random:V");
  
  // Standard MLP
  if (m_Methods["MLP"] == 1) {
    factory->BookMethod( dataloader, TMVA::Types::kMLP, "MLP", "H:!V:NeuronType=tanh:VarTransform=N:NCycles=100:HiddenLayers=N+5:TestRate=5:!UseRegulator" );
  }
  
  // Boosted decision tree: Decorrelation + Adaptive Boost
  if (m_Methods["BDTD"] == 1) {
    factory->BookMethod( dataloader, TMVA::Types::kBDT, "BDTD", "!H:!V:NTrees=400:MinNodeSize=5%:MaxDepth=3:BoostType=AdaBoost:SeparationType=GiniIndex:nCuts=20:VarTransform=Decorrelate" );
  }
  
  if (m_Methods["PDEFoamBoost"] == 1) {
    factory->BookMethod( dataloader, TMVA::Types::kPDEFoam, "PDEFoamBoost","!H:!V:Boost_Num=30:Boost_Transform=linear:SigBgSeparate=F:MaxDepth=4:UseYesNoCell=T:DTLogic=MisClassificationError:FillFoamWithOrigWeights=F:TailCut=0:nActiveCells=500:nBin=20:Nmin=400:Kernel=None:Compress=T" );
  }
  

  if (m_Methods["PDERSPCA"] == 1) {
    factory->BookMethod( dataloader, TMVA::Types::kPDERS, "PDERSPCA", "!H:!V:VolumeRangeMode=Adaptive:KernelEstimator=Gauss:GaussSigma=0.3:NEventsMin=400:NEventsMax=600:VarTransform=PCA" );
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

  m_TheadMutex.lock();
  m_ThreadRunning[ThreadID] = false;
  m_TheadMutex.unlock();
}



// MResponseMultipleComptonTMVA.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
