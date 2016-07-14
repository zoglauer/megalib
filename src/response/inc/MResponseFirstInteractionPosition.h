/*
 * MResponseFirstInteractionPosition.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MResponseFirstInteractionPosition__
#define __MResponseFirstInteractionPosition__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MResponseBase.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MResponseFirstInteractionPosition : public MResponseBase
{
  // public interface:
 public:
  MResponseFirstInteractionPosition();
  virtual ~MResponseFirstInteractionPosition();

  //! Do all the response creation
  virtual bool CreateResponse();

  // protected methods:
 protected:
  //MResponseFirstInteractionPosition() {};
  //MResponseFirstInteractionPosition(const MResponseFirstInteractionPosition& ResponseFirstInteractionPosition) {};

  //! Load the simulation file in revan and mimrec as well as the configuration files:
  virtual bool OpenFiles();

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:



#ifdef ___CINT___
 public:
  ClassDef(MResponseFirstInteractionPosition, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
