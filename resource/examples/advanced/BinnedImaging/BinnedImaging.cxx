/* 
 * BinnedComptonImaging.cxx
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
#include <vector>
#include <cmath>
#include <map>
#include <thread>
#include <mutex>
using namespace std;

// ROOT
#include <TApplication.h>
#include <TStyle.h>
#include <TH1.h>
#include <TH2.h>
#include <TCanvas.h>
#include <MString.h>
#include <TRotation.h>
#include <TMatrix.h>
#include <TMath.h>

// MEGAlib
#include "MGlobal.h"
#include "MStreams.h"
#include "MDGeometryQuest.h"
#include "MDDetector.h"
#include "MFileEventsSim.h"
#include "MDVolumeSequence.h"
#include "MSimEvent.h"
#include "MSimHT.h"
#include "MFileEventsTra.h"
#include "MComptonEvent.h"
#include "MPhysicalEvent.h"
#include "MResponseMatrixON.h"
#include "MResponseMatrixAxis.h"
#include "MResponseMatrixAxisSpheric.h"
#include "MEventSelector.h"
#include "MSettingsMimrec.h"
#include "MImageGalactic.h"
#include "MTimer.h"


/******************************************************************************/


/* TODO: I doubt the pointing, i.e. the time from which we calculate the s_j's and the full repsone is right... we should do it for all events, actually all the time we are pointing not just for the events. If I just do it for the events then the off-axis response is wrong
 * 
 * 
 */ 
class BinnedComptonImaging
{
public:
  /// Default constructor
  BinnedComptonImaging();
  /// Default destructor
  ~BinnedComptonImaging();
  
  /// Parse the command line
  bool ParseCommandLine(int argc, char** argv);
  /// Reconstruct the image
  bool Reconstruct();
  /// Interrupt the analysis
  void Interrupt() { m_Interrupt = true; }


protected:
  //! Read and prepare the detector response
  bool PrepareResponse();
  
  //! Read the events and prepare the data space
  bool PrepareDataSpace();
  
  //! Build the background model
  bool BuildBackgroundModel();
  
  //! Create the response matrix for an event
  MResponseMatrixON CreateResponseSliceGalactic(unsigned int DBin, MRotation InverseGalRot);
  
  //! Create the Galactic response
  bool CreateGalacticResponse();
  
  //! Create the Galactic background modfel
  bool CreateGalacticBackgroundModel();
  
  //! Create an exposure map
  bool CreateExposureMap();
  
  //! Parallel response rotation in to Galactic coordinates
  bool RotateResponseInParallel(unsigned int ThreadID, vector<unsigned int> PointingBinsX, vector<unsigned int> PointingBinsZ);  
  //! Parallel background model rotation
  bool RotateBackgroundModelInParallel(unsigned int ThreadID, unsigned int ModelID, vector<unsigned int> PointingBinsX, vector<unsigned int> PointingBinsZ);
  
  //! Reconstruct an image using Richardson-Lucy
  bool ReconstructRL();
  
  //! Reconstruct an image using Maximum Entropy
  bool ReconstructMEM();
  
  //! Show an image in Galactic coordinates
  bool ShowImageGalacticCoordinates(MResponseMatrixON Image, MString Title, MString zAxis, bool Save = false, MString SaveTitle = "");
  
  
private:
  //! True, if the analysis needs to be interrupted
  bool m_Interrupt;
  
  //! Write files
  bool m_WriteFiles;
  
  //! Just create the background model
  bool m_JustBuildBackgroundModel;
  
  /// Simulation file name
  MString m_FileName;
  /// Mimrec configuration file name
  MString m_MimrecCfgFileName;
  /// Response file name
  MString m_ResponseFileName;
  
  /// The number of iterations the algorithm has to run
  unsigned int m_Iterations;
  
  /// The algorithm (1: RL, 2: MaxEnt)
  unsigned int m_DeconvolutionAlgorithm;
  
  //! The stored response
  MResponseMatrixON m_Response;
  //! The number of response bins
  unsigned long m_RBins;
  //! The number of image space bins
  unsigned long m_IBins;
  //! The number of data space bins
  unsigned long m_DBins;
  
  //! Response matrix dimension 0
  unsigned int m_InitialEnergyBins;
  //! Response matrix dimension 1
  unsigned int m_InitialDirectionBins;
  //! Response matrix dimension 2
  unsigned int m_FinalEnergyBins;
  //! Response matrix dimension 3
  unsigned int m_FinalPhiBins;
  //! Response matrix dimension 4
  unsigned int m_FinalDirectionBins;
  //! Response matrix dimension 5
  unsigned int m_FinalElectronDirectionBins;
  //! Response matrix dimension 6
  unsigned int m_FinalDistanceBins;
  
  
  
  //! The data
  MResponseMatrixON m_Data;
  
  //! The stored pointing
  MResponseMatrixON m_Pointing;
  
  //! The number of events:
  unsigned long m_NEvents;
  //! Resonse slices for each event
  vector<MResponseMatrixON> m_EventResponseSlices;
  
  //! A file name for the galactic rotations
  MString m_ResponseGalacticFileName;
  //! The complete response
  MResponseMatrixON m_ResponseGalactic;
  //! A speed up matrix, do calculate the Galactic response faster
  vector<unsigned long> m_GalacticResponseSpeedUpMatrix;
  //! The maximum Compton scatter angle bin:
  unsigned int m_MaxFinalPhiBin;
  
  //! The exposure map
  MResponseMatrixON m_ExposureMap;
  
  //! The observation time
  double m_ObservationTime;
  
  //! The start area
  double m_StartArea;
  
  //! The area of one bin in steradians
  double m_Steradians;
  
  
  bool m_UseBackgroundModel;
  //! File names of the background models
  vector<MString> m_BackgroundModelFileName;
  //! The background model
  vector<MResponseMatrixON> m_BackgroundModel;
  //! The rotated background model in Galactic coordinates
  vector<MResponseMatrixON> m_BackgroundModelGalactic;
  //! The total amount of background in the model
  vector<double> m_TotalBackgroundInModel;
  
  //! Indicating of the threads are still running
  vector<bool> m_ThreadRunning;
  //! The main mutex
  mutex m_ThreadMutex;
  
  //! The longitude shift of all FISBEL images 
  double m_LongitudeShift;
  
  //! Out put file name prefix
  MString m_Prefix;
};

/******************************************************************************/


/******************************************************************************
 * Default constructor
 */
BinnedComptonImaging::BinnedComptonImaging() : m_Interrupt(false)
{
  gStyle->SetPalette(kBird);
 	gROOT->SetBatch(true);

  m_DeconvolutionAlgorithm = 1;
  m_UseBackgroundModel = false;  
  m_JustBuildBackgroundModel = false;
  
  m_WriteFiles = false;
  
  m_LongitudeShift = -180;
  
  m_Prefix = "";
}


/******************************************************************************
 * Default destructor
 */
BinnedComptonImaging::~BinnedComptonImaging()
{
  // Intentionally left blank
}


/******************************************************************************
 * Parse the command line
 */
bool BinnedComptonImaging::ParseCommandLine(int argc, char** argv)
{
  ostringstream Usage;
  Usage<<endl;
  Usage<<"  Usage: BinnedComptonImaging <options>"<<endl;
  Usage<<"    General options:"<<endl;
  Usage<<"         -f:   tra file name"<<endl;
  Usage<<"         -c:   mimrec configuration file"<<endl;
  Usage<<"         -r:   response file"<<endl;
  Usage<<"         -g:   load the galactic rotations"<<endl;
  Usage<<"         -b:   load the pre-created background model (can appear more than once)"<<endl;
  Usage<<"         -i:   number of iterations (default: 5)"<<endl;
  Usage<<"         -a:   algorithm: rl (default) or mem"<<endl;
  Usage<<"         -cb:  create background model"<<endl;
  Usage<<"         -w:   write files"<<endl;
  Usage<<"         -p:   output prefix"<<endl;
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
    Option = argv[i];  //delete EventFile;

    // First check if each option has sufficient arguments:
    // Single argument
    if (Option == "-f" || Option == "-a" || Option == "-i") {
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
    } else if (Option == "-r") {
      m_ResponseFileName = argv[++i];
      cout<<"Accepting response file name: "<<m_ResponseFileName<<endl;
    } else if (Option == "-c") {
      m_MimrecCfgFileName = argv[++i];
      cout<<"Accepting mimrec configuration file name: "<<m_MimrecCfgFileName<<endl;
    } else if (Option == "-b") {
      m_BackgroundModelFileName.push_back(argv[++i]);
      m_UseBackgroundModel = true;
      cout<<"Accepting background model file name: "<<m_BackgroundModelFileName.back()<<endl;
    } else if (Option == "-g") {
      m_ResponseGalacticFileName = argv[++i];
      cout<<"Accepting galactic response file name: "<<m_ResponseGalacticFileName<<endl;
    } else if (Option == "-p") {
      m_Prefix = argv[++i];
      cout<<"Accepting output file name prefix: "<<m_Prefix<<endl;
    } else if (Option == "-i") {
      m_Iterations = atoi(argv[++i]);
      cout<<"Accepting iterations: "<<m_Iterations<<endl;
    } else if (Option == "-a") {
      MString Algo(argv[++i]);
      Algo.ToLowerInPlace();
      if (Algo == "rl") {
        m_DeconvolutionAlgorithm = 1;
        cout<<"Accepting Richardson Lucy"<<endl;
      } else if (Algo == "mem") {
        m_DeconvolutionAlgorithm = 2;
        cout<<"Accepting Maximum-Entropy"<<endl;
      } else {
        cout<<"Error: Unknown algorithm: "<<Algo<<endl;
        return false;
      }
    } else if (Option == "-cb") {
      m_JustBuildBackgroundModel = true;
      cout<<"Accepting to creating a background model"<<endl;
    } else if (Option == "-w") {
      m_WriteFiles = true;
      cout<<"Accepting to write files to disk"<<endl;
    } else {
      cout<<"Error: Unknown option \""<<Option<<"\"!"<<endl;
      cout<<Usage.str()<<endl;
      return false;
    }
  }
  
  if (m_FileName == "") {
    cout<<"Error: Need a simulation file name!"<<endl;
    cout<<Usage.str()<<endl;
    return false;
  }
  
  if (m_ResponseFileName == "") {
    cout<<"Error: Need a response file name!"<<endl;
    cout<<Usage.str()<<endl;
    return false;
  }
  
  if (m_MimrecCfgFileName == "") {
    cout<<"Error: Need a Mimrec configuration file name!"<<endl;
    cout<<Usage.str()<<endl;
    return false;
  }
  
  return true;
}


