/*
 * MDShapeCONS.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MDShapeCONS__
#define __MDShapeCONS__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <TCONS.h>
#include <MString.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MDShape.h"
#include "MVector.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MDShapeCONS : public MDShape
{
  // public interface:
 public:
  MDShapeCONS();
  virtual ~MDShapeCONS();

  bool Initialize(double HalfHeight, double RminBottom, double RmaxBottom, double RminTop, double RmaxTop, double PhiMin, double PhiMax);

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
  double GetPhiMin();
  double GetPhiMax();

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
  TCONS* m_CONS;

  double m_HalfHeight;
  double m_RminBottom;
  double m_RmaxBottom;
  double m_RminTop;
  double m_RmaxTop;
  double m_PhiMin;
  double m_PhiMax;


#ifdef ___CINT___
 public:
  ClassDef(MDShapeCONS, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
