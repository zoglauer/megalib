/*
 * MGUIEFlatToolBarButton.h
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


#ifndef __MGUIEFlatToolBarButton__
#define __MGUIEFlatToolBarButton__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include "TROOT.h"
#include "TGButton.h"
#include "TGWidget.h"
#include "TGPicture.h"
#include "TGToolTip.h"
#include "TGButtonGroup.h"


// MEGAlib libs:

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MGUIEFlatToolBarButton : public TGPictureButton
{
  // public interface:
 public:
  MGUIEFlatToolBarButton(const TGWindow* Parent, const TGPicture* Picture, int Id = -1);
  virtual ~MGUIEFlatToolBarButton();

  virtual void SetFlat(const bool IsFlat = true);
  virtual bool IsFlat() const;

  virtual void SetState(EButtonState state, bool);

  virtual bool HandleCrossing(Event_t *event);
  virtual bool HandleFocusChange(Event_t *event);

  // protected methods:
 protected:


  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  bool m_Flat;


#ifdef ___CLING___
 public:
  ClassDef(MGUIEFlatToolBarButton, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
