/*
 * MEREventClusterizer.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MEREventClusterizer__
#define __MEREventClusterizer__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <TROOT.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MERConstruction.h"
#include "MRawEventIncarnations.h"
#include "MRawEventIncarnationList.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! Find clusters for hits originating from the same gamma ray 
class MEREventClusterizer : public MERConstruction
{
  // public interface:
 public:
  //! Default constructor
  MEREventClusterizer();
  //! Default destructor
  virtual ~MEREventClusterizer();

  //! Do the analysis - this will add events to the whole incarnation list
  virtual bool Analyze(MRawEventIncarnationList* List) { return false; }

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
  ClassDef(MEREventClusterizer, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
