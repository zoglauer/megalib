/* 
 * ExternalAnalysisPipelineExample.cxx
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
#include <vector>
#include <csignal>
using namespace std;

// ROOT
#include <TROOT.h>
#include <TEnv.h>
#include <TSystem.h>
#include <TApplication.h>
#include <TStyle.h>
#include <TCanvas.h>
#include <TH1.h>
#include <TH2.h>

// MEGAlib
#include "MGlobal.h"
#include "MStreams.h"
#include "MDGeometryQuest.h"
#include "MDDetector.h"
#include "MFileEventsSim.h"
#include "MDVolumeSequence.h"
#include "MSimEvent.h"
#include "MSimHT.h"
#include "MGeometryRevan.h"
#include "MRERawEvent.h"
#include "MREHit.h"
#include "MRawEventAnalyzer.h"
#include "MERCSRChiSquare.h"
#include "MImagerExternallyManaged.h"
#include "MBPData.h"
#include "MImage.h"

/******************************************************************************/

class ExternalAnalysisPipelineExample
{
public:
  /// Default constructor
  ExternalAnalysisPipelineExample();
  /// Default destructor
  ~ExternalAnalysisPipelineExample();
  
  /// Parse the command line
  bool ParseCommandLine(int argc, char** argv);
  /// Initialze the run
  bool Initialize();
  /// Analyze what eveer needs to be analyzed...
  bool Analyze();
  /// Interrupt the analysis
  void Interrupt() { m_Interrupt = true; }

private:
  ///
  MRERawEvent* Convert(MSimEvent*);

private:
  /// True, if the analysis needs to be interrupted
  bool m_Interrupt;
  /// The geometry file name
  MString m_GeometryFileName;
  /// The simulation file name
  MString m_SimulationFileName;
  
  /// A standard geometry
  MDGeometryQuest* m_Geometry;
  /// A special geometry for revan
  MGeometryRevan* m_RevanGeometry;
  
  /// The event reconstructor
  MRawEventAnalyzer* m_RawEventAnalyzer;
  
  /// The image reconstructor
  MImagerExternallyManaged* m_Imager;
  
  
  TH1D* m_HistEnergyBefore;
  TH1D* m_HistEnergyAfter;
  TH1D* m_HistEnergyCompton;
};

/******************************************************************************/


/******************************************************************************
 * Default constructor
 */
ExternalAnalysisPipelineExample::ExternalAnalysisPipelineExample() : m_Interrupt(false)
{
}


/******************************************************************************
 * Default destructor
 */
ExternalAnalysisPipelineExample::~ExternalAnalysisPipelineExample()
{
  // Intentionally left blank
}


/******************************************************************************
 * Parse the command line
 */
bool ExternalAnalysisPipelineExample::ParseCommandLine(int argc, char** argv)
{
  ostringstream Usage;
  Usage<<endl;
  Usage<<"  Usage: ExternalAnalysisPipelineExample <options>"<<endl;
  Usage<<"    General options:"<<endl;
  Usage<<"         -g:   geometry file name"<<endl;
  Usage<<"         -s:   simulation file name"<<endl;
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
    if (Option == "-s" || Option == "-g") {
      if (!((argc > i+1) && 
            (argv[i+1][0] != '-' || isalpha(argv[i+1][1]) == 0))){
        cout<<"Error: Option "<<argv[i][1]<<" needs a second argument!"<<endl;
        cout<<Usage.str()<<endl;
        return false;
      }
    } 
    // Multiple arguments template
    /*
    else if (Option == "-??") {
      if (!((argc > i+2) && 
            (argv[i+1][0] != '-' || isalpha(argv[i+1][1]) == 0) && 
            (argv[i+2][0] != '-' || isalpha(argv[i+2][1]) == 0))){
        cout<<"Error: Option "<<argv[i][1]<<" needs two arguments!"<<endl;
        cout<<Usage.str()<<endl;
        return false;
      }
    }
    */

    // Then fulfill the options:
    if (Option == "-s") {
      m_SimulationFileName = argv[++i];
      cout<<"Accepting simulation file name: "<<m_SimulationFileName<<endl;
    } else if (Option == "-g") {
      m_GeometryFileName = argv[++i];
      cout<<"Accepting geometry file name: "<<m_GeometryFileName<<endl;
    } else {
      cout<<"Error: Unknown option \""<<Option<<"\"!"<<endl;
      cout<<Usage.str()<<endl;
      return false;
    }
  }

  return true;
}


