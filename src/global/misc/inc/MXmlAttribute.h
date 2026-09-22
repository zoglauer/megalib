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


class MXmlAttribute : public MXmlData
{
  // public interface:
 public:
  //! Default constructor
  MXmlAttribute();
  //! Constructor -- no values
  MXmlAttribute(MXmlNode* MotherNode, MString Name);
  //! Constructor -- content is string
  MXmlAttribute(MXmlNode* MotherNode, MString Name, MString Value);
  //! Constructor -- content is integer
  MXmlAttribute(MXmlNode* MotherNode, MString Name, int Value);
  //! Constructor -- content is unsigned integer
  MXmlAttribute(MXmlNode* MotherNode, MString Name, unsigned int Value);
  //! Constructor -- content is double
  MXmlAttribute(MXmlNode* MotherNode, MString Name, double Value);
  //! Constructor -- content is a boolean
  MXmlAttribute(MXmlNode* MotherNode, MString Name, bool Value);
  //! Default destructor
  virtual ~MXmlAttribute();

  //! Returns the XML text
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
