/*
 * MEREventTypeTorch.cxx
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
#include "MEREventTypeTorch.h"


// MEGAlib libs:
#include "MAssert.h"
#include "MFileEventsType.h"

// Torch libs:
//#pragma push_macro("ClassDef")
//#undef ClassDef
#include <torch/torch.h>
#include <torch/script.h>
//#pragma pop_macro("ClassDef")

// C++ std libs:
#include <iostream>
#include <memory>
#include <vector>


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MEREventTypeTorch)
#endif




////////////////////////////////////////////////////////////////////////////////


MEREventTypeTorch::MEREventTypeTorch() : MEREventType()
{
  m_EventTypeFileName = "";
  m_FileEventsType = new MFileEventsType();
}

MEREventTypeTorch::~MEREventTypeTorch()
{
  delete m_FileEventsType;
}


////////////////////////////////////////////////////////////////////////////////

void MEREventTypeTorch::SetParameters(MString EventTypeFileName)
{
  m_EventTypeFileName = EventTypeFileName;
  try {
    m_module = torch::jit::load(m_EventTypeFileName);
    m_module.eval();
    mout << "Module " << m_EventTypeFileName << " loaded successfully." << endl;
  } catch (const c10::Error& e) {
    merr << "Error loading model: " << e.msg() << endl;
  }
}

bool MEREventTypeTorch::PostAnalysis()
{
  return m_FileEventsType->Close();
}


////////////////////////////////////////////////////////////////////////////////

bool MEREventTypeTorch::Analyze(MRawEventIncarnations* List)
{
  MERConstruction::Analyze(List);

  if (! m_FileEventsType->IsOpen() ) {//First event
    m_FileEventsType->Open(m_EventTypeFileName); //Read-mode
  }

  // Read file
  streampos filePos = m_FileEventsType->GetFilePosition(); // Position at start of REI
  MRERawEvent* RE = nullptr;  
  for (int e = 0; e < m_List->GetNRawEvents(); e++) {
    m_FileEventsType->Seek(filePos);// Rewind to position at start of REI
    RE = m_List->GetRawEventAt(e);
    while(m_FileEventsType->GetNextEvent() && m_FileEventsType->GetEventId() != RE->GetEventId()) {}
    mout << "evtid=" << m_FileEventsType->GetEventId() << endl;
    if (m_FileEventsType->GetEventId() == RE->GetEventId()) { // if found matching ID
      RE->SetEventType( m_FileEventsType->GetEventType() );
      RE->SetEventTypeProbability( m_FileEventsType->GetEventTypeProbability() );
    } else {
      merr << "MEREventTypeTorch: No event type found for event ID " << RE->GetEventId() << endl;
      RE->SetEventType( c_UnknownEvent );
      RE->SetEventTypeProbability( 0. );
    }
  }
  return true;
}


////////////////////////////////////////////////////////////////////////////////

MString MEREventTypeTorch::ToString(bool CoreOnly) const
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

