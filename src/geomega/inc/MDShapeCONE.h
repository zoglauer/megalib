/*
 * MDShapeCONE.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MDShapeCONE__
#define __MDShapeCONE__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <TCONE.h>
#include <MString.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MDShape.h"
#include "MVector.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MDShapeCONE : public MDShape
{
  // public interface:
 public:
  MDShapeCONE();
  virtual ~MDShapeCONE();

  bool Initialize(double HalfHeight, double RminBottom, double RmaxBottom, double RminTop, double RmaxTop);

  TShape* GetShape();
  void CreateShape();
  MVector GetSize();

  MString ToString();
  MString GetGeant3DIM(MString ShortName);
  MString GetGeant3DATA(MString ShortName);
  MString GetGeant3ShapeName();
  int GetGeant3NumberOfParameters();
  MString GetMGeantDATA(MString ShortName);
  MString GetGeomega() const;

  double GetHalfHeight();
  double GetRminBottom();
  double GetRmaxBottom();
  double GetRminTop();
  double GetRmaxTop();

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
  TCONE *m_CONE;

  double m_HalfHeight;
  double m_RminBottom;
  double m_RmaxBottom;
  double m_RminTop;
  double m_RmaxTop;


#ifdef ___CINT___
 public:
  ClassDef(MDShapeCONE, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
