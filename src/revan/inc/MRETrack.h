/*
 * MRETrack.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MRETrack__
#define __MRETrack__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MRESE.h"

// Forward declarations:
class MHit;


////////////////////////////////////////////////////////////////////////////////


class MRETrack : public MRESE
{
  // Public Interface:
 public:
  MRETrack();
  MRETrack(MRETrack* Track);
  virtual ~MRETrack();

  double ComputeMinDistance(MRESE* RESE);
  MVector ComputeMinDistanceVector(MRESE* RESE);
  bool AreAdjacent(MRESE* R, double Sigma = 3, int Level = 1);

  int GetDetector();

  void AddRESE(MRESE* RESE);
  MRESE* RemoveRESE(MRESE* RESE);

  MString ToString(bool WithLink = true, int Level = 0);

  void SetStartPoint(MRESE* RESE);
  MRESE* GetStartPoint();

  void SetStopPoint(MRESE* RESE);
  MRESE* GetStopPoint();

  //! Return true if the start direction of the track is fixed
  bool IsDirectionFixed() const { return m_IsDirectionFixed; }
  //! Set a fixed direction for the track
  //! Currently the fixed direction is not invalidated if new start point is added - but don't rely on this... 
  void SetFixedDirection(const MVector& Direction) { m_FixedDirection = Direction; m_IsDirectionFixed = true; }
  //! Remove a fixed direction of the track
  void RemoveFixedDirection() { m_FixedDirection = g_VectorNotDefined; m_IsDirectionFixed = false; }

  //! Return the direction of the track calculated from the start point -
  //! The length is ignored if we have a fixed start direction!
  MVector GetDirection(int Length = 1);
  //! Return the final direction of the track calculated from the stop point
  //! A fixed start direction does not influence this function
  MVector GetFinalDirection(int Length = 1);
  //! Return the direction of the track calculated by one of the end points
  //! A fixed start direction does not influence this function
  MVector GetDirectionOfEndPoint(MRESE* EndPoint, int Length = 1);
  //! Return the average direction of the track (fitted straight line)
  //! A fixed start direction does not influence this function
  MVector GetAverageDirection();

  //! Return the center of gravity of the track
  MVector GetCenterOfGravity();

  //! Only add the RESE to the end point list (do not link, do not add it to the track)
  void AddEndPoint(MRESE* RESE);
  //! Adds and links the new to the old endpoint AND adds the hit to this Track!
  void AddEndPoint(MRESE* NewEndPoint, MRESE* OldEndPoint);
  //! Append a track to the endpoint AND adds the hit to this Track AND removes them from the track!
  void AddEndPoint(MRETrack* Track, MRESE* TrackEndPoint, MRESE* OldEndPoint);
  MRESE* GetEndPointAt(int i);
  MRESE* GetOtherEndPoint(MRESE* RESE);
  int GetNEndPoints();
  void RemoveEndPoint(MRESE* RESE);
  void RemoveAllEndPoints();
  bool CreateEndPoints();

  void LinkSequential();
  bool ValidateLinks();

  bool ResetIterator(MRESE* RESE = 0);
  MRESE* GetNextRESE();

  bool IsValid();
  MRETrack* Duplicate();

  double CalculateStraightnessScore();
  double CalculateFirstLastHitScore();

  double CalculatePearsonCorrelation(bool OneIsGood = true);
  double CalculateSpearmanRankCorrelation();

  void SetQualityFactor(const double QualityFactor);
  double GetQualityFactor() const;

 protected:
  void RecalculateResolutions();
  

  // private members:
 private:
  //! The start point
  MRESE* m_Start;
  //! The end point
  MRESE* m_Stop; 

  //! A list of end points (i.e. possible starts and stops)
  MRESEList* m_EndPoints;

  //! To be removed... iterator RESE (current)
  MRESE* m_RESEIt_c;
  //! To be removed... iterator RESE (previous)
  MRESE* m_RESEIt_p;


  double m_QualityFactor;

  //! True if we have a fixed start direction
  bool m_IsDirectionFixed;
  //! The fixed start direction
  MVector m_FixedDirection;


#ifdef ___CINT___
 public:
  ClassDef(MRETrack, 0) // this is one track
#endif

};

std::ostream& operator<<(std::ostream& os, /*const*/ MRETrack& Track);

#endif


////////////////////////////////////////////////////////////////////////////////
