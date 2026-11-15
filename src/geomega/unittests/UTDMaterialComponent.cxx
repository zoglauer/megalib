/*
 * UTDMaterialComponent.cxx
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
#include <limits>
#include <new>
#include <vector>
using namespace std;

// MEGAlib libs:
#include "MDMaterialComponent.h"
#include "MString.h"
#include "MUnitTest.h"


////////////////////////////////////////////////////////////////////////////////


//! Give the test access to the protected element lookup
class AccessMaterialComponent : public MDMaterialComponent
{
public:
  using MDMaterialComponent::SetElement;
};


////////////////////////////////////////////////////////////////////////////////


//! Unit test of the single material component MDMaterialComponent
class UTDMaterialComponent : public MUnitTest
{
public:
  //! Default constructor
  UTDMaterialComponent() : MUnitTest("UTDMaterialComponent") {}
  //! Default destructor
  virtual ~UTDMaterialComponent() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Test the default state, also on memory filled with a pattern
  bool TestDefaultConstruction();
  //! Test the copy constructor
  bool TestCopy();
  //! Test the explicit isotope by atoms setter with its limits
  bool TestSetByAtomicWeightingIsotope();
  //! Test the natural composition by atoms setter
  bool TestSetByAtomicWeightingNatural();
  //! Test the explicit isotope by mass setter with its limits
  bool TestSetByMassWeightingIsotope();
  //! Test the natural composition by mass setter
  bool TestSetByMassWeightingNatural();
  //! Test the protected element lookup against the periodic table
  bool TestElementTable();
  //! Test the weighting setters and the rounding of the atoms getter
  bool TestWeightingSetters();
  //! Test the validation limits
  bool TestValidate();
  //! Test the string description
  bool TestToString();
  //! Test the Geomega output
  bool TestGetGeomega();
  //! Test the reuse of one object for several operations
  bool TestReuse();

  //! Return true if the component has exactly the given content
  bool HasContent(const MDMaterialComponent& C, double A, int Z, double Weighting, MDMaterialComponentWeightingType Type, bool Natural) const;
};


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTDMaterialComponent::Run()
{
  bool Passed = true;

  Passed = TestDefaultConstruction() && Passed;
  Passed = TestCopy() && Passed;
  Passed = TestSetByAtomicWeightingIsotope() && Passed;
  Passed = TestSetByAtomicWeightingNatural() && Passed;
  Passed = TestSetByMassWeightingIsotope() && Passed;
  Passed = TestSetByMassWeightingNatural() && Passed;
  Passed = TestElementTable() && Passed;
  Passed = TestWeightingSetters() && Passed;
  Passed = TestValidate() && Passed;
  Passed = TestToString() && Passed;
  Passed = TestGetGeomega() && Passed;
  Passed = TestReuse() && Passed;

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Return true if the component has exactly the given content
bool UTDMaterialComponent::HasContent(const MDMaterialComponent& C, double A, int Z, double Weighting, MDMaterialComponentWeightingType Type, bool Natural) const
{
  if (fabs(C.GetAtomicWeight() - A) > 1E-12) return false;
  if (C.GetAtomicNumber() != Z) return false;
  if (fabs(C.GetWeightingByMass() - Weighting) > 1E-12) return false;
  if (C.GetWeightingType() != Type) return false;
  if (C.HasNaturalIsotopeComposition() != Natural) return false;
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the default state, also on memory filled with a pattern
bool UTDMaterialComponent::TestDefaultConstruction()
{
  bool Passed = true;

  // The constructor sets natural hydrogen with one atom
  MDMaterialComponent C;
  Passed = EvaluateNear("MDMaterialComponent()", "default", "The default atomic weight is the one of natural hydrogen", C.GetAtomicWeight(), 1.0079, 1E-12) && Passed;
  Passed = Evaluate("MDMaterialComponent()", "default", "The default atomic number is the one of hydrogen", C.GetAtomicNumber(), 1) && Passed;
  Passed = Evaluate("MDMaterialComponent()", "default", "The default weighting is one atom", C.GetWeightingByAtoms(), 1) && Passed;
  Passed = EvaluateTrue("MDMaterialComponent()", "default", "The default weighting type is by atoms", C.GetWeightingType() == MDMaterialComponentWeightingType::c_ByAtoms) && Passed;
  Passed = EvaluateTrue("MDMaterialComponent()", "default", "The default has natural composition", C.HasNaturalIsotopeComposition()) && Passed;
  Passed = EvaluateTrue("MDMaterialComponent()", "default", "The default component is valid", C.Validate()) && Passed;

  // Use two fill patterns and compare the content: an uninitialized member would differ
  for (unsigned char Pattern : {static_cast<unsigned char>(0x5A), static_cast<unsigned char>(0xA5)}) {
    alignas(MDMaterialComponent) unsigned char Block[sizeof(MDMaterialComponent)];
    FillMemory(Block, sizeof(Block), Pattern);
    MDMaterialComponent* OnPattern = new (Block) MDMaterialComponent();
    Passed = EvaluateTrue("MDMaterialComponent()", "memory filled with a pattern", "The constructor sets every member", HasContent(*OnPattern, 1.0079, 1, 1, MDMaterialComponentWeightingType::c_ByAtoms, true)) && Passed;
    OnPattern->~MDMaterialComponent();
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the copy constructor
bool UTDMaterialComponent::TestCopy()
{
  bool Passed = true;

  MDMaterialComponent Isotope;
  Isotope.SetByMassWeighting(26.5, 13, 0.25);
  MDMaterialComponent IsotopeCopy(Isotope);
  Passed = EvaluateTrue("MDMaterialComponent(const MDMaterialComponent&)", "isotope by mass", "The copy has all values of the original", HasContent(IsotopeCopy, 26.5, 13, 0.25, MDMaterialComponentWeightingType::c_ByMass, false)) && Passed;

  MDMaterialComponent Natural;
  Natural.SetByAtomicWeighting("Pb", 3);
  MDMaterialComponent NaturalCopy(Natural);
  Passed = EvaluateTrue("MDMaterialComponent(const MDMaterialComponent&)", "natural by atoms", "The copy has all values of the original", HasContent(NaturalCopy, 207.2, 82, 3, MDMaterialComponentWeightingType::c_ByAtoms, true)) && Passed;

  // The copy has to be independent of the original
  NaturalCopy.SetByMassWeighting("Fe", 0.5);
  Passed = EvaluateTrue("MDMaterialComponent(const MDMaterialComponent&)", "changed copy", "A change of the copy does not reach the original", HasContent(Natural, 207.2, 82, 3, MDMaterialComponentWeightingType::c_ByAtoms, true)) && Passed;
  Isotope.SetWeightingByMass(0.75);
  Passed = EvaluateTrue("MDMaterialComponent(const MDMaterialComponent&)", "changed original", "A change of the original does not reach the copy", HasContent(IsotopeCopy, 26.5, 13, 0.25, MDMaterialComponentWeightingType::c_ByMass, false)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the explicit isotope by atoms setter with its limits
bool UTDMaterialComponent::TestSetByAtomicWeightingIsotope()
{
  bool Passed = true;

  MDMaterialComponent C;
  Passed = EvaluateTrue("SetByAtomicWeighting(A, Z, N)", "27.5, 13, 2", "A valid isotope is accepted", C.SetByAtomicWeighting(27.5, 13, 2)) && Passed;
  Passed = EvaluateTrue("SetByAtomicWeighting(A, Z, N)", "27.5, 13, 2", "All values are stored and there is no natural composition", HasContent(C, 27.5, 13, 2, MDMaterialComponentWeightingType::c_ByAtoms, false)) && Passed;
  Passed = Evaluate("SetByAtomicWeighting(A, Z, N)", "27.5, 13, 2", "The atoms getter returns the number of atoms", C.GetWeightingByAtoms(), 2) && Passed;

  // Rejected input must leave the old values
  struct BadInput { double A; int Z; int N; const char* Description; };
  vector<BadInput> BadInputs = {
    { 0.0, 13, 2, "zero atomic weight" },
    { -1.0, 13, 2, "negative atomic weight" },
    { 27.5, 0, 2, "atomic number just below 1" },
    { 27.5, -1, 2, "negative atomic number" },
    { 300.0, 129, 2, "atomic number just above 128" },
    { 27.5, 13, 0, "zero atoms" },
    { 27.5, 13, -1, "negative atoms" }
  };
  for (const BadInput& Bad : BadInputs) {
    DisableDefaultStreams();
    bool Result = C.SetByAtomicWeighting(Bad.A, Bad.Z, Bad.N);
    EnableDefaultStreams();
    Passed = EvaluateFalse("SetByAtomicWeighting(A, Z, N)", Bad.Description, "The input is rejected", Result) && Passed;
    Passed = EvaluateTrue("SetByAtomicWeighting(A, Z, N)", Bad.Description, "The rejection leaves the old values", HasContent(C, 27.5, 13, 2, MDMaterialComponentWeightingType::c_ByAtoms, false)) && Passed;
  }

  // The limits themselves are accepted
  Passed = EvaluateTrue("SetByAtomicWeighting(A, Z, N)", "Z = 1", "The lower limit of the atomic number is accepted", C.SetByAtomicWeighting(1.0, 1, 1)) && Passed;
  Passed = EvaluateTrue("SetByAtomicWeighting(A, Z, N)", "Z = 128", "The upper limit of the atomic number is accepted", C.SetByAtomicWeighting(300.0, 128, 1)) && Passed;
  Passed = Evaluate("SetByAtomicWeighting(A, Z, N)", "Z = 128", "The atomic number 128 is stored", C.GetAtomicNumber(), 128) && Passed;
  Passed = EvaluateTrue("SetByAtomicWeighting(A, Z, N)", "tiny A", "The smallest positive atomic weight is accepted", C.SetByAtomicWeighting(numeric_limits<double>::min(), 1, 1)) && Passed;

  DisableDefaultStreams();
  bool NaNResult = C.SetByAtomicWeighting(numeric_limits<double>::quiet_NaN(), 13, 1);
  EnableDefaultStreams();
  // Defect GEO-F-11: A not-a-number atomic weight passes the check for a positive value
  Passed = ExpectedToFail(EvaluateFalse("SetByAtomicWeighting(A, Z, N)", "NaN atomic weight", "A not-a-number atomic weight is rejected since it is not positive", NaNResult)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the natural composition by atoms setter
bool UTDMaterialComponent::TestSetByAtomicWeightingNatural()
{
  bool Passed = true;

  // IUPAC: Si 28.0855
  MDMaterialComponent C;
  Passed = EvaluateTrue("SetByAtomicWeighting(Name, N)", "Si, 3", "A known element is accepted", C.SetByAtomicWeighting("Si", 3)) && Passed;
  Passed = EvaluateTrue("SetByAtomicWeighting(Name, N)", "Si, 3", "The element values are stored with natural composition", HasContent(C, 28.0855, 14, 3, MDMaterialComponentWeightingType::c_ByAtoms, true)) && Passed;

  // Failures leave the old values
  vector<pair<MString, int>> BadInputs = { {"Si", 0}, {"Si", -2}, {"Xx", 1}, {"", 1}, {" Si", 1}, {"Silicon", 1} };
  for (const pair<MString, int>& Bad : BadInputs) {
    MString Input = MString("\"") + Bad.first + "\", " + Bad.second;
    DisableDefaultStreams();
    bool Result = C.SetByAtomicWeighting(Bad.first, Bad.second);
    EnableDefaultStreams();
    Passed = EvaluateFalse("SetByAtomicWeighting(Name, N)", Input, "The input is rejected", Result) && Passed;
    Passed = EvaluateTrue("SetByAtomicWeighting(Name, N)", Input, "The rejection leaves the old values", HasContent(C, 28.0855, 14, 3, MDMaterialComponentWeightingType::c_ByAtoms, true)) && Passed;
  }

  // A natural element follows an explicit isotope and the flag is switched
  C.SetByAtomicWeighting(27.5, 13, 1);
  C.SetByAtomicWeighting("Al", 1);
  Passed = EvaluateTrue("SetByAtomicWeighting(Name, N)", "after an isotope", "The natural composition flag is set again", C.HasNaturalIsotopeComposition()) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the explicit isotope by mass setter with its limits
bool UTDMaterialComponent::TestSetByMassWeightingIsotope()
{
  bool Passed = true;

  MDMaterialComponent C;
  Passed = EvaluateTrue("SetByMassWeighting(A, Z, W)", "27.5, 13, 0.25", "A valid isotope is accepted", C.SetByMassWeighting(27.5, 13, 0.25)) && Passed;
  Passed = EvaluateTrue("SetByMassWeighting(A, Z, W)", "27.5, 13, 0.25", "All values are stored and there is no natural composition", HasContent(C, 27.5, 13, 0.25, MDMaterialComponentWeightingType::c_ByMass, false)) && Passed;

  // Rejected input must leave the old values
  struct BadInput { double A; int Z; double W; const char* Description; };
  vector<BadInput> BadInputs = {
    { 0.0, 13, 0.5, "zero atomic weight" },
    { -1.0, 13, 0.5, "negative atomic weight" },
    { 27.5, 0, 0.5, "atomic number just below 1" },
    { 300.0, 129, 0.5, "atomic number just above 128" },
    { 27.5, 13, 0.0, "zero mass fraction" },
    { 27.5, 13, -0.5, "negative mass fraction" }
  };
  for (const BadInput& Bad : BadInputs) {
    DisableDefaultStreams();
    bool Result = C.SetByMassWeighting(Bad.A, Bad.Z, Bad.W);
    EnableDefaultStreams();
    Passed = EvaluateFalse("SetByMassWeighting(A, Z, W)", Bad.Description, "The input is rejected", Result) && Passed;
    Passed = EvaluateTrue("SetByMassWeighting(A, Z, W)", Bad.Description, "The rejection leaves the old values", HasContent(C, 27.5, 13, 0.25, MDMaterialComponentWeightingType::c_ByMass, false)) && Passed;
  }

  // The limits themselves are accepted
  Passed = EvaluateTrue("SetByMassWeighting(A, Z, W)", "Z = 128", "The upper limit of the atomic number is accepted", C.SetByMassWeighting(300.0, 128, 1E-300)) && Passed;

  DisableDefaultStreams();
  bool NaNResult = C.SetByMassWeighting(27.5, 13, numeric_limits<double>::quiet_NaN());
  EnableDefaultStreams();
  // Defect GEO-F-11: A not-a-number mass fraction passes the check for a positive value
  Passed = ExpectedToFail(EvaluateFalse("SetByMassWeighting(A, Z, W)", "NaN mass fraction", "A not-a-number mass fraction is rejected since it is not positive", NaNResult)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the natural composition by mass setter
bool UTDMaterialComponent::TestSetByMassWeightingNatural()
{
  bool Passed = true;

  // IUPAC: O 15.9994
  MDMaterialComponent C;
  Passed = EvaluateTrue("SetByMassWeighting(Name, W)", "O, 0.2315", "A known element is accepted", C.SetByMassWeighting("O", 0.2315)) && Passed;
  Passed = EvaluateTrue("SetByMassWeighting(Name, W)", "O, 0.2315", "The element values are stored with natural composition", HasContent(C, 15.9994, 8, 0.2315, MDMaterialComponentWeightingType::c_ByMass, true)) && Passed;

  vector<pair<MString, double>> BadInputs = { {"O", 0.0}, {"O", -0.1}, {"Xx", 0.5}, {"", 0.5} };
  for (const pair<MString, double>& Bad : BadInputs) {
    MString Input = MString("\"") + Bad.first + "\", " + Bad.second;
    DisableDefaultStreams();
    bool Result = C.SetByMassWeighting(Bad.first, Bad.second);
    EnableDefaultStreams();
    Passed = EvaluateFalse("SetByMassWeighting(Name, W)", Input, "The input is rejected", Result) && Passed;
    Passed = EvaluateTrue("SetByMassWeighting(Name, W)", Input, "The rejection leaves the old values", HasContent(C, 15.9994, 8, 0.2315, MDMaterialComponentWeightingType::c_ByMass, true)) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the protected element lookup against the periodic table
bool UTDMaterialComponent::TestElementTable()
{
  bool Passed = true;

  // Periodic table up to Z = 100 with the IUPAC 2021 standard atomic weights, abridged
  // The mass numbers of the longest-lived isotopes are used for the elements without stable isotopes
  struct Element { const char* Name; double Weight; };
  const vector<Element> Elements = {
    {"H", 1.008}, {"He", 4.0026}, {"Li", 6.94}, {"Be", 9.0122}, {"B", 10.81}, {"C", 12.011}, {"N", 14.007}, {"O", 15.999}, {"F", 18.998}, {"Ne", 20.180},
    {"Na", 22.990}, {"Mg", 24.305}, {"Al", 26.982}, {"Si", 28.085}, {"P", 30.974}, {"S", 32.06}, {"Cl", 35.45}, {"Ar", 39.95}, {"K", 39.098}, {"Ca", 40.078},
    {"Sc", 44.956}, {"Ti", 47.867}, {"V", 50.942}, {"Cr", 51.996}, {"Mn", 54.938}, {"Fe", 55.845}, {"Co", 58.933}, {"Ni", 58.693}, {"Cu", 63.546}, {"Zn", 65.38},
    {"Ga", 69.723}, {"Ge", 72.630}, {"As", 74.922}, {"Se", 78.971}, {"Br", 79.904}, {"Kr", 83.798}, {"Rb", 85.468}, {"Sr", 87.62}, {"Y", 88.906}, {"Zr", 91.224},
    {"Nb", 92.906}, {"Mo", 95.95}, {"Tc", 98.0}, {"Ru", 101.07}, {"Rh", 102.91}, {"Pd", 106.42}, {"Ag", 107.87}, {"Cd", 112.41}, {"In", 114.82}, {"Sn", 118.71},
    {"Sb", 121.76}, {"Te", 127.60}, {"I", 126.90}, {"Xe", 131.29}, {"Cs", 132.91}, {"Ba", 137.33}, {"La", 138.91}, {"Ce", 140.12}, {"Pr", 140.91}, {"Nd", 144.24},
    {"Pm", 145.0}, {"Sm", 150.36}, {"Eu", 151.96}, {"Gd", 157.25}, {"Tb", 158.93}, {"Dy", 162.50}, {"Ho", 164.93}, {"Er", 167.26}, {"Tm", 168.93}, {"Yb", 173.05},
    {"Lu", 174.97}, {"Hf", 178.49}, {"Ta", 180.95}, {"W", 183.84}, {"Re", 186.21}, {"Os", 190.23}, {"Ir", 192.22}, {"Pt", 195.08}, {"Au", 196.97}, {"Hg", 200.59},
    {"Tl", 204.38}, {"Pb", 207.2}, {"Bi", 208.98}, {"Po", 209.0}, {"At", 210.0}, {"Rn", 222.0}, {"Fr", 223.0}, {"Ra", 226.0}, {"Ac", 227.0}, {"Th", 232.04},
    {"Pa", 231.04}, {"U", 238.03}, {"Np", 237.0}, {"Pu", 244.0}, {"Am", 243.0}, {"Cm", 247.0}, {"Bk", 247.0}, {"Cf", 251.0}, {"Es", 252.0}, {"Fm", 257.0}
  };
  Passed = EvaluateSize("SetElement()", "reference table", "The reference table has 100 elements", Elements.size(), 100UL) && Passed;

  // The code has an older IUPAC table: the largest difference to the abridged reference is 0.011 at Se (78.96 vs. 78.971)
  const double Tolerance = 0.012;
  for (unsigned int i = 0; i < Elements.size(); ++i) {
    AccessMaterialComponent C;
    bool Result = C.SetElement(Elements[i].Name);
    Passed = EvaluateTrue("SetElement()", Elements[i].Name, "The element is found", Result) && Passed;
    Passed = Evaluate("SetElement()", Elements[i].Name, "The atomic number is the one of the periodic table", C.GetAtomicNumber(), static_cast<int>(i + 1)) && Passed;
    Passed = EvaluateNear("SetElement()", Elements[i].Name, "The atomic weight is the standard atomic weight", C.GetAtomicWeight(), Elements[i].Weight, Tolerance) && Passed;
    Passed = EvaluateTrue("SetElement()", Elements[i].Name, "The element has natural composition", C.HasNaturalIsotopeComposition()) && Passed;
  }

  // Attention: The lookup is deliberately case insensitive, it lowers both sides
  AccessMaterialComponent Lower;
  Passed = EvaluateTrue("SetElement()", "pb", "The lookup ignores the case", Lower.SetElement("pb")) && Passed;
  Passed = Evaluate("SetElement()", "pb", "The lookup ignores the case and returns lead", Lower.GetAtomicNumber(), 82) && Passed;
  Passed = EvaluateTrue("SetElement()", "PB", "The lookup ignores the case", Lower.SetElement("PB")) && Passed;

  // The limit of the table is Z = 100 which is also the validation limit
  AccessMaterialComponent Beyond;
  Beyond.SetByAtomicWeighting(50.0, 25, 7);
  Passed = EvaluateFalse("SetElement()", "Md (Z = 101)", "The first element above the limit is not found", Beyond.SetElement("Md")) && Passed;
  Passed = EvaluateFalse("SetElement()", "unknown name", "An unknown name is not found", Beyond.SetElement("Qq")) && Passed;
  Passed = EvaluateFalse("SetElement()", "empty name", "An empty name is not found", Beyond.SetElement("")) && Passed;
  Passed = EvaluateTrue("SetElement()", "failed lookups", "Failed lookups leave the old values", HasContent(Beyond, 50.0, 25, 7, MDMaterialComponentWeightingType::c_ByAtoms, false)) && Passed;

  // A successful lookup keeps the weighting
  Passed = EvaluateTrue("SetElement()", "Fe", "The element is found", Beyond.SetElement("Fe")) && Passed;
  Passed = EvaluateTrue("SetElement()", "Fe", "The weighting stays and the element changes", HasContent(Beyond, 55.845, 26, 7, MDMaterialComponentWeightingType::c_ByAtoms, true)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the weighting setters and the rounding of the atoms getter
bool UTDMaterialComponent::TestWeightingSetters()
{
  bool Passed = true;

  // By mass on an atoms component switches the type and keeps the element
  MDMaterialComponent C;
  C.SetByAtomicWeighting("Fe", 2);
  Passed = EvaluateTrue("SetWeightingByMass()", "0.75", "A positive mass fraction is accepted", C.SetWeightingByMass(0.75)) && Passed;
  Passed = EvaluateTrue("SetWeightingByMass()", "0.75", "The weighting and the type change, the element does not", HasContent(C, 55.845, 26, 0.75, MDMaterialComponentWeightingType::c_ByMass, true)) && Passed;

  // By atoms on a mass component
  Passed = EvaluateTrue("SetWeightingByAtoms()", "5", "A positive number of atoms is accepted", C.SetWeightingByAtoms(5)) && Passed;
  Passed = EvaluateTrue("SetWeightingByAtoms()", "5", "The weighting and the type change, the element does not", HasContent(C, 55.845, 26, 5, MDMaterialComponentWeightingType::c_ByAtoms, true)) && Passed;

  // Rejections leave the old values
  for (double Bad : {0.0, -0.5}) {
    DisableDefaultStreams();
    bool Result = C.SetWeightingByMass(Bad);
    EnableDefaultStreams();
    Passed = EvaluateFalse("SetWeightingByMass()", Bad, "A non-positive mass fraction is rejected", Result) && Passed;
    Passed = EvaluateTrue("SetWeightingByMass()", Bad, "The rejection leaves the old values", HasContent(C, 55.845, 26, 5, MDMaterialComponentWeightingType::c_ByAtoms, true)) && Passed;
  }
  for (int Bad : {0, -1}) {
    DisableDefaultStreams();
    bool Result = C.SetWeightingByAtoms(Bad);
    EnableDefaultStreams();
    Passed = EvaluateFalse("SetWeightingByAtoms()", Bad, "A non-positive number of atoms is rejected", Result) && Passed;
    Passed = EvaluateTrue("SetWeightingByAtoms()", Bad, "The rejection leaves the old values", HasContent(C, 55.845, 26, 5, MDMaterialComponentWeightingType::c_ByAtoms, true)) && Passed;
  }

  // Attention: As documented in the header the atoms getter rounds a mass fraction to the next integer
  C.SetWeightingByMass(0.49);
  Passed = Evaluate("GetWeightingByAtoms()", "mass fraction 0.49", "A value just below one half is rounded down", C.GetWeightingByAtoms(), 0) && Passed;
  C.SetWeightingByMass(0.5);
  Passed = Evaluate("GetWeightingByAtoms()", "mass fraction 0.5", "One half is rounded up", C.GetWeightingByAtoms(), 1) && Passed;
  C.SetWeightingByMass(1.49);
  Passed = Evaluate("GetWeightingByAtoms()", "mass fraction 1.49", "A value just below one and a half is rounded down", C.GetWeightingByAtoms(), 1) && Passed;

  // The largest number of atoms survives the rounding
  C.SetWeightingByAtoms(numeric_limits<int>::max());
  Passed = Evaluate("GetWeightingByAtoms()", "largest int", "The largest number of atoms is returned unchanged", C.GetWeightingByAtoms(), numeric_limits<int>::max()) && Passed;

  DisableDefaultStreams();
  bool NaNResult = C.SetWeightingByMass(numeric_limits<double>::quiet_NaN());
  EnableDefaultStreams();
  // Defect GEO-F-11: A not-a-number mass fraction passes the check for a positive value
  Passed = ExpectedToFail(EvaluateFalse("SetWeightingByMass()", "NaN", "A not-a-number mass fraction is rejected since it is not positive", NaNResult)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the validation limits
bool UTDMaterialComponent::TestValidate()
{
  bool Passed = true;

  MDMaterialComponent C;
  C.SetByAtomicWeighting(26.0, 13, 1);
  Passed = EvaluateTrue("Validate()", "A = 26, Z = 13", "A normal isotope is valid", C.Validate()) && Passed;

  // Atomic weight limit: A >= Z, here hydrogen with A = Z is the only physical case
  C.SetByAtomicWeighting(1.0, 1, 1);
  Passed = EvaluateTrue("Validate()", "A = 1, Z = 1", "A = Z is just inside of the limit", C.Validate()) && Passed;

  C.SetByAtomicWeighting(1.0 - 1E-9, 1, 1);
  DisableDefaultStreams();
  bool JustBelow = C.Validate();
  EnableDefaultStreams();
  Passed = EvaluateFalse("Validate()", "A = 1 - 1E-9, Z = 1", "A just below Z is outside of the limit", JustBelow) && Passed;

  C.SetByMassWeighting(1.0, 2, 1.0);
  DisableDefaultStreams();
  bool OnlyOne = C.Validate();
  EnableDefaultStreams();
  Passed = EvaluateFalse("Validate()", "A = 1, Z = 2", "A smaller than Z is invalid also for A = 1", OnlyOne) && Passed;

  // Atomic number limit: Z <= 100
  C.SetByAtomicWeighting(257.0, 100, 1);
  Passed = EvaluateTrue("Validate()", "Z = 100", "Z = 100 is just inside of the limit", C.Validate()) && Passed;

  C.SetByAtomicWeighting(258.0, 101, 1);
  DisableDefaultStreams();
  bool Beyond = C.Validate();
  EnableDefaultStreams();
  Passed = EvaluateFalse("Validate()", "Z = 101", "Z = 101 is just outside of the limit", Beyond) && Passed;

  // Validation does not change anything
  C.SetByMassWeighting("Cu", 0.3);
  Passed = EvaluateTrue("Validate()", "natural Cu by mass", "A natural element by mass is valid", C.Validate()) && Passed;
  Passed = EvaluateTrue("Validate()", "natural Cu by mass", "The validation leaves the values", HasContent(C, 63.546, 29, 0.3, MDMaterialComponentWeightingType::c_ByMass, true)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the string description
bool UTDMaterialComponent::TestToString()
{
  bool Passed = true;

  MDMaterialComponent C;
  C.SetByAtomicWeighting(26.5, 13, 2);
  Passed = EvaluateTrue("ToString()", "isotope by atoms", "The string has Z, A, and the atoms", C.ToString() == "Z=13, A=26.5, atoms=2") && Passed;

  C.SetByMassWeighting(26.5, 13, 0.25);
  Passed = EvaluateTrue("ToString()", "isotope by mass", "The string has Z, A, and the mass fraction", C.ToString() == "Z=13, A=26.5, mass fraction=0.25") && Passed;

  C.SetByAtomicWeighting("Pb", 3);
  Passed = EvaluateTrue("ToString()", "natural by atoms", "The string names the natural composition", C.ToString() == "Z=82 (A=natural composition), atoms=3") && Passed;

  C.SetByMassWeighting("Pb", 0.125);
  Passed = EvaluateTrue("ToString()", "natural by mass", "The string names the natural composition", C.ToString() == "Z=82 (A=natural composition), mass fraction=0.125") && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the Geomega output
bool UTDMaterialComponent::TestGetGeomega()
{
  bool Passed = true;

  MDMaterialComponent C;
  C.SetByAtomicWeighting(26.5, 13, 2);
  Passed = EvaluateTrue("GetGeomega()", "isotope by atoms", "The keyword is followed by A, Z, and atoms", C.GetGeomega() == ".ComponentByAtoms 26.5 13 2") && Passed;

  C.SetByMassWeighting(26.5, 13, 0.25);
  Passed = EvaluateTrue("GetGeomega()", "isotope by mass", "The keyword is followed by A, Z, and mass fraction", C.GetGeomega() == ".ComponentByMass 26.5 13 0.25") && Passed;

  C.SetByAtomicWeighting("Pb", 3);
  Passed = EvaluateTrue("GetGeomega()", "natural by atoms", "The keyword is followed by the element name and atoms", C.GetGeomega() == ".ComponentByAtoms Pb 3") && Passed;

  C.SetByMassWeighting("W", 0.125);
  Passed = EvaluateTrue("GetGeomega()", "natural by mass", "The keyword is followed by the element name and the mass fraction", C.GetGeomega() == ".ComponentByMass W 0.125") && Passed;

  C.SetByMassWeighting("W", 0.123456789);
  vector<MString> Tokens = C.GetGeomega().Tokenize(" ");
  double Written = Tokens.back().ToDouble();
  // Defect GEO-F-10: The stream default of 6 digits truncates a mass fraction which the parser would read back differently
  Passed = ExpectedToFail(EvaluateNear("GetGeomega()", "mass fraction 0.123456789", "The written mass fraction has the precision of the stored value", Written, 0.123456789, 1E-12)) && Passed;

  C.SetByMassWeighting(26.9815386, 13, 0.5);
  Tokens = C.GetGeomega().Tokenize(" ");
  Written = Tokens[1].ToDouble();
  // Defect GEO-F-10: The stream default of 6 digits truncates an isotope weight which the parser would read back differently
  Passed = ExpectedToFail(EvaluateNear("GetGeomega()", "A = 26.9815386", "The written atomic weight has the precision of the stored value", Written, 26.9815386, 1E-9)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the reuse of one object for several operations
bool UTDMaterialComponent::TestReuse()
{
  bool Passed = true;

  MDMaterialComponent C;
  C.SetByMassWeighting("Fe", 0.5);
  C.SetByAtomicWeighting(27.5, 13, 2);
  Passed = EvaluateTrue("SetByAtomicWeighting(A, Z, N)", "after natural by mass", "No value of the old content is left", HasContent(C, 27.5, 13, 2, MDMaterialComponentWeightingType::c_ByAtoms, false)) && Passed;

  C.SetByMassWeighting("Pb", 0.1);
  Passed = EvaluateTrue("SetByMassWeighting(Name, W)", "after isotope by atoms", "No value of the old content is left", HasContent(C, 207.2, 82, 0.1, MDMaterialComponentWeightingType::c_ByMass, true)) && Passed;

  C.SetByMassWeighting(14.0, 7, 0.2);
  Passed = EvaluateTrue("SetByMassWeighting(A, Z, W)", "after natural by mass", "No value of the old content is left", HasContent(C, 14.0, 7, 0.2, MDMaterialComponentWeightingType::c_ByMass, false)) && Passed;

  C.SetByAtomicWeighting("H", 4);
  Passed = EvaluateTrue("SetByAtomicWeighting(Name, N)", "after isotope by mass", "No value of the old content is left", HasContent(C, 1.0079, 1, 4, MDMaterialComponentWeightingType::c_ByAtoms, true)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTDMaterialComponent Test;
  return Test.Run() == true ? 0 : 1;
}


// UTDMaterialComponent.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
