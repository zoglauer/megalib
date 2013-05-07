/*
 * MGUIEFileSelector.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUIEFileSelector__
#define __MGUIEFileSelector__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <TROOT.h>
#include <TGFrame.h>
#include <TGLabel.h>
#include <MString.h>
#include <TGTextEntry.h>
#include <TGTextBuffer.h>
#include <TGButton.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIElement.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MGUIEFileSelector : public MGUIElement
{
  // public interface:
 public:
  MGUIEFileSelector(const TGWindow* Parent, MString Label, MString FileName);
  virtual ~MGUIEFileSelector();

  void SetFileName(MString Name);
  void SetFileType(MString Name, MString Suffix);
  void SetFileTypes(const char** FileTypes);
  void SetEnabled(bool flag);

  MString GetFileName();
  bool ProcessMessage(long Message, long Parameter1, long Parameter2);


  // private methods:
 private:
  void Create();


  // private members:
 private:
  const char** m_FileTypes;

  MString m_Label;
  MString m_FileName;

  TGLabel* m_TextLabel;
  TGLayoutHints* m_TextLabelLayout;

  TGHorizontalFrame* m_InputFrame;
  TGLayoutHints* m_InputFrameLayout;

  TGLayoutHints* m_InputLayout;
  TGTextBuffer* m_InputBuffer;
  TGTextEntry* m_Input;

  TGLayoutHints* m_ButtonFolderLayout;
  TGPictureButton* m_ButtonFolder;



#ifdef ___CINT___
 public:
  ClassDef(MGUIEFileSelector, 0) // Basic GUI element: a label, an input field for files and a file button 
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
