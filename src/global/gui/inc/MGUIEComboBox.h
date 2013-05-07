/*
 * MGUIEComboBox.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUIEComboBox__
#define __MGUIEComboBox__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <TROOT.h>
#include <TGFrame.h>
#include <TGLabel.h>
#include <MString.h>
#include <TGButton.h>
#include <TGComboBox.h>

// Standard libs::
#include "vector"
using std::vector;

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIElement.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MGUIEComboBox : public MGUIElement
{
  // public interface:
 public:
  MGUIEComboBox(const TGWindow* Parent, MString Label, bool Emphasize = false, unsigned int Id = 1); 
  virtual ~MGUIEComboBox();

  void Create();
  bool ProcessMessage(long Message, long Parameter1, long Parameter2);

  void SetComboBoxSize(int Size = 100);

  void Add(MString Entry, bool IsSelected = false);
  void Remove(MString Entry);
  MString GetSelected();

  // private methods:
 private:
  void AddAll();

  // private members:
 private:
  unsigned int m_Id;

  MString m_Label;

  int m_Selected;
  int m_ComboBoxSize;
  vector<MString> m_Names;


  TGLabel* m_TextLabel;
  TGLayoutHints* m_TextLabelLayout;

  TGComboBox* m_ComboBox;
  TGLayoutHints* m_ComboBoxLayout;
  

  enum Type { e_Nothing };


#ifdef ___CINT___
 public:
  ClassDef(MGUIEComboBox, 0) // GUI window for unkown purpose ...
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
