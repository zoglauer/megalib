/*
 * MPointSourceSelector.h
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


#ifndef __MPointSourceSelector__
#define __MPointSourceSelector__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include "TObject.h"
#include <MString.h>
#include <TObjArray.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MParser.h"
#include "MPointSource.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MPointSourceSelector : MParser
{
  // public interface:
 public:
  MPointSourceSelector();
  virtual ~MPointSourceSelector();

  //! Open the file name and read the header
  virtual bool Open(MString FileName, unsigned int Way = 0);

  bool LoadList(MString FileName);

  void AddPointSource(const MPointSource& PointSource);
  MPointSource GetPointSourceAt(unsigned int i) const;
  unsigned int GetNPointSources() const;

  MPointSource& operator[] (unsigned i);

  // protected methods:
 protected:
  bool TokenizeLine(MTokenizer& T);


  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  vector<MPointSource> m_List;

  friend ostream& operator<<(ostream& os, const MPointSourceSelector& P);


#ifdef ___CLING___
 public:
  ClassDef(MPointSourceSelector, 0) // loads and/or selects pointsources out of a MPointSourceList
#endif

};

#endif

ostream& operator<<(ostream& os, const MPointSourceSelector& P);


////////////////////////////////////////////////////////////////////////////////
