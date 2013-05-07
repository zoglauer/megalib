/*
 * MGUIEStatusBar.cxx
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
// MGUIEStatusBar
//
//
// This class is an elementary GUI-widget:
//
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MGUIEStatusBar.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MStreams.h"
#include "MStreams.h"


////////////////////////////////////////////////////////////////////////////////


MGUIEStatusBar::MGUIEStatusBar(const TGWindow *Parent, MString Label, bool Emphasize, int Width) :
  MGUIElement(Parent, kRaisedFrame | kHorizontalFrame)
{
  // Creates a frame containing a label and an entry-box 
  //
  // Parent:   parent Window, where this frame is contained
  // Label:    text of the label
  // Value:    default entry

  m_IsEmphasized = Emphasize;

	m_Width = Width;
  m_Label = Label;

  Init();
}



////////////////////////////////////////////////////////////////////////////////


MGUIEStatusBar::~MGUIEStatusBar()
{
  // Destruct this instance of MGUIEStatusBar

  if (MustCleanup() == kNoCleanup) {
    delete m_LabelGraphics;

    delete m_TextLabel;
    delete m_TextLabelLayout;
    
    delete m_LabelFrame;
    delete m_LabelFrameLayout;
    
    m_FieldNames->Delete();
    
    m_FieldTitles->Delete();
    m_FieldFrames->Delete();
    
    m_FieldFrameLayouts->Delete();
    
    m_FieldContents->Delete();
    
    m_FieldContentValues->Delete();
    m_FieldContentFrames->Delete();
    m_FieldContentFrameLayouts->Delete();
    
    delete m_FieldTitleLayout;
    delete m_FieldContentLayout;
  }

  delete m_FieldNames;
  delete m_FieldTitles;
  delete m_FieldFrames;
  delete m_FieldFrameLayouts;
  delete m_FieldContents;
  delete m_FieldContentValues;
  delete m_FieldContentFrames;
  delete m_FieldContentFrameLayouts;
}



////////////////////////////////////////////////////////////////////////////////


void MGUIEStatusBar::Init()
{
  // Initilizations common to all constructors:

	m_IsCreated = false;

	m_FieldNames = new TObjArray();
	m_FieldFrames = new TObjArray();
	m_FieldFrameLayouts = new TObjArray();
	m_FieldTitles = new TObjArray();
	m_FieldContentValues = new TObjArray();
	m_FieldContentFrames = new TObjArray();
	m_FieldContentFrameLayouts = new TObjArray();
	m_FieldContents = new TObjArray();
}


////////////////////////////////////////////////////////////////////////////////


void MGUIEStatusBar::Create()
{
  // Create the label and the input-field.


	m_LabelFrameLayout = new TGLayoutHints(kLHintsLeft, 0, 0, 0, 0);
	if (m_Width == c_Min) {
		m_LabelFrame = new TGCompositeFrame(this, 100, 100, kHorizontalFrame);
	} else {
    m_LabelFrame = new TGCompositeFrame(this, 100, 100, kHorizontalFrame | kFixedWidth);
		m_LabelFrame->Resize(m_Width, 30);
	}
	AddFrame(m_LabelFrame, m_LabelFrameLayout);

  
  m_TextLabel = new TGLabel(m_LabelFrame, new TGString(m_Label));
  if (m_IsEmphasized == true) {
    m_TextLabel->SetTextFont(m_EmphasizedFont);
  }
  m_TextLabelLayout = new TGLayoutHints(kLHintsLeft, 3, 2, 2, 2);
  m_LabelFrame->AddFrame(m_TextLabel, m_TextLabelLayout);


	// Now add the fields:
	m_FieldTitleLayout = new TGLayoutHints(kLHintsLeft, 3, 3, 2, 2);
	m_FieldContentLayout = new TGLayoutHints(kLHintsLeft, 3, 3, 0, 0);

	for (int f = 0; f <= m_FieldNames->GetLast(); ++f) {
		// Create

		TGLayoutHints* FieldFrameLayout;
		if (m_FieldWidth[f] == c_Max) {
			FieldFrameLayout = new TGLayoutHints(kLHintsLeft | kLHintsExpandX, 5, 0, 0, 0);
		} else {
			FieldFrameLayout = new TGLayoutHints(kLHintsLeft, 5, 0, 0, 0);
		}

		TGCompositeFrame* FieldFrame = 
			new TGCompositeFrame(this, 100, 100, kHorizontalFrame); 
		AddFrame(FieldFrame, FieldFrameLayout);

		MString Name = ((TObjString*) m_FieldNames->At(f))->GetString().Data();
		TGLabel* FieldTitle = new TGLabel(FieldFrame, new TGString(Name));
		FieldFrame->AddFrame(FieldTitle, m_FieldTitleLayout);

		TGCompositeFrame* FieldContentFrame;
		TGLayoutHints* FieldContentFrameLayout;
		if (m_FieldWidth[f] == c_Max) {
			FieldContentFrame = new TGCompositeFrame(FieldFrame, 100, 100, kHorizontalFrame | kSunkenFrame); 
			FieldContentFrameLayout = new TGLayoutHints(kLHintsRight | kLHintsExpandX, 2, 2, 2, 2);
		} else if (m_FieldWidth[f] == c_Min) {
			FieldContentFrame = new TGCompositeFrame(FieldFrame, 100, 100, kHorizontalFrame | kSunkenFrame); 
			FieldContentFrameLayout = new TGLayoutHints(kLHintsRight, 2, 2, 2, 2);
		} else {
			FieldContentFrame = new TGCompositeFrame(FieldFrame, 100, 100, kHorizontalFrame | kSunkenFrame | kFixedWidth); 
			FieldContentFrameLayout = new TGLayoutHints(kLHintsRight, 2, 2, 2, 2);
		}
		FieldContentFrame->Resize(m_FieldWidth[f], 30);
		FieldFrame->AddFrame(FieldContentFrame, FieldContentFrameLayout);
		FieldContentFrame->Layout();

		Name = ((TObjString*) m_FieldContentValues->At(f))->GetString();
		TGLabel* FieldContent = new TGLabel(FieldContentFrame, Name);
		FieldContentFrame->AddFrame(FieldContent, m_FieldContentLayout);
		FieldContent->Layout();

		// Store
		m_FieldFrames->AddLast(FieldFrame);
		m_FieldFrameLayouts->AddLast(FieldFrameLayout);
		m_FieldTitles->AddLast(FieldTitle);
		m_FieldContentFrames->AddLast(FieldContentFrame);
		m_FieldContentFrameLayouts->AddLast(FieldContentFrameLayout);
		m_FieldContents->AddLast(FieldContent);
	}
  

  // Give this element the default size of its content:
  Resize(GetDefaultWidth(), GetDefaultHeight()+200); 

	Layout();
	MapSubwindows();

	m_IsCreated = true;

  return;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIEStatusBar::Add(MString Field, int Width)
{
	// Add a new field
	// Returns false, if the field already exists or the element has already been created

	if (m_IsCreated == true) return false;

	// Test if the name does not already exist:
	for (int f = 0; f <= m_FieldNames->GetLast(); ++f) {
		if (Field == ((TObjString*) m_FieldNames->At(f))->GetString().Data()) {
			return false;
		}
	}

	m_FieldNames->AddLast(new TObjString(Field));
	m_FieldWidth.push_back(Width);
	m_FieldContentValues->AddLast(new TObjString("0"));

	return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIEStatusBar::SetContent(MString Field, double Value)
{
	// Set the content of field Field:

	// Test if the name does not already exist:
	for (int f = 0; f <= m_FieldNames->GetLast(); ++f) {
		if (Field == ((TObjString*) m_FieldNames->At(f))->GetString().Data()) {
			((TObjString*) m_FieldContentValues->At(f))->SetString((char*) MakeSmartString(Value).Data());

			if (m_IsCreated == true) {
				((TGLabel*) m_FieldContents->At(f))->SetText(MakeSmartString(Value));
				Layout();				
			}

			return true;
		}
	}

	return false;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIEStatusBar::SetContent(MString Field, unsigned int Value)
{
  // Set the content of field Field:

  // Test if the name does not already exist:
  for (int f = 0; f <= m_FieldNames->GetLast(); ++f) {
    if (Field == ((TObjString*) m_FieldNames->At(f))->GetString().Data()) {
      ((TObjString*) m_FieldContentValues->At(f))->SetString((char*) MakeSmartString(Value).Data());

      if (m_IsCreated == true) {
        ((TGLabel*) m_FieldContents->At(f))->SetText(MakeSmartString(Value));
        Layout();       
      }

      return true;
    }
  }
  
  return false;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIEStatusBar::SetContent(MString Field, int Value)
{
  // Set the content of field Field:

  // Test if the name does not already exist:
  for (int f = 0; f <= m_FieldNames->GetLast(); ++f) {
    if (Field == ((TObjString*) m_FieldNames->At(f))->GetString().Data()) {
      ((TObjString*) m_FieldContentValues->At(f))->SetString((char*) MakeSmartString(Value).Data());

      if (m_IsCreated == true) {
        ((TGLabel*) m_FieldContents->At(f))->SetText(MakeSmartString(Value));
        Layout();       
      }

      return true;
    }
  }
  
  return false;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIEStatusBar::SetContent(MString Field, MString Value)
{
	// Set the content of field Field:

	// Test if the name does not already exist:
	for (int f = 0; f <= m_FieldNames->GetLast(); ++f) {
		if (Field == ((TObjString*) m_FieldNames->At(f))->GetString().Data()) {
			((TObjString*) m_FieldContentValues->At(f))->SetString((char*) Value.Data());

			if (m_IsCreated == true) {
				((TGLabel*) m_FieldContents->At(f))->SetText(Value);
				Layout();
			}

			return true;
		}
	}

	return false;
}


////////////////////////////////////////////////////////////////////////////////


MString MGUIEStatusBar::MakeSmartString(double Number)
{
  // Make a smart string, i.e. cut of zeros, etc.

  ostringstream out;
  out.precision(12); // Not too high to avoid the round-off errors
  out<<Number;
  
  MString S(out);
  S.StripBack('0');
  
  return S;
}


// MGUIEStatusBar.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
