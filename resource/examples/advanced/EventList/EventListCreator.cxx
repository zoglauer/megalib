/* 
 * EventListCreator.cxx
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
#include <fstream>
#include <sstream>
#include <csignal>
#include <cstdlib>
using namespace std;

// ROOT
#include <TROOT.h>
#include <TEnv.h>
#include <TSystem.h>
#include <TApplication.h>
#include <TStyle.h>
#include <TCanvas.h>
#include <TH1.h>
#include <TH2.h>
#include <TRandom.h>
#include <TMath.h>
#include <TVector3.h>

// MEGAlib
#include "MGlobal.h"


////////////////////////////////////////////////////////////////////////////////


//! The particle information
class Particle
{
  public:
  long m_ID;
  bool m_ConcurrentWithPrevious;
  short m_ParticleType;
  double m_ParticleExcitation;
  double m_Time;
  double m_StartPosX;
  double m_StartPosY;
  double m_StartPosZ;
  double m_DirX;
  double m_DirY;
  double m_DirZ;
  double m_PolX;
  double m_PolY;
  double m_PolZ;
  double m_Energy; 
  
  bool Stream(ofstream& out) {
    out<<m_ID<<" "<<m_ConcurrentWithPrevious<<" "<<m_ParticleType<<" "<<m_ParticleExcitation<<" "<<m_Time<<" "<<m_StartPosX<<" "<<m_StartPosY<<" "<<m_StartPosZ<<" "<<m_DirX<<" "<<m_DirY<<" "<<m_DirZ<<" "<<m_PolX<<" "<<m_PolY<<" "<<m_PolZ<<" "<<m_Energy<<endl;
    return true;
  };
};


//! A standalone program based on MEGAlib and ROOT
class EventListCreator
{
public:
  //! Default constructor
  EventListCreator();
  //! Default destructor
  ~EventListCreator();
  
  //! Parse the command line
  bool ParseCommandLine(int argc, char** argv);
  //! Create the event list
  void Create();
  //! Interrupt the analysis
  void Interrupt() { m_Interrupt = true; }

private:
  //! True, if the analysis needs to be interrupted
  bool m_Interrupt;
  //! Use random polarization instead of identical
  bool m_UseRandomPolarization;
  //! Use isotropic emission anstead of a beam
  bool m_UseIsotropicEmission;
  //! Output file name
  MString m_OutputFileName;
  //! The number of events to simulate
  long m_NEvents;
};


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
EventListCreator::EventListCreator() : m_Interrupt(false)
{
  gStyle->SetPalette(1, 0);
  
  m_UseRandomPolarization = false;
  m_UseIsotropicEmission = false;
  m_OutputFileName = "EventList.dat";
  m_NEvents = 10000000;
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
EventListCreator::~EventListCreator()
{
  // Intentionally left blank
}


////////////////////////////////////////////////////////////////////////////////


//! Parse the command line
bool EventListCreator::ParseCommandLine(int argc, char** argv)
{
  ostringstream Usage;
  Usage<<endl;
  Usage<<"  Usage: EventListCreator <options>"<<endl;
  Usage<<"    General options:"<<endl;
  Usage<<"         -o:   Output file name"<<endl;
  Usage<<"         -e:   Number of events (max: 10,000,000)"<<endl;
  Usage<<"         -r:   Use random polarization instead of identical"<<endl;
  Usage<<"         -i:   Use isotropic emission instead up an upward/downward beam"<<endl;
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
    if (Option == "-f") {
      if (!((argc > i+1) && 
            (argv[i+1][0] != '-' || isalpha(argv[i+1][1]) == 0))){
        cout<<"Error: Option "<<argv[i][1]<<" needs a second argument!"<<endl;
        cout<<Usage.str()<<endl;
        return false;
      }
    } 
    // Multiple arguments template
    /*
    else if (Option == "-??") {
      if (!((argc > i+2) && 
            (argv[i+1][0] != '-' || isalpha(argv[i+1][1]) == 0) && 
            (argv[i+2][0] != '-' || isalpha(argv[i+2][1]) == 0))){
        cout<<"Error: Option "<<argv[i][1]<<" needs two arguments!"<<endl;
        cout<<Usage.str()<<endl;
        return false;
      }
    }
    */

    // Then fulfill the options:
    if (Option == "-o") {
      m_OutputFileName = argv[++i];
      cout<<"Accepting output file name: "<<m_OutputFileName<<endl;
    } else if (Option == "-e") {
      m_NEvents = atoi(argv[++i]);
      if (m_NEvents > 10000000) {
        m_NEvents = 10000000;
        cout<<"Accepting the MAXIMUM number of events: "<<m_NEvents<<endl;
      } else {
        cout<<"Accepting this number of events: "<<m_NEvents<<endl;
      }
    } else if (Option == "-r") {
      m_UseRandomPolarization = true;
      cout<<"Accepting random polarization"<<endl;
    } else if (Option == "-i") {
      m_UseIsotropicEmission = true;
      cout<<"Accepting istropic emission"<<endl;
    } else {
      cout<<"Error: Unknown option \""<<Option<<"\"!"<<endl;
      cout<<Usage.str()<<endl;
      return false;
    }
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void EventListCreator::Create()
{
  ofstream out;
  out.open(m_OutputFileName);
  
  double Flux = 1.0;
  double Time = 0;
  for (unsigned int i = 0; i < m_NEvents; i += 2) {
    Time += gRandom->Exp(1.0/Flux);
    
    TVector3 Direction;
    
    if (m_UseIsotropicEmission == true) {
      double Theta = acos(1-2*gRandom->Rndm());
      double Phi = 2*TMath::Pi()*gRandom->Rndm();
      Direction.SetMagThetaPhi(1.0, Theta, Phi);
    } else {
      Direction.SetXYZ(0.0, 0.0, -1.0);
    }
    
    TVector3 Polarization = Direction.Orthogonal();
    Polarization.Rotate(2*TMath::Pi()*gRandom->Rndm(), Direction);
    
    Particle D1;
    D1.m_ID = i+1;
    D1.m_ConcurrentWithPrevious = false;
    D1.m_ParticleType = 1;
    D1.m_ParticleExcitation = 0;
    D1.m_Time = Time;
    D1.m_StartPosX = 0.0;
    D1.m_StartPosY = 0.0;
    D1.m_StartPosZ = 0.0;    
    D1.m_DirX = Direction[0];
    D1.m_DirY = Direction[1];
    D1.m_DirZ = Direction[2];
    D1.m_PolX = Polarization[0];
    D1.m_PolY = Polarization[1];
    D1.m_PolZ = Polarization[2];
    D1.m_Energy = 511;
    D1.Stream(out);

    if (m_UseRandomPolarization == true) {
      Polarization.Rotate(2*TMath::Pi()*gRandom->Rndm(), Direction);      
    }
    
    Particle D2;
    D2.m_ID = i+2;
    D2.m_ConcurrentWithPrevious = true;
    D2.m_ParticleType = 1;
    D2.m_ParticleExcitation = 0;
    D2.m_Time = Time;
    D2.m_StartPosX = 0.0;
    D2.m_StartPosY = 0.0;
    D2.m_StartPosZ = 0.0;    
    D2.m_DirX = -Direction[0];
    D2.m_DirY = -Direction[1];
    D2.m_DirZ = -Direction[2];
    D2.m_PolX = Polarization[0];
    D2.m_PolY = Polarization[1];
    D2.m_PolZ = Polarization[2];
    D2.m_Energy = 511;
    D2.Stream(out);
  }
  
  out.close();
}



////////////////////////////////////////////////////////////////////////////////


EventListCreator* g_Prg = 0;
int g_NInterruptCatches = 1;


////////////////////////////////////////////////////////////////////////////////


//! Called when an interrupt signal is flagged
//! All catched signals lead to a well defined exit of the program
void CatchSignal(int a)
{
  if (g_Prg != 0 && g_NInterruptCatches-- > 0) {
    cout<<"Catched signal Ctrl-C (ID="<<a<<"):"<<endl;
    g_Prg->Interrupt();
  } else {
    abort();
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Main program
int main(int argc, char** argv)
{
  // Catch a user interupt for graceful shutdown
  // signal(SIGINT, CatchSignal);

  // Initialize global MEGALIB variables, especially mgui, etc.
  MGlobal::Initialize();

  TApplication EventListCreatorApp("EventListCreatorApp", 0, 0);

  g_Prg = new EventListCreator();

  if (g_Prg->ParseCommandLine(argc, argv) == false) {
    cerr<<"Error during parsing of command line!"<<endl;
    return -1;
  } 
  g_Prg->Create();
  
  //EventListCreatorApp.Run();

  cout<<"Program exited normally!"<<endl;

  return 0;
}


////////////////////////////////////////////////////////////////////////////////
