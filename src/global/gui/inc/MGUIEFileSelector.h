/*
 * MGUIEFileSelector.h
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
  MGUIEFileSelector(const TGWindow* Parent, MString Label, MString FileName = "");
  virtual ~MGUIEFileSelector();

  void SetFileName(MString Name);
  void SetFileType(MString Name, MString Suffix);
  //void SetFileTypes(const char** FileTypes);
  void SetEnabled(bool flag);

  MString GetFileName();
  bool ProcessMessage(long Message, long Parameter1, long Parameter2);


  // private methods:
 private:
  void Create();
  
  //! Select a file name
  bool SelectFileName();


  // private members:
 private:
  //! The file types in the ROOT compatible char** format
  const char** m_FileTypes;
  //! The number of file types added
  unsigned int m_NFileTypes;
  
  MString m_Label;
  MString m_FileName;

  TGLabel* m_TextLabel;
  TGLayoutHints* m_TextLabelLayout;

  TGHorizontalFrame* m_InputFrame;
  TGLayoutHints* m_InputFrameLayout;

  TGLayoutHints* m_InputLayout;
  TGTextEntry* m_Input;

  TGLayoutHints* m_ButtonFolderLayout;
  TGPictureButton* m_ButtonFolder;



#ifdef ___CLING___
 public:
  ClassDef(MGUIEFileSelector, 0) // Basic GUI element: a label, an input field for files and a file button 
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
