/*
 * MGUIEMinMaxEntry.cxx
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
// MGUIEMinMaxEntry
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MGUIEMinMaxEntry.h"

// Standard libs:
#include <cstdlib>
#include <cctype>
using namespace std;

// ROOT libs:
#include <TGMsgBox.h>

// MEGAlib libs:
#include "MStreams.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MGUIEMinMaxEntry)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIEMinMaxEntry::MGUIEMinMaxEntry(const TGWindow *Parent, MString Label, bool Emphasize,
																		 double MinValue, double MaxValue, 
																		 bool Limits, double Min, double Max) :
  MGUIElement(Parent, kHorizontalFrame)
{
  // Creates a frame containing two entry-boxes 

  m_Label = Label;
	m_IsEmphasized = Emphasize;
  m_MinValue = MinValue;
  m_MaxValue = MaxValue;

	m_Limits = Limits;
	m_Min = Min;
	m_Max = Max;
  
  m_Type = e_Double;

  Init();

  Create(1);
}


////////////////////////////////////////////////////////////////////////////////


MGUIEMinMaxEntry::MGUIEMinMaxEntry(const TGWindow *Parent, MString Label, bool Emphasize, 
																	 MString MinLabel, MString MaxLabel, 
                                   double MinValue, double MaxValue,
																	 bool Limits, double Min, double Max) :
  MGUIElement(Parent)
{
  // Creates a frame containing two entry-boxes 

  m_Label = Label;
	m_IsEmphasized = Emphasize;

  m_MinLabel = MinLabel;
  m_MaxLabel = MaxLabel;
  m_MinValue = MinValue;
  m_MaxValue = MaxValue;

	m_Limits = Limits;
	m_Min = Min;
	m_Max = Max;
  
  m_Type = e_Double;

  Init();

  Create(3);
}


////////////////////////////////////////////////////////////////////////////////


MGUIEMinMaxEntry::MGUIEMinMaxEntry(const TGWindow *Parent, MString Label, bool Emphasize,
                                   int MinValue, int MaxValue, 
                                   bool Limits, int Min, int Max) :
  MGUIElement(Parent, kHorizontalFrame)
{
  // Creates a frame containing two entry-boxes 

  m_Label = Label;
	m_IsEmphasized = Emphasize;
  m_MinValue = MinValue;
  m_MaxValue = MaxValue;

	m_Limits = Limits;
	m_Min = Min;
	m_Max = Max;
  
  m_Type = e_Integer;

  Init();

  Create(1);
}


////////////////////////////////////////////////////////////////////////////////


MGUIEMinMaxEntry::MGUIEMinMaxEntry(const TGWindow *Parent, MString Label, bool Emphasize, 
																	 MString MinLabel, MString MaxLabel, 
                                   int MinValue, int MaxValue,
																	 bool Limits, int Min, int Max) :
  MGUIElement(Parent)
{
  // Creates a frame containing two entry-boxes 

  m_Label = Label;
	m_IsEmphasized = Emphasize;

  m_MinLabel = MinLabel;
  m_MaxLabel = MaxLabel;
  m_MinValue = MinValue;
  m_MaxValue = MaxValue;

	m_Limits = Limits;
	m_Min = Min;
	m_Max = Max;
  
  m_Type = e_Integer;

  Init();

  Create(3);
}


////////////////////////////////////////////////////////////////////////////////


MGUIEMinMaxEntry::~MGUIEMinMaxEntry()
{
  // default destructor

  if (MustCleanup() == kNoCleanup) {
    delete m_TextLabel;
    delete m_TextLabelLayout;
    
    delete m_MinEntry;
    delete m_MaxEntry;
    
    delete m_EntryLayout;
  }
}


////////////////////////////////////////////////////////////////////////////////


void MGUIEMinMaxEntry::Init()
{
  // All initialization common to all constructors:

}


////////////////////////////////////////////////////////////////////////////////


void MGUIEMinMaxEntry::Create(int Mode)
{
  // There are two modes:
  // Mode 1: only one line is displayed starting with the label and two entry boxes
  // Mode 3: Three lines are displayed one of the main label, 
  //         one for the min label and entry and one for the 
  //         max label and entry

  // The label:
  m_TextLabel = new TGLabel(this, new TGString(m_Label));
  if (m_IsEmphasized == true) {
    m_TextLabel->SetTextFont(m_EmphasizedFont);
  }    
  m_TextLabelLayout =
    new TGLayoutHints(kLHintsLeft | kLHintsTop, 0, 20, 3, 4);
  AddFrame(m_TextLabel, m_TextLabelLayout);
  
	if (Mode == 1) {
		m_EntryLayout =	new TGLayoutHints(kLHintsRight | kLHintsTop, 5, 0, 0, 0);
	} else {
		m_EntryLayout =	new TGLayoutHints(kLHintsExpandX | kLHintsTop, 20, 0, 0, 2);
	}

  if (m_Type == e_Integer) {
    m_MaxEntry = new MGUIEEntry(this, m_MaxLabel, false, int(m_MaxValue), m_Limits, int(m_Min), int(m_Max));
    m_MinEntry = new MGUIEEntry(this, m_MinLabel, false, int(m_MinValue), m_Limits, int(m_Min), int(m_Max));
  } else {
    m_MaxEntry = new MGUIEEntry(this, m_MaxLabel, false, m_MaxValue, m_Limits, m_Min, m_Max);
    m_MinEntry = new MGUIEEntry(this, m_MinLabel, false, m_MinValue, m_Limits, m_Min, m_Max);
  }

	if (Mode == 1) {
		AddFrame(m_MaxEntry, m_EntryLayout);
		AddFrame(m_MinEntry, m_EntryLayout);
	} else {
		AddFrame(m_MinEntry, m_EntryLayout);
		AddFrame(m_MaxEntry, m_EntryLayout);
	}

  return;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIEMinMaxEntry::IsModified()
{
  //! Return true if the content has been modified 

  if (m_MinEntry != 0 && m_MinEntry->IsModified() == true) return true; 
  if (m_MaxEntry != 0 && m_MaxEntry->IsModified() == true) return true; 

  return false;
}


////////////////////////////////////////////////////////////////////////////////


double MGUIEMinMaxEntry::GetMinValueDouble()
{
  // 

  if (m_Type == e_Integer) {
    merr<<m_Label<<": Internal data type is integer not double"<<show;
  }

  return m_MinEntry->GetAsDouble();
}


////////////////////////////////////////////////////////////////////////////////


double MGUIEMinMaxEntry::GetMaxValueDouble()
{
  //

  if (m_Type == e_Integer) {
    merr<<m_Label<<": Internal data type is integer not double"<<show;
  }

  return m_MaxEntry->GetAsDouble();
}


////////////////////////////////////////////////////////////////////////////////


int MGUIEMinMaxEntry::GetMinValueInt()
{
  // 

  if (m_Type == e_Double) {
    merr<<m_Label<<": Internal data type is double not integer"<<show;
  }

  return m_MinEntry->GetAsInt();
}


////////////////////////////////////////////////////////////////////////////////


int MGUIEMinMaxEntry::GetMaxValueInt()
{
  //

  if (m_Type == e_Double) {
    merr<<m_Label<<": Internal data type is double not integer"<<show;
  }

  return m_MaxEntry->GetAsInt();
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIEMinMaxEntry::CheckRange(double MinMin, double MinMax, 
                                  double MaxMin, double MaxMax, 
                                  bool MinMustBeSmaller)
{
  // Check if the input-field contain numbers, and that they are within [Min;Max]

  ostringstream S;


  if (m_MinEntry->GetAsDouble() < MinMin) {
    S<<"Error in first input field of \""<<m_Label<<"\""<<endl
     <<"Value ("<<m_MinEntry->GetAsDouble()<<") is below its minimum value ("<<MinMin<<")";
    new TGMsgBox(gClient->GetRoot(), GetParent(), "Type error", 
                 S.str().c_str(), 
                 kMBIconStop, kMBOk);
    return false;
  }

  if (m_MinEntry->GetAsDouble() > MinMax) {
    S<<"Error in first input field of \""<<m_Label<<"\""<<endl
     <<"Value ("<<m_MinEntry->GetAsDouble()<<") is above its maximum value ("<<MinMax<<")";
    new TGMsgBox(gClient->GetRoot(), GetParent(), "Type error", 
                 S.str().c_str(), 
                 kMBIconStop, kMBOk);
    return false;
  }

  if (m_MaxEntry->GetAsDouble() < MaxMin) {
    S<<"Error in second input field of \""<<m_Label<<"\""<<endl
     <<"Value ("<<m_MaxEntry->GetAsDouble()<<") is below its minimum value ("<<MaxMin<<")";
    new TGMsgBox(gClient->GetRoot(), GetParent(), "Type error", 
                 S.str().c_str(), 
                 kMBIconStop, kMBOk);
    return false;
  }

  if (m_MaxEntry->GetAsDouble() > MaxMax) {
    S<<"Error in second input field of \""<<m_Label<<"\""<<endl
     <<"Value ("<<m_MaxEntry->GetAsDouble()<<") is above its maximum value ("<<MaxMax<<")";
    new TGMsgBox(gClient->GetRoot(), GetParent(), "Type error", 
                 S.str().c_str(), 
                 kMBIconStop, kMBOk);
    return false;
  }

  if (m_MinEntry->GetAsDouble() > m_MaxEntry->GetAsDouble()) {
    S<<"Error in \""<<m_Label<<"\""<<endl
     <<"The minmum value (first one) is larger than the maximum value (second one)!";
    new TGMsgBox(gClient->GetRoot(), GetParent(), "Type error", 
                 S.str().c_str(), 
                 kMBIconStop, kMBOk);
    return false;
  }

  if (MinMustBeSmaller == true) {
    if (m_MinEntry->GetAsDouble() == m_MaxEntry->GetAsDouble()) {
      S<<"Error in \""<<m_Label<<"\""<<endl
       <<"The minmum value (first one) is equal to the maximum value (second one)!";
      new TGMsgBox(gClient->GetRoot(), GetParent(), "Type error", 
                   S.str().c_str(), 
                   kMBIconStop, kMBOk);
      return false;
    }
  }
	

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIEMinMaxEntry::CheckRange(int MinMin, int MinMax, 
                                  int MaxMin, int MaxMax, 
                                  bool MinMustBeSmaller)
{
  // Check if the input-field contain numbers, and that they are within [Min;Max]

  ostringstream S;


  if (m_MinEntry->GetAsInt() < MinMin) {
    S<<"Error in first input field of \""<<m_Label<<"\""<<endl
     <<"Value ("<<m_MinEntry->GetAsInt()<<") is below its minimum value ("<<MinMin<<")";
    new TGMsgBox(gClient->GetRoot(), GetParent(), "Type error", 
                 S.str().c_str(), 
                 kMBIconStop, kMBOk);
    return false;
  }

  if (m_MinEntry->GetAsInt() > MinMax) {
    S<<"Error in first input field of \""<<m_Label<<"\""<<endl
     <<"Value ("<<m_MinEntry->GetAsInt()<<") is above its maximum value ("<<MinMax<<")";
    new TGMsgBox(gClient->GetRoot(), GetParent(), "Type error", 
                 S.str().c_str(), 
                 kMBIconStop, kMBOk);
    return false;
  }

  if (m_MaxEntry->GetAsInt() < MaxMin) {
    S<<"Error in second input field of \""<<m_Label<<"\""<<endl
     <<"Value ("<<m_MaxEntry->GetAsInt()<<") is below its minimum value ("<<MaxMin<<")";
    new TGMsgBox(gClient->GetRoot(), GetParent(), "Type error", 
                 S.str().c_str(), 
                 kMBIconStop, kMBOk);
    return false;
  }

  if (m_MaxEntry->GetAsInt() > MaxMax) {
    S<<"Error in second input field of \""<<m_Label<<"\""<<endl
     <<"Value ("<<m_MaxEntry->GetAsInt()<<") is above its maximum value ("<<MaxMax<<")";
    new TGMsgBox(gClient->GetRoot(), GetParent(), "Type error", 
                 S.str().c_str(), 
                 kMBIconStop, kMBOk);
    return false;
  }

  if (m_MinEntry->GetAsInt() > m_MaxEntry->GetAsInt()) {
    S<<"Error in \""<<m_Label<<"\""<<endl
     <<"The minmum value (first one) is larger than the maximum value (second one)!";
    new TGMsgBox(gClient->GetRoot(), GetParent(), "Type error", 
                 S.str().c_str(), 
                 kMBIconStop, kMBOk);
    return false;
  }
  
  if (MinMustBeSmaller == true) {
    if (m_MinEntry->GetAsInt() == m_MaxEntry->GetAsInt()) {
      S<<"Error in \""<<m_Label<<"\""<<endl
       <<"The minmum value (first one) is equal to the maximum value (second one)!";
      new TGMsgBox(gClient->GetRoot(), GetParent(), "Type error", 
                   S.str().c_str(), 
                   kMBIconStop, kMBOk);
      return false;
    }
  }
	

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MGUIEMinMaxEntry::SetEntryFieldSize(int Size)
{
  // Set the size of the entry field

  m_Size = Size;

  m_MinEntry->SetEntryFieldSize(Size);
  m_MaxEntry->SetEntryFieldSize(Size);
}


////////////////////////////////////////////////////////////////////////////////


void MGUIEMinMaxEntry::SetEnabled(bool flag)
{
	m_IsEnabled = flag;
	
  m_MinEntry->SetEnabled(m_IsEnabled);
  m_MaxEntry->SetEnabled(m_IsEnabled);
}

////////////////////////////////////////////////////////////////////////////////


MString MGUIEMinMaxEntry::MakeSmartString(double Number)
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


bool MGUIEMinMaxEntry::CheckForNumber(MString Number)
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


double MGUIEMinMaxEntry::ToNumber(MString Number) 
{
  // Convert a string to a number

  return atof(Number);
}


// MGUIEMinMaxEntry.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
