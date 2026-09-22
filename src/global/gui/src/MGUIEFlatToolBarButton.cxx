/*
 * MGUIEFlatToolBarButton.cxx
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
// MGUIEFlatToolBarButton
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MGUIEFlatToolBarButton.h"

// Standard libs:
#include <limits>
using namespace std;

// ROOT libs:
#include "TVirtualX.h"

// MEGAlib libs:
#include "MStreams.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MGUIEFlatToolBarButton)
#endif


//////////////////////////////////////////////////////////////////////////////


MGUIEFlatToolBarButton::MGUIEFlatToolBarButton(const TGWindow* Parent, const TGPicture* Picture, int Id)
  : TGPictureButton(Parent, Picture, Id)
{
  // Construct an instance of MGUIEFlatToolBarButton
  
  m_Flat = false;
  
  gVirtualX->SelectInput(fId, numeric_limits<unsigned int>::max());
}


////////////////////////////////////////////////////////////////////////////////


MGUIEFlatToolBarButton::~MGUIEFlatToolBarButton()
{
  // Delete this instance of MGUIEFlatToolBarButton
}


////////////////////////////////////////////////////////////////////////////////


void MGUIEFlatToolBarButton::SetFlat(const bool IsFlat)
{

  if (m_Flat != IsFlat) {
    m_Flat = IsFlat;

    // Do not set the fOption parameter directly, to take care of boerder-width changes
    int Options = fOptions;

    if (m_Flat == true) {
      Options &= ~kRaisedFrame;
      switch (fState) {
      case kButtonEngaged:
      case kButtonDown:
        Options |= kDoubleBorder;
        Options |= kSunkenFrame;
        break;
      case kButtonDisabled:
      case kButtonUp:
        Options &= ~kDoubleBorder;
        Options &= ~kRaisedFrame;
        break;
      }           
    } else {
      Options |= kDoubleBorder;
      switch (fState) {
      case kButtonEngaged:
      case kButtonDown:
        Options &= ~kRaisedFrame;
        Options |= kSunkenFrame;
        break;
      case kButtonDisabled:
      case kButtonUp:
        Options &= ~kSunkenFrame;
        Options |= kRaisedFrame;
        break;
      }     
    }
    ChangeOptions(Options);
  }
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIEFlatToolBarButton::IsFlat() const
{
  return m_Flat;
}


////////////////////////////////////////////////////////////////////////////////


void MGUIEFlatToolBarButton::SetState(EButtonState state, bool)
{
  // Set button state.
  
  bool was = !IsDown();   // true if button was off

  if (state != fState) {
    // Do not set the fOption parameter directly, to take care of boerder-width changes
    unsigned int Options = fOptions;
    switch (state) {
    case kButtonEngaged:
    case kButtonDown:
      Options |= kSunkenFrame;
//      if (m_Flat == true) {
//        Options |= kDoubleBorder;
//      } else {
        Options &= ~kRaisedFrame;
//      }
      break;
    case kButtonDisabled:
    case kButtonUp:
      Options &= ~kSunkenFrame;
//      if (m_Flat == true) {
//        Options &= ~kDoubleBorder;
//      } else {
        Options |= kRaisedFrame;
//      }
      break;
    }
    ChangeOptions(Options);
    fClient->NeedRedraw(this);
    fState = state;
  }

  bool now = !IsDown();               // true if button now is off

  // emit signals
  if (was && !now) {
    Pressed();                          // emit Pressed  = was off , now on
    if (fStayDown) Clicked();           // emit Clicked
  }

  if (!was && now) {
    Released();                         // emit Released = was on , now off
    Clicked();                          // emit Clicked
  }

  if ((was != now) && IsToggleButton()) Toggled(!now); // emit Toggled  = was != now
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIEFlatToolBarButton::HandleCrossing(Event_t *event)
{
  // Handle mouse crossing event.

  // Modified 9.6.02
  if (m_Flat && (fState == kButtonUp)) {
    //cout<<"Flat & Up"<<endl;
    int Options = fOptions;
   if (event->fType == kEnterNotify) {
    Options |= kDoubleBorder;
    Options |= kRaisedFrame;
    //cout<<"Crossing: Enter"<<endl;
   } else if (event->fType == kLeaveNotify) {
    Options &= ~kDoubleBorder;
    Options &= ~kRaisedFrame;
    //cout<<"Crossing leave!"<<endl;
   } else {
     //cout<<"Crossing !"<<endl;
   }
    ChangeOptions(Options);
   gClient->NeedRedraw(this);
  }

  TGButton::HandleCrossing(event);

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIEFlatToolBarButton::HandleFocusChange(Event_t *event)
{
  
  if (event->fType == kFocusOut) {
    unsigned int Options = fOptions;
    switch (fState) {
    case kButtonEngaged:
    case kButtonDown:
      break;
    case kButtonDisabled:
    case kButtonUp:
      Options &= ~kSunkenFrame;
      Options &= ~kDoubleBorder;
      Options &= ~kRaisedFrame;
      break;
    }
    ChangeOptions(Options);
    fClient->NeedRedraw(this);
  }

  return true;
}


// MGUIEFlatToolBarButton.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