/******************************************************************************
 * Do whatever analysis is necessary
 */
bool ExternalAnalysisPipelineExample::Initialize()
{
  // Load geometry:
  m_Geometry = new MDGeometryQuest();

  if (m_Geometry->ScanSetupFile(m_GeometryFileName) == true) {
    cout<<"Geometry "<<m_Geometry->GetName()<<" loaded!"<<endl;
    m_Geometry->ActivateNoising(false);
    m_Geometry->SetGlobalFailureRate(0.0);
  } else {
    cout<<"Loading of geometry "<<m_Geometry->GetName()<<" failed!!"<<endl;
    return false;
  } 
  
  // Load geometry:
  m_RevanGeometry = new MGeometryRevan();

  if (m_RevanGeometry->ScanSetupFile(m_GeometryFileName) == true) {
    cout<<"Geometry "<<m_RevanGeometry->GetName()<<" loaded!"<<endl;
    m_RevanGeometry->ActivateNoising(false);
    m_RevanGeometry->SetGlobalFailureRate(0.0);
  } else {
    cout<<"Loading of geometry "<<m_RevanGeometry->GetName()<<" failed!!"<<endl;
    return false;
  }  
  
  // Initialize the raw event analyzer for HEMI:
  m_RawEventAnalyzer = new MRawEventAnalyzer();
  m_RawEventAnalyzer->SetGeometry(m_RevanGeometry);

  //m_RawEventAnalyzer->SetCoincidenceAlgorithm(MRawEventAnalyzer::c_CoincidenceAlgoWindow);
  //m_RawEventAnalyzer->SetCoincidenceWindow(2E-6);
  
  m_RawEventAnalyzer->SetHitClusteringAlgorithm(MRawEventAnalyzer::c_HitClusteringAlgoNone);
 
  m_RawEventAnalyzer->SetTrackingAlgorithm(MRawEventAnalyzer::c_TrackingAlgoRank);
  m_RawEventAnalyzer->SetDoTracking(true);
  m_RawEventAnalyzer->SetSearchComptonTracks(true);
  m_RawEventAnalyzer->SetSearchPairTracks(true);
  
  m_RawEventAnalyzer->SetCSRAlgorithm(MRawEventAnalyzer::c_CSRAlgoFoM);
  m_RawEventAnalyzer->SetClassicUndecidedHandling(MERCSRChiSquare::c_UndecidedLargerKleinNishinaTimesPhoto);
  m_RawEventAnalyzer->SetAssumeD1First(false);
  m_RawEventAnalyzer->SetGuaranteeStartD1(false);
  m_RawEventAnalyzer->SetUseComptelTypeEvents(true);
  m_RawEventAnalyzer->SetRejectOneDetectorTypeOnlyEvents(false);
  
  m_RawEventAnalyzer->SetDecayAlgorithm(MRawEventAnalyzer::c_DecayAlgoNone);
  
  m_RawEventAnalyzer->SetTotalEnergyMax(3500.0);
  m_RawEventAnalyzer->SetTotalEnergyMin(0.0);

  
  if (m_RawEventAnalyzer->PreAnalysis() == false) return false;
  
  // Some default histograms....
  m_HistEnergyBefore = new TH1D("HistBefore", "HistBefore", 100, 0, 2000);
  m_HistEnergyAfter = new TH1D("HistAfter", "HistAfter", 100, 0, 2000);
  m_HistEnergyCompton = new TH1D("HistCompton", "HistCompton", 100, 0, 2000);
  
  m_Imager = new MImagerExternallyManaged(MCoordinateSystem::c_Spheric);
  m_Imager->SetGeometry(m_Geometry);
    
  // Maths:
  m_Imager->SetApproximatedMaths(true);
    
    // Set the dimensions of the image
  m_Imager->SetViewport(0*c_Rad, 
                        360*c_Rad, 
                        180,
                        0*c_Rad,
                        180*c_Rad,
                        90,
                        c_FarAway/10, 
                        c_FarAway, 
                        1,
                        MVector(0, 1, 0),
                        MVector(-1, 0, 0));
 
    
    // Set the draw modes
    //m_Imager->SetDrawMode(m_Data->GetImageDrawMode());
    //m_Imager->SetPalette(m_Data->GetImagePalette());
    //m_Imager->SetSourceCatalog(m_Data->GetImageSourceCatalog());
    
    //if (Animate == true) {
    //  m_Imager->SetAnimationMode(m_Data->GetAnimationMode());
    //  m_Imager->SetAnimationFrameTime(m_Data->GetAnimationFrameTime());
    //  m_Imager->SetAnimationFileName(m_Data->GetAnimationFileName());
    //} else {
    // m_Imager->SetAnimationMode(MImager::c_AnimateNothing); 
   // }
    
  // Set the response type:
  m_Imager->SetResponseGaussian(3, 30, 3, 2.5, 0.5, false);

  // A new event selector:
  MEventSelector S;
  S.SetGeometry(m_Geometry);
  S.SetComptonAngle(5, 180);
  S.UseComptons(true);
  S.UseTrackedComptons(true);
  S.UseNotTrackedComptons(true);
  S.UsePairs(true);
  S.UsePhotos(false);
  m_Imager->SetEventSelector(S);

  m_Imager->SetDeconvolutionAlgorithmClassicEM();
  m_Imager->SetStopCriterionByIterations(5);

  m_Imager->Initialize();
  
  return true;
}


