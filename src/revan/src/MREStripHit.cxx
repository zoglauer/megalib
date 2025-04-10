/*
 * MREStripHit.cxx
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
// MREStripHit.cxx
//
// This is the implementation of a Hit containing its position, its deposited
// energy and the detectortype.
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MREStripHit.h"

// Standard libs:
#include <iostream>
#include <cfloat>
#include <iomanip>
using namespace std;

// MEGAlib:
#include "MDGeometryQuest.h"
#include "MStreams.h"
#include "MTokenizer.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MREStripHit)
#endif


////////////////////////////////////////////////////////////////////////////////


MREStripHit::MREStripHit() : MRESE()
{
  m_SubElementType = MRESE::c_StripHit;
  m_IsValid = true;
  m_FixedResolutions = false;
  m_Position = g_VectorNotDefined;
  
  m_IsXStrip = true;
  m_StripID = g_UnsignedIntNotDefined;
  m_DetectorID = g_UnsignedLongNotDefined;
  m_NonStripPosition = g_DoubleNotDefined;
  m_DepthPosition = g_DoubleNotDefined;
}


////////////////////////////////////////////////////////////////////////////////


bool MREStripHit::ParseLine(MString HitString, int Version)
{
  // If an error occurs during reading the string, the hit is marked as
  // m_IsValid = false

  Reset();
  
  m_IsValid = false;
    
  return m_IsValid;
}


////////////////////////////////////////////////////////////////////////////////


MREStripHit::MREStripHit(MREStripHit* StripHit) : MRESE((MRESE *) StripHit)
{
  // Special copy-constructor

  m_IsValid = StripHit->m_IsValid; 
  m_SubElementType = MRESE::c_StripHit;
  m_FixedResolutions = StripHit->m_FixedResolutions;

  m_IsXStrip = StripHit->m_IsXStrip;
  m_StripID = StripHit->m_StripID;
  m_DetectorID = StripHit->m_DetectorID;
  m_NonStripPosition = StripHit->m_NonStripPosition;
  m_DepthPosition = StripHit->m_DepthPosition;
} 


////////////////////////////////////////////////////////////////////////////////


MREStripHit::~MREStripHit()
{
  // Delete this object, but do not delete its links and RESEs 
}


////////////////////////////////////////////////////////////////////////////////


double MREStripHit::ComputeMinDistance(MRESE *RESE)
{
  // Compute the minimum distance between this hit and the sub element:

  return ComputeMinDistanceVector(RESE).Mag();
}


////////////////////////////////////////////////////////////////////////////////


MVector MREStripHit::ComputeMinDistanceVector(MRESE *RESE)
{
  // Compute the minimum distance between this hit and the sub element.
  // The pointer points to the RESE.

  return MVector(DBL_MAX, DBL_MAX, DBL_MAX);
}


////////////////////////////////////////////////////////////////////////////////


bool MREStripHit::AreStripsAdjacent(MREStripHit* SH)
{
  // Check if the R are in neighboring pixels
  // The functionality must be implemented in the derived classes!

  if (SH->GetType() == MRESE::c_StripHit) {
    if (m_DetectorID == SH->m_DetectorID && m_IsXStrip == SH->m_IsXStrip && abs(m_StripID - SH->m_StripID) == 1) {
      return true;
    }
  }

  return false;
}


////////////////////////////////////////////////////////////////////////////////


MString MREStripHit::ToString(bool WithLink, int Level)
{
  // Returns a MString containing the relevant data of this object
  //
  // WithLink: Display the links
  // Level:    A level of N displays 3*N blancs before the text

  char Text[1000];

//   MString String("");
//   for (int i = 0; i < Level; i++) {
//     String += MString("   ");
//   }
//   sprintf(Text, "P: (%.3lf, %.3lf, %.3lf), E: %.3lf\n", 
//          m_Position.X(), m_Position.Y(), m_Position.Z(), m_Energy);
//   String += MString(Text);

  MString String("");
  sprintf(Text, "StripHit (%d) at %s-strip %d with %.2f+-%.2f keV in detector %d (ID: %ld)\n", 
          m_ID, (m_IsXStrip == true ? "x" : "y"), m_StripID, 
          m_Energy, m_EnergyResolution, m_Detector, m_DetectorID);
  for (int i = 0; i < Level; i++) {
    String += MString("   ");
  }
  String += MString(Text);

  if (WithLink == true && GetNLinks() > 0) {
    for (int i = 0; i < Level+1; i++) {
      String += MString("   ");
    }
    String += MString("Linked with: ");   
    for (int a = 0; a < GetNLinks(); a++) {
      sprintf(Text, "  (%d)", GetLinkAt(a)->GetID());
      String += MString(Text);
    }
    String += MString("\n");
  }

  return String;
}


////////////////////////////////////////////////////////////////////////////////


MString MREStripHit::ToEvtaString(const int ScientificPrecision, const int Version)
{  
  //! Convert to a string in the evta file

  ostringstream S;
  /*
  int WidthPos;
  int WidthEnergy;
  int WidthTime;
  int Precision;
  if (ScientificPrecision > 0) {
    WidthPos = ScientificPrecision+7;
    WidthEnergy = ScientificPrecision+6;
    WidthTime = ScientificPrecision+6;
    Precision = ScientificPrecision;
    S.setf(ios_base::scientific, ios_base::floatfield);
  } else {
    WidthPos = 10;
    WidthEnergy = 10;
    WidthTime = 11;
    Precision = 5;
    S.setf(ios_base::fixed, ios_base::floatfield);
  }

  S<<"SH "<<m_Detector<<";";
  S<<setprecision(Precision);
  S<<setw(WidthPos)<<m_Position[0]<<";";
  S<<setw(WidthPos)<<m_Position[1]<<";";
  S<<setw(WidthPos)<<m_Position[2]<<";";
  S<<setw(WidthEnergy)<<m_Energy<<";";
  S<<scientific;
  S<<setw(WidthTime)<<m_Time;
  S<<endl;

  */
  return S.str().c_str();
}


