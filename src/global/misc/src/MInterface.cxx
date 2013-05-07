/*
 * MInterface.cxx
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
// MInterface
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MInterface.h"

// Standard libs:
#include <iostream>
using namespace std;

// ROOT libs:
#include "TMath.h"

// MEGAlib libs:
#include "MAssert.h"
#include "MSettings.h"
#include "MGUIMain.h"
#include "MDGeometryQuest.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MInterface)
#endif


////////////////////////////////////////////////////////////////////////////////


MInterface::MInterface() : m_Geometry(0), m_UseGui(true), m_Gui(0), m_BasicGuiData(0)
{
  // default constructor
}


////////////////////////////////////////////////////////////////////////////////


MInterface::~MInterface()
{
  // default destructor

  delete m_Geometry;
}


////////////////////////////////////////////////////////////////////////////////


MDGeometryQuest* MInterface::GetGeometry()
{
  // Return a pointer to the geometry
  // If there is no geometry, the pointer is zero

  return m_Geometry;
}


////////////////////////////////////////////////////////////////////////////////


bool MInterface::DetermineAxis(double& xMin, double& xMax, 
                               double& yMin, double& yMax, 
                               double& zMin, double& zMax, 
                               const vector<MVector>& Positions)
{
  // Calculate optimum axes given a set of vectors, which include all positions
  // Return values are the axis-dimensions

  if (Positions.size() == 0) {
    mout<<"Error: Unable to determine optimized axes since no positions are given!"<<endl;
    return false;
  }
  
  // Test dimension of image:
  xMin = numeric_limits<double>::max();
  yMin = numeric_limits<double>::max();
  zMin = numeric_limits<double>::max();

  xMax = -numeric_limits<double>::max();
  yMax = -numeric_limits<double>::max();
  zMax = -numeric_limits<double>::max();
  
  for (unsigned int p = 0; p < Positions.size(); ++p) {
    if (Positions[p][0] > xMax) xMax = Positions[p][0];
    if (Positions[p][1] > yMax) yMax = Positions[p][1];
    if (Positions[p][2] > zMax) zMax = Positions[p][2];
    if (Positions[p][0] < xMin) xMin = Positions[p][0];
    if (Positions[p][1] < yMin) yMin = Positions[p][1];
    if (Positions[p][2] < zMin) zMin = Positions[p][2];
  }

  mdebug<<"Dimensions before optimization: "<<endl;
  mdebug<<"x:"<<xMin<<" - "<<xMax<<endl;
  mdebug<<"y:"<<yMin<<" - "<<yMax<<endl;
  mdebug<<"z:"<<zMin<<" - "<<zMax<<endl;


  // Add some space left and right...
  double Diff = 0.1*(xMax-xMin);
  double MaxDiff = Diff;
  xMax += Diff;
  xMin -= Diff;
  Diff = 0.1*(yMax-yMin);
  if (Diff > MaxDiff) MaxDiff = Diff;
  yMax += Diff;
  yMin -= Diff;
  Diff = 0.1*(zMax-zMin);
  if (Diff > MaxDiff) MaxDiff = Diff;
  zMax += Diff;
  zMin -= Diff;


  // Protect against identical:
  double MinDiff = 0.00001;
  if (xMin == xMax) {
    xMin -= MinDiff;
    xMax += MinDiff;
  }
  if (yMin == yMax) {
    yMin -= MinDiff;
    yMax += MinDiff;
  }
  if (zMin == zMax) {
    zMin -= MinDiff;
    zMax += MinDiff;
  }

  // make sure we get small numbers:
  int Scaler = 1;
  while (MaxDiff < 1) {
    MaxDiff *=10;
    Scaler *=10;
  }

  xMin = TMath::Floor(xMin*Scaler)/Scaler;
  xMax = TMath::Ceil(xMax*Scaler)/Scaler;
  yMin = TMath::Floor(yMin*Scaler)/Scaler;
  yMax = TMath::Ceil(yMax*Scaler)/Scaler;
  zMin = TMath::Floor(zMin*Scaler)/Scaler;
  zMax = TMath::Ceil(zMax*Scaler)/Scaler;

  // Symmetrize
  if (xMin < 0 && xMax > 0) {
    if (fabs(fabs(xMin) - fabs(xMax)) < 0.5*(0.5*(fabs(xMin) + fabs(xMax)))) {
      if (fabs(xMax) > fabs(xMin)) {
        xMin = -xMax;
      } else {
        xMax = -xMin;
      }
    }
  }
  if (yMin < 0 && yMax > 0) {
    if (fabs(fabs(yMin) - fabs(yMax)) < 0.5*(0.5*(fabs(yMin) + fabs(yMax)))) {
      if (fabs(yMax) > fabs(yMin)) {
        yMin = -yMax;
      } else {
        yMax = -yMin;
      }
    }
  }
  if (zMin < 0 && zMax > 0) {
    if (fabs(fabs(zMin) - fabs(zMax)) < 0.5*(0.5*(fabs(zMin) + fabs(zMax)))) {
      if (fabs(zMax) > fabs(zMin)) {
        zMin = -zMax;
      } else {
        zMax = -zMin;
      }
    }
  }


  mdebug<<endl;
  mdebug<<"Setting dimensions of geometry to:"<<endl;
  mdebug<<"  x: "<<xMin<<" - "<<xMax<<endl;
  mdebug<<"  y: "<<yMin<<" - "<<yMax<<endl;
  mdebug<<"  z: "<<zMin<<" - "<<zMax<<endl;
  mdebug<<endl;
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


double* MInterface::CreateAxisBins(double Min, double Max, int NBins, bool IsLog)
{
  //! Return an array usable for ROOT histograms

  if (IsLog == true) {
    if (Min <= 0) Min = 1E-9;
  }
  if (NBins < 1) {
    merr<<"Number of bins ("<<NBins<<") must be >= 1! Using 1!"<<show;
    NBins = 2;
  }
  if (Min >= Max) {
    merr<<"Minimum ("<<Min<<") must be smaller than maximum ("<<Max<<")! Using Max = Min + 1.0!"<<show;
    Max = Min+1;
  }

  double* Bins = new double[NBins+1];
  double Dist = 0.0;

  if (IsLog == true) {
    Min = log(Min);
    Max = log(Max);
    Dist = (Max-Min)/(NBins);
    for (int i = 0; i < NBins+1; ++i) {
      Bins[i] = exp(Min+i*Dist);
    }  
  } else {
    Dist = (Max-Min)/(NBins);
    for (int i = 0; i < NBins+1; ++i) {
      Bins[i] = Min+i*Dist;
    }  
  }

  return Bins;
}



////////////////////////////////////////////////////////////////////////////////

  
double MInterface::GetFWHM(TF1* Function, double Min, double Max) 
{
  // Return the FWHM of the function Function within the limits of Min, Max
  // if an error occurs, return a negative number
  // if Min=Max, then ignore these values

  double x, xmin, xmax;
  double xHighest = 0;
  double yHighest = 0;
  double FWHMmin = 0, FWHMmax = 0;

  if (Min == Max) {
    Min = -1000;
    Max = +1000;
  }

  int NSteps = 100;
  double xStep = (Max-Min)/NSteps;
  // Step One: determine the maximum:
  // Stage a:
  xmin = Min;
  xmax = Max;
  yHighest = DBL_MIN;
  for (x = xmin; x <= xmax; x += xStep) {
    if (Function->Eval(x) > yHighest) {
      yHighest = Function->Eval(x);
      xHighest = x;
    }
  }
  
  // Stage b:
  xmin = xHighest - xStep;
  xmax = xHighest + xStep;
  xStep /= NSteps;
  for (x = xmin; x <= xmax; x += xStep) {
    if (Function->Eval(x) > yHighest) {
      yHighest = Function->Eval(x);
      xHighest = x;
    }
  }
  
  //cout<<"Found maximum: x="<<xHighest<<", y="<<yHighest<<endl;


  // Step Two: determine left border
  // NSteps /= 2;
  xStep = (xHighest - Min)/NSteps;
  // Step One: determine the maximum:
  // Stage a:
  xmin = Min;
  xmax = xHighest;
  for (x = xmax; x >= xmin; x -= xStep) {
    if (Function->Eval(x) < 0.5*yHighest) {
      // Stage b:
      xmin = x;
      xmax = x + xStep;
      xStep /= NSteps;
      FWHMmin = 0.5*(xmax+xmin);
      for (x = xmin; x < xmax; x += xStep) {
        if (Function->Eval(x) >= 0.5*yHighest) {
          FWHMmin = x;
          break;
        }
      }
      break;
    }
  }

  //cout<<"Found left border: x="<<FWHMmin<<endl;


  // Step three: determine right border
  xStep = (Max - xHighest)/NSteps;
  // Step One: determine the maximum:
  // Stage a:
  xmin = xHighest;
  xmax = Max;
  for (x = xmin; x <= xmax; x += xStep) {
    if (Function->Eval(x) < 0.5*yHighest) {
      // Stage b:
      xmax = x;
      xmin = x - xStep;
      xStep /= NSteps;
      FWHMmax = 0.5*(xmax+xmin);
      for (x = xmax; x > xmin; x -= xStep) {
        if (Function->Eval(x) >= 0.5*yHighest) {
          FWHMmax = x;
          break;
        }
      }
      break;
    }
  }

  //cout<<"Found left border: x="<<FWHMmax<<endl;

  return FWHMmax - FWHMmin;
}


// MInterface.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