/******************************************************************************
 * Build the background model from the current data set
 */
bool BinnedComptonImaging::BuildBackgroundModel()
{ 
  mout<<endl<<"Starting to build background model..."<<endl;
  
  MResponseMatrixON Model("Background Model");
  Model.AddAxis(m_Response.GetAxis(2)); // energy
  Model.AddAxis(m_Response.GetAxis(3)); // phi
  Model.AddAxis(m_Response.GetAxis(4)); // direction of scattered gamma ray
  Model.AddAxis(m_Response.GetAxis(5)); // direction of recoil electron
  Model.AddAxis(m_Response.GetAxis(6)); // distance
  
  
  // Read configuration file
  MSettingsMimrec MimrecCfg(false);
  MimrecCfg.Read(m_MimrecCfgFileName);
  MEventSelector EventSelector;
  EventSelector.SetSettings(&MimrecCfg);  
  
  // Open the *.tra file
  MFileEventsTra* EventFile = new MFileEventsTra();
  if (EventFile->Open(m_FileName) == false) return false;
  EventFile->ShowProgress();
  
  
  // Backproject all events:
  cout<<"Filling data space for background model..."<<endl;
  
  float Ei;
  float Phi;
  MVector Dg;
  float Chi;
  float Psi;
  MVector De;
  float Distance;
  
  // Fill the data space
  MPhysicalEvent* Event = 0;
  MComptonEvent* ComptonEvent = 0;
  
  while ((Event = EventFile->GetNextEvent()) != 0) {
    
    if (EventSelector.IsQualifiedEventFast(Event) == false) {
      delete Event;
      continue;
    }
    
    if (Event->GetType() == MPhysicalEvent::c_Compton) {
      ComptonEvent = dynamic_cast<MComptonEvent*>(Event);
      
      Ei = ComptonEvent->Ei();
      Phi = ComptonEvent->Phi()*c_Deg;
      Dg = -ComptonEvent->Dg(); // Invert!
      De = -ComptonEvent->De(); // Invert!
      Distance = ComptonEvent->FirstLeverArm();
      
      if (Phi < 0 || Phi > 60) continue;
      
      Chi = Dg.Phi()*c_Deg;
      while (Chi < 0) Chi += 360.0;
      while (Chi > 360) Chi -= 360.0;
      Psi = Dg.Theta()*c_Deg;
      
      Model.Add(vector<double>{Ei, Phi, Psi, Chi, 0.0, 0.0, Distance}, 1);
    } // if Compton
    
    delete Event;
  }
  
  delete EventFile;
  
  gSystem->ProcessEvents();
  
  MString FileName = m_ResponseFileName;
  FileName.ReplaceAllInPlace("imagingresponse", "backgroundmodel");
  if (FileName == m_ResponseFileName) {
    FileName += ".backgroundmodel.rsp.gz";
  }
  Model.Write(m_Prefix + FileName);
  
  return true;
}


/******************************************************************************
 * Prepare the response:
 * (1) Read
 * (2) Normalize to the ratio of counts detected vs emitted per bin
 */
