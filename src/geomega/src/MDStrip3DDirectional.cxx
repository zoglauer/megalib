/*
 * MDStrip3DDirectional.cxx
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


////////////////////////////////////////////////////////////////////////////////
//
// MDStrip3DDirectional
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MDStrip3DDirectional.h"

// Standard libs:
#include <limits>
#include <sstream>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MAssert.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MDStrip3DDirectional)
#endif


////////////////////////////////////////////////////////////////////////////////


const int MDStrip3DDirectional::c_DirectionalResolutionTypeUnknown  = 0;
const int MDStrip3DDirectional::c_DirectionalResolutionTypeIdeal    = 1;
const int MDStrip3DDirectional::c_DirectionalResolutionTypeGauss    = 2;


////////////////////////////////////////////////////////////////////////////////


MDStrip3DDirectional::MDStrip3DDirectional(MString Name) : MDStrip3D(Name)
{
  // Construct an instance of MDStrip3DDirectional

  m_Type = c_Strip3DDirectional;
  m_Description = c_Strip3DDirectionalName;

  m_DirectionalResolutionType = c_DirectionalResolutionTypeUnknown;
}


////////////////////////////////////////////////////////////////////////////////


MDStrip3DDirectional::MDStrip3DDirectional(const MDStrip3DDirectional& S) : MDStrip3D(S)
{
  m_DirectionalResolutionType = S.m_DirectionalResolutionType;
  m_DirectionalResolution = S.m_DirectionalResolution; 
  m_DirectionalResolutionSigma = S.m_DirectionalResolutionSigma; 
}

 
////////////////////////////////////////////////////////////////////////////////


MDDetector* MDStrip3DDirectional::Clone()
{
  // Duplicate this detector

  massert(this != 0);
  return new MDStrip3DDirectional(*this);
}


////////////////////////////////////////////////////////////////////////////////


bool MDStrip3DDirectional::CopyDataToNamedDetectors()
{
  //! Copy data to named detectors
  
  MDDetector::CopyDataToNamedDetectors();
  
  if (m_IsNamedDetector == true) return true;
  
  for (unsigned int n = 0; n < m_NamedDetectors.size(); ++n) {
    if (dynamic_cast<MDStrip3DDirectional*>(m_NamedDetectors[n]) == 0) {
      mout<<"   ***  Internal error  ***  in detector "<<m_Name<<endl;
      mout<<"We have a named detector ("<<m_NamedDetectors[n]->GetName()<<") which is not of the same type as the base detector!"<<endl;
      return false;
    }
    MDStrip3DDirectional* D = dynamic_cast<MDStrip3DDirectional*>(m_NamedDetectors[n]);

    if (D->m_DirectionalResolutionType == c_DirectionalResolutionTypeUnknown && 
        m_DirectionalResolutionType != c_DirectionalResolutionTypeUnknown) {
      D->m_DirectionalResolutionType = m_DirectionalResolutionType; 
      D->m_DirectionalResolution = m_DirectionalResolution; 
      D->m_DirectionalResolutionSigma = m_DirectionalResolutionSigma; 
    }
  }
   
  return true; 
}


////////////////////////////////////////////////////////////////////////////////


MDStrip3DDirectional::~MDStrip3DDirectional()
{
  // Delete this instance of MDStrip3DDirectional
}


////////////////////////////////////////////////////////////////////////////////


void MDStrip3DDirectional::SetDirectionalResolutionAt(const double Energy, 
                                                      const double Resolution, 
                                                      const double Sigma)
{
  // Set a variable depth resolution

  if (Energy < 0) {
    mout<<"   ***  Error  ***  in detector "<<m_Name<<endl;
    mout<<"Energy for directional resolution needs to be non-negative!"<<endl;
    return; 
  }
  if (Resolution <= 0) {
    mout<<"   ***  Error  ***  in detector "<<m_Name<<endl;
    mout<<"Directional resolution needs to be positive!"<<endl;
    return; 
  }
  if (Sigma < 0) {
    mout<<"   ***  Error  ***  in detector "<<m_Name<<endl;
    mout<<"Sigma of directional resolution needs to be non-negative!"<<endl;
    return; 
  }

  m_DirectionalResolutionType = c_DirectionalResolutionTypeGauss;
  m_DirectionalResolution.Add(Energy, Resolution);
  m_DirectionalResolutionSigma.Add(Energy, Sigma);
}


////////////////////////////////////////////////////////////////////////////////


bool MDStrip3DDirectional::NoiseDirection(MVector& Dir, double Energy) const
{
  // Noise the direction

  if (m_DirectionalResolutionType == c_DirectionalResolutionTypeIdeal) {
    // do nothing
  } else if (m_DirectionalResolutionType == c_DirectionalResolutionTypeGauss) {
    
    double Angle = 0.0;
    Energy = sqrt(Dir[0]*Dir[0] + Dir[1]*Dir[1]);

    Dir[2] = 0.0;
    Dir.Unitize();


    // Find the angle:
    if (Dir[0] == 0.0 && Dir[1] == 0.0) {
      Angle = 0.0;
    } else {
      if (Dir[0] != 0) {
        Angle = atan2(Dir[1], Dir[0]);
      } else { 
        if (Dir[1] > 0) {
          Angle = c_Pi/2;
        } else {
          Angle = -c_Pi/2;
        }
      }
    }

    // Determine the resolution:
    int Trials = 5;
    double Sigma;
    do {
      Sigma = gRandom->Gaus(m_DirectionalResolution.Evaluate(Energy), 
                            m_DirectionalResolutionSigma.Evaluate(Energy));
      Trials--;
    } while (Sigma < 0 && Trials >= 0);
    if (Trials <= 0) {
      Sigma = m_DirectionalResolution.Evaluate(Energy);
    }
  
    Angle = gRandom->Gaus(Angle, Sigma);

    // Back to x and y: 
    Dir[0] = cos(Angle);
    Dir[1] = sin(Angle);
    Dir[2] = 0.0;
    
  } else {
    mout<<"   ***  Error  ***  in detector "<<m_Name<<endl;
    mout<<"Unknown directional resolution type: "<<m_DirectionalResolutionType<<endl;
    return false;   
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


MString MDStrip3DDirectional::GetGeomega() const
{
  // Return all detector characteristics in Geomega-Format

  ostringstream out;

  MString Strip3D = MDStrip3D::GetGeomega();
  Strip3D.ReplaceAll("Strip3D", "Strip3DDirectional");

  out<<Strip3D;
  for (unsigned int d = 0; d < m_DirectionalResolution.GetNDataPoints(); ++d) {
    out<<m_Name<<".DirectionalResolution "<<
      m_DirectionalResolution.GetDataPointX(d)<<" "<<
      m_DirectionalResolution.GetDataPointY(d)<<" "<<
      m_DirectionalResolutionSigma.GetDataPointY(d)<<endl;
  }  
  
  return out.str().c_str();  
}


////////////////////////////////////////////////////////////////////////////////


MString MDStrip3DDirectional::ToString() const
{
  //

  ostringstream out;

  out<<"Detector "<<m_Name<<" - 3D-Strip"<<endl;
  out<<"   with sensitive volumes: ";  
  for (unsigned int i = 0; i < m_SVs.size(); i++) {
    out<<m_SVs[i]->GetName()<<" ";
  }
  out<<endl<<"   width: "<<m_WidthX<<", "<<m_WidthY<<endl;
  out<<"   offset: "<<m_OffsetX<<", "<<m_OffsetY<<endl;
  out<<"   pitch: "<<m_PitchX<<", "<<m_PitchY<<endl;
  out<<"   striplength: "<<m_StripLengthX<<", "<<m_StripLengthY<<endl;
  out<<"   stripnumber: "<<m_NStripsX<<", "<<m_NStripsY<<endl;

  return out.str().c_str();  
}


////////////////////////////////////////////////////////////////////////////////


//! Check if all input is reasonable
bool MDStrip3DDirectional::Validate()
{
  if (MDStrip3D::Validate() == false) {
    return false;
  }

  if (m_DirectionalResolutionType == c_DirectionalResolutionTypeUnknown) {
    mout<<"   ***  Info  ***  for detector "<<m_Name<<endl;
    mout<<"No dirdctional resolution defined --- assuming ideal"<<endl; 
    m_DirectionalResolutionType = c_DirectionalResolutionTypeIdeal;
  }

  
  return true;
}


// MDStrip3DDirectional.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
