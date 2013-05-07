/* 
 * ResponseGenerator.cxx
 *
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 *
 * This code implementation is the intellectual propery of
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
#include <limits>
#include <iomanip>
#include <algorithm>
using namespace std;

// ROOT
#include <TSystem.h>
#include <TEnv.h>
#include <TApplication.h>
#include <TH1.h>
#include <TH2.h>
#include <TCanvas.h>
#include <TStyle.h>
#include <TROOT.h>
#include <TMath.h>

// MEGAlib
#include "MAssert.h"
#include "MRESE.h"
#include "MRESEIterator.h"
#include "MGeometryRevan.h"
#include "MRERawEvent.h"
#include "MRawEventAnalyzer.h"
#include "MFileEventsSim.h"
#include "MRETrack.h"
#include "MStreams.h"
#include "MResponseMatrixO1.h"
#include "MResponseMatrixO2.h"
#include "MResponseMatrixO3.h"
#include "MResponseMatrixO4.h"
#include "MResponseMatrixO5.h"
#include "MResponseMatrixO6.h"
#include "MResponseMatrixO7.h"
#include "MComptonEvent.h"
#include "MPhysicalEvent.h"
#include "MPairEvent.h"
#include "MSettingsRevan.h"
#include "MSettingsMimrec.h"
#include "MEventSelector.h"
#include "MERCSRBayesian.h"


/******************************************************************************/

class ResponseGenerator
{
public:
  /// Default constructor
  ResponseGenerator();
  /// Default destructor
  ~ResponseGenerator();
  
  /// Parse the command line
  bool ParseCommandLine(int argc, char** argv);
  /// Analyze what eveer needs to be analyzed...
  bool Analyze();
  /// Interrupt the analysis
  void Interrupt() { m_Interrupt = true; }
  /// Load the next set of events:
  bool LoadEvents();
  /// Load the geometry, return 0 on failure
  MGeometryRevan* LoadGeometry(bool ActivateNoise, double GlobalFailureRate);
  /// Load the sim file:
  bool OpenSimulationFile();

  const static int c_Unknown           = 0;
  const static int c_NoTrackDual       = 1;
  const static int c_NoTrackMultiple   = 2;
  const static int c_TrackDual         = 3;
  const static int c_TrackMultiple     = 4;

  const static int c_ModeUnknown       = 0;
  const static int c_ModeTracks        = 1;
  const static int c_ModeComptons      = 2;
  const static int c_ModeVerify        = 3;
  const static int c_ModeImaging       = 4;
  const static int c_ModeEarthHorizon  = 5;

  const static float c_NoBound;

private:
  bool DoSimulationSanityCheck();

  bool GenerateTrackPdf();
  bool GenerateComptonPdf();
  bool GenerateImagingPdf();
  bool GenerateEarthHorizonPdf();

  bool VerifyTrackReconstruction();
  bool VerifyComptonReconstruction();

  bool IsTrackStart(MRESE& Start, MRESE& Central, double Energy);
  bool AreReseInSequence(MRESE& Start, MRESE& Central, MRESE& Stop, double Energy);
  bool IsTrackStop(MRESE& Central, MRESE& Stop, double Energy);

  bool AreIdsInSequence(const vector<int>& Ids);
  vector<int> GetReseIds(MRESE* RESE);
  vector<int> GetOriginIds(MRESE* RESE);
  bool IsTrackCompletelyAbsorbed(const vector<int>& Ids, double Energy);

  double CalculateAngleIn(MRESE& Start, MRESE& Central);
  double CalculateAngleOutPhi(MRESE& Start, MRESE& Central, MRESE& Stop);
  double CalculateAngleOutTheta(MRESE& Start, MRESE& Central, MRESE& Stop);


  bool IsComptonStart(MRESE& Start, double Etot, double Eres);
  bool IsComptonTrack(MRESE& Start, MRESE& Central, int StartPosition, 
                      double Etot, double Eres);
  bool IsComptonEnd(MRESE& Stop);
  bool IsComptonSequence(MRESE& Start, MRESE& Stop,
                         int StartPosition, double StopEnergy, double Eres);
  bool IsComptonSequence(MRESE& Start, MRESE& Central, MRESE& Stop,
                         int StartPosition, double StopEnergy, double Eres);
  bool IsSingleCompton(MRESE& Start);

  bool AreInComptonSequence(const vector<int>& StartOriginIds, 
                            const vector<int>& CentralOriginIds,
                            int StartPosition);
  bool ContainsOnlyComptonDependants(vector<int> AllSimIds);
  bool IsAbsorbed(const vector<int>& AllSimIds, double Energy, double EnergyResolution);
  bool IsTotalAbsorbed(const vector<int>& AllSimIds, double Energy, double EnergyResolution);

  double CalculateDPhi(MRESE& Start, MRESE& Central, MRESE& Stop, double Etot);
  double CalculateDPhiInDegree(MRESE& Start, MRESE& Central, MRESE& Stop, double Etot);
  double CalculatePhiE(MRESE& Central, double Etot);
  double CalculatePhiEInDegree(MRESE& Central, double Etot);
  double CalculateAlphaE(MRETrack& Start, MRESE& Central, double Etot);
  double CalculateAlphaEInDegree(MRETrack& Start, MRESE& Central, double Etot);
  double CalculateAlphaG(MRETrack& Start, MRESE& Central, double Etot);
  double CalculateAlphaGInDegree(MRETrack& Start, MRESE& Central, double Etot);
  double CalculateDAlpha(MRETrack& Start, MRESE& Central, double Etot);
  double CalculateDAlphaInDegree(MRETrack& Start, MRESE& Central, double Etot);
  double CalculateMinLeverArm(MRESE& Start, MRESE& Central, MRESE& Stop);
  double CalculatePhotoDistance(MRESE& Start, MRESE& Stop, double Etot);
  double CalculateComptonDistance(MRESE& Start, MRESE& Stop, double Etot);

  int  IsTrackOk(MRERawEvent* RE, MRETrack* Track, bool& Escape, bool& StartMissing, bool& Curling, bool& Angle, bool& Mix);
  bool IsRevanHit(MRERawEvent* RE, MVector Pos); // For 2D strip only!
  bool IsTrackCompletelyAbsorbed(int SimId);
  double GetIdealDepositedEnergy(int MinId);


  bool IsSequenceOk(MRERawEvent* RE);
  bool IsAbsorbed(MRERawEvent* RE);
  bool SeveralComptonsInStart();
  bool SeveralComptonsInCenter();
  bool MissingStartHit();
  bool HighDoppler();
  bool Track();
  bool LongTrack();
  bool AddOnHit();

  bool StartHitMissingEnergy();
  bool LastEscape();

  int GetMaterial(MRESE& RESE);

  vector<float> CreateLogDist(float Min, float Max, int Bins, 
                              float MinBound = c_NoBound, float MaxBound = c_NoBound,
                              float Offset = 0, bool Inverted = false);
  vector<float> CreateEquiDist(float Min, float Max, int Bins, 
                               float MinBound = c_NoBound, float MaxBound = c_NoBound,
                               float Offset = 0, bool Inverted = false);

private:
  /// True, if the analysis needs to be interrupted
  bool m_Interrupt;
  /// Name of the geometry file:
  string m_GeometryFileName;
  /// Name of the input file:
  string m_FileName;
  /// Name of the respons file:
  string m_ResponseName;

  /// Current operation mode
  int m_Mode;
  bool m_Verify;


  //
  int m_EventType;


  // Diagnostics
  int m_NERAll;
  int m_NERPassed;
  int m_NERPassedNoTrackDual;
  int m_NERPassedNoTrackMultiple;
  int m_NERPassedTrackDual;
  int m_NERPassedTrackMultiple;

  int m_NERPassedNoTrackDualWS;
  int m_NERPassedNoTrackMultipleWS;
  int m_NERPassedTrackDualWS;
  int m_NERPassedTrackMultipleWS;
  int m_NERPassedTrackDualWT;
  int m_NERPassedTrackMultipleWT;
  int m_NERPassedTrackDualWST;
  int m_NERPassedTrackMultipleWST;

  //
  MRawEventAnalyzer* m_ReReader;
  MFileEventsSim* m_SiReader;

  MEventSelector m_MimrecEventSelector;

  MGeometryRevan* m_SiGeometry;
  MGeometryRevan* m_ReGeometry;

  //! The current sivan event
  MSimEvent* m_SiEvent;
  //! The current revan event
  MRERawEvent* m_ReEvent;
  //! The IDs of all RESEs
  map<MRESE*, vector<int> > m_Ids;
  //! The origin IDs of all RESEs
  map<MRESE*, vector<int> > m_OriginIds;

  int m_MaxNEvents;
  int m_StartEvent;

  int m_CSRMaxLength;
  double m_CSRThresholdMin;
  double m_CSRThresholdMax;

  double m_MaxEnergyDifference;
  double m_MaxEnergyDifferencePercent;

  double m_MaxTrackEnergyDifference;
  double m_MaxTrackEnergyDifferencePercent;

  MString m_RevanCfgFileName;
  MString m_MimrecCfgFileName;

  bool m_NoAbsorptions;
  int m_MaxAbsorptions;

  int m_SaveAfter;
};


/******************************************************************************/

class Triple
{
public:
  Triple(int Start, int Central, int Stop) {
    massert(Start > 0); massert(Central > 0); massert(Stop > 0);
    massert(Start != Central); massert(Stop != Central); massert(Start != Stop);

    m_Start = Start;
    m_Stop = Stop;
    m_Central = Central;
  }
  // Copy constuctor, destructor and Assignment operator 
  // should be correctly defined by the compiler

  bool operator==(const Triple& T) {
    if (m_Start == T.m_Start &&
        m_Central == T.m_Central &&
        m_Stop == T.m_Stop) {
      return true;
    }

    return false;
  }

  int GetStart() { return m_Start; }
  int GetCentral() { return m_Central; }
  int GetStop() { return m_Stop; }

  friend ostream& operator<<(ostream& os, Triple& T);

private:
  int m_Start;
  int m_Central;
  int m_Stop;
};

/******************************************************************************/

ostream& operator<<(ostream& os, Triple& T)
{
  os<<"("<<T.m_Start<<", "<<T.m_Central<<", "<<T.m_Stop<<")";
  return os;
}

/******************************************************************************/

class Quadruple
{
public:
  Quadruple(int Start, int Central, int Stop, int Etot = 0) {
    massert(Start > 0); massert(Central > 0); massert(Stop > 0);
    massert(Start != Central); massert(Stop != Central); massert(Start != Stop);

    m_Start = Start;
    m_Stop = Stop;
    m_Central = Central;
    m_Etot = Etot;
  }
  // Copy constuctor, destructor and Assignment operator 
  // should be correctly defined by the compiler

  bool operator==(const Quadruple& T) {
    if (m_Start != T.m_Start) return false;
    if (m_Central != T.m_Central) return false;
    if (m_Stop != T.m_Stop) return false;
    if (m_Etot != T.m_Etot) return false;
    return true;
  }

  int GetStart() { return m_Start; }
  int GetCentral() { return m_Central; }
  int GetStop() { return m_Stop; }
  int GetEtot() { return m_Etot; }

  friend ostream& operator<<(ostream& os, Quadruple& T);

private:
  int m_Start;
  int m_Central;
  int m_Stop;
  int m_Etot;
};


/******************************************************************************/

ostream& operator<<(ostream& os, Quadruple& T)
{
  os<<"("<<T.m_Start<<", "<<T.m_Central<<", "<<T.m_Stop<<", "<<T.m_Etot<<")";
  return os;
}
    

/******************************************************************************/


const float ResponseGenerator::c_NoBound = numeric_limits<float>::max()/3.1;


/******************************************************************************
 * Default constructor
 */
ResponseGenerator::ResponseGenerator() : m_Interrupt(false)
{
  // Intentionally left blanck

  m_Mode = c_ModeUnknown;
  m_Verify = false;

  m_ReReader = 0;
  m_SiReader = 0;

  m_ReEvent = 0;
  m_SiEvent = 0;

  m_SiGeometry = 0;
  m_ReGeometry = 0;

  // Event types:
  m_NERAll = 0;
  m_NERPassed = 0;
  m_NERPassedNoTrackDual = 0;
  m_NERPassedNoTrackMultiple = 0;
  m_NERPassedTrackDual = 0;
  m_NERPassedTrackMultiple = 0;

  m_NERPassedNoTrackDualWS = 0;
  m_NERPassedNoTrackMultipleWS = 0;
  m_NERPassedTrackDualWS = 0;
  m_NERPassedTrackMultipleWS = 0;
  m_NERPassedTrackDualWT = 0;
  m_NERPassedTrackMultipleWT = 0;
  m_NERPassedTrackDualWST = 0;
  m_NERPassedTrackMultipleWST = 0;

  m_CSRMaxLength = 7;
  m_CSRThresholdMin = 0.0;
  m_CSRThresholdMax = 50.0;

  m_StartEvent = 0;
  m_MaxNEvents = numeric_limits<int>::max();

  m_EventType = c_Unknown;

  m_MaxEnergyDifference = 5; // keV
  m_MaxEnergyDifferencePercent = 0.02;

  m_MaxTrackEnergyDifference = 30; // keV
  m_MaxTrackEnergyDifferencePercent = 0.1;

  m_NoAbsorptions = false;
  m_MaxAbsorptions = 5;

  m_SaveAfter = 100000;

  m_RevanCfgFileName = g_StringNotDefined;
  m_MimrecCfgFileName = g_StringNotDefined;

  gStyle->SetPalette(1, 0);
}


/******************************************************************************
 * Default destructor
 */
ResponseGenerator::~ResponseGenerator()
{
  // Intentionally left blanck
}


/******************************************************************************
 * Parse the command line
 */
bool ResponseGenerator::ParseCommandLine(int argc, char** argv)
{
  ostringstream Usage;
  Usage<<endl;
  Usage<<"  Usage: ResponseGenerator <options>"<<endl;
  Usage<<"    General options:"<<endl;
  Usage<<"      -g  --geometry        file  m  geometry file name"<<endl;
  Usage<<"      -f  --filename        file  m  file name"<<endl;  
  Usage<<"      -d  --debug                    enable debug"<<endl;
  Usage<<"      -r  --response-name   file     response file name"<<endl;
  Usage<<"      -v  --verify                   verify"<<endl;
  Usage<<"      -m  --mode            char  m  modes: t (= track) or c (= compton) or i (= imaging) or e (= earth horizon)"<<endl;
  Usage<<"      -i  --max-id          int      do the analysis up to id"<<endl;
  Usage<<"      -c  --revan-config    file     use this revan configuration file instead of defaults"<<endl;
  Usage<<"      -b  --mimrec-config   file     use this mimrec configuration file instead of defaults for the imaging response"<<endl;
  Usage<<"      -s  --save            int      save after this amount of entries"<<endl;
  Usage<<"          --no-absorptions           don't calculate absoption probabilities"<<endl;
  Usage<<"      -h  --help                     print this help"<<endl;
  Usage<<endl;

  string Option, SubOption;

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
    if (Option == "-f" || Option == "--filename" ||
        Option == "-g" || Option == "--geometry" ||
        Option == "-r" || Option == "--response-name" ||
        Option == "-m" || Option == "--mode" ||
        Option == "-i" || Option == "--max-id" ||
        Option == "-c" || Option == "--revan-config" ||
        Option == "-b" || Option == "--mimrec-config" ||
        Option == "-s" || Option == "--save") {
      if (!((argc > i+1) && argv[i+1][0] != '-')){
        cout<<"Error: Option "<<argv[i][1]<<" needs a second argument!"<<endl;
        cout<<Usage.str()<<endl;
        return false;
      }
    } 
    // Multiple arguments_
    //else if (Option == "-??") {
    //  if (!((argc > i+2) && argv[i+1][0] != '-' && argv[i+2][0] != '-')){
    //    cout<<"Error: Option "<<argv[i][1]<<" needs two arguments argument!"<<endl;
    //    cout<<Usage.str()<<endl;
    //    return false;
    //  }
    //}

    // Then fulfill the options:
    if (Option == "-f" || Option == "--filename") {
      m_FileName = argv[++i];
      cout<<"Accepting file name: "<<m_FileName<<endl;
    } else if (Option == "-g" || Option == "--geometry") {
      m_GeometryFileName = argv[++i];
      cout<<"Accepting geometry file name: "<<m_GeometryFileName<<endl;
    } else if (Option == "-c" || Option == "--revan-config") {
      m_RevanCfgFileName = argv[++i];
      cout<<"Accepting revan configuration file name: "<<m_RevanCfgFileName<<endl;
    } else if (Option == "-b" || Option == "--mimrec-config") {
      m_MimrecCfgFileName = argv[++i];
      cout<<"Accepting mimrec configuration file name: "<<m_MimrecCfgFileName<<endl;
    } else if (Option == "-r" || Option == "--response-name") {
      m_ResponseName = argv[++i];
      cout<<"Accepting response name: "<<m_ResponseName<<endl;
    } else if (Option == "-i" || Option == "--max-id") {
      m_MaxNEvents = atoi(argv[++i]);
      cout<<"Simulating up to max number of events: "<<m_MaxNEvents<<endl;
    } else if (Option == "-s" || Option == "--save") {
      m_SaveAfter = atoi(argv[++i]);
      cout<<"Saving after this amount of events: "<<m_SaveAfter<<endl;
    } else if (Option == "-m" || Option == "--mode") {
      SubOption = argv[++i];
      if (SubOption == "t") {
        m_Mode = c_ModeTracks;
        cout<<"Choosing Track mode"<<endl;
      } else if (SubOption == "c") {
        m_Mode = c_ModeComptons;
        cout<<"Choosing Compton mode"<<endl;
      } else if (SubOption == "i") {
        m_Mode = c_ModeImaging;
        cout<<"Choosing Imaging mode"<<endl;
      } else if (SubOption == "e") {
        m_Mode = c_ModeEarthHorizon;
        cout<<"Choosing Earth Horizon mode"<<endl;
      } else {
        cout<<"Error: Unknown suboption \""<<SubOption<<"\"!"<<endl;
        cout<<Usage.str()<<endl;
        return false;
      }
    } else if (Option == "-v") {
      m_Verify = true;
      cout<<"Choosing Verification mode"<<endl;
    } else if (Option == "--no-absorptions") {
      m_NoAbsorptions = true;
      cout<<"Calculating no absorptions"<<endl;
    } else if (Option == "-d") {
      if (g_DebugLevel < 2) g_DebugLevel = 2;
      cout<<"Enabling debug!"<<endl;
      mdebug<<"Debug enabled!"<<endl;
    } else {
      cout<<"Error: Unknown option \""<<Option<<"\"!"<<endl;
      cout<<Usage.str()<<endl;
      return false;
    }
  }
  
  if (m_Mode == c_ModeUnknown) {
    cout<<"Error: you have to define a mode!"<<endl;
    cout<<Usage.str()<<endl;
    return false;
  }
  if (m_GeometryFileName == "") {
    cout<<"Error: No geometry file name given!"<<endl;
    cout<<Usage.str()<<endl;
    return false;
  }
  if (m_FileName == "") {
    cout<<"Error: No file name given!"<<endl;
    cout<<Usage.str()<<endl;
    return false;  
  }
  if (m_ResponseName == "") {
    cout<<"Error: No response name given!"<<endl;
    cout<<Usage.str()<<endl;
    return false;
  }
  
  return true;
}


/******************************************************************************
 * 
 */
MGeometryRevan* ResponseGenerator::LoadGeometry(bool ActivateNoise, double GlobalFailureRate)
{
  MGeometryRevan* ReGeometry = new MGeometryRevan();
  if (ReGeometry->ScanSetupFile(m_GeometryFileName.c_str()) == true) {
    cout<<"ReSi: Geometry "<<ReGeometry->GetName()<<" loaded!"<<endl;
    ReGeometry->ActivateNoising(ActivateNoise);
    ReGeometry->SetGlobalFailureRate(GlobalFailureRate);
  } else {
    cout<<"ReSi: Loading of geometry "<<ReGeometry->GetName()<<" failed!!"<<endl;
    delete ReGeometry;
    ReGeometry = 0;
  } 

  return ReGeometry;
}


/******************************************************************************
 *  Load the sim file:
 */
bool ResponseGenerator::OpenSimulationFile()
{
  m_ReReader = new MRawEventAnalyzer(m_FileName.c_str(), "", m_ReGeometry);

  if (m_RevanCfgFileName == g_StringNotDefined) {
    m_ReReader->SetClusteringAlgorithm(MRawEventAnalyzer::c_ClusteringAlgoAdjacent);
    m_ReReader->SetTrackingAlgorithm(MRawEventAnalyzer::c_TrackingAlgoPearson);
    //m_ReReader->SetTrackingAlgorithm(MRawEventAnalyzer::c_TrackingAlgoBayesian);
    m_ReReader->SetCSRAlgorithm(MRawEventAnalyzer::c_CSRAlgoBayesian);
    m_ReReader->SetDecayAlgorithm(MRawEventAnalyzer::c_DecayAlgoNone);

    //m_ReReader->SetBETFileName("$(MEGALIB)/resource/response/BCT.MEGA.v1.t.goodbad.rsp");

    m_ReReader->SetStandardClusterizerMinDistanceD1(0.071);
    m_ReReader->SetStandardClusterizerMinDistanceD2(1.1);
    m_ReReader->SetStandardClusterizerMinDistanceD3(0.199);
    m_ReReader->SetLeverArmMin(0);
    m_ReReader->SetLeverArmMax(10000);
    m_ReReader->SetTotalEnergyMin(0);
    m_ReReader->SetTotalEnergyMax(1000000);
    m_ReReader->SetCSRThresholdMin(m_CSRThresholdMin);
    m_ReReader->SetCSRThresholdMax(m_CSRThresholdMax);
    //m_ReReader->SetEventIdMin(1);
    //m_ReReader->SetEventIdMax(100);
    
    if (m_Mode == c_ModeTracks) {
      m_ReReader->SetCSRAlgorithm(MRawEventAnalyzer::c_CSRAlgoNone);
      m_ReReader->SetTrackingAlgorithm(MRawEventAnalyzer::c_TrackingAlgoPearson);
      m_ReReader->SetGuaranteeStartD1(false);
      m_ReReader->SetSearchPairTracks(false);
      m_ReReader->SetSearchComptonTracks(true);
      m_ReReader->SetNTrackSequencesToKeep(numeric_limits<int>::max());
      m_ReReader->SetSearchMIPTracks(false);
      if (m_Verify == true) {
        m_ReReader->SetTrackingAlgorithm(MRawEventAnalyzer::c_TrackingAlgoBayesian);
        m_ReReader->AddDecayEnergy(511.004, 2.0);
        m_ReReader->SetBCTFileName(m_ResponseName.c_str());
        m_ReReader->SetKeepAllComptonTracks(false);
        m_ReReader->SetMaxComptonJump(2);
      }
    } else if (m_Mode == c_ModeComptons) {
      m_ReReader->SetGuaranteeStartD1(false);
      m_ReReader->SetSearchPairTracks(false);
      m_ReReader->SetSearchComptonTracks(false);
      m_ReReader->SetSearchMIPTracks(false);
      m_ReReader->SetCSRMaxNHits(m_CSRMaxLength);
      m_ReReader->SetCSROnlyCreateSequences(true);
      if (m_Verify == true) {
        //m_ReReader->SetTrackingAlgorithm(MRawEventAnalyzer::c_TrackingAlgoNone);
        //m_ReReader->SetCSRAlgorithm(MRawEventAnalyzer::c_CSRAlgoBayesian);
        m_ReReader->SetCSRAlgorithm(MRawEventAnalyzer::c_CSRAlgoFoM);
        m_ReReader->AddDecayEnergy(511.004, 2.0);
        m_ReReader->SetBCTFileName(m_ResponseName.c_str());
        m_ReReader->SetGuaranteeStartD1(false);
        //m_ReReader->SetDoTracking(true);
        m_ReReader->SetSearchPairTracks(true);
        m_ReReader->SetSearchComptonTracks(true);
        m_ReReader->SetSearchMIPTracks(true);
        m_ReReader->SetKeepAllComptonTracks(false);
        m_ReReader->SetCSRMaxNHits(m_CSRMaxLength);
        m_ReReader->SetCSROnlyCreateSequences(false);
        m_ReReader->SetMaxComptonJump(1);
      }
    } else if (m_Mode == c_ModeImaging || m_Mode == c_ModeEarthHorizon) {
      m_ReReader->SetCSRAlgorithm(MRawEventAnalyzer::c_CSRAlgoFoM);
      m_ReReader->AddDecayEnergy(511.004, 2.0);
      m_ReReader->SetBCTFileName(m_ResponseName.c_str());
      m_ReReader->SetGuaranteeStartD1(false);
      //m_ReReader->SetDoTracking(true);
      m_ReReader->SetSearchPairTracks(true);
      m_ReReader->SetSearchComptonTracks(true);
      m_ReReader->SetSearchMIPTracks(true);
      m_ReReader->SetCSRMaxNHits(m_CSRMaxLength);
      m_ReReader->SetCSROnlyCreateSequences(false);
      m_ReReader->SetMaxComptonJump(1);   
      m_ReReader->SetRejectOneDetectorTypeOnlyEvents(false); 
    }
  } else {
    MSettingsRevan Cfg(false);
    Cfg.ReadData(m_RevanCfgFileName);
    Cfg.UpdateRawEventAnalyzer(*m_ReReader);

    // Defaults:
    if (m_Verify == false && m_Mode == c_ModeComptons) {
      m_ReReader->SetCSROnlyCreateSequences(true);
    }

    if (m_Verify == false && m_Mode == c_ModeTracks) {
      m_ReReader->SetNTrackSequencesToKeep(numeric_limits<int>::max());
    }
  }

  if (m_MimrecCfgFileName != g_StringNotDefined) {
    MSettingsMimrec Cfg(false);
    Cfg.ReadData(m_MimrecCfgFileName);
    Cfg.UpdateEventSelector(m_MimrecEventSelector);
  }

  //m_ReReader->SetCSRMaxNHits(4);
  if (m_ReReader->PreAnalysis() == false) return false;

  m_SiReader = new MFileEventsSim(m_SiGeometry);
  if (m_SiReader->Open(m_FileName.c_str()) == false) return false;

  return true;
}


