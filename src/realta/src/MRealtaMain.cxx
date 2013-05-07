/*
 * MRealtaMain.cxx
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
// MRealtaMain.cxx
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MRealtaMain.h"

// Standard libs:
#include <iostream>
#include <csignal>
using namespace std;

// ROOT libs:
#include <TROOT.h>
#include <TApplication.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIRealtaMain.h"


//////////////////////////////////////////////////////////////////////////////////


MGUIRealtaMain* g_Realta = 0;
int g_NInterruptCatches = 1;


//////////////////////////////////////////////////////////////////////////////////


void CatchSignal(int a)
{
  if (g_Realta != 0 && g_NInterruptCatches-- > 0) {
    cout<<"Catched signal Ctrl-C (ID="<<a<<") -- trying gracefull shutdown."<<endl;
    cout<<"If it doesn't work within a few seconds, hit Ctrl-C again for termination."<<endl;
    g_Realta->CloseWindow();
  } else {
    abort();
  }
}


//////////////////////////////////////////////////////////////////////////////////


int main(int argc, char** argv)
{
  // Main function... the beginning...

  // Setup the interrupt catcher
  signal(SIGINT, CatchSignal);

  // Initialize global MEGAlib variables, especially mgui, etc.
  MGlobal::Initialize("Realta", "real-time analysis");

  // Initialize ROOT
  TApplication* AppRealta = new TApplication("Realta", 0, 0);

  // Launch Realta GUI:
  g_Realta = new MGUIRealtaMain();
  g_Realta->DoControlLoop();

  // Start the main event loop... Is this needed since we have our own loop??
  AppRealta->Run();


  return 0;
}


// MRealtaMain: the end...
//////////////////////////////////////////////////////////////////////////////////
