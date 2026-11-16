/*
 * UTERCSRTMVA.cxx
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
#include <iostream>
#include <vector>
using namespace std;

// ROOT libs:
#include "TApplication.h"
#include "TError.h"
#include "TROOT.h"

// MEGAlib libs:
#include "MERCSRTMVA.h"
#include "MERCSRTMVAMethods.h"
#include "UTERCSRShared.h"


////////////////////////////////////////////////////////////////////////////////


//! Unit test of the Compton sequence reconstruction with TMVA MERCSRTMVA
//! There are no trained TMVA weights in the tests, so only the handling of the steering file is tested
class UTERCSRTMVA : public UTERCSRBase
{
public:
  //! Default constructor
  UTERCSRTMVA() : UTERCSRBase("UTERCSRTMVA") {}
  //! Default destructor
  virtual ~UTERCSRTMVA() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Write a steering file with the content and return its name
  MString WriteSteeringFile(const MString& Name, const MString& Content);
  //! Set the parameters with a used method, whose weights file does not exist, and return the result
  bool SetParametersWithoutWeights();
  //! Set the parameters without a used method, analyze a two-site event, and return true if the event is not good
  bool AnalyzeWithoutMethods();

  //! Test a missing steering file
  bool TestMissingSteeringFile();
  //! Test steering files without used methods
  bool TestWithoutMethods();
  //! Test the rejected steering files
  bool TestRejectedSteeringFiles();

  //! The test telescope
  MGeometryRevan m_Geometry;
};


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTERCSRTMVA::Run()
{
  bool Passed = true;

  Passed = EvaluateTrue("ScanTelescopeGeometry()", "telescope", "The test telescope can be scanned", ScanTelescopeGeometry(m_Geometry)) && Passed;

  if (m_Geometry.IsScanned() == true) {
    Passed = TestMissingSteeringFile() && Passed;
    Passed = TestWithoutMethods() && Passed;
    Passed = TestRejectedSteeringFiles() && Passed;
  }

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Write a steering file with the content and return its name
MString UTERCSRTMVA::WriteSteeringFile(const MString& Name, const MString& Content)
{
  MString FileName = GetTemporaryFileName(Name);
  if (WriteTextFile(FileName, Content) == false) {
    return "";
  }

  return FileName;
}


////////////////////////////////////////////////////////////////////////////////


//! Set the parameters with a used method, whose weights file does not exist, and return the result
bool UTERCSRTMVA::SetParametersWithoutWeights()
{
  MString FileName = WriteSteeringFile("NoWeights.tmva", "SL 2 3\nTA BDTD\nPT false\n");

  MERCSRTMVAMethods Methods;
  Methods.AddUsedMethod(MERCSRTMVAMethod::c_BDTD);

  MERCSRTMVA CSR;
  BeginCaptureOutput();
  DisableDefaultStreams();
  bool Accepted = CSR.SetParameters(FileName, Methods, &m_Geometry);
  EnableDefaultStreams();
  EndCaptureOutput();

  return Accepted;
}


////////////////////////////////////////////////////////////////////////////////


//! Set the parameters without a used method, analyze a two-site event, and return true if the event is not good
bool UTERCSRTMVA::AnalyzeWithoutMethods()
{
  MString FileName = WriteSteeringFile("NoMethods.tmva", "SL 2 3\nTA BDTD\nPT false\n");

  MERCSRTMVAMethods Methods;
  MERCSRTMVA CSR;
  BeginCaptureOutput();
  CSR.SetParameters(FileName, Methods, &m_Geometry);
  EndCaptureOutput();

  MRERawEvent* Event = CreateEvent(m_Geometry, { MVector(0.0, 0.0, 3.0), MVector(1.0, 1.0, -8.0) }, { 100.0, 400.0 });
  MRawEventIncarnations List;
  List.SetInitialRawEvent(Event);
  CSR.Analyze(&List);
  bool IsNotGood = true;
  if (Event->IsGoodEvent() == true) {
    IsNotGood = false;
  }
  List.DeleteAll();

  return IsNotGood;
}


////////////////////////////////////////////////////////////////////////////////


//! Test a missing steering file
bool UTERCSRTMVA::TestMissingSteeringFile()
{
  bool Passed = true;

  MERCSRTMVAMethods Methods;
  Methods.AddUsedMethod(MERCSRTMVAMethod::c_BDTD);

  MERCSRTMVA CSR;
  MString Missing = GetTemporaryFileName("Missing.tmva");
  DisableDefaultStreams();
  bool Accepted = CSR.SetParameters(Missing, Methods, &m_Geometry);
  EnableDefaultStreams();
  Passed = EvaluateFalse("SetParameters()", "missing steering file", "The parameters are rejected", Accepted) && Passed;

  MString Text = CSR.ToString();
  Passed = EvaluateTrue("ToString()", "missing steering file", "The title is listed", Text.Contains("# CSR - TMVA options:\n")) && Passed;
  Passed = EvaluateTrue("ToString()", "missing steering file", "The file name is listed", Text.Contains(MString("# File name:                          ") + Missing + "\n")) && Passed;
  Passed = EvaluateTrue("ToString()", "missing steering file", "The method is listed with its description", Text.Contains("# Method:                             Boosted decision trees\n")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test steering files without used methods
bool UTERCSRTMVA::TestWithoutMethods()
{
  bool Passed = true;

  // The steering file allows sequences of 2 and 3 hits, and no method is used: no weights file is read
  MString FileName = WriteSteeringFile("Steering.tmva", "# Steering file of the test\nSL 2 3\nTA BDTD,MLP\nPT false\n");
  Passed = EvaluateFalse("WriteSteeringFile()", "steering file", "The steering file can be written", FileName.IsEmpty()) && Passed;

  MERCSRTMVAMethods Methods;
  MERCSRTMVA CSR;
  BeginCaptureOutput();
  bool Accepted = CSR.SetParameters(FileName, Methods, &m_Geometry, 0.0, 0.5, 5);
  MString Output = EndCaptureOutput();
  // Defect REV-G-8: Without a used method the parameters are accepted, and the analysis reads the name of the first method from an empty list
  Passed = ExpectedToFail(EvaluateFalse("SetParameters()", "no used method", "The parameters are rejected", Accepted)) && Passed;
  Passed = ExpectedToCrash(EvaluateTrue("Analyze()", "no used method", "An event is not reconstructed without a method", AnalyzeWithoutMethods())) && Passed;
  // Defect REV-G-9: The number of interactions and readers is written to cout
  Passed = ExpectedToFail(EvaluateTrue("SetParameters()", "no used method", "Nothing is written to cout", Output.IsEmpty())) && Passed;

  MString Text = CSR.ToString();
  Passed = EvaluateTrue("ToString()", "no used method", "The file name is listed", Text.Contains(MString("# File name:                          ") + FileName + "\n")) && Passed;
  Passed = EvaluateFalse("ToString()", "no used method", "No method is listed", Text.Contains("# Method:")) && Passed;
  // The longest sequence of the steering file limits the number of interactions
  Passed = EvaluateTrue("ToString()", "5 interactions allowed, 3 in the steering file", "The maximum number of interactions is 3", Text.Contains("# MaxNInteractions:                    3\n")) && Passed;

  // The steering file does not increase the number of interactions
  BeginCaptureOutput();
  CSR.SetParameters(FileName, Methods, &m_Geometry, 0.0, 0.5, 2);
  EndCaptureOutput();
  Text = CSR.ToString();
  Passed = EvaluateTrue("ToString()", "2 interactions allowed, 3 in the steering file", "The maximum number of interactions is 2", Text.Contains("# MaxNInteractions:                    2\n")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the rejected steering files
bool UTERCSRTMVA::TestRejectedSteeringFiles()
{
  bool Passed = true;

  // The used method was not trained
  MString NotTrained = WriteSteeringFile("NotTrained.tmva", "SL 2 3\nTA BDTD\nPT false\n");
  MERCSRTMVAMethods Methods;
  Methods.AddUsedMethod(MERCSRTMVAMethod::c_MLP);
  MERCSRTMVA CSR;
  DisableDefaultStreams();
  bool Accepted = CSR.SetParameters(NotTrained, Methods, &m_Geometry);
  EnableDefaultStreams();
  Passed = EvaluateFalse("SetParameters()", "MLP used, BDTD trained", "The parameters are rejected", Accepted) && Passed;
  Passed = EvaluateTrue("SetParameters()", "MLP used, BDTD trained", "The methods of the caller are not changed", Methods.IsUsedMethod(MERCSRTMVAMethod::c_MLP)) && Passed;

  // A wrong path keyword
  MString WrongPath = WriteSteeringFile("WrongPath.tmva", "SL 2 3\nTA BDTD\nPT true false\n");
  MERCSRTMVAMethods NoMethods;
  DisableDefaultStreams();
  Accepted = CSR.SetParameters(WrongPath, NoMethods, &m_Geometry);
  EnableDefaultStreams();
  Passed = EvaluateFalse("SetParameters()", "PT with two values", "The parameters are rejected", Accepted) && Passed;

  // Defect REV-G-10: A missing weights file ends in an exception of TMVA, which nobody catches
  Passed = ExpectedToCrash(EvaluateFalse("SetParameters()", "BDTD used and trained, no weights file", "The parameters are rejected", SetParametersWithoutWeights())) && Passed;

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

  UTERCSRTMVA Test;
  return Test.Run() == true ? 0 : 1;
}


// UTERCSRTMVA.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
