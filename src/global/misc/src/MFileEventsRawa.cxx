/*
 * MFileEventsRawa.cxx
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
// MFileEventsRawa
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MFileEventsRawa.h"

// Standard libs:
#include <iostream>
using std::cout;
using std::endl;

// ROOT libs:

// MEGAlib libs:
#include "MTokenizer.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MFileEventsRawa)
#endif


////////////////////////////////////////////////////////////////////////////////


MFileEventsRawa::MFileEventsRawa() : MFileEvents()
{
  // Construct an instance of MFileEventsRawa

  m_EventId = -1;;
}


////////////////////////////////////////////////////////////////////////////////


MFileEventsRawa::~MFileEventsRawa()
{
  // Delete this instance of MFileEventsRawa
}


////////////////////////////////////////////////////////////////////////////////


bool MFileEventsRawa::Open(MString FileName, unsigned int Way)
{
  //

  m_IncludeFileUsed = false;
  m_IncludeFile = new MFileEventsRawa();
  m_IncludeFile->SetIsIncludeFile(true);

  return MFile::Open(FileName, c_Read);
}


////////////////////////////////////////////////////////////////////////////////


bool MFileEventsRawa::LoadNextEvent()
{
  // Load the next event. Return false if it is the last one

  cout<<"I was unable to test this file after upgrading MFile - we wmight loose the first event..."<<endl;

  // Some temporary variables:
  int Int1;
  //double Double1;

  if (UpdateProgress() == false) return false;

  // Parse the dammed file:
  MString Line;
  while(!m_File.eof()) {
    Line.ReadLine(m_File);

    if (Line[0] == 'S' && Line[1] == 'E') {
      if (sscanf(Line.Data(), "SE%i", &Int1) == 1) {
        m_EventId = Int1;
        m_NEvents++;

        // Reset Everything:
        m_TimeWalk = 0;
        m_PulseFormAdcs.resize(0);
        m_PulseForms.resize(0);
        m_TriggerMask.resize(0);
      }
    } else if (Line[0] == 'T' && Line[1] == 'W') {
      if (sscanf(Line.Data(), "TW%i", &Int1) == 1) {
        m_TimeWalk = Int1;
      }
    } else if (Line[0] == 'T' && Line[1] == 'D') {
      if (sscanf(Line.Data(), "TD%i", &Int1) == 1) {
        m_TimeTillD2Trigger = Int1;
      }
    } else if (Line[0] == 'T' && Line[1] == 'M') {
      MString TM(Line);
      TM.ReplaceAll("TM ", "");
      TM.ReplaceAll(";", "");
      while (TM.Length() >= 4) {
        m_TriggerMask.push_back(TM.GetSubString(0,4));
        TM.Remove(0, 4);
      }
    } else if (Line[0] == 'P' && Line[1] == 'H') {
      MTokenizer T(Line);
      m_PulseFormAdcs.push_back(T.GetTokenAtAsString(1));
      m_PulseForms.resize(m_PulseFormAdcs.size());
      for (int t = 4; t < T.GetNTokens(); ++t) {
        m_PulseForms.back().push_back(T.GetTokenAtAsInt(t));
      }
    }

    // If we have already the next event, we return:
    if (Line[0] == 'S' && Line[1] == 'E') {
      return true;
    }
  } // while (true)

  return true;
}


////////////////////////////////////////////////////////////////////////////////


vector<int> MFileEventsRawa::GetPulseFormFor(MString Name) 
{ 
  for (unsigned n = 0; n < m_PulseFormAdcs.size(); ++n) {
    if (m_PulseFormAdcs[n] == Name) {
      return m_PulseForms[n];
    }
  }

  vector<int> a;
  return a; 
}


////////////////////////////////////////////////////////////////////////////////


bool MFileEventsRawa::HasTriggered(MString Detector)
{
  for (unsigned int m = 0; m < m_TriggerMask.size(); ++m) {
    if (m_TriggerMask[m] == Detector) {
      return true;
    }
  }

  return false;
}


////////////////////////////////////////////////////////////////////////////////


bool MFileEventsRawa::OpenIncludeFile(MString Line)
{
  // Open an include file

  return false;
}


// MFileEventsRawa.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
