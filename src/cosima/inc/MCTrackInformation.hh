/*
 * MCTrackInformation.hh
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */



/******************************************************************************
 *
 * Objects of this class belong to a track
 * This class stores temporary additional information of this track
 *
 */

#ifndef ___MCTrackInformation___
#define ___MCTrackInformation___

// Geant4:
#include "G4VUserTrackInformation.hh"


/******************************************************************************/

class MCTrackInformation : public G4VUserTrackInformation
{
  // public interface:
public:
  /// Default constructor
  MCTrackInformation();
  /// Extended constructor
  MCTrackInformation(int Id, int OriginId);
  /// Default destructor
  virtual ~MCTrackInformation();

  /// Set, if the digitization of the last hit was successful
  void SetDigitized(const bool IsDigitized);
  /// Return true, if the digitization of the last hit was successful
  bool IsDigitized() const;

  /// Set the id if this track
  void SetId(const int Id);
  /// Return the id of this track
  int GetId() const;
  
  /// Set the originating track Id 
  void SetOriginId(const int OriginId);
  /// Return the originating track Id 
  int GetOriginId() const;

  /// Not used
  void Print() const {}

  // protected methods:
protected:
  

  // protected members:
protected:


  // private members:
private:
  /// Track if a digitization happened for the LAST energy deposit
  bool m_IsDigitized;

  /// Id of the track
  int m_Id;

  /// Id of the origin track
  int m_OriginId;
};

#endif


/*
 * MCTrackInformation.hh: the end...
 ******************************************************************************/
