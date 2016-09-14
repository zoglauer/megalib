/*
 * MViewPort.cxx
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
#include "MViewPort.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MViewPort)
#endif


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MViewPort::MViewPort()
{
  m_x1Min = 0;
  m_x1Max = 1;
  m_x1NBins = 1;
  m_x2Min = 0;
  m_x2Max = 1;
  m_x2NBins = 1;
  m_x3Min = 0;
  m_x3Max = 1;
  m_x3NBins = 1;

  m_x1IntervalLength = 1;
  m_x2IntervalLength = 1;
  m_x3IntervalLength = 1;
  
  m_NImageBins = 1;
  
  m_x1BinCenter = new double[m_x1NBins]; 
  m_x1BinCenter[0] = 0.5;
  m_x2BinCenter = new double[m_x2NBins]; 
  m_x2BinCenter[0] = 0.5;
  m_x3BinCenter = new double[m_x3NBins]; 
  m_x3BinCenter[0] = 0.5;
   
  m_XAxis.SetXYZ(1.0, 0.0, 0.0);
  m_ZAxis.SetXYZ(0.0, 0.0, 1.0);

  m_Rotation.ResizeTo(3,3);
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
MViewPort::~MViewPort()
{
}


////////////////////////////////////////////////////////////////////////////////


bool MViewPort::SetDimensions(double x1Min, double x1Max, unsigned int x1NBins, 
                              double x2Min, double x2Max, unsigned  int x2NBins,
                              double x3Min, double x3Max, unsigned int x3NBins,
                              MVector xAxis, MVector zAxis)
{
  // Set the dimensions of the viewport (minimum and maximum x and y-values, 
  // number of bins)

  if (x1NBins == 0 || x2NBins == 0 || x3NBins == 0) {
    cout<<"Error - Viewport: You need at least 1 bin for all dimensions"<<endl;
    return false;
  }
  
  if (x1Max <= x1Min) {
    cout<<"Error - Viewport: dimension x1 (x, phi): The minimum must be smaller than the maximum"<<endl;
    return false;     
  }
  if (x2Max <= x2Min) {
    cout<<"Error - Viewport: dimension x2 (y, theta): The minimum must be smaller than the maximum"<<endl;
    return false;     
  }
  if (x3Max <= x3Min) {
    cout<<"Error - Viewport: dimension x3 (z, radius): The minimum must be smaller than the maximum"<<endl;
    return false;     
  }
  
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

  // Now compute for all bins their center:
  delete [] m_x1BinCenter;
  m_x1BinCenter = new double[m_x1NBins];
  for (unsigned int k = 0; k < m_x1NBins; ++k) {
    m_x1BinCenter[k] = m_x1Min + (0.5+k)*m_x1IntervalLength;
  }

  delete [] m_x2BinCenter;
  m_x2BinCenter = new double[m_x2NBins];
  for (unsigned int k = 0; k < m_x2NBins; ++k) {
    m_x2BinCenter[k] = m_x2Min + (0.5+k)*m_x2IntervalLength;
  }

  delete [] m_x3BinCenter;
  m_x3BinCenter = new double[m_x3NBins];
  for (unsigned int k = 0; k < m_x3NBins; ++k) {
    m_x3BinCenter[k] = m_x3Min + (0.5+k)*m_x3IntervalLength;
  }

  //m_x3BinCenter = 0.5*(m_x3Min + m_x3Max);
  
  //cout<<"x:"<<m_x1Min*grad<<"!"<<m_x1Max*grad<<"!"<<m_x1NBins<<endl;
  //cout<<"y:"<<m_x2Min*grad<<"!"<<m_x2Max*grad<<"!"<<m_x2NBins<<endl;
  //cout<<"ILengths: "<<m_x1IntervalLength*grad<<"!"<<m_x2IntervalLength*grad<<"!"<<m_x1Min*grad<<endl;

  m_NImageBins = m_x1NBins*m_x2NBins*m_x3NBins;

  
    // Create Rotation:
  m_XAxis = xAxis;
  m_ZAxis = zAxis;


  // First compute the y-Axis vector:
  MVector yAxis = zAxis.Cross(xAxis);
  
  //cout<<"RotVector x: "<<m_RotationXAxis.X()<<"!"<<m_RotationXAxis.Y()<<"!"<<m_RotationXAxis.Z()<<endl;
  //cout<<"RotVector y: "<<m_RotationYAxis.X()<<"!"<<m_RotationYAxis.Y()<<"!"<<m_RotationYAxis.Z()<<endl;
  //cout<<"RotVector z: "<<m_RotationZAxis.X()<<"!"<<m_RotationZAxis.Y()<<"!"<<m_RotationZAxis.Z()<<endl;

  m_Rotation(0,0) = xAxis.X();
  m_Rotation(1,0) = xAxis.Y();
  m_Rotation(2,0) = xAxis.Z();
  m_Rotation(0,1) = yAxis.X();
  m_Rotation(1,1) = yAxis.Y();
  m_Rotation(2,1) = yAxis.Z();
  m_Rotation(0,2) = zAxis.X();
  m_Rotation(1,2) = zAxis.Y();
  m_Rotation(2,2) = zAxis.Z();
  
  return true;
}


// MViewPort.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
