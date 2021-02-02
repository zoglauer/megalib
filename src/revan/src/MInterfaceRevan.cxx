/*
 * MInterfaceRevan.cxx
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
// MInterfaceRevan
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MInterfaceRevan.h"

// Standard libs:
#include <vector>
#include <map>
#include <limits>
#include <iostream>
#include <sstream>
#include <iomanip>
using namespace std;

// ROOT libs:
#include "TMath.h"
#include "TROOT.h"
#include "TCanvas.h"
#include "TH1.h"
#include "TH2.h"
#include "TH3.h"
#include "TGraph.h"
#include "TColor.h"
#include "TStyle.h"
#include "TProfile.h"
#include "TGMsgBox.h"
#include "TSystem.h"
#include "TLine.h"


// MEGAlib libs:
#include "MAssert.h"
#include "MStreams.h"
#include "MGlobal.h"
#include "MERCSR.h"
#include "MREHit.h"
#include "MRECluster.h"
#include "MRETrack.h"
#include "MRESE.h"
#include "MRawEventAnalyzer.h"
#include "MDGeometryQuest.h"
#include "MComptonEvent.h"
#include "MDShapeBRIK.h"
#include "MImage3D.h"
#include "MGUIRevanMain.h"
#include "MStreams.h"
#include "MTimer.h"
#include "MERHitClusterizer.h"
#include "MSpectralAnalyzer.h"
#include "MPeak.h"
#include "MIsotope.h"
#include "MPrelude.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MInterfaceRevan)
#endif


////////////////////////////////////////////////////////////////////////////////


MInterfaceRevan::MInterfaceRevan() : MInterface()
{
  // standard constructor

  m_Geometry = 0;
  m_Data = new MSettingsRevan();
  m_BasicGuiData = dynamic_cast<MSettings*>(m_Data);
}


////////////////////////////////////////////////////////////////////////////////


MInterfaceRevan::~MInterfaceRevan()
{
  // default destructor

  m_Analyzer = 0;
  m_Geometry = 0;
}


////////////////////////////////////////////////////////////////////////////////


bool MInterfaceRevan::ParseCommandLine(int argc, char** argv)
{
  ostringstream Usage;
  Usage<<endl;
  Usage<<"  Usage: Revan <options>"<<endl;
  Usage<<endl;
  Usage<<"      -f --filename <filename>:"<<endl;
  Usage<<"             Use this file as evta- or sim-file"<<endl;
  Usage<<"      -g --geometry <filename>:"<<endl;
  Usage<<"             Use this file as geometry-file"<<endl;
  Usage<<"      -c --configuration <filename>.cfg:"<<endl;
  Usage<<"             Use this file as parameter file."<<endl;
  Usage<<"             All other given infromations such as -f and -g overwrite information in the configuration file."<<endl;
  Usage<<"             If no configuration file is give ~/.revan.cfg is used"<<endl;
  Usage<<"      -C --change-configuration <pattern>:"<<endl;
  Usage<<"             Replace any value in the configuration file (-C can be used multiple times)"<<endl;
  Usage<<"             E.g. to change the coincidence window, one would set pattern to:"<<endl;
  Usage<<"             -C CoincidenceWindow=1e-06"<<endl;
  Usage<<endl;
  Usage<<"         --oi:"<<endl;
  Usage<<"             Save the OI information, in case tra files are generated"<<endl;
  Usage<<endl;
  Usage<<"      -a --analyze:"<<endl;
  Usage<<"             Analyze the evta-file given with the -f option, otherwise the file in the configuration file"<<endl;
  Usage<<"      -s --generate spectra:"<<endl;
  Usage<<"             Generate spectra using the options previously set in the GUI and stored in the configuration file"<<endl;
  Usage<<endl;
  Usage<<"      -d --debug:"<<endl;
  Usage<<"             Use debug mode"<<endl;
  Usage<<"      -n --no-gui:"<<endl;
  Usage<<"             Do not use a graphical user interface"<<endl;
  Usage<<"      -h --help:"<<endl;
  Usage<<"             You know the answer..."<<endl;
  Usage<<endl;

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

    // Double argument
    if (Option == "-f" || Option == "--filename" ||
        Option == "-c" || Option == "--configuration" ||
        Option == "-j" || Option == "--jobs" ||
        Option == "-g" || Option == "--geometry") {
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
      g_Verbosity = 2;
      cout<<"Command-line parser: Use debug mode"<<endl;
    } else if (Option == "--configuration" || Option == "-c") {
      m_Data->Read(argv[++i]);
      cout<<"Command-line parser: Use configuration file "<<m_Data->GetSettingsFileName()<<endl;
    }
  }

  // Look if we need to change the configuration
  for (int i = 1; i < argc; i++) {
    Option = argv[i];
    if (Option == "--change-configuration" || Option == "-C") {
      if (m_Data->Change(argv[++i]) == false) {
        cout<<"ERROR: Command-line parser: Unable to change this configuration value: "<<argv[i]<<endl;
      } else {
        cout<<"Command-line parser: Changing this configuration value: "<<argv[i]<<endl;
      }
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
        cout<<"Command-line parser: The geometry file could not be opened correctly: "<<argv[i]<<endl;
        return false;
      }
      cout<<"Command-line parser: Use geometry file "<<m_Data->GetGeometryFileName()<<endl;
    } else if (Option == "--filename" || Option == "-f") {
      if (m_Data->SetCurrentFileName(argv[++i]) == false) {
        cout<<"Command-line parser: The file could not be opened correctly: "<<argv[i]<<endl;
        return false;
      }
      cout<<"Command-line parser: Use file "<<m_Data->GetCurrentFileName()<<endl;
    } else if (Option == "--oi") {
      m_Data->SetSaveOI(true);
      cout<<"Command-line parser: Store OI"<<endl;
    } else if (Option == "--jobs" || Option == "-j") {
      m_Data->SetNJobs(atoi(argv[++i]));
      cout<<"Command-line parser: Use file "<<m_Data->GetCurrentFileName()<<endl;
    } else if (Option == "--special" || Option == "--development") {
      m_Data->SetSpecialMode(true);
      cout<<"Command-line parser: Activating development extras mode - hope, you know what you are doing..."<<endl;
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
  for (int i = 1; i < argc; i++) {
    Option = argv[i];
    if (Option == "--analyze" || Option == "-a") {
      cout<<"Command-line parser: Analyzing..."<<endl;
      AnalyzeEvents();
      return true;
    } else if (Option == "--generate-spectra" || Option == "-s") {
      cout<<"Command-line parser: Generate spectra (use the options from the configuration file)"<<endl;
      GenerateSpectra();
      return true;
    }
  }

  // Execute some low level commands
  if (m_UseGui == true) {
    m_Gui = new MGUIRevanMain(this, m_Data);
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


bool MInterfaceRevan::LoadConfiguration(MString FileName)
{
  // Load the configuration file

  if (m_Data == 0) {
    m_Data = new MSettingsRevan();
    m_BasicGuiData = dynamic_cast<MSettings*>(m_Data);
    if (m_UseGui == true) {
      m_Gui->SetConfiguration(m_Data);
    }
  }

  m_Data->Read(FileName);

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MInterfaceRevan::SaveConfiguration(MString FileName)
{
  // Save the configuration file

  massert(m_Data != 0);

  m_Data->Write(FileName);

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MInterfaceRevan::SetGeometry(MString FileName, bool UpdateGui)
{
  if (m_Geometry != 0) {
    delete m_Geometry;
    m_Geometry = 0;
  }

  if (FileName == g_StringNotDefined) return false;

  // Check if the geometry exists:
  if (MFile::FileExists(FileName) == false) {
    mgui<<"The geometry file \""<<FileName<<"\" does not exist!!"<<error;
    m_Data->SetGeometryFileName(g_StringNotDefined);
    return false;
  }

  MFile::ExpandFileName(FileName);

  m_Geometry = new MGeometryRevan();
  if (m_Geometry->ScanSetupFile(FileName, false) == false) {
    mgui<<"Loading of geometry \""<<FileName<<"\" failed!"<<endl;
    mgui<<"Please check the output for geometry errors and correct them!"<<error;
    delete m_Geometry;
    m_Geometry = 0;
    m_Data->SetGeometryFileName(g_StringNotDefined);
    return false;
  }

  m_Data->SetGeometryFileName(FileName);
  mout<<"Geometry "<<m_Geometry->GetName()<<" loaded!"<<endl;

  if (m_UseGui == true && UpdateGui == true) {
    m_Gui->UpdateConfiguration();
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MInterfaceRevan::IsInitialized()
{
  if (m_Geometry == 0) {
    mgui<<"Please load a geometry first!"<<error;
    return false;
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MInterfaceRevan::SetGuiData(MRawEventAnalyzer& REA)
{
  // Transfer Gui data to the analyzer

  // This function also appears in MGUIEview!!

  REA.SetSettings(m_Data);
  REA.SetBatch(!m_UseGui);

  m_Data->Write();
}


////////////////////////////////////////////////////////////////////////////////


void MInterfaceRevan::AnalyzeEvents()
{
  // Start Revan main program and analyze all events...

  if (IsInitialized() == false) return;

  MTimer Timer;

  MString FilenameOut = m_Data->GetCurrentFileName();
  if (FilenameOut.EndsWith("evta")) {
    FilenameOut.Replace(FilenameOut.Length()-4, 4, "tra");
  } else if (FilenameOut.EndsWith("evta.gz")) {
    FilenameOut.Replace(FilenameOut.Length()-7, 7, "tra.gz");
  } else if (FilenameOut.EndsWith("sim")) {
    FilenameOut.Replace(FilenameOut.Length()-3, 3, "tra");
  } else if (FilenameOut.EndsWith("sim.gz")) {
    FilenameOut.Replace(FilenameOut.Length()-6, 6, "tra.gz");
  }
  
  //FilenameOut = MFile::GetWorkingDirectory() + "/" + MFile::GetBaseName(FilenameOut);

  MRawEventAnalyzer Analyzer;
  Analyzer.SetGeometry(m_Geometry);
  if (Analyzer.SetInputModeFile(m_Data->GetCurrentFileName()) == false) return;
  if (Analyzer.SetOutputModeFile(FilenameOut) == false) return;
  SetGuiData(Analyzer);

  if (Analyzer.PreAnalysis() == false) {
    mout<<"Event reconstruction: Initialization failed."<<endl;
    return;
  }
  unsigned int ReturnCode = 0;
  do {
    ReturnCode = Analyzer.AnalyzeEvent();
  } while (ReturnCode != MRawEventAnalyzer::c_AnalysisNoEventsLeftInFile && ReturnCode != MRawEventAnalyzer::c_AnalysisSavingEventFailed);
  if (Analyzer.PostAnalysis() == false) {
    mout<<"Event reconstruction: Postprocessing failed"<<endl;
    return;
  }

  mout<<"Event reconstruction finished in "<<Timer.ElapsedTime()<<" sec."<<endl;
}


////////////////////////////////////////////////////////////////////////////////


void MInterfaceRevan::GenerateSpectra()
{
  // Export the initial energy spectrum

  if (IsInitialized() == false) return;

  bool xLog = m_Data->GetSpectrumLog();
  const int xNBins = m_Data->GetSpectrumBins();
  double xMin = m_Data->GetSpectrumMin();
  double xMax = m_Data->GetSpectrumMax();

  double* xBins = CreateAxisBins(xMin, xMax, xNBins, xLog);

  bool BeforeReconstruction = m_Data->GetSpectrumBefore();
  bool AfterReconstruction = m_Data->GetSpectrumAfter();

  bool ByInstrument = m_Data->GetSpectrumSortByInstrument();
  bool ByDetectorType = m_Data->GetSpectrumSortByDetectorType();
  bool ByNamedDetector = m_Data->GetSpectrumSortByNamedDetector();
  bool ByDetector = m_Data->GetSpectrumSortByDetector();

  bool Combine = m_Data->GetSpectrumCombine();

  bool OutputScreen = m_Data->GetSpectrumOutputToScreen();
  bool OutputFile = m_Data->GetSpectrumOutputToFile();

  vector<TH1D*> AllSpectra;


  // Step 1: Let's create all the histograms:
  TH1D* SpectrumBeforeByInstrument = nullptr;
  map<int, TH1D*> SpectrumBeforeByDetectorType;
  map<MString, TH1D*> SpectrumBeforeByNamedDetector;
  map<MVector, TH1D*> SpectrumBeforeByDetector;
  if (BeforeReconstruction == true) {
    if (ByInstrument == true) {
      if (Combine == true) {
        SpectrumBeforeByInstrument = new TH1D("SpectrumCombinedBeforeByInstrument", "Event spectrum before reconstruction for the whole instrument", xNBins, xBins);
      } else {
        SpectrumBeforeByInstrument = new TH1D("HitSpectrumBeforeByInstrument", "Hit spectrum before reconstruction for the whole instrument", xNBins, xBins);
      }
      AllSpectra.push_back(SpectrumBeforeByInstrument);
    }
    if (ByDetectorType == true) {
      // Generated on the fly
    }
    if (ByDetector == true) {
      // Generated on the fly
    }
  }

  TH1D* SpectrumAfterByInstrument = nullptr;
  map<int, TH1D*> SpectrumAfterByDetectorType;
  map<MString, TH1D*> SpectrumAfterByNamedDetector;
  map<MVector, TH1D*> SpectrumAfterByDetector;
  if (AfterReconstruction == true) {
    if (ByInstrument == true) {
      if (Combine == true) {
        SpectrumAfterByInstrument = new TH1D("SpectrumCombinedAfterByInstrument", "Event spectrum after reconstruction for the whole instrument", xNBins, xBins);
      } else {
        SpectrumAfterByInstrument = new TH1D("HitSpectrumAfterByInstrument", "Hit spectrum after reconstruction for the whole instrument", xNBins, xBins);
      }
      AllSpectra.push_back(SpectrumAfterByInstrument);
    }
    if (ByDetectorType == true) {
      // Generated on the fly
    }
    if (ByNamedDetector == true) {
      // Generated on the fly
    }
    if (ByDetector == true) {
      // Generated on the fly
    }
  }


  // Step 2: Let's fill all the histograms:
  MRawEventAnalyzer Analyzer;
  Analyzer.SetGeometry(m_Geometry);
  if (Analyzer.SetInputModeFile(m_Data->GetCurrentFileName()) == false) return;

  SetGuiData(Analyzer);
  Analyzer.SetTotalEnergyMin(xMin);
  Analyzer.SetTotalEnergyMax(xMax);
  if (Analyzer.PreAnalysis() == false) return;

  unsigned int ReturnCode;
  do {
    ReturnCode = Analyzer.AnalyzeEvent();

    if (ReturnCode == MRawEventAnalyzer::c_AnalysisNoEventsLeftInFile ||
        ReturnCode == MRawEventAnalyzer::c_AnalysisSavingEventFailed) break;

    MRERawEvent* Before = Analyzer.GetInitialRawEvent();
    vector<MRERawEvent*> AfterRecon = Analyzer.GetOptimumEvents();
    
    // BEFORE :
    if (BeforeReconstruction == true && Before != nullptr) {

      if (ByInstrument == true) {
        if (Combine == true) {
          SpectrumBeforeByInstrument->Fill(Before->GetEnergy());
        } else {
          for (int r = 0; r < Before->GetNRESEs(); ++r) {
            SpectrumBeforeByInstrument->Fill(Before->GetRESEAt(r)->GetEnergy());
          }
        }
      }

      if (ByDetectorType == true) {
        if (Combine == true) {
          map<int, double> EnergyBeforeByDetectorType;
          for (int r = 0; r < Before->GetNRESEs(); ++r) {
            EnergyBeforeByDetectorType[Before->GetRESEAt(r)->GetDetector()] += Before->GetRESEAt(r)->GetEnergy();
          }
          for (auto E: EnergyBeforeByDetectorType) {
            if (SpectrumBeforeByDetectorType[E.first] == nullptr) {
              TH1D* H = new TH1D(MString("SpectrumBeforeCombinedByDetectorType_") + E.first,
                                 MString("Spectrum before reconstruction for detector type ") + E.first + MString(" (individual hits of the events have been combined)"), xNBins, xBins);
              SpectrumBeforeByDetectorType[E.first] = H;
              AllSpectra.push_back(H);
            }
            SpectrumBeforeByDetectorType[E.first]->Fill(E.second);
          }
        } else {
          for (int r = 0; r < Before->GetNRESEs(); ++r) {
            int DetectorType = Before->GetRESEAt(r)->GetDetector();
            if (SpectrumBeforeByDetectorType[DetectorType] == nullptr) {
              TH1D* H = new TH1D(MString("HitSpectrumBeforeByDetectorType_") + DetectorType,
                                 MString("Spectrum for individual hits before reconstruction for detector type ") + DetectorType, xNBins, xBins);
              SpectrumBeforeByDetectorType[DetectorType] = H;
              AllSpectra.push_back(H);
            }
            SpectrumBeforeByDetectorType[DetectorType]->Fill(Before->GetRESEAt(r)->GetEnergy());
          }
        }
      }

      if (ByNamedDetector == true) {
        if (Combine == true) {
          map<MString, double> EnergyBeforeByNamedDetector;
          for (int r = 0; r < Before->GetNRESEs(); ++r) {
            MString Name = Before->GetRESEAt(r)->GetVolumeSequence()->GetDetector()->GetName();
            EnergyBeforeByNamedDetector[Name] += Before->GetRESEAt(r)->GetEnergy();
          }
          for (auto E: EnergyBeforeByNamedDetector) {
            if (SpectrumBeforeByNamedDetector[E.first] == nullptr) {
              TH1D* H = new TH1D(MString("SpectrumBeforeCombinedByNamedDetector_") + E.first,
                                 MString("Spectrum before reconstruction for detector ") + E.first + MString(" (individual hits of the events have been combined)"),  xNBins, xBins);
              SpectrumBeforeByNamedDetector[E.first] = H;
              AllSpectra.push_back(H);
            }
            SpectrumBeforeByNamedDetector[E.first]->Fill(E.second);
          }
        } else {
          for (int r = 0; r < Before->GetNRESEs(); ++r) {
            MString Name = Before->GetRESEAt(r)->GetVolumeSequence()->GetDeepestVolume()->GetName();
            if (SpectrumBeforeByNamedDetector[Name] == nullptr) {
              TH1D* H = new TH1D(MString("HitSpectrumBeforeByNamedDetector_") + Name,
                                 MString("Spectrum for individual hits before reconstruction for named detector ") + Name,  xNBins, xBins);
              SpectrumBeforeByNamedDetector[Name] = H;
              AllSpectra.push_back(H);
            }
            SpectrumBeforeByNamedDetector[Name]->Fill(Before->GetRESEAt(r)->GetEnergy());
          }
        }
      } // By named detector


      if (ByDetector == true) {
        if (Combine == true) {
          map<MVector, double> EnergyBeforeByDetector;
          for (int r = 0; r < Before->GetNRESEs(); ++r) {
            MVector Position = Before->GetRESEAt(r)->GetVolumeSequence()->GetDeepestVolume()->GetShape()->GetUniquePosition();
            Position = Before->GetRESEAt(r)->GetVolumeSequence()->GetPositionInFirstVolume(Position, Before->GetRESEAt(r)->GetVolumeSequence()->GetDeepestVolume());
            EnergyBeforeByDetector[Position] += Before->GetRESEAt(r)->GetEnergy();
          }
          MString Name = Before->GetRESEAt(0)->GetVolumeSequence()->GetDeepestVolume()->GetName(); // We are guaranteed to have at least 1 hit...
          for (auto E: EnergyBeforeByDetector) {
            MVector Pos = E.first;
            if (SpectrumBeforeByDetector[E.first] == nullptr) {
              TH1D* H = new TH1D(MString("SpectrumBeforeCombinedByDetector_") + Name + "_" + Pos.ToString(),
                                 MString("Spectrum before reconstruction for detector ") + Name + " at position " + Pos.ToString() + MString(" (individual hits of the events have been combined)"),  xNBins, xBins);
              SpectrumBeforeByDetector[E.first] = H;
              AllSpectra.push_back(H);
            }
            SpectrumBeforeByDetector[E.first]->Fill(E.second);
          }
        } else {
          for (int r = 0; r < Before->GetNRESEs(); ++r) {
            MVector Position = Before->GetRESEAt(r)->GetVolumeSequence()->GetDeepestVolume()->GetShape()->GetUniquePosition();
            Position = Before->GetRESEAt(r)->GetVolumeSequence()->GetPositionInFirstVolume(Position, Before->GetRESEAt(r)->GetVolumeSequence()->GetDeepestVolume());
            MString Name = Before->GetRESEAt(r)->GetVolumeSequence()->GetDeepestVolume()->GetName();
            if (SpectrumBeforeByDetector[Position] == nullptr) {
              TH1D* H = new TH1D(MString("HitSpectrumBeforeByDetector_") + Name + "_" + Position.ToString(),
                                 MString("Spectrum for individual hits before reconstruction for detector ") + Name + " at position " + Position.ToString(),  xNBins, xBins);
              SpectrumBeforeByDetector[Position] = H;
              AllSpectra.push_back(H);
            }
            SpectrumBeforeByDetector[Position]->Fill(Before->GetRESEAt(r)->GetEnergy());
          }
        }
      } // By detector
    } // BEFORE



    // AFTER
    if (AfterReconstruction == true) {
      
      for (MRERawEvent* After: AfterRecon) {
        if (After == nullptr) continue;
        
        if (ByInstrument == true) {
          if (Combine == true) {
            SpectrumAfterByInstrument->Fill(After->GetEnergy());
          } else {
            for (int r = 0; r < After->GetNRESEs(); ++r) {
              SpectrumAfterByInstrument->Fill(After->GetRESEAt(r)->GetEnergy());
            }
          }
        }
        
        if (ByDetectorType == true) {
          if (Combine == true) {
            map<int, double> EnergyAfterByDetectorType;
            for (int r = 0; r < After->GetNRESEs(); ++r) {
              EnergyAfterByDetectorType[After->GetRESEAt(r)->GetDetector()] += After->GetRESEAt(r)->GetEnergy();
            }
            for (auto E: EnergyAfterByDetectorType) {
              if (SpectrumAfterByDetectorType[E.first] == nullptr) {
                TH1D* H = new TH1D(MString("SpectrumAfterCombinedByDetectorType_") + E.first, 
                                   MString("Spectrum after reconstruction for detector type ") + E.first + MString(" (individual hits of the events have been combined)"), xNBins, xBins);
                SpectrumAfterByDetectorType[E.first] = H;
                AllSpectra.push_back(H);
              }
              SpectrumAfterByDetectorType[E.first]->Fill(E.second);
              if (E.second > 2000) {
                cout<<E.second<<endl;
                cout<<After->ToString()<<endl;
              }
            }        
          } else {
            for (int r = 0; r < After->GetNRESEs(); ++r) {
              int DetectorType = After->GetRESEAt(r)->GetDetector();
              if (SpectrumAfterByDetectorType[DetectorType] == nullptr) {
                TH1D* H = new TH1D(MString("HitSpectrumAfterByDetectorType_") + DetectorType, 
                                   MString("Spectrum for individual hits after reconstruction for detector type ") + DetectorType, xNBins, xBins);
                SpectrumAfterByDetectorType[DetectorType] = H;
                AllSpectra.push_back(H);
              }
              SpectrumAfterByDetectorType[DetectorType]->Fill(After->GetRESEAt(r)->GetEnergy());
            }
          }      
        }
        
        if (ByNamedDetector == true) {
          if (Combine == true) {
            map<MString, double> EnergyAfterByNamedDetector;
            for (int r = 0; r < After->GetNRESEs(); ++r) {
              MString Name = After->GetRESEAt(r)->GetVolumeSequence()->GetDetector()->GetName();
              EnergyAfterByNamedDetector[Name] += After->GetRESEAt(r)->GetEnergy();
            }
            for (auto E: EnergyAfterByNamedDetector) {
              if (SpectrumAfterByNamedDetector[E.first] == nullptr) {
                TH1D* H = new TH1D(MString("SpectrumAfterCombinedByNamedDetector_") + E.first, 
                                   MString("Spectrum after reconstruction for detector ") + E.first + MString(" (individual hits of the events have been combined)"),  xNBins, xBins);
                SpectrumAfterByNamedDetector[E.first] = H;
                AllSpectra.push_back(H);
              }
              SpectrumAfterByNamedDetector[E.first]->Fill(E.second);
            }
          } else {
            for (int r = 0; r < After->GetNRESEs(); ++r) {
              MString Name = After->GetRESEAt(r)->GetVolumeSequence()->GetDeepestVolume()->GetName();
              if (SpectrumAfterByNamedDetector[Name] == nullptr) {
                TH1D* H = new TH1D(MString("HitSpectrumAfterByNamedDetector_") + Name,
                                   MString("Spectrum for individual hits after reconstruction for named detector ") + Name,  xNBins, xBins);
                SpectrumAfterByNamedDetector[Name] = H;
                AllSpectra.push_back(H);
              }
              SpectrumAfterByNamedDetector[Name]->Fill(After->GetRESEAt(r)->GetEnergy());
            }
          }
        } // By named detector
        
        if (ByDetector == true) {
          if (Combine == true) {
            map<MVector, double> EnergyAfterByDetector;
            for (int r = 0; r < After->GetNRESEs(); ++r) {
              MVector Position = After->GetRESEAt(r)->GetVolumeSequence()->GetDeepestVolume()->GetShape()->GetUniquePosition();
              Position = After->GetRESEAt(r)->GetVolumeSequence()->GetPositionInFirstVolume(Position, After->GetRESEAt(r)->GetVolumeSequence()->GetDeepestVolume());
              EnergyAfterByDetector[Position] += After->GetRESEAt(r)->GetEnergy();
            }
            MString Name = After->GetRESEAt(0)->GetVolumeSequence()->GetDeepestVolume()->GetName(); // We are guaranteed to have at least 1 hit...
            for (auto E: EnergyAfterByDetector) {              
              MVector Pos = E.first;
              if (SpectrumAfterByDetector[E.first] == nullptr) {
                TH1D* H = new TH1D(MString("SpectrumAfterCombinedByDetector_") + Name + "_" + Pos.ToString(), 
                                   MString("Spectrum after reconstruction for detector ") + Name + " at position " + Pos.ToString() + MString(" (individual hits of the events have been combined)"),  xNBins, xBins);
                SpectrumAfterByDetector[E.first] = H;
                AllSpectra.push_back(H);
              }
              SpectrumAfterByDetector[E.first]->Fill(E.second);
            }
          } else {
            for (int r = 0; r < After->GetNRESEs(); ++r) {
              MVector Position = After->GetRESEAt(r)->GetVolumeSequence()->GetDeepestVolume()->GetShape()->GetUniquePosition();
              Position = After->GetRESEAt(r)->GetVolumeSequence()->GetPositionInFirstVolume(Position, After->GetRESEAt(r)->GetVolumeSequence()->GetDeepestVolume());
              MString Name = After->GetRESEAt(r)->GetVolumeSequence()->GetDeepestVolume()->GetName();
              if (SpectrumAfterByDetector[Position] == nullptr) {
                TH1D* H = new TH1D(MString("HitSpectrumAfterByDetector_") + Name + "_" + Position.ToString(), 
                                   MString("Spectrum for individual hits after reconstruction for detector ") + Name + " at position " + Position.ToString(),  xNBins, xBins);
                SpectrumAfterByDetector[Position] = H;
                AllSpectra.push_back(H);
              }
              SpectrumAfterByDetector[Position]->Fill(After->GetRESEAt(r)->GetEnergy());
            }
          }
        } // By detector
      } // each after event
    } // AFTER

  } while (true);

  if (Analyzer.PostAnalysis() == false) return;


  // Step 3: Nicen it!

  for (TH1D* Hist: AllSpectra) {
    Hist->SetBit(kCanDelete);
    Hist->SetStats(false);
    Hist->SetFillColor(8);
    Hist->GetXaxis()->SetTitle("Energy [keV]");
    Hist->GetYaxis()->SetTitle("counts/keV");
  }


  // Step 4: Dump it!

  if (OutputFile == true) {
    for (TH1D* Hist: AllSpectra) {
      MString FileName = m_Data->GetCurrentFileName();
      if (FileName.EndsWith(".gz")) {
        FileName.RemoveLast(3);
      }
      if (FileName.EndsWith(".sim")) {
        FileName.RemoveLast(4);
      }
      if (FileName.EndsWith(".evta")) {
        FileName.RemoveLast(5);
      }
      FileName += "_";
      FileName += Hist->GetName();
      FileName += ".dat";

      ofstream out(FileName);
      out<<"# "<<Hist->GetTitle()<<endl;
      out<<"# File: "<<m_Data->GetCurrentFileName()<<endl;
      out<<"# SE marks the start of the data, EN the end - everything else might change"<<endl;
      out<<"# The first row is the low edge of the bin, the second the counts in this bin"<<endl;
      out<<"# The last data line before the EN is the high edge of the last bin, followed by a \"-\""<<endl;
      out<<endl;
      out<<"SE"<<endl;
      for (int b = 1; b <= Hist->GetNbinsX() + 1; ++b) {
        if (b <= Hist->GetNbinsX()) {
          out<<Hist->GetBinLowEdge(b)<<"  "<<Hist->GetBinContent(b)<<endl;
        } else {
          out<<Hist->GetBinLowEdge(b-1) + Hist->GetBinWidth(b-1)<<"   -"<<endl;
        }
      }
      out<<"EN"<<endl;
      out<<endl;
      out<<"# end"<<endl;
      out.close();
    }
  }

  if (OutputScreen == true) {
    int c = 0;
    const int MaxCanvases = 100;
    for (TH1D* Hist: AllSpectra) {
      if (++c > MaxCanvases) {
        mgui<<"We only display up to "<<MaxCanvases<<" individual spectra"<<info;
        break;
      }

      // Convert to cts/keV
      for (int b = 1; b <= Hist->GetXaxis()->GetNbins(); ++b) {
        if (Hist->GetBinContent(b) > 0) {
          Hist->SetBinContent(b, Hist->GetBinContent(b)/Hist->GetBinWidth(b));
        }
      }

      TCanvas* C = new TCanvas();
      C->cd();
      if (xLog == true) C->SetLogx();
      Hist->Draw();
      C->Update();
    }
  }


  delete [] xBins;
}


////////////////////////////////////////////////////////////////////////////////


void MInterfaceRevan::ExportSpectrum()
{
  // Export the initial energy spectrum

  if (IsInitialized() == false) return;

  bool xLog = m_Data->GetExportSpectrumLog();
  const int xNBins = m_Data->GetExportSpectrumBins();
  double xMin = m_Data->GetExportSpectrumMin();
  double xMax = m_Data->GetExportSpectrumMax();

  double* xBins = CreateAxisBins(xMin, xMax, xNBins, xLog);

  TH1D* Hist = new TH1D("InitialEnergySpectrum", "Initial energy spectrum",  xNBins, xBins);
  Hist->SetBit(kCanDelete);

  MRawEventAnalyzer Analyzer;
  Analyzer.SetGeometry(m_Geometry);
  if (Analyzer.SetInputModeFile(m_Data->GetCurrentFileName()) == false) return;

  SetGuiData(Analyzer);
  if (Analyzer.PreAnalysis() == false) return;

  MRERawEvent* RE = 0;
  while ((RE = Analyzer.GetNextInitialRawEventFromFile()) != 0) {
    Hist->Fill(RE->GetEnergy());
    delete RE;
  }

  ofstream out(m_Data->GetExportSpectrumFileName());
  out<<"# Exported spectrum before event reconstruction"<<endl;
  out<<"# File: "<<m_Data->GetCurrentFileName()<<endl;
  out<<"# SE marks the start of the data, EN the end - everything else might change"<<endl;
  out<<"# The first row is the low edge of the bin, the second the counts in this bin"<<endl;
  out<<"# The last data line before the EN is the high edge of the last bin, followed by a \"-\""<<endl;
  out<<endl;
  out<<"SE"<<endl;
  for (int b = 1; b <= Hist->GetNbinsX() + 1; ++b) {
    if (b <= Hist->GetNbinsX()) {
      out<<Hist->GetBinLowEdge(b)<<"  "<<Hist->GetBinContent(b)<<endl;
    } else {
      out<<Hist->GetBinLowEdge(b-1) + Hist->GetBinWidth(b-1)<<"   -"<<endl;
    }
  }
  out<<"EN"<<endl;
  out<<endl;
  out<<"# end"<<endl;
  out.close();

  delete [] xBins;
  delete Hist;
}


////////////////////////////////////////////////////////////////////////////////


void MInterfaceRevan::InitialEnergySpectrum()
{
  if (IsInitialized() == false) return;

  bool xLog = false;
  const int xNBins = 400;
  double xMin = m_Data->GetTotalEnergyMin();
  double xMax = m_Data->GetTotalEnergyMax();

  double* xBins = CreateAxisBins(xMin, xMax, xNBins, xLog);

  TH1D* Hist = new TH1D("InitialEnergySpectrum", "Initial energy spectrum",  xNBins, xBins);
  Hist->SetBit(kCanDelete);
  Hist->SetStats(false);
  Hist->SetFillColor(8);
  Hist->GetXaxis()->SetTitle("Energy [keV]");
  Hist->GetYaxis()->SetTitle("counts/keV");
  //Hist->GetYaxis()->SetTitle("counts/keV/s");

  MRawEventAnalyzer Analyzer;
  Analyzer.SetGeometry(m_Geometry);
  if (Analyzer.SetInputModeFile(m_Data->GetCurrentFileName()) == false) return;

  SetGuiData(Analyzer);
  if (Analyzer.PreAnalysis() == false) return;

  unsigned int NEventsInside = 0;
  unsigned int NEventsOutside = 0;

  MRERawEvent* RE = 0;
  while ((RE = Analyzer.GetNextInitialRawEventFromFile()) != 0) {
    Hist->Fill(RE->GetEnergy());
    if (RE->GetEnergy() >= xMin && RE->GetEnergy() <= xMax) {
      NEventsInside++;
    } else {
      NEventsOutside++;
    }
    delete RE;
  }


  // Normalize to counts/keV
  for (int b = 1; b <= Hist->GetNbinsX(); ++b) {
    Hist->SetBinContent(b, Hist->GetBinContent(b)/Hist->GetBinWidth(b));
  }

//   double Time = 1000.0;
//   mimp<<"Warning: Normalizing by time!"<<show;
//   for (int b = 1; b <= Hist->GetNbinsX(); ++b) {
//     Hist->SetBinContent(b, Hist->GetBinContent(b)/Time);
//   }

  TCanvas* HistCanvas = new TCanvas("InitialEnergySpectrumCanvas", "Initial energy spectrum canvas");
  HistCanvas->cd();
  Hist->Draw();
  HistCanvas->Update();

  delete [] xBins;

  cout<<endl;
  cout<<"Initial energy spectrum - some additional statistics:"<<endl;
  cout<<"Number of events inside shown histogram: "<<NEventsInside<<" (of "<<NEventsInside+NEventsOutside<<" total events)"<<endl;
  cout<<endl;
}


////////////////////////////////////////////////////////////////////////////////


void MInterfaceRevan::SpectralAnalyzer()
{
  //! The spectral analyzer

  MSpectralAnalyzer S;


  // Fill the initial histogram:
  S.SetGeometry(m_Geometry);
  S.SetSpectrum(1000, m_Data->GetTotalEnergyMin(), m_Data->GetTotalEnergyMax(), 2);

  if (IsInitialized() == false) return;

  MRawEventAnalyzer Analyzer;
  Analyzer.SetGeometry(m_Geometry);
  if (Analyzer.SetInputModeFile(m_Data->GetCurrentFileName()) == false) return;

  SetGuiData(Analyzer);
  if (Analyzer.PreAnalysis() == false) return;


  MRERawEvent* RE = 0;
  while ((RE = Analyzer.GetNextInitialRawEventFromFile()) != 0) {
    S.FillSpectrum(RE->GetEnergy());

    delete RE;
  }

  // Set the GUI options
  if (IsInitialized() == false) return;

  // peak search
  //  S.SetHistBinsSpectralyzer(m_Data->GetSpectralHistBinsSpectralyzer());
  S.SetSignaltoNoiseRatio(m_Data->GetSpectralSignaltoNoiseRatio());
  S.SetPoissonLimit(m_Data->GetSpectralPoissonLimit());


  //peak selection
  //  S.SetDeconvolutionLimit(m_Data->GetSpectralDeconvolutionLimit());

  // Isotope Selection
  S.SetIsotopeFileName(m_Data->GetSpectralIsotopeFileName());
  S.SetEnergyRange(m_Data->GetSpectralEnergyRange());

  // Do the analysis
  if (S.FindIsotopes() == true) {
//    cout<<"Found "<<S.GetNIsotopes()<<" isotopes."<<endl;
  }
}


////////////////////////////////////////////////////////////////////////////////


void MInterfaceRevan::Coincidence()
{
  // This function shows the time between individual hits

  bool xLog = true;
  const int xNBins = 100;
  double xMin = 1E-8;
  double xMax = 1E+2;

  double* xBins = CreateAxisBins(xMin, xMax, xNBins, xLog);

  TH1D* Hist = new TH1D("Coincidence", "Time between events",  xNBins, xBins);
  Hist->SetBit(kCanDelete);
  Hist->SetStats(false);
  Hist->SetFillColor(8);
  Hist->GetXaxis()->SetTitle("Time [s]");
  Hist->GetYaxis()->SetTitle("counts");
  //Hist->GetYaxis()->SetTitle("counts/keV/s");

  MRawEventAnalyzer Analyzer;
  Analyzer.SetGeometry(m_Geometry);
  if (Analyzer.SetInputModeFile(m_Data->GetCurrentFileName()) == false) return;

  SetGuiData(Analyzer);
  if (Analyzer.PreAnalysis() == false) return;

  MRERawEvent* This = 0;
  MRERawEvent* Last = 0;
  while ((This = Analyzer.GetNextInitialRawEventFromFile()) != 0) {
    if (Last == 0) {
      Last = This;
      continue;
    }
    Hist->Fill(fabs((This->GetEventTime() - Last->GetEventTime()).GetAsSeconds()));

    delete Last;
    Last = This;
  }

  TCanvas* HistCanvas = new TCanvas("CoincidenceCanvas", "Coincidence canvas");
  HistCanvas->cd();
  HistCanvas->SetLogx();
  Hist->Draw();
  HistCanvas->Update();

  delete [] xBins;
}



////////////////////////////////////////////////////////////////////////////////


void MInterfaceRevan::TriggerStatistics()
{
  if (IsInitialized() == false) return;


  MFileEventsEvta* Reader = new MFileEventsEvta(m_Geometry);
  if (Reader->Open(m_Data->GetCurrentFileName()) == false) {
    mout<<"Unable to open file "<<m_Data->GetCurrentFileName()<<". Aborting!"<<endl;
    return;
  }
  Reader->ShowProgress();

  MRERawEvent* RE = 0;
  while ((RE = Reader->GetNextEvent()) != 0) {
    delete RE;
  }
  mout<<Reader->GetERNoising()->ToString()<<endl;

  delete Reader;

  return;
}


// ////////////////////////////////////////////////////////////////////////////////


// void MInterfaceRevan::ShowCSRTestStatistics()
// {
//   if (IsInitialized() == false) return;

//   MRawEventAnalyzer Analyzer(MString(m_Data->GetCurrentFileName()), "", m_Geometry);
//   SetGuiData(Analyzer);
//   if (Analyzer.PreAnalysis() == false) return;


//   TH1D* CSRGood = new TH1D("CSR Good Test Statistics", "CSR Good Test Statistics",
//         50, m_Data->GetCSRThresholdMin(), m_Data->GetCSRThresholdMax());
//   CSRGood->SetBit(kCanDelete);
//   CSRGood->SetFillColor(8);

//   TH1D* CSRAll = new TH1D("CSR All Test Statistics", "CSR All Test Statistics",
//        50, m_Data->GetCSRThresholdMin(), m_Data->GetCSRThresholdMax());
//   CSRAll->SetBit(kCanDelete);
//   CSRAll->SetFillColor(8);

//   MERCSR& CSR = Analyzer.GetCSR();

//   while (Analyzer.AnalyzeNextEvent() != false) {
//     if (Analyzer.GetRawEvent() != 0) {
//       cout<<"-->"<<endl;
//       cout<<"-->"<<Analyzer.GetRawEvent()->GetScoreComptonQualityFactor1()<<endl;
//       cout<<"-->"<<endl;
//       if (Analyzer.GetRawEvent()->GetScoreComptonQualityFactor1() > MRERawEvent::c_NoScore) {
//         CSRGood->Fill(Analyzer.GetRawEvent()->GetScoreComptonQualityFactor1());
//       }

//       map<double, vector<MRESE*>, greater_equal<double> >& TestStatistics =
//         CSR.GetTestStatistics();

//       map<double, vector<MRESE*>, greater_equal<double> >::iterator TestStatisticsIterator;

//       for (TestStatisticsIterator = TestStatistics.begin();
//            TestStatisticsIterator != TestStatistics.end();
//            TestStatisticsIterator++) {
//         CSRAll->Fill((*TestStatisticsIterator).first);
//       }

//     }
//   }

//   TCanvas* CSRGoodCanvas =
//     new TCanvas("CSR Test Good Statistics Canvas", "CSR Test Good Statistics Canvas", 640, 480);
//   CSRGoodCanvas->cd();
//   CSRGood->Draw();
//   CSRGoodCanvas->Update();

//   TCanvas* CSRAllCanvas =
//     new TCanvas("CSR Test All Statistics Canvas", "CSR Test All Statistics Canvas", 640, 480);
//   CSRAllCanvas->cd();
//   CSRAll->Draw();
//   CSRAllCanvas->Update();

//   if (Analyzer.PostAnalysis() == false) return;

//   return;
// }


////////////////////////////////////////////////////////////////////////////////


void MInterfaceRevan::EnergyDistribution()
{
  if (IsInitialized() == false) return;

  const int NBins = 100;
  double EMax = m_Data->GetTotalEnergyMax();
  double EMin = m_Data->GetTotalEnergyMin();

  TH1D* Before = new TH1D("InitialEnergySpectrum",
        "Energy spectrum BEFORE reconstruction",
        NBins, EMin, EMax);
  Before->SetBit(kCanDelete);
  Before->SetFillColor(8);
  Before->GetXaxis()->SetTitle("Energy [keV]");
  Before->GetYaxis()->SetTitle("counts");

  TH1D* After = new TH1D("InitialEnergySpectrum",
       "Energy spectrum AFTER reconstruction",
       NBins, EMin, EMax);
  After->SetBit(kCanDelete);
  After->SetFillColor(8);
  After->GetXaxis()->SetTitle("Energy [keV]");
  After->GetYaxis()->SetTitle("counts");

  MRawEventAnalyzer Analyzer;
  Analyzer.SetGeometry(m_Geometry);
  if (Analyzer.SetInputModeFile(m_Data->GetCurrentFileName()) == false) return;

  SetGuiData(Analyzer);
  if (Analyzer.PreAnalysis() == false) return;

  unsigned int ReturnCode;
  do {
    ReturnCode = Analyzer.AnalyzeEvent();

    if (ReturnCode == MRawEventAnalyzer::c_AnalysisNoEventsLeftInFile ||
        ReturnCode == MRawEventAnalyzer::c_AnalysisSavingEventFailed) break;

    if (Analyzer.GetInitialRawEvent() != 0) {
      Before->Fill(Analyzer.GetInitialRawEvent()->GetEnergy());
    }

    vector<MRERawEvent*> REs = Analyzer.GetOptimumEvents();
    for (auto RE: REs) {
      if (RE == nullptr) continue;
      After->Fill(RE->GetEnergy());
    }

  } while (true);

  if (Analyzer.PostAnalysis() == false) return;

  TCanvas* BeforeCanvas = new TCanvas("Before", "Before", 640, 480);
  BeforeCanvas->cd();
  Before->Draw();
  BeforeCanvas->Update();

  TCanvas* AfterCanvas = new TCanvas("After", "After", 640, 480);
  AfterCanvas->cd();
  After->Draw();
  AfterCanvas->Update();
}


////////////////////////////////////////////////////////////////////////////////


void MInterfaceRevan::InitialEventStatistics()
{
  //! Dump some initial event statistics, i.e. average hits, average energy deposit, average channels

  if (IsInitialized() == false) return;

  double AverageEnergy = 0.0;
  unsigned int NEvents = 0;

  unsigned int MaxFoundHits = 0;
  unsigned int MaxHits = 100;
  vector<int> Hits(MaxHits, 0);

  unsigned int NSeparatedHitsEvents = 0;
  unsigned int MaxFoundSeparatedHits = 0;
  unsigned int MaxSeparatedHits = 100;
  vector<int> SeparatedHits(MaxSeparatedHits, 0);

  MRawEventAnalyzer Analyzer;
  Analyzer.SetGeometry(m_Geometry);
  if (Analyzer.SetInputModeFile(m_Data->GetCurrentFileName()) == false) return;

  SetGuiData(Analyzer);
  if (Analyzer.PreAnalysis() == false) return;

  // Set up a dummy hit clusterizer
  MERHitClusterizer HitClusterizer;
  if (m_Data->GetHitClusteringAlgorithm() == MRawEventAnalyzer::c_HitClusteringAlgoDistance) {
    HitClusterizer.SetParameters(m_Data->GetStandardClusterizerMinDistanceD1(), 
                              m_Data->GetStandardClusterizerMinDistanceD2(),
                              m_Data->GetStandardClusterizerMinDistanceD3(),
                              m_Data->GetStandardClusterizerMinDistanceD4(),
                              m_Data->GetStandardClusterizerMinDistanceD5(),
                              m_Data->GetStandardClusterizerMinDistanceD6(),
                              m_Data->GetStandardClusterizerMinDistanceD7(),
                              m_Data->GetStandardClusterizerMinDistanceD8(),
                              m_Data->GetStandardClusterizerCenterIsReference());
  } else if (m_Data->GetHitClusteringAlgorithm() == MRawEventAnalyzer::c_HitClusteringAlgoAdjacent) {
    HitClusterizer.SetParameters(m_Data->GetAdjacentLevel(), m_Data->GetAdjacentSigma());
  }
  MRawEventIncarnations* RawEvents = new MRawEventIncarnations(0);

  MRERawEvent* RE = 0;
  while ((RE = Analyzer.GetNextInitialRawEventFromFile()) != 0) {
    unsigned int NHits = RE->GetNRESEs();
    if (NHits < MaxHits) {
      if (MaxFoundHits < NHits) MaxFoundHits = NHits;
      Hits[NHits]++;
    }
    ++NEvents;
    AverageEnergy += RE->GetEnergy();

    RawEvents->SetInitialRawEvent(RE);
    HitClusterizer.Analyze(RawEvents);
    unsigned int NSeparatedHits = RawEvents->GetRawEventAt(0)->GetNRESEs();

    if (NSeparatedHits > 1 && NSeparatedHits < MaxSeparatedHits && NHits < 10) {
      if (MaxFoundSeparatedHits < NSeparatedHits) MaxFoundSeparatedHits = NSeparatedHits;
      SeparatedHits[NSeparatedHits]++;
      NSeparatedHitsEvents++;
    }

    // delete RE; --> delete by RawEvents
  }

  mout<<"Initial event statistics: "<<endl;
  double TotalHits = 0;
  double AverageHits = 0;
  for (unsigned int h = 1; h <= MaxFoundHits; ++h) {
    cout<<h<<": "<<Hits[h]<<endl;
    TotalHits += Hits[h];
    AverageHits += h*Hits[h];
  }
  if (TotalHits == 0 && NEvents == 0) {
    mout<<"   No hits or no events found..."<<endl;
    return;
  }
  AverageHits /= TotalHits;
  AverageEnergy /= NEvents;

  double TotalSeparatedHits = 0;
  double AverageSeparatedHits = 0;
  for (unsigned int h = 1; h <= MaxFoundSeparatedHits; ++h) {
    cout<<h<<": "<<SeparatedHits[h]<<endl;
    TotalSeparatedHits += SeparatedHits[h];
    AverageSeparatedHits += h*SeparatedHits[h];
  }
  if (TotalSeparatedHits == 0 && NSeparatedHitsEvents == 0) {
    mout<<"   No hits or no events found which are well separated..."<<endl;
    return;
  }
  AverageSeparatedHits /= TotalSeparatedHits;


  mout<<"  Number of events:       "<<NEvents<<endl;
  mout<<"  Average number of hits: "<<AverageHits<<endl;
  mout<<"  Average energy deposit: "<<AverageEnergy<<endl;

  mout<<"  Number of events with separated hits:       "<<NSeparatedHitsEvents<<endl;
  mout<<"  Average number of hits ion spearated hit events: "<<AverageSeparatedHits<<endl;


  return;
}


////////////////////////////////////////////////////////////////////////////////


void MInterfaceRevan::HitStatistics()
{
  if (IsInitialized() == false) return;

  vector<MString> DetectorNames;
  vector<TH1D*> Histograms;

  MFileEventsEvta* Reader = new MFileEventsEvta(m_Geometry);
  if (Reader->Open(m_Data->GetCurrentFileName()) == false) {
    mout<<"Unable to open file "<<m_Data->GetCurrentFileName()<<". Aborting!"<<endl;
    return;
  }
  Reader->ShowProgress(m_UseGui);


//   MVector Pos;
   MRESE* RESE = 0;
//   MRERawEvent* RE = 0;
   MString Detector;

   unsigned int ind = 0;

//   while ((RE = Reader->GetNextEvent()) != 0) {

  MERHitClusterizer HitClusterizer;
  if (m_Data->GetHitClusteringAlgorithm() == MRawEventAnalyzer::c_HitClusteringAlgoDistance) {
    HitClusterizer.SetParameters(m_Data->GetStandardClusterizerMinDistanceD1(), 
                              m_Data->GetStandardClusterizerMinDistanceD2(),
                              m_Data->GetStandardClusterizerMinDistanceD3(),
                              m_Data->GetStandardClusterizerMinDistanceD4(),
                              m_Data->GetStandardClusterizerMinDistanceD5(),
                              m_Data->GetStandardClusterizerMinDistanceD6(),
                              m_Data->GetStandardClusterizerMinDistanceD7(),
                              m_Data->GetStandardClusterizerMinDistanceD8(),
                              m_Data->GetStandardClusterizerCenterIsReference());
  } else if (m_Data->GetHitClusteringAlgorithm() == MRawEventAnalyzer::c_HitClusteringAlgoAdjacent) {
    HitClusterizer.SetParameters(m_Data->GetAdjacentLevel(), m_Data->GetAdjacentSigma());
  }

  MRERawEvent* RE = 0;
  MRawEventIncarnations* RawEvents = new MRawEventIncarnations(0);

  while ((RE = Reader->GetNextEvent()) != 0) {
    RawEvents->SetInitialRawEvent(RE);
    HitClusterizer.Analyze(RawEvents);
  
    massert(RawEvents->GetNRawEvents() == 1);

    MRERawEvent* RW = RawEvents->GetRawEventAt(0);

    for (int i = 0; i < RW->GetNRESEs(); ++i) {
      RESE = RW->GetRESEAt(i);
      Detector = RESE->GetVolumeSequence()->GetDetector()->GetName();

      for (ind = 0; ind < DetectorNames.size(); ++ind) {
        if (DetectorNames[ind] == Detector) break;
      }
      if (ind == DetectorNames.size()) {
        DetectorNames.push_back(Detector);

        TH1D* Hist = new TH1D(Detector, Detector, 100, 0, 1000);
        Hist->SetBit(kCanDelete);
        Hist->SetDirectory(0);
        Hist->SetXTitle("Energy [keV]");
        Hist->SetYTitle("counts");
        Hist->SetFillColor(8);
        Histograms.push_back(Hist);
      }

      Histograms[ind]->Fill(RESE->GetEnergy());
    }
    //delete RE; done in RawEvents!
  }

  for (unsigned int i = 0; i < Histograms.size(); ++i) {
    TCanvas* Canvas = new TCanvas(DetectorNames[i], DetectorNames[i]);
    Canvas->cd();
    //Histograms[i]->SetStats(false);
    Histograms[i]->Draw();
    Canvas->Update();
  }
}

////////////////////////////////////////////////////////////////////////////////

/*
void MInterfaceRevan::HitStatistics()
{
  if (IsInitialized() == false) return;

  merr<<"This function contains hard-coded detector values!"<<endl;

  //int Bin = 0;
  //int NBins = 100;
  //double Energy;
  double MinEnergy = 400;
  double MaxEnergy = 100000;


  map<MString, int> DetectorHits;
  map<MString, int>::iterator Iter;
  map<MString, double> DetectorEfficiency;
  map<MString, double>::iterator EIter;

  MFileEventsEvta* Reader = new MFileEventsEvta(m_Geometry);
  if (Reader->Open(m_Data->GetCurrentFileName()) == false) {
    mout<<"Unable to open file "<<m_Data->GetCurrentFileName()<<". Aborting!"<<endl;
    return;
  }
  Reader->ShowProgress(m_UseGui);

  MVector Pos;
  MRESE* RESE = 0;
  MRERawEvent* RE = 0;
  MString Detector;

  while ((RE = Reader->GetNextEvent()) != 0) {
    if (RE->GetNRESEs() != 1) continue;
    for (int i = 0; i < RE->GetNRESEs(); ++i) {
      RESE = RE->GetRESEAt(i);
      Pos = RESE->GetPosition();
      if (RESE->GetEnergy() < MaxEnergy && RESE->GetEnergy() > MinEnergy) {
        Detector = m_Geometry->GetDetectorName(Pos);
        DetectorHits[Detector]++;
      }
    }
    delete RE;
  }

  cout<<"Hits per detector up to "<<setw(6)<<MaxEnergy<<"keV:"<<endl;
  for (Iter = DetectorHits.begin(); Iter != DetectorHits.end(); Iter++) {
    cout<<setw(13)<<(*Iter).first<<": "<<setw(6)<<(*Iter).second<<" hits"<<endl;
  }

  int SumD1 = 0;
  int SumD22 = 0;
  int SumD24 = 0;
  int SumD28 = 0;
  for (Iter = DetectorHits.begin(); Iter != DetectorHits.end(); Iter++) {
    MString Name = (*Iter).first;
    if (Name == "athene" || Name == "daphne" ||
        Name == "xantippe" || Name == "aphrodite") {
      SumD28 += (*Iter).second;
    } else if (Name == "medusa" || Name == "fortuna" || Name == "penelope" || Name == "venus" ||
         Name == "hydra" || Name == "aetna" || Name == "ariadne" || Name == "helena") {
      SumD24 += (*Iter).second;
    } else if (Name == "antigone" || Name == "diane" || Name == "persephone" || Name == "minerva" ||
         Name == "pallas" || Name == "thetis" || Name == "circe" || Name == "europa") {
      SumD22 += (*Iter).second;
    } else {
      SumD1 += (*Iter).second;
    }
  }

  for (Iter = DetectorHits.begin(); Iter != DetectorHits.end(); Iter++) {
    MString Name = (*Iter).first;
    if (Name == "athene" || Name == "daphne" ||
        Name == "xantippe" || Name == "aphrodite") {
      DetectorEfficiency[Name] = 4.0*(*Iter).second/SumD28;
    } else if (Name == "medusa" || Name == "fortuna" || Name == "penelope" || Name == "venus" ||
         Name == "hydra" || Name == "aetna" || Name == "ariadne" || Name == "helena") {
      DetectorEfficiency[Name] = 8.0*(*Iter).second/SumD24;
    } else if (Name == "antigone" || Name == "diane" || Name == "persephone" || Name == "minerva" ||
         Name == "pallas" || Name == "thetis" || Name == "circe" || Name == "europa") {
      DetectorEfficiency[Name] = 8.0*(*Iter).second/SumD22;
    } else {
      DetectorEfficiency[Name] = 11.0*(*Iter).second/SumD1;
    }
  }

  cout<<"  // Efficiency between "<<MinEnergy<<"keV and "<<MaxEnergy<<"keV:"<<endl;
  for (EIter = DetectorEfficiency.begin(); EIter != DetectorEfficiency.end(); EIter++) {
    //cout<<setw(13)<<(*EIter).first<<": "<<setw(6)<<(*EIter).second<<endl;
    cout<<"  m_Efficiency[\""<<(*EIter).first<<"\"] = "<<(*EIter).second<<";"<<endl;
  }
}
*/


