/*
 * MCRun.hh
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */



/******************************************************************************
 *
 * Class representing a run with individual sources
 *
 */

#ifndef ___MCRun___
#define ___MCRun___

// Cosima:
#include "MCSource.hh"
#include "MCOrientation.hh"
#include "MCIsotopeStore.hh"

// Geant4:
#include "G4ParticleDefinition.hh"
#include "G4Event.hh"
#include "G4GeneralParticleSource.hh"
#include "G4Ions.hh"

// Root:
#include <MString.h>

// Standard lib:
#include <vector>
#include <set>
#include <functional>
using namespace std;


/******************************************************************************/

struct NextEmissionSort {
public:
  bool operator() (const MCSource* S1, const MCSource* S2) const {

    if (S1->GetID() == S2->GetID()) return false;
    //return S1 < S2;
    ///cout<<"Sorting: "<<S1->GetName()<<": "<<S1->GetNextEmission()<<" - "<<S2->GetName()<<": "<<S2->GetNextEmission()<<endl;
    if (S1->GetNextEmission() == S2->GetNextEmission()) {
      //cout<<"Returning A:"<<((S1 < S2) ? "true" : "false")<<endl;
      return S1 < S2;
    } else {
      //cout<<"Returning B:"<<((S1->GetNextEmission() < S2->GetNextEmission()) ? "true" : "false")<<endl;
      return (S1->GetNextEmission() < S2->GetNextEmission());
    }
  }
};

class MCRun
{
  // public interface:
public:
  /// Default constructor
  MCRun();
  /// Default destructor
  virtual ~MCRun();

  /// Some preparation before we can start the run:
  bool Initialize();

  /// Set the name of the Run
  void SetName(const MString& Name) { m_Name = Name; }
  /// Return the name of this run
  MString GetName() const { return m_Name; }

  /// Set the parallel ID of this run - call CheckIncarnation() afterwards
  void SetParallelID(const int ParallelID) { m_ParallelID = ParallelID; }
  /// Get the parallel ID of this run
  int GetParallelID() const { return m_ParallelID; }

  /// Set a FIXED incarnation ID of this run if the IncarnationID is larger than 0, otherwise it is detrmined automatically - call CheckIncarnation() afterwards
  void SetIncarnationID(const int IncarnationID) { m_IncarnationID = IncarnationID; if (m_IncarnationID > 0) m_IsIncarnationIDFixed = true; else m_IsIncarnationIDFixed = false; }
  /// Get the incarnation of this run --- will obviously only give final results if ParallelID and FileName are set!
  int GetIncarnationID() { return m_IncarnationID; }

  /// Set the file name of this run. Returns always true - call CheckIncarnation() afterwards
  bool SetFileName(const MString& FileName) { m_FileName = FileName; /* CheckIncarnationID(); */ return true; }
  /// Returns the name of the underlaying file (empty string if there is none, i.e. no saving of the file is wished)
  MString GetFileName() const { return m_FileName; }
  
  /// Set if we want to zip the file - call CheckIncarnation() afterwards
  bool SetZip(const bool Zip) { m_Zip = Zip; return true; }
  /// Return if we want to zip the file
  bool GetZip() const { return m_Zip; }
  
  /// Set if we want to store the data in binary format
  bool SetStoreBinary(const bool StoreBinary) { m_StoreBinary = StoreBinary; return true; }
  /// Return if we want to store the data in binary format
  bool GetStoreBinary() const { return m_StoreBinary; }
  
  /// Check the incarnation this run --- will obviously only give final results if ParallelID and FileName are set!
  void CheckIncarnationID();

  /// Set the name of TCP/IP host. Returns always true
  bool SetTcpIpHostName(const MString& TcpIpHostName) { m_TcpIpHostName = TcpIpHostName; return true; }
  /// Retuns the name of TCP/IP host (empty string if there is none, i.e. no TCP/IP connection is wished)
  MString GetTcpIpHostName() const { return m_TcpIpHostName; }

  /// Set the TCP/IP port
  bool SetTcpIpPort(const unsigned int TcpIpPort) { m_TcpIpPort = TcpIpPort; return true; }
  /// Retuns the TCP/IP port (empty string if there is none, i.e. no TCP/IP connection is wished)
  unsigned int GetTcpIpPort() const { return m_TcpIpPort; }

