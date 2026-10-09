/*
 * MXmlDocument.h
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


#ifndef __MXmlDocument__
#define __MXmlDocument__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <fstream>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MXmlNode.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! A lightweight XML document wrapper for MEGAlib-written XML files
class MXmlDocument : public MXmlNode
{
  // public interface:
 public:
  //! Default constructor
  MXmlDocument();
  //! Constructor with the document root name
  MXmlDocument(const MString& Name);
  //! Default destructor
  virtual ~MXmlDocument();

  //! Load an XML document
  bool Load(MString FileName);
  //! Save an XML document
  bool Save(MString FileName);

  // protected methods:
 protected:
  //MXmlDocument() {};
  //MXmlDocument(const MXmlDocument& XmlDocument) {};

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:


#ifdef ___CLING___
 public:
  ClassDef(MXmlDocument, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
