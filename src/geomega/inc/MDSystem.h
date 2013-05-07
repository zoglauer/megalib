/*
 * MDSystem.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MDSystem__
#define __MDSystem__


////////////////////////////////////////////////////////////////////////////////


// Standard libs::
#include <vector>
using namespace std;

// ROOT libs:
#include <TROOT.h>
#include <MString.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MTime.h"
#include "MDVolumeSequence.h"

// Forward declarations:
class MDDetector;

////////////////////////////////////////////////////////////////////////////////


class MDSystem
{
  // public interface:
 public:
  MDSystem(const MString& Name);
  virtual ~MDSystem();

  virtual MString GetName() const;

  void Reset();

  virtual bool Validate();

  //! Noise the time
  void NoiseTime(MTime& Time);

  //! Set the energy resolution type, return false if you try to overwrite an existing type
  virtual bool SetTimeResolutionType(const int TimeResolutionType);
  //! Return the energy resolution type
  virtual double GetTimeResolutionType() const { return m_TimeResolutionType; }

  virtual void SetTimeResolutionGaussSigma(const double Time);
  virtual double GetTimeResolutionGaussSigma() const { return m_TimeResolutionGaussSigma; }

  virtual MString GetGeomega() const;
  virtual MString ToString() const;

  static const int c_TimeResolutionTypeUnknown;
  static const int c_TimeResolutionTypeIdeal;
  static const int c_TimeResolutionTypeGauss;

  // protected methods:
 protected:


  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  //! Name of the trigger 
  MString m_Name;

  //! The type of time resolution
  int m_TimeResolutionType;
  //! The Gaussian sigma of the time resolutionm
  double m_TimeResolutionGaussSigma;


  // private members:
 private:
  friend ostream& operator<<(ostream& os, const MDSystem& T);


#ifdef ___CINT___
 public:
  ClassDef(MDSystem, 0) // Represents the system
#endif

};

ostream& operator<<(ostream& os, const MDSystem& T);

#endif


////////////////////////////////////////////////////////////////////////////////
