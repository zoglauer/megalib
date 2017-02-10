/*
 * MResponseManipulatorMain.cxx
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


//////////////////////////////////////////////////////////////////////////////////


void CatchSignal(int a)
{
  // Called when an interrupt signal is flagged
  // All catched signals lead to a well defined exit of the program

  cout<<"Catched signal Ctrl-C (ID="<<a<<"):"<<endl;
  
  if (g_Prg != 0) {
    g_Prg->Interrupt();
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