/******************************************************************************
 * Do whatever analysis is necessary
 */
MRERawEvent* ExternalAnalysisPipelineExample::Convert(MSimEvent* SE)
{
  MRERawEvent* RE = new MRERawEvent();
  
  RE->SetEventTime(SE->GetTime());
  RE->SetEventID(SE->GetID());
  
  // Create raw event hit out of each SimHT and add it to the raw event (RE)
  for (unsigned int h = 0; h < SE->GetNHTs(); ++h) {
    MREHit* REHit = new MREHit();
    REHit->SetDetector(SE->GetHTAt(h)->GetDetectorType());
    REHit->SetPosition(SE->GetHTAt(h)->GetPosition());
    REHit->SetEnergy(SE->GetHTAt(h)->GetEnergy());
    REHit->SetTime(SE->GetHTAt(h)->GetTime() + SE->GetTime().GetAsSeconds());
    REHit->RetrieveResolutions(m_Geometry);
    REHit->Noise(m_Geometry);  // <- for sims only!!
    RE->AddRESE(REHit);
  }
  
  return RE;
}


/******************************************************************************
 * Do whatever analysis is necessary
 */
bool ExternalAnalysisPipelineExample::Analyze()
{
  if (m_Interrupt == true) return false;

  // Initialize:
  if (Initialize() == false) return false;

  MFileEventsSim* SimReader = new MFileEventsSim(m_Geometry);
  if (SimReader->Open(m_SimulationFileName) == false) {
    cout<<"Unable to open sim file!"<<endl; 
    return false;
  }
  cout<<"Opened file "<<SimReader->GetFileName()<<" created with MEGAlib version: "<<SimReader->GetMEGAlibVersion()<<endl;
  SimReader->ShowProgress();

  TCanvas* Backprojection = new TCanvas();
  TCanvas* Iterated= new TCanvas();
  
  
  vector<MPhysicalEvent*> Events;
  vector<MBPData*> ResponseSlices;
  vector<MImage*> Images;
  
  // Loop over the events
  unsigned int ReturnCode;
  MSimEvent* SimEvent = 0;
  MRERawEvent* RawEvent = 0;
  MRERawEvent* BestRawEvent = 0;
  while ((SimEvent = SimReader->GetNextEvent(false)) != 0) {
    // Convert to MRERawEvent
    RawEvent = Convert(SimEvent);
    delete SimEvent;
    if (RawEvent == 0) continue;
    
    m_HistEnergyBefore->Fill(RawEvent->GetEnergy());

    
    // Reconstruct
    m_RawEventAnalyzer->AddRawEvent(RawEvent);
    ReturnCode = m_RawEventAnalyzer->AnalyzeEvent();
    
    if (ReturnCode != MRawEventAnalyzer::c_AnalysisSucess) continue;
    // delete RawEvent; --> it is deleted by the m_RawEventAnalyzer
    
    BestRawEvent = 0;
    if (m_RawEventAnalyzer->GetSingleOptimumEvent() != 0) {
      BestRawEvent = m_RawEventAnalyzer->GetSingleOptimumEvent();
    } else if (m_RawEventAnalyzer->GetSingleBestTryEvent() != 0) {
      BestRawEvent = m_RawEventAnalyzer->GetSingleBestTryEvent();
    }
    if (BestRawEvent != 0) {
      m_HistEnergyAfter->Fill(BestRawEvent->GetEnergy());

      MPhysicalEvent* Phys = BestRawEvent->GetPhysicalEvent();
      Events.push_back(Phys);
      
      // Image
      MBPData* Data = m_Imager->CalculateResponseSlice(Phys);
      if (Data != 0) {
        ResponseSlices.push_back(Data);
      }
      
      if ((ResponseSlices.size() % 100) == 0 && ResponseSlices.size() > 0) {
        for (unsigned int i = 0; i < Images.size(); ++i) delete Images[i];
        m_Imager->ResetStopCriterion();
        cout<<"Content: "<<ResponseSlices.size()<<endl;
        Images = m_Imager->Deconvolve(ResponseSlices);
        Images.front()->Display(Backprojection);
        Images.back()->Display(Iterated);
      }
      
    } else {
      //mout<<"No good event found..."<<endl;
    }
  }
  SimReader->ShowProgress(false);

  // Finalize:

  TCanvas* CanvasEnergyBefore = new TCanvas();
  CanvasEnergyBefore->cd();
  m_HistEnergyBefore->Draw();
  CanvasEnergyBefore->Update();
   
  
  TCanvas* CanvasEnergyAfter = new TCanvas();
  CanvasEnergyAfter->cd();
  m_HistEnergyAfter->Draw();
  CanvasEnergyAfter->Update();

  for (unsigned int i = 0; i < Images.size(); ++i) delete Images[i];
  Images = m_Imager->Deconvolve(ResponseSlices);
  Images.front()->Display(Backprojection);
  Images.back()->Display(Iterated);
  
  return true;
}


