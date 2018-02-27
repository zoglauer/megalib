/*
 * MAtmosphericAbsorption.cxx
 *
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 *
 * This code implementation is the intellectual property of
 * Andreas Zoglauer.
 *
 * By copying, distributing or modifying the Program (or any work
 * based on the Program) you indicate your acceptance of this statement,
 * and all its terms.
 *
 */


// Include the header:
#include "MAtmosphericAbsorption.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
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