////////////////////////////////////////////////////////////////////////////////


void MInterfaceRevan::DetectorTypeClusterDistribution(bool Before)
{
  if (IsInitialized() == false) return;

  int NEvents = 0;
  int NHits = 0;
  map<int, vector<unsigned int> > ClusterDistribution;
  map<int, unsigned int> Hits;

  MRawEventAnalyzer Analyzer;
  Analyzer.SetGeometry(m_Geometry);
  if (Analyzer.SetInputModeFile(m_Data->GetCurrentFileName()) == false) return;

  SetGuiData(Analyzer);
  if (Before == true) {
    Analyzer.SetTrackingAlgorithm(MRawEventAnalyzer::c_TrackingAlgoNone);
    Analyzer.SetPairAlgorithm(MRawEventAnalyzer::c_PairKalman2D);
    Analyzer.SetCSRAlgorithm(MRawEventAnalyzer::c_CSRAlgoNone);
    Analyzer.SetDecayAlgorithm(MRawEventAnalyzer::c_DecayAlgoNone);
  }

  if (Analyzer.PreAnalysis() == false) return;

  MRESE* RESE = nullptr;
  MRETrack* Track = nullptr;
  vector<MRERawEvent*> REs;

  unsigned int ReturnCode;
  do {
    ReturnCode = Analyzer.AnalyzeEvent();

    if (ReturnCode == MRawEventAnalyzer::c_AnalysisNoEventsLeftInFile ||
        ReturnCode == MRawEventAnalyzer::c_AnalysisSavingEventFailed) break;

    if (Before == true) {
      REs = Analyzer.GetInitialRawEvents();
    } else {
      REs = Analyzer.GetOptimumEvents();
    }

    for (auto RE: REs) {
      if (RE == nullptr) continue;
      
      Hits.clear();
      for (int i = 0; i < RE->GetNRESEs(); ++i) {
        RESE = RE->GetRESEAt(i);
        if (RESE->GetType() == MRESE::c_Track) {
          // Split top-level tracks:
          Track = dynamic_cast<MRETrack*>(RESE);
          for (int h = 0; h < Track->GetNRESEs(); ++h) {
            Hits[Track->GetRESEAt(h)->GetDetector()]++;
          }
        } else {
          Hits[RESE->GetDetector()]++;
        }
      }
      bool Added = false;
      for (map<int, unsigned int>::iterator Iter = Hits.begin();
           Iter != Hits.end(); ++Iter) {
        if (ClusterDistribution[(*Iter).first].size() < (*Iter).second + 1) {
          ClusterDistribution[(*Iter).first].resize((*Iter).second + 1);
        }
        cout<<"Hits:   "<<(*Iter).first<<":  "<<(*Iter).second<<endl;
        NHits += (*Iter).second;
        if ((*Iter).second > 0) Added = true;
        ClusterDistribution[(*Iter).first][(*Iter).second]++;
      }
      if (Added == true) {
        NEvents++;
      }
    }
  } while (true);

  if (Analyzer.PostAnalysis() == false) return;

  MString SuffixName;
  MString SuffixTitle;
  if (Before == true) {
    SuffixName = "BeforeReconstruction";
    SuffixTitle = " before Reconstruction";
  } else {
    SuffixName = "AfterReconstruction";
    SuffixTitle = " after Reconstruction";
  }

  // To screen...
  for (map<int, vector<unsigned int> >::iterator Iter = ClusterDistribution.begin();
       Iter != ClusterDistribution.end(); ++Iter) {
    TH1D* Clusters = new TH1D(MString("ClusterDistribution") +
                              MDDetector::GetDetectorTypeName((*Iter).first) + SuffixName,
                              MString("Cluster Distribution in Detector Type ") +
                              MDDetector::GetDetectorTypeName((*Iter).first) + SuffixTitle,
                              (*Iter).second.size(), 0.5, (*Iter).second.size()+0.5);
    Clusters->SetBit(kCanDelete);
    Clusters->SetFillColor(8);
    Clusters->GetXaxis()->SetTitle("Number of Clusters per Event and Detector Type");
    Clusters->GetYaxis()->SetTitle("counts");

    for (int b = 0; b < Clusters->GetNbinsX(); ++b) {
      Clusters->SetBinContent(b, (*Iter).second[b]);
    }

    TCanvas* ClustersCanvas =
      new TCanvas(MString("CanvasClusterDistribution") +
                  MDDetector::GetDetectorTypeName((*Iter).first) + SuffixName,
                  MString("Canvas for Cluster Distribution in Detector Type ") +
                  MDDetector::GetDetectorTypeName((*Iter).first) + SuffixTitle);
    ClustersCanvas->cd();
    Clusters->Draw();
    ClustersCanvas->Update();
  }

  mout<<"Average number of hits per triggered events: "<<((NEvents > 0) ? double(NHits)/NEvents : 0)<<endl;
}


