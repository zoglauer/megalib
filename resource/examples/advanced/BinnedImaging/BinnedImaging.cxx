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
#include "MEventSelector.h"
#include "MSettingsMimrec.h"
#include "MImageGalactic.h"
#include "MResponseMatrixAxisSpheric.h"


/******************************************************************************/

class BinnedComptonImaging
{
public:
  /// Default constructor
  BinnedComptonImaging();
  /// Default destructor
  ~BinnedComptonImaging();
  
  /// Parse the command line
  bool ParseCommandLine(int argc, char** argv);
  /// Analyze whatever needs to be analyzed...
  bool Analyze();
  /// Analyze the event, return true if it has to be writen to file
  bool AnalyzeEvent(MSimEvent& Event);
  /// Interrupt the analysis
  void Interrupt() { m_Interrupt = true; }

  /// Show spectra per detector
  bool Spectra();

protected:
  /// Parallel response rotation
  bool RotateResponseInParallel(unsigned int ThreadID, vector<unsigned int> PointingBinsX, vector<unsigned int> PointingBinsZ);  
  ///Parallel background model rotation
  bool RotateBackgroundModelInParallel(unsigned int ThreadID, unsigned int Model, vector<unsigned int> PointingBinsX, vector<unsigned int> PointingBinsZ);

  /// Create the background model
  bool CreateBackgroundModel();
  
  /// Create the list mode response
  MResponseMatrixON* CreateListModeResponse(MComptonEvent* C);
  
  
private:
  /// True, if the analysis needs to be interrupted
  bool m_Interrupt;

  //! Just create the background model
  bool m_JustDoBackgroundModel;
  //! Just create the galactic response file
  bool m_JustDoGalacticResponse;
  
  //! Use hybrid mode
  bool m_UseHybridMode;
  
  //! Perform reorientations
  bool m_ReOrient;
  
  /// Simulation file name
  MString m_FileName;
  /// Mimrec configuration file name
  MString m_MimrecCfgFileName;
  /// Response file name
  MString m_ResponseFileName;
  /// Response file name
  vector<MString> m_BackgroundModelFileName;
  /// Response file name
  MString m_ResponseGalacticFileName;
  /// The number of iterations the algorithm has to run
  unsigned int m_Iterations;
  
  /// True if we do MaxEnt, MaxLikelihood otherwise
  bool m_UseMaximumEntropy;
  
  /// Indicating of the threads are still running
  vector<bool> m_ThreadRunning;
  /// The main mutex
  mutex m_ThreadMutex;
  
  //! The stored pointings
  MResponseMatrixON m_Pointing;
  //! The stored response
  MResponseMatrixON m_Response;
  //! The rotated response in Galactic coordinates
  MResponseMatrixON m_ResponseGalactic;
  
  //! The background model
  vector<MResponseMatrixON> m_BackgroundModel;
  //! The rotated background model in Galactic coordinates
  vector<MResponseMatrixON> m_BackgroundModelGalactic;
  
  //! The number of bins in list mode
  unsigned int m_ListModeBins;
};

/******************************************************************************/


/******************************************************************************
 * Default constructor
 */
BinnedComptonImaging::BinnedComptonImaging() : m_Interrupt(false)
{
  m_Iterations = 5;
  
  m_JustDoBackgroundModel = false;
  m_JustDoGalacticResponse = false;
  
  m_UseMaximumEntropy = false;
  gStyle->SetPalette(kBird);
  
  m_UseHybridMode = false;
  m_ListModeBins = 10*1650;
  
  m_ReOrient = true;
}


/******************************************************************************
 * Default destructor
 */
BinnedComptonImaging::~BinnedComptonImaging()
{
  // Intentionally left blanck
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
  Usage<<"         -g:   response galactic file"<<endl;
  Usage<<"         -b:   background model (can be more than 1)"<<endl;
  Usage<<"         -i:   number of iterations (default: 5)"<<endl;
  Usage<<"         -a:   algorithm: rl (default) or mem"<<endl;
  Usage<<"         -cg:  create galactic response file"<<endl;
  Usage<<"         -cb:  create background model"<<endl;
  Usage<<"         -hy:  use hybrid mode"<<endl;
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
    } else if (Option == "-g") {
      m_ResponseGalacticFileName = argv[++i];
      cout<<"Accepting response galactic file name: "<<m_ResponseGalacticFileName<<endl;
    } else if (Option == "-b") {
      m_BackgroundModelFileName.push_back(argv[++i]);
      cout<<"Accepting background model file name: "<<m_BackgroundModelFileName.back()<<endl;
    } else if (Option == "-c") {
      m_MimrecCfgFileName = argv[++i];
      cout<<"Accepting mimrec configuration file name: "<<m_MimrecCfgFileName<<endl;
    } else if (Option == "-i") {
      m_Iterations = atoi(argv[++i]);
      cout<<"Accepting iterations: "<<m_Iterations<<endl;
    } else if (Option == "-a") {
      MString Algo(argv[++i]);
      Algo.ToLowerInPlace();
      if (Algo == "rl") {
        m_UseMaximumEntropy = false;
        cout<<"Accepting Richardson Lucy"<<endl;
      } else if (Algo == "mem") {
        m_UseMaximumEntropy = true;
        cout<<"Accepting Maximum-Entropy"<<endl;
      } else {
        cout<<"Error: Unknown algorithm: "<<Algo<<endl;
        return false;
      }
    } else if (Option == "-cb") {
      m_JustDoBackgroundModel = true;
      cout<<"Doing background model"<<endl;
    } else if (Option == "-cg") {
      m_JustDoGalacticResponse = true;
      cout<<"Doing galactic response"<<endl;
    } else if (Option == "-cg") {
      m_JustDoGalacticResponse = true;
      cout<<"Doing galactic response"<<endl;
    } else if (Option == "-hy") {
      m_UseHybridMode = true;
      cout<<"Using hybrid mode"<<endl;
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
  
  //
  if (m_JustDoBackgroundModel == true) {
    CreateBackgroundModel(); 
    return false;
  }
  
  return true;
}


/******************************************************************************
 * Create the background model
 */
bool BinnedComptonImaging::CreateBackgroundModel()
{ 
  // Open the response file - which determines the data space grid...
  if (m_Response.Read(m_ResponseFileName) == false) {
    mgui<<"Cannot read response file: \""<<m_ResponseFileName<<"\""<<endl;
    return false;
  }
  
  MResponseMatrixON Data("Data");
  Data.AddAxis(m_Response.GetAxis(2)); // energy
  Data.AddAxis(m_Response.GetAxis(3)); // phi
  Data.AddAxis(m_Response.GetAxis(4)); // direction of scattered gamma ray
  
  
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
  cout<<"Filling data space..."<<endl;
  
  float Ei;
  float Phi;
  MVector Dg;
  float Chi;
  float Psi;
  
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
      
      
      if (Phi < 0 || Phi > 60) continue;
      
      Chi = Dg.Phi()*c_Deg;
      while (Chi < 0) Chi += 360.0;
      while (Chi > 360) Chi -= 360.0;
      Psi = Dg.Theta()*c_Deg;
      
      //cout<<Event->GetId()<<": "<<Phi<<":"<<Psi<<": "<<Chi<<endl;
      
      Data.Add(vector<double>{Ei, Phi, Psi, Chi}, 1);
    } // if Compton
    
    delete Event;
  }
  
  delete EventFile;
  
  gSystem->ProcessEvents();

  MString FileName = m_ResponseFileName;
  FileName.ReplaceAllInPlace("imagingresponse", "backgroundmodel");
  if (FileName == m_ResponseFileName) {
    FileName += ".backgroundmodel.rsp";
  }
  Data.Write(FileName);
  
  return true;
}


/******************************************************************************
 * Parallel response rotation
 */
