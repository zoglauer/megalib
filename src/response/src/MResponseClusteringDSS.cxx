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
#include "MResponseMatrixO4.h"
#include "MResponseMatrixO5.h"
#include "MResponseMatrixO6.h"
#include "MResponseMatrixO7.h"
#include "MSettingsRevan.h"
#include "MRESEIterator.h"
#include "MRECluster.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MResponseClusteringDSS)
#endif


////////////////////////////////////////////////////////////////////////////////


MResponseClusteringDSS::MResponseClusteringDSS() : MResponseBase()
{
  // Construct an instance of MResponseClusteringDSS
}


////////////////////////////////////////////////////////////////////////////////


MResponseClusteringDSS::~MResponseClusteringDSS()
{
  // Delete this instance of MResponseClusteringDSS
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseClusteringDSS::OpenFiles()
{
  // Load the simulation file --- has to be called after the geometry is loaded

  m_ReReader = new MRawEventAnalyzer();
  m_ReReader->SetGeometry(m_ReGeometry);
  if (m_ReReader->SetInputModeFile(m_SimulationFileName) == false) return false;

  // We ignore this configuration file - we only need clustering
  m_ReReader->SetCoincidenceAlgorithm(MRawEventAnalyzer::c_CoincidenceAlgoNone);
  m_ReReader->SetClusteringAlgorithm(MRawEventAnalyzer::c_ClusteringAlgoAdjacent);
  m_ReReader->SetTrackingAlgorithm(MRawEventAnalyzer::c_TrackingAlgoNone);
  m_ReReader->SetCSRAlgorithm(MRawEventAnalyzer::c_CSRAlgoNone);
  m_ReReader->SetDecayAlgorithm(MRawEventAnalyzer::c_DecayAlgoNone);

  m_ReReader->SetAdjacentLevel(2);
  m_ReReader->SetAdjacentSigma(-1);
  
  
  if (m_ReReader->PreAnalysis() == false) return false;

  m_SiReader = new MFileEventsSim(m_SiGeometry);
  if (m_SiReader->Open(m_SimulationFileName) == false) return false;

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseClusteringDSS::CreateResponse()
{
  // Create the multiple Compton response

  if ((m_SiGeometry = LoadGeometry(false, 0.0)) == 0) return false;
  if ((m_ReGeometry = LoadGeometry(true, 0.0)) == 0) return false;

  if (OpenFiles() == false) return false;

  // create axis:
  vector<float> EnergyAxis;
  EnergyAxis = CreateEquiDist(0, 10000, 100);
  
  vector<float> StripsAxis;
  StripsAxis = CreateEquiDist(1.5, 10.5, 9);



  MResponseMatrixO2 SeparableYes("SeparableYes", EnergyAxis, StripsAxis);
  SeparableYes.SetAxisNames("Energy [keV]", "Combined hit strips");
  
  MResponseMatrixO2 SeparableNo("SeparableNo", EnergyAxis, StripsAxis);
  SeparableNo.SetAxisNames("Energy [keV]", "Combined hit strips");
  
  // create axis:
  vector<float> DualEnergyAxis;
  DualEnergyAxis = CreateLogDist(15, 5000, 30, 1, 10000);
  // create axis:
  vector<float> DualDepthAxis = { 0, 0.05, 0.1, 0.2, 0.4, 0.8, 1.5 };


  MResponseMatrixO3 DualSeparableYes("DualSeparableYes", DualEnergyAxis, DualEnergyAxis, DualDepthAxis);
  DualSeparableYes.SetAxisNames("Energy [keV]", "Energy [keV]", "Depth difference [cm]");
  
  MResponseMatrixO3 DualSeparableNo("DualSeparableNo", DualEnergyAxis, DualEnergyAxis, DualDepthAxis);
  DualSeparableNo.SetAxisNames("Energy [keV]", "Energy [keV]", "Depth difference [cm]");


  int Counter = 0;
  while (InitializeNextMatchingEvent() == true) {

    if (g_Verbosity > c_Chatty) cout<<"Match: "<<m_SiEvent->GetId()<<endl;
    
    MRawEventList* REList = m_ReReader->GetRawEventList();

    if (REList->GetNRawEvents() != 1) {
      cout<<"Something went wrong: we have more than one good event"<<endl;
      continue;
    }
    
    MRERawEvent* RawEvent = REList->GetRawEventAt(0);
    int e_max = RawEvent->GetNRESEs();
    
    int NClusters = 0;
    for (int e = 0; e < e_max; ++e) {
      if (RawEvent->GetRESEAt(e)->GetType() == MRESE::c_Cluster) NClusters++;
    }
    if (NClusters == 0) continue;
    
    if (g_Verbosity > c_Chatty) cout<<"# Clusters: "<<NClusters<<endl;
    if (g_Verbosity > c_Chatty) cout<<m_SiEvent->ToSimString()<<endl;
    
    
    for (int e = 0; e < e_max; ++e) {
      MRESE* RESE = RawEvent->GetRESEAt(e);

      if (RESE->GetType() != MRESE::c_Cluster) continue;

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
        SeparableYes.Add(Cluster->GetEnergy(), Cluster->GetNRESEs());
        if (Cluster->GetNRESEs() == 2) {
          MVector PosInDet0 = Cluster->GetRESEAt(0)->GetVolumeSequence()->GetPositionInSensitiveVolume();
          MVector PosInDet1 = Cluster->GetRESEAt(1)->GetVolumeSequence()->GetPositionInSensitiveVolume();
          double DepthDifference = fabs(PosInDet0.Z() - PosInDet1.Z());
          if (Cluster->GetRESEAt(0)->GetEnergy() < Cluster->GetRESEAt(1)->GetEnergy()) {
            DualSeparableYes.Add(Cluster->GetRESEAt(0)->GetEnergy(), Cluster->GetRESEAt(1)->GetEnergy(), DepthDifference);
          } else  {
            DualSeparableYes.Add(Cluster->GetRESEAt(1)->GetEnergy(), Cluster->GetRESEAt(0)->GetEnergy(), DepthDifference);           
          }
        }
      } else {
        if (g_Verbosity > c_Chatty) cout<<"Result: Necessary clustering"<<endl;
        SeparableNo.Add(Cluster->GetEnergy(), Cluster->GetNRESEs());
        if (Cluster->GetNRESEs() == 2) {
          MVector PosInDet0 = Cluster->GetRESEAt(0)->GetVolumeSequence()->GetPositionInSensitiveVolume();
          MVector PosInDet1 = Cluster->GetRESEAt(1)->GetVolumeSequence()->GetPositionInSensitiveVolume();
          double DepthDifference = fabs(PosInDet0.Z() - PosInDet1.Z());
          if (Cluster->GetRESEAt(0)->GetEnergy() < Cluster->GetRESEAt(1)->GetEnergy()) {
            DualSeparableNo.Add(Cluster->GetRESEAt(0)->GetEnergy(), Cluster->GetRESEAt(1)->GetEnergy(), DepthDifference);
          } else  {
            DualSeparableNo.Add(Cluster->GetRESEAt(1)->GetEnergy(), Cluster->GetRESEAt(0)->GetEnergy(), DepthDifference);           
          }
        }
      }
    }
    
    if (++Counter % m_SaveAfter == 0) {
      SeparableYes.Write(m_ResponseName + ".allseparable.yes" + m_Suffix, true);
      SeparableNo.Write(m_ResponseName + ".allseparable.no" + m_Suffix, true);
      DualSeparableYes.Write(m_ResponseName + ".dualseparable.yes" + m_Suffix, true);
      DualSeparableNo.Write(m_ResponseName + ".dualseparable.no" + m_Suffix, true);
    }
  }
  
  SeparableYes.Write(m_ResponseName + ".allseparable.yes" + m_Suffix, true);
  SeparableNo.Write(m_ResponseName + ".allseparable.no" + m_Suffix, true);
  DualSeparableYes.Write(m_ResponseName + ".dualseparable.yes" + m_Suffix, true);
  DualSeparableNo.Write(m_ResponseName + ".dualseparable.no" + m_Suffix, true);

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
