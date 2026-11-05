/*
 * UTSourceBasics.cc
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


// Cosima:
#include "MCOrientation.hh"
#include "MCSource.hh"
#include "UTSDHitShared.h"
#include "UTSourceShared.h"

// MEGAlib:
#include "MTokenizer.h"
#include "MUnitTest.h"

// Standard lib:
#include <cmath>
#include <cstring>
#include <limits>
#include <new>
#include <set>
#include <string>
#include <utility>
#include <vector>
using namespace std;


//! Unit test class for the basic data handling of MCSource: constants, defaults, setters, getters, validation,
//! start area, flux, derived energy parameters, mean energies, and the string output
class UTSourceBasics : public UTSourceTestBase
{
public:
  //! Default constructor
  UTSourceBasics() : UTSourceTestBase("UTSourceBasics") {}
  //! Default destructor
  virtual ~UTSourceBasics() {}

  //! Run all tests
  virtual bool Run();

private:
  //! One named constant with its expected value
  struct Constant {
    //! The name of the constant
    const char* m_Name;
    //! The value in the code
    int m_Actual;
    //! The value which is written in source files and thus must never change
    int m_Expected;
  };

  //! Test the numerical values of all constants
  bool TestConstants();
  //! Test the default state after construction, including uninitialized members
  bool TestDefaults();
  //! Test the unique IDs and names
  bool TestIdentity();
  //! Test the coordinate system, spectral type, and start area type setters
  bool TestSimpleTypeSetters();
  //! Test setting the beam type
  bool TestSetBeamType();
  //! Test setting the particle type and its consistency with the Geant4 definitions
  bool TestSetParticleType();
  //! Test the flux, including the scaling with the start area
  bool TestFlux();
  //! Test the abort on setting the flux before the start area parameters
  bool TestFluxOrder();
  //! Test the light curve settings
  bool TestLightCurveSettings();
  //! Test the polarization settings and the polarization name
  bool TestPolarization();
  //! Test orientation, successor, volume, and isotope count
  bool TestSmallProperties();
  //! Test the start area parameters and the average start area
  bool TestStartArea();
  //! Test the validation of the position parameters
  bool TestSetPositionValidation();
  //! Test storing and retrieving the position and energy parameters
  bool TestParameterStorage();
  //! Test the validation of the energy parameters
  bool TestSetEnergyValidation();
  //! Test the parameters derived from the energy parameters
  bool TestDerivedEnergyParameters();
  //! Test the spectrum and beam files
  bool TestFiles();
  //! Test the mean energy
  bool TestMeanEnergy();
  //! Test the string output
  bool TestStrings();
  bool TestRandomGuard();
  //! Test the far field options Earth occultation and transmission probability
  bool TestFarFieldOptions();

  //! Return the Band function as documented: E^alpha exp(-E/E0) below the break, the matching power law with beta above
  static double Band(double E, double Alpha, double Beta, double E0)
  {
    const double Break = (Alpha - Beta)*E0;
    return (E <= Break) ? pow(E, Alpha)*exp(-E/E0) : pow(Break, Alpha - Beta)*exp(Beta - Alpha)*pow(E, Beta);
  }
};


////////////////////////////////////////////////////////////////////////////////


bool UTSourceBasics::Run()
{
  bool Passed = true;

  Passed = TestConstants() && Passed;
  Passed = TestDefaults() && Passed;
  Passed = TestIdentity() && Passed;
  Passed = TestSimpleTypeSetters() && Passed;
  Passed = TestSetBeamType() && Passed;
  Passed = TestSetParticleType() && Passed;
  Passed = TestFlux() && Passed;
  Passed = TestFluxOrder() && Passed;
  Passed = TestLightCurveSettings() && Passed;
  Passed = TestPolarization() && Passed;
  Passed = TestSmallProperties() && Passed;
  Passed = TestStartArea() && Passed;
  Passed = TestSetPositionValidation() && Passed;
  Passed = TestParameterStorage() && Passed;
  Passed = TestSetEnergyValidation() && Passed;
  Passed = TestDerivedEnergyParameters() && Passed;
  Passed = TestFiles() && Passed;
  Passed = TestMeanEnergy() && Passed;
  Passed = TestStrings() && Passed;
  Passed = TestRandomGuard() && Passed;
  Passed = TestFarFieldOptions() && Passed;

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSourceBasics::TestConstants()
{
  bool Passed = true;

  // The IDs are used in source files and sim files, thus a renumbering would silently break them:
  const vector<Constant> Constants = {
    { "c_Invalid", MCSource::c_Invalid, -99999987 },
    { "c_FarField", MCSource::c_FarField, 1 }, { "c_NearField", MCSource::c_NearField, 2 },
    { "c_Map", MCSource::c_Map, 0 }, { "c_Monoenergetic", MCSource::c_Monoenergetic, 1 }, { "c_Linear", MCSource::c_Linear, 2 },
    { "c_PowerLaw", MCSource::c_PowerLaw, 3 }, { "c_BrokenPowerLaw", MCSource::c_BrokenPowerLaw, 4 }, { "c_CutOffPowerLaw", MCSource::c_CutOffPowerLaw, 5 },
    { "c_Comptonized", MCSource::c_Comptonized, 6 }, { "c_Gaussian", MCSource::c_Gaussian, 7 }, { "c_ThermalBremsstrahlung", MCSource::c_ThermalBremsstrahlung, 8 },
    { "c_BlackBody", MCSource::c_BlackBody, 9 }, { "c_BandFunction", MCSource::c_BandFunction, 10 }, { "c_FileDifferentialFlux", MCSource::c_FileDifferentialFlux, 11 },
    { "c_Activation", MCSource::c_Activation, 12 }, { "c_NormalizedEnergyBeamFluxFunction", MCSource::c_NormalizedEnergyBeamFluxFunction, 13 },
    { "c_StartAreaUnknown", MCSource::c_StartAreaUnknown, 0 }, { "c_StartAreaSphere", MCSource::c_StartAreaSphere, 1 }, { "c_StartAreaTube", MCSource::c_StartAreaTube, 2 },
    { "c_LightCurveFlat", MCSource::c_LightCurveFlat, 1 }, { "c_LightCurveFile", MCSource::c_LightCurveFile, 2 },
    { "c_FarFieldPoint", MCSource::c_FarFieldPoint, 1 }, { "c_FarFieldArea", MCSource::c_FarFieldArea, 2 }, { "c_FarFieldGaussian", MCSource::c_FarFieldGaussian, 3 },
    { "c_FarFieldAssymetricGaussian", MCSource::c_FarFieldAssymetricGaussian, 4 }, { "c_FarFieldFileZenithDependent", MCSource::c_FarFieldFileZenithDependent, 5 },
    { "c_FarFieldNormalizedEnergyBeamFluxFunction", MCSource::c_FarFieldNormalizedEnergyBeamFluxFunction, 6 }, { "c_FarFieldIsotropic", MCSource::c_FarFieldIsotropic, 7 },
    { "c_FarFieldDisk", MCSource::c_FarFieldDisk, 8 }, { "c_FarFieldEarthOccultation", MCSource::c_FarFieldEarthOccultation, 9 },
    { "c_NearFieldPoint", MCSource::c_NearFieldPoint, 10 }, { "c_NearFieldRestrictedPoint", MCSource::c_NearFieldRestrictedPoint, 11 },
    { "c_NearFieldDiffractionPoint", MCSource::c_NearFieldDiffractionPoint, 12 },
    { "c_NearFieldLine", MCSource::c_NearFieldLine, 13 }, { "c_NearFieldRestrictedLine", MCSource::c_NearFieldRestrictedLine, 14 }, { "c_NearFieldBox", MCSource::c_NearFieldBox, 15 },
    { "c_NearFieldDisk", MCSource::c_NearFieldDisk, 16 }, { "c_NearFieldSphere", MCSource::c_NearFieldSphere, 17 }, { "c_NearFieldBeam", MCSource::c_NearFieldBeam, 18 },
    { "c_NearFieldActivation", MCSource::c_NearFieldActivation, 19 }, { "c_NearFieldBeam1DProfile", MCSource::c_NearFieldBeam1DProfile, 20 },
    { "c_NearFieldBeam2DProfile", MCSource::c_NearFieldBeam2DProfile, 21 }, { "c_NearFieldConeBeam", MCSource::c_NearFieldConeBeam, 22 },
    { "c_NearFieldConeBeamGauss", MCSource::c_NearFieldConeBeamGauss, 23 }, { "c_NearFieldFanBeam", MCSource::c_NearFieldFanBeam, 24 },
    { "c_NearFieldIlluminatedDisk", MCSource::c_NearFieldIlluminatedDisk, 25 }, { "c_NearFieldIlluminatedSquare", MCSource::c_NearFieldIlluminatedSquare, 26 },
    { "c_NearFieldVolume", MCSource::c_NearFieldVolume, 27 }, { "c_NearFieldFlatMap", MCSource::c_NearFieldFlatMap, 28 },
    { "c_NearFieldReverseDirectionToPredecessor", MCSource::c_NearFieldReverseDirectionToPredecessor, 29 },
    { "c_PolarizationNone", MCSource::c_PolarizationNone, 1 }, { "c_PolarizationRandom", MCSource::c_PolarizationRandom, 2 }, { "c_PolarizationAbsolute", MCSource::c_PolarizationAbsolute, 3 },
    { "c_PolarizationRelativeX", MCSource::c_PolarizationRelativeX, 4 }, { "c_PolarizationRelativeY", MCSource::c_PolarizationRelativeY, 5 },
    { "c_PolarizationRelativeZ", MCSource::c_PolarizationRelativeZ, 6 }, { "c_PolarizationGalactic", MCSource::c_PolarizationGalactic, 7 },
    { "c_Gamma", MCSource::c_Gamma, 1 }, { "c_Positron", MCSource::c_Positron, 2 }, { "c_Electron", MCSource::c_Electron, 3 }, { "c_Proton", MCSource::c_Proton, 4 },
    { "c_AntiProton", MCSource::c_AntiProton, 5 }, { "c_Neutron", MCSource::c_Neutron, 6 }, { "c_AntiNeutron", MCSource::c_AntiNeutron, 7 },
    { "c_MuonPlus", MCSource::c_MuonPlus, 8 }, { "c_MuonMinus", MCSource::c_MuonMinus, 9 }, { "c_TauonPlus", MCSource::c_TauonPlus, 10 }, { "c_TauonMinus", MCSource::c_TauonMinus, 11 },
    { "c_ElectronNeutrino", MCSource::c_ElectronNeutrino, 12 }, { "c_AntiElectronNeutrino", MCSource::c_AntiElectronNeutrino, 13 },
    { "c_MuonNeutrino", MCSource::c_MuonNeutrino, 14 }, { "c_AntiMuonNeutrino", MCSource::c_AntiMuonNeutrino, 15 },
    { "c_TauonNeutrino", MCSource::c_TauonNeutrino, 16 }, { "c_AntiTauonNeutrino", MCSource::c_AntiTauonNeutrino, 17 },
    { "c_Deuteron", MCSource::c_Deuteron, 18 }, { "c_Triton", MCSource::c_Triton, 19 }, { "c_He3", MCSource::c_He3, 20 }, { "c_Alpha", MCSource::c_Alpha, 21 },
    { "c_GenericIon", MCSource::c_GenericIon, 22 }, { "c_PiPlus", MCSource::c_PiPlus, 23 }, { "c_PiZero", MCSource::c_PiZero, 24 }, { "c_PiMinus", MCSource::c_PiMinus, 25 },
    { "c_Eta", MCSource::c_Eta, 26 }, { "c_EtaPrime", MCSource::c_EtaPrime, 27 }, { "c_KaonPlus", MCSource::c_KaonPlus, 28 }, { "c_KaonZero", MCSource::c_KaonZero, 29 },
    { "c_AntiKaonZero", MCSource::c_AntiKaonZero, 30 }, { "c_KaonZeroS", MCSource::c_KaonZeroS, 31 }, { "c_KaonZeroL", MCSource::c_KaonZeroL, 32 }, { "c_KaonMinus", MCSource::c_KaonMinus, 33 },
    { "c_Lambda", MCSource::c_Lambda, 34 }, { "c_AntiLambda", MCSource::c_AntiLambda, 35 }, { "c_SigmaPlus", MCSource::c_SigmaPlus, 36 }, { "c_AntiSigmaPlus", MCSource::c_AntiSigmaPlus, 37 },
    { "c_SigmaZero", MCSource::c_SigmaZero, 38 }, { "c_AntiSigmaZero", MCSource::c_AntiSigmaZero, 39 }, { "c_SigmaMinus", MCSource::c_SigmaMinus, 40 },
    { "c_AntiSigmaMinus", MCSource::c_AntiSigmaMinus, 41 }, { "c_XiZero", MCSource::c_XiZero, 42 }, { "c_AntiXiZero", MCSource::c_AntiXiZero, 43 },
    { "c_XiMinus", MCSource::c_XiMinus, 44 }, { "c_AntiXiMinus", MCSource::c_AntiXiMinus, 45 }, { "c_OmegaMinus", MCSource::c_OmegaMinus, 46 },
    { "c_AntiOmegaMinus", MCSource::c_AntiOmegaMinus, 47 }, { "c_RhoPlus", MCSource::c_RhoPlus, 48 }, { "c_RhoZero", MCSource::c_RhoZero, 49 }, { "c_RhoMinus", MCSource::c_RhoMinus, 50 },
    { "c_DeltaMinus", MCSource::c_DeltaMinus, 51 }, { "c_DeltaZero", MCSource::c_DeltaZero, 52 }, { "c_DeltaPlus", MCSource::c_DeltaPlus, 53 },
    { "c_DeltaPlusPlus", MCSource::c_DeltaPlusPlus, 54 }, { "c_Omega", MCSource::c_Omega, 55 }
  };
  for (const Constant& C: Constants) {
    Passed = Evaluate("MCSource::constants", C.m_Name, "The constant has the value used in source files", C.m_Actual, C.m_Expected) && Passed;
  }

  // Within each family the IDs must be unique:
  set<int> Spectral;
  for (int I: { MCSource::c_Map, MCSource::c_Monoenergetic, MCSource::c_Linear, MCSource::c_PowerLaw, MCSource::c_BrokenPowerLaw, MCSource::c_CutOffPowerLaw,
                MCSource::c_Comptonized, MCSource::c_Gaussian, MCSource::c_ThermalBremsstrahlung, MCSource::c_BlackBody, MCSource::c_BandFunction,
                MCSource::c_FileDifferentialFlux, MCSource::c_Activation, MCSource::c_NormalizedEnergyBeamFluxFunction }) Spectral.insert(I);
  Passed = EvaluateSize("MCSource::constants", "spectral types", "All spectral types have different IDs", Spectral.size(), 14) && Passed;

  set<int> Beams;
  for (int I = 1; I <= 29; ++I) Beams.insert(I);
  set<int> BeamConstants;
  for (const Constant& C: Constants) {
    MString Name = C.m_Name;
    if (Name.BeginsWith("c_FarField") == true && Name != "c_FarField") BeamConstants.insert(C.m_Actual);
    if (Name.BeginsWith("c_NearField") == true && Name != "c_NearField") BeamConstants.insert(C.m_Actual);
  }
  Passed = EvaluateTrue("MCSource::constants", "beam types", "The far field and near field beam types use the IDs 1 to 29 exactly once", BeamConstants == Beams) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSourceBasics::TestDefaults()
{
  bool Passed = true;

  MCSource S("Defaults");
  Passed = Evaluate("MCSource()", "name", "The name is the one given to the constructor", S.GetName(), MString("Defaults")) && Passed;
  Passed = Evaluate("GetNGeneratedParticles()", "default", "No particle has been generated", S.GetNGeneratedParticles(), 0L) && Passed;
  Passed = EvaluateFalse("IsFluxVariable()", "default", "The flux is not variable", S.IsFluxVariable()) && Passed;
  Passed = EvaluateNear("GetNextEmission()", "default", "The next emission is at time zero", S.GetNextEmission(), 0.0, 1e-12) && Passed;
  Passed = Evaluate("GetNEventsToSkip()", "default", "No event is skipped", S.GetNEventsToSkip(), 0U) && Passed;
  Passed = Evaluate("GetNEventsSkipped()", "default", "No event has been skipped", S.GetNEventsSkipped(), 0U) && Passed;
  Passed = EvaluateTrue("IsActive()", "default", "A new source is active", S.IsActive()) && Passed;
  Passed = EvaluateFalse("IsEventList()", "default", "A new source is no event list", S.IsEventList()) && Passed;
  Passed = EvaluateFalse("IsBuildUpEventList()", "default", "A new source is no build up event list", S.IsBuildUpEventList()) && Passed;
  Passed = Evaluate("SizeEventList()", "default", "The event list is empty", S.SizeEventList(), 0U) && Passed;
  Passed = Evaluate("GetCoordinateSystem()", "default", "The coordinate system is not set", S.GetCoordinateSystem(), MCSource::c_Invalid) && Passed;
  Passed = Evaluate("GetSpectralType()", "default", "The spectral type is not set", S.GetSpectralType(), MCSource::c_Invalid) && Passed;
  Passed = Evaluate("GetBeamType()", "default", "The beam type is not set", S.GetBeamType(), MCSource::c_Invalid) && Passed;
  Passed = Evaluate("GetParticleType()", "default", "The particle type is not set", S.GetParticleType(), MCSource::c_Invalid) && Passed;
  Passed = EvaluateNear("GetParticleExcitation()", "default", "The excitation is zero", S.GetParticleExcitation(), 0.0, 1e-12) && Passed;
  Passed = EvaluateTrue("GetParticleDefinition()", "default", "There is no particle definition", S.GetParticleDefinition() == nullptr) && Passed;
  Passed = EvaluateNear("GetFlux()", "default", "The flux is not set", S.GetFlux(), MCSource::c_Invalid, 1e-6) && Passed;
  Passed = EvaluateNear("GetInputFlux()", "default", "The input flux is not set", S.GetInputFlux(), MCSource::c_Invalid, 1e-6) && Passed;
  Passed = Evaluate("GetLightCurveType()", "default", "The default light curve is flat", S.GetLightCurveType(), MCSource::c_LightCurveFlat) && Passed;
  Passed = Evaluate("GetPolarizationType()", "default", "The default polarization type is none", S.GetPolarizationType(), MCSource::c_PolarizationNone) && Passed;
  Passed = EvaluateNear("GetPolarizationDegree()", "default", "The default polarization degree is zero", S.GetPolarizationDegree(), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetPolarizationVector()", "default", "The default polarization vector is zero", S.GetPolarizationVector().mag(), 0.0, 1e-12) && Passed;
  Passed = EvaluateFalse("IsSuccessor()", "default", "A new source is no successor", S.IsSuccessor()) && Passed;
  Passed = Evaluate("GetSuccessor()", "default", "A new source has no successor", S.GetSuccessor(), MString("")) && Passed;
  Passed = Evaluate("GetVolume()", "default", "A new source has no volume", S.GetVolume(), MString("")) && Passed;
  Passed = Evaluate("GetStartAreaType()", "default", "The start area type is unknown", S.GetStartAreaType(), MCSource::c_StartAreaUnknown) && Passed;
  Passed = EvaluateNear("GetStartAreaAverageArea()", "default", "The average start area is zero", S.GetStartAreaAverageArea(), 0.0, 1e-12) && Passed;
  Passed = EvaluateFalse("GetOrientation()", "default", "A new source is not oriented", S.GetOrientation().IsOriented()) && Passed;

  for (unsigned int i = 1; i <= 11; ++i) {
    Passed = EvaluateNear("GetPositionParameter()", MString("parameter ") + i, "Position parameters are not set after construction", S.GetPositionParameter(i), MCSource::c_Invalid, 1e-6) && Passed;
  }
  for (unsigned int i = 1; i <= 6; ++i) {
    Passed = EvaluateNear("GetEnergyParameter()", MString("parameter ") + i, "Energy parameters are not set after construction", S.GetEnergyParameter(i), MCSource::c_Invalid, 1e-6) && Passed;
  }

  // Members which are not initialized are detected by constructing the source in memory filled with different patterns:
  for (unsigned char Pattern: { (unsigned char) 0x00, (unsigned char) 0xFF, (unsigned char) 0x5A }) {
    alignas(MCSource) unsigned char Buffer[sizeof(MCSource)];
    memset(Buffer, Pattern, sizeof(Buffer));
    MCSource* P = new (Buffer) MCSource("Pattern");
    Passed = EvaluateNear("GetEnergyParameter()", MString("seventh parameter, memory pattern ") + (int) Pattern, "The seventh energy parameter is not set after construction, like the first six", P->GetEnergyParameter(7), MCSource::c_Invalid, 1e-6) && Passed;
    P->~MCSource();
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSourceBasics::TestIdentity()
{
  bool Passed = true;

  MCSource A("A");
  MCSource B("B");
  MCSource C;
  Passed = EvaluateTrue("GetID()", "two sources", "Two sources have different IDs", A.GetID() != B.GetID()) && Passed;
  Passed = Evaluate("GetID()", "consecutive", "IDs are handed out consecutively", B.GetID(), A.GetID() + 1) && Passed;
  Passed = Evaluate("GetID()", "default constructor", "The default constructor also gets the next ID", C.GetID(), B.GetID() + 1) && Passed;
  Passed = Evaluate("GetName()", "default constructor", "A source without name has an empty name", C.GetName(), MString("")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSourceBasics::TestSimpleTypeSetters()
{
  bool Passed = true;

  // Coordinate system:
  {
    MCSource S("Coordinates");
    SilenceOutput();
    bool Far = S.SetCoordinateSystem(MCSource::c_FarField);
    RestoreOutput();
    Passed = EvaluateTrue("SetCoordinateSystem()", "far field", "The far field coordinate system is accepted", Far) && Passed;
    Passed = Evaluate("GetCoordinateSystem()", "far field", "The far field coordinate system is stored", S.GetCoordinateSystem(), MCSource::c_FarField) && Passed;
    SilenceOutput();
    bool Near = S.SetCoordinateSystem(MCSource::c_NearField);
    RestoreOutput();
    Passed = EvaluateTrue("SetCoordinateSystem()", "near field", "The near field coordinate system is accepted", Near) && Passed;
    for (int Invalid: { 0, 3, -1, MCSource::c_Invalid }) {
      SilenceOutput();
      bool Result = S.SetCoordinateSystem(Invalid);
      RestoreOutput();
      Passed = EvaluateFalse("SetCoordinateSystem()", MString("invalid ") + Invalid, "An unknown coordinate system is rejected", Result) && Passed;
      Passed = Evaluate("SetCoordinateSystem()", MString("invalid ") + Invalid, "A rejected coordinate system does not change the stored one", S.GetCoordinateSystem(), MCSource::c_NearField) && Passed;
    }
  }

  // Spectral type:
  {
    MCSource S("Spectral");
    for (int Type = 0; Type <= 13; ++Type) {
      SilenceOutput();
      bool Result = S.SetSpectralType(Type);
      RestoreOutput();
      Passed = EvaluateTrue("SetSpectralType()", MString("type ") + Type, "Every defined spectral type is accepted", Result) && Passed;
      Passed = Evaluate("GetSpectralType()", MString("type ") + Type, "The spectral type is stored", S.GetSpectralType(), Type) && Passed;
    }
    for (int Invalid: { 14, 100, -1, MCSource::c_Invalid }) {
      SilenceOutput();
      bool Result = S.SetSpectralType(Invalid);
      RestoreOutput();
      Passed = EvaluateFalse("SetSpectralType()", MString("invalid ") + Invalid, "An unknown spectral type is rejected", Result) && Passed;
      Passed = Evaluate("SetSpectralType()", MString("invalid ") + Invalid, "A rejected spectral type does not change the stored one", S.GetSpectralType(), 13) && Passed;
    }
  }

  // Start area type:
  {
    MCSource S("StartAreaType");
    for (int Type: { MCSource::c_StartAreaUnknown, MCSource::c_StartAreaSphere, MCSource::c_StartAreaTube }) {
      Passed = EvaluateTrue("SetStartAreaType()", MString("type ") + Type, "Every defined start area type is accepted", S.SetStartAreaType(Type)) && Passed;
      Passed = Evaluate("GetStartAreaType()", MString("type ") + Type, "The start area type is stored", S.GetStartAreaType(), Type) && Passed;
    }
    for (int Invalid: { 3, 100, -1, MCSource::c_Invalid }) {
      SilenceOutput();
      const bool Result = S.SetStartAreaType(Invalid);
      RestoreOutput();
      Passed = EvaluateFalse("SetStartAreaType()", MString("invalid ") + Invalid, "An unknown start area type is rejected", Result) && Passed;
      Passed = Evaluate("SetStartAreaType()", MString("invalid ") + Invalid, "A rejected start area type does not change the stored one", S.GetStartAreaType(), MCSource::c_StartAreaTube) && Passed;
    }

    // Beam types which require a sphere reject the tube:
    const vector<pair<int, MString>> NeedSphere = { { MCSource::c_NearFieldIlluminatedDisk, "IlluminatedDisk" }, { MCSource::c_NearFieldIlluminatedSquare, "IlluminatedSquare" },
                                                     { MCSource::c_NearFieldRestrictedPoint, "RestrictedPoint" }, { MCSource::c_NearFieldRestrictedLine, "RestrictedLine" } };
    for (const auto& N: NeedSphere) {
      MCSource T("NeedSphere");
      SilenceOutput();
      T.SetBeamType(MCSource::c_NearField, N.first);
      bool Result = T.SetStartAreaType(MCSource::c_StartAreaTube);
      RestoreOutput();
      Passed = EvaluateFalse("SetStartAreaType()", N.second + " with a tube", "A beam type which requires a spherical start area rejects the tube, as the message in the code says", Result) && Passed;
    }
  }

  // Excitation:
  {
    MCSource S("Excitation");
    Passed = EvaluateTrue("SetParticleExcitation()", "58.59 keV", "An excitation is accepted", S.SetParticleExcitation(58.59*keV)) && Passed;
    Passed = EvaluateNear("GetParticleExcitation()", "58.59 keV", "The excitation is stored", S.GetParticleExcitation()/keV, 58.59, 1e-9) && Passed;
    Passed = EvaluateTrue("SetParticleExcitation()", "zero", "A zero excitation is accepted", S.SetParticleExcitation(0.0)) && Passed;
    Passed = EvaluateNear("GetParticleExcitation()", "zero", "A zero excitation is stored", S.GetParticleExcitation(), 0.0, 1e-12) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSourceBasics::TestSetBeamType()
{
  bool Passed = true;

  // All defined beam types are accepted in both coordinate systems:
  for (int Coordinates: { MCSource::c_FarField, MCSource::c_NearField }) {
    for (int Beam = 1; Beam <= 29; ++Beam) {
      MCSource S("Beam");
      SilenceOutput();
      bool Result = S.SetBeamType(Coordinates, Beam);
      RestoreOutput();
      Passed = EvaluateTrue("SetBeamType()", MString("coordinates ") + Coordinates + ", beam " + Beam, "Every defined beam type is accepted", Result) && Passed;
      Passed = Evaluate("GetBeamType()", MString("coordinates ") + Coordinates + ", beam " + Beam, "The beam type is stored", S.GetBeamType(), Beam) && Passed;
      Passed = Evaluate("GetCoordinateSystem()", MString("coordinates ") + Coordinates + ", beam " + Beam, "The coordinate system is stored", S.GetCoordinateSystem(), Coordinates) && Passed;
    }
  }

  // Unknown beam types:
  for (int Invalid: { 0, 30, 31, 100, -1, MCSource::c_Invalid }) {
    MCSource S("Beam");
    SilenceOutput();
    bool Result = S.SetBeamType(MCSource::c_NearField, Invalid);
    RestoreOutput();
    Passed = EvaluateFalse("SetBeamType()", MString("invalid beam ") + Invalid, "An unknown beam type is rejected", Result) && Passed;
    Passed = Evaluate("SetBeamType()", MString("invalid beam ") + Invalid, "A rejected beam type does not set the beam type", S.GetBeamType(), MCSource::c_Invalid) && Passed;
  }

  // Unknown coordinate systems:
  for (int Invalid: { 0, 3, -1, MCSource::c_Invalid }) {
    MCSource S("Beam");
    SilenceOutput();
    bool Result = S.SetBeamType(Invalid, MCSource::c_NearFieldPoint);
    RestoreOutput();
    Passed = EvaluateFalse("SetBeamType()", MString("invalid coordinates ") + Invalid, "An unknown coordinate system is rejected", Result) && Passed;
    Passed = Evaluate("SetBeamType()", MString("invalid coordinates ") + Invalid, "A rejected coordinate system does not set the beam type", S.GetBeamType(), MCSource::c_Invalid) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSourceBasics::TestSetParticleType()
{
  bool Passed = true;

  Passed = EvaluateTrue("InitializeEnvironment()", "particle definitions", "The Geant4 environment can be created", InitializeEnvironment()) && Passed;

  // Elementary particles:
  for (int Type = 1; Type <= 55; ++Type) {
    // Generic ion: it has a Geant4 definition but no source can be a generic ion
    if (Type == MCSource::c_GenericIon) continue;

    MCSource S("Particle");
    SilenceOutput();
    bool Accepted = S.SetParticleType(Type);
    G4ParticleDefinition* Definition = MCSteppingAction::GetParticleDefinition(Type, 0.0);
    RestoreOutput();
    Passed = EvaluateTrue("SetParticleType()", MString("type ") + Type, "A particle type is accepted if and only if Geant4 has a definition for it", Accepted == (Definition != nullptr)) && Passed;
    if (Accepted == true) {
      Passed = Evaluate("GetParticleType()", MString("type ") + Type, "The particle type is stored", S.GetParticleType(), Type) && Passed;
    }
  }

  // Ions are given as 1000*Z + A:
  for (int Ion: { 1001, 11024, 27060, 92238 }) {
    MCSource S("Ion");
    SilenceOutput();
    bool Accepted = S.SetParticleType(Ion);
    RestoreOutput();
    Passed = EvaluateTrue("SetParticleType()", MString("ion ") + Ion, "An ion ID is accepted", Accepted) && Passed;
    Passed = Evaluate("GetParticleType()", MString("ion ") + Ion, "The ion ID is stored", S.GetParticleType(), Ion) && Passed;
  }

  // Not accepted:
  for (int Invalid: { 0, -1, 56, 1000, MCSource::c_Invalid }) {
    MCSource S("Invalid");
    SilenceOutput();
    bool Accepted = S.SetParticleType(Invalid);
    RestoreOutput();
    Passed = EvaluateFalse("SetParticleType()", MString("invalid ") + Invalid, "An unknown particle type is rejected", Accepted) && Passed;
    Passed = Evaluate("SetParticleType()", MString("invalid ") + Invalid, "A rejected particle type is not stored", S.GetParticleType(), MCSource::c_Invalid) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSourceBasics::TestFlux()
{
  bool Passed = true;

  // Without a coordinate system the flux is used as it is:
  {
    MCSource S("Flux");
    SilenceOutput();
    bool Result = S.SetFlux(1.5/second);
    RestoreOutput();
    Passed = EvaluateTrue("SetFlux()", "positive", "A positive flux is accepted", Result) && Passed;
    Passed = EvaluateNear("GetInputFlux()", "positive", "The input flux is stored", S.GetInputFlux()*second, 1.5, 1e-12) && Passed;
    Passed = EvaluateNear("GetFlux()", "positive", "Without a far field the flux is the input flux", S.GetFlux()*second, 1.5, 1e-12) && Passed;

    for (double Invalid: { 0.0, -2.0 }) {
      SilenceOutput();
      bool Rejected = S.SetFlux(Invalid);
      RestoreOutput();
      Passed = EvaluateFalse("SetFlux()", MString("invalid ") + Invalid, "A flux which is not positive is rejected", Rejected) && Passed;
      Passed = EvaluateNear("SetFlux()", MString("invalid ") + Invalid, "A rejected flux does not change the stored one", S.GetInputFlux()*second, 1.5, 1e-12) && Passed;
    }
  }

  // The normalized energy-beam-flux-function is already normalized:
  {
    MCSource S("Normalized spectrum");
    SilenceOutput();
    S.SetSpectralType(MCSource::c_NormalizedEnergyBeamFluxFunction);
    bool Result = S.SetFlux(1.0);
    RestoreOutput();
    Passed = EvaluateFalse("SetFlux()", "normalized spectrum", "A source with a normalized energy-beam-flux-function does not accept a flux", Result) && Passed;

    MCSource T("Normalized beam");
    SilenceOutput();
    T.SetBeamType(MCSource::c_FarField, MCSource::c_FarFieldNormalizedEnergyBeamFluxFunction);
    bool Result2 = T.SetFlux(1.0);
    RestoreOutput();
    Passed = EvaluateFalse("SetFlux()", "normalized beam", "A far field beam with a normalized energy-beam-flux-function does not accept a flux", Result2) && Passed;
  }

  // The far field flux is a flux per area: the source flux is multiplied with the average start area:
  {
    const double Radius = 50*cm;
    const double FluxPerArea = 2.0/(cm*cm*second);

    // Flux before the start area (the type is unknown then, thus the flux cannot be scaled yet):
    MCSource S("Far field flux");
    SilenceOutput();
    S.SetBeamType(MCSource::c_FarField, MCSource::c_FarFieldPoint);
    S.SetPosition(30*deg, 45*deg);
    bool Flux = S.SetFlux(FluxPerArea);
    S.SetStartAreaType(MCSource::c_StartAreaSphere);
    bool Areas = S.SetStartAreaParameters(0.0, 0.0, 0.0, 0.0, 0.0, 0.0, Radius);
    RestoreOutput();
    Passed = EvaluateTrue("SetFlux()", "far field, before start area", "The flux is accepted before the start area is known", Flux) && Passed;
    Passed = EvaluateTrue("SetStartAreaParameters()", "sphere", "The start sphere is accepted after the flux", Areas) && Passed;
    Passed = EvaluateNear("GetFlux()", "far field", "The far field flux is the flux per area times the start area", S.GetFlux()*second, FluxPerArea*second*c_Pi*Radius*Radius, 1e-9*FluxPerArea*second*c_Pi*Radius*Radius) && Passed;
    Passed = EvaluateNear("GetInputFlux()", "far field", "The input flux is not scaled", S.GetInputFlux()*(cm*cm*second), 2.0, 1e-12) && Passed;
  }

  // A restricted point source only emits into the cone which hits the start sphere:
  {
    const double Radius = 20*cm;
    const double Distance = 100*cm;
    MCSource S("Restricted point");
    SilenceOutput();
    S.SetBeamType(MCSource::c_NearField, MCSource::c_NearFieldRestrictedPoint);
    bool Position = S.SetPosition(0.0, 0.0, Distance);
    bool Flux = S.SetFlux(10.0);
    S.SetStartAreaType(MCSource::c_StartAreaSphere);
    S.SetStartAreaParameters(0.0, 0.0, 0.0, 0.0, 0.0, 0.0, Radius);
    RestoreOutput();
    const double Cone = asin(Radius/Distance);
    Passed = EvaluateTrue("SetPosition()", "restricted point", "The restricted point source is accepted", Position) && Passed;
    Passed = EvaluateTrue("SetFlux()", "restricted point", "The flux of the restricted point source is accepted", Flux) && Passed;
    Passed = EvaluateNear("GetPositionParameter()", "restricted point", "The opening angle is the angle under which the start sphere is seen", S.GetPositionParameter(7), Cone, 1e-9) && Passed;
    Passed = EvaluateNear("GetFlux()", "restricted point", "The flux is the fraction of the full sphere covered by the cone", S.GetFlux(), 10.0*(1.0 - cos(Cone))/2.0, 1e-9) && Passed;

    // A point inside the start sphere emits everywhere:
    MCSource T("Restricted point inside");
    SilenceOutput();
    T.SetBeamType(MCSource::c_NearField, MCSource::c_NearFieldRestrictedPoint);
    T.SetPosition(0.0, 0.0, 5*cm);
    T.SetFlux(10.0);
    T.SetStartAreaType(MCSource::c_StartAreaSphere);
    T.SetStartAreaParameters(0.0, 0.0, 0.0, 0.0, 0.0, 0.0, Radius);
    RestoreOutput();
    Passed = EvaluateNear("GetFlux()", "restricted point inside", "A point inside the start sphere keeps its full flux", T.GetFlux(), 10.0, 1e-9) && Passed;
    Passed = EvaluateNear("GetPositionParameter()", "restricted point inside", "The opening angle is pi inside the start sphere", T.GetPositionParameter(7), c_Pi, 1e-9) && Passed;
  }

  // A restricted line source: the cone is the one under which the sphere is seen from the line
  {
    const double Radius = 20*cm;
    MCSource S("Restricted line");
    SilenceOutput();
    S.SetBeamType(MCSource::c_NearField, MCSource::c_NearFieldRestrictedLine);
    bool Position = S.SetPosition(50*cm, 0.0, -10*cm, 50*cm, 0.0, 10*cm);
    bool Flux = S.SetFlux(10.0);
    S.SetStartAreaType(MCSource::c_StartAreaSphere);
    S.SetStartAreaParameters(0.0, 0.0, 0.0, 0.0, 0.0, 0.0, Radius);
    RestoreOutput();
    const double Cone = asin(Radius/(50*cm));
    Passed = EvaluateTrue("SetPosition()", "restricted line", "The restricted line source is accepted", Position) && Passed;
    Passed = EvaluateTrue("SetFlux()", "restricted line", "The flux of the restricted line source is accepted", Flux) && Passed;
    Passed = EvaluateNear("GetFlux()", "restricted line", "The flux is the fraction of the full sphere covered by the cone", S.GetFlux(), 10.0*(1.0 - cos(Cone))/2.0, 1e-9) && Passed;

    // A line which crosses the start sphere becomes a normal line source:
    MCSource T("Restricted line crossing");
    SilenceOutput();
    T.SetBeamType(MCSource::c_NearField, MCSource::c_NearFieldRestrictedLine);
    T.SetFlux(10.0);
    T.SetStartAreaType(MCSource::c_StartAreaSphere);
    T.SetStartAreaParameters(0.0, 0.0, 0.0, 0.0, 0.0, 0.0, Radius);
    bool Crossing = T.SetPosition(10*cm, 0.0, -10*cm, 10*cm, 0.0, 10*cm);
    RestoreOutput();
    Passed = EvaluateTrue("SetPosition()", "restricted line crossing the sphere", "The line is accepted", Crossing) && Passed;
    Passed = Evaluate("GetBeamType()", "restricted line crossing the sphere", "A restricted line which crosses the start sphere becomes a normal line source", T.GetBeamType(), MCSource::c_NearFieldLine) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSourceBasics::TestFluxOrder()
{
  bool Passed = true;

  // Both orders of flux and start area which are not the usual one are rejected with false (they used to abort the whole process in UpgradeFlux()):
  auto FluxAfterType = []() {
    MCSource S("Order");
    bool OK = S.SetBeamType(MCSource::c_FarField, MCSource::c_FarFieldPoint);
    OK = S.SetPosition(30*deg, 45*deg) && OK;
    OK = S.SetStartAreaType(MCSource::c_StartAreaSphere) && OK;
    // The result of the flux: it cannot be normalized to the start area before the start area parameters are known
    return OK && S.SetFlux(1.0);
  };
  auto ParametersBeforeFlux = []() {
    MCSource S("Order");
    bool OK = S.SetBeamType(MCSource::c_FarField, MCSource::c_FarFieldPoint);
    OK = S.SetPosition(30*deg, 45*deg) && OK;
    OK = S.SetStartAreaType(MCSource::c_StartAreaSphere) && OK;
    return OK && S.SetStartAreaParameters(0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 10.0);
  };
  SilenceOutput();
  UTSDHitShared::ChildOutcome A = UTSDHitShared::RunInChild(FluxAfterType);
  UTSDHitShared::ChildOutcome B = UTSDHitShared::RunInChild(ParametersBeforeFlux);
  RestoreOutput();
  Passed = EvaluateTrue("SetFlux()", "start area type set, parameters missing", "Setting the flux before the start area parameters is rejected (the flux cannot be normalized to an unknown start area): the call returns false, and does not crash, time out, or fail to fork", A == UTSDHitShared::ChildOutcome::c_ReturnedFalse) && Passed;
  Passed = EvaluateTrue("SetStartAreaParameters()", "flux not set", "Setting the start area parameters before the flux is rejected (there is no flux to normalize): the call returns false, and does not crash, time out, or fail to fork", B == UTSDHitShared::ChildOutcome::c_ReturnedFalse) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSourceBasics::TestLightCurveSettings()
{
  bool Passed = true;

  MCSource S("Light curve");
  Passed = EvaluateTrue("SetLightCurveType()", "file", "The light curve type file is accepted", S.SetLightCurveType(MCSource::c_LightCurveFile)) && Passed;
  Passed = Evaluate("GetLightCurveType()", "file", "The light curve type is stored", S.GetLightCurveType(), MCSource::c_LightCurveFile) && Passed;
  Passed = EvaluateTrue("SetLightCurveType()", "flat", "The light curve type flat is accepted", S.SetLightCurveType(MCSource::c_LightCurveFlat)) && Passed;
  for (int Invalid: { 0, 3, -1, MCSource::c_Invalid }) {
    SilenceOutput();
    bool Result = S.SetLightCurveType(Invalid);
    RestoreOutput();
    Passed = EvaluateFalse("SetLightCurveType()", MString("invalid ") + Invalid, "An unknown light curve type is rejected", Result) && Passed;
    Passed = Evaluate("SetLightCurveType()", MString("invalid ") + Invalid, "A rejected light curve type does not change the stored one", S.GetLightCurveType(), MCSource::c_LightCurveFlat) && Passed;
  }

  // A light curve file is only accepted for the light curve type file:
  const MString GoodFile = GetTemporaryFileName("LightCurve.dat");
  const MString ShortFile = GetTemporaryFileName("LightCurveShort.dat");
  Passed = EvaluateTrue("WriteTextFile()", "light curve", "The light curve fixture can be written", WriteTextFile(GoodFile, "DP 0 1\nDP 10 2\nDP 20 1\nEN\n")) && Passed;
  Passed = EvaluateTrue("WriteTextFile()", "short light curve", "The short light curve fixture can be written", WriteTextFile(ShortFile, "DP 0 1\nEN\n")) && Passed;

  SilenceOutput();
  bool FlatType = S.SetLightCurve(GoodFile, false);
  RestoreOutput();
  Passed = EvaluateFalse("SetLightCurve()", "flat type", "A light curve file is rejected for the flat light curve type", FlatType) && Passed;
  Passed = EvaluateFalse("IsFluxVariable()", "flat type", "A rejected light curve does not make the flux variable", S.IsFluxVariable()) && Passed;

  SilenceOutput();
  S.SetLightCurveType(MCSource::c_LightCurveFile);
  bool Missing = S.SetLightCurve(GetTemporaryFileName("Missing.dat"), false);
  bool Short = S.SetLightCurve(ShortFile, false);
  RestoreOutput();
  Passed = EvaluateFalse("SetLightCurve()", "missing file", "A missing light curve file is rejected", Missing) && Passed;
  Passed = EvaluateFalse("SetLightCurve()", "one entry", "A light curve with less than two entries is rejected", Short) && Passed;
  Passed = EvaluateFalse("IsFluxVariable()", "rejected files", "Rejected light curve files do not make the flux variable", S.IsFluxVariable()) && Passed;

  SilenceOutput();
  bool Good = S.SetLightCurve(GoodFile, true);
  RestoreOutput();
  Passed = EvaluateTrue("SetLightCurve()", "valid file", "A valid light curve file is accepted", Good) && Passed;
  Passed = EvaluateTrue("IsFluxVariable()", "valid file", "A valid light curve makes the flux variable", S.IsFluxVariable()) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSourceBasics::TestPolarization()
{
  bool Passed = true;

  MCSource S("Polarization");
  const vector<pair<int, MString>> Types = { { MCSource::c_PolarizationNone, "None" }, { MCSource::c_PolarizationRandom, "Random" }, { MCSource::c_PolarizationAbsolute, "Absolute" },
                                             { MCSource::c_PolarizationRelativeX, "RelativeX" }, { MCSource::c_PolarizationRelativeY, "RelativeY" },
                                             { MCSource::c_PolarizationRelativeZ, "RelativeZ" }, { MCSource::c_PolarizationGalactic, "Galactic" } };
  for (const auto& T: Types) {
    Passed = EvaluateTrue("SetPolarizationType()", T.second, "Every defined polarization type is accepted", S.SetPolarizationType(T.first)) && Passed;
    Passed = Evaluate("GetPolarizationType()", T.second, "The polarization type is stored", S.GetPolarizationType(), T.first) && Passed;
    // The spectral type is not set, thus it must not influence the name:
    Passed = Evaluate("GetPolarizationTypeAsString()", T.second, "The polarization name is the one of the polarization type", MString(S.GetPolarizationTypeAsString()), T.second) && Passed;
  }
  for (int Invalid: { 0, 8, -1, MCSource::c_Invalid }) {
    Passed = EvaluateFalse("SetPolarizationType()", MString("invalid ") + Invalid, "An unknown polarization type is rejected", S.SetPolarizationType(Invalid)) && Passed;
    Passed = Evaluate("SetPolarizationType()", MString("invalid ") + Invalid, "A rejected polarization type does not change the stored one", S.GetPolarizationType(), MCSource::c_PolarizationGalactic) && Passed;
  }

  // The polarization name must not depend on the spectral type:
  MCSource T("Polarization name");
  T.SetPolarizationType(MCSource::c_PolarizationAbsolute);
  SilenceOutput();
  T.SetSpectralType(MCSource::c_Monoenergetic);
  RestoreOutput();
  Passed = Evaluate("GetPolarizationTypeAsString()", "spectral type set", "The polarization name does not depend on the spectral type", MString(T.GetPolarizationTypeAsString()), MString("Absolute")) && Passed;

  // Degree:
  MCSource D("Degree");
  for (double Degree: { 0.0, 0.25, 0.5, 1.0 }) {
    D.SetPolarizationDegree(Degree);
    Passed = EvaluateNear("SetPolarizationDegree()", MString("degree ") + Degree, "A valid polarization degree is stored", D.GetPolarizationDegree(), Degree, 1e-12) && Passed;
  }
  SilenceOutput();
  D.SetPolarizationDegree(1.5);
  RestoreOutput();
  Passed = EvaluateNear("SetPolarizationDegree()", "1.5", "A polarization degree above 100% is set to 100%", D.GetPolarizationDegree(), 1.0, 1e-12) && Passed;
  SilenceOutput();
  D.SetPolarizationDegree(-0.5);
  RestoreOutput();
  Passed = EvaluateNear("SetPolarizationDegree()", "-0.5", "A negative polarization degree is set to 0%", D.GetPolarizationDegree(), 0.0, 1e-12) && Passed;

  // Parameters are always accepted:
  Passed = EvaluateTrue("SetPolarization()", "three parameters", "The polarization parameters are accepted", D.SetPolarization(1.0, 0.0, 0.0)) && Passed;
  Passed = EvaluateTrue("SetPolarization()", "default parameters", "Default polarization parameters are accepted", D.SetPolarization()) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSourceBasics::TestSmallProperties()
{
  bool Passed = true;

  // Orientation:
  {
    MCSource S("Orientation");
    MCOrientation Orientation;
    MTokenizer Tokens;
    Tokens.Analyze("Src.Orientation Galactic Fixed 0 0 0 90", false);
    SilenceOutput();
    bool Parsed = Orientation.Parse(Tokens);
    RestoreOutput();
    Passed = EvaluateTrue("MCOrientation::Parse()", "galactic fixed", "The orientation fixture can be parsed", Parsed) && Passed;
    Passed = EvaluateTrue("SetOrientation()", "galactic fixed", "An orientation is accepted", S.SetOrientation(Orientation)) && Passed;
    Passed = EvaluateTrue("GetOrientation()", "galactic fixed", "The orientation is stored", S.GetOrientation().IsOriented()) && Passed;
    Passed = EvaluateTrue("GetOrientation()", "galactic fixed", "The coordinate system of the orientation is stored", S.GetOrientation().GetCoordinateSystem() == MCOrientationCoordinateSystem::c_Galactic) && Passed;
  }

  // Successor and volume:
  {
    MCSource S("Successor");
    Passed = EvaluateTrue("SetIsSuccessor()", "true", "The successor flag is accepted", S.SetIsSuccessor(true)) && Passed;
    Passed = EvaluateTrue("IsSuccessor()", "true", "The successor flag is stored", S.IsSuccessor()) && Passed;
    Passed = EvaluateTrue("SetIsSuccessor()", "false", "Clearing the successor flag is accepted", S.SetIsSuccessor(false)) && Passed;
    Passed = EvaluateFalse("IsSuccessor()", "false", "The cleared successor flag is stored", S.IsSuccessor()) && Passed;
    Passed = EvaluateTrue("SetSuccessor()", "Other", "A successor name is accepted", S.SetSuccessor("Other")) && Passed;
    Passed = Evaluate("GetSuccessor()", "Other", "The successor name is stored", S.GetSuccessor(), MString("Other")) && Passed;
    Passed = EvaluateTrue("SetVolume()", "Cube", "A volume is accepted", S.SetVolume("Cube")) && Passed;
    Passed = Evaluate("GetVolume()", "Cube", "The volume is stored", S.GetVolume(), MString("Cube")) && Passed;
  }

  // Isotope count: limited to what a double can count exactly
  {
    MCSource S("Isotope count");
    const double Max = pow(2.0, numeric_limits<double>::digits);
    SilenceOutput();
    bool One = S.SetIsotopeCount(1.0);
    bool MaxOK = S.SetIsotopeCount(Max);
    bool TooMuch = S.SetIsotopeCount(nextafter(Max, 2*Max));
    bool Zero = S.SetIsotopeCount(0.0);
    bool Negative = S.SetIsotopeCount(-5.0);
    RestoreOutput();
    Passed = EvaluateTrue("SetIsotopeCount()", "one", "One isotope is accepted", One) && Passed;
    Passed = EvaluateTrue("SetIsotopeCount()", "2^53", "The largest exactly countable number of isotopes is accepted", MaxOK) && Passed;
    Passed = EvaluateFalse("SetIsotopeCount()", "above 2^53", "More isotopes than a double can count are rejected", TooMuch) && Passed;
    Passed = EvaluateFalse("SetIsotopeCount()", "zero", "Zero isotopes are rejected", Zero) && Passed;
    Passed = EvaluateFalse("SetIsotopeCount()", "negative", "A negative number of isotopes is rejected", Negative) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSourceBasics::TestStartArea()
{
  bool Passed = true;

  // Parameter validation:
  {
    MCSource S("Start area");
    SilenceOutput();
    bool NoParameters = S.SetStartAreaParameters();
    bool NoRotation = S.SetStartAreaParameters(0.0, 0.0, 0.0);
    bool Rotated = S.SetStartAreaParameters(0.0, 0.0, 0.0, 0.1, 0.0, 0.0, 10.0);
    bool NoType = S.SetStartAreaParameters(0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 10.0);
    S.SetFlux(1.0);
    S.SetStartAreaType(MCSource::c_StartAreaSphere);
    bool NoRadius = S.SetStartAreaParameters(0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
    bool ZeroRadius = S.SetStartAreaParameters(0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
    bool NegativeRadius = S.SetStartAreaParameters(0.0, 0.0, 0.0, 0.0, 0.0, 0.0, -1.0);
    RestoreOutput();
    Passed = EvaluateFalse("SetStartAreaParameters()", "no parameters", "Missing parameters are rejected", NoParameters) && Passed;
    Passed = EvaluateFalse("SetStartAreaParameters()", "no rotation", "A missing rotation is rejected", NoRotation) && Passed;
    Passed = EvaluateFalse("SetStartAreaParameters()", "rotation", "A rotation is rejected since it is not implemented", Rotated) && Passed;
    Passed = EvaluateFalse("SetStartAreaParameters()", "no type", "Start area parameters without a start area type are rejected", NoType) && Passed;
    Passed = EvaluateFalse("SetStartAreaParameters()", "no radius", "A sphere without radius is rejected", NoRadius) && Passed;
    Passed = EvaluateFalse("SetStartAreaParameters()", "zero radius", "A sphere with radius zero is rejected", ZeroRadius) && Passed;
    Passed = EvaluateFalse("SetStartAreaParameters()", "negative radius", "A sphere with a negative radius is rejected", NegativeRadius) && Passed;
    Passed = EvaluateNear("GetStartAreaAverageArea()", "rejected parameters", "Rejected parameters do not set a start area", S.GetStartAreaAverageArea(), 0.0, 1e-12) && Passed;
  }

  // Sphere:
  for (double Radius: { 1.0*cm, 37.5*cm, 400.0*cm }) {
    MCSource S("Sphere");
    SilenceOutput();
    S.SetFlux(1.0);
    S.SetStartAreaType(MCSource::c_StartAreaSphere);
    bool OK = S.SetStartAreaParameters(1.0*cm, 2.0*cm, 3.0*cm, 0.0, 0.0, 0.0, Radius);
    RestoreOutput();
    Passed = EvaluateTrue("SetStartAreaParameters()", MString("sphere ") + Radius/cm + " cm", "A start sphere is accepted", OK) && Passed;
    Passed = EvaluateNear("GetStartAreaAverageArea()", MString("sphere ") + Radius/cm + " cm", "The average start area of a sphere is pi r^2", S.GetStartAreaAverageArea(), c_Pi*Radius*Radius, 1e-9*c_Pi*Radius*Radius) && Passed;
  }

  // Tube - radius R and height H, the projected area of the code is A(theta) = 4 R (H sin(theta) + R |cos(theta)|):
  const double R = 30*cm;
  const double H = 80*cm;
  auto Area = [R, H](double Theta) { return 4*R*(H*sin(Theta) + R*fabs(cos(Theta))); };

  // Tube parameter validation:
  {
    MCSource S("Tube");
    SilenceOutput();
    S.SetBeamType(MCSource::c_FarField, MCSource::c_FarFieldPoint);
    S.SetPosition(0.7, 0.3);
    S.SetFlux(1.0);
    S.SetStartAreaType(MCSource::c_StartAreaTube);
    bool NoHeight = S.SetStartAreaParameters(0.0, 0.0, 0.0, 0.0, 0.0, 0.0, R);
    bool ZeroHeight = S.SetStartAreaParameters(0.0, 0.0, 0.0, 0.0, 0.0, 0.0, R, 0.0);
    bool ZeroRadius = S.SetStartAreaParameters(0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, H);
    bool OK = S.SetStartAreaParameters(0.0, 0.0, 0.0, 0.0, 0.0, 0.0, R, H);
    RestoreOutput();
    Passed = EvaluateFalse("SetStartAreaParameters()", "tube without height", "A tube without height is rejected", NoHeight) && Passed;
    Passed = EvaluateFalse("SetStartAreaParameters()", "tube with zero height", "A tube with height zero is rejected", ZeroHeight) && Passed;
    Passed = EvaluateFalse("SetStartAreaParameters()", "tube with zero radius", "A tube with radius zero is rejected", ZeroRadius) && Passed;
    Passed = EvaluateTrue("SetStartAreaParameters()", "tube", "A valid tube is accepted", OK) && Passed;
    Passed = EvaluateNear("GetStartAreaAverageArea()", "tube, point source", "The start area for a point source is the projected area for its theta", S.GetStartAreaAverageArea(), Area(0.7), 1e-9*Area(0.7)) && Passed;
  }

  // Isotropic source: average of the projected area over the sphere, pi R H + 2 R^2
  {
    MCSource S("Tube isotropic");
    SilenceOutput();
    S.SetBeamType(MCSource::c_FarField, MCSource::c_FarFieldIsotropic);
    S.SetPosition();
    S.SetFlux(1.0);
    S.SetStartAreaType(MCSource::c_StartAreaTube);
    bool OK = S.SetStartAreaParameters(0.0, 0.0, 0.0, 0.0, 0.0, 0.0, R, H);
    RestoreOutput();
    const double Expected = c_Pi*R*H + 2*R*R;
    Passed = EvaluateTrue("SetStartAreaParameters()", "tube isotropic", "A tube for an isotropic source is accepted", OK) && Passed;
    Passed = EvaluateNear("GetStartAreaAverageArea()", "tube isotropic", "The average start area is the average of the projected area over the sphere", S.GetStartAreaAverageArea(), Expected, 1e-3*Expected) && Passed;
  }

  // Area source: average of the projected area over the solid angle of the source - independent of the azimuth range
  {
    const double Theta1 = 20*deg;
    const double Theta2 = 70*deg;
    const double Expected = Integrate([&Area](double Theta) { return Area(Theta)*sin(Theta); }, Theta1, Theta2)/(cos(Theta1) - cos(Theta2));
    for (double PhiMax: { 360*deg, 90*deg }) {
      MCSource S("Tube area");
      SilenceOutput();
      S.SetBeamType(MCSource::c_FarField, MCSource::c_FarFieldArea);
      S.SetPosition(Theta1, Theta2, 0.0, PhiMax);
      S.SetFlux(1.0);
      S.SetStartAreaType(MCSource::c_StartAreaTube);
      bool OK = S.SetStartAreaParameters(0.0, 0.0, 0.0, 0.0, 0.0, 0.0, R, H);
      RestoreOutput();
      Passed = EvaluateTrue("SetStartAreaParameters()", MString("tube area, phi to ") + PhiMax/deg + " deg", "A tube for an area source is accepted", OK) && Passed;
      Passed = EvaluateNear("GetStartAreaAverageArea()", MString("tube area, phi to ") + PhiMax/deg + " deg", "The average start area does not depend on the azimuth range of the area source", S.GetStartAreaAverageArea(), Expected, 1e-3*Expected) && Passed;
    }
  }

  // Beam types without a tube implementation are rejected:
  for (int Beam: { MCSource::c_FarFieldGaussian, MCSource::c_FarFieldDisk, MCSource::c_FarFieldAssymetricGaussian }) {
    MCSource S("Tube unsupported");
    SilenceOutput();
    S.SetBeamType(MCSource::c_FarField, Beam);
    S.SetPosition(0.5, 0.5, 0.1, 0.1, 0.0);
    S.SetFlux(1.0);
    S.SetStartAreaType(MCSource::c_StartAreaTube);
    bool Result = S.SetStartAreaParameters(0.0, 0.0, 0.0, 0.0, 0.0, 0.0, R, H);
    RestoreOutput();
    Passed = EvaluateFalse("SetStartAreaParameters()", MString("tube, beam ") + Beam, "A tube is rejected for a beam type which does not support it", Result) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSourceBasics::TestSetPositionValidation()
{
  bool Passed = true;

  Passed = EvaluateTrue("InitializeEnvironment()", "volumes", "The Geant4 environment can be created", InitializeEnvironment()) && Passed;

  //! One case: a beam type, its parameters, and the expected result
  struct Case {
    int m_Coordinates;
    int m_Beam;
    vector<double> m_P;
    bool m_Expected;
    const char* m_Description;
  };
  const int Far = MCSource::c_FarField;
  const int Near = MCSource::c_NearField;
  const vector<Case> Cases = {
    { Far, MCSource::c_FarFieldPoint, { 30*deg, 45*deg }, true, "FarFieldPoint with a valid direction" },
    { Far, MCSource::c_FarFieldPoint, { 0.0, 0.0 }, true, "FarFieldPoint with theta 0" },
    { Far, MCSource::c_FarFieldPoint, { 180*deg, 0.0 }, true, "FarFieldPoint with theta 180 deg" },
    { Far, MCSource::c_FarFieldPoint, { -1*deg, 0.0 }, false, "FarFieldPoint with a negative theta" },
    { Far, MCSource::c_FarFieldPoint, { 181*deg, 0.0 }, false, "FarFieldPoint with theta above 180 deg" },
    { Far, MCSource::c_FarFieldArea, { 10*deg, 80*deg, 0.0, 360*deg }, true, "FarFieldArea with a valid range" },
    { Far, MCSource::c_FarFieldArea, { -1*deg, 80*deg, 0.0, 360*deg }, false, "FarFieldArea with a negative minimum theta" },
    { Far, MCSource::c_FarFieldArea, { 10*deg, 181*deg, 0.0, 360*deg }, false, "FarFieldArea with a maximum theta above 180 deg" },
    { Far, MCSource::c_FarFieldArea, { 80*deg, 10*deg, 0.0, 360*deg }, false, "FarFieldArea with maximum theta below minimum theta" },
    { Far, MCSource::c_FarFieldArea, { 10*deg, 80*deg, 90*deg, 90*deg }, false, "FarFieldArea with maximum phi equal to minimum phi" },
    { Far, MCSource::c_FarFieldGaussian, { 30*deg, 45*deg, 5*deg }, true, "FarFieldGaussian with a valid sigma" },
    { Far, MCSource::c_FarFieldGaussian, { 200*deg, 45*deg, 5*deg }, false, "FarFieldGaussian with theta above 180 deg" },
    { Far, MCSource::c_FarFieldGaussian, { 30*deg, 45*deg, -5*deg }, false, "FarFieldGaussian with a negative sigma" },
    { Far, MCSource::c_FarFieldDisk, { 30*deg, 45*deg, 5*deg }, true, "FarFieldDisk with a valid radius" },
    { Far, MCSource::c_FarFieldDisk, { -30*deg, 45*deg, 5*deg }, false, "FarFieldDisk with a negative theta" },
    { Far, MCSource::c_FarFieldDisk, { 30*deg, 45*deg, -5*deg }, false, "FarFieldDisk with a negative radius" },
    { Far, MCSource::c_FarFieldAssymetricGaussian, { 30*deg, 45*deg, 5*deg, 3*deg, 0.0 }, true, "FarFieldAssymetricGaussian with valid sigmas" },
    { Far, MCSource::c_FarFieldAssymetricGaussian, { 30*deg, 45*deg, -5*deg, 3*deg, 0.0 }, false, "FarFieldAssymetricGaussian with a negative first sigma" },
    { Far, MCSource::c_FarFieldAssymetricGaussian, { 30*deg, 45*deg, 5*deg, -3*deg, 0.0 }, false, "FarFieldAssymetricGaussian with a negative second sigma" },
    { Near, MCSource::c_NearFieldPoint, { 1*cm, 2*cm, 3*cm }, true, "NearFieldPoint" },
    { Near, MCSource::c_NearFieldDiffractionPoint, { 0.0, 0.0, 0.0, 0.0, 0.0, 1.0 }, true, "DiffractionPoint with a normal vector" },
    { Near, MCSource::c_NearFieldDiffractionPoint, { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 }, false, "DiffractionPoint with a zero normal vector" },
    { Near, MCSource::c_NearFieldLine, { 0.0, 0.0, 0.0, 10*cm, 0.0, 0.0 }, true, "NearFieldLine with different end points" },
    { Near, MCSource::c_NearFieldLine, { 1*cm, 2*cm, 3*cm, 1*cm, 2*cm, 3*cm }, false, "NearFieldLine with identical end points" },
    { Near, MCSource::c_NearFieldBox, { 0.0, 0.0, 0.0, 10*cm, 10*cm, 10*cm }, true, "NearFieldBox" },
    { Near, MCSource::c_NearFieldBox, { 1*cm, 2*cm, 3*cm, 1*cm, 2*cm, 3*cm }, false, "NearFieldBox with identical corners" },
    { Near, MCSource::c_NearFieldSphere, { 0.0, 0.0, 0.0, 1*cm, 2*cm, 3*cm }, true, "NearFieldSphere with valid radii" },
    { Near, MCSource::c_NearFieldSphere, { 0.0, 0.0, 0.0, 0.0, 2*cm, 3*cm }, false, "NearFieldSphere with a zero x radius" },
    { Near, MCSource::c_NearFieldSphere, { 0.0, 0.0, 0.0, 1*cm, -2*cm, 3*cm }, false, "NearFieldSphere with a negative y radius" },
    { Near, MCSource::c_NearFieldSphere, { 0.0, 0.0, 0.0, 1*cm, 2*cm, 0.0 }, false, "NearFieldSphere with a zero z radius" },
    { Near, MCSource::c_NearFieldDisk, { 0, 0, 0, 0, 0, 1, 1*cm, 5*cm, 2*cm, 0.0, 360*deg }, true, "NearFieldDisk with valid radii" },
    { Near, MCSource::c_NearFieldDisk, { 0, 0, 0, 0, 0, 1, -1*cm, 5*cm, 2*cm, 0.0, 360*deg }, false, "NearFieldDisk with a negative inner radius" },
    { Near, MCSource::c_NearFieldDisk, { 0, 0, 0, 0, 0, 1, 0.0, 5*cm, 2*cm, 0.0, 360*deg }, true, "NearFieldDisk with inner radius zero" },
    { Near, MCSource::c_NearFieldDisk, { 0, 0, 0, 0, 0, 1, 0.0, 0.0, 2*cm, 0.0, 360*deg }, false, "NearFieldDisk with outer radius zero" },
    { Near, MCSource::c_NearFieldDisk, { 0, 0, 0, 0, 0, 1, 1*cm, 5*cm, 0.0, 0.0, 360*deg }, false, "NearFieldDisk with height zero" },
    { Near, MCSource::c_NearFieldDisk, { 0, 0, 0, 0, 0, 1, 5*cm, 5*cm, 2*cm, 0.0, 360*deg }, false, "NearFieldDisk with inner radius equal to outer radius" },
    { Near, MCSource::c_NearFieldBeam, { 0, 0, 0, 0, 0, 1, 2*cm }, true, "NearFieldBeam" },
    { Near, MCSource::c_NearFieldBeam, { 0, 0, 0, 0, 0, 0, 2*cm }, false, "NearFieldBeam with a zero direction" },
    { Near, MCSource::c_NearFieldBeam, { 0, 0, 0, 0, 0, 1, 0.0 }, false, "NearFieldBeam with radius zero" },
    { Near, MCSource::c_NearFieldBeam1DProfile, { 0, 0, 0, 0, 0, 0 }, false, "NearFieldBeam1DProfile with a zero direction" },
    { Near, MCSource::c_NearFieldBeam2DProfile, { 0, 0, 0, 0, 0, 1 }, true, "NearFieldBeam2DProfile with a direction" },
    { Near, MCSource::c_NearFieldFlatMap, { 0, 0, 0, 0, 0, 0 }, false, "NearFieldFlatMap with a zero direction" },
    { Near, MCSource::c_NearFieldConeBeam, { 0, 0, 0, 0, 0, 1, 10*deg }, true, "NearFieldConeBeam" },
    { Near, MCSource::c_NearFieldConeBeam, { 0, 0, 0, 0, 0, 0, 10*deg }, false, "NearFieldConeBeam with a zero direction" },
    { Near, MCSource::c_NearFieldConeBeam, { 0, 0, 0, 0, 0, 1, 0.0 }, false, "NearFieldConeBeam with opening angle zero" },
    { Near, MCSource::c_NearFieldConeBeamGauss, { 0, 0, 0, 0, 0, 1, 10*deg, 3*deg }, true, "NearFieldConeBeamGauss" },
    { Near, MCSource::c_NearFieldConeBeamGauss, { 0, 0, 0, 0, 0, 1, 10*deg, 0.0 }, false, "NearFieldConeBeamGauss with sigma zero" },
    { Near, MCSource::c_NearFieldConeBeamGauss, { 0, 0, 0, 0, 0, 1, 0.0, 3*deg }, false, "NearFieldConeBeamGauss with opening angle zero" },
    { Near, MCSource::c_NearFieldFanBeam, { 0, 0, 0, 1, 0, 0, 0, 1, 0, 0.5*cm }, true, "NearFieldFanBeam with beam width" },
    { Near, MCSource::c_NearFieldFanBeam, { 0, 0, 0, 1, 0, 0, 0, 1, 0, 0.0 }, true, "NearFieldFanBeam with beam width zero (point beam)" },
    { Near, MCSource::c_NearFieldFanBeam, { 0, 0, 0, 0, 0, 0, 0, 1, 0, 0.5*cm }, false, "NearFieldFanBeam with a zero first direction" },
    { Near, MCSource::c_NearFieldFanBeam, { 0, 0, 0, 1, 0, 0, 0, 0, 0, 0.5*cm }, false, "NearFieldFanBeam with a zero second direction" },
    { Near, MCSource::c_NearFieldFanBeam, { 0, 0, 0, 1, 0, 0, 0, 1, 0, -0.5*cm }, false, "NearFieldFanBeam with a negative beam width" },
    { Near, MCSource::c_NearFieldIlluminatedDisk, { 0, 0, 0, 5*cm, 30*deg, 0, 40*deg, 60*deg }, true, "IlluminatedDisk" },
    { Near, MCSource::c_NearFieldIlluminatedDisk, { 0, 0, 0, 0.0, 30*deg, 0, 40*deg, 60*deg }, false, "IlluminatedDisk with radius zero" },
    { Near, MCSource::c_NearFieldIlluminatedDisk, { 0, 0, 0, 5*cm, 181*deg, 0, 40*deg, 60*deg }, false, "IlluminatedDisk with a disk theta above 180 deg" },
    { Near, MCSource::c_NearFieldIlluminatedDisk, { 0, 0, 0, 5*cm, 30*deg, 0, -1*deg, 60*deg }, false, "IlluminatedDisk with a negative beam theta" },
    { Near, MCSource::c_NearFieldIlluminatedSquare, { 0, 0, 0, 5*cm, 30*deg, 0, 40*deg, 60*deg }, true, "IlluminatedSquare" },
    { Near, MCSource::c_NearFieldIlluminatedSquare, { 0, 0, 0, -5*cm, 30*deg, 0, 40*deg, 60*deg }, false, "IlluminatedSquare with a negative half length" },
    { Near, MCSource::c_NearFieldIlluminatedSquare, { 0, 0, 0, 5*cm, -1*deg, 0, 40*deg, 60*deg }, false, "IlluminatedSquare with a negative box theta" },
    { Near, MCSource::c_NearFieldIlluminatedSquare, { 0, 0, 0, 5*cm, 30*deg, 0, 181*deg, 60*deg }, false, "IlluminatedSquare with a beam theta above 180 deg" }
  };
  for (const Case& C: Cases) {
    MCSource S("Position");
    SilenceOutput();
    S.SetBeamType(C.m_Coordinates, C.m_Beam);
    bool Result = S.SetPosition(C.m_P.size() > 0 ? C.m_P[0] : MCSource::c_Invalid, C.m_P.size() > 1 ? C.m_P[1] : MCSource::c_Invalid,
                                C.m_P.size() > 2 ? C.m_P[2] : MCSource::c_Invalid, C.m_P.size() > 3 ? C.m_P[3] : MCSource::c_Invalid,
                                C.m_P.size() > 4 ? C.m_P[4] : MCSource::c_Invalid, C.m_P.size() > 5 ? C.m_P[5] : MCSource::c_Invalid,
                                C.m_P.size() > 6 ? C.m_P[6] : MCSource::c_Invalid, C.m_P.size() > 7 ? C.m_P[7] : MCSource::c_Invalid,
                                C.m_P.size() > 8 ? C.m_P[8] : MCSource::c_Invalid, C.m_P.size() > 9 ? C.m_P[9] : MCSource::c_Invalid,
                                C.m_P.size() > 10 ? C.m_P[10] : MCSource::c_Invalid);
    RestoreOutput();
    Passed = Evaluate("SetPosition()", C.m_Description, C.m_Expected == true ? "The position is accepted" : "The position is rejected", Result, C.m_Expected) && Passed;
  }

  // Volumes must exist in the geometry:
  for (int Beam: { MCSource::c_NearFieldActivation, MCSource::c_NearFieldVolume }) {
    MCSource S("Volume");
    SilenceOutput();
    S.SetBeamType(Near, Beam);
    S.SetVolume("Cube");
    bool Valid = S.SetPosition();
    S.SetVolume("DoesNotExist");
    bool Invalid = S.SetPosition();
    RestoreOutput();
    Passed = EvaluateTrue("SetPosition()", MString("beam ") + Beam + ", volume Cube", "An existing volume is accepted", Valid) && Passed;
    Passed = EvaluateFalse("SetPosition()", MString("beam ") + Beam + ", missing volume", "A volume which is not in the geometry is rejected", Invalid) && Passed;
  }

  // A rejected position is reported. The setters store the values before they validate them, which is fine: a rejected value makes
  // MCParameterFile::Open() fail and cosima stops, thus a source is never used after a rejection and the stored values are not part of the contract
  {
    MCSource S("Rejected position");
    SilenceOutput();
    S.SetBeamType(Far, MCSource::c_FarFieldPoint);
    bool First = S.SetPosition(30*deg, 45*deg);
    bool Second = S.SetPosition(-10*deg, 10*deg);
    RestoreOutput();
    Passed = EvaluateTrue("SetPosition()", "first", "The first position is accepted", First) && Passed;
    Passed = EvaluateFalse("SetPosition()", "second", "The invalid second position is rejected (also after a valid first one)", Second) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSourceBasics::TestParameterStorage()
{
  bool Passed = true;

  // A disk source stores 11 parameters:
  {
    MCSource S("Disk parameters");
    SilenceOutput();
    S.SetBeamType(MCSource::c_NearField, MCSource::c_NearFieldDisk);
    bool OK = S.SetPosition(1, 2, 3, 0, 0, 1, 4, 5, 6, 7, 8);
    RestoreOutput();
    Passed = EvaluateTrue("SetPosition()", "disk", "The disk is accepted", OK) && Passed;
    for (unsigned int i = 1; i <= 11; ++i) {
      const double Expected = (i == 4 || i == 5) ? 0.0 : ((i == 6) ? 1.0 : (i < 4 ? i : i - 3));
      Passed = EvaluateNear("GetPositionParameter()", MString("parameter ") + i, "The position parameter is the one which has been set", S.GetPositionParameter(i), Expected, 1e-12) && Passed;
    }
  }

  // The position function accepts 14 parameters - all must be readable, also the ones which are derived for the fan beam:
  {
    MCSource S("Fan beam parameters");
    SilenceOutput();
    S.SetBeamType(MCSource::c_NearField, MCSource::c_NearFieldFanBeam);
    bool OK = S.SetPosition(0, 0, 0, 1, 0, 0, 0, 1, 0, 0.5);
    RestoreOutput();
    Passed = EvaluateTrue("SetPosition()", "fan beam", "The fan beam is accepted", OK) && Passed;
    Passed = EvaluateNear("GetPositionParameter()", "fan beam, parameter 11", "The angle between the two directions is stored in parameter 11", S.GetPositionParameter(11), c_Pi/2, 1e-9) && Passed;
  }

  // Energy parameters: an activation does not validate them, thus all seven can carry distinct values
  {
    MCSource S("Energy parameters");
    SilenceOutput();
    S.SetSpectralType(MCSource::c_Activation);
    bool OK = S.SetEnergy(1, 2, 3, 4, 5, 6, 7);
    RestoreOutput();
    Passed = EvaluateTrue("SetEnergy()", "activation", "Seven energy parameters are accepted", OK) && Passed;
    for (unsigned int i = 1; i <= 7; ++i) {
      Passed = EvaluateNear("GetEnergyParameter()", MString("parameter ") + i, "The energy parameter is the one which has been set", S.GetEnergyParameter(i), i, 1e-12) && Passed;
    }
  }

  // A rejected energy is reported. As for the position, the stored values after a rejection are not part of the contract: cosima stops reading the
  // parameter file at the first rejected value
  {
    MCSource S("Rejected energy");
    SilenceOutput();
    S.SetSpectralType(MCSource::c_Linear);
    bool First = S.SetEnergy(100*keV, 1000*keV);
    bool Second = S.SetEnergy(-5*keV, 1000*keV);
    RestoreOutput();
    Passed = EvaluateTrue("SetEnergy()", "first", "The first energy range is accepted", First) && Passed;
    Passed = EvaluateFalse("SetEnergy()", "second", "The invalid second energy range is rejected (also after a valid first one)", Second) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSourceBasics::TestSetEnergyValidation()
{
  bool Passed = true;

  //! One case: a spectral type, its parameters, and the expected result
  struct Case {
    int m_Type;
    vector<double> m_P;
    bool m_Expected;
    const char* m_Description;
  };
  const vector<Case> Cases = {
    { MCSource::c_Monoenergetic, { 511*keV }, true, "Monoenergetic" },
    { MCSource::c_Monoenergetic, { 0.0 }, false, "Monoenergetic with energy zero" },
    { MCSource::c_Monoenergetic, { -511*keV }, false, "Monoenergetic with a negative energy" },
    { MCSource::c_Linear, { 100*keV, 1000*keV }, true, "Linear" },
    { MCSource::c_Linear, { 0.0, 1000*keV }, false, "Linear with minimum energy zero" },
    { MCSource::c_Linear, { 1000*keV, 1000*keV }, false, "Linear with maximum equal to minimum" },
    { MCSource::c_Linear, { 1000*keV, 100*keV }, false, "Linear with maximum below minimum" },
    { MCSource::c_PowerLaw, { 10*keV, 1000*keV, 2.0 }, true, "PowerLaw" },
    { MCSource::c_PowerLaw, { 10*keV, 1000*keV, -1.0 }, true, "PowerLaw with a rising spectrum" },
    { MCSource::c_PowerLaw, { -10*keV, 1000*keV, 2.0 }, false, "PowerLaw with a negative minimum energy" },
    { MCSource::c_PowerLaw, { 10*keV, 10*keV, 2.0 }, false, "PowerLaw with maximum equal to minimum" },
    { MCSource::c_BrokenPowerLaw, { 10*keV, 1000*keV, 100*keV, 1.5, 2.5 }, true, "BrokenPowerLaw" },
    { MCSource::c_BrokenPowerLaw, { 10*keV, 1000*keV, 1000*keV, 1.5, 2.5 }, true, "BrokenPowerLaw with the break at the maximum energy" },
    { MCSource::c_BrokenPowerLaw, { 10*keV, 1000*keV, 10*keV, 1.5, 2.5 }, false, "BrokenPowerLaw with the break at the minimum energy" },
    { MCSource::c_BrokenPowerLaw, { 10*keV, 1000*keV, 2000*keV, 1.5, 2.5 }, false, "BrokenPowerLaw with the break above the maximum energy" },
    { MCSource::c_BrokenPowerLaw, { 10*keV, 1000*keV, 100*keV, 0.0, 2.5 }, false, "BrokenPowerLaw with the first index zero" },
    { MCSource::c_BrokenPowerLaw, { 10*keV, 1000*keV, 100*keV, 1.5, -2.5 }, false, "BrokenPowerLaw with a negative second index" },
    { MCSource::c_CutOffPowerLaw, { 10*keV, 1000*keV, 1.5, 300*keV }, true, "CutOffPowerLaw" },
    { MCSource::c_CutOffPowerLaw, { 10*keV, 1000*keV, 1.5, 0.0 }, false, "CutOffPowerLaw with cut-off energy zero" },
    { MCSource::c_CutOffPowerLaw, { 1000*keV, 10*keV, 1.5, 300*keV }, false, "CutOffPowerLaw with maximum below minimum" },
    { MCSource::c_Comptonized, { 10*keV, 1000*keV, -1.0, 200*keV }, true, "Comptonized" },
    { MCSource::c_Comptonized, { 10*keV, 1000*keV, -1.0, -200*keV }, false, "Comptonized with a negative peak energy" },
    { MCSource::c_Comptonized, { 0.0, 1000*keV, -1.0, 200*keV }, false, "Comptonized with minimum energy zero" },
    { MCSource::c_Gaussian, { 511*keV, 2*keV, 3.0 }, true, "Gaussian" },
    { MCSource::c_Gaussian, { 0.0, 2*keV, 3.0 }, true, "Gaussian with energy zero (negative energies are thrown again)" },
    { MCSource::c_Gaussian, { -1*keV, 2*keV, 3.0 }, false, "Gaussian with a negative energy" },
    { MCSource::c_Gaussian, { 511*keV, 0.0, 3.0 }, false, "Gaussian with sigma zero" },
    { MCSource::c_Gaussian, { 511*keV, 2*keV, 0.0 }, false, "Gaussian with cut-off zero" },
    { MCSource::c_ThermalBremsstrahlung, { 10*keV, 1000*keV, 30*keV }, true, "ThermalBremsstrahlung" },
    { MCSource::c_ThermalBremsstrahlung, { 10*keV, 1000*keV, 0.0 }, false, "ThermalBremsstrahlung with temperature zero" },
    { MCSource::c_BlackBody, { 10*keV, 1000*keV, 30*keV }, true, "BlackBody" },
    { MCSource::c_BlackBody, { 10*keV, 1000*keV, -30*keV }, false, "BlackBody with a negative temperature" },
    { MCSource::c_BlackBody, { 1000*keV, 10*keV, 30*keV }, false, "BlackBody with maximum below minimum" },
    { MCSource::c_BandFunction, { 10*keV, 1000*keV, -1.0, -2.3, 200*keV }, true, "BandFunction" },
    { MCSource::c_BandFunction, { 10*keV, 1000*keV, 0.5, -2.3, 200*keV }, true, "BandFunction with a positive low-energy index (as in the BATSE catalog)" },
    { MCSource::c_BandFunction, { 10*keV, 1000*keV, -1.0, 0.5, 200*keV }, false, "BandFunction with a positive high-energy index" },
    { MCSource::c_BandFunction, { 10*keV, 1000*keV, -2.3, -1.0, 200*keV }, false, "BandFunction with the high-energy index above the low-energy index" },
    { MCSource::c_BandFunction, { 10*keV, 1000*keV, -1.0, -2.3, 2000*keV }, true, "BandFunction with the break energy above the maximum energy" },
    { MCSource::c_BandFunction, { 10*keV, 1000*keV, -1.0, -2.3, 5*keV }, true, "BandFunction with the break energy below the minimum energy" },
    { MCSource::c_BandFunction, { 10*keV, 1000*keV, -1.0, -2.3, 0.0 }, false, "BandFunction with the break energy zero" },
    { MCSource::c_BandFunction, { 10*keV, 1000*keV, -1.0, 0.0, 200*keV }, false, "BandFunction with the high-energy index zero" },
    { MCSource::c_BandFunction, { 10*keV, 1000*keV, -1.0, -1.0, 200*keV }, false, "BandFunction with identical indices" },
    { MCSource::c_Map, { 1.0 }, true, "Map" },
    { MCSource::c_Activation, { }, true, "Activation" }
  };
  for (const Case& C: Cases) {
    MCSource S("Energy");
    SilenceOutput();
    S.SetSpectralType(C.m_Type);
    bool Result = S.SetEnergy(C.m_P.size() > 0 ? C.m_P[0] : MCSource::c_Invalid, C.m_P.size() > 1 ? C.m_P[1] : MCSource::c_Invalid,
                              C.m_P.size() > 2 ? C.m_P[2] : MCSource::c_Invalid, C.m_P.size() > 3 ? C.m_P[3] : MCSource::c_Invalid,
                              C.m_P.size() > 4 ? C.m_P[4] : MCSource::c_Invalid, C.m_P.size() > 5 ? C.m_P[5] : MCSource::c_Invalid);
    RestoreOutput();
    Passed = Evaluate("SetEnergy()", C.m_Description, C.m_Expected == true ? "The energy is accepted" : "The energy is rejected", Result, C.m_Expected) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSourceBasics::TestDerivedEnergyParameters()
{
  bool Passed = true;

  // Broken power law: factor which connects the two power laws at the break
  {
    MCSource S("Broken power law");
    SilenceOutput();
    S.SetSpectralType(MCSource::c_BrokenPowerLaw);
    S.SetEnergy(10*keV, 1000*keV, 100*keV, 1.5, 2.5);
    RestoreOutput();
    Passed = EvaluateNear("GetEnergyParameter()", "broken power law", "The connection factor is Eb^(alpha2 - alpha1)", S.GetEnergyParameter(6), pow(100*keV, 2.5 - 1.5), 1e-12) && Passed;
  }

  // Cut-off power law: boundaries of the transformed range and the maximum of the exponential
  {
    MCSource S("Cut-off power law");
    SilenceOutput();
    S.SetSpectralType(MCSource::c_CutOffPowerLaw);
    S.SetEnergy(10*keV, 1000*keV, 1.5, 300*keV);
    RestoreOutput();
    Passed = EvaluateNear("GetEnergyParameter()", "cut-off power law, 5", "Parameter 5 is Emin^(1 - alpha)", S.GetEnergyParameter(5), pow(10*keV, 1 - 1.5), 1e-9) && Passed;
    Passed = EvaluateNear("GetEnergyParameter()", "cut-off power law, 6", "Parameter 6 is Emax^(1 - alpha)", S.GetEnergyParameter(6), pow(1000*keV, 1 - 1.5), 1e-9) && Passed;
    Passed = EvaluateNear("GetEnergyParameter()", "cut-off power law, 7", "Parameter 7 is the exponential at the minimum energy", S.GetEnergyParameter(7), exp(-10.0/300.0), 1e-12) && Passed;
  }

  // Black body: maximum of E^2/(exp(E/T)-1) in the energy range
  {
    // The maximum of x^2/(exp(x)-1) is where 2*(exp(x)-1) = x*exp(x), i.e. x = 2*(1 - exp(-x)): solved here by fixed-point iteration
    // instead of copying the constant from the source (the iteration is a contraction around x = 1.5936)
    double x = 2.0;
    for (int i = 0; i < 200; ++i) x = 2.0*(1.0 - exp(-x));
    auto BlackBody = [](double E, double T) { return E*E/(exp(E/T) - 1); };
    for (double T: { 30*keV, 5*keV, 1000*keV }) {
      MCSource S("Black body");
      SilenceOutput();
      S.SetSpectralType(MCSource::c_BlackBody);
      S.SetEnergy(10*keV, 1000*keV, T);
      RestoreOutput();
      double Peak = x*T;
      double Maximum = BlackBody(Peak, T);
      if (Peak < 10*keV) Maximum = BlackBody(10*keV, T);
      if (Peak > 1000*keV) Maximum = BlackBody(1000*keV, T);
      Passed = EvaluateNear("GetEnergyParameter()", MString("black body, T = ") + T/keV + " keV", "Parameter 4 is the maximum of the black body spectrum in the energy range", S.GetEnergyParameter(4), Maximum, 1e-9*Maximum) && Passed;
    }
  }

  // Band function: maximum, i.e. the value at the minimum energy for falling spectra
  {
    MCSource S("Band");
    SilenceOutput();
    S.SetSpectralType(MCSource::c_BandFunction);
    S.SetEnergy(10*keV, 1000*keV, -1.0, -2.3, 200*keV);
    RestoreOutput();
    const double Expected = Band(10*keV, -1.0, -2.3, 200*keV);
    Passed = EvaluateNear("GetEnergyParameter()", "band", "Parameter 6 is the value of the Band function at the minimum energy", S.GetEnergyParameter(6), Expected, 1e-9*Expected) && Passed;
  }

  // Comptonized: E^alpha exp(-(alpha + 2) E/Epeak)
  {
    auto Comptonized = [](double E, double Alpha, double Peak) { return pow(E, Alpha)*exp(-(Alpha + 2)*E/Peak); };

    // Falling spectrum without peak in the range: maximum at the minimum energy, full range
    MCSource S("Comptonized");
    SilenceOutput();
    S.SetSpectralType(MCSource::c_Comptonized);
    S.SetEnergy(10*keV, 1000*keV, -1.0, 200*keV);
    RestoreOutput();
    const double Expected = Comptonized(10*keV, -1.0, 200*keV);
    Passed = EvaluateNear("GetEnergyParameter()", "comptonized, falling", "Parameter 5 is the maximum of the spectrum in the range", S.GetEnergyParameter(5), Expected, 1e-9*Expected) && Passed;
    Passed = EvaluateNear("GetEnergyParameter()", "comptonized, falling", "Parameter 6 is the minimum energy if the spectrum is not negligible there", S.GetEnergyParameter(6)/keV, 10.0, 1e-9) && Passed;
    Passed = EvaluateNear("GetEnergyParameter()", "comptonized, falling", "Parameter 7 is the maximum energy if the spectrum is not negligible there", S.GetEnergyParameter(7)/keV, 1000.0, 1e-9) && Passed;

    // Peak inside the range: alpha = 1, Epeak = 300 keV has its maximum at Epeak * alpha/(alpha + 2) = 100 keV
    MCSource P("Comptonized peak");
    SilenceOutput();
    P.SetSpectralType(MCSource::c_Comptonized);
    P.SetEnergy(10*keV, 1000*keV, 1.0, 300*keV);
    RestoreOutput();
    const double PeakValue = Comptonized(100*keV, 1.0, 300*keV);
    Passed = EvaluateNear("GetEnergyParameter()", "comptonized, peak", "Parameter 5 is the value at the peak of the spectrum", P.GetEnergyParameter(5), PeakValue, 1e-9*PeakValue) && Passed;

    // A very steep spectrum: the range is cut where the spectrum drops below 1e-15 of its maximum
    MCSource T("Comptonized steep");
    SilenceOutput();
    T.SetSpectralType(MCSource::c_Comptonized);
    T.SetEnergy(10*keV, 1000*keV, -1.0, 2*keV);
    RestoreOutput();
    const double Maximum = T.GetEnergyParameter(5);
    const double Upper = T.GetEnergyParameter(7);
    Passed = EvaluateNear("GetEnergyParameter()", "comptonized, steep, reduced", "The upper end of the range is reduced below the requested maximum (shown: the excess)", max(Upper - 1000*keV, 0.0), 0.0, 1e-9) && Passed;
    Passed = EvaluateNear("GetEnergyParameter()", "comptonized, steep, minimum", "The upper end of the range stays above the minimum energy (shown: the shortfall)", max(10*keV - Upper, 0.0), 0.0, 1e-9) && Passed;
    Passed = EvaluateNear("GetEnergyParameter()", "comptonized, steep, upper", "The spectrum at the upper end of the range is not negligible (shown: the shortfall below 1e-15)", max(1e-15 - Comptonized(Upper, -1.0, 2*keV)/Maximum, 0.0), 0.0, 0.0) && Passed;
    Passed = EvaluateNear("GetEnergyParameter()", "comptonized, steep, above", "The spectrum just above the upper end of the range is negligible (shown: the excess over 1e-15)", max(Comptonized(Upper + 1*keV, -1.0, 2*keV)/Maximum - 1e-15, 0.0), 0.0, 0.0) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSourceBasics::TestFiles()
{
  bool Passed = true;

  // Spectrum from file:
  {
    const MString Good = GetTemporaryFileName("Spectrum.dat");
    const MString Negative = GetTemporaryFileName("SpectrumNegative.dat");
    const MString Single = GetTemporaryFileName("SpectrumSingle.dat");
    Passed = EvaluateTrue("WriteTextFile()", "spectrum", "The spectrum fixture can be written", WriteTextFile(Good, "DP 100 1.0\nDP 200 2.0\nDP 300 0.5\nEN\n")) && Passed;
    Passed = EvaluateTrue("WriteTextFile()", "negative spectrum", "The negative spectrum fixture can be written", WriteTextFile(Negative, "DP 100 1.0\nDP 200 -2.0\nEN\n")) && Passed;
    Passed = EvaluateTrue("WriteTextFile()", "single spectrum", "The single entry spectrum fixture can be written", WriteTextFile(Single, "DP 100 1.0\nEN\n")) && Passed;

    MCSource Wrong("Wrong spectral type");
    SilenceOutput();
    Wrong.SetSpectralType(MCSource::c_Monoenergetic);
    bool WrongType = Wrong.SetEnergy(Good);
    RestoreOutput();
    Passed = EvaluateFalse("SetEnergy()", "file, wrong spectral type", "A spectrum file is only accepted for the file spectral type", WrongType) && Passed;

    MCSource S("Spectrum file");
    SilenceOutput();
    S.SetSpectralType(MCSource::c_FileDifferentialFlux);
    bool Missing = S.SetEnergy(GetTemporaryFileName("Missing.dat"));
    bool NegativeValues = S.SetEnergy(Negative);
    bool SingleEntry = S.SetEnergy(Single);
    bool OK = S.SetEnergy(Good);
    RestoreOutput();
    Passed = EvaluateFalse("SetEnergy()", "missing spectrum file", "A missing spectrum file is rejected", Missing) && Passed;
    Passed = EvaluateFalse("SetEnergy()", "negative values", "A spectrum with negative values is rejected", NegativeValues) && Passed;
    Passed = EvaluateFalse("SetEnergy()", "single entry", "A spectrum with less than two entries is rejected", SingleEntry) && Passed;
    Passed = EvaluateTrue("SetEnergy()", "valid spectrum file", "A valid spectrum file is accepted", OK) && Passed;
  }

  // Zenith dependent beam from file:
  {
    const MString Good = GetTemporaryFileName("Zenith.dat");
    const MString Zero = GetTemporaryFileName("ZenithZero.dat");
    Passed = EvaluateTrue("WriteTextFile()", "zenith", "The zenith fixture can be written", WriteTextFile(Good, "DP 0 1.0\nDP 45 0.5\nDP 90 0.1\nEN\n")) && Passed;
    Passed = EvaluateTrue("WriteTextFile()", "zero zenith", "The zero zenith fixture can be written", WriteTextFile(Zero, "DP 0 0.0\nDP 90 0.0\nEN\n")) && Passed;

    MCSource Wrong("Wrong beam");
    SilenceOutput();
    Wrong.SetBeamType(MCSource::c_FarField, MCSource::c_FarFieldPoint);
    bool WrongBeam = Wrong.SetPosition(Good);
    RestoreOutput();
    Passed = EvaluateFalse("SetPosition()", "file, wrong beam type", "A beam file is rejected for a beam type without file", WrongBeam) && Passed;

    MCSource S("Zenith file");
    SilenceOutput();
    S.SetBeamType(MCSource::c_FarField, MCSource::c_FarFieldFileZenithDependent);
    bool Missing = S.SetPosition(GetTemporaryFileName("Missing.dat"));
    bool ZeroMax = S.SetPosition(Zero);
    bool OK = S.SetPosition(Good);
    RestoreOutput();
    Passed = EvaluateFalse("SetPosition()", "missing zenith file", "A missing zenith file is rejected", Missing) && Passed;
    Passed = EvaluateFalse("SetPosition()", "zero maximum", "A zenith distribution with maximum zero is rejected", ZeroMax) && Passed;
    Passed = EvaluateTrue("SetPosition()", "valid zenith file", "A valid zenith file is accepted", OK) && Passed;
    Passed = EvaluateNear("GetPositionParameter()", "zenith file, 1", "Parameter 1 is the minimum zenith angle", S.GetPositionParameter(1)/deg, 0.0, 1e-9) && Passed;
    Passed = EvaluateNear("GetPositionParameter()", "zenith file, 2", "Parameter 2 is the maximum zenith angle", S.GetPositionParameter(2)/deg, 90.0, 1e-9) && Passed;
    Passed = EvaluateNear("GetPositionParameter()", "zenith file, 3", "Parameter 3 is the maximum of the distribution", S.GetPositionParameter(3), 1.0, 1e-9) && Passed;
  }

  // Radial profile from file:
  {
    const MString Good = GetTemporaryFileName("Profile.dat");
    Passed = EvaluateTrue("WriteTextFile()", "profile", "The profile fixture can be written", WriteTextFile(Good, "DP 0 1.0\nDP 1 0.5\nDP 2 0.0\nEN\n")) && Passed;
    MCSource S("Profile file");
    SilenceOutput();
    S.SetBeamType(MCSource::c_NearField, MCSource::c_NearFieldBeam1DProfile);
    bool Missing = S.SetPosition(GetTemporaryFileName("Missing.dat"));
    bool OK = S.SetPosition(Good);
    RestoreOutput();
    Passed = EvaluateFalse("SetPosition()", "missing profile", "A missing profile file is rejected", Missing) && Passed;
    Passed = EvaluateTrue("SetPosition()", "valid profile", "A valid radial profile file is accepted", OK) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSourceBasics::TestMeanEnergy()
{
  bool Passed = true;

  auto Configure = [this](MCSource& S, int Type, const vector<double>& P) {
    SilenceOutput();
    S.SetSpectralType(Type);
    bool OK = S.SetEnergy(P.size() > 0 ? P[0] : MCSource::c_Invalid, P.size() > 1 ? P[1] : MCSource::c_Invalid, P.size() > 2 ? P[2] : MCSource::c_Invalid,
                          P.size() > 3 ? P[3] : MCSource::c_Invalid, P.size() > 4 ? P[4] : MCSource::c_Invalid);
    RestoreOutput();
    return OK;
  };

  // Monoenergetic, linear, Gaussian:
  {
    MCSource S("Mono");
    Configure(S, MCSource::c_Monoenergetic, { 511*keV });
    Passed = EvaluateNear("GetMeanEnergy()", "mono", "The mean energy of a line is its energy", S.GetMeanEnergy()/keV, 511.0, 1e-9) && Passed;
    MCSource L("Linear");
    Configure(L, MCSource::c_Linear, { 100*keV, 1000*keV });
    Passed = EvaluateNear("GetMeanEnergy()", "linear", "The mean energy of a flat spectrum is the middle of the range", L.GetMeanEnergy()/keV, 550.0, 1e-9) && Passed;
    MCSource G("Gaussian");
    Configure(G, MCSource::c_Gaussian, { 511*keV, 2*keV, 3.0 });
    Passed = EvaluateNear("GetMeanEnergy()", "gaussian", "The mean energy of a Gaussian is its centroid", G.GetMeanEnergy()/keV, 511.0, 1e-9) && Passed;
  }

  // Power laws, including the special indices 1 and 2:
  for (double Index: { 1.0, 2.0, 1.7, 2.6, -0.5 }) {
    MCSource S("Power law");
    Configure(S, MCSource::c_PowerLaw, { 10*keV, 1000*keV, Index });
    const double Expected = MeanOfDensity([Index](double E) { return pow(E, -Index); }, 10*keV, 1000*keV);
    Passed = EvaluateNear("GetMeanEnergy()", MString("power law, index ") + Index, "The mean energy of a power law is the ratio of the integrals", S.GetMeanEnergy(), Expected, 1e-6*Expected) && Passed;
  }

  // Broken power laws, including the special indices 1 and 2 in both branches:
  const vector<pair<double, double>> Indices = { { 1.5, 2.5 }, { 2.0, 1.0 }, { 1.0, 2.0 }, { 0.5, 3.0 }, { 2.0, 2.0 }, { 1.0, 1.0 } };
  for (const auto& I: Indices) {
    MCSource S("Broken power law");
    Configure(S, MCSource::c_BrokenPowerLaw, { 10*keV, 1000*keV, 100*keV, I.first, I.second });
    const double E1 = 10*keV;
    const double Eb = 100*keV;
    const double E2 = 1000*keV;
    auto Below = [&](double E) { return pow(E/E1, -I.first); };
    auto Above = [&](double E) { return pow(Eb/E1, -I.first)*pow(E/Eb, -I.second); };
    const double Numerator = Integrate([&](double E) { return E*Below(E); }, E1, Eb) + Integrate([&](double E) { return E*Above(E); }, Eb, E2);
    const double Denominator = Integrate(Below, E1, Eb) + Integrate(Above, Eb, E2);
    const double Expected = Numerator/Denominator;
    Passed = EvaluateNear("GetMeanEnergy()", MString("broken power law, indices ") + I.first + " and " + I.second, "The mean energy of a broken power law is the ratio of the integrals of the connected power laws", S.GetMeanEnergy(), Expected, 1e-6*Expected) && Passed;
  }

  // The spectra which have no closed form for their mean: the reference is the ratio of two numerical integrals of the density
  // (a linear Simpson grid with 4000 intervals, independent of the logarithmic grid of the source), 1e-5 relative is well above its error
  {
    const double Emin = 10*keV;
    const double Emax = 1000*keV;
    auto Check = [&](const MString& Name, int Type, const vector<double>& P, const function<double(double)>& Density, double Low, double High) {
      MCSource S(Name.Data());
      Configure(S, Type, P);
      const double Expected = MeanOfDensity(Density, Low, High);
      return EvaluateNear("GetMeanEnergy()", Name, "The mean energy is the ratio of the integrals of E*f(E) and f(E)", S.GetMeanEnergy(), Expected, 1e-5*Expected);
    };

    for (const pair<double, double>& I: vector<pair<double, double>>({ { 1.5, 300*keV }, { 0.0, 100*keV }, { -1.0, 1000*keV } })) {
      Passed = Check(MString("cut-off power law, index ") + I.first + ", cut-off " + I.second/keV + " keV", MCSource::c_CutOffPowerLaw, { Emin, Emax, I.first, I.second },
                     [I](double E) { return pow(E, -I.first)*exp(-E/I.second); }, Emin, Emax) && Passed;
    }
    for (const pair<double, double>& I: vector<pair<double, double>>({ { -1.0, 200*keV }, { 1.0, 300*keV }, { -1.5, 100*keV } })) {
      // The generator uses the range in which the spectrum is not negligible (parameters 6 and 7):
      MCSource Probe("Comptonized range");
      Configure(Probe, MCSource::c_Comptonized, { Emin, Emax, I.first, I.second });
      Passed = Check(MString("comptonized, alpha ") + I.first + ", peak " + I.second/keV + " keV", MCSource::c_Comptonized, { Emin, Emax, I.first, I.second },
                     [I](double E) { return pow(E, I.first)*exp(-(I.first + 2)*E/I.second); }, Probe.GetEnergyParameter(6), Probe.GetEnergyParameter(7)) && Passed;
    }
    for (double T: { 30*keV, 300*keV }) {
      Passed = Check(MString("thermal bremsstrahlung, T ") + T/keV + " keV", MCSource::c_ThermalBremsstrahlung, { Emin, Emax, T },
                     [T](double E) { return exp(-E/T)/E; }, Emin, Emax) && Passed;
    }
    for (double T: { 30*keV, 5*keV, 2000*keV }) {
      Passed = Check(MString("black body, T ") + T/keV + " keV", MCSource::c_BlackBody, { Emin, Emax, T },
                     [T](double E) { return E*E/(exp(E/T) - 1); }, Emin, Emax) && Passed;
    }
    for (const vector<double>& I: vector<vector<double>>({ { -1.0, -2.3, 200 }, { -0.5, -1.5, 200 }, { -1.9, -3.0, 200 }, { 0.5, -2.5, 200 }, { 2.0, -3.0, 100 }, { 1.0, -2.5, 2000 } })) {
      const double Alpha = I[0];
      const double Beta = I[1];
      const double E0 = I[2]*keV;
      const double Break = (Alpha - Beta)*E0;
      Passed = Check(MString("band, alpha ") + Alpha + ", beta " + Beta + ", E0 " + I[2] + " keV", MCSource::c_BandFunction, { Emin, Emax, Alpha, Beta, E0 },
                     [=](double E) { return (E <= Break) ? pow(E, Alpha)*exp(-E/E0) : pow(Break, Alpha - Beta)*exp(Beta - Alpha)*pow(E, Beta); }, Emin, Emax) && Passed;
    }
    // The Gaussian is restricted to its cut-off window and to positive energies (window from 0 to 140 keV):
    Passed = Check("gaussian, window reaching below zero", MCSource::c_Gaussian, { 50*keV, 30*keV, 3.0 },
                   [](double E) { return exp(-0.5*(E - 50*keV)*(E - 50*keV)/(30*keV*30*keV)); }, 0.0, 140*keV) && Passed;
  }

  // Gaussian centered at zero: half Gaussian cut at c sigma, closed form of the mean: sigma*(1 - exp(-c^2/2)) / (sqrt(pi/2)*erf(c/sqrt(2)))
  {
    const double Sigma = 100*keV;
    const double CutOff = 5.0;
    MCSource S("Half Gaussian");
    Configure(S, MCSource::c_Gaussian, { 0.0, Sigma, CutOff });
    const double Expected = Sigma*(1.0 - exp(-CutOff*CutOff/2.0))/(sqrt(M_PI/2.0)*erf(CutOff/sqrt(2.0)));
    Passed = EvaluateNear("GetMeanEnergy()", "gaussian, centered at zero", "The mean energy of the half Gaussian has a closed form", S.GetMeanEnergy(), Expected, 1e-9*Expected) && Passed;
  }

  // File spectrum with linear interpolation: per segment the integrals of f and E*f are exact
  {
    const MString File = GetTemporaryFileName("MeanEnergySpectrum.dat");
    Passed = EvaluateTrue("WriteTextFile()", "spectrum", "The spectrum fixture can be written", WriteTextFile(File, "DP 100 1.0\nDP 200 3.0\nDP 400 0.5\nDP 1000 0.5\nEN\n")) && Passed;
    MCSource S("File spectrum");
    SilenceOutput();
    S.SetSpectralType(MCSource::c_FileDifferentialFlux);
    bool OK = S.SetEnergy(File);
    RestoreOutput();
    Passed = EvaluateTrue("SetEnergy()", "file spectrum", "The spectrum file is accepted", OK) && Passed;
    const double X[4] = { 100.0, 200.0, 400.0, 1000.0 };
    const double Y[4] = { 1.0, 3.0, 0.5, 0.5 };
    double Area = 0.0, Moment = 0.0;
    for (unsigned int i = 0; i < 3; ++i) {
      const double dx = X[i+1] - X[i];
      Area += 0.5*(Y[i] + Y[i+1])*dx;
      Moment += dx/6.0*(2*X[i]*Y[i] + X[i]*Y[i+1] + X[i+1]*Y[i] + 2*X[i+1]*Y[i+1]);
    }
    Passed = EvaluateNear("GetMeanEnergy()", "file spectrum", "The mean energy of a linearly interpolated spectrum is the exact ratio of the integrals", S.GetMeanEnergy()/keV, Moment/Area, 1e-6*Moment/Area) && Passed;
  }

  // A narrow line in a file spectrum (2 eV wide at 500 keV, zero elsewhere): between the table points the spectrum is linear, thus the mean energy of
  // the symmetric triangle is its center - a fixed integration grid steps over such a feature
  {
    const MString File = GetTemporaryFileName("MeanEnergyNarrowLine.dat");
    Passed = EvaluateTrue("WriteTextFile()", "narrow line", "The spectrum fixture can be written", WriteTextFile(File, "DP 100 0.0\nDP 500 0.0\nDP 500.001 1.0\nDP 500.002 0.0\nDP 1000 0.0\nEN\n")) && Passed;
    MCSource S("Narrow line");
    SilenceOutput();
    S.SetSpectralType(MCSource::c_FileDifferentialFlux);
    bool OK = S.SetEnergy(File);
    RestoreOutput();
    Passed = EvaluateTrue("SetEnergy()", "narrow line", "The spectrum file is accepted", OK) && Passed;
    Passed = EvaluateNear("GetMeanEnergy()", "narrow line", "The mean energy of a narrow line is its center", S.GetMeanEnergy()/keV, 500.001, 1e-6) && Passed;
  }

  // Spectra without a defined mean energy return zero:
  {
    MCSource S("Activation");
    SilenceOutput();
    S.SetSpectralType(MCSource::c_Activation);
    S.SetEnergy();
    const double Mean = S.GetMeanEnergy();
    RestoreOutput();
    Passed = EvaluateNear("GetMeanEnergy()", "activation", "A spectrum without a defined mean energy returns zero", Mean, 0.0, 1e-12) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSourceBasics::TestRandomGuard()
{
  bool Passed = true;

  // The guard must restore the complete state of both generators, not just the seeds: after a guarded scope in which random numbers were drawn,
  // the next numbers are the ones which would have been drawn without the scope
  auto Draw = []() { return vector<double>({ gRandom->Rndm(), gRandom->Rndm(), CLHEP::HepRandom::getTheEngine()->flat(), CLHEP::HepRandom::getTheEngine()->flat() }); };

  // Both runs start from the same advanced state (seeded, then 7 draws of each generator). The reference continues without a guard, the other run draws inside
  // a guard first. A restoration of only the seeds would restart the sequences instead of continuing them
  auto Prepare = [&Draw]() {
    gRandom->SetSeed(123);
    CLHEP::HepRandom::setTheSeed(456);
    for (unsigned int i = 0; i < 7; ++i) Draw();
  };

  Prepare();
  const vector<double> Reference = Draw();

  Prepare();
  vector<double> Inside;
  {
    RandomGuard Guard(999);
    for (unsigned int i = 0; i < 50; ++i) Inside = Draw();
  }
  const vector<double> After = Draw();
  Passed = EvaluateTrue("RandomGuard", "inside", "Inside the guard the numbers differ from the unguarded sequence", Inside[0] != Reference[0] && Inside[2] != Reference[2]) && Passed;
  for (unsigned int i = 0; i < 4; ++i) {
    Passed = EvaluateNear("RandomGuard", MString("restored ") + (i < 2 ? "ROOT " : "CLHEP ") + i, "After the guard the generators continue as if the guard had not been there", After[i], Reference[i], 1e-15) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSourceBasics::TestStrings()
{
  bool Passed = true;

  // Names of the spectral types:
  const vector<pair<int, MString>> SpectralNames = {
    { MCSource::c_Map, "Map" }, { MCSource::c_Monoenergetic, "Mono" }, { MCSource::c_Linear, "Linear" }, { MCSource::c_PowerLaw, "PowerLaw" },
    { MCSource::c_BrokenPowerLaw, "BrokenPowerLaw" }, { MCSource::c_CutOffPowerLaw, "CutOffPowerLaw" }, { MCSource::c_Comptonized, "Comptonized" },
    { MCSource::c_Gaussian, "Gaussian" }, { MCSource::c_ThermalBremsstrahlung, "ThermalBremsstrahlung" }, { MCSource::c_BlackBody, "BlackBody" },
    { MCSource::c_BandFunction, "BandFunction" }, { MCSource::c_FileDifferentialFlux, "FileDifferentialFlux" }, { MCSource::c_Activation, "Activation" },
    { MCSource::c_NormalizedEnergyBeamFluxFunction, "NormalizedEnergyBeamFluxFunction" }
  };
  for (const auto& N: SpectralNames) {
    MCSource S("Names");
    SilenceOutput();
    S.SetSpectralType(N.first);
    RestoreOutput();
    Passed = Evaluate("GetSpectralTypeAsString()", N.second, "The spectral type has its name", MString(S.GetSpectralTypeAsString()), N.second) && Passed;
  }
  MCSource Unknown("Unknown");
  Passed = Evaluate("GetSpectralTypeAsString()", "not set", "A spectral type which is not set has no name", MString(Unknown.GetSpectralTypeAsString()), MString("")) && Passed;
  Passed = Evaluate("GetSpectralAsString()", "not set", "A spectral type which is not set has no description", MString(Unknown.GetSpectralAsString()), MString("")) && Passed;
  Passed = Evaluate("GetBeamTypeAsString()", "not set", "A beam type which is not set has no name", MString(Unknown.GetBeamTypeAsString()), MString("")) && Passed;
  Passed = Evaluate("GetBeamAsString()", "not set", "A beam type which is not set has no description", MString(Unknown.GetBeamAsString()), MString("")) && Passed;

  // Spectrum descriptions with the units of the source file (energies in keV):
  const vector<tuple<int, vector<double>, MString>> Spectra = {
    { MCSource::c_Monoenergetic, { 511*keV }, "Mono 511" },
    { MCSource::c_Linear, { 100*keV, 1000*keV }, "Linear 100 1000" },
    { MCSource::c_PowerLaw, { 10*keV, 1000*keV, 2.0 }, "PowerLaw 10 1000 2" },
    { MCSource::c_BrokenPowerLaw, { 10*keV, 1000*keV, 100*keV, 1.5, 2.5 }, "BrokenPowerLaw 10 1000 100 1.5 2.5" },
    { MCSource::c_CutOffPowerLaw, { 10*keV, 1000*keV, 1.5, 300*keV }, "CutOffPowerLaw 10 1000 1.5 300" },
    { MCSource::c_Comptonized, { 10*keV, 1000*keV, -1.0, 200*keV }, "Comptonized 10 1000 -1 200" },
    { MCSource::c_Gaussian, { 511*keV, 2*keV, 3.0 }, "Gaussian 511 2 3" },
    { MCSource::c_ThermalBremsstrahlung, { 10*keV, 1000*keV, 30*keV }, "ThermalBremsstrahlung 10 1000 30" },
    { MCSource::c_BlackBody, { 10*keV, 1000*keV, 30*keV }, "BlackBody 10 1000 30" },
    { MCSource::c_BandFunction, { 10*keV, 1000*keV, -1.0, -2.3, 200*keV }, "BandFunction 10 1000 -1 -2.3 200" }
  };
  for (const auto& E: Spectra) {
    MCSource S("Spectrum");
    SilenceOutput();
    S.SetSpectralType(get<0>(E));
    const vector<double>& P = get<1>(E);
    S.SetEnergy(P.size() > 0 ? P[0] : MCSource::c_Invalid, P.size() > 1 ? P[1] : MCSource::c_Invalid, P.size() > 2 ? P[2] : MCSource::c_Invalid,
                P.size() > 3 ? P[3] : MCSource::c_Invalid, P.size() > 4 ? P[4] : MCSource::c_Invalid);
    RestoreOutput();
    Passed = Evaluate("GetSpectralAsString()", get<2>(E), "The spectrum is described like in the source file", MString(S.GetSpectralAsString()), get<2>(E)) && Passed;
  }

  // Beam names:
  const vector<pair<int, MString>> BeamNames = {
    { MCSource::c_NearFieldPoint, "Point" }, { MCSource::c_NearFieldLine, "Line" }, { MCSource::c_NearFieldBox, "Box" }, { MCSource::c_NearFieldDisk, "Disk" },
    { MCSource::c_NearFieldSphere, "Sphere" }, { MCSource::c_NearFieldBeam, "HomogeneousBeam" }, { MCSource::c_NearFieldActivation, "Activation" },
    { MCSource::c_NearFieldRestrictedPoint, "RestrictedPoint" }, { MCSource::c_NearFieldRestrictedLine, "RestrictedLine" },
    { MCSource::c_NearFieldDiffractionPoint, "DiffractionPointSource" },
    { MCSource::c_NearFieldBeam1DProfile, "RadialProfileBeam" }, { MCSource::c_NearFieldBeam2DProfile, "MapProfileBeam" }, { MCSource::c_NearFieldConeBeam, "ConeBeam" },
    { MCSource::c_NearFieldConeBeamGauss, "GaussianConeBeam" }, { MCSource::c_NearFieldFanBeam, "FanBeam" }, { MCSource::c_NearFieldIlluminatedDisk, "IlluminatedDisk" },
    { MCSource::c_NearFieldIlluminatedSquare, "IlluminatedSquare" }, { MCSource::c_NearFieldVolume, "Volume" }, { MCSource::c_NearFieldFlatMap, "Map" },
    { MCSource::c_NearFieldReverseDirectionToPredecessor, "ReverseDirectionToPredecessor" },
    { MCSource::c_FarFieldPoint, "FarFieldPointSource" }, { MCSource::c_FarFieldArea, "FarFieldAreaSource" }, { MCSource::c_FarFieldGaussian, "FarFieldGaussian" },
    { MCSource::c_FarFieldAssymetricGaussian, "FarFieldAssymetricGaussian" }, { MCSource::c_FarFieldFileZenithDependent, "FarFieldFileZenithDependent" },
    { MCSource::c_FarFieldEarthOccultation, "FarFieldEarthOccultation" }, { MCSource::c_FarFieldNormalizedEnergyBeamFluxFunction, "FarFieldNormalizedEnergyBeamFluxFunction" },
    { MCSource::c_FarFieldIsotropic, "FarFieldIsotropic" }, { MCSource::c_FarFieldDisk, "FarFieldDisk" }
  };
  for (const auto& N: BeamNames) {
    MCSource S("Beam names");
    SilenceOutput();
    S.SetBeamType(N.first < MCSource::c_NearFieldPoint ? MCSource::c_FarField : MCSource::c_NearField, N.first);
    RestoreOutput();
    Passed = Evaluate("GetBeamTypeAsString()", N.second, "The beam type has its name", MString(S.GetBeamTypeAsString()), N.second) && Passed;
  }

  // Beam descriptions with the units of the source file (lengths in cm, angles in deg):
  {
    MCSource P("Point");
    SilenceOutput();
    P.SetBeamType(MCSource::c_NearField, MCSource::c_NearFieldPoint);
    P.SetPosition(1*cm, 2*cm, 3*cm);
    RestoreOutput();
    Passed = Evaluate("GetBeamAsString()", "point", "A point source is described like in the source file", MString(P.GetBeamAsString()), MString("PointSource 1 2 3")) && Passed;

    MCSource FP("Far field point");
    SilenceOutput();
    FP.SetBeamType(MCSource::c_FarField, MCSource::c_FarFieldPoint);
    FP.SetPosition(30*deg, 45*deg);
    RestoreOutput();
    Passed = Evaluate("GetBeamAsString()", "far field point", "A far field point source is described in degrees", MString(FP.GetBeamAsString()), MString("FarFieldPointSource 30 45")) && Passed;

    MCSource FA("Far field area");
    SilenceOutput();
    FA.SetBeamType(MCSource::c_FarField, MCSource::c_FarFieldArea);
    FA.SetPosition(10*deg, 80*deg, 0.0, 360*deg);
    RestoreOutput();
    Passed = Evaluate("GetBeamAsString()", "far field area", "A far field area source is described in degrees", MString(FA.GetBeamAsString()), MString("FarFieldAreaSource 10 80 0 360")) && Passed;

    MCSource G("Gaussian");
    SilenceOutput();
    G.SetBeamType(MCSource::c_FarField, MCSource::c_FarFieldGaussian);
    G.SetPosition(30*deg, 45*deg, 5*deg);
    RestoreOutput();
    Passed = Evaluate("GetBeamAsString()", "far field gaussian", "A Gaussian far field source is described in degrees", MString(G.GetBeamAsString()), MString("FarFieldGaussian 30 45 5")) && Passed;

    MCSource L("Line");
    SilenceOutput();
    L.SetBeamType(MCSource::c_NearField, MCSource::c_NearFieldLine);
    L.SetPosition(1*cm, 2*cm, 3*cm, 4*cm, 5*cm, 6*cm);
    RestoreOutput();
    Passed = Evaluate("GetBeamAsString()", "line", "A line source is described like in the source file", MString(L.GetBeamAsString()), MString("LineSource 1 2 3 4 5 6")) && Passed;

    MCSource D("Disk");
    SilenceOutput();
    D.SetBeamType(MCSource::c_NearField, MCSource::c_NearFieldDisk);
    D.SetPosition(1*cm, 2*cm, 3*cm, 0.0, 0.0, 1.0, 4*cm, 5*cm, 6*cm, 7*deg, 8*deg);
    RestoreOutput();
    Passed = Evaluate("GetBeamAsString()", "disk", "A disk is described like in the source file", MString(D.GetBeamAsString()), MString("Disk 1 2 3 0 0 1 4 5 6 7 8")) && Passed;

    MCSource B("Beam");
    SilenceOutput();
    B.SetBeamType(MCSource::c_NearField, MCSource::c_NearFieldBeam);
    B.SetPosition(1*cm, 2*cm, 3*cm, 0.0, 0.0, -1.0, 2*cm);
    RestoreOutput();
    Passed = Evaluate("GetBeamAsString()", "homogeneous beam", "A homogeneous beam is described like in the source file", MString(B.GetBeamAsString()), MString("HomogeneousBeam 1 2 3 0 0 -1 2")) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSourceBasics::TestFarFieldOptions()
{
  bool Passed = true;

  // Earth occultation is only available for far field sources:
  {
    MCSource Near("Near field");
    SilenceOutput();
    Near.SetCoordinateSystem(MCSource::c_NearField);
    bool NearResult = Near.SetEarthOccultation(60*deg, false);
    MCSource None("No coordinate system");
    bool NoneResult = None.SetEarthOccultation(60*deg, false);
    MCSource Far("Far field");
    Far.SetCoordinateSystem(MCSource::c_FarField);
    bool FarResult = Far.SetEarthOccultation(60*deg, true);
    RestoreOutput();
    Passed = EvaluateFalse("SetEarthOccultation()", "near field", "Earth occultation is rejected for near field sources", NearResult) && Passed;
    Passed = EvaluateFalse("SetEarthOccultation()", "no coordinate system", "Earth occultation is rejected for sources without coordinate system", NoneResult) && Passed;
    Passed = EvaluateTrue("SetEarthOccultation()", "far field", "Earth occultation is accepted for far field sources", FarResult) && Passed;
  }

  // The far field transmission probability is a two dimensional function of zenith and energy:
  {
    const MString Good = GetTemporaryFileName("Transmission.dat");
    Passed = EvaluateTrue("WriteTextFile()", "transmission", "The transmission fixture can be written", WriteTextFile(Good, "XA 0 90\nYA 100 1000\nAP 0 0 1.0\nAP 0 1 1.0\nAP 1 0 0.5\nAP 1 1 0.5\nEN\n")) && Passed;

    MCSource Near("Near field");
    SilenceOutput();
    Near.SetCoordinateSystem(MCSource::c_NearField);
    bool NearResult = Near.SetFarFieldTransmissionProbability(Good);
    MCSource Far("Far field");
    Far.SetCoordinateSystem(MCSource::c_FarField);
    bool MissingResult = Far.SetFarFieldTransmissionProbability(GetTemporaryFileName("Missing.dat"));
    bool FarResult = Far.SetFarFieldTransmissionProbability(Good);
    RestoreOutput();
    Passed = EvaluateFalse("SetFarFieldTransmissionProbability()", "near field", "A transmission probability is rejected for near field sources", NearResult) && Passed;
    Passed = EvaluateFalse("SetFarFieldTransmissionProbability()", "missing file", "A missing transmission probability file is rejected", MissingResult) && Passed;
    Passed = EvaluateTrue("SetFarFieldTransmissionProbability()", "far field", "A valid transmission probability is accepted for far field sources", FarResult) && Passed;
  }

  // The normalized energy-beam-flux-function needs its beam type:
  {
    MCSource S("Normalized");
    SilenceOutput();
    S.SetBeamType(MCSource::c_FarField, MCSource::c_FarFieldPoint);
    bool WrongBeam = S.SetNormalizedEnergyBeamFluxFunction(GetTemporaryFileName("Missing.dat"));
    S.SetBeamType(MCSource::c_FarField, MCSource::c_FarFieldNormalizedEnergyBeamFluxFunction);
    bool Missing = S.SetNormalizedEnergyBeamFluxFunction(GetTemporaryFileName("Missing.dat"));
    RestoreOutput();
    Passed = EvaluateFalse("SetNormalizedEnergyBeamFluxFunction()", "wrong beam type", "The function is rejected for other beam types", WrongBeam) && Passed;
    Passed = EvaluateFalse("SetNormalizedEnergyBeamFluxFunction()", "missing file", "A missing function file is rejected", Missing) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTSourceBasics Test;
  return Test.Run() == true ? 0 : 1;
}
