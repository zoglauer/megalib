/*
 * MERDecay.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MERDecay__
#define __MERDecay__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <vector>
using namespace std;

// MEGAlib libs:
#include "MGlobal.h"
#include "MERConstruction.h"
#include "MRawEventList.h"

// Forward declarations:
class MRERawEvent;

////////////////////////////////////////////////////////////////////////////////


class MERDecay : public MERConstruction
{
  // public interface:
 public:
  MERDecay();
  virtual ~MERDecay();

  virtual bool SetParameters(MString FileName, 
                             const vector<double>& Energy, 
                             const vector<double>& EnergyError);
  virtual bool Analyze(MRawEventList* List);

  virtual bool PreAnalysis();
  virtual bool PostAnalysis();

  virtual MString ToString(bool CoreOnly = false) const;

  static const int c_AllCombinations;
  static const int c_AllButOneCombination;
  static const int c_ComptonSequenceEnd;


  // protected methods:
 protected:
  //MERDecay() {};
  //MERDecay(const MERDecay& ERDecay) {};

  bool IsDecay(double Energy, double EnergyError);
  bool CheckCombinations(double Energy, double EnergyError, 
                         int Start, MRERawEvent* RE, int Level);

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  vector<int> m_Occupation;
  vector<double> m_Energy;
  vector<double> m_EnergyErrorSigma;

  int m_Mode;

#ifdef ___CLING___
 public:
  ClassDef(MERDecay, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
