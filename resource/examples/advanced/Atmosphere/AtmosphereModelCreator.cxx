/* 
 * AtmosphereModelCreator.cxx
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
#include <iomanip>
#include <string>
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

// MEGAlib
#include "MGlobal.h"
#include "MStreams.h"
#include "MParser.h"

////////////////////////////////////////////////////////////////////////////////


//! A standalone program based on MEGAlib and ROOT
class AtmosphereModelCreator
{
public:
  //! Default constructor
  AtmosphereModelCreator();
  //! Default destructor
  ~AtmosphereModelCreator();
  
  //! Parse the command line
  bool ParseCommandLine(int argc, char** argv);
  //! Analyze what eveer needs to be analyzed...
  bool Analyze();
  //! Interrupt the analysis
  void Interrupt() { m_Interrupt = true; }

private:
  //! True, if the analysis needs to be interrupted
  bool m_Interrupt;
  //! The input file name
  MString m_FileName;
  //! Test sphere altitude
  double m_Altitude;
  //! Compressed atmosphere
  bool m_Compressed;
};


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
AtmosphereModelCreator::AtmosphereModelCreator() : m_Interrupt(false)
{
  gStyle->SetPalette(55, 0);
  
  m_Altitude = 33.5; // km
  m_Compressed = false;
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
AtmosphereModelCreator::~AtmosphereModelCreator()
{
  // Intentionally left blank
}


////////////////////////////////////////////////////////////////////////////////


//! Parse the command line
bool AtmosphereModelCreator::ParseCommandLine(int argc, char** argv)
{
  ostringstream Usage;
  Usage<<endl;
  Usage<<"  Usage: AtmosphereModelCreator <options>"<<endl;
  Usage<<"    General options:"<<endl;
  Usage<<"         -f:   file name"<<endl;
  Usage<<"         -c:   compressed: greate a thin slab for near-field simulations, instead of the whole atmosphere"<<endl;
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
    if (Option == "-f") {
      m_FileName = argv[++i];
      cout<<"Accepting file name: "<<m_FileName<<endl;
    } else if (Option == "-c") {
      m_Compressed = true;
      cout<<"Creating compressed atmosphere."<<endl;
    } else {
      cout<<"Error: Unknown option \""<<Option<<"\"!"<<endl;
      cout<<Usage.str()<<endl;
      return false;
    }
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Do whatever analysis is necessary
bool AtmosphereModelCreator::Analyze()
{
  if (m_Interrupt == true) return false;

  MParser P;
  if (P.Open(m_FileName) == false) {
    merr<<"Unable to open file: "<<m_FileName<<endl;
    return false;
  }
  
  struct DataSet {
    double Height;
    double O;
    double N2;
    double O2;
    double Density;
    double Temperature_neutral;
    double Temperature_exospheric;
    double He;
    double Ar;
    double H;
    double N;
    double Anomalous_Oxygen;  
  };
  
  vector<DataSet> DataSets;
  
  bool Started = false;
  for (unsigned int l = 0; l < P.GetNLines(); ++l) {    
    if (P.GetLine(l).BeginsWith("1         2         3         4         5         6         7         8         9         10         11         12") == true) {
      Started = true;
      continue;
    }
    if (Started == false) continue;
    
    MTokenizer* T = P.GetTokenizerAt(l);
    if (T->GetNTokens() == 0) continue;
    if (T->GetNTokens() != 12) {
      merr<<"Incorrect number of tokens: "<<T->GetNTokens()<<endl;
      return false;
    }
    
    DataSet New;
    New.Height = T->GetTokenAtAsDouble(0);
    New.O = T->GetTokenAtAsDouble(1);
    New.N2 = T->GetTokenAtAsDouble(2);
    New.O2 = T->GetTokenAtAsDouble(3);
    New.Density = T->GetTokenAtAsDouble(4);
    New.Temperature_neutral = T->GetTokenAtAsDouble(5);
    New.Temperature_exospheric = T->GetTokenAtAsDouble(6);
    New.He = T->GetTokenAtAsDouble(7);
    New.Ar = T->GetTokenAtAsDouble(8);
    New.H = T->GetTokenAtAsDouble(9);
    New.N = T->GetTokenAtAsDouble(10);
    New.Anomalous_Oxygen = T->GetTokenAtAsDouble(11);
    DataSets.push_back(New);
  }
  
  // Now create the model
  MString ModelFileName = m_FileName;
  m_FileName.ReplaceAll(".txt", "");
  if (m_Compressed == true) m_FileName += ".compressed";
  m_FileName += ".geo";
  ofstream out;
  out.open(m_FileName);
  if (out.is_open() == false) {
    merr<<"Unable to open model file \""<<m_FileName<<"\""<<endl; 
    return false;
  }
  out<<fixed;
  
  
  unsigned int Power = 1;
  double Size = pow(2, Power) * 100000; // 1024 km
  if (Size < 102400000) Size = 102400000;
  
  out<<"# Atmosphere model created from file "<<m_FileName<<endl;
  out<<endl;
  out<<"Name AtmosphereModel"<<endl;
  out<<endl;
  out<<"# Tiny Surrounding Sphere above atomsphere @ 200 km"<<endl;
  out<<"SurroundingSphere 0.1 0 0 20000000 0.1"<<endl;
  out<<endl;
  out<<"Volume World"<<endl;
  out<<"World.Material Vacuum"<<endl;
  out<<"World.Shape BOX "<<100*Size<<" "<<100*Size<<" "<<100*Size<<endl;
  out<<"World.Visibility 1"<<endl;
  out<<"World.Position 0 0 0"<<endl;
  out<<"World.Mother 0"<<endl;
  out<<endl;
  out<<"Include $(MEGALIB)/resource/examples/geomega/materials/Materials.geo"<<endl;
  out<<endl;
  
  for (unsigned int i = 1; i < DataSets.size(); ++i) {
    double HalfHeight = 0.5 * (DataSets[i].Height - DataSets[i-1].Height);
    if (m_Compressed == false) HalfHeight *= 100000;

    if (m_Compressed == true) {
      if (HalfHeight < m_Altitude) continue;
      // HalfHeight is in cm, Altutude in km, so it fits agaian :)
    }
    
    double H = DataSets[i-1].H;
    double He = DataSets[i-1].He;
    double N = DataSets[i-1].N + 2*DataSets[i-1].N2;
    double O = DataSets[i-1].O + 2*DataSets[i-1].O2;
    double Ar = DataSets[i-1].Ar;
    
    // Normalize so that we can convert it to ints
    double Sum = H+He+N+O+Ar;
    H = 1000000*H/Sum;
    He = 1000000*He/Sum;
    N = 1000000*N/Sum;
    O = 1000000*O/Sum;
    Ar = 1000000*Ar/Sum;
    
    double Density = DataSets[i-1].Density;
    if (m_Compressed == true) Density *= 100000;
    
    MString MaterialName("MaterialSlice_");
    MaterialName += DataSets[i-1].Height;
    MaterialName += "_";
    MaterialName += DataSets[i].Height;
    
    out<<"Material "<<MaterialName<<endl;
    out<<MaterialName<<".Density "<<scientific<<Density<<fixed<<endl;
    if (int(H) > 0) out<<MaterialName<<".ComponentByAtoms H "<<int(H)<<endl;
    if (int(He) > 0) out<<MaterialName<<".ComponentByAtoms He "<<int(He)<<endl;
    if (int(N) > 0) out<<MaterialName<<".ComponentByAtoms N "<<int(N)<<endl;
    if (int(O) > 0) out<<MaterialName<<".ComponentByAtoms O "<<int(O)<<endl;
    if (int(Ar) > 0) out<<MaterialName<<".ComponentByAtoms Ar "<<int(Ar)<<endl;
    out<<endl;
    
    MString VolumeName("VolumeSlice_");
    VolumeName += DataSets[i-1].Height;
    VolumeName += "_";
    VolumeName += DataSets[i].Height;
   
    
    out<<"Volume "<<VolumeName<<endl;
    out<<VolumeName<<".Material "<<MaterialName<<endl;
    out<<VolumeName<<".Shape BOX "<<Size/2<<" "<<Size/2<<" "<<HalfHeight<<endl;
    out<<VolumeName<<".Visibility  1"<<endl;
    out<<VolumeName<<".Position 0 0 "<<HalfHeight<<endl;
    out<<VolumeName<<".Mother World"<<endl;
    out<<endl;
    
  }
  
  if (m_Compressed == false) {
    // Find the plane with the test spheres
    for (unsigned int i = 1; i < DataSets.size(); ++i) {
      if (DataSets[i-1].Height <= m_Altitude && DataSets[i].Height > m_Altitude) {
        MString VolumeName("VolumeSlice_");
        VolumeName += DataSets[i-1].Height;
        VolumeName += "_";
        VolumeName += DataSets[i].Height;
        
        MString MaterialName("MaterialSlice_");
        MaterialName += DataSets[i-1].Height;
        MaterialName += "_";
        MaterialName += DataSets[i].Height;
        for (unsigned int p = Power; p <= Power; p--) {
          double Size = 100000 * pow(2, p) - (Power - p); 
          double Height = 50000 - (Power - p); 
          out<<"Volume SphereBox_"<<p<<endl;
          out<<"SphereBox_"<<p<<".Material "<<MaterialName<<endl;
          out<<"SphereBox_"<<p<<".Shape BOX "<<0.5*Size<<" "<<0.5*Size<<" "<<Height<<endl;
          out<<"SphereBox_"<<p<<".Visibility 1"<<endl;
          if (p == Power) {
            out<<"SphereBox_"<<p<<".Mother "<<VolumeName<<endl;
          } else {
            out<<"SphereBox_"<<p<<".Copy SphereBox_pp_"<<p<<endl;
            out<<"SphereBox_pp_"<<p<<".Position "<<0.5*Size<<" "<<0.5*Size<<" 0.0"<<endl;
            out<<"SphereBox_pp_"<<p<<".Mother SphereBox_"<<p+1<<endl;
            out<<"SphereBox_"<<p<<".Copy SphereBox_pm_"<<p<<endl;
            out<<"SphereBox_pm_"<<p<<".Position "<<0.5*Size<<" "<<-0.5*Size<<" 0.0"<<endl;
            out<<"SphereBox_pm_"<<p<<".Mother SphereBox_"<<p+1<<endl;
            out<<"SphereBox_"<<p<<".Copy SphereBox_mp_"<<p<<endl;
            out<<"SphereBox_mp_"<<p<<".Position "<<-0.5*Size<<" "<<0.5*Size<<" 0.0"<<endl;
            out<<"SphereBox_mp_"<<p<<".Mother SphereBox_"<<p+1<<endl;
            out<<"SphereBox_"<<p<<".Copy SphereBox_mm_"<<p<<endl;
            out<<"SphereBox_mm_"<<p<<".Position "<<-0.5*Size<<" "<<-0.5*Size<<" 0.0"<<endl;
            out<<"SphereBox_mm_"<<p<<".Mother SphereBox_"<<p+1<<endl;
            out<<endl;
          }
        }
        
        out<<"Volume TestSphere"<<endl;
        out<<"TestSphere.Material "<<MaterialName<<endl;
        out<<"TestSphere.Shape Sphere 0 "<<50000 - Power - 1<<endl;
        out<<"TestSphere.Visibility 1"<<endl;
        out<<"TestSphere.Position 0 0 0"<<endl;
        out<<"TestSphere.Mother SphereBox_0"<<endl;
      }
    }
  }

  out<<endl;

  
  out.close();
  
  
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


AtmosphereModelCreator* g_Prg = 0;
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
  MGlobal::Initialize("Standalone", "a standalone example program");

  TApplication AtmosphereModelCreatorApp("AtmosphereModelCreatorApp", 0, 0);

  g_Prg = new AtmosphereModelCreator();

  if (g_Prg->ParseCommandLine(argc, argv) == false) {
    cerr<<"Error during parsing of command line!"<<endl;
    return -1;
  } 
  if (g_Prg->Analyze() == false) {
    cerr<<"Error during analysis!"<<endl;
    return -2;
  } 

  //AtmosphereModelCreatorApp.Run();

  cout<<"Program exited normally!"<<endl;

  return 0;
}


////////////////////////////////////////////////////////////////////////////////
