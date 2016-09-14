/*
 * MViewPort.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MViewPort__
#define __MViewPort__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! A dummy definition of a class
class MViewPort
{
  // public interface:
 public:
  //! Default constructor
  MViewPort();
  //! Default destuctor 
  virtual ~MViewPort();

  //! Set the viewport / image dimensions
  virtual bool SetDimensions(double xMin, double xMax, unsigned int xNBins, 
                             double yMin, double yMax, unsigned int yNBins,
                             double zMin = 0, double zMax = 0, unsigned int zNBins = 1,
                             MVector xAxis = MVector(1.0, 0.0, 0.0), 
                             MVector zAxis = MVector(0.0, 0.0, 1.0));

  // protected methods:
 protected:

  // private methods:
 private:



  // protected members:
 protected:
  //! minimum x (phi) of the viewport
  double m_x1Min;
  //! maximum x (phi) of the viewport
  double m_x1Max;
  //! minimum y (theta) of the viewport
  double m_x2Min; 
  //! maximum y (theta) of the viewport
  double m_x2Max; 
  //! minimum z (radius) of the viewport
  double m_x3Min;
  //! maximum z (radius) of the viewport
  double m_x3Max;

  //! number of bins in x (phi) direction
  unsigned int m_x1NBins;                     
  //! number of bins in y (theta) direction
  unsigned int m_x2NBins; 
  //! number of bins in z (radius) direction
  unsigned int m_x3NBins; 

  //! length of a bin in x (phi) direction
  double m_x1IntervalLength;
  //! length of a bin in y (theta) direction
  double m_x2IntervalLength;
  //! length of a bin in z (radius) direction
  double m_x3IntervalLength;
  
  //! Center of the x (phi) bins
  double* m_x1BinCenter;    
  //! Center of the y (theta) bins
  double* m_x2BinCenter;    
  //! Center of the z (radius) bins
  double* m_x3BinCenter;    
  
  //! Total number of bins
  unsigned int m_NImageBins; 

  //! The x-axis rotation
  MVector m_XAxis;
  //! The z-axis rotation
  MVector m_ZAxis;

  //! The stored rotation matrix
  TMatrix m_Rotation;
  
  
  // private members:
 private:



#ifdef ___CINT___
 public:
  ClassDef(MViewPort, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
