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
#include <mutex>
using namespace std;

// MEGAlib libs:
#include "MGlobal.h"
#include "MResponseMatrix.h"
#include "MResponseMatrixAxis.h"

// ROOT libs:
#include "TGraph.h"
#include "TVirtualStreamerInfo.h" // Required for the ROOt streamer

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! N dimensional response matrix with arbitrary axes
class MResponseMatrixON : public MResponseMatrix
{
  // public interface:
 public:
  //! Default constructor
  MResponseMatrixON(bool IsSparse = false);
  //! Default constructor with response name
  MResponseMatrixON(const MString& Name, bool IsSparse = false);
  //! Default copy constructor
  MResponseMatrixON(const MResponseMatrixON&);
  //! Default destructor
  virtual ~MResponseMatrixON();
  
  //! Clear all data
  virtual void Clear();

  //! True if the matrix is sparse
  bool IsSparse() const { return m_IsSparse; }
  
  //! Switch to sparse mode 
  void SwitchToSparse();
  //! Switch to non-sparse mode
  void SwitchToNonSparse();
  
  //! Add an axis
  void AddAxis(const MResponseMatrixAxis& Axis);
  //! Add a linear axis
  void AddAxisLinear(const MString& Name, unsigned long NBins, double Min, double Max, double UnderFlowMin = g_DoubleNotDefined, double OverFlowMax = g_DoubleNotDefined);
  //! Add a logarithmic axis
  void AddAxisLogarithmic(const MString& Name, unsigned long NBins, double Min, double Max, double UnderFlowMin = g_DoubleNotDefined, double OverFlowMax = g_DoubleNotDefined);

  //! Assignment operator
  MResponseMatrixON& operator=(const MResponseMatrixON& Other);
  
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
  //! Add a scalar to the matrix -- this will automatically convert a sparse matrix to a non-sparse one
  MResponseMatrixON& operator+=(const float& Value);
  //! Subtract a scalar from the matrix -- this will automatically convert a sparse matrix to a non-sparse one
  MResponseMatrixON& operator-=(const float& Value);
  //! Multiply matrix by a scalar
  MResponseMatrixON& operator*=(const float& Value);  
  //! Divide matrix by a scalar
  MResponseMatrixON& operator/=(const float& Value);  
  
  //! Collapse some of the axes indicated by true
  MResponseMatrixON Collapse(vector<bool> AxesToCollpse);
  
  //! return an axis name (order starts with 1)
  //! Throws exception
  vector<MString> GetAxisNames(unsigned int AxisIndex) const;
  
  //! Return the number of bins
  virtual unsigned long GetNBins() const { return m_NumberOfBins; }
  
  //! Return the number of sparse bins
  virtual unsigned long GetNumberOfSparseBins() const { return m_BinsSparse.size(); }
  
  //! Return the number of axes
  unsigned int GetNumberOfAxes() { return m_Axes.size(); }
  
  //! Return a const reference to the given axis
  //! Throws exception MExceptionIndexOutOfBounds
  const MResponseMatrixAxis& GetAxis(unsigned int AxisIndex) const;
  
  //! Check if the values are inside the range of all axes
  bool InRange(vector<double> X) const;
  //! Check if the bins are inside the range of all axes
  bool InRange(vector<unsigned long> Bins) const;
  
  // Check for bins
  
  //! Find the bin of m_Values corresponding to the axis bins X
  unsigned long FindBin(vector<unsigned long> X) const;
  //! Find the bin of m_Values corresponding to the axis values X
  unsigned long FindBin(vector<double> X) const;
  
  //! Find the axes bins corresponding to the axis values X
  vector<unsigned long> FindBins(vector<double> X) const;
  //! Find the axes bins corresponding to the internal value bin Bin
  vector<unsigned long> FindBins(unsigned long Bin) const;
  //! Find the axes bins corresponding to the sparse Bin
  vector<unsigned long> FindBinsSparse(unsigned long SparseBin) const;
  
  // Interface to modify the content
  
