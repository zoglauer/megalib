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

// Addition Christian Lang - Backproject
  //---------------------------------------------------------
  virtual bool Backproject(MPhysicalEvent* Event, double* Image, int* Bins,
  int& NUsedBins, double& Maximum, double X1Position, double Y1Position,
  double Z1Position, double X2Position, double Y2Position, double Z2Position);
//------------------------------------------------------------
  //! Set the viewport, i.e. image dimensions
  virtual bool SetDimensions(double x1Min, double x1Max, unsigned int x1NBins, 
                             double x2Min, double x2Max, unsigned int x2NBins,
                             double x3Min = 0, double x3Max = 0, unsigned int x3NBins = 1,
                             MVector x1Axis = MVector(1.0, 0.0, 0.0), 
                             MVector x3Axis = MVector(0.0, 0.0, 1.0));

  // protected methods:
 protected:
  //! Assimilate the event data - stores parts of the event data in the class for acceleration pruposes 
  virtual bool Assimilate(MPhysicalEvent* Event);

  //! Calculate the repsonse of a Compton event
  bool BackprojectionCompton(double* Image, int* Bins, int& NUsedBins, double& Maximum);
  
   // Addition Christian Lang
  //---------------------------------------------------------
  bool BackprojectionComptonLine(double* Image, int* Bins, int& NUsedBins, double& Maximum,
  double X1Position, double Y1Position, double Z1Position, double X2Position, double Y2Position, double Z2Position
  );
  //---------------------------------------------------------
  
  //! Calculate the repsonse of a Pair event
  bool BackprojectionPair(double* Image, int* Bins, int& NUsedBins, double& Maximum);


  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:



#ifdef ___CLING___
 public:
  ClassDef(MBackprojectionNearField, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
