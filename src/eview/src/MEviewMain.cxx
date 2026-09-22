/*
 * MEviewMain.cxx
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
// MEviewMain.cxx
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MEviewMain.h"

// // Standard libs:

// // ROOT libs:
#include <TROOT.h>
#include <TEnv.h>
#include <TApplication.h>

// // MEGAlib libs:
#include "MGUIEviewMain.h"
#include "MStreams.h"


//////////////////////////////////////////////////////////////////////////////////


int main(int argc, char** argv)
{
  // Main function... the beginning...

  // Initialize global MEGALIB variables, especially mgui, etc.
  MGlobal::Initialize("Eview", "an event viewer");

  TApplication* Eview = new TApplication("Eview", 0, 0);

  // Do we have a X up and running?
  if (gClient == 0 || gClient->GetRoot() == 0) {
    cout<<"No X-client found. This is a GUI only application. Exiting."<<endl;
    return 0;
  }

  MGUIEviewMain EviewMain(gClient->GetRoot(), gClient->GetRoot());
  if (EviewMain.ParseCommandLine(argc, argv) == false) {
    return 1;
  } else {
    Eview->Run();
  }

  return 0;
}


// MEviewMain: the end...
//////////////////////////////////////////////////////////////////////////////////
