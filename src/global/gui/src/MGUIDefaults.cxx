/*
 * MGUIDefaults.cxx
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
// MGUIDefaults
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MGUIDefaults.h"

// Standard libs:

// ROOT libs:
#include "TGClient.h"
#include "TGResourcePool.h"
#include "TEnv.h"

// MEGAlib libs:
#include "MStreams.h"
#include "MSettingsGlobal.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MGUIDefaults)
#endif


////////////////////////////////////////////////////////////////////////////////


//! Get an instance of this singleton
MGUIDefaults* MGUIDefaults::GetInstance()
{
  static MGUIDefaults D; 
  
  return &D;
}


////////////////////////////////////////////////////////////////////////////////



MGUIDefaults::MGUIDefaults()
{
  // Construct an instance of MGUIDefaults
  
  MSettingsGlobal S;
  S.Read();
  if (S.GetFontScaler() == "tiny") {
    m_DefaultFontSize = 8;
  } else if (S.GetFontScaler() == "small") {
    m_DefaultFontSize = 10;
  } else if (S.GetFontScaler() == "normal") {
    m_DefaultFontSize = 12;
  } else if (S.GetFontScaler() == "large") {
    m_DefaultFontSize = 18;
  } else if (S.GetFontScaler() == "huge") {
    m_DefaultFontSize = 24;
  } else if (S.GetFontScaler() == "gigantic") {
    m_DefaultFontSize = 36;
  } else {
    mout<<"Error: Unknow font scaler: "<<S.GetFontScaler()<<". Expected: normal, large, huge, gigantic"<<endl;
    m_DefaultFontSize = 12;
  }
  
  // Change the global default font:
  MString FontName;
  FontName = "-*-helvetica-medium-r-*-*-";
  FontName += m_DefaultFontSize;
  FontName += "-*-*-*-*-*-iso8859-1";
  gEnv->SetValue("Gui.DefaultFont", FontName);
  gEnv->SetValue("Gui.MenuFont", FontName);
  gEnv->SetValue("Gui.DocPropFont", FontName);
  gEnv->SetValue("Gui.IconFont", FontName);
  gEnv->SetValue("Gui.StatusFont", FontName);
  
  FontName = "-*-helvetica-bold-r-*-*-";
  FontName += m_DefaultFontSize;
  FontName += "-*-*-*-*-*-iso8859-1";
  gEnv->SetValue("Gui.MenuHiFont", FontName);
  
  FontName = "-*-courier-medium-r-*-*-";
  FontName += m_DefaultFontSize;
  FontName += "-*-*-*-*-*-iso8859-1";
  gEnv->SetValue("Gui.DocFixedFont", FontName);
  
  m_NormalMediumFont = 0;
  m_NormalBoldFont = 0;
  m_ItalicMediumFont = 0;
  m_ItalicBoldFont = 0;
}


////////////////////////////////////////////////////////////////////////////////


MGUIDefaults::~MGUIDefaults()
{
  // Delete this instance of MGUIDefaults
}


////////////////////////////////////////////////////////////////////////////////


//! Get the default font
const TGFont* MGUIDefaults::GetNormalMediumFont()
{ 
  if (m_NormalMediumFont != 0) return m_NormalMediumFont;
  
  MString FontName;
  FontName = "-*-helvetica-medium-r-*-*-";
  FontName += m_DefaultFontSize;
  FontName += "-*-*-*-*-*-iso8859-1";
  m_NormalMediumFont = gClient->GetFont(FontName);
  if (!m_NormalMediumFont) {
    mout<<"ERROR: Having trouble to initialize the default font: "<<FontName<<endl; 
    m_NormalMediumFont = gClient->GetResourcePool()->GetDefaultFont();
  }

  return m_NormalMediumFont;
}


////////////////////////////////////////////////////////////////////////////////


//! Get the emphasized font
const TGFont* MGUIDefaults::GetNormalBoldFont()
{ 
  if (m_NormalBoldFont != 0) return m_NormalBoldFont;
  
  MString FontName;
  FontName = "-*-helvetica-bold-r-*-*-";
  FontName += m_DefaultFontSize;
  FontName += "-*-*-*-*-*-iso8859-1";
  m_NormalBoldFont = gClient->GetFont(FontName);
  if (!m_NormalBoldFont) {
    mout<<"ERROR: Having trouble to initialize the default font: "<<FontName<<endl; 
    m_NormalBoldFont = gClient->GetResourcePool()->GetDefaultFont();
  }

  return m_NormalBoldFont;
}


////////////////////////////////////////////////////////////////////////////////


//! Get the italic medium font
const TGFont* MGUIDefaults::GetItalicMediumFont()
{ 
  if (m_ItalicMediumFont != 0) return m_ItalicMediumFont;
  
  MString FontName;
  FontName = "-*-helvetica-medium-o-*-*-";
  FontName += m_DefaultFontSize;
  FontName += "-*-*-*-*-*-iso8859-1";
  m_ItalicMediumFont = gClient->GetFont(FontName);
  if (!m_ItalicMediumFont) {
    mout<<"ERROR: Having trouble to initialize the default font: "<<FontName<<endl; 
    m_ItalicMediumFont = gClient->GetResourcePool()->GetDefaultFont();
  }

  return m_ItalicMediumFont;
}


////////////////////////////////////////////////////////////////////////////////


//! Get the italic bold font
const TGFont* MGUIDefaults::GetItalicBoldFont()
{ 
  if (m_ItalicBoldFont != 0) return m_ItalicBoldFont;
  
  MString FontName;
  FontName = "-*-helvetica-bold-o-*-*-";
  FontName += m_DefaultFontSize;
  FontName += "-*-*-*-*-*-iso8859-1";
  m_ItalicBoldFont = gClient->GetFont(FontName);
  if (!m_ItalicBoldFont) {
    mout<<"ERROR: Having trouble to initialize the default font: "<<FontName<<endl; 
    m_ItalicBoldFont = gClient->GetResourcePool()->GetDefaultFont();
  }

  return m_ItalicBoldFont;
}


// MGUIDefaults.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
