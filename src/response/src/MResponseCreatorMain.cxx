/*
 * MResponseCreatorMain.cxx
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
