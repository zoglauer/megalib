/*
 * MCalibration.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MCalibration__
#define __MCalibration__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! A general calibration result
class MCalibration
{
  // public interface:
 public:
  //! Default constructor
  MCalibration();
  //! Default destuctor 
  virtual ~MCalibration();

  //! Clone this calibration - the returned calibration must be deleted!
  virtual MCalibration* Clone() const;

  //! Return true if the calibration is completd
  bool IsCalibrated() const { return m_IsCalibrated; }
  //! Set the calibration status
  void IsCalibrated(bool Flag) { m_IsCalibrated = Flag; }
  
  //! Return the data as parsable string
  virtual MString ToParsableString(bool WithDescriptor = false) { return ""; } 
  
  
  // protected methods:
 protected:

  // private methods:
 private:



  // protected members:
 protected:
  //! Flag indicating that the calibration is completed
  bool m_IsCalibrated;


  // private members:
 private:


#ifdef ___CLING___
 public:
  ClassDef(MCalibration, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
