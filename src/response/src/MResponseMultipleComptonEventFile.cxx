/*
 * MResponseMultipleComptonEventFile.cxx
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
// MResponseMultipleComptonEventFile
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MResponseMultipleComptonEventFile.h"

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
ClassImp(MResponseMultipleComptonEventFile)
#endif


////////////////////////////////////////////////////////////////////////////////


MResponseMultipleComptonEventFile::MResponseMultipleComptonEventFile()
{
  // Construct an instance of MResponseMultipleComptonEventFile
  
  m_ResponseNameSuffix = "tmva";
  
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


MResponseMultipleComptonEventFile::~MResponseMultipleComptonEventFile()
{
  // Delete this instance of MResponseMultipleComptonEventFile
}


////////////////////////////////////////////////////////////////////////////////


//! Return a brief description of this response class
MString MResponseMultipleComptonEventFile::Description()
{
  return MString("create ROOT good/bad event files for TMVA methods");
}


////////////////////////////////////////////////////////////////////////////////


//! Return information on the parsable options for this response class
MString MResponseMultipleComptonEventFile::Options()
{
  ostringstream out;
  out<<"             initial:   use the path to the initial IA (default: true)"<<endl;
  
  return MString(out);
}


////////////////////////////////////////////////////////////////////////////////


//! Parse the options
bool MResponseMultipleComptonEventFile::ParseOptions(const MString& Options)
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
  mout<<"Choosen options for creating TMVA data files:"<<endl;
  mout<<"  Path to first IA:          "<<(m_UsePathToFirstIA == true ? "true" : "false")<<endl;
  mout<<endl;
  
  return true;
}



////////////////////////////////////////////////////////////////////////////////


//! Initialize the response matrices and their generation
bool MResponseMultipleComptonEventFile::Initialize() 
{ 
  // Initialize next matching event, save if necessary
  if (MResponseBuilder::Initialize() == false) return false;
  
  // We are not doing any sequencing or decay detection!
  m_ReReader->SetCSRAlgorithm(MRawEventAnalyzer::c_CSRAlgoNone);
  m_ReReader->SetDecayAlgorithm(MRawEventAnalyzer::c_DecayAlgoNone);
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
  //   // Verify:
  //   for (unsigned int i = 2; i < m_Permutator.size(); ++i) {
  //     cout<<"Size: "<<i<<endl;
  //     for (unsigned int j = 0; j < m_Permutator[i].size(); ++j) {
  //       cout<<"  Permutation "<<j<<"/"<<m_Permutator[i].size()<<": "<<endl;
  //       for (unsigned int k = 0; k < m_Permutator[i][j].size(); ++k) {
  //         cout<<m_Permutator[i][j][k]<<" ";
  //       }
  //       cout<<endl;
  //     }
  //   }  
  
  // Create the ROOT tree for TMVA analysis  
  m_DS.Initialize(m_MaxNInteractions, m_UsePathToFirstIA);
  
  m_DS.CreateTrees(m_TreeGood, m_TreeBad);
  
  m_SaveAfter = numeric_limits<unsigned long>::max();
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseMultipleComptonEventFile::Save()
{
  // Nothing here, only save at the end
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseMultipleComptonEventFile::Analyze()
{
  // Create the multiple Compton response
  
  // Initlize next matching event, save if necessary
  if (MResponseBuilder::Analyze() == false) return false;
  
  // Go ahead event by event and compare the results: 
  MRERawEvent* RE = nullptr;
  MRawEventIncarnations* REList = m_ReReader->GetRawEventList();
  vector<MRESE*> RESEs;
  
  
  // Current event parameters
  vector<MRESE*> SequencedRESEs;
  vector<int> SequencedRESEsIDs;
  bool StartResolved = false;
  bool CompletelyAbsorbed = false;
  
  unsigned int SequenceLength = 0;
  
  
  int r_max = REList->GetNRawEvents();
  for (int r = 0; r < r_max; ++r) {
    RE = REList->GetRawEventAt(r);
        
    // Check if complete sequence is ok:
    SequenceLength = (unsigned int) RE->GetNRESEs();

    mdebug<<"Start (ID: "<<RE->GetEventID()<<") with n="<<SequenceLength<<endl;
    
    // Step 1:
    // Some initial selections:
    if (SequenceLength <= 1) {
      mdebug<<"CreateResponse: Not enough hits: "<<SequenceLength<<endl;
      continue;
    }
    
    if (SequenceLength > (unsigned int) m_MaxNInteractions) {
      mdebug<<"CreateResponse: Too many hits: "<<SequenceLength<<endl;
      continue;
    }
    
    if (RE->GetRejectionReason() == MRERawEvent::c_RejectionTotalEnergyOutOfLimits ||
        RE->GetRejectionReason() == MRERawEvent::c_RejectionLeverArmOutOfLimits ||
        RE->GetRejectionReason() == MRERawEvent::c_RejectionEventIdOutOfLimits) {
      mdebug<<"CreateResponse: Event did not pass event selections"<<endl;
      continue;
    }
    
    if (g_Verbosity >= c_Chatty) { 
      mout<<"Matched event (Sim ID: "<<RE->GetEventID()<<")"<<endl;
      mout<<RE->ToString()<<endl;
    }
    
    
    // Step 2:
    // Analyze the current event
    RESEs.clear();
    for (int i = 0; i < RE->GetNRESEs(); ++i) RESEs.push_back(RE->GetRESEAt(i));
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
      //continue;
    }        
    
    
    // Build a tree element for all permutations:
    for (unsigned int p = 0; p < m_Permutator[SequenceLength].size(); ++p) {
      
      vector<MRESE*> RESEs;
      for (unsigned int r = 0; r < SequenceLength; ++r) {
        RESEs.push_back(SequencedRESEs[m_Permutator[SequenceLength][p][r]]);
      }
      
      m_DS.Fill(RE->GetEventID(), RESEs, m_SiGeometry);
      
      if (p == 0 && StartResolved == true && CompletelyAbsorbed == true) {
        mdebug<<"Add good sequence (ID: "<<RE->GetEventID()<<")"<<endl;
        m_TreeGood[SequenceLength-2]->Fill();
        //m_TreeGood[SequenceLength-2]->Show();
      } else {
        mdebug<<"Add bad sequence (ID: "<<RE->GetEventID()<<")"<<endl;
        m_TreeBad[SequenceLength-2]->Fill();          
        //m_TreeBad[SequenceLength-2]->Show();
      }
    } // all permutations
  } // All raw events

  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Finalize the response generation (i.e. save the data a final time )
bool MResponseMultipleComptonEventFile::Finalize() 
{ 
  // We only save at the end since ROOT has trouble updating the file...
  for (unsigned int t = 0; t < m_TreeGood.size(); ++t) {
    TFile GoodOne(GetFilePrefix() + ".seq" + (t+2) + ".good.root", "recreate");
    GoodOne.cd();
    //m_TreeGood[t]->Print();
    m_TreeGood[t]->Write();
    //m_TreeGood[t]->Show(20);
    GoodOne.Close();
    
    TFile BadOne(GetFilePrefix() + ".seq" + (t+2) + ".bad.root", "recreate");
    BadOne.cd();
    m_TreeBad[t]->Write();
    BadOne.Close();
  }
  
  return MResponseBuilder::Finalize(); 
}


////////////////////////////////////////////////////////////////////////////////


void MResponseMultipleComptonEventFile::Teach()
{
  // Teach the neural network the events
  
 
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseMultipleComptonEventFile::AreCompletelyAbsorbed(const vector<MRESE*>& RESEs, MRERawEvent* RE)
{
  // Return true if ALL individual RESEs are completely absorbed
  // AND the photon from the start of the event!
    
  vector<int> AllOriginIds;
  
  for (unsigned int i = 0; i < RESEs.size(); ++i) {
    vector<int> OriginIds = GetOriginIds(RESEs[i]);
    if (IsAbsorbed(OriginIds, RESEs[i]->GetEnergy(), RESEs[i]->GetEnergyResolution()) == false) {
      if (g_Verbosity >= c_Chatty) mout<<"RESE "<<RESEs[i]->GetID()<<" is not completely absorbed!"<<endl;
      return false;
    }
    for (unsigned int j = 0; j < OriginIds.size(); ++j) {
      if (find(AllOriginIds.begin(), AllOriginIds.end(), OriginIds[j]) == AllOriginIds.end()) {
        AllOriginIds.push_back(OriginIds[j]);
      }
    }
  }
  
  if (IsTotalAbsorbed(AllOriginIds, RE->GetEnergy(), RE->GetEnergyResolution()) == false) {
    if (g_Verbosity >= c_Chatty) mout<<"Whole event "<<RE->GetID()<<" is not completely absorbed!"<<endl;
    return false;
  }
  
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseMultipleComptonEventFile::FindFirstInteractions(const vector<MRESE*>& RESEs, MRESE*& First, MRESE*& Second)
{
  // Return true if those were found
  // 
  // First and second are the *detected* first and second interaction ---
  // there may be some undetected before and in between
  
  //mout<<"FindFirstInteractions"<<endl;
  
  First = 0;
  Second = 0;
  
  // Determine for all RESEs the Origin IDs
  vector<vector<int>> OriginIds;
  for (unsigned int r = 0; r < RESEs.size(); ++r) {
    OriginIds.push_back(GetOriginIds(RESEs[r]));
  }
  
  // Motherparticle of smallest ID - needs to be a photon!
  int Smallest = numeric_limits<int>::max();
  for (unsigned int i = 0; i < OriginIds.size(); ++i) {
    for (unsigned int j = 0; j < OriginIds[i].size(); ++j) {
      if (OriginIds[i][j] < Smallest) Smallest = OriginIds[i][j];
    }
  }
  
  if (m_SiEvent->GetIAAt(Smallest-1)->GetOriginID() == 0) {
    if (m_SiEvent->GetIAAt(Smallest-1)->GetProcess() != "ANNI" &&
      m_SiEvent->GetIAAt(Smallest-1)->GetProcess() != "INIT") {
      if (g_Verbosity >= c_Chatty) mout<<"FindFirstInteractions: IA Type not OK: "<<m_SiEvent->GetIAAt(Smallest-1)->GetProcess()<<endl;
      return false;
    }
  } else {
    if (m_SiEvent->GetIAAt(m_SiEvent->GetIAAt(Smallest-1)->GetOriginID()-1)->GetProcess() != "ANNI" &&
      m_SiEvent->GetIAAt(m_SiEvent->GetIAAt(Smallest-1)->GetOriginID()-1)->GetProcess() != "INIT") {
      if (g_Verbosity >= c_Chatty) mout<<"FindFirstInteractions: IA Type not OK: "<<m_SiEvent->GetIAAt(m_SiEvent->GetIAAt(Smallest-1)->GetOriginID()-1)->GetProcess()<<endl;
      return false;
    }
  }
  
  if (m_SiEvent->GetIAAt(m_SiEvent->GetIAAt(Smallest-1)->GetOriginID()-1)->GetSecondaryParticleID() == 1) {
    // Find one and only RESE associated with this:
    for (unsigned int i = 0; i < OriginIds.size(); ++i) {
      for (unsigned int j = 0; j < OriginIds[i].size(); ++j) {
        if (OriginIds[i][j] == Smallest) {
          if (First == 0) {
            First = RESEs[i];
          } else {
            // If we have more than one RESE associated with this, then we have no clear first hit 
            // and have to reject...
            if (g_Verbosity >= c_Chatty) mout<<"FindFirstInteractions: First: RESE "<<First->GetID()<<" and "<<RESEs[i]->GetID()<<" are associated with start IA "<<Smallest<<endl;
            return false;
          }
        }
      }
    }
  } else {
    // Only photons can be good...
    if (g_Verbosity >= c_Chatty) mout<<"FindFirstInteractions: IA which triggered first RESE is no photon: "<<m_SiEvent->GetIAAt(m_SiEvent->GetIAAt(Smallest-1)->GetOriginID()-1)->GetSecondaryParticleID()<<endl;
    return false;
  }
  
  // Go through the main event tree in the file and find the RESEs:
  for (unsigned int s = Smallest+1; s < m_SiEvent->GetNIAs(); ++s) {
    // The second event in the sequence needs to have the same origin than the first one
    if (m_SiEvent->GetIAAt(Smallest)->GetOriginID() == m_SiEvent->GetIAAt(s)->GetOriginID()) {
      for (unsigned int i = 0; i < OriginIds.size(); ++i) {
        for (unsigned int j = 0; j < OriginIds[i].size(); ++j) {
          if (OriginIds[i][j] == int(s) && RESEs[i] != First) {
            if (Second == 0) {
              Second = RESEs[i];
            } else {
              // If we have more than one RESE associated with this, then we have no clear first hit 
              // and have to reject...
              if (g_Verbosity >= c_Chatty) mout<<"FindFirstInteractions: Second: RESE "<<Second->GetID()<<" and "<<RESEs[i]->GetID()<<" are associated with current IA "<<i<<endl;
              return false;
            }
          }
        }
      }
      if (Second != 0) break;
    }
  }
  
  if (First == 0 || Second == 0) {
    if (g_Verbosity >= c_Chatty) mout<<"FindFirstInteractions: Did not find first and second!"<<endl;
    return false;
  }
  
  //cout<<"First RESE: "<<First->GetID()<<", Second RESE: "<<Second->GetID()<<endl;
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseMultipleComptonEventFile::FindCorrectSequence(const vector<MRESE*>& RESEs, vector<MRESE*>& SequencedRESEs)
{
  // RESEs: The unsorted input RESEs
  // SequencedRESEs: The sorted output RESEs
  //
  // Returns true if a good sequence was found -- only interaction 1 & 2 count

  
  // Create the output array and initialize it with nullptr's
  SequencedRESEs.resize(RESEs.size());
  for (unsigned int i = 0; i < SequencedRESEs.size(); ++i) SequencedRESEs[i] = nullptr;
  
  // Determine for all RESEs the Origin IDs
  vector<vector<int>> OriginIds;
  for (unsigned int r = 0; r < RESEs.size(); ++r) {
    OriginIds.push_back(GetOriginIds(RESEs[r]));
  }
  
  // Mother particle corresponding to smallest ID --- force requirement that it is a photon?

  // Find the smallest origin ID in the sequence
  int Smallest = numeric_limits<int>::max();
  for (unsigned int i = 0; i < OriginIds.size(); ++i) {
    for (unsigned int j = 0; j < OriginIds[i].size(); ++j) {
      if (OriginIds[i][j] < Smallest) Smallest = OriginIds[i][j];
    }
  }
  
  // Check if the start of the sequence is a good one
  // What about BREM & RAYL & COMP, or even PHOT? Everything can give rise to a good sequence... It just needs to produce a photon...
  if (m_SiEvent->GetIAAt(Smallest-1)->GetOriginID() == 0) { // Why can this happen... this would be init
    if (m_SiEvent->GetIAAt(Smallest-1)->GetProcess() != "ANNI" && m_SiEvent->GetIAAt(Smallest-1)->GetProcess() != "INIT") {
      if (g_Verbosity >= c_Chatty) mout<<"FindCorrectSequence: Interaction type of sequence start not OK: "<<m_SiEvent->GetIAAt(Smallest-1)->GetProcess()<<endl;
      return false;
    }
  } else {
    if (m_SiEvent->GetIAAt(m_SiEvent->GetIAAt(Smallest-1)->GetOriginID()-1)->GetProcess() != "ANNI" &&
        m_SiEvent->GetIAAt(m_SiEvent->GetIAAt(Smallest-1)->GetOriginID()-1)->GetProcess() != "INIT") {
      if (g_Verbosity >= c_Chatty) mout<<"FindCorrectSequence: Interaction type of sequence not OK: "<<m_SiEvent->GetIAAt(m_SiEvent->GetIAAt(Smallest-1)->GetOriginID()-1)->GetProcess()<<endl;
      return false;
    }
  }
  
  // The start particle must be or create a photon
  if (m_SiEvent->GetIAAt(m_SiEvent->GetIAAt(Smallest-1)->GetOriginID()-1)->GetSecondaryParticleID() == 1) { // Bug: RAYL should be OK as start!
    // Find one and only RESE associated with this:
    for (unsigned int i = 0; i < OriginIds.size(); ++i) {
      for (unsigned int j = 0; j < OriginIds[i].size(); ++j) {
        if (OriginIds[i][j] == Smallest) {
          if (SequencedRESEs[0] == nullptr) {
            SequencedRESEs[0] = RESEs[i];
          } else {
            // If we have more than one RESE associated with this, then we have no clear first hit 
            // and have to reject...
            if (g_Verbosity >= c_Chatty) mout<<"FindCorrectSequence: First: RESE "<<SequencedRESEs[0]->GetID()<<" and "<<RESEs[i]->GetID()<<" are associated with start IA "<<Smallest<<endl;
            return false;
          }
        }
      }
    }
  } else {
    // Only photons can be good...
    if (g_Verbosity >= c_Chatty) mout<<"FindCorrectSequence: IA which triggered first RESE is no photon: "<<m_SiEvent->GetIAAt(m_SiEvent->GetIAAt(Smallest-1)->GetOriginID()-1)->GetSecondaryParticleID()<<endl;
    return false;
  }
  
  //mout<<"FindCorrectSequence: First RESE "<<SequencedRESEs[0]->GetID()<<" (Smallest: "<<Smallest<<")"<<endl;
  
  // Check if there is a Compton is missing, RAYL is OK!
  for (unsigned int s = Smallest; s <= m_SiEvent->GetNIAs(); ++s) {
    // The second event in the sequence needs to have the same origin than the first one
    if (m_SiEvent->GetIAAt(Smallest-1)->GetOriginID() == m_SiEvent->GetIAAt(s-1)->GetOriginID() && m_SiEvent->GetIAAt(s-1)->GetProcess() == "COMP") {
      //cout<<m_SiEvent->GetIAAt(s)->GetEnergy()<<endl;
      bool Found = false;
      for (unsigned int i = 0; i < OriginIds.size(); ++i) {
        for (unsigned int j = 0; j < OriginIds[i].size(); ++j) {
          //cout<<int(s+1)<<":"<<OriginIds[i][j]<<endl;
          if (OriginIds[i][j] == int(s)) {
            Found = true;
            break;
          }
        }
      }
      if (Found == false) {
        if (g_Verbosity >= c_Chatty) mout<<"FindCorrectSequence: Gap! Cannot find representant for Compton ID "<<s<<endl;
        return false;
      }
    }
  }
  
  // Go through the main event tree in the file and find the RESE sequence:
  unsigned int FreeSpotInSequencedRESEs = 1;
  for (unsigned int s = Smallest; s <= m_SiEvent->GetNIAs(); ++s) {  // We need equal here, since s is the ID 
    // The second event in the sequence needs to have the same origin than the first one
    if (m_SiEvent->GetIAAt(Smallest-1)->GetOriginID() == m_SiEvent->GetIAAt(s-1)->GetOriginID()) {
      // Loop over all RESEs
      for (unsigned int r = 0; r < RESEs.size(); ++r) {
        // If the current RESE is not yet in the list:
        if (find(SequencedRESEs.begin(), SequencedRESEs.end(), RESEs[r]) == SequencedRESEs.end()) {
          // Loop over its origins...
          for (unsigned int j = 0; j < OriginIds[r].size(); ++j) {
            // If one of the origins corresponds to the IA
            if (OriginIds[r][j] == int(s)) {
              SequencedRESEs[FreeSpotInSequencedRESEs] = RESEs[r];
              //mout<<"FindCorrectSequence: next RESE "<<SequencedRESEs[FreeSpotInSequencedRESEs]->GetID()<<endl;
              FreeSpotInSequencedRESEs++;
            }
          }
        }
      }
    }
  }
  
  // Check if all but the last interaction only contain a Compton interaction and it's possible dependents:
  for (unsigned int s = 0; s < SequencedRESEs.size()-1; ++s) {
    if (SequencedRESEs[s] != nullptr) {
      vector<int> SequencedRESEsIDs = GetOriginIds(SequencedRESEs[s]);
      if (ContainsOnlyComptonDependants(SequencedRESEsIDs) == false) {
        if (g_Verbosity >= c_Chatty) mout<<"FindCorrectSequence: Not only Compton (+ dependents) in interaction "<<SequencedRESEs[s]->GetID()<<endl;
        return false;
      }
      if (NumberOfComptonInteractions(SequencedRESEsIDs) != 1) {  // Bug: Comp -> Brms -> Comp must be OK!
        if (g_Verbosity >= c_Chatty) mout<<"FindCorrectSequence: Not exactly one Compton in interaction "<<SequencedRESEs[s]->GetID()<<endl;
        return false;        
      }
      //mout<<"FindCorrectSequence: All Compton "<<SequencedRESEs[s]->GetID()<<endl;
    }
  }
  
  /*
  mout<<"Sequence: ";
  for (unsigned int s = 0; s < SequencedRESEs.size(); ++s) {
    if (SequencedRESEs[s] == nullptr) {
      mout<<" ?";
    } else {
      mout<<" "<<SequencedRESEs[s]->GetID();
    }
  }
  mout<<endl;
  */
  
  for (unsigned int s = 0; s < SequencedRESEs.size(); ++s) {
    if (SequencedRESEs[s] == nullptr) {
      if (g_Verbosity >= c_Chatty) mout<<"FindCorrectSequence: Did not find complete sequence!"<<endl;
      return false;
    }
  }
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


unsigned int MResponseMultipleComptonEventFile::NumberOfComptonInteractions(vector<int> AllSimIds)
{
  unsigned int N = 0;
  
  for (unsigned int i = 0; i < AllSimIds.size(); ++i) {
    if (m_SiEvent->GetIAAt(AllSimIds[i]-1)->GetProcess() == "COMP") {
      N++;
    }
  }
  
  return N;
}


////////////////////////////////////////////////////////////////////////////////


void MResponseMultipleComptonEventFile::Shuffle(vector<MRESE*>& RESEs)
{
  //! Shuffle the RESEs around...
  
  unsigned int size = RESEs.size();
  for (unsigned int i = 0; i < 2*size; ++i) {
    unsigned int From = gRandom->Integer(size);
    unsigned int To = gRandom->Integer(size);
    MRESE* Temp = RESEs[To];
    RESEs[To] = RESEs[From];
    RESEs[From] = Temp;
  }
}


// MResponseMultipleComptonEventFile.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
