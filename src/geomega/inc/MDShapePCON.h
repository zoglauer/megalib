/*
 * MDShapePCON.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MDShapePCON__
#define __MDShapePCON__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <vector>
using namespace std;

// ROOT libs:
#include <TPCON.h>
#include <MString.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MDShape.h"
#include "MVector.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MDShapePCON : public MDShape
{
  // public interface:
 public:
  MDShapePCON();
  virtual ~MDShapePCON();

  bool Initialize(double Phi, double DPhi, unsigned int NSection);
  bool AddSection(unsigned int Section, double z, double Rmin, double Rmax);

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

  double GetPhi();
  double GetDPhi();
  unsigned int GetNSections();
  double GetZ(unsigned int Section);
  double GetRmin(unsigned int Section);
  double GetRmax(unsigned int Section);

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
  TPCON* m_PCON;

  double m_Phi;
  double m_DPhi;
  unsigned int m_NSections;
  vector<double> m_Z;
  vector<double> m_Rmin;
  vector<double> m_Rmax;

#ifdef ___CINT___
 public:
  ClassDef(MDShapePCON, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
