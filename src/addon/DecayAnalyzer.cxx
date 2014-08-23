/* 
 * DecayAnalyzer.cxx
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
///#include <pair>
#include <string>
#include <iomanip>
#include <algorithm>
using namespace std;

// ROOT
#include <TROOT.h>
#include <TEnv.h>
#include <TSystem.h>
#include <TApplication.h>
#include <TStyle.h>

// MEGAlib
#include "MStreams.h"
#include "MDGeometryQuest.h"
#include "MDDetector.h"
#include "MFileEventsSim.h"
#include "MDVolumeSequence.h"
#include "MSimEvent.h"
#include "MSimHT.h"

/******************************************************************************/

class EnergyPair : public pair<double, double>
{
public:
  EnergyPair(double Energy, double Rate, MString Type) { 
    m_Energy = Energy; m_Rate = Rate; m_Type = Type;
  }

  bool operator<(EnergyPair& Pair) {
    if (m_Rate > Pair.m_Rate) return true;
    return false;
  }

  double m_Energy;
  double m_Rate;
  MString m_Type;
};

/******************************************************************************/

class EnergyPairList
{
public:
  EnergyPairList() : m_Threshold(10), m_Limit(1) {}

  void SetThreshold(double Threshold) { m_Threshold = Threshold; }
  void SetLimit(int Limit) { m_Limit = Limit; }

  void Add(double Energy, MString Type) {
    if (Energy < m_Threshold) return;

    for (unsigned int i = 0; i < m_List.size(); ++i) {
      if (m_List[i].m_Energy == Energy) {
        m_List[i].m_Rate += 1;
        return;
      }
    }
    m_List.push_back(EnergyPair(Energy, 1, Type));
  }

  unsigned int NEntries() const { return m_List.size(); }

  string Dump() {

    vector<EnergyPair> List = m_List;;

    // Count entries:
    double NEntries = 0;
    for (unsigned int i = 0; i < List.size(); ++i) {
      NEntries += List[i].m_Rate;
    }
    
    // Sort:
    sort(List.begin(), List.end());

    // Dump to string
    ostringstream out;
    out.setf(ios_base::fixed, ios_base::floatfield);
    out.precision(4);
    for (unsigned int i = 0; i < List.size(); ++i) {
      if (List[i].m_Rate > m_Limit) {
        out<<"DE "<<setw(12)<<List[i].m_Energy<<"  1.5   // "
           <<setw(8)<<100.0*List[i].m_Rate/NEntries<<" % ("
           <<List[i].m_Type<<")"<<endl;
      }
    }
    return out.str();
  }

protected:
  vector<EnergyPair> m_List;
  double m_Threshold;
  int m_Limit;
};


/******************************************************************************/


class DecayAnalyzer
{
public:
  /// Default constructor
  DecayAnalyzer();
  /// Default destructor
  ~DecayAnalyzer();
  
  /// Parse the command line
  bool ParseCommandLine(int argc, char** argv);
  /// Analyze what eveer needs to be analyzed...
  bool Analyze();
  /// Interrupt the analysis
  void Interrupt() { m_Interrupt = true; }

private:
  /// True, if the analysis needs to be interrupted
  bool m_Interrupt;

  /// File name
  MString m_FileName;

  /// File name
  MString m_OutFileName;

  /// File name
  MString m_GeometryFileName;
};

/******************************************************************************/


/******************************************************************************
 * Default constructor
 */
DecayAnalyzer::DecayAnalyzer() : m_Interrupt(false)
{
  gStyle->SetPalette(1, 0);
}


/******************************************************************************
 * Default destructor
 */
DecayAnalyzer::~DecayAnalyzer()
{
  // Intentionally left blanck
}


/******************************************************************************
 * Parse the command line
 */
bool DecayAnalyzer::ParseCommandLine(int argc, char** argv)
{
  ostringstream Usage;
  Usage<<endl;
  Usage<<"  Usage: DecayAnalyzer <options>"<<endl;
  Usage<<"    General options:"<<endl;
  Usage<<"         -f:   file name"<<endl;
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
    if (Option == "-f" || Option == "-g") {
			if (!((argc > i+1) && argv[i+1][0] != '-')){
				cout<<"Error: Option "<<argv[i][1]<<" needs a second argument!"<<endl;
				cout<<Usage.str()<<endl;
				return false;
			}
		} 
		// Multiple arguments_
		//else if (Option == "-??") {
		//	if (!((argc > i+2) && argv[i+1][0] != '-' && argv[i+2][0] != '-')){
		//		cout<<"Error: Option "<<argv[i][1]<<" needs two arguments!"<<endl;
		//		cout<<Usage.str()<<endl;
		//		return false;
		//	}
		//}

		// Then fulfill the options:
    if (Option == "-f") {
      m_FileName = argv[++i];
			cout<<"Accepting file name: "<<m_FileName<<endl;
    } else if (Option == "-g") {
      m_GeometryFileName = argv[++i];
			cout<<"Accepting geometry file name: "<<m_GeometryFileName<<endl;
    } else if (Option == "--debug") {
      if (g_Verbosity < 2) g_Verbosity = 2;
			cout<<"Activating debug level"<<endl;
		} else {
			cout<<"Error: Unknown option \""<<Option<<"\"!"<<endl;
			cout<<Usage.str()<<endl;
			return false;
		}
  }

  if (m_FileName == "" || m_GeometryFileName == "") {
    cout<<"Error: You need to gve a simulation and geometry file name!"<<endl;
    cout<<Usage.str()<<endl;
    return false;   
  }

  m_OutFileName = m_FileName;
  m_OutFileName.Replace(m_FileName.Length()-4, 4, ".decay");

  return true;
}


