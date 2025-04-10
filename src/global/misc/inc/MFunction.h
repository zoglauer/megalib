/*
 * MFunction.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MFunction__
#define __MFunction__


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


class MFunction
{
  // public interface:
 public:
  //! Default constructor
  MFunction();
  //! Copy constructor
  MFunction(const MFunction& Function);
  //! Default destructor
  virtual ~MFunction();

  //! Default assignment constructor
  const MFunction& operator=(const MFunction& F);

  //! Set the basic data, load the file and parse it
  bool Set(const MString FileName, 
           const MString KeyWord = "DP");

  //! Set the basic data from a 1D ResponseMatrix
  bool Set(const MResponseMatrixO1& Response);

  //! Set the basic data from two vectors
  bool Set(const vector<double>& X, const vector<double>& Y, unsigned int InterpolationType = c_InterpolationLinLin);

  //! Set the basic data from two vectors
  bool Set(const vector<long double>& X, const vector<long double>& Y, unsigned int InterpolationType = c_InterpolationLinLin);

  //! Add a data point
  bool Add(const long double x, const long double y);

  //! Multiple the x-axis by some value
  void ScaleX(long double Scaler);

  //! Scale the content by some value
  void ScaleY(long double Scaler);

  //! Evaluate the data for a specific x value
  long double Evaluate(long double x) const;

  //! Deprectad - Evaluate the data for a specific x value
  long double Eval(long double x) const;

  //! Integrate all the data from min to max
  long double Integrate() const;

  //! Integrate the data from min to max
  long double Integrate(long double XMin, long double XMax) const;

  //! Return a random number distributed as the underlying function
  long double GetRandom();

  //! Return a random number distributed as the underlying function times the x-value
  //! This is helpfull in case the function is a profile of a radial 2D function
  long double GetRandomTimesX();

  //! Return the size of the array
  long double GetSize() const { return m_X.size(); }

  //! Get the minimum x-value
  long double GetXMin() const;
  //! Get the maximum x-value
  long double GetXMax() const;
  //! Get the minimum y-value
  long double GetYMin() const;
  //! Get the maximum y-value
  long double GetYMax() const;

  //! Find the x value starting from Start which would be achieved after integrating until Integral
  //! If we go beyond x_max, x_max is returned if we are not cyclic, otherwise we continue at x_0
  long double FindX(long double XStart, long double Integral, bool Cyclic);
  
  //! ID representing an unknown interpolation
  static const unsigned int c_InterpolationUnknown;
  //! ID representing a constant interpolation
  static const unsigned int c_InterpolationConstant;
  //! ID representing no interpolation
  static const unsigned int c_InterpolationNone;
  //! ID representing a linear-linear interpolation (default)
  static const unsigned int c_InterpolationLinLin;
  //! ID representing a linear-logarithmic interpolation
  static const unsigned int c_InterpolationLinLog;
  //! ID representing a logarithmic-linear interpolation
  static const unsigned int c_InterpolationLogLin;
  //! ID representing a logarithmic-logarithmic interpolation
  static const unsigned int c_InterpolationLogLog;
  //! ID representing a apline interpolation (order 3)
  static const unsigned int c_InterpolationSpline3;
  //! ID representing a apline interpolation (order 5)
  static const unsigned int c_InterpolationSpline5;

  //! Plot the function in a Canvas (diagnostics only)
  void Plot();

  // Access to the stored data:
  
  //! Get the number of data points
  unsigned int GetNDataPoints() const { return m_X.size(); }
  //! Get the x value of the data point
  long double GetDataPointX(unsigned int n) const { return m_X.at(n); }
  //! Get the y value of the data point
  long double GetDataPointY(unsigned int n) const { return m_Y.at(n); }

  //! Save the data:
  bool Save(const MString FileName, const MString Keyword = "DP");


  // protected methods:
 protected:
  //! The interpolation stage of the GetRandom() function
  long double GetRandomInterpolate(long double Itot);

  //! Implementation of Lambert's W function branches 0 and -1
  long double LambertW(long double x, int Branch);

  //! Check the dynamic range and give a warning message otherwise
  void CheckDynamicRange();

  // private methods:
 private:

  // protected members:
 protected:

  // private members:
 private:
  //! Interpolation method used
  unsigned int m_InterpolationType;

  //! The x-axis data
  vector<long double> m_X;
  //! The y-axis data
  vector<long double> m_Y;

  //! True if the minimum y-value is non-negative
  bool m_YNonNegative;

  // For random number generation:
  //! The function as cumulative distribution:
  vector<long double> m_Cumulative;
  

#ifdef ___CLING___
 public:
  ClassDef(MFunction, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
