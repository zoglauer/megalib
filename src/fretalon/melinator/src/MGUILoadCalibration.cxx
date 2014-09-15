/*
 * MGUILoadCalibration.cxx
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
// MGUILoadCalibration
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MGUILoadCalibration.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MStreams.h"
#include "MFile.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MGUILoadCalibration)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUILoadCalibration::MGUILoadCalibration(const TGWindow* Parent, 
                                                   const TGWindow* Main, 
                                                   MSettingsMelinator* Settings)
  : MGUIDialog(Parent, Main)
{
  // Construct an instance of MGUILoadCalibration and bring it to the screen

  m_Parent = (TGWindow *) Parent;
  m_Settings = Settings;

  // use hierarchical cleaning
  SetCleanup(kDeepCleanup);

  Create();
}


////////////////////////////////////////////////////////////////////////////////


MGUILoadCalibration::~MGUILoadCalibration()
{
  // Delete an instance of MGUILoadCalibration
}


////////////////////////////////////////////////////////////////////////////////


void MGUILoadCalibration::Create()
{
  // Create the main window

  // Load the isotopes
  if (m_IsotopeStore.Load("$(MEGALIB)/resource/libraries/Calibration.isotopes") == false) {
    merr<<"Unable to load calibration isotopes... This GUI will be missing some integral parts!"<<show;
  }
  
  
  // We start with a name and an icon...
  SetWindowName("Load calibration file");  

  AddSubTitle("Set the calibration file(s) to load and its associated isotope(s)"); 

  TGLayoutHints* IsotopeFrameLayout = new TGLayoutHints(kLHintsCenterX | kLHintsTop | kLHintsExpandX, 20*m_FontScaler, 20*m_FontScaler, 5*m_FontScaler, 0);

  TGLayoutHints* FirstLabelLayout = new TGLayoutHints(kLHintsLeft | kLHintsTop, 0, 0, 0, 0);
  TGLayoutHints* FileSelectorLayout = new TGLayoutHints(kLHintsLeft | kLHintsExpandX, 5*m_FontScaler, 0, 0, 0);
  TGLayoutHints* IsotopeLabelLayout = new TGLayoutHints(kLHintsLeft | kLHintsTop, 20*m_FontScaler, 0, 0, 0);
  TGLayoutHints* ButtonLayout = new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 20*m_FontScaler, 0, 0, 0);
  
  TGLayoutHints* FooterLayout = new TGLayoutHints(kLHintsCenterX | kLHintsTop, 20*m_FontScaler, 20*m_FontScaler, 20*m_FontScaler, 0);
  
  vector<MString> CalibrationFiles = m_Settings->GetCalibrationFiles();
  vector<unsigned int> CalibrationGroupIDs = m_Settings->GetCalibrationGroupIDs();
  vector<vector<MString>> CalibrationIsotopes = m_Settings->GetCalibrationIsotopes();
  
  // Add an empty one
  while (CalibrationFiles.size() < 15) {
    CalibrationFiles.push_back("");
    CalibrationGroupIDs.push_back(0);
    CalibrationIsotopes.push_back(vector<MString>());
  }
  
  for (unsigned int f = 0; f < CalibrationFiles.size(); ++f) {
    TGHorizontalFrame* CalibrationFrame = new TGHorizontalFrame(this);
    AddFrame(CalibrationFrame, IsotopeFrameLayout);
    
    MString Name;
    Name += f+1;
    Name += ":";
    TGLabel* Label = new TGLabel(CalibrationFrame, Name);
    Label->ChangeOptions(kFixedWidth);
    Label->SetWidth(m_FontScaler*20);
    Label->SetTextJustify(kTextRight);
    CalibrationFrame->AddFrame(Label, FirstLabelLayout);

    MGUIEFileSelector* FileName = new MGUIEFileSelector(CalibrationFrame,
                                  "",
                                  CalibrationFiles[f]);
    FileName->SetFileType("Calib file", "*.roa");
    FileName->SetFileType("Calib file", "*.dat");
    //FileName->ChangeOptions(kFixedWidth);
    //FileName->SetWidth(m_FontScaler*500);
    CalibrationFrame->AddFrame(FileName, FileSelectorLayout);
    m_FileNames.push_back(FileName);
  
    TGLabel* IsotopeLabel = new TGLabel(CalibrationFrame, "Isotopes:  ");
    CalibrationFrame->AddFrame(IsotopeLabel, IsotopeLabelLayout);
  
    while (CalibrationIsotopes[f].size() < 3) {
      CalibrationIsotopes[f].push_back(""); 
    }
    m_Isotopes.push_back(vector<TGComboBox*>());
    for (unsigned int i = 0; i < CalibrationIsotopes[f].size(); ++i) {
      TGComboBox* Isotope = new TGComboBox(CalibrationFrame, 10000 + 100*f + i);
      AddIsotopes(Isotope, CalibrationIsotopes[f][i]);
      CalibrationFrame->AddFrame(Isotope, FirstLabelLayout);
      m_Isotopes.back().push_back(Isotope);
    }
 
    MGUIEEntry* GroupID = new MGUIEEntry(CalibrationFrame, "Group ID: ", false, (int) CalibrationGroupIDs[f], true, 0);
    GroupID->SetEntryFieldSize(m_FontScaler*40);
    CalibrationFrame->AddFrame(GroupID, IsotopeLabelLayout);
    m_GroupIDs.push_back(GroupID);
    
    MString Icon = "$(ROOTSYS)/icons/bld_delete.xpm";
    MFile::ExpandFileName(Icon);

    TGPictureButton* Clear = new TGPictureButton(CalibrationFrame, Icon, 10000 + 100*f + 99);
    Clear->Associate(this);
    CalibrationFrame->AddFrame(Clear, ButtonLayout);
  }
 
  TGLabel* Footer1 = new TGLabel(this, "Tips: Calibration files with the same group ID will be merged.  You can add additional isotopes to the file: $(MEGALIB)/resource/libraries/Calibration.isotopes");
  AddFrame(Footer1, FooterLayout);
  
  
  AddOKCancelButtons();

  PositionWindow(GetDefaultWidth(), GetDefaultHeight(), true);

  // and bring it to the screen.
  MapSubwindows();
  MapWindow();  

  Layout();

  fClient->WaitFor(this);
 
  return;
}

////////////////////////////////////////////////////////////////////////////////


bool MGUILoadCalibration::ProcessMessage(long Message, long Parameter1, 
                                         long Parameter2)
{
  // Process the messages for this application

  switch (GET_MSG(Message)) {
  case kC_COMMAND:
    switch (GET_SUBMSG(Message)) {
    case kCM_BUTTON:
      if (Parameter1 % 100 == 99) {
        int ID = (Parameter1 - 10099) / 100;
        m_FileNames[ID]->SetFileName("");
        m_GroupIDs[ID]->SetValue(0);
        for (unsigned int i = 0; i < m_Isotopes[ID].size(); ++i) {
          m_Isotopes[ID][i]->Select(0);
        }
      }
    default:
      break;
    }
  default:
    break;
  }

  return MGUIDialog::ProcessMessage(Message, Parameter1, Parameter2);
}


////////////////////////////////////////////////////////////////////////////////


//! Add the isotopes to the combo box
void MGUILoadCalibration::AddIsotopes(TGComboBox* ComboBox, MString Selected)
{
  ComboBox->AddEntry("None", 0);
  ComboBox->Select(0);
  for (unsigned int i = 0; i < m_IsotopeStore.GetNumberOfIsotopes(); ++i) {
    ComboBox->AddEntry(m_IsotopeStore.Get(i).GetName(), i+1);
    if (m_IsotopeStore.Get(i).GetName() == Selected) {
      ComboBox->Select(i+1);
    }
  }
  ComboBox->Associate(this);
  ComboBox->SetHeight(m_FontScaler*19);
  ComboBox->SetWidth(m_FontScaler*60);
  unsigned int Height = ComboBox->GetListBox()->GetNumberOfEntries()*ComboBox->GetListBox()->GetItemVsize();
  ComboBox->GetListBox()->SetHeight(Height);

}


////////////////////////////////////////////////////////////////////////////////


//! Action after the Apply or OK button has been pressed.
bool MGUILoadCalibration::OnApply()
{
  vector<MString> CalibrationFiles;
  vector<unsigned int> CalibrationGroupIDs;
  vector<vector<MString>> CalibrationIsotopes;
  
  for (auto FileName: m_FileNames) {
    CalibrationFiles.push_back(FileName->GetFileName()); 
  }
  for (auto IsotopeVector: m_Isotopes) {
    CalibrationIsotopes.push_back(vector<MString>());
    for (auto Isotope: IsotopeVector) {
      MString Text = dynamic_cast<TGTextLBEntry*>(Isotope->GetSelectedEntry())->GetText()->GetString();
      CalibrationIsotopes.back().push_back(Text);
    }
  }
  for (auto GroupID: m_GroupIDs) {
    CalibrationGroupIDs.push_back(GroupID->GetAsInt());
  }
 
  m_Settings->SetAllCalibrationFiles(CalibrationFiles, CalibrationGroupIDs, CalibrationIsotopes);
  
  return true;
}


// MGUILoadCalibration: the end...
////////////////////////////////////////////////////////////////////////////////
