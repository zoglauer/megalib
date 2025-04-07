/*
* MGUIMainMelinator.cxx
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


// Include the header:
#include "MGUIMainMelinator.h"

// Standard libs:
#include <sstream>
#include <iostream>
#include <iomanip>
using namespace std;

// ROOT libs:
#include <TSystem.h>
#include <KeySymbols.h>
#include <TApplication.h>
#include <TGPicture.h>
#include <TStyle.h>
#include <TGButton.h>
#include <TGLabel.h>
#include <TGWindow.h>
#include <TGFrame.h>
#include <TGClient.h>
#include <TIterator.h>
#include <TList.h>
#include <TObject.h>
#include <TGResourcePool.h>

// MEGAlib libs:
#include "MStreams.h"
#include "MTimer.h"
#include "MGUIDefaults.h"
#include "MFile.h"
#include "MMath.h"
#include "MGUIAbout.h"
#include "MGUIGeometry.h"
#include "MGUILoadCalibration.h"
#include "MGUIEFileSelector.h"
#include "MIsotope.h"
#include "MIsotopeStore.h"
#include "MCalibrationModel.h"
#include "MCalibrateEnergyFindLines.h"
#include "MCalibrateEnergyDetermineModel.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MGUIMainMelinator)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIMainMelinator::MGUIMainMelinator(MInterfaceMelinator* Interface,
                                     MSettingsMelinator* Settings)
  : TGMainFrame(gClient->GetRoot(), 1200, 700, kVerticalFrame),
    m_Interface(Interface), m_Settings(Settings)
{
  // use hierarchical cleaning
  SetCleanup(kDeepCleanup);

  //BindKey(this, gVirtualX->KeysymToKeycode(kKey_L), kAnyModifier);
  BindKey(this, gVirtualX->KeysymToKeycode(kKey_S), kAnyModifier);
  BindKey(this, gVirtualX->KeysymToKeycode(kKey_E), kAnyModifier);
  BindKey(this, gVirtualX->KeysymToKeycode(kKey_Return), kAnyModifier);
  BindKey(this, gVirtualX->KeysymToKeycode(kKey_Enter), kAnyModifier);
  //BindKey(this, gVirtualX->KeysymToKeycode(kKey_Left), kAnyModifier);
  //BindKey(this, gVirtualX->KeysymToKeycode(kKey_Right), kAnyModifier);
  
  m_ActiveCollection = 0;
  m_ActiveLineFit = 0;
  m_ActiveResultIsEnergy = true;
}


////////////////////////////////////////////////////////////////////////////////


MGUIMainMelinator::~MGUIMainMelinator()
{
  // Deep Cleanup automatically deletes all used GUI elements
}


////////////////////////////////////////////////////////////////////////////////


void MGUIMainMelinator::Create()
{
  // Create the main window

  double FontScaler = MGUIDefaults::GetInstance()->GetFontScaler();
  
  int ScrollBarWidth = 50;
  int ControlColumnWidth = 225 + ScrollBarWidth;

  // Give it a default size
  Resize(FontScaler*1200, 250 + FontScaler*600);
  
  // We start with a name and an icon...
  SetWindowName("Melinator");  
  
  MString Path(g_MEGAlibPath + "/resource/icons/global/Icon.xpm");
  MFile::ExpandFileName(Path);
  SetIconPixmap(Path);

  // Some standard layouts:
  TGLayoutHints* TopLeftLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 0, 0, 0, 3);
  TGLayoutHints* TopExpandXLayout = new TGLayoutHints(kLHintsTop | kLHintsExpandX, 0, 0, FontScaler*3, 3);
  TGLayoutHints* BottomExpandXLayout = new TGLayoutHints(kLHintsBottom | kLHintsExpandX, 10, 2, 20, 17);
  //TGLayoutHints* TopExpandXYLayout = new TGLayoutHints(kLHintsTop | kLHintsExpandX | kLHintsExpandY, 2, 2, 2, 3);
  TGLayoutHints* TopLeftTextLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft, 0, 0, FontScaler*3, 0);
  TGLayoutHints* TopRightLayout = new TGLayoutHints(kLHintsTop | kLHintsExpandX | kLHintsRight, FontScaler*10, 0, 0, 0);
  TGLayoutHints* ButtonLayout = new TGLayoutHints(kLHintsTop | kLHintsRight, 2, 2, FontScaler*3, 3);

  TGLayoutHints* CenterYLeftLayout = new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 0, 0, 0, 0);
  TGLayoutHints* CenterYRightEntryLayout = new TGLayoutHints(kLHintsCenterY | kLHintsRight, 0, 0, 0, 0);

  
  //TGLayoutHints* TopRightLayout = new TGLayoutHints(kLHintsTop | kLHintsRight, 2, 2, 2, 3);

  TGLayoutHints* GroupLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 2, 2 + ScrollBarWidth, 5, 5);
  
  // We have three main columns - the control column on the left, data column in the center, and selection on the right 

  
  // The columns
  TGLayoutHints* ColumnsLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX | kLHintsExpandY, 0, 0, 0, 0);
  TGHorizontalFrame* Columns = new TGHorizontalFrame(this, 150, 1500); //, kRaisedFrame);
  AddFrame(Columns, ColumnsLayout);


  // Left column: control column

  TGLayoutHints* ControlColumnLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandY, 5, 0, 0, 0);
  TGVerticalFrame* ControlColumn = new TGVerticalFrame(Columns, ControlColumnWidth, ControlColumnWidth); //, kRaisedFrame);
  Columns->AddFrame(ControlColumn, ControlColumnLayout);

  
  // In the beginning we build the menus and define their layout, ... 
  TGLayoutHints* MenuBarItemLayoutLeft = new TGLayoutHints(kLHintsTop | kLHintsLeft, 0, 0, 0, 0);
  //TGLayoutHints* MenuBarItemLayoutRight = new TGLayoutHints(kLHintsTop | kLHintsRight, 0, 0, 0, 0);
  
  // We continue with the menu bar and its layout ...
  TGLayoutHints* MenuBarLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 0, 0, 0, 0);
  
  TGMenuBar* MenuBar = new TGMenuBar(ControlColumn, 1, 1, kHorizontalFrame); // | kRaisedFrame);
  ControlColumn->AddFrame(MenuBar, MenuBarLayout);

  TGPopupMenu* MenuFiles = new TGPopupMenu(gClient->GetRoot());
  MenuFiles->AddLabel("Calibration file");
  MenuFiles->AddEntry("Choose files and isotopes", c_ChooseCalibrationFiles);
  MenuFiles->AddEntry("Load files", c_LoadLast);
  MenuFiles->AddSeparator();
  MenuFiles->AddLabel("Calibration result");
  MenuFiles->AddEntry("Save As...", c_SaveAs);
  MenuFiles->AddEntry("Save", c_Save);
  MenuFiles->AddSeparator();
  MenuFiles->AddLabel("Configuration file");
  //MenuFiles->AddEntry("Open", c_LoadConfig);
  MenuFiles->AddEntry("Open -> use command line at launch", c_LoadConfig);
  MenuFiles->AddEntry("Save As", c_SaveConfig);
  MenuFiles->AddSeparator();
  MenuFiles->AddLabel("Geometry file");
  MenuFiles->AddEntry("Open", c_Geometry);
  MenuFiles->AddSeparator();
  MenuFiles->AddEntry("Exit", c_Exit);
  MenuFiles->Associate(this);
  MenuBar->AddPopup("Files", MenuFiles, MenuBarItemLayoutLeft);

  TGPopupMenu* MenuInfo = new TGPopupMenu(fClient->GetRoot());
  MenuInfo->AddEntry("About", c_About);
  MenuInfo->Associate(this);
  MenuBar->AddPopup("Info", MenuInfo, MenuBarItemLayoutLeft);

    
  // Control column
  
  // Title section
  
  // Title
  MString TitleIconName(g_MEGAlibPath + "/resource/icons/melinator/Melinator.xpm");
  MFile::ExpandFileName(TitleIconName);
  
  TGLayoutHints* TitleIconLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsCenterX, 2, 2, 10, 2);
  if (MFile::Exists(TitleIconName) == true) {
    const TGPicture* TitlePicture = fClient->GetPicture(TitleIconName);
    if (TitlePicture == 0) {
      mgui<<"Can't find picture "<<TitleIconName<<"! Aborting!"<<error;
      return;
    }
    TGIcon* TitleIcon = new TGIcon(ControlColumn, TitlePicture, TitlePicture->GetWidth()+2, TitlePicture->GetHeight()+2);
    ControlColumn->AddFrame(TitleIcon, TitleIconLayout);
  } else {
    FontStruct_t LabelFont;
    LabelFont = gClient->GetFontByName("-*-helvetica-bold-r-*-*-24-*-*-*-*-*-iso8859-1"); 
    TGGC* Graphics = new TGGC(TGLabel::GetDefaultGC());
    Graphics->SetFont(gVirtualX->GetFontHandle(LabelFont));
    TGLabel* Title = new TGLabel(ControlColumn, "Melinator", Graphics->GetGC(), LabelFont);
    ControlColumn->AddFrame(Title, TitleIconLayout);
  }

  // Sub-title
  TGLabel* SubTitle = new TGLabel(ControlColumn, "MEGAlib's line calibrator");
  SubTitle->SetTextFont(MGUIDefaults::GetInstance()->GetItalicMediumFont()->GetFontStruct());
  TGLayoutHints* SubTitleLayout = new TGLayoutHints(kLHintsTop | kLHintsCenterX | kLHintsExpandX, 10, 10, 0, 20);
  ControlColumn->AddFrame(SubTitle, SubTitleLayout);

  

  // Options section

  TGLayoutHints* OptionsLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandY, 5, 0, 0, 0);
  TGCanvas* OptionsCanvas = new TGCanvas(Columns, ControlColumnWidth, ControlColumnWidth, 0);

  TGVerticalFrame* OptionsFrame = new TGVerticalFrame(OptionsCanvas->GetViewPort(), ControlColumnWidth, ControlColumnWidth, 0);
  OptionsCanvas->SetContainer(OptionsFrame);
  OptionsCanvas->SetScrolling(TGCanvas::kCanvasScrollVertical);

  ControlColumn->AddFrame(OptionsCanvas, OptionsLayout);


  //  Section: Main spectrum
  TGGroupFrame* HistogramGroup = new TGGroupFrame(OptionsFrame, "Display of spectrum", kVerticalFrame);
  OptionsFrame->AddFrame(HistogramGroup, GroupLayout);
  
  
  TGHorizontalFrame* ADCFrame = new TGHorizontalFrame(HistogramGroup);
  HistogramGroup->AddFrame(ADCFrame, TopLeftLayout);
  
  TGLabel* ADCLabelMin = new TGLabel(ADCFrame, "Range:");
  ADCLabelMin->ChangeOptions(kFixedWidth);
  ADCLabelMin->SetTextJustify(kTextLeft);
  ADCLabelMin->SetWidth(FontScaler*50);  
  ADCFrame->AddFrame(ADCLabelMin, TopLeftTextLayout);
  
  m_HistogramRangeMin = new TGNumberEntry(ADCFrame, m_Settings->GetHistogramMin());
  ADCFrame->AddFrame(m_HistogramRangeMin, TopLeftLayout);
  
  TGLabel* ADCLabelMax = new TGLabel(ADCFrame, "  to  ");
  ADCFrame->AddFrame(ADCLabelMax, TopLeftTextLayout);
  
  m_HistogramRangeMax = new TGNumberEntry(ADCFrame, m_Settings->GetHistogramMax());
  ADCFrame->AddFrame(m_HistogramRangeMax, TopLeftLayout);
  
  
  TGHorizontalFrame* BinningModeFrame = new TGHorizontalFrame(HistogramGroup);
  HistogramGroup->AddFrame(BinningModeFrame, TopLeftLayout);
  
  TGLabel* BinsLabel = new TGLabel(BinningModeFrame, "Binning: ");
  BinsLabel->ChangeOptions(kFixedWidth);
  BinsLabel->SetTextJustify(kTextLeft);
  BinsLabel->SetWidth(FontScaler*50);  
  BinningModeFrame->AddFrame(BinsLabel, TopLeftTextLayout);
  
  m_HistogramBinningMode = new TGComboBox(BinningModeFrame, c_HistogramBinningMode);
  m_HistogramBinningMode->AddEntry("Fixed bins", MMelinator::c_HistogramBinningModeFixedNumberOfBins);
  m_HistogramBinningMode->AddEntry("Fixed cts/bins", MMelinator::c_HistogramBinningModeFixedCountsPerBin);
  m_HistogramBinningMode->AddEntry("Bayesian Block", MMelinator::c_HistogramBinningModeBayesianBlocks);
  m_HistogramBinningMode->Select(m_Settings->GetHistogramBinningMode());
  m_HistogramBinningMode->Associate(this);
  m_HistogramBinningMode->SetHeight(FontScaler*24);
  m_HistogramBinningMode->SetWidth(FontScaler*136);
  m_HistogramBinningMode->GetListBox()->SetHeight(m_HistogramBinningMode->GetListBox()->GetNumberOfEntries()*m_HistogramBinningMode->GetListBox()->GetItemVsize());
  BinningModeFrame->AddFrame(m_HistogramBinningMode, TopLeftLayout);
  
  
  TGHorizontalFrame* BinningValueFrame = new TGHorizontalFrame(HistogramGroup);
  HistogramGroup->AddFrame(BinningValueFrame, TopLeftLayout);
 
  TGLabel* HistogramBinningModeValueIntroLabel = new TGLabel(BinningValueFrame, "with:");
  TGLayoutHints* HistogramBinningModeValueIntroLabelLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft, FontScaler*52, FontScaler*2, FontScaler*3, 0);
  BinningValueFrame->AddFrame(HistogramBinningModeValueIntroLabel, HistogramBinningModeValueIntroLabelLayout);
  
  m_HistogramBinningModeValue = new TGNumberEntry(BinningValueFrame, m_Settings->GetHistogramBinningModeValue());
  m_HistogramBinningModeValue->SetLimits(TGNumberFormat::kNELLimitMinMax, 1, 1000000); 
  BinningValueFrame->AddFrame(m_HistogramBinningModeValue, TopLeftLayout);
  
  m_HistogramBinningModeValueLabel = new TGLabel(BinningValueFrame, "   TBD later   ");
  BinningValueFrame->AddFrame(m_HistogramBinningModeValueLabel, TopLeftTextLayout);
  
  OnSwitchHistogramBinningMode(m_Settings->GetHistogramBinningMode());

  
  TGHorizontalFrame* LogFrame = new TGHorizontalFrame(HistogramGroup);
  HistogramGroup->AddFrame(LogFrame, TopLeftLayout);
 
  
  TGLabel* LogLabel = new TGLabel(LogFrame, "Log: ");
  LogLabel->ChangeOptions(kFixedWidth);
  LogLabel->SetTextJustify(kTextLeft);
  LogLabel->SetWidth(FontScaler*50);  
  LogFrame->AddFrame(LogLabel, TopLeftTextLayout);
  
  m_HistogramLogX = new TGCheckButton(LogFrame, "x-axis", c_HistogramLogX);
  m_HistogramLogX->Associate(this);
  m_HistogramLogX->SetState(m_Settings->GetHistogramLogX() ? kButtonDown : kButtonUp);
  LogFrame->AddFrame(m_HistogramLogX, TopLeftTextLayout);
  
  m_HistogramLogY = new TGCheckButton(LogFrame, "y-axis", c_HistogramLogY);
  m_HistogramLogY->Associate(this);
  m_HistogramLogY->SetState(m_Settings->GetHistogramLogY() ? kButtonDown : kButtonUp);
  LogFrame->AddFrame(m_HistogramLogY, TopLeftTextLayout);
  
  
  m_UpdateHistogramButton = new TGTextButton(HistogramGroup, "Update main spectrum", c_UpdateHistogram); 
  m_UpdateHistogramButton->Associate(this);
  HistogramGroup->AddFrame(m_UpdateHistogramButton, TopExpandXLayout);
  



  // Section: Peak finding
  TGGroupFrame* PeakFindingGroup = new TGGroupFrame(OptionsFrame, "Peak finding options", kVerticalFrame);
  OptionsFrame->AddFrame(PeakFindingGroup, GroupLayout);

  TGHorizontalFrame* PeakFindingPriorFrame = new TGHorizontalFrame(PeakFindingGroup);
  PeakFindingGroup->AddFrame(PeakFindingPriorFrame, TopLeftLayout);

  TGLabel* PeakFindingPriorLabel = new TGLabel(PeakFindingPriorFrame, "Bayesian block prior:  ");
  PeakFindingPriorLabel->SetWrapLength(FontScaler*150);
  PeakFindingPriorFrame->AddFrame(PeakFindingPriorLabel, CenterYLeftLayout);

  m_PeakFindingPrior = new TGNumberEntry(PeakFindingPriorFrame, m_Settings->GetPeakFindingPrior());
  m_PeakFindingPrior->SetLimits(TGNumberFormat::kNELLimitMinMax, 1, 1000000);
  m_PeakFindingPrior->SetWidth(FontScaler*70);
  PeakFindingPriorFrame->AddFrame(m_PeakFindingPrior, CenterYRightEntryLayout);


  TGHorizontalFrame* PeakFindingExcludeFirstNumberOfBinsFrame = new TGHorizontalFrame(PeakFindingGroup);
  PeakFindingGroup->AddFrame(PeakFindingExcludeFirstNumberOfBinsFrame, TopLeftLayout);

  TGLabel* PeakFindingExcludeFirstNumberOfBinsLabel = new TGLabel(PeakFindingExcludeFirstNumberOfBinsFrame, "Number of low-energy bins to exclude:  ");
  PeakFindingExcludeFirstNumberOfBinsLabel->SetWrapLength(FontScaler*150);
  PeakFindingExcludeFirstNumberOfBinsFrame->AddFrame(PeakFindingExcludeFirstNumberOfBinsLabel, CenterYLeftLayout);

  m_PeakFindingExcludeFirstNumberOfBins = new TGNumberEntry(PeakFindingExcludeFirstNumberOfBinsFrame, m_Settings->GetPeakFindingExcludeFirstNumberOfBins());
  m_PeakFindingExcludeFirstNumberOfBins->SetLimits(TGNumberFormat::kNELLimitMinMax, 1, 1000000);
  m_PeakFindingExcludeFirstNumberOfBins->SetWidth(FontScaler*70);
  PeakFindingExcludeFirstNumberOfBinsFrame->AddFrame(m_PeakFindingExcludeFirstNumberOfBins, CenterYRightEntryLayout);


  TGHorizontalFrame* PeakFindingMinimumPeakCountsFrame = new TGHorizontalFrame(PeakFindingGroup);
  PeakFindingGroup->AddFrame(PeakFindingMinimumPeakCountsFrame, TopLeftLayout);

  TGLabel* PeakFindingMinimumPeakCountsLabel = new TGLabel(PeakFindingMinimumPeakCountsFrame, "Minimum counts in peak:  ");
  PeakFindingMinimumPeakCountsLabel->SetWrapLength(FontScaler*150);
  PeakFindingMinimumPeakCountsFrame->AddFrame(PeakFindingMinimumPeakCountsLabel, CenterYLeftLayout);

  m_PeakFindingMinimumPeakCounts = new TGNumberEntry(PeakFindingMinimumPeakCountsFrame, m_Settings->GetPeakFindingMinimumPeakCounts());
  m_PeakFindingMinimumPeakCounts->SetLimits(TGNumberFormat::kNELLimitMinMax, 1, 1000000);
  m_PeakFindingMinimumPeakCounts->SetWidth(FontScaler*70);
  PeakFindingMinimumPeakCountsFrame->AddFrame(m_PeakFindingMinimumPeakCounts, CenterYRightEntryLayout);




  
  //  Section: Secondary spectrum
  TGGroupFrame* PeakGroup = new TGGroupFrame(OptionsFrame, "Display of peak spectrum", kVerticalFrame);
  OptionsFrame->AddFrame(PeakGroup, GroupLayout);
  
  TGHorizontalFrame* PeakBinningModeFrame = new TGHorizontalFrame(PeakGroup);
  PeakGroup->AddFrame(PeakBinningModeFrame, TopLeftLayout);

  TGLabel* PeakBinningLabel = new TGLabel(PeakBinningModeFrame, "Binning: ");
  PeakBinningLabel->ChangeOptions(kFixedWidth);
  PeakBinningLabel->SetTextJustify(kTextLeft);
  PeakBinningLabel->SetWidth(FontScaler*50);  
  PeakBinningModeFrame->AddFrame(PeakBinningLabel, TopLeftTextLayout);
  
  m_PeakHistogramBinningMode = new TGComboBox(PeakBinningModeFrame, c_PeakHistogramBinningMode);
  m_PeakHistogramBinningMode->AddEntry("Fixed bins", MMelinator::c_HistogramBinningModeFixedNumberOfBins);
  m_PeakHistogramBinningMode->AddEntry("Fixed cts/bins", MMelinator::c_HistogramBinningModeFixedCountsPerBin);
  m_PeakHistogramBinningMode->AddEntry("Bayesian Block", MMelinator::c_HistogramBinningModeBayesianBlocks);
  m_PeakHistogramBinningMode->Select(m_Settings->GetPeakHistogramBinningMode());
  m_PeakHistogramBinningMode->Associate(this);
  m_PeakHistogramBinningMode->SetHeight(FontScaler*24);
  //m_PeakHistogramBinningMode->SetWidth(FontScaler*136);
  m_PeakHistogramBinningMode->GetListBox()->SetHeight(m_PeakHistogramBinningMode->GetListBox()->GetNumberOfEntries()*m_PeakHistogramBinningMode->GetListBox()->GetItemVsize());
  PeakBinningModeFrame->AddFrame(m_PeakHistogramBinningMode, TopLeftLayout);
  
  TGHorizontalFrame* PeakBinningValueFrame = new TGHorizontalFrame(PeakGroup);
  PeakGroup->AddFrame(PeakBinningValueFrame, TopLeftLayout);
 
  TGLabel* PeakHistogramBinningModeValueIntroLabel = new TGLabel(PeakBinningValueFrame, "with:  ");
  TGLayoutHints* PeakHistogramBinningModeValueIntroLabelLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft, FontScaler*52, 2, 5, 0);
  PeakBinningValueFrame->AddFrame(PeakHistogramBinningModeValueIntroLabel, PeakHistogramBinningModeValueIntroLabelLayout);
  
  m_PeakHistogramBinningModeValue = new TGNumberEntry(PeakBinningValueFrame, m_Settings->GetPeakHistogramBinningModeValue());
  m_PeakHistogramBinningModeValue->SetLimits(TGNumberFormat::kNELLimitMinMax, 1, 1000000); 
  PeakBinningValueFrame->AddFrame(m_PeakHistogramBinningModeValue, TopLeftLayout);
  
  m_PeakHistogramBinningModeValueLabel = new TGLabel(PeakBinningValueFrame, "  TBD later");
  PeakBinningValueFrame->AddFrame(m_PeakHistogramBinningModeValueLabel, TopLeftTextLayout);
  
  OnSwitchPeakHistogramBinningMode(m_Settings->GetPeakHistogramBinningMode());

  
  /*
  TGHorizontalFrame* LogFrame = new TGHorizontalFrame(PeakGroup);
  PeakGroup->AddFrame(LogFrame, TopLeftLayout);
  
  TGLabel* LogLabel = new TGLabel(LogFrame, "Log: ");
  LogFrame->AddFrame(LogLabel, TopLeftTextLayout);
  
  m_HistogramLogX = new TGCheckButton(LogFrame, "x-axis", c_HistogramLogX);
  m_HistogramLogX->Associate(this);
  m_HistogramLogX->SetState(m_Settings->GetHistogramLogX() ? kButtonDown : kButtonUp);
  LogFrame->AddFrame(m_HistogramLogX, TopLeftTextLayout);
  
  m_HistogramLogY = new TGCheckButton(LogFrame, "y-axis", c_HistogramLogY);
  m_HistogramLogY->Associate(this);
  m_HistogramLogY->SetState(m_Settings->GetHistogramLogY() ? kButtonDown : kButtonUp);
  LogFrame->AddFrame(m_HistogramLogY, TopLeftTextLayout);
  */
  
  m_PeakHistogramUpdateButton = new TGTextButton(PeakGroup, "Update peak spectrum", c_UpdatePeakHistogram); 
  m_PeakHistogramUpdateButton->Associate(this);
  PeakGroup->AddFrame(m_PeakHistogramUpdateButton, TopExpandXLayout);
  
  
  
  // Peak parametrization section
  TGGroupFrame* FitGroup = new TGGroupFrame(OptionsFrame, "Peak parametrization options", kVerticalFrame);
  OptionsFrame->AddFrame(FitGroup, GroupLayout);

  TGHorizontalFrame* PeakParametrizationMethodFrame = new TGHorizontalFrame(FitGroup);
  FitGroup->AddFrame(PeakParametrizationMethodFrame, TopLeftLayout);

  TGLabel* PeakParametrizationMethodLabel = new TGLabel(PeakParametrizationMethodFrame, "Method: ");
  PeakParametrizationMethodLabel->ChangeOptions(kFixedWidth);
  PeakParametrizationMethodLabel->SetTextJustify(kTextLeft);
  PeakParametrizationMethodLabel->SetWidth(FontScaler*70);
  PeakParametrizationMethodFrame->AddFrame(PeakParametrizationMethodLabel, TopLeftTextLayout);
  
  m_PeakParametrizationMethod = new TGComboBox(PeakParametrizationMethodFrame, c_PeakParametrizationMethod);
  m_PeakParametrizationMethod->AddEntry("Bayesian Blocks", MCalibrateEnergyFindLines::c_PeakParametrizationMethodBayesianBlockPeak);
  m_PeakParametrizationMethod->AddEntry("Smoothing", MCalibrateEnergyFindLines::c_PeakParametrizationMethodSmoothedPeak);
  m_PeakParametrizationMethod->AddEntry("Fitting", MCalibrateEnergyFindLines::c_PeakParametrizationMethodFittedPeak);
  m_PeakParametrizationMethod->Select(m_Settings->GetPeakParametrizationMethod());
  m_PeakParametrizationMethod->Associate(this);
  m_PeakParametrizationMethod->SetHeight(FontScaler*24);
  //m_PeakParametrizationMethod->SetWidth(FontScaler*136);
  m_PeakParametrizationMethod->GetListBox()->SetHeight(m_PeakParametrizationMethod->GetListBox()->GetNumberOfEntries()*m_PeakParametrizationMethod->GetListBox()->GetItemVsize());
  PeakParametrizationMethodFrame->AddFrame(m_PeakParametrizationMethod, TopRightLayout);
  
  m_PeakParametrizationOptions = new TGCompositeFrame(FitGroup);
  FitGroup->AddFrame(m_PeakParametrizationOptions, TopLeftLayout);
  
  OnSwitchPeakParametrizationMode(m_Settings->GetPeakParametrizationMethod());
  
  
  // Calibration model determination section
  TGGroupFrame* CalibrationModel = new TGGroupFrame(OptionsFrame, "Calibration model options", kVerticalFrame);
  OptionsFrame->AddFrame(CalibrationModel, GroupLayout);

  m_CalibrationModelZeroCrossing = new TGCheckButton(CalibrationModel, "Energy assignment only: Assume the energy calibration crosses (0/0)", c_CalibrationModelZeroCrossing);
  m_CalibrationModelZeroCrossing->Associate(this);
  m_CalibrationModelZeroCrossing->SetWrapLength(FontScaler*200);
  m_CalibrationModelZeroCrossing->SetState(m_Settings->GetCalibrationModelZeroCrossing() ? kButtonDown : kButtonUp);
  CalibrationModel->AddFrame(m_CalibrationModelZeroCrossing, TopLeftLayout);

  TGHorizontalFrame* CalibrationModelDeterminationFrame = new TGHorizontalFrame(CalibrationModel);
  CalibrationModel->AddFrame(CalibrationModelDeterminationFrame, TopLeftLayout);
  
  TGLabel* CalibrationModelDeterminationLabel = new TGLabel(CalibrationModelDeterminationFrame, "Method: ");
  CalibrationModelDeterminationLabel->ChangeOptions(kFixedWidth);
  CalibrationModelDeterminationLabel->SetTextJustify(kTextLeft);
  CalibrationModelDeterminationLabel->SetWidth(FontScaler*70);
  CalibrationModelDeterminationFrame->AddFrame(CalibrationModelDeterminationLabel, TopLeftTextLayout);
  
  m_CalibrationModelDeterminationMethod = new TGComboBox(CalibrationModelDeterminationFrame, c_CalibrationModelDeterminationMethod);
  m_CalibrationModelDeterminationMethod->AddEntry("Interpolation", MCalibrateEnergyDetermineModel::c_CalibrationModelStepWise);
  m_CalibrationModelDeterminationMethod->AddEntry("Fitting", MCalibrateEnergyDetermineModel::c_CalibrationModelFit);
  m_CalibrationModelDeterminationMethod->AddEntry("Select best fit", MCalibrateEnergyDetermineModel::c_CalibrationModelBestFit);
  m_CalibrationModelDeterminationMethod->Select(m_Settings->GetCalibrationModelDeterminationMethod());
  m_CalibrationModelDeterminationMethod->Associate(this);
  m_CalibrationModelDeterminationMethod->SetHeight(FontScaler*24);
  //m_CalibrationModelDeterminationMethod->SetWidth(FontScaler*136);
  m_CalibrationModelDeterminationMethod->GetListBox()->SetHeight(m_CalibrationModelDeterminationMethod->GetListBox()->GetNumberOfEntries()*m_CalibrationModelDeterminationMethod->GetListBox()->GetItemVsize());
  CalibrationModelDeterminationFrame->AddFrame(m_CalibrationModelDeterminationMethod, TopRightLayout);
  
  m_CalibrationModelDeterminationOptions = new TGCompositeFrame(CalibrationModel);
  CalibrationModel->AddFrame(m_CalibrationModelDeterminationOptions, TopLeftLayout);
  
  OnSwitchCalibrationModelDeterminationMode(m_Settings->GetCalibrationModelDeterminationMethod());
  
  
  
  // The final "Calibration all" button
  m_FitAllButton = new TGTextButton(ControlColumn, "Run full calibration", c_FitAll); 
  m_FitAllButton->Associate(this);
  m_FitAllButton->SetHeight(FontScaler*25);
  ControlColumn->AddFrame(m_FitAllButton, BottomExpandXLayout);
  
  
  
  
  
  // 2. Data column
    
  TGLayoutHints* DataColumnLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX | kLHintsExpandY, 5, 5, 5, 5);
  TGVerticalFrame* DataColumn = new TGVerticalFrame(Columns, 100, 100); //, kRaisedFrame);
  Columns->AddFrame(DataColumn, DataColumnLayout);

  TGLayoutHints* CanvasLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX | kLHintsExpandY, 2, 2, 2, 2);

  // Main view:
  TGVerticalFrame* MainDataView = new TGVerticalFrame(DataColumn, 100, 100); //, kRaisedFrame);
  DataColumn->AddFrame(MainDataView, DataColumnLayout); 
  

  TGHorizontalFrame* CanvasControl = new TGHorizontalFrame(MainDataView, 100, 100); //, kRaisedFrame);
  TGLayoutHints* CanvasControlLayout =  new TGLayoutHints(kLHintsTop | kLHintsExpandX | kLHintsCenterX, 0, 0, 0, 0);
  MainDataView->AddFrame(CanvasControl, CanvasControlLayout);

  // The buttons itself
  
  m_MainHistogramLabel = new TGLabel(CanvasControl, "Load some calibration data to display its histogram here...");
  CanvasControl->AddFrame(m_MainHistogramLabel, TopLeftTextLayout);
  
  m_ForwardButton = new TGTextButton(CanvasControl, "Next", c_Next); 
  m_ForwardButton->Associate(this);
  m_ForwardButton->SetWidth(100);
  m_ForwardButton->SetEnabled(false);
  m_ForwardButton->SetMargins(FontScaler*5, FontScaler*5);
  CanvasControl->AddFrame(m_ForwardButton, ButtonLayout);

  m_BackButton = new TGTextButton(CanvasControl, "Back", c_Back); 
  m_BackButton->Associate(this);
  m_BackButton->SetMinWidth(100);
  m_BackButton->SetEnabled(false);
  m_BackButton->SetMargins(FontScaler*5, FontScaler*5);
  CanvasControl->AddFrame(m_BackButton, ButtonLayout);

  TGTextButton* FitButton = new TGTextButton(CanvasControl, "Parametrize", c_Fit); 
  FitButton->Associate(this);
  FitButton->SetWidth(100);
  FitButton->SetMargins(FontScaler*5, FontScaler*5);
  CanvasControl->AddFrame(FitButton, ButtonLayout);

  TGTextButton* FitWithDiagnosticsButton = new TGTextButton(CanvasControl, "Parametrize with diagnostics", c_FitWithDiagnostics); 
  FitWithDiagnosticsButton->Associate(this);
  FitWithDiagnosticsButton->SetWidth(100);
  FitWithDiagnosticsButton->SetMargins(FontScaler*5, FontScaler*5);
  CanvasControl->AddFrame(FitWithDiagnosticsButton, ButtonLayout);

  m_SpectrumCanvas = new MGUIEReadOutUnitsCanvas(this, "SpectrumCanvas", MainDataView);
  MainDataView->AddFrame(m_SpectrumCanvas, CanvasLayout);

    
  // The calibration view:
  TGHorizontalFrame* CalibrationView = new TGHorizontalFrame(DataColumn, 100, 100); //, kRaisedFrame);
  DataColumn->AddFrame(CalibrationView, DataColumnLayout); 
    
  // The fit view:
  TGVerticalFrame* FitView = new TGVerticalFrame(CalibrationView, 100, 100); //, kRaisedFrame);
  CalibrationView->AddFrame(FitView, DataColumnLayout); 

  TGHorizontalFrame* FitControl = new TGHorizontalFrame(FitView, 100, 100); //, kRaisedFrame);
  TGLayoutHints* FitControlLayout =  new TGLayoutHints(kLHintsTop | kLHintsExpandX | kLHintsCenterX, 0, 0, 0, 0);
  FitView->AddFrame(FitControl, FitControlLayout);
  
  m_FitHistogramLabel = new TGLabel(FitControl, "Fit the data first...");
  FitControl->AddFrame(m_FitHistogramLabel, TopLeftTextLayout);
  
  m_FitForwardButton = new TGTextButton(FitControl, "Next", c_NextFit); 
  m_FitForwardButton->Associate(this);
  m_FitForwardButton->SetWidth(100);
  m_FitForwardButton->SetEnabled(false);
  m_FitForwardButton->SetMargins(FontScaler*5, FontScaler*5);
  FitControl->AddFrame(m_FitForwardButton, ButtonLayout);

  m_FitBackButton = new TGTextButton(FitControl, "Back", c_PreviousFit); 
  m_FitBackButton->Associate(this);
  m_FitBackButton->SetMinWidth(100);
  m_FitBackButton->SetEnabled(false);
  m_FitBackButton->SetMargins(FontScaler*5, FontScaler*5);
  FitControl->AddFrame(m_FitBackButton, ButtonLayout);

  m_FitToggleButton = new TGTextButton(FitControl, "Toggle", c_ToggleFit); 
  m_FitToggleButton->Associate(this);
  m_FitToggleButton->SetMinWidth(100);
  m_FitToggleButton->SetEnabled(false);
  m_FitToggleButton->SetMargins(FontScaler*5, FontScaler*5);
  FitControl->AddFrame(m_FitToggleButton, ButtonLayout);

  m_FitCanvas = new TRootEmbeddedCanvas("FitCanvas", FitView, 100, 100);
  FitView->AddFrame(m_FitCanvas, CanvasLayout);

  
    
  // The results view:
  TGVerticalFrame* ResultsView = new TGVerticalFrame(CalibrationView, 100, 100); //, kRaisedFrame);
  CalibrationView->AddFrame(ResultsView, DataColumnLayout); 


  TGHorizontalFrame* ResultsControl = new TGHorizontalFrame(ResultsView, 100, 100); //, kRaisedFrame);
  TGLayoutHints* ResultsControlLayout =  new TGLayoutHints(kLHintsTop | kLHintsExpandX | kLHintsCenterX, 0, 0, 0, 0);
  ResultsView->AddFrame(ResultsControl, ResultsControlLayout);
  
  m_ResultsHistogramLabel = new TGLabel(ResultsControl, "Calibration model: none");
  TGLayoutHints* ResultsLabelLayout =  new TGLayoutHints(kLHintsTop | kLHintsLeft, 0, 0, 4+FontScaler*2, 3+FontScaler*2);
  ResultsControl->AddFrame(m_ResultsHistogramLabel, ResultsLabelLayout);
  
  m_ResultsToggleButton = new TGTextButton(ResultsControl, "Toggle", c_ResultsToggle); 
  m_ResultsToggleButton->Associate(this);
  m_ResultsToggleButton->SetWidth(100);
  //m_ResultsToggleButton->SetEnabled(false);
  m_ResultsToggleButton->SetMargins(FontScaler*5, FontScaler*5);
  ResultsControl->AddFrame(m_ResultsToggleButton, ButtonLayout);
  
  m_ResultsCanvas = new MGUIEReadOutUnitsCanvas(this, "ResultsCanvas", ResultsView);
  ResultsView->AddFrame(m_ResultsCanvas, CanvasLayout);
  
  
  
  // 3. Selection column
    
  
  int SelectionColumnWidth = 135;
  
  
  TGLayoutHints* SelectionColumnLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandY, 10, 5, 14, 12);
  TGVerticalFrame* SelectionColumn = new TGVerticalFrame(Columns, SelectionColumnWidth, SelectionColumnWidth); //, kRaisedFrame);
  Columns->AddFrame(SelectionColumn, SelectionColumnLayout);
  
  TGLabel* MainSelectionLabel = new TGLabel(SelectionColumn, "Select a read-out element:");
  TGLayoutHints* MainSelectionLabelLayout = new TGLayoutHints(kLHintsTop | kLHintsExpandX, 0, 20, 2, 20);
  SelectionColumn->AddFrame(MainSelectionLabel, MainSelectionLabelLayout);

  TGLabel* RMSLabel = new TGLabel(SelectionColumn, "Color coding in RMS of final fit:");
  TGLayoutHints* RMSTitlelLayout = new TGLayoutHints(kLHintsTop | kLHintsExpandX, 0, 20, 2, 5);
  SelectionColumn->AddFrame(RMSLabel, RMSTitlelLayout);

  TGHorizontalFrame* RMSFrame = new TGHorizontalFrame(SelectionColumn, SelectionColumnWidth, SelectionColumnWidth); //, kRaisedFrame);
  TGLayoutHints* RMSFrameLayout = new TGLayoutHints(kLHintsTop | kLHintsExpandX, 5, 20, 2, 20);
  SelectionColumn->AddFrame(RMSFrame, RMSFrameLayout);

  TGLayoutHints* RMSLabelLayout = new TGLayoutHints(kLHintsCenterY | kLHintsExpandX, 0, 0, 0, 0);
 
  TGLabel* SmallerOneLabel = new TGLabel(RMSFrame, "0-1");
  SmallerOneLabel->SetBackgroundColor(gROOT->GetColor(kGreen+1)->GetPixel());
  SmallerOneLabel->SetMargins(0, 0, 5, 0);
  RMSFrame->AddFrame(SmallerOneLabel, RMSLabelLayout);

  TGLabel* SmallerTwoLabel = new TGLabel(RMSFrame, "1-2");
  SmallerTwoLabel->SetBackgroundColor(gROOT->GetColor(kSpring)->GetPixel());
  SmallerTwoLabel->SetMargins(0, 0, 5, 0);
  RMSFrame->AddFrame(SmallerTwoLabel, RMSLabelLayout);

  TGLabel* SmallerThreeLabel = new TGLabel(RMSFrame, "2-3");
  SmallerThreeLabel->SetBackgroundColor(gROOT->GetColor(kYellow)->GetPixel());
  SmallerThreeLabel->SetMargins(0, 0, 5, 0);
  RMSFrame->AddFrame(SmallerThreeLabel, RMSLabelLayout);

  TGLabel* SmallerFourLabel = new TGLabel(RMSFrame, "3-4");
  SmallerFourLabel->SetBackgroundColor(gROOT->GetColor(kOrange-2)->GetPixel());
  SmallerFourLabel->SetMargins(0, 0, 5, 0);
  RMSFrame->AddFrame(SmallerFourLabel, RMSLabelLayout);

  TGLabel* SmallerFiveLabel = new TGLabel(RMSFrame, "4-5");
  SmallerFiveLabel->SetBackgroundColor(gROOT->GetColor(kOrange+1)->GetPixel());
  SmallerFiveLabel->SetMargins(0, 0, 5, 0);
  RMSFrame->AddFrame(SmallerFiveLabel, RMSLabelLayout);

  TGLabel* LargerFiveLabel = new TGLabel(RMSFrame, "5+");
  LargerFiveLabel->SetBackgroundColor(gROOT->GetColor(kRed)->GetPixel());
  LargerFiveLabel->SetMargins(0, 0, 5, 0);
  RMSFrame->AddFrame(LargerFiveLabel, RMSLabelLayout);

  TGLabel* BadLabel = new TGLabel(RMSFrame, "Bad");
  BadLabel->SetBackgroundColor(gROOT->GetColor(kGray+1)->GetPixel());  
  BadLabel->SetMargins(0, 0, 5, 0);
  RMSFrame->AddFrame(BadLabel, RMSLabelLayout);
  
  // Main view:
  TGLayoutHints* MainSelectionCanvasLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX | kLHintsExpandY, 0, 0, 5, 5);
  m_MainSelectionCanvas = new MGUIEReadOutElementView(SelectionColumn);
  m_MainSelectionCanvas->Associate(this);
  SelectionColumn->AddFrame(m_MainSelectionCanvas, MainSelectionCanvasLayout); 
  
  CreateSelection();
  
  
  
  MapSubwindows();
  MapWindow();  
  Layout();

  return;
}


