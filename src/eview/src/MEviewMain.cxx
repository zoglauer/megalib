/*
 * MEviewMain.cxx
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