////////////////////////////////////////////////////////////////////////////////


void MREStripHit::AddRESE(MRESE* RESE)
{
  // Despite good object-oriented-programming, adding a RESE to a hit is not
  // very reasonable, so give a warning and do nothing.

  merr<<"You cannot add a RESE to a single strip!"<<endl;
}


////////////////////////////////////////////////////////////////////////////////


MREStripHit* MREStripHit::Duplicate()
{
  // duplicate this hit

  return new MREStripHit(this);
}


////////////////////////////////////////////////////////////////////////////////


bool MREStripHit::UpdateVolumeSequence(MDGeometryQuest* Geometry)
{
  /* This optimization is for another day...
  if (m_VolumeSequence != nullptr && 
      m_VolumeSequence->GetNVolumes() > 0 &&
      m_VolumeSequence->GetPosition(0) == m_Position) {
    return true; 
  }
  */
  
  MDVolumeSequence* V = Geometry->GetVolumeSequencePointer(m_Position, true, true);

  // Check if we do have a resonable volume sequence:
  if (V->GetDetector() == 0) {
    merr<<"MREStripHit::UpdateVolumeSequence: Found volume sequence without detector!"<<show;
    delete V;
    return false;
  } else {
    delete m_VolumeSequence;
    m_VolumeSequence = V;
  }
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MREStripHit::RetrieveResolutions(MDGeometryQuest* Geometry)
{
  // Do not do anything if we have fixed resolutions
  if (m_FixedResolutions == true) return true;
  
  if (UpdateVolumeSequence(Geometry) == false) return false;
  
  Geometry->GetResolutions(m_Position, m_Energy, m_Time, *m_VolumeSequence, m_PositionResolution, m_EnergyResolution, m_TimeResolution);
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


MDGridPointCollection MREStripHit::Grid(MDGeometryQuest* Geometry)
{
  // This function is only (O-N-L-Y) called when this hit has been simulated
  // It splits the hit into multiple ones the energy and position of the hit.
  
  //cout<<"Pos before: "<<m_Position[0]<<"!"<<m_Position[1]<<"!"<<m_Position[2]<<endl;
  
  if (m_Detector < MDDetector::c_MinDetector || m_Detector > MDDetector::c_MaxDetector) {
    merr<<"Unknown detector ID: "<<m_Detector<<show;
    return MDGridPointCollection(*m_VolumeSequence); ;
  }

  if (Geometry == nullptr) {
    return MDGridPointCollection(*m_VolumeSequence); 
  }
  
  return Geometry->Grid(m_Position, m_Energy, m_Time, *m_VolumeSequence);
}


////////////////////////////////////////////////////////////////////////////////


bool MREStripHit::Noise(MDGeometryQuest* Geometry)
{
  // This function is only (O-N-L-Y) called when this hit has been simulated
  // It noises the energy and position of the hit.
  
  return false;
}


// MREStripHit.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
