/*
 * MBackprojectionNearField.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MBackprojectionNearField__
#define __MBackprojectionNearField__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include "TObject.h"

// MEGAlib libs:
#include "MGlobal.h"
#include "MBackprojection.h"
#include "MPhysicalEvent.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MBackprojectionNearField : public MBackprojection
{
  // public interface:
 public:
  //! Default constructor
  MBackprojectionNearField(MCoordinateSystem CoordinateSystem = MCoordinateSystem::c_Cartesian2D);
  //! Default destructor
  virtual ~MBackprojectionNearField();

  //! Prepare all backprojections - must be called before the backprojections
  virtual void PrepareBackprojection();
  //! The entry point into the backprojection
  virtual bool Backproject(MPhysicalEvent* Event, double* Image, int* Bins, int& NUsedBins, double& Maximum);

  //! Set the viewport, i.e. image dimensions
  virtual bool SetDimensions(double xMin, double xMax, unsigned int xNBins, 
                             double yMin, double yMax, unsigned int yNBins, 
                             double zMin = 0, double zMax = 1, unsigned int zNBins = 1,
                             MVector xAxis = MVector(1.0, 0.0, 0.0), MVector zAxis = MVector(0.0, 0.0, 1.0));

  // protected methods:
 protected:
  //! Assimilate the event data - stores parts of the event data in the class for acceleration pruposes 
  virtual bool Assimilate(MPhysicalEvent* Event);

  //! Calculate the repsonse of a Compton event
  bool BackprojectionCompton(double* Image, int* Bins, int& NUsedBins, double& Maximum);
  //! Calculate the repsonse of a Pair event
  bool BackprojectionPair(double* Image, int* Bins, int& NUsedBins, double& Maximum);


  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:



#ifdef ___CINT___
 public:
  ClassDef(MBackprojectionNearField, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
