/*
 * MPhysicalEvent.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MPhysicalEvent__
#define __MPhysicalEvent__


////////////////////////////////////////////////////////////////////////////////

// Standard libs::
#include <fstream>
#include <string>
using namespace std;

// ROOT libs:
#include <TROOT.h>
#include <TRotMatrix.h>
#include <MString.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MTime.h"
#include "MVector.h"
#include "MStreams.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MPhysicalEvent
{
  // public interface:
 public:
  MPhysicalEvent();
  virtual ~MPhysicalEvent();

  //! Assimilate this event --- only works if the types are identical
  virtual bool Assimilate(MPhysicalEvent* E);
  //! Create a copy of this event
  virtual MPhysicalEvent* Duplicate();
  //! Stream the content to the given file-stream
  virtual bool Stream(fstream& Stream, int Version, bool Read, bool Fast = false, bool ReadDelayed = false);
  //! Parse the content of the stream
  virtual int ParseLine(const char* Line, bool Fast = false);
  //! Parse the content of the stream
  virtual bool ParseDelayed(bool Fast = false);

  //! Validate the event and calculate all high level data...
  virtual bool Validate() { return false; };

  virtual void Reset();

  // Remove this (name contains "event")
  int GetEventType() const;
  int GetType() const { return GetEventType(); };

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

  //! Return the pointing (z-axis) - longitude component
  double GetGalacticPointingZAxisLongitude() const { return m_GalacticPointingZAxis.Phi(); }      
  //! Return the pointing (z-axis) - latitude component 
  double GetGalacticPointingZAxisLatitude() const { return m_GalacticPointingZAxis.Theta() - 0.5*c_Pi; }      

  //! Return the detector rotation as rotation matrix
  TMatrix GetGalacticPointingRotationMatrix() const;


  //! Return if we have a detector rotation
  bool HasDetectorRotation() const { return m_HasDetectorRotation; }

  //! Set the x-axis of the detector coordinate system
  void SetDetectorRotationXAxis(const MVector Rot);
  //! Set the z-axis of the detector coordinate system
  void SetDetectorRotationZAxis(const MVector Rot);
  
  //! Get the x-axis of the detector coordinate system
  MVector GetDetectorRotationXAxis() const;
  //! Get the z-axis of the detector coordinate system
  MVector GetDetectorRotationZAxis() const;

  //! Return the detector rotation as rotation matrix
  TMatrix GetDetectorRotationMatrix() const;

  
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
  TMatrix GetHorizonPointingRotationMatrix() const;

  
  void SetTime(const MTime Time) { m_Time = Time; }
  MTime GetTime() const { return m_Time; }

  void SetId(const unsigned int Id) { m_Id = Id; }
  unsigned int GetId() const { return m_Id; };

  void SetTimeWalk(const int TimeWalk) { m_TimeWalk = TimeWalk; }
  int GetTimeWalk() const { return m_TimeWalk; }

  //! Get the total energy of the event
  virtual double GetEnergy() const;
  virtual double Ei() const { return GetEnergy(); }
  //! Get the position of the event - this is the location of the initial interaction!
  virtual MVector GetPosition() const;
  //! Get the origin direction of the event - if it has none return g_VectorNotDefined
  //! In detector coordinates - this is the reverse travel direction!
  virtual MVector GetOrigin() const;

  virtual bool IsGoodEvent() const { return m_IsGoodEvent; }
  virtual bool AllHitsGood() const { return m_AllHitsGood; }
  virtual void SetAllHitsGood(bool Flag) { m_AllHitsGood = Flag; }

  //! Set a flag indicating that this event originates from a decay
  void SetDecay(const bool Flag) { m_Decay = Flag; }
  //! Return true if this event originated from a decay
  bool IsDecay() const { return m_Decay; }

  //! Set a flag indicating that this event is bad
  void SetBad(const bool Flag, const MString BadString = "") { m_Bad = Flag; if (Flag == false) m_BadString = ""; else m_BadString = BadString; }
  //! Return true the if this event is bad
  bool IsBad() const { return m_Bad; }
  //! Return a string indicating why this event is bad
  MString GetBadString() const { return m_BadString; }

  //! Set the OI information
  void SetOIInformation(const MVector Position, const MVector Direction) { m_OIPosition = Position; m_OIDirection = Direction; }
  //! Get the OI position information
  MVector GetOIPosition() const { return m_OIPosition; }
  //! Get the OI direction information
  MVector GetOIDirection() const { return m_OIDirection; }

  virtual MString ToString() const;

  static const int c_Unknown;
  static const int c_Compton;
  static const int c_Pair;
  static const int c_Muon;
  static const int c_Shower;
  static const int c_Photo;
  static const int c_Decay;
  static const int c_Unidentifiable;


  // protected methods:
 protected:


  // private methods:
 private:


  // public members:
 public:


  // protected members:
 protected:
  //! Type of the event 
  int m_EventType;

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

  //! The time this event occurred
  MTime m_Time;

  //! The event ID
  unsigned int m_Id;

  // True if this event has been passed all tests..
  bool m_IsGoodEvent; 
  //! To be removed...
  bool m_AllHitsGood;

  //! True if this event has a decay flag
  bool m_Decay;
  //! True if this event has be qualified as bad event
  bool m_Bad;
  //! String giving the reason this event is qualified as bad
  MString m_BadString;

  //! The time walk between D1 and D2 in the MEGA detector
  int m_TimeWalk;

  //! OI Position (the meaning is a secret...)
  MVector m_OIPosition;
  //! OI Direction (the meaning is a secret...)
  MVector m_OIDirection;

  //! Store the read lines for delayed parsing
  vector<string> m_Lines; // string fatser than MString, vector faster than list

  // private members:
 private:



#ifdef ___CINT___
 public:
  ClassDef(MPhysicalEvent, 1)   // base class for compton and pair events
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
