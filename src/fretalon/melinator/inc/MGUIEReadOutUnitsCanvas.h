/*
 * MGUIEReadOutUnitsCanvas.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUIEReadOutUnitsCanvas__
#define __MGUIEReadOutUnitsCanvas__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:
#include "TRootEmbeddedCanvas.h"

// MEGAlib libs:
#include "MGlobal.h"

// Forward declarations:
class MGUIMainMelinator;

////////////////////////////////////////////////////////////////////////////////


//! A canvas which handles button presses by the user
class MGUIEReadOutUnitsCanvas : public TRootEmbeddedCanvas
{
  // public interface:
 public:
  //! Default constructor
  MGUIEReadOutUnitsCanvas(MGUIMainMelinator* UI, TString Name, const TGWindow* Parent);
  //! Default destuctor 
  virtual ~MGUIEReadOutUnitsCanvas();

  //! Handle button presses in the canvas
  virtual Bool_t HandleContainerButton(Event_t * Event);
  
  // protected methods:
 protected:

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  //! The user interface to call back
  MGUIMainMelinator* m_UI;


#ifdef ___CINT___
 public:
  ClassDef(MGUIEReadOutUnitsCanvas, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
