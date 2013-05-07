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
#include "MProjection.h"
#include "MGUIEEntry.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MGUIARM)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIARM::MGUIARM(const TGWindow* p, const TGWindow* main, MSettingsMimrec* Data, 
                 MARMTYPE ID, bool& OkPressed)
  : MGUIDialog(p, main), m_OkPressed(OkPressed)
{
  // standard constructor

  m_GUIData = Data;
  m_ID = ID;
  m_OkPressed = false;

  if (m_ID < MARMTYPE_GAMMA || m_ID > MARMTYPE_ENERGY) {
    Fatal("void MGUIARM::Create()",
          "Unkown identifier: %d", m_ID);
  }
  
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

  // There are two GUI types, which use almost the same GUI:
  // The ARM for scattered gamma and the ARM for recoil electrons

  switch (m_ID) {
  case MARMTYPE_GAMMA:
    SetWindowName("ARM of scattered gamma-ray"); 
    break;
  case MARMTYPE_ELECTRON:
    SetWindowName("ARM of recoil electron");  
    break;
  case MARMTYPE_ENERGY:
    SetWindowName("Energy spectra");  
    break;
  default:
    break;
  }

  switch (m_ID) {
  case MARMTYPE_GAMMA:
  case MARMTYPE_ELECTRON:
    AddSubTitle("Please enter the position from where you want to measure\n"
                "the ARM and the length of the interval around the peak");
    break;
  case MARMTYPE_ENERGY:
    AddSubTitle("Please enter the position from where you want to measure\n"
                "the energy spectra and the length of the interval around the position");
    break;
  default:
    break;
  }

  m_SelectorLayout = new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsExpandX, 100, 100, 2, 2);

  if (m_GUIData->GetCoordinateSystem() == MProjection::c_Spheric) {
    m_ThetaIsX = new MGUIEEntry(this, "Theta [deg]:", false, m_GUIData->GetTPTheta());
    m_PhiIsY = new MGUIEEntry(this, "Phi [deg]:", false, m_GUIData->GetTPPhi());

    switch (m_ID) {
    case MARMTYPE_GAMMA:
      m_Distance = new MGUIEEntry(this, "Radius [deg]:", false, m_GUIData->GetTPDistanceTrans());
      m_Bins = new MGUIEEntry(this, "Bins:", false, m_GUIData->GetHistBinsARMGamma());
      break;
    case MARMTYPE_ELECTRON:
      m_Distance = new MGUIEEntry(this, "Radius [deg]:", false, m_GUIData->GetTPDistanceLong());
      m_Bins = new MGUIEEntry(this, "Bins:", false, m_GUIData->GetHistBinsARMElectron());
      break;
    case MARMTYPE_ENERGY:
      m_Distance = new MGUIEEntry(this, "Radius [deg]:", false, m_GUIData->GetTPDistanceTrans());
      m_Bins = new MGUIEEntry(this, "Bins:", false, m_GUIData->GetHistBinsSpectrum());
      break;
    default:
      break;
    }

    AddFrame(m_ThetaIsX, m_SelectorLayout);
    AddFrame(m_PhiIsY, m_SelectorLayout);
    AddFrame(m_Distance, m_SelectorLayout);
    AddFrame(m_Bins, m_SelectorLayout);
    
  } else if (m_GUIData->GetCoordinateSystem() == MProjection::c_Galactic) {
    m_ThetaIsX = new MGUIEEntry(this, "Latitude [deg]:", false, m_GUIData->GetTPGalLatitude());
    m_PhiIsY = new MGUIEEntry(this, "Longitude [deg]:", false, m_GUIData->GetTPGalLongitude());

    switch (m_ID) {
    case MARMTYPE_GAMMA:
      m_Distance = new MGUIEEntry(this, "Radius [deg]:", false, m_GUIData->GetTPDistanceTrans());
      m_Bins = new MGUIEEntry(this, "Bins:", false, m_GUIData->GetHistBinsARMGamma());
      break;
    case MARMTYPE_ELECTRON:
      m_Distance = new MGUIEEntry(this, "Radius [deg]:", false, m_GUIData->GetTPDistanceLong());
      m_Bins = new MGUIEEntry(this, "Bins:", false, m_GUIData->GetHistBinsARMElectron());
      break;
    case MARMTYPE_ENERGY:
      m_Distance = new MGUIEEntry(this, "Radius [deg]:", false, m_GUIData->GetTPDistanceTrans());
      m_Bins = new MGUIEEntry(this, "Bins:", false, m_GUIData->GetHistBinsSpectrum());
      break;
    default:
      break;
    }

    AddFrame(m_ThetaIsX, m_SelectorLayout);
    AddFrame(m_PhiIsY, m_SelectorLayout);
    AddFrame(m_Distance, m_SelectorLayout);
    AddFrame(m_Bins, m_SelectorLayout);
    
  } else if (m_GUIData->GetCoordinateSystem() == MProjection::c_Cartesian2D ||
             m_GUIData->GetCoordinateSystem() == MProjection::c_Cartesian3D) {
    m_ThetaIsX = new MGUIEEntry(this, "x [cm]:", false, m_GUIData->GetTPX());
    m_PhiIsY = new MGUIEEntry(this, "y [cm]:", false, m_GUIData->GetTPY());
    m_RadiusIsZ = new MGUIEEntry(this, "z [cm]:", false, m_GUIData->GetTPZ());

    switch (m_ID) {
    case MARMTYPE_GAMMA:
      m_Distance = new MGUIEEntry(this, "Radius [deg]:", false, m_GUIData->GetTPDistanceTrans());
      m_Bins = new MGUIEEntry(this, "Bins:", false, m_GUIData->GetHistBinsARMGamma());
      break;
    case MARMTYPE_ELECTRON:
      m_Distance = new MGUIEEntry(this, "Radius [deg]:", false, m_GUIData->GetTPDistanceLong());
      m_Bins = new MGUIEEntry(this, "Bins:", false, m_GUIData->GetHistBinsARMElectron());
      break;
    case MARMTYPE_ENERGY:
      m_Distance = new MGUIEEntry(this, "Radius [deg]:", false, m_GUIData->GetTPDistanceTrans());
      m_Bins = new MGUIEEntry(this, "Bins:", false, m_GUIData->GetHistBinsSpectrum());
      break;
    default:
      break;
    }

    AddFrame(m_ThetaIsX, m_SelectorLayout);
    AddFrame(m_PhiIsY, m_SelectorLayout);
    AddFrame(m_RadiusIsZ, m_SelectorLayout);
    AddFrame(m_Distance, m_SelectorLayout);
    AddFrame(m_Bins, m_SelectorLayout);
  } else {
    merr<<"Unknown coordinate system ID: "<<m_GUIData->GetCoordinateSystem()<<fatal;
  }

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