bool BinnedComptonImaging::PrepareResponse()
{
  // Current layout of imaging response:
  // Image space:
  // (0) energy
  // (1) initial direction in detector coordinates
  // Data space:
  // (2) energy
  // (3) phi
  // (4) direction scattered gamma ray detector coordinates
  // (5) direction recoil electron in detector coordinates
  // (6) distance between interactions
  
  cout<<endl<<"Response preparation: started"<<endl;
  
  // Open the response file - which determines the image data space grid...
  if (m_Response.Read(m_ResponseFileName) == false) {
    mgui<<"Error: Cannot read response file: \""<<m_ResponseFileName<<"\""<<endl;
    return false;
  }
  cout<<"Switching to non-sparse"<<endl;
  m_Response.SwitchToNonSparse();
  
  // Retrieve some infor which will be lost later:
  long Started = m_Response.GetSimulatedEvents();
  m_StartArea = m_Response.GetFarFieldStartArea();
  m_Steradians = 4*c_Pi / m_InitialDirectionBins;
  if (m_StartArea == 0) {
    cout<<"Error no start area given"<<endl;
    return false;
  }
  
  cout<<"Determining maximum phi:"<<endl;
  // Create maximum populated phi-bin:
  
  m_InitialEnergyBins = m_Response.GetAxis(0).GetNumberOfBins();
  m_InitialDirectionBins = m_Response.GetAxis(1).GetNumberOfBins();
  
  m_FinalEnergyBins = m_Response.GetAxis(2).GetNumberOfBins();
  m_FinalPhiBins = m_Response.GetAxis(3).GetNumberOfBins();
  m_FinalDirectionBins = m_Response.GetAxis(4).GetNumberOfBins();
  m_FinalElectronDirectionBins = m_Response.GetAxis(5).GetNumberOfBins();
  m_FinalDistanceBins = m_Response.GetAxis(6).GetNumberOfBins();
  
  unsigned long M1 = 1;
  unsigned long M2 = M1*m_InitialEnergyBins;
  unsigned long M3 = M2*m_InitialDirectionBins;
  unsigned long M4 = M3*m_FinalEnergyBins;
  unsigned long M5 = M4*m_FinalPhiBins;
  unsigned long M6 = M5*m_FinalDirectionBins;
  unsigned long M7 = M6*m_FinalElectronDirectionBins;
  
  m_MaxFinalPhiBin = 0;
  for (unsigned int ie = 0; ie < m_InitialEnergyBins; ++ie) {
    unsigned long B1 = M1*ie;
    for (unsigned int id = 0; id < m_InitialDirectionBins; ++id) {
      unsigned long B2 = B1 + M2*id;
      for (unsigned int fe = 0; fe < m_FinalEnergyBins; ++fe) {
        unsigned long B3 = B2 + M3*fe;
        for (unsigned int fp = 0; fp < m_FinalPhiBins; ++fp) {
          unsigned long B4 = B3 + M4*fp;
          for (unsigned int fd = 0; fd < m_FinalDirectionBins; ++fd) {
            unsigned long B5 = B4 + M5*fd;
            for (unsigned int fed = 0; fed < m_FinalElectronDirectionBins; ++fed) {
              unsigned long B6 = B5 + M6*fed;
              for (unsigned int fdi = 0; fdi < m_FinalDistanceBins; ++fdi) {
                unsigned long B7 = B6 + M7*fdi;
                
                if (m_Response.Get(B7) != 0) {
                  if (fp >= m_MaxFinalPhiBin) m_MaxFinalPhiBin = fp+1; 
                }
              }
            }
          }
        }
      }
    }
  }
  
  cout<<"Maximum phi bin: "<<m_MaxFinalPhiBin<<endl;
  
  /*
  if (m_MaxFinalPhiBin+1 < m_Response.GetAxis(3).GetNumberOfBins()) {
    cout<<"Simplifying response!"<<endl;
    vector<double> Edges = m_Response.GetAxis(3).Get1DBinEdges();
    Edges.resize(m_MaxFinalPhiBin+1);
    
    MResponseMatrixAxis PhiAxis("Phi axis - shrunk");
    PhiAxis.SetBinEdges(Edges);
    
    MResponseMatrixON Response2;
    Response2.SetName("Response - phi shortened");
    Response2.AddAxis(m_Response.GetAxis(0)); // energy
    Response2.AddAxis(m_Response.GetAxis(1)); // image space
    Response2.AddAxis(m_Response.GetAxis(2)); // energy
    Response2.AddAxis(PhiAxis); // phi
    Response2.AddAxis(m_Response.GetAxis(4)); // direction scattered gamma ray 
    Response2.AddAxis(m_Response.GetAxis(5)); // direction recoil electron 
    Response2.AddAxis(m_Response.GetAxis(6)); // distance
    
    m_FinalPhiBins = Response2.GetAxis(3).GetNumberOfBins();

    // Copy the data
    for (unsigned int ie = 0; ie < m_InitialEnergyBins; ++ie) {
      unsigned long B1 = M1*ie;
      for (unsigned int id = 0; id < m_InitialDirectionBins; ++id) {
        unsigned long B2 = B1 + M2*id;
        for (unsigned int fe = 0; fe < m_FinalEnergyBins; ++fe) {
          unsigned long B3 = B2 + M3*fe;
          for (unsigned int fp = 0; fp < m_FinalPhiBins; ++fp) {
            unsigned long B4 = B3 + M4*fp;
            for (unsigned int fd = 0; fd < m_FinalDirectionBins; ++fd) {
              unsigned long B5 = B4 + M5*fd;
              for (unsigned int fed = 0; fed < m_FinalElectronDirectionBins; ++fed) {
                unsigned long B6 = B5 + M6*fed;
                for (unsigned int fdi = 0; fdi < m_FinalDistanceBins; ++fdi) {
                  unsigned long B7 = B6 + M7*fdi;

                  Response2.Set(B7, m_Response.Get(B7));
                }
              }
            }
          }
        }
      }
    }
    
    m_Response = Response2;
  }
  */
  
  //! The number of image sapce bins
  m_IBins = m_InitialEnergyBins * m_InitialDirectionBins;
  //! The number of data space bins
  m_DBins = m_FinalEnergyBins * m_FinalPhiBins * m_FinalDirectionBins * m_FinalElectronDirectionBins * m_FinalDistanceBins;
  //! The total number of repsons ebins
  m_RBins = m_IBins*m_DBins;

  
  // Normalize response:
  // Each bin contains the ratio of detected vs. started photons
  
  // Emitted per bin -- area is constant per bin by definition!
  float Emitted = float(Started/m_InitialDirectionBins);
  
  MTimer T2;
  float MaxRatio = 0;
  if (m_Response.IsSparse() == true) {
    for (unsigned int sb = 0; sb < m_Response.GetNumberOfSparseBins(); ++sb) {
      float Ratio = m_Response.GetSparse(sb) / Emitted;
      m_Response.SetSparse(sb, Ratio);
      if (Ratio > MaxRatio) MaxRatio = Ratio;
    }
  } else {
    for (unsigned int ib = 0; ib < m_IBins; ++ib) {
      for (unsigned long db = 0; db < m_DBins; ++db) {
        float Ratio = m_Response.Get(ib + m_IBins*db) / Emitted;
        m_Response.Set(ib + m_IBins*db, Ratio);
        if (Ratio > MaxRatio) MaxRatio = Ratio;
      }
    }
  }
  cout<<"T2: "<<T2.GetElapsed()<<endl;
  
  
  // Create speed up matrix:
  
  m_GalacticResponseSpeedUpMatrix.resize(m_RBins);
  
  M1 = 1;
  M2 = M1*m_Response.GetAxis(0).GetNumberOfBins();
  M3 = M2*m_Response.GetAxis(1).GetNumberOfBins();
  M4 = M3*m_Response.GetAxis(2).GetNumberOfBins();
  M5 = M4*m_Response.GetAxis(3).GetNumberOfBins();
  M6 = M5*m_Response.GetAxis(4).GetNumberOfBins();
  M7 = M6*m_Response.GetAxis(5).GetNumberOfBins();
  
  unsigned long GalBin = 0;
  for (unsigned int ie = 0; ie < m_InitialEnergyBins; ++ie) {
    unsigned long B1 = M1*ie;
    for (unsigned int id = 0; id < m_InitialDirectionBins; ++id) {
      unsigned long B2 = B1 + M2*id;
      for (unsigned int fe = 0; fe < m_FinalEnergyBins; ++fe) {
        unsigned long B3 = B2 + M3*fe;
        for (unsigned int fp = 0; fp < m_FinalPhiBins; ++fp) {
          unsigned long B4 = B3 + M4*fp;
          for (unsigned int fd = 0; fd < m_FinalDirectionBins; ++fd) {
            unsigned long B5 = B4 + M5*fd;
            for (unsigned int fed = 0; fed < m_FinalElectronDirectionBins; ++fed) {
              unsigned long B6 = B5 + M6*fed;
              for (unsigned int fdi = 0; fdi < m_FinalDistanceBins; ++fdi) {
                unsigned long B7 = B6 + M7*fdi;
                
                m_GalacticResponseSpeedUpMatrix[B7] = GalBin;
              }
            }
          }
        }
      }
    }
  }
  
  
  
  // Create an empty Galactic response with updated Galactic coordinates 
	MResponseMatrixAxisSpheric Gal("Galactic Latitude [deg]", "Galactic Longitude [deg]");
  Gal.SetFISBEL(m_Response.GetAxis(1).GetNumberOfBins(), m_LongitudeShift);
	
	m_ResponseGalactic.SetName("Response Galactic");
  m_ResponseGalactic.AddAxis(m_Response.GetAxis(0)); // energy
  m_ResponseGalactic.AddAxis(Gal); // image space in GALACTIC coordinates
  m_ResponseGalactic.AddAxis(m_Response.GetAxis(2)); // energy
  m_ResponseGalactic.AddAxis(m_Response.GetAxis(3)); // phi
  m_ResponseGalactic.AddAxis(Gal); // direction scattered gamma ray in GALACTIC coordinates
  m_ResponseGalactic.AddAxis(m_Response.GetAxis(5)); // direction recoil electron in GALACTIC coordinates
  m_ResponseGalactic.AddAxis(m_Response.GetAxis(6)); // distance
  m_ResponseGalactic.SwitchToNonSparse();
  
  cout<<"Response preparation: finished"<<endl;
  
  return true;
}


/******************************************************************************
 * Create the response matrix for a single event
 */
MResponseMatrixON BinnedComptonImaging::CreateResponseSliceGalactic(unsigned int LocalDBin, MRotation InverseGalRot)
{  
  MResponseMatrixON Slice;
  Slice.AddAxis(m_Response.GetAxis(0)); // energy
  Slice.AddAxis(m_Response.GetAxis(1)); // image space
  
  for (unsigned int ie = 0; ie < m_InitialEnergyBins; ++ie) {
    for (unsigned int id = 0; id < m_InitialDirectionBins; ++id) {
      unsigned int GalacticIBin = ie + id*m_InitialEnergyBins;
  
      vector<double> GalacticPointing = Slice.GetAxis(1).GetBinCenters(id);
      
      MVector GalacticBinCenter;
      GalacticBinCenter.SetMagThetaPhi(1.0, GalacticPointing[0]*c_Rad, GalacticPointing[1]*c_Rad);
      MVector LocalBinCenter = InverseGalRot*GalacticBinCenter;
      
      double LocalPointingTheta = LocalBinCenter.Theta()*c_Deg;
      double LocalPointingPhi = LocalBinCenter.Phi()*c_Deg;
      
      while (LocalPointingPhi < 0) LocalPointingPhi += 360.0;
      while (LocalPointingPhi > 360) LocalPointingPhi -= 360.0;
      
      unsigned int LocalIBin = ie + Slice.GetAxis(1).GetAxisBin(LocalPointingTheta, LocalPointingPhi)*m_InitialEnergyBins;
      
      Slice.Set(GalacticIBin, m_Response.Get(LocalIBin + m_IBins*LocalDBin));
    }
  }
  
  return Slice;
}


/******************************************************************************
 * Prepare the response:
 * (1) Read the events and prepare the response slices
 */
