/*
 * MGUIAbout.cxx
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
// MGUIAbout
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MGUIAbout.h"

// Standard libs:
#include <sstream>
using namespace std;

// ROOT libs:
#include "TGPicture.h"
#include "TGTab.h"

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIEText.h"
#include "MStreams.h"
#include "MParser.h"
#include "MFile.h"

// Test:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MGUIAbout)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIAbout::MGUIAbout(const TGWindow* Parent, const TGWindow* Main)
  : MGUIDialog(Parent, Main)
{
  // Construct an instance of MGUIAbout and bring it to the screen

  // use hierarchical cleaning
  SetCleanup(kDeepCleanup);

  m_ProgramName = "MEGAlib"; 
  m_IconPath = ""; 
  m_LeadProgrammer = "";
  m_Programmers = "";
  m_AdditionalProgrammers = "";
  m_Email = "zog@ssl.berkeley.edu";
  m_Updates = "You can find the latest version of MEGAlib at\n" + g_Homepage;
  m_Copyright = "(C) by Andreas Zoglauer and contributors\nAll rights reserved";
  m_MasterReference = "A. Zoglauer et al., \"MEGAlib - The Medium Energy Gamma-ray\nAstronomy Library\", NewAR 50 (7-8), 629-632, 2006";
}


////////////////////////////////////////////////////////////////////////////////


MGUIAbout::~MGUIAbout()
{
  // kDeepCleanup is activated
}


////////////////////////////////////////////////////////////////////////////////


void MGUIAbout::Create()
{
  // Create the main window

  // We start with a name and an icon...
  ostringstream str;
  str<<"About "<<m_ProgramName;

  SetWindowName(str.str().c_str());  

  // Set the tabs:
  TGTab* MainTab = new TGTab(this, 300, 300);
  TGLayoutHints* MainTabLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX | kLHintsExpandY, 10, 10, 10, 10);
  AddFrame(MainTab, MainTabLayout);

  TGCompositeFrame* AboutFrame = MainTab->AddTab("About");
  TGCompositeFrame* ReferencesFrame = MainTab->AddTab("References");
  TGCompositeFrame* PeopleFrame = MainTab->AddTab("People");
  TGCompositeFrame* BugsFrame = MainTab->AddTab("Bugs");
  TGCompositeFrame* DisclaimerFrame = MainTab->AddTab("Disclaimer");


  TGLayoutHints* LabelLayout = new TGLayoutHints(kLHintsExpandX, 30, 30, 10, 2);
  TGLayoutHints* MasterReferenceLabelLayout = new TGLayoutHints(kLHintsLeft, 30, 30, 30, 2);
  TGLayoutHints* ReferenceLayout = new TGLayoutHints(kLHintsLeft, 30, 30, 15, 2);
  TGLayoutHints* ReferenceTextLayout = new TGLayoutHints(kLHintsLeft, 30, 30, 2, 2);
  TGLayoutHints* TextLayout = new TGLayoutHints(kLHintsExpandX, 30, 30, 2, 5);
  TGLayoutHints* ReferenceTopicLayout = new TGLayoutHints(kLHintsLeft, 30, 30, 0, 10);
  TGLayoutHints* LastReferenceTopicLayout = new TGLayoutHints(kLHintsLeft, 30, 30, 0, 30);
  TGLayoutHints* VersionLayout = new TGLayoutHints(kLHintsExpandX, 30, 30, 0, 10);
  TGLayoutHints* TitleLayout = new TGLayoutHints(kLHintsCenterX | kLHintsExpandX | kLHintsTop, 30, 30, 20, 15);


  // The about frame:

  if (m_IconPath != "") {
    MFile::ExpandFileName(m_IconPath);
    const TGPicture* m_IconPicture = gClient->GetPicture(m_IconPath);
    if (m_IconPicture == 0) {
      mgui<<"Can't find icon "<<m_IconPath<<"! Aborting!"<<error;
      CloseWindow();
    }
    TGHorizontalFrame* m_IconFrame = new TGHorizontalFrame(AboutFrame, 100, 100);
    AboutFrame->AddFrame(m_IconFrame, TitleLayout);
   
    TGIcon* m_Icon = new TGIcon(m_IconFrame, m_IconPicture, m_IconPicture->GetWidth(), 
                        m_IconPicture->GetHeight());
    TGLayoutHints* m_IconLayout = new TGLayoutHints(kLHintsCenterY | kLHintsCenterX, 0, 0, 0, 0);
    m_IconFrame->AddFrame(m_Icon, m_IconLayout);
  } else {
    FontStruct_t LabelFont;
    LabelFont = gClient->GetFontByName("-*-helvetica-bold-r-*-*-24-*-*-*-*-*-iso8859-1");	
    TGGC* m_Graphics = new TGGC(TGLabel::GetDefaultGC());
    m_Graphics->SetFont(gVirtualX->GetFontHandle(LabelFont));
    TGLabel* m_Title = new TGLabel(AboutFrame, new TGString(m_ProgramName), m_Graphics->GetGC(), LabelFont);
    AboutFrame->AddFrame(m_Title, TitleLayout);
  }

  ostringstream v;
  v<<"MEGAlib version "<<g_VersionString;
  MGUIEText* m_VersionText = new MGUIEText(AboutFrame, v.str().c_str(), MGUIEText::c_Centered);
  AboutFrame->AddFrame(m_VersionText, VersionLayout);


  if (m_LeadProgrammer != "") {
    MGUIEText* m_LeadLabel = new MGUIEText(AboutFrame, "Lead programmer:", MGUIEText::c_Centered, true);
    AboutFrame->AddFrame(m_LeadLabel, LabelLayout);
    MGUIEText* m_LeadText = new MGUIEText(AboutFrame, m_LeadProgrammer, MGUIEText::c_Centered);
    AboutFrame->AddFrame(m_LeadText, TextLayout);
  }

  if (m_Programmers != "") {
    MGUIEText* m_ProgrammersLabel = new MGUIEText(AboutFrame, "Programmers:", MGUIEText::c_Centered, true);
    AboutFrame->AddFrame(m_ProgrammersLabel, LabelLayout);
    MGUIEText* m_ProgrammersText = new MGUIEText(AboutFrame, m_Programmers, MGUIEText::c_Centered);
    AboutFrame->AddFrame(m_ProgrammersText, TextLayout);
  }

  if (m_AdditionalProgrammers != "") {
    MGUIEText* m_AdditionalLabel = new MGUIEText(AboutFrame, "Additional programmers:", MGUIEText::c_Centered, true);
    AboutFrame->AddFrame(m_AdditionalLabel, LabelLayout);
    MGUIEText* m_AdditionalText = new MGUIEText(AboutFrame, m_AdditionalProgrammers, MGUIEText::c_Centered);
    AboutFrame->AddFrame(m_AdditionalText, TextLayout);
  }

  if (m_Updates != "") {
    MGUIEText* m_UpdatesText = new MGUIEText(AboutFrame, m_Updates, MGUIEText::c_Centered);
    TGLayoutHints* UpdatesLayout = new TGLayoutHints(kLHintsExpandX, 30, 30, 20, 0);
    AboutFrame->AddFrame(m_UpdatesText, UpdatesLayout);
  }


  if (m_Copyright != "") {
    MGUIEText* m_CopyrightText = new MGUIEText(AboutFrame, m_Copyright, MGUIEText::c_Centered);
    TGLayoutHints* CopyrightLayout = new TGLayoutHints(kLHintsExpandX, 30, 30, 10, 40);
    AboutFrame->AddFrame(m_CopyrightText, CopyrightLayout);
  }

  // Define the window width here:
  int TabWidth = MainTab->GetDefaultWidth();


  // The references frame:

  if (m_MasterReference != "") {
    TGLabel* MasterReferenceLabel = new TGLabel(ReferencesFrame, "Main MEGAlib reference:");
    MasterReferenceLabel->SetTextFont(m_EmphasizedFont);
    MasterReferenceLabel->SetWrapLength(TabWidth);
    ReferencesFrame->AddFrame(MasterReferenceLabel, MasterReferenceLabelLayout);
    TGLabel* MasterReferenceText = new TGLabel(ReferencesFrame, m_MasterReference);
    ReferencesFrame->AddFrame(MasterReferenceText, ReferenceTextLayout);
  }


  if (m_References.size() != 0) {
    MGUIEText* m_ReferencesLabel;
    if (m_MasterReference == "") {
      if (m_References.size() == 1) {
        m_ReferencesLabel = new MGUIEText(ReferencesFrame, "Reference:", MGUIEText::c_Centered, true);
      } else {
        m_ReferencesLabel = new MGUIEText(ReferencesFrame, "References:", MGUIEText::c_Centered, true);
      }
    } else {
      if (m_References.size() == 1) {
        m_ReferencesLabel = new MGUIEText(ReferencesFrame, "Special reference:", MGUIEText::c_Centered, true);
      } else {
        m_ReferencesLabel = new MGUIEText(ReferencesFrame, "Special references:", MGUIEText::c_Centered, true);
      }
    }
    m_ReferencesLabel->SetWrapLength(TabWidth);
    ReferencesFrame->AddFrame(m_ReferencesLabel, ReferenceLayout);
    for (unsigned int r = 0; r < m_References.size(); ++r) {
      TGLabel* Reference = new TGLabel(ReferencesFrame, m_References[r]);
      Reference->SetWrapLength(TabWidth);
      ReferencesFrame->AddFrame(Reference, ReferenceTextLayout);
      TGLabel* ReferenceTopic = new TGLabel(ReferencesFrame, MString("(") + m_Topics[r] + MString(")"));
      ReferenceTopic->SetWrapLength(TabWidth);
      ReferenceTopic->SetTextFont(m_ItalicFont);
      if (r == m_References.size()-1) {
        ReferencesFrame->AddFrame(ReferenceTopic, LastReferenceTopicLayout);
      } else {
        ReferencesFrame->AddFrame(ReferenceTopic, ReferenceTopicLayout);
      }
    }
  }



  // The People frame:

  TGLabel* PeopleIntroLabel = new TGLabel(PeopleFrame, "The MEGAlib developers and contributors:");
  PeopleIntroLabel->SetTextFont(m_EmphasizedFont);
  PeopleIntroLabel->SetWrapLength(TabWidth);
  TGLayoutHints* PeopleIntroLayout = new TGLayoutHints(kLHintsLeft, 30, 30, 30, 5);
  PeopleFrame->AddFrame(PeopleIntroLabel, PeopleIntroLayout);

  // Read the People list from file:
  MString FileName = "$(MEGALIB)/doc/Peoples.txt";
  vector<MString> DeveloperList;
  if (MFile::Exists(FileName) == true) {
    MParser Parser;
   if (Parser.Open(FileName, MFile::c_Read) == true) {

      for (unsigned int i = 0; i < Parser.GetNLines(); ++i) {
        if (Parser.GetTokenizerAt(i)->GetNTokens() == 0) continue;
        if (Parser.GetTokenizerAt(i)->IsTokenAt(0, "NM") == true) {
          DeveloperList.push_back(Parser.GetTokenizerAt(i)->GetTokenAfterAsString(1));
        }
      }
      Parser.Close();    
    } 
  }

  MString People;
  if (DeveloperList.size() > 0) {
    for (unsigned int i = 0; i < DeveloperList.size(); ++i) {
      People += DeveloperList[i];
      if (i != DeveloperList.size()-1) People += ", ";
    }
  } else {
    People = "The file with the developers/contributors is empty --- you are probably using Windows...";
  }

  TGLabel* DeveloperLabel = new TGLabel(PeopleFrame, People);
  DeveloperLabel->SetWrapLength(TabWidth);
  TGLayoutHints* DeveloperLayout = new TGLayoutHints(kLHintsLeft, 30, 30, 0, 5);
  PeopleFrame->AddFrame(DeveloperLabel, DeveloperLayout);

  TGLabel* PeopleExtroLabel = new TGLabel(PeopleFrame, "If you do not appear here, but you think you should, then please write me an email: zog@ssl.berkeley.edu");
  PeopleExtroLabel->SetWrapLength(TabWidth);
  PeopleExtroLabel->SetTextFont(m_ItalicFont);
  TGLayoutHints* PeopleExtroLayout = new TGLayoutHints(kLHintsLeft, 30, 30, 30, 5);
  PeopleFrame->AddFrame(PeopleExtroLabel, PeopleExtroLayout);



  // The bugs frame:

  TGLabel* ProblemsLabel = new TGLabel(BugsFrame, "Troubles with MEGAlib:");
  ProblemsLabel->SetTextFont(m_EmphasizedFont);
  ProblemsLabel->SetWrapLength(TabWidth);
  TGLayoutHints* ProblemsLabelLayout = new TGLayoutHints(kLHintsLeft, 30, 30, 30, 5);
  BugsFrame->AddFrame(ProblemsLabel, ProblemsLabelLayout);

  TGLabel* Problems = new TGLabel(BugsFrame, "No program or documentation is free of bugs and obscurities. Neither is MEGAlib. However, to minimize their occurances, and thus to improve the performance of the program, if you find any problem in the latest version, please report it in a reproducable form to the lead developer:");
  Problems->SetWrapLength(TabWidth);
  TGLayoutHints* ProblemsLayout = new TGLayoutHints(kLHintsLeft, 30, 30, 0, 5);
  BugsFrame->AddFrame(Problems, ProblemsLayout);

  TGLabel* EmailLabel = new TGLabel(BugsFrame, m_Email);
  EmailLabel->SetWrapLength(TabWidth);
  TGLayoutHints* EmailLabelLayout = new TGLayoutHints(kLHintsCenterX, 30, 30, 10, 10);
  BugsFrame->AddFrame(EmailLabel, EmailLabelLayout);

  TGLabel* QuestionsLabel = new TGLabel(BugsFrame, "MEGAlib is a rather complex piece of software, which requires a deep understanding of the simulated and analyzed physical processes and detectors. If you have read the given references and the documentation, and still have questions, feel free to ask me.");
  QuestionsLabel->SetWrapLength(TabWidth);
  TGLayoutHints* QuestionsLabelLayout = new TGLayoutHints(kLHintsLeft, 30, 30, 5, 30);
  BugsFrame->AddFrame(QuestionsLabel, QuestionsLabelLayout);


  // The disclaimer frame:

  TGLabel* DisclaimerLabel = new TGLabel(DisclaimerFrame, "Disclaimer:");
  DisclaimerLabel->SetTextFont(m_EmphasizedFont);
  DisclaimerLabel->SetWrapLength(TabWidth);
  TGLayoutHints* DisclaimerLabelLayout = new TGLayoutHints(kLHintsLeft, 30, 30, 30, 5);
  DisclaimerFrame->AddFrame(DisclaimerLabel, DisclaimerLabelLayout);

  TGLabel* Disclaimer = new TGLabel(DisclaimerFrame, "Finally, the usual disclaimer applies: MEGAlib comes without warranty! Use it at your own risk! If your computer fails during a MEGAlib data simulation/analysis \"stress test\", or if MEGAlib deletes all your data, it's your fault!");
  Disclaimer->SetWrapLength(TabWidth);
  TGLayoutHints* DisclaimerLayout = new TGLayoutHints(kLHintsLeft, 30, 30, 0, 30);
  DisclaimerFrame->AddFrame(Disclaimer, DisclaimerLayout);


  AddButtons(MGUIDialog::c_Ok, false, 0);

  PositionWindow(GetDefaultWidth(), GetDefaultHeight(), false);

  // and bring it to the screen.
  MapSubwindows();
  MapWindow();  

  Layout();

  return;
}


// MGUIAbout: the end...
////////////////////////////////////////////////////////////////////////////////
