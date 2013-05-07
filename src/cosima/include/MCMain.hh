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

  /// Initialize Geant4 
  bool Initialize(int argc, char** argv);

  /// Execute the run or macro
  bool Execute();

  /// Create the absorption probability files
  void CreateAbsorptionProbabilityFiles();

  /// Interrupt and stop the run after the next event
  bool Interrupt();


  // protected methods:
protected:
  /// Parse the command line
  bool ParseCommandLine(int argc, char** argv);
  

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
