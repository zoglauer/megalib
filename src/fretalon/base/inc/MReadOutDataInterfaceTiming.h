/*
 * MReadOutDataInterfaceTiming.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MReadOutDataInterfaceTiming__
#define __MReadOutDataInterfaceTiming__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <iostream>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MReadOutData.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! A read-out data interface just consisting of one timing value
class MReadOutDataInterfaceTiming
{
  // public interface:
 public:
  //! Default constructor
  MReadOutDataInterfaceTiming();
  //! Constructor given the data
  MReadOutDataInterfaceTiming(double Timing);
  //! Simple default destructor
  virtual ~MReadOutDataInterfaceTiming();

  //! Clear the content of this read-out data element
  virtual void Clear();

  //! Set the timing
  void SetTiming(double Timing) { m_Timing = Timing; }
  //! Get the timing
  double GetTiming() const { return m_Timing; }
  
  //! Return the number of parsable elements
  virtual unsigned int GetNumberOfParsableElements() const;  
  //! Return the data as parsable string
  virtual MString ToParsableString() const; 
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
  //! The timing
  double m_Timing;
  
  // private members:
 private:



#ifdef ___CINT___
 public:
  ClassDef(MReadOutDataInterfaceTiming, 0) // no description
#endif

};

//! Streamify the read-out data
ostream& operator<<(ostream& os, const MReadOutDataInterfaceTiming& R);

#endif


////////////////////////////////////////////////////////////////////////////////
