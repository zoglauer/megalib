/*
 * MPhysicalEventAnalyzer.cxx                                   v0.1  01/01/2001
 *
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 *
 * This code implementation is the intellectual property of
 * Andreas Zoglauer.
 *
 * By copying, distributing or modifying the Program (or any work
 * based on the Program) you indicate your acceptance of this statement,
 * and all its terms.
 *
 */


////////////////////////////////////////////////////////////////////////////////
//
// MPhysicalEventAnalyzer
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MPhysicalEventAnalyzer.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MBackprojectionFarFieldArea.h"
#include "MBackprojectionCart2DArea.h"
#include "MBackprojectionCart3DArea.h"
#include "MResponseGaussian.h"
#include "MBPDataSparseImageOneByte.h"
#include "MBPDataImageOneByte.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MPhysicalEventAnalyzer)
#endif


////////////////////////////////////////////////////////////////////////////////


MPhysicalEventAnalyzer::MPhysicalEventAnalyzer(Int_t CoordinateSystem)
{
  // Construct an instance of MPhysicalEventAnalyzer

  // Initialize the backprojection-class
  if (CoordinateSystem == 0) {
    m_BP = (MBackprojection *) new MBackprojectionFarFieldArea();
  } else if (CoordinateSystem == 1) {
    m_BP = (MBackprojection *) new MBackprojectionCart2DArea();
  } else if (CoordinateSystem == 2) {
    m_BP = (MBackprojection *) new MBackprojectionCart3DArea();
  } else {
    Fatal("MPhysicalEventAnalyzer::MPhysicalEventAnalyzer(Int_t CoordinateSystem)", 
          "Unkown coordinate system: %d", CoordinateSystem);
  }      
}


////////////////////////////////////////////////////////////////////////////////


MPhysicalEventAnalyzer::~MPhysicalEventAnalyzer()
{
  // Delete this instance of MPhysicalEventAnalyzer
}


////////////////////////////////////////////////////////////////////////////////


void MPhysicalEventAnalyzer::SetViewport(Double_t xMin, Double_t xMax, Int_t xNBins, 
                                         Double_t yMin, Double_t yMax, Int_t yNBins,
                                         Double_t zMin, Double_t zMax, Int_t zNBins,
                                         Double_t xAxis, Double_t yAxis, Double_t zAxis)
{
  // Set the viewport of this event
  // In spherical coordinates x means theta and y means phi

  m_BP->SetViewportDimensions(xMin, xMax, xNBins, 
                              yMin, yMax, yNBins, 
                              zMin, zMax, zNBins,
                              xAxis, yAxis, zAxis);
   
  m_NBins = xNBins*yNBins*zNBins;

  m_BP->PrepareBackprojection();
}


////////////////////////////////////////////////////////////////////////////////


void MPhysicalEventAnalyzer::SetGaussianFits(Double_t Longitudinal, 
                                             Double_t Transversal, Double_t Pair)
{
  // Set the fit-parameter of the backprojections

  // ---> zu ändern!
  m_BP->SetGaussianFits(Longitudinal, Transversal, Pair);
  
  MResponseGaussian *Response = new MResponseGaussian();
  Response->SetGaussianFits(Longitudinal, Transversal);

  m_BP->SetResponse((MResponse *) Response);
}


////////////////////////////////////////////////////////////////////////////////


void MPhysicalEventAnalyzer::SetSelectionTotalEnergy(Double_t MinEnergy, Double_t MaxEnergy)
{
  // set the minimum and maximum allowed energy

  m_MinEnergy = MinEnergy;
  m_MaxEnergy = MaxEnergy;
}


////////////////////////////////////////////////////////////////////////////////


void MPhysicalEventAnalyzer::SetSelectionEnergyGamma(Double_t MinEnergy, Double_t MaxEnergy)
{
  // set the minimum and maximum allowed energy of the scattered gamma

  m_MinEnergyGamma = MinEnergy;
  m_MaxEnergyGamma = MaxEnergy;
}


////////////////////////////////////////////////////////////////////////////////


void MPhysicalEventAnalyzer::SetSelectionEnergyElectron(Double_t MinEnergy, 
                                                    Double_t MaxEnergy)
{
  // set the minimum and maximum allowed energy of the recoil electron

  m_MinEnergyElectron = MinEnergy;
  m_MaxEnergyElectron = MaxEnergy;
}

void MPhysicalEventAnalyzer::SetSelectionComptonAngle(Double_t MinComptonAngle, Double_t MaxComptonAngle)
{
  // set the minimum and maximum allowed energy of the recoil electron

  m_MinComptonAngle = MinComptonAngle;
  m_MaxComptonAngle = MaxComptonAngle;
}

////////////////////////////////////////////////////////////////////////////////


void MPhysicalEventAnalyzer::SetSelectionTime(Double_t MinTime, Double_t MaxTime)
{
  // set the minimum and maximum allowed time

  m_MinTime = MinTime;
  m_MaxTime = MaxTime;
}


////////////////////////////////////////////////////////////////////////////////


void MPhysicalEventAnalyzer::SetSelectionEventType(Bool_t Compton, Bool_t Double, 
                                                   Bool_t Pair, Bool_t NotTracked, 
                                                   Bool_t Tracked)
{
  // Set the allowed event types:

  m_ComptonEvents = Compton;
  m_DoubleEvents = Double;
  m_PairEvents = Pair;

  m_TrackedEvents = Tracked;
  m_NotTrackedEvents = NotTracked;
}


////////////////////////////////////////////////////////////////////////////////


Bool_t MPhysicalEventAnalyzer::Analyze(MPhysicalEvent* Event)
{
  //

  Bool_t GoodEvent = kFALSE;
  m_BackprojectionEvent = 0;
  Double_t Limit = 0;
  Int_t NAboveLimit = 0;
  Double_t *SingleBackprojection = new Double_t[m_NBins];
  Int_t i;
  for (i = 0; i < m_NBins; i++) {
    SingleBackprojection[i] = 0.0;
  }

  if (m_BP->Backproject(Event, SingleBackprojection, Limit, NAboveLimit) == kTRUE) {
    GoodEvent = kTRUE;
    // Store as sparse matrix ...
    if (NAboveLimit < 0.33*m_NBins) {
      m_BackprojectionEvent = (MBPData *) new MBPDataSparseImageOneByte(SingleBackprojection, 
                                                                        m_NBins, 
                                                                        Limit, 
                                                                        NAboveLimit);
    }
    // ... or as a simple image matrix.
    else { 
      m_BackprojectionEvent = (MBPData *) new MBPDataImageOneByte(SingleBackprojection, m_NBins);
    }
  }
   
  delete [] SingleBackprojection;

  return GoodEvent;
}


////////////////////////////////////////////////////////////////////////////////


MBPData* MPhysicalEventAnalyzer::GetBackprojectionEvent()
{
  //

  return m_BackprojectionEvent;
}


////////////////////////////////////////////////////////////////////////////////



// MPhysicalEventAnalyzer.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
