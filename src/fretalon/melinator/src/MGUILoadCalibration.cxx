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

  TGLayoutHints* SubSubTitleLayout = new TGLayoutHints(kLHintsTop | kLHintsCenterX | kLHintsExpandX, 20, 20, 20, 0);
  TGLayoutHints* SingleLayout = new TGLayoutHints(kLHintsLeft | kLHintsExpandX, 20, 20, 20, 0);
  TGLayoutHints* IsotopeFrameLayout = new TGLayoutHints(kLHintsLeft | kLHintsExpandX, 20, 20, 5, 0);
  TGLayoutHints* IsotopeLabelLayout = new TGLayoutHints(kLHintsLeft | kLHintsTop, 0, 5, 0, 0);
  TGLayoutHints* GroupIDLabelLayout = new TGLayoutHints(kLHintsLeft | kLHintsTop, 30, 5, 0, 0);
  TGLayoutHints* FooterLayout = new TGLayoutHints(kLHintsCenterX | kLHintsTop, 20, 20, 40, 0);

  TGLabel* SubSubTitle = new TGLabel(this, "Calibration files with the same group ID will be merged!");
  AddFrame(SubSubTitle, SubSubTitleLayout);
  
  m_FileName1 = 
    new MGUIEFileSelector(this,
                          "First calibration file:",
                          m_Settings->GetCalibrationFile1());
  m_FileName1->SetFileType("Calib file", "*.roa");
  m_FileName1->SetFileType("Calib file", "*.dat");
  AddFrame(m_FileName1, SingleLayout);

  TGHorizontalFrame* IsotopeFrame1 = new TGHorizontalFrame(this);
  AddFrame(IsotopeFrame1, IsotopeFrameLayout);
  
  TGLabel* IsotopeLabel1 = new TGLabel(IsotopeFrame1, "Isotopes:  ");
  IsotopeFrame1->AddFrame(IsotopeLabel1, IsotopeLabelLayout);
  
  m_Isotope11 = new TGComboBox(IsotopeFrame1, c_Isotope11);
  AddIsotopes(m_Isotope11, m_Settings->GetCalibrationFile1Isotope1());
  IsotopeFrame1->AddFrame(m_Isotope11, IsotopeLabelLayout);
  
  m_Isotope12 = new TGComboBox(IsotopeFrame1, c_Isotope12);
  AddIsotopes(m_Isotope12, m_Settings->GetCalibrationFile1Isotope2());
  IsotopeFrame1->AddFrame(m_Isotope12, IsotopeLabelLayout);
  
  m_Isotope13 = new TGComboBox(IsotopeFrame1, c_Isotope13);
  AddIsotopes(m_Isotope13, m_Settings->GetCalibrationFile1Isotope3());
  IsotopeFrame1->AddFrame(m_Isotope13, IsotopeLabelLayout);
 
  m_GroupID1 = new MGUIEEntry(IsotopeFrame1, "Group ID: ", false, (int) m_Settings->GetCalibrationFile1GroupID(), true, 0);
  m_GroupID1->SetEntryFieldSize(m_FontScaler*40);
  IsotopeFrame1->AddFrame(m_GroupID1, GroupIDLabelLayout);

  
  m_FileName2 = 
    new MGUIEFileSelector(this,
                          "Second calibration file:",
                          m_Settings->GetCalibrationFile2());
  m_FileName2->SetFileType("Calib file", "*.roa");
  m_FileName2->SetFileType("Calib file", "*.dat");
  AddFrame(m_FileName2, SingleLayout);

  TGHorizontalFrame* IsotopeFrame2 = new TGHorizontalFrame(this);
  AddFrame(IsotopeFrame2, IsotopeFrameLayout);
  
  TGLabel* IsotopeLabel2 = new TGLabel(IsotopeFrame2, "Isotopes:  ");
  IsotopeFrame2->AddFrame(IsotopeLabel2, IsotopeLabelLayout);
  
  m_Isotope21 = new TGComboBox(IsotopeFrame2, c_Isotope21);
  AddIsotopes(m_Isotope21, m_Settings->GetCalibrationFile2Isotope1());
  IsotopeFrame2->AddFrame(m_Isotope21, IsotopeLabelLayout);
  
  m_Isotope22 = new TGComboBox(IsotopeFrame2, c_Isotope22);
  AddIsotopes(m_Isotope22, m_Settings->GetCalibrationFile2Isotope2());
  IsotopeFrame2->AddFrame(m_Isotope22, IsotopeLabelLayout);
  
  m_Isotope23 = new TGComboBox(IsotopeFrame2, c_Isotope23);
  AddIsotopes(m_Isotope23, m_Settings->GetCalibrationFile2Isotope3());
  IsotopeFrame2->AddFrame(m_Isotope23, IsotopeLabelLayout);
 
  m_GroupID2 = new MGUIEEntry(IsotopeFrame2, "Group ID: ", false, (int) m_Settings->GetCalibrationFile2GroupID(), true, 0);
  m_GroupID2->SetEntryFieldSize(m_FontScaler*40);
  IsotopeFrame2->AddFrame(m_GroupID2, GroupIDLabelLayout);

  
  m_FileName3 = 
    new MGUIEFileSelector(this,
                          "Third calibration file:",
                          m_Settings->GetCalibrationFile3());
  m_FileName3->SetFileType("Calib file", "*.roa");
  m_FileName3->SetFileType("Calib file", "*.dat");
  AddFrame(m_FileName3, SingleLayout);

  TGHorizontalFrame* IsotopeFrame3 = new TGHorizontalFrame(this);
  AddFrame(IsotopeFrame3, IsotopeFrameLayout);
  
  TGLabel* IsotopeLabel3 = new TGLabel(IsotopeFrame3, "Isotopes:  ");
  IsotopeFrame3->AddFrame(IsotopeLabel3, IsotopeLabelLayout);
  
  m_Isotope31 = new TGComboBox(IsotopeFrame3, c_Isotope31);
  AddIsotopes(m_Isotope31, m_Settings->GetCalibrationFile3Isotope1());
  IsotopeFrame3->AddFrame(m_Isotope31, IsotopeLabelLayout);
  
  m_Isotope32 = new TGComboBox(IsotopeFrame3, c_Isotope32);
  AddIsotopes(m_Isotope32, m_Settings->GetCalibrationFile3Isotope2());
  IsotopeFrame3->AddFrame(m_Isotope32, IsotopeLabelLayout);
  
  m_Isotope33 = new TGComboBox(IsotopeFrame3, c_Isotope33);
  AddIsotopes(m_Isotope33, m_Settings->GetCalibrationFile3Isotope3());
  IsotopeFrame3->AddFrame(m_Isotope33, IsotopeLabelLayout);
 
  m_GroupID3 = new MGUIEEntry(IsotopeFrame3, "Group ID: ", false, (int) m_Settings->GetCalibrationFile3GroupID(), true, 0);
  m_GroupID3->SetEntryFieldSize(m_FontScaler*40);
  IsotopeFrame3->AddFrame(m_GroupID3, GroupIDLabelLayout);

  
  m_FileName4 = 
    new MGUIEFileSelector(this,
                          "Fourth calibration file:",
                          m_Settings->GetCalibrationFile4());
  m_FileName4->SetFileType("Calib file", "*.roa");
  m_FileName4->SetFileType("Calib file", "*.dat");
  AddFrame(m_FileName4, SingleLayout);

  TGHorizontalFrame* IsotopeFrame4 = new TGHorizontalFrame(this);
  AddFrame(IsotopeFrame4, IsotopeFrameLayout);
  
  TGLabel* IsotopeLabel4 = new TGLabel(IsotopeFrame4, "Isotopes:  ");
  IsotopeFrame4->AddFrame(IsotopeLabel4, IsotopeLabelLayout);
  
  m_Isotope41 = new TGComboBox(IsotopeFrame4, c_Isotope41);
  AddIsotopes(m_Isotope41, m_Settings->GetCalibrationFile4Isotope1());
  IsotopeFrame4->AddFrame(m_Isotope41, IsotopeLabelLayout);
  
  m_Isotope42 = new TGComboBox(IsotopeFrame4, c_Isotope42);
  AddIsotopes(m_Isotope42, m_Settings->GetCalibrationFile4Isotope2());
  IsotopeFrame4->AddFrame(m_Isotope42, IsotopeLabelLayout);
  
  m_Isotope43 = new TGComboBox(IsotopeFrame4, c_Isotope43);
  AddIsotopes(m_Isotope43, m_Settings->GetCalibrationFile4Isotope3());
  IsotopeFrame4->AddFrame(m_Isotope43, IsotopeLabelLayout);
 
  m_GroupID4 = new MGUIEEntry(IsotopeFrame4, "Group ID: ", false, (int) m_Settings->GetCalibrationFile4GroupID(), true, 0);
  m_GroupID4->SetEntryFieldSize(m_FontScaler*40);
  IsotopeFrame4->AddFrame(m_GroupID4, GroupIDLabelLayout);

  
  m_FileName5 = 
    new MGUIEFileSelector(this,
                          "Fifth calibration file:",
                          m_Settings->GetCalibrationFile5());
  m_FileName5->SetFileType("Calib file", "*.roa");
  m_FileName5->SetFileType("Calib file", "*.dat");
  AddFrame(m_FileName5, SingleLayout);

  TGHorizontalFrame* IsotopeFrame5 = new TGHorizontalFrame(this);
  AddFrame(IsotopeFrame5, IsotopeFrameLayout);
  
  TGLabel* IsotopeLabel5 = new TGLabel(IsotopeFrame5, "Isotopes:  ");
  IsotopeFrame5->AddFrame(IsotopeLabel5, IsotopeLabelLayout);
  
  m_Isotope51 = new TGComboBox(IsotopeFrame5, c_Isotope51);
  AddIsotopes(m_Isotope51, m_Settings->GetCalibrationFile5Isotope1());
  IsotopeFrame5->AddFrame(m_Isotope51, IsotopeLabelLayout);
  
  m_Isotope52 = new TGComboBox(IsotopeFrame5, c_Isotope52);
  AddIsotopes(m_Isotope52, m_Settings->GetCalibrationFile5Isotope2());
  IsotopeFrame5->AddFrame(m_Isotope52, IsotopeLabelLayout);
  
  m_Isotope53 = new TGComboBox(IsotopeFrame5, c_Isotope53);
  AddIsotopes(m_Isotope53, m_Settings->GetCalibrationFile5Isotope3());
  IsotopeFrame5->AddFrame(m_Isotope53, IsotopeLabelLayout);
 
  m_GroupID5 = new MGUIEEntry(IsotopeFrame5, "Group ID: ", false, (int) m_Settings->GetCalibrationFile5GroupID(), true, 0);
  m_GroupID5->SetEntryFieldSize(m_FontScaler*40);
  IsotopeFrame5->AddFrame(m_GroupID5, GroupIDLabelLayout);
  
  
  m_FileName6 = 
    new MGUIEFileSelector(this,
                          "Sixth calibration file:",
                          m_Settings->GetCalibrationFile6());
  m_FileName6->SetFileType("Calib file", "*.roa");
  m_FileName6->SetFileType("Calib file", "*.dat");
  AddFrame(m_FileName6, SingleLayout);

  TGHorizontalFrame* IsotopeFrame6 = new TGHorizontalFrame(this);
  AddFrame(IsotopeFrame6, IsotopeFrameLayout);
  
  TGLabel* IsotopeLabel6 = new TGLabel(IsotopeFrame6, "Isotopes:  ");
  IsotopeFrame6->AddFrame(IsotopeLabel6, IsotopeLabelLayout);
  
  m_Isotope61 = new TGComboBox(IsotopeFrame6, c_Isotope61);
  AddIsotopes(m_Isotope61, m_Settings->GetCalibrationFile6Isotope1());
  IsotopeFrame6->AddFrame(m_Isotope61, IsotopeLabelLayout);
  
  m_Isotope62 = new TGComboBox(IsotopeFrame6, c_Isotope62);
  AddIsotopes(m_Isotope62, m_Settings->GetCalibrationFile6Isotope2());
  IsotopeFrame6->AddFrame(m_Isotope62, IsotopeLabelLayout);
  
  m_Isotope63 = new TGComboBox(IsotopeFrame6, c_Isotope63);
  AddIsotopes(m_Isotope63, m_Settings->GetCalibrationFile6Isotope3());
  IsotopeFrame6->AddFrame(m_Isotope63, IsotopeLabelLayout);
 
  m_GroupID6 = new MGUIEEntry(IsotopeFrame6, "Group ID: ", false, (int) m_Settings->GetCalibrationFile6GroupID(), true, 0);
  m_GroupID6->SetEntryFieldSize(m_FontScaler*40);
  IsotopeFrame6->AddFrame(m_GroupID6, GroupIDLabelLayout);

  
  m_FileName7 = 
    new MGUIEFileSelector(this,
                          "Seventh calibration file:",
                          m_Settings->GetCalibrationFile7());
  m_FileName7->SetFileType("Calib file", "*.roa");
  m_FileName7->SetFileType("Calib file", "*.dat");
  AddFrame(m_FileName7, SingleLayout);

  TGHorizontalFrame* IsotopeFrame7 = new TGHorizontalFrame(this);
  AddFrame(IsotopeFrame7, IsotopeFrameLayout);
  
  TGLabel* IsotopeLabel7 = new TGLabel(IsotopeFrame7, "Isotopes:  ");
  IsotopeFrame7->AddFrame(IsotopeLabel7, IsotopeLabelLayout);
  
  m_Isotope71 = new TGComboBox(IsotopeFrame7, c_Isotope71);
  AddIsotopes(m_Isotope71, m_Settings->GetCalibrationFile7Isotope1());
  IsotopeFrame7->AddFrame(m_Isotope71, IsotopeLabelLayout);
  
  m_Isotope72 = new TGComboBox(IsotopeFrame7, c_Isotope72);
  AddIsotopes(m_Isotope72, m_Settings->GetCalibrationFile7Isotope2());
  IsotopeFrame7->AddFrame(m_Isotope72, IsotopeLabelLayout);
  
  m_Isotope73 = new TGComboBox(IsotopeFrame7, c_Isotope73);
  AddIsotopes(m_Isotope73, m_Settings->GetCalibrationFile7Isotope3());
  IsotopeFrame7->AddFrame(m_Isotope73, IsotopeLabelLayout);
 
  m_GroupID7 = new MGUIEEntry(IsotopeFrame7, "Group ID: ", false, (int) m_Settings->GetCalibrationFile7GroupID(), true, 0);
  m_GroupID7->SetEntryFieldSize(m_FontScaler*40);
  IsotopeFrame7->AddFrame(m_GroupID7, GroupIDLabelLayout);
 
  
  TGLabel* Footer1 = new TGLabel(this, "You can add additional isotopes to the file: $(MEGALIB)/resource/libraries/Calibration.isotopes");
  AddFrame(Footer1, FooterLayout);
  
  
  AddOKCancelButtons();

  PositionWindow(GetDefaultWidth(), GetDefaultHeight(), false);

  // and bring it to the screen.
  MapSubwindows();
  MapWindow();  

  Layout();

  fClient->WaitFor(this);
 
  return;
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
  ComboBox->SetWidth(m_FontScaler*80);
  unsigned int Height = ComboBox->GetListBox()->GetNumberOfEntries()*ComboBox->GetListBox()->GetItemVsize();
  ComboBox->GetListBox()->SetHeight(Height);

}


