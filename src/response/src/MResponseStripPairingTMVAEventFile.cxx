/*
 * MResponseStripPairingTMVAEventFile.cxx
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
// MResponseStripPairingTMVAEventFile
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MResponseStripPairingTMVAEventFile.h"

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
ClassImp(MResponseStripPairingTMVAEventFile)
#endif


////////////////////////////////////////////////////////////////////////////////


MResponseStripPairingTMVAEventFile::MResponseStripPairingTMVAEventFile()
{
  // Construct an instance of MResponseStripPairingTMVAEventFile
  
  m_ResponseNameSuffix = "strippairing.tmva";
  
  m_MaxNHitsX = 6;
  m_MaxNHitsY = 6;
}


////////////////////////////////////////////////////////////////////////////////


MResponseStripPairingTMVAEventFile::~MResponseStripPairingTMVAEventFile()
{
  // Delete this instance of MResponseStripPairingTMVAEventFile
}


////////////////////////////////////////////////////////////////////////////////


//! Return a brief description of this response class
MString MResponseStripPairingTMVAEventFile::Description()
{
  return MString("create ROOT event files for TMVA algorithms for strip pairing");
}


////////////////////////////////////////////////////////////////////////////////


//! Return information on the parsable options for this response class
MString MResponseStripPairingTMVAEventFile::Options()
{
  ostringstream out;
  //out<<"             initial:   use the path to the initial IA (default: true)"<<endl;
  
  return MString(out);
}


////////////////////////////////////////////////////////////////////////////////


//! Parse the options
bool MResponseStripPairingTMVAEventFile::ParseOptions(const MString& Options)
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
bool MResponseStripPairingTMVAEventFile::Initialize() 
{ 
  // Initialize next matching event, save if necessary
  if (MResponseBuilder::Initialize() == false) return false;

  // Turn off any noising
  m_ReGeometry->ActivateNoising(false);
  
  // We ignore the loaded configuration file
  m_ReReader->SetClusteringAlgorithm(MRawEventAnalyzer::c_ClusteringAlgoNone);
  m_ReReader->SetTrackingAlgorithm(MRawEventAnalyzer::c_TrackingAlgoNone);
  m_ReReader->SetCSRAlgorithm(MRawEventAnalyzer::c_CSRAlgoNone);
  m_ReReader->SetDecayAlgorithm(MRawEventAnalyzer::c_DecayAlgoNone);
  
  // Start pre-analysis
  if (m_ReReader->PreAnalysis() == false) return false;
  
  // Create the ROOT trees for the TMVA analysis
  m_DataSets = vector<vector<MERStripPairingDataSet*>>(m_MaxNHitsX+1, vector<MERStripPairingDataSet*>(m_MaxNHitsY+1, nullptr));
  m_Trees = vector<vector<TTree*>>(m_MaxNHitsX+1, vector<TTree*>(m_MaxNHitsY+1, nullptr));
  
  for (unsigned int x = 1; x <= m_MaxNHitsX; ++x) {
    for (unsigned int y = 1; y <= m_MaxNHitsY; ++y) {
      MERStripPairingDataSet* DS = new MERStripPairingDataSet();
      DS->Initialize(x, y);
      MString Name("StripPairing_");
      Name += x;
      Name += "_";
      Name += y;
      m_Trees[x][y] = DS->CreateTree(Name);
      m_DataSets[x][y] = DS;
    }
  }

  // We only can save once at the end, thus:
  m_SaveAfter = numeric_limits<unsigned long>::max();
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseStripPairingTMVAEventFile::Save()
{
  // Nothing here, only save at the end
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Shuffle the strips
void MResponseStripPairingTMVAEventFile::ShuffleStrips(vector<unsigned int>& StripIDs, vector<double>& Energies)
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


bool MResponseStripPairingTMVAEventFile::Analyze()
{
  // Create the strip pairing response
  
  // Initialize the next matching event, save if necessary
  if (MResponseBuilder::Analyze() == false) return false;
  
  // We need to have at least an "INIT" in the simulation file per event 
  if (m_SiEvent->GetNIAs() == 0) {
    return true;
  }
  
  // We require the initial raw event
  MRERawEvent* RE = m_ReReader->GetInitialRawEvent();
  if (RE == nullptr) {
    return true;
  }
  
  // (I) Find the ideal interaction points
  
  
  
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
  
  // (1) Collect the data
  for (unsigned int r = 0; r < RESEs.size(); ++r) {
    MRESE* R = RESEs[r];
    MDVolumeSequence* VS = R->GetVolumeSequence();
    
    VolumeTree.push_back(VS->ToStringVolumes().GetHash()); // x 
    VolumeTree.push_back(VolumeTree.back()); // y = x
    
    MVector PositionInDetector = VS->GetPositionInSensitiveVolume();
    MDGridPoint GP = VS->GetDetector()->GetGridPoint(PositionInDetector);
    
    IsXStrip.push_back(true);
    StripID.push_back(GP.GetXGrid());
    StripEnergy.push_back(R->GetEnergy());
    StripVolumeSequences.push_back(VS);
    Ignore.push_back(false);
    Origins.push_back(GetOriginIds(R));
    
    IsXStrip.push_back(false);
    StripID.push_back(GP.GetYGrid());
    StripEnergy.push_back(R->GetEnergy());
    StripVolumeSequences.push_back(VS);
    Ignore.push_back(false);
    Origins.push_back(GetOriginIds(R));
  }
  
  
  // (2) Combine hits in same strip
  for (unsigned int s1 = 0; s1 < VolumeTree.size(); ++s1) {
    if (Ignore[s1] == true) continue;
    for (unsigned int s2 = s1+1; s2 < VolumeTree.size(); ++s2) {
      if (Ignore[s2] == true) continue;
      if (VolumeTree[s1] != VolumeTree[s2]) continue;
      if (IsXStrip[s1] != IsXStrip[s2]) continue;
      if (StripID[s1] != StripID[s2]) continue;
      
      // Match:
      StripEnergy[s1] += StripEnergy[s2];
      StripEnergy[s2] = 0;
      Ignore[s2] = true;
    }
  }
  
  
  // (3) Noise & thresholds
  m_ReGeometry->ActivateNoising(true);
  for (unsigned int s1 = 0; s1 < VolumeTree.size(); ++s1) {
    if (Ignore[s1] == true) continue;
   
    MVector Pos = StripVolumeSequences[s1]->GetPositionInDetector();
    double Energy = StripEnergy[s1];
    double Time = 0;
    MDVolume* Volume = StripVolumeSequences[s1]->GetSensitiveVolume();
    MDDetector* D = StripVolumeSequences[s1]->GetDetector();

    D->Noise(Pos, Energy, Time, Volume);
    
    StripEnergy[s1] = Energy;
    if (Energy == 0) Ignore[s1] = true;
  }
  m_ReGeometry->ActivateNoising(false);

  
  // (4) Assemble by detector and write to file
  for (unsigned int s1 = 0; s1 < VolumeTree.size(); ++s1) {
    if (Ignore[s1] == true) continue;
    
    vector<unsigned int> XStripIDs;
    vector<unsigned int> YStripIDs;
    vector<double> XStripEnergies;
    vector<double> YStripEnergies;
    vector<int> AllOrigins;
    
    // (a) Collect
    if (IsXStrip[s1] == true) {
      XStripIDs.push_back(StripID[s1]);
      XStripEnergies.push_back(StripEnergy[s1]);
    } else {
      YStripIDs.push_back(StripID[s1]);
      YStripEnergies.push_back(StripEnergy[s1]);
    }
    for (int O: Origins[s1]) {
      if (find(AllOrigins.begin(), AllOrigins.end(), O) == AllOrigins.end()) {
        AllOrigins.push_back(O);
      }
    }
    Ignore[s1] = true;
    
    for (unsigned int s2 = s1+1; s2 < VolumeTree.size(); ++s2) {
      if (Ignore[s2] == true) continue;
      if (VolumeTree[s1] != VolumeTree[s2]) continue;

      if (IsXStrip[s2] == true) {
        XStripIDs.push_back(StripID[s2]);
        XStripEnergies.push_back(StripEnergy[s2]);
      } else {
        YStripIDs.push_back(StripID[s2]);
        YStripEnergies.push_back(StripEnergy[s2]);
      }
      for (int O: Origins[s2]) {
        if (find(AllOrigins.begin(), AllOrigins.end(), O) == AllOrigins.end()) {
          AllOrigins.push_back(O);
        }
      }
      Ignore[s2] = true;      
    }
    
    // (b) Shuffle -- otherwise we will get patterns...
    ShuffleStrips(XStripIDs, XStripEnergies);
    ShuffleStrips(YStripIDs, YStripEnergies);
    

    // (c) Create list of intersections with energy deposits
    
    // All intersections
    unsigned int NIntersections = 0;
    bool UndetectedHit = false;
    vector<double> AllIntersections(XStripIDs.size() * YStripIDs.size(), 0.0); // sorting x + y*XStripIDs.size()
    
    vector<unsigned int> EvaluationXStripIDs;
    vector<unsigned int> EvaluationYStripIDs;
    for (unsigned int r = 0; r < RESEs.size(); ++r) {
      MVector PositionInDetector = StripVolumeSequences[s1]->GetPositionInVolume(RESEs[r]->GetPosition(), StripVolumeSequences[s1]->GetDetectorVolume());
      if (StripVolumeSequences[s1]->GetDetectorVolume()->GetShape()->IsInside(PositionInDetector) == true) {
        MDGridPoint GP = StripVolumeSequences[s1]->GetDetector()->GetGridPoint(PositionInDetector);
        if (GP.GetType() != MDGridPoint::c_Unknown) {
          
          bool xFound = false;
          unsigned int xID = 0;
          for (unsigned int i = 0; i < XStripIDs.size(); ++i) {
            if (GP.GetXGrid() == XStripIDs[i]) {
              xFound = true;
              xID = i;
              break;
            }
          }
          bool yFound = false;
          unsigned int yID = 0;
          for (unsigned int i = 0; i < YStripIDs.size(); ++i) {
            if (GP.GetYGrid() == YStripIDs[i]) {
              yFound = true;
              yID = i;
              break;
            }
          }
          if (xFound && yFound == true) {
            if (AllIntersections[xID + yID*XStripIDs.size()] == 0.0) {
              AllIntersections[xID + yID*XStripIDs.size()] = 1.0;
              ++NIntersections;
            }
          } else {
            UndetectedHit = true; 
          }
        }
      }
    }

    // (d) Write to file
    if (XStripIDs.size() == 0 || XStripIDs.size() > m_MaxNHitsX) continue;
    if (YStripIDs.size() == 0 || YStripIDs.size() > m_MaxNHitsY) continue;
    
    if (m_DataSets[XStripIDs.size()][YStripIDs.size()]->FillEventData(RE->GetEventID(), XStripIDs, YStripIDs, XStripEnergies, YStripEnergies) == false) {
      continue;
    }
    if (m_DataSets[XStripIDs.size()][YStripIDs.size()]->FillResultData(NIntersections, UndetectedHit, AllIntersections) == false) {
      continue;
    }
    
    m_Trees[XStripIDs.size()][YStripIDs.size()]->Fill();
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Finalize the response generation (i.e. save the data a final time )
bool MResponseStripPairingTMVAEventFile::Finalize() 
{ 
  // We only save at the end since ROOT has trouble updating the file...
  for (unsigned int x = 1; x <= m_MaxNHitsX; ++x) {
    for (unsigned int y = 1; y <= m_MaxNHitsY; ++y) {
      TFile Tree(m_ResponseName + ".x" + x + ".y" + y + ".strippairing.root", "recreate");
      Tree.cd();
      m_Trees[x][y]->Write();
      Tree.Close();      
    }
  }
  
  return MResponseBuilder::Finalize(); 
}


// MResponseStripPairingTMVAEventFile.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
