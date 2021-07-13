/*
 * MGUIEImageContainer.cxx
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
// MGUIEImageContainer
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MGUIEImageContainer.h"

// Standard libs:
using namespace std;
#include <iostream>

// ROOT libs:
#include <TObjArray.h>
#include <TCanvas.h>
#include <TPad.h>
#include <TTime.h>
#include <TSystem.h>

// MEGAlib libs:
#include "MGUIEImage.h"
#include "MImage.h"
#include "MStreams.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MGUIEImageContainer)
#endif


//////////////////////////////////////////////////////////////////////////////


int MGUIEImageContainer::m_CanvasCounter = 0;


//////////////////////////////////////////////////////////////////////////////


MGUIEImageContainer::MGUIEImageContainer(const TGWindow *Parent, MString Title, bool DisplayHeader) : 
  MGUIElement(Parent)
{
  // Construct an instance of MGUIEImageContainer

  m_Title = Title;
  m_DisplayHeader = DisplayHeader;

  m_Container = new TObjArray();
  m_FrameContainer = new TObjArray();

  if (m_DisplayHeader == true) {
    m_TitleFrame = new TGCompositeFrame(this, 100, 16, kHorizontalFrame);
    m_TitleFrameLayout = new TGLayoutHints(kLHintsExpandX, 0, 0, 0, 0);

    m_TitleLabel = new TGLabel(m_TitleFrame, m_Title);
    m_TitleLabelLayout = new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 10, 0, 0, 0);

    m_ButtonLayout = new TGLayoutHints(kLHintsTop | kLHintsRight, 0, 0, 0, 0);
    m_PrintButton = new TGPictureButton(m_TitleFrame, fClient->GetPicture("resource/icons/printer_xs.xpm"), 1);
  }


  m_MatrixLayout = 
    new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX | kLHintsCenterX | kLHintsCenterY |
                      kLHintsExpandY, 0, 0, 0, 0);

  m_NRows = -1;
  m_NColumns = -1;
  m_ArrangeLR = true;
}


////////////////////////////////////////////////////////////////////////////////


void MGUIEImageContainer::SetRowsAndColumns(int NRows, int NColumns, bool ArrangeLR)
{
  // Set the number of rows and columns of the matrix-like layout
  // Values containing -1 are adjusted by the class

  m_NRows = NRows;
  m_NColumns = NColumns;
  m_ArrangeLR = ArrangeLR;
}


////////////////////////////////////////////////////////////////////////////////


MGUIEImageContainer::~MGUIEImageContainer()
{
  // Delete this instance of MGUIEImageContainer

  cout<<"Should I delete the MGUIEImages????"<<endl;
  if (MustCleanup() == kNoCleanup) {
    delete m_Container;
  
    delete m_PrintButton;
    delete m_ButtonLayout;
    
    delete m_MatrixLayout;
  }
}

////////////////////////////////////////////////////////////////////////////////


void MGUIEImageContainer::Create()
{
  // Create the GUI element

  Arrange();

  MapSubwindows();
  MapWindow();  
  Layout();
}


////////////////////////////////////////////////////////////////////////////////


void MGUIEImageContainer::Arrange()
{
  // Arrange the images:

  // Step one: Remove all frames
  if (fList != 0) {

    TGFrameElement *el;
    TIter next(fList);
  
    while ((el = (TGFrameElement *) next())) {
      fList->Remove(el);
      delete el;
    }
  }


  // Step two: Add the buttons:
  if (m_DisplayHeader == true) {
    AddFrame(m_TitleFrame, m_TitleFrameLayout);
    m_TitleFrame->AddFrame(m_TitleLabel, m_TitleLabelLayout);
    m_TitleFrame->AddFrame(m_PrintButton, m_ButtonLayout);
    m_PrintButton->Associate(this);
  }
  if (m_ArrangeLR == true) {
    m_MatrixFrame = new TGCompositeFrame(this, 100, 100, kVerticalFrame);
  } else {
    m_MatrixFrame = new TGCompositeFrame(this, 100, 100, kHorizontalFrame);
  }
  AddFrame(m_MatrixFrame, m_MatrixLayout);


  // Step three: Add the frames in the wished (matrix-like) fashion:
  int NColumns = 0, NRows = 0;

  //cout<<"Start: R/C: "<<m_NRows<<"!"<<m_NColumns<<endl;
  
  // Matrix case:
  if (m_NRows == -1 && m_NColumns == -1) {
    //cout<<"Case: -1/-1"<<endl;
    NColumns = (int) (sqrt((double) GetNImages())); // (double) : Only for gcc 3.3 SUSE exp
    if (NColumns < sqrt((double) GetNImages())) {
      NColumns++;
    }
    if (GetNImages() <= NColumns*NColumns - NColumns) {
      NRows = NColumns-1;
    } else {
      NRows = NColumns;
    }
  } else if (m_NRows == -1 && m_NColumns != -1) {
    //cout<<"Case: -1/?"<<endl;
    NColumns = m_NColumns;
    if (GetNImages() % m_NColumns == 0) {
      NRows = GetNImages() % m_NColumns;
    } else {
      NRows = GetNImages() / m_NColumns + 1;
    }
  } else if (m_NRows != -1 && m_NColumns == -1) {
    //cout<<"Case: ?/-1"<<endl;
    NRows = m_NRows;
    if (GetNImages() % m_NRows == 0) {
      NColumns = GetNImages() % m_NRows;
    } else {
      NColumns = GetNImages() / m_NRows +1;
    }
  } else {
    NRows = m_NRows;
    NColumns = m_NColumns;
  }  


  int c, r, i = 0;
  TGCompositeFrame *H;

  if (m_ArrangeLR == true) {
    
    // Fill rows:
    for (r = 0; r < NRows; r++) {
      if (i >= GetNImages()) {
        break; 
      }
      H = new TGHorizontalFrame(m_MatrixFrame, 100, 100, kHorizontalFrame);
      m_FrameContainer->AddLast(H);
      m_MatrixFrame->AddFrame(H, m_MatrixLayout);
      // Fill columns:
      for (c = 0; c < NColumns; c++) {
        if (i < GetNImages()) {
          H->AddFrame(new MGUIEImage(H, GetImageAt(i)), m_MatrixLayout);
          i++;
        } else {
          break;
        }
      }
    }
  } else {

    // Fill Columns:
    for (c = 0; c < NColumns; c++) {
      if (i >= GetNImages()) {
        break; 
      }
      H = new TGCompositeFrame(m_MatrixFrame, 100, 100, kVerticalFrame);
      m_FrameContainer->AddLast(H);
      m_MatrixFrame->AddFrame(H, m_MatrixLayout);
      // Fill rows:
      for (r = 0; r < NRows; r++) {
        if (i < GetNImages()) {
          //cout<<(int) GetImageAt(i)<<"!"<<i<<endl;
          H->AddFrame(new MGUIEImage(H, GetImageAt(i)), m_MatrixLayout);
          i++;
        } else {
          H->AddFrame(new MGUIEImage(H, 0), m_MatrixLayout);
          i++;
          //break;
        }
      }
    }
  }

  MapSubwindows();
  MapWindow();  
  Layout();

  return;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIEImageContainer::ProcessMessage(long Message, long Parameter1, 
                                  long Parameter2)
{
  // Process the messages for this application

  switch (GET_MSG(Message)) {
  case kC_COMMAND:
    switch (GET_SUBMSG(Message)) {
    case kCM_BUTTON:
      switch (Parameter1) {
      case 1: // Print-Button
        Print();
        break;
        
      default:
        break;
      }
    }
  }
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MGUIEImageContainer::AddImage(MImage *Image)
{
  // Add an image to the container but do not display it unless Create() has
  // been called

  m_Container->AddLast(Image);
}


////////////////////////////////////////////////////////////////////////////////


int MGUIEImageContainer::GetNImages()
{
  // Return the number of stored images

  return m_Container->GetLast() + 1;
}


////////////////////////////////////////////////////////////////////////////////


MImage* MGUIEImageContainer::GetImageAt(int i)
{
  // Return the image at position i

  if (i >= 0 && i < GetNImages()) {
    return (MImage *) (m_Container->At(i));
  } else {
    Fatal("MGUIEImageContainer::GetImageAt",
          "Index out of bounds (min = 0; max = %d; this = %d)", GetNImages(), i);
    return 0;
  }
}


////////////////////////////////////////////////////////////////////////////////


void MGUIEImageContainer::Print()
{

  // Create a canvas all stuff is printed to

  char Text[1000];
  sprintf(Text, "Printable Canvas - %i", MGUIEImageContainer::m_CanvasCounter++);

  TCanvas *C = new TCanvas(Text, Text, 3200, 2400);

  int NColumns = 0, NRows = 0;
  
  // Devide the canvas:
  // Matrix case:
  if (m_NRows == -1 && m_NColumns == -1) {
    //cout<<"Case: -1/-1"<<endl;
    NColumns = (int) (sqrt((double) GetNImages()));  // (double) : Only for gcc 3.3 SUSE exp
    if (NColumns < sqrt((double) GetNImages())) {
      NColumns++;
    }
    if (GetNImages() <= NColumns*NColumns - NColumns) {
      NRows = NColumns-1;
    } else {
      NRows = NColumns;
    }
  } else if (m_NRows == -1 && m_NColumns != -1) {
    //cout<<"Case: -1/?"<<endl;
    NColumns = m_NColumns;
    if (GetNImages() % m_NColumns == 0) {
      NRows = GetNImages() % m_NColumns;
    } else {
      NRows = GetNImages() / m_NColumns + 1;
    }
  } else if (m_NRows != -1 && m_NColumns == -1) {
    //cout<<"Case: ?/-1"<<endl;
    NRows = m_NRows;
    if (GetNImages() % m_NRows == 0) {
      NColumns = GetNImages() % m_NRows;
    } else {
      NColumns = GetNImages() / m_NRows +1;
    }
  } else {
    NRows = m_NRows;
    NColumns = m_NColumns;
  }  

  // 
  C->Divide(NColumns, NRows, 0, 0);
  int i = 0, r, c;
  for (r = 0; r < NRows; r++) {
    for (c = 0; c < NColumns; c++) {
      C->cd(r*NColumns + c + 1);

      if (i < GetNImages()) {
        GetImageAt(i)->DrawCopy();
        i++;
      } else {
        break;
      }
    }
  }

  TDatime T;
  MString S = T.AsSQLString();
  S.ReplaceAll(" ", "_");

  ostringstream FileName;
  FileName<<m_Title<<"."<<S<<".eps";
  C->Print(FileName.str().c_str());

  //delete C;
}


////////////////////////////////////////////////////////////////////////////////


void MGUIEImageContainer::Reset()
{
  // Reset all images to their default values

  int i;
  for (i = 0; i < GetNImages(); i++) {
    GetImageAt(i)->Reset();
  }
}


// MGUIEImageContainer.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
