/*
 * MResponseEventClusterizerTMVA.cxx
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
// MResponseEventClusterizerTMVA
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MResponseEventClusterizerTMVA.h"

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
ClassImp(MResponseEventClusterizerTMVA)
#endif


////////////////////////////////////////////////////////////////////////////////


MResponseEventClusterizerTMVA::MResponseEventClusterizerTMVA()
{
  // Construct an instance of MResponseEventClusterizerTMVA
  
  m_ResponseNameSuffix = "tmva";
  m_MaxNHits = 25;
  m_MaxNEvents = 10000000; // 10 Mio
  m_MethodsString = "MLP";
  m_EnergyBinEdges = { 0, 100000 };
}


////////////////////////////////////////////////////////////////////////////////


MResponseEventClusterizerTMVA::~MResponseEventClusterizerTMVA()
{
  // Delete this instance of MResponseEventClusterizerTMVA
}


////////////////////////////////////////////////////////////////////////////////


//! Return a brief description of this response class
MString MResponseEventClusterizerTMVA::Description()
{
  return MString("Event clusterizer using TMVA machine learning");
}


////////////////////////////////////////////////////////////////////////////////


//! Return information on the parsable options for this response class
MString MResponseEventClusterizerTMVA::Options()
{
  ostringstream out;
  out<<"             methods:          the selected TMVA methods (default: \"MLP\")"<<endl;
  out<<"             maxhits :         the maximum number of interactions (default: 5)"<<endl;
  out<<"             energybinedges :  comma seperated list of the edges of the energy bins (default: \"0,100000\")"<<endl;
  
  return MString(out);
}


////////////////////////////////////////////////////////////////////////////////


//! Parse the options
bool MResponseEventClusterizerTMVA::ParseOptions(const MString& Options)
{
  // Split the different options
  vector<MString> Split1 = Options.Tokenize(":");
  // Split Option <-> Value
  vector<vector<MString>> Split2;
  for (MString S: Split1) {
    Split2.push_back(S.Tokenize("=")); 
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
  
  // Parse
  for (unsigned int i = 0; i < Split2.size(); ++i) {
    string Value = Split2[i][1].Data();
    
    if (Split2[i][0] == "methods") {
      m_MethodsString = Value;
    } else if (Split2[i][0] == "maxhits") {
      m_MaxNHits = stoi(Value);
    } else if (Split2[i][0] == "energybinedges") {
      vector<MString> AsText = MString(Value).Tokenize(",");
      m_EnergyBinEdges.clear();
      for (auto S: AsText) m_EnergyBinEdges.push_back(S.ToInt());
    } else {
      mout<<"Error: Unrecognized option "<<Split2[i][0]<<endl;
      return false;
    }
  }
  
  // Sanity checks:
  if (m_MaxNHits < 1) {
    mout<<"Error: You need at least 1 hits (but better 5..20)"<<endl;
    return false;    
  }
  
  if (m_MaxNEvents < 1000) {
    mout<<"Error: I am not going to start training with less than 1000 events..."<<endl;
    return false;    
  }
  
  cout<<"Currently only MLP has been coded -- thus I am just using that"<<endl;
  //m_MethodsString = "MLP";
  
  // Dump it for user info
  mout<<endl;
  mout<<"Choosen options for event clustering using TMVA:"<<endl;
  mout<<"  TMVA methods:                                                  "<<m_MethodsString<<endl;
  mout<<"  Maximum number of hits (assuming you have so many data files): "<<m_MaxNHits<<endl;
  mout<<"  Energy bin edges:                                              ";
  for (auto E: m_EnergyBinEdges) mout<<E<<" ";
  mout<<endl;
  mout<<endl;
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Initialize the response matrices and their generation
bool MResponseEventClusterizerTMVA::Initialize() 
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
  
  // Second find all the files for different number of hits
  MString Prefix = m_DataFileName.GetSubString(0, m_DataFileName.FindLast(".hits"));
  int NumberOfGroups = m_DataFileName.GetSubString(m_DataFileName.FindLast(".groups") + 7, m_DataFileName.FindLast(".eventclusterizer.root")).ToInt();
  
  if (NumberOfGroups == 0) {
    merr<<"Found no groups in the file name..."<<endl;
    return false;
  }

  unsigned int s = 0;
  unsigned int s_max = m_MaxNHits;
  for (s = 2; s <= s_max; ++s) {
    MString Name = Prefix + ".hits" + s + ".groups" + NumberOfGroups + ".eventclusterizer.root";

    MFile::ExpandFileName(Name);
    
    cout<<"File name: "<<Name<<endl;
    
    if (MFile::Exists(Name) == true) {
      m_FileNames.push_back(Name);
      m_FileHits.push_back(s);
      m_MaxNHits = s;
      mout<<"Found and added data file for hit number "<<s<<endl;
    } else {
      break;
    }
  }
  
  if (m_FileNames.size() == 0) {
    merr<<"No usable data files found"<<endl;
    return false;
  }
  
  // Create output directory
  MString OutDir = m_ResponseName;
  if (MFile::FileExists(OutDir) == true) {
    merr<<"Cannot create output directory"<<endl;
    return false;   
  }
  gSystem->mkdir(OutDir);  
  
  // Check how many groups we have
  TFile* File = new TFile(m_FileNames[0]); 
  TTree* Tree = (TTree*) File->Get("EventClusterizer");
  if (Tree == nullptr) {
    merr<<"Unable to read data tree from file "<<m_FileNames[0]<<endl;
    return false;
  }
  TObjArray* Branches = Tree->GetListOfBranches();
  TBranch* B = dynamic_cast<TBranch*>(Branches->Last());
  if (B != nullptr) {
    TString Name = B->GetName();
    vector<MString> Tokens = MString(Name).Tokenize("_");
    if (Tokens.size() == 3) {
      m_MaxNGroups = Tokens[2].ToUnsignedInt();
      if (m_MaxNGroups > 0) {
        m_MaxNGroups -= 1; // Because we always add an overflow...
      } else {
        merr<<"Unable to parse the max number of groups from branch name."<<endl;
        return false;
      }
    } else {
      merr<<"Unable to find correct last ResultHitGroups branches in tree "<<m_FileNames[0]<<endl;
      return false;
    }
  } else {
    merr<<"Unable to to find branches in tree "<<m_FileNames[0]<<endl;
    return false;
  }
  File->Close();
  
  
  // Create steering file
  ofstream out;
  out.open(m_ResponseName + ".eventclusterizer.tmva");
  out<<"# TMVA steering file"<<endl;
  out<<endl;
  out<<"# Maximum number of hits"<<endl;
  out<<"MH "<<m_MaxNHits<<endl;
  out<<endl;
  out<<"# Maximum number of groups"<<endl;
  out<<"MG "<<m_MaxNGroups<<endl;
  out<<endl;
  out<<"# Energy bins"<<endl;
  out<<"EB ";
  for (auto E: m_EnergyBinEdges) out<<E<<" ";
  out<<endl;
  out<<endl;
  out<<"# Trained Algorithms (e.g. MLP, BDTD, PDEFoamBoost, DNN_GPU, DNN_CPU)"<<endl;
  out<<"TA "<<m_Methods.GetUsedMethodsString()<<endl;
  out<<endl;  
  out<<"# Directory name"<<endl;
  out<<"DN "<<m_ResponseName<<endl;
  out<<endl;
  out.close();
  
  gSystem->ChangeDirectory(m_ResponseName);
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseEventClusterizerTMVA::Analyze()
{
  // Everything happens here
  
  unsigned int NThreads = m_FileNames.size() * (m_EnergyBinEdges.size() - 1);
  unsigned int MaxThreadsRunning = thread::hardware_concurrency() / 2;
  vector<thread> Threads(NThreads);
  m_ThreadRunning.resize(NThreads, false);
  
  // We start with the longest running threads at higher sequence length
  //for (unsigned int t = NThreads-1; t < NThreads; --t) {
  for (unsigned int t = 0; t < NThreads; ++t) {
      // Start a thread
    m_ThreadRunning[t] = true;
    Threads[t] = thread(&MResponseEventClusterizerTMVA::AnalysisThreadEntry, this, t);
    // 5 seconds to give it a chance to fully initialize
    this_thread::sleep_for(chrono::milliseconds(500)); 
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
  
  
void MResponseEventClusterizerTMVA::AnalysisThreadEntry(unsigned int ThreadID)
{
  // We lock all the initializations
  m_TheadMutex.lock();
  cout<<"Launching thread "<<ThreadID<<endl;
  
  MTime Now;
  MString TimeString = Now.GetShortString();
  
  unsigned int FileBin = ThreadID % m_FileHits.size();
  unsigned int EnergyBin = ThreadID / m_FileHits.size();
  
  unsigned int NHits = m_FileHits[FileBin];
  
  MString ResultsFile = m_ResponseName + ".hits" + NHits + ".emin" + m_EnergyBinEdges[EnergyBin] + ".emax" + m_EnergyBinEdges[EnergyBin+1];
  TFile* Results = new TFile(ResultsFile + ".root", "recreate");
  
  
  TFile* File = new TFile(m_FileNames[FileBin]); 
  TTree* FullDataTree = (TTree*) File->Get("EventClusterizer");
  if (FullDataTree == nullptr) {
    merr<<"Unable to read events tree from file "<<m_FileNames[FileBin]<<endl;
    
    m_ThreadRunning[ThreadID] = false;
    m_TheadMutex.unlock();
    
    return;
  }

  // Filter energy
  gROOT->cd();
  MString EnergyString = "Energy_1";
  for (unsigned int i = 2; i <= NHits; ++i) {
    EnergyString += " + Energy_" + MString(i);
  }
  EnergyString = MString("(") + EnergyString + ") >= " + m_EnergyBinEdges[EnergyBin] + " && (" + EnergyString + ") <= " + m_EnergyBinEdges[EnergyBin+1];
    
  TTree* Tree = FullDataTree->CopyTree(EnergyString, "");
    
  // Limit the number of events:
  unsigned long TreeSize = Tree->GetEntries();
  if (TreeSize > m_MaxNEvents) {
    cout<<"Reducing source tree size from "<<TreeSize<<" to "<<m_MaxNEvents<<" (i.e. the maximum set)"<<endl;
    TTree* NewTree = Tree->CloneTree(0);
    NewTree->SetDirectory(0);
    
    for (unsigned long i = 0; i < m_MaxNEvents; ++i) {
      Tree->GetEntry(i);
      NewTree->Fill();
    }
    
    Tree = NewTree;
    TreeSize = Tree->GetEntries();
  }  
  
  if (TreeSize < 1000) {
    cout<<"ERROR: You have less than 1000 events for training (hits = "<<FileBin+2<<", E=["<<m_EnergyBinEdges[EnergyBin]<<","<<m_EnergyBinEdges[EnergyBin+1]<<"])... You should have > 100,000 or better millions..."<<endl;
    File->Close();
    
    m_ThreadRunning[ThreadID] = false;
    m_TheadMutex.unlock();
    
    return;
  }

  
  TMVA::Factory *factory = new TMVA::Factory("TMVARegression", Results, "!V:Silent:Color:DrawProgressBar:Transformations=I;D;P;G,D:AnalysisType=Regression");
  
  TMVA::DataLoader* DataLoader = new TMVA::DataLoader(ResultsFile.Data());
  
  m_TheadMutex.unlock();
  
  vector<TString> IgnoredBranches = { "SimulationID" }; 
  
  TObjArray* Branches = Tree->GetListOfBranches();
  for (int b = 0; b < Branches->GetEntries(); ++b) {
    TBranch* B = dynamic_cast<TBranch*>(Branches->At(b));
    TString Name = B->GetName();
    if (Name.BeginsWith("Result") == false) {
      if (find(IgnoredBranches.begin(), IgnoredBranches.end(), Name) == IgnoredBranches.end()) {
        DataLoader->AddVariable(Name, 'F');
      } else {
        DataLoader->AddSpectator(Name, 'F');
      }
    }
  }
  
  // Add the target variables:
  for (int b = 0; b < Branches->GetEntries(); ++b) {
    TBranch* B = dynamic_cast<TBranch*>(Branches->At(b));
    TString Name = B->GetName();
    if (Name.BeginsWith("Result") == true) {
      DataLoader->AddTarget(Name, "F");
    }
  }

  DataLoader->AddRegressionTree(Tree, 1.0);

  
  // Random splitting, new seed each time, verbose output 
  TString Prep = "SplitMode=Random:SplitSeed=0:V";
  DataLoader->PrepareTrainingAndTestTree("", Prep);
  
  // Standard MLP
  if (m_Methods.IsUsedMethod(MERCSRTMVAMethod::c_MLP) == true) {
    // New seed each run via RandomSeed=0
    factory->BookMethod(DataLoader, TMVA::Types::kMLP, "MLP", 
                        "!H:!V:VarTransform=Norm:NeuronType=tanh:NCycles=20000:HiddenLayers=3*N,N:TestRate=6:TrainingMethod=BFGS:Sampling=0.3:SamplingEpoch=0.8:ConvergenceImprove=1e-6:ConvergenceTests=15:!UseRegulator");
  }
  
  // Boosted decision tree: Decorrelation + Adaptive Boost
  if (m_Methods.IsUsedMethod(MERCSRTMVAMethod::c_BDTD) == true) {
    factory->BookMethod(DataLoader, TMVA::Types::kBDT, "BDT", 
                        "!H:!V:NTrees=400:MinNodeSize=5%:MaxDepth=3:BoostType=AdaBoost:SeparationType=GiniIndex:nCuts=20:VarTransform=Decorrelate");
  }
  
  if (m_Methods.IsUsedMethod(MERCSRTMVAMethod::c_PDEFoamBoost) == true) {
    factory->BookMethod(DataLoader, TMVA::Types::kPDEFoam, "PDEFoam", 
                        "!H:!V:MultiTargetRegression=T:TargetSelection=Mpv:TailCut=0.001:VolFrac=0.0666:nActiveCells=500:nSampl=2000:nBin=5:Compress=T:Kernel=None:Nmin=10:VarTransform=None");
  }
  
  factory->TrainAllMethods();
  factory->TestAllMethods();
  factory->EvaluateAllMethods();
  
  Results->Close();
  
  delete factory;
  delete DataLoader;

  File->Close();  
  
  m_TheadMutex.lock();
  m_ThreadRunning[ThreadID] = false;
  m_TheadMutex.unlock();
}


// MResponseEventClusterizerTMVA.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
