/*
 * MBackprojectionCart3DArea.cxx
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
// MBackprojectionCart3DArea
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MBackprojectionCart3DArea.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MBackprojectionCart3DArea)
#endif


////////////////////////////////////////////////////////////////////////////////


MBackprojectionCart3DArea::MBackprojectionCart3DArea()
{
  // default constructor
}


////////////////////////////////////////////////////////////////////////////////


MBackprojectionCart3DArea::~MBackprojectionCart3DArea()
{
  // default destructor
}




////////////////////////////////////////////////////////////////////////////////


void MBackprojectionCart3DArea::PrepareBackprojection()
{
  MBackprojectionCart2DArea::PrepareBackprojection();
}


////////////////////////////////////////////////////////////////////////////////


bool MBackprojectionCart3DArea::Backproject(MPhysicalEvent* Event, 
                                              double *Image, 
                                              double& Limit, 
                                              int& NAboveLimit)
{
  // Take over all the necessary event data and perform some elementary computations:
  // the compton angle, the cone axis, the most probable origin of the gamma ray 
  // if possible, the center of the cone.

  if (MBackprojectionCart3DArea::Assimilate(Event) == false) return false;

  if (m_EventType == 1) {
    return BackprojectionCompton(Image, Limit, NAboveLimit);
  } else if (m_EventType == 5) {
    Error("MBackprojectionSphereArea::Backproject(...)",
          "Only Compton-events allowed!");
  }

  return false;
}


////////////////////////////////////////////////////////////////////////////////


bool MBackprojectionCart3DArea::BackprojectionCompton(double* Image, 
                                                        double& Limit, 
                                                        int& NAboveLimit)
{
  // We only need to call the base function here:
 
  if (MBackprojectionCart2DArea::BackprojectionCompton(Image, Limit, NAboveLimit) == false) {
    Limit = 0;
    NAboveLimit = 0;
    return false;
  }
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MBackprojectionCart3DArea::Assimilate(MPhysicalEvent *Event)
{
  //

  return MBackprojectionCart2DArea::Assimilate(Event);
}


// MBackprojectionCart3DArea.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
