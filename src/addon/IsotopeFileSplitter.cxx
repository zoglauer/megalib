/* 
 * IsotopeFileSplitter.cxx
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
#include <cstdlib>
#include <fstream>
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


////////////////////////////////////////////////////////////////////////////////


//! A standalone program based on MEGAlib and ROOT
class IsotopeFileSplitter
{
public:
  //! Default constructor
  IsotopeFileSplitter();
  //! Default destructor
  ~IsotopeFileSplitter();
  
  //! Parse the command line
  bool ParseCommandLine(int argc, char** argv);
  //! Analyze what eveer needs to be analyzed...
  bool Analyze();
  //! Interrupt the analysis
  void Interrupt() { m_Interrupt = true; }

private:
  //! True, if the analysis needs to be interrupted
  bool m_Interrupt;
  //! The file name
  MString m_FileName;
};


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
IsotopeFileSplitter::IsotopeFileSplitter() : m_Interrupt(false)
{
  gStyle->SetPalette(1, 0);
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
IsotopeFileSplitter::~IsotopeFileSplitter()
{
  // Intentionally left blank
}


////////////////////////////////////////////////////////////////////////////////


//! Parse the command line
bool IsotopeFileSplitter::ParseCommandLine(int argc, char** argv)
{
  ostringstream Usage;
  Usage<<endl;
  Usage<<"  Usage: IsotopeFileSplitter <options>"<<endl;
  Usage<<"    General options:"<<endl;
  Usage<<"         -f:   file name"<<endl;
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

    // Then fulfill the options:
    if (Option == "-f") {
      m_FileName = argv[++i];
      cout<<"Accepting file name: "<<m_FileName<<endl;
    } else {
      cout<<"Error: Unknown option \""<<Option<<"\"!"<<endl;
      cout<<Usage.str()<<endl;
      return false;
    }
  }

  if (m_FileName.IsEmpty() == true) {
    cout<<" You do need to give a file name..."<<endl;
    cout<<Usage.str()<<endl;
    return false;    
  }
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Do whatever analysis is necessary
bool IsotopeFileSplitter::Analyze()
{
  if (m_Interrupt == true) return false;
  
  ifstream in;
  in.open(m_FileName);
  if (in.is_open() == false) {
    cout<<"Unable to open the isotope file"<<endl;
    return false;
  }
  
  ofstream out;
  
  MString Time;
  MString Line;
  while (in.good() == true) {
    Line.ReadLine(in);
    
    if (Line.BeginsWith("TT") == true) {
      Time = Line;
    }
    if (Line.BeginsWith("VN") == true) {
      if (out.is_open() == true) {
        out<<"EN"<<endl;
        out.close();
      }
      
      MString Name = m_FileName;
      if (Name.EndsWith(".dat")) {
        Name = Name.GetSubString(0, Name.Length()-3);
      } else {
        Name += ".";
      }
      Name += Line.GetSubString(3, Line.Length());
      Name += ".dat";
     
      out.open(Name);
      if (out.is_open() == false) {
        cout<<"Unable to open the output file \""<<Name<<"\""<<endl;
        return false;
      }     
      
      out<<Time<<endl;
      out<<Line<<endl;
    } else {
      if (out.is_open()) {
        out<<Line<<endl;
      }
    }
    if (Line.BeginsWith("EN")) {
      out<<"EN"<<endl;
      out.close(); 
    }
  }
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


IsotopeFileSplitter* g_Prg = 0;
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

  TApplication IsotopeFileSplitterApp("IsotopeFileSplitterApp", 0, 0);

  g_Prg = new IsotopeFileSplitter();

  if (g_Prg->ParseCommandLine(argc, argv) == false) {
    cerr<<"Error during parsing of command line!"<<endl;
    return -1;
  } 
  if (g_Prg->Analyze() == false) {
    cerr<<"Error during analysis!"<<endl;
    return -2;
  } 

  //IsotopeFileSplitterApp.Run();

  cout<<"Program exited normally!"<<endl;

  return 0;
}


////////////////////////////////////////////////////////////////////////////////