bool BinnedComptonImaging::PrepareDataSpace()
{
  cout<<endl<<"Data space preparation: started"<<endl;
  
  // Create the data space
  m_Data.SetName("Data");
  m_Data.AddAxis(m_ResponseGalactic.GetAxis(2)); // energy
  m_Data.AddAxis(m_ResponseGalactic.GetAxis(3)); // phi
  m_Data.AddAxis(m_ResponseGalactic.GetAxis(4)); // direction of scattered gamma ray in GALACTIC coordinates
  m_Data.AddAxis(m_ResponseGalactic.GetAxis(5)); // direction of recoil electron in GALACTIC coordinates
  m_Data.AddAxis(m_ResponseGalactic.GetAxis(6)); // distance
  
  // Create the pointing file
  m_Pointing.SetName("Pointing");
  m_Pointing.AddAxis(m_ResponseGalactic.GetAxis(1)); // direction of scattered gamma ray in GALACTIC coordinates
  m_Pointing.AddAxis(m_ResponseGalactic.GetAxis(4)); // direction of scattered gamma ray in GALACTIC coordinates
  
  MResponseMatrixON m_PointingZ;
  m_PointingZ.AddAxis(m_ResponseGalactic.GetAxis(1));
  
  // Read configuration file
  MSettingsMimrec MimrecCfg(false);
  MimrecCfg.Read(m_MimrecCfgFileName);
  MEventSelector EventSelector;
  EventSelector.SetSettings(&MimrecCfg);  
  
  // Open the *.tra file
  MFileEventsTra* EventFile = new MFileEventsTra();
  if (EventFile->Open(m_FileName) == false) return false;
  EventFile->ShowProgress();
  
  float Ei;
  float Phi;
  MVector Dg;
  MVector De;
  float Chi;
  float Psi;
  float Distance;
  
  // Fill the data space
  m_NEvents = 0;
  m_EventResponseSlices.clear();
  
  m_ObservationTime = 0.0;
  MPhysicalEvent* Event = 0;
  MComptonEvent* ComptonEvent = 0;
  
  // The list-mode responses:  
  double PreviousTime = -1;
  vector<double> TimeBetweenEvents;
  while ((Event = EventFile->GetNextEvent()) != 0) {
    
    // TODO: Do time check here, i.e. when we are OFF - for all other time we ahave to do the pointing
    
    // Take care of pointing
    if (PreviousTime != -1) {
      TimeBetweenEvents.push_back(Event->GetTime().GetAsSeconds() - PreviousTime);
    }
    PreviousTime = Event->GetTime().GetAsSeconds();
    
    //cout<<Event->GetId()<<":  X (la/lo): "<<90+ComptonEvent->GetGalacticPointingXAxisLatitude()*c_Deg<<", "<<ComptonEvent->GetGalacticPointingXAxisLongitude()*c_Deg<<"   Z (la/lo): "<<90+ComptonEvent->GetGalacticPointingZAxisLatitude()*c_Deg<<", "<<ComptonEvent->GetGalacticPointingZAxisLongitude()*c_Deg<<endl;
    
    if (Event->HasGalacticPointing() == false) {
      cout<<"Event has no Galactic pointing!"<<endl;
      delete Event;
      continue;        
    }
    
    // Check if we have a good event
    if (EventSelector.IsQualifiedEventFast(Event) == false) {
      delete Event;
      continue;
    }
    
    //! Set the pointing
    m_Pointing.Add(vector<double>{ 90 + Event->GetGalacticPointingXAxisLatitude()*c_Deg, Event->GetGalacticPointingXAxisLongitude()*c_Deg, 90 + Event->GetGalacticPointingZAxisLatitude()*c_Deg, Event->GetGalacticPointingZAxisLongitude()*c_Deg } );
    m_PointingZ.Add(vector<double>{ 90 + Event->GetGalacticPointingZAxisLatitude()*c_Deg, Event->GetGalacticPointingZAxisLongitude()*c_Deg } );

    
    // Add Comptons to the data base
    if (Event->GetType() == MPhysicalEvent::c_Compton) {
      ComptonEvent = dynamic_cast<MComptonEvent*>(Event);
        
      Ei = ComptonEvent->Ei();
      Phi = ComptonEvent->Phi()*c_Deg;
      Dg = -ComptonEvent->Dg(); // Invert!
      De = -ComptonEvent->De(); // Invert!
      Distance = ComptonEvent->FirstLeverArm();
      
      // Limit for the time being
      if (Phi < 0 || Phi > 60) continue;

      Chi = Dg.Phi()*c_Deg;
      while (Chi < 0) Chi += 360.0;
      while (Chi > 360) Chi -= 360.0;
      Psi = Dg.Theta()*c_Deg;

      vector<double> DataLocal = {Ei, Phi, Psi, Chi, 0.0, 0.0, Distance};
      unsigned int DBinLocal = m_Data.FindBin(DataLocal);
      
      MRotation R = ComptonEvent->GetGalacticPointingRotationMatrix();
      Dg = R*Dg;
      De = R*De;
      
      Chi = Dg.Phi()*c_Deg;
      while (Chi < 0) Chi += 360.0;
      while (Chi > 360) Chi -= 360.0;
      Psi = Dg.Theta()*c_Deg;
      
      
      vector<double> DataGal = {Ei, Phi, Psi, Chi, 0.0, 0.0, Distance};
      unsigned int DBinGal = m_Data.FindBin(DataGal);
      
      m_Data.Add(DBinGal, 1);
      
      MRotation IR = ComptonEvent->GetGalacticPointingInverseRotationMatrix();
      
      m_EventResponseSlices.push_back(CreateResponseSliceGalactic(DBinLocal, ComptonEvent->GetGalacticPointingInverseRotationMatrix()));
      
      ++m_NEvents;
      
      if (m_NEvents > 0 && m_NEvents % 500 == 0) cout<<"Processed "<<m_NEvents<<" events"<<endl;
    } // if Compton
    
    m_ObservationTime = Event->GetTime().GetAsSeconds();
    
    delete Event;
    
    /*
    if (m_NEvents == 5000) {
      cout<<"STOP after 5000 events"<<endl;
      break;
    }
    */
  }
  EventFile->ShowProgress(false);
  EventFile->Close();

  gSystem->ProcessEvents();  
  
  cout<<"Obs time: "<<m_ObservationTime<<endl;
  
  cout<<"Collected "<<m_NEvents<<" events."<<endl;
  
  cout<<"Pointing - sum before: "<<m_Pointing.GetSum()<<endl;
  
  // Normalize pointing by time
  cout<<"Normalize pointing..."<<endl;
  if (TimeBetweenEvents.size() < 2) {
    cout<<"ERROR: Not enough events passed the event selections!"<<endl;
    return false;
  }
  sort(TimeBetweenEvents.begin(), TimeBetweenEvents.end());
  double MediumTime = TimeBetweenEvents[TimeBetweenEvents.size()/2];
  cout<<"Medium time: "<<MediumTime<<" & Events: "<<TimeBetweenEvents.size()+1<<endl;
  m_Pointing /= m_Pointing.GetSum();
  
  cout<<"Pointing - sum: "<<m_Pointing.GetSum()<<endl;
  
  if (m_WriteFiles == true) {
    m_Pointing.Write(m_Prefix + "Pointing.rsp.gz");
    m_Data.Write(m_Prefix + "Data.rsp.gz");
  }
  
  cout<<"Data space preparation: finished with "<<m_Data.GetSum()<<" events"<<endl;
  
  return true;
}


/******************************************************************************
 * Parallel response rotation
 */
bool BinnedComptonImaging::RotateResponseInParallel(unsigned int ThreadID, vector<unsigned int> PointingBinsX, vector<unsigned int> PointingBinsZ)
{ 
  
  vector<unsigned long> Bins;
  vector<float> Values;
  
  for (unsigned int b = 0; b < PointingBinsX.size(); ++b) {
    
    cout<<"Thread #"<<ThreadID<<": "<<b+1<<"/"<<PointingBinsX.size()<<endl;
    
    // First check if we have data, otherwise skip
    unsigned int PointingBin = PointingBinsX[b] + PointingBinsZ[b]*m_InitialDirectionBins;
    double PointingScaler = m_Pointing.Get(PointingBin);
    
    // Then create the rotation
    vector<double> XPointing = m_Pointing.GetAxis(0).GetBinCenters(PointingBinsX[b]);
    vector<double> ZPointing = m_Pointing.GetAxis(1).GetBinCenters(PointingBinsZ[b]);
    cout<<XPointing[0]<<":"<<XPointing[1]<<endl;
    cout<<ZPointing[0]<<":"<<ZPointing[1]<<endl;
    
    MRotationInterface RI;
    RI.SetGalacticPointingXAxis(XPointing[1], XPointing[0]-90); // Convert to Galactic 
    RI.SetGalacticPointingZAxis(ZPointing[1], ZPointing[0]-90); // Convert to Galactic 
    MRotation RInv = RI.GetGalacticPointingInverseRotationMatrix(); //  good - 1 sigma width of point source 27.7 deg
    MRotation R = RI.GetGalacticPointingRotationMatrix(); // definitely wrong when looking at an ideal point source
    
    cout<<"R (p): "<<R<<endl;
    
    // Precalculate the rotation map
    // --> Problem: Some map to the same bin...
    vector<unsigned int> RotatedBinMappingGalacticToLocal;
    vector<unsigned int> RotatedBinMappingLocalToGalactic;
    for (unsigned int id = 0; id < m_InitialDirectionBins; ++id) {
      // Calculate the new detector pointing
      vector<double> GalacticPointing = m_ResponseGalactic.GetAxis(1).GetBinCenters(id);
      //cout<<GalacticPointing[0]<<":"<<GalacticPointing[1]<<endl;
      MVector GalIn;
      GalIn.SetMagThetaPhi(1.0, GalacticPointing[0]*c_Rad, GalacticPointing[1]*c_Rad);
      MVector LocalIn = RInv*GalIn;
      
      //cout<<R*GalIn<<" vs. "<<R2*GalIn<<" --> "<<LocalIn.Angle(R2*GalIn)*c_Deg<<endl;
      
      // Find that bin in the new response:
      RotatedBinMappingGalacticToLocal.push_back(m_Response.GetAxis(1).GetAxisBin(LocalIn.Theta()*c_Deg, LocalIn.Phi()*c_Deg));
      
      // cout<<"Mapping: "<<id<<"->"<<RotatedBinMapping[id]<<endl;
      
      vector<double> LocalPointing = m_ResponseGalactic.GetAxis(1).GetBinCenters(id); // Same as above GalacticPointing
      LocalIn.SetMagThetaPhi(1.0, LocalPointing[0]*c_Rad, LocalPointing[1]*c_Rad);
      GalIn = R*LocalIn;
      RotatedBinMappingLocalToGalactic.push_back(m_Response.GetAxis(1).GetAxisBin(GalIn.Theta()*c_Deg, GalIn.Phi()*c_Deg));      
    }
    
    
    // Loop over the new response to make sure that there are no gaps
    // --> Cannot be done in sparse mode
    
    if (m_Response.IsSparse() == true) {
      for (unsigned long sb = 0; sb < m_Response.GetNumberOfSparseBins(); ++sb) {
        vector<unsigned long> Bins = m_Response.FindBinsSparse(sb);
        Bins[1] = RotatedBinMappingLocalToGalactic[Bins[1]];
        Bins[4] = RotatedBinMappingLocalToGalactic[Bins[4]];        
        //Bins[5] = RotatedBinMappingLocalToGalactic[Bins[5]];
        
        unsigned long Bin = m_ResponseGalactic.FindBin(Bins); // this and ...
        float Value = m_Response.GetSparse(sb)*PointingScaler; // this can be parallelized, but not add 
        
        m_ThreadMutex.lock();
        m_ResponseGalactic.Add(Bin, Value); 
        m_ThreadMutex.unlock();
      }
    } else {
      //! Logic: a1 + S1*a2 + S1*S2*a3 + S1*S2*S3*a4 + S1*S2*S3*S4*a5  
      unsigned long M1 = 1;
      unsigned long M2 = M1*m_InitialEnergyBins;
      unsigned long M3 = M2*m_InitialDirectionBins;
      unsigned long M4 = M3*m_FinalEnergyBins;
      unsigned long M5 = M4*m_FinalPhiBins;
      unsigned long M6 = M5*m_FinalDirectionBins;
      unsigned long M7 = M6*m_FinalElectronDirectionBins;
      
      //unsigned long GalBin = 0;
      for (unsigned int ie = 0; ie < m_InitialEnergyBins; ++ie) {
        unsigned long A1 = M1*ie;
        unsigned long B1 = M1*ie;
        for (unsigned int id = 0; id < m_InitialDirectionBins; ++id) {
          unsigned long A2 = A1 + M2*RotatedBinMappingGalacticToLocal[id];
          unsigned long B2 = B1 + M2*id;
          
          for (unsigned int fe = 0; fe < m_FinalEnergyBins; ++fe) {
            unsigned long A3 = A2 + M3*fe;
            unsigned long B3 = B2 + M3*fe;
            for (unsigned int fp = 0; fp < m_MaxFinalPhiBin; ++fp) {
              unsigned long A4 = A3 + M4*fp;
              unsigned long B4 = B3 + M4*fp;
              for (unsigned int fd = 0; fd < m_FinalDirectionBins; ++fd) {
                unsigned long A5 = A4 + M5*RotatedBinMappingGalacticToLocal[fd];
                unsigned long B5 = B4 + M5*fd;
                for (unsigned int fed = 0; fed < m_FinalElectronDirectionBins; ++fed) {
                  unsigned long A6 = A5 + M6*fed; // TODO: <--- the rotation needs to be added here too at some point 
                  unsigned long B6 = B5 + M6*fed;
                  for (unsigned int fdi = 0; fdi < m_FinalDistanceBins; ++fdi) {
                    
                    unsigned long A7 = A6 + M7*fdi;
                    unsigned long B7 = B6 + M7*fdi;
                    
                    double ResponseData = m_Response.Get(A7)*PointingScaler; // Normalize by time in pointing  
                    if (ResponseData == 0) continue;
                    
                    Bins.push_back(B7);
                    Values.push_back(ResponseData);
                    
                    //m_ThreadMutex.lock();
                    //m_ResponseGalactic.Add(B7, ResponseData); 
                    //m_ThreadMutex.unlock();
                  }
                }
              }
            }
          }
          
          // Just try to lock and store the data
          if (m_ThreadMutex.try_lock() == true) {
            m_ResponseGalactic.Add(Bins, Values);
            m_ThreadMutex.unlock();
            Bins.clear();
            Values.clear();
          }
        }
        
        // Definitely store the data here
        m_ThreadMutex.lock();
        m_ResponseGalactic.Add(Bins, Values);
        m_ThreadMutex.unlock();
        Bins.clear();
        Values.clear();
      }
    }
  }
  
  m_ThreadRunning[ThreadID] = false;
  
  return true;
}


