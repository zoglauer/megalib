/*
 * MDGeometryQuest.cxx
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
// MDGeometryQuest
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MDGeometryQuest.h"

// Standard libs:
#include <iostream>
#include <iomanip>
#include <map>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MStreams.h"
#include "MAssert.h"
#include "MDVolume.h"
#include "MDShapeSPHE.h"
#include "MDShapeTUBS.h"
#include "MDShapeBRIK.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MDGeometryQuest)
#endif


////////////////////////////////////////////////////////////////////////////////


MDGeometryQuest::MDGeometryQuest() : 
  m_AllwaysAssumeTrigger(false), m_ActivateNoising(true)
{
  // default constructor  

  m_Hits.clear();
  m_Links.clear();
}



////////////////////////////////////////////////////////////////////////////////


MDGeometryQuest::~MDGeometryQuest()
{
  // default destructor

  for (unsigned int i = 0; i < m_Hits.size(); ++i) {
    delete m_Hits[i];
  }

  for (unsigned int i = 0; i < m_Links.size(); ++i) {
    delete m_Links[i];
  }
}


////////////////////////////////////////////////////////////////////////////////


bool MDGeometryQuest::IsSensitive(MVector Pos)
{ 
  // Return true if Pos is inside a sensitive volume

  if (m_GeometryScanned == false) {
    mgui<<"Geometry has to be scanned first"<<error;
    merr<<"Geometry has to be scanned first"<<endl;
    return false;
  }

  MDVolume *V = m_WorldVolume->GetVolume(Pos);

  if (V->GetDetector() != 0) {
    return true;
  } else {
    return false;
  }
}


////////////////////////////////////////////////////////////////////////////////


MDVolume* MDGeometryQuest::GetVolume(MVector Pos)
{ 
  // Find the lowermost volume in the hierarchy, Pos is inside

  if (m_GeometryScanned == false) {
    mgui<<"Geometry has to be scanned first"<<error;
    merr<<"Geometry has to be scanned first"<<endl;
    return 0;
  }

  return m_WorldVolume->GetVolume(Pos);
}


////////////////////////////////////////////////////////////////////////////////


MDMaterial* MDGeometryQuest::GetMaterial(MVector Pos)
{ 
  // Find the lowermost volume in the hierarchy, Pos is inside

  MDVolume* Volume = m_WorldVolume->GetVolume(Pos);
  if (Volume == 0) return 0;

  return Volume->GetMaterial();
}



////////////////////////////////////////////////////////////////////////////////


bool MDGeometryQuest::Noise(MVector& Pos, double& Energy, double& Time, 
                            MDVolumeSequence& S)
{
  // Noise a hit

  if (m_GeometryScanned == false) {
    mgui<<"Geometry has to be scanned first"<<error;
    merr<<"Geometry has to be scanned first"<<endl;
    return false;
  }

  if (S.GetDetector() == 0) {
    mout<<"Warning: No detector present for hit at "
        <<Pos[0]<<", "<<Pos[1]<<", "<<Pos[2]<<endl;
    mout<<"         Setting energy to zero!"<<endl;
    Energy = 0;
    return false;
  }


  // Now correctly rotate the position resolution into world coordinated:
  for (unsigned int i = 0; i < S.GetNVolumes(); i++) {
    Pos -= S.GetVolumeAt(i)->GetPosition();// translate 
    if (S.GetVolumeAt(i)->IsRotated() == true) {
      Pos = S.GetVolumeAt(i)->GetRotationMatrix() * Pos;    // rotate
    }
  }

  // Now we are in the detector voxel...
  S.GetDetector()->Noise(Pos, Energy, Time, S.GetDeepestVolume());

  // Now correctly rotate the position resolution into world coordinated:
  for (unsigned int i = S.GetNVolumes()-1; i < S.GetNVolumes(); --i) {
    if (S.GetVolumeAt(i)->IsRotated() == true) {
      Pos = S.GetVolumeAt(i)->GetInvRotationMatrix() * Pos;    // rotate
    }
    Pos += S.GetVolumeAt(i)->GetPosition(); // translate 
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MDGeometryQuest::Noise(MVector& Pos, double& Energy, double& Time)
{
  // Noise a hit

  MDVolumeSequence VS = GetVolumeSequence(Pos, true, true);
  return Noise(Pos, Energy, Time, VS);
}
 

////////////////////////////////////////////////////////////////////////////////


void MDGeometryQuest::ApplyPulseShape(double Time, MVector& Pos, double& Energy)
{
  // 

  if (m_GeometryScanned == false) {
    mgui<<"Geometry has to be scanned first"<<error;
    merr<<"Geometry has to be scanned first"<<endl;
    return;
  }

  MVector Temp = Pos;

  //cout<<"Inside: "<<Pos.X()<<"!"<<Pos.Y()<<"!"<<Pos.Z()<<endl;
  if (m_WorldVolume->ApplyPulseShape(Time, Pos, Energy) == false) {
    cout<<"Outside: "<<Temp.X()<<"!"<<Temp.Y()<<"!"<<Temp.Z()<<endl;
    //Fatal("", "");
  }


  Pos = Temp;
}


////////////////////////////////////////////////////////////////////////////////


int MDGeometryQuest::GetNVolumesInHierarchy()
{
  return 0;
}


////////////////////////////////////////////////////////////////////////////////


void MDGeometryQuest::AddHit(MString Name, MVector Position, double Size, int Color)
{
  //

  MDVolume *V = new MDVolume(Name, Name);
  MDShapeSPHE* SPHE = new MDShapeSPHE(); // Shape is deleted with the volume...
  SPHE->Initialize(0.0, Size, 0.0, 180.0, 0.0, 360.0);
  V->SetShape(SPHE);
  V->SetPosition(Position);
  V->SetVisibility(1);
  V->SetColor(Color);
  V->SetMaterial(MDMaterial::GetDummyMaterial());

  if (m_WorldVolume != 0) {
    V->SetMother(m_WorldVolume);
    //m_WorldVolume->AddDaughter(V);
  }

  V->Validate();

  m_Hits.push_back(V);
}


////////////////////////////////////////////////////////////////////////////////


void MDGeometryQuest::RemoveAllHits()
{
  //

  //cout<<"D: "<<m_WorldVolume->GetNDaughters()<<endl;
  unsigned int i;
  for (i = 0; i < m_Hits.size(); i++) {
    m_WorldVolume->RemoveDaughter(m_Hits[i]);
    //cout<<"Removing..."<<endl;
  }
  //cout<<"D: "<<m_WorldVolume->GetNDaughters()<<endl;
  //cout<<"Removing all hits!"<<endl;

  for (unsigned int i = 0; i < m_Hits.size(); ++i) {
    delete m_Hits[i];
  }
  m_Hits.resize(0);
}


////////////////////////////////////////////////////////////////////////////////


void MDGeometryQuest::AddLink(MVector PositionA, MVector PositionB, double Size, int Color)
{
  MDVolume *V = new MDVolume("A Link", "Link");
  MDShapeTUBS* TUBS = new MDShapeTUBS(); // Shape is deleted with the volume...
  TUBS->Initialize(0.0, Size, 0.5*(PositionB - PositionA).Mag(), 0.0, 360.0);
  V->SetShape(TUBS);
  V->SetPosition(0.5*(PositionA+PositionB));
  V->SetRotation(0.0, (PositionB - PositionA).Theta()*c_Deg, (PositionB - PositionA).Phi()*c_Deg);
  V->SetVisibility(1);
  V->SetColor(Color);
  V->SetMaterial(MDMaterial::GetDummyMaterial());

  //cout<<"rotation: "<<(PositionB - PositionA).Theta()*c_Deg<<"!"<<(PositionB - PositionA).Phi()*c_Deg<<endl;

  if (m_WorldVolume != 0) {
    V->SetMother(m_WorldVolume);
  }

  V->Validate();

  m_Links.push_back(V);
}


////////////////////////////////////////////////////////////////////////////////


void MDGeometryQuest::RemoveAllLinks()
{

  unsigned int i;
  for (i = 0; i < m_Links.size(); ++i) {
    m_WorldVolume->RemoveDaughter(m_Links[i]);
    //cout<<"Removing..."<<endl;
  }

  for (unsigned int i = 0; i < m_Links.size(); ++i) {
    delete m_Links[i];
  }
  m_Links.resize(0);
}


////////////////////////////////////////////////////////////////////////////////


bool MDGeometryQuest::DrawGeometry(TCanvas *Canvas)
{
  // Draw the geometry with X3D
  // The geometry must have been loaded previously

  if (m_GeometryScanned == false) {
    mgui<<"Geometry has to be scanned first"<<error;
    merr<<"Geometry has to be scanned first"<<endl;
    return false;
  }

  return MDGeometry::DrawGeometry(Canvas);
}


////////////////////////////////////////////////////////////////////////////////


MDVolume* MDGeometryQuest::GetEmbeddingVolume(MVector Position)
{
  // Return the volume, Position is laying inside:

  return m_WorldVolume->GetVolume(Position);
}


////////////////////////////////////////////////////////////////////////////////


void MDGeometryQuest::AllwaysAssumeTrigger(bool Flag)
{
  // Assume that each event has always triggered

  m_AllwaysAssumeTrigger = Flag;
}


////////////////////////////////////////////////////////////////////////////////


MDDetector* MDGeometryQuest::GetDetector(MVector Position)
{
  // Return the detector which corresponds to position Pos

  MDVolumeSequence S = GetVolumeSequence(Position);
  return S.GetDetector();
}


////////////////////////////////////////////////////////////////////////////////


MString MDGeometryQuest::GetDetectorName(MVector Position)
{
  // Return the name of the detector which corresponds to position Pos

  MDVolumeSequence S = GetVolumeSequence(Position);

  if (S.GetDetector() != 0) {
    MDVolume* V = S.GetDetector()->GetDetectorVolume();
    // Search the volume sequence for a volume which is a clone of this volume:
    for (unsigned int c = 0; c < V->GetNClones(); ++c) {
      for (unsigned int s = 0; s < S.GetNVolumes(); ++s) {
        if (S.GetVolumeAt(s)->GetName() == V->GetCloneAt(c)->GetName()) {
          return S.GetVolumeAt(s)->GetName();
        }
      }
    }
    
    return V->GetName();
  }

  return "";
}


////////////////////////////////////////////////////////////////////////////////


void MDGeometryQuest::ActivateNoising(bool ActivateNoising)
{
  for (unsigned int d = 0; d < GetNDetectors(); ++d) {
    GetDetectorAt(d)->ActivateNoising(ActivateNoising);
  }

  m_ActivateNoising = ActivateNoising;
}

////////////////////////////////////////////////////////////////////////////////


bool MDGeometryQuest::GetActivateNoising()
{
  return m_ActivateNoising;
}


////////////////////////////////////////////////////////////////////////////////


bool MDGeometryQuest::AreInSameVoxel(MVector Pos1, MVector Pos2)
{
  // Check if those positions are in the same detector voxel

  static const MVector Zero(0.0, 0.0, 0.0);

  if (GetDetectorName(Pos1) == GetDetectorName(Pos2) && 
      GetVolume(Pos1)->GetDetector()->AreNear(Pos1, Zero, Pos2, Zero, 0, 0) == true) {
    return true;
  } else {
    return false;
  }
}


////////////////////////////////////////////////////////////////////////////////


void MDGeometryQuest::SetGlobalFailureRate(double FailureRate)
{
  for (unsigned int d = 0; d < GetNDetectors(); ++d) {
    GetDetectorAt(d)->SetFailureRate(FailureRate);
  }
}


////////////////////////////////////////////////////////////////////////////////


void MDGeometryQuest::GetResolutions(MVector Position, double Energy, double Time, 
                                     MVector& PositionResolution, 
                                     double& EnergyResolution,
                                     double& TimeResolution)
{
  MDVolumeSequence S = GetVolumeSequence(Position);
  GetResolutions(Position, Energy, Time, S, 
                 PositionResolution, EnergyResolution, TimeResolution);
}


////////////////////////////////////////////////////////////////////////////////


void MDGeometryQuest::GetResolutions(MVector Position, double Energy, double Time, 
                                     MDVolumeSequence& S,
                                     MVector& PositionResolution, 
                                     double& EnergyResolution, 
                                     double& TimeResolution)
{
  if (S.GetDetector() == 0) {
    merr<<"Hit ("<<Position[0]<<", "<<Position[1]<<", "<<Position[2]
        <<") was not in a sensitive detector!!!"<<endl;
    EnergyResolution = 0;
    TimeResolution = 0;
    PositionResolution = MVector(0.0, 0.0, 0.0);
  } else {
    EnergyResolution = S.GetDetector()->GetEnergyResolution(Energy, Position);
    TimeResolution = S.GetDetector()->GetTimeResolution(Energy);
    PositionResolution = S.GetDetector()->GetPositionResolution(Position, Energy);
    // Now correctly rotate the position resolution into world coordinated:
    for (unsigned int i = S.GetNVolumes() - 1; i < S.GetNVolumes(); i--) {
      if (S.GetVolumeAt(i)->IsRotated() == true) {
        PositionResolution = S.GetVolumeAt(i)->GetInvRotationMatrix() * PositionResolution;    // rotate
      }
    }
    PositionResolution = PositionResolution.Abs();
  }
}


////////////////////////////////////////////////////////////////////////////////


double MDGeometryQuest::GetAbsorptionProbability(MVector Start, MVector Stop, 
                                                 double Energy)
{
  return GetAbsorptionProbability(Start, Stop, Energy, 0);
}


////////////////////////////////////////////////////////////////////////////////


double MDGeometryQuest::GetPhotoAbsorptionProbability(MVector Start, MVector Stop, 
                                                      double Energy)
{
  return GetAbsorptionProbability(Start, Stop, Energy, 1);
}


////////////////////////////////////////////////////////////////////////////////


double MDGeometryQuest::GetComptonAbsorptionProbability(MVector Start, MVector Stop, 
                                                        double Energy)
{
  return GetAbsorptionProbability(Start, Stop, Energy, 2);
}


////////////////////////////////////////////////////////////////////////////////


double MDGeometryQuest::GetPairAbsorptionProbability(MVector Start, MVector Stop, 
                                                      double Energy)
{
  return GetAbsorptionProbability(Start, Stop, Energy, 3);
}


////////////////////////////////////////////////////////////////////////////////


double MDGeometryQuest::GetRayleighAbsorptionProbability(MVector Start, MVector Stop, 
                                                 double Energy)
{
  return GetAbsorptionProbability(Start, Stop, Energy, 4);
}


////////////////////////////////////////////////////////////////////////////////


map<MDMaterial*, double> MDGeometryQuest::GetPathLengths(MVector Start, MVector Stop)
{
  map<MDMaterial*, double> Lengths;

  m_WorldVolume->GetAbsorptionLengths(Lengths, Start, Stop);

  return Lengths;
}

////////////////////////////////////////////////////////////////////////////////


double MDGeometryQuest::GetAbsorptionProbability(MVector Start, MVector Stop, 
                                                 double Energy, int Mode)
{
  // Return the specified absorption probability

  // Remark: The density has already been incorporated into the coefficient!

  //cout<<endl;
  //cout<<"Calculating absorptions: "<<Start<<" -> "<<Stop<<" with "<<Energy<<endl;

  map<MDMaterial*, double> Lengths;
  map<MDMaterial*, double>::iterator LengthsIter;

  m_WorldVolume->GetAbsorptionLengths(Lengths, Start, Stop);

  double AbsProp = 1.0;
  for (LengthsIter = (Lengths.begin()); 
       LengthsIter != Lengths.end(); LengthsIter++) {
      //cout<<" Mat: "<<(*LengthsIter).first->GetName()
      //    <<" Length: "<<(*LengthsIter).second
      //    <<" Energy: "<<Energy
      //    <<" Coe: "<<(*LengthsIter).first->GetAbsorptionCoefficient(Energy)
      //    <<" Res: "<<exp(-(*LengthsIter).first->GetAbsorptionCoefficient(Energy)*(*LengthsIter).second)<<endl;

    if ((*LengthsIter).second < 0) {
      merr<<"Error: Ignoring absorption length < 0: " 
          <<" Mat: "<<(*LengthsIter).first->GetName()
          <<" Length: "<<(*LengthsIter).second
          <<" Energy: "<<Energy
          <<" Coe: "<<(*LengthsIter).first->GetPhotoAbsorptionCoefficient(Energy)
          <<" Res: "<<exp(-(*LengthsIter).first->GetPhotoAbsorptionCoefficient(Energy)*(*LengthsIter).second)<<endl;
      continue;
    }
    if (Mode == 1) {
      AbsProp *= exp(-(*LengthsIter).first->GetPhotoAbsorptionCoefficient(Energy)*(*LengthsIter).second);
      //cout<<" Mat: "<<(*LengthsIter).first->GetName()
      //    <<" Length: "<<(*LengthsIter).second
      //    <<" Energy: "<<Energy
      //    <<" Coe: "<<(*LengthsIter).first->GetPhotoAbsorptionCoefficient(Energy)
      //    <<" Res: "<<exp(-(*LengthsIter).first->GetPhotoAbsorptionCoefficient(Energy)*(*LengthsIter).second)<<endl;
    } else if (Mode == 2) {
      AbsProp *= exp(-(*LengthsIter).first->GetComptonAbsorptionCoefficient(Energy)*(*LengthsIter).second);
       //cout<<" Mat: "<<(*LengthsIter).first->GetName()
       //    <<" Length: "<<(*LengthsIter).second
       //    <<" Energy: "<<Energy
       //    <<" Coe: "<<(*LengthsIter).first->GetComptonAbsorptionCoefficient(Energy)
       //    <<" Res: "<<exp(-(*LengthsIter).first->GetComptonAbsorptionCoefficient(Energy)*(*LengthsIter).second)<<endl;
    } else if (Mode == 3) {
      AbsProp *= exp(-(*LengthsIter).first->GetPairAbsorptionCoefficient(Energy)*(*LengthsIter).second);
    } else if (Mode == 4) {
      AbsProp *= exp(-(*LengthsIter).first->GetRayleighAbsorptionCoefficient(Energy)*(*LengthsIter).second);
    } else {
      AbsProp *= exp(-(*LengthsIter).first->GetAbsorptionCoefficient(Energy)*(*LengthsIter).second);
    }
  }

  if (AbsProp < 0.0) {
    merr<<"Absorption probability lower than 0! --> Setting to 0!"<<endl;
    AbsProp = 0;
  } 
  if (AbsProp > 1.0) {
    merr<<"Absorption probability greater than 1! --> Setting to 1!"<<endl;
    AbsProp = 1;
  }
  
  return 1.0 - AbsProp;
}


////////////////////////////////////////////////////////////////////////////////


bool MDGeometryQuest::GetComptonIntersection(const MComptonEvent& Compton)
{
  // A Compton cone is parametrized in the following way:
  // x^2 + y^2 = e*z^2 with e := tan(phi)^2

  // The coordinate transformation from a Compton-cone whose axis is the 
  // z-axis to the real cone:

  TMatrix M(3,3);
  M(0,0) = -Compton.Dg().Z();
  M(1,0) =  Compton.Dg().X();
  M(2,0) =  Compton.Dg().Y();

  M(0,1) =  Compton.Dg().Y(); 
  M(1,1) = -Compton.Dg().Z();
  M(2,1) =  Compton.Dg().X();

  M(0,2) = -Compton.Dg().X();
  M(1,2) = -Compton.Dg().Y();
  M(2,2) = -Compton.Dg().Z();
  if (M.Determinant() == 0) {
    merr<<"Unhandled event because determinant of rotation matrix is zero"<<endl;
    return false;
  }
  M = M.Invert();
  
  MVector Position;
  
  double z;
  double r;
  if (Compton.Phi() < c_Pi/2) {
    z = ((MDShapeBRIK*) (m_WorldVolume->GetShape()))->GetSizeX(); // Bad!!
    r = z*tan(Compton.Phi());
  } else {
    z = - ((MDShapeBRIK*) (m_WorldVolume->GetShape()))->GetSizeX(); // Bad!!
    r = - z*tan(Compton.Phi());
  }

  const int Samples = 360; // Bad!!!
  double Angle;

  map<MDMaterial*, double> Lengths;
  map<MDMaterial*, double>::iterator LengthsIter;

  for (int s = 0; s < Samples; ++s) {
    Angle = s*360.0/Samples*c_Rad;
    Position[0] = r*cos(Angle);
    Position[1] = r*sin(Angle);
    Position[2] = z;

    Position = M*Position;
    Position += Compton.C1();
    
    for (unsigned int d = 0; d < m_WorldVolume->GetNDaughters(); ++d) {
      m_WorldVolume->GetDaughterAt(d)->GetAbsorptionLengths(Lengths, Compton.C1(), Position);
      for (LengthsIter = (Lengths.begin()); 
           LengthsIter != Lengths.end(); LengthsIter++) {
        if ((*LengthsIter).second >= 0) {
          mdebug<<"Found intersection at angle "<<Angle*c_Deg<<" with "<<MVector(0,0,1).Angle(Position)*c_Deg<<endl;
          return true;
        }
      }    
    }
  }
  mdebug<<"Did not find any intersections!"<<endl;
          
  return false;
}


// MDGeometryQuest.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
