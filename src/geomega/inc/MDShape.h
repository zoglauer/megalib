/*
 * MDShape.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MDShape__
#define __MDShape__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <MString.h>
#include <TShape.h>
#include <TGeoShape.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MVector.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MDShape
{
  // public interface:
 public:
  MDShape();
  virtual ~MDShape();

  virtual TShape* GetShape() = 0;
  virtual MVector GetSize() = 0;
  virtual MString ToString() = 0;
  virtual void CreateShape() = 0;

  virtual MString GetGeant3DIM(MString ShortName) = 0;
  virtual MString GetGeant3DATA(MString ShortName) = 0;
  virtual MString GetGeant3ShapeName() = 0;
  virtual int GetGeant3NumberOfParameters() = 0;
  virtual MString GetMGeantDATA(MString ShortName) = 0;
  virtual MString GetGeomega() const = 0;
  virtual MString GetType();

	virtual vector<MVector> CreateSurfacePattern(const unsigned int Detail = 0) const;
  virtual bool IsInside(const MVector& Pos, const double Tolerance = 0, const bool PreferOutside = false);
  virtual double DistanceInsideOut(const MVector& Pos, const MVector& Dir, double Tolerance = 0);
  virtual double DistanceOutsideIn(const MVector& Pos, const MVector& Dir, double Tolerance = 0);

  virtual double GetVolume() = 0;
  virtual MVector GetRandomPositionInside(); 

  //! Scale this shape by Factor
  virtual void Scale(const double Factor) = 0;

  //! Return a unique position within the volume of the detector (center if possible)
  virtual MVector GetUniquePosition() const = 0;

  void SetColor(int Color);
  int GetColor();

  static const double c_NoIntersection;

  // protected methods:
 protected:


  // private methods:
 private:



  // protected members:
 protected:
  int m_Color;
  MString m_Type;

  TGeoShape* m_Geo;

  // private members:
 private:


#ifdef ___CINT___
 public:
  ClassDef(MDShape, 0) // virtual base class for all shapes
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
