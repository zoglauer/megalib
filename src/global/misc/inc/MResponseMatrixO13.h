/*
 * MResponseMatrixO13.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MResponseMatrixO13__
#define __MResponseMatrixO13__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <iostream>
#include <vector>
#include <functional>
using namespace std;

// MEGAlib libs:
#include "MGlobal.h"
#include "MResponseMatrixOx.h"
#include "MResponseMatrixO12.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MResponseMatrixO13 : public MResponseMatrixOx
{
  // public interface:
 public:
  MResponseMatrixO13();
  MResponseMatrixO13(vector<float> x1Dim, vector<float> x2Dim, 
                     vector<float> x3Dim, vector<float> x4Dim, 
                     vector<float> x5Dim, vector<float> x6Dim, 
                     vector<float> x7Dim, vector<float> x8Dim, 
                     vector<float> x9Dim, vector<float> x10Dim, 
                     vector<float> x11Dim, vector<float> x12Dim, 
                     vector<float> x13Dim);
  MResponseMatrixO13(MString Name, 
                     vector<float> x1Dim, vector<float> x2Dim, 
                     vector<float> x3Dim, vector<float> x4Dim, 
                     vector<float> x5Dim, vector<float> x6Dim, 
                     vector<float> x7Dim, vector<float> x8Dim, 
                     vector<float> x9Dim, vector<float> x10Dim, 
                     vector<float> x11Dim, vector<float> x12Dim, 
                     vector<float> x13Dim);
  virtual ~MResponseMatrixO13();

  void Init();

  bool operator==(const MResponseMatrixO13& R);  
  MResponseMatrixO13& operator+=(const MResponseMatrixO13& R);  
  MResponseMatrixO13& operator-=(const MResponseMatrixO13& R);  
  MResponseMatrixO13& operator/=(const MResponseMatrixO13& R);  

  MResponseMatrixO13& operator+=(const float& Value);  
  MResponseMatrixO13& operator*=(const float& Value);  

  void SetAxis(vector<float> x1Dim, vector<float> x2Dim, 
               vector<float> x3Dim, vector<float> x4Dim, 
               vector<float> x5Dim, vector<float> x6Dim, 
               vector<float> x7Dim, vector<float> x8Dim, 
               vector<float> x9Dim, vector<float> x10Dim, 
               vector<float> x11Dim, vector<float> x12Dim, 
               vector<float> x13Dim);

  void SetAxisNames(MString x1Name, MString x2Name, 
                    MString x3Name, MString x4Name, 
                    MString x5Name, MString x6Name, 
                    MString x7Name, MString x8Name, 
                    MString x9Name, MString x10Name, 
                    MString x11Name, MString x12Name, 
                    MString x13Name);
  MString GetAxisName(unsigned int order = 13) const;

  /// Set a value & expand if it does not exist
  void SetBinContent(unsigned int x1, unsigned int x2, unsigned int x3, 
                     unsigned int x4, unsigned int x5, unsigned int x6, 
                     unsigned int x7, unsigned int x8, unsigned int x9, 
                     unsigned int x10, unsigned int x11, unsigned int x12, 
                     unsigned int x13, float Value = 1);
  /// Add a value to the bin closest to x, y
  void Add(float x1, float x2, float x3, float x4, 
           float x5, float x6, float x7, float x8, 
           float x9, float x10, float x11, float x12, 
           float x13, float Value = 1);
  void SetMatrix(unsigned int b, MResponseMatrixO12 R12);

  virtual unsigned long GetNBins() const;

  virtual float GetAxisContent(unsigned int b, unsigned int order = 13) const;
  virtual vector<float> GetAxis(unsigned int order = 13) const;
  virtual unsigned int GetAxisBins(unsigned int order = 13) const;
  virtual float GetAxisBinCenter(unsigned int b, unsigned int order = 13) const;
  virtual unsigned int GetAxisBin(float v, unsigned int order = 13) const;
  virtual float GetAxisMinimum(unsigned int order = 13) const;
  virtual float GetAxisMaximum(unsigned int order = 13) const;
  virtual float GetAxisLowEdge(unsigned int b, unsigned int order = 13) const;
  virtual float GetAxisHighEdge(unsigned int b, unsigned int order = 13) const;

  virtual float GetBinContent(unsigned int x1, unsigned int x2, 
                              unsigned int x3, unsigned int x4, 
                              unsigned int x5, unsigned int x6, 
                              unsigned int x7, unsigned int x8, 
                              unsigned int x9, unsigned int x10, 
                              unsigned int x11, unsigned int x12, 
                              unsigned int x13) const;
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
                              unsigned int x11, unsigned int x11axis,
                              unsigned int x12, unsigned int x12axis, 
                              unsigned int x13, unsigned int x13axis) const;
  virtual float GetBinArea(unsigned int x1, unsigned int x2, 
                           unsigned int x3, unsigned int x4, 
                           unsigned int x5, unsigned int x6, 
                           unsigned int x7, unsigned int x8, 
                           unsigned int x9, unsigned int x10, 
                           unsigned int x11, unsigned int x12, 
                           unsigned int x13) const;
  virtual float Get(float x1, float x2, float x3, float x4, 
                    float x5, float x6, float x7, float x8, 
                    float x9, float x10, float x11, float x12, 
                    float x13) const;
  virtual float GetInterpolated(float x1, float x2, float x3, float x4, 
                                float x5, float x6, float x7, float x8, 
                                float x9, float x10, float x11, float x12, 
                                float x13, bool DoExtrapolate = false) const;

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
  virtual MResponseMatrixO12 GetSumMatrixO12(unsigned int a1, unsigned int a2, 
                                             unsigned int a3, unsigned int a4, 
                                             unsigned int a5, unsigned int a6, 
                                             unsigned int a7, unsigned int a8, 
                                             unsigned int a9, unsigned int a10, 
                                             unsigned int a11, unsigned int a12) const;
  virtual MResponseMatrixO13 GetSumMatrixO13(unsigned int a1, unsigned int a2, 
                                             unsigned int a3, unsigned int a4, 
                                             unsigned int a5, unsigned int a6, 
                                             unsigned int a7, unsigned int a8, 
                                             unsigned int a9, unsigned int a10, 
                                             unsigned int a11, unsigned int a12, 
                                             unsigned int a13) const;

  virtual bool Write(MString FileName, bool Stream = false);

  void Show(float x1, float x2, float x3, float x4, 
            float x5, float x6, float x7, float x8, 
            float x9, float x10, float x11, float x12, 
            float x13, bool Normalized = true);

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
  MString m_NameAxisO13;
  vector<float> m_AxisO13;
  vector<MResponseMatrixO12> m_AxesO12;
  

  friend ostream& operator<<(ostream& os, const MResponseMatrixO13& R);


#ifdef ___CINT___
 public:
  ClassDef(MResponseMatrixO13, 1) // response matrix of order 13
#endif

};

ostream& operator<<(ostream& os, const MResponseMatrixO13& R);

#endif


////////////////////////////////////////////////////////////////////////////////
