/*
 * MERTrackPearson.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MERTrackPearson__
#define __MERTrackPearson__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MERTrack.h"

// Forward declarations:
class MRETrack;
class MRERawEvent;


////////////////////////////////////////////////////////////////////////////////


class MERTrackPearson : public MERTrack
{
  // public interface:
 public:
  MERTrackPearson();
  virtual ~MERTrackPearson();

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


#ifdef ___CINT___
 public:
  ClassDef(MERTrackPearson, 0)
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
