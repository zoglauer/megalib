/*
 * ETCosimaToMimrecImaging.h
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


#ifndef __ETCosimaToMimrecImaging__
#define __ETCosimaToMimrecImaging__


////////////////////////////////////////////////////////////////////////////////


// MEGAlib libs:
#include "MEndToEndTest.h"


////////////////////////////////////////////////////////////////////////////////


//! End-to-end test cosima -> revan -> mimrec with the COSI-like geometry Max.geo.setup: source positions
//! A 662 keV point source is simulated from several directions. For the reconstructed Compton events which absorb the full energy:
//! * The angular resolution measure (ARM) relative to the TRUE source direction (from the beam definition) peaks at zero and has a narrow core
//! * The true direction is clearly better than the directions around it (the check is sensitive)
//! * A back projection of the events on a grid of directions peaks at the true direction
//! * Mimrec: the image in spherical coordinates of the detector peaks at the true direction, and the ARM plot for the true position has the same content as the own ARM
//!   analysis with the peak at zero
//! * The reconstructed total energy is the true energy of the photon
class ETCosimaToMimrecImaging : public MEndToEndTest
{
public:
  //! Default constructor
  ETCosimaToMimrecImaging() : MEndToEndTest("ETCosimaToMimrecImaging") {}
  //! Default destructor
  virtual ~ETCosimaToMimrecImaging() {}

  //! Run all tests
  virtual bool Run();

};

#endif


////////////////////////////////////////////////////////////////////////////////
