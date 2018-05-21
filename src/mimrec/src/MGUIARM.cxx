/*
 * MGUIARM.cxx
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
// MGUIARM
//
//
// Dialog box, which provides entry-boxes for ARM-parameter (theta, phi, radius)
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MGUIARM.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MCoordinateSystem.h"
#include "MGUIEEntry.h"
#include "MGUIDefaults.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MGUIARM)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIARM::MGUIARM(const TGWindow* p, const TGWindow* main, MSettingsMimrec* Data, 
                 MGUIARMModes ID, bool& OkPressed)
  : MGUIDialog(p, main), m_OkPressed(OkPressed)
{
  // standard constructor

  m_Settings = Data;
  m_Mode = ID;
  m_OkPressed = false;
  
  m_UseLog = nullptr;
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


MGUIARM::~MGUIARM()
{
  // standard destructor --- deep clean up activated!
}


////////////////////////////////////////////////////////////////////////////////


void MGUIARM::Create()
{
  // Create the main window

  double Scaler = MGUIDefaults::GetInstance()->GetFontScaler();
  
  
  // There are two GUI types, which use almost the same GUI:
  // The ARM for scattered gamma and the ARM for recoil electrons
  if (m_Mode == MGUIARMModes::m_ARMGamma) {
    SetWindowName("ARM of Scattered Gamma Ray"); 
    AddSubTitle("Choose the histogram binning and\nthe source position with acceptance window");
  } else if (m_Mode == MGUIARMModes::m_ARMElectron) {
    SetWindowName("ARM of Recoil Electron");  
    AddSubTitle("Choose the histogram binning and\nthe source position with acceptance window");
  } else if (m_Mode == MGUIARMModes::m_Spectrum) {
    SetWindowName("Energy Spectrum");  
    AddSubTitle("Choose the histogram binning and\nan optional source position with acceptance window");
  }

  
  // All histogram types have the number of bins
  if (m_Mode == MGUIARMModes::m_ARMGamma) {
    m_Bins = new MGUIEEntry(this, "Number of bins in histogram:", false, m_Settings->GetHistBinsARMGamma());
  } else if (m_Mode == MGUIARMModes::m_ARMElectron) {
    m_Bins = new MGUIEEntry(this, "Number of bins in histogram:", false, m_Settings->GetHistBinsARMElectron());
  } else if (m_Mode == MGUIARMModes::m_Spectrum) {
    m_Bins = new MGUIEEntry(this, "Number of bins in histogram:", false, m_Settings->GetHistBinsSpectrum());
  }
  TGLayoutHints* BinsLayout = new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsExpandX, 50*Scaler, 50*Scaler, 2*Scaler, 10*Scaler);
  AddFrame(m_Bins, BinsLayout);

  TGLayoutHints* SelectorLayout = new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsExpandX, 50*Scaler, 50*Scaler, 2*Scaler, 2*Scaler);
  TGLayoutHints* SelectorSubLayout = nullptr;
  
  // In spectral mode we have a checkbutton to choose if we want the test position
  if (m_Mode == MGUIARMModes::m_Spectrum) {
    // The two radio buttons to choose if we want to use a test position
    m_UseLog = new TGCheckButton(this, "Use logarithmic binning", m_UseLogID);
    m_UseLog->SetWrapLength(300*Scaler);
    m_UseLog->Associate(this);
    m_UseLog->SetState(m_Settings->GetLogBinningSpectrum() ? kButtonDown : kButtonUp);
    AddFrame(m_UseLog, SelectorLayout);

    m_UseTestPosition = new TGCheckButton(this, "Use source position and acceptance window (only Compton and pair events will be displayed)", m_UseTestPositionID);
    m_UseTestPosition->SetWrapLength(300*Scaler);
    m_UseTestPosition->Associate(this);
    m_UseTestPosition->SetState(m_Settings->GetTPUse() ? kButtonDown : kButtonUp);
    AddFrame(m_UseTestPosition, SelectorLayout);

    SelectorSubLayout = new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsExpandX, (50+20)*Scaler, 50*Scaler, 2*Scaler, 2*Scaler);    
  } else {
    
    TGLabel* TestPositionLabel = new TGLabel(this, "Source position and acceptance window:");
    TGLayoutHints* TestPositionLayout = new TGLayoutHints(kLHintsLeft | kLHintsTop, 50*Scaler, 50*Scaler, 10*Scaler, 2*Scaler);
    AddFrame(TestPositionLabel, TestPositionLayout);
    
    SelectorSubLayout = new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsExpandX, (50+20)*Scaler, 50*Scaler, 2*Scaler, 2*Scaler);
  }
 
  if (m_Settings->GetCoordinateSystem() == MCoordinateSystem::c_Spheric) {
    
    m_ThetaIsX = new MGUIEEntry(this, "Theta [deg]:", false, m_Settings->GetTPTheta());
    AddFrame(m_ThetaIsX, SelectorSubLayout);
    m_PhiIsY = new MGUIEEntry(this, "Phi [deg]:", false, m_Settings->GetTPPhi());
    AddFrame(m_PhiIsY, SelectorSubLayout);

    
  } else if (m_Settings->GetCoordinateSystem() == MCoordinateSystem::c_Galactic) {
    
    m_ThetaIsX = new MGUIEEntry(this, "Latitude [deg]:", false, m_Settings->GetTPGalLatitude());
    AddFrame(m_ThetaIsX, SelectorSubLayout);
    m_PhiIsY = new MGUIEEntry(this, "Longitude [deg]:", false, m_Settings->GetTPGalLongitude());
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

  if (m_Mode == MGUIARMModes::m_ARMGamma) {
    m_Distance = new MGUIEEntry(this, "Acceptance radius [deg]:", false, m_Settings->GetTPDistanceTrans());
  } else if (m_Mode == MGUIARMModes::m_ARMElectron) {
    m_Distance = new MGUIEEntry(this, "Acceptance radius [deg]:", false, m_Settings->GetTPDistanceLong());
  } else if (m_Mode == MGUIARMModes::m_Spectrum) {
    m_Distance = new MGUIEEntry(this, "Acceptance radius [deg]:", false, m_Settings->GetTPDistanceTrans());
  } 
  AddFrame(m_Distance, SelectorSubLayout);
  
  
  AddButtons();

  if (m_Mode == MGUIARMModes::m_Spectrum && m_UseTestPosition->GetState() == kButtonUp) {
    if (m_ThetaIsX != nullptr) m_ThetaIsX->SetEnabled(false);
    if (m_PhiIsY != nullptr) m_PhiIsY->SetEnabled(false);
    if (m_RadiusIsZ != nullptr) m_RadiusIsZ->SetEnabled(false);
    if (m_Distance != nullptr) m_Distance->SetEnabled(false);  
  }
  
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


bool MGUIARM::ProcessMessage(long Message, long Parameter1, 
                                        long Parameter2)
{
  // Process the messages for this application

  if (GET_MSG(Message) == kC_COMMAND) {
    if (GET_SUBMSG(Message) == kCM_CHECKBUTTON) {
      if (Parameter1 == m_UseTestPositionID) {
        if (m_Mode == MGUIARMModes::m_Spectrum) {
          if (m_UseTestPosition->GetState() == kButtonUp) {
            if (m_ThetaIsX != nullptr) m_ThetaIsX->SetEnabled(false);
            if (m_PhiIsY != nullptr) m_PhiIsY->SetEnabled(false);
            if (m_RadiusIsZ != nullptr) m_RadiusIsZ->SetEnabled(false);
            if (m_Distance != nullptr) m_Distance->SetEnabled(false);  
          } else {
            if (m_ThetaIsX != nullptr) m_ThetaIsX->SetEnabled(true);
            if (m_PhiIsY != nullptr) m_PhiIsY->SetEnabled(true);
            if (m_RadiusIsZ != nullptr) m_RadiusIsZ->SetEnabled(true);
            if (m_Distance != nullptr) m_Distance->SetEnabled(true);  
          }
        }
      }
    }
  }
  
  return MGUIDialog::ProcessMessage(Message, Parameter1, Parameter2);
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIARM::OnApply()
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
  
  if (m_UseLog != nullptr) {
    m_Settings->SetLogBinningSpectrum(m_UseLog->GetState() == kButtonDown ? true : false);
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

  if (m_Mode == MGUIARMModes::m_ARMGamma) {
    m_Settings->SetTPDistanceTrans(m_Distance->GetAsDouble());
    m_Settings->SetHistBinsARMGamma(m_Bins->GetAsInt());
  } else if (m_Mode == MGUIARMModes::m_ARMElectron) {
    m_Settings->SetTPDistanceLong(m_Distance->GetAsDouble());
    m_Settings->SetHistBinsARMElectron(m_Bins->GetAsInt());
  } else if (m_Mode == MGUIARMModes::m_Spectrum) {
    m_Settings->SetTPDistanceTrans(m_Distance->GetAsDouble());
    m_Settings->SetHistBinsSpetrum(m_Bins->GetAsInt());
  }
  
  m_OkPressed = true;
  
  return true;
}


// MGUIARM: the end...
////////////////////////////////////////////////////////////////////////////////
