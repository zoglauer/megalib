/*
 * Upgrade.cxx                                             v1.0  19/11/2002
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


/******************************************************************************
 *
 * Upgrade
 *
 * Usage examples:
 *
launch Upgrade -s setupfiles/20021009_MEGA.setup -i ../../Data/Prototype/Myonen/20021118_224332_MuonMeasurement/MuonMeasurement.uevta -o ./../Data/Prototype/Myonen/20021118_224332_MuonMeasurement/MuonMeasurement.evta
 *
 ******************************************************************************/


// Standard libs:
#include <stdlib.h>
#include <vector>
using std::vector;
#include <list>
using std::list;

// ROOT libs:
#include "MString.h"
#include "TSystem.h"
#include "TH2.h"
#include "TVector3.h"
#include "TApplication.h"
#include "TCanvas.h"
#include "TFile.h"
#include "TStyle.h"
#include "TF1.h"
#include "TGClient.h"
#include "TGButton.h"
#include "TGFrame.h"

// Mega libs:
#include "MStreams.h"
#include "MDaq.h"
#include "MEventData.h"
#include "MCsIPixelDouble.h"
#include "MTime.h"
#include "MGUIDialog.h"
#include "MRawEventAnalyzer.h"

/******************************************************************************/

const int c_Unknown = 0;
const int c_Uevta = 1;
const int c_Evta = 2;
const int c_Pev = 4;

/******************************************************************************/

int GetFileType(MString Name)
{
  int Type = c_Unknown;

  Name.Remove(0, Name.Last('.')+1);
  if (Name == "uevta") {
    Type = c_Uevta;
  } else if (Name == "evta" || Name == "sim") {
    Type = c_Evta;
  } else if (Name == "pev") {
    Type = c_Pev;
  } else {
    Type = c_Uevta;
  }

  return Type;
}

/******************************************************************************/


int main(int argc, char** argv)
{
  cout<<endl;
  cout<<"Upgrade (C) by Andreas Zoglauer"<<endl;
  cout<<endl;

  MStr Usage;
  Usage<<endl;
  Usage<<"  Usage: Upgrade <options>"<<endl;
  Usage<<"         -s:   setup file name  (types: setup)"<<endl;
  Usage<<"         -i:   input file name  (types: evta, uevta or sim)"<<endl;
  Usage<<"         -o:   output file name (types: evta or pev)"<<endl;
  Usage<<endl;
  Usage<<"  The following upgrades are possible:"<<endl;
  Usage<<"         uevta  ->  evta"<<endl;
  Usage<<"         uevta  ->  pev"<<endl;
  Usage<<"         evta   ->  pev"<<endl;
  Usage<<"         sim    ->  pev"<<endl;
  Usage<<endl;

  MString SetupFileName = "";
  MString InputFileName = "";
  int InputFileType = c_Unknown;
  MString OutputFileName = "";
  int OutputFileType = c_Unknown;

  // Now parse the command line or options
  for (Int_t i = 1; i < argc; i++) {
    if (argv[i][0] == '-') {
      switch(argv[i][1]) {
      case 's':
      case 'i':
      case 'o':
        if (!((argc > i+1) && argv[i+1][0] != '-')){
          cout<<"Error: Option "<<argv[i][1]<<" needs a second argument!"<<endl;
          cout<<Usage<<endl;
          exit(1);
        }
        break;
      default:
        cout<<"Error: Unknown option "<<argv[i][1]<<endl;
        cout<<Usage<<endl;
        exit(1);
        break;
      }

      switch(argv[i][1]) {
      case 's':
        SetupFileName = argv[i+1];
        break;
      case 'i':
        InputFileName = argv[i+1];
        break;
      case 'o':
        OutputFileName = argv[i+1];
        break;
      default:
        break;
      }
    }
  }

  if (SetupFileName == "") {
    cout<<"Error: Please give a setup file"<<endl;
    cout<<Usage<<endl;
    exit(1);    
  }

  if (InputFileName == "") {
    cout<<"Error: Please give a input file"<<endl;
    cout<<Usage<<endl;
    exit(1);    
  }

  InputFileType = GetFileType(InputFileName);
  if (InputFileType == c_Unknown) {
    cout<<"Error: The input file must be of type uevta, evta or sim"<<endl;
    cout<<Usage<<endl;
    exit(1);    
  }

  if (OutputFileName == "") {
    cout<<"Error: Please give a output file"<<endl;
    cout<<Usage<<endl;
    exit(1);    
  }

  OutputFileType = GetFileType(OutputFileName);
  if (OutputFileType == c_Unknown) {
    cout<<"Error: The input file must be of type evta or pev"<<endl;
    cout<<Usage<<endl;
    exit(1);    
  }

  if (OutputFileType <= InputFileType) {
    cout<<"Error: The output file needs to be of higher order than the input file:"<<endl;
    cout<<Usage<<endl;
    exit(1);    
  }  

  // Do the actual upgrade:
  // Step: uevta --> evta
  if (InputFileType == c_Uevta) {
    MString FileName;
    if (OutputFileType == c_Evta) {
      FileName = OutputFileName;
    } else {
      FileName = InputFileName;
      FileName.Replace(FileName.Last('.')+1, FileName.Length(), "");
      FileName += "evta";
    }

    MDaq Daq;
    Daq.ResetDaq();
    Daq.Setup(SetupFileName);
    Daq.SetInputMode(MDaq::e_InputFile);
    Daq.ReadHighData(InputFileName, 6);
    Daq.WriteHighData(6, FileName, "");
    Daq.StartDaq();
    
    InputFileType = c_Evta;
    InputFileName = FileName;
  }
  
  if (InputFileType == c_Evta && OutputFileType == c_Pev) {
		cout<<"I: "<<InputFileType<<" O:"<<OutputFileType<<endl;
    TApplication App("Upgrade", 0, 0);
    MRawEventAnalyzer Analyzer(InputFileName, OutputFileName); 
    Analyzer.AnalyzeAllEvents();
  }


  return 0;
}
