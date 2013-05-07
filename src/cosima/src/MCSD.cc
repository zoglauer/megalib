/*
 * MCSD.cxx
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


// Cosima:
#include "MCSD.hh"

// MEGAlib:
#include "MDDetector.h"


/******************************************************************************/


const int MCSD::c_Unknown      = MDDetector::c_NoDetectorType;
const int MCSD::c_2DStrip      = MDDetector::c_Strip2D;
const int MCSD::c_Calorimeter  = MDDetector::c_Calorimeter;
const int MCSD::c_3DStrip      = MDDetector::c_Strip3D;
const int MCSD::c_Scintillator = MDDetector::c_Scintillator;
const int MCSD::c_DriftChamber = MDDetector::c_DriftChamber;
const int MCSD::c_Voxel3D      = MDDetector::c_Voxel3D;
const int MCSD::c_AngerCamera  = MDDetector::c_AngerCamera;


/******************************************************************************
 * Default constructor, which only sets the name and the type to Unknown
 */
MCSD::MCSD(G4String Name) : G4VSensitiveDetector(Name), 
                            m_Type(c_Unknown),
                            m_DiscretizeHits(true),
                            m_UseEnergyLoss(false),
                            m_HasTimeResolution(false),
                            m_Epsilon(1E-10)
{
  // Intentionally left blank

}


/******************************************************************************
 * Not yet implemented
 */
MCSD::~MCSD()
{
  // Intentionally left blank
}


/******************************************************************************
 * Return the type of the detector:
 * c_Unknown = 0;
 * c_2DStrip = 1;
 * c_Calorimeter = 2;
 * c_Scintillator = 4;
 */
int MCSD::GetType()
{
  return m_Type;
}

/*
 * MCSD.cc: the end...
 ******************************************************************************/
