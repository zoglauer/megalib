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


#ifdef ___CINT___
ClassImp(MDStrip3DDirectional)
#endif


////////////////////////////////////////////////////////////////////////////////


MDStrip3DDirectional::MDStrip3DDirectional(MString Name) : MDStrip3D(Name)
{
  // Construct an instance of MDStrip3DDirectional

  m_Type = c_Strip3DDirectional;
  m_Description = c_Strip3DDirectionalName;

  m_DirectionalResolution = new MSpline(MSpline::Interpolation);
  m_DirectionalResolutionSigma = new MSpline(MSpline::Interpolation); 
}


////////////////////////////////////////////////////////////////////////////////


MDStrip3DDirectional::MDStrip3DDirectional(const MDStrip3DDirectional& S)
{
  m_DirectionalResolution = new MSpline(*(S.m_DirectionalResolution)); 
  m_DirectionalResolutionSigma = new MSpline(*(S.m_DirectionalResolutionSigma)); 
}

 
////////////////////////////////////////////////////////////////////////////////


MDDetector* MDStrip3DDirectional::Clone()
{
  // Duplicate this detector

  massert(this != 0);
  return new MDStrip3DDirectional(*this);
}


////////////////////////////////////////////////////////////////////////////////


MDStrip3DDirectional::~MDStrip3DDirectional()
{
  // Delete this instance of MDStrip3DDirectional

  delete m_DirectionalResolution; 
  delete m_DirectionalResolutionSigma; 
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

  m_DirectionalResolution->AddDataPoint(Energy, Resolution);
  m_DirectionalResolutionSigma->AddDataPoint(Energy, Sigma);
}


////////////////////////////////////////////////////////////////////////////////


bool MDStrip3DDirectional::NoiseDirection(MVector& Dir, double Energy) const
{
  // Noise the direction

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
    Sigma = gRandom->Gaus(m_DirectionalResolution->Get(Energy), 
                          m_DirectionalResolutionSigma->Get(Energy));
    Trials--;
  } while (Sigma < 0 && Trials >= 0);
  if (Trials <= 0) {
    Sigma = m_DirectionalResolution->Get(Energy);
  }
  
  Angle = gRandom->Gaus(Angle, Sigma);

  // Back to x and y: 
  Dir[0] = cos(Angle);
  Dir[1] = sin(Angle);
  Dir[2] = 0.0;


  mimp<<"One could introduce some kind of threshold here..."<<show;
  // if (Something) {
  // Dir = g_VectorNotDefined;
  // }

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
  for (int d = 0; d < m_DirectionalResolution->GetNDataPoints(); ++d) {
    out<<m_Name<<".DirectionalResolution "<<
      m_DirectionalResolution->GetDataPointXValueAt(d)<<" "<<
      m_DirectionalResolution->GetDataPointYValueAt(d)<<" "<<
      m_DirectionalResolutionSigma->GetDataPointYValueAt(d)<<endl;
  }  
  
  return out.str().c_str();  
}


////////////////////////////////////////////////////////////////////////////////


MString MDStrip3DDirectional::GetMGeant() const
{
  mimp<<"This class is not yet ready for MGGPOD's MEGAlib extension (standard MGGPOD ok)!!!"<<endl;

  return MDStrip3D::GetMGeant();  
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

  return true;
}


// MDStrip3DDirectional.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
