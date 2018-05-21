/*
 * MREAM.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MREAM__
#define __MREAM__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <sstream>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MDVolumeSequence.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MREAM
{
  // public interface:
 public:
  //! Constructor of Raw Event Additional Measurement
  MREAM();
  //! Copy constructor of Raw Event Additional Measurement
  MREAM(const MREAM& REAM);
  //! Destructor of Raw Event Additional Measurement
  virtual ~MREAM();

  //! Default assignment constructor
  const MREAM& operator=(const MREAM&);

  //! In almost all cases when we need a replica of this object we do not know what it is -
  //! Thus we need a general duplication mechanism
  virtual MREAM* Clone();

  //! Return the type of the additional measurement
  unsigned int GetType() const { return m_Type; }

  //! Set the volume sequence (this object becomes owner of the sequence!)
  void SetVolumeSequence(MDVolumeSequence* S) { m_VolumeSequence = S; }
  //! Return the volume sequence (this object stays owner of the sequence!)
  MDVolumeSequence* GetVolumeSequence() { return m_VolumeSequence; }

  //! Parse an input file line
  virtual int ParseLine(const char* Line, int Version) { return 2; } // To be changed to a constant of MParser? 

  //! Dump the essential content of this REAM
  virtual MString ToString(int Level = 0);
  //! Convert to a string in the evta file
  virtual MString ToEvtaString(const int Precision, const int Version = 1) { return MString(""); }

  // The different types of additional measurements:
  //! Type of additional measurement unknown
  static const unsigned int c_Unknown;
  //! Total energy deposit in the drift chamber
  static const unsigned int c_DriftChamberEnergy;
  //! Total energy deposit in a guard ring
  static const unsigned int c_GuardRingHit;
  //! Direction of a charged particle as measured by a Strip3DDirectional detector
  static const unsigned int c_Directional;
  //! OD information
  static const unsigned int c_StartInformation;
  //! Lowest ID of available additional measurement (for loops)
  static const unsigned int c_Min;
  //! Highest ID of available additional measurement (for loops)
  static const unsigned int c_Max;

  // protected methods:
 protected:

  // private methods:
 private:



  // protected members:
 protected:
  //! The type - in case we want to cast
  unsigned int m_Type;

  //! The obligatory volume sequence
  MDVolumeSequence* m_VolumeSequence;


  // private members:
 private:


#ifdef ___CLING___
 public:
  ClassDef(MREAM, 0) // Root stuff...
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
