/*
 * MImagerExternallyManaged.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MImagerExternallyManaged__
#define __MImagerExternallyManaged__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <vector>
using namespace std;

// ROOT libs:
#include <TObjArray.h>
#include <TMatrix.h>
#include <TThread.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MVector.h"
#include "MBPDataImage.h"
#include "MBackprojection.h"
#include "MEventSelector.h"
#include "MFileEventsTra.h"
#include "MSensitivity.h"
#include "MImage.h"
#include "MImager.h"

// Forward declarations:
class MBPData;
class MLMLAlgorithms;


////////////////////////////////////////////////////////////////////////////////


class MImagerExternallyManaged : public MImager
{
  // Public Interface:
 public:
  //! Standard constructor
  MImagerExternallyManaged(int CoordinateSystem);
  //! Default destructor
  virtual ~MImagerExternallyManaged();

  //! Enable the use of GUI features, i.e. progress bar, call ProcessEvents, etc.
  void UseGUI(bool UseGUI = true) { m_UseGUI = UseGUI; }

  //! Call before the response slice calculation after all options are set
  bool Initialize();
  
  //! Calculate the response slice for the given event
  //! Return the data or zero in case the event is not within the event selection, not within the image or we are out of events
  MBPData* CalculateResponseSlice(MPhysicalEvent* Event);

  //! Deconvolve a set of response slices 
  vector<MImage*> Deconvolve(vector<MBPData*> ResponseSlices);


  // protected methods:
 protected:


  // private members:
 private:
  //! True if GUI features are used, i.e. progress bar, call ProcessEvents, etc. (default: true)
  bool m_UseGUI;

#ifdef ___CINT___
 public:
  ClassDef(MImagerExternallyManaged, 0) // Computes and stores system matrix
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
