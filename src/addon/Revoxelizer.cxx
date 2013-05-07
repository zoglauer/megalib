/* 
 * Revoxelizer.cxx
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
#include <map>
using namespace std;

// ROOT
#include <TApplication.h>
#include <TStyle.h>
#include <TH1.h>
#include <TCanvas.h>
#include <MString.h>

// MEGAlib
#include "MGlobal.h"
#include "MStreams.h"
#include "MDGeometryQuest.h"
#include "MDDetector.h"
#include "MFileEventsSim.h"
#include "MDVolumeSequence.h"
#include "MSimEvent.h"
#include "MSimHT.h"

/******************************************************************************/

class Revoxelizer
{
public:
  /// Default constructor
  Revoxelizer();
  /// Default destructor
  ~Revoxelizer();
  
  /// Parse the command line
  bool ParseCommandLine(int argc, char** argv);
  /// Analyze whatever needs to be analyzed...
  bool Analyze();
  /// Revoxelize by distance
  bool RevoxelizeByDistance(MSimEvent* Event);
  /// Revoxelize by geometry
  bool RevoxelizeByGeometry(MSimEvent* Event);
  /// Interrupt the analysis
  void Interrupt() { m_Interrupt = true; }

  /// Show spectra per detector
  bool Spectra();

private:
  /// True, if the analysis needs to be interrupted
  bool m_Interrupt;

  /// Simulation file name
  MString m_FileName;
  /// Output file name
  MString m_OutputFileName;
  /// Geometry file name
  MString m_GeoFileName;
  
  /// Cluserization mode
  int m_Mode;
  /// Distance
  double m_MinDistance;
};

/******************************************************************************/


/******************************************************************************
 * Default constructor
 */
Revoxelizer::Revoxelizer() : m_Interrupt(false)
{
  gStyle->SetPalette(1, 0);
}


/******************************************************************************
 * Default destructor
 */
Revoxelizer::~Revoxelizer()
{
  // Intentionally left blanck
}


/******************************************************************************
 * Parse the command line
 */
bool Revoxelizer::ParseCommandLine(int argc, char** argv)
{
  ostringstream Usage;
  Usage<<endl;
  Usage<<"  Usage: Revoxelizer <options>"<<endl;
  Usage<<"    Choose one clusterization mode:"<<endl;
  Usage<<"         -d <double>:   by distance"<<endl;
  Usage<<"         -s:   as given in the geometry file (for revoxilization into larger voxels)"<<endl;
  Usage<<"    General options:"<<endl;
  Usage<<"         -f:   simulation file name"<<endl;
  Usage<<"         -o:   output simulation file name"<<endl;
  Usage<<"         -g:   geometry file name"<<endl;
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
    if (Option == "-f" || Option == "-o") {
      if (!((argc > i+1) && argv[i+1][0] != '-')){
        cout<<"Error: Option "<<argv[i][1]<<" needs a second argument!"<<endl;
        cout<<Usage.str()<<endl;
        return false;
      }
    } 
    // Multiple arguments_
    //else if (Option == "-??") {
    //  if (!((argc > i+2) && argv[i+1][0] != '-' && argv[i+2][0] != '-')){
    //    cout<<"Error: Option "<<argv[i][1]<<" needs two arguments!"<<endl;
    //    cout<<Usage.str()<<endl;
    //    return false;
    //  }
    //}

    // Then fulfill the options:
    if (Option == "-f") {
      m_FileName = argv[++i];
      cout<<"Accepting file name: "<<m_FileName<<endl;
    } else if (Option == "-o") {
      m_OutputFileName = argv[++i];
      cout<<"Accepting output file name: "<<m_OutputFileName<<endl;
    } else if (Option == "-g") {
      m_GeoFileName = argv[++i];
      cout<<"Accepting file name: "<<m_GeoFileName<<endl;
    } else if (Option == "-d") {
      m_Mode = 1;
      m_MinDistance = atof(argv[++i]);
      cout<<"Accepting clustering by distance: "<<m_MinDistance<<endl;
    } else if (Option == "-s") {
      m_Mode = 2;
      cout<<"Accepting re-voxelization as given in the geometry file"<<endl;
    } else {
      cout<<"Error: Unknown option \""<<Option<<"\"!"<<endl;
      cout<<Usage.str()<<endl;
      return false;
    }
  }

  if (m_Mode == 0) {
    cout<<"Error: Need to give a clusterzation/voxelization mode!"<<endl;
    cout<<Usage.str()<<endl;
    return false;
  }

  if (m_FileName == "") {
    cout<<"Error: Need a simulation file name!"<<endl;
    cout<<Usage.str()<<endl;
    return false;
  }

  if (m_GeoFileName == "") {
    cout<<"Error: Need a geometry file name!"<<endl;
    cout<<Usage.str()<<endl;
    return false;
  }

  if (m_FileName.EndsWith(".sim") == true) {
    if (m_OutputFileName == "") {
      m_OutputFileName = m_FileName;
      m_OutputFileName.Replace(m_FileName.Length()-4, 4, ".mod.sim");
      cout<<"Accepting output file name: "<<m_OutputFileName<<endl;
    }    
  } else {
    cout<<"Error: Need a simulation file name, not a "<<m_FileName<<" file "<<endl;
    cout<<Usage.str()<<endl;
    return false;
  }

  return true;
}


