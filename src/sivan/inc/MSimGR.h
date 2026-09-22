/*
 * MSimGR.h
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


#ifndef __MSimGR__
#define __MSimGR__


////////////////////////////////////////////////////////////////////////////////


// Standard libs

// ROOT libs

// MEGAlib libs
#include "MGlobal.h"
#include "MVector.h"
#include "MDGeometryQuest.h"
#include "MDVolumeSequence.h"
#include "MBinaryStore.h"


////////////////////////////////////////////////////////////////////////////////


class MSimGR
{
  // Public Interface:
 public:
  //! Default constructor of the GR keyword
  MSimGR(MDGeometryQuest* Geo = 0);
  //! Default constructor of the GR keyword
  MSimGR(const int Detector, const MVector& Position, const double Energy, MDGeometryQuest* Geo = 0);
  //! Default destructor
  virtual ~MSimGR();

  //! Parse one line of the sim file 
  bool AddRawInput(MString LineBuffer, const int Version);

  //! Parse the data from a binary stream
  bool ParseBinary(MBinaryStore& Store, const int BinaryPrecision = 32, const int Version = 25);
  
  
  //! Return the number of the detector, where the hit took place
  int GetDetectorType() const { return m_DetectorType; };
  //! Set the detctor type in which the hit took place
  void SetDetectorType(const int Detector) { m_DetectorType = Detector; }

  //! Retrieve the position
  MVector GetPosition() const { return m_Position; }
  //! Set the position
  void SetPosition(const MVector& Position) { m_Position = Position; }

  //! Retrieve the noised energy
  double GetEnergy() const { return m_Energy; }
  //! Set the (noised) energy
  void SetEnergy(const double Energy) { m_Energy = Energy; m_OriginalEnergy = m_Energy; }

  //! Return the original energy deposit - before noising
  double GetOriginalEnergy() const { return m_OriginalEnergy; }

  //! Set the volume sequence of this hit
  void SetVolumeSequence(MDVolumeSequence* VS);
  //! Return the volume sequence of this hit
  MDVolumeSequence* GetVolumeSequence();

  //! Return the content as a string for the sim file
  MString ToSimString(const int WhatToStore = 1, const int Precision = 0, const int Version = 25) const;
  //! Convert the *key* content to binary
  bool ToBinary(MBinaryStore& Out, const int WhatToStore = 1, const int BinaryPrecision = 32, const int Version = 25);
  
  // protected methods:
 protected:
  //! Do the actual noising of this hit - called by SetEnergy & SetPosition
  bool Noise(bool RecalculateVolumeSequence = true);


  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  //! Type of the detector where the guard ring hit took place
  int m_DetectorType;
  //! The position
  MVector m_Position;
  //! The energy
  double m_Energy;

  //! Energy deposit, before noising
  double m_OriginalEnergy;       

  //! Pointer to the geometry description
  MDGeometryQuest* m_Geometry;     
  //! Volume history of this hit
  MDVolumeSequence* m_VolumeSequence;       
  
  //! The flags from noising
  MString m_NoiseFlags;
  

#ifdef ___CLING___
 public:
  ClassDef(MSimGR, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
