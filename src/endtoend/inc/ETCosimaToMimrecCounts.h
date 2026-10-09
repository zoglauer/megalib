/*
 * ETCosimaToMimrecCounts.h
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


#ifndef __ETCosimaToMimrecCounts__
#define __ETCosimaToMimrecCounts__


////////////////////////////////////////////////////////////////////////////////


// MEGAlib libs:
#include "MEndToEndTest.h"


////////////////////////////////////////////////////////////////////////////////


//! End-to-end test cosima -> revan -> mimrec with the COSI-like geometry Max.geo.setup: counts
//! * The number of generated particles is flux * start area * time (independent expectation, Poisson)
//! * It scales with the time and with the flux
//! * The detection efficiency (triggers per generated particle) does not depend on the time or the flux, and is the same for four source directions which are
//!   rotated by 90 degrees around the (four-fold symmetric) detector axis
//! * The arrival times of the triggered events are uniform in the simulated time
//! * The fraction of events which revan reconstructs as Compton events does not depend on the time or the flux
//! * The number of events which mimrec selects (energy window around the line, extraction with -x) per generated particle does not depend on the time, the flux,
//!   or the rotation either, and mimrec selects almost all of the Compton events of the energy window of the own analysis (and no other events)
class ETCosimaToMimrecCounts : public MEndToEndTest
{
public:
  //! Default constructor
  ETCosimaToMimrecCounts() : MEndToEndTest("ETCosimaToMimrecCounts") {}
  //! Default destructor
  virtual ~ETCosimaToMimrecCounts() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Return the scenario of a point source (polar angle and azimuth in degree) with a flux in ph/cm2/s
  ETSimScenario Point(const MString& Name, double Theta, double Phi, double Flux, double Time) const;
  //! Return the KS distance of the times from the uniform distribution in [0, T]
  static double UniformDistance(vector<double> Times, double Duration);
};

#endif


////////////////////////////////////////////////////////////////////////////////
