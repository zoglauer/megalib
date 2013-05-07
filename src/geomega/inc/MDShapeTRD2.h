/*
 * MDShapeTRD2.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MDShapeTRD2__
#define __MDShapeTRD2__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <TTRD2.h>
#include <MString.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MDShape.h"
#include "MVector.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MDShapeTRD2 : public MDShape
{
  // public interface:
 public:
  MDShapeTRD2();
  virtual ~MDShapeTRD2();

  bool Initialize(double dx1, double dx2, double dy1, double dy2, double z);

  TShape* GetShape();
  void CreateShape();
  MVector GetSize();

  double GetDx1() const;
  double GetDx2() const;
  double GetDy1() const;
  double GetDy2() const;
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
  TTRD2* m_TRD2;

  double m_Dx1;
  double m_Dx2;
  double m_Dy1;
  double m_Dy2;
  double m_Z;


#ifdef ___CINT___
 public:
  ClassDef(MDShapeTRD2, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