////////////////////////////////////////////////////////////////////////////////


//! Action after the Apply or OK button has been pressed.
bool MGUILoadCalibration::OnApply()
{
  if (m_FileName1->GetFileName() != m_Settings->GetCalibrationFile1()) {
    m_Settings->SetCalibrationFile1(m_FileName1->GetFileName());
  }
  MString Text;
  Text = dynamic_cast<TGTextLBEntry*>(m_Isotope11->GetSelectedEntry())->GetText()->GetString();
  if (Text != m_Settings->GetCalibrationFile1Isotope1()) {
    m_Settings->SetCalibrationFile1Isotope1(Text);
  }
  Text = dynamic_cast<TGTextLBEntry*>(m_Isotope12->GetSelectedEntry())->GetText()->GetString();
  if (Text != m_Settings->GetCalibrationFile1Isotope2()) {
    m_Settings->SetCalibrationFile1Isotope2(Text);
  }
  Text = dynamic_cast<TGTextLBEntry*>(m_Isotope13->GetSelectedEntry())->GetText()->GetString();
  if (Text != m_Settings->GetCalibrationFile1Isotope3()) {
    m_Settings->SetCalibrationFile1Isotope3(Text);
  }
  if (m_GroupID1->GetAsInt() != (int) m_Settings->GetCalibrationFile1GroupID()) {
    m_Settings->SetCalibrationFile1GroupID(m_GroupID1->GetAsInt()); 
  }
 
 
  if (m_FileName2->GetFileName() != m_Settings->GetCalibrationFile2()) {
    m_Settings->SetCalibrationFile2(m_FileName2->GetFileName());
  }
  Text = dynamic_cast<TGTextLBEntry*>(m_Isotope21->GetSelectedEntry())->GetText()->GetString();
  if (Text != m_Settings->GetCalibrationFile2Isotope2()) {
    m_Settings->SetCalibrationFile2Isotope1(Text);
  }
  Text = dynamic_cast<TGTextLBEntry*>(m_Isotope22->GetSelectedEntry())->GetText()->GetString();
  if (Text != m_Settings->GetCalibrationFile2Isotope2()) {
    m_Settings->SetCalibrationFile2Isotope2(Text);
  }
  Text = dynamic_cast<TGTextLBEntry*>(m_Isotope23->GetSelectedEntry())->GetText()->GetString();
  if (Text != m_Settings->GetCalibrationFile2Isotope3()) {
    m_Settings->SetCalibrationFile2Isotope3(Text);
  }
  if (m_GroupID2->GetAsInt() != (int) m_Settings->GetCalibrationFile2GroupID()) {
    m_Settings->SetCalibrationFile2GroupID(m_GroupID2->GetAsInt()); 
  }

  
  if (m_FileName3->GetFileName() != m_Settings->GetCalibrationFile3()) {
    m_Settings->SetCalibrationFile3(m_FileName3->GetFileName());
  }
  Text = dynamic_cast<TGTextLBEntry*>(m_Isotope31->GetSelectedEntry())->GetText()->GetString();
  if (Text != m_Settings->GetCalibrationFile3Isotope1()) {
    m_Settings->SetCalibrationFile3Isotope1(Text);
  }
  Text = dynamic_cast<TGTextLBEntry*>(m_Isotope32->GetSelectedEntry())->GetText()->GetString();
  if (Text != m_Settings->GetCalibrationFile3Isotope2()) {
    m_Settings->SetCalibrationFile3Isotope2(Text);
  }
  Text = dynamic_cast<TGTextLBEntry*>(m_Isotope33->GetSelectedEntry())->GetText()->GetString();
  if (Text != m_Settings->GetCalibrationFile3Isotope3()) {
    m_Settings->SetCalibrationFile3Isotope3(Text);
  }
  if (m_GroupID3->GetAsInt() != (int) m_Settings->GetCalibrationFile3GroupID()) {
    m_Settings->SetCalibrationFile3GroupID(m_GroupID3->GetAsInt()); 
  }
 
 
  if (m_FileName4->GetFileName() != m_Settings->GetCalibrationFile4()) {
    m_Settings->SetCalibrationFile4(m_FileName4->GetFileName());
  }
  Text = dynamic_cast<TGTextLBEntry*>(m_Isotope41->GetSelectedEntry())->GetText()->GetString();
  if (Text != m_Settings->GetCalibrationFile4Isotope1()) {
    m_Settings->SetCalibrationFile4Isotope1(Text);
  }
  Text = dynamic_cast<TGTextLBEntry*>(m_Isotope42->GetSelectedEntry())->GetText()->GetString();
  if (Text != m_Settings->GetCalibrationFile4Isotope2()) {
    m_Settings->SetCalibrationFile4Isotope2(Text);
  }
  Text = dynamic_cast<TGTextLBEntry*>(m_Isotope43->GetSelectedEntry())->GetText()->GetString();
  if (Text != m_Settings->GetCalibrationFile4Isotope3()) {
    m_Settings->SetCalibrationFile4Isotope3(Text);
  }
  if (m_GroupID4->GetAsInt() != (int) m_Settings->GetCalibrationFile4GroupID()) {
    m_Settings->SetCalibrationFile4GroupID(m_GroupID4->GetAsInt()); 
  }
 
 
  if (m_FileName5->GetFileName() != m_Settings->GetCalibrationFile5()) {
    m_Settings->SetCalibrationFile5(m_FileName5->GetFileName());
  }
  Text = dynamic_cast<TGTextLBEntry*>(m_Isotope51->GetSelectedEntry())->GetText()->GetString();
  if (Text != m_Settings->GetCalibrationFile5Isotope1()) {
    m_Settings->SetCalibrationFile5Isotope1(Text);
  }
  Text = dynamic_cast<TGTextLBEntry*>(m_Isotope52->GetSelectedEntry())->GetText()->GetString();
  if (Text != m_Settings->GetCalibrationFile5Isotope2()) {
    m_Settings->SetCalibrationFile5Isotope2(Text);
  }
  Text = dynamic_cast<TGTextLBEntry*>(m_Isotope53->GetSelectedEntry())->GetText()->GetString();
  if (Text != m_Settings->GetCalibrationFile5Isotope3()) {
    m_Settings->SetCalibrationFile5Isotope3(Text);
  }
  if (m_GroupID5->GetAsInt() != (int) m_Settings->GetCalibrationFile5GroupID()) {
    m_Settings->SetCalibrationFile5GroupID(m_GroupID5->GetAsInt()); 
  }
 
 
  if (m_FileName6->GetFileName() != m_Settings->GetCalibrationFile6()) {
    m_Settings->SetCalibrationFile6(m_FileName6->GetFileName());
  }
  Text = dynamic_cast<TGTextLBEntry*>(m_Isotope61->GetSelectedEntry())->GetText()->GetString();
  if (Text != m_Settings->GetCalibrationFile6Isotope1()) {
    m_Settings->SetCalibrationFile6Isotope1(Text);
  }
  Text = dynamic_cast<TGTextLBEntry*>(m_Isotope62->GetSelectedEntry())->GetText()->GetString();
  if (Text != m_Settings->GetCalibrationFile6Isotope2()) {
    m_Settings->SetCalibrationFile6Isotope2(Text);
  }
  Text = dynamic_cast<TGTextLBEntry*>(m_Isotope63->GetSelectedEntry())->GetText()->GetString();
  if (Text != m_Settings->GetCalibrationFile6Isotope3()) {
    m_Settings->SetCalibrationFile6Isotope3(Text);
  }
  if (m_GroupID6->GetAsInt() != (int) m_Settings->GetCalibrationFile6GroupID()) {
    m_Settings->SetCalibrationFile6GroupID(m_GroupID6->GetAsInt()); 
  }
 
 
  if (m_FileName7->GetFileName() != m_Settings->GetCalibrationFile7()) {
    m_Settings->SetCalibrationFile7(m_FileName7->GetFileName());
  }
  Text = dynamic_cast<TGTextLBEntry*>(m_Isotope71->GetSelectedEntry())->GetText()->GetString();
  if (Text != m_Settings->GetCalibrationFile7Isotope1()) {
    m_Settings->SetCalibrationFile7Isotope1(Text);
  }
  Text = dynamic_cast<TGTextLBEntry*>(m_Isotope72->GetSelectedEntry())->GetText()->GetString();
  if (Text != m_Settings->GetCalibrationFile7Isotope2()) {
    m_Settings->SetCalibrationFile7Isotope2(Text);
  }
  Text = dynamic_cast<TGTextLBEntry*>(m_Isotope73->GetSelectedEntry())->GetText()->GetString();
  if (Text != m_Settings->GetCalibrationFile7Isotope3()) {
    m_Settings->SetCalibrationFile7Isotope3(Text);
  }
  if (m_GroupID7->GetAsInt() != (int) m_Settings->GetCalibrationFile7GroupID()) {
    m_Settings->SetCalibrationFile7GroupID(m_GroupID7->GetAsInt()); 
  }
 
  
	return true;
}


// MGUILoadCalibration: the end...
////////////////////////////////////////////////////////////////////////////////
