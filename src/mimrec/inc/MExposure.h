/*
 * MExposure.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MExposure__
#define __MExposure__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MRotation.h"
#include "MViewPort.h"
#include "MExposureMode.h"
#include "MResponseMatrixO2.h"
#include "MPhysicalEvent.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MExposure : public MViewPort
{
  // public interface:
 public:
  //! Default constructor
  MExposure();
  //! Default destructor
  virtual ~MExposure();

  //! Get the current exposure mode
  MExposureMode GetMode() { return m_Mode; }
  
  //! Set the efficiency file and switch to that mode
  bool SetEfficiencyFile(MString EfficiencyFile);
  
  //! Set the viewport / image dimensions
  virtual bool SetDimensions(double xMin, double xMax, unsigned int xNBins, 
                             double yMin, double yMax, unsigned int yNBins, 
                             double zMin = 0, double zMax = 0, unsigned int zNBins = 0,
                             MVector xAxis = MVector(1.0, 0.0, 0.0), MVector zAxis = MVector(0.0, 0.0, 1.0));

  //! Create the exposure for one event
  virtual bool Expose(const MPhysicalEvent* Event);
  
  //! Return a copy of the current exposure map. Unit: cm2 * sec / sr
  //! User must delete array via "delete [] ..."
  virtual double* GetExposure();
  
  
  // protected methods:
 protected:
  //! Apply the latest exposure to the exposure map
  bool ApplyExposure();
  //! A distance metric to determine when we have to recalculate
  //! The value is just the largest difference between the axis vectors
  double DistanceMetric(const MRotation& A, const MRotation& B);

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  //! The exposure mode
  MExposureMode m_Mode;
   
  //! The exposure image
  double* m_Exposure;

  //! The last applied rotation
  MRotation m_LastRotation;
  //! The last applied time
  MTime m_LastTime;

  //! The current rotation -- the exposure of that one has not yet been applied to the map!
  MRotation m_CurrentRotation;
  //! The current time -- the exposure of that one has not yet been applied to the map!
  MTime m_CurrentTime;
  
  // Mode: Calculated by efficiency
  
  //! The efficiency response file for mode: CalculateFromEfficiency
  MResponseMatrixO2 m_Efficiency;
  //! The rotation of the efficiency matrix for mode: CalculateFromEfficiency
  MRotation m_EfficiencyRotation;
  //! The simulation start area for mode: CalculateFromEfficiency
  double m_EfficiencyStartArea;
  //! The number of simulated events for mode: CalculateFromEfficiency
  long m_EfficiencySimulatedEvents;
  
  //! Counter: How many times did we add something
  long m_NExposureUpdates;
  
#ifdef ___CINT___
 public:
  ClassDef(MExposure, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
