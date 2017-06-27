/*
 * MDTriggerUnit.cxx
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
// MDTriggerUnit
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MDTriggerUnit.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MAssert.h"
#include "MStreams.h"
#include "MDDetector.h"
#include "MDGeometry.h"
#include "MDTrigger.h"
#include "MDVolume.h"
#include "MDStrip2D.h"
#include "MDVoxel3D.h"
#include "MDGuardRing.h"
#include "MDTriggerBasic.h"
#include "MDTriggerMap.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MDTriggerUnit)
#endif


////////////////////////////////////////////////////////////////////////////////


MDTriggerUnit::MDTriggerUnit(MDGeometry* Geometry)
{
  // Construct an instance of MDTriggerUnit

  m_Geometry = Geometry;

  m_AlwaysAssumeTrigger = false;  
  m_AlwaysAssumeVeto = false;

  m_IgnoreVetoes = false;
  m_IgnoreThresholds = false;
}


////////////////////////////////////////////////////////////////////////////////


MDTriggerUnit::~MDTriggerUnit()
{
  // Delete this instance of MDTriggerUnit
  
  m_Geometry = 0;
}


////////////////////////////////////////////////////////////////////////////////


void MDTriggerUnit::Reset()
{
  for (unsigned int t = 0; t < m_Geometry->GetNTriggers(); ++t) {
    m_Geometry->GetTriggerAt(t)->Reset();
  }
}


////////////////////////////////////////////////////////////////////////////////


bool MDTriggerUnit::Validate() const
{
  // Validate the trigger unit
  
  // Check that we do not mix Basic and Universal triggers classes
  if (m_Geometry->GetNTriggers() > 0) {
    MDTriggerType Type = m_Geometry->GetTriggerAt(0)->GetType();
    for (unsigned int t = 1; t < m_Geometry->GetNTriggers(); ++t) {
      if (m_Geometry->GetTriggerAt(t)->GetType() != Type) {
        mout<<"   ***  Error  ***  in trigger unit"<<endl;
        mout<<"You cannot mix trigger class (e.g. basic and universal)"<<endl;
        return false;
      }
    }
  }
  
  // Check if all detectors are existing for universal triggers
  for (unsigned int t = 0; t < m_Geometry->GetNTriggers(); ++t) {
    if (m_Geometry->GetTriggerAt(t)->GetType() == MDTriggerType::c_Universal) {
      vector<MString> DetectorNames = dynamic_cast<MDTriggerMap*>(m_Geometry->GetTriggerAt(t))->GetDetectors();
      
      vector<MDDetector*> Detectors = m_Geometry->GetDetectorList();
      for (MString Name: DetectorNames) {
        bool Found = false;
        for (unsigned int d = 0; d < Detectors.size(); ++d) {
          if (Detectors[d]->GetName() == Name) {
            Found = true;
            break;
          }
        }
        if (Found == false) {
          mout<<"   ***  Error  ***  in trigger "<<m_Geometry->GetTriggerAt(t)->GetName()<<endl;
          mout<<"Unknown detector: "<<Name<<endl;
          return false;          
        }
      }
    }
  }
  
  // Make sure that all detectors which have only veto triggers have NoiseThresholdEqualsTriggerThreshold set
  for (unsigned int d = 0; d < m_Geometry->GetNDetectors(); ++d) {
    int NVetoes = 0;
    int NTriggers = 0;
    for (unsigned int t = 0; t < m_Geometry->GetNTriggers(); ++t) {
      if (m_Geometry->GetTriggerAt(t)->IsVetoing(m_Geometry->GetDetectorAt(d)) == true) {
        NVetoes++;
      }
      if (m_Geometry->GetTriggerAt(t)->IsTriggering(m_Geometry->GetDetectorAt(d)) == true) {
        NTriggers++; 
      }
    }
    if (NVetoes > 0 && NTriggers == 0 && m_Geometry->GetDetectorAt(d)->GetNoiseThresholdEqualsTriggerThreshold() == false) {
      mout<<"   ***  Error  ***  Triggers with vetoes"<<endl;
      mout<<"A detector (here: "<<m_Geometry->GetDetectorAt(d)->GetName()<<"), which only has veto triggers, must have the flag \"NoiseThresholdEqualsTriggerThreshold true\"!"<<endl;
      return false;
    }
  }  
  
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Set a flag indicating that vetoes are ignored and transfer it to all triggers
void MDTriggerUnit::IgnoreVetoes(bool IgnoreVetoesFlag) 
{ 
  m_IgnoreVetoes = IgnoreVetoesFlag; 
  for (unsigned int t = 0; t < m_Geometry->GetNTriggers(); ++t) {
    m_Geometry->GetTriggerAt(t)->IgnoreVetoes(m_IgnoreVetoes);
  }
}


////////////////////////////////////////////////////////////////////////////////


bool MDTriggerUnit::AddHit(const MVector& Position, 
                           const double& Energy)
{
  MDVolumeSequence S = m_Geometry->GetVolumeSequence(Position, false, true);
  return AddHit(Energy, S);
}


////////////////////////////////////////////////////////////////////////////////


bool MDTriggerUnit::AddHit(const double& Energy, MDVolumeSequence& V)
{
  bool Added = false;

  if (V.GetDetector() != 0) {
    for (unsigned int t = 0; t < m_Geometry->GetNTriggers(); ++t) {
      mdebug<<"Trying to a hit with "<<Energy<<" keV in detector "<<V.GetDetector()->GetName()<<" to trigger "<<m_Geometry->GetTriggerAt(t)->GetName()<<endl;
      if (m_IgnoreThresholds == true || V.GetDetector()->IsAboveTriggerThreshold(Energy, V.GetGridPoint()) == true) { 
        mdebug<<" --> Above trigger threshold ";
        if (m_Geometry->GetTriggerAt(t)->AddHit(V) == true) {
          mdebug<<" and added"<<endl;
          Added = true;
        } else {
          mdebug<<" but NOT added"<<endl;
        }
      }
    }
  } else {
    merr<<"MDTriggerUnit: No detector for hit at position: "<<V.GetPositionAt(0)<<show;
  }

  return Added;
}


////////////////////////////////////////////////////////////////////////////////


bool MDTriggerUnit::AddGuardRingHit(const MVector& Position, 
                                    const double& Energy)
{

  MDVolumeSequence S = m_Geometry->GetVolumeSequence(Position, false, true);
  return AddGuardRingHit(Energy, S);
}


////////////////////////////////////////////////////////////////////////////////


bool MDTriggerUnit::AddGuardRingHit(const double& Energy, MDVolumeSequence& V)
{
  bool Added = false;

  if (V.GetDetector() != 0) {
    for (unsigned int t = 0; t < m_Geometry->GetNTriggers(); ++t) {
      if (V.GetDetector()->HasGuardRing() == true) {
        if (m_IgnoreThresholds == true || V.GetDetector()->GetGuardRing()->IsAboveTriggerThreshold(Energy, V.GetGridPoint()) == true) { 
          if (m_Geometry->GetTriggerAt(t)->AddGuardRingHit(V) == true) {
            Added = true;
          }
        }
      } else {
        merr<<"Detector "<<V.GetDetector()->GetName()<<" has no guardring ?? !!"<<endl;
      }
    }
  }

  return Added;
}


////////////////////////////////////////////////////////////////////////////////


bool MDTriggerUnit::HasTriggered() 
{
  if (m_AlwaysAssumeTrigger == true) {
    mdebug<<"Trigger always on!"<<endl;
    return true;
  }
  if (m_AlwaysAssumeVeto == true) {
    mdebug<<"Veto always on!"<<endl;
    return false;
  }

  // If no triggers are defined then we have not vetoed but triggered 
  if (m_Geometry->GetNTriggers() == 0) {
    mdebug<<"No triggers defined!"<<endl;
    return true;
  }

  // If we have a non-vetoable trigger, we have triggered
  for (unsigned int t = 0; t < m_Geometry->GetNTriggers(); ++t) {
    mdebug<<m_Geometry->GetTriggerAt(t)->GetName()<<": Non-vetoably triggered? "<<(m_Geometry->GetTriggerAt(t)->HasNonVetoablyTriggered() == true ? "yes" : "no")<<endl;
    if (m_Geometry->GetTriggerAt(t)->HasNonVetoablyTriggered() == true) {
      mdebug<<m_Geometry->GetTriggerAt(t)->GetName()<<" triggered!"<<endl;
      return true;
    }
  }
  
  
  // If we have one veto then we have not triggered
  if (m_IgnoreVetoes == false) { // This should not be neceassary since the triggers handle it...
    for (unsigned int t = 0; t < m_Geometry->GetNTriggers(); ++t) {
      mdebug<<m_Geometry->GetTriggerAt(t)->GetName()<<" vetoed? "<<(m_Geometry->GetTriggerAt(t)->HasVetoed() == true ? "yes" : "no")<<endl;
      if (m_Geometry->GetTriggerAt(t)->HasVetoed() == true) {
        mdebug<<m_Geometry->GetTriggerAt(t)->GetName()<<" vetoed!"<<endl;
        return false;
      }
    }
  }

  // Check for real triggers:
  for (unsigned int t = 0; t < m_Geometry->GetNTriggers(); ++t) {
    mdebug<<m_Geometry->GetTriggerAt(t)->GetName()<<" triggered? "<<(m_Geometry->GetTriggerAt(t)->HasTriggered() == true ? "yes" : "no")<<endl;
    if (m_Geometry->GetTriggerAt(t)->HasTriggered() == true) {
      mdebug<<m_Geometry->GetTriggerAt(t)->GetName()<<" triggered!"<<endl;
      return true;
    }
  }

  return false;
}


////////////////////////////////////////////////////////////////////////////////


bool MDTriggerUnit::HasVetoed() 
{
  if (m_AlwaysAssumeTrigger == true) {
    return false;
  }
  if (m_AlwaysAssumeVeto == true) {
    return true;
  }

  // If we ignore vetoes, we don't have a veto
  if (m_IgnoreVetoes == true) {
    return false;
  }

  // If no triggers are defined then we have not vetoed but triggered 
  if (m_Geometry->GetNTriggers() == 0) {
    return false;
  }
  
  // If we have a non-vetoable trigger, we have not vetoed
  for (unsigned int t = 0; t < m_Geometry->GetNTriggers(); ++t) {
    mdebug<<m_Geometry->GetTriggerAt(t)->GetName()<<" non-vetoably triggered? "<<(m_Geometry->GetTriggerAt(t)->HasNonVetoablyTriggered() == true ? "yes" : "no")<<endl;
    if (m_Geometry->GetTriggerAt(t)->HasNonVetoablyTriggered() == true) {
      mdebug<<m_Geometry->GetTriggerAt(t)->GetName()<<" no vetoed!"<<endl;
      return false;
    }
  }
  
  // If we have one veto then we have not triggered
  for (unsigned int t = 0; t < m_Geometry->GetNTriggers(); ++t) {
    if (m_Geometry->GetTriggerAt(t)->HasVetoed() == true) {
      mdebug<<m_Geometry->GetTriggerAt(t)->GetName()<<" vetoed!"<<endl;
      return true;
    }
  }

  return false;
}


////////////////////////////////////////////////////////////////////////////////


//! Return a list of all the triggers which have been raised
vector<MString> MDTriggerUnit::GetTriggerNameList()
{
  vector<MString> List;
  
  for (unsigned int t = 0; t < m_Geometry->GetNTriggers(); ++t) {
    if (m_Geometry->GetTriggerAt(t)->HasTriggered() == true) {
      List.push_back(m_Geometry->GetTriggerAt(t)->GetName());
    }
  }
  
  return List;
}


////////////////////////////////////////////////////////////////////////////////


//! Return a list of all the vetoes which have been raised
vector<MString> MDTriggerUnit::GetVetoNameList()
{
  vector<MString> List;
  
  // Check for vetoes:
  for (unsigned int t = 0; t < m_Geometry->GetNTriggers(); ++t) {
    if (m_Geometry->GetTriggerAt(t)->HasVetoed() == true) {
      List.push_back(m_Geometry->GetTriggerAt(t)->GetName());
    }
  }
  
  return List;
}


////////////////////////////////////////////////////////////////////////////////


ostream& operator<<(ostream& os, const MDTriggerUnit& R)
{
  os<<"Trigger unit:"<<endl;
  
  // Check for vetoes:
  for (unsigned int t = 0; t < R.m_Geometry->GetNTriggers(); ++t) {
    os<<*(R.m_Geometry->GetTriggerAt(t));
  }

  return os;
}


// MDTriggerUnit.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
