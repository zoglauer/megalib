/*
 * MCEventAction.hh
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */



/******************************************************************************
 *
 * The event action implementation
 *
 */

#ifndef ___MCEventAction___
#define ___MCEventAction___

// Geant4:
#include "G4UserEventAction.hh"
#include "globals.hh"

// Standard lib:
#include <fstream>
#include <map>
#include <string>
using namespace std;

// Cosima:
#include "MC2DStripHit.hh"
#include "MCScintillatorHit.hh"
#include "MCCalorBarHit.hh"
#include "MCDriftChamberHit.hh"
#include "MCAngerCameraHit.hh"
#include "MCVoxel3DHit.hh"
#include "MCParameterFile.hh"

// MEGAlib:
#include "MFile.h"
#include "MTimer.h"
#include "MTransceiverTcpIp.h"
#include "MSimEvent.h"
#include "MDTriggerUnit.h"


/******************************************************************************/

class MCEventAction : public G4UserEventAction
{
  // public interface:
public:
  /// Constructor taking the run parameters as argument
  MCEventAction(MCParameterFile& RunParameters, const bool Zip, 
                const long Seed);
  /// Default destructor
  virtual ~MCEventAction();

  /// Set the collection ID of the hits
  void SetCollectionName(G4String Name, int Type);

  /// Set the function to which we relegate the events
  void SetRelegator(void (Relegator)(MSimEvent*));
  
  /// Prepare for next run: Open the output file and reset
  bool NextRun();

  /// Things to do before the start of a new event (reset last event data) 
  void BeginOfEventAction(const G4Event* Event);
  /// Test for trigger and store all events
  void EndOfEventAction(const G4Event* Event);

  /// Remove the last events data
  void Reset();
  /// Add an IA to this event:
  void AddIA(G4String ProcessID, 
             int InteractionID,
             int OriginID,
             int DetectorID,
             double Time,
             G4ThreeVector Position,
             int MotherParticleID,
             G4ThreeVector NewMotherMomentumDirection,
             G4ThreeVector NewMotherPolarization,
             double NewMotherKineticEnergy,
             int SecondaryParticleID,
             G4ThreeVector SecondaryMomentumDirection,
             G4ThreeVector SecondaryPolarization,
             double SecondaryEnergy);
  /// Add a passive material output information
  void AddDepositPassiveMaterial(double Energy, string MaterialName);

  /// Interrupts the executon at the end of the next event
  void Interrupt() { m_Interrupt = true; }

  // protected methods:
protected:
  /// Write the file header 
  bool WriteFileHeader(double ObservationStartTime);

  /// Save the event to file (only saves the event if we really want to)
  bool SaveEventToFile(MSimEvent* Event);
  /// Transmit event via TCP/IP (only transmit it if we have an open transceiver)
  bool TransmitEvent(MSimEvent* Event);

  // protected members:
protected:


  // private members:
private:
  /// Class containing all run parameter 
  MCParameterFile& m_RunParameters;

  /// Invalid collection ID 
  static const int c_InvalidCollID;

  /// Id of the tracker collection
  vector<G4String> m_2DStripCollNames;
  /// Id of the calorimeter collection
  vector<G4String> m_CalorimeterCollNames;
  /// Id of the 3Dstrip collection
  vector<G4String> m_3DStripCollNames;
  /// Id of the Voxel3D collection
  vector<G4String> m_Voxel3DCollNames;
  /// Id of the szintillator collection
  vector<G4String> m_ScintillatorCollNames;
  /// Id of the drift chamber collection
  vector<G4String> m_DriftChamberCollNames;
  /// Id of the Anger camera collection
  vector<G4String> m_AngerCameraCollNames;

  /// True if the evnts should be saved to file
  bool m_SaveEvents;
  /// Stream to the output file
  MFile m_OutFile;
  /// Name of the current iutput file
  string m_OutFileName;
  /// Id of the output file, if the maximum file size has been exceeded
  int m_FileNumber;
  /// Incarnation Id of the output file, if the base file name already exists
  int m_Incarnation;

  /// True if the evnts should be saved to file
  bool m_TransmitEvents;
  /// TCP/IP: The name of the output host
  string m_Host;
  /// TCP/IP: The port on the host
  //int m_Port;
  /// The transceiver
  MTransceiverTcpIp m_Transceiver;
  
  /// True if the events should be handed over to a function
  bool m_RelegateEvents;
  /// The function pointer for relegation
  void (*m_Relegator)(MSimEvent*);
  
  /// Seed of the random number generator at the beginning of the event
  long m_Seed;

  /// Parallel ID: Additional ID assigned from outside to uniquely identify sim files from one batch of parallel started simulations 
  int m_ParallelID;
  /// Assigned fixed incarnation ID of the output file(s)
  //int m_IncarnationID;

  /// The temporary store of the simulated event
  MSimEvent* m_Event;

  /// Determines how much detail shall be stored in the simulations file
  unsigned int m_StoreSimulationInfo;
  /// Determines in which version the output file shall be stored
  unsigned int m_StoreSimulationInfoVersion;
  /// True if hits as posiiton/enrgy and not as bars/strips should be written to the file  
  G4bool m_StoreCalibrated;
  /// True if only events with which have triggered should be stored
  G4bool m_StoreOnlyTriggeredEvents;
  /// True if each hit in its own event
  G4bool m_StoreOneHitPerEvent;

  /// Precision (number of digits after '.') for scientific storage
  int m_StoreScientificPrecision;

  /// A timer stopping the real event action time
  MTimer m_Timer;
  /// The total time spent during all event actions:
  double m_TotalTime;
  /// Flag indicating if the timer has started
  bool m_TimerStarted;

  /// Map of passive material name and deposited energy
  map<string, double> m_PassiveMaterialMap; 

  /// True, if the run should be terminated at the end of this event
  bool m_Interrupt;
  /// True, if the sim file should be zip after its generation
  bool m_Zip;

  /// Store the trigger unit for fast access
  MDTriggerUnit* m_TriggerUnit;

};

#endif

   
/*
 * MCEventAction.hh: the end...
 ******************************************************************************/
 
