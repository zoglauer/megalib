/*
 * MGUIEEntryList.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUIEEntryList__
#define __MGUIEEntryList__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <TROOT.h>
#include <TGFrame.h>
#include <TGLabel.h>
#include <MString.h>
#include <TGButton.h>
#include <TObjArray.h>
#include <TArrayI.h>
#include <TGCanvas.h>

// Standard libs::
#include <limits.h>
#include <float.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIElement.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MGUIEEntryList : public MGUIElement
{
  // public interface:
 public:
  MGUIEEntryList(const TGWindow* Parent, MString Label = "", int Mode = c_MultipleLine);
  virtual ~MGUIEEntryList();

  bool ProcessMessage(long Message, long Parameter1, long Parameter2);

  void Add(MString Label, int Value = 0, bool Limits = false, int Min = -numeric_limits<int>::max(), int Max = numeric_limits<int>::max()) {
    Add(Label, (long) Value, Limits, (long) Min, (long) Max); 
  }
  void Add(MString Label, long Value = 0, bool Limits = false, 
           long Min = -numeric_limits<long>::max(), long Max = numeric_limits<long>::max());
  void Add(MString Label, double Value = 0.0, bool Limits = false, 
           double Min = -1.0*numeric_limits<double>::max(), double Max = numeric_limits<double>::max());
  void Add(MString Label, MString Value = "");

  void Create();

  //! Return true if the content has been modified 
  bool IsModified();

  //! True if this element is enabled
  virtual void SetEnabled(bool Enabled = true);

  //! Set the wrap length of all text
  virtual void SetWrapLength(int WrapLength);

  int GetNEntrys();

  bool IsInt(int Entry = -1, int Min = -numeric_limits<int>::max(), int Max = numeric_limits<int>::max()) {
    return IsInt(Entry, (long) Min, (long) Max); 
  }
  bool IsInt(int Entry = -1, long Min = -numeric_limits<long>::max(), long Max = numeric_limits<long>::max());
  bool IsDouble(int Entry = -1, double Min = -numeric_limits<double>::max(), double Max = numeric_limits<double>::max());

  MString GetAsString(int Entry);
  long GetAsInt(int Entry);
  double GetAsDouble(int Entry);

  void SetValue(int Entry, double Value);
  void SetValue(int Entry, long Value);
  void SetValue(int Entry, MString Value);

  void SetEntryFieldSize(int Size = 70);

  enum Mode { c_MultipleLine, c_SingleLine };

  // protected methods:
 protected:


  // private methods:
 private:


  // protected members:
 protected:


  // private members:
 private:
  MString m_Label;

  int m_Mode;

  int m_Size;

  TGLabel* m_TextLabel;                     // the label of the GUI element
  TGLayoutHints* m_TextLabelLayout;         // its layout

  TObjArray* m_EntryList;                   // array containing the entries
  TGLayoutHints* m_EntryLayout;             // the layout of the checknuttons


#ifdef ___CINT___
 public:
  ClassDef(MGUIEEntryList, 0) // Basic GUI element: a entry list
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
