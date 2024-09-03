/*
 * MGUIARMExtended.cxx
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
// MGUIARMExtended
//
//
// Dialog box, which provides entry-boxes for ARM-parameter (theta, phi, radius)
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MGUIARMExtended.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MCoordinateSystem.h"
#include "MGUIEEntry.h"
#include "MGUIDefaults.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MGUIARMExtended)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIARMExtended::MGUIARMExtended(const TGWindow* p, const TGWindow* main, MSettingsMimrec* Data, bool& OkPressed)
  : MGUIDialog(p, main), m_OkPressed(OkPressed)
{
  // standard constructor

  m_Settings = Data;
  m_OkPressed = false;
  
  m_UseTestPosition = nullptr;  
  
  m_ThetaIsX = nullptr;
  m_PhiIsY = nullptr;
  m_RadiusIsZ = nullptr;
  m_Distance = nullptr;
  
  // use hierarchical cleaning
  SetCleanup(kDeepCleanup);

  Create();
}


////////////////////////////////////////////////////////////////////////////////


MGUIARMExtended::~MGUIARMExtended()
{
  // standard destructor --- deep clean up activated!
}


////////////////////////////////////////////////////////////////////////////////


void MGUIARMExtended::Create()
{
  // Create the main window

  double Scaler = MGUIDefaults::GetInstance()->GetFontScaler();
  
  SetWindowName("Angular Resolution Measure");
  AddSubTitle("Choose the fitting and histogram binning options for creating the\nAngular Resolution Measure (ARM) for Compton events");


  TGLabel* TestPositionLabel = new TGLabel(this, "Choose the source location and the ARM acceptance window:");
  TGLayoutHints* TestPositionLayout = new TGLayoutHints(kLHintsLeft | kLHintsTop, 50*Scaler, 50*Scaler, 10*Scaler, 2*Scaler);
  AddFrame(TestPositionLabel, TestPositionLayout);
    
  TGLayoutHints* SelectorLayout = new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsExpandX, 50*Scaler, 50*Scaler, 2*Scaler, 2*Scaler);
  TGLayoutHints* SelectorSubLayout = new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsExpandX, (50+20)*Scaler, 50*Scaler, 2*Scaler, 2*Scaler);
 
  if (m_Settings->GetCoordinateSystem() == MCoordinateSystem::c_Spheric) {
    
    m_ThetaIsX = new MGUIEEntry(this, "Theta (0 - 180 deg):", false, m_Settings->GetTPTheta());
    AddFrame(m_ThetaIsX, SelectorSubLayout);
    m_PhiIsY = new MGUIEEntry(this, "Phi (-180 - 360 deg):", false, m_Settings->GetTPPhi());
    AddFrame(m_PhiIsY, SelectorSubLayout);

    
  } else if (m_Settings->GetCoordinateSystem() == MCoordinateSystem::c_Galactic) {
    
    m_ThetaIsX = new MGUIEEntry(this, "Latitude (-90 - 90 deg):", false, m_Settings->GetTPGalLatitude());
    AddFrame(m_ThetaIsX, SelectorSubLayout);
    m_PhiIsY = new MGUIEEntry(this, "Longitude (0 - 360 deg):", false, m_Settings->GetTPGalLongitude());
    AddFrame(m_PhiIsY, SelectorSubLayout);

    
  } else if (m_Settings->GetCoordinateSystem() == MCoordinateSystem::c_Cartesian2D ||
             m_Settings->GetCoordinateSystem() == MCoordinateSystem::c_Cartesian3D) {

    m_ThetaIsX = new MGUIEEntry(this, "x [cm]:", false, m_Settings->GetTPX());
    AddFrame(m_ThetaIsX, SelectorSubLayout);
    m_PhiIsY = new MGUIEEntry(this, "y [cm]:", false, m_Settings->GetTPY());
    AddFrame(m_PhiIsY, SelectorSubLayout);
    m_RadiusIsZ = new MGUIEEntry(this, "z [cm]:", false, m_Settings->GetTPZ());
    AddFrame(m_RadiusIsZ, SelectorSubLayout);

  }

  m_Distance = new MGUIEEntry(this, "Acceptance radius (0 - 180 deg):", false, m_Settings->GetTPDistanceTrans());
  AddFrame(m_Distance, SelectorSubLayout);
  
  // All histogram types have the number of bins
  m_Bins = new MGUIEEntry(this, "Number of bins in histogram (odd numbers preferred):", false, m_Settings->GetHistBinsARMGamma());
  TGLayoutHints* BinsLayout = new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsExpandX, 50*Scaler, 50*Scaler, 10*Scaler, 2*Scaler);
  AddFrame(m_Bins, BinsLayout);




  AddButtons();
  
  // Let's resize and position the window, 
  PositionWindow(GetDefaultWidth(), GetDefaultHeight(), false);
  
  // and bring it to the screen.
  MapSubwindows();
  MapWindow();  
  Layout();

  fClient->WaitFor(this);
 
  return;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIARMExtended::ProcessMessage(long Message, long Parameter1,
                                        long Parameter2)
{
  // Process the messages for this application
  
  return MGUIDialog::ProcessMessage(Message, Parameter1, Parameter2);
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIARMExtended::OnApply()
{
  // First test the data (m_RadiusIsZ has not to be checked!)
  if (m_Settings->GetCoordinateSystem() == MCoordinateSystem::c_Spheric) {
    if (m_ThetaIsX->IsDouble(0, 180) == false || 
        m_PhiIsY->IsDouble(-360, 360) == false || 
        m_Distance->IsDouble(-180, 180) == false) {
      return false;
    }
  } else if (m_Settings->GetCoordinateSystem() == MCoordinateSystem::c_Galactic) {
    if (m_ThetaIsX->IsDouble(-90, 90) == false || 
        m_PhiIsY->IsDouble(-360, 360) == false || 
        m_Distance->IsDouble(-180, 180) == false) {
      return false;
    }
  } else if (m_Settings->GetCoordinateSystem() == MCoordinateSystem::c_Cartesian2D ||
             m_Settings->GetCoordinateSystem() == MCoordinateSystem::c_Cartesian3D) {
    if (m_Distance->IsDouble(-180, 180) == false) {
      return false;
    }
  }
  
  if (m_UseTestPosition != nullptr) {
    m_Settings->SetTPUse(m_UseTestPosition->GetState() == kButtonDown ? true : false);
  }
  
  // Otherwise update the data:
  if (m_Settings->GetCoordinateSystem() == MCoordinateSystem::c_Spheric) {
    m_Settings->SetTPTheta(m_ThetaIsX->GetAsDouble());
    m_Settings->SetTPPhi(m_PhiIsY->GetAsDouble());
  } else if (m_Settings->GetCoordinateSystem() == MCoordinateSystem::c_Galactic) {
    m_Settings->SetTPGalLatitude(m_ThetaIsX->GetAsDouble());
    m_Settings->SetTPGalLongitude(m_PhiIsY->GetAsDouble());
  } else if (m_Settings->GetCoordinateSystem() == MCoordinateSystem::c_Cartesian2D ||
             m_Settings->GetCoordinateSystem() == MCoordinateSystem::c_Cartesian3D) {
    m_Settings->SetTPX(m_ThetaIsX->GetAsDouble());
    m_Settings->SetTPY(m_PhiIsY->GetAsDouble());
    m_Settings->SetTPZ(m_RadiusIsZ->GetAsDouble());
  }

  m_Settings->SetTPDistanceTrans(m_Distance->GetAsDouble());
  m_Settings->SetHistBinsARMGamma(m_Bins->GetAsInt());
  
  m_OkPressed = true;
  
  return true;
}


// MGUIARMExtended: the end...
////////////////////////////////////////////////////////////////////////////////
