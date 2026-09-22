/*
 * MCTrackingAction.hh
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
 * Description of this class
 *
 */

#ifndef ___MCTrackingAction___
#define ___MCTrackingAction___

// Geant4:
#include "G4UserTrackingAction.hh"
#include "G4Track.hh"

// Cosima:

// MEGAlib:

// Standard lib:

// Forward declarations:


/******************************************************************************/

class MCTrackingAction : public G4UserTrackingAction
{
  // public interface:
public:
  /// Default constructor
  MCTrackingAction();
  /// Default destructor
  virtual ~MCTrackingAction();

  /// Actions before tracking starts
  virtual void PreUserTrackingAction(const G4Track* Track);

  /// Set the number of generated primaries in this EVENT (not track!)
  void SetNGeneratedParticles(int NGeneratedParticles);

  // protected methods:
protected:
  

  // protected members:
protected:


  // private members:
private:
  /// Number of generated primaries in this EVENT (not track!)
  int m_NPrimaries;

};

#endif


/*
 * MCTrackingAction.hh: the end...
 ******************************************************************************/
