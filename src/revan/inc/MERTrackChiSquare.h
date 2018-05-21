/*
 * MERTrackChiSquare.h
 *
 * Copyright (C) by Mark Polsen & Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MERTrackChiSquare__
#define __MERTrackChiSquare__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MGeometryRevan.h"
#include "MERTrack.h"


// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MERTrackChiSquare : public MERTrack
{
  // public interface:
 public:
  MERTrackChiSquare();
  virtual ~MERTrackChiSquare();

  //! Set all special parameters - this function should not rely on a previous 
  //! call to SetParameters()
  virtual bool SetSpecialParameters() { return true; }

  virtual MString ToString(bool CoreOnly = false) const;

  // protected methods:
 protected:

  virtual bool EvaluateTracks(MRERawEvent* ER);

  // private methods:
 private:



  // protected members:
 protected:

  // private members:
 private:


#ifdef ___CLING___
 public:
  ClassDef(MERTrackChiSquare, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
