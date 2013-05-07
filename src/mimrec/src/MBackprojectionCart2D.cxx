/*
 * MBackprojectionCart2D.cxx                                   v0.1  01/01/2001
 *
 *
 * Copyright (C) 1998-2009 by Andreas Zoglauer.
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
// MBackprojectionCart2D
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MBackprojectionCart2D.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MBackprojectionCart2D)
#endif


////////////////////////////////////////////////////////////////////////////////


MBackprojectionCart2D::MBackprojectionCart2D()
{
  // default constructor
}


////////////////////////////////////////////////////////////////////////////////


MBackprojectionCart2D::~MBackprojectionCart2D()
{
  // default destructor
}


////////////////////////////////////////////////////////////////////////////////


void MBackprojectionCart2D::PrepareBackprojection()
{
  

}


////////////////////////////////////////////////////////////////////////////////


bool MBackprojectionCart2D::Backproject(MPhysicalEvent* Event, double *Image, 
                                          double &Limit, double &NAboveLimit)
{


  return false;
}

////////////////////////////////////////////////////////////////////////////////


bool MBackprojectionCart2D::Assimilate(MPhysicalEvent *Event)
{
  // Take over all the necessary event data and perform some elementary computations:
  // the compton angle, the cone axis, the most probable origin of the gamma ray, 
  // and here: the intersection of the cone axis with the sphere. 
  //
  // If an error occures, normally because the event data is so bad that the event
  // can hardly be caused by compton effect, we return false.

  if (MBackprojection::Assimilate(Event) == false) {
    return false;
  }
  
  if (m_EventType == 1) {             
    return true;
  }
  else if (m_EventType == 2) {
    return true;
  }  
  else if (m_EventType == 5) {
    return false;
  }  

  return false;
}


////////////////////////////////////////////////////////////////////////////////


double MBackprojectionCart2D::DistanceConecirclePoint(double x, double y, double z)
{
  // The searched Distance is NOT (N-O-T) the shortest distance between the point and
  // the conesection in the reconstructed plane, but the the angle between the CONE and the point.

  return 0.0;
}


////////////////////////////////////////////////////////////////////////////////


void MBackprojectionCart2D::SetViewportDimensions(double x1Min, double x1Max, int x1NBins, 
                                                  double x2Min, double x2Max, int x2NBins, 
                                                  double x3Min, double x3Max, int x3NBins,
                                                  MVector xAxis, MVector zAxis)
{
  // Set the dimensions of the viewport

  MBackprojection::SetViewportDimensions(x1Min, x1Max, x1NBins, 
                                         x2Min, x2Max, x2NBins,
                                         x3Min, x3Max, x3NBins);

  return;
}

// MBackprojectionCart2D.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
