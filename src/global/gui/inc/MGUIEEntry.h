/*
 * MGUIEEntry.h
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


#ifndef __MGUIEEntry__
#define __MGUIEEntry__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <TROOT.h>
#include <TGFrame.h>
#include <TGLabel.h>
#include <MString.h>
#include <TGTextEntry.h>
#include <TGTextBuffer.h>
#include <TGNumberEntry.h>

// Standard libs::
#include <limits>
#include <cstdlib>
#include <ctype.h>
using namespace std;

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIElement.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


// Actually it would be better to store all those global Ids in one place...
enum MGUIEEntryIds {
  kC_ENTRY = 2000,
  kET_CHANGED = 1
};


////////////////////////////////////////////////////////////////////////////////


class MGUIEEntry : public MGUIElement
{
  // public interface:
 public:
  MGUIEEntry(const TGWindow* Parent, MString Label, bool Emphasize, 
             double Value = 0.0, bool Limits = true, 
             double Min = -numeric_limits<double>::max()/2, double Max = numeric_limits<double>::max()/2); 
  MGUIEEntry(const TGWindow* Parent, MString Label, bool Emphasize, 
             int Value = 0, bool Limits = true, 
             int Min = -numeric_limits<int>::max()/2, int Max = numeric_limits<int>::max()/2); 
  MGUIEEntry(const TGWindow* Parent, MString Label, bool Emphasize, 
             long Value = 0, bool Limits = true, 
             long Min = -numeric_limits<long>::max()/2, long Max = numeric_limits<long>::max()/2); 
  MGUIEEntry(const TGWindow* Parent, MString Label, bool Emphasize, MString Value = " "); 
  virtual ~MGUIEEntry();

  void Init();

  //! Return true if the content has been modified 
  bool IsModified() { return m_IsModified; }

  //! Set the wrap length of all text
  virtual void SetWrapLength(int WrapLength);


  void Associate(TGCompositeFrame* w, int Id);
  bool ProcessMessage(long Message, long Parameter1, long Parameter2);

  void SetEntryFieldSize(int Size = 70);
  virtual void SetEnabled(bool flag = true);

  bool IsInt(long Min = -numeric_limits<long>::max(), long Max = numeric_limits<long>::max());
  bool IsDouble(double Min = -numeric_limits<double>::max(), double Max = numeric_limits<double>::max());

  MString GetAsString();
  long GetAsInt();
  double GetAsDouble();

  void SetValue(double Value);
  void SetValue(long Value);
  void SetValue(unsigned long Value);
  void SetValue(MString Value);

  // private methods:
 private:
  void Create();

  MString MakeSmartString(double Number);
  bool CheckForNumber(MString Number);
  double ToNumber(MString Number);
  bool CheckRange(double Value, double Min, double Max);

  // private members:
 private:
  TGCompositeFrame* m_MessageWindow;
  int m_Id;

  //! Flag indicating that the content was modifed
  bool m_IsModified;

  MString m_Label;

  MString m_Value;

  double m_ValueAsDouble;
  double m_Min;
  double m_Max;

  int m_Type;
  bool m_Limits;
  int m_Size;

  TGLabel* m_TextLabel;
  TGLayoutHints* m_TextLabelLayout;

  TGLayoutHints* m_InputLayout;

  TGTextBuffer* m_InputBuffer;
  TGTextEntry* m_Input;
  TGNumberEntry* m_NumberInput;

  enum Type { e_String, e_Integer, e_Double };


#ifdef ___CLING___
 public:
  ClassDef(MGUIEEntry, 0) // GUI window for unkown purpose ...
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
