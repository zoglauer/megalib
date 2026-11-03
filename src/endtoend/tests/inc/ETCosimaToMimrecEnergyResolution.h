/*
 * ETCosimaToMimrecEnergyResolution.h
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


#ifndef __ETCosimaToMimrecEnergyResolution__
#define __ETCosimaToMimrecEnergyResolution__


////////////////////////////////////////////////////////////////////////////////


// MEGAlib libs:
#include "MEndToEndTest.h"


////////////////////////////////////////////////////////////////////////////////


//! End-to-end test cosima -> revan -> mimrec with the COSI-like geometry Max.geo.setup: energy resolution of the detector effects
//! * Mono lines at the energies of the resolution table of the geometry (100, 200, 500, 1000, 2000 keV)
//! * Single interaction (photo) events at the line energy: the width of the peak is the tabulated energy resolution of the detector
//! * All events with the line energy: the effective area, the width (robust sigma, FWHM, 90% half width), and the fraction in the peak are compared with the reference values
//!   of the detector effects engine MEGAlib 1-4 (the references are the mean of independent runs - the Geant4 version changes the details of the simulation)
class ETCosimaToMimrecEnergyResolution : public MEndToEndTest
{
public:
  //! Default constructor
  ETCosimaToMimrecEnergyResolution();
  //! Default destructor
  virtual ~ETCosimaToMimrecEnergyResolution() {}

  //! Run all tests
  virtual bool RunTest();

};

#endif


////////////////////////////////////////////////////////////////////////////////
