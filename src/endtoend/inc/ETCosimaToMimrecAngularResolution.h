/*
 * ETCosimaToMimrecAngularResolution.h
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


#ifndef __ETCosimaToMimrecAngularResolution__
#define __ETCosimaToMimrecAngularResolution__


////////////////////////////////////////////////////////////////////////////////


// MEGAlib libs:
#include "MEndToEndTest.h"


////////////////////////////////////////////////////////////////////////////////


//! End-to-end test cosima -> revan -> mimrec with the COSI-like geometry Max.geo.setup: the angular resolution measure (ARM)
//! * ARM contains all resolutions of the detector (energy, position, depth) and the pairing and ordering of the hits, thus it is the most sensitive indicator for changes in
//!   the detector effects. Mono lines at 200, 500, and 1000 keV from two directions (0 and 40 degree polar angle)
//! * The Compton events of the full energy peak: effective area, ARM FWHM, containment (68% and 90%) of |ARM|, and the mean ARM are compared with the reference values
//!   of the detector effects engine MEGAlib 1-4 (the references are the mean of independent runs - the Geant4 version changes the details of the simulation)
class ETCosimaToMimrecAngularResolution : public MEndToEndTest
{
public:
  //! Default constructor
  ETCosimaToMimrecAngularResolution();
  //! Default destructor
  virtual ~ETCosimaToMimrecAngularResolution() {}

  //! Run all tests
  virtual bool Run();

};

#endif


////////////////////////////////////////////////////////////////////////////////
