/*
 * MReadOutDataTemperature.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MReadOutDataTemperature__
#define __MReadOutDataTemperature__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <iostream>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MTokenizer.h"
#include "MReadOutData.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! This basic read-out data just consisting of one temperature value as unsigned int
class MReadOutDataTemperature : public MReadOutData
{
  // public interface:
 public:
  //! The type name --- must be unique
  static const MString m_Type;
  //! The type name ID --- must be unique
  static const long m_TypeID;

  //! Default constructor
  MReadOutDataTemperature();
  //! Constructor given the data
  MReadOutDataTemperature(MReadOutData* Data);
  //! Simple default destructor
  virtual ~MReadOutDataTemperature();

  //! Return the type of this read-out data --- hard coded to save space
  virtual MString GetType() const { return m_Type; }
  //! Return the type ID of this read-out data --- hard coded to save memory
  virtual long GetTypeID() const { return m_TypeID; }

  //! Clear the content of this read-out data element
  virtual void Clear();

  //! Set the temperature
  void SetTemperature(double Temperature) { m_Temperature = Temperature; }
  //! Get the temperature
  double GetTemperature() const { return m_Temperature; }

  //! Clone this data element - the returned element must be deleted
  virtual MReadOutDataTemperature* Clone() const;
  
  //! Return the number of parsable elements
  virtual unsigned int GetNumberOfParsableElements() const;  
  //! Return the data as parsable string
  virtual MString ToParsableString(bool WithDescriptor = false) const; 
  //! Parse the data from the tokenizer 
  virtual bool Parse(const MTokenizer& T, unsigned int StartElement);
  
  //! Dump a string
  virtual MString ToString() const;
  
  
  // protected methods:
 protected:

  // private methods:
 private:



  // protected members:
 protected:
  //! The temperature value
  double m_Temperature;

  // private members:
 private:



#ifdef ___CLING___
 public:
  ClassDef(MReadOutDataTemperature, 0) // no description
#endif

};

//! Streamify the read-out data
ostream& operator<<(ostream& os, const MReadOutDataTemperature& R);

#endif


////////////////////////////////////////////////////////////////////////////////
