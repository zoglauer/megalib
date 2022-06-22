/*
 * MInterfaceGeomega.cxx
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
// MInterfaceGeomega
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MInterfaceGeomega.h"

// Standard libs:
#include <iostream>
#include <sstream>
#include <vector>
using namespace std;

// ROOT libs:
#include "TROOT.h"
#include "MString.h"
#include "TCanvas.h"
#include "TView.h"
#include "TGMsgBox.h"
#include "TH2.h"
#include "TCanvas.h"

// MEGAlib libs:
#include "MGlobal.h"
#include "MAssert.h"
#include "MFile.h"
#include "MStreams.h"
#include "MDVolume.h"
#include "MDCalorimeter.h"
#include "MDStrip2D.h"
#include "MDStrip3D.h"
#include "MDDetector.h"
#include "MGUIGeomegaMain.h"
#include "MGUIMGeant.h"
#include "MVector.h"
#include "MDGridPoint.h"
#include "MPrelude.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MInterfaceGeomega)
#endif


////////////////////////////////////////////////////////////////////////////////


MInterfaceGeomega::MInterfaceGeomega() : MInterface()
{
  // standard constructor
  m_Geometry = new MDGeometryQuest();
  m_Data = new MSettingsGeomega();
  m_BasicGuiData = dynamic_cast<MSettings*>(m_Data);
}



////////////////////////////////////////////////////////////////////////////////


MInterfaceGeomega::~MInterfaceGeomega()
{
  // standard destructor
  
  // do not delete m_Geometry, because it is deleted in the base class;
  delete m_Data;
  // do not delete m_BasicGuiData, because it is just a pointer to m_Data!
}


////////////////////////////////////////////////////////////////////////////////


bool MInterfaceGeomega::ParseCommandLine(int argc, char** argv)
{
  ostringstream Usage;
  Usage<<endl;
  Usage<<"  Usage: Geomega <options>"<<endl;
  Usage<<endl;
  Usage<<"      -f --filename <filename> or"<<endl;
  Usage<<"      -g --geometry <filename>:"<<endl;
  Usage<<"             Use this file as geometry-file"<<endl;
  Usage<<"      -c --configuration <filename>.cfg:"<<endl;
  Usage<<"             Use this file as parameter file."<<endl;
  Usage<<"             All other given infromations such as -f and -g overwrite information in the configuration file."<<endl;
  Usage<<"             If no configuration file is give ~/.geomega.cfg is used"<<endl;
  Usage<<"      -a --auto:"<<endl;
  Usage<<"             Automatically show the geometry upon start"<<endl;
  Usage<<endl;
  Usage<<"      -s --startvolume <name>:"<<endl;
  Usage<<"             Use this volume as world volume"<<endl;
  Usage<<"             (If this volume exists as several"<<endl;
  Usage<<"              copies, use one of the Copy names!)"<<endl;
  Usage<<"             WARNING: DO NOT trust absolute coordinate"<<endl;
  Usage<<"                      values anymore !!!!!"<<endl;
  Usage<<"      -r --reveal <name>:"<<endl;
  Usage<<"             Only show this volume and all other volumes given with this option"<<endl;
  Usage<<endl;
  Usage<<"      -d --debug:"<<endl;
  Usage<<"             Use debug mode"<<endl;
  Usage<<"      -v --verbosity <integer>:"<<endl;
  Usage<<"             The higher the integer, the higher the verbosity..."<<endl;
  Usage<<"      -h --help:"<<endl;
  Usage<<"             You know the answer..."<<endl;
  Usage<<endl;
  Usage<<"         --create-cross-sections:"<<endl;
  Usage<<"             Create cross section files"<<endl;
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
    if (Option == "-f" || Option == "--filename" ||
        Option == "-g" || Option == "--geometry" ||
        Option == "--create-mggpod" ||
        Option == "-s" || Option == "--startvolume") {
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
    } else if (Option == "--verbosity" || Option == "-v") {
      g_Verbosity = atoi(argv[++i]);
      cout<<"Command-line parser: Using verbosity level "<<g_Verbosity<<endl;
    } else if (Option == "--debug" || Option == "-d") {
      if (g_Verbosity < c_Warning) g_Verbosity = c_Warning;
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
    if (Option == "--filename" || Option == "-f" || 
        Option == "--geometry" || Option == "-g") {
      if (m_Data->SetCurrentFileName(argv[++i]) == false) {
        cout<<"Command-line parser: The file could not be opened correctly!"<<endl;
        return false;
      }
      cout<<"Command-line parser: Use file "<<m_Data->GetCurrentFileName()<<endl;
    } else if (Option == "--startvolume" || Option == "-s") {
      m_Geometry->SetStartVolumeName(argv[++i]);
      cout<<"Command-line parser: Using start (=new mother) volume "<<m_Geometry->GetStartVolumeName()<<endl;
    } else if (Option == "--reveal" || Option == "-r") {
      MString Preferred = argv[++i];
      m_Geometry->AddPreferredVisibleVolume(Preferred);
      cout<<"Command-line parser: Useing preferred volume "<<Preferred<<endl;
    } else if (Option == "--special") {
      m_Data->SetSpecialMode(true);
      cout<<"Command-line parser: Activating special mode - hope, you know what you are doing..."<<endl;
    }
  }


  // Now parse all batch-mode options
  for (int i = 1; i < argc; i++) {
    Option = argv[i];
    if (Option == "--create-cross-sections") {
      cout<<"Command-line parser: Creating cross section files"<<endl;
      gROOT->SetBatch(true);
      CreateCrossSections();
      return false;
    }
  }

  if (m_UseGui == true) {
    m_Gui = new MGUIGeomegaMain(this, m_Data);
    m_Gui->Create();
  } else {
    return false;
  }

  // Show change log / license if changed:
  MPrelude P;
  if (P.Play() == false) return false; // license was not accepted
  
  // Now parse all high-level options
  for (int i = 1; i < argc; i++) {
    Option = argv[i];
    if (Option == "--auto" || Option == "-a") {
      cout<<"Command-line parser: View geometry"<<endl;
      ViewGeometry();
    }
  }

  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MInterfaceGeomega::ReloadRequired() 
{
  //! Return true if we need to (re-) load the geometry

  if (m_Geometry->IsScanned() == false) {
    return true;
  }
  if (m_Geometry->RequiresReload() == true) {
    return true;
  }

  MString GeometryFilename = m_Geometry->GetFileName();
  MFile::ExpandFileName(GeometryFilename);
  
  MString SettingsFilename = m_Data->GetCurrentFileName();
  MFile::ExpandFileName(SettingsFilename);

  if (GeometryFilename != SettingsFilename) {
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////


bool MInterfaceGeomega::LoadConfiguration(MString FileName)
{  
  // Load the configuration file

  if (m_Data == 0) {
    m_Data = new MSettingsGeomega();
    m_BasicGuiData = dynamic_cast<MSettings*>(m_Data);
    if (m_UseGui == true) {
      m_Gui->SetConfiguration(m_Data);
    }
  }
  
  m_Data->Read(FileName);

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MInterfaceGeomega::SaveConfiguration(MString FileName)
{
  // Save the configuration file

  if (m_Data == nullptr) return false;

  m_Data->Write(FileName);

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MInterfaceGeomega::ReadGeometry()
{
  // We do not ignore short names here since we might generate Geant3/MGGPOD files
  m_Geometry->LaunchedByGeomega();
  return m_Geometry->ScanSetupFile(m_Data->GetCurrentFileName());
}


////////////////////////////////////////////////////////////////////////////////


void MInterfaceGeomega::ViewGeometry()
{
  // Create a display of the geometry

  if (ReloadRequired() == true) {
    if (ReadGeometry() == false) {
      return;
    }
  }
  
  m_Geometry->DrawGeometry(nullptr, "ogle");
}


////////////////////////////////////////////////////////////////////////////////


void MInterfaceGeomega::RaytraceGeometry()
{
  // Create a display of the geometry

  if (ReloadRequired() == true) {
    if (ReadGeometry() == false) {
      return;
    }
  }
  
  m_Geometry->DrawGeometry(nullptr, "raytrace");
}


////////////////////////////////////////////////////////////////////////////////


void MInterfaceGeomega::TestIntersections()
{
  // Test for intersections
  
  ostringstream Diagnostics;
  HasIntersections(Diagnostics);
  mout<<Diagnostics.str()<<endl;
}
  
  
////////////////////////////////////////////////////////////////////////////////
  
  
bool MInterfaceGeomega::HasIntersections(ostringstream& Diagnostics)
{
  // Test for intersections

  
  if (ReloadRequired() == true) {
    if (ReadGeometry() == false) {
      Diagnostics<<"   ***  Error  ***"<<endl;
      Diagnostics<<"Unable to read diagnostics"<<endl;
      return false;
    }
  }

  mout<<endl;
  mout<<"Overlap checking started... this might take a while depending on the complexity of the geometry..."<<endl;
  
  Diagnostics<<endl;
  Diagnostics<<endl;
  Diagnostics<<"Overlap checking result:"<<endl;
  Diagnostics<<endl;
  Diagnostics<<"Part 1: ROOT"<<endl;
  Diagnostics<<endl;
  
  //bool NoOverlaps = m_Geometry->CheckOverlaps(Diagnostics);
  //if (NoOverlaps == false) return false;
  
  m_Geometry->CheckOverlaps(Diagnostics);
  
  Diagnostics<<endl;
  Diagnostics<<"Part 2: Geant4"<<endl;
  Diagnostics<<endl;
  
  if (MFile::Exists(g_MEGAlibPath + "/bin/cosima") == false) {
    Diagnostics<<"   ***  Warning  ***"<<endl;
    Diagnostics<<"Cannot check intersections with Geant4 since cosima is not present."<<endl;
    return false;
  }

  MString FileName = gSystem->TempDirectory();
  FileName += "/DelMe.source";

  ofstream out;
  out.open(FileName);
  if (out.is_open() == false) {
    Diagnostics<<"   ***  Error  ***"<<endl;
    Diagnostics<<"Unable to create cosima source file for overlap check"<<endl;
    return false;
  }

  out<<"Version                     1"<<endl;
  out<<"Geometry                   "<<m_Data->GetCurrentFileName()<<endl;
  out<<"CheckForOverlaps            1000 0.0001"<<endl;
  out<<"PhysicsListEM               LivermorePol"<<endl;

  out<<"Run Minimum"<<endl;
  out<<"Minimum.FileName            DelMe"<<endl;
  out<<"Minimum.NEvents             1"<<endl;
  
  out<<"Minimum.Source MinimumS"<<endl;
  out<<"MinimumS.ParticleType       1"<<endl;
  out<<"MinimumS.Beam               PointSource 0 0 0 "<<endl;
  out<<"MinimumS.Spectrum           Mono 511"<<endl;
  out<<"MinimumS.Flux               1.0"<<endl;

  out.close();

  MString WorkingDirectory = gSystem->WorkingDirectory();
  gSystem->ChangeDirectory(gSystem->TempDirectory());
  auto IgnoreLevel = gErrorIgnoreLevel;
  gErrorIgnoreLevel = kFatal;
  TString Pipe = gSystem->GetFromPipe(MString("bash -c \"source ${MEGALIB}/bin/source-megalib.sh; cosima ") + FileName + MString(" 2>&1 | sed -n '/G4Exception-START/,/G4Exception-END/p' | sed -e '/G4Exception-/c\\ '\""));
  //gSystem->GetFromPipe(MString("rm -f DelMe.*.sim ") + FileName);
  gErrorIgnoreLevel = IgnoreLevel;
  gSystem->ChangeDirectory(WorkingDirectory);
  
  if (Pipe != "") {
    Diagnostics<<"Geant4 error summary:"<<endl;
    Diagnostics<<Pipe<<endl;
  } else {
    Diagnostics<<"No extrusions or overlaps detected with Geant4"<<endl;
  }
  Diagnostics<<endl;
  
  return (Pipe == "") ? "true" : "false";
}


////////////////////////////////////////////////////////////////////////////////


void MInterfaceGeomega::CalculateMasses()
{
  // Calculate the mass of the geometry

  if (ReloadRequired() == true) {
    if (ReadGeometry() == false) {
      return;
    }
  }

  m_Geometry->CalculateMasses();  
}


////////////////////////////////////////////////////////////////////////////////


void MInterfaceGeomega::CreateCrossSections()
{
  // Calculate the mass of the geometry

  if (ReloadRequired() == true) {
   if (ReadGeometry() == false) {
      return;
    }
  }

  m_Geometry->CreateCrossSectionFiles();  
}


////////////////////////////////////////////////////////////////////////////////


void MInterfaceGeomega::DumpInformation()
{
  // Dump information about the geometry

  if (ReloadRequired() == true) {
    if (ReadGeometry() == false) {
      return;
    }
  }

  m_Geometry->DumpInformation();  
}


////////////////////////////////////////////////////////////////////////////////


void MInterfaceGeomega::FindVolume(MVector Pos)
{
  // Find the volume sequence for position Pos

  if (ReloadRequired() == true) {
    if (ReadGeometry() == false) {
      return;
    }
  }

  // First a goody: Check for overlaps:
  vector<MDVolume*> OverlappingVolumes;
  m_Geometry->GetWorldVolume()->FindOverlaps(Pos, OverlappingVolumes);

  cout<<endl;
  if (OverlappingVolumes.size() == 0) {
    cout<<"Outside worldvolume "<<Pos<<" cm:"<<endl;   
  } else if (OverlappingVolumes.size() == 1) {
    cout<<"Details for position "<<Pos<<" cm (no overlaps found) :"<<endl;
    MDVolumeSequence Vol = m_Geometry->GetVolumeSequence(Pos);
    cout<<Vol.ToString()<<endl;
  } else {
    cout<<"Following volumes overlap at position "<<Pos<<" cm:"<<endl;
    for (unsigned int i = 0; i < OverlappingVolumes.size(); ++i) {
      cout<<OverlappingVolumes[i]->GetName()<<endl;
    }
  }
}


////////////////////////////////////////////////////////////////////////////////


void MInterfaceGeomega::GetPathLengths(const MVector& Start, const MVector& Stop)
{
  // Find the volume sequence for position Pos

  if (ReloadRequired() == true) {
    if (ReadGeometry() == false) {
      return;
    }
  }

  map<MDMaterial*, double> Couples = m_Geometry->GetPathLengths(Start, Stop);
  
  for (const auto& C : Couples) {
    cout<<C.first->GetName()<<": "<<C.second<<" cm"<<endl;
  }  
}


////////////////////////////////////////////////////////////////////////////////


void MInterfaceGeomega::GetResolutions()
{
  // Return the resolutions of all detectors:

  if (ReloadRequired() == true) {
    if (ReadGeometry() == false) {
      return;
    }
  }

  int NBins = 101;

  for (unsigned int d = 0; d < m_Geometry->GetNDetectors(); ++d) {

    double Emin = 0;
    double Emax = 1000;

    MDDetector* D = m_Geometry->GetDetectorAt(d); 
    TH2D* ERes = new TH2D(MString("Energy resolution ") + D->GetName(), 
                          MString("Energy resolution ") + D->GetName(), 
                          NBins, Emin, Emax, NBins, Emin, Emax);
    ERes->SetXTitle("Initial energy [keV]");
    ERes->SetYTitle("Measured energy [keV]");

    double Pmin = 0;
    double Pmax = 1000;
    bool HasDepthResolution = false;
    if (D->GetType() == MDDetector::c_Calorimeter) {
      HasDepthResolution = dynamic_cast<MDCalorimeter*>(D)->HasDepthResolution();
    } else if (D->GetType() == MDDetector::c_Strip3D) {
      HasDepthResolution = true;;
    }

    TH2D* PRes = 0;
    TH2D* PDiff = 0;
    if (HasDepthResolution == true) {
      Pmin = -D->GetStructuralSize().Z();
      Pmax = D->GetStructuralSize().Z();
      double Tolerance = (Pmax-Pmin)/1000000;
      PRes = new TH2D(MString("Position resolution ") + D->GetName(), 
                      MString("Position resolution ") + D->GetName(), 
                      NBins, Pmin-Tolerance, Pmax+Tolerance, 
                      NBins, Pmin-Tolerance, Pmax+Tolerance);
      PRes->SetXTitle("Initial position [cm]");
      PRes->SetYTitle("Measured position [cm]");

      PDiff = new TH2D(MString("Position deviation ") + D->GetName(), 
                       MString("Position deviation ") + D->GetName(), 
                       NBins, -(Pmax-Pmin), Pmax-Pmin, 
                       NBins, Emin, Emax);
      PDiff->SetXTitle("Position difference [cm]");
      PDiff->SetYTitle("Measured energy [keV]");
    }

    double EnergyIn;
    double EnergyOut;
    MVector PosIn;
    MVector PosOut;
    double TimeIn;
    double TimeOut;
    double Depth;
    MString Flags;
    for (int i = 0; i < NBins*NBins*100; ++i) {
      EnergyIn = gRandom->Uniform(Emin, Emax);
      EnergyOut = EnergyIn;

      PosIn = MVector(0, 0, 0);
      if (HasDepthResolution == true) {
        Depth = gRandom->Uniform(Pmin, Pmax);
        PosIn.SetZ(Depth);
      }
      PosOut = PosIn;
      TimeIn = 0;
      TimeOut = TimeIn;
      D->Noise(PosOut, EnergyOut, TimeOut, Flags, D->GetSensitiveVolume(0));
      if (EnergyOut != 0) {
        ERes->Fill(EnergyIn, EnergyOut);
        if (HasDepthResolution == true) {
          PRes->Fill(PosIn.Z(), PosOut.Z());
          PDiff->Fill(PosIn.Z() - PosOut.Z(), EnergyOut);
        }
      }
    }
    TCanvas* EResCanvas = 
      new TCanvas(MString("Energy resolution ") + D->GetName(), 
                  MString("Energy resolution ") + D->GetName(),
                  400, 400);
    EResCanvas->cd();
    EResCanvas->SetFillColor(0);
    EResCanvas->SetBorderMode(0);    
    ERes->Draw("colz");
    EResCanvas->Update();

    if (HasDepthResolution == true) {
      TCanvas* PResCanvas = 
        new TCanvas(MString("Position resolution ") + D->GetName(), 
                    MString("Position resolution ") + D->GetName(),
                    400, 400);
      PResCanvas->cd();
      PResCanvas->SetFillColor(0);
      PResCanvas->SetBorderMode(0);    
      PRes->Draw("colz");
      PResCanvas->Update();    
      TCanvas* PDiffCanvas = 
        new TCanvas(MString("Position deviation ") + D->GetName(), 
                    MString("Position deviation ") + D->GetName(),
                    400, 400);
      PDiffCanvas->cd();
      PDiffCanvas->SetFillColor(0);
      PDiffCanvas->SetBorderMode(0);    
      PDiff->Draw("colz");
      PDiffCanvas->Update();    
    }

    // Depth dependent energy resolution for Strip detectors
    if (D->GetType() == MDDetector::c_Strip3D ||
        D->GetType() == MDDetector::c_Strip3DDirectional ||
        D->GetType() == MDDetector::c_DriftChamber) {
      MDStrip3D* Strip = dynamic_cast<MDStrip3D*>(D);
      if (Strip->HasEnergyResolutionDepthCorrection() == true) {
        double FixedEnergy = 1000;
        TH2D* EandDRes = new TH2D(MString("Energy resolution as a function of depth at 1 MeV ") + D->GetName(), 
                              MString("Energy resolution as a function of depth at 1 MeV ") + D->GetName(), 
                              NBins, 0.9*FixedEnergy, 1.1*FixedEnergy, NBins, -D->GetStructuralSize().Z(), D->GetStructuralSize().Z());
        EandDRes->SetXTitle("Measured energy [keV]");
        EandDRes->SetYTitle("Depth [cm]");
        
        for (int i = 0; i < NBins*NBins*100; ++i) {
          EnergyIn = FixedEnergy;
          EnergyOut = EnergyIn;
          
          double Depth = gRandom->Uniform(-Strip->GetStructuralSize().Z(), Strip->GetStructuralSize().Z());
          PosIn = MVector(0, 0, Depth);
          PosOut = PosIn;
          TimeIn = 0;
          TimeOut = TimeIn;
          Strip->Noise(PosOut, EnergyOut, TimeOut, Flags, Strip->GetSensitiveVolume(0));
          if (EnergyOut != 0) {
            EandDRes->Fill(EnergyOut, Depth);
          }
        }

        TCanvas* EandDResCanvas = 
          new TCanvas(MString("Energy resolution as a function of depth at 1 MeV ") + D->GetName(), 
                      MString("Energy resolution as a function of depth at 1 MeV ") + D->GetName(),
                      400, 400);
        EandDResCanvas->cd();
        EandDResCanvas->SetFillColor(0);
        EandDResCanvas->SetBorderMode(0);    
        EandDRes->Draw("colz");
        EandDResCanvas->Update();    
      }
      if (Strip->HasTriggerThresholdDepthCorrection() == true) {
        double Emin = 0;
        double Emax = 200;
        TH2D* ThresholdVsDepth = new TH2D(MString("Trigger threshold as a function of depth at 1 MeV ") + D->GetName(), 
                                          MString("Trigger threshold as a function of depth at 1 MeV ") + D->GetName(), 
                                          NBins, Emin, Emax, NBins, -D->GetStructuralSize().Z(), D->GetStructuralSize().Z());
        ThresholdVsDepth->SetXTitle("Measured energy [keV]");
        ThresholdVsDepth->SetYTitle("Depth [cm]");

        for (int i = 0; i < NBins*NBins*10; ++i) {
          EnergyIn = gRandom->Uniform(Emin, Emax);
          EnergyOut = EnergyIn;
          
          double Depth = gRandom->Uniform(-Strip->GetStructuralSize().Z(), Strip->GetStructuralSize().Z());
          PosIn = MVector(0, 0, Depth);
          PosOut = PosIn;
          TimeIn = 0;
          TimeOut = TimeIn;
          Strip->Noise(PosOut, EnergyOut, TimeOut, Flags, Strip->GetSensitiveVolume(0));
          if (EnergyOut != 0) {
            MDGridPoint Point(0, 0, 0, MDGridPoint::c_Voxel, Depth, EnergyIn); 
            if (Strip->IsAboveTriggerThreshold(EnergyOut, Point) == true) {
              ThresholdVsDepth->Fill(EnergyOut, Depth);
            }
          }
        }

        TCanvas* ThresholdVsDepthCanvas = 
          new TCanvas(MString("Trigger threshold as a function of depth at 1 MeV ") + D->GetName(), 
                      MString("Trigger threshold as a function of depth at 1 MeV ") + D->GetName(),
                      400, 400);
        ThresholdVsDepthCanvas->cd();
        ThresholdVsDepthCanvas->SetFillColor(0);
        ThresholdVsDepthCanvas->SetBorderMode(0);    
        ThresholdVsDepth->Draw("colz");
        ThresholdVsDepthCanvas->Update();            
      }
    }
  }
}


////////////////////////////////////////////////////////////////////////////////

/*
  void MInterfaceGeomega::ShowGeometryWithAxis() 
  {
  mout<<"Test only!"<<endl;

  if (m_Geometry->IsScanned() == false) {
  if (ReadGeometry() == false) {
  return;
  }
  }

  vector<MVector> Positions;
  Positions.push_back(MVector(57, 94, 125));
  Positions.push_back(MVector(-28.5, 28.5, 129));
  Positions.push_back(MVector(16, 10, 51));
  Positions.push_back(MVector(-7.5, -28.5, 41.0));
  Positions.push_back(MVector(-70, -175, 135));

  const double SizeAxis = 0.5;
  const double SizePosition = 2;
  const double SizeLine = 0.3;

  double LengthAxisXP = 30;
  double LengthAxisXM = -30;
  double LengthAxisYP = 30;
  double LengthAxisYM = -30;
  double LengthAxisZP = 30;
  double LengthAxisZM = -10;
  double LengthAxisMin = 0;

  for (unsigned int p = 0; p < Positions.size(); ++p) {
  // Add hit:
  m_Geometry->AddHit(Positions[p], SizePosition, 1);
  m_Geometry->AddLink(Positions[p], MVector(Positions[p][0], Positions[p][1], 0), SizeLine, 1);
  m_Geometry->AddLink(MVector(Positions[p][0], Positions[p][1], 0), MVector(Positions[p][0], 0, 0), SizeLine, 1);
  m_Geometry->AddLink(MVector(Positions[p][0], Positions[p][1], 0), MVector(0, Positions[p][1], 0), SizeLine, 1);    
  if (Positions[p][0] > LengthAxisXP) {
  LengthAxisXP = 1.1*Positions[p][0];
  } else if (Positions[p][0] < LengthAxisXM) {
  LengthAxisXM = 1.1*Positions[p][0];
  }
  if (Positions[p][1] > LengthAxisYP) {
  LengthAxisYP = 1.1*Positions[p][1];
  } else if (Positions[p][1] < LengthAxisYM) {
  LengthAxisYM = 1.1*Positions[p][1];
  }
  if (Positions[p][2] > LengthAxisZP) {
  LengthAxisZP = 1.1*Positions[p][2];
  } else if (Positions[p][2] < LengthAxisZM) {
  LengthAxisZM = 1.1*Positions[p][2];
  }
  }

  

  // Add axis:
  if (LengthAxisXM < -LengthAxisMin) {
  m_Geometry->AddLink(MVector(LengthAxisXM,0,0), MVector(-LengthAxisMin, 0, 0), SizeAxis, 1);
  }
  if (LengthAxisXP > -LengthAxisMin) {
  m_Geometry->AddLink(MVector(LengthAxisMin,0,0), MVector(LengthAxisXP, 0, 0), SizeAxis, 1);
  // x-marking:
  m_Geometry->AddLink(MVector(LengthAxisXP,-5,0), MVector(LengthAxisXP-5, -10, 0), SizeLine, 1);
  m_Geometry->AddLink(MVector(LengthAxisXP-5,-5,0), MVector(LengthAxisXP, -10, 0), SizeLine, 1);
  // x-arrow:
  m_Geometry->AddLink(MVector(LengthAxisXP, -2.5, 0), MVector(LengthAxisXP+5, 0, 0), SizeLine, 1);
  m_Geometry->AddLink(MVector(LengthAxisXP, 2.5, 0), MVector(LengthAxisXP+5, 0, 0), SizeLine, 1);
  }

  if (LengthAxisYM < -LengthAxisMin) {
  m_Geometry->AddLink(MVector(0,LengthAxisYM,0), MVector(0, -LengthAxisMin, 0), SizeAxis, 1);
  }
  if (LengthAxisYP > LengthAxisMin) {
  m_Geometry->AddLink(MVector(0,LengthAxisMin,0), MVector(0, LengthAxisYP, 0), SizeAxis, 1);
  // y-marking:
  m_Geometry->AddLink(MVector(-10, LengthAxisYP, 0), MVector(-7.5, LengthAxisYP-2.5, 0), SizeLine, 1);
  m_Geometry->AddLink(MVector(-10, LengthAxisYP-5, 0), MVector(-5, LengthAxisYP, 0), SizeLine, 1);
  // y-arrow:
  m_Geometry->AddLink(MVector(-2.5, LengthAxisYP, 0), MVector(0, LengthAxisYP+5, 0), SizeLine, 1);
  m_Geometry->AddLink(MVector(2.5, LengthAxisYP, 0), MVector(0, LengthAxisYP+5, 0), SizeLine, 1);
  }

  if (LengthAxisZM < -LengthAxisMin) {
  m_Geometry->AddLink(MVector(0,0,LengthAxisZM), MVector(0, 0, -LengthAxisMin), SizeAxis, 1);
  }
  
  if (LengthAxisZP > LengthAxisMin) {
  m_Geometry->AddLink(MVector(0,0,25), MVector(0, 0, LengthAxisZP), SizeAxis, 1);
  // z-marking:
  m_Geometry->AddLink(MVector(-10, 0, LengthAxisZP), MVector(-5, 0, LengthAxisZP), SizeLine, 1);
  m_Geometry->AddLink(MVector(-10, 0, LengthAxisZP), MVector(-5, 0, LengthAxisZP-5), SizeLine, 1);
  m_Geometry->AddLink(MVector(-10, 0, LengthAxisZP-5), MVector(-5, 0, LengthAxisZP-5), SizeLine, 1);
  // z-arrow:
  m_Geometry->AddLink(MVector(-2.5, 0, LengthAxisZP), MVector(0, 0, LengthAxisZP+5), SizeLine, 1);
  m_Geometry->AddLink(MVector(2.5, 0, LengthAxisZP), MVector(0, 0, LengthAxisZP+5), SizeLine, 1);
  }


  ViewGeometry();
  }
*/

