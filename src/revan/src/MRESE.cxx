/*
 * MRESE.cxx
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
// MRESE.cxx
//
// RESE = Raw Event Sub Element
// Abstract base class for all raw event sub elements like tracks, hits,
// clusters, etc.
//
// This class has several tasks:
//
// (1) Provide the basic data interface for the derived classes
//     (ID, type, position, energy, detectortype)
// (2) Provide the linked list functionality for the event and track structure
//     (two consecutive hits (or other RESEs) can be linked with each other)
// (3) Provide basic the container functionality: A RESE can be a container
//     for other RESEs, e.g. the RESE cluster is a container for hits, a track 
//     is a container for clusters and hits, etc.
//
////////////////////////////////////////////////////////////////////////////////


// Include the header
#include "MRESE.h"

// Standard libs:
#include <limits>
#include <iostream>
using namespace std;

// ROOT libs
#include <TObjArray.h>
#include <MString.h>

// MEGAlib:
#include "MAssert.h"
#include "MStreams.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MRESE)
#endif


////////////////////////////////////////////////////////////////////////////////


int MRESE::m_IDCounter = 0;

const int MRESE::c_Unknown         = 0;
const int MRESE::c_Hit             = 1;
const int MRESE::c_Track           = 2;
const int MRESE::c_Cluster         = 3;
const int MRESE::c_Bremsstrahlung  = 4;
const int MRESE::c_Event           = 5;
const int MRESE::c_StripHit        = 6;


////////////////////////////////////////////////////////////////////////////////


void MRESE::ResetIDCounter() 
{
  m_IDCounter = 0;
}


////////////////////////////////////////////////////////////////////////////////


MRESE::MRESE()
{
  // default constructor

  if (m_IDCounter == numeric_limits<int>::max()) {
    ResetIDCounter();
  }
  m_ID = ++m_IDCounter;

  m_VolumeSequence = nullptr;
  m_RESEList = nullptr;
  m_LinkList = nullptr;

  m_SubElementType = MRESE::c_Unknown;

  Reset();
}


////////////////////////////////////////////////////////////////////////////////


MRESE::MRESE(MVector Position, double Energy, double Time, int Detector, 
             MVector PositionRes, double EnergyRes, double TimeRes)
{
  // x, y, z:   Position of the RESE 
  // Energy:    Energy of this RESE
  // Detector:  Number of the detector (see SetDetector(...))

  if (m_IDCounter == numeric_limits<int>::max()) {
    ResetIDCounter();
  }
  m_ID = ++m_IDCounter;

  m_VolumeSequence = nullptr;
  m_RESEList = nullptr;
  m_LinkList = nullptr;

  m_SubElementType = MRESE::c_Unknown;

  Reset();

  SetPosition(Position);
  SetEnergy(Energy);
  SetTime(Time);
  SetDetector(Detector);
  SetPositionResolution(PositionRes);
  SetEnergyResolution(EnergyRes);
  SetTimeResolution(TimeRes);
}


////////////////////////////////////////////////////////////////////////////////


MRESE::MRESE(MRESE* RESE)
{
  // Special copy-constructor:
  // The links of the RESE cannot be resolved, so the m_LinkList is unchanged, 
  // and must be updated in a derived class.

  // We want to keep the ID of this RESE!

  m_SubElementType = RESE->m_SubElementType;   
  m_Position = RESE->m_Position;     
  m_Energy = RESE->m_Energy;       
  m_Time = RESE->m_Time;       
  m_AdditionalEnergy = RESE->m_AdditionalEnergy;       
  m_PositionResolution = RESE->m_PositionResolution;     
  m_EnergyResolution = RESE->m_EnergyResolution;       
  m_TimeResolution = RESE->m_TimeResolution;       
  m_Detector = RESE->m_Detector;        
  m_ID = RESE->m_ID;
  m_IsValid = RESE->m_IsValid;
  m_AllowOverwrite = RESE->m_AllowOverwrite;
  m_VolumeSequence = new MDVolumeSequence(*(RESE->m_VolumeSequence));

  m_RESEList = RESE->GetRESEList()->Duplicate();
  m_LinkList = new MRESEList();
  
  m_NoiseFlags = RESE->m_NoiseFlags;
}


////////////////////////////////////////////////////////////////////////////////


MRESE::~MRESE()
{
  // The destructor does not delete the included RESEs

  //cout<<"Deleting RESE "<<m_ID<<endl;

  //m_RESEList->DeleteAll();

  delete m_VolumeSequence;
  delete m_RESEList;
  delete m_LinkList;
}


////////////////////////////////////////////////////////////////////////////////


void MRESE::Reset()
{
  // Common initializations

  m_Energy = 0;
  m_AdditionalEnergy = 0;
  m_Time = 0;
  m_Position = MVector(0.0, 0.0, 0.0);

  // Do not change these values from zero --- zero means ideal!
  m_EnergyResolution = 0;
  m_PositionResolution = MVector(0.0, 0.0, 0.0);
  m_TimeResolution = 0;

  m_Detector = MDDetector::c_NoDetectorType;

  delete m_VolumeSequence;
  m_VolumeSequence = new MDVolumeSequence();
  
  delete m_LinkList;
  m_LinkList = new MRESEList();
  
  if (m_RESEList != nullptr) {
    m_RESEList->DeleteAll();
    delete m_RESEList;
  }
  m_RESEList = new MRESEList();

  m_IsValid = true;
  m_AllowOverwrite = true;
  
  m_NoiseFlags = "";
}


////////////////////////////////////////////////////////////////////////////////


bool MRESE::operator==(MRESE& RESE)
{
  // Return true if the two hits are identical

  massert(this != 0);

  if (m_SubElementType == RESE.m_SubElementType &&
      m_Position == RESE.m_Position &&
      m_Energy == RESE.m_Energy &&
      m_Time == RESE.m_Time &&
      m_AdditionalEnergy == RESE.m_AdditionalEnergy &&
      m_Detector == RESE.m_Detector) {
    return true;
  } 

  return false;
}


////////////////////////////////////////////////////////////////////////////////


int MRESE::GetType() 
{ 
  // Return the type of this RESE (defined in RESE_Types):
  //
  // MRESE::c_Unknown = 0,
  // MRESE::c_Hit = 1,
  // MRESE::c_Track = 2,
  // MRESE::c_Cluster = 3,
  // MRESE::c_Bremsstrahlung = 4

  massert(this != 0);

  return m_SubElementType; 
}


////////////////////////////////////////////////////////////////////////////////


MVector MRESE::GetPosition() 
{
  // Return the position of this RESE

  massert(this != 0);

  return m_Position; 
}


////////////////////////////////////////////////////////////////////////////////


double MRESE::GetTime() 
{
  // Return the time of this RESE

  massert(this != 0);

  return m_Time; 
}


////////////////////////////////////////////////////////////////////////////////


double MRESE::GetEnergy() 
{ 
  // Return the total energy of this RESE

  massert(this != 0);

  return m_Energy + m_AdditionalEnergy; 
}


////////////////////////////////////////////////////////////////////////////////


MVector MRESE::GetPositionResolution() 
{
  // Return the position of this RESE

  massert(this != 0);

  return m_PositionResolution; 
}


////////////////////////////////////////////////////////////////////////////////


double MRESE::GetTimeResolution() 
{ 
  // Return the total energy of this RESE

  massert(this != 0);

  return m_TimeResolution; 
}


////////////////////////////////////////////////////////////////////////////////


double MRESE::GetEnergyResolution() 
{ 
  // Return the total energy of this RESE

  massert(this != 0);

  return m_EnergyResolution; 
}


////////////////////////////////////////////////////////////////////////////////


int MRESE::GetDetector() 
{ 
  // Return the Detector this RESE lays in:
  //
  // RESE_DNone = -1
  // RESE_DMultiple = 0
  // MDDetector::c_Strip2D = 1
  // MDDetector::c_Calorimeter = 2
  // MDDetector::c_Strip3D = 3

  massert(this != 0);

  return m_Detector; 
}


////////////////////////////////////////////////////////////////////////////////


void MRESE::SetPosition(MVector Position)
{
  // Set the position of this RESE

  m_Position = Position;
}


////////////////////////////////////////////////////////////////////////////////


void MRESE::SetPosition(double x, double y, double z)
{
  // Set the position of this RESE

  massert(this != 0);

  m_Position = MVector(x, y, z);
}


////////////////////////////////////////////////////////////////////////////////


void MRESE::SetPositionResolution(MVector Position)
{
  // Set the position resolution of this RESE

  m_PositionResolution = Position;
}


////////////////////////////////////////////////////////////////////////////////


void MRESE::SetTimeResolution(double Time)
{
  // Set the position resolution of this RESE

  m_TimeResolution = Time;
}


////////////////////////////////////////////////////////////////////////////////


void MRESE::SetTime(double Time)
{
  // Set the position of this RESE

  massert(this != 0);

  m_Time = Time;
}


////////////////////////////////////////////////////////////////////////////////


void MRESE::SetEnergy(double Energy)
{
  // Set the total energy of this RESE

  massert(this != 0);

  m_Energy = Energy;
}
 

////////////////////////////////////////////////////////////////////////////////


void MRESE::SetAdditionalEnergy(double Energy)
{
  // Set some addiotional energy
  
  massert(this != 0);

  m_AdditionalEnergy = Energy;
}


////////////////////////////////////////////////////////////////////////////////


void MRESE::SetEnergyResolution(double Energy)
{
  // Set the total energy resolution of this RESE

  massert(this != 0);

  m_EnergyResolution = Energy;
}


////////////////////////////////////////////////////////////////////////////////


void MRESE::SetDetector(int Detector)
{
  // Set the Detector this RESE lays in, e.g.
  //
  // RESE_DNone = -1
  // RESE_DMultiple = 0
  // MDDetector::c_Strip2D = 1
  // MDDetector::c_Calorimeter = 2
  // MDDetector::c_Strip3D = 3

  massert(this != 0);

  m_Detector = Detector;
}


////////////////////////////////////////////////////////////////////////////////


void MRESE::SetVolumeSequence(MDVolumeSequence* VS)
{
  // Set the volume sequence
  
  delete m_VolumeSequence;
  m_VolumeSequence = VS;
}


////////////////////////////////////////////////////////////////////////////////


MDVolumeSequence* MRESE::GetVolumeSequence()
{
  // Return the volume sequence
  
  return m_VolumeSequence;
}


////////////////////////////////////////////////////////////////////////////////


void MRESE::SetGridPoint(const MDGridPoint& GP)
{
  // Set the grid point
  
  m_GridPoint = GP;
}


////////////////////////////////////////////////////////////////////////////////


MDGridPoint MRESE::GetGridPoint() const
{
  // Return the grid point
  
  return m_GridPoint;
}


////////////////////////////////////////////////////////////////////////////////


int MRESE::GetID() 
{ 
  // Return the unique ID of this RESE

  massert(this != 0);

  return m_ID; 
}


////////////////////////////////////////////////////////////////////////////////


MRESEList* MRESE::GetRESEList()
{
  // Get the list of all RESEs

  massert(this != 0);

  return m_RESEList;
}


////////////////////////////////////////////////////////////////////////////////


MRESEList* MRESE::GetLinkList()
{
  // Get the list of all links
  
  massert(this != 0);

  return m_LinkList;
}


////////////////////////////////////////////////////////////////////////////////


void MRESE::AddLink(MRESE* RESE)
{
  // Add a link to the RESE

  massert(this != 0);

  // Since this is a double-linked list, only two links are allowed:
  if (GetNLinks() > 1) {
    merr<<"This RESE already has two links:"<<endl;
    for (int i = 0; i < GetNLinks(); ++i) {
      mout<<"Old link "<<i<<": "<<GetLinkAt(i)->GetID()<<endl;
    }
    mout<<"Requested new link: "<<RESE->GetID()<<endl;
    massert(GetNLinks() <= 1);
  }

  // It is not allowed to exist already:
  if (GetNLinks() == 1) {
    if (GetLinkAt(0)->GetID() == RESE->GetID()) {
      merr<<"Link with ID "<<RESE->GetID()<<" already exists. - Refusing to add this link!"<<endl;
      cout<<this->ToString()<<endl;
      cout<<RESE->ToString()<<endl;
      massert(GetLinkAt(0)->GetID() != RESE->GetID());
    }
  }

  m_LinkList->AddRESE(RESE);
}


////////////////////////////////////////////////////////////////////////////////


void MRESE::RemoveLink(MRESE* RESE)
{
  // Remove a link from this RESE and compress the list

  massert(this != 0);

  m_LinkList->RemoveRESE(RESE);
  m_LinkList->CompressRESEs();
}


////////////////////////////////////////////////////////////////////////////////


bool MRESE::IsLink(MRESE* RESE)
{
  // Test if RESE is a link of *this* RESE

  massert(this != 0);

  return m_LinkList->ContainsRESE(RESE);
}


////////////////////////////////////////////////////////////////////////////////


bool MRESE::HasLinks()
{
  // Return true, if this RESE is linked with other RESEs

  massert(this != 0);

  if (m_LinkList->GetNRESEs() <= 0) {
    return false;
  } else {
    return true;
  }
}


////////////////////////////////////////////////////////////////////////////////


MRESE* MRESE::GetOtherLink(MRESE* RESE)
{
  // Return the other link as RESE
  // If there is no other link return 0
  // If RESE == 0, return the first (and hopefully the only) link

  massert(this != 0);

  if (GetNLinks() == 0) {
    merr<<"This RESE has no links!"<<endl;
    massert(0);
    return 0;
  }

  if (GetNLinks() > 2) {
    merr<<"This RESE has more than two links:"<<endl<<ToString()<<endl;
    massert(0);
    return 0;
  }

  if (RESE == 0) {
    return GetLinkAt(0);
  }

  if (GetNLinks() == 1) {
    if (GetLinkAt(0) == RESE) {
      return 0;
    } else {
      merr<<"This case should never happen..."<<endl;
      massert(0);
      return 0;
    }
  }

  if (GetLinkAt(0) == RESE) {
    return GetLinkAt(1);
  } else {
    return GetLinkAt(0);
  } 
}


////////////////////////////////////////////////////////////////////////////////


MRESE* MRESE::GetLinkAt(int i)
{
  // Return the i-th link

  massert(this != 0);

  return m_LinkList->GetRESEAt(i);
}


////////////////////////////////////////////////////////////////////////////////


void MRESE::RemoveAllLinks()
{
  // Remove all links in this RESE
  
  massert(this != 0);

  while (GetNLinks() > 0) {
    RemoveLink(GetLinkAt(0));
  }
}


////////////////////////////////////////////////////////////////////////////////


int MRESE::GetNLinks()
{
  // Return the number of stored links

  massert(this != 0);

  return m_LinkList->GetNRESEs();
} 


////////////////////////////////////////////////////////////////////////////////


void MRESE::AddRESE(MRESE* RESE)
{
  // Add a RESE to the list

  massert(this != 0);

  if (RESE != 0) {
    if (GetNRESEs() == 0) {
      delete m_VolumeSequence;
      m_VolumeSequence = new MDVolumeSequence(*(RESE->GetVolumeSequence()));
    } else {
      m_VolumeSequence->Join(*(RESE->GetVolumeSequence()));
    }
    m_RESEList->AddRESE(RESE);
  }

  RecalculateResolutions();
}


////////////////////////////////////////////////////////////////////////////////


void MRESE::AddRESEFirst(MRESE *RESE)
{
  // Add a RESE to the first element of the list:

  massert(this != 0);

  m_RESEList->AddRESEFirst(RESE);

  RecalculateResolutions();
}


////////////////////////////////////////////////////////////////////////////////


int MRESE::GetNRESEs()
{
  // Return the number of stored RESEs

  massert(this != 0);

  return m_RESEList->GetNRESEs();
}


////////////////////////////////////////////////////////////////////////////////


MRESE* MRESE::GetRESEAt(int i)
{
  // Return the i-th RESE

  massert(this != 0);

  return m_RESEList->GetRESEAt(i);
}


////////////////////////////////////////////////////////////////////////////////


MRESE* MRESE::GetRESE(int ID)
{
  // Return the RESE with the ID id

  massert(this != 0);

  return m_RESEList->GetRESE(ID);
}


////////////////////////////////////////////////////////////////////////////////


MRESE* MRESE::GetRESEContainingRESE(int ID) 
{
  // Get RESE by ID - Return toplevel RESE which is or contains a RESE with this ID

  massert(this != 0);

  return m_RESEList->GetRESEContainingRESE(ID);
}



////////////////////////////////////////////////////////////////////////////////


bool MRESE::ContainsRESE(MRESE* RESE)
{
  // Return true, RESE is stored in *this* RESE

  massert(this != 0);

  return m_RESEList->ContainsRESE(RESE);
}


////////////////////////////////////////////////////////////////////////////////


bool MRESE::ContainsRESE(int ID)
{
  // Return true, RESE is stored in *this* RESE

  massert(this != 0);

  return m_RESEList->ContainsRESE(ID);
}


////////////////////////////////////////////////////////////////////////////////


MRESE* MRESE::RemoveRESE(MRESE* RESE)
{
  // Remove a RESE from *this* RESE. Afterwards we have an empty slot!

  massert(this != 0);

  m_RESEList->RemoveRESE(RESE);
  RecalculateResolutions();

  return RESE;
}


////////////////////////////////////////////////////////////////////////////////


MRESE* MRESE::RemoveRESEAt(int i)
{
  // Return the removed RESE.  Afterwards we have an empty slot!

  massert(this != 0);

  MRESE* RESE = m_RESEList->GetRESEAt(i);
  m_RESEList->RemoveRESE(RESE);
  RecalculateResolutions();

  return RESE;
}


////////////////////////////////////////////////////////////////////////////////


MRESE* MRESE::RemoveRESE(int ID)
{
  // Return the removed RESE.  Afterwards we have an empty slot!

  massert(this != 0);

  MRESE* RESE = m_RESEList->GetRESE(ID);
  m_RESEList->RemoveRESE(RESE);
  RecalculateResolutions();

  return RESE;
}


////////////////////////////////////////////////////////////////////////////////


MRESE* MRESE::RemoveRESEAndCompress(MRESE* RESE)
{
  // Remove a RESE from *this* RESE. There is no empty slot left behind

  massert(this != 0);

  m_RESEList->RemoveRESE(RESE);
  RecalculateResolutions();
  CompressRESEs();

  return RESE;
}


////////////////////////////////////////////////////////////////////////////////


MRESE* MRESE::RemoveRESEAtAndCompress(int i)
{
  // Return the removed RESE. There is no empty slot left behind

  massert(this != 0);

  MRESE* RESE = m_RESEList->GetRESEAt(i);
  m_RESEList->RemoveRESE(RESE);
  RecalculateResolutions();
  CompressRESEs();

  return RESE;
}


////////////////////////////////////////////////////////////////////////////////


MRESE* MRESE::RemoveRESEAndCompress(int ID)
{
  // Return the removed RESE. There is no empty slot left behind

  massert(this != 0);

  MRESE* RESE = m_RESEList->GetRESE(ID);
  m_RESEList->RemoveRESE(RESE);
  RecalculateResolutions();
  CompressRESEs();

  return RESE;
}


////////////////////////////////////////////////////////////////////////////////


void MRESE::RemoveAll()
{
  // Delete the links and RESEs of this event
  
  m_RESEList->RemoveAllRESEs();
}


////////////////////////////////////////////////////////////////////////////////


void MRESE::RemoveAllAndCompress()
{
  // Delete the links and RESEs of this event
  
  m_RESEList->RemoveAllRESEs();
  m_RESEList->Compress();
}


////////////////////////////////////////////////////////////////////////////////


void MRESE::DeleteRESE(MRESE* RESE)
{
  // Delete RESE from *this* RESE. Afterwards we have an empty slot!

  massert(this != 0);

  m_RESEList->RemoveRESE(RESE);
  RecalculateResolutions();

  RESE->DeleteAll();
  delete RESE;
  RESE = nullptr;
}


////////////////////////////////////////////////////////////////////////////////


void MRESE::DeleteRESEAt(int i)
{
  // Delete the RESE at position i.  Afterwards we have an empty slot!

  massert(this != 0);

  MRESE* RESE = m_RESEList->GetRESEAt(i);
  m_RESEList->RemoveRESE(RESE);
  RecalculateResolutions();

  RESE->DeleteAll();
  delete RESE;
}


////////////////////////////////////////////////////////////////////////////////


void MRESE::DeleteRESE(int ID)
{
  // Delete the RESE at position i.  Afterwards we have an empty slot!

  massert(this != 0);

  MRESE* RESE = m_RESEList->GetRESE(ID);
  m_RESEList->RemoveRESE(RESE);
  RecalculateResolutions();

  RESE->DeleteAll();
  delete RESE;
}


////////////////////////////////////////////////////////////////////////////////


void MRESE::DeleteRESEAndCompress(MRESE* RESE)
{
  // Delete RESE from *this* RESE. There is no empty slot left behind

  massert(this != 0);

  m_RESEList->RemoveRESE(RESE);
  RecalculateResolutions();
  CompressRESEs();

  RESE->DeleteAll();
  delete RESE;
  RESE = nullptr;
}


////////////////////////////////////////////////////////////////////////////////


void MRESE::DeleteRESEAtAndCompress(int i)
{
  // Delete the RESE at position i. There is no empty slot left behind

  massert(this != 0);

  MRESE* RESE = m_RESEList->GetRESEAt(i);
  m_RESEList->RemoveRESE(RESE);
  RecalculateResolutions();
  CompressRESEs();

  RESE->DeleteAll();
  delete RESE;
}


////////////////////////////////////////////////////////////////////////////////


void MRESE::DeleteRESEAndCompress(int ID)
{
  // Delete the RESE at position i. There is no empty slot left behind

  massert(this != 0);

  MRESE* RESE = m_RESEList->GetRESE(ID);
  m_RESEList->RemoveRESE(RESE);
  RecalculateResolutions();
  CompressRESEs();

  RESE->DeleteAll();
  delete RESE;
}


////////////////////////////////////////////////////////////////////////////////


void MRESE::DeleteAll()
{
  // Delete the links and RESEs of this event
  
  m_RESEList->DeleteAll();
}


////////////////////////////////////////////////////////////////////////////////


void MRESE::CompressRESEs()
{
  // Remove empty slots, which have been produced be RemoveRESE() 

  massert(this != 0);

  m_RESEList->CompressRESEs();
}


////////////////////////////////////////////////////////////////////////////////


void MRESE::Shuffle()
{
  //! Shuffle the RESEs around...
  
  unsigned int size = GetNRESEs();
  for (unsigned int i = 0; i < 2*size; ++i) {
    unsigned int From = gRandom->Integer(size);
    unsigned int To = gRandom->Integer(size);
    MRESE* Temp = m_RESEList->GetRESEAt(To);
    m_RESEList->SetRESEAt(To, m_RESEList->GetRESEAt(From));
    m_RESEList->SetRESEAt(From, Temp);
  }
}


////////////////////////////////////////////////////////////////////////////////


double MRESE::ComputeMinDistance(MRESE *RESE)
{
  // Compute the minimum distance between two RESE
  // The functionality must be implemented in the derived classes!

  massert(this != 0);

  merr<<"This functionality must be implemented in the derived classes!"<<endl;

  return numeric_limits<double>::max();
}


////////////////////////////////////////////////////////////////////////////////


MVector MRESE::ComputeMinDistanceVector(MRESE *RESE)
{
  // Compute the minimum distance between two RESE
  // The functionality must be implemented in the derived classes!

  massert(this != 0);

  merr<<"This functionality must be implemented in the derived classes!"<<endl;

  return MVector(numeric_limits<double>::max(), numeric_limits<double>::max(), 
                  numeric_limits<double>::max());
}


////////////////////////////////////////////////////////////////////////////////


bool MRESE::AreAdjacent(MRESE* R, double Sigma, int Level)
{
  // Check if the RESE are in neighboring pixels
  // The functionality must be implemented in the derived classes!

  massert(this != 0);

  merr<<"This functionality must be implemented in the derived classes!"<<endl;

  return false;
}


////////////////////////////////////////////////////////////////////////////////


MString MRESE::ToString(bool WithLink, int Level)
{
  // Returns a MString containing the relevant data of this object
  // Should be overwritten in derived classes

  massert(this != 0);

  return MString("Unkown RESE");
}


////////////////////////////////////////////////////////////////////////////////


MString MRESE::ToEvtaString(const int Precision, const int Version) 
{
  //! Convert to a string in the evta file

  ostringstream out;

  // Since we don't know anything what type we have here, we just loop over its daughters...
  for (int i = 0; i < GetNRESEs(); ++i) {
    out<<GetRESEAt(i)->ToEvtaString(Precision, Version);
  }
  
  return out.str().c_str();
}


////////////////////////////////////////////////////////////////////////////////


MRESE* MRESE::Duplicate()
{
  // Duplicate this RESE

  // make some tests, if this RESE can be duplicated ...

  massert(this != 0);

  return new MRESE(this);
}


////////////////////////////////////////////////////////////////////////////////


bool MRESE::IsValid()
{
  massert(this != 0);

  return m_IsValid;
}


////////////////////////////////////////////////////////////////////////////////


void MRESE::RecalculateResolutions()
{
  if (m_AllowOverwrite == false) return;

  m_PositionResolution = MVector(0.0, 0.0, 0.0);
  m_EnergyResolution = 0;
  for (int r = 0; r < GetNRESEs(); ++r) {
    if (GetRESEAt(r) == 0) continue; // Someday I have to remove the need to compress everything...
    m_PositionResolution.m_X += 
      GetRESEAt(r)->GetPositionResolution()[0]*GetRESEAt(r)->GetPositionResolution()[0];
    m_PositionResolution.m_Y += 
      GetRESEAt(r)->GetPositionResolution()[1]*GetRESEAt(r)->GetPositionResolution()[1];
    m_PositionResolution.m_Z += 
      GetRESEAt(r)->GetPositionResolution()[2]*GetRESEAt(r)->GetPositionResolution()[2];
    m_EnergyResolution += 
      GetRESEAt(r)->GetEnergyResolution()*GetRESEAt(r)->GetEnergyResolution();
  }
  m_PositionResolution[0] = sqrt(m_PositionResolution[0]);
  m_PositionResolution[1] = sqrt(m_PositionResolution[1]);
  m_PositionResolution[2] = sqrt(m_PositionResolution[2]);
  m_EnergyResolution = sqrt(m_EnergyResolution);

  // The time resolution is the time resolution of the oldest hit:
  double MinTime = numeric_limits<double>::max();
  for (int r = 0; r < GetNRESEs(); ++r) {
    if (GetRESEAt(r) == 0) continue; // Someday I have to remove the need to compress everything...
    if (GetRESEAt(r)->GetTime() < MinTime) {
      MinTime = GetRESEAt(r)->GetTime();
      m_TimeResolution = GetRESEAt(r)->GetTimeResolution();
    }
  }
  

  

//   mimp<<"What should I use for the time resolution of the cluster???"<<show;
//   double MinTime = numeric_limits<double>::max();
//   double MaxTime = -numeric_limits<double>::max();
//   for (int r = 0; r < GetNRESEs(); ++r) {
//     if (GetRESEAt(r) == 0) continue; // Someday I have to remove the need to compress everything...
//     if (GetRESEAt(r)->GetTime() > MaxTime) {
//       MaxTime = GetRESEAt(r)->GetTime();
//     }
//     if (GetRESEAt(r)->GetTime() < MaxTime) {
//       MinTime = GetRESEAt(r)->GetTime();      
//     }
//   }
//   m_TimeResolution = MaxTime - MinTime;
//   for (int r = 0; r < GetNRESEs(); ++r) {
//     if (GetRESEAt(r) == 0) continue; // Someday I have to remove the need to compress everything...
//     if (GetRESEAt(r)->GetTimeResolution() > m_TimeResolution) {
//       m_TimeResolution = GetRESEAt(r)->GetTimeResolution();
//     }
//   }
}


////////////////////////////////////////////////////////////////////////////////


void MRESE::ReID() 
{
  //! Redo all IDs to make sure they are unique

  m_IDCounter++;
  m_ID = m_IDCounter;
  for (int r = 0; r < GetNRESEs(); ++r) {
    GetRESEAt(r)->ReID();
  }
}


////////////////////////////////////////////////////////////////////////////////


MPhysicalEventHit MRESE::CreatePhysicalEventHit()
{
  //! Convert to a MPhysicalEventHit  
  
  MPhysicalEventHit H;
  
  H.Set(m_Position, m_PositionResolution, m_Energy, m_EnergyResolution, m_Time, m_TimeResolution);
  
  return H;
}


// MRESE.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
