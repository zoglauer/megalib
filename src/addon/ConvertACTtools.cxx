/* 
 * ConvertACTtools.cxx
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
#include <fstream>
#include <string>
#include <sstream>
#include <csignal>
#include <iomanip>
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
#include "MFile.h"


/******************************************************************************/

class ConvertACTtools
{
public:
  /// Default constructor
  ConvertACTtools();
  /// Default destructor
  ~ConvertACTtools();
  
  /// Parse the command line
  bool ParseCommandLine(int argc, char** argv);
  /// Analyze what eveer needs to be analyzed...
  bool Analyze();
  /// Interrupt the analysis
  void Interrupt() { m_Interrupt = true; }

private:
  /// True, if the analysis needs to be interrupted
  bool m_Interrupt;
  /// The name of the original ISOU file
  MString m_ISOUFileName; 
  /// The name of the original COSU file
  MString m_COSUFileName; 
};

/******************************************************************************/


/******************************************************************************
 * Default constructor
 */
ConvertACTtools::ConvertACTtools() : m_Interrupt(false)
{
  m_ISOUFileName = g_StringNotDefined;
  m_COSUFileName = g_StringNotDefined;
}


/******************************************************************************
 * Default destructor
 */
ConvertACTtools::~ConvertACTtools()
{
  // Intentionally left blanck
}


/******************************************************************************
 * Parse the command line
 */
bool ConvertACTtools::ParseCommandLine(int argc, char** argv)
{
  ostringstream Usage;
  Usage<<endl;
  Usage<<"  Usage: ConvertACTtools <options>"<<endl;
  Usage<<"    General options:"<<endl;
  Usage<<"         -i:   ISOU (beam) file name"<<endl;
  Usage<<"         -c:   COSU (spectrum) file name"<<endl;
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
    if (Option == "-i") {
      m_ISOUFileName = argv[++i];
      cout<<"Accepting file name: "<<m_ISOUFileName<<endl;
    } else if (Option == "-c") {
      m_COSUFileName = argv[++i];
      cout<<"Accepting file name: "<<m_COSUFileName<<endl;
    } else {
      cout<<"Error: Unknown option \""<<Option<<"\"!"<<endl;
      cout<<Usage.str()<<endl;
      return false;
    }
  }

  if (m_ISOUFileName == g_StringNotDefined) {
    cout<<"You have to give an ISOU file name!"<<endl;
    cout<<Usage.str()<<endl;
    return false;    
  }

  if (m_COSUFileName == g_StringNotDefined) {
    cout<<"You have to give an COSU file name!"<<endl;
    cout<<Usage.str()<<endl;
    return false;    
  }

  if (MFile::Exists(m_ISOUFileName) == false) {
    cout<<"The ISOU file does not exist!"<<endl;
    cout<<Usage.str()<<endl;
    return false;    
  }

  if (MFile::Exists(m_COSUFileName) == false) {
    cout<<"The COSU file does not exist!"<<endl;
    cout<<Usage.str()<<endl;
    return false;    
  }

  return true;
}


/******************************************************************************
 * Do whatever analysis is necessary
 */
