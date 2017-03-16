/*
 * MEfficiency.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MEfficiency__
#define __MEfficiency__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MString.h"
#include "MRotation.h"
#include "MResponseMatrixO2.h"
#include "MResponseMatrixO4.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! A dummy definition of a class
class MEfficiency
{
  // public interface:
 public:
  //! Default constructor
  MEfficiency();
  //! Default destuctor
  virtual ~MEfficiency();

  //! Load the data
  bool Load(MString EfficiencyFile);

  //! Return true if this is a far field efficiency (return false for near-field efficiency)
  bool IsFarField() { return m_IsFarField; }

  //! Get the data - theta and phi are in detector coordinates
  //! If no efficiency file has been loaded, return 1
  double Get(double theta, double Phi, bool Interpolate = true);

  //! Get the data in near field coordinates
  //! If no efficiency file has been loaded, return 1
  double GetNearField(double x, double y, double z, bool Interpolate = true);


  // protected methods:
 protected:
  //! Load the near-field file
  bool LoadNearField(MString EfficiencyFile);
  //! Load the far-field file
  bool LoadFarField(MString EfficiencyFile);


  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  //! Flag indicating that the efficiency file has been loaded succesfully
  bool m_IsLoaded;

  //! Flag indicating that the loaded file is far-field od near-field
  bool m_IsFarField;

  //! The efficiency response file for mode: CalculateFromEfficiency
  MResponseMatrixO2 m_Efficiency;
  //! The rotation of the efficiency matrix for mode: CalculateFromEfficiency
  MRotation m_EfficiencyRotation;

  //! The efficiency in near field coordinates
  MResponseMatrixO4 m_EfficiencyNearField;

  //! The simulation start area for mode: CalculateFromEfficiency
  double m_EfficiencyStartArea;
  //! The number of simulated events for mode: CalculateFromEfficiency
  long m_EfficiencySimulatedEvents;


#ifdef ___CINT___
 public:
  ClassDef(MEfficiency, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
