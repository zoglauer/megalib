/*
 * MDShapeSPHE.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MDShapeSPHE__
#define __MDShapeSPHE__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <TSPHE.h>
#include <MString.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MDShape.h"
#include "MVector.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MDShapeSPHE : public MDShape
{
  // public interface:
 public:
  MDShapeSPHE();
  virtual ~MDShapeSPHE();

  bool Initialize(double Rmin, double Rmax, 
                  double Thetamin, double Thetamax, 
                  double Phimin, double Phimax);

  TShape* GetShape();
  void CreateShape();
  MVector GetSize();

  double GetRmin() const;
  double GetRmax() const;
  double GetThetamin() const;
  double GetThetamax() const;
  double GetPhimin() const;
  double GetPhimax() const;


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
  TSPHE *m_SPHE;

  double m_Rmin;
  double m_Rmax;
  double m_Thetamin;
  double m_Thetamax;
  double m_Phimin;
  double m_Phimax;


#ifdef ___CINT___
 public:
  ClassDef(MDShapeSPHE, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
