/*
 * MResponseBase.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MResponseBase__
#define __MResponseBase__


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


// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MResponseBase
{
  // public interface:
 public:
  //! Default constructor for base class for all response creation classes
  MResponseBase();
  //! Default destructor
  virtual ~MResponseBase();

  //! Raises the interrupt flag
  void Interrupt() { m_Interrupt = true; }

  //! Set and verify the simulation file name
  bool SetSimulationFileName(const MString FileName);
  //! Set and verify the geometry file name
  bool SetGeometryFileName(const MString FileName);
  //! Set the response name (used for the file name suffix)
  void SetResponseName(const MString Name);

  //! Set and verify the revan configuration file name
  bool SetRevanConfigurationFileName(const MString FileName);
  //! Set and verify the mimrec configuration file name
  bool SetMimrecConfigurationFileName(const MString FileName);

  //! Set the ID for the start event
  void SetStartEventID(const unsigned int ID) { m_StartEventID = ID; }
  //! Set the maximum number of analyzed event
  void SetMaxNumberOfEvents(const unsigned int Number) { m_MaxNEvents = Number; }
  //! Safe the response after this amount of events are stored
  void SetSaveAfterNumberOfEvents(const unsigned int Number) { m_SaveAfter = Number; }

  //! Compress the final response files
  void SetCompression(const bool Compress) { m_Compress = Compress; if (m_Compress == true) m_Suffix = ".rsp.gz"; else m_Suffix = ".rsp"; }

  //! Do all the response creation
  virtual bool CreateResponse() = 0;


  // protected methods:
 protected:
  //! Obviously we do not want a copy constructor right now...
  // MResponseBase(const MResponseBase& ResponseBase) {};

  //! Create the (soon to be) pdfs  
  virtual bool CreateMatrices() { return true; }; // = 0 in the near future !!!

  //! Save the (soon to be) pdfs  
  virtual bool SaveMatrices() { return true; }; // = 0 in the near future !!!

  //! Load the geometry, return 0 on failure
  MGeometryRevan* LoadGeometry(bool ActivateNoise, double GlobalFailureRate);
  //! Load the simulation file in revan and mimrec as well as the configuration files:
  virtual bool OpenFiles() = 0;

  //! Initialize the next sivan/revan matching event for response creation
  bool InitializeNextMatchingEvent();
  //! Do a sanity check if the simulations are usable for this task
  bool SanityCheckSimulations();

  //! Return a list of IDs of that RESE
  vector<int> GetReseIds(MRESE* RESE);
  //! Return a list of Sivan Origin IA Ids
  vector<int> GetOriginIds(MRESE* RESE);

  //! Check if IDs (RESE or origins?) are in sequence
  bool AreIdsInSequence(const vector<int>& Ids);

  //! Create a log axis for the response file
  vector<float> CreateLogDist(float Min, float Max, int Bins, 
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
  MString m_SimulationFileName;
  //! Name of the geometry file
  MString m_GeometryFileName;
  //! Name of the response
  MString m_ResponseName;

  //! Revan configuration file name
  MString m_RevanCfgFileName;
  //! Mimrec configuration file name
  MString m_MimrecCfgFileName;

  //! ID for the start event
  unsigned int m_StartEventID;
  //! Maximum number of events to be analyzed 
  unsigned int m_MaxNEvents;
  //! Safe the response after this amount of events are stored
  unsigned int m_SaveAfter;
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

  //
  MRawEventAnalyzer* m_ReReader;
  MFileEventsSim* m_SiReader;

  //! True if the sim file was read completely
  bool m_ReaderFinished;
  
  unsigned int m_RevanEventID ;
  unsigned int m_RevanLevel;
  unsigned int m_SivanEventID;
  unsigned int m_SivanLevel;
 
  
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

  const static float c_NoBound;


  // private members:
 private:


#ifdef ___CINT___
 public:
  ClassDef(MResponseBase, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
