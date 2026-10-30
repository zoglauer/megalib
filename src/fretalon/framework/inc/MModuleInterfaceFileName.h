/*
 * MModuleInterfaceFileName.h
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


#ifndef __MModuleInterfaceFileName__
#define __MModuleInterfaceFileName__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! Interface providing a file name for various modules
class MModuleInterfaceFileName
{
  // public interface:
 public:
  //! Default constructor
  MModuleInterfaceFileName();
  //! Default destructor
  virtual ~MModuleInterfaceFileName();

  //! Create a new object of this class 
  virtual MModuleInterfaceFileName* Clone() = 0;

  //! Get the file name
  MString GetFileName() const { return m_FileName; }
  //! Set the file name
  void SetFileName(const MString& Name) { m_FileName = Name; }  


  // protected methods:
 protected:


  // private methods:
 private:



  // protected members:
 protected:
  //! The file name
  MString m_FileName;

  // private members:
 private:

  
  
#ifdef ___CLING___
 public:
  ClassDef(MModuleInterfaceFileName, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
