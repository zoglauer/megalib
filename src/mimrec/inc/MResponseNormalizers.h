/*
 * MResponseNormalizers.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MResponseNormalizers__
#define __MResponseNormalizers__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MRotation.h"
#include "MViewPort.h"
#include "MResponseMatrixON.h"
#include "MPhysicalEvent.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! This class contains the list-mode event normalizers
class MResponseNormalizers : public MViewPort
{
  // public interface:
 public:
  //! Default constructor
  MResponseNormalizers();
  //! Default destructor
  virtual ~MResponseNormalizers();

  //! Load the near-field efficiency data files
  bool LoadNearFieldEfficiencies(MString NearFieldDetectionEfficiency, MString NearFieldParameterEfficiency);

  //! Return true if we have response normalizers:
  bool UseNormalizers() const { return m_UseNormalizers; }

  //! Set the viewport / image dimensions
  virtual bool SetDimensions(double xMin, double xMax, unsigned int xNBins,
                             double yMin, double yMax, unsigned int yNBins,
                             double zMin = 0, double zMax = 0, unsigned int zNBins = 0,
                             MVector xAxis = MVector(1.0, 0.0, 0.0), MVector zAxis = MVector(0.0, 0.0, 1.0));

  //! Return the near-field detection efficiency
  double GetNearFieldDetectionEfficiency(unsigned int ImageBin);
  
  //! Return the near-field parameter efficiency
  double GetNearFieldParameterEfficiency(const vector<double>& AxesValues);


  /*
  //! Create the ResponseNormalizers for one event
  virtual bool Expose(MPhysicalEvent* Event);

  //! Return a copy of the current ResponseNormalizers map. Unit: cm2 * sec / sr
  //! User must delete array via "delete [] ..."
  virtual double* GetResponseNormalizers();
  */

  // protected methods:
 protected:

  
  
  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  //! Flag indicating that we are using normalizers
  bool m_UseNormalizers;
  //! Flag indicating that the efficiency file has been loaded succesfully
  bool m_IsLoaded;

  //! The near-field detection efficiency
  MResponseMatrixON m_NearFieldDetectionEfficiency;
  //! The near-field parameter efficiency
  MResponseMatrixON m_NearFieldParameterEfficiency;

  //! The near-field detection efficiency binned into imaging bins
  vector<double> m_BinnedNearFieldDetectionEfficiency;

#ifdef ___CLING___
 public:
  ClassDef(MResponseNormalizers, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