////////////////////////////////////////////////////////////////////////////////


//! Create the selection GUI element
void MGUIMainMelinator::CreateSelection()
{
  m_MainSelectionCanvas->ClearReadOutElements();
  
  for (unsigned int c = 0; c < m_Melinator.GetNumberOfCollections(); ++c) {
    //if (c > 10) break;
    MReadOutCollection& C = m_Melinator.GetCollection(c);
    m_MainSelectionCanvas->Add(C.GetReadOutElement(), c+c_ROEButtons);
  }

  m_MainSelectionCanvas->Create();
  
  MapSubwindows();
  MapWindow();  
  Layout();
}


////////////////////////////////////////////////////////////////////////////////


//! Handle all key presses
bool MGUIMainMelinator::HandleKey(Event_t* Event)
{
  // Here we handle all keys...

  char   tmp[10];
  unsigned int keysym;

  // Test if we have a key release:
  if (Event->fType != kKeyRelease) return false;

  // First we get the key...
  gVirtualX->LookupString(Event, tmp, sizeof(tmp), keysym);
  
  // ... and than we do what we need to do...
  
  // The following keys need an initialized hardware
  switch ((EKeySym) keysym) {
  case kKey_Escape:
    OnExit();
    break;
  case kKey_Return:
  case kKey_Enter:
    //OnStart();
    break;
  case kKey_l:
  case kKey_L:
    //OnXAxis(true);
    break;
  case kKey_s:
  case kKey_S:
    OnSaveConfiguration();
    break;
  case kKey_Left:
    OnBack();
    break;
  case kKey_Right:
    OnNext();
    break;
  default:
    break;
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIMainMelinator::ProcessMessage(long Message, long Parameter1, long Parameter2)
{
  // Process the messages for this application

  //cout<<Message<<":"<<Parameter1<<":"<<Parameter2<<endl;
  
  bool Status = true;

  switch (GET_MSG(Message)) {
  case kC_COMMAND:
    switch (GET_SUBMSG(Message)) {
    case kCM_BUTTON:
      if (Parameter1 >= c_ROEButtons) {
        Status = OnSwitchCollection(Parameter1 - c_ROEButtons);
      } else {
        switch (Parameter1) {
        case c_Exit:
          Status = OnExit();
          break;

        case c_Back:
          Status = OnBack();
          break;

        case c_Next:
          Status = OnNext();
          break;

        case c_PreviousFit:
          Status = OnPreviousFit();
          break;

        case c_NextFit:
          Status = OnNextFit();
          break;

        case c_ToggleFit:
          Status = OnToggleFit();
          break;

        case c_Fit:
          Status = OnFit();
          break;

        case c_FitWithDiagnostics:
          Status = OnFitWithDiagnostics();
          break;

        case c_FitAll:
          Status = OnFitAll();
          break;

        case c_UpdateHistogram:
          Status = OnUpdateHistogram();
          break;
          
        case c_UpdatePeakHistogram:
          Status = OnUpdatePeakHistogram();
          break;
          
        case c_ResultsToggle:
          Status = OnToggleResults();
          break;
          
        default:
          break;
        }
      }
      
    case kCM_RADIOBUTTON:
      break;

    case kCM_CHECKBUTTON:
      switch (Parameter1) {
        case c_HistogramLogX:
          if (m_HistogramLogX->GetState() == kButtonDown) {
            OnXAxis(true);
          } else if (m_HistogramLogX->GetState() == kButtonUp) {
            OnXAxis(false);
          }
          break;
        case c_HistogramLogY:
          if (m_HistogramLogY->GetState() == kButtonDown) {
            OnYAxis(true);
          } else if (m_HistogramLogY->GetState() == kButtonUp) {
            OnYAxis(false);
          }
          break;
        default:
          break;
      }
      break;

    case kCM_COMBOBOX:
      switch (Parameter1) {
        case c_HistogramBinningMode:
          OnSwitchHistogramBinningMode((unsigned int) Parameter2);
          break;
        case c_PeakHistogramBinningMode:
          OnSwitchPeakHistogramBinningMode((unsigned int) Parameter2);
          break;
        case c_PeakParametrizationMethod:
          OnSwitchPeakParametrizationMode((unsigned int) Parameter2);
          break;
        case c_CalibrationModelDeterminationMethod:
          OnSwitchCalibrationModelDeterminationMode((unsigned int) Parameter2);
          break;
        default:
          break;
      } 
      break;

    case kCM_MENU:
      switch (Parameter1) {

      case c_LoadConfig:
        Status = OnLoadConfiguration();
        break;

      case c_SaveConfig:
        Status = OnSaveConfiguration();
        break;

      case c_Geometry:
        Status = OnGeometry();
        break;

      case c_Exit:
        Status = OnExit();
        break;

      case c_ChooseCalibrationFiles:
        Status = OnChooseCalibrationFiles();
        break;

      case c_LoadLast:
        Status = OnLoadLast();
        break;

      case c_Save:
        Status = OnSave();
        break;

      case c_SaveAs:
        Status = OnSaveAs();
        break;

      case c_About:
        Status = OnAbout();
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
  
  return Status;
}


////////////////////////////////////////////////////////////////////////////////


void MGUIMainMelinator::CloseWindow()
{
  // Call exit for controlled good-bye

  OnExit();
}


////////////////////////////////////////////////////////////////////////////////


//! Update the setting with the GUI data
bool MGUIMainMelinator::UpdateSettings()
{
  m_Settings->SetHistogramMin(m_HistogramRangeMin->GetNumber());
  m_Settings->SetHistogramMax(m_HistogramRangeMax->GetNumber());
  m_Settings->SetHistogramBinningMode(m_HistogramBinningMode->GetSelected());
  m_Settings->SetHistogramBinningModeValue(m_HistogramBinningModeValue->GetNumber());
  if (m_HistogramLogX->GetState() == kButtonDown) {
    m_Settings->SetHistogramLogX(true);
  } else if (m_HistogramLogX->GetState() == kButtonUp) {
    m_Settings->SetHistogramLogX(false);
  }
  if (m_HistogramLogY->GetState() == kButtonDown) {
    m_Settings->SetHistogramLogY(true);
  } else if (m_HistogramLogY->GetState() == kButtonUp) {
    m_Settings->SetHistogramLogY(false);
  }
  m_Settings->SetPeakHistogramBinningMode(m_PeakHistogramBinningMode->GetSelected());
  m_Settings->SetPeakHistogramBinningModeValue(m_PeakHistogramBinningModeValue->GetNumber());
  
  m_Settings->SetPeakFindingPrior(m_PeakFindingPrior->GetNumber());
  m_Settings->SetPeakFindingExcludeFirstNumberOfBins(m_PeakFindingExcludeFirstNumberOfBins->GetNumber());
  m_Settings->SetPeakFindingMinimumPeakCounts(m_PeakFindingMinimumPeakCounts->GetNumber());

  m_Settings->SetPeakParametrizationMethod(m_PeakParametrizationMethod->GetSelected());
  if (m_PeakParametrizationMethod->GetSelected() == MCalibrateEnergyFindLines::c_PeakParametrizationMethodFittedPeak) {
    m_Settings->SetPeakParametrizationMethodFittingBackgroundModel(m_PeakParametrizationMethodFittingBackgroundModel->GetSelected());
    m_Settings->SetPeakParametrizationMethodFittingEnergyLossModel(m_PeakParametrizationMethodFittingEnergyLossModel->GetSelected());
    m_Settings->SetPeakParametrizationMethodFittingPeakShapeModel(m_PeakParametrizationMethodFittingPeakShapeModel->GetSelected());
  }
  
  m_Settings->SetCalibrationModelZeroCrossing(m_CalibrationModelZeroCrossing->GetState() == kButtonDown ? true : false);
  m_Settings->SetCalibrationModelDeterminationMethod(m_CalibrationModelDeterminationMethod->GetSelected());
  if (m_CalibrationModelDeterminationMethod->GetSelected() == MCalibrateEnergyDetermineModel::c_CalibrationModelFit) {
    m_Settings->SetCalibrationModelDeterminationMethodFittingEnergyModel(m_CalibrationModelDeterminationMethodFittingEnergyModel->GetSelected());
    m_Settings->SetCalibrationModelDeterminationMethodFittingFWHMModel(m_CalibrationModelDeterminationMethodFittingFWHMModel->GetSelected());
  }
  
  
  m_Settings->Write();
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Close the applictaion
bool MGUIMainMelinator::OnExit()
{
  UpdateSettings();
  m_Settings->Write(); // Update alreday writes, but it is logical that we write here too so keep it
  
  //m_Interface->Exit();
  gApplication->Terminate(0);

  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Switch the histogram binning mode
bool MGUIMainMelinator::OnSwitchHistogramBinningMode(unsigned int ID)
{  
  if (ID == MMelinator::c_HistogramBinningModeFixedNumberOfBins) {
    m_HistogramBinningModeValueLabel->SetText("  bins");
  } else if (ID == MMelinator::c_HistogramBinningModeFixedCountsPerBin) {
    m_HistogramBinningModeValueLabel->SetText("  cts/bin");
  } else if (ID == MMelinator::c_HistogramBinningModeBayesianBlocks) {
    m_HistogramBinningModeValueLabel->SetText("  as prior");
  } else {
    m_HistogramBinningModeValueLabel->SetText("ERROR");
  }

  //MapSubwindows();
  MapWindow();
  //Layout();
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Switch the histogram binning mode
bool MGUIMainMelinator::OnSwitchPeakParametrizationMode(unsigned int ID)
{    
  if (ID == MCalibrateEnergyFindLines::c_PeakParametrizationMethodBayesianBlockPeak) {
    m_PeakParametrizationOptions->RemoveAll(); // Deletes everyting too 
  } else if (ID == MCalibrateEnergyFindLines::c_PeakParametrizationMethodSmoothedPeak) {
    m_PeakParametrizationOptions->RemoveAll(); // Deletes everyting too 
  } else if (ID == MCalibrateEnergyFindLines::c_PeakParametrizationMethodFittedPeak) { 
    m_PeakParametrizationOptions->RemoveAll(); // Deletes everyting too 
    
    double FontScaler = MGUIDefaults::GetInstance()->GetFontScaler();
    unsigned int Height = 0;
    
    TGLayoutHints* TopLeftLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 0, 0, 0, 0);  
    TGLayoutHints* TopLeftTextLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft, 0, 0, FontScaler*3, 0);  
    TGLayoutHints* TopRightLayout = new TGLayoutHints(kLHintsTop | kLHintsExpandX | kLHintsRight, FontScaler*10, 0, 2, 3);

    TGHorizontalFrame* BackgroundModelFrame = new TGHorizontalFrame(m_PeakParametrizationOptions);
    m_PeakParametrizationOptions->AddFrame(BackgroundModelFrame, TopLeftLayout);

    TGLabel* BackgroundModelLabel = new TGLabel(BackgroundModelFrame, "Background: ");
    BackgroundModelLabel->ChangeOptions(kFixedWidth);
    BackgroundModelLabel->SetTextJustify(kTextLeft);
    BackgroundModelLabel->SetWidth(FontScaler*70);
    BackgroundModelFrame->AddFrame(BackgroundModelLabel, TopLeftTextLayout);
    
    m_PeakParametrizationMethodFittingBackgroundModel = new TGComboBox(BackgroundModelFrame, c_PeakParametrizationMethodFittingBackgroundModel);
    m_PeakParametrizationMethodFittingBackgroundModel->AddEntry("None", MCalibrationFit::c_BackgroundModelNone);
    m_PeakParametrizationMethodFittingBackgroundModel->AddEntry("Flat", MCalibrationFit::c_BackgroundModelFlat);
    m_PeakParametrizationMethodFittingBackgroundModel->AddEntry("Linear", MCalibrationFit::c_BackgroundModelLinear);
    m_PeakParametrizationMethodFittingBackgroundModel->Select(m_Settings->GetPeakParametrizationMethodFittingBackgroundModel());
    m_PeakParametrizationMethodFittingBackgroundModel->Associate(this);
    m_PeakParametrizationMethodFittingBackgroundModel->SetHeight(FontScaler*24);
    //m_PeakParametrizationMethodFittingBackgroundModel->SetWidth(FontScaler*100);
    Height = m_PeakParametrizationMethodFittingBackgroundModel->GetListBox()->GetNumberOfEntries()*m_PeakParametrizationMethodFittingBackgroundModel->GetListBox()->GetItemVsize();
    m_PeakParametrizationMethodFittingBackgroundModel->GetListBox()->SetHeight(Height);
    BackgroundModelFrame->AddFrame(m_PeakParametrizationMethodFittingBackgroundModel, TopRightLayout);

    TGHorizontalFrame* EnergyLossModelFrame = new TGHorizontalFrame(m_PeakParametrizationOptions);
    m_PeakParametrizationOptions->AddFrame(EnergyLossModelFrame, TopLeftLayout);

    TGLabel* EnergyLossModelLabel = new TGLabel(EnergyLossModelFrame, "Energy-loss: ");
    EnergyLossModelLabel->ChangeOptions(kFixedWidth);
    EnergyLossModelLabel->SetWidth(FontScaler*70);
    EnergyLossModelLabel->SetTextJustify(kTextLeft);
    EnergyLossModelFrame->AddFrame(EnergyLossModelLabel, TopLeftTextLayout);

    m_PeakParametrizationMethodFittingEnergyLossModel = new TGComboBox(EnergyLossModelFrame, c_PeakParametrizationMethodFittingEnergyLossModel);
    m_PeakParametrizationMethodFittingEnergyLossModel->AddEntry("None", MCalibrationFit::c_EnergyLossModelNone);
    m_PeakParametrizationMethodFittingEnergyLossModel->AddEntry("Delta (gauss convolved)", MCalibrationFit::c_EnergyLossModelGaussianConvolvedDeltaFunction);   m_PeakParametrizationMethodFittingEnergyLossModel->AddEntry("Delta (gaus+exp convolved)", MCalibrationFit::c_EnergyLossModelGaussianConvolvedDeltaFunctionWithExponentialDecay);
    m_PeakParametrizationMethodFittingEnergyLossModel->Select(m_Settings->GetPeakParametrizationMethodFittingEnergyLossModel());
    m_PeakParametrizationMethodFittingEnergyLossModel->Associate(this);
    m_PeakParametrizationMethodFittingEnergyLossModel->SetHeight(FontScaler*24);
    //m_PeakParametrizationMethodFittingEnergyLossModel->SetWidth(FontScaler*100);
    Height = m_PeakParametrizationMethodFittingEnergyLossModel->GetListBox()->GetNumberOfEntries()*m_PeakParametrizationMethodFittingEnergyLossModel->GetListBox()->GetItemVsize();
    m_PeakParametrizationMethodFittingEnergyLossModel->GetListBox()->SetHeight(Height);
    EnergyLossModelFrame->AddFrame(m_PeakParametrizationMethodFittingEnergyLossModel, TopRightLayout);


    TGHorizontalFrame* PeakShapeModelFrame = new TGHorizontalFrame(m_PeakParametrizationOptions);
    m_PeakParametrizationOptions->AddFrame(PeakShapeModelFrame, TopLeftLayout);

    TGLabel* PeakShapeModelLabel = new TGLabel(PeakShapeModelFrame, "Peak-shape: ");
    PeakShapeModelLabel->ChangeOptions(kFixedWidth);
    PeakShapeModelLabel->SetWidth(FontScaler*70);
    PeakShapeModelLabel->SetTextJustify(kTextLeft);
    PeakShapeModelFrame->AddFrame(PeakShapeModelLabel, TopLeftTextLayout);

    m_PeakParametrizationMethodFittingPeakShapeModel = new TGComboBox(PeakShapeModelFrame, c_PeakParametrizationMethodFittingPeakShapeModel);
    m_PeakParametrizationMethodFittingPeakShapeModel->AddEntry("Gaussian", MCalibrationFit::c_PeakShapeModelGaussian);
    m_PeakParametrizationMethodFittingPeakShapeModel->AddEntry("Gauss-Landau", MCalibrationFit::c_PeakShapeModelGaussLandau);
    m_PeakParametrizationMethodFittingPeakShapeModel->Select(m_Settings->GetPeakParametrizationMethodFittingPeakShapeModel());
    m_PeakParametrizationMethodFittingPeakShapeModel->Associate(this);
    m_PeakParametrizationMethodFittingPeakShapeModel->SetHeight(FontScaler*24);
    m_PeakParametrizationMethodFittingPeakShapeModel->SetWidth(FontScaler*100);
    //m_PeakParametrizationMethodFittingPeakShapeModel->GetListBox()->SetMinWidth(1000);
    Height = m_PeakParametrizationMethodFittingPeakShapeModel->GetListBox()->GetNumberOfEntries()*m_PeakParametrizationMethodFittingPeakShapeModel->GetListBox()->GetItemVsize();
    m_PeakParametrizationMethodFittingPeakShapeModel->GetListBox()->SetHeight(Height);
    PeakShapeModelFrame->AddFrame(m_PeakParametrizationMethodFittingPeakShapeModel, TopRightLayout);
  }

  MapSubwindows();
  MapWindow();  
  Layout();  
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Switch the histogram binning mode
bool MGUIMainMelinator::OnSwitchCalibrationModelDeterminationMode(unsigned int ID)
{    
  if (ID == MCalibrateEnergyDetermineModel::c_CalibrationModelStepWise) {
    m_CalibrationModelDeterminationOptions->RemoveAll(); // Deletes everyting too 
  } else if (ID == MCalibrateEnergyDetermineModel::c_CalibrationModelFit) {
    m_CalibrationModelDeterminationOptions->RemoveAll(); // Deletes everyting too 
    
    double FontScaler = MGUIDefaults::GetInstance()->GetFontScaler();
    
    TGLayoutHints* TopLeftLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 0, 0, 0, 0);  
    TGLayoutHints* TopLeftTextLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft, 0, 0, FontScaler*3, 0);  
    TGLayoutHints* TopRightLayout = new TGLayoutHints(kLHintsTop | kLHintsExpandX | kLHintsRight, FontScaler*10, 0, 2, 3);

    TGHorizontalFrame* EnergyFittingModelFrame = new TGHorizontalFrame(m_CalibrationModelDeterminationOptions);
    m_CalibrationModelDeterminationOptions->AddFrame(EnergyFittingModelFrame, TopLeftLayout);

    TGLabel* EnergyFittingModelLabel = new TGLabel(EnergyFittingModelFrame, "Energy: ");
    EnergyFittingModelLabel->ChangeOptions(kFixedWidth);
    EnergyFittingModelLabel->SetWidth(FontScaler*70);
    EnergyFittingModelLabel->SetTextJustify(kTextLeft);
    EnergyFittingModelFrame->AddFrame(EnergyFittingModelLabel, TopLeftTextLayout);
    
    m_CalibrationModelDeterminationMethodFittingEnergyModel = new TGComboBox(EnergyFittingModelFrame, c_CalibrationModelDeterminationMethodFittingEnergyModel);
    m_CalibrationModelDeterminationMethodFittingEnergyModel->AddEntry("Poly 1 Through Zero", MCalibrationModel::c_CalibrationModelPoly1Zero);
    m_CalibrationModelDeterminationMethodFittingEnergyModel->AddEntry("Poly 1", MCalibrationModel::c_CalibrationModelPoly1);
    m_CalibrationModelDeterminationMethodFittingEnergyModel->AddEntry("Poly 2", MCalibrationModel::c_CalibrationModelPoly2);
    m_CalibrationModelDeterminationMethodFittingEnergyModel->AddEntry("Poly 3", MCalibrationModel::c_CalibrationModelPoly3);
    m_CalibrationModelDeterminationMethodFittingEnergyModel->AddEntry("Poly 4", MCalibrationModel::c_CalibrationModelPoly4);
    m_CalibrationModelDeterminationMethodFittingEnergyModel->AddEntry("Poly 1 + Inv 1 Zero", MCalibrationModel::c_CalibrationModelPoly1Inv1Zero);
    m_CalibrationModelDeterminationMethodFittingEnergyModel->AddEntry("Poly 1 + Inv 1", MCalibrationModel::c_CalibrationModelPoly1Inv1);
    m_CalibrationModelDeterminationMethodFittingEnergyModel->AddEntry("Poly 2 + Inv 1 Zero", MCalibrationModel::c_CalibrationModelPoly2Inv1Zero);
    m_CalibrationModelDeterminationMethodFittingEnergyModel->AddEntry("Poly 2 + Inv 1", MCalibrationModel::c_CalibrationModelPoly2Inv1);
    m_CalibrationModelDeterminationMethodFittingEnergyModel->AddEntry("Poly 1 + Exp 1", MCalibrationModel::c_CalibrationModelPoly1Exp1);
    m_CalibrationModelDeterminationMethodFittingEnergyModel->AddEntry("Poly 1 + Exp 2", MCalibrationModel::c_CalibrationModelPoly1Exp2);
    m_CalibrationModelDeterminationMethodFittingEnergyModel->AddEntry("Poly 1 + Exp 3", MCalibrationModel::c_CalibrationModelPoly1Exp3);
    m_CalibrationModelDeterminationMethodFittingEnergyModel->AddEntry("Poly 1 + Log 1", MCalibrationModel::c_CalibrationModelPoly1Log1);
    m_CalibrationModelDeterminationMethodFittingEnergyModel->AddEntry("Poly 2 + Log 1", MCalibrationModel::c_CalibrationModelPoly2Log1);
    m_CalibrationModelDeterminationMethodFittingEnergyModel->Select(m_Settings->GetCalibrationModelDeterminationMethodFittingEnergyModel());
    m_CalibrationModelDeterminationMethodFittingEnergyModel->Associate(this);
    m_CalibrationModelDeterminationMethodFittingEnergyModel->SetHeight(FontScaler*24);
    //m_CalibrationModelDeterminationMethodFittingEnergyModel->SetWidth(FontScaler*130);
    unsigned int HeightEnergy = m_CalibrationModelDeterminationMethodFittingEnergyModel->GetListBox()->GetNumberOfEntries()*m_CalibrationModelDeterminationMethodFittingEnergyModel->GetListBox()->GetItemVsize();
    m_CalibrationModelDeterminationMethodFittingEnergyModel->GetListBox()->SetHeight(HeightEnergy);
    EnergyFittingModelFrame->AddFrame(m_CalibrationModelDeterminationMethodFittingEnergyModel, TopRightLayout);
    
    TGHorizontalFrame* FWHMFittingModelFrame = new TGHorizontalFrame(m_CalibrationModelDeterminationOptions);
    m_CalibrationModelDeterminationOptions->AddFrame(FWHMFittingModelFrame, TopLeftLayout);
    
    TGLabel* FWHMFittingModelLabel = new TGLabel(FWHMFittingModelFrame, "FWHM: ");
    FWHMFittingModelLabel->ChangeOptions(kFixedWidth);
    FWHMFittingModelLabel->SetWidth(FontScaler*70);
    FWHMFittingModelLabel->SetTextJustify(kTextLeft);
    FWHMFittingModelFrame->AddFrame(FWHMFittingModelLabel, TopLeftTextLayout);
    
    m_CalibrationModelDeterminationMethodFittingFWHMModel = new TGComboBox(FWHMFittingModelFrame, c_CalibrationModelDeterminationMethodFittingFWHMModel);
    m_CalibrationModelDeterminationMethodFittingFWHMModel->AddEntry("Poly 1 Through Zero", MCalibrationModel::c_CalibrationModelPoly1Zero);
    m_CalibrationModelDeterminationMethodFittingFWHMModel->AddEntry("Poly 1", MCalibrationModel::c_CalibrationModelPoly1);
    m_CalibrationModelDeterminationMethodFittingFWHMModel->AddEntry("Poly 2", MCalibrationModel::c_CalibrationModelPoly2);
    m_CalibrationModelDeterminationMethodFittingFWHMModel->AddEntry("Poly 3", MCalibrationModel::c_CalibrationModelPoly3);
    m_CalibrationModelDeterminationMethodFittingFWHMModel->AddEntry("Poly 4", MCalibrationModel::c_CalibrationModelPoly4);
    m_CalibrationModelDeterminationMethodFittingFWHMModel->AddEntry("Poly 1 + Inv 1", MCalibrationModel::c_CalibrationModelPoly1Inv1);
    m_CalibrationModelDeterminationMethodFittingFWHMModel->AddEntry("Poly 1 + Exp 1", MCalibrationModel::c_CalibrationModelPoly1Exp1);
    m_CalibrationModelDeterminationMethodFittingFWHMModel->AddEntry("Poly 1 + Exp 2", MCalibrationModel::c_CalibrationModelPoly1Exp2);
    m_CalibrationModelDeterminationMethodFittingFWHMModel->AddEntry("Poly 1 + Exp 3", MCalibrationModel::c_CalibrationModelPoly1Exp3);
    m_CalibrationModelDeterminationMethodFittingFWHMModel->Select(m_Settings->GetCalibrationModelDeterminationMethodFittingFWHMModel());
    m_CalibrationModelDeterminationMethodFittingFWHMModel->Associate(this);
    m_CalibrationModelDeterminationMethodFittingFWHMModel->SetHeight(FontScaler*24);
    //m_CalibrationModelDeterminationMethodFittingFWHMModel->SetWidth(FontScaler*130);
    unsigned int HeightFWHM = m_CalibrationModelDeterminationMethodFittingFWHMModel->GetListBox()->GetNumberOfEntries()*m_CalibrationModelDeterminationMethodFittingFWHMModel->GetListBox()->GetItemVsize();
    m_CalibrationModelDeterminationMethodFittingFWHMModel->GetListBox()->SetHeight(HeightFWHM);
    FWHMFittingModelFrame->AddFrame(m_CalibrationModelDeterminationMethodFittingFWHMModel, TopRightLayout);
    
  } else if (ID == MCalibrateEnergyDetermineModel::c_CalibrationModelBestFit) { 
    m_CalibrationModelDeterminationOptions->RemoveAll(); // Deletes everyting too 
  }

  MapSubwindows();
  MapWindow();  
  Layout();  
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Switch the histogram binning mode
bool MGUIMainMelinator::OnSwitchPeakHistogramBinningMode(unsigned int ID)
{  
  if (ID == MMelinator::c_HistogramBinningModeFixedNumberOfBins) {
    m_PeakHistogramBinningModeValueLabel->SetText("  bins");
  } else if (ID == MMelinator::c_HistogramBinningModeFixedCountsPerBin) {
    m_PeakHistogramBinningModeValueLabel->SetText("  cts/bin");
  } else if (ID == MMelinator::c_HistogramBinningModeBayesianBlocks) {
    m_PeakHistogramBinningModeValueLabel->SetText("  as prior");
  } else {
    m_PeakHistogramBinningModeValueLabel->SetText("  ERROR");
  }

  MapSubwindows();
  MapWindow();  
  Layout();  
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Toggle the log display of the histogram x-axis
bool MGUIMainMelinator::OnXAxis(bool IsLog)
{
  if (IsLog) {
    m_SpectrumCanvas->GetCanvas()->SetLogx(1);
  } else {
    m_SpectrumCanvas->GetCanvas()->SetLogx(0);
  }
  m_SpectrumCanvas->GetCanvas()->Update();

  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Toggle the log display of the histogram y-axis
bool MGUIMainMelinator::OnYAxis(bool IsLog)
{
  if (IsLog) {
    /*
    bool OneBinIsZero = false;
    // Get a list of primitives
    TIter Next(m_SpectrumCanvas->GetCanvas()->GetListOfPrimitives());
    TObject* Obj;
    while ((Obj = Next())) {
      TH1D* Hist = dynamic_cast<TH1D*>(Obj);
      if (Hist != 0) {
        for (int b = 1; b <= Hist->GetNbinsX(); ++b) {
          if (Hist->GetBinContent(b) <= 0) {
            cout<<Hist->GetTitle()<<": Bin "<<b<<" is zero!"<<endl; 
            OneBinIsZero = true;
            break;
          }
        }
      }
      if (OneBinIsZero == true) break;
    }

    
    if (OneBinIsZero == true) {
      merr<<"Cannot switch to log since one or more bins are zero or negative"<<show;
      m_HistogramLogY->SetState(kButtonUp);
    } else {    
      m_SpectrumCanvas->GetCanvas()->SetLogy(1);
    }
    */
    m_SpectrumCanvas->GetCanvas()->SetLogy(1);
  } else {
    m_SpectrumCanvas->GetCanvas()->SetLogy(0);
  }
  m_SpectrumCanvas->GetCanvas()->Update();

  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Choose the calibration files and isotopes
bool MGUIMainMelinator::OnChooseCalibrationFiles()
{
  new MGUILoadCalibration(gClient->GetRoot(), this, m_Settings);
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIMainMelinator::OnLoadLast()
{
  // Closes the application
   
  m_Melinator.Clear();
  m_Melinator.SetSelectedDetectorID(m_Settings->GetSelectedDetectorID());
  m_Melinator.SetSelectedDetectorSide(m_Settings->GetSelectedDetectorSide());
  m_Melinator.SetSelectedTemperatureWindow(m_Settings->GetMinimumTemperature(), m_Settings->GetMaximumTemperature());
  m_Melinator.SetPeakFindingPrior(m_Settings->GetPeakFindingPrior());
  m_Melinator.SetPeakFindingExcludeFirstNumberOfBins(m_Settings->GetPeakFindingExcludeFirstNumberOfBins());
  m_Melinator.SetPeakFindingMinimumPeakCounts(m_Settings->GetPeakFindingMinimumPeakCounts());

  
  MString FileName;
  MString IsotopeName;
  MIsotope Isotope;
  vector<MIsotope> Isotopes;

  vector<MString> AllFileNames;
  vector<vector<MIsotope> > AllIsotopes;
  vector<unsigned int> AllGroupIDs;
  
  MIsotopeStore Store;
  if (Store.Load("$(MEGALIB)/resource/libraries/Calibration.isotopes", true) == false) {
    merr<<"Unable to load calibration isotopes..."<<show;
    return false;
  }
  
  AllFileNames = m_Settings->GetCalibrationFiles();
  AllGroupIDs = m_Settings->GetCalibrationGroupIDs();

  vector<vector<MString>> AllIsotopeNames = m_Settings->GetCalibrationIsotopes();
  
  for (auto S: AllIsotopeNames) {
    AllIsotopes.push_back(vector<MIsotope>());
    for (auto IsotopeName: S) {
      if (IsotopeName != "None") {
        if (Store.Contains(IsotopeName) == false) {
          merr<<"Unable to find isotope: "<<IsotopeName<<show;
          return false;
        }
        AllIsotopes.back().push_back(Store.Get(IsotopeName));
      }
    }
  }

  
  m_Melinator.Load(AllFileNames, AllIsotopes, AllGroupIDs);
  
  //m_Melinator.RemoveCollectionsWithNoPositiveEntries();
    
  if (m_Melinator.GetNumberOfCollections() == 0) {
    mgui<<"There was no usable data on file!"<<show;
    //UpdateDisplay(g_UnsignedIntNotDefined, g_UnsignedIntNotDefined, true);
    return false;
  }
  
  CreateSelection();
  UpdateDisplay(0, 0, true);
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Actions when the save button has been pressed
bool MGUIMainMelinator::OnSave()
{  
  bool Return = m_Melinator.Save(m_Settings->GetSaveAsFileName());

  if (Return == false) {
    mgui<<"Unable to save file:"<<endl<<m_Settings->GetSaveAsFileName()<<error; 
  }
  
  return Return;
}


////////////////////////////////////////////////////////////////////////////////


//! Actions when the save as button has been pressed
bool MGUIMainMelinator::OnSaveAs()
{
  const char** Types = new const char*[4];
  Types[0] = "Energy calibration file";
  Types[1] = "*.ecal";
  Types[2] = 0;
  Types[3] = 0;
  

  TGFileInfo Info;
  Info.fFileTypes = (const char **) Types;
  Info.fIniDir = StrDup(gSystem->DirName(m_Settings->GetSaveAsFileName()));
  new TGFileDialog(gClient->GetRoot(), this, kFDSave, &Info);
  
  delete [] Types;
  
  // Get the filename ...
  if ((char *) Info.fFilename != 0) {
    m_Settings->SetSaveAsFileName(MString(Info.fFilename));
    return OnSave();
  } else {
    return true;
  }
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIMainMelinator::OnLoad()
{  
  return false;
}


////////////////////////////////////////////////////////////////////////////////


//! Action when one of the ROE buttons was pressed
bool MGUIMainMelinator::OnSwitchCollection(unsigned int Collection)
{
  if (Collection < m_Melinator.GetNumberOfCollections()) { 
    m_ActiveCollection = Collection;
    UpdateDisplay(m_ActiveCollection, m_ActiveLineFit, m_ActiveResultIsEnergy);
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Action when the next button has been pressed
bool MGUIMainMelinator::OnNext()
{
  if (m_ActiveCollection < m_Melinator.GetNumberOfCollections()-1) { 
    m_ActiveCollection++;
    UpdateDisplay(m_ActiveCollection, m_ActiveLineFit, m_ActiveResultIsEnergy);
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Action when the back button has been pressed
bool MGUIMainMelinator::OnBack()
{
  if (m_ActiveCollection > 0) { 
    m_ActiveCollection--;
    UpdateDisplay(m_ActiveCollection, m_ActiveLineFit, m_ActiveResultIsEnergy);
  }
 
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Actions when the next fit button has been pressed
bool MGUIMainMelinator::OnNextFit()
{
  if (m_ActiveLineFit < m_Melinator.GetNumberOfCalibrationSpectralPoints(m_ActiveCollection)-1) {
    m_ActiveLineFit++;
    UpdateDisplay(m_ActiveCollection, m_ActiveLineFit, m_ActiveResultIsEnergy);
  }
  return true; 
}


////////////////////////////////////////////////////////////////////////////////


//! Actions when the previous fit button has been pressed
bool MGUIMainMelinator::OnPreviousFit()
{
  if (m_ActiveLineFit > 0) { 
    m_ActiveLineFit--;
    UpdateDisplay(m_ActiveCollection, m_ActiveLineFit, m_ActiveResultIsEnergy);
  }
  return true; 
}


////////////////////////////////////////////////////////////////////////////////


//! Actions when the toggle-line button has been pressed
bool MGUIMainMelinator::OnToggleFit()
{
  // Special - if all are off, we turn on all
  unsigned int NGoodPoints = 0;
  for (unsigned int p = 0; p < m_Melinator.GetNumberOfCalibrationSpectralPoints(m_ActiveCollection); ++p) {
    if (m_Melinator.GetCalibrationSpectralPoint(m_ActiveCollection, p).IsGood() == true) {
      ++NGoodPoints;
    }
  }
  if (NGoodPoints == 0) {
    for (unsigned int p = 0; p < m_Melinator.GetNumberOfCalibrationSpectralPoints(m_ActiveCollection); ++p) {
      m_Melinator.GetCalibrationSpectralPoint(m_ActiveCollection, p).IsGood(true);
    }    
  } else {
    // Toggle line
    MCalibrationSpectralPoint& P = m_Melinator.GetCalibrationSpectralPoint(m_ActiveCollection, m_ActiveLineFit);
    P.IsGood(!P.IsGood());
  }
  
  // Redo fit!
  m_Melinator.ReCalibrateModel(m_ActiveCollection);
  
  UpdateDisplay(m_ActiveCollection, m_ActiveLineFit, m_ActiveResultIsEnergy);
  
  return true; 
}


////////////////////////////////////////////////////////////////////////////////


//! Update the histogram with the latest settings
bool MGUIMainMelinator::OnUpdateHistogram()
{
  if (m_Melinator.GetNumberOfCollections() > 0) {
    UpdateDisplay(m_ActiveCollection, m_ActiveLineFit, m_ActiveResultIsEnergy);
  }
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Update the peakhistogram with the latest settings
bool MGUIMainMelinator::OnUpdatePeakHistogram()
{
  if (m_Melinator.GetNumberOfCollections() > 0) {
    UpdateDisplay(m_ActiveCollection, m_ActiveLineFit, m_ActiveResultIsEnergy);
  }
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Update the peakhistogram with the latest settings
bool MGUIMainMelinator::OnToggleResults()
{
  if (m_Melinator.GetNumberOfCollections() > 0) {
    m_ActiveResultIsEnergy = !m_ActiveResultIsEnergy;
    UpdateCalibration(m_ActiveCollection, m_ActiveResultIsEnergy);
  }
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Update all graphs
void MGUIMainMelinator::UpdateMelinator()
{
  UpdateSettings();

  m_Melinator.SetPeakFindingPrior(m_Settings->GetPeakFindingPrior());
  m_Melinator.SetPeakFindingExcludeFirstNumberOfBins(m_Settings->GetPeakFindingExcludeFirstNumberOfBins());
  m_Melinator.SetPeakFindingMinimumPeakCounts(m_Settings->GetPeakFindingMinimumPeakCounts());

  if (m_Melinator.GetNumberOfCollections() > 0) {
    m_Melinator.SetPeakParametrizationMethod(m_PeakParametrizationMethod->GetSelected());
    if (m_PeakParametrizationMethod->GetSelected() == MCalibrateEnergyFindLines::c_PeakParametrizationMethodFittedPeak) {
      m_Melinator.SetPeakParametrizationMethodFittedPeakOptions(m_PeakParametrizationMethodFittingBackgroundModel->GetSelected(), m_PeakParametrizationMethodFittingEnergyLossModel->GetSelected(), m_PeakParametrizationMethodFittingPeakShapeModel->GetSelected());
    }

    if (m_CalibrationModelZeroCrossing->GetState() == kButtonDown) {
      m_Melinator.SetCalibrationModelEnergyAssignmentMethod(MCalibrateEnergyAssignEnergyModes::e_LinearZeroCrossing);
    } else {
      m_Melinator.SetCalibrationModelEnergyAssignmentMethod(MCalibrateEnergyAssignEnergyModes::e_Linear);
    }
    m_Melinator.SetCalibrationModelDeterminationMethod(m_CalibrationModelDeterminationMethod->GetSelected());
    if (m_CalibrationModelDeterminationMethod->GetSelected() == MCalibrateEnergyDetermineModel::c_CalibrationModelFit) {
      m_Melinator.SetCalibrationModelDeterminationMethodFittingEnergyOptions(m_CalibrationModelDeterminationMethodFittingEnergyModel->GetSelected());
      m_Melinator.SetCalibrationModelDeterminationMethodFittingFWHMOptions(m_CalibrationModelDeterminationMethodFittingFWHMModel->GetSelected());
    }
  }
}

////////////////////////////////////////////////////////////////////////////////


//! Update all graphs
bool MGUIMainMelinator::UpdateDisplay(unsigned int Collection, unsigned int Line, bool ActiveResultIsEnergy)
{
  UpdateCollection(Collection, Line);
  UpdateLineFit(Collection, Line);
  UpdateCalibration(Collection, ActiveResultIsEnergy);

  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Set the active collection and put it to the screen
void MGUIMainMelinator::UpdateCollection(unsigned int Collection, unsigned int Line)
{

  UpdateSettings();
  
  if (Collection < m_Melinator.GetNumberOfCollections()) {
    m_ActiveCollection = Collection;
    
    if (m_ActiveCollection == 0) {
      m_BackButton->SetEnabled(false);
    } else {
      m_BackButton->SetEnabled(true);
    }
    if (m_ActiveCollection >= m_Melinator.GetNumberOfCollections()-1) {
      m_ForwardButton->SetEnabled(false);
    } else {
      m_ForwardButton->SetEnabled(true);
    }
    
    m_MainHistogramLabel->SetText(MString("Spectrum for ") + m_Melinator.GetCollection(m_ActiveCollection).GetReadOutElement().ToString());
    Layout();
    
    m_Melinator.SetHistogramProperties(m_Settings->GetHistogramMin(), m_Settings->GetHistogramMax(), 
      m_Settings->GetHistogramBinningMode(), m_Settings->GetHistogramBinningModeValue());

    m_Melinator.DrawSpectrum(*(m_SpectrumCanvas->GetCanvas()), m_ActiveCollection, Line);

    OnXAxis(m_Settings->GetHistogramLogX());
    OnYAxis(m_Settings->GetHistogramLogY());

    //if (Line != m_ActiveLineFit) {
    //  UpdateLineFit(Collection, Line);
    //}
    
    //m_Melinator.DrawCalibration(*(m_ResultsCanvas->GetCanvas()), m_ActiveCollection);
    
  } else {
    m_BackButton->SetEnabled(false);
    m_ForwardButton->SetEnabled(false);
    m_SpectrumCanvas->GetCanvas()->Clear();
    m_SpectrumCanvas->GetCanvas()->Update();
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Update the line fit
void MGUIMainMelinator::UpdateLineFit(unsigned int Collection, unsigned int Line)
{  
  unsigned int NumberOfCalibrationSpectralPoints = 
    m_Melinator.GetNumberOfCalibrationSpectralPoints(Collection);
    
  if (NumberOfCalibrationSpectralPoints > 0) {
     m_ActiveLineFit = Line;
    while (m_ActiveLineFit >= NumberOfCalibrationSpectralPoints) {
      m_ActiveLineFit--;
    }
    
    if (m_ActiveLineFit == 0) {
      m_FitBackButton->SetEnabled(false);
    } else {
      m_FitBackButton->SetEnabled(true);
    }
    if (m_ActiveLineFit >= NumberOfCalibrationSpectralPoints-1) {
      m_FitForwardButton->SetEnabled(false);
    } else {
      m_FitForwardButton->SetEnabled(true);
    }
    
    if (NumberOfCalibrationSpectralPoints > 0) {
      m_FitToggleButton->SetEnabled(true);
    } else {
      m_FitToggleButton->SetEnabled(false);
    }
    
    m_Melinator.DrawLineFit(*(m_FitCanvas->GetCanvas()), Collection, m_ActiveLineFit, 
      m_Settings->GetPeakHistogramBinningMode(), m_Settings->GetPeakHistogramBinningModeValue());
    
    MCalibrationSpectralPoint P = m_Melinator.GetCalibrationSpectralPoint(Collection, m_ActiveLineFit);
    ostringstream Text;
    if (P.IsGood()) {
      MIsotope I = P.GetIsotope();
      Text<<I.GetName()<<" ("<<fixed<<setprecision(1)<<P.GetEnergy()<<" keV at "<<fixed<<setprecision(1)<<P.GetPeak()<<" rou's)";      
    } else {
      Text<<"Excluded line at "<<fixed<<setprecision(1)<<P.GetPeak()<<" rou's";
    }
    m_FitHistogramLabel->SetText(Text.str().c_str());
    Layout();
    
  } else {
    m_FitBackButton->SetEnabled(false);
    m_FitForwardButton->SetEnabled(false);
    m_FitToggleButton->SetEnabled(false);
    m_FitCanvas->GetCanvas()->Clear(); 
    m_FitCanvas->GetCanvas()->Update();
    m_FitHistogramLabel->SetText("Fit display");
    Layout();
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Update the calibration
void MGUIMainMelinator::UpdateCalibration(unsigned int Collection, bool DrawEnergyCalibration) 
{
  m_Melinator.DrawCalibration(*(m_ResultsCanvas->GetCanvas()), Collection, DrawEnergyCalibration);
  
  if (m_Melinator.HasEnergyCalibrationModel(Collection) == true && m_Melinator.HasFWHMCalibrationModel(Collection) == true) {

    if (DrawEnergyCalibration == true) {
      MCalibrationModel& M = m_Melinator.GetEnergyCalibrationModel(Collection);
      ostringstream Text;
      Text<<"Energy calibration model: "<<M.GetName();
      m_ResultsHistogramLabel->SetText(Text.str().c_str());
    } else {
      MCalibrationModel& M = m_Melinator.GetFWHMCalibrationModel(Collection);
      ostringstream Text;      
      Text<<"FWHM calibration model: "<<M.GetName();
      m_ResultsHistogramLabel->SetText(Text.str().c_str());
    }

    Layout();
  } else {
    if (DrawEnergyCalibration == true) {
      m_ResultsHistogramLabel->SetText("Energy calibration model: interpolation");
    } else {
      m_ResultsHistogramLabel->SetText("FWHM calibration model: interpolation");
    }
    Layout();
  }
  
  // Update the calibration also updates the coloring of the buttons:
  
  // Get
  vector<double> FitQualities;
  vector<bool> KnownOutliers;
  for (unsigned int i = 0; i < m_Melinator.GetNumberOfCollections(); ++i) {
    double FitQuality = m_Melinator.GetCalibrationQuality(i);
    
    if (FitQuality < 0 || FitQuality > 10) {
      KnownOutliers.push_back(true);
    } else {
      KnownOutliers.push_back(false);
    }
    
    FitQualities.push_back(FitQuality);
  }

  
  // RMS with outlier detection:
  MMath M;
  vector<bool> IsOutlier = KnownOutliers;
  if (FitQualities.size() >= 3) {
    IsOutlier = M.ModifiedThomsonTauTest(FitQualities, 0.01, KnownOutliers);
  }
  
  unsigned int GoodEntries = 0;
  double RMS = 0;
  for (unsigned int i = 0; i < FitQualities.size(); ++i) {
    if (IsOutlier[i] == false) {
      RMS += pow(FitQualities[i], 2);
      GoodEntries += 1;
    }
  }
  
  //cout<<"Good entries"<<GoodEntries<<endl;
  if (GoodEntries == 0) {
    // Nothing to do
    return;
  }
  
  RMS = sqrt(RMS/GoodEntries); // should be -1, but we don't want to handle the single case extra and it's close enough
    
  // Now set the colors:
  for (unsigned int i = 0; i < m_Melinator.GetNumberOfCollections(); ++i) {
    if (m_Melinator.GetCalibrationQuality(i) == -1) {
      m_MainSelectionCanvas->SetQuality(m_Melinator.GetCollection(i).GetReadOutElement(), -1);
    } else {
      double Quality = 0;
      if (RMS > 0) {
        Quality = m_Melinator.GetCalibrationQuality(i)/RMS;
      }
      m_MainSelectionCanvas->SetQuality(m_Melinator.GetCollection(i).GetReadOutElement(), Quality);
    }
    //cout<<m_Melinator.GetCalibrationQuality(i)<<endl;
    //cout<<"Collection: "<<i<<" ("<<m_Melinator.GetCollection(i).GetReadOutElement()<<") (Fitquality: "<<FitQualities[i]<<" RMS:"<<RMS<<", Outlier: "<<(IsOutlier[i] ? "true" : "false")<<")"<<endl;
  }
  
}


////////////////////////////////////////////////////////////////////////////////


//! Switch to the line near the given X value
void MGUIMainMelinator::SwitchToLine(double X)
{
  unsigned int Lines = m_Melinator.GetNumberOfCalibrationSpectralPoints(m_ActiveCollection);
  if (Lines == 0) return;
  
  unsigned int Closest = 0;
  double MinDistance = numeric_limits<double>::max(); 
  
  for (unsigned int p = 0; p < Lines; ++p) {
    MCalibrationSpectralPoint& P = m_Melinator.GetCalibrationSpectralPoint(m_ActiveCollection, p);
    if (m_ActiveResultIsEnergy == true) {
      if (fabs(P.GetPeak() - X) < MinDistance) {
        Closest = p;
        MinDistance = fabs(P.GetPeak() - X);
      }
    } else {
      if (fabs(P.GetEnergy() - X) < MinDistance) {
        Closest = p;
        MinDistance = fabs(P.GetEnergy() - X);
      }
    }
  }
  
  UpdateDisplay(m_ActiveCollection, Closest, m_ActiveResultIsEnergy);
}


////////////////////////////////////////////////////////////////////////////////


//! Fit the current histogram
bool MGUIMainMelinator::OnFit()
{
  UpdateMelinator();

  if (m_ActiveCollection < m_Melinator.GetNumberOfCollections()) {
    m_Melinator.Calibrate(m_ActiveCollection, false);
    OnUpdateHistogram();
  }
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Fit the current histogram
bool MGUIMainMelinator::OnFitWithDiagnostics()
{
  UpdateMelinator();

  if (m_ActiveCollection < m_Melinator.GetNumberOfCollections()) {
    m_Melinator.Calibrate(m_ActiveCollection, true);
    OnUpdateHistogram();
  }
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Fit the current histogram
bool MGUIMainMelinator::OnFitAll()
{
  UpdateMelinator();

  if (m_Melinator.GetNumberOfCollections() > 0) {
    m_Melinator.Calibrate(false);
    OnUpdateHistogram();
  }
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Load a configuration file from disk
bool MGUIMainMelinator::OnLoadConfiguration()
{
  //! Loading can leave the app in an inconsistent state, thus not do it now

  /*
  const char** Types = new const char*[4];
  Types[0] = "Configuration file";
  Types[1] = "*.cfg";
  Types[2] = 0;
  Types[3] = 0;


  TGFileInfo Info;
  Info.fFileTypes = (const char **) Types;
  new TGFileDialog(gClient->GetRoot(), this, kFDOpen, &Info);
  
  delete [] Types;
  
  // Get the filename ...
  if ((char *) Info.fFilename != 0) {
    m_Interface->LoadConfiguration(MString(Info.fFilename));
  }
  */

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIMainMelinator::OnSaveConfiguration()
{
  // Save a configuration file...

  const char** Types = new const char*[4];
  Types[0] = "Configuration file";
  Types[1] = "*.cfg";
  Types[2] = 0;
  Types[3] = 0;
  

  TGFileInfo Info;
  Info.fFileTypes = (const char **) Types;
  //Info.fIniDir = StrDup(gSystem->DirName(m_Settings->GetCurrentFile()));
  new TGFileDialog(gClient->GetRoot(), this, kFDSave, &Info);
  
  // Get the filename and store all the data in the settings...
  if ((char *) Info.fFilename != 0) {
    UpdateSettings();
    m_Interface->SaveConfiguration(MString(Info.fFilename));
  } 

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIMainMelinator::OnGeometry()
{
  // Show the geometry dialog
  // Returns the geometry file name
  
  MGUIGeometry* Geo = new MGUIGeometry(gClient->GetRoot(), this, m_Settings->GetGeometryFileName());
  gClient->WaitForUnmap(Geo);
  MString Name = Geo->GetGeometryFileName();
  delete Geo;
  for (unsigned int i = 0; i < 100; ++i) {
    gSystem->ProcessEvents();
  }

  m_Settings->SetGeometryFileName(Name);

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIMainMelinator::OnAbout()
{
  // Launch the about dialog

  MGUIAbout* About = new MGUIAbout(gClient->GetRoot(), this);
  About->SetProgramName("Melinator");
  About->SetIconPath(g_MEGAlibPath + "/resource/icons/melinator/Melinator.xpm");
  About->SetLeadProgrammer("Andreas Zoglauer");
  About->SetCopyright("(C) by Andreas Zoglauer\nAll rights reserved");
  //About->SetReference("Implementation details of the imaging approach", 
  //                    "A. Zoglauer et al., \"Design, implementation, and optimization of MEGAlib's image reconstruction tool Mimrec \", NIM A 652, 2011");
  //About->SetReference("A detailed description of list-mode likelihood image reconstruction - in German", 
  //                    "A. Zoglauer, \"Methods of image reconstruction for the MEGA Compton telescope\", Diploma thesis, TU Munich, 2000");
  //About->SetReference("Chapter 5: List-mode image reconstruction applied to the MEGA telecope", 
  //                    "A. Zoglauer, \"First Light for the Next Generation of Compton and Pair Telescopes\", Doctoral thesis, TU Munich, 2005");
  About->Create();
  
  return true;
}


// MGUIMainMelinator: the end...
////////////////////////////////////////////////////////////////////////////////
