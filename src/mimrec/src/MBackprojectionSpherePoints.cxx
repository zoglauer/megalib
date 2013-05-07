/*
 * MBackprojectionSpherePoiunts.cxx
 *
 *
 * Copyright (C) 1999-2009 by Andreas Zoglauer.
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
// MBackprojectionSpherePoints.cxx
//
////////////////////////////////////////////////////////////////////////////////


#include "MBackprojectionSpherePoints.h"

#ifdef ___CINT___
ClassImp(MBackprojectionSpherePoints)
#endif


////////////////////////////////////////////////////////////////////////////////


MBackprojectionSpherePoints::MBackprojectionSpherePoints()
{
  // standard constructor
}


////////////////////////////////////////////////////////////////////////////////


MBackprojectionSpherePoints::~MBackprojectionSpherePoints()
{
  if (m_AreaBin != 0)
    {
      //delete [] m_AreaBin;
      //delete [] m_xBin;
      //delete [] m_yBin;
      //delete [] m_zBin;
    }
}


////////////////////////////////////////////////////////////////////////////////


void MBackprojectionSpherePoints::PrepareBackprojection()
{
  MBackprojectionSphere::PrepareBackprojection();

  int t, p;

  m_xBin = new double[m_NImageBins];
  m_yBin = new double[m_NImageBins];
  m_zBin = new double[m_NImageBins];

  for (t = 0; t < m_x1NBins; t++) {
    for (p = 0; p < m_x2NBins; p++) {
      ToCartesean(m_x1BinCenter[t], m_x2BinCenter[p], m_x3BinCenter[0], 
                  m_xBin[t+p*m_x1NBins], m_yBin[t+p*m_x1NBins], m_zBin[t+p*m_x1NBins]);
    }
  }

  m_AreaBin = new double[m_x1NBins];

  for (t = 0; t < m_x1NBins; t++)
    m_AreaBin[t] = 1.0;
  //(m_x2IntervalLength * /*2*pi*m_DistanceToObject*/
  //  fabs((cos(m_x1Min + k*m_x1IntervalLength) - cos(m_x1Min + (k+1)*m_x1IntervalLength))));
}


////////////////////////////////////////////////////////////////////////////////


bool MBackprojectionSpherePoints::Backproject(MPhysicalEvent *Event, double *Image)
{
  cout<<"MBackprojectionSpherePoints::Backproject(const MPhysicalEventData" 
    "*EventData, double *Image): "
    "No longer implemented!"<<endl;

  return false;
}


////////////////////////////////////////////////////////////////////////////////


bool MBackprojectionSpherePoints::Backproject
  (MPhysicalEvent *Event, double *Image, double& Limit, 
   int& NAboveLimit)
{
  // Take over all the necessary event data and perform some elementary computations:
  // the compton angle, the cone axis, the most probable origin of the gamma ray 
  // if possible, the center of the cone. 

  if (Assimilate(Event) == false) return false;

  if (m_EventType == 1) {
    return BackprojectionCompton(Image, Limit, NAboveLimit);
  } else if (m_EventType == 2) {
     return BackprojectionPair(Image, Limit, NAboveLimit);
  }
  cout<<"MBackprojectionSpherePoints::Backproject: Error!"<<endl;

  return false;
}


////////////////////////////////////////////////////////////////////////////////


bool MBackprojectionSpherePoints::BackprojectionCompton
  (double *Image, double& Limit, int& NAboveLimit)
{
  // Compton-Backprojection-algorithm:
  // The event expands to a double-gausshaped banana
  //
  // Image            :  the produced binned image - must have the correct dimensions
  // Limit            :  the limit 
  // NABoveLimit      :

  Limit = 0.5;
  NAboveLimit = 0;

  // Points:
  if (m_C->HasTrack() == false) return false;

  double t, p, r;
  ToSpherical(m_C->Di().X(), 
              m_C->Di().Y(), 
              m_C->Di().Z(), 
              t, p, r);

  // add it to the correct list-element
  if (p >= m_x1Min && p <= m_x1Max && t >= m_x2Min && t <= m_x2Max) {
    //cout<<"Bin: "<<(int) ((int) ((p - m_x1Min)/m_x1IntervalLength) + (int) ((t - m_x2Min)/m_x2IntervalLength) * m_x1NBins)<<": "<<t<<"!"<<p<<endl;
    //cout<<"Bin: p"<<(int) ((p - m_x1Min)/m_x1IntervalLength)<<" t: "<<(int) ((t - m_x2Min)/m_x2IntervalLength)<<endl;
    Image[(int) ((int) ((p - m_x1Min)/m_x1IntervalLength) + (int) ((t - m_x2Min)/m_x2IntervalLength) * m_x1NBins)] += 1;
  }
  else {
    return false;
  }

  NAboveLimit++;

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MBackprojectionSpherePoints::BackprojectionPair
  (double *Image, double& Limit, int& NAboveLimit)
{
  // Compton-Backprojection-algorithm:
  // The event expands to a double-gausshaped banana
  //
  // Image            :  the produced binned image - must have the correct dimensions
  // Limit            :  the limit 
  // NABoveLimit      :

  //cout<<"Now backprojecting a pair-event..."<<endl;

  // Tronsform the gamma direction to spherical coordinates:
  double t, p, r;

  ToSpherical(m_P->m_IncomingGammaDirection.X(), m_P->m_IncomingGammaDirection.Y(), m_P->m_IncomingGammaDirection.Z(), 
              t, p, r);


  // ... and add it to the appropriate bin:
  if (p >= m_x1Min && p <= m_x1Max && t >= m_x2Min && t <= m_x2Max) {
    Image[(int) ((int) ((p - m_x1Min)/m_x1IntervalLength) + (int) ((t - m_x2Min)/m_x2IntervalLength) * m_x1NBins)] += 1;
  } else {
    return false;
  }

  // I think we have a sparse matrix...
  Limit = 0.5;
  NAboveLimit = 1;
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MBackprojectionSpherePoints::Assimilate(MPhysicalEvent *Event)
{
  // Take over all the necessary event data and perform some elementary computations:
  // the compton angle, the cone axis, the most probable origin of the gamma ray, 
  // the intersection of the cone axis with the sphere. 
  //
  // If an error occures, normally because the event data is so bad that the event
  // can hardly be caused by compton effect, we return false.

  return MBackprojectionSphere::Assimilate(Event);
}


// MBackprojectionSpherePoints: the end...
////////////////////////////////////////////////////////////////////////////////
