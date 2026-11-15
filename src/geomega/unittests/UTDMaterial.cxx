/*
 * UTDMaterial.cxx
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
#include <cmath>
#include <cstdio>
#include <limits>
#include <vector>
using namespace std;

// ROOT libs:
#include "TApplication.h"
#include "TEnv.h"
#include "TError.h"
#include "TGeoMaterial.h"
#include "TGeoMedium.h"
#include "TROOT.h"

// MEGAlib libs:
#include "MDGeometry.h"
#include "MDMaterial.h"
#include "MDMaterialComponent.h"
#include "MGlobal.h"
#include "MResponseMatrixO1.h"
#include "MString.h"
#include "UTGeomegaShared.h"


////////////////////////////////////////////////////////////////////////////////


//! Unit test of the material MDMaterial
//! Attention: The declared functions IsClone and GetCloneTemplate have no definition and cannot be linked, thus they are not tested
class UTDMaterial : public UTGeomegaBase
{
public:
  //! Default constructor
  UTDMaterial() : UTGeomegaBase("UTDMaterial") {}
  //! Default destructor
  virtual ~UTDMaterial() {}

  //! Run all tests
  virtual bool Run();

private:
  //! One element of a mixture: mass fraction, atomic number, atomic weight
  struct ETElement { double MassFraction; double Z; double A; };

  //! Test the constructors
  bool TestConstruction();
  //! Test the ID counter
  bool TestIDs();
  //! Test the dummy material
  bool TestDummyMaterial();
  //! Test the conversion between element names and atomic numbers
  bool TestNameConversion();
  //! Test the density and radiation length setters
  bool TestDensityAndRadiationLength();
  //! Test the component setters and the access to the components
  bool TestComponents();
  //! Test the determination of the main component
  bool TestMainComponent();
  //! Test the string descriptions
  bool TestStrings();
  //! Test the validation
  bool TestValidate();
  //! Test the hash
  bool TestHash();
  //! Test the mass fractions of the ROOT mixture against hand computed values
  bool TestRootMediumComposition();
  //! Test the radiation length against the formula and published values
  bool TestRadiationLength();
  //! Test the IDs of the ROOT media
  bool TestRootMediumIDs();
  //! Test the clone interface
  bool TestClones();
  //! Test reading the cross sections from fixture files
  bool TestCrossSectionFiles();
  //! Test the cross section directories
  bool TestCrossSectionDirectories();
  //! Test the cross sections stored with MEGAlib against published values
  bool TestStoredCrossSections();
  //! Test the material lookups of a geometry
  bool TestLookups();
  //! Test the material keywords of the geometry file
  bool TestKeywords();

  //! Return the radiation length from the Rossi-Greisen formula with Davies-Bethe-Maximon Coulomb correction (cm)
  double GetRadiationLengthByFormula(double Density, const vector<ETElement>& Elements) const;
  //! Write the five cross section files, the values on the energy axis 10, 20, 40 keV are 1, 3, 7 times factor times the process number
  bool WriteCrossSections(const MString& Directory, const MString& Name, unsigned long Hash, double Factor = 1.0);
  //! Return true if the scan of a small geometry with the given materials succeeds
  bool ScanMaterials(MDGeometry& Geometry, const MString& Materials);
  //! Return the head with a private vacuum material
  MString GetSmallHead() const;
};


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTDMaterial::Run()
{
  bool Passed = true;

  Passed = TestConstruction() && Passed;
  Passed = TestIDs() && Passed;
  Passed = TestDummyMaterial() && Passed;
  Passed = TestNameConversion() && Passed;
  Passed = TestDensityAndRadiationLength() && Passed;
  Passed = TestComponents() && Passed;
  Passed = TestMainComponent() && Passed;
  Passed = TestStrings() && Passed;
  Passed = TestValidate() && Passed;
  Passed = TestHash() && Passed;
  Passed = TestRootMediumComposition() && Passed;
  Passed = TestRadiationLength() && Passed;
  Passed = TestRootMediumIDs() && Passed;
  Passed = TestClones() && Passed;
  Passed = TestCrossSectionFiles() && Passed;
  Passed = TestCrossSectionDirectories() && Passed;
  Passed = TestStoredCrossSections() && Passed;
  Passed = TestLookups() && Passed;
  Passed = TestKeywords() && Passed;

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the radiation length from the Rossi-Greisen formula with Davies-Bethe-Maximon Coulomb correction (cm)
double UTDMaterial::GetRadiationLengthByFormula(double Density, const vector<ETElement>& Elements) const
{
  // X0 [g/cm2] = 716.408 A / (Z^2 (Lrad - f(Z)) + Z Lrad') with Lrad = ln(183 Z^-1/3) and Lrad' = ln(1440 Z^-2/3)
  // Mixtures: 1/X0 = sum w_i/X0_i (PDG, Passage of particles through matter)
  const double FineStructure = 1.0/137.036;
  double Inverse = 0;
  for (const ETElement& E : Elements) {
    double a2 = pow(FineStructure*E.Z, 2);
    double Coulomb = a2*(1.0/(1.0 + a2) + 0.20206 - 0.0369*a2 + 0.0083*a2*a2 - 0.002*a2*a2*a2);
    double Lrad = log(183.0*pow(E.Z, -1.0/3.0));
    double LradPrime = log(1440.0*pow(E.Z, -2.0/3.0));
    Inverse += E.MassFraction*(E.Z*E.Z*(Lrad - Coulomb) + E.Z*LradPrime)/(716.408*E.A);
  }
  return 1.0/Inverse/Density;
}


////////////////////////////////////////////////////////////////////////////////


//! Write the five cross section files, the values on the energy axis 10, 20, 40 keV are 1, 3, 7 times factor times the process number
bool UTDMaterial::WriteCrossSections(const MString& Directory, const MString& Name, unsigned long Hash, double Factor)
{
  const vector<MString> Processes = { "Total", "Photo", "Compton", "Pair", "Rayleigh" };
  for (unsigned int p = 0; p < Processes.size(); ++p) {
    MString Content;
    Content += "Type    ResponseMatrixO1\n";
    Content += "Version 1\n\n";
    Content += MString("NM ") + Processes[p] + " cross section\n";
    Content += "CE false\n";
    Content += MString("HA ") + Hash + "\n\n";
    Content += MString("R1 10.0 ") + (Factor*(p + 1)*1.0) + "\n";
    Content += MString("R1 20.0 ") + (Factor*(p + 1)*3.0) + "\n";
    Content += MString("R1 40.0 ") + (Factor*(p + 1)*7.0) + "\n";
    if (WriteTextFile(Directory + "/Xsection." + Processes[p] + "." + Name + ".rsp", Content) == false) return false;
  }
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the head with a private vacuum material
MString UTDMaterial::GetSmallHead() const
{
  MString Head;
  Head += "Name UnitTest\n";
  Head += "Version 1.0\n";
  Head += "SurroundingSphere 50.0 0.0 0.0 0.0 50.0\n";
  Head += "Material Nothing\n";
  Head += "Nothing.Density 1E-12\n";
  Head += "Nothing.Component H 1\n";
  Head += "Volume World\n";
  Head += "World.Material Nothing\n";
  Head += "World.Shape BRIK 100.0 100.0 100.0\n";
  Head += "World.Visibility 0\n";
  Head += "World.Mother 0\n";
  return Head;
}


////////////////////////////////////////////////////////////////////////////////


//! Return true if the scan of a small geometry with the given materials succeeds
bool UTDMaterial::ScanMaterials(MDGeometry& Geometry, const MString& Materials)
{
  return ScanGeometry(Geometry, GetSmallHead() + Materials);
}


////////////////////////////////////////////////////////////////////////////////


//! Test the constructors
bool UTDMaterial::TestConstruction()
{
  bool Passed = true;

  MDMaterial Default;
  Passed = EvaluateTrue("MDMaterial()", "default", "The name is not defined", Default.GetName() == g_StringNotDefined) && Passed;
  Passed = EvaluateNear("MDMaterial()", "default", "The density is not defined", Default.GetDensity(), g_DoubleNotDefined, 0.0) && Passed;
  Passed = EvaluateNear("MDMaterial()", "default", "The radiation length is not defined", Default.GetRadiationLength(), g_DoubleNotDefined, 0.0) && Passed;
  Passed = Evaluate("MDMaterial()", "default", "There are no components", Default.GetNComponents(), 0U) && Passed;
  Passed = Evaluate("MDMaterial()", "default", "There are no clones", Default.GetNClones(), 0U) && Passed;
  Passed = EvaluateFalse("MDMaterial()", "default", "There are no cross sections", Default.AreCrossSectionsPresent()) && Passed;
  Passed = Evaluate("MDMaterial()", "default", "The hash is zero before the validation", Default.GetHash(), 0UL) && Passed;
  Passed = EvaluateTrue("MDMaterial()", "default", "The cross section directory is not defined", Default.GetCrossSectionFileDirectory() == g_StringNotDefined) && Passed;
  Passed = EvaluateTrue("MDMaterial()", "default", "The default cross section directory is not defined", Default.GetDefaultCrossSectionFileDirectory() == g_StringNotDefined) && Passed;

  MDMaterial Named("Alu");
  Passed = EvaluateTrue("MDMaterial(Name)", "Alu", "The name is stored", Named.GetName() == "Alu") && Passed;
  Passed = EvaluateNear("MDMaterial(Name)", "Alu", "The density is not defined", Named.GetDensity(), g_DoubleNotDefined, 0.0) && Passed;
  Passed = EvaluateNear("MDMaterial(Name)", "Alu", "The radiation length is not defined", Named.GetRadiationLength(), g_DoubleNotDefined, 0.0) && Passed;
  Passed = EvaluateTrue("MDMaterial(Name)", "Alu", "The cross section directory is not defined", Named.GetCrossSectionFileDirectory() == g_StringNotDefined) && Passed;

  // Defect GEO-F-07: The name constructor does not initialize the default cross section directory as the default constructor does
  Passed = ExpectedToFail(EvaluateTrue("MDMaterial(Name)", "Alu", "The default cross section directory is not defined", Named.GetDefaultCrossSectionFileDirectory() == g_StringNotDefined)) && Passed;

  Named.SetName("Other");
  Passed = EvaluateTrue("SetName()", "Other", "The new name replaces the old one", Named.GetName() == "Other") && Passed;
  Named.SetName("");
  Passed = EvaluateTrue("SetName()", "empty", "An empty name is stored", Named.GetName() == "") && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the ID counter
bool UTDMaterial::TestIDs()
{
  bool Passed = true;

  MDMaterial::ResetIDs();
  MDMaterial First;
  MDMaterial Second("Second");
  MDMaterial Third;
  Passed = Evaluate("ResetIDs()", "first material after the reset", "The counting starts at one", First.GetID(), 1) && Passed;
  Passed = Evaluate("GetID()", "second material", "The name constructor uses the next number", Second.GetID(), 2) && Passed;
  Passed = Evaluate("GetID()", "third material", "The default constructor uses the next number", Third.GetID(), 3) && Passed;

  MDMaterial::ResetIDs();
  MDMaterial Again;
  Passed = Evaluate("ResetIDs()", "second reset", "The counting starts at one again", Again.GetID(), 1) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the dummy material
bool UTDMaterial::TestDummyMaterial()
{
  bool Passed = true;

  MDMaterial* Dummy = MDMaterial::GetDummyMaterial();
  Passed = EvaluateTrue("GetDummyMaterial()", "first call", "A material is returned", Dummy != nullptr) && Passed;
  Passed = EvaluateTrue("GetDummyMaterial()", "second call", "The same material is returned", Dummy == MDMaterial::GetDummyMaterial()) && Passed;
  Passed = EvaluateTrue("GetDummyMaterial()", "name", "The name is the one of the dummy", Dummy->GetName() == "GeomegaDummy") && Passed;
  Passed = EvaluateNear("GetDummyMaterial()", "density", "The density is vacuum like", Dummy->GetDensity(), 1E-12, 1E-24) && Passed;
  Passed = EvaluateNear("GetDummyMaterial()", "radiation length", "The radiation length is vacuum like", Dummy->GetRadiationLength(), 1E+12, 1.0) && Passed;
  Passed = Evaluate("GetDummyMaterial()", "components", "The dummy has one component", Dummy->GetNComponents(), 1U) && Passed;
  Passed = Evaluate("GetDummyMaterial()", "components", "The component is hydrogen", Dummy->GetAtomicNumberMainComponent(), 1) && Passed;
  Passed = EvaluateTrue("GetDummyMaterial()", "hash", "The dummy has been validated since it has a hash", Dummy->GetHash() != 0UL) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the conversion between element names and atomic numbers
bool UTDMaterial::TestNameConversion()
{
  bool Passed = true;

  // Periodic table up to Z = 111
  const vector<MString> Names = {
    "H", "He", "Li", "Be", "B", "C", "N", "O", "F", "Ne", "Na", "Mg", "Al", "Si", "P", "S", "Cl", "Ar", "K", "Ca",
    "Sc", "Ti", "V", "Cr", "Mn", "Fe", "Co", "Ni", "Cu", "Zn", "Ga", "Ge", "As", "Se", "Br", "Kr", "Rb", "Sr", "Y", "Zr",
    "Nb", "Mo", "Tc", "Ru", "Rh", "Pd", "Ag", "Cd", "In", "Sn", "Sb", "Te", "I", "Xe", "Cs", "Ba", "La", "Ce", "Pr", "Nd",
    "Pm", "Sm", "Eu", "Gd", "Tb", "Dy", "Ho", "Er", "Tm", "Yb", "Lu", "Hf", "Ta", "W", "Re", "Os", "Ir", "Pt", "Au", "Hg",
    "Tl", "Pb", "Bi", "Po", "At", "Rn", "Fr", "Ra", "Ac", "Th", "Pa", "U", "Np", "Pu", "Am", "Cm", "Bk", "Cf", "Es", "Fm",
    "Md", "No", "Lr", "Rf", "Db", "Sg", "Bh", "Hs", "Mt", "Ds", "Rg"
  };
  Passed = EvaluateSize("ConvertNameToAtomicNumber()", "reference table", "The reference table has 111 elements", Names.size(), 111UL) && Passed;

  for (unsigned int i = 0; i < Names.size(); ++i) {
    Passed = Evaluate("ConvertNameToAtomicNumber()", Names[i], "The atomic number is the one of the periodic table", MDMaterial::ConvertNameToAtomicNumber(Names[i]), static_cast<int>(i + 1)) && Passed;
    Passed = EvaluateTrue("ConvertAtomicNumberToName()", i + 1, "The name is the one of the periodic table", MDMaterial::ConvertAtomicNumberToName(i + 1) == Names[i]) && Passed;
  }

  // Failure paths return the sentinels
  for (const MString& Bad : vector<MString>{ "", "Xx", "Uub", "H ", "Lead" }) {
    DisableDefaultStreams();
    int Result = MDMaterial::ConvertNameToAtomicNumber(Bad);
    EnableDefaultStreams();
    Passed = Evaluate("ConvertNameToAtomicNumber()", MString("\"") + Bad + "\"", "An unknown name returns zero", Result, 0) && Passed;
  }
  for (int Bad : {0, -1, 112, 1000}) {
    DisableDefaultStreams();
    MString Result = MDMaterial::ConvertAtomicNumberToName(Bad);
    EnableDefaultStreams();
    Passed = EvaluateTrue("ConvertAtomicNumberToName()", Bad, "An unknown atomic number returns an empty string", Result == "") && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the density and radiation length setters
bool UTDMaterial::TestDensityAndRadiationLength()
{
  bool Passed = true;

  MDMaterial M("Test");
  Passed = EvaluateTrue("SetDensity()", "2.7", "A positive density is accepted", M.SetDensity(2.7)) && Passed;
  Passed = EvaluateNear("GetDensity()", "2.7", "The density is stored", M.GetDensity(), 2.7, 1E-12) && Passed;
  Passed = EvaluateTrue("SetDensity()", "1E-300", "A tiny positive density is accepted", M.SetDensity(1E-300)) && Passed;
  Passed = EvaluateNear("GetDensity()", "1E-300", "The density is stored", M.GetDensity(), 1E-300, 1E-310) && Passed;
  M.SetDensity(2.7);

  for (double Bad : {0.0, -1E-300, -2.7}) {
    DisableDefaultStreams();
    bool Result = M.SetDensity(Bad);
    EnableDefaultStreams();
    Passed = EvaluateFalse("SetDensity()", Bad, "A non-positive density is rejected", Result) && Passed;
    Passed = EvaluateNear("SetDensity()", Bad, "The rejection leaves the old density", M.GetDensity(), 2.7, 1E-12) && Passed;
  }

  Passed = EvaluateTrue("SetRadiationLength()", "8.9", "A positive radiation length is accepted", M.SetRadiationLength(8.9)) && Passed;
  Passed = EvaluateNear("GetRadiationLength()", "8.9", "The radiation length is stored", M.GetRadiationLength(), 8.9, 1E-12) && Passed;

  for (double Bad : {0.0, -1E-300, -8.9}) {
    DisableDefaultStreams();
    bool Result = M.SetRadiationLength(Bad);
    EnableDefaultStreams();
    Passed = EvaluateFalse("SetRadiationLength()", Bad, "A non-positive radiation length is rejected", Result) && Passed;
    Passed = EvaluateNear("SetRadiationLength()", Bad, "The rejection leaves the old value", M.GetRadiationLength(), 8.9, 1E-12) && Passed;
  }

  DisableDefaultStreams();
  bool NaNDensity = M.SetDensity(numeric_limits<double>::quiet_NaN());
  EnableDefaultStreams();
  // Defect GEO-F-11: A not-a-number density passes the check for a positive value
  Passed = ExpectedToFail(EvaluateFalse("SetDensity()", "NaN", "A not-a-number density is rejected since it is not positive", NaNDensity)) && Passed;

  DisableDefaultStreams();
  bool NaNLength = M.SetRadiationLength(numeric_limits<double>::quiet_NaN());
  EnableDefaultStreams();
  // Defect GEO-F-11: A not-a-number radiation length passes the check for a positive value
  Passed = ExpectedToFail(EvaluateFalse("SetRadiationLength()", "NaN", "A not-a-number radiation length is rejected since it is not positive", NaNLength)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the component setters and the access to the components
bool UTDMaterial::TestComponents()
{
  bool Passed = true;

  MDMaterial M("Test");

  // Access on an empty list
  DisableDefaultStreams();
  MDMaterialComponent* OnEmpty = M.GetComponentAt(0);
  EnableDefaultStreams();
  Passed = EvaluateTrue("GetComponentAt()", "empty list", "The index 0 is out of bounds and returns null", OnEmpty == nullptr) && Passed;

  // All setters in sequence
  MDMaterialComponent* Owned = new MDMaterialComponent();
  Owned->SetByAtomicWeighting("Fe", 7);
  Passed = EvaluateTrue("SetComponent()", "new component", "A component is accepted", M.SetComponent(Owned)) && Passed;
  DisableDefaultStreams();
  bool NullResult = M.SetComponent(nullptr);
  EnableDefaultStreams();
  Passed = EvaluateFalse("SetComponent()", "nullptr", "A null pointer is rejected", NullResult) && Passed;
  Passed = Evaluate("SetComponent()", "nullptr", "A null pointer is not added", M.GetNComponents(), 1U) && Passed;
  Passed = EvaluateTrue("GetComponentAt()", "index 0", "The component itself is stored - the material owns it", M.GetComponentAt(0) == Owned) && Passed;

  Passed = EvaluateTrue("SetComponentByAtomicWeighting(A, Z, N)", "27.5, 13, 2", "A valid isotope is accepted", M.SetComponentByAtomicWeighting(27.5, 13, 2)) && Passed;
  Passed = EvaluateTrue("SetComponentByAtomicWeighting(Name, N)", "O, 3", "A valid element is accepted", M.SetComponentByAtomicWeighting("O", 3)) && Passed;
  Passed = EvaluateTrue("SetComponentByMassWeighting(A, Z, W)", "14.5, 7, 0.25", "A valid isotope is accepted", M.SetComponentByMassWeighting(14.5, 7, 0.25)) && Passed;
  Passed = EvaluateTrue("SetComponentByMassWeighting(Name, W)", "Ar, 0.5", "A valid element is accepted", M.SetComponentByMassWeighting("Ar", 0.5)) && Passed;
  Passed = Evaluate("GetNComponents()", "five components", "All components are in the list", M.GetNComponents(), 5U) && Passed;

  // Check the order and the content
  MDMaterialComponent* C = M.GetComponentAt(1);
  Passed = EvaluateTrue("SetComponentByAtomicWeighting(A, Z, N)", "27.5, 13, 2", "The isotope is by atoms and not natural", C->GetWeightingType() == MDMaterialComponentWeightingType::c_ByAtoms && C->HasNaturalIsotopeComposition() == false && C->GetAtomicNumber() == 13 && C->GetWeightingByAtoms() == 2) && Passed;
  C = M.GetComponentAt(2);
  Passed = EvaluateTrue("SetComponentByAtomicWeighting(Name, N)", "O, 3", "The element is by atoms and natural", C->GetWeightingType() == MDMaterialComponentWeightingType::c_ByAtoms && C->HasNaturalIsotopeComposition() == true && C->GetAtomicNumber() == 8 && C->GetWeightingByAtoms() == 3) && Passed;
  C = M.GetComponentAt(3);
  Passed = EvaluateTrue("SetComponentByMassWeighting(A, Z, W)", "14.5, 7, 0.25", "The isotope is by mass and not natural", C->GetWeightingType() == MDMaterialComponentWeightingType::c_ByMass && C->HasNaturalIsotopeComposition() == false && C->GetAtomicNumber() == 7) && Passed;
  Passed = EvaluateNear("SetComponentByMassWeighting(A, Z, W)", "14.5, 7, 0.25", "The mass fraction is stored", C->GetWeightingByMass(), 0.25, 1E-12) && Passed;
  C = M.GetComponentAt(4);
  Passed = EvaluateTrue("SetComponentByMassWeighting(Name, W)", "Ar, 0.5", "The element is by mass and natural", C->GetWeightingType() == MDMaterialComponentWeightingType::c_ByMass && C->HasNaturalIsotopeComposition() == true && C->GetAtomicNumber() == 18) && Passed;
  Passed = EvaluateNear("SetComponentByMassWeighting(Name, W)", "Ar, 0.5", "The mass fraction is stored", C->GetWeightingByMass(), 0.5, 1E-12) && Passed;

  // Boundary indices
  Passed = EvaluateTrue("GetComponentAt()", "n - 1", "The last index exists", M.GetComponentAt(4) != nullptr) && Passed;
  DisableDefaultStreams();
  MDMaterialComponent* OutOfBounds = M.GetComponentAt(5);
  EnableDefaultStreams();
  Passed = EvaluateTrue("GetComponentAt()", "n", "The index n is out of bounds and returns null", OutOfBounds == nullptr) && Passed;

  // Failures do not add anything
  DisableDefaultStreams();
  bool AtomIsotope = M.SetComponentByAtomicWeighting(0.0, 13, 2);
  bool AtomName = M.SetComponentByAtomicWeighting("Xx", 2);
  bool MassIsotope = M.SetComponentByMassWeighting(27.5, 0, 0.5);
  bool MassName = M.SetComponentByMassWeighting("O", -1.0);
  EnableDefaultStreams();
  Passed = EvaluateFalse("SetComponentByAtomicWeighting(A, Z, N)", "zero A", "An invalid isotope is rejected", AtomIsotope) && Passed;
  Passed = EvaluateFalse("SetComponentByAtomicWeighting(Name, N)", "unknown name", "An invalid element is rejected", AtomName) && Passed;
  Passed = EvaluateFalse("SetComponentByMassWeighting(A, Z, W)", "zero Z", "An invalid isotope is rejected", MassIsotope) && Passed;
  Passed = EvaluateFalse("SetComponentByMassWeighting(Name, W)", "negative mass", "An invalid mass fraction is rejected", MassName) && Passed;
  Passed = Evaluate("GetNComponents()", "after failures", "No component has been added", M.GetNComponents(), 5U) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the determination of the main component
bool UTDMaterial::TestMainComponent()
{
  bool Passed = true;

  MDMaterial Empty("Empty");
  Passed = Evaluate("GetAtomicNumberMainComponent()", "no components", "Without components zero is returned", Empty.GetAtomicNumberMainComponent(), 0) && Passed;

  MDMaterial Single("Single");
  Single.SetComponentByAtomicWeighting("Ge", 1);
  Passed = Evaluate("GetAtomicNumberMainComponent()", "only Ge", "The only component is the main component", Single.GetAtomicNumberMainComponent(), 32) && Passed;

  // By mass: the largest mass fraction independent of the order
  MDMaterial Air("Air");
  Air.SetComponentByMassWeighting("N", 0.7556);
  Air.SetComponentByMassWeighting("O", 0.2315);
  Air.SetComponentByMassWeighting("Ar", 0.0129);
  Passed = Evaluate("GetAtomicNumberMainComponent()", "air by mass", "Nitrogen has the largest mass fraction", Air.GetAtomicNumberMainComponent(), 7) && Passed;

  MDMaterial Reversed("Reversed");
  Reversed.SetComponentByMassWeighting("Ar", 0.0129);
  Reversed.SetComponentByMassWeighting("O", 0.2315);
  Reversed.SetComponentByMassWeighting("N", 0.7556);
  Passed = Evaluate("GetAtomicNumberMainComponent()", "air by mass, reversed", "The order does not matter", Reversed.GetAtomicNumberMainComponent(), 7) && Passed;

  // A tie goes to the larger atomic number, in both orders
  MDMaterial Tie("Tie");
  Tie.SetComponentByMassWeighting("Fe", 0.5);
  Tie.SetComponentByMassWeighting("Al", 0.5);
  Passed = Evaluate("GetAtomicNumberMainComponent()", "tie Fe, Al", "A tie goes to the larger atomic number", Tie.GetAtomicNumberMainComponent(), 26) && Passed;
  MDMaterial TieReversed("TieReversed");
  TieReversed.SetComponentByMassWeighting("Al", 0.5);
  TieReversed.SetComponentByMassWeighting("Fe", 0.5);
  Passed = Evaluate("GetAtomicNumberMainComponent()", "tie Al, Fe", "A tie goes to the larger atomic number", TieReversed.GetAtomicNumberMainComponent(), 26) && Passed;

  // CsI by atoms: Tie in atoms, and Cs has also the larger mass
  MDMaterial CsI("CsI");
  CsI.SetComponentByAtomicWeighting("Cs", 1);
  CsI.SetComponentByAtomicWeighting("I", 1);
  Passed = Evaluate("GetAtomicNumberMainComponent()", "CsI by atoms", "Cesium is the main component", CsI.GetAtomicNumberMainComponent(), 55) && Passed;

  // BGO, mass share by hand: Bi 4*208.98 = 835.9, Ge 3*72.64 = 217.9, O 12*16.00 = 192.0, thus Bi is 67% of the mass
  MDMaterial BGO("BGO");
  BGO.SetComponentByAtomicWeighting("Bi", 4);
  BGO.SetComponentByAtomicWeighting("Ge", 3);
  BGO.SetComponentByAtomicWeighting("O", 12);
  // Defect GEO-F-05: A number of atoms is compared as if it would be a mass fraction, BGO is reported as oxygen
  Passed = ExpectedToFail(Evaluate("GetAtomicNumberMainComponent()", "BGO by atoms", "Bismuth has by far the largest mass fraction", BGO.GetAtomicNumberMainComponent(), 83)) && Passed;

  // Alumina, mass share by hand: Al 2*26.98 = 53.96, O 3*16.00 = 48.00, thus Al is 53% of the mass
  MDMaterial Alumina("Alumina");
  Alumina.SetComponentByAtomicWeighting("Al", 2);
  Alumina.SetComponentByAtomicWeighting("O", 3);
  // Defect GEO-F-05: A number of atoms is compared as if it would be a mass fraction, alumina is reported as oxygen
  Passed = ExpectedToFail(Evaluate("GetAtomicNumberMainComponent()", "Al2O3 by atoms", "Aluminum has the largest mass fraction", Alumina.GetAtomicNumberMainComponent(), 13)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the string descriptions
bool UTDMaterial::TestStrings()
{
  bool Passed = true;

  MDMaterial M("Water");
  M.SetDensity(0.5);
  M.SetRadiationLength(36.5);
  M.SetComponentByAtomicWeighting("H", 2);
  M.SetComponentByAtomicWeighting("O", 1);

  MString Expected = "Material Water\nWater.Density 0.5\nWater.ComponentByAtoms H 2\nWater.ComponentByAtoms O 1\n\n";

  // Defect GEO-F-15: The radiation length given by the user is not part of the Geomega description
  Passed = ExpectedToFail(EvaluateTrue("GetGeomega()", "radiation length 36.5", "The output has the radiation length keyword", M.GetGeomega().Contains("Water.RadiationLength 36.5"))) && Passed;

  MDMaterial NoLength("Water");
  NoLength.SetDensity(0.5);
  NoLength.SetComponentByAtomicWeighting("H", 2);
  NoLength.SetComponentByAtomicWeighting("O", 1);
  Passed = EvaluateTrue("GetGeomega()", "by atoms", "The output is the material keyword, the density, and all components", NoLength.GetGeomega() == Expected) && Passed;

  MDMaterial ByMass("Mix");
  ByMass.SetDensity(1.25);
  ByMass.SetComponentByMassWeighting(14.5, 7, 0.75);
  ByMass.SetComponentByMassWeighting("O", 0.25);
  Passed = EvaluateTrue("GetGeomega()", "by mass", "The output has isotope and natural components", ByMass.GetGeomega() == "Material Mix\nMix.Density 1.25\nMix.ComponentByMass 14.5 7 0.75\nMix.ComponentByMass O 0.25\n\n") && Passed;

  MDMaterial Empty("Empty");
  Empty.SetDensity(1.0);
  Passed = EvaluateTrue("GetGeomega()", "no components", "The output has only name and density", Empty.GetGeomega() == "Material Empty\nEmpty.Density 1\n\n") && Passed;

  // The sensitivity flag has no setter and stays at its not-defined value
  MString Description = MString("Material: Water - D:0.5, R:36.5, S:") + g_IntNotDefined + ", C's: (0) Z=1 (A=natural composition), atoms=2 (1) Z=8 (A=natural composition), atoms=1\n";
  Passed = EvaluateTrue("ToString()", "water", "The description has density, radiation length, sensitivity, and all components", M.ToString() == Description) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the validation
bool UTDMaterial::TestValidate()
{
  bool Passed = true;

  auto Validate = [this](MDMaterial& M) {
    DisableDefaultStreams();
    bool Result = M.Validate();
    EnableDefaultStreams();
    return Result;
  };

  // Everything which is required
  MDMaterial NoName;
  NoName.SetDensity(1.0);
  NoName.SetComponentByAtomicWeighting("H", 1);
  Passed = EvaluateFalse("Validate()", "no name", "A material without name is invalid", Validate(NoName)) && Passed;

  MDMaterial NoComponents("NoComponents");
  NoComponents.SetDensity(1.0);
  Passed = EvaluateFalse("Validate()", "no components", "A material without components is invalid", Validate(NoComponents)) && Passed;

  MDMaterial NoDensity("NoDensity");
  NoDensity.SetComponentByAtomicWeighting("H", 1);
  Passed = EvaluateFalse("Validate()", "no density", "A material without density is invalid", Validate(NoDensity)) && Passed;

  MDMaterial BadComponent("BadComponent");
  BadComponent.SetDensity(1.0);
  BadComponent.SetComponentByAtomicWeighting(258.0, 101, 1);
  Passed = EvaluateFalse("Validate()", "Z = 101", "A material with an invalid component is invalid", Validate(BadComponent)) && Passed;

  MDMaterial Mixed("Mixed");
  Mixed.SetDensity(1.0);
  Mixed.SetComponentByAtomicWeighting("H", 2);
  Mixed.SetComponentByMassWeighting("O", 0.9);
  Passed = EvaluateFalse("Validate()", "atoms and mass", "A mix of weighting types is invalid", Validate(Mixed)) && Passed;
  MDMaterial MixedReversed("MixedReversed");
  MixedReversed.SetDensity(1.0);
  MixedReversed.SetComponentByMassWeighting("O", 0.9);
  MixedReversed.SetComponentByMassWeighting("H", 0.1);
  MixedReversed.SetComponentByAtomicWeighting("N", 1);
  Passed = EvaluateFalse("Validate()", "mass, mass, atoms", "A mix of weighting types is invalid if only the last one differs", Validate(MixedReversed)) && Passed;

  // The normalization of the mass fractions: 3, 1 -> 0.75, 0.25
  MDMaterial Normalize("Normalize");
  Normalize.SetDensity(1.0);
  Normalize.SetComponentByMassWeighting("Fe", 3.0);
  Normalize.SetComponentByMassWeighting("Al", 1.0);
  Passed = EvaluateTrue("Validate()", "mass fractions 3, 1", "The material is valid", Validate(Normalize)) && Passed;
  Passed = EvaluateNear("Validate()", "mass fractions 3, 1", "The first fraction is normalized to the sum", Normalize.GetComponentAt(0)->GetWeightingByMass(), 0.75, 1E-12) && Passed;
  Passed = EvaluateNear("Validate()", "mass fractions 3, 1", "The second fraction is normalized to the sum", Normalize.GetComponentAt(1)->GetWeightingByMass(), 0.25, 1E-12) && Passed;
  Passed = EvaluateTrue("Validate()", "mass fractions 3, 1", "The weighting type stays by mass", Normalize.GetComponentAt(0)->GetWeightingType() == MDMaterialComponentWeightingType::c_ByMass) && Passed;

  // By atoms is not normalized
  MDMaterial Atoms("Atoms");
  Atoms.SetDensity(1.0);
  Atoms.SetComponentByAtomicWeighting("H", 2);
  Atoms.SetComponentByAtomicWeighting("O", 1);
  Passed = EvaluateTrue("Validate()", "H2O", "The material is valid", Validate(Atoms)) && Passed;
  Passed = Evaluate("Validate()", "H2O", "The number of atoms stays", Atoms.GetComponentAt(0)->GetWeightingByAtoms(), 2) && Passed;

  MDMaterial SingleMass("SingleMass");
  SingleMass.SetDensity(1.0);
  SingleMass.SetComponentByMassWeighting("Fe", 0.5);
  Validate(SingleMass);
  // Defect GEO-F-06: One mass fraction is not normalized to one, since the normalization only runs for at least two components
  Passed = ExpectedToFail(EvaluateNear("Validate()", "one component with mass fraction 0.5", "The only mass fraction is normalized to one", SingleMass.GetComponentAt(0)->GetWeightingByMass(), 1.0, 1E-12)) && Passed;

  // The given radiation length stays
  MDMaterial WithLength("WithLength");
  WithLength.SetDensity(2.7);
  WithLength.SetRadiationLength(1234.5);
  WithLength.SetComponentByAtomicWeighting("Al", 1);
  Passed = EvaluateTrue("Validate()", "given radiation length", "The material is valid", Validate(WithLength)) && Passed;
  Passed = EvaluateNear("Validate()", "given radiation length", "The value of the user is not replaced", WithLength.GetRadiationLength(), 1234.5, 1E-9) && Passed;

  // The directories get their default values
  MDMaterial Directories;
  Directories.SetName("Directories");
  Directories.SetDensity(1.0);
  Directories.SetComponentByAtomicWeighting("H", 1);
  Validate(Directories);
  MString Stored = g_MEGAlibPath + "/resource/examples/geomega/materials";
  // Defect GEO-F-08: The default directory is a path which does not exist any more
  Passed = ExpectedToFail(EvaluateTrue("Validate()", "no directories set", "The default cross section directory is the one with the materials", Directories.GetDefaultCrossSectionFileDirectory() == Stored)) && Passed;
  // Defect GEO-F-08: The default directory is a path which does not exist any more
  Passed = ExpectedToFail(EvaluateTrue("Validate()", "no directories set", "The cross section directory is the one with the materials", Directories.GetCrossSectionFileDirectory() == Stored)) && Passed;

  MDMaterial NamedDirectories("NamedDirectories");
  NamedDirectories.SetDensity(1.0);
  NamedDirectories.SetComponentByAtomicWeighting("H", 1);
  Validate(NamedDirectories);
  // Defect GEO-F-07: The name constructor leaves the default directory empty, so that the validation does not set it
  Passed = ExpectedToFail(EvaluateTrue("Validate()", "name constructor", "The validation sets the default cross section directory", NamedDirectories.GetDefaultCrossSectionFileDirectory() != "")) && Passed;

  // The directories given by the user stay
  MDMaterial GivenDirectories("GivenDirectories");
  GivenDirectories.SetDensity(1.0);
  GivenDirectories.SetComponentByAtomicWeighting("H", 1);
  GivenDirectories.SetDefaultCrossSectionFileDirectory("/nonexisting/default");
  GivenDirectories.SetCrossSectionFileDirectory("/nonexisting/new");
  Validate(GivenDirectories);
  Passed = EvaluateTrue("Validate()", "given directories", "The default cross section directory stays", GivenDirectories.GetDefaultCrossSectionFileDirectory() == "/nonexisting/default") && Passed;
  Passed = EvaluateTrue("Validate()", "given directories", "The cross section directory stays", GivenDirectories.GetCrossSectionFileDirectory() == "/nonexisting/new") && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the hash
bool UTDMaterial::TestHash()
{
  bool Passed = true;

  // Fill and validate a material
  auto Validated = [this](MDMaterial& M, double Density, double Length, double Weighting) {
    M.SetDensity(Density);
    M.SetRadiationLength(Length);
    M.SetComponentByMassWeighting("Fe", Weighting);
    M.SetComponentByMassWeighting("Al", 1.0);
    DisableDefaultStreams();
    M.Validate();
    EnableDefaultStreams();
  };

  MDMaterial Base("Base");
  Validated(Base, 1.0, 10.0, 1.0);
  Passed = EvaluateTrue("GetHash()", "validated", "The hash is set by the validation", Base.GetHash() != 0UL) && Passed;

  // The name is not part of the hash, the cross section files have the name
  MDMaterial Same("OtherName");
  Validated(Same, 1.0, 10.0, 1.0);
  Passed = Evaluate("GetHash()", "same content, other name", "The name does not change the hash", Same.GetHash(), Base.GetHash()) && Passed;

  // Every other value is part of it
  MDMaterial OtherDensity("Base");
  Validated(OtherDensity, 1.001, 10.0, 1.0);
  Passed = EvaluateTrue("GetHash()", "other density", "The density changes the hash", OtherDensity.GetHash() != Base.GetHash()) && Passed;

  MDMaterial OtherLength("Base");
  Validated(OtherLength, 1.0, 10.001, 1.0);
  Passed = EvaluateTrue("GetHash()", "other radiation length", "The radiation length changes the hash", OtherLength.GetHash() != Base.GetHash()) && Passed;

  MDMaterial OtherWeight("Base");
  Validated(OtherWeight, 1.0, 10.0, 1.001);
  Passed = EvaluateTrue("GetHash()", "other weighting", "The weighting changes the hash", OtherWeight.GetHash() != Base.GetHash()) && Passed;

  // Other weighting type, same numbers
  MDMaterial ByAtoms("Base");
  ByAtoms.SetDensity(1.0);
  ByAtoms.SetRadiationLength(10.0);
  ByAtoms.SetComponentByAtomicWeighting("Fe", 1);
  ByAtoms.SetComponentByAtomicWeighting("Al", 1);
  MDMaterial ByMass("Base");
  ByMass.SetDensity(1.0);
  ByMass.SetRadiationLength(10.0);
  ByMass.SetComponentByMassWeighting("Fe", 1.0);
  DisableDefaultStreams();
  ByAtoms.Validate();
  ByMass.Validate();
  EnableDefaultStreams();
  Passed = EvaluateTrue("GetHash()", "other number of components", "The components change the hash", ByAtoms.GetHash() != ByMass.GetHash()) && Passed;

  // A second validation of an unchanged material keeps the hash - the cross section files depend on it
  MDMaterial Twice("Twice");
  Twice.SetDensity(1.0);
  Twice.SetComponentByMassWeighting("Fe", 0.1);
  Twice.SetComponentByMassWeighting("Al", 0.2);
  Twice.SetComponentByMassWeighting("O", 0.7);
  DisableDefaultStreams();
  Twice.Validate();
  unsigned long FirstHash = Twice.GetHash();
  Twice.Validate();
  EnableDefaultStreams();
  Passed = Evaluate("GetHash()", "second validation", "The hash does not change", Twice.GetHash(), FirstHash) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the mass fractions of the ROOT mixture against hand computed values
bool UTDMaterial::TestRootMediumComposition()
{
  bool Passed = true;

  // Water: w(H) = 2*1.0079/(2*1.0079 + 15.9994) and w(O) = 15.9994/(2*1.0079 + 15.9994)
  MDMaterial Water("UTWater");
  Water.SetDensity(0.998);
  Water.SetComponentByAtomicWeighting("H", 2);
  Water.SetComponentByAtomicWeighting("O", 1);
  TGeoMedium* Medium = Water.GetRootMedium();
  Passed = EvaluateTrue("GetRootMedium()", "water", "A medium exists", Medium != nullptr) && Passed;
  Passed = EvaluateTrue("GetRootMedium()", "water", "The same medium is returned the second time", Water.GetRootMedium() == Medium) && Passed;
  TGeoMixture* Mixture = dynamic_cast<TGeoMixture*>(Medium->GetMaterial());
  Passed = EvaluateTrue("GetRootMedium()", "water", "The material is a mixture", Mixture != nullptr) && Passed;
  if (Mixture == nullptr) return false;

  const double WaterMass = 2*1.0079 + 15.9994;
  Passed = EvaluateTrue("GetRootMedium()", "water", "The name is the name of the material", MString(Medium->GetName()) == "UTWater") && Passed;
  Passed = EvaluateNear("GetRootMedium()", "water", "The density is stored", Mixture->GetDensity(), 0.998, 1E-12) && Passed;
  Passed = Evaluate("GetRootMedium()", "water", "There are two elements", Mixture->GetNelements(), 2) && Passed;
  Passed = EvaluateNear("GetRootMedium()", "water", "H has its atomic number", Mixture->GetZmixt()[0], 1.0, 1E-12) && Passed;
  Passed = EvaluateNear("GetRootMedium()", "water", "O has its atomic number", Mixture->GetZmixt()[1], 8.0, 1E-12) && Passed;
  Passed = EvaluateNear("GetRootMedium()", "water", "H has its atomic weight", Mixture->GetAmixt()[0], 1.0079, 1E-12) && Passed;
  Passed = EvaluateNear("GetRootMedium()", "water", "O has its atomic weight", Mixture->GetAmixt()[1], 15.9994, 1E-12) && Passed;
  Passed = EvaluateNear("GetRootMedium()", "water", "The mass fraction of H is 0.11190 by hand", Mixture->GetWmixt()[0], 2*1.0079/WaterMass, 1E-12) && Passed;
  Passed = EvaluateNear("GetRootMedium()", "water", "The mass fraction of O is 0.88810 by hand", Mixture->GetWmixt()[1], 15.9994/WaterMass, 1E-12) && Passed;
  Passed = Evaluate("GetRootMedium()", "water", "H has two atoms", Mixture->GetNmixt()[0], 2) && Passed;
  Passed = Evaluate("GetRootMedium()", "water", "O has one atom", Mixture->GetNmixt()[1], 1) && Passed;

  // BGO: Bi4Ge3O12 with 4*208.9804 + 3*72.64 + 12*15.9994 = 1245.74
  MDMaterial BGO("UTBGO");
  BGO.SetDensity(7.13);
  BGO.SetComponentByAtomicWeighting("Bi", 4);
  BGO.SetComponentByAtomicWeighting("Ge", 3);
  BGO.SetComponentByAtomicWeighting("O", 12);
  TGeoMixture* BGOMixture = dynamic_cast<TGeoMixture*>(BGO.GetRootMedium()->GetMaterial());
  const double BGOMass = 4*208.9804 + 3*72.64 + 12*15.9994;
  Passed = EvaluateNear("GetRootMedium()", "BGO", "The mass fraction of Bi is 0.67102 by hand", BGOMixture->GetWmixt()[0], 4*208.9804/BGOMass, 1E-12) && Passed;
  Passed = EvaluateNear("GetRootMedium()", "BGO", "The mass fraction of Ge is 0.17493 by hand", BGOMixture->GetWmixt()[1], 3*72.64/BGOMass, 1E-12) && Passed;
  Passed = EvaluateNear("GetRootMedium()", "BGO", "The mass fraction of O is 0.15412 by hand", BGOMixture->GetWmixt()[2], 12*15.9994/BGOMass, 1E-12) && Passed;

  // Explicit isotopes by atoms: 2*10.5 + 3*20.5 = 82.5
  MDMaterial Isotopes("UTIsotopes");
  Isotopes.SetDensity(1.5);
  Isotopes.SetComponentByAtomicWeighting(10.5, 5, 2);
  Isotopes.SetComponentByAtomicWeighting(20.5, 10, 3);
  TGeoMixture* IsotopesMixture = dynamic_cast<TGeoMixture*>(Isotopes.GetRootMedium()->GetMaterial());
  Passed = EvaluateNear("GetRootMedium()", "isotopes", "The mass fraction is 21/82.5 by hand", IsotopesMixture->GetWmixt()[0], 21.0/82.5, 1E-12) && Passed;
  Passed = EvaluateNear("GetRootMedium()", "isotopes", "The mass fraction is 61.5/82.5 by hand", IsotopesMixture->GetWmixt()[1], 61.5/82.5, 1E-12) && Passed;
  Passed = EvaluateNear("GetRootMedium()", "isotopes", "The atomic weight is the given one", IsotopesMixture->GetAmixt()[0], 10.5, 1E-12) && Passed;

  // By mass: after the validation 0.7556 + 0.2315 + 0.0129 = 1 and the mass fractions go unchanged to ROOT
  MDMaterial Air("UTAir");
  Air.SetDensity(1.2041E-3);
  Air.SetComponentByMassWeighting("N", 0.7556);
  Air.SetComponentByMassWeighting("O", 0.2315);
  Air.SetComponentByMassWeighting("Ar", 0.0129);
  DisableDefaultStreams();
  Air.Validate();
  EnableDefaultStreams();
  TGeoMixture* AirMixture = dynamic_cast<TGeoMixture*>(Air.GetRootMedium()->GetMaterial());
  Passed = EvaluateNear("GetRootMedium()", "air", "The mass fraction of N is the given one", AirMixture->GetWmixt()[0], 0.7556, 1E-12) && Passed;
  Passed = EvaluateNear("GetRootMedium()", "air", "The mass fraction of O is the given one", AirMixture->GetWmixt()[1], 0.2315, 1E-12) && Passed;
  Passed = EvaluateNear("GetRootMedium()", "air", "The mass fraction of Ar is the given one", AirMixture->GetWmixt()[2], 0.0129, 1E-12) && Passed;
  Passed = EvaluateNear("GetRootMedium()", "air", "The density is stored", AirMixture->GetDensity(), 1.2041E-3, 1E-12) && Passed;

  // By mass with a sum different from one: 3, 1 -> 0.75, 0.25
  MDMaterial Scaled("UTScaled");
  Scaled.SetDensity(3.0);
  Scaled.SetComponentByMassWeighting("Fe", 3.0);
  Scaled.SetComponentByMassWeighting("Al", 1.0);
  DisableDefaultStreams();
  Scaled.Validate();
  EnableDefaultStreams();
  TGeoMixture* ScaledMixture = dynamic_cast<TGeoMixture*>(Scaled.GetRootMedium()->GetMaterial());
  Passed = EvaluateNear("GetRootMedium()", "scaled", "The mass fraction is normalized", ScaledMixture->GetWmixt()[0], 0.75, 1E-12) && Passed;
  Passed = EvaluateNear("GetRootMedium()", "scaled", "The mass fraction is normalized", ScaledMixture->GetWmixt()[1], 0.25, 1E-12) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the radiation length against the formula and published values
bool UTDMaterial::TestRadiationLength()
{
  bool Passed = true;

  // Published values: PDG, Atomic and nuclear properties of materials, radiation length in g/cm2
  struct Reference { const char* Name; double Density; vector<ETElement> Elements; double PublishedGramPerCm2; };
  const vector<Reference> References = {
    { "Al", 2.7, {{1.0, 13, 26.9815}}, 24.01 },
    { "Si", 2.33, {{1.0, 14, 28.0855}}, 21.82 },
    { "Fe", 7.87, {{1.0, 26, 55.845}}, 13.84 },
    { "Cu", 8.96, {{1.0, 29, 63.546}}, 12.86 },
    { "Ge", 5.323, {{1.0, 32, 72.64}}, 12.25 },
    { "W", 19.3, {{1.0, 74, 183.84}}, 6.76 },
    { "Pb", 11.35, {{1.0, 82, 207.2}}, 6.37 }
  };

  // Measured: The stored ROOT constant 1.39621E-3 differs from 1/716.408 by 2.6E-4, thus the limit 3E-4
  const double FormulaLimit = 3E-4;
  // Measured: The largest difference to the PDG table is 0.19% at Al - the PDG table has tabulated Lrad for light elements
  const double PublishedLimit = 0.002;

  for (const Reference& R : References) {
    MDMaterial M(MString("UTX0") + R.Name);
    M.SetDensity(R.Density);
    M.SetComponentByAtomicWeighting(R.Name, 1);
    DisableDefaultStreams();
    M.Validate();
    EnableDefaultStreams();
    double Formula = GetRadiationLengthByFormula(R.Density, R.Elements);
    Passed = EvaluateNear("Validate()", R.Name, "The radiation length is the one of the Rossi-Greisen formula", M.GetRadiationLength(), Formula, FormulaLimit*Formula) && Passed;
    Passed = EvaluateNear("Validate()", R.Name, "The radiation length in g/cm2 is the published one", M.GetRadiationLength()*R.Density, R.PublishedGramPerCm2, PublishedLimit*R.PublishedGramPerCm2) && Passed;
  }

  // Compounds by atoms: Water with mass fractions by hand, PDG: 36.08 g/cm2
  const double WaterMass = 2*1.0079 + 15.9994;
  MDMaterial Water("UTX0Water");
  Water.SetDensity(1.0);
  Water.SetComponentByAtomicWeighting("H", 2);
  Water.SetComponentByAtomicWeighting("O", 1);
  DisableDefaultStreams();
  Water.Validate();
  EnableDefaultStreams();
  double WaterFormula = GetRadiationLengthByFormula(1.0, {{2*1.0079/WaterMass, 1, 1.0079}, {15.9994/WaterMass, 8, 15.9994}});
  Passed = EvaluateNear("Validate()", "water", "The radiation length is the one of the Rossi-Greisen formula", Water.GetRadiationLength(), WaterFormula, FormulaLimit*WaterFormula) && Passed;
  // Measured: Water deviates by 0.89% from the published value - the formula is only an approximation for hydrogen
  Passed = EvaluateNear("Validate()", "water", "The radiation length in g/cm2 is the published one", Water.GetRadiationLength(), 36.08, 0.009*36.08) && Passed;

  // Compounds by mass: Air, PDG: 36.62 g/cm2
  MDMaterial Air("UTX0Air");
  Air.SetDensity(1.2041E-3);
  Air.SetComponentByMassWeighting("N", 0.7556);
  Air.SetComponentByMassWeighting("O", 0.2315);
  Air.SetComponentByMassWeighting("Ar", 0.0129);
  DisableDefaultStreams();
  Air.Validate();
  EnableDefaultStreams();
  double AirFormula = GetRadiationLengthByFormula(1.2041E-3, {{0.7556, 7, 14.0067}, {0.2315, 8, 15.9994}, {0.0129, 18, 39.948}});
  Passed = EvaluateNear("Validate()", "air", "The radiation length is the one of the Rossi-Greisen formula", Air.GetRadiationLength(), AirFormula, FormulaLimit*AirFormula) && Passed;
  // Measured: Air deviates by 0.40% from the published value - the formula is only an approximation for light elements
  Passed = EvaluateNear("Validate()", "air", "The radiation length in g/cm2 is the published one", Air.GetRadiationLength()*1.2041E-3, 36.62, 0.0045*36.62) && Passed;

  // The radiation length scales inverse with the density
  MDMaterial Dense("UTX0Dense");
  Dense.SetDensity(5.4);
  Dense.SetComponentByAtomicWeighting("Al", 1);
  DisableDefaultStreams();
  Dense.Validate();
  EnableDefaultStreams();
  double AluFormula = GetRadiationLengthByFormula(5.4, {{1.0, 13, 26.9815}});
  Passed = EvaluateNear("Validate()", "twice the density of Al", "The radiation length is half as long", Dense.GetRadiationLength(), AluFormula, FormulaLimit*AluFormula) && Passed;

  MDMaterial Half("UTX0Half");
  Half.SetDensity(2.7);
  Half.SetComponentByMassWeighting("Al", 0.5);
  DisableDefaultStreams();
  Half.Validate();
  EnableDefaultStreams();
  double HalfFormula = GetRadiationLengthByFormula(2.7, {{1.0, 13, 26.9815}});
  // Defect GEO-F-06: One component by mass with a fraction different from one leads to a wrong radiation length
  Passed = ExpectedToFail(EvaluateNear("Validate()", "only Al with mass fraction 0.5", "The radiation length is the one of pure aluminum", Half.GetRadiationLength(), HalfFormula, FormulaLimit*HalfFormula)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the IDs of the ROOT media
bool UTDMaterial::TestRootMediumIDs()
{
  bool Passed = true;

  MDMaterial First("UTIDFirst");
  First.SetDensity(1.0);
  First.SetComponentByAtomicWeighting("H", 1);
  MDMaterial Second("UTIDSecond");
  Second.SetDensity(1.0);
  Second.SetComponentByAtomicWeighting("He", 1);

  // Defect GEO-F-09: The medium gets the counter of all materials instead of the ID of its material, thus all media of a geometry share one ID
  Passed = ExpectedToFail(EvaluateTrue("GetRootMedium()", "two materials", "Two media have different IDs", First.GetRootMedium()->GetId() != Second.GetRootMedium()->GetId())) && Passed;
  // Defect GEO-F-09: The medium gets the counter of all materials instead of the ID of its material, thus all media of a geometry share one ID
  Passed = ExpectedToFail(Evaluate("GetRootMedium()", "first material", "The medium has the ID of the material", First.GetRootMedium()->GetId(), First.GetID())) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the clone interface
bool UTDMaterial::TestClones()
{
  bool Passed = true;

  MDMaterial Template("Template");
  Template.SetDensity(2.5);
  Template.SetRadiationLength(11.5);
  Template.SetComponentByAtomicWeighting("Fe", 2);
  Template.SetComponentByMassWeighting(14.5, 7, 0.5);
  Template.SetCrossSectionFileDirectory("/nonexisting/template");

  // The list of clones
  MDMaterial FreshClone("FreshClone");
  MDMaterial OwnClone("OwnClone");
  OwnClone.SetDensity(9.5);
  OwnClone.SetRadiationLength(99.5);
  OwnClone.SetComponentByAtomicWeighting("Pb", 1);
  OwnClone.SetCrossSectionFileDirectory("/nonexisting/own");
  MDMaterial Stranger("Stranger");

  Passed = Evaluate("GetNClones()", "no clone", "There is no clone at the start", Template.GetNClones(), 0U) && Passed;
  Passed = Evaluate("GetCloneId()", "no clone", "A material which is no clone has no ID", Template.GetCloneId(&FreshClone), numeric_limits<unsigned int>::max()) && Passed;
  Template.AddClone(&FreshClone);
  Template.AddClone(&OwnClone);
  Passed = Evaluate("AddClone()", "two clones", "There are two clones", Template.GetNClones(), 2U) && Passed;
  Passed = EvaluateTrue("GetCloneAt()", "index 0", "The first clone is the first one added", Template.GetCloneAt(0) == &FreshClone) && Passed;
  Passed = EvaluateTrue("GetCloneAt()", "index n - 1", "The second clone is the last one added", Template.GetCloneAt(1) == &OwnClone) && Passed;
  Passed = Evaluate("GetCloneId()", "first clone", "The ID is the index", Template.GetCloneId(&FreshClone), 0U) && Passed;
  Passed = Evaluate("GetCloneId()", "second clone", "The ID is the index", Template.GetCloneId(&OwnClone), 1U) && Passed;
  Passed = Evaluate("GetCloneId()", "other material", "A material which is no clone has no ID", Template.GetCloneId(&Stranger), numeric_limits<unsigned int>::max()) && Passed;
  Passed = Evaluate("GetCloneId()", "nullptr", "A null pointer has no ID", Template.GetCloneId(nullptr), numeric_limits<unsigned int>::max()) && Passed;

  // The setter is only reachable, the getter of the template has no definition
  Stranger.SetCloneTemplate(&Template);
  Passed = Evaluate("SetCloneTemplate()", "template of another material", "The list of clones of the template does not change", Template.GetNClones(), 2U) && Passed;

  // Defect GEO-F-26: An index behind the end stops the program with a ROOT Fatal
  Passed = ExpectedToCrash(EvaluateTrue("GetCloneAt()", "index n", "The first index after the end returns no material, and the program goes on", Template.GetCloneAt(2) == nullptr)) && Passed;

  // Copying the data only sets what is not set
  Passed = EvaluateTrue("CopyDataToClones()", "two clones", "The copy succeeds", Template.CopyDataToClones()) && Passed;

  Passed = EvaluateNear("CopyDataToClones()", "fresh clone", "The density is copied", FreshClone.GetDensity(), 2.5, 1E-12) && Passed;
  Passed = EvaluateNear("CopyDataToClones()", "fresh clone", "The radiation length is copied", FreshClone.GetRadiationLength(), 11.5, 1E-12) && Passed;
  Passed = Evaluate("CopyDataToClones()", "fresh clone", "The components are copied", FreshClone.GetNComponents(), 2U) && Passed;
  Passed = EvaluateTrue("CopyDataToClones()", "fresh clone", "The cross section directory is copied", FreshClone.GetCrossSectionFileDirectory() == "/nonexisting/template") && Passed;
  Passed = Evaluate("CopyDataToClones()", "fresh clone", "The first component is the one of the template", FreshClone.GetComponentAt(0)->GetAtomicNumber(), 26) && Passed;
  Passed = EvaluateTrue("CopyDataToClones()", "fresh clone", "The second component has the type of the template", FreshClone.GetComponentAt(1)->GetWeightingType() == MDMaterialComponentWeightingType::c_ByMass) && Passed;
  Passed = EvaluateTrue("CopyDataToClones()", "fresh clone", "The components are deep copies", FreshClone.GetComponentAt(0) != Template.GetComponentAt(0)) && Passed;
  FreshClone.GetComponentAt(0)->SetWeightingByAtoms(9);
  Passed = Evaluate("CopyDataToClones()", "fresh clone", "A change of the copy does not reach the template", Template.GetComponentAt(0)->GetWeightingByAtoms(), 2) && Passed;

  Passed = EvaluateNear("CopyDataToClones()", "clone with own values", "The own density stays", OwnClone.GetDensity(), 9.5, 1E-12) && Passed;
  Passed = EvaluateNear("CopyDataToClones()", "clone with own values", "The own radiation length stays", OwnClone.GetRadiationLength(), 99.5, 1E-12) && Passed;
  Passed = Evaluate("CopyDataToClones()", "clone with own values", "The own components stay", OwnClone.GetNComponents(), 1U) && Passed;
  Passed = EvaluateTrue("CopyDataToClones()", "clone with own values", "The own directory stays", OwnClone.GetCrossSectionFileDirectory() == "/nonexisting/own") && Passed;

  // A second copy does not add components
  Passed = EvaluateTrue("CopyDataToClones()", "second call", "The copy succeeds", Template.CopyDataToClones()) && Passed;
  Passed = Evaluate("CopyDataToClones()", "second call", "The components are not added twice", FreshClone.GetNComponents(), 2U) && Passed;

  // No clones
  MDMaterial Lonely("Lonely");
  Passed = EvaluateTrue("CopyDataToClones()", "no clones", "The copy succeeds", Lonely.CopyDataToClones()) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test reading the cross sections from fixture files
bool UTDMaterial::TestCrossSectionFiles()
{
  bool Passed = true;

  if (PrepareTemporaryDirectory("xsection") == false || PrepareTemporaryDirectory("xdefault") == false) {
    Passed = EvaluateTrue("LoadCrossSections()", "fixture", "The fixture directories can be created", false) && Passed;
    return Passed;
  }
  const MString Directory = GetTemporaryDirectoryName("xsection");
  const MString DefaultDirectory = GetTemporaryDirectoryName("xdefault");

  MDMaterial M("UTCross");
  M.SetDensity(1.0);
  M.SetRadiationLength(10.0);
  M.SetComponentByAtomicWeighting("Al", 1);
  M.SetCrossSectionFileDirectory(Directory);
  M.SetDefaultCrossSectionFileDirectory(DefaultDirectory);

  // No files
  DisableDefaultStreams();
  bool Validated = M.Validate();
  EnableDefaultStreams();
  Passed = EvaluateTrue("Validate()", "no cross section files", "Missing files are not an error of the validation", Validated) && Passed;
  Passed = EvaluateFalse("AreCrossSectionsPresent()", "no cross section files", "There are no cross sections", M.AreCrossSectionsPresent()) && Passed;

  auto Quiet = [this](double Value) { EnableDefaultStreams(); return Value; };
  DisableDefaultStreams();
  Passed = EvaluateNear("GetAbsorptionCoefficient()", "no cross section files", "The result is zero", Quiet(M.GetAbsorptionCoefficient(15.0)), 0.0, 0.0) && Passed;
  DisableDefaultStreams();
  Passed = EvaluateNear("GetPhotoAbsorptionCoefficient()", "no cross section files", "The result is zero", Quiet(M.GetPhotoAbsorptionCoefficient(15.0)), 0.0, 0.0) && Passed;
  DisableDefaultStreams();
  Passed = EvaluateNear("GetComptonAbsorptionCoefficient()", "no cross section files", "The result is zero", Quiet(M.GetComptonAbsorptionCoefficient(15.0)), 0.0, 0.0) && Passed;
  DisableDefaultStreams();
  Passed = EvaluateNear("GetPairAbsorptionCoefficient()", "no cross section files", "The result is zero", Quiet(M.GetPairAbsorptionCoefficient(15.0)), 0.0, 0.0) && Passed;
  DisableDefaultStreams();
  Passed = EvaluateNear("GetRayleighAbsorptionCoefficient()", "no cross section files", "The result is zero", Quiet(M.GetRayleighAbsorptionCoefficient(15.0)), 0.0, 0.0) && Passed;

  // All files in the new directory, the process numbers are 1: Total, 2: Photo, 3: Compton, 4: Pair, 5: Rayleigh
  Passed = EvaluateTrue("LoadCrossSections()", "fixture", "The fixture files can be written", WriteCrossSections(Directory, "UTCross", M.GetHash())) && Passed;
  DisableDefaultStreams();
  bool Loaded = M.LoadCrossSections();
  EnableDefaultStreams();
  Passed = EvaluateTrue("LoadCrossSections()", "all files", "The files are loaded", Loaded) && Passed;
  Passed = EvaluateTrue("AreCrossSectionsPresent()", "all files", "The cross sections are present", M.AreCrossSectionsPresent()) && Passed;

  // Values on the axis and linear interpolation between them (float accuracy)
  const double Tolerance = 1E-5;
  Passed = EvaluateNear("GetAbsorptionCoefficient()", "10 keV", "The value on the axis comes from the total file", M.GetAbsorptionCoefficient(10.0), 1.0, Tolerance) && Passed;
  Passed = EvaluateNear("GetAbsorptionCoefficient()", "20 keV", "The value on the axis comes from the total file", M.GetAbsorptionCoefficient(20.0), 3.0, Tolerance) && Passed;
  Passed = EvaluateNear("GetAbsorptionCoefficient()", "15 keV", "The value is interpolated linearly: (1 + 3)/2", M.GetAbsorptionCoefficient(15.0), 2.0, Tolerance) && Passed;
  Passed = EvaluateNear("GetAbsorptionCoefficient()", "25 keV", "The value is interpolated linearly: 3 + 4*5/20", M.GetAbsorptionCoefficient(25.0), 4.0, Tolerance) && Passed;
  Passed = EvaluateNear("GetPhotoAbsorptionCoefficient()", "15 keV", "The value comes from the photo file: 2*(1 + 3)/2", M.GetPhotoAbsorptionCoefficient(15.0), 4.0, Tolerance) && Passed;
  Passed = EvaluateNear("GetComptonAbsorptionCoefficient()", "15 keV", "The value comes from the Compton file: 3*(1 + 3)/2", M.GetComptonAbsorptionCoefficient(15.0), 6.0, Tolerance) && Passed;
  Passed = EvaluateNear("GetPairAbsorptionCoefficient()", "15 keV", "The value comes from the pair file: 4*(1 + 3)/2", M.GetPairAbsorptionCoefficient(15.0), 8.0, Tolerance) && Passed;
  Passed = EvaluateNear("GetRayleighAbsorptionCoefficient()", "15 keV", "The value comes from the Rayleigh file: 5*(1 + 3)/2", M.GetRayleighAbsorptionCoefficient(15.0), 10.0, Tolerance) && Passed;

  // The first and the last value are used outside of the axis
  Passed = EvaluateNear("GetAbsorptionCoefficient()", "5 keV", "Below the axis the lowest value is used", M.GetAbsorptionCoefficient(5.0), 1.0, Tolerance) && Passed;
  Passed = EvaluateNear("GetAbsorptionCoefficient()", "0 keV", "Below the axis the lowest value is used", M.GetAbsorptionCoefficient(0.0), 1.0, Tolerance) && Passed;
  Passed = EvaluateNear("GetAbsorptionCoefficient()", "40 keV", "The last value on the axis is used", M.GetAbsorptionCoefficient(40.0), 7.0, Tolerance) && Passed;
  Passed = EvaluateNear("GetAbsorptionCoefficient()", "1000 keV", "Above the axis the highest value is used", M.GetAbsorptionCoefficient(1000.0), 7.0, Tolerance) && Passed;

  // One file is missing: There are no cross sections any more and no stale values
  MString PairFile = Directory + "/Xsection.Pair.UTCross.rsp";
  Passed = EvaluateTrue("LoadCrossSections()", "fixture", "One file can be removed", RemoveTemporaryFile(PairFile)) && Passed;
  DisableDefaultStreams();
  Loaded = M.LoadCrossSections();
  EnableDefaultStreams();
  Passed = EvaluateTrue("LoadCrossSections()", "pair file missing", "A missing file is no error", Loaded) && Passed;
  Passed = EvaluateFalse("AreCrossSectionsPresent()", "pair file missing", "The cross sections are not complete", M.AreCrossSectionsPresent()) && Passed;
  DisableDefaultStreams();
  Passed = EvaluateNear("GetAbsorptionCoefficient()", "pair file missing", "No stale value is returned", Quiet(M.GetAbsorptionCoefficient(15.0)), 0.0, 0.0) && Passed;

  // The fall back to the default directory for the missing file: Twice the value
  Passed = EvaluateTrue("LoadCrossSections()", "fixture", "The default files can be written", WriteCrossSections(DefaultDirectory, "UTCross", M.GetHash(), 2.0)) && Passed;
  DisableDefaultStreams();
  Loaded = M.LoadCrossSections();
  EnableDefaultStreams();
  Passed = EvaluateTrue("LoadCrossSections()", "fall back", "The files are loaded", Loaded) && Passed;
  Passed = EvaluateTrue("AreCrossSectionsPresent()", "fall back", "The cross sections are complete", M.AreCrossSectionsPresent()) && Passed;
  Passed = EvaluateNear("GetPairAbsorptionCoefficient()", "fall back", "The missing file comes from the default directory: 2*4*2", M.GetPairAbsorptionCoefficient(15.0), 16.0, Tolerance) && Passed;
  Passed = EvaluateNear("GetAbsorptionCoefficient()", "fall back", "The existing file has the priority: 1*2", M.GetAbsorptionCoefficient(15.0), 2.0, Tolerance) && Passed;

  // A wrong hash: The files belong to another material
  Passed = EvaluateTrue("LoadCrossSections()", "fixture", "Files with a wrong hash can be written", WriteCrossSections(Directory, "UTCross", M.GetHash() + 1)) && Passed;
  DisableDefaultStreams();
  Loaded = M.LoadCrossSections();
  EnableDefaultStreams();
  Passed = EvaluateTrue("LoadCrossSections()", "wrong hash", "A wrong hash is no error", Loaded) && Passed;
  Passed = EvaluateFalse("AreCrossSectionsPresent()", "wrong hash", "The cross sections are not usable", M.AreCrossSectionsPresent()) && Passed;

  // Loading again after the fix works
  Passed = EvaluateTrue("LoadCrossSections()", "fixture", "Files with the correct hash can be written", WriteCrossSections(Directory, "UTCross", M.GetHash())) && Passed;
  DisableDefaultStreams();
  Loaded = M.LoadCrossSections();
  EnableDefaultStreams();
  Passed = EvaluateTrue("AreCrossSectionsPresent()", "correct hash again", "The cross sections are usable again", Loaded == true && M.AreCrossSectionsPresent() == true) && Passed;

  // A file which is not a response file, once with and once without a usable file in the default directory
  Passed = EvaluateTrue("LoadCrossSections()", "fixture", "A corrupt file can be written", WriteTextFile(Directory + "/Xsection.Total.UTCross.rsp", "This is no response file\n")) && Passed;
  DisableDefaultStreams();
  Loaded = M.LoadCrossSections();
  EnableDefaultStreams();
  Passed = EvaluateTrue("LoadCrossSections()", "corrupt new file, good default file", "The default file is used", Loaded) && Passed;
  Passed = EvaluateNear("GetAbsorptionCoefficient()", "corrupt new file, good default file", "The value comes from the default directory: 2*2", M.GetAbsorptionCoefficient(15.0), 4.0, Tolerance) && Passed;

  Passed = EvaluateTrue("LoadCrossSections()", "fixture", "A corrupt file can be written", WriteTextFile(DefaultDirectory + "/Xsection.Total.UTCross.rsp", "This is no response file\n")) && Passed;
  DisableDefaultStreams();
  Loaded = M.LoadCrossSections(true);
  EnableDefaultStreams();
  Passed = EvaluateFalse("LoadCrossSections()", "corrupt new file, corrupt default file", "Unreadable files are an error", Loaded) && Passed;
  Passed = EvaluateFalse("AreCrossSectionsPresent()", "corrupt new file, corrupt default file", "There are no cross sections", M.AreCrossSectionsPresent()) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the cross section directories
bool UTDMaterial::TestCrossSectionDirectories()
{
  bool Passed = true;

  MDMaterial M("UTDirectories");
  M.SetDefaultCrossSectionFileDirectory("/some/default");
  Passed = EvaluateTrue("SetDefaultCrossSectionFileDirectory()", "/some/default", "The directory is stored", M.GetDefaultCrossSectionFileDirectory() == "/some/default") && Passed;
  M.SetCrossSectionFileDirectory("/some/new");
  Passed = EvaluateTrue("SetCrossSectionFileDirectory()", "/some/new", "The directory is stored", M.GetCrossSectionFileDirectory() == "/some/new") && Passed;
  Passed = EvaluateTrue("SetCrossSectionFileDirectory()", "/some/new", "The other directory does not change", M.GetDefaultCrossSectionFileDirectory() == "/some/default") && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the cross sections stored with MEGAlib against published values
bool UTDMaterial::TestStoredCrossSections()
{
  bool Passed = true;

  MDGeometry Geometry;
  Passed = EvaluateTrue("ScanSetupFile()", "materials of MEGAlib", "The geometry can be scanned", ScanGeometry(Geometry, GetGeometryHead(), "Stored.geo.setup")) && Passed;

  // The default directory has files for all materials of the file
  unsigned int NumberOfPresent = 0;
  for (unsigned int m = 0; m < Geometry.GetNMaterials(); ++m) {
    if (Geometry.GetMaterialAt(m)->AreCrossSectionsPresent() == true) {
      ++NumberOfPresent;
    }
  }
  // Defect GEO-F-17: The hash of no stored cross section file matches the hash of its material, thus none of them is used
  Passed = ExpectedToFail(Evaluate("AreCrossSectionsPresent()", "materials of MEGAlib", "The cross sections stored with MEGAlib match all their materials", NumberOfPresent, Geometry.GetNMaterials())) && Passed;

  // The stored values are read directly since the material refuses them
  // NIST XCOM, total attenuation with coherent scattering at 1 MeV in cm2/g
  struct Reference { const char* Name; double MassAttenuation; };
  const vector<Reference> References = {
    { "Aluminium", 6.146E-02 },
    { "Silicon", 6.361E-02 },
    { "Iron", 5.995E-02 },
    { "Copper", 5.901E-02 }
  };

  MString Directory = g_MEGAlibPath + "/resource/examples/geomega/materials/Xsection.";
  for (const Reference& R : References) {
    MDMaterial* M = Geometry.GetMaterial(R.Name);
    Passed = EvaluateTrue("GetMaterial()", R.Name, "The material exists", M != nullptr) && Passed;
    if (M == nullptr) continue;

    double Sum = 0;
    double Total = 0;
    for (const MString& Process : vector<MString>{ MString("Total"), MString("Photo"), MString("Compton"), MString("Pair"), MString("Rayleigh") }) {
      MResponseMatrixO1 CrossSection;
      Passed = EvaluateTrue("Read()", Directory + Process + "." + R.Name, "The stored file can be read", CrossSection.Read(Directory + Process + "." + R.Name + ".rsp")) && Passed;
      if (Process == "Total") {
        Total = CrossSection.GetInterpolated(1000.0);
      } else {
        Sum += CrossSection.GetInterpolated(1000.0);
      }
    }

    // Measured: Geant4 deviates by up to 0.15% from XCOM at 1 MeV for these light elements
    double Expected = R.MassAttenuation*M->GetDensity();
    Passed = EvaluateNear("Read()", R.Name, "The attenuation at 1 MeV is the published one", Total, Expected, 0.002*Expected) && Passed;

    // The numbers of the files have 7 digits
    Passed = EvaluateNear("Read()", R.Name, "The total is the sum of all processes", Total, Sum, 1E-6*Total) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the material lookups of a geometry
bool UTDMaterial::TestLookups()
{
  bool Passed = true;

  MString Materials;
  Materials += "Material First\nFirst.Density 1.5\nFirst.ComponentByAtoms H 2\nFirst.ComponentByAtoms O 1\n";
  Materials += "Material Second\nSecond.Density 2.5\nSecond.ComponentByMass Fe 0.25\nSecond.ComponentByMass Al 0.75\n";

  MDGeometry Geometry;
  Passed = EvaluateTrue("ScanSetupFile()", "two materials", "The geometry can be scanned", ScanMaterials(Geometry, Materials)) && Passed;

  // Three materials: The one of the world first
  Passed = Evaluate("GetNMaterials()", "three materials", "All materials are in the list", Geometry.GetNMaterials(), 3U) && Passed;
  Passed = EvaluateTrue("GetMaterialAt()", "index 0", "The first defined material comes first", Geometry.GetMaterialAt(0) != nullptr && Geometry.GetMaterialAt(0)->GetName() == "Nothing") && Passed;
  Passed = EvaluateTrue("GetMaterialAt()", "index n - 1", "The last defined material comes last", Geometry.GetMaterialAt(2) != nullptr && Geometry.GetMaterialAt(2)->GetName() == "Second") && Passed;
  DisableDefaultStreams();
  MDMaterial* OutOfBounds = Geometry.GetMaterialAt(3);
  EnableDefaultStreams();
  Passed = EvaluateTrue("GetMaterialAt()", "index n", "The index n is out of bounds and returns null", OutOfBounds == nullptr) && Passed;

  MDMaterial* First = Geometry.GetMaterial("First");
  Passed = EvaluateTrue("GetMaterial()", "First", "The material of this name is returned", First != nullptr && First == Geometry.GetMaterialAt(1)) && Passed;
  Passed = EvaluateTrue("GetMaterial()", "first", "The lookup is case sensitive", Geometry.GetMaterial("first") == nullptr) && Passed;
  Passed = EvaluateTrue("GetMaterial()", "Third", "An unknown name returns null", Geometry.GetMaterial("Third") == nullptr) && Passed;
  Passed = EvaluateTrue("GetMaterial()", "empty name", "An empty name returns null", Geometry.GetMaterial("") == nullptr) && Passed;

  Passed = Evaluate("GetMaterialIndex()", "Second", "The index of the material is returned", Geometry.GetMaterialIndex("Second"), 2U) && Passed;
  Passed = Evaluate("GetMaterialIndex()", "Third", "An unknown name returns the not-defined value", Geometry.GetMaterialIndex("Third"), g_UnsignedIntNotDefined) && Passed;

  // Values of the file
  if (First != nullptr) {
    Passed = EvaluateNear("ScanSetupFile()", "First", "The density comes from the file", First->GetDensity(), 1.5, 1E-12) && Passed;
    Passed = Evaluate("ScanSetupFile()", "First", "The number of atoms comes from the file", First->GetComponentAt(0)->GetWeightingByAtoms(), 2) && Passed;
  }

  // Unused materials: The two extra ones are not used by the only volume
  vector<MDMaterial*> Unused = Geometry.GetListOfUnusedMaterials();
  Passed = EvaluateSize("GetListOfUnusedMaterials()", "two extra materials", "The two materials without volumes are unused", Unused.size(), 2UL) && Passed;

  // A volume uses the material of the file
  MDGeometry VolumeGeometry;
  Passed = EvaluateTrue("ScanSetupFile()", "volume", "A volume can use a material", ScanMaterials(VolumeGeometry, Materials + "Volume Box\nBox.Material First\nBox.Shape BRIK 1 1 1\nBox.Mother World\n")) && Passed;
  MDVolume* Box = VolumeGeometry.GetVolume("Box");
  Passed = EvaluateTrue("ScanSetupFile()", "volume", "The volume has the material", Box != nullptr && Box->GetMaterial() == VolumeGeometry.GetMaterial("First")) && Passed;
  Passed = EvaluateFalse("ScanSetupFile()", "unknown material of a volume", "An unknown material is rejected", ScanMaterials(VolumeGeometry, Materials + "Volume Box\nBox.Material Third\nBox.Shape BRIK 1 1 1\nBox.Mother World\n")) && Passed;

  // The materials which are copies
  MDGeometry CopyGeometry;
  Passed = EvaluateTrue("ScanSetupFile()", "copy", "A material can be copied", ScanMaterials(CopyGeometry, Materials + "First.Copy Duplicate\n")) && Passed;
  MDMaterial* Duplicate = CopyGeometry.GetMaterial("Duplicate");
  Passed = EvaluateTrue("ScanSetupFile()", "copy", "The copy exists", Duplicate != nullptr) && Passed;
  if (Duplicate != nullptr) {
    Passed = EvaluateNear("ScanSetupFile()", "copy", "The copy has the density of the template", Duplicate->GetDensity(), 1.5, 1E-12) && Passed;
    Passed = Evaluate("ScanSetupFile()", "copy", "The copy has the components of the template", Duplicate->GetNComponents(), 2U) && Passed;
    Passed = Evaluate("ScanSetupFile()", "copy", "The copy is registered at the template", CopyGeometry.GetMaterial("First")->GetCloneId(Duplicate), 0U) && Passed;
    Passed = Evaluate("ScanSetupFile()", "copy", "The copy has the hash of the template", Duplicate->GetHash(), CopyGeometry.GetMaterial("First")->GetHash()) && Passed;
    Passed = EvaluateNear("ScanSetupFile()", "copy", "The copy has the radiation length of the template", Duplicate->GetRadiationLength(), CopyGeometry.GetMaterial("First")->GetRadiationLength(), 1E-9) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the material keywords of the geometry file
bool UTDMaterial::TestKeywords()
{
  bool Passed = true;

  // All ways to give a component
  {
    MString Materials;
    Materials += "Material Natural\nNatural.Density 1\nNatural.Component Fe 2\n";
    Materials += "Material Atoms\nAtoms.Density 1\nAtoms.ComponentByAtoms Fe 3\n";
    Materials += "Material Isotope\nIsotope.Density 1\nIsotope.ComponentByAtoms 27.5 13 4\n";
    Materials += "Material Mass\nMass.Density 1\nMass.ComponentByMass Fe 1\n";
    Materials += "Material MassIsotope\nMassIsotope.Density 1\nMassIsotope.ComponentByMass 27.5 13 1\n";
    MDGeometry Geometry;
    Passed = EvaluateTrue("ScanSetupFile()", "all component keywords", "All component keywords are accepted", ScanMaterials(Geometry, Materials)) && Passed;

    if (Geometry.GetMaterial("MassIsotope") != nullptr) {
      MDMaterialComponent* C = Geometry.GetMaterial("Natural")->GetComponentAt(0);
      Passed = EvaluateTrue("ScanSetupFile()", "Component Fe 2", "The old keyword is by atoms and natural", C->GetWeightingType() == MDMaterialComponentWeightingType::c_ByAtoms && C->HasNaturalIsotopeComposition() == true && C->GetWeightingByAtoms() == 2) && Passed;
      C = Geometry.GetMaterial("Atoms")->GetComponentAt(0);
      Passed = EvaluateTrue("ScanSetupFile()", "ComponentByAtoms Fe 3", "The keyword is by atoms and natural", C->GetWeightingType() == MDMaterialComponentWeightingType::c_ByAtoms && C->HasNaturalIsotopeComposition() == true && C->GetWeightingByAtoms() == 3) && Passed;
      C = Geometry.GetMaterial("Isotope")->GetComponentAt(0);
      Passed = EvaluateTrue("ScanSetupFile()", "ComponentByAtoms 27.5 13 4", "The keyword is by atoms and an isotope", C->GetWeightingType() == MDMaterialComponentWeightingType::c_ByAtoms && C->HasNaturalIsotopeComposition() == false && C->GetAtomicNumber() == 13 && C->GetWeightingByAtoms() == 4) && Passed;
      C = Geometry.GetMaterial("Mass")->GetComponentAt(0);
      Passed = EvaluateTrue("ScanSetupFile()", "ComponentByMass Fe 1", "The keyword is by mass and natural", C->GetWeightingType() == MDMaterialComponentWeightingType::c_ByMass && C->HasNaturalIsotopeComposition() == true) && Passed;
      C = Geometry.GetMaterial("MassIsotope")->GetComponentAt(0);
      Passed = EvaluateTrue("ScanSetupFile()", "ComponentByMass 27.5 13 1", "The keyword is by mass and an isotope", C->GetWeightingType() == MDMaterialComponentWeightingType::c_ByMass && C->HasNaturalIsotopeComposition() == false && C->GetAtomicNumber() == 13) && Passed;
      Passed = EvaluateNear("ScanSetupFile()", "ComponentByMass 27.5 13 1", "The atomic weight comes from the file", C->GetAtomicWeight(), 27.5, 1E-12) && Passed;
    }
  }

  // The radiation length comes from the file
  {
    MDGeometry Geometry;
    Passed = EvaluateTrue("ScanSetupFile()", "RadiationLength", "The keyword is accepted", ScanMaterials(Geometry, "Material L\nL.Density 1\nL.RadiationLength 12.5\nL.Component Fe 1\n")) && Passed;
    if (Geometry.GetMaterial("L") != nullptr) {
      Passed = EvaluateNear("ScanSetupFile()", "RadiationLength", "The value comes from the file", Geometry.GetMaterial("L")->GetRadiationLength(), 12.5, 1E-12) && Passed;
    }
  }

  // The sensitivity keyword is accepted without effect
  {
    MDGeometry Geometry;
    Passed = EvaluateTrue("ScanSetupFile()", "Sensitivity", "The redundant keyword is accepted", ScanMaterials(Geometry, "Material S\nS.Density 1\nS.Sensitivity 1\nS.Component Fe 1\n")) && Passed;
    if (Geometry.GetMaterial("S") != nullptr) {
      Passed = EvaluateTrue("ScanSetupFile()", "Sensitivity", "The flag stays not defined", Geometry.GetMaterial("S")->ToString().Contains(MString("S:") + g_IntNotDefined)) && Passed;
    }
  }

  // Everything which has to be rejected
  struct Bad { const char* Description; const char* Material; };
  const vector<Bad> Bads = {
    { "no density", "Material B\nB.Component Fe 1\n" },
    { "no component", "Material B\nB.Density 1\n" },
    { "unknown element by atoms", "Material B\nB.Density 1\nB.ComponentByAtoms Xx 1\n" },
    { "unknown element by mass", "Material B\nB.Density 1\nB.ComponentByMass Xx 1\n" },
    { "too few tokens by atoms", "Material B\nB.Density 1\nB.ComponentByAtoms Fe\n" },
    { "too many tokens by atoms", "Material B\nB.Density 1\nB.ComponentByAtoms 27 13 1 1\n" },
    { "too few tokens by mass", "Material B\nB.Density 1\nB.ComponentByMass Fe\n" },
    { "too many tokens by mass", "Material B\nB.Density 1\nB.ComponentByMass 27 13 1 1\n" },
    { "too many tokens of the density", "Material B\nB.Density 1 2\nB.Component Fe 1\n" },
    { "too many tokens of the radiation length", "Material B\nB.Density 1\nB.RadiationLength 1 2\nB.Component Fe 1\n" },
    { "unknown keyword", "Material B\nB.Density 1\nB.Component Fe 1\nB.Unknown 1\n" },
    { "mixed weighting types", "Material B\nB.Density 1\nB.ComponentByAtoms Fe 1\nB.ComponentByMass Al 0.5\n" },
    { "duplicate name", "Material B\nB.Density 1\nB.Component Fe 1\nMaterial B\nB.Density 1\nB.Component Fe 1\n" },
    { "duplicate name in other case", "Material B\nB.Density 1\nB.Component Fe 1\nMaterial b\nb.Density 1\nb.Component Fe 1\n" },
    { "copy with an existing name", "Material B\nB.Density 1\nB.Component Fe 1\nMaterial C\nC.Density 1\nC.Component Fe 1\nB.Copy C\n" },
    { "no name", "Material\n" },
    { "two names", "Material A B\n" }
  };
  for (const Bad& B : Bads) {
    MDGeometry Geometry;
    Passed = EvaluateFalse("ScanSetupFile()", B.Description, "The input is rejected", ScanMaterials(Geometry, B.Material)) && Passed;
  }

  {
    MDGeometry Geometry;
    bool Result = ScanMaterials(Geometry, "Material N\nN.Density 1\nN.Density -5\nN.Component Fe 1\n");
    // Defect GEO-F-14: The return value of the setter is ignored, a negative density after a valid one is accepted without error
    Passed = ExpectedToFail(EvaluateFalse("ScanSetupFile()", "negative density after a valid one", "A negative density is rejected", Result)) && Passed;
  }
  {
    MDGeometry Geometry;
    bool Result = ScanMaterials(Geometry, "Material N\nN.Density 1\nN.RadiationLength -5\nN.Component Fe 1\n");
    // Defect GEO-F-14: The return value of the setter is ignored, a negative radiation length is replaced by the computed one without error
    Passed = ExpectedToFail(EvaluateFalse("ScanSetupFile()", "negative radiation length", "A negative radiation length is rejected", Result)) && Passed;
  }
  {
    MDGeometry Geometry;
    bool Result = ScanMaterials(Geometry, "Material N\nN.Density 1\nN.ComponentByAtoms 27.5 13 1\nN.ComponentByAtoms 16.0 8 0\n");
    // Defect GEO-F-14: The return value of the setter is ignored, an invalid isotope is dropped from the material without error
    Passed = ExpectedToFail(EvaluateFalse("ScanSetupFile()", "isotope by atoms with zero atoms", "An invalid component is rejected", Result)) && Passed;
  }
  {
    MDGeometry Geometry;
    bool Result = ScanMaterials(Geometry, "Material N\nN.Density 1\nN.ComponentByMass 27.5 13 0.5\nN.ComponentByMass 16.0 8 -0.5\n");
    // Defect GEO-F-14: The return value of the setter is ignored, an invalid isotope is dropped from the material without error
    Passed = ExpectedToFail(EvaluateFalse("ScanSetupFile()", "isotope by mass with negative mass fraction", "An invalid component is rejected", Result)) && Passed;
  }
  {
    MDGeometry Geometry;
    bool Result = ScanMaterials(Geometry, "Material N\nN.Density 1\nN.ComponentByAtoms C 1\nN.ComponentByAtoms H 1.5\n");
    // Defect GEO-F-16: A non-integer number of atoms is silently truncated, CH1.5 becomes CH with a wrong mass fraction
    Passed = ExpectedToFail(EvaluateFalse("ScanSetupFile()", "1.5 atoms", "A non-integer number of atoms is not silently changed and thus rejected", Result)) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  // Batch mode first, otherwise ROOT connects to the display: a test must not depend on a display being available
  gROOT->SetBatch(true);
  TApplication ROOT("ROOT", 0, 0);
  gErrorIgnoreLevel = kWarning;

  UTDMaterial Test;
  return Test.Run() == true ? 0 : 1;
}


// UTDMaterial.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
