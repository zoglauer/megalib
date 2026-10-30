/*
 * MSimEventLoader.h
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


#ifndef __MSimEventLoader__
#define __MSimEventLoader__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include "TObject.h"

// MEGAlib libs:
#include "MGlobal.h"
#include "MSimEvent.h"
#include "MDGeometryQuest.h"

// Standard libs:
#include <iostream>
#include <fstream>
#include <stdlib.h>

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MSimEventLoader : public TObject
{
  // public interface:
 public:
  MSimEventLoader();
  MSimEventLoader(MString FileName);
  ~MSimEventLoader();

  void Init();

  void SetFileName(MString FileName);
  void SetGeometry(MDGeometryQuest *Geo);

  MSimEvent* GetNextEvent();
  bool IsFileOpen();

  double GetProgress();


  // protected methods:
 protected:
  bool OpenFile();

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  MString m_FileName;            // Name of the file

  fstream *m_FileStream;         // pointer to the filestream
  int m_FileLength;

  int m_LineLength;            // length of the line-buffer
  char *m_LineBuffer;          // buffer for one event as ascii-text

  MSimEvent* m_Event;

  MDGeometryQuest *m_Geometry;


#ifdef ___CLING___
 public:
  ClassDef(MSimEventLoader, 0) // loads simulated events
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