  /// Set the revan configuration file name. Returns always true
  bool SetRevanConfigurationFileName(const MString& RevanConfigurationFileName) { m_RevanConfigurationFileName = RevanConfigurationFileName; return true; }
  /// Retuns the revan configuration file name (empty string if there is none, i.e. no reconstruction connection is wished)
  MString GetRevanConfigurationFileName() const { return m_RevanConfigurationFileName; }

  /// Set the tra file name. Returns always true
  bool SetTraFileName(const MString& TraFileName) { m_TraFileName = TraFileName; return true; }
  /// Retuns the tra file name (empty string if there is none, i.e. no reconstruction connection is wished)
  MString GetTraFileName() const { return m_TraFileName; }


  /// Set the name of the geometry file
  bool SetGeometryFileName(const MString& Name);
  /// Return the nme of the geometry file
  MString GetGeometryFileName() const { return m_GeometryFileName; }

  /// Set the activation mode 
  bool SetActivationMode(const int ActivationMode);

  /// Set the duration (in seconds) of this run
  bool SetDuration(const double& Duration);
  /// Return the duration (in seconds) of this run  
  double GetDuration() const { return m_Duration; }
  /// Set the maximum number of triggers for this run
  bool SetTriggers(const long& Triggers);
  /// Return the maximum number of triggers for this run
  long GetTriggers() const { return m_Triggers; }
  /// Set the maximum number of events for this run
  bool SetEvents(const long& Events);
  /// Return the maximum number of events for this run
  long GetEvents() const { return m_Events; }
  
  /// Set the sky-orientation
  bool SetSkyOrientation(const MCOrientation& Orientation) { m_SkyOrientation = Orientation; return true; }
  /// Return the sky orientation
  MCOrientation GetSkyOrientation() const { return m_SkyOrientation; }
  /// Return the sky orientation as const reference
  const MCOrientation& GetSkyOrientationReference() const { return m_SkyOrientation; }
  
  /// Set the detector orientation
  bool SetDetectorOrientation(const MCOrientation& Orientation) { m_DetectorOrientation = Orientation; return true; }
  /// Return the detector orientation
  MCOrientation GetDetectorOrientation() const { return m_DetectorOrientation; }
  /// Return the detector orientation as const reference
  const MCOrientation& GetDetectorOrientationReference() const { return m_DetectorOrientation; }
  
  /// Return the stop condition type
  int GetStopCondition() const { return m_StopCondition; }

  /// Add a source to this run (by giving a name). If Test is true, a test is performed whether a source with this name already exists.
  bool AddSource(const MString& Name, bool Test = false); 
  /// Add a source to this run. If Test is true, a test is performed whether a source with this name already exists.
  bool AddSource(MCSource* Source, bool Test = false); 
  /// Get the list of sources of this run
  vector<MCSource*>& GetSourceList() { return m_SourceList; }
  /// Get the number of sources in this run
  unsigned int GetNSources() { return m_SourceList.size(); }
  /// Get a specific source
  MCSource* GetSource(unsigned int i);

  /// Add an event to an event list (used for activation simulation)
  bool AddToBuildUpEventList(double Energy, G4ThreeVector Position, G4ThreeVector Direction, G4ThreeVector Polarization, long double Time, G4ParticleDefinition* ParticleType, MString VolumeName);

  /// Skip one event in the activation source given the particle type and volume name 
  bool SkipOneEvent(G4ParticleDefinition* ParticleType, MString VolumeName); 

  /// Add an element to the radioactive particle store
  void AddIsotope(G4Ions* Ion, G4TouchableHistory* Hist);
  /// Set isotope store name
  void SetIsotopeStoreFileName(MString Name) { m_IsotopeStoreName = Name; }
  /// Add an element to the radioactive particle store
  void SaveIsotopeStore();

  /// Set the current time
  void SetSimulatedTime(long double Time) { m_SimulatedTime = Time; }
  /// Set the current time
  long double GetSimulatedTime() const { return m_SimulatedTime; }

