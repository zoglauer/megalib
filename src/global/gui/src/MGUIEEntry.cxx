/*
 * MGUIEEntry.cxx
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
// MGUIEEntry
//
//
// This class is an elementary GUI-widget:
// It contains a text-label and a input-field.
// It can be checked, if the input is within a preselected range of values.
//
// If the user changes the content the entry the message
// kC_ENTRY - kET_CHANGED - m_Id is sent to the message window,
// which must have been set via Associate(...) before.
// 
// Example:
//
// m_Layout = new TGLayoutHints(kLHintsExpandX | kLHintsTop, 20, 20, 7, 0);
// m_Isotopes = new MGUIEEntry(this,
//                             MString("Source Name:"),
//                             m_Data->GetRSSourceName());
// m_Isotopes->SetEntryFieldSize(150);
// m_Isotopes->Associate(this, e_IsotopesId);
// AddFrame(m_Isotopes, m_Layout);
//
// in ProcessMessage(...):
//
// mlog<<m_Isotopes->GetAsString()<<endl;;
//
//
// 
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MGUIEEntry.h"

// Standard libs:

// ROOT libs:
#include <TGMsgBox.h>

// MEGAlib libs:
#include "MAssert.h"
#include "MStreams.h"


////////////////////////////////////////////////////////////////////////////////


MGUIEEntry::MGUIEEntry(const TGWindow* Parent, MString Label, bool Emphasize, MString Value) :
  MGUIElement(Parent, kHorizontalFrame)
{
  // Creates a frame containing a label and an entry-box 
  //
  // Parent:   parent Window, where this frame is contained
  // Label:    text of the label
  // Value:    default entry

  m_IsEmphasized = Emphasize;

  m_Label = Label;
  m_Value = Value;
  m_Type = e_String;
  m_Limits = false;

  Init();
}


////////////////////////////////////////////////////////////////////////////////


MGUIEEntry::MGUIEEntry(const TGWindow* Parent, MString Label, bool Emphasize, 
                       double Value, bool Limits, double Min, double Max) :
  MGUIElement(Parent, kHorizontalFrame)
{
  // Creates a frame containing a label and an entry-box  
  //
  // Parent:   parent Window, where this frame is contained
  // Label:    text of the label
  // Value:    default entry

  m_IsEmphasized = Emphasize;

  m_Label = Label;
	m_ValueAsDouble = Value;
  m_Value = MakeSmartString(Value);
  m_Type = e_Double;
  m_Limits = Limits;
  m_Min = Min;
  m_Max = Max;

  Init();
}


////////////////////////////////////////////////////////////////////////////////


MGUIEEntry::MGUIEEntry(const TGWindow* Parent, MString Label, bool Emphasize, 
                       int Value, bool Limits, int Min, int Max) :
  MGUIElement(Parent, kHorizontalFrame)
{
  // Creates a frame containing a label and an entry-box  
  //
  // Parent:   parent Window, where this frame is contained
  // Label:    text of the label
  // Value:    default entry

  m_IsEmphasized = Emphasize;

  m_Label = Label;
	m_ValueAsDouble = Value;
  m_Value = MakeSmartString(Value);
  m_Type = e_Integer;
  m_Limits = Limits;
  m_Min = Min;
  m_Max = Max;

  Init();
}


////////////////////////////////////////////////////////////////////////////////


MGUIEEntry::~MGUIEEntry()
{
  // Destruct this instance of MGUIEEntry

  if (MustCleanup() == kNoCleanup) {
    delete m_TextLabel;
    delete m_TextLabelLayout;
    
    delete m_InputLayout;
    delete m_Input;
    delete m_NumberInput;
  }
}



////////////////////////////////////////////////////////////////////////////////


void MGUIEEntry::Init()
{
  // Initilizations common to all constructors:

  m_Input = 0;
  m_NumberInput = 0;
  
  m_MessageWindow = 0;
  m_Id = -1;

  m_IsEnabled = true;

  m_Size = 85;

  m_IsModified = false;

  Create();
}


////////////////////////////////////////////////////////////////////////////////


void MGUIEEntry::Associate(TGCompositeFrame* w, int Id)
{
  // Set a window to which all messages are sent

  m_MessageWindow = w;
  m_Id = Id;
}


////////////////////////////////////////////////////////////////////////////////


void MGUIEEntry::Create()
{
  // Create the label and the input-field.

  // Label:
  m_TextLabel = new TGLabel(this, new TGString(m_Label));
  if (m_IsEmphasized == true) {
    m_TextLabel->SetTextFont(m_EmphasizedFont);
  }    
  m_TextLabelLayout =
    new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsCenterY, 0, 0, 0, 0);
  AddFrame(m_TextLabel, m_TextLabelLayout);


  // Input field:
  m_InputLayout = new TGLayoutHints(kLHintsRight | kLHintsTop | kLHintsCenterY, 5, 0, 0, 0);
  if (m_Limits == false) {
    m_InputBuffer = new TGTextBuffer(10);
    m_InputBuffer->AddText(0, m_Value);
    m_Input = new TGTextEntry(this, m_InputBuffer);
    m_Input->Resize(m_Size, m_Input->GetDefaultHeight());
    m_Input->Associate(this);
    AddFrame(m_Input, m_InputLayout);
		m_Input->Layout();
  } else {
		if (m_Type == e_Integer) {
		  m_NumberInput = new TGNumberEntry(this, m_ValueAsDouble, 9, -1, TGNumberFormat::kNESInteger);
		} else {
		  m_NumberInput = new TGNumberEntry(this, m_ValueAsDouble, 12, -1, TGNumberFormat::kNESReal);
		}
		m_NumberInput->SetLimits(TGNumberFormat::kNELLimitMinMax, m_Min, m_Max);
		m_NumberInput->Associate(this);
    m_NumberInput->Resize(m_Size, m_NumberInput->GetDefaultHeight());
    m_NumberInput->Associate(this);
    AddFrame(m_NumberInput, m_InputLayout);
		m_NumberInput->Layout();
  }

  // Give this element the default size of its content:
  Resize(GetDefaultWidth(), GetDefaultHeight()); 

  m_IsModified = false;

  return;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIEEntry::ProcessMessage(long Message, long Parameter1, 
                                  long Parameter2)
{
  // Process the messages for this application, mainly the scollbar moves:
  
  // cout<<"Entry Msg: "<<GET_MSG(Message)<<"!"<<GET_SUBMSG(Message)<<"!"<<Parameter1<<"!"<<Parameter2<<endl;

  switch (GET_MSG(Message)) {
  case kC_TEXTENTRY:
    switch (GET_SUBMSG(Message)) {
    case kTE_TEXTCHANGED:
    case kTE_ENTER:
    case kTE_TAB:
      if (m_MessageWindow != 0) {
        SendMessage(m_MessageWindow, MK_MSG((EWidgetMessageTypes) kC_ENTRY, 
                                            (EWidgetMessageTypes) kET_CHANGED), m_Id, 0);
      }
      m_IsModified = true;
      //cout<<"Modified: "<<m_Label<<": "<<m_NumberInput->GetNumber()<<endl;
      break;
    default:
      break;
    }
  default:
    break;
  }
  
  return true;
}



////////////////////////////////////////////////////////////////////////////////


void MGUIEEntry::SetValue(double Value)
{
	m_ValueAsDouble = Value;
  m_Value = MakeSmartString(Value);

  if (m_Limits == false) {
    m_Input->SetText(m_Value);
  } else {
    m_NumberInput->SetNumber(m_ValueAsDouble);
  }
  Layout();
  m_IsModified = false;
}


////////////////////////////////////////////////////////////////////////////////


void MGUIEEntry::SetValue(int Value)
{
	m_ValueAsDouble = Value;
  m_Value = MakeSmartString(Value);

  if (m_Limits == false) {
    m_Input->SetText(m_Value);
  } else {
    m_NumberInput->SetNumber(m_ValueAsDouble);
  }
  Layout();
  m_IsModified = false;
}


////////////////////////////////////////////////////////////////////////////////


void MGUIEEntry::SetValue(unsigned int Value)
{
	m_ValueAsDouble = double(Value);
  m_Value = MakeSmartString(Value);

  if (m_Limits == false) {
    m_Input->SetText(m_Value);
  } else {
    m_NumberInput->SetNumber(m_ValueAsDouble);
  }
  Layout();
  m_IsModified = false;
}


////////////////////////////////////////////////////////////////////////////////


void MGUIEEntry::SetValue(MString Value)
{
  m_Value = Value; 

  if (m_Limits == false) {
    m_Input->SetText(m_Value);
  } else {
    m_NumberInput->SetNumber(m_ValueAsDouble);
  }
  Layout();
  m_IsModified = false;
}


////////////////////////////////////////////////////////////////////////////////


void MGUIEEntry::SetEntryFieldSize(int Size)
{
  // Set the size of the entry field

  m_Size = Size;

	if (m_Limits == false) {
		m_Input->Resize(m_Size, m_Input->GetDefaultHeight());
		m_Input->Layout();
	} else {
		m_NumberInput->Resize(m_Size, m_NumberInput->GetDefaultHeight());
		m_NumberInput->Layout();
	}
}


////////////////////////////////////////////////////////////////////////////////


void MGUIEEntry::SetEnabled(bool flag)
{
	m_IsEnabled = flag;
	
	if (m_Limits == false) {
		m_Input->SetEnabled(m_IsEnabled);
	} else {
		m_NumberInput->SetState(m_IsEnabled);
	}
}


////////////////////////////////////////////////////////////////////////////////


MString MGUIEEntry::GetAsString()
{
  // Return the value entered in the input-field.

  return m_Input->GetText();
}


////////////////////////////////////////////////////////////////////////////////


double MGUIEEntry::GetAsDouble()
{
  // Return the value entered in the input-field. Its recommended, to check the
  // type and range before!!

  if (m_Type != e_Double) {
    merr<<m_Label<<": Internal data type is not double!"<<endl;
    massert(false);
  }

	if (m_Limits == false) {
		return ToNumber(m_Input->GetText());
	} else {
		return m_NumberInput->GetNumber();
	}
}


////////////////////////////////////////////////////////////////////////////////


int MGUIEEntry::GetAsInt()
{
  // Return the value entered in the input-field. Its recommended, to check the
  // type and range before!!

  if (m_Type != e_Integer) {
    merr<<m_Label<<": Internal data type is not integer!"<<endl;
    massert(false);
  }

	if (m_Limits == false) {
		return (int) ToNumber(m_Input->GetText());	
	} else {
		return m_NumberInput->GetIntNumber();
	}
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIEEntry::IsInt(int Min, int Max)
{
  // Check if the input-field contains an integer and that it is within the range

  ostringstream S;

	if (m_Limits == true) {
		if (m_Type == e_Integer) {
			return CheckRange(m_NumberInput->GetNumber(), Min, Max);
		} else {
			S<<"Error in input field of \""<<m_Label<<"\""<<endl
			 <<"Field does not contain an integer!";
			new TGMsgBox(gClient->GetRoot(), GetParent(), "Type error", 
									 S.str().c_str(), 
									 kMBIconStop, kMBOk);
			return false;
		}
	}

  // Check if it is a number:
  if (CheckForNumber(m_Input->GetText()) == false) {
    S<<"Error in input field of \""<<m_Label<<"\""<<endl
     <<"Field does not contain a number!";
    new TGMsgBox(gClient->GetRoot(), GetParent(), "Type error", 
                 S.str().c_str(), 
                 kMBIconStop, kMBOk);
    return false;
  }
 
  // Check if it is an integer:
  if (atoi(m_Input->GetText()) != atof(m_Input->GetText())) {
    S<<"Error in input field of \""<<m_Label<<"\""<<endl
     <<"Field does not contain an integer!";
    new TGMsgBox(gClient->GetRoot(), GetParent(), "Type error", 
                 S.str().c_str(), 
                 kMBIconStop, kMBOk);
    return false;
  }

  // Check if its within the range
  return CheckRange(ToNumber(m_Input->GetText()), Min, Max);
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIEEntry::IsDouble(double Min, double Max)
{
  // Check if the input-field contains a double and that it is within the range

  ostringstream S;

	if (m_Limits == true) {
		if (m_Type == e_Double || e_Integer) {
			return CheckRange(m_NumberInput->GetNumber(), Min, Max);
		} else {
			S<<"Error in input field of \""<<m_Label<<"\""<<endl
			 <<"Field does not contain a double!";
			new TGMsgBox(gClient->GetRoot(), GetParent(), "Type error", 
									 S.str().c_str(), 
									 kMBIconStop, kMBOk);
			return false;
		}
	}

  // Check if it is a number:
  if (CheckForNumber(m_Input->GetText()) == false) {
    S<<"Error in input field of \""<<m_Label<<"\""<<endl
     <<"Field does not contain a number!";
    new TGMsgBox(gClient->GetRoot(), GetParent(), "Type error", 
                 S.str().c_str(), 
                 kMBIconStop, kMBOk);
    return false;
  }

  // Check if its within the range
  return CheckRange(ToNumber(m_Input->GetText()), Min, Max);
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIEEntry::CheckRange(double Value, double Min, double Max)
{
  // Check if the input-field contains numbers, and that they are within [Min;Max]

  ostringstream S;

  if (Value < Min) {
    S<<"Error in input field of \""<<m_Label<<"\""<<endl
     <<"Value \""<<Value<<"\" is below its minimum value \""<<Min<<"\"";
    new TGMsgBox(gClient->GetRoot(), GetParent(), "Limit error", 
                 S.str().c_str(), 
                 kMBIconStop, kMBOk);
    return false;
  }

  if (Value > Max) {
    S<<"Error in input field of \""<<m_Label<<"\""<<endl
     <<"Value \""<<Value<<"\" is above its maximum value \""<<Max<<"\"";
    new TGMsgBox(gClient->GetRoot(), GetParent(), "Limit error", 
                 S.str().c_str(), 
                 kMBIconStop, kMBOk);
    return false;
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


MString MGUIEEntry::MakeSmartString(double Number)
{
  // Make a smart string, i.e. cut of zeros, etc.

  ostringstream out;
  out.precision(12); // Not too high to avoid the round-off errors
  out<<Number;
  
  MString S(out);
  S.StripBack('0');
  
  return S;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIEEntry::CheckForNumber(MString Number)
{
  // Return true if Number is a ... number!

  Number.ReplaceAll(".", "");
  Number.ReplaceAll("E", "");
  Number.ReplaceAll("e", "");
  Number.ReplaceAll("+", "");
  Number.ReplaceAll("-", "");

  if (Number.IsEmpty() == true) {
    return false;
  }

  Number.ReplaceAll("1", "");
  Number.ReplaceAll("2", "");
  Number.ReplaceAll("3", "");
  Number.ReplaceAll("4", "");
  Number.ReplaceAll("5", "");
  Number.ReplaceAll("6", "");
  Number.ReplaceAll("7", "");
  Number.ReplaceAll("8", "");
  Number.ReplaceAll("9", "");
  Number.ReplaceAll("0", "");

  return Number.IsEmpty();
}


////////////////////////////////////////////////////////////////////////////////


double MGUIEEntry::ToNumber(MString Number) 
{
  // Convert a string to a number

  return atof(Number);
}


// MGUIEEntry.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
