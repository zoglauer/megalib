/*
 * MBackprojection.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MBackprojection__
#define __MBackprojection__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MFastMath.h"
#include "MProjection.h"
#include "MViewPort.h"
#include "MPhysicalEvent.h"
#include "MResponse.h"
#include "MEfficiency.h"
#include "MResponseNormalizers.h"
#include "MDGeometryQuest.h"


///////////////////////////////////////////////////////////////////////////////


//! Backprojection base class
class MBackprojection : public MProjection, public MViewPort
{
  // Public Interface:
 public:
  // The default constructor
  MBackprojection(MCoordinateSystem CoordianteSystem);
  // The default destructor
  virtual ~MBackprojection();

  //! Choose if you want to use maths approximations
  virtual void SetApproximatedMaths(bool Approximated = true) { m_ApproximatedMaths = Approximated; if (m_Response != 0) m_Response->SetApproximatedMaths(m_ApproximatedMaths); }

  //! Prepare all backprojections - must be called before the backprojections
  virtual void PrepareBackprojection();  
  //virtual bool Backproject(MPhysicalEvent* Event, double* Image) = 0;
  virtual bool Backproject(MPhysicalEvent* Event, double* Image, int* Bins, int& NUsedBins, double& Maximum) = 0;

  //! Set the geometry
  virtual void SetGeometry(MDGeometryQuest* Geometry) { m_Geometry = Geometry; }
  //! Use absorption for response calculation
  void SetUseAbsorptions(bool Absorptions) { m_UseAbsorptions = Absorptions; }

  //! Set the response
  virtual void SetResponse(MResponse* Response) { m_Response = Response; m_Response->SetApproximatedMaths(m_ApproximatedMaths); }
  //! Get the response
  MResponse* GetResponse() const { return m_Response; }

  //! Set the response normalizers
  virtual void SetResponseNormalizers(MResponseNormalizers* ResponseNormalizers) { m_ResponseNormalizers = ResponseNormalizers; }

  //! Set the efficiency
  virtual void SetEfficiency(MEfficiency* Efficiency) { m_Efficiency = Efficiency; }


  // protected methods:
 protected:
  //! Assimilate the event data - stores parts of the event data in the class for acceleration pruposes 
  virtual bool Assimilate(MPhysicalEvent* Event);


  //! Rotate the detector
  virtual void Rotate(double &x, double &y, double &z);



  //! Optimized version of calculating an angle
  //! This function is absolutely time critical!
  double Angle(double u, double v, double w, double x, double y, double z) {
    double Nenner = (u*x + v*y + w*z)/sqrt((u*u + v*v + w*w)*(x*x + y*y + z*z));
    
    if (m_ApproximatedMaths == false) {
      if (Nenner > 1.0) return 0.0;
      if (Nenner < -1.0) return c_Pi;
      return acos(Nenner);
    } else {
      // Acos checks if Nenner is within -1 <= x <= 1
      return MFastMath::acos(Nenner);
    }
  }

  //! Test if a value is not NaN
  bool InRange(double x) {
    if (x < +numeric_limits<double>::max() && 
        x > -numeric_limits<double>::max()) {
      return true; 
    } else {
      return false; 
    }
  }

  // protected members:
 protected:

  //! The response - Compton as well as pair
  MResponse* m_Response;              

  //! The response normalizers
  MResponseNormalizers* m_ResponseNormalizers;

  //! The overall detector efficiency
  MEfficiency* m_Efficiency;              

  //! True if absorptions are used during response calculations
  bool m_UseAbsorptions;

  //! The geometry description
  MDGeometryQuest* m_Geometry;




#ifdef ___CLING___
 public:
  ClassDef(MBackprojection, 0)  // abstract base class for backprojections
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
