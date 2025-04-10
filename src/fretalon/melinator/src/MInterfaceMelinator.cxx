/*
* MInterfaceMelinator.cxx
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
// MInterfaceMelinator
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MInterfaceMelinator.h"

// Standard libs:
#include <iostream>
#include <sstream>
#include <vector>
using namespace std;

// ROOT libs:
#include "TROOT.h"
#include "TCanvas.h"
#include "TView.h"
#include "TGMsgBox.h"
#include "TH2.h"
#include "TCanvas.h"
#include "TApplication.h"

// MEGAlib libs:
#include "MGlobal.h"
#include "MAssert.h"
#include "MString.h"
#include "MStreams.h"
#include "MTimer.h"
#include "MGUIMainMelinator.h"
#include "MSettingsMelinator.h"
#include "MPrelude.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MInterfaceMelinator)
#endif


////////////////////////////////////////////////////////////////////////////////


MInterfaceMelinator::MInterfaceMelinator() : MInterface()
{
  // Standard constructor
  
  m_Geometry = new MDGeometryQuest();
  m_Data = new MSettingsMelinator();
  m_BasicGuiData = dynamic_cast<MSettings*>(m_Data);
}


////////////////////////////////////////////////////////////////////////////////


MInterfaceMelinator::~MInterfaceMelinator()
{
  // standard destructor
  
  // do not delete m_Geometry, because it is deleted in the base class;
  delete m_Data;
  // do not delete m_BasicGuiData, because it is just a pointer to m_Data!
}


////////////////////////////////////////////////////////////////////////////////


bool MInterfaceMelinator::ParseCommandLine(int argc, char** argv)
{
  ostringstream Usage;
  Usage<<endl;
  Usage<<"  Usage: Melinator <options>"<<endl;
  Usage<<endl;
  Usage<<"      -l"<<endl;
  Usage<<"             Load the last used calibration files"<<endl;
  Usage<<"      -p"<<endl;
  Usage<<"             Load the last used calibration files and parametrize all peaks"<<endl;
  Usage<<"      -a"<<endl;
  Usage<<"             Load the last used calibration files, parametrize all peaks, save the result, and exit"<<endl;
  Usage<<"      -e <filename>.ecal:"<<endl;
  Usage<<"             ecal file name"<<endl;
  Usage<<"      -d --detector <int>:"<<endl;
  Usage<<"             Only look at this detector"<<endl;
  Usage<<"      -s --side <int>:"<<endl;
  Usage<<"             Only look at this detector side (0: HV/negative, 1: LV/positive, else: all)"<<endl;
  Usage<<"      -t --temperature <double> <double>:"<<endl;
  Usage<<"             Only look at this temperature window: [min...max]"<<endl;
  Usage<<"      -v --verbosity <0..5>:"<<endl;
  Usage<<"             Only look at this detector"<<endl;
  Usage<<"      -c --configuration <filename>.cfg:"<<endl;
  Usage<<"             Use this file as configuration file."<<endl;
  Usage<<"             If no configuration file is give ~/.melinator.cfg is used"<<endl;
  Usage<<"      -h --help:"<<endl;
  Usage<<"             You know the answer..."<<endl;
  Usage<<endl;
  
  // Store some options temporarily:
  MString Option;
  
  // Check for help
  for (int i = 1; i < argc; i++) {
    Option = argv[i];
    if (Option == "-h" || Option == "--help" || Option == "?" || Option == "-?") {
      cout<<Usage.str()<<endl;
      return false;
    }
  }
  
  // First check if all options are ok:
  for (int i = 1; i < argc; i++) {
    Option = argv[i];
    
    // Single argument
    if (Option == "-c" || Option == "--configuration" ||
        Option == "-s" || Option == "--side" ||
        Option == "-e" || Option == "--ecal" ||
        Option == "-d" || Option == "--detector" ||
        Option == "-v" || Option == "--verbosity") {
      if (!((argc > i+1) && argv[i+1][0] != '-')){
        cout<<"Error: Option "<<argv[i][1]<<" needs a second argument!"<<endl;
        cout<<Usage.str()<<endl;
        return false;
      }
    }
  }
  
  // Now parse all very low level options
  for (int i = 1; i < argc; i++) {
    Option = argv[i];
    if (Option == "--verbosity" || Option == "-v") {
      g_Verbosity = atoi(argv[++i]);
    }
  }
  
  // Now parse all low level options
  for (int i = 1; i < argc; i++) {
    Option = argv[i];
    if (Option == "--configuration" || Option == "-c") {
      m_Data->Read(argv[++i]);
      cout<<"Command-line parser: Use configuration file "<<argv[i]<<endl;
    }
  }
  // Now parse all low level options
  for (int i = 1; i < argc; i++) {
    Option = argv[i];
    if (Option == "--ecal" || Option == "-e") {
      m_Data->SetSaveAsFileName(argv[++i]);
      cout<<"Command-line parser: Use save as file name "<<m_Data->GetSaveAsFileName()<<endl;
    } else if (Option == "--detector" || Option == "-d") {
      m_Data->SetSelectedDetectorID(atoi(argv[++i]));
      cout<<"Command-line parser: Use only this detector: "<<m_Data->GetSelectedDetectorID()<<endl;
    } else if (Option == "--side" || Option == "-s") {
      m_Data->SetSelectedDetectorSide(atoi(argv[++i]));
      cout<<"Command-line parser: Use only this detector side (0: HV/neg, 1: LV/pos, else: all): "<<m_Data->GetSelectedDetectorSide()<<endl;
    } else if (Option == "--temperature" || Option == "-t") {
      m_Data->SetMinimumTemperature(atof(argv[++i]));
      m_Data->SetMaximumTemperature(atof(argv[++i]));
      cout<<"Command-line parser: Use only this temperature window: ["<<m_Data->GetMinimumTemperature()<<", "<<m_Data->GetMaximumTemperature()<<"]"<<endl;
    } else if (Option == "--auto" || Option == "-a") {
      // Parse later
    }
  }
  
  // Now parse all high level options
  m_Gui = new MGUIMainMelinator(this, m_Data);
  m_Gui->Create();

  // Now some automatic things done after the GUI is up
  bool LoadLast = false;
  bool Parametrize = false;
  bool Automatic = false;
  for (int i = 1; i < argc; i++) {
    Option = argv[i];
    if (Option == "-l") {
      LoadLast = true;
      cout<<"Command-line parser: Automatically loading the last file(s)..."<<endl;
    } else if (Option == "-p") {
      LoadLast = true;
      Parametrize = true;
      cout<<"Command-line parser: Automatically parametrizing the peaks in the last file(s)..."<<endl;
    } else if (Option == "-a") {
      LoadLast = true;
      Parametrize = true;
      Automatic = true;
      cout<<"Command-line parser: Automatically load, parametrizing the peaks, save, and exit..."<<endl;
    }
  }
  
  if (LoadLast == true) {
    m_Gui->OnLoadLast();
  }
  if (Parametrize == true) {
    m_Gui->OnFitAll();
  }
  if (Automatic == true) {
    m_Gui->OnSave();
    Exit();
  }
  

  // Show change log / license if changed:
  MPrelude P;
  if (P.Play() == false) return false; // license was not accepted
  
  //Load();

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MInterfaceMelinator::Load()
{
  return false;
}


////////////////////////////////////////////////////////////////////////////////


bool MInterfaceMelinator::Exit()
{
  delete m_Gui;
  delete m_Data;
  delete m_Geometry;
  exit(0);
}


////////////////////////////////////////////////////////////////////////////////


bool MInterfaceMelinator::LoadConfiguration(MString FileName)
{  
  // Load the configuration file

  if (m_Data == nullptr) {
    m_Data = new MSettingsMelinator();
    m_BasicGuiData = dynamic_cast<MSettings*>(m_Data);
    //m_Gui->SetConfiguration(m_Data);
  }
  
  m_Data->Read(FileName);

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MInterfaceMelinator::SaveConfiguration(MString FileName)
{
  // Save the configuration file

  massert(m_Data != 0);
  m_Data->Write(FileName);

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MInterfaceMelinator::SetGeometry(MString FileName, bool UpdateGui)
{
  if (m_Geometry != 0) {
    delete m_Geometry;
    m_Geometry = 0;
  }

  if (FileName.EndsWith(g_StringNotDefined) == true) return false;

  // Check if the geometry exists:
  if (MFile::FileExists(FileName) == false) {
    mgui<<"The geometry file \""<<FileName<<"\" does not exist!!"<<error;
    m_BasicGuiData->SetGeometryFileName(g_StringNotDefined);
    return false;
  }

  MFile::ExpandFileName(FileName);

  m_Geometry = new MDGeometryQuest();
  if (m_Geometry->ScanSetupFile(FileName, false) == true) {
    m_BasicGuiData->SetGeometryFileName(FileName);
    mout<<"Geometry "<<m_Geometry->GetName()<<" loaded!"<<endl;
  } else {
    mgui<<"Loading of geometry \""<<FileName<<"\" failed!"<<endl;
    mgui<<"Please check the output for geometry errors and correct them!"<<error;
    delete m_Geometry;
    m_Geometry = 0;
    m_BasicGuiData->SetGeometryFileName(g_StringNotDefined);
  } 

  if (m_UseGui == true && UpdateGui == true) {
    //m_Gui->UpdateConfiguration();
  }

  return true;
}


// MInterfaceMelinator: the end...
////////////////////////////////////////////////////////////////////////////////
