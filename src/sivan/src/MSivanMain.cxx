/*
 * MSivanMain.cxx
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
