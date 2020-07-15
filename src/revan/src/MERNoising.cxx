/*
 * MERNoising.cxx
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
// MERNoising
//
// Find all clusters...
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MERNoising.h"

// Standard libs:
#include <cstdlib>
#include <iomanip>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MStreams.h"
#include "MAssert.h"
#include "MREAM.h"
#include "MREAMDriftChamberEnergy.h"
#include "MREAMGuardRingHit.h"
#include "MREAMDirectional.h"
#include "MREStripHit.h"
#include "MDDriftChamber.h"
#include "MDStrip2D.h"
#include "MDStrip3D.h"
#include "MDVoxel3D.h"
#include "MDStrip3DDirectional.h"
#include "MDGuardRing.h"
#include "MDDetectorEffectsEngine.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MERNoising)
#endif


////////////////////////////////////////////////////////////////////////////////


MERNoising::MERNoising()
{
  // Construct an instance of MERNoising
}


////////////////////////////////////////////////////////////////////////////////


MERNoising::~MERNoising()
{
  // Delete this instance of MERNoising
}


////////////////////////////////////////////////////////////////////////////////


bool MERNoising::PreAnalysis()
{
  m_NTriggeredEvents = 0;
  m_NVetoedEvents = 0;
  m_NNotTriggeredOrVetoedEvents = 0;
  
  m_TriggerMapTriggerNames.clear();
  m_VetoMapTriggerNames.clear();
  m_VetoMapVetoNames.clear();
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MERNoising::Analyze(MRERawEvent* Event)
{
  // Do all the things required for simulated events such as noising, triggering, etc.

  // Reset the data of the engine
  MDDetectorEffectsEngine& DEE = *(m_Geometry->GetDetectorEffectsEngine());
  DEE.Reset();
  
  // Set all new data
  DEE.AddEventTime(Event->GetEventTime());
  for (int h = 0; h < Event->GetNRESEs(); ++h) {
    MRESE* RESE = Event->GetRESEAt(h);
    if (RESE->GetType() == MRESE::c_Hit) {
      MREHit* Hit = dynamic_cast<MREHit*>(RESE);
      DEE.AddHit(Hit->GetPosition(), Hit->GetEnergy(), Hit->GetTime(), Hit->GetOriginIDs(), *(Hit->GetVolumeSequence()));
    } else {
      // Nasty complaint 
      mout<<"MRERawEvent* MFileEventsEvta::GetNextEvent(): Cannot noise a hit which is no single hit!"<<endl;
      mout<<RESE->ToString()<<endl;
    }
  }
  for (vector<MREAM*>::iterator Iter = Event->GetREAMBegin(); Iter != Event->GetREAMEnd(); ) {
    if ((*Iter)->GetType() == MREAM::c_GuardRingHit) {
      MREAMGuardRingHit* GR = dynamic_cast<MREAMGuardRingHit*>(*Iter);
      DEE.AddHit(MVector(0.0, 0.0, 0.0), GR->GetEnergy(), 0.0, set<unsigned int>(), *(GR->GetVolumeSequence()));
      Event->DeleteREAM(Iter);
    } else {
      ++Iter;
    }
  }
      
  
  
  // Run the engine
  DEE.Run();
  
  // Clear the old data
  Event->DeleteAll();
  Event->CompressRESEs();
  
  // Retrieve the noised data
  const vector<MDGridPointCollection>& GridPointCollections = DEE.GetGridPointCollections();
  
  // Create hits:
  //cout<<GridPointCollections.size()<<endl;
  for (unsigned int c = 0; c < GridPointCollections.size(); ++c) {
    //cout<<GridPointCollections[c].GetNGridPoints()<<endl;
    for (unsigned int p = 0; p < GridPointCollections[c].GetNGridPoints(); ++p) {
      const MDGridPoint& P = GridPointCollections[c].GetGridPointAt(p);

      
      
      if (P.GetType() == MDGridPoint::c_XStrip || P.GetType() == MDGridPoint::c_YStrip) {
        MREStripHit* Hit = new MREStripHit();
        Hit->SetEnergy(P.GetEnergy());
        
        Hit->SetTime(P.GetTime());
        Hit->SetNoiseFlags(P.GetFlags());
        //cout<<"Pos of P: "<<P.GetPosition()<<endl;
        //cout<<"Worl P: "<<GridPointCollections[c].GetWorldPositionGridPointAt(p)<<endl;
        Hit->SetPosition(GridPointCollections[c].GetWorldPositionGridPointAt(p));
        Hit->SetVolumeSequence(new MDVolumeSequence(GridPointCollections[c].GetVolumeSequence()));
        Hit->SetDetector(Hit->GetVolumeSequence()->GetDetector()->GetType());
        Hit->SetEnergyResolution(Hit->GetVolumeSequence()->GetDetector()->GetEnergyResolution(P.GetEnergy()));
        //Hit->RetrieveResolutions(m_Geometry);
        //Hit->SetPosition(MVector(0, 0, 0));
        
        MString DetectorID = "";
        for (unsigned int v = 0; v < Hit->GetVolumeSequence()->GetNVolumes(); ++v) {
          DetectorID += Hit->GetVolumeSequence()->GetVolumeAt(v)->GetName();
        }
        
        Hit->SetDetectorID(DetectorID.GetHash());
        Hit->IsXStrip((P.GetType() == MDGridPoint::c_XStrip) ? true : false);
        Hit->SetStripID((P.GetType() == MDGridPoint::c_XStrip) ? P.GetXGrid() : P.GetYGrid());
        Hit->SetDepthPosition(P.GetPosition().Z());
        //cout<<"Setting pos: "<<((P.GetType() == MDGridPoint::c_XStrip) ? P.GetPosition().X() : P.GetPosition().Y())<<endl;
        Hit->SetNonStripPosition((P.GetType() == MDGridPoint::c_XStrip) ? P.GetPosition().X() : P.GetPosition().Y());
        
        Hit->AddOriginIDs(P.GetOriginIDs());
        
        Hit->SetGridPoint(P);
        
        Event->AddRESE(Hit);
      } else {
        MREHit* Hit = new MREHit();
        Hit->SetEnergy(P.GetEnergy());
        Hit->SetPosition(GridPointCollections[c].GetWorldPositionGridPointAt(p));
        Hit->SetTime(P.GetTime());
        Hit->SetNoiseFlags(P.GetFlags());
        Hit->UpdateVolumeSequence(m_Geometry);
        Hit->SetDetector(Hit->GetVolumeSequence()->GetDetector()->GetType());
        Hit->RetrieveResolutions(m_Geometry);
        Hit->SetGridPoint(P);
        
        Hit->AddOriginIDs(P.GetOriginIDs());
        
        Event->AddRESE(Hit);
      }
      
      //cout<<Hit->ToString()<<endl;
      //cout<<"Added"<<endl;
    }
  }
  
  // Set the time
  Event->SetEventTime(DEE.GetEventTime());
  
  return true;
  
  /*
  MRESE* RESE = 0;
  MString TriggerName;
  MDTriggerUnit* Trigger = m_Geometry->GetTriggerUnit();

  // Step 1: Noise

  // Noise the system time
  MDSystem* System = m_Geometry->GetSystem();
  if (System != 0) {
    MTime Time = Event->GetEventTime();
    System->NoiseTime(Time);
    Event->SetEventTime(Time);
  }
  
  
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
        //
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
  */

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MERNoising::AddStatistics(MERNoising* Noising)
{
  // Add the statistics
  
  for (map<MString, long>::const_iterator Iter = Noising->m_TriggerMapTriggerNames.begin(); Iter != Noising->m_TriggerMapTriggerNames.end(); ++Iter) {
    m_TriggerMapTriggerNames[(*Iter).first] += (*Iter).second;
  }

  for (map<MString, long>::const_iterator Iter = Noising->m_VetoMapTriggerNames.begin(); Iter != Noising->m_VetoMapTriggerNames.end(); ++Iter) {
    m_VetoMapTriggerNames[(*Iter).first] += (*Iter).second;
  }
  for (map<MString, long>::const_iterator Iter = Noising->m_VetoMapVetoNames.begin(); Iter != Noising->m_VetoMapVetoNames.end(); ++Iter) {
    m_VetoMapVetoNames[(*Iter).first] += (*Iter).second;
  }
  
  m_NTriggeredEvents += Noising->m_NTriggeredEvents;
  m_NVetoedEvents += Noising->m_NVetoedEvents;
  m_NNotTriggeredOrVetoedEvents += Noising->m_NNotTriggeredOrVetoedEvents;
}


