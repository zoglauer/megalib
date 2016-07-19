/* 
 * SimpleComptonImaging.cxx
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
#include <TH2.h>
#include <TCanvas.h>
#include <MString.h>
#include <TRotation.h>
#include <TMatrix.h>
#include <TMath.h>

// MEGAlib
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



/******************************************************************************/

class SimpleComptonImaging
{
public:
  /// Default constructor
  SimpleComptonImaging();
  /// Default destructor
  ~SimpleComptonImaging();
  
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

private:
  /// True, if the analysis needs to be interrupted
  bool m_Interrupt;

  /// Simulation file name
  MString m_FileName;
  /// The angular resolution as FWHM of ARM
  double m_AngularResolution;
  /// The number of iterations the algorithm has to run
  unsigned int m_Iterations;
};

/******************************************************************************/


/******************************************************************************
 * Default constructor
 */
SimpleComptonImaging::SimpleComptonImaging() : m_Interrupt(false)
{
  gStyle->SetPalette(1, 0);
  
  m_AngularResolution = 5;
  m_Iterations = 5;
}


/******************************************************************************
 * Default destructor
 */
SimpleComptonImaging::~SimpleComptonImaging()
{
  // Intentionally left blanck
}


/******************************************************************************
 * Parse the command line
 */
bool SimpleComptonImaging::ParseCommandLine(int argc, char** argv)
{
  ostringstream Usage;
  Usage<<endl;
  Usage<<"  Usage: SimpleComptonImaging <options>"<<endl;
  Usage<<"    General options:"<<endl;
  Usage<<"         -f:   tra file name"<<endl;
  Usage<<"         -a:   angular resolution as FWHM of ARM (default 5)"<<endl;
  Usage<<"         -i:   number of iterations (default: 5)"<<endl;
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
    } else if (Option == "-a") {
      m_AngularResolution = atof(argv[++i]);
      cout<<"Accepting angular resolution: "<<m_AngularResolution<<endl;
    } else if (Option == "-i") {
      m_Iterations = atoi(argv[++i]);
      cout<<"Accepting iterations: "<<m_Iterations<<endl;
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

  return true;
}


/******************************************************************************
 * Do whatever analysis is necessary
 */
