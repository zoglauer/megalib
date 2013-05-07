/*
 * MDGeometryQuest.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
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
  ~MDGeometryQuest();

  virtual bool DrawGeometry(TCanvas *Canvas = 0);

  bool IsSensitive(MVector Pos);
  MDVolume* GetVolume(MVector Pos); 
  MDMaterial* GetMaterial(MVector Pos);

  bool Noise(MVector& Pos, double& Energy, double& Time);
  bool Noise(MVector& Pos, double& Energy, double& Time, MDVolumeSequence& VS);
  void ApplyPulseShape(double Time, MVector& Pos, double& Energy);

  void GetResolutions(MVector Pos, double Energy, double Time, 
                      MVector& PRes, double& ERes, double& TRes);
  void GetResolutions(MVector Pos, double Energy, double Time, MDVolumeSequence& VS,
                      MVector& PRes, double& ERes, double& TRes);

  MDVolume* GetEmbeddingVolume(MVector Position);
  MDDetector* GetDetector(MVector Position);
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



#ifdef ___CINT___
 public:
  ClassDef(MDGeometryQuest, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
