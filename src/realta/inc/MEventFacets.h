/*
 * MEventFacets.h
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


#ifndef __MEventFacets__
#define __MEventFacets__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include "TObject.h"

// MEGAlib libs:
#include "MGlobal.h"

// Forward declarations:
class MHitEvent;
class MPhysicalEvent;
class MBPData;

////////////////////////////////////////////////////////////////////////////////


class MEventFacets : public TObject
{
  // public interface:
 public:
  MEventFacets();
  ~MEventFacets();

  void SetHitEvent(MHitEvent *Event);
  MHitEvent* GetHitEvent();

  void SetPhysicalEvent(MPhysicalEvent *Event);
  MPhysicalEvent* GetPhysicalEvent();

  void SetBackprojectionEvent(MBPData *Event);
  MBPData* GetBackprojectionEvent();

  Bool_t IsFullyAnalyzed();


  // protected methods:
 protected:


  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  MHitEvent *m_HitEvent;
  MPhysicalEvent *m_PhysicalEvent;
  MBPData *m_BackprojectionEvent;

  Bool_t m_FullyAnalyzed;


#ifdef ___CLING___
 public:
  ClassDef(MEventFacets, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