/******************************************************************************
 * Do whatever analysis is necessary
 */
bool DecayAnalyzer::Analyze()
{
  if (m_Interrupt == true) return false;
  // Load geometry:
  MDGeometryQuest Geometry;

  if (Geometry.ScanSetupFile(m_GeometryFileName) == true) {
    cout<<"Geometry "<<Geometry.GetName()<<" loaded!"<<endl;
    Geometry.ActivateNoising(false);
    Geometry.SetGlobalFailureRate(0.0);
  } else {
    cout<<"Loading of geometry "<<Geometry.GetName()<<" failed!!"<<endl;
    return false;
  }  

  MFileEventsSim SiReader(&Geometry);
  if (SiReader.Open(m_FileName) == false) {
    cout<<"Unable to open sim file!"<<endl; 
    return false;
  }
  SiReader.ShowProgress();  

  EnergyPairList List;
  List.SetLimit(0);
  List.SetThreshold(0);

  MSimEvent* SiEvent = 0;
  while ((SiEvent = SiReader.GetNextEvent(false)) != 0) {
    if (m_Interrupt == true) return false;
    
    mdebug<<SiEvent->GetID()<<endl;

    for (unsigned int i = 0; i < SiEvent->GetNIAs(); ++i) {
      if ((SiEvent->GetIAAt(i)->GetProcess() == "BET-" || 
           SiEvent->GetIAAt(i)->GetProcess() == "BET+" || 
           SiEvent->GetIAAt(i)->GetProcess() == "ANNI" || 
           SiEvent->GetIAAt(i)->GetProcess() == "ECAP" || 
           SiEvent->GetIAAt(i)->GetProcess() == "ISML") && 
          SiEvent->GetIAAt(i)->GetSecondaryParticleID() == 1 &&
          SiEvent->IsIACompletelyAbsorbed(SiEvent->GetIAAt(i)->GetID()) == 0 &&
          SiEvent->IsIAResolved(SiEvent->GetIAAt(i)->GetID())) {
        mdebug<<"Adding: "<<SiEvent->GetIAAt(i)->GetSecondaryEnergy()<<endl;
        List.Add(SiEvent->GetIAAt(i)->GetSecondaryEnergy(), SiEvent->GetIAAt(i)->GetProcess());
      } else {
        mdebug<<"Rejecting: "<<SiEvent->GetIAAt(i)->GetSecondaryEnergy()<<endl;
        mdebug<<"  Type: "<<SiEvent->GetIAAt(i)->GetProcess()
              <<" P#: "<<SiEvent->GetIAAt(i)->GetSecondaryParticleID()
              <<" Abs: "<<int(SiEvent->IsIACompletelyAbsorbed(SiEvent->GetIAAt(i)->GetID()))
              <<" Res: "<<int(SiEvent->IsIAResolved(SiEvent->GetIAAt(i)->GetID()))<<endl;
      }
    }

    delete SiEvent;
  }

  if (List.NEntries() == 0) {
    mgui<<"No beta decays, de-exitations, annihilations, etc. found!"<<endl;
    return false;
  }

  string Text = List.Dump();
  cout<<Text<<endl;

  ofstream out;
  out.open(m_OutFileName);
  if (out.is_open() == true) {
    out<<"# Decay file"<<endl;
    out<<"#"<<endl;
    out<<"# Original file: "<<m_FileName<<endl;
    out<<"#"<<endl;
    out<<"# Important comment:"<<endl;
    out<<"# This file contains the energy of all photons produced during decays"<<endl;
    out<<"# It is highly recommended that you modify the file and keep only the most important ones!"<<endl;
    out<<"#       ======"<<endl;
    out<<"# Otherwise you reject too many good events during the decay search of the event reconstruction"<<endl;
    out<<"#"<<endl;
    out<<"# Andreas"<<endl;
    out<<"#"<<endl;
    out<<"# Format:"<<endl;
    out<<"# DE  Line  EnergyWindow (Sigma of measured (!) width)"<<endl;
    out<<endl;
    out<<"Type Decay"<<endl;
    out<<"Version 2"<<endl;
    out<<endl;
    out<<Text<<endl;
    out<<endl;
  } else {
    mout<<"Unable to open output file \""<<m_OutFileName<<"\"!"<<endl;
  }

  return true;
}


/******************************************************************************/

DecayAnalyzer* g_Prg = 0;

/******************************************************************************/


/******************************************************************************
 * Called when an interrupt signal is flagged
 * All catched signals lead to a well defined exit of the program
 */
void CatchSignal(int a)
{
  cout<<"Catched signal Ctrl-C (ID="<<a<<"):"<<endl;
  
  if (g_Prg != 0) {
    g_Prg->Interrupt();
  }
}


/******************************************************************************
 * Main program
 */
int main(int argc, char** argv)
{
  //void (*handler)(int);
	//handler = CatchSignal;
  //(void) signal(SIGINT, CatchSignal);

  // Initialize global MEGALIB variables, especially mgui, etc.
  MGlobal::Initialize();

  TApplication DecayAnalyzerApp("DecayAnalyzerApp", 0, 0);

  g_Prg = new DecayAnalyzer();

  if (g_Prg->ParseCommandLine(argc, argv) == false) {
    cerr<<"Error during parsing of command line!"<<endl;
    return -1;
  } 
  if (g_Prg->Analyze() == false) {
    cerr<<"Error during analysis!"<<endl;
    return -2;
  } 

  //DecayAnalyzerApp.Run();

  cout<<"Program exited normally!"<<endl;

  return 0;
}

/*
 * Cosima: the end...
 ******************************************************************************/
