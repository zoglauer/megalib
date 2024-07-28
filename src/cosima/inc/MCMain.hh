/*
 * MCMain.hh
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */



/******************************************************************************
 *
 * Main entry point to the C++ part of Cosima. Parses the command line and
 * initializes Geant4
 *
 */

#ifndef ___MCMain___
#define ___MCMain___

// Geant4:
#include "MCParameterFile.hh"

// Cosima:
#include "globals.hh"
#include "MSimEvent.h"

// Forward declarations:
class MCRunManager;
class G4VisManager;
class G4UIsession;
class G4UImanager;

/******************************************************************************/

class MCMain
{
  // public interface:
public:
  /// Default constructor
  MCMain();
  /// Default destructor
  virtual ~MCMain();
  
  /// Parse the command line
  /// 0: Everything OK
  /// 1: Help was displayed
  /// 2 or higher: An error occurred
  unsigned int ParseCommandLine(int argc, char** argv);
  
  /// Initialize Geant4 
  bool Initialize();

  /// Set an event relegator function (i.e. your own event handler) 
  void SetEventRelegator(void (Relegator)(MSimEvent*));

  /// Execute the run or macro
  bool Execute();

  /// Create the absorption probability files
  void CreateAbsorptionProbabilityFiles();

  /// Interrupt and stop the run after the next event
  bool Interrupt();

  /// Get the parameter file
  MCParameterFile& GetParameterFile() { return m_RunParameters; }
  
  // protected methods:
protected:

  

  // protected members:
protected:


  // private members:
private:
  /// File containing all the run parameters
  MCParameterFile m_RunParameters;

  /// Modified Geant4 Run Manager 
  MCRunManager* m_RunManager;

  /// Visualization Manager
  G4VisManager* m_VisManager;
  /// Interactive Geant4 session
  G4UIsession* m_Session;
  /// User interface
  G4UImanager* m_UI;

  /// True if we execute a macro
  bool m_HasMacro;
  /// True if we switch into interactive mode
  bool m_Interactive;
  /// True, if we are in debug mode
  bool m_UseDebug;
  /// True, if the sim files should be gzip'ed after their generation
  bool m_Zip;
  /// Level of debug output: (0: none, 1: standard, 2: debug, 3: complete Geant4)
  int m_Verbosity;
  /// Parallel ID: Additional ID assigned from outside to uniquely identify sim files from one batch of parallel started simulations 
  int m_ParallelID;
  /// Assigned fixed incarbation ID of the output file(s) 
  int m_IncarnationID;
  /// No timeout: Normally the simulation times out if no events are stored, this flag prevents this
  bool m_NoTimeOut;

  /// Name of the parameter file
  MString m_ParameterFileName;
  /// Name of the macro file
  MString m_MacroFileName;
  /// Name of the geomega file to convert to
  MString m_ConvertFileName;

  /// Initial seed of the random number generator
  long m_Seed;
};

#endif


/*
 * MCMain.hh: the end...
 ******************************************************************************/
