/*
 * MGUINormalizers.cxx
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
// MGUINormalizers
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MGUINormalizers.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MGUINormalizers)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUINormalizers::MGUINormalizers(const TGWindow* Parent, const TGWindow* Main, MSettingsImaging* Settings)
  : MGUIDialog(Parent, Main)
{
  // standard constructor

  m_Settings = Settings;

  // use hierarchical cleaning
  SetCleanup(kDeepCleanup);

  Create();
}


////////////////////////////////////////////////////////////////////////////////


MGUINormalizers::~MGUINormalizers()
{
  // kDeepCleanup is activated
}


////////////////////////////////////////////////////////////////////////////////


void MGUINormalizers::Create()
{
  // Create the main window
  
  // We start with a name and an icon...
  SetWindowName("Exposure / Sensitivity matrix / Efficiency --- same thing different names");  

  if (m_Settings->GetCoordinateSystem() == MCoordinateSystem::c_Spheric) {

    AddSubTitle("Far-field normalizers"); 

    TGLabel* NormalizersLabel = new TGLabel(this, "Normalizers not yet implemented");
    AddFrame(NormalizersLabel);
  } 
  
  else if (m_Settings->GetCoordinateSystem() == MCoordinateSystem::c_Cartesian2D ||
           m_Settings->GetCoordinateSystem() == MCoordinateSystem::c_Cartesian3D) {

    AddSubTitle("Near-field normalizers"); 
             
    TGLayoutHints* FileSelectorLayout = new TGLayoutHints(kLHintsLeft | kLHintsExpandX, m_FontScaler*(65+20), m_FontScaler*65, m_FontScaler*10, 0);
    TGLayoutHints* UseLayout = new TGLayoutHints(kLHintsExpandX | kLHintsTop | kLHintsLeft, m_FontScaler*65, m_FontScaler*65, m_FontScaler*10, m_FontScaler*5);


    m_UseNearFieldNormalizers = new TGCheckButton(this, "Use normalizers:", c_UseNearFieldNormalizers);
    AddFrame(m_UseNearFieldNormalizers, UseLayout);
    m_UseNearFieldNormalizers->Associate(this);
    m_UseNearFieldNormalizers->SetState((m_Settings->GetUseNearFieldNormalizers() == true) ?  kButtonDown : kButtonUp);

    
    m_DetectionEfficiency = new MGUIEFileSelector(this, "Detection efficiency (probability that a gamma ray emitted from image space bin x is detected):");
    m_DetectionEfficiency->SetFileType("Response file", "*.emittedxdetectedanywhere.rsp");
    m_DetectionEfficiency->SetFileType("Gzip'ed response file", "*.emittedxdetectedanywhere.rsp.gz");
    m_DetectionEfficiency->SetFileName(m_Settings->GetNearFieldDetectionEfficiency());

    AddFrame(m_DetectionEfficiency, FileSelectorLayout);
    
    m_ParameterEfficiency = new MGUIEFileSelector(this, "Parameter efficiency (probability that an event with parameters y is detected):");
    m_ParameterEfficiency->SetFileType("Response file", "*.emittedanywheredetectedy.rsp");
    m_ParameterEfficiency->SetFileType("Gzip'ed response file", "*.emittedanywheredetectedy.rsp.gz");
    m_ParameterEfficiency->SetFileName(m_Settings->GetNearFieldParameterEfficiency());

    AddFrame(m_ParameterEfficiency, FileSelectorLayout);
    
    if (m_UseNearFieldNormalizers->GetState() == kButtonUp) {
      m_DetectionEfficiency->SetEnabled(false);
      m_ParameterEfficiency->SetEnabled(false);
    } else {
      m_DetectionEfficiency->SetEnabled(true);
      m_ParameterEfficiency->SetEnabled(true);
    }
  }

  AddOKCancelButtons();

  PositionWindow(GetDefaultWidth(), GetDefaultHeight(), false);

  // and bring it to the screen.
  MapSubwindows();
  MapWindow();  

  Layout();
 
  return;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUINormalizers::ProcessMessage(long Message, long Parameter1, long Parameter2)
{
  // Process the messages for this application
  //cout<<"Messages: "<<Message<<", "<<Parameter1<<", "<<Parameter2<<endl;

  bool Status = true;

  switch (GET_MSG(Message)) {
  case kC_COMMAND:
    switch (GET_SUBMSG(Message)) {
    case kCM_BUTTON:
      switch (Parameter1) {
      case e_Ok:
        Status = OnOk();
        break;
        
      case e_Cancel:
        Status = OnCancel();
        break;

      case e_Apply:
        Status = OnApply();
        break;
        
      default:
        break;
      }
      break;
    case kCM_CHECKBUTTON:
      switch (Parameter1) {
      case c_UseNearFieldNormalizers:
        if (m_UseNearFieldNormalizers->GetState() == kButtonUp) {
          m_DetectionEfficiency->SetEnabled(false);
          m_ParameterEfficiency->SetEnabled(false);
        } else {
          m_DetectionEfficiency->SetEnabled(true);
          m_ParameterEfficiency->SetEnabled(true);
        }
        break;

      default:
        break; 
      }
      break;
      
    default:
      break;
    }
  default:
    break;
  }
  
  return Status;
}

////////////////////////////////////////////////////////////////////////////////


bool MGUINormalizers::OnApply()
{
  // The Apply button has been pressed

  if (m_Settings->GetUseNearFieldNormalizers() != ((m_UseNearFieldNormalizers->GetState() == kButtonDown) ? true : false)) {
    m_Settings->SetUseNearFieldNormalizers((m_UseNearFieldNormalizers->GetState() == kButtonDown) ? true : false);
  }

  if (m_Settings->GetNearFieldDetectionEfficiency() != m_DetectionEfficiency->GetFileName()) {
    m_Settings->SetNearFieldDetectionEfficiency(m_DetectionEfficiency->GetFileName());
  }
  
  if (m_Settings->GetNearFieldParameterEfficiency() != m_ParameterEfficiency->GetFileName()) {
    m_Settings->SetNearFieldParameterEfficiency(m_ParameterEfficiency->GetFileName());
  }

  return true;
}


// MGUINormalizers: the end...
////////////////////////////////////////////////////////////////////////////////
