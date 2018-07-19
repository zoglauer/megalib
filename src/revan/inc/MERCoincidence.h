/*
 * MERCoincidence.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MERCoincidence__
#define __MERCoincidence__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <TROOT.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MTime.h"
#include "MERConstruction.h"
#include "MRawEventIncarnations.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MERCoincidence : public MERConstruction
{
  // public interface:
 public:
  //! Default constructor
  MERCoincidence();
  //! Default destructor
  virtual ~MERCoincidence();

  //! Set the coincidence window
  bool SetCoincidenceWindow(double Time);
  //! Search for coincidences --- return the event or zero if non found
  MRERawEvent* Search(MRawEventIncarnations* List, bool Clear);

  //! Dump some elemenatry information about the algorithm settings
  MString ToString(bool CoreOnly) const;

  //! ID representing no coincidence search
  static const int c_None = 0;
  //! ID representing a coincidence window
  static const int c_Window = 1;

  // protected methods:
 protected:

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  //! The used algorithm
  int m_Algorithm;

  //! The coincidence window
  MTime m_Window;

  //! The number of found coincidences
  int m_NFoundCoincidences;
  

#ifdef ___CLING___
 public:
  ClassDef(MERCoincidence, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
