/*
 * MEarthHorizon.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MEarthHorizon__
#define __MEarthHorizon__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <sstream>
#include <iostream>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MVector.h"
#include "MResponseMatrixO2.h"
#include "MPhysicalEvent.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MEarthHorizon
{
  // public interface:
 public:
  //! Default constructor - test is set to "no test", Earth is in nadir, horizon at 90 deg
  MEarthHorizon();
  //! Copy constructor 
  MEarthHorizon(const MEarthHorizon& EarthHorizon);
  //! Default destructor
  virtual ~MEarthHorizon();
  //! Default assignment constructor
  const MEarthHorizon& operator=(const MEarthHorizon&);

  //! Do not apply any earth horizon tests
  bool SetNoTest();
  //! Reject all events whose cone is below or intersects the horizon
  bool SetIntersectionTest();
  //! Reject all events who have a certain probability to come from earth.  
  bool SetProbabilityTest(const double MaxProbability,
                          const MString ComptonResponseFileName,
                          const MString PairResponseFileName = g_StringNotDefined);  

  //! Set the position of earth in detector coordinates and 
  //! the azimuthal angle of the horizon realtive to this position
  bool SetEarthHorizon(const MVector& PositionEarth, 
                       const double HorizonAngle);

  //! Return true if the event come from earth
  bool IsEventFromEarth(MPhysicalEvent* Event, bool DumpOutput = false) const;

  static const int c_NoTest;
  static const int c_IntersectionTest;
  static const int c_ProbabilityTest;

  static const int c_Min;
  static const int c_Max;

  // protected methods:
 protected:
  //! Return true if the event come from earth via intersection test
  bool IsEventFromEarthByIntersectionTest(MPhysicalEvent* Event, bool DumpOutput = false) const;

  //! Return true if the event come from earth via probability test
  bool IsEventFromEarthByProbabilityTest(MPhysicalEvent* Event, bool DumpOutput = false) const;

  // private methods:
 private:
  friend ostream& operator<<(ostream& os, const MEarthHorizon& E);



  // protected members:
 protected:


  // private members:
 private:
  //! Position of earth (center) in detector coordinates
  MVector m_PositionEarth;
  //! (Azimuth-) Angle from the earth position to earth horizon
  double m_HorizonAngle;

  //! Type of the earth horizon test
  int m_TestType;

  //! Maximum allowed probability that the event came from earth
  double m_MaxProbability;
  //! The (smplified) response matrix for Compton events 
  MResponseMatrixO2 m_ComptonResponse;
  //! The (simplified) response matrix for Pair events
  MResponseMatrixO2 m_PairResponse;
  //! True if we have a Compton response
  bool m_ValidComptonResponse;
  //! True if we have a Pair response
  bool m_ValidPairResponse;

#ifdef ___CINT___
 public:
  ClassDef(MEarthHorizon, 0) // no description
#endif

};

ostream& operator<<(ostream& os, const MEarthHorizon& E);

#endif


////////////////////////////////////////////////////////////////////////////////
