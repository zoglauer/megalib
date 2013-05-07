/*
 * MERCSRToFWithEnergyRecovery.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MERCSRToFWithEnergyRecovery__
#define __MERCSRToFWithEnergyRecovery__


////////////////////////////////////////////////////////////////////////////////

#include <vector>
#include <map>
using namespace std;

// ROOT libs:
#include <TROOT.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MERConstruction.h"
#include "MERCSR.h"
#include "MRawEventList.h"
#include "MComptonEvent.h"
#include "MVector.h"


// Forward declarations:
class MRESE;
class MGeometryRevan;

////////////////////////////////////////////////////////////////////////////////


class MERCSRToFWithEnergyRecovery : public MERCSR
{
  // public interface:
 public:
  MERCSRToFWithEnergyRecovery();
  virtual ~MERCSRToFWithEnergyRecovery();

  virtual bool SetParameters(MGeometryRevan* Geometry, 
                             double ThresholdMin = 0.0, 
                             double ThresholdMax = 1.0, 
                             int MaxNHits = 5,
                             bool GuaranteeStartD1 = true,
                             bool CreateOnlyPermutations = false);

  virtual MString ToString(bool CoreOnly = false) const;

  static const int c_TestStatisticsChiSquareWithoutErrors;
  static const int c_TestStatisticsChiSquareWithErrors;
  static const int c_TestStatisticsChiSquareProbability;

  static const int c_TestStatisticsFirst;
  static const int c_TestStatisticsLast;

  // protected methods:
 protected:
  //MERCSRToFWithEnergyRecovery() {};
  //MERCSRToFWithEnergyRecovery(const MERCSRToFWithEnergyRecovery& ERCSR) {};

  virtual double ComputeQualityFactor(vector<MRESE*>& Interactions);

  /// Returns the escaped energy of the initial photons 
  virtual double GetEscapedEnergy(vector<MRESE*>& RESEs);


  // private methods:
 private:
  /// The actual calculation of the quality factor is done here
  void CalculateQF(vector<MRESE*>& RESEs, double& QF, double& Eavg);

  /// Split tracks
  virtual void ModifyEventList();

  // protected members:
 protected:
  int m_TestStatisticsType;
  int m_MaxSearchSequence;

  // private members:
 private:

#ifdef ___CINT___
 public:
  ClassDef(MERCSRToFWithEnergyRecovery, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
