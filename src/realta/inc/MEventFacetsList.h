/*
 * MEventFacetsList.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MEventFacetsList__
#define __MEventFacetsList__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include "TObject.h"
#include <TList.h>

// MEGAlib libs:
#include "MGlobal.h"

// Forward declarations:
class MEventFacets;

////////////////////////////////////////////////////////////////////////////////


class MEventFacetsList : public TObject
{
  // public interface:
 public:
  MEventFacetsList(UInt_t MaximumSize);
  ~MEventFacetsList();

  void SetMaximumSize(UInt_t MaximumSize);
  UInt_t GetSize();

  void Add(MEventFacets *Event);

  void RestartIterator(MEventFacets *Event = 0);
  MEventFacets* Next();

  // protected methods:
 protected:


  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  TList *m_List;

  UInt_t m_MaximumSize;

  MEventFacets *m_IterationEvent;



#ifdef ___CINT___
 public:
  ClassDef(MEventFacetsList, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
