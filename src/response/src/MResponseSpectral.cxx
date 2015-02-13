/*
 * MResponseSpectral.cxx
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
// MResponseSpectral
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MResponseSpectral.h"

// Standard libs:
#include <vector>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MAssert.h"
#include "MStreams.h"
#include "MSettingsRevan.h"
#include "MSettingsMimrec.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MResponseSpectral)
#endif


////////////////////////////////////////////////////////////////////////////////


MResponseSpectral::MResponseSpectral()
{
  // Construct an instance of MResponseSpectral
}


////////////////////////////////////////////////////////////////////////////////


MResponseSpectral::~MResponseSpectral()
{
  // Delete this instance of MResponseSpectral
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseSpectral::SetRevanConfigurationFileName(const MString FileName)
{
  // Set and verify the simulation file name

  if (MFile::Exists(FileName) == false) {
    mout<<"*** Error: \""<<FileName<<"\" does not exist"<<endl;
    return false;
  }
  m_RevanCfgFileName = FileName;

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseSpectral::OpenSimulationFile()
{
  // Load the simulation file --- has to be called after the geometry is loaded

  m_ReReader = new MRawEventAnalyzer();
  m_ReReader->SetGeometry(m_ReGeometry);
  if (m_ReReader->SetInputModeFile(m_SimulationFileName) == false) return false;

  MSettingsRevan RevanCfg(false);
  RevanCfg.Read(m_RevanCfgFileName);
  m_ReReader->SetSettings(&RevanCfg);

  if (m_ReReader->PreAnalysis() == false) return false;

  m_SiReader = new MFileEventsSim(m_SiGeometry);
  if (m_SiReader->Open(m_SimulationFileName) == false) return false;

  return true;
}

////////////////////////////////////////////////////////////////////////////////


bool MResponseSpectral::CreateResponse()
{
  // Create the multiple Compton response

  if ((m_SiGeometry = LoadGeometry(false, 0.0)) == 0) return false;
  if ((m_ReGeometry = LoadGeometry(true, 0.0)) == 0) return false;

  if (OpenSimulationFile() == false) return false;

  cout<<"Generating spectral response"<<endl;

  vector<float> AxisEnergy2;
  AxisEnergy2 = CreateLogDist(10, 20000, 1000, 1, 100000);

  MResponseMatrixO2 Energy("Energy", AxisEnergy2, AxisEnergy2);
  Energy.SetAxisNames("ideal energy [keV]", 
                      "measured energy [keV]");

  MRawEventList* REList = 0;

  int Counter = 0;
  while (InitializeNextMatchingEvent() == true) {
    REList = m_ReReader->GetRawEventList();
    MRERawEvent* RE = REList->GetInitialRawEvent();
    if (RE != 0) {
      Energy.Add(m_SiEvent->GetIAAt(0)->GetSecondaryEnergy(), RE->GetEnergy());
    }
    if (++Counter % m_SaveAfter == 0) {
      Energy.Write(m_ResponseName + ".energy" + m_Suffix, true);
    }
  }  

  Energy.Write(m_ResponseName + ".energy" + m_Suffix, true);  

  return true;
}


// MResponseSpectral.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
