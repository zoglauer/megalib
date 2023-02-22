/*
 * MInterfaceSivan.cxx
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
// MInterfaceSivan
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MInterfaceSivan.h"

// Standard libs:
#include <fstream>
#include <sstream>
#include <list>
#include <vector>
#include <limits>
#include <iomanip>
#include <algorithm>
using namespace std;

// ROOT libs:
#include "TFile.h"
#include "TSystem.h"
#include "TLatex.h"
#include "TPaveText.h"
#include "TPaveLabel.h"
#include "THStack.h"
#include "TLegend.h"
#include "TMath.h"
#include "TH1.h"
#include "TH2.h"
#include "TH3.h"
#include "TGeoElement.h"
#include "TMarker.h"

// MEGAlib libs:
#include "MGlobal.h"
#include "MAssert.h"
#include "MSystem.h"
#include "MGUIProgressBar.h"
#include "MSimEventLoader.h"
#include "MComptonEvent.h"
#include "MPairEvent.h"
#include "MImage3D.h"
#include "MImage2D.h"
#include "MDGeometryQuest.h"
#include "MDVolumeSequence.h"
#include "MFileEventsSim.h"
#include "MFileEventsTra.h"
#include "MStreams.h"
#include "MGUISivanMain.h"
#include "MVector.h"
#include "MFitFunctions.h"
#include "MPrelude.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MInterfaceSivan)
#endif


////////////////////////////////////////////////////////////////////////////////


MInterfaceSivan::MInterfaceSivan()
{
  // standard constructor

  m_Geometry = 0;
  m_Data = new MSettingsSivan();
  m_BasicGuiData = dynamic_cast<MSettings*>(m_Data);
}


////////////////////////////////////////////////////////////////////////////////


MInterfaceSivan::~MInterfaceSivan()
{
  // standard destructor
}


////////////////////////////////////////////////////////////////////////////////


bool MInterfaceSivan::ParseCommandLine(int argc, char** argv)
{
  ostringstream Usage;
  Usage<<endl;
  Usage<<"  Usage: sivan <options>"<<endl;
  Usage<<endl;
  Usage<<"    Basic options:"<<endl;
  Usage<<"      -g --geometry <filename>.geo.setup:"<<endl;
  Usage<<"             Use this file as geometry-file"<<endl;
  Usage<<"      -f --filename <filename>.tra:"<<endl;
  Usage<<"             This is the file which is going to be analyzed"<<endl;
  Usage<<"      -c --configuration <filename>.cfg:"<<endl;
  Usage<<"             Use this file as parameter file (uses files from -f and -g)"<<endl;
  Usage<<"             If no configuration file is give ~/.sivan.cfg is used"<<endl;
  Usage<<"      -d --debug:"<<endl;
  Usage<<"             Use debug mode"<<endl;
  Usage<<"      -h --help:"<<endl;
  Usage<<"             You know the answer..."<<endl;
  Usage<<endl;

  // Store some options temporarily:
  m_UseGui = true;

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
    if (Option == "-g" || Option == "--geometry" ||
        Option == "-c" || Option == "--configuration" ||
         Option == "-f" || Option == "--filename") {
      if (!((argc > i+1) && argv[i+1][0] != '-')){
        cout<<"Error: Option "<<argv[i][1]<<" needs a second argument!"<<endl;
        cout<<Usage.str()<<endl;
        return false;
      }
    }   
    // Double argument
//     if (Option == "-c" || Option == "--calibrate") {
//      if (!((argc > i+2) && argv[i+1][0] != '-' && argv[i+2][0] != '-')){
//        cout<<"Error: Option "<<argv[i][1]<<" needs a second argument!"<<endl;
//        cout<<Usage.str()<<endl;
//        return false;
//      }
//    }

  }
    
  // Now parse all low level options
  for (int i = 1; i < argc; i++) {
    Option = argv[i];
    if (Option == "--no-gui" || Option == "-n") {
      cout<<"Command-line parser: Do not use the gui"<<endl;
      m_UseGui = false;
      gROOT->SetBatch(true);
    } else if (Option == "--debug" || Option == "-d") {
      if (g_Verbosity < 2) g_Verbosity = 2;
      cout<<"Command-line parser: Use debug mode"<<endl;
    } else if (Option == "--configuration" || Option == "-c") {
      m_Data->Read(argv[++i]);
      cout<<"Command-line parser: Use configuration file "<<m_Data->GetSettingsFileName()<<endl;
    }
  }

  // Do we have a X up and running?
  if (gClient == 0 || gClient->GetRoot() == 0 || gROOT->IsBatch() == true) {
    cout<<"Command-line parser: No X-client or ROOT in batch mode: Using batch mode"<<endl;
    cout<<"                     If you didn't give a batch command, the program will exit."<<endl;
    m_UseGui = false;
    gROOT->SetBatch(true);
  }

  // Now parse all low level options, which overwrite data in the configuration file:
  for (int i = 1; i < argc; i++) {
    Option = argv[i];
    if (Option == "--geometry" || Option == "-g") {
      if (m_Data->SetGeometryFileName(argv[++i]) == false) {
        cout<<"Command-line parser: The geometry file could not be opened correctly!"<<endl;
        return false;
      }
      cout<<"Command-line parser: Use geometry file "<<m_Data->GetGeometryFileName()<<endl;
    } else if (Option == "--filename" || Option == "-f") {
      if (m_Data->SetCurrentFileName(argv[++i]) == false) {
        cout<<"Command-line parser: The sim file could not be opened correctly!"<<endl;
        return false;
      }
      cout<<"Command-line parser: Use file "<<m_Data->GetCurrentFileName()<<endl;
    } else if (Option == "--special") {
      m_Data->SetSpecialMode(true);
      cout<<"Command-line parser: Activating special mode - hope, you know what you are doing..."<<endl;
    }
  }

  // Load the geometry
  if (SetGeometry(m_Data->GetGeometryFileName(), false) == false) {
    cout<<"Command-line parser: "<<m_Data->GetGeometryFileName()<<" is no aceptable geometry file!"<<endl;
    cout<<"Command-line parser: Please give a correct geometry file via the -g option."<<endl;
    if (m_UseGui == true) {
      cout<<"Command-line parser: Trying to start with a dummy geometry..."<<endl;
      m_Data->SetGeometryFileName(g_MEGAlibPath + "/resource/examples/geomega/special/Dummy.geo.setup");
      if (SetGeometry(m_Data->GetGeometryFileName(), false) == false) {
        cout<<"Command-line parser: Hmmm, even reading of dummy geometry failed... Bye."<<endl;
        return false;
      }
    } else {
      return false;
    }
  }

  // Now parse all high level options, which do not invoke the GUI
  // We do not have some yet...

  if (m_UseGui == true) {
    m_Gui = new MGUISivanMain(this, m_Data);
    m_Gui->Create();
  } else {
    return false;
  }

  // Show change log / license if changed:
  MPrelude P;
  if (P.Play() == false) return false; // license was not accepted

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MInterfaceSivan::LoadConfiguration(MString FileName)
{  
  // Load the configuration file

  if (m_Data == 0) {
    m_Data = new MSettingsSivan();
    m_BasicGuiData = dynamic_cast<MSettings*>(m_Data);
    if (m_UseGui == true) {
      m_Gui->SetConfiguration(m_Data);
    }
  }
  
  m_Data->Read(FileName);

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MInterfaceSivan::SaveConfiguration(MString FileName)
{
  // Save the configuration file

  massert(m_Data != 0);

  m_Data->Write(FileName);

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MInterfaceSivan::SetGeometry(MString FileName, bool UpdateGui)
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
    m_Gui->UpdateConfiguration();
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////

  
void MInterfaceSivan::AnalyzeSimEvents(bool UseIdealEvent)
{
  // Extract the good events from the event-file:
  
  //cout<<"Temporarily deactivated noising!"<<endl;
  //m_Geometry->ActivateNoising(false);
  //m_Geometry->SetGlobalFailureRate(0);
  

  //char Text[10];
  MComptonEvent* Compton = new MComptonEvent();
  MPairEvent* Pair = new MPairEvent();
  MString Name = m_Data->GetCurrentFileName();

  // what events to take:
  int MinimumTrackLength = 2;
  double WidthPSF = 4.6;

  // Some infos about the events:
  int NEvents = 0;
  int NVetoedEvents = 0;
  int NNotVetoedEvents = 0;

  int NTrackedEvents = 0;

  int NRetrievable = 0;
  int NTriggers = 0;

  int NRComptonEvents = 0;
  int NRPairEvents = 0;
  int NRUnknownEvents = 0;

  int NRFirstIAD1 = 0;
  int NRFirstIAD2 = 0;
  int NRFirstIAD3 = 0;
  int NRFirstIADOther = 0;
  int NRFirstIANotSensitive = 0;

  int NIASD11 = 0;
  int NIASD12 = 0;
  int NIASD13 = 0;
  int NIASD21 = 0;
  int NIASD22 = 0;
  int NIASD23 = 0;
  int NIASD31 = 0;
  int NIASD32 = 0;
  int NIASD33 = 0;
  int NIASD00 = 0;

  int NRComptonD1 = 0;
  int NRPairD1 = 0;
  int NRComptonD3 = 0;
  int NRComptonD1WithTrack = 0;
  int NRComptonD1WithoutTrack = 0;
  int NRComptonD1WithTrackInARM = 0;
  int NRComptonD1WithoutTrackInARM = 0;

  int NWrittenEvents = 0;


//   int NClusteredEvents = 0;
//   int NClusteredEventsD1 = 0;
//   int NClusteredEventsD2 = 0;
//   int NClusteredEventsD3 = 0;

  int NDoubleD1 = 0;
  int NDoubleD1WithTrack = 0;
  int NTriplePlusD1 = 0;


//   double Dummy = 0;

  // Some data representants (the data will change whether it is a compton or pair event)
  MVector A, B, C;
  int TrackLength = 0;

  MSimEvent *Event = 0;

  // Open the simulation file:
  MFileEventsSim *EventFile = new MFileEventsSim(m_Geometry);
  if (EventFile->Open(m_Data->GetCurrentFileName()) == false) return;
  EventFile->ShowProgress();

  // Open the physical events file:
  Name = Name.Replace(Name.Last('.'), Name.Length(), "");
  if (UseIdealEvent == true) {
    Name += ".isiv.tra";
  } else {
    Name += ".rsiv.tra";
  }
  MFileEventsTra* PhysFile = new MFileEventsTra();
  if (PhysFile->Open(Name, MFile::c_Write) == false) {
    delete PhysFile;
    PhysFile = 0;
    return;
  }
  PhysFile->SetGeometryFileName(m_BasicGuiData->GetGeometryFileName());
  PhysFile->WriteHeader();

  bool MoreEvents = true;
  while (MoreEvents == true) {
    if (Event != 0) delete Event;
    if ((Event = EventFile->GetNextEvent()) == 0) {
      MoreEvents = false;
      break;
    }

    NEvents++;

    //cout<<"Event: "<<Event->GetID()<<endl;

    // Trigger condition: 3 separated hits:
//    if (Event->GetNTriggeredLayers() < 3) {
//      continue;
//    }
    NTriggers++;

    //cout<<"Clusters: "<<Event->GetAverageClusterSize(1)<<endl;
    
//     if (Event->IsTriggered() == true) {
//       NEvents++;
//     } else {
//       continue;
//     }
    
   
    // Determine if there is some kind of veto:
    if (Event->GetVeto() == true) {
      NVetoedEvents++;
      continue;
    } else {
      NNotVetoedEvents++;
    }
    
    // Determine if there is some kind of track: 
    if (Event->HasTrack(MinimumTrackLength) == true) {
      NTrackedEvents++;
    }

    // Determine whether the event is retrievable

    if ((UseIdealEvent == true && 
         Event->GetIEventRetrieval() == MSimEvent::IdealRetrievalGood) ||
        (UseIdealEvent == false && 
         Event->GetIEventRetrieval() == MSimEvent::IdealRetrievalGood && 
         Event->GetREventRetrieval() == MSimEvent::RealRetrievalGood)) {
      NRetrievable++;
    } else {
      continue;
    }

    // Determine the event location:    
    if (Event->GetEventType() == MSimEvent::Compton) {
      if (Event->GetEventLocation() == MSimEvent::D1) {
        NRFirstIAD1++;
      } else if (Event->GetEventLocation() == MSimEvent::D2) {
        NRFirstIAD2++;
      } else if (Event->GetEventLocation() == MSimEvent::D3) {
        NRFirstIAD3++;
      } else if (Event->GetEventLocation() == MSimEvent::D4 || Event->GetEventLocation() == MSimEvent::D5 || Event->GetEventLocation() == MSimEvent::D6 || Event->GetEventLocation() == MSimEvent::D7 || Event->GetEventLocation() == MSimEvent::D8) {
        NRFirstIADOther++;
      } else {
        NRFirstIANotSensitive++;
      }
      // ... and the interaction sequence:
      if (Event->GetICFirstIADetector() == 1 && Event->GetICSecondIADetector() == 1) {
        NIASD11++;
      } else if (Event->GetICFirstIADetector() == 1 && Event->GetICSecondIADetector() == 2) {
        NIASD12++;
      } else if (Event->GetICFirstIADetector() == 1 && Event->GetICSecondIADetector() == 3) {
        NIASD13++;
      } else if (Event->GetICFirstIADetector() == 2 && Event->GetICSecondIADetector() == 1) {
        NIASD21++;
      } else if (Event->GetICFirstIADetector() == 2 && Event->GetICSecondIADetector() == 2) {
        NIASD22++;
      } else if (Event->GetICFirstIADetector() == 2 && Event->GetICSecondIADetector() == 3) {
        NIASD23++;
      } else if (Event->GetICFirstIADetector() == 3 && Event->GetICSecondIADetector() == 1) {
        NIASD31++;
      } else if (Event->GetICFirstIADetector() == 3 && Event->GetICSecondIADetector() == 2) {
        NIASD32++;
      } else if (Event->GetICFirstIADetector() == 3 && Event->GetICSecondIADetector() == 3) {
        NIASD33++;
      } else {
        NIASD00++;        
      }
    }

    

    // Determine the event type
    if (Event->GetEventType() == MSimEvent::Compton) {
      NRComptonEvents++;

//       // Determine "D3-contained"ness
//       if (Event->GetICFirstIADetector() == 3 && Event->GetICSecondIADetector() == 3 && Event->GetICThirdIADetector() == 3) {
//        // Check if the events are well separated:
//        if ((Event->GetIAAt(1)->GetPosition() - Event->GetIAAt(2)->GetPosition()).Mag() > 5 && 
//            (Event->GetIAAt(2)->GetPosition() - Event->GetIAAt(3)->GetPosition()).Mag() > 5) {
//          NTriplePlusD1++;
//        } else {
//          continue;
//        }
//       } else {
//        continue;
//      }



      // Determine "D1-contained"ness
      if (Event->GetICFirstIADetector() == 1 && Event->GetICSecondIADetector() == 1 && Event->GetICThirdIADetector() == 1) {
        NTriplePlusD1++;
        //cout<<Event->GetSimulationData()<<endl;
      } else if (Event->GetICFirstIADetector() == 1 && Event->GetICSecondIADetector() == 1) {
        NDoubleD1++;
   
        // Check for track:
        if (Event->GetRCElectronD(MinimumTrackLength) != MVector(0.0, 0.0, 0.0)) {
          NDoubleD1WithTrack++;
        } else {
          //continue;
        }
      }

      if (Event->GetEventLocation() == MSimEvent::D1 || 
          Event->GetEventLocation() == MSimEvent::D2 ||
          Event->GetEventLocation() == MSimEvent::D3 ||
          Event->GetEventLocation() == MSimEvent::D4 ||
          Event->GetEventLocation() == MSimEvent::D7 ||
          Event->GetEventLocation() == MSimEvent::D8) {
        //if (Event->GetICFirstIADetector() != 1 || Event->GetICSecondIADetector() != 3) continue;


//         // The electron has to be contained in D1:
//         if (Event->IsElectronContainedInD1() == false) {
//           cout<<"Elektron leaves tracker!"<<endl;
//           continue;
//         }

        // Only allow events with a track or at least three IAs:
        
//         if (Event->GetRCElectronD(MinimumTrackLength) != MVector(0.0, 0.0, 0.0)) {
//           // Good...
//         } else if (Event->GetNIAs() > 3) {
//           if (Event->GetIAAt(1)->GetProcess().CompareTo("COMP") == 0 &&
//               Event->GetIAAt(2)->GetProcess().CompareTo("COMP") == 0 &&
//               (Event->GetIAAt(3)->GetProcess().CompareTo("COMP") == 0 || Event->GetIAAt(3)->GetProcess().CompareTo("PHOT")== 0)) {
//             // Good...
//           } else {
//             //cout<<Event->GetSimulationData()<<endl;
//             continue;
//           }
//         } else {
//           //cout<<Event->GetSimulationData()<<endl;
//           continue;
//         }
        

        if (Event->GetEventLocation() == MSimEvent::D1) NRComptonD1++;
        if (Event->GetEventLocation() == MSimEvent::D3) NRComptonD3++;

            
        // Check for track:
        A = Event->GetRCElectronD(MinimumTrackLength);
        
        if (A == MVector(0.0, 0.0, 0.0)) {
          TrackLength = 1;
          NRComptonD1WithoutTrack++;
          if (fabs(ComptonAngle(Event->GetRCEnergyElectron(), Event->GetRCEnergyGamma()) -
                   ComptonAngle(Event->GetICEnergyElectron(), Event->GetICEnergyGamma()))*c_Deg < WidthPSF) {
            NRComptonD1WithoutTrackInARM++;
          }
        } else {
          TrackLength = 2;
          NRComptonD1WithTrack++;
          if (fabs(ComptonAngle(Event->GetRCEnergyElectron(), Event->GetRCEnergyGamma()) -
                   ComptonAngle(Event->GetICEnergyElectron(), Event->GetICEnergyGamma()))*c_Deg < WidthPSF) {
            NRComptonD1WithTrackInARM++;
          }
        }

//         if (Event->GetRCEnergy() < 1900 || Event->GetRCEnergy() > 2100) continue;

//         Compton->SetDe(A);
          
//         //Compton->SetC1(Event->GetRCFirstIAClustered());
//         Compton->SetC1(Event->GetICFirstIA());
        
//         //Compton->SetEe(Event->GetRCEnergyElectron());
//         Compton->SetEe(Event->GetICEnergyElectron());
        
//         //Compton->SetC2(Event->GetRCSecondIAClustered());
//         Compton->SetC2(Event->GetICSecondIA());
        
//         //Compton->SetEg(Event->GetRCEnergyGamma());
//         Compton->SetEg(Event->GetICEnergyGamma());
        
           
//         Compton->SetLeverArm((Event->GetICFirstIA() - Event->GetICSecondIA()).Mag()); // Wrong one!
//         mimp<<"Lever arm is wrong implemented!"<<show;

        
        if (UseIdealEvent == false) {
          Compton->SetDe(A);
          
          //Compton->SetC1(Event->GetRCFirstIA());
          Compton->SetC1(Event->GetRCFirstIAClustered());
          Compton->SetEe(Event->GetRCEnergyElectron());
          
          //Compton->SetC2(Event->GetRCSecondIA());
          Compton->SetC2(Event->GetRCSecondIAClustered());
          Compton->SetEg(Event->GetRCEnergyGamma());

          if (Compton->IsKinematicsOK() == false) {
            continue;
          }
        } else {
          // Possible ideal data
          if (TrackLength > 1) {
            Compton->SetDe(Event->GetICElectronD());
          } else {
            Compton->SetDe(MVector(0, 0, 0));
          }

          Compton->SetC1(Event->GetICFirstIA());
          Compton->SetEe(Event->GetICEnergyElectron());
          
          Compton->SetC2(Event->GetICSecondIA());
          Compton->SetEg(Event->GetICEnergyGamma());
          Compton->SetLeverArm((Event->GetICFirstIA() - Event->GetICSecondIA()).Mag()); // Wrong one!
          mimp<<"Lever arm is wrong implemented!"<<show;
        }        

        Compton->Set(*dynamic_cast<MRotationInterface*>(Event));        
        Compton->SetSequenceLength(2);
        Compton->SetId(Event->GetID());
        Compton->SetTime(Event->GetTime());
        Compton->SetTrackLength(TrackLength);
        mimp<<"Track length is wrong implemented!"<<endl;

        NWrittenEvents++;
        PhysFile->AddEvent((MPhysicalEvent *) Compton);
      }
    } else if (Event->GetEventType() == MSimEvent::Pair) {
      //cout<<"We have a pair!"<<endl;
      NRPairEvents++;

      if (Event->GetEventLocation() == MSimEvent::D1) {
        NRFirstIAD1++;
      } else if (Event->GetEventLocation() == MSimEvent::D2) {
        NRFirstIAD2++;
      } else if (Event->GetEventLocation() == MSimEvent::D3) {
        NRFirstIAD3++;
      } else if (Event->GetEventLocation() == MSimEvent::D4 || Event->GetEventLocation() == MSimEvent::D5 || Event->GetEventLocation() == MSimEvent::D6 || Event->GetEventLocation() == MSimEvent::D7 || Event->GetEventLocation() == MSimEvent::D8) {
        NRFirstIADOther++;
      } else {
        NRFirstIANotSensitive++;
      }

      if (Event->GetEventLocation() == MSimEvent::D1) { 
        NRPairD1++;
        
        // Now write the event:
        if (UseIdealEvent == false) {
          Pair->SetId(Event->GetID());
          Pair->SetTime(Event->GetTime());
          Pair->SetPairCreationIA(Event->GetRPIA());
          Pair->SetElectronDirection(Event->GetRPElectronD());
          Pair->SetPositronDirection(Event->GetRPPositronD());
          Pair->SetEnergyElectron(Event->GetRPEnergyElectron());
          Pair->SetEnergyPositron(Event->GetRPEnergyPositron());
          Pair->SetInitialEnergyDeposit(Event->GetRPInitialEnergyDeposit());
          Pair->Set(*dynamic_cast<MRotationInterface*>(Event));
          PhysFile->AddEvent((MPhysicalEvent *) Pair);
        } else {
          Pair->SetId(Event->GetID());
          Pair->SetTime(Event->GetTime());
          Pair->SetPairCreationIA(Event->GetIPVertex());
          Pair->SetElectronDirection(Event->GetIPElectronDir());
          Pair->SetPositronDirection(Event->GetIPPositronDir());
          Pair->SetEnergyElectron(Event->GetIPElectronEnergy());
          Pair->SetEnergyPositron(Event->GetIPPositronEnergy());
          mimp<<"Ideal pair initial deposit is wrong!"<<endl;
          Pair->SetInitialEnergyDeposit(100.0);
          Pair->Set(*dynamic_cast<MRotationInterface*>(Event));
          PhysFile->AddEvent((MPhysicalEvent *) Pair);
        }
        //PhysFile->AddEvent((MPhysicalEvent *) Pair);
      }
    } else {
      NRUnknownEvents++;
      //cout<<"Not Compton - not pair..."<<endl;
    }
  }


  //cout<<"Average deviation: "<<sqrt(1/NWrittenEvents*Dummy)<<"!"<<Dummy<<endl;

  //Pev->Close();
  //fout.close();

  //delete Progress;

  cout<<endl;
  cout<<"Event-statistics (theory)"<<endl;
  cout<<"========================="<<endl<<endl;
  cout<<"Total number of simulated events             : "<<NEvents<<endl;
  cout<<"Total number of triggered events             : "<<NTriggers<<endl;
  cout<<endl;
  cout<<"Total number of NOT vetoed events:           : "<<NNotVetoedEvents<<" (vetoed: "<<NVetoedEvents<<")"<<endl;
  cout<<"   * with Track                              : "<<NTrackedEvents<<endl;
  cout<<endl;
  cout<<"All following events have no veto"<<endl;
  cout<<endl;
  cout<<"Total Number of retrievable events           : "<<NRetrievable<<endl;
  cout<<"   * Compton events                          : "<<NRComptonEvents<<endl;
  cout<<"   * Pair events                             : "<<NRPairEvents<<endl;
  cout<<"   * Unidentified events                     : "<<NRUnknownEvents<<endl;
  cout<<endl;
  cout<<"All following events are retrievable"<<endl;
  cout<<endl;
  cout<<"First hit in ..."<<endl;
  cout<<"   * D1                                      : "<<NRFirstIAD1<<endl;
  cout<<"   * D2                                      : "<<NRFirstIAD2<<endl;
  cout<<"   * D3                                      : "<<NRFirstIAD3<<endl;
  cout<<"   * DX                                      : "<<NRFirstIADOther<<endl;
  cout<<"   * Insensitive Material                    : "<<NRFirstIANotSensitive<<endl;
  cout<<endl;
  cout<<"Interaction Sequences for Comptons:"<<endl;
  cout<<"   * D1 - D1                                 : "<<NIASD11<<endl;
  cout<<"   * D1 - D2                                 : "<<NIASD12<<endl;
  cout<<"   * D1 - D3                                 : "<<NIASD13<<endl;
  cout<<"   * D2 - D1                                 : "<<NIASD21<<endl;
  cout<<"   * D2 - D2                                 : "<<NIASD22<<endl;
  cout<<"   * D2 - D3                                 : "<<NIASD23<<endl;
  cout<<"   * D3 - D1                                 : "<<NIASD31<<endl;
  cout<<"   * D3 - D2                                 : "<<NIASD32<<endl;
  cout<<"   * D3 - D3                                 : "<<NIASD33<<endl;
  cout<<"   * unknown                                 : "<<NIASD00<<endl;
  cout<<endl;
  cout<<"Compton events in D1                         : "<<NRComptonD1<<endl;
  cout<<"   * with track                              : "<<NRComptonD1WithTrack<<endl;
  //cout<<"        * in ARM                             : "<<NRComptonD1WithTrackInARM<<endl;
  cout<<"   * without track                           : "<<NRComptonD1WithoutTrack<<endl;
  //cout<<"        * in ARM                             : "<<NRComptonD1WithoutTrackInARM<<endl;
  cout<<"ComptonEvents in D3                          : "<<NRComptonD3<<endl;
  cout<<endl;
  cout<<"Multiple D1 events:"<<endl;
  cout<<"   * Double D1                               : "<<NDoubleD1<<endl; 
  cout<<"       - with track                          : "<<NDoubleD1WithTrack<<endl; 
  cout<<"   * Triple and more D1                      : "<<NTriplePlusD1<<endl; 
  cout<<endl;
  cout<<"Pair events in D1                            : "<<NRPairD1<<endl;
  cout<<endl;
  cout<<"Events written to file                       : "<<NWrittenEvents<<"\a"<<endl;
  cout<<endl;
  cout<<endl;

  delete Compton;
  delete Pair;
  
  EventFile->Close();
  delete EventFile;
  
  PhysFile->Close();
  delete PhysFile;
}


////////////////////////////////////////////////////////////////////////////////




////////////////////////////////////////////////////////////////////////////////

  
// void MInterfaceSivan::AnalyzeSimEventsOrig()
// {
//   // Extract the good events from the event-file:

// //   char Text[10];
//   MComptonEvent *Compton = new MComptonEvent();
//   MString Name = m_Data->GetCurrentFileName();

//   // what events to take:
//   int MinimumTrackLength = 2;
//   double WidthPSF = 4.6;

//   // Some infos about the events:
//   int NEvents = 0;
//   int NVetoedEvents = 0;
//   int NNotVetoedEvents = 0;

//   int NTrackedEvents = 0;

//   int NRetrievable = 0;

//   int NRComptonEvents = 0;
//   int NRPairEvents = 0;
//   int NRUnknownEvents = 0;

//   int NRFirstIAD1 = 0;
//   int NRFirstIAD2 = 0;
//   int NRFirstIANotSensitive = 0;

//   int NRComptonD1 = 0;
//   int NRComptonD1WithTrack = 0;
//   int NRComptonD1WithoutTrack = 0;
//   int NRComptonD1WithTrackInARM = 0;
//   int NRComptonD1WithoutTrackInARM = 0;

//   int NWrittenEvents = 0;

//   int NClusteredEvents = 0;
//   int NClusteredEventsD1 = 0;
//   int NClusteredEventsD2 = 0;

//   // Some data representants (the data will change whether it is a compton or pair event)
//   MVector A, B, C;

//   MSimEvent *Event;

//   // Open the simulation file:
//   MFileEventsSim *EventFile = new MFileEventsSim(m_Geometry);
//   if (EventFile->Open(m_Data->GetCurrentFileName()) == false) return;
//   EventFile->ShowProgress();

//   // Open the physical events file:
//   Name = Name.Replace(Name.Last('.'), Name.Length(), "") + ".tra";
//   MFileEventsTra* PhysFile = new MFileEventsTra(Name, MFile::c_Append);
//   if (PhysFile->IsOpen() == false) {
//     delete PhysFile;
//     PhysFile = 0;
//     return;
//   }


//   while ((Event = EventFile->GetNextEvent()) != 0) {

//     /*
//     if (Event->IsTriggered() == true) {
//       NEvents++;
//     } else {
//       continue;
//     }
//     */

        
//     // Determine if there is some kind of veto:
//     if (Event->GetVeto() == true) {
//       NVetoedEvents++;
//       continue;
//     } else {
//       NNotVetoedEvents++;
//     }
    

//     // Determine if there is some kind of track: 
//     if (Event->HasTrack(MinimumTrackLength) == true) {
//       NTrackedEvents++;
//     }

//     // Determine whether the event is retrievable
//     if (Event->GetIEventRetrieval() == MSimEvent::IdealRetrievalBad || 
//         Event->GetREventRetrieval() == MSimEvent::RealRetrievalBad) {
//       continue;
//     } else {
//       NRetrievable++;
//     }

//     // Determine the event location:
//     if (Event->GetEventLocation() == MSimEvent::D1) {
//       NRFirstIAD1++;
//     } else if (Event->GetEventLocation() == MSimEvent::D2) {
//       NRFirstIAD2++;
//     } else {
//       NRFirstIANotSensitive++;
//     }

//     // Remove events with their second interacion in D1:
//     if (Event->IsCSecondIAD1() == true && Event->GetEventLocation() == MSimEvent::D1) {
//       cout<<"Removing Double-D1-event..."<<endl;
//       //cout<<Event->ToString()<<endl;
//       continue;
//     }
   
    
//     // Get ideal data
//     /*
//     if (Event->GetEventType() == MSimEvent::Compton && 
//         Event->GetEventLocation() == MSimEvent::D1) {
      
//       cout<<"Writing ideal information..."<<endl;
//       Compton->SetDe(Event->GetICElectronD());
//       Compton->SetC1(Event->GetICFirstIA());
//       Compton->SetC2(Event->GetICSecondIA());
//       Compton->SetEe(Event->GetICEnergyElectron());
//       Compton->SetEg(Event->GetICEnergyGamma());
//       //fout<<Compton->ToBasicString()<<endl;
//       NWrittenEvents++;
//       //cout<<"Writing..."<<Compton->ToBasicString()<<endl;
//       //sprintf(Text, "PEV%i", NWrittenEvents);
//       //Compton->Write(Text, TObject::kOverwrite);
//       PhysFile->AddEvent(Compton);
//     }
//     continue;
//     */
        

//     //cout<<Event->GetRCEnergyGamma()<<"!"<<Event->GetRCEnergyElectron()<<endl;
    

//     // Determine the event type
//     if (Event->GetEventType() == MSimEvent::Compton) {
//       NRComptonEvents++;
//       if (Event->GetEventLocation() == MSimEvent::D1) {
//         NRComptonD1++;
//         A = Event->GetRCElectronD(MinimumTrackLength);
        
