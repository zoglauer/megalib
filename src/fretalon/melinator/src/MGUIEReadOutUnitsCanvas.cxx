/*
 * MGUIEReadOutUnitsCanvas.cxx
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
#include "MGUIEReadOutUnitsCanvas.h"

// Standard libs:

// ROOT libs:
#include "TCanvas.h"

// MEGAlib libs:
#include "MGUIMainMelinator.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
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
    
  double HistX, HistY;
  if (fCanvas != 0) {
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
