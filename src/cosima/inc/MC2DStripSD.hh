/*
 * MC2DStripSD.hh
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
 * Class representing the sensitive detector of a strip detector
 *
 */

#ifndef ___MC2DStripSD___
#define ___MC2DStripSD___

// Geant4:
#include "G4VSensitiveDetector.hh"
#include "globals.hh"

// Cosima:
#include "MC2DStripHit.hh"
#include "MCSD.hh"

// Forward declarations:
class G4HCofThisEvent;
class G4Step;

/******************************************************************************/


class MC2DStripSD : public MCSD
{
  // public interface:
public:
  /// Default constructor: Sets the name, type and default values for geometry
  MC2DStripSD(G4String Name);
  /// Default destructor: empty
  virtual ~MC2DStripSD();
  
  /// Initialize the hit collection
  void Initialize(G4HCofThisEvent*);
  /// Analyze and store the hits
  G4bool ProcessHits(G4Step* Step, G4TouchableHistory* ROHist);
  /// Generate and store the hits 
  G4bool PostProcessHits(const G4Step* Step);
  /// Housekeeping
  void EndOfEvent(G4HCofThisEvent*);

  /// Size of the wafer
  void SetSize(double xSize, double ySize) { m_XSize = xSize; m_YSize = ySize; }
  /// Offset from the edge to the first strip
  void SetOffset(double xOffset, double yOffset) { m_XOffset = xOffset; m_YOffset = yOffset; }
  /// Distance between the strips
  void SetPitch(double xPitch, double yPitch) { m_XPitch = xPitch; m_YPitch = yPitch; }
  /// Number of strips
  void SetNStrips(int xNStrips, int yNStrips) { m_XNStrips = xNStrips; m_YNStrips = yNStrips; }
  /// Set whether or not this is a 3D strip (z-resolution or not)
  void SetIs3D(bool Is3D = true);

  /// Set a unique position on the guard ring 
  void SetUniqueGuardringPosition(G4ThreeVector Pos) { m_UniqueGuardringPosition = Pos; }

  // protected methods:
protected:
  

  // protected members:
protected:


  // private members:
protected:
  /// Container for the hits of one event
  MC2DStripHitsCollection* m_HitCollection;      

  /// x-Size of the volume of this detector
  double m_XSize;
  /// x-Offset from the edge to the sensitive volume
  double m_XOffset;
  /// x-Distance between the strips
  double m_XPitch;
  /// x-Number of strips:
  int m_XNStrips;

  /// y-Size of the volume of this detector
  double m_YSize;
  /// y-Offset from the edge to the sensitive volume
  double m_YOffset;
  /// y-Distance between the strips
  double m_YPitch;
  /// y-Number of strips:
  int m_YNStrips;

  /// True if this has z-resolution:
  bool m_Is3D;

  /// A unique position in the guardring
  G4ThreeVector m_UniqueGuardringPosition;

};

#endif


/*
 * MC2DStripSD.hh: the end...
 ******************************************************************************/





