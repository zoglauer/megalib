/*
 * MGUIEModule.h
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


#ifndef __MGUIEModule__
#define __MGUIEModule__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include "TGFrame.h"
#include "TGLabel.h"
#include "TObjArray.h"

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIElement.h"
#include "MModule.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MGUIEModule : public MGUIElement
{
  // Public Interface:
 public:
  //! Default constructor
  MGUIEModule(const TGWindow* Parent, unsigned int ID, MModule* Module = 0);
  //! default destructor
  virtual ~MGUIEModule();

  //! Transfer button clicks to this window
  void Associate(const TGWindow* Window);

  //! Enable/disable the change and remove button
  void SetEnabled(bool Enabled);

  // protected methods:
 protected:
  //! Create this GUI element
  void Create();


  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  //! ID
  unsigned int m_ID;
  //! The module
  MModule* m_Module;

  //! The options button
  TGTextButton* m_OptionsButton;
  //! The remove button
  TGTextButton* m_RemoveButton;
  //! The change button
  TGTextButton* m_ChangeButton;


#ifdef ___CLING___
 public:
  ClassDef(MGUIEModule, 0) // GUI window for unkown purpose ...
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
