/*
 * MFunctionTime.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MFunctionTime__
#define __MFunctionTime__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <TROOT.h>
#include <TSpline.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MParser.h"
#include "MResponseMatrixO1.h"

// Standard libs:
#include <vector>
using namespace std;

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MFunctionTime
{
  // public interface:
 public:
  //! Default constructor
  MFunctionTime();
  //! Copy constructor
  MFunctionTime(const MFunctionTime& Function);
  //! Default destructor
  virtual ~MFunctionTime();

  //! Default assignment constructor
  const MFunctionTime& operator=(const MFunctionTime& F);

  //! Set the basic data, load the file and parse it
  bool Set(const MString FileName, const MString KeyWord = "DP");

  //! Set the basic data from two vector
  bool Set(const vector<MTime>& X, const vector<double>& Y, unsigned int InterpolationType = c_InterpolationLinLin);

  //! Add a data point
  bool Add(const MTime& x, const double y);

  //! Multiple the x-axis by some value
  void ScaleX(double Scaler);

  //! Scale the content by some value
  void ScaleY(double Scaler);

  //! Evaluate the data for a specific x value 
  double Evaluate(const MTime& x) const;

  //! Integrate all the data from min to max
  double Integrate() const;

  //! Integrate the data from min to max
  double Integrate(MTime XMin, MTime XMax) const;

  //! Return a random number distributed as the underlying function
  double GetRandom();

  //! Return a random number distributed as the underlying function times the x-value
  //! This is helpfull in case the function is a profile of a radial 2D function
  double GetRandomTimesX();

  //! Return the size of the array
  double GetSize() const { return m_X.size(); } 

  //! Get the minimum x-value
  MTime GetXMin() const;
  //! Get the maximum x-value
  MTime GetXMax() const;
  //! Get the minimum y-value
  double GetYMin() const;
  //! Get the maximum y-value
  double GetYMax() const;

  //! Find the x value starting from Start which would be achieved after integrating until Integral
  //! If we go beyond x_max, x_max is returned if we are not cyclic, otherwise we continue at x_0
  MTime FindX(const MTime& XStart, double Integral, bool Cyclic);
  
  //! ID representing an unknown interpolation
  static const unsigned int c_InterpolationUnknown;
  //! ID representing a linear-linear interpolation (default)
  static const unsigned int c_InterpolationLinLin;

  //! Plot the function in a Canvas (diagnostics only)
  void Plot();

  // Access to the stored data:
  
  //! Get the number of data points
  unsigned int GetNDataPoints() const { return m_X.size(); }
  //! Get the x value of the data point
  MTime GetDataPointX(unsigned int n) const { return m_X.at(n); }
  //! Get the y value of the data point
  double GetDataPointY(unsigned int n) const { return m_Y.at(n); }

  //! Save the data:
  bool Save(const MString FileName, const MString Keyword = "DP");


  // protected methods:
 protected:
  //! The interpolation stage of the GetRandom() function
  double GetRandomInterpolate(double Itot);

  // private methods:
 private:

  // protected members:
 protected:

  // private members:
 private:
  //! Interpolation method used
  unsigned int m_InterpolationType;

  //! The x-axis data
  vector<MTime> m_X;
  //! The y-axis data
  vector<double> m_Y;

  //! True if the minimum y-value is non-negative
  bool m_YNonNegative;

  // For random number generation:
  //! The function as cumulative distribution:
  vector<double> m_Cumulative;
  

#ifdef ___CLING___
 public:
  ClassDef(MFunctionTime, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
