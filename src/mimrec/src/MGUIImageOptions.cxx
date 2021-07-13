/*
 * MGUIImageOptions.cxx
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
// MGUIImageOptions
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MGUIImageOptions.h"

// Standard libs:

// ROOT libs:
#include <TGMsgBox.h>

// MEGAlib libs:
#include "MSystem.h"
#include "MImage.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MGUIImageOptions)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIImageOptions::MGUIImageOptions(const TGWindow* Parent, const TGWindow* Main, MSettingsImaging* Data)
  : MGUIDialog(Parent, Main)
{
  // standard constructor

  m_GUIData = Data;

  // use hierarchical cleaning
  SetCleanup(kDeepCleanup);

  Create();
}


////////////////////////////////////////////////////////////////////////////////


MGUIImageOptions::~MGUIImageOptions()
{
  // kDeepCleanup is activated
}


////////////////////////////////////////////////////////////////////////////////


void MGUIImageOptions::Create()
{
  // Create the main window

  // We start with a name and an icon...
  SetWindowName("Image drawing options");  

  AddSubTitle("Choose the image drawing options"); 

  TGLayoutHints* FrameLayout = new TGLayoutHints(kLHintsExpandX | kLHintsTop, 20*m_FontScaler, 20*m_FontScaler, 10*m_FontScaler, 10*m_FontScaler);
 
  TGLayoutHints* LeftLayout = new TGLayoutHints(kLHintsLeft | kLHintsTop, 0, 10*m_FontScaler, 0, 0);
  TGLayoutHints* RightLayout = new TGLayoutHints(kLHintsRight | kLHintsTop, 10*m_FontScaler, 0, 0, 0);


  TGHorizontalFrame* PaletteFrame = new TGHorizontalFrame(this);
  AddFrame(PaletteFrame, FrameLayout);

  TGLabel* PaletteLabel = new TGLabel(PaletteFrame, "Please choose a color palette:");
  PaletteFrame->AddFrame(PaletteLabel, LeftLayout);

  m_Palette = new TGComboBox(PaletteFrame);
  m_Palette->AddEntry("Rainbow", MImage::c_RootDefault);
  m_Palette->AddEntry("Black-White", MImage::c_BlackWhite);
  m_Palette->AddEntry("White-Black", MImage::c_WhiteBlack);
  m_Palette->AddEntry("Deep Sea", MImage::c_DeepSea);
  m_Palette->AddEntry("Sky line", MImage::c_SkyLine);
  m_Palette->AddEntry("Incendescent", MImage::c_Incendescent);
  m_Palette->AddEntry("White Red Black", MImage::c_WhiteRedBlack);
  m_Palette->AddEntry("Andreas' thesis", MImage::c_Thesis);
  m_Palette->AddEntry("Andreas' thesis - variability at low end", MImage::c_ThesisVarAtLow);
  m_Palette->AddEntry("Improved Rainbow", MImage::c_Rainbow);
  m_Palette->AddEntry("Bird", MImage::c_Bird);
  m_Palette->AddEntry("Viridis", MImage::c_Viridis);
  m_Palette->AddEntry("Cividis", MImage::c_Cividis);
  m_Palette->Select(m_GUIData->GetImagePalette());
  m_Palette->Associate(this);
  m_Palette->SetHeight(m_FontScaler*18);
  m_Palette->SetWidth(m_FontScaler*200);
  PaletteFrame->AddFrame(m_Palette, RightLayout);
  

  TGHorizontalFrame* DrawModeFrame = new TGHorizontalFrame(this);
  AddFrame(DrawModeFrame, FrameLayout);

  TGLabel* DrawModeLabel = new TGLabel(DrawModeFrame, "Please choose a drawing mode:");
  DrawModeFrame->AddFrame(DrawModeLabel, LeftLayout);

  m_DrawMode = new TGComboBox(DrawModeFrame);
  m_DrawMode->AddEntry("Boxes 2D", MImage::c_COL);
  m_DrawMode->AddEntry("Boxes 2D with palette", MImage::c_COLZ);
  m_DrawMode->AddEntry("Smooth contours 2D", MImage::c_COLCONT4);
  m_DrawMode->AddEntry("Smooth contours 2D with palette", MImage::c_COLCONT4Z);
  m_DrawMode->Associate(this);
  m_DrawMode->Select(m_GUIData->GetImageDrawMode());
  m_DrawMode->SetHeight(m_FontScaler*18);
  m_DrawMode->SetWidth(m_FontScaler*200);
  DrawModeFrame->AddFrame(m_DrawMode, RightLayout);


  TGLayoutHints* SourceCatalogLayout = new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsExpandX, 20*m_FontScaler, 20*m_FontScaler, 10*m_FontScaler, 10*m_FontScaler);

  m_SourceCatalog = new MGUIEFileSelector(this, "Choose a source catalog for images in Galactic coordinates (empty = use no catalog):", 
                                          m_GUIData->GetImageSourceCatalog());
  m_SourceCatalog->SetFileType("MEGAlib source catalog", "*.scat");
  AddFrame(m_SourceCatalog, SourceCatalogLayout);


  AddButtons();

  PositionWindow(GetDefaultWidth(), GetDefaultHeight(), false);

  // and bring it to the screen.
  MapSubwindows();
  MapWindow();  

  Layout();
 
  return;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIImageOptions::OnApply()
{
  // The Apply button has been pressed

  if (m_Palette->GetSelected() != m_GUIData->GetImagePalette()) {
    m_GUIData->SetImagePalette(m_Palette->GetSelected());
  }

  if (m_DrawMode->GetSelected() != m_GUIData->GetImageDrawMode()) {
    m_GUIData->SetImageDrawMode(m_DrawMode->GetSelected());
  }

  if (m_SourceCatalog->GetFileName() != m_GUIData->GetImageSourceCatalog()) {
    m_GUIData->SetImageSourceCatalog(m_SourceCatalog->GetFileName());
  }

  return true;
}


// MGUIImageOptions: the end...
////////////////////////////////////////////////////////////////////////////////
