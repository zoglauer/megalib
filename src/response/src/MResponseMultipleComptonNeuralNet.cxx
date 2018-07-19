/*
 * MResponseMultipleComptonNeuralNet.cxx
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
// MResponseMultipleComptonNeuralNet
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MResponseMultipleComptonNeuralNet.h"

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
#include "MERCSRBayesian.h"
#include "MNeuron.h"
#include "MInputNeuron.h"
#include "MBiasNeuron.h"
#include "MFeedForwardNeuron.h"
#include "MAdalineNeuron.h"
#include "MBackpropagationNeuron.h"
#include "MBackpropagationMiddleNeuron.h"
#include "MBackpropagationOutputNeuron.h"
#include "MSynapse.h"
#include "MAdalineSynapse.h"
#include "MBackpropagationSynapse.h"
#include "MNeuralNetworkBackpropagation.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MResponseMultipleComptonNeuralNet)
#endif


////////////////////////////////////////////////////////////////////////////////


MResponseMultipleComptonNeuralNet::MResponseMultipleComptonNeuralNet()
{
  // Construct an instance of MResponseMultipleComptonNeuralNet
  
  m_ResponseNameSuffix = "nn";
  
  m_DoAbsorptions = true;
  m_MaxAbsorptions = 10;
  m_MaxNInteractions = 10;
  
  mimp<<"Is this stuff still needed????"<<show;
  m_MaxEnergyDifference = 5; // keV
  m_MaxEnergyDifferencePercent = 0.02;
  
  m_MaxTrackEnergyDifference = 30; // keV
  m_MaxTrackEnergyDifferencePercent = 0.1;
  
  // We can save much more frequently here, since the files are a lot smaller
  m_SaveAfter = 10000;
  
  m_UseRawData = true;
  m_UseDistances = true;
  m_UseInteractionProbabilities = true;
  m_UseComptonScatterProbabilities = true;
  m_UseComptonScatterAngles = true;
  m_UseDPhiCriterion = true;
  
  //   m_UseRawData = false;
  //   m_UseDistances = false;
  //   m_UseInteractionProbabilities = false;
  //   m_UseComptonScatterProbabilities = false;
  //   m_UseComptonScatterAngles = false;
  //   m_UseDPhiCriterion = true;
}


////////////////////////////////////////////////////////////////////////////////


MResponseMultipleComptonNeuralNet::~MResponseMultipleComptonNeuralNet()
{
  // Delete this instance of MResponseMultipleComptonNeuralNet
}


////////////////////////////////////////////////////////////////////////////////


//! Initialize the response matrices and their generation
bool MResponseMultipleComptonNeuralNet::Initialize() 
{ 
  // Initialize next matching event, save if necessary
  if (MResponseBuilder::Initialize() == false) return false;
  
  // We are not doing any sequencing or decay detection!
  m_ReReader->SetCSRAlgorithm(MRawEventAnalyzer::c_CSRAlgoNone);
  m_ReReader->SetDecayAlgorithm(MRawEventAnalyzer::c_DecayAlgoNone);
  if (m_ReReader->PreAnalysis() == false) return false;
  
  
  // Create the neural net 
  
  int f = 0;
  
  m_MaxNInteractions = 3;
  mimp<<"Fixing m_MaxNInteractions = "<<m_MaxNInteractions<<show;
  
  // Information needs to go into base file! 
  m_EnergyMin.push_back(0.0);
  //   m_EnergyMax.push_back(130.0);
  //   m_EnergyMin.push_back(130.0);
  // m_EnergyMax.push_back(350.0);
  // m_EnergyMin.push_back(350.0);
  // m_EnergyMax.push_back(850.0);
  // m_EnergyMin.push_back(850.0);
  m_EnergyMax.push_back(1500.0);
  //   _EnergyMin.push_back(1500.0);
  //   m_EnergyMax.push_back(3500.0);
  //   m_EnergyMin.push_back(3500.0);
  //   m_EnergyMax.push_back(10000.0);
  
  // Determine normalizations: All x, y, z and E-values have to be between 0.2 and 0.8
  // Positions are normalized linearly, while Energies are normalized logarithmically
  m_xMin = -10.0;
  m_xInt =  20.0;
  m_yMin = -10.0;
  m_yInt = 20.0;
  m_zMin =  -6.0;
  m_zInt =  8.0;
  m_MaximumDistance = sqrt(m_xInt*m_xInt + m_yInt*m_yInt + m_zInt*m_zInt);
  
  unsigned int TotalInputDoubles = 0;
  double MiddleNodeScaler = 1.0;
  
  m_SequenceNNs.resize(m_EnergyMin.size());
  m_SequenceNNIOStore.resize(m_EnergyMin.size());
  m_SequenceBestVerificationDataRatio.resize(m_EnergyMin.size());
  for (unsigned int e = 0; e < m_EnergyMin.size(); ++e) {
    m_SequenceNNs[e].resize(m_MaxNInteractions+1);
    m_SequenceNNIOStore[e].resize(m_MaxNInteractions+1);
    m_SequenceBestVerificationDataRatio[e].resize(m_MaxNInteractions+1);
    for (unsigned int i = 2; i <= (unsigned int) m_MaxNInteractions; ++i) {
      
      //m_SequenceNNs[e][i].SetNNeuralNetworks(7);
      
      // Determine number of input nodes:
      unsigned int NInputNodes = 0;
      f = int(TMath::Factorial(i));
      if (m_UseRawData == true) {
        // i * (x,y,z,E)              = 4 * i
        NInputNodes += 4*i;
      }
      if (m_UseDistances == true) {
        // TODO: We actually have less than that...
        // all distances              = (i-1) * i!
        NInputNodes += (i-1)*f;        
      }
      if (m_UseInteractionProbabilities == true) {
        // all interaction prob's     = (i-1) * i!
        NInputNodes += (i-1)*f;        
      }
      if (m_UseComptonScatterProbabilities == true) {
        // all interaction prob's     = (i-1) * i!
        NInputNodes += (i-1)*f;        
      }
      if (m_UseComptonScatterAngles == true) {
        // all cosphi via energy      = (i-1) * i!
        // all cosphi via geometry    = (i-2) * i!
        NInputNodes += (i-1)*f + (i-2)*f;        
      }
      if (m_UseDPhiCriterion == true) {
        // all dPhi's plus average
        if (i >= 3) {
          NInputNodes += (i-2)*f + f;
        }        
      }
      m_SequenceNNs[e][i].SetNInputNodes(NInputNodes);
      TotalInputDoubles += NInputNodes;
      
      // Best performance is reached for 24 middle nodes if a 3-site event is given
      if (i == 2) {
        MiddleNodeScaler = 8.0;
      } else if (i == 3) {
        MiddleNodeScaler = 8.0;
      } else if (i == 4) {
        MiddleNodeScaler = 4.0;        
      } else {
        MiddleNodeScaler = 2.0;        
      }
      
      // X x (input nodes + output nodes)
      m_SequenceNNs[e][i].SetNMiddleNodes(int(MiddleNodeScaler*f));
      
      // 2 + all possible first two combinations
      m_SequenceNNs[e][i].SetNOutputNodes(f);
      
      m_SequenceNNs[e][i].SetLearningRate(0.2);
      m_SequenceNNs[e][i].SetMomentum(0.9);
      m_SequenceNNs[e][i].SetNUserComments(4);
      m_SequenceNNs[e][i].Create();
      
      m_SequenceBestVerificationDataRatio[e][i] = 0.0;
    }
  }
  
  m_QualityNNs.resize(m_EnergyMin.size());
  m_QualityNNIOStore.resize(m_EnergyMin.size());
  m_QualityBestVerificationDataRatio.resize(m_EnergyMin.size());
  for (unsigned int e = 0; e < m_EnergyMin.size(); ++e) {
    m_QualityNNs[e].resize(m_MaxNInteractions+1);
    m_QualityNNIOStore[e].resize(m_MaxNInteractions+1);
    m_QualityBestVerificationDataRatio[e].resize(m_MaxNInteractions+1);
    for (unsigned int i = 2; i <= (unsigned int) m_MaxNInteractions; ++i) {
      
      //m_QualityNNs[e][i].SetNNeuralNetworks(7); 
      
      // Determine number of input nodes:
      unsigned int NInputNodes = 0;
      f = int(TMath::Factorial(i));
      if (m_UseRawData == true) {
        // i * (x,y,z,E)              = 4 * i
        NInputNodes += 4*i;
      }
      if (m_UseDistances == true) {
        // all distances              = (i-1) * i!
        NInputNodes += (i-1)*f;        
      }
      if (m_UseInteractionProbabilities == true) {
        // all interaction prob's     = (i-1) * i!
        NInputNodes += (i-1)*f;        
      }
      if (m_UseComptonScatterAngles == true) {
        // all cosphi via energy      = (i-1) * i!
        // all cosphi via geometry    = (i-2) * i!
        NInputNodes += (i-1)*f + (i-2)*f;        
      }
      if (m_UseComptonScatterProbabilities == true) {
        // all interaction prob's     = (i-1) * i!
        NInputNodes += (i-1)*f;        
      }
      if (m_UseDPhiCriterion == true) {
        // all dPhi's plus average
        if (i >= 3) {
          NInputNodes += (i-2)*f + f;
        }
      }
      m_QualityNNs[e][i].SetNInputNodes(NInputNodes);
      
      // Best performance is reached for 24 middle nodes if a 3-site event is given
      if (i == 2) {
        MiddleNodeScaler = 4.0;
      } else if (i == 3) {
        MiddleNodeScaler = 4.0;
      } else if (i == 4) {
        MiddleNodeScaler = 1.0;        
      } else {
        MiddleNodeScaler = 2.0;        
      }
      
      m_QualityNNs[e][i].SetNMiddleNodes(int(MiddleNodeScaler*f));
      // only one single number - the smaller, the better the quality...
      
      m_QualityNNs[e][i].SetNOutputNodes(1);
      m_QualityNNs[e][i].SetLearningRate(0.2);
      m_QualityNNs[e][i].SetMomentum(0.9);
      m_QualityNNs[e][i].SetNUserComments(4);
      m_QualityNNs[e][i].Create();
    }
  }
  
  // Let's save the "best" one's - although the first one is bad
  Save();
  
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
  
  // Determine how many events to store:
  MSystem System;
  int RAM = System.GetFreeRAM();
  
  m_EventsToStore = RAM/(2*sizeof(double)*TotalInputDoubles);
  mout<<"Events to store per IOStore list (limited by RAM: "<<RAM<<"): "<<m_EventsToStore<<endl;
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseMultipleComptonNeuralNet::Save()
{
  MResponseBuilder::Save(); 

  ofstream fout;
  fout.open(GetFilePrefix() + ".erm");
  if (fout.is_open() == false) {
    mout<<"Unable to open file "<<m_ResponseName<<".nn.erm"<<endl;
    return false;
  }
  
  fout<<"# Neural network event reconstruction master file"<<endl;
  fout<<endl;
  fout<<"# DO NOT MANIPULATE THIS FILE!"<<endl;
  fout<<"# The read routine has no error checks!"<<endl;
  fout<<endl;
  fout<<"Version 1"<<endl;
  fout<<"Type erm"<<endl;
  fout<<endl;
  
  fout<<"# Energy intervals"<<endl;
  fout<<"MI ";
  for (unsigned int i = 0; i < m_EnergyMin.size(); ++i) fout<<m_EnergyMin[i]<<" ";
  fout<<endl;
  fout<<"MA ";
  for (unsigned int i = 0; i < m_EnergyMax.size(); ++i) fout<<m_EnergyMax[i]<<" ";
  fout<<endl;
  fout<<endl;
  
  fout<<"# Detector dimensions"<<endl;
  fout<<"DX "<<m_xMin<<" "<<m_xInt<<endl;
  fout<<"DY "<<m_yMin<<" "<<m_yInt<<endl;
  fout<<"DZ "<<m_zMin<<" "<<m_zInt<<endl;
  fout<<endl;
  
  fout<<"# Used dimensions"<<endl;
  fout<<"DI UseRawData "<<((m_UseRawData == true) ? "true" : "false")<<endl;
  fout<<"DI UseDistances "<<((m_UseDistances == true) ? "true" : "false")<<endl;
  fout<<"DI UseInteractionProbabilities "<<((m_UseInteractionProbabilities == true) ? "true" : "false")<<endl;
  fout<<"DI UseComptonScatterProbabilities "<<((m_UseComptonScatterProbabilities == true) ? "true" : "false")<<endl;
  fout<<"DI UseComptonScatterAngles "<<((m_UseComptonScatterAngles == true) ? "true" : "false")<<endl;
  fout<<"DI UseDPhiCriterion "<<((m_UseDPhiCriterion == true) ? "true" : "false")<<endl;
  fout<<endl;
  
  fout<<"# Networks"<<endl;
  bool Return = true;
  for (unsigned int e = 0; e < m_EnergyMin.size(); ++e) {
    for (unsigned int i = 2; i <= (unsigned int) m_MaxNInteractions; ++i) {
      ostringstream out;
      out<<GetFilePrefix()<<".e"<<e<<".s"<<i<<".seq.rsp";
      if (m_SequenceNNs[e][i].Save(out.str().c_str()) == false) {
        Return = false;
      }
      fout<<"NS "<<e<<" "<<i<<" "<<out.str()<<endl;
    }
  }
  for (unsigned int e = 0; e < m_EnergyMin.size(); ++e) {
    for (unsigned int i = 2; i <= (unsigned int) m_MaxNInteractions; ++i) {
      ostringstream out;
      out<<GetFilePrefix()<<".e"<<e<<".s"<<i<<".qual.rsp";
      if (m_QualityNNs[e][i].Save(out.str().c_str()) == false) {
        Return = false;
      }
      fout<<"NQ "<<e<<" "<<i<<" "<<out.str()<<endl;
    }
  }
  
  fout<<endl;
  fout<<"EN"<<endl;
  
  fout.close();
  
  return Return;
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseMultipleComptonNeuralNet::SaveMatrixSequenceNN(unsigned int e, unsigned int s, MString Flag)
{
  ostringstream out;
  out<<GetFilePrefix()<<Flag<<".e"<<e<<".s"<<s<<".seq.rsp";
  return m_SequenceNNs[e][s].Save(out.str().c_str());
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseMultipleComptonNeuralNet::SaveMatrixQualityNN(unsigned int e, unsigned int s, MString Flag)
{
  ostringstream out;
  out<<GetFilePrefix()<<Flag<<".e"<<e<<".s"<<s<<".qual.rsp";
  return m_QualityNNs[e][s].Save(out.str().c_str());
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseMultipleComptonNeuralNet::Analyze()
{
  // Create the multiple Compton response
  
  // How many teaching rounds per updated data set
  unsigned int NumberOfTeachingRounds = 5;
  
  
  // Go ahead event by event and compare the results: 
  vector<MRESE*> RESEs;
  
  vector<vector<int>> ClassicSequenceGood(m_EnergyMin.size());
  vector<vector<int>> ClassicSequenceBad(m_EnergyMin.size());
  
  for (unsigned int e = 0; e < m_EnergyMin.size(); ++e) {
    ClassicSequenceGood[e].resize(m_MaxNInteractions+1);
    ClassicSequenceBad[e].resize(m_MaxNInteractions+1);
  }
  
  // Current event parameters
  vector<MRESE*> Sorted;
  vector<int> SortedIDs;
  bool StartResolved = false;
  bool CompletelyAbsorbed = false;
  
  unsigned int SequenceBin = 0;
  unsigned int EnergyBin = 0;
  
  unsigned int SequenceLength = 0;
  
  int CounterToNextTeaching = 0;
  
  do {
    // Stay in an infinite event loop, i.e. re-read the sim file
    // Exit only on an error
    int InitializationTry = 0;
    while (InitializationTry < 2) {
      if (InitializeNextMatchingEvent() == false) {
        //         CloseSimulationFile();
        //         if (OpenSimulationFile() == false) {
        //           InitializationTry = 1;
        //         }
      } else {
        break;
      }
      InitializationTry++;
    }
    if (InitializationTry >= 2) {
      mout<<"CreateResponse: Error during reinitialization of infinite event loop"<<endl;
      break;
    }
    
    
    
    //g_DebugLevel = 1;
    
    for (auto RE: m_ReEvents) {
      if (RE == nullptr) continue;
      
      // Check if complete sequence is ok:
      SequenceLength = (unsigned int) RE->GetNRESEs();
      
      //if (SequenceLength !=3) continue;
      
      
      // Step 1:
      // Some initial selections:
      if (RE->GetEnergy() < m_EnergyMin.front() || RE->GetEnergy() > m_EnergyMax.back()) continue;
      
      if (SequenceLength <= 1) {
        mdebug<<"CreateResponse: Not enough hits: "<<SequenceLength<<endl;
        continue;
      }
      
      if (SequenceLength > (unsigned int) m_MaxNInteractions) {
        mdebug<<"CreateResponse: To many hits: "<<SequenceLength<<endl;
        continue;
      }
      
      if (g_Verbosity >= c_Chatty) { 
        mout<<"Matched event (Sim ID: "<<RE->GetEventId()<<")"<<endl;
        mout<<RE->ToString()<<endl;
      }
      
      //       m_SiEvent->SetEventNumber(++SimplyCounter);
      //       if (SequenceLength == 2) {
      //         Simply2<<m_SiEvent->ToSimString(24);
      //       }
      //       if (SequenceLength == 3) {
      //         Simply3<<m_SiEvent->ToSimString(24);
      //       }
      //       if (SequenceLength == 4) {
      //         Simply4<<m_SiEvent->ToSimString(24);
      //       }
      //       continue;
      
      // Step 2:
      // Analyze the current event
      RESEs.clear();
      for (int i = 0; i < RE->GetNRESEs(); ++i) RESEs.push_back(RE->GetRESEAt(i));
      Sorted.clear();
      Sorted.resize(RESEs.size());
      
      StartResolved = FindCorrectSequence(RESEs, Sorted);
      CompletelyAbsorbed = AreCompletelyAbsorbed(RESEs, RE);
      if (StartResolved == true && CompletelyAbsorbed == true) {
        if (g_Verbosity >= c_Chatty) mout<<" --> Good event!"<<endl;
      } else {
        if (g_Verbosity >= c_Chatty) mout<<" --> Bad event: completely aborbed: "<<(CompletelyAbsorbed ? "true" : "false")<<"  resolved: "<<(StartResolved ? "true" : "false")<<endl;
        //continue;
      }        
      
      // FindCorrectSequence is not working as expected thus ignore it for the moment...
      if (StartResolved == false) continue;
      
      //       // Store the event for later learning
      //       m_Events.push_back(RE->Duplicate());
      //       m_StartResolved.push_back(StartResolved);
      //       m_CompletelyAbsorbed.push_back(CompletelyAbsorbed);
      SortedIDs.clear();
      for (unsigned int i = 0; i < Sorted.size(); ++i) {
        if (Sorted[i] != 0) {
          SortedIDs.push_back(Sorted[i]->GetID());
        } else {
          SortedIDs.push_back(-1);
        }
      }
      //       m_SortedRESEIDs.push_back(SortedIDs);
      
      // Set all values to the NN
      
      //       // Step 1:
      //       // Grap the data:
      //       RE = *EventIterator;
      //       StartResolved = *StartResolvedIterator;
      //       CompletelyAbsorbed = *CompletelyAbsorbedIterator;
      //       SortedRESEIDs = *SortedIterator;
      
      //       RESEs.clear();
      //       for (int i = 0; i < RE->GetNRESEs(); ++i) RESEs.push_back(RE->GetRESEAt(i));
      
      SequenceBin = (unsigned int) RE->GetNRESEs();
      
      EnergyBin = numeric_limits<unsigned int>::max();
      for (unsigned int i = 0; i < m_EnergyMax.size(); ++i) {
        if (RE->GetEnergy() >= m_EnergyMin[i] && RE->GetEnergy() <= m_EnergyMax[i]) {
          EnergyBin = i;
          break;
        }
      }
      if (EnergyBin == numeric_limits<unsigned int>::max()) {
        mout<<"Energy not within NN limits: "<<RE->GetEnergy()<<endl;
        EnergyBin = m_EnergyMax.size() -1;
      }
      
      
      // Let's be annoying to the NN, sort the master RESEs randomly
      Shuffle(RESEs);
      
      if (g_Verbosity >= c_Chatty) {
        cout<<"Raw event "<<RE->GetEventId()<<endl;
        for (unsigned int re = 0; re < RESEs.size(); ++re) {
          cout<<re<<":"<<RESEs[re]->ToString();
        }
        for (unsigned int re = 0; re < SortedIDs.size(); ++re) {
          cout<<SortedIDs[re]<<" -> ";
        }
        cout<<endl;
        if (StartResolved == true && CompletelyAbsorbed == true) {
          cout<<"Good event"<<endl;
        } else {
          cout<<"Bad event"<<endl;
        }
      }
      
      
      // Step 2:
      // Teach the neural net:
      
      // Set the input:
      bool InputValid = true;
      unsigned int PosSequenceNN = 0;
      unsigned int PosQualityNN = 0;
      
      
      // (a) Raw data:
      if (m_UseRawData == true) {
        for (unsigned int r = 0; r < RESEs.size(); ++r) {
          double x = (RESEs[r]->GetPosition().X() - m_xMin)/m_xInt;
          double y = (RESEs[r]->GetPosition().Y() - m_yMin)/m_yInt;
          double z = (RESEs[r]->GetPosition().Z() - m_zMin)/m_zInt;
          double E = 0.1+0.8*(RESEs[r]->GetEnergy() - m_EnergyMin.front())/(m_EnergyMax[EnergyBin] - m_EnergyMin.front());
          
          if (m_SequenceNNs[EnergyBin][SequenceBin].SetInput(PosSequenceNN++, x) == false) InputValid = false;
          if (m_SequenceNNs[EnergyBin][SequenceBin].SetInput(PosSequenceNN++, y) == false) InputValid = false;
          if (m_SequenceNNs[EnergyBin][SequenceBin].SetInput(PosSequenceNN++, z) == false) InputValid = false;
          if (m_SequenceNNs[EnergyBin][SequenceBin].SetInput(PosSequenceNN++, E) == false) InputValid = false;
          if (m_QualityNNs[EnergyBin][SequenceBin].SetInput(PosQualityNN++, x) == false) InputValid = false;
          if (m_QualityNNs[EnergyBin][SequenceBin].SetInput(PosQualityNN++, y) == false) InputValid = false;
          if (m_QualityNNs[EnergyBin][SequenceBin].SetInput(PosQualityNN++, z) == false) InputValid = false;
          if (m_QualityNNs[EnergyBin][SequenceBin].SetInput(PosQualityNN++, E) == false) InputValid = false;
        }
      }
      
      // (b) distances:
      if (m_UseDistances == true) {
        for (unsigned int i = 0; i < m_Permutator[SequenceBin].size(); ++i) {
          for (unsigned int r = 0; r < SequenceBin-1; ++r) {
            double Distance = (RESEs[m_Permutator[SequenceBin][i][r]]->GetPosition() - RESEs[m_Permutator[SequenceBin][i][r+1]]->GetPosition()).Mag();
            
            if (g_Verbosity >= c_Chatty) {
              cout<<"Distance: S"<<i<<": ";
              for  (unsigned int re = 0; re < RESEs.size(); ++re) {
                cout<<RESEs[m_Permutator[SequenceBin][i][re]]->GetID();
                if (re < RESEs.size()-1) {
                  cout<<" -> ";
                } else {
                  cout<<" : ";
                }
              }
              cout<<" Distance="<<Distance;
            }
            // Normalize between 0.1 .. 0.9;
            Distance = 0.8*Distance/m_MaximumDistance+0.1;
            if (g_Verbosity >= c_Chatty) {
              if (g_Verbosity >= c_Chatty) cout<<" --> "<<Distance<<endl;
            }
            if (m_SequenceNNs[EnergyBin][SequenceBin].SetInput(PosSequenceNN++, Distance) == false) InputValid = false;
            if (m_QualityNNs[EnergyBin][SequenceBin].SetInput(PosQualityNN++, Distance) == false) InputValid = false;	  
          }
        }
      }
      
      // (c) absorption/scatter probabilities:
      if (m_UseInteractionProbabilities == true) {
        for (unsigned int i = 0; i < m_Permutator[SequenceBin].size(); ++i) {
          double Energy = RE->GetEnergy();
          for (unsigned int r = 0; r < SequenceBin-1; ++r) {
            double Prob = 0.5;
            Energy -= RESEs[m_Permutator[SequenceBin][i][r]]->GetEnergy();
            // For larger events, this is too time consuming
            //if (RESEs.size() <= 2) {
            if (r == SequenceBin-2) {
              Prob = CalculateAbsorptionProbabilityTotal(*RESEs[m_Permutator[SequenceBin][i][r]], *RESEs[m_Permutator[SequenceBin][i][r+1]], Energy);
            } else {
              Prob = CalculateAbsorptionProbabilityCompton(*RESEs[m_Permutator[SequenceBin][i][r]], *RESEs[m_Permutator[SequenceBin][i][r+1]], Energy);
            }
            //}
            
            
            if (g_Verbosity >= c_Chatty) {
              cout<<"Prob's: S"<<i<<": ";
              for  (unsigned int re = 0; re < RESEs.size(); ++re) {
                cout<<RESEs[m_Permutator[SequenceBin][i][re]]->GetID();
                if (re < RESEs.size()-1) {
                  cout<<" -> ";
                } else {
                  cout<<" : ";
                }
              }
              cout<<" Prob="<<Prob;
            }
            // Normalize between 0.1 .. 0.9;
            Prob = 0.8*Prob+0.1;
            if (g_Verbosity >= c_Chatty) {
              if (g_Verbosity >= c_Chatty) cout<<" --> "<<Prob<<endl;
            }
            if (m_SequenceNNs[EnergyBin][SequenceBin].SetInput(PosSequenceNN++, Prob) == false) InputValid = false;
            if (m_QualityNNs[EnergyBin][SequenceBin].SetInput(PosQualityNN++, Prob) == false) InputValid = false;	  
          }
        }
      }
      
      // (d) Compton scatter angles
      //     (+) phi's via energy:
      if (m_UseComptonScatterAngles == true) {
        for (unsigned int i = 0; i < m_Permutator[SequenceBin].size(); ++i) {
          double Energy = RE->GetEnergy();
          for (unsigned int r = 0; r < SequenceBin-1; ++r) {
            double CosPhi = CalculateCosPhiE(*RESEs[m_Permutator[SequenceBin][i][r]], Energy);
            Energy -= RESEs[m_Permutator[SequenceBin][i][r]]->GetEnergy();
            
            if (g_Verbosity >= c_Chatty) {
              cout<<"Phi's (kin): S"<<i<<": ";
              for  (unsigned int re = 0; re < RESEs.size(); ++re) {
                cout<<RESEs[m_Permutator[SequenceBin][i][re]]->GetID();
                if (re < RESEs.size()-1) {
                  cout<<" -> ";
                } else {
                  cout<<" : ";
                }
              }
              cout<<" cosPhi="<<CosPhi;
            }
            // Normalize between 0.1 .. 0.9;
            CosPhi = 0.2+0.6/2.0*(CosPhi+1);
            if (CosPhi > 0.9) CosPhi = 0.9;
            if (CosPhi < 0.1) CosPhi = 0.1;
            if (g_Verbosity >= c_Chatty) {
              if (g_Verbosity >= c_Chatty) cout<<" --> "<<CosPhi<<endl;
            }
            if (m_SequenceNNs[EnergyBin][SequenceBin].SetInput(PosSequenceNN++, CosPhi) == false) InputValid = false;
            if (m_QualityNNs[EnergyBin][SequenceBin].SetInput(PosQualityNN++, CosPhi) == false) InputValid = false;	  
          }
        }
        
        //    (+) phi's via geometry:
        for (unsigned int i = 0; i < m_Permutator[SequenceBin].size(); ++i) {
          for (unsigned int r = 1; r < SequenceBin-1; ++r) {
            double CosPhi = CalculateCosPhiG(*RESEs[m_Permutator[SequenceBin][i][r-1]], 
                                             *RESEs[m_Permutator[SequenceBin][i][r]], 
                                             *RESEs[m_Permutator[SequenceBin][i][r+1]]);
            
            if (g_Verbosity >= c_Chatty) {
              cout<<"Phi's (geo): S"<<i<<": ";
              for  (unsigned int re = 0; re < RESEs.size(); ++re) {
                cout<<RESEs[m_Permutator[SequenceBin][i][re]]->GetID();
                if (re < RESEs.size()-1) {
                  cout<<" -> ";
                } else {
                  cout<<" : ";
                }
              }
              cout<<" cosPhi="<<CosPhi;
            }
            // Normalize between 0.1 .. 0.9;
            CosPhi = 0.2+0.6/2.0*(CosPhi+1);
            if (CosPhi > 0.9) CosPhi = 0.9;
            if (CosPhi < 0.1) CosPhi = 0.1;
            if (g_Verbosity >= c_Chatty) {
              if (g_Verbosity >= c_Chatty) cout<<" --> "<<CosPhi<<endl;
            }
            if (m_SequenceNNs[EnergyBin][SequenceBin].SetInput(PosSequenceNN++, CosPhi) == false) InputValid = false;
            if (m_QualityNNs[EnergyBin][SequenceBin].SetInput(PosQualityNN++, CosPhi) == false) InputValid = false;	  
          }
        }
      }
      
      
      // (e) Compton scatter propabilities:
      if (m_UseComptonScatterProbabilities == true) {
        for (unsigned int i = 0; i < m_Permutator[SequenceBin].size(); ++i) {
          double Energy = RE->GetEnergy();
          for (unsigned int r = 0; r < SequenceBin-1; ++r) {
            double Phi = CalculatePhiEInDegree(*RESEs[m_Permutator[SequenceBin][i][r]], Energy);
            if (Phi < 0) Phi = 0;
            if (Phi > 180) Phi = 180;
            double Prob = MComptonEvent::GetKleinNishinaNormalizedByArea(Energy, Phi*c_Rad);
            
            Energy -= RESEs[m_Permutator[SequenceBin][i][r]]->GetEnergy();
            
            if (g_Verbosity >= c_Chatty) {
              cout<<"Compton prob's: S"<<i<<": ";
              for  (unsigned int re = 0; re < RESEs.size(); ++re) {
                cout<<RESEs[m_Permutator[SequenceBin][i][re]]->GetID();
                if (re < RESEs.size()-1) {
                  cout<<" -> ";
                } else {
                  cout<<" : ";
                }
              }
              cout<<" Prob="<<Prob;
            }
            // Normalize between 0.1 .. 0.9;
            Prob = 0.1 + 0.8*Prob;
            if (Prob > 0.9) Prob = 0.9;
            if (Prob < 0.1) Prob = 0.1;
            if (g_Verbosity >= c_Chatty) {
              if (g_Verbosity >= c_Chatty) cout<<" --> "<<Prob<<endl;
            }
            if (m_SequenceNNs[EnergyBin][SequenceBin].SetInput(PosSequenceNN++, Prob) == false) InputValid = false;
            if (m_QualityNNs[EnergyBin][SequenceBin].SetInput(PosQualityNN++, Prob) == false) InputValid = false;	  
          }
        }
      }
      
      
      // (f) dphi's:
      if (m_UseDPhiCriterion == true) {
        int BestSequence = 0;
        double BestSequenceValue = numeric_limits<double>::max();
        if (RESEs.size() >= 3) {
          for (unsigned int i = 0; i < m_Permutator[SequenceBin].size(); ++i) {
            double Energy = RE->GetEnergy();
            double AvgDPhi = 0.0;
            if (g_Verbosity >= c_Chatty) {
              cout<<"S"<<i<<": ";
              for  (unsigned int re = 0; re < RESEs.size(); ++re) {
                cout<<RESEs[m_Permutator[SequenceBin][i][re]]->GetID();
                if (re < RESEs.size()-1) {
                  cout<<" -> ";
                } else {
                  cout<<" : ";
                }
              }
            }
            for (unsigned int r = 1; r < SequenceBin-1; ++r) {
              Energy -= RESEs[m_Permutator[SequenceBin][i][r-1]]->GetEnergy();
              double DPhi = CalculateDCosPhi(*RESEs[m_Permutator[SequenceBin][i][r-1]], 
                                             *RESEs[m_Permutator[SequenceBin][i][r]], 
                                             *RESEs[m_Permutator[SequenceBin][i][r+1]], 
                                             Energy);
              if (g_Verbosity >= c_Chatty) {
                if (g_Verbosity >= c_Chatty) cout<<DPhi<<" - ";
              }
              AvgDPhi += DPhi*DPhi;
              
              DPhi = fabs(DPhi);
              //DPhi *= 0.75;
              //DPhi += 0.2;
              //if (DPhi > 0.8) DPhi = 0.8; 
              DPhi = 2*atan(DPhi)/c_Pi;
              if (g_Verbosity >= c_Chatty) {
                cout<<" - training value "<<r<<": "<<DPhi<<endl;
              }
              if (m_SequenceNNs[EnergyBin][SequenceBin].SetInput(PosSequenceNN++, DPhi) == false) InputValid = false;
              if (m_QualityNNs[EnergyBin][SequenceBin].SetInput(PosQualityNN++, DPhi) == false) InputValid = false;
            }
            AvgDPhi = fabs(sqrt(AvgDPhi));
            if (g_Verbosity >= c_Chatty) {
              cout<<" Avg: "<<AvgDPhi;
            }
            if (AvgDPhi < BestSequenceValue) {
              BestSequence = i;
              BestSequenceValue = AvgDPhi;
            }
            // Normalize for input into NN
            //AvgDPhi *= 0.75;
            //AvgDPhi += 0.2;
            //if (AvgDPhi > 0.8) AvgDPhi = 0.8; 
            AvgDPhi = 2*atan(AvgDPhi)/c_Pi;
            if (m_SequenceNNs[EnergyBin][SequenceBin].SetInput(PosSequenceNN++, AvgDPhi) == false) InputValid = false;
            if (m_QualityNNs[EnergyBin][SequenceBin].SetInput(PosQualityNN++, AvgDPhi) == false) InputValid = false;
            if (g_Verbosity >= c_Chatty) {
              cout<<" - training value: "<<AvgDPhi<<endl;
            }
          }
          // Check if the sequence with the lowest dPhi is the best sequence: 
          bool BestFindable = true;
          for (unsigned int i = 0; i <= 1; ++i) {
            if (RESEs[m_Permutator[SequenceBin][BestSequence][i]]->GetID() != SortedIDs[i]) {
              BestFindable = false;
              break;
            }
          }
          if (BestFindable == true) {
            ClassicSequenceGood[EnergyBin][SequenceBin]++;
            if (g_Verbosity >= c_Chatty) mout<<"Best findable: "<<BestSequence<<":"<<ClassicSequenceGood[EnergyBin][SequenceBin]<<endl;
          } else {
            ClassicSequenceBad[EnergyBin][SequenceBin]++;          
            if (g_Verbosity >= c_Chatty) mout<<"Best not findable: "<<BestSequence<<":"<<ClassicSequenceBad[EnergyBin][SequenceBin]<<endl;
          }
        }
      }
      
      // (d) Complain if there was an error
      if (InputValid == false) {
        mout<<"Input to neural network is not correctly normalized!"<<endl;
        massert(false);
        continue;
      }
      
      //cout<<m_SequenceNNs[EnergyBin][SequenceBin].GetIOStore().ToString()<<endl;
      
      // Determine which output neuron should indicate the correct sequence
      int CorrectSequence = 0; 
      if (StartResolved == true) {
        for (unsigned int i = 0; i < m_Permutator[SequenceBin].size(); ++i) {
          bool Found = true;
          for (unsigned int r = 0; r < RESEs.size(); ++r) {
            if (RESEs[m_Permutator[SequenceBin][i][r]]->GetID() != SortedIDs[r]) {
              Found = false;
              break;
            }
          }
          //cout<<RESEs[m_Permutator[SequenceBin][i][0]]->GetID()<<":"<<First->GetID()<<"   "<<RESEs[m_Permutator[SequenceBin][i][1]]->GetID()<<":"<<Second->GetID()<<endl;
          if (Found == true) {
            break;
          } else {
            CorrectSequence++;
          }
        }
      }
      if (g_Verbosity >= c_Chatty) cout<<"Sequence ID: "<<CorrectSequence<<" i.e. "<<CorrectSequence<<" output slot"<<endl;
      
      // Store the correct sequence
      
      
      // Set preliminary output values to retrieve the IOStores
      
      // Good quality means 0.25; bad = 0.75
      //cout<<"Completely absorbed: "<<(CompletelyAbsorbed ? "true" : "false")<<"  start resolved: "<<(StartResolved ? "true" : "false")<<endl;
      if (CompletelyAbsorbed == false || StartResolved == false) {
        m_QualityNNs[EnergyBin][SequenceBin].SetOutput(0, m_BadValue);
        if (g_Verbosity >= c_Chatty) {
          cout<<"Desired ouput: "<<m_BadValue<<endl;
        }
      } else {
        m_QualityNNs[EnergyBin][SequenceBin].SetOutput(0, m_GoodValue);
        if (g_Verbosity >= c_Chatty) {
          cout<<"Desired output: "<<m_GoodValue<<endl;
        }
      }
      
      // ... get IO store ...
      MNeuralNetworkIO IO = m_QualityNNs[EnergyBin][SequenceBin].GetIOStore();

      // Add additional data
      IO.AddUserValue("ID", RE->GetEventId());
      IO.AddUserValue("CompletelyAbsorbed", double(CompletelyAbsorbed));
      IO.AddUserValue("StartResolved", double(StartResolved));
      if (gRandom->Rndm() < 0.15) {
        IO.IsVerificationData(true);
      } else{
        IO.IsVerificationData(false);
      }
      
        // ... and store ...
      m_QualityNNIOStore[EnergyBin][SequenceBin].Add(IO);
      
      
      // We learn the sequence only if we do have a GOOD event!
      if (CompletelyAbsorbed == true && StartResolved == true) {
        for (int i = 0; i < TMath::Factorial(SequenceBin); ++i) {
          if (i != CorrectSequence) {
            m_SequenceNNs[EnergyBin][SequenceBin].SetOutput(i, m_BadValue);
            if (g_Verbosity >= c_Chatty) {
              cout<<"Learning sequence "<<i<<": "<<m_BadValue<<endl;
            }
          } else {
            m_SequenceNNs[EnergyBin][SequenceBin].SetOutput(i, m_GoodValue);
            if (g_Verbosity >= c_Chatty) {
              cout<<"Learning sequence "<<i<<": "<<m_GoodValue<<endl;
            }
          }
        }
        
        // ... get IO store ...
        MNeuralNetworkIO IO = m_SequenceNNs[EnergyBin][SequenceBin].GetIOStore();
        
        // Add event ID:
        IO.AddUserValue("ID", RE->GetEventId());
        if (gRandom->Rndm() < 0.15) {
          IO.IsVerificationData(true);
        } else{
          IO.IsVerificationData(false);
        }
        
        // ... and store ...
        m_SequenceNNIOStore[EnergyBin][SequenceBin].Add(IO);

        MString Comment4 = "# Success ratio (cla): ";
        if (SequenceBin == 2) { // == 2-site events
          Comment4 += "N/A (we only check for the minimum dphi as criteria, which requires 3+ hits)"; 
        } else {
          double ClassicRatio = 100.0*double(ClassicSequenceGood[EnergyBin][SequenceBin])/(ClassicSequenceGood[EnergyBin][SequenceBin] + ClassicSequenceBad[EnergyBin][SequenceBin]);
          Comment4 += ClassicRatio;
        }
        m_SequenceNNs[EnergyBin][SequenceBin].SetUserComment(3, Comment4);
        
        
        
        
        CounterToNextTeaching++;
      }
      
    } // For each raw event...
    
    if (/*g_Verbosity >= c_Info &&*/ CounterToNextTeaching > 0 && CounterToNextTeaching % 100 == 0) {
      cout<<"Counter to next teaching: "<<CounterToNextTeaching<<endl;
    }
    
    // Teach, and check if we have to remove events from the store
    if (CounterToNextTeaching >= 10000) {
      CounterToNextTeaching = 0;
      for (unsigned int t = 0; t < NumberOfTeachingRounds; ++t) {
        Teach();
      }
      
      //
      for (unsigned int e = 0; e < m_EnergyMin.size(); ++e) {
        for (unsigned int s = 2; s <= (unsigned int) m_MaxNInteractions; ++s) {
          cout<<"Bin size: e="<<e<<", seq="<<s<<": "<<m_SequenceNNIOStore[e][s].Size()<<endl;
          if (m_SequenceNNIOStore[e][s].Size() > m_EventsToStore) {
            cout<<" ---> Cutting..."<<endl;
            while (m_SequenceNNIOStore[e][s].Size() > m_EventsToStore) m_SequenceNNIOStore[e][s].RemoveFirst();
          }
        }
      }
      
      for (unsigned int e = 0; e < m_EnergyMin.size(); ++e) {
        for (unsigned int s = 2; s <= (unsigned int) m_MaxNInteractions; ++s) {
          cout<<"Bin size: e="<<e<<", seq="<<s<<": "<<m_QualityNNIOStore[e][s].Size()<<endl;
          if (m_QualityNNIOStore[e][s].Size() > m_EventsToStore) {
            cout<<" ---> Cutting..."<<endl;
            while (m_QualityNNIOStore[e][s].Size() > m_EventsToStore) m_QualityNNIOStore[e][s].RemoveFirst();
          }
        }
      }
    }
    
    // g_DebugLevel = 0;
  } while (m_Interrupt == false);
  
  //   Simply2.close();
  //   Simply3.close();
  //   Simply4.close();
  
  // Do a final teaching:
  if (m_Interrupt == false) {
    Teach();
  }
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Finalize the response generation (i.e. save the data a final time )
bool MResponseMultipleComptonNeuralNet::Finalize() 
{ 
  return MResponseBuilder::Finalize(); 
}


