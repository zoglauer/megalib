/*
 * MERCSRTMVA.h
 *
 * Copyright (C) 1998-2007 by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MERCSRTMVA__
#define __MERCSRTMVA__


////////////////////////////////////////////////////////////////////////////////

// Standard libs:
#include <vector>
#include <map>
using namespace std;

// Root libs:
#include "TMVA/Reader.h"

// MEGAlib libs:
#include "MGlobal.h"
#include "MERConstruction.h"
#include "MERCSR.h"
#include "MRawEventList.h"
#include "MComptonEvent.h"
#include "MVector.h"
#include "MERCSRDataSet.h"

// Forward declarations:
class MRESE;
class MRETrack;
class MGeometryRevan;

////////////////////////////////////////////////////////////////////////////////


class MERCSRTMVA : public MERCSR
{
  // public interface:
 public:
  MERCSRTMVA();
  virtual ~MERCSRTMVA();

  virtual bool SetParameters(MString FileName,
                             MGeometryRevan* Geometry,
                             double ThresholdMin = 0.0,
                             double ThresholdMax = 0.5,
                             int MaxNHits = 5,
                             bool GuaranteeStartD1 = true,
                             bool CreateOnlyPermutations = false);


  virtual MString ToString(bool CoreOnly = false) const;

  virtual int ComputeAllQualityFactors(MRERawEvent* RE);

  // protected methods:
 protected:
  //MERCSRTMVA() {};
  //MERCSRTMVA(const MERCSRTMVA& ERCSR) {};

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  //! The training data file name
  MString m_FileName;

  //! The available methods
  map<MString, int> m_AvailableMethods;

  //! The used methods
  vector<MString> m_UsedMethods;
  
  //! The data set
  MERCSRDataSet m_DS;
  
  //! The TMVA readers - one per sequence length
  vector<TMVA::Reader*> m_Readers;
  
  
#ifdef ___CINT___
 public:
  ClassDef(MERCSRTMVA, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
