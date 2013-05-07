/*
 * MGUIEDummy.cxx
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
// MGUIEDummy
//
//
// This class is an elementary GUI-widget:
//
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MGUIEDummy.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MStreams.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MGUIEDummy)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIEDummy::MGUIEDummy(const TGWindow* Parent, MString Label, bool Emphasize) :
  MGUIElement(Parent)
{
  // Creates a frame containing a label and an entry-box 
  //
  // Parent:   parent Window, where this frame is contained
  // Label:    text of the label

  m_Label = Label;
  m_IsEnabled = true;
  m_IsEmphasized = Emphasize;
}


////////////////////////////////////////////////////////////////////////////////


MGUIEDummy::~MGUIEDummy()
{
  // Destruct this instance of MGUIEDummy

  if (MustCleanup() == kNoCleanup) {
    // Do something...
  }
}


////////////////////////////////////////////////////////////////////////////////


void MGUIEDummy::Create()
{
  // Create the label and the input-field.


  // Give this element the default size of its content:
  Resize(GetDefaultWidth(), GetDefaultHeight()); 

  return;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIEDummy::ProcessMessage(long Message, long Parameter1, 
                                long Parameter2)
{
  // Process the messages for this application, mainly the scollbar moves:

  switch (GET_MSG(Message)) {
  case kC_COMMAND:
    switch (GET_SUBMSG(Message)) {
    case kCM_CHECKBUTTON:
      break;
    case kCM_BUTTON:
    case kCM_MENU:
      switch (Parameter1) {
      default:
        break;
      }
      break;
    default:
      break;
    }
    break;
  default:
    break;
  }

  
  return true;
}


// MGUIEDummy.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
