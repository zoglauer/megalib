/*
 * MReadOut.h
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


//! A single channel read out consisting of a read-out element and its associated data
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
  //! Return a reference to the read-out element
  MReadOutElement& GetReadOutElement() { return *m_ROE; }
  
  //! Set the read-out data
  void SetReadOutData(const MReadOutData& ROD);
   //! Return a const reference to the read-out data
  const MReadOutData& GetReadOutData() const { return *m_ROD; }
   //! Return a reference to the read-out data
  MReadOutData& GetReadOutData() { return *m_ROD; }
  
  //! Return the data as parsable string
  virtual MString ToParsableString(bool WithDescriptor = false, const MString& Keyword = "UH");
  
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


#ifdef ___CLING___
 public:
  ClassDef(MReadOut, 0) // no description
#endif

};

//! Streamify the read out 
ostream& operator<<(ostream& os, const MReadOut& RO);

#endif


////////////////////////////////////////////////////////////////////////////////
