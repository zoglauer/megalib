/*
 * MHitData.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MHitData__
#define __MHitData__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <TROOT.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MVector.h"

// Forward declarations:

////////////////////////////////////////////////////////////////////////////////


class MHitData : public TObject
{
  // public interface:
 public:
  MHitData();
  MHitData(UInt_t Detector, MVector Position, Double_t Energy);
  ~MHitData();

  void SetDetector(UInt_t Detector);
  void SetPosition(MVector Position);
  void SetEnergy(Double_t Energy);

  UInt_t GetDetector();
  MVector GetPosition();
  Double_t GetEnergy();

  // protected methods:
 protected:


  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  UInt_t m_Detector;
  MVector m_Position;
  Double_t m_Energy;


#ifdef ___CINT___
 public:
  ClassDef(MHitData, 1) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
