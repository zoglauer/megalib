/*
 * UTERCSRTMVAMethods.cxx
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

// MEGAlib libs:
#include "MERCSRTMVAMethods.h"
#include "MString.h"
#include "MUnitTest.h"


////////////////////////////////////////////////////////////////////////////////


//! Unit test of the list of TMVA methods MERCSRTMVAMethods
class UTERCSRTMVAMethods : public MUnitTest
{
public:
  //! Default constructor
  UTERCSRTMVAMethods() : MUnitTest("UTERCSRTMVAMethods") {}
  //! Default destructor
  virtual ~UTERCSRTMVAMethods() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Test the conversions between methods and their names
  bool TestNames();
  //! Test the list of all methods
  bool TestAllMethods();
  //! Test the used methods
  bool TestUsedMethods();
  //! Test setting the used methods from a text
  bool TestSetUsedMethods();
  //! Test copies
  bool TestCopies();
};


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTERCSRTMVAMethods::Run()
{
  bool Passed = true;

  Passed = TestNames() && Passed;
  Passed = TestAllMethods() && Passed;
  Passed = TestUsedMethods() && Passed;
  Passed = TestSetUsedMethods() && Passed;
  Passed = TestCopies() && Passed;

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the conversions between methods and their names
bool UTERCSRTMVAMethods::TestNames()
{
  bool Passed = true;

  MERCSRTMVAMethods Methods;

  vector<MERCSRTMVAMethod> All = { MERCSRTMVAMethod::c_MLP, MERCSRTMVAMethod::c_BDTD, MERCSRTMVAMethod::c_PDEFoam, MERCSRTMVAMethod::c_PDEFoamBoost, MERCSRTMVAMethod::c_DNN_CPU, MERCSRTMVAMethod::c_DNN_GPU };
  vector<MString> Names = { "MLP", "BDTD", "PDEFoam", "PDEFoamBoost", "DNN_CPU", "DNN_GPU" };
  vector<MString> FullNames = { "Multi-layer perceptron", "Boosted decision trees", "Likelihood estimator using self-adapting phase-space binning (PDE-Foam)", "Likelihood estimator using boosted self-adapting phase-space binning (PDE-Foam)", "Deep neural network on CPU", "Deep neural network on GPU" };

  for (unsigned int m = 0; m < All.size(); ++m) {
    Passed = Evaluate("GetString()", Names[m], "The method has its name", Methods.GetString(All[m]), Names[m]) && Passed;
    Passed = Evaluate("GetFullString()", Names[m], "The method has its description", Methods.GetFullString(All[m]), FullNames[m]) && Passed;
    Passed = EvaluateTrue("GetMethod()", Names[m], "The name gives the method", Methods.GetMethod(Names[m]) == All[m]) && Passed;
    MString Lower = Names[m];
    Lower.ToLowerInPlace();
    Passed = EvaluateTrue("GetMethod()", Lower, "The name in lower case gives the method", Methods.GetMethod(Lower) == All[m]) && Passed;
    Passed = Evaluate("enum MERCSRTMVAMethod", Names[m], "The methods are numbered from 1", static_cast<int>(All[m]), static_cast<int>(m + 1)) && Passed;
  }

  Passed = Evaluate("enum MERCSRTMVAMethod", "c_Unknown", "The unknown method is 0", static_cast<int>(MERCSRTMVAMethod::c_Unknown), 0) && Passed;
  Passed = Evaluate("enum MERCSRTMVAMethod", "c_Overflow", "The overflow is 7", static_cast<int>(MERCSRTMVAMethod::c_Overflow), 7) && Passed;

  Passed = Evaluate("GetString()", "c_Unknown", "The unknown method is called Unknown", Methods.GetString(MERCSRTMVAMethod::c_Unknown), MString("Unknown")) && Passed;
  Passed = Evaluate("GetString()", "c_Overflow", "The overflow is called Unknown", Methods.GetString(MERCSRTMVAMethod::c_Overflow), MString("Unknown")) && Passed;
  Passed = Evaluate("GetFullString()", "c_Unknown", "The unknown method is described as Unknown", Methods.GetFullString(MERCSRTMVAMethod::c_Unknown), MString("Unknown")) && Passed;
  Passed = Evaluate("GetFullString()", "c_Overflow", "The overflow is described as Unknown", Methods.GetFullString(MERCSRTMVAMethod::c_Overflow), MString("Unknown")) && Passed;

  Passed = EvaluateTrue("GetMethod()", "empty name", "An empty name is unknown", Methods.GetMethod("") == MERCSRTMVAMethod::c_Unknown) && Passed;
  Passed = EvaluateTrue("GetMethod()", "Unknown", "The name of the unknown method is unknown", Methods.GetMethod("Unknown") == MERCSRTMVAMethod::c_Unknown) && Passed;
  Passed = EvaluateTrue("GetMethod()", "BDT", "A part of a name is unknown", Methods.GetMethod("BDT") == MERCSRTMVAMethod::c_Unknown) && Passed;
  Passed = EvaluateTrue("GetMethod()", "MLP with blank", "A name with a blank is unknown", Methods.GetMethod(" MLP") == MERCSRTMVAMethod::c_Unknown) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the list of all methods
bool UTERCSRTMVAMethods::TestAllMethods()
{
  bool Passed = true;

  MERCSRTMVAMethods Methods;
  vector<MERCSRTMVAMethod> All = Methods.GetAllMethods();
  Passed = EvaluateSize("GetAllMethods()", "default", "There are six methods", All.size(), 6) && Passed;
  for (unsigned int m = 0; m < All.size(); ++m) {
    Passed = Evaluate("GetAllMethods()", MString("method ") + m, "The methods are in the order of their numbers", static_cast<int>(All[m]), static_cast<int>(m + 1)) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the used methods
bool UTERCSRTMVAMethods::TestUsedMethods()
{
  bool Passed = true;

  MERCSRTMVAMethods Methods;
  Passed = Evaluate("MERCSRTMVAMethods()", "default", "No method is used", Methods.GetNumberOfUsedMethods(), 0U) && Passed;
  Passed = EvaluateSize("MERCSRTMVAMethods()", "default", "The list of used methods is empty", Methods.GetUsedMethods().size(), 0) && Passed;
  Passed = Evaluate("MERCSRTMVAMethods()", "default", "The text of the used methods is empty", Methods.GetUsedMethodsString(), MString("")) && Passed;
  Passed = EvaluateFalse("IsUsedMethod()", "default", "The BDTD is not used", Methods.IsUsedMethod(MERCSRTMVAMethod::c_BDTD)) && Passed;
  Passed = EvaluateFalse("IsUsedMethod()", "c_Unknown", "The unknown method is not used", Methods.IsUsedMethod(MERCSRTMVAMethod::c_Unknown)) && Passed;
  Passed = EvaluateFalse("IsUsedMethod()", "c_Overflow", "The overflow is not used", Methods.IsUsedMethod(MERCSRTMVAMethod::c_Overflow)) && Passed;

  // The methods are listed in the order of their numbers, not in the order they were added
  Methods.AddUsedMethod(MERCSRTMVAMethod::c_DNN_CPU);
  Methods.AddUsedMethod(MERCSRTMVAMethod::c_BDTD);
  Passed = Evaluate("AddUsedMethod()", "DNN_CPU, BDTD", "Two methods are used", Methods.GetNumberOfUsedMethods(), 2U) && Passed;
  Passed = EvaluateTrue("AddUsedMethod()", "DNN_CPU, BDTD", "The BDTD is used", Methods.IsUsedMethod(MERCSRTMVAMethod::c_BDTD)) && Passed;
  Passed = EvaluateTrue("AddUsedMethod()", "DNN_CPU, BDTD", "The DNN_CPU is used", Methods.IsUsedMethod(MERCSRTMVAMethod::c_DNN_CPU)) && Passed;
  Passed = EvaluateFalse("AddUsedMethod()", "DNN_CPU, BDTD", "The MLP is not used", Methods.IsUsedMethod(MERCSRTMVAMethod::c_MLP)) && Passed;
  vector<MERCSRTMVAMethod> Used = Methods.GetUsedMethods();
  Passed = EvaluateSize("GetUsedMethods()", "DNN_CPU, BDTD", "The list has two methods", Used.size(), 2) && Passed;
  if (Used.size() == 2) {
    Passed = EvaluateTrue("GetUsedMethods()", "DNN_CPU, BDTD", "The BDTD is first", Used[0] == MERCSRTMVAMethod::c_BDTD) && Passed;
    Passed = EvaluateTrue("GetUsedMethods()", "DNN_CPU, BDTD", "The DNN_CPU is second", Used[1] == MERCSRTMVAMethod::c_DNN_CPU) && Passed;
  }
  Passed = Evaluate("GetUsedMethodsString()", "DNN_CPU, BDTD", "The text lists both methods", Methods.GetUsedMethodsString(), MString("BDTD,DNN_CPU")) && Passed;

  // Adding a method twice
  Methods.AddUsedMethod(MERCSRTMVAMethod::c_BDTD);
  Passed = Evaluate("AddUsedMethod()", "BDTD again", "Still two methods are used", Methods.GetNumberOfUsedMethods(), 2U) && Passed;

  // Removing methods
  Methods.UnsetUsedMethod(MERCSRTMVAMethod::c_BDTD);
  Passed = Evaluate("UnsetUsedMethod()", "BDTD", "One method is used", Methods.GetNumberOfUsedMethods(), 1U) && Passed;
  Passed = EvaluateFalse("UnsetUsedMethod()", "BDTD", "The BDTD is not used any more", Methods.IsUsedMethod(MERCSRTMVAMethod::c_BDTD)) && Passed;
  Passed = Evaluate("UnsetUsedMethod()", "BDTD", "The text lists the remaining method", Methods.GetUsedMethodsString(), MString("DNN_CPU")) && Passed;
  Methods.UnsetUsedMethod(MERCSRTMVAMethod::c_MLP);
  Passed = Evaluate("UnsetUsedMethod()", "a method which is not used", "One method is used", Methods.GetNumberOfUsedMethods(), 1U) && Passed;

  Methods.AddUsedMethod(MERCSRTMVAMethod::c_MLP);
  Methods.ResetUsedMethods();
  Passed = Evaluate("ResetUsedMethods()", "two methods", "No method is used", Methods.GetNumberOfUsedMethods(), 0U) && Passed;
  Passed = Evaluate("ResetUsedMethods()", "two methods", "The text of the used methods is empty", Methods.GetUsedMethodsString(), MString("")) && Passed;

  // All methods
  vector<MERCSRTMVAMethod> All = Methods.GetAllMethods();
  for (unsigned int m = 0; m < All.size(); ++m) {
    Methods.AddUsedMethod(All[m]);
  }
  Passed = Evaluate("AddUsedMethod()", "all methods", "Six methods are used", Methods.GetNumberOfUsedMethods(), 6U) && Passed;
  Passed = Evaluate("GetUsedMethodsString()", "all methods", "The text lists all methods", Methods.GetUsedMethodsString(), MString("MLP,BDTD,PDEFoam,PDEFoamBoost,DNN_CPU,DNN_GPU")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test setting the used methods from a text
bool UTERCSRTMVAMethods::TestSetUsedMethods()
{
  bool Passed = true;

  MERCSRTMVAMethods Methods;
  Passed = EvaluateTrue("SetUsedMethods()", "MLP,bdtd", "The methods are accepted", Methods.SetUsedMethods("MLP,bdtd")) && Passed;
  Passed = Evaluate("SetUsedMethods()", "MLP,bdtd", "The text lists both methods", Methods.GetUsedMethodsString(), MString("MLP,BDTD")) && Passed;

  // The used methods are not reset
  Passed = EvaluateTrue("SetUsedMethods()", "DNN_GPU", "The method is accepted", Methods.SetUsedMethods("DNN_GPU")) && Passed;
  Passed = Evaluate("SetUsedMethods()", "DNN_GPU", "The text lists all three methods", Methods.GetUsedMethodsString(), MString("MLP,BDTD,DNN_GPU")) && Passed;

  // The text of the used methods can be read back
  MERCSRTMVAMethods ReadBack;
  Passed = EvaluateTrue("SetUsedMethods()", "text of used methods", "The text of the used methods is accepted", ReadBack.SetUsedMethods(Methods.GetUsedMethodsString())) && Passed;
  Passed = Evaluate("SetUsedMethods()", "text of used methods", "The same methods are used", ReadBack.GetUsedMethodsString(), Methods.GetUsedMethodsString()) && Passed;

  // An unknown method
  MERCSRTMVAMethods Wrong;
  DisableDefaultStreams();
  bool Accepted = Wrong.SetUsedMethods("Foo");
  EnableDefaultStreams();
  Passed = EvaluateFalse("SetUsedMethods()", "Foo", "An unknown method is rejected", Accepted) && Passed;
  Passed = Evaluate("SetUsedMethods()", "Foo", "No method is used", Wrong.GetNumberOfUsedMethods(), 0U) && Passed;

  DisableDefaultStreams();
  Accepted = Wrong.SetUsedMethods("MLP BDTD");
  EnableDefaultStreams();
  Passed = EvaluateFalse("SetUsedMethods()", "MLP BDTD", "Methods separated by a blank are rejected", Accepted) && Passed;

  // No methods
  MERCSRTMVAMethods None;
  Passed = EvaluateTrue("SetUsedMethods()", "empty text", "An empty text is accepted", None.SetUsedMethods("")) && Passed;
  Passed = Evaluate("SetUsedMethods()", "empty text", "No method is used", None.GetNumberOfUsedMethods(), 0U) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test copies
bool UTERCSRTMVAMethods::TestCopies()
{
  bool Passed = true;

  MERCSRTMVAMethods Original;
  Original.AddUsedMethod(MERCSRTMVAMethod::c_PDEFoam);

  MERCSRTMVAMethods Copy(Original);
  Passed = Evaluate("MERCSRTMVAMethods()", "copy", "The copy has the used methods", Copy.GetUsedMethodsString(), MString("PDEFoam")) && Passed;
  Copy.AddUsedMethod(MERCSRTMVAMethod::c_MLP);
  Passed = Evaluate("MERCSRTMVAMethods()", "changed copy", "The original is not changed", Original.GetUsedMethodsString(), MString("PDEFoam")) && Passed;

  MERCSRTMVAMethods Assigned;
  Assigned.AddUsedMethod(MERCSRTMVAMethod::c_DNN_GPU);
  Assigned = Copy;
  Passed = Evaluate("operator=", "assignment", "The assigned object has the used methods of the source", Assigned.GetUsedMethodsString(), MString("MLP,PDEFoam")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTERCSRTMVAMethods Test;
  return Test.Run() == true ? 0 : 1;
}


// UTERCSRTMVAMethods.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
