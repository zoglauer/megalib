/*
 * MRotationInterface.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MRotationInterface__
#define __MRotationInterface__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MVector.h"
#include "MRotation.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! The detector rotation interface for all event types
class MRotationInterface
{
  // public interface:
 public:
  //! Default constructor
  MRotationInterface();
  //! Default destuctor
  virtual ~MRotationInterface();

  //! Reset to default values
  void Reset();

  //! Check if the data is OK
  bool Validate();

  //! Set all data
  void Set(MRotationInterface& RO);

  //! Parse a line
  //! Warning: This is for fast parsing: No checks are performed what-so-ever!
  //! Returns 0, if the line got correctly parsed
  //! Returns 1, if the line got not correctly parsed
  int ParseLine(const char* Line, bool Fast = false);

  //! Stream to a file
  //! Reading has to be done in the derived class
  void Stream(ostringstream& S);

  //! Set if you wish to store the coordinates in galactic coordinates
  void SetHasGalacticPointing(bool GalacticPointing = true) { m_HasGalacticPointing = GalacticPointing; }
  //! Return if we have galactic coodinates
  bool HasGalacticPointing() const { return m_HasGalacticPointing; }

  //! Set the x-axis of the galactic coordinate system - input is in degrees
  void SetGalacticPointingXAxis(const double Longitude, const double Latitude);
  //! Set the z-axis of the galactic coordinate system - input is in degrees
  void SetGalacticPointingZAxis(const double Longitude, const double Latitude);
  //! Set the x-axis of the galactic coordinate system
  void SetGalacticPointingXAxis(const MVector XAxis) { m_HasGalacticPointing = true; m_GalacticPointingXAxis = XAxis; }
  //! Set the z-axis of the galactic coordinate system
  void SetGalacticPointingZAxis(const MVector ZAxis) { m_HasGalacticPointing = true; m_GalacticPointingZAxis = ZAxis; }

  //! Get the x-axis of the galactic coordinate system - input is in degrees
  MVector GetGalacticPointingXAxis() const { return m_GalacticPointingXAxis; }
  //! Get the z-axis of the galactic coordinate system - input is in degrees
  MVector GetGalacticPointingZAxis() const { return m_GalacticPointingZAxis; }

  //! Return the pointing (x-axis) - longitude component in radians!
  double GetGalacticPointingXAxisLongitude() const { return (m_GalacticPointingXAxis.Phi() < 0) ? m_GalacticPointingXAxis.Phi() + c_TwoPi : m_GalacticPointingXAxis.Phi(); }
  //! Return the pointing (x-axis) - latitude component in radians!
  double GetGalacticPointingXAxisLatitude() const { return m_GalacticPointingXAxis.Theta() - 0.5*c_Pi; }

  //! Return the pointing (z-axis) - longitude component in radians!
  double GetGalacticPointingZAxisLongitude() const { return (m_GalacticPointingZAxis.Phi() < 0) ? m_GalacticPointingZAxis.Phi() + c_TwoPi : m_GalacticPointingZAxis.Phi(); }
  //! Return the pointing (z-axis) - latitude component in radians!
  double GetGalacticPointingZAxisLatitude() const { return m_GalacticPointingZAxis.Theta() - 0.5*c_Pi; }

  //! Return the Galactic rotation as rotation matrix
  MRotation GetGalacticPointingRotationMatrix();
  //! Return the Galactic rotation as rotation matrix
  MRotation GetGalacticPointingInverseRotationMatrix();



  //! Return if we have a detector rotation
  bool HasDetectorRotation() const { return m_HasDetectorRotation; }

  //! Set the x-axis of the detector coordinate system - input is in degrees
  void SetDetectorPointingXAxis(const double Phi, const double Theta);
  //! Set the z-axis of the detector coordinate system - input is in degrees
  void SetDetectorPointingZAxis(const double Phi, const double Theta);

  //! Set the x-axis of the detector coordinate system
  void SetDetectorRotationXAxis(const MVector Rot);
  //! Set the z-axis of the detector coordinate system
  void SetDetectorRotationZAxis(const MVector Rot);

  //! Get the x-axis of the detector coordinate system
  MVector GetDetectorRotationXAxis() const;
  //! Get the z-axis of the detector coordinate system
  MVector GetDetectorRotationZAxis() const;

  //! Return the detector rotation as rotation matrix
  MRotation GetDetectorRotationMatrix() const;
  //! Return the inverse detector rotation as rotation matrix
  MRotation GetDetectorInverseRotationMatrix() const;



  //! Return if we have a horizon pointing
  bool HasHorizonPointing() const { return m_HasHorizonPointing; }

  //! Set the x-axis of the horizon coordinate system - input is in degrees
  void SetHorizonPointingXAxis(const double AzimuthNorth, const double Elevation) { m_HasHorizonPointing = true; m_HorizonPointingXAxis.SetMagThetaPhi(1.0, (90-Elevation)*c_Rad, AzimuthNorth*c_Rad); }
  //! Set the z-axis of the horizon coordinate system - input is in degrees
  void SetHorizonPointingZAxis(const double AzimuthNorth, const double Elevation) { m_HasHorizonPointing = true; m_HorizonPointingZAxis.SetMagThetaPhi(1.0, (90-Elevation)*c_Rad, AzimuthNorth*c_Rad); }
  //! Set the x-axis of the horizon coordinate system
  void SetHorizonPointingXAxis(const MVector XAxis) { m_HasHorizonPointing = true; m_HorizonPointingXAxis = XAxis; }
  //! Set the z-axis of the horizon coordinate system
  void SetHorizonPointingZAxis(const MVector ZAxis) { m_HasHorizonPointing = true; m_HorizonPointingZAxis = ZAxis; }

  //! Get the x-axis of the horizon coordinate system
  MVector GetHorizonPointingXAxis() const { return m_HorizonPointingXAxis; }
  //! Get the z-axis of the horizon coordinate system
  MVector GetHorizonPointingZAxis() const { return m_HorizonPointingZAxis; }

  //! Get the rotation of the horizon coordinate system
  MRotation GetHorizonPointingRotationMatrix() const;


  // protected methods:
 protected:

  // private methods:
 private:



  // protected members:
 protected:
  //! The event ID
  long m_Id;

  //! If true, a pointing in galactic coordinates is present
  bool m_HasGalacticPointing;
  //! The rotation around the X axis
  MVector m_GalacticPointingXAxis;
  //! The rotation around the Z axis
  MVector m_GalacticPointingZAxis;

  //! If true, a detector rotation is present
  bool m_HasDetectorRotation;
  //! The rotation around the X axis
  MVector m_DetectorRotationXAxis;
  //! The rotation around the Z axis
  MVector m_DetectorRotationZAxis;

  //! If true, a horizon in detector coordinates is present
  bool m_HasHorizonPointing;
  //! Pointing of the detector in the horizon coordinate system - X axis
  MVector m_HorizonPointingXAxis;
  //! Pointing of the detector in the horizon coordinate system - Z axis
  MVector m_HorizonPointingZAxis;

  //! True if we already have calculated the Galactic pointing rotation
  bool m_IsGalacticPointingRotationCalculated;
  //! The Galactic pointing rotation - only filled if already calculated
  MRotation m_GalacticPointingRotation;

  //! True if we already have calculated the INVERSE Galactic pointing rotation
  bool m_IsGalacticPointingInverseRotationCalculated;
  //! The INVERSE Galactic pointing rotation - only filled if already calculated
  MRotation m_GalacticPointingInverseRotation;

  // private members:
 private:



#ifdef ___CLING___
 public:
  ClassDef(MRotationInterface, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
