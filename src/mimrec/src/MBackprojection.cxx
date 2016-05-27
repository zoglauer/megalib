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


#ifdef ___CINT___
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

  m_Response = 0;
  m_Geometry = 0;
  m_UseAbsorptions = false;
}


////////////////////////////////////////////////////////////////////////////////


MBackprojection::~MBackprojection()
{
  // standard destructor

  delete m_Response;
}


////////////////////////////////////////////////////////////////////////////////


void MBackprojection::SetViewportDimensions(double x1Min, double x1Max, int x1NBins, 
                                            double x2Min, double x2Max, int x2NBins,
                                            double x3Min, double x3Max, int x3NBins,
                                            MVector xAxis, MVector zAxis)
{
  // Set the dimensions of the viewport (minimum and maximum x and y-values, 
  // number of bins)

  m_x1Min = x1Min;
  m_x1Max = x1Max;
  m_x1NBins = x1NBins;
  m_x2Min = x2Min;
  m_x2Max = x2Max;
  m_x2NBins = x2NBins;
  m_x3Min = x3Min;
  m_x3Max = x3Max;
  m_x3NBins = x3NBins;

  m_x1IntervalLength = (m_x1Max - m_x1Min)/m_x1NBins;
  m_x2IntervalLength = (m_x2Max - m_x2Min)/m_x2NBins;
  m_x3IntervalLength = (m_x3Max - m_x3Min)/m_x3NBins;

  //cout<<"x:"<<m_x1Min*grad<<"!"<<m_x1Max*grad<<"!"<<m_x1NBins<<endl;
  //cout<<"y:"<<m_x2Min*grad<<"!"<<m_x2Max*grad<<"!"<<m_x2NBins<<endl;
  //cout<<"ILengths: "<<m_x1IntervalLength*grad<<"!"<<m_x2IntervalLength*grad<<"!"<<m_x1Min*grad<<endl;

  m_NImageBins = m_x1NBins*m_x2NBins*m_x3NBins;

  return;
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



// MBackprojection: the end...
////////////////////////////////////////////////////////////////////////////////
