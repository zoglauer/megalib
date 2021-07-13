/*
 * MReadOutElementStrip.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MReadOutElementStrip__
#define __MReadOutElementStrip__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MReadOutElement.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! The read-out element of a strip. In addition to the detector ID it also has 
//! a strip ID.
class MReadOutElementStrip : public MReadOutElement
{
  // public interface:
 public:
  //! Read out element of a SINGLE-SIDED strip detector  
  MReadOutElementStrip();
  //! Read out element of a SINGLE-SIDED strip detector  
  MReadOutElementStrip(unsigned int DetectorID, unsigned int StripID);
  //! Simple default destructor
  virtual ~MReadOutElementStrip();

  //! Clone this read-out element - the returned element must be deleted!
  virtual MReadOutElementStrip* Clone() const;

  //! Clear the content of this read-out element
  virtual void Clear();

  //! Compare two read-out elements
  virtual bool operator==(const MReadOutElement& R) const;
  //! Smaller than operator
  virtual bool operator<(const MReadOutElement& R) const;

  //! Return true if this read-out element is of the given type
  virtual bool IsOfType(const MString& String) const;
  //! Return the type of this read-out element
  virtual MString GetType() const;

  //! Set the strip ID
  void SetStripID(unsigned int StripID) { m_StripID = StripID; }
  //! Get the strip ID
  unsigned int GetStripID() const { return m_StripID; }
  
  //! Return the number of parsable elements
  virtual unsigned int GetNumberOfParsableElements() const;  
  //! Parse the data from the tokenizer 
  virtual bool Parse(const MTokenizer& T, unsigned int StartElement);
  //! Return the data as parsable string
  virtual MString ToParsableString(bool WithDescriptor = false) const; 

  //! Dump a string
  virtual MString ToString() const;


  // protected methods:
 protected:

  // private methods:
 private:



  // protected members:
 protected:
   //! The ID of the associated strip
  unsigned int m_StripID;

  // private members:
 private:



#ifdef ___CLING___
 public:
  ClassDef(MReadOutElementStrip, 0) // no description
#endif

};

//! Streamify the read-out element
ostream& operator<<(ostream& os, const MReadOutElementStrip& R);

#endif


////////////////////////////////////////////////////////////////////////////////
