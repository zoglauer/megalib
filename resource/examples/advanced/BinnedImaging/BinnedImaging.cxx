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
  bool RotateBackgroundModelInParallel(unsigned int ThreadID, vector<unsigned int> PointingBinsX, vector<unsigned int> PointingBinsZ);

  /// Create the background model
  bool CreateBackgroundModel();
  
private:
  /// True, if the analysis needs to be interrupted
  bool m_Interrupt;

  //! Just create the background model
  bool m_JustDoBackgroundModel;
  //! Just create the galactic response file
  bool m_JustDoGalacticResponse;
  
  
  /// Simulation file name
  MString m_FileName;
  /// Mimrec configuration file name
  MString m_MimrecCfgFileName;
  /// Response file name
  MString m_ResponseFileName;
  /// Response file name
  MString m_BackgroundModelFileName;
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
  MResponseMatrixON m_BackgroundModel;
  //! The rotated background model in Galactic coordinates
  MResponseMatrixON m_BackgroundModelGalactic;
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
  Usage<<"         -b:   background model"<<endl;
  Usage<<"         -i:   number of iterations (default: 5)"<<endl;
  Usage<<"         -a:   algorithm: rl (default) or mem"<<endl;
  Usage<<"         -cg:  create galactic response file"<<endl;
  Usage<<"         -cb:  create background model"<<endl;
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
      m_BackgroundModelFileName = argv[++i];
      cout<<"Accepting background model file name: "<<m_BackgroundModelFileName<<endl;
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

  Data.Write("BackgroundModel.rsp");
  
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
bool BinnedComptonImaging::RotateBackgroundModelInParallel(unsigned int ThreadID, vector<unsigned int> PointingBinsX, vector<unsigned int> PointingBinsZ)
{ 
  // Dimensions for later:
  unsigned int FinalEnergyBins = m_BackgroundModel.GetAxis(0).GetNumberOfBins();
  unsigned int FinalPhiBins = m_BackgroundModel.GetAxis(1).GetNumberOfBins();
  unsigned int FinalDirectionBins = m_BackgroundModel.GetAxis(2).GetNumberOfBins();
  
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
      vector<double> GalacticPointing = m_BackgroundModelGalactic.GetAxis(2).GetBinCenters(id);
      //cout<<GalacticPointing[0]<<":"<<GalacticPointing[1]<<endl;
      MVector GalIn;
      GalIn.SetMagThetaPhi(1.0, GalacticPointing[0]*c_Rad, GalacticPointing[1]*c_Rad);
      MVector LocalIn = R*GalIn;
      
      //cout<<R*GalIn<<" vs. "<<R2*GalIn<<" --> "<<LocalIn.Angle(R2*GalIn)*c_Deg<<endl;
      
      // Find that bin in the new response:
      RotatedBinMapping.push_back(m_BackgroundModel.GetAxis(2).GetAxisBin(LocalIn.Theta()*c_Deg, LocalIn.Phi()*c_Deg));
      
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
              
          double ModelData = m_BackgroundModel.Get(A3)*PointingScaler; // Normalize by time in pointing  
          if (ModelData == 0) continue;
              
          m_ThreadMutex.lock();
          m_BackgroundModelGalactic.Add(B3, ModelData); 
          m_ThreadMutex.unlock();
        }
      }
    }

  }
  
  m_ThreadRunning[ThreadID] = false;
  
  return true;
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
  if (m_BackgroundModelFileName != "") UseBackgroundModel = true;
  if (UseBackgroundModel == true) {
    if (m_BackgroundModel.Read(m_BackgroundModelFileName) == false) {
      mgui<<"Error: Cannot read background model file: \""<<m_BackgroundModelFileName<<"\""<<endl;
      return false;
    }
    // Check if the dimensions are identical:
    if (m_Response.GetAxis(2) != m_BackgroundModel.GetAxis(0) ||
        m_Response.GetAxis(3) != m_BackgroundModel.GetAxis(1) ||
        m_Response.GetAxis(4) != m_BackgroundModel.GetAxis(2)) {
      mgui<<"Error: The response and background model axes arre not identical"<<endl;
      return false;
    }
    
    m_BackgroundModelGalactic.SetName("Response Galactic");
    m_BackgroundModelGalactic.AddAxis(m_BackgroundModel.GetAxis(0)); // energy
    m_BackgroundModelGalactic.AddAxis(m_BackgroundModel.GetAxis(1)); // phi
    m_BackgroundModelGalactic.AddAxis(m_BackgroundModel.GetAxis(2)); // direction scattered gamma ray IN GALACTIC coordinates
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
      
      if (ComptonEvent->HasGalacticPointing() == true) {
        MRotation R = ComptonEvent->GetGalacticPointingRotationMatrix();
        Dg = R*Dg;
      } else {
        cout<<"Event has no Galactic pointing!"<<endl;
        delete Event;
        continue;        
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

  double m_TotalBackgroundInModel = 0;
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
    if (Split == 0) {
      m_ThreadRunning.resize(1, true);
      RotateBackgroundModelInParallel(0, PointingBinsX, PointingBinsZ);
    } else {
      vector<thread> Threads(std::thread::hardware_concurrency());
      m_ThreadRunning.resize(Threads.size(), true);
      for (unsigned int t = 0; t < Threads.size(); ++t) {
        m_ThreadRunning[t] = true;
        vector<unsigned int> X(PointingBinsX.begin() + t*Split, (t == Threads.size() - 1) ? PointingBinsX.end() : PointingBinsX.begin() + (t+1)*Split);
        vector<unsigned int> Z(PointingBinsZ.begin() + t*Split, (t == Threads.size() - 1) ? PointingBinsZ.end() : PointingBinsZ.begin() + (t+1)*Split);
        Threads[t] = thread(&BinnedComptonImaging::RotateBackgroundModelInParallel, this, t, X, Z);
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
    m_BackgroundModelGalactic.Write("BackgroundModelGalactic.rsp");
    cout<<"Number of directions in pointings file: "<<Dirs<<endl;
    
    m_TotalBackgroundInModel = m_BackgroundModelGalactic.GetSum();
    
    
    // Backproject background model
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
    
    double BackgroundScaler = 1.0;
    
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
      G->SetTitle("Galaxy view");
      G->SetXAxisTitle("Galactic Longitude [deg]");
      G->SetYAxisTitle("Galactic Latitude [deg]");
      G->SetValueAxisTitle("Flux");
      G->SetDrawOption(MImage::c_COLZ);
      G->SetSpectrum(MImage::c_Rainbow);
      G->SetSourceCatalog("$(MEGALIB)/resource/catalogs/Crab.scat");
      G->Normalize(false);
      G->SetFISBEL(ImageData);
      G->Display();
      
      //NewImage.Write(MString("Image_") + (i+1) + ".rsp");
      
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
    
    double BackgroundScaler = 1.0;
    
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
              NewMean += BackgroundScaler * m_BackgroundModelGalactic.Get(fe + FinalEnergyBins*fp + FinalEnergyBins*FinalPhiBins*fd);
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
              
              if (Data.Get(B3) > 0 && Mean.Get(B3) > 0) {
                B_corr += m_BackgroundModelGalactic.Get(B3) * Data.Get(B3) / Mean.Get(B3);
              }
            }
          }
        }
        
        BackgroundScaler *= B_corr / m_TotalBackgroundInModel;
        
        cout<<"Background scaling factor: "<<BackgroundScaler<<"!"<<B_corr<<"!"<<m_TotalBackgroundInModel<<endl;
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
      G->SetSourceCatalog("$(MEGALIB)/resource/catalogs/Crab.scat");
      G->Normalize(false);
      G->SetFISBEL(ImageData);
      G->Display();
      
      //Display.Write(MString("Image_") + (i+1) + ".rsp");
      
      gSystem->ProcessEvents();
      
      if (m_Interrupt == true) break;
    }
  }
  
  gSystem->ProcessEvents();  
  
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