bool ConvertACTtools::Analyze()
{
  if (m_Interrupt == true) return false;

  // We have to perform three tasks:
  // (1) Convert the isou file
  // (2) Convert the cosu file
  // (3) Determine the correct average flux in ph/cm2/s for cosima

  vector<double> Energy;
  vector<double> EnergyIntensity;
  vector<double> Angle;
  vector<double> AngleIntensity;


  // (1) Convert the isou file

  MString iName = m_ISOUFileName;
  iName.ReplaceAll(".dat", "");
  iName += ".cosima.dat";

  ofstream ifout;
  ifout.open(iName);
  if (ifout.is_open() == false) {
    mout<<"Unable to open file "<<iName<<endl;
    return false;
  }
  ifout<<"# Converted ACTtools file: "<<m_ISOUFileName<<endl;
  ifout<<endl;
  ifout<<"IP LIN"<<endl;
  ifout<<endl;
  ifout.setf(ios_base::scientific, ios_base::floatfield);
  ifout.precision(6);

  MParser iParser;

  if (iParser.Open(m_ISOUFileName, MFile::c_Read) == false) {
    mout<<"Unable to open file "<<m_ISOUFileName<<endl;
    return false;
  }

  for (unsigned int i = 0; i < iParser.GetNLines(); ++i) {
    if (iParser.GetTokenizerAt(i)->GetNTokens() == 0) continue;
    if (iParser.GetTokenizerAt(i)->GetNTokens() != 2) {
      mout<<"Wrong number of arguments!"<<endl;
      return false;
    } else {
      ifout<<"DP "
           <<setw(10)<<iParser.GetTokenizerAt(i)->GetTokenAtAsDouble(0)<<"  "
           <<setw(10)<<iParser.GetTokenizerAt(i)->GetTokenAtAsDouble(1)<<endl;
      Angle.push_back(iParser.GetTokenizerAt(i)->GetTokenAtAsDouble(0)*c_Rad);
      AngleIntensity.push_back(iParser.GetTokenizerAt(i)->GetTokenAtAsDouble(1));
    }
  }
  ifout<<endl;
  ifout<<"EN"<<endl;
  ifout.close();
  iParser.Close();



  // (2) Convert the cosu file

  MString cName = m_COSUFileName;
  cName.ReplaceAll(".dat", "");
  cName += ".cosima.dat";

  ofstream cfout;
  cfout.open(cName);
  if (cfout.is_open() == false) {
    mout<<"Unable to open file "<<cName<<endl;
    return false;
  }
  cfout<<"# Converted ACTtools file: "<<m_COSUFileName<<endl;
  cfout<<endl;
  cfout<<"IP LIN"<<endl;
  cfout<<endl;
  cfout.setf(ios_base::scientific, ios_base::floatfield);
  cfout.precision(6);

  MParser cParser;

  if (cParser.Open(m_COSUFileName, MFile::c_Read) == false) {
    mout<<"Unable to open file "<<m_COSUFileName<<endl;
    return false;
  }

  for (unsigned int i = 0; i < cParser.GetNLines(); ++i) {
    if (cParser.GetTokenizerAt(i)->GetNTokens() == 0) continue;
    if (cParser.GetTokenizerAt(i)->GetNTokens() != 2) {
      mout<<"Wrong number of arguments!"<<endl;
      return false;
    } else {
      cfout<<"DP  "
           <<setw(10)<<cParser.GetTokenizerAt(i)->GetTokenAtAsDouble(0)*1000000<<"  "
           <<setw(10)<<cParser.GetTokenizerAt(i)->GetTokenAtAsDouble(1)/10000/1000000<<endl;
      Energy.push_back(cParser.GetTokenizerAt(i)->GetTokenAtAsDouble(0)*1000000);
      EnergyIntensity.push_back(cParser.GetTokenizerAt(i)->GetTokenAtAsDouble(1)/10000/1000000);
    }
  }
  cfout<<endl;
  cfout<<"EN"<<endl;
  cfout.close();
  cParser.Close();


  // (3) Determine the correct average flux in ph/cm2/s for cosima

  // Integrate energy:
  double EnergyIntegral = 0.0;
  for (unsigned int i = 0; i < Energy.size()-1; ++i) {
    EnergyIntegral += 0.5*(EnergyIntensity[i+1] + EnergyIntensity[i])*(Energy[i+1]-Energy[i]);
  }
  //cout<<"Flux:            "<<EnergyIntegral<<" ph/cm2/s/sr"<<endl;
    
  // Integrate angles
  double AngleIntegral = 0.0;
  for (unsigned int i = 0; i < Angle.size()-1; ++i) {
    double Area = 2*c_Pi*(cos(Angle[i]) - cos(Angle[i+1]));
    AngleIntegral += 0.5*(AngleIntensity[i+1] + AngleIntensity[i])*Area;
  }
  //cout<<"Angle integral:  "<<AngleIntegral<<endl;
  cout<<endl;
  cout<<"In cosima please set the following parameters in cosima:"<<endl;
  cout<<"Run.Beam       FarFieldFileZenithDependent "<<iName<<endl;
  cout<<"Run.Spectrum   File "<<cName<<endl;
  cout<<"Run.Flux       "<<AngleIntegral*EnergyIntegral<<endl;
  cout<<endl;
  

  return true;
}


/******************************************************************************/

ConvertACTtools* g_Prg = 0;
int g_NInterruptCatches = 1;

/******************************************************************************/


/******************************************************************************
 * Called when an interrupt signal is flagged
 * All catched signals lead to a well defined exit of the program
 */
void CatchSignal(int a)
{
  if (g_Prg != 0 && g_NInterruptCatches-- > 0) {
    cout<<"Catched signal Ctrl-C (ID="<<a<<"):"<<endl;
    g_Prg->Interrupt();
  } else {
    abort();
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

  TApplication ConvertACTtoolsApp("ConvertACTtoolsApp", 0, 0);

  g_Prg = new ConvertACTtools();

  if (g_Prg->ParseCommandLine(argc, argv) == false) {
    cerr<<"Error during parsing of command line!"<<endl;
    return -1;
  } 
  if (g_Prg->Analyze() == false) {
    cerr<<"Error during analysis!"<<endl;
    return -2;
  } 

  //ConvertACTtoolsApp.Run();

  cout<<"Program exited normally!"<<endl;

  return 0;
}

/*
 * Cosima: the end...
 ******************************************************************************/
