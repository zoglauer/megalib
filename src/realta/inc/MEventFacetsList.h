/*
 * MEventFacetsList.h
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


#ifndef __MEventFacetsList__
#define __MEventFacetsList__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include "TObject.h"
#include <TList.h>

// MEGAlib libs:
#include "MGlobal.h"

// Forward declarations:
class MEventFacets;

////////////////////////////////////////////////////////////////////////////////


class MEventFacetsList : public TObject
{
  // public interface:
 public:
  MEventFacetsList(UInt_t MaximumSize);
  ~MEventFacetsList();

  void SetMaximumSize(UInt_t MaximumSize);
  UInt_t GetSize();

  void Add(MEventFacets *Event);

  void RestartIterator(MEventFacets *Event = 0);
  MEventFacets* Next();

  // protected methods:
 protected:


  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  TList *m_List;

  UInt_t m_MaximumSize;

  MEventFacets *m_IterationEvent;



#ifdef ___CLING___
 public:
  ClassDef(MEventFacetsList, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
