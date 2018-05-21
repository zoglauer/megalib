/*
 * MResponseEventQualityTMVAEventFile.cxx
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
// MResponseEventQualityTMVAEventFile
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MResponseEventQualityTMVAEventFile.h"

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


#ifdef ___CLING___
ClassImp(MResponseEventQualityTMVAEventFile)
#endif


////////////////////////////////////////////////////////////////////////////////


MResponseEventQualityTMVAEventFile::MResponseEventQualityTMVAEventFile()
{
  // Construct an instance of MResponseEventQualityTMVAEventFile
  
  m_ResponseNameSuffix = "quality.tmva";
  
  m_DoAbsorptions = true;
  m_MaxAbsorptions = 10;
  
  m_MaxEnergyDifference = 5; // keV
  m_MaxEnergyDifferencePercent = 0.02;
  
  m_MaxTrackEnergyDifference = 30; // keV
  m_MaxTrackEnergyDifferencePercent = 0.1;
  
  // We can save much more frequently here, since the files are a lot smaller
  m_SaveAfter = numeric_limits<long>::max();
  
  m_UsePathToFirstIA = true;
}


////////////////////////////////////////////////////////////////////////////////


MResponseEventQualityTMVAEventFile::~MResponseEventQualityTMVAEventFile()
{
  // Delete this instance of MResponseEventQualityTMVAEventFile
}


////////////////////////////////////////////////////////////////////////////////


//! Return a brief description of this response class
MString MResponseEventQualityTMVAEventFile::Description()
{
  return MString("create ROOT event files for TMVA algorithms to determine event quality");
}


////////////////////////////////////////////////////////////////////////////////


//! Return information on the parsable options for this response class
MString MResponseEventQualityTMVAEventFile::Options()
{
  ostringstream out;
  out<<"             initial:   use the path to the initial IA (default: true)"<<endl;
  
  return MString(out);
}


////////////////////////////////////////////////////////////////////////////////


//! Parse the options
bool MResponseEventQualityTMVAEventFile::ParseOptions(const MString& Options)
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
    
    if (Split2[i][0] == "initial") {
      if (Value == "true") {
        m_UsePathToFirstIA = true;
      } else if (Value == "false") {
        m_UsePathToFirstIA = false;
      } else {
        mout<<"Error: Unrecognized value ("<<Value<<") for option "<<Value<<endl;
      }
    } else {
      mout<<"Error: Unrecognized option "<<Split2[i][0]<<endl;
      return false;
    }
  }
  
  // Dump it for user info
  mout<<endl;
  mout<<"Choosen options for creating TMVA quality data files:"<<endl;
  mout<<"  Path to first IA:          "<<(m_UsePathToFirstIA == true ? "true" : "false")<<endl;
  mout<<endl;
  
  return true;
}



////////////////////////////////////////////////////////////////////////////////


//! Initialize the response matrices and their generation
bool MResponseEventQualityTMVAEventFile::Initialize() 
{ 
  // Initialize next matching event, save if necessary
  if (MResponseBuilder::Initialize() == false) return false;
  
  if (m_ReReader->PreAnalysis() == false) return false;
  
  
  // Build permutation matrix:
  m_Permutator.resize(m_MaxNInteractions+1);
  for (unsigned int i = 2; i <= (unsigned int) m_MaxNInteractions; ++i) {
    vector<vector<unsigned int>> Permutations;
    vector<unsigned int> Indices;
    for (unsigned int j = 0; j < i; ++j) {
      Indices.push_back(j);
    }
    vector<unsigned int>::iterator Begin = Indices.begin();
    vector<unsigned int>::iterator End = Indices.end();
    do {
      Permutations.push_back(Indices);
    } while (next_permutation(Begin, End));
    m_Permutator[i] = Permutations;
  }
  
  
  // Create the ROOT trees for the TMVA analysis
  for (unsigned int s = 2; s <= m_MaxNInteractions; ++s) {
    MERQualityDataSet* DS = new MERQualityDataSet();
    DS->Initialize(s, m_UsePathToFirstIA);
    MString Name("Quality_seq");
    Name += s;
    m_Trees.push_back(DS->CreateTree(Name));
    m_DataSets.push_back(DS);
  }
  
  m_SaveAfter = numeric_limits<unsigned long>::max();
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseEventQualityTMVAEventFile::Save()
{
  // Nothing here, only save at the end
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseEventQualityTMVAEventFile::Analyze()
{
  // Create the multiple Compton response
  
  // Initialize the next matching event, save if necessary
  if (MResponseBuilder::Analyze() == false) return false;
  
  // We need to have at least an "INIT" in the simulation file per event 
  if (m_SiEvent->GetNIAs() == 0) {
    return true;
  }
  
  // We require a successful reconstruction 
  MRawEventList* REList = m_ReReader->GetRawEventList();
  if (REList->HasOptimumEvent() == false) {
    return true;
  }
  
  // ... leading to an event
  MPhysicalEvent* Event = REList->GetOptimumEvent()->GetPhysicalEvent();
  if (Event == nullptr) {
    return true;
  }
  
  // ... which needs to be a Compton event
  if (Event->GetType() != MPhysicalEvent::c_Compton) {
    return true;
  }
  
  // ... that passed the event selections
  if (m_MimrecEventSelector.IsQualifiedEvent(Event) == false) {
    return true; 
  }
  
  // ... and has a good kinematics
  MComptonEvent* Compton = (MComptonEvent*) Event;
  if (Compton->IsKinematicsOK() == false) {
    return true;
  }
  
  
  // Go ahead event by event and compare the results: 
  MRERawEvent* RE = REList->GetOptimumEvent();
  vector<MRESE*> RESEs;
  
  
  // Current event parameters
  vector<MRESE*> SequencedRESEs;
  vector<int> SequencedRESEsIDs;
  bool StartResolved = false;
  bool CompletelyAbsorbed = false;
  
  unsigned int SequenceLength = (unsigned int) RE->GetNRESEs();
  
  mdebug<<"Start (ID: "<<RE->GetEventID()<<") with n="<<SequenceLength<<endl;
  
  // Step 1:
  // Some initial selections:
  if (SequenceLength > (unsigned int) m_MaxNInteractions) {
    mdebug<<"CreateResponse: Too many hits: "<<SequenceLength<<endl;
    return true;
  }
  
  if (RE->GetRejectionReason() == MRERawEvent::c_RejectionTotalEnergyOutOfLimits ||
      RE->GetRejectionReason() == MRERawEvent::c_RejectionLeverArmOutOfLimits ||
      RE->GetRejectionReason() == MRERawEvent::c_RejectionEventIdOutOfLimits) {
    mdebug<<"CreateResponse: Event did not pass event selections"<<endl;
    return true;
  }
  
  if (g_Verbosity >= c_Chatty) { 
    mout<<"Matched event (Sim ID: "<<RE->GetEventID()<<")"<<endl;
    mout<<RE->ToString()<<endl;
  }
  
  // Step 2:
  // Analyze the current event
  RESEs.clear();
  
  MRESEIterator I(RE->GetStartPoint()); // Start is the start point of the sequence
  MRESE* R = nullptr;
  while ((R = I.GetNextRESE()) != nullptr) { 
    RESEs.push_back(R); 
  }
  SequencedRESEs.clear();
  SequencedRESEs.resize(RESEs.size());
  
  StartResolved = FindCorrectSequence(RESEs, SequencedRESEs);
  
  // If we don't have sequenced RESEs at that point something went badly wrong with the event, so we skip it here
  bool FoundNullPtr = false;
  for (unsigned int i = 0; i < SequenceLength; ++i) {
    if (SequencedRESEs[i] == nullptr) {
      FoundNullPtr = true;
      break;
    }
  }
  if (FoundNullPtr == true) {
    //mout<<"Event cannot be sequenced correctly (Sim ID: "<<RE->GetEventID()<<")"<<endl;
    SequencedRESEs = RESEs;
  }
  
  
  CompletelyAbsorbed = AreCompletelyAbsorbed(RESEs, RE);
  if (StartResolved == true && CompletelyAbsorbed == true) {
    if (g_Verbosity >= c_Chatty) mout<<" --> Good event!"<<endl;
  } else {
    if (g_Verbosity >= c_Chatty) mout<<" --> Bad event: completely aborbed: "<<(CompletelyAbsorbed ? "true" : "false")<<"  resolved: "<<(StartResolved ? "true" : "false")<<endl;
  }        
  
  // Now fill the data set
  m_DataSets[SequenceLength-2]->FillEventData(RE->GetEventID(), RESEs, m_SiGeometry);
  m_DataSets[SequenceLength-2]->FillEvaluationIsCompletelyAborbed(CompletelyAbsorbed);
  m_DataSets[SequenceLength-2]->FillEvaluationIsReconstructable(StartResolved);
  m_DataSets[SequenceLength-2]->FillEvaluationZenithAngle(m_SiEvent->GetIAAt(0)->GetDirection().Theta()*c_Deg);
  
  m_Trees[SequenceLength-2]->Fill();
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Finalize the response generation (i.e. save the data a final time )
bool MResponseEventQualityTMVAEventFile::Finalize() 
{ 
  // We only save at the end since ROOT has trouble updating the file...
  for (unsigned int t = 0; t < m_Trees.size(); ++t) {
    TFile Tree(m_ResponseName + ".seq" + (t+2) + ".quality.root", "recreate");
    Tree.cd();
    m_Trees[t]->Write();
    Tree.Close();
  }
  
  return MResponseBuilder::Finalize(); 
}


// MResponseEventQualityTMVAEventFile.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