/******************************************************************************
 * Load the next set of events
 * Returns false in case of interrupt or if no more events are left
 */
bool ResponseGenerator::LoadEvents()
{
  int IdRe = 0;
  int IdSi = 0;
  bool MoreEvents = true;
  bool Increase = true;

  while (IdRe != IdSi || Increase == true) {
    if (IdSi > m_MaxNEvents) return false;

    if (IdRe < IdSi) {
      // Read revan
      mout<<"Searching revan event..."<<endl;

      // Clean:
      // delete m_ReEvent; // automatically deleted!
      m_ReEvent = 0;

      // Load/Analyze
      if (m_ReReader->AnalyzeOneEvent() == false) {
        cout<<"ReSi: Did not find any raw event!"<<endl;
        MoreEvents = false;
        break;
      }

      if (m_ReReader->GetRawEventList() != 0) {
        if (m_ReReader->GetRawEventList()->GetNRawEvents() > 0) {
          m_ReEvent = m_ReReader->GetRawEventList()->GetRawEventAt(0);
        }
      }

      // Decide future:
      if (m_ReEvent != 0 && m_ReEvent->GetEventType() != MRERawEvent::c_PairEvent) {
        IdRe = m_ReEvent->GetEventId();
        Increase = false;
        mout<<"ReSi: ER found good solution (Id="<<m_ReEvent->GetEventId()<<")!"<<endl;
      } else {
        Increase = true;
        mout<<"ReSi: Did not find good raw event!"<<endl;
      }
      
    } else {
      mout<<"Sivan..."<<endl;

      // Clean:
      delete m_SiEvent;
      m_SiEvent = 0;
      
      // Read:
      if (m_Verify == true) {
        m_SiEvent = m_SiReader->GetNextEvent();
      } else {
        m_SiEvent = m_SiReader->GetNextEvent(false);
      }

      // Decide:
      if (m_SiEvent != 0) {
        // Test if it is not truncated:
        if (m_SiEvent->GetNIAs() > 1 && m_SiEvent->GetIAAt(m_SiEvent->GetNIAs()-1)->GetType() != "TRNC") {
          
          IdSi = m_SiEvent->GetEventNumber();
          Increase = false;	  
          mout<<"ReSi: SI found good solution (Id="<<m_SiEvent->GetEventNumber()<<")!"<<endl;
        } else {
          // Ignore this event...
          mout<<"ReSi: SI found NO good solution (Id="<<m_SiEvent->GetEventNumber()<<") TRNC or not enough IAs!"<<endl;
          Increase = true;
        }
      } else {
        Increase = true;
        mout<<"ReSi: No more events!"<<endl;
        MoreEvents = false;
        break;
      }
    }

    // Before we jump out here, we have to do some sanity checks:
    if (!(IdRe != IdSi || Increase == true)) {
      m_Ids.clear();
      m_OriginIds.clear();
      
      if (DoSimulationSanityCheck() == false) {
        Increase = true;
        mout<<"ReSi: Something is wrong with your simulation! Posibilities are"<<endl;
        mout<<"      * You do not have interaction information (IA)"<<endl;
        mout<<"      * The step length is too long (e.g. longer than your pitch)"<<endl;
        mout<<"      * You have too high production thresholds"<<endl;
        mout<<"      * Something else..."<<endl;
      }
    }
  }
  
  return MoreEvents;  
}

/******************************************************************************
 * Do a sanity check if the simulation is ok
 * E.g. all remaining clusters need to have on sim origin != 1 !!
 */
bool ResponseGenerator::DoSimulationSanityCheck()
{
  //cout<<m_SiEvent->GetEventNumber()<<" vs. "<<m_ReEvent->GetEventId()<<endl;
  //cout<<m_SiEvent->GetSimulationData()<<endl;
  //cout<<m_ReEvent->ToString()<<endl;
  
  if (m_SiEvent->GetNIgnoredHTs() > 0) {
    mout<<"We have ignored HTs in the sim file -> check your geometry, because noising was deactivated!"<<endl;
    return false;
  }

  MRawEventList* REList = m_ReReader->GetRawEventList();

  int r_max = REList->GetNRawEvents();
  for (int r = 0; r < r_max; ++r) {
    MRERawEvent* RE = REList->GetRawEventAt(r);
    if (RE->GetVertex() != 0) continue;

    if (int(m_SiEvent->GetNHTs()) < RE->GetNRESEs()) {
      mout<<"The simulation has less hits than the raw event!!!"<<endl;
      return false;
    }

    for (int i = 0; i < RE->GetNRESEs(); ++i) {
      MRESE* RESE = RE->GetRESEAt(i);

      vector<int> EndOriginIds = GetOriginIds(RESE);

      bool More = false;
      for (unsigned int i = 0; i < EndOriginIds.size(); ++i) {
        if (EndOriginIds[i] > 1) {
          More = true;
          break;
        }
      }

      if (More == false) {
        return false;
      }
    }
  }

  return true;
}


/******************************************************************************
 * Do whatever analysis is necessary
 */
bool ResponseGenerator::Analyze()
{
  if ((m_SiGeometry = LoadGeometry(false, 0.0)) == 0) return false;
  if ((m_ReGeometry = LoadGeometry(true, 0.0)) == 0) return false;

  if (OpenSimulationFile() == false) return false;

  if (m_Mode == c_ModeTracks) {
    if (m_Verify == true) {
      return VerifyTrackReconstruction();
    } else {
      return GenerateTrackPdf();
    }
  } else if (m_Mode == c_ModeComptons) {
    if (m_Verify == true) {
      return VerifyComptonReconstruction();
    } else {
      return GenerateComptonPdf();
    } 
  } else if (m_Mode == c_ModeImaging) {
    return GenerateImagingPdf();
  } else if (m_Mode == c_ModeEarthHorizon) {
    return GenerateEarthHorizonPdf();
  }

  return false;
}


/******************************************************************************
 * Do whatever analysis is necessary
 */
bool ResponseGenerator::VerifyTrackReconstruction()
{
  cout<<"Verifying track reconstruction..."<<endl;

  bool NoEscape = false;

  int NEvents = 0;
  int NLastLayerHit = 0;
  int NCurls = 0;
  vector<int> NGoodTracks(100, 0);
  vector<int> NBadTracks(100, 0);

  int WNEscape = 0;
  int WNStartMissing = 0;
  int WNCurling = 0;
  int WNAngle = 0;
  int WNMix = 0;

  unsigned int LBins = 30;
  vector<float> l = CreateLogDist(1E-4, 2, LBins+1);



  float* LogAxis = new float[LBins+1];
  LogAxis[0] = 1E-30;
  for (unsigned int i = 1; i <LBins+1; ++i) {
    LogAxis[i] = l[i];
  }
  
  TH1D* BPGood = new TH1D("BPGood", "BPGood", LBins, LogAxis);
  BPGood->GetXaxis()->SetTitle("1 - filter result");
  BPGood->GetYaxis()->SetTitle("counts");
  TH1D* BPBad = new TH1D("BPBad", "BPBad", LBins, LogAxis);
  BPBad->GetXaxis()->SetTitle("1 - filter result");
  BPBad->GetYaxis()->SetTitle("counts");


  while (LoadEvents() == true) {

    MRERawEvent* RE = 0;
    MRawEventList* REList = m_ReReader->GetRawEventList();

    NEvents++;

    mout<<endl<<endl<<endl;
    mout<<"Verifying event ("<<m_SiEvent->GetEventNumber()<<" vs. "<<m_ReEvent->GetEventId()<<") :"<<endl;
    mout<<endl;

    if (m_SiEvent->GetEventType() != 1) { // Compton
      mout<<"No Compton event!"<<endl;
      continue;
    }

    if (REList->GetNRawEvents() == 0) {
      //RNoRawEvent++;
      mout<<"  --> No raw event!"<<endl;
    } else if (REList->GetNRawEvents() == 0) {
      //RMoreThanOneRawEvent++;
      mout<<"  --> More than one raw event!"<<endl;
    }

    RE = REList->GetRawEventAt(0);
    massert(RE != 0);

    //g_DebugLevel = 1;

    int NTracks = 0;
    for (int i = 0; i < RE->GetNRESEs(); ++i) {
      if (RE->GetRESEAt(i)->GetType() == MRESE::c_Track) {
        NTracks++;
      }
    }
    if (NTracks > 1) continue;

    for (int i = 0; i < RE->GetNRESEs(); ++i) {
      if (RE->GetRESEAt(i)->GetType() == MRESE::c_Track) {

        // Check N of Hit layers:
        double NHitLayers = 0;
        
        static long NWarnings = 0;
        if (NWarnings == 0) {
          merr<<"Does not work for all geometries --> Only Mega-type!"<<endl;
          // --> RevanGeometry weiﬂ wie...
        }
        NWarnings = 1;

        double MaxLayer = -numeric_limits<double>::max();
        double MinLayer = numeric_limits<double>::max();
        for (int h = 0; h < RE->GetRESEAt(i)->GetNRESEs(); h++) {
          if (RE->GetRESEAt(i)->GetRESEAt(h)->GetPosition()[2] > MaxLayer) {
            MaxLayer = RE->GetRESEAt(i)->GetRESEAt(h)->GetPosition()[2];
          }
          if (RE->GetRESEAt(i)->GetRESEAt(h)->GetPosition()[2] < MinLayer) {
            MinLayer = RE->GetRESEAt(i)->GetRESEAt(h)->GetPosition()[2];
          }
        }
        NHitLayers = (MaxLayer-MinLayer)+1;

//         cout<<"Hit: "<<NHitLayers<<"!"<<0.5*RE->GetRESEAt(i)->GetNRESEs()<<endl;
//         cout<<MaxLayer<<"!"<<MinLayer<<endl;
//         if (RE->GetRESEAt(i)->GetNRESEs()/NHitLayers > 1.2) {
//           NCurls++;
//           continue;
//         }

//         if (MinLayer < 16) {
//           NLastLayerHit++;
//           continue;
//         }


        cout<<"Passed layer selection"<<endl;

        //        if (((MRETrack*) RE->GetRESEAt(i))->GetScore() < 0.01) continue; 


        cout<<((MRETrack*) RE->GetRESEAt(i))->ToString()<<endl;

        bool Escape = false;
        bool StartMissing = false;
        bool Curling = false;
        bool Angle = false;
        bool Mix = false;
        int Ret = IsTrackOk(RE, (MRETrack*) RE->GetRESEAt(i), Escape, StartMissing, Curling, Angle, Mix);
        if (Ret == 0) {
          mout<<" --> GOOD track  "<<RE->GetRESEAt(i)->GetNRESEs()
              <<"  "<<((MRETrack*) RE->GetRESEAt(i))->GetQualityFactor()<<endl;
          NGoodTracks[RE->GetRESEAt(i)->GetNRESEs()]++;
          BPGood->Fill(((MRETrack*) RE->GetRESEAt(i))->GetQualityFactor());
        } else {
          if (NoEscape == false || (NoEscape == true && Escape == false)) {
            mout<<" --> BAD track  "<<RE->GetRESEAt(i)->GetNRESEs()
                <<"  "<<((MRETrack*) RE->GetRESEAt(i))->GetQualityFactor()<<endl;
            NBadTracks[RE->GetRESEAt(i)->GetNRESEs()]++;
            BPBad->Fill(((MRETrack*) RE->GetRESEAt(i))->GetQualityFactor());
            
            if (Mix == true) {
              WNMix++;
            }
            if (Escape == true) {
              WNEscape++;
            }
            if (StartMissing == true) {
              WNStartMissing++;
            }
            if (Curling == true) {
              WNCurling++;
            }
            if (Angle == true) {
              WNAngle++;
            }
          }
        }
        mout<<endl<<endl<<endl;
      }
    }

    //g_DebugLevel = 0;
  }

  cout<<"Failure reasons:"<<endl;
  cout<<"==============="<<endl;
  cout<<endl;
  cout<<"Reconstructed events:               "<<NEvents<<endl; 
  cout<<endl;
  int SumGood = 0;
  int SumBad = 0;
  for (unsigned int i = 0; i < NGoodTracks.size(); ++i) {
    SumGood += NGoodTracks[i];
    SumBad += NBadTracks[i];
  }  
  cout<<"Number of curls:                   "<<NCurls<<endl; 
  cout<<"Number of last layers:             "<<NLastLayerHit<<endl; 
  cout<<"Number of tracks:                  "<<SumGood+SumBad<<endl; 
  cout<<endl;

  cout<<"From all found tracks: "<<(100.0*SumGood)/(SumGood+SumBad)<<"% good!"<<endl;
  for (unsigned int i = 0; i < NGoodTracks.size(); ++i) {
    if (NGoodTracks[i]+NBadTracks[i] > 0) {
      cout<<"Length: "<<setw(3)<<i
          <<"  G: "<<setw(5)<<NGoodTracks[i]
          <<"  B: "<<setw(5)<<NBadTracks[i]
          <<"  ("<<100.0*NGoodTracks[i]/(NGoodTracks[i]+NBadTracks[i])<<" %)"<<endl;
    }
  }

  cout<<"Reason for wrong:"<<endl;
  cout<<endl;
  cout<<"Start is missing: "<<100.0*WNStartMissing/SumBad<<endl;
  cout<<"Leakage:          "<<100.0*WNEscape/SumBad<<endl;
  cout<<"Curling:          "<<100.0*WNCurling/SumBad<<endl;
  cout<<"Angle:            "<<100.0*WNAngle/SumBad<<endl;
  cout<<"Mix:              "<<100.0*WNMix/SumBad<<endl;


  TCanvas* CBPGood = new TCanvas();
  CBPGood->cd();
  CBPGood->SetLogx();
  CBPGood->SetLogy();
  BPGood->Draw();
  BPGood->GetXaxis()->SetRange(2, LBins);
  CBPGood->Update();
  CBPGood->SaveAs(MString(m_ResponseName) + ".BPGood.root");

  TCanvas* CBPBad = new TCanvas();
  CBPBad->cd();
  CBPBad->SetLogx();
  CBPBad->SetLogy();
  BPBad->Draw();
  BPBad->SetMaximum(BPGood->GetMaximum());
  BPBad->GetXaxis()->SetRange(2, LBins);
  CBPBad->Update();
  CBPBad->SaveAs(MString(m_ResponseName) + ".BPBad.root");

  TH1D* RunningSN = new TH1D("RunningS", "RunningSN", LBins, LogAxis);
  RunningSN->GetXaxis()->SetTitle("Q_{e}");
  RunningSN->GetYaxis()->SetTitle("Average reconstruction efficiency [%]");

  for (int b = 2; b <= RunningSN->GetNbinsX(); ++b) {
    if (BPGood->Integral(2, b) > 0) {
      RunningSN->SetBinContent(b, BPGood->Integral(2, b)/(BPBad->Integral(2, b)+BPGood->Integral(2, b)));
    } else {
      RunningSN->SetBinContent(b, 0);
    }
  }

  TCanvas* CRunningSN = new TCanvas();
  CRunningSN->cd();
  CRunningSN->SetLogx();
  RunningSN->Draw();
  RunningSN->GetXaxis()->SetRange(2, LBins);
  CRunningSN->Update();
  CRunningSN->SaveAs(MString(m_ResponseName) + ".RunningSN.root");


  TH1D* GoodBadratio = new TH1D("GoodBadratio", "GoodBadratio", LBins, LogAxis);
  GoodBadratio->GetXaxis()->SetTitle("Q_{e}");
  GoodBadratio->GetYaxis()->SetTitle("Percentage of correct reconstrcutions [%]");

  for (int b = 1; b <= GoodBadratio->GetNbinsX(); ++b) {
    if (BPBad->GetBinContent(b) > 0 || BPGood->GetBinContent(b) > 0) {
      GoodBadratio->SetBinContent(b, BPGood->GetBinContent(b) / 
                                  (BPBad->GetBinContent(b) + BPGood->GetBinContent(b)));
    }
  }

  TCanvas* CGoodBadratio = new TCanvas();
  CGoodBadratio->cd();
  CGoodBadratio->SetLogx();
  GoodBadratio->Draw();
  GoodBadratio->GetXaxis()->SetRange(2, LBins);
  CGoodBadratio->Update();
  CGoodBadratio->SaveAs(MString(m_ResponseName) + ".GoodBadratio.root");
                     
  return true;
}

/******************************************************************************
 * Do whatever analysis is necessary
 */
