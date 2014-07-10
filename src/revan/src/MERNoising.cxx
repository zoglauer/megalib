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
#include "MREAMGuardringHit.h"
#include "MREAMDirectional.h"
#include "MDDriftChamber.h"
#include "MDStrip2D.h"
#include "MDStrip3D.h"
#include "MDVoxel3D.h"
#include "MDStrip3DDirectional.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
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
  m_NNotTriggeredEvents = 0;
  m_TriggerMap.clear();
  m_VetoMap.clear();

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MERNoising::Analyze(MRERawEvent* Event)
{
  // Do all the things required for simulated events such as noising, triggering, etc.

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
  
  // Step 1.1: Noise hits:
  int h_max = Event->GetNRESEs();
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

  // Step 1.2: Noise additional measurements:
  for (vector<MREAM*>::iterator Iter = Event->GetREAMBegin();
       Iter != Event->GetREAMEnd(); /* handled externally due to delete */) {
    if ((*Iter)->GetType() == MREAM::c_DriftChamberEnergy) {
      MREAMDriftChamberEnergy* DCE = dynamic_cast<MREAMDriftChamberEnergy*>(*Iter);
      double Energy = DCE->GetEnergy();
      MDVolumeSequence* V = DCE->GetVolumeSequence();
      dynamic_cast<MDDriftChamber*>(V->GetDetector())->NoiseLightEnergy(Energy);
      DCE->SetEnergy(Energy);
      DCE->SetEnergyResolution(dynamic_cast<MDDriftChamber*>(V->GetDetector())->GetLightEnergyResolution(Energy));
	    ++Iter;
	  } else if ((*Iter)->GetType() == MREAM::c_GuardringHit) {
      MREAMGuardringHit* GR = dynamic_cast<MREAMGuardringHit*>(*Iter);
      double Energy = GR->GetEnergy();
      MDVolumeSequence* V = GR->GetVolumeSequence();
      if (V->GetDetector()->GetType() == MDDetector::c_Voxel3D) {
        dynamic_cast<MDVoxel3D*>(V->GetDetector())->NoiseGuardring(Energy);
        GR->SetEnergyResolution(dynamic_cast<MDVoxel3D*>(V->GetDetector())->GetGuardringEnergyResolution(Energy));
      } else if (V->GetDetector()->GetType() == MDDetector::c_Strip2D ||
                 V->GetDetector()->GetType() == MDDetector::c_Strip3D ||
                 V->GetDetector()->GetType() == MDDetector::c_Strip3DDirectional ||
                 V->GetDetector()->GetType() == MDDetector::c_DriftChamber) {
        dynamic_cast<MDStrip2D*>(V->GetDetector())->NoiseGuardring(Energy);
        GR->SetEnergyResolution(dynamic_cast<MDStrip2D*>(V->GetDetector())->GetGuardringEnergyResolution(Energy));
      } else {
        merr<<"Detector "<<V->GetDetector()->GetName()<<" has no guard ring ?? !!"<<endl;
      }
      GR->SetEnergy(Energy);
      if (Energy == 0) {
        Event->DeleteREAM(Iter);
      } else {
        ++Iter;
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

  // Step b: Test trigger
  Trigger->Reset();
  h_max = Event->GetNRESEs();
  for (int h = 0; h < h_max; ++h) {
    RESE = Event->GetRESEAt(h);
    if (RESE->GetType() == MRESE::c_Hit) {
      if (Trigger->AddHit(RESE->GetEnergy(), *(RESE->GetVolumeSequence())) == false) {
        /*
        if (RESE->GetDetector() == MDDetector::c_Scintillator) {
          mimp<<"Removing scintillator hits which have not triggered..."<<show;
          RESE = Event->GetRESEAt(h);
          Event->RemoveRESE(RESE);
          mdebug<<"TG - Event: "<<Event->GetEventID()<<": Rejecting hit (trigger not added)!"<<endl;
          delete RESE;
        }
        */
      }
    } else {
      merr<<"We shouldn't have anything else but hits at this point!"<<endl;
    }
  }
  Event->CompressRESEs();
  for (unsigned int a = 0; a < Event->GetNREAMs(); ++a) {
    if (Event->GetREAMAt(a)->GetType() == MREAM::c_GuardringHit) {
      MREAMGuardringHit* GR = dynamic_cast<MREAMGuardringHit*>(Event->GetREAMAt(a));
      Trigger->AddGuardringHit(GR->GetEnergy(), *(GR->GetVolumeSequence()));
    }
  }


  if ((TriggerName = Trigger->GetNameVeto()) != "") {
    m_VetoMap[TriggerName]++;
    h_max = Event->GetNRESEs();
    for (int h = 0; h < h_max; ++h) {
      RESE = Event->GetRESEAt(h);
      Event->RemoveRESE(RESE);
      mdebug<<"TG - Event: "<<Event->GetEventID()<<": Rejecting hit (event not triggered)!"<<endl;
      delete RESE;
    }
    Event->CompressRESEs();
    mdebug<<"TG - Event: "<<Event->GetEventID()<<" vetoed by: "<<TriggerName<<endl;
  } else if ((TriggerName = Trigger->GetNameTrigger()) != "") {
    m_TriggerMap[TriggerName]++;
    mdebug<<"TG - Event: "<<Event->GetEventID()<<" triggered with: "<<TriggerName<<endl;
  } else {
    m_NNotTriggeredEvents++;
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
       Iter != Event->GetREAMEnd(); /* handled externally due to delete ++Iter */) {
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
}


////////////////////////////////////////////////////////////////////////////////


void MERNoising::AddStatistics(MERNoising* Noising)
{
  // Add the statistics
  
  for (map<MString, int>::const_iterator Iter = Noising->m_TriggerMap.begin();
       Iter != Noising->m_TriggerMap.end(); ++Iter) {
    m_TriggerMap[(*Iter).first] += (*Iter).second;
  }

  for (map<MString, int>::const_iterator Iter = Noising->m_VetoMap.begin();
       Iter != Noising->m_VetoMap.end(); ++Iter) {
    m_VetoMap[(*Iter).first] += (*Iter).second;
  }

  m_NNotTriggeredEvents += Noising->m_NNotTriggeredEvents;
}


////////////////////////////////////////////////////////////////////////////////


MString MERNoising::ToString(bool) const
{
  ostringstream out;

  map<MString, int>::const_iterator Iter;
  int NTriggeredEvents = 0;
  int NVetoedEvents = 0;
  for (Iter = m_VetoMap.begin(); Iter != m_VetoMap.end(); ++Iter) {
    NVetoedEvents += (*Iter).second;
  }
  for (Iter = m_TriggerMap.begin(); Iter != m_TriggerMap.end(); ++Iter) {
    NTriggeredEvents += (*Iter).second;
  }

  int Width = 6;
  size_t Length = 52;

  if (m_NNotTriggeredEvents > 0 || NTriggeredEvents > 0 || NVetoedEvents > 0) {
    out<<"Trigger statistics:"<<endl;
    out<<"  Not triggered events: .................................. "<<setw(Width)<<m_NNotTriggeredEvents<<endl;

    out<<"  Number of vetoed events: ............................... "<<setw(Width)<<NVetoedEvents<<endl;
    for (Iter = m_VetoMap.begin(); Iter != m_VetoMap.end(); ++Iter) {
      size_t L = (*Iter).first.Length();
      out<<"    "<<(*Iter).first<<": ";
      if (L < Length) for (size_t l = 0; l < Length-L; ++l) out<<".";
      out<<" "<<setw(Width)<<(*Iter).second<<endl;
    }

    out<<"  Number of triggered events: ............................ "<<setw(Width)<<NTriggeredEvents<<endl;
    for (Iter = m_TriggerMap.begin(); Iter != m_TriggerMap.end(); ++Iter) {
      size_t L = (*Iter).first.Length();
      out<<"    "<<(*Iter).first<<": ";
      if (L < Length) for (size_t l = 0; l < Length-L; ++l) out<<".";
      out<<" "<<setw(Width)<<(*Iter).second<<endl;
    }
    //out<<"  If several trigger criteria are fulfilled only the first one (according to the sequence in the geometry file) is reported!"<<endl;
  }

  return out.str().c_str();
}


// MERNoising.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
