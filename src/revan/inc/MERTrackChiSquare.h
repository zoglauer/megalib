/*
 * MERTrackChiSquare.h
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


#ifndef __MERTrackChiSquare__
#define __MERTrackChiSquare__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MGeometryRevan.h"
#include "MERTrack.h"


// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MERTrackChiSquare : public MERTrack
{
  // public interface:
 public:
  MERTrackChiSquare();
  virtual ~MERTrackChiSquare();

  //! Set all special parameters - this function should not rely on a previous 
  //! call to SetParameters()
  virtual bool SetSpecialParameters() { return true; }

  virtual MString ToString(bool CoreOnly = false) const;

  // protected methods:
 protected:

  virtual bool EvaluateTracks(MRERawEvent* ER);

  // private methods:
 private:



  // protected members:
 protected:

  // private members:
 private:


#ifdef ___CLING___
 public:
  ClassDef(MERTrackChiSquare, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
