/*
 * MResponseBuilder.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MResponseBuilder__
#define __MResponseBuilder__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MGeometryRevan.h"
#include "MRESE.h"
#include "MRERawEvent.h"
#include "MRawEventAnalyzer.h"
#include "MSimEvent.h"
#include "MFileEventsSim.h"
#include "MEventSelector.h"
#include "MSettingsRevan.h"
#include "MSettingsMimrec.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////

//! Base class for the response generation
//! This file provides functions to get the events as well as basic analysis functions
//! for the response generation
//! We have to modes here:
//! (1) If the data file is given, then analyze all events
//! (2) If SetEvent is used analyze one event
class MResponseBuilder
{
  // public interface:
 public:
  //! Default constructor for base class for all response creation classes
  MResponseBuilder();
  //! Default destructor
  virtual ~MResponseBuilder();

  //! Initialize the response matrices and their generation
  virtual bool Initialize();

  //! Analyze one event
  virtual bool Analyze();
    
  //! Finalize the response generation (i.e. save the data a final time )
  virtual bool Finalize();
  
  //! Raises the interrupt flag
  void Interrupt() { m_Interrupt = true; }

  //! Set the response name (used for the file name prefix)
  void SetResponseName(const MString Name);
  
  //! Set and verify the geometry file name
  bool SetGeometryFileName(const MString FileName);
  
  //! Set the response file axis dimensions
  bool SetResponseLayoutFileName(const MString FileName);
  
  //! Set, verify, and open the data file name (sim or evta) - this sets the input mode to file 
  bool SetEvent(const MString String, bool NeedsNoising, int Version);
  
  //! Set, verify, and open the data file name (sim or evta) - this sets the input mode to file 
  bool SetDataFileName(const MString FileName);

  //! Set and verify the revan settings file name
  bool SetRevanSettingsFileName(const MString FileName);
  //! Set and verify the mimrec settings file name
  bool SetMimrecSettingsFileName(const MString FileName);

  //! Set (overwrite) the ID for the start event
  void SetStartEventID(const unsigned int ID) { m_StartEventID = ID; }
  
  //! Set the maximum number of analyzed event
  void SetMaxNumberOfEvents(const unsigned int Number) { m_MaxNEvents = Number; }

  //! Compress the final response files
  void SetCompression(const bool Compress) { m_Compress = Compress; if (m_Compress == true) m_Suffix = ".rsp.gz"; else m_Suffix = ".rsp"; }
  //! Safe the response after this amount of events are stored
  void SetSaveAfterNumberOfEvents(const unsigned int Number) { m_SaveAfter = Number; }

 
  // protected methods:
 protected:

  //! Save the response matrices
  virtual bool Save();

  //! Load the geometry, return 0 on failure
  MGeometryRevan* LoadGeometry(bool ActivateNoise, double GlobalFailureRate);

  //! Get the output file prefix
  MString GetFilePrefix() const;

  //! Initialize the next sivan/revan matching event for response creation
  bool InitializeNextMatchingEvent();
  
  
  //! Do a sanity check if the simulations are usable for this task
  bool SanityCheckSimulations();

  //! Return a list of REVAN IDs of that RESE
  vector<int> GetReseIds(MRESE* RESE);
  //! Return a list of Sivan Origin IA IDs for the given RESE
  vector<int> GetOriginIds(MRESE* RESE);

  //! Check if IDs (RESE or origins?) are in sequence
  bool AreIdsInSequence(const vector<int>& Ids);

  //! Create a log axis for the response file
  vector<float> CreateLogDist(float Min, float Max, int Bins, 
                              float MinBound = c_NoBound, 
                              float MaxBound = c_NoBound,
                              float Offset = 0, bool Inverted = false);

  //! Create a log axis with minimum bin size threshold for the response file
  vector<float> CreateThresholdedLogDist(float Min, float Max, int Bins, float Threshold, 
                                         float MinBound = c_NoBound, 
                                         float MaxBound = c_NoBound,
                                         float Offset = 0, bool Inverted = false);

  //! Create a standard axis for the response file
  vector<float> CreateEquiDist(float Min, float Max, int Bins, 
                               float MinBound = c_NoBound, 
                               float MaxBound = c_NoBound,
                               float Offset = 0, bool Inverted = false);


  // private methods:
 private:



  // protected members:
 protected:
  //! Flag indicating that an interrupt should be performed
  bool m_Interrupt;

  //! Name of the simulation file
  MString m_DataFileName;
  //! Name of the geometry file
  MString m_GeometryFileName;
  //! Name of the response
  MString m_ResponseName;
  //! A lower case response suffix unique for all response names, which is added to the file names
  MString m_ResponseNameSuffix;

  //! Revan settings file name
  MString m_RevanSettingsFileName;
  //! The revan settings file
  MSettingsRevan m_RevanSettings;
  
  //! Mimrec settings file name
  MString m_MimrecSettingsFileName;
  //! The mimrec settings file
  MSettingsMimrec m_MimrecSettings;

  //! ID for the start event
  unsigned long m_StartEventID;
  //! Maximum number of events to be analyzed 
  unsigned long m_MaxNEvents;
  //! Safe the response after this amount of events are stored
  unsigned long m_SaveAfter;
  //! Compress the response files
  bool m_Compress;
  //! The file suffix 
  MString m_Suffix;
  
  //! The number of simulated events
  long m_NumberOfSimulatedEventsClosedFiles;
  //! The number of simulated events
  long m_NumberOfSimulatedEventsThisFile;
  
  //! Flag indicating that the only the initial IA is required
  bool m_OnlyINITRequired;


  // Sivan/Revan interface:

  //! The available modes
  enum class MResponseBuilderReadMode : int { File, EventByEvent };
  
  //! The used read mode
  MResponseBuilderReadMode m_Mode = MResponseBuilderReadMode::File;
  
  //! A counter for all analyzed events
  unsigned long m_Counter;
  
  //! The current sivan event
  MSimEvent* m_SiEvent;
  //! The current revan event
  MRERawEvent* m_ReEvent;  
  
  //! For read-mode file: The revan reader
  MRawEventAnalyzer* m_ReReader;
  //! For read-mode file: The sivan reader
  MFileEventsSim* m_SiReader;

  //! For read-mode file: True if the sim file was read completely
  bool m_ReaderFinished;
  
  unsigned int m_RevanEventID ;
  unsigned int m_RevanLevel;
  unsigned int m_SivanEventID;
  unsigned int m_SivanLevel;
 
  //! 
  MEventSelector m_MimrecEventSelector;

  MGeometryRevan* m_SiGeometry;
  MGeometryRevan* m_ReGeometry;
  
  //! The IDs of all RESEs
  map<MRESE*, vector<int> > m_Ids;
  //! The origin IDs of all RESEs - only created on demand by GetOriginIDs()
  map<MRESE*, vector<int> > m_OriginIds;

  const static float c_NoBound;


  // private members:
 private:


#ifdef ___CLING___
 public:
  ClassDef(MResponseBuilder, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
