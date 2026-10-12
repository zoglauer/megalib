/*
 * ETCosimaToMimrecPositionResolution.h
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


#ifndef __ETCosimaToMimrecPositionResolution__
#define __ETCosimaToMimrecPositionResolution__


////////////////////////////////////////////////////////////////////////////////


// MEGAlib libs:
#include "MEndToEndTest.h"


////////////////////////////////////////////////////////////////////////////////


//! End-to-end test cosima -> revan -> mimrec with the COSI-like geometry Max.geo.setup: position resolution of the detector effects
//! * Mono lines at 100 and 200 keV: photo events with a single hit and the full energy, compared with the position of the photo absorption in the sim file
//! * The strips determine x and y: the residual is uniformly distributed over the strip, the RMS is the strip pitch (0.185 cm) over the square root of twelve
//! * The depth is smeared with the depth resolution of the geometry (0.02 cm)
//! * The widths, the mean residuals, and the fraction of events which are not in the strip of the interaction are compared with the reference values of the
//!   detector effects engine MEGAlib 1-4
class ETCosimaToMimrecPositionResolution : public MEndToEndTest
{
public:
  //! Default constructor
  ETCosimaToMimrecPositionResolution();
  //! Default destructor
  virtual ~ETCosimaToMimrecPositionResolution() {}

  //! Run all tests
  virtual bool Run();

};

#endif


////////////////////////////////////////////////////////////////////////////////
