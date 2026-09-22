/*
 * MMelinatorMain.cxx
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
// MMelinatorMain.cxx
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MMelinatorMain.h"

// Standard libs:
#include <iostream>
#include <exception>
#include <new>
using namespace std;

// ROOT libs:
#include <TROOT.h>
#include <TEnv.h>
#include <TApplication.h>
#include <MString.h>
#include <TSystem.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MInterfaceMelinator.h"


//////////////////////////////////////////////////////////////////////////////////


//! Main function... the beginning...
int main(int argc, char** argv)
{
  // Let's try to catch all exceptions...
  try {
    // Initialize global MEGAlib variables, especially mgui, etc.
    MGlobal::Initialize();

    TApplication* AppMelinator = new TApplication("Melinator", 0, 0);

    MInterfaceMelinator Melinator;
    if (Melinator.ParseCommandLine(argc, argv) == false) {
      return 0;
    } else {
      AppMelinator->Run();
    }
  } catch (exception& e) {
    cout<<endl;
    cout<<"Unfortunately an unrecoverable error occured either in MEGAlib or ROOT..."<<endl; 
    cout<<"Perhaps the following exception text helps you to figure out what went wrong:"<<endl;
    cout<<endl;
    cout<<e.what()<<endl;
    cout<<endl;
    cout<<"You might consider filing a bug report."<<endl;
    cout<<"For details see: https://megalibtoolkit.com/feedback"<<endl;
    cout<<endl;
    return 1;
  }
  
  return 0;
}


// MMelinatorMain: the end...
//////////////////////////////////////////////////////////////////////////////////