bool MGUIARM::OnApply()
{

  // First test the data (m_RadiusIsZ has not to be checked!)
  if (m_GUIData->GetCoordinateSystem() == MProjection::c_Spheric) {
    if (m_ThetaIsX->IsDouble(0, 180) == false || 
        m_PhiIsY->IsDouble(-360, 360) == false || 
        m_Distance->IsDouble(-180, 180) == false) {
      return false;
    }
  } else if (m_GUIData->GetCoordinateSystem() == MProjection::c_Galactic) {
    if (m_ThetaIsX->IsDouble(-90, 90) == false || 
        m_PhiIsY->IsDouble(-360, 360) == false || 
        m_Distance->IsDouble(-180, 180) == false) {
      return false;
    }
  } else if (m_GUIData->GetCoordinateSystem() == MProjection::c_Cartesian2D ||
             m_GUIData->GetCoordinateSystem() == MProjection::c_Cartesian3D) {
    if (m_Distance->IsDouble(-180, 180) == false) {
      return false;
    }
  }
  
  // Otherwise update the data:
  if (m_GUIData->GetCoordinateSystem() == MProjection::c_Spheric) {
    m_GUIData->SetTPTheta(m_ThetaIsX->GetAsDouble());
    m_GUIData->SetTPPhi(m_PhiIsY->GetAsDouble());

    switch (m_ID) {
    case MARMTYPE_GAMMA:
      m_GUIData->SetTPDistanceTrans(m_Distance->GetAsDouble());
      m_GUIData->SetHistBinsARMGamma(m_Bins->GetAsInt());
      break;
    case MARMTYPE_ELECTRON:
      m_GUIData->SetTPDistanceLong(m_Distance->GetAsDouble());
      m_GUIData->SetHistBinsARMElectron(m_Bins->GetAsInt());
      break;
    case MARMTYPE_ENERGY:
      m_GUIData->SetTPDistanceTrans(m_Distance->GetAsDouble());
      m_GUIData->SetHistBinsSpetrum(m_Bins->GetAsInt());
      break;
    default:
      break;
    }
  } else if (m_GUIData->GetCoordinateSystem() == MProjection::c_Galactic) {
    m_GUIData->SetTPGalLatitude(m_ThetaIsX->GetAsDouble());
    m_GUIData->SetTPGalLongitude(m_PhiIsY->GetAsDouble());

    switch (m_ID) {
    case MARMTYPE_GAMMA:
      m_GUIData->SetTPDistanceTrans(m_Distance->GetAsDouble());
      m_GUIData->SetHistBinsARMGamma(m_Bins->GetAsInt());
      break;
    case MARMTYPE_ELECTRON:
      m_GUIData->SetTPDistanceLong(m_Distance->GetAsDouble());
      m_GUIData->SetHistBinsARMElectron(m_Bins->GetAsInt());
      break;
    case MARMTYPE_ENERGY:
      m_GUIData->SetTPDistanceTrans(m_Distance->GetAsDouble());
      m_GUIData->SetHistBinsSpetrum(m_Bins->GetAsInt());
      break;
    default:
      break;
    }
  } else if (m_GUIData->GetCoordinateSystem() == MProjection::c_Cartesian2D ||
             m_GUIData->GetCoordinateSystem() == MProjection::c_Cartesian3D) {
    m_GUIData->SetTPX(m_ThetaIsX->GetAsDouble());
    m_GUIData->SetTPY(m_PhiIsY->GetAsDouble());
    m_GUIData->SetTPZ(m_RadiusIsZ->GetAsDouble());

    switch (m_ID) {
    case MARMTYPE_GAMMA:
      m_GUIData->SetTPDistanceTrans(m_Distance->GetAsDouble());
      m_GUIData->SetHistBinsARMGamma(m_Bins->GetAsInt());
      break;
    case MARMTYPE_ELECTRON:
      m_GUIData->SetTPDistanceLong(m_Distance->GetAsDouble());
      m_GUIData->SetHistBinsARMElectron(m_Bins->GetAsInt());
      break;
    case MARMTYPE_ENERGY:
      m_GUIData->SetTPDistanceTrans(m_Distance->GetAsDouble());
      m_GUIData->SetHistBinsSpetrum(m_Bins->GetAsInt());
      break;
    default:
      break;
    }
  } else {
    merr<<"Unknown coordinate system ID: "<<m_GUIData->GetCoordinateSystem()<<fatal;
  }

  m_OkPressed = true;
	
  return true;
}


// MGUIARM: the end...
////////////////////////////////////////////////////////////////////////////////
