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
  void Interrupt() { if (m_Creator != 0) m_Creator->Interrupt(); m_Interrupt = true; }


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
  unsigned int m_Mode;
  //! Only verify the reconstruction approach
  bool m_Verify;

  //! Event ID to start analysis with
  unsigned long m_StartEvent;
  //! Maximum Number of analysed events (not stop ID)
  unsigned long m_MaxNEvents;
  //! Save after so many analyzed events the response file
  unsigned long m_SaveAfter;
  
  
  //! Name of the revan configuration file
  MString m_RevanCfgFileName;
  //! Name of the mimrec configuration file
  MString m_MimrecCfgFileName;

  //! Don't look at absorptions when in multiple Compton mode
  bool m_NoAbsorptions;
  //! Maximum number of interactions to look at in Compton mode
  unsigned int m_MaxNInteractions;
  //! TMVA methods to use (default BDTD)
  MString m_TMVAMethodsString;

  //! Compress the output response files
  bool m_Compress;

  //! The interrupt flag
  bool m_Interrupt;
  
  //! Modes
  enum MResponseModes {
    c_ModeUnknown,
    c_ModeTracks,
    c_ModeComptons,
    c_ModeComptonsEventFile,
    c_ModeComptonsLens,
    c_ModeComptonsNeuralNetwork,
    c_ModeComptonsTMVA,
    c_ModeVerify,
    c_ModeImagingListMode,
    c_ModeImagingBinnedMode,
    c_ModeImagingCodedMask,
    c_ModeEarthHorizon,
    c_ModeFirstInteractionPosition,
    c_ModeSpectral,
    c_ModeARM,
    c_ModeEfficiency,
    c_ModeEfficiencyNearField,
    c_ModeClusteringDSS,
    c_ModeEventQuality,
    c_ModeStripPairingTMVAEventFile,
    c_ModeEventQualityTMVAEventFile,
    c_ModeComptelDataSpace,
    c_ModeEventClusterizerTMVAEventFile
  };


#ifdef ___CLING___
 public:
  ClassDef(MResponseCreator, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
