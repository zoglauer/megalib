/*
 * MReadOutDataWithTiming.h
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


#ifndef __MReadOutDataWithTiming__
#define __MReadOutDataWithTiming__


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


//! The basic read-out data just consisting of one (ADC) value and a timing (not time) value 
class MReadOutDataWithTiming : public MReadOutData
{
  // public interface:
 public:
  //! Default constructor
  MReadOutDataWithTiming();
  //! Constructor given the data
  MReadOutDataWithTiming(double Data, double Timing);
  //! Simple default destructor
  virtual ~MReadOutDataWithTiming();

  //! Return true if this read-out data is of the given type
  virtual bool IsOfType(const MString& String) const;

  //! Clear the content of this read-out data element
  virtual void Clear();

  //! Return true is all values are zero
  virtual bool IsZero() const;

  //! Clone this data element - the returned element must be deleted
  virtual MReadOutDataWithTiming* Clone() const;

  //! Set the timing
  void SetTiming(double Timing) { m_Timing = Timing; }
  //! Get the timing
  double GetTiming() const { return m_Timing; }
  
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
  //! The timing
  double m_Timing;
  
  // private members:
 private:



#ifdef ___CLING___
 public:
  ClassDef(MReadOutDataWithTiming, 0) // no description
#endif

};

//! Streamify the read-out data
ostream& operator<<(ostream& os, const MReadOutDataWithTiming& R);

#endif


////////////////////////////////////////////////////////////////////////////////
