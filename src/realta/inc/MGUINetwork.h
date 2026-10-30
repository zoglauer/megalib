/*
 * MGUINetwork.h
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


#ifndef __MGUINetwork__
#define __MGUINetwork__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIDialog.h"
#include "MGUIERBList.h"
#include "MSettingsRealta.h"

// Forward declarations:
class MGUIEEntry;

////////////////////////////////////////////////////////////////////////////////


class MGUINetwork : public MGUIDialog
{
  // Public Interface:
 public:
  MGUINetwork(const TGWindow* p, const TGWindow* main, MSettingsRealta* Settings);
  virtual ~MGUINetwork();


  // protected methods:
 protected:
  virtual void Create();
  virtual bool OnApply();
  virtual bool ProcessMessage(Long_t Message, Long_t Parameter1, Long_t Parameter2);


  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  MSettingsRealta* m_Settings;

  MGUIEEntry* m_Host;
  MGUIEEntry* m_Port;

  MGUIERBList* m_TransceiverMode;
  
  TGCheckButton *m_ConnectOnStart;


#ifdef ___CLING___
 public:
  ClassDef(MGUINetwork, 0) // GUI window: ...
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
