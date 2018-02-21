/*
 * MResponseClusteringDSS.cxx
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
// MResponseClusteringDSS
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MResponseClusteringDSS.h"

// Standard libs:
#include <limits>
using namespace std;

// ROOT libs:
#include "TMath.h"

// MEGAlib libs:
#include "MAssert.h"
#include "MStreams.h"
#include "MResponseMatrixO1.h"
#include "MResponseMatrixO2.h"
#include "MResponseMatrixO3.h"

#include "MSettingsRevan.h"
#include "MRESEIterator.h"
#include "MRECluster.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MResponseClusteringDSS)
#endif


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MResponseClusteringDSS::MResponseClusteringDSS() : MResponseBuilder()
{
  m_ResponseNameSuffix = "clustering";
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
MResponseClusteringDSS::~MResponseClusteringDSS()
{
  // Nothing to delete
}

  
////////////////////////////////////////////////////////////////////////////////


//! Initialize the response matrices and their generation
bool MResponseClusteringDSS::Initialize() 
{ 
  if (MResponseBuilder::Initialize() == false) return false;

  // We ignore the loaded configuration file - we only need clustering
  m_ReReader->SetCoincidenceAlgorithm(MRawEventAnalyzer::c_CoincidenceAlgoNone);
  m_ReReader->SetHitClusteringAlgorithm(MRawEventAnalyzer::c_HitClusteringAlgoAdjacent);
  m_ReReader->SetTrackingAlgorithm(MRawEventAnalyzer::c_TrackingAlgoNone);
  m_ReReader->SetCSRAlgorithm(MRawEventAnalyzer::c_CSRAlgoNone);
  m_ReReader->SetDecayAlgorithm(MRawEventAnalyzer::c_DecayAlgoNone);

  m_ReReader->SetAdjacentLevel(2);
  m_ReReader->SetAdjacentSigma(-1);
    
  if (m_ReReader->PreAnalysis() == false) return false;

  // Create matrices:
  vector<float> EnergyAxis;
  EnergyAxis = CreateEquiDist(0, 10000, 100);
  
  vector<float> StripsAxis;
  StripsAxis = CreateEquiDist(1.5, 10.5, 9);

  m_SeparableYes.SetName("SeparableYes"); 
  m_SeparableYes.SetAxis(EnergyAxis, StripsAxis);
  m_SeparableYes.SetAxisNames("Energy [keV]", "Combined hit strips");
  
  m_SeparableNo.SetName("SeparableNo");
  m_SeparableNo.SetAxis(EnergyAxis, StripsAxis);
  m_SeparableNo.SetAxisNames("Energy [keV]", "Combined hit strips");
  
  // create axis:
  vector<float> DualEnergyAxis;
  DualEnergyAxis = CreateLogDist(15, 5000, 30, 1, 10000);
  // create axis:
  vector<float> DualDepthAxis = { 0, 0.05, 0.1, 0.2, 0.4, 0.8, 1.5 };


  m_DualSeparableYes.SetName("DualSeparableYes");
  m_DualSeparableYes.SetAxis(DualEnergyAxis, DualEnergyAxis, DualDepthAxis);
  m_DualSeparableYes.SetAxisNames("Energy [keV]", "Energy [keV]", "Depth difference [cm]");
  
  m_DualSeparableNo.SetName("DualSeparableNo");
  m_DualSeparableNo.SetAxis(DualEnergyAxis, DualEnergyAxis, DualDepthAxis);
  m_DualSeparableNo.SetAxisNames("Energy [keV]", "Energy [keV]", "Depth difference [cm]");
  
  
  return true;
}

  
////////////////////////////////////////////////////////////////////////////////


//! Analyze the current event
bool MResponseClusteringDSS::Analyze() 
{ 
  // Initialize next matching event, save if necessary
  if (MResponseBuilder::Analyze() == false) return false;
  
  int e_max = m_ReEvent->GetNRESEs();
  
  int NClusters = 0;
  for (int e = 0; e < e_max; ++e) {
    if (m_ReEvent->GetRESEAt(e)->GetType() == MRESE::c_Cluster) NClusters++;
  }
  if (NClusters == 0) return true;
  
  if (g_Verbosity > c_Chatty) cout<<"# Clusters: "<<NClusters<<endl;
  if (g_Verbosity > c_Chatty) cout<<m_SiEvent->ToSimString()<<endl;
  
  
  for (int e = 0; e < e_max; ++e) {
    MRESE* RESE = m_ReEvent->GetRESEAt(e);
    
    if (RESE->GetType() != MRESE::c_Cluster) return true;
    
    MRECluster* Cluster = dynamic_cast<MRECluster*>(RESE);
    if (g_Verbosity > c_Chatty) mout<<"Looking at cluster: "<<Cluster->ToString()<<endl;
    
    vector<vector<int>> Origins;
    for (int r = 0; r < Cluster->GetNRESEs(); ++r) {
      Origins.push_back(GetOriginIds(Cluster->GetRESEAt(r)));
    }
    
    // A cluster is not necessary when
    // (a) not two RESEs have the same origin ID
    // (b) the origin of one RESEs is not in another RESE
    
    bool UnnecessaryClustering = true;
    // Check (a):
    for (int r = 0; r < Cluster->GetNRESEs(); ++r) {
      for (int rp = r+1; rp < Cluster->GetNRESEs(); ++rp) {
        for (unsigned int o = 0; o < Origins[r].size(); ++o) {
          for (unsigned int op = 0; op < Origins[rp].size(); ++op) {
            // Check (a)
            if (Origins[r][o] == Origins[rp][op]) {
              if (g_Verbosity > c_Chatty) cout<<"Same origin check: "<<Origins[r][o]<<":"<<Origins[rp][op]<<endl;
              UnnecessaryClustering = false;
              break;
            }
            // Check (b)
            if (HasAsAncestor(Origins[r][o], Origins[rp][op]) == true ||
              HasAsAncestor(Origins[rp][op], Origins[r][o]) == true) {
              if (g_Verbosity > c_Chatty) cout<<"Ancestor check: "<<Origins[r][o]<<":"<<Origins[rp][op]<<endl;               
              UnnecessaryClustering = false;
            break;
              }
          }
          if (UnnecessaryClustering == false) break;
        }
        if (UnnecessaryClustering == false) break;
      }
      if (UnnecessaryClustering == false) break;
    }
    
    if (UnnecessaryClustering == true) {
      if (g_Verbosity > c_Chatty) cout<<"Result: Unnecessary clustering"<<endl;
      m_SeparableYes.Add(Cluster->GetEnergy(), Cluster->GetNRESEs());
      if (Cluster->GetNRESEs() == 2) {
        MVector PosInDet0 = Cluster->GetRESEAt(0)->GetVolumeSequence()->GetPositionInSensitiveVolume();
        MVector PosInDet1 = Cluster->GetRESEAt(1)->GetVolumeSequence()->GetPositionInSensitiveVolume();
        double DepthDifference = fabs(PosInDet0.Z() - PosInDet1.Z());
        if (Cluster->GetRESEAt(0)->GetEnergy() < Cluster->GetRESEAt(1)->GetEnergy()) {
          m_DualSeparableYes.Add(Cluster->GetRESEAt(0)->GetEnergy(), Cluster->GetRESEAt(1)->GetEnergy(), DepthDifference);
        } else  {
          m_DualSeparableYes.Add(Cluster->GetRESEAt(1)->GetEnergy(), Cluster->GetRESEAt(0)->GetEnergy(), DepthDifference);           
        }
      }
    } else {
      if (g_Verbosity > c_Chatty) cout<<"Result: Necessary clustering"<<endl;
      m_SeparableNo.Add(Cluster->GetEnergy(), Cluster->GetNRESEs());
      if (Cluster->GetNRESEs() == 2) {
        MVector PosInDet0 = Cluster->GetRESEAt(0)->GetVolumeSequence()->GetPositionInSensitiveVolume();
        MVector PosInDet1 = Cluster->GetRESEAt(1)->GetVolumeSequence()->GetPositionInSensitiveVolume();
        double DepthDifference = fabs(PosInDet0.Z() - PosInDet1.Z());
        if (Cluster->GetRESEAt(0)->GetEnergy() < Cluster->GetRESEAt(1)->GetEnergy()) {
          m_DualSeparableNo.Add(Cluster->GetRESEAt(0)->GetEnergy(), Cluster->GetRESEAt(1)->GetEnergy(), DepthDifference);
        } else  {
          m_DualSeparableNo.Add(Cluster->GetRESEAt(1)->GetEnergy(), Cluster->GetRESEAt(0)->GetEnergy(), DepthDifference);           
        }
      }
    }
  }
  
  
  return true;
}

  
////////////////////////////////////////////////////////////////////////////////


//! Finalize the response generation (i.e. save the data a final time )
bool MResponseClusteringDSS::Finalize() 
{ 
  return MResponseBuilder::Finalize(); 
}


////////////////////////////////////////////////////////////////////////////////


//! Save the data
bool MResponseClusteringDSS::Save()
{
  MResponseBuilder::Save(); 

  m_SeparableYes.Write(GetFilePrefix() + ".allseparable.yes" + m_Suffix, true);
  m_SeparableNo.Write(GetFilePrefix() + ".allseparable.no" + m_Suffix, true);
  m_DualSeparableYes.Write(GetFilePrefix() + ".dualseparable.yes" + m_Suffix, true);
  m_DualSeparableNo.Write(GetFilePrefix() + ".dualseparable.no" + m_Suffix, true);

  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Check if the first ID has the second one as ancestor
bool MResponseClusteringDSS::HasAsAncestor(int ID, int AncestorID) 
{
  // Check we have a good ID
  if (ID <= 0 || AncestorID <= 0) return false;
  
  // An ancestor always has a smaller ID
  if (ID <= AncestorID) return false;
  
  int Origin = ID;
  while (Origin != 0 && Origin > AncestorID) {
    Origin = m_SiEvent->GetIAAt(Origin-1)->GetOriginID();
    if (Origin == AncestorID) return true;
  }
  
  return false;
}


// MResponseClusteringDSS.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
