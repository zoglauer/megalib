/*
 * MGUISivanMain.h
 *
 * Copyright (C) by the MEGAlib contributors.
 *
 * This file is part of MEGAlib.
 *
 * MEGAlib is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * MEGAlib is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
 * License (License.md) for more details.
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
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
                   c_IsotopeGeneration,
                   c_ActivationPerIncidenceEnergy,
                   c_EnergyPerVoxel, 
                   c_ComptonEnergyEscape, 
                   c_StartLocations,
                   c_IncidenceAngle,
                   c_IncidenceEnergy,
                   c_IncidenceVsMeasuredEnergy,
                   c_InitialInteraction,
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
                   c_OriginVolumesOfActivation,
                   c_Test };


#ifdef ___CLING___
 public:
  ClassDef(MGUISivanMain, 0) // main window of the Sivan GUI
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
