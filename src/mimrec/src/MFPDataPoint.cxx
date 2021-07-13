/*
 * MFPDataPoint.cxx
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
// MFPDataPoint.cxx
//
////////////////////////////////////////////////////////////////////////////////


#include "MFPDataPoint.h"

#ifdef ___CLING___
ClassImp(MFPDataPoint)
#endif


////////////////////////////////////////////////////////////////////////////////


MFPDataPoint::MFPDataPoint()
{
  // standard constructor
}


////////////////////////////////////////////////////////////////////////////////


MFPDataPoint::MFPDataPoint(double Psi, double Eta, 
         double Phi, double Theta)
{
  m_Psi = Psi;
  m_Eta = Eta;
  m_Phi = Phi;
  m_Theta = Theta;
}


////////////////////////////////////////////////////////////////////////////////


MFPDataPoint::~MFPDataPoint()
{
  // standard destructor
}


// MFPDataPoint: the end...
////////////////////////////////////////////////////////////////////////////////