bool ResponseGenerator::VerifyComptonReconstruction()
{
  bool UseFoM = true;

  //m_ReReader->SetLeverArmMin(2);
  if (UseFoM == true) {
    m_ReReader->SetTrackingAlgorithm(MRawEventAnalyzer::c_CSRAlgoFoM);
  }

  double Threshold = 0;

  int NEvents = 0;

  int NComptonEvents = 0;
  int NPairEvents = 0;
  int NUnknownEvents = 0;

  int RNoRawEvent = 0;
  int RMoreThanOneRawEvent = 0;
  
  int CorrectComptonSequenceAccepted = 0;
  int CorrectComptonSequenceRejected = 0;
  int CorrectComptonAsPairRejected = 0;
  int CorrectComptonAsPairDecayRejected = 0;
  int IncorrectComptonSequenceAccepted = 0;
  int IncorrectComptonSequenceRejected = 0;
  int IncorrectComptonAsPairAccepted = 0;

  int CorrectComptonIncompleteAbsorbed = 0;
  int IncorrectComptonIncompleteAbsorbed = 0;

  int WrongSequenceMissingHit = 0;
  int WrongSequenceDualHit = 0;
  int WrongSequenceSeveralComptonsInCenter = 0;
  int WrongSequenceManyPossibilities = 0;
  int WrongSequenceHighDoppler = 0;
  int WrongSequenceTrack = 0;
  int WrongSequenceLongTrack = 0;
  int WrongSequenceAdditionalHit = 0;
  int WrongSequencePair = 0;
  int WrongSequenceUnknown = 0;

  int MissingEnergyStart = 0;
  int MissingEnergyEscape = 0;
  int MissingEnergyRest = 0;

  int MissclassifiedPair = 0;
  int MissclassifiedComptonPhoto = 0;
  int MissclassifiedComptonPair = 0;
  int MissclassifiedComptonRest = 0;
  int MissclassifiedUnknown = 0;

  int CorrectPair = 0;
  //int CorrectCompton = 0;

  double Min;
  if (UseFoM == true) {
    Min = 1E-12;
  } else {
    Min = 1E-10;
  }

  unsigned int LBins = 100;
  vector<float> l = CreateLogDist(Min, 2, LBins+1);
  float* LogAxis = new float[LBins+1];
  LogAxis[0] = 1E-30;
  for (unsigned int i = 1; i <LBins+1; ++i) {
    LogAxis[i] = l[i];
  }
  
  TH1D* BPGood = new TH1D("BPGood", "BPGood", LBins, LogAxis);
  TH1D* BPBad = new TH1D("BPBad", "BPBad", LBins, LogAxis);
  TH1D* BPBadWSMissingStart = new TH1D("BPBadMissingStart", "BPBadMissingStart", LBins, LogAxis);
  TH1D* BPBadWSDualStart = new TH1D("BPBadDualStart", "BPBadDualStart", LBins, LogAxis);
  TH1D* BPBadWSDual = new TH1D("BPBadDual", "BPBadDual", LBins, LogAxis);
  TH1D* BPBadWSMany = new TH1D("BPBadMany", "BPBadMany", LBins, LogAxis);
  TH1D* BPBadWSTrack = new TH1D("BPBadTrack", "BPBadTrack", LBins, LogAxis);
  TH1D* BPBadWSPair = new TH1D("BPBadPair", "BPBadPair", LBins, LogAxis);
  TH1D* BPBadWSRest = new TH1D("BPBadRest", "BPBadRest", LBins, LogAxis);

  while (LoadEvents() == true) {

    MRERawEvent* RE = 0;
    MRawEventList* REList = m_ReReader->GetRawEventList();

    NEvents++;
    mout<<"Verifying event ("<<NEvents<<") :";

    if (REList->GetNRawEvents() == 0) {
      RNoRawEvent++;
      mout<<"  --> No raw event!"<<endl;
    } else if (REList->GetNRawEvents() == 0) {
      RMoreThanOneRawEvent++;
      mout<<"  --> More than one raw event!"<<endl;
    }

    RE = REList->GetRawEventAt(0);
    massert(RE != 0);

    cout<<RE->ToString()<<endl;

    if (RE->GetNRESEs() == 2) {
      mout<<"  --> dual site"<<endl;
      continue;
    }

    // Test the quality of this event
    if (m_SiEvent->GetEventType() == 1) { // Compton
      NComptonEvents++;
      cout<<"Type: "<<RE->GetEventType()<<endl;
      if (RE->GetEventType() == MRERawEvent::c_ComptonEvent) {

        // Check Compton part:
        if (IsAbsorbed(RE) == false) {
          if (RE->GetComptonQualityFactor1() > Threshold) {
            mout<<"  --> Incomplete absorbed Compton ("<<RE->GetNRESEs()<<") NOT rejected (TS:"
                <<RE->GetComptonQualityFactor1()<<", "
                <<RE->GetComptonQualityFactor2()<<")!  ";
            IncorrectComptonIncompleteAbsorbed++;

            // Do some further investigations:
            if (StartHitMissingEnergy() == true) {
              mout<<"Start hit is missing energy!"<<endl;
              MissingEnergyStart++;
            } else if (LastEscape() == true) {
              mout<<"Photon escapes!"<<endl;
              MissingEnergyEscape++;
            } else {
              mout<<"Unknown!"<<endl;
              MissingEnergyRest++;
            }

          } else {
            mout<<"  --> Incomplete absorbed Compton ("<<RE->GetNRESEs()<<") rejected (TS:"<<RE->GetComptonQualityFactor1()<<")!"<<endl;
            CorrectComptonIncompleteAbsorbed++;
          }
          if (UseFoM == true) {
            BPBad->Fill(RE->GetComptonQualityFactor1());
          } else {
            BPBad->Fill(1-RE->GetComptonQualityFactor1());
          }
          if (RE->GetComptonQualityFactor1() > 0.99) cout<<m_SiEvent->GetEventNumber()<<endl;
        }
        // Completely absorbed:
        else {
          if (IsSequenceOk(RE) == true) {
            if (UseFoM == true) {
              BPGood->Fill(RE->GetComptonQualityFactor1());
            } else {
              BPGood->Fill(1-RE->GetComptonQualityFactor1());
            }
            if (RE->GetComptonQualityFactor1() > Threshold) {
              mout<<"  --> Good sequence ("<<RE->GetNRESEs()<<") accepted (TS:"<<RE->GetComptonQualityFactor1()<<")!"<<endl;
              CorrectComptonSequenceAccepted++;
            } else {
              mout<<"  --> Good sequence ("<<RE->GetNRESEs()<<") rejected (TS:"<<RE->GetComptonQualityFactor1()<<")!"<<endl;
              CorrectComptonSequenceRejected++;
            }
          } else {
            cout<<"Filling BAD Histo!"<<endl;
            if (UseFoM == true) {
              BPBad->Fill(RE->GetComptonQualityFactor1());
            } else {
              BPBad->Fill(1-RE->GetComptonQualityFactor1());
            }
            if (RE->GetComptonQualityFactor1() > 0.99) cout<<m_SiEvent->GetEventNumber()<<endl;
            if (RE->GetComptonQualityFactor1() > Threshold) {
              mout<<"  --> Wrong sequence ("<<RE->GetNRESEs()<<") (TS:"<<RE->GetComptonQualityFactor1()<<")! ";
              IncorrectComptonSequenceAccepted++;

              // Do some further investigations:
              if (SeveralComptonsInStart() == true) {
                mout<<"Several C. in start!"<<endl;
                WrongSequenceDualHit++;
                if (UseFoM == true) {
                  BPBadWSDualStart->Fill(RE->GetComptonQualityFactor1());
                } else {
                  BPBadWSDualStart->Fill(1-RE->GetComptonQualityFactor1());
                }
              } else if (MissingStartHit() == true) {
                mout<<"Missing first C.!"<<endl;
                WrongSequenceMissingHit++;
                if (UseFoM == true) {
                  BPBadWSMissingStart->Fill(RE->GetComptonQualityFactor1());
                } else {
                  BPBadWSMissingStart->Fill(1-RE->GetComptonQualityFactor1());
                }
              } else if (SeveralComptonsInCenter() == true) {
                mout<<"Several Compton Center!"<<endl;
                WrongSequenceSeveralComptonsInCenter++;
                if (UseFoM == true) {
                  BPBadWSDual->Fill(RE->GetComptonQualityFactor1());
                } else {
                  BPBadWSDual->Fill(1-RE->GetComptonQualityFactor1());
                }
              } else if (HighDoppler() == true) {
                mout<<"High Doppler!"<<endl;
                WrongSequenceHighDoppler++;
                if (UseFoM == true) {
                  BPBadWSRest->Fill(RE->GetComptonQualityFactor1());
                } else {
                  BPBadWSRest->Fill(1-RE->GetComptonQualityFactor1());
                }
              } else if (RE->GetComptonQualityFactor1() > 0 && 
                         RE->GetComptonQualityFactor2()/RE->GetComptonQualityFactor1() < 2) {
                mout<<"Many possibilites!"<<RE->GetComptonQualityFactor2()/RE->GetComptonQualityFactor1()<<endl;
                WrongSequenceManyPossibilities++;
                if (UseFoM == true) {
                  BPBadWSMany->Fill(RE->GetComptonQualityFactor1());
                } else {
                  BPBadWSMany->Fill(1-RE->GetComptonQualityFactor1());
                }
              } else if (Track() == true) {
                mout<<"Electron Track!"<<endl;
                WrongSequenceTrack++;
                if (UseFoM == true) {
                  BPBadWSTrack->Fill(RE->GetComptonQualityFactor1());
                } else {
                  BPBadWSTrack->Fill(1-RE->GetComptonQualityFactor1());
                }
              } else if (LongTrack() == true) {
                mout<<"Long track!"<<endl;
                WrongSequenceLongTrack++;
                if (UseFoM == true) {
                  BPBadWSRest->Fill(RE->GetComptonQualityFactor1());
                } else {
                  BPBadWSRest->Fill(1-RE->GetComptonQualityFactor1());
                }
              } else if (AddOnHit() == true) {
                mout<<"AdditionalHit"<<endl;
                WrongSequenceAdditionalHit++;
                if (UseFoM == true) {
                  BPBadWSRest->Fill(RE->GetComptonQualityFactor1());
                } else {
                  BPBadWSRest->Fill(1-RE->GetComptonQualityFactor1());
                }
              } else {
                mout<<"Unknown sequence problem!"<<endl;
                WrongSequenceUnknown++;
                if (UseFoM == true) {
                  BPBadWSRest->Fill(RE->GetComptonQualityFactor1());
                } else {
                  BPBadWSRest->Fill(1-RE->GetComptonQualityFactor1());
                }
              }
            } else {
              mout<<"  --> Wrong sequence rejected (TS:"<<RE->GetComptonQualityFactor1()<<")!"<<endl;
              IncorrectComptonSequenceRejected++;
            }
          }
        }
      } else if (RE->GetEventType() == MRERawEvent::c_PairEvent) {
        mout<<"  --> Misclassified compton event: pair ("<<RE->GetEventType()<<")"<<endl;
        MissclassifiedComptonPair++;
      } else if (RE->GetEventType() == MRERawEvent::c_PhotoEvent) {
        mout<<"  --> Misclassified compton event: photo ("<<RE->GetEventType()<<")"<<endl;
        MissclassifiedComptonPhoto++;
      } else {
        mout<<"  --> Misclassified compton event: unknown ("<<RE->GetEventType()<<")"<<endl;
        MissclassifiedComptonRest++;
      }
    } else if (m_SiEvent->GetEventType() == 2) { // Pair
      NPairEvents++;
      if (RE->GetEventType() == MRERawEvent::c_PairEvent) {
        mout<<"Pair-check not implemented!"<<endl;
        CorrectPair++;
      } else if (RE->GetEventType() == MRERawEvent::c_ComptonEvent) {
        if (RE->IsDecay() == false) {
          cout<<"Filling BAD No Decay!"<<endl;
          WrongSequencePair++;
          if (UseFoM == true) {
            BPBad->Fill(RE->GetComptonQualityFactor1());
            BPBadWSPair->Fill(RE->GetComptonQualityFactor1());
          } else {
            BPBad->Fill(1-RE->GetComptonQualityFactor1());
            BPBadWSPair->Fill(1-RE->GetComptonQualityFactor1());
          }
          if (RE->GetComptonQualityFactor1() > Threshold) {
            mout<<"  --> Pair as Compton accepted! (TS:"<<RE->GetComptonQualityFactor1()<<")! ";
            IncorrectComptonAsPairAccepted++;
          } else {
            mout<<"  --> Pair as Compton rejected! (TS:"<<RE->GetComptonQualityFactor1()<<")! "<<endl;
            CorrectComptonAsPairRejected++;
          }
        } else {
          mout<<"  --> Pair as Compton rejected (decay)! (TS:"<<RE->GetComptonQualityFactor1()<<")! "<<endl;
          CorrectComptonAsPairDecayRejected++;
        }
      } else {
        mout<<"  --> Misclassified pair event!"<<endl;
        MissclassifiedPair++;
      }
    } else {
      NUnknownEvents++;
      if (RE->GetEventType() == MRERawEvent::c_ComptonEvent) {
        mout<<"  --> Unknown event ("
            <<((m_SiEvent->GetNIAs() >= 2) ? m_SiEvent->GetIAAt(1)->GetType() : "?")
            <<") classified as Compton event!"<<endl;
        MissclassifiedComptonRest++;
      } else if (RE->GetEventType() == MRERawEvent::c_PairEvent) {
        mout<<"  --> Unknown event ("
            <<((m_SiEvent->GetNIAs() >= 2) ? m_SiEvent->GetIAAt(1)->GetType() : "?")
            <<") classified as pair event!"<<endl;
        MissclassifiedPair++;
      } else {
        mout<<"  --> Unknown event ("
            <<((m_SiEvent->GetNIAs() >= 2) ? m_SiEvent->GetIAAt(1)->GetType() : "?")
            <<") classified as unknown event!"<<endl;
        MissclassifiedUnknown++;        
      }
    }
  }

  TCanvas* CBPGood = new TCanvas();
  CBPGood->cd();
  CBPGood->SetLogx();
  CBPGood->SetLogy();
  BPGood->Draw();
  BPGood->SetMaximum(BPBad->GetMaximum());
  BPGood->GetXaxis()->SetRange(2, LBins);
  CBPGood->Update();

  TCanvas* CBPBad = new TCanvas();
  CBPBad->cd();
  CBPBad->SetLogx();
  CBPBad->SetLogy();
  BPBad->Draw();
  BPBad->GetXaxis()->SetRange(2, LBins);
  CBPBad->Update();

  TCanvas* CBPBadWSMissingStart = new TCanvas();
  CBPBadWSMissingStart->cd();
  CBPBadWSMissingStart->SetLogx();
  CBPBadWSMissingStart->SetLogy();
  BPBadWSMissingStart->Draw();
  BPBadWSMissingStart->GetXaxis()->SetRange(2, LBins);
  CBPBadWSMissingStart->Update();

  TCanvas* CBPBadWSDualStart = new TCanvas();
  CBPBadWSDualStart->cd();
  CBPBadWSDualStart->SetLogx();
  CBPBadWSDualStart->SetLogy();
  BPBadWSDualStart->Draw();
  BPBadWSDualStart->GetXaxis()->SetRange(2, LBins);
  CBPBadWSDualStart->Update();

  TCanvas* CBPBadWSMany = new TCanvas();
  CBPBadWSMany->cd();
  CBPBadWSMany->SetLogx();
  CBPBadWSMany->SetLogy();
  BPBadWSMany->Draw();
  BPBadWSMany->GetXaxis()->SetRange(2, LBins);
  CBPBadWSMany->Update();

  TCanvas* CBPBadWSTrack = new TCanvas();
  CBPBadWSTrack->cd();
  CBPBadWSTrack->SetLogx();
  CBPBadWSTrack->SetLogy();
  BPBadWSTrack->Draw();
  BPBadWSTrack->GetXaxis()->SetRange(2, LBins);
  CBPBadWSTrack->Update();

  TCanvas* CBPBadWSPair = new TCanvas();
  CBPBadWSPair->cd();
  CBPBadWSPair->SetLogx();
  CBPBadWSPair->SetLogy();
  BPBadWSPair->Draw();
  BPBadWSPair->GetXaxis()->SetRange(2, LBins);
  CBPBadWSPair->Update();

  TCanvas* CBPBadWSDual = new TCanvas();
  CBPBadWSDual->cd();
  CBPBadWSDual->SetLogx();
  CBPBadWSDual->SetLogy();
  BPBadWSDual->Draw();
  BPBadWSDual->GetXaxis()->SetRange(2, LBins);
  CBPBadWSDual->Update();

  TCanvas* CBPBadWSRest = new TCanvas();
  CBPBadWSRest->cd();
  CBPBadWSRest->SetLogx();
  CBPBadWSRest->SetLogy();
  BPBadWSRest->Draw();
  BPBadWSRest->GetXaxis()->SetRange(2, LBins);
  CBPBadWSRest->Update();

  TH1D* RunningSN = new TH1D("RunningS", "RunningSN", LBins, LogAxis);
  int ThresholdBin80 = 0;
  double GoodEvents80 = 0;
  int ThresholdBin50 = 0;
  double GoodEvents50 = 0;
  for (int b = 1; b <= RunningSN->GetNbinsX(); ++b) {
    if (BPGood->Integral(0, b) > 0) {
      RunningSN->SetBinContent(b, BPGood->Integral(0, b)/(BPBad->Integral(0, b)+BPGood->Integral(0, b)));
      if (BPGood->Integral(0, b)/(BPBad->Integral(0, b)+BPGood->Integral(0, b)) > 0.8) {
        ThresholdBin80 = b;
        GoodEvents80 = BPGood->Integral(0, b);
      }
      if (BPGood->Integral(0, b)/(BPBad->Integral(0, b)+BPGood->Integral(0, b)) > 0.5) {
        ThresholdBin50 = b;
        GoodEvents50 = BPGood->Integral(0, b);
      }
    } else {
      RunningSN->SetBinContent(b, 0);
    }
  }
    
  cout<<"Number of good events till p = 80% (TS="
      <<RunningSN->GetBinCenter(ThresholdBin80)<<"):"
      <<GoodEvents80/BPGood->Integral(0, RunningSN->GetNbinsX())<<endl;
  cout<<"Number of good events till p = 50% (TS="
      <<RunningSN->GetBinCenter(ThresholdBin50)<<"):"
      <<GoodEvents50/BPGood->Integral(0, RunningSN->GetNbinsX())<<endl;

  TCanvas* CRunningSN = new TCanvas();
  CRunningSN->cd();
  CRunningSN->SetLogx();
  RunningSN->Draw();
  RunningSN->GetXaxis()->SetRange(2, LBins);
  CRunningSN->Update();

  cout<<"Failure reasons:"<<endl;
  cout<<"==============="<<endl;
  cout<<endl;
  cout<<"Reconstructed events:               "<<NEvents<<endl; 
  cout<<endl;
  cout<<"No raw event:                       "<<RNoRawEvent<<endl;
  cout<<"More than one raw event:            "<<RMoreThanOneRawEvent<<endl;
  cout<<endl;
  cout<<"Number of Comptons:                 "<<NComptonEvents<<endl;
  cout<<"Missclassified photo:               "<<MissclassifiedComptonPhoto<<endl;
  cout<<"Missclassified pair:                "<<MissclassifiedComptonPair<<endl;
  cout<<"Missclassified rest:                "<<MissclassifiedComptonRest<<endl;
  cout<<endl;
  cout<<"Correct identified:   "<<endl;
  cout<<"Good Sequence accepted:             "<<CorrectComptonSequenceAccepted<<endl;
  cout<<"Bad Sequence rejected:              "<<IncorrectComptonSequenceRejected<<endl;
  cout<<"Incomplete Absorption rejected:     "<<CorrectComptonIncompleteAbsorbed<<endl;
  cout<<"Pair rejected:                      "<<CorrectComptonAsPairRejected<<endl;
  cout<<"Pair rejected as decay:             "<<CorrectComptonAsPairDecayRejected<<endl;
  cout<<endl;
  cout<<"Incorrect identified: "<<endl;
  cout<<"Correct sequence rejected:          "<<CorrectComptonSequenceRejected<<endl;
  cout<<"Incorrect sequence accepted:        "<<IncorrectComptonSequenceAccepted<<endl;
  cout<<"    * Dual first hit:               "<<WrongSequenceDualHit<<endl;
  cout<<"    * Missing first hit:            "<<WrongSequenceMissingHit<<endl;
  cout<<"    * Several Compton center:       "<<WrongSequenceSeveralComptonsInCenter<<endl;
  cout<<"    * Many good possibilities:      "<<WrongSequenceManyPossibilities<<endl;
  cout<<"    * High Doppler:                 "<<WrongSequenceHighDoppler<<endl;
  cout<<"    * Track:                        "<<WrongSequenceTrack<<endl;
  cout<<"    * Pair:                         "<<WrongSequencePair<<endl;
  cout<<"    * Long Track:                   "<<WrongSequenceLongTrack<<endl;
  cout<<"    * AdditonalHit:                 "<<WrongSequenceAdditionalHit<<endl;
  cout<<"    * Remaining:                    "<<WrongSequenceUnknown<<endl;
  cout<<"Incomplete Absorption accepted:     "<<IncorrectComptonIncompleteAbsorbed<<endl;
  cout<<"    * Missing start energy:         "<<MissingEnergyStart<<endl;
  cout<<"    * Photon escape:                "<<MissingEnergyEscape<<endl;
  cout<<"    * Remaining:                    "<<MissingEnergyRest<<endl;
  cout<<"Pair accepted:                      "<<IncorrectComptonAsPairAccepted<<endl;
  cout<<endl;
  cout<<"Number of Pairs:                    "<<NPairEvents<<endl;
  cout<<"Misclassified pair:                 "<<MissclassifiedPair<<endl;
  cout<<"CorrectPair:                        "<<CorrectPair<<endl;
  cout<<endl;
  cout<<"Number of Unknowns:                 "<<NUnknownEvents<<endl;
  cout<<"Missclassified Unknown:             "<<MissclassifiedUnknown<<endl;
  cout<<endl;


  return true;
}


/******************************************************************************
 * Return codes:
 * 0: ok
 * 1: Not track at all
 * 2: start not ok
 * 3: missing hits
 * 4: too many hits
 * 5: Hit in cluster not ok
 * 6: Hits not in sequence
 * 7: No common origin
 */
int ResponseGenerator::IsTrackOk(MRERawEvent* RE, MRETrack* Track, bool& Escape, bool& StartMissing, bool& Curling, bool& Angle, bool& Mix)
{
  // A track is ok if 
  // (1) its two start points are in the correct sequence
  // (2) it contains all hits originating from the COMP interaction

  int Return = 0; 

  if (Track->GetNRESEs() <= 1) return 1;

  MRESEIterator Iter;
  Iter.Start(Track->GetStartPoint());
  Iter.GetNextRESE();

  const int IdOffset = 2;

  vector<int> StartIds = GetReseIds(Iter.GetCurrent());
  vector<int> CentralIds = GetReseIds(Iter.GetNext());
  if (StartIds.size() == 0) return false;
  if (CentralIds.size() == 0) return false;

 // Test (2)
  int CommonOrigin = -1;
  for (unsigned int i = 0; i < StartIds.size(); ++i) {
    for (unsigned int j = 0; j < CentralIds.size(); ++j) {
      for (unsigned int oi = 0; oi < m_SiEvent->GetHTAt(StartIds[i]-IdOffset)->GetNOrigins(); ++oi) {
        for (unsigned int oj = 0; oj < m_SiEvent->GetHTAt(CentralIds[j]-IdOffset)->GetNOrigins(); ++oj) {
          if (m_SiEvent->GetHTAt(StartIds[i]-IdOffset)->GetOriginAt(oi) == 
              m_SiEvent->GetHTAt(CentralIds[j]-IdOffset)->GetOriginAt(oj)) {
            CommonOrigin = m_SiEvent->GetHTAt(CentralIds[j]-IdOffset)->GetOriginAt(oj);
            break;
          }
        }
      }
    }
  }
  if (CommonOrigin == 1) CommonOrigin = -1;

  if (CommonOrigin == -1) {
    mdebug<<"   Is track start: No common origin!"<<endl;
    Mix = true;
    return 7;
  }


  // All other hits from the common origin are later than latest of 
  // StartIds:
  double Timing = -numeric_limits<double>::max();
  for (unsigned int i = 0; i < StartIds.size(); ++i) {
    if (m_SiEvent->GetHTAt(StartIds[i]-IdOffset)->GetTime() > Timing) {
      Timing = m_SiEvent->GetHTAt(StartIds[i]-IdOffset)->GetTime();
    }
  }
  unsigned int i_max = m_SiEvent->GetNHTs();
  for (unsigned int i = 0; i < i_max; ++i) {
    if (m_SiEvent->GetHTAt(i)->IsOrigin(CommonOrigin)) {
      if (m_SiEvent->GetHTAt(i)->GetTime() < Timing) {
        bool Found = false;
        for (unsigned int o = 0; o < StartIds.size(); ++o) {
          if (int(i) == StartIds[o]-IdOffset) {
            Found = true;
            break;
          }
        }
        if (Found == false) {
          if (IsRevanHit(RE, m_SiEvent->GetHTAt(i)->GetPosition()) == true) {
            mout<<m_SiEvent->GetHTAt(i)->GetPosition()[0]
                <<"!"<<m_SiEvent->GetHTAt(i)->GetPosition()[1]
                <<"!"<<m_SiEvent->GetHTAt(i)->GetPosition()[2]<<endl;
            mdebug<<"   Is track start: Not first hit (timing)"<<endl;
            Return = 1;    
            break;
          } 
        }
      }
    }
  }

  vector<int> FirstSecondIds = StartIds;
  for (unsigned int i = 0; i < CentralIds.size(); ++i) {
    FirstSecondIds.push_back(CentralIds[i]);
  }

  // All other hits from the common origin are later than latest of 
  // FirstSecondIds:
  Timing = -numeric_limits<double>::max();
  for (unsigned int i = 0; i < FirstSecondIds.size(); ++i) {
    if (m_SiEvent->GetHTAt(FirstSecondIds[i]-IdOffset)->GetTime() > Timing) {
      Timing = m_SiEvent->GetHTAt(FirstSecondIds[i]-IdOffset)->GetTime();
    }
  }
  i_max = m_SiEvent->GetNHTs();
  for (unsigned int i = 0; i < i_max; ++i) {
    if (m_SiEvent->GetHTAt(i)->IsOrigin(CommonOrigin)) {
      if (m_SiEvent->GetHTAt(i)->GetTime() < Timing) {
        bool Found = false;
        for (unsigned int o = 0; o < FirstSecondIds.size(); ++o) {
          if (int(i) == FirstSecondIds[o]-IdOffset) {
            Found = true;
            break;
          }
        }
        if (Found == false) {
          if (IsRevanHit(RE, m_SiEvent->GetHTAt(i)->GetPosition()) == true) {
            mout<<m_SiEvent->GetHTAt(i)->GetPosition()[0]
                <<"!"<<m_SiEvent->GetHTAt(i)->GetPosition()[1]
                <<"!"<<m_SiEvent->GetHTAt(i)->GetPosition()[2]<<endl;
            mdebug<<"   Is track start: Not first hit (timing)"<<endl;
            Return = 1;
            break;
          } 
        }
      }
    }
  }
  

  if (Return == 0) return 0;

  // Check for reason why bad:

  // (a) curling/ping-pong:
  double MaxLayer = -numeric_limits<double>::max();
  double MinLayer = numeric_limits<double>::max();
  for (int h = 0; h < Track->GetNRESEs(); h++) {
    if (Track->GetRESEAt(h)->GetPosition()[2] > MaxLayer) {
      MaxLayer = Track->GetRESEAt(h)->GetPosition()[2];
    }
    if (Track->GetRESEAt(h)->GetPosition()[2] < MinLayer) {
      MinLayer = Track->GetRESEAt(h)->GetPosition()[2];
    }
  }
  double NHitLayers = (MaxLayer-MinLayer)+1;
  
  if (Track->GetNRESEs()/NHitLayers > 1.2) {
    Curling = true;
    cout<<"Curling!"<<endl;
  } else {
    Curling = false;
  }

  // (b) First hit missing:
  StartMissing = false;

  if (CommonOrigin != -1) {
    
    int HTNumber = 0;
    double Timing = numeric_limits<double>::max();
    for (unsigned int i = 0; i < m_SiEvent->GetNHTs(); ++i) {
      if (m_SiEvent->GetHTAt(i)->GetTime() < Timing) {
        Timing = m_SiEvent->GetHTAt(i)->GetTime();
        HTNumber = i;
      }
    }

    if (IsRevanHit(RE, m_SiEvent->GetHTAt(HTNumber)->GetPosition()) == false) {
      StartMissing = true;
      cout<<"Start missing!"<<endl;
    }
  }

  // (c) Escape:
  Escape = false;

  if (IsTrackCompletelyAbsorbed(CommonOrigin) == false) {
    cout<<"Escape!"<<endl;
    Escape = true;
  }

  // (d) Angle
  Angle = false;
  if (CommonOrigin != -1) {
    MVector Dir = m_SiEvent->GetIAAt(CommonOrigin-1)->GetDirection();
    if (Dir.Angle(MVector(0, 0, 1))*c_Deg > 60 && Dir.Angle(MVector(0, 0, 1))*c_Deg < 120) {
      cout<<"Angle!"<<endl;
      Angle = true;
    }
    cout<<"A: "<<Dir.Angle(MVector(0, 0, 1))*c_Deg<<endl;
  }
  return Return;


 //  // Test (1)
//   if (AreIdsInSequence(StartIds) == false) {
//     mdebug<<"   Is track start: Start IDs not in sequence"<<endl;
//     return 5;
//   }
//   if (AreIdsInSequence(CentralIds) == false) {
//     mdebug<<"Is track Start: Central IDs not in sequence"<<endl;
//     return 5;
//   }

//   // Test for holes:
//   // The hit after the last one from start is the first from central:
//   double TimeLastFirst = m_SiEvent->GetHTAt(StartIds.back()-IdOffset)->GetTime();
//   double TimeFirstLast = m_SiEvent->GetHTAt(CentralIds.front()-IdOffset)->GetTime();
//   for (unsigned int h = 0; h < m_SiEvent->GetNHTs(); ++h) {
//     if (m_SiEvent->GetHTAt(h)->GetTime() > TimeLastFirst && 
//         m_SiEvent->GetHTAt(h)->GetTime() < TimeFirstLast) {
//       if (IsRevanHit(RE, m_SiEvent->GetHTAt(h)->GetPosition()) == true) {
//         mout<<m_SiEvent->GetHTAt(h)->GetPosition()[0]
//             <<"!"<<m_SiEvent->GetHTAt(h)->GetPosition()[1]
//             <<"!"<<m_SiEvent->GetHTAt(h)->GetPosition()[2]<<endl;
//         mdebug<<"   Is track start: Hit between first and central according to time!"<<endl;
//         return 6;      
//       }
//     }
//   } 
  
 

//   // Test (3)
//   double Timing = numeric_limits<double>::max();
//   for (unsigned int i = 0; i < StartIds.size(); ++i) {
//     if (m_SiEvent->GetHTAt(StartIds[i]-IdOffset)->GetTime() < Timing) {
//       Timing = m_SiEvent->GetHTAt(StartIds[i]-IdOffset)->GetTime();
//     }
//   }
//   unsigned int i_max = m_SiEvent->GetNHTs();
//   for (unsigned int i = 0; i < i_max; ++i) {
//     if (m_SiEvent->GetHTAt(i)->IsOrigin(CommonOrigin)) {
//       if (m_SiEvent->GetHTAt(i)->GetTime() < Timing) {
//         if (IsRevanHit(RE, m_SiEvent->GetHTAt(i)->GetPosition()) == true) {
//           mout<<m_SiEvent->GetHTAt(i)->GetPosition()[0]
//               <<"!"<<m_SiEvent->GetHTAt(i)->GetPosition()[1]
//               <<"!"<<m_SiEvent->GetHTAt(i)->GetPosition()[2]<<endl;
//           mdebug<<"   Is track start: Not first hit (timing)"<<endl;
//           return 1;    
//         } 
//       }
//     }
//   }

//   if (IsTrackCompletelyAbsorbed(StartIds, Energy) == false) {
//     mdebug<<"   Is track start: Not completely absorbed"<<endl;
//     return false;
//   }


//   if (IsTrackStart(*Iter.GetCurrent(), *Iter.GetNext(), Track->GetEnergy()) == true) {
//     mout<<"  --> GOOD track start!"<<endl;
//     return 0;
//   } else {
//     mout<<"  --> BAD track start!"<<endl;
//     return 2;
//   }

  return 0; // Everything ok
}


