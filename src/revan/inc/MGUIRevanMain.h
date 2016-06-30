/*
 * MGUIRevanMain.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUIRevanMain__
#define __MGUIRevanMain__


////////////////////////////////////////////////////////////////////////////////


// Standard libs

// ROOT libs
#include <TGMenu.h>
#include <TGButton.h>
#include <TGFrame.h>
#include <TGFileDialog.h>
#include <TGIcon.h>
#include <TGPicture.h>

// MEGAlib libs
#include "MGlobal.h"
#include "MGUIMain.h"
#include "MSettingsRevan.h"
#include "MGUIEFlatToolBar.h"
#include "MGUIEStatusBar.h"
#include "MSettingsSpectralOptions.h"
#include "MGUISpectralAnalyzer.h"

class MInterfaceRevan;


////////////////////////////////////////////////////////////////////////////////


class MGUIRevanMain : public MGUIMain
{
  // Public Session:
 public:
  MGUIRevanMain(MInterfaceRevan* Interface, MSettingsRevan* Data);
  virtual ~MGUIRevanMain();

  virtual void Create();
  virtual bool ProcessMessage(long Message, long Parameter1, long Parameter2);

  // protected members:
 protected:
  virtual void Open();
  virtual void Launch();
  virtual void About();

  // private members:
 private:
  MInterfaceRevan* m_Interface;
  MSettingsRevan* m_Data;

  // Additional menu items:
  TGPopupMenu* m_MenuReconstruction;
  TGPopupMenu* m_MenuApplications;

  // - Title
  const TGPicture* m_TitlePicture;
  TGLayoutHints* m_TitleIconLayout;
  TGIcon* m_TitleIcon;

  enum ButtonIDs {
    c_Min = c_LastMainButtonID, 
    c_Options, 
    c_OptionsGeneral, 
    c_OptionsCoincidence, 
    c_OptionsClustering, 
    c_OptionsTracking, 
    c_OptionsSequencing, 
    c_OptionsDecay, 
    c_Start, 
    c_Spectrum,
    c_ExportSpectrum,
    c_SpectralAnalyzer,
    c_Coincidence,
    c_EnergyInitial,
    c_EnergyDistribution, 
    c_EnergyPerCentralTrackElement,
    c_HitStatistics, 
    c_SpatialHitDistribution, 
    c_SpatialEnergyDistribution, 
    c_DepthProfileByDetector,
    c_CSRTestStatistics,
    c_Polarization, 
    c_BeamPath,
    c_NClusters,
    c_DetectorTypeClusterDistributionBefore,
    c_DetectorTypeClusterDistributionAfter,
    c_DetectorTypeHitDistributionBefore,
    c_DetectorTypeHitDistributionAfter,
    c_InitialEventStatistics,
    c_TriggerStatistics,
    c_Test,
    c_Max }; 


#ifdef ___CINT___
 public:
  ClassDef(MGUIRevanMain, 0) // main window of the Revan GUI
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
