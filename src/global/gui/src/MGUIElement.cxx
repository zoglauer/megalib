/*
 * MGUIElement.cxx
 *
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 *
 * This code implementation is the intellectual property of
 * Andreas Zoglauer.
 *
 * By copying, distributing or modifying the Program (or any work
 * based on the Program) you indicate your acceptance of this statement,
 * and all its terms.
 *
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
