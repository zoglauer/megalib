/*
 * MResponseMatrixO5.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MResponseMatrixO5__
#define __MResponseMatrixO5__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <iostream>
#include <vector>
#include <functional>
using namespace std;

// MEGAlib libs:
#include "MGlobal.h"
#include "MResponseMatrix.h"
#include "MResponseMatrixO4.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MResponseMatrixO5 : public MResponseMatrix
{
  // public interface:
 public:
  MResponseMatrixO5();
  MResponseMatrixO5(vector<float> x1Dim, vector<float> x2Dim, 
                    vector<float> x3Dim, vector<float> x4Dim, vector<float> x5Dim);
  MResponseMatrixO5(MString Name, vector<float> x1Dim, vector<float> x2Dim, 
                    vector<float> x3Dim, vector<float> x4Dim, vector<float> x5Dim);
  virtual ~MResponseMatrixO5();

  void Init();

  bool operator==(const MResponseMatrixO5& R);  
  MResponseMatrixO5& operator+=(const MResponseMatrixO5& R);  
  MResponseMatrixO5& operator-=(const MResponseMatrixO5& R);  
  MResponseMatrixO5& operator/=(const MResponseMatrixO5& R);  

  MResponseMatrixO5& operator+=(const float& Value);  
  MResponseMatrixO5& operator*=(const float& Value);  

  void SetAxis(vector<float> x1Dim, vector<float> x2Dim, 
               vector<float> x3Dim, vector<float> x4Dim, vector<float> x5Dim);

  void SetAxisNames(MString x1Name, MString x2Name, 
                    MString x3Name, MString x4Name, MString x5Name);
  MString GetAxisName(unsigned int order = 5) const;

  /// Set a value & expand if it does not exist
  void SetBinContent(unsigned int x1, unsigned int x2, unsigned int x3, 
                     unsigned int x4, unsigned int x5, float Value = 1);
  /// Add a value to the bin closest to x, y
  void Add(float x1, float x2, float x3, float x4, float x5, float Value = 1);
  void SetMatrix(unsigned int b, MResponseMatrixO4 R4);

  virtual unsigned long GetNBins() const;

  virtual float GetAxisContent(unsigned int b, unsigned int order = 5) const;
  virtual vector<float> GetAxis(unsigned int order = 5) const;
  virtual unsigned int GetAxisBins(unsigned int order = 5) const;
  virtual float GetAxisBinCenter(unsigned int b, unsigned int order = 5) const;
  virtual unsigned int GetAxisBin(float v, unsigned int order = 5) const;
  virtual float GetAxisMinimum(unsigned int order = 5) const;
  virtual float GetAxisMaximum(unsigned int order = 5) const;
  virtual float GetAxisLowEdge(unsigned int b, unsigned int order = 5) const;
  virtual float GetAxisHighEdge(unsigned int b, unsigned int order = 5) const;

  virtual float GetBinContent(unsigned int x1, unsigned int x2, 
                              unsigned int x3, unsigned int x4, unsigned int x5) const;
  virtual float GetBinContent(unsigned int x1, unsigned int x1axis, 
                              unsigned int x2, unsigned int x2axis,
                              unsigned int x3, unsigned int x3axis,
                              unsigned int x4, unsigned int x4axis,
                              unsigned int x5, unsigned int x5axis) const;
  virtual float GetBinArea(unsigned int x1, unsigned int x2, 
                           unsigned int x3, unsigned int x4, 
                           unsigned int x5) const;
  virtual float Get(float x1, float x2, float x3, float x4, float x5) const;
  virtual float GetInterpolated(float x1, float x2, float x3, float x4, float x5, 
                                bool DoExtrapolate = false) const;

  virtual float GetMaximum() const;
  virtual float GetMinimum() const;
  virtual float GetSum() const;

  virtual MResponseMatrixO1 GetSumMatrixO1(unsigned int a1) const;
  virtual MResponseMatrixO2 GetSumMatrixO2(unsigned int a1, unsigned int a2) const;
  virtual MResponseMatrixO3 GetSumMatrixO3(unsigned int a1, unsigned int a2, 
                                           unsigned int a3) const;
  virtual MResponseMatrixO4 GetSumMatrixO4(unsigned int a1, unsigned int a2, 
                                           unsigned int a3, unsigned int a4) const;
  virtual MResponseMatrixO5 GetSumMatrixO5(unsigned int a1, unsigned int a2, 
                                           unsigned int a3, unsigned int a4, 
                                           unsigned int a5) const;

  //! Return a slice of order 1 of this matrix (x1 is from axis a1)
  virtual MResponseMatrixO1 GetSliceInterpolated(float x1, unsigned int a1, 
                                                 float x2, unsigned int a2,
                                                 float x3, unsigned int a3, 
                                                 float x4, unsigned int a4); 

  virtual bool Write(MString FileName, bool Stream = false);

  TH1* GetHistogram(float x1, float x2, float x3, float x4, float x5, bool Normalized = true);
  void Show(float x1, float x2, float x3, float x4, float x5, bool Normalized = true);

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
  MString m_NameAxisO5;
  vector<float> m_AxisO5;
  vector<MResponseMatrixO4> m_AxesO4;
  

  friend ostream& operator<<(ostream& os, const MResponseMatrixO5& R);


#ifdef ___CINT___
 public:
  ClassDef(MResponseMatrixO5, 1) // response matrix of order 5
#endif

};

ostream& operator<<(ostream& os, const MResponseMatrixO5& R);

#endif


////////////////////////////////////////////////////////////////////////////////
