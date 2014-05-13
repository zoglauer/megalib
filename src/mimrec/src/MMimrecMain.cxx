/*
 * MMimrecMain.cxx
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


////////////////////////////////////////////////////////////////////////////////
//
// MMimrecMain.cxx
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MMimrecMain.h"

// Standard libs:
#include <iostream>
using namespace std;

// ROOT libs:
#include <TROOT.h>
#include <TEnv.h>
#include <TApplication.h>
#include <MString.h>
#include <TSystem.h>

// MEGAlib libs:
#include "MInterfaceMimrec.h"


//////////////////////////////////////////////////////////////////////////////////


int main(int argc, char** argv)
{
  // Main function... the beginning...

  // Initialize global MEGAlib variables, especially mgui, etc.
  MGlobal::Initialize("Mimrec", "MEGAlib image reconstruction (and more)");

	TApplication* AppMimrec = new TApplication("Mimrec", 0, 0);

	MInterfaceMimrec Mimrec;
  if (Mimrec.ParseCommandLine(argc, argv) == false) {
    return 0;
  } else {
    AppMimrec->Run();
  }  

  return 0;
}


// MMimrecMain: the end...
//////////////////////////////////////////////////////////////////////////////////
