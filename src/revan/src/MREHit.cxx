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


#ifdef ___CLING___
ClassImp(MREHit)
#endif


////////////////////////////////////////////////////////////////////////////////


MREHit::MREHit() : MRESE()
{
  m_SubElementType = MRESE::c_Hit;
  m_IsValid = true;
  m_FixedResolutions = false;
}


////////////////////////////////////////////////////////////////////////////////


bool MREHit::ParseLine(MString HitString, int Version)
{
  // If an error occurs during reading the string, the hit is marked as
  // m_IsValid = false

  Reset();

  // Handle the most common MEGAlib 2.x sim / evta file versions
  if (Version == 25 && HitString.BeginsWith("HTsim ")) {
    Version = 101;
  } else if (Version == 21 && HitString.BeginsWith("HT ")) {
    Version = 200;
  }

  m_IsValid = true;
  m_FixedResolutions = false;


  if (Version == 100) {
    vector<MString> Splits = HitString.Tokenize(";");
    if (Splits.size() >= 5) {
      vector<MString> SubSplit = Splits[0].Tokenize(" ");
      if (SubSplit.size() == 2) {
        m_Detector = atoi(SubSplit[1]);
      
        m_Position[0] = atof(Splits[1]);
        m_Position[1] = atof(Splits[2]);
        m_Position[2] = atof(Splits[3]);
        m_Energy = atof(Splits[4]);
        
        for (unsigned int s = 5; s < Splits.size(); ++s) {
          //cout<<"Adding origin: "<<atoi(Splits[s])<<endl;
          m_OriginIDs.insert(m_OriginIDs.begin(), atoi(Splits[s])); 
        }

        m_Time = 0;
        m_PositionResolution.SetXYZ(0.0, 0.0, 0.0);
        m_EnergyResolution = 0.0;
        m_TimeResolution = 0.0;
        m_IsValid = true;
      }
    }
  } else if (Version == 101) {
    vector<MString> Splits = HitString.Tokenize(";");
    if (Splits.size() >= 6) {
      vector<MString> SubSplit = Splits[0].Tokenize(" ");
      if (SubSplit.size() == 2) {
        m_Detector = atoi(SubSplit[1]);
        
        m_Position[0] = atof(Splits[1]);
        m_Position[1] = atof(Splits[2]);
        m_Position[2] = atof(Splits[3]);
        m_Energy = atof(Splits[4]);
        m_Time = atof(Splits[5]);

        for (unsigned int s = 6; s < Splits.size(); ++s) {
          //cout<<"Adding origin: "<<atoi(Splits[s])<<endl;
          m_OriginIDs.insert(m_OriginIDs.begin(), atoi(Splits[s])); 
        }
        //cout<<"Origin IDs: "; for (auto I: m_OriginIDs) cout<<I<<" "; cout<<endl;
        
        m_PositionResolution.SetXYZ(0.0, 0.0, 0.0);
        m_EnergyResolution = 0.0;
        m_TimeResolution = 0.0;
        m_IsValid = true;
      }
    }
  } else if (Version == 200) {
    if (sscanf(HitString, "HT %d;%lf;%lf;%lf;%lf;%lf;%lf;%lf;%lf\n",
               &m_Detector,
               &m_Position[0],
               &m_Position[1],
               &m_Position[2],
               &m_Energy,
               &m_PositionResolution[0],
               &m_PositionResolution[1],
               &m_PositionResolution[2],
               &m_EnergyResolution) == 9) {
      m_Time = 0;
      m_TimeResolution = 0;
      m_IsValid = true;
      m_FixedResolutions = true;
    }
  } else if (Version == 201) {
    if (sscanf(HitString, "HT %d;%lf;%lf;%lf;%lf;%lf;%lf;%lf;%lf;%lf;%lf\n",
               &m_Detector,
               &m_Position[0],
               &m_Position[1],
               &m_Position[2],
               &m_Energy,
               &m_Time,
               &m_PositionResolution[0],
               &m_PositionResolution[1],
               &m_PositionResolution[2],
               &m_EnergyResolution,
               &m_TimeResolution
              ) == 11) {
      m_IsValid = true;
      m_FixedResolutions = true;
    }
  } else if (Version == 4) { // The original MEGA Duke format:
    MTokenizer T(HitString, ';', false);
    if (T.GetNTokens() == 10) {
      sscanf(T.GetTokenAt(0), "HT%*4s%d", &m_Detector);
      m_Position.SetXYZ(T.GetTokenAtAsDouble(1), T.GetTokenAtAsDouble(2), T.GetTokenAtAsDouble(3));
      m_PositionResolution.SetXYZ(T.GetTokenAtAsDouble(6), T.GetTokenAtAsDouble(7), T.GetTokenAtAsDouble(8));
      m_Energy = T.GetTokenAtAsDouble(4);
      m_EnergyResolution = T.GetTokenAtAsDouble(9);
      m_IsValid = true;
      if (T.GetTokenAt(5) != "OK") {
        if (T.GetTokenAt(5).Contains("OF") == true) {
          m_IsValid = true; // Just an overflow 
        } else if (T.GetTokenAt(5).Contains("BA") == true || 
          T.GetTokenAt(5).Contains("XO") == true || 
          T.GetTokenAt(5).Contains("YO") == true) {
          m_IsValid = false;
        }
      }
    } else {
      m_IsValid = false;
    }
  } else {
    merr<<"Unknown version of sim/evta file (Version from file: "<<Version<<"), please upgrade (or use old version of MEGAlib prior to 3.0)"<<endl;
    m_IsValid = false;
  }
  
  return m_IsValid;
}


