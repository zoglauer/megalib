/*
 * MGUISignificance.cxx
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
// MGUISignificance
//
//
// Dialog box, which provides entry-boxes for Significance-parameter (theta, phi, radius)
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MGUISignificance.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MProjection.h"
#include "MGUIEEntry.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MGUISignificance)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUISignificance::MGUISignificance(const TGWindow* p, const TGWindow* main, MSettingsImaging* Data, 
                 bool& OkPressed)
  : MGUIDialog(p, main), m_OkPressed(OkPressed)
{
  // standard constructor

  m_GUIData = Data;
  m_OkPressed = false;

  // use hierarchical cleaning
  SetCleanup(kDeepCleanup);

  Create();
}


////////////////////////////////////////////////////////////////////////////////


MGUISignificance::~MGUISignificance()
{
  // standard destructor --- deep clean up activated!
}


////////////////////////////////////////////////////////////////////////////////


void MGUISignificance::Create()
{
  // Create the main window

  // There are two GUI types, which use almost the same GUI:
  // The Significance for scattered gamma and the Significance for recoil electrons

  SetWindowName("Significance map via distant test positions"); 

  AddSubTitle("Please enter the size of the radius around the test positions\n"
        "and the distance between source and test positions");

  m_SelectorLayout = new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsExpandX, 100, 100, 2, 2);
  
  m_Radius = new MGUIEEntry(this, "Radius around source and test positions (e.g. 1/2 FWHM  of ARM) [deg]:", false, m_GUIData->GetSignificanceMapRadius(), true, 0.0, 180.0);
  AddFrame(m_Radius, m_SelectorLayout);

  m_Distance = new MGUIEEntry(this, "Distance between source and test position [deg]:", false, m_GUIData->GetSignificanceMapDistance(), true, 0.0, 180.0);
  AddFrame(m_Distance, m_SelectorLayout);

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


bool MGUISignificance::OnApply()
{
  // First test the data (m_RadiusIsZ has not to be checked!)
  if (m_Radius->IsDouble(0, 180) == false ||
      m_Distance->IsDouble(0,180) == false) {
    return false;
  }
  
  m_GUIData->SetSignificanceMapDistance(m_Distance->GetAsDouble());
  m_GUIData->SetSignificanceMapRadius(m_Radius->GetAsDouble());

  m_OkPressed = true;
  
  return true;
}


// MGUISignificance: the end...
////////////////////////////////////////////////////////////////////////////////