/******************************************************************************
 * Create the Galactic response
 */
bool BinnedComptonImaging::CreateGalacticResponse()
{  
  cout<<endl<<"Creation of response in Galactic coordinates: started"<<endl;
  
  MTimer Timer;
  
  
  // Step 0: If we should load it, then just load it
  if (m_ResponseGalacticFileName != "") {
    
    m_ResponseGalactic = MResponseMatrixON();
    
    if (m_ResponseGalactic.Read(m_ResponseGalacticFileName) == false) {
      mgui<<"Cannot read response file: \""<<m_ResponseGalacticFileName<<"\""<<endl;
      return false;
    }
    // Same basic checks:
    if (m_ResponseGalactic.GetNumberOfAxes() != m_Response.GetNumberOfAxes() || m_ResponseGalactic.GetNBins() != m_Response.GetNBins()) {
      mgui<<"The Galactic response read from file and the detector response have not the same dimensions"<<endl;
      return false;
    }
    
    return true;
  }
  
  
  // Step 1: Create a list of directions - pointing bins in this case
  vector<unsigned int> PointingBinsX;
  vector<unsigned int> PointingBinsZ;
  int Dirs = 0;
  // Create a new entry into the Galactic response for each pointing
  for (unsigned int x = 0; x < m_InitialDirectionBins; ++x) {
    for (unsigned int z = 0; z < m_InitialDirectionBins; ++z) {
      //cout<<x<<"/"<<z<<"/"<<InitialDirectionBins<<endl;
      
      // First check if we have data, otherwise skip
      unsigned int PointingBin = x + z*m_InitialDirectionBins;
      if (m_Pointing.Get(PointingBin) == 0) continue;
      Dirs++;
      
      PointingBinsX.push_back(x);
      PointingBinsZ.push_back(z);
    }
  }
  
  // Step 2: Do the actual rotation in parallel
  unsigned int Split = PointingBinsX.size() / std::thread::hardware_concurrency();
  if (Split == 0) {
    m_ThreadRunning.resize(1, true);
    RotateResponseInParallel(0, PointingBinsX, PointingBinsZ);
  } else {
    
    Split += 1; // so that the last thread does not get an excess of pointings
    
    vector<thread> Threads(std::thread::hardware_concurrency());
    m_ThreadRunning.resize(Threads.size(), true);
    for (unsigned int t = 0; t < Threads.size(); ++t) {
      m_ThreadRunning[t] = true;
      vector<unsigned int> X(PointingBinsX.begin() + t*Split, (t == Threads.size() - 1) ? PointingBinsX.end() : PointingBinsX.begin() + (t+1)*Split);
      vector<unsigned int> Z(PointingBinsZ.begin() + t*Split, (t == Threads.size() - 1) ? PointingBinsZ.end() : PointingBinsZ.begin() + (t+1)*Split);
      Threads[t] = thread(&BinnedComptonImaging::RotateResponseInParallel, this, t, X, Z);
    }
    
    while (true) {
      bool Finished = true;
      for (unsigned int t = 0; t < Threads.size(); ++t) {
        if (m_ThreadRunning[t] == true) {
          Finished = false;
          break;
        }
      }
      if (Finished == false) {
        this_thread::sleep_for(chrono::milliseconds(1));
      } else {
        for (unsigned int t = 0; t < Threads.size(); ++t) {
          Threads[t].join();
        }
        break;
      }
    }
  }
  
  // RotateResponseInParallel(PointingBinsX, PointingBinsZ);
  
  if (m_WriteFiles == true) {
    cout<<endl<<"Writing rotated response"<<endl;
    m_ResponseGalactic.Write(m_Prefix + "ResponseGalactic.rsp.gz");
  }
  
  cout<<"Number of directions in pointings file: "<<Dirs<<endl;
  cout<<"Response Galactic normalization: "<<m_ResponseGalactic.GetSum()<<"  vs. "<<m_Response.GetSum()<<endl;
  cout<<endl;
  cout<<"Galactic response created in "<<Timer.GetElapsed()<<" seconds"<<endl;
  
  return true;
}


/******************************************************************************
 * Parallel background model rotation
 */
