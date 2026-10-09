/*
 * ETCosimaToMimrecOrientation.h
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


#ifndef __ETCosimaToMimrecOrientation__
#define __ETCosimaToMimrecOrientation__


////////////////////////////////////////////////////////////////////////////////


// MEGAlib libs:
#include "MEndToEndTest.h"


////////////////////////////////////////////////////////////////////////////////


//! End-to-end test cosima -> revan -> mimrec with the COSI-like geometry Max.geo.setup: orientation of the detector in the Galactic sky
//! A source is fixed in Galactic coordinates, the detector orientation comes from an orientation file (OG: time, x axis latitude and longitude, z axis latitude and longitude).
//! The direction of the source in the detector coordinate system is known from vector math in Galactic Cartesian coordinates:
//! * The polar angle (from the z axis) of the direction to the source is the angle between the z axis of the detector and the source, at the time of every event
//!   - for static pointings, and for a detector which slews with a constant rate along the Galactic equator past the source
//! * If the x axis points to the source (z perpendicular to it), the azimuth is zero (independent of the handedness of the coordinate system)
//! * While slewing, the azimuth of the source flips by 180 degrees when the source crosses the z axis
//! * Mimrec: for the static pointings the ARM plot for the direction of the source in the detector frame (from the true direction of the sim file) peaks at zero
//!   and has the same content as the own ARM analysis
//! * The reconstructed Compton events are consistent with the true (time dependent) direction of the source: the ARM peaks at zero
class ETCosimaToMimrecOrientation : public MEndToEndTest
{
public:
  //! Default constructor
  ETCosimaToMimrecOrientation() : MEndToEndTest("ETCosimaToMimrecOrientation") {}
  //! Default destructor
  virtual ~ETCosimaToMimrecOrientation() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Return the Galactic unit vector of a latitude and longitude in degrees
  MVector Galactic(double Latitude, double Longitude);
};

#endif


////////////////////////////////////////////////////////////////////////////////
