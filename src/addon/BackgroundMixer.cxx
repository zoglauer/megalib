/* 
 * BackgroundMixer.cxx
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

// Standard
#include <iostream>
#include <string>
#include <limits>
#include <sstream>
#include <algorithm>
#include <csignal>
using namespace std;

// ROOT
#include <TROOT.h>
#include <TEnv.h>
#include <TSystem.h>
#include <TApplication.h>
#include <TStyle.h>
#include <TCanvas.h>
#include <THStack.h>
#include <TH1.h>
#include <TH2.h>
#include <TLegend.h>
#include <TLegendEntry.h>

// MEGAlib
#include "MGlobal.h"
#include "MAssert.h"
#include "MStreams.h"
#include "MEventSelector.h"
#include "MSettingsMimrec.h"
#include "MFileEventsTra.h"
#include "MPhysicalEvent.h"
#include "MComptonEvent.h"
#include "MFileEventsEvta.h"
#include "MRERawEvent.h"
#include "MRESE.h"
#include "MDGeometryQuest.h"
#include "MGeometryRevan.h"
#include "MDGrid.h"
#include "MDGridPoint.h"
#include "MDGridPointCollection.h"


/******************************************************************************/

class BackgroundMixer
{
public:
  /// Default constructor
  BackgroundMixer();
  /// Default destructor
  ~BackgroundMixer();
  
  /// Parse the command line
  bool ParseCommandLine(int argc, char** argv);
  /// Analyze what eveer needs to be analyzed...
  bool Analyze();
  /// Do the analysis if we have tra files
  bool AnalyzeTra();
  /// Do the analysis if we have sim files
  bool AnalyzeSim();
  /// Interrupt the analysis
  void Interrupt() { m_Interrupt = true; }
  /// True if we use the Gui
  bool UseGui() { return m_UseGui; }

private:
  /// True, if the analysis needs to be interrupted
  bool m_Interrupt;

  //! The basic mode
  int m_Mode;

  static const int c_UnknownMode = 0;
  static const int c_SimMode     = 1;
  static const int c_TraMode     = 2;


  vector<MString> m_BackgroundFileNames;
  vector<MString> m_BackgroundNames;
  vector<double>  m_BackgroundTimes;
  vector<int>     m_BackgroundEvents;
  vector<TH1D*>   m_BackgroundHistograms;

  vector<MString> m_SourceFileNames;
  vector<MString> m_SourceNames;
  vector<double>  m_SourceTimes;
  vector<int>     m_SourceEvents;
  vector<TH1D*>   m_SourceHistograms;

  MString m_ConfigurationFileName;

  //! Prefix for the output files:
  MString m_Prefix;
  
  //! Name of the geometry file
  MString m_GeometryFileName;

  //! Maximum time
  double m_MaximumTime;

  //! Do the extraction
  bool m_Extract;
  MString m_ExtractionFileName;

  int m_NBins;
  double m_Min;
  double m_Max;
  double* m_Bins;

  //! true if we should use a GUI
  bool m_UseGui;
};

/******************************************************************************/

BackgroundMixer* g_Prg = 0;
int g_NInterruptCatches = 1;

/******************************************************************************/


/******************************************************************************
 * Default constructor
 */
BackgroundMixer::BackgroundMixer() : m_Interrupt(false)
{
  m_Mode = c_UnknownMode;
  m_GeometryFileName = g_StringNotDefined;
  m_ConfigurationFileName = g_StringNotDefined;
  m_MaximumTime = g_DoubleNotDefined;
  MTime Time;
  m_Prefix = MString("Prefix_") + Time.GetShortString();

  m_Extract = false;
  m_ExtractionFileName = "Extracted";

  m_UseGui = true;

  gStyle->SetPalette(1, 0);
}


/******************************************************************************
 * Default destructor
 */
BackgroundMixer::~BackgroundMixer()
{
  // Intentionally left blank
}


/******************************************************************************
 * Parse the command line
 */
