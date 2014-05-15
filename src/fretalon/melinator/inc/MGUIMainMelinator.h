/*
* MGUIMelinatorMain.h
*
* Copyright (C) by Andreas Zoglauer.
* All rights reserved.
*
* Please see the source-file for the copyright-notice.
*
*/


#ifndef __MGUIMelinatorMain__
#define __MGUIMelinatorMain__


////////////////////////////////////////////////////////////////////////////////


// Standard libs
#include <vector>
using namespace std;

// ROOT libs
#include <TGMenu.h>
#include <TGButton.h>
#include <TGFrame.h>
#include <TGFileDialog.h>
#include <TGIcon.h>
#include <TGComboBox.h>
#include <TGPicture.h>
#include <TRootEmbeddedCanvas.h>
#include <TGNumberEntry.h>

// MEGAlib libs
#include "MGlobal.h"
#include "MGUIDialog.h"
#include "MInterfaceMelinator.h"
#include "MSettingsMelinator.h"
#include "MMelinator.h"
#include "MGUIEReadOutElement.h"
#include "MGUIEReadOutElementView.h"

// Forward declarations:
class MGUIEFileSelector;

////////////////////////////////////////////////////////////////////////////////


class MGUIMelinatorMain : public TGMainFrame
{
  // Public members:
public:
  //! Default constructor
  MGUIMelinatorMain(MInterfaceMelinator* Interface, MSettingsMelinator* Settings);
  //! Default destructor
  virtual ~MGUIMelinatorMain();

  //! Create the GUI
  virtual void Create();
  //! Process all button, etc. messages
  virtual bool ProcessMessage(long Message, long Parameter1, long Parameter2);
  //! Called when the "x" is pressed
  virtual void CloseWindow();

  //! Handle some keys
  bool HandleKey(Event_t* Event);

  //! Actions when the load last button has been pressed
  virtual bool OnLoadLast();
  //! Parmetrize all data
  virtual bool OnFitAll();

  // protected members:
protected:
  //! Set the active collection
  void UpdateCollection(unsigned int Collection, unsigned int Line);
  //! Set the active collection
  void UpdateLineFit(unsigned int Collection, unsigned int Line);
  //! Update the calibration
  void UpdateCalibration(unsigned int Collection);

  //! Update all graphs
  bool UpdateDisplay(unsigned int Collection, unsigned int Line);

  //! Update the saved GUI data
  bool Update();
  
  //! Create the selection GUI element    
  void CreateSelection();

  
  //! Actions when the exit button has been pressed
  virtual bool OnExit();
  //! Actions when the save key has been pressed
  virtual bool OnSaveConfiguration();
  //! Actions when the load key has been pressed
  virtual bool OnLoadConfiguration();
  //! Actions when the load key has been pressed
  virtual bool OnGeometry();
  //! Actions when the about button has been pressed
  virtual bool OnAbout();
  //! Actions when the load button has been pressed
  virtual bool OnLoad();
  //! Actions when the save button has been pressed
  virtual bool OnSave();
  //! Actions when the save as button has been pressed
  virtual bool OnSaveAs();
  //! Actions when the next button has been pressed
  virtual bool OnNext();
  //! Actions when the back button has been pressed
  virtual bool OnBack();
  //! Actions when the next fit button has been pressed
  virtual bool OnNextFit();
  //! Actions when the previous fit button has been pressed
  virtual bool OnPreviousFit();
  //! Actions when the load calibration button has been pressed
  virtual bool OnChooseCalibrationFiles();
  //! Toggle the X-log display of the histogram x-axis
  virtual bool OnXAxis(bool IsLog);
  //! Toggle the X-log display of the histogram y-axis
  virtual bool OnYAxis(bool IsLog);
  //! Update the histogram with the latest settings
  virtual bool OnUpdateHistogram();
  //! Update the peak histogram with the latest settings
  virtual bool OnUpdatePeakHistogram();
  //! Do a fit of the current histogram
  virtual bool OnFit();
  //! Do a fit of the current histogram and display diagnostics info
  virtual bool OnFitWithDiagnostics();
  //! Switch the histogram binning mode
  virtual bool OnSwitchHistogramBinningMode(unsigned int ID);
  //! Switch the histogram binning mode for the peak histrogram
  virtual bool OnSwitchPeakHistogramBinningMode(unsigned int ID);
  //! Action when one of the ROE buttons was pressed
  virtual bool OnSwitchCollection(unsigned int Collection);

  // private members:
private:
  //! Reference to all interface functions
  MInterfaceMelinator* m_Interface;
  //! Reference to all GUI data
  MSettingsMelinator* m_Settings;

