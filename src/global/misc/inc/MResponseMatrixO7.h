/*
 * MResponseMatrixO7.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MResponseMatrixO7__
#define __MResponseMatrixO7__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <iostream>
#include <vector>
#include <functional>
using namespace std;

// MEGAlib libs:
#include "MGlobal.h"
#include "MResponseMatrixOx.h"
#include "MResponseMatrixO6.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MResponseMatrixO7 : public MResponseMatrixOx
{
  // public interface:
 public:
  MResponseMatrixO7();
  MResponseMatrixO7(vector<float> x1Dim, vector<float> x2Dim, 
                    vector<float> x3Dim, vector<float> x4Dim, 
                    vector<float> x5Dim, vector<float> x6Dim, 
                    vector<float> x7Dim);
  MResponseMatrixO7(MString Name, 
                    vector<float> x1Dim, vector<float> x2Dim, 
                    vector<float> x3Dim, vector<float> x4Dim, 
                    vector<float> x5Dim, vector<float> x6Dim, 
                    vector<float> x7Dim);
  virtual ~MResponseMatrixO7();

  void Init();

  bool operator==(const MResponseMatrixO7& R);  
  MResponseMatrixO7& operator+=(const MResponseMatrixO7& R);  
  MResponseMatrixO7& operator-=(const MResponseMatrixO7& R);  
  MResponseMatrixO7& operator/=(const MResponseMatrixO7& R);  

  MResponseMatrixO7& operator+=(const float& Value);  
  MResponseMatrixO7& operator*=(const float& Value);  

  void SetAxis(vector<float> x1Dim, vector<float> x2Dim, 
               vector<float> x3Dim, vector<float> x4Dim, 
               vector<float> x5Dim, vector<float> x6Dim, 
               vector<float> x7Dim);

  void SetAxisNames(MString x1Name, MString x2Name, 
                    MString x3Name, MString x4Name, 
                    MString x5Name, MString x6Name, 
                    MString x7Name);
  MString GetAxisName(unsigned int order = 7) const;

  /// Set a value & expand if it does not exist
  void SetBinContent(unsigned int x1, unsigned int x2, unsigned int x3, 
                     unsigned int x4, unsigned int x5, unsigned int x6, 
                     unsigned int x7, float Value = 1);
  /// Add a value to the bin closest to x, y
  void Add(float x1, float x2, float x3, float x4, 
           float x5, float x6, float x7, float Value = 1);
  void SetMatrix(unsigned int b, MResponseMatrixO6 R6);

  virtual unsigned long GetNBins() const;

  virtual float GetAxisContent(unsigned int b, unsigned int order = 7) const;
  virtual vector<float> GetAxis(unsigned int order = 7) const;
  virtual unsigned int GetAxisBins(unsigned int order = 7) const;
  virtual float GetAxisBinCenter(unsigned int b, unsigned int order = 7) const;
  virtual unsigned int GetAxisBin(float v, unsigned int order = 7) const;
  virtual float GetAxisMinimum(unsigned int order = 7) const;
  virtual float GetAxisMaximum(unsigned int order = 7) const;
  virtual float GetAxisLowEdge(unsigned int b, unsigned int order = 7) const;
  virtual float GetAxisHighEdge(unsigned int b, unsigned int order = 7) const;

  virtual float GetBinContent(unsigned int x1, unsigned int x2, 
                              unsigned int x3, unsigned int x4, 
                              unsigned int x5, unsigned int x6, 
                              unsigned int x7) const;
  virtual float GetBinContent(unsigned int x1, unsigned int x1axis, 
                              unsigned int x2, unsigned int x2axis,
                              unsigned int x3, unsigned int x3axis,
                              unsigned int x4, unsigned int x4axis,
                              unsigned int x5, unsigned int x5axis,
                              unsigned int x6, unsigned int x6axis,
                              unsigned int x7, unsigned int x7axis) const;
  virtual float GetBinArea(unsigned int x1, unsigned int x2, 
                           unsigned int x3, unsigned int x4, 
                           unsigned int x5, unsigned int x6, 
                           unsigned int x7) const;
  virtual float Get(float x1, float x2, float x3, float x4, float x5, float x6, float x7) const;
  virtual float GetInterpolated(float x1, float x2, float x3, 
                                float x4, float x5, float x6, float x7, 
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

  virtual bool Write(MString FileName, bool Stream = false);

  void Show(float x1, float x2, float x3, float x4, float x5, float x6, float x7, bool Normalized = true);

  // protected methods:
 protected:


  // private methods:
 private:
  //! Read the specific data of this class - the main file handling is done in the base class!
  virtual bool ReadSpecific(MFileResponse& Parser, const MString& Type, const int Version, const bool MultiThreaded = false);



  // protected members:
 protected:


  // private members:
 private:
  MString m_NameAxisO7;
  vector<float> m_AxisO7;
  vector<MResponseMatrixO6> m_AxesO6;
  

  friend ostream& operator<<(ostream& os, const MResponseMatrixO7& R);


#ifdef ___CLING___
 public:
  ClassDef(MResponseMatrixO7, 1) // response matrix of order 7
#endif

};

ostream& operator<<(ostream& os, const MResponseMatrixO7& R);

#endif


////////////////////////////////////////////////////////////////////////////////
