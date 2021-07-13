/*
 * MAssembly.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MAssembly__
#define __MAssembly__


////////////////////////////////////////////////////////////////////////////////


// standard libs
#include <iostream>
using namespace std;

// ROOT libs
#include "MVector.h"

// MEGAlib libs:
#include "MGlobal.h"
#include "MSupervisor.h"


////////////////////////////////////////////////////////////////////////////////


class MAssembly
{
  // Public Interface:
 public:
  //! Default constructor
  MAssembly();
  //! Default destructor
  virtual ~MAssembly();

  //! Each interface must be able to parse a command line - 
  //! this function is called by main()
  bool ParseCommandLine(int argc, char** argv);
  
  //! Called when hit Control-C: Set the interrupt which will end the analysis in the supervisor
  void SetInterrupt(bool Flag = true) { m_Supervisor->SetHardInterrupt(Flag); }

  // Module types:
  static const int c_EventLoader              =  1;
  static const int c_EventLoaderMeasurement   =  2;
  static const int c_EventLoaderSimulation    =  3;
  static const int c_DetectorEffectsEngine    =  4;
  static const int c_EventCoincidence         =  5;
  static const int c_EnergyCalibration        =  6;
  static const int c_ChargeSharingCorrection  =  7;
  static const int c_CrosstalkCorrection      =  8;
  static const int c_DepthCorrection          =  9;
  static const int c_StripPairing             = 10;
  static const int c_PositionDetermiation     = 11;
  static const int c_AspectReconstruction     = 12;
  static const int c_EventReconstruction      = 13;
  static const int c_EventSaver               = 14;
  static const int c_EventTransmitter         = 15;
  static const int c_EventFilter              = 16;
  static const int c_NoRestriction            = 17;
  
  // protected methods:
 protected:
  
  // private methods:
 private:


  // protected members:
 protected:


  // private members:
 private:
  //! True if the GUI is used
  bool m_UseGui;
  
  //! The store for all user data of the GUI:
  MSupervisor* m_Supervisor;
  
  //! The interrupt flag - the analysis will stop when this flag is set
  bool m_Interrupt;

  
#ifdef ___CINT___
 public:
  ClassDef(MAssembly, 0) // image reconstruction management class 
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
