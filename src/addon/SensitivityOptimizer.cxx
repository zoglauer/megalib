/* 
 * SensitivityOptimizer.cxx
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
#include <limits>
#include <vector>
using namespace std;

// ROOT
#include <TROOT.h>
#include <TEnv.h>
#include <TSystem.h>
#include <TApplication.h>
#include <TStyle.h>
#include <MString.h>

// MEGAlib
#include "MGlobal.h"
#include "MAssert.h"
#include "MStreams.h"
#include "MMath.h"
#include "MFile.h"
#include "MVector.h"
#include "MTimer.h"
#include "MFileEventsTra.h"
#include "MPhysicalEvent.h"
#include "MComptonEvent.h"
#include "MPairEvent.h"
#include "MSettingsMimrec.h"
#include "MEventSelector.h"
#include "MResponseMatrixO16.h"
#include "MDGeometryQuest.h"

/******************************************************************************/

class SensitivityPoint
{
public:
  //! Default constructor
  SensitivityPoint() : 
    m_ObservationTime(1000000.0), 
    m_SourceStartAreaPerPhotons(1.0), 
    m_SourceCounts(0), 
    m_EnergyWindowScaler(1.0) {};
  //! Copy constructor
  SensitivityPoint(const SensitivityPoint& S) { (*this) = S; }
  //! Default destructor
  ~SensitivityPoint() {};

  //! Default assignment constructor
  const SensitivityPoint& operator=(const SensitivityPoint& S) {
    m_EventSelector = S.m_EventSelector;
    m_ObservationTime = S.m_ObservationTime;
    
    m_SourceStartAreaPerPhotons = S.m_SourceStartAreaPerPhotons;
    m_SourceCounts = S.m_SourceCounts;
    
    m_BackgroundTimes = S.m_BackgroundTimes;
    m_Counts = S.m_Counts;

    m_EnergyWindowScaler = S.m_EnergyWindowScaler;

    return (*this);
  }

  //! Set the main event selector
  void UseComplexEquation(bool Flag = true) { 
    m_Complex = Flag;
  }

  //! Set the main event selector
  void SetEventSelector(MEventSelector EventSelector) { 
    m_EventSelector = EventSelector;
    m_EnergyWindowScaler = 1;
  }

  void SetLargeEnergyWindow(double NormalMin, double NormalMax, double LargeMin, double LargeMax) {
    massert(NormalMax != NormalMin);
    massert(LargeMax != LargeMin);

    m_EnergyWindowScaler = (NormalMax-NormalMin)/(LargeMax-LargeMin);
    m_EventSelector.SetFirstTotalEnergy(LargeMin, LargeMax);
  }


  //! Set the observation time
  void SetObservationTime(double ObservationTime) { 
    m_ObservationTime = ObservationTime;
  }

  //! Set the start area
  void SetStartAreaPerPhotons(double StartAreaPerPhotons) { 
    m_SourceStartAreaPerPhotons = StartAreaPerPhotons;
  }

  //! Set the name of background type
  void SetBackgroundName(unsigned int FileId, MString BackgroundName) {
    if (m_BackgroundNames.size() <= FileId) {
      m_BackgroundTimes.resize(FileId+1);
      m_Counts.resize(FileId+1);
      m_BackgroundNames.resize(FileId+1);
    }
    m_BackgroundNames[FileId] = BackgroundName;
  }

  //! Set background file observation time 
  void SetBackgroundTime(unsigned int FileId, double BackgroundTime) {
    if (m_BackgroundTimes.size() <= FileId) {
      m_BackgroundTimes.resize(FileId+1);
      m_BackgroundNames.resize(FileId+1);
      m_Counts.resize(FileId+1);
    }
    m_BackgroundTimes[FileId] = BackgroundTime;
  }


  //! If the event passes the event selction criteria,
  void TestBackgroundEvent(MPhysicalEvent* Event, unsigned int FileID) {
    if (m_EventSelector.IsQualifiedEventFast(Event) == true) {
      m_Counts[FileID]++;
    }
  }


  //! If the event passes the event selection criteria,
  void TestSourceEvent(MPhysicalEvent* Event) {
    if (m_EventSelector.IsQualifiedEventFast(Event) == true) {
      //cout<<"Qualified"<<endl;
      // ... AND it must ORIGINATE from the given ARM-radius
      if (Event->GetOIDirection() == g_VectorNotDefined) {
        //mout<<"Your source file does not contain OD information... Cannot count event "<<Event->GetId()<<"..."<<endl;
        m_SourceCounts++;
      } else {
        if (m_EventSelector.IsDirectionWithinARMWindow(Event->GetOIDirection()) == true) {
          cout<<"WithinARMWindow: "<<Event->GetId()<<endl;
          m_SourceCounts++;
        }
      }
    } else {
      //mlog<<"Not qualified"<<endl;
      //cout<<m_EventSelector.ToString()<<endl;
    }
  }

  //! 
  double GetSourceCounts() {
    return m_SourceCounts;
  }

  //! 
  double GetBackgroundCounts() {
    double Counts = 0;
    for (unsigned int i = 0; i < m_Counts.size(); ++i) Counts += m_Counts[i];
    Counts *= m_EnergyWindowScaler;
    return Counts;
  }

  //! 
  bool AllBackgroundComponentCountsAbove(int Counts) {
    for (unsigned int i = 0; i < m_Counts.size(); ++i) {
      if (m_Counts[i] < Counts) return false;
    }
    return true;
  }

  //! 
  double GetBackgroundRate() {
    double BackgroundRate = 0.0;
    for (unsigned int i = 0; i < m_Counts.size(); ++i) {
      if (m_BackgroundTimes[i] == 0) {
        merr<<"No measurement time given for file ID "<<i<<endl;
        return 0;
      }
      BackgroundRate += (m_Counts[i]*m_EnergyWindowScaler)/m_BackgroundTimes[i];
    }
    return BackgroundRate;
  }

  //! 
  double GetBackgroundRateUncertainty() {
    double BackgroundRateUncertainty = 0.0;
    for (unsigned int i = 0; i < m_Counts.size(); ++i) {
      if (m_BackgroundTimes[i] == 0) {
        merr<<"No measurement time given for file ID "<<i<<endl;
        return 0;
      }
      BackgroundRateUncertainty += pow((sqrt(m_Counts[i])*m_EnergyWindowScaler)/m_BackgroundTimes[i], 2);
    }
    BackgroundRateUncertainty = sqrt(BackgroundRateUncertainty);

    return BackgroundRateUncertainty;
  }


  //! 
  double GetEffectiveArea() {
    if (m_SourceStartAreaPerPhotons <= 0) {
      merr<<"No start area per photons defined"<<endl;
      return 0;
    }
    return m_SourceStartAreaPerPhotons * m_SourceCounts;
  }

  //! 
  double GetEffectiveAreaUncertainty() {
    if (m_SourceStartAreaPerPhotons <= 0) {
      merr<<"No start area per photons defined"<<endl;
      return 0;
    }
    return m_SourceStartAreaPerPhotons * sqrt(double(m_SourceCounts)); // double() due to a compiler warning
  }


  //! 
  double GetSensitivity() {
    double Sigma = 3.0;

    if (m_ObservationTime <= 0) {
      merr<<"No observation time"<<endl;
      return 0;
    }
    if (m_SourceStartAreaPerPhotons <= 0) {
      merr<<"No start area per photons defined"<<endl;
      return 0;
    }
    
    double EffectiveArea = m_SourceStartAreaPerPhotons * m_SourceCounts;

    if (EffectiveArea == 0) {
      merr<<"No effective area: StartAreaPerPhotons="<<m_SourceStartAreaPerPhotons<<" Source counts="<<m_SourceCounts<<endl;
      //mlog<<"No effective area: StartAreaPerPhotons="<<m_SourceStartAreaPerPhotons<<" Source counts="<<m_SourceCounts<<endl;
      //mlog<<(*this)<<endl;
      return 0;
    }

    double BackgroundRate = 0.0;
    for (unsigned int i = 0; i < m_Counts.size(); ++i) {
      if (m_BackgroundTimes[i] == 0) {
        merr<<"No measurement time given for file ID "<<i<<endl;
        return 0;
      }
      BackgroundRate += (m_Counts[i]*m_EnergyWindowScaler)/m_BackgroundTimes[i];
    }
    double Sensitivity;
    if (m_Complex == false) {
      Sensitivity = Sigma*sqrt(BackgroundRate*m_ObservationTime)/m_ObservationTime/EffectiveArea;
    } else {
      Sensitivity = 0.5*(Sigma*Sigma + Sigma*sqrt(Sigma*Sigma + 4*BackgroundRate*m_ObservationTime))/m_ObservationTime/EffectiveArea;
    }

    return Sensitivity;
  }

  //! 
  double GetSensitivityUncertainty() {
    double Sigma = 3.0;

    if (m_ObservationTime <= 0) {
      merr<<"No observation time"<<endl;
      return 0;
    }
    if (m_SourceStartAreaPerPhotons <= 0) {
      merr<<"No start area per photons defined"<<endl;
      return 0;
    }
    
    double EffectiveArea = m_SourceStartAreaPerPhotons * m_SourceCounts;
    double EffectiveAreaUncertainty = m_SourceStartAreaPerPhotons * sqrt(double(m_SourceCounts)); // double() due to a compiler warning

    if (EffectiveArea == 0) {
      merr<<"No effective area: StartAreaPerPhotons="<<m_SourceStartAreaPerPhotons<<" Source counts="<<m_SourceCounts<<endl;
      //mlog<<"No effective area: StartAreaPerPhotons="<<m_SourceStartAreaPerPhotons<<" Source counts="<<m_SourceCounts<<endl;
      //mlog<<"Data: "<<(*this)<<endl;
      return 0;
    }

    double BackgroundRate = 0.0;
    double BackgroundRateUncertainty = 0.0;
    for (unsigned int i = 0; i < m_Counts.size(); ++i) {
      if (m_BackgroundTimes[i] == 0) {
        merr<<"No measurement time given for file ID "<<i<<endl;
        return 0;
      }
      BackgroundRate += (m_Counts[i]*m_EnergyWindowScaler)/m_BackgroundTimes[i];
      BackgroundRateUncertainty += pow((sqrt(m_Counts[i])*m_EnergyWindowScaler)/m_BackgroundTimes[i], 2);
    }
    BackgroundRateUncertainty = sqrt(BackgroundRateUncertainty);

    double Sensitivity;
    double SensitivityUncertainty;
    if (m_Complex == false) {
      Sensitivity = Sigma*sqrt(BackgroundRate*m_ObservationTime)/m_ObservationTime/EffectiveArea;
      SensitivityUncertainty = 
        Sigma/sqrt(m_ObservationTime)/EffectiveArea * 
        sqrt((BackgroundRateUncertainty*BackgroundRateUncertainty/4/BackgroundRate) + 
             (BackgroundRate*EffectiveAreaUncertainty*EffectiveAreaUncertainty/EffectiveArea/EffectiveArea));
    } else {
      Sensitivity = 0.5*(Sigma*Sigma + Sigma*sqrt(Sigma*Sigma + 4*BackgroundRate*m_ObservationTime))/m_ObservationTime/EffectiveArea;
      SensitivityUncertainty = 
        sqrt(pow(Sigma*BackgroundRateUncertainty/sqrt(Sigma*Sigma + 4*BackgroundRate*m_ObservationTime)/EffectiveArea, 2) + 
             pow(Sensitivity*EffectiveAreaUncertainty/EffectiveArea, 2));
    }

    return SensitivityUncertainty;
  }

private:
  MEventSelector m_EventSelector;
  double m_ObservationTime;

  double m_SourceStartAreaPerPhotons;
  int m_SourceCounts;

  vector<double> m_BackgroundTimes;
  vector<MString> m_BackgroundNames;
  vector<double> m_Counts;

  double m_EnergyWindowScaler;

  bool m_Complex;

  friend ostream& operator<<(ostream& os, SensitivityPoint& S);
};


/******************************************************************************/


ostream& operator<<(ostream& os, SensitivityPoint& S)
{
  double TotalBackgroundRate = 0.0;

  os<<endl;
  os<<"Summary of this sensitivity point "<<endl;
  os<<"*********************************"<<endl;
  os<<endl;
  os<<S.m_EventSelector<<endl;
  os<<"Source counts: "<<S.m_SourceCounts<<" cts"<<endl;
  if (S.m_SourceCounts < 1000) {
    os<<endl;
    if (S.m_SourceCounts < 30) {
      os<<"WARNING !!! WARNING !!! WARNING"<<endl;
      os<<endl;
      os<<"Your source count rates is below 30 counts."<<endl;
      os<<"The sensitivity result is complete bull-shit"<<endl;
      os<<endl;
      os<<"You should aim at having at least 1000, better 10000 source counts in order for the result to be reliable!"<<endl;
      os<<endl;
      os<<"WARNING !!! WARNING !!! WARNING"<<endl;
    } else {
      os<<"Important information:"<<endl;
      os<<endl;
      os<<"You should aim at having at least 1000, better 10000 source counts in order for the result to be reliable!"<<endl;
    }
    os<<endl;
  }
  os<<endl;
  os<<"Background statistics"<<endl;
  double LowestBackground = numeric_limits<double>::max();
  for (unsigned int b = 0; b < S.m_Counts.size(); ++b) {
    if (S.m_Counts[b]*S.m_EnergyWindowScaler < LowestBackground) LowestBackground = S.m_Counts[b]*S.m_EnergyWindowScaler;
    os<<"  *  "<<S.m_BackgroundNames[b]<<endl;
    os<<"     Background counts in source window "<<b<<": "<<S.m_Counts[b]*S.m_EnergyWindowScaler<<" cts --> Rate: "<<S.m_Counts[b]*S.m_EnergyWindowScaler/S.m_BackgroundTimes[b]<<" cts/sec"<<endl;
    TotalBackgroundRate += S.m_Counts[b]*S.m_EnergyWindowScaler/S.m_BackgroundTimes[b];
  }
  os<<endl;
  os<<" -> EffectiveArea:         "<<S.GetEffectiveArea()<<" cm2 +- "<<S.GetEffectiveAreaUncertainty()<<" cm2"<<endl;
  os<<" -> Total background rate: "<<S.GetBackgroundRate()<<" cts/s +- "<<S.GetBackgroundRateUncertainty()<<" cts/s"<<endl;
  os<<" -> Sensitivity:           "<<S.GetSensitivity()<<" ph/cm2/s +- "<<S.GetSensitivityUncertainty()<<" ph/cm2/s"<<endl;
  os<<endl;
  if (LowestBackground < 100) {
    if (LowestBackground < 30) {
      os<<"WARNING !!! WARNING !!! WARNING"<<endl;
      os<<endl;
      os<<"One of your background count rates is below 30 counts."<<endl;
      os<<"You need at least 30 counts for the calculation to be applicable (Gaussian statsitics)!"<<endl;
      os<<"But you need a few hundred counts for the calculation to be really reliable!"<<endl;
      os<<endl;
      os<<"However, if other, stronger background components dominate the overall background count rate significantly,"<<endl;
      os<<"(factor of ~10) then the result is still OK. If not, then your sensitivity is definitely too good!"<<endl;
      os<<"In addition, since we scan the whole data space, it would be good to have a few hundred counts,"<<endl;
      os<<"in order to avoid fake minima due to count statistics!"<<endl;
      os<<endl;
      os<<"WARNING !!! WARNING !!! WARNING"<<endl;
    } else {
      os<<"Important information:"<<endl;
      os<<endl;
      os<<"Since we scan the whole data space, it would be good to have a few hundred background counts,"<<endl;
      os<<"in order to avoid fake minima due to count statistics!"<<endl;
      os<<"Thus there is a chance that the above number is too good!"<<endl;
    }
  }
  os<<endl;
  
  return os;
}

/******************************************************************************/

class SensitivityOptimizer
{
public:
  //! Default constructor
  SensitivityOptimizer();
  //! Default destructor
  virtual ~SensitivityOptimizer();
  
  //! Parse the command line
  bool ParseCommandLine(int argc, char** argv);
  //! Analyze what eveer needs to be analyzed...
  bool Analyze();
  //! Interrupt the analysis
  void Interrupt() { m_Interrupt = true; }

private:
  //! Generate a list of master files, which contain only events which 
  //! fullfill the requirements in the configuration file:
  bool GenerateMasterFiles(MString Directory);
  //! Generate one master file
  bool GenerateMasterFile(MString In, MString Out);
  //! Find the event parameters with the optimum sensitivity 
  bool FindOptimum();

  unsigned int GetPhotoSize();
  unsigned int GetPhotoIndex(unsigned int EnergyMin, unsigned int BRA, unsigned int BDE, unsigned int PosTheta, unsigned int PosPhi);

  unsigned int GetTrackedComptonSize();
  unsigned int GetTrackedComptonIndex(unsigned int BDE, unsigned int BRA, unsigned int CQF, unsigned int TQF,  unsigned int EHP, unsigned int EHC, unsigned int EnergyMin, unsigned int Phi, unsigned int The, unsigned int SPD, unsigned int ARM, unsigned int TSL, unsigned int CSL, unsigned int FDI, unsigned int PosTheta, unsigned int PosPhi); 

  unsigned int GetUntrackedComptonSize();
  unsigned int GetUntrackedComptonIndex(unsigned int BDE, unsigned int BRA, unsigned int CQF, unsigned int EHP, unsigned int EHC, unsigned int EnergyMin, unsigned int Phi,unsigned int ARM, unsigned int CSL, unsigned int FDI, unsigned int PosTheta, unsigned int PosPhi); 

  unsigned int GetPairSize();
  unsigned int GetPairIndex(unsigned int IDP, unsigned int POP, unsigned int BDE, unsigned int BRA, unsigned int TQF, unsigned int EHP, unsigned int EHC, unsigned int EnergyMin, unsigned int ARM, unsigned int TSL, unsigned int PosTheta, unsigned int PosPhi);

  unsigned int FindSourceIndex(unsigned int theta, unsigned int phi);

private:
  MString m_Name;

  //! Mode of operation
  int m_ModeSourceExtension;
  int m_ModeSpectrum;
  int m_ModeObservation;
  static const int s_ModeNotDefined        = 0;
  static const int s_ModePointSource       = 1;
  static const int s_ModeExtended          = 2;
  static const int s_ModeLine              = 3;
  static const int s_ModeContinuum         = 4;
  static const int s_ModeSingleObs         = 5;   
  static const int s_ModeAllSkyObs         = 6;   

  //! True, if the analysis needs to be interrupted
  bool m_Interrupt;

  //! List of all source file names
  vector<MString> m_SourceFile;
  //!
  vector<double> m_SourceStartArea;
  //!
  vector<int> m_SourceStartPhotons;
  //!
  vector<double> m_SourceStartFluence;
  //!
  vector<float> m_SourcePowerLaw;
  //!
  vector<float> m_SourcePowerLawEmin;
  //!
  vector<float> m_SourcePowerLawEmax;
  //!
  vector<vector<int> > m_SourceStartPhotonsContinuumSensitivityBin;
  //! 
  vector<float> m_SourceTheta;
  //! 
  vector<float> m_SourcePhi;

  //! True if the complex sensitivity equation should be used
  bool m_ComplexEquation;

  //! List of all background file names
  vector<MString> m_BackgroundFiles;
  //! List of all measurement times
  vector<double> m_BackgroundTimes;

  //! The main event selector
  MEventSelector m_EventSelector;

  //!
  double m_ObservationTime;

  vector<unsigned int> m_CSLMin;
  vector<unsigned int> m_CSLMax;
  vector<unsigned int> m_TSLMin;
  vector<unsigned int> m_TSLMax;
  vector<float> m_EHC;
  vector<float> m_EHP;
  MString m_EHPFile;
  vector<float> m_Phi;
  vector<float> m_The;
  vector<float> m_CQF;
  vector<float> m_TQF;
  vector<float> m_ARMorRadius;
  vector<float> m_SPD;
  vector<float> m_POP;
  vector<float> m_IDP;
  vector<float> m_FDI;
  vector<float> m_BRA;
  vector<float> m_BDE;
  vector<float> m_PosTheta;
  vector<float> m_PosPhi;
  vector<float> m_EnergyMin;
  vector<float> m_EnergyMax;
  vector<float> m_AreaRadius;

  //! Large energy window to collect more background events - later scaled.
  //! Negative or zero values mean: Not used
  double m_EnergyWindowMin;
  double m_EnergyWindowMax;

  //! Include the geometry
  MDGeometryQuest m_Geometry;

  //! Only use background bins, where all background components conatin data
  int m_MinBackgroundCounts;
};



/******************************************************************************/


/******************************************************************************
 * Default constructor
 */
SensitivityOptimizer::SensitivityOptimizer() : m_Interrupt(false)
{
  gStyle->SetPalette(1, 0);

  m_ModeSourceExtension = s_ModeNotDefined;
  m_ModeSpectrum = s_ModeNotDefined;
  m_ModeObservation = s_ModeNotDefined;
  m_ComplexEquation = true;

  //m_SourceFile = g_StringNotDefined;
  m_EHPFile = g_StringNotDefined;
  //m_SourcePowerLaw = g_FloatNotDefined;
  m_Name = "Optimizer";
  m_EnergyWindowMin = -1;
  m_EnergyWindowMax = -1;

  m_ObservationTime = 1000000;

  m_MinBackgroundCounts = 0;
}


/******************************************************************************
 * Default destructor
 */
SensitivityOptimizer::~SensitivityOptimizer()
{
  // Intentionally left blanck
}


/******************************************************************************
 * Parse the command line
 */
