/*
 * MGUIMimrecMain.h
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


#ifndef __MGUIMimrecMain__
#define __MGUIMimrecMain__


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
#include "MSettingsMimrec.h"
#include "MSettingsSpectralOptions.h"
#include "MGUISpectralAnalyzer.h"
#include "MGUIEFlatToolBar.h"
#include "MGUIEStatusBar.h"

class MInterfaceMimrec;


////////////////////////////////////////////////////////////////////////////////


class MGUIMimrecMain : public MGUIMain
{
  // Public Session:
 public:
  MGUIMimrecMain(MInterfaceMimrec* Interface, MSettingsMimrec* Data);
  virtual ~MGUIMimrecMain();

  virtual void Create();
  virtual bool ProcessMessage(long Message, long Parameter1, long Parameter2);

  // protected methods:
 protected:
  virtual void Open();
  virtual void Launch(bool Animate = false);
  virtual void About();

  // private methods:
 private:

  // private members:
 private:
  MInterfaceMimrec* m_Interface;
  MSettingsMimrec* m_Data;

  // - Title
  const TGPicture* m_TitlePicture;
  TGLayoutHints* m_TitleIconLayout;
  TGIcon* m_TitleIcon;

  enum ButtonIDs { c_CoordinateSystem = c_LastMainButtonID, 
                   c_Zoom, 
                   c_ImageOptions, 
                   c_Algorithm, 
                   c_Response, 
                   c_FitParameter, 
                   c_Exposure, 
                   c_EventSelection,  
                   c_ShowEventSelections, 
                   c_ShowEventSelectionsStepwise, 
                   c_ExtractEvents, 
                   c_PointSourceDeselection, 
                   c_Memory, 
                   c_LikelihoodAlgorithm, 
                   c_LikelihoodPenalty, 
                   c_Animation, 
                   c_StartReconstruction, 
                   c_StopReconstruction,
                   c_ThetaOriginDistribution,
                   c_SpectralAnalyzer,
                   c_ResponseArmGamma,
                   c_ResponseArmGammaClassic,
                   c_ResponseArmGammaVsCompton,
                   c_ResponseArmGammaVsDistance, 
                   c_ResponsePhiKinVsPhiGeo, 
                   c_ResponseSignificanceMap, 
                   c_ResponseArmGammaVsComptonProbability, 
                   c_ResponseArmGammaVsClusteringProbability, 
                   c_ResponseComptonProbabilityWithARMSelection, 
                   c_ResponseEnergyVsComptonProbability, 
                   c_ResponseComptonSequenceLengthVsComptonProbability, 
                   c_ResponseArmElectron, 
                   c_ResponseSpdElectron, 
                   c_ResponseSpdElectronVsCompton, 
                   c_ResponseDualArm, 
                   c_ResponseArmComparison, 
                   c_ResponseResolutionMeasurePET,
                   c_ResponseBackground, 
                   c_ResponseSpectrum, 
                   c_ResponseInitialEnergyDeposit, 
                   c_ResponseEnergyDistributionD1D2, 
                   c_ResponsePhi, 
                   c_ResponseDistance, 
                   c_ResponseTime, 
                   c_ResponseExposureMap, 
                   c_ResponseTimeWalk, 
                   c_ResponseCoincidenceWindow, 
                   c_ResponseAzimuthalElectronScatterAngle,
                   c_ResponseAzimuthalComptonScatterAngle, 
                   c_ResponsePolarization, 
                   c_ResponseStandardAnalysis, 
                   c_ResponseOpeningAnglePair, 
                   c_ResponseAngularResolutionPair, 
                   c_ResponseAngularResolutionVsQualityFactorPair, 
                   c_ResponseClusteringQualityFactor, 
                   c_ResponseComptonQualityFactor, 
                   c_ResponseTrackQualityFactor, 
                   c_ResponseSPDVsTrackQualityFactor, 
                   c_ResponseSPDVsTotalScatterAngleDeviation, 
                   c_ResponseSequenceLengths,
                   c_ResponseSelectIds,
                   c_ResponseLocationOfInitialInteraction,
                   c_ResponseEarthCenterDistance,
                   c_ResponsePointingInGalacticCoordinates,
                   c_ResponseCreateCosimaOrientationFile,
                   c_ResponseHorizonInSphericalDetectorCoordinates,
                   c_ResponseDirectionScatteredGammaRay,
                   c_ResponseTest }; 

#ifdef ___CLING___
 public:
  ClassDef(MGUIMimrecMain, 0) // main window of the Mimrec GUI
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
