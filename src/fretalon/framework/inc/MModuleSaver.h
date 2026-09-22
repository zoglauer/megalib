/*
 * MModuleSaver.h
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


#ifndef __MModuleSaver__
#define __MModuleSaver__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <fstream>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MString.h"

// Nuclearizer libs:
#include "MModule.h"
#include "MReadOutFileFormat.h"
#include "MModuleInterfaceFileName.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MModuleSaver : public MModule, public MModuleInterfaceFileName
{
  // public interface:
 public:
  //! Default constructor
  MModuleSaver();
  //! Default destructor
  virtual ~MModuleSaver();

  //! Create a new object of this class 
  virtual MModuleSaver* Clone()  { return new MModuleSaver(); }
  
  //! Initialize the module
  virtual bool Initialize();

  //! Finalize the module
  virtual void Finalize();

  //! Main data analysis routine, which updates the event to a new level 
  virtual bool AnalyzeEvent(MReadOutAssembly* Event);

  //! Show the options GUI
  virtual void ShowOptionsGUI();

  //! Set the read-out file fomat to be written in the header
  void SetReadOutAssemblyFileFormat(const MReadOutFileFormat& Format) { m_RoaFileFormat = Format; }

  //! Read the configuration data from an XML node
  virtual bool ReadXmlConfiguration(MXmlNode* Node);
  //! Create an XML node tree from the configuration
  virtual MXmlNode* CreateXmlConfiguration();

  static const unsigned int c_RoaFile  = 0;
  static const unsigned int c_DatFile  = 1;
  static const unsigned int c_EvtaFile = 2;
  static const unsigned int c_SimFile  = 3;
  
  // protected methods:
 protected:
  
  // private methods:
 private:

  // protected members:
 protected:


  // private members:
 private:
  //! The operation mode
  unsigned int m_Mode;
  
  //! Output stream for dat file
  ofstream m_Out;

  //! The read-out file fomat to be written in the header
  MReadOutFileFormat m_RoaFileFormat;

  
#ifdef ___CLING___
 public:
  ClassDef(MModuleSaver, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
