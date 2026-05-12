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
#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>

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


int MUnitTest::RunChildProcess(const MString& Executable, const MString& Argument, const MString& OutputFileName)
{
  pid_t Child = fork();
  if (Child == 0) {
    if (OutputFileName.IsEmpty() == false) {
      int Log = open(OutputFileName.Data(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
      if (Log >= 0) {
        dup2(Log, STDOUT_FILENO);
        dup2(Log, STDERR_FILENO);
        close(Log);
      }
    }

    execl(Executable.Data(), Executable.Data(), Argument.Data(), static_cast<char*>(0));
    _exit(127);
  }

  if (Child < 0) {
    return -1;
  }

  int Status = 0;
  if (waitpid(Child, &Status, 0) < 0) {
    return -1;
  }

  return Status;
}


// MUnitTest.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
