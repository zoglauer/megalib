/*
 * MPairEvent.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MPairEvent__
#define __MPairEvent__


////////////////////////////////////////////////////////////////////////////////


// standard libs
#include <iostream>
using namespace std;

// ROOT libs

// MEGAlib libs:
#include "MGlobal.h"
#include "MCoordinateSystem.h"
#include "MPhysicalEvent.h"


////////////////////////////////////////////////////////////////////////////////


class MPairEvent : public MPhysicalEvent
{
  // Public Interface:
 public:
  MPairEvent();
  virtual ~MPairEvent();

  virtual void Reset();

  bool Assimilate(MPairEvent* PairEvent);
  bool Assimilate(MPhysicalEvent* Event);
  bool Assimilate(char* LineBuffer);
  virtual bool Stream(MFile& File, int Version, bool Read, bool Fast = false, bool ReadDelayed = false);
  virtual int ParseLine(const char* Line, bool Fast = false);
  //! Create a copy of this event
  virtual MPhysicalEvent* Duplicate();

  //! Validate the event and calculate all high level data...
  virtual bool Validate();

  MVector GetPairCreationIA() const;
  void SetPairCreationIA(const MVector& PCIA);

  MVector GetElectronDirection() const;
  void SetElectronDirection(const MVector& ED);

  MVector GetPositronDirection() const;
  void SetPositronDirection(const MVector& PD);

  double GetEnergyElectron() const;
  void SetEnergyElectron(const double Energy);
  double GetEnergyErrorElectron() const;
  void SetEnergyErrorElectron(const double Energy);

  double GetEnergyPositron() const;
  void SetEnergyPositron(const double Energy);
  double GetEnergyErrorPositron() const;
  void SetEnergyErrorPositron(const double Energy);

  double GetInitialEnergyDeposit() const;
  void SetInitialEnergyDeposit(const double Energy);

  //! Set the quality factor of the combined electron track
  void SetTrackQualityFactor(const double TrackQualityFactor) { m_TrackQualityFactor = TrackQualityFactor; }
  double GetTrackQualityFactor() { return m_TrackQualityFactor; }

  double GetOpeningAngle() const;
  
  MPhysicalEvent* Data() { return (MPhysicalEvent *) this; } 

  virtual double GetEnergy() const;
  inline double Ei() const { return GetEnergy(); }

  //! Get the position of the event - this is the location of the initial interaction!
  virtual MVector GetPosition() const { return GetPairCreationIA(); }
  //! Get the origin direction of the event - if it has none return g_VectorNotDefined
  //! In detector coordinates - this is the reverse travel direction!
  virtual MVector GetOrigin() const { return m_IncomingGammaDirection; }


  MString ToBasicString();

  bool MostProbableDirectionIncomingGamma();

  //! Return the angular resolution measure value for the given test position in the given coordinate system
  double GetARMGamma(const MVector& Position, const MCoordinateSystem& CS = MCoordinateSystem::c_Cartesian2D);
  //! Return the azimuthal scatter angle value for the given test position in the given coordinate system
  double GetAzimuthalScatterAngle(const MVector& Position, const MCoordinateSystem& CS = MCoordinateSystem::c_Cartesian2D);


  // protected methods:
 protected:


  // private methods:
 private:

 public:
  MVector m_IncomingGammaDirection;

  // protected members:
 protected:


  // private members:
 private:
  static const double m_RestEnergyElectron;

  MVector m_PairCreationIA;         // position of the pair interaction
  MVector m_ElectronDirection;      // direction of the electron
  MVector m_PositronDirection;      // direction of the positron

  double m_EnergyElectron;         // energy of the first pair electron 
  double m_EnergyPositron;         // energy of the second pair electron 

  double m_EnergyErrorElectron;    // energy error of the first pair electron 
  double m_EnergyErrorPositron;    // energy error of the second pair electron 

  double m_InitialEnergyDeposit;

  double m_TrackQualityFactor;

  bool m_EPDistinguishable;        // true if elctron and positron can be distiguished


#ifdef ___CLING___
 public:
  ClassDef(MPairEvent, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