bool BinnedComptonImaging::RotateBackgroundModelInParallel(unsigned int ThreadID, unsigned int ModelID, vector<unsigned int> PointingBinsX, vector<unsigned int> PointingBinsZ)
{ 
  // For all pointings...

  
  for (unsigned int b = 0; b < PointingBinsX.size(); ++b) {
    
    cout<<"Thread #"<<ThreadID<<": "<<b+1<<"/"<<PointingBinsX.size()<<endl;
    
    // First check if we have data, otherwise skip
    unsigned int PointingBin = PointingBinsX[b] + PointingBinsZ[b]*m_FinalDirectionBins;
    double PointingScaler = m_Pointing.Get(PointingBin);
    
    // Then create the rotation
    vector<double> XPointing = m_Pointing.GetAxis(0).GetBinCenters(PointingBinsX[b]);
    vector<double> ZPointing = m_Pointing.GetAxis(1).GetBinCenters(PointingBinsZ[b]);
    
    MRotationInterface RI;
    RI.SetGalacticPointingXAxis(XPointing[1], XPointing[0]-90); // Convert to Galactic 
    RI.SetGalacticPointingZAxis(ZPointing[1], ZPointing[0]-90); // Convert to Galactic 
    MRotation R = RI.GetGalacticPointingInverseRotationMatrix(); //good
    //MRotation R = RI.GetGalacticPointingRotationMatrix(); // wrong, but too good to believe
    
    
    // Precalculate the rotation map
    // --> Problem: Some map to the same bin...
    vector<unsigned int> RotatedBinMapping;
    for (unsigned int id = 0; id < m_FinalDirectionBins; ++id) {
      // Calculate the new detector pointing
      vector<double> GalacticPointing = m_BackgroundModelGalactic[ModelID].GetAxis(2).GetBinCenters(id);
      //cout<<GalacticPointing[0]<<":"<<GalacticPointing[1]<<endl;
      MVector GalIn;
      GalIn.SetMagThetaPhi(1.0, GalacticPointing[0]*c_Rad, GalacticPointing[1]*c_Rad);
      MVector LocalIn = R*GalIn;
      
      //cout<<R*GalIn<<" vs. "<<R2*GalIn<<" --> "<<LocalIn.Angle(R2*GalIn)*c_Deg<<endl;
      
      // Find that bin in the new response:
      RotatedBinMapping.push_back(m_BackgroundModel[ModelID].GetAxis(2).GetAxisBin(LocalIn.Theta()*c_Deg, LocalIn.Phi()*c_Deg));
      
      // cout<<"Mapping: "<<id<<"->"<<RotatedBinMapping[id]<<endl;
    }
    
    
    // Loop over the new response...
    //! Logic: a1 + S1*a2 + S1*S2*a3 + S1*S2*S3*a4 + S1*S2*S3*S4*a5  
    unsigned long M1 = 1;
    unsigned long M2 = M1*m_FinalEnergyBins;
    unsigned long M3 = M2*m_FinalPhiBins;
    unsigned long M4 = M3*m_FinalDirectionBins;
    unsigned long M5 = M4*m_FinalElectronDirectionBins;
    
    
    for (unsigned int fe = 0; fe < m_FinalEnergyBins; ++fe) {
      unsigned long A1 = M1*fe;
      unsigned long B1 = M1*fe;
      for (unsigned int fp = 0; fp < m_FinalPhiBins; ++fp) {
        unsigned long A2 = A1 + M2*fp;
        unsigned long B2 = B1 + M2*fp;
        for (unsigned int fd = 0; fd < m_FinalDirectionBins; ++fd) {
          unsigned int long A3 = A2 + M3*RotatedBinMapping[fd];
          unsigned int long B3 = B2 + M3*fd;
          for (unsigned int fed = 0; fed < m_FinalElectronDirectionBins; ++fed) {
            unsigned long A4 = A3 + M4*fed;
            unsigned long B4 = B3 + M4*fed;
            for (unsigned int fdi = 0; fdi < m_FinalDistanceBins; ++fdi) {
              unsigned int long A5 = A4 + M5*fdi;
              unsigned int long B5 = B4 + M5*fdi;
              
              double ModelData = m_BackgroundModel[ModelID].Get(A5)*PointingScaler; // Normalize by time in pointing  
              if (ModelData == 0) continue;
              
              m_ThreadMutex.lock();
              m_BackgroundModelGalactic[ModelID].Add(B5, ModelData); 
              m_ThreadMutex.unlock();
            }
          }
        }
      }
    }
    
  }
  
  m_ThreadRunning[ThreadID] = false;
  
  return true;
}


/******************************************************************************
 * Create the Galactic response
 */
bool BinnedComptonImaging::CreateGalacticBackgroundModel()
{  
  
  if (m_UseBackgroundModel == true) {

    cout<<endl<<"Creation of background models in Galactic coordinates: started"<<endl;
    
    // Step 1: Open the files and create all matrices
    m_BackgroundModel.resize(m_BackgroundModelFileName.size());
    m_BackgroundModelGalactic.resize(m_BackgroundModelFileName.size());
    for (unsigned int b = 0; b < m_BackgroundModelFileName.size(); ++b) {
      if (m_BackgroundModel[b].Read(m_BackgroundModelFileName[b]) == false) {
        mgui<<"Error: Cannot read background model file: \""<<m_BackgroundModelFileName[b]<<"\""<<endl;
        return false;
      }
      // Check if the dimensions are identical:
      if (m_Response.GetAxis(2) != m_BackgroundModel[b].GetAxis(0) ||
          m_Response.GetAxis(3) != m_BackgroundModel[b].GetAxis(1) ||
          m_Response.GetAxis(4) != m_BackgroundModel[b].GetAxis(2) ||
          m_Response.GetAxis(5) != m_BackgroundModel[b].GetAxis(3) ||
          m_Response.GetAxis(6) != m_BackgroundModel[b].GetAxis(4)) {
       mgui<<"Error: The response and background model axes arre not identical"<<endl;
        return false;
      }
      
			MResponseMatrixAxisSpheric A("Galactic Latitude [deg]", "Galactic Longitude [deg]");
			A.SetFISBEL(m_BackgroundModel[b].GetAxis(2).GetNumberOfBins(), m_LongitudeShift);


      m_BackgroundModelGalactic[b].SetName(MString("Background model in Galactic coordiantes #") + (b+1));
      m_BackgroundModelGalactic[b].AddAxis(m_BackgroundModel[b].GetAxis(0)); // energy
      m_BackgroundModelGalactic[b].AddAxis(m_BackgroundModel[b].GetAxis(1)); // phi
      m_BackgroundModelGalactic[b].AddAxis(A); // direction of scattered gamma ray IN GALACTIC coordinates
      m_BackgroundModelGalactic[b].AddAxis(A); // direction of recoil electron IN GALACTIC coordinates
      m_BackgroundModelGalactic[b].AddAxis(m_BackgroundModel[b].GetAxis(4)); // distance
    }
    
    
    m_TotalBackgroundInModel.resize(m_BackgroundModel.size());
    for (double& Total: m_TotalBackgroundInModel) Total = 0;
    
    // Step 2 Create a list of directions - pointing bins in this case
    vector<unsigned int> PointingBinsX;
    vector<unsigned int> PointingBinsZ;
    int Dirs = 0;
    // Create a new entry into the Galactic response for each pointing
    for (unsigned int x = 0; x < m_InitialDirectionBins; ++x) {
      for (unsigned int z = 0; z < m_InitialDirectionBins; ++z) {
        //cout<<x<<"/"<<z<<"/"<<InitialDirectionBins<<endl;
        
        // First check if we have data, otherwise skip
        unsigned int PointingBin = x + z*m_InitialDirectionBins;
        if (m_Pointing.Get(PointingBin) == 0) continue;
        Dirs++;
        
        PointingBinsX.push_back(x);
        PointingBinsZ.push_back(z);
      }
    }
    
    // Step 3: Do the actual rotation into Galactic coordiantes in parallel
    unsigned int Split = PointingBinsX.size() / std::thread::hardware_concurrency();
    for (unsigned int b = 0; b < m_BackgroundModel.size(); ++b) {
      cout<<"   ... background model #"<<b+1<<endl;
      if (Split == 0) {
        m_ThreadRunning.resize(1, true);
        RotateBackgroundModelInParallel(0, b, PointingBinsX, PointingBinsZ);
      } else {
        vector<thread> Threads(std::thread::hardware_concurrency());
        m_ThreadRunning.resize(Threads.size(), true);
        for (unsigned int t = 0; t < Threads.size(); ++t) {
          m_ThreadRunning[t] = true;
          vector<unsigned int> X(PointingBinsX.begin() + t*Split, (t == Threads.size() - 1) ? PointingBinsX.end() : PointingBinsX.begin() + (t+1)*Split);
          vector<unsigned int> Z(PointingBinsZ.begin() + t*Split, (t == Threads.size() - 1) ? PointingBinsZ.end() : PointingBinsZ.begin() + (t+1)*Split);
          Threads[t] = thread(&BinnedComptonImaging::RotateBackgroundModelInParallel, this, t, b, X, Z);
        }
        while (true) {
          bool Finished = true;
          for (unsigned int t = 0; t < Threads.size(); ++t) {
            if (m_ThreadRunning[t] == true) {
              Finished = false;
              break;
            }
          }
          if (Finished == false) {
            this_thread::sleep_for(chrono::milliseconds(1));
          } else {
            for (unsigned int t = 0; t < Threads.size(); ++t) {
              Threads[t].join();
            }
            break;
          }
        }
      }
      
      // Normalize to 1.0:
      m_BackgroundModelGalactic[b] /= m_BackgroundModelGalactic[b].GetSum();
    }
        
    if (m_WriteFiles == true) {
      cout<<"Writing rotated Galactic background"<<endl;
      for (unsigned int b = 0; b < m_BackgroundModel.size(); ++b) { 
        m_BackgroundModelGalactic[b].Write(m_Prefix + MString("BackgroundModelGalactic_model") + b + ".rsp.gz");
      }
    }
    for (unsigned int b = 0; b < m_BackgroundModel.size(); ++b) { 
      m_TotalBackgroundInModel[b] = m_BackgroundModelGalactic[b].GetSum();
    }
  }  
  
  return true;
}


/******************************************************************************
 * Create an exposure map
 */
bool BinnedComptonImaging::CreateExposureMap()
{
  m_ExposureMap.SetName("ExposureMap");
  m_ExposureMap.AddAxis(m_ResponseGalactic.GetAxis(0)); // energy
  m_ExposureMap.AddAxis(m_ResponseGalactic.GetAxis(1)); // image space
  
  
  for (unsigned int ib = 0; ib < m_IBins; ++ib) {
    float Sum = 0.0;      
    for (unsigned long db = 0; db < m_DBins; ++db) {
      Sum += m_ResponseGalactic.Get(ib + m_IBins*db);
    }
    m_ExposureMap.Set(ib, Sum * m_ObservationTime * m_StartArea);
  }
  
  if (m_WriteFiles == true) {
    cout<<"Writing exposure map"<<endl;
    m_ExposureMap.Write(m_Prefix + "ExposureMap.rsp.gz");
  }
  
  ShowImageGalacticCoordinates(m_ExposureMap, "Exposure", "cm^2 * sec", true, m_Prefix + "Exposure");
  
  return true;
}


