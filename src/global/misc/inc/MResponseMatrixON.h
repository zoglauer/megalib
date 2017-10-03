/*
 * MResponseMatrixON.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MResponseMatrixON__
#define __MResponseMatrixON__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <iostream>
#include <vector>
#include <functional>
using namespace std;

// MEGAlib libs:
#include "MGlobal.h"
#include "MResponseMatrix.h"
#include "MResponseMatrixAxis.h"

// ROOT libs:
#include <TGraph.h>

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! N dimensional response matrix with arbitrary axes
class MResponseMatrixON : public MResponseMatrix
{
  // public interface:
 public:
  //! Default constructor
  MResponseMatrixON();
  //! Default constructor with response name
  MResponseMatrixON(const MString& Name);
  //! DEfault destructor
  virtual ~MResponseMatrixON();
  
  //! Add an axis
  void AddAxis(const MResponseMatrixAxis& Axis);
  //! Add a linear axis
  void AddAxisLinear(const MString& Name, unsigned int NBins, double Min, double Max, double UnderFlowMin = g_DoubleNotDefined, double OverFlowMax = g_DoubleNotDefined);
  //! Add a logarithmic axis
  void AddAxisLogarithmic(const MString& Name, unsigned int NBins, double Min, double Max, double UnderFlowMin = g_DoubleNotDefined, double OverFlowMax = g_DoubleNotDefined);

  //! Equality operator -- only the axes are considered, NOT the content
  bool operator==(const MResponseMatrixON& ResponseMatrixO1);
  //! Add another matrix' content to this response matrix
  MResponseMatrixON& operator+=(const MResponseMatrixON& ResponseMatrixO1);
  //! Subtract another matrix' content from this response matrix
  MResponseMatrixON& operator-=(const MResponseMatrixON& ResponseMatrixO1);
  //! Divide each bin of the response matrix, of one is zero the result is zero
  MResponseMatrixON& operator/=(const MResponseMatrixON& ResponseMatrixO1);  
  //! Initialize the response - after axes have been add
  void Init();
  //! Add a scalar to the matrix
  MResponseMatrixON& operator+=(const float& Value);
  //! Subtract a scalar from the matrix
  MResponseMatrixON& operator-=(const float& Value);
  //! Multiply matrix by a scalar
  MResponseMatrixON& operator*=(const float& Value);  
  //! Divide matrix by a scalar
  MResponseMatrixON& operator/=(const float& Value);  
  
  //! return an axis name (order starts with 1)
  //! Throws exception
  vector<MString> GetAxisNames(unsigned int AxisIndex) const;
  
  //! Return the number of bins
  virtual unsigned long GetNBins() const;
  
  //! Return the number of axes
  unsigned int GetNumberOfAxes() { return m_Axes.size(); }
  
  //! Return a const reference to the given axis
  //! Throws exception MExceptionIndexOutOfBounds
  const MResponseMatrixAxis& GetAxis(unsigned int AxisIndex) const;
  
  //! Check if the values are inside the range of all axes
  bool InRange(vector<double> X) const;
  //! Check if the bins are inside the range of all axes
  bool InRange(vector<unsigned int> Bins) const;
  
  // Interface to modify the content
  
  //! Set the content of a specific bin -- directly without error checks
  //! Logic: a1 + S1*a2 + S1*S2*a3 + S1*S2*S3*a4 + ....  
  void Set(unsigned int Bin, float Value = 1) { m_Values.at(Bin) = Value; }
  //! Set the bin content
  //! Throw exception "MExceptionTestFailed" when out of bounds
  void Set(vector<unsigned int> AxisBins, float Value = 1);
  //! Find the bin and set the value
  //! Throws exception MExceptionTestFailed
  void Set(vector<double> AxisValues, float Value = 1);
  //! Find the bin and add the value
  //! Throws exception MExceptionTestFailed
  void Add(vector<unsigned int> AxisValues, float Value = 1);
  //! Find the bin and add the value
  //! Throws exception MExceptionTestFailed
  void Add(vector<double> AxisValues, float Value = 1);
  //! Add to the content of a specific bin -- directly without error checks
  //! Logic: a1 + S1*a2 + S1*S2*a3 + S1*S2*S3*a4 + ....  
  void Add(unsigned int Bin, float Value = 1) { m_Values.at(Bin) += Value; }
  
  // Interface to retrieve the content

  //! Get the area of a bin corresponding to the specific value
  virtual float GetArea(vector<double> AxisValues) const;
  //! Get the content of a specific bin 
  virtual float Get(vector<unsigned int> AxisBins) const;
  //! Get the content of a specific bin -- directly without error checks
  //! Logic: a1 + S1*a2 + S1*S2*a3 + S1*S2*S3*a4 + ....  
  virtual float Get(unsigned int Bin) const { return m_Values.at(Bin); }
  //! Get the content of the bin corresponding to the specific value
  virtual float Get(vector<double> AxisValues) const;
  //! Get the interpolated content of the bin corresponding to the specific value
  virtual float GetInterpolated(vector<double> AxisValues, bool DoExtrapolate = false) const;
  
  
  // Interface to the axes:
  
  // Miscellaneous
  
  //! Find the maximum value 
  virtual float GetMaximum() const;
  //! Find the minimum value
  virtual float GetMinimum() const;
  //! Return the sum
  virtual double GetSum() const;

  //! Write the values of the response to file
  virtual bool Write(MString FileName, bool Stream = false);

  //! Smooth the content of the response
  virtual void Smooth(unsigned int Times = 1);
  
  //! Show as an image
  void ShowSlice(vector<float> Axes, bool Normalized = true, MString Title = "");
  
  //! Return a string with statistics numbers
  virtual MString GetStatistics() const;
  

  // protected methods:
 protected:
 

  // private methods:
 private:
  //! Read the specific data of this class - the main file handling is done in the base class!
  virtual bool ReadSpecific(MFileResponse& Parser, const MString& Type, const int Version);

  //! Find the bin of m_Values corresponding to the axis bins X
  unsigned int FindBin(vector<unsigned int> X) const;
  //! Find the bin of m_Values corresponding to the axis values X
  unsigned int FindBin(vector<double> X) const;
  //! Find the axes bins corresponding to the axis values X
  vector<unsigned int> FindBins(vector<double> X) const;
  //! Find the axes bins corresponding to the internal value bin Bin
  vector<unsigned int> FindBins(unsigned int Bin) const;
    
  //! Given an order, return the axis it belongs to
  //! Can throw: MExceptionValueOutOfBounds, MExceptionNeverReachThatLineOfCode
  MResponseMatrixAxis* GetAxisByOrder(unsigned int Order);
  
  // protected members:
 protected:
  //! Name of this response
  //MString m_Name;
  //! Order/Dimension of this response 
  //unsigned int m_Order;
  
  //! The axes
  vector<MResponseMatrixAxis*> m_Axes;
  //! The data
  vector<float> m_Values;


  // private members:
 private:
  friend ostream& operator<<(ostream& os, const MResponseMatrixON& R);


#ifdef ___CINT___
 public:
  ClassDef(MResponseMatrixON, 1) // response matrix of order 1 (linear)
#endif

};

ostream& operator<<(ostream& os, const MResponseMatrixON& R);

#endif


////////////////////////////////////////////////////////////////////////////////
