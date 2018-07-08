/*
 * MSivanMain.cxx
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
// MSivanMain.cxx
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MSivanMain.h"

// // Standard libs:
#include <iostream>

// // ROOT libs:
#include <TROOT.h>
#include <TApplication.h>
#include <TEnv.h>
#include <MString.h>
#include <TSystem.h>

// // MEGAlib libs:
#include "MInterfaceSivan.h"


//////////////////////////////////////////////////////////////////////////////////


int main(int argc, char** argv)
{
  // Main function... the beginning...

  // Initialize global MEGALIB variables, especially mgui, etc.
  MGlobal::Initialize("Sivan", "simulated event analysis");

  TApplication* AppSivan = new TApplication("Sivan", 0, 0);

  MInterfaceSivan Sivan;
  if (Sivan.ParseCommandLine(argc, argv) == false) {
    return 1;
  } 
  
  if (Sivan.UseUI() == true) {
    AppSivan->Run();
  }

  return 0;
}


// MSivanMain: the end...
//////////////////////////////////////////////////////////////////////////////////