  /// Set the number of generated particles in the particle source
  void SetNGeneratedParticles(int NGeneratedParticles) { m_NGeneratedParticles = NGeneratedParticles; }
  /// Add a number of generated particles in the particle source
  void AddNGeneratedParticles(int NGeneratedParticles) { m_NGeneratedParticles += NGeneratedParticles; }
  /// Get the number of generated particles in the particle source
  int GetNGeneratedParticles() const { return m_NGeneratedParticles; }

  /// Add a number of simulated events
  void AddSimulatedEvent() { ++m_NSimulatedEvents; }
  /// Get the number of simulated events
  long GetNSimulatedEvents() const { return m_NSimulatedEvents; }

  /// Add a number of triggered events
  void AddTriggeredEvent() { ++m_NTriggeredEvents; }
  /// Get the number of triggered events
  long GetNTriggeredEvents() const { return m_NTriggeredEvents; }

  /// Generate all primary particles for the primary generator action
  /// The function is only to be called by MCPrimaryGeneratorAction::GeneratePrimaries()
  void GeneratePrimaries(G4Event* Event, G4GeneralParticleSource* ParticleSource);

  /// Determine the next emission for all sources  
  void DetermineNextEmissions();

  /// Check all stop conditions
  bool CheckStopConditions();

  /// Stop this run
  void Stop();

  /// Print some run statics to cout
  void DumpRunStatistics(double CPUTime = 0.0);

  /// ID for number of triggers stop condition
  static const int c_StopByTriggers;
  /// ID for number of events stop condition
  static const int c_StopByEvents;
  /// ID for time stop condition
  static const int c_StopByTime;



  // protected methods:
protected:


  // protected members:
protected:


  // private members:
private:
  /// Name of this run
  MString m_Name;
  /// Name of the underlaying file (empty if no saving to file is wished)
  MString m_FileName;
  /// Name of the geometry file
  MString m_GeometryFileName;

  /// Do we want to zip the output files
  bool m_Zip;
  
  /// Do we wnat to store the data in binary format
  bool m_StoreBinary;
  
  /// If multiple simulations are started in parallel, this is the ID 
  int m_ParallelID;
  /// If multiple filenames exists, this is the ID of the current incarnation
  int m_IncarnationID;
  /// True if the incarnation ID is fixed
  bool m_IsIncarnationIDFixed;

  /// The TCP/IP host name (empty if no connection is wished)
  MString m_TcpIpHostName;
  /// The TCP/IP port
  unsigned int m_TcpIpPort;
  
  /// The ravn configuration file name (empty if to reconstruction is desired)
  MString m_RevanConfigurationFileName;
  /// The tra file name to save the reconstructed events to
  MString m_TraFileName;

  /// List of sources
  vector<MCSource*> m_SourceList;

  /// The detector orientation
  MCOrientation m_DetectorOrientation;
  /// The sky orientation
  MCOrientation m_SkyOrientation;
  
  /// Map indicating, when we have the next emission for the source
  set<MCSource*, NextEmissionSort> m_NextEmission;
  //set<MCSource*> m_NextEmission;
  /// Scaling factor for next emission time calculation
  double m_NextEmissionScale;

  /// Duration of this run in seconds
  long double m_Duration;
  /// Maximum bnumber of triggered events
  long m_Triggers;
  /// Maximum number of events
  long m_Events;

  /// Type of the stop condition for this run
  int m_StopCondition;
  
  /// Activation mode (normal, build-up, decay)
  int m_ActivationMode;

  /// The file name of the isotope store
  MString m_IsotopeStoreName;
  /// True if the isotope store name has been updated with parrallel ID and incarnation ID
  bool m_IsotopeStoreNameUpdated;
  /// The radioactive particle store:
  MCIsotopeStore m_IsotopeStore;
  /// The number of added isotopes to the store
  unsigned int m_NAddedIsotopes;

  /// Simulated time
  long double m_SimulatedTime;

  /// Number of simulated events
  long m_NSimulatedEvents;

  /// Number of (pre-) triggered events
  long m_NTriggeredEvents;

  /// Number of generated particles (one event can have more than one)
  long m_NGeneratedParticles;

  /// Number of events which have been skipped (only required for speed optimizations)
  unsigned long m_NSkippedEvents;
};

#endif


/*
 * MCRun.hh: the end...
 ******************************************************************************/
