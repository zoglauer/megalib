/*
 * MGUIEStatusBar.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUIEStatusBar__
#define __MGUIEStatusBar__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <TROOT.h>
#include <TGFrame.h>
#include <TGLabel.h>
#include <MString.h>
#include <TObjArray.h>
#include <TObjString.h>

// Standard libs::
#include <limits.h>
#include <float.h>
#include <stdlib.h>
#include <ctype.h>
#include <vector>
using std::vector;

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIElement.h"

// Forward declarations:



////////////////////////////////////////////////////////////////////////////////


class MGUIEStatusBar : public MGUIElement
{
  // public interface:
 public:
  MGUIEStatusBar(const TGWindow* Parent, MString Title = "", bool Emphasized = false, int Width = -9998); 
  virtual ~MGUIEStatusBar();

  bool Add(MString Field = "", int Width = 100, bool ShowTitle = true);

  bool SetContent(MString Field, double Value);
  bool SetContent(MString Field, int Value);
  bool SetContent(MString Field, unsigned int Value);
  bool SetContent(MString Field, MString Value);

  void Create();

  enum Widths { c_Max = -9999, c_Min = -9998};

  // private methods:
 private:
  void Init();

  MString MakeSmartString(double Number);

  // private members:
 private:
  bool m_IsCreated;

  int m_Width;
  
  TGGC* m_LabelGraphics;
  MString m_Label;

  TGCompositeFrame* m_LabelFrame;
  TGLayoutHints* m_LabelFrameLayout;

  TGLabel *m_TextLabel;
  TGLayoutHints *m_TextLabelLayout;

  TGLayoutHints* m_FieldTitleLayout;
  TGLayoutHints* m_FieldContentLayout;

  TObjArray* m_FieldNames;
  vector<int> m_FieldWidth;
  //! True if the title is shown
  vector<bool> m_FieldShowTitle;

  TObjArray* m_FieldFrames;
  TObjArray* m_FieldFrameLayouts;
  TObjArray* m_FieldTitles;
  TObjArray* m_FieldContentValues;
  TObjArray* m_FieldContentFrames;
  TObjArray* m_FieldContentFrameLayouts;
  TObjArray* m_FieldContents;


#ifdef ___CINT___
 public:
  ClassDef(MGUIEStatusBar, 0)
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