/******************************************************************************
 * Only works for 2D Strip!
 */
bool ResponseGenerator::IsTrackCompletelyAbsorbed(int SimId)
{
  vector<int> OriginateFromTwo;
  OriginateFromTwo.push_back(SimId);

  // Check for dependents:
  for (unsigned int i = 0; i < m_SiEvent->GetNIAs(); ++i) {
    for (unsigned int a = 0; a < OriginateFromTwo.size(); ++a) {
      if (m_SiEvent->GetIAAt(i)->GetOrigin() == OriginateFromTwo[a]) {
        OriginateFromTwo.push_back(i+1);
        break;
      }
    }
  }

  double REnergy = 0;
  for (unsigned int i = 0; i < m_SiEvent->GetNHTs(); ++i) {
    for (unsigned int j = 0; j < OriginateFromTwo.size(); ++j) {
      if (m_SiEvent->GetHTAt(i)->IsOrigin(OriginateFromTwo[j]) == true) {
        REnergy += m_SiEvent->GetHTAt(i)->GetEnergy();
      }
    }
  }
  double IEnergy = GetIdealDepositedEnergy(SimId);

  cout<<"E: "<<IEnergy<<"!"<<REnergy<<endl;
  if (fabs((REnergy-IEnergy)/IEnergy) > m_MaxTrackEnergyDifferencePercent &&
      fabs(REnergy-IEnergy) > m_MaxTrackEnergyDifference) {
    cout<<"Escape E: "<<fabs((REnergy-IEnergy)/IEnergy)<<"!"<<m_MaxTrackEnergyDifferencePercent
        <<"!"<<fabs(REnergy-IEnergy)<<"!"<<m_MaxTrackEnergyDifference<<endl;
    return false;
  }

  return true;
}


/******************************************************************************
 * Only works for 2D Strip!
 */
double ResponseGenerator::GetIdealDepositedEnergy(int MinId)
{
  double Ideal = 0;

  MSimIA* Top = 0;
  MSimIA* Bottom = 0;
  if (m_SiEvent->GetIAAt(MinId-2)->GetOrigin() == m_SiEvent->GetIAAt(MinId-1)->GetOrigin()) {
    Top = m_SiEvent->GetIAAt(MinId-2);
    Bottom = m_SiEvent->GetIAAt(MinId-1);
  } else {
    Top = m_SiEvent->GetIAAt(m_SiEvent->GetIAAt(MinId-1)->GetOrigin()-1);
    Bottom = m_SiEvent->GetIAAt(MinId-1);
  }

  if (m_SiEvent->GetIAAt(MinId-2)->GetType() == "COMP") {
    Ideal = Top->GetMotherEnergy()-Bottom->GetMotherEnergy();
  } else {
    Ideal = Top->GetEnergy()-Bottom->GetMotherEnergy();
  }

  return Ideal;
}


/******************************************************************************
 * Only works for 2D Strip!
 */
bool ResponseGenerator::IsRevanHit(MRERawEvent* RE, MVector Pos)
{
  for (int i = 0; i < RE->GetNRESEs(); ++i) {
    MRESE* RESE = RE->GetRESEAt(i);
    if (RESE->GetType() == MRESE::c_Hit) {
      if ((Pos - RESE->GetPosition()).Mag() < 0.01) return true;
    } else if (RESE->GetType() == MRESE::c_Track) {
      for (int j = 0; j < RESE->GetNRESEs(); ++j) {
        if (RESE->GetRESEAt(j)->GetType() == MRESE::c_Hit) {
          if ((Pos - RESE->GetRESEAt(j)->GetPosition()).Mag() < 0.01) return true;
        } else if (RESE->GetRESEAt(j)->GetType() == MRESE::c_Cluster) {
          for (int k = 0; k < RESE->GetRESEAt(j)->GetNRESEs(); ++k) {
            if ((Pos - RESE->GetRESEAt(j)->GetRESEAt(k)->GetPosition()).Mag() < 0.01) return true;
          }
        }
      }
    } else if (RESE->GetType() == MRESE::c_Cluster) {
      for (int k = 0; k < RESE->GetNRESEs(); ++k) {
        if ((Pos - RESE->GetRESEAt(k)->GetPosition()).Mag() < 0.01) return true;
      }
    }
  }

  return false;
}


/******************************************************************************
 *
 */
bool ResponseGenerator::IsSequenceOk(MRERawEvent* RE)
{
  // Check if the given sequence is ok:
  //           MRESEIterator Iter;
  //           Iter.Start(RE->GetStartPoint());
//           double Etot = RE->GetEnergy();
            
//           Iter.GetNextRESE();
//           bool GoodSequenceExists = true;
//           int StartPosition = 0;
//           while (Iter.GetNext() != 0) {
//             StartPosition++;
//             Etot -= Iter.GetCurrent()->GetEnergy();
//             if (IsComptonSequence(*Iter.GetCurrent(), *Iter.GetNext(), StartPosition, Etot) == false) {
//               GoodSequenceExists = false;
//               break;
//             }
//             Iter.GetNextRESE();        
//           }

  massert(RE->GetNRESEs() >= 2);
  bool GoodSequenceExists = true;
  MRESEIterator Iter;
  Iter.Start(RE->GetStartPoint());
  Iter.GetNextRESE();
  
  if (IsComptonStart(*Iter.GetCurrent(), -RE->GetEnergy(), RE->GetEnergyResolution()) == false) {  //??
    GoodSequenceExists = false;
  }
  if (IsSingleCompton(*Iter.GetCurrent()) == false) {  
    GoodSequenceExists = false;
  }
  if (IsComptonSequence(*Iter.GetCurrent(), 
                        *Iter.GetNext(), 1, 
                        -(RE->GetEnergy() - Iter.GetCurrent()->GetEnergy()),
                        sqrt(RE->GetEnergyResolution()*RE->GetEnergyResolution()-
                             Iter.GetCurrent()->GetEnergy()*Iter.GetCurrent()->GetEnergy())) == false) {
    GoodSequenceExists = false;
  }

  // Check if ARM is ok:
  MComptonEvent* C = (MComptonEvent*) RE->GetPhysicalEvent();
  if (C != 0) {
    bool GoodSequenceExists = true;
    if (C->GetARMGamma(m_SiEvent->GetIAAt(0)->GetPosition())*c_Deg > 10) {
      GoodSequenceExists = false;
    }
    // delete C; // event belongs to RE!!!!
  }

  return GoodSequenceExists;
}


/******************************************************************************
 *
 */
bool ResponseGenerator::IsAbsorbed(MRERawEvent* RE)
{
  // Check if the raw event is absorbed:

  if (fabs(m_SiEvent->GetIAAt(0)->GetEnergy() - RE->GetEnergy()) > 3*RE->GetEnergyResolution()) {
    return false;
  }

//   if (m_SiEvent->GetEnergyDepositNotSensitiveMaterial() != 0) {
//     return false;
//   }

//   for (unsigned int i = 0; i < m_SiEvent->GetNIAs(); ++i) {
//     if (m_SiEvent->GetIAAt(i)->GetType() == "ESCP") {
//       return false;
//     }
//   }

//   // Check if all HTs are present in RE
//   int Number = 0;
//   for (int i = 0; i < RE->GetNRESEs(); ++i) {
//     MRESE* RESE = RE->GetRESEAt(i);
//     if (RESE->GetType() == MRESE::c_Hit) {
//       Number++;
//     } else if (RESE->GetType() == MRESE::c_Track) {
//       for (int j = 0; j < RESE->GetNRESEs(); ++j) {
//         if (RESE->GetRESEAt(j)->GetType() == MRESE::c_Hit) {
//           Number++;
//         } else if (RESE->GetRESEAt(j)->GetType() == MRESE::c_Cluster) {
//           Number += RESE->GetRESEAt(j)->GetNRESEs();
//         }
//       }
//     } else if (RESE->GetType() == MRESE::c_Cluster) {
//       Number += RESE->GetNRESEs();
//     }
//   }

//   if (Number != m_SiEvent->GetNHTs()) {
//     mout<<"Hits: "<<Number<<" HTs: "<<m_SiEvent->GetNHTs()<<endl;
//     return false;
//   }


  return true;

//   // Correct Absorbed: 
//   double IEnergy = m_SiEvent->GetIAAt(0)->GetEnergy();
//   double REnergy = 0;
//   for (unsigned int h = 0; h < m_SiEvent->GetNHTs(); ++h) {
//     REnergy += m_SiEvent->GetHTAt(h)->GetEnergy();
//   }
//   cout<<"Checking global...";
//   if (fabs(REnergy-IEnergy)/IEnergy > m_MaxEnergyDifferencePercent &&
//       fabs(REnergy-IEnergy) > m_MaxEnergyDifference) {
//     cout<<"failed"<<endl;
//     return false;
//   }
//   cout<<"ok"<<endl;

//   if (IsSequenceOk(RE) == true) {
//         // Look at start:
//     MRESEIterator Iter;
//     Iter.Start(RE->GetStartPoint());
//     Iter.GetNextRESE();
//     double Etot = RE->GetEnergy();
    
//     // Test (2)
//     vector<int> StartIds = GetReseIds(*Iter.GetCurrent());
//     vector<int> StartOriginIds = GetOriginIds(StartIds);
//     if (StartOriginIds.size() == 0) {
//       mdebug<<"IsAbsorbed: Start has no Sim IDs!"<<endl;
//       return false;
//     }
    
//     // Absorption:
//     cout<<"Looking a "<<Iter.GetCurrent()->GetID()<<": "<<Etot<<endl;
//     if (IsTotalAbsorbed(StartOriginIds, Etot) == false) {
//       mout<<"IsAbsorbed: Not completely absorbed: "<<Iter.GetCurrent()->GetID()<<endl;
//       return false;
//     }
    
//     Etot -= Iter.GetCurrent()->GetEnergy();
//     Iter.GetNextRESE();
    
//     // Test (2)
//     StartIds = GetReseIds(*Iter.GetCurrent());
//     StartOriginIds = GetOriginIds(StartIds);
//     if (StartOriginIds.size() == 0) {
//       mdebug<<"IsAbsorbed: Start has no Sim IDs!"<<endl;
//       return false;
//     }
    
//     // Absorption:
//     cout<<"Looking a "<<Iter.GetCurrent()->GetID()<<": "<<Etot<<endl;
//     if (IsTotalAbsorbed(StartOriginIds, Etot) == false) {
//       mout<<"IsAbsorbed: Not completely absorbed: "<<Iter.GetCurrent()->GetID()<<endl;
//       return false;
//     }
//   }

  return true;
}

/******************************************************************************
 * Do whatever analysis is necessary
 */
bool ResponseGenerator::StartHitMissingEnergy()
{
  if (m_SiEvent->GetIAAt(1)->GetDetector() == 0) return true;

  vector<int> OriginateFromTwo;
  OriginateFromTwo.push_back(2);

  // Check for dependents:
  for (unsigned int i = 0; i < m_SiEvent->GetNIAs(); ++i) {
    for (unsigned int a = 0; a < OriginateFromTwo.size(); ++a) {
      if (m_SiEvent->GetIAAt(i)->GetOrigin() == OriginateFromTwo[a]) {
        OriginateFromTwo.push_back(i+1);
        break;
      }
    }
  }

  for (unsigned int a = 0; a < OriginateFromTwo.size(); ++a) {
    if (m_SiEvent->GetIAAt(OriginateFromTwo[a]-1)->GetType() == "ESCP") return true;
  }

  double REnergy = 0;
  for (unsigned int i = 0; i < m_SiEvent->GetNHTs(); ++i) {
    if (m_SiEvent->GetHTAt(i)->IsOrigin(2) == true) {
      REnergy += m_SiEvent->GetHTAt(i)->GetEnergy();
    }
  }
  double IEnergy = m_SiEvent->GetIAAt(0)->GetEnergy() - 
    m_SiEvent->GetIAAt(1)->GetMotherEnergy();
  
  if ((REnergy-IEnergy)/IEnergy < -m_MaxEnergyDifferencePercent &&
      REnergy-IEnergy < -m_MaxEnergyDifference) {
    return true;
  }

  return false;
}


/******************************************************************************
 * Do whatever analysis is necessary
 */
bool ResponseGenerator::LastEscape()
{
  for (unsigned int i = 0; i < m_SiEvent->GetNIAs(); ++i) {
    if (m_SiEvent->GetIAAt(i)->GetOrigin() == 1) {
      if (m_SiEvent->GetIAAt(i)->GetType() == "ESCP") return true;
    }    
  }

  return false;
}


/******************************************************************************
 * Do whatever analysis is necessary
 */
bool ResponseGenerator::Track()
{
  MRawEventList* REList = m_ReReader->GetRawEventList();
  massert(REList != 0);

  if (REList->GetNRawEvents() != 1) return false;

  MRERawEvent* RE = REList->GetRawEventAt(0);
  massert(RE != 0);

  if (RE->GetNRESEs() == 0) return false;

  for (unsigned int i = 0; i < m_SiEvent->GetNIAs(); ++i) {
    if (m_SiEvent->GetIAAt(i)->GetParticleNumber() == 2 ||
        m_SiEvent->GetIAAt(i)->GetParticleNumber() == 3) {
      MRESEIterator Iter;
      Iter.Start(RE->GetStartPoint());
      Iter.GetNextRESE();
      
      int NHits = 0;
      do {
        //vector<int> StartIds = GetReseIds(*(Iter.GetCurrent()));
        vector<int> StartOriginIds = GetOriginIds(Iter.GetCurrent());

        for (unsigned int s = 0; s < StartOriginIds.size(); ++s) {
          if (StartOriginIds[s] == m_SiEvent->GetIAAt(i)->GetNumber()) {
            NHits++;
          }
        }
        
        if (NHits >= 2) {
          return true;
        }

        Iter.GetNextRESE();
      } while (Iter.GetCurrent() != 0);  
    }
  }

  return false;
}


/******************************************************************************
 * Do whatever analysis is necessary
 */
bool ResponseGenerator::LongTrack()
{
  MRawEventList* REList = m_ReReader->GetRawEventList();
  massert(REList != 0);

  if (REList->GetNRawEvents() != 1) return false;

  MRERawEvent* RE = REList->GetRawEventAt(0);
  massert(RE != 0);

  if (RE->GetNRESEs() == 0) return false;

  MRESEIterator Iter;
  Iter.Start(RE->GetStartPoint());
  Iter.GetNextRESE();

  do {
    vector<int> StartIds = GetReseIds(Iter.GetCurrent());
    vector<int> StartOriginIds = GetOriginIds(Iter.GetCurrent());
    if (StartOriginIds.size() == 0) {
      mdebug<<"Start has no Sim IDs!"<<endl;
      return false;
    }
    
    int NComptons = 0;
    for (unsigned int i = 0; i < StartOriginIds.size(); ++i) {
      if (m_SiEvent->GetIAAt(StartOriginIds[i]-1)->GetType() == "COMP") {
        NComptons++;
      }
    }
    if (NComptons == 1 && StartIds.size() >= 4) return true;

    Iter.GetNextRESE();
  } while (Iter.GetCurrent() != 0);  

  return false;
}


/******************************************************************************
 * Do whatever analysis is necessary
 */
bool ResponseGenerator::AddOnHit()
{
  MRawEventList* REList = m_ReReader->GetRawEventList();
  massert(REList != 0);

  if (REList->GetNRawEvents() != 1) return false;

  MRERawEvent* RE = REList->GetRawEventAt(0);
  massert(RE != 0);

  if (RE->GetNRESEs() == 0) return false;
  
  MRESEIterator Iter;
  Iter.Start(RE->GetStartPoint());
  Iter.GetNextRESE();

  do {
    //vector<int> StartIds = GetReseIds(*(Iter.GetCurrent()));
    vector<int> StartOriginIds = GetOriginIds(Iter.GetCurrent());
    if (StartOriginIds.size() == 0) {
      mdebug<<"Start has no Sim IDs!"<<endl;
      return false;
    }
    
    bool HasCompton = false;
    for (unsigned int i = 0; i < StartOriginIds.size(); ++i) {
      if (m_SiEvent->GetIAAt(StartOriginIds[i]-1)->GetType() == "COMP" ||
          m_SiEvent->GetIAAt(StartOriginIds[i]-1)->GetType() == "PHOT") {
        HasCompton = true;
      }
    }
    if (HasCompton == false) return true;

    Iter.GetNextRESE();
  } while (Iter.GetCurrent() != 0);  

  return false;
}


/******************************************************************************
 * Do whatever analysis is necessary
 */
bool ResponseGenerator::HighDoppler()
{
  const double Limit = 5;

  if (m_SiEvent->GetNIAs() >= 2 && m_SiEvent->GetIAAt(1)->GetType() == "COMP") {
    if (fabs(m_SiEvent->GetIAAt(0)->GetEnergy() - 
             m_SiEvent->GetIAAt(1)->GetEnergy() - 
             m_SiEvent->GetIAAt(1)->GetMotherEnergy()) > Limit) {
      return true;
    }
  }
  if (m_SiEvent->GetNIAs() >= 3 && m_SiEvent->GetIAAt(2)->GetType() == "COMP") {
    if (fabs(m_SiEvent->GetIAAt(1)->GetMotherEnergy() - 
             m_SiEvent->GetIAAt(2)->GetEnergy() - 
             m_SiEvent->GetIAAt(2)->GetMotherEnergy()) > Limit) {
      return true;
    }
  }
  if (m_SiEvent->GetNIAs() >= 4 && m_SiEvent->GetIAAt(3)->GetType() == "COMP") {
    if (fabs(m_SiEvent->GetIAAt(2)->GetMotherEnergy() - 
             m_SiEvent->GetIAAt(3)->GetEnergy() - 
             m_SiEvent->GetIAAt(3)->GetMotherEnergy()) > Limit) {
      return true;
    }
  }
  if (m_SiEvent->GetNIAs() >= 5 && m_SiEvent->GetIAAt(4)->GetType() == "COMP") {
    if (fabs(m_SiEvent->GetIAAt(3)->GetMotherEnergy() - 
             m_SiEvent->GetIAAt(4)->GetEnergy() - 
             m_SiEvent->GetIAAt(4)->GetMotherEnergy()) > Limit) {
      return true;
    }
  }

  return false;
}


/******************************************************************************
 * Do whatever analysis is necessary
 */
bool ResponseGenerator::SeveralComptonsInStart()
{
  MRawEventList* REList = m_ReReader->GetRawEventList();
  massert(REList != 0);

  if (REList->GetNRawEvents() != 1) return false;

  MRERawEvent* RE = REList->GetRawEventAt(0);
  massert(RE != 0);

  if (RE->GetNRESEs() == 0) return false;

  MRESE* Start = RE->GetStartPoint();

  //vector<int> StartIds = GetReseIds(*Start);
  vector<int> StartOriginIds = GetOriginIds(Start);
  if (StartOriginIds.size() == 0) {
    mdebug<<"Start has no Sim IDs!"<<endl;
    return false;
  }

  bool Found2 = false;
  bool Found3 = false;

  for (unsigned int i = 0; i < StartOriginIds.size(); ++i) {
    if (StartOriginIds[i] == 2) {
      Found2 = true;
    } else if (StartOriginIds[i] == 3) {
      Found3 = true;
    }
  }

  if (Found2 == true && Found3 == true) return true;

  return false;
}


/******************************************************************************
 * Do whatever analysis is necessary
 */
bool ResponseGenerator::SeveralComptonsInCenter()
{
  MRawEventList* REList = m_ReReader->GetRawEventList();
  massert(REList != 0);

  if (REList->GetNRawEvents() != 1) return false;

  MRERawEvent* RE = REList->GetRawEventAt(0);
  massert(RE != 0);

  if (RE->GetNRESEs() == 0) return false;

  MRESE* Start = RE->GetStartPoint();

  MRESEIterator Iter;
  Iter.Start(Start);
  Iter.GetNextRESE();

  do {
    if (IsComptonEnd(*Iter.GetCurrent()) == false) {
      //vector<int> StartIds = GetReseIds(*Iter.GetCurrent());
      vector<int> StartOriginIds = GetOriginIds(Iter.GetCurrent());
      if (StartOriginIds.size() == 0) {
        mdebug<<"Start has no Sim IDs!"<<endl;
        return false;
      }
      
      int Smallest = numeric_limits<int>::max();
      
      for (unsigned int i = 0; i < StartOriginIds.size(); ++i) {
        if (StartOriginIds[i] < Smallest) {
          Smallest = StartOriginIds[i];
        }
      }
      
      for (unsigned int i = 0; i < StartOriginIds.size(); ++i) {
        if (StartOriginIds[i] == Smallest+1) {
          if (m_SiEvent->GetIAAt(StartOriginIds[i]-1)->GetType() == "COMP") {
            mdebug<<"SeveralComptonCenter: Multiple Comptons!"<<endl;
            return true;
          }
        }
      }    
    }

    Iter.GetNextRESE();
  } while (Iter.GetCurrent() != 0);

  return false;
}


/******************************************************************************
 * Do whatever analysis is necessary
 */
bool ResponseGenerator::MissingStartHit()
{
  MRawEventList* REList = m_ReReader->GetRawEventList();
  massert(REList != 0);

  if (REList->GetNRawEvents() != 1) return false;

  MRERawEvent* RE = REList->GetRawEventAt(0);
  massert(RE != 0);

  if (RE->GetNRESEs() == 0) return false;

  MRESEIterator Iter;
  Iter.Start(RE->GetStartPoint());
  Iter.GetNextRESE();

  bool Found2 = false;
  bool Found3 = false;
  do {
    //vector<int> StartIds = GetReseIds(*(Iter.GetCurrent()));
    vector<int> StartOriginIds = GetOriginIds(Iter.GetCurrent());
    if (StartOriginIds.size() == 0) {
      mdebug<<"Start has no Sim IDs!"<<endl;
      return false;
    }
    
    for (unsigned int i = 0; i < StartOriginIds.size(); ++i) {
      if (StartOriginIds[i] == 2) {
        Found2 = true;
      }
      if (StartOriginIds[i] == 3) {
        Found3 = true;
      }
    }

    Iter.GetNextRESE();
  } while (Iter.GetCurrent() != 0);

  if (Found2 == false || Found3 == false) return true;

  return false;
}


/******************************************************************************
 * Do whatever analysis is necessary
 */
