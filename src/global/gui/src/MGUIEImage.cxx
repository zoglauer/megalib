/*
 * MGUIEImage.cxx
 *
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 *
 * This code implementation is the intellectual property of
 * Andreas Zoglauer.
 *
 * By copying, distributing or modifying the Program (or any work
 * based on the Program) you indicate your acceptance of this statement,
 * and all its terms.
 *
 */


////////////////////////////////////////////////////////////////////////////////
//
// MGUIEImage
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MGUIEImage.h"

// Standard libs:

// ROOT libs:
#include <TCanvas.h>

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MGUIEImage)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIEImage::MGUIEImage(const TGWindow *Parent, MImage *Image, unsigned int Width, unsigned int Height, bool DisplayUndock) : 
  MGUIElement(Parent)
{
  // Construct an instance of MGUIEImage

  SetWidth(Width);
  SetHeight(Height);

  m_Image = Image;
  m_DisplayUndock = DisplayUndock;

  Create();
}


////////////////////////////////////////////////////////////////////////////////


MGUIEImage::~MGUIEImage()
{
  // Delete this instance of MGUIEImage

  if (MustCleanup() == kNoCleanup) {
    if (m_DisplayUndock == true) {
      delete m_UndockButton;
      delete m_UndockButtonLayout;
    }

    delete m_ImageCanvas;
    delete m_ImageCanvasLayout;
  }
}


////////////////////////////////////////////////////////////////////////////////


void MGUIEImage::Create()
{
  // Create the GUI element

  // Dispolay a canvas only if we have an image
  if (m_Image != 0) {

    // Display the undock button
    if (m_DisplayUndock == true) {
      m_UndockButton = new TGPictureButton(this, fClient->GetPicture("resource/icons/red_1p.xpm"), 1);
      m_UndockButton->Associate(this);
      m_UndockButtonLayout = new TGLayoutHints(kLHintsTop | kLHintsRight, 0, 0, 0, 0);
      AddFrame(m_UndockButton, m_UndockButtonLayout);
    }

    m_ImageCanvas = new TRootEmbeddedCanvas("EC 1", this, GetWidth(), GetHeight());
    m_ImageCanvasLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX |
                                            kLHintsExpandY, 0, 0, 0, 0);

    m_ImageCanvas->GetCanvas()->Clear();
    m_Image->Display(m_ImageCanvas->GetCanvas());

    AddFrame(m_ImageCanvas, m_ImageCanvasLayout);
  }
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIEImage::ProcessMessage(long Message, long Parameter1, 
                                  long Parameter2)
{
  // Process the messages for this application
  
  switch (GET_MSG(Message)) {
  case kC_COMMAND:
    switch (GET_SUBMSG(Message)) {
    case kCM_BUTTON:
      switch (Parameter1) {
      case 1: // Undock-Button
        Undock();
        break;
        
      default:
        break;
      }
    }

  default:
    break;
  }
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


MImage* MGUIEImage::GetImage()
{
  //

  return m_Image;
}



////////////////////////////////////////////////////////////////////////////////


void MGUIEImage::Undock()
{
  // Create a canvas all stuff is printed to

  if (m_Image != 0) {
    new TCanvas(m_Image->GetTitle(), m_Image->GetTitle());
    m_Image->DrawCopy();
  }
}


// MGUIEImage.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
