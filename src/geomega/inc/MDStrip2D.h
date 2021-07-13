/*
 * MDStrip2D.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MDStrip2D__
#define __MDStrip2D__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MDDetector.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MDStrip2D : public MDDetector
{
  // public interface:
 public:
  MDStrip2D(MString String = "");
  MDStrip2D(const MDStrip2D& S);
  virtual ~MDStrip2D();

  virtual MDDetector* Clone();

  //! Copy data to named detectors
  virtual bool CopyDataToNamedDetectors();

  virtual void Noise(MVector& Pos, double& Energy, double& Time, MDVolume* Volume) const;

  virtual vector<MDGridPoint> Discretize(const MVector& Pos, 
                                         const double& Energy, 
                                         const double& Time, 
                                         MDVolume* Volume) const;
  //! Return the Grid point of this position
  virtual MDGridPoint GetGridPoint(const MVector& Pos) const;
  //! Return a position in detector volume coordinates
  virtual MVector GetPositionInDetectorVolume(const unsigned int xGrid, 
                                              const unsigned int yGrid,
                                              const unsigned int zGrid,
                                              const MVector PositionInGrid,
                                              const unsigned int Type,
                                              MDVolume* Volume);
  virtual MVector GetPositionResolution(const MVector& Pos, const double Energy) const;

  virtual MString GetGeomega() const;
  virtual MString ToString() const;

  
  //! Set that this detector has a guard ring
  //! It will be verified in Validate(), and then if there is no actual offset you will get an error there
  void HasGuardRing(bool HasGuardRing);
  //! We also use this variation from the base class
  using MDDetector::HasGuardRing;
  
  

  void SetWidth(const double x, const double y);
  double GetWidthX() const { return m_WidthX; }
  double GetWidthY() const { return m_WidthY; }

  void SetOffset(const double x, const double y);
  double GetOffsetX() const { return m_OffsetX; }
  double GetOffsetY() const { return m_OffsetY; }

  void SetPitch(const double x, const double y);
  double GetPitchX() const { return m_PitchX; }
  double GetPitchY() const { return m_PitchY; }

  void SetStripLength(const double x, const double y);
  double GetStripLengthX() const { return m_StripLengthX; }
  double GetStripLengthY() const { return m_StripLengthY; }

  void SetOrientation(const int Orientation) { m_Orientation = Orientation; }
  int GetOrientation() const { return m_Orientation; }

  void SetNStrips(const int x, const int y);
  int GetNStripsX() const { return m_NStripsX; }
  int GetNStripsY() const { return m_NStripsY; }
  
  int GetNWafersX() const { return m_NWafersX; }
  int GetNWafersY() const { return m_NWafersY; }

  virtual bool AreNear(const MVector& Pos1, const MVector& dPos1, 
                       const MVector& Pos2, const MVector& dPos2, 
                       const double Sigma, const int Level) const;
                       
  //! Given a position in the detector find the strips
  virtual bool DetermineStrips(const MVector& Pos, int& xStrip, int& yStrip) const;

  //! Check if all input is reasonable
  virtual bool Validate();


  // protected methods:
 protected:

  //! The grid is only created if it is actually used
  virtual void CreateBlockedTriggerChannelsGrid();


  // private methods:
 private:



  // protected members:
 protected:
  //! x width of one wafer
  double m_WidthX;
  //! y width of one wafer
  double m_WidthY;

  //! x offset after which the first strip starts
  double m_OffsetX;
  //! y offset after which the first strip starts
  double m_OffsetY;

  //! x thickness of the strips
  double m_PitchX;
  //! y thickness of the strips
  double m_PitchY;

  //! x number of strips
  int m_NStripsX;
  //! y number of strips
  int m_NStripsY;

  //! x number of wafers
  int m_NWafersX;
  //! y number of wafers
  int m_NWafersY;

  //! x length of the strips
  double m_StripLengthX;
  //! y length of the strips
  double m_StripLengthY;
  
  int m_Orientation;

  

  // private members:
 private:


#ifdef ___CLING___
 public:
  ClassDef(MDStrip2D, 0) // a basic SiStrip detetctor
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
