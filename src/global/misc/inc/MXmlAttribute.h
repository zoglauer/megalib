/*
 * MXmlAttribute.h
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


#ifndef __MXmlAttribute__
#define __MXmlAttribute__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <cstdlib>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MXmlData.h"

// Forward declarations:
class MXmlNode;


////////////////////////////////////////////////////////////////////////////////


//! An XML attribute used by the lightweight MEGAlib XML writer/reader
class MXmlAttribute : public MXmlData
{
  // public interface:
 public:
  //! Default constructor
  MXmlAttribute();
  //! Constructor without a value
  MXmlAttribute(MXmlNode* MotherNode, const MString& Name);
  //! Constructor with a string value
  MXmlAttribute(MXmlNode* MotherNode, const MString& Name, const MString& Value);
  //! Constructor -- content is integer
  MXmlAttribute(MXmlNode* MotherNode, const MString& Name, int Value);
  //! Constructor -- content is unsigned integer
  MXmlAttribute(MXmlNode* MotherNode, const MString& Name, unsigned int Value);
  //! Constructor -- content is double
  MXmlAttribute(MXmlNode* MotherNode, const MString& Name, double Value);
  //! Constructor -- content is a boolean
  MXmlAttribute(MXmlNode* MotherNode, const MString& Name, bool Value);
  //! Default destructor
  virtual ~MXmlAttribute();

  //! Return the XML text
  virtual MString ToString();


  // protected methods:
 protected:


  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:


#ifdef ___CLING___
 public:
  ClassDef(MXmlAttribute, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
