/*
 * MERCSRBayesian.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MERCSRBayesian__
#define __MERCSRBayesian__


////////////////////////////////////////////////////////////////////////////////

// Standard libs:
#include <vector>
#include <map>
using namespace std;

// Root libs:
#include <TROOT.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MERConstruction.h"
#include "MERCSR.h"
#include "MRawEventList.h"
#include "MComptonEvent.h"
#include "MResponseMatrixO1.h"
#include "MResponseMatrixO2.h"
#include "MResponseMatrixO3.h"
#include "MResponseMatrixO4.h"
#include "MResponseMatrixO5.h"
#include "MResponseMatrixO6.h"
#include "MResponseMatrixO7.h"
#include "MVector.h"

// Forward declarations:
class MRESE;
class MRETrack;
class MGeometryRevan;

////////////////////////////////////////////////////////////////////////////////


class MERCSRBayesian : public MERCSR
{
  // public interface:
 public:
  MERCSRBayesian();
  virtual ~MERCSRBayesian();

  bool SetParameters(MString FileName,
                     MGeometryRevan* Geometry,
                     double ThresholdMin = 0.0,
                     double ThresholdMax = 0.5,
                     int MaxNHits = 5,
                     bool GuaranteeStartD1 = true,
                     bool CreateOnlyPermutations = false);

  static double CalculateDPhiInDegree(MRESE* Start, MRESE* Central, MRESE* Stop, double Energy);
  static double CalculateDCosPhi(MRESE* Start, MRESE* Central, MRESE* Stop, double Energy);
  static double CalculatePhiEInDegree(MRESE* Central, double Etot);
  static double CalculateCosPhiE(MRESE* Central, double Etot);
  static double CalculatePhiGInDegree(MRESE* Start, MRESE* Central, MRESE* Stop);
  static double CalculateCosPhiG(MRESE* Start, MRESE* Central, MRESE* Stop);
  
  static double CalculateDAlphaInDegree(MRETrack* Start, MRESE* Central, double Energy);
  static double CalculateDCosAlpha(MRETrack* Start, MRESE* Central, double Energy);
  static double CalculateAlphaEInDegree(MRETrack* Start, MRESE* Central, double Energy);
  static double CalculateCosAlphaE(MRETrack* Start, MRESE* Central, double Energy);
  static double CalculateAlphaGInDegree(MRETrack* Start, MRESE* Central, double Energy);
  static double CalculateCosAlphaG(MRETrack* Start, MRESE* Central, double Energy);

  static double CalculateMinLeverArm(const MVector& Start, const MVector& Central, const MVector& Stop);
  static int GetMaterial(MRESE* Hit);

  virtual MString ToString(bool CoreOnly = false) const;

  // protected methods:
 protected:
  //MERCSRBayesian() {};
  //MERCSRBayesian(const MERCSRBayesian& ERCSR) {};

  virtual void VerifyEntries(float& NEntriesGood, float& NEntriesBad);

  // private methods:
 private:
  virtual double ComputeQualityFactor(vector<MRESE*>& Interactions);



  // protected members:
 protected:


  // private members:
 private:
  unsigned int m_UseAbsorptionsUpTo; 

  MString m_FileName;

  MResponseMatrixO2 m_GoodBad;

  MResponseMatrixO4 m_GoodDualDeposit;
  float m_SumGoodDualDeposit;
  MResponseMatrixO4 m_BadDualDeposit;
  float m_SumBadDualDeposit;

  MResponseMatrixO4 m_GoodStartDeposit;
  MResponseMatrixO1 m_SumGoodStartDeposit;
  MResponseMatrixO4 m_BadStartDeposit;
  MResponseMatrixO1 m_SumBadStartDeposit;

  MResponseMatrixO6 m_GoodTrack;
  MResponseMatrixO1 m_SumGoodTrack;
  MResponseMatrixO6 m_BadTrack;
  MResponseMatrixO1 m_SumBadTrack;

  MResponseMatrixO4 m_GoodComptonDistance;
  MResponseMatrixO1 m_SumGoodComptonDistance;
  MResponseMatrixO4 m_BadComptonDistance;
  MResponseMatrixO1 m_SumBadComptonDistance;

  MResponseMatrixO4 m_GoodPhotoDistance;
  MResponseMatrixO1 m_SumGoodPhotoDistance;
  MResponseMatrixO4 m_BadPhotoDistance;
  MResponseMatrixO1 m_SumBadPhotoDistance;

  MResponseMatrixO6 m_GoodCompton;
  MResponseMatrixO1 m_SumGoodCompton;
  MResponseMatrixO6 m_BadCompton;
  MResponseMatrixO1 m_SumBadCompton;

  MResponseMatrixO3 m_GoodStopDeposit;
  MResponseMatrixO1 m_SumGoodStopDeposit;
  MResponseMatrixO3 m_BadStopDeposit;
  MResponseMatrixO1 m_SumBadStopDeposit;


#ifdef ___CLING___
 public:
  ClassDef(MERCSRBayesian, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