////////////////////////////////////////////////////////////////////////////////


MREHit::MREHit(MREHit *Hit) : MRESE((MRESE *) Hit)
{
  // Special copy-constructor

  m_IsValid = Hit->m_IsValid; 
  m_SubElementType = MRESE::c_Hit;
  m_FixedResolutions = Hit->m_FixedResolutions;
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

  const int Length = 5000;
  char Text[Length];

//   MString String("");
//   for (int i = 0; i < Level; i++) {
//     String += MString("   ");
//   }
//   sprintf(Text, Length, "P: (%.3lf, %.3lf, %.3lf), E: %.3lf\n",
//          m_Position.X(), m_Position.Y(), m_Position.Z(), m_Energy);
//   String += MString(Text);

  MString String("");
  snprintf(Text, Length, "Hit (%d) at (%.3f, %.3f, %.3f)+-(%.3f, %.3f, %.3f) with %.2f+-%.2f keV in detector %d\n",
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
      snprintf(Text, Length, "  (%d)", GetLinkAt(a)->GetID());
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


bool MREHit::UpdateVolumeSequence(MDGeometryQuest* Geometry)
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
    merr<<"MREHit::UpdateVolumeSequence: Found volume sequence without detector!"<<show;
    delete V;
    return false;
  } else {
    delete m_VolumeSequence;
    m_VolumeSequence = V;
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MREHit::RetrieveResolutions(MDGeometryQuest* Geometry)
{
  // Do not do anything if we have fixed resolutions
  if (m_FixedResolutions == true) return true;

  if (UpdateVolumeSequence(Geometry) == false) return false;

  Geometry->GetResolutions(m_Position, m_Energy, m_Time, *m_VolumeSequence, m_PositionResolution, m_EnergyResolution, m_TimeResolution);

  return true;
}


////////////////////////////////////////////////////////////////////////////////


MDGridPointCollection MREHit::Grid(MDGeometryQuest* Geometry)
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


bool MREHit::Noise(MDGeometryQuest* Geometry)
{
  // This function is only (O-N-L-Y) called when this hit has been simulated
  // It noises the energy and position of the hit.

  //cout<<"Pos before: "<<m_Position[0]<<"!"<<m_Position[1]<<"!"<<m_Position[2]<<endl;
  if (Geometry != 0) {
    m_NoiseFlags = "";
    Geometry->Noise(m_Position, m_Energy, m_Time, m_NoiseFlags, *m_VolumeSequence);
    cout<<"NF: "<<m_NoiseFlags<<endl;
  }
  //cout<<"Pos after: "<<m_Position[0]<<"!"<<m_Position[1]<<"!"<<m_Position[2]<<endl;

  if (m_Detector < MDDetector::c_MinDetector || m_Detector > MDDetector::c_MaxDetector) {
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
