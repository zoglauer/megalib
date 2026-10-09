/*
 * SphereWorldSimulator.cxx
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


// Cosima classes:
#include <CLHEP/Units/SystemOfUnits.h>
#include "MCMain.hh"

// Root classes:
#include "TApplication.h"
#include "TRandom.h"
#include "TMath.h"

// MEGAlib classes:
#include "MStreams.h"
#include "MSimEvent.h"
#include "MBinnerFISBEL.h"

// Standard lib:
#include <iostream>
#include <vector>
#include <csignal>
using namespace std;

#include "Base.hxx"


/******************************************************************************/
  
MCMain* g_Main = 0;
Processor* g_Processor = 0; 
int g_NInterrupts = 3;

/******************************************************************************
 * Called when an interrupt signal is flagged
 * All catched signals lead to a well defined exit of the program
 */
void CatchSignal(int a)
{
  cout<<"Catched signal Ctrl-C (ID="<<a<<"):"<<endl;
  
  --g_NInterrupts;
  if (g_NInterrupts <= 0) {
    cout<<"Aborting..."<<endl;
    abort();
  } else {
    cout<<"Trying to cancel the run at the end of the next event..."<<endl;
    if (g_Main != 0) {
      g_Main->Interrupt();
    }
    cout<<"If you hit "<<g_NInterrupts
        <<" more times, then I will abort immediately!"<<endl;
  }
}


/******************************************************************************
 * This function is just an example on how to use the relegation of a just simulated event
 */
void Relegator(MSimEvent* Event)
{
  g_Processor->Add(Event);
  return;
}


/******************************************************************************
 * Main program
 */
int main(int argc, char** argv)
{
  // Set a default error handler and catch some signals...
  signal(SIGINT, CatchSignal);
  
  // Initialize global MEGAlib variables, especially mgui, etc.
  if (MGlobal::Initialize("Cosima", "the cosmic simulator of MEGAlib") == false) {
    return 1;
  }

  g_Main = new MCMain();
  // Load the program
  if (g_Main->Initialize(argc, argv) == false) {
    cout<<"An error occurred during initialization"<<endl;
    delete g_Main;
    return -1;
  }

  g_Processor = new Processor();
  MCParameterFile& File = g_Main->GetParameterFile();
  
  vector<MCRun>& Runs = File.GetRunList();
  vector<MCSource*>& Sources = Runs[0].GetSourceList();
  if (Sources.size() != 1) {
    cout<<"You must have exactly one source!"<<endl;
    return 1;
  }
  if (Sources[0]->GetSpectralType() != MCSource::c_Monoenergetic) {
    cout<<"You must have exactly one *MONO-ENERGETIC* source!"<<endl;
    return 1;
  }
  double Energy = Sources[0]->GetEnergyParameter(1)/CLHEP::keV;
  g_Processor->Initialize(Energy);
  
  
  // Example relegation of an event to the above relegator function
  g_Main->SetEventRelegator(Relegator);
 
  if (g_Main->Execute() == false) {
    cout<<"An error occurred during run/macro execution"<<endl;
    delete g_Main;
    return -1;
  }

  delete g_Processor;
  delete g_Main;

  return 0;
}


/*
 * Cosima: the end...
 ******************************************************************************/
