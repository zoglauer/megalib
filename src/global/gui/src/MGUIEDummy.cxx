/*
 * MGUIEDummy.cxx
 *
 * Copyright (C) by the MEGAlib contributors.
 *
 * This file is part of MEGAlib.
 *
 * MEGAlib is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * MEGAlib is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
 * License (License.md) for more details.
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
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


#ifdef ___CLING___
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
