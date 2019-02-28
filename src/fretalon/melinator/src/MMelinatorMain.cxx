/*
 * MMelinatorMain.cxx
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
    cout<<"For details see: http://megalibtoolkit.com/feedback"<<endl;
    cout<<endl;
    return 1;
  }
  
  return 0;
}


// MMelinatorMain: the end...
//////////////////////////////////////////////////////////////////////////////////
