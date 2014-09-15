/*
 * MReadOut.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MReadOut__
#define __MReadOut__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MReadOutElement.h"
#include "MReadOutData.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! A single chanel read out consisting of a read-out element and its associated data
class MReadOut
{
  // public interface:
 public:
  //! Default constructor
  MReadOut();
  //! Constructor setting the read out element and data
  MReadOut(const MReadOutElement& ROE, const MReadOutData& ROD);
  //! Copy constructor
  MReadOut(const MReadOut& RO);
  //! Default destructor
  virtual ~MReadOut();

  //! Assignment operator
  MReadOut& operator=(const MReadOut& RO);
  
  //! Set the read-out element
  void SetReadOutElement(const MReadOutElement& ROE);
  //! Return a const reference to the read-out element
  const MReadOutElement& GetReadOutElement() const { return *m_ROE; }
  
  //! Set the read-out data
  void SetReadOutData(const MReadOutData& ROD);
   //! Return a const reference to the read-out data
  const MReadOutData& GetReadOutData() const { return *m_ROD; }
  
  //! Return the data as parsable string
  virtual MString ToParsableString(bool WithDescriptor = false); 
  
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
  //! The read-out element
  MReadOutElement* m_ROE;
  //! The associated data
  MReadOutData* m_ROD;


#ifdef ___CINT___
 public:
  ClassDef(MReadOut, 0) // no description
#endif

};

//! Streamify the read out 
ostream& operator<<(ostream& os, const MReadOut& RO);

#endif


////////////////////////////////////////////////////////////////////////////////
