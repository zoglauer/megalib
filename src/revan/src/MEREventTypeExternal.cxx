/*
 * MEREventTypeExternal.cxx
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
#include "MEREventTypeExternal.h"


// MEGAlib libs:
#include "MAssert.h"
#include "MFileEventsType.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MEREventTypeExternal)
#endif




////////////////////////////////////////////////////////////////////////////////


MEREventTypeExternal::MEREventTypeExternal() : MEREventType()
{
  m_EventTypeFileName = "";
  m_FileEventsType = new MFileEventsType();
}

MEREventTypeExternal::~MEREventTypeExternal()
{
  delete m_FileEventsType;
}


////////////////////////////////////////////////////////////////////////////////

void MEREventTypeExternal::SetParameters(MString EventTypeFileName)
{
  m_EventTypeFileName = EventTypeFileName;
}

bool MEREventTypeExternal::PostAnalysis()
{
  return m_FileEventsType->Close();
}


////////////////////////////////////////////////////////////////////////////////

bool MEREventTypeExternal::Analyze(MRawEventIncarnations* List)
{
  massert(m_FileEventsType);
  MERConstruction::Analyze(List);

  if (! m_FileEventsType->IsOpen() ) {//First event
    m_FileEventsType->Open(m_EventTypeFileName); //Read-mode
  }

  // Read file
  MRERawEvent* RE = nullptr;  
  for (int e = 0; e < m_List->GetNRawEvents(); e++) {
    RE = m_List->GetRawEventAt(e);
    //mout << "Looking for event id=" << RE->GetEventId() << endl;
    while(m_FileEventsType->GetNextEvent() && m_FileEventsType->GetEventId() != RE->GetEventId()) {
      //mout << " - Got event id=" <<  m_FileEventsType->GetEventId() << " at pos=" << m_FileEventsType->GetFilePosition() << endl;
    }
    //mout << "evtid=" << m_FileEventsType->GetEventId() << endl;
    if (m_FileEventsType->GetEventId() == -1) {//If reached end of file
       m_FileEventsType->Rewind(false);
    }
    if (m_FileEventsType->GetEventId() == RE->GetEventId()) { // if found matching ID
      RE->SetEventType( m_FileEventsType->GetEventType() );
      RE->SetEventTypeProbability( m_FileEventsType->GetEventTypeProbability() );
    } else {
      mout << "MEREventTypeExternal: No event type found for event ID " << RE->GetEventId() << endl;
      RE->SetEventType( c_UnknownEvent );
      RE->SetEventTypeProbability( 0. );
      RE->SetEventReconstructed();
      RE->SetRejectionReason(MRERawEvent::c_RejectionNoExternalEventType);// TBC of course!!!!!
    }
    //m_FileEventsType->Rewind(false);
    //m_FileEventsType->Seek(filePos);// Be kind, Rewind to position at start of REI
  }
  return true;
}


////////////////////////////////////////////////////////////////////////////////

MString MEREventTypeExternal::ToString(bool CoreOnly) const
{
  // Dump an options string gor the tra file:
  ostringstream out;

  if (CoreOnly == false) {
    out<<"# Event type - options:"<<endl;
    out<<"# "<<endl;
  }
  out<<"# Event Type file name:         "<<m_EventTypeFileName<<endl;
  if (CoreOnly == false) {
    out<<"# "<<endl;
  }

  return out.str().c_str();
}