////////////////////////////////////////////////////////////////////////////////


void MInterfaceRevan::DetectorTypeHitDistribution(bool Before)
{
  if (IsInitialized() == false) return;

  int NEvents = 0;
  int NHits = 0;
  int NTwoPlusEvents = 0;
  int NTwoPlusHits = 0;
  map<int, vector<unsigned int> > HitDistribution;
  map<int, unsigned int> Hits;

  MRawEventAnalyzer Analyzer;
  Analyzer.SetGeometry(m_Geometry);
  if (Analyzer.SetInputModeFile(m_Data->GetCurrentFileName()) == false) return;

  SetGuiData(Analyzer);
  if (Before == true) {
    Analyzer.SetTrackingAlgorithm(MRawEventAnalyzer::c_TrackingAlgoNone);
    Analyzer.SetPairAlgorithm(MRawEventAnalyzer::c_PairKalman2D);
    Analyzer.SetCSRAlgorithm(MRawEventAnalyzer::c_CSRAlgoNone);
    Analyzer.SetDecayAlgorithm(MRawEventAnalyzer::c_DecayAlgoNone);
  }

  if (Analyzer.PreAnalysis() == false) return;

  MRESE* RESE = 0;
  MRECluster* Cluster = 0;
  MRETrack* Track = 0;
  vector<MRERawEvent*> REs;
  
  unsigned int ReturnCode;
  do {
    ReturnCode = Analyzer.AnalyzeEvent();

    if (ReturnCode == MRawEventAnalyzer::c_AnalysisNoEventsLeftInFile ||
        ReturnCode == MRawEventAnalyzer::c_AnalysisSavingEventFailed) break;

    if (Before == true) {
      REs = Analyzer.GetBestTryEvents();
    } else {
      REs = Analyzer.GetOptimumEvents();
    }

    for (auto RE: REs) {
      if (RE == nullptr) continue;
      
      Hits.clear();
      for (int i = 0; i < RE->GetNRESEs(); ++i) {
        RESE = RE->GetRESEAt(i);
        if (RESE->GetType() == MRESE::c_Track) {
          // Split top-level tracks:
          Track = dynamic_cast<MRETrack*>(RESE);
          for (int h = 0; h < Track->GetNRESEs(); ++h) {
            if (RESE->GetType() == MRESE::c_Cluster) {
              // Split second-level clusters:
              Cluster = dynamic_cast<MRECluster*>(RESE);
              for (int h = 0; h < Cluster->GetNRESEs(); ++h) {
                Hits[Cluster->GetRESEAt(h)->GetDetector()]++;
              }
            } else {
              Hits[Track->GetRESEAt(h)->GetDetector()]++;
            }
          }
        } else if (RESE->GetType() == MRESE::c_Cluster) {
          // Split top-level clusters:
          Cluster = dynamic_cast<MRECluster*>(RESE);
          for (int h = 0; h < Cluster->GetNRESEs(); ++h) {
            Hits[Cluster->GetRESEAt(h)->GetDetector()]++;
          }
        } else {
          Hits[RESE->GetDetector()]++;
        }
      }
      bool Added = false;
      bool IsTwoPlus = false;
      for (map<int, unsigned int>::iterator Iter = Hits.begin(); Iter != Hits.end(); ++Iter) {
        if (HitDistribution[(*Iter).first].size() < (*Iter).second + 1) {
          HitDistribution[(*Iter).first].resize((*Iter).second + 1);
        }
        cout<<"Hits:   "<<(*Iter).first<<":  "<<(*Iter).second<<endl;
        NHits += (*Iter).second;
        if ((*Iter).second > 1) {
          NTwoPlusHits += (*Iter).second;
          IsTwoPlus = true;
        }
        if ((*Iter).second > 0) Added = true;
        HitDistribution[(*Iter).first][(*Iter).second]++;
      }
      if (Added == true) {
        NEvents++;
        if (IsTwoPlus == true) {
          NTwoPlusEvents++;
        }
      }
    }
  } while (true);

  if (Analyzer.PostAnalysis() == false) return;

  MString SuffixName;
  MString SuffixTitle;
  if (Before == true) {
    SuffixName = "BeforeReconstruction";
    SuffixTitle = " before Reconstruction";
  } else {
    SuffixName = "AfterReconstruction";
    SuffixTitle = " after Reconstruction";
  }

  // To screen...
  for (map<int, vector<unsigned int> >::iterator Iter = HitDistribution.begin();
       Iter != HitDistribution.end(); ++Iter) {
    TH1D* Hits = new TH1D(MString("HitDistribution") +
                              MDDetector::GetDetectorTypeName((*Iter).first) + SuffixName,
                              MString("Hit Distribution in Detector Type ") +
                              MDDetector::GetDetectorTypeName((*Iter).first) + SuffixTitle,
                              (*Iter).second.size(), 0.5, (*Iter).second.size()+0.5);
    Hits->SetBit(kCanDelete);
    Hits->SetFillColor(8);
    Hits->GetXaxis()->SetTitle("Number of Hits per Event and Detector Type");
    Hits->GetYaxis()->SetTitle("counts");

    for (int b = 0; b < Hits->GetNbinsX(); ++b) {
      Hits->SetBinContent(b, (*Iter).second[b]);
    }

    TCanvas* HitsCanvas =
      new TCanvas(MString("CanvasHitDistribution") +
                  MDDetector::GetDetectorTypeName((*Iter).first) + SuffixName,
                  MString("Canvas for Hit Distribution in Detector Type ") +
                  MDDetector::GetDetectorTypeName((*Iter).first) + SuffixTitle);
    HitsCanvas->cd();
    Hits->Draw();
    HitsCanvas->Update();
  }

  mout<<"Average number of hits per triggered events: "<<((NEvents > 0) ? double(NHits)/NEvents : 0)<<endl;
  mout<<"Average number of hits per triggered 2+ events: "<<((NTwoPlusEvents > 0) ? double(NTwoPlusHits)/NTwoPlusEvents : 0)<<endl;
}