bool ResponseGenerator::GenerateTrackPdf()
{
  //cout<<"Trying to generate a pdf for track recognition - please stand by..."<<endl;

  vector<Quadruple> QuadruplesGoodCenter;
  vector<Quadruple> QuadruplesBadCenter;
  vector<Quadruple> QuadruplesGoodStop;
  vector<Quadruple> QuadruplesBadStop;
  vector<Quadruple> QuadruplesGoodStart;
  vector<Quadruple> QuadruplesBadStart;
  // vector<Quadruple>::iterator QuadruplesIter;

  MRETrack* Track = 0;
  MRESE* RESE = 0;
  MRERawEvent* RE = 0;
  MRawEventList* REList = 0;

  int r_max, e_max;

  // the data:
  double Etot;
  double Edep;
  double AngleIn;
  double AngleOutTheta;
  double AngleOutPhi;

  // create axis:
  vector<float> Axis;
  vector<float> X1Axis;
  vector<float> X2Axis;
  vector<float> X3Axis;
  vector<float> X4Axis;
  vector<float> X5Axis;
  

  vector<float> EmptyAxis;
  EmptyAxis.push_back(1E-10);
  EmptyAxis.push_back(1E+10);


  // Global good/bad:
  X1Axis.clear();
  X1Axis = CreateEquiDist(0, 2, 2);
  MResponseMatrixO1 GoodBadTable("GoodBadTable", X1Axis); 
  GoodBadTable.SetAxisNames("GoodBad");


  // Matrix Dual:

  // Etot
  X1Axis.clear();
  X1Axis = CreateEquiDist(1, 500, 15);
  Axis = CreateEquiDist(600, 1000, 4, c_NoBound, 10000);
  copy(Axis.begin(), Axis.end(), back_inserter(X1Axis));
 
  // Angle in
  X2Axis.clear();
  X2Axis = CreateEquiDist(0, 90, 6);
  //X2Axis = CreateEquiDist(0, 90, 1);

  // Edep
  X3Axis.clear();
  X3Axis = CreateEquiDist(1, 500, 15); // Mega
  // X3Axis = CreateEquiDist(1, 500, 50); // Superior
  Axis = CreateEquiDist(600, 1000, 4, c_NoBound, 10000);
  copy(Axis.begin(), Axis.end(), back_inserter(X3Axis));

  MResponseMatrixO3 PdfDualGood("TracksStartGood", X1Axis, X2Axis, X3Axis);
  PdfDualGood.SetAxisNames("E_{tot}", "Angle_{In}", "E_{dep}");
  MResponseMatrixO3 PdfDualBad("TracksStartBad", X1Axis, X2Axis, X3Axis);
  PdfDualBad.SetAxisNames("E_{tot}", "Angle_{In}", "E_{dep}");


  // Matrix Start:

  // Etot
  X1Axis.clear();
  X1Axis = CreateEquiDist(1, 1500, 10);
  Axis = CreateEquiDist(1800, 3000, 4);
  copy(Axis.begin(), Axis.end(), back_inserter(X1Axis));
  Axis = CreateEquiDist(4000, 10000, 6, c_NoBound, 100000);
  copy(Axis.begin(), Axis.end(), back_inserter(X1Axis));

  // Angle in
  X2Axis.clear();
  X2Axis = CreateEquiDist(0, 90, 6);
  //X2Axis = CreateEquiDist(0, 90, 1);

  // Edep
  X3Axis.clear();
  // X3Axis = CreateEquiDist(1, 500, 20); // Mega
  X3Axis = CreateEquiDist(1, 500, 50); // Superior
  Axis = CreateEquiDist(600, 1000, 5, c_NoBound, 10000);
  copy(Axis.begin(), Axis.end(), back_inserter(X3Axis));

  MResponseMatrixO3 PdfStartGood("TracksStartGood", X1Axis, X2Axis, X3Axis);
  PdfStartGood.SetAxisNames("E_{tot}", "Angle_{In}", "E_{dep}");
  MResponseMatrixO3 PdfStartBad("TracksStartBad", X1Axis, X2Axis, X3Axis);
  PdfStartBad.SetAxisNames("E_{tot}", "Angle_{In}", "E_{dep}");


  // Matrix Central:

  // Edep:
  X3Axis.clear();
  // X3Axis = CreateEquiDist(1, 100, 2);  // Mega
  // Axis = CreateEquiDist(115, 400, 19); // Mega
  // copy(Axis.begin(), Axis.end(), back_inserter(X3Axis));
  X3Axis = CreateEquiDist(1, 400, 40); // Superior
  Axis = CreateEquiDist(500, 1000, 5, c_NoBound, 10000);
  copy(Axis.begin(), Axis.end(), back_inserter(X3Axis));

  // out phi
  X4Axis.clear();
  //X4Axis = CreateEquiDist(0, 180, 1);
  X4Axis = CreateEquiDist(0, 180, 6);

  // out theta
  X5Axis.clear();
  //X5Axis = CreateEquiDist(0, 180, 1);
  X5Axis = CreateEquiDist(0, 90, 6);
  Axis = CreateEquiDist(120, 180, 2);
  copy(Axis.begin(), Axis.end(), back_inserter(X5Axis));


  MResponseMatrixO5 PdfGood("TracksGood", X1Axis, X2Axis, X3Axis, X4Axis, X5Axis);
  PdfGood.SetAxisNames("E_{tot}", "Angle_{In}", "E_{dep}", "Angle_{Out, #varphi}", "Angle_{Out, #vartheta}");
  MResponseMatrixO5 PdfBad("TracksBad", X1Axis, X2Axis, X3Axis, X4Axis, X5Axis);
  PdfBad.SetAxisNames("E_{tot}", "Angle_{In}", "E_{dep}", "Angle_{Out, #varphi}", "Angle_{Out, #vartheta}");


  // Matrix Stop:

  // Etot
  X1Axis.clear();
  // X1Axis = CreateEquiDist(1, 1500, 45); // Mega
  X1Axis = CreateEquiDist(1, 1500, 150); // Superior
  Axis = CreateEquiDist(1800, 3000, 12);
  copy(Axis.begin(), Axis.end(), back_inserter(X1Axis));
  Axis = CreateEquiDist(4000, 10000, 6, c_NoBound, 100000);
  copy(Axis.begin(), Axis.end(), back_inserter(X1Axis));

  // Angle in
  X2Axis.clear();
  X2Axis = CreateEquiDist(0, 90, 45);

  MResponseMatrixO2 PdfStopGood("PdfStopGood", X1Axis, X2Axis);
  PdfStopGood.SetAxisNames("E_{tot}", "Angle_{In}");
  MResponseMatrixO2 PdfStopBad("PdfStopBad", X1Axis, X2Axis);
  PdfStopBad.SetAxisNames("E_{tot}", "Angle_{In}");


  int Counter = 0;
  while (LoadEvents() == true) {

    //g_DebugLevel = 1;

    //cout<<"New event"<<endl;
    
    RESE = 0;
    RE = 0;
    REList = m_ReReader->GetRawEventList();

    QuadruplesGoodStart.clear();
    QuadruplesBadStart.clear();

    QuadruplesGoodCenter.clear();
    QuadruplesBadCenter.clear();

    QuadruplesGoodStop.clear();
    QuadruplesBadStop.clear();

    // First try to find the tracks in all the RawEvents:
    r_max = REList->GetNRawEvents();
    for (int r = 0; r < r_max; ++r) {
      RE = REList->GetRawEventAt(r);

      
      e_max = RE->GetNRESEs();
      for (int e = 0; e < e_max; ++e) {
        RESE = RE->GetRESEAt(e);

        if (RESE->GetType() != MRESE::c_Track) continue;

        bool AllGood = true;

        Track = (MRETrack*) RESE;
        mdebug<<"Looking at track: "<<Track->ToString()<<endl;

        MRESEIterator Iter;
        Iter.Start(Track->GetStartPoint());

        if (Iter.GetNRESEs() < 2) continue;


        if (Iter.GetNRESEs() == 2) {
          Iter.GetNextRESE();
          Etot = Track->GetEnergy();
          if (IsTrackStart(*Iter.GetCurrent(), *Iter.GetNext(), Etot) == true &&
              IsTrackStop(*Iter.GetCurrent(), *Iter.GetNext(), Etot - Iter.GetCurrent()->GetEnergy()) == true) {
            mdebug<<"GOOD dual: "<<Iter.GetCurrent()->GetID()<<" - "<<Iter.GetNext()->GetID()<<" - "<<Etot<<endl;
            PdfDualGood.Add(Etot, 
                            CalculateAngleIn(*Iter.GetCurrent(), *Iter.GetNext())*c_Deg,
                            Iter.GetCurrent()->GetEnergy());
          } else {
            mdebug<<"BAD dual: "<<Iter.GetCurrent()->GetID()<<" - "<<Iter.GetNext()->GetID()<<" - "<<Etot<<endl;
            PdfDualBad.Add(Etot, 
                           CalculateAngleIn(*Iter.GetCurrent(), *Iter.GetNext())*c_Deg,
                           Iter.GetCurrent()->GetEnergy());
            AllGood = false;
          }
        } else {
          // Start of the track:
          Iter.GetNextRESE();
          Etot = Track->GetEnergy();
          if (IsTrackStart(*Iter.GetCurrent(), *Iter.GetNext(), Etot) == true) {
            mdebug<<"GOOD start: "<<Iter.GetCurrent()->GetID()<<" - "<<Iter.GetNext()->GetID()<<" - "<<Etot<<endl;
            PdfStartGood.Add(Etot, 
                             CalculateAngleIn(*Iter.GetCurrent(), *Iter.GetNext())*c_Deg,
                             Iter.GetCurrent()->GetEnergy());
          } else {
            mdebug<<"BAD start: "<<Iter.GetCurrent()->GetID()<<" - "<<Iter.GetNext()->GetID()<<" - "<<Etot<<endl;
            PdfStartBad.Add(Etot, 
                              CalculateAngleIn(*Iter.GetCurrent(), *Iter.GetNext())*c_Deg,
                            Iter.GetCurrent()->GetEnergy());
            AllGood = false;
          }

        
          // Central part of the track
          Iter.GetNextRESE();
          while (Iter.GetNext() != 0) {
            Etot -= Iter.GetPrevious()->GetEnergy();

            // Decide if it is good or bad...
            // In the current implementation/simulation the hits have to be in increasing order...
            if (AreReseInSequence(*Iter.GetPrevious(), *Iter.GetCurrent(), *Iter.GetNext(), Etot) == true) {
              // Retrieve the data:
              Edep = Iter.GetCurrent()->GetEnergy();
              AngleIn = CalculateAngleIn(*Iter.GetPrevious(), *Iter.GetCurrent())*c_Deg;
              AngleOutPhi = CalculateAngleOutPhi(*Iter.GetPrevious(), *Iter.GetCurrent(), *Iter.GetNext())*c_Deg;
              AngleOutTheta = CalculateAngleOutTheta(*Iter.GetPrevious(), *Iter.GetCurrent(), *Iter.GetNext())*c_Deg;
              mdebug<<"GOOD central: "<<Iter.GetPrevious()->GetID()<<" - "<<Iter.GetCurrent()->GetID()
                    <<" - "<<Iter.GetNext()->GetID()<<" - "<<Etot<<endl;
              PdfGood.Add(Etot, AngleIn, Edep, AngleOutPhi, AngleOutTheta, 1);
            } else {
              // Retrieve the data:
              Edep = Iter.GetCurrent()->GetEnergy();
              AngleIn = CalculateAngleIn(*Iter.GetPrevious(), *Iter.GetCurrent())*c_Deg;
              AngleOutPhi = CalculateAngleOutPhi(*Iter.GetPrevious(), *Iter.GetCurrent(), *Iter.GetNext())*c_Deg;
              AngleOutTheta = CalculateAngleOutTheta(*Iter.GetPrevious(), *Iter.GetCurrent(), *Iter.GetNext())*c_Deg;
              PdfBad.Add(Etot, AngleIn, Edep, AngleOutPhi, AngleOutTheta, 1);
              mdebug<<"BAD central: "<<Iter.GetPrevious()->GetID()<<" - "<<Iter.GetCurrent()->GetID()
                    <<" - "<<Iter.GetNext()->GetID()<<" - "<<Etot<<endl;

              AllGood = false;
            } // Add good / bad
            Iter.GetNextRESE();
          } // If we have a next element
          

          // The stop section:
          Etot -= Iter.GetPrevious()->GetEnergy();
          
          if (IsTrackStop(*Iter.GetPrevious(), *Iter.GetCurrent(), Etot) == true) {
            mdebug<<"GOOD stop: "<<Iter.GetPrevious()->GetID()
                  <<" - "<<Iter.GetCurrent()->GetID()<<" - "<<Etot<<endl;
            PdfStopGood.Add(Etot, CalculateAngleIn(*Iter.GetPrevious(), *Iter.GetCurrent())*c_Deg);
          } else {
            mdebug<<"BAD stop: "<<Iter.GetPrevious()->GetID()
                  <<" - "<<Iter.GetCurrent()->GetID()<<" - "<<Etot<<endl;
            PdfStopBad.Add(Etot, CalculateAngleIn(*Iter.GetPrevious(), *Iter.GetCurrent())*c_Deg);
            AllGood = false;
          }
        }
        if (AllGood == false) {
          GoodBadTable.Add(0.5, 1);
          //mdebug<<"No good sequence exists"<<endl<<endl<<endl<<endl;
        } else {
          GoodBadTable.Add(1.5, 1);
          mdebug<<"One good sequence exists"<<endl<<endl<<endl<<endl;
        }
      } // all reses
    } // all raw events
    
      // mdebug<<"Good triples:"<<endl;
    for (unsigned int t = 0; t < QuadruplesGoodCenter.size(); ++t) {
      //cout<<"t: "<<QuadruplesGood[t];
      //mdebug<<t<<": "<<QuadruplesGood[t]<<endl;
    }
      
    //mdebug<<"Bad triples:"<<endl;
    for (unsigned int t = 0; t < QuadruplesBadCenter.size(); ++t) {
      //mdebug<<t<<": "<<QuadruplesBad[t]<<endl;
    }
    
    if (++Counter % m_SaveAfter == 0) {
      GoodBadTable.Write((m_ResponseName + ".t.goodbad.rsp").c_str(), true);
  
      PdfStartGood.Write((m_ResponseName + ".t.start.good.rsp").c_str(), true);
      PdfStartBad.Write((m_ResponseName + ".t.start.bad.rsp").c_str(), true);
  
      PdfDualGood.Write((m_ResponseName + ".t.dual.good.rsp").c_str(), true);
      PdfDualBad.Write((m_ResponseName + ".t.dual.bad.rsp").c_str(), true);
      
      PdfGood.Write((m_ResponseName + ".t.central.good.rsp").c_str(), true);
      PdfBad.Write((m_ResponseName + ".t.central.bad.rsp").c_str(), true);
      
      PdfStopGood.Write((m_ResponseName + ".t.stop.good.rsp").c_str(), true);
      PdfStopBad.Write((m_ResponseName + ".t.stop.bad.rsp").c_str(), true);
    }

    if (m_Interrupt == true) break;

    //g_DebugLevel = 0;
  }
  
  GoodBadTable.Write((m_ResponseName + ".t.goodbad.rsp").c_str(), true);
  
  PdfStartGood.Write((m_ResponseName + ".t.start.good.rsp").c_str(), true);
  PdfStartBad.Write((m_ResponseName + ".t.start.bad.rsp").c_str(), true);
  
  PdfDualGood.Write((m_ResponseName + ".t.dual.good.rsp").c_str(), true);
  PdfDualBad.Write((m_ResponseName + ".t.dual.bad.rsp").c_str(), true);

  PdfGood.Write((m_ResponseName + ".t.central.good.rsp").c_str(), true);
  PdfBad.Write((m_ResponseName + ".t.central.bad.rsp").c_str(), true);

  PdfStopGood.Write((m_ResponseName + ".t.stop.good.rsp").c_str(), true);
  PdfStopBad.Write((m_ResponseName + ".t.stop.bad.rsp").c_str(), true);

  return true;
}


/******************************************************************************
 * 
 */
double ResponseGenerator::CalculateAngleIn(MRESE& Start, MRESE& Central)
{
  static long NWarnings = 0;
  if (NWarnings == 0) {
    merr<<"Does not work for all geometries --> Only Mega-type!"<<endl;
    // --> RevanGeometry weiﬂ wie...
  }
  NWarnings = 1;
  
  double Angle = (Central.GetPosition()-Start.GetPosition()).Angle(MVector(0, 0, 1));
  if (Angle > TMath::Pi()/2) Angle = TMath::Pi() - Angle;

  return Angle;
}


/******************************************************************************
 *
 */
double ResponseGenerator::CalculateAngleOutTheta(MRESE& Start, MRESE& Central, MRESE& Stop)
{
  MVector In = Central.GetPosition() - Start.GetPosition();
  MVector Out = Stop.GetPosition() - Central.GetPosition();

  return In.Angle(Out);
}


/******************************************************************************
 * 
 */
double ResponseGenerator::CalculateAngleOutPhi(MRESE& Start, MRESE& Central, MRESE& Stop)
{
  static long NWarnings = 0;
  if (NWarnings == 0) {
    merr<<"Does not work for all geometries --> Only Mega-type!"<<endl;
    // --> RevanGeometry weiﬂ wie...
  }
  NWarnings = 1;

  MVector E1 = (Central.GetPosition() - Start.GetPosition()).Cross(MVector(0, 0, 1));
  MVector E2 = (Central.GetPosition() - Start.GetPosition()).Cross(Stop.GetPosition() - Central.GetPosition());

  return E1.Angle(E2);
}


/******************************************************************************
 * Return true if the given RESEs are in sequence
 */
bool ResponseGenerator::IsTrackStart(MRESE& Start, MRESE& Central, double Energy)
{
  // According to the current simulation (GMega), the RESEs are in sequence if
  // (1) the IDs of their hits are in increasing order without holes
  // (2) all originate from the same IA
  // (3) The start condition is fullfilled, when the start hit has the lowest 
  //     time-tag of all hits with this origin

  const int IdOffset = 2;

  vector<int> StartIds = GetReseIds(&Start);
  vector<int> CentralIds = GetReseIds(&Central);
  if (StartIds.size() == 0) return false;
  if (CentralIds.size() == 0) return false;


  // Test (1)
  if (AreIdsInSequence(StartIds) == false) {
    mdebug<<"   Is track start: Start IDs not in sequence"<<endl;
    return false;
  }
  if (AreIdsInSequence(CentralIds) == false) {
    mdebug<<"Is track Start: Central IDs not in sequence"<<endl;
    return false;
  }

  // Test for holes:
  // The hit after the last one from start is the first from central:
  double TimeLastFirst = m_SiEvent->GetHTAt(StartIds.back()-IdOffset)->GetTime();
  double TimeFirstLast = m_SiEvent->GetHTAt(CentralIds.front()-IdOffset)->GetTime();
  for (unsigned int h = 0; h < m_SiEvent->GetNHTs(); ++h) {
    if (m_SiEvent->GetHTAt(h)->GetTime() > TimeLastFirst && 
        m_SiEvent->GetHTAt(h)->GetTime() < TimeFirstLast) {
      mdebug<<"   Is track start: Hit between first and central according to time: "
            <<h<<endl;
      mdebug<<"LF: "<<TimeLastFirst<<" FL: "<<TimeFirstLast<<endl;
      return false;      
    }
  } 


//   if (abs(CentralIds[0] - StartIds[StartIds.size()-1]) != 1 && 
//       abs(CentralIds[CentralIds.size()-1] - StartIds[0]) != 1) {
//     mdebug<<"   Is track start: Distance not equal 1: "
//           <<abs(CentralIds[0] - StartIds[StartIds.size()-1])<<", "
//           <<abs(CentralIds[CentralIds.size()-1] - StartIds[0])<<endl;
//     return false;
//   }
  
  // Test (2)
  int CommonOrigin = -1;
  for (unsigned int i = 0; i < StartIds.size(); ++i) {
    for (unsigned int j = 0; j < CentralIds.size(); ++j) {
      for (unsigned int oi = 0; oi < m_SiEvent->GetHTAt(StartIds[i]-IdOffset)->GetNOrigins(); ++oi) {
        for (unsigned int oj = 0; oj < m_SiEvent->GetHTAt(CentralIds[j]-IdOffset)->GetNOrigins(); ++oj) {
          if (m_SiEvent->GetHTAt(StartIds[i]-IdOffset)->GetOriginAt(oi) == 
              m_SiEvent->GetHTAt(CentralIds[j]-IdOffset)->GetOriginAt(oj)) {
            CommonOrigin = m_SiEvent->GetHTAt(CentralIds[j]-IdOffset)->GetOriginAt(oj);
            break;
          }
        }
      }
    }
  }

  if (CommonOrigin == -1) {
    mdebug<<"   Is track start: No common origin!"<<endl;
    return false;
  }

  // Test (3)
  double Timing = numeric_limits<double>::max();
  for (unsigned int i = 0; i < StartIds.size(); ++i) {
    if (m_SiEvent->GetHTAt(StartIds[i]-IdOffset)->GetTime() < Timing) {
      Timing = m_SiEvent->GetHTAt(StartIds[i]-IdOffset)->GetTime();
    }
  }
  unsigned int i_max = m_SiEvent->GetNHTs();
  for (unsigned int i = 0; i < i_max; ++i) {
    if (m_SiEvent->GetHTAt(i)->IsOrigin(CommonOrigin)) {
      if (m_SiEvent->GetHTAt(i)->GetTime() < Timing) {
        mdebug<<"   Is track start: Not first hit (timing)"<<endl;
        return false;
      }
    }
  }

  if (Energy > 0) {
    if (IsTrackCompletelyAbsorbed(StartIds, Energy) == false) {
      mdebug<<"   Is track start: Not completely absorbed"<<endl;
      return false;
    }
  }

  return true;
}



/******************************************************************************
 * Return true if the given RESEs are in sequence
 */
bool ResponseGenerator::IsTrackStop(MRESE& Central, 
                                    MRESE& Stop, double Energy)
{
  // According to the current simulation (GMega), the RESEs are in sequence if
  // (1) the IDs of their hits are in increasing order without holes
  // (2) all originate from the same IA (espec. the first and last hit in the 
  //     sequence)
  // (3) The start condition is fullfilled, when the start hit has the highest 
  //     time-tag of all hits with this origin

  const int IdOffset = 2;

  vector<int> CentralIds = GetReseIds(&Central);
  vector<int> StopIds = GetReseIds(&Stop);
  
  // Test (1)
  if (AreIdsInSequence(CentralIds) == false) {
    mdebug<<"   IsStop: Central IDs not in sequence"<<endl;
    return false;
  }
  if (AreIdsInSequence(StopIds) == false) {
    mdebug<<"   IsStop: Stop IDs not in sequence"<<endl;
    return false;
  }
  // Test for holes:
  // The hit after the last one from start is the first from central:
  double TimeLastFirst = m_SiEvent->GetHTAt(CentralIds.back()-IdOffset)->GetTime();
  double TimeFirstLast = m_SiEvent->GetHTAt(StopIds.front()-IdOffset)->GetTime();
  for (unsigned int h = 0; h < m_SiEvent->GetNHTs(); ++h) {
    if (m_SiEvent->GetHTAt(h)->GetTime() > TimeLastFirst && 
        m_SiEvent->GetHTAt(h)->GetTime() < TimeFirstLast) {
      mdebug<<"   IsStop: Hit between first and central according to time!"<<endl;
      return false;      
    }
  } 
//   if (abs(StopIds[0] - CentralIds[CentralIds.size()-1]) != 1 && 
//       abs(StopIds[StopIds.size()-1] - CentralIds[0]) != 1) {
//     mdebug<<"   IsStop: Distance not equal 1"<<endl;
//     return false;
//   }


  // Test (2)
  int CommonOrigin = -1;
  for (unsigned int i = 0; i < CentralIds.size(); ++i) {
    for (unsigned int j = 0; j < StopIds.size(); ++j) {
      for (unsigned int oi = 0; oi < m_SiEvent->GetHTAt(CentralIds[i]-IdOffset)->GetNOrigins(); ++oi) {
        for (unsigned int oj = 0; oj < m_SiEvent->GetHTAt(StopIds[j]-IdOffset)->GetNOrigins(); ++oj) {
          if (m_SiEvent->GetHTAt(CentralIds[i]-IdOffset)->GetOriginAt(oi) == 
              m_SiEvent->GetHTAt(StopIds[j]-IdOffset)->GetOriginAt(oj)) {
            CommonOrigin = m_SiEvent->GetHTAt(StopIds[j]-IdOffset)->GetOriginAt(oj);
            break;
          }
        }
      }
    }
  }

  if (CommonOrigin == -1) {
    mdebug<<"   IsStop: No common origin!"<<endl;
    return false;
  }

  // Test (3)
  double Timing = -numeric_limits<double>::max();
  for (unsigned int i = 0; i < StopIds.size(); ++i) {
    if (m_SiEvent->GetHTAt(StopIds[i]-IdOffset)->GetTime() > Timing) {
      Timing = m_SiEvent->GetHTAt(StopIds[i]-IdOffset)->GetTime();
    }
  }
  unsigned int i_max = m_SiEvent->GetNHTs();
  for (unsigned int i = 0; i < i_max; ++i) {
    if (m_SiEvent->GetHTAt(i)->IsOrigin(CommonOrigin)) {
      if (m_SiEvent->GetHTAt(i)->GetTime() > Timing) {
        mdebug<<"   IsStop: Not last hit (timing)"<<endl;
        return false;
      }
    }
  }

  if (IsTrackCompletelyAbsorbed(StopIds, Energy) == false) {
    mdebug<<"   Is track stop: Not completely absorbed"<<endl;
    return false;
  }

  return true;
}


/******************************************************************************
 * Return true if the track is completely absorbed
 */