bool SensitivityOptimizer::ParseCommandLine(int argc, char** argv)
{
  ostringstream Usage;
  Usage<<endl;
  Usage<<"  Usage: SensitivityOptimizer <options>"<<endl;
  Usage<<"    General options:"<<endl;
  Usage<<"      -n <name>:                                    prefix for the output files"<<endl;
  Usage<<"      -t <time>:                                    Observation time in seconds"<<endl;
  Usage<<"      -g <name>:                                    Geometry file name"<<endl;
  Usage<<"      --simple:                                     Use simple equation for sensitivity calculation, default and recommended is complex"<<endl;
  Usage<<"      -h:                                           Print this help"<<endl;
  Usage<<endl;
  Usage<<"    Input file options:"<<endl;
  Usage<<"      -p <file> <sim photons> <sim area> <theta> <phi>:  "<<endl;
  Usage<<"                                                    Source file, inlcuding the number of simulated photons and their start area"<<endl;
  Usage<<"      -pa <file> <sim photons> <fluence>:  "<<endl;
  Usage<<"                                                    Source file for extended line source sensitivity calculations, including the number of simulated photons and the fluence [ph/cm2]: flux [ph/cm2/s] x simulated observation time "<<endl;
  Usage<<"      -k <file> <sim photons> <sim area> <plaw> <plaw Emin> <plaw Emax> <theta> <phi>: "<<endl;
  Usage<<"                                                    Source file for continuum sensitivity calculation, inlcuding the number of simulated photons and their start area and the power law index for determining the started photons per bin"<<endl;
  Usage<<"      -b <file> <time>:                             Background file name and measurement time"<<endl;
  Usage<<endl;
  Usage<<"    Data space options:"<<endl;
  Usage<<"      -c <file>:                                    Mimrec configuration file"<<endl;
  Usage<<endl;
  Usage<<"      --ptheta <t min> <t max> <t steps>:    P/C    theta position of the source (should include all but not more values from -p and -k)"<<endl;
  Usage<<"      --pphi <p min> <p max> <p steps>:      P/C    phi position of the source (should include all but not more values from -p and -k)"<<endl;
  Usage<<"      --csl <min le> <max le> <max>:         C      Compton sequence length: the minimum varies between min le and max le, the maximum is fixed to max "<<endl;
  Usage<<"      --tsl <min le> <max le> <max>:         C      Track sequence length: the minimum varies between min le and max le, the maximum is fixed to max "<<endl;
  Usage<<"      --ehc <min> <max> <steps>:             P/C    Earth horizon cut in deg: 0..min..max [0..180]"<<endl;
  Usage<<"      --ehp <min> <max> <steps> <rsp file>:  C      Earth horizon probability: 0..min..max [0..1]"<<endl;
  Usage<<"      --phi <min> <max> <steps>:             C      Compton scatter angle in deg: 0..min..max [0..180]"<<endl;
  Usage<<"      --the <min> <max> <steps>:             C      Total scatter angle geo-kin difference in deg: 0..min..max [180]"<<endl;
  Usage<<"      --cqf <min> <max> <steps>:             C      Compton quality factor: 0..min..max"<<endl;
  Usage<<"      --tqf <min> <max> <steps>:             P/C    Track quality factor: 0..min..max"<<endl;
  Usage<<"      --fdi <min> <max> <steps>:             C      Distance between first and second interaction (min..max..inf)"<<endl;
  Usage<<"      --arm <min> <max> <steps>:             P/C    ARM in deg (Radius: 0..min..max)"<<endl;
  Usage<<"      --spd <min> <max> <steps>:             C      SPD in deg (Radius: 0..min..max) - Not necessary for not tracked events"<<endl;
  Usage<<"      --egy <mean> <min> <max> <steps>:      P/C    Energy in keV (Radius: 0..min..max)"<<endl;
  Usage<<"      --contegy <min> <max>:                 P/C    Energy interval for continuum sensitivity"<<endl;
  Usage<<"      --pop <min> <max> <steps>:             P      Opening angle pairs (Radius: 0..min..max)"<<endl;
  Usage<<"      --idp <min> <max> <steps>:             P      Initial energy deposit (Energy: 0..min..max)"<<endl;
  Usage<<"      --bra <min> <max> <steps>:             C/P    Beam radius (Radius: 0..min..max)"<<endl;
  Usage<<"      --bde <min> <max> <steps>:             C/P    Beam depth (Radius: 0..min..max)"<<endl;
  Usage<<endl;
  Usage<<"      Special:"<<endl;
  Usage<<"      -w <min> <max>:                               Use a larger energy window for collection of background events (in keV)"<<endl;
  Usage<<"      --min-background-counts:                    Only use bins where all background components contain data"<<endl;
  Usage<<endl;
  Usage<<endl;
  Usage<<"    What does e.g. \"--arm <min> <max> <steps>\" mean?"<<endl;
  Usage<<"    If you have e.g. \"--arm 2 3 3\" then you get the following bins:"<<endl;
  Usage<<"    (1) r=0..2 deg, (2) r=0..2.5 deg, (3) r=0..3 deg"<<endl;
  Usage<<"    "<<endl;
  Usage<<"    "<<endl;
  Usage<<endl;

  string Option;

  // Check for help
  for (int i = 1; i < argc; i++) {
    Option = argv[i];
    if (Option == "-h" || Option == "--help" || Option == "?" || Option == "-?") {
      mlog<<Usage.str()<<endl;
      return false;
    }
  }

  MString GeometryFile = g_StringNotDefined;
  MString ConfigurationFile = g_StringNotDefined;
  MString MasterDir = g_StringNotDefined;

  // Now parse the command line options:
  for (int i = 1; i < argc; i++) {
		Option = argv[i];

		// First check if each option has sufficient arguments:
		// Single argument
    if (Option == "-c" || Option == "-g" || Option == "-m" || Option == "-n") {
			if (!((argc > i+1) && 
            (argv[i+1][0] != '-' || isalpha(argv[i+1][1]) == 0))){
				mlog<<"Error: Option \""<<argv[i][1]<<"\" needs one argument!"<<endl;
				mlog<<Usage.str()<<endl;
				return false;
			}
		} else if (Option == "--csl" || Option == "-b" || Option == "-w" || Option == "--pos") {
			if (!((argc > i+2) && 
            (argv[i+1][0] != '-' || isalpha(argv[i+1][1]) == 0) && 
            (argv[i+2][0] != '-' || isalpha(argv[i+2][1]) == 0))) {
				mlog<<"Error: Option \""<<argv[i][1]<<"\" needs two arguments!"<<endl;
				mlog<<Usage.str()<<endl;
				return false;
			}
		} else if (Option == "--csl" || Option == "--ehc" || Option == "--phi" || Option == "--the" || 
               Option == "--cqf" || Option == "-s" || Option == "--arm" || Option == "--spd" || 
               Option == "--tqf" || Option == "--csl" || Option == "--pop" || Option == "--idp" || 
               Option == "--bra" || Option == "--bde") {
			if (!((argc > i+3) && 
            (argv[i+1][0] != '-' || isalpha(argv[i+1][1]) == 0) && 
            (argv[i+2][0] != '-' || isalpha(argv[i+2][1]) == 0) && 
            (argv[i+3][0] != '-' || isalpha(argv[i+3][1]) == 0))) {
				mlog<<"Error: Option \""<<argv[i][1]<<"\" needs three arguments!"<<endl;
				mlog<<Usage.str()<<endl;
				return false;
			}
		} else if (Option == "--egy" || Option == "--ehp" || Option == "-k") {
			if (!((argc > i+4) && 
            (argv[i+1][0] != '-' || isalpha(argv[i+1][1]) == 0) && 
            (argv[i+2][0] != '-' || isalpha(argv[i+2][1]) == 0) && 
            (argv[i+3][0] != '-' || isalpha(argv[i+3][1]) == 0) && 
            (argv[i+4][0] != '-' || isalpha(argv[i+4][1]) == 0))) {
				mlog<<"Error: Option \""<<argv[i][1]<<"\" needs four arguments!"<<endl;
				mlog<<Usage.str()<<endl;
				return false;
			}
		}
  }

  // Now parse the command line options:
  for (int i = 1; i < argc; i++) {
		Option = argv[i];
    
    if (Option == "-n") {
      m_Name = argv[++i];
			mlog<<"Accepting output file name: "<<m_Name<<endl;
      mlog.Connect(m_Name + ".log");
    }
  }

  // Now parse the command line options:
  for (int i = 1; i < argc; i++) {
		Option = argv[i];

		// Then fulfill the options:
    if (Option == "-p") {
      if ((m_ModeSourceExtension != s_ModeNotDefined && m_ModeSourceExtension != s_ModePointSource) ||
          (m_ModeSpectrum != s_ModeNotDefined && m_ModeSpectrum != s_ModeLine)) {
        mlog<<"-p: You only can use one of -p, -pa, -k, or -ka (multiple occurances of p or k ok)!"<<endl;
        mlog<<Usage.str()<<endl;
        return false;               
      }
      m_ModeSourceExtension = s_ModePointSource;
      m_ModeSpectrum = s_ModeLine;
      m_SourceFile.push_back(argv[++i]);
      if (MFile::Exists(m_SourceFile.back()) == false) {
        mlog<<"-p: Cannot find source file: "<<m_SourceFile.back()<<endl;
        mlog<<Usage.str()<<endl;
        return false;       
      }
      m_SourceStartPhotons.push_back(atoi(argv[++i]));
      m_SourceStartArea.push_back(atof(argv[++i]));
      m_SourceTheta.push_back(atof(argv[++i]));
      m_SourcePhi.push_back(atof(argv[++i]));
			mlog<<"Accepting source file data: "<<m_SourceFile.back()<<" "
          <<m_SourceStartPhotons.back()<<" "<<m_SourceStartArea.back()<<" "
          <<m_SourceTheta.back()<<" "<<m_SourcePhi.back()<<endl;
    } else if (Option == "-pa") {
      if ((m_ModeSourceExtension != s_ModeNotDefined && m_ModeSourceExtension != s_ModeExtended) ||
          (m_ModeSpectrum != s_ModeNotDefined && m_ModeSpectrum != s_ModeLine)) {
        mlog<<"-pa: You only can use one of -p, -pa, -k, or -ka!"<<endl;
        mlog<<Usage.str()<<endl;
        return false;               
      }
      m_ModeSourceExtension = s_ModeExtended;
      m_ModeSpectrum = s_ModeLine;
      m_SourceFile.push_back(argv[++i]);
      if (MFile::Exists(m_SourceFile.back()) == false) {
        mlog<<"-pa: Cannot find source file: "<<m_SourceFile.back()<<endl;
        mlog<<Usage.str()<<endl;
        return false;       
      }
      m_SourceStartPhotons.push_back(atoi(argv[++i]));
      m_SourceStartFluence.push_back(atof(argv[++i]));
			mlog<<"Accepting source file data: "<<m_SourceFile.back()<<" "
          <<m_SourceStartPhotons.back()<<" "<<m_SourceStartFluence.back()<<endl;
    } else if (Option == "-k") {
      if ((m_ModeSourceExtension != s_ModeNotDefined && m_ModeSourceExtension != s_ModePointSource) ||
          (m_ModeSpectrum != s_ModeNotDefined && m_ModeSpectrum != s_ModeContinuum)) {
        mlog<<"-k: You only can use one of -p, -pa, -k, or -ka (multiple occurances of -p and -k ok...)!"<<endl;
        mlog<<Usage.str()<<endl;
        return false;               
      }
      m_ModeSourceExtension = s_ModePointSource;
      m_ModeSpectrum = s_ModeContinuum;
      m_SourceFile.push_back(argv[++i]);
      if (MFile::Exists(m_SourceFile.back()) == false) {
        mlog<<"-k: Cannot find source file: "<<m_SourceFile.back()<<endl;
        mlog<<Usage.str()<<endl;
        return false;       
      }
      m_SourceStartPhotons.push_back(atoi(argv[++i]));
      m_SourceStartArea.push_back(atof(argv[++i]));
      m_SourcePowerLaw.push_back(atof(argv[++i]));
      m_SourcePowerLawEmin.push_back(atof(argv[++i]));
      m_SourcePowerLawEmax.push_back(atof(argv[++i]));
      m_SourceTheta.push_back(atof(argv[++i]));
      m_SourcePhi.push_back(atof(argv[++i]));
      vector<int> Empty;
      m_SourceStartPhotonsContinuumSensitivityBin.push_back(Empty);
			mlog<<"Accepting continuum source file name: "<<m_SourceFile.back()
          <<" (start photons: "<<m_SourceStartPhotons.back()
          <<", start area: "<<m_SourceStartArea.back()
          <<", power law: "<<m_SourcePowerLaw.back()
          <<", emin:  "<<m_SourcePowerLawEmin.back()
          <<", emax:  "<<m_SourcePowerLawEmax.back()
          <<", theta: "<<m_SourceTheta.back()
          <<", phi:   "<<m_SourcePhi.back()<<")"<<endl;
    } else if (Option == "-b") {
      m_BackgroundFiles.push_back(argv[++i]);
      m_BackgroundTimes.push_back(atof(argv[++i]));
      if (MFile::FileExists(m_BackgroundFiles.back()) == false) {
        mlog<<"File \""<<m_BackgroundFiles.back()<<"\" does not exist!"<<endl; 
        return false;
      }
			mlog<<"Accepting background file name: "<<m_BackgroundFiles.back()<<" ("<<m_BackgroundTimes.back()<<"sec)"<<endl;
    } else if (Option == "-n") { ++i;
    } else if (Option == "-d") { if (g_Verbosity < 2) g_Verbosity = 2;
    } else if (Option == "-c") {
      ConfigurationFile = argv[++i];
      if (MFile::Exists(ConfigurationFile) == false) {
        mlog<<"-c: Cannot find configuration file: "<<ConfigurationFile<<endl;
        mlog<<Usage.str()<<endl;
        return false;       
      }
			mlog<<"Accepting configuration file name: "<<ConfigurationFile<<endl;
    } else if (Option == "-g") {
      GeometryFile = argv[++i];
      if (MFile::Exists(GeometryFile) == false) {
        mlog<<"-g: Cannot find geometry file: "<<GeometryFile<<endl;
        mlog<<Usage.str()<<endl;
        return false;       
      }
			mlog<<"Accepting configuration file name: "<<ConfigurationFile<<endl;
    } else if (Option == "--simple") {
      m_ComplexEquation = false; 
			mlog<<"Using simple sensitivity equation"<<endl;
    } else if (Option == "--min-background-counts") {
      m_MinBackgroundCounts = atoi(argv[++i]); 
			mlog<<"Using only bins where all background components have at leats "<<m_MinBackgroundCounts<<" counts"<<endl;
    } else if (Option == "-t") {
      m_ObservationTime = atof(argv[++i]); 
			mlog<<"Accepting observation time: "<<m_ObservationTime<<endl;
    } else if (Option == "-w") {
      m_EnergyWindowMin = atof(argv[++i]);
      m_EnergyWindowMax = atof(argv[++i]);
			mlog<<"Accepting new energy Window: "<<m_EnergyWindowMin<<" - "<<m_EnergyWindowMax<<endl;
    } else if (Option == "-m") {
      MasterDir = argv[++i];
			mlog<<"Accepting generation of master file"<<endl;
    } else if (Option == "--csl") {
      int minmin = atoi(argv[++i]);
      int minmax = atoi(argv[++i]);
      int max = atoi(argv[++i]);
      if (minmin > max || minmax > max) {
        mlog<<"--csl: min > max"<<endl;
        mlog<<Usage.str()<<endl;
        return false;       
      }
      if (minmin > minmax) {
        mlog<<"--csl: minmin > minmax"<<endl;
        mlog<<Usage.str()<<endl;
        return false;       
      }
      for (int s = minmin; s <= minmax; ++s) {
        m_CSLMin.push_back((unsigned int) s);
        m_CSLMax.push_back((unsigned int) max);
      }
			mlog<<"Accepting "<<m_CSLMin.size()<<" Compton sequence length intervals: ";
      for (unsigned int i = 0; i < m_CSLMin.size(); ++i) {
        mlog<<m_CSLMin[i]<<"-"<<m_CSLMax[i]<<"  ";
      }
      mlog<<endl;
    } else if (Option == "--tsl") {
      int minmin = atoi(argv[++i]);
      int minmax = atoi(argv[++i]);
      int max = atoi(argv[++i]);
      if (minmin > max || minmax > max) {
        mlog<<"--tsl: min > max"<<endl;
        mlog<<Usage.str()<<endl;
        return false;       
      }
      if (minmin > minmax) {
        mlog<<"--tsl: minmin > minmax"<<endl;
        mlog<<Usage.str()<<endl;
        return false;       
      }
      for (int s = minmin; s <= minmax; ++s) {
        m_TSLMin.push_back((unsigned int) s);
        m_TSLMax.push_back((unsigned int) max);
      }
			mlog<<"Accepting "<<m_TSLMin.size()<<" track sequence length intervals: ";
      for (unsigned int i = 0; i < m_TSLMin.size(); ++i) {
        mlog<<m_TSLMin[i]<<"-"<<m_TSLMax[i]<<"  ";
      }
      mlog<<endl;
    } else if (Option == "--ehc") {
      float min = atof(argv[++i]);
      float max = atof(argv[++i]);
      if (min < 0 || min > 180 || max < 0 || max > 180) {
        mlog<<"--ehc: All values have to be between 0 and 180!"<<endl;
        mlog<<Usage.str()<<endl;
        return false;               
      }
      if (min > max) {
        mlog<<"--ehc: min > max"<<endl;
        mlog<<Usage.str()<<endl;
        return false;       
      }
      int steps = atoi(argv[++i]);
      if (steps == 0) {
        mlog<<"You need to give a reasonable amount of steps for --ehc!"<<endl;
        mlog<<Usage.str()<<endl;
        return false;
      }
      m_EHC.push_back(min);
      for (int s = 1; s < steps; ++s) m_EHC.push_back(min + s*(max-min)/(steps-1));
			mlog<<"Accepting "<<m_EHC.size()<<" earth horizon cut steps: ";
      for (unsigned int i = 0; i < m_EHC.size(); ++i) {
        mlog<<m_EHC[i]<<"  ";
      }
      mlog<<endl;
    } else if (Option == "--ehp") {
      float min = atof(argv[++i]);
      float max = atof(argv[++i]);
      if (min < 0 || min > 1 || max < 0 || max > 1) {
        mlog<<"--ehp: All values have to be between 0 and 1!"<<endl;
        mlog<<Usage.str()<<endl;
        return false;               
      }
      if (min > max) {
        mlog<<"--ehp: min > max"<<endl;
        mlog<<Usage.str()<<endl;
        return false;       
      }
      int steps = atoi(argv[++i]);
      if (steps == 0) {
        mlog<<"You need to give a reasonable amount of steps for --ehp!"<<endl;
        mlog<<Usage.str()<<endl;
        return false;
      }
      m_EHP.push_back(min);
      for (int s = 1; s < steps; ++s) m_EHP.push_back(min + s*(max-min)/(steps-1));

      m_EHPFile = argv[++i];
      if (MFile::Exists(m_EHPFile) == false) {
        mlog<<"-ehp: Cannot find file: "<<m_EHPFile<<endl;
        mlog<<Usage.str()<<endl;
        return false;       
      }

			mlog<<"Accepting "<<m_EHP.size()<<" earth horizon probability steps: ";
      for (unsigned int i = 0; i < m_EHP.size(); ++i) {
        mlog<<m_EHP[i]<<"  ";
      }
      mlog<<endl;
    } else if (Option == "--fdi") {
      float min = atof(argv[++i]);
      float max = atof(argv[++i]);
      if (min > max) {
        mlog<<"--fdi: min > max"<<endl;
        mlog<<Usage.str()<<endl;
        return false;       
      }
      int steps = atoi(argv[++i]);
      if (steps == 0) {
        mlog<<"You need to give a reasonable amount of steps for --fdi!"<<endl;
        mlog<<Usage.str()<<endl;
        return false;
      }
      m_FDI.push_back(min);
      for (int s = 1; s < steps; ++s) m_FDI.push_back(min + s*(max-min)/(steps-1));
			mlog<<"Accepting "<<m_FDI.size()<<" first Compton distance steps: ";
      for (unsigned int i = 0; i < m_FDI.size(); ++i) {
        mlog<<m_FDI[i]<<"  ";
      }
      mlog<<endl;
    } else if (Option == "--bra") {
      float min = atof(argv[++i]);
      float max = atof(argv[++i]);
      if (min > max) {
        mlog<<"--bra: min > max"<<endl;
        mlog<<Usage.str()<<endl;
        return false;       
      }
      int steps = atoi(argv[++i]);
      if (steps == 0) {
        mlog<<"You need to give a reasonable amount of steps for --bra!"<<endl;
        mlog<<Usage.str()<<endl;
        return false;
      }
      m_BRA.push_back(min);
      for (int s = 1; s < steps; ++s) m_BRA.push_back(min + s*(max-min)/(steps-1));
			mlog<<"Accepting "<<m_BRA.size()<<" beam radii steps: ";
      for (unsigned int i = 0; i < m_BRA.size(); ++i) {
        mlog<<m_BRA[i]<<"  ";
      }
      mlog<<endl;
    } else if (Option == "--bde") {
      float min = atof(argv[++i]);
      float max = atof(argv[++i]);
      if (min > max) {
        mlog<<"--bde: min > max"<<endl;
        mlog<<Usage.str()<<endl;
        return false;       
      }
      int steps = atoi(argv[++i]);
      if (steps == 0) {
        mlog<<"You need to give a reasonable amount of steps for --bde!"<<endl;
        mlog<<Usage.str()<<endl;
        return false;
      }
      m_BDE.push_back(min);
      for (int s = 1; s < steps; ++s) m_BDE.push_back(min + s*(max-min)/(steps-1));
			mlog<<"Accepting "<<m_BDE.size()<<" beam depth steps: ";
      for (unsigned int i = 0; i < m_BDE.size(); ++i) {
        mlog<<m_BDE[i]<<"  ";
      }
      mlog<<endl;
    } else if (Option == "--phi") {
      float min = atof(argv[++i]);
      float max = atof(argv[++i]);
      if (min > max) {
        mlog<<"--phi: min > max"<<endl;
        mlog<<Usage.str()<<endl;
        return false;       
      }
      int steps = atoi(argv[++i]);
      if (steps == 0) {
        mlog<<"You need to give a reasonable amount of steps for --phi!"<<endl;
        mlog<<Usage.str()<<endl;
        return false;
      }
      m_Phi.push_back(min);
      for (int s = 1; s < steps; ++s) m_Phi.push_back(min + s*(max-min)/(steps-1));
			mlog<<"Accepting "<<m_Phi.size()<<" Compton scatter angle steps: ";
      for (unsigned int i = 0; i < m_Phi.size(); ++i) {
        mlog<<m_Phi[i]<<"  ";
      }
      mlog<<endl;
    } else if (Option == "--the") {
      float min = atof(argv[++i]);
      float max = atof(argv[++i]);
      if (min > max) {
        mlog<<"--the: min > max"<<endl;
        mlog<<Usage.str()<<endl;
        return false;       
      }
      int steps = atoi(argv[++i]);
      if (steps == 0) {
        mlog<<"You need to give a reasonable amount of steps for --phi!"<<endl;
        mlog<<Usage.str()<<endl;
        return false;
      }
      m_The.push_back(min);
      for (int s = 1; s < steps; ++s) m_The.push_back(min + s*(max-min)/(steps-1));
			mlog<<"Accepting "<<m_The.size()<<" total scatter angle steps: ";
      for (unsigned int i = 0; i < m_The.size(); ++i) {
        mlog<<m_The[i]<<"  ";
      }
      mlog<<endl;
    } else if (Option == "--cqf") {
      float min = atof(argv[++i]);
      float max = atof(argv[++i]);
      if (min > max) {
        mlog<<"--cqf: min > max"<<endl;
        mlog<<Usage.str()<<endl;
        return false;       
      }
      int steps = atoi(argv[++i]);
      if (steps == 0) {
        mlog<<"You need to give a reasonable amount of steps for --cqf!"<<endl;
        mlog<<Usage.str()<<endl;
        return false;
      }
      m_CQF.push_back(min);
      for (int s = 1; s < steps; ++s) m_CQF.push_back(min + s*(max-min)/(steps-1));
			mlog<<"Accepting "<<m_CQF.size()<<" Compton quality factor steps: ";
      for (unsigned int i = 0; i < m_CQF.size(); ++i) {
        mlog<<m_CQF[i]<<"  ";
      }
      mlog<<endl;
    } else if (Option == "--tqf") {
      float min = atof(argv[++i]);
      float max = atof(argv[++i]);
      if (min > max) {
        mlog<<"--tqf: min > max"<<endl;
        mlog<<Usage.str()<<endl;
        return false;       
      }
      int steps = atoi(argv[++i]);
      if (steps == 0) {
        mlog<<"You need to give a reasonable amount of steps for --tqf!"<<endl;
        mlog<<Usage.str()<<endl;
        return false;
      }
      m_TQF.push_back(min);
      for (int s = 1; s < steps; ++s) m_TQF.push_back(min + s*(max-min)/(steps-1));
			mlog<<"Accepting "<<m_TQF.size()<<" Track quality factor steps: ";
      for (unsigned int i = 0; i < m_TQF.size(); ++i) {
        mlog<<m_TQF[i]<<"  ";
      }
      mlog<<endl;
    } else if (Option == "--ptheta") {
      float min = atof(argv[++i]);
      float max = atof(argv[++i]);
      if (min > max) {
        mlog<<"--ptheta: min > max"<<endl;
        mlog<<Usage.str()<<endl;
        return false;       
      }
      int steps = atoi(argv[++i]);
      if (steps == 0) {
        mlog<<"You need to give a reasonable amount of steps for --ptheta!"<<endl;
        mlog<<Usage.str()<<endl;
        return false;
      }
      m_PosTheta.push_back(min);
      for (int s = 1; s < steps; ++s) m_PosTheta.push_back(min + s*(max-min)/(steps-1));
			mlog<<"Accepting "<<m_PosTheta.size()<<" theta position steps: ";
      for (unsigned int i = 0; i < m_PosTheta.size(); ++i) {
        mlog<<m_PosTheta[i]<<"  ";
      }
      mlog<<endl;
    } else if (Option == "--pphi") {
      float min = atof(argv[++i]);
      float max = atof(argv[++i]);
      if (min > max) {
        mlog<<"--pphi: min > max"<<endl;
        mlog<<Usage.str()<<endl;
        return false;       
      }
      int steps = atoi(argv[++i]);
      if (steps == 0) {
        mlog<<"You need to give a reasonable amount of steps for --pphi!"<<endl;
        mlog<<Usage.str()<<endl;
        return false;
      }
      m_PosPhi.push_back(min);
      for (int s = 1; s < steps; ++s) m_PosPhi.push_back(min + s*(max-min)/(steps-1));
			mlog<<"Accepting "<<m_PosPhi.size()<<" phi position steps: ";
      for (unsigned int i = 0; i < m_PosPhi.size(); ++i) {
        mlog<<m_PosPhi[i]<<"  ";
      }
      mlog<<endl;
    } else if (Option == "--arm") {
      float min = atof(argv[++i]);
      float max = atof(argv[++i]);
      if (min > max) {
        mlog<<"--arm: min > max"<<endl;
        mlog<<Usage.str()<<endl;
        return false;       
      }
      int steps = atoi(argv[++i]);
      if (steps == 0) {
        mlog<<"You need to give a reasonable amount of steps for --arm!"<<endl;
        mlog<<Usage.str()<<endl;
        return false;
      }
      m_ARMorRadius.push_back(min);
      for (int s = 1; s < steps; ++s) m_ARMorRadius.push_back(min + s*(max-min)/(steps-1));
			mlog<<"Accepting "<<m_ARMorRadius.size()<<" ARM steps: ";
      for (unsigned int i = 0; i < m_ARMorRadius.size(); ++i) {
        mlog<<m_ARMorRadius[i]<<"  ";
      }
      mlog<<endl;
    } else if (Option == "--spd") {
      float min = atof(argv[++i]);
      float max = atof(argv[++i]);
      if (min > max) {
        mlog<<"--spd: min > max"<<endl;
        mlog<<Usage.str()<<endl;
        return false;       
      }
      int steps = atoi(argv[++i]);
      if (steps == 0) {
        mlog<<"You need to give a reasonable amount of steps for --spd!"<<endl;
        mlog<<Usage.str()<<endl;
        return false;
      }
      m_SPD.push_back(min);
      for (int s = 1; s < steps; ++s) m_SPD.push_back(min + s*(max-min)/(steps-1));
			mlog<<"Accepting "<<m_SPD.size()<<" SPD steps: ";
      for (unsigned int i = 0; i < m_SPD.size(); ++i) {
        mlog<<m_SPD[i]<<"  ";
      }
      mlog<<endl;
    } else if (Option == "--pop") {
      float min = atof(argv[++i]);
      float max = atof(argv[++i]);
      if (min > max) {
        mlog<<"--pop: min > max"<<endl;
        mlog<<Usage.str()<<endl;
        return false;       
      }
      int steps = atoi(argv[++i]);
      if (steps == 0) {
        mlog<<"You need to give a reasonable amount of steps for --pop!"<<endl;
        mlog<<Usage.str()<<endl;
        return false;
      }
      m_POP.push_back(min);
      for (int s = 1; s < steps; ++s) m_POP.push_back(min + s*(max-min)/(steps-1));
			mlog<<"Accepting "<<m_POP.size()<<" POP steps: ";
      for (unsigned int i = 0; i < m_POP.size(); ++i) {
        mlog<<m_POP[i]<<"  ";
      }
      mlog<<endl;
    } else if (Option == "--idp") {
      float min = atof(argv[++i]);
      float max = atof(argv[++i]);
      if (min > max) {
        mlog<<"--idp: min > max"<<endl;
        mlog<<Usage.str()<<endl;
        return false;       
      }
      int steps = atoi(argv[++i]);
      if (steps == 0) {
        mlog<<"You need to give a reasonable amount of steps for --idp!"<<endl;
        mlog<<Usage.str()<<endl;
        return false;
      }
      m_IDP.push_back(min);
      for (int s = 1; s < steps; ++s) m_IDP.push_back(min + s*(max-min)/(steps-1));
			mlog<<"Accepting "<<m_IDP.size()<<" IDP steps: ";
      for (unsigned int i = 0; i < m_IDP.size(); ++i) {
        mlog<<m_IDP[i]<<"  ";
      }
      mlog<<endl;
    } else if (Option == "--egy") {
      if (m_SourcePowerLaw.size() > 0) {
        mlog<<"--egy: You only can use either -k or -s & --egy!!"<<m_SourcePowerLaw<<endl;
        mlog<<Usage.str()<<endl;
        return false;               
      }

      float mean = atof(argv[++i]);
      float min = atof(argv[++i]);
      float max = atof(argv[++i]);
      if (min > max) {
        mlog<<"--egy: min > max"<<endl;
        mlog<<Usage.str()<<endl;
        return false;       
      }
      int steps = atoi(argv[++i]);
      if (steps == 0) {
        mlog<<"You need to give a reasonable amount of steps for --egy!"<<endl;
        mlog<<Usage.str()<<endl;
        return false;
      }
      m_EnergyMax.push_back(mean + min);
      m_EnergyMin.push_back(mean - min);
      for (int s = 1; s < steps; ++s) {
        m_EnergyMax.push_back(mean + min + s*(max-min)/(steps-1));
        m_EnergyMin.push_back(mean - min - s*(max-min)/(steps-1));
      }
			mlog<<"Accepting "<<m_EnergyMin.size()<<" Energy steps: ";
      for (unsigned int i = 0; i < m_EnergyMax.size(); ++i) {
        mlog<<"("<<m_EnergyMin[i]<<"-"<<m_EnergyMax[i]<<")  ";
      }
      mlog<<endl;
    } else if (Option == "--contegy") {
      float min = atof(argv[++i]);
      float max = atof(argv[++i]);
      if (min >+ max) {
        mlog<<"--contegy: min >= max"<<endl;
        mlog<<Usage.str()<<endl;
        return false;       
      }
      m_EnergyMax.push_back(max);
      m_EnergyMin.push_back(min);
			mlog<<"Accepting continuum energy interval: "<<m_EnergyMin.back()<<"-"<<m_EnergyMax.back()<<endl;
		} else {
			mlog<<"Error: Unknown option \""<<Option<<"\"!"<<endl;
			mlog<<Usage.str()<<endl;
			return false;
		}
  }

  if (ConfigurationFile == g_StringNotDefined) {
    mlog<<"You need to give a configuration file!"<<endl;
    mlog<<Usage.str()<<endl;
    return false;
  }

  if (m_SourceFile.size() == 0) {
    mlog<<"You need to give a source file!"<<endl;
    mlog<<Usage.str()<<endl;
    return false;
  }

  if (m_ModeSourceExtension == s_ModeExtended) {
    if (m_ARMorRadius.size() > 0) {
      mlog<<"You cannot define an ARM radius in extended source mode --- overriding"<<endl;
      m_ARMorRadius.clear();
    }
    // Define some default area bins, corresponding to 0.25 deg^2, 1.0 deg^2, 4.0 deg^2, etc.
    double DefaultRadius = sqrt(c_Rad*c_Rad/c_Pi);
    //m_ARMorRadius.push_back(0.25*DefaultRadius*c_Deg);
    //m_ARMorRadius.push_back(0.50*DefaultRadius*c_Deg);
    //m_ARMorRadius.push_back(1.00*DefaultRadius*c_Deg);
    //m_ARMorRadius.push_back(2.00*DefaultRadius*c_Deg);
    m_ARMorRadius.push_back(4.00*DefaultRadius*c_Deg);
    //m_ARMorRadius.push_back(8.00*DefaultRadius*c_Deg);
  }


  // Do a source file position check:
  if (m_SourceTheta.size() > 1) {
    m_ModeObservation = s_ModeAllSkyObs;

    // Check if we can find a source file for each test position:
    for (unsigned int x = 0; x < m_PosTheta.size(); ++x) {
      for (unsigned int y = 0; y < m_PosPhi.size(); ++y) {
        if (FindSourceIndex(x, y) == numeric_limits<unsigned int>::max()) {
          mlog<<"Cannot find a source file for position theta="<<m_PosTheta[x]<<", phi="<<m_PosPhi[y]<<"."<<endl;
          mlog<<"Make sure you include such a source file."<<endl;
          mlog<<Usage.str()<<endl;
          return false;      
        }
      }
    }
  } else {
    m_ModeObservation = s_ModeSingleObs;
  }

  if (m_BackgroundFiles.size() == 0) {
    mlog<<"You need to give at least one background file!"<<endl;
    mlog<<Usage.str()<<endl;
    return false;
  }

  if (m_PosTheta.size() == 0) {
    mlog<<"You need to give at least one theta position!"<<endl;
    mlog<<Usage.str()<<endl;
    return false;
  }

  if (m_PosPhi.size() == 0) {
    mlog<<"You need to give at least one phi position!"<<endl;
    mlog<<Usage.str()<<endl;
    return false;
  }

  if (m_CSLMin.size() == 0) {
    mlog<<"You need to define a Compton sequence length!"<<endl;
    mlog<<Usage.str()<<endl;
    return false;
  }

  if (m_TSLMin.size() == 0) {
    m_TSLMin.push_back(1);
    m_TSLMax.push_back(numeric_limits<unsigned int>::max());
    mlog<<"No Track sequence length given - defaulting to open: 1..inf - this is absolutely ok if you have no tracks!"<<endl;
  }

  if (m_ARMorRadius.size() == 0) {
    mlog<<"You need to define an ARM (point source mode) or radius (extended source mode)!"<<endl;
    mlog<<Usage.str()<<endl;
    return false;
  }

  if (m_FDI.size() == 0) {
    m_FDI.push_back(0);
    mlog<<"No first Compton distance given - defaulting to open: 0..inf"<<endl;
  }

  if (m_BRA.size() == 0) {
    m_BRA.push_back(0.01*numeric_limits<float>::max());
    mlog<<"No beam radius given - defaulting to open: 0..inf"<<endl;
  }

  if (m_BDE.size() == 0) {
    m_BDE.push_back(0.01*numeric_limits<float>::max());
    mlog<<"No beam depth given - defaulting to open: 0..inf"<<endl;
  }

  if (m_SPD.size() == 0) {
    m_SPD.push_back(180);
    mlog<<"No SPD given - defaulting to open: 0..180 - this is absolutely ok if you have no tracks!"<<endl;
  }

  if (m_IDP.size() == 0) {
    m_IDP.push_back(100000);
    mlog<<"No IDP given - defaulting to open: 0..100000 - this is absolutely ok if you have no pairs!"<<endl;
  }

  if (m_POP.size() == 0) {
    m_POP.push_back(180);
    mlog<<"No POP given - defaulting to open: 0..180 - this is absolutely ok if you have no pairs!"<<endl;
  }

  if (m_Phi.size() == 0) {
    mlog<<"You need to define a Phi selection!"<<endl;
    mlog<<Usage.str()<<endl;
    return false;
  }

  if (m_The.size() == 0) {
    mlog<<"No total scatter angle given - defaulting to open: 0..180 - this is absolutely ok if you have no tracks!"<<endl;
    m_The.push_back(180);
  }

  if (m_ModeSpectrum == s_ModeContinuum) {
    for (unsigned int x = 0; x < m_SourceFile.size(); ++x) {
      // Check if Emin is always ok:
      for (unsigned int e = 0; e < m_EnergyMin.size(); ++e) {
        if (m_SourcePowerLawEmin[x] > m_EnergyMin[e]) {
          m_EnergyMin[e] = m_SourcePowerLawEmin[x];
        }
      }
      // Check if Emax is always ok:
      for (unsigned int e = 0; e < m_EnergyMax.size(); ++e) {
        if (m_SourcePowerLawEmax[x] < m_EnergyMax[e]) {
          m_EnergyMax[e] = m_SourcePowerLawEmax[x];
        }
      }

      // Calculate start photons per bin:
      double Constant = m_SourceStartPhotons[x]*(m_SourcePowerLaw[x]+1)/(pow(m_SourcePowerLawEmax[x], m_SourcePowerLaw[x]+1) - 
                                                                         pow(m_SourcePowerLawEmin[x], m_SourcePowerLaw[x]+1));
      mlog<<"Constant: "<<Constant<<" of "<<m_SourceFile[x]<<endl;
      for (unsigned int e = 0; e < m_EnergyMin.size(); ++e) {
        m_SourceStartPhotonsContinuumSensitivityBin[x].push_back(int(Constant/(m_SourcePowerLaw[x]+1)*(pow(m_EnergyMax[e], m_SourcePowerLaw[x]+1) - 
                                                                                                    pow(m_EnergyMin[e], m_SourcePowerLaw[x]+1))));
        mlog<<"Continuum bin: "<<m_EnergyMin[e]<<" - "<<m_EnergyMax[e]<<": Counts: "<<m_SourceStartPhotonsContinuumSensitivityBin[x].back()<<endl;
      }
      
      mlog<<"Sanity check: min="<<m_SourcePowerLawEmin[x]<<" max="<<m_SourcePowerLawEmax[x]<<" counts="
          <<int(Constant/(m_SourcePowerLaw[x]+1)*(pow(m_SourcePowerLawEmax[x], m_SourcePowerLaw[x]+1) - pow(m_SourcePowerLawEmin[x], m_SourcePowerLaw[x]+1)))<<endl;
    }
  }

  if (m_EnergyMin.size() == 0) {
    mlog<<"You need to define an energy selection!"<<endl;
    mlog<<Usage.str()<<endl;
    return false;
  }

  if (m_CQF.size() == 0) {
    mlog<<"You need to define a Compton quality factor selection!"<<endl;
    mlog<<Usage.str()<<endl;
    return false;
  }

  if (m_TQF.size() == 0) {
    mlog<<"No track quality factor given - defaulting to open: 0..very large!"<<endl;
    m_TQF.push_back(numeric_limits<float>::max()/2);
  }

  if (m_EHC.size() == 0) {
    mlog<<"You need to define an earth horizon cut !"<<endl;
    mlog<<Usage.str()<<endl;
    return false;
  }

  if (m_EHC.size() > 0 && m_EHC[0] >= 180) {
    mlog<<"You deselected all events through EHC cut!"<<endl;
    mlog<<Usage.str()<<endl;
    return false;
  }

  if (m_EHP.size() == 0) {
    mlog<<"No earth horizon probability given - defaulting to open: 0..1 - this is absolutely ok if you only want to do the intersection based earth horizon cut!"<<endl;
    m_EHP.push_back(180);
  }

  if (m_EnergyWindowMin > 0 && m_EnergyWindowMax > 0) {
    if (m_EnergyWindowMin > m_EnergyMin.back() || m_EnergyWindowMax < m_EnergyMax.back()) {
      mlog<<"Your user defined energy window for background accumulation needs to be larger than the scanned energy range!"<<endl;
      mlog<<Usage.str()<<endl;
      return false;      
    }
  }

  if (GeometryFile == g_StringNotDefined) {
    mlog<<"You need to define a geometry!"<<endl;
    mlog<<Usage.str()<<endl;
    return false;
  }

  // Load geometry:
  if (m_Geometry.ScanSetupFile(GeometryFile, false, true) == true) {
    cout<<"Geometry "<<m_Geometry.GetName()<<" loaded!"<<endl;
  } else {
    cout<<"Loading of geometry "<<m_Geometry.GetName()<<" failed!!"<<endl;
    return false;
  }  

  // Load the configuration file and initialize the event selector:
  MSettingsMimrec Data;
  Data.Read(ConfigurationFile);
  m_EventSelector.SetSettings(&Data);
  m_EventSelector.SetGeometry(&m_Geometry);

  return true;
}


