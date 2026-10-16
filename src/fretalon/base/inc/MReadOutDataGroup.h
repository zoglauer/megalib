/*
 * MReadOutDataGroup.h
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


#ifndef __MReadOutDataGroup__
#define __MReadOutDataGroup__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// Standard libs:
#include <deque>

// ROOT libs:
#include "TH1.h"
#include "TF1.h"

// MEGAlib libs:
#include "MGlobal.h"
#include "MReadOutData.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! A group of read-out datas 
class MReadOutDataGroup
{
  // public interface:
 public:
  //! Default constructor
  MReadOutDataGroup();
  //! Default constructor with name
  MReadOutDataGroup(const MString& Name);
  //! The copy constructor 
  MReadOutDataGroup(const MReadOutDataGroup& ReadOutDataGroup);
  //! The move constructor (needed so that vector reallocation/insertion does not deep-copy all hits)
  MReadOutDataGroup(MReadOutDataGroup&& RODG) noexcept;
  //! Default destructor
  virtual ~MReadOutDataGroup();

  //! Assignment operator
  MReadOutDataGroup& operator=(const MReadOutDataGroup& RO);
  //! Move assignment operator
  MReadOutDataGroup& operator=(MReadOutDataGroup&& RODG) noexcept;

  //! Return the name of the group
  MString GetName() const { return m_Name; }
  
  //! Add this read-out data 
  void Add(const MReadOutData& ROD);

  //! Move the content from the given into this read-out data group
  void Move(MReadOutDataGroup& RODG);
    
  //! Return the number of available read-out data's
  unsigned int GetNumberOfReadOutDatas() const { return m_ADCs.size() + m_RODs.size(); }
  
  //! Return a specific read-out data, in case it doesn't exist throw the exception MExceptionIndexOutOfBounds
  //! ATTENTION: Plain ADC values are stored in a compact way. For those the returned reference points to a per-thread 
  //! temporary, which is only valid until the next call of GetReadOutData() in the same thread --- use it immediately
  MReadOutData& GetReadOutData(unsigned int i);
  //! Return a specific read-out data, in case it doesn't exist throw the exception MExceptionIndexOutOfBounds
  const MReadOutData& GetReadOutData(unsigned int i) const;
  
  //! Dump a string
  virtual MString ToString() const;
  
  //! Switch the compact storage of plain ADC values on or off (global, default on) --- only affects data added afterwards
  static void SetCompactStorage(bool Compact);
  //! Return true if plain ADC values are stored compactly
  static bool GetCompactStorage();
  
  // protected methods:
 protected:

   
  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  //! Name of this group
  MString m_Name;
   
  //! Plain ADC values (without any wrapped data) are stored compactly: 4 bytes instead of ~45 bytes per entry
  //! They are at the indices [0..m_ADCs.size()-1], followed by the m_RODs
  deque<unsigned int> m_ADCs;
  //! A list of all other data
  vector<MReadOutData*> m_RODs;
  
  
#ifdef ___CLING___
 public:
  ClassDef(MReadOutDataGroup, 0) // no description
#endif

};

//! Streamify the read-out data group
ostream& operator<<(ostream& os, const MReadOutDataGroup& R);

#endif


////////////////////////////////////////////////////////////////////////////////
