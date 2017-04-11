/*
 * MCParameterFile.cxx
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


// Cosima:
#include "MCParameterFile.hh"
#include "MCPhysicsList.hh"
#include "MCVHit.hh"
#include "MCIsotopeStore.hh"

// MEGAlib:
#include "MGlobal.h"
#include "MStreams.h"
#include "MSimEvent.h"

// Geant4:
#include "G4SystemOfUnits.hh"


/******************************************************************************/


const int MCParameterFile::c_DecayModeNormal                  = 0;
const int MCParameterFile::c_DecayModeIgnore                  = 1;
const int MCParameterFile::c_DecayModeBuildUp                 = 2;
const int MCParameterFile::c_DecayModeActivationBuildUp       = 3;
const int MCParameterFile::c_DecayModeActivationDelayedDecay  = 4;


/******************************************************************************
 * Default constructor - the parsing of the file takes place in Parse()
 */
MCParameterFile::MCParameterFile() : MParser(' ', true), 
                                     m_DefaultRangeCut(0.005*mm),
                                     m_PhysicsListEM(MCPhysicsList::c_EMLivermore), 
                                     m_PhysicsListHD(MCPhysicsList::c_HDNone), 
                                     m_DecayMode(c_DecayModeIgnore),
                                     m_StoreCalibrated(true), 
                                     m_StoreScientific(false),
                                     m_StoreScientificPrecision(5),
                                     m_StoreSimulationInfo(MSimEvent::c_StoreSimulationInfoAll),
                                     m_StoreSimulationInfoVersion(MSimEvent::GetOutputVersion()),
                                     m_StoreSimulationInfoIonization(false),
                                     m_StoreOnlyTriggeredEvents(true),
                                     m_StoreOneHitPerEvent(false),
                                     m_DiscretizeHits(true),
                                     m_CheckForOverlaps(false),
                                     m_OverlapCheckResolution(1000),
                                     m_OverlapCheckTolerance(1*um),
                                     m_CreateCrossSectionFiles(false),
                                     m_CrossSectionFileDirectory(""),
                                     m_ActiveRun(0),
                                     m_DetectorTimeConstant(1*ns)
{
  // Intentionally left blank
}


/******************************************************************************
 * Default destructor
 */
MCParameterFile::~MCParameterFile()
{
  // Intentionally left blank
}


/******************************************************************************
 * Parse the Run file and return true, if everything went right
 */
