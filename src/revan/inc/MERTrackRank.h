/*
 * MERTrackRank.h
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


#ifndef __MERTrackRank__
#define __MERTrackRank__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MERTrack.h"

// Forward declarations:
class MRETrack;
class MRERawEvent;


////////////////////////////////////////////////////////////////////////////////


class MERTrackRank : public MERTrack
{
  // public interface:
 public:
  MERTrackRank();
  virtual ~MERTrackRank();

  //! Set all special parameters - this function should not rely on a previous 
  //! call to SetParameters()
  virtual bool SetSpecialParameters() { return true; }

  virtual MString ToString(bool CoreOnly = false) const;


  // protected methods:
 protected:
  virtual bool EvaluateTracks(MRERawEvent* ER);
  virtual bool EvaluateTrack(MRETrack* Track);

  virtual void SortByTrackQualityFactor(MRawEventIncarnations* List);


  // private methods:
 private:



  // protected members:
 protected:


#ifdef ___CLING___
 public:
  ClassDef(MERTrackRank, 0)
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