//         if (A == MVector(0.0, 0.0, 0.0)) {
//           NRComptonD1WithoutTrack++;
//           if (fabs(ComptonAngle(Event->GetRCEnergyElectron(), Event->GetRCEnergyGamma()) -
//                    ComptonAngle(Event->GetICEnergyElectron(), Event->GetICEnergyGamma()))*c_Deg < WidthPSF) {
//             NRComptonD1WithoutTrackInARM++;
//           }
//         } else {
//           NRComptonD1WithTrack++;
//           if (fabs(ComptonAngle(Event->GetRCEnergyElectron(), Event->GetRCEnergyGamma()) -
//                    ComptonAngle(Event->GetICEnergyElectron(), Event->GetICEnergyGamma()))*c_Deg < WidthPSF) {
//             NRComptonD1WithTrackInARM++;
//           }
//         }
//         //cout<<ComptonAngle(Event->GetRCEnergyElectron(), Event->GetRCEnergyGamma())<<endl;
//         //cout<<Event->GetRCEnergyElectron()<<"!"<<Event->GetRCEnergyGamma()<<endl;

      
//         //if (fabs(ComptonAngle(Event->GetRCEnergyElectron(), Event->GetRCEnergyGamma()) -
//         //       ComptonAngle(Event->GetICEnergyElectron(), Event->GetICEnergyGamma()))*c_Deg < 2*WidthPSF) {
      
//         //
//         // Check if the events have clusters:
//         bool HasClusters = false;
//         bool HasClustersD1 = false;
//         bool HasClustersD2 = false;
//         for (int i = 0; i < Event->GetNClusters(); i++) {
//           if (Event->GetClusterAt(i)->GetNHTs() > 1) {
//             if (HasClusters == false) {
//               HasClusters = true;
//               NClusteredEvents++;
//             }
//             if (Event->GetClusterAt(i)->GetDetectorType() == 1 && HasClustersD1 == false) {
//               HasClustersD1 = true;
//               NClusteredEventsD1++;
//             }
//             if (Event->GetClusterAt(i)->GetDetectorType() == 2 && HasClustersD2 == false) {
//               HasClustersD2 = true;
//               NClusteredEventsD2++;
//             }
//           }
//         }

//         // Now write the event:
//         Compton->SetId(Event->GetID());
//         Compton->SetTime(Event->GetTime());
//         cout<<Event->GetID()<<"!"<<Event->GetTime()<<endl;
//         Compton->SetDe(A);
       
//         //Compton->SetC1(Event->GetRCFirstIA());
//         Compton->SetC1(Event->GetRCFirstIAClustered());
//         Compton->SetEe(Event->GetRCEnergyElectron());

//         //Compton->SetC2(Event->GetRCSecondIA());
//         Compton->SetC2(Event->GetRCSecondIAClustered());
//         Compton->SetEg(Event->GetRCEnergyGamma());

//         // Possible ideal data
//         //Compton->SetDe(Event->GetICElectronD());

//         //Compton->SetC1(Event->GetICFirstIA());
//         //Compton->SetEe(Event->GetICEnergyElectron());

//         //Compton->SetC2(Event->GetICSecondIA());
//         //Compton->SetEg(Event->GetICEnergyGamma());

//         NWrittenEvents++;
//         //fout<<Compton->ToBasicString();
//         //sprintf(Text, "PEV%i", NWrittenEvents);
//         //Compton->Write(Text, TObject::kOverwrite);
//         PhysFile->AddEvent((MPhysicalEvent *) Compton);
//       }
//     } else if (Event->GetEventType() == MSimEvent::Pair) {
//       NRPairEvents++;
//     } else {
//       NRUnknownEvents++;
//     }
//   }

//   //Pev->Close();
//   //fout.close();

//   //delete Progress;

//   cout<<endl;
//   cout<<"Event-statistics (theory)"<<endl;
//   cout<<"========================="<<endl<<endl;
//   cout<<"Total number of triggered events             : "<<NEvents<<endl;
//   cout<<endl;
//   cout<<"Total number of NOT vetoed events:           : "<<NNotVetoedEvents<<endl;
//   cout<<"   * with Track                              : "<<NTrackedEvents<<endl;
//   cout<<endl;
//   cout<<"All following events have no veto"<<endl;
//   cout<<endl;
//   cout<<"Total Number of retrievable events           : "<<NRetrievable<<endl;
//   cout<<"   * Compton events                          : "<<NRComptonEvents<<endl;
//   cout<<"   * Pair events                             : "<<NRPairEvents<<endl;
//   cout<<"   * Unidentified events                     : "<<NRUnknownEvents<<endl;
//   cout<<endl;
//   cout<<"All following events are retrievable"<<endl;
//   cout<<endl;
//   cout<<"First hit in ..."<<endl;
//   cout<<"   * D1                                      : "<<NRFirstIAD1<<endl;
//   cout<<"   * D2                                      : "<<NRFirstIAD2<<endl;
//   cout<<"   * Insensitive Material                    : "<<NRFirstIANotSensitive<<endl;
//   cout<<endl;
//   cout<<"ComptonEvents in D1                          : "<<NRComptonD1<<endl;
//   cout<<"   * with track                              : "<<NRComptonD1WithTrack<<endl;
//   cout<<"        * in ARM                             : "<<NRComptonD1WithTrackInARM<<endl;
//   cout<<"   * without track                           : "<<NRComptonD1WithoutTrack<<endl;
//   cout<<"        * in ARM                             : "<<NRComptonD1WithoutTrackInARM<<endl;
//   cout<<endl;
//   cout<<"Events written to file                       : "<<NWrittenEvents<<"\a"<<endl;
//   cout<<endl;
//   if (NWrittenEvents > 0) {
//     cout<<"Events with clustered hits                   : "<<NClusteredEvents<<" ("<<(NClusteredEvents*100)/NWrittenEvents<<"%)"<<endl;
//     cout<<"Events with clustered hits in D1             : "<<NClusteredEventsD1<<" ("<<(NClusteredEventsD1*100)/NWrittenEvents<<"%)"<<endl;
//     cout<<"Events with clustered hits in D2             : "<<NClusteredEventsD2<<" ("<<(NClusteredEventsD2*100)/NWrittenEvents<<"%)"<<endl;
//   }
//   cout<<endl;
// }


////////////////////////////////////////////////////////////////////////////////


// void MInterfaceSivan::CreateResponse()
// {
//   // Create a "E--E2--phi_bar-phi_geo"-Response
  
//   MSimEvent *Event;
//   MSimEventLoader *Loader = new MSimEventLoader(m_Data->GetCurrentFileName());
//   if (Loader->IsFileOpen() == false) {
//     return; 
//   }

//   double E1min = 0, E1max = 30000;
//   int E1bins = 20;
//   double E2min = 0, E2max = 30000;
//   int E2bins = 20;
//   double Phimin = -50, Phimax = 50;
//   int Phibins = 100;


//   int All = 0, Good = 0;
//   double phi;

//   MResponseMatrixO3 *Matrix = 
//     new MResponseMatrixO3(Phimin, Phimax, Phibins, E1min, E1max, E1bins, E2min, E2max, E2bins);

//   // Start the progress display:
//   MGUIProgressBar *Progress;
//   Progress = new MGUIProgressBar(0, "Status", "Sivan: Progress of event analysis");
//   Progress->SetMinMax(0, 1);
//   gSystem->ProcessEvents();

//   while ((Event = Loader->GetNextEvent()) != 0) {
//     Progress->SetValue(Loader->GetProgress());
//     if (Progress->TestCancel() == true) break;
    
//     All++;
//     //cout<<Event->GetTOF()<<"!"<<((Event->GetVeto() == true) ? 1 : 0)<<endl;
//     if (Event->GetTOF() > 5.0 && Event->GetTOF() < 6.0 && 
//         Event->GetVeto() == false &&
//         Event->GetRNHitsD1() == 1 && Event->GetRNHitsD2() == 1 &&
//         Event->GetEventType() == MSimEvent::Pair) {
//       Good++;

//       //cout<<endl<<"Event: "<<Good<<" of "<<All<<endl;

//       //cout<<"Energy: "<<Event->GetREnergyD1()<<"!"<<Event->GetREnergyD2()<<endl;

//       //cout<<"phi_geo: "<<ComptonAngle(Event->GetICEnergyElectron(), Event->GetICEnergyGamma())*c_Deg<<endl;
//       //cout<<"phi_bar: "<<ComptonAngle(Event->GetREnergyD1(), Event->GetREnergyD2())*c_Deg<<endl;
//       //cout<<"Phi_ang: "<<Event->GetICOrigin().Angle(Event->GetRCentralHitD2() - Event->GetRCentralHitD1())*c_Deg<<endl;

//       phi = Event->GetICOrigin().Angle(Event->GetRCentralHitD2() - Event->GetRCentralHitD1())*c_Deg - 
//         ComptonAngle(Event->GetREnergyD1(), Event->GetREnergyD2())*c_Deg;

//       Matrix->Add(phi, Event->GetREnergyD1(), Event->GetREnergyD2(), 1);
      
//       //cout<<"Writing: "<<phi<<"!"<<Event->GetREnergyD1()<<"!"<<Event->GetREnergyD2()<<endl; 
//     }
//   }

//   delete Progress;

//   cout<<"We had "<<All<<" events and accepted "<<Good<<"!"<<endl;

//   //cout<<Matrix->ToString()<<endl;

  
//   MImage2D *Image = new MImage2D("Maps of Maxima", Matrix->GetMaximumMap(), "E1", E1min, E1max, E1bins, "E2", E2min, E2max, E2bins, 2, "LEGO2");
//   Image->Display();

  
//   MImage2D *Image2 = new MImage2D("Maps of standard deviation", Matrix->GetSigmaMap(), "E1", E1min, E1max, E1bins, "E2", E2min, E2max, E2bins, 2, "LEGO2");
//   Image2->Display();
  
//   // Display one ocused E1/E2-value 
// }


////////////////////////////////////////////////////////////////////////////////


void MInterfaceSivan::RetrieveCosima()
{
  // Work around for retrieving event information from Cosima:

  cout<<"Retrieve Compton!"<<endl;

  MComptonEvent *Compton = new MComptonEvent();
  // MPairEvent *Pair = new MPairEvent();
  MString Name = m_Data->GetCurrentFileName();


  // Open the simulation file:
  // gcc 2.95.3: fstream FileCpp(Name, ios::in, 0664);
  fstream FileCpp(Name, ios_base::in);
  if (FileCpp.is_open() == false) return;

  // Open the physical events file:
  Name = Name.Replace(Name.Last('.'), Name.Length(), "") + ".tra";
  MFileEventsTra* PhysFile = new MFileEventsTra();
  if (PhysFile->Open(Name, MFile::c_Write) == false) {
    delete PhysFile;
    PhysFile = 0;
    return;
  }


  const int LineLength = 1000;
  char LineBuffer[LineLength];
  bool Started = false;

  double xPos = 0;
  double yPos = 0;
  double zPos = 0;
  double EnergyElectron = 0;
  double xDirElectron = 0;
  double yDirElectron = 0;
  double zDirElectron = 0;
  double EnergyGamma = 0;
  double xDirGamma = 0;
  double yDirGamma = 0;
  double zDirGamma = 0;

  // Search SE - the first IA after SE is the one wanted:
  while (FileCpp.good() == true) {
    //cout<<"Searching...!"<<endl;
    if (FileCpp.getline(LineBuffer, LineLength, '\n')) {
      //cout<<LineBuffer<<endl;
      if (LineBuffer[0] == 'S' && LineBuffer[1] == 'E') {
        Started = true;
      }
      if (Started == false) continue;
      if (LineBuffer[0] == 'I' && LineBuffer[1] == 'A') {

        //cout<<"Compton!"<<endl;
        // Now parse the input:
        if (sscanf(LineBuffer, "IA C;%lf;%lf;%lf;%lf;%lf;%lf;%lf;%lf;%lf;%lf;%lf;%*f;%*f;%*f\n",
              &xPos, &yPos, &zPos, &xDirElectron, &yDirElectron, &zDirElectron, &xDirGamma, &yDirGamma, &zDirGamma, &EnergyGamma, &EnergyElectron) != 11) {
          cout<<"Error scanning..."<<endl;
        }

        //cout<<xPos<<"!"<<yPos<<"!"<<zPos<<"!"<<xDirElectron<<"!"<<yDirElectron<<"!"<<zDirElectron<<"!"<<xDirGamma<<"!"<<yDirGamma<<"!"<<zDirGamma<<"!"<<EnergyGamma<<"!"<<EnergyElectron<<endl;
        Compton->SetDe(MVector(xDirElectron, yDirElectron, zDirElectron));
        Compton->SetC1(MVector(xPos, yPos, zPos));
        Compton->SetC2(MVector(xPos, yPos, zPos) + MVector(xDirGamma, yDirGamma, zDirGamma));
        Compton->SetEe(EnergyElectron);
        Compton->SetEg(EnergyGamma);

        //cout<<Compton->ToString()<<endl;

        PhysFile->AddEvent((MPhysicalEvent *) Compton);

        Started = false;
      }      
    }
  }
  
  PhysFile->Close();
  delete PhysFile;
  
  return;
}


////////////////////////////////////////////////////////////////////////////////


double MInterfaceSivan::ComptonAngle(double E1, double E2)
{
  // Return the Compton scatter angle


  double Value = 1 - 511.004 * 
    (1/E2 - 1/(E1 + E2));

  if (Value <= -1 || Value >= 1) {
    return -1;
  }

  return acos(Value);
}


////////////////////////////////////////////////////////////////////////////////


void MInterfaceSivan::ViewHits()
{
  // View the hits in a 3D-plot:

  MSimEvent *Event;
  MSimEventLoader *Loader = new MSimEventLoader(m_Data->GetCurrentFileName());
  if (Loader->IsFileOpen() == false) {
    return; 
  }
  Loader->SetGeometry(m_Geometry);
  

  // Start the progress display:
  MGUIProgressBar *Progress;
  Progress = new MGUIProgressBar("Status", "Sivan: Progress of event analysis");
  Progress->SetMinMax(0, 1);
  gSystem->ProcessEvents();

 
  MVector Pos;
  int NBins = 100;

  int xBin, yBin, zBin;
  double xMin = -50, xMax = 50;
  double yMin = -50, yMax = 50;
  double zMin = -50, zMax = 50;
  double xBinWidth = (xMax-xMin)/NBins;
  double yBinWidth = (yMax-yMin)/NBins;
  double zBinWidth = (zMax-zMin)/NBins;

  double *Array = new double[NBins*NBins*NBins];

  while ((Event = Loader->GetNextEvent()) != 0) {
    //cout<<"Hi!"<<endl;
    Progress->SetValue(Loader->GetProgress());
    if (Progress->TestCancel() == true) break;
    
    for (unsigned int i = 0; i < Event->GetNHTs(); i++) {
      //if (Event->GetHTAt(i)->GetDetectorType() != 2) continue;
      Pos = Event->GetHTAt(i)->GetPosition();
      if (Pos.X() > xMin && Pos.X() < xMax &&
          Pos.Y() > yMin && Pos.Y() < yMax &&
          Pos.Z() > zMin && Pos.Z() < zMax) {
        xBin = (int) ((Pos.X() - xMin)/xBinWidth);
        yBin = (int) ((Pos.Y() - yMin)/yBinWidth);
        zBin = (int) ((Pos.Z() - zMin)/zBinWidth);

        Array[xBin + yBin*NBins + zBin*NBins*NBins] += 1;
      }      
    }
  }

  delete Progress;

  MImage3D* Image = 
    new MImage3D("Hit display", Array,
                 "X", xMin, xMax, NBins,
                 "Y", yMin, yMax, NBins,
                 "Z", zMin, zMax, NBins);

  Image->Display();

  delete [] Array;
  
  return;
}


////////////////////////////////////////////////////////////////////////////////


void MInterfaceSivan::EnergySpectrum()
{
  // 

  cout<<"Spectrum!"<<endl;

  int NBins = 100;
  double EMin = 0;
  double EMax = 1000;

  // Open the simulation file:
  MFileEventsSim* EventFile = new MFileEventsSim(m_Geometry);
  if (EventFile->Open(m_Data->GetCurrentFileName()) == false) return;
  EventFile->ShowProgress();

  TH1D* Moliere = new TH1D("Spectrum", "Spectrum", NBins, EMin, EMax);
  Moliere->SetBit(kCanDelete);

  MSimEvent* Event;
  while ((Event = EventFile->GetNextEvent()) != 0) {
    // The real direction of the electron can be found form the IA information
    if (Event->GetNHTs() == 1) { 
      Moliere->Fill(Event->GetHTAt(0)->GetEnergy());
    }

    delete Event;
  }

  TCanvas* Canvas = new TCanvas("Spectrum", "Spectrum");
  Canvas->cd();
  Moliere->Draw();
  Canvas->Update();
}


////////////////////////////////////////////////////////////////////////////////


void MInterfaceSivan::EnergyLoss()
{
  // 

  int NBins = 100;
  double iEnergyMin = 0;
  double iEnergyMax = 1000;
  double rEnergyMin = 0;
  double rEnergyMax = 1000;
  TH2D* ELoss = 
    new TH2D("Energy loss scatter plot", "Energy loss scatter plot", 
             NBins, iEnergyMin, iEnergyMax, NBins, rEnergyMin, rEnergyMax);
  ELoss->SetBit(kCanDelete);

  // Start the Event loader
  MSimEventLoader *Loader = new MSimEventLoader(m_Data->GetCurrentFileName());
  if (Loader->IsFileOpen() == false) {
    return; 
  }
  Loader->SetGeometry(m_Geometry);

  // Start the progress display:
  MGUIProgressBar *Progress;
  Progress = new MGUIProgressBar("Status", "Sivan: Progress of event analysis");
  Progress->SetMinMax(0, 1);
  gSystem->ProcessEvents();

  // Event analysis
  MSimEvent *Event;
  while ((Event = Loader->GetNextEvent()) != 0) {
    Progress->SetValue(Loader->GetProgress());
    if (Progress->TestCancel() == true) break;
    
    ELoss->Fill(Event->GetICEnergy(), Event->GetRCEnergy(), 1);
  }

  delete Progress;

  TCanvas* CELoss = 
    new TCanvas("Energy loss scatter plot", "Energy loss scatter plot", 800, 800);
  CELoss->cd();
  ELoss->Draw();
  ELoss->SetXTitle("Input energy [keV]");
  ELoss->SetYTitle("Measured energy [keV]");

  return;
}


////////////////////////////////////////////////////////////////////////////////


// void MInterfaceSivan::InputEnergy()
// {


//   const int NBins = 100;
//   int Bin;
//   double *EnergyArray = new double[NBins];
//   double BinWidth = 
//     (10000 - 1000)/NBins;
  
//   MSimEvent *Event;
//   MSimEventLoader *Loader = new MSimEventLoader(m_Data->GetCurrentFileName());

//   Loader->SetGeometry(m_Geometry);
//   if (Loader->IsFileOpen() == false) {
//     return; 
//   }

//   while ((Event = Loader->GetNextEvent()) != 0) {
//     //cout<<"Input: "<<Event->GetICEnergy()<<endl;
//     Bin = int ((Event->GetICEnergy() - 1000)/BinWidth);
//     if (Bin >= 0 && Bin < NBins) {
//       EnergyArray[Bin] += 1;
//     }
//   }

//   //  ... and display it.
//   MDisplay Display; 
//   Display.DisplayHistogram("Input energy distribution", "Energy [keV]", "Number of counts", 
//                             EnergyArray, 1000, 10000, NBins);

//   delete [] EnergyArray;
// }


////////////////////////////////////////////////////////////////////////////////


void MInterfaceSivan::TriggerPatternEfficiency()
{
  // Calculate trigger pattern efficiencies:

  MSimEvent *Event;
  MSimEventLoader *Loader = new MSimEventLoader(m_Data->GetCurrentFileName());

  Loader->SetGeometry(m_Geometry);
  if (Loader->IsFileOpen() == false) {
    return; 
  }

  unsigned int MaxHitsD1 = 15 +1;
  unsigned int MaxHitsD2 = 1 +1;
  vector<vector<int> > Hits;
  Hits.resize(MaxHitsD1);
  for (unsigned int i = 0; i < MaxHitsD1; ++i) {
    Hits[i].resize(MaxHitsD2+1, 0);
  }


  unsigned int NHitsD1, NHitsD2;
  while ((Event = Loader->GetNextEvent()) != 0) {
    //cout<<"Event:"<<endl;
    //cout<<Event->ToString()<<endl;
    NHitsD1 = Event->GetNTriggeredLayers();
    //cout<<"Layers: "<<NHitsD1<<endl;
   

    NHitsD2 = 0;
    for (unsigned int i = 0; i < Event->GetNHTs(); ++i) {
      if (Event->GetHTAt(i)->GetDetectorType() == 2) {
        NHitsD2++;
      }
    }
    if (NHitsD1 < MaxHitsD1 && NHitsD2 < MaxHitsD2) {
      Hits[NHitsD1][NHitsD2]++;
    }
  }  

  // Display result of all different patterns:
  int PatternHits;
  for (unsigned int d1 = 0; d1 < MaxHitsD1; ++d1) {
    for (unsigned int d2 = 0; d2 < MaxHitsD2; ++d2) {
      cout<<"Pattern "<<d1<<"-"<<d2<<": "<<Hits[d1][d2]<<endl;;
      PatternHits = 0;
      for (unsigned int hd1 = d1; hd1 < MaxHitsD1; ++hd1) {
        for (unsigned int hd2 = d2; hd2 < MaxHitsD2; ++hd2) {
          PatternHits += Hits[hd1][hd2];
        }
      }
      cout<<" * Pattern "<<d1<<"p-"<<d2<<"p: "<<PatternHits<<endl;;
      PatternHits = 0;
      for (unsigned int hd1 = d1; hd1 < MaxHitsD1; ++hd1) {
        PatternHits += Hits[hd1][d2];
      }
      cout<<" * Pattern "<<d1<<"p-"<<d2<<": "<<PatternHits<<endl;;
    }
  }

  // create Histogram
  TH2D* Histo = new TH2D("A", "A", MaxHitsD1, -0.5, MaxHitsD1 +0.5, MaxHitsD2, -0.5, MaxHitsD2 + 0.5);
  for (unsigned int d1 = 0; d1 < MaxHitsD1; ++d1) {
    for (unsigned int d2 = 0; d2 < MaxHitsD2; ++d2) {
      Histo->Fill(d1, d2, Hits[d1][d2]);
    }
  }
  Histo->SetBit(kCanDelete);
  Histo->SetXTitle("d1");
  Histo->SetYTitle("d2");
  Histo->Draw("colz");

  return;
}


////////////////////////////////////////////////////////////////////////////////


void MInterfaceSivan::BeamMonitorEfficiency()
{
  // Calculate trigger pattern efficiencies:

  MSimEvent* Event;
  MFileEventsSim* EventFile = new MFileEventsSim(m_Geometry);
  if (EventFile->Open(m_Data->GetCurrentFileName()) == false) return;
  EventFile->ShowProgress();

  TH1D* Energy = new TH1D("Energy", "Energy", 120, 0, 5000);
  Energy->SetBit(kCanDelete);

  int NTriggeredEvents = 0;
  while ((Event = EventFile->GetNextEvent()) != 0) {
    //cout<<"Event: "<<Event->GetID()<<endl;
    if (Event->GetREnergy()) {
      Energy->Fill(Event->GetREnergy());
      NTriggeredEvents++;
    }
    delete Event;
  }  

  Energy->Draw();

  cout<<"Number of triggered events: "<<NTriggeredEvents<<endl;


  return;
}


////////////////////////////////////////////////////////////////////////////////


void MInterfaceSivan::TestOnly()
{
  // Compute the average energy loss of the electron in the first layer

  MSimEvent* Event;
  MFileEventsSim* EventFile = new MFileEventsSim(m_Geometry);
  m_Geometry->ActivateNoising(false);
  if (EventFile->Open(m_Data->GetCurrentFileName()) == false) return;
  EventFile->ShowProgress();


  TH1D* Hist = new TH1D("InitialEnergySpectrum", "Initial energy spectrum", 
                        10000, 0, 10000);
  Hist->SetBit(kCanDelete);
  Hist->SetFillColor(8);
  Hist->GetXaxis()->SetTitle("Energy [keV]");
  Hist->GetYaxis()->SetTitle("counts/s");
  
  while ((Event = EventFile->GetNextEvent()) != 0) {
    double MeasuredEnergy = 0.0;
    Hist->Fill(MeasuredEnergy);
  }

  return;

//   MSimEvent* Event;
//   MFileEventsSim* EventFile = new MFileEventsSim(m_Geometry);
//   m_Geometry->ActivateNoising(false);
//   if (EventFile->Open(m_Data->GetCurrentFileName()) == false) return;
//   EventFile->ShowProgress();

//   double TotalIncoming = 0.0;
//   double TotalEscape = 0.0;
//   double TotalNotSensitive = 0.0;
//   double TotalMeasured = 0.0;
  
//   double Escape = 0.0;
//   int NTriggeredEvents = 0;

//   TH1D* DepositsFirstLayer = new TH1D("D", "D", 1000, 0, 500);

//   int NFirstIAD1 = 0;

//   while ((Event = EventFile->GetNextEvent()) != 0) {
//     //cout<<"Event: "<<Event->GetID()<<endl;
//     if (Event->GetRNHitsD1() >= 1 && Event->GetRNHitsD2() >= 1) {
      
//       unsigned int i, j;
//       int Vertex = -1;
//       double Energy = 0;
//       double zLayer = 0.0;
      
//       if (Event->GetNIAs() > 1) {
//         if (Event->GetIAAt(1)->GetDetectorType() == 1) {
//           NFirstIAD1++;
//           zLayer = Event->GetIAAt(1)->GetPosition().Z();
//           Vertex = Event->GetIAAt(1)->GetID();
//         } else {
//           continue;
//         }
//       }
      
//       double Smallest = numeric_limits<double>::max();
//       // Test if the first energy deposit is in a D1:
//       for (i = 0; i < Event->GetNHTs(); ++i) {
//         if (Event->GetHTAt(i)->IsOrigin(Vertex) == true) {
//           if (Event->GetHTAt(i)->GetDetectorType() == 1) {
//             if (fabs(Event->GetHTAt(i)->GetPosition().Z() - zLayer) < 0.5) {
//               if (Event->GetHTAt(i)->GetEnergy() < Smallest) {
//                 Smallest = Event->GetHTAt(i)->GetEnergy();
//               }
//             } 
//           }
//         }
//       }

//       if (Smallest != numeric_limits<double>::max()) {
//         DepositsFirstLayer->Fill(Smallest);
//       }

//     }

//     delete Event;
//   }

//   DepositsFirstLayer->Draw();

//   cout<<"First D1: "<<NFirstIAD1<<endl;

//   return;

//   /*
//   MSimEvent* Event;
//   MFileEventsSim* EventFile = new MFileEventsSim(m_Geometry);
//   if (EventFile->Open(m_Data->GetCurrentFileName()) == false) return;
//   EventFile->ShowProgress();

//   double TotalIncoming = 0.0;
//   double TotalEscape = 0.0;
//   double TotalNotSensitive = 0.0;
//   double TotalMeasured = 0.0;
  
//   double Escape = 0.0;
//   int NTriggeredEvents = 0;

//   while ((Event = EventFile->GetNextEvent()) != 0) {
//     //cout<<"Event: "<<Event->GetID()<<endl;
//     if (Event->GetRNHitsD1() >= 1 && Event->GetRNHitsD2() >= 1) {
//       NTriggeredEvents++;
//       cout<<Event->GetID()<<endl;
//       TotalIncoming += Event->GetIAAt(0)->GetSecondaryEnergy();

//       Escape = 0.0;
//       for (unsigned int i = 0; i < Event->GetNIAs(); ++i) {
//         if (Event->GetIAAt(i)->GetProcess() == "ESCP") {
//           Escape += Event->GetIAAt(i)->GetMotherEnergy();
//         }
//       }
//       TotalEscape += Escape;
//       cout<<Escape<<endl;
//       TotalNotSensitive += Event->GetEnergyDepositNotSensitiveMaterial();
//       cout<<Event->GetEnergyDepositNotSensitiveMaterial()<<endl;
//       TotalMeasured += Event->GetREnergy();
//     }
//     delete Event;
//   }  

//   cout<<"Number of triggered events: "<<NTriggeredEvents<<endl;
//   cout<<"Avg incoming: "<<TotalIncoming<<" - "<<TotalIncoming/NTriggeredEvents<<endl;
//   cout<<"Avg measured: "<<TotalMeasured<<" - "<<TotalMeasured/NTriggeredEvents<<endl;
//   cout<<endl;
//   cout<<"Avg ecape: "<<TotalEscape<<" - "<<TotalEscape/NTriggeredEvents<<endl;
//   cout<<"Avg not sensitive: "<<TotalNotSensitive<<" - "<<TotalNotSensitive/NTriggeredEvents<<endl;

//   return;
//   */

//   //HitsPerEnergy();
//   //EnergySpectrum();

//   //BeamMonitorEfficiency();

//   /*
//   Moliere();
//   ///DopplerBroadening();
//   //InputEnergy();
//   //EnergyInCrystal();
  
//   //RetrieveCosima();

//   //PitchAnalysis();

//   //SumDeposits();
//   return;

//   int i, Inside = 0, Outside = 0;

//   // Initalize geometry;

//   // Start the Event loader
//   MSimEvent *Event;
//   MSimEventLoader *Loader = new MSimEventLoader(m_Data->GetCurrentFileName());
//   if (Loader->IsFileOpen() == false) {
//     return; 
//   }

//   MSimHT* HT;
//   MVector Pos;
//   double Energy;
//   while ((Event = Loader->GetNextEvent()) != 0) {

//     for (i = 0; i < Event->GetNHTs(); i++) {
//       HT = Event->GetHTAt(i);
//       Pos = HT->GetPosition();
//       Energy = HT->GetEnergy();
//       cout<<"Before: "<<Pos.X()<<", "<<Pos.Y()<<", "<<Pos.Z()<<", "<<Energy<<endl;
//       m_Geometry->Noise(Pos, Energy);
//       cout<<"After: "<<Pos.X()<<", "<<Pos.Y()<<", "<<Pos.Z()<<", "<<Energy<<endl;
//     }
//   }

//   cout<<"Inside: "<<Inside<<"    Outside: "<<Outside<<endl;

//   return;
//   */
}


////////////////////////////////////////////////////////////////////////////////


void MInterfaceSivan::AverageEnergyLossInFirstLayer()
{
  //

  int NEvents = 0; 
  double Energy = 0;
  double Average = 0.0;

  // Start the Event loader
  MSimEvent *Event;
  MSimEventLoader *Loader = new MSimEventLoader(m_Data->GetCurrentFileName());
  if (Loader->IsFileOpen() == false) {
    return; 
  }
  Loader->SetGeometry(m_Geometry);

  while ((Event = Loader->GetNextEvent()) != 0) {
    //if (Event->GetEnergyDepositNotSensitiveMaterial() > 0) continue;

    Energy = Event->GetRFirstEnergyDepositElectron();
    if (Energy < 0.0001) continue; 
    Average += Energy;
    NEvents++;
  }

  if (NEvents > 1) {
    Average /= NEvents;
  }

  cout<<"The average energy deposit in the first layer was: "<<Average<<endl;

  return;
}


////////////////////////////////////////////////////////////////////////////////


