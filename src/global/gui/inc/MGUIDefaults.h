/*
 * MGUIDefaults.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUIDefaults__
#define __MGUIDefaults__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:
#include "TGFont.h"

// MEGAlib libs:
#include "MGlobal.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MGUIDefaults
{
  // public interface:
 public:
  //! Get an instance of this singleton
  static MGUIDefaults* GetInstance();

  //! Get the normal medium font
  const TGFont* GetNormalMediumFont();
  //! Get the normal bold font
  const TGFont* GetNormalBoldFont();
  //! Get the italic medium font
  const TGFont* GetItalicMediumFont();
  //! Get the italic bold font
  const TGFont* GetItalicBoldFont();
  
  //! Get the font scaler relative to the default 12
  double GetFontScaler() { return m_DefaultFontSize / 12.0; }
  
  // protected methods:
 protected:

  // private methods:
 private:
  //! Protected constructor
  MGUIDefaults();
  //! Protected destructor
  ~MGUIDefaults();
  //! Protected copy constructor 
  MGUIDefaults(MGUIDefaults const&);   // Don't Implement
  //! Protected copy operator
  void operator=(MGUIDefaults const&); // Don't implement

  // protected members:
 protected:


  // private members:
 private:
  //! The font size
  int m_DefaultFontSize;
   
  //! The normal medium font
  const TGFont* m_NormalMediumFont;
  //! The normal bold font
  const TGFont* m_NormalBoldFont;
  //! The italic medium font
  const TGFont* m_ItalicMediumFont;
  //! The italic bold font
  const TGFont* m_ItalicBoldFont;
  

#ifdef ___CLING___
 public:
  ClassDef(MGUIDefaults, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
