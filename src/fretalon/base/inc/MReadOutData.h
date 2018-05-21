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
  MReadOutData();
  //! Standard constructor
  MReadOutData(MReadOutData* ReadOutData);
  //! Simple default destructor
  virtual ~MReadOutData();

  //! Return the type of this read-out data --- hard coded to save space
  virtual MString GetType() const { return "base"; }
  //! Return the type ID of this read-out data --- hard coded to save memory
  virtual long GetTypeID() const { return 0; }

  //! Return the combined type of this read-out data and its wrapped data
  virtual MString GetCombinedType() const;

  //! Return true if this read-out data is of the given type
  bool IsOfType(const MString& Type) const { return (GetType() == Type); }
  //! Return true if this read-out data is of the given type
  bool IsOfType(long Type) const { return (GetTypeID() == Type); }

  //! Clear the content of this read-out data element
  virtual void Clear() { if (m_Wrapped != 0) m_Wrapped->Clear(); }

  void SetWrapped(MReadOutData* ROD) { m_Wrapped = ROD; }
  
  //! Return this read-out data as the given type - return nullptr if not possible
  virtual MReadOutData* Get(long TypeID);
  
  //! Clone this data element - the returned element must be deleted
  virtual MReadOutData* Clone() const { return new MReadOutData(); }
  
  //! Return the number of parsable elements
  virtual unsigned int GetNumberOfParsableElements() const { return m_Wrapped != 0 ? m_Wrapped->GetNumberOfParsableElements() : 0; }  
  //! Parse the data from the tokenizer 
  virtual bool Parse(const MTokenizer& T, unsigned int StartElement);
  
  //! Return the data as parsable string
  virtual MString ToParsableString(bool WithDescriptor = false) const { return m_Wrapped != 0 ? m_Wrapped->ToParsableString(WithDescriptor) : ""; }
  //! Dump a string
  virtual MString ToString() const { return m_Wrapped != 0 ? m_Wrapped->ToString() : ""; }  
  
  
  // protected methods:
 protected:
  //! Standard constructor - do the decoration
  MReadOutData(MReadOutData* Data, const MString& Type);

  // private methods:
 private:



  // protected members:
 protected:
  //! The wrapped read-out data
  MReadOutData* m_Wrapped;


  // private members:
 private:

  
#ifdef ___CLING___
 public:
  ClassDef(MReadOutData, 0) // no description
#endif

};

//! Streamify the read-out data
ostream& operator<<(ostream& os, const MReadOutData& R);

#endif


////////////////////////////////////////////////////////////////////////////////
