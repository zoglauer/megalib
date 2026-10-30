/*
 * MSpectralyzerMain.cxx
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
// MSpectralyzerMain.cxx
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MSpectralyzerMain.h"

// Standard libs:
#include <iostream>
#include <csignal>
using namespace std;

// ROOT libs:
#include <TROOT.h>
#include <TEnv.h>
#include <TApplication.h>
#include <MString.h>
#include <TSystem.h>

// MEGAlib libs:
#include "MSpectralAnalyzer.h"
#include "MGlobal.h"


//////////////////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv)
{
  // Main function... the beginning...


  // Initialize global MEGALIB variables, especially mgui, etc.
  MGlobal::Initialize("MSpectralAnalyzer", "a spectral analyzer and isotope identifier for MEGAlib");

  /*MSpectralAnalyzer* g_Prg =*/ new MSpectralAnalyzer();

  cout<<"For the time being this is just a placeholder... Exiting..."<<endl;
  
  return 0;
}


// MSpectralyzerMain: the end...
//////////////////////////////////////////////////////////////////////////////////
