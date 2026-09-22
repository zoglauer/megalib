/*
 * MGUIEReadOutUnitsCanvas.h
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


#ifdef ___CLING___
 public:
  ClassDef(MGUIEReadOutUnitsCanvas, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