bool ResponseGenerator::IsTrackCompletelyAbsorbed(const vector<int>& Ids, double Energy)
{
  // Prerequisites: Ids have only one common origin!
  // Realization: Nothing originates from this ID AFTER this hit:

  const int IdOffset = 2;

  // Get origin Id
  int Origin = -1;
  for (unsigned int i = 0; i < Ids.size(); ++i) {
    for (unsigned int oi = 0; oi < m_SiEvent->GetHTAt(Ids[i]-IdOffset)->GetNOrigins(); ++oi) {
      if (m_SiEvent->GetHTAt(Ids[i]-IdOffset)->GetOriginAt(oi) != 1) { 
        Origin = m_SiEvent->GetHTAt(Ids[i]-IdOffset)->GetOriginAt(oi);
        break;
      }
    }
  } 
  if (Origin <= 1) {
    mdebug<<"   IsTrackCompletelyAbsorbed: No origin"<<endl;
    return false;
  }

//   // Check if we do haven an IA originating from this - which is not part of this track:
//   for (unsigned int i = 0; i < m_SiEvent->GetNIAs(); ++i) {
//     if (m_SiEvent->GetIAAt(i)->GetOrigin() == Origin) {
//       if (m_SiEvent->GetIAAt(i)->GetType() == "BREM") {
//         // Search the closest hit:
//         int MinHT = -1;
//         double MinDist = numeric_limits<double>::max();
//         for (unsigned int h = 0; h < m_SiEvent->GetNHTs(); ++h) {
//           if (m_SiEvent->GetHTAt(h)->IsOrigin(Origin) == true) {
//             if ((m_SiEvent->GetIAAt(i)->GetPosition() - m_SiEvent->GetHTAt(h)->GetPosition()).Mag() < MinDist) {
//               MinDist = (m_SiEvent->GetIAAt(i)->GetPosition() - m_SiEvent->GetHTAt(h)->GetPosition()).Mag();
//               MinHT = h;
//             }
//           }
//         }
//         if (MinHT > 0) {
//           if (m_SiEvent->GetHTAt(MinHT)->GetTime() >= m_SiEvent->GetHTAt(Ids[0]-IdOffset)->GetTime()) {
//             mdebug<<"   IsTrackCompletelyAbsorbed: Bremsstrahlung emitted after this hit!"<<endl;
//             return false;
//           }
//         }
//       } else {
//         mdebug<<"   IsTrackCompletelyAbsorbed: Originating interaction: "<<m_SiEvent->GetIAAt(i)->GetType()<<endl;
//         return false;
//       }
//     }
//   }

  // Check for complete absorption:
  const double EnergyLimitPercent = 0.15; // %
  const double EnergyLimit = 20; //keV

  double RealEnergy = 0;
  for (unsigned int h = 0; h < m_SiEvent->GetNHTs(); ++h) {
    if (m_SiEvent->GetHTAt(h)->IsOrigin(Origin) == true) {
      if (m_SiEvent->GetHTAt(h)->GetTime() >= m_SiEvent->GetHTAt(Ids[0]-IdOffset)->GetTime()) {
        RealEnergy += m_SiEvent->GetHTAt(h)->GetEnergy();
      }
    }
  }
  
  if (fabs(Energy - RealEnergy) > EnergyLimit &&
      fabs(Energy - RealEnergy)/RealEnergy > EnergyLimitPercent) {
    mdebug<<"   IsTrackCompletelyAbsorbed: Missing/Not much energy: i="
          <<RealEnergy<<" r="<<Energy<<endl;
    return false;   
  }


  return  true;
}


/******************************************************************************
 * Return true if the given RESEs are in sequence
 */
bool ResponseGenerator::AreReseInSequence(MRESE& Start, 
                                          MRESE& Central, 
                                          MRESE& Stop, double Energy)
{
  // According to the current simulation (GMega), the RESEs are in sequence if
  // (1) the IDs of their hits are in increasing order without holes
  // (2) all originate from the same IA (espec. the first and last hit in the 
  //     sequence)

  const int IdOffset = 2;

  vector<int> StartIds = GetReseIds(&Start);
  vector<int> CentralIds = GetReseIds(&Central);
  vector<int> StopIds = GetReseIds(&Stop);
  
  // Test (1)
  if (AreIdsInSequence(StartIds) == false) {
    mdebug<<"   IsCentral: Start IDs not in sequence"<<endl;
    return false;
  }
  if (AreIdsInSequence(CentralIds) == false) {
    mdebug<<"   IsCentral: Central IDs not in sequence"<<endl;
    return false;
  }
  if (AreIdsInSequence(StopIds) == false) {
    mdebug<<"   IsCentral: Stop IDs not in sequence"<<endl;
    return false;
  }
  // Test for holes:
  // The hit after the last one from start is the first from central:
  double TimeLastFirst = m_SiEvent->GetHTAt(StartIds.back()-IdOffset)->GetTime();
  double TimeFirstLast = m_SiEvent->GetHTAt(CentralIds.front()-IdOffset)->GetTime();
  for (unsigned int h = 0; h < m_SiEvent->GetNHTs(); ++h) {
    if (m_SiEvent->GetHTAt(h)->GetTime() > TimeLastFirst && 
        m_SiEvent->GetHTAt(h)->GetTime() < TimeFirstLast) {
      mdebug<<"   IsCentral: Hit between first and central according to time!"<<endl;
      return false;      
    }
  } 
  TimeLastFirst = m_SiEvent->GetHTAt(CentralIds.back()-IdOffset)->GetTime();
  TimeFirstLast = m_SiEvent->GetHTAt(StopIds.front()-IdOffset)->GetTime();
  for (unsigned int h = 0; h < m_SiEvent->GetNHTs(); ++h) {
    if (m_SiEvent->GetHTAt(h)->GetTime() > TimeLastFirst && 
        m_SiEvent->GetHTAt(h)->GetTime() < TimeFirstLast) {
      mdebug<<"   IsCentral: Hit between first and central according to time!"<<endl;
      return false;      
    }
  } 


  // Test (2)
  int CommonOrigin = -1;
  for (unsigned int i = 0; i < StartIds.size(); ++i) {
    for (unsigned int j = 0; j < CentralIds.size(); ++j) {
      for (unsigned int k = 0; k < StopIds.size(); ++k) {
        for (unsigned int oi = 0; oi < m_SiEvent->GetHTAt(StartIds[i]-IdOffset)->GetNOrigins(); ++oi) {
          for (unsigned int oj = 0; oj < m_SiEvent->GetHTAt(CentralIds[j]-IdOffset)->GetNOrigins(); ++oj) {
            for (unsigned int ok = 0; ok < m_SiEvent->GetHTAt(StopIds[k]-IdOffset)->GetNOrigins(); ++ok) {
              if (m_SiEvent->GetHTAt(StartIds[i]-IdOffset)->GetOriginAt(oi) == 
                  m_SiEvent->GetHTAt(CentralIds[j]-IdOffset)->GetOriginAt(oj) && 
                  m_SiEvent->GetHTAt(StartIds[i]-IdOffset)->GetOriginAt(oi) == 
                  m_SiEvent->GetHTAt(StopIds[k]-IdOffset)->GetOriginAt(ok)) {
                CommonOrigin = m_SiEvent->GetHTAt(CentralIds[j]-IdOffset)->GetOriginAt(oj);
                break;
              }
            }
          }
        }
      }
    }
  }
  if (CommonOrigin == -1) {
    mdebug<<"   IsCentral: No common origin!"<<endl;
    return false;
  }

  // (3) Timing:
  if (m_SiEvent->GetHTAt(StartIds[0]-IdOffset)->GetTime() > 
      m_SiEvent->GetHTAt(CentralIds[0]-IdOffset)->GetTime() ||
      m_SiEvent->GetHTAt(CentralIds[0]-IdOffset)->GetTime() >
      m_SiEvent->GetHTAt(StopIds[0]-IdOffset)->GetTime()) {
    mdebug<<"   IsCentral: Timing wrong"<<endl;
    return false;
  }


  if (IsTrackCompletelyAbsorbed(CentralIds, Energy) == false) {
    mdebug<<"   IsCentral: Not completely absorbed"<<endl;
    return false;
  }

  return true;
}


/******************************************************************************
 * Return true if the given Ids are in sequence without holes
 */
bool ResponseGenerator::AreIdsInSequence(const vector<int>& Ids)
{
  const int IdOffset = 2;

//   for (unsigned int i = 0; i < Ids.size()-1; ++i) {
//     if (Ids[i+1] - Ids[i] != 1) return false;
//   }

  vector<int> Origins;
  for (unsigned int i = 0; i < Ids.size(); ++i) {
    for (unsigned int h = 0; h < m_SiEvent->GetHTAt(Ids[i]-IdOffset)->GetNOrigins(); ++h) {
      bool Contained = false;
      for (unsigned int o = 0; o < Origins.size(); ++o) {
        if (Origins[o] == m_SiEvent->GetHTAt(Ids[i]-IdOffset)->GetOriginAt(h)) {
          Contained = true;
          break;
        }
      }
      if (Contained == false) {
        Origins.push_back(m_SiEvent->GetHTAt(Ids[i]-IdOffset)->GetOriginAt(h));
      }
    }
  }

  double MinTime = numeric_limits<double>::max();
  double MaxTime = -numeric_limits<double>::max();
  for (unsigned int i = 0; i < Ids.size(); ++i) {
    if (m_SiEvent->GetHTAt(Ids[i]-IdOffset)->GetTime() > MaxTime) {
      MaxTime = m_SiEvent->GetHTAt(Ids[i]-IdOffset)->GetTime();
    }
    if (m_SiEvent->GetHTAt(Ids[i]-IdOffset)->GetTime() < MinTime) {
      MinTime = m_SiEvent->GetHTAt(Ids[i]-IdOffset)->GetTime();
    }  
  }

  // No hit with a listed origin is allowed to be between min and max time:
  for (unsigned int h = 0; h < m_SiEvent->GetNHTs(); ++h) {
    if (m_SiEvent->GetHTAt(h)->GetTime() > MinTime &&
        m_SiEvent->GetHTAt(h)->GetTime() < MaxTime) {
      for (unsigned int o = 0; o < Origins.size(); ++o) {
        if (m_SiEvent->GetHTAt(h)->IsOrigin(Origins[o]) == true) {
          bool Found = false;
          for (unsigned int i = 0; i < Ids.size(); ++i) {
            if (int(h) == Ids[i]-IdOffset) {
              Found = true;
              break;
            }
          }
          if (Found == false) {
            return false;
          }
        }
      }
    }
  }

  return true;
}


/******************************************************************************
 * Return true if the given RESEs are in sequence
 */
vector<int> ResponseGenerator::GetReseIds(MRESE* Start)
{ 
  massert(Start->GetType() == MRESE::c_Track || 
          Start->GetType() == MRESE::c_Cluster || 
          Start->GetType() == MRESE::c_Hit);

  map<MRESE*, vector<int> >::iterator Iter = m_Ids.find(Start);

  if (Iter != m_Ids.end()) {
    return (*Iter).second;
  } else {
    vector<int> Ids;
    MRESE* RESE = 0;
    MRESE* SubRESE = 0;

    if (Start->GetType() == MRESE::c_Track) {
      int r_max = Start->GetNRESEs();
      for (int r = 0; r < r_max; ++r) {
        RESE = Start->GetRESEAt(r);
        massert(RESE->GetType() == MRESE::c_Hit || RESE->GetType() == MRESE::c_Cluster);
        if (RESE->GetType() == MRESE::c_Cluster) {
          for (int c = 0; c < RESE->GetNRESEs(); ++c) {
            SubRESE = RESE->GetRESEAt(c);
            massert(SubRESE->GetType() == MRESE::c_Hit);
            Ids.push_back(SubRESE->GetID());
          }
        } else {
          Ids.push_back(RESE->GetID());
        }
      }
    } else if (Start->GetType() == MRESE::c_Cluster) {
      int r_max = Start->GetNRESEs();
      for (int r = 0; r < r_max; ++r) {
        RESE = Start->GetRESEAt(r);
        massert(RESE->GetType() == MRESE::c_Hit);
        Ids.push_back(RESE->GetID());
      }
    } else {
      Ids.push_back(Start->GetID());
    }
    
    sort(Ids.begin(), Ids.end());
 
    m_Ids[Start] = Ids;

    return Ids;
  }
}


/******************************************************************************
 * Do whatever analysis is necessary
 */
bool ResponseGenerator::GenerateComptonPdf()
{
  //cout<<"Trying to generate tripple Compton Pdf..."<<endl;

  vector<float> Axis1;
  vector<float> Axis2;
  vector<float> Axis3;
  vector<float> Axis4;

  vector<float> LAxis;
  for (int i = 2; i <= m_CSRMaxLength+1; ++i) {
    LAxis.push_back(i);
  }

  vector<float> MAxis; // Material: 0: unknown, 1: Si, 2: Ge, 3: Xe, 4: CsI
  MAxis = CreateEquiDist(-0.5, 4.5, 5);


  // Global good/bad:
  Axis1 = CreateEquiDist(0, 2, 2);
  MResponseMatrixO2 GoodBadTable("GoodBadTable", Axis1, LAxis); 
  GoodBadTable.SetAxisNames("GoodBad", "N");

  Axis3.clear();
  Axis3 = Axis2 = CreateEquiDist(0, 180, 20, -1, 181);


  // Start point:
  MResponseMatrixO4 PdfStartGood("MC - Start - Good", 
                                 CreateLogDist(100, 5000, 18, 1, 20000),
                                 Axis3, 
                                 LAxis, MAxis);
  PdfStartGood.SetAxisNames("E_{tot} [keV]", "cos#varphi", "N", "Material (0: ?, 1: Si, 2: Ge, 3: Xe, 4: CsI)");
  MResponseMatrixO4 PdfStartBad("MC - Start - Bad", 
                                CreateLogDist(100, 5000, 18, 1, 20000), 
                                Axis3, 
                                LAxis, MAxis);
  PdfStartBad.SetAxisNames("E_{tot} [keV]", "cos#varphi", "N", "Material (0: ?, 1: Si, 2: Ge, 3: Xe, 4: CsI)");


  // Track:
  MResponseMatrixO6 PdfTrackGood("MC - Track - Good", 
                                 CreateEquiDist(0, 180, 36, c_NoBound, 181),
                                 CreateEquiDist(0, 180, 1, c_NoBound, 181),
                                 CreateEquiDist(0, 1000000, 1),
                                 CreateLogDist(500, 10000, 10, 0, 100000, 0, false),
                                 LAxis, MAxis);
  PdfTrackGood.SetAxisNames("#Delta #alpha [deg]", "#alpha_{G} [deg]", "d [cm]", "E_{e}", 
                            "N", "Material (0: ?, 1: Si, 2: Ge, 3: Xe, 4: CsI)");
  MResponseMatrixO6 PdfTrackBad("MC - Track - Bad", 
                                CreateEquiDist(0, 180, 36, c_NoBound, 181),
                                CreateEquiDist(0, 180, 1, c_NoBound, 181),
                                CreateEquiDist(0, 1000000, 1),
                                CreateLogDist(500, 10000, 10, 0, 100000, 0, false),
                                LAxis, MAxis);
  PdfTrackBad.SetAxisNames("#Delta #alpha [deg]", "#alpha_{G} [deg]", "d [cm]", "E_{e}", 
                           "N", "Material (0: ?, 1: Si, 2: Ge, 3: Xe, 4: CsI)");


  
  // Distance:
  Axis1 = CreateEquiDist(0, 0.99, 39);
  Axis1.push_back(1.01);
  Axis2 = CreateLogDist(15, 5000, 38, 1, 20000);
  
  MResponseMatrixO4 PdfDistanceGood("MC - Compton distance - good", 
                                    Axis1,
                                    Axis2, 
                                    LAxis, MAxis);
  PdfDistanceGood.SetAxisNames("p_{A}", "E_{tot} [keV]", "N", "Material (0: ?, 1: Si, 2: Ge, 3: Xe, 4: CsI)");
  MResponseMatrixO4 PdfDistanceBad("MC - Compton distance - bad", 
                                   Axis1, 
                                   Axis2, 
                                   LAxis, MAxis);
  PdfDistanceBad.SetAxisNames("p_{A}", "E_{tot} [keV]", "N", "Material (0: ?, 1: Si, 2: Ge, 3: Xe, 4: CsI)");


  // Lastdistance:
  Axis1 = CreateEquiDist(0, 0.99, 39);
  Axis1.push_back(1.01);
  Axis2 = CreateLogDist(25, 5000, 38, 1, 20000);
 
  MResponseMatrixO4 PdfLastdistanceGood("MC - Photo distance - good", 
                                        Axis1,
                                        Axis2, 
                                        LAxis, MAxis);
  PdfLastdistanceGood.SetAxisNames("p_{A}", "E_{tot} [keV]", "N", "Material (0: ?, 1: Si, 2: Ge, 3: Xe, 4: CsI)");
  MResponseMatrixO4 PdfLastdistanceBad("MC - Photo distance - bad", 
                                       Axis1, 
                                       Axis2, 
                                       LAxis, MAxis);
  PdfLastdistanceBad.SetAxisNames("p_{A}", "E_{tot} [keV]", "N", "Material (0: ?, 1: Si, 2: Ge, 3: Xe, 4: CsI)");


  // CentralCompton:

  // cos Angles
  // Axis1 = CreateLogDist(1E-3, TMath::Pi(), 20, 0);
  // Axis2 = CreateEquiDist(-1, 1, 10, -2, 2);
  
  Axis1 = CreateLogDist(0.1, 180, 18, 0.001, 181, 0, false);
  Axis2 = CreateEquiDist(0, 180, 10, -1, 181);

  Axis3.clear();
  Axis3 = CreateLogDist(0.1, 10, 7, 0.01, 100, 0, false); 

  Axis4.clear();
  Axis4 = CreateLogDist(100, 1500, 4, 1, 10000, 0, false);


  MResponseMatrixO6 PdfComptonGood("MC - Central - good", 
                                   Axis1,
                                   Axis2, 
                                   Axis3,
                                   Axis4,
                                   LAxis, MAxis);
  PdfComptonGood.SetAxisNames("dcos#varphi", "cos#varphi_{E}", "d_{E} [cm]", 
                              "E_{tot} [keV]", "N", "Material (0: ?, 1: Si, 2: Ge, 3: Xe, 4: CsI)");
  MResponseMatrixO6 PdfComptonBad("MC - Central - bad", 
                                  Axis1, 
                                  Axis2,
                                  Axis3,
                                  Axis4,
                                  LAxis, MAxis);
  PdfComptonBad.SetAxisNames("dcos#varphi", "cos#varphi_{E}", "d_{E} [cm]", 
                             "E_{tot} [keV]", "N", "Material (0: ?, 1: Si, 2: Ge, 3: Xe, 4: CsI)");

  // End point:
  MResponseMatrixO3 PdfEndGood("MC - Stop - good", 
                               CreateLogDist(20, 5000, 100, 1, 20000), LAxis, MAxis);
  PdfEndGood.SetAxisNames("E_{tot} [keV]", "N", "Material (0: ?, 1: Si, 2: Ge, 3: Xe, 4: CsI)");
  MResponseMatrixO3 PdfEndBad("MC - Stop - bad", 
                              CreateLogDist(20, 5000, 100, 1, 20000), LAxis, MAxis);
  PdfEndBad.SetAxisNames("E_{tot} [keV]", "N", "Material (0: ?, 1: Si, 2: Ge, 3: Xe, 4: CsI)");


  // Go ahead event by event and compare the results: 
  MRESE* RESE = 0;
  MRERawEvent* RE = 0;
  MRawEventList* REList = 0;
  double Etot = 0;
  double Eres = 0;

  MTimer Total;
  Total.Start();
  double TFirstLoop = 0;
  MTimer FirstLoop;
  double TSecondLoop = 0;
  MTimer SecondLoop;
  double TLoadLoop = 0;
  MTimer LoadLoop;

  int Counter = 0;
  do {  
    LoadLoop.Start();
    if (LoadEvents() == false) break;

    TLoadLoop += LoadLoop.ElapsedTime();

    RESE = 0;
    RE = 0;
    REList = m_ReReader->GetRawEventList();

    SecondLoop.Start();

    //g_DebugLevel = 1;


    int r_max = REList->GetNRawEvents();
    for (int r = 0; r < r_max; ++r) {
      RE = REList->GetRawEventAt(r);
      
      if (RE->GetNRESEs() <= 1) {
        mdebug<<"GeneratePdf: Not enough hits!"<<endl;
        continue;
      }

      mdebug<<RE->ToString()<<endl;


      if (RE->GetStartPoint() == 0) continue;

      // Check if complete sequence is ok:
      bool SequenceOk = true;
      int SequenceLength = RE->GetNRESEs();
      int PreviousPosition = 0;

      // Look at start:
      MRESEIterator Iter;
      Iter.Start(RE->GetStartPoint());
      Iter.GetNextRESE();
      Etot = RE->GetEnergy();
      Eres = RE->GetEnergyResolution();
      if (IsComptonStart(*Iter.GetCurrent(), Etot, Eres) == true) {
        mdebug<<"--------> Found GOOD Compton start!"<<endl;
        PdfStartGood.Add(Etot, CalculatePhiEInDegree(*Iter.GetCurrent(), Etot), SequenceLength, GetMaterial(*Iter.GetCurrent()));
      } else {
        mdebug<<"--------> Found bad Compton start!"<<endl;
        PdfStartBad.Add(Etot, CalculatePhiEInDegree(*Iter.GetCurrent(), Etot), SequenceLength, GetMaterial(*Iter.GetCurrent()));
        SequenceOk = false;
      }

      // Track at start?
      if (Iter.GetCurrent()->GetType() == MRESE::c_Track) {
        double dAlpha = CalculateDAlphaInDegree(*((MRETrack*) Iter.GetCurrent()), *Iter.GetNext(), Etot);
        double Alpha = CalculateAlphaGInDegree(*((MRETrack*) Iter.GetCurrent()), *Iter.GetNext(), Etot);
//         cout<<"Phi: "<<CalculatePhiEInDegree(*Iter.GetCurrent(), Etot)<<endl;
        if (IsComptonTrack(*Iter.GetCurrent(), *Iter.GetNext(), PreviousPosition, Etot, Eres) == true) {
          mdebug<<"--------> Found GOOD Track start!"<<endl;
          PdfTrackGood.Add(dAlpha, Alpha, 1, Iter.GetCurrent()->GetEnergy(), SequenceLength, GetMaterial(*Iter.GetCurrent()));
//             cout<<"Filling: "<<dAlpha<<"!"<<1<<"!"<<1<<"!"<<Iter.GetCurrent()->GetEnergy()
//                 <<"!"<<SequenceLength<<"!"<<GetMaterial(*Iter.GetCurrent())<<endl;
         } else {
          mdebug<<"--------> Found bad Track start!"<<endl;
          PdfTrackBad.Add(dAlpha, Alpha, 1, Iter.GetCurrent()->GetEnergy(), SequenceLength, GetMaterial(*Iter.GetCurrent()));
          SequenceOk = false;
//             cout<<"Filling: "<<dAlpha<<"!"<<1<<"!"<<1<<"!"<<Iter.GetCurrent()->GetEnergy()
//                 <<"!"<<SequenceLength<<"!"<<GetMaterial(*Iter.GetCurrent())<<endl;
         }
      }


      // Central part of the Compton track
      Iter.GetNextRESE();
      while (Iter.GetNext() != 0) {
        // Add here
        Etot -= Iter.GetPrevious()->GetEnergy();
        Eres = sqrt(Eres*Eres - Iter.GetPrevious()->GetEnergyResolution()*Iter.GetPrevious()->GetEnergyResolution());
        PreviousPosition++;

        // In the current implementation/simulation the hits have to be in increasing order...
        if (m_NoAbsorptions == false && SequenceLength <= m_MaxAbsorptions) {
          double ComptonDistance = 
            CalculateComptonDistance(*Iter.GetPrevious(), *Iter.GetCurrent(), Etot);
          mdebug<<"Dist C: "<<ComptonDistance<<": real:"<<(Iter.GetCurrent()->GetPosition() - Iter.GetPrevious()->GetPosition()).Mag()<<endl;
          if (IsComptonSequence(*Iter.GetPrevious(), *Iter.GetCurrent(), PreviousPosition, Etot, Eres) == true) {
            mdebug<<"--------> Found GOOD Distance sequence!"<<endl;
            // Retrieve the data:
            PdfDistanceGood.Add(ComptonDistance, Etot, SequenceLength, GetMaterial(*Iter.GetCurrent()));
          } else {
            mdebug<<"--------> Found bad Distance sequence!"<<endl;
            // Retrieve the data:
            PdfDistanceBad.Add(ComptonDistance, Etot, SequenceLength, GetMaterial(*Iter.GetCurrent()));
              SequenceOk = false;
          } // Add good / bad
        }
        
        // Decide if it is good or bad...
        // In the current implementation/simulation the hits have to be in increasing order...
        double dPhi = CalculateDPhiInDegree(*Iter.GetPrevious(), *Iter.GetCurrent(), *Iter.GetNext(), Etot);
        double PhiE = CalculatePhiEInDegree(*Iter.GetCurrent(), Etot);
        double Lever = CalculateMinLeverArm(*Iter.GetPrevious(), *Iter.GetCurrent(), *Iter.GetNext());
        int Material = GetMaterial(*Iter.GetCurrent());

//         mout<<"Real good "<<m_SiEvent->GetEventNumber()<<": "
//             <<dPhi<<", "
//             <<PhiE<< ", "
//             <<Lever<< ", "
//             <<Etot<< ", "<<SequenceLength<<endl;
          
        if (IsComptonSequence(*Iter.GetPrevious(), *Iter.GetCurrent(), *Iter.GetNext(), PreviousPosition, Etot, Eres) == true) {
          mdebug<<"--------> Found GOOD Compton sequence!"<<endl;
          // Retrieve the data:
          PdfComptonGood.Add(dPhi, PhiE, Lever, Etot, SequenceLength, Material);

//           if (SequenceLength == 3) {
//             mout<<"Good event: "<<m_SiEvent->GetEventNumber()<<": "
//                 <<dPhi<<", "
//                 <<PhiE<< ", "
//                 <<Lever<< ", "
//                 <<Etot<< ", "<<SequenceLength<<endl;
//           }

//           if (dPhi > 0.05 && dPhi < 0.1 &&
//               PhiE > -1 && PhiE < -0.5 &&
//               Lever > 0 && Lever < 2 &&
//               Etot > 200 && Etot < 500 &&
//               SequenceLength == 3) {
//             mout<<"Real good "<<m_SiEvent->GetEventNumber()<<": "
//                 <<dPhi<<", "
//                 <<PhiE<< ", "
//                 <<Lever<< ", "
//                 <<Etot<< ", "<<SequenceLength<<endl;
//           }

        } else {
          mdebug<<"--------> Found bad Compton sequence!"<<endl;
          // Retrieve the data:
          PdfComptonBad.Add(dPhi, PhiE, Lever, Etot, SequenceLength, Material);
          SequenceOk = false;

//           if (dPhi < 0.001 &&
//               PhiE > -0.8 &&
//               SequenceLength == 5) {
//             mout<<"Real good "<<m_SiEvent->GetEventNumber()<<": "
//                 <<dPhi<<", "
//                 <<PhiE<< ", "
//                 <<Lever<< ", "
//                 <<Etot<< ", "<<SequenceLength<<endl;
//           }

//           if (SequenceLength == 3) {
//             cerr<<"Good Bad event: "<<m_SiEvent->GetEventNumber()<<": "
//                 <<dPhi<<", "
//                 <<PhiE<< ", "
//                 <<Lever<< ", "
//                 <<Etot<< ", "<<SequenceLength<<endl;
//           }

        } // Add good / bad

        // Track at central?
        if (Iter.GetCurrent()->GetType() == MRESE::c_Track) {
          //MRETrack* T = (MRETrack*) Iter.GetCurrent();
          double dAlpha = CalculateDAlphaInDegree(*((MRETrack*) Iter.GetCurrent()), *Iter.GetNext(), Etot);
          double Alpha = CalculateAlphaGInDegree(*((MRETrack*) Iter.GetCurrent()), *Iter.GetNext(), Etot);
          if (IsComptonTrack(*Iter.GetCurrent(), *Iter.GetNext(), PreviousPosition, Etot, Eres) == true) {
            mdebug<<"--------> Found GOOD Track start (central)!"<<endl;
            PdfTrackGood.Add(dAlpha, Alpha, 1, Iter.GetCurrent()->GetEnergy(), SequenceLength, GetMaterial(*Iter.GetCurrent()));
//             cout<<"Filling: "<<dAlpha<<"!"<<1<<"!"<<1<<"!"<<Iter.GetCurrent()->GetEnergy()
//                 <<"!"<<SequenceLength<<"!"<<GetMaterial(*Iter.GetCurrent())<<endl;
          } else {
            mdebug<<"--------> Found bad Track start (central)!"<<endl;
            PdfTrackBad.Add(dAlpha, Alpha, 1, Iter.GetCurrent()->GetEnergy(), SequenceLength, GetMaterial(*Iter.GetCurrent()));
//             cout<<"Filling: "<<dAlpha<<"!"<<1<<"!"<<1<<"!"<<Iter.GetCurrent()->GetEnergy()
//                 <<"!"<<SequenceLength<<"!"<<GetMaterial(*Iter.GetCurrent())<<endl;
            SequenceOk = false;
          }
        }
        Iter.GetNextRESE();
      }

      Etot -= Iter.GetPrevious()->GetEnergy();
      Eres = sqrt(Eres*Eres - Iter.GetPrevious()->GetEnergyResolution()*Iter.GetPrevious()->GetEnergyResolution());
      PreviousPosition++;

      // Decide if it is good or bad...
      // In the current implementation/simulation the hits have to be in increasing order...
      if (m_NoAbsorptions == false && SequenceLength <= m_MaxAbsorptions) {
        double LastDistance = CalculatePhotoDistance(*Iter.GetPrevious(), *Iter.GetCurrent(), Etot);
        mdebug<<"Dist P: "<<LastDistance<<": real:"<<(Iter.GetCurrent()->GetPosition() - Iter.GetPrevious()->GetPosition()).Mag()<<endl;
        if (IsComptonSequence(*Iter.GetPrevious(), *Iter.GetCurrent(), PreviousPosition, Etot, Eres) == true) {
          mdebug<<"--------> Found GOOD Lastdistance sequence!"<<endl;
          // Retrieve the data:
          PdfLastdistanceGood.Add(CalculatePhotoDistance(*Iter.GetPrevious(), *Iter.GetCurrent(), Etot), 
                                  Etot, SequenceLength, GetMaterial(*Iter.GetCurrent()));
        } else {
          mdebug<<"--------> Found bad Lastdistance sequence!"<<endl;
          // Retrieve the data:
          PdfLastdistanceBad.Add(CalculatePhotoDistance(*Iter.GetPrevious(), *Iter.GetCurrent(), Etot), 
                                 Etot, SequenceLength, GetMaterial(*Iter.GetCurrent()));
          SequenceOk = false;
        } // Add good / bad
      }

      // Look at end:
      if (IsComptonEnd(*Iter.GetCurrent()) == true) {
        mdebug<<"--------> Found GOOD Compton end!"<<endl;
        PdfEndGood.Add(Etot, SequenceLength, GetMaterial(*Iter.GetCurrent()));
        mdebug<<"Etot: "<<Etot<<endl;
      } else {
        mdebug<<"--------> Found bad Compton end!"<<endl;
        PdfEndBad.Add(Etot, SequenceLength, GetMaterial(*Iter.GetCurrent()));
        mdebug<<"Etot: "<<Etot<<endl;
        SequenceOk = false;
      }

      if (SequenceOk == false) {
        GoodBadTable.Add(0.5, SequenceLength, 1);
        //mdebug<<"No good sequence exists"<<endl<<endl<<endl<<endl;
      } else {
        GoodBadTable.Add(1.5, SequenceLength, 1);
        mdebug<<"One good sequence exists"<<endl<<endl<<endl<<endl;
      }
    } // For each raw event...

    TSecondLoop += SecondLoop.ElapsedTime();

    if (++Counter % m_SaveAfter == 0) {
      GoodBadTable.Write((m_ResponseName + ".tc.goodbad.rsp").c_str(), true);
  
      PdfStartGood.Write((m_ResponseName + ".tc.start.good.rsp").c_str(), true);
      PdfStartBad.Write((m_ResponseName + ".tc.start.bad.rsp").c_str(), true);
      
      PdfTrackGood.Write((m_ResponseName + ".tc.track.good.rsp").c_str(), true);
      PdfTrackBad.Write((m_ResponseName + ".tc.track.bad.rsp").c_str(), true);
  
      PdfComptonGood.Write((m_ResponseName + ".tc.compton.good.rsp").c_str(), true);
      PdfComptonBad.Write((m_ResponseName + ".tc.compton.bad.rsp").c_str(), true);
  
      PdfDistanceGood.Write((m_ResponseName + ".tc.comptondistance.good.rsp").c_str(), true);
      PdfDistanceBad.Write((m_ResponseName + ".tc.comptondistance.bad.rsp").c_str(), true);
  
      PdfLastdistanceGood.Write((m_ResponseName + ".tc.photodistance.good.rsp").c_str(), true);
      PdfLastdistanceBad.Write((m_ResponseName + ".tc.photodistance.bad.rsp").c_str(), true);

      PdfEndGood.Write((m_ResponseName + ".tc.stop.good.rsp").c_str(), true);
      PdfEndBad.Write((m_ResponseName + ".tc.stop.bad.rsp").c_str(), true);
    }

    if (m_Interrupt == true) break;

    //g_DebugLevel = 0;
  } while (true);

  cerr<<"Some timings:"<<endl;
  cerr<<"TTotal:      "<<Total.ElapsedTime()<<endl;  
  cerr<<"TLoadLoop:   "<<TLoadLoop<<endl;
  cerr<<"TFirstLoop:  "<<TFirstLoop<<endl;
  cerr<<"TSecondLoop: "<<TSecondLoop<<endl;


  GoodBadTable.Write((m_ResponseName + ".tc.goodbad.rsp").c_str(), true);
  
  PdfStartGood.Write((m_ResponseName + ".tc.start.good.rsp").c_str(), true);
  PdfStartBad.Write((m_ResponseName + ".tc.start.bad.rsp").c_str(), true);
  
  PdfTrackGood.Write((m_ResponseName + ".tc.track.good.rsp").c_str(), true);
  PdfTrackBad.Write((m_ResponseName + ".tc.track.bad.rsp").c_str(), true);
  
  PdfComptonGood.Write((m_ResponseName + ".tc.compton.good.rsp").c_str(), true);
  PdfComptonBad.Write((m_ResponseName + ".tc.compton.bad.rsp").c_str(), true);
  
  PdfDistanceGood.Write((m_ResponseName + ".tc.comptondistance.good.rsp").c_str(), true);
  PdfDistanceBad.Write((m_ResponseName + ".tc.comptondistance.bad.rsp").c_str(), true);
  
  PdfLastdistanceGood.Write((m_ResponseName + ".tc.photodistance.good.rsp").c_str(), true);
  PdfLastdistanceBad.Write((m_ResponseName + ".tc.photodistance.bad.rsp").c_str(), true);

  PdfEndGood.Write((m_ResponseName + ".tc.stop.good.rsp").c_str(), true);
  PdfEndBad.Write((m_ResponseName + ".tc.stop.bad.rsp").c_str(), true);

  return true;
}


