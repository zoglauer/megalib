/*
 * MGUIOptionsFileChooser.h
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


#ifndef __MGUIOptionsFileChooser__
#define __MGUIOptionsFileChooser__


////////////////////////////////////////////////////////////////////////////////


// Standard libs
#include <vector>
using namespace std;

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
#include "MModule.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MGUIOptionsFileChooser : public MGUIOptions
{
  // public Session:
 public:
  //! Default constructor
  MGUIOptionsFileChooser(MModule* Module);
  //! Default destructor
  virtual ~MGUIOptionsFileChooser();

  //! Set the description
  void SetDescription(MString Description) { m_Description = Description; }
  //! Set the filetypes (form "*.roa") and there description (form "read-out assembly files")
  void SetFileTypes(vector<MString> Type, vector<MString> TypeDescription) { m_FileTypes = Type; m_FileTypeDescriptions = TypeDescription; }
  
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
  //! The description 
  MString m_Description;
  //! The file types
  vector<MString> m_FileTypes;
  //! The file types description
  vector<MString> m_FileTypeDescriptions;
   
  //! Select which file to load
  MGUIEFileSelector* m_FileSelector;


#ifdef ___CLING___
 public:
  ClassDef(MGUIOptionsFileChooser, 1) // basic class for dialog windows
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
