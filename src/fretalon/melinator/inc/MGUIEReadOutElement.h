/*
 * MGUIEReadOutElement.h
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


#ifndef __MGUIEReadOutElement__
#define __MGUIEReadOutElement__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <TROOT.h>
#include "TGButton.h"
#include "TGWidget.h"
#include "TGPicture.h"
#include "TGToolTip.h"
#include "TGButtonGroup.h"

// MEGAlib libs:
#include "MReadOutElement.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! Button like element for changing the selected ROE plus indicators
class MGUIEReadOutElement : public TGTransientFrame
{
  // public interface:
 public:
  //! Standard constructor
  MGUIEReadOutElement(TGCompositeFrame* Parent, const MReadOutElement& ROE, unsigned int ID);
  //! Default destructor
  virtual ~MGUIEReadOutElement();

  //! Get the read-out element
  const MReadOutElement& GetReadOutElement() const { return m_ROE; }
  
  //! Set a quality factor representing the background color of the element
  void SetQuality(double Quality);
  
  //! Associate the button click to this window
  void Associate(const TGWindow* W) { m_TextButton->Associate(W); }
  
  // private methods:
 private:

  // protected members:
 protected:


  // private members:
 private:
  //! The readout element
  const MReadOutElement& m_ROE;
  //! The unique ID of the associated text button
  unsigned int m_ID;
  
  //! The text button
  TGTextButton* m_TextButton;

  //! Quality factor
  double m_Quality;
  
  //! The default background color
  Pixel_t m_DefaultBackgroundColor;
  
#ifdef ___CLING___
 public:
  ClassDef(MGUIEReadOutElement, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
