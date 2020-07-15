/*
 * MDTriggerBasic.cxx
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
// MDTriggerBasic
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MDTriggerBasic.h"

// Standard libs:
#include <sstream>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MAssert.h"
#include "MStreams.h"
#include "MDDetector.h"
#include "MDGuardRing.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MDTriggerBasic)
#endif


////////////////////////////////////////////////////////////////////////////////


MDTriggerBasic::MDTriggerBasic(const MString& Name) : MDTrigger(Name)
{
  // Construct an instance of MDTriggerBasic

  m_TriggerType = MDTriggerType::c_Basic;

  m_IsVeto = false;
  m_IsTriggerByDetector = true; 
}


////////////////////////////////////////////////////////////////////////////////


MDTriggerBasic::~MDTriggerBasic()
{
  // Delete this instance of MDTriggerBasic
}


////////////////////////////////////////////////////////////////////////////////


MString MDTriggerBasic::GetName() const
{
  // Return the name of this trigger condition

  return m_Name;
}


////////////////////////////////////////////////////////////////////////////////


void MDTriggerBasic::SetDetectorType(const int Detectortype, const unsigned int Hits)
{
  // Set the trigger parameter Deterctortype and the minimum number of hits
  // in this detector

  m_DetectorTypes.push_back(Detectortype);
  m_Hits.push_back(Hits);
  m_Types.push_back(c_Detector);

  m_TriggerTest.resize(m_TriggerTest.size()+1);
  m_VolumeSequenceTest.resize(m_VolumeSequenceTest.size()+1);
}


////////////////////////////////////////////////////////////////////////////////


void MDTriggerBasic::SetDetector(MDDetector* Detector, const unsigned int Hits)
{
  // Set the trigger parameter Deterctortype and the minimum number of hits
  // in this detector

  m_Detectors.push_back(Detector);
  m_Hits.push_back(Hits);
  m_Types.push_back(c_Detector);

  m_TriggerTest.resize(m_TriggerTest.size()+1);
  m_VolumeSequenceTest.resize(m_VolumeSequenceTest.size()+1);
}


////////////////////////////////////////////////////////////////////////////////


void MDTriggerBasic::SetGuardRingDetectorType(const int Detectortype, const unsigned int Hits)
{
  // Set the trigger parameter Deterctortype and the minimum number of hits
  // in this detector

  m_DetectorTypes.push_back(Detectortype);
  m_Hits.push_back(Hits);
  m_Types.push_back(c_GuardRing);

  m_TriggerTest.resize(m_TriggerTest.size()+1);
  m_VolumeSequenceTest.resize(m_VolumeSequenceTest.size()+1);
}


////////////////////////////////////////////////////////////////////////////////


void MDTriggerBasic::SetGuardRingDetector(MDDetector* Detector, const unsigned int Hits)
{
  // Set the trigger parameter Deterctortype and the minimum number of hits
  // in this detector

  // At this stage the guard rings are not yet set --> do it during validate
  //if (Detector->GetType() != MDDetector::c_GuardRing) {
  //  if (Detector->HasGuardRing() == true) {
  //    Detector = dynamic_cast<MDDetector*>(Detector->GetGuardRing()); 
  //  }
  //}
  
  m_Detectors.push_back(Detector);
  m_Hits.push_back(Hits);
  m_Types.push_back(c_GuardRing);

  m_TriggerTest.resize(m_TriggerTest.size()+1);
  m_VolumeSequenceTest.resize(m_VolumeSequenceTest.size()+1);
}


////////////////////////////////////////////////////////////////////////////////


void MDTriggerBasic::Reset()
{
  for (unsigned int i = 0; i < m_TriggerTest.size(); ++i) {
    m_TriggerTest[i].clear();
    m_VolumeSequenceTest[i].clear();
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Return true if this trigger is triggering (not vetoing)
bool MDTriggerBasic::IsTriggering(MDDetector* D) const 
{ 
  if (Applies(D) == true && m_IsVeto == false) {
    return true;
  }
  
  return false; 
}


////////////////////////////////////////////////////////////////////////////////


//! Return true if this trigger is vetoing
bool MDTriggerBasic::IsVetoing(MDDetector* D) const 
{ 
  if (Applies(D) == true && m_IsVeto == true) {
    return true;
  }
  
  return false; 
}  


////////////////////////////////////////////////////////////////////////////////


bool MDTriggerBasic::AddHit(const MDVolumeSequence& VS) 
{
  massert(VS.GetDetector() != 0);

  bool Return = false;

  //mdebug<<m_Name<<": trying to add hit...";

  if (m_IsTriggerByDetector == false) {
    const MDDetector* Detector = VS.GetDetector();
    if (Detector->IsNamedDetector() == true) Detector = Detector->GetNamedAfterDetector();
    if (m_DetectorTypes.size() != 0) {
      for (unsigned int i = 0; i < m_DetectorTypes.size(); ++i) {
        if (m_DetectorTypes[i] == Detector->GetType() &&
            m_Types[i] == c_Detector) {
          m_TriggerTest[i].push_back(1);
          //mdebug<<" done!";
          Return = true;
        }
      }
    } else {
      for (unsigned int i = 0; i < m_Detectors.size(); ++i) {
        if (m_Detectors[i]->GetName() == Detector->GetName() &&
            m_Types[i] == c_Detector) {
          m_TriggerTest[i].push_back(1);
          //mdebug<<" done!";
          Return = true;
        }
      }
    }
  } else {
    const MDDetector* Detector = VS.GetDetector();
    if (Detector->IsNamedDetector() == true) Detector = Detector->GetNamedAfterDetector();
    if (m_DetectorTypes.size() != 0) {
      for (unsigned int i = 0; i < m_DetectorTypes.size(); ++i) {
        if (m_DetectorTypes[i] == Detector->GetType() &&
            m_Types[i] == c_Detector) {
          bool Exists = false;
          for (unsigned int v = 0; v < m_VolumeSequenceTest[i].size(); ++v) {
            if (VS.HasSameDetector(m_VolumeSequenceTest[i][v]) == true) {
              m_TriggerTest[i][v]++;
              Exists = true;
              //mdebug<<" done!";
              Return = true;
              break;
            }
          }
          if (Exists == false) {
            m_VolumeSequenceTest[i].push_back(VS);
            m_TriggerTest[i].push_back(1);
            //mdebug<<" done!";
            Return = true;
          }
          break;
        }
      }
    } else {
      for (unsigned int i = 0; i < m_Detectors.size(); ++i) {
        if (m_Detectors[i]->GetName() == Detector->GetName() &&
            m_Types[i] == c_Detector) {
          bool Exists = false;
          for (unsigned int v = 0; v < m_VolumeSequenceTest[i].size(); ++v) {
            if (VS.HasSameDetector(m_VolumeSequenceTest[i][v]) == true) {
              m_TriggerTest[i][v]++;
              Exists = true;
              //mdebug<<" done!";
              Return = true;
              break;
            }
          }
          if (Exists == false) {
            m_VolumeSequenceTest[i].push_back(VS);
            m_TriggerTest[i].push_back(1);
            //mdebug<<" done!";
            Return = true;
          }
          break;
        }
      }
    }
  }
  //mdebug<<endl;

  return Return;
}


////////////////////////////////////////////////////////////////////////////////


bool MDTriggerBasic::AddGuardRingHit(const MDVolumeSequence& VS) 
{
  massert(VS.GetDetector() != 0);

  bool Return = false;

  //mdebug<<m_Name<<": adding guard ring hit"<<endl;

  if (m_IsTriggerByDetector == false) {
    const MDDetector* Detector = VS.GetDetector();
    if (Detector->IsNamedDetector() == true) Detector = Detector->GetNamedAfterDetector();
    if (m_DetectorTypes.size() != 0) {
      for (unsigned int i = 0; i < m_DetectorTypes.size(); ++i) {
        if (m_DetectorTypes[i] == Detector->GetType() && m_Types[i] == c_GuardRing) {
          m_TriggerTest[i].push_back(1);
          //mdebug<<" done!";
          Return = true;
          break;
        }
      }
    } else {
      for (unsigned int i = 0; i < m_Detectors.size(); ++i) {
        //cout<<m_Detectors[i]->GetName()<<":"<<Detector->GetName()<<endl;
        if (m_Detectors[i]->GetName() == Detector->GetName() && m_Types[i] == c_GuardRing) {
          m_TriggerTest[i].push_back(1);
          //mdebug<<" done!";
          Return = true;
          break;
        }
      }
    }
  } else {
    const MDDetector* Detector = VS.GetDetector();
    if (Detector->IsNamedDetector() == true) Detector = Detector->GetNamedAfterDetector();
    if (m_DetectorTypes.size() != 0) {
      for (unsigned int i = 0; i < m_DetectorTypes.size(); ++i) {
        if (m_DetectorTypes[i] == Detector->GetType() && m_Types[i] == c_GuardRing) {
          bool Exists = false;
          for (unsigned int v = 0; v < m_VolumeSequenceTest[i].size(); ++v) {
            if (VS.HasSameDetector(m_VolumeSequenceTest[i][v]) == true) {
              m_TriggerTest[i][v]++;
              Exists = true;
              //mdebug<<" done!";
              Return = true;
              break;
            }
          }
          if (Exists == false) {
            m_VolumeSequenceTest[i].push_back(VS);
            m_TriggerTest[i].push_back(1);
            //mdebug<<" done!";
            Return = true;
          }
          break;
        }
      }
    } else {
      for (unsigned int i = 0; i < m_Detectors.size(); ++i) {
        //cout<<m_Detectors[i]->GetName()<<":"<<Detector->GetName()<<endl;
        if (m_Detectors[i]->GetName() == Detector->GetName() && m_Types[i] == c_GuardRing) {
          bool Exists = false;
          for (unsigned int v = 0; v < m_VolumeSequenceTest[i].size(); ++v) {
            //cout<<VS.GetDetector()->GetName()<<":"<<m_VolumeSequenceTest[i][v].GetDetector()->GetName()<<endl;
            if (VS.HasSameDetector(m_VolumeSequenceTest[i][v]) == true) {
              m_TriggerTest[i][v]++;
              Exists = true;
              //mdebug<<" done!";
              Return = true;
              break;
            }
          }
          if (Exists == false) {
            m_VolumeSequenceTest[i].push_back(VS);
            m_TriggerTest[i].push_back(1);
            ///mdebug<<" done!";
            Return = true;
          }
          break;
        }
      }
    }
  }
  ///mdebug<<endl;

  return Return;
}


////////////////////////////////////////////////////////////////////////////////


bool MDTriggerBasic::HasTriggered()
{
  ///mdebug<<m_Name<<": Trigger test: Triggered!"<<endl;

  // If this is a veto trigger, then we cannot have triggered...
  if (m_IsVeto == true) {
    ///mdebug<<m_Name<<": Trigger test: N/A - this is a veto trigger"<<endl;
    return false;
  }
  
  if (m_Hits.size() == 0) {
    //mdebug<<m_Name<<": Trigger test: Not triggered (no trigger criteria)!"<<endl;
    return false;
  }

  for (unsigned int h = 0; h < m_Hits.size(); ++h) {
    if (m_TriggerTest[h].size() < m_Hits[h]) {
      //mdebug<<m_Name<<": Trigger test: Not triggered!"<<endl;
      return false;
    } 
  }

  //mdebug<<m_Name<<": Trigger test: Triggered!"<<endl;
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MDTriggerBasic::HasVetoed()
{
  // If we ignore vetoes, we never veto...
  if (m_IgnoreVetoes == true) {
    ///mdebug<<m_Name<<": Veto test: N/A - we are ignoring vetoes"<<endl;
    return false;    
  }
  
  // If this is not a veto trigger, then we cannot have vetoed...
  if (m_IsVeto == false) {
    //mdebug<<m_Name<<": Veto test: N/A - this is a NOT veto trigger"<<endl;
    return false;
  }
  
  if (m_Hits.size() == 0) {
    //mdebug<<m_Name<<": Veto test: Not vetoes (no veto criteria)!"<<endl;
    return false;
  }
  
  for (unsigned int h = 0; h < m_Hits.size(); ++h) {
    if (m_TriggerTest[h].size() < m_Hits[h]) {
      //mdebug<<m_Name<<": Veto test: Not vetod"<<endl;
      return false;
    } 
  }

  //mdebug<<m_Name<<": Veto test: Veto!"<<endl;
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MDTriggerBasic::Applies(MDDetector* Detector) const
{
  //! Return true if this trigger applies to the detector
  
  for (unsigned int i = 0; i < m_DetectorTypes.size(); ++i) {
    if (m_DetectorTypes[i] == Detector->GetType() && m_Types[i] == c_Detector) {
      return true;
    }
  }

  for (unsigned int i = 0; i < m_Detectors.size(); ++i) {
    if (m_Detectors[i]->GetName() == Detector->GetName() && m_Types[i] == c_Detector) {
      return true;
    }
  }  
  
  return false;
}


////////////////////////////////////////////////////////////////////////////////


bool MDTriggerBasic::IncludesDetectorAsPositiveTrigger(MDDetector* Detector)
{
  //! Return true, if this detector is part of this trigger, excluding vetoes and guard rings

  if (m_IsVeto == true) return false;
  if (Detector->IsNamedDetector() == true) Detector = Detector->GetNamedAfterDetector();

  if (m_DetectorTypes.size() != 0) {
    for (unsigned int i = 0; i < m_DetectorTypes.size(); ++i) {
      if (m_DetectorTypes[i] == Detector->GetType() && m_Types[i] == c_Detector) {
        return true;
      }
    }
  } else {
    for (unsigned int i = 0; i < m_Detectors.size(); ++i) {
      if (m_Detectors[i]->GetName() == Detector->GetName() && m_Types[i] == c_Detector) {
        return true;
      }
    }
  }

  return false;
}


////////////////////////////////////////////////////////////////////////////////


MString MDTriggerBasic::GetGeomega() const
{
  // Return as Geomega type volume tree

  ostringstream out;
  out<<"Trigger "<<m_Name<<endl;
  out<<m_Name<<".Veto "<<((m_IsVeto == true) ? "true" : "false")<<endl;
  if (m_IsTriggerByDetector == true) {
    out<<m_Name<<".TriggerByDetector "<<true<<endl;
  } else {
    out<<m_Name<<".TriggerByChannel "<<true<<endl;
  }
  for (unsigned int d = 0; d < m_DetectorTypes.size(); ++d) {
    if (m_Types[d] == c_Detector) {
      out<<m_Name<<".DetectorType "<<MDDetector::GetDetectorTypeName(m_DetectorTypes[d])<<" "<<m_Hits[d]<<endl;
    } else if  (m_Types[d] == c_GuardRing) {
      out<<m_Name<<".GuardRingDetectorType "<<MDDetector::GetDetectorTypeName(m_DetectorTypes[d])<<" "<<m_Hits[d]<<endl;
    }
  }
  for (unsigned int d = 0; d < m_Detectors.size(); ++d) {
    if (m_Types[d] == c_Detector) {
      out<<m_Name<<".Detector "<<m_Detectors[d]->GetName()<<" "<<m_Hits[d]<<endl;
    } else if  (m_Types[d] == c_GuardRing) {
      out<<m_Name<<".GuardRingDetector "<<m_Detectors[d]->GetName()<<" "<<m_Hits[d]<<endl;
    }
  }
  
  return out.str(); 
}

////////////////////////////////////////////////////////////////////////////////


MString MDTriggerBasic::ToString() const
{
  // Return the Geant4 code of this trigger condition

  ostringstream os;

  os<<"Trigger (Basic): "<<m_Name<<endl;
  os<<"  Veto: "<<(m_IsVeto == true ? "yes" : "no")<<endl;
  
  for (unsigned int i = 0; i < m_DetectorTypes.size(); ++i) {
    os<<"  Detector types: "<<m_DetectorTypes[i]<<" "<<m_Hits[i];
    if (m_Types[i] == c_GuardRing) {
      os<<" (guard ring!)";
    }
    os<<endl;
  }
  for (unsigned int i = 0; i < m_Detectors.size(); ++i) {
    os<<"  Detector names: "<<m_Detectors[i]->GetName()<<" "<<m_Hits[i];
    if (m_Types[i] == c_GuardRing) {
      os<<" (guard ring!)";
    }
    os<<endl;
  }

  return os.str();
}


////////////////////////////////////////////////////////////////////////////////


bool MDTriggerBasic::Validate()
{
  // Make sure everything is reasonable:

  if (m_DetectorTypes.size() == 0 && m_Detectors.size() == 0) {
    mout<<"   ***  Error  ***  in trigger "<<m_Name<<endl;
    mout<<"You defined no triggering detectors!"<<endl;
    return false;
  }
  if (m_DetectorTypes.size() != 0 && m_Detectors.size() != 0) {
    mout<<"   ***  Error  ***  in trigger "<<m_Name<<endl;
    mout<<"You are not allowed to define triggering detectors AND triggering detector types!"<<endl;
    return false;
  }

  for (unsigned int d = 0; d < m_DetectorTypes.size(); ++d) {
    if (m_DetectorTypes[d] < MDDetector::c_MinDetector || 
        m_DetectorTypes[d] > MDDetector::c_MaxDetector) {
      mout<<"   ***  Error  ***  in trigger "<<m_Name<<endl;
      mout<<"Undefined detector type: "<<m_DetectorTypes[d]<<endl;
      return false;      
    }
  }
  
  for (unsigned int d = 0; d < m_DetectorTypes.size(); ++d) {
    for (unsigned int e = d+1; e < m_DetectorTypes.size(); ++e) {
      if (m_DetectorTypes[d] == m_DetectorTypes[e]) {
        mout<<"   ***  Error  ***  in trigger "<<m_Name<<endl;
        mout<<"The following detector type appears twice: "<<m_DetectorTypes[d]<<endl;
        return false;      
      }
    }
  }
  
  for (unsigned int d = 0; d < m_Detectors.size(); ++d) {
    if (m_Detectors[d]->IsNamedDetector() == true) m_Detectors[d] = m_Detectors[d]->GetNamedAfterDetector();
  }
    
  for (unsigned int d = 0; d < m_Detectors.size(); ++d) {
    for (unsigned int e = d+1; e < m_Detectors.size(); ++e) {
      if (m_Detectors[d]->GetName() == m_Detectors[e]->GetName()) {
        mout<<"   ***  Error  ***  in trigger "<<m_Name<<endl;
        mout<<"The following detector appears twice: "<<m_Detectors[d]->GetName()<<endl;
        return false;      
      }
    }
  }

  for (unsigned int t = 0; t < m_Types.size(); ++t) {
    if (m_Types[t] == c_GuardRing) { 
      if (m_Detectors[t]->GetType() != MDDetector::c_GuardRing) {
        if (m_Detectors[t]->HasGuardRing() == true) {
          m_Detectors[t] = dynamic_cast<MDDetector*>(m_Detectors[t]->GetGuardRing()); 
        } else {
          mout<<"   ***  Error  ***  in trigger "<<m_Name<<endl;
          mout<<"Guard ring veto without guard ring detector: "<<m_Detectors[t]->GetName()<<endl;
          return false;
        }
      }
    }
  }
  
  
  // For veto detector the noise threshold should be equal to the trigger threshold
  /* This is currently done MDGeometry
  if (m_IsVeto == true) {
    for (unsigned int d = 0; d < m_Detectors.size(); ++d) {
      if (m_Detectors[d]->GetNoiseThresholdEqualsTriggerThreshold() == false) {
        mout<<"   ***  Serious warning  ***  in trigger "<<m_Name<<endl;
        mout<<"A veto detector should have always set the flag \"NoiseThresholdEqualsTriggerThreshold\": "<<m_Detectors[d]->GetName()<<endl;
        mout<<"Otherwise hits below the trigger threshold but above the noise threshold will stay in the hit list for event freconstruction!"<<endl;
      }
    }
    for (unsigned int t = 0; t < m_DetectorTypes.size(); ++t) {
      
    }
  }
  */
  
  
  bool IsGuardRing = false;
  for (unsigned int j = 0; j < m_Types.size(); j++) {
    if (m_Types[j] == c_GuardRing) {
      IsGuardRing = true;
      break;
    }
  }
  if (IsGuardRing == true && m_IsVeto == false) {
    //mout<<"   ***  Error  ***  in trigger "<<m_Name<<endl;
    //mout<<"A guard ring must always be combined with a veto trigger!"<<endl;
    //mout<<"Anything else wouldn't work during pre-triggering in the simulation"<<endl;
    //return false;      
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


ostream& operator<<(ostream& os, const MDTriggerBasic& Trigger)
{
  os<<Trigger.ToString();

  return os;
}


// MDTriggerBasic.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
