/*
 * MReadOutElement.h
 *
 * Copyright (C) by the MEGAlib contributors.
 *
 * This file is part of MEGAlib.
 *
 * MEGAlib is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * MEGAlib is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
 * License (License.md) for more details.
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */


#ifndef __MReadOutElement__
#define __MReadOutElement__


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


//! The basic read-out element just consisting of a single (detector) ID
class MReadOutElement
{
  // public interface:
 public:
  //! A basic read out element - only knows a detector
  MReadOutElement();
  //! A basic read out element - only knows a detector
  MReadOutElement(unsigned int DetectorID);
  //! Simple default destructor
  virtual ~MReadOutElement();

  //! Clone this data element - the returned element must be deleted!
  virtual MReadOutElement* Clone() const;

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
  //! Return true if this read-out element is of the same type as the given one
  virtual bool AreOfSameType(const MReadOutElement& ROE) const { return GetType().AreIdentical(ROE.GetType()); }
  
  //! Return the minimum number of hits which compose a good hit
  //! I.e. for a double strip it is two (n and p side), otherwise it is usually 1
  virtual unsigned int GetMinimumNumberOfReadOutsForGoodInteraction() const { return 1; }
  
  //! Set the detector ID
  void SetDetectorID(unsigned int DetectorID) { m_DetectorID = DetectorID; }
  //! Get the detector ID
  unsigned int GetDetectorID() const { return m_DetectorID; }
  
  //! Return the number of parsable elements
  virtual unsigned int GetNumberOfParsableElements() const;  
  //! Parse the data from the tokenizer 
  virtual bool Parse(const MTokenizer& T, unsigned int StartElement);
  //! Return the data as parsable string
  virtual MString ToParsableString(bool WithDescriptor = false) const; 
  
  //! Dump as a descriptive string
  virtual MString ToString() const;
  
  
  // protected methods:
 protected:

  // private methods:
 private:



  // protected members:
 protected:
  //! The detector ID
  unsigned int m_DetectorID;
  
  
  
  // private members:
 private:



#ifdef ___CLING___
 public:
  ClassDef(MReadOutElement, 0) // no description
#endif

};

//! Streamify the read-out element
ostream& operator<<(ostream& os, const MReadOutElement& R);

#endif


////////////////////////////////////////////////////////////////////////////////