////////////////////////////////////////////////////////////////////////////////


void MInterfaceRevan::DepthProfileByDetector()
{
  // View the hits in a 3D- and other plots:

  if (IsInitialized() == false) return;

  MFileEventsEvta* Reader = new MFileEventsEvta(m_Geometry);
  if (Reader->Open(m_Data->GetCurrentFileName()) == false) {
    mout<<"Unable to open file "<<m_Data->GetCurrentFileName()<<". Aborting!"<<endl;
    return;
  }
  Reader->ShowProgress(m_UseGui);

  double MinTotalEnergy = m_Data->GetTotalEnergyMin();
  double MaxTotalEnergy = m_Data->GetTotalEnergyMax();

  // Since we do not know anything about the detectors we have to store the events by detector first:

  // Map the detector center to a vector of depths
  map<MVector, vector<double>> Depths;

  // Step 1: Accumulate all hits:

  MRERawEvent* RE = 0;
  while ((RE = Reader->GetNextEvent()) != 0) {

    // Make sure the total energy is right:
    double Total = 0;
    for (int i = 0; i < RE->GetNRESEs(); ++i) {
      Total += RE->GetRESEAt(i)->GetEnergy();
    }
    if (Total >= MinTotalEnergy && Total <= MaxTotalEnergy) {
      // Save positions and energies
      for (int i = 0; i < RE->GetNRESEs(); ++i) {
        MDVolumeSequence* V = RE->GetRESEAt(i)->GetVolumeSequence();
        MVector Position = V->GetPositionInSensitiveVolume();
        MVector DetectorPosition = V->GetUniqueWorldPositionOfDetector();
        if (Position != g_VectorNotDefined && DetectorPosition != g_VectorNotDefined) {
          Depths[DetectorPosition].push_back(Position.Z());
        }
      }
    }

    delete RE;
  }

  // Step 2: Show everything:
  for (auto& Pair: Depths) {
    MString Title;
    double Min = numeric_limits<double>::max();
    double Max = -numeric_limits<double>::max();
    MDVolumeSequence VS = m_Geometry->GetVolumeSequence(Pair.first);
    if (VS.GetDetector() != nullptr) {
      Title = VS.GetDetector()->GetName();
      MDVolume* V = VS.GetSensitiveVolume();
      if (V != nullptr && V->GetShape()->GetType() == "BRIK") {
        Min = -dynamic_cast<MDShapeBRIK*>(V->GetShape())->GetSizeZ();
        Max = dynamic_cast<MDShapeBRIK*>(V->GetShape())->GetSizeZ();
      } else {
        for (double N: Pair.second) {
          if (N < Min) Min = N;
          if (N > Max) Max = N;
        }
      }
    } else {
      Title = Pair.first.ToString();
      for (double N: Pair.second) {
        if (N < Min) Min = N;
        if (N > Max) Max = N;
      }
    }

    TH1D* Hist = new TH1D(Title, Title, 100, Min, Max);
    Hist->SetXTitle("Interaction depth [cm]");
    Hist->SetYTitle("counts");
    for (double N: Pair.second) {
      Hist->Fill(N);
    }

    TCanvas* Canvas = new TCanvas();
    Canvas->cd();
    Hist->Draw();
    Canvas->Update();
  }


}


