/*
 * MGUIPosition.cxx
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
// MGUIPosition
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MGUIPosition.h"

// Standard libs:
#include <iostream>
#include <sstream>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MStreams.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MGUIPosition)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIPosition::MGUIPosition(const TGWindow* Parent, const TGWindow* Main, 
                           MSettingsGeomega* Data, bool& OkPressed)
  : MGUIDialog(Parent, Main), m_OkPressed(OkPressed)
{
  // Construct an instance of MGUIPosition and bring it to the screen

  m_Data = Data;

  // use hierarchical cleaning
  SetCleanup(kDeepCleanup);

  m_OkPressed = false;

  Create();
}


////////////////////////////////////////////////////////////////////////////////


MGUIPosition::~MGUIPosition()
{
  // kDeepCleanup is activated
}


////////////////////////////////////////////////////////////////////////////////


void MGUIPosition::Create()
{
  // Create the main window

  // We start with a name and an icon...
  SetWindowName("Volume information");  
  AddSubTitle("Please enter a position in cm"); 

  m_PositionLayout = new TGLayoutHints(kLHintsExpandX | kLHintsTop, 20, 20, 2, 2);

  m_Position = new MGUIEEntryList(this, "Position [cm]:", MGUIEEntryList::c_SingleLine);
  m_Position->Add("", m_Data->GetPosition().X(), true);
  m_Position->Add("", m_Data->GetPosition().Y(), true);
  m_Position->Add("", m_Data->GetPosition().Z(), true);
  m_Position->Create();
  AddFrame(m_Position, m_PositionLayout);

  m_PositionString = new MGUIEEntry(this, "Position as String \"x; y; z\" [cm]:", false, "");
  m_PositionString->SetEntryFieldSize(275);
  AddFrame(m_PositionString, m_PositionLayout);

  AddButtons();
  PositionWindow(GetDefaultWidth(), GetDefaultHeight());

  // and bring it to the screen.
  MapSubwindows();
  MapWindow();  

  Layout();

  fClient->WaitFor(this);
 
  return;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIPosition::OnApply()
{
  // The Apply button has been pressed

  MVector V;
  if (m_PositionString->GetAsString() != "") {
    istringstream input(m_PositionString->GetAsString().Data());
    char ch;
    double x, y, z;
    
    input>>x;
    if (!input.good()) {
      mgui<<"Incorrectly formated input: x"<<error;
      return false;
    }
    
    ch = '\0';
    input>>ch;
    if (!input.good()) {
      mgui<<"Incorrectly formated input: general error"<<error;
      return false;
    } else if ((ch != ';' && ch != ',') || !input.good()) {
      mgui<<"Incorrectly formated input: first semicolon = "<<ch<<error;
      return false;
    }
    
    input>>y;
    if (!input.good()) {
      mgui<<"Incorrectly formated input: y"<<error;
      return false;
    }
    
    ch = '\0';
    input>>ch;
    if (!input.good()) {
      mgui<<"Incorrectly formated input: general error"<<error;
      return false;
    } else if ((ch != ';' && ch != ',') || !input.good()) {
      mgui<<"Incorrectly formated input: second semikolon = "<<ch<<error;
      return false;
    }

    input>>z;
    if (!input.eof()) {
      mgui<<"Incorrectly formated input: z"<<error;
      return false;
    }
    
    V.SetXYZ(x, y, z);
  } else {
    V.SetXYZ(m_Position->GetAsDouble(0),
             m_Position->GetAsDouble(1), 
             m_Position->GetAsDouble(2));
  }
  
  m_Data->SetPosition(V);

  m_OkPressed = true; 

  return true;
}


// MGUIPosition: the end...
////////////////////////////////////////////////////////////////////////////////