void MInterfaceSivan::TrackLengthVersusEnergy()
{
  // 

  int NEvents = 0; 
  int MaxLength = 30;
  int Bin;
  double *Length = new double[MaxLength];
  double Average = 0.0;

  // Start the Event loader
  MSimEvent *Event;
  MSimEventLoader *Loader = new MSimEventLoader(m_Data->GetCurrentFileName());
  if (Loader->IsFileOpen() == false) {
    delete [] Length;
    return; 
  }

  // Start the progress display:
  MGUIProgressBar *Progress;
  Progress = new MGUIProgressBar("Status", "Sivan: Progress of event analysis");
  Progress->SetMinMax(0, 1);
  gSystem->ProcessEvents();

  while ((Event = Loader->GetNextEvent()) != 0) {
    Progress->SetValue(Loader->GetProgress());
    if (Progress->TestCancel() == true) break;
    //if (Event->GetEnergyDepositNotSensitiveMaterial() > 0) continue;

    //cout<<Event->GetEventLocation()<<"!"<<Event->GetEventType()<<endl;

    if (Event->GetEventLocation() == MSimEvent::D1 &&
        Event->GetEventType() == MSimEvent::Compton) {
      Bin = Event->GetLengthFirstTrack();
      if (Bin >= 0 && Bin < MaxLength) {
        Length[Bin]++;
        Average += Bin;
        NEvents++;
        //cout<<Bin<<endl;
      }
    }
  }

  delete [] Length;
  delete Progress;

  if (NEvents > 1) {
    Average /= NEvents;
  }


  cout<<"The average track length was: "<<Average<<endl;

  return;
}


////////////////////////////////////////////////////////////////////////////////


void MInterfaceSivan::EnergyPerVoxel()
{
  // Distribution of the individual energy deposits in the crystals:

  mgui<<"Warning this is junk software!"<<error;

  // Initialize the data:
  MSimHT *Hit;

  MString DetectorName = "";
  //double TotalEnergy = 662;

  int NBins = 100;
  double MinEnergy = 0.0; // keV
  double MaxEnergy = 3500.0; // keV

  TH1D* VoxelD1 = new TH1D("Energy deposits in D1 strips", "Energy deposits in D1 STRIPS", NBins, MinEnergy, MaxEnergy);
  VoxelD1->SetBit(kCanDelete);
  TH1D* VoxelD2 = new TH1D("Energy deposits in D2 voxels", "Energy deposits in D2 voxel", NBins, MinEnergy, MaxEnergy);
  VoxelD1->SetBit(kCanDelete);

  TH1D* VoxelD2FullyAbsorbed = new TH1D("FullyAbsorbed", "FullyAbsorbed", NBins, MinEnergy, MaxEnergy);
  VoxelD2FullyAbsorbed->SetBit(kCanDelete);
  VoxelD2FullyAbsorbed->SetStats(false);
  TH1D* VoxelD2Photo = new TH1D("Photo", "Photo", NBins, MinEnergy, MaxEnergy);
  VoxelD2Photo->SetBit(kCanDelete);
  VoxelD2Photo->SetStats(false);
  TH1D* VoxelD2SingleScatter = new TH1D("SingleScatter", "SingleScatter", NBins, MinEnergy, MaxEnergy);
  VoxelD2SingleScatter->SetBit(kCanDelete);
  VoxelD2SingleScatter->SetStats(false);
  TH1D* VoxelD2DoubleScatter = new TH1D("DoubleScatter", "DoubleScatter", NBins, MinEnergy, MaxEnergy);
  VoxelD2DoubleScatter->SetBit(kCanDelete);
  VoxelD2DoubleScatter->SetStats(false);
  TH1D* VoxelD2TripplePScatter = new TH1D("TripplePScatter", "TripplePScatter", NBins, MinEnergy, MaxEnergy);
  VoxelD2TripplePScatter->SetBit(kCanDelete);
  VoxelD2TripplePScatter->SetStats(false);
  TH1D* VoxelD2ScatteredIn = new TH1D("ScatteredIn", "ScatteredIn", NBins, MinEnergy, MaxEnergy);
  VoxelD2ScatteredIn->SetBit(kCanDelete);
  VoxelD2ScatteredIn->SetStats(false);
  TH1D* VoxelD2Rest = new TH1D("Rest", "Rest", NBins, MinEnergy, MaxEnergy);
  VoxelD2Rest->SetBit(kCanDelete);
  VoxelD2Rest->SetStats(false);


  // Open the simulation file:
  MSimEvent* Event = 0;
  MFileEventsSim* EventFile = new MFileEventsSim(m_Geometry);
  if (EventFile->Open(m_Data->GetCurrentFileName()) == false) return;
  EventFile->ShowProgress();

  // Analysis - part:
  double Energy, x = 0, z = 0;
  bool MoreEvents = true;
  while (MoreEvents == true) {
    if (Event != 0) delete Event;
    if ((Event = EventFile->GetNextEvent()) == 0) {
      MoreEvents = false;
      break;
    }

    cout<<"Next event..."<<Event->GetNHTs()<<endl;

    if (Event->GetNHTs() == 1) 
      {
      //Event->GetEventType() == MSimEvent::Compton) {
      // Check each hit. If it is in D2, and if it is the only hit then add it;
      for (unsigned int h = 0; h < Event->GetNHTs(); h++) {
        Hit = Event->GetHTAt(h);

        // Check if the hit happened in the requested detector:
        if (DetectorName != "") {
          MDVolumeSequence V = m_Geometry->GetVolumeSequence(Hit->GetPosition());
          if (V.HasVolume(DetectorName) == false) continue;
        } 

        if (Hit->GetDetectorType() == 1 && Hit->IsAdded() == false) {
          Energy = Hit->GetEnergy();
          x = Hit->GetPosition().X();
          z = Hit->GetPosition().Z();
          Hit->SetAddFlag(true);
          // Now check if there is a hit with similar x/z Values (same layer, same strip):
          for (unsigned int hh = h+1; hh < Event->GetNHTs(); hh++) {
            if (Event->GetHTAt(hh)->IsAdded() == false && 
                Event->GetHTAt(hh)->GetPosition().X() == x &&
                Event->GetHTAt(hh)->GetPosition().Z() == z) {
              Energy += Event->GetHTAt(hh)->GetEnergy();
              Event->GetHTAt(hh)->SetAddFlag(true);
            }
          }
          VoxelD1->Fill(Energy, 1);
          //cout<<"x: "<<x<<"  Energy: "<<Energy<<endl;
        } else if (Hit->GetDetectorType() == 2) {
          VoxelD2->Fill(Hit->GetEnergy(), 1);

          if (Event->GetNIAs() >= 2) {
            bool ScatteredIn = false;
            if (Event->GetIAAt(1)->GetDetectorType() != 2) {
              VoxelD2ScatteredIn->Fill(Hit->GetEnergy());
              ScatteredIn = true;
              cout<<"Sequence: Scattered in from outside D2! Origin detector:"
                  <<Event->GetIAAt(1)->GetDetectorType()<<endl;
            } else {
              bool NotScatteredIn = false;
              for (unsigned int o = 0; o < Hit->GetNOrigins(); ++o) {
                if (Hit->GetOriginAt(o) == 2) {
                  NotScatteredIn = true;
                }
              }
              if (NotScatteredIn == false) {
                VoxelD2ScatteredIn->Fill(Hit->GetEnergy());
                ScatteredIn = true;
                cout<<"Sequence: Scattered in from D2!"<<endl;
              }
            }

            if (ScatteredIn == false) {
              // Check how many Comptons happened in this volume:
              int NComptons = 0;
              bool Photo = false;
              //bool Compton = false;
              //bool Pair = false;
              //bool Brems = false;
              


              for (unsigned int o = 0; o < Hit->GetNOrigins(); ++o) {
                if (Event->GetIAAt(Hit->GetOriginAt(o)-1)->GetProcess() == "COMP") {
                  NComptons++;
                } else if (Event->GetIAAt(Hit->GetOriginAt(o)-1)->GetProcess() == "PHOT") {
                  Photo = true;
                } else if (Event->GetIAAt(Hit->GetOriginAt(o)-1)->GetProcess() == "INIT") {
                  ;
                }
              }
              if (NComptons == 1) {
                VoxelD2SingleScatter->Fill(Hit->GetEnergy());
                cout<<"Sequence: Single Compton!"<<endl;
              } else if (NComptons == 2) {
                VoxelD2DoubleScatter->Fill(Hit->GetEnergy());
                cout<<"Sequence: Double Compton!"<<endl;
              } else if (NComptons >= 2) {
                VoxelD2TripplePScatter->Fill(Hit->GetEnergy());
                cout<<"Sequence: Tripple Compton!"<<endl;
              } else if (Photo == true) {
                VoxelD2Photo->Fill(Hit->GetEnergy());
                cout<<"Sequence: Photo effect!"<<endl;
              } else {
                VoxelD2Rest->Fill(Hit->GetEnergy());
                cout<<"Sequence: Unknown!"<<endl;
              }
            }
          } else {
            VoxelD2Rest->Fill(Hit->GetEnergy());
            cout<<"Sequence: Unknown!"<<endl;
          }
          //cout<<Event->GetSimulationData()<<endl;
        } // D2
      } // all hits loop
    } // only one hit
  } // More events

  TCanvas* D1Canvas = new TCanvas();
  D1Canvas->cd();
  VoxelD1->Draw();

  TCanvas* D2Canvas = new TCanvas();
  D2Canvas->cd();
  VoxelD2->Draw();

  TCanvas* D2SplitCanvas = new TCanvas("Single crystal spectrum for Cs^{137} shown by its interaction components",
                                       "Single crystal spectrum for Cs^{137} shown by its interaction components");
  D2SplitCanvas->cd();


  cout<<"Summary: "<<endl;
  cout<<"FullyAbsorbed:   "<<VoxelD2FullyAbsorbed->Integral(1, NBins)<<endl;
  cout<<"Photo:   "<<VoxelD2Photo->Integral(1, NBins)<<endl;
  cout<<"SingleScatter:   "<<VoxelD2SingleScatter->Integral(1, NBins)<<endl;
  cout<<"DoubleScatter:   "<<VoxelD2DoubleScatter->Integral(1, NBins)<<endl;
  cout<<"TripplePScatter:   "<<VoxelD2TripplePScatter->Integral(1, NBins)<<endl;
  cout<<"ScatteredIn:   "<<VoxelD2ScatteredIn->Integral(1, NBins)<<endl;
  cout<<"Rest:   "<<VoxelD2Rest->Integral(1, NBins)<<endl;
  //cout<<":   "<<VoxelD2->Integral(1, NBins)<<endl;

  cout<<"Dump as cvs file: "<<endl;
  for (int i = 1; i <= NBins; ++i) {
    cout<<i<<";\t"
        <<VoxelD2Photo->GetBinContent(i)<<";\t"
        <<VoxelD2SingleScatter->GetBinContent(i)<<";\t"
        <<VoxelD2DoubleScatter->GetBinContent(i)<<";\t"
        <<VoxelD2TripplePScatter->GetBinContent(i)<<";\t"
        <<VoxelD2ScatteredIn->GetBinContent(i)<<endl;
  }



  int Color = 1;

  VoxelD2FullyAbsorbed->SetFillColor(Color++);

  VoxelD2Photo->SetFillColor(Color++);
  VoxelD2Photo->Add(VoxelD2FullyAbsorbed);
  VoxelD2SingleScatter->SetFillColor(Color++);
  VoxelD2SingleScatter->Add(VoxelD2Photo);
  VoxelD2DoubleScatter->SetFillColor(Color++);
  VoxelD2DoubleScatter->Add(VoxelD2SingleScatter);
  VoxelD2TripplePScatter->SetFillColor(Color++);
  VoxelD2TripplePScatter->Add(VoxelD2DoubleScatter);
  VoxelD2ScatteredIn->SetFillColor(Color++);
  VoxelD2ScatteredIn->Add(VoxelD2TripplePScatter);
  VoxelD2Rest->SetFillColor(Color++);
  VoxelD2Rest->Add(VoxelD2ScatteredIn);

  VoxelD2Rest->Draw();
  VoxelD2ScatteredIn->Draw("SAME");
  VoxelD2TripplePScatter->Draw("SAME");
  VoxelD2DoubleScatter->Draw("SAME");
  VoxelD2SingleScatter->Draw("SAME");
  VoxelD2Photo->Draw("SAME");
  VoxelD2FullyAbsorbed->Draw("SAME");

  Color = 1;
  double TextSize = 0.03;
  //double lx = 50.0;
  double ly = VoxelD2Rest->GetMaximum();
  double dly = VoxelD2Rest->GetMaximum()/15;

  D2SplitCanvas->Update();
  D2SplitCanvas->cd();
 
  TPaveText* Text = 0;
  
  ly -= dly;
  Text = new TPaveText( 0.5*MaxEnergy+100, ly-0.15*dly, MaxEnergy-50, ly);
  Text->AddText("Photo effect");
  Text->SetTextSize(TextSize);
  Text->SetFillColor(++Color);
  Text->Draw();
 
  ly -= dly;
  Text = new TPaveText( 0.5*MaxEnergy+100, ly-0.15*dly, MaxEnergy-50, ly);
  Text->AddText("Single Compton scatter");
  Text->SetTextSize(TextSize);
  Text->SetFillColor(++Color);
  Text->Draw();
 
  ly -= dly;
  Text = new TPaveText( 0.5*MaxEnergy+100, ly-0.15*dly, MaxEnergy-50, ly);
  Text->AddText("Double Compton scatter");
  Text->SetTextSize(TextSize);
  Text->SetFillColor(++Color);
  Text->Draw();
 
  ly -= dly;
  Text = new TPaveText( 0.5*MaxEnergy+100, ly-0.15*dly, MaxEnergy-50, ly);
  Text->AddText("Triple Compton scatter");
  Text->SetTextSize(TextSize);
  Text->SetFillColor(++Color);
  Text->Draw();
 
  ly -= dly;
  Text = new TPaveText( 0.5*MaxEnergy+100, ly-0.15*dly, MaxEnergy-50, ly);
  Text->AddText("Scattered from outside in");
  Text->SetTextSize(TextSize);
  Text->SetFillColor(++Color);
  Text->Draw();
 
  ly -= dly;
  Text = new TPaveText( 0.5*MaxEnergy+100, ly-0.15*dly, MaxEnergy-50, ly);
  Text->AddText("Unaccouted");
  Text->SetTextSize(TextSize);
  Text->SetFillColor(++Color);
  Text->Draw();

  VoxelD2Rest->SetTitle("Single crystal spectrum for Cs^{137} shown by its interaction components");
  D2SplitCanvas->Update();

 //  t->SetTextColor(Color++);
//   t->DrawLatex(lx, (ly-=dly), "Photo");
//   t->SetTextColor(Color++);
//   t->DrawLatex(lx, (ly-=dly), "SingleScatter");
//   t->SetTextColor(Color++);
//   t->DrawLatex(lx, (ly-=dly), "DoubleScatter");
//   t->SetTextColor(Color++);
//   t->DrawLatex(lx, (ly-=dly), "TripplePScatter");
//   t->SetTextColor(Color++);
//   t->DrawLatex(lx, (ly-=dly), "ScatteredIn");
//   t->SetTextColor(Color++);
//   t->DrawLatex(lx, (ly-=dly), "Rest");
//   t->SetTextColor(Color++);
  //t->DrawLatex(lx, ly, "");

  return;
}


////////////////////////////////////////////////////////////////////////////////


// void MInterfaceSivan::MultipleComptons()
// {
//   // Display the (ideal) distance between two compton interactions,
//   // and the angular difference - real/ideal - between the two hits.

//   cout<<"Multiple Comptons!"<<endl;

//   int NAnalyzedEvents = 0;
//   int NSeparatedTriples = 0;
//   int NNotSeparatedTriples = 0;

//   int b;
//   MSimHT *Hit2, *Hit3;

//   // Angle:
//   double Angle;
//   int AngleBins = 50;
//   double AngleMin = 0;
//   double AngleMax = +20;
//   double AngleBinWidth = (AngleMax - AngleMin)/AngleBins;

//   double *AArray = new double[AngleBins];
//   for (b = 0; b < AngleBins; b++) AArray[b] = 0;
  
//   // Distance
//   double Distance;
//   int DistanceBins = 100;
//   double DistanceMin = 0;
//   double DistanceMax = +20;
//   double DistanceBinWidth = (DistanceMax - DistanceMin)/DistanceBins;

//   double *DArray = new double[DistanceBins];
//   for (b = 0; b < DistanceBins; b++) DArray[b] = 0;


//   // Start the Event loader
//   MSimEvent *Event;
//   MSimEventLoader *Loader = new MSimEventLoader(m_Data->GetCurrentFileName());
//   if (Loader->IsFileOpen() == false) {
//     return; 
//   }
//   Loader->SetGeometry(m_Geometry);

//   // Start the progress display:
//   MGUIProgressBar *Progress;
//   Progress = new MGUIProgressBar(0, "Status", "Sivan: Progress of event analysis");
//   Progress->SetMinMax(0, 1);
//   gSystem->ProcessEvents();

//   // Analysis - part:
//   while ((Event = Loader->GetNextEvent()) != 0) {
//     NAnalyzedEvents++;

//     Progress->SetValue(Loader->GetProgress());
//     if (Progress->TestCancel() == true) break;

//     if (Event->GetEventLocation() == MSimEvent::D1 &&
//         Event->GetEventType() == MSimEvent::Compton) {
      
//       // Check if we have a multiple compton event:
//       if (Event->GetNIAs() < 4) continue;
//       // We need three comptons:
//       if (Event->GetIAAt(1)->GetProcess().CompareTo("COMP") == 0 && Event->GetIAAt(1)->GetDetectorType() == 1 &&
//           Event->GetIAAt(2)->GetProcess().CompareTo("COMP") == 0 && Event->GetIAAt(2)->GetDetectorType() == 2 &&
//           Event->GetIAAt(3)->GetProcess().CompareTo("COMP") == 0 && Event->GetIAAt(3)->GetDetectorType() == 2) {
//         // We need seperated crystals for 2 & 3:
//         //cout<<"Three comptons..."<<endl;
//         Hit2 = 0;
//         Hit3 = 0;
//         for (unsigned int i = 0; i < Event->GetNHTs(); i++) {
//           if (Event->GetHTAt(i)->IsOrigin(3) == true) {
//             Hit2 = Event->GetHTAt(i);
//           } else if (Event->GetHTAt(i)->IsOrigin(4) == true) {
//             Hit3 = Event->GetHTAt(i);
//           }
//         }
//         /*
//         if (Hit2 != 0) {
//           cout<<Hit2->ToString()<<endl;
//         } else {
//           cout<<"Hit2 bad!"<<endl;
//         }
//         if (Hit3 != 0) {
//           cout<<Hit3->ToString()<<endl;
//         } else {
//           cout<<"Hit3 bad!"<<endl;
//         }
//         */
        
//         if (Hit2 != 0 && Hit3 != 0 && Hit2 != Hit3) {
//           NSeparatedTriples++;
//           // So finally we have a multiple compton event...
//           if (NSeparatedTriples > 27000) {
//             cout<<"Break after 27000 triples..."<<endl;
//             break;
//           }
//          //cout<<"Seperated!"<<endl;

//           // Compute the angle between the real and the measured scatter angle:
//           Angle = fabs((Hit3->GetPosition() - Hit2->GetPosition()).Angle(Event->GetIAAt(3)->GetPosition() - Event->GetIAAt(2)->GetPosition())*c_Deg);
//           b = (int) ((Angle - AngleMin)/AngleBinWidth);
//           if (b >= 0 && b < AngleBins) {
//             AArray[b] += 1.0;
//           }

//         } else {
//           NNotSeparatedTriples++;
//         }

//         // For the distance we do not care if they are separated or not...
//         // Compute the distance between the two interactions:
//         Distance = (Event->GetIAAt(3)->GetPosition() - Event->GetIAAt(2)->GetPosition()).Mag();
//         b = (int) floor((Distance - DistanceMin)/DistanceBinWidth+0.5);
//         if (b >= 0 && b < DistanceBins) {
//           DArray[b] += 1.0;
//         }
//       }
//     }
//   }

//   cout<<"Analyzed events: "<<NAnalyzedEvents<<endl;
//   cout<<"Number of not separable triples:"<<NNotSeparatedTriples<<endl;
//   cout<<"Number of separable triples: "<<NSeparatedTriples<<endl;
//   cout<<"Others: "<<NAnalyzedEvents - NSeparatedTriples - NNotSeparatedTriples<<endl;

//   // Average distance between two Compton-IA:
//   Distance = 0;
//   double Sum = 0;
//   for (b = 0; b < DistanceBins; b++) {
//     Distance += DArray[b]*(b+0.5)*DistanceBinWidth;
//     Sum += DArray[b];
//   }
//   Distance /= Sum;
//   cout<<endl;
//   cout<<"Average distance between second and third interaction: "<<Distance<<endl;

//   MDisplay ADisplay; 
//   ADisplay.DisplayHistogram("Angle between real and measured Compton angle", "Angle [deg]", "Number of counts", 
//                             AArray, AngleMin, AngleMax, AngleBins);
//   MDisplay DDisplay; 
//   DDisplay.DisplayHistogram("Distance between second and third interaction", "Distance [cm]", "Number of counts", 
//                             DArray, DistanceMin, DistanceMax, DistanceBins);

//   delete Progress;
// }


////////////////////////////////////////////////////////////////////////////////


void MInterfaceSivan::DopplerBroadening()
{
  // Calculate Doppler Broadening
  
  MSimEvent *Event;

  double PhiMin = 0.0;
  double PhiMax = 180.0;

  double EMin = 0.0;
  double EMax = 5000.0;

  int NBinsL = 2*15+1;
  int NBinsE = 100+1;
  int NBinsH = 2*3600+1;
  int NBins = 201;

  double dE;
  double iEe, iEg;
  double dEe, dEg;
  double iPhi, rPhi;

  // For fitting:
  TF1 *L = new TF1("Lorentz2", Lorentz2, -NBinsH, +NBinsH, 2);
  L->SetParameters(1,1);
  L->SetParLimits(0, 0, 99999);
  L->SetParLimits(1, 0, 99999);
  L->SetParNames("Height", "Width", "Mean");

  // All the histograms
  TH1D *PhidPhiStdDev = new TH1D("80% containment angle vs. #varphi", "80% containment angle vs. #varphi", NBinsL, PhiMin, PhiMax);
  PhidPhiStdDev->SetBit(kCanDelete);

  TH1D *PhiDevStat = new TH1D("PhiDevStat", "PhiDevStat", NBinsL, PhiMin, PhiMax);
  PhiDevStat->SetBit(kCanDelete);

  TH2D *PhiDev2D = new TH2D("PhiDev2D", "PhiDev2D", NBinsL, PhiMin, PhiMax, NBinsH, -PhiMax, PhiMax);
  PhiDev2D->SetBit(kCanDelete);
  //TH1D *PhiDev1D = new TH1D("PhiDev1D", "PhiDev1D", NBinsL, PhiMin, PhiMax);

  //TH1D *PhiDevEnergy = new TH1D("Standard deviation in #varphi vs. energy", "Standard deviation of #varphi vs. energy", NBinsE, EMin, EMax);
  TH2D *PhiDevEnergyStat = new TH2D("PhiDevEnergyStat", "PhiDevEnergyStat", NBinsE, EMin, EMax, NBinsH, -PhiMax, PhiMax);
  PhiDevEnergyStat->SetBit(kCanDelete);

  TH2D *EDevEnergyStat = new TH2D("EDevEnergyStat", "EDevEnergyStat", NBinsE, EMin, EMax, 10*NBinsE, -1000, 1000);
  EDevEnergyStat->SetBit(kCanDelete);

  TH1D *EiEe = new TH1D("EiEe", "EiEe", NBinsE*2, -100, +100);
  EiEe->SetBit(kCanDelete);
  TH1D *EiEg = new TH1D("EiEg", "EiEg", NBinsE*2, -100, +100);
  EiEg->SetBit(kCanDelete);

  TH1D *Profile = new TH1D("Compton-Profile", "Compton-Profile", NBins, -20, 20);
  Profile->SetBit(kCanDelete);

  TH2D *iPhirPhi = new TH2D("#varphi_{ideal} vs. #varphi_{real}", "#varphi_{ideal} vs. #varphi_{real}", NBins, 0, PhiMax, NBins, 0, PhiMax);
  iPhirPhi->SetBit(kCanDelete);
  
  TH2D *Test = new TH2D("Test", "Test", NBinsE, PhiMin, PhiMax, NBinsE, -100, +100);
  Test->SetBit(kCanDelete);

  // Open the simulation file:
  MFileEventsSim* EventFile = new MFileEventsSim(m_Geometry);
  if (EventFile->Open(m_Data->GetCurrentFileName()) == false) return;
  EventFile->ShowProgress();

  while ((Event = EventFile->GetNextEvent()) != 0) {
    
    if (Event->GetNIAs() > 2) {
      if (Event->GetIAAt(1)->GetProcess() == "COMP" && Event->GetIAAt(2)->GetProcess() != "BREM") {
        for (unsigned int i = 0; i < Event->GetNHTs(); i++) {
          if (Event->GetHTAt(i)->GetNOrigins() == 1 && 
              Event->GetHTAt(i)->GetOriginAt(0) == 2 &&
              (Event->GetICEnergyElectron()+Event->GetICEnergyGamma() - (Event->GetRCEnergyElectron()+Event->GetRCEnergyGamma())) < 0.1) {


            iEe = Event->GetICEnergyElectron();
            iEg = Event->GetICEnergyGamma();
            //rEe = Event->GetRCEnergyElectron();
            //rEg = Event->GetRCEnergyGamma();

            // Compute the ideal scatter angle by the interaction directions ...
            //iPhi = Event->GetICOrigin().Angle(Event->GetICSecondIA() - Event->GetICFirstIA())*c_Deg;
            iPhi = Event->GetICOrigin().Angle(Event->GetICPhotonD())*c_Deg;

            // ... and the "real"/measured via the energies:
            rPhi = 1 - 511.004 * (1/iEg - 1/(iEe + iEg));
            if (rPhi <= -1 || rPhi >= 1) {
              //cout<<"Event incompatible with Compton scattering!"<<endl;
              break;
            }


            // ????
            dE = iEg - 511.004/(1+ 511.004/Event->GetICEnergy() - cos(iPhi*c_Rad));
            //cout<<iEg<<"!"<<iEe<<"!"<<Event->GetICEnergy()<<"!"<<511.004/(1 + 511.004/Event->GetICEnergy() - cos(iPhi*c_Rad))<<endl;
            
            rPhi = acos(rPhi)*c_Deg;

            //if (rPhi > 55 || rPhi < 35) break; 
            // 
            iPhirPhi->Fill(iPhi, rPhi, 1);

            //
            Profile->Fill((iPhi-rPhi), 1);

            // 
            PhiDevStat->Fill(iPhi, 1);
            PhidPhiStdDev->Fill(iPhi, (rPhi-iPhi)*(rPhi-iPhi));
            PhiDev2D->Fill(iPhi, rPhi-iPhi, 1);

            // 
            //PhiDevEnergyStat->Fill(iEe+iEg, 1);
            PhiDevEnergyStat->Fill(iEe+iEg, rPhi-iPhi, 1);
            //cout<<"R:"<<rPhi<<" I:"<<iPhi<<" m:"<<(rPhi-iPhi)*(rPhi-iPhi)<<endl;

            EDevEnergyStat->Fill(iEe+iEg, dE, 1);

            // Equivalent energy resolution
            dEe = (iPhi-rPhi)*c_Rad*sin(iPhi*c_Rad)*(iEe+iEg)*(iEe+iEg)/511.004;
            cout<<(iPhi-rPhi)<<"!"<<sin(iPhi)<<"!"<<(iEe+iEg)*(iEe+iEg)<<"!"<<511.004;
            dEg = (iPhi-rPhi)*c_Rad*sin(iPhi*c_Rad)/511.004 /(1.0/((iEe+iEg)*(iEe+iEg)) - 1.0/(iEg*iEg));

            EiEe->Fill(dEe, 1);
            EiEg->Fill(dEg, 1);

            Test->Fill(rPhi-iPhi, dEe, 1);
          }
        }
      } else {
        //cout<<"Event is no Compton event: "<<Event->GetIAAt(1)->GetProcess()<<endl;
      }
    } 

    delete Event;
  }

  
  // Do the plots:
  // =============



  // Plot the Phi_ideal - Phi_real distribution
  TCanvas *CiPhirPhi = new TCanvas("Phi IdealReal", "Phi IdealReal", 480, 480);
  iPhirPhi->Draw("CONT0Z");
  CiPhirPhi->Update();



  // Plot phi-distribution calculated via standard deviation:
  for (int i = 1; i <= NBinsL; i++) {
    if (PhiDevStat->GetBinContent(i) > 1) {
      PhidPhiStdDev->SetBinContent(i, sqrt(1.0/(PhiDevStat->GetBinContent(i)-1)*PhidPhiStdDev->GetBinContent(i)));
    } else {
      PhidPhiStdDev->SetBinContent(i, 0);
    }
  }
  TCanvas *CPhidPhiStdDev = new TCanvas("PhidPhi std. dev.", "PhidPhi std.dev.", 480, 320);
  //CPhidPhiStdDev->cd();
  PhidPhiStdDev->Draw();
  //CPhidPhiStdDev->Modified();
  CPhidPhiStdDev->Update();



  // Plot the Compton-Profile:
  TCanvas *CProfile = new TCanvas("Compton-Profile", "Profile", 480, 320);
  //L->SetParameters(1,1);
  //Profile->Fit("Lorentz2", "r");
  Profile->Draw();
  CProfile->Update();

  
//   // Plot phi-distribution calculated via lorentzians:
//   for (i = 1; i < NBinsL; i++) {
//     L->SetParameters(1,1);
//     Proj = PhiDev2D->ProjectionY("Px", i, i+1);
//     if (Proj->GetSum() > MinFit) {
//       Proj->Fit("Lorentz2", "r");
//       PhiDev1D->SetBinContent(i, L->GetParameter("Width"));
//     } else {
//       PhiDev1D->SetBinContent(i, 0);
//     }
//     delete Proj;
//   }
  
  
//   PhiDev1D->SetMinimum(0);
//   PhiDev1D->SetMaximum(10);
//   TCanvas *CPhidPhiLorentz = new TCanvas("PhidPhi lorentz", "PhidPhi lorentz", 480, 320);
//   PhiDev1D->Draw();
//   CPhidPhiLorentz->Update();

//   CProfile = new TCanvas("Compton-Profile", "Profile", 480, 320);
//   L->SetParameters(1,1);
//   Profile->Fit("Lorentz2", "r");
//   Profile->Draw();
//   CProfile->Update();

//   // Plot energy/phi-distribution via standard deviation:
//   /*
//   for (i = 1; i <= NBinsE; i++) {
//     if (PhiDevEnergyStat->GetBinContent(i) > 1) {
//       PhiDevEnergy->SetBinContent(i, sqrt(1.0/(PhiDevEnergyStat->GetBinContent(i)-1)*PhiDevEnergy->GetBinContent(i)));
//     } else {
//       PhiDevEnergy->SetBinContent(i, 0);
//     }
//   }
//   TCanvas *CPhiDevEnergy = new TCanvas("PhiDevEnergy", "PhiDevEnergy", 480, 320);
//   PhiDevEnergy->Draw();
//   */


//   // Plot energy/phi-distribution via lorentzians:
//   for (i = 1; i <= NBinsE; i++) {
//     L->SetParameters(1,1);
//     Proj = PhiDevEnergyStat->ProjectionY("Px", i, i);
//     if (Proj->GetSum() > MinFit) {
//       Proj->Fit("Lorentz2", "r");
//       PhiDevEnergy->SetBinContent(i, L->GetParameter("Width"));
//     } else {
//       PhiDevEnergy->SetBinContent(i, 0);
//     }
//     delete Proj;
//   }
//   PhiDevEnergy->SetMinimum(0);
//   PhiDevEnergy->SetMaximum(5);
//   TCanvas *CPhiDevEnergy = new TCanvas("dPhi (lorentz) via Energy", "dPhi (lorentz) via Energy", 480, 320);
//   PhiDevEnergy->Draw();



//   CProfile = new TCanvas("Compton-Profile", "Profile", 480, 320);
//   L->SetParameters(1,1);
//   Profile->Fit("Lorentz2", "r");
//   Profile->Draw();
//   CProfile->Update();

//   // Plot the energy "error"
//   TCanvas *CEDevEnergy = new TCanvas("EDevEnergy", "EDevEnergy", 480, 320);
//   for (i = 1; i <= NBinsE; i++) {
//     N = 0;
//     for (j = 0; j <= NBinsH; j++) {
//       N += EDevEnergyStat->GetBinContent(i, j);
//     }
//     for (j = 0; j <= NBinsH; j++) {
//       if (N != 0) {
//         EDevEnergyStat->SetBinContent(i, j, EDevEnergyStat->GetBinContent(i, j)/N);
//       }
//     }
//   }
//   EDevEnergyStat->Draw();

  TCanvas *CEiEe = new TCanvas("EiEe", "EiEe", 480, 320);
  CEiEe->cd();
  EiEe->Fit("Lorentz2", "r");
  EiEe->Draw();

  // Calculate the 66 % containment range:
  double Level = 0.66;
  int AllHits = (int) EiEe->Integral();
  int Left = EiEe->GetMaximumBin()-1;
  int Right = EiEe->GetMaximumBin()+1;
  int CurrentHits = (int) EiEe->GetBinContent(EiEe->GetMaximumBin());
  
  while (CurrentHits < AllHits*Level) {
    if (EiEe->GetBinContent(Left) > EiEe->GetBinContent(Right)) {
      CurrentHits += (int) EiEe->GetBinContent(Left);
      Left--;
    } else {
      CurrentHits += (int) EiEe->GetBinContent(Right);
      Right++;
    }
  }

  cout<<"EiEe - Interval: "<<(Right-Left-1)*EiEe->GetXaxis()->GetBinWidth(1)<<endl;


  TCanvas *CEiEg = new TCanvas("EiEg", "EiEg", 480, 320);
  CEiEg->cd();
  EiEg->Fit("Lorentz2", "r");
  EiEg->Draw();
  // Calculate the 66 % containment range:
  Level = 0.66;
  AllHits = (int) EiEg->Integral();
  Left = EiEg->GetMaximumBin()-1;
  Right = EiEg->GetMaximumBin()+1;
  CurrentHits = (int) EiEg->GetBinContent(EiEg->GetMaximumBin());
  
  while (CurrentHits < AllHits*Level) {
    if (EiEg->GetBinContent(Left) > EiEg->GetBinContent(Right)) {
      CurrentHits += (int) EiEg->GetBinContent(Left);
      Left--;
    } else {
      CurrentHits += (int) EiEg->GetBinContent(Right);
      Right++;
    }
  }

  cout<<"EiEg - Interval: "<<(Right-Left-1)*EiEg->GetXaxis()->GetBinWidth(1)<<endl;

  // Calculate the 66 % containment range:
  Level = 0.66;
  AllHits = (int) Profile->Integral();
  Left = Profile->GetMaximumBin()-1;
  Right = Profile->GetMaximumBin()+1;
  CurrentHits = (int) Profile->GetBinContent(Profile->GetMaximumBin());
  
  cout<<CurrentHits<<"!"<<Left<<"!"<<Right<<endl;
  while (CurrentHits < AllHits*Level) {
    if (Profile->GetBinContent(Left) > Profile->GetBinContent(Right)) {
      CurrentHits += (int) Profile->GetBinContent(Left);
      Left--;
    } else {
      CurrentHits += (int) Profile->GetBinContent(Right);
      Right++;
    }
    cout<<CurrentHits<<"!"<<Left<<"!"<<Right<<endl;
  }

  
  cout<<"Profile - Interval: "<<(Right-Left-1)*Profile->GetXaxis()->GetBinWidth(1)<<endl;
//   TCanvas *CTest = new TCanvas("Test", "Test", 480, 320);
//   Test->Draw();
}


