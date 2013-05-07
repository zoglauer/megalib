/*
 * MBackprojectionCartesian.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MBackprojectionCartesian__
#define __MBackprojectionCartesian__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include "TObject.h"

// MEGAlib libs:
#include "MGlobal.h"
#include "MBackprojection.h"
#include "MPhysicalEvent.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MBackprojectionCartesian : public MBackprojection
{
  // public interface:
 public:
  //! Default constructor
  MBackprojectionCartesian(int CoordinateSystem = MProjection::c_Cartesian2D);
  //! Default destructor
  virtual ~MBackprojectionCartesian();

  //! Prepare all backprojections - must be called before the backprojections
  virtual void PrepareBackprojection();
  //! The entry point into the backprojection
  virtual bool Backproject(MPhysicalEvent* Event, double* Image, int* Bins, int& NUsedBins, double& Maximum);

  //! Set the viewport, i.e. image dimensions
  virtual void SetViewportDimensions(double xMin, double xMax, int xNBins, 
                                     double yMin, double yMax, int yNBins, 
                                     double zMin = 0, double zMax = 0, int zNBins = 1,
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
  //! Center of the bins (x) x are the image coordinates not the detector coordinates!
  double* m_xBinCenter;  
  //! Center of the bins (y)
  double* m_yBinCenter;  
  //! Center of bins z
  double* m_zBinCenter;  


  // private members:
 private:



#ifdef ___CINT___
 public:
  ClassDef(MBackprojectionCartesian, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
