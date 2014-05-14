/*
 * MReadOutData.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MReadOutData__
#define __MReadOutData__


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


//! The basic read-out data class - doesn't contain any specific data
class MReadOutData
{
  // public interface:
 public:
  //! Default constructor
  MReadOutData() {};
  //! Simple default destructor
  virtual ~MReadOutData() {};

  //! Clear the content of this read-out data element
  virtual void Clear() {}

  //! Return true if this read-out data is of the given type
  virtual bool IsOfType(const MString& String) const { return false; }
  //! Return the type of this read-out data
  virtual MString GetType() const { return "none"; }

  //! Clone this data element - the returned element must be deleted
  virtual MReadOutData* Clone() const { return new MReadOutData(); }
  
  //! Return the number of parsable elements
  virtual unsigned int GetNumberOfParsableElements() const { return 0; }  
  //! Parse the data from the tokenizer 
  virtual bool Parse(const MTokenizer& T, unsigned int StartElement) { return false; }
  
  //! Dump a string
  virtual MString ToString() const { return "Read-out data"; };
  
  
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
  ClassDef(MReadOutData, 0) // no description
#endif

};

//! Streamify the read-out data
ostream& operator<<(ostream& os, const MReadOutData& R);

#endif


////////////////////////////////////////////////////////////////////////////////
