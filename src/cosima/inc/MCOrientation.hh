/*
 * MCOrientation.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MCOrientation__
#define __MCOrientation__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <vector>
using namespace std; 

// Geant4:
#include "G4SystemOfUnits.hh"
#include "G4ThreeVector.hh"

// ROOT libs:
#include "TRotMatrix.h"
#include "TGeoMatrix.h"

// MEGAlib libs:
#include "MGlobal.h"
#include "MRotation.h"
#include "MTokenizer.h"
#include "MDDebugInfo.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! This class represents all orientations in Cosima
enum class MCOrientationCoordinateSystem : int { 
  c_Local = 0, c_Galactic = 1 
};

//! I/O for MCOrientationCoordinateSystem
std::ostream& operator<<(std::ostream& os, MCOrientationCoordinateSystem C);


////////////////////////////////////////////////////////////////////////////////


//! This class represents all orientations in Cosima
class MCOrientation
{
  // public interface:
 public:
  //! Standard constructor
  //! The default constructs a local fixed orientation
  //! Axes defines the type: do I need two axes to define the orientation (e.g. for the detector and the sky, or just one axis, e.g. position in Galactic coordiantes)
  MCOrientation();
  //! Standard copy constructor
  //MCOrientation(const MCOrientation& Orientation);
  //! Default destructor
  virtual ~MCOrientation();
  
  //! Set everything to defaultm_ZPhiLongitude
  void Clear();
  
  //! True if we actually have an orientation
  bool IsOriented() const { return m_IsOriented; }
  
  //! True if we have looping data, i.e. periodically repeating movement patterns
  bool IsLooping() const { return m_IsLooping; }
  
  //! Return the coordiante system
  MCOrientationCoordinateSystem GetCoordinateSystem() const { return m_CoordianteSystem; }
  
  //! Parse some tokenized text
  bool Parse(const MTokenizer& Tokenizer);

  //! Get the orientation
  bool GetOrientation(double Time, double& XThetaLat, double& XPhiLong, double& ZThetaLat, double& ZPhiLong) const;
  
  //! Get the Earth coordinates
  bool GetEarthCoordinate(double Time, double& Alt, double& Lat, double& Long) const;
  
  //! Perform the orientation for the given time for a position and a direction from local to oriented coordiante system
  bool OrientPositionAndDirection(double Time, G4ThreeVector& Position, G4ThreeVector& Direction) const;  
  //! Perform the invers orientation for the given time for a position and a direction from oriented to local coordiante system
  bool OrientPositionAndDirectionInvers(double Time, G4ThreeVector& Position, G4ThreeVector& Direction) const;
  
  //! Perform the orientation for the given time for a direction (or polarization) from local to oriented coordiante system
  bool OrientDirection(double Time, G4ThreeVector& Direction) const;  
  //! Perform the invers orientation for the given time for a direction  (or polarization) from oriented to local coordiante system
  bool OrientDirectionInvers(double Time, G4ThreeVector& Direction) const;
  
  //! Return the start time or zero if there is none
  double GetStartTime() const;
  
  //! Return the stop time or zero if there is none
  double GetStopTime() const;
  
  //! Dump content into a string
  MString ToString() const;
  
  // protected methods:
 protected:
  //! Read a list of orientations from file
  bool Read(MString FileName);
   
  //! Check if Time is covered in the time array
  bool InRange(double Time) const;

  //! Find the closest index - always check with InRange() first to avoid exceptions
  unsigned int FindClosestIndex(double Time) const;

  
  // private methods:
 private:


  // protected members:
 protected:


  // private members:
 private:
  //! Name of the vector
  MString m_Name;
  
  //! The coordinate system
  MCOrientationCoordinateSystem m_CoordianteSystem;
  
  //! The times
  vector<double> m_Times;
  
  //! The x-Axis theta / latitude
  vector<double> m_XThetaLat;
  //! The x-Axis phi / longitude
  vector<double> m_XPhiLong;
  
  //! The z-Axis theta / latitude
  vector<double> m_ZThetaLat;
  //! The z-Axis phi / longitude
  vector<double> m_ZPhiLong;
  
  //! The Earth altitude
  vector<double> m_EarthAlt;
  //! The Earth  lat in galactic coordinate
  vector<double> m_EarthLat;
  //! The Earth  lat in galactic coordinate
  vector<double> m_EarthLong;
  
  //! The translation
  vector<MVector> m_Translations;
  
  //! The rotation matrix
  vector<MRotation> m_Rotations;
  //! The rotation matrix
  vector<MRotation> m_RotationsInvers;
  
  //! True if the sequence is looping (repeating) infinitely
  bool m_IsLooping;
  
  //! The last index used:
  unsigned int m_LastIndex;

  //! True if we actually have an orientation besides the default one
  bool m_IsOriented;

};

#endif


////////////////////////////////////////////////////////////////////////////////
