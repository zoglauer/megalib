/*
 * MGUISivanMain.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUISivanMain__
#define __MGUISivanMain__


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
#include "MSettingsSivan.h"
#include "MGUIEFlatToolBar.h"
#include "MGUIEStatusBar.h"

class MInterfaceSivan;


////////////////////////////////////////////////////////////////////////////////


class MGUISivanMain : public MGUIMain
{
  // Public Session:
 public:
  MGUISivanMain(MInterfaceSivan* Interface, MSettingsSivan* Data);
  virtual ~MGUISivanMain();

  virtual void Create();
  virtual bool ProcessMessage(long Message, long Parameter1, long Parameter2);

  // protected members:
 protected:
  virtual void Open();
  virtual void Launch();
  virtual void About();

  // private members:
 private:
  MInterfaceSivan* m_Interface;
  MSettingsSivan* m_Data;

  // Menu:
  TGPopupMenu* m_MenuAnalysis;

  // - Title
  const TGPicture* m_TitlePicture;
  TGLayoutHints* m_TitleIconLayout;
  TGIcon* m_TitleIcon;

  enum ButtonIDs { c_Options = c_LastMainButtonID, 
                   c_Start, 
                   c_IdealAnalysis, 
                   c_Chance, 
                   c_ViewHits, 
                   c_EnergyLoss, 
                   c_ChargeSharing, 
                   c_InteractionsPerVoxel,
                   c_EnergyPerNucleus,
                   c_EnergyPerVoxel, 
                   c_ComptonEnergyEscape, 
                   c_IncidenceAngle,
                   c_IncidenceEnergy,
                   c_IncidenceVsMeasuredEnergy,
                   c_DepositedEnergy,
                   c_InitialComptonScatterAngle,
                   c_EnergyOfSecondaries,
                   c_InteractionDetectorSequence,
                   c_Hits, 
                   c_FirstHitPosition, 
                   c_AllHitPositions, 
                   c_MissingInteractions, 
                   c_CompleteAbsorptionRatio, 
                   c_DopplerARM,  
                   c_NInteractions,
                   c_SecondaryGenerationPattern,
                   c_EnergyLossByMaterial,
                   c_Test };


#ifdef ___CINT___
 public:
  ClassDef(MGUISivanMain, 0) // main window of the Sivan GUI
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
