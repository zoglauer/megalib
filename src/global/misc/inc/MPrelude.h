/*
 * MPrelude.h
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


#ifndef __MPrelude__
#define __MPrelude__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MSettingsGlobal.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MPrelude
{
  // public interface:
 public:
  //! The prelude: Show the change log
  MPrelude();
  //! Default destructor
  virtual ~MPrelude();

  //! Start the prelude
  bool Play();

  // protected methods:
 protected:

  
  // private methods:
 private:


  // protected members:
 protected:


  // private members:
 private:
  //! The key data read from an XML file and stored there afterwards
  MSettingsGlobal m_Settings;  
  
#ifdef ___CLING___
 public:
  ClassDef(MPrelude, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
