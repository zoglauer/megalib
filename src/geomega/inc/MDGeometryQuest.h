/*
 * MDGeometryQuest.h
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


#ifndef __MDGeometryQuest__
#define __MDGeometryQuest__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <TObjArray.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MComptonEvent.h"
#include "MDGeometry.h"
#include "MDVolumeSequence.h"
#include "MDVolume.h"
#include "MDMaterial.h"
#include "MDGridPointCollection.h"

// Standard libs::
#include <vector>
#include <map>
using namespace std;

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MDGeometryQuest : public MDGeometry
{
  // public interface:
 public:
  MDGeometryQuest();
  virtual ~MDGeometryQuest();

  virtual bool DrawGeometry(TCanvas* Canvas = nullptr, bool RestoreView = false, MString Mode = "ogle");

  bool IsSensitive(MVector Pos);
  
  using MDGeometry::GetVolume;
  MDVolume* GetVolume(MVector Pos);
  
  using MDGeometry::GetMaterial;
  MDMaterial* GetMaterial(MVector Pos);

  MDGridPointCollection Grid(MVector& Pos, double& Energy, double& Time, MDVolumeSequence& VS);
  
  
  bool Noise(MVector& Pos, double& Energy, double& Time, MString& Flags);
  bool Noise(MVector& Pos, double& Energy, double& Time, MString& Flags, MDVolumeSequence& VS);
  void ApplyPulseShape(double Time, MVector& Pos, double& Energy);

  //! Get the resolutions at a world position, or zero them and return false outside sensitive detectors
  bool GetResolutions(MVector Pos, double Energy, double Time,
                      MVector& PRes, double& ERes, double& TRes);
  //! Get the resolutions using the volume sequence, or zero them and return false if its detector or sensitive volume is missing
  bool GetResolutions(MVector Pos, double Energy, double Time, MDVolumeSequence& VS,
                      MVector& PRes, double& ERes, double& TRes);

  MDVolume* GetEmbeddingVolume(MVector Position);
  MString GetDetectorName(MVector Position);
  bool AreInSameVoxel(MVector Pos1, MVector Pos2);

  int GetNVolumesInHierarchy();

  void AddHit(MString Name, MVector Position, double Size, int Color = 1);
  void RemoveAllHits();

  void AddLink(MVector PositionA, MVector PositionB, double Size, int Color = 1);
  void RemoveAllLinks();


  // Global options:
  void AllwaysAssumeTrigger(bool Flag = true);
  void ActivateNoising(bool Activate = true);
  bool GetActivateNoising();
  //! Ignore (true) or apply (false, default) the veto triggers when the trigger
  //! unit is evaluated, e.g. in MSimEvent::Analyze() when reading sim files.
  //! Must be called after ScanSetupFile(), since the triggers have to exist.
  void IgnoreVetoes(bool Ignore = true);
  //! Return true if veto triggers are ignored
  bool GetIgnoreVetoes() const;
  //! Ignore (true) or apply (false, default) the thresholds: the noise thresholds of all
  //! detectors (incl. guard rings) during noising, and the trigger thresholds in the trigger unit.
  //! Thus hits below threshold are kept and can trigger an event.
  //! Must be called after ScanSetupFile(), since the detectors have to exist.
  void IgnoreThresholds(bool Ignore = true);
  //! Return true if thresholds are ignored
  bool GetIgnoreThresholds() const;
  void SetGlobalFailureRate(double FailureRate = 0.0);


  // Absorption interface:
  double GetAbsorptionProbability(MVector Start, MVector Stop, double Energy);
  double GetPhotoAbsorptionProbability(MVector Start, MVector Stop, double Energy);
  double GetComptonAbsorptionProbability(MVector Start, MVector Stop, double Energy);
  double GetPairAbsorptionProbability(MVector Start, MVector Stop, double Energy);
  double GetRayleighAbsorptionProbability(MVector Start, MVector Stop, double Energy);

  bool GetComptonIntersection(const MComptonEvent& Compton);
  map<MDMaterial*, double> GetPathLengths(MVector Start, MVector Stop);


  // protected methods:
 protected:
  double GetAbsorptionProbability(MVector Start, MVector Stop, double Energy, int Mode);


  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:

  vector<MDVolume*> m_Hits;
  vector<MDVolume*> m_Links;

  bool m_AllwaysAssumeTrigger;
  bool m_ActivateNoising;
  bool m_IgnoreVetoes;
  bool m_IgnoreThresholds;



#ifdef ___CLING___
 public:
  ClassDef(MDGeometryQuest, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
