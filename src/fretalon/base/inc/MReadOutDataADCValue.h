/*
 * MReadOutDataADCValue.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MReadOutDataADCValue__
#define __MReadOutDataADCValue__


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


//! This basic read-out data just consisting of one (ADC) value
class MReadOutDataADCValue : public MReadOutData
{
  // public interface:
 public:
  //! The type name --- must be unique
  static const MString m_Type;
  //! The type name ID --- must be unique
  static const long m_TypeID;

  //! Default constructor
  MReadOutDataADCValue();
  //! Constructor wrapping around another data
  MReadOutDataADCValue(MReadOutData* Data);
  //! Simple default destructor
  virtual ~MReadOutDataADCValue();

  //! Return the type of this read-out data --- hard coded to save space
  virtual MString GetType() const { return m_Type; }
  //! Return the type ID of this read-out data --- hard coded to save memory
  virtual long GetTypeID() const { return m_TypeID; }

  //! Clear the content of this read-out data element
  virtual void Clear();

  //! Return true if the data is zero
  virtual bool IsZero() const;
  //! Return true if the data is positive
  virtual bool IsPositive() const;

  //! Set the ADC value
  void SetADCValue(unsigned int ADCValue) { m_ADCValue = ADCValue; }
  //! Get the ADC value
  unsigned int GetADCValue() const { return m_ADCValue; }

 
  //! Clone this data element - the returned element must be deleted
  virtual MReadOutDataADCValue* Clone() const;
  
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
  //! The data
  unsigned int m_ADCValue;

  // private members:
 private:


#ifdef ___CINT___
 public:
  ClassDef(MReadOutDataADCValue, 0) // no description
#endif

};

//! Streamify the read-out data
ostream& operator<<(ostream& os, const MReadOutDataADCValue& R);

#endif


////////////////////////////////////////////////////////////////////////////////