////////////////////////////////////////////////////////////////////////////////


void MInterfaceSivan::SumDeposits()
{
  // Sum up all deposits in all events - however the might look like
  double Energy = 0;
  MSimEvent* Event;

  // Open the simulation file:
  MFileEventsSim* EventFile = new MFileEventsSim(m_Geometry);
  if (EventFile->Open(m_Data->GetCurrentFileName()) == false) return;
  EventFile->ShowProgress();

  while ((Event = EventFile->GetNextEvent()) != 0) {
    for (unsigned int i = 0; i < Event->GetNHTs(); i++) {
      Energy += Event->GetHTAt(i)->GetEnergy();
    }
  }

  cout<<"Total Energy: "<<Energy<<endl;
}


////////////////////////////////////////////////////////////////////////////////


void MInterfaceSivan::PitchAnalysis()
{
  // Analyze different D1 pitches:
  
  int NFiles = 12;
  const char *FileList[12] = {
    "/home/andi/Bildrekonstruktion/Programme/GMega02/Pitch_low.sim",
    "/home/andi/Bildrekonstruktion/Programme/GMega03/Pitch_low.sim",
    "/home/andi/Bildrekonstruktion/Programme/GMega04/Pitch_low.sim", 
    "/home/andi/Bildrekonstruktion/Programme/GMega05/Pitch_low.sim",
    "/home/andi/Bildrekonstruktion/Programme/GMega06/Pitch_low.sim", 
    "/home/andi/Bildrekonstruktion/Programme/GMega07/Pitch_low.sim", 
    "/home/andi/Bildrekonstruktion/Programme/GMega08/Pitch_low.sim",
    "/home/andi/Bildrekonstruktion/Programme/GMega09/Pitch_low.sim",
    "/home/andi/Bildrekonstruktion/Programme/GMega10/Pitch_low.sim",
    "/home/andi/Bildrekonstruktion/Programme/GMega12/Pitch_low.sim",
    "/home/andi/Bildrekonstruktion/Programme/GMega15/Pitch_low.sim",
    "/home/andi/Bildrekonstruktion/Programme/GMega20/Pitch_low.sim" };
  
  const char *GeoList[12] = {
    "/home/andi/Bildrekonstruktion/Programme/MIWorks/examples/Pitch02.geo.setup",
    "/home/andi/Bildrekonstruktion/Programme/MIWorks/examples/Pitch03.geo.setup",
    "/home/andi/Bildrekonstruktion/Programme/MIWorks/examples/Pitch04.geo.setup",
    "/home/andi/Bildrekonstruktion/Programme/MIWorks/examples/Pitch05.geo.setup",
    "/home/andi/Bildrekonstruktion/Programme/MIWorks/examples/Pitch06.geo.setup",
    "/home/andi/Bildrekonstruktion/Programme/MIWorks/examples/Pitch07.geo.setup",
    "/home/andi/Bildrekonstruktion/Programme/MIWorks/examples/Pitch08.geo.setup",
    "/home/andi/Bildrekonstruktion/Programme/MIWorks/examples/Pitch09.geo.setup",
    "/home/andi/Bildrekonstruktion/Programme/MIWorks/examples/Pitch10.geo.setup",
    "/home/andi/Bildrekonstruktion/Programme/MIWorks/examples/Pitch12.geo.setup",
    "/home/andi/Bildrekonstruktion/Programme/MIWorks/examples/Pitch15.geo.setup",
    "/home/andi/Bildrekonstruktion/Programme/MIWorks/examples/Pitch20.geo.setup" };

  double Pitches[12] = { 0.02, 0.03, 0.04, 0.05, 0.06, 0.07, 0.08, 0.09, 0.10, 0.12, 0.15, 0.20 };
  //double Pitches[12] = { 0.02, 0.03, 0.04, 0.05, 0.06, 0.07, 0.08, 0.09, 0.10, 0.11, 0.12, 0.13 };
  

  /*
  char *FileList[12] = {
    "/home/andi/Bildrekonstruktion/Programme/GMega02/Pitch_high.sim",
    "/home/andi/Bildrekonstruktion/Programme/GMega03/Pitch_high.sim",
    "/home/andi/Bildrekonstruktion/Programme/GMega04/Pitch_high.sim", 
    "/home/andi/Bildrekonstruktion/Programme/GMega05/Pitch_high.sim",
    "/home/andi/Bildrekonstruktion/Programme/GMega06/Pitch_high.sim", 
    "/home/andi/Bildrekonstruktion/Programme/GMega07/Pitch_high.sim", 
    "/home/andi/Bildrekonstruktion/Programme/GMega08/Pitch_high.sim",
    "/home/andi/Bildrekonstruktion/Programme/GMega09/Pitch_high.sim",
    "/home/andi/Bildrekonstruktion/Programme/GMega10/Pitch_high.sim",
    "/home/andi/Bildrekonstruktion/Programme/GMega12/Pitch_high.sim",
    "/home/andi/Bildrekonstruktion/Programme/GMega15/Pitch_high.sim",
    "/home/andi/Bildrekonstruktion/Programme/GMega20/Pitch_high.sim" };
  */
  
  /*
  // Test:
  int NFiles = 1;
  char *FileList[1] = {
    "/home/andi/Bildrekonstruktion/Programme/GMega20/Pitch_low.sim" };
  char *GeoList[1] = {
    "/home/andi/Bildrekonstruktion/Programme/MIWorks/examples/Pitch20.geo.setup" };

  //double Pitches[12] = { 0.02, 0.03, 0.04, 0.05, 0.06, 0.07, 0.08, 0.09, 0.10, 0.12, 0.15, 0.20 };
  double Pitches[1] = { 0.20 };
  */


  int MaxNEvents = 200000;

  int NPitchBins = 20;
  double PitchMin = 0.005;
  double PitchMax = 0.205;

  int NEeBins = 7;
  double EeMin = 500.0;
  double EeMax = 4000.0;

  int NPhiBins = 100;
  double PhiMin = -2.0;
  double PhiMax = 2.0;

  // Histogram dEe(Ee, pitch) dEe
  TH2D* dEeHist = new TH2D("dEe", "dEe", NPitchBins, PitchMin, PitchMax, NEeBins, EeMin, EeMax);
  dEeHist->SetBit(kCanDelete);
  TH2D* dEeHistAux = new TH2D("dEe - aux", "dEe - aux", NPitchBins, PitchMin, PitchMax, NEeBins, EeMin, EeMax);
  dEeHistAux->SetBit(kCanDelete);

  // Histogram dMoliere(Ee, pitch)
  TH2D* dMoliereHist = new TH2D("dMoliere", "dMoliere", NPitchBins, PitchMin, PitchMax, NEeBins, EeMin, EeMax);
  dMoliereHist->SetBit(kCanDelete);
  TH2D* dMoliereHistAux = new TH2D("dMoliere - aux", "dMoliere - aux", NPitchBins, PitchMin, PitchMax, NEeBins, EeMin, EeMax);
  dMoliereHistAux->SetBit(kCanDelete);

  // ClusterSize
  TH2D* ClusterHist = new TH2D("Clusters", "Clusters", NPitchBins, PitchMin, PitchMax, NEeBins, EeMin, EeMax);
  ClusterHist->SetBit(kCanDelete);
  TH2D* ClusterHistAux = new TH2D("Clusters - aux", "Clusters - aux", NPitchBins, PitchMin, PitchMax, NEeBins, EeMin, EeMax);
  ClusterHistAux->SetBit(kCanDelete);

  // Energy deposit
  TH2D* EDepositHist = new TH2D("Energy deposit", "Energy deposit", NPitchBins, PitchMin, PitchMax, NEeBins, EeMin, EeMax);
  EDepositHist->SetBit(kCanDelete);
  TH2D* EDepositHistAux = new TH2D("Energy deposit - aux", "Energy deposit - aux", NPitchBins, PitchMin, PitchMax, NEeBins, EeMin, EeMax);
  EDepositHistAux->SetBit(kCanDelete);

  // traversed layers
  TH2D* LayersHist = new TH2D("Layers", "Layers", NPitchBins, PitchMin, PitchMax, NEeBins, EeMin, EeMax);
  LayersHist->SetBit(kCanDelete);
  TH2D* LayersHistAux = new TH2D("Layers - aux", "Layers - aux", NPitchBins, PitchMin, PitchMax, NEeBins, EeMin, EeMax);
  LayersHistAux->SetBit(kCanDelete);

  // ARM D1 only
  TH2D* ARMHist = new TH2D("ARM", "ARM", NPitchBins, PitchMin, PitchMax, NEeBins, EeMin, EeMax);
  ARMHist->SetBit(kCanDelete);
  TH3D* ARM3DHist = new TH3D("ARM 3D", "ARM 3D", NPitchBins, PitchMin, PitchMax, NEeBins, EeMin, EeMax, NPhiBins, PhiMin, PhiMax);
  ARM3DHist->SetBit(kCanDelete);


  MSimEvent *Event = 0;
  MFileEventsSim *EventFile = 0;
  MDGeometryQuest *Geo = 0;
  bool MoreEvents;
  int NEvents;

  double rEe, iEe, iEg, Angle, phi, ARM;

  for (int f = 0; f < NFiles; f++) {
    cout<<"Analyzing file: "<<FileList[f]<<" with pitch: "<<Pitches[f]<<endl;

    // Load the geometry:
    if (Geo != 0) delete Geo;
    Geo = new MDGeometryQuest();
    if (Geo->ScanSetupFile(GeoList[f]) == true) {
    } else {
      delete Geo;
      Geo = 0;
    }

    // Open the file:
    if (EventFile != 0) delete EventFile;
    EventFile = new MFileEventsSim(Geo);
    if (EventFile->Open(FileList[f]) == false) {
    }
    EventFile->ShowProgress();

    MoreEvents = true;
    NEvents = 0;

    while (MoreEvents == true && NEvents < MaxNEvents) {
      if (Event != 0) delete Event;
      if ((Event = EventFile->GetNextEvent()) == 0) {
        MoreEvents = false;
        break;
      }


      // Determine whether the event is retrievable
      if (Event->GetIEventRetrieval() == MSimEvent::IdealRetrievalBad || 
          Event->GetREventRetrieval() == MSimEvent::RealRetrievalBad) {
        continue;
      }

      // Only analyze compton events ...
      if (Event->GetEventType() == MSimEvent::Compton) {

        // ... which have only one interaction in D1
        if (Event->GetICFirstIADetector() == 1 && Event->GetICSecondIADetector() == 1) {
          continue;
        }

        if (Event->GetEventLocation() == MSimEvent::D1) {

          // The electron has to be contained in D1:
          if (Event->IsElectronContainedInD1() == false) {
            continue;
          }

          NEvents++;

          // Store some data:
          rEe = Event->GetRCEnergyElectron();
          iEe = Event->GetICEnergyElectron();
          iEg = Event->GetICEnergyGamma();
          Angle = Event->GetICElectronD().Angle(Event->GetRCElectronD(2));
          phi = acos(1 - 511.004 * (1/iEg - 1/(rEe + iEg)));
          ARM = (Event->GetRCFirstIAClustered() - Event->GetICSecondIA()).Angle(MVector(0.0, 0.0, 10E+30)) - phi;

          // Now fill the histograms:
          dEeHist->Fill(Pitches[f], rEe, (rEe-iEe)*(rEe-iEe));
          dEeHistAux->Fill(Pitches[f], rEe, 1);

          dMoliereHist->Fill(Pitches[f], rEe, Angle*Angle);
          dMoliereHistAux->Fill(Pitches[f], rEe, 1);

          ClusterHist->Fill(Pitches[f], rEe, Event->GetAverageClusterSize(1));
          ClusterHistAux->Fill(Pitches[f], rEe, 1);

          EDepositHist->Fill(Pitches[f], rEe, Event->GetAverageEnergyDeposit(1));
          EDepositHistAux->Fill(Pitches[f], rEe, 1);

          LayersHist->Fill(Pitches[f], rEe, Event->GetLengthFirstTrack());
          LayersHistAux->Fill(Pitches[f], rEe, 1);

          ARM3DHist->Fill(Pitches[f], rEe, ARM*c_Deg, 1);

      
        } // only D1 first
      } // only Comptons
    } // while (MoreEvents == true)
  } // for all files


  // Display all the canvases

  // Histogram dEe(Ee, pitch) "dEe
  TCanvas *dEeCanvas = new TCanvas("Canvas dEe", "Canvas dEe", 800, 600);
  // Compute standard deviation:
  for (int p = 1; p <= NPitchBins; p++) {
    for (int e = 1; e <= NEeBins; e++) {
      if (dEeHistAux->GetBinContent(p, e) > 1) {
        dEeHist->SetBinContent(p, e, sqrt(1/(dEeHistAux->GetBinContent(p, e)-1) * dEeHist->GetBinContent(p, e)));
      } else {
        dEeHist->SetBinContent(p, e, 0);
      }
    }
  }  
  dEeHist->SetStats(false);
  dEeHist->Draw("LEGO2");
  dEeCanvas->SaveAs("P01_dEe.eps");
  dEeCanvas->SaveAs("P01_dEe.root");

  // Histogram Deposit probability:
  TCanvas *DepositProbabilityCanvas = new TCanvas("Canvas deposit probability", "Canvas deposit probability", 800, 600);
  DepositProbabilityCanvas->cd();
  // Compute standard deviation:
  dEeHistAux->SetStats(false);
  dEeHistAux->Draw("LEGO2Z");
  


  // Histogram dMoliere(Ee, pitch)
  TCanvas *dMoliereCanvas = new TCanvas("Canvas Moliere", "Canvas Moliere", 800, 600);
  // Compute standard deviation:
  for (int p = 1; p <= NPitchBins; p++) {
    for (int e = 1; e <= NEeBins; e++) {
      if (dMoliereHistAux->GetBinContent(p, e) > 1) {
        dMoliereHist->SetBinContent(p, e, c_Deg*sqrt(1/(dMoliereHistAux->GetBinContent(p, e)-1) * dMoliereHist->GetBinContent(p, e)));
      } else {
        dMoliereHist->SetBinContent(p, e, 0);
      }
    }
  }
  dMoliereHist->SetStats(false);
  dMoliereHist->Draw("LEGO2Z");
  dMoliereCanvas->SaveAs("P01_Moliere.eps");
  dMoliereCanvas->SaveAs("P01_Moliere.root");

  // Histogram dCluster(Ee, pitch)
  TCanvas *ClusterCanvas = new TCanvas("Canvas Cluster", "Canvas Cluster", 800, 600);
  // Compute standard deviation:
  for (int p = 1; p <= NPitchBins; p++) {
    for (int e = 1; e <= NEeBins; e++) {
      if (ClusterHistAux->GetBinContent(p, e) > 1) {
        ClusterHist->SetBinContent(p, e, ClusterHist->GetBinContent(p, e)/ClusterHistAux->GetBinContent(p, e));
      } else {
        ClusterHist->SetBinContent(p, e, 0);
      }
    }
  }
  ClusterHist->SetStats(false);
  ClusterHist->Draw("LEGO2Z");
  ClusterCanvas->SaveAs("P01_Cluster.eps");
  ClusterCanvas->SaveAs("P01_Cluster.root");


  // Histogram EDeposit(Ee, pitch)
  TCanvas *EDepositCanvas = new TCanvas("Canvas Energy Deposit", "Canvas Energy Deposit", 800, 600);
  // Compute standard deviation:
  for (int p = 1; p <= NPitchBins; p++) {
    for (int e = 1; e <= NEeBins; e++) {
      if (EDepositHistAux->GetBinContent(p, e) > 1) {
        EDepositHist->SetBinContent(p, e, EDepositHist->GetBinContent(p, e)/EDepositHistAux->GetBinContent(p, e));
      } else {
        EDepositHist->SetBinContent(p, e, 0);
      }
    }
  }
  EDepositHist->SetStats(false);
  EDepositHist->Draw("LEGO2Z");
  EDepositCanvas->SaveAs("P01_EDeposit.eps");
  EDepositCanvas->SaveAs("P01_EDeposit.root");


  // Histogram Layers(Ee, pitch)
  TCanvas *LayersCanvas = new TCanvas("Hit Layers Deposit", "Hit Layers Deposit", 800, 600);
  // Compute standard deviation:
  for (int p = 1; p <= NPitchBins; p++) {
    for (int e = 1; e <= NEeBins; e++) {
      if (LayersHistAux->GetBinContent(p, e) > 1) {
        LayersHist->SetBinContent(p, e, LayersHist->GetBinContent(p, e)/LayersHistAux->GetBinContent(p, e));
      } else {
        LayersHist->SetBinContent(p, e, 0);
      }
    }
  }
  LayersHist->SetStats(false);
  LayersHist->Draw("LEGO2Z");
  LayersCanvas->SaveAs("P01_Layers.eps");
  LayersCanvas->SaveAs("P01_Layers.root");


  // Histgram ARM(Ee, pitch):

  // For fitting:
  int MinFit = 10;
  TH1D *Proj;
  TF1 *L = new TF1("Gauss", Gauss, PhiMin*0.5, PhiMax*0.5, 3);
  L->SetParameters(1, 0, 1);
  //L->SetParLimits(0, 0, 9999999);
  //L->SetParLimits(1, 0, 9999999);
  L->SetParNames("Height", "Mean", "Sigma");

  
  TCanvas *ARMCanvas = new TCanvas("ARM (FWHM) D1 only!", "ARM (FWHM) D1 only!", 800, 600);
  const int Length = 100;
  char ID[Length];
  for (int p = 1; p <= NPitchBins; p++) {
    for (int e = 1; e <= NEeBins; e++) {
      snprintf(ID, Length, "p: %d - E: %d", e,p);
      Proj = ARM3DHist->ProjectionZ(ID, p, p, e, e);
      cout<<Proj->GetSum()<<endl;
      if (Proj->GetSum() > MinFit) {
        //Proj->Fit("Lorentz2", "r");
        Proj->Fit("Gauss", "r");
        TCanvas *PCan = new TCanvas(ID, ID, 640, 480);
        Proj->Draw("HIST");
        PCan->Update();
        //return;
        ARMHist->SetBinContent(p, e, 2.35*L->GetParameter("Sigma"));
      } else {
        ARMHist->SetBinContent(p, e, 0);
      }
      //delete Proj;
    }
  }
  ARMHist->SetStats(false);
  ARMHist->Draw("LEGO2Z");
  ARMCanvas->SaveAs("P01_ARM.eps");
  ARMCanvas->SaveAs("P01_ARM.root");

  return;
}


////////////////////////////////////////////////////////////////////////////////


void MInterfaceSivan::ChanceCoincidences()
{
  // Noise the data and collect chance coincidences
  // Write the data as "simn"

  cout<<"Analysing chance coincidences!"<<endl;

  const double TimeSpan = 0.01; // s
  //const double DeadTime = 0.01; // s
  const double FrontTime = 0.000005; // s
  const double BackTime = 0.000001; // s
  const double ShapeTime = 0.00003; // s
  const double CascadeTime = 0.000000001; // s

  //const int TriggerType = 12;


  // Open the simulation file:
  MFileEventsSim* EventFile = new MFileEventsSim(m_Geometry);
  if (EventFile->Open(m_Data->GetCurrentFileName()) == false) return;
  EventFile->ShowProgress();

  int NCasc = 0;
  //int NKoinzCasc = 0;

  int NInShapeTime = 0;
  int NOutShapeTime = 0;

  int NSingleTrig = 0;

  int NKoinz = 0;
  int NKoinzReal = 0;
  int NKoinzTime = 0;

  int NKoinzRealAndShape = 0;
  int NKoinzTimeAndShape = 0;

  bool HasTriggered = false;
  bool NoMoreEvents = false;  // dito

  double FirstTime = 0;
  double AverageEventDistance = 0;

  MSimEvent* Event = 0;
  MSimEvent* LastEvent = 0;
  list<MSimEvent*> History; // store some events...
  list<MSimEvent*>::iterator Iter;
  list<MSimEvent*>::iterator IterCheck;

  list<MSimEvent*> TimeCoincidence; // store some events...
  list<MSimEvent*> ShapeCoincidence; // store some events...

  // Display the data:
  double Energy = 0;
  TCanvas* SpectrumCanvas = new TCanvas("SpectrumC", "SpectrumC", 0, 0, 640, 480);
  TH1D* Spectrum = new TH1D("Spectrum", "Spectrum", 50, 0, 3000);
  Spectrum->SetBit(kCanDelete);

  // Start filling the history with data:
  while (true) {

    // Get next event:
    Event = EventFile->GetNextEvent();

    // If there are still events available
    if (Event != 0) {
      NSingleTrig++;

      // Check and count if the event is within the pulse shape of the last:
      if (History.size() > 0) {
        if (Event->GetTime() - History.back()->GetTime() < ShapeTime) {
          NInShapeTime++;
          if (Event->GetTime() - History.back()->GetTime() < CascadeTime) {
            NCasc++;
          }
        } else {
          NOutShapeTime++;
        }
      }

      // Compute average time:
      if (NSingleTrig > 1) {
        AverageEventDistance = (Event->GetTime().GetAsSeconds() - FirstTime) / (NSingleTrig - 1);
      } else {
        FirstTime = Event->GetTime().GetAsSeconds();
      }

      // Add events as until we cover the time span TimeSpan in out history
      History.push_back(Event);
      if (History.back()->GetTime() - History.front()->GetTime() < TimeSpan) {
        continue; // i.e. get next event
      }
    } else {
      NoMoreEvents = true;
    }


    // Check for a trigger:
    HasTriggered = false;
    for (Iter = History.begin(); Iter != History.end(); Iter++) {

      //cout<<"Look at:"<<(*Iter)->GetID()<<endl;
      
      // Check if we have still enough events in front of us:
      if (History.back()->GetTime() - (*Iter)->GetTime() < 0.1*TimeSpan) {
        if (NoMoreEvents == false) {
          LastEvent = *Iter;
          break; // end "check-trigger"-for loop
        }
      }
      
      if ((*Iter)->GetRNHitsD1() > 0) {
        
        HasTriggered = false;
        
        // Check back-time "back" for coincident events
        for (IterCheck = Iter; IterCheck != History.begin(); IterCheck--) {
          if ((*IterCheck) == (*Iter)) continue;
          if ((*Iter)->GetTime() - (*IterCheck)->GetTime() < BackTime) {
            TimeCoincidence.push_back((*IterCheck));
            cout<<"Back: "<<(*IterCheck)->GetID()<<"!"<<(*IterCheck)->GetREnergyD1() + (*IterCheck)->GetREnergyD2()<<endl;
          } else if ((*Iter)->GetTime() - (*IterCheck)->GetTime() < ShapeTime) {
            ShapeCoincidence.push_back((*IterCheck));
            cout<<"Shape: "<<(*IterCheck)->GetID()<<"!"<<(*IterCheck)->GetREnergyD1() + (*IterCheck)->GetREnergyD2()<<endl;
          } else {
            break;
          }
        }
        // ... and forward
        for (IterCheck = Iter; IterCheck != History.end(); IterCheck++) {
          if ((*IterCheck) == (*Iter)) continue;
          if ((*IterCheck)->GetTime() - (*Iter)->GetTime() < FrontTime) {
            TimeCoincidence.push_back((*IterCheck));
            cout<<"Front: "<<(*IterCheck)->GetID()<<"!"<<(*IterCheck)->GetREnergyD1() + (*IterCheck)->GetREnergyD2()<<endl;
          } else {
            break;
          }
        }

        if ((*Iter)->GetRNHitsD1() > 0 && (*Iter)->GetRNHitsD2() > 0) {
          if (ShapeCoincidence.size() > 0) {
            NKoinzRealAndShape++;
          } else {
            NKoinzReal++;
          }
        }
        TimeCoincidence.push_back(*Iter); 

        // Check if we have 1p1p coincidence:
        for (IterCheck = TimeCoincidence.begin(); IterCheck != TimeCoincidence.end(); IterCheck++) {
          if ((*IterCheck)->GetRNHitsD2() > 0) {
            HasTriggered = true;
            cout<<"Trigger: "<<(*Iter)->GetID()<<" and "<<(*IterCheck)->GetID()<<endl;
            break;
          }
        }
        

        if (HasTriggered == true) {

          NKoinz++;

          if (!((*Iter)->GetRNHitsD1() > 0 && (*Iter)->GetRNHitsD2() > 0)) {
            if (ShapeCoincidence.size() > 0) {
              NKoinzTimeAndShape++;
            } else {
              NKoinzTime++;
            }
          }

          Energy = 0;

          for (IterCheck = TimeCoincidence.begin(); IterCheck != TimeCoincidence.end(); IterCheck++) {
//             cout<<"Dabei: "<<(*IterCheck)->GetID()<<" with "
//                 <<(*IterCheck)->GetREnergyD1() + (*IterCheck)->GetREnergyD2()
//                 <<" and "<<(*IterCheck)->GetTime()<<endl;
            Energy += (*IterCheck)->GetREnergyD1() + (*IterCheck)->GetREnergyD2();
          }

          for (IterCheck = ShapeCoincidence.begin(); IterCheck != ShapeCoincidence.end(); IterCheck++) {
            double E = 0;
            for (unsigned int h = 0; h < (*IterCheck)->GetNHTs(); ++h) {
              double HitEnergy = (*IterCheck)->GetHTAt(h)->GetEnergy();
              MVector HitPosition = (*IterCheck)->GetHTAt(h)->GetPosition();
              m_Geometry->ApplyPulseShape((*Iter)->GetTime().GetAsSeconds() - (*IterCheck)->GetTime().GetAsSeconds(), HitPosition, HitEnergy);
              E += HitEnergy;
            }
            Energy += E;
//             cout<<"Bei shape dabei: "<<(*IterCheck)->GetID()<<" with "
//                 <<E<<" and "<<(*IterCheck)->GetTime()<<endl;
          }
//           cout<<"Energy: "<<Energy<<endl;

          SpectrumCanvas->cd();
          Spectrum->Fill(Energy);
          Spectrum->Draw();
          SpectrumCanvas->Update();
          
          // Energy from pulse-shape coincidence...
          
          // Dump the data
          
        }
          
        TimeCoincidence.resize(0);
        ShapeCoincidence.resize(0);
        
      } // check for trigger loop after D1
        
    } // check history

    // Clean up history
    if (LastEvent != 0) { 
      while (LastEvent->GetTime() - History.front()->GetTime() > ShapeTime) {
        Event = History.front();
        History.pop_front();
        delete Event;
        if (History.size() == 0) break;
      }
      LastEvent = 0;
    } else {
      break;
    }
    cout<<"Size: "<<History.size()<<endl;

  } // loop until no events are left

  // Clear history...
  for (Iter = History.begin(); Iter != History.end(); Iter++) {
    delete (*Iter);
  }

  // Summary:
  cout<<"Summary: "<<endl;
  cout<<endl;
  cout<<"NInShapeTime:                 "<<NInShapeTime<<endl;
  cout<<"NOutShapeTime:                "<<NOutShapeTime<<endl;
  cout<<endl;
  cout<<"NCasc:                        "<<NCasc<<endl;
  cout<<endl;
  cout<<"Average event distance [us]:  "<<AverageEventDistance*1000000<<endl;
  cout<<endl;
  cout<<"NSingleTrig:                  "<<NSingleTrig<<endl;
  cout<<"NKoinz:                       "<<NKoinz<<" - "<<100.0*NKoinz/NSingleTrig<<"%"<<endl;
  cout<<"NKoinzReal:                   "<<NKoinzReal<<" - "<<100.0*NKoinzReal/NSingleTrig<<"%"<<endl;
  cout<<"NKoinzTime:                   "<<NKoinzTime<<" - "<<100.0*NKoinzTime/NSingleTrig<<"%"<<endl;
  cout<<"NKoinzRealAndShape:           "<<NKoinzRealAndShape<<" - "<<100.0*NKoinzRealAndShape/NSingleTrig<<"%"<<endl;
  cout<<"NKoinzTimeAndShape:           "<<NKoinzTimeAndShape<<" - "<<100.0*NKoinzTimeAndShape/NSingleTrig<<"%"<<endl;

}


////////////////////////////////////////////////////////////////////////////////


