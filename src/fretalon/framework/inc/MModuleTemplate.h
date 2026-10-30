/*
 * MModuleTemplate.h
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


#ifndef __MModuleTemplate__
#define __MModuleTemplate__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MModule.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MModuleTemplate : public MModule
{
  // public interface:
 public:
  //! Default constructor
  MModuleTemplate();
  //! Default destructor
  virtual ~MModuleTemplate();
  
  //! Create a new object of this class 
  virtual MModuleTemplate* Clone() { return new MModuleTemplate(); }
  
  //! Initialize the module
  virtual bool Initialize();

  //! Main data analysis routine, which updates the event to a new level 
  virtual bool AnalyzeEvent(MReadOutAssembly* Event);
  
  //! Finalize the module
  virtual void Finalize();

  //! Show the options GUI
  virtual void ShowOptionsGUI();

  //! Read the configuration data from an XML node
  virtual bool ReadXmlConfiguration(MXmlNode* Node);
  //! Create an XML node tree from the configuration
  virtual MXmlNode* CreateXmlConfiguration();


  // protected methods:
 protected:

  // private methods:
 private:
  //! No Copy constructor
  MModuleTemplate(const MModuleTemplate&) = delete;
  //! No copying whatsoever
  MModuleTemplate& operator=(const MModuleTemplate&) = delete;


  // protected members:
 protected:


  // private members:
 private:




#ifdef ___CLING___
 public:
  ClassDef(MModuleTemplate, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
