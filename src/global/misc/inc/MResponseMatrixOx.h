/*
 * MResponseMatrixOx.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MResponseMatrixOx__
#define __MResponseMatrixOx__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <vector>
#include <sstream>
using namespace std;

// MEGAlib libs:
#include "MGlobal.h"
#include "MFileResponse.h"
#include "MResponseMatrix.h"

// Forward declarations:
class MResponseMatrixO1;

////////////////////////////////////////////////////////////////////////////////


class MResponseMatrixOx : public MResponseMatrix
{
  // public interface:
 public:
  MResponseMatrixOx();
  MResponseMatrixOx(MString Name);
  virtual ~MResponseMatrixOx();

  virtual bool Read(MString FileName);
  
  virtual float GetAxisContent(unsigned int b, unsigned int order = 0) const = 0;
  virtual vector<float> GetAxis(unsigned int order = 0) const = 0;
  virtual unsigned int GetAxisBins(unsigned int order = 0) const = 0;
  virtual MString GetAxisName(unsigned int order = 0) const = 0;
  virtual float GetAxisMinimum(unsigned int order = 0) const = 0;
  virtual float GetAxisMaximum(unsigned int order = 0) const = 0;
  virtual float GetAxisLowEdge(unsigned int b, unsigned int order = 0) const = 0;
  virtual float GetAxisHighEdge(unsigned int b, unsigned int order = 0) const = 0;

  bool IsIncreasing(vector<float> Axis) const;

  void SetValuesCenteredFlag(bool Centered) { m_ValuesCentered = Centered; }
  bool AreValuesCentered() const { return m_ValuesCentered; }

  virtual MResponseMatrixO1 GetSumMatrixO1(unsigned int order = 0) const = 0;


  // protected methods:
 protected:
  //! Find the axis-bin where the axis value contains "Value" 
  int FindBin(const vector<float>& Array, float Value) const;
  //! This assumes
  int FindBinCentered(const vector<float>& Array, float Value) const;
  bool AreIncreasing(unsigned int order, 
                     unsigned int a1 = c_UnusedAxis,
                     unsigned int a2 = c_UnusedAxis,
                     unsigned int a3 = c_UnusedAxis,
                     unsigned int a4 = c_UnusedAxis,
                     unsigned int a5 = c_UnusedAxis,
                     unsigned int a6 = c_UnusedAxis,
                     unsigned int a7 = c_UnusedAxis,
                     unsigned int a8 = c_UnusedAxis,
                     unsigned int a9 = c_UnusedAxis,
                     unsigned int a10 = c_UnusedAxis,
                     unsigned int a11 = c_UnusedAxis,
                     unsigned int a12 = c_UnusedAxis,
                     unsigned int a13 = c_UnusedAxis,
                     unsigned int a14 = c_UnusedAxis,
                     unsigned int a15 = c_UnusedAxis,
                     unsigned int a16 = c_UnusedAxis,
                     unsigned int a17 = c_UnusedAxis,
                     unsigned int a18 = c_UnusedAxis) const;
                     
                     


  // private methods:
 private:

  // protected members:
 protected:
  static const unsigned long c_SizeLimit;
  static const unsigned long c_Outside;
  static const unsigned int c_UnusedAxis;  
  
  //! True if the values correspond to bin centers
  bool m_ValuesCentered;

  // private members:
 private:

#ifdef ___CINT___
 public:
  ClassDef(MResponseMatrixOx, 1) // base class for all response matrices
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
