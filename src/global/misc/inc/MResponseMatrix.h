/*
 * MResponseMatrix.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MResponseMatrix__
#define __MResponseMatrix__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <vector>
#include <sstream>
using namespace std;

// MEGAlib libs:
#include "MGlobal.h"
#include "MFileResponse.h"

// Forward declarations:
class MResponseMatrixO1;

////////////////////////////////////////////////////////////////////////////////


class MResponseMatrix
{
  // public interface:
 public:
  MResponseMatrix();
  MResponseMatrix(MString Name);
  virtual ~MResponseMatrix();

  void SetName(const MString& Name) { m_Name = Name; }
  MString GetName() const { return m_Name; }

  //! Set a hash value --- usually used to verify an outside object is still
  //! identical with that which has been used to create this response 
  void SetHash(const unsigned long& Hash) { m_Hash = Hash; }
  //! Return a hash value --- the has is not created internally, but has to be set
  unsigned long GetHash() const { return m_Hash; }

  unsigned int GetOrder() const { return m_Order; }

  virtual bool Read(MString FileName);
  virtual bool Write(MString FileName, bool Stream = false) = 0;

  // The number of simulated events which generated this response
  void SetSimulatedEvents(long SimulatedEvents) { m_NumberOfSimulatedEvents = SimulatedEvents; }
  // Get he number of simulated events which generated this response
  long GetSimulatedEvents() const { return m_NumberOfSimulatedEvents; }
  
  //! The start area of far-field simulations
  void SetFarFieldStartArea(double Area) { m_FarFieldStartArea = Area; }
  double GetFarFieldStartArea() const { return m_FarFieldStartArea; }
  
  virtual unsigned long GetNBins() const = 0;
  virtual float GetMaximum() const = 0;
  virtual float GetMinimum() const = 0;
  virtual float GetSum() const = 0;
  
  static const float c_ShowX;
  static const float c_ShowY;
  static const float c_ShowZ;
  static const float c_ShowNo;

  
  // protected methods:
 protected:
  //! Write some basic header data to the file/stream 
  void WriteHeader(ostringstream& out);
  
  //! Read the class specific info from the file
  virtual bool ReadSpecific(MFileResponse&, const MString&, const int) { return true; };
  
  // private methods:
 private:

  // protected members:
 protected:

  //! Name of this response
  MString m_Name;

  //! Order/Dimension of this response 
  unsigned int m_Order;
  
  //! Number of events simulated to generate this response
  long m_NumberOfSimulatedEvents;
  //! The start area of far-field simulations
  double m_FarFieldStartArea;

  //! A hash value --- this value is not calculated but has to be set from outside or read in via file
  unsigned long m_Hash;

  // private members:
 private:

#ifdef ___CINT___
 public:
  ClassDef(MResponseMatrix, 1) // base class for all response matrices
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
