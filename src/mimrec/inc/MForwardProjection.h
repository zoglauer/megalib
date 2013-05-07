/*
 * MForwardProjection.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MForwardProjection__
#define __MForwardProjection__


////////////////////////////////////////////////////////////////////////////////


// standard libs
#include <iostream>

// ROOT libs
#include "TObject.h"

// MEGAlib libs:
#include "MGlobal.h"
#include "MProjection.h"
#include "MPhysicalEvent.h"
#include "MFPDataPoint.h"


////////////////////////////////////////////////////////////////////////////////


class MForwardProjection : public MProjection
{
  // Public Interface:
 public:
  MForwardProjection();
  ~MForwardProjection();


  void SetSpaceBins(int NBinsPsi, int NBinsEta, int NBinsPhi, int NBinsTheta);

  bool ForwardProject(MPhysicalEvent *Event, MFPDataPoint *Point);
  bool Assimilate(MPhysicalEvent *Event);


  // protected methods:
 protected:


  // private methods:
 private:



  // protected members:
 protected:
  int m_NBinsPsi;
  int m_NBinsEta; 
  int m_NBinsPhi;
  int m_NBinsTheta;


  // private members:
 private:



#ifdef ___CINT___
 public:
  ClassDef(MForwardProjection, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
