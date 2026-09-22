/*
 * MGUIEFlatToolBar.h
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


#ifndef __MGUIEFlatToolBar__
#define __MGUIEFlatToolBar__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <TROOT.h>
#include <TGFrame.h>
#include <TObjArray.h>

// MEGAlib libs:
#include "MString.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MGUIEFlatToolBar : public TGCompositeFrame
{
  // public interface:
 public:
  MGUIEFlatToolBar(const TGWindow *Parent, unsigned int w, unsigned int h, unsigned int Options);
  virtual ~MGUIEFlatToolBar();

  bool Add(const TGWindow* Associate, MString IconFile, int Id = -1, unsigned int Distance = 0, MString DistToolTip = "");

  // protected methods:
 protected:


  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  TObjArray* m_Pictures;
  TObjArray* m_Buttons;
  TObjArray* m_Layouts;


#ifdef ___CLING___
 public:
  ClassDef(MGUIEFlatToolBar, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
