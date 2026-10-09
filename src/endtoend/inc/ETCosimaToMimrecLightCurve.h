/*
 * ETCosimaToMimrecLightCurve.h
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


#ifndef __ETCosimaToMimrecLightCurve__
#define __ETCosimaToMimrecLightCurve__


////////////////////////////////////////////////////////////////////////////////


// MEGAlib libs:
#include "MEndToEndTest.h"


////////////////////////////////////////////////////////////////////////////////


//! End-to-end test cosima -> revan -> mimrec with the COSI-like geometry Max.geo.setup: time structure
//! * A light curve (relative levels 1, 4, 1 in the intervals 0-30 s, 30-50 s, 50-100 s) is normalized so that its mean is the flux: the number of generated
//!   particles is flux * area * time, and the triggered events are distributed over the intervals as the integral of the light curve
//! * The arrival times follow the light curve (KS against the analytic cumulative distribution)
//! * Runs which stop after a number of triggers: exactly that many triggers, and the run time scales with the number of triggers
//! * The light curve of mimrec (-l) contains the events which mimrec extracts (-x), and its counts in the three intervals follow the light curve of the source
//! * The event IDs and times in the sim and tra files are ordered, and the tra file contains the events of the sim file with their times
class ETCosimaToMimrecLightCurve : public MEndToEndTest
{
public:
  //! Default constructor
  ETCosimaToMimrecLightCurve() : MEndToEndTest("ETCosimaToMimrecLightCurve") {}
  //! Default destructor
  virtual ~ETCosimaToMimrecLightCurve() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Return the index of the interval of the light curve which contains the time in seconds: 0: before 30 s, 1: before 50 s, 2: later
  unsigned int GetTimeBin(double Time) const;
};

#endif


////////////////////////////////////////////////////////////////////////////////
