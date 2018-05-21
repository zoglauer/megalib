/*
 * MGUIImageManager.cxx
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
// MGUIImageManager
//
// This class is a main GUI window allowing the user to view and manipulate
// his images.
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MGUIImageManager.h"

// Standard libs:

// ROOT libs:
#include <TGMsgBox.h>

// MEGAlib libs:
#include "MSystem.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MGUIImageManager)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIImageManager::MGUIImageManager()
  : MGUIDialog()
{
  // standard constructor

  m_ImageList = new TObjArray(10);
}

////////////////////////////////////////////////////////////////////////////////


MGUIImageManager::MGUIImageManager(const TGWindow *p, const TGWindow *main)
  : MGUIDialog(p, main)
{
  // standard constructor

  m_ImageList = new TObjArray(10);

  Create();
}


////////////////////////////////////////////////////////////////////////////////


MGUIImageManager::~MGUIImageManager()
{
  // standard destructor

}


////////////////////////////////////////////////////////////////////////////////


void MGUIImageManager::Create()
{
  // Create the main window - but do not display it

  // We start with a name and an icon...
  SetWindowName("M.I.Works. image manager");  

  m_CanvasColumn = new TGHorizontalFrame(this, 100, 10, kRaisedFrame);
  m_CanvasColumnLayout = new TGLayoutHints(kLHintsExpandX | kLHintsExpandY, 10, 10, 10, 10);
  AddFrame(m_CanvasColumn, m_CanvasColumnLayout);

  m_ECanvas = new TRootEmbeddedCanvas("EC 1", m_CanvasColumn, 100, 100);
  m_ECanvasLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX |
                                      kLHintsExpandY, 5, 5, 5, 5);
  m_CanvasColumn->AddFrame(m_ECanvas, m_ECanvasLayout);

  AddOKCancelButtons();
  PositionWindow(GetDefaultWidth(), GetDefaultHeight(), false);

  return;
}


////////////////////////////////////////////////////////////////////////////////


void MGUIImageManager::Display()
{
  // Display the window

  MapSubwindows();
  MapWindow();  

  Layout();
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIImageManager::ProcessMessage(long Message, long Parameter1, 
                                        long Parameter2)
{
  // Process the messages for this application

  switch (GET_MSG(Message)) {
  case kC_COMMAND:
    switch (GET_SUBMSG(Message)) {
    case kCM_BUTTON:
      switch (Parameter1) {
      case 1: // OK

        CloseWindow();
        break;
        
      case 2: // Cancel
        CloseWindow();
        break;
        
      default:
        break;
      }
    default:
      break;
    }
  default:
    break;
  }
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MGUIImageManager::CloseWindow()
{
  // this class is NOT allowed to delete itself

  UnmapWindow();
}


////////////////////////////////////////////////////////////////////////////////


void MGUIImageManager::AddImage(MImage* Image)
{ 
  // Add an image to the internal image list

  m_ImageList->AddLast(Image);
}


////////////////////////////////////////////////////////////////////////////////


MImage* MGUIImageManager::GetImageAt(int i)
{ 
  // Get the image with number i of the internal image list

  if (i >= 0 && i < GetNImages()) {
    return (MImage *) m_ImageList->At(i);
  } else {
    Fatal("MGUIImageManager::GetImageAt(int i)",
          "Index (%d) out of bounds (%d, %d)", i, 0, GetNImages()-1);
    // this error shouldn't cause the program to crash, so we return :
    return new MImage();
  }
}


////////////////////////////////////////////////////////////////////////////////


void MGUIImageManager::RemoveImage(MImage* Image)
{ 
  // Remove the image Image from the internal image list

  if (m_ImageList->Contains(Image) == false) {
    Fatal("MGUIImageManager::RemoveImage(MImage* Image)",
          "Can't remove a Image not contained in the list!");
    return;
  }

  m_ImageList->Remove(Image);
}


////////////////////////////////////////////////////////////////////////////////


int MGUIImageManager::GetNImages()
{
  // return the number of images stored in the internal image list

  return m_ImageList->GetLast()+1;
}


////////////////////////////////////////////////////////////////////////////////


void MGUIImageManager::AddAndDisplay(MImage* Image)
{
  // Add an image to the container and display it in the activated viewport

  //AddImage(Image);

  m_ECanvas->GetCanvas()->Clear();
  Image->Display(m_ECanvas->GetCanvas());

  return;
}


////////////////////////////////////////////////////////////////////////////////



// MGUIImageManager: the end...
////////////////////////////////////////////////////////////////////////////////