void MInterfaceSivan::Moliere()
{
  // The input has to be file containing only electron tracks...

  // Open the simulation file:
  MFileEventsSim* EventFile = new MFileEventsSim(m_Geometry);
  if (EventFile->Open(m_Data->GetCurrentFileName()) == false) return;
  EventFile->ShowProgress();

  MVector Ideal;
  MVector Real;

  double NEntries = 0;
  TH1D* Moliere = new TH1D("M", "M", 1440, -180, 180);
  Moliere->SetBit(kCanDelete);

  TH2D* Scatter = new TH2D("MS", "MS", 41, -3, 3, 41, -3, 3);
  Scatter->SetBit(kCanDelete);

  MSimEvent* Event;
  while ((Event = EventFile->GetNextEvent()) != 0) {
    // The real direction of the electron can be found form the IA information
    if (Event->GetNIAs() > 0) {
      Ideal = Event->GetIAAt(0)->GetSecondaryDirection();
      
      if (Event->GetNHTs() > 2) {
        if (fabs(Event->GetHTAt(1)->GetPosition().Z() - Event->GetHTAt(0)->GetPosition().Z()) > 0.5) {
          Real = Event->GetHTAt(1)->GetPosition() - Event->GetHTAt(0)->GetPosition();
          Ideal[1] = 0.0;
          Real[1] = 0.0;
          Moliere->Fill(Ideal.Angle(Real)*180/c_Pi);
          Moliere->Fill(-Ideal.Angle(Real)*180/c_Pi);
          NEntries++;

          Scatter->Fill(Event->GetHTAt(1)->GetPosition().X() - Event->GetHTAt(0)->GetPosition().X(),
                        Event->GetHTAt(1)->GetPosition().Y() - Event->GetHTAt(0)->GetPosition().Y());
//           if (Ideal.Angle(Real)*180/c_Pi > 90) {
//             cout<<Event->ToString()<<endl;
//           }
        }
      }
    }

    delete Event;
  }
  
  // Determine 68.27% containment:
  double Sum = 0;
  for (int b = 1; b <= Moliere->GetNbinsX(); ++b) {
    Sum += Moliere->GetBinContent(b);
    cout<<Sum<<"!"<<NEntries<<endl;
    if (Sum/NEntries >= 0.6837) {
      cout<<"68.37% containment reached at: "<<Moliere->GetBinCenter(b)<<endl;
      break;
    }
  }

  TCanvas* Canvas = new TCanvas("MC", "MC", 0, 0, 640, 480);
  Canvas->cd();
  Moliere->Draw();
  Canvas->Update();

  TCanvas* ScatterCanvas = new TCanvas("MCS", "MCS", 0, 480, 640, 480);
  ScatterCanvas->cd();
  Scatter->Draw("colz");
  ScatterCanvas->Update();
}


////////////////////////////////////////////////////////////////////////////////


void MInterfaceSivan::EnergyLossByMaterial()
{
  // Find the minimum and maximum start energy
  double Emin = 0;
  double Emax = 0;
  FindMinimumAndMaximumStartEnergy(Emin, Emax, 10000);
  
  Emin = 0;
  Emax *= 1.1;
  
  
  // Open the simulation file:
  MFileEventsSim EventFile(m_Geometry);
  if (EventFile.Open(m_Data->GetCurrentFileName()) == false) return;
  EventFile.ShowProgress();

  int NBins = 100;

  //! Count events with losses as a function of energy
  TH1D* NEventsWithLossesHist = new TH1D("NEventsWithLossesHist", "NEventsWithLossesHist", NBins, Emin, Emax);
  TH1D* CombinedLossHist = new TH1D("CombinedLossHist", "CombinedLossHist", NBins, Emin, Emax);
  
  map<TString, TH1D*> LossHistograms;
  TH1D* EscapeLossHist = new TH1D("EscapeLoss", "Escape", NBins, Emin, Emax);
  LossHistograms[TString("Escape")] = EscapeLossHist;
  
  map<TString, TH1I*> CountHistograms;
  TH1I* EscapeCountHist = new TH1I("EscapeCounts", "Escape", NBins, Emin, Emax);
  CountHistograms[TString("Escape")] = EscapeCountHist;
  
  MSimEvent* Event;
  while ((Event = EventFile.GetNextEvent()) != 0) {
    if (Event->GetNIAs() == 0 || Event->GetTotalEnergyDepositBeforeNoising() == 0) {
      delete Event;
      continue;
    }
    
    /* 
    //For AMEGO
    MDVolumeSequence VS = m_Geometry->GetVolumeSequence(Event->GetIAAt(1)->GetPosition());
    if (VS.GetDetector() == nullptr || VS.GetDetector()->GetName() != "SStrip") {
      delete Event;
      continue;
    }
    */
    
    bool FoundLoss = false;
    
    for (unsigned int p = 0; p < Event->GetNPMs(); ++p) {
      MSimPM* PM = Event->GetPMAt(p);
      TString Name = PM->GetMaterialName().Data();
      bool Found = false;
      for (map<TString, TH1I*>::iterator I = CountHistograms.begin(); I != CountHistograms.end(); ++I) {
        if (I->first == TString(Name.Data())) {
          Found = true;
          I->second->Fill(Event->GetTotalEnergyDepositBeforeNoising(), 1);
          FoundLoss = true;
          break;
        }
      }
      for (map<TString, TH1D*>::iterator I = LossHistograms.begin(); I != LossHistograms.end(); ++I) {
        if (I->first == TString(Name.Data())) {
          Found = true;
          I->second->Fill(Event->GetTotalEnergyDepositBeforeNoising(), PM->GetEnergy());
          CombinedLossHist->Fill(Event->GetTotalEnergyDepositBeforeNoising(), PM->GetEnergy());          
          break;
        }
      }
      if (Found == false) {
        TH1I* CountHist = new TH1I(Name + "Counts", Name, NBins, Emin, Emax);
        CountHist->Fill(Event->GetTotalEnergyDepositBeforeNoising(), 1);
        FoundLoss = true;
        CountHistograms[Name] = CountHist;
        
        TH1D* LossHist = new TH1D(Name + "Loss", Name, NBins, Emin, Emax);
        LossHist->Fill(Event->GetTotalEnergyDepositBeforeNoising(), PM->GetEnergy());
        CombinedLossHist->Fill(Event->GetTotalEnergyDepositBeforeNoising(), PM->GetEnergy());          
        LossHistograms[Name] = LossHist;
      }
    }
    
    double Energy = 0;
    for (unsigned int i = 0; i < Event->GetNIAs(); ++i) {
      if (Event->GetIAAt(i)->GetProcess() == "ESCP") {
        Energy += Event->GetIAAt(i)->GetMotherEnergy();
      }
    }
    if (Energy > 0) {
      CountHistograms[TString("Escape")]->Fill(Event->GetTotalEnergyDepositBeforeNoising(), 1);
      LossHistograms[TString("Escape")]->Fill(Event->GetTotalEnergyDepositBeforeNoising(), Energy);
      CombinedLossHist->Fill(Event->GetTotalEnergyDepositBeforeNoising(), Energy);          
    }
    
    if (FoundLoss == true) {
      NEventsWithLossesHist->Fill(Event->GetTotalEnergyDepositBeforeNoising(), 1); 
    }
    
    delete Event;
  }

  
  // Stack the Hists:
  THStack* CountStack = new THStack("CountStack", "Number of events with the given energy-loss feature");
  TLegend* CountLegend = new TLegend(0.18,0.55,0.38,0.83, NULL, "brNDC");
  
  
  unsigned int Color = 1;
  for (map<TString, TH1I*>::iterator I = CountHistograms.begin(); I != CountHistograms.end(); ++I) {
    I->second->SetFillColor(++Color);
    CountStack->Add(I->second);
    CountLegend->AddEntry(I->second, I->second->GetTitle());
  }

  // Now draw:
  TCanvas* ECanvas = new TCanvas("EnergyCanvas", "Energy Spectrum", 800, 600);
  ECanvas->cd();
  CountStack->Draw();
  CountStack->GetHistogram()->SetXTitle("Measured energy [keV]");
  CountStack->GetHistogram()->SetYTitle("Events with given energy loss feature");
  CountStack->Draw();
  CountLegend->Draw();

  
  // Stack the Hists:
  THStack* LossStack = new THStack("LossStack", "Average energy loss due to the given energy-loss feature");
  TLegend* LossLegend = new TLegend(0.18,0.55,0.38,0.83, NULL, "brNDC");
  
  Color = 1;
  
  for (map<TString, TH1D*>::iterator I = LossHistograms.begin(); I != LossHistograms.end(); ++I) {
    TH1D* LossHist = I->second;
    
    for (int b = 1; b <= NEventsWithLossesHist->GetXaxis()->GetNbins(); ++b) {
      if (NEventsWithLossesHist->GetBinContent(b) > 0) {
        LossHist->SetBinContent(b, LossHist->GetBinContent(b) / NEventsWithLossesHist->GetBinContent(b));
        LossHist->SetBinError(b, 0);
      }
    }
    
    LossHist->SetFillColor(++Color);
    LossStack->Add(LossHist);
    LossLegend->AddEntry(LossHist, LossHist->GetTitle());
  }
  
  // Now draw:
  TCanvas* LossCanvas = new TCanvas("AvgEnergyLossCanvas", "Average Energy Loss", 800, 600);
  LossCanvas->cd();
  LossStack->Draw("");
  LossStack->GetHistogram()->SetXTitle("Measured energy [keV]");
  LossStack->GetHistogram()->SetYTitle("Average energy loss [keV]");
  LossStack->Draw("");
  LossLegend->Draw();
  LossCanvas->Update();
}


////////////////////////////////////////////////////////////////////////////////


void MInterfaceSivan::HitsPerEnergy()
{
  // Open the simulation file:
  MFileEventsSim EventFile(m_Geometry);
  if (EventFile.Open(m_Data->GetCurrentFileName()) == false) return;
  EventFile.ShowProgress();

  int NBins = 10;
  double Emin = 0;
  double Emax = 5000;

  TH1D* NEvents = new TH1D("NEvents", "NEvents", NBins, Emin, Emax);
  NEvents->SetBit(kCanDelete);

  TH1D* D1Hist = new TH1D("HD1", "HD1", NBins, Emin, Emax);
  D1Hist->SetBit(kCanDelete);
  TH1D* D2Hist = new TH1D("HD2", "HD2", NBins, Emin, Emax);
  D2Hist->SetBit(kCanDelete);
  TH1D* D3Hist = new TH1D("HD3", "HD3", NBins, Emin, Emax);
  D3Hist->SetBit(kCanDelete);

  MSimEvent* Event;
  while ((Event = EventFile.GetNextEvent()) != 0) {
    int ND1 = 0;
    int ND2 = 0;
    int ND3 = 0;

    for (unsigned int i = 0; i < Event->GetNHTs(); ++i) {
      if (Event->GetHTAt(i)->GetDetectorType() == 1) {
        ND1++;
      } else if (Event->GetHTAt(i)->GetDetectorType() == 2) {
        ND2++;
      } else if (Event->GetHTAt(i)->GetDetectorType() == 3) {
        ND3++;
      }
    }
    if ((ND1 > 0 && ND2 > 0) || (ND3 > 0)) {
      for (unsigned int i = 0; i < Event->GetNHTs(); ++i) {
        if (Event->GetHTAt(i)->GetDetectorType() == 1) {
          D1Hist->Fill(Event->GetIAAt(0)->GetSecondaryEnergy(), 1);
          cout<<"D1"<<endl;
        } else if (Event->GetHTAt(i)->GetDetectorType() == 2) {
          D2Hist->Fill(Event->GetIAAt(0)->GetSecondaryEnergy(), 1);
          cout<<"D2"<<endl;
        } else if (Event->GetHTAt(i)->GetDetectorType() == 3) {
          D3Hist->Fill(Event->GetIAAt(0)->GetSecondaryEnergy(), 1);
        }
      }
      NEvents->Fill(Event->GetIAAt(0)->GetSecondaryEnergy(), 1);
    }
  }

  // Normalize:
  D1Hist->Divide(NEvents);
  D2Hist->Divide(NEvents);
  D3Hist->Divide(NEvents); 
  
  new TCanvas();
  D3Hist->Draw();
  
  new TCanvas();
  D2Hist->Draw();
  
  new TCanvas();
  D1Hist->Draw();   
}


////////////////////////////////////////////////////////////////////////////////


void MInterfaceSivan::ComptonEnergyEscape()
{
  mout<<"Calculating Compton energy escape..."<<endl;

  m_Geometry->ActivateNoising(false);

  int c_MinComptons = 2;
  int c_MinHits = 3;

  // Open the simulation file:
  MFileEventsSim EventFile(m_Geometry);
  if (EventFile.Open(m_Data->GetCurrentFileName()) == false) {
    mgui<<"Unable to open file"<<error;
    return;
  }
  EventFile.ShowProgress();

  if (EventFile.GetVersion() < 21) {
    mgui<<"You need a sim file of version 21 or higher (ConvertMGGPOD -s 21 is ok) "
        <<" to perform this functionality!"<<error;
    return;
  }

  int NCompletelyAbsorbed = 0;
  int NIncompletelyAbsorbed = 0;

  int LossStart = 0;
  int LossIntermediate = 0;
  int LossEnd = 0;

  int NGoodEscapes = 0;
  int NGoodEscapes2m = 0;
  int NGoodEscapes3 = 0;
  int NGoodEscapes4 = 0;
  int NGoodEscapes5 = 0;
  int NGoodEscapes6 = 0;
  int NGoodEscapes7 = 0;
  int NGoodEscapes8p = 0;

  MSimEvent* Event;
  while ((Event = EventFile.GetNextEvent()) != 0) {
    mdebug<<Event->GetID()<<endl;

    int NComptons = 0;
    for (unsigned int i = 1; i < Event->GetNIAs(); ++i) {
      if (Event->GetIAAt(i)->GetProcess() == "COMP") {
        NComptons++;
      } else {
        break;
      }
    }

    if (NComptons < c_MinComptons) {
      mdebug<<"Not enough Comptons: "<<NComptons<<endl;
      delete Event;
      continue;
    }

    int NHits = 0;
    vector<int> IAsInvolved;
    for (unsigned int h = 0; h < Event->GetNHTs(); ++h) {
      bool IsInvolved = false;
      for (unsigned int i = 0; i < IAsInvolved.size(); ++i) {
        if (Event->GetHTAt(h)->IsOrigin(IAsInvolved[i]) == true) {
          IsInvolved = true; 
          break;
        }
      }
      if (IsInvolved == false) {
        NHits++;
        for (unsigned int o = 0; o < Event->GetHTAt(h)->GetNOrigins(); ++o) {
          if (Event->GetHTAt(h)->GetOriginAt(o) != 1) {
            IAsInvolved.push_back(Event->GetHTAt(h)->GetOriginAt(o));
          }
        }
      }
    }

    if (NHits < c_MinHits) {
      mdebug<<"Not enough hits: "<<NHits<<endl;
      delete Event;
      continue;
    }

    vector<int> Losses;
    for (unsigned int i = 1; i < Event->GetNIAs(); ++i) {
      //if (Event->GetIAAt(i)->GetProcess() != "COMP") break;
      if (Event->GetIAAt(i)->GetOriginID() != 1 || 
          Event->GetIAAt(i)->GetProcess() == "ESCP") break;
      
      // Rename to IALoss
      int IAHasLoss = 0; // 0: no loss, 1: partly lossed, 2: total loss
      
      // Get all descendents:
      vector<int> Ids;
      Ids.push_back(Event->GetIAAt(i)->GetID());
      
      for (unsigned int g = 1; g < Event->GetNIAs(); ++g) {
        for (unsigned int a = 0; a < Ids.size(); ++a) {
          if (Event->GetIAAt(g)->GetOriginID() == Ids[a]) {
            Ids.push_back(Event->GetIAAt(g)->GetID());
            mdebug<<"New descendent: "<<Event->GetIAAt(g)->GetID()<<endl;
            break;
          }
        }
      }
      
      // Check if one of them has a single hit:
      for (unsigned int h = 0; h < Event->GetNHTs(); ++h) {
        for (unsigned int j = 0; j < Ids.size(); ++j) {
          if (Event->GetHTAt(h)->IsOrigin(Ids[j]) == true && 
              Event->GetHTAt(h)->IsOrigin(Event->GetIAAt(i)->GetID()) == false) {
            mdebug<<"Descendent has single hit"<<endl;
            IAHasLoss = 1;
            break;
          }
        }
        if (IAHasLoss != 0) break;
      }
      
      int Loss = Event->IsIACompletelyAbsorbed(Event->GetIAAt(i)->GetID());
      if (Loss > 0) IAHasLoss = Loss;
      Losses.push_back(IAHasLoss);
    }

    // Check for loss
    bool HasLoss = false;
    bool IsGoodEscape = true;
    
    // Start is missing:
    if (Losses[0] > 0) {
      for (unsigned int l = 1; l < Losses.size(); ++l) {
        if (Losses[l] == 0) {
          LossStart++;
          HasLoss = true;
          IsGoodEscape = false;
          mdebug<<"Start"<<endl;
          break;
        }
      }
    }
    
    // Intermediate is missing:
    bool FoundStart = false;
    bool CentralMissing = false;
    bool FoundEnd = false;
    
    for (unsigned int l = 0; l < Losses.size(); ++l) {
      if (FoundStart == false) {
        if (Losses[l] == 0) FoundStart = true;
      } else if (CentralMissing == false) {
        if (Losses[l] > 0) CentralMissing = true;
      } else if (FoundEnd == false) {
        if (Losses[l] == 0) {
          FoundEnd = true;
          mdebug<<"Intermediate"<<endl;
          HasLoss = true;
          IsGoodEscape = false;
          LossIntermediate++;
          break;
        }
      }
    }
    
    bool Escaped = true;
    bool Counted = false;
    for (unsigned int l = Losses.size()-1; l < Losses.size(); --l) {
      if (Event->GetIAAt(l+1)->GetProcess() == "PHOT") {
        if (Losses[l] > 0) {
          if (Counted == false) {
            HasLoss = true;
            LossEnd++;
            Counted = true;
            mdebug<<"Loss end"<<endl;
          }
          // We need a completely missing final photo effect for an escape!
          if (Losses[l] < 2) {
            Escaped = false;
          }
        } else {
          Escaped = false;
        }
      }
    }

    if (Escaped == true) {
      HasLoss = true;
      if (Counted == false) {
        LossEnd++;
        Counted = true;
        mdebug<<"Loss end"<<endl;
      }
      mdebug<<"Escaped!"<<endl;
    }

    
    if (Escaped == true && IsGoodEscape == true) {
      NGoodEscapes++;
      if (NHits <= 2) {
        mdebug<<"Is 2m!"<<endl;
        NGoodEscapes2m++;
      }
      if (NHits == 3) NGoodEscapes3++;
      if (NHits == 4) NGoodEscapes4++;
      if (NHits == 5) NGoodEscapes5++;
      if (NHits == 6) NGoodEscapes6++;
      if (NHits == 7) NGoodEscapes7++;
      if (NHits >= 8) NGoodEscapes8p++;
      mdebug<<"Good escape!"<<endl;
    }
    if (HasLoss == true) {
      NIncompletelyAbsorbed++;
    } else {
      mdebug<<"Completely absorbed!"<<endl;
      NCompletelyAbsorbed++;
    }
    delete Event;
  }
  

  mout<<endl;
  mout<<endl;
  mout<<"Loss statistics for "<<c_MinComptons<<"+ Comptons and "<<c_MinHits<<"+ hits"<<endl;
  mout<<"==========================================="<<endl;
  mout<<endl;
  mout<<"No interactions in passive material: "<<NCompletelyAbsorbed<<endl;
  mout<<"Some interactions in passive material: "<<NIncompletelyAbsorbed<<endl;
  mout<<"  Where in the Compton track happens the energy loss (multiples allowed):"<<endl;
  mout<<"    * Start: "<<LossStart<<endl;
  mout<<"    * Intermediate: "<<LossIntermediate<<endl;
  mout<<"    * End: "<<LossEnd<<endl;
  mout<<"  Lost interactions only at the end: "
      <<NGoodEscapes<<" (details: "
      <<NGoodEscapes2m<<" (2- hits), "
      <<NGoodEscapes3<<" (3 hits), "
      <<NGoodEscapes4<<" (4 hits), "
      <<NGoodEscapes5<<" (5 hits), "
      <<NGoodEscapes6<<" (6 hits), "
      <<NGoodEscapes7<<" (7 hits), "
      <<NGoodEscapes8p<<" (8+ hits) )"<<endl;
  mout<<endl;
  mout<<endl;
  mout<<"Remarks/restrictions: "<<endl;
  mout<<"  a) Deposits below the thresholds and particles travellling from "<<endl
      <<"     active to passive material are not considered!!! "<<endl;
  mout<<"  b) You need a simulation file of version 21 or higher with complete "<<endl
      <<"     interaction information (IAs or INIT2/ACT2)"<<endl;
  mout<<"  c) It is difficult to take into account all possibilities,"<<endl
      <<"     thus correctness is not guaranteed"<<endl;
  mout<<"  d) Bremsstrahlung, etc. is counted as loss if it is not stopped"<<endl
      <<"     in the same voxel as the original particle"<<endl;
  mout<<"  e) Rayleigh-scattering is completely ignored"<<endl;
  mout<<"  f) Events other than Comptons are ignored"<<endl;
  mout<<"  g) \"Start\" means the first hit has not deposited all its energy"<<endl; 
  mout<<"  h) \"Intermediate\" means somewhere in the sequence is energy missing"<<endl; 
  mout<<"  i) \"End\" means partially absorbed hits at the end as well as escapes"<<endl; 
  mout<<"  j) "<<c_MinComptons<<"+ Comptons does NOT mean we have "<<endl
      <<"     "<<c_MinComptons<<"+ detected Compton hits, but this amount of simulated Compton interactions"<<endl;
  mout<<"  k) Tracks are not considered as different hits for the \""<<c_MinHits<<"+ hits\""<<endl;
  mout<<"  l) An ideal energy resolution is assumed!"<<endl; 

  m_Geometry->ActivateNoising(true);
}


////////////////////////////////////////////////////////////////////////////////


void MInterfaceSivan::CompleteAbsorptionRatio()
{
  // 
  mout<<"Calculating complete absorption ratio..."<<endl;

  // Open the simulation file:
  MFileEventsSim EventFile(m_Geometry);
  if (EventFile.Open(m_Data->GetCurrentFileName()) == false) {
    mgui<<"Unable to open file"<<error;
    return;
  }
  EventFile.ShowProgress();

  int NBins = 50;
  double Emin = 0;
  double Emax = 2500;
  TH1D* IncompletelyHist = new TH1D("IncompletelyAbsorbed", "Incompletely absorbed", NBins, Emin, Emax);
  TH1D* CompletelyHist = new TH1D("CompletelyAbsorbed", "Completely absorbed", NBins, Emin, Emax);

  double RealEnergy;
  double RealEnergyResolution;
  double IdealEnergy;
  MSimEvent* Event;

  const unsigned int MaxHits = 100;
  vector<int> Completely(MaxHits);
  vector<int> Incompletely(MaxHits);
  vector<int> Recoverable(MaxHits);

  while ((Event = EventFile.GetNextEvent()) != 0) {
    mdebug<<Event->GetID()<<endl;

    if (Event->GetNIAs() >= 1) {
      IdealEnergy = Event->GetIAAt(0)->GetSecondaryEnergy();

      if (Event->GetNHTs() < MaxHits) {
        RealEnergy = 0;
        RealEnergyResolution = 0;
        for (unsigned int h = 0; h < Event->GetNHTs(); ++h) {
          if (Event->GetHTAt(h)->GetDetectorType() == 4) continue;
          RealEnergy += Event->GetHTAt(h)->GetEnergy();
          
          MVector PRes;
          double ERes;
          double TRes;
          m_Geometry->GetResolutions(Event->GetHTAt(h)->GetPosition(), 
                                     Event->GetHTAt(h)->GetEnergy(), 
                                     Event->GetHTAt(h)->GetTime(),
                                     *(Event->GetHTAt(h)->GetVolumeSequence()), 
                                     PRes, ERes, TRes);
          RealEnergyResolution += ERes*ERes;
        }

        if (fabs(RealEnergy - IdealEnergy) > 3*sqrt(RealEnergyResolution)) {
          IncompletelyHist->Fill(RealEnergy);
          Incompletely[Event->GetNHTs()]++;
        } else {
          CompletelyHist->Fill(RealEnergy);
          Completely[Event->GetNHTs()]++;
        }
      }
    }
    delete Event;
  }

  int Complete = 0;
  int Incomplete = 0;
  mout<<"Completely to incompletely absorbed events:"<<endl;  
  for (unsigned int i = 0; i < 20; ++i) {
    if (i > 2) {
      Complete += Completely[i];
      Incomplete += Incompletely[i];
    }
    cout<<setw(2)<<i<<":"<<setw(8)<<Completely[i]<<" : "<<setw(8)<<Incompletely[i]<<endl;
  }
  mout<<"Summary: "<<100.0*double(Complete)/(Complete+Incomplete)<<"% of 3+ are completely absorbed..."<<endl;

  TH1D* Ratio = new TH1D("Ratio", "Ratio", NBins, Emin, Emax);
  for (int b = 1; b <= Ratio->GetNbinsX(); ++b) {
    if (CompletelyHist->GetBinContent(b) != 0 && IncompletelyHist->GetBinContent(b) != 0) {
      Ratio->SetBinContent(b, CompletelyHist->GetBinContent(b)/(CompletelyHist->GetBinContent(b) + IncompletelyHist->GetBinContent(b)));
    }
  }
  TCanvas* RatioCanvas = new TCanvas();
  RatioCanvas->cd();
  Ratio->Draw();
  RatioCanvas->Update();

  mout<<endl;
  mout<<endl;
  mout<<"Remarks/restrictions: "<<endl;
  mout<<"  a) A event is considered as completely absorbed, if the "<<endl
      <<"     measured energy is within 3 sigma of the ideal energy"<<endl;

}

 
////////////////////////////////////////////////////////////////////////////////


 
void MInterfaceSivan::InitialInteraction()
{  
  //! Shows a table of initial interaction vs detector type
  
  // Open the simulation file:
  MFileEventsSim EventFile(m_Geometry);
  if (EventFile.Open(m_Data->GetCurrentFileName()) == false) {
    mgui<<"Unable to open file"<<error;
    return;
  }
  EventFile.ShowProgress();

  // A vector of interaction types (0: PHOT, 1: COMP, 2: PAIR, 3: Rest)
  vector<long> m_Types(4, 0);
  vector<MString> m_DetectorNames;
  
  vector<vector<long>> m_OccupancyMatrix;
  
  MSimEvent* Event;
  while ((Event = EventFile.GetNextEvent(false)) != 0) {
    if (Event->GetNIAs() >= 2) {
      unsigned int Type = 3;
      if (Event->GetIAAt(1)->GetProcess() == "PHOT") {
        Type = 0; 
      } else if (Event->GetIAAt(1)->GetProcess() == "COMP") {
        Type = 1; 
      } else if (Event->GetIAAt(1)->GetProcess() == "PAIR") {
        Type = 2;
      }
      
      MDVolumeSequence VS = m_Geometry->GetVolumeSequence(Event->GetIAAt(1)->GetPosition());
      if (VS.GetDetector() != nullptr) {       
        MString Name = VS.GetDetector()->GetName();
        bool Found = false;
        for (unsigned int n = 0; n < m_DetectorNames.size(); ++n) {
          if (m_DetectorNames[n] == Name) {
            m_OccupancyMatrix[n][Type]++;
            Found = true;
          }
        }
        if (Found == false) {
          m_OccupancyMatrix.push_back(m_Types);
          m_OccupancyMatrix.back()[Type]++;
          m_DetectorNames.push_back(Name);
        }
      }
    }
    delete Event;
  }
  
  // Now print the matrix:
  cout<<endl;
  cout<<endl;
  cout<<"Statistics on type and detector of the first interaction "<<endl;
  cout<<"======================================================== "<<endl;
  cout<<endl;
  
  // Determine sums and totals
  vector<long> ProcessTotals(4, 0);
  vector<long> DetectorTotals(m_DetectorNames.size(), 0);
  long Total = 0;
  for (unsigned int n = 0; n < m_DetectorNames.size(); ++n) {
    for (unsigned int i = 0; i < m_OccupancyMatrix[n].size(); ++i) {
      ProcessTotals[i] += m_OccupancyMatrix[n][i];
      DetectorTotals[n] += m_OccupancyMatrix[n][i];
      Total += m_OccupancyMatrix[n][i];
    }
  }
    
  // Determine with of detector column
  unsigned int WidthCol1 = 0;
  for (MString N: m_DetectorNames) {
    if (N.Length() > WidthCol1) WidthCol1 = N.Length();
  }
  WidthCol1 += 2;
  
  
  // Determine width of number columns
  unsigned int Width = (unsigned int) log10((double) Total) + 1;
  if (Width < 4) Width = 4;

  cout<<"As counts:"<<endl<<endl;
  cout<<setw(WidthCol1+3)<<" | "<<setw(Width)<<right<<"PHOT"<<" | "<<setw(Width)<<right<<"COMP"<<" | "<<setw(Width)<<right<<"PAIR"<<" | "<<setw(Width)<<right<<"Rest"<<" | "<<setw(Width)<<right<<" Tot"<<" |"<<endl;
  for (unsigned int i = 0; i < WidthCol1 + 5*Width + 6*3 - 1; ++i) cout<<"-";
  cout<<endl;
  for (unsigned int n = 0; n < m_DetectorNames.size(); ++n) {
    cout<<setw(WidthCol1)<<m_DetectorNames[n]<<" | ";
    for (unsigned int i = 0; i < m_OccupancyMatrix[n].size(); ++i) {
      cout<<setw(Width)<<right<<m_OccupancyMatrix[n][i]<<" | ";
    }
    cout<<setw(Width)<<right<<DetectorTotals[n]<<" | ";
    cout<<endl;
  }
  for (unsigned int i = 0; i < WidthCol1 + 5*Width + 6*3 - 1; ++i) cout<<"-";
  cout<<endl;
  // Sums:
  cout<<setw(WidthCol1)<<right<<"Total"<<" | ";
  for (unsigned int i = 0; i < ProcessTotals.size(); ++i) {
    cout<<setw(Width)<<right<<ProcessTotals[i]<<" | ";
  }
  cout<<setw(Width)<<right<<Total<<" | "<<endl;
  cout<<endl;
  cout<<endl;
  
  
  Width = 6;
  cout<<"As percentage:"<<endl<<endl;
  cout<<setw(WidthCol1+3)<<" | "<<setw(Width)<<right<<"PHOT"<<"  | "<<setw(Width)<<right<<"COMP"<<"  | "<<setw(Width)<<right<<"PAIR"<<"  | "<<setw(Width)<<right<<"Rest"<<"  | "<<setw(Width)<<right<<"Total"<<"  |"<<endl;
  for (unsigned int i = 0; i < WidthCol1 + 5*Width + 6*3 + 5 - 1; ++i) cout<<"-";
  cout<<endl;
  for (unsigned int n = 0; n < m_DetectorNames.size(); ++n) {
    cout<<setw(WidthCol1)<<m_DetectorNames[n]<<" | ";
    for (unsigned int i = 0; i < m_OccupancyMatrix[n].size(); ++i) {
      cout<<setw(Width)<<fixed<<setprecision(2)<<right<<100.0*m_OccupancyMatrix[n][i]/Total<<"% | ";
    }
    cout<<setw(Width)<<fixed<<setprecision(2)<<right<<100.0*DetectorTotals[n]/Total<<"% | ";
    cout<<endl;
  }
  for (unsigned int i = 0; i < WidthCol1 + 5*Width + 6*3 + 5 - 1; ++i) cout<<"-";
  cout<<endl;
  // Sums:
  cout<<setw(WidthCol1)<<right<<"Total"<<" | ";
  for (unsigned int i = 0; i < ProcessTotals.size(); ++i) {
    cout<<setw(Width)<<fixed<<setprecision(2)<<right<<100.0*ProcessTotals[i]/Total<<"% | ";
  }
  cout<<setw(Width)<<fixed<<setprecision(2)<<right<<100.0*Total/Total<<"% | "<<endl;
  cout<<endl;
  
  return;
}
  
  
////////////////////////////////////////////////////////////////////////////////