bool MCParameterFile::Parse()
{
  mdebug<<"Parsing parameter file..."<<endl;

  // The parsing is 5 step process
  // Step 1: Version and Runs
  // Step 2: Primary Run parameters
  // Step 3: Secondary Run parameters
  // Step 4: Primary Source parameters
  // Step 5: Secondary Source parameters

  if (m_IsOpen == false) {
    mout<<"Unable to open parameter file "<<m_FileName<<endl;
    return false;
  }

  unsigned int i;
  // Step 0: Check for Includes:
  bool IncludesFound = false;
  do {
    IncludesFound = false;
    for (i = 0; i < GetNLines(); ++i) {
      MTokenizer* T = GetTokenizerAt(i);
      
      if (T->GetNTokens() == 0) continue;
      if (T->IsTokenAt(0, "Include", true) == true) {
        if (T->GetNTokens() >= 2) {
          IncludesFound = true;
          MParser Parser(' ', true);
          if (Parser.Open(T->GetTokenAfterAsString(1)) == false) {
            mout<<"Unable to parse included file: "<<T->GetTokenAfterAsString(1)<<endl;
            return false;
          }
          RemoveLine(i);
          for (unsigned int l = 0; l < Parser.GetNLines(); ++l) {
            InsertLineBefore(const_cast<char*> (Parser.GetTokenizerAt(l)->GetText().Data()), i+l);
          }
        }
      }
    }
  } while (IncludesFound == true);


  // The type of the start area from which far field photons are started
  int StartAreaType = MCSource::c_StartAreaUnknown;
  // The parameters of the start area
  vector<double> StartAreaParameters(8);

  /// Parameters of the start area
  // Step 1: Check the version of the file
  int Version = 1;
  for (i = 0; i < GetNLines(); ++i) {
    MTokenizer* T = GetTokenizerAt(i);

    if (T->GetNTokens() == 0) continue;
    if (T->IsTokenAt(0, "Version", true) == true) {
      Version = T->GetTokenAtAsInt(1);
      mdebug<<"Setting version to "<<Version<<endl;
    } else if (T->IsTokenAt(0, "Run", true) == true) {
      if (T->GetNTokens() == 2) {
        // ToDo: Check if run already exists:
        m_RunList.resize(m_RunList.size()+1);
        m_RunList.back().SetName(T->GetTokenAt(1));
        mdebug<<"Adding run "<<T->GetTokenAt(1)<<endl;
      } else {
        Typo(i, "Cannot parse Run token correctly!");
        return false;
      }
    } else if (T->IsTokenAt(0, "Activator", true) == true) {
      if (T->GetNTokens() == 2) {
        // ToDo: Check if run already exists:
        m_ActivatorList.resize(m_ActivatorList.size()+1);
        m_ActivatorList.back().SetName(T->GetTokenAt(1));
        mdebug<<"Adding activator "<<T->GetTokenAt(1)<<endl;
      } else {
        Typo(i, "Cannot parse Activator token correctly!");
        return false;
      }
    } else if (T->IsTokenAt(0, "Geometry", true) == true) {
      if (T->GetNTokens() >= 2) {
        MString FileName = T->GetTokenAfterAsString(1);
        MFile::ExpandFileName(FileName);
        if (MFile::FileExists(FileName) == false) {
          Typo(i, "File does not exist!");
          return false;          
        }
        m_GeometryFileName = FileName;
        mdebug<<"Using geometry: "<<m_GeometryFileName<<endl;
      } else {
        Typo(i, "Cannot parse token GeometryFile correctly:"
             " Number of tokens is not correct!");
        return false;
      }
    } else if (T->IsTokenAt(0, "DetectorTimeConstant", true) == true) {
      if (T->GetNTokens() == 2) {
        m_DetectorTimeConstant = T->GetTokenAtAsDouble(1)*s;
        mdebug<<"Using detector time constant: "<<m_DetectorTimeConstant<<endl;
      } else {
        Typo(i, "Cannot parse token DetectorTimeConstant correctly:"
             " Number of tokens is not correct!");
        return false;
      }
    } else if (T->IsTokenAt(0, "CheckForOverlaps", true) == true) {
      if (T->GetNTokens() == 3) {
        m_CheckForOverlaps = true;
        m_OverlapCheckResolution = T->GetTokenAtAsInt(1);
        m_OverlapCheckTolerance = T->GetTokenAtAsDouble(2)*cm;
        mdebug<<"Performing overlap check (Points:"<<m_OverlapCheckResolution<<", Tolerance: "<<m_OverlapCheckTolerance<<"cm)"<<endl;
      } else {
        Typo(i, "Cannot parse token CheckForOverlaps correctly:"
             " Number of tokens is not correct!");
        return false;
      }
    } else if (T->IsTokenAt(0, "CreateCrossSectionFiles", true) == true) {
      if (T->GetNTokens() == 2) {
        m_CreateCrossSectionFiles = true;
        MString FileName = T->GetTokenAfterAsString(1);
        MFile::ExpandFileName(FileName);
        m_CrossSectionFileDirectory = FileName;
        mdebug<<"Creating cross section files ("<<m_CrossSectionFileDirectory<<")"<<endl;
      } else {
        Typo(i, "Cannot parse token CreateCrossSectionFiles correctly:"
             " Number of tokens is not correct!");
        return false;
      }
    } else if (T->IsTokenAt(0, "DefaultRangeCut", true) == true) {
      if (T->GetNTokens() == 2) {
        m_DefaultRangeCut = T->GetTokenAtAsDouble(1)*cm;
        mdebug<<"using default range cut "<<T->GetTokenAt(1)<<endl;
      } else {
        Typo(i, "Cannot parse DefaultRangeCut token correctly!");
        return false;
      }
    } else if (T->IsTokenAt(0, "Region", true) == true) {
      if (T->GetNTokens() == 2) {
        // ToDo: Check if run already exists:
        m_RegionList.resize(m_RegionList.size()+1);
        m_RegionList.back().SetName(T->GetTokenAt(1));
        mdebug<<"Adding region "<<T->GetTokenAt(1)<<endl;
      } else {
        Typo(i, "Cannot parse Region token correctly!");
        return false;
      }
    } else if (T->IsTokenAt(0, "Package", true) == true) {
      mout<<"The keyword \"Package\" is depreciated.";
      mout<<"Please use \"PhysicsListEM\" instead to distinguish between various physics lists."<<endl;
      if (T->GetNTokens() == 2) {
        m_PhysicsListEM = T->GetTokenAtAsInt(1);
        if (m_PhysicsListEM >= MCPhysicsList::c_EMMin && m_PhysicsListEM <= MCPhysicsList::c_EMMax) {
          mout<<"Using Package: "<<m_PhysicsListEM<<endl;
        } else {
          Typo(i, "Cannot parse token Package correctly:"
               " Unknown package ID!");
          return false;
        }
      } else {
        Typo(i, "Cannot parse token Package correctly:"
             " Number of tokens is not correct!");
        return false;
      }
    } else if (T->IsTokenAt(0, "PhysicsListEM", true) == true) {
      if (T->GetNTokens() == 2) {
        MString Type = T->GetTokenAtAsString(1);
        Type.ToLower();
        if (Type == "none") {
          m_PhysicsListEM = MCPhysicsList::c_EMNone;
        } else if (Type == "standard") {
          m_PhysicsListEM = MCPhysicsList::c_EMStandard;
        } else if (Type == "livermore") {
          m_PhysicsListEM = MCPhysicsList::c_EMLivermore;
        } else if (Type == "livermore-pol" || Type == "livermorepol") {
          m_PhysicsListEM = MCPhysicsList::c_EMLivermorePolarized;
        } else if (Type == "livermore-g4lecs") {
          mout<<"Depreciated: The G4LECS package is outdated since Geant4 9.2 and superseeded by the default Livermore physics list."<<endl;
          mout<<"             Therefore the livermore-g4lecs option does no longer exist."<<endl;
          mout<<"             Using the default Livermore physics list (which includes Doppler-broadening)."<<endl;
          m_PhysicsListEM = MCPhysicsList::c_EMLivermore;
        } else if (Type == "penelope") {
          m_PhysicsListEM = MCPhysicsList::c_EMPenelope;
        } else {
          Typo(i, "Cannot parse token PhysicsListEM correctly:"
               " Unknown package string (Usage: e.g. \"PhysicsListEM Livermore\"");
          return false;
        }
        mdebug<<"Using EM-Package: "<<m_PhysicsListEM<<endl;
      } else {
        Typo(i, "Cannot parse token PhysicsListEM correctly:"
             " Number of tokens is not correct!");
        return false;
      }
    } else if (T->IsTokenAt(0, "PhysicsListHD", true) == true) {
      if (T->GetNTokens() == 2) {
        MString Type = T->GetTokenAtAsString(1);
        Type.ToLower();
        if (Type == "none") {
          m_PhysicsListHD = MCPhysicsList::c_HDNone;
        } else if (Type == "qgsp-bic-hp") {
          m_PhysicsListHD = MCPhysicsList::c_HDQGSP_BIC_HP;
        } else if (Type == "qgsp-bert-hp") {
          m_PhysicsListHD = MCPhysicsList::c_HDQGSP_BERT_HP;
        } else if (Type == "ftfp-bert-hp") {
          m_PhysicsListHD = MCPhysicsList::c_HDFTFP_BERT_HP;
        } else if (Type == "ftfp-inclxx-hp") {
          m_PhysicsListHD = MCPhysicsList::c_HDFTFP_INCLXX_HP;
        } else if (Type == "qgsp-inclxx-hp") {
          m_PhysicsListHD = MCPhysicsList::c_HDQGSP_INCLXX_HP;
        } else {
          Typo(i, "Cannot parse token PhysicsListHD correctly:"
               " Unknown package string (Usage: e.g. \"PhysicsListHD qgsp-bic-hp\"");
          return false;
        }
        mdebug<<"Using HD-Package: "<<m_PhysicsListHD<<endl;
      } else {
        Typo(i, "Cannot parse token PhysicsListHD correctly:"
             " Number of tokens is not correct!");
        return false;
      }
    } else if (T->IsTokenAt(0, "ActivateFluorescence", true) == true || 
               T->IsTokenAt(0, "PhysicsListEMActivateFluorescence", true) == true) {
      mout<<"Depreciated: Fluorescence is activated by default where possible."<<endl;
    } else if (T->IsTokenAt(0, "DecayMode", true) == true) {
      if (T->GetNTokens() == 2) {
        MString Mode = T->GetTokenAtAsString(1);
        Mode.ToLower();
        if (Mode == "normal") {
          m_DecayMode = c_DecayModeNormal;
        } else if  (Mode == "buildup" || Mode == "build-up") {
          m_DecayMode = c_DecayModeBuildUp;  
        } else if  (Mode == "ignore") {
          m_DecayMode = c_DecayModeIgnore;  
        } else if  (Mode == "activationbuildup") {
          m_DecayMode = c_DecayModeActivationBuildUp;
        } else if  (Mode == "activationdelayeddecay") {
          m_DecayMode = c_DecayModeActivationDelayedDecay;
        } else {
          Typo(i, "Cannot parse token DecayMode correctly:"
               " Unknown Decay mode!");
          return false;
        }
      } else {
        Typo(i, "Cannot parse token DecayMode correctly:"
             " Number of tokens is not correct!");
        return false;
      }
    } else if (T->IsTokenAt(0, "StoreCalibrated", true) == true) {
      if (T->GetNTokens() == 2) {
        m_StoreCalibrated = T->GetTokenAtAsBoolean(1);
        mdebug<<"Storing calibrated: "<<((m_StoreCalibrated == true) ? "true" : "false")<<endl;
      } else {
        Typo(i, "Cannot parse token StoreCalibrated correctly:"
             " Number of tokens is not correct!");
        return false;
      }
    } else if (T->IsTokenAt(0, "StoreSimulationInfoVersion", true) == true) {
      if (T->GetNTokens() == 2) {
        m_StoreSimulationInfoVersion = T->GetTokenAtAsInt(1);
        mdebug<<"Using the following output version of simulation information: "<<m_StoreSimulationInfoVersion<<endl;
      } else {
        Typo(i, "Cannot parse token StoreSimulationInfoVersion correctly:"
             " Number of tokens is not correct!");
        return false;
      }
    } else if (T->IsTokenAt(0, "StoreSimulationInfo", true) == true) {
      if (T->GetNTokens() == 2) {
        MString Type = T->GetTokenAtAsString(1);
        Type.ToLower();
        if (Type == "yes" || Type == "all" || Type == "true") {
          m_StoreSimulationInfo = MSimEvent::c_StoreSimulationInfoAll;
        // } else if (Type == "deposits-only") {
        // m_StoreSimulationInfo = MSimEvent::c_StoreSimulationInfoDepositsOnly;
        } else if (Type == "init" || Type == "init-only") {
          m_StoreSimulationInfo = MSimEvent::c_StoreSimulationInfoInitOnly;
        } else if (Type == "no" || Type == "none" || Type == "false") {
          m_StoreSimulationInfo = MSimEvent::c_StoreSimulationInfoNone;
        } else {
          Typo(i, "Cannot parse token StoreSimulationInfo"
               " Unknown package string (Usage: e.g. \"StoreSimulationInfo all\"");
          return false;
        }
        mdebug<<"Storing simulation info: "<<m_StoreSimulationInfo<<endl;
      } else {
        Typo(i, "Cannot parse token StoreSimulationInfo correctly:"
             " Number of tokens is not correct!");
        return false;
      }
    } else if (T->IsTokenAt(0, "StoreSimulationInfoIonization", true) == true) {
      if (T->GetNTokens() == 2) {
        m_StoreSimulationInfoIonization = T->GetTokenAtAsBoolean(1);
        if (m_StoreSimulationInfoIonization == true) {
          m_StoreSimulationInfo = MSimEvent::c_StoreSimulationInfoAll;
        }
        mdebug<<"Storing simulation info of ionization: "<<((m_StoreSimulationInfoIonization == true) ? "true" : "false")<<endl;
      } else {
        Typo(i, "Cannot parse token StoreSimulationInfoIonization correctly:"
             " Number of tokens is not correct!");
        return false;
      }
    } else if (T->IsTokenAt(0, "StoreSimulationInfoWatchedVolumes", true) == true ||
               T->IsTokenAt(0, "StoreSimulationInfoWatchedVolume", true) == true) {
      if (T->GetNTokens() >= 2) {
        for (unsigned int w = 1; w < T->GetNTokens(); ++w) {
          m_StoreSimulationInfoWatchedVolumes.push_back(T->GetTokenAt(w));
          mdebug<<"Watching volume: "<<m_StoreSimulationInfoWatchedVolumes.back()<<endl;
        }
      } else {
        Typo(i, "Cannot parse token StoreSimulationWatchedVolumes correctly:"
             " Number of tokens is not correct!");
        return false;
      }
    } else if (T->IsTokenAt(0, "StoreScientific", true) == true ||
               T->IsTokenAt(0, "StoreSimulationInfoScientific", true) == true) {
      if (T->GetNTokens() == 3) {
        m_StoreScientific = T->GetTokenAtAsBoolean(1);
        m_StoreScientificPrecision = T->GetTokenAtAsInt(2);
        if (m_StoreScientificPrecision < 3) {
          mout<<"Minimum storage precision is 3, not "<<m_StoreScientificPrecision<<". Using 3."<<endl;
          m_StoreScientificPrecision = 3;
        }
        mdebug<<"Storing simulation info in scientific format: "<<((m_StoreScientific == true) ? "true" : "false")<<endl;
      } else {
        Typo(i, "Cannot parse token StoreSimulationInfoScientific correctly:"
             " Number of tokens is not correct!");
        return false;
      }
    } else if (T->IsTokenAt(0, "StoreOnlyEventsWithEnergyLoss", true) == true ||
               T->IsTokenAt(0, "StoreOnlyTriggeredEvents", true) == true) {
      if (T->GetNTokens() == 2) {
        m_StoreOnlyTriggeredEvents = T->GetTokenAtAsBoolean(1);
        mdebug<<"Storing only triggered events: "<<((m_StoreOnlyTriggeredEvents == true) ? "true" : "false")<<endl;
      } else {
        Typo(i, "Cannot parse token StoreOnlyTriggeredEvents correctly:"
             " Number of tokens is not correct!");
        return false;
      }
    } else if (T->IsTokenAt(0, "StoreOneHitPerEvent", true) == true) {
      if (T->GetNTokens() == 2) {
        m_StoreOneHitPerEvent = T->GetTokenAtAsBoolean(1);
        mdebug<<"Storing one hit per event: "<<((m_StoreOneHitPerEvent == true) ? "true" : "false")<<endl;
      } else {
        Typo(i, "Cannot parse token StoreOneHitPerEvent:"
             " Number of tokens is not correct!");
        return false;
      }
    } else if (T->IsTokenAt(0, "DiscretizeHits", true) == true) {
      if (T->GetNTokens() == 2) {
        m_DiscretizeHits = T->GetTokenAtAsBoolean(1);
        mdebug<<"Discretizing hits: "<<((m_DiscretizeHits == true) ? "true" : "false")<<endl;
      } else {
        Typo(i, "Cannot parse token DiscretizeHits correctly:"
             " Number of tokens is not correct!");
        return false;
      }
    } else if (T->IsTokenAt(0, "BlackAbsorber", true) == true) {
      if (T->GetNTokens() >= 2) {
        for (unsigned int w = 1; w < T->GetNTokens(); ++w) {
          m_BlackAbsorbers.push_back(T->GetTokenAt(w));
          mdebug<<"Adding black absorber: "<<m_BlackAbsorbers.back()<<endl;
        }
      } else {
        Typo(i, "Cannot parse token BlackAbsorber correctly:"
             " Number of tokens is not correct!");
        return false;
      }
    } else if (T->IsTokenAt(0, "StartArea", true) == true) {
      if (T->IsTokenAt(1, "Sphere", true) == true) {
        if (T->GetNTokens() == 9 ) {
          StartAreaType = MCSource::c_StartAreaSphere;
          StartAreaParameters.clear();
          StartAreaParameters.push_back(T->GetTokenAtAsDouble(2)*cm);
          StartAreaParameters.push_back(T->GetTokenAtAsDouble(3)*cm);
          StartAreaParameters.push_back(T->GetTokenAtAsDouble(4)*cm);
          StartAreaParameters.push_back(T->GetTokenAtAsDouble(5));
          StartAreaParameters.push_back(T->GetTokenAtAsDouble(6));
          StartAreaParameters.push_back(T->GetTokenAtAsDouble(7));
          StartAreaParameters.push_back(T->GetTokenAtAsDouble(8)*cm);
          StartAreaParameters.push_back(MCSource::c_Invalid);
        } else {
          Typo(i, "Cannot parse token StartArea Sphere correctly:"
               " Number of tokens is not correct!");
          return false;
        }
      } else if (T->IsTokenAt(1, "Tube", true) == true) {
        if (T->GetNTokens() == 10) {
          StartAreaType = MCSource::c_StartAreaTube;
          StartAreaParameters.clear();
          StartAreaParameters.push_back(T->GetTokenAtAsDouble(2)*cm);
          StartAreaParameters.push_back(T->GetTokenAtAsDouble(3)*cm);
          StartAreaParameters.push_back(T->GetTokenAtAsDouble(4)*cm);
          StartAreaParameters.push_back(T->GetTokenAtAsDouble(5));
          StartAreaParameters.push_back(T->GetTokenAtAsDouble(6));
          StartAreaParameters.push_back(T->GetTokenAtAsDouble(7));
          StartAreaParameters.push_back(T->GetTokenAtAsDouble(8)*cm);
          StartAreaParameters.push_back(T->GetTokenAtAsDouble(9)*cm);
        } else {
          Typo(i, "Cannot parse token StartArea Tube correctly:"
               " Number of tokens is not correct!");
          return false;
        }
      } else {
        Typo(i, "Cannot parse token StartArea correctly:"
             " Neither keyword Sphere nor Tube found at position 2!");
        return false;
      }
    }
  } // Step 1  


  // If we have no runs, quit with an error
  if (m_CreateCrossSectionFiles == false) {
    if (m_RunList.size() == 0 && m_ActivatorList.size() == 0) {
      mout<<"Parameter file does not contain any runs or activators!"<<endl;
      return false;
    }
  }

  // Step 2a: Primary Region parameters
  MCRegion* Region = 0;

  for (i = 0; i < GetNLines(); ++i) {
    MTokenizer* T = GetTokenizerAt(i);

    if (T->GetNTokens() == 0) continue;
    if ((Region = GetRegion(T->GetTokenAt(0))) != 0) {
      if (T->IsTokenAt(1, "Volume", true) == true) {
        if (T->GetNTokens() == 3) {
          // ToDo: Check if source already exists:
          if (Region->SetVolumeName(T->GetTokenAtAsString(2)) == true) {
            mdebug<<"Setting volume name "<<T->GetTokenAtAsString(2)<<" for region "
                  <<Region->GetName()<<endl;
          } else {
            Typo(i, "Cannot parse token Region.Volume correctly");
            return false;             
          }
        } else {
          Typo(i, "Cannot parse token Region.Volume correctly:"
               " Number of tokens is not correct!");
          return false;
        }
      }       
      else if (T->IsTokenAt(1, "RangeCut", true) == true) {
        if (T->GetNTokens() == 3) {
          // ToDo: Check if source already exists:
          if (Region->SetRangeCut(T->GetTokenAtAsDouble(2)*cm) == true) {
            mdebug<<"Setting range cut "<<T->GetTokenAtAsDouble(2)<<" cm for region "
                  <<Region->GetName()<<endl;
          } else {
            Typo(i, "Cannot parse token Region.RangeCut correctly");
            return false;             
          }
        } else {
          Typo(i, "Cannot parse token Region.RangeCut correctly:"
               " Number of tokens is not correct!");
          return false;
        }
      }
      else if (T->IsTokenAt(1, "CutAllSecondaries", true) == true) {
        if (T->GetNTokens() == 3) {
          // ToDo: Check if source already exists:
          if (Region->SetCutAllSecondaries(T->GetTokenAtAsBoolean(2)) == true) {
            mdebug<<"Setting flag to cut all secondaries "<<(T->GetTokenAtAsBoolean(2) ? "true" : "false")<<" for region "<<Region->GetName()<<endl;
          } else {
            Typo(i, "Cannot parse token Region.CutAllSecondaries correctly");
            return false;             
          }
        } else {
          Typo(i, "Cannot parse token Region.CutAllSecondaries correctly:"
               " Number of tokens is not correct!");
          return false;
        }
      }       
    }
  }


  // Step 2bi: Primary Activator parameters
  MCActivator* Activator = 0;

  for (i = 0; i < GetNLines(); ++i) {
    MTokenizer* T = GetTokenizerAt(i);
    if (T->GetNTokens() == 0) continue;
    if ((Activator = GetActivator(T->GetTokenAt(0))) != 0) {
      if (T->IsTokenAt(1, "IsotopeProductionFile", true) == true) {
        if (T->GetNTokens() >= 3) {
          MString FileName = T->GetTokenAfterAsString(2);
          MFile::ApplyPath(FileName, m_FileName);
          if (MFile::Exists(FileName) == true) {
            if (Activator->AddCountsFile(FileName) == true) {
              mdebug<<"Setting isotope production file "<<FileName<<" for activator "
                    <<Activator->GetName()<<endl;
            } else {
              Typo(i, "Cannot parse token IsotopeProductionFile correctly:"
                   " Adding counts file failed");
              return false;             
            }
          } else {
            Typo(i, "Cannot parse token IsotopeProductionFile correctly:"
                 " File does not exist - since multiple can be defined and missing one is a non-issue, I will ignore this for the time being...");
            //return false;             
          }
        } else {
          Typo(i, "Cannot parse token IsotopeProductionFile correctly:"
               " Number of tokens is not correct!");
          return false;
        }
      }
      else if (T->IsTokenAt(1, "ActivationFile", true) == true) {
        if (T->GetNTokens() >= 3) {
          if (Activator->SetOutputFileName(T->GetTokenAfterAsString(2)) == true) {
            Activator->SetOutputModeActivities();
            mdebug<<"Setting activation file "<<T->GetTokenAfterAsString(2)<<" for activator "
                  <<Activator->GetName()<<endl;
          } else {
            Typo(i, "Cannot parse token ActivationFile correctly:"
                 " Adding counts file failed");
            return false;             
          }
        } else {
          Typo(i, "Cannot parse token ActivationFile correctly:"
               " Number of tokens is not correct!");
          return false;
        }
      }
      else if (T->IsTokenAt(1, "IsotopeCountFile", true) == true) {
        if (T->GetNTokens() >= 3) {
          if (Activator->SetOutputFileName(T->GetTokenAfterAsString(2)) == true) {
            Activator->SetOutputModeParticles();
            mdebug<<"Setting activation file "<<T->GetTokenAfterAsString(2)<<" for activator "
                  <<Activator->GetName()<<" in isotope count mode"<<endl;
          } else {
            Typo(i, "Cannot parse token IsotopeCountFile correctly:"
                 " Adding counts file failed");
            return false;             
          }
        } else {
          Typo(i, "Cannot parse token IsotopeCountFile correctly:"
               " Number of tokens is not correct!");
          return false;
        }
      }
      else if (T->IsTokenAt(1, "ActivationMode", true) == true) {
        if (T->GetNTokens() >= 4) {
          MString Mode = T->GetTokenAtAsString(2);
          Mode.ToLower();
          if (Mode == "constantirradiation") {
            if (T->GetNTokens() != 4) {
              Typo(i, "Cannot parse token ActivationMode (constantirradiation) correctly:"
                   " Number of tokens is not correct!");
              return false;
            }
            if (Activator->SetConstantIrradiation(T->GetTokenAtAsDouble(3)*s) == true) {
              mdebug<<"Set constant irradiation (activation time "<<T->GetTokenAtAsDouble(2)<<" sec) for activator "
                    <<Activator->GetName()<<endl;
            } else {
              Typo(i, "Cannot parse token ActivationMode (constantirradiation) correctly"
                   "");
              return false;             
            }
          } else if  (Mode == "constantirradiationwithcooldown") {
            if (T->GetNTokens() != 5) {
              Typo(i, "Cannot parse token ActivationMode (constantirradiationwithcooldown) correctly:"
                   " Number of tokens is not correct!");
              return false;
            }
            if (Activator->SetConstantIrradiationWithCooldown(T->GetTokenAtAsDouble(3)*s, T->GetTokenAtAsDouble(4)*s) == true) {
              mdebug<<"Set constant irradiation with cool down (activation time "<<T->GetTokenAtAsDouble(2)<<" sec, cool down time "<<T->GetTokenAtAsDouble(3)<<" sec) for activator "
                    <<Activator->GetName()<<endl;
            } else {
              Typo(i, "Cannot parse token ActivationMode (constantirradiationwithcooldown) correctly");
              return false;             
            }
          } else if  (Mode == "timeprofile") {
            Typo(i, "Cannot parse token ActivationTime correctly: Not yet implemented");
//             if (Activator->SetTimeProfile() == true) {
//               mdebug<<"Set constant irradiation (activation time "<<T->GetTokenAtAsDouble(2)<<" sec) for activator "
//                     <<Activator->GetName()<<endl;
//             } else {
//               Typo(i, "Cannot parse token ActivationTime correctly"
//                    "");
//               return false;             
//             }
          } else {
            Typo(i, "Cannot parse token ActivationMode correctly:"
                 " Unknown Decay mode!");
            return false;
          }
        } else {
          Typo(i, "Cannot parse token ActivationMode correctly:"
               " Number of tokens is not correct!");
          return false;
        }
      }
    }
  }


  // Step 2c: Primary Run parameters
  MCRun* Run = 0;

  for (i = 0; i < GetNLines(); ++i) {
    MTokenizer* T = GetTokenizerAt(i);

    if (T->GetNTokens() == 0) continue;
    if ((Run = GetRun(T->GetTokenAt(0))) != 0) {
      //cout<<"Checking run: "<<Run->GetName()<<endl;
      if (T->IsTokenAt(1, "Duration", true) == true ||
          T->IsTokenAt(1, "Time", true) == true) {
        if (T->GetNTokens() == 3) {
          // ToDo: Check if source already exists:
          if (Run->SetDuration(T->GetTokenAtAsDouble(2)*s) == 
              true) {
            mdebug<<"Setting duration "<<T->GetTokenAtAsDouble(2)<<" for run "
                  <<Run->GetName()<<endl;
          } else {
            Typo(i, "Cannot parse token Duration correctly:"
                 " Number not positive?");
            return false;             
          }
        } else {
          Typo(i, "Cannot parse token Duration correctly:"
               " Number of tokens is not correct!");
          return false;
        }
      } else if (T->IsTokenAt(1, "FileName", true) == true) {
        if (T->GetNTokens() == 3) {
          // ToDo: Check if source already exists:
          if (Run->SetFileName(T->GetTokenAtAsString(2)) == true) {
            if (MFile::FileExists(T->GetTokenAtAsString(2)) == false) {
              mdebug<<"Setting output file name "<<T->GetTokenAtAsString(2)
                    <<" for run "<<Run->GetName()<<endl;
            } else {
              Typo(i, "File does already exist!");
              return false;             
            }
          } else {
            Typo(i, "Cannot parse token FileName correctly!");
            return false;             
          }
        } else {
          Typo(i, "Cannot parse token FileName correctly:"
               " Number of tokens is not correct!");
          return false;
        }
      } else if (T->IsTokenAt(1, "TcpIp", true) == true) {
        if (T->GetNTokens() == 4) {
          Run->SetTcpIpHostName(T->GetTokenAtAsString(2));
          Run->SetTcpIpPort(T->GetTokenAtAsUnsignedInt(3));
        } else {
          Typo(i, "Cannot parse token TcpIp correctly:"
               " Number of tokens is not correct!");
          return false;
        }
      } else if (T->IsTokenAt(1, "Triggers", true) == true || 
                 T->IsTokenAt(1, "Trigger", true) == true ||
                 T->IsTokenAt(1, "NTrigger", true) == true ||
                 T->IsTokenAt(1, "NTriggers", true) == true) {
        if (T->GetNTokens() >= 3) {
          // ToDo: Check if source already exists:
          if (Run->SetTriggers(T->GetTokenAtAsInt(2)) == true) {
            mdebug<<"Setting number of triggers "
                  <<T->GetTokenAtAsInt(2)<<" for run "<<Run->GetName()<<endl;
          } else {
            Typo(i, "Cannot parse token Triggers correctly:"
                 " Number not positive?");
            return false;             
          }
        } else {
          Typo(i, "Cannot parse token Triggers correctly:"
               " Number of tokens is not correct!");
          return false;
        }
      } else if (T->IsTokenAt(1, "Events", true) == true ||
                 T->IsTokenAt(1, "Event", true) == true ||
                 T->IsTokenAt(1, "NEvent", true) == true ||
                 T->IsTokenAt(1, "NEvents", true) == true) {
        if (T->GetNTokens() >= 3) {
          // ToDo: Check if source already exists:
          if (Run->SetEvents(T->GetTokenAtAsInt(2)) == true) {
            mdebug<<"Setting number of events "
                  <<T->GetTokenAtAsInt(2)<<" for run "<<Run->GetName()<<endl;
          } else {
            Typo(i, "Cannot parse token Events correctly:"
                 " Number not positive?");
            return false;             
          }
        } else {
          Typo(i, "Cannot parse token Events correctly:"
               " Number of tokens is not correct!");
          return false;
        }
      } else if (T->IsTokenAt(1, "IsotopeProductionFile", true) == true) {
        if (T->GetNTokens() >= 3) {
          MString Name = T->GetTokenAfterAsString(2);
          Run->SetIsotopeStoreFileName(Name);
        } else {
          Typo(i, "Cannot parse token IsotopeProductionFile correctly:"
               " Number of tokens is not correct!");
          return false;
        }
      } else if (T->IsTokenAt(1, "OrientationSky", true) == true) {
        if (T->GetNTokens() >= 4) {
          MCOrientation O;
          if (O.Parse(*T) == false) {
            Typo(i, "Cannot parse token \"OrientationSky\" correctly");
            return false;
          }          
          // The sky can only be rotated if in Galactic coordinates
          if (O.GetCoordinateSystem() != MCOrientationCoordinateSystem::c_Galactic && O.IsOriented() == true) {
            Typo(i, "\"OrientationSky\" can only have an orientation in Galactic coordinates!");
            return false;            
          }
          Run->SetSkyOrientation(O);
          
        } else {
          Typo(i, "Cannot parse token \"OrientationSky\" correctly: Number of tokens is not correct!");
          return false;
        }
      } else if (T->IsTokenAt(1, "OrientationDetector", true) == true) {
        if (T->GetNTokens() >= 4) {
          MCOrientation O;
          if (O.Parse(*T) == false) {
            Typo(i, "Cannot parse token \"OrientationDetector\" correctly");
            return false;
          }
          if (O.GetCoordinateSystem() != MCOrientationCoordinateSystem::c_Local || O.IsOriented() == true) {
            Typo(i, "\"OrientationDetector\" for the time being the detector cannot be oriented (leave at Local Fixed)!");
            return false;            
          }
          Run->SetDetectorOrientation(O);
        } else {
          Typo(i, "Cannot parse token \"OrientationDetector\" correctly: Number of tokens is not correct!");
          return false;
        }
      }
    } // is run
  } // Step 2

  // Step 3: Secondary Run parameters
  MCSource* Source = 0;
  for (i = 0; i < GetNLines(); ++i) {
    MTokenizer* T = GetTokenizerAt(i);

    if (T->GetNTokens() == 0) continue;
    if ((Run = GetRun(T->GetTokenAt(0))) != 0) {
      if (T->IsTokenAt(1, "Source", true) == true) {
        if (T->GetNTokens() == 3) {
          if ((Source = GetSource(T->GetTokenAt(2))) == 0) {
            Run->AddSource(T->GetTokenAt(2));
            mdebug<<"Adding source "<<T->GetTokenAt(2)<<" to run "
                  <<Run->GetName()<<endl;
          } else {
            Typo(i, "Source already exists!");
            return false;
          }
        } else {
          Typo(i, "Cannot parse Source token correctly!");
          return false;
        }
      }
      else if (T->IsTokenAt(1, "ActivationSources", true) == true) {
        if (T->GetNTokens() == 3) {
          // Extract all sources from the file
          MCIsotopeStore Store;
          if (Store.Load(T->GetTokenAtAsString(2)) == true) {
            vector<MCSource*> List = Store.CreateSourceListByActivity();
            for (unsigned int so = 0; so < List.size(); ++so) {
              // We could check if it already exists and add the rate here...
              cout<<"Source: "<<List[so]->GetName()<<endl;
              Run->AddSource(List[so], false);
            }
          } else {
            Typo(i, "Cannot load ActivationSources file token correctly!");
            return false;
          }
        } else {
          Typo(i, "Cannot parse ActivationSources token correctly!");
          return false;
        }
      }
      else if (T->IsTokenAt(1, "IsotopeCountSources", true) == true) {
        if (T->GetNTokens() == 3) {
          // Extract all sources from the file
          MCIsotopeStore Store;
          if (Store.Load(T->GetTokenAtAsString(2)) == true) {
            vector<MCSource*> List = Store.CreateSourceListByIsotopeCount();
            for (unsigned int so = 0; so < List.size(); ++so) {
              // We could check if it already exists and add the rate here...
              cout<<"Source: "<<List[so]->GetName()<<endl;
              Run->AddSource(List[so], false);
            }
          } else {
            Typo(i, "Cannot load IsotopeCountSources file token correctly!");
            return false;
          }
        } else {
          Typo(i, "Cannot parse IsotopeCountSources token correctly!");
          return false;
        }
      }
      else if (T->IsTokenAt(1, "Trigger", true) == true) {
        mout<<"Depreciated: The Trigger keyword is no longer used."<<endl;
        mout<<"             Instead the trigger criteria in the geometry file are used as pre-trigger criteria."<<endl;
        mout<<"             This means vetoes are ignored and the thresholds are set to zero."<<endl;
      }
    } // is run
  } // Step 3



  // Step 4: Ultimate source parameters
  for (i = 0; i < GetNLines(); ++i) {
    MTokenizer* T = GetTokenizerAt(i);

    if (T->GetNTokens() == 0) continue;

    // Source parameters:
    if ((Source = GetSource(T->GetTokenAt(0))) != 0) {
      if (T->IsTokenAt(1, "EventList", true) == true) {
        if (T->GetNTokens() == 3) {
          if (Source->SetEventListFromFile(T->GetTokenAtAsString(2)) == false) {
            Typo(i, "Unable to add event list!");
            return false;
          }
        } else {
          Typo(i, "Cannot parse token SpectralType correctly: Number of tokens is not correct!");
          return false;
        }
      }
    }
  }
      
      
  // Step 5: Primary source parameters
  for (i = 0; i < GetNLines(); ++i) {
    MTokenizer* T = GetTokenizerAt(i);

    if (T->GetNTokens() == 0) continue;

    // Source parameters:
    if ((Source = GetSource(T->GetTokenAt(0))) != 0) {
      
      if (Source->IsEventList() == true) {
        if (T->IsTokenAt(1, "EventList", true) == false) {
          Typo(i, "This source is an event list and cannot have any other parameters!");
          return false;
        }
      }
      
      //cout<<"Checking run: "<<Run->GetName()<<endl;
      if (T->IsTokenAt(1, "SpectralType", true) == true) {
        mout<<"Depreciated: SpectralType"<<endl
            <<"  The keywords \"SpectralType\", \"Energy\", and \"Position\" are depreciated!"<<endl
            <<"  Please use \"Spectrum\" and \"Beam\" instead."<<endl
            <<"  Consult the manual for further information."<<show;

        if (T->GetNTokens() == 3) {
          // ToDo: Check if source already exists:
          if (Source->SetSpectralType(T->GetTokenAtAsInt(2)) == true) {
            mdebug<<"Setting spectral type "<<T->GetTokenAtAsInt(2)
                  <<" for source "<<Source->GetName()<<endl;
          } else {
            Typo(i, "Cannot parse token SpectralType correctly:"
                 " Unknown spectral type!");
            return false;
          }
        } else {
          Typo(i, "Cannot parse token SpectralType correctly:"
               " Number of tokens is not correct!");
          return false;
        }
      } 
      
      
      else if (T->IsTokenAt(1, "Spectrum", true) == true) {
        if (T->GetNTokens() >= 3) {
          // Begin different spectra
          MString Type = T->GetTokenAtAsString(2);
          Type.ToLower();
          if (Type == "mono" || Type == "m") {
            Source->SetSpectralType(MCSource::c_Monoenergetic);
            if (T->GetNTokens() == 4) {
              if (Source->SetEnergy(T->GetTokenAtAsDouble(3)*keV) == true) {
                mdebug<<"Setting energy "<<T->GetTokenAtAsDouble(3)*keV
                      <<"keV for source "<<Source->GetName()<<endl;
              } else {
                Typo(i, "Cannot parse token Spectrum - mono correctly:"
                     " Content not reasonable");
                return false;
              }
            } else {
              Typo(i, "Cannot parse token Spectrum - mono correctly:"
                   " Number of tokens is not correct!");
              return false;
            }
          } 
          else if (Type == "linear" || Type == "l") {
            if (T->GetNTokens() == 5) {
              Source->SetSpectralType(MCSource::c_Linear);
              if (Source->SetEnergy(T->GetTokenAtAsDouble(3)*keV, 
                                    T->GetTokenAtAsDouble(4)*keV) == true) {
                mdebug<<"Setting energy min ="<<T->GetTokenAtAsDouble(3)
                      <<" keV and max = "<<T->GetTokenAtAsDouble(4)
                      <<"keV for source "<<Source->GetName()<<endl;
              } else {
                Typo(i, "Cannot parse token Spectrum - linear correctly:"
                     " Content not reasonable");
                return false;
              }
            } else {
              Typo(i, "Cannot parse token Spectrum - linear correctly:"
                   " Number of tokens is not correct!");
              return false;
            }
          } 
          else if (Type == "powerlaw" || Type == "pl") {
            if (T->GetNTokens() == 6) {
              Source->SetSpectralType(MCSource::c_PowerLaw);
              if (Source->SetEnergy(T->GetTokenAtAsDouble(3)*keV, 
                                    T->GetTokenAtAsDouble(4)*keV,
                                    T->GetTokenAtAsDouble(5)) == true) {
                mdebug<<"Setting energy min ="<<T->GetTokenAtAsDouble(3)
                      <<" keV and max = "<<T->GetTokenAtAsDouble(4)
                      <<" keV and alpha = "<<T->GetTokenAtAsDouble(5)
                      <<" keV for source "<<Source->GetName()<<endl;
              } else {
                Typo(i, "Cannot parse token Spectrum - power law correctly:"
                     " Content not reasonable");
                return false;
              }
            } else {
              Typo(i, "Cannot parse token Spectrum - power law correctly:"
                   " Number of tokens is not correct!");
              return false;
            }
          } 
          else if (Type == "brokenpowerlaw" || Type == "bpl") {
            if (T->GetNTokens() == 8) {
              Source->SetSpectralType(MCSource::c_BrokenPowerLaw);
              if (Source->SetEnergy(T->GetTokenAtAsDouble(3)*keV, 
                                    T->GetTokenAtAsDouble(4)*keV,
                                    T->GetTokenAtAsDouble(5)*keV,
                                    T->GetTokenAtAsDouble(6),
                                    T->GetTokenAtAsDouble(7)) == true) {
                mdebug<<"Setting energy min = "<<T->GetTokenAtAsDouble(3)
                      <<" keV and max = "<<T->GetTokenAtAsDouble(4)
                      <<" keV and break = "<<T->GetTokenAtAsDouble(5)
                      <<" keV and alpha low = "<<T->GetTokenAtAsDouble(6)
                      <<" and  alpha high = "<<T->GetTokenAtAsDouble(7)
                      <<" for source "<<Source->GetName()<<endl;
              } else {
                Typo(i, "Cannot parse token Spectrum - broken power law correctly:"
                     " Content not reasonable");
                return false;
              }
            } else {
              Typo(i, "Cannot parse token Spectrum - broken power law correctly:"
                   " Number of tokens is not correct!");
              return false;
            }
          } 
          else if (Type == "gaussian" || Type == "g") {
            if (T->GetNTokens() == 6) {
              Source->SetSpectralType(MCSource::c_Gaussian);
              if (Source->SetEnergy(T->GetTokenAtAsDouble(3)*keV, 
                                    T->GetTokenAtAsDouble(4)*keV,
                                    T->GetTokenAtAsDouble(5)*keV) == true) {
                mdebug<<"Setting avg = "<<T->GetTokenAtAsDouble(3)
                      <<" sigma = "<<T->GetTokenAtAsDouble(4)
                      <<" sigma cut-off = "<<T->GetTokenAtAsDouble(5)
                      <<" for source "<<Source->GetName()<<endl;
              } else {
                Typo(i, "Cannot parse token Spectrum - Gaussian correctly:"
                     " Content not reasonable");
                return false;
              }
            } else {
              Typo(i, "Cannot parse token Spectrum - Gaussian correctly:"
                   " Number of tokens is not correct!");
              return false;
            }
          } 
          else if (Type == "thermalbremsstrahlung" || Type == "tb") {
            if (T->GetNTokens() == 6) {
              Source->SetSpectralType(MCSource::c_ThermalBremsstrahlung);
              if (Source->SetEnergy(T->GetTokenAtAsDouble(3)*keV, 
                                    T->GetTokenAtAsDouble(4)*keV,
                                    T->GetTokenAtAsDouble(5)*keV) == true) {
                mdebug<<"Setting min = "<<T->GetTokenAtAsDouble(3)
                      <<" max = "<<T->GetTokenAtAsDouble(4)
                      <<" temperature = "<<T->GetTokenAtAsDouble(5)
                      <<" for source "<<Source->GetName()<<endl;
              } else {
                Typo(i, "Cannot parse token Spectrum - thermal bremsstrahlung correctly:"
                     " Content not reasonable");
                return false;
              }
            } else {
              Typo(i, "Cannot parse token Spectrum - thermal bremsstrahlung correctly:"
                   " Number of tokens is not correct!");
              return false;
            }
          } 
          else if (Type == "blackbody" || Type == "bb") {
            if (T->GetNTokens() == 6) {
              Source->SetSpectralType(MCSource::c_BlackBody);
              if (Source->SetEnergy(T->GetTokenAtAsDouble(3)*keV, 
                                    T->GetTokenAtAsDouble(4)*keV,
                                    T->GetTokenAtAsDouble(5)*keV) == true) {
                mdebug<<"Setting min = "<<T->GetTokenAtAsDouble(3)
                      <<" max = "<<T->GetTokenAtAsDouble(4)
                      <<" temperature = "<<T->GetTokenAtAsDouble(5)
                      <<" for source "<<Source->GetName()<<endl;
              } else {
                Typo(i, "Cannot parse token Spectrum - black body correctly:"
                     " Content not reasonable");
                return false;
              }
            } else {
              Typo(i, "Cannot parse token Spectrum - black body correctly:"
                   " Number of tokens is not correct!");
              return false;
            }
          } 
          else if (Type == "bandfunction" || Type == "bf") {
            if (T->GetNTokens() == 8) {
              Source->SetSpectralType(MCSource::c_BandFunction);
              if (Source->SetEnergy(T->GetTokenAtAsDouble(3)*keV, 
                                    T->GetTokenAtAsDouble(4)*keV,
                                    T->GetTokenAtAsDouble(5),
                                    T->GetTokenAtAsDouble(6),
                                    T->GetTokenAtAsDouble(7)*keV) == true) {
                mdebug<<"Setting min = "<<T->GetTokenAtAsDouble(3)
                      <<" max = "<<T->GetTokenAtAsDouble(4)
                      <<" alpha = "<<T->GetTokenAtAsDouble(5)
                      <<" beta = "<<T->GetTokenAtAsDouble(6)
                      <<" E0 = "<<T->GetTokenAtAsDouble(7)
                      <<" for source "<<Source->GetName()<<endl;
              } else {
                Typo(i, "Cannot parse token Spectrum - band function correctly:"
                     " Content not reasonable");
                return false;
              }
            } else {
              Typo(i, "Cannot parse token Spectrum - band function correctly:"
                   " Number of tokens is not correct!");
              return false;
            }
          } 
          else if (Type == "filediffflux" || Type == "file" || Type == "f") {
            if (T->GetNTokens() >= 4) {
              Source->SetSpectralType(MCSource::c_FileDifferentialFlux);
              MString FileName = T->GetTokenAfterAsString(3);
              MFile::ApplyPath(FileName, m_FileName);
              if (MFile::Exists(FileName) == false) {
                Typo(i, "Cannot parse token Spectrum - file correctly:"
                     " File not found!");
                return false;
              }
              if (Source->SetEnergy(FileName) == false) {
                Typo(i, "Cannot parse token Spectrum - file correctly:"
                     " Unable to initialize spectrum");
                return false;
              }
              mdebug<<"Using spectrum: "<<Type<<endl;
            } else {
              Typo(i, "Cannot parse token Spectrum - file correctly:"
                   " Number of tokens must be larger than 3!");
              return false;
            }
          }
          else if (Type == "normalizedenergybeamfluxfunction") {
            if (T->GetNTokens() == 3) {
              Source->SetSpectralType(MCSource::c_NormalizedEnergyBeamFluxFunction);
              // nothing else to be done, the rest happens in the beam
              mdebug<<"Using spectrum: "<<Type<<endl;
            } else {
              Typo(i, "Cannot parse token Spectrum - normalizedenergybeamfluxfunction correctly:"
                   " Number of tokens must be equal to 3!");
              cout<<"Tokens: "<<T->GetNTokens()<<endl;
              return false;
            }
          }
          else {
            Typo(i, MString("Cannot parse token Spectrum correctly: Unknown spectral type: ") + Type);
            return false;
          }
          // End different spectra
        } else {
          Typo(i, "Cannot parse token \"Spectrum\" correctly:"
               " Number of tokens must be larger than 3!");
          return false;
        }
      } 
      
      
      else if (T->IsTokenAt(1, "Beam", true) == true) {
        if (T->GetNTokens() >= 3) {
          MString Type = T->GetTokenAtAsString(2);
          Type.ToLower();
          if (Type == "farfieldpointsource" || Type == "farfieldpoint" || Type == "ffps") {
            if (T->GetNTokens() == 5) {
              Source->SetBeamType(MCSource::c_FarField,
                                  MCSource::c_FarFieldPoint);
              if (Source->SetPosition(T->GetTokenAtAsDouble(3)*deg,
                                      T->GetTokenAtAsDouble(4)*deg) == true) {
                mdebug<<"Setting position "<<T->GetTokenAtAsDouble(3)
                      <<"/"<<T->GetTokenAtAsDouble(4)
                      <<" for source "<<Source->GetName()<<endl;
              } else {
                Typo(i, "Cannot parse token \"Beam - far field point source\" correctly:"
                     " Content not reasonable");
                return false;
              }
            } else {
              Typo(i, "Cannot parse token \"Beam - far field point source\" correctly:"
                   " Number of tokens is not correct!");
              return false;
            }
          } 
          else if (Type == "farfieldareasource" || Type == "farfieldarea" || Type == "ffas") {
            if (T->GetNTokens() == 7) {
              Source->SetBeamType(MCSource::c_FarField,
                                  MCSource::c_FarFieldArea);
              if (Source->SetPosition(T->GetTokenAtAsDouble(3)*deg,
                                      T->GetTokenAtAsDouble(4)*deg,
                                      T->GetTokenAtAsDouble(5)*deg,
                                      T->GetTokenAtAsDouble(6)*deg) == true) {
                mdebug<<"Setting position "<<T->GetTokenAtAsDouble(3)
                      <<"/"<<T->GetTokenAtAsDouble(4)
                      <<"/"<<T->GetTokenAtAsDouble(5)
                      <<"/"<<T->GetTokenAtAsDouble(6)
                      <<" for source "<<Source->GetName()<<endl;
              } else {
                Typo(i, "Cannot parse token \"Beam - far field area source\" correctly:"
                     " Content not reasonable");
                return false;
              }
            } else {
              Typo(i, "Cannot parse token \"Beam - far field area source\" correctly:"
                   " Number of tokens is not correct (!= 8)!");
              return false;
            }
          } 
          else if (Type == "farfieldgaussian" || Type == "ffg") {
            if (T->GetNTokens() == 6) {
              Source->SetBeamType(MCSource::c_FarField,
                                  MCSource::c_FarFieldGaussian);
              if (Source->SetPosition(T->GetTokenAtAsDouble(3)*deg,
                                      T->GetTokenAtAsDouble(4)*deg,
                                      T->GetTokenAtAsDouble(5)*deg) == true) {
                mdebug<<"Setting far field position theta="<<T->GetTokenAtAsDouble(3)
                <<", phi="<<T->GetTokenAtAsDouble(4)<<", sigma="<<T->GetTokenAtAsDouble(5)
                <<" for source "<<Source->GetName()<<endl;
                } else {
                  Typo(i, "Cannot parse token \"Beam - far field gaussian\" correctly:"
                  " Content not reasonable");
                  return false;
                }
            } else {
              Typo(i, "Cannot parse token \"Beam - far field gaussian\" correctly:"
              " Number of tokens is not correct!");
              return false;
            }
          } 
          else if (Type == "farfieldfilezenithdependent" || Type == "fffzd" || Type == "filespherezenithdep") {
            if (T->GetNTokens() >= 4) {
              Source->SetBeamType(MCSource::c_FarField,
                                  MCSource::c_FarFieldFileZenithDependent);
              MString FileName = T->GetTokenAfterAsString(3);
              MFile::ApplyPath(FileName, m_FileName);
              if (MFile::Exists(FileName) == false) {
                Typo(i, "Cannot parse token \"Beam - far field file zenith dependent\" correctly:"
                     " File not found!");
                return false;
              }
              if (Source->SetPosition(FileName) == false) {
                Typo(i, "Cannot parse token \"Beam - far field file zenith dependent\" correctly:"
                     " Unable to initialize beam");
                return false;
              }
              mdebug<<"Using beam: "<<Type<<endl;
            } else {
              Typo(i, "Cannot parse token \"Beam - far field file zenith dependent\" correctly:"
                   " Number of tokens must be larger than 3!");
              return false;
            }
          }
          else if (Type == "farfieldnormalizedenergybeamfluxfunction") {
            if (T->GetNTokens() >= 4) {
              Source->SetBeamType(MCSource::c_FarField,
                                  MCSource::c_FarFieldNormalizedEnergyBeamFluxFunction);
              MString FileName = T->GetTokenAfterAsString(3);
              MFile::ApplyPath(FileName, m_FileName);
              if (MFile::Exists(FileName) == false) {
                Typo(i, "Cannot parse token \"Beam - far field normalized energy beam flux function\" correctly:"
                     " File not found!");
                return false;
              }
              if (Source->SetNormalizedEnergyBeamFluxFunction(FileName) == false) {
                Typo(i, "Cannot parse token \"Beam - far field normalized energy beam flux function\" correctly:"
                     " Unable to initialize beam");
                return false;
              }
              mdebug<<"Using beam: "<<Type<<endl;
            } else {
              Typo(i, "Cannot parse token \"Beam - far field normalized energy beam flux function\" correctly:"
                   " Number of tokens must be larger than 3!");
              return false;
            }
          }
          else if (Type == "pointsource" || Type == "point" || Type == "ps") {
            if (T->GetNTokens() == 6) {
              Source->SetBeamType(MCSource::c_NearField,
                                  MCSource::c_NearFieldPoint);
              if (Source->SetPosition(T->GetTokenAtAsDouble(3)*cm,
                                      T->GetTokenAtAsDouble(4)*cm,
                                      T->GetTokenAtAsDouble(5)*cm) == true) {
                mdebug<<"Setting position "<<T->GetTokenAtAsDouble(3)
                      <<"/"<<T->GetTokenAtAsDouble(4)
                      <<"/"<<T->GetTokenAtAsDouble(5)
                      <<" for source "<<Source->GetName()<<endl;
              } else {
                Typo(i, "Cannot parse token \"Beam - point source\" correctly:"
                     " Content not reasonable");
                return false;
              }
            } else {
              Typo(i, "Cannot parse token \"Beam - point source\" correctly:"
                   " Number of tokens is not correct (!= 6)!");
              return false;
            }
          }  
          else if (Type == "restrictedpointsource" || Type == "restrictedpoint" || Type == "pointsourcerestricted" || Type == "rps" || Type == "psr") {
            if (T->GetNTokens() == 6) {
              Source->SetBeamType(MCSource::c_NearField,
                                  MCSource::c_NearFieldRestrictedPoint);
              if (Source->SetPosition(T->GetTokenAtAsDouble(3)*cm,
                                      T->GetTokenAtAsDouble(4)*cm,
                                      T->GetTokenAtAsDouble(5)*cm) == true) {
                mdebug<<"Setting position "<<T->GetTokenAtAsDouble(3)
                      <<"/"<<T->GetTokenAtAsDouble(4)
                      <<"/"<<T->GetTokenAtAsDouble(5)
                      <<" for source "<<Source->GetName()<<endl;
              } else {
                Typo(i, "Cannot parse token \"Beam - restricted point source\" correctly:"
                     " Content not reasonable");
                return false;
              }
            } else {
              Typo(i, "Cannot parse token \"Beam - restricted point source\" correctly:"
                   " Number of tokens is not correct (!= 6)!");
              return false;
            }
          }  
          else if (Type == "linesource" || Type == "line" || Type == "ls") {
            if (T->GetNTokens() == 9) {
              Source->SetBeamType(MCSource::c_NearField,
                                  MCSource::c_NearFieldLine);
              if (Source->SetPosition(T->GetTokenAtAsDouble(3)*cm,
                                      T->GetTokenAtAsDouble(4)*cm,
                                      T->GetTokenAtAsDouble(5)*cm,
                                      T->GetTokenAtAsDouble(6)*cm,
                                      T->GetTokenAtAsDouble(7)*cm,
                                      T->GetTokenAtAsDouble(8)*cm) == true) {
                mdebug<<"Setting position "<<T->GetTokenAtAsDouble(3)
                      <<"/"<<T->GetTokenAtAsDouble(4)
                      <<"/"<<T->GetTokenAtAsDouble(5)
                      <<"/"<<T->GetTokenAtAsDouble(6)
                      <<"/"<<T->GetTokenAtAsDouble(7)
                      <<"/"<<T->GetTokenAtAsDouble(8)
                      <<" for source "<<Source->GetName()<<endl;
              } else {
                Typo(i, "Cannot parse token \"Beam - line source\" correctly:"
                     " Content not reasonable");
                return false;
              }
            } else {
              Typo(i, "Cannot parse token \"Beam - line source\" correctly:"
                   " Number of tokens is not correct (!= 9)!");
              return false;
            }
          }  
          else if (Type == "restrictedlinesource" || Type == "restrictedline" || Type == "rls") {
            if (T->GetNTokens() == 9) {
              Source->SetBeamType(MCSource::c_NearField,
                                  MCSource::c_NearFieldRestrictedLine);
              if (Source->SetPosition(T->GetTokenAtAsDouble(3)*cm,
                                      T->GetTokenAtAsDouble(4)*cm,
                                      T->GetTokenAtAsDouble(5)*cm,
                                      T->GetTokenAtAsDouble(6)*cm,
                                      T->GetTokenAtAsDouble(7)*cm,
                                      T->GetTokenAtAsDouble(8)*cm) == true) {
                mdebug<<"Setting position "<<T->GetTokenAtAsDouble(3)
                      <<"/"<<T->GetTokenAtAsDouble(4)
                      <<"/"<<T->GetTokenAtAsDouble(5)
                      <<"/"<<T->GetTokenAtAsDouble(6)
                      <<"/"<<T->GetTokenAtAsDouble(7)
                      <<"/"<<T->GetTokenAtAsDouble(8)
                      <<" for source "<<Source->GetName()<<endl;
              } else {
                Typo(i, "Cannot parse token \"Beam - restricted line source\" correctly:"
                     " Content not reasonable");
                return false;
              }
            } else {
              Typo(i, "Cannot parse token \"Beam - restricted line source\" correctly:"
                   " Number of tokens is not correct (!= 9)!");
              return false;
            }
          }  
          else if (Type == "boxsource" || Type == "box" || Type == "bs") {
            if (T->GetNTokens() == 9) {
              Source->SetBeamType(MCSource::c_NearField,
                                    MCSource::c_NearFieldBox);
              if (Source->SetPosition(T->GetTokenAtAsDouble(3)*cm,
                                      T->GetTokenAtAsDouble(4)*cm,
                                      T->GetTokenAtAsDouble(5)*cm,
                                      T->GetTokenAtAsDouble(6)*cm,
                                      T->GetTokenAtAsDouble(7)*cm,
                                      T->GetTokenAtAsDouble(8)*cm) == true) {
                mdebug<<"Setting position "<<T->GetTokenAtAsDouble(3)
                      <<"/"<<T->GetTokenAtAsDouble(4)
                      <<"/"<<T->GetTokenAtAsDouble(5)
                      <<"/"<<T->GetTokenAtAsDouble(6)
                      <<"/"<<T->GetTokenAtAsDouble(7)
                      <<"/"<<T->GetTokenAtAsDouble(8)
                      <<" for source "<<Source->GetName()<<endl;
              } else {
                Typo(i, "Cannot parse token \"Beam - box source\" correctly:"
                     " Content not reasonable");
                return false;
              }
            } else {
              Typo(i, "Cannot parse token \"Beam - box source\" correctly:"
                   " Number of tokens is not correct (!= 9)!");
              return false;
            }
          }  
          else if (Type == "spheresource" || Type == "sphere" || Type == "ss") {
            if (T->GetNTokens() == 9) {
              Source->SetBeamType(MCSource::c_NearField,
                                  MCSource::c_NearFieldSphere);
              if (Source->SetPosition(T->GetTokenAtAsDouble(3)*cm,
                                      T->GetTokenAtAsDouble(4)*cm,
                                      T->GetTokenAtAsDouble(5)*cm,
                                      T->GetTokenAtAsDouble(6)*cm,
                                      T->GetTokenAtAsDouble(7)*cm,
                                      T->GetTokenAtAsDouble(8)*cm) == true) {
                mdebug<<"Setting position "<<T->GetTokenAtAsDouble(3)
                      <<"/"<<T->GetTokenAtAsDouble(4)
                      <<"/"<<T->GetTokenAtAsDouble(5)
                      <<" with radii "<<T->GetTokenAtAsDouble(6)
                      <<"/"<<T->GetTokenAtAsDouble(7)
                      <<"/"<<T->GetTokenAtAsDouble(8)
                      <<" for source "<<Source->GetName()<<endl;
              } else {
                Typo(i, "Cannot parse token \"Beam - sphere source\" correctly:"
                     " Content not reasonable");
                return false;
              }
            } else {
              Typo(i, "Cannot parse token \"Beam - sphere source\" correctly:"
                   " Number of tokens is not correct (!= 9)!");
              return false;
            }
          }  
          else if (Type == "discsource" || Type == "disc" || Type == "disksource" || Type == "disk" || Type == "ds") {
            if (T->GetNTokens() == 14) {
              Source->SetBeamType(MCSource::c_NearField,
                                  MCSource::c_NearFieldDisk);
              if (Source->SetPosition(T->GetTokenAtAsDouble(3)*cm,
                                      T->GetTokenAtAsDouble(4)*cm,
                                      T->GetTokenAtAsDouble(5)*cm,
                                      T->GetTokenAtAsDouble(6)*cm,
                                      T->GetTokenAtAsDouble(7)*cm,
                                      T->GetTokenAtAsDouble(8)*cm,
                                      T->GetTokenAtAsDouble(9)*cm,
                                      T->GetTokenAtAsDouble(10)*cm,
                                      T->GetTokenAtAsDouble(11)*cm,
                                      T->GetTokenAtAsDouble(12)*deg,
                                      T->GetTokenAtAsDouble(13)*deg) == true) {
                mdebug<<"Setting position "<<T->GetTokenAtAsDouble(3)
                      <<"/"<<T->GetTokenAtAsDouble(4)
                      <<"/"<<T->GetTokenAtAsDouble(5)
                      <<" with direction "<<T->GetTokenAtAsDouble(6)
                      <<"/"<<T->GetTokenAtAsDouble(7)
                      <<"/"<<T->GetTokenAtAsDouble(8)
                      <<" with inner radius "<<T->GetTokenAtAsDouble(9)
                      <<" with outer radius "<<T->GetTokenAtAsDouble(10)
                      <<" with height "<<T->GetTokenAtAsDouble(11)
                      <<" with angles "<<T->GetTokenAtAsDouble(12)
                      <<" to "<<T->GetTokenAtAsDouble(13)
                      <<" for source "<<Source->GetName()<<endl;
              } else {
                Typo(i, "Cannot parse token \"Beam - disk source\" correctly:"
                     " Content not reasonable");
                return false;
              }
            } else {
              Typo(i, "Cannot parse token \"Beam - disk source\" correctly:"
                   " Number of tokens is not correct (!= 14)!");
              return false;
            }
          }  
          else if (Type == "homogeneousbeam" || Type == "hb") {
            if (T->GetNTokens() == 10) {
              Source->SetBeamType(MCSource::c_NearField,
                                    MCSource::c_NearFieldBeam);
              if (Source->SetPosition(T->GetTokenAtAsDouble(3)*cm,
                                      T->GetTokenAtAsDouble(4)*cm,
                                      T->GetTokenAtAsDouble(5)*cm,
                                      T->GetTokenAtAsDouble(6),
                                      T->GetTokenAtAsDouble(7),
                                      T->GetTokenAtAsDouble(8),
                                      T->GetTokenAtAsDouble(9)*cm) == true) {
                mdebug<<"Setting position "<<T->GetTokenAtAsDouble(3)
                      <<"/"<<T->GetTokenAtAsDouble(4)
                      <<"/"<<T->GetTokenAtAsDouble(5)
                      <<"/"<<T->GetTokenAtAsDouble(6)
                      <<"/"<<T->GetTokenAtAsDouble(7)
                      <<"/"<<T->GetTokenAtAsDouble(8)
                      <<"/"<<T->GetTokenAtAsDouble(9)
                      <<" for source "<<Source->GetName()<<endl;
              } else {
                Typo(i, "Cannot parse token \"Beam - homogeneous beam\" correctly:"
                     " Content not reasonable");
                return false;
              }
            } else {
              Typo(i, "Cannot parse token \"Beam - homogeneous beam\" correctly:"
                   " Number of tokens is not correct (!= 10)!");
              return false;
            }
          }  
          else if (Type == "radialprofilebeam" || Type == "rpb") {
            if (T->GetNTokens() == 10) {
              Source->SetBeamType(MCSource::c_NearField,
                                  MCSource::c_NearFieldBeam1DProfile);
              MString FileName = T->GetTokenAfterAsString(9);
              MFile::ApplyPath(FileName, m_FileName);
              if (MFile::Exists(FileName) == false) {
                Typo(i, "Cannot parse token \"Beam - radial profile beam\" correctly:"
                     " File not found!");
                return false;
              }
              if (Source->SetPosition(T->GetTokenAtAsDouble(3)*cm,
                                      T->GetTokenAtAsDouble(4)*cm,
                                      T->GetTokenAtAsDouble(5)*cm,
                                      T->GetTokenAtAsDouble(6),
                                      T->GetTokenAtAsDouble(7),
                                      T->GetTokenAtAsDouble(8)) == true &&
                  Source->SetPosition(FileName) == true) {
                mdebug<<"Setting position "<<T->GetTokenAtAsDouble(3)
                      <<"/"<<T->GetTokenAtAsDouble(4)
                      <<"/"<<T->GetTokenAtAsDouble(5)
                      <<"/"<<T->GetTokenAtAsDouble(6)
                      <<"/"<<T->GetTokenAtAsDouble(7)
                      <<"/"<<T->GetTokenAtAsDouble(8)
                      <<" for source "<<Source->GetName()<<endl;
              } else {
                Typo(i, "Cannot parse token \"Beam - radial profile beam\" correctly:"
                     " Content not reasonable");
                return false;
              }
            } else {
              Typo(i, "Cannot parse token \"Beam - radial profile beam\" correctly:"
                   " Number of tokens is not correct (!= 10)!");
              return false;
            }
          }  
          else if (Type == "mapprofilebeam" || Type == "profiledbeam" || Type == "mpb" || Type == "pb") {
            if (T->GetNTokens() == 10) {
              mout<<"Warning: Using the MapProfileBeam without rotation around z-axis is depreciated"<<endl;
              mout<<"         Please update your source file according to the documentation"<<endl;
              

              Source->SetBeamType(MCSource::c_NearField,
                                  MCSource::c_NearFieldBeam2DProfile);
              MString FileName = T->GetTokenAfterAsString(9);
              MFile::ApplyPath(FileName, m_FileName);
              if (MFile::Exists(FileName) == false) {
                Typo(i, "Cannot parse token \"Beam - map profile beam\" correctly:"
                     " File not found!");
                return false;
              }
              if (Source->SetPosition(T->GetTokenAtAsDouble(3)*cm,
                                      T->GetTokenAtAsDouble(4)*cm,
                                      T->GetTokenAtAsDouble(5)*cm,
                                      T->GetTokenAtAsDouble(6),
                                      T->GetTokenAtAsDouble(7),
                                      T->GetTokenAtAsDouble(8),
                                      0.0) == true &&
                  Source->SetPosition(FileName) == true) {
                mdebug<<"Setting position "<<T->GetTokenAtAsDouble(3)
                      <<"/"<<T->GetTokenAtAsDouble(4)
                      <<"/"<<T->GetTokenAtAsDouble(5)
                      <<"/"<<T->GetTokenAtAsDouble(6)
                      <<"/"<<T->GetTokenAtAsDouble(7)
                      <<"/"<<T->GetTokenAtAsDouble(8)
                      <<" for source "<<Source->GetName()<<endl;
              } else {
                Typo(i, "Cannot parse token \"Beam - map profile beam\" correctly:"
                     " Content not reasonable");
                return false;
              }
            } else if (T->GetNTokens() == 11) {
              Source->SetBeamType(MCSource::c_NearField,
                                  MCSource::c_NearFieldBeam2DProfile);
              MString FileName = T->GetTokenAfterAsString(10);
              MFile::ApplyPath(FileName, m_FileName);
              if (MFile::Exists(FileName) == false) {
                Typo(i, "Cannot parse token \"Beam - map profile beam\" correctly:"
                     " File not found!");
                return false;
              }
              if (Source->SetPosition(T->GetTokenAtAsDouble(3)*cm,
                                      T->GetTokenAtAsDouble(4)*cm,
                                      T->GetTokenAtAsDouble(5)*cm,
                                      T->GetTokenAtAsDouble(7),
                                      T->GetTokenAtAsDouble(8),
                                      T->GetTokenAtAsDouble(9),
                                      T->GetTokenAtAsDouble(6)*deg) == true &&
                  Source->SetPosition(FileName) == true) {
                mdebug<<"Setting position "<<T->GetTokenAtAsDouble(3)
                      <<"/"<<T->GetTokenAtAsDouble(4)
                      <<"/"<<T->GetTokenAtAsDouble(5)
                      <<", with normal vector "<<T->GetTokenAtAsDouble(7)
                      <<"/"<<T->GetTokenAtAsDouble(8)
                      <<"/"<<T->GetTokenAtAsDouble(9)
                      <<" and z-axis rotation  "<<T->GetTokenAtAsDouble(6)
                      <<" for source "<<Source->GetName()<<endl;
              } else {
                Typo(i, "Cannot parse token \"Beam - map profile beam\" correctly:"
                     " Content not reasonable");
                return false;
              }
            } else {
              Typo(i, "Cannot parse token \"Beam - map profile beam\" correctly:"
                   " Number of tokens is not correct (!= 10)!");
              return false;
            }
          }  
          else if (Type == "diffractionpointsource" || Type == "diffractionpoint" || Type == "dps") {
            if (T->GetNTokens() == 11) {
              Source->SetBeamType(MCSource::c_NearField,
                                  MCSource::c_NearFieldDiffractionPoint);
              MString FileName = T->GetTokenAfterAsString(10);
              MFile::ApplyPath(FileName, m_FileName);
              if (MFile::Exists(FileName) == false) {
                Typo(i, "Cannot parse token \"Beam - diffraction point source\" correctly:"
                     " File not found!");
                return false;
              }
              if (Source->SetPosition(T->GetTokenAtAsDouble(3)*cm,
                                      T->GetTokenAtAsDouble(4)*cm,
                                      T->GetTokenAtAsDouble(5)*cm,
                                      T->GetTokenAtAsDouble(7),
                                      T->GetTokenAtAsDouble(8),
                                      T->GetTokenAtAsDouble(9),
                                      T->GetTokenAtAsDouble(6)*deg) == true &&
                  Source->SetPosition(FileName) == true) {
                mdebug<<"Setting position "<<T->GetTokenAtAsDouble(3)
                      <<"/"<<T->GetTokenAtAsDouble(4)
                      <<"/"<<T->GetTokenAtAsDouble(5)
                      <<", with normal vector "<<T->GetTokenAtAsDouble(7)
                      <<"/"<<T->GetTokenAtAsDouble(8)
                      <<"/"<<T->GetTokenAtAsDouble(9)
                      <<" and z-axis rotation  "<<T->GetTokenAtAsDouble(6)
                      <<" for source "<<Source->GetName()<<endl;
              } else {
                Typo(i, "Cannot parse token \"Beam - diffraction point source\" correctly:"
                     " Content not reasonable");
                return false;
              }
            } else {
              Typo(i, "Cannot parse token \"Beam - diffraction point source\" correctly:"
                   " Number of tokens is not correct (!= 11)!");
              return false;
            }
          }  
          else if (Type == "diffractionpointsourcekspace" || Type == "diffractionpointkspace" || Type == "dpsk") {
            if (T->GetNTokens() == 11) {
              Source->SetBeamType(MCSource::c_NearField,
                                  MCSource::c_NearFieldDiffractionPointKSpace);
              MString FileName = T->GetTokenAfterAsString(10);
              MFile::ApplyPath(FileName, m_FileName);
              if (MFile::Exists(FileName) == false) {
                Typo(i, "Cannot parse token \"Beam - diffraction point source k space\" correctly:"
                     " File not found!");
                return false;
              }
              if (Source->SetPosition(T->GetTokenAtAsDouble(3)*cm,
                                      T->GetTokenAtAsDouble(4)*cm,
                                      T->GetTokenAtAsDouble(5)*cm,
                                      T->GetTokenAtAsDouble(7),
                                      T->GetTokenAtAsDouble(8),
                                      T->GetTokenAtAsDouble(9),
                                      T->GetTokenAtAsDouble(6)*deg) == true &&
                  Source->SetPosition(FileName) == true) {
                mdebug<<"Setting position "<<T->GetTokenAtAsDouble(3)
                      <<"/"<<T->GetTokenAtAsDouble(4)
                      <<"/"<<T->GetTokenAtAsDouble(5)
                      <<", with normal vector "<<T->GetTokenAtAsDouble(7)
                      <<"/"<<T->GetTokenAtAsDouble(8)
                      <<"/"<<T->GetTokenAtAsDouble(9)
                      <<" and z-axis rotation  "<<T->GetTokenAtAsDouble(6)
                      <<" for source "<<Source->GetName()<<endl;
              } else {
                Typo(i, "Cannot parse token \"Beam - diffraction point source k space\" correctly:"
                     " Content not reasonable");
                return false;
              }
            } else {
              Typo(i, "Cannot parse token \"Beam - diffraction point source k space\" correctly:"
                   " Number of tokens is not correct (!= 11)!");
              return false;
            }
          }  
          else if (Type == "conebeam" || Type == "cb") {
            if (T->GetNTokens() == 10) {
              Source->SetBeamType(MCSource::c_NearField,
                                    MCSource::c_NearFieldConeBeam);
              if (Source->SetPosition(T->GetTokenAtAsDouble(3)*cm,
                                      T->GetTokenAtAsDouble(4)*cm,
                                      T->GetTokenAtAsDouble(5)*cm,
                                      T->GetTokenAtAsDouble(6),
                                      T->GetTokenAtAsDouble(7),
                                      T->GetTokenAtAsDouble(8),
                                      T->GetTokenAtAsDouble(9)*deg) == true) {
                mdebug<<"Setting position "<<T->GetTokenAtAsDouble(3)
                      <<"/"<<T->GetTokenAtAsDouble(4)
                      <<"/"<<T->GetTokenAtAsDouble(5)
                      <<"/"<<T->GetTokenAtAsDouble(6)
                      <<"/"<<T->GetTokenAtAsDouble(7)
                      <<"/"<<T->GetTokenAtAsDouble(8)
                      <<"/"<<T->GetTokenAtAsDouble(9)
                      <<" for source "<<Source->GetName()<<endl;
              } else {
                Typo(i, "Cannot parse token \"Beam - cone beam\" correctly:"
                     " Content not reasonable");
                return false;
              }
            } else {
              Typo(i, "Cannot parse token \"Beam - cone beam\" correctly:"
                   " Number of tokens is not correct (!= 10)!");
              return false;
            }
          }  
          else if (Type == "gaussianconebeam" || Type == "gcb") {
            if (T->GetNTokens() == 11) {
              Source->SetBeamType(MCSource::c_NearField,
                                    MCSource::c_NearFieldConeBeamGauss);
              if (Source->SetPosition(T->GetTokenAtAsDouble(3)*cm,
                                      T->GetTokenAtAsDouble(4)*cm,
                                      T->GetTokenAtAsDouble(5)*cm,
                                      T->GetTokenAtAsDouble(6),
                                      T->GetTokenAtAsDouble(7),
                                      T->GetTokenAtAsDouble(8),
                                      T->GetTokenAtAsDouble(9)*deg,
                                      T->GetTokenAtAsDouble(10)*deg) == true) {
                mdebug<<"Setting position "
                      <<T->GetTokenAtAsDouble(3)
                      <<"/"<<T->GetTokenAtAsDouble(4)
                      <<"/"<<T->GetTokenAtAsDouble(5)
                      <<"/"<<T->GetTokenAtAsDouble(6)
                      <<"/"<<T->GetTokenAtAsDouble(7)
                      <<"/"<<T->GetTokenAtAsDouble(8)
                      <<"/"<<T->GetTokenAtAsDouble(9)
                      <<"/"<<T->GetTokenAtAsDouble(10)
                      <<" for source "<<Source->GetName()<<endl;
              } else {
                Typo(i, "Cannot parse token \"Beam - gaussian cone beam\" correctly:"
                     " Content not reasonable");
                return false;
              }
            } else {
              Typo(i, "Cannot parse token \"Beam - gaussian cone beam\" correctly:"
                   " Number of tokens is not correct (!= 11)!");
              return false;
            }
          }  
          else if (Type == "fanbeam" || Type == "fb") {
            if (T->GetNTokens() == 13) {
              Source->SetBeamType(MCSource::c_NearField,
                                  MCSource::c_NearFieldFanBeam);
              // Pos, Dir1, Dir2, Radius
              if (Source->SetPosition(T->GetTokenAtAsDouble(3)*cm,
                                      T->GetTokenAtAsDouble(4)*cm,
                                      T->GetTokenAtAsDouble(5)*cm,
                                      T->GetTokenAtAsDouble(6),
                                      T->GetTokenAtAsDouble(7),
                                      T->GetTokenAtAsDouble(8),
                                      T->GetTokenAtAsDouble(9),
                                      T->GetTokenAtAsDouble(10),
                                      T->GetTokenAtAsDouble(11),
                                      T->GetTokenAtAsDouble(12)*cm) == true) {
                mdebug<<"Setting position (pos, dir1, dir2, radius) "
                      <<T->GetTokenAtAsDouble(3)
                      <<"/"<<T->GetTokenAtAsDouble(4)
                      <<"/"<<T->GetTokenAtAsDouble(5)
                      <<"/"<<T->GetTokenAtAsDouble(6)
                      <<"/"<<T->GetTokenAtAsDouble(7)
                      <<"/"<<T->GetTokenAtAsDouble(8)
                      <<"/"<<T->GetTokenAtAsDouble(9)
                      <<"/"<<T->GetTokenAtAsDouble(10)
                      <<"/"<<T->GetTokenAtAsDouble(11)
                      <<"/"<<T->GetTokenAtAsDouble(12)
                      <<" for source "<<Source->GetName()<<endl;
              } else {
                Typo(i, "Cannot parse token \"Beam - fan beam\" correctly:"
                     " Content not reasonable");
                return false;
              }
            } else {
              Typo(i, "Cannot parse token \"Beam - fan beam\" correctly:"
                   " Number of tokens is not correct (!= 13)!");
              return false;
            }
          }  
          else if (Type == "illuminateddisk" || Type == "id") {
            if (T->GetNTokens() == 11) {
              Source->SetBeamType(MCSource::c_NearField,
                                    MCSource::c_NearFieldIlluminatedDisk);
              if (Source->SetPosition(T->GetTokenAtAsDouble(3)*cm,
                                      T->GetTokenAtAsDouble(4)*cm,
                                      T->GetTokenAtAsDouble(5)*cm,
                                      T->GetTokenAtAsDouble(6)*cm,
                                      T->GetTokenAtAsDouble(7)*deg,
                                      T->GetTokenAtAsDouble(8)*deg,
                                      T->GetTokenAtAsDouble(9)*deg,
                                      T->GetTokenAtAsDouble(10)*deg) == true) {
                mdebug<<"Setting position "<<T->GetTokenAtAsDouble(3)
                      <<"/"<<T->GetTokenAtAsDouble(4)
                      <<"/"<<T->GetTokenAtAsDouble(5)
                      <<"/"<<T->GetTokenAtAsDouble(6)
                      <<"/"<<T->GetTokenAtAsDouble(7)
                      <<"/"<<T->GetTokenAtAsDouble(8)
                      <<"/"<<T->GetTokenAtAsDouble(9)
                      <<"/"<<T->GetTokenAtAsDouble(10)
                      <<" for source "<<Source->GetName()<<endl;
              } else {
                Typo(i, "Cannot parse token \"Beam - illuminated disk\" correctly:"
                     " Content not reasonable");
                return false;
              }
            } else {
              Typo(i, "Cannot parse token \"Beam - illuminated disk\" correctly:"
                   " Number of tokens is not correct (!= 11)!");
              return false;
            }
          }  
          else if (Type == "illuminatedbox" || Type == "ib") {
            if (T->GetNTokens() == 11) {
              Source->SetBeamType(MCSource::c_NearField,
                                  MCSource::c_NearFieldIlluminatedSquare);
              if (Source->SetPosition(T->GetTokenAtAsDouble(3)*cm,
                                      T->GetTokenAtAsDouble(4)*cm,
                                      T->GetTokenAtAsDouble(5)*cm,
                                      T->GetTokenAtAsDouble(6)*cm,
                                      T->GetTokenAtAsDouble(7)*deg,
                                      T->GetTokenAtAsDouble(8)*deg,
                                      T->GetTokenAtAsDouble(9)*deg,
                                      T->GetTokenAtAsDouble(10)*deg) == true) {
                mdebug<<"Setting position "<<T->GetTokenAtAsDouble(3)
                      <<"/"<<T->GetTokenAtAsDouble(4)
                      <<"/"<<T->GetTokenAtAsDouble(5)
                      <<"/"<<T->GetTokenAtAsDouble(6)
                      <<"/"<<T->GetTokenAtAsDouble(7)
                      <<"/"<<T->GetTokenAtAsDouble(8)
                      <<"/"<<T->GetTokenAtAsDouble(9)
                      <<"/"<<T->GetTokenAtAsDouble(10)
                      <<" for source "<<Source->GetName()<<endl;
              } else {
                Typo(i, "Cannot parse token \"Beam - illuminated box\" correctly:"
                     " Content not reasonable");
                return false;
              }
            } else {
              Typo(i, "Cannot parse token \"Beam - illuminated box\" correctly:"
                   " Number of tokens is not correct (!= 11)!");
              return false;
            }
          }  
          else if (Type == "flatmap") {
            if (T->GetNTokens() == 11) {
              Source->SetBeamType(MCSource::c_NearField,
                                  MCSource::c_NearFieldFlatMap);
              MString FileName = T->GetTokenAfterAsString(10);
              MFile::ApplyPath(FileName, m_FileName);
              if (MFile::Exists(FileName) == false) {
                Typo(i, "Cannot parse token \"flat map\" correctly:"
                     " File not found!");
                return false;
              }
              if (Source->SetPosition(T->GetTokenAtAsDouble(3)*cm,
                                      T->GetTokenAtAsDouble(4)*cm,
                                      T->GetTokenAtAsDouble(5)*cm,
                                      T->GetTokenAtAsDouble(7),
                                      T->GetTokenAtAsDouble(8),
                                      T->GetTokenAtAsDouble(9),
                                      T->GetTokenAtAsDouble(6)*deg) == true &&
                  Source->SetPosition(FileName) == true) {
                mdebug<<"Setting position "<<T->GetTokenAtAsDouble(3)
                      <<"/"<<T->GetTokenAtAsDouble(4)
                      <<"/"<<T->GetTokenAtAsDouble(5)
                      <<", with normal vector "<<T->GetTokenAtAsDouble(7)
                      <<"/"<<T->GetTokenAtAsDouble(8)
                      <<"/"<<T->GetTokenAtAsDouble(9)
                      <<" and z-axis rotation  "<<T->GetTokenAtAsDouble(6)
                      <<" for source "<<Source->GetName()<<endl;
              } else {
                Typo(i, "Cannot parse token \"flat map\" correctly:"
                     " Content not reasonable");
                return false;
              }
            } else {
              Typo(i, "Cannot parse token \"flat map\" correctly:"
                   " Number of tokens is not correct (!= 11)!");
              return false;
            }
          }  
          else if (Type == "activation" || Type == "act") {
            /// Only internally used!
          }
          else if (Type == "reversedirectiontopredecessor") {
            Source->SetBeamType(MCSource::c_NearField,
                                MCSource::c_NearFieldReverseDirectionToPredecessor);
          }
          else if (Type == "volume" || Type == "vol") {
            if (T->GetNTokens() == 4) {
              Source->SetBeamType(MCSource::c_NearField,
                                  MCSource::c_NearFieldVolume);
              if (Source->SetVolume(T->GetTokenAtAsString(3)) == true) {
                mdebug<<"Setting start volume "<<T->GetTokenAtAsString(3)
                      <<" for source "<<Source->GetName()<<endl;
              } else {
                Typo(i, "Cannot parse token \"Beam - volume\" correctly:"
                     " Content not reasonable");
                return false;
              }
            } else {
              Typo(i, "Cannot parse token \"Beam - volume\" correctly:"
                   " Number of tokens is not correct (!= 4)!");
              return false;
            }
          }  
          else {
            Typo(i, MString("Cannot parse token \"Beam\" correctly: Unknown beam type: ") + Type);
            return false;
          }
        } else {
          Typo(i, "Cannot parse token \"Beam\" correctly:"
               " Number of tokens must be larger or equal 3!");
          return false;
        }
      }
      
      
      else if (T->IsTokenAt(1, "ParticleType", true) == true) {
        if (T->GetNTokens() == 3) {
          if (Source->SetParticleType(T->GetTokenAtAsInt(2)) == true) {
            mdebug<<"Setting particle type "<<Source->GetParticleType()
                  <<" for source "<<Source->GetName()<<endl;
          } else {
            Typo(i, "Cannot parse token ParticleTyp correctly:"
                 " ?? Perhaps the value is non-positive ??");
            return false;
          }
        } else {
          Typo(i, "Cannot parse token ParticleTyp correctly:"
               " Number of tokens is not correct!");
          return false;
        }
      } 
      
      
      else if (T->IsTokenAt(1, "Polarization", true) == true) {
        if (T->GetNTokens() >= 3) {
          MString Type = T->GetTokenAtAsString(2);
          Type.ToLower();
          if (Type == "true" || Type == "false" || Type == "1" || Type == "0") {
            mout<<"Depreciated: Polarization"<<endl
              <<"  You are using the old polarization format. Please update to the new format."<<endl
              <<"  Consult the manual for further information."<<show;
            if (Type == "true" || Type == "1") {
              if (T->GetNTokens() == 7) {
                Source->SetPolarizationType(MCSource::c_PolarizationAbsolute);
                Source->SetPolarizationDegree(T->GetTokenAtAsDouble(6));
                Source->SetPolarization(T->GetTokenAtAsDouble(3),
                                        T->GetTokenAtAsDouble(4),
                                        T->GetTokenAtAsDouble(5));
              } else {
                Typo(i, "Cannot parse token \"Polarization true\" correctly: Number of tokens is too small!");
                return false;
              }
            } else {
              Typo(i, "Relative polarization is only implemented in the new format!");
              return false;
            }
          } else if (Type == "no" || Type == "none") {
            Source->SetPolarizationType(MCSource::c_PolarizationNone);
          } else if (Type == "random") {
            Source->SetPolarizationType(MCSource::c_PolarizationRandom);
          } else if (Type == "absolute") {
            if (T->GetNTokens() == 7) {
              Source->SetPolarizationType(MCSource::c_PolarizationAbsolute);
              Source->SetPolarizationDegree(T->GetTokenAtAsDouble(3));
              Source->SetPolarization(T->GetTokenAtAsDouble(4),
                                      T->GetTokenAtAsDouble(5),
                                      T->GetTokenAtAsDouble(6));
            } else {
              Typo(i, "Cannot parse token \"Polarization absolute\" correctly: Number of tokens is not correct!");
              return false;
            }
          } else if (Type == "relativex") {
            if (T->GetNTokens() == 5) {
              Source->SetPolarizationType(MCSource::c_PolarizationRelativeX);
              Source->SetPolarizationDegree(T->GetTokenAtAsDouble(3));
              Source->SetPolarization(T->GetTokenAtAsDouble(4)*deg);
            } else {
              Typo(i, "Cannot parse token \"Polarization RelativeX\" correctly: Number of tokens is not correct!");
              return false;
            }
          } else if (Type == "relativey") {
            if (T->GetNTokens() == 5) {
              Source->SetPolarizationType(MCSource::c_PolarizationRelativeY);
              Source->SetPolarizationDegree(T->GetTokenAtAsDouble(3));
              Source->SetPolarization(T->GetTokenAtAsDouble(4)*deg);
            } else {
              Typo(i, "Cannot parse token \"Polarization RelativeY\" correctly: Number of tokens is not correct!");
              return false;
            }
          } else if (Type == "relativez") {
            if (T->GetNTokens() == 5) {
              Source->SetPolarizationType(MCSource::c_PolarizationRelativeZ);
              Source->SetPolarizationDegree(T->GetTokenAtAsDouble(3));
              Source->SetPolarization(T->GetTokenAtAsDouble(4)*deg);
            } else {
              Typo(i, "Cannot parse token \"Polarization RelativeZ\" correctly: Number of tokens is not correct!");
              return false;
            }
          } else {
            Typo(i, "Unknown polarization type!");
            return false;
          }
        } else {
          Typo(i, "Cannot parse token Polarization correctly: Number of tokens is too small!");
          return false;
        }
      } 
      
      
      else if (T->IsTokenAt(1, "Successor", true) == true) {
        if (T->GetNTokens() == 3) {
          if (Source->SetSuccessor(T->GetTokenAtAsString(2)) == true) {
            if (GetSource(T->GetTokenAtAsString(2)) != 0) {
              mdebug<<"Setting successor "<<Source->GetSuccessor()
                    <<" for source "<<Source->GetName()<<endl;
            } else {
              Typo(i, "Successor does not exist!");
              return false;
            }
          } else {
            Typo(i, "Cannot parse token Successor correctly");
            return false;
          }
        } else {
          Typo(i, "Cannot parse token Successor correctly:"
               " Number of tokens is not correct!");
          return false;
        }
      } 
      
      
      else if (T->IsTokenAt(1, "IsSuccessor", true) == true) {
        if (T->GetNTokens() == 3) {
          if (Source->SetIsSuccessor(T->GetTokenAtAsBoolean(2)) == true) {
            mdebug<<"Setting is successor "<<Source->IsSuccessor()
                  <<" for source "<<Source->GetName()<<endl;
          } else {
            Typo(i, "Cannot parse token IsSuccessor correctly");
            return false;
          }
        } else {
          Typo(i, "Cannot parse token IsSuccessor correctly:"
               " Number of tokens is not correct!");
          return false;
        }
      } 
      
      
      else if (T->IsTokenAt(1, "LightCurve", true) == true) {
        if (T->GetNTokens() > 3) {
          MString Type = T->GetTokenAtAsString(2);
          Type.ToLower();
          if (Type == "file") {
            if (T->GetNTokens() >= 5) {
              MString FileName = T->GetTokenAfterAsString(4);
              MFile::ExpandFileName(FileName);
              if (MFile::FileExists(FileName) == false) {
                Typo(i, "File does not exist!");
                return false;          
              }
              Source->SetLightCurveType(MCSource::c_LightCurveFile);
              if (Source->SetLightCurve(FileName, 
                                        T->GetTokenAtAsBoolean(3)) == true) {
                mdebug<<"Setting LightCurve for source "<<Source->GetName()<<endl;
              } else {
                Typo(i, "Cannot parse token LightCurve correctly. Setting failed.");
                return false;
              }
            } else {
              Typo(i, "Cannot parse token LightCurve - file correctly: Number of tokens is not correct!");
              return false;
            }
          } else if (Type == "flat" || Type == "none") {
            Source->SetLightCurveType(MCSource::c_LightCurveFlat);
          } else {
            Typo(i, "Unknown light curve type!");
            cout<<"Type: "<<Type<<endl;
            return false;
          }
        } else {
          Typo(i, "Cannot parse token LightCurve - file correctly: Number of tokens is not correct!");
          return false;
        }
      } 
      
      else if (T->IsTokenAt(1, "Orientation", true) == true) {
        if (T->GetNTokens() >= 4) {
          MCOrientation O;
          if (O.Parse(*T) == false) {
            Typo(i, "Cannot parse token \"Orientation\" correctly");
            return false;
          }
          Source->SetOrientation(O);
        } else {
          Typo(i, "Cannot parse token \"Orientation\" correctly: Number of tokens is not correct!");
          return false;
        }
      }
      
      else if (T->IsTokenAt(1, "TotalEnergyFlux", true) == true) {
        if (T->GetNTokens() == 3) {
          if (Source->SetTotalEnergyFlux(T->GetTokenAtAsDouble(2)*keV/cm/cm) == true) {
            mdebug<<"Setting total energy flux "<<T->GetTokenAtAsDouble(2)
                  <<" MeV/cm2 for source "<<Source->GetName()<<endl;
          } else {
            Typo(i, "Cannot parse token TotalEnergyFlux correctly:"
                 " ?? Perhaps the value is non-positive ??");
            return false;
          }
        } else {
          Typo(i, "Cannot parse token TotalEnergyFlux correctly:"
               " Number of tokens is not correct!");
          return false;
        }
      } else if (T->IsTokenAt(1, "Position", true) == true) {
      } else if (T->IsTokenAt(1, "Energy", true) == true) {
      } else if (T->IsTokenAt(1, "Intensity", true) == true) {
      } else if (T->IsTokenAt(1, "Flux", true) == true) {
      } else if (T->IsTokenAt(1, "EventList", true) == true) {
      } else {      
        Typo(i, MString("Unknown keyword: ") + T->GetTokenAt(1));
        return false;
      }
    } // is source
  } // Step 4

  // --> begin depreciated
  // Step 5: Secondary Source parameters
  for (i = 0; i < GetNLines(); ++i) {
    MTokenizer* T = GetTokenizerAt(i);

    if (T->GetNTokens() == 0) continue;
    if ((Source = GetSource(T->GetTokenAt(0))) != 0) {
      // Position:
      if (T->IsTokenAt(1, "Position", true) == true) {
        mout<<"Depreciated: Position"<<endl
            <<"  The keywords \"SpectralType\", \"Energy\", and \"Position\" are depreciated!"<<endl
            <<"  Please use \"Spectrum\" and \"Beam\" instead."<<endl
            <<"  Consult the manual for further information."<<show;

        // Sphere:
        if (T->GetNTokens() < 5) {
          Typo(i, "Position has not enough parameters");
        }

        if (T->GetTokenAtAsInt(2) == MCSource::c_FarField) {
          if (T->GetTokenAtAsInt(3) == 1) {
            if (T->GetNTokens() == 6) {
              Source->SetBeamType(MCSource::c_FarField,
                                    MCSource::c_FarFieldPoint);
              if (Source->SetPosition(T->GetTokenAtAsDouble(4)*deg,
                                      T->GetTokenAtAsDouble(5)*deg) == true) {
                mdebug<<"Setting position "<<T->GetTokenAtAsDouble(4)
                      <<"/"<<T->GetTokenAtAsDouble(5)
                      <<" for source "<<Source->GetName()<<endl;
              } else {
                Typo(i, "Cannot parse token Position correctly:"
                     " Content not reasonable");
                return false;
              }
            } else {
              Typo(i, "Cannot parse token Position correctly:"
                   " Number of tokens is not correct!");
              return false;
            }
          } else if (T->GetTokenAtAsInt(3) == 2) {
            if (T->GetNTokens() == 8) {
              Source->SetBeamType(MCSource::c_FarField,
                                    MCSource::c_FarFieldArea);
              if (Source->SetPosition(T->GetTokenAtAsDouble(4)*deg,
                                      T->GetTokenAtAsDouble(5)*deg,
                                      T->GetTokenAtAsDouble(6)*deg,
                                      T->GetTokenAtAsDouble(7)*deg) == true) {
                mdebug<<"Setting position "<<T->GetTokenAtAsDouble(4)
                      <<"/"<<T->GetTokenAtAsDouble(5)
                      <<"/"<<T->GetTokenAtAsDouble(6)
                      <<"/"<<T->GetTokenAtAsDouble(7)
                      <<" for source "<<Source->GetName()<<endl;
              } else {
                Typo(i, "Cannot parse token Position correctly:"
                     " Content not reasonable");
                return false;
              }
            } else {
              Typo(i, "Cannot parse token Position correctly:"
                   " Number of tokens is not correct (!= 8)!");
              return false;
            }
          } else {
            Typo(i, "Combination CoordinateSystem and BeamType"
                 " is not yet implemented!");
            return false;
          }
        } else if (T->GetTokenAtAsInt(2) == MCSource::c_NearField) {
          if (T->GetTokenAtAsInt(3) == 1) {
            if (T->GetNTokens() == 7) {
              Source->SetBeamType(MCSource::c_NearField,
                                    MCSource::c_NearFieldPoint);
              if (Source->SetPosition(T->GetTokenAtAsDouble(4)*cm,
                                      T->GetTokenAtAsDouble(5)*cm,
                                      T->GetTokenAtAsDouble(6)*cm) == true) {
                mdebug<<"Setting position "<<T->GetTokenAtAsDouble(4)
                      <<"/"<<T->GetTokenAtAsDouble(5)
                      <<"/"<<T->GetTokenAtAsDouble(6)
                      <<" for source "<<Source->GetName()<<endl;
              } else {
                Typo(i, "Cannot parse token Position correctly:"
                     " Content not reasonable");
                return false;
              }
            } else {
              Typo(i, "Cannot parse token Position correctly:"
                   " Number of tokens is not correct!");
              return false;
            }
          } else if (T->GetTokenAtAsInt(3) == 2) {
            if (T->GetNTokens() == 10) {
              Source->SetBeamType(MCSource::c_NearField,
                                    MCSource::c_NearFieldLine);
              if (Source->SetPosition(T->GetTokenAtAsDouble(4)*cm,
                                      T->GetTokenAtAsDouble(5)*cm,
                                      T->GetTokenAtAsDouble(6)*cm,
                                      T->GetTokenAtAsDouble(7)*cm,
                                      T->GetTokenAtAsDouble(8)*cm,
                                      T->GetTokenAtAsDouble(9)*cm) == true) {
                mdebug<<"Setting position "<<T->GetTokenAtAsDouble(4)
                      <<"/"<<T->GetTokenAtAsDouble(5)
                      <<"/"<<T->GetTokenAtAsDouble(6)
                      <<"/"<<T->GetTokenAtAsDouble(7)
                      <<"/"<<T->GetTokenAtAsDouble(8)
                      <<"/"<<T->GetTokenAtAsDouble(9)
                      <<" for source "<<Source->GetName()<<endl;
              } else {
                Typo(i, "Cannot parse token Position correctly:"
                     " Content not reasonable");
                return false;
              }
            } else {
              Typo(i, "Cannot parse token Position correctly:"
                   " Number of tokens is not correct!");
              return false;
            }
          } else if (T->GetTokenAtAsInt(3) == 3) {
            if (T->GetNTokens() == 10) {
              Source->SetBeamType(MCSource::c_NearField,
                                    MCSource::c_NearFieldBox);
              if (Source->SetPosition(T->GetTokenAtAsDouble(4)*cm,
                                      T->GetTokenAtAsDouble(5)*cm,
                                      T->GetTokenAtAsDouble(6)*cm,
                                      T->GetTokenAtAsDouble(7)*cm,
                                      T->GetTokenAtAsDouble(8)*cm,
                                      T->GetTokenAtAsDouble(9)*cm) == true) {
                mdebug<<"Setting position "<<T->GetTokenAtAsDouble(4)
                      <<"/"<<T->GetTokenAtAsDouble(5)
                      <<"/"<<T->GetTokenAtAsDouble(6)
                      <<"/"<<T->GetTokenAtAsDouble(7)
                      <<"/"<<T->GetTokenAtAsDouble(8)
                      <<"/"<<T->GetTokenAtAsDouble(9)
                      <<" for source "<<Source->GetName()<<endl;
              } else {
                Typo(i, "Cannot parse token Position correctly:"
                     " Content not reasonable");
                return false;
              }
            } else {
              Typo(i, "Cannot parse token Position correctly:"
                   " Number of tokens is not correct!");
              return false;
            }
          } 
          // Sphere
          else if (T->GetTokenAtAsInt(3) == 4) {
            if (T->GetNTokens() == 10) {
              Source->SetBeamType(MCSource::c_NearField,
                                    MCSource::c_NearFieldSphere);
              if (Source->SetPosition(T->GetTokenAtAsDouble(4)*cm,
                                      T->GetTokenAtAsDouble(5)*cm,
                                      T->GetTokenAtAsDouble(6)*cm,
                                      T->GetTokenAtAsDouble(7)*cm,
                                      T->GetTokenAtAsDouble(8)*cm,
                                      T->GetTokenAtAsDouble(9)*cm) == true) {
                mdebug<<"Setting position "<<T->GetTokenAtAsDouble(4)
                      <<"/"<<T->GetTokenAtAsDouble(5)
                      <<"/"<<T->GetTokenAtAsDouble(6)
                      <<" with radii "<<T->GetTokenAtAsDouble(7)
                      <<"/"<<T->GetTokenAtAsDouble(8)
                      <<"/"<<T->GetTokenAtAsDouble(9)
                      <<" for source "<<Source->GetName()<<endl;
              } else {
                Typo(i, "Cannot parse token Position correctly:"
                     " Content not reasonable");
                return false;
              }
            } else {
              Typo(i, "Cannot parse token Position correctly:"
                   " Number of tokens is not correct!");
              return false;
            }
          } 
          // Point conic
          else if (T->GetTokenAtAsInt(3) == 6) {
            if (T->GetNTokens() == 7) {
              Source->SetBeamType(MCSource::c_NearField,
                                    MCSource::c_NearFieldRestrictedPoint);
              if (Source->SetPosition(T->GetTokenAtAsDouble(4)*cm,
                                      T->GetTokenAtAsDouble(5)*cm,
                                      T->GetTokenAtAsDouble(6)*cm) == true) {
                mdebug<<"Setting position "<<T->GetTokenAtAsDouble(4)
                      <<"/"<<T->GetTokenAtAsDouble(5)
                      <<"/"<<T->GetTokenAtAsDouble(6)
                      <<" for source "<<Source->GetName()<<endl;
              } else {
                Typo(i, "Cannot parse token Position correctly:"
                     " Content not reasonable");
                return false;
              }
            } else {
              Typo(i, "Cannot parse token Position correctly:"
                   " Number of tokens is not correct!");
              return false;
            }
          } 
          // Beam
          else if (T->GetTokenAtAsInt(3) == 5) {
            if (T->GetNTokens() == 11) {
              Source->SetBeamType(MCSource::c_NearField,
                                    MCSource::c_NearFieldBeam);
              if (Source->SetPosition(T->GetTokenAtAsDouble(4)*cm,
                                      T->GetTokenAtAsDouble(5)*cm,
                                      T->GetTokenAtAsDouble(6)*cm,
                                      T->GetTokenAtAsDouble(7),
                                      T->GetTokenAtAsDouble(8),
                                      T->GetTokenAtAsDouble(9),
                                      T->GetTokenAtAsDouble(10)*cm) == true) {
                mdebug<<"Setting position "<<T->GetTokenAtAsDouble(4)
                      <<"/"<<T->GetTokenAtAsDouble(5)
                      <<"/"<<T->GetTokenAtAsDouble(6)
                      <<"/"<<T->GetTokenAtAsDouble(7)
                      <<"/"<<T->GetTokenAtAsDouble(8)
                      <<"/"<<T->GetTokenAtAsDouble(9)
                      <<"/"<<T->GetTokenAtAsDouble(10)
                      <<" for source "<<Source->GetName()<<endl;
              } else {
                Typo(i, "Cannot parse token Position correctly:"
                     " Content not reasonable");
                return false;
              }
            } else {
              Typo(i, "Cannot parse token Position correctly:"
                   " Number of tokens is not correct!");
              return false;
            }
          }
          // Beam with 2D profile
          else if (T->GetTokenAtAsInt(3) == 8) {
            if (T->GetNTokens() == 11) {
              Source->SetBeamType(MCSource::c_NearField,
                                    MCSource::c_NearFieldBeam2DProfile);
              if (Source->SetPosition(T->GetTokenAtAsDouble(4)*cm,
                                      T->GetTokenAtAsDouble(5)*cm,
                                      T->GetTokenAtAsDouble(6)*cm,
                                      T->GetTokenAtAsDouble(7),
                                      T->GetTokenAtAsDouble(8),
                                      T->GetTokenAtAsDouble(9)) == true &&
                  Source->SetPosition(T->GetTokenAfterAsString(10)) == true) {
                mdebug<<"Setting position "<<T->GetTokenAtAsDouble(4)
                      <<"/"<<T->GetTokenAtAsDouble(5)
                      <<"/"<<T->GetTokenAtAsDouble(6)
                      <<"/"<<T->GetTokenAtAsDouble(7)
                      <<"/"<<T->GetTokenAtAsDouble(8)
                      <<"/"<<T->GetTokenAtAsDouble(9)
                      <<" for source "<<Source->GetName()<<endl;
              } else {
                Typo(i, "Cannot parse token Position correctly:"
                     " Content not reasonable");
                return false;
              }
            } else {
              Typo(i, "Cannot parse token Position correctly:"
                   " Number of tokens is not correct!");
              return false;
            }

          }
          // divergent beam (homogeneous intensity) from point source
          else if (T->GetTokenAtAsInt(3) == 9) {
            if (T->GetNTokens() == 11) {
              Source->SetBeamType(MCSource::c_NearField,
                                    MCSource::c_NearFieldConeBeam);
              if (Source->SetPosition(T->GetTokenAtAsDouble(4)*cm,
                                      T->GetTokenAtAsDouble(5)*cm,
                                      T->GetTokenAtAsDouble(6)*cm,
                                      T->GetTokenAtAsDouble(7),
                                      T->GetTokenAtAsDouble(8),
                                      T->GetTokenAtAsDouble(9),
                                      T->GetTokenAtAsDouble(10)*deg) == true) {
                mdebug<<"Setting position "<<T->GetTokenAtAsDouble(4)
                      <<"/"<<T->GetTokenAtAsDouble(5)
                      <<"/"<<T->GetTokenAtAsDouble(6)
                      <<"/"<<T->GetTokenAtAsDouble(7)
                      <<"/"<<T->GetTokenAtAsDouble(8)
                      <<"/"<<T->GetTokenAtAsDouble(9)
                      <<"/"<<T->GetTokenAtAsDouble(10)
                      <<" for source "<<Source->GetName()<<endl;
              } else {
                Typo(i, "Cannot parse token Position correctly:"
                     " Content not reasonable");
                return false;
              }
            } else {
              Typo(i, "Cannot parse token Position correctly:"
                   " Number of tokens is not correct!");
              return false;
            }
          }
          // divergent beam (Gaussian intensity profile) from point source
          else if (T->GetTokenAtAsInt(3) == 10) {
            if (T->GetNTokens() == 12) {
              Source->SetBeamType(MCSource::c_NearField,
                                    MCSource::c_NearFieldConeBeamGauss);
              if (Source->SetPosition(T->GetTokenAtAsDouble(4)*cm,
                                      T->GetTokenAtAsDouble(5)*cm,
                                      T->GetTokenAtAsDouble(6)*cm,
                                      T->GetTokenAtAsDouble(7),
                                      T->GetTokenAtAsDouble(8),
                                      T->GetTokenAtAsDouble(9),
                                      T->GetTokenAtAsDouble(10)*deg,
                                      T->GetTokenAtAsDouble(11)*deg) == true) {
                mdebug<<"Setting position "<<T->GetTokenAtAsDouble(4)
                      <<"/"<<T->GetTokenAtAsDouble(5)
                      <<"/"<<T->GetTokenAtAsDouble(6)
                      <<"/"<<T->GetTokenAtAsDouble(7)
                      <<"/"<<T->GetTokenAtAsDouble(8)
                      <<"/"<<T->GetTokenAtAsDouble(9)
                      <<"/"<<T->GetTokenAtAsDouble(10)
                      <<" for source "<<Source->GetName()<<endl;
              } else {
                Typo(i, "Cannot parse token Position correctly:"
                     " Content not reasonable");
                return false;
              }
            } else {
              Typo(i, "Cannot parse token Position correctly:"
                   " Number of tokens is not correct!");
              return false;
            }

          } else {
            Typo(i, "Combination CoordinateSystem and BeamType is"
                 " not yet implemented!");
            return false;
          }
        } // region type
      } 
      //
      else if (T->IsTokenAt(1, "Energy", true) == true) {
        mout<<"Depreciated: Energy"<<endl
            <<"  The keywords \"SpectralType\", \"Energy\", and \"Position\" are depreciated!"<<endl
            <<"  Please use \"Spectrum\" and \"Beam\" instead."<<endl
            <<"  Consult the manual for further information."<<show;

        if (Source->GetSpectralType() == MCSource::c_Monoenergetic) {
          if (T->GetNTokens() == 3) {
            if (Source->SetEnergy(T->GetTokenAtAsDouble(2)*keV) == true) {
              mdebug<<"Setting energy "<<T->GetTokenAtAsDouble(2)*keV
                    <<"keV for source "<<Source->GetName()<<endl;
            } else {
              Typo(i, "Cannot parse token Energy correctly:"
                   " Content not reasonable");
              return false;
            }
          } else {
            Typo(i, "Cannot parse token Energy correctly:"
                 " Number of tokens is not correct!");
            return false;
          }
        } else if (Source->GetSpectralType() == MCSource::c_Linear) {
          if (T->GetNTokens() == 4) {
            if (Source->SetEnergy(T->GetTokenAtAsDouble(2)*keV, 
                                  T->GetTokenAtAsDouble(3)*keV) == true) {
              mdebug<<"Setting energy min ="<<T->GetTokenAtAsDouble(2)
                    <<" keV and max = "<<T->GetTokenAtAsDouble(3)
                    <<"keV for source "<<Source->GetName()<<endl;
            } else {
              Typo(i, "Cannot parse token Energy correctly:"
                   " Content not reasonable");
              return false;
            }
          } else {
            Typo(i, "Cannot parse token Energy correctly:"
                 " Number of tokens is not correct!");
            return false;
          }
        } else if (Source->GetSpectralType() == MCSource::c_PowerLaw) {
          if (T->GetNTokens() == 5) {
            if (Source->SetEnergy(T->GetTokenAtAsDouble(2)*keV, 
                                  T->GetTokenAtAsDouble(3)*keV,
                                  T->GetTokenAtAsDouble(4)) == true) {
              mdebug<<"Setting energy min ="<<T->GetTokenAtAsDouble(2)
                    <<" keV and max = "<<T->GetTokenAtAsDouble(3)
                    <<" keV and alpha = "<<T->GetTokenAtAsDouble(4)
                    <<"keV for source "<<Source->GetName()<<endl;
            } else {
              Typo(i, "Cannot parse token Energy - power law correctly:"
                   " Content not reasonable");
              return false;
            }
          } else {
            Typo(i, "Cannot parse token Energy - power law correctly:"
                 " Number of tokens is not correct!");
            return false;
          }
        } else if (Source->GetSpectralType() == MCSource::c_BrokenPowerLaw) {
          if (T->GetNTokens() == 7) {
            if (Source->SetEnergy(T->GetTokenAtAsDouble(2)*keV, 
                                  T->GetTokenAtAsDouble(3)*keV,
                                  T->GetTokenAtAsDouble(4)*keV,
                                  T->GetTokenAtAsDouble(5),
                                  T->GetTokenAtAsDouble(6)) == true) {
              mdebug<<"Setting energy min = "<<T->GetTokenAtAsDouble(2)
                    <<" keV and max = "<<T->GetTokenAtAsDouble(3)
                    <<" keV and break = "<<T->GetTokenAtAsDouble(4)
                    <<" keV and alpha low = "<<T->GetTokenAtAsDouble(5)
                    <<" and  alpha high = "<<T->GetTokenAtAsDouble(6)
                    <<"for source "<<Source->GetName()<<endl;
            } else {
              Typo(i, "Cannot parse token Energy - broken power law correctly:"
                   " Content not reasonable");
              return false;
            }
          } else {
            Typo(i, "Cannot parse token Energy - broken power law correctly:"
                 " Number of tokens is not correct!");
            return false;
          }
        } else if (Source->GetSpectralType() == MCSource::c_Gaussian) {
          if (T->GetNTokens() == 5) {
            if (Source->SetEnergy(T->GetTokenAtAsDouble(2)*keV, 
                                  T->GetTokenAtAsDouble(3)*keV,
                                  T->GetTokenAtAsDouble(4)*keV) == true) {
              mdebug<<"Setting avg = "<<T->GetTokenAtAsDouble(2)
                    <<" sigma = "<<T->GetTokenAtAsDouble(3)
                    <<" sigma cut-off = "<<T->GetTokenAtAsDouble(4)
                    <<"for source "<<Source->GetName()<<endl;
            } else {
              Typo(i, "Cannot parse token Energy - Gaussian correctly:"
                   " Content not reasonable");
              return false;
            }
          } else {
            Typo(i, "Cannot parse token Energy - Gaussian correctly:"
                 " Number of tokens is not correct!");
            return false;
          }
        } else if (Source->GetSpectralType() == MCSource::c_ThermalBremsstrahlung) {
          if (T->GetNTokens() == 5) {
            if (Source->SetEnergy(T->GetTokenAtAsDouble(2)*keV, 
                                  T->GetTokenAtAsDouble(3)*keV,
                                  T->GetTokenAtAsDouble(4)*keV) == true) {
              mdebug<<"Setting min = "<<T->GetTokenAtAsDouble(2)
                    <<" max = "<<T->GetTokenAtAsDouble(3)
                    <<" temperature = "<<T->GetTokenAtAsDouble(4)
                    <<"for source "<<Source->GetName()<<endl;
            } else {
              Typo(i, "Cannot parse token Energy - thermal bremsstrahlung correctly:"
                   " Content not reasonable");
              return false;
            }
          } else {
            Typo(i, "Cannot parse token Energy - thermal bremsstrahlung correctly:"
                 " Number of tokens is not correct!");
            return false;
          }
        } else if (Source->GetSpectralType() == MCSource::c_BlackBody) {
          if (T->GetNTokens() == 5) {
            if (Source->SetEnergy(T->GetTokenAtAsDouble(2)*keV, 
                                  T->GetTokenAtAsDouble(3)*keV,
                                  T->GetTokenAtAsDouble(4)*keV) == true) {
              mdebug<<"Setting min = "<<T->GetTokenAtAsDouble(2)
                    <<" max = "<<T->GetTokenAtAsDouble(3)
                    <<" temperature = "<<T->GetTokenAtAsDouble(4)
                    <<"for source "<<Source->GetName()<<endl;
            } else {
              Typo(i, "Cannot parse token Energy - thermal bremsstrahlung correctly:"
                   " Content not reasonable");
              return false;
            }
          } else {
            Typo(i, "Cannot parse token Energy - thermal bremsstrahlung correctly:"
                 " Number of tokens is not correct!");
            return false;
          }
        } else if (Source->GetSpectralType() == MCSource::c_BandFunction) {
          if (T->GetNTokens() == 7) {
            if (Source->SetEnergy(T->GetTokenAtAsDouble(2)*keV, 
                                  T->GetTokenAtAsDouble(3)*keV,
                                  T->GetTokenAtAsDouble(4),
                                  T->GetTokenAtAsDouble(5),
                                  T->GetTokenAtAsDouble(6)*keV) == true) {
              mdebug<<"Setting min = "<<T->GetTokenAtAsDouble(2)
                    <<" max = "<<T->GetTokenAtAsDouble(3)
                    <<" alpha = "<<T->GetTokenAtAsDouble(4)
                    <<" beta = "<<T->GetTokenAtAsDouble(5)
                    <<" E0 = "<<T->GetTokenAtAsDouble(6)
                    <<"for source "<<Source->GetName()<<endl;
            } else {
              Typo(i, "Cannot parse token Energy - thermal bremsstrahlung correctly:"
                   " Content not reasonable");
              return false;
            }
          } else {
            Typo(i, "Cannot parse token Energy - thermal bremsstrahlung correctly:"
                 " Number of tokens is not correct!");
            return false;
          }
        } else {
          Typo(i, "SpectralType is not yet implemented!");
          return false;
        }
      } // token
    } // is source
  } // Step 5
  // --> end depreciated


  // Step 6: Tertiary Source parameters
  for (i = 0; i < GetNLines(); ++i) {
    MTokenizer* T = GetTokenizerAt(i);

    if (T->GetNTokens() == 0) continue;
    if ((Source = GetSource(T->GetTokenAt(0))) != 0) {
      if (T->IsTokenAt(1, "Intensity", true) == true ||
          T->IsTokenAt(1, "Flux", true) == true) {
        if (T->GetNTokens() == 3) {
          double Flux = 0.0;
          if (Source->GetCoordinateSystem() == MCSource::c_FarField) {
            Flux = T->GetTokenAtAsDouble(2)/cm/cm/s;
          } else {
            Flux = T->GetTokenAtAsDouble(2)/s;
          }

          if (Source->SetFlux(Flux) == true) {
            mdebug<<"Setting average total flux "<<T->GetTokenAtAsDouble(2)
                  <<" gamma/sec[/cm2] for source "<<Source->GetName()<<endl;
          } else {
            Typo(i, "Cannot parse token Flux correctly:"
                 " ?? Perhaps the value is non-positive ??");
            return false;
          }
        } else {
          Typo(i, "Cannot parse token Flux correctly:"
               " Number of tokens is not correct!");
          return false;
        }
      } // token
    } // is source
  } // Step 6






  // Do some consistency checks...
  if (MFile::Exists(m_GeometryFileName) == false) {
    mout<<"The geometry file does not exist: "<<m_GeometryFileName<<endl;
    return false;
  }
  
  if (m_CreateCrossSectionFiles == false) {
    // Check if we have a run
    if (m_RunList.size() < 1 && m_ActivatorList.size() < 1) {
      mout<<"The source file must contain at least one run or activator!"<<endl;
      return false;
    }
    
    // Check if each run has a source:
    for (unsigned int r = 0; r < m_RunList.size(); ++r) {
      vector<MCSource*>& Sources = m_RunList[r].GetSourceList();
      if (Sources.size() == 0) {
        mout<<"The run "<<m_RunList[r].GetName()<<" contains no sources!"<<endl;
        return false;
      }
    }
  }
  
  // If the source beam is of type: c_NearFieldReverseDirectionToPredecessor then make sure we have a predecessor
  for (unsigned int r = 0; r < m_RunList.size(); ++r) {
    vector<MCSource*>& Sources = m_RunList[r].GetSourceList();
    for (unsigned int so = 0; so < Sources.size(); ++so) {
      if (Sources[so]->GetBeamType() == MCSource::c_NearFieldReverseDirectionToPredecessor) {
        bool Found = false;
        for (unsigned int so2 = 0; so2 < Sources.size(); ++so2) {
          if (Sources[so]->GetName() == Sources[so2]->GetSuccessor()) {
            Found = true;
            break;
          }
        }
        if (Found == false) {
          mout<<"The source "<<Sources[so]->GetName()<<" has a beam of type "<<Sources[so]->GetBeamTypeAsString()<<", but the source is not the successor of any other source!"<<endl;
          return false;
        }
      }
    }
  }

  // Check if each region has a volume and a cut:
  for (unsigned int r = 0; r < m_RegionList.size(); ++r) {
    if (m_RegionList[r].GetVolumeName() == g_StringNotDefined) {
      mout<<"The region "<<m_RegionList[r].GetName()<<" contains no volume!"<<endl;
      return false;
    }
    if (m_RegionList[r].GetRangeCut() == g_DoubleNotDefined) {
      mout<<"The region "<<m_RegionList[r].GetName()<<" contains no range cut!"<<endl;
      return false;
    }
  }


  // Set the start area
  if (StartAreaType != MCSource::c_StartAreaUnknown) {
    if (StartAreaParameters.size() != 8) {
      mout<<"Something went wrong: The number of start parameters is not 8!"<<endl;
      return false;
    }
    for (unsigned int r = 0; r < m_RunList.size(); ++r) {
      vector<MCSource*>& Sources = m_RunList[r].GetSourceList();
      for (unsigned int so = 0; so < Sources.size(); ++so) {
        Sources[so]->SetStartAreaType(StartAreaType);
        Sources[so]->SetStartAreaParameters(StartAreaParameters[0],
                                           StartAreaParameters[1],
                                           StartAreaParameters[2],
                                           StartAreaParameters[3],
                                           StartAreaParameters[4],
                                           StartAreaParameters[5],
                                           StartAreaParameters[6],
                                           StartAreaParameters[7]);
      }
    }
  }

  // Check if all successor are valid:
  for (unsigned int r = 0; r < m_RunList.size(); ++r) {
    vector<MCSource*>& Sources = m_RunList[r].GetSourceList();
    for (unsigned int so = 0; so < Sources.size(); ++so) {
//       if (Sources[s]->GetFlux() <= 0 && Sources[s]->IsSuccessor() == false) {
//         mout<<"Source "<<Sources[s]->GetName()
//             <<" does not have a positive flux!"<<endl;
//         return false;        
//       }
      if (Sources[so]->IsSuccessor() == true) {
        // Check if that's true
        int NSuccessings = 0;
        for (unsigned int j = 0; j < Sources.size(); ++j) {
          if (Sources[j]->GetSuccessor() == Sources[so]->GetName()) {
            NSuccessings++;
          }
        }
        if (NSuccessings == 0) {
          mout<<"Source "<<Sources[so]->GetName()
              <<" is not the successor of any source!"<<endl;
          return false;
        } else if (NSuccessings > 1) {
          mout<<"Source "<<Sources[so]->GetName()
              <<" is the successor of "<<NSuccessings<<" sources:"<<endl;
          for (unsigned int j = 0; j < Sources.size(); ++j) {
            if (Sources[j]->GetSuccessor() == Sources[so]->GetName()) {
              mout<<"  * "<<Sources[j]->GetName()<<endl;
            }
          }
          return false;
        }
      }
    }
  }


  // Give the detector time constant to the activator
  for (unsigned int r = 0; r < m_ActivatorList.size(); ++r) {
    m_ActivatorList[r].SetHalfLifeCutOff(m_DetectorTimeConstant);
  }
  
  // Give the activation mode to the run
  for (unsigned int r = 0; r < m_RunList.size(); ++r) {
    m_RunList[r].SetActivationMode(m_DecayMode);
  }


  return true;
}


