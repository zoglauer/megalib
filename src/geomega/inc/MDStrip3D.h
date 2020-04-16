/*
 * MDStrip3D.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MDStrip3D__
#define __MDStrip3D__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MDStrip2D.h"
#include "MStreams.h"
#include "MString.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! A strip detector with depth resolution (such as the COSI detectors)
class MDStrip3D : public MDStrip2D
{
  // public interface:
 public:
  MDStrip3D(MString Name = "");
  MDStrip3D(const MDStrip3D& S);
  virtual ~MDStrip3D();

  virtual MDDetector* Clone();

  //! Copy data to named detectors
  virtual bool CopyDataToNamedDetectors();

  
  //! Noise the hit
  virtual void Noise(MVector& Pos, double& Energy, double& Time, MString& Flags, MDVolume* Volume) const;

  //! Grid a hit, here: split due to charge transport
  virtual vector<MDGridPoint> Grid(const MVector& Pos, const double& Energy, const double& Time, const MDVolume* Volume) const;
  //! Return the Grid point of this position
  virtual MDGridPoint GetGridPoint(const MVector& Pos) const;
  //! Return a position in detector volume coordinates
  virtual MVector GetPositionInDetectorVolume(const unsigned int xGrid, 
                                              const unsigned int yGrid,
                                              const unsigned int zGrid,
                                              const MVector PositionInGrid,
                                              const unsigned int Type,
                                              const MDVolume* Volume) const;
  virtual MVector GetPositionResolution(const MVector& Pos, const double Energy) const;
  
  //! Set a threshold in the depth resolution: Below this value, no depth resolution exists
  virtual bool SetDepthResolutionThreshold(const double DepthResolutionThreshold);
  //! Set the depth resolution for a certain energy
  virtual bool SetDepthResolutionAt(const double Energy, const double Resolution, const double Sigma);

  //! Add an depth correction factor to the energy resolution
  virtual bool SetEnergyResolutionDepthCorrection(const double InputDepth, 
                                                  const double PeakEnergy1, 
                                                  const double Width1, 
                                                  const double PeakEnergy2 = g_DoubleNotDefined, 
                                                  const double Width2 = g_DoubleNotDefined, 
                                                  const double Ratio = g_DoubleNotDefined);
  //! Return the depth correction factor to the energy resolution
  //double GetEnergyResolutionDepthCorrection(const double Depth) const;
  //! Return true if this detector has a depth dependent energy resolution:
  bool HasEnergyResolutionDepthCorrection() const;
  //! Return the energy resolution at an certain energy, including potential depth corrections
  virtual double GetEnergyResolution(const double Energy, const MVector& Position) const;

  virtual double GetEnergyResolutionPeak1(const double Energy, const MVector& PositionInDetector = c_NullVector) const;
  virtual double GetEnergyResolutionWidth1(const double Energy, const MVector& PositionInDetector = c_NullVector) const;
  virtual double GetEnergyResolutionPeak2(const double Energy, const MVector& PositionInDetector = c_NullVector) const;
  virtual double GetEnergyResolutionWidth2(const double Energy, const MVector& PositionInDetector = c_NullVector) const;
  virtual double GetEnergyResolutionRatio(const double Energy, const MVector& PositionInDetector = c_NullVector) const;

  //! Add an depth correction factor to the trigger threshold
  bool SetTriggerThresholdDepthCorrection(const double Depth, const double Correction);
  //! Return the depth correction factor to the trigger threshold
  double GetTriggerThresholdDepthCorrection(const double Depth) const;
  //! Return true if this detector has a depth dependent trigger threshold:
  bool HasTriggerThresholdDepthCorrection() const;
  //! Return the trigger threshold including potential depth corrections
  virtual double GetTriggerThreshold(const MVector& Position) const;
  //! In ANY case the real trigger threshold is below this value...
  virtual double GetSecureUpperLimitTriggerThreshold() const;

  //! Add an depth correction factor to the noise threshold
  bool SetNoiseThresholdDepthCorrection(const double Depth, const double Correction);
  //! Return the depth correction factor to the noise threshold
  double GetNoiseThresholdDepthCorrection(const double Depth) const;
  //! Return true if this detector has a depth dependent noise threshold:
  bool HasNoiseThresholdDepthCorrection() const;
  //! Return the noise threshold at an certain energy, including potential depth corrections
  virtual double GetNoiseThreshold(const MVector& Position) const;


  //! Set the drift constant in the field
  void SetDriftConstant(const double DriftConstant) { m_DriftConstant = DriftConstant; }
  //! Get the drift constant in the field
  double GetDriftConstant() const { return m_DriftConstant; }
  //! Set the energy per drifting particle
  void SetEnergyPerElectron(const double EnergyPerElectron) { m_EnergyPerElectron = EnergyPerElectron; }
  //! Get the energy per drifting particle
  double GetEnergyPerElectron() const { return m_EnergyPerElectron; }
  
  
  virtual MString GetGeomega() const;
  virtual MString ToString() const;

  //! Check if all input is reasonable
  virtual bool Validate();


  // protected methods:
 protected:


  // private methods:
 private:



  // protected members:
 protected:
  //! The type of the depth resolution 
  int m_DepthResolutionType;
  //! The energy dependent depth resolution
  MFunction m_DepthResolution; 
  //! The energy dependednt depth resolution one sigma variation
  MFunction m_DepthResolutionSigma;
  
  //! An energy threshold below which we don't have a depth resolution
  double m_DepthResolutionThreshold;

  
  //! Flag indicating we have a depth correction to the energy resolution
  bool m_EnergyResolutionDepthCorrectionSet;
  
  //! Some strip detectors, such as CZT have a depth dependent energy resolution
  //! This additional factor is encoded here
  MFunction m_EnergyResolutionDepthCorrectionPeak1; 
  MFunction m_EnergyResolutionDepthCorrectionWidth1; 
  MFunction m_EnergyResolutionDepthCorrectionPeak2; 
  MFunction m_EnergyResolutionDepthCorrectionWidth2; 
  MFunction m_EnergyResolutionDepthCorrectionRatio; 

  

  //! Flag indicating that a depth dependent trigger threshold has been set
  bool m_TriggerThresholdDepthCorrectionSet; 
  //! Some strip detectors, such as CZT have a depth dependent trigger threshold - this is the correction function
  MFunction m_TriggerThresholdDepthCorrection; 
  //! Flag indicating that a depth dependent noise threshold has been set
  bool m_NoiseThresholdDepthCorrectionSet; 
  //! Some strip detectors, such as CZT have a depth dependent noise threshold - this is the correction function
  MFunction m_NoiseThresholdDepthCorrection; 
  
  
  //! Constant describing the opening cone of the charge drift: c * sqrt(Drift Length)
  double m_DriftConstant;
  //! Energy per drifting electron/hole
  double m_EnergyPerElectron;

  // private members:
 private:



#ifdef ___CLING___
 public:
  ClassDef(MDStrip3D, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