/******************************************************************************
 * Return the material ID
 */
int ResponseGenerator::GetMaterial(MRESE& Hit)
{
  return MERCSRBayesian::GetMaterial(&Hit);
}


/******************************************************************************
 * Create axis with bins in logaritmic distance
 */
vector<float> ResponseGenerator::CreateLogDist(float Min, float Max, int Bins, 
                                               float MinBound, float MaxBound,
                                               float Offset, bool Inverted)
{
  vector<float> Axis;

  if (MinBound != c_NoBound) {
    Axis.push_back(MinBound);
  }

  Min = log(Min);
  Max = log(Max);
  float Dist = (Max-Min)/(Bins);

  for (int i = 0; i < Bins+1; ++i) {
    Axis.push_back(exp(Min+i*Dist));
  }

  if (MaxBound != c_NoBound) {
    Axis.push_back(MaxBound);
  }

  if (Inverted == true) {
    vector<float> Temp = Axis;
    for (unsigned int i = 1; i < Temp.size()-1; ++i) {
      Axis[i] = Axis[i-1] + (Temp[Temp.size()-i]-Temp[Temp.size()-i-1]);
    }
  }

  for (unsigned int i = 0; i < Axis.size(); ++i) {
    Axis[i] += Offset;
  }

  return Axis;
}


/******************************************************************************
 * Create axis with bins in logaritmic distance
 */
vector<float> ResponseGenerator::CreateEquiDist(float Min, float Max, int Bins, 
                                                float MinBound, float MaxBound,
                                                float Offset, bool Inverted)
{
  vector<float> Axis;

  if (MinBound != c_NoBound) {
    Axis.push_back(MinBound);
  }

  float Dist = (Max-Min)/(Bins);

  for (int i = 0; i < Bins+1; ++i) {
    Axis.push_back(Min+i*Dist);
  }

  if (MaxBound != c_NoBound) {
    Axis.push_back(MaxBound);
  }

  if (Inverted == true) {
    vector<float> Temp = Axis;
    for (unsigned int i = 1; i < Temp.size()-1; ++i) {
      Axis[i] = Axis[i-1] + (Temp[Temp.size()-i]-Temp[Temp.size()-i-1]);
    }
  }

  for (unsigned int i = 0; i < Axis.size(); ++i) {
    Axis[i] += Offset;
  }

  return Axis;
}


/******************************************************************************
 * Return true if the given RESEs are in sequence
 */
bool ResponseGenerator::IsComptonTrack(MRESE& Start, MRESE& Center, 
                                       int PreviousPosition, double Etot, double Eres)
{
  // A good start point of the track consists of the following:
  // (1) Start is a track
  // (2) Start is only one Compton interaction
  // (3) One of Centers Compton interactions directly follows the IA of start
  // (4) The first hit of start is the first hit of the track
  //     The second hit of start is the second hit of the track

  mdebug<<"IsComptonTrack: Looking at: "<<Start.GetID()<<"("<<Etot<<")"<<endl;

  // (1) 
  if (Start.GetType() != MRESE::c_Track) {
    mdebug<<"IsComptonTrack: No track!"<<endl;
    return false;    
  }

  // (2)
  if (IsSingleCompton(Start) == false) {
    mdebug<<"IsComptonTrack: Start has multiple Comptons!"<<endl;
    return false;        
  }

  // (3)
  if (IsComptonSequence(Start, Center, PreviousPosition, Etot-Start.GetEnergy(), Eres) == false) {
    // Attention: Eres is a little to large...
    mdebug<<"IsComptonTrack: No Compton sequence!"<<endl;
    return false;        
  }

  // (4)
  MRESEIterator Iter;
  MRETrack* Track = (MRETrack*) &Start;
  Iter.Start(Track->GetStartPoint());
  Iter.GetNextRESE();
  if (IsTrackStart(*Iter.GetCurrent(), *Iter.GetNext(), Track->GetEnergy()) == false) {
    mdebug<<"IsComptonTrack: Track is wrong!"<<endl;
    return false;    
  }
    
  return true;
}



/******************************************************************************
 * Return true if the given RESEs are in sequence
 */
bool ResponseGenerator::IsComptonStart(MRESE& Start, double Etot, double Eres)
{
  // A goor start point for tripple Comptons requires:
  // (1) An absorption better than 98%
  // (2) the IDs of their hits are in increasing order without holes
  // (3) We have one Compton interaction and some other interaction
  // (4) The origin is only the first Compton interaction

  mdebug<<"IsComptonStart: Looking at: "<<Start.GetID()<<endl;


  // Test (2)
  //vector<int> StartIds = GetReseIds(Start);
  vector<int> StartOriginIds = GetOriginIds(&Start);
  if (StartOriginIds.size() == 0) {
    mdebug<<"IsComptonStart: Start has no Sim IDs!"<<endl;
    return false;
  }
  
  // Absorption:
  if (Etot > 0) {
    if (IsAbsorbed(StartOriginIds, Start.GetEnergy(), Start.GetEnergyResolution()) == false) {
      mdebug<<"IsComptonStart: Central not completely absorbed!"<<endl;
      return false;
    }
  }
  
  //cout<<"Cont only Start!"<<endl;
  if (ContainsOnlyComptonDependants(StartOriginIds) == false) {
    mdebug<<"IsComptonStart: Start contains not only Compton dependants"<<endl;
    return false;
  }

  // Test (3)
  if (m_SiEvent->GetNIAs() < 3) { 
    mdebug<<"IsComptonStart: Not enough interactions!"<<endl;
    return false;    
  }
  if (m_SiEvent->GetIAAt(1)->GetType() != "COMP") {
    mdebug<<"IsComptonStart: Second interaction is no Compton!"<<endl;
    return false;    
  }
  
  // Test (4)
  int SmallestSimId = numeric_limits<int>::max();
  for (unsigned int i = 0; i < StartOriginIds.size(); ++i) {
    if (StartOriginIds[i] != 1) {
      if (SmallestSimId > StartOriginIds[i]) {
        SmallestSimId = StartOriginIds[i];
      }
    }
  }
  
  if (SmallestSimId != 2) {
    mdebug<<"IsComptonStart: Not correct start point: "<<SmallestSimId<<endl;
    return false;            
  }

  if (Etot > 0) {
    if (IsTotalAbsorbed(StartOriginIds, Etot, Eres) == false) {
      mdebug<<"Not completely absorbed!"<<endl;
      return false;
    }
  }

  return true;
}


/******************************************************************************
 * Return true if the given RESEs are in sequence
 */
bool ResponseGenerator::IsComptonSequence(MRESE& Start, MRESE& Central, 
                                          int StartPosition, double Etot, 
                                          double Eres)
{
  // A good start point for double Comptons requires:
  // (1) An absorption better than 99%
  // (2) the IDs of their hits are in increasing order without holes
  // (3) Start is the StartPosition's Compton IA

  mdebug<<"IsComptonSequence2: Looking at: "
        <<Start.GetID()<<" - "<<Central.GetID()<<endl;


  // Test (2)
  //vector<int> StartIds = GetReseIds(Start);
  vector<int> StartOriginIds = GetOriginIds(&Start);
  if (StartOriginIds.size() == 0) {
    mdebug<<"IsComptonSequence2: Start has no Sim IDs!"<<endl;
    return false;
  }
  
  //vector<int> CentralIds = GetReseIds(Central);
  vector<int> CentralOriginIds = GetOriginIds(&Central);
  if (CentralOriginIds.size() == 0) {
    mdebug<<"IsComptonSequence2: Central has no Sim IDs!"<<endl;
    return false;
  }
  

  if (ContainsOnlyComptonDependants(CentralOriginIds) == false) {
    mdebug<<"IsComptonSequence2: Central contains not only Compton dependants"<<endl;
    return false;
  }
  if (AreInComptonSequence(StartOriginIds, CentralOriginIds, StartPosition) == false) {
    mdebug<<"IsComptonSequence2: Not in Compton sequence!"<<endl;
    return false;
  }

  if (Etot > 0) {
    if (IsTotalAbsorbed(CentralOriginIds, Etot, Eres) == false) {
      mdebug<<"IsComptonSequence2: Not completely absorbed!"<<endl;
      return false;
    }
  }

  return true;
}


/******************************************************************************
 * Return true if the given RESEs are in sequence
 */
bool ResponseGenerator::IsComptonSequence(MRESE& Start, MRESE& Central, 
                                          MRESE& Stop, int StartPosition, 
                                          double Etot, double Eres)
{
  // A goor start point for tripple Comptons requires:
  // (1) An absorption better than 98%
  // (2) the IDs of their hits are in increasing order without holes

  mdebug<<"IsComptonSequence3: Looking at: "
        <<Start.GetID()<<" - "<<Central.GetID()<<" - "<<Stop.GetID()<<endl;


  // Test (2)
  //vector<int> StartIds = GetReseIds(Start);
  vector<int> StartOriginIds = GetOriginIds(&Start);
  if (StartOriginIds.size() == 0) {
    mdebug<<"IsComptonSequence3: Start has no Sim IDs!"<<endl;
    return false;
  }
  
//   // Absorption:
//   if (IsAbsorbed(StartOriginIds, Start.GetEnergy()) == false) {
//     mdebug<<"IsComptonSequence3: Start not completely absorbed!"<<endl;
//     return false;
//   }
  
  //vector<int> CentralIds = GetReseIds(Central);
  vector<int> CentralOriginIds = GetOriginIds(&Central);
  if (CentralOriginIds.size() == 0) {
    mdebug<<"IsComptonSequence3: Central has no Sim IDs!"<<endl;
    return false;
  }
  
  // Absorption:
  if (IsAbsorbed(CentralOriginIds, Central.GetEnergy(), Central.GetEnergyResolution()) == false) {
    mdebug<<"IsComptonSequence3: Central not completely absorbed!"<<endl;
    return false;
  }
  
  //vector<int> StopIds = GetReseIds(Stop);
  vector<int> StopOriginIds = GetOriginIds(&Stop);
  if (StopOriginIds.size() == 0) {
    mdebug<<"IsComptonSequence3: Stop has no Sim IDs!"<<endl;
    return false;
  }
  
//   // Absorption:
//   if (IsAbsorbed(StopOriginIds, Stop.GetEnergy()) == false) {
//     mdebug<<"IsComptonSequence3: Stop not completely absorbed!"<<endl;
//     return false;
//   }
  
  //cout<<"Cont only Start!"<<endl;
//   if (ContainsOnlyComptonDependants(StartOriginIds) == false) {
//     mdebug<<"IsComptonSequence3: Start contains not only Compton dependants"<<endl;
//     return false;
//   }
  //cout<<"Cont only Central!"<<endl;
  if (ContainsOnlyComptonDependants(CentralOriginIds) == false) {
    mdebug<<"IsComptonSequence3: Central contains not only Compton dependants"<<endl;
    return false;
  }
  if (ContainsOnlyComptonDependants(StopOriginIds) == false) {
    mdebug<<"IsComptonSequence3: Stop contains not only Compton dependants"<<endl;
    return false;
  }
  if (AreInComptonSequence(StartOriginIds, CentralOriginIds, StartPosition) == false) {
    mdebug<<"IsComptonSequence3: Not in Compton sequence!"<<endl;
    return false;
  }
  if (AreInComptonSequence(CentralOriginIds, StopOriginIds, 0) == false) {
    mdebug<<"IsComptonSequence3: Not in Compton sequence!"<<endl;
    return false;
  }

  if (IsTotalAbsorbed(CentralOriginIds, Etot, Eres) == false) {
    mdebug<<"IsComptonSequence3: Not completely absorbed!"<<endl;
    return false;
  }

  bool FoundBehind = false;
  for (unsigned int i = 0; i < StartOriginIds.size(); ++i) {
    if (StopOriginIds[0] - StartOriginIds[i] == 2) {
      FoundBehind = true;
    }
  }
  if (FoundBehind == false) {
    mdebug<<"IsComptonSequence3: More than one Compton between start and stop!"<<endl;
    return false;
  }

  return true;
}


/******************************************************************************
 * Return true if the given RESEs are in sequence
 */
bool ResponseGenerator::IsComptonEnd(MRESE& End)
{
  // A good start point for tripple Comptons requires:
  // (1) An absorption better than 98%
  // (2) the IDs of their hits are in increasing order without holes
  // (3) We have one Compton interaction and some other interaction
  // (4) The origin is only the first Compton interaction

  mdebug<<"IsComptonEnd: Looking at: "<<End.GetID()<<endl;



  // Test (2)
//   vector<int> CentralIds = GetReseIds(Central);
//   vector<int> CentralOriginIds = GetOriginIds(CentralIds);
//   if (CentralOriginIds.size() == 0) {
//     mdebug<<"IsComptonEnd: Central has no Sim IDs!"<<endl;
//     return false;
//   }
  
  //vector<int> EndIds = GetReseIds(End);
  vector<int> EndOriginIds = GetOriginIds(&End);
  if (EndOriginIds.size() == 0) {
    mdebug<<"IsComptonEnd: End has no Sim IDs!"<<endl;
    return false;
  }
  
  // Absorption:
  if (IsTotalAbsorbed(EndOriginIds, End.GetEnergy(), End.GetEnergyResolution()) == false) {
    mdebug<<"IsComptonEnd: Not completely absorbed!"<<endl;
    return false;
  }

//   if (ContainsOnlyComptonDependants(CentralOriginIds) == false) {
//     mdebug<<"IsComptonEnd: Central contains not only Compton dependants"<<endl;
//     return false;
//   }
  if (ContainsOnlyComptonDependants(EndOriginIds) == false) {
    mdebug<<"IsComptonEnd: End contains not only Compton dependants"<<endl;
    return false;
  }
//   if (AreInComptonSequence(CentralOriginIds, EndOriginIds) == false) {
//     mdebug<<"IsComptonEnd: Not in Compton sequence!"<<endl;
//     return false;
//   }
  
  // Test (3)
  if (m_SiEvent->GetNIAs() < 3) { 
    mdebug<<"IsComptonEnd: Not enough interactions!"<<endl;
    return false;    
  }
  
//   // Test (4)
//   // End needs to contain the last interaction of track 1 
//   int LastIA = 0;
//   for (unsigned int i = 1; i < m_SiEvent->GetNIAs(); ++i) {
//     if (m_SiEvent->GetIAAt(i)->GetOrigin() == 1) {
//       LastIA = i+1;
//     } else {     
//       break;
//     }
//   }
//   bool FoundLastIA = false;
//   for (unsigned int i = 0; i < EndOriginIds.size(); ++i) {
//     if (EndOriginIds[i] == LastIA) {
//       FoundLastIA = true;
//       break;
//     }
//   }
//   if (FoundLastIA == false) {
//     mdebug<<"IsComptonEnd: Last interaction of track 1 not part of last hit"<<endl;
//     return false;    
//   }

  // Motherparticle of smallest ID needs to be a photon:
  int Smallest = EndOriginIds[0];
  for (unsigned int i = 1; i < EndOriginIds.size(); ++i) {
    if (EndOriginIds[i] < Smallest) Smallest = EndOriginIds[i];
  }

  if (m_SiEvent->GetIAAt(Smallest-1)->GetOrigin() == 0) {
    if (m_SiEvent->GetIAAt(Smallest-1)->GetType() != "ANNI" &&
        m_SiEvent->GetIAAt(Smallest-1)->GetType() != "INIT") {
      return false;
    }
  } else {
    if (m_SiEvent->GetIAAt(m_SiEvent->GetIAAt(Smallest-1)->GetOrigin()-1)->GetType() != "ANNI" &&
        m_SiEvent->GetIAAt(m_SiEvent->GetIAAt(Smallest-1)->GetOrigin()-1)->GetType() != "INIT") {
      return false;
    }
  }

  return true;
}


