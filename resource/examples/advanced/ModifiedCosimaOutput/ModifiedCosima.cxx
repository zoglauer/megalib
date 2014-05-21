/******************************************************************************
 *                                                                            *
 * ModifiedCosima.cc                                                          *
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
#include "TRandom.h"

// MEGAlib classes:
#include "MStreams.h"
#include "MSimEvent.h"

// Standard lib:
#include <iostream>
#include <vector>
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
  
struct Interaction {
  uint64_t ID;
  float X;
  float Y;
  float Z;
  float E;
};



/******************************************************************************
 *
 */
class Saver
{
  // public interface:
 public:
  //! Default constructor
  Saver() : m_NSimParticles(0), m_MaxBufferSize(100000), m_BufferSize(0) {
    m_Buffer = new char[sizeof(Interaction)*m_MaxBufferSize];
    ostringstream Name;
    Name<<"AirWorld_Run_"<<gRandom->Integer(8999999)+1000000<<gRandom->Integer(8999999)+1000000<<".bsim";
    m_FileName = Name.str();
    m_TmpFileName = Name.str().append("-tmp");
    
    Open();
  }
  //! Default destuctor 
  virtual ~Saver() { Close(); }
  
  bool Open() {
    m_Out.open(m_TmpFileName.c_str(), ios::binary);
    return m_Out.is_open();
  }
  
  bool Flush() {
    m_Out.write(m_Buffer, m_BufferSize*sizeof(Interaction));
    m_BufferSize = 0;
    return true;
  }
  
  bool Close() {
    Flush();
    m_Out.close();
    // Rename:
    rename(m_TmpFileName.c_str(), m_FileName.c_str());
    return true;
  }
  
  bool Add(MSimEvent* Event) {
    // Start with a list of origins
    vector<int> Origins;
    vector<Interaction> Starts;
    
    vector<Interaction> AllIAs;
    
    Interaction IAPoint;
    
    for (unsigned int i = 0; i < Event->GetNIAs(); ++i) {
      MSimIA* IA = Event->GetIAAt(i); 
      // Check if we have an initialization process        
      if (IA->GetProcess() == "INIT" || IA->GetProcess() == "ANNI") {
        if (IA->GetSecondaryParticleID() != 1) {
          merr<<"We should have a photon and not: "<<IA->GetSecondaryParticleID()<<error;   
          continue;
        } 
        
        ++m_NSimParticles;
        IAPoint.ID = m_NSimParticles;
        IAPoint.X = IA->GetPosition().X();
        IAPoint.Y = IA->GetPosition().Y();
        IAPoint.Z = IA->GetPosition().Z();
        IAPoint.E = IA->GetSecondaryEnergy();
        
        Starts.push_back(IAPoint);
        Origins.push_back(IA->GetID());
      }
    }
    
    for (unsigned int o = 0; o < Origins.size(); ++o) {
      AllIAs.push_back(Starts[o]);
        
      for (unsigned int i = 0; i < Event->GetNIAs(); ++i) {
        MSimIA* IA = Event->GetIAAt(i); 
        if (IA->GetOriginID() == Origins[o]) {
          if (IA->GetProcess() == "COMP" || IA->GetProcess() == "RAYL") {
            // We ignore fluoresence generations during Compton IA -- i.e. we only look at the first electron
            if (IA->GetProcess() == "COMP" && IA->GetSecondaryParticleID() != 3) continue;
            IAPoint.ID = Starts[o].ID;
            IAPoint.X = IA->GetPosition().X();
            IAPoint.Y = IA->GetPosition().Y();
            IAPoint.Z = IA->GetPosition().Z();
            IAPoint.E = IA->GetMotherEnergy();
            AllIAs.push_back(IAPoint);
          } else if (IA->GetProcess() == "PAIR" || IA->GetProcess() == "PHOT" || IA->GetProcess() == "ESCP" || IA->GetProcess() == "BLAK") {
            IAPoint.ID = Starts[o].ID;
            IAPoint.X = IA->GetPosition().X();
            IAPoint.Y = IA->GetPosition().Y();
            IAPoint.Z = IA->GetPosition().Z();
            IAPoint.E = 0;
            AllIAs.push_back(IAPoint);
            break;
          } else {
            merr<<"Uncover process of initial photon: "<<IA->GetProcess()<<error;           
          }
        }
      }
    }
    
    bool WasFlushed = false;
    for (unsigned int i = 0; i < AllIAs.size(); ++i) {
      if (m_BufferSize == m_MaxBufferSize) {
        Flush();
        WasFlushed = true;
      }
      memcpy(m_Buffer + sizeof(Interaction)*m_BufferSize, reinterpret_cast<char *>(&AllIAs[i]), sizeof(Interaction));
      m_BufferSize++;
    }
    // Flush again to always have full events on file
    if (WasFlushed == true) Flush();
    
    return true;
  }

  
  // private members:
 private:
  uint64_t m_NSimParticles;
  string m_TmpFileName;
  string m_FileName;
  ofstream m_Out;
  int m_MaxBufferSize; // In Interactions
  int m_BufferSize; // In Interactions
  char* m_Buffer;
  
   
};


/******************************************************************************
 * This function is just an example on how to use the relegation of a just simulated event
 */
void Relegator(MSimEvent* Event)
{
  static Saver S;
  S.Add(Event);
  return;
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
  g_Main->SetEventRelegator(Relegator);
 
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
