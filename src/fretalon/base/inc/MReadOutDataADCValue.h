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
#include "MReadOutDataInterfaceADCValue.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! This basic read-out data just consisting of one (ADC) value
class MReadOutDataADCValue : public MReadOutData, public MReadOutDataInterfaceADCValue
{
  // public interface:
 public:
  //! Default constructor
  MReadOutDataADCValue();
  //! Constructor given the data
  MReadOutDataADCValue(double Data);
  //! Simple default destructor
  virtual ~MReadOutDataADCValue();

  //! Clear the content of this read-out data element
  virtual void Clear();

  //! Return true if this read-out data is of the given type
  virtual bool IsOfType(const MString& String) const;
  //! Return the type of this read-out data
  virtual MString GetType() const;

  //! Clone this data element - the returned element must be deleted
  virtual MReadOutDataADCValue* Clone() const;
  
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
