/*
 * MResponseMatrixO3.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MResponseMatrixO3__
#define __MResponseMatrixO3__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <iostream>
#include <vector>
#include <functional>
using namespace std;

// MEGAlib libs:
#include "MGlobal.h"
#include "MResponseMatrix.h"
#include "MResponseMatrixO2.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MResponseMatrixO3 : public MResponseMatrix
{
  // public interface:
 public:
  MResponseMatrixO3();
  MResponseMatrixO3(vector<float> x1Dim, vector<float> x2Dim, 
                    vector<float> x3Dim);
  MResponseMatrixO3(MString Name, vector<float> x1Dim, vector<float> x2Dim, 
                    vector<float> x3Dim);
  virtual ~MResponseMatrixO3();

  void Init();

  bool operator==(const MResponseMatrixO3& R);  
  MResponseMatrixO3& operator+=(const MResponseMatrixO3& R);  
  MResponseMatrixO3& operator-=(const MResponseMatrixO3& R);  
  MResponseMatrixO3& operator/=(const MResponseMatrixO3& R);  

  MResponseMatrixO3& operator+=(const float& Value);  
  MResponseMatrixO3& operator*=(const float& Value);  

  void SetAxis(vector<float> x1Dim, vector<float> x2Dim, vector<float> x3Dim);

  void SetAxisNames(MString x1Name, MString x2Name, MString x3Name);
  MString GetAxisName(unsigned int order = 3) const;

  /// Set a value & expand if it does not exist
  void SetBinContent(unsigned int x1, unsigned int x2, unsigned int x3, float Value = 1);
  /// Add a value to the bin closest to x, y
  void Add(float x1, float x2, float x3, float Value = 1);
  void SetMatrix(unsigned int b, MResponseMatrixO2 R2);

  virtual unsigned long GetNBins() const;

  virtual float GetAxisContent(unsigned int b, unsigned int order = 3) const;
  virtual vector<float> GetAxis(unsigned int order = 3) const;
  virtual unsigned int GetAxisBins(unsigned int order = 3) const;
  virtual float GetAxisBinCenter(unsigned int b, unsigned int order = 3) const;
  virtual unsigned int GetAxisBin(float v, unsigned int order = 3) const;
  virtual float GetAxisMinimum(unsigned int order = 3) const;
  virtual float GetAxisMaximum(unsigned int order = 3) const;
  virtual float GetAxisLowEdge(unsigned int b, unsigned int order = 3) const;
  virtual float GetAxisHighEdge(unsigned int b, unsigned int order = 3) const;

  virtual float GetBinContent(unsigned int x1, unsigned int x2, unsigned int x3) const;
  virtual float GetBinContent(unsigned int x1, unsigned int x1axis, 
                              unsigned int x2, unsigned int x2axis,
                              unsigned int x3, unsigned int x3axis) const;
  virtual float GetBinArea(unsigned int x1, unsigned int x2, unsigned int x3) const;
  virtual float Get(float x1, float x2, float x3) const;
  virtual float GetInterpolated(float x1, float x2, float x3, bool DoExtrapolate = false) const;

  virtual float GetMaximum() const;
  virtual float GetMinimum() const;
  virtual float GetSum() const;

  virtual MResponseMatrixO1 GetSumMatrixO1(unsigned int a1) const;
  virtual MResponseMatrixO2 GetSumMatrixO2(unsigned int a1, unsigned int a2) const;
  virtual MResponseMatrixO3 GetSumMatrixO3(unsigned int a1, unsigned int a2, 
                                           unsigned int a3) const;

  //! Return a slice of order 1 of this matrix (x1 is from axis a1)
  virtual MResponseMatrixO1 GetSliceInterpolated(float x1, unsigned int a1, 
                                                 float x2, unsigned int a2); 

  virtual bool Write(MString FileName, bool Stream = false);

  virtual void Smooth(unsigned int Times = 1);

  //! Return as a histogram
  TH1* GetHistogram(float x1 = c_ShowX, float x2 = c_ShowY, 
                    float x3 = c_ShowZ, bool Normalized = true);

  //! Show as a histogram
  void Show(float x1 = c_ShowX, float x2 = c_ShowY, 
            float x3 = c_ShowZ, bool Normalized = true);

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
  MString m_NameAxisO3;
  vector<float> m_AxisO3;
  vector<MResponseMatrixO2> m_AxesO2;
  

  friend ostream& operator<<(ostream& os, const MResponseMatrixO3& R);


#ifdef ___CINT___
 public:
  ClassDef(MResponseMatrixO3, 1) // response matrix of order 2 (area)
#endif

};

ostream& operator<<(ostream& os, const MResponseMatrixO3& R);

#endif


////////////////////////////////////////////////////////////////////////////////
