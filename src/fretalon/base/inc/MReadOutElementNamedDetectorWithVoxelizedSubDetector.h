/*
 * MReadOutElementNamedDetectorWithVoxelizedSubDetector.h
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


#ifndef __MReadOutElementNamedDetectorWithVoxelizedSubDetector__
#define __MReadOutElementNamedDetectorWithVoxelizedSubDetector__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MReadOutElement.h"

// Forward declarations:

////////////////////////////////////////////////////////////////////////////////


//! The read-out element of a BGO voxel. It reads the detector name and the voxel x, y, z, ID
class MReadOutElementNamedDetectorWithVoxelizedSubDetector : public MReadOutElement
{
  // public interface:
 public:
  //! default constructor - Read out element of a voxel 3D
  MReadOutElementNamedDetectorWithVoxelizedSubDetector();

  //! full constructor - Read out element of a voxel 3D
  MReadOutElementNamedDetectorWithVoxelizedSubDetector(const MString& DetectorID, unsigned int CrystalID, unsigned int VoxelXID, unsigned int VoxelYID, unsigned int VoxelZID);

  //! Simple default destructor
  virtual ~MReadOutElementNamedDetectorWithVoxelizedSubDetector();

  //! Clone this read-out element - the returned element must be deleted!
  virtual MReadOutElementNamedDetectorWithVoxelizedSubDetector* Clone() const;

  //! Clear the content of this read-out element
  virtual void Clear();

  //! Compare two read-out elements
  virtual bool operator==(const MReadOutElement& R) const;

  //! Return true if this read-out element is of the given type
  virtual bool IsOfType(const MString& String) const;
  //! Return the type of this read-out element
  virtual MString GetType() const;

  //! Set detector ID as string
  void SetDetectorID(const MString& DetectorID) { m_DetectorID = DetectorID; }
  //! Get detector ID as string
  MString GetDetectorID() const { return m_DetectorID; }

  //! Set crystal ID as int
  void SetCrystalID(unsigned int CrystalID) { m_CrystalID = CrystalID; }
  //! Get crystal ID as int
  unsigned int GetCrystalID() const { return m_CrystalID; }

  //! Set Voxel X ID as int
  void SetVoxelXID(unsigned int VoxelXID) { m_VoxelXID = VoxelXID; }
  //! Get Voxel X ID as int
  unsigned int GetVoxelXID() const { return m_VoxelXID; }

  //! Set Voxel Y ID as int
  void SetVoxelYID(unsigned int VoxelYID) { m_VoxelYID = VoxelYID; }
  //! Set Voxel Y ID as int
  unsigned int GetVoxelYID() const { return m_VoxelYID; }

  //! Set Voxel Z ID as int
  void SetVoxelZID(unsigned int VoxelZID) { m_VoxelZID = VoxelZID; }
  //! Get Voxel Z ID as int
  unsigned int GetVoxelZID() const { return m_VoxelZID; }

  //! Return the number of parsable elements
  virtual unsigned int GetNumberOfParsableElements() const;
  //! Parse the data from the tokenizer
  virtual bool Parse(const MTokenizer& T, unsigned int StartElement);
  //! Return the data as parsable string
  virtual MString ToParsableString(bool WithDescriptor = false) const;

  //! Dump a string
  virtual MString ToString() const;


  // protected methods:
 protected:
  // private methods:
 private:
  // protected members:
 protected:
  //! Detector ID as string
  MString m_DetectorID;
  //! Crystal ID as int
  unsigned int m_CrystalID;

  //! Voxel index X as int
  unsigned int m_VoxelXID;
  //! Voxel index Y as int
  unsigned int m_VoxelYID;
  //! Voxel index Z as int
  unsigned int m_VoxelZID;

  // private members:
 private:
#ifdef ___CLING___
 public:
  ClassDef(MReadOutElementNamedDetectorWithVoxelizedSubDetector, 0) // no description
#endif
};

//! Streamify the read-out element
ostream& operator<<(ostream& os, const MReadOutElementNamedDetectorWithVoxelizedSubDetector& R);

#endif


////////////////////////////////////////////////////////////////////////////////
