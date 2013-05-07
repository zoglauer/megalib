/*
 * MGUIEFlatToolBarButton.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUIEFlatToolBarButton__
#define __MGUIEFlatToolBarButton__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <TROOT.h>
#include "TGButton.h"
#include "TGWidget.h"
#include "TGPicture.h"
#include "TGToolTip.h"
#include "TGButtonGroup.h"


// MEGAlib libs:

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MGUIEFlatToolBarButton : public TGPictureButton
{
  // public interface:
 public:
  MGUIEFlatToolBarButton(const TGWindow* Parent, const TGPicture* Picture, int Id = -1);
  virtual ~MGUIEFlatToolBarButton();

	virtual void SetFlat(const bool IsFlat = true);
	virtual bool IsFlat() const;

	virtual void SetState(EButtonState state);

	virtual bool HandleCrossing(Event_t *event);
	virtual bool HandleFocusChange(Event_t *event);

  // protected methods:
 protected:


  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
	bool m_Flat;


#ifdef ___CINT___
 public:
  ClassDef(MGUIEFlatToolBarButton, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