bool BackgroundMixer::ParseCommandLine(int argc, char** argv)
{
  mimp<<"Maxtime is not idiot safe implemented"<<show;


  ostringstream Usage;
  Usage<<endl;
  Usage<<"  Usage: BackgroundMixer <options>"<<endl;
  Usage<<"    General options:"<<endl;
  Usage<<"          -s <file> <time>:            Source file name and total measurement time"<<endl;
  Usage<<"          -b <file> <time>:            Background file name and total measurement time"<<endl;
  Usage<<"          -se <file> <time> <events>:  Source file name and total measurement time as well as TOTAL number of events on file"<<endl;
  Usage<<"          -be <file> <time> <events>:  Background file name and total measurement time as well as TOTAL number of events on file"<<endl;
  Usage<<"          -t <time>:                   Stop after this time (needs to be lower/equal than the individual times)"<<endl;
  Usage<<"          -g <file>:                   Geometry file name"<<endl;
  Usage<<"          -w <bins> <min> <max>:       The histogram"<<endl;
  Usage<<"          -c <file>:                   Mimrec configuration file"<<endl;
  Usage<<"          -n <name>:                   Prefix for the output files"<<endl;
  Usage<<"          -e <file>:                   Extract and combine tra files into this file"<<endl;
  Usage<<"          --no-gui:                    Don't show windows"<<endl;
  // Usage<<"         -f:   file name"<<endl;
  Usage<<"          -h:   print this help"<<endl;
  Usage<<endl;

  string Option;

  // Check for help
  for (int i = 1; i < argc; i++) {
    Option = argv[i];
    if (Option == "-h" || Option == "--help" || Option == "?" || Option == "-?") {
      mlog<<Usage.str()<<endl;
      return false;
    }
  }

  // Now parse the master command line options:
  for (int i = 1; i < argc; i++) {
    Option = argv[i];
   
    if (Option == "-w") {
      m_NBins = atoi(argv[++i]);
      m_Min = atof(argv[++i]);
      m_Max = atof(argv[++i]);
    } else if (Option == "-n") {
      m_Prefix = argv[++i];
      mlog.Connect(m_Prefix + ".log");
      mlog<<"Accepting output file name: "<<m_Prefix<<endl;
    }
  }

  bool IsLog = false;
  double Min = m_Min;
  double Max = m_Max;
  int NBins = m_NBins;
  if (IsLog == true) {
    if (Min <= 0) Min = 1;
  }
  if (NBins < 1) {
    merr<<"Number of bins ("<<NBins<<") must be >= 1! Using 1!"<<show;
    NBins = 2;
  }
  if (Min >= Max) {
    merr<<"Minimum ("<<Min<<") must be smaller than maximum ("<<Max<<")! Using Max = Min + 1.0!"<<show;
    Max = Min+1;
  }

  m_Bins = new double[NBins+1];
  double Dist = 0.0;

  if (IsLog == true) {
    Min = log(Min);
    Max = log(Max);
    Dist = (Max-Min)/(NBins);
    for (int i = 0; i < NBins+1; ++i) {
      m_Bins[i] = exp(Min+i*Dist);
    }  
  } else {
    Dist = (Max-Min)/(NBins);
    for (int i = 0; i < NBins+1; ++i) {
      m_Bins[i] = Min+i*Dist;
    }  
  }


  // Now parse the command line options:
  for (int i = 1; i < argc; i++) {
    Option = argv[i];

    mimp<<"Argument length verification is missing"<<show;
    // First check if each option has sufficient arguments:
    // Single argument
    if (Option == "-f") {
      if (!((argc > i+1) && 
            (argv[i+1][0] != '-' || isalpha(argv[i+1][1]) == 0))){
        mlog<<"Error: Option "<<argv[i][1]<<" needs a second argument!"<<endl;
        mlog<<Usage.str()<<endl;
        return false;
      }
    } 
    // Multiple arguments template
    /*
    else if (Option == "-??") {
      if (!((argc > i+2) && 
            (argv[i+1][0] != '-' || isalpha(argv[i+1][1]) == 0) && 
            (argv[i+2][0] != '-' || isalpha(argv[i+2][1]) == 0))){
        mlog<<"Error: Option "<<argv[i][1]<<" needs two arguments!"<<endl;
        mlog<<Usage.str()<<endl;
        return false;
      }
    }
    */

    // Then fulfill the options:
    if (Option == "-g") {
      m_GeometryFileName = argv[++i];
      if (MFile::Exists(m_GeometryFileName) == false) {
        mlog<<"-g: Cannot find file: "<<m_GeometryFileName<<endl;
        mlog<<Usage.str()<<endl;
        return false;       
      }
      mlog<<"Accepting file name: "<<m_GeometryFileName<<endl;
    } else if (Option == "-c") {
      m_ConfigurationFileName = argv[++i];
      if (MFile::Exists(m_ConfigurationFileName) == false) {
        mlog<<"-c: Cannot find file: "<<m_ConfigurationFileName<<endl;
        mlog<<Usage.str()<<endl;
        return false;       
      }
      mlog<<"Accepting configuration file name: "<<m_ConfigurationFileName<<endl;
    } else if (Option == "-e") {
      m_Extract = true;
      m_ExtractionFileName = argv[++i];
      mlog<<"Accepting extraction file name: "<<m_ExtractionFileName<<endl;
    } else if (Option == "-t") {
      m_MaximumTime = atof(argv[++i]);
      mlog<<"Accepting maximum time: "<<m_MaximumTime<<endl;
    } else if (Option == "-s" || Option == "-se") {
      m_SourceFileNames.push_back(argv[++i]);
      MString Name = m_SourceFileNames.back();
      if (MFile::Exists(Name) == false) {
        mlog<<"-s/-se: Cannot find file: "<<Name<<endl;
        mlog<<Usage.str()<<endl;
        return false;       
      }
      if (Name.EndsWith(".sim") == true) {
        if (m_Mode == c_SimMode) {
          // Everything fine
        } else if (m_Mode == c_TraMode) {
          mlog<<"Error: You are in tra mode and add a sim file!"<<endl;
          mlog<<Usage.str()<<endl;
          return false;          
        } else {
          m_Mode = c_SimMode;
        }
      }
      if (Name.EndsWith(".tra") == true) {
        if (m_Mode == c_TraMode) {
          // Everything fine
        } else if (m_Mode == c_SimMode) {
          mlog<<"Error: You are in sim mode and add a tra file!"<<endl;
          mlog<<Usage.str()<<endl;
          return false;          
        } else {
          m_Mode = c_TraMode;
        }
      }
      Name.ReplaceAll(".sim", "");
      Name.ReplaceAll(".tra", "");
      Name.ReplaceAll(".", "_");
      Name.Remove(0, Name.Last('/')+1);
      m_SourceNames.push_back(Name);
      m_SourceTimes.push_back(atof(argv[++i]));
      if (Option  == "-se") {
        m_SourceEvents.push_back(atoi(argv[++i]));
      } else {
        m_SourceEvents.push_back(g_IntNotDefined);
      }
      m_SourceHistograms.push_back(new TH1D(m_SourceNames.back(), m_SourceNames.back(), m_NBins, m_Bins));
      if (m_SourceTimes.back() <= 0) {
        mlog<<"Error: You need to give positive times..."<<endl;
        mlog<<Usage.str()<<endl;
        return false;
      }
      mlog<<"Accepting background file name and time: "
          <<m_SourceFileNames.back()<<" "<<m_SourceTimes.back()<<"!"<<endl;
    } else if (Option == "-b" || Option == "-be") {
      m_BackgroundFileNames.push_back(argv[++i]);
      MString Name = m_BackgroundFileNames.back();
      if (MFile::Exists(Name) == false) {
        mlog<<"-b/-be: Cannot find file: "<<Name<<endl;
        mlog<<Usage.str()<<endl;
        return false;       
      }
      if (Name.EndsWith(".sim") == true) {
        if (m_Mode == c_SimMode) {
          // Everything fine
        } else if (m_Mode == c_TraMode) {
          mlog<<"Error: You are in tra mode and add a sim file!"<<endl;
          mlog<<Usage.str()<<endl;
          return false;          
        } else {
          m_Mode = c_SimMode;
        }
      }
      if (Name.EndsWith(".tra") == true) {
        if (m_Mode == c_TraMode) {
          // Everything fine
        } else if (m_Mode == c_SimMode) {
          mlog<<"Error: You are in sim mode and add a tra file!"<<endl;
          mlog<<Usage.str()<<endl;
          return false;          
        } else {
          m_Mode = c_TraMode;
        }
      }
      Name.ReplaceAll(".sim", "");
      Name.ReplaceAll(".tra", "");
      Name.ReplaceAll(".", "_");
      Name.Remove(0, Name.Last('/')+1);
      m_BackgroundNames.push_back(Name);
      m_BackgroundTimes.push_back(atof(argv[++i]));
      if (Option  == "-be") {
        m_BackgroundEvents.push_back(atoi(argv[++i]));
      } else {
        m_BackgroundEvents.push_back(g_IntNotDefined);
      }
      m_BackgroundHistograms.push_back(new TH1D(m_BackgroundNames.back(), m_BackgroundNames.back(), m_NBins, m_Bins));
      if (m_BackgroundTimes.back() <= 0) {
        mlog<<"Error: You need to give positive times..."<<endl;
        mlog<<Usage.str()<<endl;
        return false;
      }
      mlog<<"Accepting background file name and time: "
          <<m_BackgroundFileNames.back()<<" "<<m_BackgroundTimes.back()<<"!"<<endl;
    } else if (Option == "-w") {
      i += 3;
    } else if (Option == "-n") {
      i += 1;
    } else if (Option == "--no-gui") {
      m_UseGui = false;
      gROOT->SetBatch(true);
    } else {
      mlog<<"Error: Unknown option \""<<Option<<"\"!"<<endl;
      mlog<<Usage.str()<<endl;
      return false;
    }
  }

  if (m_MaximumTime != g_DoubleNotDefined) {
    for (unsigned int i = 0; i < m_BackgroundTimes.size(); ++i) {
      if (m_BackgroundTimes[i] < m_MaximumTime) {
        mlog<<"Error: Maximum time is not the smallest one..."<<endl;
        mlog<<Usage.str()<<endl;
        return false;
      }
    }
    for (unsigned int i = 0; i < m_SourceTimes.size(); ++i) {
      if (m_SourceTimes[i] < m_MaximumTime) {
        mlog<<"Error: Maximum time is not the smallest one..."<<endl;
        mlog<<Usage.str()<<endl;
        return false;
      }
    }
  }

  if (m_BackgroundNames.size() == 0 && m_SourceNames.size() == 0) {
    mlog<<"Error: You need at least one data file..."<<endl;
    mlog<<Usage.str()<<endl;
    return false;
  }

  if (m_GeometryFileName == g_StringNotDefined) {
    mlog<<"Error: You need to give a geometry..."<<endl;
    mlog<<Usage.str()<<endl;
    return false;    
  }

  if (m_Mode == c_TraMode && m_ConfigurationFileName == g_StringNotDefined) {
    mlog<<"Error: You need to give a configuartion file when you are using tra files..."<<endl;
    mlog<<Usage.str()<<endl;
    return false;    
  }

  return true;
}


