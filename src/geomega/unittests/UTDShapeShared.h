/*
 * UTDShapeShared.h
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


#ifndef __UTDShapeShared__
#define __UTDShapeShared__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <cmath>
#include <functional>
#include <sstream>
#include <vector>

// POSIX libs: fork, waitpid, and dup2 for the crash checks have no C++ equivalent
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>

// ROOT libs:
#include <TEnv.h>
#include <TError.h>
#include <TRandom.h>
#include <TRandom3.h>
using namespace std;

// MEGAlib libs:
#include "MDDebugInfo.h"
#include "MDShape.h"
#include "MStreams.h"
#include "MString.h"
#include "MTokenizer.h"
#include "MVector.h"
#include "UTGeomegaShared.h"


////////////////////////////////////////////////////////////////////////////////


//! Base class of the shape unit tests with analytic cross checks of a shape
class UTDShapeBase : public UTGeomegaBase
{
public:
  //! Standard constructor with the name of the test
  UTDShapeBase(const MString& Name) : UTGeomegaBase(Name), m_TestRandom(20261104) {}
  //! Default destructor
  virtual ~UTDShapeBase() {}

protected:
  //! Analytic truth if a point is inside the shape
  using MTruth = function<bool(double, double, double)>;

  //! Return a uniform random point in the box between Min and Max
  MVector GetBoxPoint(const MVector& Min, const MVector& Max)
  {
    return MVector(Min.X() + m_TestRandom.Rndm()*(Max.X() - Min.X()),
                   Min.Y() + m_TestRandom.Rndm()*(Max.Y() - Min.Y()),
                   Min.Z() + m_TestRandom.Rndm()*(Max.Z() - Min.Z()));
  }

  //! Return an isotropic random unit vector
  MVector GetRandomDirection()
  {
    double CosTheta = 2*m_TestRandom.Rndm() - 1;
    double SinTheta = sqrt(1 - CosTheta*CosTheta);
    double Phi = 2*c_Pi*m_TestRandom.Rndm();
    return MVector(SinTheta*cos(Phi), SinTheta*sin(Phi), CosTheta);
  }

  //! Return a uniform random point of the analytic truth by rejection in the box
  MVector GetTruthPoint(const MTruth& Truth, const MVector& Min, const MVector& Max)
  {
    MVector P;
    do {
      P = GetBoxPoint(Min, Max);
    } while (Truth(P.X(), P.Y(), P.Z()) == false);
    return P;
  }

  //! Return the analytic truth of a TRAP or of a GTRA without twist
  MTruth GetTrapTruth(double Dz, double Theta, double Phi, double H1, double Bl1, double Tl1, double Alpha1, double H2, double Bl2, double Tl2, double Alpha2)
  {
    double TiltX = tan(Theta*c_Rad)*cos(Phi*c_Rad);
    double TiltY = tan(Theta*c_Rad)*sin(Phi*c_Rad);
    return GetInterpolatedQuadTruth(Dz, GetTrapFace(-Dz*TiltX, -Dz*TiltY, H1, Bl1, Tl1, Alpha1), GetTrapFace(+Dz*TiltX, +Dz*TiltY, H2, Bl2, Tl2, Alpha2));
  }

  //! Check that a validated shape can be configured again with Set and with Parse (if the line is not empty), and that its ROOT shape follows: the distance from the origin along the direction is the analytic truth
  bool CheckReconfiguration(const MString& Name, MDShape& Shape, const MVector& Direction, const function<bool()>& SetFirst, double First, const function<bool()>& SetSecond, double Second, const MString& ParseLine = "", double Parsed = 0.0, const function<bool()>& SetInvalid = nullptr)
  {
    bool Passed = true;
    const MVector Origin(0.0, 0.0, 0.0);
    const double Tolerance = 1e-9;

    Passed = EvaluateTrue("Set()", Name + ", first", "The first configuration is accepted", SetFirst()) && Passed;
    Passed = EvaluateTrue("Validate()", Name + ", first", "The first configuration is valid", Shape.Validate()) && Passed;
    Passed = EvaluateNear("DistanceInsideOut()", Name + ", first", "The ROOT shape has the first configuration", Shape.DistanceInsideOut(Origin, Direction), First, Tolerance) && Passed;

    Passed = EvaluateTrue("Set()", Name + ", second", "A validated shape can be configured again", SetSecond()) && Passed;
    Passed = EvaluateTrue("Validate()", Name + ", second", "The second configuration is valid", Shape.Validate()) && Passed;
    Passed = EvaluateNear("DistanceInsideOut()", Name + ", second", "The ROOT shape follows the second configuration", Shape.DistanceInsideOut(Origin, Direction), Second, Tolerance) && Passed;

    if (ParseLine != "") {
      Passed = EvaluateTrue("Parse()", Name + ", parse", "A validated shape can be parsed again", ParseQuiet(Shape, ParseLine)) && Passed;
      Passed = EvaluateTrue("Validate()", Name + ", parse", "The parsed configuration is valid", Shape.Validate()) && Passed;
      Passed = EvaluateNear("DistanceInsideOut()", Name + ", parse", "The ROOT shape follows the parsed configuration", Shape.DistanceInsideOut(Origin, Direction), Parsed, Tolerance) && Passed;
    }

    if (SetInvalid) {
      DisableDefaultStreams();
      const bool Accepted = SetInvalid();
      EnableDefaultStreams();
      Passed = EvaluateFalse("Set()", Name + ", invalid", "An invalid configuration of a valid shape is rejected", Accepted) && Passed;
      Passed = EvaluateTrue("Validate()", Name + ", invalid", "The shape is still valid after the rejected configuration", Shape.Validate()) && Passed;
      Passed = EvaluateNear("DistanceInsideOut()", Name + ", invalid", "The shape keeps its last valid configuration", Shape.DistanceInsideOut(Origin, Direction), Parsed, Tolerance) && Passed;
    }

    return Passed;
  }

  //! Parse a line silently and return the result
  bool ParseQuiet(MDShape& Shape, const MString& Line)
  {
    MTokenizer Tokenizer(Line);
    MDDebugInfo Info(Line, "UnitTest", 1);
    DisableDefaultStreams();
    bool Result = Shape.Parse(Tokenizer, Info);
    EnableDefaultStreams();
    return Result;
  }

  //! Call the function silently and return its result
  bool Quiet(const function<bool()>& Call)
  {
    Int_t ErrorLevel = gErrorIgnoreLevel;
    gErrorIgnoreLevel = kFatal;
    DisableDefaultStreams();
    bool Result = Call();
    EnableDefaultStreams();
    gErrorIgnoreLevel = ErrorLevel;
    return Result;
  }

  //! Return the number of uniform random points of a box at which IsInside differs from the analytic truth
  unsigned int GetNumberOfInsideDifferences(MDShape& Shape, const MTruth& Truth, const MVector& Min, const MVector& Max, unsigned int Points = 20000)
  {
    unsigned int Different = 0;
    for (unsigned int i = 0; i < Points; ++i) {
      MVector P = GetBoxPoint(Min, Max);
      if (Shape.IsInside(P) != Truth(P.X(), P.Y(), P.Z())) {
        ++Different;
      }
    }
    return Different;
  }

  //! Return the truth of a solid with quadrilateral cross sections between the z values -Dz and +Dz
  //! The 4 corners of the two faces (in the same order around the face) are interpolated linearly in z
  MTruth GetInterpolatedQuadTruth(double Dz, const vector<MVector>& Bottom, const vector<MVector>& Top)
  {
    return [=](double x, double y, double z) {
      if (fabs(z) > Dz) {
        return false;
      }
      double T = (z + Dz)/(2*Dz);
      int Positive = 0;
      int Negative = 0;
      for (unsigned int c = 0; c < 4; ++c) {
        unsigned int d = (c + 1)%4;
        double Ax = Bottom[c].X() + T*(Top[c].X() - Bottom[c].X());
        double Ay = Bottom[c].Y() + T*(Top[c].Y() - Bottom[c].Y());
        double Bx = Bottom[d].X() + T*(Top[d].X() - Bottom[d].X());
        double By = Bottom[d].Y() + T*(Top[d].Y() - Bottom[d].Y());
        double Cross = (Bx - Ax)*(y - Ay) - (By - Ay)*(x - Ax);
        if (Cross > 0) {
          ++Positive;
        }
        if (Cross < 0) {
          ++Negative;
        }
      }
      if (Positive > 0 && Negative > 0) {
        return false;
      }
      return true;
    };
  }

  //! Return the relative one sigma error of the volume of a boolean shape - 10000 hits per run in the surrounding box, the shapes differ in their number of runs
  double GetVolumeError(double Volume, double BoxVolume, unsigned int Runs)
  {
    // The number of trials for a fixed number of hits has a negative binomial distribution
    double P = Volume/BoxVolume;
    if (P >= 1) {
      P = 1;
    }
    return sqrt((1 - P)/10000/Runs);
  }

  //! Return the analytic truth of a cone: radii are linear from bottom at -H to top at +H, the angles are in degrees
  MTruth GetConeTruth(double H, double RminB, double RmaxB, double RminT, double RmaxT, double Phi1 = 0, double Phi2 = 360)
  {
    return [=](double x, double y, double z) {
      if (fabs(z) > H) {
        return false;
      }
      double T = (z + H)/(2*H);
      double R = sqrt(x*x + y*y);
      if (R < RminB + T*(RminT - RminB) || R > RmaxB + T*(RmaxT - RmaxB)) {
        return false;
      }
      double Phi = atan2(y, x)*c_Deg;
      if (Phi < Phi1) {
        Phi += 360;
      }
      if (Phi < Phi1) {
        Phi += 360;
      }
      if (Phi > Phi2) {
        return false;
      }
      return true;
    };
  }

  //! Return the corners of a face of a TRAP, the center is given, the angle Alpha is in degrees
  vector<MVector> GetTrapFace(double CenterX, double CenterY, double H, double Bl, double Tl, double Alpha)
  {
    double Shift = H*tan(Alpha*c_Rad);
    return { MVector(CenterX - Shift - Bl, CenterY - H, 0), MVector(CenterX + Shift - Tl, CenterY + H, 0),
             MVector(CenterX + Shift + Tl, CenterY + H, 0), MVector(CenterX - Shift + Bl, CenterY - H, 0) };
  }

  //! Check IsInside against the analytic truth at uniform random points of a box
  bool CheckInsideAgainstTruth(const MString& Name, MDShape& Shape, const MTruth& Truth, const MVector& Min, const MVector& Max, unsigned int Points = 20000)
  {
    unsigned int Different = 0;
    unsigned int NumberInside = 0;
    for (unsigned int i = 0; i < Points; ++i) {
      MVector P = GetBoxPoint(Min, Max);
      bool T = Truth(P.X(), P.Y(), P.Z());
      if (T == true) {
        ++NumberInside;
      }
      if (Shape.IsInside(P) != T) {
        ++Different;
      }
    }

    bool Passed = true;
    Passed = EvaluateTrue("IsInside()", Name, "The sample contains points inside and outside of the shape", NumberInside > 0 && NumberInside < Points) && Passed;
    Passed = Evaluate("IsInside()", Name, "All random points agree with the analytic truth", Different, 0U) && Passed;
    return Passed;
  }

  //! Check if the hit-or-miss volume of the analytic truth agrees with the expected volume
  //! Binomial error of N points - 5 sigma
  bool CheckTruthVolume(const MString& Name, const MTruth& Truth, const MVector& Min, const MVector& Max, double Expected, unsigned int Points = 200000)
  {
    unsigned int Hits = 0;
    for (unsigned int i = 0; i < Points; ++i) {
      MVector P = GetBoxPoint(Min, Max);
      if (Truth(P.X(), P.Y(), P.Z()) == true) {
        ++Hits;
      }
    }
    double BoxVolume = (Max.X() - Min.X())*(Max.Y() - Min.Y())*(Max.Z() - Min.Z());
    double P = static_cast<double>(Hits)/Points;
    double Sigma = BoxVolume*sqrt(P*(1 - P)/Points);
    return EvaluateNear("GetVolume()", Name, "The hit-or-miss volume of the analytic truth agrees with the expected volume", P*BoxVolume, Expected, 5*Sigma);
  }

  //! Check GetRandomPositionInside: all inside and same first and second moments as a rejection sample of the truth
  //! Errors of mean and variance of two samples of size N - 5 sigma
  bool CheckRandomPositions(const MString& Name, MDShape& Shape, const MTruth& Truth, const MVector& Min, const MVector& Max, unsigned int Points = 20000)
  {
    gRandom->SetSeed(4711);

    vector<MVector> Sample(Points);
    vector<MVector> Reference(Points);
    unsigned int Outside = 0;
    for (unsigned int i = 0; i < Points; ++i) {
      Sample[i] = Shape.GetRandomPositionInside();
      Reference[i] = GetTruthPoint(Truth, Min, Max);
      // Attention: The positions of the optimized versions can be on the surface within rounding
      bool In = false;
      for (double dX : {-1e-9, 1e-9}) {
        for (double dY : {-1e-9, 1e-9}) {
          for (double dZ : {-1e-9, 1e-9}) {
            if (Truth(Sample[i].X() + dX, Sample[i].Y() + dY, Sample[i].Z() + dZ) == true) {
              In = true;
            }
          }
        }
      }
      if (In == false) {
        ++Outside;
      }
    }

    bool Passed = true;
    Passed = Evaluate("GetRandomPositionInside()", Name, "All random positions are inside the shape", Outside, 0U) && Passed;

    for (unsigned int Axis = 0; Axis < 3; ++Axis) {
      double MeanS = 0;
      double MeanR = 0;
      for (unsigned int i = 0; i < Points; ++i) {
        MeanS += Sample[i][Axis];
        MeanR += Reference[i][Axis];
      }
      MeanS /= Points;
      MeanR /= Points;

      double VarS = 0;
      double VarR = 0;
      double FourthR = 0;
      for (unsigned int i = 0; i < Points; ++i) {
        VarS += (Sample[i][Axis] - MeanS)*(Sample[i][Axis] - MeanS);
        double D = Reference[i][Axis] - MeanR;
        VarR += D*D;
        FourthR += D*D*D*D;
      }
      VarS /= Points;
      VarR /= Points;
      FourthR /= Points;

      double SigmaMean = sqrt(2*VarR/Points);
      double SigmaVar = sqrt(2*fabs(FourthR - VarR*VarR)/Points);
      MString AxisName = Name + " axis ";
      AxisName += Axis;
      Passed = EvaluateNear("GetRandomPositionInside()", AxisName, "The mean agrees with the uniform distribution in the shape", MeanS, MeanR, 5*SigmaMean) && Passed;
      Passed = EvaluateNear("GetRandomPositionInside()", AxisName, "The variance agrees with the uniform distribution in the shape", VarS, VarR, 5*SigmaVar) && Passed;
    }

    // The coordinates also have to be distributed together like the uniform distribution: means and variances per axis do not see, for example, points on a diagonal
    double MeanS[3] = { 0, 0, 0 };
    double MeanR[3] = { 0, 0, 0 };
    for (unsigned int i = 0; i < Points; ++i) {
      for (unsigned int Axis = 0; Axis < 3; ++Axis) {
        MeanS[Axis] += Sample[i][Axis]/Points;
        MeanR[Axis] += Reference[i][Axis]/Points;
      }
    }
    for (unsigned int A = 0; A < 3; ++A) {
      const unsigned int B = (A + 1)%3;
      double CovS = 0;
      double CovR = 0;
      double SquareR = 0;
      for (unsigned int i = 0; i < Points; ++i) {
        CovS += (Sample[i][A] - MeanS[A])*(Sample[i][B] - MeanS[B])/Points;
        const double ProductR = (Reference[i][A] - MeanR[A])*(Reference[i][B] - MeanR[B]);
        CovR += ProductR/Points;
        SquareR += ProductR*ProductR/Points;
      }
      MString PairName = Name + " axes ";
      PairName += A;
      PairName += B;
      const double SigmaCov = sqrt(2*fabs(SquareR - CovR*CovR)/Points);
      Passed = EvaluateNear("GetRandomPositionInside()", PairName, "The covariance of two axes agrees with the uniform distribution in the shape", CovS, CovR, 5*SigmaCov) && Passed;
    }

    // Each octant around the center holds as many points as in the uniform distribution, also for a dependence which has no covariance. Error of the difference of two counts: sqrt(sum)
    unsigned int CountS[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
    unsigned int CountR[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
    for (unsigned int i = 0; i < Points; ++i) {
      unsigned int OctantS = 0;
      unsigned int OctantR = 0;
      for (unsigned int Axis = 0; Axis < 3; ++Axis) {
        OctantS += (Sample[i][Axis] > MeanR[Axis]) ? (1U << Axis) : 0U;
        OctantR += (Reference[i][Axis] > MeanR[Axis]) ? (1U << Axis) : 0U;
      }
      ++CountS[OctantS];
      ++CountR[OctantR];
    }
    for (unsigned int Octant = 0; Octant < 8; ++Octant) {
      MString OctantName = Name + " octant ";
      OctantName += Octant;
      Passed = EvaluateNear("GetRandomPositionInside()", OctantName, "The number of points in the octant agrees with the uniform distribution in the shape", static_cast<double>(CountS[Octant]), static_cast<double>(CountR[Octant]), 5*sqrt(static_cast<double>(CountS[Octant] + CountR[Octant])) + 1e-9) && Passed;
    }

    return Passed;
  }

  //! Check the distance functions along random rays against the analytic truth
  bool CheckDistances(const MString& Name, MDShape& Shape, const MTruth& Truth, const MVector& Min, const MVector& Max, unsigned int Rays = 2000)
  {
    // The step across the surface, large compared to the tolerance of ROOT
    const double Delta = 1e-6;
    const unsigned int Steps = 50;

    // Rays from the inside to the first surface:
    unsigned int WrongInsideOut = 0;
    for (unsigned int r = 0; r < Rays; ++r) {
      MVector P = GetTruthPoint(Truth, Min, Max);
      MVector D = GetRandomDirection();
      double L = Shape.DistanceInsideOut(P, D);
      bool Good = true;
      MVector Before = P + (L - Delta)*D;
      MVector After = P + (L + Delta)*D;
      if (Truth(Before.X(), Before.Y(), Before.Z()) == false || Truth(After.X(), After.Y(), After.Z()) == true) {
        Good = false;
      }
      for (unsigned int s = 0; s < Steps; ++s) {
        MVector Q = P + ((s + 0.5)/Steps*(L - Delta))*D;
        if (Truth(Q.X(), Q.Y(), Q.Z()) == false) {
          Good = false;
        }
      }
      if (Good == false) {
        ++WrongInsideOut;
      }
    }

    // Rays from the outside to an inside point, thus always a hit:
    unsigned int WrongOutsideIn = 0;
    for (unsigned int r = 0; r < Rays; ++r) {
      MVector P;
      do {
        P = GetBoxPoint(Min, Max);
      } while (Truth(P.X(), P.Y(), P.Z()) == true);
      MVector Target = GetTruthPoint(Truth, Min, Max);
      MVector D = (Target - P).Unit();
      double L = Shape.DistanceOutsideIn(P, D);
      bool Good = true;
      if (L > (Target - P).Mag() + Delta) {
        Good = false;
      } else {
        MVector After = P + (L + Delta)*D;
        if (Truth(After.X(), After.Y(), After.Z()) == false) {
          Good = false;
        }
        for (unsigned int s = 0; s < Steps; ++s) {
          MVector Q = P + ((s + 0.5)/Steps*(L - Delta))*D;
          if (Truth(Q.X(), Q.Y(), Q.Z()) == true) {
            Good = false;
          }
        }
      }
      if (Good == false) {
        ++WrongOutsideIn;
      }
    }

    bool Passed = true;
    Passed = Evaluate("DistanceInsideOut()", Name, "The first exit of all random rays agrees with the analytic truth", WrongInsideOut, 0U) && Passed;
    Passed = Evaluate("DistanceOutsideIn()", Name, "The first entry of all random rays agrees with the analytic truth", WrongOutsideIn, 0U) && Passed;
    return Passed;
  }

  //! Check IsInside, the random positions, and the distance functions against the analytic truth
  bool CheckAgainstTruth(const MString& Name, MDShape& Shape, const MTruth& Truth, const MVector& Min, const MVector& Max)
  {
    bool Passed = true;
    Passed = CheckInsideAgainstTruth(Name, Shape, Truth, Min, Max) && Passed;
    Passed = CheckRandomPositions(Name, Shape, Truth, Min, Max) && Passed;
    Passed = CheckDistances(Name, Shape, Truth, Min, Max) && Passed;
    return Passed;
  }

  //! Check the tolerance handling of IsInside at a point on the surface and its outward direction
  bool CheckToleranceAtSurface(const MString& Name, MDShape& Shape, const MVector& Surface, const MVector& Outward)
  {
    const double Tolerance = 0.01;
    const double Offset = 0.005;
    MVector In = Surface - Offset*Outward;
    MVector Out = Surface + Offset*Outward;

    bool Passed = true;
    Passed = EvaluateTrue("IsInside()", Name, "A point inside the surface is inside", Shape.IsInside(In)) && Passed;
    Passed = EvaluateFalse("IsInside()", Name, "A point outside the surface is outside", Shape.IsInside(Out)) && Passed;
    Passed = EvaluateTrue("IsInside()", Name, "A point just outside the surface is inside if the tolerance prefers the inside", Shape.IsInside(Out, Tolerance, false)) && Passed;
    Passed = EvaluateFalse("IsInside()", Name, "A point just inside the surface is outside if the tolerance prefers the outside", Shape.IsInside(In, Tolerance, true)) && Passed;
    Passed = EvaluateTrue("IsInside()", Name, "A point just inside the surface stays inside if the tolerance prefers the inside", Shape.IsInside(In, Tolerance, false)) && Passed;
    Passed = EvaluateFalse("IsInside()", Name, "A point just outside the surface stays outside if the tolerance prefers the outside", Shape.IsInside(Out, Tolerance, true)) && Passed;
    return Passed;
  }

  //! Check the generic scaling state machine, Shape must be valid and not be scaled yet
  //! Afterwards the shape is scaled with Factor along Axes - VolumeTolerance is relative
  bool CheckScaleState(const MString& Name, MDShape& Shape, double Factor, const MString& Axes, double VolumeTolerance = 1e-9)
  {
    bool Passed = true;
    Passed = EvaluateFalse("IsScaled()", Name, "A new shape is not scaled", Shape.IsScaled()) && Passed;
    Passed = EvaluateNear("GetScaler()", Name, "The scaler of a new shape is one", Shape.GetScaler(), 1.0, 1e-15) && Passed;

    Passed = EvaluateFalse("Scale()", Name, "An axis other than X, Y, Z is rejected", Quiet([&]() { return Shape.Scale(2.0, "XQ"); })) && Passed;
    Passed = EvaluateFalse("IsScaled()", Name, "A rejected axis leaves the shape unscaled", Shape.IsScaled()) && Passed;

    Passed = EvaluateTrue("Scale()", Name, "A scaling by one is accepted", Quiet([&]() { return Shape.Scale(1.0, Axes); })) && Passed;
    Passed = EvaluateFalse("IsScaled()", Name, "A scaling by one does not mark the shape as scaled", Shape.IsScaled()) && Passed;

    Passed = EvaluateTrue("Scale()", Name, "A valid scaling is accepted", Quiet([&]() { return Shape.Scale(Factor, Axes); })) && Passed;
    Passed = EvaluateTrue("IsScaled()", Name, "The shape is marked as scaled", Shape.IsScaled()) && Passed;
    Passed = EvaluateNear("GetScaler()", Name, "The scaler is stored", Shape.GetScaler(), Factor, 1e-15) && Passed;
    Passed = EvaluateTrue("GetScalingAxis()", Name, "The axes are stored", Shape.GetScalingAxis() == Axes) && Passed;

    double Volume = Shape.GetVolume();
    Passed = EvaluateTrue("Scale()", Name, "Scaling again with the same values is accepted", Quiet([&]() { return Shape.Scale(Factor, Axes); })) && Passed;
    Passed = EvaluateNear("Scale()", Name, "Scaling again with the same values does not scale twice", Shape.GetVolume(), Volume, VolumeTolerance*Volume) && Passed;
    Passed = EvaluateFalse("Scale()", Name, "A second different scaling is rejected", Quiet([&]() { return Shape.Scale(Factor + 1, Axes); })) && Passed;
    Passed = EvaluateNear("GetScaler()", Name, "A rejected second scaling keeps the scaler", Shape.GetScaler(), Factor, 1e-15) && Passed;

    return Passed;
  }

  //! Random numbers of the test - independent of gRandom used by the code under test
  TRandom3 m_TestRandom;
};


#endif


// UTDShapeShared.h: the end...
////////////////////////////////////////////////////////////////////////////////
