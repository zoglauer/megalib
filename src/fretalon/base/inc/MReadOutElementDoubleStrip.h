/*
 * MReadOutElementDoubleStrip.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MReadOutElementDoubleStrip__
#define __MReadOutElementDoubleStrip__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MReadOutElementStrip.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! The read-out element of a double-sided strip detector. 
//! In addition to the detector ID and the strip ID from the base clases it also
//! contains the side (positive or negative).
class MReadOutElementDoubleStrip : public MReadOutElementStrip
{
  // public interface:
 public:
  //! Read out element of a DOUBLE-SIDED strip detector  
  MReadOutElementDoubleStrip(); 
  //! Read out element of a DOUBLE-SIDED strip detector  
  MReadOutElementDoubleStrip(unsigned int DetectorID, unsigned int StripID, bool IsPositiveStrip);
  //! Simple default destructor
  virtual ~MReadOutElementDoubleStrip();

  //! Clone this read-out element - the returned element must be deleted!
  virtual MReadOutElementDoubleStrip* Clone() const;

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
  
  //! Return the minimum number of hits which compose a good hit
  //! I.e. for a double strip it is two (n and p side), otherwise it is usually 1
  virtual unsigned int GetMinimumNumberOfReadOutsForGoodInteraction() const { return 2; }

  //! Set the strip type (positive equals the lowest voltage value side of the detector)
  void IsPositiveStrip(bool IsPositiveStrip) { m_IsLowVoltageStrip = IsPositiveStrip; }
  //! Return the strip type (positive equals the lowest voltage value side of the detector)
  bool IsPositiveStrip() const { return m_IsLowVoltageStrip; }

  //! Is this the-low voltage strip (= negative in old nomenclature)
  void IsLowVoltageStrip(bool IsLowVoltageStrip) { m_IsLowVoltageStrip = IsLowVoltageStrip; }
  //! Return true if this is a low-voltage strip
  bool IsLowVoltageStrip() const { return m_IsLowVoltageStrip; }


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
  //! The side of double sided strip detector
  bool m_IsLowVoltageStrip;

  // private members:
 private:



#ifdef ___CLING___
 public:
  ClassDef(MReadOutElementDoubleStrip, 0) // no description
#endif

};

//! Streamify the read-out element
ostream& operator<<(ostream& os, const MReadOutElementDoubleStrip& R);

#endif


////////////////////////////////////////////////////////////////////////////////
