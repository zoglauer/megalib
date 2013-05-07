/*
 * MBackprojectionCart3DArea.h
 *
 * Copyright (C) 1998-2009 by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MBackprojectionCart3DArea__
#define __MBackprojectionCart3DArea__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include "TObject.h"

// MEGAlib libs:
#include "MGlobal.h"
#include "MBackprojectionCart2DArea.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MBackprojectionCart3DArea : public MBackprojectionCart2DArea
{
  // public interface:
 public:
  MBackprojectionCart3DArea();
  ~MBackprojectionCart3DArea();

  void PrepareBackprojection();
  bool Backproject(MPhysicalEvent *Event, double *Image) { return false; };
  bool Backproject(MPhysicalEvent *Event, 
		     double *Image, double& Limit, int& NBelowLimit);

  bool Assimilate(MPhysicalEvent *Event);

  bool BackprojectionCompton(double *Image, double& Limit, int& NAboveLimit);


  // protected methods:
 protected:


  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:



#ifdef ___CINT___
 public:
  ClassDef(MBackprojectionCart3DArea, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
