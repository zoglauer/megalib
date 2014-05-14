/*
 * MReadOutData.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MReadOutDataADCValueWithTiming__
#define __MReadOutDataADCValueWithTiming__


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
#include "MReadOutDataInterfaceTiming.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! This basic read-out data just consisting of one (ADC) value plus timing
class MReadOutDataADCValueWithTiming : public MReadOutData, public MReadOutDataInterfaceADCValue, public MReadOutDataInterfaceTiming
{
  // public interface:
 public:
  //! Default constructor
  MReadOutDataADCValueWithTiming();
  //! Constructor given the data
  MReadOutDataADCValueWithTiming(double Data, double Timing);
  //! Simple default destructor
  virtual ~MReadOutDataADCValueWithTiming();

  //! Clear the content of this read-out data element
  virtual void Clear();

  //! Return true if this read-out data is of the given type
  virtual bool IsOfType(const MString& String) const;
  //! Return the type of this read-out data
  virtual MString GetType() const;

  //! Clone this data element - the returned element must be deleted
  virtual MReadOutDataADCValueWithTiming* Clone() const;
  
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
  ClassDef(MReadOutDataADCValueWithTiming, 0) // no description
#endif

};

//! Streamify the read-out data
ostream& operator<<(ostream& os, const MReadOutDataADCValueWithTiming& R);

#endif


////////////////////////////////////////////////////////////////////////////////
