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


bool MDTriggerUnit::AddGuardringHit(const MVector& Position, 
                                    const double& Energy)
{

  MDVolumeSequence S = m_Geometry->GetVolumeSequence(Position, false, true);
  return AddGuardringHit(Energy, S);
}


////////////////////////////////////////////////////////////////////////////////


bool MDTriggerUnit::AddGuardringHit(const double& Energy, MDVolumeSequence& V)
{
  bool Added = false;

  if (V.GetDetector() != 0) {
    for (unsigned int t = 0; t < m_Geometry->GetNTriggers(); ++t) {
      if (V.GetDetector()->HasGuardring() == true) {
        if (V.GetDetector()->GetType() == MDDetector::c_Strip2D ||
            V.GetDetector()->GetType() == MDDetector::c_Strip3D ||
            V.GetDetector()->GetType() == MDDetector::c_Strip3DDirectional ||
            V.GetDetector()->GetType() == MDDetector::c_DriftChamber) {
          if (m_IgnoreThresholds == true || dynamic_cast<MDStrip2D*>(V.GetDetector())->IsAboveGuardringTriggerThreshold(Energy) == true) { 
            if (m_Geometry->GetTriggerAt(t)->AddGuardringHit(V) == true) {
              Added = true;
            }
          }
        } else if (V.GetDetector()->GetType() == MDDetector::c_Voxel3D) {
          if (m_IgnoreThresholds == true || dynamic_cast<MDVoxel3D*>(V.GetDetector())->IsAboveGuardringTriggerThreshold(Energy) == true) { 
            if (m_Geometry->GetTriggerAt(t)->AddGuardringHit(V) == true) {
              Added = true;
            }
          }
        } else {
          merr<<"Detector "<<V.GetDetector()->GetName()<<" has no guardring ?? !!"<<endl;
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

  // If we have one veto then we have not triggered
  if (m_IgnoreVetoes == false) {
    for (unsigned int t = 0; t < m_Geometry->GetNTriggers(); ++t) {
      if (m_Geometry->GetTriggerAt(t)->HasVetoed() == true) {
        mdebug<<m_Geometry->GetTriggerAt(t)->GetName()<<" vetoed!"<<endl;
        return false;
      }
    }
  }

  // Check for real triggers:
  for (unsigned int t = 0; t < m_Geometry->GetNTriggers(); ++t) {
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


//! Return a list of all the vetoes which have been raised
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