/******************************************************************************
 * Do whatever analysis is necessary
 */
unsigned int SensitivityOptimizer::FindSourceIndex(unsigned int theta, unsigned int phi)
{
  double Theta = m_PosTheta[theta];
  double Phi = m_PosPhi[phi];

  for (unsigned int i = 0; i < m_SourceTheta.size(); ++i) {
    // float comparison... --- make sure you never did any maths on the values...
    if (fabs(m_SourceTheta[i] - Theta) < 1E-5 && fabs(m_SourcePhi[i] - Phi) < 1E-5) {
      return i;
    }
  }

  cout<<"Error: No source file for angles: theta="<<Theta<<", phi="<<Phi<<endl;
  return numeric_limits<unsigned int>::max();
}


/******************************************************************************
 * Do whatever analysis is necessary
 */
unsigned int SensitivityOptimizer::GetPhotoSize() 
{
  return m_EnergyMin.size()*m_BRA.size()*m_BDE.size()*m_PosTheta.size()*m_PosPhi.size();
}


/******************************************************************************
 * Do whatever analysis is necessary
 */
unsigned int SensitivityOptimizer::GetPhotoIndex(unsigned int BDE, unsigned int BRA, unsigned int EnergyMin, unsigned int PosTheta, unsigned int PosPhi)
{
  unsigned int Index = PosPhi + m_PosPhi.size()*(PosTheta + m_PosTheta.size()*(EnergyMin + m_EnergyMin.size()*(BRA + m_BRA.size()*(BDE))));
  if (Index >= GetPhotoSize()) {
    cout<<"Photo index out of bounds: "<<Index<<" >= "<<GetPhotoSize()<<endl;
    return 0;
  }

  return Index;
}


