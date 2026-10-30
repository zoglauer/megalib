/*
 * MHitEventAnalyzer.h
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


#ifndef __MHitEventAnalyzer__
#define __MHitEventAnalyzer__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include "TObject.h"

// MEGAlib libs:
#include "MGlobal.h"

// Forward declarations:
class MHitEvent;
class MRawEventIncarnations;
class MPhysicalEvent;
class MDGeometryQuest;
class MGeometryBasic;


////////////////////////////////////////////////////////////////////////////////


class MHitEventAnalyzer : public TObject
{
  // public interface:
 public:
  MHitEventAnalyzer(MDGeometryQuest* Geo);
  ~MHitEventAnalyzer();

  Bool_t Analyze(MHitEvent* Event);
  MPhysicalEvent* GetPhysicalEvent();


  // protected methods:
 protected:


  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  MRawEventIncarnations *m_RawEvents;  // array of all "possible" events
  MDGeometryQuest *m_Geometry;
  MGeometryBasic *m_Geo;


#ifdef ___CLING___
 public:
  ClassDef(MHitEventAnalyzer, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