/******************************************************************************/

ExternalAnalysisPipelineExample* g_Prg = 0;
int g_NInterruptCatches = 1;

/******************************************************************************/


/******************************************************************************
 * Called when an interrupt signal is flagged
 * All catched signals lead to a well defined exit of the program
 */
void CatchSignal(int a)
{
  if (g_Prg != 0 && g_NInterruptCatches-- > 0) {
    cout<<"Catched signal Ctrl-C (ID="<<a<<"):"<<endl;
    g_Prg->Interrupt();
  } else {
    abort();
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

  // Initialize global MEGALIB variables, especially mgui, etc.
  MGlobal::Initialize();

  TApplication ExternalAnalysisPipelineExampleApp("ExternalAnalysisPipelineExampleApp", 0, 0);

  g_Prg = new ExternalAnalysisPipelineExample();

  if (g_Prg->ParseCommandLine(argc, argv) == false) {
    cerr<<"Error during parsing of command line!"<<endl;
    return -1;
  } 
  if (g_Prg->Analyze() == false) {
    cerr<<"Error during analysis!"<<endl;
    return -2;
  } 

  ExternalAnalysisPipelineExampleApp.Run();

  cout<<"Program exited normally!"<<endl;

  return 0;
}

/*
 * Cosima: the end...
 ******************************************************************************/