/******************************************************************************
 * Do whatever analysis is necessary
 */
bool Revoxelizer::Analyze()
{
  if (m_Interrupt == true) return false;

  // Load geometry:
  MDGeometryQuest Geometry;

  if (Geometry.ScanSetupFile(m_GeoFileName) == true) {
    cout<<"Geometry "<<Geometry.GetName()<<" loaded!"<<endl;
    Geometry.ActivateNoising(false);
    Geometry.SetGlobalFailureRate(0.0);
  } else {
    cout<<"Loading of geometry "<<Geometry.GetName()<<" failed!!"<<endl;
    return false;
  }  

  MFileEventsSim SiReader(&Geometry);
  if (SiReader.Open(m_FileName) == false) {
    cout<<"Unable to open sim file!"<<endl; 
    return false;
  }
  cout<<"Opened file "<<SiReader.GetFileName()<<" created with MEGAlib version: "<<SiReader.GetMEGAlibVersion()<<endl;
  SiReader.ShowProgress();

  
  //cout<<"Triggered events: "<<SiReader.GetNEvents(false)<<" --- Observation time: "<<SiReader.GetObservationTime()<<" sec  --  simulated events: "<<SiReader.GetSimulatedEvents()<<endl;

  // Open output file:
  MFileEventsSim SiWriter(&Geometry);
  if (SiWriter.Open(m_OutputFileName, MFile::c_Write) == false) {
    cout<<"Unable to open output file!"<<endl;
    return false;
  }
   
  SiWriter.SetGeometryFileName(m_GeoFileName);
  SiWriter.SetVersion(25);
  SiWriter.WriteHeader();

  MSimEvent* SiEvent = 0;
  while ((SiEvent = SiReader.GetNextEvent(false)) != 0) {
    if (m_Mode == 1) {
      if (RevoxelizeByDistance(SiEvent) == true) {
        SiWriter.AddEvent(SiEvent);      
      }
    } else if (m_Mode == 2) {
      if (RevoxelizeByGeometry(SiEvent) == true) {
        SiWriter.AddEvent(SiEvent);      
      }
    }
    delete SiEvent;
  }

  cout<<"Observation time: "<<SiReader.GetObservationTime()<<" sec  --  simulated events: "<<SiReader.GetSimulatedEvents()<<endl;

  SiWriter.SetObservationTime(SiReader.GetObservationTime());
  SiWriter.SetSimulatedEvents(SiReader.GetSimulatedEvents());
  
  SiWriter.CloseEventList();
  SiWriter.Close();

  SiReader.Close();
  
  return true;
}


/******************************************************************************
 * Analyze the event, return true if it has to be writen to file
 */
bool Revoxelizer::RevoxelizeByGeometry(MSimEvent* Event)
{
  for (int d = MDDetector::c_MinDetector; d <= MDDetector::c_MaxDetector; ++d) {
    Event->Discretize(d);
  }
  
  return true;
}