/******************************************************************************
 * Reconstruct the image in RL mode
 */
bool BinnedComptonImaging::ReconstructRL()
{
  // Create an initial backprojection
  cout<<"Creating initial backprojection..."<<endl;
  
  MResponseMatrixON Image("Image");
  Image.AddAxis(m_ResponseGalactic.GetAxis(0)); // energy
  Image.AddAxis(m_ResponseGalactic.GetAxis(1)); // image space

  MTimer T;
  
  for (unsigned int ib = 0; ib < m_IBins; ++ib) {
    float Content = 0.0;
    for (unsigned long e = 0; e < m_NEvents; ++e) {
      Content += m_EventResponseSlices[e].Get(ib);
    }
    Image.Set(ib, Content);
  }
  cout<<"Done: "<<T.GetElapsed()<<endl;
  
  
  // Check if content is reasonable
  double Sum = Image.GetSum();
  if (Sum == 0) {
    cout<<"Error: First backprojection sum image is zero"<<endl;
    return false;
  }
  if (isfinite(Sum) == false) {
    cout<<"Error: First backprojection image contains non-finite number."<<endl;
    return false;
  }

  // Show & write
  if (m_WriteFiles == true) {
    Image.Write(m_Prefix + "FirstBackprojection.rsp.gz");
  }

  ShowImageGalacticCoordinates(Image, "1st backprojection", "[a.u.]");
  
  // Assume that all data is background:
  vector<double> BackgroundScaler(m_BackgroundModel.size(), 0.0);
  if (m_UseBackgroundModel == true) {
    for (unsigned long db = 0; db < m_DBins; ++db) {
      for (unsigned int b = 0; b < m_BackgroundModel.size(); ++b) {
        BackgroundScaler[b] += m_BackgroundModelGalactic[b].Get(db);
      }
    }
    
    for (unsigned int b = 0; b < m_BackgroundModel.size(); ++b) {
      BackgroundScaler[b] *= m_Data.GetSum()/BackgroundScaler[b];
      cout<<"Starting background scaler: "<<BackgroundScaler[b]<<endl;
    }
  }
  
  // The iterations
  MResponseMatrixON Mean;
  Mean.AddAxis(m_ResponseGalactic.GetAxis(2)); // energy
  Mean.AddAxis(m_ResponseGalactic.GetAxis(3)); // phi
  Mean.AddAxis(m_ResponseGalactic.GetAxis(4)); // direction scattered gamma ray  
  Mean.AddAxis(m_ResponseGalactic.GetAxis(5)); // direction recoil electron 
  Mean.AddAxis(m_ResponseGalactic.GetAxis(6)); // distance  
  
  for (unsigned int i = 0; i < m_Iterations; ++i) {
    cout<<endl<<"Iteration: "<<i+1<<" - convolve"<<endl;
    
    // Convolve:
    for (unsigned long db = 0; db < m_DBins; ++db) {
      float NewMean = 0;
      
      for (unsigned int ib = 0; ib < m_IBins; ++ib) {
        NewMean += Image.Get(ib) * m_ResponseGalactic.Get(ib + m_IBins*db);
      }
      
      if (m_UseBackgroundModel == true) {
        for (unsigned int b = 0; b < m_BackgroundModel.size(); ++b) {
          NewMean += BackgroundScaler[b] * m_BackgroundModelGalactic[b].Get(db);
        }
      }
      
      Mean.Set(db, NewMean);
    }    
    
    /*
    if (m_WriteFiles == true) {
      Mean.Write(MString("Mean_") + (i+1) + ".rsp");
    }
    */
    
    cout<<"Iteration: "<<i+1<<" - deconvolve"<<endl;
    

    // Now deconvolve the background scaling factor first :
    if (m_UseBackgroundModel == true) {
      vector<double> B_corr(m_BackgroundModel.size(), 0);
      
      for (unsigned long db = 0; db < m_DBins; ++db) {
        if (m_Data.Get(db) > 0 && Mean.Get(db) > 0) {
          for (unsigned int m = 0; m < m_BackgroundModel.size(); ++m) {
            B_corr[m] += m_BackgroundModelGalactic[m].Get(db) * m_Data.Get(db) / Mean.Get(db);
          }
        }      
      }
      
      for (unsigned int m = 0; m < m_BackgroundModel.size(); ++m) {
        BackgroundScaler[m] *= B_corr[m] / m_TotalBackgroundInModel[m];
        cout<<"Background scaling factor: "<<BackgroundScaler[m]<<"!"<<B_corr[m]<<"!"<<m_TotalBackgroundInModel[m]<<endl;
      }
    }
    
    
    double ImageFlux = 0.0;
    
    for (unsigned int ib = 0; ib < m_IBins; ++ib) {
      float Content = 0.0;
      float Sum = 0.0;      
      for (unsigned long db = 0; db < m_DBins; ++db) {
        if (Mean.Get(db) > 0) {
          Content += m_ResponseGalactic.Get(ib + m_IBins*db) * m_Data.Get(db) / Mean.Get(db);
        }
        Sum += m_ResponseGalactic.Get(ib + m_IBins*db);
      }
      if (Sum > 0) {
        Image.Set(ib, Content * Image.Get(ib) / Sum);
        ImageFlux += (Content * Image.Get(ib) / Sum) / m_ObservationTime / m_StartArea;
      }
    }
    
    cout<<"Sums:  mean="<<Mean.GetSum()<<" image="<<Image.GetSum()<<"  data="<<m_Data.GetSum()<<endl;
    cout<<"Image content: "<<ImageFlux<<" ph/cm2/s for T="<<m_ObservationTime<<" sec and A="<<m_StartArea<<" cm^2"<<endl;
    
    ShowImageGalacticCoordinates(Image, MString("RL image at iteration ") + (i+1) + " with flux " + ImageFlux + " ph/cm2/s", "Flux", true, m_Prefix + MString("RL image at iteration ") + (i+1));
    
    if (m_Interrupt == true) break;
  }  

  return true;
}


/******************************************************************************
 * Reconstruct the image in MEM mode
 */
