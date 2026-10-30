/*
 * MMimrecMain.cxx
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

  TApplication AppMimrec("Mimrec", 0, 0);

  MInterfaceMimrec Mimrec;
  if (Mimrec.ParseCommandLine(argc, argv) == false) {
    return 0;
  } 
  
  if (Mimrec.UseUI() == true) {
    AppMimrec.Run();
  }  

  return 0;
}


// MMimrecMain: the end...
//////////////////////////////////////////////////////////////////////////////////
