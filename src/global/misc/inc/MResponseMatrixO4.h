/*
 * MResponseMatrixO4.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MResponseMatrixO4__
#define __MResponseMatrixO4__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <iostream>
#include <vector>
#include <functional>
using namespace std;

// MEGAlib libs:
#include "MGlobal.h"
#include "MResponseMatrix.h"
#include "MResponseMatrixO3.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MResponseMatrixO4 : public MResponseMatrix
{
  // public interface:
 public:
  MResponseMatrixO4();
  MResponseMatrixO4(vector<float> x1Dim, vector<float> x2Dim, 
                    vector<float> x3Dim, vector<float> x4Dim);
  MResponseMatrixO4(MString Name, vector<float> x1Dim, vector<float> x2Dim, 
                    vector<float> x3Dim, vector<float> x4Dim);
  virtual ~MResponseMatrixO4();

  void Init();

  bool operator==(const MResponseMatrixO4& R);  
  MResponseMatrixO4& operator+=(const MResponseMatrixO4& R);  
  MResponseMatrixO4& operator-=(const MResponseMatrixO4& R);  
  MResponseMatrixO4& operator/=(const MResponseMatrixO4& R);  

  MResponseMatrixO4& operator+=(const float& Value);  
  MResponseMatrixO4& operator*=(const float& Value);  

  void SetAxis(vector<float> x1Dim, vector<float> x2Dim, 
               vector<float> x3Dim, vector<float> x4Dim);

  void SetAxisNames(MString x1Name, MString x2Name, 
                    MString x3Name, MString x4Name);
  MString GetAxisName(unsigned int order = 4) const;

  /// Set a value & expand if it does not exist
  void SetBinContent(unsigned int x1, unsigned int x2, unsigned int x3, unsigned int x4, float Value = 1);
  /// Add a value to the bin closest to x, y
  void Add(float x1, float x2, float x3, float x4, float Value = 1);
  void SetMatrix(unsigned int b, MResponseMatrixO3 R3);

  virtual unsigned int GetNBins() const;

  virtual float GetAxisContent(unsigned int b, unsigned int order = 4) const;
  virtual vector<float> GetAxis(unsigned int order = 4) const;
  virtual unsigned int GetAxisBins(unsigned int order = 4) const;
  virtual float GetAxisBinCenter(unsigned int b, unsigned int order = 4) const;
  virtual unsigned int GetAxisBin(float v, unsigned int order = 4) const;
  virtual float GetAxisMinimum(unsigned int order = 4) const;
  virtual float GetAxisMaximum(unsigned int order = 4) const;
  virtual float GetAxisLowEdge(unsigned int b, unsigned int order = 4) const;
  virtual float GetAxisHighEdge(unsigned int b, unsigned int order = 4) const;

  virtual float GetBinContent(unsigned int x1, unsigned int x2, 
                              unsigned int x3, unsigned int x4) const;
  virtual float GetBinContent(unsigned int x1, unsigned int x1axis, 
                              unsigned int x2, unsigned int x2axis,
                              unsigned int x3, unsigned int x3axis,
                              unsigned int x4, unsigned int x4axis) const;
  virtual float GetBinArea(unsigned int x1, unsigned int x2, 
                           unsigned int x3, unsigned int x4) const;
  virtual float Get(float x1, float x2, float x3, float x4) const;
  virtual float GetInterpolated(float x1, float x2, float x3, float x4, 
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

  virtual bool Write(MString FileName, bool Stream = false);

  void Show(float x1, float x2, float x3, float x4, bool Normalized = true);

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
  MString m_NameAxisO4;
  vector<float> m_AxisO4;
  vector<MResponseMatrixO3> m_AxesO3;
  

  friend ostream& operator<<(ostream& os, const MResponseMatrixO4& R);


#ifdef ___CINT___
 public:
  ClassDef(MResponseMatrixO4, 1) // response matrix of order 2 (area)
#endif

};

ostream& operator<<(ostream& os, const MResponseMatrixO4& R);

#endif


////////////////////////////////////////////////////////////////////////////////
