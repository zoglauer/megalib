/*
 * MResponseManipulatorMain.cxx
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
// MResponseManipulatorMain.cxx
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MResponseManipulatorMain.h"

// Standard libs:
#include <iostream>
#include <csignal>
#include <cstdlib>
using namespace std;

// ROOT libs:
#include <TROOT.h>
#include <TEnv.h>
#include <TApplication.h>
#include <MString.h>
#include <TSystem.h>

// MEGAlib libs:
#include "MResponseManipulator.h"


//////////////////////////////////////////////////////////////////////////////////


MResponseManipulator* g_Prg = 0;
int g_NInterruptCatches = 1;


////////////////////////////////////////////////////////////////////////////////


//! Called when an interrupt signal is flagged
//! All catched signals lead to a well defined exit of the program
void CatchSignal(int a)
{
  if (g_Prg != 0 && g_NInterruptCatches-- > 0) {
    cout<<"Catched signal Ctrl-C (ID="<<a<<"):"<<endl;
    g_Prg->Interrupt();
  } else {
    abort();
  }
}


//////////////////////////////////////////////////////////////////////////////////


int main(int argc, char** argv)
{
  // Main function... the beginning...

  // Catch a user interupt for graceful shutdown
  signal(SIGINT, CatchSignal);

  // Initialize global MEGALIB variables, especially mgui, etc.
  MGlobal::Initialize("Response Manipulator");


  TApplication ResponseManipulatorApp("MResponseManipulatorApp", 0, 0);

  g_Prg = new MResponseManipulator();

  if (g_Prg->ParseCommandLine(argc, argv) == false) {
    cerr<<"Error during parsing of command line!"<<endl;
    return -1;
  } 
  if (g_Prg->Analyze() == false) {
    cerr<<"Error during analysis!"<<endl;
    return -2;
  } 

  if (gROOT->GetListOfCanvases()->First() != 0) {
    ResponseManipulatorApp.Run();
  }

  return 0;
}


// MResponseManipulatorMain: the end...
//////////////////////////////////////////////////////////////////////////////////
