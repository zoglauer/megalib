/*
 * MAtmosphericAbsorption.cxx
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


// Include the header:
#include "MAtmosphericAbsorption.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MAtmosphericAbsorption)
#endif


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MAtmosphericAbsorption::MAtmosphericAbsorption()
{
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
MAtmosphericAbsorption::~MAtmosphericAbsorption()
{
}


////////////////////////////////////////////////////////////////////////////////


//! Read the data from file
bool MAtmosphericAbsorption::Read(MString FileName)
{
  return m_TransmissionProbabilities.Set(FileName, "AP");
}


////////////////////////////////////////////////////////////////////////////////


//! Get transmission probability a value
double MAtmosphericAbsorption::GetTransmissionProbability(double Altitude, double Azimuth, double Energy) const
{
  if (Altitude < m_TransmissionProbabilities.GetXMin()) return 0;
  if (Azimuth > m_TransmissionProbabilities.GetYMax()) return 0;
  
  return m_TransmissionProbabilities.Evaluate(Altitude, Azimuth, Energy);
}


// MAtmosphericAbsorption.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