void MInterfaceSivan::IncidenceAngle()
{
  // Open the simulation file:
  MFileEventsSim EventFile(m_Geometry);
  if (EventFile.Open(m_Data->GetCurrentFileName()) == false) {
    mgui<<"Unable to open file"<<error;
    return;
  }
  EventFile.ShowProgress();

  int ThetaNBins = 90;
  double ThetaMin = 0.0;
  double ThetaMax = 180.0;
  double ThetaWidth = (ThetaMax-ThetaMin)/ThetaNBins;
  int PhiNBins = 180;
  double PhiMin = -180.0;
  double PhiMax = 180.0;

  TH1D* Angle = new TH1D("IncidenceAngle", "Incidence Angle (relative to movement down the z-axis)", ThetaNBins, ThetaMin, ThetaMax);
  Angle->SetBit(kCanDelete);
  Angle->SetXTitle("Incidence Angle [deg]");
  Angle->SetYTitle("counts/sr");
  Angle->SetFillColor(8);

  TH2D* Origin = new TH2D("MovementDirection", "Movement direction (spherical coordinates) in cts/sr", PhiNBins, PhiMin, PhiMax, ThetaNBins, ThetaMin, ThetaMax);
  Origin->SetBit(kCanDelete);
  Origin->SetXTitle("Phi [deg]");
  Origin->SetYTitle("Theta [deg]");
  Origin->SetZTitle("counts/sr");
  Origin->SetFillColor(8);

  MSimEvent* Event;
  while ((Event = EventFile.GetNextEvent(false)) != 0) {
    if (Event->GetNIAs() >= 1) {
      Angle->Fill(Event->GetIAAt(0)->GetSecondaryDirection().Angle(MVector(0, 0, -1))*c_Deg);
      Origin->Fill(Event->GetIAAt(0)->GetSecondaryDirection().Phi()*c_Deg, Event->GetIAAt(0)->GetSecondaryDirection().Theta()*c_Deg);
    }
    delete Event;
  }
  
  // Normalize to counts/sr
  for (int t = 1; t <= Angle->GetXaxis()->GetNbins(); ++t) {
    double Area = 2*c_Pi * (cos((t-1)*ThetaWidth*c_Rad) - cos(t*ThetaWidth*c_Rad));
    Angle->SetBinContent(t, Angle->GetBinContent(t) / Area);
  }
  Angle->SetMinimum(0.0);
  for (int t = 1; t <= Origin->GetYaxis()->GetNbins(); ++t) {
    double Area = 2*c_Pi * (cos((t-1)*ThetaWidth*c_Rad) - cos(t*ThetaWidth*c_Rad));
    for (int p = 1; p <= Origin->GetXaxis()->GetNbins(); ++p) {
      Origin->SetBinContent(p, t, Origin->GetBinContent(p, t) / (Area/PhiNBins));
    }
  }
  Origin->SetMinimum(0.0);

  TCanvas* AngleCanvas = new TCanvas("IncidenceAngleCanvas", "Incidence Angle Canvas", 800, 600);
  AngleCanvas->cd();
  Angle->Draw();
  AngleCanvas->Update();

  TCanvas* OriginCanvas = new TCanvas("OriginCanvas", "Origin Canvas", 800, 600);
  OriginCanvas->cd();
  OriginCanvas->SetLogz();
  Origin->Draw("colz");
  OriginCanvas->Update();


  mout<<endl;
  mout<<endl;
  mout<<"Remarks/restrictions: "<<endl;
  mout<<"  * Incidence angle is relative to (0, 0, -1)"<<endl;

  return;
}


////////////////////////////////////////////////////////////////////////////////


void MInterfaceSivan::SecondaryGenerationPattern()
{
  // This function gives histograms of emission patterns of secondaries 

  int NBins = 100;
  double EnergyThreshold = 0;

  // Open the simulation file:
  MFileEventsSim EventFile(m_Geometry);
  if (EventFile.Open(m_Data->GetCurrentFileName()) == false) {
    mgui<<"Unable to open file"<<error;
    return;
  }
  EventFile.ShowProgress();
  EventFile.SetProgressTitle("Progress", "Progress of axes determination...");

  // Test dimension of image:
  
  MSimEvent* Event;
  MSimIA* IA = 0;
  int MaxNIAs = 10000;
  vector<MVector> Positions;
  mdebug<<"Testing dimensions of geometry..."<<endl;
  while ((Event = EventFile.GetNextEvent(false)) != 0) {
    for (unsigned int h = 0; h < Event->GetNIAs(); ++h) {
      IA = Event->GetIAAt(h);
      if (IA->GetProcess() != "INIT" && IA->GetProcess() != "ESCP") {
        Positions.push_back(IA->GetPosition());
        MaxNIAs--;
      }
    }
    
    delete Event;
    if (MaxNIAs <= 0) break;
  }

  double xMin, xMax, yMin, yMax, zMin, zMax;
  DetermineAxis(xMin, xMax, yMin, yMax, zMin, zMax, Positions);


  EventFile.Rewind();
  EventFile.SetProgressTitle("Progress", "Progress of histogramming...");

  MVector Pos;
  map<int, TH3D*> EmissionPatterns;
  while ((Event = EventFile.GetNextEvent(false)) != 0) {
    for (unsigned int h = 0; h < Event->GetNIAs(); ++h) {
      IA = Event->GetIAAt(h);
      if (IA->GetProcess() != "INIT" && IA->GetProcess() != "ESCP") {
        Pos = IA->GetPosition();
  
        // If the particle is not a nucleus (up to alpha is ok):
        if (IA->GetSecondaryParticleID() < 100 && IA->GetSecondaryEnergy() > EnergyThreshold) {
          TH3D* Hist = EmissionPatterns[IA->GetSecondaryParticleID()];
          if (Hist == 0) {
            ostringstream out;
            out<<"Emission pattern for particle type: "<<IA->GetSecondaryParticleID();
            Hist = new TH3D(out.str().c_str(), out.str().c_str(), NBins, xMin, xMax, NBins, yMin, yMax, NBins, zMin, zMax);
            Hist->SetXTitle("x [cm]");
            Hist->SetYTitle("y [cm]");
            Hist->SetZTitle("z [cm]");
            EmissionPatterns[IA->GetSecondaryParticleID()] = Hist;
          }
          Hist->Fill(Pos.X(), Pos.Y(), Pos.Z());
        }
      }
    }
    
    delete Event;
  }

  for (map<int, TH3D*>::iterator Iter = EmissionPatterns.begin(); 
       Iter != EmissionPatterns.end(); ++Iter) {
    // Create projections ...
    TH2D* xyProjection = new TH2D(MString((*Iter).second->GetTitle()) + " - xy", MString((*Iter).second->GetTitle()) + " - xy", NBins, xMin, xMax, NBins, yMin, yMax);
    xyProjection->SetXTitle("x [cm]");
    xyProjection->SetYTitle("y [cm]");
    TH2D* xzProjection = new TH2D(MString((*Iter).second->GetTitle()) + " - xz", MString((*Iter).second->GetTitle()) + " - xz", NBins, xMin, xMax, NBins, zMin, zMax);
    xzProjection->SetXTitle("x [cm]");
    xzProjection->SetYTitle("z [cm]");
    TH2D* yzProjection = new TH2D(MString((*Iter).second->GetTitle()) + " - yz", MString((*Iter).second->GetTitle()) + " - yz", NBins, yMin, yMax, NBins, zMin, zMax);
    yzProjection->SetXTitle("y [cm]");
    yzProjection->SetYTitle("z [cm]");
    TH1D* xProjection = new TH1D(MString((*Iter).second->GetTitle()) + " - x", MString((*Iter).second->GetTitle()) + " - xy", NBins, xMin, xMax);
    xProjection->SetXTitle("x [cm]");
    xProjection->SetYTitle("counts");
    TH1D* yProjection = new TH1D(MString((*Iter).second->GetTitle()) + " - y", MString((*Iter).second->GetTitle()) + " - xz", NBins, yMin, yMax);
    yProjection->SetXTitle("y [cm]");
    yProjection->SetYTitle("counts");
    TH1D* zProjection = new TH1D(MString((*Iter).second->GetTitle()) + " - z", MString((*Iter).second->GetTitle()) + " - yz", NBins, zMin, zMax);
    zProjection->SetXTitle("z [cm]");
    zProjection->SetYTitle("counts");
    for (int bx = 0; bx <+ NBins; ++bx) {
      for (int by = 0; by <+ NBins; ++by) {
        for (int bz = 0; bz <+ NBins; ++bz) {
          xyProjection->SetBinContent(bx, by, xyProjection->GetBinContent(bx, by) + (*Iter).second->GetBinContent(bx, by, bz));
          xzProjection->SetBinContent(bx, bz, xzProjection->GetBinContent(bx, bz) + (*Iter).second->GetBinContent(bx, by, bz));
          yzProjection->SetBinContent(by, bz, yzProjection->GetBinContent(by, bz) + (*Iter).second->GetBinContent(bx, by, bz));
          xProjection->SetBinContent(bx, xProjection->GetBinContent(bx) + (*Iter).second->GetBinContent(bx, by, bz));
          yProjection->SetBinContent(by, yProjection->GetBinContent(by) + (*Iter).second->GetBinContent(bx, by, bz));
          zProjection->SetBinContent(bz, zProjection->GetBinContent(bz) + (*Iter).second->GetBinContent(bx, by, bz));
        }
      }
    }
    // And draw them...
    TCanvas* Canvas = new TCanvas((*Iter).second->GetTitle(), (*Iter).second->GetTitle(), 700, 700);
    Canvas->Divide(3, 3);
    Canvas->cd(1);
    (*Iter).second->Draw();
    Canvas->cd(4);
    xyProjection->Draw("colz");
    Canvas->cd(5);
    xzProjection->Draw("colz");
    Canvas->cd(6);
    yzProjection->Draw("colz");
    Canvas->cd(7);
    xProjection->Draw();
    Canvas->cd(8);
    yProjection->Draw();
    Canvas->cd(9);
    zProjection->Draw();
    Canvas->Update();
  }


  mout<<endl;
  mout<<endl;
  mout<<"Remarks/restrictions: "<<endl;
  mout<<"  * An energy threshold of "<<EnergyThreshold<<" keV was used"<<endl;
  mout<<"  * No heavy (A > 4) nuclei are reported"<<endl;
  mout<<"  * The particle ID is that from the simulation (see Cosima documentation)"<<endl;

  return;
}



////////////////////////////////////////////////////////////////////////////////


void MInterfaceSivan::InitialComptonScatterAngle()
{
  MFileEventsSim EventFile(m_Geometry);
  if (EventFile.Open(m_Data->GetCurrentFileName()) == false) {
    mgui<<"Unable to open file"<<error;
    return;
  }
  EventFile.ShowProgress();

  TH1D* Angle = new TH1D("InitialComptonScatterAngle", 
                         "Initial Compton Scatter Angle", 90, 0, 180);
  Angle->SetBit(kCanDelete);
  Angle->SetXTitle("Compton scatter angle [#circ]");
  Angle->SetYTitle("counts");
  Angle->SetFillColor(8);

  MSimEvent* Event = 0;
  while ((Event = EventFile.GetNextEvent(false)) != 0) {
    MVector Start, Center, Stop;
    bool FoundAll = false;
    if (Event->GetNIAs() >= 1) {
      Start = Event->GetIAAt(0)->GetPosition();
      if (Event->GetNIAs() >= 2 && Event->GetIAAt(1)->GetOriginID() == 1 && 
          Event->GetIAAt(1)->GetProcess() == "COMP") {
        Center = Event->GetIAAt(1)->GetPosition();
        for (unsigned int i = 2; i < Event->GetNIAs(); ++i) {
          if (Event->GetIAAt(i)->GetOriginID() == 1 && Event->GetIAAt(i)->GetPosition() != Center) {
            Stop = Event->GetIAAt(i)->GetPosition();
            FoundAll = true;
            break;
          }
        }
      }
    }
    if (FoundAll == true) {
      double A = (Center - Start).Angle(Stop - Center) * c_Deg;
      Angle->Fill(A);
      if (A == 0) {
        cout<<Event->ToSimString()<<endl;
      }
    }
    delete Event;
  }

  TCanvas* Canvas = 
    new TCanvas("InitialComptonScatterAngleCanvas", 
                "Initial Compton Scatter Angle", 800, 600);
  Canvas->cd();
  Angle->Draw();
  Canvas->Update();
  mout<<endl;
  mout<<endl;
  mout<<"Remarks/restrictions: "<<endl;
  mout<<"  * none"<<endl;

  return;
}


////////////////////////////////////////////////////////////////////////////////


void MInterfaceSivan::NInteractions()
{
  // Open the simulation file:
  MFileEventsSim EventFile(m_Geometry);
  if (EventFile.Open(m_Data->GetCurrentFileName()) == false) {
    mgui<<"Unable to open file"<<error;
    return;
  }
  EventFile.ShowProgress();

  int NEvents = 0;

  int NComptonsFirst = 0;
  int NPairsFirst = 0;
  int NPhotosFirst = 0;
  int NRayleighsFirst = 0;

  int NComptons = 0;
  int NPairs = 0;
  int NPhotos = 0;
  int NRayleighs = 0;

  MSimEvent* Event;
  while ((Event = EventFile.GetNextEvent(false)) != 0) {
    if (Event->GetNIAs() > 1) {
      mdebug<<"Event: "<<Event->GetID()<<endl;
      NEvents++;
      if (Event->GetIAAt(1)->GetOriginID() == 1) {
        if (Event->GetIAAt(1)->GetProcess() == "PHOT") {
          mdebug<<"  Photo first "<<endl;
          NPhotosFirst++;
          NPhotos++;
        } else if (Event->GetIAAt(1)->GetProcess() == "COMP") {
          mdebug<<"  Compton first "<<endl;
          NComptonsFirst++;
          NComptons++;
        } else if (Event->GetIAAt(1)->GetProcess() == "PAIR") {
          mdebug<<"  Pair first "<<endl;
          NPairsFirst++;
          NPairs++;
        } else if (Event->GetIAAt(1)->GetProcess() == "RAYL") {
          mdebug<<"  Rayleigh first "<<endl;
          NRayleighsFirst++;
          NRayleighs++;
        }
      }

      for (unsigned int i = 2; i < Event->GetNIAs(); ++i) {
        if (Event->GetIAAt(i)->GetOriginID() == 1) {
          if (Event->GetIAAt(i)->GetProcess() == "PHOT") {
            mdebug<<"  Photo"<<endl;
            NPhotos++;
            break; // Count only first photo effect
          } else if (Event->GetIAAt(i)->GetProcess() == "COMP") {
            mdebug<<"  Compton"<<endl;
            NComptons++;
          } else if (Event->GetIAAt(i)->GetProcess() == "PAIR") {
            mdebug<<"  Pair"<<endl;
            NPairs++;
          } else if (Event->GetIAAt(i)->GetProcess() == "RAYL") {
            mdebug<<"  Rayleigh"<<endl;
            NRayleighs++;
          }
        }
      }
    }
    delete Event;
  }

  mout<<"Statistics on the ideal - not the detected interactions"<<endl;
  mout<<"  of the initial event"<<endl;
  mout<<endl;
  if (NEvents > 0) {
    double Average = double(NPhotos + NComptons + NPairs + NRayleighs)/NEvents;
    mout<<"First interaction: "<<endl;
    mout<<"Photo:  .....................  "<<100.0*NPhotosFirst/NEvents<<"%"<<endl;
    mout<<"Compton:  ...................  "<<100.0*NComptonsFirst/NEvents<<"%"<<endl;
    mout<<"Pair:  ......................  "<<100.0*NPairsFirst/NEvents<<"%"<<endl;
    mout<<"Rayleigh:  ..................  "<<100.0*NRayleighsFirst/NEvents<<"%"<<endl;
    mout<<endl;
    mout<<"All interactions: "<<endl;
    mout<<"Photo:  .....................  "<<100.0*NPhotos/(Average*NEvents)<<"%"<<endl;
    mout<<"Compton:  ...................  "<<100.0*NComptons/(Average*NEvents)<<"%"<<endl;
    mout<<"Pair:  ......................  "<<100.0*NPairs/(Average*NEvents)<<"%"<<endl;
    mout<<"Rayleigh:  ..................  "<<100.0*NRayleighs/(Average*NEvents)<<"%"<<endl;
    mout<<endl;
    mout<<"Avg. number of interactions:   "<<Average<<endl;
    mout<<endl;
  } else {
    mout<<"Ne events detected!"<<endl;
  }
  mout<<endl;
  mout<<endl;
  mout<<"Remarks/restrictions: "<<endl;
  mout<<"  * The statistics is only on the original particle (photon)"<<endl;
  mout<<"    --> Secondaries are not counted"<<endl;
  mout<<"  * The final photo effect is counted as one interaction"<<endl;

  return;
}


////////////////////////////////////////////////////////////////////////////////


void MInterfaceSivan::FindMinimumAndMaximumStartEnergy(double& Min, double& Max, unsigned int NEventsToCheck)
{
  MFileEventsSim EventFile(m_Geometry);
  if (EventFile.Open(m_Data->GetCurrentFileName()) == false) {
    mgui<<"Unable to open file"<<error;
    return;
  }
  EventFile.ShowProgress();
  
  Min = numeric_limits<double>::max();
  Max = -numeric_limits<double>::max();
  
  MSimEvent* Event;
  mout<<"Testing energy dimensions..."<<endl;
  while ((Event = EventFile.GetNextEvent(false)) != 0) {
    if (Event->GetNIAs() >= 1) {
      if (Event->GetIAAt(0)->GetSecondaryEnergy() < Min) Min = Event->GetIAAt(0)->GetSecondaryEnergy();
      if (Event->GetIAAt(0)->GetSecondaryEnergy() > Max) Max = Event->GetIAAt(0)->GetSecondaryEnergy();
    }
    
    delete Event;
    if (NEventsToCheck-- <= 0) break;
  }
  mout<<"Energy limits of sim file: "<<Min<<" - "<<Max<<" keV"<<endl;

  EventFile.Close();
}


////////////////////////////////////////////////////////////////////////////////


void MInterfaceSivan::IncidenceEnergy()
{
  // Open the simulation file:
  MFileEventsSim EventFile(m_Geometry);
  if (EventFile.Open(m_Data->GetCurrentFileName()) == false) {
    mgui<<"Unable to open file"<<error;
    return;
  }
  EventFile.ShowProgress();

  double EMin = numeric_limits<double>::max();
  double EMax = -numeric_limits<double>::max();
  
  MSimEvent* Event;
  int MaxNHits = 1000;
  mout<<"Testing energy dimensions..."<<endl;
  while ((Event = EventFile.GetNextEvent(false)) != 0) {
    if (Event->GetNIAs() >= 1) {
      if (Event->GetIAAt(0)->GetSecondaryEnergy() < EMin) EMin = Event->GetIAAt(0)->GetSecondaryEnergy();
      if (Event->GetIAAt(0)->GetSecondaryEnergy() > EMax) EMax = Event->GetIAAt(0)->GetSecondaryEnergy();
    }
    
    delete Event;
    if (MaxNHits-- <= 0) break;
  }
  mout<<"E:"<<EMin<<" - "<<EMax<<endl;

  EMax += max(0.2*(EMax-EMin), 10.0);
  EMin -= max(0.2*(EMax-EMin), 10.0);

  EMin = TMath::Floor(EMin);
  EMax = TMath::Ceil(EMax);
  if (EMin < 0) EMin = 1;

  mout<<endl;
  mout<<"Setting dimensions to:"<<endl;
  mout<<"x:"<<EMin<<" - "<<EMax<<endl;
  mout<<endl;

  bool IsLog = false;
  if (EMax/EMin > 100) IsLog = true;
  int ENBins = 100;
  double* EBins = CreateAxisBins(EMin, EMax, ENBins, IsLog);


  TH1D* Energy = new TH1D("Energy", "Energy", ENBins, EBins);
  Energy->SetBit(kCanDelete);
  Energy->SetXTitle("Energy [keV]");
  Energy->SetYTitle("counts/keV");
  Energy->SetFillColor(8);

  EventFile.Rewind();
  while ((Event = EventFile.GetNextEvent(false)) != 0) {
    if (Event->GetNIAs() >= 1) {
      Energy->Fill(Event->GetIAAt(0)->GetSecondaryEnergy());
    }

    delete Event;
  }

  // Normalize to counts/keV
  for (int b = 1; b <= Energy->GetNbinsX(); ++b) {
    Energy->SetBinContent(b, Energy->GetBinContent(b)/Energy->GetBinWidth(b));
  }
  


  TCanvas* ECanvas = new TCanvas("Energy Canvas", "Energy Canvas", 800, 600);
  if (IsLog == true) ECanvas->SetLogx();
  ECanvas->cd();
  Energy->Draw();
  ECanvas->Update();

  delete [] EBins;

  return;
}


////////////////////////////////////////////////////////////////////////////////


void MInterfaceSivan::IncidenceVsMeasuredEnergy()
{
  cout<<"Temporarily deactivated noising!"<<endl;
  m_Geometry->ActivateNoising(false);
  m_Geometry->SetGlobalFailureRate(0);


  // Open the simulation file:
  MFileEventsSim EventFile(m_Geometry);
  if (EventFile.Open(m_Data->GetCurrentFileName()) == false) {
    mgui<<"Unable to open file"<<error;
    return;
  }
  EventFile.ShowProgress();

  double EMin = numeric_limits<double>::max();
  double EMax = -numeric_limits<double>::max();
  
  MSimEvent* Event;
  int MaxNHits = 1000;
  mout<<"Testing energy dimensions..."<<endl;
  while ((Event = EventFile.GetNextEvent(true)) != 0) { // True required, so that we do trigger
    if (Event->GetNIAs() >= 1) {
      if (Event->GetIAAt(0)->GetSecondaryEnergy() < EMin) EMin = Event->GetIAAt(0)->GetSecondaryEnergy();
      if (Event->GetIAAt(0)->GetSecondaryEnergy() > EMax) EMax = Event->GetIAAt(0)->GetSecondaryEnergy();
      if (Event->GetREnergy() > 0 && Event->GetREnergy() < EMin) EMin = Event->GetREnergy();
      if (Event->GetREnergy() > EMax) EMax = Event->GetREnergy();
    }
    
    delete Event;
    if (MaxNHits-- <= 0) break;
  }
  mout<<"E:"<<EMin<<" - "<<EMax<<endl;

  //EMax += max(0.2*(EMax-EMin), 10.0);
  //EMin -= max(0.2*(EMax-EMin), 10.0);

  EMin = TMath::Floor(EMin);
  EMax = TMath::Ceil(EMax);
  if (EMin < 0) EMin = 1;

  mout<<endl;
  mout<<"Setting dimensions to:"<<endl;
  mout<<"x:"<<EMin<<" - "<<EMax<<endl;
  mout<<endl;

  bool IsLog = false;
  if (EMax/EMin > 100) IsLog = true;
  
  unsigned long NEvents = 0;
  NEvents = EventFile.GetNEvents(false);
  
  int ENBins = 100;
  
  if (NEvents > 0) {
    ENBins = sqrt(NEvents)/10;
    if (ENBins < 5) ENBins = 5;
  }
  
  double* EBins = CreateAxisBins(EMin, EMax, ENBins, IsLog);


  TH2D* Energy = new TH2D("Energies", "Incidence vs. measured energies", ENBins, EBins, ENBins, EBins);
  Energy->SetBit(kCanDelete);
  Energy->SetXTitle("Incidence energy [keV]");
  Energy->SetYTitle("Measured energy [keV]");
  Energy->SetZTitle("counts/keV^2");
  Energy->SetFillColor(8);

  EventFile.Rewind();
  while ((Event = EventFile.GetNextEvent(true)) != 0) { // True required, so that we do trigger and veto
    if (Event->GetNIAs() >= 1 && Event->GetREnergy() > 0) {
      Energy->Fill(Event->GetIAAt(0)->GetSecondaryEnergy(), Event->GetREnergy());
    }

    delete Event;
  }

  // Normalize to counts/keV^2
  double Min = numeric_limits<double>::max();
  for (int bx = 1; bx <= Energy->GetNbinsX(); ++bx) {
    for (int by = 1; by <= Energy->GetNbinsY(); ++by) {
      double Value = Energy->GetBinContent(bx, by)/Energy->GetXaxis()->GetBinWidth(bx)/Energy->GetYaxis()->GetBinWidth(by);
      Energy->SetBinContent(bx, by, Value);
      if (Value > 0 && Value < Min) Min = Value;
    }
  }
  Energy->SetMinimum(Min);

  TCanvas* ECanvas = new TCanvas("Energy Canvas", "Energy Canvas", 800, 600);
  if (IsLog == true) {
    ECanvas->SetLogx();
    ECanvas->SetLogy();
  }
  ECanvas->SetLogz();
  ECanvas->cd();
  Energy->Draw("colz");
  ECanvas->Update();

  delete [] EBins;

  return;
}


////////////////////////////////////////////////////////////////////////////////


void MInterfaceSivan::EnergyOfSecondaries()
{
  mimp<<"Exclude all different INIT types"<<show;

  // Open the simulation file:
  MFileEventsSim EventFile(m_Geometry);
  if (EventFile.Open(m_Data->GetCurrentFileName()) == false) {
    mgui<<"Unable to open file"<<error;
    return;
  }
  EventFile.ShowProgress();

  double EMin = numeric_limits<double>::max();
  double EMax = -numeric_limits<double>::max();
  
  MSimEvent* Event;
  int MaxNHits = 1000;
  mout<<"Testing energy dimensions..."<<endl;
  while ((Event = EventFile.GetNextEvent(false)) != 0) {
    for (unsigned int i = 0; i < Event->GetNIAs(); ++i) {
      if (Event->GetIAAt(i)->GetSecondaryEnergy() > 0) {
        if (Event->GetIAAt(i)->GetProcess() != "INIT") {
          if (Event->GetIAAt(i)->GetSecondaryEnergy() < EMin) EMin = Event->GetIAAt(i)->GetSecondaryEnergy();
          if (Event->GetIAAt(i)->GetSecondaryEnergy() > EMax) EMax = Event->GetIAAt(i)->GetSecondaryEnergy();
        }
      }
    }

    delete Event;
    if (MaxNHits-- <= 0) break;
  }
  mout<<"E:"<<EMin<<" - "<<EMax<<endl;

  EMax += max(0.2*(EMax-EMin), 10.0);
  EMin -= max(0.2*(EMax-EMin), 10.0);

  EMin = TMath::Floor(EMin);
  EMax = TMath::Ceil(EMax);
  if (EMin < 0) EMin = 0.1;

  mout<<endl;
  mout<<"Setting dimensions to:"<<endl;
  mout<<"x:"<<EMin<<" - "<<EMax<<endl;
  mout<<endl;

  bool IsLog = false;
  if (EMax/EMin > 100) IsLog = true;
  int ENBins = 1000;
  double* EBins = CreateAxisBins(EMin, EMax, ENBins, IsLog);


  TH1D* EnergyPhotons = new TH1D("EnergyPhotons", "Energy of secondary photons", ENBins, EBins);
  EnergyPhotons->SetBit(kCanDelete);
  EnergyPhotons->SetXTitle("Energy [keV]");
  EnergyPhotons->SetYTitle("counts/keV");
  EnergyPhotons->SetFillColor(8);

  TH1D* EnergyElectrons = new TH1D("EnergyElectrons", "Energy of secondary electrons", ENBins, EBins);
  EnergyElectrons->SetBit(kCanDelete);
  EnergyElectrons->SetXTitle("Energy [keV]");
  EnergyElectrons->SetYTitle("counts/keV");
  EnergyElectrons->SetFillColor(8);

  EventFile.Rewind();
  while ((Event = EventFile.GetNextEvent(false)) != 0) {
    for (unsigned int i = 0; i < Event->GetNIAs(); ++i) {
      if (Event->GetIAAt(i)->GetProcess() != "INIT") { 
        if (Event->GetIAAt(i)->GetSecondaryEnergy() > 0) {
          if (Event->GetIAAt(i)->GetSecondaryParticleID() == 1) {
            EnergyPhotons->Fill(Event->GetIAAt(i)->GetSecondaryEnergy());
          } else if (Event->GetIAAt(i)->GetSecondaryParticleID() == 3) {
            EnergyElectrons->Fill(Event->GetIAAt(i)->GetSecondaryEnergy());
          }
        }
      }
    }

    delete Event;
  }

  // Normalize to counts/keV
  for (int b = 1; b <= EnergyPhotons->GetNbinsX(); ++b) {
    EnergyElectrons->SetBinContent(b, EnergyElectrons->GetBinContent(b)/EnergyElectrons->GetBinWidth(b));
    EnergyPhotons->SetBinContent(b, EnergyPhotons->GetBinContent(b)/EnergyPhotons->GetBinWidth(b));
  }
  


  TCanvas* EnergyElectronsCanvas = new TCanvas("EnergyElectronsCanvas", "EnergyElectronsCanvas", 800, 600);
  if (IsLog == true) EnergyElectronsCanvas->SetLogx();
  EnergyElectronsCanvas->cd();
  EnergyElectrons->Draw();
  EnergyElectronsCanvas->Update();

  TCanvas* EnergyPhotonsCanvas = new TCanvas("EnergyPhotonsCanvas", "EnergyPhotonsCanvas", 800, 600);
  if (IsLog == true) EnergyPhotonsCanvas->SetLogx();
  EnergyPhotonsCanvas->cd();
  EnergyPhotons->Draw();
  EnergyPhotonsCanvas->Update();

  delete [] EBins;

  return;
}


////////////////////////////////////////////////////////////////////////////////


void MInterfaceSivan::AverageNumberOfHits()
{
  // Determine the number of hits per event:

  // Open the simulation file:
  MFileEventsSim EventFile(m_Geometry);
  if (EventFile.Open(m_Data->GetCurrentFileName()) == false) {
    mgui<<"Unable to open file"<<error;
    return;
  }
  EventFile.ShowProgress();

  int NEvents = 0;
  double NHits = 0;
  map<MString, double> NHitsPerDetectorType;

  
  MSimEvent* Event;
  while ((Event = EventFile.GetNextEvent(false)) != 0) {
    if (Event->GetNHTs() > 0) {
      NEvents++;
      NHits += Event->GetNHTs();
      for (unsigned int h = 0; h < Event->GetNHTs(); ++h) {
        NHitsPerDetectorType[Event->GetHTAt(h)->GetVolumeSequence()->GetDetector()->GetName()]++;
      }
    }
    delete Event;
  }

  ostringstream out;
  
  if (NEvents > 0) {
    out<<"Average number of hits:   "<<NHits/NEvents<<endl;
    map<MString, double>::iterator NHitsPerDetectorTypeIter;
    out<<endl;
    out<<"Average number of hits per detector type: "<<endl;
    
    for (NHitsPerDetectorTypeIter = NHitsPerDetectorType.begin(); 
         NHitsPerDetectorTypeIter != NHitsPerDetectorType.end(); 
         NHitsPerDetectorTypeIter++) {
      out<<setw(24)<<(*NHitsPerDetectorTypeIter).first<<": "
         <<setw(10)<<(*NHitsPerDetectorTypeIter).second/NEvents<<"+-"<<sqrt((*NHitsPerDetectorTypeIter).second)/NEvents<<endl;
    }
    out<<endl;
  } else {
    out<<"No events passed the event-selections!"<<endl;
  }

  mgui<<out.str()<<endl;

  return;
}


