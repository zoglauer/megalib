/*
 * MRevanMain.cxx
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
// MRevanMain.cxx
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MRevanMain.h"

// // Standard libs:
#include <iostream>
using namespace std;

// // ROOT libs:
#include <TROOT.h>
#include <TApplication.h>
#include <TEnv.h>
#include <MString.h>
#include <TSystem.h>

// // MEGAlib libs:
#include "MInterfaceRevan.h"


//////////////////////////////////////////////////////////////////////////////////


int main(int argc, char** argv)
{
  // Main function... the beginning...

  // Initialize global MEGALIB variables, especially mgui, etc.
  MGlobal::Initialize("Revan", "real event analysis");

	TApplication* AppRevan = new TApplication("Revan", 0, 0);

  MInterfaceRevan Revan;
  if (Revan.ParseCommandLine(argc, argv) == false) {
    return 1;
  } else {
    AppRevan->Run();
  }

  return 0;
}


// MRevanMain: the end...
//////////////////////////////////////////////////////////////////////////////////
