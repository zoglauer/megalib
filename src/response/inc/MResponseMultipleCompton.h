/*
 * MResponseMultipleCompton.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MResponseMultipleCompton__
#define __MResponseMultipleCompton__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MResponseBuilder.h"
#include "MRESE.h"
#include "MRETrack.h"
#include "MResponseMatrixO1.h"
#include "MResponseMatrixO2.h"
#include "MResponseMatrixO3.h"
#include "MResponseMatrixO4.h"
#include "MResponseMatrixO5.h"
#include "MResponseMatrixO6.h"
#include "MResponseMatrixO7.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////

//! Create a Bayesian Compton sequenceing response 
class MResponseMultipleCompton : public MResponseBuilder
{
  // public interface:
 public:
  //! Default constructor
  MResponseMultipleCompton();
  //! Default destructor
  virtual ~MResponseMultipleCompton();

  //! Set whether or not absorptions should be considered
  void SetDoAbsorptions(const bool Flag = true) { m_DoAbsorptions = Flag; }
  //! Set whether or not absorptions should be considered
  void SetMaxNInteractions(const unsigned int MaxNInteractions) { m_MaxNInteractions = MaxNInteractions; }

  // protected methods:
 protected:

  bool IsTrackStart(MRESE& Start, MRESE& Central, double Energy);
  bool IsTrackStop(MRESE& Central, MRESE& Stop, double Energy);

  bool IsComptonStart(MRESE& Start, double Etot, double Eres);
  bool IsComptonTrack(MRESE& Start, MRESE& Central, int StartPosition, 
                      double Etot, double Eres);
  bool IsComptonEnd(MRESE& Stop);
  bool IsComptonSequence(MRESE& Start, MRESE& Stop,
                         int StartPosition, double StopEnergy, double Eres);
  bool IsComptonSequence(MRESE& Start, MRESE& Central, MRESE& Stop,
                         int StartPosition, double StopEnergy, double Eres);
  bool IsSingleCompton(MRESE& Start);

  bool AreInComptonSequence(const vector<int>& StartOriginIds, 
                            const vector<int>& CentralOriginIds,
                            int StartPosition);
  bool AreReseInSequence(MRESE& Start, MRESE& Central, MRESE& Stop, double Energy);

  bool ContainsOnlyComptonDependants(vector<int> AllSimIds);
  bool IsAbsorbed(const vector<int>& AllSimIds, double Energy, double EnergyResolution);
  bool IsTotalAbsorbed(const vector<int>& AllSimIds, double Energy, double EnergyResolution);
  bool IsTrackCompletelyAbsorbed(const vector<int>& Ids, double Energy);
  double GetIdealDepositedEnergy(int MinId);

  //! Return the number of Compton interaction from the Sim ID with the given origin
  unsigned int NumberOfComptonInteractions(vector<int> AllSimIds, int Origin);
  
  
  int GetMaterial(MRESE& RESE);

  double CalculateDCosPhi(MRESE& Start, MRESE& Central, MRESE& Stop, double Etot);
  double CalculateDPhiInDegree(MRESE& Start, MRESE& Central, MRESE& Stop, double Etot);
  double CalculateCosPhiE(MRESE& Central, double Etot);
  double CalculatePhiEInDegree(MRESE& Central, double Etot);
  double CalculateCosPhiG(MRESE& Start, MRESE& Central, MRESE& Stop);
  double CalculatePhiGInDegree(MRESE& Start, MRESE& Central, MRESE& Stop);
  double CalculateCosAlphaE(MRETrack& Start, MRESE& Central, double Etot);
  double CalculateAlphaEInDegree(MRETrack& Start, MRESE& Central, double Etot);
  double CalculateCosAlphaG(MRETrack& Start, MRESE& Central, double Etot);
  double CalculateAlphaGInDegree(MRETrack& Start, MRESE& Central, double Etot);
  double CalculateDCosAlpha(MRETrack& Start, MRESE& Central, double Etot);
  double CalculateDAlphaInDegree(MRETrack& Start, MRESE& Central, double Etot);
  double CalculateMinLeverArm(MRESE& Start, MRESE& Central, MRESE& Stop);

  double CalculateAbsorptionProbabilityPhoto(MRESE& Start, MRESE& Stop, double Etot);
  double CalculateAbsorptionProbabilityCompton(MRESE& Start, MRESE& Stop, double Etot);
  double CalculateAbsorptionProbabilityTotal(MRESE& Start, MRESE& Stop, double Etot);

  // private methods:
 private:



  // protected members:
 protected:
  //! Minimum energy range
  double m_EnergyMinimum;
  //! Maximum energy range
  double m_EnergyMaximum;

  //! Do or not to do absorptions
  bool m_DoAbsorptions;
  //! MaximumSequenceLength up to which absorptions are considered
  unsigned int m_MaxAbsorptions;
  //! Maximum number of individual Compton interactions to be considered
  unsigned int m_MaxNInteractions;

  double m_MaxEnergyDifference;
  double m_MaxEnergyDifferencePercent;

  double m_MaxTrackEnergyDifference;
  double m_MaxTrackEnergyDifferencePercent;



  // private members:
 private:


#ifdef ___CLING___
 public:
  ClassDef(MResponseMultipleCompton, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
