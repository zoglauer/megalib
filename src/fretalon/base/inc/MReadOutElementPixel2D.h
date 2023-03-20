/*
 * MReadOutElementPixel2D.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MReadOutElementPixel2D__
#define __MReadOutElementPixel2D__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MReadOutElement.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! The read-out element of a 2D pixel detector.
//! In addition to the detector ID and the x and y numbering of the pixel is added.
class MReadOutElementPixel2D : public MReadOutElement
{
  // public interface:
 public:
  //! Read out element of a 2D pixel detector
  MReadOutElementPixel2D();
  //! Read out element of a 2D pixel detector
  MReadOutElementPixel2D(unsigned int DetectorID, unsigned int XPixelID, unsigned int YPixelID);
  //! Simple default destructor
  virtual ~MReadOutElementPixel2D();

  //! Clone this read-out element - the returned element must be deleted!
  virtual MReadOutElementPixel2D* Clone() const;

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
  
  //! Set the x pixel ID
  void SetXPixelID(unsigned int XPixelID) { m_XPixelID = XPixelID; }
  //! Get the x pixel ID
  unsigned int GetXPixelID() const { return m_XPixelID; }

  //! Set the y pixel ID
  void SetYPixelID(unsigned int YPixelID) { m_YPixelID = YPixelID; }
  //! Get the y pixel ID
  unsigned int GetYPixelID() const { return m_YPixelID; }

  //! Return the minimum number of hits which compose a good hit
  //! I.e. for a double strip it is two (n and p side), otherwise it is usually 1
  virtual unsigned int GetMinimumNumberOfReadOutsForGoodInteraction() const { return 2; }

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
  //! The x ID of the pixel
  unsigned int m_XPixelID;
  //! The y ID of the pixel
  unsigned int m_YPixelID;

  // private members:
 private:



#ifdef ___CLING___
 public:
  ClassDef(MReadOutElementPixel2D, 0) // no description
#endif

};

//! Streamify the read-out element
ostream& operator<<(ostream& os, const MReadOutElementPixel2D& R);

#endif


////////////////////////////////////////////////////////////////////////////////
