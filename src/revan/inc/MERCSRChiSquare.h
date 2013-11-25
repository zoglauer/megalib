/*
 * MERCSRChiSquare.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MERCSRChiSquare__
#define __MERCSRChiSquare__


////////////////////////////////////////////////////////////////////////////////

#include <vector>
#include <map>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MERCSR.h"
#include "MRawEventList.h"
#include "MComptonEvent.h"
#include "MGeometryRevan.h"

// Forward declarations:
class MRESE;
class MGeometryRevan;

////////////////////////////////////////////////////////////////////////////////


class MERCSRChiSquare : public MERCSR
{
  // public interface:
 public:
  MERCSRChiSquare();
  virtual ~MERCSRChiSquare();

  bool SetParameters(MGeometryRevan* Geometry,
                     double QualityFactorMin = 0.0,
                     double QualityFactorMax = 1000,
                     int MaxNIterations = 5,
                     bool GuaranteeStartD1 = true,
                     bool CreateOnlyPermutations = false,
                     bool UseComptelEvents = true,
                     int UndecidedHandling = 0,
                     bool RejectOneDetectorTypeOnlyEvents = true,
                     MGeometryRevan* OriginObjects = 0);

  virtual MString ToString(bool CoreOnly = false) const;

  static const int c_TSUnknown;
  static const int c_TSSimple;
  static const int c_TSSimpleWithErrors;
  static const int c_TSChiSquare;

  static const int c_TSFirst;
  static const int c_TSLast;

  static const int c_UndecidedIgnore;
  static const int c_UndecidedAssumestartD1;
  static const int c_UndecidedLargerKleinNishina;
  static const int c_UndecidedLargerKleinNishinaTimesPhoto;
  static const int c_UndecidedLargerEnergyDeposit;

  // protected methods:
 protected:
  //MERCSRChiSquare() {};
  //MERCSRChiSquare(const MERCSRChiSquare& ERCSRChiSquare) {};


  virtual void FindComptonSequenceDualHitEvent(MRERawEvent* RE);
  virtual void FindComptonSequence(MRERawEvent* RE);

  virtual double ComputeQualityFactor(vector<MRESE*>& Interactions);


  bool OriginatesFromObjects(const MComptonEvent& Compton);

  // private methods:
 private:



  // protected members:
 protected:

  bool m_UseComptelTypeEvents;
  bool m_RejectOneDetectorTypeOnlyEvents;
  int m_UndecidedHandling;
  bool m_AssumeD1First;
  int m_TypeTestStatistics;

  MGeometryRevan* m_OriginObjects;
  
   
  map<double, vector<MRESE*>, less_equal<double> > m_TestStatistics;
  map<double, vector<MRESE*>, less_equal<double> >::iterator m_TestStatisticsIterator;


#ifdef ___CINT___
 public:
  ClassDef(MERCSRChiSquare, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
