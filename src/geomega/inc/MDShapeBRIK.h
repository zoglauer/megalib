/*
 * MDShapeBRIK.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MDShapeBRIK__
#define __MDShapeBRIK__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <TBRIK.h>
#include <MString.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MDShape.h"
#include "MVector.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MDShapeBRIK : public MDShape
{
  // public interface:
 public:
  MDShapeBRIK();
  virtual ~MDShapeBRIK();

  bool Initialize(double x, double y, double z);

  TShape* GetShape();
  void CreateShape();
  MVector GetSize();

  virtual bool IsInside(const MVector& Pos, const double Tolerance = 0, const bool PreferOutside = false);

  MString ToString();
  MString GetGeant3DIM(MString ShortName);
  MString GetGeant3DATA(MString ShortName);
  MString GetGeant3ShapeName();
  int GetGeant3NumberOfParameters();
  MString GetMGeantDATA(MString ShortName);
  MString GetGeomega() const;

  double GetSizeX();
  double GetSizeY();
  double GetSizeZ();

  double GetVolume();

  //! Scale this shape by Factor
  virtual void Scale(const double Factor);

  //! Return a unique position within the volume of the detector (center if possible)
  virtual MVector GetUniquePosition() const;

  //! Return a random position withn this volume
  virtual MVector GetRandomPositionInside(); 

	virtual vector<MVector> CreateSurfacePattern(const unsigned int Detail = 0) const;

  // protected methods:
 protected:


  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  TBRIK *m_BRIK;

  double m_Dx;
  double m_Dy;
  double m_Dz;


#ifdef ___CINT___
 public:
  ClassDef(MDShapeBRIK, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
