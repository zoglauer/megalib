/*
 * ETCosimaToMimrecPolarization.h
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


#ifndef __ETCosimaToMimrecPolarization__
#define __ETCosimaToMimrecPolarization__


////////////////////////////////////////////////////////////////////////////////


// MEGAlib libs:
#include "MEndToEndTest.h"


////////////////////////////////////////////////////////////////////////////////


//! End-to-end test cosima -> revan -> mimrec with the COSI-like geometry Max.geo.setup: polarization
//! Linearly polarized photons Compton scatter preferentially perpendicular to their polarization vector: the distribution of the azimuthal scatter angle eta
//! around the incident direction is ~ 1 - M*cos(2*(eta - alpha)) for the polarization angle alpha, with the modulation M > 0.
//! With the on-axis source (the four-fold symmetric detector adds no modulation) the modulation vector (<cos 2 eta>, <sin 2 eta>) of the full energy events is
//! * zero (within 5 sigma) for unpolarized photons
//! * (-M, 0), (0, -M), (+M, 0) for polarization angles of 0, 45, and 90 degrees: the modulation is the same size and rotates with the polarization (the phase is right)
//! * For a source off-axis, with a polarization vector which is NOT perpendicular to the direction (cosima uses its projection onto the plane perpendicular to
//!   the direction), the modulation relative to the projected vector is clearly negative compared with the unpolarized control.
//! * Mimrec (--polarization, with an independent unpolarized simulation as background to correct the geometry): the polarization angle of the polarized sources
//!   is measured at 0, 45, and 90 degrees (within 5 sigma + 2 degrees), the modulation is significant, and an unpolarized source has no significant modulation
//! The azimuthal angle is computed from the reconstructed Compton events, i.e. the whole chain simulation + reconstruction is tested.
class ETCosimaToMimrecPolarization : public MEndToEndTest
{
public:
  //! Default constructor
  ETCosimaToMimrecPolarization() : MEndToEndTest("ETCosimaToMimrecPolarization") {}
  //! Default destructor
  virtual ~ETCosimaToMimrecPolarization() {}

  //! Run all tests
  virtual bool Run();

private:
  //! The modulation vector of the full energy events relative to the axis E1 in the plane perpendicular to the incident direction D (E2 = D x E1, or given)
  struct Modulation {
    //! The mean of cos(2 eta) of the events
    double m_Cos = 0.0;
    //! The mean of sin(2 eta) of the events
    double m_Sin = 0.0;
    //! The number of events
    unsigned int m_N = 0;
    //! Return the uncertainty of each of the two components: sqrt(1/(2N)) for a small modulation
    double Sigma() const
    {
      if (m_N == 0) {
        return 1e9;
      }
      return sqrt(0.5/m_N);
    }
  };

  //! Measure the modulation of the events of a tra file
  static Modulation Measure(const ETTraFileCoreData& Tra, const MVector& E1, const MVector& E2, const MVector& Direction);
};

#endif


////////////////////////////////////////////////////////////////////////////////
