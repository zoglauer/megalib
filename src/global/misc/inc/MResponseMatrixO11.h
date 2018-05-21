/*
 * MResponseMatrixO11.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MResponseMatrixO11__
#define __MResponseMatrixO11__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <iostream>
#include <vector>
#include <functional>
using namespace std;

// MEGAlib libs:
#include "MGlobal.h"
#include "MResponseMatrixOx.h"
#include "MResponseMatrixO10.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MResponseMatrixO11 : public MResponseMatrixOx
{
  // public interface:
 public:
  MResponseMatrixO11();
  MResponseMatrixO11(vector<float> x1Dim, vector<float> x2Dim, 
                     vector<float> x3Dim, vector<float> x4Dim, 
                     vector<float> x5Dim, vector<float> x6Dim, 
                     vector<float> x7Dim, vector<float> x8Dim, 
                     vector<float> x9Dim, vector<float> x10Dim, 
                     vector<float> x11Dim);
  MResponseMatrixO11(MString Name, 
                     vector<float> x1Dim, vector<float> x2Dim, 
                     vector<float> x3Dim, vector<float> x4Dim, 
                     vector<float> x5Dim, vector<float> x6Dim, 
                     vector<float> x7Dim, vector<float> x8Dim, 
                     vector<float> x9Dim, vector<float> x10Dim, 
                     vector<float> x11Dim);
  virtual ~MResponseMatrixO11();

  void Init();

  bool operator==(const MResponseMatrixO11& R);  
  MResponseMatrixO11& operator+=(const MResponseMatrixO11& R);  
  MResponseMatrixO11& operator-=(const MResponseMatrixO11& R);  
  MResponseMatrixO11& operator/=(const MResponseMatrixO11& R);  

  MResponseMatrixO11& operator+=(const float& Value);  
  MResponseMatrixO11& operator*=(const float& Value);  

  void SetAxis(vector<float> x1Dim, vector<float> x2Dim, 
               vector<float> x3Dim, vector<float> x4Dim, 
               vector<float> x5Dim, vector<float> x6Dim, 
               vector<float> x7Dim, vector<float> x8Dim, 
               vector<float> x9Dim, vector<float> x10Dim, 
               vector<float> x11Dim);

  void SetAxisNames(MString x1Name, MString x2Name, 
                    MString x3Name, MString x4Name, 
                    MString x5Name, MString x6Name, 
                    MString x7Name, MString x8Name, 
                    MString x9Name, MString x10Name, 
                    MString x11Name);
  MString GetAxisName(unsigned int order = 11) const;

  /// Set a value & expand if it does not exist
  void SetBinContent(unsigned int x1, unsigned int x2, unsigned int x3, 
                     unsigned int x4, unsigned int x5, unsigned int x6, 
                     unsigned int x7, unsigned int x8, unsigned int x9, 
                     unsigned int x10, unsigned int x11, float Value = 1);
  /// Add a value to the bin closest to x, y
  void Add(float x1, float x2, float x3, float x4, 
           float x5, float x6, float x7, float x8, 
           float x9, float x10, float x11, float Value = 1);
  void SetMatrix(unsigned int b, MResponseMatrixO10 R10);

  virtual unsigned long GetNBins() const;

  virtual float GetAxisContent(unsigned int b, unsigned int order = 11) const;
  virtual vector<float> GetAxis(unsigned int order = 11) const;
  virtual unsigned int GetAxisBins(unsigned int order = 11) const;
  virtual float GetAxisBinCenter(unsigned int b, unsigned int order = 11) const;
  virtual unsigned int GetAxisBin(float v, unsigned int order = 11) const;
  virtual float GetAxisMinimum(unsigned int order = 11) const;
  virtual float GetAxisMaximum(unsigned int order = 11) const;
  virtual float GetAxisLowEdge(unsigned int b, unsigned int order = 11) const;
  virtual float GetAxisHighEdge(unsigned int b, unsigned int order = 11) const;

  virtual float GetBinContent(unsigned int x1, unsigned int x2, 
                              unsigned int x3, unsigned int x4, 
                              unsigned int x5, unsigned int x6, 
                              unsigned int x7, unsigned int x8, 
                              unsigned int x9, unsigned int x10, 
                              unsigned int x11) const;
  virtual float GetBinContent(unsigned int x1, unsigned int x1axis, 
                              unsigned int x2, unsigned int x2axis,
                              unsigned int x3, unsigned int x3axis,
                              unsigned int x4, unsigned int x4axis,
                              unsigned int x5, unsigned int x5axis,
                              unsigned int x6, unsigned int x6axis,
                              unsigned int x7, unsigned int x7axis,
                              unsigned int x8, unsigned int x8axis,
                              unsigned int x9, unsigned int x9axis,
                              unsigned int x10, unsigned int x10axis,
                              unsigned int x11, unsigned int x11axis) const;
  virtual float GetBinArea(unsigned int x1, unsigned int x2, 
                           unsigned int x3, unsigned int x4, 
                           unsigned int x5, unsigned int x6, 
                           unsigned int x7, unsigned int x8, 
                           unsigned int x9, unsigned int x10, 
                           unsigned int x11) const;
  virtual float Get(float x1, float x2, float x3, float x4, 
                    float x5, float x6, float x7, float x8, 
                    float x9, float x10, float x11) const;
  virtual float GetInterpolated(float x1, float x2, float x3, float x4, 
                                float x5, float x6, float x7, float x8, 
                                float x9, float x10, float x11, 
                                bool DoExtrapolate = false) const;

  virtual float GetMaximum() const;
  virtual float GetMinimum() const;
  virtual double GetSum() const;

  virtual MResponseMatrixO1 GetSumMatrixO1(unsigned int a1) const;
  virtual MResponseMatrixO2 GetSumMatrixO2(unsigned int a1, unsigned int a2) const;
  virtual MResponseMatrixO3 GetSumMatrixO3(unsigned int a1, unsigned int a2, 
                                           unsigned int a3) const;
  virtual MResponseMatrixO4 GetSumMatrixO4(unsigned int a1, unsigned int a2, 
                                           unsigned int a3, unsigned int a4) const;
  virtual MResponseMatrixO5 GetSumMatrixO5(unsigned int a1, unsigned int a2, 
                                           unsigned int a3, unsigned int a4, 
                                           unsigned int a5) const;
  virtual MResponseMatrixO6 GetSumMatrixO6(unsigned int a1, unsigned int a2, 
                                           unsigned int a3, unsigned int a4, 
                                           unsigned int a5, unsigned int a6) const;
  virtual MResponseMatrixO7 GetSumMatrixO7(unsigned int a1, unsigned int a2, 
                                           unsigned int a3, unsigned int a4, 
                                           unsigned int a5, unsigned int a6, 
                                           unsigned int a7) const;
  virtual MResponseMatrixO8 GetSumMatrixO8(unsigned int a1, unsigned int a2, 
                                           unsigned int a3, unsigned int a4, 
                                           unsigned int a5, unsigned int a6, 
                                           unsigned int a7, unsigned int a8) const;
  virtual MResponseMatrixO9 GetSumMatrixO9(unsigned int a1, unsigned int a2, 
                                           unsigned int a3, unsigned int a4, 
                                           unsigned int a5, unsigned int a6, 
                                           unsigned int a7, unsigned int a8, 
                                           unsigned int a9) const;
  virtual MResponseMatrixO10 GetSumMatrixO10(unsigned int a1, unsigned int a2, 
                                             unsigned int a3, unsigned int a4, 
                                             unsigned int a5, unsigned int a6, 
                                             unsigned int a7, unsigned int a8, 
                                             unsigned int a9, unsigned int a10) const;
  virtual MResponseMatrixO11 GetSumMatrixO11(unsigned int a1, unsigned int a2, 
                                             unsigned int a3, unsigned int a4, 
                                             unsigned int a5, unsigned int a6, 
                                             unsigned int a7, unsigned int a8, 
                                             unsigned int a9, unsigned int a10, 
                                             unsigned int a11) const;

  virtual bool Write(MString FileName, bool Stream = false);

  void Show(float x1, float x2, float x3, float x4, 
            float x5, float x6, float x7, float x8, 
            float x9, float x10, float x11, bool Normalized = true);

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
  MString m_NameAxisO11;
  vector<float> m_AxisO11;
  vector<MResponseMatrixO10> m_AxesO10;
  

  friend ostream& operator<<(ostream& os, const MResponseMatrixO11& R);


#ifdef ___CLING___
 public:
  ClassDef(MResponseMatrixO11, 1) // response matrix of order 11
#endif

};

ostream& operator<<(ostream& os, const MResponseMatrixO11& R);

#endif


////////////////////////////////////////////////////////////////////////////////
