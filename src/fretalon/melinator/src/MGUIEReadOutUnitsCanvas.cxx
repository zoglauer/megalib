/*
 * MGUIEReadOutUnitsCanvas.cxx
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


// Include the header:
#include "MGUIEReadOutUnitsCanvas.h"

// Standard libs:

// ROOT libs:
#include "TCanvas.h"

// MEGAlib libs:
#include "MGUIMainMelinator.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MGUIEReadOutUnitsCanvas)
#endif


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MGUIEReadOutUnitsCanvas::MGUIEReadOutUnitsCanvas(MGUIMainMelinator* UI, TString Name, const TGWindow* Parent) : TRootEmbeddedCanvas(Name, Parent, 100, 100), m_UI(UI)
{
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
MGUIEReadOutUnitsCanvas::~MGUIEReadOutUnitsCanvas()
{
}


////////////////////////////////////////////////////////////////////////////////


//! Handle button presses in the canvas
Bool_t MGUIEReadOutUnitsCanvas::HandleContainerButton(Event_t *event)
{
  Int_t x = event->fX;
  Int_t y = event->fY;
  
  if (fCanvas != nullptr && fCanvas->GetListOfPrimitives()->First() != nullptr) {
    double HistX, HistY;
    // Y is wrong, only X is right...
    fCanvas->PixeltoXY(x, y, HistX, HistY);
    if (fCanvas->GetLogx() == 1) HistX = pow(10, HistX);
    if (fCanvas->GetLogy() == 1) HistY = pow(10, HistY);
    //cout<<x<<":"<<y<<"      "<<HistX<<":"<<HistY<<endl;
    m_UI->SwitchToLine(HistX);
  }
  
  return TRootEmbeddedCanvas::HandleContainerButton(event);
}

  
// MGUIEReadOutUnitsCanvas.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
