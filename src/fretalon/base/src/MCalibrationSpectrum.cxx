/*
 * MCalibrationSpectrum.cxx
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
#include "MCalibrationSpectrum.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MExceptions.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MCalibrationSpectrum)
#endif


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MCalibrationSpectrum::MCalibrationSpectrum()
{
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
MCalibrationSpectrum::~MCalibrationSpectrum()
{
}


////////////////////////////////////////////////////////////////////////////////


//! Clear the content
void MCalibrationSpectrum::Clear()
{
  m_SpectralPoints.clear();
}


////////////////////////////////////////////////////////////////////////////////


//! Clone this calibration - the returned calibration must be deleted!
MCalibrationSpectrum* MCalibrationSpectrum::Clone() const
{
  MCalibrationSpectrum* C = new MCalibrationSpectrum();
  C->m_IsCalibrated = m_IsCalibrated;
  C->m_SpectralPoints = m_SpectralPoints;
  
  return C;
}


////////////////////////////////////////////////////////////////////////////////


//! Add a spectral point to the read-out data group
void MCalibrationSpectrum::AddSpectralPoint(unsigned int ROG, const MCalibrationSpectralPoint& Point) 
{ 
  if (ROG < m_SpectralPoints.size()) {
    m_SpectralPoints[ROG].push_back(Point);
    return;
  }

  throw MExceptionIndexOutOfBounds(0, m_SpectralPoints.size(), ROG);
}

  
////////////////////////////////////////////////////////////////////////////////


//! Get the number of spectral points in the roud-out data group
unsigned int MCalibrationSpectrum::GetNumberOfSpectralPoints(unsigned int ROG) const 
{ 
  if (ROG < m_SpectralPoints.size()) {
    return m_SpectralPoints[ROG].size(); 
  }
  
  throw MExceptionIndexOutOfBounds(0, m_SpectralPoints.size(), ROG);
  
  return 0;
}


////////////////////////////////////////////////////////////////////////////////


//! Get a spectral point 
//! If it does not exist return an MExceptionIndexOutOfBounds exception
MCalibrationSpectralPoint& MCalibrationSpectrum::GetSpectralPoint(unsigned int ROG, unsigned int i)
{
  if (ROG < m_SpectralPoints.size()) {
    if (i < m_SpectralPoints[ROG].size()) {
      return m_SpectralPoints[ROG][i];
    }

    throw MExceptionIndexOutOfBounds(0, m_SpectralPoints[ROG].size(), i);
  
    // Might crash, but will never reach this point unless exceptions are turned off
    return m_SpectralPoints[0][0];
  }
  
  throw MExceptionIndexOutOfBounds(0, m_SpectralPoints.size(), i);
  
  // Might crash, but will never reach this point unless exceptions are turned off
  return m_SpectralPoints[0][0];
}
  

////////////////////////////////////////////////////////////////////////////////


//! Return the calibration in a saveable format
MString MCalibrationSpectrum::ToParsableString(bool WithDescriptor)
{
  ostringstream out;
  
  // Make a single list of the points and store them for sorting
  vector<MCalibrationSpectralPoint> Points;
  for (unsigned int rog = 0; rog < m_SpectralPoints.size(); ++rog) {
    for (unsigned int p = 0; p < m_SpectralPoints[rog].size(); ++p) {
      if (GetSpectralPoint(rog, p).IsGood()) {
        Points.push_back(GetSpectralPoint(rog, p));
      }
    }
  }
  
  // Sort the points by energy:
      
  // If we have multiple points with the same energy -- warn for now because we first need to have such an example to test it...
      
  if (WithDescriptor == true) {
    out<<"pak "; 
  }
  out<<Points.size()<<" ";

  for (unsigned int p = 0; p < Points.size(); ++p) {
    out<<Points[p].GetPeak()<<" "<<Points[p].GetEnergy()<<" "; 
  }
  out<<endl;

  return out.str();
}  



// MCalibrationSpectrum.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
