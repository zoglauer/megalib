/*
 * MHitEvent.h
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


#ifndef __MHitEvent__
#define __MHitEvent__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <TROOT.h>
#include <TObjArray.h>

// MEGAlib libs:
#include "MGlobal.h"

// Forward declarations:
class MHitData;

////////////////////////////////////////////////////////////////////////////////


class MHitEvent : public TObject
{
  // public interface:
 public:
  MHitEvent();
  ~MHitEvent();

  void AddHit(MHitData* Hit);
  Int_t GetNHits();
  MHitData* GetHitAt(Int_t i);

  void SetID(UInt_t ID);
  UInt_t GetID();

  // protected methods:
 protected:


  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  TObjArray* m_Hits;

  Int_t m_ID;


#ifdef ___CLING___
 public:
  ClassDef(MHitEvent, 1) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
