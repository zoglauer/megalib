/*
 * MGUIEConnections.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUIEConnections__
#define __MGUIEConnections__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <TROOT.h>
#include <TGFrame.h>
#include <TGLabel.h>
#include <MString.h>
#include <TGButton.h>
#include <TGCanvas.h>

// Standard libs::
#include <vector>
using std::vector;

// MEGAlib libs:
#include "MGlobal.h"
#include "MConnection.h"
#include "MGUIElement.h"
#include "MGUIEConnection.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MGUIEConnections : public MGUIElement
{
  // public interface:
 public:
  MGUIEConnections(const TGWindow* Parent, MString Label, bool Emphasize); 
  virtual ~MGUIEConnections();

  void Create();
  void Associate(TGCompositeFrame* w);

  void Add(MConnection* Connection);
  void Remove(MConnection* Connection);

  void AddTransceiverName(MString Name);
  void RemoveTransceiverName(MString Name);

  vector<MConnection*> GetMarked();

  // private methods:
 private:
  bool ProcessMessage(long Message, long Parameter1, long Parameter2);

  // private members:
 private:
  TGCompositeFrame* m_MessageWindow;

  MString m_Label;
  TGLabel* m_TextLabel;
  TGLayoutHints* m_TextLabelLayout;

  TGLayoutHints* m_ConnectionsLayout;
  TGCanvas* m_Connections;
  TGCompositeFrame* m_Container;

  TGLayoutHints* m_ConnectionLayout;
  vector<MGUIEConnection*> m_ConnectionList;
  vector<MString> m_TransceiverNames;

  TGLayoutHints* m_ButtonFrameLayout;
  TGCompositeFrame* m_ButtonFrame;

  TGLayoutHints* m_ButtonLayout;
  TGTextButton* m_AddButton;
  TGTextButton* m_RemoveButton;

  enum ButtonIDs { e_Add = 300, e_Remove };


#ifdef ___CINT___
 public:
  ClassDef(MGUIEConnections, 0) // GUI window for unkown purpose ...
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
