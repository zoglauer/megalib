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
#include "MResponseBase.h"
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


class MResponseMultipleCompton : public MResponseBase
{
  // public interface:
 public:
  MResponseMultipleCompton();
  virtual ~MResponseMultipleCompton();

  //! Set whether or not absorptions should be considered
  void SetDoAbsorptions(const bool Flag = true) { m_DoAbsorptions = Flag; }

  //! Do all the response creation
  virtual bool CreateResponse();

  // protected methods:
 protected:
  //MResponseMultipleCompton() {};
  //MResponseMultipleCompton(const MResponseMultipleCompton& ResponseMultipleCompton) {};

  //! Create the (soon to be) pdfs  
  virtual bool CreateMatrices();

  //! Store the (soon to be) pdfs  
  virtual bool SaveMatrices();

  //! Load the simulation file in revan and mimrec as well as the configuration files:
  virtual bool OpenFiles();

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

  int GetMaterial(MRESE& RESE);

  double CalculateDCosPhi(MRESE& Start, MRESE& Central, MRESE& Stop, double Etot);
  double CalculateDPhiInDegree(MRESE& Start, MRESE& Central, MRESE& Stop, double Etot);
  double CalculateCosPhiE(MRESE& Central, double Etot);
  double CalculatePhiEInDegree(MRESE& Central, double Etot);
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
  //! Do or not to do absorptions
  bool m_DoAbsorptions;
  //! MaximumSequenceLength up to which absorptions are considered
  int m_MaxAbsorptions;
  //! Maximum number of individual Compton interactions to be considered
  int m_CSRMaxLength;

  double m_MaxEnergyDifference;
  double m_MaxEnergyDifferencePercent;

  double m_MaxTrackEnergyDifference;
  double m_MaxTrackEnergyDifferencePercent;

  // The event/pdf matrices:
  //!
  MResponseMatrixO2 m_GoodBadTable;
  //!
  MResponseMatrixO4 m_PdfStartGood;

  MResponseMatrixO4 m_PdfStartBad;

  MResponseMatrixO6 m_PdfTrackGood;

  MResponseMatrixO6 m_PdfTrackBad;

  MResponseMatrixO4 m_PdfComptonScatterProbabilityGood;

  MResponseMatrixO4 m_PdfComptonScatterProbabilityBad;

  MResponseMatrixO4 m_PdfPhotoAbsorptionProbabilityGood;

  MResponseMatrixO4 m_PdfPhotoAbsorptionProbabilityBad;

  MResponseMatrixO6 m_PdfComptonGood;

  MResponseMatrixO6 m_PdfComptonBad;

  MResponseMatrixO4 m_PdfDualGood;

  MResponseMatrixO4 m_PdfDualBad;


  // private members:
 private:


#ifdef ___CINT___
 public:
  ClassDef(MResponseMultipleCompton, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
