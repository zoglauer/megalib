/*
 * MModuleLoaderRoa.h
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


#ifndef __MModuleLoaderRoa__
#define __MModuleLoaderRoa__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MFileReadOuts.h"
#include "MModule.h"
#include "MModuleInterfaceFileName.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MModuleLoaderRoa : public MModule, public MModuleInterfaceFileName
{
  // public interface:
 public:
  //! Default constructor
  MModuleLoaderRoa();
  //! Default destructor
  virtual ~MModuleLoaderRoa();

  //! Create a new object of this class 
  virtual MModuleLoaderRoa* Clone() { return new MModuleLoaderRoa(); };

  //! Initialize the module
  virtual bool Initialize();

  //! Initialize the module
  virtual void Finalize();

  //! Main data analysis routine, which updates the event to a new level 
  virtual bool AnalyzeEvent(MReadOutAssembly* Event);

  //! Show the options GUI
  virtual void ShowOptionsGUI();

  //! Read the configuration data from an XML node
  virtual bool ReadXmlConfiguration(MXmlNode* Node);
  //! Create an XML node tree from the configuration
  virtual MXmlNode* CreateXmlConfiguration();


  // protected methods:
 protected:
  //! Reads one event from file - return zero in case of no more events present or an Error occured
  bool ReadNextEvent(MReadOutAssembly* Event);

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  //! The read-out file
  MFileReadOuts m_RoaFile;
  
  
#ifdef ___CLING___
 public:
  ClassDef(MModuleLoaderRoa, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
