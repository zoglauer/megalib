/*
 * MGUIElement.cxx
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
// MGUIElement
//
//
// This class is an elementary GUI-widget:
//
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MGUIElement.h"

// Standard libs:

// ROOT libs:
#include <TGFont.h>
#include <TGResourcePool.h>

// MEGAlib libs:
#include "MGUIDefaults.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MGUIElement)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIElement::MGUIElement(const TGWindow* Parent, unsigned int Options) :
  TGCompositeFrame(Parent, 100, 100, Options)
{
  // Standard constructor

  m_EmphasizedFont = MGUIDefaults::GetInstance()->GetNormalBoldFont()->GetFontStruct();
  m_FontScaler = MGUIDefaults::GetInstance()->GetFontScaler();

  m_IsEnabled = true;
  m_WrapLength = 999;
}


////////////////////////////////////////////////////////////////////////////////


MGUIElement::~MGUIElement()
{
  // Destruct this instance of MGUIElement
}


// MGUIElement.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
