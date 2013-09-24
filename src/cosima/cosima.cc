/******************************************************************************
 *                                                                            *
 * Cosima.cc                                                                  *
 *                                                                            *
 * Copyright (C) by Andreas Zoglauer.                                         *
 * All rights reserved.                                                       *
 *                                                                            *
 * Please see the file Licence.txt for further copyright information.         *
 *                                                                            *
 ******************************************************************************/

// Cosima classes:
#include "MCMain.hh"

// Root classes:
#include "TApplication.h"

// MEGAlib classes:
#include "MStreams.h"
#include "MSimEvent.h"

// Standard lib:
#include <iostream>
#include <csignal>
using namespace std;

/******************************************************************************/
  
MCMain* g_Main = 0;
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
void Relegator(MSimEvent* E)
{
  cout<<"Relegated event: "<<E->GetID()<<endl; 
}


/******************************************************************************
 * Main program
 */
int main(int argc, char** argv)
{
  // Set a default error handler and catch some signals...
  signal(SIGINT, CatchSignal);

  // Initialize ROOT:
  TApplication ROOT("ROOT", 0, 0);

  // Initialize global MEGAlib variables, especially mgui, etc.
  MGlobal::Initialize("Cosima", "the cosmic simulator of MEGAlib");
  __merr.SetHeader("COSIMA-ERROR:");

  g_Main = new MCMain();
  // Load the program
  if (g_Main->Initialize(argc, argv) == false) {
    cout<<"An error occurred during initialization"<<endl;
    delete g_Main;
    return -1;
  }
  
  // Example relegation of an event to the above relegator function
  //g_Main->SetEventRelegator(Relegator);
 
  if (g_Main->Execute() == false) {
    cout<<"An error occurred during run/macro execution"<<endl;
    delete g_Main;
    return -1;
  }

  delete g_Main;

  return 0;
}



/*
 * Cosima: the end...
 ******************************************************************************/
