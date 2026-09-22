/*
 * MRECluster.h
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


#ifndef __MRECluster__
#define __MRECluster__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MRESE.h"

// Forward declarations:
class MString;

class MREHit;


////////////////////////////////////////////////////////////////////////////////


class MRECluster : public MRESE
{
  // Public Interface:
 public:
  MRECluster();
  MRECluster(MRECluster* Cluster);
  virtual ~MRECluster();

  double ComputeMinDistance(MRESE *RESE);
  MVector ComputeMinDistanceVector(MRESE *RESE);
  bool AreAdjacent(MRESE* R, double Sigma = 3, int Level = 1);

  void AddRESE(MRESE *RESE);
  MRESE* RemoveRESE(MRESE *RESE);

  MString ToString(bool WithLink = true, int Level = 0);
  MRECluster* Duplicate();

  bool IsValid();

  // protected methods:
 protected:
  void AddHit(MREHit *Hit);


#ifdef ___CLING___
 public:
  ClassDef(MRECluster, 0) // a cluster of hits
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