////////////////////////////////////////////////////////////////////////////////


void MInterfaceRevan::SpatialDistribution(bool UseEnergy)
{
  // View the hits in a 3D- and other plots:

  if (IsInitialized() == false) return;

  MFileEventsEvta* Reader = new MFileEventsEvta(m_Geometry);
  if (Reader->Open(m_Data->GetCurrentFileName()) == false) {
    mout<<"Unable to open file "<<m_Data->GetCurrentFileName()<<". Aborting!"<<endl;
    return;
  }
  Reader->ShowProgress(m_UseGui);

  unsigned int MaxNPositions = 10000000;
  vector<MVector> Positions;
  vector<double> Energies;

  double MinTotalEnergy = m_Data->GetTotalEnergyMin();
  double MaxTotalEnergy = m_Data->GetTotalEnergyMax();

  // Step 1: Accumulate many, many hits:

  MRESE* RESE = 0;
  MRERawEvent* RE = 0;
  while ((RE = Reader->GetNextEvent()) != 0) {

    // Make sure the total energy is right:
    double Total = 0;
    for (int i = 0; i < RE->GetNRESEs(); ++i) {
      Total += RE->GetRESEAt(i)->GetEnergy();
    }
    if (Total >= MinTotalEnergy && Total <= MaxTotalEnergy) {
      // Save positions and energies
      for (int i = 0; i < RE->GetNRESEs(); ++i) {
        Positions.push_back(RE->GetRESEAt(i)->GetPosition());
        Energies.push_back(RE->GetRESEAt(i)->GetEnergy());
      }
    }

    delete RE;

    if (Positions.size() > MaxNPositions) {
      break;
    }
  }

  // Step 2: Create the histograms
  double xMin = -100;
  double xMax = +100;
  double yMin = -100;
  double yMax = +100;
  double zMin = -100;
  double zMax = +100;
  DetermineAxis(xMin, xMax, yMin, yMax, zMin, zMax, Positions);

  int MaxNBins = 200;


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

  while ((RE = Reader->GetNextEvent()) != 0) {
    for (int i = 0; i < RE->GetNRESEs(); ++i) {
      RESE = RE->GetRESEAt(i);
      Pos = RESE->GetPosition();
      if (UseEnergy == true) Energy = RESE->GetEnergy();

      xyzHist->Fill(Pos[0], Pos[1], Pos[2], Energy);
      xyHist->Fill(Pos[0], Pos[1], Energy);
      xzHist->Fill(Pos[0], Pos[2], Energy);
      yzHist->Fill(Pos[1], Pos[2], Energy);
      xHist->Fill(Pos[0], Energy);
      yHist->Fill(Pos[1], Energy);
      zHist->Fill(Pos[2], Energy);
    }

    delete RE;
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


void MInterfaceRevan::EnergyPerDetector()
{
  if (IsInitialized() == false) return;

  MFileEventsEvta* Reader = new MFileEventsEvta(m_Geometry);
  if (Reader->Open(m_Data->GetCurrentFileName()) == false) {
    mout<<"Unable to open file "<<m_Data->GetCurrentFileName()<<". Aborting!"<<endl;
    return;
  }
  Reader->ShowProgress(m_UseGui);

  double Emax = 2200;
  MVector BottomCenter(0, 0, 6.3);
  MVector BottomDim(9, 8, 6); //4);

  TH1D* DHist = new TH1D("Ed", "Ed", 100, 0, Emax);
  DHist->SetBit(kCanDelete);
  TH1D* D1Hist = new TH1D("Ed1", "Ed1", 100, 0, Emax);
  D1Hist->SetBit(kCanDelete);
  TH1D* D2sHist = new TH1D("Ed2s", "Ed2s", 100, 0, Emax);
  D2sHist->SetBit(kCanDelete);
  TH1D* D2dHist = new TH1D("Ed2d", "Ed2d", 100, 0, Emax);
  D2dHist->SetBit(kCanDelete);
  TH1D* D2d1Hist = new TH1D("Ed2d1", "Ed2d1", 100, 0, Emax);
  D2d1Hist->SetBit(kCanDelete);
  TH1D* D2d2Hist = new TH1D("Ed2d2", "Ed2d2", 100, 0, Emax);
  D2d2Hist->SetBit(kCanDelete);
  TH1D* D2d3Hist = new TH1D("Ed2d3", "Ed2d3", 100, 0, Emax);
  D2d3Hist->SetBit(kCanDelete);
  TH1D* D2d4Hist = new TH1D("Ed2d4", "Ed2d4", 100, 0, Emax);
  D2d4Hist->SetBit(kCanDelete);


  int NEvents = 0;
  double AvgEnergyD1 = 0;
  int AllHitsD1 = 0;
  double AvgEnergyD2 = 0;
  int AllHitsD2 = 0;


  MVector Pos;
  MRESE* RESE = 0;
  MRERawEvent* RE = 0;
  while ((RE = Reader->GetNextEvent()) != 0) {
    // categorize the beam into layers:

    int NHitsD1 = 0;
    int NHitsD2 = 0;
    for (int i = 0; i < RE->GetNRESEs(); ++i) {
      RESE = RE->GetRESEAt(i);
      if (RESE->GetDetector() == 1) {
        NHitsD1++;
      } else if (RESE->GetDetector() == 2) {
        NHitsD2++;
      }
    }

    if (RE->GetEnergy() > Emax ||
  //if (RE->GetEnergy() > 1.1*Emax || RE->GetEnergy() < 0.9*Emax ||
        NHitsD1 == 0 || NHitsD2 == 0) {
      delete RE;
      continue;
    }

    NEvents++;
    for (int i = 0; i < RE->GetNRESEs(); ++i) {
      RESE = RE->GetRESEAt(i);
      if (RESE->GetDetector() == 1) {
        AllHitsD1++;
        AvgEnergyD1 += RESE->GetEnergy();
      } else if (RESE->GetDetector() == 2) {
        AllHitsD2++;
        AvgEnergyD2 += RESE->GetEnergy();
      }
    }


    bool HitD2side = false;
    bool HitD2bottom = false;
    for (int i = 0; i < RE->GetNRESEs(); ++i) {
      RESE = RE->GetRESEAt(i);
      if (RESE->GetDetector() == 1) {
        D1Hist->Fill(RESE->GetEnergy());
      } else if (RESE->GetDetector() == 2) {
        MVector Pos = RESE->GetPosition();
        Pos -= BottomCenter;
        if (fabs(Pos[0]) - BottomDim[0] > 0 || fabs(Pos[1]) - BottomDim[1] > 0 || fabs(Pos[2]) - BottomDim[2] > 0) {
          D2sHist->Fill(RESE->GetEnergy());
          HitD2side = true;
        } else {
          HitD2bottom = true;
          D2dHist->Fill(RESE->GetEnergy());
          if (Pos[0] > 0 && Pos[1] > 0) {
            D2d1Hist->Fill(RESE->GetEnergy());
          } else if (Pos[0] > 0 && Pos[1] < 0) {
            D2d2Hist->Fill(RESE->GetEnergy());
          } else if (Pos[0] < 0 && Pos[1] < 0) {
            D2d3Hist->Fill(RESE->GetEnergy());
          } else if (Pos[0] < 0 && Pos[1] > 0) {
            D2d4Hist->Fill(RESE->GetEnergy());
          }
        }
      }
    }
    if (HitD2bottom == true && HitD2side == false) {
      DHist->Fill(RE->GetEnergy());
    }

    delete RE;
  }

  TCanvas* DCanvas = new TCanvas();
  DCanvas->cd();
  DHist->Draw();
  DCanvas->Update();

  TCanvas* D1Canvas = new TCanvas();
  D1Canvas->cd();
  D1Hist->Draw();
  D1Canvas->Update();

  TCanvas* D2sCanvas = new TCanvas();
  D2sCanvas->cd();
  D2sHist->Draw();
  D2sCanvas->Update();

  TCanvas* D2dCanvas = new TCanvas();
  D2dCanvas->cd();
  D2dHist->Draw();
  D2dCanvas->Update();

  //  TCanvas* D2d1Canvas = new TCanvas();
  //  D2d1Canvas->cd();
  //  D2d1Hist->Draw();
  //  D2d1Canvas->Update();

  //  TCanvas* D2d2Canvas = new TCanvas();
  //  D2d2Canvas->cd();
  //  D2d2Hist->Draw();
  //  D2d2Canvas->Update();

  //  TCanvas* D2d3Canvas = new TCanvas();
  //  D2d3Canvas->cd();
  //  D2d3Hist->Draw();
  //  D2d3Canvas->Update();

  //  TCanvas* D2d4Canvas = new TCanvas();
  //  D2d4Canvas->cd();
  //  D2d4Hist->Draw();
  //  D2d4Canvas->Update();


  cout<<"Avg Energy D1: "<<AvgEnergyD1/AllHitsD1<<endl;
  cout<<"Avg Energy D2: "<<AvgEnergyD2/AllHitsD1<<endl;

  cout<<"Avg hits D1: "<<double(AllHitsD1)/NEvents<<endl;
  cout<<"Avg hits D2: "<<double(AllHitsD2)/NEvents<<endl;

}


////////////////////////////////////////////////////////////////////////////////


void MInterfaceRevan::Test()
{
  if (IsInitialized() == false) return;

  return;
}


////////////////////////////////////////////////////////////////////////////////


void MInterfaceRevan::EnergyPerCentralTrackElement()
{
  if (IsInitialized() == false) return;

  const int NBins = 125;
  double EMin = 100;
  double EMax = 1000;

  TH1D* StartEnergy = new TH1D("StartEnergySpectrum",
                                 "Energy of start track element",
                                 NBins/2, EMin, EMax);
  StartEnergy->SetBit(kCanDelete);
  StartEnergy->SetFillColor(8);
  StartEnergy->GetXaxis()->SetTitle("Energy [keV]");
  StartEnergy->GetYaxis()->SetTitle("counts");

  TH1D* StartAngle = new TH1D("StartAngleDstribution",
                                "Angle of start track element",
                                90, 0, 180);
  StartAngle->SetBit(kCanDelete);
  StartAngle->SetFillColor(8);
  StartAngle->GetXaxis()->SetTitle("Angle [#circ]");
  StartAngle->GetYaxis()->SetTitle("counts");

  TH1D* CentralEnergy = new TH1D("CentralEnergySpectrum",
                                 "Energy of central track element",
                                 NBins, EMin, EMax);
  CentralEnergy->SetBit(kCanDelete);
  CentralEnergy->SetFillColor(8);
  CentralEnergy->GetXaxis()->SetTitle("Energy [keV]");
  CentralEnergy->GetYaxis()->SetTitle("counts");

  TH1D* CentralAngle = new TH1D("CentralAngleDstribution",
                                "Angle of central track element",
                                90, 0, 180);
  CentralAngle->SetBit(kCanDelete);
  CentralAngle->SetFillColor(8);
  CentralAngle->GetXaxis()->SetTitle("Angle [#circ]");
  CentralAngle->GetYaxis()->SetTitle("counts");

  TH1D* StopEnergy = new TH1D("StopEnergySpectrum",
                                 "Energy of stop track element",
                                 NBins, EMin, EMax);
  StopEnergy->SetBit(kCanDelete);
  StopEnergy->SetFillColor(8);
  StopEnergy->GetXaxis()->SetTitle("Energy [keV]");
  StopEnergy->GetYaxis()->SetTitle("counts");

  TH1D* StopAngle = new TH1D("StopAngleDstribution",
                                "Angle of stop track element",
                                90, 0, 180);
  StopAngle->SetBit(kCanDelete);
  StopAngle->SetFillColor(8);
  StopAngle->GetXaxis()->SetTitle("Angle [#circ]");
  StopAngle->GetYaxis()->SetTitle("counts");


  MRawEventAnalyzer Analyzer;
  Analyzer.SetGeometry(m_Geometry);
  if (Analyzer.SetInputModeFile(m_Data->GetCurrentFileName()) == false) return;

  SetGuiData(Analyzer);
  if (Analyzer.PreAnalysis() == false) return;

  unsigned int ReturnCode;
  do {
    ReturnCode = Analyzer.AnalyzeEvent();

    if (ReturnCode == MRawEventAnalyzer::c_AnalysisNoEventsLeftInFile ||
        ReturnCode == MRawEventAnalyzer::c_AnalysisSavingEventFailed) break;
    
    vector<MRERawEvent*> REs = Analyzer.GetOptimumEvents();
    
    for (auto RE: REs) {
      if (RE == nullptr) continue;
      for (int h = 0; h < RE->GetNRESEs(); ++h) {
        if (RE->GetRESEAt(h)->GetType() == MRESE::c_Track) {
          MRETrack* Track = (MRETrack*) (RE->GetRESEAt(h));
          if ((Track->GetRESEAt(1)->GetPosition()-Track->GetRESEAt(0)->GetPosition()).Angle(MVector(0, 0, -1))*c_Deg < 20) {
            StartEnergy->Fill(Track->GetRESEAt(0)->GetEnergy());
            StartAngle->Fill((Track->GetRESEAt(1)->GetPosition()-Track->GetRESEAt(0)->GetPosition()).Angle(MVector(0, 0, -1))*c_Deg);
            for (int t = 1; t < Track->GetNRESEs()-1; ++t) {
              CentralEnergy->Fill(Track->GetRESEAt(t)->GetEnergy());
              CentralAngle->Fill((Track->GetRESEAt(t)->GetPosition()-Track->GetRESEAt(t-1)->GetPosition()).Angle(MVector(0, 0, -1))*c_Deg);
              break;
            }
            StopEnergy->Fill(Track->GetRESEAt(Track->GetNRESEs()-1)->GetEnergy());
            StopAngle->Fill((Track->GetRESEAt(Track->GetNRESEs()-1)->GetPosition()-Track->GetRESEAt(Track->GetNRESEs()-2)->GetPosition()).Angle(MVector(0, 0, -1))*c_Deg);
          }
        }
      }
    }
  } while (true);

  if (Analyzer.PostAnalysis() == false) return;

  TCanvas* StartEnergyCanvas = new TCanvas("StartEnergy", "StartEnergy", 640, 480);
  StartEnergyCanvas->cd();
  StartEnergy->Draw();
  StartEnergyCanvas->Update();

  TCanvas* StartAngleCanvas = new TCanvas("StartAngle", "StartAngle", 640, 480);
  StartAngleCanvas->cd();
  StartAngle->Draw();
  StartAngleCanvas->Update();

  TCanvas* CentralEnergyCanvas = new TCanvas("CentralEnergy", "CentralEnergy", 640, 480);
  CentralEnergyCanvas->cd();
  CentralEnergy->Draw();
  CentralEnergyCanvas->Update();

  TCanvas* CentralAngleCanvas = new TCanvas("CentralAngle", "CentralAngle", 640, 480);
  CentralAngleCanvas->cd();
  CentralAngle->Draw();
  CentralAngleCanvas->Update();

  TCanvas* StopEnergyCanvas = new TCanvas("StopEnergy", "StopEnergy", 640, 480);
  StopEnergyCanvas->cd();
  StopEnergy->Draw();
  StopEnergyCanvas->Update();

  TCanvas* StopAngleCanvas = new TCanvas("StopAngle", "StopAngle", 640, 480);
  StopAngleCanvas->cd();
  StopAngle->Draw();
  StopAngleCanvas->Update();

  return;
}


////////////////////////////////////////////////////////////////////////////////


void MInterfaceRevan::FindPolarization()
{
  if (IsInitialized() == false) return;

  TH2D* Hist = new TH2D("Pol", "Pol", 100, -20, 20, 100, -20, 20);
  Hist->SetBit(kCanDelete);
  Hist->SetXTitle("X");
  Hist->SetYTitle("Y");

  int Number = 3;
  double Stops[3] = { 0.00, 0.50, 1.00 };
  double Red[3] = { 1.00, 0.50, 0.00 };
  double Green[3] = { 1.00, 0.50, 0.00 };
  double Blue[3] = { 1.00, 0.50, 0.00 };
  TColor::CreateGradientColorTable(Number, Stops, Red, Green, Blue, 100);

  MFileEventsEvta* Reader = new MFileEventsEvta(m_Geometry);
  if (Reader->Open(m_Data->GetCurrentFileName()) == false) {
    mout<<"Unable to open file "<<m_Data->GetCurrentFileName()<<". Aborting!"<<endl;
    return;
  }
  Reader->ShowProgress(m_UseGui);

  int NEvents = 0;
  MRESE* R;
  MRERawEvent* RE = 0;

  int SideX = 0;
  int SideY = 0;

  int NRESEs = 5;
  double MinTotalEnergy = 400;
  double MaxTotalEnergy = 100000;
  double MinGammaEnergy = 400;
  double MaxGammaEnergy = 100000;


  while ((RE = Reader->GetNextEvent()) != 0) {
    // categorize the beam into layers:

    NEvents++;

    if (RE->GetNRESEs() < NRESEs) {
      if (RE->GetEnergy() >= MinTotalEnergy && RE->GetEnergy() < MaxTotalEnergy) {
        for (int r = 0; r < RE->GetNRESEs(); ++r) {
          R = RE->GetRESEAt(r);
          if (R->GetEnergy() >= MinGammaEnergy && R->GetEnergy() < MaxGammaEnergy) {
            if (R->GetPosition().Z() > 0) {
              if (fabs(R->GetPosition().X()) > 9.5 || fabs(R->GetPosition().Y()) > 9.5) {
                Hist->Fill(R->GetPosition().X(), R->GetPosition().Y());
                if (fabs(R->GetPosition().X()) > 15 && fabs(R->GetPosition().Y()) < 12) {
                  SideX++;
                } else if (fabs(R->GetPosition().Y()) > 15 && fabs(R->GetPosition().X()) < 12) {
                  SideY++;
                }
              } // x,y position
            } // z position
          } // Gamma Energy
        } // RESE loop
      } // Energy
    } // Number of RESE
    delete RE;
  }

  TCanvas* Canvas = new TCanvas();
  Hist->Draw("lego2");
  Canvas->Update();

  cout<<"Events: "<<NEvents<<endl;
  cout<<"Side comparison: x="<<SideX<<"  y="<<SideY<<" m="<<abs(SideX-SideY)/(SideX+SideY)<<endl;

  return;
}

////////////////////////////////////////////////////////////////////////////////


void MInterfaceRevan::StartDistribution()
{
  if (IsInitialized() == false) return;

  // View the hits in a 3D-plot:

  //MRESE *RESE;
  MRERawEvent* RE = nullptr;
  MRawEventAnalyzer Analyzer;
  Analyzer.SetGeometry(m_Geometry);
  if (Analyzer.SetInputModeFile(m_Data->GetCurrentFileName()) == false) return;
  SetGuiData(Analyzer);

  int NBins = 50;
  double xMin = -20;
  double xMax = 20;
  double yMin = -20;
  double yMax = 20;

  double eMin = 0;
  double eMax = 700;

  TH1D* DOM = new TH1D("DOM", "DOM", NBins, 0, 180);
  DOM->SetBit(kCanDelete);

  TH1D* EFirst = new TH1D("EFirst", "EFirst", NBins, eMin, eMax);
  EFirst->SetBit(kCanDelete);
  TH1D* ESecond = new TH1D("ESecond", "ESecond", NBins, eMin, eMax);
  ESecond->SetBit(kCanDelete);
  TH1D* EThird = new TH1D("EThird", "EThird", NBins, eMin, eMax);
  EThird->SetBit(kCanDelete);

  TH2D* Double = new TH2D("Double", "Double", NBins, xMin, xMax, NBins, yMin, yMax);
  Double->SetBit(kCanDelete);
  TH2D* Triple = new TH2D("Triple+", "Triple+", NBins, xMin, xMax, NBins, yMin, yMax);
  Triple->SetBit(kCanDelete);


  // Start the progress display:
  unsigned int ReturnCode;
  do {
    ReturnCode = Analyzer.AnalyzeEvent();

    if (ReturnCode == MRawEventAnalyzer::c_AnalysisNoEventsLeftInFile ||
        ReturnCode == MRawEventAnalyzer::c_AnalysisSavingEventFailed) break;

    if ((RE = Analyzer.GetInitialRawEvent()) != 0) {

      cout<<"Number: "<<RE->GetNRESEs()<<endl;
      if (RE->GetNRESEs() == 2) {
        if (RE->GetRESEAt(0)->GetPosition().Z() == 24.720 &&
            RE->GetRESEAt(1)->GetPosition().Z() == 22.720 ) {
          Double->Fill(RE->GetRESEAt(0)->GetPosition().X(), RE->GetRESEAt(0)->GetPosition().Y(), 1);
          EFirst->Fill(RE->GetRESEAt(0)->GetEnergy(), 1);
          ESecond->Fill(RE->GetRESEAt(1)->GetEnergy(), 1);
        }
      } else if (RE->GetNRESEs() == 3) {
        if (RE->GetRESEAt(0)->GetPosition().Z() == 24.720 &&
            RE->GetRESEAt(1)->GetPosition().Z() == 23.720 &&
            RE->GetRESEAt(2)->GetPosition().Z() == 22.720 &&
            RE->GetRESEAt(0)->GetPosition().X() > -3 &&
            RE->GetRESEAt(0)->GetPosition().X() < 3 &&
            RE->GetRESEAt(0)->GetPosition().Y() > -3 &&
            RE->GetRESEAt(0)->GetPosition().Y() < 3 ) {
          Triple->Fill(RE->GetRESEAt(0)->GetPosition().X(), RE->GetRESEAt(0)->GetPosition().Y(), 1);
        
          vector<MRERawEvent*> REs = Analyzer.GetOptimumEvents();
          for (auto REA: REs) {
            if (REA == nullptr) continue;
            
            // Get the track...
            for (int t = 0; t < REA->GetNRESEs(); t++) {
              if (REA->GetRESEAt(t)->GetType() == MRESE::c_Track) {
                cout<<((MRETrack*) REA->GetRESEAt(t))->ToString()<<endl;
                DOM->Fill(((MRETrack*) REA->GetRESEAt(t))->GetDirection().Theta()*c_Deg, 1);
              }
            }
            EFirst->Fill(RE->GetRESEAt(0)->GetEnergy(), 1);
            cout<<"Adding first: "<<RE->GetRESEAt(0)->GetEnergy()<<endl;
            ESecond->Fill(RE->GetRESEAt(1)->GetEnergy(), 1);
            cout<<"Adding second: "<<RE->GetRESEAt(1)->GetEnergy()<<endl;
            EThird->Fill(RE->GetRESEAt(2)->GetEnergy(), 1);
            cout<<"Adding third: "<<RE->GetRESEAt(2)->GetEnergy()<<endl;
          }
        }
      }
    }
  } while (true);

  TCanvas* DOMCanvas = new TCanvas();
  DOMCanvas->cd();
  DOM->Draw();
  cout<<" < 90: "<<DOM->Integral(0, NBins/2)<<"   > 90: "<<DOM->Integral(NBins/2+1, NBins)<<endl;

  //   TCanvas* DoubleCanvas = new TCanvas();
  //   Double->Draw();

  //   TCanvas* TrippleCanvas = new TCanvas();
  //   Triple->Draw();

  TCanvas* EFirstCanvas = new TCanvas();
  EFirstCanvas->cd();
  EFirst->Draw();

  TCanvas* ESecondCanvas = new TCanvas();
  ESecondCanvas->cd();
  ESecond->Draw();

  TCanvas* EThirdCanvas = new TCanvas();
  EThirdCanvas->cd();
  EThird->Draw();

  return;
}


////////////////////////////////////////////////////////////////////////////////


void MInterfaceRevan::FindBeamPath()
{
  if (IsInitialized() == false) return;

  //

  int Number = 3;
  double Stops[3] = { 0.00, 0.50, 1.00 };
  double Red[3] = { 1.00, 0.50, 0.00 };
  double Green[3] = { 1.00, 0.50, 0.00 };
  double Blue[3] = { 1.00, 0.50, 0.00 };
  TColor::CreateGradientColorTable(Number, Stops, Red, Green, Blue, 100);

  MFileEventsEvta* Reader = new MFileEventsEvta(m_Geometry);
  if (Reader->Open(m_Data->GetCurrentFileName()) == false) {
    mout<<"Unable to open file "<<m_Data->GetCurrentFileName()<<". Aborting!"<<endl;
    return;
  }
  Reader->ShowProgress(m_UseGui);

  // Create a list of histograms:
  vector<double> zValues;
  vector<TH2D*> Histos;
  vector<TCanvas*> Canvases;

  unsigned int Layers = 11;
  char Title[100];
  double zStart = 15.7;
  for (unsigned int l = 0; l < Layers; ++l) {
    zValues.push_back(zStart);
    zStart += 1;
    sprintf(Title, "Layer %d", l);
    Histos.push_back(new TH2D(Title, Title, 128, -3.008, 3.008, 128, -3.008, 3.008));
    Histos[l]->SetBit(kCanDelete);
    Histos[l]->SetStats(false);
    Canvases.push_back(new TCanvas(Title, Title, 0, 0, 480, 480));
  }

  bool EventUsed = false;
  int NEvents = 0;
  MRESE* R;
  MRERawEvent* RE = 0;
  while ((RE = Reader->GetNextEvent()) != 0) {
    EventUsed = false;
    // categorize the beam into layers:
    for (int r = 0; r < RE->GetNRESEs(); ++r) {
      R = RE->GetRESEAt(r);
      if (R->GetDetector() == 1) {
        for (unsigned int l = 0; l < Layers; ++l) {
          if (fabs(R->GetPosition().Z() - zValues[l]) < 0.3) {
            Histos[l]->Fill(R->GetPosition().X(), R->GetPosition().Y());
            EventUsed = true;
          }
        }
      }
    }
    if (EventUsed == true) NEvents++;
    delete RE;
  }


  // Show the histograms:
  for (unsigned int l = 0; l < Layers; ++l) {
    Canvases[l]->cd();
    Histos[l]->Draw("colz");
    Canvases [l]->Update();
  }

  // Now do a fit to determine the path
  TGraph* PathX = new TGraph(Layers);
  for (unsigned int l = 0; l < Layers; ++l) {
    PathX->SetPoint(l, zValues[l], Histos[l]->GetMean(1));
  }
  TCanvas* PathXCanvas = new TCanvas("PathX", "PathX", 0, 0, 640, 480);
  PathXCanvas->cd();
  TF1* FitX = new TF1("FitX", "pol1");
  PathX->Fit(FitX);
  PathX->SetMarkerStyle(3);
  PathX->Draw("AP");
  PathXCanvas->Update();

  TGraph* PathY = new TGraph(Layers);
  for (unsigned int l = 0; l < Layers; ++l) {
    PathY->SetPoint(l, zValues[l], Histos[l]->GetMean(2));
  }
  TCanvas* PathYCanvas = new TCanvas("PathY", "PathY", 0, 0, 640, 480);
  PathYCanvas->cd();
  TF1* FitY = new TF1("FitY", "pol1");
  PathY->Fit(FitY);
  PathY->SetMarkerStyle(3);
  PathY->Draw("AP");
  PathYCanvas->Update();

  // Plot Profile of first layer:
  TH1D* Projection = Histos[Layers-2]->ProjectionX();
  Projection->SetBit(kCanDelete);
  TCanvas* ProjectionCanvas = new TCanvas("Projection", "Projection", 0, 0, 640, 480);
  ProjectionCanvas->cd();
  Projection->Draw();
  ProjectionCanvas->Update();

  // Plot Profile of first layer:
  TProfile* Profile = Histos[Layers-2]->ProfileX();
  Profile->SetBit(kCanDelete);
  TCanvas* ProfileCanvas = new TCanvas("Profile", "Profile", 0, 0, 640, 480);
  ProfileCanvas->cd();
  Profile->Draw();
  ProfileCanvas->Update();

  cout<<"Number of used events: "<<NEvents<<endl;


  cout<<"The beam:"<<endl;

  cout<<"    "<<FitX->GetParameter(0)<<"\t   "<<FitX->GetParameter(1)*20<<"\t"<<endl;
  cout<<"x = "<<FitY->GetParameter(0)<<"\t + "<<FitY->GetParameter(1)*20<<"\t"<<endl;
  cout<<"    "<<0<<"\t   "<<20<<"\t"<<endl;

  cout<<"Deviation: "<<MVector(FitX->GetParameter(1)*20, FitY->GetParameter(1)*20, 20).Angle(MVector(0, 0, 1))*c_Deg<<endl;


  delete Reader;
}


////////////////////////////////////////////////////////////////////////////////


void MInterfaceRevan::NumberOfClusters()
{
  // Dump number of clusters per event

  if (IsInitialized() == false) return;

  vector<unsigned int> Clusters;

  MRawEventAnalyzer Analyzer;
  Analyzer.SetGeometry(m_Geometry);
  if (Analyzer.SetInputModeFile(m_Data->GetCurrentFileName()) == false) return;
  SetGuiData(Analyzer);
  Analyzer.SetTrackingAlgorithm(MRawEventAnalyzer::c_TrackingAlgoNone);
  Analyzer.SetPairAlgorithm(MRawEventAnalyzer::c_PairKalman2D);
  Analyzer.SetCSRAlgorithm(MRawEventAnalyzer::c_CSRAlgoNone);
  Analyzer.SetDecayAlgorithm(MRawEventAnalyzer::c_DecayAlgoNone);

  if (Analyzer.PreAnalysis() == false) return;
  unsigned int ReturnCode;
  do {
    ReturnCode = Analyzer.AnalyzeEvent();

    if (ReturnCode == MRawEventAnalyzer::c_AnalysisNoEventsLeftInFile ||
        ReturnCode == MRawEventAnalyzer::c_AnalysisSavingEventFailed) break;

    vector<MRERawEvent*> REs = Analyzer.GetBestTryEvents();
    for (auto RE: REs) {
      if (RE == nullptr) continue;
      if (Clusters.size() < (unsigned int) RE->GetNRESEs()) {
        Clusters.resize(RE->GetNRESEs() + 1);
      }
      Clusters[RE->GetNRESEs()]++;
    }
    // delete RE; // ?????
  } while (true);

  TH1D* ClustersHist =
    new TH1D("NumberOfClustersPerEvent",
             "Number of Cluster per Event",
             Clusters.size(), 0.5, Clusters.size()+0.5);
  ClustersHist->SetBit(kCanDelete);
  ClustersHist->SetFillColor(8);
  ClustersHist->GetXaxis()->SetTitle("Clusters");
  ClustersHist->GetYaxis()->SetTitle("counts");

  for (int b = 0; b < ClustersHist->GetNbinsX(); ++b) {
    ClustersHist->SetBinContent(b, Clusters[b]);
  }

  TCanvas* ClustersCanvas =
    new TCanvas("CanvasNumberOfClustersPerEvent",
                "Canvas for Number of Cluster per event");
  ClustersCanvas->cd();
  ClustersHist->Draw();
  ClustersCanvas->Update();


  mimp<<"Only energy selection considered!"<<show;

  double Sum = 0;
  mout<<"Clusters: "<<endl;
  for (unsigned int i = 0; i < Clusters.size(); ++i) {
    mout<<i<<": "<<Clusters[i]<<endl;
    Sum += Clusters[i];
  }
  mout<<"Sum: "<<Sum<<endl;
}


// MInterfaceRevan.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
