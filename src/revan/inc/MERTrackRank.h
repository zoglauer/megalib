/*
 * MERTrackRank.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MERTrackRank__
#define __MERTrackRank__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MERTrack.h"

// Forward declarations:
class MRETrack;
class MRERawEvent;


////////////////////////////////////////////////////////////////////////////////


class MERTrackRank : public MERTrack
{
  // public interface:
 public:
  MERTrackRank();
  virtual ~MERTrackRank();

  //! Set all special parameters - this function should not rely on a previous 
  //! call to SetParameters()
  virtual bool SetSpecialParameters() { return true; }

  virtual MString ToString(bool CoreOnly = false) const;


  // protected methods:
 protected:
  virtual bool EvaluateTracks(MRERawEvent* ER);
  virtual bool EvaluateTrack(MRETrack* Track);

  virtual void SortByTrackQualityFactor(MRawEventList* List);


  // private methods:
 private:



  // protected members:
 protected:


#ifdef ___CLING___
 public:
  ClassDef(MERTrackRank, 0)
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
