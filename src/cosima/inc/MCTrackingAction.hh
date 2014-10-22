/*
 * MCTrackingAction.hh
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */



/******************************************************************************
 *
 * Description of this class
 *
 */

#ifndef ___MCTrackingAction___
#define ___MCTrackingAction___

// Geant4:
#include "G4UserTrackingAction.hh"
#include "G4Track.hh"

// Cosima:

// MEGAlib:

// Standard lib:

// Forward declarations:


/******************************************************************************/

class MCTrackingAction : public G4UserTrackingAction
{
  // public interface:
public:
  /// Default constructor
  MCTrackingAction();
  /// Default destructor
  virtual ~MCTrackingAction();

  /// Actions before tracking starts
  virtual void PreUserTrackingAction(const G4Track* Track);

  /// Set the number of generated primaries in this EVENT (not track!)
  void SetNGeneratedParticles(int NGeneratedParticles);

  // protected methods:
protected:
  

  // protected members:
protected:


  // private members:
private:
  /// Number of generated primaries in this EVENT (not track!)
  int m_NPrimaries;

};

#endif


/*
 * MCTrackingAction.hh: the end...
 ******************************************************************************/
