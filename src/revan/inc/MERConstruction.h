/*
 * MERConstruction.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MERConstruction__
#define __MERConstruction__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <TROOT.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MRawEventIncarnations.h"
#include "MGeometryRevan.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MERConstruction
{
  // public interface:
 public:
  MERConstruction();
  virtual ~MERConstruction();

  virtual bool Analyze(MRawEventIncarnations* List);

  virtual void SetGeometry(MGeometryRevan* Geometry) { m_Geometry = Geometry; }

  virtual bool PreAnalysis() { return true; }
  virtual bool PostAnalysis() { return true; }

  virtual MString ToString(bool CoreOnly = false) const = 0;

  // protected methods:
 protected:
  //MERConstruction() {};
  //MERConstruction(const MERConstruction& ERConstruction) {};

  /// Modification routine at the beginning of the analysis to add/modify the events in the list before the analysis
  virtual void ModifyEventList();

  // private methods:
 private:



  // protected members:
 protected:
  //! The global event (incarnation) list
  MRawEventIncarnations* m_List;

  //! The geometry - used by most of the algorithms
  MGeometryRevan* m_Geometry;

  // private members:
 private:


#ifdef ___CLING___
 public:
  ClassDef(MERConstruction, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
