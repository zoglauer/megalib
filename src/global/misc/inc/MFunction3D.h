/*
 * MFunction3D.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MFunction3D__
#define __MFunction3D__


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


class MFunction3D
{
  // public interface:
 public:
  //! Default constructor
  MFunction3D();
  //! Copy constructor
  MFunction3D(const MFunction3D& Function3D);
  //! Default destructor
  virtual ~MFunction3D();

  //! Default assignment constructor
  const MFunction3D& operator=(const MFunction3D& F);

  //! Set the basic data, load the file and parse it
  virtual bool Set(const MString FileName,
                   const MString KeyWord,
                   const unsigned int InterpolationType = c_InterpolationLinear);

  //! Set the basic data from four vectors
  virtual bool Set(const vector<double>& X, const vector<double>& Y, const vector<double>& Z, const vector<double>& Values, unsigned int InterpolationType = c_InterpolationLinear);

  //! Multiply the x-axis by some value
  void ScaleX(double Scaler);

  //! Scale the x-axis to it's new min and max
  void RescaleX(double XMin, double XMax);

  //! Multiply the y-axis by some value
  void ScaleY(double Scaler);

  //! Scale the x-axis to it's new min and max
  void RescaleY(double YMin, double YMax);

  //! Multiply the z-axis by some value
  void ScaleZ(double Scaler);

  //! Scale the x-axis to it's new min and max
  void RescaleZ(double ZMin, double ZMax);

  //! Invert the x-axis content
  void InvertX();

  //! Invert the y-axis content
  void InvertY();

  //! Invert the z-axis content
  void InvertZ();

  //! Scale the value axis by some value
  void ScaleV(double Scaler);

  //! Evaluate the data for a specific x value 
  double Evaluate(double x, double y, double z) const;
  double Eval(double x, double y, double z) const; //deprecated

  //! Return random numbers x, y distributed as the underlying function
  virtual void GetRandom(double& x, double& y, double& z);

  //! Get the minimum x-value
  double GetXMin() const;
  //! Get the maximum x-value
  double GetXMax() const;
  //! Get the minimum y-value
  double GetYMin() const;
  //! Get the maximum y-value
  double GetYMax() const;
  //! Get the minimum z-value
  double GetZMin() const;
  //! Get the maximum z-value
  double GetZMax() const;
  //! Get the minimum value
  double GetVMin() const;
  //! Get the maximum value
  double GetVMax();

  //! Return the x-axis
  vector<double> GetXAxis() const { return m_X; }
  //! Return the y-axis
  vector<double> GetYAxis() const { return m_Y; }
  //! Return the z-axis
  vector<double> GetZAxis() const { return m_Z; }
  //! Return all values
  vector<double> GetValues() const { return m_V; }

  //! ID representing an unknown interpolation
  static const unsigned int c_InterpolationUnknown;
  //! ID representing a constant interpolation
  static const unsigned int c_InterpolationConstant;
  //! ID representing a linear interpolation (default)
  static const unsigned int c_InterpolationLinear;

  //! Plot the function in a Canvas (diagnostics only)
  virtual void Plot(bool Random = false);

  //! Save the data:
  virtual bool Save(const MString FileName, const MString Keyword = "DP");

  // protected methods:
 protected:
  //! Find the x bin fast (switches between linear search and binary search)
  int FindXBin(double x) const;
  //! Find the y bin fast (switches between linear search and binary search)
  int FindYBin(double y) const;
  //! Find the z bin fast (switches between linear search and binary search)
  int FindZBin(double z) const;

  // private methods:
 private:

  // protected members:
 protected:
  //! Interpolation method used
  unsigned int m_InterpolationType;

  //! The x-axis data
  vector<double> m_X;
  //! If the edges are equidistant this is the distance
  double m_XDistance;
  //! The y-axis data
  vector<double> m_Y;
  //! If the edges are equidistant this is the distance
  double m_YDistance;
  //! The z-axis data
  vector<double> m_Z;
  //! If the edges are equidistant this is the distance
  double m_ZDistance;
  //! The value-axis data
  vector<double> m_V;

  //! For random number generation store the maximum
  double m_Maximum;

  // private members:
 private:
  

#ifdef ___CLING___
 public:
  ClassDef(MFunction3D, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
