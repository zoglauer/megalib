/*
 * PulseForms.C
 *
 * Copyright (C) by Andreas Zoglauer, Robert Andritschke, Florian Schopper
 * All rights reserved.
 *
 * This code implementation is the intellectual property 
 * of Robert Andritschke, Florian Schopper, Andreas Zoglauer.
 *
 * By copying, distributing or modifying the Program (or any work based on the 
 * Program) you indicate your acceptance of this statement, and all its terms.
 *
 */

/******************************************************************************
 *
 * This program does something - no question...

launch PulseForms -n 2 -t 754 -b 200 -i 1101 --minimum-peak 25 -s apf -s rt -s thressig --auto-start -o /data/mess/Waveforms/WaveformsD2 -s cpf -n 2 -s pf --trigger-delay 500 --detector daphne --baseline-sigma 12

launch PulseForms -n 2 -t 754 -b 200 -i 1101 --minimum-peak 25 -s apf -s rt -s thressig --auto-start -f /data/mess/Waveforms/WaveformsD1/WF_odysseus\:Chip#5_odysseus\:Chip#2_poseidon\:Chip#5_poseidon\:Chip#2_bachus\:Chip#5_bachus\:Chip#2_101.20030111_061208.rawa --D1 -s cpf -n 1 -s pf --trigger-delay 500

launch PulseForms -n 2 -t 754 -b 200 -i 1101 --minimum-peak 25 -s apf -s rt -s thressig --auto-start -f /data/mess/Waveforms/WaveformsD2/WF_thetis\:Chip#1_antigone\:Chip#1_101.20021220_022027.rawa -s cpf -n 1 -s pf --trigger-delay 500

launch PulseForms -n 2 -t 754 -b 200 -i 1101 --minimum-peak 20 -d 3500 -p Fits.20030114_180401.csv -s apf -s rt -s thressig --auto-start -s crealt -s crealtd -o /data/mess/Waveforms/WaveformsD2 --detector thetis --store-results

launch PulseForms -n 2 -t 754 -b 200 -i 1101 --minimum-peak 20 -d 3500 -p Fits.20030114_180401.csv -s apf -s rt -s thressig --auto-start -s crealt -s crealtd -o /data/mess/Waveforms/WaveformsD2Muon/ --myons --store-results -s capf -s ctr

launch PulseForms -n 2 -t 754 -b 200 -i 1101 --trigger-delay 500 --show-fits -p Fits.D2.20030116_161733.csv


*
*/


/******************************************************************************/

// Standard libraries:
#include <vector>
#include <string>
#include <fstream>
#include <limits>
#include <sstream>
using namespace std;

// Root libraries:
#include <TSystem.h>
#include <TCanvas.h>
#include <TH1.h>
#include <TH2.h>
#include <TApplication.h>
#include <TGFrame.h>
#include <TGButton.h>
#include <TGClient.h>
#include <TF1.h>
#include <TProfile.h>
#include <TStyle.h>
#include <TColor.h>

// Mega libraries:
#include "MStreams.h"
#include "MTokenizer.h"
#include "MFileEventsRawa.h"
#include "MTime.h"

/******************************************************************************/


/******************************************************************************/

class MPulseForms : public TGMainFrame
{
  // Public Interface:
public:
  MPulseForms();
  virtual ~MPulseForms();

  Bool_t ParseCommandLine(int argc, char *argv[]);
  void Create();

  // protected methods:
protected:
  Bool_t ProcessMessage(Long_t Message, Long_t Parameter1, Long_t Parameter2);
  void CloseWindow();

	Bool_t OnNext();
	Bool_t OnLoop();
	Bool_t OnStop();
	Bool_t OnStore();
	Bool_t OnTest();
	Bool_t OnClose();

  // private methods:
private:
  Bool_t AnalyzeEvent();

  double EvaluateCalibration(Int_t PF, double Abszisse);
  Bool_t TestCalibration(Int_t PF);
  

  Bool_t StoreFitFunctions();
  Bool_t LoadFitFunctions(bool IgnoreNPulseForms = false);
	void ShowFits();

  Bool_t CheckTriggerMask(Int_t PF);

  Bool_t FillHistograms(Int_t PF);
	Bool_t CorrectPulseForm(Int_t PF);
	Bool_t FindMaxima(Int_t PF);
  Bool_t FindBaselines(Int_t PF);
  Bool_t FindTriggerThreshold(Int_t PF);
  Bool_t FindRealTriggerThreshold(Int_t PF);

	Bool_t CreateNormalizedPulseFormDensityHistogram(Int_t PF);

  Double_t FindMaximumX(TF1* Function, double Min, double Max);
  Double_t FindZeroCrossing(TF1* Function, double Max, double Zero);

  Bool_t PrepareEventAnalysis();
  void FinalCalculations();

  // protected members:
protected:


  // private members:
private:
  // Gui
  TGCompositeFrame* m_MainFrame;
  TGLayoutHints* m_MainFrameLayout;

  TGTextButton* m_NextButton;
  TGTextButton* m_CloseButton;
  TGTextButton* m_TestButton;
  TGTextButton* m_StoreButton;
  TGTextButton* m_LoopButton;
  TGTextButton* m_StopButton;
  TGLayoutHints* m_ButtonLayout;

  bool m_StopLoop;
  bool m_StopAtNewFile;
  bool m_AutoStart;

  bool m_UseMyons;
  bool m_IsD1;

	bool m_StoreResults;

	bool m_ShowFits;
	bool m_ShowTriggerDistributionOnly;

  bool m_IsTestCalibrationMode;
	bool m_IsChipLevelInitialized;
	bool m_IsMEGALevelInitialized;

	// Some global constants
	double m_Threshold;
	double m_MaxBaselineSigma;

  // Data:
  unsigned int m_CurrentFile;
	vector<MString> m_FileNames;
  vector<MFileEventsRawa*> m_Files;
  vector<int> m_ChannelPerFile;
	vector<vector<int> > m_ChipIdsPerFile;
	vector<vector<MString> > m_ChipsPerFile;
  vector<vector<MString> > m_TriggeredChips;

	MString m_Detector;

  int m_PulseStartOffset;
  int m_TriggerDelay;
  int m_TriggerHoldDelay;
  int m_PulseBinWidth;

  int m_StartEvent;
	int m_StartRun;
	int m_StopRun;


  MString m_CalibFileName;
  vector<vector<double> > m_CalibPolynom;
  vector<vector<double> > m_InvCalibPolynom;
  vector<vector<double> > m_MaximumRisePolynom;
  vector<double> m_AveragePolynom;
  vector<MString> m_CalibPolynomName;
  vector<MString> m_AllChipNames;

  vector<MString> m_ChipNames;
  unsigned int m_NPulseForms;
  vector<double> m_Baseline;
  vector<int> m_NBaselines;
  vector<bool> m_BaselineFound;
  vector<bool> m_BaselinesFinished;

  vector<double> m_AverageSigma;
	vector<int> m_NSigmas;

	vector<double> m_Maximum;

  //
  TF1* m_BaselineFit;
  TF1* m_MaximumFit;
  TF1* m_AverageFit;
  TF1* m_RiseFit;
  TF1* m_ComplicatedFit;
  TF1* m_SignalRiseFit;

  // Display
  bool m_CanvasesPrepared;

  bool m_ShowPulseForm;
  vector<TCanvas*> m_PulseFormCanvas;
  vector<TH1D*> m_PulseFormHistogram;
  vector<TH1D*> m_BaselineFunction;
  vector<TH1D*> m_TriggerFunction;
  vector<TH1D*> m_HoldFunction;

  bool m_ShowCorrectedPulseForm;
  vector<TCanvas*> m_CorrectedPulseFormCanvas;
  vector<TH1D*> m_CorrectedPulseFormHistogram;

  bool m_ShowNormalizedPulseForm;
  vector<TCanvas*> m_NormalizedPulseFormCanvas;
  vector<TH1D*> m_NormalizedPulseFormHistogram;

  bool m_ShowNormalizedPulseFormDensity;
  vector<TCanvas*> m_NormalizedPulseFormDensityCanvas;
  vector<TH2D*> m_NormalizedPulseFormDensityHistogram;

  bool m_ShowAveragedPulseForm;
  vector<TCanvas*> m_AveragedPulseFormCanvas;
  vector<TH1D*> m_AveragedPulseFormHistogram;
	vector<int> m_NAverages;

  bool m_ShowBaselineDistribution;
  vector<TCanvas*> m_BaselineDistributionCanvas;
  vector<TH1D*> m_BaselineDistributionHistogram;

  bool m_ShowBaselineDensity;
  vector<TCanvas*> m_BaselineDensityCanvas;
  vector<TH2D*> m_BaselineDensityHistogram;

  bool m_ShowAverageBaseline;
  vector<TCanvas*> m_AverageBaselineCanvas;
  vector<TH1D*> m_AverageBaselineHistogram;
	vector<int> m_NAverageBaselines;

  bool m_ShowZeroCrossing;
  vector<TCanvas*> m_ZeroCrossingCanvas;
  vector<TH1D*> m_ZeroCrossingHistogram;

  bool m_ShowTriggerThreshold;
  vector<TCanvas*> m_TriggerThresholdCanvas;
  vector<TH2D*> m_TriggerThresholdHistogram;

  bool m_ShowRiseTimePerSignal;
  vector<TCanvas*> m_RiseTimePerSignalCanvas;
  vector<TH2D*> m_RiseTimePerSignalHistogram;

  bool m_ShowTriggerTimePerSignal;
  vector<TCanvas*> m_TriggerTimePerSignalCanvas;
  vector<TH2D*> m_TriggerTimePerSignalHistogram;

  bool m_ShowThresholdOverSignal;
  vector<TCanvas*> m_ThresholdOverSignalCanvas;
  vector<TH2D*> m_ThresholdOverSignalHistogram;

  bool m_ShowMaximumCorrection;
  vector<TCanvas*> m_MaximumCorrectionCanvas;
  vector<TH2D*> m_MaximumCorrectionHistogram;
  vector<TH1D*> m_MaximumCorrectionBisectorFunction;

  bool m_ShowMaximumWrongness;
  vector<TCanvas*> m_MaximumWrongnessCanvas;
  vector<TH2D*> m_MaximumWrongnessHistogram;
  vector<TH1D*> m_MaximumWrongnessBisectorFunction;

  bool m_ShowAverageChipPulseFormDensity;
  vector<TCanvas*> m_AverageChipPulseFormDensityCanvas;
  vector<TH2D*> m_AverageChipPulseFormDensityHistogram;

  bool m_ShowAverageChipPulseForm;
  vector<TCanvas*> m_AverageChipPulseFormCanvas;
  vector<TH1D*> m_AverageChipPulseFormHistogram;
	vector<int> m_AverageChipPulseFormNAverages;

  bool m_ShowChipRiseTimePerSignal;
  vector<TCanvas*> m_ChipRiseTimePerSignalCanvas;
  vector<TH2D*> m_ChipRiseTimePerSignalHistogram;
	vector<TProfile*> m_ChipRiseTimePerSignalProfile;

  bool m_ShowChipSignalOverRise;
  vector<TCanvas*> m_ChipSignalOverRiseCanvas;
  vector<TH2D*> m_ChipSignalOverRiseHistogram;
	vector<TProfile*> m_ChipSignalOverRiseProfile;

  bool m_ShowChipTriggerTimePerSignal;
  vector<TCanvas*> m_ChipTriggerTimePerSignalCanvas;
  vector<TH2D*> m_ChipTriggerTimePerSignalHistogram;

  bool m_ShowChipTriggerTime;
  vector<TCanvas*> m_ChipTriggerTimeCanvas;
  vector<TH1D*> m_ChipTriggerTimeHistogram;

  bool m_ShowChipRealThreshold;
  vector<TCanvas*> m_ChipRealThresholdCanvas;
  vector<TH1D*> m_ChipRealThresholdHistogram;
	vector<double> m_AvgRealThreshold;

  bool m_ShowChipRealThresholdPlusBase;
  vector<TCanvas*> m_ChipRealThresholdPlusBaseCanvas;
  vector<TH1D*> m_ChipRealThresholdPlusBaseHistogram;

  bool m_ShowChipRealThresholdDelay;
  vector<TCanvas*> m_ChipRealThresholdDelayCanvas;
  vector<TH1D*> m_ChipRealThresholdDelayHistogram;
	vector<double> m_AvgRealThresholdDelay;

  bool m_ShowMEGARiseTimePerSignal;
  TCanvas* m_MEGARiseTimePerSignalCanvas;
  TH2D* m_MEGARiseTimePerSignalHistogram;

  bool m_ShowMEGATriggerTimePerSignal;
  TCanvas* m_MEGATriggerTimePerSignalCanvas;
  TH2D* m_MEGATriggerTimePerSignalHistogram;

  bool m_ShowMEGATriggerTime;
  TCanvas* m_MEGATriggerTimeCanvas;
  TH1D* m_MEGATriggerTimeHistogram;

  enum ButtonIDs { e_Next = 100, e_Loop, e_Stop, e_Store, e_Close, e_Test };
};



/*******************************************************************************
 * Fit function for the completion of the pulse shape for D1
 */
double ShapeCompletionFit(Double_t *x, Double_t *par)
{
	// We have three real parameter and 10 fixed:
	// 0..9: polynom
	// 10: translation
	// 11: width
	// 12: height

	double a =  (x[0]-par[10])*par[11];
	if (a < 0 || a > 8000)  return 0.0;

	double Result = 0.0;

	double Potenz = 0.0;
	for (int p = 0; p < 10; ++p) {
		if (Potenz == 0.0) {
			Potenz = 1.0;
		} else {
			Potenz *= a;
		}
		Result += par[p]*Potenz;
	}
	Result *= par[12];

	return Result;
}



/*******************************************************************************
 * Fit function for the determination of the average polynom
 */
double AvgPolynomFit(Double_t *x, Double_t *par)
{
	// We have one real parameter and 10 fixed:
	// 0: width of polynom
	// 1..10: polynom

	double Result = 0.0;

	double Potenz = 0.0;
	for (int p = 1; p <= 10; ++p) {
		if (Potenz == 0.0) {
			Potenz = 1.0;
		} else {
			Potenz *= x[0]*par[0];
		}
		Result += par[p]*Potenz;
	}

	return Result;
}


/*******************************************************************************
 * Fit function for the threshold over signal histogram
 */
double ComplicatedFit(Double_t *x, Double_t *par)
{
	// We have two real parameters (Threshold and Delay) and 20 (!) fixed
	// 2..11 are the polynom
	// 12..21 are the inverted polynom

	if (x[0] == 0.0) return 0.0;
	double Result = par[0]/x[0];
	if (Result >= 1.0) return 0.0;

	double Potenz = 0.0;
	double NewResult = 0.0;
	for (int p = 12; p < 22; ++p) {
		if (Potenz == 0.0) {
			Potenz = 1.0;
		} else {
			Potenz *= Result;
		}
		NewResult += par[p]*Potenz;
	}
	Result = NewResult + par[1];

	Potenz = 0.0;
	NewResult = 0.0;
	for (int p = 2; p < 12; ++p) {
		if (Potenz == 0.0) {
			Potenz = 1.0;
		} else {
			Potenz *= Result;
		}
		NewResult += par[p]*Potenz;
	}
	Result = NewResult*x[0];

	return Result;
}

/*******************************************************************************
 * Entry point to the programm:
 */
int main(int argc, char *argv[])
{
  cout<<endl;
  cout<<"PulseForms (C) by Andreas Zoglauer"<<endl;
  cout<<endl;

  TApplication PulseFormsApp("PulseForms", 0, 0);
  MPulseForms* PF = new MPulseForms();
  if (PF->ParseCommandLine(argc, argv) == true) {
    PF->Create();
		PulseFormsApp.Run();
  }

	return 0;
}


/******************************************************************************
 *
 */
MPulseForms::MPulseForms()
  : TGMainFrame(gClient->GetRoot(), 400, 35)
{
  // Construct an instance of MPulseForms and bring it to the screen

  m_NPulseForms = 0;
  m_PulseStartOffset = -1;
  m_TriggerHoldDelay = -1;
	m_TriggerDelay = -1;
  m_PulseBinWidth = -1;
  m_StartEvent = 1;
	m_StartRun = 0;
	m_StopRun = 10000000;
	
  m_UseMyons = false;
	m_IsD1 = false;

	m_StoreResults = false;
	m_ShowFits = false;
	m_ShowTriggerDistributionOnly = false;
	
  m_CurrentFile = 0;
	
	m_Threshold = 10;
	m_MaxBaselineSigma = -1;
	m_IsTestCalibrationMode = false;

  m_AutoStart = false;
  m_StopAtNewFile = false;
 
	m_CanvasesPrepared = false;
	m_IsChipLevelInitialized = false;
	m_IsMEGALevelInitialized = false;

  m_ShowPulseForm = false;
  m_ShowCorrectedPulseForm = false;
  m_ShowNormalizedPulseForm = false;
  m_ShowNormalizedPulseFormDensity = false;
  m_ShowBaselineDistribution = false;
  m_ShowBaselineDensity = false;
  m_ShowZeroCrossing = false;
  m_ShowAveragedPulseForm = false;
  m_ShowTriggerThreshold = false;
  m_ShowRiseTimePerSignal = false;
  m_ShowTriggerTimePerSignal = false;
  m_ShowThresholdOverSignal = false;
  m_ShowMaximumCorrection = false;
  m_ShowMaximumWrongness = false;

	m_ShowAverageChipPulseFormDensity = false;
	m_ShowAverageChipPulseForm = false;
  m_ShowChipRiseTimePerSignal = false;
  m_ShowChipSignalOverRise = false;
  m_ShowChipTriggerTimePerSignal = false;
  m_ShowChipTriggerTime = false;
  m_ShowChipRealThreshold = false;
  m_ShowChipRealThresholdPlusBase = false;
  m_ShowChipRealThresholdDelay = false;

  m_ShowMEGARiseTimePerSignal = false;
  m_ShowMEGATriggerTimePerSignal = false;
  m_ShowMEGATriggerTime = false;

  m_BaselineFit = 0;
  m_MaximumFit = 0;
  m_AverageFit = 0;
  m_RiseFit = 0;

  // Create palette:
  Int_t Number = 3;
  Double_t Stops[3] = { 0.00, 0.50, 1.00 };
  Double_t Red[3] = { 1.00, 0.50, 0.00 };
  Double_t Green[3] = { 1.00, 0.50, 0.00 };
  Double_t Blue[3] = { 1.00, 0.50, 0.00 };
  TColor::CreateGradientColorTable(Number, Stops, Red, Green, Blue, 100);

	// This now is a REALLY **DIRTY** hack...
	m_AveragePolynom.push_back(-4.00620e-02);
	m_AveragePolynom.push_back(4.17145e-04);
	m_AveragePolynom.push_back(-2.38072e-07);
	m_AveragePolynom.push_back(1.38210e-10);
	m_AveragePolynom.push_back(-4.32926e-14);
	m_AveragePolynom.push_back(7.65560e-18);
	m_AveragePolynom.push_back(-8.07560e-22);
	m_AveragePolynom.push_back(5.06304e-26);
	m_AveragePolynom.push_back(-1.74564e-30);
	m_AveragePolynom.push_back(2.55389e-35);

//    1  p0          -4.00620e-02   1.14106e-02   4.98065e-06  -7.64862e-05
//    2  p1           4.17145e-04   1.94169e-05   2.07967e-09  -4.72991e+00
//    3  p2          -2.38072e-07   3.91282e-09   7.14216e-13  -3.68098e+04
//    4  p3           1.38210e-10   4.05509e-13   4.14631e-16  -3.35286e+08
//    5  p4          -4.32926e-14   3.93857e-17   1.29878e-19  -3.27464e+12
//    6  p5           7.65560e-18   3.69814e-21   2.29668e-23  -3.32681e+16
//    7  p6          -8.07560e-22   3.38077e-25   2.42268e-27  -3.46843e+20
//    8  p7           5.06304e-26   2.98561e-29   1.51891e-31  -3.68372e+24
//    9  p8          -1.74564e-30   2.50881e-33   5.23691e-36  -3.96772e+28
//   10  p9           2.55389e-35   1.94735e-37   7.66166e-41  -4.32128e+32


}


/******************************************************************************
 *
 */
MPulseForms::~MPulseForms()
{
  // Delete an instance of MPulseForms
}


