/*
 * MDShapeTUBS.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MDShapeTUBS__
#define __MDShapeTUBS__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <TTUBS.h>
#include <MString.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MDShape.h"
#include "MVector.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MDShapeTUBS : public MDShape
{
  // public interface:
 public:
  MDShapeTUBS();
  virtual ~MDShapeTUBS();

  bool Initialize(double Rmin, double Rmax, double HalfHeight, double Phi1, double Phi2);

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
  
  double GetRmin();
  double GetRmax();
  double GetHeight();
  double GetPhi1();
  double GetPhi2();

  double GetVolume();

  //! Scale this shape by Factor
  virtual void Scale(const double Factor);

  //! Return a unique position within the volume of the detector (center if possible)
  virtual MVector GetUniquePosition() const;

  //! Return a random position withn this volume
  virtual MVector GetRandomPositionInside(); 

  // protected methods:
 protected:


  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  TTUBS *m_TUBS;

  double m_Rmin;
  double m_Rmax;
  double m_HalfHeight;
  double m_Phi1;
  double m_Phi2;


#ifdef ___CINT___
 public:
  ClassDef(MDShapeTUBS, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
