/*
 * MERCSR.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MERCSR__
#define __MERCSR__


////////////////////////////////////////////////////////////////////////////////

#include <vector>
#include <map>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MERConstruction.h"
#include "MRawEventList.h"
#include "MComptonEvent.h"
#include "MGeometryRevan.h"

// Forward declarations:
class MRESE;
class MGeometryRevan;

////////////////////////////////////////////////////////////////////////////////


class MERCSR : public MERConstruction
{
  // public interface:
 public:
  MERCSR();
  virtual ~MERCSR();

  /// Set all parameters at once
  bool SetParameters(MGeometryRevan* Geometry,
                     double QualityFactorMin = 0.0,
                     double QualityFactorMax = 1000,
                     int MaxNIteractions = 5,
                     bool GuaranteeStartD1 = false,
                     bool CreateOnlyPermutations = false);

  /// Entry point to the analysis the given event list
  virtual bool Analyze(MRawEventList* List);

  /// Return some comments about the basic settings
  virtual MString ToString(bool CoreOnly = false) const;

  static const double c_CSRFailed;

  // protected methods:
 protected:
  //MERCSR() {};
  //MERCSR(const MERCSR& ERCSR) {};

  /// The functions manages the search for the ebst sequence
  virtual void FindComptonSequence(MRERawEvent* RE);

  /// Compute all quality factors, return the number of good ones
  virtual int ComputeAllQualityFactors(MRERawEvent* RE);

  /// Calculates the Quality factor for the sequence "Interactions"
  virtual double ComputeQualityFactor(vector<MRESE*>& Interactions);

  
  virtual void FindPermutations(vector<MRESE*> RESEs, int Level, vector<vector<MRESE*> >& Permutations);
  void LeftShift(vector<MRESE*>& RESEs, int Level);

  /// Return a list of all permutations of the given raw event
  MRawEventList* CreateOnlyPermutations(MRERawEvent* RE);

  /// Return the (possibly) escaped energy for this event - defaults to zero
  virtual double GetEscapedEnergy(vector<MRESE*>& RESEs);

  /// Compute the position error for an angle defined by those three vectors
  double ComputePositionError(MRESE* First, MRESE* Second, MRESE* Third);

  // private methods:
 private:



  // protected members:
 protected:
  MGeometryRevan* m_Geometry;
  double m_QualityFactorMin;
  double m_QualityFactorMax;
  int m_MaxNInteractions;
  bool m_CreateOnlyPermutations;
  bool m_GuaranteeStartD1;

  /// The sorted list of quality factors
  map<double, vector<MRESE*>, less_equal<double> > m_QualityFactors;
  /// An iterator over the sorted list of quality factors
  map<double, vector<MRESE*>, less_equal<double> >::iterator m_QualityFactorsIterator;


#ifdef ___CINT___
 public:
  ClassDef(MERCSR, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