////////////////////////////////////////////////////////////////////////////////


void MInterfaceSivan::LocationOfFirstDetectedInteraction()
{
  // Open the simulation file:
  MFileEventsSim EventFile(m_Geometry);
  if (EventFile.Open(m_Data->GetCurrentFileName()) == false) {
    mgui<<"Unable to open file"<<error;
    return;
  }
  EventFile.ShowProgress();

  double xMin = numeric_limits<double>::max();
  double yMin = numeric_limits<double>::max();
  double zMin = numeric_limits<double>::max();

  double xMax = -numeric_limits<double>::max();
  double yMax = -numeric_limits<double>::max();
  double zMax = -numeric_limits<double>::max();
  
  MVector Pos;
  MSimEvent* Event;
  MSimHT* HT = 0;
  unsigned int MaxNHits = 0;
  mdebug<<"Testing dimensions of geometry..."<<endl;
  while ((Event = EventFile.GetNextEvent(false)) != 0) {
    for (unsigned int h = 0; h < Event->GetNHTs(); ++h) {
      HT = Event->GetHTAt(h);
      Pos = HT->GetPosition();
      if (Pos[0] > xMax) xMax = Pos[0];
      if (Pos[1] > yMax) yMax = Pos[1];
      if (Pos[2] > zMax) zMax = Pos[2];
      if (Pos[0] < xMin) xMin = Pos[0];
      if (Pos[1] < yMin) yMin = Pos[1];
      if (Pos[2] < zMin) zMin = Pos[2];
      MaxNHits++;
    }
    delete Event;

    if (MaxNHits > m_Data->GetNInitializationEvents()) break;
  }

  mdebug<<"x:"<<xMin<<" - "<<xMax<<endl;
  mdebug<<"y:"<<yMin<<" - "<<yMax<<endl;
  mdebug<<"z:"<<zMin<<" - "<<zMax<<endl;

  double Diff = 0.15*(xMax-xMin);
  xMax += Diff;
  xMin -= Diff;
  if (xMax-xMin > 10 && xMax-xMin < 100) {
    xMax = int(xMax/10 + 1)*10;
    xMin = int(xMin/10 - 1)*10;
  }

  Diff = 0.25*(yMax-yMin);
  yMax += Diff;
  yMin -= Diff;
  if (yMax-yMin > 10 && yMax-yMin < 100) {
    yMax = int(yMax/10 + 1)*10;
    yMin = int(yMin/10 - 1)*10;
  }

  Diff = 0.25*(zMax-zMin);
  zMax += Diff;
  zMin -= Diff;
  if (zMax-zMin > 10 && zMax-zMin < 100) {
    zMax = int(zMax/10 + 1)*10;
    zMin = int(zMin/10 - 1)*10;
  }

  xMin = TMath::Floor(xMin);
  xMax = TMath::Ceil(xMax);
  yMin = TMath::Floor(yMin);
  yMax = TMath::Ceil(yMax);
  zMin = TMath::Floor(zMin);
  zMax = TMath::Ceil(zMax);

  mdebug<<endl;
  mdebug<<"Setting dimensions of geometry to:"<<endl;
  mdebug<<"x:"<<xMin<<" - "<<xMax<<endl;
  mdebug<<"y:"<<yMin<<" - "<<yMax<<endl;
  mdebug<<"z:"<<zMin<<" - "<<zMax<<endl;
  mdebug<<endl;

  TH1D* zPosition = new TH1D("z Position", "z Position", 200, zMin, zMax);
  zPosition->SetBit(kCanDelete);
  zPosition->SetXTitle("z position [cm]");
  zPosition->SetYTitle("counts");
  zPosition->SetFillColor(8);

  TH2D* xyPosition = new TH2D("xy Position", "xy Position", 200, xMin, xMax, 200, yMin, yMax);
  xyPosition->SetBit(kCanDelete);
  xyPosition->SetXTitle("x position [cm]");
  xyPosition->SetYTitle("y position [cm]");
  xyPosition->SetZTitle("counts");
  xyPosition->SetFillColor(8);

  const int MaxTrackLength = 100;
  vector<int> TrackLength(MaxTrackLength);
 
  map<MString, unsigned int> Materials;

  EventFile.Rewind();
  while ((Event = EventFile.GetNextEvent(false)) != 0) {
    int HTWithSmallestOrigin = -1;
    int SmallestOrigin = numeric_limits<int>::max();
    
    for (unsigned int h = 0; h < Event->GetNHTs(); ++h) {
      HT = Event->GetHTAt(h);
      for (unsigned int i = 0; i < HT->GetNOrigins(); ++i) {
        if (HT->GetOriginAt(i) != 1 && HT->GetOriginAt(i) < SmallestOrigin) { 
          SmallestOrigin = HT->GetOriginAt(i);
          HTWithSmallestOrigin = h;
        }
      }
    }
    if (HTWithSmallestOrigin > -1) {
      xyPosition->Fill(Event->GetHTAt(HTWithSmallestOrigin)->GetPosition()[0],
           Event->GetHTAt(HTWithSmallestOrigin)->GetPosition()[1]);
      zPosition->Fill(Event->GetHTAt(HTWithSmallestOrigin)->GetPosition()[2]);

      MDVolumeSequence* S = m_Geometry->GetVolumeSequencePointer(Event->GetHTAt(HTWithSmallestOrigin)->GetPosition());
      Materials[S->GetDeepestVolume()->GetMaterial()->GetName()]++;
      delete S;
      
      int Length = Event->GetLengthOfFirstComptonTrack();
      if (Length >= 0 && Length < MaxTrackLength) {
        TrackLength[Length]++;
      }
    }

    delete Event;
  }

  map<MString, unsigned int>::iterator MaterialsIter;
  cout<<endl;

  cout<<"Materials of first (detected) interaction:"<<endl;

  int AllHits = 0;
  for (MaterialsIter = (Materials.begin()); 
       MaterialsIter != Materials.end(); MaterialsIter++) {
    AllHits += (*MaterialsIter).second;
  }
  for (MaterialsIter = (Materials.begin()); 
       MaterialsIter != Materials.end(); MaterialsIter++) {
    cout<<setw(24)<<(*MaterialsIter).first<<": "<<setw(10)<<(*MaterialsIter).second
        <<" ("<<setprecision(4)<<setw(9)<<double((*MaterialsIter).second)/AllHits*100.0<<"%)"<<endl;
  }
  cout<<endl;

  cout<<"Track Length in 2D-strip detectors of first (detected) Compton interaction: "<<endl;
  double Sum = 0;
  int Values = 0;
  int MaxDisplay = 10;
  for (int i = 1; i < MaxDisplay; ++i) {
    Values += TrackLength[i];
    Sum += i*TrackLength[i];
    cout<<setw(3)<<i<<"  :"<<setw(10)<<TrackLength[i]<<endl;
  }
  int Overflow = 0;
  for (int i = MaxDisplay+1; i < MaxTrackLength; ++i) {
    Values += TrackLength[i];
    Sum += i*TrackLength[i];
    Overflow += TrackLength[i];
  }
  cout<<setw(3)<<MaxDisplay<<"+ :"<<setw(10)<<Overflow<<endl;
  cout<<endl;
  cout<<"Average: "<<((Values > 0) ? Sum/Values : 0)<<endl;
  cout<<endl;

  TCanvas* zCanvas = new TCanvas("ZPositionCanvas", "z Position Canvas", 800, 600);
  zCanvas->cd();
  zPosition->Draw();
  zCanvas->Update();

  TCanvas* xyCanvas = new TCanvas("XYPositionCanvas", "xy Position Canvas", 800, 800);
  xyCanvas->cd();
  xyPosition->Draw("colz");
  xyCanvas->Update();
}


////////////////////////////////////////////////////////////////////////////////


void MInterfaceSivan::LocationsOfAllDetectedInteractions()
{
  // Open the simulation file:
  MFileEventsSim EventFile(m_Geometry);
  if (EventFile.Open(m_Data->GetCurrentFileName()) == false) {
    mgui<<"Unable to open file"<<error;
    return;
  }
  EventFile.ShowProgress();

  double xMin = numeric_limits<double>::max();
  double yMin = numeric_limits<double>::max();
  double zMin = numeric_limits<double>::max();

  double xMax = -numeric_limits<double>::max();
  double yMax = -numeric_limits<double>::max();
  double zMax = -numeric_limits<double>::max();
  
  MVector Pos;
  MSimEvent* Event;
  MSimHT* HT = 0;
  int MaxNHits = 1000;
  mdebug<<"Testing dimensions of geometry..."<<endl;
  while ((Event = EventFile.GetNextEvent(false)) != 0) {
    for (unsigned int h = 0; h < Event->GetNHTs(); ++h) {
      HT = Event->GetHTAt(h);
      Pos = HT->GetPosition();
      if (Pos[0] > xMax) xMax = Pos[0];
      if (Pos[1] > yMax) yMax = Pos[1];
      if (Pos[2] > zMax) zMax = Pos[2];
      if (Pos[0] < xMin) xMin = Pos[0];
      if (Pos[1] < yMin) yMin = Pos[1];
      if (Pos[2] < zMin) zMin = Pos[2];
      MaxNHits--;
    }
    
    delete Event;
    if (MaxNHits <= 0) break;
  }
  mdebug<<"x:"<<xMin<<" - "<<xMax<<endl;
  mdebug<<"y:"<<yMin<<" - "<<yMax<<endl;
  mdebug<<"z:"<<zMin<<" - "<<zMax<<endl;

  double Diff = 0.15*(xMax-xMin);
  xMax += Diff;
  xMin -= Diff;
  Diff = 0.25*(yMax-yMin);
  yMax += Diff;
  yMin -= Diff;
  Diff = 0.25*(zMax-zMin);
  zMax += Diff;
  zMin -= Diff;

  xMin = TMath::Floor(xMin);
  xMax = TMath::Ceil(xMax);
  yMin = TMath::Floor(yMin);
  yMax = TMath::Ceil(yMax);
  zMin = TMath::Floor(zMin);
  zMax = TMath::Ceil(zMax);

  mdebug<<endl;
  mdebug<<"Setting dimensions of geometry to:"<<endl;
  mdebug<<"x:"<<xMin<<" - "<<xMax<<endl;
  mdebug<<"y:"<<yMin<<" - "<<yMax<<endl;
  mdebug<<"z:"<<zMin<<" - "<<zMax<<endl;
  mdebug<<endl;

  TH1D* zPosition = new TH1D("z Position", "z Position", 200, zMin, zMax);
  zPosition->SetBit(kCanDelete);
  zPosition->SetXTitle("z position [cm]");
  zPosition->SetYTitle("counts");
  zPosition->SetFillColor(8);

  TH2D* xyPosition = new TH2D("xy Position", "xy Position", 200, xMin, xMax, 200, yMin, yMax);
  xyPosition->SetBit(kCanDelete);
  xyPosition->SetXTitle("x position [cm]");
  xyPosition->SetYTitle("y position [cm]");
  xyPosition->SetZTitle("counts");
  xyPosition->SetFillColor(8);

  while ((Event = EventFile.GetNextEvent(false)) != 0) {

//     bool IsZBelow95 = false;
//     for (unsigned int h = 0; h < Event->GetNHTs(); ++h) {
//       if (Event->GetHTAt(h)->GetPosition()[2] < 95) {
//         IsZBelow95 = true;
//         break;
//       }
//     } 
    
//     if (IsZBelow95 == false) {
      for (unsigned int h = 0; h < Event->GetNHTs(); ++h) {
        xyPosition->Fill(Event->GetHTAt(h)->GetPosition()[0],
                         Event->GetHTAt(h)->GetPosition()[1]);
        zPosition->Fill(Event->GetHTAt(h)->GetPosition()[2]);
      }
//     }

    delete Event;

//     if (Event->GetID() > 50000) break;
  }

  TCanvas* zCanvas = new TCanvas("z Position Canvas", "z Position Canvas", 800, 600);
  zCanvas->cd();
  zPosition->Draw();
  zCanvas->Update();

  TCanvas* xyCanvas = new TCanvas("xy Position Canvas", "xy Position Canvas", 800, 600);
  xyCanvas->cd();
  xyPosition->Draw("colz");
  xyCanvas->Update();
}


////////////////////////////////////////////////////////////////////////////////


void MInterfaceSivan::MissingInteractionsStatistics()
{
  // Open the simulation file:
  MFileEventsSim EventFile(m_Geometry);
  if (EventFile.Open(m_Data->GetCurrentFileName()) == false) {
    mgui<<"Unable to open file"<<error;
    return;
  }
  EventFile.ShowProgress();

  double xMin = numeric_limits<double>::max();
  double yMin = numeric_limits<double>::max();
  double zMin = numeric_limits<double>::max();

  double xMax = -numeric_limits<double>::max();
  double yMax = -numeric_limits<double>::max();
  double zMax = -numeric_limits<double>::max();
  
  MVector Pos;
  MSimEvent* Event;
  MSimIA* IA = 0;
  int MaxNHits = 10000;
  mdebug<<"Testing dimensions of geometry..."<<endl;
  while ((Event = EventFile.GetNextEvent(false)) != 0) {
    for (unsigned int h = 0; h < Event->GetNIAs(); ++h) {
      IA = Event->GetIAAt(h);
      if (IA->GetProcess() != "INIT" && IA->GetProcess() != "ESCP") {
        Pos = IA->GetPosition();
  
        if (Pos[0] > xMax) xMax = Pos[0];
        if (Pos[1] > yMax) yMax = Pos[1];
        if (Pos[2] > zMax) zMax = Pos[2];
        if (Pos[0] < xMin) xMin = Pos[0];
        if (Pos[1] < yMin) yMin = Pos[1];
        if (Pos[2] < zMin) zMin = Pos[2];
        MaxNHits--;
      }
    }
    
    delete Event;
    if (MaxNHits <= 0) break;
  }

  mdebug<<"x:"<<xMin<<" - "<<xMax<<endl;
  mdebug<<"y:"<<yMin<<" - "<<yMax<<endl;
  mdebug<<"z:"<<zMin<<" - "<<zMax<<endl;

  double Diff = 0.1*(xMax-xMin);
  xMax += Diff;
  xMin -= Diff;
  Diff = 0.1*(yMax-yMin);
  yMax += Diff;
  yMin -= Diff;
  Diff = 0.1*(zMax-zMin);
  zMax += Diff;
  zMin -= Diff;

  xMin = TMath::Floor(xMin);
  xMax = TMath::Ceil(xMax);
  yMin = TMath::Floor(yMin);
  yMax = TMath::Ceil(yMax);
  zMin = TMath::Floor(zMin);
  zMax = TMath::Ceil(zMax);

  mdebug<<endl;
  mdebug<<"Setting dimensions of geometry to:"<<endl;
  mdebug<<"  x: "<<xMin<<" - "<<xMax<<endl;
  mdebug<<"  y: "<<yMin<<" - "<<yMax<<endl;
  mdebug<<"  z: "<<zMin<<" - "<<zMax<<endl;
  mdebug<<endl;


  TH1D* zPosAll = new TH1D("z position of all missing IAs", 
                           "z position of all missing IAs", 200, zMin, zMax);
  zPosAll->SetBit(kCanDelete);
  zPosAll->SetXTitle("z [cm]");
  zPosAll->SetYTitle("counts");
  zPosAll->SetFillColor(8);

  TH2D* xyPosAll = new TH2D("xy Position of all missing IAs", 
                            "xy Position of all missing IAs", 200, xMin, xMax, 200, yMin, yMax);
  xyPosAll->SetBit(kCanDelete);
  xyPosAll->SetXTitle("x position [cm]");
  xyPosAll->SetYTitle("y position [cm]");
  xyPosAll->SetZTitle("counts");
  xyPosAll->SetFillColor(8);

  TH3D* xyzPosAll = new TH3D("xyz Position of all missing IAs", 
                            "xyz Position of all missing IAs", 100, xMin, xMax, 100, yMin, yMax, 100, zMin, zMax);
  xyzPosAll->SetBit(kCanDelete);
  xyzPosAll->SetXTitle("x position [cm]");
  xyzPosAll->SetYTitle("y position [cm]");
  xyzPosAll->SetZTitle("z position [cm]");
  xyzPosAll->SetFillColor(8);

  TH1D* zPosStart = new TH1D("z position of missing IAs at the start of the event", 
                             "z position of missing IAs at the start of the event", 200, zMin, zMax);
  zPosStart->SetBit(kCanDelete);
  zPosStart->SetXTitle("z [cm]");
  zPosStart->SetYTitle("counts");
  zPosStart->SetFillColor(8);

  TH2D* xyPosStart = new TH2D("xy Position of missing IAs at the start of the event", 
                              "xy Position of missing IAs at the start of the event", 200, xMin, xMax, 200, yMin, yMax);
  xyPosStart->SetBit(kCanDelete);
  xyPosStart->SetXTitle("x position [cm]");
  xyPosStart->SetYTitle("y position [cm]");
  xyPosStart->SetZTitle("counts");
  xyPosStart->SetFillColor(8);

  TH3D* xyzPosStart = new TH3D("xyz Position missing IAs at the start of the event", 
                               "xyz Position missing IAs at the start of the event", 100, xMin, xMax, 100, yMin, yMax, 100, zMin, zMax);
  xyzPosStart->SetBit(kCanDelete);
  xyzPosStart->SetXTitle("x position [cm]");
  xyzPosStart->SetYTitle("y position [cm]");
  xyzPosStart->SetZTitle("z position [cm]");
  xyzPosStart->SetFillColor(8);

  int NEvents = 0;
  double EscapedEnergy = 0.0;
  double NSEnergy = 0.0;
  map<MString, int> Materials;
  map<MString, double> EnergyInPassiveMaterial;

  EventFile.Rewind();
  while ((Event = EventFile.GetNextEvent(false)) != 0) {
    for (unsigned int i = 1; i < Event->GetNIAs(); ++i) {
      if (Event->GetIAAt(i)->GetProcess() != "INIT" && 
          Event->GetIAAt(i)->GetProcess() != "ESCP") {
        if (Event->GetIAAt(i)->GetDetectorType() == 0) {
          zPosAll->Fill(Event->GetIAAt(i)->GetPosition()[2]);
          xyPosAll->Fill(Event->GetIAAt(i)->GetPosition()[0], Event->GetIAAt(i)->GetPosition()[1]);
          xyzPosAll->Fill(Event->GetIAAt(i)->GetPosition()[0], Event->GetIAAt(i)->GetPosition()[1], Event->GetIAAt(i)->GetPosition()[2]);
          if (i == 1) {
            zPosStart->Fill(Event->GetIAAt(i)->GetPosition()[2]);
            xyPosStart->Fill(Event->GetIAAt(i)->GetPosition()[0], Event->GetIAAt(i)->GetPosition()[1]);
            xyzPosStart->Fill(Event->GetIAAt(i)->GetPosition()[0], Event->GetIAAt(i)->GetPosition()[1], Event->GetIAAt(i)->GetPosition()[2]);
          }
          MDVolumeSequence* S = m_Geometry->GetVolumeSequencePointer(Event->GetIAAt(i)->GetPosition());
          Materials[S->GetDeepestVolume()->GetMaterial()->GetName()]++;
          delete S;
        }
      }
      if (Event->GetIAAt(i)->GetProcess() == "ESCP") {
        EscapedEnergy += Event->GetIAAt(i)->GetMotherEnergy();
      }
    }
    for (unsigned int p = 0; p < Event->GetNPMs(); ++p) {
      EnergyInPassiveMaterial[Event->GetPMAt(p)->GetMaterialName()] += Event->GetPMAt(p)->GetEnergy();
    }
    NSEnergy += Event->GetEnergyDepositNotSensitiveMaterial();
    NEvents++;

    delete Event;
  }

  TCanvas* CanvasAll = new TCanvas("zPosAll Canvas", "zPosAll Canvas", 800, 600);
  CanvasAll->cd();
  zPosAll->Draw();
  CanvasAll->Update();

  TCanvas* CanvasStart = new TCanvas("zPosStart Canvas", "zPosStart Canvas", 800, 600);
  CanvasStart->cd();
  zPosStart->Draw();
  CanvasStart->Update();

  TCanvas* CanvasxyAll = new TCanvas("xyPosAll Canvas", "xyPosAll Canvas", 800, 600);
  CanvasxyAll->cd();
  xyPosAll->Draw("colz");
  CanvasxyAll->Update();

  TCanvas* CanvasxyStart = new TCanvas("xyPosStart Canvas", "xyPosStart Canvas", 800, 600);
  CanvasxyStart->cd();
  xyPosStart->Draw("colz");
  CanvasxyStart->Update();

  TCanvas* CanvasxyzAll = new TCanvas("xyzPosAll Canvas", "xyzPosAll Canvas", 800, 600);
  CanvasxyzAll->cd();
  xyzPosAll->Draw("colz");
  CanvasxyzAll->Update();

  TCanvas* CanvasxyzStart = new TCanvas("xyzPosStart Canvas", "xyzPosStart Canvas", 800, 600);
  CanvasxyzStart->cd();
  xyzPosStart->Draw("colz");
  CanvasxyzStart->Update();

  map<MString, int>::iterator MaterialsIter;
  mout<<endl;
  mout<<"Materials of passive material interaction:"<<endl;

  int AllHits = 0;
  for (MaterialsIter = (Materials.begin()); 
       MaterialsIter != Materials.end(); MaterialsIter++) {
    AllHits += (*MaterialsIter).second;
  }
  for (MaterialsIter = (Materials.begin()); 
       MaterialsIter != Materials.end(); MaterialsIter++) {
    cout<<setw(24)<<(*MaterialsIter).first<<": "<<setw(10)<<(*MaterialsIter).second
        <<" ("<<setprecision(4)<<setw(9)<<double((*MaterialsIter).second)/AllHits*100.0<<"%)"<<endl;
  }

  if (NEvents > 0) {
    mout<<endl;
    mout<<"Energy deposit per passive material (average: "<<NSEnergy/NEvents<<")"<<endl;
    map<MString, double>::iterator PMIter;
    for (PMIter = EnergyInPassiveMaterial.begin();
         PMIter != EnergyInPassiveMaterial.end();
         PMIter++) {
      cout<<setw(24)<<(*PMIter).first<<": "<<setw(10)<<(*PMIter).second/NEvents<<" keV"<<endl;
    }

    mout<<endl;
    mout<<"Average escaped energy: "<<EscapedEnergy/NEvents<<" keV"<<endl;
  }

  mout<<endl;
  mout<<endl;
  mout<<"Remarks/restrictions: "<<endl;
  mout<<"  * This statistics does only count interactions in passive material."<<endl;
  mout<<"    It does not take into account events which moved from active to"<<endl;
  mout<<"    passive material nor hits below the thresholds!"<<endl;

  return;
}


////////////////////////////////////////////////////////////////////////////////


void MInterfaceSivan::DopplerArm()
{
  // Open the simulation file:
  MFileEventsSim EventFile(m_Geometry);
  if (EventFile.Open(m_Data->GetCurrentFileName()) == false) {
    mgui<<"Unable to open file"<<error;
    return;
  }
  EventFile.ShowProgress();

  int NBins = 201;
  int MaxAngle = 5;

  TH1D* AngleD1 = new TH1D("DopplerARMD1", "Doppler in 2D strip (ARM)", NBins, -MaxAngle, MaxAngle);
  AngleD1->SetBit(kCanDelete);
  AngleD1->SetXTitle("ARM [#circ]");
  AngleD1->SetYTitle("counts");
  AngleD1->SetFillColor(8);

  TH1D* AngleD2 = new TH1D("DopplerARMD2", "Doppler in calorimeter (ARM)", NBins, -MaxAngle, MaxAngle);
  AngleD2->SetBit(kCanDelete);
  AngleD2->SetXTitle("ARM [#circ]");
  AngleD2->SetYTitle("counts");
  AngleD2->SetFillColor(8);

  TH1D* AngleD3 = new TH1D("DopplerARMD3", "Doppler in 3D strip (ARM)", NBins, -MaxAngle, MaxAngle);
  AngleD3->SetBit(kCanDelete);
  AngleD3->SetXTitle("ARM [#circ]");
  AngleD3->SetYTitle("counts");
  AngleD3->SetFillColor(8);

  double PhiAngle;
  double PhiEnergy;
  MSimEvent* Event;
  while ((Event = EventFile.GetNextEvent(false)) != 0) {
    if (Event->GetNIAs() >= 3) {
      if (Event->GetIAAt(1)->GetProcess() == "COMP" && Event->GetIAAt(2)->GetOriginID() == 1) {
        PhiAngle = Event->GetIAAt(0)->GetSecondaryDirection().Angle(Event->GetIAAt(2)->GetPosition() -  Event->GetIAAt(1)->GetPosition())*c_Deg;
        PhiEnergy = 1 - 511.004 * (1/(Event->GetIAAt(0)->GetSecondaryEnergy()-Event->GetIAAt(1)->GetSecondaryEnergy()) - 1/Event->GetIAAt(0)->GetSecondaryEnergy());
        if (PhiEnergy >= -1 && PhiEnergy < 1) {
          PhiEnergy = acos(PhiEnergy)*c_Deg;
          if (Event->GetIAAt(1)->GetDetectorType() == 1) {
            //cout<<"D1"<<endl;
            AngleD1->Fill(PhiAngle-PhiEnergy);
          } if (Event->GetIAAt(1)->GetDetectorType() == 2) {
            //cout<<"D2"<<endl;
            AngleD2->Fill(PhiAngle-PhiEnergy);
          } if (Event->GetIAAt(1)->GetDetectorType() == 3) {
            //cout<<"D3"<<endl;
            AngleD3->Fill(PhiAngle-PhiEnergy);
          }
          //cout<<Event->GetIAAt(1)->ToSimString(21)<<endl;
        }
      }
    }
    delete Event;
  }

  if (AngleD1->GetSum() > 0) {
    TCanvas* CanvasD1 = new TCanvas("Incidence Angle Canvas D1", "Incidence Angle Canvas D1", 800, 600);
    CanvasD1->cd();
    AngleD1->Draw();
    CanvasD1->Update();
  }


  if (AngleD2->GetSum() > 0) {
    TCanvas* CanvasD2 = new TCanvas("Incidence Angle Canvas D2", "Incidence Angle Canvas D2", 800, 600);
    CanvasD2->cd();
    AngleD2->Draw();
    CanvasD2->Update();
  }

  if (AngleD3->GetSum() > 0) {
    TCanvas* CanvasD3 = new TCanvas("Incidence Angle Canvas D3", "Incidence Angle Canvas D3", 800, 600);
    CanvasD3->cd();
    AngleD3->Draw();
    CanvasD3->Update();
  }

  mout<<endl;
  mout<<endl;
  mout<<"Remarks/restrictions: "<<endl;
  mout<<"  * None"<<endl;

  return;
}


////////////////////////////////////////////////////////////////////////////////


void MInterfaceSivan::InteractionDetectorSequence()
{
  // 

  mout<<endl;
  mout<<"Determining interaction detector-type sequence for the *initial* particle..."<<endl;
  mout<<endl;

  // Open the simulation file:
  MFileEventsSim EventFile(m_Geometry);
  if (EventFile.Open(m_Data->GetCurrentFileName()) == false) {
    mgui<<"Unable to open file"<<error;
    return;
  }
  EventFile.ShowProgress();

  // First create a list of all named detectors
  vector<int> FirstHit(m_Geometry->GetNDetectors(), 0);
  vector<vector<int> > HitSequence(m_Geometry->GetNDetectors(), FirstHit);

  unsigned int IndexPassiveMaterial = m_Geometry->GetNDetectors();
  vector<int> FirstHitIdeal(m_Geometry->GetNDetectors()+1, 0);
  map<vector<int>, int> HitSequenceIdeal;

  MSimEvent* Event;
  while ((Event = EventFile.GetNextEvent(false)) != 0) {

    // Part A:
    unsigned int LowestIndex = numeric_limits<unsigned int>::max();
    int Origin = numeric_limits<int>::max();
    for (unsigned int h = 0; h < Event->GetNHTs(); ++h) {
      if (Event->GetHTAt(h)->GetSmallestOrigin(1) < Origin) {
        Origin = Event->GetHTAt(h)->GetSmallestOrigin(1);
        LowestIndex = h;
      }
    }
    if (Origin != numeric_limits<int>::max()) {
      unsigned int DetectorLowestIndex = m_Geometry->GetDetectorIndex(Event->GetHTAt(LowestIndex)->GetVolumeSequence()->GetDetector()->GetName());
      FirstHit[DetectorLowestIndex]++;
      
      // Find smallest origin higher than this:
      unsigned int SecondLowestIndex = numeric_limits<unsigned int>::max();
      int SecondOrigin = numeric_limits<int>::max();
      for (unsigned int h = 0; h < Event->GetNHTs(); ++h) {
        for (unsigned int s = 0; s < Event->GetHTAt(h)->GetNOrigins(); ++s) {
          if (Event->GetHTAt(h)->GetOriginAt(s) < SecondOrigin && 
              Event->GetHTAt(h)->GetOriginAt(s) > Origin) {
            SecondOrigin = Event->GetHTAt(h)->GetSmallestOrigin(1);
            SecondLowestIndex = h;
          }
        }
      }
      
      if (SecondOrigin != numeric_limits<int>::max()) {
        unsigned int DetectorSecondLowestIndex = m_Geometry->GetDetectorIndex(Event->GetHTAt(SecondLowestIndex)->GetVolumeSequence()->GetDetector()->GetName());
        HitSequence[DetectorLowestIndex][DetectorSecondLowestIndex]++;
      }
    }

    // Part B:
    MDVolumeSequence V;
    if (Event->GetNIAs() > 1) {
      // Set the initial detector type
      V = m_Geometry->GetVolumeSequence(Event->GetIAAt(1)->GetPosition());
      if (V.GetDetector() != 0) {
        FirstHitIdeal[m_Geometry->GetDetectorIndex(V.GetDetector()->GetName())]++;
      } else {
        FirstHitIdeal[IndexPassiveMaterial]++;        
      }
      // Now generate the sequence:
      vector<int> Sequence;
      MString LastType = Event->GetIAAt(0)->GetProcess();
      int LastOrigin = Event->GetIAAt(0)->GetOriginID();
      MVector LastPosition = Event->GetIAAt(0)->GetPosition();
      
      for (unsigned int i = 1; i < Event->GetNIAs(); ++i) {
        if (Event->GetIAAt(i)->GetOriginID() != 1) continue;
        if (Event->GetIAAt(i)->GetProcess() == "ESCP") continue;
        // Don't count if type, position, and mother is identical to the previous
        // This takes care of PHOT, PAIR, etc. generating several particles
        if (Event->GetIAAt(i)->GetProcess() != LastType || 
            Event->GetIAAt(i)->GetOriginID() != LastOrigin ||
            Event->GetIAAt(i)->GetPosition() != LastPosition) {
          V = m_Geometry->GetVolumeSequence(Event->GetIAAt(i)->GetPosition());
          if (V.GetDetector() != 0) {
            Sequence.push_back(m_Geometry->GetDetectorIndex(V.GetDetector()->GetName()));
          } else {
            Sequence.push_back(IndexPassiveMaterial);
          }

          LastType = Event->GetIAAt(i)->GetProcess();
          LastOrigin = Event->GetIAAt(i)->GetOriginID();
          LastPosition = Event->GetIAAt(i)->GetPosition();
        }
      }

      // Just add, map takes care of the housekeeping
      HitSequenceIdeal[Sequence]++;
    }

    delete Event;
  }  

  // Dump the data:
  

  mout<<"Part A: Analysis based on measured data --- excludes hits in passive material"<<endl;

  mout<<endl;
  mout<<"Location of first *detected* interaction: "<<endl;
  for (unsigned int d = 0; d < FirstHit.size(); ++d) {
    if (FirstHit[d] > 0) {
      mout<<m_Geometry->GetDetectorAt(d)->GetName()<<": "<<FirstHit[d]<<endl;
    }
  }
  
  mout<<endl;
  mout<<"Interaction sequence by detector type of measured events: "<<endl;
  for (unsigned int h = 0; h < HitSequence.size(); ++h) {
    for (unsigned int d = 0; d < HitSequence[h].size(); ++d) {
      if (HitSequence[h][d] > 0) {
        mout<<m_Geometry->GetDetectorAt(h)->GetName()<<"->"<<m_Geometry->GetDetectorAt(d)->GetName()<<": "<<HitSequence[h][d]<<endl;
      }
    }
  }

  mout<<endl;
  mout<<"Part B: Analysis based on ideal (unmeasured) data --- includes hits in passive material"<<endl;
  mout<<endl;


  mout<<"Location of first interaction (does not need to be detected): "<<endl;
  for (unsigned int d = 0; d < FirstHitIdeal.size(); ++d) {
    if (FirstHit[d] > 0) {
      if (d == IndexPassiveMaterial) {
        mout<<"Passive material: "<<FirstHit[d]<<endl;
      } else {
        mout<<m_Geometry->GetDetectorAt(d)->GetName()<<": "<<FirstHit[d]<<endl;
      }
    }
  }
  
  mout<<endl;
  mout<<"Interaction sequence by detector type of measured events: "<<endl;
  // Determine threshold:
  int NEvents = 0;
  for (map<vector<int>, int>::iterator I = HitSequenceIdeal.begin();
       I != HitSequenceIdeal.end(); ++I) {
    NEvents += (*I).second;
  }
  int Threshold = NEvents/1000;
  int Excluded = 0;
  for (map<vector<int>, int>::iterator I = HitSequenceIdeal.begin();
       I != HitSequenceIdeal.end(); ++I) {
    if ((*I).second > Threshold) {
      for (unsigned int d = 0; d < (*I).first.size(); ++d) {
        if ((*I).first[d] == int(IndexPassiveMaterial)) {
          mout<<" Passive ";
        } else {
          mout<<" "<<m_Geometry->GetDetectorAt((*I).first[d])->GetName()<<" ";
        }
        if (d < (*I).first.size() - 1) {
          mout<<"->";
        }
      }
      mout<<": "<<(*I).second<<endl;
    } else {
      Excluded += (*I).second;
    }
  }
  mout<<endl;
  mout<<"Excluded (rare) sequence counts: "<<Excluded<<" of "<<NEvents<<endl;

  mout<<endl;
  mout<<"Remarks for part B:"<<endl;
  mout<<" * This includes only the path of the primary particle - not the secondaries!"<<endl;
  mout<<" * All interactions at the same position and of same type are considered as one"<<endl;
  mout<<"   (thus taking care of PHOT, PAIR, etc. generating several particles)"<<endl;
  mout<<" * Escapes are not counted"<<endl;
  mout<<endl;

}


