/*
 * MSpline.cxx
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
// MSpline
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MSpline.h"

// Standard libs:
#include <iostream>
#include <sstream>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MAssert.h"
#include "MStreams.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MSpline)
#endif


////////////////////////////////////////////////////////////////////////////////


MSpline::MSpline(int Mode)
{
  // 

  m_Mode = Mode;

  m_DataPoints = new TObjArray();
  m_NDataPoints = 0;
}

////////////////////////////////////////////////////////////////////////////////


MSpline::MSpline(const MSpline& S)
{
  m_DataPoints = new TObjArray(*S.m_DataPoints);
  m_NDataPoints = S.m_NDataPoints;
  m_Mode = S.m_Mode;
}


////////////////////////////////////////////////////////////////////////////////


MSpline::~MSpline()
{
  // default destructor
}


////////////////////////////////////////////////////////////////////////////////


void MSpline::AddDataPoint(double x, double y)
{
  // Add a new data point

  m_DataPoints->Expand(m_NDataPoints+1);

  int i, j;
  for (i = 0; i < m_NDataPoints; i++) {
    if (GetDataPointAt(i)->X() > x) {
      for (j = m_NDataPoints-1; j > i; j--) {
        SetDataPointAt(GetDataPointAt(j), j+1);
      }
      SetDataPointAt(new TVector2(x, y), i);
      return;
    } else if (GetDataPointAt(i)->X() == x) {
      mout<<"   ***  Warning  ***  "<<endl;
      mout<<"X-value (x="<<x<<", y="<<y<<") defined twice! Ignoring second incarnation!"<<endl;
      return;
    }
  }

  m_NDataPoints++;
  SetDataPointAt(new TVector2(x, y), m_NDataPoints-1);
}


////////////////////////////////////////////////////////////////////////////////


double MSpline::Get(double x)
{
  // 

  int i;
  int Position = -1;

  if (m_NDataPoints == 0) {
    return 0.0;
  }

  if (m_NDataPoints == 1) {
    return GetDataPointAt(0)->Y();
  }

  if (m_Mode == Interpolation) {
    // First search the position:

    if (m_NDataPoints == 0) return 0;

    for (i = 0; i < m_NDataPoints; i++) {
      if (GetDataPointAt(i)->X() > x) {
        break;
      } 
      Position = i;
    }

    double m, t;

    // Position = -1: extrapolate
    if (Position == -1) {
      m = (GetDataPointAt(0)->Y() - GetDataPointAt(1)->Y()) / 
        (GetDataPointAt(0)->X() - GetDataPointAt(1)->X());
      if (m == 0) {
        t = GetDataPointAt(0)->Y(); 
      } else {
        t = GetDataPointAt(0)->Y() - m * GetDataPointAt(0)->X();
      }
      return m*x+t;
    } 
    
    // Position = m_NDataPoints: extrapolate
    else if (Position >= m_NDataPoints-1) {
      m = (GetDataPointAt(Position)->Y() - GetDataPointAt(Position-1)->Y())/
        (GetDataPointAt(Position)->X() - GetDataPointAt(Position-1)->X());
      t = GetDataPointAt(Position)->Y() - m * GetDataPointAt(Position)->X();
      return m*x+t;
    }

    // Position = [0..m_NDataPoints-1] : interpolate
    else {
      return GetDataPointAt(Position+1)->Y() - (GetDataPointAt(Position + 1)->X() - x)/
        (GetDataPointAt(Position + 1)->X() - GetDataPointAt(Position)->X())*
        (GetDataPointAt(Position + 1)->Y() - GetDataPointAt(Position)->Y());
    }
    
    
  } else {
    Error("double MSpline::Get(double x)",
          "Ooops I did it again: Only interpolation is available yet - no splines");
  }

  return 0.0;
}


////////////////////////////////////////////////////////////////////////////////


int MSpline::GetNDataPoints()
{
  // Return the number of data points

  return m_NDataPoints;
}


////////////////////////////////////////////////////////////////////////////////


double MSpline::GetDataPointXValueAt(int i)
{
  // Return the number of data points

  massert(i >= 0 && i < m_NDataPoints);

  return ((TVector2 *) m_DataPoints->At(i))->X();
}


////////////////////////////////////////////////////////////////////////////////


double MSpline::GetDataPointYValueAt(int i)
{
  // Return the number of data points

  massert(i >= 0 && i < m_NDataPoints);

  return ((TVector2 *) m_DataPoints->At(i))->Y();
}


////////////////////////////////////////////////////////////////////////////////


TVector2* MSpline::GetDataPointAt(int i)
{
  // return the TVector2 at position i in the list. Counting starts with zero!

  if (i >= 0 && i < m_NDataPoints) {
    return (TVector2 *) m_DataPoints->At(i);
  } else {
    Fatal("TVector2* MSpline::GetDataPointAt(int i)",
          "Index (%d) out of bounds (%d, %d)", i, 0, m_NDataPoints);
    // this error shouldn't cause the program to crash, so we return :
    return 0;
  }
}


////////////////////////////////////////////////////////////////////////////////


void MSpline::SetDataPointAt(TVector2* DataPoint, int i)
{
  // return the TVector2 at position i in the list. Counting starts with zero!

  if (i >= 0 && i < m_NDataPoints) {
    m_DataPoints->AddAt(DataPoint, i);
  } else {
    Fatal("TVector2* MSpline::SetDataPointAt(TVector2* DataPoint, int i)",
          "Index (%d) out of bounds (%d, %d)", i, 0, m_NDataPoints);
  }
}


////////////////////////////////////////////////////////////////////////////////


MString MSpline::ToString()
{
  //

  ostringstream out;
  out<<"TSpline Data-Points: "<<endl;
  for (int i = 0; i < m_NDataPoints; i++) {
    out<<GetDataPointAt(i)->X()<<": "<<GetDataPointAt(i)->Y()<<endl;
  }

  return out.str().c_str();
}


// MSpline.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
