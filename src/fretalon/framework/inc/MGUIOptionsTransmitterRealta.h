/*
 * MGUIOptionsTransmitterRealta.h
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


#ifndef __MGUIOptionsTransmitterRealta__
#define __MGUIOptionsTransmitterRealta__


////////////////////////////////////////////////////////////////////////////////

    
// ROOT libs:
#include <TROOT.h>
#include <TVirtualX.h>
#include <TGWindow.h>
#include <TObjArray.h>
#include <TGFrame.h>
#include <TGButton.h>
#include <MString.h>
#include <TGClient.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIEFileSelector.h"
#include "MGUIOptions.h"
#include "MGUIEEntry.h"

// Nuclearizer libs:
#include "MModule.h"


// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MGUIOptionsTransmitterRealta : public MGUIOptions
{
  // public Session:
 public:
  //! Default constructor
  MGUIOptionsTransmitterRealta(MModule* Module);
  //! Default destructor
  virtual ~MGUIOptionsTransmitterRealta();

  //! Process all button, etc. messages
  virtual bool ProcessMessage(long Message, long Parameter1, long Parameter2);

  //! The creation part which gets overwritten
  virtual void Create();

  // protected methods:
 protected:

  //! Actions after the Apply or OK button has been pressed
  virtual bool OnApply();


  // protected members:
 protected:

  // private members:
 private:
  MGUIEEntry* m_HostName;
  MGUIEEntry* m_HostPort;

#ifdef ___CLING___
 public:
  ClassDef(MGUIOptionsTransmitterRealta, 1) // basic class for dialog windows
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
