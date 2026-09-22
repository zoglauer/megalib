/*
 * MGUIEImageContainer.h
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


#ifndef __MGUIEImageContainer__
#define __MGUIEImageContainer__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include "TObject.h"
#include <TGFrame.h>
#include <TGButton.h>
#include <TGLabel.h>
#include <MString.h>
#include <TDatime.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIElement.h"

// Forward declarations:
class TObjArray;
class MImage;
class MGUIEImage;


////////////////////////////////////////////////////////////////////////////////


class MGUIEImageContainer : public MGUIElement
{
  // public interface:
 public:
  MGUIEImageContainer(const TGWindow* Parent, MString Title = "", bool DisplayHeader = false);
  virtual ~MGUIEImageContainer();

  void Create();
  void Arrange();
  void SetRowsAndColumns(int NRows = -1, int NColumns = -1, bool ArrangeLeftRight = true);

  bool ProcessMessage(long Message, long Parameter1, long Parameter2);

  void AddImage(MImage* Image);
  int GetNImages();
  MImage* GetImageAt(int i);

  void Reset();


  // protected methods:
 protected:
  void Print();

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  MString m_Title;                   // Title of this image container
  TGLabel* m_TitleLabel;             // GUI-Encapsulation of the title
  TGLayoutHints* m_TitleLabelLayout; // its layout

  TGCompositeFrame* m_TitleFrame;
  TGLayoutHints* m_TitleFrameLayout;

  TObjArray* m_Container;            // The container for the images
  TObjArray* m_FrameContainer;       // needed???

  TGLayoutHints* m_MatrixLayout;     // Layout for the images ordered in a matrix
  TGCompositeFrame* m_MatrixFrame;     // Layout for the images ordered in a matrix

  TGPictureButton* m_PrintButton;    // the Button for printing
  TGLayoutHints* m_ButtonLayout;     // its layout

  int m_NRows;                     // number of rows in the matrix
  int m_NColumns;                  // number of coumns in the matrix
  bool m_ArrangeLR;                // true arrange from left-right else top-bottom

  bool m_DisplayHeader;            // true if the header containing the title and the buttons should be display

  static int m_CanvasCounter;      // special ID of the displayed canvas


#ifdef ___CLING___
 public:
  ClassDef(MGUIEImageContainer, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
