/*
 * MDStrip3DDirectional.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MDStrip3DDirectional__
#define __MDStrip3DDirectional__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <MString.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MDStrip3D.h"
#include "MStreams.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MDStrip3DDirectional : public MDStrip3D
{
  // public interface:
 public:
  MDStrip3DDirectional(MString Name = "");
  MDStrip3DDirectional(const MDStrip3DDirectional& S);
  virtual ~MDStrip3DDirectional();

  virtual MDDetector* Clone();

  virtual bool NoiseDirection(MVector& Direction, double Energy) const;
  virtual void SetDirectionalResolutionAt(const double Energy, const double Resolution, const double Sigma);

  virtual MString ToString() const;

  //! Check if all input is reasonable
  virtual bool Validate();

  virtual MString GetGeomega() const;
  virtual MString GetMGeant() const;

  // protected methods:
 protected:


  // private methods:
 private:



  // protected members:
 protected:
  MSpline* m_DirectionalResolution; 
  MSpline* m_DirectionalResolutionSigma; 


  // private members:
 private:



#ifdef ___CINT___
 public:
  ClassDef(MDStrip3DDirectional, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
