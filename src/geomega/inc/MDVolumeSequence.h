/*
 * MDVolumeSequence.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MDVolumeSequence__
#define __MDVolumeSequence__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <MString.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MVector.h"
#include "MRotation.h"
#include "MDGridPoint.h"

// Standard libs:
#include <vector>
using namespace std;

// Forward declarations:
class MDVolume;
class MDDetector;

////////////////////////////////////////////////////////////////////////////////


class MDVolumeSequence 
{
  // public interface:
 public:
  //! Default constructor
  MDVolumeSequence();
  //! Copy constructor
  MDVolumeSequence(const MDVolumeSequence& V);
  //! Default destructor
  virtual ~MDVolumeSequence();

  //! Join two volume sequences: This is a special version for revan and should only be used there
  void Join(MDVolumeSequence VS);
  //! Empty the volume sequence
  void Reset();

  //! Returns true if the volume sequence is empty
  bool IsEmpty() const { return ((m_Volumes.size() == 0) ? true : false); }
  
  //! Add a volume sequence to the end - should always be used with AddPosition
  void AddVolume(MDVolume* Volume);
  //! Add a volume sequence to the front
  void AddVolumeFront(MDVolume* Volume);
  //! Return the number of volume sequences 
  unsigned int GetNVolumes() const;
  //! Return a volume sequence at a specific position
  MDVolume* GetVolumeAt(unsigned int i) const;
  //! Return the deepest = last volume in the sequence
  MDVolume* GetDeepestVolume() const;

  //! Add a position to the end - should always be used with AddVolume
  void AddPosition(const MVector& V);
  //! Add a position to the front - should always be used with AddVolumeFront
  void AddPositionFront(const MVector& V);
  //! Return a position at a specific point in the vector
  MVector GetPositionAt(unsigned int i) const;

  //! Add a rotation to the end 
  //void AddRotation(TMatrixD Rot);

  //! Set the detector
  void SetDetector(MDDetector* Detector);
  //! Get the detector or return zero if there is no detector
  MDDetector* GetDetector() const;

  //! Set the position within the detector volume
  void SetPositionInDetector(MVector Pos);
  //! Return the position within the detector volume --- before using test with GetDetectorVolume() != 0 is there is a detector!
  MVector GetPositionInDetector() const;
  //! Return the position in the detector as grid point - this info is not stored but calculated on the fly!
  MDGridPoint GetGridPoint() const;

  //! Set the detector volume
  void SetDetectorVolume(MDVolume* Volume);
  //! Get the detector volume or zero if there is not detector volume
  MDVolume* GetDetectorVolume() const;

  //! Set the position within the sensitive volume
  void SetPositionInSensitiveVolume(MVector Pos);
  //! Return the position within the sensitive volume --- before using test with GetSensitiveVolume() != 0 is there is such a volume
  MVector GetPositionInSensitiveVolume() const;

  //! Set the senitive volume
  void SetSensitiveVolume(MDVolume* Volume);
  //! Return the sensitive volume or zero if there is no sensitive volume in the sequence
  MDVolume* GetSensitiveVolume() const;

  //! Return true if a volume of the given name is in the sequence (make sure to consider the naming conventions during removal of virtual volumes)
  bool HasVolume(MString Name) const;
  //! Return true if the volume is in the sequence
  bool HasVolume(MDVolume* Volume) const;
  //! Return true if both volume sequences have the same detector (if multiple volumes have the same MDDetector, it is checked if the detector is at the same position)
  bool HasSameDetector(const MDVolumeSequence& VS) const;
  //! Return true if both volume sequences have the same detector (if multiple volumes have the same MDDetector, it is checked if the detector is at the same position)
  bool HasSameDetector(MDVolumeSequence* VS) const;

  //! Given a position in volume Volume,  rotate/translate in the first (the world) volume
  MVector GetPositionInFirstVolume(const MVector& Position, MDVolume* Volume) const;

  //! Return the rotation of the given volume IN the world volume
  MRotation GetRotationInFirstVolume(MDVolume* Volume) const;
  //! Return the TOTAL rotation world volume -> deepest volume
  MRotation GetRotation();

  //! Given a position in the world volume, rotate/translate into volume Volume
  //! It is not mandatory if the position is really inside - only the rotations/translations are executed
  //! Return g_VectorNotDefined on error (e.g. volume does not exist)
  MVector GetPositionInVolume(const MVector& Position, MDVolume* Volume) const;

  //! Return a unique world position of the detector volume
  //! This position allows to distinguish different detectors even if they are clones/copies
  //! Return g_VectorNotDefined in case of error
  MVector GetUniqueWorldPositionOfDetector() const;
  
  //! Dump the content to string
  MString ToString() const;
  //! Just dump the sequence of volumes to string
  MString ToStringVolumes() const;


  // protected methods:
 protected:


  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  // Keep this section as simple as possible - a lot of copying is probably necessary:

  //! The central part - pointers to the volumes sorted from outside in
  vector<MDVolume*> m_Volumes;
  //! Positions in this volumes
  vector<MVector> m_Positions;

  //! The detector - zero if there is none
  MDDetector* m_Detector;
  //! The named detector - zero if there is none
  MDDetector* m_NamedDetector;
  //! The detector volume (not the sensitive volume!)
  MDVolume* m_DetectorVolume;
  //! The position in the detector
  MVector m_PositionInDetector;
  //! The sensitive volume
  MDVolume* m_SensitiveVolume;
  //! The position in the sensitive volume
  MVector m_PositionInSensitiveVolume;

  //! True if we have a valid rotation - the rotation is only calculated upon request to save computation time
  bool m_ValidRotation;
  //! The rotation of the deepest volume relative to the world volume
  MRotation m_RotMatrix; 


#ifdef ___CLING___
 public:
  ClassDef(MDVolumeSequence, 0) // auxiliary file, which keeps info about a volumes parents, its sensitive volume and the belonging detector
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