/******************************************************************************
 * Return the run list of this parameter file
 */
vector<MCRun>& MCParameterFile::GetRunList()
{
  return m_RunList;
}


/******************************************************************************
 * Return a run per name or zero if it does not exist
 */
MCRun* MCParameterFile::GetRun(MString Name)
{
  for (unsigned int i = 0; i < m_RunList.size(); ++i) {
    if (m_RunList[i].GetName() == Name) {
      return &m_RunList[i];
    }
  }

  return 0;
}


/******************************************************************************
 * Remove all but the given run from the list
 */
void MCParameterFile::RestrictToRun(int Run)
{
  for (int i = 0; i < Run; ++i) {
    m_RunList.erase(m_RunList.begin());
  }
  while (m_RunList.size() > 1) {
    m_RunList.erase(m_RunList.begin()+1);
  }
}


/******************************************************************************
 * Return an activator per name or zero if it does not exist
 */
MCActivator* MCParameterFile::GetActivator(MString Name)
{
  for (unsigned int i = 0; i < m_ActivatorList.size(); ++i) {
    if (m_ActivatorList[i].GetName() == Name) {
      return &m_ActivatorList[i];
    }
  }

  return 0;
}


/******************************************************************************
 * Return a region per name or zero if it does not exist
 */
MCRegion* MCParameterFile::GetRegion(MString Name)
{
  for (unsigned int i = 0; i < m_RegionList.size(); ++i) {
    if (m_RegionList[i].GetName() == Name) {
      return &m_RegionList[i];
    }
  }

  return 0;
}


/******************************************************************************
 * Return a source per name or zero if it does not exist
 */
MCSource* MCParameterFile::GetSource(MString Name)
{
  for (unsigned int i = 0; i < m_RunList.size(); ++i) {
    for (unsigned int so = 0; so < m_RunList[i].GetSourceList().size(); ++so) {
      if (Name.AreIdentical(m_RunList[i].GetSourceList()[so]->GetName())) {
        return m_RunList[i].GetSourceList()[so];
      }
    }
  }

  return 0;
}


/*
 * MCParameterFile.cc: the end...
 ******************************************************************************/