/******************************************************************************
 *
 */
bool ResponseGenerator::IsAbsorbed(const vector<int>& AllSimIds, 
                                   double Energy, double EnergyResolution)
{
  massert(AllSimIds.size() > 0);

  EnergyResolution = 3*EnergyResolution+2;

  int MinId = numeric_limits<int>::max();
  //cout<<"Ids: ";
  for (unsigned int i = 0; i < AllSimIds.size(); ++i) {
    //cout<<AllSimIds[i]<<" ";
    if (MinId > AllSimIds[i] && AllSimIds[i] > 1) MinId = AllSimIds[i];
  }
  //cout<<endl;

  if (MinId == numeric_limits<int>::max()) {
    return false;
  }

  double Ideal;

  MSimIA* Top = 0;
  MSimIA* Bottom = 0;
  if (m_SiEvent->GetIAAt(MinId-2)->GetOrigin() == m_SiEvent->GetIAAt(MinId-1)->GetOrigin()) {
    Top = m_SiEvent->GetIAAt(MinId-2);
    Bottom = m_SiEvent->GetIAAt(MinId-1);
  } else {
    Top = m_SiEvent->GetIAAt(m_SiEvent->GetIAAt(MinId-1)->GetOrigin()-1);
    Bottom = m_SiEvent->GetIAAt(MinId-1);
  }

  if (m_SiEvent->GetIAAt(MinId-2)->GetType() == "COMP") {
    Ideal = Top->GetMotherEnergy()-Bottom->GetMotherEnergy();
  } else {
    Ideal = Top->GetEnergy()-Bottom->GetMotherEnergy();
  }

//   if (fabs(Energy - Ideal)/Ideal > m_MaxEnergyDifferencePercent &&
//       fabs(Energy - Ideal) > m_MaxEnergyDifference) {
//     mdebug<<"IsAbsorbed: Not completely absorbed: i="<<Ideal<<" r="<<Energy<<endl;
//     return false;
//   } else {
//     mdebug<<"Abs!: i="<<Ideal<<" r="<<Energy<<endl;
//   }
  if (fabs(Energy - Ideal) > EnergyResolution) {
    mdebug<<"IsAbsorbed: Not completely absorbed: i="<<Ideal<<"  r="<<Energy<<"  3*Eres+2="<<EnergyResolution<<endl;
    return false;
  } else {
    mdebug<<"Abs!: i="<<Ideal<<" r="<<Energy<<endl;
  }


  return true;
}


/******************************************************************************
 *
 */
bool ResponseGenerator::IsTotalAbsorbed(const vector<int>& AllSimIds, 
                                        double Energy, double EnergyResolution)
{
  massert(AllSimIds.size() > 0);

  EnergyResolution = 3*EnergyResolution+2;

  int MinId = numeric_limits<int>::max();
  for (unsigned int i = 0; i < AllSimIds.size(); ++i) {
    if (MinId > AllSimIds[i] && AllSimIds[i] > 1) MinId = AllSimIds[i];
  }

  if (MinId == numeric_limits<int>::max()) return false;

  double Ideal;
  MSimIA* Top = 0;
  if (m_SiEvent->GetIAAt(MinId-2)->GetOrigin() == m_SiEvent->GetIAAt(MinId-1)->GetOrigin()) {
    Top = m_SiEvent->GetIAAt(MinId-2);
  } else {
    Top = m_SiEvent->GetIAAt(m_SiEvent->GetIAAt(MinId-1)->GetOrigin()-1);
  }

  if (m_SiEvent->GetIAAt(MinId-2)->GetType() == "COMP") {
    Ideal = Top->GetMotherEnergy();
  } else {
    Ideal = Top->GetEnergy();
  }

  if (MinId-2 != 0) {
    Ideal = m_SiEvent->GetIAAt(MinId-2)->GetMotherEnergy();
  } else {
    Ideal = m_SiEvent->GetIAAt(MinId-2)->GetEnergy();
  }

  if (fabs(Ideal - Energy) > EnergyResolution) {
      mdebug<<"Is totally absorbed: Not completely absorbed: Tot abs: i="<<Ideal<<"  r="<<Energy<<"  3*Eres+2="<<EnergyResolution<<endl;
    return false;
  } else {
    mdebug<<"Is totally absorbed: Completely absorbed: Tot abs: i="<<Ideal<<"  r="<<Energy<<"  3*Eres+2="<<EnergyResolution<<endl;
  }
//   if (fabs((Ideal - Energy)/Ideal) > m_MaxEnergyDifferencePercent &&
//       fabs(Ideal - Energy) > m_MaxEnergyDifference ) {
//       mdebug<<"Is totally absorbed: Not completely absorbed: Tot abs: i="<<Ideal<<" r="<<Energy<<endl;
//     return false;
//   }

  return true;
}


/******************************************************************************
 *
 */
vector<int> ResponseGenerator::GetOriginIds(MRESE* RESE)
{
  // Extremely time critical function!

  const int IdOffset = 2;

  map<MRESE*, vector<int> >::iterator Iter = m_OriginIds.find(RESE);

  if (Iter != m_OriginIds.end()) {
    return (*Iter).second;
  } else {
    
    vector<int> Ids = GetReseIds(RESE);

    vector<int> OriginIds;
    OriginIds.reserve(10);

    // Generate sim IDs:
    for (vector<int>::iterator Iter = Ids.begin();
         Iter != Ids.end(); ++Iter) {
      MSimHT* HT = m_SiEvent->GetHTAt((*Iter)-IdOffset);
      for (unsigned int o = 0; o < HT->GetNOrigins(); ++o) {
        int Origin = int(HT->GetOriginAt(o));
        if (find(OriginIds.begin(), OriginIds.end(), Origin) == OriginIds.end()) { // not found
          if (Origin >= 1 && 
              m_SiEvent->GetIAAt(Origin-1)->GetType() != "INIT" && 
              m_SiEvent->GetIAAt(Origin-1)->GetType() != "ANNI") {
            OriginIds.push_back(Origin);
          }
        }
      }
    }

    sort(OriginIds.begin(), OriginIds.end());
    m_OriginIds[RESE] = OriginIds;

    return OriginIds;
  }
}


/******************************************************************************
 *
 */
bool ResponseGenerator::IsSingleCompton(MRESE& Start)
{
  // Check if all interaction in start are from one single Compton interaction:

  //cout<<Start.ToString()<<endl;

  //vector<int> StartIds = GetReseIds(Start);
  vector<int> StartOriginIds = GetOriginIds(&Start);
  if (StartOriginIds.size() == 0) {
    mdebug<<"IsSingleCompton: Start has no Sim IDs!"<<endl;
    return false;
  }

  int NPhotos = 0;
  int NComptons = 0;
  for (unsigned int i = 0; i < StartOriginIds.size(); ++i) {
    //cout<<"Id:"<<StartOriginIds[i]<<endl;
    if (m_SiEvent->GetIAAt(StartOriginIds[i]-1)->GetType() == "COMP") {
      NComptons++;
    } else if (m_SiEvent->GetIAAt(StartOriginIds[i]-1)->GetType() == "PHOT") {
      NPhotos++;
    }
  }

  if (NComptons == 1 && NPhotos >= 0) {
    mdebug<<"IsSingleCompton: Single Compton: C="<<NComptons<<" P="<<NPhotos<<endl;
    return true;
  } else if (NComptons == 0 && NPhotos >= 1) {
    mdebug<<"IsSingleCompton: No Compton but photo: C="<<NComptons<<" P="<<NPhotos<<endl;
    return true;
  } else {
    mdebug<<"IsSingleCompton: No single Compton or photo: C="<<NComptons<<" P="<<NPhotos<<endl;
  }

  return false;
}


/******************************************************************************
 *
 */
bool ResponseGenerator::AreInComptonSequence(const vector<int>& StartOriginIds, 
                                             const vector<int>& CentralOriginIds,
                                             int StartPosition)
{
    // They are in good sequence if 
  // (1) Both smallest IDs originate from 1
  // (2) The IDs are in sequence

  massert(StartOriginIds.size() > 0);
  massert(CentralOriginIds.size() > 0);
  
  if (m_SiEvent->GetIAAt(StartOriginIds[0]-1)->GetType() != "COMP") {
    mdebug<<"CS: No COMP"<<endl;
    return false;
  }

  if (m_SiEvent->GetIAAt(StartOriginIds[0]-1)->GetOrigin() != 
      m_SiEvent->GetIAAt(CentralOriginIds[0]-1)->GetOrigin()) {
    mdebug<<"CS: No common origin"<<endl;
    return false;
  }

//   if (m_SiEvent->GetIAAt(StartOriginIds[0]-1)->GetMotherParticleNumber() != 1) {
//     mdebug<<"CS: Mother is no photon: "<<m_SiEvent->GetIAAt(StartOriginIds[0]-1)->GetMotherParticleNumber()<<endl;
//     return false;
//   }

  bool FoundBehind = false;
  for (unsigned int i = 0; i < StartOriginIds.size(); ++i) {
    //cout<<"testing: "<<CentralOriginIds[0]<<"-"<<StartOriginIds[i]<<endl;
    if (CentralOriginIds[0] - StartOriginIds[i] == 1) {
      FoundBehind = true;
      break;
    }
  }
  if (FoundBehind == false) {
    mdebug<<"CS: Not behind each other!"<<endl;
    return false;
  }

  return true;
}


/******************************************************************************
 *
 */
bool ResponseGenerator::ContainsOnlyComptonDependants(vector<int> AllSimIds)
{
  massert(AllSimIds.size() > 0);

  int SmallestSimId = numeric_limits<int>::max();
  vector<int> GoodSimIds;

  // Generate sim IDs:
  //cout<<"All: ";
  for (unsigned int i = 0; i < AllSimIds.size(); ++i) {
    //cout<<AllSimIds[i]<<" ";
    if (SmallestSimId > AllSimIds[i]) {
      SmallestSimId = AllSimIds[i];
    }
  }
  //cout<<endl;

  vector<int>::iterator Iter;
  GoodSimIds.push_back(SmallestSimId);
  //cout<<"Adding smallest: "<<SmallestSimId<<endl;
  Iter = find(AllSimIds.begin(), AllSimIds.end(), SmallestSimId);
  AllSimIds.erase(Iter);

  bool MoreSmallest = false;
  do {
    MoreSmallest = false;
    
    for (unsigned int i = 0; i < AllSimIds.size(); ++i) {
      if (AllSimIds[i] == SmallestSimId+1 && 
          m_SiEvent->GetIAAt(AllSimIds[i]-1)->GetOrigin() == 
          m_SiEvent->GetIAAt(SmallestSimId-1)->GetOrigin()) {
        SmallestSimId = AllSimIds[i];
        MoreSmallest = true;
        GoodSimIds.push_back(SmallestSimId);
        //cout<<"Adding smallest: "<<SmallestSimId<<endl;
        Iter = find(AllSimIds.begin(), AllSimIds.end(), SmallestSimId);
        AllSimIds.erase(Iter);
        break;
      }
    }
    
  } while (MoreSmallest == true);


  // Check for dependents:
  bool DependantsFound = false;
  do {
    DependantsFound = false;
    for (unsigned int g = 0; g < GoodSimIds.size(); ++g) {
      for (unsigned int a = 0; a < AllSimIds.size(); ++a) {
        //cout<<"Testing all: "<<AllSimIds[a]<<endl;
        if (m_SiEvent->GetIAAt(AllSimIds[a]-1)->GetOrigin() == GoodSimIds[g]) {
          //cout<<"Found good: "<<AllSimIds[a]<<endl;
          GoodSimIds.push_back(AllSimIds[a]);
          Iter = find(AllSimIds.begin(), AllSimIds.end(), AllSimIds[a]);
          AllSimIds.erase(Iter);
          DependantsFound = true;
          break;
        }
      }
    }
  } while (DependantsFound == true);

  // If we have origins other than dependants:
  if (AllSimIds.size() > 0) {
    mdebug<<"ContainsOnlyComptonDependants: Hits other than dependants: ";
    for (unsigned int a = 0; a < AllSimIds.size(); ++a) {
      mdebug<<AllSimIds[a]<<" ";
    }
    mdebug<<endl;
    return false;
  }

  sort(GoodSimIds.begin(), GoodSimIds.end());

  // Check for dependants which are not contained: 
  for (unsigned int i = 0; i < m_SiEvent->GetNIAs(); ++i) {
    int Origin = m_SiEvent->GetIAAt(i)->GetOrigin();
    //cout<<"Checkin ID="<<i+1<<" with o= "<<Origin<<endl;
    bool OriginatesFromGood = false;
    bool ContainedInGood = false;   
    for (unsigned int g = 0; g < GoodSimIds.size(); ++g) {
      //cout<<"Verifying: "<<GoodSimIds[g]<<endl;
      if (int(i+1) == GoodSimIds[g]) {
        //cout<<"Contained in good!"<<endl;
        ContainedInGood = true;
      } 
      if (Origin == GoodSimIds[g]) {
        //cout<<"Originates from good!"<<endl;
        OriginatesFromGood = true;
      }
    }
    if (OriginatesFromGood == true && ContainedInGood == false) {
      mdebug<<"ContainsOnlyComptonDependants: Originates but not contained: "<<i+1<<endl;
      return false;
    }
  }

  return true;
}



/******************************************************************************
 *
 */
double ResponseGenerator::CalculateMinLeverArm(MRESE& Start, MRESE& Central, 
                                               MRESE& Stop)
{
  return MERCSRBayesian::CalculateMinLeverArm(Start.GetPosition(),
                                              Central.GetPosition(),
                                              Stop.GetPosition());
}


/******************************************************************************
 *
 */
double ResponseGenerator::CalculateDPhiInDegree(MRESE& Start, MRESE& Central, 
                                        MRESE& Stop, double Energy)
{
  return MERCSRBayesian::CalculateDPhiInDegree(&Start, &Central, &Stop, Energy);
}


/******************************************************************************
 *
 */
double ResponseGenerator::CalculateDAlpha(MRETrack& Track, MRESE& Central, double Energy)
{
  return MERCSRBayesian::CalculateDAlpha(&Track, &Central, Energy);
}


/******************************************************************************
 *
 */
double ResponseGenerator::CalculateDAlphaInDegree(MRETrack& Track, MRESE& Central, double Energy)
{
  return MERCSRBayesian::CalculateDAlphaInDegree(&Track, &Central, Energy);
}


/******************************************************************************
 *
 */
double ResponseGenerator::CalculateAlphaE(MRETrack& Track, MRESE& Central, double Energy)
{
  return MERCSRBayesian::CalculateAlphaE(&Track, &Central, Energy);
}


/******************************************************************************
 *
 */
double ResponseGenerator::CalculateAlphaEInDegree(MRETrack& Track, MRESE& Central, double Energy)
{
  return MERCSRBayesian::CalculateAlphaEInDegree(&Track, &Central, Energy);
}


/******************************************************************************
 *
 */
double ResponseGenerator::CalculateAlphaG(MRETrack& Track, MRESE& Central, double Energy)
{
  return MERCSRBayesian::CalculateAlphaG(&Track, &Central, Energy);
}


/******************************************************************************
 *
 */
double ResponseGenerator::CalculateAlphaGInDegree(MRETrack& Track, MRESE& Central, double Energy)
{
  return MERCSRBayesian::CalculateAlphaGInDegree(&Track, &Central, Energy);
}


/******************************************************************************
 *
 */
double ResponseGenerator::CalculatePhotoDistance(MRESE& Start, MRESE& Stop, 
                                                 double Etot)
{
  double Distance = 
    m_ReGeometry->GetPhotoAbsorptionProbability(Start.GetPosition(), 
                                                Stop.GetPosition(), Etot); 

  //cout<<"Distance:†"<<Distance<<" E: "<<Etot<<endl;
  //cout<<Start.ToString()<<endl;
  //cout<<Stop.ToString()<<endl;
  return Distance;
}


/******************************************************************************
 *
 */
double ResponseGenerator::CalculateComptonDistance(MRESE& Start, MRESE& Stop, 
                                                   double Etot)
{
  double Distance = 
    m_ReGeometry->GetComptonAbsorptionProbability(Start.GetPosition(), 
                                                  Stop.GetPosition(), Etot); 

  //cout<<"Distance:†"<<Distance<<" E: "<<Etot<<endl;
  //cout<<Start.ToString()<<endl;
  //cout<<Stop.ToString()<<endl;
  return Distance;
}


/******************************************************************************
 *
 */
double ResponseGenerator::CalculatePhiEInDegree(MRESE& Central, double Etot)
{
  return MERCSRBayesian::CalculatePhiEInDegree(&Central, Etot);
}


/******************************************************************************
 * Generate imaging response files
 */
bool ResponseGenerator::GenerateImagingPdf()
{
  cout<<"Generating imaging pdf"<<endl;

  vector<float> AxisPhiDiff;

  vector<float> Axis = CreateLogDist(0.1, 180, 20);
  // Add Inverted:
  for (unsigned int b = Axis.size()-1; b < Axis.size(); --b) {
    AxisPhiDiff.push_back(-Axis[b]);
  }
  AxisPhiDiff.push_back(0);
  for (unsigned int b = 0; b < Axis.size(); ++b) {
    AxisPhiDiff.push_back(Axis[b]);
  }

  // Simple Phi_Real/Phi_Meas-Response:
  vector<float> AxisPhiReal = CreateEquiDist(0, 180, 18);
  vector<float> AxisEi = CreateLogDist(100, 20000, 15);

  MResponseMatrixO3 Phi("Phi", AxisPhiDiff, AxisPhiReal, AxisEi);
  Phi.SetAxisNames("Phi_{meas}-Phi_{real} [deg]", 
                   "Phi_{real} [deg]", 
                   "E_{i} [keV]");


  vector<float> AxisEpsilonDiff = CreateLogDist(0.1, 180, 30, 0);
  vector<float> AxisEpsilonReal = CreateEquiDist(0, 180, 1);
  vector<float> AxisEe = CreateLogDist(100, 20000, 20);

  MResponseMatrixO3 Epsilon("Epsilon", AxisEpsilonDiff, AxisEpsilonReal, AxisEe);
  Epsilon.SetAxisNames("Epsilon_{meas}-Epsilon_{real} [deg]", 
                       "Epsilon_{real} [deg]", 
                       "E_{e} [keV]");


  vector<float> AxisEnergy2;
  AxisEnergy2 = CreateLogDist(100, 20000, 100, 1, 100000);

  MResponseMatrixO2 Energy("Energy", AxisEnergy2, AxisEnergy2);
  Energy.SetAxisNames("ideal energy [keV]", 
                      "measured energy [keV]");


  double PhiMeas;
  double PhiDiff;
  double Spd;
  MVector IdealOriginDir;

  MRawEventList* REList = 0;
  MPhysicalEvent* Event = 0;
  MComptonEvent* Compton = 0;

  int Counter = 0;
  while (LoadEvents() == true) {
    REList = m_ReReader->GetRawEventList();

    if (REList->HasOptimumEvent() == true) {
      Event = REList->GetOptimumEvent()->GetPhysicalEvent();
      if (Event != 0) {
        if (m_MimrecEventSelector.IsQualifiedEvent(Event, true) == true) {
          if (Event->GetEventType() == MPhysicalEvent::c_Compton) {
            Compton = (MComptonEvent*) Event;

            if (Compton->IsKinematicsOK() == false) continue;
            PhiMeas = Compton->Phi()*c_Deg;
            
            // Now get the ideal origin:
            if (m_SiEvent->GetNIAs() > 0) {
              IdealOriginDir = m_SiEvent->GetIAAt(0)->GetPosition();
              
              // Phi response:
              PhiDiff = Compton->GetARMGamma(IdealOriginDir)*c_Deg;
              Phi.Add(PhiDiff, PhiMeas, Compton->Ei());
              
              // Epsilon response:
              if (Compton->HasTrack() == true) {
                Spd = Compton->GetSPDElectron(IdealOriginDir)*c_Deg;
                Epsilon.Add(Spd, 1, Compton->Ee());
              }
              
              // Energy response:
              Energy.Add(m_SiEvent->GetIAAt(0)->GetEnergy(), Compton->Ei());
            }
          }
        }
      }    
    }
    if (++Counter % m_SaveAfter == 0) {
      Phi.Write((m_ResponseName + ".phi.rsp").c_str(), true);
      Epsilon.Write((m_ResponseName + ".epsilon.rsp").c_str(), true);
      Energy.Write((m_ResponseName + ".energy.rsp").c_str(), true);
    }
  }  

  Phi.Write((m_ResponseName + ".phi.rsp").c_str(), true);
  Epsilon.Write((m_ResponseName + ".epsilon.rsp").c_str(), true);
  Energy.Write((m_ResponseName + ".energy.rsp").c_str(), true);

  return true;
}


/******************************************************************************
 * Generate a special earth horizon cut response matrix
 */
bool ResponseGenerator::GenerateEarthHorizonPdf()
{
  cout<<"Generating earth horizon pdf"<<endl;

  const double MaxESigma = 100;
  const double MaxARM = 5;

  // Earth horizon cut:
  vector<float> AxisEHCEpsilonDiff;
  AxisEHCEpsilonDiff = CreateEquiDist(0, 180, 36);

  vector<float> AxisEHCElectronEnergy;
  AxisEHCElectronEnergy = CreateLogDist(100, 5000, 20, 1);

  vector<float> Add = CreateLogDist(7000, 100000, 4); 
  for (unsigned int i = 0; i < Add.size(); ++i) {
    AxisEHCElectronEnergy.push_back(Add[i]);
  }

  MResponseMatrixO2 EHC("EHC", AxisEHCEpsilonDiff, AxisEHCElectronEnergy);
  EHC.SetAxisNames("SPD [#circ]", "Measured electron energy [keV]");

  double PhiMeas;
  double Spd;
  MVector IdealOriginDir;

  MRawEventList* REList = 0;
  MPhysicalEvent* Event = 0;
  MComptonEvent* Compton = 0;

  int Counter = 0;
  while (LoadEvents() == true) {
    REList = m_ReReader->GetRawEventList();

    if (REList->HasOptimumEvent() == true) {
      Event = REList->GetOptimumEvent()->GetPhysicalEvent();
      if (Event != 0) {
        if (m_MimrecEventSelector.IsQualifiedEvent(Event) == true) {
          if (Event->GetEventType() == MPhysicalEvent::c_Compton) {
            Compton = (MComptonEvent*) Event;
            //Compton->Calculate();
            if (Compton->IsKinematicsOK() == false) continue;

            PhiMeas = Compton->Phi()*c_Deg;
            
            // Now get the ideal origin:
            if (m_SiEvent->GetNIAs() > 0) {
              IdealOriginDir = m_SiEvent->GetIAAt(0)->GetPosition();
              
              // Only take completely absorbed events within ARM
              if (fabs(m_SiEvent->GetIAAt(0)->GetEnergy() - Compton->Ei()) < MaxESigma*Compton->dEi() &&
                  Compton->GetARMGamma(IdealOriginDir)*c_Deg < MaxARM) {
                // Epsilon/EHC response:
                if (Compton->HasTrack() == true) {
                  Spd = Compton->GetSPDElectron(IdealOriginDir)*c_Deg;
                  EHC.Add(Spd, Compton->Ee());
                }
              }
            }
          }
        }
      }    
    }
    if (++Counter % m_SaveAfter == 0) {
      EHC.Write((m_ResponseName + ".compton.ehc.rsp").c_str(), true);
    }
  }  

  EHC.Write((m_ResponseName + ".compton.ehc.rsp").c_str(), true);

  return true;
}


/******************************************************************************/

ResponseGenerator* g_Prg = 0;

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

  // Initialize global MEGALIB variables, especially mgui, etc.
  MGlobal::Initialize();

  TApplication ResponseGeneratorApp("ResponseGeneratorApp", 0, 0);

  g_Prg = new ResponseGenerator();

  if (g_Prg->ParseCommandLine(argc, argv) == false) {
    cerr<<"Error during parsing of command line!"<<endl;
    return -1;
  } 
  if (g_Prg->Analyze() == false) {
    cerr<<"Error during analysis!"<<endl;
    return -2;
  } 

  if (gROOT->GetListOfCanvases()->First() != 0) {
    ResponseGeneratorApp.Run();
  }

  cout<<"Program exited normally!"<<endl;

  return 0;
}

/*
 * Cosima: the end...
 ******************************************************************************/
