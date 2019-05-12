/*
 * MDDriftChamber.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MDDriftChamber__
#define __MDDriftChamber__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <MString.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MDStrip3D.h"
#include "MStreams.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MDDriftChamber : public MDStrip3D
{
  // public interface:
 public:
  MDDriftChamber(MString Name = "");
  MDDriftChamber(const MDDriftChamber& D);
  virtual ~MDDriftChamber();

  virtual MDDetector* Clone();
  //! Copy data to named detectors
  virtual bool CopyDataToNamedDetectors();

  virtual void Noise(MVector& Pos, double& Energy, double& Time, MDVolume* Volume) const;
  virtual bool NoiseLightEnergy(double& Energy) const;

  //! Grid a hit, here: split due to charge transport
  virtual vector<MDGridPoint> Grid(const MVector& Pos, const double& Energy, const double& Time, const MDVolume* Volume) const;

  void SetLightSpeed(const double LightSpeed) { m_LightSpeed = LightSpeed; }
  double GetLightSpeed() const { return m_LightSpeed; }
  void SetLightDetectorPosition(const int Pos) { m_LightDetectorPosition = Pos; }
  int GetLightDetectorPosition() const { return m_LightDetectorPosition; }

  void SetLightEnergyResolution(const double Energy, const double Resolution);
  virtual double GetLightEnergyResolution(const double Energy) const;

  //! Return the travel time between the interaction position and the PMTs
  double GetLightTravelTime(const MVector& Position) const;


  virtual MString GetGeomega() const;
  virtual MString ToString() const;

  //! Check if all input is reasonable
  virtual bool Validate();

  
  static const int c_LightEnergyResolutionTypeUnknown;
  static const int c_LightEnergyResolutionTypeIdeal;
  static const int c_LightEnergyResolutionTypeGauss;

  
  // protected methods:
 protected:


  // private methods:
 private:



  // protected members:
 protected:
  

  // private members:
 private:
  //! Speed of (optical) light in the detector material
  double m_LightSpeed;
  //! Position of the light sensitive detector (1: +x; -1:-x; 2: +y; -2 -y; 3: +z; -3:-z)
  int m_LightDetectorPosition;

  //! The type of the light energy resolution (unknown, ideal, gauss)
  int m_LightEnergyResolutionType;
  //! Vector holding the energy resolution as one sigma in keV of the energy
  //! measured by the light
  MFunction m_LightEnergyResolution; 

#ifdef ___CLING___
 public:
  ClassDef(MDDriftChamber, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
