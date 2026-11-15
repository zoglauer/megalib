/*
 * UTDDetector.cxx
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
#include <cstring>
#include <new>
using namespace std;

// MEGAlib libs:
#include "UTDDetectorShared.h"


////////////////////////////////////////////////////////////////////////////////


//! Minimal concrete detector exposing the protected parts of the base class
class UTDDetectorStub : public MDDetector
{
public:
  //! Default constructor
  UTDDetectorStub(MString Name = "Stub") : MDDetector(Name) { m_Type = c_ACS; }
  //! Copy constructor
  UTDDetectorStub(const UTDDetectorStub& S) : MDDetector(S) {}
  //! Clone the detector
  virtual MDDetector* Clone() { return new UTDDetectorStub(*this); }
  //! Return no grid points
  virtual vector<MDGridPoint> Grid(const MVector&, const double&, const double&, const MDVolume*) const { return vector<MDGridPoint>(); }
  //! Return the first grid point
  virtual MDGridPoint GetGridPoint(const MVector&) const { return MDGridPoint(0, 0, 0, MDGridPoint::c_Voxel); }
  //! Return the origin
  virtual MVector GetPositionInDetectorVolume(const unsigned int, const unsigned int, const unsigned int, const MVector, const unsigned int, const MDVolume*) const { return MVector(0.0, 0.0, 0.0); }
  //! Do nothing
  virtual void Noise(MVector&, double&, double&, MString&, MDVolume*) const {}

  using MDDetector::ApplyEnergyCalibration;
  using MDDetector::ApplyEnergyResolution;
  using MDDetector::ApplyNoiseThreshold;
  using MDDetector::ApplyOverflow;
  using MDDetector::ApplyTimeResolution;
  using MDDetector::GetGeomegaCommon;
  using MDDetector::m_EnergyResolutionType;
  using MDDetector::m_ID;
  using MDDetector::m_IDCounter;
  using MDDetector::m_NoiseActive;
  using MDDetector::m_PulseShapeSet;
  using MDDetector::m_SVs;
  using MDDetector::m_SensID;
  using MDDetector::m_StructuralSize;
  using MDDetector::m_TimeResolutionType;
};


////////////////////////////////////////////////////////////////////////////////


//! Unit test of the detector base class MDDetector
class UTDDetector : public UTDDetectorBase
{
public:
  //! Default constructor
  UTDDetector() : UTDDetectorBase("UTDDetector") {}
  //! Default destructor
  virtual ~UTDDetector() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Test the detector type constants, names, and their conversions
  bool TestTypes();
  //! Test that the constructors initialize all members
  bool TestConstruction();
  //! Test the simple setters and getters
  bool TestSettersGetters();
  //! Test the input checks and the table look-ups of the energy resolution
  bool TestEnergyResolutionTable();
  //! Test applying the energy resolution
  bool TestApplyEnergyResolution();
  //! Test applying the time resolution
  bool TestApplyTimeResolution();
  //! Test the thresholds and the overflow on the base class
  bool TestThresholdsAndOverflow();
  //! Test the pulse shape
  bool TestPulseShape();
  //! Test the energy calibration and the energy loss map
  bool TestCalibrationAndEnergyLoss();
  //! Test the handling of the sensitive volumes and the detector volume
  bool TestVolumes();
  //! Test all validation branches of the base class
  bool TestValidate();
  //! Test the copy constructor
  bool TestCopy();
  //! Test the named detector interface without a geometry
  bool TestNamedDetectorInterface();
  //! Test copying the data to named detectors
  bool TestCopyDataToNamedDetectors();
  //! Test the keyword parsing of the settings common to all detectors
  bool TestKeywords();
  //! Test the interplay of several detectors and named detectors in a geometry
  bool TestDetectorsInGeometry();
  //! Test the generation of the geomega text
  bool TestGeomegaText();
  //! Test the remaining small public functions
  bool TestMiscellaneous();
};


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTDDetector::Run()
{
  bool Passed = true;

  Passed = TestTypes() && Passed;
  Passed = TestConstruction() && Passed;
  Passed = TestSettersGetters() && Passed;
  Passed = TestEnergyResolutionTable() && Passed;
  Passed = TestApplyEnergyResolution() && Passed;
  Passed = TestApplyTimeResolution() && Passed;
  Passed = TestThresholdsAndOverflow() && Passed;
  Passed = TestPulseShape() && Passed;
  Passed = TestCalibrationAndEnergyLoss() && Passed;
  Passed = TestVolumes() && Passed;
  Passed = TestValidate() && Passed;
  Passed = TestCopy() && Passed;
  Passed = TestNamedDetectorInterface() && Passed;
  Passed = TestCopyDataToNamedDetectors() && Passed;
  Passed = TestKeywords() && Passed;
  Passed = TestDetectorsInGeometry() && Passed;
  Passed = TestGeomegaText() && Passed;
  Passed = TestMiscellaneous() && Passed;

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the detector type constants, names, and their conversions
bool UTDDetector::TestTypes()
{
  bool Passed = true;

  // These numbers are part of the sim file format
  Passed = Evaluate("c_NoDetectorType", "constant", "The number is part of the sim file format", MDDetector::c_NoDetectorType, 0) && Passed;
  Passed = Evaluate("c_Strip2D", "constant", "The number is part of the sim file format", MDDetector::c_Strip2D, 1) && Passed;
  Passed = Evaluate("c_Calorimeter", "constant", "The number is part of the sim file format", MDDetector::c_Calorimeter, 2) && Passed;
  Passed = Evaluate("c_Strip3D", "constant", "The number is part of the sim file format", MDDetector::c_Strip3D, 3) && Passed;
  Passed = Evaluate("c_ACS", "constant", "The number is part of the sim file format", MDDetector::c_ACS, 4) && Passed;
  Passed = Evaluate("c_Scintillator", "constant", "The number is part of the sim file format", MDDetector::c_Scintillator, 4) && Passed;
  Passed = Evaluate("c_DriftChamber", "constant", "The number is part of the sim file format", MDDetector::c_DriftChamber, 5) && Passed;
  Passed = Evaluate("c_Strip3DDirectional", "constant", "The number is part of the sim file format", MDDetector::c_Strip3DDirectional, 6) && Passed;
  Passed = Evaluate("c_AngerCamera", "constant", "The number is part of the sim file format", MDDetector::c_AngerCamera, 7) && Passed;
  Passed = Evaluate("c_Voxel3D", "constant", "The number is part of the sim file format", MDDetector::c_Voxel3D, 8) && Passed;
  Passed = Evaluate("c_GuardRing", "constant", "The number is part of the sim file format", MDDetector::c_GuardRing, 9) && Passed;

  // Name to type and back for all types which are not known defects
  vector<int> Types = { MDDetector::c_Strip2D, MDDetector::c_Calorimeter, MDDetector::c_Strip3D, MDDetector::c_Scintillator, MDDetector::c_DriftChamber, MDDetector::c_AngerCamera, MDDetector::c_Voxel3D, MDDetector::c_GuardRing };
  for (int Type: Types) {
    Passed = Evaluate("GetDetectorType(GetDetectorTypeName())", Type, "The name round trip restores the type", MDDetector::GetDetectorType(MDDetector::GetDetectorTypeName(Type)), Type) && Passed;
    Passed = EvaluateTrue("IsValidDetectorType(MString)", Type, "The name of the type is valid", MDDetector::IsValidDetectorType(MDDetector::GetDetectorTypeName(Type))) && Passed;
  }

  // Defect GEO-D-001: GetDetectorTypeName has no branch for the type Strip3DDirectional and returns NoDetectorType
  DisableDefaultStreams();
  MString DirectionalName = MDDetector::GetDetectorTypeName(MDDetector::c_Strip3DDirectional);
  EnableDefaultStreams();
  Passed = ExpectedToFail(EvaluateTrue("GetDetectorTypeName()", "Strip3DDirectional", "The name of the type is Strip3DDirectional", DirectionalName == "Strip3DDirectional")) && Passed;

  Passed = Evaluate("GetDetectorType()", "Strip3DDirectional", "The name is converted to the type", MDDetector::GetDetectorType("Strip3DDirectional"), MDDetector::c_Strip3DDirectional) && Passed;
  Passed = Evaluate("GetDetectorType()", "ACS name", "The ACS name is the scintillator", MDDetector::GetDetectorType(MDDetector::c_ACSName), MDDetector::c_Scintillator) && Passed;

  // Unknown types
  DisableDefaultStreams();
  int UnknownType = MDDetector::GetDetectorType("Unknown");
  MString UnknownName = MDDetector::GetDetectorTypeName(99);
  EnableDefaultStreams();
  Passed = Evaluate("GetDetectorType()", "unknown name", "An unknown name is no detector type", UnknownType, MDDetector::c_NoDetectorType) && Passed;
  Passed = EvaluateTrue("GetDetectorTypeName()", "unknown type", "An unknown type has the name NoDetectorType", UnknownName == MDDetector::c_NoDetectorTypeName) && Passed;
  Passed = EvaluateFalse("IsValidDetectorType(MString)", "unknown name", "An unknown name is invalid", MDDetector::IsValidDetectorType(MString("Unknown"))) && Passed;
  Passed = EvaluateFalse("IsValidDetectorType(MString)", "NoDetectorType", "The name of the missing type is invalid", MDDetector::IsValidDetectorType(MDDetector::c_NoDetectorTypeName)) && Passed;
  Passed = EvaluateTrue("IsValidDetectorType(MString)", "Strip3DDirectional", "The name is valid", MDDetector::IsValidDetectorType(MString("Strip3DDirectional"))) && Passed;

  // Boundaries of the type IDs
  Passed = EvaluateFalse("IsValidDetectorType(int)", "c_MinDetector - 1", "The ID below the minimum is invalid", MDDetector::IsValidDetectorType(MDDetector::c_MinDetector - 1)) && Passed;
  Passed = EvaluateTrue("IsValidDetectorType(int)", "c_MinDetector", "The minimum ID is valid", MDDetector::IsValidDetectorType(MDDetector::c_MinDetector)) && Passed;
  Passed = EvaluateTrue("IsValidDetectorType(int)", "c_MaxDetector", "The maximum ID is valid", MDDetector::IsValidDetectorType(MDDetector::c_MaxDetector)) && Passed;
  Passed = EvaluateFalse("IsValidDetectorType(int)", "c_MaxDetector + 1", "The ID above the maximum is invalid", MDDetector::IsValidDetectorType(MDDetector::c_MaxDetector + 1)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test that the constructors initialize all members
bool UTDDetector::TestConstruction()
{
  bool Passed = true;

  UTDDetectorStub Stub("Name");
  Passed = EvaluateTrue("MDDetector()", "name", "The name of the constructor is stored", Stub.GetName() == "Name") && Passed;
  Passed = EvaluateNear("MDDetector()", "noise threshold", "The noise threshold is not defined", Stub.GetNoiseThreshold(), g_DoubleNotDefined, 1e-9*g_DoubleNotDefined) && Passed;
  Passed = EvaluateNear("MDDetector()", "noise threshold sigma", "The noise threshold sigma is not defined", Stub.GetNoiseThresholdSigma(), g_DoubleNotDefined, 1e-9*g_DoubleNotDefined) && Passed;
  Passed = EvaluateNear("MDDetector()", "trigger threshold", "The trigger threshold is not defined", Stub.GetTriggerThreshold(), g_DoubleNotDefined, 1e-9*g_DoubleNotDefined) && Passed;
  Passed = EvaluateNear("MDDetector()", "trigger threshold sigma", "The trigger threshold sigma is not defined", Stub.GetTriggerThresholdSigma(), g_DoubleNotDefined, 1e-9*g_DoubleNotDefined) && Passed;
  Passed = EvaluateNear("MDDetector()", "failure rate", "The failure rate is not defined", Stub.GetFailureRate(), g_DoubleNotDefined, 1e-9*g_DoubleNotDefined) && Passed;
  Passed = EvaluateNear("MDDetector()", "overflow", "The overflow is not defined", Stub.GetOverflow(), g_DoubleNotDefined, 1e-9*g_DoubleNotDefined) && Passed;
  Passed = EvaluateNear("MDDetector()", "overflow sigma", "The overflow sigma is not defined", Stub.GetOverflowSigma(), g_DoubleNotDefined, 1e-9*g_DoubleNotDefined) && Passed;
  Passed = EvaluateTrue("MDDetector()", "structural offset", "The structural offset is not defined", Stub.GetStructuralOffset() == g_VectorNotDefined) && Passed;
  Passed = EvaluateTrue("MDDetector()", "structural pitch", "The structural pitch is not defined", Stub.GetStructuralPitch() == g_VectorNotDefined) && Passed;
  Passed = EvaluateTrue("MDDetector()", "structural size", "The structural size is not defined", Stub.GetStructuralSize() == g_VectorNotDefined) && Passed;
  Passed = EvaluateTrue("MDDetector()", "detector volume", "There is no detector volume", Stub.GetDetectorVolume() == nullptr) && Passed;
  Passed = EvaluateTrue("MDDetector()", "common volume", "There is no common volume", Stub.GetCommonVolume() == nullptr) && Passed;
  Passed = EvaluateFalse("MDDetector()", "guard ring", "There is no guard ring", Stub.HasGuardRing()) && Passed;
  Passed = EvaluateTrue("MDDetector()", "guard ring pointer", "The guard ring pointer is empty", Stub.GetGuardRing() == nullptr) && Passed;
  Passed = EvaluateFalse("MDDetector()", "named", "It is not a named detector", Stub.IsNamedDetector()) && Passed;
  Passed = EvaluateFalse("MDDetector()", "named detectors", "It has no named detectors", Stub.HasNamedDetectors()) && Passed;
  Passed = EvaluateFalse("MDDetector()", "divisions", "No divisions are used", Stub.UseDivisions()) && Passed;
  Passed = Evaluate("MDDetector()", "energy loss type", "The energy loss type is unknown", Stub.GetEnergyLossType(), MDDetector::c_EnergyLossTypeUnknown) && Passed;
  Passed = EvaluateNear("MDDetector()", "energy resolution type", "The energy resolution type is unknown", Stub.GetEnergyResolutionType(), MDDetector::c_EnergyResolutionTypeUnknown, 1e-12) && Passed;

  // Fill a memory block with a pattern and look for not initialized members
  const unsigned char Pattern = 0xA7;
  int PatternInt = 0;
  memset(&PatternInt, Pattern, sizeof(int));

  vector<unsigned char> Block(sizeof(UTDDetectorStub) + 64);
  void* Aligned = reinterpret_cast<void*>((reinterpret_cast<size_t>(Block.data()) + 63) & ~static_cast<size_t>(63));

  memset(Block.data(), Pattern, Block.size());
  UTDDetectorStub* Placed = new (Aligned) UTDDetectorStub("Placed");
  int SensID = Placed->m_SensID;
  bool PulseShapeSet = Placed->m_PulseShapeSet;
  Placed->~UTDDetectorStub();

  // Defect GEO-D-002: The standard constructor does not initialize the sensitive volume ID m_SensID
  Passed = ExpectedToFail(EvaluateTrue("MDDetector()", "pattern filled memory, m_SensID", "The member m_SensID is initialized", SensID != PatternInt)) && Passed;
  Passed = EvaluateFalse("MDDetector()", "pattern filled memory, m_PulseShapeSet", "The member m_PulseShapeSet is initialized", PulseShapeSet) && Passed;

  // Now the copy constructor:
  UTDDetectorStub Source("Source");
  memset(Block.data(), Pattern, Block.size());
  Placed = new (Aligned) UTDDetectorStub(Source);
  unsigned char PulseShapeSetByte = 0;
  memcpy(&PulseShapeSetByte, &Placed->m_PulseShapeSet, 1);
  Placed->~UTDDetectorStub();

  // Defect GEO-D-003: The copy constructor does not initialize m_PulseShapeSet
  Passed = ExpectedToFail(EvaluateTrue("MDDetector(const MDDetector&)", "pattern filled memory, m_PulseShapeSet", "The member m_PulseShapeSet is initialized", PulseShapeSetByte != Pattern)) && Passed;

  // The ID counter wraps at its maximum, and the detector still needs an ID
  int OldCounter = UTDDetectorStub::m_IDCounter;
  UTDDetectorStub::m_IDCounter = numeric_limits<int>::max();
  memset(Block.data(), Pattern, Block.size());
  Placed = new (Aligned) UTDDetectorStub("Wrapped");
  int WrappedID = Placed->m_ID;
  Placed->~UTDDetectorStub();
  UTDDetectorStub::m_IDCounter = OldCounter;

  // Defect GEO-D-004: When the ID counter reaches its maximum the new detector gets no ID at all
  Passed = ExpectedToFail(EvaluateTrue("MDDetector()", "ID counter at maximum", "The member m_ID is initialized", WrappedID != PatternInt)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the simple setters and getters
bool UTDDetector::TestSettersGetters()
{
  bool Passed = true;

  UTDDetectorStub Stub;

  Stub.SetName("Other");
  Passed = EvaluateTrue("SetName()", "other", "The name is replaced", Stub.GetName() == "Other") && Passed;
  Passed = Evaluate("GetType()", "stub", "The type set by the derived class is returned", Stub.GetType(), MDDetector::c_ACS) && Passed;
  Passed = EvaluateTrue("GetTypeName()", "stub", "The description is unknown as long as a derived class does not set it", Stub.GetTypeName() == "Unknown") && Passed;

  Stub.UseDivisions("DIVX", "DIVY", "DIVZ");
  Passed = EvaluateTrue("UseDivisions()", "three names", "Divisions are used", Stub.UseDivisions()) && Passed;
  Passed = EvaluateTrue("GetShortNameDivisionX()", "DIVX", "The x-name is stored", Stub.GetShortNameDivisionX() == "DIVX") && Passed;
  Passed = EvaluateTrue("GetShortNameDivisionY()", "DIVY", "The y-name is stored", Stub.GetShortNameDivisionY() == "DIVY") && Passed;
  Passed = EvaluateTrue("GetShortNameDivisionZ()", "DIVZ", "The z-name is stored", Stub.GetShortNameDivisionZ() == "DIVZ") && Passed;

  Stub.SetNoiseThreshold(12.5);
  Stub.SetNoiseThresholdSigma(1.5);
  Stub.SetTriggerThreshold(22.5);
  Stub.SetTriggerThresholdSigma(2.5);
  Stub.SetOverflow(1000.5);
  Stub.SetOverflowSigma(3.5);
  Stub.SetFailureRate(0.125);
  Passed = EvaluateNear("SetNoiseThreshold()", "12.5", "The value is stored", Stub.GetNoiseThreshold(), 12.5, 1e-12) && Passed;
  Passed = EvaluateNear("SetNoiseThresholdSigma()", "1.5", "The value is stored", Stub.GetNoiseThresholdSigma(), 1.5, 1e-12) && Passed;
  Passed = EvaluateNear("SetTriggerThreshold()", "22.5", "The value is stored", Stub.GetTriggerThreshold(), 22.5, 1e-12) && Passed;
  Passed = EvaluateNear("SetTriggerThresholdSigma()", "2.5", "The value is stored", Stub.GetTriggerThresholdSigma(), 2.5, 1e-12) && Passed;
  Passed = EvaluateNear("SetOverflow()", "1000.5", "The value is stored", Stub.GetOverflow(), 1000.5, 1e-12) && Passed;
  Passed = EvaluateNear("SetOverflowSigma()", "3.5", "The value is stored", Stub.GetOverflowSigma(), 3.5, 1e-12) && Passed;
  Passed = EvaluateNear("SetFailureRate()", "0.125", "The value is stored", Stub.GetFailureRate(), 0.125, 1e-12) && Passed;

  // Upper limit by the documented formula 1.25*T + 7*S
  Passed = EvaluateNear("GetSecureUpperLimitTriggerThreshold()", "22.5, 2.5", "The limit is 1.25 T + 7 S", Stub.GetSecureUpperLimitTriggerThreshold(), 45.625, 1e-12) && Passed;

  Stub.SetNoiseThresholdEqualsTriggerThreshold();
  Passed = EvaluateTrue("SetNoiseThresholdEqualsTriggerThreshold()", "default argument", "The flag is set by default", Stub.GetNoiseThresholdEqualsTriggerThreshold()) && Passed;
  Stub.SetNoiseThresholdEqualsTriggerThreshold(false);
  Passed = EvaluateFalse("SetNoiseThresholdEqualsTriggerThreshold()", "false", "The flag can be cleared", Stub.GetNoiseThresholdEqualsTriggerThreshold()) && Passed;

  Stub.SetStructuralOffset(MVector(0.25, 0.5, 0.75));
  Stub.SetStructuralPitch(MVector(1.25, 1.5, 1.75));
  Passed = EvaluateVectorNear("SetStructuralOffset()", "0.25, 0.5, 0.75", "The offset is stored", Stub.GetStructuralOffset(), MVector(0.25, 0.5, 0.75), 1e-12) && Passed;
  Passed = EvaluateVectorNear("SetStructuralPitch()", "1.25, 1.5, 1.75", "The pitch is stored", Stub.GetStructuralPitch(), MVector(1.25, 1.5, 1.75), 1e-12) && Passed;

  // The energy resolution type can only be set once
  UTDDetectorStub Type;
  DisableDefaultStreams();
  bool Invalid = Type.SetEnergyResolutionType(99);
  EnableDefaultStreams();
  Passed = EvaluateFalse("SetEnergyResolutionType()", "invalid type", "An invalid type is rejected", Invalid) && Passed;
  Passed = EvaluateTrue("SetEnergyResolutionType()", "first time", "The first type is accepted", Type.SetEnergyResolutionType(MDDetector::c_EnergyResolutionTypeGauss)) && Passed;
  Passed = EvaluateTrue("SetEnergyResolutionType()", "same type", "Setting the same type again is fine", Type.SetEnergyResolutionType(MDDetector::c_EnergyResolutionTypeGauss)) && Passed;
  DisableDefaultStreams();
  bool Overwrite = Type.SetEnergyResolutionType(MDDetector::c_EnergyResolutionTypeLorentz);
  EnableDefaultStreams();
  Passed = EvaluateFalse("SetEnergyResolutionType()", "other type", "Overwriting the type is rejected", Overwrite) && Passed;
  Passed = EvaluateNear("GetEnergyResolutionType()", "after the rejection", "The first type is kept", Type.GetEnergyResolutionType(), MDDetector::c_EnergyResolutionTypeGauss, 1e-12) && Passed;
  UTDDetectorStub Unknown;
  DisableDefaultStreams();
  bool UnknownType = Unknown.SetEnergyResolutionType(MDDetector::c_EnergyResolutionTypeUnknown);
  EnableDefaultStreams();
  Passed = EvaluateTrue("SetEnergyResolutionType()", "unknown on a new detector", "Setting the current type again is fine", UnknownType) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the input checks and the table look-ups of the energy resolution
bool UTDDetector::TestEnergyResolutionTable()
{
  bool Passed = true;

  UTDDetectorStub Stub;
  Stub.SetEnergyResolutionType(MDDetector::c_EnergyResolutionTypeGaussLandau);
  Stub.SetEnergyResolution(100.0, 110.0, 2.0, 120.0, 3.0, 0.2);
  Stub.SetEnergyResolution(300.0, 310.0, 6.0, 320.0, 7.0, 0.6);

  // Invalid entries must not change the tables:
  DisableDefaultStreams();
  Stub.SetEnergyResolution(-1.0, 110.0, 2.0);
  Stub.SetEnergyResolution(200.0, -1.0, 2.0);
  Stub.SetEnergyResolution(200.0, 110.0, -1.0);
  Stub.SetEnergyResolution(200.0, 110.0, 2.0, -1.0);
  Stub.SetEnergyResolution(200.0, 110.0, 2.0, 120.0, -1.0);
  Stub.SetEnergyResolution(200.0, 110.0, 2.0, 120.0, 3.0, -0.1);
  Stub.SetEnergyResolution(200.0, 110.0, 2.0, 120.0, 3.0, 1.1);
  EnableDefaultStreams();

  // Linear interpolation at 200 keV as the middle of the table
  Passed = EvaluateNear("GetEnergyResolutionPeak1()", "200 keV", "All invalid entries are ignored and the first peak is interpolated", Stub.GetEnergyResolutionPeak1(200.0), 210.0, 1e-9) && Passed;
  Passed = EvaluateNear("GetEnergyResolutionWidth1()", "200 keV", "The first width is interpolated", Stub.GetEnergyResolutionWidth1(200.0), 4.0, 1e-9) && Passed;
  Passed = EvaluateNear("GetEnergyResolutionPeak2()", "200 keV", "The second peak is interpolated", Stub.GetEnergyResolutionPeak2(200.0), 220.0, 1e-9) && Passed;
  Passed = EvaluateNear("GetEnergyResolutionWidth2()", "200 keV", "The second width is interpolated", Stub.GetEnergyResolutionWidth2(200.0), 5.0, 1e-9) && Passed;
  Passed = EvaluateNear("GetEnergyResolutionRatio()", "200 keV", "The ratio is interpolated", Stub.GetEnergyResolutionRatio(200.0), 0.4, 1e-9) && Passed;
  Passed = EvaluateNear("GetEnergyResolution()", "200 keV, Gauss-Landau", "The average width is the width of the Gaussian", Stub.GetEnergyResolution(200.0), 4.0, 1e-9) && Passed;

  // Extrapolation outside the table and clamping
  DisableDefaultStreams();
  double Peak1 = Stub.GetEnergyResolutionPeak1(-100.0);
  double Width1 = Stub.GetEnergyResolutionWidth1(-100.0);
  double Peak2 = Stub.GetEnergyResolutionPeak2(-200.0);
  double Width2 = Stub.GetEnergyResolutionWidth2(-100.0);
  double RatioLow = Stub.GetEnergyResolutionRatio(-100.0);
  double RatioHigh = Stub.GetEnergyResolutionRatio(1000.0);
  EnableDefaultStreams();
  Passed = EvaluateNear("GetEnergyResolutionPeak1()", "extrapolated to -90 keV", "A negative peak is clamped to zero", Peak1, 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetEnergyResolutionWidth1()", "extrapolated to -2 keV", "A negative width is clamped to zero", Width1, 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetEnergyResolutionPeak2()", "extrapolated to -180 keV", "A negative peak is clamped to zero", Peak2, 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetEnergyResolutionWidth2()", "extrapolated to -1 keV", "A negative width is clamped to zero", Width2, 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetEnergyResolutionRatio()", "extrapolated to -0.2", "A negative ratio is clamped to zero", RatioLow, 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetEnergyResolutionRatio()", "extrapolated to 2", "A ratio above one is clamped to one", RatioHigh, 1.0, 1e-12) && Passed;

  // The types without a width
  UTDDetectorStub Ideal;
  Ideal.SetEnergyResolutionType(MDDetector::c_EnergyResolutionTypeIdeal);
  Passed = EvaluateNear("GetEnergyResolution()", "ideal", "An ideal detector has no width", Ideal.GetEnergyResolution(100.0), 0.0, 1e-12) && Passed;
  UTDDetectorStub None;
  None.SetEnergyResolutionType(MDDetector::c_EnergyResolutionTypeNone);
  Passed = EvaluateNear("GetEnergyResolution()", "none", "A detector without energy has no width", None.GetEnergyResolution(100.0), 0.0, 1e-12) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test applying the energy resolution
bool UTDDetector::TestApplyEnergyResolution()
{
  bool Passed = true;
  gRandom->SetSeed(2001);

  // Unknown type
  UTDDetectorStub Unknown;
  double Energy = 100.0;
  DisableDefaultStreams();
  bool Result = Unknown.ApplyEnergyResolution(Energy);
  EnableDefaultStreams();
  Passed = EvaluateFalse("ApplyEnergyResolution()", "unknown type", "An unknown type is an error", Result) && Passed;
  Passed = EvaluateNear("ApplyEnergyResolution()", "unknown type", "The energy stays unchanged", Energy, 100.0, 1e-12) && Passed;

  // Ideal and none
  UTDDetectorStub Ideal;
  Ideal.SetEnergyResolutionType(MDDetector::c_EnergyResolutionTypeIdeal);
  Energy = 100.0;
  Passed = EvaluateTrue("ApplyEnergyResolution()", "ideal", "The ideal type is successful", Ideal.ApplyEnergyResolution(Energy)) && Passed;
  Passed = EvaluateNear("ApplyEnergyResolution()", "ideal", "The energy stays unchanged", Energy, 100.0, 1e-12) && Passed;
  UTDDetectorStub None;
  None.SetEnergyResolutionType(MDDetector::c_EnergyResolutionTypeNone);
  Energy = 100.0;
  Passed = EvaluateTrue("ApplyEnergyResolution()", "none", "The type none is successful", None.ApplyEnergyResolution(Energy)) && Passed;
  Passed = EvaluateNear("ApplyEnergyResolution()", "none", "The energy is zero", Energy, 0.0, 1e-12) && Passed;

  // Gauss: the width at the input energy is used, not the one at the already shifted peak: peak = 2 E, width = E/100
  UTDDetectorStub Gauss;
  Gauss.SetEnergyResolutionType(MDDetector::c_EnergyResolutionTypeGauss);
  Gauss.SetEnergyResolution(100.0, 200.0, 1.0);
  Gauss.SetEnergyResolution(1000.0, 2000.0, 10.0);
  vector<double> Values;
  for (unsigned int i = 0; i < c_Samples; ++i) {
    Energy = 500.0;
    Gauss.ApplyEnergyResolution(Energy);
    Values.push_back(Energy);
  }
  Passed = CheckGauss("ApplyEnergyResolution()", "Gauss with a shifted peak", Values, 1000.0, 5.0) && Passed;

  // Gauss-Landau whose peak is identical to the input energy, and Gaussian only with the Landau part switched off
  UTDDetectorStub GaussOnly;
  GaussOnly.SetEnergyResolutionType(MDDetector::c_EnergyResolutionTypeGaussLandau);
  GaussOnly.SetEnergyResolution(100.0, 100.0, 4.0, 100.0, 1.0, 1.0);
  GaussOnly.SetEnergyResolution(900.0, 900.0, 4.0, 900.0, 1.0, 1.0);
  Values.clear();
  for (unsigned int i = 0; i < c_Samples; ++i) {
    Energy = 500.0;
    GaussOnly.ApplyEnergyResolution(Energy);
    Values.push_back(Energy);
  }
  double Mean = 0.0;
  double RMS = 0.0;
  GetGaussLandauMoments(500.0, 4.0, 500.0, 1.0, 1.0, Mean, RMS);
  Passed = EvaluateNear("ApplyEnergyResolution()", "Gauss-Landau with ratio 1", "The mean is the one of the truncated Gaussian", MStatistics::GetMean(Values), Mean, GetMeanTolerance(RMS)) && Passed;
  Passed = EvaluateNear("ApplyEnergyResolution()", "Gauss-Landau with ratio 1", "The width is the one of the truncated Gaussian", MStatistics::GetStandardDeviation(Values), RMS, GetRMSTolerance(RMS)) && Passed;

  // Peak different from the input energy: the distribution must be the same as with an identical peak at this energy
  UTDDetectorStub Shifted;
  Shifted.SetEnergyResolutionType(MDDetector::c_EnergyResolutionTypeGaussLandau);
  Shifted.SetEnergyResolution(100.0, 130.0, 2.0, 130.0, 3.0, 0.7);
  Shifted.SetEnergyResolution(900.0, 930.0, 2.0, 930.0, 3.0, 0.7);
  Values.clear();
  for (unsigned int i = 0; i < c_Samples; ++i) {
    Energy = 100.0;
    Shifted.ApplyEnergyResolution(Energy);
    Values.push_back(Energy);
  }
  GetGaussLandauMoments(130.0, 2.0, 130.0, 3.0, 0.7, Mean, RMS);

  // Defect GEO-D-005: Gauss-Landau rejection sampling takes its maximum at the input energy instead of at the peak, so a calibration offset turns the peak into a flat distribution
  Passed = ExpectedToFail(EvaluateNear("ApplyEnergyResolution()", "Gauss-Landau peak 130 keV at input 100 keV", "The width agrees with the numeric integration of the distribution", MStatistics::GetStandardDeviation(Values), RMS, 2.0*GetRMSTolerance(RMS))) && Passed;

  // A wide Landau tail makes rejections frequent
  UTDDetectorStub Tail;
  Tail.SetEnergyResolutionType(MDDetector::c_EnergyResolutionTypeGaussLandau);
  Tail.SetEnergyResolution(1000.0, 1000.0, 1.0, 1000.0, 30.0, 0.9);
  Tail.SetEnergyResolution(3000.0, 3000.0, 1.0, 3000.0, 30.0, 0.9);
  Values.clear();
  for (unsigned int i = 0; i < c_Samples; ++i) {
    Energy = 2000.0;
    Tail.ApplyEnergyResolution(Energy);
    Values.push_back(Energy);
  }
  GetGaussLandauMoments(2000.0, 1.0, 2000.0, 30.0, 0.9, Mean, RMS);

  // Defect GEO-D-006: After 100 rejections of the Gauss-Landau sampling the last rejected random energy is used instead of a sample of the distribution
  Passed = ExpectedToFail(EvaluateNear("ApplyEnergyResolution()", "Gauss-Landau narrow Gaussian and wide Landau", "The width agrees with the numeric integration of the distribution", MStatistics::GetStandardDeviation(Values), RMS, 2.0*GetRMSTolerance(RMS))) && Passed;

  // A sharp Gaussian part with a zero width makes the distribution a line plus the Landau part
  UTDDetectorStub Sharp;
  Sharp.SetEnergyResolutionType(MDDetector::c_EnergyResolutionTypeGaussLandau);
  Sharp.SetEnergyResolution(100.0, 100.0, 0.0, 100.0, 3.0, 1.0);
  Sharp.SetEnergyResolution(900.0, 900.0, 0.0, 900.0, 3.0, 1.0);
  Values.clear();
  for (unsigned int i = 0; i < 1000; ++i) {
    Energy = 500.0;
    Sharp.ApplyEnergyResolution(Energy);
    Values.push_back(Energy);
  }

  // Defect GEO-D-007: With a Gaussian width of zero the Gauss-Landau sampling divides zero by zero, accepts the first trial, and returns uniform energies
  Passed = ExpectedToFail(EvaluateNear("ApplyEnergyResolution()", "Gauss-Landau with width zero and ratio 1", "A Gaussian of width zero without any Landau part returns the peak", MStatistics::GetStandardDeviation(Values), 0.0, 0.5)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test applying the time resolution
bool UTDDetector::TestApplyTimeResolution()
{
  bool Passed = true;
  gRandom->SetSeed(2002);

  UTDDetectorStub Stub;
  Passed = EvaluateFalse("HasTimeResolution()", "unknown type", "An unknown type has no time resolution", Stub.HasTimeResolution()) && Passed;
  DisableDefaultStreams();
  double UnknownResolution = Stub.GetTimeResolution(100.0);
  double Time = 5.0;
  bool Result = Stub.ApplyTimeResolution(Time, 100.0);
  EnableDefaultStreams();
  Passed = EvaluateNear("GetTimeResolution()", "unknown type", "The resolution of an unknown type is huge", UnknownResolution, 1E15, 1.0) && Passed;
  Passed = EvaluateFalse("ApplyTimeResolution()", "unknown type", "An unknown type is an error", Result) && Passed;
  Passed = EvaluateNear("ApplyTimeResolution()", "unknown type", "The time stays unchanged", Time, 5.0, 1e-12) && Passed;

  Stub.m_TimeResolutionType = MDDetector::c_TimeResolutionTypeNone;
  Passed = EvaluateFalse("HasTimeResolution()", "type none", "The type none has no time resolution", Stub.HasTimeResolution()) && Passed;
  Passed = EvaluateNear("GetTimeResolution()", "type none", "The resolution of the type none is huge", Stub.GetTimeResolution(100.0), 1E15, 1.0) && Passed;

  Stub.m_TimeResolutionType = MDDetector::c_TimeResolutionTypeIdeal;
  Passed = EvaluateTrue("HasTimeResolution()", "ideal", "An ideal type has a time resolution", Stub.HasTimeResolution()) && Passed;
  Passed = EvaluateNear("GetTimeResolution()", "ideal", "The resolution of the ideal type is zero", Stub.GetTimeResolution(100.0), 0.0, 1e-12) && Passed;
  Time = 5.0;
  Passed = EvaluateTrue("ApplyTimeResolution()", "ideal", "The ideal type is successful", Stub.ApplyTimeResolution(Time, 100.0)) && Passed;
  Passed = EvaluateNear("ApplyTimeResolution()", "ideal", "The time stays unchanged", Time, 5.0, 1e-12) && Passed;

  // One table entry: constant
  UTDDetectorStub Single;
  Single.SetTimeResolution(100.0, 3.0);
  Passed = EvaluateTrue("SetTimeResolution()", "one point", "The type is now Gauss", Single.HasTimeResolution()) && Passed;
  Passed = EvaluateNear("GetTimeResolution()", "one point", "One point is a constant", Single.GetTimeResolution(5000.0), 3.0, 1e-12) && Passed;
  vector<double> Values;
  for (unsigned int i = 0; i < c_Samples; ++i) {
    Time = 50.0;
    Single.ApplyTimeResolution(Time, 1000.0);
    Values.push_back(Time);
  }
  Passed = CheckGauss("ApplyTimeResolution()", "one point", Values, 50.0, 3.0) && Passed;

  // Two entries: interpolation and extrapolation
  UTDDetectorStub Table;
  Table.SetTimeResolution(100.0, 1.0);
  Table.SetTimeResolution(300.0, 3.0);
  Passed = EvaluateNear("GetTimeResolution()", "between the points", "The resolution is interpolated", Table.GetTimeResolution(200.0), 2.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetTimeResolution()", "above the table", "The resolution is extrapolated", Table.GetTimeResolution(500.0), 5.0, 1e-12) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the thresholds and the overflow on the base class
bool UTDDetector::TestThresholdsAndOverflow()
{
  bool Passed = true;
  gRandom->SetSeed(2003);

  UTDDetectorStub Stub;
  Stub.SetNoiseThreshold(10.0);
  Stub.SetNoiseThresholdSigma(0.0);
  Stub.SetTriggerThreshold(20.0);
  Stub.SetTriggerThresholdSigma(0.0);
  Stub.SetOverflow(100.0);
  Stub.SetOverflowSigma(0.0);

  double Energy = 9.999;
  Passed = EvaluateTrue("ApplyNoiseThreshold()", "just below", "The threshold is applied", Stub.ApplyNoiseThreshold(Energy)) && Passed;
  Passed = EvaluateNear("ApplyNoiseThreshold()", "just below", "The energy is zero", Energy, 0.0, 1e-12) && Passed;
  Energy = 10.001;
  Passed = EvaluateFalse("ApplyNoiseThreshold()", "just above", "The threshold is not applied", Stub.ApplyNoiseThreshold(Energy)) && Passed;
  Passed = EvaluateNear("ApplyNoiseThreshold()", "just above", "The energy stays", Energy, 10.001, 1e-12) && Passed;

  Energy = 99.999;
  Passed = EvaluateFalse("ApplyOverflow()", "just below", "The overflow is not applied", Stub.ApplyOverflow(Energy)) && Passed;
  Passed = EvaluateNear("ApplyOverflow()", "just below", "The energy stays", Energy, 99.999, 1e-12) && Passed;
  Energy = 100.001;
  Passed = EvaluateTrue("ApplyOverflow()", "just above", "The overflow is applied", Stub.ApplyOverflow(Energy)) && Passed;
  Passed = EvaluateNear("ApplyOverflow()", "just above", "The energy is the overflow", Energy, 100.0, 1e-12) && Passed;

  MDGridPoint Point(0, 0, 0, MDGridPoint::c_Voxel);
  Passed = EvaluateFalse("IsAboveTriggerThreshold()", "just below", "There is no trigger", Stub.IsAboveTriggerThreshold(19.999, Point)) && Passed;
  Passed = EvaluateTrue("IsAboveTriggerThreshold()", "just above", "There is a trigger", Stub.IsAboveTriggerThreshold(20.001, Point)) && Passed;

  // Blocked channel: the base class has one channel
  Passed = EvaluateTrue("BlockTriggerChannel()", "channel 0, 0", "The only channel can be blocked", Stub.BlockTriggerChannel(0, 0)) && Passed;
  Passed = EvaluateFalse("IsAboveTriggerThreshold()", "blocked channel", "A blocked channel never triggers", Stub.IsAboveTriggerThreshold(1000.0, Point)) && Passed;

  // The noise threshold is the trigger threshold with the sigma of the trigger threshold
  UTDDetectorStub Equal;
  Equal.SetNoiseThresholdEqualsTriggerThreshold(true);
  Equal.SetTriggerThreshold(20.0);
  Equal.SetTriggerThresholdSigma(2.0);
  double Survivors = 0.0;
  for (unsigned int i = 0; i < c_Samples; ++i) {
    Energy = 22.0;
    if (Equal.ApplyNoiseThreshold(Energy) == false) {
      Survivors += 1.0;
    }
  }
  Passed = EvaluateNear("ApplyNoiseThreshold()", "noise equals trigger, one sigma above", "The surviving fraction is Phi(1)", Survivors/c_Samples, GetPhi(1.0), GetFractionTolerance(GetPhi(1.0))) && Passed;
  Passed = EvaluateTrue("IsAboveTriggerThreshold()", "noise equals trigger", "A remaining hit always triggers", Equal.IsAboveTriggerThreshold(0.1, Point)) && Passed;
  Passed = EvaluateTrue("BlockTriggerChannel()", "noise equals trigger", "The channel can be blocked", Equal.BlockTriggerChannel(0, 0)) && Passed;
  Passed = EvaluateFalse("IsAboveTriggerThreshold()", "noise equals trigger, blocked", "A blocked channel also does not trigger in this mode", Equal.IsAboveTriggerThreshold(1000.0, Point)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the pulse shape
bool UTDDetector::TestPulseShape()
{
  bool Passed = true;

  UTDDetectorStub Stub;
  Passed = EvaluateNear("ApplyPulseShape()", "no shape", "Without a pulse shape the result is zero", Stub.ApplyPulseShape(5.0, 100.0), 0.0, 1e-12) && Passed;

  // Polynomial by hand: (2 + 3 t) E
  Stub.SetPulseShape(2.0, 3.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 10.0);
  Passed = EvaluateNear("ApplyPulseShape()", "inside", "The result is the polynomial times the energy", Stub.ApplyPulseShape(4.0, 10.0), 140.0, 1e-9) && Passed;
  Passed = EvaluateNear("ApplyPulseShape()", "at the lower limit", "The lower limit is included", Stub.ApplyPulseShape(1.0, 10.0), 50.0, 1e-9) && Passed;
  Passed = EvaluateNear("ApplyPulseShape()", "at the upper limit", "The upper limit is included", Stub.ApplyPulseShape(10.0, 10.0), 320.0, 1e-9) && Passed;
  Passed = EvaluateNear("ApplyPulseShape()", "below", "Below the lower limit the result is zero", Stub.ApplyPulseShape(0.999, 10.0), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("ApplyPulseShape()", "above", "Above the upper limit the result is zero", Stub.ApplyPulseShape(10.001, 10.0), 0.0, 1e-12) && Passed;

  // Replacing the shape:
  Stub.SetPulseShape(1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 2.0);
  Passed = EvaluateNear("SetPulseShape()", "second shape", "The second shape replaces the first one", Stub.ApplyPulseShape(1.0, 7.0), 7.0, 1e-9) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the energy calibration and the energy loss map
bool UTDDetector::TestCalibrationAndEnergyLoss()
{
  bool Passed = true;

  // Energy calibration
  UTDDetectorStub Stub;
  double Energy = 10.0;
  Passed = EvaluateTrue("ApplyEnergyCalibration()", "no calibration", "Without a calibration the call is successful", Stub.ApplyEnergyCalibration(Energy)) && Passed;
  Passed = EvaluateNear("ApplyEnergyCalibration()", "no calibration", "The energy stays unchanged", Energy, 10.0, 1e-12) && Passed;

  MFunction Calibration;
  Calibration.Add(0.0, 1.0);
  Calibration.Add(100.0, 201.0);
  Stub.SetEnergyCalibration(Calibration);
  Passed = EvaluateTrue("ApplyEnergyCalibration()", "calibration", "With a calibration the call is successful", Stub.ApplyEnergyCalibration(Energy)) && Passed;
  Passed = EvaluateNear("ApplyEnergyCalibration()", "calibration", "The energy is calibrated: 1 + 2 E", Energy, 21.0, 1e-9) && Passed;

  // An empty calibration switches the calibration off
  MFunction Empty;
  Stub.SetEnergyCalibration(Empty);
  Energy = 10.0;
  Passed = EvaluateTrue("ApplyEnergyCalibration()", "empty calibration", "With an empty calibration the call is successful", Stub.ApplyEnergyCalibration(Energy)) && Passed;
  Passed = EvaluateNear("ApplyEnergyCalibration()", "empty calibration", "The energy stays unchanged", Energy, 10.0, 1e-12) && Passed;

  // Energy loss map
  UTDDetectorStub Loss;
  DisableDefaultStreams();
  Loss.SetEnergyLossMap(GetTemporaryFileName("DoesNotExist.dat"));
  EnableDefaultStreams();
  Passed = Evaluate("SetEnergyLossMap()", "missing file", "A missing map results in no energy loss", Loss.GetEnergyLossType(), MDDetector::c_EnergyLossTypeNone) && Passed;

  const MString FileName = GetTemporaryFileName("Loss.dat");
  Passed = EvaluateTrue("WriteTextFile()", "loss map", "The map can be created", WriteTextFile(FileName, "XA -1 1\nYA -1 1\nZA -1 1\nDP -1 -1 -1 0.5\nDP -1 -1 1 0.5\nDP -1 1 -1 0.5\nDP -1 1 1 0.5\nDP 1 -1 -1 0.5\nDP 1 -1 1 0.5\nDP 1 1 -1 0.5\nDP 1 1 1 0.5\nEN\n")) && Passed;
  UTDDetectorStub Map;
  Map.SetEnergyLossMap(FileName);
  Passed = Evaluate("SetEnergyLossMap()", "valid file", "A valid map is used", Map.GetEnergyLossType(), MDDetector::c_EnergyLossTypeMap) && Passed;
  Passed = EvaluateNear("GetEnergyLossMap()", "valid file", "The map returns the values of the file", Map.GetEnergyLossMap().Evaluate(0.0, 0.0, 0.0), 0.5, 1e-9) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the handling of the sensitive volumes and the detector volume
bool UTDDetector::TestVolumes()
{
  bool Passed = true;

  MDDetector::ResetIDs();

  UTDDetectorStub Stub;
  MDVolume V1("V1");
  MDVolume V2("V2");
  Stub.AddSensitiveVolume(&V1);
  Stub.AddSensitiveVolume(&V2);

  Passed = Evaluate("GetNSensitiveVolumes()", "two volumes", "Both volumes are counted", Stub.GetNSensitiveVolumes(), 2u) && Passed;
  Passed = EvaluateTrue("GetSensitiveVolume()", "index 0", "The first volume is returned", Stub.GetSensitiveVolume(0) == &V1) && Passed;
  Passed = EvaluateTrue("GetSensitiveVolume()", "index n - 1", "The last volume is returned", Stub.GetSensitiveVolume(1) == &V2) && Passed;
  DisableDefaultStreams();
  MDVolume* OutOfBounds = Stub.GetSensitiveVolume(2);
  EnableDefaultStreams();
  Passed = EvaluateTrue("GetSensitiveVolume()", "index n", "The index after the last one returns a nullptr", OutOfBounds == nullptr) && Passed;

  Passed = Evaluate("AddSensitiveVolume()", "first volume", "The first volume gets the ID 1", V1.GetSensitiveVolumeID(), 1) && Passed;
  Passed = Evaluate("AddSensitiveVolume()", "second volume", "The second volume gets the ID 2", V2.GetSensitiveVolumeID(), 2) && Passed;
  Passed = EvaluateTrue("AddSensitiveVolume()", "volume", "The volume knows the detector", V1.GetDetector() == &Stub) && Passed;
  Passed = Evaluate("GetGlobalNSensitiveVolumes()", "two volumes", "Two sensitive volumes exist", Stub.GetGlobalNSensitiveVolumes(), 2) && Passed;

  MDDetector::ResetIDs();
  Passed = Evaluate("ResetIDs()", "after reset", "No sensitive volumes are registered", Stub.GetGlobalNSensitiveVolumes(), 0) && Passed;

  Stub.SetDetectorVolume(&V1);
  Passed = EvaluateTrue("SetDetectorVolume()", "V1", "The volume is stored", Stub.GetDetectorVolume() == &V1) && Passed;
  Stub.SetCommonVolume(&V2);
  Passed = EvaluateTrue("SetCommonVolume()", "V2", "The common volume is stored", Stub.GetCommonVolume() == &V2) && Passed;

  // A named detector does not take over the volume
  UTDDetectorStub Base("Base");
  UTDDetectorStub* Named = new UTDDetectorStub("Named");
  Base.AddNamedDetector(Named);
  MDVolume V3("V3");
  MDDetector* None = nullptr;
  V3.SetIsDetectorVolume(None);
  Named->SetDetectorVolume(&V3);
  Passed = EvaluateTrue("SetDetectorVolume()", "named detector", "The named detector stores the volume", Named->GetDetectorVolume() == &V3) && Passed;
  Passed = EvaluateTrue("SetDetectorVolume()", "named detector", "The volume does not know the named detector as its detector", V3.GetDetector() != Named) && Passed;
  delete Named;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test all validation branches of the base class
bool UTDDetector::TestValidate()
{
  bool Passed = true;

  // Helper for the failing validations: silent call
  auto ValidateQuiet = [this](MDDetector& Detector) {
    DisableDefaultStreams();
    bool Result = Detector.Validate();
    EnableDefaultStreams();
    return Result;
  };

  UTDDetectorStub NoVolumes;
  Passed = EvaluateFalse("Validate()", "no volumes", "A detector without volumes is invalid", ValidateQuiet(NoVolumes)) && Passed;

  UTDDetectorStub NoSensitive;
  MDVolume DetectorOnly("DetectorOnly");
  NoSensitive.SetDetectorVolume(&DetectorOnly);
  Passed = EvaluateFalse("Validate()", "no sensitive volume", "A detector without sensitive volume is invalid", ValidateQuiet(NoSensitive)) && Passed;

  // The volume without any settings gets all defaults
  UTDDetectorStub Defaults;
  MDVolume Single("Single");
  Defaults.AddSensitiveVolume(&Single);
  Passed = EvaluateTrue("Validate()", "one sensitive volume", "One sensitive volume is sufficient", ValidateQuiet(Defaults)) && Passed;
  Passed = EvaluateTrue("Validate()", "one sensitive volume", "The sensitive volume becomes the detector volume", Defaults.GetDetectorVolume() == &Single) && Passed;
  Passed = EvaluateTrue("Validate()", "one sensitive volume", "The sensitive volume becomes the common volume", Defaults.GetCommonVolume() == &Single) && Passed;
  Passed = EvaluateVectorNear("Validate()", "one sensitive volume", "The pitch is zero", Defaults.GetStructuralPitch(), MVector(0.0, 0.0, 0.0), 1e-12) && Passed;
  Passed = EvaluateVectorNear("Validate()", "one sensitive volume", "The offset is zero", Defaults.GetStructuralOffset(), MVector(0.0, 0.0, 0.0), 1e-12) && Passed;
  Passed = EvaluateNear("Validate()", "defaults", "The energy resolution type is ideal", Defaults.GetEnergyResolutionType(), MDDetector::c_EnergyResolutionTypeIdeal, 1e-12) && Passed;
  Passed = Evaluate("Validate()", "defaults", "The energy loss type is none", Defaults.GetEnergyLossType(), MDDetector::c_EnergyLossTypeNone) && Passed;
  Passed = EvaluateFalse("Validate()", "defaults", "There is no time resolution", Defaults.HasTimeResolution()) && Passed;
  Passed = EvaluateFalse("Validate()", "defaults", "The noise threshold is independent of the trigger threshold", Defaults.GetNoiseThresholdEqualsTriggerThreshold()) && Passed;
  Passed = EvaluateNear("Validate()", "defaults", "The noise threshold is zero", Defaults.GetNoiseThreshold(), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("Validate()", "defaults", "The noise threshold sigma is zero", Defaults.GetNoiseThresholdSigma(), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("Validate()", "defaults", "The trigger threshold is zero", Defaults.GetTriggerThreshold(), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("Validate()", "defaults", "The trigger threshold sigma is zero", Defaults.GetTriggerThresholdSigma(), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("Validate()", "defaults", "The failure rate is zero", Defaults.GetFailureRate(), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("Validate()", "defaults", "The overflow is huge", Defaults.GetOverflow(), 10E+20, 1.0) && Passed;
  Passed = EvaluateNear("Validate()", "defaults", "The overflow sigma is one", Defaults.GetOverflowSigma(), 1.0, 1e-12) && Passed;

  // A threshold without sigma gets the sigma zero
  UTDDetectorStub NoSigma;
  MDVolume NoSigmaVolume("NoSigmaVolume");
  NoSigma.AddSensitiveVolume(&NoSigmaVolume);
  NoSigma.SetNoiseThreshold(10.0);
  NoSigma.SetTriggerThreshold(20.0);
  Passed = EvaluateTrue("Validate()", "thresholds without sigma", "The detector is valid", ValidateQuiet(NoSigma)) && Passed;
  Passed = EvaluateNear("Validate()", "thresholds without sigma", "The noise threshold stays", NoSigma.GetNoiseThreshold(), 10.0, 1e-12) && Passed;
  Passed = EvaluateNear("Validate()", "thresholds without sigma", "The noise threshold sigma is zero", NoSigma.GetNoiseThresholdSigma(), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("Validate()", "thresholds without sigma", "The trigger threshold stays", NoSigma.GetTriggerThreshold(), 20.0, 1e-12) && Passed;
  Passed = EvaluateNear("Validate()", "thresholds without sigma", "The trigger threshold sigma is zero", NoSigma.GetTriggerThresholdSigma(), 0.0, 1e-12) && Passed;

  // Virtual and cloned volumes
  UTDDetectorStub VirtualDetector;
  MDVolume Virtual("Virtual");
  Virtual.SetVirtual(true);
  VirtualDetector.AddSensitiveVolume(&Virtual);
  VirtualDetector.SetDetectorVolume(&Virtual);
  Passed = EvaluateFalse("Validate()", "virtual detector volume", "A virtual detector volume is invalid", ValidateQuiet(VirtualDetector)) && Passed;

  UTDDetectorStub VirtualSensitive;
  MDVolume Real("Real");
  MDVolume VirtualS("VirtualS");
  VirtualS.SetVirtual(true);
  VirtualSensitive.SetDetectorVolume(&Real);
  VirtualSensitive.AddSensitiveVolume(&VirtualS);
  Passed = EvaluateFalse("Validate()", "virtual sensitive volume", "A virtual sensitive volume is invalid", ValidateQuiet(VirtualSensitive)) && Passed;

  UTDDetectorStub ClonedDetector;
  MDVolume Template("Template");
  MDVolume Clone("Clone");
  Clone.SetCloneTemplate(&Template);
  ClonedDetector.SetDetectorVolume(&Clone);
  ClonedDetector.AddSensitiveVolume(&Clone);
  Passed = EvaluateFalse("Validate()", "cloned detector volume", "A cloned detector volume is invalid", ValidateQuiet(ClonedDetector)) && Passed;

  // Separate detector and sensitive volume: Structure is required
  MDVolume DV("DV");
  MDVolume SV("SV");
  UTDDetectorStub NoPitch;
  NoPitch.SetDetectorVolume(&DV);
  NoPitch.AddSensitiveVolume(&SV);
  NoPitch.SetStructuralOffset(MVector(0.0, 0.0, 0.0));
  Passed = EvaluateFalse("Validate()", "no pitch", "Without a structural pitch the detector is invalid", ValidateQuiet(NoPitch)) && Passed;

  UTDDetectorStub NoOffset;
  NoOffset.SetDetectorVolume(&DV);
  NoOffset.AddSensitiveVolume(&SV);
  NoOffset.SetStructuralPitch(MVector(0.0, 0.0, 0.0));
  Passed = EvaluateFalse("Validate()", "no offset", "Without a structural offset the detector is invalid", ValidateQuiet(NoOffset)) && Passed;

  vector<MVector> Negatives = { MVector(-0.1, 0.0, 0.0), MVector(0.0, -0.1, 0.0), MVector(0.0, 0.0, -0.1) };
  for (const MVector& Negative: Negatives) {
    UTDDetectorStub NegativePitch;
    NegativePitch.SetDetectorVolume(&DV);
    NegativePitch.AddSensitiveVolume(&SV);
    NegativePitch.SetStructuralPitch(Negative);
    NegativePitch.SetStructuralOffset(MVector(0.0, 0.0, 0.0));
    Passed = EvaluateFalse("Validate()", "negative pitch", "A negative pitch component is invalid", ValidateQuiet(NegativePitch)) && Passed;

    UTDDetectorStub NegativeOffset;
    NegativeOffset.SetDetectorVolume(&DV);
    NegativeOffset.AddSensitiveVolume(&SV);
    NegativeOffset.SetStructuralPitch(MVector(0.0, 0.0, 0.0));
    NegativeOffset.SetStructuralOffset(Negative);
    Passed = EvaluateFalse("Validate()", "negative offset", "A negative offset component is invalid", ValidateQuiet(NegativeOffset)) && Passed;
  }

  // Energy resolution table size
  for (unsigned int Points = 0; Points <= 2; ++Points) {
    UTDDetectorStub Table;
    MDVolume TableVolume("TableVolume");
    Table.AddSensitiveVolume(&TableVolume);
    Table.SetEnergyResolutionType(MDDetector::c_EnergyResolutionTypeGauss);
    for (unsigned int p = 0; p < Points; ++p) {
      Table.SetEnergyResolution(100.0 + p, 100.0 + p, 1.0);
    }
    if (Points < 2) {
      Passed = EvaluateFalse("Validate()", Points, "Less than two points of the energy resolution are invalid", ValidateQuiet(Table)) && Passed;
    } else {
      Passed = EvaluateTrue("Validate()", Points, "Two points of the energy resolution are valid", ValidateQuiet(Table)) && Passed;
    }
  }

  // Equal thresholds
  UTDDetectorStub Equal;
  MDVolume EqualVolume("EqualVolume");
  Equal.AddSensitiveVolume(&EqualVolume);
  Equal.SetNoiseThresholdEqualsTriggerThreshold(true);
  Equal.SetNoiseThreshold(10.0);
  Equal.SetNoiseThresholdSigma(1.0);
  Equal.SetTriggerThreshold(20.0);
  Passed = EvaluateTrue("Validate()", "equal thresholds with a noise threshold", "The noise threshold is ignored", ValidateQuiet(Equal)) && Passed;
  Passed = EvaluateNear("Validate()", "equal thresholds with a noise threshold", "The noise threshold is zero", Equal.GetNoiseThreshold(), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("Validate()", "equal thresholds with a noise threshold", "The noise threshold sigma is zero", Equal.GetNoiseThresholdSigma(), 0.0, 1e-12) && Passed;

  UTDDetectorStub EqualSigma;
  MDVolume EqualSigmaVolume("EqualSigmaVolume");
  EqualSigma.AddSensitiveVolume(&EqualSigmaVolume);
  EqualSigma.SetNoiseThresholdEqualsTriggerThreshold(true);
  EqualSigma.SetNoiseThreshold(0.0);
  EqualSigma.SetNoiseThresholdSigma(1.0);
  Passed = EvaluateFalse("Validate()", "equal thresholds with only a noise sigma", "A sigma of the ignored threshold is an error", ValidateQuiet(EqualSigma)) && Passed;

  // Named detector needs an assignment
  UTDDetectorStub Base("Base");
  UTDDetectorStub Named("Named");
  MDVolume NamedVolume("NamedVolume");
  Named.SetDetectorVolume(&NamedVolume);
  Named.AddSensitiveVolume(&NamedVolume);
  Base.AddNamedDetector(&Named);
  Passed = EvaluateFalse("Validate()", "named detector without assignment", "A named detector needs a volume sequence", ValidateQuiet(Named)) && Passed;
  MDVolumeSequence Sequence;
  Sequence.AddVolume(&NamedVolume);
  Named.SetVolumeSequence(Sequence);
  Passed = EvaluateTrue("Validate()", "named detector with assignment", "A named detector with a volume sequence is valid", ValidateQuiet(Named)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the copy constructor
bool UTDDetector::TestCopy()
{
  bool Passed = true;

  UTDDetectorStub Stub("Original");
  MDVolume V1("V1");
  MDVolume V2("V2");
  Stub.AddSensitiveVolume(&V1);
  Stub.SetDetectorVolume(&V2);
  Stub.SetCommonVolume(&V2);
  Stub.UseDivisions("X", "Y", "Z");
  Stub.SetNoiseThreshold(1.0);
  Stub.SetNoiseThresholdSigma(2.0);
  Stub.SetTriggerThreshold(3.0);
  Stub.SetTriggerThresholdSigma(4.0);
  Stub.SetOverflow(5.0);
  Stub.SetOverflowSigma(6.0);
  Stub.SetFailureRate(0.5);
  Stub.SetStructuralOffset(MVector(7.0, 8.0, 9.0));
  Stub.SetStructuralPitch(MVector(10.0, 11.0, 12.0));
  Stub.SetEnergyResolutionType(MDDetector::c_EnergyResolutionTypeGauss);
  Stub.SetEnergyResolution(100.0, 100.0, 1.0);
  Stub.SetEnergyResolution(200.0, 200.0, 3.0);
  Stub.SetTimeResolution(100.0, 5.0);
  Stub.SetPulseShape(1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 2.0);
  Stub.ActivateNoising(false);
  Stub.SetNoiseThresholdEqualsTriggerThreshold(true);
  Stub.BlockTriggerChannel(0, 0);
  MFunction Calibration;
  Calibration.Add(0.0, 0.0);
  Calibration.Add(1.0, 3.0);
  Stub.SetEnergyCalibration(Calibration);

  UTDDetectorStub Copy(Stub);
  Passed = EvaluateTrue("MDDetector(const MDDetector&)", "name", "The name is copied", Copy.GetName() == "Original") && Passed;
  Passed = Evaluate("MDDetector(const MDDetector&)", "type", "The type is copied", Copy.GetType(), Stub.GetType()) && Passed;
  Passed = EvaluateTrue("MDDetector(const MDDetector&)", "detector volume", "The detector volume is copied", Copy.GetDetectorVolume() == &V2) && Passed;
  Passed = EvaluateTrue("MDDetector(const MDDetector&)", "common volume", "The common volume is copied", Copy.GetCommonVolume() == &V2) && Passed;
  Passed = EvaluateTrue("MDDetector(const MDDetector&)", "divisions", "The divisions are copied", Copy.UseDivisions() && Copy.GetShortNameDivisionZ() == "Z") && Passed;
  Passed = EvaluateNear("MDDetector(const MDDetector&)", "noise threshold", "The value is copied", Copy.GetNoiseThreshold(), 1.0, 1e-12) && Passed;
  Passed = EvaluateNear("MDDetector(const MDDetector&)", "noise threshold sigma", "The value is copied", Copy.GetNoiseThresholdSigma(), 2.0, 1e-12) && Passed;
  Passed = EvaluateNear("MDDetector(const MDDetector&)", "trigger threshold", "The value is copied", Copy.GetTriggerThreshold(), 3.0, 1e-12) && Passed;
  Passed = EvaluateNear("MDDetector(const MDDetector&)", "trigger threshold sigma", "The value is copied", Copy.GetTriggerThresholdSigma(), 4.0, 1e-12) && Passed;
  Passed = EvaluateNear("MDDetector(const MDDetector&)", "overflow", "The value is copied", Copy.GetOverflow(), 5.0, 1e-12) && Passed;
  Passed = EvaluateNear("MDDetector(const MDDetector&)", "overflow sigma", "The value is copied", Copy.GetOverflowSigma(), 6.0, 1e-12) && Passed;
  Passed = EvaluateNear("MDDetector(const MDDetector&)", "failure rate", "The value is copied", Copy.GetFailureRate(), 0.5, 1e-12) && Passed;
  Passed = EvaluateVectorNear("MDDetector(const MDDetector&)", "offset", "The value is copied", Copy.GetStructuralOffset(), MVector(7.0, 8.0, 9.0), 1e-12) && Passed;
  Passed = EvaluateVectorNear("MDDetector(const MDDetector&)", "pitch", "The value is copied", Copy.GetStructuralPitch(), MVector(10.0, 11.0, 12.0), 1e-12) && Passed;
  Passed = EvaluateNear("MDDetector(const MDDetector&)", "energy resolution", "The table is copied", Copy.GetEnergyResolutionWidth1(150.0), 2.0, 1e-9) && Passed;
  Passed = EvaluateNear("MDDetector(const MDDetector&)", "time resolution", "The table is copied", Copy.GetTimeResolution(100.0), 5.0, 1e-9) && Passed;
  Passed = EvaluateNear("MDDetector(const MDDetector&)", "pulse shape", "The pulse shape is copied", Copy.ApplyPulseShape(1.0, 7.0), 7.0, 1e-9) && Passed;
  Passed = EvaluateFalse("MDDetector(const MDDetector&)", "noise activation", "The flag is copied", Copy.m_NoiseActive) && Passed;
  Passed = EvaluateTrue("MDDetector(const MDDetector&)", "equal thresholds", "The flag is copied", Copy.GetNoiseThresholdEqualsTriggerThreshold()) && Passed;
  Passed = EvaluateFalse("MDDetector(const MDDetector&)", "blocked channels", "The blocked channels are copied", Copy.IsAboveTriggerThreshold(100.0, MDGridPoint(0, 0, 0))) && Passed;
  double Energy = 2.0;
  Copy.ApplyEnergyCalibration(Energy);
  Passed = EvaluateNear("MDDetector(const MDDetector&)", "calibration", "The calibration is copied", Energy, 6.0, 1e-9) && Passed;

  // Defect GEO-D-008: The copy constructor loops over its own empty list of sensitive volumes and thus does not copy them
  Passed = ExpectedToFail(Evaluate("MDDetector(const MDDetector&)", "sensitive volumes", "The sensitive volumes are copied", Copy.GetNSensitiveVolumes(), 1u)) && Passed;

  // The copy is independent
  Copy.SetNoiseThreshold(100.0);
  Copy.SetEnergyResolution(300.0, 300.0, 5.0);
  Passed = EvaluateNear("MDDetector(const MDDetector&)", "independence", "A change of the copy does not change the original", Stub.GetNoiseThreshold(), 1.0, 1e-12) && Passed;
  Passed = EvaluateNear("MDDetector(const MDDetector&)", "independence", "A change of the table of the copy does not change the original", Stub.GetEnergyResolutionWidth1(300.0), 5.0, 1e-9) && Passed;

  // Different IDs and independent pulse shape: destruction of the copy must not remove the shape of the original
  {
    UTDDetectorStub Short(Stub);
    Passed = EvaluateTrue("MDDetector(const MDDetector&)", "ID", "The copy gets its own ID", Short.m_ID != Stub.m_ID) && Passed;
  }
  Passed = EvaluateNear("~MDDetector()", "copy destroyed", "The pulse shape of the original still works", Stub.ApplyPulseShape(1.0, 7.0), 7.0, 1e-9) && Passed;

  // Clone
  unique_ptr<MDDetector> Cloned(Stub.Clone());
  Passed = EvaluateTrue("Clone()", "stub", "A clone has the same name", Cloned->GetName() == "Original") && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the named detector interface without a geometry
bool UTDDetector::TestNamedDetectorInterface()
{
  bool Passed = true;

  UTDDetectorStub Base("Base");
  UTDDetectorStub N1("N1");
  UTDDetectorStub N2("N2");
  UTDDetectorStub N3("N3");

  Passed = EvaluateTrue("AddNamedDetector()", "first", "The first named detector is added", Base.AddNamedDetector(&N1)) && Passed;
  Passed = EvaluateTrue("AddNamedDetector()", "second", "The second named detector is added", Base.AddNamedDetector(&N2)) && Passed;
  DisableDefaultStreams();
  bool NamedOfNamed = N1.AddNamedDetector(&N3);
  EnableDefaultStreams();
  Passed = EvaluateFalse("AddNamedDetector()", "named detector of a named detector", "A named detector cannot have named detectors", NamedOfNamed) && Passed;
  Passed = EvaluateFalse("AddNamedDetector()", "rejected", "The rejected detector is not named", N3.IsNamedDetector()) && Passed;

  Passed = EvaluateTrue("IsNamedDetector()", "N1", "N1 is a named detector", N1.IsNamedDetector()) && Passed;
  Passed = EvaluateFalse("IsNamedDetector()", "Base", "The base detector is not a named detector", Base.IsNamedDetector()) && Passed;
  Passed = EvaluateTrue("GetNamedAfterDetector()", "N1", "N1 knows its base detector", N1.GetNamedAfterDetector() == &Base) && Passed;
  Passed = EvaluateTrue("GetNamedAfterDetector()", "Base", "The base detector has no base detector", Base.GetNamedAfterDetector() == nullptr) && Passed;
  Passed = EvaluateTrue("HasNamedDetectors()", "Base", "The base detector has named detectors", Base.HasNamedDetectors()) && Passed;
  Passed = EvaluateFalse("HasNamedDetectors()", "N1", "A named detector has no named detectors", N1.HasNamedDetectors()) && Passed;
  Passed = EvaluateTrue("HasNamedDetector()", "N2", "The named detector is found", Base.HasNamedDetector("N2")) && Passed;
  Passed = EvaluateFalse("HasNamedDetector()", "N3", "A not added detector is not found", Base.HasNamedDetector("N3")) && Passed;
  Passed = Evaluate("GetNNamedDetectors()", "two", "Two named detectors are counted", Base.GetNNamedDetectors(), 2u) && Passed;
  Passed = EvaluateTrue("GetNamedDetectorName()", "index 0", "The first name is returned", Base.GetNamedDetectorName(0) == "N1") && Passed;
  Passed = EvaluateTrue("GetNamedDetectorName()", "index n - 1", "The last name is returned", Base.GetNamedDetectorName(1) == "N2") && Passed;

  DisableDefaultStreams();
  MString BeyondEnd = Base.GetNamedDetectorName(3);
  EnableDefaultStreams();
  Passed = EvaluateTrue("GetNamedDetectorName()", "index n + 1", "An index out of range returns an empty name", BeyondEnd == "") && Passed;

  // Defect GEO-D-009: GetNamedDetectorName checks i > size and reads behind the end of the list for i == size
  Passed = ExpectedToCrash(EvaluateTrue("GetNamedDetectorName()", "index n", "An index one past the end returns an empty name and does not read outside of the list", Base.GetNamedDetectorName(2) == "")) && Passed;

  // Defect GEO-D-010: GetNamedDetectorVolumeSequence checks i > size and reads behind the end of the list for i == size
  Passed = ExpectedToCrash(EvaluateTrue("GetNamedDetectorVolumeSequence()", "index n", "An index one past the end returns an empty sequence and does not read outside of the list", Base.GetNamedDetectorVolumeSequence(2).IsEmpty())) && Passed;

  MDVolume V("V");
  MDVolumeSequence Sequence;
  Sequence.AddVolume(&V);
  N1.SetVolumeSequence(Sequence);
  Passed = EvaluateFalse("GetNamedDetectorVolumeSequence()", "index 0", "The sequence of the first named detector is returned", Base.GetNamedDetectorVolumeSequence(0).IsEmpty()) && Passed;
  Passed = EvaluateTrue("GetNamedDetectorVolumeSequence()", "index n - 1", "The sequence of the last named detector is returned", Base.GetNamedDetectorVolumeSequence(1).IsEmpty()) && Passed;

  // Empty base detector
  UTDDetectorStub Empty("Empty");
  Passed = EvaluateTrue("FindNamedDetector()", "no named detectors", "Nothing is found", Empty.FindNamedDetector(Sequence) == nullptr) && Passed;
  DisableDefaultStreams();
  MVector NotFound = Empty.GetGlobalPosition(MVector(0.0, 0.0, 0.0), "N1");
  EnableDefaultStreams();
  Passed = EvaluateTrue("GetGlobalPosition()", "unknown named detector", "An unknown named detector returns g_VectorNotDefined", NotFound == g_VectorNotDefined) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test copying the data to named detectors
bool UTDDetector::TestCopyDataToNamedDetectors()
{
  bool Passed = true;

  UTDDetectorStub Base("Base");
  UTDDetectorStub Inherit("Inherit");
  UTDDetectorStub Own("Own");
  Base.AddNamedDetector(&Inherit);
  Base.AddNamedDetector(&Own);

  MDVolume SV("SV");
  MDVolume DV("DV");
  Base.AddSensitiveVolume(&SV);
  Base.SetDetectorVolume(&DV);
  Base.SetStructuralOffset(MVector(1.0, 2.0, 3.0));
  Base.SetStructuralPitch(MVector(4.0, 5.0, 6.0));
  Base.SetEnergyResolutionType(MDDetector::c_EnergyResolutionTypeGauss);
  Base.SetEnergyResolution(100.0, 100.0, 1.0);
  Base.SetEnergyResolution(200.0, 200.0, 3.0);
  Base.SetTimeResolution(100.0, 5.0);
  Base.SetFailureRate(0.5);
  Base.SetNoiseThreshold(10.0);
  Base.SetNoiseThresholdSigma(1.0);
  Base.SetTriggerThreshold(20.0);
  Base.SetTriggerThresholdSigma(2.0);
  Base.SetOverflow(1000.0);
  Base.SetOverflowSigma(10.0);
  Base.UseDivisions("X", "Y", "Z");
  Base.SetPulseShape(1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 2.0);
  MFunction Calibration;
  Calibration.Add(0.0, 0.0);
  Calibration.Add(1.0, 3.0);
  Base.SetEnergyCalibration(Calibration);

  // Own settings of the named detector:
  Own.SetEnergyResolutionType(MDDetector::c_EnergyResolutionTypeGauss);
  Own.SetEnergyResolution(100.0, 100.0, 7.0);
  Own.SetEnergyResolution(200.0, 200.0, 7.0);
  Own.SetTimeResolution(100.0, 8.0);
  Own.SetFailureRate(0.25);
  Own.SetNoiseThreshold(11.0);
  Own.SetNoiseThresholdSigma(1.5);
  Own.SetTriggerThreshold(21.0);
  Own.SetTriggerThresholdSigma(2.5);
  Own.SetOverflow(1001.0);
  Own.SetOverflowSigma(11.0);
  MFunction OwnCalibration;
  OwnCalibration.Add(0.0, 0.0);
  OwnCalibration.Add(1.0, 5.0);
  Own.SetEnergyCalibration(OwnCalibration);

  Passed = EvaluateTrue("CopyDataToNamedDetectors()", "base", "Copying is successful", Base.CopyDataToNamedDetectors()) && Passed;

  // Inherited
  Passed = Evaluate("CopyDataToNamedDetectors()", "inherited sensitive volumes", "The sensitive volumes are inherited", Inherit.GetNSensitiveVolumes(), 1u) && Passed;
  Passed = EvaluateTrue("CopyDataToNamedDetectors()", "inherited detector volume", "The detector volume is inherited", Inherit.GetDetectorVolume() == &DV) && Passed;
  Passed = EvaluateVectorNear("CopyDataToNamedDetectors()", "inherited offset", "The offset is inherited", Inherit.GetStructuralOffset(), MVector(1.0, 2.0, 3.0), 1e-12) && Passed;
  Passed = EvaluateVectorNear("CopyDataToNamedDetectors()", "inherited pitch", "The pitch is inherited", Inherit.GetStructuralPitch(), MVector(4.0, 5.0, 6.0), 1e-12) && Passed;
  Passed = EvaluateNear("CopyDataToNamedDetectors()", "inherited energy resolution", "The table is inherited", Inherit.GetEnergyResolutionWidth1(150.0), 2.0, 1e-9) && Passed;
  Passed = EvaluateNear("CopyDataToNamedDetectors()", "inherited time resolution", "The table is inherited", Inherit.GetTimeResolution(100.0), 5.0, 1e-9) && Passed;
  Passed = EvaluateNear("CopyDataToNamedDetectors()", "inherited failure rate", "The value is inherited", Inherit.GetFailureRate(), 0.5, 1e-12) && Passed;
  Passed = EvaluateNear("CopyDataToNamedDetectors()", "inherited noise threshold", "The value is inherited", Inherit.GetNoiseThreshold(), 10.0, 1e-12) && Passed;
  Passed = EvaluateNear("CopyDataToNamedDetectors()", "inherited noise threshold sigma", "The value is inherited", Inherit.GetNoiseThresholdSigma(), 1.0, 1e-12) && Passed;
  Passed = EvaluateNear("CopyDataToNamedDetectors()", "inherited trigger threshold", "The value is inherited", Inherit.GetTriggerThreshold(), 20.0, 1e-12) && Passed;
  Passed = EvaluateNear("CopyDataToNamedDetectors()", "inherited trigger threshold sigma", "The value is inherited", Inherit.GetTriggerThresholdSigma(), 2.0, 1e-12) && Passed;
  Passed = EvaluateNear("CopyDataToNamedDetectors()", "inherited overflow", "The value is inherited", Inherit.GetOverflow(), 1000.0, 1e-12) && Passed;
  Passed = EvaluateNear("CopyDataToNamedDetectors()", "inherited overflow sigma", "The value is inherited", Inherit.GetOverflowSigma(), 10.0, 1e-12) && Passed;
  Passed = EvaluateTrue("CopyDataToNamedDetectors()", "inherited divisions", "The divisions are inherited", Inherit.UseDivisions()) && Passed;
  Passed = EvaluateNear("CopyDataToNamedDetectors()", "inherited pulse shape", "The pulse shape is inherited", Inherit.ApplyPulseShape(1.0, 7.0), 7.0, 1e-9) && Passed;
  double Energy = 2.0;
  Inherit.ApplyEnergyCalibration(Energy);
  Passed = EvaluateNear("CopyDataToNamedDetectors()", "inherited calibration", "The calibration is inherited", Energy, 6.0, 1e-9) && Passed;

  // Own settings stay
  Passed = EvaluateNear("CopyDataToNamedDetectors()", "own energy resolution", "The own table stays", Own.GetEnergyResolutionWidth1(150.0), 7.0, 1e-9) && Passed;
  Passed = EvaluateNear("CopyDataToNamedDetectors()", "own time resolution", "The own table stays", Own.GetTimeResolution(100.0), 8.0, 1e-9) && Passed;
  Passed = EvaluateNear("CopyDataToNamedDetectors()", "own failure rate", "The own value stays", Own.GetFailureRate(), 0.25, 1e-12) && Passed;
  Passed = EvaluateNear("CopyDataToNamedDetectors()", "own noise threshold", "The own value stays", Own.GetNoiseThreshold(), 11.0, 1e-12) && Passed;
  Passed = EvaluateNear("CopyDataToNamedDetectors()", "own noise threshold sigma", "The own value stays", Own.GetNoiseThresholdSigma(), 1.5, 1e-12) && Passed;
  Passed = EvaluateNear("CopyDataToNamedDetectors()", "own trigger threshold", "The own value stays", Own.GetTriggerThreshold(), 21.0, 1e-12) && Passed;
  Passed = EvaluateNear("CopyDataToNamedDetectors()", "own trigger threshold sigma", "The own value stays", Own.GetTriggerThresholdSigma(), 2.5, 1e-12) && Passed;
  Passed = EvaluateNear("CopyDataToNamedDetectors()", "own overflow", "The own value stays", Own.GetOverflow(), 1001.0, 1e-12) && Passed;
  Passed = EvaluateNear("CopyDataToNamedDetectors()", "own overflow sigma", "The own value stays", Own.GetOverflowSigma(), 11.0, 1e-12) && Passed;
  Energy = 2.0;
  Own.ApplyEnergyCalibration(Energy);
  Passed = EvaluateNear("CopyDataToNamedDetectors()", "own calibration", "The own calibration stays", Energy, 10.0, 1e-9) && Passed;

  // The call on a named detector does nothing
  Passed = EvaluateTrue("CopyDataToNamedDetectors()", "named detector", "The call on a named detector is successful", Inherit.CopyDataToNamedDetectors()) && Passed;
  Passed = Evaluate("CopyDataToNamedDetectors()", "named detector", "The call on a named detector does not change anything", Inherit.GetNSensitiveVolumes(), 1u) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the keyword parsing of the settings common to all detectors
bool UTDDetector::TestKeywords()
{
  bool Passed = true;

  const MString Body = GetBoxVolume("Box", 1.0, 1.0, 1.0) + "Scintillator D\nD.SensitiveVolume Box\nD.DetectorVolume Box\n";

  Passed = EvaluateTrue("ScanSetupFile()", "minimal detector", "A minimal detector is accepted", IsScanOK(Body)) && Passed;

  // The name of a detector
  Passed = EvaluateFalse("ScanSetupFile()", "detector without name", "A detector type without a name is rejected", IsScanOK(GetBoxVolume("Box", 1.0, 1.0, 1.0) + "Scintillator\n")) && Passed;
  Passed = EvaluateFalse("ScanSetupFile()", "detector with a used name", "A detector with the name of a volume is rejected", IsScanOK(GetBoxVolume("Box", 1.0, 1.0, 1.0) + "Scintillator Box\n")) && Passed;

  // Volumes
  Passed = EvaluateFalse("ScanSetupFile()", "unknown sensitive volume", "A sensitive volume which does not exist is rejected", IsScanOK(GetBoxVolume("Box", 1.0, 1.0, 1.0), "Scintillator D\nD.SensitiveVolume Other\n")) && Passed;
  Passed = EvaluateFalse("ScanSetupFile()", "unknown detector volume", "A detector volume which does not exist is rejected", IsScanOK(GetBoxVolume("Box", 1.0, 1.0, 1.0), "Scintillator D\nD.SensitiveVolume Box\nD.DetectorVolume Other\n")) && Passed;
  Passed = EvaluateFalse("ScanSetupFile()", "no volumes", "A detector without volumes is rejected", IsScanOK(GetBoxVolume("Box", 1.0, 1.0, 1.0), "Scintillator D\n")) && Passed;

  // Token numbers
  vector<MString> Wrong = {
    "D.UnknownKeyword 1\n",
    "D.NoiseThresholdEqualsTriggerThreshold\n",
    "D.NoiseThresholdEqualsTriggerThreshold 1 2\n",
    "D.NoiseThreshold\n",
    "D.NoiseThreshold 1 2 3\n",
    "D.TriggerThreshold\n",
    "D.TriggerThreshold 1 2 3\n",
    "D.FailureRate\n",
    "D.FailureRate 1 2\n",
    "D.Overflow\n",
    "D.Overflow 1 2 3\n",
    "D.EnergyLossMap\n",
    "D.EnergyLossMap DoesNotExist.dat\n",
    "D.EnergyResolution\n",
    "D.EnergyResolution Gauss 1 2\n",
    "D.EnergyResolution Gauss 1 2 3 4\n",
    "D.EnergyResolution Lorentz 1 2\n",
    "D.EnergyResolution GaussLandau 1 2 3\n",
    "D.EnergyResolution Unknown 1 2 3\n",
    "D.EnergyResolutionType\n",
    "D.EnergyResolutionType Unknown\n",
    "D.EnergyCalibration\n",
    "D.EnergyCalibration DoesNotExist.dat\n",
    "D.TimeResolution 1\n",
    "D.TimeResolution 1 2 3\n",
    "D.PulseShape 1 2 3\n",
    "D.StructuralOffset 1 2\n",
    "D.StructuralOffset 1 2 3 4\n",
    "D.StructuralPitch 1 2\n",
    "D.StructuralPitch 1 2 3 4\n",
    "D.BlockTrigger 1\n",
    "D.Assign 1 2\n"
  };
  for (const MString& Keyword: Wrong) {
    Passed = EvaluateFalse("ScanSetupFile()", Keyword, "The wrong keyword is rejected without a crash", IsScanOK(Body, Keyword)) && Passed;
  }

  // Defect GEO-D-022: The check of the token number of the Named keyword is never reached with too few tokens, thus a line without a name is silently ignored
  Passed = ExpectedToFail(EvaluateFalse("ScanSetupFile()", "D.Named", "A named detector without a name is rejected", IsScanOK(Body, "D.Named\n"))) && Passed;

  // Keywords for other detector types
  vector<MString> OtherTypes = {
    "D.Offset 0.1 0.1\n",
    "D.StripNumber 2 2\n",
    "D.Voxels 2 2 2\n",
    "D.GuardRingTriggerThreshold 1 2\n",
    "D.GuardRingEnergyResolution 1 1 2\n",
    "D.DepthResolution 100 1\n",
    "D.DepthResolutionThreshold 100\n",
    "D.EnergyResolutionDepthCorrection 0 1\n",
    "D.TriggerThresholdDepthCorrection 0 1\n",
    "D.NoiseThresholdDepthCorrection 0 1\n",
    "D.DirectionalResolution 100 1\n",
    "D.PositionResolution 100 1\n",
    "D.Positioning XY\n",
    "D.LightSpeed 1\n",
    "D.LightDetectorPosition 1\n",
    "D.DriftConstant 1\n",
    "D.EnergyPerElectron 1\n",
    "D.LightEnergyResolution 1 1\n"
  };
  for (const MString& Keyword: OtherTypes) {
    Passed = EvaluateFalse("ScanSetupFile()", Keyword, "A keyword of another detector type is rejected", IsScanOK(Body, Keyword)) && Passed;
  }

  // Accepted forms: Depreciated form of the energy resolution with a Gauss type
  MDGeometry Deprecated;
  Passed = EvaluateTrue("ScanSetupFile()", "deprecated energy resolution", "The old format of the energy resolution is accepted", Scan(Deprecated, Body, "D.EnergyResolutionAt 100 2\nD.EnergyResolutionAt 500 6\n")) && Passed;
  if (CheckDetector(Passed, "Deprecated", Deprecated, "D", MDDetector::c_Scintillator)) {
    Passed = EvaluateNear("ScanSetupFile()", "deprecated energy resolution", "The type of the old format is Gauss", Deprecated.GetDetector("D")->GetEnergyResolutionType(), MDDetector::c_EnergyResolutionTypeGauss, 1e-12) && Passed;
    Passed = EvaluateNear("ScanSetupFile()", "deprecated energy resolution", "The width is read", Deprecated.GetDetector("D")->GetEnergyResolutionWidth1(300.0), 4.0, 1e-9) && Passed;
    Passed = EvaluateNear("ScanSetupFile()", "deprecated energy resolution", "The peak is identical to the input energy", Deprecated.GetDetector("D")->GetEnergyResolutionPeak1(300.0), 300.0, 1e-9) && Passed;
  }
  MDGeometry DeprecatedShift;
  Passed = EvaluateTrue("ScanSetupFile()", "deprecated energy resolution with shift", "The old format with a shift is accepted", Scan(DeprecatedShift, Body, "D.EnergyResolutionAt 100 2 0 5\nD.EnergyResolutionAt 500 6 0 5\n")) && Passed;
  if (CheckDetector(Passed, "DeprecatedShift", DeprecatedShift, "D", MDDetector::c_Scintillator)) {
    Passed = EvaluateNear("ScanSetupFile()", "deprecated energy resolution with shift", "The peak is shifted by the last value", DeprecatedShift.GetDetector("D")->GetEnergyResolutionPeak1(300.0), 305.0, 1e-9) && Passed;
  }

  // Mixed types
  Passed = EvaluateFalse("ScanSetupFile()", "mixed energy resolution types", "Mixing Gauss and Lorentz is rejected", IsScanOK(Body, "D.EnergyResolution Gauss 100 100 1\nD.EnergyResolution Lorentz 200 200 1\n")) && Passed;
  Passed = EvaluateFalse("ScanSetupFile()", "one point", "One point of the energy resolution is rejected", IsScanOK(Body, "D.EnergyResolution Gauss 100 100 1\n")) && Passed;

  // The noise threshold with only the threshold value
  MDGeometry Thresholds;
  Passed = EvaluateTrue("ScanSetupFile()", "thresholds", "The thresholds are accepted", Scan(Thresholds, Body, "D.NoiseThreshold 11 1\nD.TriggerThreshold 22 2\nD.Overflow 33 3\nD.FailureRate 0.5\n")) && Passed;
  if (CheckDetector(Passed, "Thresholds", Thresholds, "D", MDDetector::c_Scintillator)) {
    MDDetector* D = Thresholds.GetDetector("D");
    Passed = EvaluateNear("ScanSetupFile()", "NoiseThreshold", "The value is read", D->GetNoiseThreshold(), 11.0, 1e-12) && Passed;
    Passed = EvaluateNear("ScanSetupFile()", "NoiseThreshold", "The sigma is read", D->GetNoiseThresholdSigma(), 1.0, 1e-12) && Passed;
    Passed = EvaluateNear("ScanSetupFile()", "TriggerThreshold", "The value is read", D->GetTriggerThreshold(), 22.0, 1e-12) && Passed;
    Passed = EvaluateNear("ScanSetupFile()", "TriggerThreshold", "The sigma is read", D->GetTriggerThresholdSigma(), 2.0, 1e-12) && Passed;
    Passed = EvaluateNear("ScanSetupFile()", "Overflow", "The value is read", D->GetOverflow(), 33.0, 1e-12) && Passed;
    Passed = EvaluateNear("ScanSetupFile()", "Overflow", "The sigma is read", D->GetOverflowSigma(), 3.0, 1e-12) && Passed;
    Passed = EvaluateNear("ScanSetupFile()", "FailureRate", "The value is read", D->GetFailureRate(), 0.5, 1e-12) && Passed;
  }

  // Defect GEO-D-011: The range of the failure rate is not validated - a rate of 1.5 is accepted without a message
  Passed = ExpectedToFail(EvaluateFalse("ScanSetupFile()", "failure rate 1.5", "A failure rate above one is rejected", IsScanOK(Body, "D.FailureRate 1.5\n"))) && Passed;
  // Defect GEO-D-012: A negative sigma of the noise threshold is accepted without a message
  Passed = ExpectedToFail(EvaluateFalse("ScanSetupFile()", "noise threshold sigma -1", "A negative sigma is rejected", IsScanOK(Body, "D.NoiseThreshold 10 -1\n"))) && Passed;
  // Defect GEO-D-013: A negative time resolution is accepted without a message
  Passed = ExpectedToFail(EvaluateFalse("ScanSetupFile()", "time resolution -1", "A negative time resolution is rejected", IsScanOK(Body, "D.TimeResolution 100 -1\n"))) && Passed;
  // Defect GEO-D-014: An invalid table line of the energy resolution is ignored by the scan and the geometry stays valid with the remaining points
  Passed = ExpectedToFail(EvaluateFalse("ScanSetupFile()", "negative width in the energy resolution", "A negative width in the table is rejected", IsScanOK(Body, "D.EnergyResolution Gauss 100 100 1\nD.EnergyResolution Gauss 200 200 1\nD.EnergyResolution Gauss 300 300 -1\n"))) && Passed;
  // Defect GEO-D-015: A channel of BlockTrigger which does not exist is reported only on the screen and the geometry stays valid
  Passed = ExpectedToFail(EvaluateFalse("ScanSetupFile()", "BlockTrigger 5 5", "A channel which does not exist is rejected", IsScanOK(Body, "D.BlockTrigger 5 5\n"))) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the interplay of several detectors and named detectors in a geometry
bool UTDDetector::TestDetectorsInGeometry()
{
  bool Passed = true;
  MString Flags;
  gRandom->SetSeed(2004);

  // Two independent detectors with their own settings
  MString Body;
  Body += "Volume A\nA.Material Silicon\nA.Shape BRIK 1 1 1\nA.Position 0 0 -5\nA.Mother World\n";
  Body += "Volume B\nB.Material Silicon\nB.Shape BRIK 1 1 1\nB.Position 0 0 5\nB.Mother World\n";
  Body += "Scintillator DA\nDA.SensitiveVolume A\nDA.DetectorVolume A\nDA.NoiseThreshold 10\nDA.TriggerThreshold 15\n";
  Body += "Scintillator DB\nDB.SensitiveVolume B\nDB.DetectorVolume B\nDB.NoiseThreshold 100\nDB.TriggerThreshold 150\nDB.EnergyResolution Gauss 100 100 1\nDB.EnergyResolution Gauss 500 500 1\n";

  MDGeometryQuest Two;
  Passed = EvaluateTrue("ScanSetupFile()", "two detectors", "The geometry is accepted", Scan(Two, Body)) && Passed;
  if (Passed == false) {
    return false;
  }

  MDVolumeSequence SA = Two.GetVolumeSequence(MVector(0.0, 0.0, -5.0), true, true);
  MDVolumeSequence SB = Two.GetVolumeSequence(MVector(0.0, 0.0, 5.0), true, true);
  Passed = EvaluateTrue("GetVolumeSequence()", "position in A", "The position in the first volume belongs to the first detector", SA.GetDetector() == Two.GetDetector("DA")) && Passed;
  Passed = EvaluateTrue("GetVolumeSequence()", "position in B", "The position in the second volume belongs to the second detector", SB.GetDetector() == Two.GetDetector("DB")) && Passed;

  vector<double> InA;
  vector<double> InB;
  for (unsigned int i = 0; i < 1000; ++i) {
    MVector Pos(0.0, 0.0, -5.0);
    double E = 50.0;
    double T = 0.0;
    Two.Noise(Pos, E, T, Flags, SA);
    InA.push_back(E);
    Pos = MVector(0.0, 0.0, 5.0);
    E = 50.0;
    T = 0.0;
    Two.Noise(Pos, E, T, Flags, SB);
    InB.push_back(E);
  }
  Passed = EvaluateNear("MDGeometryQuest::Noise()", "first detector", "The first detector keeps the 50 keV hit exactly", MStatistics::GetMean(InA), 50.0, 1e-9) && Passed;
  Passed = EvaluateNear("MDGeometryQuest::Noise()", "second detector", "The second detector removes the 50 keV hit", MStatistics::GetMean(InB), 0.0, 1e-12) && Passed;

  // Noise without a detector
  MDGeometryQuest NoDetector;
  MString BodyNoDetector = GetBoxVolume("Passive", 1.0, 1.0, 1.0);
  Passed = EvaluateTrue("ScanSetupFile()", "no detector", "The geometry is accepted", Scan(NoDetector, BodyNoDetector)) && Passed;
  MVector Pos(0.5, 0.5, 0.5);
  double E = 50.0;
  double T = 0.0;
  ostringstream Swallow;
  streambuf* Original = cout.rdbuf(Swallow.rdbuf());
  DisableDefaultStreams();
  bool Result = NoDetector.Noise(Pos, E, T, Flags);
  EnableDefaultStreams();
  cout.rdbuf(Original);
  Passed = EvaluateFalse("MDGeometryQuest::Noise()", "not in a detector", "Noising outside a detector is an error", Result) && Passed;
  Passed = EvaluateNear("MDGeometryQuest::Noise()", "not in a detector", "The energy is zero", E, 0.0, 1e-12) && Passed;

  // Noise before the scan:
  MDGeometryQuest NotScanned;
  MDVolumeSequence EmptySequence;
  DisableDefaultStreams();
  Result = NotScanned.Noise(Pos, E, T, Flags, EmptySequence);
  EnableDefaultStreams();
  Passed = EvaluateFalse("MDGeometryQuest::Noise()", "not scanned", "Noising before the scan is an error", Result) && Passed;

  // Named detectors of a scintillator with their own settings and the Assign keyword
  MString Named = Body;
  Named += "Volume C\nC.Material Silicon\nC.Shape BRIK 1 1 1\nC.Position 0 5 0\nC.Mother World\n";
  Named += "Scintillator Base\nBase.SensitiveVolume C\nBase.DetectorVolume C\nBase.NoiseThreshold 20\n";
  Named += "Base.Named N1\nN1.Assign World.C\nN1.NoiseThreshold 60\n";

  MDGeometryQuest GeometryNamed;
  Passed = EvaluateTrue("ScanSetupFile()", "named scintillator", "The geometry is accepted", Scan(GeometryNamed, Named)) && Passed;
  if (CheckDetector(Passed, "GeometryNamed", GeometryNamed, "N1", MDDetector::c_Scintillator)) {
    MDDetector* N1 = GeometryNamed.GetDetector("N1");
    MDDetector* B = GeometryNamed.GetDetector("Base");
    Passed = EvaluateTrue("IsNamedDetector()", "N1", "N1 is a named detector", N1->IsNamedDetector()) && Passed;
    Passed = EvaluateTrue("GetNamedAfterDetector()", "N1", "N1 is named after the base", N1->GetNamedAfterDetector() == B) && Passed;
    Passed = EvaluateNear("GetNoiseThreshold()", "N1", "The own threshold is used", N1->GetNoiseThreshold(), 60.0, 1e-12) && Passed;
    Passed = EvaluateNear("GetNoiseThreshold()", "Base", "The base keeps its threshold", B->GetNoiseThreshold(), 20.0, 1e-12) && Passed;

    MDVolumeSequence SN = GeometryNamed.GetVolumeSequence(MVector(0.0, 5.0, 0.0), true, true);
    Passed = EvaluateTrue("MDVolumeSequence::GetDetector()", "named volume", "The sequence points to the named detector", SN.GetDetector() == N1) && Passed;
    // Defect GEO-D-021: FindNamedDetector compares detector names, so a sequence which already points to the named detector does not match the stored sequence which points to the base
    Passed = ExpectedToFail(EvaluateTrue("FindNamedDetector()", "sequence", "The named detector is found by a sequence of that detector", B->FindNamedDetector(SN) == N1)) && Passed;
    Pos = MVector(0.0, 5.0, 0.0);
    E = 50.0;
    T = 0.0;
    GeometryNamed.Noise(Pos, E, T, Flags, SN);
    Passed = EvaluateNear("MDGeometryQuest::Noise()", "named detector", "The threshold of the named detector removes the 50 keV hit", E, 0.0, 1e-12) && Passed;

    MVector Global = B->GetGlobalPosition(MVector(0.25, 0.5, 0.75), "N1");
    Passed = EvaluateVectorNear("GetGlobalPosition()", "from the base", "The position is shifted by the volume position", Global, MVector(0.25, 5.5, 0.75), 1e-9) && Passed;
    Global = N1->GetGlobalPosition(MVector(0.25, 0.5, 0.75), "N1");
    Passed = EvaluateVectorNear("GetGlobalPosition()", "from the named detector", "The position is shifted by the volume position", Global, MVector(0.25, 5.5, 0.75), 1e-9) && Passed;
    DisableDefaultStreams();
    Global = B->GetGlobalPosition(MVector(0.25, 0.5, 0.75), "N2");
    EnableDefaultStreams();
    Passed = EvaluateTrue("GetGlobalPosition()", "unknown name", "An unknown name returns g_VectorNotDefined", Global == g_VectorNotDefined) && Passed;
  }

  // Assign errors
  Passed = EvaluateFalse("ScanSetupFile()", "Assign on a base detector", "The Assign keyword is only for named detectors", IsScanOK(Named, "Base.Assign World.C\n")) && Passed;
  MString NoAssign = GetBoxVolume("C", 1.0, 1.0, 1.0) + "Scintillator Base\nBase.SensitiveVolume C\nBase.DetectorVolume C\nBase.Named N1\n";
  Passed = EvaluateFalse("ScanSetupFile()", "named detector without Assign", "A named detector needs the Assign keyword", IsScanOK(NoAssign)) && Passed;
  Passed = EvaluateFalse("ScanSetupFile()", "Assign to an unknown volume", "An unknown volume in the sequence is rejected", IsScanOK(NoAssign, "N1.Assign World.X\n")) && Passed;
  Passed = EvaluateFalse("ScanSetupFile()", "Assign without the world", "The sequence must start with the world", IsScanOK(NoAssign, "N1.Assign C\n")) && Passed;
  Passed = EvaluateFalse("ScanSetupFile()", "Assign to a position outside", "A position outside of a detector is rejected", IsScanOK(NoAssign, "N1.Assign 50 50 50\n")) && Passed;
  Passed = EvaluateTrue("ScanSetupFile()", "Assign by position", "A position inside a detector is accepted", IsScanOK(NoAssign, "N1.Assign 0.5 0.5 0.5\n")) && Passed;
  Passed = EvaluateFalse("ScanSetupFile()", "named detector of a named detector", "A named detector of a named detector is rejected", IsScanOK(NoAssign, "N1.Assign World.C\nN1.Named N2\nN2.Assign World.C\n")) && Passed;
  Passed = EvaluateFalse("ScanSetupFile()", "named detector with sensitive volume", "A named detector takes the volumes from its base", IsScanOK(NoAssign, "N1.Assign World.C\nN1.SensitiveVolume C\n")) && Passed;
  Passed = EvaluateFalse("ScanSetupFile()", "named detector with detector volume", "A named detector takes the volumes from its base", IsScanOK(NoAssign, "N1.Assign World.C\nN1.DetectorVolume C\n")) && Passed;
  Passed = EvaluateFalse("ScanSetupFile()", "named detector with structural offset", "A named detector takes the structure from its base", IsScanOK(NoAssign, "N1.Assign World.C\nN1.StructuralOffset 0 0 0\n")) && Passed;
  Passed = EvaluateFalse("ScanSetupFile()", "named detector with structural pitch", "A named detector takes the structure from its base", IsScanOK(NoAssign, "N1.Assign World.C\nN1.StructuralPitch 0 0 0\n")) && Passed;

  // Base.BlockTrigger is applied after copying the data to the named detectors
  MDGeometry Blocked;
  Passed = EvaluateTrue("ScanSetupFile()", "BlockTrigger on the base", "The geometry is accepted", Scan(Blocked, NoAssign, "N1.Assign World.C\nBase.BlockTrigger 0 0\n")) && Passed;
  if (CheckDetector(Passed, "Blocked", Blocked, "N1", MDDetector::c_Scintillator)) {
    Passed = EvaluateFalse("IsAboveTriggerThreshold()", "BlockTrigger on the base, base", "The base does not trigger", Blocked.GetDetector("Base")->IsAboveTriggerThreshold(1000.0, MDGridPoint(0, 0, 0))) && Passed;
    // Defect GEO-D-016: BlockTrigger is parsed after the copy to the named detectors, thus channels blocked on the base still trigger in its named detectors
    Passed = ExpectedToFail(EvaluateFalse("IsAboveTriggerThreshold()", "BlockTrigger on the base, named", "The named detector inherits the blocked channel", Blocked.GetDetector("N1")->IsAboveTriggerThreshold(1000.0, MDGridPoint(0, 0, 0)))) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the generation of the geomega text
bool UTDDetector::TestGeomegaText()
{
  bool Passed = true;

  UTDDetectorStub Stub("Stub");
  MDVolume V("V");
  Stub.AddSensitiveVolume(&V);
  Stub.SetDetectorVolume(&V);
  Stub.SetStructuralOffset(MVector(0.25, 0.5, 0.75));
  Stub.SetStructuralPitch(MVector(1.25, 1.5, 1.75));
  Stub.SetEnergyResolutionType(MDDetector::c_EnergyResolutionTypeGauss);
  Stub.SetEnergyResolution(100.0, 110.0, 2.0);
  Stub.SetEnergyResolution(200.0, 210.0, 3.0);
  Stub.SetTimeResolution(100.0, 5.0);
  Stub.SetNoiseThreshold(10.0);
  Stub.SetNoiseThresholdSigma(1.0);
  Stub.SetTriggerThreshold(20.0);
  Stub.SetTriggerThresholdSigma(2.0);
  Stub.SetOverflow(1000.0);
  Stub.SetOverflowSigma(10.0);
  Stub.SetFailureRate(0.5);
  Stub.SetPulseShape(1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 3.0, 8.0);

  MString Text = Stub.GetGeomegaCommon();
  Passed = EvaluateTrue("GetGeomegaCommon()", "volumes", "The detector volume is written", Text.Contains("Stub.DetectorVolume V")) && Passed;
  Passed = EvaluateTrue("GetGeomegaCommon()", "volumes", "The sensitive volume is written", Text.Contains("Stub.SensitiveVolume V")) && Passed;
  Passed = EvaluateTrue("GetGeomegaCommon()", "pitch", "The pitch is written", Text.Contains("Stub.StructuralPitch 1.25 1.5 1.75")) && Passed;
  Passed = EvaluateTrue("GetGeomegaCommon()", "offset", "The offset is written", Text.Contains("Stub.StructuralOffset 0.25 0.5 0.75")) && Passed;
  Passed = EvaluateTrue("GetGeomegaCommon()", "energy resolution", "The energy resolution is written", Text.Contains("Stub.EnergyResolution Gauss 200 210 3")) && Passed;
  Passed = EvaluateTrue("GetGeomegaCommon()", "time resolution", "The time resolution is written", Text.Contains("Stub.TimeResolution 100 5")) && Passed;
  Passed = EvaluateTrue("GetGeomegaCommon()", "trigger threshold", "The trigger threshold is written", Text.Contains("Stub.TriggerThreshold 20 2")) && Passed;
  Passed = EvaluateTrue("GetGeomegaCommon()", "noise threshold", "The noise threshold is written", Text.Contains("Stub.NoiseThreshold 10 1")) && Passed;
  Passed = EvaluateTrue("GetGeomegaCommon()", "overflow", "The overflow is written", Text.Contains("Stub.Overflow 1000 10")) && Passed;
  Passed = EvaluateTrue("GetGeomegaCommon()", "failure rate", "The failure rate is written", Text.Contains("Stub.FailureRate 0.5")) && Passed;

  // Defect GEO-D-017: GetGeomegaCommon writes the lower limit of the pulse shape twice and never the upper limit
  Passed = ExpectedToFail(EvaluateTrue("GetGeomegaCommon()", "pulse shape", "The pulse shape ends with the lower and the upper limit", Text.Contains(" 3 8\n"))) && Passed;

  // The options switch the blocks off
  MString Reduced = Stub.GetGeomegaCommon(false, false, false, false, false, false, false, false, false);
  Passed = EvaluateTrue("GetGeomegaCommon()", "everything off", "Nothing is written", Reduced == "") && Passed;

  // Equal thresholds and the other types
  Stub.SetNoiseThresholdEqualsTriggerThreshold(true);
  Text = Stub.GetGeomegaCommon();
  Passed = EvaluateTrue("GetGeomegaCommon()", "equal thresholds", "The flag is written", Text.Contains("Stub.NoiseThresholdEqualsTriggerThreshold")) && Passed;

  // Defect GEO-D-018: The keyword NoiseThresholdEqualsTriggerThreshold requires a value in the parser, but it is written without one
  Passed = ExpectedToFail(EvaluateTrue("GetGeomegaCommon()", "equal thresholds", "The flag is written in the format the parser requires", Text.Contains("Stub.NoiseThresholdEqualsTriggerThreshold true"))) && Passed;

  // Defect GEO-D-019: The energy resolution types None and Ideal are not written, so None becomes Ideal in a round trip
  UTDDetectorStub NoEnergy("NoEnergy");
  NoEnergy.AddSensitiveVolume(&V);
  NoEnergy.SetDetectorVolume(&V);
  NoEnergy.SetEnergyResolutionType(MDDetector::c_EnergyResolutionTypeNone);
  Passed = ExpectedToFail(EvaluateTrue("GetGeomegaCommon()", "energy resolution type none", "The energy resolution type none is written", NoEnergy.GetGeomegaCommon().Contains("NoEnergy.EnergyResolution None"))) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the remaining small public functions
bool UTDDetector::TestMiscellaneous()
{
  bool Passed = true;

  UTDDetectorStub Stub("Stub");
  Stub.ActivateNoising(false);
  Passed = EvaluateFalse("ActivateNoising()", "false", "Noising is off", Stub.m_NoiseActive) && Passed;
  Stub.ActivateNoising();
  Passed = EvaluateTrue("ActivateNoising()", "default argument", "Noising is on by default", Stub.m_NoiseActive) && Passed;

  Passed = EvaluateFalse("IsVeto()", "base class", "The base class never vetoes", Stub.IsVeto(MVector(0.0, 0.0, 0.0), 1000.0)) && Passed;

  // The base class returns the structural size as the position resolution
  Stub.m_StructuralSize = MVector(1.0, 2.0, 3.0);
  Passed = EvaluateVectorNear("GetPositionResolution()", "base class", "The resolution is the structural size", Stub.GetPositionResolution(MVector(0.0, 0.0, 0.0), 100.0), MVector(1.0, 2.0, 3.0), 1e-12) && Passed;

  // Near: with pitch 1 and half size 1 cells are 3 cm wide, the distance has to be below 1.1 cells per level
  Stub.SetStructuralPitch(MVector(1.0, 1.0, 1.0));
  MVector Null(0.0, 0.0, 0.0);
  Passed = EvaluateTrue("AreNear()", "just inside level 1", "A distance of 3.29 cm is near", Stub.AreNear(MVector(0.0, 0.0, 0.0), Null, MVector(3.29, 0.0, 0.0), Null, 1.0, 1)) && Passed;
  Passed = EvaluateFalse("AreNear()", "just outside level 1", "A distance of 3.31 cm is not near", Stub.AreNear(MVector(0.0, 0.0, 0.0), Null, MVector(3.31, 0.0, 0.0), Null, 1.0, 1)) && Passed;

  // Text output
  MDVolume V("V");
  Stub.AddSensitiveVolume(&V);
  Stub.SetDetectorVolume(&V);
  MString Text = Stub.ToString();
  Passed = EvaluateTrue("ToString()", "base class", "The name is in the text", Text.Contains("Stub")) && Passed;
  Passed = EvaluateTrue("ToString()", "base class", "The volume is in the text", Text.Contains("with detector volume: V")) && Passed;

  // Defect GEO-D-020: ToString dereferences the detector volume without a check and crashes on a detector which is not fully set up
  UTDDetectorStub NoVolume("NoVolume");
  Passed = ExpectedToCrash(EvaluateTrue("ToString()", "no detector volume", "A detector without volume can be dumped", NoVolume.ToString().Contains("NoVolume"))) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  gROOT->SetBatch(true);
  gErrorIgnoreLevel = kWarning;
  TApplication ROOT("ROOT", 0, 0);

  UTDDetector Test;
  return Test.Run() == true ? 0 : 1;
}


// UTDDetector.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