////////////////////////////////////////////////////////////////////////////////


void MResponseMultipleComptonNeuralNet::Teach()
{
  // Teach the neural network the events
  
  // Go ahead event by event and compare the results: 
  int VerboseLevel = 0;
  
  // The statistics section:
  vector<vector<int>> SequenceGood(m_EnergyMin.size());
  vector<vector<int>> SequenceBad(m_EnergyMin.size());
  vector<vector<double>> SequenceRatio(m_EnergyMin.size());
  
  vector<vector<int>> SequenceVerificationDataGood(m_EnergyMin.size());
  vector<vector<int>> SequenceVerificationDataBad(m_EnergyMin.size());
  vector<vector<double>> SequenceVerificationDataRatio(m_EnergyMin.size());
  
  vector<vector<int>> QualityGood(m_EnergyMin.size());
  vector<vector<int>> QualityBad(m_EnergyMin.size());
  vector<vector<double>> QualityRatio(m_EnergyMin.size());
  
  vector<vector<int>> QualityVerificationDataGood(m_EnergyMin.size());
  vector<vector<int>> QualityVerificationDataBad(m_EnergyMin.size());
  vector<vector<double>> QualityVerificationDataRatio(m_EnergyMin.size());
  
  for (unsigned int e = 0; e < m_EnergyMin.size(); ++e) {
    SequenceGood[e].resize(m_MaxNInteractions+1);
    SequenceBad[e].resize(m_MaxNInteractions+1);
    SequenceRatio[e].resize(m_MaxNInteractions+1);
    
    SequenceVerificationDataGood[e].resize(m_MaxNInteractions+1);
    SequenceVerificationDataBad[e].resize(m_MaxNInteractions+1);
    SequenceVerificationDataRatio[e].resize(m_MaxNInteractions+1);
    
    QualityGood[e].resize(m_MaxNInteractions+1);
    QualityBad[e].resize(m_MaxNInteractions+1);
    QualityRatio[e].resize(m_MaxNInteractions+1);
    
    QualityVerificationDataGood[e].resize(m_MaxNInteractions+1);
    QualityVerificationDataBad[e].resize(m_MaxNInteractions+1);
    QualityVerificationDataRatio[e].resize(m_MaxNInteractions+1);
  }
  
  for (unsigned int e = 0; e < m_EnergyMin.size(); ++e) {
    for (unsigned int s = 2; s <= (unsigned int) m_MaxNInteractions; ++s) {
      SequenceGood[e][s] = 0;
      SequenceBad[e][s] = 0;
      SequenceRatio[e][s] = 0;
      
      SequenceVerificationDataGood[e][s] = 0;
      SequenceVerificationDataBad[e][s] = 0;
      SequenceVerificationDataRatio[e][s] = 0;
      
      QualityGood[e][s] = 0;
      QualityBad[e][s] = 0;
      QualityRatio[e][s] = 0;
      
      QualityVerificationDataGood[e][s] = 0;
      QualityVerificationDataBad[e][s] = 0;
      QualityVerificationDataRatio[e][s] = 0;
    }
  }
  
  for (unsigned int e = 0; e < m_EnergyMin.size(); ++e) {
    cout<<"Energy loop: "<<e<<endl;
    for (unsigned int s = 2; s <= (unsigned int) m_MaxNInteractions; ++s) {
      cout<<"Sequence loop: "<<s<<"/"<<m_MaxNInteractions<<endl;
      cout<<" + size sequence store: "<<m_SequenceNNIOStore[e][s].Size()<<endl;
      cout<<" + size quality store: "<<m_QualityNNIOStore[e][s].Size()<<endl;
      
      // Loop A-1: Learn Sequence 
      for (unsigned int io = 0; io < m_SequenceNNIOStore[e][s].Size(); ++io) {
        
        // Get and check input:
        MNeuralNetworkIO SequenceIOStore = m_SequenceNNIOStore[e][s].Get(io);
      
        // We do not want to look at verification data in the first loop
        if (SequenceIOStore.IsVerificationData() == true) continue;
      
        // Determine the correct sequence as known from the simulation --- we have exactly one good one with a value of 0.25 -- find it
        int CorrectSequence = 0;
        for (unsigned int i = 0; i < SequenceIOStore.GetNOutputs(); ++i) {
          if (SequenceIOStore.GetOutput(i) < 0.5) {
            CorrectSequence = i;
            break;
          }
        }
      
        // Set input:
        m_SequenceNNs[e][s].SetInput(SequenceIOStore);
        // Run the neural net, i.e. determine its output
        m_SequenceNNs[e][s].Run();
      
      
        // If the correct sequence also has the smallest output of the neural net, then the neural net knows the sequence
        bool NNKnowsSequence = true;
        int NNSuggestedSequence = m_SequenceNNs[e][s].GetOutputNodeWithSmallestValue();
        if (NNSuggestedSequence != CorrectSequence) {
          NNKnowsSequence = false;
        }
      
        // Store performance data
        if (NNKnowsSequence == true) {
          SequenceGood[e][s]++;
        } else {
          SequenceBad[e][s]++;
        }
      
        // ... prepare for learning ...
        m_SequenceNNs[e][s].SetOutputError(SequenceIOStore);
        // ... and learn ...
        m_SequenceNNs[e][s].Learn();
      
        // Dump some text:
        if (VerboseLevel > 0) {
          cout<<"Event ID: "<<SequenceIOStore.GetUserValue("ID")<<endl;
          for (unsigned int i = 0; i < SequenceIOStore.GetNOutputs(); ++i) {
            if (NNKnowsSequence == true) {
              cout<<"GOOD: Neural net has already learned the sequence of this event"<<endl;
            } else {
              cout<<"BAD: Neural net doesn't know the sequence of this event"<<endl;
            }
          }
        }
      
        if (m_Interrupt == true) break;
      } // sequenced IO loop for learning
           
      
      
      // Loop A-2: Learn Quality
      // -----------------------
      
      // We want to have an equal amount of good and bad ones in the training set,
      // thus determine a percentage of bad ones to ignore  
      unsigned int GoodOnes = 0;
      unsigned int BadOnes = 0;
      for (unsigned int io = 0; io < m_QualityNNIOStore[e][s].Size(); ++io) {
        if (m_QualityNNIOStore[e][s].Get(io).GetOutput(0) == m_GoodValue) {
          GoodOnes++; 
        } else {
          BadOnes++;
        }
      }
      bool RejectBadOnes = true;
      double RejectionRatio = 0.0;
      if (BadOnes > GoodOnes) {
        RejectionRatio = double(BadOnes - GoodOnes)/(BadOnes + GoodOnes);
        RejectBadOnes = true;
      } else {
        RejectionRatio = double(GoodOnes - BadOnes)/(BadOnes + GoodOnes);
        RejectBadOnes = false;
      }
      
      // Now do the learning
      for (unsigned int io = 0; io < m_QualityNNIOStore[e][s].Size(); ++io) {
        
        // Get and check input:
        MNeuralNetworkIO IO = m_QualityNNIOStore[e][s].Get(io);
      
        // We do not want to look at verification data in the first loop
        if (IO.IsVerificationData() == true) continue;
        
        // Reject a certain amout of bad ones to have the same amount of good and bad events:
        if (IO.GetOutput(0) == m_GoodValue) {
          if (RejectBadOnes == false) {
            if (gRandom->Rndm() < RejectionRatio) continue;
          }
        } else {
          if (RejectBadOnes == true) {
            if (gRandom->Rndm() < RejectionRatio) continue;
          }
        }
        
        
        // Set input:
        m_QualityNNs[e][s].SetInput(IO);
        // Run the neural net, i.e. determine its output
        m_QualityNNs[e][s].Run();
      
      
        // We know the quality if the output is with +-0.1 from the expected output
        bool NNKnowsQuality = false;
        if ((IO.GetOutput(0) > 0.5 && m_QualityNNs[e][s].GetOutput(0) > 0.5) || 
            (IO.GetOutput(0) < 0.5 && m_QualityNNs[e][s].GetOutput(0) < 0.5)) {
          NNKnowsQuality = true;
        }
        cout<<"Quality difference: "<<IO.GetOutput(0)<<":"<<m_QualityNNs[e][s].GetOutput(0)<<endl;
      
        // Store performance data
        if (NNKnowsQuality == true) {
          QualityGood[e][s]++;
        } else {
          QualityBad[e][s]++;
        }
      
        // ... prepare for learning ...
        m_QualityNNs[e][s].SetOutputError(IO);
        // ... and learn ...
        m_QualityNNs[e][s].Learn();
      
        // Dump some text:
        if (VerboseLevel > 0) {
          cout<<"Event ID: "<<IO.GetUserValue("ID")<<endl;
          for (unsigned int i = 0; i < IO.GetNOutputs(); ++i) {
            if (NNKnowsQuality == true) {
              cout<<"GOOD: Neural net has already learned the quality of this event"<<endl;
            } else {
              cout<<"BAD: Neural net doesn't know the quality of this event"<<endl;
            }
          }
        }
      
        if (m_Interrupt == true) break;
      } // sequenced IO loop for learning
           
           
      // -------
           
      // Loop B-1: Test with verfication IOs
      for (unsigned int io = 0; io < m_SequenceNNIOStore[e][s].Size(); ++io) {
        
        // Get and check input:
        MNeuralNetworkIO SequenceIOStore = m_SequenceNNIOStore[e][s].Get(io);
           
        // We do not want to look at training data in the second loop
        if (SequenceIOStore.IsVerificationData() == false) continue;
           
        // Determine the correct sequence as known by simulation --- we have exactly one good one if a value of 0.25
        int CorrectSequence = 0;
        for (unsigned int i = 0; i < SequenceIOStore.GetNOutputs(); ++i) {
          if (SequenceIOStore.GetOutput(i) < 0.5) {
            CorrectSequence = i;
            break;
          }
        }
           
        // Set input:
        m_SequenceNNs[e][s].SetInput(SequenceIOStore);
        // Run the neural net, i.e. determine its output
        m_SequenceNNs[e][s].Run();
           
           
        // If the correct sequence also has the smallest output of the neural net, then the neural net knows the sequence
        bool NNKnowsSequence = true;
        int NNSuggestedSequence = m_SequenceNNs[e][s].GetOutputNodeWithSmallestValue();
        if (NNSuggestedSequence != CorrectSequence) {
          NNKnowsSequence = false;
        }
           
        // Store verification performance data
        if (NNKnowsSequence == true) {
          SequenceVerificationDataGood[e][s]++;
        } else {
          SequenceVerificationDataBad[e][s]++;
        }
           
        // Dump some text:
        if (VerboseLevel > 0) {
          cout<<"Event ID: "<<SequenceIOStore.GetUserValue("ID")<<endl;
          for (unsigned int i = 0; i < SequenceIOStore.GetNOutputs(); ++i) {
            if (NNKnowsSequence == true) {
              cout<<"GOOD: Neural net has already learned the sequence of this event"<<endl;
            } else {
              cout<<"BAD: Neural net doesn't know the sequence of this event"<<endl;
            }
          }
        }
           
        if (m_Interrupt == true) break;
      } // sequenced IO loop for verification
                
    
    
      // Loop B-2: Test quality with verfication IOs
      for (unsigned int io = 0; io < m_QualityNNIOStore[e][s].Size(); ++io) {
        
        // Get and check input:
        MNeuralNetworkIO IO = m_QualityNNIOStore[e][s].Get(io);
           
        // We do not want to look at training data in the second loop
        if (IO.IsVerificationData() == false) continue;
        
         // Reject a certain amout of bad ones to have the same amount of good and bad events:
        if (IO.GetOutput(0) == m_GoodValue) {
          if (RejectBadOnes == false) {
            if (gRandom->Rndm() < RejectionRatio) continue;
          }
        } else {
          if (RejectBadOnes == true) {
            if (gRandom->Rndm() < RejectionRatio) continue;
          }
        }
          
         // Set input:
        m_QualityNNs[e][s].SetInput(IO);
        // Run the neural net, i.e. determine its output
        m_QualityNNs[e][s].Run();
      
      
        // We know the quality if the output is with +-0.1 from the expected output
        bool NNKnowsQuality = false;
        if ((IO.GetOutput(0) > 0.5 && m_QualityNNs[e][s].GetOutput(0) > 0.5) || 
            (IO.GetOutput(0) < 0.5 && m_QualityNNs[e][s].GetOutput(0) < 0.5)) {
          NNKnowsQuality = true;
        }
        cout<<"Verficication quality difference: "<<IO.GetOutput(0)<<":"<<m_QualityNNs[e][s].GetOutput(0)<<endl;
     
        // Store verification performance data
        if (NNKnowsQuality == true) {
          QualityVerificationDataGood[e][s]++;
          cout<<" --> good "<<endl;
        } else {
          QualityVerificationDataBad[e][s]++;
          cout<<" --> bad "<<endl;
       }
           
        // Dump some text:
        if (VerboseLevel > 0) {
          cout<<"Event ID: "<<IO.GetUserValue("ID")<<endl;
          for (unsigned int i = 0; i < IO.GetNOutputs(); ++i) {
            if (NNKnowsQuality == true) {
              cout<<"GOOD: Neural net has already learned the sequence of this event"<<endl;
            } else {
              cout<<"BAD: Neural net doesn't know the sequence of this event"<<endl;
            }
          }
        }
           
        if (m_Interrupt == true) break;
      } // sequenced IO loop for verification
                
      if (m_Interrupt == true) break;
    } // sequence loop
    
    if (m_Interrupt == true) break;
  } // energy loop
  
  
  
  // Check what we have learned:
  for (unsigned int e = 0; e < m_EnergyMin.size(); ++e) {
    for (unsigned int s = 2; s <= (unsigned int) m_MaxNInteractions; ++s) {
      cout<<endl;
      cout<<"Performance for sequence length "<<s<<" within "<<m_EnergyMin[e]<<" - "<<m_EnergyMax[e]<<" keV"<<endl;
      
      // Sequence:
      if (SequenceGood[e][s] + SequenceBad[e][s] > 0) {
        MString Comment1 = "# Number of different training events: ";
        Comment1 += m_SequenceNNIOStore[e][s].Size();
        m_SequenceNNs[e][s].SetUserComment(0, Comment1);
        
        cout<<"(a) Sequence "<<s<<endl;
        SequenceRatio[e][s] = 100.0*double(SequenceGood[e][s])/(SequenceGood[e][s]+SequenceBad[e][s]);
        cout<<"    This loop (all):  Good: "<<SequenceGood[e][s]<<" Bad: "<<SequenceBad[e][s]<<" Ratio: "<<SequenceRatio[e][s]<<endl;
        
        MString Comment2 = "# Success ratio (all): ";
        Comment2 += SequenceRatio[e][s];
        m_SequenceNNs[e][s].SetUserComment(1, Comment2);
        
        if (SequenceVerificationDataGood[e][s]+SequenceVerificationDataBad[e][s] > 0) {
          SequenceVerificationDataRatio[e][s] = 100.0*double(SequenceVerificationDataGood[e][s])/(SequenceVerificationDataGood[e][s]+SequenceVerificationDataBad[e][s]);
          cout<<"    This loop (ver):  Good: "<<SequenceVerificationDataGood[e][s]<<" Bad: "<<SequenceVerificationDataBad[e][s]
          <<" Ratio: "<<SequenceVerificationDataRatio[e][s]<<" (best: "<<m_SequenceBestVerificationDataRatio[e][s]<<")"<<endl;
          
          MString Comment3 = "# Success ratio (ver): ";
          Comment3 += SequenceVerificationDataRatio[e][s];
          m_SequenceNNs[e][s].SetUserComment(2, Comment3);
          
          if (m_Interrupt == false) {
            if (SequenceVerificationDataRatio[e][s] > m_SequenceBestVerificationDataRatio[e][s] && SequenceGood[e][s] + SequenceBad[e][s] > 1000) {
              m_SequenceBestVerificationDataRatio[e][s] = SequenceVerificationDataRatio[e][s];
              SaveMatrixSequenceNN(e, s, ".best");
            }
          }
        }
      }
      
      // Quality:
      if (QualityGood[e][s] + QualityBad[e][s] > 0) {
        MString Comment1 = "# Number of different training events: ";
        Comment1 += m_QualityNNIOStore[e][s].Size();
        m_QualityNNs[e][s].SetUserComment(0, Comment1);
        
        cout<<"(b) Quality "<<s<<endl;
        QualityRatio[e][s] = 100.0*double(QualityGood[e][s])/(QualityGood[e][s]+QualityBad[e][s]);
        cout<<"    This loop (all):  Good: "<<QualityGood[e][s]<<" Bad: "<<QualityBad[e][s]<<" Ratio: "<<QualityRatio[e][s]<<endl;
        
        MString Comment2 = "# Success ratio (all): ";
        Comment2 += QualityRatio[e][s];
        m_QualityNNs[e][s].SetUserComment(1, Comment2);
        
        if (QualityVerificationDataGood[e][s]+QualityVerificationDataBad[e][s] > 0) {
          QualityVerificationDataRatio[e][s] = 100.0*double(QualityVerificationDataGood[e][s])/(QualityVerificationDataGood[e][s]+QualityVerificationDataBad[e][s]);
          cout<<"    This loop (ver):  Good: "<<QualityVerificationDataGood[e][s]<<" Bad: "<<QualityVerificationDataBad[e][s]
          <<" Ratio: "<<QualityVerificationDataRatio[e][s]<<" (best: "<<m_QualityBestVerificationDataRatio[e][s]<<")"<<endl;
          
          MString Comment3 = "# Success ratio (ver): ";
          Comment3 += QualityVerificationDataRatio[e][s];
          m_QualityNNs[e][s].SetUserComment(2, Comment3);
          
          if (m_Interrupt == false) {
            if (QualityVerificationDataRatio[e][s] > m_QualityBestVerificationDataRatio[e][s] && QualityGood[e][s] + QualityBad[e][s] > 1000) {
              m_QualityBestVerificationDataRatio[e][s] = QualityVerificationDataRatio[e][s];
              SaveMatrixQualityNN(e, s, ".best");
            }
          }
        }

      }
    }
  } // statistics loop
  
  
  Save();
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseMultipleComptonNeuralNet::AreCompletelyAbsorbed(const vector<MRESE*>& RESEs, MRERawEvent* RE)
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


bool MResponseMultipleComptonNeuralNet::FindFirstInteractions(const vector<MRESE*>& RESEs, MRESE*& First, MRESE*& Second)
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
  
  if (m_SiEvent->GetIAAt(Smallest-1)->GetOrigin() == 0) {
    if (m_SiEvent->GetIAAt(Smallest-1)->GetType() != "ANNI" &&
      m_SiEvent->GetIAAt(Smallest-1)->GetType() != "INIT") {
      if (g_Verbosity >= c_Chatty) mout<<"FindFirstInteractions: IA Type not OK: "<<m_SiEvent->GetIAAt(Smallest-1)->GetType()<<endl;
    return false;
      }
  } else {
    if (m_SiEvent->GetIAAt(m_SiEvent->GetIAAt(Smallest-1)->GetOrigin()-1)->GetType() != "ANNI" &&
      m_SiEvent->GetIAAt(m_SiEvent->GetIAAt(Smallest-1)->GetOrigin()-1)->GetType() != "INIT") {
      if (g_Verbosity >= c_Chatty) mout<<"FindFirstInteractions: IA Type not OK: "<<m_SiEvent->GetIAAt(m_SiEvent->GetIAAt(Smallest-1)->GetOrigin()-1)->GetType()<<endl;
    return false;
      }
  }
  
  if (m_SiEvent->GetIAAt(m_SiEvent->GetIAAt(Smallest-1)->GetOrigin()-1)->GetParticleNumber() == 1) {
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
    if (g_Verbosity >= c_Chatty) mout<<"FindFirstInteractions: IA which triggered first RESE is no photon: "<<m_SiEvent->GetIAAt(m_SiEvent->GetIAAt(Smallest-1)->GetOrigin()-1)->GetParticleNumber()<<endl;
    return false;
  }
  
  // Go through the main event tree in the file and find the RESEs:
  for (unsigned int s = Smallest+1; s < m_SiEvent->GetNIAs(); ++s) {
    // The second event in the sequence needs to have the same origin than the first one
    if (m_SiEvent->GetIAAt(Smallest)->GetOrigin() == m_SiEvent->GetIAAt(s)->GetOrigin()) {
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


bool MResponseMultipleComptonNeuralNet::FindCorrectSequence(const vector<MRESE*>& RESEs, vector<MRESE*>& Sorted)
{
  // Return true if those were found
  // 
  // First and second are the *detected* first and second interaction ---
  // there may be some undetected before and in between
  
  //mout<<"FindFirstInteractions"<<endl;
  Sorted.resize(RESEs.size());
  for (unsigned int i = 0; i < Sorted.size(); ++i) Sorted[i] = 0;
  
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
  
  // What about BREM
  if (m_SiEvent->GetIAAt(Smallest-1)->GetOrigin() == 0) {
    if (m_SiEvent->GetIAAt(Smallest-1)->GetType() != "ANNI" && m_SiEvent->GetIAAt(Smallest-1)->GetType() != "INIT") {
      if (g_Verbosity >= c_Chatty) mout<<"FindCorrectSequence: IA Type not OK: "<<m_SiEvent->GetIAAt(Smallest-1)->GetType()<<endl;
        return false;
      }
    } else {
      if (m_SiEvent->GetIAAt(m_SiEvent->GetIAAt(Smallest-1)->GetOrigin()-1)->GetType() != "ANNI" &&
          m_SiEvent->GetIAAt(m_SiEvent->GetIAAt(Smallest-1)->GetOrigin()-1)->GetType() != "INIT") {
        if (g_Verbosity >= c_Chatty) mout<<"FindCorrectSequence: IA Type not OK: "<<m_SiEvent->GetIAAt(m_SiEvent->GetIAAt(Smallest-1)->GetOrigin()-1)->GetType()<<endl;
        return false;
      }
    }
  
  if (m_SiEvent->GetIAAt(m_SiEvent->GetIAAt(Smallest-1)->GetOrigin()-1)->GetParticleNumber() == 1) {
    // Find one and only RESE associated with this:
    for (unsigned int i = 0; i < OriginIds.size(); ++i) {
      for (unsigned int j = 0; j < OriginIds[i].size(); ++j) {
        if (OriginIds[i][j] == Smallest) {
          if (Sorted[0] == 0) {
            Sorted[0] = RESEs[i];
          } else {
            // If we have more than one RESE associated with this, then we have no clear first hit 
            // and have to reject...
            if (g_Verbosity >= c_Chatty) mout<<"FindCorrectSequence: First: RESE "<<Sorted[0]->GetID()<<" and "<<RESEs[i]->GetID()<<" are associated with start IA "<<Smallest<<endl;
            return false;
          }
        }
      }
    }
  } else {
    // Only photons can be good...
    if (g_Verbosity >= c_Chatty) mout<<"FindCorrectSequence: IA which triggered first RESE is no photon: "<<m_SiEvent->GetIAAt(m_SiEvent->GetIAAt(Smallest-1)->GetOrigin()-1)->GetParticleNumber()<<endl;
    return false;
  }
  
  //mout<<"FindCorrectSequence: First RESE "<<Sorted[0]->GetID()<<" (Smallest: "<<Smallest<<")"<<endl;
  
  // Check if there is a Compton is missing...
  for (unsigned int s = Smallest; s < m_SiEvent->GetNIAs(); ++s) {
    // The second event in the sequence needs to have the same origin than the first one
    if (m_SiEvent->GetIAAt(Smallest)->GetOrigin() == m_SiEvent->GetIAAt(s)->GetOrigin() &&
      m_SiEvent->GetIAAt(s)->GetType() == "COMP") {
      //cout<<m_SiEvent->GetIAAt(s)->GetEnergy()<<endl;
      bool Found = false;
    for (unsigned int i = 0; i < OriginIds.size(); ++i) {
      for (unsigned int j = 0; j < OriginIds[i].size(); ++j) {
        //cout<<int(s+1)<<":"<<OriginIds[i][j]<<endl;
        if (OriginIds[i][j] == int(s+1)) {
          Found = true;
          break;
        }
      }
    }
    if (Found == false) {
      if (g_Verbosity >= c_Chatty) mout<<"FindCorrectSequence: Gap! Cannot find representant for Compton ID "<<s+1<<endl;
      return false;
    }
      }
  }
  
  // Go through the main event tree in the file and find the RESE sequence:
  unsigned int FreeSpotInSorted = 1;
  for (unsigned int s = Smallest; s < m_SiEvent->GetNIAs(); ++s) {
    // The second event in the sequence needs to have the same origin than the first one
    if (m_SiEvent->GetIAAt(Smallest)->GetOrigin() == m_SiEvent->GetIAAt(s)->GetOrigin()) {
      // Loop over all RESEs
      for (unsigned int i = 0; i < OriginIds.size(); ++i) {
        // If the current RESE is not yet in the list:
        if (find(Sorted.begin(), Sorted.end(), RESEs[i]) == Sorted.end()) {
          for (unsigned int j = 0; j < OriginIds[i].size(); ++j) {
            if (OriginIds[i][j] == int(s)) {
              Sorted[FreeSpotInSorted] = RESEs[i];
              //mout<<"FindCorrectSequence: next RESE "<<Sorted[FreeSpotInSorted]->GetID()<<endl;
              FreeSpotInSorted++;
            }
          }
        }
      }
    }
  }
  
  // Check if in all but the last interaction only contain a Compton interaction and it's possible dependents:
  for (unsigned int s = 0; s < Sorted.size()-1; ++s) {
    if (Sorted[s] != 0) {
      vector<int> SortedIDs = GetOriginIds(Sorted[s]);
      if (ContainsOnlyComptonDependants(SortedIDs) == false) {
        if (g_Verbosity >= c_Chatty) mout<<"FindCorrectSequence: Not only Compton (+ dependents) in interaction "<<Sorted[s]->GetID()<<endl;
        return false;
      }
      if (NumberOfComptonInteractions(SortedIDs) != 1) {
        if (g_Verbosity >= c_Chatty) mout<<"FindCorrectSequence: Not exactly one Compton in interaction "<<Sorted[s]->GetID()<<endl;
        return false;        
      }
      //mout<<"FindCorrectSequence: All Compton "<<Sorted[s]->GetID()<<endl;
    }
  }
  
  //   mout<<"Sequence: ";
  //   for (unsigned int s = 0; s < Sorted.size(); ++s) {
  //     if (Sorted[s] == 0) {
  //       mout<<" ?";
  //     } else {
  //       mout<<" "<<Sorted[s]->GetID();
  //     }
  //   }
  //   mout<<endl;
  
  for (unsigned int s = 0; s < Sorted.size(); ++s) {
    if (Sorted[s] == 0) {
      if (g_Verbosity >= c_Chatty) mout<<"FindCorrectSequence: Did not find complete sequence!"<<endl;
      return false;
    }
  }
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


unsigned int MResponseMultipleComptonNeuralNet::NumberOfComptonInteractions(vector<int> AllSimIds)
{
  unsigned int N = 0;
  
  for (unsigned int i = 0; i < AllSimIds.size(); ++i) {
    if (m_SiEvent->GetIAAt(AllSimIds[i]-1)->GetType() == "COMP") {
      N++;
    }
  }
  
  return N;
}


////////////////////////////////////////////////////////////////////////////////


void MResponseMultipleComptonNeuralNet::Shuffle(vector<MRESE*>& RESEs)
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


// MResponseMultipleComptonNeuralNet.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