////////////////////////////////////////////////////////////////////////////////


MString MERNoising::ToString(bool) const
{
  ostringstream out;

  int Width = 6;
  size_t Length = 50;

  if (m_NNotTriggeredOrVetoedEvents > 0 || m_NTriggeredEvents > 0 || m_NVetoedEvents > 0) {
    out<<endl;
    out<<"Trigger statistics:"<<endl;
    out<<"  Not triggered events: .................................. "<<setw(Width)<<m_NNotTriggeredOrVetoedEvents<<endl;

    out<<"  Number of vetoed events: ............................... "<<setw(Width)<<m_NVetoedEvents<<endl;
    if (m_NVetoedEvents > 0) {
      out<<"    Raised vetoes (multiples possible)"<<endl;
      for (auto Iter = m_VetoMapVetoNames.begin(); Iter != m_VetoMapVetoNames.end(); ++Iter) {
        size_t L = (*Iter).first.Length();
        out<<"      "<<(*Iter).first<<": ";
        if (L < Length) for (size_t l = 0; l < Length-L; ++l) out<<".";
        out<<" "<<setw(Width)<<(*Iter).second<<endl;
      }
      out<<"    Raised triggers canceled by a veto (multiples possible)"<<endl;
      for (auto Iter = m_VetoMapTriggerNames.begin(); Iter != m_VetoMapTriggerNames.end(); ++Iter) {
        size_t L = (*Iter).first.Length();
        out<<"      "<<(*Iter).first<<": ";
        if (L < Length) for (size_t l = 0; l < Length-L; ++l) out<<".";
        out<<" "<<setw(Width)<<(*Iter).second<<endl;
      }
    }
    
    out<<"  Number of triggered events: ............................ "<<setw(Width)<<m_NTriggeredEvents<<endl;
    if (m_NTriggeredEvents > 0) {
      out<<"    Raised triggers (multiples possible)"<<endl;
      for (auto Iter = m_TriggerMapTriggerNames.begin(); Iter != m_TriggerMapTriggerNames.end(); ++Iter) {
        size_t L = (*Iter).first.Length();
        out<<"      "<<(*Iter).first<<": ";
        if (L < Length) for (size_t l = 0; l < Length-L; ++l) out<<".";
        out<<" "<<setw(Width)<<(*Iter).second<<endl;
      }
    }
    out<<endl;
  }

  return out.str().c_str();
}


// MERNoising.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
