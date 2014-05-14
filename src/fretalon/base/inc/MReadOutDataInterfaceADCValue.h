/*
 * MReadOutDataInterfaceADCValue.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MReadOutDataInterfaceADCValue__
#define __MReadOutDataInterfaceADCValue__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <iostream>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MTokenizer.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! A read-out data interface just consisting of one ADC value
class MReadOutDataInterfaceADCValue
{
  // public interface:
 public:
  //! Default constructor
  MReadOutDataInterfaceADCValue();
  //! Constructor given the data
  MReadOutDataInterfaceADCValue(double ADCValue);
  //! Simple default destructor
  virtual ~MReadOutDataInterfaceADCValue();

  //! Clear the content of this read-out data element
  virtual void Clear();

  //! Return true if the data is zero
  virtual bool IsZero() const;

  //! Return true if the data is positive
  virtual bool IsPositive() const;

  //! Set the ADC value
  void SetADCValue(double ADCValue) { m_ADCValue = ADCValue; }
  //! Get the ADC value
  double GetADCValue() const { return m_ADCValue; }
  
  //! Return the number of parsable elements
  virtual unsigned int GetNumberOfParsableElements() const;  
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
  double m_ADCValue;

  // private members:
 private:



#ifdef ___CINT___
 public:
  ClassDef(MReadOutDataInterfaceADCValue, 0) // no description
#endif

};

//! Streamify the read-out data
ostream& operator<<(ostream& os, const MReadOutDataInterfaceADCValue& R);

#endif


////////////////////////////////////////////////////////////////////////////////