/******************************************************************************
 *
 */
void MPulseForms::Create()
{
  // Create the main window

  // We start with a name and an icon...
  SetWindowName("Pulse-form analyzer");

  m_MainFrameLayout = new TGLayoutHints(kLHintsExpandX| kLHintsExpandY, 5, 5, 5, 5);
  m_MainFrame = new TGCompositeFrame(this, 80, 60, kHorizontalFrame);
  AddFrame(m_MainFrame, m_MainFrameLayout);

  m_ButtonLayout = new TGLayoutHints(kLHintsExpandX | kLHintsExpandY, 2, 2, 0, 0);

  m_NextButton = new TGTextButton(m_MainFrame, "Next", e_Next);
  m_NextButton->Associate(this);
  m_MainFrame->AddFrame(m_NextButton, m_ButtonLayout);

  m_LoopButton = new TGTextButton(m_MainFrame, "Loop", e_Loop);
  m_LoopButton->Associate(this);
  m_MainFrame->AddFrame(m_LoopButton, m_ButtonLayout);

  m_StopButton = new TGTextButton(m_MainFrame, "Stop", e_Stop);
  m_StopButton->Associate(this);
  m_MainFrame->AddFrame(m_StopButton, m_ButtonLayout);

  m_TestButton = new TGTextButton(m_MainFrame, "Test", e_Test);
  m_TestButton->Associate(this);
  m_MainFrame->AddFrame(m_TestButton, m_ButtonLayout);

  m_StoreButton = new TGTextButton(m_MainFrame, "Store", e_Store);
  m_StoreButton->Associate(this);
  m_MainFrame->AddFrame(m_StoreButton, m_ButtonLayout);

  m_CloseButton = new TGTextButton(m_MainFrame, "Close", e_Close);
  m_CloseButton->Associate(this);
  m_MainFrame->AddFrame(m_CloseButton, m_ButtonLayout);

  // and bring it to the screen.
  MapSubwindows();
  MapWindow();  

  Layout();
 
  PrepareEventAnalysis();  

	if (m_AutoStart == true) {
		OnLoop();
	}

  return;
}


/******************************************************************************
 *
 */
