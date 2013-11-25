/*
 * MGUIETime.cxx
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
// MGUIETime
//
//
// This class is an elementary GUI-widget:
// It contains a text-label and a input-field.
// It can be checked, if the input is within a preselected range of values.
//
//
// Example:
//
// 
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MGUIETime.h"

// Standard libs:
#include "MStreams.h"
#include <time.h>

// ROOT libs:
#include <TGMsgBox.h>
#include <TDatime.h>

// MEGAlib libs:
#include "MTime.h"


////////////////////////////////////////////////////////////////////////////////


MGUIETime::MGUIETime(const TGWindow* Parent) :
  MGUIElement(Parent)
{
  // Creates a frame containing a label and an entry-box 
  //
  // Parent:   parent Window, where this frame is contained
  // Label:    text of the label
  // Value:    default entry

  Create();
}


////////////////////////////////////////////////////////////////////////////////


MGUIETime::~MGUIETime()
{
  // Destruct this instance of MGUIETime
}


////////////////////////////////////////////////////////////////////////////////


void MGUIETime::Create()
{
  // Create the label and the input-field.

  int i; 
  char Text[5];
  MTime Time;

  m_ComboLayout = new TGLayoutHints(kLHintsLeft | kLHintsTop, 1, 1, 0, 0);
  m_LabelLayout =
    new TGLayoutHints(kLHintsLeft | kLHintsTop, 1, 1, 0, 0);

  // Hours:
  m_Hours = new TGComboBox(this, c_Hour);
  for (i = 0; i < 24; i++) {
    sprintf(Text, "%i", i);
    m_Hours->AddEntry(Text, i);
  }
  m_Hours->Resize(40, 20);
  m_Hours->Select(Time.GetHours());
  AddFrame(m_Hours, m_ComboLayout);

  // Colon:
  m_LabelColon = new TGLabel(this, ":");
  AddFrame(m_LabelColon, m_LabelLayout);

  // Minutes:
  m_Minutes = new TGComboBox(this, c_Minute);
  m_Minutes->AddEntry("00", 0);
  m_Minutes->AddEntry("05", 1);
  for (i = 2; i < 12; i++) {
    sprintf(Text, "%i", i*5);
    m_Minutes->AddEntry(Text, i);
  }
  m_Minutes->Select(Time.GetMinutes()/5);
  m_Minutes->Resize(40, 20);
  AddFrame(m_Minutes, m_ComboLayout);

  // At:
  m_LabelAt = new TGLabel(this, "at");
  AddFrame(m_LabelAt, m_LabelLayout);

  // Days:
  m_Days = new TGComboBox(this, c_Day);
  for (i = 1; i < 32; i++) {
    sprintf(Text, "%i", i);
    m_Days->AddEntry(Text, i);
  }
  m_Days->Select(Time.GetDays());
  m_Days->Resize(40, 20);
  AddFrame(m_Days, m_ComboLayout);

  // Months:
  m_Months = new TGComboBox(this, c_Month);
  m_Months->AddEntry("Jan", 1);
  m_Months->AddEntry("Feb", 2);
  m_Months->AddEntry("Mar", 3);
  m_Months->AddEntry("Apr", 4);
  m_Months->AddEntry("Mai", 5);
  m_Months->AddEntry("Jun", 6);
  m_Months->AddEntry("Jul", 7);
  m_Months->AddEntry("Aug", 8);
  m_Months->AddEntry("Sep", 9);
  m_Months->AddEntry("Oct", 10);
  m_Months->AddEntry("Nov", 11);
  m_Months->AddEntry("Dec", 12);
  m_Months->Resize(50, 20);
  m_Months->Select(Time.GetMonths());
  AddFrame(m_Months, m_ComboLayout);

  // Years:
  m_Years = new TGComboBox(this, c_Year);
  for (i = 2000; i < 2021; i+=1) {
    sprintf(Text, "%i", i);
    m_Years->AddEntry(Text, i);
  }
  m_Years->Select(Time.GetYears());
  m_Years->Resize(55, 20);
  AddFrame(m_Years, m_ComboLayout);


  return;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIETime::ProcessMessage(long Message, long Parameter1, 
                               long Parameter2)
{
  // Process the messages for this application

  MTime Time;

  switch (GET_MSG(Message)) {
  case kC_COMMAND:
    switch (GET_SUBMSG(Message)) {
    case kCM_COMBOBOX:
      switch (Parameter1) { // Combo-ID
      case c_Minute:
      case c_Hour:
      case c_Day:
      case c_Month:
      case c_Year:
        // Validate the date:

      default:
        break;
      }
      break;
    default:
      break;
    }
    break;
  default:
    break;
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


MString MGUIETime::GetAsString()
{
  // Return the value entered in the input-field.

  return "Not implemented";
}



// MGUIETime.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
