/*
 * MGUIPolarization.cxx
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
// MGUIPolarization
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MGUIPolarization.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MStreams.h"
#include "MFile.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MGUIPolarization)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIPolarization::MGUIPolarization(const TGWindow* Parent, 
                                   const TGWindow* Main, 
                                   MSettingsMimrec* Data, 
                                   bool& OkPressed)
  : MGUIDialog(Parent, Main), m_OkPressed(OkPressed)
{
  // Construct an instance of MGUIPolarization and bring it to the screen

  m_Data = Data;
  m_OkPressed = false;

  // use hierarchical cleaning
  SetCleanup(kDeepCleanup);

  Create();
}


////////////////////////////////////////////////////////////////////////////////


MGUIPolarization::~MGUIPolarization()
{
  // Delete an instance of MGUIPolarization
}


////////////////////////////////////////////////////////////////////////////////


void MGUIPolarization::Create()
{
  // Create the main window

  // We start with a name and an icon...
  SetWindowName("Polarization");  
  AddSubTitle("Analyze the polarization of an azimuthal scatter distribution"); 

  TGLayoutHints* SingleLayout = new TGLayoutHints(kLHintsLeft | kLHintsExpandX, 20, 20, 5, 0);

  m_PolarizationFile = 
    new MGUIEFileSelector(this,
                          "File containing polarized data:",
                          m_Data->GetCurrentFileName());
  m_PolarizationFile->SetFileType("TRA", "*.tra");
  AddFrame(m_PolarizationFile, SingleLayout);

  m_BackgroundFile = 
    new MGUIEFileSelector(this,
                          "File containing unpolarized data:",
                          m_Data->GetPolarizationBackgroundFileName());
  m_BackgroundFile->SetFileType("TRA", "*.tra");
  AddFrame(m_BackgroundFile, SingleLayout);

	TGVerticalFrame* ButtonFrame = new TGVerticalFrame(this, 200, 150);
	TGLayoutHints* ButtonFrameLayout = 
		new TGLayoutHints(kLHintsTop | kLHintsCenterX, 
											5, 5, 30, 8);
	AddFrame(ButtonFrame, ButtonFrameLayout);
  

  m_Theta = new MGUIEEntry(ButtonFrame, "Theta [°]:      ", false, m_Data->GetTPTheta());
  ButtonFrame->AddFrame(m_Theta, SingleLayout);

  m_Phi = new MGUIEEntry(ButtonFrame, "Phi [°]:", false, m_Data->GetTPPhi());
  ButtonFrame->AddFrame(m_Phi, SingleLayout);

  m_Cut = new MGUIEEntry(ButtonFrame, "ARM cut [°]:", false, m_Data->GetPolarizationArmCut());
  ButtonFrame->AddFrame(m_Cut, SingleLayout);

  m_Bins = new MGUIEEntry(ButtonFrame, "Bins:", false, m_Data->GetHistBinsPolarization());
  ButtonFrame->AddFrame(m_Bins, SingleLayout);

  AddButtons();

  PositionWindow(GetDefaultWidth(), GetDefaultHeight(), false);

  // and bring it to the screen.
  MapSubwindows();
  MapWindow();  

  Layout();

  fClient->WaitFor(this);
 
  return;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIPolarization::OnApply()
{
	// The Apply button has been pressed

  // First test the data
  if (m_Theta->IsDouble(-90, 90) == false || 
      m_Phi->IsDouble(0, 180) == false ||
      m_Phi->IsDouble(0, 360) == false) {
    return false;
  }
  
  // Otherwise update the data:
  m_Data->SetTPTheta(m_Theta->GetAsDouble());
  m_Data->SetTPPhi(m_Phi->GetAsDouble());
  m_Data->SetPolarizationArmCut(m_Cut->GetAsDouble());
  m_Data->SetHistBinsPolarization(m_Bins->GetAsInt());
     
  if (MFile::Exists(m_PolarizationFile->GetFileName()) == false) {
    mgui<<"File "<<m_PolarizationFile->GetFileName()<<" does not exist!"<<endl;
    return false;
  }
     
  if (MFile::Exists(m_BackgroundFile->GetFileName()) == false) {
    mgui<<"File "<<m_BackgroundFile->GetFileName()<<" does not exist!"<<endl;
    return false;
  }

  m_Data->SetCurrentFileName(m_PolarizationFile->GetFileName());
  m_Data->SetPolarizationBackgroundFileName(m_BackgroundFile->GetFileName());

  m_OkPressed = true;

	return true;
}


// MGUIPolarization: the end...
////////////////////////////////////////////////////////////////////////////////
