/*
 * UTDGDMLImport.cxx
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
#include <functional>
#include <iostream>
#include <sstream>

// POSIX libs: process control has no equivalent in the C++ standard library
#include <signal.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <unistd.h>
using namespace std;

// ROOT libs:
#include <TApplication.h>
#include <TError.h>
#include <TGeoManager.h>
#include <TROOT.h>
#include <TSystem.h>

// MEGAlib libs:
#include "MDGDMLImport.h"
#include "MUnitTest.h"
#include "UTGeomegaShared.h"


////////////////////////////////////////////////////////////////////////////////


//! Unit test of the importer of GDML files MDGDMLImport
class UTDGDMLImport : public UTGeomegaBase
{
public:
  //! Default constructor
  UTDGDMLImport() : UTGeomegaBase("UTDGDMLImport") {}
  //! Default destructor
  virtual ~UTDGDMLImport() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Return the text of a GDML file with the given solids and a volume made from the solid InnerShape
  MString GetGDML(const MString& Solids) const;

  //! Test the object before a file is read
  bool TestNoFile();
  //! Test the error paths of Parse
  bool TestBadFiles();
  //! Test the conversion of a file with boxes
  bool TestBoxes();
  //! Test the conversion of a file with an unsupported shape
  bool TestUnsupportedShape();
  //! Test the global state after an import
  bool TestGlobalState();
};


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTDGDMLImport::Run()
{
  bool Passed = true;

  Passed = TestNoFile() && Passed;
  Passed = TestBadFiles() && Passed;
  Passed = TestBoxes() && Passed;
  Passed = TestUnsupportedShape() && Passed;
  Passed = TestGlobalState() && Passed;

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the text of a GDML file with the given solids and a volume made from the solid InnerShape
MString UTDGDMLImport::GetGDML(const MString& Solids) const
{
  MString Text = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<gdml xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">\n<define/>\n";
  Text += "<materials><material name=\"Al\" Z=\"13\"><D value=\"2.7\"/><atom value=\"26.98\"/></material></materials>\n";
  Text += "<solids><box name=\"WorldBox\" x=\"100\" y=\"100\" z=\"100\" lunit=\"cm\"/>" + Solids + "</solids>\n";
  Text += "<structure><volume name=\"GDMLInner\"><materialref ref=\"Al\"/><solidref ref=\"InnerShape\"/></volume>\n";
  Text += "<volume name=\"GDMLWorld\"><materialref ref=\"Al\"/><solidref ref=\"WorldBox\"/><physvol><volumeref ref=\"GDMLInner\"/></physvol></volume></structure>\n";
  Text += "<setup name=\"Default\" version=\"1.0\"><world ref=\"GDMLWorld\"/></setup>\n</gdml>\n";
  return Text;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the object before a file is read
bool UTDGDMLImport::TestNoFile()
{
  bool Passed = true;

  MDGDMLImport Importer;
  Passed = EvaluateTrue("GetAsGeomega()", "nothing read", "Without a file the text is empty", Importer.GetAsGeomega() == "") && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the error paths of Parse
bool UTDGDMLImport::TestBadFiles()
{
  bool Passed = true;

  MDGDMLImport Importer;
  DisableDefaultStreams();
  bool Missing = Importer.Parse(GetTemporaryFileName("DoesNotExist.gdml"));
  bool Empty = Importer.Parse("");
  EnableDefaultStreams();
  Passed = EvaluateFalse("Parse()", "missing file", "A missing file is rejected", Missing) && Passed;
  Passed = EvaluateFalse("Parse()", "empty file name", "An empty file name is rejected", Empty) && Passed;
  Passed = EvaluateTrue("GetAsGeomega()", "after a missing file", "After a missing file the text is empty", Importer.GetAsGeomega() == "") && Passed;

  // A file with an extension of neither GDML nor ROOT:
  const MString Text = GetTemporaryFileName("Text.txt");
  // Without its file the scenario cannot run: a missing file would look like the known failure
  if (EvaluateTrue("WriteTextFile()", "fixture", "The file of the test can be written", WriteTextFile(Text, "Just some text\n")) == false) {
    return false;
  }
  // Defect GEO-A-56: Parse returns true for every existing file, also if the import fails and there is no geometry
  MDGDMLImport NoGeometry;
  // Hide the error messages of ROOT about the file
  Int_t ErrorLevel = gErrorIgnoreLevel;
  gErrorIgnoreLevel = kFatal;
  const bool IsTextParsed = NoGeometry.Parse(Text);
  gErrorIgnoreLevel = ErrorLevel;
  Passed = ExpectedToFail(EvaluateFalse("Parse()", "text file", "A file which is no geometry is rejected", IsTextParsed)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the conversion of a file with boxes
bool UTDGDMLImport::TestBoxes()
{
  bool Passed = true;

  const MString File = GetTemporaryFileName("Boxes.gdml");
  // Without its file the scenario cannot run: a missing file would look like the known failure
  if (EvaluateTrue("WriteTextFile()", "fixture", "The file of the test can be written", WriteTextFile(File, GetGDML("<box name=\"InnerShape\" x=\"10\" y=\"20\" z=\"30\" lunit=\"cm\"/>"))) == false) {
    return false;
  }

  MDGDMLImport Importer;
  Passed = EvaluateTrue("Parse()", "file with boxes", "The file is read", Importer.Parse(File)) && Passed;

  MString Text;
  BeginCaptureOutput();
  Text = Importer.GetAsGeomega();
  const MString Printed = EndCaptureOutput();

  // The result is a geometry: read it back, and compare its volumes with the file (half lengths are half of the full lengths in the file)
  MDGeometry Geometry;
  const bool Scanned = ScanGeometry(Geometry, Text);
  MDVolume* Inner = Scanned == true ? Geometry.GetVolume("GDMLInner") : nullptr;
  MDVolume* World = Scanned == true ? Geometry.GetVolume("GDMLWorld") : nullptr;
  // Defect GEO-A-55: GetAsGeomega always returns an empty text and prints its progress directly to the screen
  Passed = ExpectedToFail(EvaluateTrue("GetAsGeomega()", "file with boxes", "The result can be read as a geometry", Scanned)) && Passed;
  // Defect GEO-A-55: GetAsGeomega always returns an empty text and prints its progress directly to the screen
  Passed = ExpectedToFail(EvaluateTrue("GetAsGeomega()", "file with boxes", "The inner volume has half lengths of 5, 10, 15", Inner != nullptr && Inner->GetShape() != nullptr && (Inner->GetShape()->GetSize() - MVector(5.0, 10.0, 15.0)).Mag() < 1e-9)) && Passed;
  // Defect GEO-A-55: GetAsGeomega always returns an empty text and prints its progress directly to the screen
  Passed = ExpectedToFail(EvaluateTrue("GetAsGeomega()", "file with boxes", "The world volume has half lengths of 50, 50, 50", World != nullptr && World->GetShape() != nullptr && (World->GetShape()->GetSize() - MVector(50.0, 50.0, 50.0)).Mag() < 1e-9)) && Passed;
  // Defect GEO-A-55: GetAsGeomega always returns an empty text and prints its progress directly to the screen
  Passed = ExpectedToFail(EvaluateTrue("GetAsGeomega()", "file with boxes", "Nothing is printed to the screen", Printed == "")) && Passed;

  // A second file with the same object:
  const MString Other = GetTemporaryFileName("Other.gdml");
  // Without its file the scenario cannot run: a missing file would look like the known failure
  if (EvaluateTrue("WriteTextFile()", "fixture", "The file of the test can be written", WriteTextFile(Other, GetGDML("<box name=\"InnerShape\" x=\"1\" y=\"2\" z=\"3\" lunit=\"cm\"/>"))) == false) {
    return false;
  }
  Passed = EvaluateTrue("Parse()", "second file", "A second file can be read", Importer.Parse(Other)) && Passed;
  MString TextSecond;
  BeginCaptureOutput();
  TextSecond = Importer.GetAsGeomega();
  EndCaptureOutput();
  MDGeometry GeometrySecond;
  MDVolume* InnerSecond = ScanGeometry(GeometrySecond, TextSecond) == true ? GeometrySecond.GetVolume("GDMLInner") : nullptr;
  // Defect GEO-A-55: GetAsGeomega always returns an empty text and prints its progress directly to the screen
  Passed = ExpectedToFail(EvaluateTrue("GetAsGeomega()", "second file", "The result has the volume of the second file with half lengths of 0.5, 1, 1.5, not the one of the first file", InnerSecond != nullptr && InnerSecond->GetShape() != nullptr && (InnerSecond->GetShape()->GetSize() - MVector(0.5, 1.0, 1.5)).Mag() < 1e-9)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the conversion of a file with an unsupported shape
bool UTDGDMLImport::TestUnsupportedShape()
{
  bool Passed = true;

  const MString File = GetTemporaryFileName("Tube.gdml");
  // Without its file the scenario cannot run: a missing file would look like the known failure
  if (EvaluateTrue("WriteTextFile()", "fixture", "The file of the test can be written", WriteTextFile(File, GetGDML("<tube name=\"InnerShape\" rmin=\"0\" rmax=\"5\" z=\"10\" deltaphi=\"360\" aunit=\"deg\" lunit=\"cm\"/>"))) == false) {
    return false;
  }

  MDGDMLImport Importer;
  Passed = EvaluateTrue("Parse()", "file with a tube", "The file is read", Importer.Parse(File)) && Passed;

  MString Text;
  DisableDefaultStreams();
  BeginCaptureOutput();
  Text = Importer.GetAsGeomega();
  EndCaptureOutput();
  EnableDefaultStreams();

  // Defect GEO-A-57: The error paths of GetAsGeomega return false, which is converted into a text and not into an empty text
  Passed = ExpectedToFail(EvaluateTrue("GetAsGeomega()", "unknown shape", "After an error the text is empty", Text == "")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the global state after an import
bool UTDGDMLImport::TestGlobalState()
{
  bool Passed = true;

  const MString File = GetTemporaryFileName("Global.gdml");
  // Without its file the scenario cannot run: a missing file would look like the known failure
  if (EvaluateTrue("WriteTextFile()", "fixture", "The file of the test can be written", WriteTextFile(File, GetGDML("<box name=\"InnerShape\" x=\"10\" y=\"20\" z=\"30\" lunit=\"cm\"/>"))) == false) {
    return false;
  }

  new TGeoManager("Before", "Before");
  MDGDMLImport Importer;
  Importer.Parse(File);
  // Defect GEO-A-60: Parse changes the global gGeoManager and leaves it there, which also changes the manager used by a surrounding MDGeometry
  // Attention: Compare the names - ROOT can delete the old manager, and a new one can get its address
  Passed = ExpectedToFail(Evaluate("Parse()", "gGeoManager", "The global ROOT geometry is the one from before the call", MString(gGeoManager != nullptr ? gGeoManager->GetName() : ""), MString("Before"))) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  // Batch mode first, otherwise ROOT connects to the display: a test must not depend on a display being available
  gROOT->SetBatch(true);
  TApplication ROOT("ROOT", 0, 0);
  gErrorIgnoreLevel = kError;

  UTDGDMLImport Test;
  return Test.Run() == true ? 0 : 1;
}


// UTDGDMLImport.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
