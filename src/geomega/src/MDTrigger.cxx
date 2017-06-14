/*
 * MDTrigger.cxx
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
// MDTrigger
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MDTrigger.h"

// Standard libs:
#include <sstream>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MAssert.h"
#include "MStreams.h"
#include "MDDetector.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MDTrigger)
#endif


////////////////////////////////////////////////////////////////////////////////


const int MDTrigger::c_Detector = 0;
const int MDTrigger::c_GuardRing = 1;


////////////////////////////////////////////////////////////////////////////////


MDTrigger::MDTrigger(const MString& Name)
{
  // Construct an instance of MDTrigger

  m_Name = Name;

  m_IsVeto = false;
  m_IsTriggerByDetector = true; 
}


////////////////////////////////////////////////////////////////////////////////


MDTrigger::~MDTrigger()
{
  // Delete this instance of MDTrigger
}


////////////////////////////////////////////////////////////////////////////////


MString MDTrigger::GetName() const
{
  // Return the name of this trigger condition

  return m_Name;
}


////////////////////////////////////////////////////////////////////////////////


void MDTrigger::SetDetectorType(const int Detectortype, const unsigned int Hits)
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


void MDTrigger::SetDetector(MDDetector* Detector, const unsigned int Hits)
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


void MDTrigger::SetGuardRingDetectorType(const int Detectortype, const unsigned int Hits)
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


void MDTrigger::SetGuardRingDetector(MDDetector* Detector, const unsigned int Hits)
{
  // Set the trigger parameter Deterctortype and the minimum number of hits
  // in this detector

  m_Detectors.push_back(Detector);
  m_Hits.push_back(Hits);
  m_Types.push_back(c_GuardRing);

  m_TriggerTest.resize(m_TriggerTest.size()+1);
  m_VolumeSequenceTest.resize(m_VolumeSequenceTest.size()+1);
}


////////////////////////////////////////////////////////////////////////////////


void MDTrigger::Reset()
{
  for (unsigned int i = 0; i < m_TriggerTest.size(); ++i) {
    m_TriggerTest[i].clear();
    m_VolumeSequenceTest[i].clear();
  }
}


////////////////////////////////////////////////////////////////////////////////


bool MDTrigger::AddHit(MDVolumeSequence& VS) 
{
  massert(VS.GetDetector() != 0);

  bool Return = false;

  mdebug<<m_Name<<": trying to add hit...";

  if (m_IsTriggerByDetector == false) {
    MDDetector* Detector = VS.GetDetector();
    if (Detector->IsNamedDetector() == true) Detector = Detector->GetNamedAfterDetector();
    if (m_DetectorTypes.size() != 0) {
      for (unsigned int i = 0; i < m_DetectorTypes.size(); ++i) {
        if (m_DetectorTypes[i] == Detector->GetType() &&
            m_Types[i] == c_Detector) {
          m_TriggerTest[i].push_back(1);
          mdebug<<" done!";
          Return = true;
        }
      }
    } else {
      for (unsigned int i = 0; i < m_Detectors.size(); ++i) {
        if (m_Detectors[i]->GetName() == Detector->GetName() &&
            m_Types[i] == c_Detector) {
          m_TriggerTest[i].push_back(1);
          mdebug<<" done!";
          Return = true;
        }
      }
    }
  } else {
    MDDetector* Detector = VS.GetDetector();
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
              mdebug<<" done!";
              Return = true;
              break;
            }
          }
          if (Exists == false) {
            m_VolumeSequenceTest[i].push_back(VS);
            m_TriggerTest[i].push_back(1);
            mdebug<<" done!";
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
              mdebug<<" done!";
              Return = true;
              break;
            }
          }
          if (Exists == false) {
            m_VolumeSequenceTest[i].push_back(VS);
            m_TriggerTest[i].push_back(1);
            mdebug<<" done!";
            Return = true;
          }
          break;
        }
      }
    }
  }
  mdebug<<endl;

  return Return;
}


////////////////////////////////////////////////////////////////////////////////


bool MDTrigger::AddGuardRingHit(MDVolumeSequence& VS) 
{
  massert(VS.GetDetector() != 0);

  bool Return = false;

  mdebug<<m_Name<<": adding guard ring hit"<<endl;

  if (m_IsTriggerByDetector == false) {
    MDDetector* Detector = VS.GetDetector();
    if (Detector->IsNamedDetector() == true) Detector = Detector->GetNamedAfterDetector();
    if (m_DetectorTypes.size() != 0) {
      for (unsigned int i = 0; i < m_DetectorTypes.size(); ++i) {
        if (m_DetectorTypes[i] == Detector->GetType() &&
            m_Types[i] == c_GuardRing) {
          m_TriggerTest[i].push_back(1);
          mdebug<<" done!";
          Return = true;
          break;
        }
      }
    } else {
      for (unsigned int i = 0; i < m_Detectors.size(); ++i) {
        if (m_Detectors[i]->GetName() == Detector->GetName() &&
            m_Types[i] == c_GuardRing) {
          m_TriggerTest[i].push_back(1);
          mdebug<<" done!";
          Return = true;
          break;
        }
      }
    }
  } else {
    MDDetector* Detector = VS.GetDetector();
    if (Detector->IsNamedDetector() == true) Detector = Detector->GetNamedAfterDetector();
    if (m_DetectorTypes.size() != 0) {
      for (unsigned int i = 0; i < m_DetectorTypes.size(); ++i) {
        if (m_DetectorTypes[i] == Detector->GetType() &&
            m_Types[i] == c_GuardRing) {
          bool Exists = false;
          for (unsigned int v = 0; v < m_VolumeSequenceTest[i].size(); ++v) {
            if (VS.HasSameDetector(m_VolumeSequenceTest[i][v]) == true) {
              m_TriggerTest[i][v]++;
              Exists = true;
              mdebug<<" done!";
              Return = true;
              break;
            }
          }
          if (Exists == false) {
            m_VolumeSequenceTest[i].push_back(VS);
            m_TriggerTest[i].push_back(1);
            mdebug<<" done!";
            Return = true;
          }
          break;
        }
      }
    } else {
      for (unsigned int i = 0; i < m_Detectors.size(); ++i) {
        if (m_Detectors[i]->GetName() == Detector->GetName() &&
            m_Types[i] == c_GuardRing) {
          bool Exists = false;
          for (unsigned int v = 0; v < m_VolumeSequenceTest[i].size(); ++v) {
            if (VS.HasSameDetector(m_VolumeSequenceTest[i][v]) == true) {
              m_TriggerTest[i][v]++;
              Exists = true;
              mdebug<<" done!";
              Return = true;
              break;
            }
          }
          if (Exists == false) {
            m_VolumeSequenceTest[i].push_back(VS);
            m_TriggerTest[i].push_back(1);
            mdebug<<" done!";
            Return = true;
          }
          break;
        }
      }
    }
  }
  mdebug<<endl;

  return Return;
}


////////////////////////////////////////////////////////////////////////////////


bool MDTrigger::HasTriggered()
{
  // If this is a veto trigger, then we cannot have triggered...
  if (m_IsVeto == true) {
    mdebug<<m_Name<<": Trigger test: N/A - this is a veto trigger"<<endl;
    return false;
  }

  for (unsigned int h = 0; h < m_Hits.size(); ++h) {
    if (m_TriggerTest[h].size() < m_Hits[h]) {
      mdebug<<m_Name<<": Trigger test: Not triggered!"<<endl;
      return false;
    } 
  }

  mdebug<<m_Name<<": Trigger test: Triggered!"<<endl;
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MDTrigger::HasVetoed()
{
  // If this is not a veto trigger, then we cannot have vetoed...
  if (m_IsVeto == false) {
    mdebug<<m_Name<<": Veto test: N/A - this is a NOT veto trigger"<<endl;
    return false;
  }

  for (unsigned int h = 0; h < m_Hits.size(); ++h) {
    if (m_TriggerTest[h].size() < m_Hits[h]) {
      mdebug<<m_Name<<": Veto test: No Veto"<<endl;
      return false;
    } 
  }

  mdebug<<m_Name<<": Veto test: Veto!"<<endl;
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MDTrigger::Applies(MDDetector* Detector) const
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


bool MDTrigger::IncludesDetectorAsPositiveTrigger(MDDetector* Detector)
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


MString MDTrigger::GetGeant3(int i)
{
  // Return the geant3 code of this trigger condition
  // i is the index of this trigger, count starts with zero

  ostringstream out;

  if (m_DetectorTypes.size() != 0) {
    out<<"      TNDETS("<<i<<") = "<<m_Hits.size()<<endl;
    for (unsigned int j = 0; j < m_Hits.size(); j++) {
      out<<"      TDTYP("<<i<<", "<<j+1<<") = "<<m_DetectorTypes[j]<<endl;
      out<<"      TDHITS("<<i<<", "<<j+1<<") = "<<m_Hits[j]<<endl;
    }
  } else {
    mimp<<"Missing Geant3 trigger condition for \"Mytrigger.Detector\" in Geant3 file"<<endl;
    mimp<<"This is only relevant if you do plain Geant3 (not mgeant/mggpod) simulations"<<endl;
    mimp<<"Only in this case use \"Mytrigger.Detector\" instead..."<<endl;
    out<<"C     Trigger "<<m_Name<<" not GMega compatible"<<endl;
    out<<"      TNTRIG = TNTRIG-1"<<endl;
  }

  return out.str().c_str(); 
}


////////////////////////////////////////////////////////////////////////////////


MString MDTrigger::GetMGeant(int i)
{
  // Return the MGeant/MGGPOD code of this trigger condition
  // i is the index of this trigger, count starts with zero

  ostringstream out;

  // Read: name of this trigger
  MString TriggerName = m_Name;
  TriggerName.ToUpper();

  out<<"TNAM "<<i<<" "<<TriggerName<<endl;
  // Trigger i is veto
  out<<"TIVT "<<i<<" "<<((m_IsVeto == true) ? "0" : "1")<<endl;
 
  bool IsGuardRing = false;
  for (unsigned int j = 0; j < m_Types.size(); j++) {
    if (m_Types[j] == c_GuardRing) {
      IsGuardRing = true;
      break;
    }
  }
 
  // GuardRing trigger (not veto is not allowed!), so there should be no problems
  if (IsGuardRing == true) {
    out<<"TNDE "<<i<<" "<<0<<endl;
  } else {
    // Read: number of detectors/detector types in trigger i
    if (m_DetectorTypes.size() != 0) {
      out<<"TNDT "<<i<<" "<<m_Hits.size()<<endl;
    } else {
      out<<"TNDE "<<i<<" "<<m_Hits.size()<<endl;
    }
    // For each of these detectors
    for (unsigned int j = 0; j < m_Hits.size(); j++) {
      if (m_DetectorTypes.size() != 0) {
        // Read: Number of sensitive volumes of this detector j in trigger i:
        out<<"TDTY "<<i<<" "<<j+1<<" "<<m_DetectorTypes[j]<<endl;
      } else {
        // Read: Number of sensitive volumes of this detector j in trigger i:
        out<<"TDNV "<<i<<" "<<j+1<<" "<<m_Detectors[j]->GetNSensitiveVolumes()<<endl;
        for (unsigned v = 0; v < m_Detectors[j]->GetNSensitiveVolumes(); ++v) {
          // Read: (v+1) sensitive volume with this name of detector (j+1) in trigger i
          MString Name = m_Detectors[j]->GetSensitiveVolume(v)->GetShortName();
          Name.ToUpper();
          out<<"TDSV "<<i<<" "<<j+1<<" "<<v+1<<" "<<Name<<endl;
        }
      }
      // Read number of hits in any or all together sensitive volumes...
      out<<"TDHI "<<i<<" "<<j+1<<" "<<m_Hits[j]<<endl;
      // Energy threshold in this detector...
      if (m_IsVeto == false) {
        out<<"TDET "<<i<<" "<<j+1<<" "<<0.0<<endl;
      } else {
        if (m_DetectorTypes.size() != 0) {
          out<<"TDET "<<i<<" "<<j+1<<" "<<c_LargestEnergy<<endl;        
        } else {
          out<<"TDET "<<i<<" "<<j+1<<" "<<m_Detectors[j]->GetSecureUpperLimitTriggerThreshold()<<endl;        
        }
      }
    }
  }

  return out.str().c_str(); 
}


////////////////////////////////////////////////////////////////////////////////


MString MDTrigger::GetGeomega()
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
  
  return out.str().c_str(); 
}

////////////////////////////////////////////////////////////////////////////////


MString MDTrigger::ToString()
{
  // Return the Geant4 code of this trigger condition

  ostringstream out;

  out<<(*this);

  return out.str().c_str();
}


////////////////////////////////////////////////////////////////////////////////


bool MDTrigger::Validate()
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


ostream& operator<<(ostream& os, const MDTrigger& R)
{
  os<<"Trigger "<<R.m_Name<<endl;
  
  for (unsigned int i = 0; i < R.m_DetectorTypes.size(); ++i) {
    os<<"  Detector types:"<<R.m_DetectorTypes[i]<<" "<<R.m_Hits[i];
    if (R.m_Types[i] == R.c_GuardRing) {
      os<<" (guard ring!)";
    }
    os<<endl;
  }
  for (unsigned int i = 0; i < R.m_Detectors.size(); ++i) {
    os<<"  Detector names:"<<R.m_Detectors[i]->GetName()<<" "<<R.m_Hits[i]<<endl;
    if (R.m_Types[i] == R.c_GuardRing) {
      os<<" (guard ring!)";
    }
    os<<endl;
  }

  return os;
}


// MDTrigger.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
