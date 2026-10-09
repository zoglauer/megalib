/*
 * MAtmosphericAbsorption.h
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


#ifndef __MAtmosphericAbsorption__
#define __MAtmosphericAbsorption__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MFunction3D.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! Atmospheric absorption transmission lookup table
class MAtmosphericAbsorption
{
  // public interface:
 public:
  //! Default constructor
  MAtmosphericAbsorption();
  //! Default destuctor 
  virtual ~MAtmosphericAbsorption();

  //! Read the data from file
  bool Read(MString FileName);
  
  //! Get transmission probability for a given altitude, azimuth, and energy.
  double GetTransmissionProbability(double Altitude, double Azimuth, double Energy) const;
  
  
  // protected methods:
 protected:

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  //! The data
  MFunction3D m_TransmissionProbabilities;


#ifdef ___CLING___
 public:
  ClassDef(MAtmosphericAbsorption, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
