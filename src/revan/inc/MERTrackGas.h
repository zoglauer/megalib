/*
 * MERTrackGas.h
 *
 * Copyright (C) by Peter Bloser & Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MERTrackGas__
#define __MERTrackGas__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MGeometryRevan.h"
#include "MERTrack.h"


// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MERTrackGas : public MERTrack
{
  // public interface:
 public:
  MERTrackGas();
  virtual ~MERTrackGas();

  //! Set all special parameters - this function should not rely on a previous 
  //! call to SetParameters()
  virtual bool SetSpecialParameters() { return true; }

  virtual MString ToString(bool CoreOnly = false) const;

  // protected methods:
 protected:
  virtual bool EvaluateTracks(MRERawEvent* ER);
  virtual MRawEventIncarnations* TrackComptons(MRERawEvent* ER);

  // private methods:
 private:



  // protected members:
 protected:

  // private members:
 private:


#ifdef ___CLING___
 public:
  ClassDef(MERTrackGas, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
