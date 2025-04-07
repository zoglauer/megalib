/*
 * MGlobal.cxx
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
// MGlobal
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MGlobal.h"

// Standard libs:
#include <cstdlib>
#include <limits>
#include <iostream>
#include <fstream>
#include <vector>
using namespace std;

// ROOT libs:
#include <TEnv.h>
#include <TStyle.h>
#include <TError.h>
#include <TSystem.h>
#include <TObjArray.h>
#include <TObjString.h>

// MEGAlib libs:
#include "MGUIDefaults.h"
#include "MStreams.h"
#include "MFile.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MGlobal)
#endif


////////////////////////////////////////////////////////////////////////////////


// Make this part of a future singleton class MGlobal
MString g_OSType = "Unknown";

unsigned int g_MajorVersion = 100;
unsigned int g_MinorVersion = 0;
unsigned int g_Version = 10000;
MString g_VersionString = "1.00.00";

// Remove year of copyright and replace with (C) by ....
const MString g_CopyrightYear = "2024";
const MString g_Homepage = "http://megalibtoolkit.com";
const MString g_MEGAlibPath = "$(MEGALIB)";

const MString g_StringNotDefined = "___NotDefined___";
const MVector g_VectorNotDefined = MVector(numeric_limits<double>::max()/11,
                                           numeric_limits<double>::max()/11,
                                           numeric_limits<double>::max()/11);
const MRotation g_RotationNotDefined = MRotation(numeric_limits<double>::max()/11,
                                                 numeric_limits<double>::max()/11,
                                                 numeric_limits<double>::max()/11,
                                                 numeric_limits<double>::max()/11,
                                                 numeric_limits<double>::max()/11,
                                                 numeric_limits<double>::max()/11,
                                                 numeric_limits<double>::max()/11,
                                                 numeric_limits<double>::max()/11,
                                                 numeric_limits<double>::max()/11);
const int g_IntNotDefined = numeric_limits<int>::max()-11;
const unsigned int g_UnsignedIntNotDefined = numeric_limits<unsigned int>::max()-11;
const unsigned long g_UnsignedLongNotDefined = numeric_limits<unsigned long>::max()-11;
const double g_DoubleNotDefined = numeric_limits<double>::max()/11;
const float g_FloatNotDefined = numeric_limits<float>::max()/11;

int g_Verbosity = 0;
const int c_Quiet    = 0;
const int c_Error    = 1;
const int c_Warning  = 2;
const int c_Info     = 3;
const int c_Chatty   = 4;
const int c_Extreme  = 5;

const double c_Pi = 3.14159265358979311600;
const double c_TwoPi = 2*c_Pi;
const double c_Sqrt2Pi = 2.506628274631;
const double c_Rad = c_Pi / 180.0;
const double c_Deg = 180.0 / c_Pi;
const double c_SpeedOfLight = 29.9792458E+9; // cm/s
const double c_E0 = 510.999; // keV
const double c_FarAway = 1E20; // cm
const double c_LargestEnergy = 0.999*numeric_limits<float>::max();
const MVector c_NullVector(0.0, 0.0, 0.0);

const double c_TeV = 1000000000;
const double c_GeV =    1000000;
const double c_MeV =       1000;
const double c_keV =          1;
const double c_eV  =          0.001;

const double c_km =      100000;
const double c_m  =         100;
const double c_cm =           1;
const double c_mm =           0.1;
const double c_um =           0.0001;
const double c_nm =           0.0000001;             


TMutex* g_Mutex = 0;
Long_t g_MainThreadID = 0;

MGlobal g_Global;


////////////////////////////////////////////////////////////////////////////////


bool MGlobal::Initialize(MString ProgramName, MString ProgramDescription)
{

#ifdef DEBUG1
  g_Verbosity = 1;
#endif

  // Find the operating systenm type
#if defined(_WIN32) || defined(_WIN64)
  g_OSType = "Windows";
#elif defined(__linux__)
  g_OSType = "Linux";
#elif defined(__APPLE__) && defined(__MACH__)
  g_OSType = "macOS";
#elif defined(__FreeBSD__)
  g_OSType = "FreeBSD";
#else
  g_OSType = "Unknown";
#endif


  // Load the GUI defaults by initializing the singleton
  MGUIDefaults::GetInstance();

  // Initialize the GUI stream
  mgui.DumpToGui(true);
  __merr.DumpToStdErr(true);
  __merr.DumpToStdOut(false);

  // Initilize some global ROOT variables:
  gEnv->SetValue("Gui.BackgroundColor", "#e3dfdf");

  // Font smoothing:
  gEnv->SetValue("X11.UseXft", "true");

  // Set a common ROOT style for all programs:
  gStyle->SetTitleBorderSize(0);
  gStyle->SetTitleFillColor(0);

  gStyle->SetTitleAlign(13);
  gStyle->SetTitleX(0.15f);
  gStyle->SetTitleY(0.95f);
  gStyle->SetTitleW(0.7f);
  //gStyle->SetTitleFont(72, "");

  gStyle->SetTitleFont(42, "XYZ");
  gStyle->SetTitleOffset(1.3f, "XY");
  //gStyle->SetTitleAlign(22);

  gStyle->SetLabelFont(42, "XYZ");

  gStyle->SetFrameBorderSize(0);
  gStyle->SetFrameBorderMode(0);
  gStyle->SetPadBorderSize(0);
  gStyle->SetPadBorderMode(0);
  gStyle->SetCanvasBorderSize(0);
  gStyle->SetCanvasBorderMode(0);
  gStyle->SetCanvasColor(0);

  gStyle->SetHistMinimumZero(true);
  gStyle->SetBit(kCanDelete);
  //gStyle->SetHistFillColor(8);
  gStyle->SetOptStat(0);

  // Speeds up drawing of canvas if an openGL capable GPU is available and we are not remote
  MString SSHClient(getenv("SSH_CLIENT"));
  gStyle->SetCanvasPreferGL(false);
  if (SSHClient.IsEmpty()) {
    if (g_OSType != "macOS") {
      gStyle->SetCanvasPreferGL(true);
    }
  }
  
  // Change the region where the drawing starts in canvases:
  float Margin = 0.15f;
  gStyle->SetPadLeftMargin(Margin);
  gStyle->SetPadRightMargin(Margin);
  gStyle->SetPadTopMargin(Margin);
  gStyle->SetPadBottomMargin(Margin);

  gStyle->SetLabelSize(0.03f, "XYZP");

  gStyle->SetPalette(55, 0);
  //gStyle->SetPalette(kBird);

  // Thinner, rounded lines for printed PS/EPS/PDF - changing this will screw up printed MImage*'s
  gStyle->SetLineScalePS(1); 
  gStyle->SetCapLinePS(1);

  // Ignore ROOT messages up to kInfo
  gErrorIgnoreLevel = kWarning;
  // For testing: abort on all errors, or warnings, or ...
  // gErrorAbortLevel = kError;


  // Read the versions:
  MString FileName = "$(MEGALIB)/config/Version.txt";
  MFile::ExpandFileName(FileName);

  ifstream in;
  in.open(FileName);
  if (in.is_open()) {
    in>>g_VersionString;
    in.close();
    // Now parse:
    vector<MString> Tokens = g_VersionString.Tokenize(".");
    if (Tokens.size() == 3) {
      g_MajorVersion = Tokens[0].ToInt()*100 + Tokens[1].ToInt();
      g_MinorVersion = Tokens[2].ToInt();
      g_Version = 100*g_MajorVersion + g_MinorVersion;
    } else {
      cout<<"BAD ERROR: Version could not be parsed correctly: \""<<g_VersionString<<"\""<<endl;
    }
  } else {
    cout<<"BAD ERROR: Cannot open file with version information (i.e. "<<FileName<<")!"<<endl;
  }

  // Show the intro
  if (ProgramName != "") {
    ShowIntro(ProgramName, ProgramDescription);
  }

  // Launch the update check at the end:
#if defined(___LINUX___) || defined(___MACOSX___)
  gSystem->Exec("bash ${MEGALIB}/config/configure_updatetest &");
#endif

  g_Mutex = new TMutex();
  g_MainThreadID = TThread::SelfId();

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MGlobal::CenterString(MString& String, unsigned int LineLength, bool DoBorders)
{
  if (LineLength < 2) return;

  while (true) {
    if (String.Length() < LineLength-2) {
      String.Prepend(" ");
    } else {
      break;
    }
    if (String.Length() < LineLength-2) {
      String.Append(" ");
    } else {
      break;
    }
  }
  if (DoBorders == true) {
    String.Prepend("*");
    String.Append("*");
  } else {
    String.Prepend(" ");
    String.Append(" ");
  }
}


////////////////////////////////////////////////////////////////////////////////


void MGlobal::ShowIntro(MString ProgramName, MString ProgramDescription)
{
  unsigned int LineLength = 0;

  MString ProgramLine = ProgramName;
  if (ProgramDescription != "") ProgramLine += MString(" - ") + ProgramDescription;
  if (ProgramLine.Length() > LineLength) LineLength = ProgramLine.Length();

  MString MEGAlibLine = "This program is part of MEGAlib version ";
  MEGAlibLine += g_VersionString;
  if (MEGAlibLine.Length() > LineLength) LineLength = MEGAlibLine.Length();

  MString CopyrightLine = "(C) by Andreas Zoglauer and contributors";
  if (CopyrightLine.Length() > LineLength) LineLength = CopyrightLine.Length();

  MString ReferenceIntroLine = "Master reference for MEGAlib:";
  if (ReferenceIntroLine.Length() > LineLength) LineLength = ReferenceIntroLine.Length();

  MString ReferenceLine = "A. Zoglauer et al., NewAR 50 (7-8), 629-632, 2006";
  if (ReferenceLine.Length() > LineLength) LineLength = ReferenceLine.Length();

  MString HomepageIntroLine = "For more information about MEGAlib please visit:";
  if (HomepageIntroLine.Length() > LineLength) LineLength = HomepageIntroLine.Length();

  MString HomepageLine = "http://megalibtoolkit.com";
  if (HomepageLine.Length() > LineLength) LineLength = HomepageLine.Length();

  MString DevelopmentVersion;
  if (g_MajorVersion % 2 == 1) {
    //if (g_MinorVersion == 0) {
    //  DevelopmentVersion = "You are using a VERY EXPERIMENTAL DEVELOPMENT version of MEGAlib";
    //} else if (g_MinorVersion < 5) {
    //  DevelopmentVersion = "You are using an EARLY development version of MEGAlib";
    //} else {
      DevelopmentVersion = "You are using a development version of MEGAlib";
    //}
    if (DevelopmentVersion.Length() > LineLength) LineLength = DevelopmentVersion.Length();
  }

  MString Update = MString("If you wish to update, make a backup, and then run \"make update\"");
  if (Update.Length() > LineLength) LineLength = Update.Length();

  LineLength += 10;

  CenterString(ProgramLine, LineLength);
  CenterString(MEGAlibLine, LineLength);
  CenterString(CopyrightLine, LineLength);
  CenterString(ReferenceIntroLine, LineLength);
  CenterString(ReferenceLine, LineLength);
  CenterString(HomepageIntroLine, LineLength);
  CenterString(HomepageLine, LineLength);
  CenterString(DevelopmentVersion, LineLength, false);
  CenterString(Update, LineLength, false);

  MString ClosedLine;
  for (unsigned int i = 0; i < LineLength; ++i) ClosedLine += '*';
  MString EmptyLine = "*";
  for (unsigned int i = 0; i < LineLength-2; ++i) EmptyLine += ' ';
  EmptyLine += "*";

  cout<<endl;
  cout<<ClosedLine<<endl;
  cout<<EmptyLine<<endl;
  cout<<ProgramLine<<endl;
  cout<<EmptyLine<<endl;
  cout<<MEGAlibLine<<endl;
  cout<<CopyrightLine<<endl;
  cout<<EmptyLine<<endl;
  cout<<ReferenceIntroLine<<endl;
  cout<<ReferenceLine<<endl;
  cout<<EmptyLine<<endl;
  cout<<HomepageIntroLine<<endl;
  cout<<HomepageLine<<endl;
  cout<<EmptyLine<<endl;
  cout<<ClosedLine<<endl;
  cout<<endl;

  // Check if this is a development version
  if (g_MajorVersion % 2 == 1) {
    cout<<DevelopmentVersion<<endl;
    cout<<endl;
  }

  // Check if we have found a newer version
  MString FileName = "$(MEGALIB)/config/UpdateCheck.txt";
  MFile::ExpandFileName(FileName);

  ifstream in;
  in.open(FileName);
  if (in.is_open()) {
    MString VersionString;
    in>>VersionString;
    in.close();
    // Now parse:
    vector<MString> Tokens = VersionString.Tokenize(".");
    if (Tokens.size() == 3) {
      unsigned int NewVersion = Tokens[0].ToInt()*10000 + Tokens[1].ToInt()*100 + Tokens[2].ToInt();
      if (NewVersion > g_Version) {
        MString NewVersion = MString("An updated MEGAlib version (") + VersionString + MString(") is in the repository!");
        CenterString(NewVersion, LineLength, false);
        cout<<NewVersion<<endl;
        cout<<Update<<endl;
        cout<<endl;
      }
    }
  }


}


// MGlobal.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
