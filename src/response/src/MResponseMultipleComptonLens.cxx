/*
 * MResponseMultipleComptonLens.cxx
 *
 * Copyright (C) by the MEGAlib contributors.
 *
 * This file is part of MEGAlib.
 *
 * MEGAlib is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * MEGAlib is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
 * License (License.md) for more details.
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
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


#ifdef ___CLING___
ClassImp(MResponseMultipleComptonLens)
#endif


////////////////////////////////////////////////////////////////////////////////


MResponseMultipleComptonLens::MResponseMultipleComptonLens()
{
  // Construct an instance of MResponseMultipleComptonLens
  
  m_ResponseNameSuffix = "lmc";
  
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
  MResponseBuilder::Save(); 

  m_PdfFromLensGood.Write(GetFilePrefix() + ".good" + m_Suffix, true);
  m_PdfFromLensBad.Write(GetFilePrefix() + ".bad" + m_Suffix, true);

  return true;
}

  
////////////////////////////////////////////////////////////////////////////////


//! Analyze the current event
bool MResponseMultipleComptonLens::Analyze() 
{ 
  // Initialize next matching event, save if necessary
  if (MResponseBuilder::Analyze() == false) return false;
  
  
  // Take only the best event and check if it originates from the lens
  MRawEventIncarnationList* List = m_ReReader->GetRawEventList();
  
  if (List->HasOnlyOptimumEvents() == true) {
    MRERawEvent* RE = List->GetOptimumEvents()[0];
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
