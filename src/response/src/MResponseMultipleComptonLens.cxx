/*
 * MResponseMultipleComptonLens.cxx
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
// MResponseMultipleComptonLens
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MResponseMultipleComptonLens.h"

// Standard libs:
#include <vector>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MAssert.h"
#include "MStreams.h"
#include "MResponseMatrixO1.h"
#include "MResponseMatrixO2.h"
#include "MResponseMatrixO3.h"
#include "MResponseMatrixO4.h"
#include "MResponseMatrixO5.h"
#include "MResponseMatrixO6.h"
#include "MResponseMatrixO7.h"
#include "MSettingsRevan.h"
#include "MRESEIterator.h"
#include "MERCSRBayesian.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MResponseMultipleComptonLens)
#endif


////////////////////////////////////////////////////////////////////////////////


MResponseMultipleComptonLens::MResponseMultipleComptonLens()
{
  // Construct an instance of MResponseMultipleComptonLens

  m_IsLensOrigin = true;
  m_LensCenter = g_VectorNotDefined;
  m_FocalSpotCenter = g_VectorNotDefined;
}


////////////////////////////////////////////////////////////////////////////////


MResponseMultipleComptonLens::~MResponseMultipleComptonLens()
{
  // Delete this instance of MResponseMultipleComptonLens
}

  
////////////////////////////////////////////////////////////////////////////////


//! Initialize the response matrices and their generation
bool MResponseMultipleComptonLens::Initialize() 
{ 
  // Initialize next matching event, save if necessary
  if (MResponseBuilder::Initialize() == false) return false;
  
  m_LensCenter = m_RevanSettings.GetLensCenter();
  m_FocalSpotCenter = m_RevanSettings.GetFocalSpotCenter();


  vector<float> AxisConeLensDistance;
  vector<float> AxisRadiusBeamCenter;
  vector<float> AxisInteractionDepth;
  
  AxisConeLensDistance = CreateLogDist(0.1, 180, 100, c_NoBound, 181, 0, false);
  AxisRadiusBeamCenter = CreateEquiDist(0, 10, 1, c_NoBound, c_NoBound); //, 10000);
  AxisInteractionDepth = CreateEquiDist(0, 10, 1, c_NoBound, c_NoBound); //, 0000); 

  m_PdfFromLensGood = MResponseMatrixO3("Originates from lens - good", 
                                        AxisConeLensDistance, 
                                        AxisRadiusBeamCenter, 
                                        AxisInteractionDepth);
  m_PdfFromLensGood.SetAxisNames("Cone-lens distance [deg]", 
                                 "Radius from beam axis [cm]", 
                                 "Interaction depth [cm]");
  m_PdfFromLensBad = MResponseMatrixO3("Originates from lens - bad", 
                                       AxisConeLensDistance, 
                                       AxisRadiusBeamCenter, 
                                       AxisInteractionDepth);
  m_PdfFromLensBad.SetAxisNames("Cone-lens distance [deg]", 
                                "Radius from beam axis [cm]", 
                                "Interaction depth [cm]");

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseMultipleComptonLens::Save()
{
  m_PdfFromLensGood.Write(m_ResponseName + ".lmc.good" + m_Suffix, true);
  m_PdfFromLensBad.Write(m_ResponseName + ".lmc.bad" + m_Suffix, true);

  return true;
}

  
////////////////////////////////////////////////////////////////////////////////


//! Analyze the current event
bool MResponseMultipleComptonLens::Analyze() 
{ 
  // Initialize next matching event, save if necessary
  if (MResponseBuilder::Analyze() == false) return false;
  
  
  // Take only the best event and check if it originates from the lens
  MRawEventList* REList = m_ReReader->GetRawEventList();
  
  if (REList->HasOptimumEvent() == true) {
    MRERawEvent* RE = REList->GetOptimumEvent();
    if (RE->GetNRESEs() > 1) {
      
      // Determine all parameters:
      double ConeLensDistance = CalculateConeLensDistance(*(RE->GetStartPoint()), *(RE->GetStartPoint()->GetLinkAt(0)), RE->GetEnergy())*c_Deg;
      double Radius = CalculateRadiusFromBeamCenter(*(RE->GetStartPoint()));
      double InteractionDepth = CalculateInteractionDepth(*(RE->GetStartPoint()));
      
      cout<<ConeLensDistance<<":"<<Radius<<":"<<InteractionDepth<<endl;
      
      if (m_IsLensOrigin == true) {
        m_PdfFromLensGood.Add(ConeLensDistance, Radius, InteractionDepth);
      } else {
        m_PdfFromLensBad.Add(ConeLensDistance, Radius, InteractionDepth);
      }
      
    }
  }
  
  return true;
}

  
////////////////////////////////////////////////////////////////////////////////


//! Finalize the response generation (i.e. save the data a final time )
bool MResponseMultipleComptonLens::Finalize() 
{ 
  return MResponseBuilder::Finalize(); 
}


////////////////////////////////////////////////////////////////////////////////


double MResponseMultipleComptonLens::CalculateConeLensDistance(MRESE& First, 
                                                               MRESE& Second, 
                                                               double Etot)
{
  // 

  double Phi = MComptonEvent::ComputePhiViaEeEg(First.GetEnergy(), Etot - First.GetEnergy());

  return fabs((First.GetPosition() - Second.GetPosition()).Angle(m_LensCenter - First.GetPosition()) - Phi);
}


////////////////////////////////////////////////////////////////////////////////


double MResponseMultipleComptonLens::CalculateRadiusFromBeamCenter(MRESE& First)
{
  // The same equations appears in MEventSelector
  return (First.GetPosition() - m_FocalSpotCenter).Cross(m_LensCenter - m_FocalSpotCenter).Mag()/(m_LensCenter - m_FocalSpotCenter).Mag();
}


////////////////////////////////////////////////////////////////////////////////


double MResponseMultipleComptonLens::CalculateInteractionDepth(MRESE& First)
{
  // Assumption: neglectable deviation from on axis incidence:

  // The same equations appears in MEventSelector
  return (First.GetPosition() - m_FocalSpotCenter).Dot(m_FocalSpotCenter - m_LensCenter)/(m_FocalSpotCenter - m_LensCenter).Mag();
}

// MResponseMultipleComptonLens.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
