/*
 * MPointSourceSelector.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MPointSourceSelector__
#define __MPointSourceSelector__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include "TObject.h"
#include <MString.h>
#include <TObjArray.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MParser.h"
#include "MPointSource.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MPointSourceSelector : MParser
{
  // public interface:
 public:
  MPointSourceSelector();
  virtual ~MPointSourceSelector();

  //! Open the file name and read the header
  virtual bool Open(MString FileName, unsigned int Way = 0);

  bool LoadList(MString FileName);

  void AddPointSource(const MPointSource& PointSource);
  MPointSource GetPointSourceAt(unsigned int i) const;
  unsigned int GetNPointSources() const;

  MPointSource& operator[] (unsigned i);

  // protected methods:
 protected:
  bool TokenizeLine(MTokenizer& T);


  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  vector<MPointSource> m_List;

  friend ostream& operator<<(ostream& os, const MPointSourceSelector& P);


#ifdef ___CINT___
 public:
  ClassDef(MPointSourceSelector, 0) // loads and/or selects pointsources out of a MPointSourceList
#endif

};

#endif

ostream& operator<<(ostream& os, const MPointSourceSelector& P);


////////////////////////////////////////////////////////////////////////////////
