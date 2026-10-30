/*
 * MResponseCreatorMain.cxx
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
// MResponseCreatorMain.cxx
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MResponseCreatorMain.h"

// Standard libs:
#include <iostream>
#include <csignal>
using namespace std;

// ROOT libs:
#include <TROOT.h>
#include <TEnv.h>
#include <TApplication.h>
#include <TSystem.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MString.h"
#include "MResponseCreator.h"



//////////////////////////////////////////////////////////////////////////////////


MResponseCreator* g_Prg = nullptr;
unsigned int g_NInterrupts = 0;


//////////////////////////////////////////////////////////////////////////////////


void CatchSignal(int a)
{
  // Called when an interrupt signal is flagged
  // All catched signals lead to a well defined exit of the program
 
  cout<<"Catched signal Ctrl-C (ID="<<a<<")"<<endl;
  
  if (g_Prg != nullptr && g_NInterrupts == 0) {
    cout<<"Press Ctrl-C again for an immediate abort!"<<endl;
    g_NInterrupts++;
    g_Prg->Interrupt();
  } else {
    abort();
  }
}


//////////////////////////////////////////////////////////////////////////////////


int main(int argc, char** argv)
{
  // Main function... the beginning...

  // Catch user interrupts
  signal(SIGINT, CatchSignal);

  // Initialize global MEGALIB variables, especially mgui, etc.
  MGlobal::Initialize("Response Creator", "a universal response generator for MEGAlib");

  g_Prg = new MResponseCreator();
  if (g_Prg->ParseCommandLine(argc, argv) == false) {
    return 1;
  } 

  return 0;
}


// MResponseCreatorMain: the end...
//////////////////////////////////////////////////////////////////////////////////
