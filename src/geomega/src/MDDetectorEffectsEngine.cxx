/*
 * MDDetectorEffectsEngine.cxx
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


// Include the header:
#include "MDDetectorEffectsEngine.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MDGeometry.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MDDetectorEffectsEngine)
#endif


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MDDetectorEffectsEngine::MDDetectorEffectsEngine()
{
  m_Geometry = nullptr;
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
MDDetectorEffectsEngine::~MDDetectorEffectsEngine()
{
}


////////////////////////////////////////////////////////////////////////////////


//! Validate the trigger unit - call after all data is set
bool MDDetectorEffectsEngine::Validate() const
{
  if (m_Geometry == nullptr) {
    mout<<"   ***  Error  ***  in detector effects engine"<<endl;
    mout<<"You need to set a geometry."<<endl;
    return false;
  }
 
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Reset the stored event data - call before each new event
void MDDetectorEffectsEngine::Reset()
{
  m_EventTime.Set(0);
  m_GridPoints.clear();
  m_VolumeSequences.clear();
  m_GridPointCollections.clear();
}


////////////////////////////////////////////////////////////////////////////////


//! Add a hit
void MDDetectorEffectsEngine::AddHit(const MVector& Position, const double& Energy, const double& Time, const MDVolumeSequence& S)
{
  m_GridPoints.push_back(MDGridPoint(0, 0, 0, MDGridPoint::c_Unknown, Position, Energy, Time));
  m_VolumeSequences.push_back(S);
  if (S.GetDetector()->GetType() == MDDetector::c_GuardRing) {
    m_GridPoints.back().SetType(MDGridPoint::c_GuardRing);
    //cout<<"Guardring"<<endl;
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Run the detector effects engine
bool MDDetectorEffectsEngine::Run()
{
  // Noise the event time
  MDSystem* System = m_Geometry->GetSystem();
  System->NoiseTime(m_EventTime);
  
  // Re-grid the hits (e.g. split charge sharing, discretize undiscritzed hits, etc.)
  m_GridPointCollections.clear();
  for (unsigned int p = 0; p < m_GridPoints.size(); ++p) {
    const MDGridPoint& P = m_GridPoints[p];
    const MDVolumeSequence& S = m_VolumeSequences[p];
    
    // First correctly rotate the position resolution into detector coordinates:
    MVector Pos = P.GetPosition();
    for (unsigned int i = 0; i < S.GetNVolumes(); i++) {
      // Translate:
      Pos -= S.GetVolumeAt(i)->GetPosition();
      // Rotate:
      if (S.GetVolumeAt(i)->IsRotated() == true) {
        Pos = S.GetVolumeAt(i)->GetRotationMatrix() * Pos;
      }
    }
    
    // Grid the hit, the returned grid point positions are in detector coordinates (although the only relevant infromation is the grid)
    //cout<<"Type: "<<S.GetDetector()->GetType()<<endl;
    vector<MDGridPoint> GridPoints = S.GetDetector()->Grid(Pos, P.GetEnergy(), P.GetTime(), S.GetDeepestVolume());
    
    // Add the grid points to the grid point collections
    MDGridPointCollection NewCollection(S);
    for (MDGridPoint& NP: GridPoints) NewCollection.Add(NP);
    
    if (P.GetEnergy() > 650) {
      int x = 0;
      int y = 0;
      double EnergyN = 0;
      for (unsigned int p = 0; p < NewCollection.GetNGridPoints(); ++p) {
        EnergyN += NewCollection.GetGridPointAt(p).GetEnergy();
        //cout<<NewCollection.GetGridPointAt(p).GetType()<<endl;
        if (NewCollection.GetGridPointAt(p).GetType() == MDGridPoint::c_XStrip) x += 1;
        if (NewCollection.GetGridPointAt(p).GetType() == MDGridPoint::c_YStrip) y += 1;
      }
      //cout<<"Spread "<<x<<" "<<y<<" "<<EnergyN/2.0<<" "<<P.GetEnergy()<<endl;
    }
    
    
    //cout<<"GPs: "<<NewCollection.GetNGridPoints()<<endl;
    
    bool Found = false;
    for (unsigned int c = 0; c < m_GridPointCollections.size(); ++c) {
      if (m_GridPointCollections[c].HasSameDetector(NewCollection)) {
        m_GridPointCollections[c].Add(NewCollection);
        Found = true;
        break;
      }
    }
    if (Found == false) {
      m_GridPointCollections.push_back(NewCollection);
    }
  }
  
  // Noise all hits
  for (unsigned int c = 0; c < m_GridPointCollections.size(); ++c) {
    const MDVolumeSequence& S = m_GridPointCollections[c].GetVolumeSequence();
    for (unsigned int p = 0; p < m_GridPointCollections[c].GetNGridPoints(); ++p) {
      MDGridPoint& P = m_GridPointCollections[c].GetGridPointAt(p);
      
      // Apply the noise -- the position is already in detector coordinates
      MVector Position = P.GetPosition();
      
      double Energy = P.GetEnergy();
      double Time = P.GetTime();
      
      MString Flags = P.GetFlags();
      S.GetDetector()->Noise(Position, Energy, Time, Flags, S.GetDeepestVolume());
      
      // Update position, energy, and time
      P.SetPosition(Position);
      P.SetEnergy(Energy);
      P.SetTime(Time);
      P.SetFlags(Flags);
    }
  }
      
  // Apply triggers
  MDTriggerUnit& TU = *(m_Geometry->GetTriggerUnit());
  TU.Reset();
  
  for (unsigned int c = 0; c < m_GridPointCollections.size(); ++c) {
    const MDVolumeSequence& S = m_GridPointCollections[c].GetVolumeSequence();
    for (unsigned int p = 0; p < m_GridPointCollections[c].GetNGridPoints(); ++p) {
      MDGridPoint& P = m_GridPointCollections[c].GetGridPointAt(p);
     
      bool AboveTriggerThreshold = false;
      if (P.GetType() == MDGridPoint::c_GuardRing) {
        //cout<<"Trigger guard ring"<<endl;
        AboveTriggerThreshold = TU.AddGuardRingHit(P.GetEnergy(), S);
      } else {
        AboveTriggerThreshold = TU.AddHit(P.GetEnergy(), S);
      }
      //cout<<(AboveTriggerThreshold ? "true" : "false")<<endl;
      P.IsAboveTriggerThreshold(AboveTriggerThreshold);
    }
  }
  
  //cout<<"Triggered: "<<(TU.HasTriggered() ? "true" : "false")<<": vetoed: "<<(TU.HasVetoed() ? "true" : "false")<<endl;
  if (TU.HasTriggered() == false || TU.HasVetoed() == true) {
    //cout<<"Clearing"<<endl;
    m_GridPointCollections.clear();
    return false;
  }
  
  // Apply read-outs
  for (unsigned int c = 0; c < m_GridPointCollections.size(); ++c) {
    const MDVolumeSequence& S = m_GridPointCollections[c].GetVolumeSequence();
    for (unsigned int p = 0; p < m_GridPointCollections[c].GetNGridPoints(); ++p) {
      MDGridPoint& P = m_GridPointCollections[c].GetGridPointAt(p);
      
      P.IsReadOut(true);
      if (P.IsAboveTriggerThreshold() == false) {
        if (S.GetDetector()->GetNoiseThresholdEqualsTriggerThreshold() == true) {
          P.IsReadOut(false);
          //cout<<"No readout: "<<P.GetEnergy()<<endl;
        } else {
          if (S.GetDetector()->IsAboveNoiseThreshold(P.GetEnergy(), P) == false) {
            P.IsReadOut(false);
            //cout<<"No readout: "<<P.GetEnergy()<<endl;
          }
        }
      }
    }
  }
  
  // Neighbors:
  
  // Look if a neighbor has triggered
  for (unsigned int c = 0; c < m_GridPointCollections.size(); ++c) {
    const MDVolumeSequence& S = m_GridPointCollections[c].GetVolumeSequence();
    for (unsigned int p = 0; p < m_GridPointCollections[c].GetNGridPoints(); ++p) {
      MDGridPoint& P = m_GridPointCollections[c].GetGridPointAt(p);
      
      if (P.IsReadOut() == false) {
        double Sigmas = S.GetDetector()->SigmasAboveNoiseLevel(P.GetEnergy(), P);
        if (Sigmas >= 7) {
          for (unsigned int p2 = 0; p2 < m_GridPointCollections[c].GetNGridPoints(); ++p2) {
            MDGridPoint& P2 = m_GridPointCollections[c].GetGridPointAt(p2);
          
            if (P2.IsAboveTriggerThreshold() == true) {
              if (fabs(P.GetXGrid() - P2.GetXGrid()) <= 1 && fabs(P.GetYGrid() - P2.GetYGrid()) <= 1) {
                P.IsReadOut(true);
                //cout<<"Found neighbor"<<endl;
              }
            }
          }
        }
      }
      
      if (P.IsReadOut() == false) {
        //cout<<"Lost: "<<P.GetEnergy()<<endl;
      }
    }
  }
  
  
  
  // Remove not read-out grid points
  for (unsigned int c = 0; c < m_GridPointCollections.size(); ++c) {
    //cout<<"B: "<<m_GridPointCollections[c].GetNGridPoints()<<endl;
    m_GridPointCollections[c].RemoveNonReadOuts();
    //cout<<"A: "<<m_GridPointCollections[c].GetNGridPoints()<<endl;
  }
  // Remove empty collections
  m_GridPointCollections.erase(std::remove_if(m_GridPointCollections.begin(), m_GridPointCollections.end(), [](const MDGridPointCollection& C) { return (C.GetNGridPoints() == 0); } ), m_GridPointCollections.end());
  
  //cout<<"Collections: "<<m_GridPointCollections.size()<<endl; 
  
  return true;
}
  
  
////////////////////////////////////////////////////////////////////////////////


// MDDetectorEffectsEngine.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
