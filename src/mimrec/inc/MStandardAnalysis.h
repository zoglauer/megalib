/*
 * MStandardAnalysis.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MStandardAnalysis__
#define __MStandardAnalysis__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MVector.h"
#include "MEventSelector.h"
#include "MDGeometryQuest.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MStandardAnalysis
{
  // public interface:
 public:
  //! Default constructor
  MStandardAnalysis();
  //! Default destructor
  virtual ~MStandardAnalysis();

  //! Set the event selector
  void SetEventSelector(MEventSelector Selector) { m_Selector = Selector; }
  
  //! Set the geometry
  void SetGeometry(MDGeometryQuest* Geometry) { m_Geometry = Geometry; }
  
  //! Set the tra file name
  void SetFileName(MString FileName) { m_FileName = FileName; }

  //! Set the position (location of point source)
  void SetPosition(MVector Position) { m_Position = Position; }
  //! Set the energy (ideal energy of mono energetic beam)
  void SetEnergy(double Energy) { m_Energy = Energy; }


  //! Do all the analysis
  bool Analyze();

  // protected methods:
 protected:
  //MStandardAnalysis() {};
  //MStandardAnalysis(const MStandardAnalysis& StandardAnalysis) {};

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  //! The event selector
  MEventSelector m_Selector;

  //! The geometry 
  MDGeometryQuest* m_Geometry;

  //! Set the tra file name
  MString m_FileName;
  

  //! The position
  MVector m_Position;
  //! The energy
  double m_Energy;
  

#ifdef ___CINT___
 public:
  ClassDef(MStandardAnalysis, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
