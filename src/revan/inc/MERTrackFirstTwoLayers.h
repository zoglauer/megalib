#ifndef __MERTrackFirstTwoLayers__
#define __MERTrackFirstTwoLayers__

#include "MGlobal.h"
#include "MGeometryRevan.h"
#include "MERTrack.h"

class MERTrackFirstTwoLayers : public MERTrack
{
  // public interface:
 public:
  MERTrackFirstTwoLayers();
  virtual ~MERTrackFirstTwoLayers();

  // protected methods:
 protected:

  // private methods:
 private:

 #ifdef ___CINT___
 public:
  ClassDef(MERTrackFirstTwoLayers, 0) // no description
#endif

};

#endif