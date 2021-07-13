/*
 * MSimIA.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MSimIA__
#define __MSimIA__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <iostream>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MVector.h"
#include "MStreams.h"

////////////////////////////////////////////////////////////////////////////////


class MSimIA
{
  // Public Interface:
 public:
  //! Default constructor
  MSimIA();
  //! Copy constructor
  MSimIA(const MSimIA& IA);
  //! Destructor
  virtual ~MSimIA();

  //! Parse a text line to fill this event
  bool AddRawInput(MString LineBuffer, int Version = 0);

  //! Set the interaction process as 4 character string, e.g. "COMP"
  void SetProcess(const MString& Process) { m_Process = Process; }
  //! Get the interaction process as 4 character string, e.g. "COMP"
  MString GetProcess() const { return m_Process; }

  //! Set the detector type number
  void SetDetectorType(const int Type) { m_DetectorType = Type; }
  //! Retrive the detector type number
  int GetDetectorType() const { return m_DetectorType; }

  //! Set the interaction ID
  void SetID(const int ID) { m_ID = ID; }
  //! Get the interaction ID
  int GetID() const { return m_ID; }

  //! Set the origin interaction ID, i.e. the ID of the interaction which created this particle
  void SetOriginID(const int OriginID) { m_OriginID = OriginID; }
  //! Get the origin interaction ID, i.e. the ID of the interaction which created this particle
  int GetOriginID() const { return m_OriginID; }

  //! Set the interaction time 
  void SetTime(const double Time) { m_Time = Time; }
  //! Get the interaction time
  double GetTime() const { return m_Time; }

  //! Set the interaction position
  void SetPosition(const MVector& Pos) { m_Position = Pos; }
  //! Get the interaction position
  MVector GetPosition() const { return m_Position; }

  //! Set the mother particle ID
  void SetMotherParticleID(const int ID) { m_MotherParticleID = ID; }
  //! Get the mother particle ID
  int GetMotherParticleID() const { return m_MotherParticleID; }

  //! Set the NEW direction of the mother particle
  void SetMotherDirection(const MVector& Dir) { m_MotherParticleDirection = Dir; }
  //! Get the NEW direction od the mother particle
  MVector GetMotherDirection() const { return m_MotherParticleDirection; }

  //! Set the NEW polarization of the mother particle
  void SetMotherPolarisation(const MVector& Pol) { m_MotherParticlePolarisation = Pol; }
  //! Get the NEW polarization of the mother particle
  MVector GetMotherPolarisation() const { return m_MotherParticlePolarisation; }

  //! Set the NEW energy of the mother particle
  void SetMotherEnergy(const double E) { m_MotherParticleEnergy = E; }
  //! Get the NEW energy of the mother particle
  double GetMotherEnergy() const { return m_MotherParticleEnergy; }

  //! Set the ID of the generated particle
  void SetSecondaryParticleID(const int ID) { m_SecondaryParticleID = ID; }
  //! Get the ID of the generated particle
  int GetSecondaryParticleID() const { return m_SecondaryParticleID; }

  //! Set the direction of the generated particle
  void SetSecondaryDirection(const MVector& Dir) { m_SecondaryParticleDirection = Dir; }
  //! Get the direction of the generated particle
  MVector GetSecondaryDirection() const { return m_SecondaryParticleDirection; }

  //! Set the polarization of the generated particle
  void SetSecondaryPolarisation(const MVector& Pol) { m_SecondaryParticlePolarisation = Pol; }
  //! Get the polarization of the generated particle
  MVector GetSecondaryPolarisation() const { return m_SecondaryParticlePolarisation; }

  //! Set the kinetic energy of the generated particle
  void SetSecondaryEnergy(const double Energy) { m_SecondaryParticleEnergy = Energy; }
  //! Get the kinetic energy of the generated particle
  double GetSecondaryEnergy() const { return m_SecondaryParticleEnergy; }


  //! Return a string compatible with the sim file format
  MString ToSimString(const int WhatToStore = 1, const int Precision = 0, const int Version = 25) const;
  //! Get the content of the class as a descriptive string
  MString ToString() const;

  //! Add a simple offset to all hit origins - needed when concatenating sim events
  void OffsetOrigins(int Offset);

  // Not needed for geant4:
  void SetParentNucleus(const int ParentNucleus) { m_ParentNucleus = ParentNucleus; }
  int GetParentNucleus() const { return m_ParentNucleus; }

  // All the depreciated function names

  void SetType(const MString& Type) { mdep<<"Please use: SetProcess"<<show; m_Process = Type; }
  MString GetType() const { mdep<<"Please use: GetProcess"<<show; return m_Process; }

  void SetNumber(const int IANumber) { mdep<<"Please use: SetID"<<show; m_ID = IANumber; }
  int GetNumber() const { mdep<<"Please use: GetID"<<show; return m_ID; }

  void SetId(const int Id) { mdep<<"Please use: SetID"<<show; m_ID = Id; }
  int GetId() const { mdep<<"Please use: GetID"<<show; return m_ID; }

  void SetOrigin(const int Origin) { mdep<<"Please use: SetOriginID"<<show; m_OriginID = Origin; }
  int GetOrigin() const { mdep<<"Please use: GetOriginID"<<show; return m_OriginID; }

  void SetDetector(const int N) { mdep<<"Please use: SetDetectorType"<<show; m_DetectorType = N; }
  int GetDetector() const { mdep<<"Please use: GetDetectorType"<<show; return m_DetectorType; }

  void SetMotherParticleNumber(const int Number) { mdep<<"Please use: SetMotherParticleID"<<show; m_MotherParticleID = Number; }
  int GetMotherParticleNumber() const { mdep<<"Please use: GetMotherParticleID"<<show; return m_MotherParticleID; }

  void SetParticleNumber(const int Number) { mdep<<"Please use: SetSecondaryParticleID"<<show; m_SecondaryParticleID = Number; }
  int GetParticleNumber() const { mdep<<"Please use: GetSecondaryParticleID"<<show; return m_SecondaryParticleID; }

  void SetDirection(const MVector& Dir) { mdep<<"Please use: SetSecondaryDirection"<<show; m_SecondaryParticleDirection = Dir; }
  MVector GetDirection() const { mdep<<"Please use: GetSecondaryDirection"<<show; return m_SecondaryParticleDirection; }

  void SetPolarisation(const MVector& Pol) { mdep<<"Please use: SetSecondaryPolarisation"<<show; m_SecondaryParticlePolarisation = Pol; }
  MVector GetPolarisation() const { mdep<<"Please use: GetSecondaryPolarisation"<<show; return m_SecondaryParticlePolarisation; }

  void SetEnergy(const double Energy) { mdep<<"Please use: SetSecondaryEnergy"<<show; m_SecondaryParticleEnergy = Energy; }
  double GetEnergy() const { mdep<<"Please use: GetSecondaryEnergy"<<show; return m_SecondaryParticleEnergy; }



  // protected methods:
 protected:


  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  //! 4 charchter string code of the process, e.g. COMP
  MString m_Process;
  //! The ID of the detector type
  int m_DetectorType;
  
  //! The interaction ID
  int m_ID;
  //! The ID of the interaction wich generated the main partcile of this interaction
  int m_OriginID;

  //! The interaction time 
  double m_Time;
  //! The interaction position
  MVector m_Position;

  //! The particle ID of the mother particle
  int m_MotherParticleID;
  //! The NEW direction of the mother particle
  MVector m_MotherParticleDirection;
  //! The NEW polarization of the mother particle
  MVector m_MotherParticlePolarisation;
  //! the NEW energy of the mother particle
  double m_MotherParticleEnergy;

  //! The ID of the generated particle
  int m_SecondaryParticleID;
  //! The direction of the generated particle
  MVector m_SecondaryParticleDirection;
  //! The polarization of the generated particle
  MVector m_SecondaryParticlePolarisation;
  //! The kinetic energy of the generated particle
  double m_SecondaryParticleEnergy;

  //! Remove: The parent nucleus
  int m_ParentNucleus;

#ifdef ___CLING___
 public:
  ClassDef(MSimIA, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
