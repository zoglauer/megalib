/*
 * UTERCSR.cxx
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


// Standard libs:
#include <cfloat>
#include <cmath>
#include <iostream>
#include <new>
#include <set>
#include <vector>
using namespace std;

// ROOT libs:
#include "TApplication.h"
#include "TError.h"
#include "TROOT.h"

// MEGAlib libs:
#include "MERCSR.h"
#include "MRETrack.h"
#include "UTERCSRShared.h"


////////////////////////////////////////////////////////////////////////////////


//! Sequencer of the tests: MERCSR needs a derived class which calculates the quality factor of a sequence
//! Here it is the number of pairs of hits which are not in ascending order of their energy, plus the energy of the first hit in MeV, plus an offset
class UTERCSRSequencer : public MERCSR
{
public:
  //! Default constructor
  UTERCSRSequencer() : MERCSR(), m_Mode(c_ModeEnergyOrder), m_Offset(0.0) {}
  //! Default destructor
  virtual ~UTERCSRSequencer() {}

  using MERCSR::ComputeAllQualityFactors;
  using MERCSR::ComputePositionError;
  using MERCSR::CreateOnlyPermutations;
  using MERCSR::FindPermutations;
  using MERCSR::GetEscapedEnergy;
  using MERCSR::LeftShift;
  using MERCSR::m_CreateOnlyPermutations;
  using MERCSR::m_Geometry;
  using MERCSR::m_GuaranteeStartD1;
  using MERCSR::m_MaxCosineLimit;
  using MERCSR::m_MaxNInteractions;
  using MERCSR::m_QualityFactorMax;
  using MERCSR::m_QualityFactorMin;
  using MERCSR::m_QualityFactors;

  //! Return the quality factor of the sequence
  virtual double ComputeQualityFactor(vector<MRESE*>& Interactions)
  {
    if (m_Mode == c_ModeAllFailed) {
      return c_CSRFailed;
    }
    if (m_Mode == c_ModeBaseClass) {
      return MERCSR::ComputeQualityFactor(Interactions);
    }
    if (m_Mode == c_ModeConstant) {
      return m_Offset;
    }

    double QualityFactor = m_Offset + 0.001*Interactions[0]->GetEnergy();
    for (unsigned int i = 0; i < Interactions.size(); ++i) {
      for (unsigned int j = i + 1; j < Interactions.size(); ++j) {
        if (Interactions[i]->GetEnergy() > Interactions[j]->GetEnergy()) {
          QualityFactor += 1.0;
        }
      }
    }

    return QualityFactor;
  }

  //! The quality factor counts the pairs which are not in ascending order of their energy
  static const int c_ModeEnergyOrder = 0;
  //! All sequences fail
  static const int c_ModeAllFailed = 1;
  //! The quality factor of the base class is used
  static const int c_ModeBaseClass = 2;
  //! All sequences have the offset as quality factor
  static const int c_ModeConstant = 3;

  //! The mode of the quality factor
  int m_Mode;
  //! The offset of the quality factor
  double m_Offset;
};


////////////////////////////////////////////////////////////////////////////////


//! Unit test of the base class of the Compton sequence reconstruction MERCSR
class UTERCSR : public UTERCSRBase
{
public:
  //! Default constructor
  UTERCSR() : UTERCSRBase("UTERCSR") {}
  //! Default destructor
  virtual ~UTERCSR() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Return a new hit without geometry at the position with the position resolution
  MREHit* CreateResolutionHit(const MVector& Position, const MVector& Resolution);
  //! Analyze a new event with the energies at the three site positions and return it - the list owns it
  MRERawEvent* AnalyzeThreeSiteEvent(UTERCSRSequencer& CSR, MRawEventIncarnations& List, double Energy0, double Energy1, double Energy2);

  //! Test the constructor
  bool TestConstruction();
  //! Test setting the parameters and the text of the options
  bool TestSetParametersAndToString();
  //! Test the left shift
  bool TestLeftShift();
  //! Test the search for all permutations
  bool TestFindPermutations();
  //! Test the position error of the scatter angle
  bool TestComputePositionError();
  //! Test the list of all quality factors
  bool TestComputeAllQualityFactors();
  //! Test the events which are no sequences: no hits, one hit, too many hits
  bool TestSpecialEvents();
  //! Test the best sequence of an event
  bool TestBestSequence();
  //! Test the limits of the quality factor
  bool TestQualityFactorLimits();
  //! Test the required start in the tracker
  bool TestGuaranteeStartD1();
  //! Test events without a good sequence
  bool TestNoGoodSequence();
  //! Test the selection of the optimum event of several incarnations
  bool TestAnalyzeIncarnations();
  //! Test the mode which only creates all permutations
  bool TestCreateOnlyPermutations();

  //! The test telescope
  MGeometryRevan m_Geometry;
};


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTERCSR::Run()
{
  bool Passed = true;

  Passed = EvaluateTrue("ScanTelescopeGeometry()", "telescope", "The test telescope can be scanned", ScanTelescopeGeometry(m_Geometry)) && Passed;

  Passed = TestConstruction() && Passed;
  Passed = TestSetParametersAndToString() && Passed;
  Passed = TestLeftShift() && Passed;
  Passed = TestFindPermutations() && Passed;
  Passed = TestComputePositionError() && Passed;
  if (m_Geometry.IsScanned() == true) {
    Passed = TestComputeAllQualityFactors() && Passed;
    Passed = TestSpecialEvents() && Passed;
    Passed = TestBestSequence() && Passed;
    Passed = TestQualityFactorLimits() && Passed;
    Passed = TestGuaranteeStartD1() && Passed;
    Passed = TestNoGoodSequence() && Passed;
    Passed = TestAnalyzeIncarnations() && Passed;
    Passed = TestCreateOnlyPermutations() && Passed;
  }

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Return a new hit without geometry at the position with the position resolution
MREHit* UTERCSR::CreateResolutionHit(const MVector& Position, const MVector& Resolution)
{
  MREHit* Hit = CreateHit(Position, 100.0, MDDetector::c_Strip2D);
  Hit->SetPositionResolution(Resolution);

  return Hit;
}


////////////////////////////////////////////////////////////////////////////////


//! Analyze a new event with the energies at the three site positions and return it - the list owns it
MRERawEvent* UTERCSR::AnalyzeThreeSiteEvent(UTERCSRSequencer& CSR, MRawEventIncarnations& List, double Energy0, double Energy1, double Energy2)
{
  vector<double> Energies = { Energy0, Energy1, Energy2 };
  MRERawEvent* Event = CreateEvent(m_Geometry, GetThreeSitePositions(), Energies);
  if (Event == nullptr) {
    return nullptr;
  }

  List.SetInitialRawEvent(Event);
  DisableDefaultStreams();
  CSR.Analyze(&List);
  EnableDefaultStreams();

  return Event;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the constructor
bool UTERCSR::TestConstruction()
{
  bool Passed = true;

  // Construct the object in a block filled with a pattern, which is no valid value of any member
  alignas(UTERCSRSequencer) unsigned char Block[sizeof(UTERCSRSequencer)];
  FillMemory(Block, sizeof(Block), 0xA5);
  UTERCSRSequencer* CSR = new (Block) UTERCSRSequencer();

  Passed = Evaluate("MERCSR()", "default", "The maximum number of interactions is 5", CSR->m_MaxNInteractions, 5) && Passed;
  // Defect REV-G-3: MERCSR has a second member m_Geometry which hides the one of MERConstruction: it is not initialized, and SetGeometry does not reach it
  Passed = ExpectedToFail(EvaluateTrue("MERCSR()", "default", "There is no geometry", CSR->m_Geometry == nullptr)) && Passed;
  Passed = EvaluateSize("MERCSR()", "default", "There are no quality factors", CSR->m_QualityFactors.size(), 0) && Passed;

  // Defect REV-G-1: The constructor does not initialize the quality factor limits, the two flags, and the cosine limit
  Passed = ExpectedToFail(EvaluateFalse("MERCSR()", "default", "The minimum quality factor is initialized", IsFillPattern(&CSR->m_QualityFactorMin, sizeof(double), 0xA5))) && Passed;
  Passed = ExpectedToFail(EvaluateFalse("MERCSR()", "default", "The maximum quality factor is initialized", IsFillPattern(&CSR->m_QualityFactorMax, sizeof(double), 0xA5))) && Passed;
  Passed = ExpectedToFail(EvaluateFalse("MERCSR()", "default", "The flag for the start in the tracker is initialized", IsFillPattern(&CSR->m_GuaranteeStartD1, sizeof(bool), 0xA5))) && Passed;
  Passed = ExpectedToFail(EvaluateFalse("MERCSR()", "default", "The flag for creating only permutations is initialized", IsFillPattern(&CSR->m_CreateOnlyPermutations, sizeof(bool), 0xA5))) && Passed;
  Passed = ExpectedToFail(EvaluateFalse("MERCSR()", "default", "The cosine limit is initialized", IsFillPattern(&CSR->m_MaxCosineLimit, sizeof(double), 0xA5))) && Passed;

  CSR->~UTERCSRSequencer();

  Passed = EvaluateNear("c_CSRFailed", "constant", "The value of a failed sequence is half of the largest double", MERCSR::c_CSRFailed, DBL_MAX/2, 0.0) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test setting the parameters and the text of the options
bool UTERCSR::TestSetParametersAndToString()
{
  bool Passed = true;

  UTERCSRSequencer CSR;
  Passed = EvaluateTrue("SetParameters()", "all parameters", "The parameters are accepted", CSR.SetParameters(&m_Geometry, 0.25, 7.5, 4, true, true)) && Passed;
  Passed = EvaluateTrue("SetParameters()", "all parameters", "The geometry is stored", CSR.m_Geometry == &m_Geometry) && Passed;
  Passed = EvaluateNear("SetParameters()", "all parameters", "The minimum quality factor is stored", CSR.m_QualityFactorMin, 0.25, 0.0) && Passed;
  Passed = EvaluateNear("SetParameters()", "all parameters", "The maximum quality factor is stored", CSR.m_QualityFactorMax, 7.5, 0.0) && Passed;
  Passed = Evaluate("SetParameters()", "all parameters", "The maximum number of interactions is stored", CSR.m_MaxNInteractions, 4) && Passed;
  Passed = EvaluateTrue("SetParameters()", "all parameters", "The start in the tracker is required", CSR.m_GuaranteeStartD1) && Passed;
  Passed = EvaluateTrue("SetParameters()", "all parameters", "Only permutations are created", CSR.m_CreateOnlyPermutations) && Passed;

  MString Text = CSR.ToString();
  Passed = EvaluateTrue("ToString()", "all parameters", "The minimum quality factor is listed", Text.Contains("# QualityFactorMin:                   0.25\n")) && Passed;
  Passed = EvaluateTrue("ToString()", "all parameters", "The maximum quality factor is listed", Text.Contains("# QualityFactorMax:                    7.5\n")) && Passed;
  Passed = EvaluateTrue("ToString()", "all parameters", "The maximum number of interactions is listed", Text.Contains("# MaxNInteractions:                    4\n")) && Passed;
  Passed = EvaluateTrue("ToString()", "core only", "The core text is the same text", CSR.ToString(true) == Text) && Passed;

  // Defect REV-G-3: MERCSR has a second member m_Geometry which hides the one of MERConstruction: it is not initialized, and SetGeometry does not reach it
  MGeometryRevan Other;
  CSR.SetGeometry(&Other);
  Passed = ExpectedToFail(EvaluateTrue("SetGeometry()", "other geometry", "The geometry which the sequence reconstruction uses is replaced", CSR.m_Geometry == &Other)) && Passed;

  // The defaults of the parameters
  Passed = EvaluateTrue("SetParameters()", "defaults", "The defaults are accepted", CSR.SetParameters(&m_Geometry)) && Passed;
  Passed = EvaluateNear("SetParameters()", "defaults", "The minimum quality factor is 0", CSR.m_QualityFactorMin, 0.0, 0.0) && Passed;
  Passed = EvaluateNear("SetParameters()", "defaults", "The maximum quality factor is 1000", CSR.m_QualityFactorMax, 1000.0, 0.0) && Passed;
  Passed = Evaluate("SetParameters()", "defaults", "The maximum number of interactions is 5", CSR.m_MaxNInteractions, 5) && Passed;
  Passed = EvaluateFalse("SetParameters()", "defaults", "The start in the tracker is not required", CSR.m_GuaranteeStartD1) && Passed;
  Passed = EvaluateFalse("SetParameters()", "defaults", "Sequences are searched", CSR.m_CreateOnlyPermutations) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the left shift
bool UTERCSR::TestLeftShift()
{
  bool Passed = true;

  UTERCSRSequencer CSR;

  vector<MREHit*> Hits;
  for (unsigned int h = 0; h < 4; ++h) {
    Hits.push_back(CreateHit(MVector(h, 0.0, 0.0), 100.0, MDDetector::c_Strip2D));
  }

  // The example of the documentation: 1,2,3,4 shifted by 3 is 2,3,1,4
  vector<MRESE*> RESEs = { Hits[0], Hits[1], Hits[2], Hits[3] };
  CSR.LeftShift(RESEs, 3);
  Passed = EvaluateTrue("LeftShift()", "level 3 of 4", "The first three elements are rotated to the left", RESEs[0] == Hits[1] && RESEs[1] == Hits[2] && RESEs[2] == Hits[0]) && Passed;
  Passed = EvaluateTrue("LeftShift()", "level 3 of 4", "The fourth element stays", RESEs[3] == Hits[3]) && Passed;

  RESEs = { Hits[0], Hits[1], Hits[2], Hits[3] };
  CSR.LeftShift(RESEs, 4);
  Passed = EvaluateTrue("LeftShift()", "level 4 of 4", "All elements are rotated to the left", RESEs[0] == Hits[1] && RESEs[1] == Hits[2] && RESEs[2] == Hits[3] && RESEs[3] == Hits[0]) && Passed;

  RESEs = { Hits[0], Hits[1], Hits[2], Hits[3] };
  CSR.LeftShift(RESEs, 2);
  Passed = EvaluateTrue("LeftShift()", "level 2 of 4", "The first two elements are swapped", RESEs[0] == Hits[1] && RESEs[1] == Hits[0] && RESEs[2] == Hits[2] && RESEs[3] == Hits[3]) && Passed;

  RESEs = { Hits[0], Hits[1], Hits[2], Hits[3] };
  CSR.LeftShift(RESEs, 1);
  Passed = EvaluateTrue("LeftShift()", "level 1 of 4", "Nothing changes", RESEs[0] == Hits[0] && RESEs[1] == Hits[1] && RESEs[2] == Hits[2] && RESEs[3] == Hits[3]) && Passed;

  // Four shifts by four restore the sequence
  for (unsigned int s = 0; s < 4; ++s) {
    CSR.LeftShift(RESEs, 4);
  }
  Passed = EvaluateTrue("LeftShift()", "four times level 4", "The sequence is restored", RESEs[0] == Hits[0] && RESEs[1] == Hits[1] && RESEs[2] == Hits[2] && RESEs[3] == Hits[3]) && Passed;

  for (unsigned int h = 0; h < Hits.size(); ++h) {
    delete Hits[h];
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the search for all permutations
bool UTERCSR::TestFindPermutations()
{
  bool Passed = true;

  UTERCSRSequencer CSR;

  vector<MREHit*> Hits;
  for (unsigned int h = 0; h < 6; ++h) {
    Hits.push_back(CreateHit(MVector(h, 0.0, 0.0), 100.0, MDDetector::c_Strip2D));
  }

  // n elements have n! different permutations
  unsigned int Factorials[7] = { 1, 1, 2, 6, 24, 120, 720 };
  for (unsigned int n = 1; n <= 6; ++n) {
    vector<MRESE*> RESEs;
    for (unsigned int h = 0; h < n; ++h) {
      RESEs.push_back(Hits[h]);
    }

    vector<vector<MRESE*>> Permutations;
    CSR.FindPermutations(RESEs, RESEs.size(), Permutations);

    set<vector<MRESE*>> Different;
    bool AllComplete = true;
    for (unsigned int p = 0; p < Permutations.size(); ++p) {
      Different.insert(Permutations[p]);
      set<MRESE*> Elements(Permutations[p].begin(), Permutations[p].end());
      if (Permutations[p].size() != n || Elements.size() != n) {
        AllComplete = false;
      }
    }

    MString Input = MString("") + n + " elements";
    Passed = EvaluateSize("FindPermutations()", Input, "There are n! permutations", Permutations.size(), Factorials[n]) && Passed;
    Passed = EvaluateSize("FindPermutations()", Input, "All permutations are different", Different.size(), Factorials[n]) && Passed;
    Passed = EvaluateTrue("FindPermutations()", Input, "Each permutation contains each element once", AllComplete) && Passed;
  }

  // No elements: there is one empty permutation
  vector<MRESE*> Empty;
  vector<vector<MRESE*>> Permutations;
  CSR.FindPermutations(Empty, 0, Permutations);
  Passed = EvaluateSize("FindPermutations()", "no elements", "There is one permutation", Permutations.size(), 1) && Passed;
  Passed = EvaluateSize("FindPermutations()", "no elements", "The permutation is empty", Permutations[0].size(), 0) && Passed;

  // A level below the size only permutes the first elements
  vector<MRESE*> Four = { Hits[0], Hits[1], Hits[2], Hits[3] };
  Permutations.clear();
  CSR.FindPermutations(Four, 3, Permutations);
  bool LastStays = true;
  for (unsigned int p = 0; p < Permutations.size(); ++p) {
    if (Permutations[p][3] != Hits[3]) {
      LastStays = false;
    }
  }
  Passed = EvaluateSize("FindPermutations()", "level 3 of 4", "There are 3! permutations", Permutations.size(), 6) && Passed;
  Passed = EvaluateTrue("FindPermutations()", "level 3 of 4", "The fourth element stays", LastStays) && Passed;

  // The list is appended to
  CSR.FindPermutations(Four, 2, Permutations);
  Passed = EvaluateSize("FindPermutations()", "second call", "The permutations are appended", Permutations.size(), 8) && Passed;

  for (unsigned int h = 0; h < Hits.size(); ++h) {
    delete Hits[h];
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the position error of the scatter angle
bool UTERCSR::TestComputePositionError()
{
  bool Passed = true;

  UTERCSRSequencer CSR;

  // A right angle: A = (1, 0, 0), B = (0, 0, 0), C = (0, 1, 0), i.e. in = B - A = (-1, 0, 0), out = C - B = (0, 1, 0), cos = 0
  // The derivatives of cos = in*out/(|in||out|) at this point: d/dA = (0, -1, 0), d/dC = (-1, 0, 0), d/dB = (1, 1, 0)
  // Error: sqrt(dAy^2 + dCx^2 + dBx^2 + dBy^2) = sqrt(0.2^2 + 0.7^2 + 0.4^2 + 0.5^2) = sqrt(0.94)
  MREHit* A = CreateResolutionHit(MVector(1.0, 0.0, 0.0), MVector(0.1, 0.2, 0.3));
  MREHit* B = CreateResolutionHit(MVector(0.0, 0.0, 0.0), MVector(0.4, 0.5, 0.6));
  MREHit* C = CreateResolutionHit(MVector(0.0, 1.0, 0.0), MVector(0.7, 0.8, 0.9));
  Passed = EvaluateNear("ComputePositionError()", "right angle", "The error is sqrt(dAy^2 + dCx^2 + dBx^2 + dBy^2)", CSR.ComputePositionError(A, B, C), sqrt(0.94), 1E-14) && Passed;
  Passed = EvaluateNear("ComputePositionError()", "right angle, reversed", "The error of the reversed sequence is sqrt(dCx^2 + dAy^2 + dBx^2 + dBy^2)", CSR.ComputePositionError(C, B, A), sqrt(0.94), 1E-14) && Passed;
  delete A;
  delete B;
  delete C;

  // General positions: compared to the numerical derivatives (central differences with a step of 1E-6 cm: relative error 1E-10)
  A = CreateResolutionHit(MVector(0.3, -1.2, 4.0), MVector(0.1, 0.2, 0.3));
  B = CreateResolutionHit(MVector(1.5, 0.4, 1.0), MVector(0.2, 0.1, 0.05));
  C = CreateResolutionHit(MVector(-2.0, 2.5, -6.0), MVector(0.3, 0.4, 0.5));
  Passed = EvaluateNear("ComputePositionError()", "general positions", "The error is the Gaussian error propagation of the cosine of the scatter angle", CSR.ComputePositionError(A, B, C), GetNumericalPositionError(A, B, C), 1E-8) && Passed;
  Passed = EvaluateNear("ComputePositionError()", "general positions, other order", "The error is the Gaussian error propagation of the cosine of the scatter angle", CSR.ComputePositionError(B, C, A), GetNumericalPositionError(B, C, A), 1E-8) && Passed;
  delete A;
  delete B;
  delete C;

  // Tracker hits of the telescope: 1 cm strips, 0.05 cm wafer
  A = CreateResolutionHit(MVector(0.0, 0.0, 3.0), MVector(1.0/sqrt(12.0), 1.0/sqrt(12.0), 0.05/sqrt(12.0)));
  B = CreateResolutionHit(MVector(2.0, 0.0, 1.0), MVector(1.0/sqrt(12.0), 1.0/sqrt(12.0), 0.05/sqrt(12.0)));
  C = CreateResolutionHit(MVector(1.0, 1.0, -8.0), MVector(10.0/sqrt(12.0), 10.0/sqrt(12.0), 8.0/sqrt(12.0)));
  Passed = EvaluateNear("ComputePositionError()", "telescope hits", "The error is the Gaussian error propagation of the cosine of the scatter angle", CSR.ComputePositionError(A, B, C), GetNumericalPositionError(A, B, C), 1E-8) && Passed;
  delete A;
  delete B;
  delete C;

  // Three hits on the z axis with a resolution along the axis only: all derivatives vanish, and the documented approximation is used
  // |cos(atan((dA + dB)/|AB|) + atan((dB + dC)/|BC|))| = cos(2*atan(0.2)) = (1 - 0.04)/(1 + 0.04) = 12/13
  A = CreateResolutionHit(MVector(0.0, 0.0, 2.0), MVector(0.0, 0.0, 0.1));
  B = CreateResolutionHit(MVector(0.0, 0.0, 1.0), MVector(0.0, 0.0, 0.1));
  C = CreateResolutionHit(MVector(0.0, 0.0, 0.0), MVector(0.0, 0.0, 0.1));
  Passed = EvaluateNear("ComputePositionError()", "straight line", "The approximation for hits on a straight line is used", CSR.ComputePositionError(A, B, C), 12.0/13.0, 1E-14) && Passed;
  delete A;
  delete B;
  delete C;

  // No position resolutions and a right angle: the cosine has no error
  A = CreateResolutionHit(MVector(1.0, 0.0, 0.0), MVector(0.0, 0.0, 0.0));
  B = CreateResolutionHit(MVector(0.0, 0.0, 0.0), MVector(0.0, 0.0, 0.0));
  C = CreateResolutionHit(MVector(0.0, 1.0, 0.0), MVector(0.0, 0.0, 0.0));
  // Defect REV-G-2: The test for a straight line is always true, since the 1 is subtracted outside of fabs, so the approximation for a straight line is used for all hits without position error
  Passed = ExpectedToFail(EvaluateNear("ComputePositionError()", "right angle without resolutions", "The error is zero", CSR.ComputePositionError(A, B, C), 0.0, 1E-14)) && Passed;
  delete A;
  delete B;
  delete C;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the list of all quality factors
bool UTERCSR::TestComputeAllQualityFactors()
{
  bool Passed = true;

  UTERCSRSequencer CSR;
  CSR.SetParameters(&m_Geometry);

  vector<double> Energies = { 100.0, 200.0, 300.0 };
  MRERawEvent* Event = CreateEvent(m_Geometry, GetThreeSitePositions(), Energies);
  Passed = EvaluateTrue("CreateEvent()", "three sites", "The event is created", Event != nullptr) && Passed;
  if (Event == nullptr) {
    return Passed;
  }

  // Number of pairs in the wrong order plus the first energy in MeV:
  // (100,200,300): 0.1  (100,300,200): 1.1  (200,100,300): 1.2  (200,300,100): 2.2  (300,100,200): 2.3  (300,200,100): 3.3
  Passed = Evaluate("ComputeAllQualityFactors()", "three sites", "All six sequences are good", CSR.ComputeAllQualityFactors(Event), 6) && Passed;
  Passed = EvaluateSize("ComputeAllQualityFactors()", "three sites", "The list has six entries", CSR.m_QualityFactors.size(), 6) && Passed;
  double Expected[6] = { 0.1, 1.1, 1.2, 2.2, 2.3, 3.3 };
  unsigned int Index = 0;
  for (auto Iter = CSR.m_QualityFactors.begin(); Iter != CSR.m_QualityFactors.end() && Index < 6; ++Iter, ++Index) {
    Passed = EvaluateNear("ComputeAllQualityFactors()", MString("entry ") + Index, "The list is sorted by increasing quality factor", Iter->first, Expected[Index], 1E-12) && Passed;
  }
  Passed = EvaluateNear("ComputeAllQualityFactors()", "best entry", "The best sequence starts with the lowest energy", CSR.m_QualityFactors.begin()->second[0]->GetEnergy(), 100.0, 0.0) && Passed;
  Passed = EvaluateNear("ComputeAllQualityFactors()", "best entry", "The best sequence ends with the highest energy", CSR.m_QualityFactors.begin()->second[2]->GetEnergy(), 300.0, 0.0) && Passed;

  // Sequences with the same quality factor are all kept
  CSR.m_Mode = UTERCSRSequencer::c_ModeConstant;
  CSR.m_Offset = 0.5;
  Passed = Evaluate("ComputeAllQualityFactors()", "identical quality factors", "All six sequences are good", CSR.ComputeAllQualityFactors(Event), 6) && Passed;
  Passed = EvaluateSize("ComputeAllQualityFactors()", "identical quality factors", "The list has six entries", CSR.m_QualityFactors.size(), 6) && Passed;

  // Failed sequences are not stored, and the list is cleared by each call
  CSR.m_Mode = UTERCSRSequencer::c_ModeAllFailed;
  Passed = Evaluate("ComputeAllQualityFactors()", "all failed", "No sequence is good", CSR.ComputeAllQualityFactors(Event), 0) && Passed;
  Passed = EvaluateSize("ComputeAllQualityFactors()", "all failed", "The list is empty", CSR.m_QualityFactors.size(), 0) && Passed;

  // The base class has no quality factor
  CSR.m_Mode = UTERCSRSequencer::c_ModeBaseClass;
  DisableDefaultStreams();
  int Good = CSR.ComputeAllQualityFactors(Event);
  EnableDefaultStreams();
  Passed = Evaluate("ComputeQualityFactor()", "base class", "The base class fails all sequences", Good, 0) && Passed;

  // The base class finds no escaped energy
  vector<MRESE*> RESEs = { Event->GetRESEAt(0), Event->GetRESEAt(1), Event->GetRESEAt(2) };
  Passed = EvaluateNear("GetEscapedEnergy()", "three sites", "The base class finds no escaped energy", CSR.GetEscapedEnergy(RESEs), 0.0, 0.0) && Passed;

  delete Event;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the events which are no sequences: no hits, one hit, too many hits
bool UTERCSR::TestSpecialEvents()
{
  bool Passed = true;

  UTERCSRSequencer CSR;
  CSR.SetParameters(&m_Geometry, 0.0, 1000.0, 3);
  MRawEventIncarnations List;

  // No hits
  MRERawEvent* Event = new MRERawEvent(&m_Geometry);
  List.SetInitialRawEvent(Event);
  Passed = EvaluateTrue("Analyze()", "no hits", "The analysis returns true", CSR.Analyze(&List)) && Passed;
  Passed = Evaluate("Analyze()", "no hits", "The event is rejected since it has no hits", Event->GetRejectionReason(), MRERawEvent::c_RejectionNoHits) && Passed;
  Passed = EvaluateFalse("Analyze()", "no hits", "The event is not good", Event->IsGoodEvent()) && Passed;
  Passed = EvaluateFalse("Analyze()", "no hits", "There is no optimum event", List.HasOptimumEvent()) && Passed;

  // One hit is a photo event
  vector<MVector> OnePosition = { MVector(0.0, 0.0, -10.0) };
  vector<double> OneEnergy = { 662.0 };
  Event = CreateEvent(m_Geometry, OnePosition, OneEnergy);
  List.SetInitialRawEvent(Event);
  CSR.Analyze(&List);
  Passed = Evaluate("Analyze()", "one hit", "The event is a photo event", Event->GetEventType(), MRERawEvent::c_PhotoEvent) && Passed;
  Passed = EvaluateTrue("Analyze()", "one hit", "The event is good", Event->IsGoodEvent()) && Passed;
  Passed = Evaluate("Analyze()", "one hit", "The event is not rejected", Event->GetRejectionReason(), MRERawEvent::c_RejectionNone) && Passed;
  Passed = EvaluateTrue("Analyze()", "one hit", "The event is the optimum event", List.GetOptimumEvent() == Event) && Passed;
  Passed = EvaluateTrue("Analyze()", "one hit", "The event is the best try", List.GetBestTryEvent() == Event) && Passed;

  // One track is rejected
  MRETrack* Track = new MRETrack();
  Track->AddRESE(CreateHit(MVector(0.0, 0.0, 3.0), 100.0, MDDetector::c_Strip2D, &m_Geometry));
  Track->AddRESE(CreateHit(MVector(0.0, 0.0, 2.0), 100.0, MDDetector::c_Strip2D, &m_Geometry));
  Event = new MRERawEvent(&m_Geometry);
  Event->AddRESE(Track);
  List.SetInitialRawEvent(Event);
  CSR.Analyze(&List);
  Passed = Evaluate("Analyze()", "one track", "The event is rejected since it is only one track", Event->GetRejectionReason(), MRERawEvent::c_RejectionOneTrackOnly) && Passed;
  Passed = EvaluateFalse("Analyze()", "one track", "The event is not good", Event->IsGoodEvent()) && Passed;
  Passed = EvaluateFalse("Analyze()", "one track", "There is no optimum event", List.HasOptimumEvent()) && Passed;

  // Three hits are allowed, four are too many
  Event = CreateEvent(m_Geometry, GetThreeSitePositions(), { 100.0, 200.0, 300.0 });
  List.SetInitialRawEvent(Event);
  CSR.Analyze(&List);
  Passed = EvaluateTrue("Analyze()", "as many hits as allowed", "The event is good", Event->IsGoodEvent()) && Passed;

  Event = CreateEvent(m_Geometry, GetFourSitePositions(), { 100.0, 200.0, 300.0, 400.0 });
  List.SetInitialRawEvent(Event);
  CSR.Analyze(&List);
  Passed = Evaluate("Analyze()", "one hit more than allowed", "The event is rejected since it has too many hits", Event->GetRejectionReason(), MRERawEvent::c_RejectionTooManyHitsCSR) && Passed;
  Passed = EvaluateFalse("Analyze()", "one hit more than allowed", "The event is not good", Event->IsGoodEvent()) && Passed;
  Passed = EvaluateFalse("Analyze()", "one hit more than allowed", "The event is not valid", Event->IsValid()) && Passed;
  Passed = EvaluateTrue("Analyze()", "one hit more than allowed", "The event has no start point", Event->GetStartPoint() == nullptr) && Passed;
  Passed = EvaluateNear("Analyze()", "one hit more than allowed", "The first quality factor is the largest double", Event->GetComptonQualityFactor1(), DBL_MAX, 0.0) && Passed;
  Passed = EvaluateNear("Analyze()", "one hit more than allowed", "The second quality factor is the largest double", Event->GetComptonQualityFactor2(), DBL_MAX, 0.0) && Passed;
  Passed = EvaluateFalse("Analyze()", "one hit more than allowed", "There is no optimum event", List.HasOptimumEvent()) && Passed;

  List.DeleteAll();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the best sequence of an event
bool UTERCSR::TestBestSequence()
{
  bool Passed = true;

  UTERCSRSequencer CSR;
  CSR.SetParameters(&m_Geometry);
  MRawEventIncarnations List;

  // The hits are already in the best order: 100, 200, 300 keV
  MRERawEvent* Event = AnalyzeThreeSiteEvent(CSR, List, 100.0, 200.0, 300.0);
  Passed = EvaluateTrue("CreateEvent()", "ascending energies", "The event is created", Event != nullptr) && Passed;
  if (Event == nullptr) {
    return Passed;
  }
  vector<int> Stored = GetStoredIDs(Event);
  vector<int> Sequence = GetSequenceIDs(Event);
  Passed = EvaluateTrue("Analyze()", "ascending energies", "The event is good", Event->IsGoodEvent()) && Passed;
  Passed = Evaluate("Analyze()", "ascending energies", "The event is a Compton event", Event->GetEventType(), MRERawEvent::c_ComptonEvent) && Passed;
  Passed = Evaluate("Analyze()", "ascending energies", "The event is not rejected", Event->GetRejectionReason(), MRERawEvent::c_RejectionNone) && Passed;
  Passed = EvaluateTrue("Analyze()", "ascending energies", "The start point is the first hit", Event->GetStartPoint() == Event->GetRESEAt(0)) && Passed;
  Passed = EvaluateTrue("Analyze()", "ascending energies", "The sequence is the stored order", Sequence == Stored) && Passed;
  Passed = Evaluate("Analyze()", "ascending energies", "The first hit has one link", Event->GetRESEAt(0)->GetNLinks(), 1) && Passed;
  Passed = Evaluate("Analyze()", "ascending energies", "The central hit has two links", Event->GetRESEAt(1)->GetNLinks(), 2) && Passed;
  Passed = Evaluate("Analyze()", "ascending energies", "The last hit has one link", Event->GetRESEAt(2)->GetNLinks(), 1) && Passed;
  // Best: (100,200,300) with 0.1, second best: (100,300,200) with one pair in the wrong order: 1.1
  Passed = EvaluateNear("Analyze()", "ascending energies", "The first quality factor is the one of the best sequence", Event->GetComptonQualityFactor1(), 0.1, 1E-12) && Passed;
  Passed = EvaluateNear("Analyze()", "ascending energies", "The second quality factor is the one of the second best sequence", Event->GetComptonQualityFactor2(), 1.1, 1E-12) && Passed;
  Passed = EvaluateNear("Analyze()", "ascending energies", "No energy is added to the event", Event->GetEnergy(), 600.0, 1E-12) && Passed;
  Passed = EvaluateTrue("Analyze()", "ascending energies", "The event is the optimum event", List.GetOptimumEvent() == Event) && Passed;
  Passed = EvaluateTrue("Analyze()", "ascending energies", "The event is the best try", List.GetBestTryEvent() == Event) && Passed;

  // The hits are not in the best order: 300, 100, 200 keV
  Event = AnalyzeThreeSiteEvent(CSR, List, 300.0, 100.0, 200.0);
  Stored = GetStoredIDs(Event);
  Sequence = GetSequenceIDs(Event);
  Passed = EvaluateTrue("Analyze()", "mixed energies", "The event is good", Event->IsGoodEvent()) && Passed;
  Passed = EvaluateTrue("Analyze()", "mixed energies", "The start point is the hit with the lowest energy", Event->GetStartPoint() == Event->GetRESEAt(1)) && Passed;
  Passed = EvaluateSize("Analyze()", "mixed energies", "The sequence has three hits", Sequence.size(), 3) && Passed;
  if (Sequence.size() == 3) {
    Passed = EvaluateTrue("Analyze()", "mixed energies", "The sequence is in ascending order of the energies", Sequence[0] == Stored[1] && Sequence[1] == Stored[2] && Sequence[2] == Stored[0]) && Passed;
  }
  Passed = EvaluateNear("Analyze()", "mixed energies", "The first quality factor is the one of the best sequence", Event->GetComptonQualityFactor1(), 0.1, 1E-12) && Passed;

  // Two hits: 50 keV first (0.05), the other order has one wrong pair and starts with 400 keV (1.4)
  Event = CreateEvent(m_Geometry, { MVector(0.0, 0.0, 3.0), MVector(1.0, 1.0, -8.0) }, { 400.0, 50.0 });
  List.SetInitialRawEvent(Event);
  CSR.Analyze(&List);
  Passed = EvaluateTrue("Analyze()", "two hits", "The event is good", Event->IsGoodEvent()) && Passed;
  Passed = EvaluateTrue("Analyze()", "two hits", "The start point is the hit with the lower energy", Event->GetStartPoint() == Event->GetRESEAt(1)) && Passed;
  Passed = EvaluateNear("Analyze()", "two hits", "The first quality factor is the one of the best sequence", Event->GetComptonQualityFactor1(), 0.05, 1E-12) && Passed;
  Passed = EvaluateNear("Analyze()", "two hits", "The second quality factor is the one of the other sequence", Event->GetComptonQualityFactor2(), 1.4, 1E-12) && Passed;

  List.DeleteAll();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the limits of the quality factor
bool UTERCSR::TestQualityFactorLimits()
{
  bool Passed = true;

  MRawEventIncarnations List;

  // The best quality factor of the event is 0.1 + 2 = 2.1
  UTERCSRSequencer CSR;
  CSR.m_Offset = 2.0;

  CSR.SetParameters(&m_Geometry, 0.0, 2.1 + 1E-9);
  MRERawEvent* Event = AnalyzeThreeSiteEvent(CSR, List, 100.0, 200.0, 300.0);
  Passed = EvaluateTrue("Analyze()", "just below the maximum", "The event is good", Event->IsGoodEvent()) && Passed;

  CSR.SetParameters(&m_Geometry, 0.0, 2.1 - 1E-9);
  Event = AnalyzeThreeSiteEvent(CSR, List, 100.0, 200.0, 300.0);
  Passed = EvaluateFalse("Analyze()", "just above the maximum", "The event is not good", Event->IsGoodEvent()) && Passed;
  Passed = Evaluate("Analyze()", "just above the maximum", "The event is rejected by the limits of the quality factor", Event->GetRejectionReason(), MRERawEvent::c_RejectionCSRThreshold) && Passed;
  Passed = Evaluate("Analyze()", "just above the maximum", "The event is no Compton event", Event->GetEventType(), MRERawEvent::c_UnknownEvent) && Passed;
  Passed = EvaluateFalse("Analyze()", "just above the maximum", "There is no optimum event", List.HasOptimumEvent()) && Passed;

  CSR.SetParameters(&m_Geometry, 2.1 - 1E-9, 1000.0);
  Event = AnalyzeThreeSiteEvent(CSR, List, 100.0, 200.0, 300.0);
  Passed = EvaluateTrue("Analyze()", "just above the minimum", "The event is good", Event->IsGoodEvent()) && Passed;

  CSR.SetParameters(&m_Geometry, 2.1 + 1E-9, 1000.0);
  Event = AnalyzeThreeSiteEvent(CSR, List, 100.0, 200.0, 300.0);
  Passed = EvaluateFalse("Analyze()", "just below the minimum", "The event is not good", Event->IsGoodEvent()) && Passed;
  Passed = Evaluate("Analyze()", "just below the minimum", "The event is rejected by the limits of the quality factor", Event->GetRejectionReason(), MRERawEvent::c_RejectionCSRThreshold) && Passed;

  // The limits themselves are allowed: all sequences have exactly 0.5
  CSR.m_Mode = UTERCSRSequencer::c_ModeConstant;
  CSR.m_Offset = 0.5;
  CSR.SetParameters(&m_Geometry, 0.5, 0.5);
  Event = AnalyzeThreeSiteEvent(CSR, List, 100.0, 200.0, 300.0);
  Passed = EvaluateTrue("Analyze()", "minimum = quality factor = maximum", "The event is good", Event->IsGoodEvent()) && Passed;
  Passed = EvaluateNear("Analyze()", "identical quality factors", "The first quality factor is the common one", Event->GetComptonQualityFactor1(), 0.5, 0.0) && Passed;
  Passed = EvaluateNear("Analyze()", "identical quality factors", "The second quality factor is the common one", Event->GetComptonQualityFactor2(), 0.5, 0.0) && Passed;

  List.DeleteAll();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the required start in the tracker
bool UTERCSR::TestGuaranteeStartD1()
{
  bool Passed = true;

  MRawEventIncarnations List;
  UTERCSRSequencer CSR;

  // The best sequence starts with the lowest energy: with 300, 200, 100 keV this is the hit in the calorimeter
  CSR.SetParameters(&m_Geometry, 0.0, 1000.0, 5, true);
  MRERawEvent* Event = AnalyzeThreeSiteEvent(CSR, List, 300.0, 200.0, 100.0);
  Passed = Evaluate("CreateEvent()", "three sites", "The third hit is in the calorimeter", Event->GetRESEAt(2)->GetDetector(), static_cast<int>(MDDetector::c_Calorimeter)) && Passed;
  Passed = EvaluateFalse("Analyze()", "start in the calorimeter, tracker required", "The event is not good", Event->IsGoodEvent()) && Passed;
  Passed = Evaluate("Analyze()", "start in the calorimeter, tracker required", "The event is rejected since it does not start in the tracker", Event->GetRejectionReason(), MRERawEvent::c_RejectionEventStartNotD1) && Passed;

  CSR.SetParameters(&m_Geometry, 0.0, 1000.0, 5, false);
  Event = AnalyzeThreeSiteEvent(CSR, List, 300.0, 200.0, 100.0);
  Passed = EvaluateTrue("Analyze()", "start in the calorimeter, tracker not required", "The event is good", Event->IsGoodEvent()) && Passed;
  Passed = EvaluateTrue("Analyze()", "start in the calorimeter, tracker not required", "The start point is the hit in the calorimeter", Event->GetStartPoint() == Event->GetRESEAt(2)) && Passed;

  CSR.SetParameters(&m_Geometry, 0.0, 1000.0, 5, true);
  Event = AnalyzeThreeSiteEvent(CSR, List, 100.0, 200.0, 300.0);
  Passed = Evaluate("CreateEvent()", "three sites", "The first hit is in the tracker", Event->GetRESEAt(0)->GetDetector(), static_cast<int>(MDDetector::c_Strip2D)) && Passed;
  Passed = EvaluateTrue("Analyze()", "start in the tracker, tracker required", "The event is good", Event->IsGoodEvent()) && Passed;

  List.DeleteAll();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test events without a good sequence
bool UTERCSR::TestNoGoodSequence()
{
  bool Passed = true;

  MRawEventIncarnations List;
  UTERCSRSequencer CSR;
  CSR.SetParameters(&m_Geometry);

  CSR.m_Mode = UTERCSRSequencer::c_ModeAllFailed;
  MRERawEvent* Event = AnalyzeThreeSiteEvent(CSR, List, 100.0, 200.0, 300.0);
  Passed = EvaluateFalse("Analyze()", "all sequences failed", "The event is not good", Event->IsGoodEvent()) && Passed;
  Passed = Evaluate("Analyze()", "all sequences failed", "The event is rejected since there is no good combination", Event->GetRejectionReason(), MRERawEvent::c_RejectionCSRNoGoodCombination) && Passed;
  Passed = EvaluateTrue("Analyze()", "all sequences failed", "The event has no start point", Event->GetStartPoint() == nullptr) && Passed;
  Passed = Evaluate("Analyze()", "all sequences failed", "The hits are not linked", Event->GetRESEAt(1)->GetNLinks(), 0) && Passed;
  Passed = EvaluateFalse("Analyze()", "all sequences failed", "There is no optimum event", List.HasOptimumEvent()) && Passed;

  // The quality factor of the base class always fails
  CSR.m_Mode = UTERCSRSequencer::c_ModeBaseClass;
  Event = AnalyzeThreeSiteEvent(CSR, List, 100.0, 200.0, 300.0);
  Passed = Evaluate("Analyze()", "quality factor of the base class", "The event is rejected since there is no good combination", Event->GetRejectionReason(), MRERawEvent::c_RejectionCSRNoGoodCombination) && Passed;

  List.DeleteAll();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the selection of the optimum event of several incarnations
bool UTERCSR::TestAnalyzeIncarnations()
{
  bool Passed = true;

  UTERCSRSequencer CSR;
  CSR.SetParameters(&m_Geometry);

  // Best quality factors: lowest energy in MeV, i.e. 0.1, 0.05, and 0.2
  MRERawEvent* First = CreateEvent(m_Geometry, GetThreeSitePositions(), { 100.0, 200.0, 300.0 });
  MRERawEvent* Second = CreateEvent(m_Geometry, GetThreeSitePositions(), { 300.0, 50.0, 200.0 });
  MRERawEvent* Third = CreateEvent(m_Geometry, GetThreeSitePositions(), { 400.0, 300.0, 200.0 });

  MRawEventIncarnations List;
  List.SetInitialRawEvent(First);
  List.AddRawEvent(Second);
  List.AddRawEvent(Third);
  Passed = EvaluateTrue("Analyze()", "three incarnations", "The analysis returns true", CSR.Analyze(&List)) && Passed;
  Passed = EvaluateTrue("Analyze()", "three incarnations", "All incarnations are good", First->IsGoodEvent() && Second->IsGoodEvent() && Third->IsGoodEvent()) && Passed;
  Passed = EvaluateTrue("Analyze()", "three incarnations", "The optimum event is the one with the lowest quality factor", List.GetOptimumEvent() == Second) && Passed;
  Passed = EvaluateTrue("Analyze()", "three incarnations", "The best try is the one with the lowest quality factor", List.GetBestTryEvent() == Second) && Passed;
  Passed = Evaluate("Analyze()", "three incarnations", "The list still has three incarnations", List.GetNRawEvents(), 3) && Passed;
  List.DeleteAll();

  // Only one of two incarnations has a quality factor within the limits: 0.1 and 0.3
  CSR.SetParameters(&m_Geometry, 0.2, 1000.0);
  First = CreateEvent(m_Geometry, GetThreeSitePositions(), { 100.0, 200.0, 300.0 });
  Second = CreateEvent(m_Geometry, GetThreeSitePositions(), { 300.0, 400.0, 500.0 });
  List.SetInitialRawEvent(First);
  List.AddRawEvent(Second);
  CSR.Analyze(&List);
  Passed = EvaluateFalse("Analyze()", "one good incarnation", "The better incarnation is below the minimum", First->IsGoodEvent()) && Passed;
  Passed = EvaluateTrue("Analyze()", "one good incarnation", "The optimum event is the good one", List.GetOptimumEvent() == Second) && Passed;
  List.DeleteAll();

  // No good incarnation: the best try of an earlier step is kept
  CSR.SetParameters(&m_Geometry, 5.0, 1000.0);
  First = CreateEvent(m_Geometry, GetThreeSitePositions(), { 100.0, 200.0, 300.0 });
  Second = CreateEvent(m_Geometry, GetThreeSitePositions(), { 300.0, 400.0, 500.0 });
  List.SetInitialRawEvent(First);
  List.AddRawEvent(Second);
  List.SetOptimumEvent(First);
  List.SetBestTryEvent(Second);
  CSR.Analyze(&List);
  Passed = EvaluateFalse("Analyze()", "no good incarnation", "There is no optimum event", List.HasOptimumEvent()) && Passed;
  Passed = EvaluateTrue("Analyze()", "no good incarnation", "The best try is kept", List.GetBestTryEvent() == Second) && Passed;
  List.DeleteAll();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the mode which only creates all permutations
bool UTERCSR::TestCreateOnlyPermutations()
{
  bool Passed = true;

  UTERCSRSequencer CSR;
  CSR.SetParameters(&m_Geometry, 0.0, 1000.0, 3, false, true);

  MRERawEvent* Event = CreateEvent(m_Geometry, GetThreeSitePositions(), { 100.0, 200.0, 300.0 });
  vector<int> Stored = GetStoredIDs(Event);
  set<int> StoredSet(Stored.begin(), Stored.end());

  MRawEventIncarnations List;
  List.SetInitialRawEvent(Event);
  List.SetOptimumEvent(Event);
  Passed = EvaluateTrue("Analyze()", "only permutations, three hits", "The analysis returns true", CSR.Analyze(&List)) && Passed;
  Passed = Evaluate("Analyze()", "only permutations, three hits", "The event is replaced by its six permutations", List.GetNRawEvents(), 6) && Passed;
  Passed = EvaluateFalse("Analyze()", "only permutations, three hits", "There is no optimum event", List.HasOptimumEvent()) && Passed;

  set<vector<int>> Sequences;
  bool AllComplete = true;
  bool AllUndecided = true;
  for (int e = 0; e < List.GetNRawEvents(); ++e) {
    vector<int> Sequence = GetSequenceIDs(List.GetRawEventAt(e));
    set<int> SequenceSet(Sequence.begin(), Sequence.end());
    if (Sequence.size() != 3 || SequenceSet != StoredSet) {
      AllComplete = false;
    }
    Sequences.insert(Sequence);
    if (List.GetRawEventAt(e)->IsGoodEvent() == true || List.GetRawEventAt(e)->GetEventType() != MRERawEvent::c_UnknownEvent) {
      AllUndecided = false;
    }
  }
  Passed = EvaluateTrue("Analyze()", "only permutations, three hits", "Each incarnation is a sequence of the three original hits", AllComplete) && Passed;
  Passed = EvaluateSize("Analyze()", "only permutations, three hits", "All six sequences are different", Sequences.size(), 6) && Passed;
  Passed = EvaluateTrue("Analyze()", "only permutations, three hits", "No incarnation is classified", AllUndecided) && Passed;
  List.DeleteAll();

  // Too many hits: the event stays and is rejected
  Event = CreateEvent(m_Geometry, GetFourSitePositions(), { 100.0, 200.0, 300.0, 400.0 });
  List.SetInitialRawEvent(Event);
  CSR.Analyze(&List);
  Passed = Evaluate("Analyze()", "only permutations, too many hits", "The event stays in the list", List.GetNRawEvents(), 1) && Passed;
  Passed = EvaluateTrue("Analyze()", "only permutations, too many hits", "The event is the original one", List.GetRawEventAt(0) == Event) && Passed;
  Passed = Evaluate("Analyze()", "only permutations, too many hits", "The event is rejected since it has too many hits", Event->GetRejectionReason(), MRERawEvent::c_RejectionTooManyHitsCSR) && Passed;

  // No hits: the event stays and is rejected
  Event = new MRERawEvent(&m_Geometry);
  List.SetInitialRawEvent(Event);
  CSR.Analyze(&List);
  Passed = Evaluate("Analyze()", "only permutations, no hits", "The event stays in the list", List.GetNRawEvents(), 1) && Passed;
  Passed = Evaluate("Analyze()", "only permutations, no hits", "The event is rejected since it has no hits", Event->GetRejectionReason(), MRERawEvent::c_RejectionNoHits) && Passed;

  // One hit: one permutation
  Event = CreateEvent(m_Geometry, { MVector(0.0, 0.0, -10.0) }, { 662.0 });
  int ID = Event->GetRESEAt(0)->GetID();
  List.SetInitialRawEvent(Event);
  CSR.Analyze(&List);
  Passed = Evaluate("Analyze()", "only permutations, one hit", "There is one incarnation", List.GetNRawEvents(), 1) && Passed;
  Passed = EvaluateTrue("Analyze()", "only permutations, one hit", "The start point is the hit", List.GetRawEventAt(0)->GetStartPoint() != nullptr && List.GetRawEventAt(0)->GetStartPoint()->GetID() == ID) && Passed;
  List.DeleteAll();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  // Batch mode first, otherwise ROOT connects to the display: a test must not depend on a display being available
  gROOT->SetBatch(true);
  TApplication ROOT("ROOT", 0, 0);
  // Hide the information messages of ROOT
  gErrorIgnoreLevel = kWarning;

  UTERCSR Test;
  return Test.Run() == true ? 0 : 1;
}


// UTERCSR.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
