/*
 * MGUIEDummy.h
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


#ifndef __MGUIEDummy__
#define __MGUIEDummy__


////////////////////////////////////////////////////////////////////////////////


// Standardlibs:

// ROOT libs:
#include <TROOT.h>
#include <TGFrame.h>
#include <TGLabel.h>
#include <MString.h>
#include <TGButton.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIElement.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MGUIEDummy : public MGUIElement
{
  // public interface:
 public:
  //! Standard constructor
  MGUIEDummy(const TGWindow* Parent, MString Label, bool Emphasize); 
  //! Default destructor
  virtual ~MGUIEDummy();

  //! Create the GUI
  void Create();
  //! Process or redirect all messages
  bool ProcessMessage(long Message, long Parameter1, long Parameter2);

  // private methods:
 private:

  // private members:
 private:
  MString m_Label;

  //! The Message-IDs of all manipulatable elements 
  enum Type { e_Nothing };


#ifdef ___CLING___
 public:
  ClassDef(MGUIEDummy, 0) // GUI window for unkown purpose ...
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
