/*
 * MSubModule.cxx
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


////////////////////////////////////////////////////////////////////////////////
//
// MSubModule
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MSubModule.h"

// Standard libs:

// ROOT libs:
#include "TSystem.h"

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MSubModule)
#endif


////////////////////////////////////////////////////////////////////////////////


MSubModule::MSubModule() : m_Name("Unnamed sub-module")
{
  // Construct an instance of MSubModule
}


////////////////////////////////////////////////////////////////////////////////


MSubModule::~MSubModule()
{
  // Delete this instance of MSubModule
}


////////////////////////////////////////////////////////////////////////////////


bool MSubModule::Initialize()
{
  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MSubModule::Clear()
{
}


////////////////////////////////////////////////////////////////////////////////


void MSubModule::Finalize()
{
}


////////////////////////////////////////////////////////////////////////////////


bool MSubModule::ReadXmlConfiguration(MXmlNode* Node)
{
  //! Read the configuration data from an XML node

  return true;
}


////////////////////////////////////////////////////////////////////////////////


MXmlNode* MSubModule::CreateXmlConfiguration(MXmlNode* Node)
{
  //! Create an XML node tree from the configuration

  /*
  MXmlNode* SomeTagNode = new MXmlNode(Node, "SomeTag", "SomeValue");
  */

  return Node;
}


// MSubModule.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
