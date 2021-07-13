/* 
 * AtmosphereEventListCreator.cxx
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

// Standard
#include <iostream>
#include <string>
#include <sstream>
#include <csignal>
#include <map>
using namespace std;

// ROOT
#include <TApplication.h>
#include <TStyle.h>
#include <TH1.h>
#include <TCanvas.h>
#include <TRandom.h>

// MEGAlib
#include "MGlobal.h"
#include "MString.h"
#include "MStreams.h"
#include "MDGeometryQuest.h"
#include "MDDetector.h"
#include "MFileEventsSim.h"
#include "MDVolumeSequence.h"
#include "MSimEvent.h"
#include "MSimHT.h"

/******************************************************************************/

class AtmosphereEventListCreator
{
public:
  //! Default constructor
  AtmosphereEventListCreator();
  //! Default destructor
  ~AtmosphereEventListCreator();
  
  //! Parse the command line
  bool ParseCommandLine(int argc, char** argv);
  //! Analyze whatever needs to be analyzed...
  bool Analyze();
  //! Interrupt the analysis
  void Interrupt() { m_Interrupt = true; }

private:
  //! True, if the analysis needs to be interrupted
  bool m_Interrupt;

  //! Simulation file name
  MString m_FileName;
  //! Geometry file name
  MString m_GeometryFileName;
  //! Surrounding sphere position
  MVector m_SSPosition;
  //! Surrounding spher radius
  double m_SSRadius;
};

/******************************************************************************/


/******************************************************************************
 * Default constructor
 */
AtmosphereEventListCreator::AtmosphereEventListCreator() : m_Interrupt(false)
{
  gStyle->SetPalette(1, 0);
  m_SSPosition = MVector(0, 0, 0);
  m_SSRadius = 60;
}


/******************************************************************************
 * Default destructor
 */
AtmosphereEventListCreator::~AtmosphereEventListCreator()
{
  // Intentionally left blank
}


/******************************************************************************
 * Parse the command line
 */
bool AtmosphereEventListCreator::ParseCommandLine(int argc, char** argv)
{
  ostringstream Usage;
  Usage<<endl;
  Usage<<"  Usage: AtmosphereEventListCreator <options>"<<endl;
  Usage<<"    General options:"<<endl;
  Usage<<"         -f:   simulation file name"<<endl;
  Usage<<"         -g:   geometry file name"<<endl;
  Usage<<"         -h:   print this help"<<endl;
  Usage<<endl;

  string Option;

  // Check for help
  for (int i = 1; i < argc; i++) {
    Option = argv[i];
    if (Option == "-h" || Option == "--help" || Option == "?" || Option == "-?") {
      cout<<Usage.str()<<endl;
      return false;
    }
  }

  // Now parse the command line options:
  for (int i = 1; i < argc; i++) {
    Option = argv[i];

    // First check if each option has sufficient arguments:
    // Single argument
    if (Option == "-f" || Option == "-o") {
      if (!((argc > i+1) && argv[i+1][0] != '-')){
        cout<<"Error: Option "<<argv[i][1]<<" needs a second argument!"<<endl;
        cout<<Usage.str()<<endl;
        return false;
      }
    } 
    // Multiple arguments_
    //else if (Option == "-??") {
    //  if (!((argc > i+2) && argv[i+1][0] != '-' && argv[i+2][0] != '-')){
    //    cout<<"Error: Option "<<argv[i][1]<<" needs two arguments!"<<endl;
    //    cout<<Usage.str()<<endl;
    //    return false;
    //  }
    //}

    // Then fulfill the options:
    if (Option == "-f") {
      m_FileName = argv[++i];
      cout<<"Accepting file name: "<<m_FileName<<endl;
    } else if (Option == "-g") {
      m_GeometryFileName = argv[++i];
      cout<<"Accepting file name: "<<m_GeometryFileName<<endl;
    } else {
      cout<<"Error: Unknown option \""<<Option<<"\"!"<<endl;
      cout<<Usage.str()<<endl;
      return false;
    }
  }

  if (m_FileName == "") {
    cout<<"Error: Need a simulation file name!"<<endl;
    cout<<Usage.str()<<endl;
    return false;
  }

  if (m_GeometryFileName == "") {
    cout<<"Error: Need a geometry file name!"<<endl;
    cout<<Usage.str()<<endl;
    return false;
  }

  if (m_FileName.EndsWith(".sim") == false && m_FileName.EndsWith(".sim.gz") == false) {
    cout<<"Error: Need a simulation file name, not a "<<m_FileName<<" file "<<endl;
    cout<<Usage.str()<<endl;
    return false;
  }

  return true;
}


/******************************************************************************
 * Do whatever analysis is necessary
 */