////////////////////////////////////////////////////////////////////////////////


void MInterfaceSivan::ChargeSharing()
{
  // Analyze the simulated charge sharing between strips

  // Open the simulation file:
  MFileEventsSim EventFile(m_Geometry);
  if (EventFile.Open(m_Data->GetCurrentFileName()) == false) {
    mgui<<"Unable to open file"<<error;
    return;
  }
  EventFile.ShowProgress();

  TH2D* ChargeSharingHisto = new TH2D("ChargeSharing", "Charge Sharing", 40, 0, 2000, 5, 0.5, 5.5);

  MSimEvent* Event;
  while ((Event = EventFile.GetNextEvent(false)) != 0) {
    for (unsigned int i = 0; i < Event->GetNIAs(); ++i) {
      // Only consider Compton recoil electrons
      if (Event->GetIAAt(i)->GetProcess() == "COMP") {
        // Check if no other IAs contaminate this IA:
        //if (Event->GetIAAt(i)->GetSecondaryEnergy() > 1000) cout<<Event->GetID()<<" enough deposit:"<<endl;
        if (Event->IsIAResolved(Event->GetIAAt(i)->GetID()) == true) {
          //if (Event->GetIAAt(i)->GetSecondaryEnergy() > 1000) cout<<Event->GetIAAt(i)->GetID()<<": Resolved!"<<endl;
          // Check if the IA is completely absorbed:
          if (Event->IsIACompletelyAbsorbed(Event->GetIAAt(i)->GetID(), 2.0, 0.02) == 0) {
            // Find all HTs which originate from this IA
            vector<MSimHT*> HTs = Event->GetHTListIncludingDescendents(Event->GetIAAt(i)->GetID());
            //if (Event->GetIAAt(i)->GetSecondaryEnergy() > 1000) cout<<Event->GetIAAt(i)->GetID()<<": Completely absorbed with "<<HTs.size()<<" HTs"<<endl;
            ChargeSharingHisto->Fill(Event->GetIAAt(i)->GetSecondaryEnergy(), HTs.size());
          } else {
            //if (Event->GetIAAt(i)->GetSecondaryEnergy() > 1000) cout<<Event->GetIAAt(i)->GetID()<<": Not completely absorbed "<<endl;
          }
        } else {
          //if (Event->GetIAAt(i)->GetSecondaryEnergy() > 1000) cout<<Event->GetIAAt(i)->GetID()<<": Not resolved!"<<endl;
        }
      }
    }
  }

  TCanvas* ChargeSharingCanvas = new TCanvas("ChargeSharingCanvas", "Charge Sharing Canvas");
  ChargeSharingCanvas->cd();
  ChargeSharingHisto->Draw("colz");
  ChargeSharingCanvas->Update();
  
  cout<<"Charge sharing summary: "<<endl;

  cout<<"Remarks:"<<endl;
  cout<<"  + Only Compton recoil electrons are considered"<<endl;
  cout<<"  + They need to be completely absorbed (+-2%)"<<endl;
  cout<<"  + Different detector types are not distinguished!"<<endl;
  cout<<"  + If Bremsstrahlung photons are emitted, the hit is not ignored!"<<endl;
  cout<<"    --> This tools is currently most valuable for Ge-ACT/NCT ..."<<endl;
}


////////////////////////////////////////////////////////////////////////////////


void MInterfaceSivan::InteractionsPerVoxel()
{
  // Analyze the simulated charge sharing between strips

  // Open the simulation file:
  MFileEventsSim EventFile(m_Geometry);
  if (EventFile.Open(m_Data->GetCurrentFileName()) == false) {
    mgui<<"Unable to open file"<<error;
    return;
  }
  EventFile.ShowProgress();

  unsigned int NHits = 0;
  unsigned int NIAs = 0;
  unsigned int NIACompton = 0;
  unsigned int NIAPhoto = 0;
  unsigned int NIAPair = 0;
  unsigned int NIARayleigh = 0;

  bool MultipleComptonAdded = false;
  bool ComptonFound = false;
  bool ComptonPhotoAdded = false;
  bool PhotoFound = false;
  unsigned int NIAMultipleCompton = 0;
  unsigned int NIAComptonPhoto = 0;

  MSimEvent* Event;
  while ((Event = EventFile.GetNextEvent(false)) != 0) {
    //cout<<"EN: "<<Event->GetID()<<endl;
    for (unsigned int h = 0; h < Event->GetNHTs(); ++h) {
      MSimHT* HT = Event->GetHTAt(h);
      NHits++;
      ComptonFound = false;
      MultipleComptonAdded = false;
      PhotoFound = false;
      ComptonPhotoAdded = false;
      for (unsigned int i = 0; i < HT->GetNOrigins(); ++i) {
        if (HT->GetOriginAt(i) == 1) continue;
        NIAs++;
        MSimIA* IA = Event->GetIAById(HT->GetOriginAt(i));
        if (IA->GetProcess() == "COMP") {
          if (ComptonFound == true && MultipleComptonAdded == false) {
            NIAMultipleCompton++;
            //cout<<"Adding multiple Compton"<<endl;
            MultipleComptonAdded = true;
          }
          if (PhotoFound == true && ComptonPhotoAdded == false) {
            NIAComptonPhoto++;
            //cout<<"Adding Compton+Photo"<<endl;
            ComptonPhotoAdded = true;
          }
          //cout<<"Adding Compton"<<endl;
          NIACompton++;
          ComptonFound = true;
        } else if (IA->GetProcess() == "PAIR") {
          //cout<<"Adding pair"<<endl;
          NIAPair++;
        } else if (IA->GetProcess() == "PHOT") {
          if (ComptonFound == true && MultipleComptonAdded == false) {
            //cout<<"Adding multiple Compton"<<endl;
            NIAMultipleCompton++;
            MultipleComptonAdded = true;
          }
          NIAPhoto++;
          //cout<<"Adding Photo"<<endl;
          PhotoFound = true;
        } else if (IA->GetProcess() == "RAYL") {
          NIARayleigh++;
          //cout<<"Adding Rayleigh"<<endl;
        }
      }
    }
  }

  if (NHits > 0) {
    cout<<"Interactions per voxel: "<<endl;
    cout<<endl;
    cout<<"All interactions per voxel:                     "<<1.0*NIAs/NHits<<" "<<endl;
    cout<<"Comptons interactions per voxel:                "<<1.0*NIACompton/NHits<<" "<<endl;
    cout<<"Photon-effect interactions per voxel:           "<<1.0*NIAPhoto/NHits<<" "<<endl;
    cout<<"Pair-creations interactions per voxel:          "<<1.0*NIAPair/NHits<<" "<<endl;
    cout<<"Rayleigh-scattering interactions per voxel:     "<<1.0*NIARayleigh/NHits<<" "<<endl;
    cout<<endl;
    cout<<"Multiple Compton interactions per voxel:        "<<1.0*NIAMultipleCompton/NHits<<" "<<endl;
    cout<<"Compton and photo interactions per voxel:       "<<1.0*NIAComptonPhoto/NHits<<" "<<endl;
    cout<<endl;
  } else {
    cout<<"No hits in the file and thus no interactions!"<<endl;
  }

  cout<<"Remarks:"<<endl;
  cout<<"  + Currently only works for voxel detectors, not for strips!"<<endl;
}


////////////////////////////////////////////////////////////////////////////////


void MInterfaceSivan::StartLocations()
{
  // Show where the photos are started in 3D
  
  bool UseEnergy = false;
  
  // Open the simulation file:
  MFileEventsSim EventFile(m_Geometry);
  if (EventFile.Open(m_Data->GetCurrentFileName()) == false) {
    mgui<<"Unable to open file"<<error;
    return;
  }
  EventFile.ShowProgress();

  double xMin = -100;
  double xMax = +100;
  double yMin = -100;
  double yMax = +100;
  double zMin = -100;
  double zMax = +100;

  int MaxNBins = 200;

  unsigned int MaxNPositions = 10000000;
  vector<MVector> Positions;
  vector<double> Energies;


  // Step 1: Accumulate many, many hits:

  MSimEvent* Event;
  while ((Event = EventFile.GetNextEvent(false)) != 0) {
    // categorize the beam into layers:
    if (Event->GetNIAs() > 0) {
      Positions.push_back(Event->GetIAAt(0)->GetPosition());
      Energies.push_back(Event->GetIAAt(0)->GetSecondaryEnergy());
    }
    
    delete Event;

    if (Positions.size() > MaxNPositions) {
      break;
    }
  }

  // Step 2: Create the histograms

  DetermineAxis(xMin, xMax, yMin, yMax, zMin, zMax, Positions);

  MString HistName;
  MString HistTitle;
  MString ValueAxisTitle;
  if (UseEnergy == true) {
    HistName = "SpatialEnergyDistribution ";
    HistTitle = "Spatial energy distribution ";
    ValueAxisTitle = "Energy [keV] per bin";
  } else {
    HistName = "SpatialHitDistribution ";
    HistTitle = "Spatial hit distribution ";
    ValueAxisTitle = "Counts per bin";
  }

  TH3D* xyzHist = new TH3D(HistName + "xyz", 
                           HistTitle + "xyz", 
                           MaxNBins, xMin, xMax,
                           MaxNBins, yMin, yMax, 
                           MaxNBins, zMin, zMax);
  xyzHist->SetBit(kCanDelete);
  xyzHist->GetXaxis()->SetTitle("x [cm]");
  xyzHist->GetYaxis()->SetTitle("y [cm]");
  xyzHist->GetZaxis()->SetTitle("z [cm]");

  TH2D* xyHist = new TH2D(HistName + "xy", 
                          HistTitle + "xy", 
                          MaxNBins, xMin, xMax,
                          MaxNBins, yMin, yMax);
  xyHist->SetBit(kCanDelete);
  xyHist->GetXaxis()->SetTitle("x [cm]");
  xyHist->GetYaxis()->SetTitle("y [cm]");
  xyHist->GetZaxis()->SetTitle(ValueAxisTitle);

  TH2D* xzHist = new TH2D(HistName + "xz", 
                          HistTitle + "xz", 
                          MaxNBins, xMin, xMax,
                          MaxNBins, zMin, zMax);
  xzHist->SetBit(kCanDelete);
  xzHist->GetXaxis()->SetTitle("x [cm]");
  xzHist->GetYaxis()->SetTitle("z [cm]");
  xzHist->GetZaxis()->SetTitle(ValueAxisTitle);

  TH2D* yzHist = new TH2D(HistName + "yz", 
                          HistTitle + "yz", 
                          MaxNBins, yMin, yMax,
                          MaxNBins, zMin, zMax);
  yzHist->SetBit(kCanDelete);
  yzHist->GetXaxis()->SetTitle("y [cm]");
  yzHist->GetYaxis()->SetTitle("z [cm]");
  yzHist->GetZaxis()->SetTitle(ValueAxisTitle);

  TH1D* xHist = new TH1D(HistName + "x", 
                         HistTitle + "x", 
                         MaxNBins, xMin, xMax);
  xHist->SetBit(kCanDelete);
  xHist->GetXaxis()->SetTitle("x [cm]");
  xHist->GetYaxis()->SetTitle(ValueAxisTitle);

  TH1D* yHist = new TH1D(HistName + "y", 
                         HistTitle + "y", 
                         MaxNBins, yMin, yMax);
  yHist->SetBit(kCanDelete);
  yHist->GetXaxis()->SetTitle("y [cm]");
  yHist->GetYaxis()->SetTitle(ValueAxisTitle);

  TH1D* zHist = new TH1D(HistName + "z", 
                         HistTitle + "z", 
                         MaxNBins, zMin, zMax);
  zHist->SetBit(kCanDelete);
  zHist->GetXaxis()->SetTitle("z [cm]");
  zHist->GetYaxis()->SetTitle(ValueAxisTitle);


  // Step 3: Fill the current events:

  MVector Pos;
  double Energy = 1.0;
  for (unsigned int p = 0; p < Positions.size(); ++p) {
    Pos = Positions[p];
    if (UseEnergy == true) Energy = Energies[p];

    xyzHist->Fill(Pos[0], Pos[1], Pos[2], Energy);
    xyHist->Fill(Pos[0], Pos[1], Energy);
    xzHist->Fill(Pos[0], Pos[2], Energy);
    yzHist->Fill(Pos[1], Pos[2], Energy);
    xHist->Fill(Pos[0], Energy);
    yHist->Fill(Pos[1], Energy);
    zHist->Fill(Pos[2], Energy);
  }


  // Step 4: Continue filling from file:

  while ((Event = EventFile.GetNextEvent(false)) != 0) {
    // categorize the beam into layers:
    if (Event->GetNIAs() > 0) {
      Pos = Event->GetIAAt(0)->GetPosition();
      if (UseEnergy == true) Energy = Event->GetIAAt(0)->GetSecondaryEnergy();
      
      xyzHist->Fill(Pos[0], Pos[1], Pos[2], Energy);
      xyHist->Fill(Pos[0], Pos[1], Energy);
      xzHist->Fill(Pos[0], Pos[2], Energy);
      yzHist->Fill(Pos[1], Pos[2], Energy);
      xHist->Fill(Pos[0], Energy);
      yHist->Fill(Pos[1], Energy);
      zHist->Fill(Pos[2], Energy);
     }
    
    delete Event;
  }


  // Step 5: Show the histograms
  TCanvas* xyzCanvas = new TCanvas(HistName + "xyz", HistTitle + "xyz");
  xyzCanvas->cd();
  xyzHist->Draw();
  xyzCanvas->Update();

  TCanvas* xyCanvas = new TCanvas(HistName + "xy", HistTitle + "xy");
  xyCanvas->cd();
  xyHist->Draw("colz");
  xyCanvas->Update();

  TCanvas* xzCanvas = new TCanvas(HistName + "xz", HistTitle + "xz");
  xzCanvas->cd();
  xzHist->Draw("colz");
  xzCanvas->Update();

  TCanvas* yzCanvas = new TCanvas(HistName + "yz", HistTitle + "yz");
  yzCanvas->cd();
  yzHist->Draw("colz");
  yzCanvas->Update();

  TCanvas* xCanvas = new TCanvas(HistName + "x", HistTitle + "x");
  xCanvas->cd();
  xHist->Draw();
  xCanvas->Update();

  TCanvas* yCanvas = new TCanvas(HistName + "y", HistTitle + "y");
  yCanvas->cd();
  yHist->Draw();
  yCanvas->Update();

  TCanvas* zCanvas = new TCanvas(HistName + "z", HistTitle + "z");
  zCanvas->cd();
  zHist->Draw();
  zCanvas->Update();

  return;
}
  

////////////////////////////////////////////////////////////////////////////////


bool EnergyPerNucleusSort(TH1D* H1, TH1D* H2) {
  return (H1->Integral() < H2->Integral());
}

void MInterfaceSivan::EnergyPerNucleus()
{
  // Display the energy per nucleus...

  // Open the simulation file:
  MFileEventsSim EventFile(m_Geometry);
  if (EventFile.Open(m_Data->GetCurrentFileName()) == false) {
    mgui<<"Unable to open file"<<error;
    return;
  }
  EventFile.ShowProgress();

  double EMin = numeric_limits<double>::max();
  double EMax = -numeric_limits<double>::max();
  
  MSimEvent* Event;
  int MaxNHits = 10000;
  mout<<"Testing energy dimensions..."<<endl;
  while ((Event = EventFile.GetNextEvent(false)) != 0) {
    if (Event->GetNIAs() >= 1 && Event->GetREnergy() > 0) {
      if (Event->GetREnergy() < EMin) EMin = Event->GetREnergy();
      if (Event->GetREnergy() > EMax) EMax = Event->GetREnergy();
    }
    
    delete Event;
    if (MaxNHits-- <= 0) break;
  }
  mout<<"E:"<<EMin<<" - "<<EMax<<endl;

  EMax += max(0.2*(EMax-EMin), 10.0);
  EMin -= max(0.2*(EMax-EMin), 10.0);

  EMin = TMath::Ceil(EMin);
  EMax = TMath::Ceil(EMax);
  if (EMin < 0) EMin = 1;

  //EMin = 0;
  //EMax = 300;
  
  mout<<endl;
  mout<<"Setting dimensions to:"<<endl;
  mout<<"x:"<<EMin<<" - "<<EMax<<endl;
  mout<<endl;

  bool IsLog = false;
  if (EMax/EMin > 100) IsLog = true;
  IsLog = false;
  int ENBins = 300;
  double* EBins = CreateAxisBins(EMin, EMax, ENBins, IsLog);

  vector<TH1D*> Histos;
  map<int, TH1D*> HistoMap;

  TH1D* Energy = new TH1D("Energy", "Energy", ENBins, EBins);
  Energy->SetBit(kCanDelete);
  Energy->SetXTitle("Energy [keV]");
  Energy->SetYTitle("counts/keV");
  Energy->SetFillColor(8);

  EventFile.Rewind();
  while ((Event = EventFile.GetNextEvent(false)) != 0) {
    if (Event->GetNIAs() >= 1 && Event->GetREnergy() > 0) {
      if (Event->GetREnergy() > EMin && Event->GetREnergy() < EMax) {
        int ID = 0;
        if (Event->GetIAAt(0)->GetMotherParticleID() != 0) {
          // MGGPOD:
          ID = Event->GetIAAt(0)->GetMotherParticleID();
        } else {
          // Cosima:
          ID = Event->GetIAAt(0)->GetSecondaryParticleID();
        }

        if (HistoMap[ID] == 0) {
          
          MString Title;
          Title += ID;
          
          TH1D* Energy = new TH1D(Title, Title, ENBins, EBins);
          Energy->SetBit(kCanDelete);
          Energy->SetXTitle("Energy [keV]");
          Energy->SetYTitle("counts/keV");
          HistoMap[ID] = Energy;
          Histos.push_back(Energy);
        }
        HistoMap[ID]->Fill(Event->GetREnergy());
      }
    }

    delete Event;
  }

  // Sort by the number of counts
  sort(Histos.begin(), Histos.end(), EnergyPerNucleusSort);  

  THStack* Stack = new THStack("Stack", "Energy spectrum for all involved nuclei");

  TLegend* Legend = new TLegend(0.15,0.35,0.3,0.85, NULL, "brNDC");

  unsigned int Color = Histos.size()+2;
  for (unsigned int i = 0; i < Histos.size(); ++i) {
    // Renormalize to cts/keV:
    for (int b = 1; b <= Histos[i]->GetNbinsX(); ++b) {
      Histos[i]->SetBinContent(b, Histos[i]->GetBinContent(b)/Histos[i]->GetBinWidth(b));
    }

    Histos[i]->SetFillColor(Color--);
    Stack->Add(Histos[i]);
  }
  int Added = 0;
  for (unsigned int i = Histos.size()-1; i < Histos.size(); --i) {
    if (Histos.size() - i < 20) {
      Legend->AddEntry(Histos[i], Histos[i]->GetTitle());
      Added++;
    }
  }
  MString LegendTitle = "Top ";
  LegendTitle += Added;
  Legend->SetHeader(LegendTitle);

  // Now draw:
  TCanvas* ECanvas = new TCanvas("EnergyCanvas", "Energy Spectrum", 800, 600);
  if (IsLog == true) ECanvas->SetLogx();
  ECanvas->cd();
  Stack->Draw();
  Stack->GetHistogram()->SetXTitle("Energy [keV]");
  Stack->GetHistogram()->SetYTitle("counts");
  Stack->Draw();
  Legend->Draw();
  ECanvas->Update();

  cout<<"Remarks:"<<endl;
  cout<<"  + If all parent nuclei have the ID 0, then the sim file didn't contain the necessary information!"<<endl;

  delete [] EBins;

  return;
}
  

////////////////////////////////////////////////////////////////////////////////


void MInterfaceSivan::IsotopeGeneration()
{
  // Open the simulation file:
  MFileEventsSim EventFile(m_Geometry);
  if (EventFile.Open(m_Data->GetCurrentFileName()) == false) {
    mgui<<"Unable to open file"<<error;
    return;
  }
  EventFile.ShowProgress();

  
  // Create the history of generated isotopes
  map<int, map<int,int>> IsotopeCountByIncidentParticle;
  MSimEvent* Event = nullptr;
  while ((Event = EventFile.GetNextEvent(false)) != 0) {
    for (unsigned int i = 0; i < Event->GetNIAs(); ++i) {
      int MotherParticleID = Event->GetIAAt(i)->GetMotherParticleID(); 
      int SecondaryParticleID = Event->GetIAAt(i)->GetSecondaryParticleID(); 
      if (Event->GetIAAt(i)->GetProcess() != "DECA") {
        IsotopeCountByIncidentParticle[MotherParticleID][SecondaryParticleID]++; 
      }
    }
    delete Event;
  }
  
  
  // Create histograms by incident particle
  for (auto M = IsotopeCountByIncidentParticle.rbegin(); M != IsotopeCountByIncidentParticle.rend(); ++M) {
    int MotherParticleID = (*M).first;
    
    auto IsotopeCount = (*M).second;
    
    // Generate the dimensions of the histogram
    int MinA = 1000;
    int MaxA = 0;
    int MinZ = 1000;
    int MaxZ = 0;
  
    bool IsotopesFound = false;
    for (auto I = IsotopeCount.begin(); I != IsotopeCount.end(); ++I) {
      int ParticleID = (*I).first;

      int Z = ParticleID / 1000;
      int A = ParticleID % 1000;
      
      if (ParticleID < 1000) continue;
      
      IsotopesFound = true;
    
      if (A < MinA) MinA = A;
      if (A > MaxA) MaxA = A;
    
      if (Z < MinZ) MinZ = Z;
      if (Z > MaxZ) MaxZ = Z;
    }
  
    if (IsotopesFound == false) continue;
  
    // Fill the histogram
    MString Title = "Isotopes generated by ";
    if (MotherParticleID == 4) {
      Title += "protons";
    } else if (MotherParticleID == 6) {
      Title += "neutrons";
    } else if (MotherParticleID == 1) {
      Title += "photons";
    } else if (MotherParticleID == 18) {
      Title += "deuterons";
    } else if (MotherParticleID == 19) {
      Title += "tritiums";
    } else if (MotherParticleID == 20) {
      Title += "He-3's";
    } else if (MotherParticleID == 21) {
      Title += "alphas";
    } else {
      Title += "particle with ID ";
      Title += MotherParticleID;
    }
    
    TH2D* Counts = new TH2D("", Title, MaxA-MinA+1, MinA-0.5, MaxA+0.5, MaxZ-MinZ+1, MinZ-0.5, MaxZ+0.5);
    Counts->SetXTitle("A");
    Counts->SetYTitle("Z");
    Counts->SetZTitle("counts/bin");
    
    for (auto I = IsotopeCount.begin(); I != IsotopeCount.end(); ++I) {
      int ParticleID = (*I).first;
      int Z = ParticleID / 1000;
      int A = ParticleID % 1000;

      Counts->Fill(A, Z, (*I).second);
    }    
  
  
    // Now draw:
    TCanvas* Canvas = new TCanvas();
    Canvas->cd();
    Counts->Draw("colz");
    Canvas->Update();
    
      
    // Add stable elements for reference
    TGeoElementTable Table;
    for (int A = MinA; A <= MaxA; ++A) {
      for (int Z = MinZ; Z <= MaxZ; ++Z) {
        TGeoElementRN* N = Table.GetElementRN(A, Z, 0);
        if (N != nullptr) {
          if (N->HalfLife() == -1) {
            TMarker* M = new TMarker(A, Z, 7);
            M->Draw();
          }
        }
      }
    }
  }
  
  
  return;
}
  

////////////////////////////////////////////////////////////////////////////////


void MInterfaceSivan::ActivationPerIncidenceEnergy()
{
  // Histogram parameters
  int NBins = 60;
  double MinEnergy = 0;
  double MaxEnergy = 300000/c_MeV;


  // Open the simulation file:
  MFileEventsSim EventFile(m_Geometry);
  if (EventFile.Open(m_Data->GetCurrentFileName()) == false) {
    mgui<<"Unable to open file"<<error;
    return;
  }
  EventFile.ShowProgress();

  
  // Create the history of particle energies creating new isotopes
  map<int, vector<double>> IsotopeCountByEnergy;
  map<int, vector<double>> StartCountByEnergy;
  MSimEvent* Event = nullptr;
  while ((Event = EventFile.GetNextEvent(false)) != 0) {
    if (Event->GetNIAs() > 0) {
      int StartParticleID = Event->GetIAAt(0)->GetSecondaryParticleID(); 
      double StartParticleEnergy = Event->GetIAAt(0)->GetSecondaryEnergy();
      StartCountByEnergy[StartParticleID].push_back(StartParticleEnergy);
      
      for (unsigned int i = 1; i < Event->GetNIAs(); ++i) {
        int SecondaryParticleID = Event->GetIAAt(i)->GetSecondaryParticleID(); 
        if (Event->GetIAAt(i)->GetProcess() != "DECA" && SecondaryParticleID > 1000) {
          IsotopeCountByEnergy[StartParticleID].push_back(StartParticleEnergy); 
        }
      }
    }
    delete Event;
  }
  
  // Now create the histograms
  for (auto I = IsotopeCountByEnergy.begin(); I != IsotopeCountByEnergy.end(); ++I) {
    int MotherParticleID = (*I).first;
  
    MString Title = "";
    if (MotherParticleID == 4) {
      Title += "Protons";
    } else if (MotherParticleID == 6) {
      Title += "Neutrons";
    } else if (MotherParticleID == 1) {
      Title += "Photons";
    } else if (MotherParticleID == 18) {
      Title += "Deuterons";
    } else if (MotherParticleID == 19) {
      Title += "Tritiums";
    } else if (MotherParticleID == 20) {
      Title += "He-3's";
    } else if (MotherParticleID == 21) {
      Title += "alphas";
    } else {
      Title += "Particle with ID ";
      Title += MotherParticleID;
    }
  
    TH1D* Started = new TH1D("", Title + ": Simulated particles", NBins, MinEnergy, MaxEnergy);
    Started->SetXTitle("Energy [MeV]");
    Started->SetYTitle("incident particles");
    
    for (auto E: StartCountByEnergy[(*I).first]) { 
      Started->Fill(E/c_MeV, 1);
    }

    TH1D* Activations = new TH1D("", Title + ": Activations Per Energy Bin", NBins, MinEnergy, MaxEnergy);
    Activations->SetXTitle("Energy [MeV]");
    Activations->SetYTitle("activations/simulated particle/MeV");
    
    TH1D* ActivationsPerIncidenceParticle = new TH1D("", Title + ": Activations Per Simulated particle Per Energy Bin", NBins, MinEnergy, MaxEnergy);
    ActivationsPerIncidenceParticle->SetXTitle("Energy [MeV]");
    ActivationsPerIncidenceParticle->SetYTitle("activations/simulated particle/MeV");
  
    for (auto E: (*I).second) { 
      ActivationsPerIncidenceParticle->Fill(E/c_MeV, 1);
      Activations->Fill(E/c_MeV, 1);
    }
    
    for (int bx = 1; bx <= Activations->GetNbinsX(); ++bx) {
      double A = ActivationsPerIncidenceParticle->GetBinContent(bx);
      double S = Started->GetBinContent(bx);
      double W = ActivationsPerIncidenceParticle->GetBinWidth(bx);      
      Activations->SetBinContent(bx, A/W);
      Activations->SetBinError(bx, sqrt(A)/Activations->GetBinWidth(bx));
      if (Started->GetBinContent(bx) > 0) {
        ActivationsPerIncidenceParticle->SetBinContent(bx, A/S/W);
        ActivationsPerIncidenceParticle->SetBinError(bx, sqrt((A*(A+S))/S/S/S)/W);
      } else {
        ActivationsPerIncidenceParticle->SetBinContent(bx, 0);
        ActivationsPerIncidenceParticle->SetBinError(bx, 0);
      }
    }
    
    // Now draw:
    TCanvas* StartedCanvas = new TCanvas();
    StartedCanvas->cd();
    Started->Draw();
    StartedCanvas->Update(); 
    
    TCanvas* ActivationsCanvas = new TCanvas();
    ActivationsCanvas->cd();
    Activations->Draw();
    ActivationsCanvas->Update(); 
    
    TCanvas* ActivationsPerIncidenceParticleCanvas = new TCanvas();
    ActivationsPerIncidenceParticleCanvas->cd();
    ActivationsPerIncidenceParticle->Draw();
    ActivationsPerIncidenceParticleCanvas->Update(); 
  }    
  
  cout<<endl;
  cout<<"Caveats for activation per incidence energy plots:"<<endl;
  cout<<"* The simulation must be done with StoreSimulationInfo all & StoreOnlyTriggeredEvents false"<<endl;
  cout<<"* One of the normalizations is per simulated particle - some of them will not hit the detector at all"<<endl;
  cout<<endl;
  
  return;
}


// MInterfaceSivan: the end...
////////////////////////////////////////////////////////////////////////////////
