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
  
  // Re-grid the his (e.g. split charge sharing, discretize undiscritzed hits, etc.)
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
    for (MDGridPoint& P: GridPoints) NewCollection.Add(P);
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
  
  
  /*
  // Step 1.1: Split hits (e.g. charge sharing onto multiple strips)
  vector<MDGridPointCollection> Collections;
  int h_max = Event->GetNRESEs();
  for (int h = 0; h < h_max; ++h) {
    MRESE* RESE = Event->GetRESEAt(h);
    if (RESE->GetType() == MRESE::c_Hit) {
      //if (((MREHit *) (RESE))->GetDetector() != MDDetector::c_Scintillator) {
      mdebug<<"TG - Event: "<<Event->GetEventID()<<": Splitting..."<<RESE->GetEnergy()<<endl;
      // Split
      MDGridPointCollection C = ((MREHit *) (RESE))->Grid(m_Geometry);
      // Remove the hit if the collection is larger than 0 and store the collection:
      if (C.GetNGridPoints() > 0) {
        Collections.push_back(C);
        Event->RemoveRESE(RESE);
        delete RESE;
      }
    } else {
      mout<<"MRERawEvent* MFileEventsEvta::GetNextEvent(): Cannot noise a hit which is no single hit!"<<endl;
      mout<<RESE->ToString()<<endl;
    }
  }
  Event->CompressRESEs();
  
  
  if (Collections.size() > 0) {
    // Merge Collections
    vector<MDGridPointCollection> CompactCollections;
    CompactCollections.push_back(Collections[0]);
    for (unsigned int c = 1; c < Collections.size(); ++c) {
      bool Found = false;
      for (unsigned int cc = 0; cc < CompactCollections.size(); ++cc) {
        if (Collections[c].HasSameDetector(CompactCollections[cc])) {
          CompactCollections[cc].Add(Collections[c]);
          Found = true;
          break;
        }
      }
      if (Found == false) {
        CompactCollections.push_back(Collections[c]);
      }
    }
    
    // Create hits:
    for (unsigned int c = 0; c < CompactCollections.size(); ++c) {
      for (unsigned int p = 0; p < CompactCollections[c].GetNGridPoints(); ++p) {
        MDGridPoint P = CompactCollections[c].GetGridPointAt(p);
        MREHit* Hit = new MREHit();
        Hit->SetEnergy(P.GetEnergy());
        Hit->SetPosition(CompactCollections[c].GetWorldPositionGridPointAt(p));
        Hit->SetTime(P.GetTime());
        Hit->UpdateVolumeSequence(m_Geometry);
        Hit->SetDetector(Hit->GetVolumeSequence()->GetDetector()->GetType());
        Hit->RetrieveResolutions(m_Geometry);
        Event->AddRESE(Hit);
      }
    }
  }
  
  
  
  // Step 1.2: Noise hits:
  h_max = Event->GetNRESEs();
  for (int h = 0; h < h_max; ++h) {
    if (Event->GetRESEAt(h)->GetType() == MRESE::c_Hit) {
      //if (((MREHit *) (Event->GetRESEAt(h)))->GetDetector() != MDDetector::c_Scintillator) {
      mdebug<<"TG - Event: "<<Event->GetEventID()<<": Noising..."<<Event->GetRESEAt(h)->GetEnergy()<<endl;
      if (((MREHit *) (Event->GetRESEAt(h)))->Noise(m_Geometry) == false) {
        // No energy: voxel bad, not in detector volume, etc:
        mdebug<<"TG - Event: "<<Event->GetEventID()<<": Rejecting hit (noise)!"<<endl;
        RESE = Event->GetRESEAt(h);
        Event->RemoveRESE(RESE);
        delete RESE;
      }
    } else {
      mout<<"MRERawEvent* MFileEventsEvta::GetNextEvent(): Cannot noise a hit which is no single hit!"<<endl;
      mout<<Event->GetRESEAt(h)->ToString()<<endl;
    }
  }
  Event->CompressRESEs();
  
  
  
  // Step 1.3: Noise additional measurements:
  for (vector<MREAM*>::iterator Iter = Event->GetREAMBegin();
       Iter != Event->GetREAMEnd(); ) {
    if ((*Iter)->GetType() == MREAM::c_DriftChamberEnergy) {
      MREAMDriftChamberEnergy* DCE = dynamic_cast<MREAMDriftChamberEnergy*>(*Iter);
      double Energy = DCE->GetEnergy();
      MDVolumeSequence* V = DCE->GetVolumeSequence();
      dynamic_cast<MDDriftChamber*>(V->GetDetector())->NoiseLightEnergy(Energy);
      DCE->SetEnergy(Energy);
      DCE->SetEnergyResolution(dynamic_cast<MDDriftChamber*>(V->GetDetector())->GetLightEnergyResolution(Energy));
      ++Iter;
    } else if ((*Iter)->GetType() == MREAM::c_GuardRingHit) {
      MREAMGuardRingHit* GR = dynamic_cast<MREAMGuardRingHit*>(*Iter);
      double Energy = GR->GetEnergy();
      MDVolumeSequence* V = GR->GetVolumeSequence();
      if (V->GetDetector()->HasGuardRing() == false || V->GetDetector()->GetGuardRing()->IsActive() == false) {
        Event->DeleteREAM(Iter);
      } else {
        MVector DummyPosition;
        double DummyTime = 0.0;
        V->GetDetector()->GetGuardRing()->Noise(DummyPosition, Energy, DummyTime, nullptr);
        GR->SetEnergy(Energy);
        GR->SetEnergyResolution(V->GetDetector()->GetGuardRing()->GetEnergyResolution(Energy));
        if (Energy == 0) {
          Event->DeleteREAM(Iter);
        } else {
          ++Iter;
        }
      }
    } else if ((*Iter)->GetType() == MREAM::c_Directional) {
      MREAMDirectional* DR = dynamic_cast<MREAMDirectional*>(*Iter);
      MVector Direction = DR->GetDirection();
      MDVolumeSequence* V = DR->GetVolumeSequence();
      MVector Center(0.0, 0.0, 0.0);
      Center = V->GetPositionInVolume(Center, V->GetDetectorVolume());
      Direction = (V->GetPositionInVolume(Direction, V->GetDetectorVolume()) - Center).Unit();
      dynamic_cast<MDStrip3DDirectional*>(V->GetDetector())->NoiseDirection(Direction, DR->GetEnergy());
      Center = MVector(0.0, 0.0, 0.0);
      Center = V->GetPositionInFirstVolume(Center, V->GetDetectorVolume());
      Direction = (V->GetPositionInFirstVolume(Direction, V->GetDetectorVolume()) - Center).Unit();
      DR->SetDirection(Direction);
      if (Direction == g_VectorNotDefined) {
        Event->DeleteREAM(Iter);
      } else {
        ++Iter;
      }
    } else {
      ++Iter;
    }
       }
       
       
       
       // Step 1.4: Test trigger
       Trigger->Reset();
       h_max = Event->GetNRESEs();
       for (int h = 0; h < h_max; ++h) {
         RESE = Event->GetRESEAt(h);
         if (RESE->GetType() == MRESE::c_Hit) {
           if (Trigger->AddHit(RESE->GetEnergy(), *(RESE->GetVolumeSequence())) == false) {

           }
         } else {
           merr<<"We shouldn't have anything else but hits at this point!"<<endl;
         }
       }
       Event->CompressRESEs();
       for (unsigned int a = 0; a < Event->GetNREAMs(); ++a) {
         if (Event->GetREAMAt(a)->GetType() == MREAM::c_GuardRingHit) {
           MREAMGuardRingHit* GR = dynamic_cast<MREAMGuardRingHit*>(Event->GetREAMAt(a));
           Trigger->AddGuardRingHit(GR->GetEnergy(), *(GR->GetVolumeSequence()));
         }
       }
       
       
       if (Trigger->HasVetoed() == true) {
         ++m_NVetoedEvents;
         
         vector<MString> TriggerNames = Trigger->GetTriggerNameList();
         for (MString S: TriggerNames) {
           m_VetoMapTriggerNames[S]++; 
         }
         vector<MString> VetoNames = Trigger->GetVetoNameList();
         for (MString S: VetoNames) {
           m_VetoMapVetoNames[S]++; 
         }
         
         h_max = Event->GetNRESEs();
         for (int h = 0; h < h_max; ++h) {
           RESE = Event->GetRESEAt(h);
           Event->RemoveRESE(RESE);
           mdebug<<"TG - Event: "<<Event->GetEventID()<<": Rejecting hit (event not triggered)!"<<endl;
           delete RESE;
         }
         Event->CompressRESEs();
         mdebug<<"TG - Event: "<<Event->GetEventID()<<" vetoed by: "<<TriggerName<<endl;
       } else if (Trigger->HasTriggered()) {
         ++m_NTriggeredEvents;
         
         vector<MString> TriggerNames = Trigger->GetTriggerNameList();
         for (MString S: TriggerNames) {
           m_TriggerMapTriggerNames[S]++; 
         }
         mdebug<<"TG - Event: "<<Event->GetEventID()<<" triggered with: "<<TriggerName<<endl;
       } else {
         ++m_NNotTriggeredOrVetoedEvents;
         
         h_max = Event->GetNRESEs();
         for (int h = 0; h < h_max; ++h) {
           RESE = Event->GetRESEAt(h);
           Event->RemoveRESE(RESE);
           mdebug<<"TG - Event: "<<Event->GetEventID()<<": Rejecting hit (event not triggered)!"<<endl;
           delete RESE;
         }
         Event->CompressRESEs();
         mdebug<<"TG - Event: "<<Event->GetEventID()<<" not triggered or vetoed!"<<endl;
       }
       
       
       // Step c: Link the MREAMDirectional to hits:
       for (vector<MREAM*>::iterator Iter = Event->GetREAMBegin();
            Iter != Event->GetREAMEnd(); ++Iter) {
         if ((*Iter)->GetType() == MREAM::c_Directional) {
           MREAMDirectional* DR = dynamic_cast<MREAMDirectional*>(*Iter);
           // Find a belonging hit:
           h_max = Event->GetNRESEs();
           for (int h = 0; h < h_max; ++h) {
             RESE = Event->GetRESEAt(h);
             if (RESE->GetType() == MRESE::c_Hit) {
               if (RESE->GetVolumeSequence()->HasSameDetector(DR->GetVolumeSequence()) == true) {
                 if (RESE->GetVolumeSequence()->GetGridPoint() == DR->GetVolumeSequence()->GetGridPoint()) {
                   DR->SetRESE(RESE);
                 }
               }
             } else {
               merr<<"We shouldn't have anything else but hits at this point!"<<endl;
             }
           }
         }
            }
            // If a linked RESE appears twice then remove the info of the one with the lower energy:
            for (vector<MREAM*>::iterator Iter = Event->GetREAMBegin();
                 Iter != Event->GetREAMEnd(); ++Iter) {
              if ((*Iter)->GetType() == MREAM::c_Directional) {
                MREAMDirectional* DR1 = dynamic_cast<MREAMDirectional*>(*Iter);
                for (vector<MREAM*>::iterator Iter2 = Iter+1;
                     Iter2 != Event->GetREAMEnd(); ++Iter2) {
                  if ((*Iter2)->GetType() == MREAM::c_Directional) {
                    MREAMDirectional* DR2 = dynamic_cast<MREAMDirectional*>(*Iter2);
                    if (DR1->GetRESE() != 0 && DR2->GetRESE() != 0) {
                      if (DR1->GetRESE()->GetID() == DR2->GetRESE()->GetID()) {
                        if (DR1->GetEnergy() < DR2->GetEnergy()) {
                          DR1->SetRESE(0);
                        } else {
                          DR2->SetRESE(0);
                        }
                      }
                    }
                  }
                     }
              }
                 }
                 // Remove all REAMs with no RESE:
                 for (vector<MREAM*>::iterator Iter = Event->GetREAMBegin();
                      Iter != Event->GetREAMEnd(); ) {
                   if ((*Iter)->GetType() == MREAM::c_Directional) {
                     MREAMDirectional* DR = dynamic_cast<MREAMDirectional*>(*Iter);
                     // If there is none remove the info:
                     if (DR->GetRESE() == 0) {
                       Event->DeleteREAM(Iter);
                     } else {
                       ++Iter;
                     }
                   } else {
                     ++Iter;
                   }
                      }
                      
                      return true;
                      

  return true; 
  */
}
  
  
////////////////////////////////////////////////////////////////////////////////


// MDDetectorEffectsEngine.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
