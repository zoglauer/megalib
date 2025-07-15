/*
 * MFileEventsType.cxx
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
// MFileEventsType
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MFileEventsType.h"

// Standard libs:

// ROOT libs:

// MEGAlib:
#include "MAssert.h"
#include "MFileEvents.h"
#include "MPhysicalEvent.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MFileEventsType)
#endif


////////////////////////////////////////////////////////////////////////////////

const long MFileEventsType::c_NoId = -1;

////////////////////////////////////////////////////////////////////////////////


MFileEventsType::MFileEventsType(MString GeometryFileName) : MFileEvents()
{
  // Construct an instance of MFileEventsType
  m_Version = 0;
  m_GeometryFileName = GeometryFileName;
  Reset();
}


////////////////////////////////////////////////////////////////////////////////


MFileEventsType::~MFileEventsType() {}


////////////////////////////////////////////////////////////////////////////////

bool MFileEventsType::Open(MString FileName, unsigned int Way, bool IsBinary)
{
  // Open the file, check if the type is correct

  m_IncludeFileUsed = false;
  m_IncludeFile = new MFileEventsType();
  m_IncludeFile->SetIsIncludeFile(true);
  
  if (FileName.EndsWith("etp") == false) {
    merr<<"This file is not an event type (.etp) file: "<<FileName<<endl;
    return false;    
  } else {
    m_FileType = MString("etp");
  }
  
  if (MFileEvents::Open(FileName, Way, IsBinary) == false) {
    return false;
  }
  
  Reset();
  return true;
}

////////////////////////////////////////////////////////////////////////////////
void MFileEventsType::Reset()
{
  m_EventId = c_NoId;
  m_EventType = c_UnknownEvent;
  m_EventTypeProbability = 0.;

  m_IsFirstEvent = true;
}

////////////////////////////////////////////////////////////////////////////////


bool MFileEventsType::Close()
{
  // Close the file
  if (m_Way == c_Write || m_Way == c_Create) Write(MString("\n\nEN\n\n"));

  return MFileEvents::Close();
}

long MFileEventsType::GetEventId()
{
  return m_EventId;
}

int MFileEventsType::GetEventType()
{
  return m_EventType;
}

double MFileEventsType::GetEventTypeProbability()
{
  return m_EventTypeProbability;
}

////////////////////////////////////////////////////////////////////////////////

bool MFileEventsType::GetNextEvent()
{
  // Recursive call if included file is in use
  if (m_IncludeFileUsed) {
    if (! dynamic_cast<MFileEventsType*>(m_IncludeFile)->GetNextEvent()) {
      if (m_IncludeFile->IsCanceled()) m_Canceled = true;
      m_IncludeFile->Close();
      m_IncludeFileUsed = false;
      if (m_Canceled) return false;
    } else {
      return true;
    }
  }
  
  MString Line;
  // Reset data fields
  unsigned int status = 0; //3 LSB for id, et, tp
  m_EventId = c_NoId;
  m_EventType = c_UnknownEvent;
  m_EventTypeProbability = 0.;

  // Resume reading file
  while (IsGood() && ReadLine(Line)) {
    if (Line.Length() < 2) continue;

    // Include another file:
    if (Line[0] == 'I' && Line[1] == 'N') {
      if (OpenIncludeFile(Line)) {
        return GetNextEvent();//now that m_IncludeFileUsed is true
      } else {
        m_IncludeFile->Close();
        m_IncludeFileUsed = false;
      }

    // New event
    } else if (Line[0] == 'S' && Line[1] == 'E') {
      if (status) return true; // Even if incomplete event - allows to skip TP
    } else if (Line[0] == 'I' && Line[1] == 'D') {
      sscanf(Line.Data(), "ID %lu", &m_EventId);
      status |= 0b001;
    } else if (Line[0] == 'E' && Line[1] == 'T' && Line.Length() > 4) {
      m_EventType = MPhysicalEvent::ParseET(&Line[3]);
      status |= 0b010;
    } else if (Line[0] == 'T' && Line[1] == 'P') {
      sscanf(Line.Data(), "TP %lf", &m_EventTypeProbability);
      status |= 0b100;
    // Next file
    } else if (Line[0] == 'N' && Line[1] == 'F') {
      if (! OpenNextFile(Line) ) {
        mout<<"Did not find a valid continuation file..."<<endl;
        return false;
      }
    // End of file
    } else if (Line[0] == 'E' && Line[1] == 'N') {
      ReadFooter(true);
      if (status) return true; // Even if incomplete event - allows to skip TP
    }
    if (status == 0b111) return true;
  }

  return false;
}


////////////////////////////////////////////////////////////////////////////////

bool MFileEventsType::AddEvent(long EventId, int EventType, double EventTypeProbability)
{
  if (m_Way == c_Read)
  {
    merr << "Only valid if file is in write-mode!" << endl;
    massert(false);
    return false;
  }
  if (!m_IsOpen) return false;

  if (m_IsFirstEvent) {
    WriteHeader();
    m_IsFirstEvent = false;
  }

  ostringstream S;
  S << "SE" << endl;
  S << "ID " << EventId << endl;
  S << "ET " << MPhysicalEvent::GetTypeStringCode(EventType) << endl;
  S << "TP " << EventTypeProbability << endl;

  Write(S);

  return true;
}

