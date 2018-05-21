/*
 * MERCSRToF.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MERCSRToF__
#define __MERCSRToF__


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
#include "MREAMDriftChamberEnergy.h"

// Forward declarations:
class MRESE;
class MGeometryRevan;

////////////////////////////////////////////////////////////////////////////////


class MERCSRToF : public MERCSR
{
  // public interface:
 public:
  MERCSRToF();
  virtual ~MERCSRToF();

  virtual MString ToString(bool CoreOnly = false) const;

  // protected methods:
 protected:
  //MERCSRToF() {};
  //MERCSRToF(const MERCSRToF& ERCSR) {};

  virtual double ComputeQualityFactor(vector<MRESE*>& Interactions);

  // private methods:
 private:
  void CalculateTS(vector<MRESE*>& RESEs, double& TS);

  //! Upgrade energies and resolutions
  virtual void ModifyEventList();

  //! 
  void CorrectEnergiesAndTime(MREAMDriftChamberEnergy* Ream, 
                              vector<MRESE*>& Interactions);

  // protected members:
 protected:


  // private members:
 private:

#ifdef ___CLING___
 public:
  ClassDef(MERCSRToF, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
