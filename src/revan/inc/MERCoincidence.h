/*
 * MERCoincidence.h
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


#ifndef __MERCoincidence__
#define __MERCoincidence__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <TROOT.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MTime.h"
#include "MERConstruction.h"
#include "MRawEventIncarnations.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MERCoincidence : public MERConstruction
{
  // public interface:
 public:
  //! Default constructor
  MERCoincidence();
  //! Default destructor
  virtual ~MERCoincidence();

  //! Set the coincidence window
  bool SetCoincidenceWindow(double Time);
  //! Search for coincidences --- return the event or zero if non found
  MRERawEvent* Search(MRawEventIncarnations* List, bool Clear);

  //! Dump some elemenatry information about the algorithm settings
  MString ToString(bool CoreOnly) const;

  //! ID representing no coincidence search
  static const int c_None = 0;
  //! ID representing a coincidence window
  static const int c_Window = 1;

  // protected methods:
 protected:

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  //! The used algorithm
  int m_Algorithm;

  //! The coincidence window
  MTime m_Window;

  //! The number of found coincidences
  int m_NFoundCoincidences;
  

#ifdef ___CLING___
 public:
  ClassDef(MERCoincidence, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
