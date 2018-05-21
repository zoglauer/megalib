/*
 * MPrelude.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MPrelude__
#define __MPrelude__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MSettingsGlobal.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MPrelude
{
  // public interface:
 public:
  //! The prelude: Show license and/or Changelog
  MPrelude();
  //! Default destructor
  virtual ~MPrelude();

  //! Start the prelude
  bool Play();

  // protected methods:
 protected:
  //! Load file
  void Load(); 
  //! Save the data file
  void Save();
  
  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  //! The key data read from an XML file and stored there afterwards
  MSettingsGlobal m_Settings;  
  
#ifdef ___CLING___
 public:
  ClassDef(MPrelude, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
