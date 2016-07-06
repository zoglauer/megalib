/*
 * MReadOutDataOrigins.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MReadOutDataOrigins__
#define __MReadOutDataOrigins__


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


//! This basic read-out data just consisting of one timing value as unsigned int
class MReadOutDataOrigins : public MReadOutData
{
  // public interface:
 public:
  //! The type name --- must be unique
  static const MString m_Type;
  //! The type name ID --- must be unique
  static const long m_TypeID;

  //! Default constructor
  MReadOutDataOrigins();
  //! Constructor given the data
  MReadOutDataOrigins(MReadOutData* Data);
  //! Simple default destructor
  virtual ~MReadOutDataOrigins();

  //! Return the type of this read-out data --- hard coded to save space
  virtual MString GetType() const { return m_Type; }
  //! Return the type ID of this read-out data --- hard coded to save memory
  virtual long GetTypeID() const { return m_TypeID; }

  //! Clear the content of this read-out data element
  virtual void Clear();

  //! Set the timing
  void SetOrigins(vector<int> Origins) { m_Origins = Origins; }
  //! Get the timing
  vector<int> GetOrigins() const { return m_Origins; }

  //! Clone this data element - the returned element must be deleted
  virtual MReadOutDataOrigins* Clone() const;
  
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
  //! The origins
  vector<int> m_Origins;

  // private members:
 private:



#ifdef ___CINT___
 public:
  ClassDef(MReadOutDataOrigins, 0) // no description
#endif

};

//! Streamify the read-out data
ostream& operator<<(ostream& os, const MReadOutDataOrigins& R);

#endif


////////////////////////////////////////////////////////////////////////////////
