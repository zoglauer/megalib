/*
 * MSpectralyzerMain.cxx
 *
 *
 * Copyright (C) by Michelle Galloway & Andreas Zoglauer.
 * All rights reserved.
 *
 *
 * This code implementation is the intellectual property of
 * Michelle Galloway & Andreas Zoglauer.
 *
 * By copying, distributing or modifying the Program (or any work
 * based on the Program) you indicate your acceptance of this statement,
 * and all its terms.
 *
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
