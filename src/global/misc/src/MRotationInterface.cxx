/*
 * MRotationInterface.cxx
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
#include "MRotationInterface.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MRotationInterface)
#endif


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MRotationInterface::MRotationInterface()
{
  Reset();
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
MRotationInterface::~MRotationInterface()
{
}


////////////////////////////////////////////////////////////////////////////////


//! Set all data
void MRotationInterface::Set(MRotationInterface& RO)
{
  m_Id = RO.m_Id;
  
  m_GalacticPointingXAxis = RO.m_GalacticPointingXAxis;
  m_GalacticPointingZAxis = RO.m_GalacticPointingZAxis;
  m_HasGalacticPointing = RO.m_HasGalacticPointing; 

  m_DetectorRotationXAxis = RO.m_DetectorRotationXAxis;
  m_DetectorRotationZAxis = RO.m_DetectorRotationZAxis;
  m_HasDetectorRotation = RO.m_HasDetectorRotation; 
  
  m_HorizonPointingXAxis = RO.m_HorizonPointingXAxis;
  m_HorizonPointingZAxis = RO.m_HorizonPointingZAxis;
  m_HasHorizonPointing = RO.m_HasHorizonPointing;   
}


////////////////////////////////////////////////////////////////////////////////


//! Reset to default values
void MRotationInterface::Reset()
{
  m_Id = 0;
  
  m_GalacticPointingXAxis = MVector(1.0, 0.0, 0.0);
  m_GalacticPointingZAxis = MVector(0.0, 0.0, 1.0);
  m_HasGalacticPointing = false; 
  m_IsGalacticPointingRotationCalculated = false;
  m_IsGalacticPointingInverseRotationCalculated = false;

  m_DetectorRotationXAxis = MVector(1.0, 0.0, 0.0);
  m_DetectorRotationZAxis = MVector(0.0, 0.0, 1.0);
  m_HasDetectorRotation = false; 
  
  m_HorizonPointingXAxis = MVector(1.0, 0.0, 0.0);
  m_HorizonPointingZAxis = MVector(0.0, 0.0, 1.0);
  m_HasHorizonPointing = false; 
}

  
////////////////////////////////////////////////////////////////////////////////


//! Parse a line
//! Returns 0, if the line got correctly parsed
//! Returns 1, if the line got not correctly parsed 
int MRotationInterface::ParseLine(const char* Line, bool Fast)
{
  int Ret = 0;
  if (Line[0] == 'R' && Line[1] == 'X') {
    if (Fast == true) {
      char* p;
      m_DetectorRotationXAxis[0] = strtod(Line+3, &p);
      m_DetectorRotationXAxis[1] = strtod(p, &p);
      m_DetectorRotationXAxis[2] = strtod(p, NULL);
    } else {
      if (sscanf(Line, "RX %lf %lf %lf", &m_DetectorRotationXAxis[0], &m_DetectorRotationXAxis[1], &m_DetectorRotationXAxis[2]) != 3) {
        Ret = 1;
      }
    }
    m_HasDetectorRotation = true;
  } else if (Line[0] == 'R' && Line[1] == 'Z') {
    if (Fast == true) {
      char* p;
      m_DetectorRotationZAxis[0] = strtod(Line+3, &p);
      m_DetectorRotationZAxis[1] = strtod(p, &p);
      m_DetectorRotationZAxis[2] = strtod(p, NULL);
    } else {
      if (sscanf(Line, "RZ %lf %lf %lf", &m_DetectorRotationZAxis[0], &m_DetectorRotationZAxis[1], &m_DetectorRotationZAxis[2]) != 3) {
        Ret = 1;
      }
    }
    m_HasDetectorRotation = true;
  } else if (Line[0] == 'G' && Line[1] == 'X') {
    double Longitude, Latitude;
    if (Fast == true) {
      char* p;
      Longitude = strtod(Line+3, &p);
      Latitude = strtod(p, NULL);
    } else {
      if (sscanf(Line, "GX %lf %lf", &Longitude, &Latitude) != 2) {
        Ret = 1;
      }
    }
    SetGalacticPointingXAxis(Longitude, Latitude);
  } else if (Line[0] == 'G' && Line[1] == 'Z') {
    double Longitude, Latitude;
    if (Fast == true) {
      char* p;
      Longitude = strtod(Line+3, &p);
      Latitude = strtod(p, NULL);
    } else {
      if (sscanf(Line, "GZ %lf %lf", &Longitude, &Latitude) != 2) {
        Ret = 1;
      }
    }
    SetGalacticPointingZAxis(Longitude, Latitude);
  } else if (Line[0] == 'H' && Line[1] == 'X') {
    double Azimuth, Elevation;
    if (Fast == true) {
      char* p;
      Azimuth = strtod(Line+3, &p);
      Elevation = strtod(p, NULL);
    } else {
      if (sscanf(Line, "HX %lf %lf", &Azimuth, &Elevation) != 2) {
        Ret = 1;
      }
    }
    SetHorizonPointingXAxis(Azimuth, Elevation);
  } else if (Line[0] == 'H' && Line[1] == 'Z') {
    double Azimuth, Elevation;
    if (Fast == true) {
      char* p;
      Azimuth = strtod(Line+3, &p);
      Elevation = strtod(p, NULL);
    } else {
      if (sscanf(Line, "HZ %lf %lf", &Azimuth, &Elevation) != 2) {
        Ret = 1;
      }
    }
    SetHorizonPointingZAxis(Azimuth, Elevation);
  }
  
  return Ret;
}

  
////////////////////////////////////////////////////////////////////////////////


//! Stream to a file
//! Reading has to be done in the derived class 
void MRotationInterface::Stream(ostringstream& S)
{
  if (m_HasGalacticPointing == true) {
    double phi = m_GalacticPointingXAxis.Phi()*c_Deg;
    while (phi < 0.0) phi += 360.0;
    S<<"GX "<<phi<<" "<<m_GalacticPointingXAxis.Theta()*c_Deg - 90<<endl;
    phi = m_GalacticPointingZAxis.Phi()*c_Deg;
    while (phi < 0.0) phi += 360.0;
    S<<"GZ "<<phi<<" "<<m_GalacticPointingZAxis.Theta()*c_Deg - 90<<endl;
  } 
  if (m_HasDetectorRotation == true) {
    S<<"RX "<<m_DetectorRotationXAxis.X()<<" "<<m_DetectorRotationXAxis.Y()<<" "<<m_DetectorRotationXAxis.Z()<<endl;
    S<<"RZ "<<m_DetectorRotationZAxis.X()<<" "<<m_DetectorRotationZAxis.Y()<<" "<<m_DetectorRotationZAxis.Z()<<endl;
  }
  if (m_HasHorizonPointing == true) {
    S<<"HX "<<m_HorizonPointingXAxis.Phi()*c_Deg<<" "<<90 - m_HorizonPointingXAxis.Theta()*c_Deg<<endl;
    S<<"HZ "<<m_HorizonPointingZAxis.Phi()*c_Deg<<" "<<90 - m_HorizonPointingZAxis.Theta()*c_Deg<<endl;
  }  
}


////////////////////////////////////////////////////////////////////////////////


bool MRotationInterface::Validate()
{
  //! Check if the data is OK

  if (m_HasGalacticPointing == true) {
    if (m_GalacticPointingXAxis.Phi() < 0.000001 &&
        m_GalacticPointingXAxis.Theta() < 0.000001 &&
        m_GalacticPointingZAxis.Phi() < 0.000001 &&
        m_GalacticPointingZAxis.Theta() < 0.000001) {
      cout<<"Error: Event ("<<m_Id<<") has no valid galactic pointing"<<endl;
      m_HasGalacticPointing = false;
    }
  }
  
  if (m_HasHorizonPointing == true) {
    if (m_HorizonPointingXAxis.Phi() < 0.000001 &&
        m_HorizonPointingXAxis.Theta() < 0.000001 &&
        m_HorizonPointingZAxis.Phi() < 0.000001 &&
        m_HorizonPointingZAxis.Theta() < 0.000001) {
      cout<<"Error: Event ("<<m_Id<<") has no valid horizon pointing"<<endl;
      m_HasHorizonPointing = false;
    }
  }
  
  if (m_HasDetectorRotation == true) {
    if (m_DetectorRotationXAxis.Phi() < 0.000001 &&
        m_DetectorRotationXAxis.Theta() < 0.000001 &&
        m_DetectorRotationZAxis.Phi() < 0.000001 &&
        m_DetectorRotationZAxis.Theta() < 0.000001) {
      cout<<"Error: Event ("<<m_Id<<") has no valid detector rotation"<<endl;
      m_HasDetectorRotation = false;
    }
  }
  
  return true;
}
  

////////////////////////////////////////////////////////////////////////////////


void MRotationInterface::SetGalacticPointingXAxis(const double Longitude, const double Latitude)
{
  // Set the X axis of the LEFT-handed galactic coordinate system:
  // Left handedness is applied via y-axis

  m_HasGalacticPointing = true;
  m_IsGalacticPointingRotationCalculated = false;
  m_IsGalacticPointingInverseRotationCalculated = false;
  m_GalacticPointingXAxis.SetMagThetaPhi(1.0, (90+Latitude)*c_Rad, Longitude*c_Rad);
}


////////////////////////////////////////////////////////////////////////////////


void MRotationInterface::SetGalacticPointingZAxis(const double Longitude, const double Latitude)
{
  // Set the Z axis of the LEFT-handed galactic coordinate system:
  // Left handedness is applied via y-axis

  m_HasGalacticPointing = true;
  m_IsGalacticPointingRotationCalculated = false;
  m_IsGalacticPointingInverseRotationCalculated = false;
  m_GalacticPointingZAxis.SetMagThetaPhi(1.0, (90+Latitude)*c_Rad, Longitude*c_Rad);
}


////////////////////////////////////////////////////////////////////////////////


void MRotationInterface::SetDetectorPointingXAxis(const double Phi, const double Theta)
{
  // Set the X axis of the LEFT-handed galactic coordinate system:
  // Left handedness is applied via y-axis
  
  m_HasDetectorRotation = true;
  m_DetectorRotationXAxis.SetMagThetaPhi(1.0, Theta*c_Rad, Phi*c_Rad);
}


////////////////////////////////////////////////////////////////////////////////


void MRotationInterface::SetDetectorPointingZAxis(const double Phi, const double Theta)
{
  // Set the Z axis of the LEFT-handed galactic coordinate system:
  // Left handedness is applied via y-axis
  
  m_HasDetectorRotation = true;
  m_DetectorRotationZAxis.SetMagThetaPhi(1.0, Theta*c_Rad, Phi*c_Rad);
}


////////////////////////////////////////////////////////////////////////////////


void MRotationInterface::SetDetectorRotationXAxis(const MVector Rot)
{
  // Set the X axis of the right-handed Cartesian coordinate system:

  m_HasDetectorRotation = true;
  m_DetectorRotationXAxis = Rot;
}


////////////////////////////////////////////////////////////////////////////////


MVector MRotationInterface::GetDetectorRotationXAxis() const
{
  // Get the X axis of the right-handed Cartesian coordinate system:

  return m_DetectorRotationXAxis;
}


////////////////////////////////////////////////////////////////////////////////


void MRotationInterface::SetDetectorRotationZAxis(const MVector Rot)
{
  // Set the Z axis of the right-handed Cartesian coordinate system:

  m_HasDetectorRotation = true;
  m_DetectorRotationZAxis = Rot;
}


////////////////////////////////////////////////////////////////////////////////


MVector MRotationInterface::GetDetectorRotationZAxis() const
{
  // Get the Z axis of the right-handed Cartesian coordinate system:

  return m_DetectorRotationZAxis;
}


////////////////////////////////////////////////////////////////////////////////


MRotation MRotationInterface::GetHorizonPointingRotationMatrix() const
{
  // Return the rotation matrix of this event

  // Verify that x and z axis are at right angle:
  if (fabs(m_HorizonPointingXAxis.Angle(m_HorizonPointingZAxis) - c_Pi/2.0)*c_Deg > 0.1) {
    cout<<"Event "<<m_Id<<": Horizon axes are not at right angle, but: "<<m_HorizonPointingXAxis.Angle(m_HorizonPointingZAxis)*c_Deg<<" deg"<<endl;
  }

  // First compute the y-Axis vector:
  MVector m_HorizonPointingYAxis = m_HorizonPointingZAxis.Cross(m_HorizonPointingXAxis);

  return MRotation(m_HorizonPointingXAxis.X(), m_HorizonPointingYAxis.X(), m_HorizonPointingZAxis.X(),
                   m_HorizonPointingXAxis.Y(), m_HorizonPointingYAxis.Y(), m_HorizonPointingZAxis.Y(),
                   m_HorizonPointingXAxis.Z(), m_HorizonPointingYAxis.Z(), m_HorizonPointingZAxis.Z());
}


////////////////////////////////////////////////////////////////////////////////


MRotation MRotationInterface::GetDetectorRotationMatrix() const
{
  // Return the rotation matrix of this event

  // Verify that x and z axis are at right angle:
  if (fabs(m_DetectorRotationXAxis.Angle(m_DetectorRotationZAxis) - c_Pi/2.0)*c_Deg > 0.1) {
    cout<<"Event "<<m_Id<<": DetectorRotation axes are not at right angle, but: "<<m_DetectorRotationXAxis.Angle(m_DetectorRotationZAxis)*c_Deg<<" deg"<<endl;
  }

  // First compute the y-Axis vector:
  MVector m_DetectorRotationYAxis = m_DetectorRotationZAxis.Cross(m_DetectorRotationXAxis);

  return MRotation(m_DetectorRotationXAxis.X(), m_DetectorRotationYAxis.X(), m_DetectorRotationZAxis.X(),
                   m_DetectorRotationXAxis.Y(), m_DetectorRotationYAxis.Y(), m_DetectorRotationZAxis.Y(),
                   m_DetectorRotationXAxis.Z(), m_DetectorRotationYAxis.Z(), m_DetectorRotationZAxis.Z());
}


////////////////////////////////////////////////////////////////////////////////


MRotation MRotationInterface::GetDetectorInverseRotationMatrix() const
{
  // Return the rotation matrix of this event

  // Verify that x and z axis are at right angle:
  if (fabs(m_DetectorRotationXAxis.Angle(m_DetectorRotationZAxis) - c_Pi/2.0)*c_Deg > 0.1) {
    cout<<"Event "<<m_Id<<": DetectorRotation axes are not at right angle, but: "<<m_DetectorRotationXAxis.Angle(m_DetectorRotationZAxis)*c_Deg<<" deg"<<endl;
  }


  // First compute the y-Axis vector:
  MVector m_DetectorRotationYAxis = m_DetectorRotationZAxis.Cross(m_DetectorRotationXAxis);

  return MRotation(m_DetectorRotationXAxis.X(), m_DetectorRotationXAxis.Y(), m_DetectorRotationXAxis.Z(),
                   m_DetectorRotationYAxis.X(), m_DetectorRotationYAxis.Y(), m_DetectorRotationYAxis.Z(),
                   m_DetectorRotationZAxis.X(), m_DetectorRotationZAxis.Y(), m_DetectorRotationZAxis.Z());
}


////////////////////////////////////////////////////////////////////////////////


MRotation MRotationInterface::GetGalacticPointingRotationMatrix()
{
  // Return the rotation matrix of this event

  if (m_IsGalacticPointingRotationCalculated == true) return m_GalacticPointingRotation;
  
  // Verify that x and z axis are at right angle:
  if (fabs(m_GalacticPointingXAxis.Angle(m_GalacticPointingZAxis) - c_Pi/2.0)*c_Deg > 0.1) {
    cout<<"Event "<<m_Id<<": GalacticPointing axes are not at right angle, but: "<<m_GalacticPointingXAxis.Angle(m_GalacticPointingZAxis)*c_Deg<<" deg"<<endl;
  }

  // First compute the y-Axis vector:
  MVector m_GalacticPointingYAxis = m_GalacticPointingZAxis.Cross(m_GalacticPointingXAxis);

  // We need a minus here since the Galactic coordinate system in left-handed!!!!
  if (m_HasGalacticPointing == true) {
    m_GalacticPointingYAxis *= -1;
  }

  m_GalacticPointingRotation.Set(m_GalacticPointingXAxis.X(), m_GalacticPointingYAxis.X(), m_GalacticPointingZAxis.X(),
                                 m_GalacticPointingXAxis.Y(), m_GalacticPointingYAxis.Y(), m_GalacticPointingZAxis.Y(),
                                 m_GalacticPointingXAxis.Z(), m_GalacticPointingYAxis.Z(), m_GalacticPointingZAxis.Z());
  m_IsGalacticPointingRotationCalculated = true;
  
  return m_GalacticPointingRotation;
}


////////////////////////////////////////////////////////////////////////////////


MRotation MRotationInterface::GetGalacticPointingInverseRotationMatrix()
{
  // Return the rotation matrix of this event

  if (m_IsGalacticPointingInverseRotationCalculated == true) return m_GalacticPointingInverseRotation;

  m_GalacticPointingInverseRotation = GetGalacticPointingRotationMatrix().GetInvers();
  m_IsGalacticPointingInverseRotationCalculated = true;
    
  return m_GalacticPointingInverseRotation;
}


// MRotationInterface.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
