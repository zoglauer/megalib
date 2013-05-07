/*
 * MDShapeTRD1.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MDShapeTRD1__
#define __MDShapeTRD1__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <TTRD1.h>
#include <MString.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MDShape.h"
#include "MVector.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MDShapeTRD1 : public MDShape
{
  // public interface:
 public:
  MDShapeTRD1();
  virtual ~MDShapeTRD1();

  bool Initialize(double dx1, double dx2, double y, double z);

  TShape* GetShape();
  void CreateShape();
  MVector GetSize();

  double GetDx1() const;
  double GetDx2() const;
  double GetY() const;
  double GetZ() const;

  MString ToString();
  MString GetGeant3DIM(MString ShortName);
  MString GetGeant3DATA(MString ShortName);
  MString GetGeant3ShapeName();
  int GetGeant3NumberOfParameters();
  MString GetMGeantDATA(MString ShortName);
  MString GetGeomega() const;

  double GetVolume();

  //! Scale this shape by Factor
  virtual void Scale(const double Factor);

  //! Return a unique position within the volume of the detector (center if possible)
  virtual MVector GetUniquePosition() const;

  // protected methods:
 protected:


  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  TTRD1 *m_TRD1;

  double m_Ddx1;
  double m_Ddx2;
  double m_Dy;
  double m_Dz;


#ifdef ___CINT___
 public:
  ClassDef(MDShapeTRD1, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