/******************************************************************************
 * Analyze the event, return true if it has to be writen to file
 */
bool Revoxelizer::RevoxelizeByDistance(MSimEvent* Event)
{
  // Add your code here

  vector<MSimHT*> Hits; // contains copies of the original hits
  list<MSimCluster*> Clusters;
  for (unsigned int h = 0; h < Event->GetNHTs(); ++h) {
    MSimCluster* Cluster = new MSimCluster();
    Cluster->AddHT(Event->GetHTAt(h)); // The original - no copy! The cluster does not delete it's hits, but makes also no copies
    Clusters.push_back(Cluster);
    Hits.push_back(Event->GetHTAt(h)); // The original - no copy!
  }
  
  for (list<MSimCluster*>::iterator c1 = Clusters.begin(); c1 != Clusters.end(); ++c1) {
    list<MSimCluster*>::iterator c2 = c1;
    ++c2;
    for (; c2 != Clusters.end(); /* handled in code due to erase ++c2 */) {
      if ((*c1)->GetVolumeSequence()->HasSameDetector((*c2)->GetVolumeSequence()) == false) { ++c2; continue; }
      double Distance = numeric_limits<double>::max();
      for (unsigned int h1 = 0; h1 < (*c1)->GetNHTs(); ++h1) {
        for (unsigned int h2 = 0; h2 < (*c2)->GetNHTs(); ++h2) {
          if (((*c1)->GetHTAt(h1)->GetPosition() - (*c2)->GetHTAt(h2)->GetPosition()).Mag() < Distance) {
            Distance = ((*c1)->GetHTAt(h1)->GetPosition() - (*c2)->GetHTAt(h2)->GetPosition()).Mag();
          }
        }
      }
      if (Distance < m_MinDistance) {
        for (unsigned int h2 = 0; h2 < (*c2)->GetNHTs(); ++h2) {
          (*c1)->AddHT((*c2)->GetHTAt(h2));
        }
        MSimCluster* Cluster = (*c2);
        c2 = Clusters.erase(c2);
        delete Cluster;
      } else {
        ++c2;
      }
    }  
  }
  
  vector<MSimHT*> NewHits;
  for (list<MSimCluster*>::iterator c1 = Clusters.begin(); c1 != Clusters.end(); ++c1) {
    NewHits.push_back((*c1)->CreateHT());
    //delete (*c1); // the cluster does not delete its hits, it is just a container!
  }

  for (unsigned int h = 0; h < Hits.size(); ++h) {
    Event->RemoveHT(Hits[h]);
    delete Hits[h];
  }
 
  for (unsigned int n = 0; n < NewHits.size(); ++n) {
    Event->AddHT(NewHits[n]);
    // delete NewHits[n]; --> deleted by the event
  }
   
  for (list<MSimCluster*>::iterator c1 = Clusters.begin(); c1 != Clusters.end(); ++c1) {
    delete (*c1);
  }
  
  return true;
}


/******************************************************************************/

Revoxelizer* g_Prg = 0;

/******************************************************************************/


/******************************************************************************
 * Called when an interrupt signal is flagged
 * All catched signals lead to a well defined exit of the program
 */
void CatchSignal(int a)
{
  cout<<"Catched signal Ctrl-C (ID="<<a<<"):"<<endl;
  
  if (g_Prg != 0) {
    g_Prg->Interrupt();
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

  // Initialize global MEGAlib variables, especially mgui, etc.
  MGlobal::Initialize();

  TApplication RevoxelizerApp("RevoxelizerApp", 0, 0);

  g_Prg = new Revoxelizer();

  if (g_Prg->ParseCommandLine(argc, argv) == false) {
    cerr<<"Error during parsing of command line!"<<endl;
    return -1;
  } 
  if (g_Prg->Analyze() == false) {
    cerr<<"Error during analysis!"<<endl;
    return -2;
  } 

  //RevoxelizerApp.Run();

  cout<<"Program exited normally!"<<endl;

  return 0;
}

/*
 * Cosima: the end...
 ******************************************************************************/
