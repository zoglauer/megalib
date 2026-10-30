/*
 * MSimPM.h
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


#ifndef __MSimPM__
#define __MSimPM__


////////////////////////////////////////////////////////////////////////////////


// Standard libs

// ROOT libs

// MEGAlib libs
#include "MGlobal.h"


////////////////////////////////////////////////////////////////////////////////


class MSimPM
{
  // Public Interface:
 public:
  //! Default constructor of the PM keyword
  MSimPM();
  //! Default constructor of the PM keyword
  MSimPM(const MString MaterialName, const double Energy);
  //! Default destructor
  virtual ~MSimPM();

  //! Parse one line of the sim file 
  bool AddRawInput(MString LineBuffer, const int Version);

  //! Return the number of the detector, where the hit took place
  MString GetMaterialName() const { return m_MaterialName; };
  //! Set the detctor type in which the hit took place
  void SetMaterialName(const MString MaterialName) { m_MaterialName = MaterialName; }

  //! Retrieve the energy
  double GetEnergy() const { return m_Energy; }
  //! Set the energy
  void SetEnergy(const double Energy) { m_Energy = Energy; }

  //! Return the content as a string for the sim file
  MString ToSimString(const int WhatToStore = 1, const int Precision = 0, const int Version = 25) const;

  // protected methods:
 protected:


  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  //! Name of the passive material, where energy was deposited
  MString m_MaterialName;
  //! The energy
  double m_Energy;


#ifdef ___CLING___
 public:
  ClassDef(MSimPM, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
