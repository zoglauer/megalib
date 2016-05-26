/*
 * MGUIElement.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUIElement__
#define __MGUIElement__


////////////////////////////////////////////////////////////////////////////////


// Standardlibs:

// ROOT libs:
#include <TROOT.h>
#include <TGFrame.h>
#include <TGLabel.h>
#include <MString.h>
#include <TGButton.h>

// MEGAlib libs:
#include "MGlobal.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MGUIElement : public TGCompositeFrame
{
  // public interface:
 public:
  //! Standard constructor
  MGUIElement(const TGWindow* Parent, unsigned int Options = kChildFrame); 
  //! Default destructor
  virtual ~MGUIElement();

  //! True if this element is enabled
  virtual void SetEnabled(bool Enabled = true) { m_IsEnabled = Enabled; }
  //! True if this element (or parts of it) are emphasized
  virtual void SetEmphasized(bool Emphasized = true) { m_IsEmphasized = Emphasized; } 
  //! Set the wrap length of all text
  virtual void SetWrapLength(int WrapLength) { m_WrapLength = WrapLength; }

  //! Return true if this element is enabled
  virtual bool IsEnabled() { return m_IsEnabled; }
  //! Return true if this element (or parts of it) are emphasized
  virtual bool IsEmphasized() { return m_IsEmphasized; } 

  // private methods:
 private:

  // protected members:
 protected:
  //! True if this element is enabled
  bool m_IsEnabled;
  //! True if this element (or parts of it) are emphasized
  bool m_IsEmphasized;

  //! Structure of an emphasized font
  FontStruct_t m_EmphasizedFont;
  //! The font scaler (normal font size is 12 - if we have anything larger we need to scale some GUIs)
  double m_FontScaler;

  //! The wrap length of text labels:
  int m_WrapLength;

#ifdef ___CINT___
 public:
  ClassDef(MGUIElement, 0) // GUI window for unkown purpose ...
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
