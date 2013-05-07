/*
 * MSimGR.cxx
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
#include "MSimGR.h"

// Standard libs:
#include <sstream>
#include <iomanip>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MDDetector.h"

#ifdef ___CINT___
ClassImp(MSimGR)
#endif


////////////////////////////////////////////////////////////////////////////////


MSimGR::MSimGR()
{
  // standard constructor

  m_DetectorType = MDDetector::c_NoDetectorType;
  m_Position = g_VectorNotDefined;
  m_Energy = 0;
}


////////////////////////////////////////////////////////////////////////////////


MSimGR::MSimGR(const int DetectorType, const MVector& Position, const double Energy) : 
  m_DetectorType(DetectorType), m_Position(Position), m_Energy(Energy)
{
  // extended constructor
}


////////////////////////////////////////////////////////////////////////////////


MSimGR::~MSimGR()
{
  // standard destructor
}


////////////////////////////////////////////////////////////////////////////////


bool MSimGR::AddRawInput(MString LineBuffer, const int Version)
{
  // Analyze one line of text input...
  
  if (sscanf(LineBuffer.Data(), "GR %lf;%lf;%lf;%lf",
             &m_Position[0], &m_Position[1], &m_Position[2],
             &m_Energy) != 4) {
    if (sscanf(LineBuffer.Data(), "GRsim %d;%lf;%lf;%lf;%lf",
               &m_DetectorType,
               &m_Position[0], &m_Position[1], &m_Position[2],
               &m_Energy) != 5) {
      return false;
    }
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


MString MSimGR::ToSimString(const int WhatToStore, const int ScientificPrecision, const int Version) const
{
  // Convert this SimEvent to the original *.sim file format...

  // If there is a significant change, make sure you make new sim version,
  // and update all reading functions

  ostringstream S;

  int WidthPos;
  int WidthEnergy;
  //int WidthTime;
  int Precision;
  if (ScientificPrecision > 0) {
    WidthPos = ScientificPrecision+7;
    WidthEnergy = ScientificPrecision+6;
    //WidthTime = ScientificPrecision+6;
    Precision = ScientificPrecision;
    S.setf(ios_base::scientific, ios_base::floatfield);
  } else {
    WidthPos = 10;
    WidthEnergy = 10;
    //WidthTime = 11;
    Precision = 5;
    S.setf(ios_base::fixed, ios_base::floatfield);
  }

  S<<"GRsim "<<m_DetectorType<<";";
  S<<setprecision(Precision);
  S<<setw(WidthPos)<<m_Position[0]<<";";
  S<<setw(WidthPos)<<m_Position[1]<<";";
  S<<setw(WidthPos)<<m_Position[2]<<";";
  S<<setw(WidthEnergy)<<m_Energy;
  S<<endl;

  return S.str().c_str();
}


// MSimGR: the end...
////////////////////////////////////////////////////////////////////////////////