  //! The melinator itself
  MMelinator m_Melinator;
  //! The active read-out data collection
  unsigned int m_ActiveCollection;
  //! The active line fit for the given read-out collection
  unsigned int m_ActiveLineFit;
  
  //! The label of the main histogram
  TGLabel* m_MainHistogramLabel;
  //! The forward button
  TGTextButton* m_ForwardButton;  
  //! The back button
  TGTextButton* m_BackButton;
  //! The view of the spectrum
  TRootEmbeddedCanvas* m_SpectrumCanvas;
  
  //! The label of the fit view
  TGLabel* m_FitHistogramLabel;
  //! The forward button of the fit view
  TGTextButton* m_FitForwardButton;  
  //! The back button of the fit view
  TGTextButton* m_FitBackButton;
  //! The view of the fit
  TRootEmbeddedCanvas* m_FitCanvas;
  
  //! The view of the fit
  TRootEmbeddedCanvas* m_ResultsCanvas;
  
  
  //! Entry representing the minimum of the histogram
  TGNumberEntry* m_HistogramRangeMin;
  //! Entry representing the maximum of the histogram
  TGNumberEntry* m_HistogramRangeMax;
  //! Choose the binning mode (fixed bins, fixed number of counts, Bayesian block)
  TGComboBox* m_HistogramBinningMode;
  //! Entry representing the number of bins, or the number of counts per bin, or the prior in the histogram
  TGNumberEntry* m_HistogramBinningModeValue;
  //! The label describing the binning mode value
  TGLabel* m_HistogramBinningModeValueLabel;
  //! Button for updating the histogram
  TGTextButton* m_UpdateHistogramButton;
  //! Check button indicating that the X-axis should be logarithmic
  TGCheckButton* m_HistogramLogX;
  //! Check button indicating that the Y-axis should be logarithmic
  TGCheckButton* m_HistogramLogY;
  
  
  //! Choose the binning mode (fixed bins, fixed number of counts, Bayesian block)
  TGComboBox* m_PeakHistogramBinningMode;
  //! Entry representing the number of bins, or the number of counts per bin, or the prior in the histogram
  TGNumberEntry* m_PeakHistogramBinningModeValue;
  //! The label describing the binning mode value
  TGLabel* m_PeakHistogramBinningModeValueLabel;
  //! Button for updating the histogram
  TGTextButton* m_PeakHistogramUpdateButton;
  
  
  //! Choose the peak parametrization method
  TGComboBox* m_PeakParametrizationMethod;
  //! Button for fitting all histograms
  TGTextButton* m_FitAllButton;
 
  //! The canvas displaying the selection including scroll bars
  MGUIEReadOutElementView* m_MainSelectionCanvas;
  //! The window where the selection will be drawn
  TGVerticalFrame* m_MainSelectionView;
  //! All the read-out element GUI's
  vector<MGUIEReadOutElement*> m_ROEButtons;
  
  
  // IDs:
  static const int c_Start                      =   1;
  static const int c_Exit                       =   2;
  static const int c_LoadConfig                 =   3;
  static const int c_SaveConfig                 =   4;
  static const int c_Geometry                   =   5;
  static const int c_About                      =   6;
  static const int c_ChooseCalibrationFiles     =   7;
  static const int c_LoadLast                   =   8;
  static const int c_UpdateHistogram            =   9;
  static const int c_UpdatePeakHistogram        =  10;
  static const int c_FitAll                     =  11;
  static const int c_HistogramBinningMode       =  12;
  static const int c_PeakHistogramBinningMode   =  13;
  static const int c_HistogramLogX              =  14;
  static const int c_HistogramLogY              =  15;
  static const int c_NextFit                    =  16;
  static const int c_PreviousFit                =  17;
  static const int c_PeakParametrizationMethod  =  18;
  static const int c_Save                       =  19;
  static const int c_SaveAs                     =  20;
  static const int c_Remove                     = 400;
  static const int c_Options                    = 500;
  static const int c_Change                     = 600;
  static const int c_Back                       = 601;
  static const int c_Next                       = 602;
  static const int c_Fit                        = 603;
  static const int c_FitWithDiagnostics         = 604;
  static const int c_BinsByCounts               = 700;
  static const int c_BinsByBins                 = 701;
  static const int c_ROEButtons                 = 1000;
  // Nothing beyond that allowed

#ifdef ___CINT___
public:
  ClassDef(MGUIMelinatorMain, 0) // main window of the Melinator GUI
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
