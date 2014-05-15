/*
 * MGUIEReadOutElement.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUIEReadOutElement__
#define __MGUIEReadOutElement__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <TROOT.h>
#include "TGButton.h"
#include "TGWidget.h"
#include "TGPicture.h"
#include "TGToolTip.h"
#include "TGButtonGroup.h"

// MEGAlib libs:
#include "MReadOutElement.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! Button like element for changing the selected ROE plus indicators
class MGUIEReadOutElement : public TGTransientFrame
{
  // public interface:
 public:
  //! Standard constructor
  MGUIEReadOutElement(TGCompositeFrame* Parent, const MReadOutElement& ROE, unsigned int ID);
  //! Default destructor
  virtual ~MGUIEReadOutElement();

  //! Associate the button click to this window
  void Associate(const TGWindow* W) { m_TextButton->Associate(W); }
  
  // private methods:
 private:

  // protected members:
 protected:


  // private members:
 private:
	//! The readout element
	const MReadOutElement& m_ROE;
  //! The unique ID of the associated text button
  unsigned int m_ID;
  
  //! The text button
  TGTextButton* m_TextButton;


#ifdef ___CINT___
 public:
  ClassDef(MGUIEReadOutElement, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
