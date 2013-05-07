/*
 * MSimDR.cxx
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
#include "MSimDR.h"

// Standard libs:
#include <sstream>
#include <iomanip>
using namespace std;

// ROOT libs:

// MEGAlib libs:


#ifdef ___CINT___
ClassImp(MSimDR)
#endif


////////////////////////////////////////////////////////////////////////////////


MSimDR::MSimDR()
{
  // standard constructor

  m_Position = g_VectorNotDefined;
  m_Direction = g_VectorNotDefined;
  m_Energy = 0;
}


////////////////////////////////////////////////////////////////////////////////


MSimDR::MSimDR(const MVector& Position, const MVector& Direction, const double Energy) : 
  m_Position(Position), m_Direction(Direction), m_Energy(Energy)
{
  // extended constructor
}


////////////////////////////////////////////////////////////////////////////////


MSimDR::~MSimDR()
{
  // standard destructor
}


////////////////////////////////////////////////////////////////////////////////


bool MSimDR::AddRawInput(MString LineBuffer, const int Version)
{
  // Analyze one line of text input...
  
  if (sscanf(LineBuffer.Data(), "DR %lf;%lf;%lf;%lf;%lf;%lf;%lf",
             &m_Position[0], &m_Position[1], &m_Position[2], 
             &m_Direction[0], &m_Direction[1], &m_Direction[2], 
             &m_Energy) != 7) {
    return false;
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


MString MSimDR::ToSimString(const int WhatToStore, const int ScientificPrecision, const int Version) const
{
  // Convert this SimEvent to the original *.sim file format...

  // If there is a significant change, make sure you make new sim version,
  // and update all reading functions

  ostringstream S;

  int WidthPos;
  int WidthDir;
  int WidthEnergy;
  int Precision;

  if (ScientificPrecision > 0) {
    WidthPos        = ScientificPrecision+7;
    WidthDir        = ScientificPrecision+7;
    WidthEnergy     = ScientificPrecision+6;
    Precision       = ScientificPrecision;
    S.setf(ios_base::scientific, ios_base::floatfield);
  } else {
    WidthPos        = 10;
    WidthDir        = 8;
    WidthEnergy     = 10;
    Precision       = 5;
    S.setf(ios_base::fixed, ios_base::floatfield);
  }

  S<<setprecision(Precision);
  S<<"DR ";
  S<<setw(WidthPos)<<m_Position.X()<<";";
  S<<setw(WidthPos)<<m_Position.Y()<<";";
  S<<setw(WidthPos)<<m_Position.Z()<<";";
  S<<setw(WidthDir)<<m_Direction.X()<<";";
  S<<setw(WidthDir)<<m_Direction.Y()<<";";
  S<<setw(WidthDir)<<m_Direction.Z()<<";";
  S<<setw(WidthEnergy)<<m_Energy<<endl;

  return S.str().c_str();
}


// MSimDR: the end...
////////////////////////////////////////////////////////////////////////////////
