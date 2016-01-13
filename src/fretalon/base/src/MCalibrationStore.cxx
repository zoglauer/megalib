/*
 * MCalibrationStore.cxx
 *
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 *
 * This code implementation is the intellectual property of
 * Andreas Zoglauer.
 *
 * By copying, distributing or modifying the Program (or any work
 * based on the Program) you indicate your acceptance of this statement,
 * and all its terms.
 *
 */


// Include the header:
#include "MCalibrationStore.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MExceptions.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MCalibrationStore)
#endif


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MCalibrationStore::MCalibrationStore()
{
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
MCalibrationStore::~MCalibrationStore()
{
  Clear();
}


////////////////////////////////////////////////////////////////////////////////


//! Remove all data
void MCalibrationStore::Clear()
{
  for (unsigned int i = 0; i < m_ROEs.size(); ++i) delete m_ROEs[i];
  for (unsigned int i = 0; i < m_ROEs.size(); ++i) delete m_Calibrations[i];
  m_ROEs.clear();
  m_Calibrations.clear();
}


////////////////////////////////////////////////////////////////////////////////


//! Add a new read-out element and a dummy calibration, if the read-out element exist, do nothing
void MCalibrationStore::Add(const MReadOutElement& ROE)
{
  bool Found = false;
  for (unsigned int r = 0; r < m_ROEs.size(); ++r) {
    if (*(m_ROEs[r]) == ROE) {
      Found = true;
      break;
    }
  }
  if (Found == false) {
    m_ROEs.push_back(ROE.Clone());
    m_Calibrations.push_back(new MCalibration());
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Remove the calibration of the read-out element and replace it by a dummy one
void MCalibrationStore::Remove(const MReadOutElement& ROE)
{
  for (unsigned int r = 0; r < m_ROEs.size(); ++r) {
    if (*(m_ROEs[r]) == ROE) {
      delete m_Calibrations[r];
      m_Calibrations[r] = new MCalibration();
      break;
    }
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Add a new read-out element and calibration, if the read-out element exits, then replace the calibration
void MCalibrationStore::Add(const MReadOutElement& ROE, const MCalibration& Calibration)
{
  bool Found = false;
  for (unsigned int r = 0; r < m_ROEs.size(); ++r) {
    if (*(m_ROEs[r]) == ROE) {
      Found = true;
      delete m_Calibrations[r];
      m_Calibrations[r] = Calibration.Clone();
      break;
    }
  }
  if (Found == false) {
    m_ROEs.push_back(ROE.Clone());
    m_Calibrations.push_back(Calibration.Clone());
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Get a calibration by its index
//! If the index is out of bound return the exception MExceptionIndexOutOfBounds
MCalibration& MCalibrationStore::GetCalibration(unsigned int c)
{
  if (c < m_Calibrations.size()) {
    return *m_Calibrations[c];
  }
  
  throw MExceptionIndexOutOfBounds(0, m_Calibrations.size(), c); 
 
  // The 0th element might not exist and there miight have been a crash, but we throw an exception...
  return *m_Calibrations[0];
}


////////////////////////////////////////////////////////////////////////////////


//! Get a read-out element by its index
//! If the index is out of bound return the exception MExceptionIndexOutOfBounds
MReadOutElement& MCalibrationStore::GetReadOutElement(unsigned int c)
{
  if (c < m_ROEs.size()) {
    return *m_ROEs[c];
  }
  
  throw MExceptionIndexOutOfBounds(0, m_ROEs.size(), c); 
 
  // The 0th element might not exist and there miight have been a crash, but we throw an exception...
  return *m_ROEs[0];
}

  
////////////////////////////////////////////////////////////////////////////////


//! Get a calibration by its associated read-out element
//! If the read-out element does not exist, return the exception MExceptionIndexOutOfBounds
MCalibration& MCalibrationStore::GetCalibration(const MReadOutElement& ROE)
{
  for (unsigned int r = 0; r < m_ROEs.size(); ++r) {
    if (*m_ROEs[r] == ROE) return *m_Calibrations[r];
  }
  
  throw MExceptionObjectDoesNotExist(ROE.ToString()); 
 
  // The 0th element might not exist and there might have been a crash, but we throw an exception...
  return *m_Calibrations[0];
}


////////////////////////////////////////////////////////////////////////////////


//! Returns the index of a calibration by its associated read-out element
//! If the read-out element does not exist, return g_UnsignedIntNotDefined
unsigned int MCalibrationStore::FindCalibration(const MReadOutElement& ROE)
{
  for (unsigned int r = 0; r < m_ROEs.size(); ++r) {
    if (*m_ROEs[r] == ROE) return r;
  }

  return g_UnsignedIntNotDefined;
}


// MCalibrationStore.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
