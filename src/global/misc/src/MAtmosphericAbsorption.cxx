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
#include "MStreams.h"


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


//! Get transmission probability for a given altitude, azimuth, and energy
double MAtmosphericAbsorption::GetTransmissionProbability(double Altitude, double Azimuth, double Energy) const
{
  if (Altitude < m_TransmissionProbabilities.GetXMin()) {
    mout<<"Warning in MAtmosphericAbsorption::GetTransmissionProbability: altitude "<<Altitude
        <<" is below the tabulated range ["<<m_TransmissionProbabilities.GetXMin()<<", "
        <<m_TransmissionProbabilities.GetXMax()<<"]; using tabulated minimum altitude"<<endl;
    Altitude = m_TransmissionProbabilities.GetXMin();
  }
  if (Altitude > m_TransmissionProbabilities.GetXMax()) {
    mout<<"Warning in MAtmosphericAbsorption::GetTransmissionProbability: altitude "<<Altitude
        <<" is above the tabulated range ["<<m_TransmissionProbabilities.GetXMin()<<", "
        <<m_TransmissionProbabilities.GetXMax()<<"]; using tabulated maximum altitude"<<endl;
    Altitude = m_TransmissionProbabilities.GetXMax();
  }
  if (Azimuth < m_TransmissionProbabilities.GetYMin()) {
    mout<<"Warning in MAtmosphericAbsorption::GetTransmissionProbability: azimuth "<<Azimuth
        <<" is below the tabulated range ["<<m_TransmissionProbabilities.GetYMin()<<", "
        <<m_TransmissionProbabilities.GetYMax()<<"]; using tabulated minimum azimuth"<<endl;
    Azimuth = m_TransmissionProbabilities.GetYMin();
  }
  if (Azimuth > m_TransmissionProbabilities.GetYMax()) {
    mout<<"Warning in MAtmosphericAbsorption::GetTransmissionProbability: azimuth "<<Azimuth
        <<" is above the tabulated range ["<<m_TransmissionProbabilities.GetYMin()<<", "
        <<m_TransmissionProbabilities.GetYMax()<<"]; using tabulated maximum azimuth"<<endl;
    Azimuth = m_TransmissionProbabilities.GetYMax();
  }
  if (Energy < m_TransmissionProbabilities.GetZMin()) {
    mout<<"Warning in MAtmosphericAbsorption::GetTransmissionProbability: energy "<<Energy
        <<" is below the tabulated range ["<<m_TransmissionProbabilities.GetZMin()<<", "
        <<m_TransmissionProbabilities.GetZMax()<<"]; using tabulated minimum energy"<<endl;
    Energy = m_TransmissionProbabilities.GetZMin();
  }
  if (Energy > m_TransmissionProbabilities.GetZMax()) {
    mout<<"Warning in MAtmosphericAbsorption::GetTransmissionProbability: energy "<<Energy
        <<" is above the tabulated range ["<<m_TransmissionProbabilities.GetZMin()<<", "
        <<m_TransmissionProbabilities.GetZMax()<<"]; using tabulated maximum energy"<<endl;
    Energy = m_TransmissionProbabilities.GetZMax();
  }
  
  return m_TransmissionProbabilities.Evaluate(Altitude, Azimuth, Energy);
}


// MAtmosphericAbsorption.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
