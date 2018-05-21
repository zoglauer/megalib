/*
 * MPhysicalEventAnalyzer.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MPhysicalEventAnalyzer__
#define __MPhysicalEventAnalyzer__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include "TObject.h"
#include <TObjArray.h>
#include <MString.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MSensitivity.h"
#include "MBackprojection.h"
#include "MBPData.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MPhysicalEventAnalyzer : public TObject
{
  // public interface:
 public:
  MPhysicalEventAnalyzer(Int_t CoordinateSystem);
  ~MPhysicalEventAnalyzer();

  void SetViewport(Double_t xMin, Double_t xMax, Int_t xNBins, 
                   Double_t yMin, Double_t yMax, Int_t yNBins,
                   Double_t zMin = 0, Double_t zMax = 0, Int_t zNBins = 1,
                   Double_t xAxis = 0, Double_t yAxis = 0, Double_t zAxis = 1);

  void SetGaussianFits(Double_t Longitudinal, Double_t Transversal, Double_t Pair);

  void SetSelectionTotalEnergy(Double_t MinEnergy, Double_t MaxEnergy);
  void SetSelectionEnergyGamma(Double_t MinEnergy, Double_t MaxEnergy);
  void SetSelectionEnergyElectron(Double_t MinEnergy, Double_t MaxEnergy);
  void SetSelectionTime(Double_t MinTime, Double_t MaxTime);
  void SetSelectionEventType(Bool_t Compton = kTRUE, Bool_t Double = kTRUE, 
                              Bool_t Pair = kTRUE, Bool_t NotTracked = kTRUE, 
                              Bool_t Tracked = kTRUE);
  void SetSelectionComptonAngle(Double_t MinComptonAngle, Double_t MaxComptonAngle);

  Bool_t Analyze(MPhysicalEvent* Event);
  MBPData* GetBackprojectionEvent();


  // protected methods:
 protected:


  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  MBackprojection *m_BP;    // the backprojection class
  MBPData *m_BackprojectionEvent;
  Int_t m_NBins;            // Number of bins (x*y*z) of the image

  // Event-Selection-Parameters:
  Double_t m_MinEnergy;          // min allowed energy of the incoming gamma-ray
  Double_t m_MaxEnergy;          // max allowed energy of the incoming gamma-ray

  Double_t m_MinEnergyGamma;     // min allowed energy of the scattered gamma-ray
  Double_t m_MaxEnergyGamma;     // max allowed energy of the scattered gamma-ray

  Double_t m_MinEnergyElectron;  // min allowed energy of the recoil electron
  Double_t m_MaxEnergyElectron;  // max allowed energy of the recoil electron

  Double_t m_MinComptonAngle;    // min allowed compton scatter angle
  Double_t m_MaxComptonAngle;    // max allowed compton scatter angle

  Double_t m_MinTime;            // min allowed time
  Double_t m_MaxTime;            // max allowed time

  Bool_t m_ComptonEvents;        // true if Compton-events are allowed
  Bool_t m_DoubleEvents;         // true if Double-events are allowed
  Bool_t m_PairEvents;           // true if Pair-events is allowed
  Bool_t m_TrackedEvents;        // true if Trackeds are allowed
  Bool_t m_NotTrackedEvents;     // true if Untrackeds are allowed
  



#ifdef ___CLING___
 public:
  ClassDef(MPhysicalEventAnalyzer, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
