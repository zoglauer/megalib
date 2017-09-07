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
ClassImp(MResponseMultipleComptonTMVA)
#endif


////////////////////////////////////////////////////////////////////////////////


MResponseMultipleComptonTMVA::MResponseMultipleComptonTMVA()
{
  // Construct an instance of MResponseMultipleComptonTMVA
  
  m_ResponseNameSuffix = "tmva";
  
  m_DoAbsorptions = true;
  m_MaxAbsorptions = 10;
  m_CSRMaxLength = 10;
  
  m_MaxEnergyDifference = 5; // keV
  m_MaxEnergyDifferencePercent = 0.02;
  
  m_MaxTrackEnergyDifference = 30; // keV
  m_MaxTrackEnergyDifferencePercent = 0.1;
  
  // We can save much more frequently here, since the files are a lot smaller
  m_SaveAfter = numeric_limits<long>::max();
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
  // Initialize next matching event, save if necessary
  if (MResponseBuilder::Initialize() == false) return false;
  
  // We are not doing any sequencing or decay detection!
  m_ReReader->SetCSRAlgorithm(MRawEventAnalyzer::c_CSRAlgoNone);
  m_ReReader->SetDecayAlgorithm(MRawEventAnalyzer::c_DecayAlgoNone);
  if (m_ReReader->PreAnalysis() == false) return false;
  
  
  // Build permutation matrix:
  m_Permutator.resize(m_CSRMaxLength+1);
  for (unsigned int i = 2; i <= (unsigned int) m_CSRMaxLength; ++i) {
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
  
  m_CSRMaxLength = 4;
  cout<<"Fixing m_CSRMaxLength = "<<m_CSRMaxLength<<endl;
  
  // Round 1: Initialize the vectors - push_back, etc. may lead to an reallocation and invalidate pointers,
  //          thus we have to do it first
  for (int c = 0; c <= m_CSRMaxLength-2; ++c) {
    int l = c+2;
    
    m_SimulationIDs.push_back(0);
    
    m_Energies.push_back(vector<double>(l));
    
    m_PositionsX.push_back(vector<double>(l));
    m_PositionsY.push_back(vector<double>(l));
    m_PositionsZ.push_back(vector<double>(l));
    
    m_ComptonScatterAngles.push_back(vector<double>(l-1));
    m_KleinNishinaProbability.push_back(vector<double>(l-1));
    
    if (l > 2) {
      m_ComptonScatterAngleDifference.push_back(vector<double>(l-2));
    }

    m_AbsorptionProbabilities.push_back(vector<double>(l-1));
    m_AbsorptionProbabilityToFirstIAAverage.push_back(0);
    m_AbsorptionProbabilityToFirstIAMaximum.push_back(0);
    m_AbsorptionProbabilityToFirstIAMinimum.push_back(0);
    m_ZenithAngle.push_back(0);
    m_NadirAngle.push_back(0);
  }
  
  // Round 2: Create the trees
  for (int c = 0; c <= m_CSRMaxLength-2; ++c) {
    int l = c+2;
    
    TTree* Good = new TTree("Good", "Good Compton ER tree"); //"ComptonTMVA", "ComptonTMVA");
    TTree* Bad = new TTree("Bad", "Bad Compton ER tree"); //"ComptonTMVA", "ComptonTMVA");
    
    MString Name("SimulationIDs");
    Good->Branch(Name, &m_SimulationIDs[c], Name + "/L");
    Bad->Branch(Name, &m_SimulationIDs[c], Name + "/L");
    
    for (unsigned int i = 0; i < m_Energies[c].size(); ++i) {
      Name = "Energy";
      Name += i+1;
      Good->Branch(Name, &m_Energies[c][i], Name + "/D");
      Bad->Branch(Name, &m_Energies[c][i], Name + "/D");
    }
    
    for (unsigned int i = 0; i < m_PositionsX[c].size(); ++i) {
      Name = "X";
      Name += i+1;
      Good->Branch(Name, &m_PositionsX[c][i], Name + "/D");
      Bad->Branch(Name, &m_PositionsX[c][i], Name + "/D");
    }
    for (unsigned int i = 0; i < m_PositionsY[c].size(); ++i) {
      Name = "Y";
      Name += i+1;
      Good->Branch(Name, &m_PositionsY[c][i], Name + "/D");
      Bad->Branch(Name, &m_PositionsY[c][i], Name + "/D");
    }
    for (unsigned int i = 0; i < m_PositionsZ[c].size(); ++i) {
      Name = "Z";
      Name += i+1;
      Good->Branch(Name, &m_PositionsZ[c][i], Name + "/D");
      Bad->Branch(Name, &m_PositionsZ[c][i], Name + "/D");
    }
    
    for (unsigned int i = 0; i < m_ComptonScatterAngles[c].size(); ++i) {
      Name = "ComptonScatterAngle";
      Name += i+1;
      Good->Branch(Name, &m_ComptonScatterAngles[c][i], Name + "/D");
      Bad->Branch(Name, &m_ComptonScatterAngles[c][i], Name + "/D");
    }
    
    for (unsigned int i = 0; i < m_KleinNishinaProbability[c].size(); ++i) {
      Name = "KleinNishinaProbability";
      Name += i+1;
      Good->Branch(Name, &m_KleinNishinaProbability[c][i], Name + "/D");
      Bad->Branch(Name, &m_KleinNishinaProbability[c][i], Name + "/D");
    }
    
    if (l > 2) {
      for (unsigned int i = 0; i < m_ComptonScatterAngleDifference[c-1].size(); ++i) { // "-1" since we only start at 3 interactions
        Name = "ComptonScatterAngleDifference";
        Name += i+1;
        Good->Branch(Name, &m_ComptonScatterAngleDifference[c-1][i], Name + "/D");
        Bad->Branch(Name, &m_ComptonScatterAngleDifference[c-1][i], Name + "/D");
      }
    }
    
    for (unsigned int i = 0; i < m_AbsorptionProbabilities[c].size(); ++i) {
      Name = "AbsorptionProbabilities";
      Name += i+1;
      Good->Branch(Name, &m_AbsorptionProbabilities[c][i], Name + "/D");
      Bad->Branch(Name, &m_AbsorptionProbabilities[c][i], Name + "/D");
    }
    
    Name = "AbsorptionProbabilityToFirstIAAverage";
    Good->Branch(Name, &m_AbsorptionProbabilityToFirstIAAverage[c], Name + "/D");
    Bad->Branch(Name, &m_AbsorptionProbabilityToFirstIAAverage[c], Name + "/D");
    
    Name = "AbsorptionProbabilityToFirstIAMaximum";
    Good->Branch(Name, &m_AbsorptionProbabilityToFirstIAMaximum[c], Name + "/D");
    Bad->Branch(Name, &m_AbsorptionProbabilityToFirstIAMaximum[c], Name + "/D");
    
    Name = "AbsorptionProbabilityToFirstIAMinimum";
    Good->Branch(Name, &m_AbsorptionProbabilityToFirstIAMinimum[c], Name + "/D");
    Bad->Branch(Name, &m_AbsorptionProbabilityToFirstIAMinimum[c], Name + "/D");
    
    Name = "ZenithAngle";
    Good->Branch(Name, &m_ZenithAngle[c], Name + "/D");
    Bad->Branch(Name, &m_ZenithAngle[c], Name + "/D");
    
    Name = "NadirAngle";
    Good->Branch(Name, &m_NadirAngle[c], Name + "/D");
    Bad->Branch(Name, &m_NadirAngle[c], Name + "/D");
    
    m_TreeGood.push_back(Good);
    m_TreeBad.push_back(Bad);
  }
  

  m_SaveAfter = numeric_limits<unsigned long>::max();
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseMultipleComptonTMVA::Save()
{
  // Nothing here, only save at the end
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseMultipleComptonTMVA::Analyze()
{
  // Create the multiple Compton response
  
  // Initlize next matching event, save if necessary
  if (MResponseBuilder::Analyze() == false) return false;
  
  // Go ahead event by event and compare the results: 
  MRERawEvent* RE = nullptr;
  MRawEventList* REList = m_ReReader->GetRawEventList();
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
    
    //cout<<"Start (ID: "<<RE->GetEventID()<<")"<<endl;
    
    // Check if complete sequence is ok:
    SequenceLength = (unsigned int) RE->GetNRESEs();
    
    // Step 1:
    // Some initial selections:
    if (SequenceLength <= 1) {
      mdebug<<"CreateResponse: Not enough hits: "<<SequenceLength<<endl;
      continue;
    }
    
    if (SequenceLength > (unsigned int) m_CSRMaxLength) {
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
      if ( SequencedRESEs[i] == nullptr) {
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
      
      m_SimulationIDs[SequenceLength-2] = RE->GetEventID();
      
      // (a) Raw data:
      for (unsigned int r = 0; r < SequenceLength; ++r) {
        m_Energies[SequenceLength-2][r] = SequencedRESEs[m_Permutator[SequenceLength][p][r]]->GetEnergy();
        m_PositionsX[SequenceLength-2][r] = SequencedRESEs[m_Permutator[SequenceLength][p][r]]->GetPosition().X();
        m_PositionsY[SequenceLength-2][r] = SequencedRESEs[m_Permutator[SequenceLength][p][r]]->GetPosition().Y();
        m_PositionsZ[SequenceLength-2][r] = SequencedRESEs[m_Permutator[SequenceLength][p][r]]->GetPosition().Z();
      }
      
      // (b) Compton scatter angle & Klein-Nishina
      double EnergyIncomingGamma = RE->GetEnergy();
      double EnergyElectron = 0.0;
      double Phi = 0.0;
      for (unsigned int r = 0; r < SequenceLength-1; ++r) {
        EnergyElectron = SequencedRESEs[m_Permutator[SequenceLength][p][r]]->GetEnergy();
        Phi = MComptonEvent::ComputePhiViaEeEg(EnergyElectron, EnergyIncomingGamma - EnergyElectron);
        m_ComptonScatterAngles[SequenceLength-2][r] = Phi*c_Deg;
        m_KleinNishinaProbability[SequenceLength-2][r] = MComptonEvent::GetKleinNishinaNormalizedByArea(EnergyIncomingGamma, Phi);
        EnergyIncomingGamma -= EnergyElectron;
        //if (p == 0 && StartResolved == true && CompletelyAbsorbed == true && Phi*c_Deg > 179.99) {
        //  cout<<"Large Compton scatter angle (Sim ID: "<<RE->GetEventID()<<") -- Start:"<<SequencedRESEs[m_Permutator[SequenceLength][p][0]]->GetEnergy()<<endl;
        //  mout<<RE->ToString()<<endl;
        //}
      }
      
      // (c) Compton scatter angle difference
      for (unsigned int r = 0; r < SequenceLength-2; ++r) {
        // Via Angle:
        MVector FirstDir = SequencedRESEs[m_Permutator[SequenceLength][p][r+1]]->GetPosition() - SequencedRESEs[m_Permutator[SequenceLength][p][r]]->GetPosition();
        MVector SecondDir = SequencedRESEs[m_Permutator[SequenceLength][p][r+2]]->GetPosition() - SequencedRESEs[m_Permutator[SequenceLength][p][r+1]]->GetPosition();
        double PhiGeo = FirstDir.Angle(SecondDir);
        m_ComptonScatterAngleDifference[SequenceLength-3][r] = PhiGeo*c_Deg - m_ComptonScatterAngles[SequenceLength-2][r+1]; // "-3" since we ponly start for 3-site events
      }
      
      // (d) Absorption probabilities
      EnergyIncomingGamma = RE->GetEnergy();
      for (unsigned int r = 0; r < SequenceLength-1; ++r) {
        EnergyIncomingGamma -= SequencedRESEs[m_Permutator[SequenceLength][p][r]]->GetEnergy();
        
        m_AbsorptionProbabilities[SequenceLength-2][r] = CalculateAbsorptionProbabilityTotal(*SequencedRESEs[m_Permutator[SequenceLength][p][r]], *SequencedRESEs[m_Permutator[SequenceLength][p][r+1]], EnergyIncomingGamma);
      }
      
      // (e) Incoming probabilities
      EnergyIncomingGamma = RE->GetEnergy();
      Phi = m_ComptonScatterAngles[SequenceLength-2][0]*c_Rad;
      MVector FirstIAPos = SequencedRESEs[m_Permutator[SequenceLength][p][0]]->GetPosition();
      MVector SecondIAPos = SequencedRESEs[m_Permutator[SequenceLength][p][1]]->GetPosition();
      MVector FirstScatteredGammaRayDir = SecondIAPos - FirstIAPos;
      // Create a vector orthogonal to FirstScatteredGammaRayDir which we can use to create the first direction on the cone
      MVector Ortho = FirstScatteredGammaRayDir.Orthogonal();
      // Create the first direction on the cone by rotating FirstScatteredGammaRayDir by Phi around Ortho
      MVector Incoming = FirstScatteredGammaRayDir;
      Incoming.RotateAroundVector(Ortho, Phi);
      
      m_AbsorptionProbabilityToFirstIAAverage[SequenceLength-2] = 0.0;
      m_AbsorptionProbabilityToFirstIAMaximum[SequenceLength-2] = 0.0;
      m_AbsorptionProbabilityToFirstIAMinimum[SequenceLength-2] = numeric_limits<double>::max();
      unsigned int Steps = 36;
      double StepWidth = c_TwoPi/Steps;
      for (unsigned int a = 0; a < Steps; ++a) {
        MVector Outgoing = -Incoming;
        Outgoing.RotateAroundVector(FirstScatteredGammaRayDir, a*StepWidth);
        Outgoing.Unitize();
        double P = m_SiGeometry->GetComptonAbsorptionProbability(FirstIAPos + 1000000*Outgoing, FirstIAPos, EnergyIncomingGamma);
        m_AbsorptionProbabilityToFirstIAAverage[SequenceLength-2] += P;
        if (P > m_AbsorptionProbabilityToFirstIAMaximum[SequenceLength-2]) {
          m_AbsorptionProbabilityToFirstIAMaximum[SequenceLength-2] = P;
        }
        if (P < m_AbsorptionProbabilityToFirstIAMinimum[SequenceLength-2]) {
          m_AbsorptionProbabilityToFirstIAMinimum[SequenceLength-2] = P;
        }
      }
      m_AbsorptionProbabilityToFirstIAAverage[SequenceLength-2] /= Steps;
      
      // (f) Zenith and Nadir angles
      MVector Zenith(0, 0, 1);
      m_ZenithAngle[SequenceLength-2] = (FirstIAPos - SecondIAPos).Angle(Zenith - FirstIAPos) - Phi;
      MVector Nadir(0, 0, -1);
      m_NadirAngle[SequenceLength-2] = (FirstIAPos - SecondIAPos).Angle(Nadir - FirstIAPos) - Phi;
      
      if (p == 0 && StartResolved == true && CompletelyAbsorbed == true) {
        //cout<<"Add good"<<endl;
        m_TreeGood[SequenceLength-2]->Fill();
        //m_TreeGood[SequenceLength-2]->Show();
      } else {
        //cout<<"Add bad"<<endl;
        m_TreeBad[SequenceLength-2]->Fill();          
        //m_TreeBad[SequenceLength-2]->Show();
      }
    } // all permutations
  } // All raw events

  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Finalize the response generation (i.e. save the data a final time )
bool MResponseMultipleComptonTMVA::Finalize() 
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
    
    //m_TreeGood[t]->SaveAs(GetFilePrefix() + ".seq" + (t+2) + ".good.root");
    //m_TreeBad[t]->SaveAs(GetFilePrefix() + ".seq" + (t+2) + ".bad.root");
  }
  
  return MResponseBuilder::Finalize(); 
}


////////////////////////////////////////////////////////////////////////////////


void MResponseMultipleComptonTMVA::Teach()
{
  // Teach the neural network the events
  
 
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseMultipleComptonTMVA::AreCompletelyAbsorbed(const vector<MRESE*>& RESEs, MRERawEvent* RE)
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


bool MResponseMultipleComptonTMVA::FindFirstInteractions(const vector<MRESE*>& RESEs, MRESE*& First, MRESE*& Second)
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


bool MResponseMultipleComptonTMVA::FindCorrectSequence(const vector<MRESE*>& RESEs, vector<MRESE*>& SequencedRESEs)
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
  
  // Check if in all but the last interaction only contain a Compton interaction and it's possible dependents:
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
  
  mout<<"Sequence: ";
  for (unsigned int s = 0; s < SequencedRESEs.size(); ++s) {
    if (SequencedRESEs[s] == nullptr) {
      mout<<" ?";
    } else {
      mout<<" "<<SequencedRESEs[s]->GetID();
    }
  }
  mout<<endl;
  
  for (unsigned int s = 0; s < SequencedRESEs.size(); ++s) {
    if (SequencedRESEs[s] == nullptr) {
      if (g_Verbosity >= c_Chatty) mout<<"FindCorrectSequence: Did not find complete sequence!"<<endl;
      return false;
    }
  }
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


unsigned int MResponseMultipleComptonTMVA::NumberOfComptonInteractions(vector<int> AllSimIds)
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


void MResponseMultipleComptonTMVA::Shuffle(vector<MRESE*>& RESEs)
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


// MResponseMultipleComptonTMVA.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