bool SimpleComptonImaging::Analyze()
{
  
  // if (m_Interrupt == true) return false;

  // Open the tra file
  MFileEventsTra* EventFile = new MFileEventsTra();
  if (EventFile->Open(m_FileName) == false) return false;
  EventFile->ShowProgress();

  // Inialize the backprojection image
  unsigned int BinsPhi = 180;
  double PhiMin = -180;
  double PhiMax = +180;
  
  unsigned int BinsTheta = 90;
  double ThetaMin = 0.0;
  double ThetaMax = 180.0;

  TH2D* AllSky = new TH2D("AllSkyImage", "All sky image", BinsPhi, PhiMin, PhiMax, BinsTheta, ThetaMin, ThetaMax);
  vector<TH2D*> Backprojections; // Stores all backprojections


  // Backproject all events:
  cout<<"Creating backprojections..."<<endl;
  
  double Ee; // Energy of recoil electron
  double Eg; // Energy of scattered gamma ray
  MVector P1; // Position of first interaction
  MVector P2; // Position of second interaction

  MPhysicalEvent* Event = 0;
  MComptonEvent* ComptonEvent = 0;
  while ((Event = EventFile->GetNextEvent()) != 0) {
    if (Event->GetType() == MPhysicalEvent::c_Compton) {
      ComptonEvent = dynamic_cast<MComptonEvent*>(Event);

      Ee = ComptonEvent->Ee();
      Eg = ComptonEvent->Eg();
      P1 = ComptonEvent->C1();
      P2 = ComptonEvent->C2(); 
      
      double Angle1 = 1 - 511/Eg + 511/(Ee + Eg);
      if (Angle1 < 1 && Angle1 > -1) {
        Angle1 = acos(Angle1);
        MVector Direction1 = P1 - P2;
        
        // Create a new histogram holding the backprojection
        ostringstream HistName;
        HistName<<"BP"<<Backprojections.size()+1;
        TH2D* Hist = new TH2D(HistName.str().c_str(), HistName.str().c_str(), BinsPhi, PhiMin, PhiMax, BinsTheta, ThetaMin, ThetaMax);
        
        for (int bx = 1; bx <= Hist->GetNbinsX(); ++bx) {
          for (int by = 1; by <= Hist->GetNbinsY(); ++by) {
            MVector Voxel;
            Voxel.SetMagThetaPhi(1.0, Hist->GetYaxis()->GetBinCenter(by)*TMath::DegToRad(), Hist->GetXaxis()->GetBinCenter(bx)*TMath::DegToRad());
            Voxel = Voxel.Unit();
            double Dist = fabs(Voxel.Angle(Direction1) - Angle1);
            // Calculate only if the distance to the cone < 3 sigma ARM
            if (Dist < 3*m_AngularResolution/2.35*TMath::DegToRad()) {
              Hist->SetBinContent(bx, by, Hist->GetBinContent(bx, by)+TMath::Gaus(Dist, 0, m_AngularResolution/2.35 * TMath::DegToRad(), true)); 
            }
          } // loop image y
        } // loop image x

        Backprojections.push_back(Hist);

      } // if cos(Angle) OK
    } // if Compton
    delete Event;
  }

  // Create an initial backprojection
  cout<<"Creating initial backprojection..."<<endl;
  for (int bx = 1; bx <= AllSky->GetNbinsX(); ++bx) {
     for (int by = 1; by <= AllSky->GetNbinsY(); ++by) {
       double Content = 0.0;
       for (unsigned int i = 0; i < Backprojections.size(); ++i) {
          Content += Backprojections[i]->GetBinContent(bx, by);
       }
       AllSky->SetBinContent(bx, by, Content);
     }
  }

  TCanvas* AllSkyCanvas = new TCanvas();
  AllSkyCanvas->cd();
  AllSky->Draw("colz");
  AllSkyCanvas->Update();

  // The iterations
  unsigned int NEvents = Backprojections.size();
  vector<double> y_mean(NEvents);
  
  for (unsigned int i = 0; i < m_Iterations; ++i) {
    cout<<"Iteration: "<<i+1<<endl;
    // Convolve:
    for (unsigned int e = 0; e < NEvents; ++e) {
      y_mean[e] = 0.0;
      for (int bx = 1; bx <= AllSky->GetNbinsX(); ++bx) {
        for (int by = 1; by <= AllSky->GetNbinsY(); ++by) {
          y_mean[e] += AllSky->GetBinContent(bx, by) * Backprojections[e]->GetBinContent(bx, by);
        }
      }
    }
                
    // Deconvolve:
    for (int bx = 1; bx <= AllSky->GetNbinsX(); ++bx) {
      for (int by = 1; by <= AllSky->GetNbinsY(); ++by) {
        double Content = 0.0;  
        for (unsigned int e = 0; e < NEvents; ++e) {
          if (y_mean[e] > 0) {
            Content += Backprojections[e]->GetBinContent(bx, by) / y_mean[e];
          }
         }
        AllSky->SetBinContent(bx, by, Content * AllSky->GetBinContent(bx, by));
      }
    }

    AllSky->Draw("colz");
    AllSkyCanvas->Update();
  }
  
  return true;
}


/******************************************************************************
 * Analyze the event, return true if it has to be writen to file
 */
bool SimpleComptonImaging::AnalyzeEvent(MSimEvent& Event)
{
  // Add your code here
  // Return true if the event should be written to file

  // Example:
  // if (Event.GetVeto() == true) return false;

  return true;
}


/******************************************************************************/

SimpleComptonImaging* g_Prg = 0;

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

  TApplication SimpleComptonImagingApp("SimpleComptonImagingApp", 0, 0);

  g_Prg = new SimpleComptonImaging();

  if (g_Prg->ParseCommandLine(argc, argv) == false) {
    cerr<<"Error during parsing of command line!"<<endl;
    return -1;
  } 
  if (g_Prg->Analyze() == false) {
    cerr<<"Error during analysis!"<<endl;
    return -2;
  } 

  SimpleComptonImagingApp.Run();

  cout<<"Program exited normally!"<<endl;

  return 0;
}

/*
 * Cosima: the end...
 ******************************************************************************/
