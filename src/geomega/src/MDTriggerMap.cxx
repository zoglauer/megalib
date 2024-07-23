/*
 * MDTriggerMap.cxx
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
// MDTriggerMap
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MDTriggerMap.h"

// Standard libs:
#include <sstream>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MAssert.h"
#include "MStreams.h"
#include "MDDetector.h"
#include "MDGuardRing.h"
#include "MParser.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MDTriggerMap)
#endif


////////////////////////////////////////////////////////////////////////////////


MDTriggerMap::MDTriggerMap(const MString& Name) : MDTrigger(Name)
{
  // Construct an instance of MDTriggerMap

  m_TriggerType = MDTriggerType::c_Universal;
}


////////////////////////////////////////////////////////////////////////////////


MDTriggerMap::~MDTriggerMap()
{
  // Delete this instance of MDTriggerMap
}


////////////////////////////////////////////////////////////////////////////////


void MDTriggerMap::Reset()
{
  for (auto& I: m_TriggerTest) I = 0;
}


////////////////////////////////////////////////////////////////////////////////


//! Read the triggers for a trigger map
bool MDTriggerMap::ReadTriggerMap(MString FileName)
{
  MParser P;
  if (P.Open(FileName) == false) {
    mout<<"   *** ERROR **** in trigger "<<m_Name<<endl;
    mout<<"Unable to open file "<<FileName<<endl;
    return false;   
  }
  
  for (unsigned int p = 0; p < P.GetNLines(); ++p) {
    MTokenizer* T = P.GetTokenizerAt(p);
    if (T->IsTokenAt(0, "DT") == true) {
      SetDetectors(T->GetTokenAtAsStringVector(1));
      break;
    }
  }
  
  for (unsigned int p = 0; p < P.GetNLines(); ++p) {
    MTokenizer* T = P.GetTokenizerAt(p);
    if (T->IsTokenAt(0, "TV") || T->IsTokenAt(0, "TN") || T->IsTokenAt(0, "VT")) {
      vector<MString> HitStrings = T->GetTokenAtAsStringVector(1);
      if (HitStrings.size() != m_Detectors.size()) {
        mout<<"   *** ERROR **** in trigger "<<m_Name<<endl;
        mout<<"Trigger does not contain the expected amount of hit conditions: "<<HitStrings.size()<<" vs. "<<m_Detectors.size()<<" expected"<<endl;       
        return false;
      }
      
      MDTriggerMode Mode = MDTriggerMode::c_VetoableTrigger;
      if (T->IsTokenAt(0, "TV") == true) { 
        Mode = MDTriggerMode::c_VetoableTrigger;
      } else if (T->IsTokenAt(0, "TN") == true) {
        Mode = MDTriggerMode::c_NonVetoableTrigger;
      } else if (T->IsTokenAt(0, "VT") == true) {
        Mode = MDTriggerMode::c_Veto;
      }
      
      vector<unsigned int> Hits(HitStrings.size(), 0);
      vector<bool> MoreHitsAllowed(HitStrings.size(), false);
      for (unsigned int h = 0; h < HitStrings.size(); ++h) {
        if (HitStrings[h].EndsWith("+") == true) {
          MoreHitsAllowed[h] = true;
          HitStrings[h].RemoveAllInPlace("+");
        }
        Hits[h] = HitStrings[h].ToUnsignedInt();
      }
      
      SetTrigger(Mode, Hits, MoreHitsAllowed);
    }
  }
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Read the triggers for a trigger map
bool MDTriggerMap::WriteTriggerMap(MString FileName)
{
  ofstream out;
  out.open(FileName);
  if (out.is_open() == false) {
    mout<<"   *** ERROR **** in trigger "<<m_Name<<endl;
    mout<<"Unable to open file "<<FileName<<endl;
    return false;
  }
  
  out<<"# Trigger map"<<endl;
  out<<endl;
  out<<"# Detectors"<<endl;
  out<<"DT ";
  for (MString Name: m_Detectors) { out<<Name<<" "; }
  out<<endl;
  out<<endl;
  out<<"#Triggers: "<<endl;
  
  for (unsigned int t = 0; t < m_TriggerModes.size(); ++t) {
    if (m_TriggerModes[t] == MDTriggerMode::c_VetoableTrigger) {
      out<<"TV ";
    } else if (m_TriggerModes[t] == MDTriggerMode::c_NonVetoableTrigger) {
      out<<"TN ";
    } else if (m_TriggerModes[t] == MDTriggerMode::c_Veto) {
      out<<"VT ";
    } else {
      mout<<"   *** ERROR **** in trigger "<<m_Name<<endl;
      mout<<"Unknown trigger/veto mode"<<endl;
      return false;      
    }
      
    for (unsigned int h = 0; h < m_Hits[t].size(); ++h) {
      out<<m_Hits[t][h];
      if (m_AllowMoreHits[t][h] == true) {
        out<<"+";
      }
      out<<" ";
    }
    out<<endl;
  } 
  out<<endl;
  
  out.close();
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Set the detector list
bool MDTriggerMap::SetDetectors(vector<MString> Detectors)
{
  m_Detectors = Detectors;
  m_TriggerTest.resize(m_Detectors.size());
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Set a trigger, veto, etc.
bool MDTriggerMap::SetTrigger(MDTriggerMode Mode, vector<unsigned int> Hits, vector<bool> AllowMoreHits)
{
  if (Hits.size() != m_Detectors.size()) {
    mout<<"   *** ERROR **** in trigger "<<m_Name<<endl;
    mout<<"Hits vector has not the correct size (expected: "<<m_Detectors.size()<<", received: "<<m_Hits.size()<<")"<<endl;
    return false;
  }
  if (AllowMoreHits.size() != m_Detectors.size()) {
    mout<<"   *** ERROR **** in trigger "<<m_Name<<endl;
    mout<<"AllowMoreHits vector has not the correct size (expected: "<<m_Detectors.size()<<", received: "<<m_AllowMoreHits.size()<<")"<<endl;
    return false;
  }
  
  m_TriggerModes.push_back(Mode);
  m_Hits.push_back(Hits);
  m_AllowMoreHits.push_back(AllowMoreHits);
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Set a trigger, veto, etc.
bool MDTriggerMap::SetTrigger(MDTriggerMode Mode, vector<MString> Detectors, vector<unsigned int> Hits, vector<bool> AllowMoreHits)
{
  vector<unsigned int> LocalHits(m_Detectors.size(), 0);
  vector<bool> LocalAllowMoreHits(m_Detectors.size(), true);
  
  for (unsigned int l = 0; l < Detectors.size(); ++l) {
    for (unsigned int d = 0; d < m_Detectors.size(); ++d) { 
      if (Detectors[l].AreIdentical(m_Detectors[d]) == true) {
        LocalHits[d] = Hits[l];
        LocalAllowMoreHits[d] = AllowMoreHits[l];
        break;
      }
    }
  }
  
  m_TriggerModes.push_back(Mode);
  m_Hits.push_back(LocalHits);
  m_AllowMoreHits.push_back(LocalAllowMoreHits);
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Return true if this trigger is triggering (not vetoing)
bool MDTriggerMap::IsTriggering(MDDetector* D) const 
{ 
  MString Name = D->GetName();
  
  unsigned int Pos = numeric_limits<unsigned int>::max();
  for (unsigned int d = 0; d < m_Detectors.size(); ++d) {
    if (Name.AreIdentical(m_Detectors[d]) == true) {
      Pos = d;
      break;
    }
  }
  if (Pos < m_Detectors.size()) {
    unsigned int NTriggers = 0;
    for (unsigned int t = 0; t < m_TriggerModes.size(); ++t) {
      if (m_TriggerModes[t] == MDTriggerMode::c_VetoableTrigger ||  m_TriggerModes[t] == MDTriggerMode::c_NonVetoableTrigger) {
        NTriggers += m_Hits[t][Pos];
      }
    }
    if (NTriggers > 0) {
      return true;
    }
  }
  
  return false; 
}


////////////////////////////////////////////////////////////////////////////////


//! Return true if this trigger is vetoing
bool MDTriggerMap::IsVetoing(MDDetector* D) const 
{ 
  MString Name = D->GetName();
  
  unsigned int Pos = numeric_limits<unsigned int>::max();
  for (unsigned int d = 0; d < m_Detectors.size(); ++d) {
    if (Name.AreIdentical(m_Detectors[d]) == true) {
      Pos = d;
      break;
    }
  }
  if (Pos < m_Detectors.size()) {
    unsigned int NVetoes = 0;
    for (unsigned int t = 0; t < m_TriggerModes.size(); ++t) {
      if (m_TriggerModes[t] == MDTriggerMode::c_Veto) {
        NVetoes += m_Hits[t][Pos];
      }
    }
    if (NVetoes > 0) {
      return true;
    }
  }
  
  return false; 
}  


////////////////////////////////////////////////////////////////////////////////


bool MDTriggerMap::AddHit(const MDVolumeSequence& VS) 
{
  bool Return = false;

  mdebug<<m_Name<<": trying to add hit...";

  const MDDetector* Detector = VS.GetDetector();
  if (Detector == nullptr) {
    mout<<"   *** ERROR **** in "<<m_Name<<endl;
    mout<<"Volume sequence has no detector!"<<endl;
    return false;
  }
  MString Name = Detector->GetName();
  for (unsigned int d = 0; d < m_Detectors.size(); ++d) {
    //cout<<"Testing: "<<Name<<":"<<m_Detectors[d]<<endl;
    if (Name.AreIdentical(m_Detectors[d])) {
      ++m_TriggerTest[d];
      mdebug<<" done -> "<<m_Detectors[d];
      Return = true;
      break;
    }
  }
  if (Return == false) {
    if (Detector->IsNamedDetector() == true) Detector = Detector->GetNamedAfterDetector();
    for (unsigned int d = 0; d < m_Detectors.size(); ++d) {
      //cout<<"Testing: "<<Name<<":"<<m_Detectors[d]<<endl;
      if (Name.AreIdentical(m_Detectors[d])) {
        ++m_TriggerTest[d];
        mdebug<<" done -> "<<m_Detectors[d];
        Return = true;
        break;
      }
    }
  }
  mdebug<<endl;

  return Return;
}


////////////////////////////////////////////////////////////////////////////////


//! Returns true if the hit could be added 
bool MDTriggerMap::AddGuardRingHit(const MDVolumeSequence& VS) 
{ 
  bool Return = false;
  
  mdebug<<m_Name<<": trying to add guard ring hit...";
  
  const MDDetector* Detector = VS.GetDetector();
  if (Detector == nullptr) {
    mout<<"   *** ERROR **** in "<<m_Name<<endl;
    mout<<"Volume sequence has no detector!"<<endl;
    return false;
  }
  if (Detector->HasGuardRing() == false) {
    mout<<"   *** ERROR **** in "<<m_Name<<endl;
    mout<<"Detector "<<Detector->GetName()<<" has not guard ring, but presumablby a guard ring hit"<<endl;
    return false;
  }  
  const MDGuardRing* GuardRingDetector = Detector->GetGuardRing();
  if (GuardRingDetector == nullptr) {
    mout<<"   *** ERROR **** in "<<m_Name<<endl;
    mout<<"Detector "<<Detector->GetName()<<" has not guard ring, but presumablby a guard ring hit"<<endl;
    return false;
  }
  
  MString Name = GuardRingDetector->GetName();
  for (unsigned int d = 0; d < m_Detectors.size(); ++d) {
    mdebug<<"Testing: "<<Name<<":"<<m_Detectors[d]<<endl;
    if (Name.AreIdentical(m_Detectors[d])) {
      ++m_TriggerTest[d];
      mdebug<<" done -> "<<m_Detectors[d];
      Return = true;
      break;
    }
  }
  if (Return == false) {
    if (GuardRingDetector->IsNamedDetector() == true) GuardRingDetector = dynamic_cast<MDGuardRing*>(GuardRingDetector->GetNamedAfterDetector());
    for (unsigned int d = 0; d < m_Detectors.size(); ++d) {
      mdebug<<"Testing: "<<Name<<":"<<m_Detectors[d]<<endl;
      if (Name.AreIdentical(m_Detectors[d])) {
        ++m_TriggerTest[d];
        mdebug<<" done -> "<<m_Detectors[d];
        Return = true;
        break;
      }
    }
  }
  mdebug<<endl;
  
  return Return;
}


////////////////////////////////////////////////////////////////////////////////


bool MDTriggerMap::HasTriggered()
{
  // We have triggered when
  // (a) a (non) vetoable trigger has been raised
  // (b) we have not vetoes (and we don't ignore vetoes)
  
  // (a)
  if (HasNonVetoablyTriggered() == true) return true;
        
  
  // (b)
  bool Triggered = false;
  for (unsigned int t = 0; t < m_TriggerModes.size(); ++t) {
    if (m_TriggerModes[t] == MDTriggerMode::c_VetoableTrigger) {
      Triggered = true;
      for (unsigned int h = 0; h < m_Hits[t].size(); ++h) {
        if (m_AllowMoreHits[t][h] == true) {
          if (m_TriggerTest[h] < m_Hits[t][h]) {
            Triggered = false;
            break;
          }
        } else {
          if (m_TriggerTest[h] < m_Hits[t][h] || m_TriggerTest[h] > m_Hits[t][h]) {
            Triggered = false;
            break;
          }         
        }
      }
      if (Triggered == true) {     
        break;
      }
    }
  }
  
  // (b) 2)
  bool Vetoed = false;
  if (m_IgnoreVetoes == false) { 
    for (unsigned int t = 0; t < m_TriggerModes.size(); ++t) {
      if (m_TriggerModes[t] == MDTriggerMode::c_Veto) {
        Vetoed = true;
        for (unsigned int h = 0; h < m_Hits[t].size(); ++h) {
          if (m_AllowMoreHits[t][h] == true) {
            if (m_TriggerTest[h] < m_Hits[t][h]) {
              Vetoed = false;
              break;
            }
          } else {
            if (m_TriggerTest[h] < m_Hits[t][h] || m_TriggerTest[h] > m_Hits[t][h]) {
              Vetoed = false;
              break;
            }         
          }
        }
        if (Vetoed == true) {     
          break;
        }
      }
    }
  }
  
  if (Vetoed == true) {
    mdebug<<m_Name<<": Trigger test: Not triggered!"<<endl;
    return false;
  }
  
  if (Triggered == true) {
    mdebug<<m_Name<<": Trigger test: Triggered!"<<endl;
    return true;
  }
  
  return Triggered;
}


////////////////////////////////////////////////////////////////////////////////


bool MDTriggerMap::HasNonVetoablyTriggered()
{
  // We have a non-vetoably triggered when
  // (a) a non vetoable trigger has been raised
  
  bool Triggered = false;
  for (unsigned int t = 0; t < m_TriggerModes.size(); ++t) {
    if (m_TriggerModes[t] == MDTriggerMode::c_NonVetoableTrigger) {
      Triggered = true;
      for (unsigned int h = 0; h < m_Hits[t].size(); ++h) {
        if (m_AllowMoreHits[t][h] == true) {
          if (m_TriggerTest[h] < m_Hits[t][h]) {
            Triggered = false;
            break;
          }
        } else {
          if (m_TriggerTest[h] < m_Hits[t][h] || m_TriggerTest[h] > m_Hits[t][h]) {
            Triggered = false;
            break;
          }         
        }
      }
      if (Triggered == true) {
        mdebug<<m_Name<<": Trigger test: Non-vetoably triggered!"<<endl;       
        break;
      }
    }
  }
  
  return Triggered;
}


////////////////////////////////////////////////////////////////////////////////


bool MDTriggerMap::HasVetoed()
{
  // We have not vetoed when
  // (a) we ignore vetoes
  // (b) a non-vetoable trigger was raised
  // We have vetoes when
  // (b) a veto trigger was raised
  
  // (a)
  if (m_IgnoreVetoes == true) {
    return false; 
  }
  
  // (b)
  if (HasNonVetoablyTriggered() == true) {
    return false;  
  }
  
  // (c)
  bool Vetoed = false;
  for (unsigned int t = 0; t < m_TriggerModes.size(); ++t) {
    if (m_TriggerModes[t] == MDTriggerMode::c_Veto) {
      Vetoed = true;
      for (unsigned int h = 0; h < m_Hits[t].size(); ++h) {
        if (m_AllowMoreHits[t][h] == true) {
          if (m_TriggerTest[h] < m_Hits[t][h]) {
            Vetoed = false;
            break;
          }
        } else {
          if (m_TriggerTest[h] < m_Hits[t][h] || m_TriggerTest[h] > m_Hits[t][h]) {
            Vetoed = false;
            break;
          }         
        }
      }
      if (Vetoed == true) {     
        mdebug<<m_Name<<": Trigger test: Vetoed!"<<endl;
        break;
      }
    }
  }
  
  return Vetoed;
}


////////////////////////////////////////////////////////////////////////////////


bool MDTriggerMap::Applies(MDDetector* Detector) const
{
  //! Return true if this trigger applies to the detector
  
  MString Name = Detector->GetName();
  for (unsigned int d = 0; d < m_Detectors.size(); ++d) {
    if (Name.AreIdentical(m_Detectors[d]) == true) {
      return true;
    }
  }
  
  return false;
}


////////////////////////////////////////////////////////////////////////////////


MString MDTriggerMap::GetGeomega() const
{
  // Return as Geomega type volume tree
  
  return "Doesn't work anyway..."; 
}

////////////////////////////////////////////////////////////////////////////////


MString MDTriggerMap::ToString() const
{
  // Return the Geant4 code of this trigger condition

  ostringstream out;

  out<<"Trigger - universal";

  return out.str();
}


////////////////////////////////////////////////////////////////////////////////


bool MDTriggerMap::Validate()
{
  // Make sure everything is reasonable:

  return true;
}


////////////////////////////////////////////////////////////////////////////////


ostream& operator<<(ostream& os, const MDTriggerMap& R)
{
  return os;
}


// MDTriggerMap.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
