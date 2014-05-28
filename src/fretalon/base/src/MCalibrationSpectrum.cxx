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
#include <algorithm>
using namespace std;

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
  m_Model = 0;
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
MCalibrationSpectrum::~MCalibrationSpectrum()
{
  //delete m_Model;
}


////////////////////////////////////////////////////////////////////////////////


//! Clear the content
void MCalibrationSpectrum::Clear()
{
  m_SpectralPoints.clear();
  delete m_Model;
  m_Model = 0;
}


////////////////////////////////////////////////////////////////////////////////


//! Clone this calibration - the returned calibration must be deleted!
MCalibrationSpectrum* MCalibrationSpectrum::Clone() const
{
  MCalibrationSpectrum* C = new MCalibrationSpectrum();
  C->m_IsCalibrated = m_IsCalibrated;
  C->m_SpectralPoints = m_SpectralPoints;
  if (m_Model != 0) {
    C->SetModel(*m_Model);
  }
  
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
//! Mode: pak  - points ADC to keV
//! Mode: pakw - points ADC to keV and fwhm
//! Mode: model - show the model (none, poly2, poly3, poly2gauss, etc.)
MString MCalibrationSpectrum::ToParsableString(const MString& Mode, bool WithDescriptor)
{
  if (Mode != "pak" && Mode != "pakw" && Mode != "model") {
    throw MExceptionUnknownMode(Mode);
    return "";
  }
  
  ostringstream out;
  
  // Make a single list of the points and store them for sorting
  vector<MCalibrationSpectralPoint> Points = GetUniquePoints();
   
  if (Mode == "pak") {
    if (WithDescriptor == true) {
      out<<"pak "; 
    }
    out<<Points.size()<<" ";

    for (unsigned int p = 0; p < Points.size(); ++p) {
      out<<Points[p].GetPeak()<<" "<<Points[p].GetEnergy()<<" "; 
    }
    out<<endl;
  } else if (Mode == "pakw") {
    if (WithDescriptor == true) {
      out<<"pakw "; 
    }
    out<<Points.size()<<" ";

    for (unsigned int p = 0; p < Points.size(); ++p) {
      out<<Points[p].GetPeak()<<" "<<Points[p].GetEnergy()<<" "<<Points[p].GetEnergyFWHM()<<" "; 
    }     
  } else if (Mode == "model") {
    if (HasModel() == true) {
      out<<m_Model->ToParsableString(WithDescriptor); 
    } else {
      out<<"none"; 
    }
  }
  
  
  return out.str();
}  
  

////////////////////////////////////////////////////////////////////////////////


//! Get a list of all unique spectral points in this spectrum
vector<MCalibrationSpectralPoint> MCalibrationSpectrum::GetUniquePoints()
{
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
  auto comp = [](MCalibrationSpectralPoint a,  MCalibrationSpectralPoint b) -> bool { return a.GetEnergy() < b.GetEnergy(); };
  sort(Points.begin(), Points.end(), comp);
        
  // If we have multiple points with the same energy -- warn for now because we first need to have such an example to test it...


  return Points;
}
  

////////////////////////////////////////////////////////////////////////////////


//! Set the calibration model
void MCalibrationSpectrum::SetModel(MCalibrationModel& Model)
{
  cout<<"Setting model"<<endl;
  delete m_Model;
  m_Model = Model.Clone();
}

////////////////////////////////////////////////////////////////////////////////


//! Get the fit, if it doesn't exist throw MExceptionObjectDoesNotExist
//! Bad design since a new fit can be set any time...
MCalibrationModel& MCalibrationSpectrum::GetModel() 
{ 
  if (m_Model == 0) {
    throw MExceptionObjectDoesNotExist("Model does not exist!");
  }
  
  return *m_Model; 
}

// MCalibrationSpectrum.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
