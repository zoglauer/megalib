/*
 * MFunction2D.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MFunction2D__
#define __MFunction2D__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <TROOT.h>
#include <TSpline.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MParser.h"

// Standard libs:
#include <vector>
using std::vector;

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MFunction2D
{
  // public interface:
 public:
  //! Default constructor
  MFunction2D();
  //! Copy constructor
  MFunction2D(const MFunction2D& Function2D);
  //! Default destructor
  virtual ~MFunction2D();

  //! Default assignment constructor
  const MFunction2D& operator=(const MFunction2D& F);

  //! Set the basic data, load the file and parse it
  bool Set(const MString FileName, 
           const MString KeyWord, 
           const unsigned int InterpolationType = c_InterpolationLinear);

  //! Multiply the x-axis by some value
  void ScaleX(double Scaler);

  //! Multiply the y-axis by some value
  void ScaleY(double Scaler);

  //! Scale the content by some value
  void ScaleZ(double Scaler);

  //! Evaluate the data for a specific x value 
  double Evaluate(double x, double y) const;
  double Eval(double x, double y) const; // deprecated

  //! Return random numbers x, y distributed as the underlying function
  void GetRandom(double& x, double& y);

  //! Get the minimum x-value
  double GetXMin() const;
  //! Get the maximum x-value
  double GetXMax() const;
  //! Get the minimum y-value
  double GetYMin() const;
  //! Get the maximum y-value
  double GetYMax() const;
  //! Get the minimum z-value
  double GetZMin();
  //! Get the maximum z-value
  double GetZMax();

  //! ID representing an unknown interpolation
  static const unsigned int c_InterpolationUnknown;
  //! ID representing a constant interpolation
  static const unsigned int c_InterpolationNone;
  //! ID representing a constant interpolation
  static const unsigned int c_InterpolationConstant;
  //! ID representing a linear interpolation (default)
  static const unsigned int c_InterpolationLinear;

  //! Plot the function in a Canvas (diagnostics only)
  void Plot();

  // protected methods:
 protected:

  // private methods:
 private:

  // protected members:
 protected:

  // private members:
 private:
  //! Interpolation method used
  unsigned int m_InterpolationType;

  //! The x-axis data
  vector<double> m_X;
  //! The y-axis data
  vector<double> m_Y;
  //! The z-axis data
  vector<double> m_Z;

  //! For random number generation store the maximum
  double m_Maximum;

  // For random number generation:
  //! The function as cumulative distribution:
  vector<double> m_Cumulative;
  

#ifdef ___CLING___
 public:
  ClassDef(MFunction2D, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