Bool_t MPulseForms::ProcessMessage(Long_t Message, Long_t Parameter1, 
																	 Long_t Parameter2)
{
  // Process the messages for this window

	Bool_t Status = kTRUE;
        
  switch (GET_MSG(Message)) {
  case kC_COMMAND:
    switch (GET_SUBMSG(Message)) {
    case kCM_BUTTON:
      switch (Parameter1) {

      case e_Next:
				Status = OnNext();
        break;
				
      case e_Loop:
				Status = OnLoop();
        break;
				
      case e_Stop:
				Status = OnStop();
        break;
        
      case e_Close:
        Status = OnClose();
        break;
        
      case e_Test:
        Status = OnTest();
        break;
        
      case e_Store:
        Status = OnStore();
        break;
				
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
  
  return Status;
}


/******************************************************************************
 *
 */
void MPulseForms::CloseWindow()
{
	// Close and delete this window

  gApplication->Terminate(0);
}


/******************************************************************************
 *
 */
Bool_t MPulseForms::OnNext()
{
  if (m_CurrentFile >= m_Files.size()) {
    cout<<"No more files left!"<<endl;
		StoreFitFunctions();
    return false;
  }

  if (m_Files[m_CurrentFile]->LoadNextEvent() == false) {
    cout<<"No more events left!"<<endl;
    FinalCalculations();
    m_CurrentFile++;
		cout<<"Switching to new file: "<<m_FileNames[m_CurrentFile]<<endl;
    m_CanvasesPrepared = false;
    return false;
  }
  
  if (m_CanvasesPrepared == false) {
    PrepareEventAnalysis();
  }

  AnalyzeEvent();

	return true;
}


/******************************************************************************
 *
 */
Bool_t MPulseForms::OnLoop()
{
  if (m_CurrentFile >= m_Files.size()) {
    cout<<"No more files left!"<<endl;
		StoreFitFunctions();
    return false;
  }

  m_StopLoop = false;

  while (m_StopLoop == false) {
    if (m_Files[m_CurrentFile]->LoadNextEvent() == false) {
      cout<<"No more events left!"<<endl;
      FinalCalculations();
      m_CurrentFile++;
			if (m_CurrentFile >= m_Files.size()) {
				cout<<"No more files left!"<<endl;
				StoreFitFunctions();
				return false;
			}
      m_CanvasesPrepared = false;
      if (m_StopAtNewFile == true) break;
    }

    if (m_CanvasesPrepared == false) {
      PrepareEventAnalysis();
    }
    AnalyzeEvent();
    gSystem->ProcessEvents();
  }

  //FinalCalculations();

	return true;
}


/******************************************************************************
 *
 */
Bool_t MPulseForms::OnStop()
{
  m_StopLoop = true;

	return true;
}



/******************************************************************************
 *
 */
Bool_t MPulseForms::OnStore()
{
	// The Apply button has been pressed

  StoreFitFunctions();

	return kTRUE;
}


/******************************************************************************
 *
 */
Bool_t MPulseForms::OnTest()
{
	// The Apply button has been pressed

  m_IsTestCalibrationMode = true;

	return kTRUE;
}



/******************************************************************************
 *
 */
Bool_t MPulseForms::OnClose()
{
	// The Apply button has been pressed

	// Store the average:
	if (m_AveragedPulseFormHistogram.size() == m_NPulseForms) {
		Char_t Text[100];
		for (unsigned int i = 0; i < m_NPulseForms; ++i) {
			sprintf(Text, "Average_%i.txt", i+1);
			ofstream out(Text);
			for (int b = 1; b <= m_AveragedPulseFormHistogram[i]->GetNbinsX(); ++b) {
				out<<m_AveragedPulseFormHistogram[i]->GetBinCenter(b)<<";"
					 <<m_AveragedPulseFormHistogram[i]->GetBinContent(b)<<endl;
			}
			out.close();
		}
	}

	StoreFitFunctions();

	CloseWindow();

	return kTRUE;
}


/******************************************************************************
 * Dito
 */
Bool_t MPulseForms::ParseCommandLine(int argc, char *argv[])
{
  ostringstream Usage;
  Usage<<endl;
  Usage<<"  Usage: PulseForms <options>"<<endl;
  Usage<<"    General options:"<<endl;
  Usage<<"         -f:   file name  (type: rawa)"<<endl;
  Usage<<"         -o:   All files in directory (type: rawa)"<<endl;
  Usage<<"         -p:   file name of stored fit to average pulse shape"<<endl;
  Usage<<"         -n:   number of pulse forms"<<endl;
  Usage<<"         -c:   chip which must have triggered (type ika1)"<<endl;
  Usage<<"         -t:   time offset between Tag and pulse start in ns (e.g. 140 for short D1, D2: 754ns or 820 with myons...)"<<endl;
  Usage<<"         -d:   trigger-hold-delay in ns (e.g. 3500 D2, 2000 D1)"<<endl;
  Usage<<"         -b:   pulse-shape bin width in ns (e.g. 200)"<<endl;
  Usage<<"         -h:   This text..."<<endl;
  Usage<<"         -i:   event, which ends initialization"<<endl;
  Usage<<"         --start:   event, with which to start"<<endl;
  Usage<<"         --stop :   event, with which to stop"<<endl;
  Usage<<"         --stop-at-new-file :   dito"<<endl;
  Usage<<"         --auto-start :  Start analysis with program start"<<endl;
  Usage<<"         --myons :  Do the analysis for myons"<<endl;
  Usage<<"         --D1 :  Data is from D1"<<endl;
	Usage<<"         --store-results: Store all final histograms as ps-files"<<endl;
	Usage<<"         --minimum-peak: Minimum number of adc counts for a valid peak"<<endl;
	Usage<<"         --trigger-delay: Time between real trigger and stored one (typical 500 ns)"<<endl;
	Usage<<"         --show-fits: To be used only together with -p: Display the stored fits"<<endl;
	Usage<<"         --show-trigger-distribution: Only show the trigger distribution, nothing else"<<endl;
	Usage<<"         --detector: Restrict analysis to this detector"<<endl;
	Usage<<"         --baseline-sigma: Below this value the sigma of the base-line must lay to be accepted"<<endl;
	Usage<<endl;
  Usage<<"    Show histogram options:"<<endl;
  Usage<<"         -s all:       Show all histograms..."<<endl;
  Usage<<"         -s pf:        Show pulse forms..."<<endl;
  Usage<<"         -s cpf:       Show corrected pulse forms..."<<endl;
  Usage<<"         -s npf:       Show normalized pulse forms..."<<endl;
  Usage<<"         -s npfd:      Show normalized pulse form density..."<<endl;
  Usage<<"         -s apf:       Show averaged pulse forms..."<<endl;
  Usage<<"         -s al:        Show average level..."<<endl;
  Usage<<"         -s ldens:     Show level density..."<<endl;
  Usage<<"         -s ld:        Show level distribution..."<<endl;
  Usage<<"         -s gd:        Show gradient distribution..."<<endl;
  Usage<<"         -s tt:        Show trigger threshold..."<<endl;
  Usage<<"         -s rt:        Show real trigger threshold delay..."<<endl;
  Usage<<"         -s tsig:      Show trigger time per signal..."<<endl;
  Usage<<"         -s thressig:  Show treshold over signal..."<<endl;
  Usage<<"         -s mc:        Show maximum correction..."<<endl;
  Usage<<"         -s mw:        Show maximum wrongness..."<<endl;
  Usage<<"         -s capf:      Show chip average pulse form..."<<endl;
  Usage<<"         -s capfd:     Show chip average pulse form density..."<<endl;
  Usage<<"         -s crt:       Show chip raise time over signal"<<endl;
  Usage<<"         -s ctr:       Show chip signal over raise time..."<<endl;
  Usage<<"         -s ctsig:     Show chip trigger time per signal..."<<endl;
  Usage<<"         -s ctrig:     Show chip trigger time..."<<endl;
  Usage<<"         -s crealt:    Show trigger threshold..."<<endl;
  Usage<<"         -s crealtd:   Show real trigger threshold..."<<endl;
  Usage<<"         -s mrt:       Show raise time for complete detector..."<<endl;
  Usage<<"         -s mtsig:     Show trigger time per signal for complete detector..."<<endl;
  Usage<<"         -s mttime:    Show trigger time for whole detector..."<<endl;
  Usage<<endl;


  MString Option, SubOption;
  MString FileName;
  MString DirectoryName;

  // Check for help
  for (Int_t i = 1; i < argc; i++) {
    Option = argv[i];
    if (Option == "-h" || Option == "--help" || Option == "?" || Option == "-?") {
      cout<<Usage<<endl;
      return false;
    }
  }

  // Now parse the command line options:
  for (Int_t i = 1; i < argc; i++) {
		Option = argv[i];

		// First check if each option has sufficient arguments:
		// Single argument
    if (Option == "-f" ||
				Option == "-p" ||
				Option == "-o" ||
				Option == "-n" ||
				Option == "-t" ||
				Option == "-d" ||
				Option == "-i" ||
				Option == "-b" ||
				Option == "-s" ||
				Option == "--start" ||
				Option == "--stop" ||
				Option == "--baseline-sigma" ||
				Option == "--trigger-delay" ||
				Option == "--detector") {
			if (!((argc > i+1) && argv[i+1][0] != '-')){
				cout<<"Error: Option "<<argv[i][1]<<" needs a second argument!"<<endl;
				cout<<Usage<<endl;
				return false;
			}
		} 
		// Multiple arguments_
		else if (Option == "-c") {
			if (!((argc > i+2) && argv[i+1][0] != '-' && argv[i+2][0] != '-')){
				cout<<"Error: Option "<<argv[i][1]<<" needs two arguments argument!"<<endl;
				cout<<Usage<<endl;
				return false;
			}
		}

		// Then fulfill the options:
		if (Option == "-h") {
			cout<<Usage<<endl;
			return false;
		} else if (Option == "-f") {
			m_FileNames.push_back(argv[++i]);
			m_Files.push_back(new MFileEventsRawa());
			cout<<"Accepting file name: "<<m_FileNames.back()<<endl;
		} else if (Option == "-o") {
			DirectoryName = argv[++i];
			cout<<"Accepting directory: "<<DirectoryName<<endl;
		} else if (Option == "--detector") {
			m_Detector = argv[++i];
			cout<<"Accepting restriction to detector: "<<m_Detector<<endl;
		} else if (Option == "-p") {
			m_CalibFileName = argv[++i];
			cout<<"Accepting calib file name: "<<m_CalibFileName<<endl;
    } else if (Option == "-c") {
			if (atoi(argv[i+1]) <= 0) {
				cout<<"Chip counting starts at 1 not zero!"<<endl;
				return false;
			}
			if (m_TriggeredChips.size() < (unsigned int) atoi(argv[i+1])) {
				m_TriggeredChips.resize(atoi(argv[i+1]));
			}
			m_TriggeredChips[atoi(argv[i+1])-1].push_back(argv[i+2]);
			cout<<"Accepting chip: "<<m_TriggeredChips[atoi(argv[i+1])-1].back()<<endl;
			i += 2;
		} else if (Option == "-n") {
			m_NPulseForms = atoi(argv[++i]);
			cout<<"Accepting number of pulse forms: "<<m_NPulseForms<<endl;
		} else if (Option == "-t") {
			m_PulseStartOffset = atoi(argv[++i]);
			cout<<"Accepting pulse start offset [ns]: "<<m_PulseStartOffset<<endl;
		} else if (Option == "-d") {
			m_TriggerHoldDelay = atoi(argv[++i]);
			cout<<"Accepting trigger hold delay [ns]: "<<m_TriggerHoldDelay<<endl;
		} else if (Option == "-b") {
			m_PulseBinWidth = atoi(argv[++i]);
			cout<<"Accepting pulse bin width [ns]: "<<m_PulseBinWidth<<endl;
		} else if (Option == "-i") {
			m_StartEvent = atoi(argv[++i]);
			cout<<"Accepting event id with which initialization ends: "<<m_StartEvent<<endl;
		} else if (Option == "--start") {
			m_StartRun = atoi(argv[++i]);
			cout<<"Accepting event id with which run starts: "<<m_StartRun<<endl;
		} else if (Option == "--stop") {
			m_StopRun = atoi(argv[++i]);
			cout<<"Accepting event id with which run stops: "<<m_StopRun<<endl;
		} else if (Option == "--minimum-peak") {
			m_Threshold = atof(argv[++i]);
			cout<<"Accepting threshold for maximum: "<<m_Threshold<<endl;
		} else if (Option == "--trigger-delay") {
			m_TriggerDelay = atoi(argv[++i]);
			cout<<"Accepting trigger-delay: "<<m_TriggerDelay<<endl;
		} else if (Option == "--baseline-sigma") {
			m_MaxBaselineSigma = atof(argv[++i]);
			cout<<"Accepting maximum baseline sigma: "<<m_MaxBaselineSigma<<endl;
		} else if (Option == "--stop-at-new-file") {
			m_StopAtNewFile = true;
			cout<<"Accepting stop at new file"<<endl;
		} else if (Option == "--auto-start") {
			m_AutoStart = true;
			cout<<"Accepting auto start"<<endl;
		} else if (Option == "--myons") {
			m_UseMyons = true;
			cout<<"Accepting analysis for myons"<<endl;
		} else if (Option == "--D1") {
			m_IsD1 = true;
			cout<<"Accepting analysis for D1"<<endl;
		} else if (Option == "--store-results") {
			m_StoreResults = true;
			cout<<"Accepting store results as ps-files"<<endl;
		} else if (Option == "--show-fits") {
			m_ShowFits = true;
			cout<<"Accepting show fits"<<endl;
		} else if (Option == "--show-trigger-distribution") {
			m_ShowTriggerDistributionOnly = true;
			m_ShowMEGATriggerTime = true;
			cout<<"Accepting show only trigger distribution"<<endl;
		} else if (Option == "-s") {
			SubOption = argv[++i];
			if (SubOption == "pf") {
				m_ShowPulseForm = true;
				cout<<"Accepting show PulseForm"<<endl;
			} else if (SubOption == "cpf") {
				m_ShowCorrectedPulseForm = true;
				cout<<"Accepting show corrected pulse forms"<<endl;
			} else if (SubOption == "npf") {
				m_ShowNormalizedPulseForm = true;
				cout<<"Accepting show NormalizedPulseForm"<<endl;
			} else if (SubOption == "npfd") {
				m_ShowNormalizedPulseFormDensity = true;
				cout<<"Accepting show NormalizedPulseFormDensity"<<endl;
			} else if (SubOption == "apf") {
				m_ShowAveragedPulseForm = true;
				cout<<"Accepting show AveragedPulseForm"<<endl;
			} else if (SubOption == "al") {
				m_ShowAverageBaseline = true;
				cout<<"Accepting show average level"<<endl;
			} else if (SubOption == "ld") {
				m_ShowBaselineDistribution = true;
				cout<<"Accepting show level distribution"<<endl;
			} else if (SubOption == "ldens") {
				m_ShowBaselineDensity = true;
				cout<<"Accepting show level density"<<endl;
			} else if (SubOption == "gd") {
				m_ShowZeroCrossing = true;
				cout<<"Accepting show gradient distribution"<<endl;
			} else if (SubOption == "tt") {
				m_ShowTriggerThreshold = true;
				cout<<"Accepting show TriggerThreshold"<<endl;
			} else if (SubOption == "tsig") {
				m_ShowTriggerTimePerSignal = true;
				cout<<"Accepting show TriggerTimePerSignal"<<endl;
			} else if (SubOption == "thressig") {
				m_ShowThresholdOverSignal = true;
				cout<<"Accepting show ThresholdOverSignal"<<endl;
			} else if (SubOption == "rt") {
				m_ShowRiseTimePerSignal = true;
				cout<<"Accepting show RiseTimePerSignal"<<endl;
			} else if (SubOption == "mc") {
				m_ShowMaximumCorrection = true;
				cout<<"Accepting show MaximumCorrection"<<endl;
			} else if (SubOption == "mw") {
				m_ShowMaximumWrongness = true;
				cout<<"Accepting show MaximumWrongness"<<endl;
			} else if (SubOption == "capfd") {
				m_ShowAverageChipPulseFormDensity = true;
				cout<<"Accepting show AverageChipPulseFormDensitys"<<endl;
			} else if (SubOption == "capf") {
				m_ShowAverageChipPulseForm = true;
				cout<<"Accepting show AverageChipPulseForms"<<endl;
			} else if (SubOption == "crt") {
				m_ShowChipRiseTimePerSignal = true;
				cout<<"Accepting show ChipRiseTimePerSignal"<<endl;
			} else if (SubOption == "ctr") {
				m_ShowChipSignalOverRise = true;
				cout<<"Accepting show ChipSignalOverRiseTime"<<endl;
			} else if (SubOption == "ctsig") {
				m_ShowChipTriggerTimePerSignal = true;
				cout<<"Accepting show ChipTriggerTimePerSignal"<<endl;
			} else if (SubOption == "ctrig") {
				m_ShowChipTriggerTime = true;
				cout<<"Accepting show ChipTriggerTime"<<endl;
			} else if (SubOption == "crealt") {
				m_ShowChipRealThreshold = true;
				cout<<"Accepting show ChipRealThreshold"<<endl;
			} else if (SubOption == "crealtpb") {
				m_ShowChipRealThresholdPlusBase = true;
				cout<<"Accepting show ChipRealThresholdPlusBase"<<endl;
			} else if (SubOption == "crealtd") {
				m_ShowChipRealThresholdDelay = true;
				cout<<"Accepting show ChipRealThresholdDelay"<<endl;
			} else if (SubOption == "mrt") {
				m_ShowMEGARiseTimePerSignal = true;
				cout<<"Accepting show MEGARiseTimePerSignal"<<endl;
			} else if (SubOption == "mtsig") {
				m_ShowMEGATriggerTimePerSignal = true;
				cout<<"Accepting show MEGATriggerTimePerSignal"<<endl;
			} else if (SubOption == "mttime") {
				m_ShowMEGATriggerTime = true;
				cout<<"Accepting show MEGATriggerTime"<<endl;
			} else if (SubOption == "all") {
				m_ShowPulseForm = true;
				m_ShowNormalizedPulseFormDensity = true;
				m_ShowCorrectedPulseForm = true;
				m_ShowAveragedPulseForm = true;
				m_ShowBaselineDensity = true;
				m_ShowBaselineDistribution = true;
				m_ShowChipRealThreshold = true;
				m_ShowChipRealThresholdPlusBase = true;
				m_ShowChipRealThresholdDelay = true;
				m_ShowTriggerThreshold = true;
				m_ShowTriggerTimePerSignal = true;
				m_ShowThresholdOverSignal = true;
				m_ShowRiseTimePerSignal = true;
				m_ShowMaximumCorrection = true;
				m_ShowMaximumWrongness = true;
				m_ShowAverageChipPulseForm = true;
				m_ShowAverageChipPulseFormDensity = true;
				m_ShowChipRiseTimePerSignal = true;
				m_ShowChipSignalOverRise = true;
				m_ShowChipTriggerTimePerSignal = true;
				m_ShowChipTriggerTime = true;
				m_ShowMEGARiseTimePerSignal = true;
				m_ShowMEGATriggerTimePerSignal = true;
				m_ShowMEGATriggerTime = true;
				cout<<"Accepting show all histograms"<<endl;
			}
		} else {
			cout<<"Error: Unknown option \""<<Option<<"\"!"<<endl;
			cout<<Usage<<endl;
			return false;
		}
  }

	if (m_ShowFits == true && m_CalibFileName != "") {
		ShowFits();
		return true;
	}



  if (DirectoryName != "") {
    // Get all file-names in the directory:
    void* Directory = gSystem->OpenDirectory(DirectoryName);
    const char* s;
    while ((s = gSystem->GetDirEntry(Directory)) != 0) {
      MString Name = s;
      if (Name.EndsWith(".rawa")) {
        m_FileNames.push_back(DirectoryName + "/" + Name);
        m_Files.push_back(new MFileEventsRawa());
			}
		}
    gSystem->FreeDirectory(Directory);
  }

	if (m_Detector != "") {
		//cout<<"Restriction... to "<<m_Detector<<endl;
		for (unsigned int f = 0; f < m_FileNames.size(); ++f) {
			if (m_FileNames[f].Contains(m_Detector) == false) {
				//cout<<"FileName "<<m_FileNames[f]<<" does not contain "<<m_Detector<<endl;
				for (unsigned int i = f; i < m_FileNames.size()-1; ++i) {
					m_FileNames[i] = m_FileNames[i+1];
          delete m_Files[i];
					m_Files[i] = m_Files[i+1];
				}
				m_FileNames.resize(m_FileNames.size()-1);
				m_Files.resize(m_Files.size()-1);
				--f;
				//cout<<"Size: "<<m_FileNames.size()<<endl;
			} else {
				//cout<<"Taking FileName "<<m_FileNames[f]<<" which contains "<<m_Detector<<endl;
			}
		}
	}


  if (m_FileNames.size() == 0) {
    cout<<"Error: A file or directory name must be given!"<<endl;
    cout<<Usage<<endl;
    return false;
  }

  int NOpenFiles = 0;
	cout<<"Size: "<<m_Files.size()<<endl;
  for (unsigned int f = 0; f < m_Files.size(); ++f) {
    if (m_Files[f]->Open(m_FileNames[f]) == false) {
      cout<<"Error: Unable to correctly open file "<<m_FileNames[f]<<"!"<<endl;
    } else {
      cout<<"Opened: "<<m_FileNames[f]<<"!"<<endl;
      NOpenFiles++;
    }
  }
  
	// Check how many chips we have:
	m_ChipsPerFile.resize(m_FileNames.size()); 
	m_ChannelPerFile.resize(m_FileNames.size()); 
	m_ChipIdsPerFile.resize(m_FileNames.size()); 

	int NDummyChips = 0;
	MString Token;
	for (unsigned int f = 0; f < m_FileNames.size(); ++f) {
		// The name MUST contain something like: "_fortuna:Chip#1_hydra:Chip#1_1"
		MString String = m_FileNames[f];
		String.ReplaceAll("_", " ");
		String.ReplaceAll(".", " ");

		MTokenizer T(String);
		// How many chips do we have?
 		for (int t = 0; t < T.GetNTokens(); ++t) {
			Token = T.GetTokenAt(t);
			if (Token.Contains("(null)") == true) {
				ostringstream S;
				S<<"Dummy:Chip#"<<NDummyChips++<<endl;
				Token = S.str().c_str();
			}
			
			if (Token.Contains(":Chip#") == true) {
				// Check if we have stored the chip in the chip list
				bool Found = false;
				for (unsigned int c = 0; c < m_AllChipNames.size(); ++c) {
					if (m_AllChipNames[c] == Token) {
						Found = true;
						m_ChipIdsPerFile[f].push_back(c);
						break;
					}
				}
				if (Found == false) {
					m_AllChipNames.push_back(Token);
					m_ChipIdsPerFile[f].push_back(m_AllChipNames.size()-1);
				}
				// And remember the chip name on a chips per file base:
				m_ChipsPerFile[f].push_back(Token);

				// if the next token does not contain the ":Chip#", then we have found the channel number:
				if (t < T.GetNTokens()) {
					if (T.GetTokenAt(t+1).Contains(":Chip#") == false) {
						m_ChannelPerFile[f] = T.GetTokenAtAsInt(t+1);
					}
				}
			}
		}
	}

	// Display list of chips:
	cout<<"Found chips: "<<endl;
	for (unsigned int c = 0; c < m_AllChipNames.size(); ++c) {
		cout<<"+ "<<m_AllChipNames[c]<<endl;
	}


  if (NOpenFiles == 0) {
    cout<<"Error: Unable to open any file!"<<endl;
    return false;
  }

  if (m_NPulseForms <= 0) {
    cout<<"Error: Need a reasonable number of pulse forms!"<<endl;
    return false;
  }
  
  if (m_PulseBinWidth <= 0) {
    cout<<"Error: Need a reasonable pulse bin width!"<<endl;
    return false;
  }
  
  if (m_TriggerDelay <= 0) {
    cout<<"Error: Need a reasonable trigger delay!"<<endl;
    return false;
  }
    
  if (m_PulseStartOffset <= 0) {
    cout<<"Error: Need a reasonable pulse start offset!"<<endl;
    return false;
  }

  if (m_CalibFileName != "") {
    if (LoadFitFunctions() == false) {
      cout<<"Error: Unable to load fit functions!"<<endl;
      return false;
    } else {
      m_IsTestCalibrationMode = true;
    }
  }

  return true;
}


/******************************************************************************
 * Dito
 */
Bool_t MPulseForms::PrepareEventAnalysis()
{
	if (m_ShowFits == true) return true;

  // Clear all vectors:
  m_Baseline.resize(0);
  m_NBaselines.resize(0);
  m_BaselineFound.resize(0);
  m_AverageSigma.resize(0);
	m_NSigmas.resize(0);
	m_Maximum.resize(0);

  //
  delete m_BaselineFit;
  delete m_MaximumFit;
  delete m_AverageFit;
  delete m_RiseFit;
	delete m_ComplicatedFit;
  delete m_SignalRiseFit;

  // Display
  for (unsigned int i = 0; i < m_PulseFormCanvas.size(); ++i) delete m_PulseFormCanvas[i];
  m_PulseFormCanvas.resize(0);
  for (unsigned int i = 0; i < m_PulseFormHistogram.size(); ++i) delete m_PulseFormHistogram[i];
  m_PulseFormHistogram.resize(0);
  for (unsigned int i = 0; i < m_BaselineFunction.size(); ++i) delete m_BaselineFunction[i];
  m_BaselineFunction.resize(0);
  for (unsigned int i = 0; i < m_TriggerFunction.size(); ++i) delete m_TriggerFunction[i];
  m_TriggerFunction.resize(0);
  for (unsigned int i = 0; i < m_HoldFunction.size(); ++i) delete m_HoldFunction[i];
  m_HoldFunction.resize(0);

  for (unsigned int i = 0; i < m_NormalizedPulseFormCanvas.size(); ++i) delete m_NormalizedPulseFormCanvas[i];
  m_NormalizedPulseFormCanvas.resize(0);
  for (unsigned int i = 0; i < m_NormalizedPulseFormHistogram.size(); ++i) delete m_NormalizedPulseFormHistogram[i];
  m_NormalizedPulseFormHistogram.resize(0);

  for (unsigned int i = 0; i < m_NormalizedPulseFormDensityCanvas.size(); ++i) delete m_NormalizedPulseFormDensityCanvas[i];
  m_NormalizedPulseFormDensityCanvas.resize(0);
  for (unsigned int i = 0; i < m_NormalizedPulseFormDensityHistogram.size(); ++i) delete m_NormalizedPulseFormDensityHistogram[i];
  m_NormalizedPulseFormDensityHistogram.resize(0);

  for (unsigned int i = 0; i < m_CorrectedPulseFormCanvas.size(); ++i) delete m_CorrectedPulseFormCanvas[i];
  m_CorrectedPulseFormCanvas.resize(0);
  for (unsigned int i = 0; i < m_CorrectedPulseFormHistogram.size(); ++i) delete m_CorrectedPulseFormHistogram[i];
  m_CorrectedPulseFormHistogram.resize(0);

  for (unsigned int i = 0; i < m_AveragedPulseFormCanvas.size(); ++i) delete m_AveragedPulseFormCanvas[i];
  m_AveragedPulseFormCanvas.resize(0);
  for (unsigned int i = 0; i < m_AveragedPulseFormHistogram.size(); ++i) delete m_AveragedPulseFormHistogram[i];
  m_AveragedPulseFormHistogram.resize(0);
	m_NAverageBaselines.resize(0);

  for (unsigned int i = 0; i < m_AverageBaselineCanvas.size(); ++i) delete m_AverageBaselineCanvas[i];
  m_AverageBaselineCanvas.resize(0);
  for (unsigned int i = 0; i < m_AverageBaselineHistogram.size(); ++i) delete m_AverageBaselineHistogram[i];
  m_AverageBaselineHistogram.resize(0);
	m_NAverages.resize(0);

  for (unsigned int i = 0; i < m_BaselineDistributionCanvas.size(); ++i) delete m_BaselineDistributionCanvas[i];
  m_BaselineDistributionCanvas.resize(0);
  for (unsigned int i = 0; i < m_BaselineDistributionHistogram.size(); ++i) delete m_BaselineDistributionHistogram[i];
  m_BaselineDistributionHistogram.resize(0);

  for (unsigned int i = 0; i < m_BaselineDensityCanvas.size(); ++i) delete m_BaselineDensityCanvas[i];
  m_BaselineDensityCanvas.resize(0);
  for (unsigned int i = 0; i < m_BaselineDensityHistogram.size(); ++i) delete m_BaselineDensityHistogram[i];
  m_BaselineDensityHistogram.resize(0);

  for (unsigned int i = 0; i < m_ZeroCrossingCanvas.size(); ++i) delete m_ZeroCrossingCanvas[i];
  m_ZeroCrossingCanvas.resize(0);
  for (unsigned int i = 0; i < m_ZeroCrossingHistogram.size(); ++i) delete m_ZeroCrossingHistogram[i];
  m_ZeroCrossingHistogram.resize(0);

  for (unsigned int i = 0; i < m_TriggerThresholdCanvas.size(); ++i) delete m_TriggerThresholdCanvas[i];
  m_TriggerThresholdCanvas.resize(0);
  for (unsigned int i = 0; i < m_TriggerThresholdHistogram.size(); ++i) delete m_TriggerThresholdHistogram[i];
  m_TriggerThresholdHistogram.resize(0);

  for (unsigned int i = 0; i < m_RiseTimePerSignalCanvas.size(); ++i) delete m_RiseTimePerSignalCanvas[i];
  m_RiseTimePerSignalCanvas.resize(0);
  for (unsigned int i = 0; i < m_RiseTimePerSignalHistogram.size(); ++i) delete m_RiseTimePerSignalHistogram[i];
  m_RiseTimePerSignalHistogram.resize(0);

  for (unsigned int i = 0; i < m_TriggerTimePerSignalCanvas.size(); ++i) delete m_TriggerTimePerSignalCanvas[i];
  m_TriggerTimePerSignalCanvas.resize(0);
  for (unsigned int i = 0; i < m_TriggerTimePerSignalHistogram.size(); ++i) delete m_TriggerTimePerSignalHistogram[i];
  m_TriggerTimePerSignalHistogram.resize(0);

  for (unsigned int i = 0; i < m_ThresholdOverSignalCanvas.size(); ++i) delete m_ThresholdOverSignalCanvas[i];
  m_ThresholdOverSignalCanvas.resize(0);
  for (unsigned int i = 0; i < m_ThresholdOverSignalHistogram.size(); ++i) delete m_ThresholdOverSignalHistogram[i];
  m_ThresholdOverSignalHistogram.resize(0);

  for (unsigned int i = 0; i < m_MaximumCorrectionCanvas.size(); ++i) delete m_MaximumCorrectionCanvas[i];
  m_MaximumCorrectionCanvas.resize(0);
  for (unsigned int i = 0; i < m_MaximumCorrectionHistogram.size(); ++i) delete m_MaximumCorrectionHistogram[i];
  m_MaximumCorrectionHistogram.resize(0);
  for (unsigned int i = 0; i < m_MaximumCorrectionBisectorFunction.size(); ++i) delete m_MaximumCorrectionBisectorFunction[i];
  m_MaximumCorrectionBisectorFunction.resize(0);

  for (unsigned int i = 0; i < m_MaximumWrongnessCanvas.size(); ++i) delete m_MaximumWrongnessCanvas[i];
  m_MaximumWrongnessCanvas.resize(0);
  for (unsigned int i = 0; i < m_MaximumWrongnessHistogram.size(); ++i) delete m_MaximumWrongnessHistogram[i];
  m_MaximumWrongnessHistogram.resize(0);
  for (unsigned int i = 0; i < m_MaximumWrongnessBisectorFunction.size(); ++i) delete m_MaximumWrongnessBisectorFunction[i];
  m_MaximumWrongnessBisectorFunction.resize(0);

  for (unsigned int i = 0; i < m_AverageChipPulseFormDensityCanvas.size(); ++i) delete m_AverageChipPulseFormDensityCanvas[i];
  m_AverageChipPulseFormDensityCanvas.resize(0);

  for (unsigned int i = 0; i < m_AverageChipPulseFormCanvas.size(); ++i) delete m_AverageChipPulseFormCanvas[i];
  m_AverageChipPulseFormCanvas.resize(0);

  for (unsigned int i = 0; i < m_ChipRiseTimePerSignalCanvas.size(); ++i) delete m_ChipRiseTimePerSignalCanvas[i];
  m_ChipRiseTimePerSignalCanvas.resize(0);

  for (unsigned int i = 0; i < m_ChipSignalOverRiseCanvas.size(); ++i) delete m_ChipSignalOverRiseCanvas[i];
  m_ChipSignalOverRiseCanvas.resize(0);
  m_ChipSignalOverRiseProfile.resize(m_NPulseForms, 0);

  for (unsigned int i = 0; i < m_ChipTriggerTimePerSignalCanvas.size(); ++i) delete m_ChipTriggerTimePerSignalCanvas[i];
  m_ChipTriggerTimePerSignalCanvas.resize(0);

  for (unsigned int i = 0; i < m_ChipTriggerTimeCanvas.size(); ++i) delete m_ChipTriggerTimeCanvas[i];
  m_ChipTriggerTimeCanvas.resize(0);

  for (unsigned int i = 0; i < m_ChipRealThresholdCanvas.size(); ++i) delete m_ChipRealThresholdCanvas[i];
  m_ChipRealThresholdCanvas.resize(0);

  for (unsigned int i = 0; i < m_ChipRealThresholdPlusBaseCanvas.size(); ++i) delete m_ChipRealThresholdPlusBaseCanvas[i];
  m_ChipRealThresholdPlusBaseCanvas.resize(0);

  for (unsigned int i = 0; i < m_ChipRealThresholdDelayCanvas.size(); ++i) delete m_ChipRealThresholdDelayCanvas[i];
  m_ChipRealThresholdDelayCanvas.resize(0);

	// Do not modify the histograms!
  // for (unsigned int i = 0; i < m_AverageChipPulseFormDensityHistogram.size(); ++i) delete m_AverageChipPulseFormDensityHistogram[i];
  // m_AverageChipPulseFormDensityHistogram.resize(0);


  char Text[100];

  int NHistograms = 0;
  if (m_ShowPulseForm == true) NHistograms++;
  if (m_ShowCorrectedPulseForm == true) NHistograms++;
  if (m_ShowNormalizedPulseForm == true) NHistograms++;
  if (m_ShowNormalizedPulseFormDensity == true) NHistograms++;
  if (m_ShowAveragedPulseForm == true) NHistograms++;
  if (m_ShowAverageBaseline == true) NHistograms++;
  if (m_ShowBaselineDistribution == true) NHistograms++;
  if (m_ShowBaselineDensity == true) NHistograms++;
  if (m_ShowZeroCrossing == true) NHistograms++;
  if (m_ShowTriggerThreshold == true) NHistograms++;
  if (m_ShowRiseTimePerSignal == true) NHistograms++;
  if (m_ShowTriggerTimePerSignal == true) NHistograms++;
  if (m_ShowThresholdOverSignal == true) NHistograms++;
  if (m_ShowMaximumCorrection == true) NHistograms++;
  if (m_ShowMaximumWrongness == true) NHistograms++;

	if (m_ShowAverageChipPulseForm == true) NHistograms++;
	if (m_ShowAverageChipPulseFormDensity == true) NHistograms++;
	if (m_ShowChipRiseTimePerSignal == true) NHistograms++;
	if (m_ShowChipSignalOverRise == true) NHistograms++;
	if (m_ShowChipTriggerTimePerSignal == true) NHistograms++;
	if (m_ShowChipTriggerTime == true) NHistograms++;
  if (m_ShowChipRealThreshold == true) NHistograms++;
  if (m_ShowChipRealThresholdPlusBase == true) NHistograms++;
  if (m_ShowChipRealThresholdDelay == true) NHistograms++;
  
	int NHistogramsY = m_NPulseForms;
	if (m_ShowMEGARiseTimePerSignal == true) NHistogramsY++;
	if (m_ShowMEGATriggerTimePerSignal == true) NHistogramsY++;

	if (NHistograms == 0) NHistograms = 1;
	if (NHistogramsY == 0) NHistogramsY = 1;

  // ... get the width and height of the display ...
  Int_t xDisplay, yDisplay;
  UInt_t wDisplay, hDisplay;
  gVirtualX->GetGeometry(-1, xDisplay, yDisplay, wDisplay, hDisplay);

  int Startx = 0;
  int Starty = 85;
  int xDistance = 8;
  int yDistance = 35;
  //int Width = (wDisplay-100)/NHistograms-xDistance*(NHistogramsY-1);
  int Width = (wDisplay-100)/NHistograms-xDistance;
  //cout<<Width<<"!"<<wDisplay<<"!"<<xDistance*(NHistogramsY-1)<<"!"<<NHistograms<<endl;
  if (Width > 500) Width = 500;
  //int Height = (hDisplay-100)/NHistogramsY-yDistance*(NHistogramsY-1);
  int Height = (hDisplay-100)/NHistogramsY-yDistance;
  if (Height > 350) Height = 350;

	cout<<hDisplay<<"!"<<NHistogramsY<<"!"<<yDistance<<endl;
	cout<<"w="<<Width<<"  h="<<Height<<endl;

  TCanvas* Canvas = 0;
  TH1D* H1D = 0;
  TH2D* H2D = 0;

	if (m_IsMEGALevelInitialized == false) {
		cout<<"Initilaizing MEGA level!"<<endl;
    int HistNumber = 0;

    sprintf(Text, "MEGA Rise-time per signal");
		if (m_UseMyons == true) {
			m_MEGARiseTimePerSignalHistogram = new TH2D(Text, Text, 40, 0, 12000, 40, 0, 1000);
		} else {
			if (m_IsD1 == true) {
				m_MEGARiseTimePerSignalHistogram = new TH2D(Text, Text, 40, 0, 5000, 40, 0, 1000);
			} else {
				m_MEGARiseTimePerSignalHistogram = new TH2D(Text, Text, 40, 0, 12000, 40, 0, 200);
			}
		}
    m_MEGARiseTimePerSignalHistogram->SetXTitle("Time [ns]");
    m_MEGARiseTimePerSignalHistogram->SetYTitle("Adc");
    //m_MEGARiseTimePerSignalHistogram->SetStats(kFALSE);
    if (m_ShowMEGARiseTimePerSignal == true) {
			cout<<"Showing..."<<endl;
      sprintf(Text, "MEGA Rise-time per signal canvas");
      m_MEGARiseTimePerSignalCanvas = 
        new TCanvas(Text, Text, Startx+ HistNumber*(Width+xDistance), 
                    Starty + (NHistogramsY-1)*(Height+yDistance), Width, Height);
      HistNumber++;
      m_MEGARiseTimePerSignalCanvas->cd();
      m_MEGARiseTimePerSignalHistogram->Draw("COLZ");
      m_MEGARiseTimePerSignalCanvas->Update();
    }


    sprintf(Text, "MEGA Trigger-time per signal");
		if (m_UseMyons == true) {
			m_MEGATriggerTimePerSignalHistogram = new TH2D(Text, Text, 40, 0, 12000, 40, 0, 1000);
		} else {
			if (m_IsD1 == true) {
				m_MEGATriggerTimePerSignalHistogram = new TH2D(Text, Text, 40, 0, 12000, 40, 0, 1000);
			} else {
				m_MEGATriggerTimePerSignalHistogram = new TH2D(Text, Text, 40, 0, 12000, 40, 0, 200);
			}
		}
    m_MEGATriggerTimePerSignalHistogram->SetXTitle("Time [ns]");
    m_MEGATriggerTimePerSignalHistogram->SetYTitle("Adc");
    m_MEGATriggerTimePerSignalHistogram->SetStats(kFALSE);
    if (m_ShowMEGATriggerTimePerSignal == true) {
      sprintf(Text, "MEGA Trigger-time per signal canvas");
      m_MEGATriggerTimePerSignalCanvas = 
        new TCanvas(Text, Text, Startx+ HistNumber*(Width+xDistance), 
                    Starty + (NHistogramsY-1)*(Height+yDistance), Width, Height);
      HistNumber++;
      m_MEGATriggerTimePerSignalCanvas->cd();
      m_MEGATriggerTimePerSignalHistogram->Draw("COLZ");
      m_MEGATriggerTimePerSignalCanvas->Update();
    }


    sprintf(Text, "MEGA Trigger-time");
		m_MEGATriggerTimeHistogram = new TH1D(Text, Text, 100, 0, 10000);
    m_MEGATriggerTimeHistogram->SetXTitle("Time [ns]");
    m_MEGATriggerTimeHistogram->SetYTitle("#");
    m_MEGATriggerTimeHistogram->SetStats(kFALSE);
    if (m_ShowMEGATriggerTime == true) {
     sprintf(Text, "MEGA Trigger-time");
      m_MEGATriggerTimeCanvas = 
        new TCanvas(Text, Text, Startx+ HistNumber*(Width+xDistance), 
                    Starty + (NHistogramsY-1)*(Height+yDistance), Width, Height);
      HistNumber++;
      m_MEGATriggerTimeCanvas->cd();
      m_MEGATriggerTimeHistogram->Draw();
      m_MEGATriggerTimeCanvas->Update();
    }

		m_IsMEGALevelInitialized = true;
	}

	if (m_IsChipLevelInitialized == false) {
		for (unsigned int c = 0; c < m_AllChipNames.size(); ++c) {
			sprintf(Text, "%s: Average chip pulse form", m_AllChipNames[c].Data());
			H1D = new TH1D(Text, Text, 100, 0, 20000);
			m_AverageChipPulseFormHistogram.push_back(H1D);
			H1D->SetMaximum(1.1);
			H1D->SetMinimum(0.0);
			H1D->SetXTitle("Time [ns]");
			H1D->SetYTitle("Form");
			H1D->SetStats(kFALSE);

			sprintf(Text, "%s: Chip Trigger Time", m_AllChipNames[c].Data());
			H1D = new TH1D(Text, Text, 50, 0, 10000);
			m_ChipTriggerTimeHistogram.push_back(H1D);
			H1D->SetMinimum(0.0);
			H1D->SetXTitle("Time [ns]");
			H1D->SetYTitle("#");
			H1D->SetStats(kFALSE);

			sprintf(Text, "%s: Chip Real threshold", m_AllChipNames[c].Data());
			H1D = new TH1D(Text, Text, 50, 0, 100);
			m_ChipRealThresholdHistogram.push_back(H1D);
			H1D->SetMinimum(0.0);
			H1D->SetXTitle("Threshold [Adc]");
			H1D->SetYTitle("#");
			//H1D->SetStats(kFALSE);

			sprintf(Text, "%s: Chip Real threshold Plus Baseline", m_AllChipNames[c].Data());
			H1D = new TH1D(Text, Text, 300, 0, 300);
			m_ChipRealThresholdPlusBaseHistogram.push_back(H1D);
			H1D->SetMinimum(0.0);
			H1D->SetXTitle("Threshold [Adc]");
			H1D->SetYTitle("#");
			//H1D->SetStats(kFALSE);

			sprintf(Text, "%s: Chip Real threshold delay", m_AllChipNames[c].Data());
			H1D = new TH1D(Text, Text, 50, 0, 1500);
			m_ChipRealThresholdDelayHistogram.push_back(H1D);
			H1D->SetMinimum(0.0);
			H1D->SetXTitle("Time [ns]");
			H1D->SetYTitle("#");
			//H1D->SetStats(kFALSE);

			sprintf(Text, "%s: Average chip pulse form density", m_AllChipNames[c].Data());
			H2D = new TH2D(Text, Text, 50, 0, 20000, 200, -0.2, 1.2);
			m_AverageChipPulseFormDensityHistogram.push_back(H2D);
			H2D->SetXTitle("Time [ns]");
			H2D->SetYTitle("Form");
			H2D->SetStats(kFALSE);

			sprintf(Text, "%s: Trigger time per signal", m_AllChipNames[c].Data());
			if (m_UseMyons == true) {
				H2D = new TH2D(Text, Text, 40, 0, 12000, 40, 0, 1000);
			} else {
				if (m_IsD1 == true) { 
          H2D = new TH2D(Text, Text, 40, 0, 12000, 40, 0, 1000);
        } else {
          H2D = new TH2D(Text, Text, 40, 0, 12000, 40, 0, 200);
        }
      }
			m_ChipTriggerTimePerSignalHistogram.push_back(H2D);
			H2D->SetXTitle("Time [ns]");
			H2D->SetYTitle("Form");
			H2D->SetStats(kFALSE);

			sprintf(Text, "%s: Rise time per signal", m_AllChipNames[c].Data());
			if (m_UseMyons == true) {
				H2D = new TH2D(Text, Text, 40, 0, 12000, 40, 0, 1000);
			} else {
				if (m_IsD1 == true) { 
					H2D = new TH2D(Text, Text, 40, 0, 5000, 40, 0, 1000);
				} else {
					H2D = new TH2D(Text, Text, 40, 0, 12000, 40, 0, 200);					
				}
			}
			m_ChipRiseTimePerSignalHistogram.push_back(H2D);
			H2D->SetXTitle("Time [ns]");
			H2D->SetYTitle("Form");
			H2D->SetStats(kFALSE);

			sprintf(Text, "%s: Signal over rise time", m_AllChipNames[c].Data());
			if (m_UseMyons == true) {
				H2D = new TH2D(Text, Text, 40, 0, 1000, 40, 0, 12000);
			} else {
				if (m_IsD1 == true) { 
					H2D = new TH2D(Text, Text, 40, 0, 1000, 40, 0, 12000);
				} else {
					H2D = new TH2D(Text, Text, 40, 0, 200, 40, 0, 12000);					
				}
			}
			m_ChipSignalOverRiseHistogram.push_back(H2D);
			H2D->SetXTitle("Rise-Time [ns]");
			H2D->SetYTitle("Signal [ADC]");
			H2D->SetStats(kFALSE);
		}
		m_AverageChipPulseFormNAverages.resize(m_AllChipNames.size(), 0);
		m_AvgRealThreshold.resize(m_AllChipNames.size());
		m_AvgRealThresholdDelay.resize(m_AllChipNames.size());
		m_MaximumRisePolynom.resize(m_AllChipNames.size());
		for (unsigned int i = 0; i < m_MaximumRisePolynom.size(); ++i) {
			m_MaximumRisePolynom[i].resize(2, 0);
		}
	}

  for (unsigned int i = 0; i < m_NPulseForms; ++i) {
		char CommonID[100];
		sprintf(CommonID, "%s ch %i: ", m_AllChipNames[m_ChipIdsPerFile[m_CurrentFile][i]].Data(), m_ChannelPerFile[m_CurrentFile]);
    int HistNumber = 0;

    sprintf(Text, "%sPulse-form histogram %i", CommonID, i+1);
    H1D = new TH1D(Text, Text, 100, 0, 20000);
    m_PulseFormHistogram.push_back(H1D);
    H1D->SetXTitle("Time [ns]");
    H1D->SetYTitle("Adc");
		if (m_IsD1 == true) {
			H1D->SetMaximum(1000);
			H1D->SetMinimum(0);
		} else {
			H1D->SetMaximum(350);
			H1D->SetMinimum(0);
		}
    // H1D->SetStats(kFALSE);
    if (m_ShowPulseForm == true) {
      sprintf(Text, "%sPulse-form canvas %i", CommonID, i+1);
      Canvas = new TCanvas(Text, Text, Startx + HistNumber*(Width+xDistance), Starty + i*(Height+yDistance), Width, Height);
      //cout<<"w!h: "<<Width<<"!"<<Height<<endl;
      HistNumber++;
      m_PulseFormCanvas.push_back(Canvas);
      Canvas->cd();
      H1D->Draw();
      Canvas->Update();
    }

    m_BaselineFunction.push_back(new TH1D(Text, Text, 100, 0, 20000)); 
    m_BaselineFunction.back()->SetLineColor(3);
    m_TriggerFunction.push_back(new TH1D(Text, Text, 100, 0, 20000)); 
    m_TriggerFunction.back()->SetLineColor(3);
    m_HoldFunction.push_back(new TH1D(Text, Text, 100, 0, 20000)); 
    m_HoldFunction.back()->SetLineColor(3);


    sprintf(Text, "%sAverage level histogram %i", CommonID, i+1);
    H1D = new TH1D(Text, Text, 100, 0, 20000);
    m_AverageBaselineHistogram.push_back(H1D);
    H1D->SetXTitle("Time [ns]");
    H1D->SetYTitle("Adc");
		if (m_IsD1 == true) {
			H1D->SetMaximum(1000);
			H1D->SetMinimum(0);
		} else {
			H1D->SetMaximum(250);
			H1D->SetMinimum(0);
		}
		// H1D->SetStats(kFALSE);
    if (m_ShowAverageBaseline == true) {
      sprintf(Text, "%sAverage level canvas%i", CommonID, i+1);
      Canvas = new TCanvas(Text, Text, Startx+ HistNumber*(Width+xDistance), Starty + i*(Height+yDistance), Width, Height);
      HistNumber++;
      m_AverageBaselineCanvas.push_back(Canvas);
      Canvas->cd();
      H1D->Draw();
      Canvas->Update();
    }


    sprintf(Text, "%sCorrected pulse-form histogram %i", CommonID, i+1);
    H1D = new TH1D(Text, Text, 100, 0, 20000);
    m_CorrectedPulseFormHistogram.push_back(H1D);
    H1D->SetXTitle("Time [ns]");
    H1D->SetYTitle("Adc");
    if (m_UseMyons == true) {
      H1D->SetMaximum(1000);
    } else {
			if (m_IsD1 == true) {
				H1D->SetMaximum(1000);
			} else {
				H1D->SetMaximum(150);
			}
		}
		H1D->SetMinimum(-10);
    // H1D->SetStats(kFALSE);
    if (m_ShowCorrectedPulseForm == true) {
      sprintf(Text, "%sCorrected pulse-form canvas%i", CommonID, i+1);
      Canvas = new TCanvas(Text, Text, Startx + HistNumber*(Width+xDistance), Starty + i*(Height+yDistance), Width, Height);
      HistNumber++;
      m_CorrectedPulseFormCanvas.push_back(Canvas);
      Canvas->cd();
      H1D->Draw();
      Canvas->Update();
    }


    sprintf(Text, "%sNormalized pulse-form histogram %i", CommonID, i+1);
    H1D = new TH1D(Text, Text, 100, 0, 20000);
    m_NormalizedPulseFormHistogram.push_back(H1D);
    H1D->SetXTitle("Time [ns]");
    H1D->SetYTitle("Adc");
    H1D->SetMaximum(1.2);
    H1D->SetMinimum(0.0);
    // H2D->SetStats(kFALSE);
    if (m_ShowNormalizedPulseForm == true) {
			cout<<"Adding normalized pulseform canvas!"<<endl;
      sprintf(Text, "%sNormalized pulse-form canvas%i", CommonID, i+1);
      Canvas = new TCanvas(Text, Text, Startx + HistNumber*(Width+xDistance), Starty + i*(Height+yDistance), Width, Height);
      HistNumber++;
      m_NormalizedPulseFormCanvas.push_back(Canvas);
      Canvas->cd();
      H1D->Draw();
      Canvas->Update();
    }

    sprintf(Text, "%sNormalized pulse-form density histogram %i", CommonID, i+1);
    H2D = new TH2D(Text, Text, 100, 0, 20000, 200, 0.0, 1.2);
    m_NormalizedPulseFormDensityHistogram.push_back(H2D);
    H2D->SetXTitle("Time [ns]");
    H2D->SetYTitle("Adc");
    // H2D->SetStats(kFALSE);
    if (m_ShowNormalizedPulseFormDensity == true) {
      sprintf(Text, "%sNormalized pulse-form density canvas%i", CommonID, i+1);
      Canvas = new TCanvas(Text, Text, Startx + HistNumber*(Width+xDistance), Starty + i*(Height+yDistance), Width, Height);
      HistNumber++;
      m_NormalizedPulseFormDensityCanvas.push_back(Canvas);
      Canvas->cd();
      H2D->Draw();
      Canvas->Update();
    }


    sprintf(Text, "%sAveraged pulse-form histogram %i", CommonID, i+1);
    H1D = new TH1D(Text, Text, 100, 0, 20000);
    m_AveragedPulseFormHistogram.push_back(H1D);
    H1D->SetXTitle("Time [ns]");
    H1D->SetYTitle("Adc");
		H1D->SetMaximum(1.1);
		H1D->SetMinimum(0.0);
    // H1D->SetStats(kFALSE);
    if (m_ShowAveragedPulseForm == true) {
      sprintf(Text, "%sAveraged pulse-form canvas%i", CommonID, i+1);
      Canvas = new TCanvas(Text, Text, Startx+ HistNumber*(Width+xDistance), Starty + i*(Height+yDistance), Width, Height);
      HistNumber++;
      m_AveragedPulseFormCanvas.push_back(Canvas);
      Canvas->cd();
      H1D->Draw();
      Canvas->Update();
    }


    sprintf(Text, "%sTriggerThreshold histogram %i", CommonID, i+1);
    H2D = new TH2D(Text, Text, 20, 0, 50, 20, 0, 6000);
    m_TriggerThresholdHistogram.push_back(H2D);
    H2D->SetXTitle("Threshold [Adc]");
    H2D->SetYTitle("TAG-Trigger Delay [ns]");
    // H2D->SetStats(kFALSE);
    if (m_ShowTriggerThreshold == true) {
      sprintf(Text, "%sTrigger Threshold %i", CommonID, i+1);
      Canvas = new TCanvas(Text, Text, Startx+ HistNumber*(Width+xDistance), Starty + i*(Height+yDistance), Width, Height);
      HistNumber++;
      m_TriggerThresholdCanvas.push_back(Canvas);
      Canvas->cd();
      H2D->Draw();
      Canvas->Update();
    }


    sprintf(Text, "%sBaseline distribution histogram %i", CommonID, i+1);
    H1D = new TH1D(Text, Text, 200, 100, 250);
    m_BaselineDistributionHistogram.push_back(H1D);
    H1D->SetXTitle("Baseline [Adc]");
    H1D->SetYTitle("#");
    // H2D->SetStats(kFALSE);
    if (m_ShowBaselineDistribution == true) {
      sprintf(Text, "%sBaseline distribution %i", CommonID, i+1);
      Canvas = new TCanvas(Text, Text, Startx+ HistNumber*(Width+xDistance), Starty + i*(Height+yDistance), Width, Height);
      HistNumber++;
      m_BaselineDistributionCanvas.push_back(Canvas);
      Canvas->cd();
      H1D->Draw();
      Canvas->Update();
    }

    sprintf(Text, "%sBaseline density histogram %i", CommonID, i+1);
    H2D = new TH2D(Text, Text, 200, 0, 20000, 250, 0, 250);
    m_BaselineDensityHistogram.push_back(H2D);
    H2D->SetXTitle("Time [ns]");
    H2D->SetYTitle("Baseline [Adc]");
    // H2D->SetStats(kFALSE);
    if (m_ShowBaselineDensity == true) {
      sprintf(Text, "%sBaseline density %i", CommonID, i+1);
      Canvas = new TCanvas(Text, Text, Startx+ HistNumber*(Width+xDistance), Starty + i*(Height+yDistance), Width, Height);
      HistNumber++;
      m_BaselineDensityCanvas.push_back(Canvas);
      Canvas->cd();
      H2D->Draw("COLZ");
      Canvas->Update();
    }


    sprintf(Text, "%sZero crossing histogram %i", CommonID, i+1);
    H1D = new TH1D(Text, Text, 20, 0, 2000);
    m_ZeroCrossingHistogram.push_back(H1D);
    H1D->SetXTitle("Gradient [Adc]");
    H1D->SetYTitle("#");
    // H2D->SetStats(kFALSE);
    if (m_ShowZeroCrossing == true) {
      sprintf(Text, "%sZero crossing distribution %i", CommonID, i+1);
      Canvas = new TCanvas(Text, Text, Startx+ HistNumber*(Width+xDistance), Starty + i*(Height+yDistance), Width, Height);
      HistNumber++;
      m_ZeroCrossingCanvas.push_back(Canvas);
      Canvas->cd();
      H1D->Draw();
      Canvas->Update();
    }


    sprintf(Text, "%sRise-time per signal %i", CommonID, i+1);
    if (m_UseMyons == true) {
      H2D = new TH2D(Text, Text, 40, 0, 12000, 40, 0, 1000);
    } else {
			if (m_IsD1 == true) {
				H2D = new TH2D(Text, Text, 40, 0, 5000, 40, 0, 1000);
			} else {
				H2D = new TH2D(Text, Text, 40, 0, 12000, 40, 0, 200);
			}
		}
    m_RiseTimePerSignalHistogram.push_back(H2D);
    H2D->SetXTitle("Time [ns]");
    H2D->SetYTitle("Adc");
    H2D->SetStats(kFALSE);
    if (m_ShowRiseTimePerSignal == true) {
      sprintf(Text, "%sRise-time per signal canvas%i", CommonID, i+1);
      Canvas = new TCanvas(Text, Text, Startx+ HistNumber*(Width+xDistance), Starty + i*(Height+yDistance), Width, Height);
      HistNumber++;
      m_RiseTimePerSignalCanvas.push_back(Canvas);
      Canvas->cd();
      H2D->Draw("COLZ");
      Canvas->Update();
    }


    sprintf(Text, "%sTrigger-time per signal %i", CommonID, i+1);
    if (m_UseMyons == true) {
      H2D = new TH2D(Text, Text, 40, 0, 12000, 40, 0, 1000);
    } else {
			if (m_IsD1 == true) {
        H2D = new TH2D(Text, Text, 40, 0, 12000, 40, 0, 1000);
      } else { 
        H2D = new TH2D(Text, Text, 40, 0, 12000, 40, 0, 200);
      }
    }
    m_TriggerTimePerSignalHistogram.push_back(H2D);
    H2D->SetXTitle("Time [ns]");
    H2D->SetYTitle("Adc");
    H2D->SetStats(kFALSE);
    if (m_ShowTriggerTimePerSignal == true) {
      sprintf(Text, "%sTrigger-time per signal canvas%i", CommonID, i+1);
      Canvas = new TCanvas(Text, Text, Startx+ HistNumber*(Width+xDistance), Starty + i*(Height+yDistance), Width, Height);
      HistNumber++;
      m_TriggerTimePerSignalCanvas.push_back(Canvas);
      Canvas->cd();
      H2D->Draw("COLZ");
      Canvas->Update();
    }


    sprintf(Text, "%sThreshold over signal %i", CommonID, i+1);
    if (m_UseMyons == true) {
      H2D = new TH2D(Text, Text, 40, 0, 1000, 40, 0, 100);
    } else {
			if (m_IsD1 == true) {
        H2D = new TH2D(Text, Text, 40, 0, 1000, 40, 0, 1000);
      } else {
        H2D = new TH2D(Text, Text, 40, 0, 200, 40, 0, 100);
      }
    }
    m_ThresholdOverSignalHistogram.push_back(H2D);
    H2D->SetXTitle("Signal [Adc]");
    H2D->SetYTitle("Threshold [Adc]");
    H2D->SetStats(kFALSE);
    if (m_ShowThresholdOverSignal == true) {
      sprintf(Text, "%sThreshold over signal canvas%i", CommonID, i+1);
      Canvas = new TCanvas(Text, Text, Startx+ HistNumber*(Width+xDistance), Starty + i*(Height+yDistance), Width, Height);
      HistNumber++;
      m_ThresholdOverSignalCanvas.push_back(Canvas);
      Canvas->cd();
      H2D->Draw("COLZ");
      Canvas->Update();
    }


    sprintf(Text, "%sMaximum correction %i", CommonID, i+1);
    if (m_UseMyons == true) {
      H2D = new TH2D(Text, Text, 50, 0, 1000, 50, 0, 1000);
    } else {
      H2D = new TH2D(Text, Text, 50, 0, 100, 50, 0, 100);
    }
    m_MaximumCorrectionHistogram.push_back(H2D);
    H2D->SetXTitle("Corrected maximum [adc]");
    H2D->SetYTitle("Real maximum [adc]");
    H2D->SetStats(kFALSE);
    if (m_ShowMaximumCorrection == true) {
      sprintf(Text, "%sMaximum correction canvas%i", CommonID, i+1);
      Canvas = new TCanvas(Text, Text, Startx+ HistNumber*(Width+xDistance), Starty + i*(Height+yDistance), Width, Height);
      HistNumber++;
      m_MaximumCorrectionCanvas.push_back(Canvas);
      Canvas->cd();
      H2D->Draw("COLZ");
      Canvas->Update();
    }
    sprintf(Text, "%sMaximum correction bisector %i", CommonID, i+1);
    if (m_UseMyons == true) {
      H1D = new TH1D(Text, Text, 50, 0, 1000);
    } else {
      H1D = new TH1D(Text, Text, 50, 0, 100);
    }
    H1D->SetLineColor(3);
    m_MaximumCorrectionBisectorFunction.push_back(H1D);
    for (int b = 1; b <= 50; ++b) {
      H1D->SetBinContent(b, 2*(b-0.5));
    }

    sprintf(Text, "%sMaximum wrongness %i", CommonID, i+1);
    if (m_UseMyons == true) {
      H2D = new TH2D(Text, Text, 50, 0, 1000, 50, 0, 1000);
    } else {
      H2D = new TH2D(Text, Text, 50, 0, 100, 50, 0, 100);
    }
    m_MaximumWrongnessHistogram.push_back(H2D);
    H2D->SetXTitle("Wrong maximum [adc]");
    H2D->SetYTitle("Real maximum [adc]");
    H2D->SetStats(kFALSE);
    if (m_ShowMaximumWrongness == true) {
      sprintf(Text, "%sMaximum wrongness canvas%i", CommonID, i+1);
      Canvas = new TCanvas(Text, Text, Startx+ HistNumber*(Width+xDistance), Starty + i*(Height+yDistance), Width, Height);
      HistNumber++;
      m_MaximumWrongnessCanvas.push_back(Canvas);
      Canvas->cd();
      H2D->Draw("COLZ");
      Canvas->Update();
    }
    sprintf(Text, "%sMaximum wrongness bisector %i", CommonID, i+1);
    if (m_UseMyons == true) {
      H1D = new TH1D(Text, Text, 50, 0, 1000);
    } else {
      H1D = new TH1D(Text, Text, 50, 0, 100);
    }
    H1D->SetLineColor(3);
    m_MaximumWrongnessBisectorFunction.push_back(H1D);
    for (int b = 1; b <= 50; ++b) {
			if (m_UseMyons == true) {
				H1D->SetBinContent(b, 2*(b-0.5));
			} else {
				H1D->SetBinContent(b, 2*(b-0.5));
			}
    }


    if (m_ShowAverageChipPulseForm == true) {
      sprintf(Text, "%sAverageChipPulseForm canvas%i", CommonID, i+1);
      Canvas = new TCanvas(Text, Text, Startx+ HistNumber*(Width+xDistance), Starty + i*(Height+yDistance), Width, Height);
      HistNumber++;
      m_AverageChipPulseFormCanvas.push_back(Canvas);
      Canvas->cd();
			// Search for the chip histogram:
			H1D = m_AverageChipPulseFormHistogram[m_ChipIdsPerFile[m_CurrentFile][i]];
			H1D->Draw();
			Canvas->Update();
    }

    if (m_ShowChipTriggerTime == true) {
      sprintf(Text, "%sChipTriggerTime canvas%i", CommonID, i+1);
      Canvas = new TCanvas(Text, Text, Startx+ HistNumber*(Width+xDistance), Starty + i*(Height+yDistance), Width, Height);
      HistNumber++;
      m_ChipTriggerTimeCanvas.push_back(Canvas);
      Canvas->cd();
			// Search for the chip histogram:
			H1D = m_ChipTriggerTimeHistogram[m_ChipIdsPerFile[m_CurrentFile][i]];
			H1D->Draw();
			Canvas->Update();
    }

    if (m_ShowChipRealThreshold == true) {
      sprintf(Text, "%sChipRealThreshold canvas%i", CommonID, i+1);
      Canvas = new TCanvas(Text, Text, Startx+ HistNumber*(Width+xDistance), Starty + i*(Height+yDistance), Width, Height);
      HistNumber++;
      m_ChipRealThresholdCanvas.push_back(Canvas);
      Canvas->cd();
			// Search for the chip histogram:
			H1D = m_ChipRealThresholdHistogram[m_ChipIdsPerFile[m_CurrentFile][i]];
			H1D->Draw();
			Canvas->Update();
    }

    if (m_ShowChipRealThresholdPlusBase == true) {
      sprintf(Text, "%sChipRealThresholdPlusBase canvas%i", CommonID, i+1);
      Canvas = new TCanvas(Text, Text, Startx+ HistNumber*(Width+xDistance), Starty + i*(Height+yDistance), Width, Height);
      HistNumber++;
      m_ChipRealThresholdPlusBaseCanvas.push_back(Canvas);
      Canvas->cd();
			// Search for the chip histogram:
			H1D = m_ChipRealThresholdPlusBaseHistogram[m_ChipIdsPerFile[m_CurrentFile][i]];
			H1D->Draw();
			Canvas->Update();
    }

    if (m_ShowChipRealThresholdDelay == true) {
      sprintf(Text, "%sChipRealThresholdDelay canvas%i", CommonID, i+1);
      Canvas = new TCanvas(Text, Text, Startx+ HistNumber*(Width+xDistance), Starty + i*(Height+yDistance), Width, Height);
      HistNumber++;
      m_ChipRealThresholdDelayCanvas.push_back(Canvas);
      Canvas->cd();
			// Search for the chip histogram:
			H1D = m_ChipRealThresholdDelayHistogram[m_ChipIdsPerFile[m_CurrentFile][i]];
			H1D->Draw();
			Canvas->Update();
    }

    if (m_ShowAverageChipPulseFormDensity == true) {
      sprintf(Text, "%sAverageChipPulseFormDensity canvas%i", CommonID, i+1);
      Canvas = new TCanvas(Text, Text, Startx+ HistNumber*(Width+xDistance), Starty + i*(Height+yDistance), Width, Height);
      HistNumber++;
      m_AverageChipPulseFormDensityCanvas.push_back(Canvas);
      Canvas->cd();
			// Search for the chip histogram:
			H2D = m_AverageChipPulseFormDensityHistogram[m_ChipIdsPerFile[m_CurrentFile][i]];
			H2D->Draw("COLZ");
			Canvas->Update();
    }

    if (m_ShowChipRiseTimePerSignal == true) {
      sprintf(Text, "%sChipRiseTimePerSignal canvas%i", CommonID, i+1);
      Canvas = new TCanvas(Text, Text, Startx+ HistNumber*(Width+xDistance), Starty + i*(Height+yDistance), Width, Height);
      HistNumber++;
      m_ChipRiseTimePerSignalCanvas.push_back(Canvas);
      Canvas->cd();
			// Search for the chip histogram:
			H2D = m_ChipRiseTimePerSignalHistogram[m_ChipIdsPerFile[m_CurrentFile][i]];
			H2D->Draw("COLZ");
			Canvas->Update();
		}

    if (m_ShowChipSignalOverRise == true) {
      sprintf(Text, "%sChipSignalOverRise canvas%i", CommonID, i+1);
      Canvas = new TCanvas(Text, Text, Startx+ HistNumber*(Width+xDistance), Starty + i*(Height+yDistance), Width, Height);
      HistNumber++;
      m_ChipSignalOverRiseCanvas.push_back(Canvas);
      Canvas->cd();
			// Search for the chip histogram:
			H2D = m_ChipSignalOverRiseHistogram[m_ChipIdsPerFile[m_CurrentFile][i]];
			H2D->Draw("COLZ");
			Canvas->Update();
		}

		if (m_ShowChipTriggerTimePerSignal == true) {
			sprintf(Text, "%sChipTriggerTimePerSignal canvas%i", CommonID, i+1);
			Canvas = new TCanvas(Text, Text, Startx+ HistNumber*(Width+xDistance), Starty + i*(Height+yDistance), Width, Height);
			HistNumber++;
			m_ChipTriggerTimePerSignalCanvas.push_back(Canvas);
			Canvas->cd();
			// Search for the chip histogram:
			H2D = m_ChipTriggerTimePerSignalHistogram[m_ChipIdsPerFile[m_CurrentFile][i]];
			H2D->Draw("COLZ");
			Canvas->Update();
		}
	}

	m_TriggeredChips.resize(m_NPulseForms);
	
	m_Baseline.resize(m_NPulseForms, 0);
	m_BaselineFound.resize(m_NPulseForms, false);
	m_BaselinesFinished.resize(m_NPulseForms, false);
	m_NBaselines.resize(m_NPulseForms, 0);
	m_BaselineFit = new TF1("BaselineFit", "pol0");
	m_BaselineFit->SetLineColor(3);
	
	m_AverageSigma.resize(m_NPulseForms, 0);
	m_NSigmas.resize(m_NPulseForms, 0);
	
	m_Maximum.resize(m_NPulseForms, 0);
	m_NAverages.resize(m_NPulseForms, 0);
	m_NAverageBaselines.resize(m_NPulseForms, 0);
		
	m_MaximumFit = new TF1("MaximumFit", "pol9");
	m_MaximumFit->SetLineColor(2);
	
	m_AverageFit = new TF1("AverageFit", "pol9");
	m_AverageFit->SetLineColor(3);
		
	m_RiseFit = new TF1("RiseFit", "pol1");
	m_RiseFit->SetLineColor(4);
		
	if (m_UseMyons == false) {
		m_SignalRiseFit = new TF1("SignalRiseFit", "pol1");
	} else {
		m_SignalRiseFit = new TF1("SignalRiseFit", "pol1");
	}
	m_SignalRiseFit->SetLineColor(3);
	
	if (m_UseMyons == false) {
		if (m_IsD1 == true) {
			m_ComplicatedFit = new TF1("ComplicatedFit", ComplicatedFit, 10, 600, 22);
		} else {
			m_ComplicatedFit = new TF1("ComplicatedFit", ComplicatedFit, 10, 170, 22);
		}
	} else {
		m_ComplicatedFit = new TF1("ComplicatedFit", ComplicatedFit, 10, 600, 22);
	}
	m_ComplicatedFit->SetLineColor(2);

	m_CanvasesPrepared = true;
	m_IsChipLevelInitialized = true;

	return true;
}


/******************************************************************************
 * Dito
 */
Bool_t MPulseForms::AnalyzeEvent()
{
	if (m_Files[m_CurrentFile]->GetEventId() < m_StartRun) return true;
	if (m_Files[m_CurrentFile]->GetEventId() > m_StopRun) return false;

	cout<<endl;
	cout<<"Current event: "<<m_Files[m_CurrentFile]->GetEventId()<<endl;

	for (unsigned int i = 0; i < m_NPulseForms; ++i) {
		cout<<"Analyzing: "<<i+1<<endl;
		if (FillHistograms(i) == false) continue;
		if (m_ShowTriggerDistributionOnly == false) {
			if (FindBaselines(i) == false) continue;
			if (m_Files[m_CurrentFile]->GetEventId() < m_StartEvent) continue;
			if (CheckTriggerMask(i) == false) continue;
			if (CorrectPulseForm(i) == false) continue;
			if (FindMaxima(i) == false) continue;
		}
		if (FindTriggerThreshold(i) == false) continue;
		if (m_ShowTriggerDistributionOnly == true) continue;
		if (CreateNormalizedPulseFormDensityHistogram(i) == false) continue;
		if (m_IsTestCalibrationMode == true) {
			TestCalibration(i);
		}
	}

	return true;
}


/******************************************************************************
 * Dito
 */
Bool_t MPulseForms::FillHistograms(Int_t i)
{
	// Check if we have the expected number of pulse forms:
	if (m_Files[m_CurrentFile]->GetNPulseFormAdcs() < m_NPulseForms) {
		cout<<"Error: Not expected number of pulse forms: "
				<<m_Files[m_CurrentFile]->GetNPulseFormAdcs()<<endl;
		return false;
	}

	vector<int> Data;

  cout<<"N PFs: "<<m_Files[m_CurrentFile]->GetNPulseFormAdcs()<<endl;
  if (m_Files[m_CurrentFile]->GetNPulseFormAdcs() == 8) {
    if (m_IsD1 == true) {
      Data = m_Files[m_CurrentFile]->GetPulseFormAt(i);
    } else {
      Data = m_Files[m_CurrentFile]->GetPulseFormAt(i+6);
    }
  } else {
    Data = m_Files[m_CurrentFile]->GetPulseFormAt(i);
  }
	m_PulseFormHistogram[i]->Reset();
	m_PulseFormHistogram[i]->SetBins(Data.size(), 0, m_PulseBinWidth*Data.size());
	m_CorrectedPulseFormHistogram[i]->SetBins(Data.size(), 0, m_PulseBinWidth*Data.size());
	m_NormalizedPulseFormHistogram[i]->SetBins(Data.size(), 0, m_PulseBinWidth*Data.size());
	m_NormalizedPulseFormDensityHistogram[i]->SetBins(Data.size(), 0, m_PulseBinWidth*Data.size(), 200, 0, 1.2);
	m_BaselineDensityHistogram[i]->SetBins(Data.size(), 0, m_PulseBinWidth*Data.size(), 250, 0, 250);
	m_AverageBaselineHistogram[i]->SetBins(Data.size(), 0, m_PulseBinWidth*Data.size());
	m_AveragedPulseFormHistogram[i]->SetBins(Data.size(), 0, m_PulseBinWidth*Data.size());
	cout<<"Start"<<m_AverageChipPulseFormHistogram.size()<<"!"<<i<<endl;
	m_AverageChipPulseFormHistogram[i]->SetBins(Data.size(), 0, m_PulseBinWidth*Data.size());
	cout<<"Fail1"<<endl;
	m_AverageChipPulseFormDensityHistogram[i]->SetBins(Data.size(), 0, m_PulseBinWidth*Data.size(), 200, 0, 1.2);
	cout<<"Fail2"<<endl;
	m_BaselineFunction[i]->SetBins(Data.size(), 0, m_PulseBinWidth*Data.size());
	m_TriggerFunction[i]->SetBins(Data.size(), 0, m_PulseBinWidth*Data.size());
	m_HoldFunction[i]->SetBins(Data.size(), 0, m_PulseBinWidth*Data.size());
	for (unsigned int b = 0; b < Data.size(); ++b) {
		m_PulseFormHistogram[i]->SetBinContent(b+1, Data[b]);
		//cout<<"  H->SetBinContent("<<b+1<<", "<<Data[b]<<");"<<endl;
	}
	//m_PulseFormHistogram[i]->Smooth(10);

	if (m_ShowPulseForm == true) {
		m_PulseFormCanvas[i]->cd();
		m_PulseFormCanvas[i]->Clear();
		m_PulseFormHistogram[i]->Draw();
		m_PulseFormCanvas[i]->Update();
	}

	if (m_BaselinesFinished[i] == true) {
		for (int b = 1; b <= m_PulseFormHistogram[i]->GetNbinsX(); ++b) {
			m_CorrectedPulseFormHistogram[i]->
				SetBinContent(b, m_PulseFormHistogram[i]->GetBinContent(b) - 
											m_AverageBaselineHistogram[i]->GetBinContent(b));
		}
                
		if (m_ShowCorrectedPulseForm == true) {
			m_CorrectedPulseFormCanvas[i]->cd();
			m_CorrectedPulseFormHistogram[i]->Draw();
			m_CorrectedPulseFormCanvas[i]->Update();
		}
	}


	return true;
}


/******************************************************************************
 * Determine the level == offset of the pulse-form histogram
 */
Bool_t MPulseForms::FindBaselines(Int_t i)
{

	// Check if we already have all levels:
	if (m_BaselineFound[i] == true && m_Files[m_CurrentFile]->GetEventId() > m_StartEvent) {
		if (m_BaselinesFinished[i] == false) {
			if (m_IsD1 == false) {
				m_PulseFormHistogram[i]->SetMaximum(m_Baseline[i]+130);
				m_PulseFormHistogram[i]->SetMinimum(m_Baseline[i]-20);
			}
			m_BaselinesFinished[i] = true;
			//m_AverageBaselineHistogram[i]->Smooth(10);
			if (m_ShowAverageBaseline == true) {
				m_AverageBaselineCanvas[i]->cd();
				m_AverageBaselineHistogram[i]->Draw();
				m_AverageBaselineCanvas[i]->Update();
			}
		}
		return true;
	}

	// Calculate the missing levels:
	if (m_MaxBaselineSigma < 0) {
		if (m_IsD1 == true) {
			m_MaxBaselineSigma = 20.0;
		} else {
			m_MaxBaselineSigma = 4.0;
		}
	}

	// Fit a level to the histogram
	m_PulseFormHistogram[i]->Fit("BaselineFit", "Q");

	if (m_ShowPulseForm == true) {
		m_PulseFormCanvas[i]->cd();
		m_PulseFormHistogram[i]->Draw();
		m_PulseFormCanvas[i]->Update();
		if (m_PulseFormHistogram[i]->GetMaximum() > 5) {
			cout<<"Setting range: "<<m_PulseFormHistogram[i]->GetMaximumBin()-10<<"!"
					<<m_PulseFormHistogram[i]->GetMaximumBin()+10<<endl;
			m_PulseFormHistogram[i]->
				SetAxisRange(m_PulseFormHistogram[i]->GetMaximumBin()-10, 
										 m_PulseFormHistogram[i]->GetMaximumBin()+10);
		}
	}
	double Baseline = m_BaselineFit->GetParameter(0);
	if (m_BaselineFound[i] == true) {
		cout<<"Current level: "<<Baseline<<"  accepted level: "<<m_Baseline[i]<<endl;
	} else {
		cout<<"Current level: "<<Baseline<<endl;
	}
  
	m_BaselineDistributionHistogram[i]->Fill(Baseline, 1);

	if (m_ShowBaselineDistribution == true) {
		m_BaselineDistributionCanvas[i]->cd();
		m_BaselineDistributionHistogram[i]->Draw();
		m_BaselineDistributionCanvas[i]->Update();
	} 
  
	for (int b = 1; b <= m_BaselineDensityHistogram[i]->GetNbinsX(); ++b) {
		m_BaselineDensityHistogram[i]->Fill(m_BaselineDensityHistogram[i]->GetBinCenter(b), m_PulseFormHistogram[i]->GetBinContent(b), 1);
	}
	if (m_ShowBaselineDensity == true) {
		m_BaselineDensityCanvas[i]->cd();
		m_BaselineDensityHistogram[i]->Draw("COLZ");
		m_BaselineDensityCanvas[i]->Update();
	} 


	// Check if the data fits to the level - calculate root mean square:
	double Sigma = 0;
	for (int b = 1; b <= m_PulseFormHistogram[i]->GetNbinsX(); ++b) {
		Sigma += (m_PulseFormHistogram[i]->GetBinContent(b) - Baseline)*
			(m_PulseFormHistogram[i]->GetBinContent(b) - Baseline);
	}
	if (m_PulseFormHistogram[i]->GetNbinsX() > 1) {
		Sigma = sqrt(1.0/(m_PulseFormHistogram[i]->GetNbinsX()-1)*Sigma);
		m_AverageSigma[i] = (m_NSigmas[i]*m_AverageSigma[i] + Sigma)/(m_NSigmas[i]+1);
		m_NSigmas[i]++;
                
		cout<<"Sigma: "<<Sigma<<" --> avg: "<<m_AverageSigma[i]<<endl;
      
		if (Sigma < m_MaxBaselineSigma) {
			m_Baseline[i] = (m_NBaselines[i]*m_Baseline[i] + Baseline)/(m_NBaselines[i]+1);
			cout<<"New acceptable level found: "<<Baseline<<" avg: "<<m_Baseline[i]<<endl;
			m_NBaselines[i]++;
			m_BaselineFound[i] = true;
      
			for (int b = 1; b <= m_BaselineFunction[i]->GetNbinsX(); ++b) {
				m_BaselineFunction[i]->SetBinContent(b, m_Baseline[i]);
			}

			// In addition fill the average level histogram:
			for (int b = 1; b <= m_AverageBaselineHistogram[i]->GetNbinsX(); ++b) {
				m_AverageBaselineHistogram[i]->
					SetBinContent(b, (m_NAverageBaselines[i]*m_AverageBaselineHistogram[i]->GetBinContent(b) + 
														m_PulseFormHistogram[i]->GetBinContent(b))/(m_NAverageBaselines[i]+1));
			}
			m_NAverageBaselines[i]++;
                        
			if (m_ShowAverageBaseline == true) {
				m_AverageBaselineCanvas[i]->cd();
				m_AverageBaselineHistogram[i]->Draw();
				m_AverageBaselineCanvas[i]->Update();
			}
		}
	}

	return m_BaselineFound[i];
}


/******************************************************************************
 * Dito
 */
Bool_t MPulseForms::CheckTriggerMask(Int_t i)
{
	for (unsigned int c = 0; c < m_TriggeredChips[i].size(); ++c) {
		if (m_Files[m_CurrentFile]->HasTriggered(m_TriggeredChips[i][c]) == false) {
			cout<<"Trigger mask does not fit (Mask: ";
			for (unsigned int d = 0; d < m_TriggeredChips[i].size()-1; ++d) {
				cout<<m_TriggeredChips[i][d]<<", ";
			}
			if (m_TriggeredChips[i].size() > 0) {
				cout<<m_TriggeredChips[i][m_TriggeredChips[i].size()-1]<<" Triggers: ";
			}
			for (unsigned int d = 0; d < m_Files[m_CurrentFile]->GetTriggerPattern().size()-1 && m_Files[m_CurrentFile]->GetTriggerPattern().size() > 0; ++d) {
				cout<<"Size: "<<m_Files[m_CurrentFile]->GetTriggerPattern().size()<<endl;
				cout<<m_Files[m_CurrentFile]->GetTriggerPattern()[d]<<", ";
			}
			if (m_Files[m_CurrentFile]->GetTriggerPattern().size() > 0 ) {
				cout<<m_Files[m_CurrentFile]->GetTriggerPattern()[m_Files[m_CurrentFile]->GetTriggerPattern().size()-1];
			}
			cout<<")"<<endl;
			return false;
		}
	}

	return true;
}


/******************************************************************************
 * Dito
 */
Bool_t MPulseForms::CorrectPulseForm(Int_t i) 
{
	return true;
}


/******************************************************************************
 * Dito
 */
Bool_t MPulseForms::FindMaxima(Int_t i)
{
	// Find the maximum:

	bool ReturnCode = true;

	// Fit Gaussian around maximum:
	double Maximum = m_CorrectedPulseFormHistogram[i]->GetBinContent(m_CorrectedPulseFormHistogram[i]->GetMaximumBin()); 
	if (Maximum < m_Threshold) {
		cout<<"No valid maximum!"<<endl;
		ReturnCode = false;
	} else {
		//      double FitMin = m_CorrectedPulseFormHistogram[i]->GetBinCenter(0);
		//      double FitMax = m_CorrectedPulseFormHistogram[i]->GetBinCenter(m_CorrectedPulseFormHistogram[i]->GetNbinsX());

		//     double FitMin = m_CorrectedPulseFormHistogram[i]->
		//       GetBinCenter(m_CorrectedPulseFormHistogram[i]->GetMaximumBin() -
		//                    0.1*m_CorrectedPulseFormHistogram[i]->GetNbinsX());
		//     double FitMax = m_CorrectedPulseFormHistogram[i]->
		//       GetBinCenter(m_CorrectedPulseFormHistogram[i]->GetMaximumBin() +
		//                    0.4*m_CorrectedPulseFormHistogram[i]->GetNbinsX());

		double FitMin;
		double FitMax;
		if (m_IsD1 == true) {
			FitMin = m_CorrectedPulseFormHistogram[i]->
				GetBinCenter(1 + (int) (0.3*m_CorrectedPulseFormHistogram[i]->GetMaximumBin()));
			FitMax = m_CorrectedPulseFormHistogram[i]->
				GetBinCenter(25 + (int) (2*m_CorrectedPulseFormHistogram[i]->GetMaximumBin()));
		} else {
			FitMin = m_CorrectedPulseFormHistogram[i]->
				GetBinCenter(5 + (int) 0.3*m_CorrectedPulseFormHistogram[i]->GetMaximumBin());  /// <--- Correct also here some day
			FitMax = m_CorrectedPulseFormHistogram[i]->
				GetBinCenter(25 + (int) 3*m_CorrectedPulseFormHistogram[i]->GetMaximumBin());
		}

    cout<<"Fit-Range for maximum-fit (bins): "
        <<1 + (int) (0.3*m_CorrectedPulseFormHistogram[i]->GetMaximumBin())
        <<"!"<<25 + (int) (2*m_CorrectedPulseFormHistogram[i]->GetMaximumBin())
        <<"!"<<m_CorrectedPulseFormHistogram[i]->GetMaximumBin()<<endl;
		m_MaximumFit->SetRange(FitMin, FitMax);
		//m_CorrectedPulseFormHistogram[i]->Fit("MaximumFit", "QR0");
		m_CorrectedPulseFormHistogram[i]->Fit("MaximumFit", "QR0");

		//         Double_t Mean, Sigma;
		//         m_CorrectedPulseFormHistogram[i]->Smooth(10);
		//         FitPeak(m_CorrectedPulseFormHistogram[i], m_MaximumFit, Mean, Sigma, -1, -1);

		double xMax = FindMaximumX(m_MaximumFit, FitMin, FitMax);
		double DelayTime = xMax + m_PulseStartOffset;
		m_Maximum[i] = m_MaximumFit->Eval(xMax);

		//cout<<"chisquare: "<<m_MaximumFit->GetChisquare()<<endl;
		if (m_Maximum[i] < m_Threshold) {
			cout<<"No valid maximum!"<<endl;
			ReturnCode = false;
		} else {
			cout<<"Valid Maximum: x="<<xMax<<" fmin="<<FitMin<<" fmax="<<FitMax<<" y="<<m_Maximum[i]<<endl;
			// Now calculate the raise:
			double RangeMin, RangeMax;
			RangeMin = 100;
			RangeMax = 0.5*xMax; 
      cout<<"Rise-fit: "<<RangeMin<<"!"<<RangeMax<<endl;
			m_RiseFit->SetRange(RangeMin, RangeMax);
			m_CorrectedPulseFormHistogram[i]->Fit("RiseFit", "RL");
			if (m_ShowCorrectedPulseForm == true) {
				m_CorrectedPulseFormCanvas[i]->cd();
				m_CorrectedPulseFormHistogram[i]->Draw();
				m_MaximumFit->Draw("Same");
				m_RiseFit->Draw("Same");
				m_CorrectedPulseFormCanvas[i]->Update();
			}

			double GradientMinFactor;
			double GradientMaxFactor;

			if (m_IsD1 == false) {
				GradientMinFactor = 0.0;
				GradientMaxFactor = 0.5;
			} else {
				GradientMinFactor = 0.05;
				GradientMaxFactor = 0.8;
			}


			double RelativeMaximum = m_Maximum[i];
        
			// Check if we have a maximum above a certain threshold
			if (RelativeMaximum < m_Threshold) {
				cout<<"Maximum not above threshold!"<<endl;
				ReturnCode = false;
			}

			double Gradient = (m_RiseFit->Eval(0.5*RangeMax) - m_RiseFit->Eval(RangeMin))/(0.5*RangeMax-RangeMin);

        
			// Check if it starts not too late:
			cout<<"Gradient: "<<Gradient<<"  allowed:["<<GradientMinFactor<<", "<<GradientMaxFactor<<"]"
					<<m_RiseFit->Eval(RangeMin)<<"<"<<m_RiseFit->Eval(0.5*RangeMax)<<"!"<<RangeMin<<"<"<<0.5*RangeMax<<endl;
			if (ReturnCode == true && (Gradient < GradientMinFactor || Gradient > GradientMaxFactor)) {
				cout<<"Out of gradient: "<<Gradient<<"  allowed:["<<GradientMinFactor<<", "<<GradientMaxFactor<<"]"
						<<m_RiseFit->Eval(RangeMin)<<"<"<<m_RiseFit->Eval(0.5*RangeMax)<<"!"<<RangeMin<<"<"<<0.5*RangeMax<<endl;
				//cout<<"Check: "<<m_MaximumFit->Eval(0)<<"!"<<m_MaximumFit->Eval(1000)<<endl;
				ReturnCode = false;
			}

			// Check if it starts not too early
			if (m_IsD1 == false) {
				if (ReturnCode == true && (m_CorrectedPulseFormHistogram[i]->GetBinContent(m_CorrectedPulseFormHistogram[i]->FindBin(0)) > m_Threshold)) {
					cout<<"Starts too early!"<<endl;
					ReturnCode = false;
				}
			}

			// To early or too late events:
			if (m_IsD1 == true) {
				int NBinsSum = 5;
				double Sum = 0;
				double MinSum = 0.025*RelativeMaximum;
				double MaxSum = 0.25*RelativeMaximum;
				for (int b = 0; b <= NBinsSum; ++b) {
					Sum += m_CorrectedPulseFormHistogram[i]->GetBinContent(b);
				}
				Sum /= NBinsSum;
				cout<<"BinSum criteria: "<<MinSum<<" < "<<Sum<<" < "<<MaxSum<<endl;
				if (Sum < MinSum || Sum > MaxSum) {
					cout<<"BinSum criteria failed!"<<endl;
					ReturnCode = false;
				}
			}

			// Find the zero point and store it
			if (ReturnCode == true) {
				double ZeroPoint = FindZeroCrossing(m_RiseFit, RangeMax, 0.2) + m_PulseStartOffset;
				m_ZeroCrossingHistogram[i]->Fill(ZeroPoint, 1);
				//cout<<"Zero-Point:"<<ZeroPoint<<endl;
				if (m_ShowZeroCrossing == true) {
					m_ZeroCrossingCanvas[i]->cd();
					m_ZeroCrossingHistogram[i]->Draw();
					m_ZeroCrossingCanvas[i]->Update();
				}
				
				m_RiseTimePerSignalHistogram[i]->Fill(DelayTime, m_Maximum[i]);
				if (m_ShowRiseTimePerSignal == true) {
					m_RiseTimePerSignalCanvas[i]->cd();
					m_RiseTimePerSignalHistogram[i]->Draw("COLZ");
					m_RiseTimePerSignalCanvas[i]->Update();
				}

				m_ChipRiseTimePerSignalHistogram[m_ChipIdsPerFile[m_CurrentFile][i]]->Fill(DelayTime, m_Maximum[i]);
				if (m_ShowChipRiseTimePerSignal == true) {
					m_ChipRiseTimePerSignalCanvas[i]->cd();
					m_ChipRiseTimePerSignalHistogram[m_ChipIdsPerFile[m_CurrentFile][i]]->Draw("COLZ");
					m_ChipRiseTimePerSignalCanvas[i]->Update();
				}

				m_ChipSignalOverRiseHistogram[m_ChipIdsPerFile[m_CurrentFile][i]]->Fill(m_Maximum[i], DelayTime);
				if (m_ShowChipSignalOverRise == true) {
					m_ChipSignalOverRiseCanvas[i]->cd();
					m_ChipSignalOverRiseHistogram[m_ChipIdsPerFile[m_CurrentFile][i]]->Draw("COLZ");
					m_ChipSignalOverRiseCanvas[i]->Update();
				}

				m_MEGARiseTimePerSignalHistogram->Fill(DelayTime, m_Maximum[i], 1);
				if (m_ShowMEGARiseTimePerSignal == true) {
					m_MEGARiseTimePerSignalCanvas->cd();
					m_MEGARiseTimePerSignalHistogram->Draw("COLZ");
					m_MEGARiseTimePerSignalCanvas->Update();
				}
			}
		}
	}

	if (m_ShowCorrectedPulseForm == true) {
		m_CorrectedPulseFormCanvas[i]->cd();
		m_BaselineFunction[i]->Draw("SAME");
		m_CorrectedPulseFormCanvas[i]->Update();    
	}

	return ReturnCode;
}


/******************************************************************************
 * Dito
 */
Bool_t MPulseForms::FindTriggerThreshold(Int_t i)
{
	double Delay;

	if (m_IsD1 == false) {
    Delay = m_Files[m_CurrentFile]->GetTimeTillD2Trigger();
	} else {
		Delay = m_Files[m_CurrentFile]->GetTimeWalk();
	}

  if (m_ShowMEGATriggerTime == false) {
    Delay -= m_PulseStartOffset;
  }

	Delay -= m_TriggerDelay;

  double Threshold = m_PulseFormHistogram[i]->GetBinContent(m_PulseFormHistogram[i]->FindBin(Delay)) - m_Baseline[i];


	m_TriggerThresholdHistogram[i]->Fill(Threshold, m_Files[m_CurrentFile]->GetTimeTillD2Trigger(), 1);
	if (m_ShowTriggerThreshold == true) {
		m_TriggerThresholdCanvas[i]->cd();
		m_TriggerThresholdHistogram[i]->Draw("COLZ");
		m_TriggerThresholdCanvas[i]->Update();
	}

	for (int b = 1; b <= m_TriggerFunction[i]->GetNbinsX(); ++b) {
		if (m_TriggerFunction[i]->GetBinCenter(b) < Delay) {
			m_TriggerFunction[i]->SetBinContent(b, 0.0);
		} else { 
			m_TriggerFunction[i]->SetBinContent(b, 100000.0);
		}
	}

	if (m_ShowCorrectedPulseForm == true) {
		m_CorrectedPulseFormCanvas[i]->cd();
		m_TriggerFunction[i]->Draw("SAME");
		m_CorrectedPulseFormCanvas[i]->Update();
	}

  cout<<"Threshold: "<<m_PulseFormHistogram[i]->FindBin(Delay)<<"  delay="<<Delay<<endl;

	m_ThresholdOverSignalHistogram[i]->
    Fill(m_Maximum[i], Threshold, 1);
	if (m_ShowThresholdOverSignal == true) {
		m_ThresholdOverSignalCanvas[i]->cd();
		m_ThresholdOverSignalHistogram[i]->Draw("COLZ");
		m_ThresholdOverSignalCanvas[i]->Update();
	}

	m_TriggerTimePerSignalHistogram[i]->Fill(Delay, m_Maximum[i], 1);
	if (m_ShowTriggerTimePerSignal == true) {
		m_TriggerTimePerSignalCanvas[i]->cd();
		m_TriggerTimePerSignalHistogram[i]->Draw("COLZ");
		m_TriggerTimePerSignalCanvas[i]->Update();
	}

	m_ChipTriggerTimeHistogram[m_ChipIdsPerFile[m_CurrentFile][i]]->Fill(Delay, 1);
	if (m_ShowChipTriggerTime == true) {
		m_ChipTriggerTimeCanvas[i]->cd();
		m_ChipTriggerTimeHistogram[m_ChipIdsPerFile[m_CurrentFile][i]]->Draw();
		m_ChipTriggerTimeCanvas[i]->Update();
	}

	m_ChipTriggerTimePerSignalHistogram[m_ChipIdsPerFile[m_CurrentFile][i]]->Fill(Delay, m_Maximum[i], 1);
	if (m_ShowChipTriggerTimePerSignal == true) {
		m_ChipTriggerTimePerSignalCanvas[i]->cd();
		m_ChipTriggerTimePerSignalHistogram[m_ChipIdsPerFile[m_CurrentFile][i]]->Draw("COLZ");
		m_ChipTriggerTimePerSignalCanvas[i]->Update();
	}

	m_MEGATriggerTimePerSignalHistogram->Fill(Delay, m_Maximum[i], 1);
	if (m_ShowMEGATriggerTimePerSignal == true) {
		m_MEGATriggerTimePerSignalCanvas->cd();
		m_MEGATriggerTimePerSignalHistogram->Draw("COLZ");
		m_MEGATriggerTimePerSignalCanvas->Update();
	}

	m_MEGATriggerTimeHistogram->Fill(Delay, 1);
	if (m_ShowMEGATriggerTime == true) {
		m_MEGATriggerTimeCanvas->cd();
		m_MEGATriggerTimeHistogram->Draw();
		m_MEGATriggerTimeCanvas->Update();
	}

	return true;
}


/******************************************************************************
 * Dito
 */
Bool_t MPulseForms::CreateNormalizedPulseFormDensityHistogram(Int_t i)
{
	Double_t NormFactor = 1.0/m_Maximum[i];
                

	for (int b = 1; b <= m_CorrectedPulseFormHistogram[i]->GetNbinsX(); ++b) {
		m_NormalizedPulseFormDensityHistogram[i]->Fill(m_CorrectedPulseFormHistogram[i]->GetBinCenter(b), 
																									 NormFactor*(m_CorrectedPulseFormHistogram[i]->GetBinContent(b)), 1);
		m_NormalizedPulseFormHistogram[i]->SetBinContent(b, 
																										 NormFactor*(m_CorrectedPulseFormHistogram[i]->GetBinContent(b)));
	}
        
	// H1D->Smooth(5);
        
	if (m_ShowNormalizedPulseForm == true) {
		m_NormalizedPulseFormCanvas[i]->cd();
		m_NormalizedPulseFormHistogram[i]->Draw();
		m_NormalizedPulseFormCanvas[i]->Update();
	}
        
	if (m_ShowNormalizedPulseFormDensity == true) {
		m_NormalizedPulseFormDensityCanvas[i]->cd();
		m_NormalizedPulseFormDensityHistogram[i]->Draw("COLZ");
		m_NormalizedPulseFormDensityCanvas[i]->Update();
	}
                
	for (int b = 1; b <= m_AveragedPulseFormHistogram[i]->GetNbinsX(); ++b) {
		m_AveragedPulseFormHistogram[i]->
			SetBinContent(b, (m_NAverages[i]*m_AveragedPulseFormHistogram[i]->GetBinContent(b) + 
												NormFactor*m_CorrectedPulseFormHistogram[i]->GetBinContent(b))/(m_NAverages[i]+1));
	}
	m_NAverages[i]++;

	if (m_ShowAveragedPulseForm == true) {
		m_AveragedPulseFormCanvas[i]->cd();
		m_AveragedPulseFormHistogram[i]->Draw();
		m_AveragedPulseFormCanvas[i]->Update();
	}

	return true;
}


/******************************************************************************
 * Dito
 */
Bool_t MPulseForms::StoreFitFunctions()
{
	char Name[100];
	MTime Time;
	sprintf(Name, "Fits.%s.csv", Time.GetShortString().Data());
	ofstream CalibFile(Name);
  
	CalibFile<<"Version 1"<<endl;
	CalibFile<<"# File format:"<<endl;
	CalibFile<<"# PF <10 Parameters of Polynom> <min valid x value> <max valid x value> <x of maximum> <pulse start offset>"<<endl;
	CalibFile<<"# Function is normalized to 1"<<endl;
	CalibFile<<"# PT <Avg. threshold> <Avg. threshold delay>"<<endl;
	CalibFile<<"# PR <2 parameters of rise polynom in signal over rise-time space in adc>"<<endl;
	CalibFile<<endl;

	double MinFit;
	double MaxFit;

	if (m_IsD1 == true) {
		MinFit = 0.0;
		MaxFit = 5000.0;
	} else {
		MinFit = 0.0;
		MaxFit = 12000.0;
	}

	for (unsigned int c = 0; c < m_AllChipNames.size(); ++c) {
		CalibFile<<"CN "<<m_AllChipNames[c]<<endl;

	
	// 	// Do the fit...
		if (m_IsD1 == true) {
			// Create a temporary histogram in order to expand to times < 0
			TCanvas* Canvas = new TCanvas("PulseShapeExpansionCanvas", "PulseShapeExpansionCanvas", 0, 0, 640, 480);
			TH1D* H1D = new TH1D("PulseShapeExpansion", 
													 "PulseShapeExpansion",
													 (int) (m_AverageChipPulseFormHistogram[c]->GetNbinsX()*1.2),
													 m_AverageChipPulseFormHistogram[c]->GetXaxis()->GetXmin() - 
													 0.2*(m_AverageChipPulseFormHistogram[c]->GetXaxis()->GetXmax() - 
																m_AverageChipPulseFormHistogram[c]->GetXaxis()->GetXmin()),
													 m_AverageChipPulseFormHistogram[c]->GetXaxis()->GetXmax());

			for (int b = 1; b <= m_AverageChipPulseFormHistogram[c]->GetNbinsX(); ++b) { 
				H1D->SetBinContent((int) (b+m_AverageChipPulseFormHistogram[c]->GetNbinsX()*0.2), m_AverageChipPulseFormHistogram[c]->GetBinContent(b));
				cout<<"Setting: "<<b+m_AverageChipPulseFormHistogram[c]->GetNbinsX()*0.2<<"!"<<m_AverageChipPulseFormHistogram[c]->GetBinContent(b)<<endl;
			}

			double FitMin = 0.0;
			double FitMax = 500.0;
			m_RiseFit->SetRange(FitMin, FitMax);
			H1D->Fit("RiseFit", "R");
			
			int StartBin = -1;
			for (int b = 1; b <= m_AverageChipPulseFormHistogram[c]->GetNbinsX()*0.2; ++b) { 
				if (m_RiseFit->Eval(H1D->GetBinCenter(b)) > 0) {
					H1D->SetBinContent(b, m_RiseFit->Eval(H1D->GetBinCenter(b)));
					if (StartBin == -1) {
						StartBin = b;
					}
				}
			}

			MinFit = H1D->GetBinCenter(StartBin);
			m_AverageFit->SetRange(H1D->GetBinCenter(StartBin), MaxFit);
			H1D->Fit("AverageFit", "QR");

			Canvas->cd();
			H1D->Draw();
			Canvas->Update();

			delete H1D;
			delete Canvas;
		} else {
			m_AverageFit->SetRange(MinFit, MaxFit);
			m_AverageChipPulseFormHistogram[c]->Fit("AverageFit", "QR");
		}

		// Now find the maximum of the fit function in order to normalize it to 1
		double xMax = 0.0;
		double Max = 0.0;
		for (double x = MinFit; x < MaxFit; x += 1.0) {
			if (m_AverageFit->Eval(x) > Max) {
				Max = m_AverageFit->Eval(x);
				xMax = x;
			}
		}
		
		if (Max == 0) {
			cout<<"Error: No reasonable fit function defined!"<<endl;
			CalibFile<<"PF 0 0 0 0 0 0 0 0 0 0 0 0 0"<<endl;
		} else {
			CalibFile<<"PF ";
			CalibFile<<m_AverageFit->GetParameter(0)/Max<<" ";
			CalibFile<<m_AverageFit->GetParameter(1)/Max<<" ";
			CalibFile<<m_AverageFit->GetParameter(2)/Max<<" ";
			CalibFile<<m_AverageFit->GetParameter(3)/Max<<" ";
			CalibFile<<m_AverageFit->GetParameter(4)/Max<<" ";
			CalibFile<<m_AverageFit->GetParameter(5)/Max<<" ";
			CalibFile<<m_AverageFit->GetParameter(6)/Max<<" ";
			CalibFile<<m_AverageFit->GetParameter(7)/Max<<" ";
			CalibFile<<m_AverageFit->GetParameter(8)/Max<<" ";
			CalibFile<<m_AverageFit->GetParameter(9)/Max<<" ";
			CalibFile<<MinFit<<" "<<MaxFit<<" "<<xMax<<" "<<m_PulseStartOffset<<endl;
		}
		
		CalibFile<<"PT "<<m_AvgRealThreshold[c]<<" "<<m_AvgRealThresholdDelay[c]<<endl;
		CalibFile<<"PR "<<m_MaximumRisePolynom[c][0]<<" "<<m_MaximumRisePolynom[c][1]<<endl;
		CalibFile<<endl;
	}

	CalibFile.close();

	return true;
}


/******************************************************************************
 * Dito
 */
Bool_t MPulseForms::LoadFitFunctions(bool StartFromScratch)
{
	ifstream CalibFile(m_CalibFileName);
  
	unsigned int NLines = 0;
	bool Error = false;
	double p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, fmin, fmax, xmax;
	const int LineLength = 1000;
	char LineBuffer[LineLength];
	char NameBuffer[100];

	if (StartFromScratch == true) {
		m_CalibPolynom.resize(0);
		m_CalibPolynomName.resize(0);
	} else {
		m_CalibPolynom.resize(m_AllChipNames.size());
		m_CalibPolynomName.resize(m_AllChipNames.size());
	}

	while (CalibFile.getline(LineBuffer, LineLength, '\n')) {
		if (sscanf(LineBuffer, "CN %s", NameBuffer) == 1) {
			
			if (StartFromScratch == true) {
				m_CalibPolynomName.push_back(NameBuffer);
				m_CalibPolynom.resize(m_CalibPolynom.size()+1);
				CalibFile.getline(LineBuffer, LineLength, '\n');
				if (sscanf(LineBuffer, "PF %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf", 
									 &p0, &p1, &p2, &p3, &p4, &p5, &p6, &p7, &p8, &p9, &fmin, &fmax, &xmax) == 13) {
					cout<<"Found calibration info for chip: "<<NameBuffer<<endl;
					m_CalibPolynom.back().push_back(p0);
					m_CalibPolynom.back().push_back(p1);
					m_CalibPolynom.back().push_back(p2);
					m_CalibPolynom.back().push_back(p3);
					m_CalibPolynom.back().push_back(p4);
					m_CalibPolynom.back().push_back(p5);
					m_CalibPolynom.back().push_back(p6);
					m_CalibPolynom.back().push_back(p7);
					m_CalibPolynom.back().push_back(p8);
					m_CalibPolynom.back().push_back(p9);
					NLines++;
				}
			} else {
				// Check if such a chip exists in our chip list:
				for (unsigned int c = 0; c < m_AllChipNames.size(); ++c) {
					if (m_AllChipNames[c] == MString(NameBuffer)) {
					
						m_CalibPolynomName[c] = NameBuffer;
						CalibFile.getline(LineBuffer, LineLength, '\n');
						if (sscanf(LineBuffer, "PF %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf", 
											 &p0, &p1, &p2, &p3, &p4, &p5, &p6, &p7, &p8, &p9, &fmin, &fmax, &xmax) == 13) {
							cout<<"Found calibration info for chip: "<<NameBuffer<<endl;
							m_CalibPolynom[c].push_back(p0);
							m_CalibPolynom[c].push_back(p1);
							m_CalibPolynom[c].push_back(p2);
							m_CalibPolynom[c].push_back(p3);
							m_CalibPolynom[c].push_back(p4);
							m_CalibPolynom[c].push_back(p5);
							m_CalibPolynom[c].push_back(p6);
							m_CalibPolynom[c].push_back(p7);
							m_CalibPolynom[c].push_back(p8);
							m_CalibPolynom[c].push_back(p9);
							NLines++;
						} else {
							cout<<"Error reading PF-Tag!"<<endl;
						}
					} // Found name
				} // names-loop
			}
		}
	}

	CalibFile.close();

	if (NLines != m_AllChipNames.size() && StartFromScratch == false) {
		cout<<"Wrong number of lines in Fit file: "<<NLines<<" (!= "<<m_AllChipNames.size()<<")"<<endl;
		Error = true;
	}


	if (Error == false) {
		cout<<"Calibration successfully loaded!"<<endl;
	} else {
		cout<<"Errors occurred during load of calibration file!"<<endl;
		exit(1);
	}

	// Now invert the fit functions:
	m_InvCalibPolynom.resize(m_CalibPolynom.size());
	for (unsigned int i = 0; i < m_CalibPolynom.size(); ++i) {
		int NShapeBins = 300;
		double minTime = 0;
		double maxTime = 10000;
		double minInv = 0.0;
		double maxInv = 1.0;
		
		//TCanvas* Shape = new TCanvas("Shape", "Shape", 0, 0, 400, 300);
		TH1D* ShapeHist = new TH1D("ShapeHist", "ShapeHist", NShapeBins, minTime, maxTime);
		
		//TCanvas* InvShape = new TCanvas("InvShape", "InvShape", 410, 0, 400, 300);
		TH1D* InvShapeHist = new TH1D("InvShapeHist", "InvShapeHist", NShapeBins, minInv, maxInv); 

		// Determine initial shape and inverted shape:
		for (int b = 1; b <= NShapeBins; ++b) {
			ShapeHist->SetBinContent(b, EvaluateCalibration(i, ShapeHist->GetBinCenter(b)));
		}


		for (int b = 1; b <= NShapeBins; ++b) {
			InvShapeHist->SetBinContent((int) (ShapeHist->GetBinContent(b)*NShapeBins/(maxInv-minInv)), ShapeHist->GetBinCenter(b));
			if (ShapeHist->GetBinContent(b+1) < ShapeHist->GetBinContent(b)) break;
		}

		// Now interpolate over missing bins:
		int FirstEmptyBin = NShapeBins+1;
		int NMissing = 0;
		double LastContent = InvShapeHist->GetBinContent(NShapeBins);
		double Difference = 0.0;
		for (int b = NShapeBins; b >= 1; --b) {
			if (LastContent < 0.0001) {
				//cout<<"Bin: "<<b<<"Not started and empty!"<<endl;
				LastContent = InvShapeHist->GetBinContent(b);
				FirstEmptyBin = b;
				NMissing = 0;
				continue;
			} else {
				if (InvShapeHist->GetBinContent(b) < 0.0001){
					//cout<<"Bin: "<<b<<"Started and empty!"<<endl;
					NMissing++;
					continue;
				} else {
					//cout<<"Bin: "<<b<<"Started and full!"<<endl;
					// Start interpolation:
					Difference = (LastContent - InvShapeHist->GetBinContent(b))/(NMissing+1);
					for (int i = 1; i <= NMissing; ++i) {
						InvShapeHist->SetBinContent(b+i, InvShapeHist->GetBinContent(b)+i*Difference);
						//cout<<"Updating Bin: "<<b<<" with"<<InvShapeHist->GetBinContent(b)+i*Difference<<endl;
					}
					LastContent = InvShapeHist->GetBinContent(b);
					NMissing = 0;
					continue;
				}
			}
		}

		// Now determine the parameters of the inverted polynom:
		TF1* InvPoly = new TF1("InvPoly", "pol9");
		InvPoly->SetRange(0.0, InvShapeHist->GetBinCenter(FirstEmptyBin));
		InvShapeHist->Fit("InvPoly", "R0");

		// Finally store the data:
		for (int p = 0; p < 10 ; ++p) {
			m_InvCalibPolynom[i].push_back(InvPoly->GetParameter(p));
		}

// 		Shape->cd();
// 		ShapeHist->Draw();
// 		Shape->Update();
		
// 		InvShape->cd();
// 		InvShapeHist->Draw();
// 		InvShape->Update();
		
// 		gSystem->ProcessEvents();

// 		gSystem->Sleep(5000);

		//delete Shape;
		delete ShapeHist;
		//delete InvShape;
		delete InvShapeHist;
		delete InvPoly;
	}

	return true;
}

/******************************************************************************
 *
 */
void MPulseForms::ShowFits()
{

	LoadFitFunctions(true);

  cout<<"Size: "<<m_CalibPolynom.size()<<endl;

	// Now show the fit functions:
	for (unsigned int f = 0; f < m_CalibPolynom.size(); ++f) {
		TCanvas* Canvas = new TCanvas(m_CalibPolynomName[f], m_CalibPolynomName[f], 0, 0, 640, 480);
		TH1D* H1D = new TH1D(m_CalibPolynomName[f], m_CalibPolynomName[f], 100, 0, 10000);
		for (int x = 1; x <= 100; ++x) {
			H1D->SetBinContent(x, EvaluateCalibration(f, H1D->GetBinCenter(x)));
		}
		Canvas->cd();
		H1D->Draw();
		Canvas->Update();
	}

	// Determine the average pulse-shape...
	if (m_CalibPolynom.size() >= 1) {

		// Search the broadest shape:
		double Height = DBL_MIN;
		int Id = -1;
		for (unsigned int f = 0; f < m_CalibPolynom.size(); ++f) {
			if (EvaluateCalibration(f, 10000) > Height) {
				Height = EvaluateCalibration(f, 10000);
				Id = (int) f;
			} 
		}

		if (Id > -1) {
			cout<<"Broadest shape: Id="<<Id<<endl;
		} else {
			cout<<"No broadest shape found!"<<endl;
			return;
		}

		TCanvas* AvgPulseShape = new TCanvas("AvgPulseShape", "AvgPulseShape", 0, 500, 640, 480);
		TH1D* AvgPulseShapeHistogram = new TH1D("AvgPulseShapeHistogram", "AvgPulseShapeHistogram", 1000, 0, 12000);
		// Initialize with the first polynom...
		for (int x = 1; x <= 1000; ++x) {
			if (AvgPulseShapeHistogram->GetBinCenter(x) > 0 && AvgPulseShapeHistogram->GetBinCenter(x) < 12000) {
				AvgPulseShapeHistogram->SetBinContent(x, EvaluateCalibration(Id, AvgPulseShapeHistogram->GetBinCenter(x)));
			}
		}
		AvgPulseShape->cd();
		AvgPulseShapeHistogram->Draw();
		AvgPulseShape->Update();

		for (unsigned int f = 1; f < m_CalibPolynom.size(); ++f) {
			// fit the other polynoms...
			TF1* Fit = new TF1("AvgPolynomFit", AvgPolynomFit, 0, 8000, 11); 
			double par[11];
			for (int p = 0; p < 10; ++p) {
				par[p+1] = m_CalibPolynom[f][p];
			}
			Fit->SetParameters(par);
			for (int p = 1; p < 11; ++p) {
				Fit->FixParameter(p, par[p]);
			}
			Fit->SetParLimits(0, 0.5, 2.0);
			AvgPulseShapeHistogram->Fit("AvgPolynomFit", "R");

			// and add
			for (int x = 1; x <= 1000; ++x) {
				AvgPulseShapeHistogram->SetBinContent(x, (f*AvgPulseShapeHistogram->GetBinContent(x) + Fit->Eval(AvgPulseShapeHistogram->GetBinCenter(x)))/(f+1));
			}

			AvgPulseShape->cd();
			AvgPulseShapeHistogram->Draw();
			AvgPulseShape->Update();

			//gSystem->ProcessEvents();
			//gSystem->Sleep(2000);

			delete Fit;
		}

		// Now dump the average pulse form:
		delete m_AverageFit;
		m_AverageFit = new TF1("AverageFit", "pol9");
		m_AverageFit->SetLineColor(3);
		AvgPulseShapeHistogram->Fit("AverageFit", "R");
		for (int p = 0; p < m_AverageFit->GetNpar(); ++p) {
			cout<<"  m_AveragePulseForm.push_back("<<m_AverageFit->GetParameter(p)<<");"<<endl;
		}

		AvgPulseShape->cd();
		AvgPulseShapeHistogram->Draw();
		AvgPulseShape->Update();
	}
}


/******************************************************************************
 *
 */
double MPulseForms::EvaluateCalibration(int i, double Abszisse)
{
	double x = 1;
	double y = 0;
	for (unsigned int n = 0; n < m_CalibPolynom[i].size(); ++n) {
		//cout<<"Eval: "<<m_CalibPolynom[i][n]<<"!"<<x<<endl;
		y += m_CalibPolynom[i][n]*x;
		x *= Abszisse;
	}

	return y;
}



/******************************************************************************
 * Let's assume we held our signal m_TriggerHoldDelay ns after the the trigger
 * That means we do not have the correct maximum...
 * So try tol correct this...
 */
Bool_t MPulseForms::TestCalibration(Int_t i)
{

	double HistogramHoldTime = m_Files[m_CurrentFile]->GetTimeTillD2Trigger() - m_PulseStartOffset + m_TriggerHoldDelay;
	double RelMax = m_Maximum[i];
	double RelWrongMax = m_CorrectedPulseFormHistogram[i]->GetBinContent(m_CorrectedPulseFormHistogram[i]->FindBin(HistogramHoldTime));

	cout<<"RelMax. "<<RelMax<<"  RelWrongMax: "<<RelWrongMax<<endl;

	if (m_ShowMaximumWrongness == true) {
		m_MaximumWrongnessCanvas[i]->cd();
		m_MaximumWrongnessHistogram[i]->Fill(RelMax, RelWrongMax, 1);
		m_MaximumWrongnessHistogram[i]->Draw("COLZ");
		m_MaximumWrongnessCanvas[i]->Update();
	}

	if (m_ShowPulseForm == true) {
		for (int b = 1; b <= m_HoldFunction[i]->GetNbinsX(); ++b) {
			if (m_HoldFunction[i]->GetBinCenter(b) < HistogramHoldTime) {
				m_HoldFunction[i]->SetBinContent(b, 0.0);
			} else { 
				m_HoldFunction[i]->SetBinContent(b, 100000.0);
			}
		}
		m_PulseFormCanvas[i]->cd();
		m_HoldFunction[i]->Draw("SAME");
		m_PulseFormCanvas[i]->Update();
	}

	if (m_IsTestCalibrationMode == true) { 
		if (m_ShowMaximumCorrection == true) {
			m_MaximumCorrectionCanvas[i]->cd();
			cout<<RelMax<<"!"<<EvaluateCalibration(i, HistogramHoldTime)<<endl;
			m_MaximumCorrectionHistogram[i]->Fill(RelMax, 1.0/EvaluateCalibration(i, HistogramHoldTime)*RelWrongMax, 1);
			m_MaximumCorrectionHistogram[i]->Draw("COLZ");
			m_MaximumCorrectionCanvas[i]->Update();
		}
	}

	return true;
}


/******************************************************************************
 * Dito
 */
double MPulseForms::FindMaximumX(TF1* Function, double Min, double Max)
{
	// Simply scan the function ...
  
	double x = DBL_MIN;
	double Value = DBL_MIN;
  

	int NSteps = 200;
	double Step = (Max - Min)/NSteps;
	for (double p = Min; p <= Max; p += Step) {
		if (Function->Eval(p) > Value) {
			Value = Function->Eval(p);
			x = p;
		}
	}

	return x;
}


/******************************************************************************
 * Dito
 */
Bool_t MPulseForms::FindRealTriggerThreshold(Int_t PF)
{

	return true;
}


/******************************************************************************
 * Dito
 */
void MPulseForms::FinalCalculations()
{
	char Title[100];

	for (unsigned int i = 0; i < m_NPulseForms; ++i) {
		if (m_ShowMaximumCorrection == true) {
			m_MaximumCorrectionCanvas[i]->cd();
			m_MaximumCorrectionBisectorFunction[i]->Draw("CSAME");
			m_MaximumCorrectionCanvas[i]->Update();
		}
		if (m_ShowMaximumWrongness == true) {
			m_MaximumWrongnessCanvas[i]->cd();
			m_MaximumWrongnessBisectorFunction[i]->Draw("CSAME");
			m_MaximumWrongnessCanvas[i]->Update();
		}


		// All stuff which is done per chip
		int ChipId = m_ChipIdsPerFile[m_CurrentFile][i];

		// Add the average pulse-form density to according chip histogram...
		TH2D* H2D = 0;
		for (unsigned int f = 0; f < m_AllChipNames.size(); ++f) {
			if (m_AllChipNames[f] == m_ChipsPerFile[m_CurrentFile][i]) {
				H2D = m_AverageChipPulseFormDensityHistogram[f];
				break;
			}
		}
		if (H2D != 0) {
			for (int b = 1; b <= m_AveragedPulseFormHistogram[i]->GetNbinsX(); ++b) {
				H2D->Fill(m_AveragedPulseFormHistogram[i]->GetBinCenter(b), m_AveragedPulseFormHistogram[i]->GetBinContent(b), 1);
			}
			if (m_ShowAverageChipPulseFormDensity == true) {
				m_AverageChipPulseFormDensityCanvas[i]->cd();
				H2D->Draw("COLZ");
				m_AverageChipPulseFormDensityCanvas[i]->Update();
			}
		} else {
			cout<<"MPulseForms::FinalCalculations(): Fatal error: Histogram not found!"<<endl;
		}

		const int MinNAverages = 10;
		if (m_NAverages[i] > MinNAverages) {
			for (int b = 1; b <= m_AverageChipPulseFormHistogram[ChipId]->GetNbinsX(); ++b) {
				m_AverageChipPulseFormHistogram[ChipId]->
					SetBinContent(b, (m_AverageChipPulseFormNAverages[ChipId]*m_AverageChipPulseFormHistogram[ChipId]->GetBinContent(b) + 
														m_AveragedPulseFormHistogram[i]->GetBinContent(b))/(m_AverageChipPulseFormNAverages[ChipId]+1));
			}
			m_AverageChipPulseFormNAverages[ChipId]++;

			// Fit the average pulse form, compute the inverted pulse form and determine the real trigger threshold:
			double MinFit = 0.0;
			double MaxFit = 12000.0;
			m_AverageFit->SetRange(MinFit, MaxFit);
		 
			if (m_ShowAveragedPulseForm == true) {
				m_AveragedPulseFormCanvas[i]->cd();
				m_AveragedPulseFormHistogram[i]->Fit("AverageFit", "QR");
				m_AveragedPulseFormCanvas[i]->Update();
			} else {
				m_AveragedPulseFormHistogram[i]->Fit("AverageFit", "QRN");
			}

			// Do the inversion stuff:
			int NShapeBins = 1000;
			double minTime = 0;
			double maxTime = 10000;
			double minInv = 0.0;
			double maxInv = 1.0;
		
			//TCanvas* Shape = new TCanvas("Shape", "Shape", 0, 0, 400, 300);
			TH1D* ShapeHist = new TH1D("ShapeHist", "ShapeHist", NShapeBins, minTime, maxTime);
		
			//TCanvas* InvShape = new TCanvas("InvShape", "InvShape", 410, 0, 400, 300);
			TH1D* InvShapeHist = new TH1D("InvShapeHist", "InvShapeHist", NShapeBins, minInv, maxInv); 

			// Determine initial shape and inverted shape:
			for (int b = 1; b <= NShapeBins; ++b) {
				ShapeHist->SetBinContent(b, m_AverageFit->Eval(ShapeHist->GetBinCenter(b)));
			}

			for (int b = 1; b <= NShapeBins; ++b) {
				InvShapeHist->SetBinContent((int) (ShapeHist->GetBinContent(b)*NShapeBins/(maxInv-minInv)), ShapeHist->GetBinCenter(b));
				if (ShapeHist->GetBinContent(b+1) < ShapeHist->GetBinContent(b)) break;
			}

			// Now interpolate over missing bins:
			int FirstEmptyBin = NShapeBins+1;
			int NMissing = 0;
			double LastContent = InvShapeHist->GetBinContent(NShapeBins);
			double Difference = 0.0;
			for (int b = NShapeBins; b >= 1; --b) {
				if (LastContent < 0.0001) {
					//cout<<"Bin: "<<b<<"Not started and empty!"<<endl;
					LastContent = InvShapeHist->GetBinContent(b);
					FirstEmptyBin = b;
					NMissing = 0;
					continue;
				} else {
					if (InvShapeHist->GetBinContent(b) < 0.0001){
						//cout<<"Bin: "<<b<<"Started and empty!"<<endl;
						NMissing++;
						continue;
					} else {
						//cout<<"Bin: "<<b<<"Started and full!"<<endl;
						// Start interpolation:
						Difference = (LastContent - InvShapeHist->GetBinContent(b))/(NMissing+1);
						for (int i = 1; i <= NMissing; ++i) {
							InvShapeHist->SetBinContent(b+i, InvShapeHist->GetBinContent(b)+i*Difference);
							//cout<<"Updating Bin: "<<b<<" with"<<InvShapeHist->GetBinContent(b)+i*Difference<<endl;
						}
						LastContent = InvShapeHist->GetBinContent(b);
						NMissing = 0;
						continue;
					}
				}
			}

			// Now determine the parameters of the inverted polynom:
			TF1* InvPoly = new TF1("InvPoly", "pol9");
			InvPoly->SetRange(0.0, InvShapeHist->GetBinCenter(FirstEmptyBin));
			InvShapeHist->Fit("InvPoly", "RQ0");

			// Finally store the data:
			double* Parameter = new double[22];
			Parameter[0] = 20;
			Parameter[1] = 500;
			for (int p = 0; p < 10; ++p) {
				Parameter[p+2] = m_AverageFit->GetParameter(p);
				Parameter[p+12] = InvPoly->GetParameter(p);
			}
			//m_ComplicatedFit->SetParLimits(0, 0, 100);
			//m_ComplicatedFit->SetParLimits(1, 0, 1000);
			m_ComplicatedFit->SetParameters(Parameter);
			for (int p = 2; p < 22; ++p) {
				m_ComplicatedFit->FixParameter(p, Parameter[p]);
			}

			//for (int p = 0; p < 22; ++p) {
			//cout<<"p"<<p<<": "<<Parameter[p]<<" - "<<m_ComplicatedFit->GetParameter(p)<<endl;
			//}

			// Do the actual fitting:
			TProfile* prof = m_ThresholdOverSignalHistogram[i]->ProfileX();
			if (m_ShowThresholdOverSignal == true) {
				m_ThresholdOverSignalCanvas[i]->cd();
				prof->Draw("SAME");
				prof->Fit("ComplicatedFit", "RQ");
				m_ThresholdOverSignalCanvas[i]->Update();
			} else {
				prof->Fit("ComplicatedFit", "RQN");
			}

			cout<<"Results ComplicatedFit: thres="<<m_ComplicatedFit->GetParameter(0)
					<<" thres+baseline="<<m_ComplicatedFit->GetParameter(0) + m_Baseline[i]
					<<" delay="<<m_ComplicatedFit->GetParameter(1)<<endl;

			// Store the results:
			m_ChipRealThresholdHistogram[ChipId]->Fill(m_ComplicatedFit->GetParameter(0));
			m_AvgRealThreshold[ChipId] = m_ChipRealThresholdHistogram[ChipId]->GetMean();
			if (m_ShowChipRealThreshold == true) {
				m_ChipRealThresholdCanvas[i]->cd();
				m_ChipRealThresholdHistogram[ChipId]->Draw();
				m_ChipRealThresholdCanvas[i]->Update();
			}

			m_ChipRealThresholdPlusBaseHistogram[ChipId]->Fill(m_ComplicatedFit->GetParameter(0) + m_Baseline[i]);
			if (m_ShowChipRealThresholdPlusBase == true) {
				m_ChipRealThresholdPlusBaseCanvas[i]->cd();
				m_ChipRealThresholdPlusBaseHistogram[ChipId]->Draw();
				m_ChipRealThresholdPlusBaseCanvas[i]->Update();
			}

			m_ChipRealThresholdDelayHistogram[ChipId]->Fill(m_ComplicatedFit->GetParameter(1));
			m_AvgRealThresholdDelay[ChipId] = m_ChipRealThresholdDelayHistogram[ChipId]->GetMean();
			if (m_ShowChipRealThresholdDelay == true) {
				m_ChipRealThresholdDelayCanvas[i]->cd();
				m_ChipRealThresholdDelayHistogram[ChipId]->Draw();
				m_ChipRealThresholdDelayCanvas[i]->Update();
			}
		

			//delete Shape;
			delete ShapeHist;
			//delete InvShape;
			delete InvShapeHist;
			delete InvPoly;
			delete prof;
			delete [] Parameter;


		}

		// Generate an average chip rise-time function 
		delete m_ChipSignalOverRiseProfile[i];
		m_ChipSignalOverRiseProfile[i] = m_ChipSignalOverRiseHistogram[ChipId]->ProfileX();

		// Determine the maximum fit range:
		double Bins = 0;
		double BinsMax = 5;
		double RiseFitMax = 0;
		for (int b = m_ChipSignalOverRiseProfile[i]->GetNbinsX(); b >= 1; --b) {
			cout<<"Bin content: "<<m_ChipSignalOverRiseProfile[i]->GetBinContent(b)<<endl;
			Bins += m_ChipSignalOverRiseProfile[i]->GetBinContent(b);
			if (Bins > BinsMax) {
				RiseFitMax = m_ChipSignalOverRiseProfile[i]->GetBinCenter(b) - 150;
				if (RiseFitMax < 200) RiseFitMax = 200;
				break;
			}
		}

		if (m_ShowChipSignalOverRise == true) {
			m_ChipSignalOverRiseCanvas[i]->cd();
			m_ChipSignalOverRiseProfile[i]->Draw();
			m_SignalRiseFit->SetRange(m_Threshold, RiseFitMax);
			m_ChipSignalOverRiseProfile[i]->Fit(m_SignalRiseFit, "R");
			m_ChipSignalOverRiseCanvas[i]->Update();
		} else {
			m_ChipSignalOverRiseProfile[i]->Fit(m_SignalRiseFit, "RN");
		}
		m_MaximumRisePolynom[ChipId].resize(2);
		// make sure we have a function max over rise-time not vice versa!
		m_MaximumRisePolynom[ChipId][0] = -m_SignalRiseFit->GetParameter(0)/m_SignalRiseFit->GetParameter(1);
		m_MaximumRisePolynom[ChipId][1] = 1.0/m_SignalRiseFit->GetParameter(1);



// 			gSystem->ProcessEvents();
// 			gSystem->ProcessEvents();
// 			gSystem->ProcessEvents();
// 			gSystem->Sleep(5000);

		if (m_ShowAverageChipPulseForm == true) {
			m_AverageChipPulseFormCanvas[i]->cd();
			m_AverageChipPulseFormHistogram[ChipId]->Draw();
			m_AverageChipPulseFormCanvas[i]->Update();
		}



		// Store all results if wished
		if (m_StoreResults == true) {
			if (m_ShowPulseForm == true) {
				sprintf(Title, "PulseForm_%s_ch_%i.ps", m_ChipsPerFile[m_CurrentFile][i].Data(), m_ChannelPerFile[m_CurrentFile]);
				m_PulseFormCanvas[i]->Print(Title);
			}
			if (m_ShowCorrectedPulseForm == true) {
				sprintf(Title, "CorrectedPulseForm_%s_ch_%i.ps", m_ChipsPerFile[m_CurrentFile][i].Data(), m_ChannelPerFile[m_CurrentFile]);
				m_CorrectedPulseFormCanvas[i]->Print(Title);
			}
			if (m_ShowNormalizedPulseForm == true) {
				sprintf(Title, "NormalizedPulseForm_%s_ch_%i.ps", m_ChipsPerFile[m_CurrentFile][i].Data(), m_ChannelPerFile[m_CurrentFile]);
				m_NormalizedPulseFormCanvas[i]->Print(Title);
			}
			if (m_ShowNormalizedPulseFormDensity == true) {
				sprintf(Title, "NormalizedPulseFormDensity_%s_ch_%i.ps", m_ChipsPerFile[m_CurrentFile][i].Data(), m_ChannelPerFile[m_CurrentFile]);
				m_NormalizedPulseFormDensityCanvas[i]->Print(Title);
			}
			if (m_ShowAveragedPulseForm == true) {
				sprintf(Title, "AveragedPulseForm_%s_ch_%i.ps", m_ChipsPerFile[m_CurrentFile][i].Data(), m_ChannelPerFile[m_CurrentFile]);
				m_AveragedPulseFormCanvas[i]->Print(Title);
			}
			if (m_ShowAverageBaseline == true) {
				sprintf(Title, "AverageBaseline_%s_ch_%i.ps", m_ChipsPerFile[m_CurrentFile][i].Data(), m_ChannelPerFile[m_CurrentFile]);
				m_AverageBaselineCanvas[i]->Print(Title);
			}
			if (m_ShowBaselineDistribution == true) {
				sprintf(Title, "BaselineDistribution_%s_ch_%i.ps", m_ChipsPerFile[m_CurrentFile][i].Data(), m_ChannelPerFile[m_CurrentFile]);
				m_BaselineDistributionCanvas[i]->Print(Title);
			}
			if (m_ShowBaselineDensity == true) {
				sprintf(Title, "BaselineDensity_%s_ch_%i.ps", m_ChipsPerFile[m_CurrentFile][i].Data(), m_ChannelPerFile[m_CurrentFile]);
				m_BaselineDensityCanvas[i]->Print(Title);
			}
			if (m_ShowZeroCrossing == true) {
				sprintf(Title, "ZeroCrossing_%s_ch_%i.ps", m_ChipsPerFile[m_CurrentFile][i].Data(), m_ChannelPerFile[m_CurrentFile]);
				m_ZeroCrossingCanvas[i]->Print(Title);
			}
			if (m_ShowTriggerThreshold == true) {
				sprintf(Title, "TriggerThreshold_%s_ch_%i.ps", m_ChipsPerFile[m_CurrentFile][i].Data(), m_ChannelPerFile[m_CurrentFile]);
				m_TriggerThresholdCanvas[i]->Print(Title);
			}
			if (m_ShowRiseTimePerSignal == true) {
				sprintf(Title, "RiseTimePerSignal_%s_ch_%i.ps", m_ChipsPerFile[m_CurrentFile][i].Data(), m_ChannelPerFile[m_CurrentFile]);
				m_RiseTimePerSignalCanvas[i]->Print(Title);
			}
			if (m_ShowTriggerTimePerSignal == true) {
				sprintf(Title, "TriggerTimePerSignal_%s_ch_%i.ps", m_ChipsPerFile[m_CurrentFile][i].Data(), m_ChannelPerFile[m_CurrentFile]);
				m_TriggerTimePerSignalCanvas[i]->Print(Title);
			}
			if (m_ShowMaximumCorrection == true) {
				sprintf(Title, "MaximumCorrection_%s_ch_%i.ps", m_ChipsPerFile[m_CurrentFile][i].Data(), m_ChannelPerFile[m_CurrentFile]);
				m_MaximumCorrectionCanvas[i]->Print(Title);
			}
			if (m_ShowMaximumWrongness == true) {
				sprintf(Title, "MaximumWrongnes_%s_ch_%i.ps", m_ChipsPerFile[m_CurrentFile][i].Data(), m_ChannelPerFile[m_CurrentFile]);
				m_MaximumWrongnessCanvas[i]->Print(Title);
			}
		}
	}

	TCanvas* DrawCanvas = new TCanvas();
	DrawCanvas->Iconify();
	for (unsigned int c = 0; c < m_AllChipNames.size(); ++c) {
		if (m_ShowAverageChipPulseForm == true) {
			sprintf(Title, "AverageChipPulseForm_%s.ps", m_AllChipNames[c].Data());
			DrawCanvas->cd();
			m_AverageChipPulseFormHistogram[c]->Draw();
			DrawCanvas->Update();
			DrawCanvas->Print(Title);
		}
		if (m_ShowAverageChipPulseFormDensity == true) {
			sprintf(Title, "AverageChipPulseFormDensity_%s.ps", m_AllChipNames[c].Data());
			DrawCanvas->cd();
			m_AverageChipPulseFormDensityHistogram[c]->Draw("COLZ");
			DrawCanvas->Update();
			DrawCanvas->Print(Title);
		}
		if (m_ShowChipTriggerTimePerSignal == true) {
			sprintf(Title, "ChipTriggerTimePerSignal_%s.ps", m_AllChipNames[c].Data());
			DrawCanvas->cd();
			m_ChipTriggerTimePerSignalHistogram[c]->Draw("COLZ");
			DrawCanvas->Update();
			DrawCanvas->Print(Title);
		}
		if (m_ShowChipRiseTimePerSignal == true) {
			sprintf(Title, "ChipRiseTimePerSignal_%s.ps", m_AllChipNames[c].Data());
			DrawCanvas->cd();
			m_ChipRiseTimePerSignalHistogram[c]->Draw("COLZ");
			DrawCanvas->Update();
			DrawCanvas->Print(Title);
		}
		if (m_ShowChipSignalOverRise == true) {
			sprintf(Title, "ChipSignalOverRise_%s.ps", m_AllChipNames[c].Data());
			DrawCanvas->cd();
			m_ChipSignalOverRiseHistogram[c]->Draw("COLZ");
			DrawCanvas->Update();
			DrawCanvas->Print(Title);
		}
		if (m_ShowChipTriggerTime == true) {
			sprintf(Title, "ChipTriggerTime_%s.ps", m_AllChipNames[c].Data());
			DrawCanvas->cd();
			m_ChipTriggerTimeHistogram[c]->Draw();
			DrawCanvas->Update();
			DrawCanvas->Print(Title);
		}
		if (m_ShowChipRealThreshold == true) {
			sprintf(Title, "ChipRealThreshold_%s.ps", m_AllChipNames[c].Data());
			DrawCanvas->cd();
			m_ChipRealThresholdHistogram[c]->Draw();
			DrawCanvas->Update();
			DrawCanvas->Print(Title);
		}
		if (m_ShowChipRealThresholdPlusBase == true) {
			sprintf(Title, "ChipRealThresholdPlusBase_%s.ps", m_AllChipNames[c].Data());
			DrawCanvas->cd();
			m_ChipRealThresholdPlusBaseHistogram[c]->Draw();
			DrawCanvas->Update();
			DrawCanvas->Print(Title);
		}
		if (m_ShowChipRealThresholdDelay == true) {
			sprintf(Title, "ChipRealThresholdDelay_%s.ps", m_AllChipNames[c].Data());
			DrawCanvas->cd();
			m_ChipRealThresholdDelayHistogram[c]->Draw();
			DrawCanvas->Update();
			DrawCanvas->Print(Title);
		}
	}

	if (m_ShowMEGARiseTimePerSignal == true) {
		sprintf(Title, "MEGARiseTimePerSignal.ps");
		m_MEGARiseTimePerSignalCanvas->Print(Title);
	}

	if (m_ShowMEGATriggerTime == true) {
		sprintf(Title, "MEGATriggerTime.ps");
		m_MEGATriggerTimeCanvas->Print(Title);
	}


}


/******************************************************************************
 * Dito
 */
Double_t MPulseForms::FindZeroCrossing(TF1* Function, double Max, double Zero)
{
	// Simply scan the function ...
  
	double Min = -10000;
	double x = Max;
	bool IsPositive;

	int NSteps = 200;
	double Step = (Max - Min)/NSteps;
	IsPositive = (Function->Eval(Max) - Zero > 0) ? true : false;
	for (double p = Max; p > Min; p -= Step) {
		//cout<<"x= "<<p<<"  y= "<<Function->Eval(p)-Zero<<" ---- "<<(int) (Function->Eval(p) > Zero)<<"!"<<(int) IsPositive<<endl;
		// Falls Vorzeichenänderung:
		if ((Function->Eval(p) > Zero) ^ IsPositive) {
			x = p;
			break;
		}
	}

	NSteps = 20;
	Max = x+Step;
	Step = Step/NSteps;
	for (double p = Max; p > x; p -= Step) {
	  if ((Function->Eval(p) > Zero) ^ IsPositive) {
	    x = p+0.5*Step;
	    break;
	  }
	}

	return x;
}

/*
 * MPulseForms end...
 ******************************************************************************/