/******************************************************************************
 * Do whatever analysis is necessary
 */
bool BackgroundMixer::Analyze()
{
  if (m_Interrupt == true) return false;

  if (m_Mode == c_TraMode) {
    return AnalyzeTra();
  } else {
    return AnalyzeSim();
  }
}


/******************************************************************************
 * Sort grid by decreasing hit count
 */
bool SortGridByDereasingHitCount(MDGridPointCollection G1, MDGridPointCollection G2) {
  return (G1.GetHits() > G2.GetHits());
}

/******************************************************************************
 * Do the tracked events analysis
 */
bool BackgroundMixer::AnalyzeSim()
{
  g_NInterruptCatches = m_BackgroundFileNames.size() + m_SourceFileNames.size() + 1;

  // Load geometry:
  MGeometryRevan* Geometry = new MGeometryRevan();
  if (Geometry->ScanSetupFile(m_GeometryFileName) == true) {
    mlog<<"Geometry "<<Geometry->GetName()<<" loaded!"<<endl;
  } else {
    mlog<<"Loading of geometry "<<Geometry->GetName()<<" failed!!"<<endl;
    return false;
  }

  // The master histogram
  TH1D* TotalTotal = new TH1D("TotalTotal", "TotalTotal", m_NBins, m_Bins);
  TotalTotal->SetXTitle("Energy [keV]");
  TotalTotal->SetYTitle("cts/keV/s");
  TotalTotal->SetLineColor(2);

  TH1D* SourceTotal = new TH1D("SourceTotal", "SourceTotal", m_NBins, m_Bins);
  SourceTotal->SetXTitle("Energy [keV]");
  SourceTotal->SetYTitle("cts/keV/s");
  SourceTotal->SetLineColor(3);

  TH1D* BackgroundTotal = new TH1D("BackgroundTotal", "BackgroundTotal", m_NBins, m_Bins);
  BackgroundTotal->SetXTitle("Energy [keV]");
  BackgroundTotal->SetYTitle("cts/keV/s");
  BackgroundTotal->SetLineColor(4);

  // The source file
  for (unsigned int bf = 0; bf < m_SourceFileNames.size(); ++bf) {

    // Open the file
    MFileEventsEvta Bkg(Geometry);
    if (Bkg.Open(m_SourceFileNames[bf]) == false) {
      mlog<<"Unable to open file "<<m_SourceFileNames[bf]<<endl;
      return false;
    }
    mlog<<"Analyzing file: "<<m_SourceFileNames[bf]<<endl;

    if (m_MaximumTime != g_DoubleNotDefined) {
      if (m_SourceEvents[bf] == g_IntNotDefined) {
      // Get the number of events we have in the file:
        m_SourceEvents[bf] = Bkg.GetNEvents(true);
        mlog<<"Total number of events on file: "<<m_SourceEvents[bf]<<endl;
        Bkg.Rewind();
      }

      // Determine the number of events we want to read:
      m_SourceEvents[bf] = int(m_MaximumTime*m_SourceEvents[bf]/m_SourceTimes[bf]);
    }

    // Main loop
    int counts = 0;
    MRERawEvent* Event = 0;
    while ((Event = Bkg.GetNextEvent()) != 0) {
      if (++counts % 10000 == 0) mlog<<"Counts: "<<counts<<endl;
      m_SourceHistograms[bf]->Fill(Event->GetEnergy());
      delete Event;
      if (m_Interrupt == true) {
        m_Interrupt = false;
        break;
      }
      if (m_MaximumTime != g_DoubleNotDefined && counts >= m_SourceEvents[bf]) {
        mlog<<"Stopping after "<<counts<<" counts..."<<endl;
        break;
      }
    }

    // Normalize histogram
    m_SourceHistograms[bf]->SetXTitle("Energy [keV]");
    m_SourceHistograms[bf]->SetYTitle("cts/keV/s");
    if (m_MaximumTime != g_DoubleNotDefined) {
      m_SourceHistograms[bf]->Scale(1.0/m_MaximumTime);
    } else {
      m_SourceHistograms[bf]->Scale(1.0/m_SourceTimes[bf]);
    }
    for (int b = 1; b <= m_SourceHistograms[bf]->GetXaxis()->GetNbins(); ++b) {
      m_SourceHistograms[bf]->SetBinContent(b, m_SourceHistograms[bf]->GetBinContent(b)/m_SourceHistograms[bf]->GetBinWidth(b));
    }

    // Add to total histograms
    TotalTotal->Add(m_SourceHistograms[bf]);
    SourceTotal->Add(m_SourceHistograms[bf]);

    // Display this component:
    TCanvas* Canvas = new TCanvas(m_SourceNames[bf] + "Canvas", m_SourceNames[bf] + " - Canvas");
    Canvas->cd();
    m_SourceHistograms[bf]->Draw();
    Canvas->Update();
  }

  // For storing the BACKGROUD hits in grids:
  vector<vector<MDGridPointCollection> > Grids(m_BackgroundFileNames.size());
  vector<vector<TH1D*> > Hists(m_BackgroundFileNames.size());
  const unsigned int NoGrid = numeric_limits<unsigned int>::max();



  // The background file
  for (unsigned int bf = 0; bf < m_BackgroundFileNames.size(); ++bf) {

    // Open the file
    MFileEventsEvta Bkg(Geometry);
    if (Bkg.Open(m_BackgroundFileNames[bf]) == false) {
      mlog<<"Unable to open file "<<m_BackgroundFileNames[bf]<<endl;
      return false;
    }
    mlog<<"Analyzing file: "<<m_BackgroundFileNames[bf]<<endl;

    if (m_MaximumTime != g_DoubleNotDefined) {
      if (m_BackgroundEvents[bf] == g_IntNotDefined) {
        // Get the number of events we have in the file:
        m_BackgroundEvents[bf] = Bkg.GetNEvents(true);
        mlog<<"Counts: "<<m_BackgroundEvents[bf]<<endl;
        Bkg.Rewind();
      }

      // Determine the number of events we want to read:
      m_BackgroundEvents[bf] = int(m_MaximumTime*m_BackgroundEvents[bf]/m_BackgroundTimes[bf]);
    }

    // Main loop
    int counts = 0;
    MRERawEvent* Event = 0;
    while ((Event = Bkg.GetNextEvent()) != 0) {
      if (++counts % 10000 == 0) mlog<<"Counts: "<<counts<<endl;
      // Add it to the summary histograms:
      m_BackgroundHistograms[bf]->Fill(Event->GetEnergy());

      // Add it to the grids:

      // The raw events are still unanalyzed --- so we can assume is has no hits
      for (int h = 0; h < Event->GetNRESEs(); ++h) {
        MRESE* Hit = Event->GetRESEAt(h);
        if (Hit->GetType() != MRESE::c_Hit) {
          merr<<"All elements of this RESE must be hits!"<<show;
          massert(Hit->GetType() == MRESE::c_Hit);
          return false;
        }

        // Check if we have a suitable grid:
        unsigned int CorrectGrid = NoGrid;
        for (unsigned g = 0; g < Grids[bf].size(); ++g) {
          if (Grids[bf][g].HasSameDetector(Hit->GetVolumeSequence()) == true) {
            CorrectGrid = g;
            break;
          }
        }
        if (CorrectGrid == NoGrid) {
          // Create a new Grid
          MDGridPointCollection Grid(*(Hit->GetVolumeSequence())); 
          Grids[bf].push_back(Grid);
          CorrectGrid = Grids[bf].size()-1;
	  // new hist:
	  TString Name = "Bkg_";
          Name += bf;
          Name += "_";
          Name += CorrectGrid;
  	  TH1D* BackgroundH = new TH1D(Name, Name, m_NBins, m_Bins);
          BackgroundH->SetXTitle("Energy [keV]");
          BackgroundH->SetYTitle("cts/keV/s");
          BackgroundH->SetLineColor(4);
          Hists[bf].push_back(BackgroundH);
        } 
        // Add hit to Grid - Grid does correct discretization
        Grids[bf][CorrectGrid].AddUndiscretized(Hit->GetVolumeSequence()->GetPositionInDetector());
        Hists[bf][CorrectGrid]->Fill(Hit->GetEnergy());
      }

      delete Event;
      if (m_Interrupt == true) {
        m_Interrupt = false;
        break;
      }
      if (m_MaximumTime != g_DoubleNotDefined && counts >= m_BackgroundEvents[bf]) {
        mlog<<"Stopping after "<<counts<<" counts..."<<endl;
        break;
      }
    }

    // Normalize histogram
    m_BackgroundHistograms[bf]->SetXTitle("Energy [keV]");
    m_BackgroundHistograms[bf]->SetYTitle("cts/s");
    if (m_MaximumTime != g_DoubleNotDefined) {
      m_BackgroundHistograms[bf]->Scale(1.0/m_MaximumTime);
    } else {
      m_BackgroundHistograms[bf]->Scale(1.0/m_BackgroundTimes[bf]);
    }
    /*
    for (int b = 1; b <= m_BackgroundHistograms[bf]->GetXaxis()->GetNbins(); ++b) {
      m_BackgroundHistograms[bf]->SetBinContent(b, m_BackgroundHistograms[bf]->GetBinContent(b)/m_BackgroundHistograms[bf]->GetBinWidth(b));
    }
    */
    // Add to total histograms
    TotalTotal->Add(m_BackgroundHistograms[bf]);
    BackgroundTotal->Add(m_BackgroundHistograms[bf]);

    // Display this component:
    TCanvas* Canvas = new TCanvas(m_BackgroundNames[bf] + "Canvas", m_BackgroundNames[bf] + " - Canvas");
    Canvas->cd();
    m_BackgroundHistograms[bf]->Draw();
    Canvas->Update();
  }


  // Draw a stacked histogram of *all* components
  TCanvas* TotalStackCanvas = new TCanvas();
  TotalStackCanvas->cd();

  TLegend* leg = new TLegend(0.4, 0.6, 0.85, 0.85, NULL, "brNDC");

  // Combine all histograms into one:
  int Color = 2;
  THStack* TotalStack = new THStack("Components", "Components");
  for (unsigned int bf = 0; bf < m_SourceFileNames.size(); ++bf) {
    m_SourceHistograms[bf]->SetLineColor(Color);
    m_SourceHistograms[bf]->SetFillColor(Color);
    TotalStack->Add(m_SourceHistograms[bf]);
    TLegendEntry* entry = leg->AddEntry("NULL", m_SourceNames[bf], "l");
    entry->SetLineColor(Color);
    entry->SetTextColor(Color);
    Color++;
    if (Color == 5) Color++;
    if (Color == 10) Color++;
  }
  for (unsigned int bf = 0; bf < m_BackgroundFileNames.size(); ++bf) {
    m_BackgroundHistograms[bf]->SetLineColor(Color);
    m_BackgroundHistograms[bf]->SetFillColor(Color);
    TotalStack->Add(m_BackgroundHistograms[bf]);
    TLegendEntry* entry = leg->AddEntry("NULL", m_BackgroundNames[bf], "l");
    entry->SetLineColor(Color);
    entry->SetTextColor(Color);
    Color++;
    if (Color == 5) Color++;
    if (Color == 10) Color++;
  }
  TotalStack->Draw();
  TotalStack->GetHistogram()->SetXTitle("Energy [keV]");
  TotalStack->GetHistogram()->SetYTitle("cts/s");
  TotalStack->Draw();
  leg->Draw();
  TotalStackCanvas->Update();


  // Draw the total Canvas
  TCanvas* TotalTotalCanvas = new TCanvas();
  TotalTotalCanvas->cd();
  TotalTotal->Draw();
  TotalTotalCanvas->Update();

  // Draw the total Canvas
  TCanvas* TotalTotalComponentsCanvas = new TCanvas();
  TotalTotalComponentsCanvas->cd();
  TotalTotal->Draw();
  SourceTotal->Draw("SAME");
  BackgroundTotal->Draw("SAME");
  TotalTotalComponentsCanvas->Update();

  if (m_SourceFileNames.size() > 0 && m_BackgroundFileNames.size() > 0) {
    TH1D* SB = new TH1D("SB", "Signal to background ratio", m_NBins, m_Bins);
    SB->SetXTitle("Energy [keV]");
    SB->SetYTitle("S/B");
    SB->SetLineColor(2);
    for (int b = 1; b < SB->GetNbinsX(); ++b) {
      if (BackgroundTotal->GetBinContent(b) > 0) {
        SB->SetBinContent(b, SourceTotal->GetBinContent(b)/BackgroundTotal->GetBinContent(b));
      }
    }
    TCanvas* SBCanvas = new TCanvas();
    SBCanvas->cd();
    SB->Draw();
    SBCanvas->Update();
  }

  // Determine total flux:
  double Int = TotalTotal->Integral()*TotalTotal->GetBinWidth(1);
  mlog<<"Total flux in histogram: "<<Int<<"cts/sec"<<endl;

  // Get the base file name of the tra file:
  MString AsciiOut = m_Prefix + "TotalTotal" + ".ASCIIspectrum.dat";
  
  ofstream out(AsciiOut, ios::out);
  
  for (int b = 1; b <= TotalTotal->GetNbinsX(); ++b) {
    out<<TotalTotal->GetBinCenter(b)<<" \t";
    out<<TotalTotal->GetBinContent(b)<<endl;
  }
  out.close();
  
  mlog<<"Wrote ASCII spectrum to "<<AsciiOut<<endl;

  // Now dump some GRID specific information:

  // Normalize the GRIDs to cts/sec:
  for (unsigned int bf = 0; bf < m_BackgroundFileNames.size(); ++bf) {
    for (unsigned int g = 0; g < Grids[bf].size(); ++g) {
      if (m_MaximumTime != g_DoubleNotDefined) {
        Grids[bf][g].SetWeight(1.0/m_MaximumTime);
        Hists[bf][g]->Scale(1.0/m_MaximumTime);
      } else {
        Grids[bf][g].SetWeight(1.0/m_BackgroundTimes[bf]);
        Hists[bf][g]->Scale(1.0/m_BackgroundTimes[bf]);
      }
    }
  }

  // Combined the individual GRIDs:
  vector<MDGridPointCollection> CombinedGrids;
  vector<TH1D*> CombinedHists;
  for (unsigned int bf = 0; bf < m_BackgroundFileNames.size(); ++bf) {
    for (unsigned int g = 0; g < Grids[bf].size(); ++g) {
      // Check if we have a suitable grid:
      unsigned int CorrectGrid = NoGrid;
      for (unsigned cg = 0; cg < CombinedGrids.size(); ++cg) {
        if (CombinedGrids[cg].HasSameDetector(Grids[bf][g]) == true) {
          CorrectGrid = cg;
          break;
        }
      }
      if (CorrectGrid == NoGrid) {
        CombinedGrids.push_back(Grids[bf][g]);
        CombinedHists.push_back(Hists[bf][g]);
      } else {
        CombinedGrids[CorrectGrid].Add(Grids[bf][g]);
        CombinedHists[CorrectGrid]->Add(Hists[bf][g]);
      }
    }
  }  

  /*
  for (unsigned int h = 0; h < CombinedHists.size(); ++h) { 
    // Get the base file name of the tra file:
    MString AsciiOut = m_Prefix + "_" + CombinedHists[h]->GetName() + ".ASCIIspectrum.dat";
  
    double SanityCheck = 0.0;

    ofstream out(AsciiOut, ios::out);
    out<<"# VS: "<<CombinedGrids[h].GetVolumeTree()<<endl;
    for (int b = 1; b <= CombinedHists[h]->GetNbinsX(); ++b) {
      out<<CombinedHists[h]->GetBinCenter(b)<<" \t";
      out<<CombinedHists[h]->GetBinContent(b)<<endl;
      SanityCheck += CombinedHists[h]->GetBinContent(b);
    }
    out.close();
  
    mlog<<"Wrote ASCII spectrum to "<<AsciiOut<<" with a total flux of "<<SanityCheck<<" cts/sec of volume "<<CombinedGrids[h].GetVolumeTree()<<endl;
  }
  */

  // Sort the grids by their number of hits
  // Sort by the number of counts
  mout<<"Started sorting grids... this might take some time..."<<endl;
  sort(CombinedGrids.begin(), CombinedGrids.end(), SortGridByDereasingHitCount);

  mlog<<"The 20 detector with the most hits (one event can have multiple hits per detector) are: "<<endl;
  unsigned int TotalHits = 0;
  for (unsigned int g = 0; g < CombinedGrids.size(); ++g) {
    TotalHits += CombinedGrids[g].GetHits();
    if (g < 20) {
      mlog<<CombinedGrids[g].GetVolumeTree()<<": ";
      mlog<<CombinedGrids[g].GetWeightedHits()<<" hits/sec"<<endl;
    }
  }
  mlog<<"Total hits: "<<TotalHits<<endl;


  //vector<MDGridPointCollection> CombinedGrids;

  return true;
}


