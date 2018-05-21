/*
 * MGUIESlider.cxx
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
// MGUIESlider
//
//
// GUI element containing a label, a slider which is labeled with its 
// min, max and current value
//
// Known bugs:
// If the GUIE gets to small - then:  *** Break *** floating point exception
// (ROOT-bug!)
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MGUIESlider.h"

// Standard libs:
#include <MStreams.h>

// ROOT libs:
#include <TGMsgBox.h>

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MGUIESlider)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIESlider::MGUIESlider(const TGWindow* Parent, MString Label, double MinValue, double UsedValue, double MaxValue) :
  MGUIElement(Parent)
{
  // Creates a frame containing a label a slider and its min, max and current value:
  //
  // Label:     label of the widget
  // MinValue:  Minimum slider value
  // MaxValue:  Maximum slider value
  // UsedValue: First slider value

  m_Label = Label;
  m_MinValue = MinValue;
  m_MaxValue = MaxValue;
  if (m_UsedValue < m_MinValue) {
    m_UsedValue = m_MinValue;
  } else if (m_UsedValue > m_MaxValue) {
    m_UsedValue = m_MaxValue; 
  } else {
    m_UsedValue = UsedValue;
  }

  if (m_MinValue > m_MaxValue) {
    Error("MGUIESlider::MGUIESlider(...)",
          "The minimum value is larger than the maximum value:\n"
          "Min = %f, Max = %f", m_MinValue, m_MaxValue);
  }

  Create();
}


////////////////////////////////////////////////////////////////////////////////


MGUIESlider::~MGUIESlider()
{
  // default destructor

  if (MustCleanup() == kNoCleanup) {
    delete m_TextLabel;
    delete m_TextLabelLayout;

    delete m_MinLabel;
    delete m_MinLabelLayout;
    delete m_UsedLabel;
    delete m_UsedLabelLayout;
    delete m_MaxLabel;
    delete m_MaxLabelLayout;
    
    delete m_SliderValuesFrame;
    delete m_SliderValuesFrameLayout;
    
    delete m_Slider;
    delete m_SliderLayout;
    
    delete m_SliderFrame;
    delete m_SliderFrameLayout;
  }
}


////////////////////////////////////////////////////////////////////////////////


void MGUIESlider::Create()
{
  // Create the GUI element

  // The label:
  m_TextLabel = new TGLabel(this, new TGString(m_Label));
  m_TextLabelLayout =
    new TGLayoutHints(kLHintsLeft | kLHintsTop, 0, 0, 0, 0);
  AddFrame(m_TextLabel, m_TextLabelLayout);


  // Slider frame containing the slider and the labels
  m_SliderFrame = new TGVerticalFrame(this, 100, 50 /*, kRaisedFrame*/);
  m_SliderFrameLayout =
    new TGLayoutHints(kLHintsExpandX | kLHintsTop, 0, 0, 0, 0);
  AddFrame(m_SliderFrame, m_SliderFrameLayout);

  m_Slider = new TGHSlider(m_SliderFrame, 100, kSlider1 | kScaleBoth, 1);
  m_Slider->Associate(this);
  m_Slider->SetRange((int) m_MinValue, (int) m_MaxValue);
  m_Slider->SetPosition((int) m_UsedValue);
  m_SliderLayout = new TGLayoutHints(kLHintsExpandX | kLHintsTop, 0, 0, 0, 0);
  m_SliderFrame->AddFrame(m_Slider, m_SliderLayout);


  // Frame containing the labels/values:
  m_SliderValuesFrame = new TGHorizontalFrame(m_SliderFrame, 100, 15);
  m_SliderValuesFrameLayout =
    new TGLayoutHints(kLHintsExpandX | kLHintsTop, 5, 5, 3, 3);
  m_SliderFrame->AddFrame(m_SliderValuesFrame, m_SliderValuesFrameLayout);

  m_MinLabel = new TGLabel(m_SliderValuesFrame, new TGString(MakeSmartString(m_MinValue)));
  m_MinLabelLayout = new TGLayoutHints(kLHintsLeft | kLHintsTop, 0, 0, 0, 0);
  m_SliderValuesFrame->AddFrame(m_MinLabel, m_MinLabelLayout);

  m_UsedLabel = new TGLabel(m_SliderValuesFrame, new TGString(MakeSmartString(m_UsedValue)));
  m_UsedLabelLayout = new TGLayoutHints(kLHintsCenterX | kLHintsTop, 0, 0, 0, 0);
  m_SliderValuesFrame->AddFrame(m_UsedLabel, m_UsedLabelLayout);

  m_MaxLabel = new TGLabel(m_SliderValuesFrame, new TGString(MakeSmartString(m_MaxValue)));
  m_MaxLabelLayout = new TGLayoutHints(kLHintsRight | kLHintsTop, 0, 0, 0, 0);
  m_SliderValuesFrame->AddFrame(m_MaxLabel, m_MaxLabelLayout);

  return;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIESlider::ProcessMessage(long Message, long Parameter1, 
                                   long Parameter2)
{
  // Process the messages for this application

  switch (GET_MSG(Message)) {
  case kC_HSLIDER:
    switch (GET_SUBMSG(Message)) {
    case kSL_POS:
      switch (Parameter1) {
      case 1: // SliderID

        // Update the current value label
        m_UsedLabel->SetText(new TGString(MakeSmartString(Parameter2)));
        m_UsedLabel->SetSize(m_UsedLabel->GetDefaultSize());
        m_UsedValue = Parameter2;
        Layout();
        break;
        
      default:
        break;
      }

    default:
      break;
    }
  default:
    break;
  }
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


double MGUIESlider::GetValue()
{
  // Return the current value of the slider

  return m_UsedValue;
}


////////////////////////////////////////////////////////////////////////////////


MString MGUIESlider::MakeSmartString(double Number)
{
  // Make a smart string, i.e. cut of zeros, etc.

  ostringstream out;
  out.precision(12); // Not too high to avoid the round-off errors
  out<<Number;
  
  MString S(out);
  S.StripBack('0');
  
  return S;
}


// MGUIESlider.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
