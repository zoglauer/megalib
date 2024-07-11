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
#include "MGUIDefaults.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MStreams.h"
#include "MFile.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
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

  double Scaler = MGUIDefaults::GetInstance()->GetFontScaler();
  
  // We start with a name and an icon...
  SetWindowName("Polarization");  
  AddSubTitle("Analyze the polarization of an azimuthal scatter distribution"); 

  TGLayoutHints* SingleLayout = new TGLayoutHints(kLHintsLeft | kLHintsExpandX, 20*Scaler, 20*Scaler, 5*Scaler, 0*Scaler);
  TGLayoutHints* CommentLayout = new TGLayoutHints(kLHintsLeft, 20*Scaler, 20*Scaler, 2*Scaler, 0*Scaler);

  m_PolarizationFile = 
    new MGUIEFileSelector(this,
                          "File containing the polarized data:",
                          m_Data->GetCurrentFileName());
  m_PolarizationFile->SetFileType("TRA", "*.tra");
  AddFrame(m_PolarizationFile, SingleLayout);

  m_BackgroundFile = 
    new MGUIEFileSelector(this,
                          "File containing unpolarized data for geometry correction:",
                          m_Data->GetPolarizationBackgroundFileName());
  m_BackgroundFile->SetFileType("TRA", "*.tra");
  AddFrame(m_BackgroundFile, SingleLayout);

  TGVerticalFrame* ButtonFrame = new TGVerticalFrame(this, 200*Scaler, 150*Scaler);
  TGLayoutHints* ButtonFrameLayout = 
    new TGLayoutHints(kLHintsTop | kLHintsCenterX, 
                      5*Scaler, 5*Scaler, 30*Scaler, 8*Scaler);
  AddFrame(ButtonFrame, ButtonFrameLayout);
  

  if (m_Data->GetCoordinateSystem() == MCoordinateSystem::c_Spheric) {

    m_ThetaIsX = new MGUIEEntry(ButtonFrame, "Theta [deg]:", false, m_Data->GetTPTheta());
    ButtonFrame->AddFrame(m_ThetaIsX, SingleLayout);
    m_PhiIsY = new MGUIEEntry(ButtonFrame, "Phi [deg]:", false, m_Data->GetTPPhi());
    ButtonFrame->AddFrame(m_PhiIsY, SingleLayout);

  } else if (m_Data->GetCoordinateSystem() == MCoordinateSystem::c_Galactic) {

    m_ThetaIsX = new MGUIEEntry(ButtonFrame, "Latitude [deg]:", false, m_Data->GetTPGalLatitude());
    ButtonFrame->AddFrame(m_ThetaIsX, SingleLayout);
    m_PhiIsY = new MGUIEEntry(ButtonFrame, "Longitude [deg]:", false, m_Data->GetTPGalLongitude());
    ButtonFrame->AddFrame(m_PhiIsY, SingleLayout);

  } else if (m_Data->GetCoordinateSystem() == MCoordinateSystem::c_Cartesian2D ||
             m_Data->GetCoordinateSystem() == MCoordinateSystem::c_Cartesian3D) {

    m_ThetaIsX = new MGUIEEntry(ButtonFrame, "x [cm]:", false, m_Data->GetTPX());
    ButtonFrame->AddFrame(m_ThetaIsX, SingleLayout);
    m_PhiIsY = new MGUIEEntry(ButtonFrame, "y [cm]:", false, m_Data->GetTPY());
    ButtonFrame->AddFrame(m_PhiIsY, SingleLayout);
    m_RadiusIsZ = new MGUIEEntry(ButtonFrame, "z [cm]:", false, m_Data->GetTPZ());
    ButtonFrame->AddFrame(m_RadiusIsZ, SingleLayout);

  }



  m_Cut = new MGUIEEntry(ButtonFrame, "ARM cut [deg]:", false, m_Data->GetPolarizationArmCut());
  ButtonFrame->AddFrame(m_Cut, SingleLayout);

  m_Bins = new MGUIEEntry(ButtonFrame, "Bins:", false, m_Data->GetHistBinsPolarization());
  ButtonFrame->AddFrame(m_Bins, SingleLayout);

  TGLabel* Comment1 = new TGLabel(this, "Remarks:");
  AddFrame(Comment1, CommentLayout);
  TGLabel* Comment2 = new TGLabel(this, "(1) The zero angle of the azimuthal scatter angle distribution will be always in the direction of the given phi angle.");
  AddFrame(Comment2, CommentLayout);
  TGLabel* Comment3 = new TGLabel(this, "(2) There is no time selection on the unpolarized data, since it is assumed it is aquired at different times or via simulations.");
  AddFrame(Comment3, CommentLayout);
  TGLabel* Comment4 = new TGLabel(this, "(3) Uncertainty calculations do not consider low-count rate regimes");
  AddFrame(Comment4, CommentLayout);


  //Comment->SetWrapLength(400*Scaler);

  
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

  // First test the data (m_RadiusIsZ has not to be checked!)
  if (m_Data->GetCoordinateSystem() == MCoordinateSystem::c_Spheric) {
    if (m_ThetaIsX->IsDouble(0, 180) == false ||
        m_PhiIsY->IsDouble(-360, 360) == false) {
      return false;
    }
  } else if (m_Data->GetCoordinateSystem() == MCoordinateSystem::c_Galactic) {
    if (m_ThetaIsX->IsDouble(-90, 90) == false ||
        m_PhiIsY->IsDouble(-360, 360) == false) {
      return false;
    }
  }
  
  // Otherwise update the data:
  // Otherwise update the data:
  if (m_Data->GetCoordinateSystem() == MCoordinateSystem::c_Spheric) {
    m_Data->SetTPTheta(m_ThetaIsX->GetAsDouble());
    m_Data->SetTPPhi(m_PhiIsY->GetAsDouble());
  } else if (m_Data->GetCoordinateSystem() == MCoordinateSystem::c_Galactic) {
    m_Data->SetTPGalLatitude(m_ThetaIsX->GetAsDouble());
    m_Data->SetTPGalLongitude(m_PhiIsY->GetAsDouble());
  } else if (m_Data->GetCoordinateSystem() == MCoordinateSystem::c_Cartesian2D ||
             m_Data->GetCoordinateSystem() == MCoordinateSystem::c_Cartesian3D) {
    m_Data->SetTPX(m_ThetaIsX->GetAsDouble());
    m_Data->SetTPY(m_PhiIsY->GetAsDouble());
    m_Data->SetTPZ(m_RadiusIsZ->GetAsDouble());
  }
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