bool BinnedComptonImaging::ReconstructMEM()
{
    // Set up Lagrange multipliers
  
  MResponseMatrixON Lagrange(false);
  Lagrange.AddAxis(m_ResponseGalactic.GetAxis(2)); // energy
  Lagrange.AddAxis(m_ResponseGalactic.GetAxis(3)); // phi
  Lagrange.AddAxis(m_ResponseGalactic.GetAxis(4)); // direction scattered gamma ray  
  Lagrange.AddAxis(m_ResponseGalactic.GetAxis(5)); // direction recoil electron 
  Lagrange.AddAxis(m_ResponseGalactic.GetAxis(6)); // distance  
  
  for (unsigned long db = 0; db < m_DBins; ++db) {
    Lagrange.Set(db, m_Data.Get(db));
  }
  
  // Normalize response
  //m_ResponseGalactic /= m_ResponseGalactic.GetSum(); // TODO: Why do we do that????
  
  
  double DataSum = m_Data.GetSum();
  cout<<"Data Sum="<<DataSum<<endl;   
  
  // The expectations
  MResponseMatrixON Expectation(false);
  Expectation.AddAxis(m_ResponseGalactic.GetAxis(2)); // energy
  Expectation.AddAxis(m_ResponseGalactic.GetAxis(3)); // phi
  Expectation.AddAxis(m_ResponseGalactic.GetAxis(4)); // diretcion scattered gamma ray  
  Expectation.AddAxis(m_ResponseGalactic.GetAxis(5)); // direction recoil electron 
  Expectation.AddAxis(m_ResponseGalactic.GetAxis(6)); // distance  
  
  vector<double> BackgroundScaler(m_BackgroundModel.size(), 1.0);
  
  double MaximumEntropy = 0;
  
  for (unsigned int i = 0; i < m_Iterations; ++i) {
    
    // Step 1: Calculate the restored image:
    
    MResponseMatrixON RestoredImage("RestoredImage", false);
    RestoredImage.AddAxis(m_ResponseGalactic.GetAxis(0)); // energy
    RestoredImage.AddAxis(m_ResponseGalactic.GetAxis(1)); // image space      
    
    double RestoredImageSum = 0;
    
    for (unsigned int ib = 0; ib < m_IBins; ++ib) {
      float Content = 0.0;    
      for (unsigned long db = 0; db < m_DBins; ++db) {
        Content += m_ResponseGalactic.Get(ib + m_IBins*db) * Lagrange.Get(db);
      }
      Content = exp(Content);
      RestoredImage.Set(ib, Content);
      RestoredImageSum += Content;
    }    

    if (RestoredImageSum == 0) {
      cerr<<"ERROR: RestoredImageSum == 0"<<endl;
      return false;
    }
    if (std::isinf(RestoredImageSum)) {
      cerr<<"ERROR: RestoredImageSum == inf"<<endl;
      return false;
    }
    cout<<"RestoredImageSum="<<RestoredImageSum<<endl;
    
    
    double Entropy = 0;
    for (unsigned int ib = 0; ib < m_IBins; ++ib) {
      float RestoredImageUpdate = RestoredImage.Get(ib)*DataSum/RestoredImageSum;
      RestoredImage.Set(ib, RestoredImageUpdate);
      Entropy += RestoredImageUpdate*log(RestoredImageUpdate);
    }      
    cout<<"Entropy: "<<Entropy<<endl;    
    
    
    
    // Step 2: Convolve the data again into data space - expectation calculation
    for (unsigned long db = 0; db < m_DBins; ++db) {
      float NewExpectation = 0;
      for (unsigned int ib = 0; ib < m_IBins; ++ib) {
        NewExpectation += RestoredImage.Get(ib) * m_ResponseGalactic.Get(ib + m_IBins*db);
      }
      
      if (m_UseBackgroundModel == true) {
        for (unsigned int b = 0; b < m_BackgroundModel.size(); ++b) {
          NewExpectation += BackgroundScaler[b] * m_BackgroundModelGalactic[b].Get(db);
        }
      }
      
      Expectation.Set(db, NewExpectation);
    }    
    
    double ExpectationSum = Expectation.GetSum();
    if (ExpectationSum == 0) {
      cerr<<"ERROR: ExpectationSum == 0"<<endl;
      return false;
    }
    cout<<"Expectation Sum="<<ExpectationSum<<endl;
    
    
    
    // Step 3: Update Lagrange multipliers
    double Scale = ExpectationSum/DataSum;
    
    if (Scale == 0) {
      cerr<<"ERROR: Scale == 0"<<endl;
      break;
    }
    
    int LCounter = 0; 
    double Limit = 1.0E-9;
    double UpdateFactor = 5000;
    
    for (unsigned long db = 0; db < m_DBins; ++db) {
      //if (m_Data.Get(db) == 0) continue;
      bool UseLog = false;
      double Update = 0;
      if (UseLog == true) {
        double PreLog1 = m_Data.Get(db)*Scale;
        if (PreLog1 < Limit) PreLog1 = Limit;
        Update += log(PreLog1);
        
        double PreLog2 = Expectation.Get(db); 
        if (PreLog2 < Limit) PreLog2 = Limit;
        Update -= log(PreLog2);
        
        cout<<"Lagrange diff: "<<Lagrange.Get(db)<<" vs. "<<m_Data.Get(db)*Scale<<" vs. "<<Expectation.Get(db)<<" --> Update: "<<Update<<endl;
        
      } else {
        Update += m_Data.Get(db)*Scale - Expectation.Get(db);
        //cout<<"Lagrange diff: "<<Lagrange.Get(db)<<" vs. "<<Data.Get(db)*Scale<<" vs. "<<Expectation.Get(db)<<" --> Update: "<<Update<<endl;
      }
      
      //cout<<"Lagrange diff: "<<Lagrange.Get(db)<<" vs. "<<Update<<endl;
      double NewL = Lagrange.Get(db) + UpdateFactor*Update;
      /*
      if (NewL < 0) {
        NewL = 0;
      }
      */
      Lagrange.Set(db, NewL);
      LCounter++;      
    }

    cout<<" Updated Lagrange entries: "<<LCounter<<endl;
    cout<<"Lagrange sum="<<Lagrange.GetSum()<<endl;
    
    
    
    // Step 4: Update the background scaling factor:
    if (m_UseBackgroundModel == true) {
      vector<double> B_corr(m_BackgroundModel.size(), 0.0);
      
      for (unsigned long db = 0; db < m_DBins; ++db) {
        
        if (m_Data.Get(db) > 0 && Expectation.Get(db) > 0) {
          for (unsigned int b = 0; b < m_BackgroundModel.size(); ++b) {
            B_corr[b] += m_BackgroundModelGalactic[b].Get(db) * m_Data.Get(db) / Expectation.Get(db);
          }
        }
      }
      
      for (unsigned int b = 0; b < m_BackgroundModel.size(); ++b) {
        BackgroundScaler[b] *= B_corr[b] / m_TotalBackgroundInModel[b];
        cout<<"Background scaling factor: "<<BackgroundScaler[b]<<"!"<<B_corr[b]<<"!"<<m_TotalBackgroundInModel[b]<<endl;
      }
    }
    
    
    // Step 5: Update the real image:
    MResponseMatrixON NewImage("NewImage", false);
    NewImage.AddAxis(m_ResponseGalactic.GetAxis(0)); // energy
    NewImage.AddAxis(m_ResponseGalactic.GetAxis(1)); // image space
    
    double ImageFlux = 0;
    for (unsigned int ib = 0; ib < m_IBins; ++ib) {
      NewImage.Set(ib, RestoredImage.Get(ib) * DataSum/RestoredImageSum);
      ImageFlux += NewImage.Get(ib) / m_ObservationTime / m_StartArea;      
    }
    cout<<"Image content: "<<ImageFlux<<" ph/cm2/s"<<endl;
    
    if (Entropy > MaximumEntropy) {
      MaximumEntropy = Entropy;
    }
    
    ShowImageGalacticCoordinates(NewImage, MString("MEM image at iteration ") + (i+1), "Flux", true, m_Prefix + MString("MEM image at iteration ") + (i+1));
    //ShowImageGalacticCoordinates(NewImage, MString("MEM image at iteration ") + (i+1) + " with flux " + ImageFlux + " ph/cm2/s", "Flux", true, MString("MEM image at iteration ") + (i+1));
    
    if (m_Interrupt == true) break;
    
  } // iterations
  
  return true;
}


/******************************************************************************
 * Reconstruct the image
 */
bool BinnedComptonImaging::Reconstruct()
{
  // Prepare the response and all helper information
  if (PrepareResponse() == false) {
    return false;
  }
  
  // Check if we have to build a background model
  if (m_JustBuildBackgroundModel == true) {
    BuildBackgroundModel();
    exit(0);
  }  
  
  // Fill data space / create the response slices
  if (PrepareDataSpace() == false) {
    return false;
  }
  
  // Create the rotated Galactic response
  if (CreateGalacticResponse() == false) {
    return false;
  }
  
  // Create the rotated Galactic response
  if (CreateGalacticBackgroundModel() == false) {
    return false;
  }
  
  // Create the exposure map
  if (CreateExposureMap() == false) {
     return false; 
  }
  
  // Create image
  if (m_DeconvolutionAlgorithm == 1) {
    ReconstructRL(); 
  } else {
    ReconstructMEM();
  }
  
  return true;
}


/******************************************************************************
 * Show an image in Galactic coordinates
 */
bool BinnedComptonImaging::ShowImageGalacticCoordinates(MResponseMatrixON Image, MString Title, MString zAxis, bool Save, MString SaveTitle)
{
  vector<double> ImageData(Image.GetAxis(1).GetNumberOfBins());
  for (unsigned int ib = 0; ib < m_IBins; ++ib) {
    ImageData[ib] = Image.Get(ib);
  }
  
  MImageGalactic* G = new MImageGalactic();
  G->SetTitle(Title);
  G->SetXAxisTitle("Galactic Longitude [deg]");
  G->SetYAxisTitle("Galactic Latitude [deg]");
  G->SetValueAxisTitle(zAxis);
  G->SetDrawOption(MImage::c_COL);
  G->SetSpectrum(MImage::c_Rainbow);
  //G->SetSourceCatalog("$(MEGALIB)/resource/catalogs/Crab.scat");
  //G->SetProjection(MImageProjection::c_Hammer);
  G->Normalize(false);
  G->SetFISBEL(ImageData, m_LongitudeShift);
  G->Display();
  
  if (Save == true) {
    MString Save = SaveTitle;
		Save += ".C";
		Save.ReplaceAllInPlace(" ", "_");
		G->SaveAs(Save);
    Save = SaveTitle;
		SaveTitle += ".eps";
		SaveTitle.ReplaceAllInPlace(" ", "_");
		G->SaveAs(SaveTitle);
    Save = SaveTitle;
		SaveTitle += ".png";
		SaveTitle.ReplaceAllInPlace(" ", "_");
		G->SaveAs(SaveTitle);
  }
  
  gSystem->ProcessEvents();
  
  return true;
}


/******************************************************************************/

BinnedComptonImaging* g_Prg = 0;
int g_NInterrupts = 2;

/******************************************************************************/


/******************************************************************************
 * Called when an interrupt signal is flagged
 * All catched signals lead to a well defined exit of the program
 */
void CatchSignal(int a)
{
  cout<<"Catched signal Ctrl-C:"<<endl;
  
  --g_NInterrupts;
  if (g_NInterrupts <= 0) {
    cout<<"Aborting..."<<endl;
    abort();
  } else {
    cout<<"Trying to cancel the analysis..."<<endl;
    if (g_Prg != 0) {
      g_Prg->Interrupt();
    }
    cout<<"If you hit "<<g_NInterrupts<<" more times, then I will abort immediately!"<<endl;
  }
}


/******************************************************************************
 * Main program
 */
int main(int argc, char** argv)
{
  // Set a default error handler and catch some signals...
  signal(SIGINT, CatchSignal);
  
  // Initialize global MEGAlib variables, especially mgui, etc.
  MGlobal::Initialize("Binned imaging test application");
  
  TApplication BinnedComptonImagingApp("BinnedComptonImagingApp", 0, 0);

  g_Prg = new BinnedComptonImaging();

  if (g_Prg->ParseCommandLine(argc, argv) == false) {
    cerr<<"Error during parsing of command line!"<<endl;
    return -1;
  } 
  if (g_Prg->Reconstruct() == false) {
    cerr<<"Error during analysis!"<<endl;
    return -2;
  } 

  //BinnedComptonImagingApp.Run();

  cout<<"Program exited normally!"<<endl;

  return 0;
}

/*
 * Cosima: the end...
 ******************************************************************************/