/******************************************************************************
 * Do whatever analysis is necessary
 */
unsigned int SensitivityOptimizer::GetTrackedComptonSize() 
{
  return m_FDI.size()*m_CSLMin.size()*m_TSLMin.size()*m_ARMorRadius.size()*m_SPD.size()*m_The.size()*m_Phi.size()*m_EnergyMin.size()*m_EHC.size()*m_EHP.size()*m_TQF.size()*m_CQF.size()*m_BRA.size()*m_BDE.size()*m_PosTheta.size()*m_PosPhi.size();
}


/******************************************************************************
 * Do whatever analysis is necessary
 */
unsigned int SensitivityOptimizer::GetUntrackedComptonSize() 
{
  return m_FDI.size()*m_CSLMin.size()*m_ARMorRadius.size()*m_Phi.size()*m_EnergyMin.size()*m_EHC.size()*m_EHP.size()*m_CQF.size()*m_BRA.size()*m_BDE.size()*m_PosTheta.size()*m_PosPhi.size();
}


/******************************************************************************
 * Do whatever analysis is necessary
 */
unsigned int SensitivityOptimizer::GetTrackedComptonIndex(unsigned int BDE, unsigned int BRA, unsigned int CQF, unsigned int TQF,  unsigned int EHP, unsigned int EHC, unsigned int EnergyMin, unsigned int Phi, unsigned int The, unsigned int SPD, unsigned int ARM, unsigned int TSL, unsigned int CSL, unsigned int FDI, unsigned int PosTheta, unsigned int PosPhi)

{
  unsigned int Index =  PosPhi + m_PosPhi.size()*(PosTheta + m_PosTheta.size()*(FDI + m_FDI.size()*(CSL + m_CSLMin.size()*(TSL + m_TSLMin.size()*(ARM + m_ARMorRadius.size()*(SPD + m_SPD.size()*(The + m_The.size()*(Phi + m_Phi.size()*(EnergyMin + m_EnergyMin.size()*(EHC + m_EHC.size()*(EHP + m_EHP.size()*(TQF + m_TQF.size()*(CQF + m_CQF.size()*(BRA + m_BRA.size()*(BDE)))))))))))))));
  if (Index >= GetTrackedComptonSize()) {
    cout<<"Tracked Compton index out of bounds: "<<Index<<" >= "<<GetTrackedComptonSize()<<endl;
    return 0;
  }

  return Index;
}


/******************************************************************************
 * Do whatever analysis is necessary
 */
unsigned int SensitivityOptimizer::GetUntrackedComptonIndex(unsigned int BDE, unsigned int BRA, unsigned int CQF, unsigned int EHP, unsigned int EHC, unsigned int EnergyMin, unsigned int Phi, unsigned int ARM, unsigned int CSL, unsigned int FDI, unsigned int PosTheta, unsigned int PosPhi)

{
  unsigned int Index =  PosPhi + m_PosPhi.size()*(PosTheta + m_PosTheta.size()*(FDI + m_FDI.size()*(CSL + m_CSLMin.size()*(ARM + m_ARMorRadius.size()*(Phi + m_Phi.size()*(EnergyMin + m_EnergyMin.size()*(EHC + m_EHC.size()*(EHP + m_EHP.size()*(CQF + m_CQF.size()*(BRA + m_BRA.size()*(BDE)))))))))));
  if (Index >= GetUntrackedComptonSize()) {
    cout<<"Untracked Compton index out of bounds: "<<Index<<" >= "<<GetUntrackedComptonSize()<<endl;
    return 0;
  }

  return Index;
}


/******************************************************************************
 * Do whatever analysis is necessary
 */
unsigned int SensitivityOptimizer::GetPairSize()
{
  return m_TSLMin.size()*m_ARMorRadius.size()*m_EnergyMin.size()*m_EHC.size()*m_EHP.size()*m_TQF.size()*m_BRA.size()*m_BDE.size()*m_POP.size()*m_IDP.size()*m_PosTheta.size()*m_PosPhi.size();
}


/******************************************************************************
 * Do whatever analysis is necessary
 */
unsigned int SensitivityOptimizer::GetPairIndex(unsigned int IDP, unsigned int POP, unsigned int BDE, unsigned int BRA, unsigned int TQF, unsigned int EHP, unsigned int EHC, unsigned int EnergyMin, unsigned int ARM, unsigned int TSL, unsigned int PosTheta, unsigned int PosPhi)
{
  unsigned int Index = PosPhi + m_PosPhi.size()*(PosTheta + m_PosTheta.size()*(TSL + m_TSLMin.size()*(ARM + m_ARMorRadius.size()*(EnergyMin + m_EnergyMin.size()*(EHC + m_EHC.size()*(EHP + m_EHP.size()*(TQF + m_TQF.size()*(BRA + m_BRA.size()*(BDE + m_BDE.size()*(POP + m_POP.size()*(IDP)))))))))));
  if (Index >= GetPairSize()) {
    cout<<"Pair index out of bounds: "<<Index<<" >= "<<GetPairSize()<<endl;
    massert(false);
  }

  return Index;
}


/******************************************************************************
 * Do whatever analysis is necessary
 */
