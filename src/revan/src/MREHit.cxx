/*
 * MREHit.cxx
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
// MREHit.cxx
//
// This is the implementation of a Hit containing its position, its deposited
// energy and the detectortype.
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MREHit.h"

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


#ifdef ___CINT___
ClassImp(MREHit)
#endif


////////////////////////////////////////////////////////////////////////////////


MREHit::MREHit() : MRESE()
{
  m_SubElementType = MRESE::c_Hit;
  m_IsValid = true;
}


////////////////////////////////////////////////////////////////////////////////


MREHit::MREHit(MVector Position, double Energy, double Time, int Detector, 
               MVector PositionRes, double EnergyRes, double TimeRes) :
  MRESE(Position, Energy, Time, Detector, PositionRes, EnergyRes, TimeRes)
{
  // Constructs a hit with the values:
  //
  // Position:  Location of the hit 
  // Energy:    deposited Energy
  // Detector:  Number of the detector, where the hit took place 
  //            (for more info see MRESE::SetDetector(int Detector))

  m_SubElementType = MRESE::c_Hit;
  m_IsValid = true;
}


////////////////////////////////////////////////////////////////////////////////


MREHit::MREHit(MString HitString, int Version) : MRESE()
{
  // Constructs a hit out of a String. The String must be of the form:
  //
  // HTsim%d;%lf;%lf;%lf;%lf\n (Detector, xPos, yPos, zPos, Energy)
  // HT%d;%lf;%lf;%lf;%lf\n (Detector, xPos, yPos, zPos, Energy)
  //
  // If an error occures during reading the string, the hit is marked as
  // m_IsValid = false

  m_SubElementType = MRESE::c_Hit;
  m_IsValid = true;

  if (Version == 1 || Version == 20) { 
    if (sscanf(HitString, "HTsim%d;%lf;%lf;%lf;%lf\n", 
               &m_Detector, 
               &m_Position[0], 
               &m_Position[1], 
               &m_Position[2], 
               &m_Energy) == 5) {
      m_IsValid = true;
    } else {
      m_IsValid = false;
      m_SubElementType = MRESE::c_Unknown;
      merr<<"Unable to read event:"<<endl
          <<HitString<<endl;
    }
  } else if (Version == 3 || Version >= 21) { 
    if (sscanf(HitString, "HTsim%d;%lf;%lf;%lf;%lf;%lf\n", 
               &m_Detector, 
               &m_Position[0], 
               &m_Position[1], 
               &m_Position[2], 
               &m_Energy, 
               &m_Time) == 6) {
      m_IsValid = true;
    } else if (sscanf(HitString, "HT %d;%lf;%lf;%lf;%lf;%lf\n",
                      &m_Detector,
                      &m_Position[0], 
                      &m_Position[1], 
                      &m_Position[2], 
                      &m_Energy,
                      &m_Time) == 6) {
      m_IsValid = true;
    } else if (sscanf(HitString, "HT %d;%lf;%lf;%lf;%lf\n",
                      &m_Detector,
                      &m_Position[0], 
                      &m_Position[1], 
                      &m_Position[2], 
                      &m_Energy) == 5) {
      m_IsValid = true;
    } else if (sscanf(HitString, "HT %d;%lf;%lf;%lf;%lf;%lf;%lf;%lf;%lf;OK\n",
                      &m_Detector,
                      &m_Position[0], 
                      &m_Position[1], 
                      &m_Position[2], 
                      &m_Energy, 
                      &m_PositionResolution[0], 
                      &m_PositionResolution[1], 
                      &m_PositionResolution[2], 
                      &m_EnergyResolution) == 9) {
      m_IsValid = true;
    } else {
      m_IsValid = false;
      m_SubElementType = MRESE::c_Unknown;
      merr<<"Unable to read event:"<<endl
          <<HitString<<endl;
    }
  } else if (Version == 4) { // MEGA Duke evta file:
    MTokenizer T(HitString, ';', false);
    if (T.GetNTokens() == 10) {
      sscanf(T.GetTokenAt(0), "HT%*4s%d", &m_Detector);
      m_Position.SetXYZ(T.GetTokenAtAsDouble(1), 
                        T.GetTokenAtAsDouble(2), 
                        T.GetTokenAtAsDouble(3));
      m_PositionResolution.SetXYZ(T.GetTokenAtAsDouble(6), 
                                  T.GetTokenAtAsDouble(7), 
                                  T.GetTokenAtAsDouble(8));
      m_Energy = T.GetTokenAtAsDouble(4);
      m_EnergyResolution = T.GetTokenAtAsDouble(9);
      m_IsValid = true;
      if (T.GetTokenAt(5) != "OK") {
        if (T.GetTokenAt(5).Contains("OF") == true) {
          m_IsValid = false;
        } else if (T.GetTokenAt(5).Contains("BA") == true || 
                   T.GetTokenAt(5).Contains("XO") == true || 
                   T.GetTokenAt(5).Contains("YO") == true) {
          m_IsValid = false;
        }
      }
    } else {
      m_IsValid = false;
      m_SubElementType = MRESE::c_Unknown;
      merr<<"Unable to read HT:"<<endl
          <<HitString<<endl;
    }
  } else { // old evta and sim...
    if (sscanf(HitString, "HTsim%d;%lf;%lf;%lf;%lf;%lf\n", 
               &m_Detector, &m_Position[0], &m_Position[1], &m_Position[2], &m_Energy, &m_Time) == 6) {
      m_IsValid = true;
    } else if (sscanf(HitString, "HTsim%d;%lf;%lf;%lf;%lf\n", 
                      &m_Detector, &m_Position[0], &m_Position[1], &m_Position[2], &m_Energy) == 5) {
      m_IsValid = true;
    } else if (sscanf(HitString, "HT%*4s%d;%lf;%lf;%lf;%lf\n", 
                      &m_Detector, &m_Position[0], &m_Position[1], &m_Position[2], &m_Energy) == 5) {
      m_IsValid = true;
    } else if (sscanf(HitString, "HT%*4s%d;%lf;%*s", 
                      &m_Detector, &m_Energy) == 2) {
      m_IsValid = true;
    } else {
      m_IsValid = false;
      m_SubElementType = MRESE::c_Unknown;
      merr<<"Unable to read event:"<<endl
          <<HitString<<endl;
    }
  }
}


////////////////////////////////////////////////////////////////////////////////


MREHit::MREHit(MREHit *Hit) : MRESE((MRESE *) Hit)
{
  // Special copy-constructor

  m_IsValid = Hit->IsValid();
  m_SubElementType = MRESE::c_Hit;
} 


////////////////////////////////////////////////////////////////////////////////


MREHit::~MREHit()
{
  // Delete this object, but do not delete its links and RESEs 
}


////////////////////////////////////////////////////////////////////////////////


double MREHit::ComputeMinDistance(MRESE *RESE)
{
  // Compute the minimum distance between this hit and the sub element:

  return ComputeMinDistanceVector(RESE).Mag();
}


////////////////////////////////////////////////////////////////////////////////


MVector MREHit::ComputeMinDistanceVector(MRESE *RESE)
{
  // Compute the minimum distance between this hit and the sub element.
  // The pointer points to the RESE.

  if (RESE->GetType() == MRESE::c_Hit) {
    return RESE->GetPosition() - m_Position;
  } else if (RESE->GetType() == MRESE::c_Cluster) {
    return RESE->ComputeMinDistanceVector(this);
  } else if (RESE->GetType() == MRESE::c_Track) {
    return RESE->ComputeMinDistanceVector(this);
  }

  // Change this to g_VectorNotDefined one day...

  return MVector(DBL_MAX, DBL_MAX, DBL_MAX);
}


////////////////////////////////////////////////////////////////////////////////


bool MREHit::AreAdjacent(MRESE* R, double Sigma, int Level)
{
  // Check if the R are in neighboring pixels
  // The functionality must be implemented in the derived classes!

  if (R->GetType() == MRESE::c_Hit) {
    if (m_VolumeSequence->HasSameDetector(*(R->GetVolumeSequence())) == true) {
      return m_VolumeSequence->GetDetector()
        ->AreNear(m_VolumeSequence->GetPositionInDetector(),
                  m_PositionResolution,
                  R->GetVolumeSequence()->GetPositionInDetector(),
                  R->GetPositionResolution(),
                  Sigma, Level);
    }
  } else if (R->GetType() == MRESE::c_Cluster) {
    return R->AreAdjacent(this, Sigma, Level);
  } else if (R->GetType() == MRESE::c_Track) {
    return R->AreAdjacent(this, Sigma, Level);
  }

  return false;
}


////////////////////////////////////////////////////////////////////////////////


MString MREHit::ToString(bool WithLink, int Level)
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
  sprintf(Text, "Hit (%d) at (%.3f, %.3f, %.3f)+-(%.3f, %.3f, %.3f) with %.2f+-%.2f keV in detector %d\n", 
          m_ID, m_Position.X(), m_Position.Y(), m_Position.Z(), 
          m_PositionResolution.X(), m_PositionResolution.Y(), m_PositionResolution.Z(), 
          m_Energy, m_EnergyResolution, m_Detector);
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


MString MREHit::ToEvtaString(const int ScientificPrecision, const int Version)
{  
  //! Convert to a string in the evta file

  ostringstream S;

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

  S<<"HT "<<m_Detector<<";";
  S<<setprecision(Precision);
  S<<setw(WidthPos)<<m_Position[0]<<";";
  S<<setw(WidthPos)<<m_Position[1]<<";";
  S<<setw(WidthPos)<<m_Position[2]<<";";
  S<<setw(WidthEnergy)<<m_Energy<<";";
  S<<scientific;
  S<<setw(WidthTime)<<m_Time;
  S<<endl;

  return S.str().c_str();
}


////////////////////////////////////////////////////////////////////////////////


void MREHit::AddRESE(MRESE* RESE)
{
  // Despite good object-oriented-programming, adding a RESE to a hit is not
  // very reasonable, so give a warning and do nothing.

  merr<<"You cannot add a RESE to a single hit!"<<endl;
}


////////////////////////////////////////////////////////////////////////////////


MREHit* MREHit::Duplicate()
{
  // duplicate this hit

  return new MREHit(this);
}


////////////////////////////////////////////////////////////////////////////////


bool MREHit::RetrieveResolutions(MDGeometryQuest* Geometry)
{
  MDVolumeSequence* V = Geometry->GetVolumeSequencePointer(m_Position, true, true);

  // Check if we do have a resonable volume sequence:
  if (V->GetDetector() == 0) {
    merr<<"Found volume sequence without detector!"<<show;
    delete V;
    return false;
  } else {
    delete m_VolumeSequence;
    m_VolumeSequence = V;
    Geometry->GetResolutions(m_Position, m_Energy, m_Time, *m_VolumeSequence, m_PositionResolution, m_EnergyResolution, m_TimeResolution);
  }
  
  return true;
}
  
  
////////////////////////////////////////////////////////////////////////////////


bool MREHit::Noise(MDGeometryQuest* Geometry)
{
  // This function is only (O-N-L-Y) called when this hit has been simulated
  // It noises the energy and position of the hit.

  //cout<<"Pos before: "<<m_Position[0]<<"!"<<m_Position[1]<<"!"<<m_Position[2]<<endl;
  if (Geometry != 0) {
    Geometry->Noise(m_Position, m_Energy, m_Time, *m_VolumeSequence);
  }
  //cout<<"Pos after: "<<m_Position[0]<<"!"<<m_Position[1]<<"!"<<m_Position[2]<<endl;

  if (m_Detector < MDDetector::c_MinDetector || 
      m_Detector > MDDetector::c_MaxDetector) {
    merr<<"Unknown detector ID: "<<m_Detector<<show;
    return false;
  }

  if (m_Energy > 0) {
    return true;
  } else {
    return false;
  }
}


// MREHit.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
