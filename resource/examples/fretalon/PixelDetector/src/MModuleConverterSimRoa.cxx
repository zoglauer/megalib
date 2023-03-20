/*
 * MModuleConverterSimRoa.cxx
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
// MModuleConverterSimRoa
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MModuleConverterSimRoa.h"

// Standard libs:

// ROOT libs:
#include "TGClient.h"

// MEGAlib libs:
#include "MReadOut.h"
#include "MReadOutSequence.h"
#include "MReadOutElementPixel2D.h"
#include "MReadOutDataADCValue.h"
#include "MGUIOptionsFileChooser.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MModuleConverterSimRoa)
#endif


////////////////////////////////////////////////////////////////////////////////


MModuleConverterSimRoa::MModuleConverterSimRoa() : MModule(), MModuleInterfaceFileName()
{
  // Construct an instance of MModuleConverterSimRoa
  
  // Set all module relevant information
  
  // Set the module name --- has to be unique
  m_Name = "Converter for Sim to Roa events";
  
  // Set the XML tag --- has to be unique --- no spaces allowed
  m_XmlTag = "XmlTagConverterSimRoa";

  // Set all modules, which have to be done before this module
  // None
  
  // Set all types this modules handles
  AddModuleType(MAssembly::c_EventLoader);
  AddModuleType(MAssembly::c_EventLoaderSimulation);
  AddModuleType(MAssembly::c_DetectorEffectsEngine);

  // Set all modules, which can follow this module
  AddSucceedingModuleType(MAssembly::c_NoRestriction);
  
  // True if this module is associated with an options GUI
  m_HasOptionsGUI = true;
  
  // This is a special start module which can generate its own events
  m_IsStartModule = true;
  
  // Allow the use of multiple threads and but not instances
  m_AllowMultiThreading = false;
  m_AllowMultipleInstances = false;
}


////////////////////////////////////////////////////////////////////////////////


MModuleConverterSimRoa::~MModuleConverterSimRoa()
{
  // Delete this instance of MModuleConverterSimRoa
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleConverterSimRoa::Initialize()
{
  // Initialize the module 
  
  // Open the simulation file - we give it the defaukt geometry which is part of all modules
  m_SimFile.SetGeometry(MModule::m_Geometry);
  if (m_SimFile.Open(m_FileName) == false) {
    cout<<"Unable to open sim file "<<m_FileName<<" - Aborting!"<<endl;
    return false;
  }
  m_SimFile.ShowProgress();

  m_AcceptedEvents = 0;
    
  return MModule::Initialize();
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleConverterSimRoa::AnalyzeEvent(MReadOutAssembly* Event)
{
  // Main data analysis routine, which updates the event to a new level:
  // Here: Just read it.

  // Read one event, if the Reader returns a nullptr, no more events are present
  MSimEvent* SimEvent = m_SimFile.GetNextEvent();
  if (SimEvent == nullptr) {
    cout<<"MModuleConverterSimRoa: No more events!"<<endl;
    m_IsFinished = true;
    return false;
  }
  
  Event->SetID(SimEvent->GetID());
  Event->SetTime(SimEvent->GetTime());

  // Loop over the hit pixels and extract the roa file information:
  for (unsigned int h = 0; h < SimEvent->GetNHTs(); ++h) {
    MSimHT* Hit = SimEvent->GetHTAt(h);

    // Convert the position to detector and pixel IDs
    MReadOutElementPixel2D ROE;
    // DetectorID: 0 is top 3 is bottom
    double Z = Hit->GetPosition().Z();
    Z += 2.0;
    int DetectorID = int(Z);
    if (DetectorID < 0 || DetectorID > 3) {
      cout<<"MModuleConverterSimRoa: Something went wrong with the Z conversion: "<<Hit->GetPosition().Z()<<" --> "<<DetectorID<<endl;
      return false;
    }
    ROE.SetDetectorID((unsigned int) DetectorID);

    // PixelID: start at negative x/y values
    double X = Hit->GetPosition().X();
    X += 3.0; //cm
    X /= 0.3; // 0.3 m strip pitch
    int XPixelID = int(X);
    if (XPixelID < 0 || XPixelID > 20) {
      cout<<"MModuleConverterSimRoa: Something went wrong with the X conversion: "<<Hit->GetPosition().X()<<" --> "<<XPixelID<<endl;
      return false;
    }
    ROE.SetXPixelID((unsigned int) XPixelID);

    double Y = Hit->GetPosition().Y();
    Y += 3.0; // cm
    Y /= 0.3; // 0.3 m strip pitch
    int YPixelID = int(Y);
    if (YPixelID < 0 || YPixelID > 20) {
      cout<<"MModuleConverterSimRoa: Something went wrong with the Y conversion: "<<Hit->GetPosition().Y()<<" --> "<<YPixelID<<endl;
      return false;
    }
    ROE.SetYPixelID((unsigned int) YPixelID);


    // Convert the energy to AD units:
    MReadOutDataADCValue ROD;
    unsigned int ADUs = (unsigned int) (10*Hit->GetEnergy());
    ROD.SetADCValue(ADUs);

    // Create a read out, and add it to the event:
    MReadOut RO(ROE, ROD);
    Event->AddReadOut(RO);
  }

  // Set the analysis progress of the event:
  Event->SetAnalysisProgress(MAssembly::c_EventLoader | MAssembly::c_EventLoaderSimulation | MAssembly::c_DetectorEffectsEngine);

  // Increase the accepted events counter
  m_AcceptedEvents++;

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MModuleConverterSimRoa::Finalize()
{
  // Initialize the module 
  
  MModule::Finalize();

  m_SimFile.Close();
}


///////////////////////////////////////////////////////////////////////////////


void MModuleConverterSimRoa::ShowOptionsGUI()
{
  //! Show the options GUI

  MGUIOptionsFileChooser* Options = new MGUIOptionsFileChooser(this);
  Options->SetDescription("Choose a sim file:");
  Options->SetFileTypes({ "*.sim", "*.sim.gz" }, { "sim files", "sim files - gzipped" });
  Options->Create();
  gClient->WaitForUnmap(Options);
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleConverterSimRoa::ReadXmlConfiguration(MXmlNode* Node)
{
  //! Read the configuration data from an XML node
  
  MXmlNode* FileNameNode = Node->GetNode("FileName");
  if (FileNameNode != 0) {
    m_FileName = FileNameNode->GetValue();
  }
 
  return true;
}


////////////////////////////////////////////////////////////////////////////////


MXmlNode* MModuleConverterSimRoa::CreateXmlConfiguration()
{
  //! Create an XML node tree from the configuration
  
  MXmlNode* Node = new MXmlNode(0, m_XmlTag);  
  new MXmlNode(Node, "FileName", m_FileName);
  
  return Node;
}


// MModuleConverterSimRoa.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