/******************************************************************************
 * Do the tracked events analysis
 */
bool BackgroundMixer::AnalyzeTra()
{
  // Sensitivity:
  g_NInterruptCatches = m_BackgroundFileNames.size() + 1;

  // The master histogram
  TH1D* TotalTotal = new TH1D("TotalTotal", "TotalTotal", m_NBins, m_Bins);
  TotalTotal->SetXTitle("Energy [keV]");
  TotalTotal->SetYTitle("cts/keV/s");
  TotalTotal->SetLineColor(2);

  TH1D* SourceTotal = new TH1D("SourceTotal", "SourceTotal", m_NBins, m_Bins);
  SourceTotal->SetXTitle("Energy [keV]");
  SourceTotal->SetYTitle("cts/keV/s");
  SourceTotal->SetLineColor(3);

  TH1D* BackgroundTotal = new TH1D("BackgroundTotal", "BackgroundTotal", m_NBins, m_Bins);
  BackgroundTotal->SetXTitle("Energy [keV]");
  BackgroundTotal->SetYTitle("cts/keV/s");
  BackgroundTotal->SetLineColor(4);

  MEventSelector EventSelector;

  MSettingsMimrec Data;
  Data.Read(m_ConfigurationFileName);
  EventSelector.SetSettings(&Data);

  // Load geometry:
  MDGeometryQuest Geometry;
  if (Geometry.ScanSetupFile(m_GeometryFileName) == true) {
    cout<<"Geometry "<<Geometry.GetName()<<" loaded!"<<endl;
  } else {
    cout<<"Loading of geometry "<<Geometry.GetName()<<" failed!!"<<endl;
    return false;
  }  

  EventSelector.SetGeometry(&Geometry);


  MFileEventsTra SourceTra;
  if (m_Extract == true) {
    if (SourceTra.Open(m_Prefix + m_ExtractionFileName + ".source.tra", MFile::c_Write) == false) {
      merr<<"Unable to open source extraction tra file!"<<endl;
      return false;
    }
  }
  MFileEventsTra BackgroundTra;
  if (m_Extract == true) {
    if (BackgroundTra.Open(m_Prefix + m_ExtractionFileName + ".background.tra", MFile::c_Write) == false) {
      merr<<"Unable to open background extraction tra file!"<<endl;
      return false;
    }
  }


  // The source file
  for (unsigned int bf = 0; bf < m_SourceFileNames.size(); ++bf) {

    // Open the file
    MFileEventsTra Source;
    if (Source.Open(m_SourceFileNames[bf]) == false) {
      mlog<<"Unable to open file "<<m_SourceFileNames[bf]<<endl;
      return false;
    }
    mlog<<"Analyzing file: "<<m_SourceFileNames[bf]<<endl;

    if (m_MaximumTime != g_DoubleNotDefined) {
      if (m_SourceEvents[bf] == g_IntNotDefined) {
        // Get the number of events we have in the file:
        m_SourceEvents[bf] = Source.GetNEvents(true);
        mlog<<"Counts: "<<m_SourceEvents[bf]<<endl;
        Source.Rewind();
      }

      // Determine the number of events we want to read:
      m_SourceEvents[bf] = int(m_MaximumTime*m_SourceEvents[bf]/m_SourceTimes[bf]);
    }

    // Main loop
    int counts = 0;
    MPhysicalEvent* Event = 0;
    while ((Event = Source.GetNextEvent()) != 0) {
      if (++counts % 10000 == 0) mlog<<"Counts: "<<counts<<endl;
      if (EventSelector.IsQualifiedEvent(Event) == true) {
        m_SourceHistograms[bf]->Fill(Event->Ei());

        if (m_Extract == true) {
          if (SourceTra.AddEvent(Event) == false) {
            merr<<"Unable to add event to source extraction tra file!"<<endl;            
          }
        }
      }
      delete Event;
      if (m_Interrupt == true) {
        m_Interrupt = false;
        break;
      }
      if (m_MaximumTime != g_DoubleNotDefined && counts >= m_SourceEvents[bf]) {
        mlog<<"Stopping after "<<counts<<" counts..."<<endl;
        break;
      }
    }

    // Normalize histogram
    m_SourceHistograms[bf]->SetXTitle("Energy [keV]");
    m_SourceHistograms[bf]->SetYTitle("cts/keV/s");
    if (m_MaximumTime != g_DoubleNotDefined) {
      m_SourceHistograms[bf]->Scale(1.0/m_MaximumTime);
    } else {
      m_SourceHistograms[bf]->Scale(1.0/m_SourceTimes[bf]);
    }    
    for (int b = 1; b <= m_SourceHistograms[bf]->GetXaxis()->GetNbins(); ++b) {
      m_SourceHistograms[bf]->SetBinContent(b, m_SourceHistograms[bf]->GetBinContent(b)/m_SourceHistograms[bf]->GetBinWidth(b));
    }

    // Add to total histograms
    TotalTotal->Add(m_SourceHistograms[bf]);
    SourceTotal->Add(m_SourceHistograms[bf]);

    // Display this component:
    TCanvas* Canvas = new TCanvas(m_SourceNames[bf] + "Canvas", m_SourceNames[bf] + " - Canvas");
    Canvas->cd();
    m_SourceHistograms[bf]->Draw();
    Canvas->Update();
  }
  
  // The background file
  for (unsigned int bf = 0; bf < m_BackgroundFileNames.size(); ++bf) {

    // Open the file
    MFileEventsTra Bkg;
    if (Bkg.Open(m_BackgroundFileNames[bf]) == false) {
      mlog<<"Unable to open file "<<m_BackgroundFileNames[bf]<<endl;
      return false;
    }
    mlog<<"Analyzing file: "<<m_BackgroundFileNames[bf]<<endl;

    if (m_MaximumTime != g_DoubleNotDefined) {
      if (m_BackgroundEvents[bf] == g_IntNotDefined) {
        // Get the number of events we have in the file:
        m_BackgroundEvents[bf] = Bkg.GetNEvents(true);
        mlog<<"Counts: "<<m_BackgroundEvents[bf]<<endl;
        Bkg.Rewind();
      }

      // Determine the number of events we want to read:
      m_BackgroundEvents[bf] = int(m_MaximumTime*m_BackgroundEvents[bf]/m_BackgroundTimes[bf]);
    }

    // Main loop
    int counts = 0;
    MPhysicalEvent* Event = 0;
    while ((Event = Bkg.GetNextEvent()) != 0) {
      if (++counts % 10000 == 0) mlog<<"Counts: "<<counts<<endl;
      if (EventSelector.IsQualifiedEvent(Event) == true) {
        m_BackgroundHistograms[bf]->Fill(Event->Ei());

        if (m_Extract == true) {
          if (BackgroundTra.AddEvent(Event) == false) {
            merr<<"Unable to add event to background extraction tra file!"<<endl;            
          }
        }
      }
      delete Event;
      if (m_Interrupt == true) {
        m_Interrupt = false;
        break;
      }
      if (m_MaximumTime != g_DoubleNotDefined && counts >= m_BackgroundEvents[bf]) {
        mlog<<"Stopping after "<<counts<<" counts..."<<endl;
        break;
      }
    }

    // Normalize histogram
    m_BackgroundHistograms[bf]->SetXTitle("Energy [keV]");
    m_BackgroundHistograms[bf]->SetYTitle("cts/keV/s");
    if (m_MaximumTime != g_DoubleNotDefined) {
      m_BackgroundHistograms[bf]->Scale(1.0/m_MaximumTime);
    } else {
      m_BackgroundHistograms[bf]->Scale(1.0/m_BackgroundTimes[bf]);
    }
    for (int b = 1; b <= m_BackgroundHistograms[bf]->GetXaxis()->GetNbins(); ++b) {
      m_BackgroundHistograms[bf]->SetBinContent(b, m_BackgroundHistograms[bf]->GetBinContent(b)/m_BackgroundHistograms[bf]->GetBinWidth(b));
    }

    // Add to total histograms
    TotalTotal->Add(m_BackgroundHistograms[bf]);
    BackgroundTotal->Add(m_BackgroundHistograms[bf]);

    // Display this component:
    TCanvas* Canvas = new TCanvas(m_BackgroundNames[bf] + "Canvas", m_BackgroundNames[bf] + " - Canvas");
    Canvas->cd();
    m_BackgroundHistograms[bf]->Draw();
    Canvas->Update();
  }


  // Draw a stacked histogram of *all* components
  TCanvas* TotalStackCanvas = new TCanvas();
  TotalStackCanvas->cd();

  TLegend* leg = new TLegend(0.4, 0.6, 0.85, 0.85, NULL, "brNDC");

  // Combine all histograms into one:
  int Color = 2;
  THStack* TotalStack = new THStack("Components", "Components");
  for (unsigned int bf = 0; bf < m_SourceFileNames.size(); ++bf) {
    m_SourceHistograms[bf]->SetLineColor(Color);
    m_SourceHistograms[bf]->SetFillColor(Color);
    TotalStack->Add(m_SourceHistograms[bf]);
    TLegendEntry* entry = leg->AddEntry("NULL", m_SourceNames[bf], "l");
    entry->SetLineColor(Color);
    entry->SetTextColor(Color);
    Color++;
    if (Color == 5) Color++;
    if (Color == 10) Color++;
  }
  for (unsigned int bf = 0; bf < m_BackgroundFileNames.size(); ++bf) {
    m_BackgroundHistograms[bf]->SetLineColor(Color);
    m_BackgroundHistograms[bf]->SetFillColor(Color);
    TotalStack->Add(m_BackgroundHistograms[bf]);
    TLegendEntry* entry = leg->AddEntry("NULL", m_BackgroundNames[bf], "l");
    entry->SetLineColor(Color);
    entry->SetTextColor(Color);
    Color++;
    if (Color == 5) Color++;
    if (Color == 10) Color++;
  }
  TotalStack->Draw();
  TotalStack->GetHistogram()->SetXTitle("Energy [keV]");
  TotalStack->GetHistogram()->SetYTitle("cts/keV/s");
  TotalStack->Draw();
  leg->Draw();
  TotalStackCanvas->Update();
  TotalStackCanvas->SaveAs("Components.C");

  // Draw the total Canvas
  TCanvas* TotalTotalCanvas = new TCanvas();
  TotalTotalCanvas->cd();
  TotalTotal->Draw();
  TotalTotalCanvas->Update();
  TotalTotalCanvas->SaveAs(m_Prefix + "TotalTotal.C");

  // Draw the total Canvas
  TCanvas* TotalTotalComponentsCanvas = new TCanvas();
  TotalTotalComponentsCanvas->cd();
  TotalTotal->Draw();
  SourceTotal->Draw("SAME");
  BackgroundTotal->Draw("SAME");
  TotalTotalComponentsCanvas->Update();
  TotalTotalComponentsCanvas->SaveAs(m_Prefix + "TotalTotalComponents.C");

  if (m_SourceFileNames.size() > 0 && m_BackgroundFileNames.size() > 0) {
    TH1D* SB = new TH1D("SB", "Signal to background ratio", m_NBins, m_Bins);
    SB->SetXTitle("Energy [keV]");
    SB->SetYTitle("S/B");
    SB->SetLineColor(2);
    for (int b = 1; b < SB->GetNbinsX(); ++b) {
      if (BackgroundTotal->GetBinContent(b) > 0) {
        SB->SetBinContent(b, SourceTotal->GetBinContent(b)/BackgroundTotal->GetBinContent(b));
      }
    }
    TCanvas* SBCanvas = new TCanvas();
    SBCanvas->cd();
    SB->Draw();
    SBCanvas->Update();
    SBCanvas->SaveAs(m_Prefix + "SB.C");
  }


  // Determine total flux:
  double Int = TotalTotal->Integral()*TotalTotal->GetBinWidth(1);
  mlog<<"Total flux: "<<Int<<"cts/sec"<<endl;

  // Get the base file name of the tra file:
  MString AsciiOut = m_Prefix + "TotalTotal" + ".ASCIIspectrum.dat";
  ofstream out(AsciiOut, ios::out);
  
  for (int b = 1; b <= TotalTotal->GetNbinsX(); ++b) {
    out<<TotalTotal->GetBinCenter(b)<<" \t";
    out<<TotalTotal->GetBinContent(b)<<endl;
  }
  out.close();
  
  mlog<<"Wrote ASCII spectrum to "<<AsciiOut<<endl;

  // Close extraction files
  if (m_Extract == true) {
    SourceTra.Close();
    BackgroundTra.Close();
  }

  return true;
}