bool AtmosphereEventListCreator::Analyze()
{
  // Load geometry:
  MDGeometryQuest* Geometry = new MDGeometryQuest();
  if (Geometry->ScanSetupFile(m_GeometryFileName) == true) {
    cout<<"Geometry "<<Geometry->GetName()<<" loaded!"<<endl;
  } else {
    cout<<"Unable to load geometry "<<Geometry->GetName()<<" - Aborting!"<<endl;
    return false;
  }  

  MFileEventsSim* Reader = new MFileEventsSim(Geometry);
  if (Reader->Open(m_FileName) == false) {
    cout<<"Unable to open sim file "<<m_FileName<<" - Aborting!"<<endl; 
    return false;
  }
  Reader->ShowProgress();

  ofstream list;
  list.open("EventList.dat");

  MSimEvent* Event = 0;
  unsigned int ID = 0;
  while ((Event = Reader->GetNextEvent()) != 0) {
    // Hitting Ctrl-C raises this flag
    if (m_Interrupt == true) return false;

    for (unsigned int i = 0; i < Event->GetNIAs(); ++i) {
      MSimIA* IA =  Event->GetIAAt(i);
      if (IA->GetProcess() == "ENTR") {
        // We have to craete a start position according to the surrounding sphere rules:
        double x, y, z;
        
        while (true) {
          x = m_SSRadius*(2*(gRandom->Rndm()-0.5));
          y = m_SSRadius*(2*(gRandom->Rndm()-0.5));
          if (sqrt(x*x+y*y) <= m_SSRadius) break;
        }
        z = m_SSRadius;

        // Rotate according to theta and phi
        MVector Direction = IA->GetMotherDirection();
        Direction *= -1;
        double Theta = Direction.Theta();
        double Phi = Direction.Phi();
        // left-handed rotation-matrix: first theta (rotation around y-axis) then phi (rotation around z-axis):
        // | +cosp -sinp 0 |   | +cost 0 +sint |   | x |
        // | +sinp +cosp 0 | * |   0   1   0   | * | y | 
        // |   0     0   1 |   | -sint 0 +cost |   | z |
        MVector Position;
        Position[0] = (x*cos(Theta)+z*sin(Theta))*cos(Phi) - y*sin(Phi);
        Position[1] = (x*cos(Theta)+z*sin(Theta))*sin(Phi) + y*cos(Phi);
        Position[2] = -x*sin(Theta)+z*cos(Theta);
      
        // Translate sphere center
        Position += m_SSPosition;
        
        list<<++ID<<" 0  1  0  "<<Event->GetTime()<<"  "
            <<Position.X()<<"  "<<Position.Y()<<"  "<<Position.Z()<<"  "
            <<IA->GetMotherDirection().X()<<"  "<<IA->GetMotherDirection().Y()<<"  "<<IA->GetMotherDirection().Z()<<"  "
            <<IA->GetMotherPolarisation().X()<<"  "<<IA->GetMotherPolarisation().Y()<<"  "<<IA->GetMotherPolarisation().Z()<<"  "
            <<IA->GetMotherEnergy()<<endl;       
      }
    }
    
    // Never forget to delete the event
    delete Event;
  }

  list.close();
  
  // Some cleanup
  delete Reader;
  delete Geometry;
  
  return true;
}


/******************************************************************************/

AtmosphereEventListCreator* g_Prg = 0;
int g_NInterrupts = 2;

/******************************************************************************/


/******************************************************************************
 * Called when an interrupt signal is flagged
 * All catched signals lead to a well defined exit of the program
 */
void CatchSignal(int a)
{
  cout<<"Catched signal Ctrl-C:"<<endl;
  
  --g_NInterrupts;
  if (g_NInterrupts <= 0) {
    cout<<"Aborting..."<<endl;
    abort();
  } else {
    cout<<"Trying to cancel the analysis..."<<endl;
    if (g_Prg != 0) {
      g_Prg->Interrupt();
    }
    cout<<"If you hit "<<g_NInterrupts<<" more times, then I will abort immediately!"<<endl;
  }
}


/******************************************************************************
 * Main program
 */
int main(int argc, char** argv)
{
  // Set a default error handler and catch some signals...
  signal(SIGINT, CatchSignal);

  // Initialize global MEGAlib variables, especially mgui, etc.
  MGlobal::Initialize();

  TApplication AtmosphereEventListCreatorApp("AtmosphereEventListCreatorApp", 0, 0);

  g_Prg = new AtmosphereEventListCreator();

  if (g_Prg->ParseCommandLine(argc, argv) == false) {
    cerr<<"Error during parsing of command line!"<<endl;
    return -1;
  } 
  if (g_Prg->Analyze() == false) {
    cerr<<"Error during analysis!"<<endl;
    return -2;
  } 

  //AtmosphereEventListCreatorApp.Run();

  cout<<"Program exited normally!"<<endl;

  return 0;
}

/*
 * Cosima: the end...
 ******************************************************************************/
