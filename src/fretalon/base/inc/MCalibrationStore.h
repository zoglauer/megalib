/*
 * MCalibrationStore.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MCalibrationStore__
#define __MCalibrationStore__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MReadOutElement.h"
#include "MCalibration.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! A store for multiple calibrations, one per read-out element
class MCalibrationStore
{
  // public interface:
 public:
  //! Default constructor
  MCalibrationStore();
  //! Default destuctor 
  virtual ~MCalibrationStore();

  //! Remove all data
  void Clear();

  //! Add a new read-out element and a dummy calibration, if the read-out element exist, do nothing
  void Add(const MReadOutElement& ROE);

  //! Add a new read-out element and calibration, if the read-out element exits, then replace the calibration
  void Add(const MReadOutElement& ROE, const MCalibration& Calibration);
  
  //! Return the number of stored read-out elements/calibrations
  unsigned int GetNumberOfElements() const { return m_ROEs.size(); }
  
  //! Get a read-out element by its index
  //! If the index is out of bound return the exception MExceptionIndexOutOfBounds
  MReadOutElement& GetReadOutElement(unsigned int c);
  
  //! Get a calibration by its index
  //! If the index is out of bound return the exception MExceptionIndexOutOfBounds
  MCalibration& GetCalibration(unsigned int c);
  //! Get a calibration by its associated read-out element
  //! If the read-out element does not exist, return the exception MExceptionIndexOutOfBounds
  MCalibration& GetCalibration(const MReadOutElement& ROE);
  //! Returns the index of a calibration by its associated read-out element
  //! If the read-out element does not exist, return g_UnsignedIntNotDefined
  unsigned int FindCalibration(const MReadOutElement& ROE);
  
  // protected methods:
 protected:

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  //! The read-out elements
  vector<MReadOutElement*> m_ROEs; 
  //! The calibrations associated with the read out elements
  vector<MCalibration*> m_Calibrations;
  

#ifdef ___CINT___
 public:
  ClassDef(MCalibrationStore, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