/******************************************************************************/

/******************************************************************************/


/******************************************************************************
 * Called when an interrupt signal is flagged
 * All catched signals lead to a well defined exit of the program
 */
void CatchSignal(int a)
{
  if (g_Prg != 0 && g_NInterruptCatches-- > 0) {
    cout<<"Catched signal Ctrl-C (ID="<<a<<"):"<<endl;
    g_Prg->Interrupt();
  } else {
    abort();
  }
}


/******************************************************************************
 * Main program
 */
int main(int argc, char** argv)
{
  // Catch a user interupt for graceful shutdown
  signal(SIGINT, CatchSignal);

  // Initialize global MEGALIB variables, especially mgui, etc.
  MGlobal::Initialize();

  TApplication BackgroundMixerApp("BackgroundMixerApp", 0, 0);

  g_Prg = new BackgroundMixer();

  if (g_Prg->ParseCommandLine(argc, argv) == false) {
    cerr<<"Error during parsing of command line!"<<endl;
    return -1;
  } 
  if (g_Prg->Analyze() == false) {
    cerr<<"Error during analysis!"<<endl;
    return -2;
  } 

  if (gROOT->GetListOfCanvases()->First() != 0 && g_Prg->UseGui() == true) {
    BackgroundMixerApp.Run();
  }

  cout<<"Program exited normally!"<<endl;

  return 0;
}

/*
 * BackgroundMixer: the end...
 ******************************************************************************/