bool BinnedComptonImaging::RotateResponseInParallel(unsigned int ThreadID, vector<unsigned int> PointingBinsX, vector<unsigned int> PointingBinsZ)
{ 
  // Dimensions for later:
  unsigned int InitialEnergyBins = m_Response.GetAxis(0).GetNumberOfBins();
  unsigned int InitialDirectionBins = m_Response.GetAxis(1).GetNumberOfBins();
  
  unsigned int FinalEnergyBins = m_Response.GetAxis(2).GetNumberOfBins();
  unsigned int FinalPhiBins = m_Response.GetAxis(3).GetNumberOfBins();
  unsigned int FinalDirectionBins = m_Response.GetAxis(4).GetNumberOfBins();
  
  for (unsigned int b = 0; b < PointingBinsX.size(); ++b) {
    
    cout<<"Thread #"<<ThreadID<<": "<<b+1<<"/"<<PointingBinsX.size()<<endl;
    
    // First check if we have data, otherwise skip
    unsigned int PointingBin = PointingBinsX[b] + PointingBinsZ[b]*InitialDirectionBins;
    double PointingScaler = m_Pointing.Get(PointingBin);
    
    // Then create the rotation
    vector<double> XPointing = m_Pointing.GetAxis(0).GetBinCenters(PointingBinsX[b]);
    vector<double> ZPointing = m_Pointing.GetAxis(1).GetBinCenters(PointingBinsZ[b]);
    cout<<XPointing[0]<<":"<<XPointing[1]<<endl;
    cout<<ZPointing[0]<<":"<<ZPointing[1]<<endl;
    
    MRotationInterface RI;
    RI.SetGalacticPointingXAxis(XPointing[1], XPointing[0]-90); // Convert to Galactic 
    RI.SetGalacticPointingZAxis(ZPointing[1], ZPointing[0]-90); // Convert to Galactic 
    MRotation R = RI.GetGalacticPointingInverseRotationMatrix(); //good
    //MRotation R = RI.GetGalacticPointingRotationMatrix(); // wromg, but too good to believe
    
    /*
     *    MRotationInterface RI2;
     *    RI2.SetGalacticPointingXAxis(128.791, 7.50619); // Convert to Galactic 
     *    RI2.SetGalacticPointingZAxis(218.049, -5.61504); // Convert to Galactic 
     *    MRotation R2 = RI2.GetGalacticPointingInverseRotationMatrix();
     */
    
    
    // Precalculate the rotation map
    // --> Problem: Some map to the same bin...
    vector<unsigned int> RotatedBinMapping;
    for (unsigned int id = 0; id < InitialDirectionBins; ++id) {
      // Calculate the new detector pointing
      vector<double> GalacticPointing = m_ResponseGalactic.GetAxis(1).GetBinCenters(id);
      //cout<<GalacticPointing[0]<<":"<<GalacticPointing[1]<<endl;
      MVector GalIn;
      GalIn.SetMagThetaPhi(1.0, GalacticPointing[0]*c_Rad, GalacticPointing[1]*c_Rad);
      MVector LocalIn = R*GalIn;
      
      //cout<<R*GalIn<<" vs. "<<R2*GalIn<<" --> "<<LocalIn.Angle(R2*GalIn)*c_Deg<<endl;
      
      // Find that bin in the new response:
      RotatedBinMapping.push_back(m_Response.GetAxis(1).GetAxisBin(LocalIn.Theta()*c_Deg, LocalIn.Phi()*c_Deg));
      
      // cout<<"Mapping: "<<id<<"->"<<RotatedBinMapping[id]<<endl;
    }
    
    
    // Loop over the new response...
    //! Logic: a1 + S1*a2 + S1*S2*a3 + S1*S2*S3*a4 + S1*S2*S3*S4*a5  
    unsigned long M1 = 1;
    unsigned long M2 = M1*InitialEnergyBins;
    unsigned long M3 = M2*InitialDirectionBins;
    unsigned long M4 = M3*FinalEnergyBins;
    unsigned long M5 = M4*FinalPhiBins;
    
    for (unsigned int ie = 0; ie < InitialEnergyBins; ++ie) {
      unsigned long A1 = M1*ie;
      unsigned long B1 = M1*ie;
      for (unsigned int id = 0; id < InitialDirectionBins; ++id) {
        unsigned long A2 = A1 + M2*RotatedBinMapping[id];
        unsigned long B2 = B1 + M2*id;
        
        for (unsigned int fe = 0; fe < FinalEnergyBins; ++fe) {
          unsigned long A3 = A2 + M3*fe;
          unsigned long B3 = B2 + M3*fe;
          for (unsigned int fp = 0; fp < FinalPhiBins; ++fp) {
            unsigned long A4 = A3 + M4*fp;
            unsigned long B4 = B3 + M4*fp;
            for (unsigned int fd = 0; fd < FinalDirectionBins; ++fd) {
              
              unsigned int long A5 = A4 + M5*RotatedBinMapping[fd];
              unsigned int long B5 = B4 + M5*fd;
              
              double ResponseData = m_Response.Get(A5)*PointingScaler; // Normalize by time in pointing  
              if (ResponseData == 0) continue;
              
              m_ThreadMutex.lock();
              m_ResponseGalactic.Add(B5, ResponseData); 
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
 * Parallel background model rotation
 */
bool BinnedComptonImaging::RotateBackgroundModelInParallel(unsigned int ThreadID, unsigned int Model, vector<unsigned int> PointingBinsX, vector<unsigned int> PointingBinsZ)
{ 
  // Dimensions for later:
  unsigned int FinalEnergyBins = m_BackgroundModel[Model].GetAxis(0).GetNumberOfBins();
  unsigned int FinalPhiBins = m_BackgroundModel[Model].GetAxis(1).GetNumberOfBins();
  unsigned int FinalDirectionBins = m_BackgroundModel[Model].GetAxis(2).GetNumberOfBins();
  
  for (unsigned int b = 0; b < PointingBinsX.size(); ++b) {
    
    cout<<"Thread #"<<ThreadID<<": "<<b+1<<"/"<<PointingBinsX.size()<<endl;
    
    // First check if we have data, otherwise skip
    unsigned int PointingBin = PointingBinsX[b] + PointingBinsZ[b]*FinalDirectionBins;
    double PointingScaler = m_Pointing.Get(PointingBin);
    
    // Then create the rotation
    vector<double> XPointing = m_Pointing.GetAxis(0).GetBinCenters(PointingBinsX[b]);
    vector<double> ZPointing = m_Pointing.GetAxis(1).GetBinCenters(PointingBinsZ[b]);
    cout<<XPointing[0]<<":"<<XPointing[1]<<endl;
    cout<<ZPointing[0]<<":"<<ZPointing[1]<<endl;
    
    MRotationInterface RI;
    RI.SetGalacticPointingXAxis(XPointing[1], XPointing[0]-90); // Convert to Galactic 
    RI.SetGalacticPointingZAxis(ZPointing[1], ZPointing[0]-90); // Convert to Galactic 
    MRotation R = RI.GetGalacticPointingInverseRotationMatrix(); //good
    //MRotation R = RI.GetGalacticPointingRotationMatrix(); // wrong, but too good to believe

    
    // Precalculate the rotation map
    // --> Problem: Some map to the same bin...
    vector<unsigned int> RotatedBinMapping;
    for (unsigned int id = 0; id < FinalDirectionBins; ++id) {
      // Calculate the new detector pointing
      vector<double> GalacticPointing = m_BackgroundModelGalactic[Model].GetAxis(2).GetBinCenters(id);
      //cout<<GalacticPointing[0]<<":"<<GalacticPointing[1]<<endl;
      MVector GalIn;
      GalIn.SetMagThetaPhi(1.0, GalacticPointing[0]*c_Rad, GalacticPointing[1]*c_Rad);
      MVector LocalIn = R*GalIn;
      
      //cout<<R*GalIn<<" vs. "<<R2*GalIn<<" --> "<<LocalIn.Angle(R2*GalIn)*c_Deg<<endl;
      
      // Find that bin in the new response:
      RotatedBinMapping.push_back(m_BackgroundModel[Model].GetAxis(2).GetAxisBin(LocalIn.Theta()*c_Deg, LocalIn.Phi()*c_Deg));
      
      // cout<<"Mapping: "<<id<<"->"<<RotatedBinMapping[id]<<endl;
    }
    
    
    // Loop over the new response...
    //! Logic: a1 + S1*a2 + S1*S2*a3 + S1*S2*S3*a4 + S1*S2*S3*S4*a5  
    unsigned long M1 = 1;
    unsigned long M2 = M1*FinalEnergyBins;
    unsigned long M3 = M2*FinalPhiBins;
    
        
    for (unsigned int fe = 0; fe < FinalEnergyBins; ++fe) {
      unsigned long A1 = M1*fe;
      unsigned long B1 = M1*fe;
      for (unsigned int fp = 0; fp < FinalPhiBins; ++fp) {
        unsigned long A2 = A1 + M2*fp;
        unsigned long B2 = B1 + M2*fp;
        for (unsigned int fd = 0; fd < FinalDirectionBins; ++fd) {
              
          unsigned int long A3 = A2 + M3*RotatedBinMapping[fd];
          unsigned int long B3 = B2 + M3*fd;
              
          double ModelData = m_BackgroundModel[Model].Get(A3)*PointingScaler; // Normalize by time in pointing  
          if (ModelData == 0) continue;
              
          m_ThreadMutex.lock();
          m_BackgroundModelGalactic[Model].Add(B3, ModelData); 
          m_ThreadMutex.unlock();
        }
      }
    }

  }
  
  m_ThreadRunning[ThreadID] = false;
  
  return true;
}


/******************************************************************************
 * Create a list mode response
 */
MResponseMatrixON* BinnedComptonImaging::CreateListModeResponse(MComptonEvent* C)
{
  double AngularResolution = 5.0;
  
  MResponseMatrixON* R = new MResponseMatrixON();
  
  MResponseMatrixAxisSpheric AxisSkyCoordinates("#nu [deg]", "#lambda [deg]");
  AxisSkyCoordinates.SetFISBEL(m_ListModeBins);
  R->AddAxis(AxisSkyCoordinates);
  
  for (unsigned int i = 0; i < m_ListModeBins; ++i) {
    vector<double> Centers = AxisSkyCoordinates.GetBinCenters(i); // 0: theta/lat  1: phi/long
    
    // Get the ARM in Galactic coordiantes
    MVector Test;
    Test.SetMagThetaPhi(c_FarAway, Centers[0]*c_Rad, Centers[1]*c_Rad);
   
    double Dist = C->GetARMGamma(Test, MCoordinateSystem::c_Galactic);
    
    if (Dist < 3*AngularResolution/2.35*TMath::DegToRad()) {
      R->Set(i, TMath::Gaus(Dist, 0, AngularResolution/2.35 * TMath::DegToRad(), true)); 
    }
  }
  
  return R;
}


/******************************************************************************
 * Do whatever analysis is necessary
 */
bool BinnedComptonImaging::Analyze()
{
  // if (m_Interrupt == true) return false;

  // Open the response file - which determines the image data space grid...
  if (m_Response.Read(m_ResponseFileName) == false) {
    mgui<<"Error: Cannot read response file: \""<<m_ResponseFileName<<"\""<<endl;
    return false;
  }
  
  // Open the background model if the is any
  bool UseBackgroundModel = false;
  if (m_BackgroundModelFileName.size() > 0) UseBackgroundModel = true;
  if (UseBackgroundModel == true) {
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
        m_Response.GetAxis(4) != m_BackgroundModel[b].GetAxis(2)) {
        mgui<<"Error: The response and background model axes arre not identical"<<endl;
        return false;
      }
        
      m_BackgroundModelGalactic[b].SetName("Response Galactic");
      m_BackgroundModelGalactic[b].AddAxis(m_BackgroundModel[b].GetAxis(0)); // energy
      m_BackgroundModelGalactic[b].AddAxis(m_BackgroundModel[b].GetAxis(1)); // phi
      m_BackgroundModelGalactic[b].AddAxis(m_BackgroundModel[b].GetAxis(2)); // direction scattered gamma ray IN GALACTIC coordinates
    }
  }
  
  MResponseMatrixON Image("Image");
  Image.AddAxis(m_Response.GetAxis(0)); // energy
  Image.AddAxis(m_Response.GetAxis(1)); // image space
  
  MResponseMatrixON Display("Display");
  Display.AddAxis(m_Response.GetAxis(0)); // energy
  Display.AddAxis(m_Response.GetAxis(1)); // image space
  
  unsigned int InitialEnergyBins = m_Response.GetAxis(0).GetNumberOfBins();
  unsigned int InitialDirectionBins = m_Response.GetAxis(1).GetNumberOfBins();

  MResponseMatrixON Data("Data");
  Data.AddAxis(m_Response.GetAxis(2)); // energy
  Data.AddAxis(m_Response.GetAxis(3)); // phi
  Data.AddAxis(m_Response.GetAxis(4)); // direction of scattered gamma ray in GALACTIC coordinates
  
  m_Pointing.SetName("Pointing");
  m_Pointing.AddAxis(m_Response.GetAxis(1)); // direction of scattered gamma ray in GALACTIC coordinates
  m_Pointing.AddAxis(m_Response.GetAxis(4)); // direction of scattered gamma ray in GALACTIC coordinates
  
  if (m_ResponseGalacticFileName != "") {
    if (m_ResponseGalactic.Read(m_ResponseGalacticFileName) == false) {
      mgui<<"Cannot read response file: \""<<m_ResponseGalacticFileName<<"\""<<endl;
      return false;
    }
  } else {
    m_ResponseGalactic.SetName("Response Galactic");
    m_ResponseGalactic.AddAxis(m_Response.GetAxis(0)); // energy
    m_ResponseGalactic.AddAxis(m_Response.GetAxis(1)); // image space in GALACTIC coordinates
    m_ResponseGalactic.AddAxis(m_Response.GetAxis(2)); // energy
    m_ResponseGalactic.AddAxis(m_Response.GetAxis(3)); // phi
    m_ResponseGalactic.AddAxis(m_Response.GetAxis(4)); // direction scattered gamma ray IN GALACTIC coordinates
  }
  
  unsigned int FinalEnergyBins = m_Response.GetAxis(2).GetNumberOfBins();
  unsigned int FinalPhiBins = m_Response.GetAxis(3).GetNumberOfBins();
  unsigned int FinalDirectionBins = m_Response.GetAxis(4).GetNumberOfBins();

  // Normalize response:
  long Started = m_Response.GetSimulatedEvents();
  double StartArea = m_Response.GetFarFieldStartArea();
  double Steradians = 4*c_Pi / InitialDirectionBins;
  if (StartArea == 0) {
    cout<<"Error no start area given"<<endl;
    return false;
  }

  cout<<"Bin area: "<<Steradians<<" sr"<<endl;
  cout<<"Start area: "<<StartArea<<endl;
  
  {
    cout<<"Normalizing response in detector coordinates..."<<endl;
    MTimer T;
    
    //! Logic: a1 + S1*a2 + S1*S2*a3 + S1*S2*S3*a4 + S1*S2*S3*S4*a5  
    float Max = 0;
    unsigned long M1 = 1;
    unsigned long M2 = M1*InitialEnergyBins;
    unsigned long M3 = M2*InitialDirectionBins;
    unsigned long M4 = M3*FinalEnergyBins;
    unsigned long M5 = M4*FinalPhiBins;
    for (unsigned int ie = 0; ie < InitialEnergyBins; ++ie) {
      unsigned long A1 = M1*ie;
      for (unsigned int id = 0; id < InitialDirectionBins; ++id) {
        unsigned long A2 = A1 + M2*id;
        long Emitted = Started/InitialDirectionBins;
        long Detected = 0;
        for (unsigned int fe = 0; fe < FinalEnergyBins; ++fe) {
          unsigned long A3 = A2 + M3*fe;
          for (unsigned int fp = 0; fp < FinalPhiBins; ++fp) {
            unsigned long A4 = A3 + M4*fp;
            for (unsigned int fd = 0; fd < FinalDirectionBins; ++fd) {
              unsigned long A5 = A4 + M5*fd;
              Detected = m_Response.Get(A5);
              float Value = float(Detected) / float(Emitted);
              m_Response.Set(A5, Value);
              if (Value > Max) Max = Value;
              // Detected = m_Response.Get(vector<unsigned int>{ie, id, fe, fp, fd});
              // m_Response.Set(vector<unsigned int>{ie, id, fe, fp, fd}, float(Detected) / float(Emitted)) / StartArea / Radians;
            }
          }
        }
      }
    }

    /*
    // Cleaning round - remove everything with less than 1% contribution
    float Threshold = 0.02*Max;
    for (unsigned int ie = 0; ie < InitialEnergyBins; ++ie) {
      unsigned long A1 = M1*ie;
      for (unsigned int id = 0; id < InitialDirectionBins; ++id) {
        unsigned long A2 = A1 + M2*id;
        long Emitted = Started/InitialDirectionBins;
        long Detected = 0;
        for (unsigned int fe = 0; fe < FinalEnergyBins; ++fe) {
          unsigned long A3 = A2 + M3*fe;
          for (unsigned int fp = 0; fp < FinalPhiBins; ++fp) {
            unsigned long A4 = A3 + M4*fp;
            for (unsigned int fd = 0; fd < FinalDirectionBins; ++fd) {
              unsigned long A5 = A4 + M5*fd;
              if (m_Response.Get(A5) < Threshold) {
                m_Response.Set(A5, 0.0);
              }
            }
          }
        }
      }
    }
    */

    cout<<"Done: "<<T.GetElapsed()<<endl;
  }
  
  
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
  cout<<"Filling data space..."<<endl;
  
  float Ei;
  float Phi;
  MVector Dg;
  float Chi;
  float Psi;

  // Fill the data space
  long Counter = 0;
  double ObservationTime = 0.0;
  MPhysicalEvent* Event = 0;
  MComptonEvent* ComptonEvent = 0;
  
  // The list-mode responses:
  vector<MResponseMatrixON*> m_ListModeResponses;
  
  double PreviousTime = -1;
  vector<double> TimeBetweenEvents;
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
      
      
      if (Phi < 0 || Phi > 60) continue;
      //if (Phi > ) continue;
      
      if (m_ReOrient == true) {
        if (ComptonEvent->HasGalacticPointing() == true) {
          MRotation R = ComptonEvent->GetGalacticPointingRotationMatrix();
          Dg = R*Dg;
        } else {
          cout<<"Event has no Galactic pointing!"<<endl;
          delete Event;
          continue;        
        }
      }
      
      if (m_UseHybridMode == true) {
        m_ListModeResponses.push_back(CreateListModeResponse(ComptonEvent));
      }
      
      // ARM
      MVector Test;
      Test.SetMagThetaPhi(c_FarAway, (-05.78+90)*c_Rad, 184.56*c_Rad);
      //cout<<"ARM : "<<Event->GetId()<<":"<<ComptonEvent->GetARMGamma(Test, MCoordinateSystem::c_Galactic)*c_Deg<<endl;
      
      
      Chi = Dg.Phi()*c_Deg;
      while (Chi < 0) Chi += 360.0;
      while (Chi > 360) Chi -= 360.0;
      Psi = Dg.Theta()*c_Deg;
      
      //cout<<Event->GetId()<<": "<<Phi<<":"<<Psi<<": "<<Chi<<endl;
      
      Data.Add(vector<double>{Ei, Phi, Psi, Chi}, 1);

      ++Counter;
      
      if (PreviousTime != -1) {
        TimeBetweenEvents.push_back(Event->GetTime().GetAsSeconds() - PreviousTime);
      }
      PreviousTime = Event->GetTime().GetAsSeconds();
      
      //cout<<Event->GetId()<<":  X (la/lo): "<<90+ComptonEvent->GetGalacticPointingXAxisLatitude()*c_Deg<<", "<<ComptonEvent->GetGalacticPointingXAxisLongitude()*c_Deg<<"   Z (la/lo): "<<90+ComptonEvent->GetGalacticPointingZAxisLatitude()*c_Deg<<", "<<ComptonEvent->GetGalacticPointingZAxisLongitude()*c_Deg<<endl;
      
      //! Set the pointing
      m_Pointing.Add(vector<double>{ 90+ComptonEvent->GetGalacticPointingXAxisLatitude()*c_Deg, ComptonEvent->GetGalacticPointingXAxisLongitude()*c_Deg, 90+ComptonEvent->GetGalacticPointingZAxisLatitude()*c_Deg, ComptonEvent->GetGalacticPointingZAxisLongitude()*c_Deg } );
      
    } // if Compton
  
    ObservationTime = Event->GetTime().GetAsSeconds();

    delete Event;
    
    //if (m_ListModeResponses.size() == 3) break;
    
    //if (Counter == 2) break;
  }
  EventFile->ShowProgress(false);
  EventFile->Close();
  //delete EventFile;
  gSystem->ProcessEvents();
  
  cout<<"Obs time: "<<ObservationTime<<endl;
  
  cout<<"Collected "<<Counter<<" events."<<endl;

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
  
  m_Pointing.Write("Pointing.rsp");
  Data.Write("Data.rsp");
  
  
  // Create rotated response
  cout<<"Creating rotated response..."<<endl;
  if (m_ResponseGalacticFileName == "") {
  
    // Step 1: Create a list of directions - pointing bins in this case
    vector<unsigned int> PointingBinsX;
    vector<unsigned int> PointingBinsZ;
    int Dirs = 0;
    // Create a new entry into the Galactic response for each pointing
    for (unsigned int x = 0; x < InitialDirectionBins; ++x) {
      for (unsigned int z = 0; z < InitialDirectionBins; ++z) {
        //cout<<x<<"/"<<z<<"/"<<InitialDirectionBins<<endl;
        
        // First check if we have data, otherwise skip
        unsigned int PointingBin = x + z*InitialDirectionBins;
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
    
    cout<<endl<<"Writing rotated response"<<endl;
    m_ResponseGalactic.Write("ResponseGalactic.rsp");
    cout<<"Number of directions in pointings file: "<<Dirs<<endl;
    cout<<"Response Galactic normalization: "<<m_ResponseGalactic.GetSum()<<"  vs. "<<m_Response.GetSum()<<endl;
  }

  
  if (m_ReOrient == false) {
    m_ResponseGalactic = m_Response; 
  }
  
  
  vector<double> m_TotalBackgroundInModel(m_BackgroundModel.size(), 0);
  if (UseBackgroundModel == true) {
    cout<<"Creating background model in Galactic coordinates"<<endl;
    
    // Step 1: Create a list of directions - pointing bins in this case
    vector<unsigned int> PointingBinsX;
    vector<unsigned int> PointingBinsZ;
    int Dirs = 0;
    // Create a new entry into the Galactic response for each pointing
    for (unsigned int x = 0; x < InitialDirectionBins; ++x) {
      for (unsigned int z = 0; z < InitialDirectionBins; ++z) {
        //cout<<x<<"/"<<z<<"/"<<InitialDirectionBins<<endl;
        
        // First check if we have data, otherwise skip
        unsigned int PointingBin = x + z*InitialDirectionBins;
        if (m_Pointing.Get(PointingBin) == 0) continue;
        Dirs++;
        
        PointingBinsX.push_back(x);
        PointingBinsZ.push_back(z);
      }
    }
    
    // Step 2: Do the actual rotation in parallel
    unsigned int Split = PointingBinsX.size() / std::thread::hardware_concurrency();
    for (unsigned int b = 0; b < m_BackgroundModel.size(); ++b) {
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
        
        if (m_ReOrient == false) {
          for (unsigned int b = 0; b < m_BackgroundModel.size(); ++b) { 
            m_BackgroundModelGalactic[b] = m_BackgroundModel[b]; 
          }
        }
      }
    }
    
    // RotateResponseInParallel(PointingBinsX, PointingBinsZ);
    
    cout<<endl<<"Writing rotated response"<<endl;
    for (unsigned int b = 0; b < m_BackgroundModel.size(); ++b) { 
      m_BackgroundModelGalactic[b].Write(MString("BackgroundModelGalactic_model") + b + ".rsp");
      m_TotalBackgroundInModel[b] = m_BackgroundModelGalactic[b].GetSum();
    }
    cout<<"Number of directions in pointings file: "<<Dirs<<endl;
    
    
    // Backproject background model
    /*
    {
      MTimer T;
      
      MResponseMatrixON BackprojectedBackground("Image");
      BackprojectedBackground.AddAxis(m_Response.GetAxis(0)); // energy
      BackprojectedBackground.AddAxis(m_Response.GetAxis(1)); // image space
      
      //! Logic: a1 + S1*a2 + S1*S2*a3 + S1*S2*S3*a4 + S1*S2*S3*S4*a5  
      unsigned long M1 = 1;
      unsigned long M2 = M1*InitialEnergyBins;
      unsigned long M3 = M2*InitialDirectionBins;
      unsigned long M4 = M3*FinalEnergyBins;
      unsigned long M5 = M4*FinalPhiBins;
      for (unsigned int ie = 0; ie < InitialEnergyBins; ++ie) {
        unsigned long A1 = M1*ie;
        for (unsigned int id = 0; id < InitialDirectionBins; ++id) {
          unsigned long A2 = A1 + M2*id;
          float Content = 0.0;
          for (unsigned int fe = 0; fe < FinalEnergyBins; ++fe) {
            unsigned long A3 = A2 + M3*fe;
            unsigned int D1 = 1*fe;
            for (unsigned int fp = 0; fp < FinalPhiBins; ++fp) {
              unsigned long A4 = A3 + M4*fp;
              unsigned int D2 = D1 +fp*FinalEnergyBins;
              for (unsigned int fd = 0; fd < FinalDirectionBins; ++fd) {
                Content += m_ResponseGalactic.Get(A4 + M5*fd) * m_BackgroundModelGalactic.Get(D2 + fd*FinalEnergyBins*FinalPhiBins);
              }
            }
          }
          BackprojectedBackground.Set(vector<unsigned int>{ ie, id }, Content);
        }
      }
      cout<<"Done: "<<T.GetElapsed()<<endl;
      
      // Normalize image to 1.0
      double Sum = BackprojectedBackground.GetSum();
      if (Sum == 0) {
        cout<<"Error: First backprojection sum image is zero"<<endl;
        return false;
      }
      if (isfinite(Sum) == false) {
        cout<<"Error: First backprojection image contains non-finite number."<<endl;
        return false;
      }
      BackprojectedBackground *= 1.0/BackprojectedBackground.GetSum();
      
      BackprojectedBackground.ShowSlice(vector<float>{ 511.0, MResponseMatrix::c_ShowX, MResponseMatrix::c_ShowY }, true);  
      gSystem->ProcessEvents();
      
      BackprojectedBackground.Write("BackprojectedBackground.rsp");
      
      vector<double> BackprojectedBackgroundData(BackprojectedBackground.GetAxis(1).GetNumberOfBins());
      for (unsigned int ie = 0; ie < InitialEnergyBins; ++ie) {
        for (unsigned int id = 0; id < InitialDirectionBins; ++id) {
          BackprojectedBackgroundData[id] = BackprojectedBackground.Get(ie + InitialEnergyBins*id);
        }
      }
      
      MImageGalactic* G = new MImageGalactic();
      G->SetTitle("Backprojected background");
      G->SetXAxisTitle("Galactic Longitude [deg]");
      G->SetYAxisTitle("Galactic Latitude [deg]");
      G->SetValueAxisTitle("Flux");
      G->SetDrawOption(MImage::c_COLZ);
      G->SetSpectrum(MImage::c_Rainbow);
      G->SetSourceCatalog("$(MEGALIB)/resource/catalogs/Crab.scat");
      G->Normalize(false);
      G->SetFISBEL(BackprojectedBackgroundData);
      G->Display();
    }
  }
  */
  
  // Create an initial backprojection
  cout<<"Creating initial backprojection..."<<endl;
  
  
  {
    MTimer T;
    
    //! Logic: a1 + S1*a2 + S1*S2*a3 + S1*S2*S3*a4 + S1*S2*S3*S4*a5  
    unsigned long M1 = 1;
    unsigned long M2 = M1*InitialEnergyBins;
    unsigned long M3 = M2*InitialDirectionBins;
    unsigned long M4 = M3*FinalEnergyBins;
    unsigned long M5 = M4*FinalPhiBins;
    for (unsigned int ie = 0; ie < InitialEnergyBins; ++ie) {
      unsigned long A1 = M1*ie;
      for (unsigned int id = 0; id < InitialDirectionBins; ++id) {
        unsigned long A2 = A1 + M2*id;
        float Content = 0.0;
        for (unsigned int fe = 0; fe < FinalEnergyBins; ++fe) {
         unsigned long A3 = A2 + M3*fe;
         unsigned int D1 = 1*fe;
         for (unsigned int fp = 0; fp < FinalPhiBins; ++fp) {
            unsigned long A4 = A3 + M4*fp;
            unsigned int D2 = D1 +fp*FinalEnergyBins;
            for (unsigned int fd = 0; fd < FinalDirectionBins; ++fd) {
              Content += m_ResponseGalactic.Get(A4 + M5*fd) * Data.Get(D2 + fd*FinalEnergyBins*FinalPhiBins);
            }
          }
        }
        Image.Set(vector<unsigned int>{ ie, id }, Content);
      }
    }
    cout<<"Done: "<<T.GetElapsed()<<endl;
    
    // Normalize image to 1.0
    double Sum = Image.GetSum();
    if (Sum == 0) {
      cout<<"Error: First backprojection sum image is zero"<<endl;
      return false;
    }
    if (isfinite(Sum) == false) {
      cout<<"Error: First backprojection image contains non-finite number."<<endl;
      return false;
    }
    Image *= 1.0/Image.GetSum();
    
    Image.ShowSlice(vector<float>{ 511.0, MResponseMatrix::c_ShowX, MResponseMatrix::c_ShowY }, true);  
    gSystem->ProcessEvents();
    
    Image.Write("FirstBackprojection.rsp");
    
    vector<double> BackprojectedDataData(Image.GetAxis(1).GetNumberOfBins());
    for (unsigned int ie = 0; ie < InitialEnergyBins; ++ie) {
      for (unsigned int id = 0; id < InitialDirectionBins; ++id) {
        BackprojectedDataData[id] = Image.Get(ie + InitialEnergyBins*id);
      }
    }
    
    MImageGalactic* G = new MImageGalactic();
    G->SetTitle("1st backprojection");
    G->SetXAxisTitle("Galactic Longitude [deg]");
    G->SetYAxisTitle("Galactic Latitude [deg]");
    G->SetValueAxisTitle("Flux");
    G->SetDrawOption(MImage::c_COLZ);
    G->SetSpectrum(MImage::c_Rainbow);
    G->SetSourceCatalog("$(MEGALIB)/resource/catalogs/Crab.scat");
    G->Normalize(false);
    G->SetFISBEL(BackprojectedDataData);
    G->Display();
    
    
  }
  
  
  // Maximum entropy
  if (m_UseHybridMode == false) {
  if (m_UseMaximumEntropy == true) {
    
    // Set up Lagrange multipliers
    
    MResponseMatrixON Lagrange;
    Lagrange.AddAxis(m_ResponseGalactic.GetAxis(2)); // energy
    Lagrange.AddAxis(m_ResponseGalactic.GetAxis(3)); // phi
    Lagrange.AddAxis(m_ResponseGalactic.GetAxis(4)); // direction scattered gamma ray  
    
    unsigned long P1 = 1;
    unsigned long P2 = P1*FinalEnergyBins;
    unsigned long P3 = P2*FinalPhiBins;
    for (unsigned int fe = 0; fe < FinalEnergyBins; ++fe) {
      unsigned long A1 = P1*fe;
      for (unsigned int fp = 0; fp < FinalPhiBins; ++fp) {
        unsigned long A2 = A1 + P2*fp;
        for (unsigned int fd = 0; fd < FinalDirectionBins; ++fd) {
          unsigned long A3 = A2 + P3*fd;
          Lagrange.Set(A3, 1.0); //Data.Get(A3));
        }
      }
    }
    
    // Normalize response
    m_ResponseGalactic /= m_ResponseGalactic.GetSum(); 


    double DataSum = Data.GetSum();
    cout<<"Data Sum="<<DataSum<<endl;   
 
    // The iterations
    MResponseMatrixON Expectation;
    Expectation.AddAxis(m_ResponseGalactic.GetAxis(2)); // energy
    Expectation.AddAxis(m_ResponseGalactic.GetAxis(3)); // phi
    Expectation.AddAxis(m_ResponseGalactic.GetAxis(4)); // diretcion scattered gamma ray  
    
    vector<double> BackgroundScaler(m_BackgroundModel.size(), 1.0);
    
    double MaximumEntropy = 0;
    int MaximumIterations = 0;
    
    for (unsigned int i = 0; i < m_Iterations; ++i) {
      
      // Step 1: Calculate the restored image:
      
      MResponseMatrixON RestoredImage("RestoredImage");
      RestoredImage.AddAxis(m_Response.GetAxis(0)); // energy
      RestoredImage.AddAxis(m_Response.GetAxis(1)); // image space      
      
      double RestoredImageSum = 0;
      
      unsigned long M1 = 1;
      unsigned long M2 = M1*InitialEnergyBins;
      unsigned long M3 = M2*InitialDirectionBins;
      unsigned long M4 = M3*FinalEnergyBins;
      unsigned long M5 = M4*FinalPhiBins;
      for (unsigned int ie = 0; ie < InitialEnergyBins; ++ie) {
        unsigned long A1 = M1*ie;
        for (unsigned int id = 0; id < InitialDirectionBins; ++id) {
          unsigned long A2 = A1 + M2*id;
          float Content = 0.0;
          unsigned long N1 = 1;
          unsigned long N2 = N1*FinalEnergyBins;
          unsigned long N3 = N2*FinalPhiBins;
          for (unsigned int fe = 0; fe < FinalEnergyBins; ++fe) {
            unsigned long A3 = A2 + M3*fe;
            unsigned long B1 = N1*fe;
            for (unsigned int fp = 0; fp < FinalPhiBins; ++fp) {
              unsigned long A4 = A3 + M4*fp;
              unsigned long B2 = B1 + N2*fp;
              for (unsigned int fd = 0; fd < FinalDirectionBins; ++fd) {
                unsigned long A5 = A4 + M5*fd;
                unsigned long B3 = B2 + N3*fd;
                
                Content += m_ResponseGalactic.Get(A5) * Lagrange.Get(B3);
              }
            }
          }
          
          Content = exp(Content);
          RestoredImage.Set(A2, Content);
          RestoredImageSum += Content;
        }
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
      for (unsigned int ie = 0; ie < InitialEnergyBins; ++ie) {
        unsigned long A1 = M1*ie;
        for (unsigned int id = 0; id < InitialDirectionBins; ++id) {
          unsigned long A2 = A1 + M2*id;
          float RestoredImageUpdate = RestoredImage.Get(A2)*DataSum/RestoredImageSum;
          RestoredImage.Set(A2, RestoredImageUpdate);
          Entropy += RestoredImageUpdate*log(RestoredImageUpdate);
        }
      }
      cout<<"Entropy: "<<Entropy<<endl;    
      
      
      // Step 2: Convolve the data again into data space - expectation calculation
      
      unsigned long P1 = 1;
      unsigned long P2 = P1*InitialEnergyBins;
      unsigned long P3 = P2*InitialDirectionBins;
      unsigned long P4 = P3*FinalEnergyBins;
      unsigned long P5 = P4*FinalPhiBins;
      for (unsigned int fe = 0; fe < FinalEnergyBins; ++fe) {
        //unsigned long A3 = P3*fe;
        for (unsigned int fp = 0; fp < FinalPhiBins; ++fp) {
          //unsigned long A4 = P4*fp;
          for (unsigned int fd = 0; fd < FinalDirectionBins; ++fd) {
            //unsigned long A5 = P5*fd;
            
            float NewExpectation = 0;
            for (unsigned int ie = 0; ie < InitialEnergyBins; ++ie) {
              //unsigned long A1 = P1*ie;
              for (unsigned int id = 0; id < InitialDirectionBins; ++id) {
                //unsigned long A2 = P2*id;
                NewExpectation += RestoredImage.Get(ie + InitialEnergyBins*id) * m_ResponseGalactic.Get(ie + P2*id + P3*fe + P4*fp + P5*fd);
              }
            }
            
            if (UseBackgroundModel == true) {
              for (unsigned int b = 0; b < m_BackgroundModel.size(); ++b) {
                NewExpectation += BackgroundScaler[b] * m_BackgroundModelGalactic[b].Get(fe + FinalEnergyBins*fp + FinalEnergyBins*FinalPhiBins*fd);
              }
            }            
            
            Expectation.Set(fe + FinalEnergyBins*fp + FinalEnergyBins*FinalPhiBins*fd, NewExpectation);
          }
        }
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
      unsigned long Q1 = 1;
      unsigned long Q2 = Q1*FinalEnergyBins;
      unsigned long Q3 = Q2*FinalPhiBins;
      for (unsigned int fe = 0; fe < FinalEnergyBins; ++fe) {
        unsigned long A1 = Q1*fe;
        for (unsigned int fp = 0; fp < FinalPhiBins; ++fp) {
          unsigned long A2 = A1 + Q2*fp;
          for (unsigned int fd = 0; fd < FinalDirectionBins; ++fd) {
            unsigned long A3 = A2 + Q3*fd;
        
            bool UseLog = true;
            double Update = 0;
            if (UseLog == true) {
              double PreLog1 = Data.Get(A3)*Scale;
              if (PreLog1 < Limit) PreLog1 = Limit;
              Update += log(PreLog1);
              
              double PreLog2 = Expectation.Get(A3); 
              if (PreLog2 < Limit) PreLog2 = Limit;
              Update -= log(PreLog2);
           
              //cout<<"Lagrange diff: "<<Lagrange.Get(A3)<<" vs. "<<Data.Get(A3)*Scale<<" vs. "<<Expectation.Get(A3)<<" --> Update: "<<Update<<endl;
 
            } else {
              Update += Data.Get(A3)*Scale - Expectation.Get(A3);
              //cout<<"Lagrange diff: "<<Lagrange.Get(A3)<<" vs. "<<Data.Get(A3)*Scale<<" vs. "<<Expectation.Get(A3)<<" --> Update: "<<Update<<endl;
            }
              
            //cout<<"Lagrange diff: "<<Lagrange.Get(A3)<<" vs. "<<Update<<endl;
            double NewL = Lagrange.Get(A3) + UpdateFactor*Update;
            if (NewL < 0) {
              NewL = 0;
            }
            Lagrange.Set(A3, NewL);
            LCounter++;
          }
        }
      }
      cout<<" Updated Lagrange entries: "<<LCounter<<endl;
      cout<<"Lagrange sum="<<Lagrange.GetSum()<<endl;
      
      
      // Step 4: Update the background scaling factor:
      if (UseBackgroundModel == true) {
        vector<double> B_corr(m_BackgroundModel.size(), 0.0);
        
        unsigned long N1 = 1;
        unsigned long N2 = N1*FinalEnergyBins;
        unsigned long N3 = N2*FinalPhiBins;
        for (unsigned int fe = 0; fe < FinalEnergyBins; ++fe) {
          unsigned long B1 = N1*fe;
          for (unsigned int fp = 0; fp < FinalPhiBins; ++fp) {
            unsigned long B2 = B1 + N2*fp;
            for (unsigned int fd = 0; fd < FinalDirectionBins; ++fd) {
              unsigned long B3 = B2 + N3*fd;
              
              if (Data.Get(B3) > 0 && Expectation.Get(B3) > 0) {
                for (unsigned int b = 0; b < m_BackgroundModel.size(); ++b) {
                  B_corr[b] += m_BackgroundModelGalactic[b].Get(B3) * Data.Get(B3) / Expectation.Get(B3);
                }
              }
            }
          }
        }
        
        for (unsigned int b = 0; b < m_BackgroundModel.size(); ++b) {
          BackgroundScaler[b] *= B_corr[b] / m_TotalBackgroundInModel[b];
          cout<<"Background scaling factor: "<<BackgroundScaler[b]<<"!"<<B_corr[b]<<"!"<<m_TotalBackgroundInModel[b]<<endl;
        }
      }      
      
      
      // Step 5: Update the real image:
      MResponseMatrixON NewImage("NewImage");
      NewImage.AddAxis(m_Response.GetAxis(0)); // energy
      NewImage.AddAxis(m_Response.GetAxis(1)); // image space
      
      double ImageFlux = 0;
      for (unsigned int ie = 0; ie < InitialEnergyBins; ++ie) {
        unsigned long A1 = P1*ie;
        for (unsigned int id = 0; id < InitialDirectionBins; ++id) {
          unsigned long A2 = A1 + P2*id;
          NewImage.Set(A2, RestoredImage.Get(A2)); // * DataSum/RestoredImageSum);
          ImageFlux += NewImage.Get(A2) / ObservationTime / StartArea;
        }
      }
      
      if (Entropy > MaximumEntropy) {
        MaximumEntropy = Entropy;
        MaximumIterations = i;
      }
      
      ostringstream Title;
      Title<<"Image at iteration "<<i+1<<" with flux "<<ImageFlux<<" ph/cm2/s";
      //NewImage.ShowSlice(vector<float>{ 511.0, MResponseMatrix::c_ShowX, MResponseMatrix::c_ShowY }, true, Title.str());
      cout<<"Image content: "<<ImageFlux<<" ph/cm2/s"<<endl;
     
      vector<double> ImageData(NewImage.GetAxis(1).GetNumberOfBins());
      for (unsigned int ie = 0; ie < InitialEnergyBins; ++ie) {
        for (unsigned int id = 0; id < InitialDirectionBins; ++id) {
          ImageData[id] = NewImage.Get(ie + InitialEnergyBins*id);
        }
      }
      
      MImageGalactic* G = new MImageGalactic();
      G->SetTitle("Galaxy view");
      G->SetXAxisTitle("Galactic Longitude [deg]");
      G->SetYAxisTitle("Galactic Latitude [deg]");
      G->SetValueAxisTitle("Flux");
      G->SetDrawOption(MImage::c_COLZ);
      G->SetSpectrum(MImage::c_Rainbow);
      //G->SetSourceCatalog("$(MEGALIB)/resource/catalogs/Crab.scat");
      //G->SetProjection(MImageProjection::c_Hammer);
      G->Normalize(false);
      G->SetFISBEL(ImageData);
      G->Display();
      
      //NewImage.Write(MString("Image_") + (i+1) + ".rsp");
      
      Display = NewImage;
      
      gSystem->ProcessEvents();
      
      if (m_Interrupt == true) break;
 
    } // iterations
  
    
  } 
  
  // Maximum likelihood
  else {
    // The iterations
    MResponseMatrixON Mean;
    Mean.AddAxis(m_ResponseGalactic.GetAxis(2)); // energy
    Mean.AddAxis(m_ResponseGalactic.GetAxis(3)); // phi
    Mean.AddAxis(m_ResponseGalactic.GetAxis(4)); // diretcion scattered gamma ray  
    
    vector<double> BackgroundScaler(m_BackgroundModel.size(), 1.0);
    
    double MaximumEntropy = 0;
    int MaximumIterations = 0;
    for (unsigned int i = 0; i < m_Iterations; ++i) {
      cout<<"Iteration: "<<i+1<<" - convolve"<<endl;
      
      // Convolve:
      //! Logic: a1 + S1*a2 + S1*S2*a3 + S1*S2*S3*a4 + S1*S2*S3*S4*a5
      //! Logic: a1 + S1*(a2 + S2*(a3 + S3*(a4 + S4*(a5))))
      unsigned long P1 = 1;
      unsigned long P2 = P1*InitialEnergyBins;
      unsigned long P3 = P2*InitialDirectionBins;
      unsigned long P4 = P3*FinalEnergyBins;
      unsigned long P5 = P4*FinalPhiBins;
      for (unsigned int fe = 0; fe < FinalEnergyBins; ++fe) {
        unsigned long A3 = P3*fe;
        for (unsigned int fp = 0; fp < FinalPhiBins; ++fp) {
          unsigned long A4 = P4*fp;
          for (unsigned int fd = 0; fd < FinalDirectionBins; ++fd) {
            unsigned long A5 = P5*fd;
            
            float NewMean = 0;
            for (unsigned int ie = 0; ie < InitialEnergyBins; ++ie) {
              unsigned long A1 = P1*ie;
              for (unsigned int id = 0; id < InitialDirectionBins; ++id) {
                unsigned long A2 = P2*id;
                NewMean += Image.Get(ie + InitialEnergyBins*id) * m_ResponseGalactic.Get(ie + P2*id + P3*fe + P4*fp + P5*fd);
                //NewMean += Image.Get(vector<unsigned int>{ie, id}) * m_ResponseGalactic.Get(vector<unsigned int>{ie, id, fe, fp, fd});
              }
            }
            
            if (UseBackgroundModel == true) {
              for (unsigned int b = 0; b < m_BackgroundModel.size(); ++b) {
                NewMean += BackgroundScaler[b] * m_BackgroundModelGalactic[b].Get(fe + FinalEnergyBins*fp + FinalEnergyBins*FinalPhiBins*fd);
              }
            }
            
            Mean.Set(fe + FinalEnergyBins*fp + FinalEnergyBins*FinalPhiBins*fd, NewMean);
            //Mean.Set(vector<unCrabAndBackground.Moving2.p2.trasigned int>{fe, fp, fd}, NewMean);
          }
        }
      }
      
      Mean.Write(MString("Mean_") + (i+1) + ".rsp");
      cout<<"Iteration: "<<i+1<<" - deconvolve"<<endl;
      
      // Now deconvolve the background scaling factor first :
      if (UseBackgroundModel == true) {
        vector<double> B_corr(m_BackgroundModel.size(), 0);
        
        unsigned long N1 = 1;
        unsigned long N2 = N1*FinalEnergyBins;
        unsigned long N3 = N2*FinalPhiBins;
        for (unsigned int fe = 0; fe < FinalEnergyBins; ++fe) {
          unsigned long B1 = N1*fe;
          for (unsigned int fp = 0; fp < FinalPhiBins; ++fp) {
            unsigned long B2 = B1 + N2*fp;
            for (unsigned int fd = 0; fd < FinalDirectionBins; ++fd) {
              unsigned long B3 = B2 + N3*fd;
              
              if (Data.Get(B3) > 0 && Mean.Get(B3) > 0) {
                for (unsigned int b = 0; b < m_BackgroundModel.size(); ++b) {
                  B_corr[b] += m_BackgroundModelGalactic[b].Get(B3) * Data.Get(B3) / Mean.Get(B3);
                }
              }
            }
          }
        }
        
        for (unsigned int b = 0; b < m_BackgroundModel.size(); ++b) {
          BackgroundScaler[b] *= B_corr[b] / m_TotalBackgroundInModel[b];
          cout<<"Background scaling factor: "<<BackgroundScaler[b]<<"!"<<B_corr[b]<<"!"<<m_TotalBackgroundInModel[b]<<endl;
        }
      }
      
      
      double ImageFlux = 0.0;
      
      MResponseMatrixON NewImage("NewImage");
      NewImage.AddAxis(m_Response.GetAxis(0)); // energy
      NewImage.AddAxis(m_Response.GetAxis(1)); // image space
      
      // Deconvolve:
      unsigned long M1 = 1;
      unsigned long M2 = M1*InitialEnergyBins;
      unsigned long M3 = M2*InitialDirectionBins;
      unsigned long M4 = M3*FinalEnergyBins;
      unsigned long M5 = M4*FinalPhiBins;
      for (unsigned int ie = 0; ie < InitialEnergyBins; ++ie) {
        unsigned long A1 = M1*ie;
        for (unsigned int id = 0; id < InitialDirectionBins; ++id) {
          unsigned long A2 = A1 + M2*id;
          float Content = 0.0;
          float Sum = 0.0;
          unsigned long N1 = 1;
          unsigned long N2 = N1*FinalEnergyBins;
          unsigned long N3 = N2*FinalPhiBins;
          for (unsigned int fe = 0; fe < FinalEnergyBins; ++fe) {
            unsigned long A3 = A2 + M3*fe;
            unsigned long B1 = N1*fe;
            for (unsigned int fp = 0; fp < FinalPhiBins; ++fp) {
              unsigned long A4 = A3 + M4*fp;
              unsigned long B2 = B1 + N2*fp;
              for (unsigned int fd = 0; fd < FinalDirectionBins; ++fd) {
                unsigned long A5 = A4 + M5*fd;
                unsigned long B3 = B2 + N3*fd;
                
                if (Mean.Get(B3) > 0) {
                  Content += m_ResponseGalactic.Get(A5) * Data.Get(B3) / Mean.Get(B3);
                }
                Sum += m_ResponseGalactic.Get(A5);
              }
            }
          }
          if (Sum > 0) {
            NewImage.Set(A2, Content);
            Image.Set(A2, Content * Image.Get(A2) / Sum);
            ImageFlux += (Content * Image.Get(A2) / Sum) / ObservationTime / StartArea;
            Display.Set(A2, Content * Image.Get(A2) / Sum / ObservationTime / StartArea / Steradians );
            //Image.Set(vector<unsigned int>{ie, id}, Content * Image.Get(vector<unsigned int>{ie, id}) / Sum);
            gSystem->ProcessEvents();
          }
        }
      }
      
      
      ostringstream Title;
      Title<<"Image at iteration "<<i+1<<" with flux "<<ImageFlux<<" ph/cm2/s";
      
      //Display.ShowSlice(vector<float>{ 511.0, MResponseMatrix::c_ShowX, MResponseMatrix::c_ShowY }, true, Title.str());
      cout<<"Image content: "<<ImageFlux<<" ph/cm2/s for T="<<ObservationTime<<" sec and A="<<StartArea<<" cm^2"<<endl;
      
      vector<double> ImageData(Display.GetAxis(1).GetNumberOfBins());
      for (unsigned int ie = 0; ie < InitialEnergyBins; ++ie) {
        for (unsigned int id = 0; id < InitialDirectionBins; ++id) {
          ImageData[id] = Display.Get(ie + InitialEnergyBins*id);
        }
      }
      
      MImageGalactic* G = new MImageGalactic();
      G->SetTitle("Galaxy view");
      G->SetXAxisTitle("Galactic Longitude [deg]");
      G->SetYAxisTitle("Galactic Latitude [deg]");
      G->SetValueAxisTitle("Flux");
      G->SetDrawOption(MImage::c_COLZ);
      G->SetSpectrum(MImage::c_Rainbow);
      //G->SetSourceCatalog("$(MEGALIB)/resource/catalogs/Crab.scat");
      G->Normalize(false);
      G->SetFISBEL(ImageData);
      G->Display();
      
      //Display.Write(MString("Image_") + (i+1) + ".rsp");
      
      gSystem->ProcessEvents();
      
      if (m_Interrupt == true) break;
    }
  }
  
  // Hybrid mode:
  } else {
  
    /*
    
    // Approach per iteration:
    // (1) Create new binned mode image
    // (2) Per bin scale list mode image with binned mode image
    // (3) Run list mode 
    // (4) From list-mode image create new binned mode start image
    // (5) Go back to (1)
    
    
    // Stage 1: Setup
    
    // Stage 1.1: Create the list mode image
    
    MResponseMatrixAxisSpheric AxisSkyCoordinates("#nu [deg]", "#lambda [deg]");
    AxisSkyCoordinates.SetFISBEL(m_ListModeBins);

    MResponseMatrixON LMImage;
    LMImage.AddAxis(AxisSkyCoordinates);
    
    
    // Stage 1.2: Conversion LM->BN and vice versa
    
    // The binned mode bin to which each list mode bin belongs 
    vector<double> ListToBinnedModeBins(m_ListModeBins);
    for (unsigned int i = 0; i < m_ListModeBins; ++i) {
      vector<double> BinCenters = LMImage.GetAxis(0).GetBinCenters(i);
      ListToBinnedModeBins[i] = Display.GetAxis(1).GetAxisBin(BinCenters[0], BinCenters[1]);
      cout<<"LM: "<<i<<"  BM: "<<ListToBinnedModeBins[i]<<endl;
    }
    
    // The binned-mode value for each list-mode image value
    MResponseMatrixON BinnedModeMean;
    BinnedModeMean.AddAxis(AxisSkyCoordinates);
    for (unsigned int i = 0; i < m_ListModeBins; ++i) {
      BinnedModeMean.Set(i, Display.Get(ListToBinnedModeBins[i]));
    }
    
    // The "other" bins list-mode bins in each binned-mode bin
    vector<vector<unsigned int>> OtherBins(m_ListModeBins);
    vector<vector<unsigned int>> AllListModeBinsInBinnedModeBin(InitialDirectionBins);
    for (unsigned int i = 0; i < m_ListModeBins; ++i) {
      unsigned int BinnedModeBin = ListToBinnedModeBins[i];
      AllListModeBinsInBinnedModeBin[BinnedModeBin].push_back(i);
      vector<unsigned int> Bins;
      for (unsigned int ii = 0; ii < m_ListModeBins; ++ii) {
        if (i == ii) continue;
        if (ListToBinnedModeBins[ii] == BinnedModeBin) {
          Bins.push_back(ii);
          cout<<"To "<<i<<" adding "<<ii<<endl;
        }
      }
      OtherBins[i] = Bins;
    }    
    
    cout<<"List mode bins in binned mode bins: "<<endl;
    unsigned int TotalBins = 0;
    for (unsigned int i = 0; i < AllListModeBinsInBinnedModeBin.size(); ++i) {
      vector<unsigned int> Bins = AllListModeBinsInBinnedModeBin[i];
      cout<<i<<":  ";
      for (unsigned int lb = 0; lb < Bins.size(); ++lb) {
        cout<<Bins[lb]<<" ";
        TotalBins++;
      }
      cout<<endl;
    }
    cout<<"Total bins: "<<TotalBins<< " vs. "<<m_ListModeBins<<endl;
    
    // Stage 1.3: Setup binned mode MEM analysis
    
    // Set up Lagrange multipliers
    
    MResponseMatrixON Lagrange;
    Lagrange.AddAxis(m_ResponseGalactic.GetAxis(2)); // energy
    Lagrange.AddAxis(m_ResponseGalactic.GetAxis(3)); // phi
    Lagrange.AddAxis(m_ResponseGalactic.GetAxis(4)); // direction scattered gamma ray  
    
    unsigned long P1 = 1;
    unsigned long P2 = P1*FinalEnergyBins;
    unsigned long P3 = P2*FinalPhiBins;
    for (unsigned int fe = 0; fe < FinalEnergyBins; ++fe) {
      unsigned long A1 = P1*fe;
      for (unsigned int fp = 0; fp < FinalPhiBins; ++fp) {
        unsigned long A2 = A1 + P2*fp;
        for (unsigned int fd = 0; fd < FinalDirectionBins; ++fd) {
          unsigned long A3 = A2 + P3*fd;
          Lagrange.Set(A3, 1.0); //Data.Get(A3));
        }
      }
    }
    
    // Normalize response
    m_ResponseGalactic /= m_ResponseGalactic.GetSum(); 
    
    double DataSum = Data.GetSum();
    cout<<"Data Sum="<<DataSum<<endl;   
    
    // calculate expectations
    MResponseMatrixON Expectation;
    Expectation.AddAxis(m_ResponseGalactic.GetAxis(2)); // energy
    Expectation.AddAxis(m_ResponseGalactic.GetAxis(3)); // phi
    Expectation.AddAxis(m_ResponseGalactic.GetAxis(4)); // diretcion scattered gamma ray  
    
    double BackgroundScaler = 1.0;
    
    double MaximumEntropy = 0;
    int MaximumIterations = 0;
    
    
    // Stage 1.3: Setup list-mode ML deconvolution
        
  
    cout<<"Create list mode first backprojection"<<endl;
    
    // Create list-mode image from list-mode data
    // --> Not good since there might be zero's, use flat image

    for (unsigned int i = 0; i < m_ListModeBins; ++i) {
      LMImage.Set(i, 1.0);
    }
    
    // Normalize:
    //LMImage *= Display.GetSum()/LMImage.GetSum()*m_ListModeBins/InitialDirectionBins;
    
    vector<double> LMImageData(m_ListModeBins);
    for (unsigned int i = 0; i < m_ListModeBins; ++i) {
      LMImageData[i] = LMImage.Get(i);
    }
    
    MImageGalactic* LMImageImage = new MImageGalactic();
    LMImageImage->SetTitle("LML First BP");
    LMImageImage->SetXAxisTitle("Galactic Longitude [deg]");
    LMImageImage->SetYAxisTitle("Galactic Latitude [deg]");
    LMImageImage->SetValueAxisTitle("Flux");
    LMImageImage->SetDrawOption(MImage::c_COLZ);
    LMImageImage->SetSpectrum(MImage::c_Rainbow);
    LMImageImage->SetSourceCatalog("$(MEGALIB)/resource/catalogs/Crab.scat");
    LMImageImage->Normalize(false);
    LMImageImage->SetFISBEL(LMImageData);
    LMImageImage->Display();
    
    cout<<"Done!"<<endl;
    gSystem->ProcessEvents();      
    

    
    // Stage 2: Iterations:
    
    for (unsigned int i = 0; i < m_Iterations; ++i) {
      
      // Stage 2.1: Do one binned-mode iteration 
      
      // Step 1: Calculate the restored image:
      
      MResponseMatrixON RestoredImage("RestoredImage");
      RestoredImage.AddAxis(m_Response.GetAxis(0)); // energy
      RestoredImage.AddAxis(m_Response.GetAxis(1)); // image space      
      
      double RestoredImageSum = 0;
      
      unsigned long M1 = 1;
      unsigned long M2 = M1*InitialEnergyBins;
      unsigned long M3 = M2*InitialDirectionBins;
      unsigned long M4 = M3*FinalEnergyBins;
      unsigned long M5 = M4*FinalPhiBins;
      for (unsigned int ie = 0; ie < InitialEnergyBins; ++ie) {
        unsigned long A1 = M1*ie;
        for (unsigned int id = 0; id < InitialDirectionBins; ++id) {
          unsigned long A2 = A1 + M2*id;
          float Content = 0.0;
          unsigned long N1 = 1;
          unsigned long N2 = N1*FinalEnergyBins;
          unsigned long N3 = N2*FinalPhiBins;
          for (unsigned int fe = 0; fe < FinalEnergyBins; ++fe) {
            unsigned long A3 = A2 + M3*fe;
            unsigned long B1 = N1*fe;
            for (unsigned int fp = 0; fp < FinalPhiBins; ++fp) {
              unsigned long A4 = A3 + M4*fp;
              unsigned long B2 = B1 + N2*fp;
              for (unsigned int fd = 0; fd < FinalDirectionBins; ++fd) {
                unsigned long A5 = A4 + M5*fd;
                unsigned long B3 = B2 + N3*fd;
                
                Content += m_ResponseGalactic.Get(A5) * Lagrange.Get(B3);
              }
            }
          }
          
          Content = exp(Content);
          RestoredImage.Set(A2, Content);
          RestoredImageSum += Content;
        }
      }
      
      
      if (RestoredImageSum == 0) {
        cerr<<"ERROR: RestoredImageSum == 0"<<endl;
        return false;
      }
      cout<<"RestoredImageSum="<<RestoredImageSum<<endl;
      
      double Entropy = 0;
      for (unsigned int ie = 0; ie < InitialEnergyBins; ++ie) {
        unsigned long A1 = M1*ie;
        for (unsigned int id = 0; id < InitialDirectionBins; ++id) {
          unsigned long A2 = A1 + M2*id;
          float RestoredImageUpdate = RestoredImage.Get(A2)*DataSum/RestoredImageSum;
          RestoredImage.Set(A2, RestoredImageUpdate);
          Entropy += RestoredImageUpdate*log(RestoredImageUpdate);
        }
      }
      cout<<"Entropy: "<<Entropy<<endl;    
      
      
      // Step 2: Convolve the data again into data space - expectation calculation
      
      unsigned long P1 = 1;
      unsigned long P2 = P1*InitialEnergyBins;
      unsigned long P3 = P2*InitialDirectionBins;
      unsigned long P4 = P3*FinalEnergyBins;
      unsigned long P5 = P4*FinalPhiBins;
      for (unsigned int fe = 0; fe < FinalEnergyBins; ++fe) {
        //unsigned long A3 = P3*fe;
        for (unsigned int fp = 0; fp < FinalPhiBins; ++fp) {
          //unsigned long A4 = P4*fp;
          for (unsigned int fd = 0; fd < FinalDirectionBins; ++fd) {
            //unsigned long A5 = P5*fd;
            
            float NewExpectation = 0;
            for (unsigned int ie = 0; ie < InitialEnergyBins; ++ie) {
              //unsigned long A1 = P1*ie;
              for (unsigned int id = 0; id < InitialDirectionBins; ++id) {
                //unsigned long A2 = P2*id;
                NewExpectation += RestoredImage.Get(ie + InitialEnergyBins*id) * m_ResponseGalactic.Get(ie + P2*id + P3*fe + P4*fp + P5*fd);
              }
            }
            
            if (UseBackgroundModel == true) {
              NewExpectation += BackgroundScaler * m_BackgroundModelGalactic.Get(fe + FinalEnergyBins*fp + FinalEnergyBins*FinalPhiBins*fd);
            }            
            
            Expectation.Set(fe + FinalEnergyBins*fp + FinalEnergyBins*FinalPhiBins*fd, NewExpectation);
          }
        }
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
      double UpdateFactor = 2000;
      unsigned long Q1 = 1;
      unsigned long Q2 = Q1*FinalEnergyBins;
      unsigned long Q3 = Q2*FinalPhiBins;
      for (unsigned int fe = 0; fe < FinalEnergyBins; ++fe) {
        unsigned long A1 = Q1*fe;
        for (unsigned int fp = 0; fp < FinalPhiBins; ++fp) {
          unsigned long A2 = A1 + Q2*fp;
          for (unsigned int fd = 0; fd < FinalDirectionBins; ++fd) {
            unsigned long A3 = A2 + Q3*fd;
            
            bool UseLog = true;
            double Update = 0;
            if (UseLog == true) {
              double PreLog1 = Data.Get(A3)*Scale;
              if (PreLog1 < Limit) PreLog1 = Limit;
              Update += log(PreLog1);
              
              double PreLog2 = Expectation.Get(A3); 
              if (PreLog2 < Limit) PreLog2 = Limit;
              Update -= log(PreLog2);
              
              //cout<<"Lagrange diff: "<<Lagrange.Get(A3)<<" vs. "<<Data.Get(A3)*Scale<<" vs. "<<Expectation.Get(A3)<<" --> Update: "<<Update<<endl;
              
            } else {
              Update += Data.Get(A3)*Scale - Expectation.Get(A3);
              //cout<<"Lagrange diff: "<<Lagrange.Get(A3)<<" vs. "<<Data.Get(A3)*Scale<<" vs. "<<Expectation.Get(A3)<<" --> Update: "<<Update<<endl;
            }
            
            //cout<<"Lagrange diff: "<<Lagrange.Get(A3)<<" vs. "<<Update<<endl;
            double NewL = Lagrange.Get(A3) + UpdateFactor*Update;
            if (NewL < 0) {
              NewL = 0;
            }
            Lagrange.Set(A3, NewL);
            LCounter++;
          }
        }
      }
      cout<<" Updated Lagrange entries: "<<LCounter<<endl;
      cout<<"Lagrange sum="<<Lagrange.GetSum()<<endl;
      
      
      // Step 4: Update the background scaling factor:
      if (UseBackgroundModel == true) {
        double B_corr = 0;
        
        unsigned long N1 = 1;
        unsigned long N2 = N1*FinalEnergyBins;
        unsigned long N3 = N2*FinalPhiBins;
        for (unsigned int fe = 0; fe < FinalEnergyBins; ++fe) {
          unsigned long B1 = N1*fe;
          for (unsigned int fp = 0; fp < FinalPhiBins; ++fp) {
            unsigned long B2 = B1 + N2*fp;
            for (unsigned int fd = 0; fd < FinalDirectionBins; ++fd) {
              unsigned long B3 = B2 + N3*fd;
              
              if (Data.Get(B3) > 0 && Expectation.Get(B3) > 0) {
                B_corr += m_BackgroundModelGalactic.Get(B3) * Data.Get(B3) / Expectation.Get(B3);
              }
            }
          }
        }
        
        BackgroundScaler *= B_corr / m_TotalBackgroundInModel;
        
        cout<<"Background scaling factor: "<<BackgroundScaler<<"!"<<B_corr<<"!"<<m_TotalBackgroundInModel<<endl;
      }      
      
      
      // Step 5: Update the real image:
      MResponseMatrixON NewImage("NewImage");
      NewImage.AddAxis(m_Response.GetAxis(0)); // energy
      NewImage.AddAxis(m_Response.GetAxis(1)); // image space
      
      double ImageFlux = 0;
      for (unsigned int ie = 0; ie < InitialEnergyBins; ++ie) {
        unsigned long A1 = P1*ie;
        for (unsigned int id = 0; id < InitialDirectionBins; ++id) {
          unsigned long A2 = A1 + P2*id;
          NewImage.Set(A2, RestoredImage.Get(A2)); // * DataSum/RestoredImageSum);
          ImageFlux += NewImage.Get(A2) / ObservationTime / StartArea;
        }
      }
      
      if (Entropy > MaximumEntropy) {
        MaximumEntropy = Entropy;
        MaximumIterations = i;
      }
      
      ostringstream Title;
      Title<<"Image at iteration "<<i+1<<" with flux "<<ImageFlux<<" ph/cm2/s";
      //NewImage.ShowSlice(vector<float>{ 511.0, MResponseMatrix::c_ShowX, MResponseMatrix::c_ShowY }, true, Title.str());
      cout<<"Image content: "<<ImageFlux<<" ph/cm2/s"<<endl;
      
      vector<double> ImageData(NewImage.GetAxis(1).GetNumberOfBins());
      for (unsigned int ie = 0; ie < InitialEnergyBins; ++ie) {
        for (unsigned int id = 0; id < InitialDirectionBins; ++id) {
          ImageData[id] = NewImage.Get(ie + InitialEnergyBins*id);
        }
      }
      
      MImageGalactic* G = new MImageGalactic();
      G->SetTitle(MString("Binned mode iteration: ") + (i+1));
      G->SetXAxisTitle("Galactic Longitude [deg]");
      G->SetYAxisTitle("Galactic Latitude [deg]");
      G->SetValueAxisTitle("Flux");
      G->SetDrawOption(MImage::c_COLZ);
      G->SetSpectrum(MImage::c_Rainbow);
      G->Normalize(false);
      G->SetFISBEL(ImageData);
      G->Display();
      
      //NewImage.Write(MString("Image_") + (i+1) + ".rsp");
      
      Display = NewImage;
      
      gSystem->ProcessEvents();
      
      if (m_Interrupt == true) break;
      
      
      
      /////////////
      // Stage 2.2: Normalize each list-mode bin to the binned-mode bin value
      
      for (unsigned int b = 0; b < InitialDirectionBins; ++b) {
        // Find all bins belonging to this binned mode bin
        vector<unsigned int> LMBins = AllListModeBinsInBinnedModeBin[b];
        double ListModeSum = 0.0;
        unsigned int NonZeroBins = 0;
        for (unsigned int lb = 0; lb < LMBins.size(); ++lb) {
          if (LMImage.Get(LMBins[lb]) > 0) {
            ListModeSum += LMImage.Get(LMBins[lb]);
            NonZeroBins++;
          }
        }
        if (ListModeSum > 0) {
          double Norm = Display.Get(b)/ (ListModeSum/NonZeroBins);
          cout<<b<<": n="<<Norm<<"("<<Display.Get(b)<<", "<<ListModeSum<<")"<<endl;
          for (unsigned int lb = 0; lb < LMBins.size(); ++lb) {
            cout<<LMImage.Get(LMBins[lb])<<" --> ";
            LMImage.Set(LMBins[lb], Norm*LMImage.Get(LMBins[lb]));
            cout<<LMImage.Get(LMBins[lb])<<endl;
          }
        }
      }
      
      // Show image:
      for (unsigned int i = 0; i < m_ListModeBins; ++i) {
        LMImageData[i] = LMImage.Get(i);
      }
      
      MImageGalactic* LMImageScaledView = new MImageGalactic();
      LMImageScaledView->SetTitle(MString("LM image after scaling - iteration ") + (i+1));
      LMImageScaledView->SetXAxisTitle("Galactic Longitude [deg]");
      LMImageScaledView->SetYAxisTitle("Galactic Latitude [deg]");
      LMImageScaledView->SetValueAxisTitle("Flux");
      LMImageScaledView->SetDrawOption(MImage::c_COLZ);
      LMImageScaledView->SetSpectrum(MImage::c_Rainbow);
      LMImageScaledView->SetSourceCatalog("$(MEGALIB)/resource/catalogs/Crab.scat");
      LMImageScaledView->Normalize(false);
      LMImageScaledView->SetFISBEL(LMImageData);
      LMImageScaledView->Display();
      
      
      
      /////////////
      // Stage 2.3: Do one list-mode iteration loop:
      
      cout<<"Starting list-mode iteration "<<i+1<<"/"<<m_Iterations<<endl;
      
      // Convolve:
      vector<double> y_mean(m_ListModeResponses.size(), 0);
      for (unsigned int e = 0; e < m_ListModeResponses.size(); ++e) {
        y_mean[e] = 0.0;
        for (unsigned int i = 0; i < m_ListModeBins; ++i) {
          y_mean[e] += LMImage.Get(i) * m_ListModeResponses[e]->Get(i);
        }
      }
      
      for (unsigned int e = 0; e < m_ListModeResponses.size(); ++e) {
        cout<<e<<": ymean="<<y_mean[e]<<endl;
      }
      
      // Deconvolve:
      for (unsigned int j = 0; j < m_ListModeBins; ++j) { // sky bins
        double c = 0.0;
        for (unsigned int e = 0; e < m_ListModeResponses.size(); ++e) {
          if (y_mean[e] > 0) {
            c += m_ListModeResponses[e]->Get(j) / y_mean[e];
          }
        }
        c *= LMImage.Get(j);
        LMImage.Set(j, c);
      }
      
      // Show image:
      for (unsigned int i = 0; i < m_ListModeBins; ++i) {
        LMImageData[i] = LMImage.Get(i) ;
      }
      
      MImageGalactic* LMBackprojectionImage = new MImageGalactic();
      LMBackprojectionImage->SetTitle(MString("LM image - iteration ") + (i+1));
      LMBackprojectionImage->SetXAxisTitle("Galactic Longitude [deg]");
      LMBackprojectionImage->SetYAxisTitle("Galactic Latitude [deg]");
      LMBackprojectionImage->SetValueAxisTitle("Flux");
      LMBackprojectionImage->SetDrawOption(MImage::c_COLZ);
      LMBackprojectionImage->SetSpectrum(MImage::c_Rainbow);
      LMBackprojectionImage->SetSourceCatalog("$(MEGALIB)/resource/catalogs/Crab.scat");
      LMBackprojectionImage->Normalize(false);
      LMBackprojectionImage->SetFISBEL(LMImageData);
      LMBackprojectionImage->Display();
      
      
      
      /////////////
      // Stage 2.4: Normalize the binnned mode image (per bin) with the list mode image
      
      double TotalBMSum = Display.GetSum();
      for (unsigned int b = 0; b < InitialDirectionBins; ++b) {
        // Find all bins belonging to this binned mode bin
        vector<unsigned int> LMBins = AllListModeBinsInBinnedModeBin[b];
        double ListModeSum = 0.0;
        for (unsigned int lb = 0; lb < LMBins.size(); ++lb) {
          ListModeSum += LMImage.Get(LMBins[lb]);  
        }
        Display.Set(b, ListModeSum/LMBins.size());
      }
      Display += TotalBMSum/Display.GetSum();
      
      vector<double> BinnedModeImageScaledData(NewImage.GetAxis(1).GetNumberOfBins());
      for (unsigned int ie = 0; ie < InitialEnergyBins; ++ie) {
        for (unsigned int id = 0; id < InitialDirectionBins; ++id) {
          BinnedModeImageScaledData[id] = Display.Get(ie + InitialEnergyBins*id);
        }
      }
      
      MImageGalactic* BinnedModeImageScaled = new MImageGalactic();
      BinnedModeImageScaled->SetTitle(MString("Binned mode image after scaling iteration: ") + (i+1));
      BinnedModeImageScaled->SetXAxisTitle("Galactic Longitude [deg]");
      BinnedModeImageScaled->SetYAxisTitle("Galactic Latitude [deg]");
      BinnedModeImageScaled->SetValueAxisTitle("Flux");
      BinnedModeImageScaled->SetDrawOption(MImage::c_COLZ);
      BinnedModeImageScaled->SetSpectrum(MImage::c_Rainbow);
      //BinnedModeImageScaled->SetSourceCatalog("$(MEGALIB)/resource/catalogs/Crab.scat");
      //BinnedModeImageScaled->SetProjection(MImageProjection::c_Hammer);
      BinnedModeImageScaled->Normalize(false);
      BinnedModeImageScaled->SetFISBEL(BinnedModeImageScaledData);
      BinnedModeImageScaled->Display();
      
      */
    } // iterations
  } // hybrid mode
    
      
      // Deconvolve with prior
     /* 
      MResponseMatrixON NewBackprojection;
      NewBackprojection.AddAxis(AxisSkyCoordinates);
      
      // We have to solve a simple quadratic equation with
      // a*l^2 + b*l + c = 0
      // with 
      // a = alpha / sigma * 1/N_K;
      // b = sum_i t_ij + alpha/sigma * 1/N_K sum_kelemK_&&_k!=j l_k - mean_K )
      // c = l_j * sum_i t_ij / y_mean_i
      for (unsigned int j = 0; j < m_ListModeBins; ++j) { // sky bins
        double l = 0;
        
        if (OtherBins[j].size() == 0) {
          cout<<"Something wicked happend: No list mode bins in the binned mode bin: Make sure your list-mode bins are at least 3x smaller than the binned mode bins"<<endl;
          LMImage.Set(i, 0);
          continue;
        }
        
        double Sigma = BinnedModeMean.Get(j);
        
        double a = Alpha / Sigma / OtherBins[j].size();
        
        double b = 0.1; // == sum_i t_ij
        double Mean = Alpha / Sigma * BinnedModeMean.Get(j);
        b -= Mean;
        
        double Avg = 0.0;
        for (unsigned int ii = 0; ii < OtherBins[j].size(); ++ii) {
          //cout<<OtherBins[j][ii]<<": "<<Alpha<<":"<<Sigma<<":"<<OtherBins[j].size()<<":"<<LMImage.Get(OtherBins[j][ii])<<endl;
          Avg += Alpha / Sigma / OtherBins[j].size() * LMImage.Get(OtherBins[j][ii]);
        }
        b += Avg;
        //cout<<"Mean: "<<Mean<<" vs. Avg: "<<Avg<<" --> b: "<<b<<endl;
        
        double c = 0; 
        for (unsigned int e = 0; e < m_ListModeResponses.size(); ++e) {
          if (y_mean[e] > 0) {
            c += m_ListModeResponses[e]->Get(j) / y_mean[e];
          }
        }
        c *= LMImage.Get(j);
        
        // Now solve:
        
        //cout<<" a="<<a<<" b="<<b<<" c="<<c<<" --> b*b="<<b*b<<"  4*a*c="<<4*a*c<<endl;
        if (b*b - 4*a*c > 0) {
          l = fabs(0.5 * (-b + sqrt(b*b - 4*a*c)) / a);
        } else {
          l = 0;
        }
        //cout<<i<<": "<<l<<endl;
        
        NewBackprojection.Set(j, l);
      }
      
      LMImage = NewBackprojection;
      
      LMImage *= Display.GetSum()/LMImage.GetSum()*m_ListModeBins/InitialDirectionBins;
      cout<<"Sum: "<<LMFirstBackprojectio
      n.GetSum()<<endl;
      */
      
  
  return true;
}


/******************************************************************************
 * Analyze the event, return true if it has to be writen to file
 */
bool BinnedComptonImaging::AnalyzeEvent(MSimEvent& Event)
{
  // Add your code here
  // Return true if the event should be written to file

  // Example:
  // if (Event.GetVeto() == true) return false;

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
  MGlobal::Initialize();
  
  TApplication BinnedComptonImagingApp("BinnedComptonImagingApp", 0, 0);

  g_Prg = new BinnedComptonImaging();

  if (g_Prg->ParseCommandLine(argc, argv) == false) {
    cerr<<"Error during parsing of command line!"<<endl;
    return -1;
  } 
  if (g_Prg->Analyze() == false) {
    cerr<<"Error during analysis!"<<endl;
    return -2;
  } 

  BinnedComptonImagingApp.Run();

  cout<<"Program exited normally!"<<endl;

  return 0;
}

/*
 * Cosima: the end...
 ******************************************************************************/