bool SensitivityOptimizer::Analyze()
{
  if (m_Interrupt == true) return false;

  // Index belonging to the source file
  int SourceIndex;

  // Photo's
  vector<SensitivityPoint> Photo_Final(GetPhotoSize());

  // Untracked Compton's
  vector<SensitivityPoint> UntrackedCompton_Final(GetUntrackedComptonSize());

  // Tracked Compton's
  vector<SensitivityPoint> TrackedCompton_Final(GetTrackedComptonSize());
  
  //Pair's
  vector<SensitivityPoint> Pair_Final(GetPairSize());
  
  // Initialize:
  mlog<<"Starting initialization..."<<endl;
  
  unsigned int c_max = m_BDE.size();
  unsigned int b_max = m_BRA.size();
  unsigned int q_max = m_CQF.size();
  unsigned int k_max = m_TQF.size();
  unsigned int r_max = m_EHP.size();
  unsigned int h_max = m_EHC.size();
  unsigned int e_max = m_EnergyMin.size();
  unsigned int p_max = m_Phi.size();
  unsigned int t_max = m_The.size();
  unsigned int s_max = m_SPD.size();
  unsigned int a_max = m_ARMorRadius.size();
  unsigned int u_max = m_TSLMin.size();
  unsigned int l_max = m_CSLMin.size();
  unsigned int f_max = m_FDI.size();
  unsigned int d_max = m_IDP.size();
  unsigned int o_max = m_POP.size();
  unsigned int x_max = m_PosTheta.size();
  unsigned int y_max = m_PosPhi.size();

  cout<<"Bins photo: "<<GetPhotoSize()<<endl;
  cout<<"Bins tracked Compton: "<<GetTrackedComptonSize()<<endl;
  cout<<"Bins untracked Compton: "<<GetUntrackedComptonSize()<<endl;
  cout<<"Bins pair: "<<GetPairSize()<<endl;


  // Photo:
  for (unsigned int c = 0; c < c_max; ++c) {
    //cout<<"c: "<<c<<":"<<c_max<<endl;
    for (unsigned int b = 0; b < b_max; ++b) {
      //cout<<"b: "<<b<<":"<<b_max<<endl;
      for (unsigned int e = 0; e < e_max; ++e) {
        for (unsigned int x = 0; x < x_max; ++x) {
          //cout<<"x: "<<x<<":"<<x_max<<endl;
          for (unsigned int y = 0; y < y_max; ++y) {
            //cout<<"y: "<<y<<":"<<y_max<<endl;

            Photo_Final[GetPhotoIndex(c, b, e, x, y)].SetObservationTime(m_ObservationTime);
            if (m_ModeSourceExtension == s_ModePointSource) {
              SourceIndex = FindSourceIndex(x, y);
              if (m_ModeSpectrum == s_ModeContinuum) {
                Photo_Final[GetPhotoIndex(c, b, e, x, y)].SetStartAreaPerPhotons(m_SourceStartArea[SourceIndex]/m_SourceStartPhotonsContinuumSensitivityBin[SourceIndex][e]);
              } else if (m_ModeSpectrum == s_ModeLine) {
                Photo_Final[GetPhotoIndex(c, b, e, x, y)].SetStartAreaPerPhotons(m_SourceStartArea[SourceIndex]/m_SourceStartPhotons[SourceIndex]);
              } 
            } else if (m_ModeSourceExtension == s_ModeExtended) {
              SourceIndex = 0;
              double Omega = 2*c_Pi;
              Photo_Final[GetPhotoIndex(c, b, e, x, y)].SetStartAreaPerPhotons(1.0/m_SourceStartFluence[SourceIndex]/Omega);
            }
            MEventSelector S = m_EventSelector;
            S.UsePhotos(true);
            S.UseComptons(false);
            S.UsePairs(false);
            S.SetFirstTotalEnergy(m_EnergyMin[e], m_EnergyMax[e]);  
            S.SetBeamRadius(m_BRA[b]);
            S.SetBeamDepth(m_BDE[c]);
            
            Photo_Final[GetPhotoIndex(c, b, e, x, y)].SetEventSelector(S);
            for (unsigned int bf = 0; bf < m_BackgroundFiles.size(); ++bf) {
              Photo_Final[GetPhotoIndex(c, b, e, x, y)].SetBackgroundTime(bf, m_BackgroundTimes[bf]);
              Photo_Final[GetPhotoIndex(c, b, e, x, y)].SetBackgroundName(bf, m_BackgroundFiles[bf]);
            }
            if (m_ModeSpectrum == s_ModeLine && m_EnergyWindowMin > 0 && m_EnergyWindowMax > 0) {
              Photo_Final[GetPhotoIndex(c, b, e, x, y)].SetLargeEnergyWindow(m_EnergyMin[e], m_EnergyMax[e], m_EnergyWindowMin, m_EnergyWindowMax);
            }
            Photo_Final[GetPhotoIndex(c, b, e, x, y)].UseComplexEquation(m_ComplexEquation);
          }
        }
      }
    }
  }

  // Tracked Compton:
  for (unsigned int c = 0; c < c_max; ++c) {
    //cout<<"c: "<<c<<":"<<c_max<<endl;
    for (unsigned int b = 0; b < b_max; ++b) {
      //cout<<"b: "<<b<<":"<<b_max<<endl;
      for (unsigned int q = 0; q < q_max; ++q) {
        //cout<<"q: "<<q<<":"<<q_max<<endl;
        for (unsigned int k = 0; k < k_max; ++k) {
          //cout<<"k: "<<k<<":"<<k_max<<endl;
          for (unsigned int r = 0; r < r_max; ++r) {
            //cout<<"r: "<<r<<":"<<r_max<<endl;
            for (unsigned int h = 0; h < h_max; ++h) {
              //cout<<"h: "<<h<<":"<<h_max<<endl;
              MEarthHorizon EH;
              EH.SetEarthHorizon(MVector(0, 0, -1E+20), m_EHC[h]*c_Rad);
              if (m_EHPFile != g_StringNotDefined) {
                EH.SetProbabilityTest(m_EHP[r], m_EHPFile);
              } else {
                EH.SetIntersectionTest();
              }
              for (unsigned int e = 0; e < e_max; ++e) {
                //cout<<"e: "<<e<<":"<<e_max<<endl;
                for (unsigned int p = 0; p < p_max; ++p) {
                  //cout<<"p: "<<p<<":"<<p_max<<endl;
                  for (unsigned int t = 0; t < t_max; ++t) {
                    //cout<<"t: "<<t<<":"<<t_max<<endl;
                    for (unsigned int s = 0; s < s_max; ++s) {
                      //cout<<"s: "<<s<<":"<<s_max<<endl;
                      for (unsigned int a = 0; a < a_max; ++a) {
                        //cout<<"a: "<<a<<":"<<a_max<<endl;
                        for (unsigned int u = 0; u < u_max; ++u) {
                          //cout<<"u: "<<u<<":"<<u_max<<endl;
                          for (unsigned int l = 0; l < l_max; ++l) {
                            //cout<<"l: "<<l<<":"<<l_max<<endl;
                            for (unsigned int f = 0; f < f_max; ++f) {
                              //cout<<"f: "<<f<<":"<<f_max<<endl;
                              for (unsigned int x = 0; x < x_max; ++x) {
                                //cout<<"x: "<<x<<":"<<x_max<<endl;
                                for (unsigned int y = 0; y < y_max; ++y) {
                                  //cout<<"y: "<<y<<":"<<y_max<<endl;
                                  TrackedCompton_Final[GetTrackedComptonIndex(c, b, q, k, r, h, e, p, t, s, a, u, l, f, x, y)].SetObservationTime(m_ObservationTime);
                                  if (m_ModeSourceExtension == s_ModePointSource) {
                                    SourceIndex = FindSourceIndex(x, y);
                                    if (m_ModeSpectrum == s_ModeContinuum) {
                                      TrackedCompton_Final[GetTrackedComptonIndex(c, b, q, k, r, h, e, p, t, s, a, u, l, f, x, y)].SetStartAreaPerPhotons(m_SourceStartArea[SourceIndex]/m_SourceStartPhotonsContinuumSensitivityBin[SourceIndex][e]);
                                    } else if (m_ModeSpectrum == s_ModeLine) {
                                      TrackedCompton_Final[GetTrackedComptonIndex(c, b, q, k, r, h, e, p, t, s, a, u, l, f, x, y)].SetStartAreaPerPhotons(m_SourceStartArea[SourceIndex]/m_SourceStartPhotons[SourceIndex]);
                                    }
                                  } else if (m_ModeSourceExtension == s_ModeExtended) {
                                    SourceIndex = 0;
                                    double Omega = 2*c_Pi*(1-cos(m_ARMorRadius[a]*c_Rad));
                                    cout<<m_SourceStartFluence[0]<<":"<<Omega<<endl;
                                    TrackedCompton_Final[GetTrackedComptonIndex(c, b, q, k, r, h, e, p, t, s, a, u, l, f, x, y)].SetStartAreaPerPhotons(1.0/m_SourceStartFluence[SourceIndex]/Omega);
                                  }
                                  MEventSelector S = m_EventSelector;
                                  S.UsePhotos(false);
                                  S.UseComptons(true);  
                                  S.UseTrackedComptons(true);
                                  S.UseNotTrackedComptons(false);
                                  S.UsePairs(false);
                                  S.SetComptonQualityFactor(0, m_CQF[q]);
                                  S.SetTrackQualityFactor(0, m_TQF[k]);
                                  S.SetEarthHorizonCut(EH);
                                  S.SetFirstTotalEnergy(m_EnergyMin[e], m_EnergyMax[e]);  
                                  S.SetComptonAngle(0, m_Phi[p]);
                                  S.SetThetaDeviationMax(m_The[t]);
                                  S.SetSequenceLength(m_CSLMin[l], m_CSLMax[l]);
                                  S.SetTrackLength(m_TSLMin[u], m_TSLMax[u]);
                                
                                  double xx = m_PosTheta[x]; 
                                  double xy = m_PosPhi[y];
                                  double xz = 1000000000.0;
                                  MMath::SphericToCartesean(xx, xy, xz);
                                  MVector Position(xx, xy, xz);

                                  S.SetSourceWindow(true, Position);
                                  S.SetSourceARM(0, m_ARMorRadius[a]);
                                  S.SetSourceSPD(0, m_SPD[s]);
                                  S.SetBeamRadius(m_BRA[b]);
                                  S.SetBeamDepth(m_BDE[c]);
                                  S.SetFirstDistance(m_FDI[f], c_FarAway);
                                  
                                  TrackedCompton_Final[GetTrackedComptonIndex(c, b, q, k, r, h, e, p, t, s, a, u, l, f, x, y)].SetEventSelector(S);
                                  for (unsigned int bf = 0; bf < m_BackgroundFiles.size(); ++bf) {
                                    TrackedCompton_Final[GetTrackedComptonIndex(c, b, q, k, r, h, e, p, t, s, a, u, l, f, x, y)].SetBackgroundTime(bf, m_BackgroundTimes[bf]);
                                    TrackedCompton_Final[GetTrackedComptonIndex(c, b, q, k, r, h, e, p, t, s, a, u, l, f, x, y)].SetBackgroundName(bf, m_BackgroundFiles[bf]);
                                  }
                                  if (m_ModeSpectrum == s_ModeLine && m_EnergyWindowMin > 0 && m_EnergyWindowMax > 0) {
                                    TrackedCompton_Final[GetTrackedComptonIndex(c, b, q, k, r, h, e, p, t, s, a, u, l, f, x, y)].SetLargeEnergyWindow(m_EnergyMin[e], m_EnergyMax[e], m_EnergyWindowMin, m_EnergyWindowMax);
                                  }
                                  TrackedCompton_Final[GetTrackedComptonIndex(c, b, q, k, r, h, e, p, t, s, a, u, l, f, x, y)].UseComplexEquation(m_ComplexEquation);
                                }
                              }
                            }
                          }
                        }
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }  

  // Untracked Compton:
  for (unsigned int c = 0; c < c_max; ++c) {
    //cout<<"c: "<<c<<":"<<c_max<<endl;
    for (unsigned int b = 0; b < b_max; ++b) {
      //cout<<"b: "<<b<<":"<<b_max<<endl;
      for (unsigned int q = 0; q < q_max; ++q) {
        //cout<<"q: "<<q<<":"<<q_max<<endl;
        for (unsigned int r = 0; r < r_max; ++r) {
          //cout<<"r: "<<r<<":"<<r_max<<endl;
          for (unsigned int h = 0; h < h_max; ++h) {
            //cout<<"h: "<<h<<":"<<h_max<<endl;
            MEarthHorizon EH;
            EH.SetEarthHorizon(MVector(0, 0, -1E+20), m_EHC[h]*c_Rad);
            if (m_EHPFile != g_StringNotDefined) {
              EH.SetProbabilityTest(m_EHP[r], m_EHPFile);
            } else {
              EH.SetIntersectionTest();
            }
            for (unsigned int e = 0; e < e_max; ++e) {
              //cout<<"e: "<<e<<":"<<e_max<<endl;
              for (unsigned int p = 0; p < p_max; ++p) {
                //cout<<"p: "<<p<<":"<<p_max<<endl;
                for (unsigned int a = 0; a < a_max; ++a) {
                  //cout<<"a: "<<a<<":"<<a_max<<endl;
                  for (unsigned int l = 0; l < l_max; ++l) {
                    //cout<<"l: "<<l<<":"<<l_max<<endl;
                    for (unsigned int f = 0; f < f_max; ++f) {
                      //cout<<"f: "<<f<<":"<<f_max<<endl;
                      for (unsigned int x = 0; x < x_max; ++x) {
                        //cout<<"x: "<<x<<":"<<x_max<<endl;
                        for (unsigned int y = 0; y < y_max; ++y) {
                          //cout<<"y: "<<y<<":"<<y_max<<endl;
                          UntrackedCompton_Final[GetUntrackedComptonIndex(c, b, q, r, h, e, p, a, l, f, x, y)].SetObservationTime(m_ObservationTime);
                          if (m_ModeSourceExtension == s_ModePointSource) {
                            SourceIndex = FindSourceIndex(x, y);
                            if (m_ModeSpectrum == s_ModeContinuum) {
                              UntrackedCompton_Final[GetUntrackedComptonIndex(c, b, q, r, h, e, p, a, l, f, x, y)].SetStartAreaPerPhotons(m_SourceStartArea[SourceIndex]/m_SourceStartPhotonsContinuumSensitivityBin[SourceIndex][e]);
                            } else if (m_ModeSpectrum == s_ModeLine) {
                              UntrackedCompton_Final[GetUntrackedComptonIndex(c, b, q, r, h, e, p, a, l, f, x, y)].SetStartAreaPerPhotons(m_SourceStartArea[SourceIndex]/m_SourceStartPhotons[SourceIndex]);
                            }
                          } else if (m_ModeSourceExtension == s_ModeExtended) {
                            SourceIndex = 0;
                            double Omega = 2*c_Pi*(1-cos(m_ARMorRadius[a]*c_Rad));
                            cout<<m_SourceStartFluence[0]<<":"<<Omega<<endl;
                            UntrackedCompton_Final[GetUntrackedComptonIndex(c, b, q, r, h, e, p, a, l, f, x, y)].SetStartAreaPerPhotons(1.0/m_SourceStartFluence[SourceIndex]/Omega);
                          }
                          MEventSelector S = m_EventSelector;
                          S.UsePhotos(false);
                          S.UseComptons(true);
                          S.UseTrackedComptons(false);
                          S.UseNotTrackedComptons(true);
                          S.UsePairs(false);
                          S.SetComptonQualityFactor(0, m_CQF[q]);
                          //S.SetTrackQualityFactor(0, m_TQF[k]);
                          S.SetEarthHorizonCut(EH);
                          S.SetFirstTotalEnergy(m_EnergyMin[e], m_EnergyMax[e]);  
                          S.SetComptonAngle(0, m_Phi[p]);
                          //S.SetThetaDeviationMax(m_The[t]);
                          S.SetSequenceLength(m_CSLMin[l], m_CSLMax[l]);
                          //S.SetTrackLength(m_TSLMin[u], m_TSLMax[u]);
                          
                          double xx = m_PosTheta[x]; 
                          double xy = m_PosPhi[y];
                          double xz = 1000000000.0;
                          MMath::SphericToCartesean(xx, xy, xz);
                          MVector Position(xx, xy, xz);
                          
                          S.SetSourceWindow(true, Position);
                          S.SetSourceARM(0, m_ARMorRadius[a]);
                          //S.SetSourceSPD(0, m_SPD[s]);
                          S.SetBeamRadius(m_BRA[b]);
                          S.SetBeamDepth(m_BDE[c]);
                          S.SetFirstDistance(m_FDI[f], c_FarAway);
                          
                          UntrackedCompton_Final[GetUntrackedComptonIndex(c, b, q, r, h, e, p, a, l, f, x, y)].SetEventSelector(S);
                          for (unsigned int bf = 0; bf < m_BackgroundFiles.size(); ++bf) {
                            UntrackedCompton_Final[GetUntrackedComptonIndex(c, b, q, r, h, e, p, a, l, f, x, y)].SetBackgroundTime(bf, m_BackgroundTimes[bf]);
                            UntrackedCompton_Final[GetUntrackedComptonIndex(c, b, q, r, h, e, p, a, l, f, x, y)].SetBackgroundName(bf, m_BackgroundFiles[bf]);
                          }
                          if (m_ModeSpectrum == s_ModeLine && m_EnergyWindowMin > 0 && m_EnergyWindowMax > 0) {
                            UntrackedCompton_Final[GetUntrackedComptonIndex(c, b, q, r, h, e, p, a, l, f, x, y)].SetLargeEnergyWindow(m_EnergyMin[e], m_EnergyMax[e], m_EnergyWindowMin, m_EnergyWindowMax);
                          }
                          UntrackedCompton_Final[GetUntrackedComptonIndex(c, b, q, r, h, e, p, a, l, f, x, y)].UseComplexEquation(m_ComplexEquation);
                        }
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }

  
  // Pairs:
  for (unsigned int d = 0; d < d_max; ++d) {
    //cout<<"d: "<<d<<":"<<d_max<<endl;
    for (unsigned int o = 0; o < o_max; ++o) {
      //cout<<"o: "<<o<<":"<<o_max<<endl;
      for (unsigned int c = 0; c < c_max; ++c) {
        //cout<<"c: "<<c<<":"<<c_max<<endl;
        for (unsigned int b = 0; b < b_max; ++b) {
          //cout<<"b: "<<b<<":"<<b_max<<endl;
          for (unsigned int k = 0; k < k_max; ++k) {
            //cout<<"k: "<<k<<":"<<k_max<<endl;
            for (unsigned int r = 0; r < r_max; ++r) {
              //cout<<"r: "<<r<<":"<<r_max<<endl;
              for (unsigned int h = 0; h < h_max; ++h) {
                //cout<<"h: "<<h<<":"<<h_max<<endl;
                MEarthHorizon EH;
                EH.SetEarthHorizon(MVector(0, 0, -1E+20), m_EHC[h]*c_Rad);
                if (m_EHPFile != g_StringNotDefined) {
                  EH.SetProbabilityTest(m_EHP[r], m_EHPFile);
                } else {
                  EH.SetIntersectionTest();
                }
                for (unsigned int e = 0; e < e_max; ++e) {
                  //cout<<"e: "<<e<<":"<<e_max<<endl;
                  for (unsigned int a = 0; a < a_max; ++a) {
                    //cout<<"a: "<<a<<":"<<a_max<<endl;
                    for (unsigned int u = 0; u < u_max; ++u) {
                      //cout<<"u: "<<u<<":"<<u_max<<endl;
                      for (unsigned int x = 0; x < x_max; ++x) {
                        //cout<<"x: "<<x<<":"<<x_max<<endl;
                        for (unsigned int y = 0; y < y_max; ++y) {
                          //cout<<"y: "<<y<<":"<<y_max<<endl;
                          Pair_Final[GetPairIndex(d, o, c, b, k, r, h, e, a, u, x, y)].SetObservationTime(m_ObservationTime);
                          if (m_ModeSourceExtension == s_ModePointSource) {
                            SourceIndex = FindSourceIndex(x, y);
                            if (m_ModeSpectrum == s_ModeContinuum) {
                              Pair_Final[GetPairIndex(d, o, c, b, k, r, h, e, a, u, x, y)].SetStartAreaPerPhotons(m_SourceStartArea[SourceIndex]/m_SourceStartPhotonsContinuumSensitivityBin[SourceIndex][e]);
                            } else if (m_ModeSpectrum == s_ModeLine) {
                              Pair_Final[GetPairIndex(d, o, c, b, k, r, h, e, a, u, x, y)].SetStartAreaPerPhotons(m_SourceStartArea[SourceIndex]/m_SourceStartPhotons[SourceIndex]);
                            }                        
                          } else if (m_ModeSourceExtension == s_ModeExtended) {
                            SourceIndex = 0;
                            double Omega = 2*c_Pi*(1-cos(m_ARMorRadius[a]*c_Rad));
                            Pair_Final[GetPairIndex(d, o, c, b, k, r, h, e, a, u, x, y)].SetStartAreaPerPhotons(1.0/m_SourceStartFluence[SourceIndex]/Omega);
                          }
                          MEventSelector S = m_EventSelector;
                          S.UsePhotos(false);
                          S.UseComptons(false);
                          S.UsePairs(true);
                          S.SetTrackQualityFactor(0, m_TQF[k]);
                          S.SetEarthHorizonCut(EH);
                          S.SetFirstTotalEnergy(m_EnergyMin[e], m_EnergyMax[e]);  
                          S.SetTrackLength(m_TSLMin[u], m_TSLMax[u]);
                          
                          double xx = m_PosTheta[x]; 
                          double xy = m_PosPhi[y];
                          double xz = 1000000000.0;
                          MMath::SphericToCartesean(xx, xy, xz);
                          MVector Position(xx, xy, xz);
                          
                          S.SetSourceWindow(true, Position);
                          S.SetSourceARM(0, m_ARMorRadius[a]);
                          S.SetBeamRadius(m_BRA[b]);
                          S.SetBeamDepth(m_BDE[c]);
                          S.SetOpeningAnglePair(0, m_POP[o]);
                          S.SetInitialEnergyDepositPair(0, m_IDP[d]);
                          
                          Pair_Final[GetPairIndex(d, o, c, b, k, r, h, e, a, u, x, y)].SetEventSelector(S);
                          for (unsigned int bf = 0; bf < m_BackgroundFiles.size(); ++bf) {
                            Pair_Final[GetPairIndex(d, o, c, b, k, r, h, e, a, u, x, y)].SetBackgroundTime(bf, m_BackgroundTimes[bf]);
                            Pair_Final[GetPairIndex(d, o, c, b, k, r, h, e, a, u, x, y)].SetBackgroundName(bf, m_BackgroundFiles[bf]);
                          }
                          if (m_ModeSpectrum == s_ModeLine && m_EnergyWindowMin > 0 && m_EnergyWindowMax > 0) {
                            Pair_Final[GetPairIndex(d, o, c, b, k, r, h, e, a, u, x, y)].SetLargeEnergyWindow(m_EnergyMin[e], m_EnergyMax[e], m_EnergyWindowMin, m_EnergyWindowMax);
                          }
                          Pair_Final[GetPairIndex(d, o, c, b, k, r, h, e, a, u, x, y)].UseComplexEquation(m_ComplexEquation);
                        }
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }



  // 
  if (m_ModeSpectrum == s_ModeLine) {
    if (m_EnergyWindowMin < 0 || m_EnergyWindowMax < 0) {
      m_EnergyWindowMin = m_EnergyMin.back();
      m_EnergyWindowMax = m_EnergyMax.back();  
    }  
  } else {
    m_EnergyWindowMin = m_EnergyMin.front();
    m_EnergyWindowMax = m_EnergyMax.back();  
  }


  // Define an "open event selector":
  MEventSelector OpenSelector = m_EventSelector;
  OpenSelector.UsePhotos(true);
  OpenSelector.UseComptons(true);
  OpenSelector.UsePairs(true);
  OpenSelector.SetComptonQualityFactor(0, m_CQF.back());
  OpenSelector.SetTrackQualityFactor(0, m_TQF.back());
  MEarthHorizon EH;
  EH.SetEarthHorizon(MVector(0, 0, -1E+20), m_EHC.front()*c_Rad);
  if (m_EHPFile == g_StringNotDefined) {
    EH.SetIntersectionTest();
  } else {
    EH.SetProbabilityTest(m_EHP.back(), m_EHPFile);
  }
  OpenSelector.SetEarthHorizonCut(EH);
  OpenSelector.SetFirstTotalEnergy(m_EnergyWindowMin, m_EnergyWindowMax); 
  OpenSelector.SetComptonAngle(0, m_Phi.back());
  OpenSelector.SetThetaDeviationMax(m_The.back());
  OpenSelector.SetSequenceLength(m_CSLMin[0], m_CSLMax[0]);
  OpenSelector.SetTrackLength(m_TSLMin[0], m_TSLMax[0]);
  OpenSelector.SetSourceWindow(false);
  //OpenSelector.SetSourceWindow(true, Position);
  //OpenSelector.SetSourceARM(0, m_ARMorRadius.back());
  //OpenSelector.SetSourceSPD(0, m_SPD.back());
  OpenSelector.SetBeamRadius(m_BRA.back());
  OpenSelector.SetBeamDepth(m_BDE.back());
  OpenSelector.SetOpeningAnglePair(0, m_POP.back());
  OpenSelector.SetInitialEnergyDepositPair(0, m_IDP.back());
  OpenSelector.SetFirstDistance(m_FDI[0], c_FarAway);

  mlog<<"Initialization done."<<endl;

  mlog<<OpenSelector<<endl;


  // Create the response matrices:
  float Step;
    
  vector<float> IDPAxis;
  for (unsigned int i = 0; i < d_max; ++i) {
    IDPAxis.push_back(m_IDP[i]);
  }
  if (IDPAxis.size() == 1) {
    IDPAxis.push_back(m_IDP[0]+1);
  } else {
    IDPAxis.push_back(IDPAxis.back() + (IDPAxis[1] - IDPAxis[0]));
  }
  Step = IDPAxis[1] - IDPAxis[0];
  for (unsigned int i = 0; i < IDPAxis.size(); ++i) {
    IDPAxis[i] -= 0.5*Step; 
  }

  vector<float> POPAxis;
  for (unsigned int i = 0; i < o_max; ++i) {
    POPAxis.push_back(m_POP[i]);
  }
  if (POPAxis.size() == 1) {
    POPAxis.push_back(m_POP[0]+1);
  } else {
    POPAxis.push_back(POPAxis.back() + (POPAxis[1] - POPAxis[0]));
  }
  Step = POPAxis[1] - POPAxis[0];
  for (unsigned int i = 0; i < POPAxis.size(); ++i) {
    POPAxis[i] -= 0.5*Step; 
  }

  vector<float> BRAAxis;
  for (unsigned int i = 0; i < b_max; ++i) {
    BRAAxis.push_back(m_BRA[i]);
  }
  if (BRAAxis.size() == 1) {
    BRAAxis.push_back(2*m_BRA[0]);
  } else {
    BRAAxis.push_back(BRAAxis.back() + (BRAAxis[1] - BRAAxis[0]));
  }
  Step = BRAAxis[1] - BRAAxis[0];
  for (unsigned int i = 0; i < BRAAxis.size(); ++i) {
    BRAAxis[i] -= 0.5*Step; 
  }
  
  vector<float> BDEAxis = m_BDE;
  if (BDEAxis.size() == 1) {
    BDEAxis.push_back(2*m_BDE[0]);
  } else {
    BDEAxis.push_back(BDEAxis.back() + (BDEAxis[1] - BDEAxis[0]));
  }
  Step = BDEAxis[1] - BDEAxis[0];
  for (unsigned int i = 0; i < BDEAxis.size(); ++i) {
    BDEAxis[i] -= 0.5*Step; 
  }

  vector<float> CQFAxis = m_CQF;
  if (q_max == 1) {
    CQFAxis.push_back(1.1*m_CQF[0]);
  } else {
    CQFAxis.push_back(CQFAxis.back() + (CQFAxis[1] - CQFAxis[0]));
  }
  Step = CQFAxis[1] - CQFAxis[0];
  for (unsigned int i = 0; i < CQFAxis.size(); ++i) {
    CQFAxis[i] -= 0.5*Step; 
  }

  vector<float> TQFAxis = m_TQF;
  if (k_max == 1) {
    TQFAxis.push_back(1.1*m_TQF[0]);
  } else {
    TQFAxis.push_back(TQFAxis.back() + (TQFAxis[1] - TQFAxis[0]));
  }
  Step = TQFAxis[1] - TQFAxis[0];
  for (unsigned int i = 0; i < TQFAxis.size(); ++i) {
    TQFAxis[i] -= 0.5*Step; 
  }
 
   
  vector<float> EHPAxis = m_EHP;
  if (EHPAxis.size() == 1) {
    EHPAxis.push_back(m_EHP[0]+1);
  } else {
    EHPAxis.push_back(EHPAxis.back() + (EHPAxis[1] - EHPAxis[0]));
  }
  Step = EHPAxis[1] - EHPAxis[0];
  for (unsigned int i = 0; i < EHPAxis.size(); ++i) {
    EHPAxis[i] -= 0.5*Step; 
  }
  
  vector<float> EHCAxis = m_EHC;
  if (EHCAxis.size() == 1) {
    EHCAxis.push_back(m_EHC[0]+1);
  } else {
    EHCAxis.push_back(EHCAxis.back() + (EHCAxis[1] - EHCAxis[0]));
  }
  Step = EHCAxis[1] - EHCAxis[0];
  for (unsigned int i = 0; i < EHCAxis.size(); ++i) {
    EHCAxis[i] -= 0.5*Step; 
  }
    
  
  vector<float> EnergyAxis = m_EnergyMax;
  if (EnergyAxis.size() == 1) {
    EnergyAxis.push_back(m_EnergyMax[0]+1);
  } else {
    EnergyAxis.push_back(EnergyAxis.back() + (EnergyAxis[1] - EnergyAxis[0]));
  }
  Step = EnergyAxis[1] - EnergyAxis[0];
  for (unsigned int i = 0; i < EnergyAxis.size(); ++i) {
    EnergyAxis[i] -= 0.5*Step; 
  }

  vector<float> PhiAxis = m_Phi;
  if (PhiAxis.size() == 1) {
    PhiAxis.push_back(m_Phi[0]+1);
  } else {
    PhiAxis.push_back(PhiAxis.back() + (PhiAxis[1] - PhiAxis[0]));
  }
  Step = PhiAxis[1] - PhiAxis[0];
  for (unsigned int i = 0; i < PhiAxis.size(); ++i) {
    PhiAxis[i] -= 0.5*Step; 
  }
    
  vector<float> TheAxis = m_The;
  if (TheAxis.size() == 1) {
    TheAxis.push_back(m_The[0]+1);
  } else {
    TheAxis.push_back(TheAxis.back() + (TheAxis[1] - TheAxis[0]));
  }
  Step = TheAxis[1] - TheAxis[0];
  for (unsigned int i = 0; i < TheAxis.size(); ++i) {
    TheAxis[i] -= 0.5*Step; 
  }
    
  vector<float> ARMAxis = m_ARMorRadius;
  if (ARMAxis.size() == 1) {
    ARMAxis.push_back(m_ARMorRadius[0]+1);
  } else {
    ARMAxis.push_back(ARMAxis.back() + (ARMAxis[1] - ARMAxis[0]));
  }
  Step = ARMAxis[1] - ARMAxis[0];
  for (unsigned int i = 0; i < ARMAxis.size(); ++i) {
    ARMAxis[i] -= 0.5*Step; 
  }

  vector<float> SPDAxis = m_SPD;
  if (SPDAxis.size() == 1) {
    SPDAxis.push_back(m_SPD[0]+1);
  } else {
    SPDAxis.push_back(SPDAxis.back() + (SPDAxis[1] - SPDAxis[0]));
  }
  Step = SPDAxis[1] - SPDAxis[0];
  for (unsigned int i = 0; i < SPDAxis.size(); ++i) {
    SPDAxis[i] -= 0.5*Step; 
  }
    
  vector<float> TSLAxis;
  for (unsigned int i = 0; i < u_max; ++i) {
    TSLAxis.push_back(m_TSLMin[i]);
  }
  if (TSLAxis.size() == 1) {
    TSLAxis.push_back(m_TSLMin[0]+1);
  } else {
    TSLAxis.push_back(TSLAxis.back() + (TSLAxis[1] - TSLAxis[0]));
  }
  Step = TSLAxis[1] - TSLAxis[0];
  for (unsigned int i = 0; i < TSLAxis.size(); ++i) {
    TSLAxis[i] -= 0.5*Step; 
  }
    
  vector<float> CSLAxis;
  for (unsigned int i = 0; i < l_max; ++i) {
    CSLAxis.push_back(m_CSLMin[i]);
  }
  if (CSLAxis.size() == 1) {
    CSLAxis.push_back(m_CSLMin[0]+1);
  } else {
    CSLAxis.push_back(CSLAxis.back() + (CSLAxis[1] - CSLAxis[0]));
  }
  Step = CSLAxis[1] - CSLAxis[0];
  for (unsigned int i = 0; i < CSLAxis.size(); ++i) {
    CSLAxis[i] -= 0.5*Step;
  }
   
  vector<float> FDIAxis;
  for (unsigned int i = 0; i < f_max; ++i) {
    FDIAxis.push_back(m_FDI[i]);
  }
  if (FDIAxis.size() == 1) {
    FDIAxis.push_back(m_FDI[0]+1);
  } else {
    FDIAxis.push_back(FDIAxis.back() + (FDIAxis[1] - FDIAxis[0]));
  }
  Step = FDIAxis[1] - FDIAxis[0];
  for (unsigned int i = 0; i < FDIAxis.size(); ++i) {
    FDIAxis[i] -= 0.5*Step; 
  }
   
  vector<float> PosThetaAxis;
  for (unsigned int i = 0; i < x_max; ++i) {
    PosThetaAxis.push_back(m_PosTheta[i]);
  }
  if (PosThetaAxis.size() == 1) {
    PosThetaAxis.push_back(m_PosTheta[0]+1);
  } else {
    PosThetaAxis.push_back(PosThetaAxis.back() + (PosThetaAxis[1] - PosThetaAxis[0]));
  }
  Step = PosThetaAxis[1] - PosThetaAxis[0];
  for (unsigned int i = 0; i < PosThetaAxis.size(); ++i) {
    PosThetaAxis[i] -= 0.5*Step; 
  }
   
  vector<float> PosPhiAxis;
  for (unsigned int i = 0; i < y_max; ++i) {
    PosPhiAxis.push_back(m_PosPhi[i]);
  }
  if (PosPhiAxis.size() == 1) {
    PosPhiAxis.push_back(m_PosPhi[0]+1);
  } else {
    PosPhiAxis.push_back(PosPhiAxis.back() + (PosPhiAxis[1] - PosPhiAxis[0]));
  }
  Step = PosPhiAxis[1] - PosPhiAxis[0];
  for (unsigned int i = 0; i < PosPhiAxis.size(); ++i) {
    PosPhiAxis[i] -= 0.5*Step; 
  }
 

  MResponseMatrixO16 Sensitivity_trackedcompton("Sensitivity Matrix", BDEAxis, BRAAxis, CQFAxis, TQFAxis, EHPAxis, EHCAxis, EnergyAxis, 
                                                PhiAxis, TheAxis, SPDAxis, ARMAxis, TSLAxis, CSLAxis, FDIAxis, PosThetaAxis, PosPhiAxis);
  Sensitivity_trackedcompton.SetAxisNames("Beam depth", 
                                          "Beam radius", 
                                          "Compton quality factor", 
                                          "Track quality factor", 
                                          "Earth horizon probability", 
                                          "Earth horizon cut [degree]", 
                                          "Energy [keV]", 
                                          "Compton scatter angle [degree]", 
                                          "Total scatter angle [degree]", 
                                          "SPD [degree]", 
                                          "ARM [degree]", 
                                          "Track length", 
                                          "Compton sequence length",
                                          "First Compton distance",
                                          "Theta position [deg]",
                                          "Phi position [deg]");
  
  MResponseMatrixO16 EffectiveArea_trackedcompton("Effective Area Matrix", BDEAxis, BRAAxis, CQFAxis, TQFAxis, EHPAxis, EHCAxis, EnergyAxis, 
                                                  PhiAxis, TheAxis, SPDAxis, ARMAxis, TSLAxis, CSLAxis, FDIAxis, PosThetaAxis, PosPhiAxis);
  EffectiveArea_trackedcompton.SetAxisNames("Beam depth", 
                                            "Beam radius", 
                                            "Compton quality factor", 
                                            "Track quality factor", 
                                            "Earth horizon probability", 
                                            "Earth horizon cut [degree]", 
                                            "Energy [keV]", 
                                            "Compton scatter angle [degree]", 
                                            "Total scatter angle [degree]", 
                                            "SPD [degree]", 
                                            "ARM [degree]", 
                                            "Track length", 
                                            "Compton sequence length",
                                            "First Compton distance",
                                            "Theta position [deg]",
                                            "Phi position [deg]");
  
  MResponseMatrixO16 BackgroundRate_trackedcompton("Background rate Matrix", BDEAxis, BRAAxis, CQFAxis, TQFAxis, EHPAxis, EHCAxis, EnergyAxis, 
                                                   PhiAxis, TheAxis, SPDAxis, ARMAxis, TSLAxis, CSLAxis, FDIAxis, PosThetaAxis, PosPhiAxis);
  BackgroundRate_trackedcompton.SetAxisNames("Beam depth", 
                                             "Beam radius", 
                                             "Compton quality factor", 
                                             "Track quality factor", 
                                             "Earth horizon probability", 
                                             "Earth horizon cut [degree]", 
                                             "Energy [keV]", 
                                             "Compton scatter angle [degree]", 
                                             "Total scatter angle [degree]", 
                                             "SPD [degree]", 
                                             "ARM [degree]", 
                                             "Track length", 
                                             "Compton sequence length",
                                             "First Compton distance",
                                             "Theta position [deg]",
                                             "Phi position [deg]");
 


  MResponseMatrixO12 Sensitivity_untrackedcompton("Sensitivity Matrix", BDEAxis, BRAAxis, CQFAxis, EHPAxis, EHCAxis, EnergyAxis, 
                                                  PhiAxis, ARMAxis, CSLAxis, FDIAxis, PosThetaAxis, PosPhiAxis);
  Sensitivity_untrackedcompton.SetAxisNames("Beam depth", 
                                            "Beam radius", 
                                            "Compton quality factor", 
                                            "Earth horizon probability", 
                                            "Earth horizon cut [degree]", 
                                            "Energy [keV]", 
                                            "Compton scatter angle [degree]", 
                                            "ARM [degree]", 
                                            "Compton sequence length",
                                            "First Compton distance",
                                            "Theta position [deg]",
                                            "Phi position [deg]");
  
  MResponseMatrixO12 EffectiveArea_untrackedcompton("Effective Area Matrix", BDEAxis, BRAAxis, CQFAxis, EHPAxis, EHCAxis, EnergyAxis, 
                                                    PhiAxis, ARMAxis, CSLAxis, FDIAxis, PosThetaAxis, PosPhiAxis);
  EffectiveArea_untrackedcompton.SetAxisNames("Beam depth", 
                                              "Beam radius", 
                                              "Compton quality factor", 
                                              "Earth horizon probability", 
                                              "Earth horizon cut [degree]", 
                                              "Energy [keV]", 
                                              "Compton scatter angle [degree]", 
                                              "ARM [degree]", 
                                              "Compton sequence length",
                                              "First Compton distance",
                                              "Theta position [deg]",
                                              "Phi position [deg]");
  
  MResponseMatrixO12 BackgroundRate_untrackedcompton("Background rate Matrix", BDEAxis, BRAAxis, CQFAxis, EHPAxis, EHCAxis, EnergyAxis, 
                                                     PhiAxis, ARMAxis, CSLAxis, FDIAxis, PosThetaAxis, PosPhiAxis);
  BackgroundRate_untrackedcompton.SetAxisNames("Beam depth", 
                                               "Beam radius", 
                                               "Compton quality factor", 
                                               "Earth horizon probability", 
                                               "Earth horizon cut [degree]", 
                                               "Energy [keV]", 
                                               "Compton scatter angle [degree]", 
                                               "ARM [degree]", 
                                               "Compton sequence length",
                                               "First Compton distance",
                                               "Theta position [deg]",
                                               "Phi position [deg]");

 


  MTimer TimerAll;
  MTimer TimerSource;
  MTimer TimerBackground;

  int counts = 0;
  MPhysicalEvent* Event = 0;
  MComptonEvent* Compton = 0;
  MPairEvent* Pair = 0;

  // Calculate effective areas:
  for (unsigned int x = 0; x < x_max; ++x) {         
    for (unsigned int y = 0; y < y_max; ++y) {         
      MFileEventsTra Source;
      if (m_ModeSourceExtension == s_ModePointSource) {
        // We have for each angle one point source
        SourceIndex = FindSourceIndex(x, y);
        if (Source.Open(m_SourceFile[SourceIndex]) == false) {
          mlog<<"Unable to open file "<<m_SourceFile[SourceIndex]<<endl;
          return false;
        }
        Source.StartThread();
        mlog<<"Analyzing file: "<<m_SourceFile[SourceIndex]<<endl;
      } else {
        // We can extract all angles from one isotrpic source --- the first one...
        SourceIndex = 0;
        if (Source.Open(m_SourceFile[SourceIndex]) == false) {
          mlog<<"Unable to open file "<<m_SourceFile[SourceIndex]<<endl;
          return false;
        }
        Source.StartThread();
        mlog<<"Analyzing file: "<<m_SourceFile[SourceIndex]<<endl;      
      }
      
      // ... loop over all events and save them if they pass the event selection criteria
      counts = 0;
      while ((Event = Source.GetNextEvent()) != 0) {
        if (++counts % 1000 == 0) mlog<<"Counts: "<<counts<<" after "<<TimerSource.GetElapsed()<<" sec"<<endl;

        //mlog<<"Test qualified!"<<endl;
        if (OpenSelector.IsQualifiedEventFast(Event) == true) {
          //mlog<<"Qualified!"<<endl;
          if (Event->GetType() == MPhysicalEvent::c_Compton) {
            //mlog<<"Good Compton!"<<endl;
            Compton = dynamic_cast<MComptonEvent*>(Event);
            
            if (Compton->HasTrack() == true) {
              //mlog<<"Found tracked Compton"<<endl;
              for (unsigned int c = 0; c < c_max; ++c) {
                //mlog<<"BDE"<<endl;
                for (unsigned int b = 0; b < b_max; ++b) {
                  //mlog<<"BRA"<<endl;
                  for (unsigned int q = 0; q < q_max; ++q) {
                    //mlog<<"CQF"<<endl;
                    if (Compton->ComptonQualityFactor1() > m_CQF[q]) continue;
                    for (unsigned int k = 0; k < k_max; ++k) {
                      //mlog<<"TQF"<<endl;
                      if (Compton->TrackQualityFactor1() > m_TQF[k]) continue;
                      for (unsigned int e = m_EnergyMax.size()-1; e < m_EnergyMax.size(); --e) {
                        //mlog<<"E"<<endl;
                        if (Compton->Ei() > m_EnergyMax[e] || Compton->Ei() < m_EnergyMin[e]) continue;
                        for (unsigned int p = 0; p < p_max; ++p) {
                          //mlog<<"Phi"<<endl;
                          if (Compton->Phi() > m_Phi[p]*c_Rad) continue;
                          for (unsigned int t = 0; t < t_max; ++t) {
                            //mlog<<"theta: "<<Compton->GetThetaDeviation()*c_Deg<<" vs. "<<m_The[t]<<endl;
                            if (Compton->DeltaTheta() > m_The[t]*c_Rad) continue;
                            for (unsigned int r = 0; r < r_max; ++r) {
                              //mlog<<"EHP"<<endl;
                              for (unsigned int h = 0; h < h_max; ++h) {
                                //mlog<<"EHC"<<endl;
                                for (unsigned int s = 0; s < s_max; ++s) {
                                  //mlog<<"SPD"<<endl;
                                  for (unsigned int a = 0; a < a_max; ++a) {
                                    //mlog<<"ARM"<<endl;
                                    for (unsigned int u = 0; u < u_max; ++u) {
                                      //mlog<<"TSL"<<endl;
                                      for (unsigned int l = 0; l < l_max; ++l) {
                                        //mlog<<"CSL"<<endl;
                                        if (Compton->SequenceLength() < m_CSLMin[l] ||
                                            Compton->SequenceLength() > m_CSLMax[l]) continue;
                                        for (unsigned int f = 0; f < f_max; ++f) {
                                          //mlog<<"FDI"<<endl;
                                          TrackedCompton_Final[GetTrackedComptonIndex(c, b, q, k, r, h, e, p, t, s, a, u, l, f, x, y)].TestSourceEvent(Event);
                                        }
                                      }
                                    }
                                  }
                                }
                              }
                            }
                          }
                        }
                      }
                    }
                  }
                }
              }
            } 
            // Untracked Compton:
            else {
              for (unsigned int c = 0; c < c_max; ++c) {
                //mlog<<"BDE"<<endl;
                for (unsigned int b = 0; b < b_max; ++b) {
                  //mlog<<"BRA"<<endl;
                  for (unsigned int q = 0; q < q_max; ++q) {
                    //mlog<<"CQF"<<endl;
                    if (Compton->ComptonQualityFactor1() > m_CQF[q]) continue;
                    for (unsigned int e = m_EnergyMax.size()-1; e < m_EnergyMax.size(); --e) {
                      //mlog<<"E"<<endl;
                      if (Compton->Ei() > m_EnergyMax[e] || Compton->Ei() < m_EnergyMin[e]) continue;
                      for (unsigned int p = 0; p < p_max; ++p) {
                        //mlog<<"Phi"<<endl;
                        if (Compton->Phi() > m_Phi[p]*c_Rad) continue;
                        for (unsigned int r = 0; r < r_max; ++r) {
                          //mlog<<"EHP"<<endl;
                          for (unsigned int h = 0; h < h_max; ++h) {
                            //mlog<<"EHC"<<endl;
                            for (unsigned int a = 0; a < a_max; ++a) {
                              //mlog<<"ARM"<<endl;
                              for (unsigned int l = 0; l < l_max; ++l) {
                                //mlog<<"CSL"<<endl;
                                if (Compton->SequenceLength() < m_CSLMin[l] ||
                                    Compton->SequenceLength() > m_CSLMax[l]) continue;
                                for (unsigned int f = 0; f < f_max; ++f) {
                                  //mlog<<"FDI"<<endl;
                                  UntrackedCompton_Final[GetUntrackedComptonIndex(c, b, q, r, h, e, p, a, l, f, x, y)].TestSourceEvent(Event);
                                  //mout<<"Found untracked Compton"<<endl;
                                }
                              }
                            }
                          }
                        }
                      }
                    }
                  }
                }
              }

            }
          } 
          // Pair:
          else if (Event->GetType() == MPhysicalEvent::c_Pair) {
            Pair = dynamic_cast<MPairEvent*>(Event);
            for (unsigned int d = 0; d < d_max; ++d) {
              //mlog<<"1"<<endl;
              for (unsigned int o = 0; o < o_max; ++o) {
                //mlog<<"1"<<endl;
                for (unsigned int c = 0; c < c_max; ++c) {
                  //cout<<"c: "<<c<<":"<<c_max<<endl;
                  for (unsigned int b = 0; b < b_max; ++b) {
                    //cout<<"b: "<<b<<":"<<b_max<<endl;
                    for (unsigned int k = 0; k < k_max; ++k) {
                      //mlog<<"1"<<endl;
                      //if (Pair->TrackQualityFactor1() > m_TQF[k]) continue;
                      for (unsigned int e = m_EnergyMax.size()-1; e < m_EnergyMax.size(); --e) {
                        //mlog<<"3"<<endl;
                        if (Pair->Ei() > m_EnergyMax[e] || Pair->Ei() < m_EnergyMin[e]) continue;
                        for (unsigned int r = 0; r < r_max; ++r) {
                          //mlog<<"2"<<endl;
                          for (unsigned int h = 0; h < h_max; ++h) {
                            //mlog<<"2"<<endl;
                            for (unsigned int a = 0; a < a_max; ++a) {
                              //mlog<<"5"<<endl;
                              for (unsigned int u = 0; u < u_max; ++u) {
                                Pair_Final[GetPairIndex(d, o, c, b, k, r, h, e, a, u, x, y)].TestSourceEvent(Event);
                              }
                            }
                          }
                        }
                      }
                    }
                  }
                }
              }
            }
          }
          // Photo:
          else if (Event->GetType() == MPhysicalEvent::c_Photo) {
            for (unsigned int c = 0; c < c_max; ++c) {
              //cout<<"c: "<<c<<":"<<c_max<<endl;
              for (unsigned int b = 0; b < b_max; ++b) {
                //cout<<"b: "<<b<<":"<<b_max<<endl;
                for (unsigned int e = m_EnergyMax.size()-1; e < m_EnergyMax.size(); --e) {
                  Photo_Final[GetPhotoIndex(c, b, e, x, y)].TestSourceEvent(Event);
                }
              }
            }
          }
        }
        
        delete Event;
        if (m_Interrupt == true) break;
      }
      Source.Close();  
    }  // loop over all source files...
  }

  cout<<OpenSelector.ToString()<<endl;


  mlog<<"Source time: "<<TimerSource.GetElapsed()<<endl;
  mlog<<"Source counts: "<<TimerSource.GetElapsed()<<endl;


  //
  // Background:
  //

  TimerBackground.Start();

  // Sensitivity:
  for (unsigned int bf = 0; bf < m_BackgroundFiles.size(); ++bf) {
    MFileEventsTra Source;
    if (Source.Open(m_BackgroundFiles[bf]) == false) {
      mlog<<"Unable to open file "<<m_BackgroundFiles[bf]<<endl;
      return false;
    }
    Source.StartThread();
    mlog<<"Analyzing file: "<<m_BackgroundFiles[bf]<<endl;
 
    // ... loop over all events and save them if they pass the event selection criteria
    int counts = 0;
    MPhysicalEvent* Event = 0;
    while ((Event = Source.GetNextEvent()) != 0) {
      if (++counts % 10000 == 0) mlog<<"Counts: "<<counts<<" after "<<TimerBackground.GetElapsed()<<" sec"<<endl;
      if (OpenSelector.IsQualifiedEventFast(Event) == true) {
        if (Event->GetType() == MPhysicalEvent::c_Compton) {

          // Tracked Compton
          Compton = dynamic_cast<MComptonEvent*>(Event);
          if (Compton->HasTrack() == true) {
            for (unsigned int c = 0; c < c_max; ++c) {
              for (unsigned int b = 0; b < b_max; ++b) {
                for (unsigned int q = 0; q < q_max; ++q) {
                  if (Compton->ComptonQualityFactor1() > m_CQF[q]) continue;
                  for (unsigned int k = 0; k < k_max; ++k) {
                    if (Compton->TrackQualityFactor1() > m_TQF[k]) continue;
                    for (unsigned int r = 0; r < r_max; ++r) {
                      for (unsigned int h = 0; h < h_max; ++h) {
                        for (unsigned int e = m_EnergyMax.size()-1; e < m_EnergyMax.size(); --e) {
                          // if (Compton->Ei() > m_EnergyMax[e] || Compton->Ei() < m_EnergyMin[e]) continue;
                          if (Compton->Ei() > m_EnergyWindowMax || Compton->Ei() < m_EnergyWindowMin) continue;
                          for (unsigned int p = 0; p < p_max; ++p) {
                            if (Compton->Phi() > m_Phi[p]*c_Rad) continue;
                            for (unsigned int t = 0; t < t_max; ++t) {
                              if (Compton->DeltaTheta() > m_The[t]*c_Rad) continue;
                              for (unsigned int s = 0; s < s_max; ++s) {
                                for (unsigned int a = 0; a < a_max; ++a) {
                                  for (unsigned int u = 0; u < u_max; ++u) {
                                    for (unsigned int l = 0; l < l_max; ++l) {
                                      if (Compton->SequenceLength() < m_CSLMin[l] ||
                                          Compton->SequenceLength() > m_CSLMax[l]) continue;
                                      for (unsigned int f = 0; f < f_max; ++f) {
                                        for (unsigned int x = 0; x < x_max; ++x) {
                                          for (unsigned int y = 0; y < y_max; ++y) {
                                            TrackedCompton_Final[GetTrackedComptonIndex(c, b, q, k, r, h, e, p, t, s, a, u, l, f, x, y)].TestBackgroundEvent(Event, bf);
                                          }
                                        }
                                      }
                                    }
                                  }
                                }
                              }
                            }
                          }
                        }
                      }
                    }
                  }
                }
              }
            }
          }
          // Untracked Compton:
          else {
            for (unsigned int c = 0; c < c_max; ++c) {
              for (unsigned int b = 0; b < b_max; ++b) {
                for (unsigned int q = 0; q < q_max; ++q) {
                  if (Compton->ComptonQualityFactor1() > m_CQF[q]) continue;
                  for (unsigned int r = 0; r < r_max; ++r) {
                    for (unsigned int h = 0; h < h_max; ++h) {
                      for (unsigned int e = m_EnergyMax.size()-1; e < m_EnergyMax.size(); --e) {
                        // if (Compton->Ei() > m_EnergyMax[e] || Compton->Ei() < m_EnergyMin[e]) continue;
                        if (Compton->Ei() > m_EnergyWindowMax || Compton->Ei() < m_EnergyWindowMin) continue;
                        for (unsigned int p = 0; p < p_max; ++p) {
                          if (Compton->Phi() > m_Phi[p]*c_Rad) continue;
                          for (unsigned int a = 0; a < a_max; ++a) {
                            for (unsigned int l = 0; l < l_max; ++l) {
                              if (Compton->SequenceLength() < m_CSLMin[l] ||
                                  Compton->SequenceLength() > m_CSLMax[l]) continue;
                              for (unsigned int f = 0; f < f_max; ++f) {
                                for (unsigned int x = 0; x < x_max; ++x) {
                                  for (unsigned int y = 0; y < y_max; ++y) {
                                    UntrackedCompton_Final[GetUntrackedComptonIndex(c, b, q, r, h, e, p, a, l, f, x, y)].TestBackgroundEvent(Event, bf);
                                  }
                                }
                              }
                            }
                          }
                        }
                      }
                    }
                  }
                }
              }
            }
          }
        }
        // Pairs:
        else if (Event->GetType() == MPhysicalEvent::c_Pair) {

          Pair = dynamic_cast<MPairEvent*>(Event);
          for (unsigned int d = 0; d < d_max; ++d) {
            //mlog<<"1"<<endl;
            for (unsigned int o = 0; o < o_max; ++o) {
              //mlog<<"1"<<endl;
              for (unsigned int c = 0; c < c_max; ++c) {
                for (unsigned int b = 0; b < b_max; ++b) {
                  for (unsigned int k = 0; k < k_max; ++k) {
                    //mlog<<"1"<<endl;
                    //if (Pair->TrackQualityFactor1() > m_TQF[k]) continue;
                    for (unsigned int e = m_EnergyMax.size()-1; e < m_EnergyMax.size(); --e) {
                      //mlog<<"3"<<endl;
                      if (Pair->Ei() > m_EnergyMax[e] || Pair->Ei() < m_EnergyMin[e]) continue;
                      for (unsigned int r = 0; r < r_max; ++r) {
                        //mlog<<"2"<<endl;
                        for (unsigned int h = 0; h < h_max; ++h) {
                          //mlog<<"2"<<endl;
                          for (unsigned int a = 0; a < a_max; ++a) {
                            //mlog<<"5"<<endl;
                            for (unsigned int u = 0; u < u_max; ++u) {
                              for (unsigned int x = 0; x < x_max; ++x) {
                                for (unsigned int y = 0; y < y_max; ++y) {
                                  Pair_Final[GetPairIndex(d, o, c, b, k, r, h, e, a, u, x, y)].TestBackgroundEvent(Event, bf);
                                }
                              }
                            }
                          }
                        }
                      }
                    }
                  }
                }
              }
            }
          }
        }
        // Photos:
        else if (Event->GetType() == MPhysicalEvent::c_Photo) {
          for (unsigned int c = 0; c < c_max; ++c) {
            for (unsigned int b = 0; b < b_max; ++b) {
              for (unsigned int e = m_EnergyMax.size()-1; e < m_EnergyMax.size(); --e) {
                for (unsigned int x = 0; x < x_max; ++x) {
                  for (unsigned int y = 0; y < y_max; ++y) {
                    //cout<<"x: "<<x<<":"<<m_Pos.size()<<endl;
                    Photo_Final[GetPhotoIndex(c, b, e, x, y)].TestBackgroundEvent(Event, bf);
                  }
                }
              }
            }
          }
        }
      }

      delete Event;
      if (m_Interrupt == true) break;
    }
    Source.Close();  
  }

  mlog<<"Background time: "<<TimerBackground.GetElapsed()<<endl;


  //
  // Summary:
  //

  // Find the best sensitivity:

  unsigned int d_best_pair = 0; 
  unsigned int o_best_pair = 0; 
  unsigned int c_best_photo = 0; unsigned int c_best_trackedcompton = 0; unsigned int c_best_untrackedcompton = 0; unsigned int c_best_pair = 0; 
  unsigned int b_best_photo = 0; unsigned int b_best_trackedcompton = 0; unsigned int b_best_untrackedcompton = 0; unsigned int b_best_pair = 0; 
  unsigned int q_best_trackedcompton = 0; unsigned int q_best_untrackedcompton = 0; 
  unsigned int k_best_trackedcompton = 0; unsigned int k_best_pair = 0; 
  unsigned int r_best_trackedcompton = 0; unsigned int r_best_untrackedcompton = 0; unsigned int r_best_pair = 0; 
  unsigned int h_best_trackedcompton = 0; unsigned int h_best_untrackedcompton = 0; unsigned int h_best_pair = 0; 
  unsigned int e_best_photo = 0; unsigned int e_best_trackedcompton = 0; unsigned int e_best_untrackedcompton = 0; unsigned int e_best_pair = 0; 
  unsigned int p_best_trackedcompton = 0; unsigned int p_best_untrackedcompton = 0;
  unsigned int t_best_trackedcompton = 0;
  unsigned int s_best_trackedcompton = 0; unsigned int s_best_pair = 0; 
  unsigned int u_best_trackedcompton = 0; unsigned int u_best_pair = 0; 
  unsigned int l_best_trackedcompton = 0; unsigned int l_best_untrackedcompton = 0; 
  unsigned int f_best_trackedcompton = 0; unsigned int f_best_untrackedcompton = 0;
  unsigned int a_best_trackedcompton = 0; unsigned int a_best_untrackedcompton = 0; unsigned int a_best_pair = 0; 
  unsigned int x_best_photo = 0; unsigned int x_best_trackedcompton = 0; unsigned int x_best_untrackedcompton = 0; unsigned int x_best_pair = 0; 
  unsigned int y_best_photo = 0; unsigned int y_best_trackedcompton = 0; unsigned int y_best_untrackedcompton = 0; unsigned int y_best_pair = 0; 
  
  float BestSensitivity_photo = numeric_limits<float>::max();
  float BestSensitivity_trackedcompton = numeric_limits<float>::max();
  float BestSensitivity_untrackedcompton = numeric_limits<float>::max();
  float BestSensitivity_pair = numeric_limits<float>::max();
  
    
  for (unsigned int d = 0; d < d_max; ++d) {
    for (unsigned int o = 0; o < o_max; ++o) {
      for (unsigned int c = 0; c < c_max; ++c) {
        for (unsigned int b = 0; b < b_max; ++b) {
          for (unsigned int q = 0; q < q_max; ++q) {
            for (unsigned int k = 0; k < k_max; ++k) {
              for (unsigned int r = 0; r < r_max; ++r) {
                for (unsigned int h = 0; h < h_max; ++h) {
                  for (unsigned int e = 0; e < e_max; ++e) {
                    for (unsigned int p = 0; p < p_max; ++p) {
                      for (unsigned int t = 0; t < t_max; ++t) {
                        for (unsigned int s = 0; s < s_max; ++s) {
                          for (unsigned int u = 0; u < u_max; ++u) {  
                            for (unsigned int l = 0; l < l_max; ++l) {  
                              for (unsigned int f = 0; f < f_max; ++f) {
                                for (unsigned int a = 0; a < a_max; ++a) {
                                  
                                  // Distinguish between extended and line modes:
                                  if (m_ModeSourceExtension == s_ModeExtended) {
                                    // Fix the optimization to the largest
                                    a = a_max -1;
                                  }
                                  
                                  // Distinguish between scanning mode and pointing mode: 
                                  if (m_ModeObservation == s_ModeSingleObs) {
                                    // Pointing mode:
                                    if (TrackedCompton_Final[GetTrackedComptonIndex(c, b, q, k, r, h, e, p, t, s, a, u, l, f, 0, 0)].GetSensitivity() < BestSensitivity_trackedcompton && 
                                        TrackedCompton_Final[GetTrackedComptonIndex(c, b, q, k, r, h, e, p, t, s, a, u, l, f, 0, 0)].GetSourceCounts() > 0 &&
                                        TrackedCompton_Final[GetTrackedComptonIndex(c, b, q, k, r, h, e, p, t, s, a, u, l, f, 0, 0)].GetBackgroundCounts() > 0 && 
                                        !(m_MinBackgroundCounts > 0 && TrackedCompton_Final[GetTrackedComptonIndex(c, b, q, k, r, h, e, p, t, s, a, u, l, f, 0, 0)].AllBackgroundComponentCountsAbove(m_MinBackgroundCounts) == false)) {
                                      BestSensitivity_trackedcompton = TrackedCompton_Final[GetTrackedComptonIndex(c, b, q, k, r, h, e, p, t, s, a, u, l, f, 0, 0)].GetSensitivity();
                                      c_best_trackedcompton = c;
                                      b_best_trackedcompton = b;
                                      q_best_trackedcompton = q;
                                      k_best_trackedcompton = k;
                                      r_best_trackedcompton = r;
                                      h_best_trackedcompton = h;
                                      e_best_trackedcompton = e;
                                      p_best_trackedcompton = p;
                                      t_best_trackedcompton = t;
                                      s_best_trackedcompton = s;
                                      a_best_trackedcompton = a;
                                      u_best_trackedcompton = u;
                                      l_best_trackedcompton = l;
                                      f_best_trackedcompton = f;
                                      x_best_trackedcompton = 0;
                                      y_best_trackedcompton = 0;
                                    }
                                    if (UntrackedCompton_Final[GetUntrackedComptonIndex(c, b, q, r, h, e, p, a, l, f, 0, 0)].GetSensitivity() < BestSensitivity_untrackedcompton && 
                                        UntrackedCompton_Final[GetUntrackedComptonIndex(c, b, q, r, h, e, p, a, l, f, 0, 0)].GetSourceCounts() > 0 &&
                                        UntrackedCompton_Final[GetUntrackedComptonIndex(c, b, q, r, h, e, p, a, l, f, 0, 0)].GetBackgroundCounts() > 0 && 
                                        !(m_MinBackgroundCounts > 0 && UntrackedCompton_Final[GetUntrackedComptonIndex(c, b, q, r, h, e, p, a, l, f, 0, 0)].AllBackgroundComponentCountsAbove(m_MinBackgroundCounts) == false)) {
                                      BestSensitivity_untrackedcompton = UntrackedCompton_Final[GetUntrackedComptonIndex(c, b, q, r, h, e, p, a, l, f, 0, 0)].GetSensitivity();
                                      c_best_untrackedcompton = c;
                                      b_best_untrackedcompton = b;
                                      q_best_untrackedcompton = q;
                                      r_best_untrackedcompton = r;
                                      h_best_untrackedcompton = h;
                                      e_best_untrackedcompton = e;
                                      p_best_untrackedcompton = p;
                                      a_best_untrackedcompton = a;
                                      l_best_untrackedcompton = l;
                                      f_best_untrackedcompton = f;
                                      x_best_untrackedcompton = 0;
                                      y_best_untrackedcompton = 0;
                                    }
                                    if (Pair_Final[GetPairIndex(d, o, c, b, k, r, h, e, a, u, 0, 0)].GetSensitivity() < BestSensitivity_pair && 
                                        Pair_Final[GetPairIndex(d, o, c, b, k, r, h, e, a, u, 0, 0)].GetSourceCounts() > 0 &&
                                        Pair_Final[GetPairIndex(d, o, c, b, k, r, h, e, a, u, 0, 0)].GetBackgroundCounts() > 0 && 
                                        !(m_MinBackgroundCounts > 0 && Pair_Final[GetPairIndex(d, o, c, b, k, r, h, e, a, u, 0, 0)].AllBackgroundComponentCountsAbove(m_MinBackgroundCounts) == false)) {
                                      BestSensitivity_pair = Pair_Final[GetPairIndex(d, o, c, b, k, r, h, e, a, u, 0, 0)].GetSensitivity();
                                      d_best_pair = d;
                                      o_best_pair = o;
                                      c_best_pair = c;
                                      b_best_pair = b;
                                      k_best_pair = k;
                                      r_best_pair = r;
                                      h_best_pair = h;
                                      e_best_pair = e;
                                      a_best_pair = a;
                                      u_best_pair = u;
                                      x_best_pair = 0;
                                      y_best_pair = 0;
                                    }
                                    if (Photo_Final[GetPhotoIndex(c, b, e, 0, 0)].GetSensitivity() < BestSensitivity_photo && 
                                        Photo_Final[GetPhotoIndex(c, b, e, 0, 0)].GetSourceCounts() > 0 &&
                                        Photo_Final[GetPhotoIndex(c, b, e, 0, 0)].GetBackgroundCounts() > 0 && 
                                        !(m_MinBackgroundCounts > 0 && Photo_Final[GetPhotoIndex(c, b, e, 0, 0)].AllBackgroundComponentCountsAbove(m_MinBackgroundCounts) == false)) {
                                      BestSensitivity_photo = Photo_Final[GetPhotoIndex(c, b, e, 0, 0)].GetSensitivity();
                                      c_best_photo = c;
                                      b_best_photo = b;
                                      e_best_photo = e;
                                      x_best_photo = 0;
                                      y_best_photo = 0;
                                    }
                                  } else if (m_ModeObservation == s_ModeAllSkyObs) {
                                    // Scanning mode
                                    double Time = m_ObservationTime;
                                    double AvgAeff = 0.0; // cm2
                                    double AvgAeffIntermediate = 0.0; // cm2
                                    double Background = 0.0; // s-1
                                    double BackgroundIntermediate = 0.0; // s-1
                                    double Sensitivity = 0.0; // ph/cm2/s
                                    
                                    // (a) Tracked Compton:
                                    
                                    // Calculate average exposure
                                    AvgAeff = 0.0; // cm-1*s-1
                                    for (unsigned int x = 0; x < x_max-1; ++x) {
                                      AvgAeffIntermediate = 0.0;
                                      for (unsigned int y = 0; y < y_max; ++y) {
                                        
                                        mlog<<"A_eff (compton):"<<m_PosTheta[x]<<", "<<m_PosPhi[y]<<": "
                                            <<TrackedCompton_Final[GetTrackedComptonIndex(c, b, q, k, r, h, e, p, t, s, a, u, l, f, x, y)].GetSourceCounts()<<": "
                                            <<TrackedCompton_Final[GetTrackedComptonIndex(c, b, q, k, r, h, e, p, t, s, a, u, l, f, x, y)].GetEffectiveArea()<<endl;
                                        AvgAeffIntermediate += (TrackedCompton_Final[GetTrackedComptonIndex(c, b, q, k, r, h, e, p, t, s, a, u, l, f, x, y)].GetEffectiveArea() +
                                                                TrackedCompton_Final[GetTrackedComptonIndex(c, b, q, k, r, h, e, p, t, s, a, u, l, f, x+1, y)].GetEffectiveArea());
                                      }
                                      AvgAeffIntermediate /= y_max;
                                      AvgAeff += AvgAeffIntermediate*(cos(m_PosTheta[x]*c_Rad)-cos(m_PosTheta[x+1]*c_Rad));
                                    }
                                    AvgAeff *= c_Pi; // factor 2 cancels
                                    AvgAeff /= (4*c_Pi);
                                    mlog<<"Average effective area [cm^2]: "<<AvgAeff<<endl;
                                    
                                    // Calculate average background:
                                    Background = 0.0; // s-1
                                    for (unsigned int x = 0; x < x_max-1; ++x) {
                                      BackgroundIntermediate = 0.0;
                                      for (unsigned int y = 0; y < y_max; ++y) {
                                        mlog<<"Bkg:"<<m_PosTheta[x]<<", "<<m_PosPhi[y]<<": "
                                            <<TrackedCompton_Final[GetTrackedComptonIndex(c, b, q, k, r, h, e, p, t, s, a, u, l, f, x, y)].GetBackgroundRate()<<endl;
                                        BackgroundIntermediate += 
                                          TrackedCompton_Final[GetTrackedComptonIndex(c, b, q, k, r, h, e, p, t, s, a, u, l, f, x, y)].GetBackgroundRate() +
                                          TrackedCompton_Final[GetTrackedComptonIndex(c, b, q, k, r, h, e, p, t, s, a, u, l, f, x+1, y)].GetBackgroundRate();
                                      }
                                      BackgroundIntermediate /= y_max;
                                      Background += BackgroundIntermediate*(cos(m_PosTheta[x]*c_Rad)-cos(m_PosTheta[x+1]*c_Rad));
                                    }
                                    Background *= c_Pi; // factor 2 cancels
                                    Background /= (4*c_Pi);
                                    mlog<<"Average background [cts/sec]: "<<Background<<endl;
                                    
                                    // Calculate average sensitivity:
                                    Sensitivity = 0.5*(3*3+3*sqrt(3*3+4*Time*Background))/(Time*AvgAeff);
                                    mlog<<"Sensitivity after "<<Time<<" s: "<<Sensitivity<<endl;
                                    if (Sensitivity < BestSensitivity_trackedcompton && 
                                        TrackedCompton_Final[GetTrackedComptonIndex(c, b, q, k, r, h, e, p, t, s, a, u, l, f, 0, 0)].GetSourceCounts() > 0 &&
                                        TrackedCompton_Final[GetTrackedComptonIndex(c, b, q, k, r, h, e, p, t, s, a, u, l, f, 0, 0)].GetBackgroundCounts() > 0) {
                                      BestSensitivity_trackedcompton = Sensitivity;
                                      c_best_trackedcompton = c;
                                      b_best_trackedcompton = b;
                                      q_best_trackedcompton = q;
                                      k_best_trackedcompton = k;
                                      r_best_trackedcompton = r;
                                      h_best_trackedcompton = h;
                                      e_best_trackedcompton = e;
                                      p_best_trackedcompton = p;
                                      t_best_trackedcompton = t;
                                      s_best_trackedcompton = s;
                                      a_best_trackedcompton = a;
                                      u_best_trackedcompton = u;
                                      l_best_trackedcompton = l;
                                      f_best_trackedcompton = f;
                                      x_best_trackedcompton = 0;
                                      y_best_trackedcompton = 0;
                                    }

                                    
                                    // (b) Untracked Compton:
                                    
                                    // Calculate average exposure
                                    AvgAeff = 0.0; // cm-1*s-1
                                    for (unsigned int x = 0; x < x_max-1; ++x) {
                                      AvgAeffIntermediate = 0.0;
                                      for (unsigned int y = 0; y < y_max; ++y) {
                                        
                                        mlog<<"A_eff (compton):"<<m_PosTheta[x]<<", "<<m_PosPhi[y]<<": "
                                            <<UntrackedCompton_Final[GetUntrackedComptonIndex(c, b, q, r, h, e, p, a, l, f, x, y)].GetSourceCounts()<<": "
                                            <<UntrackedCompton_Final[GetUntrackedComptonIndex(c, b, q, r, h, e, p, a, l, f, x, y)].GetEffectiveArea()<<endl;
                                        AvgAeffIntermediate += (UntrackedCompton_Final[GetUntrackedComptonIndex(c, b, q, r, h, e, p, a, l, f, x, y)].GetEffectiveArea() +
                                                                UntrackedCompton_Final[GetUntrackedComptonIndex(c, b, q, r, h, e, p, a, l, f, x+1, y)].GetEffectiveArea());
                                      }
                                      AvgAeffIntermediate /= y_max;
                                      AvgAeff += AvgAeffIntermediate*(cos(m_PosTheta[x]*c_Rad)-cos(m_PosTheta[x+1]*c_Rad));
                                    }
                                    AvgAeff *= c_Pi; // factor 2 cancels
                                    AvgAeff /= (4*c_Pi);
                                    mlog<<"Average effective area [cm^2]: "<<AvgAeff<<endl;
                                    
                                    // Calculate average background:
                                    Background = 0.0; // s-1
                                    for (unsigned int x = 0; x < x_max-1; ++x) {
                                      BackgroundIntermediate = 0.0;
                                      for (unsigned int y = 0; y < y_max; ++y) {
                                        mlog<<"Bkg:"<<m_PosTheta[x]<<", "<<m_PosPhi[y]<<": "
                                            <<UntrackedCompton_Final[GetUntrackedComptonIndex(c, b, q, r, h, e, p, a, l, f, x, y)].GetBackgroundRate()<<endl;
                                        BackgroundIntermediate += 
                                          UntrackedCompton_Final[GetUntrackedComptonIndex(c, b, q, r, h, e, p, a, l, f, x, y)].GetBackgroundRate() +
                                          UntrackedCompton_Final[GetUntrackedComptonIndex(c, b, q, r, h, e, p, a, l, f, x+1, y)].GetBackgroundRate();
                                      }
                                      BackgroundIntermediate /= y_max;
                                      Background += BackgroundIntermediate*(cos(m_PosTheta[x]*c_Rad)-cos(m_PosTheta[x+1]*c_Rad));
                                    }
                                    Background *= c_Pi; // factor 2 cancels
                                    Background /= (4*c_Pi);
                                    mlog<<"Average background [cts/sec]: "<<Background<<endl;
                                    
                                    // Calculate average sensitivity:
                                    Sensitivity = 0.5*(3*3+3*sqrt(3*3+4*Time*Background))/(Time*AvgAeff);
                                    mlog<<"Sensitivity after "<<Time<<" s: "<<Sensitivity<<endl;
                                    if (Sensitivity < BestSensitivity_untrackedcompton && 
                                        UntrackedCompton_Final[GetUntrackedComptonIndex(c, b, q, r, h, e, p, a, l, f, 0, 0)].GetSourceCounts() > 0 &&
                                        UntrackedCompton_Final[GetUntrackedComptonIndex(c, b, q, r, h, e, p, a, l, f, 0, 0)].GetBackgroundCounts() > 0) {
                                      BestSensitivity_untrackedcompton = Sensitivity;
                                      c_best_untrackedcompton = c;
                                      b_best_untrackedcompton = b;
                                      q_best_untrackedcompton = q;
                                      r_best_untrackedcompton = r;
                                      h_best_untrackedcompton = h;
                                      e_best_untrackedcompton = e;
                                      p_best_untrackedcompton = p;
                                      a_best_untrackedcompton = a;
                                      l_best_untrackedcompton = l;
                                      f_best_untrackedcompton = f;
                                      x_best_untrackedcompton = 0;
                                      y_best_untrackedcompton = 0;
                                    }
                                    
                                    // (c) Pair:

                                    // Calculate average exposure
                                    AvgAeff = 0.0; // cm-1*s-1
                                    for (unsigned int x = 0; x < x_max-1; ++x) {
                                      AvgAeffIntermediate = 0.0;
                                      for (unsigned int y = 0; y < y_max; ++y) {
                                        mlog<<"A_eff (pair):"<<m_PosTheta[x]<<", "<<m_PosPhi[y]<<": "
                                            <<Pair_Final[GetPairIndex(d, o, c, b, k, r, h, e, a, u, x, y)].GetSourceCounts()<<": "
                                            <<Pair_Final[GetPairIndex(d, o, c, b, k, r, h, e, a, u, x, y)].GetEffectiveArea()<<endl;
                                        AvgAeffIntermediate += Pair_Final[GetPairIndex(d, o, c, b, k, r, h, e, a, u, x, y)].GetEffectiveArea() +
                                          Pair_Final[GetPairIndex(d, o, c, b, k, r, h, e, a, u, x+1, y)].GetEffectiveArea();
                                      }
                                      AvgAeffIntermediate /= y_max;
                                      AvgAeff += AvgAeffIntermediate*(cos(m_PosTheta[x]*c_Rad)-cos(m_PosTheta[x+1]*c_Rad));
                                    }
                                    AvgAeff *= c_Pi; // factor 2 cancels
                                    AvgAeff /= (4*c_Pi);
                                    mlog<<"Average effective area [cm^2]: "<<AvgAeff<<endl;

                                    // Calculate average background:
                                    Background = 0.0; // s-1
                                    for (unsigned int x = 0; x < x_max-1; ++x) {
                                      BackgroundIntermediate = 0.0;
                                      for (unsigned int y = 0; y < y_max; ++y) {
                                        mlog<<"Bkg:"<<m_PosTheta[x]<<", "<<m_PosPhi[y]<<": "
                                            <<Pair_Final[GetPairIndex(d, o, c, b, k, r, h, e, a, u, x, y)].GetBackgroundRate()<<endl;
                                        BackgroundIntermediate += 
                                          Pair_Final[GetPairIndex(d, o, c, b, k, r, h, e, a, u, x, y)].GetBackgroundRate() +
                                          Pair_Final[GetPairIndex(d, o, c, b, k, r, h, e, a, u, x+1, y)].GetBackgroundRate();                                          
                                      }
                                      BackgroundIntermediate /= y_max;
                                      Background += BackgroundIntermediate*(cos(m_PosTheta[x]*c_Rad)-cos(m_PosTheta[x+1]*c_Rad));
                                    }
                                    Background *= c_Pi; // factor 2 cancels
                                    Background /= (4*c_Pi);
                                    mlog<<"Average background [cts/sec]: "<<Background<<endl;
                                    
                                    // Calculate average sensitivity:
                                    Sensitivity = 0.5*(3*3+3*sqrt(3*3+4*Time*Background))/(Time*AvgAeff);
                                    mlog<<"Sensitivity after "<<Time<<" s: "<<Sensitivity<<endl;
                                    if (Sensitivity < BestSensitivity_pair && 
                                        Pair_Final[GetPairIndex(d, o, c, b, k, r, h, e, a, u, 0, 0)].GetSourceCounts() > 0 &&
                                        Pair_Final[GetPairIndex(d, o, c, b, k, r, h, e, a, u, 0, 0)].GetBackgroundCounts() > 0) {
                                      BestSensitivity_pair = Sensitivity;
                                      d_best_pair = d;
                                      o_best_pair = o;
                                      c_best_pair = c;
                                      b_best_pair = b;
                                      k_best_pair = k;
                                      r_best_pair = r;
                                      h_best_pair = h;
                                      e_best_pair = e;
                                      a_best_pair = a;
                                      u_best_pair = u;
                                      x_best_pair = 0;
                                      y_best_pair = 0;
                                    }
                                      
                                    
                                    // (d) Photo:
                                    
                                    // Calculate average exposure
                                    AvgAeff = 0.0; // cm-1*s-1
                                    for (unsigned int x = 0; x < x_max-1; ++x) {
                                      AvgAeffIntermediate = 0.0;
                                      for (unsigned int y = 0; y < y_max; ++y) {
                                        mlog<<"A_eff (photo):"<<m_PosTheta[x]<<", "<<m_PosPhi[y]<<": "
                                            <<Photo_Final[GetPhotoIndex(c, b, e, x, y)].GetSourceCounts()<<": "
                                            <<Photo_Final[GetPhotoIndex(c, b, e, x, y)].GetEffectiveArea()<<endl;
                                        AvgAeffIntermediate += Photo_Final[GetPhotoIndex(c, b, e, x, y)].GetEffectiveArea() +
                                          Photo_Final[GetPhotoIndex(c, b, e, x+1, y)].GetEffectiveArea();
                                      }
                                      AvgAeffIntermediate /= y_max;
                                      AvgAeff += AvgAeffIntermediate*(cos(m_PosTheta[x]*c_Rad)-cos(m_PosTheta[x+1]*c_Rad));
                                    }
                                    AvgAeff *= c_Pi; // factor 2 cancels
                                    AvgAeff /= (4*c_Pi);
                                    mlog<<"Average effective area [cm^2]: "<<AvgAeff<<endl;
                                    
                                    // Calculate average background:
                                    Background = 0.0; // s-1
                                    for (unsigned int x = 0; x < x_max-1; ++x) {
                                      BackgroundIntermediate = 0.0;
                                      for (unsigned int y = 0; y < y_max; ++y) {
                                        mlog<<"Bkg:"<<m_PosTheta[x]<<": "<<Photo_Final[GetPhotoIndex(c, b, e, x, y)].GetBackgroundRate()<<endl;
                                        BackgroundIntermediate += 
                                          Photo_Final[GetPhotoIndex(c, b, e, x, y)].GetBackgroundRate() +
                                          Photo_Final[GetPhotoIndex(c, b, e, x+1, y)].GetBackgroundRate();
                                      }
                                      BackgroundIntermediate /= y_max;
                                      Background += BackgroundIntermediate*(cos(m_PosTheta[x]*c_Rad)-cos(m_PosTheta[x+1]*c_Rad));
                                    }
                                    Background *= c_Pi; // factor 2 cancels
                                    Background /= (4*c_Pi);
                                    mlog<<"Average background [cts/sec]: "<<Background<<endl;
                                    
                                    // Calculate average sensitivity:
                                    Sensitivity = 0.5*(3*3+3*sqrt(3*3+4*Time*Background))/(Time*AvgAeff);
                                    mlog<<"Sensitivity after "<<Time<<" s: "<<Sensitivity<<endl;
                                    if (Sensitivity < BestSensitivity_photo && 
                                        Photo_Final[GetPhotoIndex(c, b, e, 0, 0)].GetSourceCounts() > 0 &&
                                        Photo_Final[GetPhotoIndex(c, b, e, 0, 0)].GetBackgroundCounts() > 0) {
                                      BestSensitivity_photo = Sensitivity;
                                      c_best_photo = c;
                                      b_best_photo = b;
                                      e_best_photo = e;
                                      x_best_photo = 0;
                                      y_best_photo = 0;
                                    }
                                  } // if line or continuum

                                  // Distinguish between extended and line modes:
                                  if (m_ModeSourceExtension == s_ModeExtended) {
                                    continue;
                                  }
                                } // for (ARM radius)
                              }
                            }
                          }
                        }
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }
  
  if (Pair_Final[GetPairIndex(d_best_pair, o_best_pair, c_best_pair, b_best_pair, k_best_pair, r_best_pair, h_best_pair, e_best_pair, a_best_pair, u_best_pair, x_best_pair, y_best_pair)].GetEffectiveArea() > 0) {
    mlog<<endl<<endl<<endl;
    mlog<<"Best achievable sensitivity - pairs"<<endl;
    mlog<<"***********************************"<<endl;
    mlog<<endl;
    mlog<<"Sensitivity:  "<<BestSensitivity_pair<<" ph/cm2/s"<<endl;
    mlog<<"Area:         "<<Pair_Final[GetPairIndex(d_best_pair, o_best_pair, c_best_pair, b_best_pair, k_best_pair, r_best_pair, h_best_pair, e_best_pair, a_best_pair, u_best_pair, x_best_pair, y_best_pair)].GetEffectiveArea()<<" cm"<<endl;
    mlog<<endl;
    
    mlog<<"    E: "<<m_EnergyMin[e_best_pair]<<" - "<<m_EnergyMax[e_best_pair];
    mlog<<"  BDE: "<<0<<" - "<<m_BDE[c_best_pair];
    mlog<<"  BRA: "<<0<<" - "<<m_BRA[b_best_pair];
    mlog<<"  TQF: "<<0<<" - "<<m_TQF[k_best_pair];
    mlog<<"  TSL: "<<m_TSLMin[u_best_pair]<<" - "<<m_TSLMax[u_best_pair];
    mlog<<"  EHP: "<<0<<" - "<<m_EHP[r_best_pair];
    mlog<<"  EHC: "<<0<<" - "<<m_EHC[h_best_pair];
    mlog<<"  SPD: "<<0<<" - "<<m_SPD[s_best_pair];
    mlog<<"  ARM: "<<0<<" - "<<m_ARMorRadius[a_best_pair];
    mlog<<"  POP: "<<0<<" - "<<m_POP[o_best_pair];
    mlog<<"  IDP: "<<0<<" - "<<m_IDP[d_best_pair];
    mlog<<endl<<endl;
    mlog<<Pair_Final[GetPairIndex(d_best_pair, o_best_pair, c_best_pair, b_best_pair, k_best_pair, r_best_pair, h_best_pair, e_best_pair, a_best_pair, u_best_pair, x_best_pair, y_best_pair)]<<endl;
    mlog<<endl<<endl<<endl;
  } else {
    mlog<<endl<<endl<<endl;
    mlog<<"Ignoring pair - no effective area..."<<endl;
    mlog<<endl<<endl<<endl;
  }

  if (TrackedCompton_Final[GetTrackedComptonIndex(c_best_trackedcompton, b_best_trackedcompton, q_best_trackedcompton, k_best_trackedcompton, r_best_trackedcompton, h_best_trackedcompton, e_best_trackedcompton, p_best_trackedcompton, t_best_trackedcompton, s_best_trackedcompton, a_best_trackedcompton, u_best_trackedcompton, l_best_trackedcompton, f_best_trackedcompton, x_best_trackedcompton, y_best_trackedcompton)].GetEffectiveArea() > 0) {
    mlog<<"Best achievable sensitivity - tracked compton"<<endl;
    mlog<<"*********************************************"<<endl;
    mlog<<endl;
    mlog<<"Sensitivity: "<<BestSensitivity_trackedcompton<<" ph/cm2/s"<<endl;
    mlog<<"Area:        "<<TrackedCompton_Final[GetTrackedComptonIndex(c_best_trackedcompton, b_best_trackedcompton, q_best_trackedcompton, k_best_trackedcompton, r_best_trackedcompton, h_best_trackedcompton, e_best_trackedcompton, p_best_trackedcompton, t_best_trackedcompton, s_best_trackedcompton, a_best_trackedcompton, u_best_trackedcompton, l_best_trackedcompton, f_best_trackedcompton, x_best_trackedcompton, y_best_trackedcompton)].GetEffectiveArea()<<" cm"<<endl;
    mlog<<endl;
    mlog<<"    E: "<<m_EnergyMin[e_best_trackedcompton]<<" - "<<m_EnergyMax[e_best_trackedcompton];
    mlog<<"  BDE: "<<0<<" - "<<m_BDE[c_best_trackedcompton];
    mlog<<"  BRA: "<<0<<" - "<<m_BRA[b_best_trackedcompton];
    mlog<<"  CQF: "<<0<<" - "<<m_CQF[q_best_trackedcompton];
    mlog<<"  TQF: "<<0<<" - "<<m_TQF[k_best_trackedcompton];
    mlog<<"  Phi: "<<0<<" - "<<m_Phi[p_best_trackedcompton];
    mlog<<"  The: "<<0<<" - "<<m_The[t_best_trackedcompton];
    mlog<<"  CSL: "<<m_CSLMin[l_best_trackedcompton]<<" - "<<m_CSLMax[l_best_trackedcompton];
    mlog<<"  TSL: "<<m_TSLMin[u_best_trackedcompton]<<" - "<<m_TSLMax[u_best_trackedcompton];
    mlog<<"  EHP: "<<0<<" - "<<m_EHP[r_best_trackedcompton];
    mlog<<"  EHC: "<<0<<" - "<<m_EHC[h_best_trackedcompton];
    mlog<<"  SPD: "<<0<<" - "<<m_SPD[s_best_trackedcompton];
    mlog<<"  ARM: "<<0<<" - "<<m_ARMorRadius[a_best_trackedcompton];
    mlog<<"  FDI: "<<m_FDI[f_best_trackedcompton]<<" - inf";
    mlog<<endl<<endl;
    mlog<<TrackedCompton_Final[GetTrackedComptonIndex(c_best_trackedcompton, b_best_trackedcompton, q_best_trackedcompton, k_best_trackedcompton, r_best_trackedcompton, h_best_trackedcompton, e_best_trackedcompton, p_best_trackedcompton, t_best_trackedcompton, s_best_trackedcompton, a_best_trackedcompton, u_best_trackedcompton, l_best_trackedcompton, f_best_trackedcompton, x_best_trackedcompton, y_best_trackedcompton)]<<endl;
    mlog<<endl<<endl<<endl;
  } else {
    mlog<<endl<<endl<<endl;
    mlog<<"Ignoring tracked Compton events - no effective area..."<<endl;
    mlog<<endl<<endl<<endl;
  }

  if (UntrackedCompton_Final[GetUntrackedComptonIndex(c_best_untrackedcompton, b_best_untrackedcompton, q_best_untrackedcompton, r_best_untrackedcompton, h_best_untrackedcompton, e_best_untrackedcompton, p_best_untrackedcompton, a_best_untrackedcompton, l_best_untrackedcompton, f_best_untrackedcompton, x_best_untrackedcompton, y_best_untrackedcompton)].GetEffectiveArea() > 0) {
    mlog<<"Best achievable sensitivity - untracked compton"<<endl;
    mlog<<"***********************************************"<<endl;
    mlog<<endl;
    mlog<<"Sensitivity: "<<BestSensitivity_untrackedcompton<<" ph/cm2/s"<<endl;
    mlog<<"Area:        "<<UntrackedCompton_Final[GetUntrackedComptonIndex(c_best_untrackedcompton, b_best_untrackedcompton, q_best_untrackedcompton, r_best_untrackedcompton, h_best_untrackedcompton, e_best_untrackedcompton, p_best_untrackedcompton, a_best_untrackedcompton, l_best_untrackedcompton, f_best_untrackedcompton, x_best_untrackedcompton, y_best_untrackedcompton)].GetEffectiveArea()<<" cm"<<endl;
    mlog<<endl;
    mlog<<"    E: "<<m_EnergyMin[e_best_untrackedcompton]<<" - "<<m_EnergyMax[e_best_untrackedcompton];
    mlog<<"  BDE: "<<0<<" - "<<m_BDE[c_best_untrackedcompton];
    mlog<<"  BRA: "<<0<<" - "<<m_BRA[b_best_untrackedcompton];
    mlog<<"  CQF: "<<0<<" - "<<m_CQF[q_best_untrackedcompton];
    mlog<<"  Phi: "<<0<<" - "<<m_Phi[p_best_untrackedcompton];
    mlog<<"  CSL: "<<m_CSLMin[l_best_untrackedcompton]<<" - "<<m_CSLMax[l_best_untrackedcompton];
    mlog<<"  EHP: "<<0<<" - "<<m_EHP[r_best_untrackedcompton];
    mlog<<"  EHC: "<<0<<" - "<<m_EHC[h_best_untrackedcompton];
    mlog<<"  ARM: "<<0<<" - "<<m_ARMorRadius[a_best_untrackedcompton];
    mlog<<"  FDI: "<<m_FDI[f_best_untrackedcompton]<<" - inf";
    mlog<<endl<<endl;
    mlog<<UntrackedCompton_Final[GetUntrackedComptonIndex(c_best_untrackedcompton, b_best_untrackedcompton, q_best_untrackedcompton, r_best_untrackedcompton, h_best_untrackedcompton, e_best_untrackedcompton, p_best_untrackedcompton, a_best_untrackedcompton, l_best_untrackedcompton, f_best_untrackedcompton, x_best_untrackedcompton, y_best_untrackedcompton)]<<endl;
    mlog<<endl<<endl<<endl;
  } else {
    mlog<<endl<<endl<<endl;
    mlog<<"Ignoring untracked Compton events - no effective area..."<<endl;
    mlog<<endl<<endl<<endl;
  }

  if (Photo_Final[GetPhotoIndex(c_best_photo, b_best_photo, e_best_photo, x_best_photo, y_best_photo)].GetEffectiveArea() > 0) {
    mlog<<"Best achievable sensitivity - photo"<<endl;
    mlog<<"***********************************"<<endl;
    mlog<<endl;
    mlog<<"Sensitivity:  "<<BestSensitivity_photo<<" ph/cm2/s"<<endl;
    mlog<<"Area:         "<<Photo_Final[GetPhotoIndex(c_best_photo, b_best_photo, e_best_photo, x_best_photo, y_best_photo)].GetEffectiveArea()<<" cm"<<endl;
    mlog<<endl;
    mlog<<"    E: "<<m_EnergyMin[e_best_photo]<<" - "<<m_EnergyMax[e_best_photo];
    mlog<<"  BDE: "<<0<<" - "<<m_BDE[c_best_photo];
    mlog<<"  BRA: "<<0<<" - "<<m_BRA[b_best_photo];
    mlog<<endl<<endl;
    mlog<<Photo_Final[GetPhotoIndex(c_best_photo, b_best_photo, e_best_photo, x_best_photo, y_best_photo)]<<endl;
    mlog<<endl<<endl;
  } else {
    mlog<<endl<<endl<<endl;
    mlog<<"Ignoring photo effect events - no effective area..."<<endl;
    mlog<<endl<<endl<<endl;
  }



  // Fill the response matrices:

  // Tracked Compton
  for (unsigned int c = 0; c < c_max; ++c) {
    for (unsigned int b = 0; b < b_max; ++b) {
      for (unsigned int q = 0; q < q_max; ++q) {
        for (unsigned int k = 0; k < k_max; ++k) {
          for (unsigned int r = 0; r < r_max; ++r) {
            for (unsigned int h = 0; h < h_max; ++h) {
              for (unsigned int e = 0; e < m_EnergyMax.size(); ++e) {
                for (unsigned int p = 0; p < p_max; ++p) {
                  for (unsigned int t = 0; t < t_max; ++t) {
                    for (unsigned int s = 0; s < s_max; ++s) {
                      for (unsigned int a = 0; a < a_max; ++a) {
                        for (unsigned int u = 0; u < u_max; ++u) {  
                          for (unsigned int l = 0; l < l_max; ++l) {  
                            for (unsigned int f = 0; f < f_max; ++f) {  
                              for (unsigned int x = 0; x < x_max; ++x) {
                                for (unsigned int y = 0; y < y_max; ++y) {
                                  BackgroundRate_trackedcompton.Add(m_BDE[c], m_BRA[b], m_CQF[q], m_TQF[k], m_EHP[r], m_EHC[h], m_EnergyMax[e], 
                                                                    m_Phi[p], m_The[t], m_SPD[s], m_ARMorRadius[a], m_TSLMin[u], m_CSLMin[l], m_FDI[f], 
                                                                    m_PosTheta[x], m_PosPhi[y], 
                                                                    TrackedCompton_Final[GetTrackedComptonIndex(c, b, q, k, r, h, e, p, t, s, a, u, l, f, x, y)].GetBackgroundRate());
                                  EffectiveArea_trackedcompton.Add(m_BDE[c], m_BRA[b], m_CQF[q], m_TQF[k], m_EHP[r], m_EHC[h], m_EnergyMax[e], 
                                                                   m_Phi[p], m_The[t], m_SPD[s], m_ARMorRadius[a], m_TSLMin[u], m_CSLMin[l], m_FDI[f], 
                                                                   m_PosTheta[x], m_PosPhi[y],
                                                                   TrackedCompton_Final[GetTrackedComptonIndex(c, b, q, k, r, h, e, p, t, s, a, u, l, f, x, y)].GetEffectiveArea());
                                  if (TrackedCompton_Final[GetTrackedComptonIndex(c, b, q, k, r, h, e, p, t, s, a, u, l, f, x, y)].GetEffectiveArea() > 0) {
                                    Sensitivity_trackedcompton.Add(m_BDE[c], m_BRA[b], m_CQF[q], m_TQF[k], m_EHP[r], m_EHC[h], m_EnergyMax[e], 
                                                                   m_Phi[p], m_The[t], m_SPD[s], m_ARMorRadius[a], m_TSLMin[u], m_CSLMin[l], m_FDI[f], 
                                                                   m_PosTheta[x], m_PosPhi[y],
                                                                   TrackedCompton_Final[GetTrackedComptonIndex(c, b, q, k, r, h, e, p, t, s, a, u, l, f, x, y)].GetSensitivity());
                                  }
                                }
                              }
                            }
                          }
                        }
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }

  // Untracked Compton
  for (unsigned int c = 0; c < c_max; ++c) {
    for (unsigned int b = 0; b < b_max; ++b) {
      for (unsigned int q = 0; q < q_max; ++q) {
        for (unsigned int r = 0; r < r_max; ++r) {
          for (unsigned int h = 0; h < h_max; ++h) {
            for (unsigned int e = 0; e < m_EnergyMax.size(); ++e) {
              for (unsigned int p = 0; p < p_max; ++p) {
                for (unsigned int a = 0; a < a_max; ++a) {
                  for (unsigned int l = 0; l < l_max; ++l) {  
                    for (unsigned int f = 0; f < f_max; ++f) {  
                      for (unsigned int x = 0; x < x_max; ++x) {
                        for (unsigned int y = 0; y < y_max; ++y) {
                          BackgroundRate_untrackedcompton.Add(m_BDE[c], m_BRA[b], m_CQF[q], m_EHP[r], m_EHC[h], m_EnergyMax[e], 
                                                              m_Phi[p], m_ARMorRadius[a], m_CSLMin[l], m_FDI[f], 
                                                              m_PosTheta[x], m_PosPhi[y], 
                                                              UntrackedCompton_Final[GetUntrackedComptonIndex(c, b, q, r, h, e, p, a, l, f, x, y)].GetBackgroundRate());
                          EffectiveArea_untrackedcompton.Add(m_BDE[c], m_BRA[b], m_CQF[q], m_EHP[r], m_EHC[h], m_EnergyMax[e], 
                                                             m_Phi[p], m_ARMorRadius[a], m_CSLMin[l], m_FDI[f], 
                                                             m_PosTheta[x], m_PosPhi[y],
                                                             UntrackedCompton_Final[GetUntrackedComptonIndex(c, b, q, r, h, e, p, a, l, f, x, y)].GetEffectiveArea());
                          if (UntrackedCompton_Final[GetUntrackedComptonIndex(c, b, q, r, h, e, p, a, l, f, x, y)].GetEffectiveArea() > 0) {
                            Sensitivity_untrackedcompton.Add(m_BDE[c], m_BRA[b], m_CQF[q], m_EHP[r], m_EHC[h], m_EnergyMax[e], 
                                                             m_Phi[p], m_ARMorRadius[a], m_CSLMin[l], m_FDI[f], 
                                                             m_PosTheta[x], m_PosPhi[y],
                                                             UntrackedCompton_Final[GetUntrackedComptonIndex(c, b, q, r, h, e, p, a, l, f, x, y)].GetSensitivity());
                          }
                        }
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }

  Sensitivity_trackedcompton.Write(m_Name + ".tc.Sensitivity.rsp", true);
  EffectiveArea_trackedcompton.Write(m_Name + ".tc.EffectiveArea.rsp", true);
  BackgroundRate_trackedcompton.Write(m_Name + ".tc.BackgroundRate.rsp", true);

  Sensitivity_untrackedcompton.Write(m_Name + ".uc.Sensitivity.rsp", true);
  EffectiveArea_untrackedcompton.Write(m_Name + ".uc.EffectiveArea.rsp", true);
  BackgroundRate_untrackedcompton.Write(m_Name + ".uc.BackgroundRate.rsp", true);

  return true;
}

/******************************************************************************
 * Generate a list of master files, which contain only events which 
 * fullfill the requirements in the configuration file:
 */
bool SensitivityOptimizer::GenerateMasterFiles(MString Directory)
{
  

  return true;
} 

/******************************************************************************
 * Generate one master file
 */
bool SensitivityOptimizer::GenerateMasterFile(MString In, MString Out)
{
  //  MFileEventsTra InFile;
  //   if (InFile.Open(In) == false) {
  //     mlog<<"Unable to open file "<<In<<endl;
  //     return false;
  //   }
  //   MFileEventsTra OutFile;
  //   if (OutFile.Open(Out, MFile::c_Create) == false) {
  //     mlog<<"Unable to open file "<<In<<endl;
  //     return false;
  //   }

  //   // ... loop over all events and save them if they pass the event selection criteria
  //   MPhysicalEvent* Event = 0;
  //   while ((Event = InFile.GetNextEvent()) != 0) {
  //     if (m_EventSelector.IsQualifiedEvent(Event) == true) {
  //       OutFile.AddEvent(Event);
  //     }
  //     delete Event;
  //   }

  //   InFile.Close();  
  //   OutFile.Close();

  return true;
} 


/******************************************************************************
 * Find the event parameters with the optimum sensitivity 
 */
bool SensitivityOptimizer::FindOptimum()
{
  return true;
}


/******************************************************************************/

SensitivityOptimizer* g_Prg = 0;

/******************************************************************************/


/******************************************************************************
 * Called when an interrupt signal is flagged
 * All catched signals lead to a well defined exit of the program
 */
void CatchSignal(int a)
{
  mlog<<"Catched signal Ctrl-C (ID="<<a<<"):"<<endl;
  
  if (g_Prg != 0) {
    g_Prg->Interrupt();
  }
}


/******************************************************************************
 * Main program
 */
int main(int argc, char** argv)
{
  // Catch a user interupt for graceful shutdown
  signal(SIGINT, CatchSignal);

  // Initialize global MEGALIB variables, especially mgui, etc.
  MGlobal::Initialize("Sensitivity Optimizer");

  TApplication SensitivityOptimizerApp("SensitivityOptimizerApp", 0, 0);

  g_Prg = new SensitivityOptimizer();

  if (g_Prg->ParseCommandLine(argc, argv) == false) {
    cerr<<"Error during parsing of command line!"<<endl;
    return -1;
  } 
  if (g_Prg->Analyze() == false) {
    cerr<<"Error during analysis!"<<endl;
    return -2;
  } 

  //SensitivityOptimizerApp.Run();

  mlog<<"Program exited normally!"<<endl;

  return 0;
}

/*
 * Cosima: the end...
 ******************************************************************************/