////////////////////////////////////////////////////////////////////////////////


void MInterfaceGeomega::TestOnly() 
{
  FindVolume(MVector(-6.839, -17.542, 6.983));

  /*

  mout<<"Test only!"<<endl;

  if (m_Geometry->IsScanned() == false) {
  if (ReadGeometry() == false) {
  return;
  }
  }



  // 40
  double theta = 0;
  vector<MVector> Positions;
  Positions.push_back(MVector(-6.4, -6.4, 0.0));
  Positions.push_back(MVector(-6.4, 0, 0.0));
  //Positions.push_back(MVector(-11.6, 0, 0.0));
  Positions.push_back(MVector(-6.4, 6.4, 0.0));
  //Positions.push_back(MVector(-6.7, 16.0, 0.0));
  Positions.push_back(MVector(0, 6.4, 0.0));
  Positions.push_back(MVector(6.4, 6.4, 0.0));
  //Positions.push_back(MVector(11.6, 0, 0.0));
  Positions.push_back(MVector(6.4, 0, 0.0));
  Positions.push_back(MVector(0, 0, 0.0));
  Positions.push_back(MVector(0, -6.4, 0.0));
  Positions.push_back(MVector(6.4, -6.4, 0.0));
  //Positions.push_back(MVector(6.7, -16.5, 0.0));
  //Positions.push_back(MVector(0, -11.6, 0.0));

  MVector Direction(0.0, sin(theta*c_Rad), cos(theta*c_Rad));

  mout<<"Direction: "<<Direction.X()<<"!"<<Direction.Y()<<"!"<<Direction.Z()<<endl;

  const double SizeLine = 0.5*1.27;

  double UpperZ = 30;
  double LowerZ = -30;

  for (unsigned int p = 0; p < Positions.size(); ++p) {
  mout<<"Position: "<<Positions[p].X()<<"!"<<Positions[p].Y()<<"!"<<Positions[p].Z()<<endl;
  MVector Start = Positions[p] + Direction*(UpperZ - Positions[p].Z())*(1.0/Direction.Z());
  MVector Stop = Positions[p] + Direction*(LowerZ - Positions[p].Z())*(1.0/Direction.Z());

  m_Geometry->AddLink(Start, Stop, SizeLine, 4);
  }

  ViewGeometry();

  // Write the run info, e.g.
  // Run1_Pos0.SubRun SR_01
  // SR_01.GoToReference 1
  // SR_01.MoveAbsolute 1
  // SR_01.XYPosition 16.4 12.9
  // SR_01.TimeSpan 480

  char SubRunName[100];
  MString RunName("Run01_Pos01");
  MVector XYZero(12.9, 16.4, 0.0);
  MVector Offset(1.75, -0.5);
  MVector NewPos;
  for (unsigned int p = 0; p < Positions.size(); ++p) {
  sprintf(SubRunName, "SR_%i", p+1);
  mout<<RunName<<".SubRun "<<SubRunName<<endl;
  if (p == 0) {
  mout<<SubRunName<<".GoToReference 1"<<endl;
  } else {
  mout<<SubRunName<<".GoToReference 0"<<endl;
  }
  mout<<SubRunName<<".MoveAbsolute 1"<<endl;
  NewPos = Positions[p] + XYZero + Offset;
  mout<<SubRunName<<".XYPosition "<<NewPos.Y()<<" "<<NewPos.X()<<endl;
  mout<<SubRunName<<".NTriggers 35000"<<endl;
  mout<<endl;
  }

  */
}


// MInterfaceGeomega: the end...
////////////////////////////////////////////////////////////////////////////////
