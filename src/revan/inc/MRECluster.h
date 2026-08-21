/*
 * MRECluster.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MRECluster__
#define __MRECluster__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MRESE.h"

// Forward declarations:
class MString;

class MREHit;


////////////////////////////////////////////////////////////////////////////////


class MRECluster : public MRESE
{
  // Public Interface:
 public:
  MRECluster();
  MRECluster(MRECluster* Cluster);
  virtual ~MRECluster();

  double ComputeMinDistance(MRESE *RESE);
  MVector ComputeMinDistanceVector(MRESE *RESE);
  bool AreAdjacent(MRESE* R, double Sigma = 3, int Level = 1);

  void AddRESE(MRESE *RESE);
  MRESE* RemoveRESE(MRESE *RESE);

  MString ToString(bool WithLink = true, int Level = 0);
  MRECluster* Duplicate();

  bool IsValid();

  
  // protected methods:
 protected:
  void AddHit(MREHit *Hit);
  
  
  // private members:
private:
  //! If set, the z-position (depth) of events with missing z-information is ignored
  bool m_IgnoreMissingDepth;
  
  

#ifdef ___CLING___
 public:
  ClassDef(MRECluster, 0) // a cluster of hits
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
