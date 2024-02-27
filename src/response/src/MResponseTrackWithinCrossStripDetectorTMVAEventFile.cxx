/*
 * MResponseTrackWithinCrossStripDetectorTMVAEventFile.cxx
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
// MResponseTrackWithinCrossStripDetectorTMVAEventFile
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MResponseTrackWithinCrossStripDetectorTMVAEventFile.h"

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
#include "MRESE.h"
#include "MREStripHit.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MResponseTrackWithinCrossStripDetectorTMVAEventFile)
#endif


////////////////////////////////////////////////////////////////////////////////


MResponseTrackWithinCrossStripDetectorTMVAEventFile::MResponseTrackWithinCrossStripDetectorTMVAEventFile()
{
  // Construct an instance of MResponseTrackWithinCrossStripDetectorTMVAEventFile
  
  m_ResponseNameSuffix = "electrontrackingwithcrossstripdetector.tmva";
  
  m_MaxNHitsX = 6;
  m_MaxNHitsY = 6;
}


////////////////////////////////////////////////////////////////////////////////


MResponseTrackWithinCrossStripDetectorTMVAEventFile::~MResponseTrackWithinCrossStripDetectorTMVAEventFile()
{
  // Delete this instance of MResponseTrackWithinCrossStripDetectorTMVAEventFile
}


////////////////////////////////////////////////////////////////////////////////


//! Return a brief description of this response class
MString MResponseTrackWithinCrossStripDetectorTMVAEventFile::Description()
{
  return MString("create ROOT event files for TMVA algorithms for electron tracking with cross strip detector");
}


////////////////////////////////////////////////////////////////////////////////


//! Return information on the parsable options for this response class
MString MResponseTrackWithinCrossStripDetectorTMVAEventFile::Options()
{
  ostringstream out;
  //out<<"             initial:   use the path to the initial IA (default: true)"<<endl;
  
  return MString(out);
}


////////////////////////////////////////////////////////////////////////////////


//! Parse the options
bool MResponseTrackWithinCrossStripDetectorTMVAEventFile::ParseOptions(const MString& Options)
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
  //mout<<"Choosen options for creating TMVA strip pairing data files:"<<endl;
  //mout<<"  Path to first IA:          "<<(m_UsePathToFirstIA == true ? "true" : "false")<<endl;
  mout<<endl;
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Initialize the response matrices and their generation
bool MResponseTrackWithinCrossStripDetectorTMVAEventFile::Initialize()
{ 
  // Initialize next matching event, save if necessary
  if (MResponseBuilder::Initialize() == false) return false;

  // Turn off any noising
  m_ReGeometry->ActivateNoising(false);
  
  // We ignore the loaded configuration file
  m_ReReader->SetEventClusteringAlgorithm(MRawEventAnalyzer::c_EventClusteringAlgoNone);
  m_ReReader->SetHitClusteringAlgorithm(MRawEventAnalyzer::c_HitClusteringAlgoNone);
  m_ReReader->SetTrackingAlgorithm(MRawEventAnalyzer::c_TrackingAlgoNone);
  m_ReReader->SetCSRAlgorithm(MRawEventAnalyzer::c_CSRAlgoNone);
  m_ReReader->SetDecayAlgorithm(MRawEventAnalyzer::c_DecayAlgoNone);
  
  // Start pre-analysis
  if (m_ReReader->PreAnalysis() == false) return false;
  
  // Create the ROOT trees for the TMVA analysis
  m_DataSets = vector<vector<MERTrackWithinCrossStripDetectorTMVADataSet*>>(m_MaxNHitsX+1, vector<MERTrackWithinCrossStripDetectorTMVADataSet*>(m_MaxNHitsY+1, nullptr));
  m_Trees = vector<vector<TTree*>>(m_MaxNHitsX+1, vector<TTree*>(m_MaxNHitsY+1, nullptr));
  
  for (unsigned int x = 1; x <= m_MaxNHitsX; ++x) {
    for (unsigned int y = 1; y <= m_MaxNHitsY; ++y) {
      MERTrackWithinCrossStripDetectorTMVADataSet* DS = new MERTrackWithinCrossStripDetectorTMVADataSet();
      DS->Initialize(x, y);
      MString Name("TrackWithinCrossStripDetectorTMVA");
      //Name += "_";
      //Name += x;
      //Name += "_";
      //Name += y;
      m_Trees[x][y] = DS->CreateTree(Name);
      m_DataSets[x][y] = DS;
    }
  }

  // We only can save once at the end, thus:
  m_SaveAfter = numeric_limits<unsigned long>::max();
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseTrackWithinCrossStripDetectorTMVAEventFile::Save()
{
  // Nothing here, only save at the end
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Shuffle the strips
void MResponseTrackWithinCrossStripDetectorTMVAEventFile::ShuffleStrips(vector<unsigned int>& StripIDs, vector<double>& Energies)
{
  unsigned int size = StripIDs.size();
  for (unsigned int i = 0; i < 2*size; ++i) {
    
    unsigned int From = gRandom->Integer(size);
    unsigned int To = gRandom->Integer(size);
    
    unsigned int TempID = StripIDs[To];
    StripIDs[To] = StripIDs[From];
    StripIDs[From] = TempID;
    
    unsigned int TempEnergy = Energies[To];
    Energies[To] = Energies[From];
    Energies[From] = TempEnergy;
  }  
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseTrackWithinCrossStripDetectorTMVAEventFile::Analyze()
{
  // Create the strip pairing response
  
  // Initialize the next matching event, save if necessary
  if (MResponseBuilder::Analyze() == false) return false;
  
  //cout<<endl<<"Start "<<m_SiEvent->GetID()<<endl;

  //cout<<endl<<m_SiEvent->ToSimString()<<endl;
  //cout<<endl<<m_ReEvent->ToString()<<endl;

  // We need to have at least an "INIT" in the simulation file per event 
  if (m_SiEvent->GetNIAs() < 2) {
    //cout<<"No INIT"<<endl;
    return true;
  }
  
  // We require the initial raw event
  MRERawEvent* RE = m_ReReader->GetInitialRawEvent();
  if (RE == nullptr) {
    //cout<<"No initial raw event"<<endl;
    return true;
  }
  //cout<<endl<<RE->ToString()<<endl;

  // (I) Event cleaning

  // For training we only want events where the first interaction is a Compton event
  if (m_SiEvent->GetIAAt(1)->GetProcess() != "COMP") {
    //cout<<"First IA is not Compton"<<endl;
    return true;
  }
  
  // It needs to be in a cross-strip detector
  if (m_SiEvent->GetIAAt(1)->GetDetectorType() != MDDetector::c_Strip3D) {
    //cout<<"First IA is not in a 3D strip detector"<<endl;
    return true;
  }

  // It needs to be fully contained

  
  // (II) Save the measured strips
  
  vector<MRESE*> RESEs;
  for (int r = 0; r < RE->GetNRESEs(); ++r) {
    RESEs.push_back(RE->GetRESEAt(r));
  }
  Shuffle(RESEs);
  
  vector<long> VolumeTree;
  vector<bool> IsXStrip;
  vector<int> StripID;
  vector<double> StripEnergy;
  vector<vector<int>> Origins;
  vector<bool> Ignore;
  vector<MDVolumeSequence*> StripVolumeSequences;
  
  vector<vector<MREStripHit*>> StripHitsPerDetector;

  // (1) Collect the data
  for (unsigned int r = 0; r < RESEs.size(); ++r) {
    MRESE* R = RESEs[r];
    if (R->GetDetector() != MDDetector::c_Strip3D) continue;

    MREStripHit* StripHit = dynamic_cast<MREStripHit*>(R);
    if (StripHit == nullptr) {
      //cout<<"ERROR: RESE is not strip hit"<<endl;
      continue;
    }

    bool DetectorFound = false;
    for (auto& SHVector: StripHitsPerDetector) {
      if (StripHit->GetVolumeSequence()->HasSameDetector(SHVector[0]->GetVolumeSequence()) == true) {
        SHVector.push_back(StripHit);
        DetectorFound = true;
        break;
      }
    }
    if (DetectorFound == false) {
      StripHitsPerDetector.push_back(vector<MREStripHit*>());
      StripHitsPerDetector.back().push_back(StripHit);
    }
  }
  if (StripHitsPerDetector.size() == 0) {
    //cout<<"No strip hits found"<<endl;
    return true;
  }

  // (2) Sort the data:
  // ... by side
  vector<vector<vector<MREStripHit*>>> StripHitsPerDetectorPerSide;
  for (auto& SHVector: StripHitsPerDetector) {
    StripHitsPerDetectorPerSide.push_back(vector<vector<MREStripHit*>>());
    StripHitsPerDetectorPerSide.back().push_back(vector<MREStripHit*>());
    StripHitsPerDetectorPerSide.back().push_back(vector<MREStripHit*>());

    for (auto SH: SHVector) {
      if (SH->IsXStrip() == true) {
        StripHitsPerDetectorPerSide.back().at(0).push_back(SH);
      } else {
        StripHitsPerDetectorPerSide.back().at(1).push_back(SH);
      }
    }
  }
  // ... by channel
  for (auto& SHDetector: StripHitsPerDetectorPerSide) {
    for (auto& SHSide: SHDetector) {
      std::sort(SHSide.begin(), SHSide.end(), [](const MREStripHit* A, const MREStripHit* B) {
        return A->GetStripID() < B->GetStripID();
      });
    }
  }

  // (3) We do not want to worry about strip pairing here, thus we only keep data with exactly one hit per detector
  for (auto SHDetector = StripHitsPerDetectorPerSide.begin(); SHDetector != StripHitsPerDetectorPerSide.end();) {
    bool Sequential = true;
    for (auto& SHSide: (*SHDetector)) {
      for (unsigned int i = 1; i < SHSide.size(); ++i) {
        if (SHSide[i]->GetStripID() != SHSide[i-1]->GetStripID()+1) {
          Sequential = false;
        }
      }
      if (Sequential == false) break;
    }

    if (Sequential == true) {
      ++SHDetector;
    } else {
      SHDetector = StripHitsPerDetectorPerSide.erase(SHDetector);
      //cout<<"Not sequential"<<endl;
    }
  }

  // (4) Now make sure
  // (a) we have exactly one interation per detector and
  // (b) its a Compton interaction
  // (c) store it position, and recoil electron direction
  vector<MVector> Positions;
  vector<MVector> RecoilElectronDirections;
  for (auto SHDetector = StripHitsPerDetectorPerSide.begin(); SHDetector != StripHitsPerDetectorPerSide.end();) {
    vector<MRESE*> RESEs;
    for (auto& SHSide: (*SHDetector)) {
      for (MREStripHit* SH: SHSide) {
        RESEs.push_back(dynamic_cast<MRESE*>(SH));
      }
    }

    vector<int> OriginIDs = GetOriginIds(RESEs);

    MSimIA* IA = m_SiEvent->GetIAAt(OriginIDs[0]-1);
    if (ContainsOnlyComptonDependants(OriginIDs) == true && IA->GetProcess() == "COMP") {
      ++SHDetector;
      Positions.push_back(IA->GetPosition());
      RecoilElectronDirections.push_back(IA->GetSecondaryDirection());
    } else {
      SHDetector = StripHitsPerDetectorPerSide.erase(SHDetector);
      //cout<<"Not exactly one IA in detector or first IA is not a compton"<<endl;
    }
  }

  
  // (5) Assemble by detector and write to file
  for (unsigned int d = 0; d < StripHitsPerDetectorPerSide.size(); ++d) {
    vector<vector<MREStripHit*>> SHDetector = StripHitsPerDetectorPerSide[d];

    vector<unsigned int> XStripIDs;
    vector<unsigned int> YStripIDs;
    vector<double> XStripEnergies;
    vector<double> YStripEnergies;
    vector<int> AllOrigins;

    for (auto& SHSideX: SHDetector[0]) {
      XStripIDs.push_back(SHSideX->GetStripID());
      XStripEnergies.push_back(SHSideX->GetEnergy());
      //cout<<"Added x"<<SHSideX->GetStripID()<<endl;
    }

    for (auto& SHSideY: SHDetector[1]) {
      YStripIDs.push_back(SHSideY->GetStripID());
      YStripEnergies.push_back(SHSideY->GetEnergy());
      //cout<<"Added y"<<SHSideY->GetStripID()<<endl;
    }


    
    // (b) Shuffle -- otherwise we will get patterns...
    //ShuffleStrips(XStripIDs, XStripEnergies);
    //ShuffleStrips(YStripIDs, YStripEnergies);
    
    // (d) Write to file
    if (XStripIDs.size() == 0 || XStripIDs.size() > m_MaxNHitsX) continue;
    if (YStripIDs.size() == 0 || YStripIDs.size() > m_MaxNHitsY) continue;
    
    if (m_DataSets[XStripIDs.size()][YStripIDs.size()]->FillEventData(RE->GetEventID(), XStripIDs, YStripIDs, XStripEnergies, YStripEnergies) == false) {
      continue;
    }
    if (m_DataSets[XStripIDs.size()][YStripIDs.size()]->FillResultData(Positions[d], RecoilElectronDirections[d]) == false) {
      continue;
    }
    
    m_Trees[XStripIDs.size()][YStripIDs.size()]->Fill();
  }

  //cout<<"Done"<<endl;

  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Finalize the response generation (i.e. save the data a final time )
bool MResponseTrackWithinCrossStripDetectorTMVAEventFile::Finalize()
{ 
  // We only save at the end since ROOT has trouble updating the file...
  for (unsigned int x = 1; x <= m_MaxNHitsX; ++x) {
    for (unsigned int y = 1; y <= m_MaxNHitsY; ++y) {
      TFile Tree(m_ResponseName + ".x" + x + ".y" + y + "." + m_ResponseNameSuffix + ".root", "recreate");
      Tree.cd();
      m_Trees[x][y]->Write();
      Tree.Close();      
    }
  }
  
  return MResponseBuilder::Finalize(); 
}


// MResponseTrackWithinCrossStripDetectorTMVAEventFile.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
