/*
 * MCRegion.hh
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



/******************************************************************************
 *
 * This class stores a Geant4 region
 *
 */

#ifndef ___MCRegion___
#define ___MCRegion___

// Geant4:
#include "globals.hh"

// ROOT:
#include "MString.h"

// Cosima:

// MEGAlib:

// Standard lib:

// Forward declarations:


/******************************************************************************/

class MCRegion
{
  // public interface:
public:
  /// Default constructor
  MCRegion(const MString& Name = "");
  /// Default destructor
  virtual ~MCRegion();

  /// Set the name of this region
  void SetName(const MString& Name) { m_Name = Name; }
  /// Return the name of this region
  const MString& GetName() const { return m_Name; }

  /// Set the root logical volume name for the region.
  /// The name must be identical with a name in the Geomega geometry, 
  /// which is not a virtual volume!
  bool SetVolumeName(const MString& VolumeName) { m_VolumeName = VolumeName; return true; }
  /// Retrieve the root logical volume name
  const MString& GetVolumeName() const { return m_VolumeName; }

  /// Set the production cut for the region
  bool SetRangeCut(const double RangeCut) { m_RangeCut = RangeCut; return true; }
  /// Retrieve the production cut for the region
  double GetRangeCut() const { return m_RangeCut; }

  /// Set the flag to cut all secondaries
  bool SetCutAllSecondaries(const bool CutAllAllSecondaries) { m_CutAllAllSecondaries = CutAllAllSecondaries; return true; }
  /// Retrieve the flag to cut all secondaries
  bool GetCutAllSecondaries() const { return m_CutAllAllSecondaries; }

  // protected methods:
protected:
  

  // protected members:
protected:


  // private members:
private:
  /// Name of the region
  MString m_Name;
  /// Name of the root logical volume
  MString m_VolumeName;
  /// The production cut for the region
  double m_RangeCut;
  /// Flag to indicate if all secondaries should be cut
  bool m_CutAllAllSecondaries;
};

#endif


/*
 * MCRegion.hh: the end...
 ******************************************************************************/
