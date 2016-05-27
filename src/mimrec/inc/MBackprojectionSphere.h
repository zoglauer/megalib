/*
 * MBackprojectionSphere.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MBackprojectionSphere__
#define __MBackprojectionSphere__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include "MString.h"
#include "TMatrix.h"

// MEGAlib libs:
#include "MGlobal.h"
#include "MBackprojection.h"
#include "MPhysicalEvent.h"
#include "MVector.h"


////////////////////////////////////////////////////////////////////////////////


class MBackprojectionSphere : public MBackprojection
{
  // public Interface:
 public:
  //! Default constructor
  MBackprojectionSphere(MCoordinateSystem CoordianteSystem = MCoordinateSystem::c_Spheric);
  //! Default destructor
  virtual ~MBackprojectionSphere();

  //! Prepare the backprojections
  virtual void PrepareBackprojection();
  //! The entry point into the backprojection
  virtual bool Backproject(MPhysicalEvent* Event, double* Image, int* Bins, int& NUsedBins, double& Maximum);

  //! Set the viewport, i.e. image dimensions
  virtual void SetViewportDimensions(double xMin, double xMax, int xNBins, 
                                     double yMin, double yMax, int yNBins, 
                                     double zMin = 0, double zMax = 0, int zNBins = 0,
                                     MVector xAxis = MVector(1.0, 0.0, 0.0), MVector zAxis = MVector(0.0, 0.0, 1.0));


  // protected methods:
 protected:
  //! Assimilate the event data - stores parts of the event data in the class for acceleration pruposes 
  virtual bool Assimilate(MPhysicalEvent* Event);

  //! Calculate the repsonse of a Compton event
  bool BackprojectionCompton(double* Image, int* Bins, int& NUsedBins, double& Maximum);
  //! Calculate the repsonse of a Pair event
  bool BackprojectionPair(double* Image, int* Bins, int& NUsedBins, double& Maximum);
  //! Calculate the repsonse of a Photo event
  bool BackprojectionPhoto(double* Image, int* Bins, int& NUsedBins, double& Maximum);



  bool ConeCenter(); 

  virtual void Rotate(double &x, double &y, double &z);
  void ToSpherical(double x, double y, double z, 
                   double &t, double &p, double &r);
  void ToCartesean(double t, double p, double r, 
                   double &x, double &y, double &z);


  //! Optimized version of calculating an angle
  //! This function is absolutely time critical!
  double FastAngle(double u, double v, double w, double x, double y, double z) {
    double Nenner = m_InvSquareDist*(u*x + v*y + w*z);
    if (m_ApproximatedMaths == true) {
      // Acos checks if Nenner is within -1 <= x <= 1
      return Acos(Nenner);
    } else {
      if (Nenner > 1.0) return 0.0;
      if (Nenner < -1.0) return c_Pi;
      return acos(Nenner);
    }
  }

  // private methods:
 private:

  // protected members:
 protected:
  //! Intersection between the cone axis and the sphere in x, y, z
  MVector m_ConeCenter;  

  //! Theta coordinate of the intersection between the cone axis and the sphere
  double m_ThetaConeCenter; 
  //! Phi coordinate of the intersection between the cone axis and the sphere
  double m_PhiConeCenter; 
  //! Radius coordinate of the intersection between the cone axis and the sphere
  double m_RadiusConeCenter;

  // Area of each bin as a function of theta
  double* m_AreaBin;    

  // Center of the bins (theta)
  double* m_x1BinCenter;    
  // Center of the bins (theta)
  double* m_x2BinCenter;    
  // Center of the bins (phi)
  double* m_x3BinCenter;    

  // x-Position of the center of the bin 
  double* m_xBin;       
  // y-Position of the center of the bin
  double* m_yBin;       
  // z-Position of the center of the bin
  double* m_zBin;       

  //! For optimization: inverted squared vector size of a vector on the sphere
  double m_InvSquareDist;

  //! The x-axis rotation
  MVector m_XAxis;

  //! The z-axis rotation
  MVector m_ZAxis;

  //! The stored rotation matrix
  TMatrix m_Rotation;


#ifdef ___CINT___
 public:
  ClassDef(MBackprojectionSphere, 0) // base class for all spherical reconstruction
#endif

};


#endif


////////////////////////////////////////////////////////////////////////////////
