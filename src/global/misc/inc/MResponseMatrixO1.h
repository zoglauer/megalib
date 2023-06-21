/*
 * MResponseMatrixO1.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MResponseMatrixO1__
#define __MResponseMatrixO1__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <iostream>
#include <vector>
#include <functional>
using namespace std;

// MEGAlib libs:
#include "MGlobal.h"
#include "MResponseMatrixOx.h"

// ROOT libs:
#include <TGraph.h>

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


template< class T > class greater_than : public unary_function< T, bool > 
{
 public:
  explicit greater_than( const T& x ) : arg2( x ) { }
  bool operator() ( const T& x ) const { return x > arg2; }

 private:
  T arg2;
};


////////////////////////////////////////////////////////////////////////////////


class MResponseMatrixO1 : public MResponseMatrixOx
{
  // public interface:
 public:
  MResponseMatrixO1();
  MResponseMatrixO1(vector<float> xDim);
  MResponseMatrixO1(MString Name, vector<float> xDim);
  virtual ~MResponseMatrixO1();

  void Init();

  bool operator==(const MResponseMatrixO1& ResponseMatrixO1);  
  MResponseMatrixO1& operator+=(const MResponseMatrixO1& ResponseMatrixO1);  
  MResponseMatrixO1& operator-=(const MResponseMatrixO1& ResponseMatrixO1);  
  MResponseMatrixO1& operator/=(const MResponseMatrixO1& ResponseMatrixO1);  

  MResponseMatrixO1& operator+=(const float& Value);  
  MResponseMatrixO1& operator*=(const float& Value);  

  void SetAxis(vector<float> xDim);

  void SetAxisNames(MString xName);
  MString GetAxisName(unsigned int order = 1) const;

  void SetBinContent(unsigned int x1, float Value = 1);
  void Set(float x1, float Value = 1);
  void Add(float x1, float Value = 1);

  virtual unsigned long GetNBins() const;

  virtual float GetAxisContent(unsigned int b, unsigned int order = 1) const;
  virtual vector<float> GetAxis(unsigned int order = 1) const;
  virtual unsigned int GetAxisBins(unsigned int order = 1) const;
  virtual float GetAxisBinCenter(unsigned int b, unsigned int order = 1) const;
  virtual unsigned int GetAxisBin(float v, unsigned int order = 1) const;
  virtual float GetAxisMinimum(unsigned int order = 1) const;
  virtual float GetAxisMaximum(unsigned int order = 1) const;
  virtual float GetAxisLowEdge(unsigned int b, unsigned int order = 1) const;
  virtual float GetAxisHighEdge(unsigned int b, unsigned int order = 1) const;

  virtual float GetBinContent(unsigned int x1) const;
  virtual float GetBinArea(unsigned int x1) const;
  virtual float Get(float x1) const;
  virtual float GetInterpolated(float x1, bool DoExtrapolate = false) const;

  virtual float GetMaximum() const;
  virtual float GetMinimum() const;
  virtual double GetSum() const;
  virtual MResponseMatrixO1 GetSumMatrixO1(unsigned int order) const;

  virtual bool Write(MString FileName, bool Stream = false);

  virtual void Smooth(unsigned int Times = 1);

  //! Return a ROOT histogram
  TH1* GetHistogram(bool Normalized = true);
  //! Show as an image
  void Show(bool Normalized = true);
  TGraph* GenerateGraph();

  // protected methods:
 protected:
 

  // private methods:
 private:
  //! Read the specific data of this class - the main file handling is done in the base class!
  virtual bool ReadSpecific(MFileResponse& Parser, const MString& Type, const int Version, const bool MultiThreaded = false);


  // protected members:
 protected:
  //! The name of the axis
  MString m_NameAxisO1;
  //! The axis bin values
  vector<float> m_AxisO1;
  //! The data
  vector<float> m_Values;


  // private members:
 private:
  friend ostream& operator<<(ostream& os, const MResponseMatrixO1& R);


#ifdef ___CLING___
 public:
  ClassDef(MResponseMatrixO1, 1) // response matrix of order 1 (linear)
#endif

};

ostream& operator<<(ostream& os, const MResponseMatrixO1& R);

#endif


////////////////////////////////////////////////////////////////////////////////
