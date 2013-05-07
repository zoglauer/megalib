/*
 * MSimPM.cxx
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
#include "MSimPM.h"

// Standard libs:
#include <sstream>
#include <iomanip>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MStreams.h"

#ifdef ___CINT___
ClassImp(MSimPM)
#endif


////////////////////////////////////////////////////////////////////////////////


MSimPM::MSimPM()
{
  // standard constructor

  m_MaterialName = "Unknown";
  m_Energy = 0;
}


////////////////////////////////////////////////////////////////////////////////


MSimPM::MSimPM(const MString MaterialName, const double Energy) : 
  m_MaterialName(MaterialName), m_Energy(Energy)
{
  // extended constructor
}


////////////////////////////////////////////////////////////////////////////////


MSimPM::~MSimPM()
{
  // standard destructor
}


////////////////////////////////////////////////////////////////////////////////


bool MSimPM::AddRawInput(MString LineBuffer, const int Version)
{
  // Analyze one line of text input...
  
  char Name[1024];
  if (sscanf(LineBuffer.Data(), "PM %s %lf\n", Name, &m_Energy) != 2) {
    mout<<"Error during scanning of sim file in token PM:"<<endl;
    mout<<"  "<<LineBuffer<<endl;
    return false;
  }

  m_MaterialName = Name;

  return true;
}


////////////////////////////////////////////////////////////////////////////////


MString MSimPM::ToSimString(const int WhatToStore, const int ScientificPrecision, const int Version) const
{
  // Convert this SimEvent to the original *.sim file format...

  // If there is a significant change, make sure you make new sim version,
  // and update all reading functions

  ostringstream S;

  int WidthEnergy;
  int Precision;
  if (ScientificPrecision > 0) {
    WidthEnergy = ScientificPrecision+6;
    Precision = ScientificPrecision;
    S.setf(ios_base::scientific, ios_base::floatfield);
  } else {
    WidthEnergy = 10;
    Precision = 5;
    S.setf(ios_base::fixed, ios_base::floatfield);
  }

  S<<"PM "<<m_MaterialName<<" ";
  S<<setprecision(Precision);
  S<<setw(WidthEnergy)<<m_Energy<<endl;

  return S.str().c_str();
}


// MSimPM: the end...
////////////////////////////////////////////////////////////////////////////////
