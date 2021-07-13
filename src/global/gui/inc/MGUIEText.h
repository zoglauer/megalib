/*
 * MGUIEText.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUIEText__
#define __MGUIEText__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include "TGFrame.h"
#include "TGLabel.h"
#include "TObjArray.h"

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIElement.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MGUIEText : public MGUIElement
{
  // Public Interface:
 public:
  MGUIEText(const TGWindow* parent, MString Text, unsigned int Alignment = 0, bool IsBold = false, bool IsItalic = false);
  virtual ~MGUIEText();

  void SetText(MString Text);
  void SetBold(bool IsBold) { m_IsBold = IsBold; }
  void SetItalic(bool IsItalic) { m_IsItalic = IsItalic; }
  void SetAlignment(unsigned int Alignment);

  enum Alignment { c_Left = 0, c_Centered, c_Right };

  // protected methods:
 protected:
  void Create();


  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  MString m_Text;                 // the text itself
  unsigned int m_Alignment;       // Alignment of the text 
  bool m_IsBold;                  // True if text is bold
  bool m_IsItalic;                // True if text is italic


#ifdef ___CLING___
 public:
  ClassDef(MGUIEText, 0) // GUI window for unkown purpose ...
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
