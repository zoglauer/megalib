/*
 * UTGeometryConverter.cc
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
#include "MCGeometryConverter.hh"

// MEGAlib:
#include "MStreams.h"
#include "MString.h"
#include "MUnitTest.h"

// Geant4:
#include "G4Box.hh"
#include "G4Cons.hh"
#include "G4DisplacedSolid.hh"
#include "G4Element.hh"
#include "G4IntersectionSolid.hh"
#include "G4LogicalVolume.hh"
#include "G4Material.hh"
#include "G4Orb.hh"
#include "G4PVPlacement.hh"
#include "G4Polycone.hh"
#include "G4Polyhedra.hh"
#include "G4RotationMatrix.hh"
#include "G4Sphere.hh"
#include "G4SubtractionSolid.hh"
#include "G4SystemOfUnits.hh"
#include "G4ThreeVector.hh"
#include "G4Transform3D.hh"
#include "G4Trap.hh"
#include "G4Trd.hh"
#include "G4Tubs.hh"
#include "G4UnionSolid.hh"

// Standard lib:
#include <algorithm>
#include <cctype>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <initializer_list>
#include <iomanip>
#include <memory>
#include <sstream>
#include <string>
#include <vector>
using namespace std;


//! Class exposing the protected functions of MCGeometryConverter to the unit test
class MCGeometryConverterAccess : public MCGeometryConverter
{
public:
  using MCGeometryConverter::CreateUniqueName;
  using MCGeometryConverter::MakeValidName;
  using MCGeometryConverter::ConvertShape;
  using MCGeometryConverter::ConvertOrientation;
  using MCGeometryConverter::Zeroing;
};


//! A small Geant4 geometry which is removed again when the object is destroyed
class UTGeometryConverterFixture
{
public:
  //! Create the materials H, O, and Water (H2O)
  UTGeometryConverterFixture()
  {
    H = new G4Element("Hydrogen", "H", 1.0, 1.008*g/mole);
    O = new G4Element("Oxygen", "O", 8.0, 15.999*g/mole);
    Water = new G4Material("Water Mat", 1.0*g/cm3, 2);
    Water->AddElement(H, 2);
    Water->AddElement(O, 1);
  }

  //! Remove all volumes, solids and materials in the right order
  ~UTGeometryConverterFixture()
  {
    for (auto I = PhysicalVolumes.rbegin(); I != PhysicalVolumes.rend(); ++I) delete *I;
    for (auto I = LogicalVolumes.rbegin(); I != LogicalVolumes.rend(); ++I) delete *I;
    for (auto I = Solids.rbegin(); I != Solids.rend(); ++I) delete *I;
    for (auto I = Materials.rbegin(); I != Materials.rend(); ++I) delete *I;
    delete Water;
    delete O;
    delete H;

    // Geant4 leaves null pointers in its static tables, which crash the next material or the converter:
    G4MaterialTable* MT = G4Material::GetMaterialTable();
    MT->erase(remove(MT->begin(), MT->end(), nullptr), MT->end());
    G4ElementTable* ET = const_cast<G4ElementTable*>(G4Element::GetElementTable());
    ET->erase(remove(ET->begin(), ET->end(), nullptr), ET->end());
    // Indices of the remaining objects must stay consistent (none are expected)
  }

  //! Add a solid, which is owned by this fixture
  template <typename T> T* Own(T* Solid) { Solids.push_back(Solid); return Solid; }
  //! Add a logical volume, which is owned by this fixture
  G4LogicalVolume* AddLogical(G4VSolid* Solid, const G4String& Name, G4Material* Material = nullptr)
  {
    LogicalVolumes.push_back(new G4LogicalVolume(Solid, Material == nullptr ? Water : Material, Name));
    return LogicalVolumes.back();
  }
  //! Place a volume, with the object transformation, in a mother (nullptr for the world)
  G4VPhysicalVolume* Place(G4LogicalVolume* Volume, const G4String& Name, G4LogicalVolume* Mother, const G4RotationMatrix& Rotation, const G4ThreeVector& Position)
  {
    PhysicalVolumes.push_back(new G4PVPlacement(G4Transform3D(Rotation, Position), Volume, Name, Mother, false, 0, false));
    return PhysicalVolumes.back();
  }
  //! Add a material, which is owned by this fixture
  G4Material* AddMaterial(const G4String& Name, double Density)
  {
    Materials.push_back(new G4Material(Name, 1.0, 1.008*g/mole, Density));
    return Materials.back();
  }

  G4Element* H;
  G4Element* O;
  G4Material* Water;
  vector<G4VSolid*> Solids;
  vector<G4LogicalVolume*> LogicalVolumes;
  vector<G4VPhysicalVolume*> PhysicalVolumes;
  vector<G4Material*> Materials;
};


//! Unit test class for MCGeometryConverter
class UTGeometryConverter : public MUnitTest
{
public:
  //! Default constructor
  UTGeometryConverter() : MUnitTest("UTGeometryConverter") {}
  //! Default destructor
  virtual ~UTGeometryConverter() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Test the header and the empty geometry
  bool TestEmptyGeometry();
  //! Test the output file handling
  bool TestOutputFile();
  //! Test the material conversion
  bool TestMaterials();
  //! Test the name conversion and the unique names
  bool TestNames();
  //! Test logical and physical volumes
  bool TestVolumes();
  //! Test the simple shapes
  bool TestSimpleShapes();
  //! Test the trapezoid conversion
  bool TestTrap();
  //! Test the polyhedra and polycones
  bool TestPolyhedra();
  //! Test the boolean solids
  bool TestBooleanSolids();
  //! Test unknown shapes
  bool TestUnknownShape();
  //! Test the orientation conversion and the zeroing
  bool TestOrientationAndZeroing();
  //! Test the precision of the output
  bool TestPrecision();
  //! Test the reuse of the converter
  bool TestReuse();

  //! Split a text into lines
  vector<string> Split(const MString& Text) const;
  //! Return true if the exact line exists
  bool HasLine(const vector<string>& Lines, const string& Line) const;
  //! Return true if a line starts with the prefix
  bool HasPrefix(const vector<string>& Lines, const string& Prefix) const;
  //! Format a number rounded to 1e-6 without negative zeros
  string Round(double Value, bool Angle) const;
  //! Return the numbers behind the prefix of the first line starting with it as one formatted string, or "<missing>"
  MString Values(const vector<string>& Lines, const string& Prefix, bool Angles = false) const;
  //! Format expected numbers in the same way
  MString Join(initializer_list<double> Numbers, bool Angles = false) const;
  //! Convert a fixture geometry with a new converter and return the lines of the file
  vector<string> ConvertToLines(const MString& FileName);
  //! Convert one solid with ConvertShape into a file and return the lines
  vector<string> ShapeLines(G4VSolid* Solid, const G4String& Name, bool PartOfVolume = true);
};


////////////////////////////////////////////////////////////////////////////////


bool UTGeometryConverter::Run()
{
  bool Passed = true;

  Passed = TestEmptyGeometry() && Passed;
  Passed = TestOutputFile() && Passed;
  Passed = TestMaterials() && Passed;
  Passed = TestNames() && Passed;
  Passed = TestVolumes() && Passed;
  Passed = TestSimpleShapes() && Passed;
  Passed = TestTrap() && Passed;
  Passed = TestPolyhedra() && Passed;
  Passed = TestBooleanSolids() && Passed;
  Passed = TestUnknownShape() && Passed;
  Passed = TestOrientationAndZeroing() && Passed;
  Passed = TestPrecision() && Passed;
  Passed = TestReuse() && Passed;

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


vector<string> UTGeometryConverter::Split(const MString& Text) const
{
  vector<string> Lines;
  istringstream In(Text.Data());
  string Line;
  while (getline(In, Line)) Lines.push_back(Line);
  return Lines;
}


////////////////////////////////////////////////////////////////////////////////


bool UTGeometryConverter::HasLine(const vector<string>& Lines, const string& Line) const
{
  for (const string& L: Lines) {
    if (L == Line) return true;
  }
  return false;
}


////////////////////////////////////////////////////////////////////////////////


bool UTGeometryConverter::HasPrefix(const vector<string>& Lines, const string& Prefix) const
{
  for (const string& L: Lines) {
    if (L.compare(0, Prefix.size(), Prefix) == 0) return true;
  }
  return false;
}


////////////////////////////////////////////////////////////////////////////////


string UTGeometryConverter::Round(double Value, bool Angle) const
{
  // Angles are compared modulo 360 deg
  if (Angle == true) {
    Value = fmod(Value, 360.0);
    if (Value < 0) Value += 360.0;
    if (fabs(Value - 360.0) < 1e-6) Value = 0.0;
  }
  double R = round(Value*1e6)/1e6;
  if (R == 0.0) R = 0.0;
  ostringstream Out;
  Out<<R;
  return Out.str();
}


////////////////////////////////////////////////////////////////////////////////


MString UTGeometryConverter::Values(const vector<string>& Lines, const string& Prefix, bool Angles) const
{
  for (const string& L: Lines) {
    if (L.compare(0, Prefix.size(), Prefix) != 0) continue;
    istringstream In(L.substr(Prefix.size()));
    string Result;
    string Token;
    while (In>>Token) {
      // nan and inf are not read by the stream operator:
      double Number = 0;
      try {
        Number = stod(Token);
      } catch (...) {
        return MString((string("<not a number: ") + Token + ">").c_str());
      }
      if (Result.empty() == false) Result += " ";
      Result += Round(Number, Angles);
    }
    return MString(Result.c_str());
  }

  return MString(("<missing: ") + Prefix + ">");
}


////////////////////////////////////////////////////////////////////////////////


MString UTGeometryConverter::Join(initializer_list<double> Numbers, bool Angles) const
{
  string Result;
  for (double N: Numbers) {
    if (Result.empty() == false) Result += " ";
    Result += Round(N, Angles);
  }
  return MString(Result.c_str());
}


////////////////////////////////////////////////////////////////////////////////


vector<string> UTGeometryConverter::ConvertToLines(const MString& FileName)
{
  MCGeometryConverter Converter;
  DisableDefaultStreams();
  ostringstream Sink;
  streambuf* Old = cout.rdbuf(Sink.rdbuf());
  bool OK = Converter.Convert(FileName.Data());
  cout.rdbuf(Old);
  EnableDefaultStreams();
  if (OK == false) return vector<string>();

  return Split(ReadTextFile(FileName));
}


////////////////////////////////////////////////////////////////////////////////


vector<string> UTGeometryConverter::ShapeLines(G4VSolid* Solid, const G4String& Name, bool PartOfVolume)
{
  const MString FileName = GetTemporaryFileName(MString("Shape_") + Name.c_str() + ".txt");
  MCGeometryConverterAccess Converter;
  {
    ofstream Out(FileName.Data());
    G4String NameCopy = Name;
    Converter.ConvertShape(Out, NameCopy, Solid, PartOfVolume);
  }
  return Split(ReadTextFile(FileName));
}


////////////////////////////////////////////////////////////////////////////////


bool UTGeometryConverter::TestEmptyGeometry()
{
  bool Passed = true;

  // Without any Geant4 object only the header and the section titles are written:
  vector<string> Lines = ConvertToLines(GetTemporaryFileName("Empty.geo.setup"));
  Passed = EvaluateTrue("Convert()", "empty geometry", "The empty geometry can be converted", Lines.size() > 0) && Passed;
  Passed = EvaluateTrue("Convert()", "empty geometry", "The file starts with the comment about the conversion", Lines.size() > 0 && Lines[0] == "# This geometry has been converted from Geant4 to MEGAlib") && Passed;
  Passed = EvaluateTrue("Convert()", "empty geometry", "The name is a placeholder", HasLine(Lines, "Name NameTbd")) && Passed;
  Passed = EvaluateTrue("Convert()", "empty geometry", "The surrounding sphere is a placeholder", HasLine(Lines, "SurroundingSphere 10000.0 0.0 0.0 0.0 10000.0")) && Passed;
  Passed = EvaluateTrue("Convert()", "empty geometry", "The material section exists", HasPrefix(Lines, "# Material descriptions:")) && Passed;
  Passed = EvaluateTrue("Convert()", "empty geometry", "The logical volume section exists", HasPrefix(Lines, "# Logical volume descriptions:")) && Passed;
  Passed = EvaluateTrue("Convert()", "empty geometry", "The physical volume section exists", HasPrefix(Lines, "# Physical volume descriptions:")) && Passed;
  Passed = EvaluateTrue("Convert()", "empty geometry", "The detector section exists", HasPrefix(Lines, "# Detector descriptions:")) && Passed;
  Passed = EvaluateFalse("Convert()", "empty geometry", "Without Geant4 objects there are no materials", HasPrefix(Lines, "Material ")) && Passed;
  Passed = EvaluateFalse("Convert()", "empty geometry", "Without Geant4 objects there are no volumes", HasPrefix(Lines, "Volume ")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTGeometryConverter::TestOutputFile()
{
  bool Passed = true;

  // The default file name is used in the current directory:
  const MString Directory = GetTemporaryDirectoryName("Cwd");
  Passed = EvaluateTrue("PrepareTemporaryDirectory()", "cwd", "The directory for the default file name can be created", PrepareTemporaryDirectory("Cwd")) && Passed;
  error_code Error;
  filesystem::path OldPath = filesystem::current_path(Error);
  filesystem::current_path(Directory.Data(), Error);
  {
    MCGeometryConverter Converter;
    DisableDefaultStreams();
    ostringstream Sink;
    streambuf* Old = cout.rdbuf(Sink.rdbuf());
    bool OK = Converter.Convert();
    cout.rdbuf(Old);
    EnableDefaultStreams();
    Passed = EvaluateTrue("Convert()", "default file name", "The default call succeeds", OK) && Passed;
  }
  filesystem::current_path(OldPath, Error);
  Passed = EvaluateTrue("Convert()", "default file name", "The default file name is G4ToMEGAlib.geo.setup", filesystem::exists(Directory.Data() + string("/G4ToMEGAlib.geo.setup"))) && Passed;

  // An existing file is replaced:
  const MString FileName = GetTemporaryFileName("Existing.geo.setup");
  Passed = EvaluateTrue("WriteTextFile()", "existing", "The existing file can be written", WriteTextFile(FileName, "old content\n")) && Passed;
  vector<string> Lines = ConvertToLines(FileName);
  Passed = EvaluateTrue("Convert()", "existing file", "An existing file is replaced", HasLine(Lines, "Name NameTbd") && HasLine(Lines, "old content") == false) && Passed;

  // An output file which cannot be created must be reported:
  {
    MCGeometryConverter Converter;
    DisableDefaultStreams();
    ostringstream Sink;
    streambuf* Old = cout.rdbuf(Sink.rdbuf());
    bool OK = Converter.Convert((GetTemporaryFileName("DoesNotExist") + "/Output.geo.setup").Data());
    cout.rdbuf(Old);
    EnableDefaultStreams();
    // Known failure: Convert() never checks whether the output file could be opened and always returns true
    Passed = ExpectedToFail(EvaluateFalse("Convert()", "missing directory", "Converting into a missing directory fails", OK)) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTGeometryConverter::TestMaterials()
{
  bool Passed = true;

  UTGeometryConverterFixture F;
  vector<string> Lines = ConvertToLines(GetTemporaryFileName("Materials.geo.setup"));

  Passed = EvaluateTrue("Convert()", "Water Mat", "The material name is made valid: the space is removed, a prefix and an index are added", HasLine(Lines, "Material mWaterMat_0")) && Passed;
  Passed = Evaluate("Convert()", "Water Mat", "The density is given in g/cm3", Values(Lines, "mWaterMat_0.Density "), Join({1.0})) && Passed;

  // Components: atomic mass, atomic number, mass fraction (the deprecated but supported Geomega syntax)
  const double Total = 2*1.008 + 15.999;
  Passed = Evaluate("Convert()", "Water Mat H", "The first component is hydrogen: atomic mass, atomic number, mass fraction", Values(Lines, "mWaterMat_0.ComponentByMass 1.008 "), Join({1.0, 2*1.008/Total})) && Passed;
  Passed = Evaluate("Convert()", "Water Mat O", "The second component is oxygen: atomic mass, atomic number, mass fraction", Values(Lines, "mWaterMat_0.ComponentByMass 15.999 "), Join({8.0, 15.999/Total})) && Passed;

  // Densities in other units and several materials in the order of the material table:
  G4Material* Dense = F.AddMaterial("Lead", 11.35*g/cm3);
  G4Material* Thin = F.AddMaterial("Vacuum Gas", 1.0e-5*mg/cm3);
  (void) Dense;
  (void) Thin;
  Lines = ConvertToLines(GetTemporaryFileName("Materials2.geo.setup"));
  Passed = EvaluateTrue("Convert()", "three materials", "The first material is converted", HasLine(Lines, "Material mWaterMat_0")) && Passed;
  Passed = EvaluateTrue("Convert()", "three materials", "The second material is converted", HasLine(Lines, "Material mLead_0")) && Passed;
  Passed = EvaluateTrue("Convert()", "three materials", "The third material is converted", HasLine(Lines, "Material mVacuumGas_0")) && Passed;
  Passed = Evaluate("Convert()", "Lead", "The density of lead is 11.35 g/cm3", Values(Lines, "mLead_0.Density "), Join({11.35})) && Passed;
  Passed = Evaluate("Convert()", "Vacuum Gas", "The density of a gas is converted", Values(Lines, "mVacuumGas_0.Density "), Join({1.0e-8})) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTGeometryConverter::TestNames()
{
  bool Passed = true;

  MCGeometryConverterAccess C;
  Passed = Evaluate("MakeValidName()", "valid name", "A valid name is kept", MString(C.MakeValidName("Crystal-1_a").c_str()), MString("Crystal-1_a")) && Passed;
  Passed = Evaluate("MakeValidName()", "spaces and dots", "Spaces and dots are removed", MString(C.MakeValidName("My Volume.1").c_str()), MString("MyVolume1")) && Passed;
  Passed = Evaluate("MakeValidName()", "slashes", "Other special characters are removed", MString(C.MakeValidName("A/B:C[0]").c_str()), MString("ABC0")) && Passed;
  Passed = Evaluate("MakeValidName()", "empty", "An empty name stays empty", MString(C.MakeValidName("").c_str()), MString("")) && Passed;
  Passed = Evaluate("MakeValidName()", "only special characters", "A name of special characters becomes empty", MString(C.MakeValidName("!!! ???").c_str()), MString("")) && Passed;
  Passed = Evaluate("MakeValidName()", "digits", "Digits are kept", MString(C.MakeValidName("0123456789").c_str()), MString("0123456789")) && Passed;

  // Unique names of materials, logical and physical volumes have different prefixes:
  UTGeometryConverterFixture F;
  G4Material* A = F.AddMaterial("A B", 1.0*g/cm3);
  G4Material* B = F.AddMaterial("AB", 2.0*g/cm3);
  G4LogicalVolume* Logical = F.AddLogical(F.Own(new G4Box("box", 1*cm, 1*cm, 1*cm)), "AB");
  G4VPhysicalVolume* Physical = F.Place(Logical, "AB", nullptr, G4RotationMatrix(), G4ThreeVector());
  Passed = Evaluate("CreateUniqueName(G4Material*)", "A B", "The first material gets index 0 and the prefix m", MString(C.CreateUniqueName(A).c_str()), MString("mAB_0")) && Passed;
  Passed = Evaluate("CreateUniqueName(G4Material*)", "AB", "A material with the same valid name gets the next index", MString(C.CreateUniqueName(B).c_str()), MString("mAB_1")) && Passed;
  Passed = Evaluate("CreateUniqueName(G4LogicalVolume*)", "AB", "A logical volume gets index 0 and the prefix l", MString(C.CreateUniqueName(Logical).c_str()), MString("lAB_0")) && Passed;
  Passed = Evaluate("CreateUniqueName(G4VPhysicalVolume*)", "AB", "A physical volume gets index 0 and the prefix p", MString(C.CreateUniqueName(Physical).c_str()), MString("pAB_0")) && Passed;

  // A second logical volume with the same name:
  G4LogicalVolume* Second = F.AddLogical(F.Own(new G4Box("box2", 1*cm, 1*cm, 1*cm)), "AB");
  Passed = Evaluate("CreateUniqueName(G4LogicalVolume*)", "AB twice", "A second logical volume with the same name gets the next index", MString(C.CreateUniqueName(Second).c_str()), MString("lAB_1")) && Passed;

  // Names which end with an underscore and a number cannot be confused with the index:
  G4LogicalVolume* Tricky1 = F.AddLogical(F.Own(new G4Box("box3", 1*cm, 1*cm, 1*cm)), "X");
  G4LogicalVolume* Tricky2 = F.AddLogical(F.Own(new G4Box("box4", 1*cm, 1*cm, 1*cm)), "X_0");
  G4LogicalVolume* Tricky3 = F.AddLogical(F.Own(new G4Box("box5", 1*cm, 1*cm, 1*cm)), "X");
  MString N1 = C.CreateUniqueName(Tricky1).c_str();
  MString N2 = C.CreateUniqueName(Tricky2).c_str();
  MString N3 = C.CreateUniqueName(Tricky3).c_str();
  Passed = EvaluateTrue("CreateUniqueName(G4LogicalVolume*)", "X, X_0, X", "The three names are different", N1 != N2 && N1 != N3 && N2 != N3) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTGeometryConverter::TestVolumes()
{
  bool Passed = true;

  UTGeometryConverterFixture F;
  G4LogicalVolume* World = F.AddLogical(F.Own(new G4Box("World", 50*cm, 50*cm, 50*cm)), "World");
  G4LogicalVolume* Crystal = F.AddLogical(F.Own(new G4Tubs("Crystal", 0*cm, 4*cm, 6*cm, 0*deg, 360*deg)), "Crystal");
  F.Place(World, "World", nullptr, G4RotationMatrix(), G4ThreeVector());
  G4RotationMatrix Rot;
  Rot.rotateZ(90*deg);
  F.Place(Crystal, "Crystal", World, Rot, G4ThreeVector(5*cm, -3*cm, 2*cm));
  F.Place(Crystal, "Crystal", World, G4RotationMatrix(), G4ThreeVector(-5*cm, 0*cm, 0*cm));

  vector<string> Lines = ConvertToLines(GetTemporaryFileName("Volumes.geo.setup"));

  // Logical volumes:
  Passed = EvaluateTrue("Convert()", "World", "The world is a volume", HasLine(Lines, "Volume lWorld_0")) && Passed;
  Passed = EvaluateTrue("Convert()", "World", "The world has the material of its logical volume", HasLine(Lines, "lWorld_0.Material mWaterMat_0")) && Passed;
  Passed = Evaluate("Convert()", "World", "The shape of the world is a box with half lengths in cm", Values(Lines, "lWorld_0.Shape BOX "), Join({50, 50, 50})) && Passed;
  Passed = EvaluateTrue("Convert()", "Crystal", "The crystal is a volume", HasLine(Lines, "Volume lCrystal_0")) && Passed;
  Passed = EvaluateTrue("Convert()", "Crystal", "The crystal has the material of its logical volume", HasLine(Lines, "lCrystal_0.Material mWaterMat_0")) && Passed;
  Passed = Evaluate("Convert()", "Crystal", "The shape of the crystal is a tube", Values(Lines, "lCrystal_0.Shape TUBS "), Join({0, 4, 6, 0, 360})) && Passed;

  // The world has no mother and is not a copy:
  Passed = EvaluateTrue("Convert()", "World", "The world has no mother", HasLine(Lines, "lWorld_0.Mother 0")) && Passed;
  Passed = Evaluate("Convert()", "World", "The world is placed at the origin", Values(Lines, "lWorld_0.Position "), Join({0, 0, 0})) && Passed;
  Passed = Evaluate("Convert()", "World", "The world is not rotated", Values(Lines, "lWorld_0.Rotation ", true), Join({90, 0, 90, 90, 0, 0}, true)) && Passed;
  Passed = EvaluateFalse("Convert()", "World", "The world is not a copy", HasPrefix(Lines, "lWorld_0.Copy")) && Passed;

  // Both placements of the crystal are copies with different names:
  Passed = EvaluateTrue("Convert()", "Crystal placement 1", "The first placement is a copy of the crystal", HasLine(Lines, "lCrystal_0.Copy pCrystal_0")) && Passed;
  Passed = EvaluateTrue("Convert()", "Crystal placement 2", "The second placement is a copy with the next index", HasLine(Lines, "lCrystal_0.Copy pCrystal_1")) && Passed;
  Passed = Evaluate("Convert()", "Crystal placement 1", "The position of the first placement is given in cm", Values(Lines, "pCrystal_0.Position "), Join({5, -3, 2})) && Passed;
  Passed = Evaluate("Convert()", "Crystal placement 1", "The rotation of the first placement is 90 deg around z: the rotated x-axis, y-axis, and z-axis as theta and phi", Values(Lines, "pCrystal_0.Rotation ", true), Join({90, 90, 90, 180, 0, 0}, true)) && Passed;
  Passed = EvaluateTrue("Convert()", "Crystal placement 1", "The mother of the first placement is the world", HasLine(Lines, "pCrystal_0.Mother lWorld_0")) && Passed;
  Passed = Evaluate("Convert()", "Crystal placement 2", "The position of the second placement is given in cm", Values(Lines, "pCrystal_1.Position "), Join({-5, 0, 0})) && Passed;
  Passed = Evaluate("Convert()", "Crystal placement 2", "The second placement is not rotated", Values(Lines, "pCrystal_1.Rotation ", true), Join({90, 0, 90, 90, 0, 0}, true)) && Passed;
  Passed = EvaluateTrue("Convert()", "Crystal placement 2", "The mother of the second placement is the world", HasLine(Lines, "pCrystal_1.Mother lWorld_0")) && Passed;

  // Every material and volume appears exactly once:
  unsigned int NVolumes = 0;
  unsigned int NMaterials = 0;
  for (const string& L: Lines) {
    if (L.compare(0, 7, "Volume ") == 0) ++NVolumes;
    if (L.compare(0, 9, "Material ") == 0) ++NMaterials;
  }
  Passed = Evaluate("Convert()", "volumes", "Every logical volume is written once", NVolumes, 2U) && Passed;
  Passed = Evaluate("Convert()", "materials", "Every material is written once", NMaterials, 1U) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTGeometryConverter::TestSimpleShapes()
{
  bool Passed = true;

  // Box:
  {
    unique_ptr<G4Box> S(new G4Box("box", 1*cm, 2*cm, 3*cm));
    vector<string> L = ShapeLines(S.get(), "Box");
    Passed = Evaluate("ConvertShape()", "G4Box", "A box is written with its half lengths in cm", Values(L, "Box.Shape BOX "), Join({1, 2, 3})) && Passed;
  }

  // Sphere: rmin, rmax, theta start, theta stop, phi start, phi stop
  {
    unique_ptr<G4Sphere> S(new G4Sphere("sphere", 1*cm, 5*cm, 10*deg, 90*deg, 20*deg, 60*deg));
    vector<string> L = ShapeLines(S.get(), "Sphere");
    Passed = Evaluate("ConvertShape()", "G4Sphere", "A sphere is written with radii in cm and start and stop angles in deg: phi start 10 deg + 90 deg, theta start 20 deg + 60 deg", Values(L, "Sphere.Shape SPHE "), Join({1, 5, 20, 80, 10, 100})) && Passed;
  }

  // Tube segment: rmin, rmax, half z, phi start, phi stop
  {
    unique_ptr<G4Tubs> S(new G4Tubs("tubs", 1*cm, 4*cm, 6*cm, 30*deg, 120*deg));
    vector<string> L = ShapeLines(S.get(), "Tubs");
    Passed = Evaluate("ConvertShape()", "G4Tubs", "A tube is written with radii and half length in cm and start and stop angle in deg", Values(L, "Tubs.Shape TUBS "), Join({1, 4, 6, 30, 150})) && Passed;
  }

  // Cone segment: half z, rmin(-z), rmax(-z), rmin(+z), rmax(+z), phi start, phi stop
  {
    unique_ptr<G4Cons> S(new G4Cons("cons", 1*cm, 2*cm, 3*cm, 4*cm, 5*cm, 0*deg, 180*deg));
    vector<string> L = ShapeLines(S.get(), "Cons");
    Passed = Evaluate("ConvertShape()", "G4Cons", "A cone is written with the half length first and then the radii at -z and +z", Values(L, "Cons.Shape CONS "), Join({5, 1, 2, 3, 4, 0, 180})) && Passed;
  }

  // Trd: dx1, dx2, dy1, dy2, dz
  {
    unique_ptr<G4Trd> S(new G4Trd("trd", 1*cm, 2*cm, 3*cm, 4*cm, 5*cm));
    vector<string> L = ShapeLines(S.get(), "Trd");
    Passed = Evaluate("ConvertShape()", "G4Trd", "A trapezoid is written with dx1 dx2 dy1 dy2 dz in cm", Values(L, "Trd.Shape TRD2 "), Join({1, 2, 3, 4, 5})) && Passed;
  }

  // The parameter line of a shape which is not part of a volume:
  {
    unique_ptr<G4Box> S(new G4Box("box", 1*cm, 2*cm, 3*cm));
    vector<string> L = ShapeLines(S.get(), "Standalone", false);
    Passed = EvaluateTrue("ConvertShape()", "not part of a volume", "A stand-alone shape starts with its shape statement", HasLine(L, "Shape BOX Standalone")) && Passed;
    Passed = Evaluate("ConvertShape()", "not part of a volume", "A stand-alone shape has a parameter line", Values(L, "Standalone.Parameters "), Join({1, 2, 3})) && Passed;
  }

  // The return value:
  {
    unique_ptr<G4Box> S(new G4Box("box", 1*cm, 2*cm, 3*cm));
    MCGeometryConverterAccess C;
    ofstream Out(GetTemporaryFileName("ShapeReturn.txt").Data());
    G4String Name = "Return";
    Passed = EvaluateTrue("ConvertShape()", "G4Box", "A supported shape is converted successfully", C.ConvertShape(Out, Name, S.get(), true)) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTGeometryConverter::TestTrap()
{
  bool Passed = true;

  // A prism-like trapezoid: dz, theta, phi, dy1, dx1, dx2, alpha1, dy2, dx3, dx4, alpha2
  auto MakeTrap = [](double Phi) {
    return new G4Trap("trap", 10*cm, 20*deg, Phi, 4*cm, 3*cm, 5*cm, 10*deg, 4*cm, 3*cm, 5*cm, 10*deg);
  };

  const double PhiValues[] = { 30, 60, 90, 150, -30, 210, 0 };
  for (double Phi: PhiValues) {
    unique_ptr<G4Trap> S(MakeTrap(Phi*deg));
    vector<string> L = ShapeLines(S.get(), "Trap");
    MString Input = MString("phi = ") + Phi + " deg";
    // Known failure: phi is taken from acos(), which cannot give angles outside 0..180 deg (-30 becomes 30, 210 becomes 150), and its argument is not clamped to [-1, 1] (nan for phi = 0)
    if (Phi == -30 || Phi == 210 || Phi == 0) {
      Passed = ExpectedToFail(Evaluate("ConvertShape()", Input, "The trapezoid parameters are written with the angles theta and phi, and the alpha angles", Values(L, "Trap.Shape TRAP ", true), Join({10, 20, Phi, 4, 3, 5, 10, 4, 3, 5, 10}, true))) && Passed;
    } else {
      Passed = Evaluate("ConvertShape()", Input, "The trapezoid parameters are written with the angles theta and phi, and the alpha angles", Values(L, "Trap.Shape TRAP ", true), Join({10, 20, Phi, 4, 3, 5, 10, 4, 3, 5, 10}, true)) && Passed;
    }
    // Known failure: phi1 from acos() and phi2 from asin() only agree in the first quadrant, so a valid trapezoid with phi outside 0..90 deg is reported as probably wrong
    if (Phi == 150 || Phi == -30 || Phi == 210) {
      Passed = ExpectedToFail(EvaluateFalse("ConvertShape()", Input, "A valid trapezoid is not reported as probably wrong", HasPrefix(L, "Echo Error"))) && Passed;
    } else {
      Passed = EvaluateFalse("ConvertShape()", Input, "A valid trapezoid is not reported as probably wrong", HasPrefix(L, "Echo Error")) && Passed;
    }
  }

  // A trapezoid without tilt:
  {
    unique_ptr<G4Trap> S(new G4Trap("trap", 10*cm, 0*deg, 0*deg, 4*cm, 3*cm, 5*cm, 0*deg, 4*cm, 3*cm, 5*cm, 0*deg));
    vector<string> L = ShapeLines(S.get(), "Untilted");
    Passed = Evaluate("ConvertShape()", "untilted", "An untilted trapezoid has zero theta and phi", Values(L, "Untilted.Shape TRAP ", true), Join({10, 0, 0, 4, 3, 5, 0, 4, 3, 5, 0}, true)) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTGeometryConverter::TestPolyhedra()
{
  bool Passed = true;

  // Polycone: start angle, opening angle, number of planes, then z, rmin, rmax for every plane
  {
    const double Z[3] = { -2*cm, 0*cm, 3*cm };
    const double RMin[3] = { 0*cm, 1*cm, 0*cm };
    const double RMax[3] = { 2*cm, 3*cm, 1*cm };
    unique_ptr<G4Polycone> S(new G4Polycone("pcon", 10*deg, 200*deg, 3, Z, RMin, RMax));
    vector<string> L = ShapeLines(S.get(), "Pcon");
    Passed = Evaluate("ConvertShape()", "G4Polycone", "A polycone is written with start angle, opening angle, and for every plane z, rmin, rmax", Values(L, "Pcon.Shape PCON "), Join({10, 200, 3, -2, 0, 2, 0, 1, 3, 3, 0, 1})) && Passed;
  }

  // Polyhedra: Geant4 and Geomega (ROOT) define the radii as the distances to the sides (tangent / inscribed circle)
  // Geant4 stores the corner radii internally - the geometry file must contain the given distances to the sides
  {
    const double Z[2] = { -1*cm, 1*cm };
    const double RMin[2] = { 0*cm, 0*cm };
    const double RMax[2] = { 2*cm, 3*cm };
    unique_ptr<G4Polyhedra> S(new G4Polyhedra("pgon", 0*deg, 360*deg, 6, 2, Z, RMin, RMax));
    vector<string> L = ShapeLines(S.get(), "Pgon");
    // Known failure: the corner radii stored in G4PolyhedraHistorical (distance / cos(pi/n)) are written unconverted instead of the distances to the sides
    Passed = ExpectedToFail(Evaluate("ConvertShape()", "G4Polyhedra hexagon", "A hexagon is written with the distances to the sides as radii", Values(L, "Pgon.Shape PGON "), Join({0, 360, 6, 2, -1, 0, 2, 1, 0, 3}))) && Passed;
  }
  {
    const double Z[2] = { 0*cm, 4*cm };
    const double RMin[2] = { 1*cm, 1*cm };
    const double RMax[2] = { 2*cm, 2*cm };
    unique_ptr<G4Polyhedra> S(new G4Polyhedra("pgon", 45*deg, 360*deg, 4, 2, Z, RMin, RMax));
    vector<string> L = ShapeLines(S.get(), "Square");
    // Known failure: the corner radii stored in G4PolyhedraHistorical (distance / cos(pi/n)) are written unconverted instead of the distances to the sides
    Passed = ExpectedToFail(Evaluate("ConvertShape()", "G4Polyhedra square", "A square tube is written with the distances to the sides as radii", Values(L, "Square.Shape PGON "), Join({45, 360, 4, 2, 0, 1, 2, 4, 1, 2}))) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTGeometryConverter::TestBooleanSolids()
{
  bool Passed = true;

  // Subtraction with a displaced second solid:
  {
    unique_ptr<G4Box> A(new G4Box("A", 10*cm, 10*cm, 10*cm));
    unique_ptr<G4Tubs> B(new G4Tubs("B", 0*cm, 2*cm, 3*cm, 0*deg, 360*deg));
    G4RotationMatrix Rot;
    Rot.rotateZ(90*deg);
    unique_ptr<G4SubtractionSolid> S(new G4SubtractionSolid("sub", A.get(), B.get(), G4Transform3D(Rot, G4ThreeVector(1*cm, 2*cm, 3*cm))));
    vector<string> L = ShapeLines(S.get(), "Sub");
    Passed = EvaluateTrue("ConvertShape()", "subtraction", "A subtraction names both solids and the orientation", HasLine(L, "Sub.Shape Subtraction Sub_SubtractionVolumeA Sub_SubtractionVolumeB Sub_SubtractionVolumeB_Orientation")) && Passed;
    Passed = EvaluateTrue("ConvertShape()", "subtraction", "The orientation is declared", HasLine(L, "Orientation Sub_SubtractionVolumeB_Orientation")) && Passed;
    Passed = Evaluate("ConvertShape()", "subtraction", "The position of the orientation is given in cm", Values(L, "Sub_SubtractionVolumeB_Orientation.Position "), Join({1, 2, 3})) && Passed;
    Passed = Evaluate("ConvertShape()", "subtraction", "The rotation of the orientation is given as theta and phi of the axes", Values(L, "Sub_SubtractionVolumeB_Orientation.Rotation ", true), Join({90, 90, 90, 180, 0, 0}, true)) && Passed;
    Passed = EvaluateTrue("ConvertShape()", "subtraction", "The first solid is a stand-alone box", HasLine(L, "Shape BOX Sub_SubtractionVolumeA")) && Passed;
    Passed = Evaluate("ConvertShape()", "subtraction", "The first solid has its parameters", Values(L, "Sub_SubtractionVolumeA.Parameters "), Join({10, 10, 10})) && Passed;
    Passed = EvaluateTrue("ConvertShape()", "subtraction", "The second solid is a stand-alone tube", HasLine(L, "Shape TUBS Sub_SubtractionVolumeB")) && Passed;
    Passed = Evaluate("ConvertShape()", "subtraction", "The second solid has its parameters", Values(L, "Sub_SubtractionVolumeB.Parameters "), Join({0, 2, 3, 0, 360})) && Passed;
  }

  // Subtraction without displacement:
  {
    unique_ptr<G4Box> A(new G4Box("A", 10*cm, 10*cm, 10*cm));
    unique_ptr<G4Box> B(new G4Box("B", 1*cm, 1*cm, 1*cm));
    unique_ptr<G4SubtractionSolid> S(new G4SubtractionSolid("sub", A.get(), B.get()));
    vector<string> L = ShapeLines(S.get(), "Plain");
    Passed = EvaluateTrue("ConvertShape()", "plain subtraction", "A subtraction without displacement has no orientation", HasLine(L, "Plain.Shape Subtraction Plain_SubtractionVolumeA Plain_SubtractionVolumeB")) && Passed;
    Passed = EvaluateFalse("ConvertShape()", "plain subtraction", "A subtraction without displacement declares no orientation", HasPrefix(L, "Orientation ")) && Passed;
  }

  // Union and intersection: the names of the two solids are taken from the statement
  struct Case { const char* Type; };
  for (unsigned int c = 0; c < 2; ++c) {
    unique_ptr<G4Box> A(new G4Box("A", 10*cm, 10*cm, 10*cm));
    unique_ptr<G4Box> B(new G4Box("B", 1*cm, 2*cm, 3*cm));
    unique_ptr<G4VSolid> S;
    string Type;
    if (c == 0) {
      S.reset(new G4UnionSolid("union", A.get(), B.get(), nullptr, G4ThreeVector(0, 0, 5*cm)));
      Type = "Union";
    } else {
      S.reset(new G4IntersectionSolid("intersection", A.get(), B.get(), nullptr, G4ThreeVector(0, 0, 5*cm)));
      Type = "Intersection";
    }
    vector<string> L = ShapeLines(S.get(), Type);
    string Prefix = Type + ".Shape " + Type + " ";
    string Statement;
    for (const string& Line: L) {
      if (Line.compare(0, Prefix.size(), Prefix) == 0) Statement = Line;
    }
    Passed = EvaluateFalse("ConvertShape()", MString(Type.c_str()), "The statement of the boolean solid exists", Statement.empty()) && Passed;
    istringstream In(Statement.substr(min(Prefix.size(), Statement.size())));
    string NameA, NameB, OrientationName;
    In>>NameA>>NameB>>OrientationName;
    Passed = EvaluateTrue("ConvertShape()", MString(Type.c_str()), "The first solid is named after the boolean solid", NameA == Type + "_" + Type + "VolumeA") && Passed;
    Passed = EvaluateFalse("ConvertShape()", MString(Type.c_str()), "The second solid has its own name", NameB.empty() || NameB == NameA) && Passed;
    Passed = EvaluateTrue("ConvertShape()", MString(Type.c_str()), "The displaced second solid gets an orientation named after it", OrientationName == NameB + "_Orientation") && Passed;
    Passed = EvaluateTrue("ConvertShape()", MString(Type.c_str()), "The second solid is a stand-alone box", HasLine(L, "Shape BOX " + NameB)) && Passed;
    Passed = Evaluate("ConvertShape()", MString(Type.c_str()), "The second solid has its parameters", Values(L, NameB + ".Parameters "), Join({1, 2, 3})) && Passed;
    Passed = Evaluate("ConvertShape()", MString(Type.c_str()), "The orientation has the translation in cm", Values(L, NameB + "_Orientation.Position "), Join({0, 0, 5})) && Passed;
    Passed = Evaluate("ConvertShape()", MString(Type.c_str()), "The first solid has its parameters", Values(L, NameA + ".Parameters "), Join({10, 10, 10})) && Passed;
  }

  // Nested boolean solids are converted recursively:
  {
    unique_ptr<G4Box> A(new G4Box("A", 10*cm, 10*cm, 10*cm));
    unique_ptr<G4Box> B(new G4Box("B", 1*cm, 1*cm, 1*cm));
    unique_ptr<G4Box> C(new G4Box("C", 2*cm, 2*cm, 2*cm));
    unique_ptr<G4UnionSolid> AB(new G4UnionSolid("AB", A.get(), B.get()));
    unique_ptr<G4SubtractionSolid> S(new G4SubtractionSolid("nested", AB.get(), C.get()));
    vector<string> L = ShapeLines(S.get(), "Nested");
    Passed = EvaluateTrue("ConvertShape()", "nested", "The outer subtraction names the union as first solid", HasLine(L, "Nested.Shape Subtraction Nested_SubtractionVolumeA Nested_SubtractionVolumeB")) && Passed;
    Passed = EvaluateTrue("ConvertShape()", "nested", "The union is declared as stand-alone shape", HasLine(L, "Shape Union Nested_SubtractionVolumeA")) && Passed;
    Passed = EvaluateTrue("ConvertShape()", "nested", "The parameters of the union name its two solids", HasPrefix(L, "Nested_SubtractionVolumeA.Parameters Nested_SubtractionVolumeA_")) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTGeometryConverter::TestUnknownShape()
{
  bool Passed = true;

  // An orb is not supported - it must be replaced by a virtual placeholder volume which can be read
  unique_ptr<G4Orb> S(new G4Orb("orb", 3*cm));
  vector<string> L = ShapeLines(S.get(), "Orb");
  Passed = EvaluateTrue("ConvertShape()", "G4Orb", "An unknown shape is marked as virtual", HasLine(L, "Orb.Virtual true")) && Passed;
  bool HasErrorShape = false;
  for (const string& Line: L) {
    if (Line.find(".Shape ERROR") != string::npos) HasErrorShape = true;
  }
  // Known failure: the shape statement of an unknown shape is written with the type ERROR, and "BOX 100.0 100.0 100.0" is appended behind it
  Passed = ExpectedToFail(EvaluateFalse("ConvertShape()", "G4Orb", "An unknown shape does not produce a shape statement with the invalid shape type ERROR", HasErrorShape)) && Passed;
  // Known failure: the placeholder box is appended to the "Orb.Shape ERROR " prefix instead of replacing the shape type
  Passed = ExpectedToFail(EvaluateTrue("ConvertShape()", "G4Orb", "An unknown shape is written as a placeholder box statement", HasPrefix(L, "Orb.Shape BOX 100"))) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTGeometryConverter::TestOrientationAndZeroing()
{
  bool Passed = true;

  MCGeometryConverterAccess C;

  // Zeroing:
  Passed = EvaluateNear("Zeroing()", "1e-11", "A very small number becomes zero", C.Zeroing(1e-11), 0.0, 1e-300) && Passed;
  Passed = EvaluateNear("Zeroing()", "-1e-11", "A very small negative number becomes zero", C.Zeroing(-1e-11), 0.0, 1e-300) && Passed;
  Passed = EvaluateNear("Zeroing()", "0", "Zero stays zero", C.Zeroing(0.0), 0.0, 1e-300) && Passed;
  Passed = EvaluateNear("Zeroing()", "1e-9", "A number above the limit is kept", C.Zeroing(1e-9), 1e-9, 1e-20) && Passed;
  Passed = EvaluateNear("Zeroing()", "-0.5", "A regular number is kept", C.Zeroing(-0.5), -0.5, 1e-15) && Passed;
  Passed = EvaluateNear("Zeroing()", "90", "An angle is kept", C.Zeroing(90.0), 90.0, 1e-12) && Passed;

  // Orientation:
  G4RotationMatrix Rot;
  Rot.rotateZ(90*deg);
  const MString FileName = GetTemporaryFileName("Orientation.txt");
  bool Result = false;
  {
    ofstream Out(FileName.Data());
    Result = C.ConvertOrientation(Out, "Orient", G4ThreeVector(1*cm, 2*cm, 3*cm), Rot);
  }
  vector<string> L = Split(ReadTextFile(FileName));
  Passed = EvaluateTrue("ConvertOrientation()", "rotation about z", "The orientation is converted successfully", Result) && Passed;
  Passed = EvaluateTrue("ConvertOrientation()", "rotation about z", "The orientation is declared", HasLine(L, "Orientation Orient")) && Passed;
  Passed = Evaluate("ConvertOrientation()", "rotation about z", "The position is given in cm", Values(L, "Orient.Position "), Join({1, 2, 3})) && Passed;
  Passed = Evaluate("ConvertOrientation()", "rotation about z", "The rotation is given as theta and phi of the rotated x, y, and z axis", Values(L, "Orient.Rotation ", true), Join({90, 90, 90, 180, 0, 0}, true)) && Passed;

  // Identity:
  {
    ofstream Out(FileName.Data());
    C.ConvertOrientation(Out, "Identity", G4ThreeVector(), G4RotationMatrix());
  }
  L = Split(ReadTextFile(FileName));
  Passed = Evaluate("ConvertOrientation()", "identity", "The identity has the axes at the coordinate axes", Values(L, "Identity.Rotation ", true), Join({90, 0, 90, 90, 0, 0}, true)) && Passed;
  Passed = Evaluate("ConvertOrientation()", "identity", "The identity has no translation", Values(L, "Identity.Position "), Join({0, 0, 0})) && Passed;

  // A generic rotation: rotation about x by 30 deg - the rotated y-axis is (0, cos 30, sin 30): theta = 60 deg, phi = 90 deg
  {
    G4RotationMatrix RotX;
    RotX.rotateX(30*deg);
    ofstream Out(FileName.Data());
    C.ConvertOrientation(Out, "RotX", G4ThreeVector(-1.5*cm, 0, 0), RotX);
  }
  L = Split(ReadTextFile(FileName));
  // Rotated x-axis (1,0,0): theta 90, phi 0; y-axis (0, cos 30, sin 30): theta 60, phi 90; z-axis (0, -sin 30, cos 30): theta 30, phi 270
  Passed = Evaluate("ConvertOrientation()", "rotation about x", "The rotated axes are written as theta and phi", Values(L, "RotX.Rotation ", true), Join({90, 0, 60, 90, 30, 270}, true)) && Passed;
  Passed = Evaluate("ConvertOrientation()", "rotation about x", "A negative translation is converted to cm", Values(L, "RotX.Position "), Join({-1.5, 0, 0})) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTGeometryConverter::TestPrecision()
{
  bool Passed = true;

  // The numbers are written with 9 significant digits - also after a trapezoid, which needs more digits
  {
    UTGeometryConverterFixture F;
    F.AddLogical(F.Own(new G4Box("Box", 1.23456789*cm, 2.3456789*cm, 3.456789*cm)), "PlainBox");
    vector<string> Lines = ConvertToLines(GetTemporaryFileName("PrecisionBox.geo.setup"));
    Passed = Evaluate("Convert()", "box only", "A box is written with 9 digits", Values(Lines, "lPlainBox_0.Shape BOX "), Join({1.23456789, 2.3456789, 3.456789})) && Passed;
    // Values() rounds to 6 digits after the decimal point, thus compare the text
    bool NineDigits = false;
    for (const string& L: Lines) {
      if (L == "lPlainBox_0.Shape BOX 1.23456789 2.3456789 3.456789") NineDigits = true;
    }
    Passed = EvaluateTrue("Convert()", "box only", "The digits of the half lengths are kept", NineDigits) && Passed;
  }
  {
    UTGeometryConverterFixture F;
    F.AddLogical(F.Own(new G4Trap("trap", 10*cm, 20*deg, 30*deg, 4*cm, 3*cm, 5*cm, 10*deg, 4*cm, 3*cm, 5*cm, 10*deg)), "Trap");
    F.AddLogical(F.Own(new G4Box("Box", 1.23456789*cm, 2.3456789*cm, 3.456789*cm)), "AfterTrap");
    vector<string> Lines = ConvertToLines(GetTemporaryFileName("PrecisionTrap.geo.setup"));
    bool NineDigits = false;
    for (const string& L: Lines) {
      if (L == "lAfterTrap_0.Shape BOX 1.23456789 2.3456789 3.456789") NineDigits = true;
    }
    // Known failure: the trapezoid branch of ConvertShape() resets the stream to setprecision(6) instead of the 9 digits set by Convert()
    Passed = ExpectedToFail(EvaluateTrue("Convert()", "box after trapezoid", "A trapezoid does not reduce the number of digits of the following shapes", NineDigits)) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTGeometryConverter::TestReuse()
{
  bool Passed = true;

  UTGeometryConverterFixture F;
  G4LogicalVolume* World = F.AddLogical(F.Own(new G4Box("World", 50*cm, 50*cm, 50*cm)), "World");
  G4LogicalVolume* Crystal = F.AddLogical(F.Own(new G4Tubs("Crystal", 0*cm, 4*cm, 6*cm, 0*deg, 360*deg)), "Crystal");
  F.Place(World, "World", nullptr, G4RotationMatrix(), G4ThreeVector());
  F.Place(Crystal, "Crystal", World, G4RotationMatrix(), G4ThreeVector(5*cm, 0, 0));

  // The same converter converts the same unchanged geometry twice - the files must be identical:
  MCGeometryConverter Converter;
  const MString First = GetTemporaryFileName("First.geo.setup");
  const MString Second = GetTemporaryFileName("Second.geo.setup");
  
  DisableDefaultStreams();
  ostringstream Sink;
  streambuf* Old = cout.rdbuf(Sink.rdbuf());
  bool OK1 = Converter.Convert(First.Data());
  bool OK2 = Converter.Convert(Second.Data());
  cout.rdbuf(Old);
  EnableDefaultStreams();
  Passed = EvaluateTrue("Convert()", "first call", "The first conversion succeeds", OK1) && Passed;
  Passed = EvaluateTrue("Convert()", "second call", "The second conversion succeeds", OK2) && Passed;
  // The unique-name counters are not reset between conversions (names change, e.g. mWaterMat_1), thus check consistency instead of identity:
  vector<string> Lines2 = Split(ReadTextFile(Second));
  string MaterialLine;
  for (const string& L: Lines2) {
    if (L.compare(0, 9, "Material ") == 0) MaterialLine = L.substr(9);
  }
  while (MaterialLine.empty() == false && isspace((unsigned char) MaterialLine.back())) MaterialLine.pop_back();
  Passed = EvaluateFalse("Convert()", "second call", "The second file declares a material", MaterialLine.empty()) && Passed;
  Passed = EvaluateTrue("Convert()", "second call", "The volumes of the second file use the declared material", HasLine(Lines2, "lWorld_1.Material " + MaterialLine)) && Passed;

  // A new converter gives the same file as the first one:
  vector<string> Lines = ConvertToLines(GetTemporaryFileName("Third.geo.setup"));
  Passed = EvaluateTrue("Convert()", "new converter", "A new converter gives the same names as the first conversion", HasLine(Lines, "Volume lWorld_0")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTGeometryConverter Test;
  return Test.Run() == true ? 0 : 1;
}
