/*
 * MGUIETime.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUIETime__
#define __MGUIETime__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <TROOT.h>
#include <TGFrame.h>
#include <TGLabel.h>
#include <MString.h>
#include <TGComboBox.h>

// Standard libs::
#include <limits.h>
#include <float.h>
#include <stdlib.h>
#include <ctype.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIElement.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MGUIETime : public MGUIElement
{
  // public interface:
 public:
  MGUIETime(const TGWindow* Parent); 
  virtual ~MGUIETime();

  MString GetAsString();


  // private methods:
 private:
  void Create();
  bool ProcessMessage(long Message, long Parameter1, long Parameter2);


  // private members:
 private:
  TGLayoutHints* m_ComboLayout;
  TGComboBox* m_Hours;
  TGComboBox* m_Minutes;
  TGComboBox* m_Days;
  TGComboBox* m_Months;
  TGComboBox* m_Years;

  TGLayoutHints* m_LabelLayout;
  TGLabel* m_LabelColon;
  TGLabel* m_LabelAt;

  enum ComboIDs { c_Hour = 1, c_Minute, c_Day, c_Month, c_Year };

#ifdef ___CLING___
 public:
  ClassDef(MGUIETime, 0) // GUI window for unkown purpose ...
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
