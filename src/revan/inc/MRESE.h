/*
 * MRESE.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MRESE__
#define __MRESE__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"

// MEGAlib libs:
#include "MVector.h"
#include "MRESEList.h"
#include "MDDetector.h"
#include "MDVolumeSequence.h"
#include "MPhysicalEventHit.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MRESE
{
  // Public Interface:
 public:
  MRESE();
  MRESE(MVector Pos, double Energy, double Time, int Detector, 
        MVector ResPos, double ResEnergy, double ResTime);
  MRESE(MRESE* RESE);
  
  //! The destructor does NOT delete the included RESEs
  //! If you want this behaviour, call DeleteAll() before calling delete
  virtual ~MRESE();

  static void ResetIDCounter();

  bool operator==(MRESE& RESE);
  virtual MRESE* Duplicate();

  // the data interface
  int GetType();
  //! Return the unique ID
  int GetID();

  //! Redo all IDs to make sure they are unique
  void ReID();

  //! If AllowOverwrite is false, neither the energy, nor position, nor time, 
  //! nor their resolutions are updated in case a RESE is added to this one! 
  //! Those function then can only be updated by the according Get/Set methods
  //! Setting this value to true, does not recalculate everything!
  void AllowOverwrite(bool Allow) { m_AllowOverwrite = Allow; }
  //! Return the allow overwrite flag
  bool AllowOverwrite() { return m_AllowOverwrite; }


  virtual void SetPosition(double x, double y, double z); 
  virtual void SetPosition(MVector Position); 
  virtual MVector GetPosition(); 
  virtual inline double GetPositionX() const { return m_Position.X(); }
  virtual inline double GetPositionY() const { return m_Position.Y(); }
  virtual inline double GetPositionZ() const { return m_Position.Z(); }

  virtual void SetPositionResolution(MVector Position); 
  virtual MVector GetPositionResolution();
  virtual inline double GetPositionResolutionX() const { return m_PositionResolution.X(); }
  virtual inline double GetPositionResolutionY() const { return m_PositionResolution.Y(); }
  virtual inline double GetPositionResolutionZ() const { return m_PositionResolution.Z(); }

  virtual void SetEnergy(double Energy); 
  virtual void SetEnergyResolution(double Energy); 
  virtual double GetEnergy(); 
  virtual double GetEnergyResolution(); 
  virtual void SetAdditionalEnergy(double Energy);
  virtual void SetTime(double Time); 
  virtual void SetTimeResolution(double Time); 
  virtual double GetTime(); 
  virtual double GetTimeResolution(); 
  virtual void SetDetector(int Detector); 
  virtual int GetDetector(); 
  virtual void SetVolumeSequence(MDVolumeSequence* VS);
  virtual MDVolumeSequence* GetVolumeSequence();
 
  // the link interface:
  int GetNLinks(); 
  bool HasLinks();
  bool IsLink(MRESE* RESE);
  void AddLink(MRESE* RESE);
  MRESE* GetLinkAt(int i);
  MRESE* GetOtherLink(MRESE* RESE);
  void RemoveLink(MRESE* RESE);
  void RemoveAllLinks();
  void LinkSequential();

  // the container interface (interfacing to a MRESEList-object)
  virtual int GetNRESEs();
  virtual bool ContainsRESE(MRESE* RESE);
  virtual bool ContainsRESE(int ID);
  virtual void AddRESE(MRESE* RESE);
  virtual void AddRESEFirst(MRESE* RESE);

  //! Get RESE by ID - ID must be directly contained - i.e. not be a RESE of a RESE, which is contained by the RESE 
  virtual MRESE* GetRESE(int ID);
  //! Get RESE by position in the array
  virtual MRESE* GetRESEAt(int i);
  //! Get RESE by ID - Return toplevel RESE which is or contains a RESE with this ID
  virtual MRESE* GetRESEContainingRESE(int ID);

  virtual MRESE* RemoveRESE(MRESE* RESE);
  virtual MRESE* RemoveRESEAndCompress(MRESE* RESE);
  virtual MRESE* RemoveRESEAt(int i);
  virtual MRESE* RemoveRESEAtAndCompress(int i);
  virtual MRESE* RemoveRESE(int ID);
  virtual MRESE* RemoveRESEAndCompress(int ID);

  virtual void DeleteRESE(MRESE* RESE);
  virtual void DeleteRESEAndCompress(MRESE* RESE);
  virtual void DeleteRESEAt(int i);
  virtual void DeleteRESEAtAndCompress(int i);
  virtual void DeleteRESE(int ID);
  virtual void DeleteRESEAndCompress(int ID);
  virtual void CompressRESEs();

  //! Shuffle the RESEs around in random order (goal: they are more or less sorted when the come from cosima, but not in real life)
  virtual void Shuffle();
  
  // delete all the RESEs which are part of this RESE
  virtual void DeleteAll();

  virtual double ComputeMinDistance(MRESE* RESE);
  virtual MVector ComputeMinDistanceVector(MRESE* RESE);
  //! Tests if RESE is adjacent to this one
  //! All non strip dimension have to be not more than Sigma position resolutions
  //! appart to be considered as "adjacent"
  virtual bool AreAdjacent(MRESE* R, double Sigma = 3, int Level = 1);
  
  virtual MString ToString(bool WithLink = true, int Level = 0);
  //! Convert to a string in the evta file
  virtual MString ToEvtaString(const int Precision, const int Version = 1);

  //! Convert to a MPhysicalEventHit
  MPhysicalEventHit CreatePhysicalEventHit();
  
  virtual bool IsValid();
  virtual void SetValid(bool Valid) { m_IsValid = Valid; }

  

  static const int c_Unknown;
  static const int c_Hit;
  static const int c_Track;
  static const int c_Cluster;
  static const int c_Bremsstrahlung;
  static const int c_Event;

 protected:
  virtual void RecalculateResolutions();
  
  //! Initialize / Reset this RESE
  void Reset();

  
  // private interface:
 private:
  MRESEList* GetRESEList();
  MRESEList* GetLinkList();


  // protected members:
 protected:
  //! Type of this RESE ()
  int m_SubElementType;

  //! Unique ID of this hit (Clones/Duplicates have the same ID)
  int m_ID;

  //! If AllowOverwrite is false, neither the energy, nor position, nor time, 
  //! nor their resolutions are updated in case a RESE is added to this one!
  //! Those function then can only be updated by the according Get/Set methods
  bool m_AllowOverwrite;

  //! Position of the hit in cm
  MVector m_Position;
  //! Energy of the hit in keV 
  double m_Energy; 
  //! Relative time of hit in s
  double m_Time;
  //! Additional energy in keV (escaped, lost in defective strips, etc.)
  double m_AdditionalEnergy;
  //! Position resolution of the hit in cm
  MVector m_PositionResolution;
  //! Energy resolution of the hit in keV 
  double m_EnergyResolution; 
  //! Time resolution of the hit in s 
  double m_TimeResolution;

  //! Number of the detector (1: D1, 2:D2)
  int m_Detector;

  //! Geomega volume sequence
  MDVolumeSequence* m_VolumeSequence;

  //! True if good RESE - currently only relevant for measured data...
  bool m_IsValid;

  //! Storage for the link-interface
  MRESEList* m_LinkList;
  //! Storage for the container-interface
  MRESEList* m_RESEList;

  // private members:
 private:
  //! Counts the distributed IDs
  static int m_IDCounter; 



#ifdef ___CINT___
 public:
  ClassDef(MRESE, 0) // sub-element of an event (hit, track, cluster)
#endif

};


////////////////////////////////////////////////////////////////////////////////


class CompareRESEByZ
{
 public:
  bool operator() (MRESE* first, MRESE* second) const {
    return (first->GetPosition().Z() > second->GetPosition().Z());
  }
};


#endif


////////////////////////////////////////////////////////////////////////////////