  //! Set the content of a specific bin -- directly without error checks
  //! Logic: a1 + S1*a2 + S1*S2*a3 + S1*S2*S3*a4 + ....  
  void Set(unsigned long Bin, float Value = 1);
  //! Set the bin content
  //! Throw exception "MExceptionTestFailed" when out of bounds
  void Set(vector<unsigned long> AxisBins, float Value = 1);
  //! Find the bin and set the value
  //! Throws exception MExceptionTestFailed
  void Set(vector<double> AxisValues, float Value = 1);
  //! Find the bin and add the value
  //! Throws exception MExceptionTestFailed
  void Add(vector<unsigned long> AxisValues, float Value = 1);
  //! Find the bin and add the value
  //! Throws exception MExceptionTestFailed
  void Add(vector<double> AxisValues, float Value = 1);
  //! Add to the content of a specific bin -- directly without error checks
  //! Logic: a1 + S1*a2 + S1*S2*a3 + S1*S2*S3*a4 + ....  
  void Add(unsigned long Bin, float Value = 1);
  //! Add a collection of bin values
  void Add(vector<unsigned long> Bins, vector<float> Values);
  
  // Specific for sparse
  
  //! Set the content of a sparse bin
  void SetSparse(unsigned long SparseBin, float Value = 1);
  //! Add to the content of a sparse bin
  void AddSparse(unsigned long SparseBin, float Value = 1);
  
  
  // Interface to retrieve the content

  //! Get the area of a bin corresponding to the specific value
  virtual float GetArea(vector<double> AxisValues) const;
  //! Get the content of a specific bin 
  virtual float Get(vector<unsigned long> AxisBins) const;
  //! Get the content of a specific bin -- directly without error checks
  //! Logic: a1 + S1*a2 + S1*S2*a3 + S1*S2*S3*a4 + ....  
  virtual float Get(unsigned long Bin) const;
  //! Get the content of the bin corresponding to the specific value
  virtual float Get(vector<double> AxisValues) const;
  //! Get the interpolated content of the bin corresponding to the specific value
  virtual float GetInterpolated(vector<double> AxisValues, bool DoExtrapolate = false) const;
  
  // Specific for sparse
  
  //! Set the content of a sparse bin
  virtual float GetSparse(unsigned long SparseBin) const;
  
  
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
  //! Calculate the number of bins
  unsigned long CalculateNumberOfBins() const;
   
  //! Read the specific data of this class - the main file handling is done in the base class!
  virtual bool ReadSpecific(MFileResponse& Parser, const MString& Type, const int Version, const bool MultiThreaded = false);

  
  //! Sort the sparse matrix
  void SortSparse();
  
  //! Given an order, return the axis it belongs to
  //! Can throw: MExceptionValueOutOfBounds, MExceptionNeverReachThatLineOfCode
  MResponseMatrixAxis* GetAxisByOrder(unsigned int Order);
  
  //! The thread entry point for parallelized sparse file reading
  void SparseReadThreadEntry(unsigned int ThreadID, unsigned int Start, unsigned int Stop);

  
  // protected members:
 protected:

  //! The number of bins
  unsigned long m_NumberOfBins;
   
  //! The axes
  vector<MResponseMatrixAxis*> m_Axes;
  //! The number of axes - just for speed up
  unsigned int m_NumberOfAxes;
  
  //! Flag indicating that we are in sparse mode
  bool m_IsSparse;
  
  //! The data in non-sparse mode
  vector<float> m_Values;
  
  //! The data in sparse mode
  vector<float> m_ValuesSparse;
  //! Axis values in sparse mode
  vector<unsigned long> m_BinsSparse;

  //! Indicator if the threads are running
  vector<bool> m_ThreadRunning;
  //! Thread parameter mutex
  mutex m_ThreadMutex;
  //! The lines to parse in parallel
  vector<MString> m_ThreadLines;
  //! Is the line data good
  vector<bool> m_ThreadGoodData;
  //! The temporarily stored extracted bin ID of the line
  vector<unsigned long> m_ThreadBins;
  //! The temporarily stored extracted value
  vector<float> m_ThreadValues;

  // private members:
 private:
  friend ostream& operator<<(ostream& os, const MResponseMatrixON& R);


#ifdef ___CLING___
 public:
  ClassDef(MResponseMatrixON, 1) 
#endif

};

ostream& operator<<(ostream& os, const MResponseMatrixON& R);

#endif


////////////////////////////////////////////////////////////////////////////////
