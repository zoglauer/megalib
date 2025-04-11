/*
 * MBackprojection.cxx
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
// MBackprojection.cxx
//
//
// This is the base class for all other Backprojection classes, and provides 
// some elemetary data:
// * viewport dimension and bins
// * distance to the reconstruction plane/sphere
// * fitparameter
// * ... more to come 
//
// Make sure to call the according Set...-methods before starting the 
// reconstruction - due to performance reasons, the correct setting is NOT,
// not tested!
//
// This is a pure virtual class, i.e. you not instantiate it directly!
//
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MBackprojection.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MAssert.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MBackprojection)
#endif


////////////////////////////////////////////////////////////////////////////////


MBackprojection::MBackprojection(MCoordinateSystem CoordinateSystem) : MProjection(CoordinateSystem)
{
  // Initialize one MBackprojection-object. This is the base class for all other
  // Backprojection classes, and provides some elemetary data:
  // * viewport dimension and bins
  // * distance to the reconstruction plane/sphere
  // * fitparameter
  // * ... more to come 
  //
  // Make sure to call the according Set...-methods before starting the 
  // reconstruction - due to performance reasons, the correct setting is NOT,
  // not tested!

  m_Response = nullptr;
  m_Efficiency = nullptr;
  m_Geometry = nullptr;
  m_UseAbsorptions = false;
}


////////////////////////////////////////////////////////////////////////////////


MBackprojection::~MBackprojection()
{
  // standard destructor

  delete m_Response;
  // geometry and efficiency do not get deleted...
}


////////////////////////////////////////////////////////////////////////////////


void MBackprojection::PrepareBackprojection()
{
  //! Prepare all backprojections - must be called before the backprojections

  if (m_Geometry == 0) m_UseAbsorptions = false;
}

 
////////////////////////////////////////////////////////////////////////////////


void MBackprojection::Rotate(double &x, double &y, double &z)
{
  // Rotate the reconstruction-coodinate system

  MVector P(x, y, z);
  // Apply the detector rotation of the individual event
  if (m_Event->HasDetectorRotation() == true) {
    P = m_Event->GetDetectorRotationMatrix() * P;
  }
  // Apply the galactic pointing rotation to the event if we have galactic coordinates
  if (m_Event->HasGalacticPointing() == true && m_CoordinateSystem == MCoordinateSystem::c_Galactic) {
    P = m_Event->GetGalacticPointingRotationMatrix() * P;
  }
    
  x = P.X();
  y = P.Y();
  z = P.Z();
}


////////////////////////////////////////////////////////////////////////////////


bool MBackprojection::Assimilate(MPhysicalEvent* Event)
{
  // Now we have to check which kind of data we want to backproject:
  // Compton or Pair-data:

  if (MProjection::Assimilate(Event) == false) return false;

  // Let the response get the event data:
  if (Event->GetType() == MPhysicalEvent::c_Compton || 
      Event->GetType() == MPhysicalEvent::c_Pair) {
    if (m_Response->AnalyzeEvent(Event) == false) return false;
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MBackprojection::ToSpherical(double x, double y, double z,
                                  double &t, double &p, double &r)
{
  // Transfer Cartesian Coordinates to Spherical

  // Now transform:
  r =  sqrt(x*x + y*y + z*z);

  if (x == 0.0 && y == 0.0) {
    p = 0.0;
  } else {
    if (x != 0) {
      p = atan2(y, x);
    } else {
      if (y > 0) {
        p = c_Pi/2;
      } else {
        p = -c_Pi/2;
      }
    }
  }

  if (x == 0.0 && y == 0.0 && z == 0.0) {
    t = 0.0;
  } else {
    if (z != 0) {
      t = atan2(sqrt(x*x + y*y), z);
    } else {
      if (sqrt(x*x + y*y) > 0) {
        t = c_Pi/2;
      } else {
        t = -c_Pi/2;
      }
    }
  }

  return;
}


////////////////////////////////////////////////////////////////////////////////


void MBackprojection::ToCartesean(double t, double p, double r,
                                  double &x, double &y, double &z)
{
  // Transfer Spherical Coordinates to Cartesean

  x = r * sin(t) * cos(p);
  y = r * sin(t) * sin(p);
  z = r * cos(t);
}

// MBackprojection: the end...
////////////////////////////////////////////////////////////////////////////////
