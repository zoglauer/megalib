/*
 * MUnitTest.cxx
 *
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 *
 * This code implementation is the intellectual property of
 * Andreas Zoglauer.
 *
 * By copying, distributing or modifying the Program (or any work
 * based on the Program) you indicate your acceptance of this statement,
 * and all its terms.
 *
 */


// Include the header:
#include "MUnitTest.h"

// Standard libs:
#include <fstream>
#include <sstream>
#include <string>

// ROOT libs:

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MUnitTest)
#endif


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MUnitTest::MUnitTest(const MString& Name)
{
  m_Name = Name;
  m_NumberOfPassedTests = 0;
  m_NumberOfFailedTests = 0;
  
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
MUnitTest::~MUnitTest()
{
}


////////////////////////////////////////////////////////////////////////////////


void MUnitTest::Summarize()
{
  mout<<"Unit test: "<<m_Name<<endl;
  mout<<"Passed tests: "<<m_NumberOfPassedTests<<endl;
  mout<<"Failed tests: "<<m_NumberOfFailedTests<<endl;
}


////////////////////////////////////////////////////////////////////////////////


bool MUnitTest::EvaluateFilesIdentical(MString Function, MString Input, MString Description, const MString& GeneratedFile, const MString& ReferenceFile)
{
  ifstream Generated(GeneratedFile.Data());
  if (Generated.is_open() == false) {
    RegisterFailure(Function, Input, Description,
                    MString("generated file opens: ") + GeneratedFile.Data(),
                    "cannot open generated file");
    return false;
  }

  ifstream Reference(ReferenceFile.Data());
  if (Reference.is_open() == false) {
    RegisterFailure(Function, Input, Description,
                    MString("reference file opens: ") + ReferenceFile.Data(),
                    "cannot open reference file");
    return false;
  }

  unsigned int LineNumber = 0;
  string GeneratedLine, ReferenceLine;

  while (true) {
    bool GotGenerated = (bool) getline(Generated, GeneratedLine);
    bool GotReference = (bool) getline(Reference, ReferenceLine);

    if (GotGenerated == false && GotReference == false) break;

    ++LineNumber;

    if (GotGenerated != GotReference) {
      ostringstream LengthOutput;
      if (GotGenerated == false) {
        LengthOutput << "generated file is shorter (ends at line " << LineNumber << ")";
      } else {
        LengthOutput << "generated file is longer (reference ends at line " << LineNumber << ")";
      }
      RegisterFailure(Function, Input, Description + MString(" (line count)"), "same number of lines", LengthOutput.str());
      return false;
    }

    if (GeneratedLine != ReferenceLine) {
      ostringstream Diff;
      Diff << "\n      line " << LineNumber << ":"
           << "\n        expected:  " << ReferenceLine
           << "\n        generated: " << GeneratedLine;
      RegisterFailure(Function, Input, Description, "identical files", Diff.str());
      return false;
    }
  }

  RegisterSuccess();
  return true;
}


// MUnitTest.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
