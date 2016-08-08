/*
 * MResponseMatrixO2.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MResponseMatrixO2__
#define __MResponseMatrixO2__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <iostream>
#include <vector>
#include <functional>
using namespace std;

// MEGAlib libs:
#include "MGlobal.h"
#include "MResponseMatrix.h"
#include "MResponseMatrixO1.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MResponseMatrixO2 : public MResponseMatrix
{
  // public interface:
 public:
  MResponseMatrixO2();
  MResponseMatrixO2(vector<float> xDim, vector<float> yDim);
  MResponseMatrixO2(MString Name, vector<float> xDim, vector<float> yDim);
  virtual ~MResponseMatrixO2();

  void Init();

  bool operator==(const MResponseMatrixO2& R);  
  MResponseMatrixO2& operator+=(const MResponseMatrixO2& R);  
  MResponseMatrixO2& operator-=(const MResponseMatrixO2& R);  
  MResponseMatrixO2& operator/=(const MResponseMatrixO2& R);  

  MResponseMatrixO2& operator+=(const float& Value);  
  MResponseMatrixO2& operator*=(const float& Value);  

  void SetAxis(vector<float> xDim, vector<float> yDim);

  void SetAxisNames(MString x1Name, MString x2Name);
  MString GetAxisName(unsigned int order = 2) const;

  /// Set a value & expand if it does not exist
  void SetBinContent(unsigned int x, unsigned int y, float Value = 1);
  /// Add a value to the bin closest to x, y
  void Add(float x, float y, float Value = 1);
  void SetMatrix(unsigned int b, MResponseMatrixO1 R1);

  virtual unsigned long GetNBins() const;

  virtual float GetAxisContent(unsigned int b, unsigned int order = 2) const;
  virtual vector<float> GetAxis(unsigned int order = 2) const;
  virtual unsigned int GetAxisBins(unsigned int order = 2) const;
  virtual float GetAxisBinCenter(unsigned int b, unsigned int order = 2) const;
  virtual unsigned int GetAxisBin(float v, unsigned int order = 2) const;
  virtual float GetAxisMinimum(unsigned int order = 2) const;
  virtual float GetAxisMaximum(unsigned int order = 2) const;
  virtual float GetAxisLowEdge(unsigned int b, unsigned int order = 2) const;
  virtual float GetAxisHighEdge(unsigned int b, unsigned int order = 2) const;

  virtual float GetBinContent(unsigned int x1, unsigned int x2) const;
  virtual float GetBinContent(unsigned int x1, unsigned int x1axis, 
                              unsigned int x2, unsigned int x2axis) const;
  virtual float GetBinArea(unsigned int x1, unsigned int x2) const;
  virtual float Get(float x1, float x2) const;
  virtual float GetInterpolated(float x1, float x2, bool DoExtrapolate = false) const;

  virtual bool Write(MString FileName, bool Stream = false);

  virtual float GetMaximum() const;
  virtual float GetMinimum() const;
  virtual float GetSum() const;

  virtual MResponseMatrixO1 GetSumMatrixO1(unsigned int a1) const;
  virtual MResponseMatrixO2 GetSumMatrixO2(unsigned int a1, unsigned int a2) const;

  //! Return a slice of order 1 of this matrix (x1 is from axis a1)
  virtual MResponseMatrixO1 GetSliceInterpolated(float x1, unsigned int a1); 

  virtual void Smooth(unsigned int Times = 1);

  //! Return as a histogram
  TH1* GetHistogram(float x1 = c_ShowX, float x2 = c_ShowY, bool Normalized = true);
  //! Show as a histogram
  void Show(float x1 = c_ShowX, float x2 = c_ShowY, bool Normalized = true);

  // protected methods:
 protected:


  // private methods:
 private:
  //! Read the specific data of this class - the main file handling is done in the base class!
  virtual bool ReadSpecific(MFileResponse& Parser, const MString& Type, const int Version);



  // protected members:
 protected:


  // private members:
 private:
  MString m_NameAxisO2;
  vector<float> m_AxisO2;
  vector<MResponseMatrixO1> m_AxesO1;
  

  friend ostream& operator<<(ostream& os, const MResponseMatrixO2& R);


#ifdef ___CINT___
 public:
  ClassDef(MResponseMatrixO2, 1) // response matrix of order 2 (area)
#endif

};

ostream& operator<<(ostream& os, const MResponseMatrixO2& R);

#endif


////////////////////////////////////////////////////////////////////////////////
