/*
 * MBackprojectionNearFieldSphericSpheric.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MBackprojectionNearFieldSpheric__
#define __MBackprojectionNearFieldSpheric__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include "TObject.h"

// MEGAlib libs:
#include "MGlobal.h"
#include "MBackprojection.h"
#include "MPhysicalEvent.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////

//! This class handles backprojections on a near-field sphere
class MBackprojectionNearFieldSpheric : public MBackprojection
{
  // public interface:
 public:
  //! Default constructor
  MBackprojectionNearFieldSpheric(MCoordinateSystem CoordinateSystem = MCoordinateSystem::c_Cartesian2D);
  //! Default destructor
  virtual ~MBackprojectionNearFieldSpheric();

  //! Prepare all backprojections - must be called before the backprojections
  virtual void PrepareBackprojection();
  //! The entry point into the backprojection
  virtual bool Backproject(MPhysicalEvent* Event, double* Image, int* Bins, int& NUsedBins, double& Maximum);

  //! Set the viewport, i.e. image dimensions
  //! Here: X = Phi, Y = Theta, Z = Radiuas
  virtual bool SetDimensions(double x1Min, double x1Max, unsigned int x1NBins, 
                             double x2Min, double x2Max, unsigned int x2NBins,
                             double x3Min = 0, double x3Max = 0, unsigned int x3NBins = 1,
                             MVector x1Axis = MVector(1.0, 0.0, 0.0), 
                             MVector x3Axis = MVector(0.0, 0.0, 1.0));

  // protected methods:
 protected:
  //! Assimilate the event data - stores parts of the event data in the class for acceleration pruposes 
  virtual bool Assimilate(MPhysicalEvent* Event);

  //! Calculate the origin probabilities for a Compton event
  bool BackprojectionCompton(double* Image, int* Bins, int& NUsedBins, double& Maximum);
  //! Calculate the origin probabilities for a Pair event
  bool BackprojectionPair(double* Image, int* Bins, int& NUsedBins, double& Maximum);
  //! Calculate the origin probabilities for a PET event
  bool BackprojectionPET(double* Image, int* Bins, int& NUsedBins, double& Maximum);
  //! Calculate the origin probabilities for a multi-event event
  bool BackprojectionMulti(double* Image, int* Bins, int& NUsedBins, double& Maximum);
  
  virtual void Rotate(double &x, double &y, double &z);
  virtual void RotateAndTranslate(double &x, double &y, double &z);



  // private methods:
 private:



  // protected members:
 protected:

  // Volume oif each bin
  double* m_VolumeBin;

  // x-Position in detector coordinates of the center of the bin
  double* m_xBin;
  // y-Position in detector coordinates of the center of the bin
  double* m_yBin;
  // z-Position in detector coordinates of the center of the bin
  double* m_zBin;


  // private members:
 private:



#ifdef ___CLING___
 public:
  ClassDef(MBackprojectionNearFieldSpheric, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
