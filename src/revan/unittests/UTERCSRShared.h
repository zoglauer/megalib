/*
 * UTERCSRShared.h
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


#ifndef __UTERCSRShared__
#define __UTERCSRShared__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <cmath>
#include <cstring>
#include <iostream>
#include <vector>
using namespace std;

// MEGAlib libs:
#include "MDDetector.h"
#include "MDVolumeSequence.h"
#include "MGeometryRevan.h"
#include "MGlobal.h"
#include "MRawEventIncarnations.h"
#include "MREHit.h"
#include "MRERawEvent.h"
#include "MRESE.h"
#include "MString.h"
#include "MVector.h"
#include "UTRevanShared.h"


////////////////////////////////////////////////////////////////////////////////


//! Base class of the unit tests of the Compton sequence reconstruction: Compton events in the test telescope
class UTERCSRBase : public UTRevanBase
{
public:
  //! Standard constructor with the name of the test
  UTERCSRBase(const MString& Name) : UTRevanBase(Name) {}
  //! Default destructor
  virtual ~UTERCSRBase() {}

protected:
  //! Return the cosine of the scatter angle at the central position, from the dot product of the two flight directions
  double GetCosScatterAngle(const MVector& Start, const MVector& Central, const MVector& Stop) const
  {
    MVector In = Central - Start;
    MVector Out = Stop - Central;

    return (In.X()*Out.X() + In.Y()*Out.Y() + In.Z()*Out.Z())/(sqrt(In.X()*In.X() + In.Y()*In.Y() + In.Z()*In.Z())*sqrt(Out.X()*Out.X() + Out.Y()*Out.Y() + Out.Z()*Out.Z()));
  }

  //! Return the position error of the cosine of the scatter angle at the second hit, from numerical derivatives
  //! Gaussian error propagation: the central difference quotient (step 1E-6 cm) of each of the nine coordinates times its resolution, added in quadrature
  double GetNumericalPositionError(MRESE* First, MRESE* Second, MRESE* Third) const
  {
    const double Step = 1E-6;

    vector<MVector> Positions = { First->GetPosition(), Second->GetPosition(), Third->GetPosition() };
    vector<MVector> Resolutions = { First->GetPositionResolution(), Second->GetPositionResolution(), Third->GetPositionResolution() };

    double Sum = 0.0;
    for (unsigned int p = 0; p < 3; ++p) {
      for (unsigned int c = 0; c < 3; ++c) {
        vector<MVector> Plus = Positions;
        vector<MVector> Minus = Positions;
        Plus[p][c] += Step;
        Minus[p][c] -= Step;
        double Derivative = (GetCosScatterAngle(Plus[0], Plus[1], Plus[2]) - GetCosScatterAngle(Minus[0], Minus[1], Minus[2]))/(2*Step);
        Sum += Derivative*Derivative*Resolutions[p][c]*Resolutions[p][c];
      }
    }

    return sqrt(Sum);
  }

  //! Return the deposits (keV) of a photon which scatters at all positions and is absorbed at the last one, with exact Compton kinematics:
  //! The last deposit and the first deposit are given - the first scatter angle depends on the unknown origin of the photon
  //! The others follow backwards from E_in = E_out/(1 - E_out/E0*(1 - cos phi)) with the geometric scatter angle phi
  //! An empty vector is returned if a scatter angle is not possible for these energies
  vector<double> GetComptonEnergies(const vector<MVector>& Positions, double FirstEnergy, double LastEnergy) const
  {
    vector<double> Energies(Positions.size(), 0.0);
    if (Positions.size() < 2) {
      return vector<double>();
    }

    Energies[0] = FirstEnergy;
    Energies[Positions.size() - 1] = LastEnergy;

    // The energy of the photon after the interaction at position i:
    double OutgoingEnergy = LastEnergy;
    for (unsigned int i = Positions.size() - 2; i >= 1; --i) {
      double CosPhi = GetCosScatterAngle(Positions[i - 1], Positions[i], Positions[i + 1]);
      double Denominator = 1.0 - OutgoingEnergy/c_E0*(1.0 - CosPhi);
      if (Denominator <= 0.0) {
        return vector<double>();
      }
      double IncomingEnergy = OutgoingEnergy/Denominator;
      Energies[i] = IncomingEnergy - OutgoingEnergy;
      OutgoingEnergy = IncomingEnergy;
    }

    return Energies;
  }

  //! Return the energy (keV) of a photon which deposited E1 before a Compton scatter with the deposit E2 and the scatter angle phi:
  //! the Compton equation 1 - cos phi = E0*E2/((E - E2)*E) solved for the energy E before the scatter, plus E1
  double GetEstimatedEnergy(double E1, double E2, double CosPhi) const
  {
    return E1 + 0.5*(E2 + sqrt(E2*E2 + 4*c_E0*E2/(1.0 - CosPhi)));
  }

  //! Return the error of the estimated energy by Gaussian error propagation of GetEstimatedEnergy
  double GetEstimatedEnergyError(double dE1, double E2, double dE2, double CosPhi, double dCosPhi) const
  {
    // With R = sqrt(E2^2 + 4*E0*E2/(1 - cos phi)): dE/dE1 = 1, dE/dE2 = 1/2 + (2*E2 + 4*E0/(1 - cos phi))/(4*R), dE/dcos phi = E0*E2/((1 - cos phi)^2*R)
    double Root = sqrt(E2*E2 + 4*c_E0*E2/(1.0 - CosPhi));
    double DerivativeE2 = 0.5 + 0.25*(2*E2 + 4*c_E0/(1.0 - CosPhi))/Root;
    double DerivativeCosPhi = c_E0*E2/((1.0 - CosPhi)*(1.0 - CosPhi)*Root);

    return sqrt(dE1*dE1 + DerivativeE2*DerivativeE2*dE2*dE2 + DerivativeCosPhi*DerivativeCosPhi*dCosPhi*dCosPhi);
  }

  //! Fill the estimates (keV) of the energy of the initial photon and their errors, one from the scatter at each central hit of the sequence
  void GetEnergyEstimates(vector<MRESE*>& Sequence, vector<double>& Estimates, vector<double>& Errors) const
  {
    Estimates.clear();
    Errors.clear();
    for (unsigned int i = 1; i + 1 < Sequence.size(); ++i) {
      // All deposits before the scatter
      double E1 = 0.0;
      double dE12 = 0.0;
      for (unsigned int j = 0; j < i; ++j) {
        E1 += Sequence[j]->GetEnergy();
        dE12 += Sequence[j]->GetEnergyResolution()*Sequence[j]->GetEnergyResolution();
      }
      double CosPhi = GetCosScatterAngle(Sequence[i - 1]->GetPosition(), Sequence[i]->GetPosition(), Sequence[i + 1]->GetPosition());
      double dCosPhi = GetNumericalPositionError(Sequence[i - 1], Sequence[i], Sequence[i + 1]);
      Estimates.push_back(GetEstimatedEnergy(E1, Sequence[i]->GetEnergy(), CosPhi));
      Errors.push_back(GetEstimatedEnergyError(sqrt(dE12), Sequence[i]->GetEnergy(), Sequence[i]->GetEnergyResolution(), CosPhi, dCosPhi));
    }
  }

  //! Return the average of the estimates, weighted with their inverse squared errors
  double GetWeightedAverage(const vector<double>& Estimates, const vector<double>& Errors) const
  {
    double Sum = 0.0;
    double Weights = 0.0;
    for (unsigned int i = 0; i < Estimates.size(); ++i) {
      Sum += Estimates[i]/(Errors[i]*Errors[i]);
      Weights += 1.0/(Errors[i]*Errors[i]);
    }

    return Sum/Weights;
  }

  //! Return the chi-square of the estimates relative to their weighted average - without the errors it is the sum of the squared differences
  double GetEstimatesChiSquare(const vector<double>& Estimates, const vector<double>& Errors, bool WithErrors) const
  {
    double Average = GetWeightedAverage(Estimates, Errors);
    double ChiSquare = 0.0;
    for (unsigned int i = 0; i < Estimates.size(); ++i) {
      if (WithErrors == true) {
        ChiSquare += (Estimates[i] - Average)*(Estimates[i] - Average)/(Errors[i]*Errors[i]);
      } else {
        ChiSquare += (Estimates[i] - Average)*(Estimates[i] - Average);
      }
    }

    return ChiSquare;
  }

  //! Return the chi-square of the time of flight between two hits: (measured time difference - distance/c)^2 divided by the squared errors
  //! The error of distance/c follows from the position resolutions of both hits by Gaussian error propagation
  double GetTimeOfFlightChiSquare(MRESE* First, MRESE* Second) const
  {
    MVector Difference = Second->GetPosition() - First->GetPosition();
    double Distance = sqrt(Difference.X()*Difference.X() + Difference.Y()*Difference.Y() + Difference.Z()*Difference.Z());

    double PositionError2 = 0.0;
    for (unsigned int c = 0; c < 3; ++c) {
      double Resolution2 = First->GetPositionResolution()[c]*First->GetPositionResolution()[c] + Second->GetPositionResolution()[c]*Second->GetPositionResolution()[c];
      PositionError2 += Difference[c]*Difference[c]/(Distance*Distance)*Resolution2/(c_SpeedOfLight*c_SpeedOfLight);
    }

    double TimeDifference = Second->GetTime() - First->GetTime() - Distance/c_SpeedOfLight;

    return TimeDifference*TimeDifference/(First->GetTimeResolution()*First->GetTimeResolution() + Second->GetTimeResolution()*Second->GetTimeResolution() + PositionError2);
  }

  //! Return three positions in the telescope: tracker layer 3, tracker layer 1, calorimeter
  vector<MVector> GetThreeSitePositions() const
  {
    vector<MVector> Positions;
    Positions.push_back(MVector(0.0, 0.0, 3.0));
    Positions.push_back(MVector(2.0, 0.0, 1.0));
    Positions.push_back(MVector(1.0, 1.0, -8.0));
    return Positions;
  }

  //! Return four positions in the telescope: tracker layer 3, 2, 1, and the calorimeter - both scatters are forward scatters
  vector<MVector> GetFourSitePositions() const
  {
    vector<MVector> Positions;
    Positions.push_back(MVector(0.0, 0.0, 3.0));
    Positions.push_back(MVector(1.0, 0.0, 2.0));
    Positions.push_back(MVector(1.5, 1.0, 1.0));
    Positions.push_back(MVector(2.0, 1.0, -8.0));
    return Positions;
  }

  //! Return a new event as CreateEvent, with a time resolution of 100 ps for all hits, and the times of a photon which flies along the positions with the speed of light
  MRERawEvent* CreateTimedEvent(MGeometryRevan& Geometry, const vector<MVector>& Positions, const vector<double>& Energies)
  {
    MRERawEvent* Event = CreateEvent(Geometry, Positions, Energies);
    if (Event == nullptr) {
      return nullptr;
    }

    double Time = 1E-9;
    for (int r = 0; r < Event->GetNRESEs(); ++r) {
      if (r > 0) {
        MVector Difference = Positions[r] - Positions[r - 1];
        Time += sqrt(Difference.X()*Difference.X() + Difference.Y()*Difference.Y() + Difference.Z()*Difference.Z())/c_SpeedOfLight;
      }
      Event->GetRESEAt(r)->SetTime(Time);
      Event->GetRESEAt(r)->SetTimeResolution(1E-10);
    }

    return Event;
  }

  //! Return the cumulative chi-square distribution, i.e. 1 - chi-square probability, for one, two, or three degrees of freedom from the closed forms:
  //! 1 degree of freedom: erf(sqrt(x/2)), 2: 1 - exp(-x/2), 3: erf(sqrt(x/2)) - sqrt(2x/pi)*exp(-x/2)
  double GetChiSquareCumulative(double ChiSquare, unsigned int DegreesOfFreedom) const
  {
    if (DegreesOfFreedom == 1) {
      return erf(sqrt(0.5*ChiSquare));
    } else if (DegreesOfFreedom == 2) {
      return 1.0 - exp(-0.5*ChiSquare);
    }

    return erf(sqrt(0.5*ChiSquare)) - sqrt(2*ChiSquare/c_Pi)*exp(-0.5*ChiSquare);
  }

  //! Return the IDs of the RESEs of the event in the order of its sequence: from the start point along the links
  vector<int> GetSequenceIDs(MRERawEvent* Event) const
  {
    vector<int> IDs;

    MRESE* Previous = nullptr;
    MRESE* Current = Event->GetStartPoint();
    while (Current != nullptr && IDs.size() <= 100) {
      IDs.push_back(Current->GetID());
      MRESE* Next = nullptr;
      for (int l = 0; l < Current->GetNLinks(); ++l) {
        if (Current->GetLinkAt(l) != Previous) {
          Next = Current->GetLinkAt(l);
        }
      }
      Previous = Current;
      Current = Next;
    }

    return IDs;
  }

  //! Return the IDs of the RESEs of the event in the order in which the event stores them
  vector<int> GetStoredIDs(MRERawEvent* Event) const
  {
    vector<int> IDs;
    for (int r = 0; r < Event->GetNRESEs(); ++r) {
      IDs.push_back(Event->GetRESEAt(r)->GetID());
    }

    return IDs;
  }
};


#endif


// UTERCSRShared.h: the end...
////////////////////////////////////////////////////////////////////////////////
