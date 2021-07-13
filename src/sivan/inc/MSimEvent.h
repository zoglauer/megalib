/*
 * MSimEvent.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MSimEvent__
#define __MSimEvent__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <vector>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MSimIA.h"
#include "MTime.h"
#include "MSimHT.h"
#include "MSimDR.h"
#include "MSimGR.h"
#include "MSimPM.h"
#include "MSimCluster.h"
#include "MDGeometryQuest.h"
#include "MPhysicalEvent.h"
#include "MVector.h"
#include "MStreams.h"
#include "MRotationInterface.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MSimEvent : public MRotationInterface
{
  // public interface:
 public:
  //! Standard constructor
  MSimEvent();
  //! Copy constructor
  MSimEvent(const MSimEvent& Event);
  // Default destructor
  virtual ~MSimEvent();

  //! Get the output version, especially of the output strings
  //! Irrelevant of the input version, the output version will always be the same
  static int GetOutputVersion() { return s_Version; }

  //! Set the geometry
  void SetGeometry(MDGeometryQuest* Geo);

  //
  // Interface to set and get all RAW data
  // 

  //! Resets all data to default values:
  void Reset();

  //! Set the event ID
  void SetID(const long ID) { m_NEvent = ID; }
  //! Get the event ID
  long GetID() const { return m_NEvent; }

  //! Set the simulation event ID of this event, i.e. the started number of events
  void SetSimulationEventID(long ID) { m_NStartedEvent = ID; }
  //! Get the simulation event ID of this event, i.e. the started number of events  
  long GetSimulationEventID() const { return m_NStartedEvent; }

  //! Set the time of the event
  void SetTime(const MTime Time) { m_Time = Time; }
  //! Get the time of the event
  MTime GetTime() const { return m_Time; }

  //! Add the content of the given event to this one
  bool Add(const MSimEvent& Event);
  
  //! Parse a single line from a sim file to this event
  bool ParseLine(MString Line, int Version = 1);
  bool AddRawInput(MString Line, int Version = 1) { return ParseLine(Line, Version); }
  
  //! Parse a full (i.e. multi-line) event from a sim file to this event
  bool ParseEvent(MString Line, int Version = 1);  
  
  //! Add an interaction to this event
  bool AddIA(const MSimIA& IA);
  //! Add an interaction to this event --- the event will delete this IA!
  void AddIA(MSimIA* IA) { if (IA != 0) m_IAs.push_back(IA); }
  //! Return the number of stored interactions (MSimIA)
  unsigned int GetNIAs();
  //! Get an IA by vector position
  MSimIA* GetIAAt(unsigned int i);
  //! Get an IA by its ID 
  MSimIA* GetIAById(int i);

  //! Add a hit to this event
  bool AddHT(const MSimHT& HT);
  //! Add a hit to this event --- the event will delete this HT!
  void AddHT(MSimHT* HT) { if (HT != 0) m_HTs.push_back(HT); } 
  //! Return the number of stored hits (MSimHT)
  unsigned int GetNHTs();
  //! Get a hit by its position in the storage vector
  MSimHT* GetHTAt(unsigned int i);
  //! Remove the given hit from the event - the HT is not deleted!
  void RemoveHT(MSimHT* HT);
  //! Remove all but the given hit from the event - the HT's are not deleted!
  void RemoveAllHTsBut(MSimHT* HT);
  //! Remove all hits from the event - the HT's are not deleted!
  void RemoveAllHTs();
  //! Delete all HTs
  void DeleteAllHTs();

  //! Add a directional information
  bool AddDR(const MSimDR& DR);
  //! Get the number of directional informations
  unsigned int GetNDRs();
  //! Get a directional information by its position in the storage vector
  MSimDR* GetDRAt(unsigned int i);

  //! Add an guard ring hit
  bool AddGR(const MSimGR& GR);
  //! Add a guardring hit to this event --- the event will delete this GR!
  void AddGR(MSimGR* GR) { if (GR != 0) m_GRs.push_back(GR); }
  //! Get the number of guard ring hits
  unsigned int GetNGRs();
  //! Get a guard-ring hit by its position in the storage vector
  MSimGR* GetGRAt(unsigned int i);

  //! Add a passive material summary information 
  bool AddPM(const MSimPM& PM);
  //! Add a passive material summary information --- the event will delete this GR!
  void AddPM(MSimPM* PM) { if (PM != 0) m_PMs.push_back(PM); }
  //! Get the number of stored passive material summary information
  unsigned int GetNPMs();
  //! Get a passive material summary information by its position in the storage vector
  MSimPM* GetPMAt(unsigned int);

  //! Add a bad event flag
  bool AddBD(const MString& Flag);
  //! Get the number of stored bad event flags
  unsigned int GetNBDs();
  //! Get a bad event flag
  MString GetBDAt(unsigned int);

  //! Add a comment
  bool AddCC(const MString& Comment);
  //! Get the number of stored comments
  unsigned int GetNCCs();
  //! Get a comment
  MString GetCCAt(unsigned int);

  // Some constants indicating what to store

  /// Flag indicating all simulation info is stored
  static const int c_StoreSimulationInfoAll;
  /// Flag indicating only simulation info of hits in active material are stored --- not used right now
  static const int c_StoreSimulationInfoDepositsOnly; 
  /// Flag indicating only INIT simulation info is stored
  static const int c_StoreSimulationInfoInitOnly; 
  /// Flag indicating NO simulation info is stored
  static const int c_StoreSimulationInfoNone;
  /// Flag indicating only IA info is stored
  static const int c_StoreSimulationInfoIAOnly;

  //! Convert the content to string
  MString ToString();
  //! Convert the content to string for the simulation file, where 
  //! WhatToStore is one of c_StoreSimulationInfoAll, c_StoreSimulationInfoDepositsOnly, c_StoreSimulationInfoInitOnly, c_StoreSimulationInfoNone
  //! Precision represents the number of digits to use, and
  //! Version is the version of the simulation output file
  MString ToSimString(const int WhatToStore = c_StoreSimulationInfoAll, const int Precision = 5, const int Version = 25);


  //
  // The OLD/ANTIQUE analysis interface:
  //
  
  bool Analyze();
  bool Discretize(int Detector);
  // 
  int GetEventType();
  int GetEventLocation();

  int GetICFirstIADetector();
  int GetICSecondIADetector();
  int GetICThirdIADetector();

  int GetIEventRetrieval();
  int GetREventRetrieval();

  MVector GetICFirstIA();
  MVector GetICSecondIA();
  MVector GetICElectronD();
  MVector GetICPhotonD();

  double GetICEnergyGamma();
  double GetICEnergyElectron();

  double GetICEnergy();
  MVector GetICOrigin();

  double GetICScatterAngle();

  double GetTOF();

  bool GetVeto();
  bool HasTrack(int NLayers = 3);
  bool IsTriggered();
  bool IsElectronContainedInD1();

  MVector GetIPVertex();
  MVector GetIPElectronDir();
  MVector GetIPPositronDir();
  double GetIPElectronEnergy();
  double GetIPPositronEnergy();


  MVector GetRCFirstIA();
  MVector GetRCFirstIAClustered();
  MVector GetRCSecondIA();
  MVector GetRCSecondIAClustered();
  MVector GetRCElectronD(int NLayers = 3);
  bool IsCSecondIAD1();

  double GetRCEnergyGamma();
  double GetRCEnergyElectron();

  double GetRCEnergy();
  double GetREnergy();

  double GetREnergyD1();
  double GetREnergyD2();

  MVector GetRCentralHitD1();
  MVector GetRCentralHitD2();

  int GetRNHitsD1();
  int GetRNHitsD2();

  MVector GetRPIA();
  MVector GetRPElectronD();
  MVector GetRPPositronD();
  
  double GetRPEnergyElectron();
  double GetRPEnergyPositron();

  double GetRPInitialEnergyDeposit();

  bool IsIAOrigin(MSimIA* IA, int Origin);
  vector<int> GetAllHTOrigins();

  //! Since storing the simulation data doubles the read-time, it is only stored if requested:
  void StoreSimulationData(bool Flag) { m_StoreSimulationData = Flag; }
  //! Retrieve the simulation data
  MString GetSimulationData();

  int GetNTriggeredLayers();
  int GetLengthOfFirstComptonTrack();

  double GetRFirstEnergyDepositElectron();
  double GetEnergyDepositNotSensitiveMaterial();
/*   void SetEnergyDepositNotSensitiveMaterial(double Energy); */

  //! Return the total energy deposit (after noising)
  double GetTotalEnergyDeposit();
  //! Return the total energy deposit (before noising)
  double GetTotalEnergyDepositBeforeNoising();
  
  //! Return true if the event is completely absorbed (the energy deposit before noising is used!)
  bool IsCompletelyAbsorbed(double AbsoluteTolerance = 0.1);

  //! Check if the IA is completely absorbed (includes descendents)
  int IsIACompletelyAbsorbed(int IA, double AbsoluteTolerance = 0.0, double RelativeTolerance = 0.0);

  //! Get all descendents, i.e. all interactions originating from this one
  //! Returns a list of IA IDs including the original one!
  vector<int> GetDescendents(int IAId);

  //! Returns true, if in all hits originating from the IA "Id" only descendents
  //! interactions are present, e.g. deposits from the recoil electron and eventually
  //! Bremsstrahlung, but not another Compton interaction
  bool IsIAResolved(int IAId);

  //! Returns a list of all HTs originating from IA and its descendents
  vector<MSimHT*> GetHTListIncludingDescendents(int IAId);

  int GetLengthFirstTrack();

  unsigned int GetNClusters();
  MSimCluster* GetClusterAt(unsigned int i);
  void AddCluster(MSimCluster* Cluster);
  double GetAverageClusterSize(int Detector);

  double GetAverageEnergyDeposit(int Detector);
  
  //! Return the total energy deposited in all guard rings
  double GetGuardRingEnergy();

  //! Return the number of ignored hits (below threshold, no detector)
  int GetNIgnoredHTs() const { return m_NIgnoredHTs; }

  MPhysicalEvent* GetPhysicalEvent(bool Ideal = false);

  void CreateClusters();

  //! Create a set of sim events which contain only one hit per event
  //! The user has to take care of deleting the events!
  vector<MSimEvent*> CreateSingleHitEvents();


  // Depreciated:

  void SetEventNumber(long EventNumber) { mdep<<"Please use: SetID"<<show; m_NEvent = EventNumber; }
  long GetEventNumber() { mdep<<"Please use: GetID"<<show; return m_NEvent; }

  void SetId(const long Id) { mdep<<"Please use: SetID"<<show; m_NEvent = Id; }
  long GetId() const { mdep<<"Please use: GetID"<<show; return m_NEvent; }

  void SetStartedEventNumber(long EventNumber) { mdep<<"Please use: SetSimulationEventID"<<show; SetSimulationEventID(EventNumber); } ;
  long GetStartedEventNumber() { mdep<<"Please use: GetSimulationEventID"<<show; return GetSimulationEventID(); };


  // public constants
 public:
  enum MSimEventType { Unknown, Compton, Pair };  
  enum MSimFirstIALoctation { NotSensitive, D1, D2, D3, D4, D5, D6, D7, D8, D9, D10 };  
  enum MSimIdealRetrieval { IdealRetrievalGood, IdealRetrievalBad };  
  enum MSimRealRetrieval { RealRetrievalGood, RealRetrievalBad };  

  // protected methods:
 protected:
  bool CheckEvent();

  void DetermineEventType();
  void DetermineEventRetrieval();
  void DetermineEventLocation();



  // private methods:
 private:


  // protected members:
 protected:


  // private members:
 private:
  //! Array of interactions
  vector<MSimIA*> m_IAs;         
  //! Array of hits
  vector<MSimHT*> m_HTs; 
  //! Array of clusters
  vector<MSimCluster*> m_Clusters;   
  //! Array of directional informations for Strip3DDirectional detectors
  vector<MSimDR*> m_DRs;   
  //! Array of guard ring hits
  vector<MSimGR*> m_GRs;   
  //! Array of passive material deposits
  vector<MSimPM*> m_PMs;   
  //! Array of bad event flags
  vector<MString> m_BDs;   
  //! Array of bad event flags
  vector<MString> m_CCs;   

  //! Event ID
  unsigned long m_NEvent;
  //! Number of simulated events up to this event ID
  unsigned long m_NStartedEvent;    

  // Special measurements: 
  //! Time of the (start) of the event
  MTime m_Time;          
  //! Time of flight between D1 and D2
  double m_TOF;

  //! Flag of firing veto dome(s)
  bool m_Veto;            
  //! Total energy deposit in the detector at position MVector (currently only used for drift chambers)
  map<MVector, double> m_TotalDetectorEnergy;


  //! If true store simulation data
  bool m_StoreSimulationData;
  //! Input data string / stream / buffer - only filled if the data is read from file and the StoreSimulationData flag is true
  MString m_SimulationData; 

  //! Event type (Compton/Pair)
  int m_EventType;        
  //! Location first interaction (as detector type)
  int m_EventLocation;    
  //! Are there two real hits of the scattered gamma ray?
  int m_REventRetrieval;  
  //! Are there two ideal hits of the scattered gamma ray?
  int m_IEventRetrieval;  

  //! Pointer to the geometry
  MDGeometryQuest* m_Geometry;
  //! Version of the input sim file
  int m_Version;            
  //! Version of the default output sim file
  static const int s_Version;            

  //! Number of ignored hits (energy below threshold, no detector)
  int m_NIgnoredHTs;

  
  
#ifdef ___CLING___
 public:
  ClassDef(MSimEvent, 0)    // representation of a simulated event
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
