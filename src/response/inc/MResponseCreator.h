/*
 * MResponseCreator.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MResponseCreator__
#define __MResponseCreator__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MResponseBase.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MResponseCreator
{
  // public interface:
 public:
  //! Default constructor
  MResponseCreator();
  //! Default destructor
  virtual ~MResponseCreator();

  //! Parse the command line
  bool ParseCommandLine(int argc, char** argv);

  //! Interrupt the analysis
  void Interrupt() { if (m_Creator != 0) m_Creator->Interrupt(); }


  // protected methods:
 protected:
  //MResponseCreator() {};
  //MResponseCreator(const MResponseCreator& ResponseCreator) {};

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  //! Pointer to the specific response creator
  MResponseBase* m_Creator;

  //! Name of the geometry file:
  MString m_GeometryFileName;
  //! Name of the input file:
  MString m_FileName;
  //! Name of the respons file:
  MString m_ResponseName;

  //! Current operation mode
  int m_Mode;
  //! Only verify the reconstruction approach
  bool m_Verify;

  //! Event ID to start analysis with
  int m_StartEvent;
  //! Maximum Number of analysed events (not stop ID) 
  int m_MaxNEvents;
  //! Save after so many analyzed events the response file 
  int m_SaveAfter;

  //! Name of the revan configuration file
  MString m_RevanCfgFileName;
  //! Name of the mimrec configuration file
  MString m_MimrecCfgFileName;

  //! Don't look at absorptions when in multiple Compton mode
  bool m_NoAbsorptions;
  
  //! Compress the output response files
  bool m_Compress;

  //! Modes
  const static int c_ModeUnknown                   = 0;
  const static int c_ModeTracks                    = 1;
  const static int c_ModeComptons                  = 2;
  const static int c_ModeComptonsLens              = 3;
  const static int c_ModeVerify                    = 4;
  const static int c_ModeImagingListMode           = 5;
  const static int c_ModeImagingBinnedMode         = 6;
  const static int c_ModeImagingCodedMask          = 7;
  const static int c_ModeEarthHorizon              = 8;
  const static int c_ModeFirstInteractionPosition  = 9;
  const static int c_ModeSpectral                  = 10;
  const static int c_ModeARM                       = 11;
  const static int c_ModeEfficiency                = 12;

#ifdef ___CINT___
 public:
  ClassDef(MResponseCreator, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
