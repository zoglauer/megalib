/*
 * MEREventTypeExternal.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MEREventTypeTorch__
#define __MEREventTypeTorch__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <TROOT.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MEREventType.h"
#include "MFileEventsType.h"
#include "MRawEventIncarnations.h"

// Torch libs:
// Unset ClassDef so that it could be used by torch
//#ifdef __ROOTCLING__
//  #undef ClassDef
//#endif

#include <torch/torch.h>
#include <torch/script.h>

// Re-def ClassDef for root by re-loading manually Rtypes.h
//#ifdef __ROOTCLING__
//  #undef ClassDef
//  #define ClassDef(name,id)                            \
//   _ClassDefOutline_(name,id,virtual,)               \
//   /** \cond HIDDEN_SYMBOLS */ static int DeclFileLine() { return __LINE__; } /** \endcond */
//#endif

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MEREventTypeTorch : public MEREventType
{
  // public interface:
 public:
  MEREventTypeTorch();
  virtual ~MEREventTypeTorch();

  //! Global parameters used by all electron tracking algorithms
  virtual void SetParameters(MString EventTypeFileName);
  bool Analyze(MRawEventIncarnations* List);

  bool PostAnalysis();
  MString ToString(bool CoreOnly = false) const;


  // protected members:
 protected:
  MString m_EventTypeFileName;
  MFileEventsType* m_FileEventsType;

  torch::jit::script::Module m_module;


#ifdef ___CLING___
 public:
  ClassDef(MEREventTypeTorch, 0) // no description
#endif

};

#endif




