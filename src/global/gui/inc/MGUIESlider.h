/*
 * MGUIESlider.h
 *
 * Copyright (C) by the MEGAlib contributors.
 *
 * This file is part of MEGAlib.
 *
 * MEGAlib is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * MEGAlib is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
 * License (License.md) for more details.
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */


#ifndef __MGUIESlider__
#define __MGUIESlider__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include "TObject.h"
#include <TGFrame.h>
#include <TGLabel.h>
#include <MString.h>
#include <TGSlider.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIElement.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MGUIESlider : public MGUIElement
{
  // public interface:
 public:
  MGUIESlider(const TGWindow* Parent, MString Label, double MinValue, double UsedValue, double MaxValue);
  virtual ~MGUIESlider();

  bool ProcessMessage(long Message, long Parameter1, long Parameter2);
  double GetValue();

  // protected methods:
 protected:


  // private methods:
 private:
  void Create();
  MString MakeSmartString(double Number);


  // protected members:
 protected:


  // private members:
 private:
  MString m_Label;                          // description label
  double m_MinValue;                      // minimum slider value
  double m_UsedValue;                     // current/used slider value
  double m_MaxValue;                      // maximum slider value

  TGLabel* m_TextLabel;                   
  TGLayoutHints* m_TextLabelLayout;

  TGVerticalFrame* m_SliderFrame;
  TGLayoutHints* m_SliderFrameLayout;

  TGHSlider* m_Slider;
  TGLayoutHints* m_SliderLayout;

  TGHorizontalFrame* m_SliderValuesFrame;
  TGLayoutHints* m_SliderValuesFrameLayout;

  TGLabel* m_MinLabel;
  TGLayoutHints* m_MinLabelLayout;
  TGLabel* m_UsedLabel;
  TGLayoutHints* m_UsedLabelLayout;
  TGLabel* m_MaxLabel;
  TGLayoutHints* m_MaxLabelLayout;


#ifdef ___CLING___
 public:
  ClassDef(MGUIESlider, 0) // a slider GUI element
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
