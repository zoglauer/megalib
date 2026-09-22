/*
 * MXmlData.h
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


#ifndef __MXmlData__
#define __MXmlData__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <cstdlib>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MTime.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MXmlData
{
  // public interface:
 public:
  //! Default constructor
  MXmlData();
  //! Constructor -- no values
  MXmlData(const MString& Name);
  //! Constructor -- content is string
  MXmlData(const MString& Name, const MString& Value);
  //! Constructor -- content is a time
  MXmlData(const MString& Name, const MTime& Value);
  //! Constructor -- content is integer
  MXmlData(const MString& Name, int Value);
  //! Constructor -- content is long
  MXmlData(const MString& Name, long Value);
  //! Constructor -- content is unsigned integer
  MXmlData(const MString& Name, unsigned int Value);
  //! Constructor -- content is unsigned long
  MXmlData(const MString& Name, unsigned long Value);
  //! Constructor -- content is double
  MXmlData(const MString& Name, double Value);
  //! Constructor -- content is a boolean
  MXmlData(const MString& Name, bool Value);
  //! Default destructor
  virtual ~MXmlData();

  //! Reset the node
  virtual void Clear();

  //! Set the name of the node
  void SetName(const MString& Name) { m_Name = Name; }
  //! Get the name of the node
  MString GetName() const { return m_Name; }

  //! Set the value of the node
  void SetValue(const MString& Value) { m_Value = Value; }
  //! Return the value of the node
  MString GetValue() const { return m_Value; }

  //! Return the value of the node
  MString GetValueAsString() const { return m_Value; }
  //! Return the value of the node as time
  MTime GetValueAsTime() const { MTime T(0); T.Set(m_Value); return T; }
  //! Return the value of the node as int
  int GetValueAsInt() const { return atoi(m_Value); }
  //! Return the value of the node as long
  long GetValueAsLong() const { return atol(m_Value); }
  //! Return the value of the node as unsigned int
  unsigned int GetValueAsUnsignedInt() const { return atoi(m_Value); }
  //! Return the value of the node as unsigned long
  unsigned int GetValueAsUnsignedLong() const { return strtoul(m_Value, NULL, 0); }
  //! Return the value of the node as double
  double GetValueAsDouble() const { return atof(m_Value); }
  //! Return the value of the node as boolean
  bool GetValueAsBoolean() const;


  // protected methods:
 protected:

  // private methods:
 private:



  // protected members:
 protected:
  //! The name of the node
  MString m_Name;
  //! The value of the node (if any)
  MString m_Value;


  // private members:
 private:


#ifdef ___CLING___
 public:
  ClassDef(MXmlData, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
