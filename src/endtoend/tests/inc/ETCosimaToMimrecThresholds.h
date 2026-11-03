/*
 * ETCosimaToMimrecThresholds.h
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


#ifndef __ETCosimaToMimrecThresholds__
#define __ETCosimaToMimrecThresholds__


////////////////////////////////////////////////////////////////////////////////


// MEGAlib libs:
#include "MEndToEndTest.h"


////////////////////////////////////////////////////////////////////////////////


//! End-to-end test cosima -> revan -> mimrec with the COSI-like geometry Max.geo.setup: noise and trigger thresholds of the detector effects
//! * Mono lines below, around (27 to 33 keV), and above the thresholds of the active detector (noise threshold 15 keV, trigger threshold 30 keV)
//! * Lines far below the trigger threshold (20, 25, 27 keV) do not trigger at all: no event is reconstructed
//! * Lines above: the events of the sim file per fluence, the effective area of the peak, the fraction of events in the peak, and the fraction of events which lost
//!   a fluorescence photon (hit below the noise threshold) are compared with the reference values of the detector effects engine MEGAlib 1-4
class ETCosimaToMimrecThresholds : public MEndToEndTest
{
public:
  //! Default constructor
  ETCosimaToMimrecThresholds();
  //! Default destructor
  virtual ~ETCosimaToMimrecThresholds() {}

  //! Run all tests
  virtual bool RunTest();

};

#endif


////////////////////////////////////////////////////////////////////////////////
