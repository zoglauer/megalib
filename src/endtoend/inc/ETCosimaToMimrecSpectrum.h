/*
 * ETCosimaToMimrecSpectrum.h
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


#ifndef __ETCosimaToMimrecSpectrum__
#define __ETCosimaToMimrecSpectrum__


////////////////////////////////////////////////////////////////////////////////


// MEGAlib libs:
#include "MEndToEndTest.h"


////////////////////////////////////////////////////////////////////////////////


//! End-to-end test cosima -> revan -> mimrec with the COSI-like geometry Max.geo.setup: energies and spectra
//! * Mono lines at three energies: the true energy in the sim file is the line energy, the reconstructed energies of the events which absorb the whole energy
//!   form a peak at the line energy, and no event has (much) more energy than the photon had
//! * Mimrec spectra: the spectrum of a line has its peak at the line energy with the width of the energy resolution, and the number of entries of the spectrum
//!   of the power law source is the number of Compton events in the energy range of the own analysis
//! * A power law source: the reconstructed energy of the full energy events is the true energy over the whole energy range (no energy dependent calibration bias),
//!   and the true energies of the triggered events lie inside the energy range of the source
class ETCosimaToMimrecSpectrum : public MEndToEndTest
{
public:
  //! Default constructor
  ETCosimaToMimrecSpectrum() : MEndToEndTest("ETCosimaToMimrecSpectrum") {}
  //! Default destructor
  virtual ~ETCosimaToMimrecSpectrum() {}

  //! Run all tests
  virtual bool Run();

};

#endif


////////////////////////////////////////////////////////////////////////////////
