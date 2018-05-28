/*
 * MResponseEventClusterizerTMVAEventFile.cxx
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
// MResponseEventClusterizerTMVAEventFile
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MResponseEventClusterizerTMVAEventFile.h"

// Standard libs:
#include <limits>
#include <algorithm>
#include <vector>
using namespace std;

// ROOT libs:
#include "TMath.h"

// MEGAlib libs:
#include "MAssert.h"
#include "MStreams.h"
#include "MSystem.h"
#include "MRESEIterator.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MResponseEventClusterizerTMVAEventFile)
#endif


////////////////////////////////////////////////////////////////////////////////


MResponseEventClusterizerTMVAEventFile::MResponseEventClusterizerTMVAEventFile()
{
  // Construct an instance of MResponseEventClusterizerTMVAEventFile
  
  m_ResponseNameSuffix = "eventclusterizer.tmva";
  
  m_MaxNHits = 15;
  m_MaxNGroups = 3;
}


////////////////////////////////////////////////////////////////////////////////


MResponseEventClusterizerTMVAEventFile::~MResponseEventClusterizerTMVAEventFile()
{
  // Delete this instance of MResponseEventClusterizerTMVAEventFile
}


////////////////////////////////////////////////////////////////////////////////


//! Return a brief description of this response class
MString MResponseEventClusterizerTMVAEventFile::Description()
{
  return MString("create ROOT event files for TMVA algorithms for event clustering");
}


////////////////////////////////////////////////////////////////////////////////


//! Return information on the parsable options for this response class
MString MResponseEventClusterizerTMVAEventFile::Options()
{
  ostringstream out;
  out<<"             maxhits:     the maximum number of hits (default: 15)"<<endl;
  out<<"             maxgroups:   the maximum number of groups (default: 3)"<<endl;
  
  return MString(out);
}


////////////////////////////////////////////////////////////////////////////////


//! Parse the options
bool MResponseEventClusterizerTMVAEventFile::ParseOptions(const MString& Options)
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
    
    if (Split2[i][0] == "maxgroups") {
      m_MaxNGroups = stoi(Value);
    } else if (Split2[i][0] == "maxhits") {
      m_MaxNHits = stoi(Value);
    } else {    
      mout<<"Error: Unrecognized option "<<Split2[i][0]<<endl;
      return false;
    }
  }
  
  // Sanity checks:
  if (m_MaxNHits <= 0) {
    mout<<"Error: The maximum number of hits must be larger than zero (probably at least around 10...)"<<endl;
    return false;    
  }
  if (m_MaxNGroups <= 0) {
    mout<<"Error: The maximum number of groups must be larger than zero (probably at least 3...)"<<endl;
    return false;    
  }
  
  // Dump it for user info
  mout<<endl;
  mout<<"Choosen options for creating TMVA event clusterizer files:"<<endl;
  mout<<"  Maximum number of hits:       "<<m_MaxNHits<<endl;
  mout<<"  Maximum number of groups:     "<<m_MaxNGroups<<endl;
  mout<<endl;
  
  return true;
}



////////////////////////////////////////////////////////////////////////////////


//! Initialize the response matrices and their generation
bool MResponseEventClusterizerTMVAEventFile::Initialize() 
{ 
  // Initialize next matching event, save if necessary
  if (MResponseBuilder::Initialize() == false) return false;
  
  // We ignore the loaded configuration file
  // TODO: Missing event clusterize None as soon as implemented
  m_ReReader->SetHitClusteringAlgorithm(MRawEventAnalyzer::c_HitClusteringAlgoNone);
  m_ReReader->SetTrackingAlgorithm(MRawEventAnalyzer::c_TrackingAlgoNone);
  m_ReReader->SetCSRAlgorithm(MRawEventAnalyzer::c_CSRAlgoNone);
  m_ReReader->SetDecayAlgorithm(MRawEventAnalyzer::c_DecayAlgoNone);  
  // Start pre-analysis
  if (m_ReReader->PreAnalysis() == false) return false;
  
  // Create the ROOT trees for the TMVA analysis
  m_DataSets = vector<MEREventClusterizerDataSet*>(m_MaxNHits);
  m_Files = vector<TFile*>(m_MaxNHits, nullptr);
  m_Trees = vector<TTree*>(m_MaxNHits, nullptr);
  
  for (unsigned int x = 0; x < m_MaxNHits; ++x) {
    // Create the file
    TFile* File = new TFile(m_ResponseName + ".maxhits" + (x+1) + ".eventclusterizer.root", "recreate");
    m_Files[x] = File;
    File->cd();
    
    MEREventClusterizerDataSet* DS = new MEREventClusterizerDataSet();
    DS->Initialize(x+1, m_MaxNGroups);
    MString Name("EventClusterizer");
    m_Trees[x] = DS->CreateTree(Name);
    m_DataSets[x] = DS;
  }

  // We only can save once at the end, thus:
  m_SaveAfter = numeric_limits<unsigned long>::max();
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseEventClusterizerTMVAEventFile::Save()
{
  // Nothing here, only save at the end
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseEventClusterizerTMVAEventFile::Analyze()
{
  // Initialize the next matching event, save if necessary
  if (MResponseBuilder::Analyze() == false) return false;
  
  // We need to have at least an "INIT" in the simulation file per event 
  if (m_SiEvent->GetNIAs() == 0) {
    return true;
  }

  // We just require the initial raw event
  MRERawEvent* RE = m_ReReader->GetInitialRawEvent();
  if (RE == nullptr) {
    return true;
  }

  // Enforce the maximum we will store
  if ((unsigned int) RE->GetNRESEs() > m_MaxNHits) {
    return true; 
  }
  
  vector<MRESE*> RESEs;
  for (int r = 0; r < RE->GetNRESEs(); ++r) {
    RESEs.push_back(RE->GetRESEAt(r)); 
  }
  Shuffle(RESEs);
  
  // Create all origin IDs
  vector<vector<int>> OriginIDs;
  for (unsigned int r = 0; r < RESEs.size(); ++r) {
    OriginIDs.push_back(GetOriginIds(RESEs[r]));
  }
  
  vector<vector<int>> MotherIDs;
  for (unsigned int r = 0; r < RESEs.size(); ++r) {
    MotherIDs.push_back(GetMotherIds(OriginIDs[r]));
  }


  // Now translate to 1 ... m_MaxNHits
  map<int, int> Translation;
  int Highest = 0;
  for (unsigned int r = 0; r < RESEs.size(); ++r) {
    for (auto O: MotherIDs[r]) {
      if (Translation[O] == 0) {
        Translation[O] = ++Highest;
      }
    }
  }
  
  // Now randomize:
  vector<int> Randomizer(Highest, -1);
  for (unsigned int r = 0; r < Randomizer.size(); ++r) {
    int R = 0;
    do {
      R = gRandom->Integer(Highest);
    } while (find(Randomizer.begin(), Randomizer.end(), R) != Randomizer.end());
    
    Randomizer[r] = R;
  }
  
  for (auto I = Translation.begin(); I != Translation.end(); ++I) {
    (*I).second = Randomizer[(*I).second - 1];
  }
  
  // Create the result data set
  vector<vector<int>> HitOriginIDs(RESEs.size());
  for (unsigned int r = 0; r < RESEs.size(); ++r) {
    for (auto O: MotherIDs[r]) {
      HitOriginIDs[r].push_back(Translation[O]);
    }
  }
  
  /*
  // Dump the origin IDs:
  if (Randomizer.size() == 4) {
    cout<<"ID: "<<m_SiEvent->GetID()<<endl;
    for (unsigned int r = 0; r < RESEs.size(); ++r) {
      cout<<RESEs[r]->ToString()<<endl;
      cout<<"  Origins: ";
      for (auto O: OriginIDs[r]) {
        cout<<O<<" "; 
      }
      cout<<endl;
      cout<<"  Mothers: ";
      for (auto O: MotherIDs[r]) {
        cout<<O<<" "; 
      }
      cout<<endl;
      cout<<"  Randomized, translated IDs: ";
      for (auto O: MotherIDs[r]) {
        cout<<Translation[O]<<" "; 
      }
      cout<<endl;
    }
  }
  */
  
  // Set the data
  m_DataSets[RESEs.size() - 1]->FillEventData(m_SiEvent->GetID(), RESEs);
  m_DataSets[RESEs.size() - 1]->FillResultData(HitOriginIDs);
  
  // Store the data
  m_Trees[RESEs.size() - 1]->Fill();
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Finalize the response generation (i.e. save the data a final time )
bool MResponseEventClusterizerTMVAEventFile::Finalize() 
{ 
  // We only save at the end since ROOT has trouble updating the file...
  for (unsigned int x = 0; x < m_MaxNHits; ++x) {
    //TFile Tree(m_ResponseName + ".maxhits" + (x+1) + ".eventclusterizer.root", "recreate");
    //Tree.cd();
    //m_Trees[x]->Write();
    //Tree.Close();
    
    m_Trees[x]->Write();
    m_Files[x]->Close();  
  }
  
  return MResponseBuilder::Finalize(); 
}


// MResponseEventClusterizerTMVAEventFile.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
