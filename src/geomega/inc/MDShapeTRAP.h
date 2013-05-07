/*
 * MDShapeTRAP.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MDShapeTRAP__
#define __MDShapeTRAP__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <TTRAP.h>
#include <MString.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MDShape.h"
#include "MVector.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MDShapeTRAP : public MDShape
{
  // public interface:
 public:
  MDShapeTRAP();
  virtual ~MDShapeTRAP();

  bool Initialize(double Dz, double Theta, double Phi, 
                  double H1, double Bl1, double Tl1, double Alpha1, 
                  double H2, double Bl2, double Tl2, double Alpha2);

  TShape* GetShape();
  void CreateShape();
  MVector GetSize();

  double GetDz() const;
  double GetTheta() const;
  double GetPhi() const;
  double GetH1() const;
  double GetBl1() const;
  double GetTl1() const;
  double GetAlpha1() const;
  double GetH2() const;
  double GetBl2() const;
  double GetTl2() const;
  double GetAlpha2() const;

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
  TTRAP *m_TRAP;

  double m_Dz;
  double m_Theta;
  double m_Phi;
  double m_H1;
  double m_Bl1;
  double m_Tl1;
  double m_Alpha1; 
  double m_H2;
  double m_Bl2;
  double m_Tl2;
  double m_Alpha2;


#ifdef ___CINT___
 public:
  ClassDef(MDShapeTRAP, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
