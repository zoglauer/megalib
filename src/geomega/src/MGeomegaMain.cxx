/*
 * MGeomegaMain.cxx
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
// MGeomegaMain.cxx
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MGeomegaMain.h"

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
#include "MInterfaceGeomega.h"


//////////////////////////////////////////////////////////////////////////////////


int main(int argc, char** argv)
{
  // Main function... the beginning...

  // Initialize global MEGALIB variables, especially mgui, etc.
  MGlobal::Initialize("Geomega", "geometry for MEGAlib");

  TApplication* AppGeomega = new TApplication("Geomega", 0, 0);

  MInterfaceGeomega Geomega;
  if (Geomega.ParseCommandLine(argc, argv) == false) {
    return 1;
  } 
  
  if (Geomega.UseUI() == true) {
    AppGeomega->Run();
  }

  return 0; 
}


// MGeomegaMain: the end...
//////////////////////////////////////////////////////////////////////////////////
