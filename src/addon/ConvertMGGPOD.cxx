/* 
 * ConvertMGGPOD.cxx
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
#include <sstream>
#include <fstream>
#include <csignal>
#include <vector>
#include <list>
#include <iomanip>
#include <limits>
using namespace std;

// ROOT
#include <TApplication.h>
#include <TROOT.h>

// MEGAlib
#include "MGlobal.h"
#include "MAssert.h"
#include "MSimEvent.h"
#include "MSimHT.h"
#include "MSimIA.h"
#include "MFileEventsSim.h"
#include "MDGeometryQuest.h"
#include "MDVolumeSequence.h"
#include "MDDetector.h"
#include "MDACS.h"
#include "MDVolume.h"
#include "MTimer.h"
#include "MGUIProgressBar.h"

// Special
#include "fitsio.h"


/******************************************************************************/

struct HT
{
public:
  HT() { Detector = -1; TrackId = -1; MotherId = -1; 
    x = 0; y = 0; z = 0; E = 0; T = 0.0; }
  int Detector;
  double x;
  double y;
  double z;
  double E;
  double T;
  int MotherId; 
  int TrackId;
  list<unsigned int> Processes;

  MVector GetPosition() const { return MVector(x, y, z); }

  MString ToString() {
    ostringstream out;
    out.setf(ios_base::fixed, ios_base::floatfield);
    out<<setprecision(20)
       <<"HTsim "<<Detector<<";"<<x<<";"<<y<<";"<<z<<";"<<E<<";";
    out.setf(ios_base::scientific, ios_base::floatfield);    
    out<<setprecision(2);
    out<<T;
    if (TrackId != -1) {
      out<<";"<<TrackId;
      /*
        list<unsigned int>::iterator Iter;
        for (Iter = Processes.begin(); Iter != Processes.end(); ++Iter) {
        if ((*Iter) <= ConvertMGGPOD::m_Processes.size()) {
        //out<<"  p: "<<(*Iter);
        out<<"  p: "<<ConvertMGGPOD::m_Processes[(*Iter)-1];
        } else {
        cerr<<"Unknown process: "<<(*Iter)<<endl;
        }
        }
      */
    }
    return out.str();
  }
};


/******************************************************************************/

class ConvertMGGPOD
{
public:
  /// Default constructor
  ConvertMGGPOD();
  /// Default destructor
  ~ConvertMGGPOD();
  
  /// Parse the command line
  bool ParseCommandLine(int argc, char** argv);
  /// Analyze what ever needs to be analyzed...
  bool Analyze();
  /// Interrupt the analysis
  void Interrupt() { m_Interrupt = true; }
  /// Display all data
  bool Display();
  /// Return whether or not the Gui might be used...
  bool UseGui() { return m_UseGui; }

  /// Geant3 ProcessNames:
  static vector<MString> m_Processes;

private:
  /// Retrieve the process name from the integer:
  MString GetProcessName(int Number);
  /// Do all the discretization
  bool Discretize(int EventID, vector<MSimIA>& IAs, vector<MSimHT>& HTs);

private:
  /// True, if the analysis needs to be interrupted
  bool m_Interrupt;

  /// Name of input file
  vector<MString> m_FileName;

  /// Name of geometry file
  MString m_GeometryFileName;

  /// Name of output file
  MString m_SimFileName;

  /// Maximum line width
  static const int m_MaxLineWidth;

  /// True if the data should be displayed
  bool m_Display;

  /// The geometry
  MDGeometryQuest m_Geometry;

  /// The simulation output file
  MFileEventsSim* m_SimFile;

  /// Event ID
  int m_EventID;

  /// Total number of vetoed events
  int m_NVetoedEvents; 

  /// Total number of ignored singles
  int m_NIgnoredSingles; 

  /// maximum number of hits per detector type
  unsigned int m_MaxNHits; 

  /// events, which exceed maximum number of hits per detector type
  int m_NExceedMaxNHits; 

  /// events, which exceed maximum number of hits per detector type
  int m_NStoredEvents; 

  /// True if GUI is used
  bool m_UseGui;

  /// Bool ignore single site events
  bool m_IgnoreSingles;

  /// Ignore veto
  bool m_IgnoreVeto;

  /// Ignore veto
  bool m_UseNoHitsEvents;

  /// Do Discretization
  bool m_DoDiscretization;

  /// Energy window:
  double m_EnergyMin;
  double m_EnergyMax;

  /// Ignore veto
  int m_NIgnoredEnergyWindow;

  //! Maximum number of events to convert
  int m_MaxEventID;
};

/******************************************************************************/

const int ConvertMGGPOD::m_MaxLineWidth = 160;
vector<MString> ConvertMGGPOD::m_Processes;


/******************************************************************************
 * Default constructor
 */
ConvertMGGPOD::ConvertMGGPOD() : m_Interrupt(false), m_Display(false)
{
  // Intentionally left blanck

  m_MaxNHits = 5000;

  m_SimFile = 0;

  m_EventID = 0;
  m_NVetoedEvents = 0;
  m_NExceedMaxNHits = 0;
  m_NIgnoredSingles = 0;
  m_NStoredEvents = 0;
  m_NIgnoredEnergyWindow = 0;

  m_Processes.push_back("NEXT"); // 1
  m_Processes.push_back("MULS");
  m_Processes.push_back("LOSS");
  m_Processes.push_back("FIEL");
  m_Processes.push_back("DCAY");
  m_Processes.push_back("PAIR"); // 6
  m_Processes.push_back("COMP"); // 7
  m_Processes.push_back("PHOT"); // 8
  m_Processes.push_back("BREM");
  m_Processes.push_back("DRAY"); // 10
  m_Processes.push_back("ANNI");
  m_Processes.push_back("HADR");
  m_Processes.push_back("ECOH");
  m_Processes.push_back("EVAP");
  m_Processes.push_back("FISS");
  m_Processes.push_back("ABSO");
  m_Processes.push_back("ANNH");
  m_Processes.push_back("CAPT");
  m_Processes.push_back("EINC");
  m_Processes.push_back("INHE"); // 20
  m_Processes.push_back("MUNU");
  m_Processes.push_back("TOFM");
  m_Processes.push_back("PFIS");
  m_Processes.push_back("SCUT");
  m_Processes.push_back("RAYL");
  m_Processes.push_back("PARA");
  m_Processes.push_back("PRED");
  m_Processes.push_back("LOOP");
  m_Processes.push_back("NULL");
  m_Processes.push_back("STOP"); // 30

  m_UseGui = true;
  m_IgnoreSingles = false;
  m_IgnoreVeto = false;
  m_UseNoHitsEvents = false;
  m_DoDiscretization = true;

  m_EnergyMin = 0;
  m_EnergyMax = numeric_limits<double>::max();

  m_MaxEventID = numeric_limits<int>::max();
}


/******************************************************************************
 * Default destructor
 */
ConvertMGGPOD::~ConvertMGGPOD()
{
  // Intentionally left blank

  if (m_SimFile != 0) {
    m_SimFile->Close();
  }

  delete m_SimFile;
}


/******************************************************************************
 * Parse the command line
 */
bool ConvertMGGPOD::ParseCommandLine(int argc, char** argv)
{
  ostringstream Usage;
  Usage<<endl;
  Usage<<"  Usage: ConvertMGGPOD <options>"<<endl;
  Usage<<"    General options:"<<endl;
  Usage<<"         -f:                           file name (option may appear several times)"<<endl;
  Usage<<"         -g:                           geometry file name"<<endl;
  Usage<<"         -o:                           output file name"<<endl;
  Usage<<"         -m:                           max number of hits per detector TYPE before discretization (default: "<<m_MaxNHits<<")"<<endl;
  Usage<<"         -d:                           display fits content"<<endl;
  Usage<<"         -i:                           Ignore singles"<<endl;
  Usage<<"         --max-event-id <max>:         Stop after this ID is reached"<<endl;
  Usage<<"         --energy-window <min> <max>:  obvious"<<endl;
  Usage<<"         --use-nohits-events:          obvious"<<endl;
  Usage<<"         --ignore-veto:                obvious"<<endl;
  Usage<<"         --no-gui:                     obvious"<<endl;
  Usage<<"         --no-discretization:          obvious"<<endl;
  Usage<<"         -h:                   print this help"<<endl;
  Usage<<endl;

  bool GeoIsOpen = false;
  string Option;

  // Check for help
  for (int i = 1; i < argc; i++) {
    Option = argv[i];
    if (Option == "-h" || Option == "--help" || Option == "?" || Option == "-?") {
      cout<<Usage.str()<<endl;
      return false;
    }
  }

  // Now parse the command line options:
  for (int i = 1; i < argc; i++) {
    Option = argv[i];

    // First check if each option has sufficient arguments:
    // Single argument
    if (Option == "-f" || Option == "-g" || Option == "-o" || 
        Option == "-m" || Option == "-s" || Option == "-v") {
      if (!((argc > i+1) && argv[i+1][0] != '-')){
        cout<<"Error: Option "<<argv[i][1]<<" needs a second argument!"<<endl;
        cout<<Usage.str()<<endl;
        return false;
      }
    } 
    // Multiple arguments_
    else if (Option == "--energy-window") {
      if (!((argc > i+2) && argv[i+1][0] != '-' && argv[i+2][0] != '-')){
        cout<<"Error: Option "<<argv[i][1]<<" needs two arguments argument!"<<endl;
        cout<<Usage.str()<<endl;
        return false;
      }
    }

    // Then fulfill the options:
    if (Option == "-f") {
      m_FileName.push_back(argv[++i]);
      cout<<"Accepting file name: "<<m_FileName.back()<<endl;
    } else if (Option == "-g") {
      m_GeometryFileName = argv[++i];
      if (m_Geometry.ScanSetupFile(m_GeometryFileName) == true) {
        cout<<"Geometry "<<m_Geometry.GetName()<<" loaded!"<<endl;
        GeoIsOpen = true;
      } else {
        cout<<"Loading of geometry "<<m_Geometry.GetName()<<" failed!!"<<endl;
        return false;
      }
      m_Geometry.ActivateNoising(false);
      m_Geometry.SetGlobalFailureRate(0.0);
    } else if (Option == "-o") {
      m_SimFileName = argv[++i];
      cout<<"Accepting output file name: "<<m_SimFileName<<endl;
    } else if (Option == "-m") {
      m_MaxNHits = atoi(argv[++i]);
      cout<<"Accepting maximum number of hits for discretizer per detector type: "<<m_MaxNHits<<endl;
    } else if (Option == "-v") {
      mdep<<"Option -v is no longer supported and has been replaced by automatic detection..."<<endl;
    } else if (Option == "-d") {
      m_Display = true;
      cout<<"Accepting display option!"<<endl;
    } else if (Option == "--ignore-veto") {
      m_IgnoreVeto = true;
      cout<<"Ignoring all raised vetos!"<<endl;
    } else if (Option == "--energy-window") {
      m_EnergyMin = atof(argv[++i]);
      m_EnergyMax = atof(argv[++i]);
      cout<<"Using energy-window: "<<m_EnergyMin<<"-"<<m_EnergyMax<<endl;
      cout<<"ATTENTION: The energy window is before noising and defective channel analysis!!!"<<endl;
    } else if (Option == "--max-event-id") {
      m_MaxEventID = atoi(argv[++i]);
      cout<<"Using maximum event Id: "<<m_MaxEventID<<endl;
    } else if (Option == "--use-nohits-events") {
      m_UseNoHitsEvents = true;
      cout<<"Using no hits events!"<<endl;
    } else if (Option == "--no-discretization") {
      m_DoDiscretization = false;
      cout<<"Not doing discretization!"<<endl;
    } else if (Option == "--no-gui") {
      m_UseGui = false;
      cout<<"Not using the graphical user interface!"<<endl;
      gROOT->SetBatch(true);
    } else if (Option == "-i") {
      m_IgnoreSingles = true;
      cout<<"Ignoring single site events!"<<endl;
    } else {
      cout<<"Error: Unknown option \""<<Option<<"\"!"<<endl;
      cout<<Usage.str()<<endl;
      return false;
    }
  }

  if (GeoIsOpen == false) {
    cout<<"You did not give a geometry file!"<<endl;
    cout<<Usage.str()<<endl;
    return false;
  }

  if (m_FileName.size() > 0 && m_FileName[0].EndsWith(".fits") == true) {
    if (m_SimFileName == "") {
      m_SimFileName = m_FileName[0];
      m_SimFileName.Replace(m_FileName[0].Length()-5, 5, ".sim");
      cout<<"Accepting output file name: "<<m_SimFileName<<endl;
    }    
    
    // Open output file:
    m_SimFile = new MFileEventsSim(&m_Geometry);
    if (m_SimFile->Open(m_SimFileName, MFile::c_Write) == false) {
      cout<<"Unable to open output file!"<<endl;
      cout<<Usage.str()<<endl;
      return false;
    }
    m_SimFile->SetGeometryFileName(m_GeometryFileName);
    m_SimFile->SetVersion(MSimEvent::GetOutputVersion());
    m_SimFile->WriteHeader();
  } else {
    cout<<"We do not have a valid fits file!"<<endl;
    cout<<Usage.str()<<endl;
    return false;
  }

  if (m_IgnoreVeto == false) {
    mimp<<"Vetoing all events in Scintillation/ACS detectors independent of trigger criteria in the geometry file"<<endl;
    mimp<<"If you do not want this launch ConvertMGGPOD with --ignore-veto"<<show;
  }

  return true;
}


/******************************************************************************
 * Do whatever analysis is necessary
 */
bool ConvertMGGPOD::Analyze()
{
  if (m_Interrupt == true) return false;

  if (m_Display == true) {
    Display();
    return true;
  }

  // Section 1:
  // initialize the files
  int NKeys = 0;
  int status = 0;
  char value[1000];
  char templt[100];

  int EventId = 0;
  fitsfile* ActFile = 0; 
  fitsfile* InitFile = 0; 
    
  double m_TimeFitsio = 0;
  MTimer Fitsio;
  double m_TimeDiscretization = 0;
  MTimer Discretization;
      
  for (unsigned int f = 0; f < m_FileName.size(); ++f) {
    if (fits_open_file(&ActFile, m_FileName[f], READONLY, &status) ||
        fits_open_file(&InitFile, m_FileName[f], READONLY, &status) ) {
      cout<<"Unable to open file \""<<m_FileName[f]<<"\""<<endl;
      return false;
    }

    ostringstream hout;
    hout<<endl;
    hout<<"OF "<<m_FileName[f]<<endl;
    hout<<endl;
    m_SimFile->AddText(hout.str().c_str());
  
    
    // Initialize ACT extension
    int TypeAct = 0;
    long RowsAct = 1;
    long NRowsAct = 0;

    // get the type:
    fits_movabs_hdu(ActFile, 2, NULL, &status);
    fits_get_hdrspace(ActFile, &NKeys, NULL, &status); // get # of keywords
    if (NKeys >= 1) {
      fits_read_keyword(ActFile, (char*) "EXTNAME", value, NULL, &status);
      string EName(value);
      if (EName ==  "'ACT1    '") {
        cout<<"Found ACT1 extension!"<<endl;
        TypeAct = 1;
      } else if (EName ==  "'ACT2    '") {
        cout<<"Found ACT2 extension!"<<endl;
        TypeAct = 2;
      } 
    } 
    if (TypeAct == 0) {
      cout<<"No valid ACT extension found!"<<endl;
      return false;
    }
       
    map<MString, int> ActColNames;
    int NColsAct = 0;
    fits_get_num_cols(ActFile, &NColsAct, &status);
    for (int c = 1; c <= NColsAct; ++c) {
      int col;
      snprintf(templt, sizeof(templt), "%d", c);
      fits_get_colname(ActFile, CASEINSEN, templt, value, &col, &status);
      ActColNames[value] = c;
    }
 
    fits_get_num_rows(ActFile, &NRowsAct, &status);
    cout<<"Rows act-file: "<<NRowsAct<<endl;


   // Initialize INIT extension
    int TypeInit = 0;
    long RowsInit = 1;
    long NRowsInit = 0;
    
    // get the type:
    fits_movabs_hdu(InitFile, 3, NULL, &status);
    fits_get_hdrspace(InitFile, &NKeys, NULL, &status); // get # of keywords
    if (NKeys >= 1) {
      fits_read_keyword(InitFile, (char*) "EXTNAME", value, NULL, &status);
      string EName(value);
      if (EName ==  "'INIT1   '") {
        cout<<"Found INIT1 extension!"<<endl;
        TypeInit = 1;
      } else if (EName ==  "'INIT2   '") {
        cout<<"Found INIT2 extension!"<<endl;
        TypeInit = 2;
      } 
    } 
    if (TypeInit == 0) {
      cout<<"No valid INIT extension found!"<<endl;
      return false;
    }

    fits_get_num_rows(InitFile, &NRowsInit, &status);
    cout<<"Rows init-file: "<<NRowsInit<<endl;

    // Get the columns:
    map<MString, int> InitColNames;
    int NColsInit = 0;
    fits_get_num_cols(InitFile, &NColsInit, &status);
    for (int c = 1; c <= NColsInit; ++c) {
      int col;
      snprintf(templt, sizeof(templt), "%d", c);
      fits_get_colname (InitFile, CASEINSEN, templt, value, &col, &status);
      InitColNames[value] = c;
    }

    ostringstream zout;
    
    //char nullstr[] = "*";
    //char* val = 0;
    int anynul;
  
    double nulldbl = 0.0;
    double valdbl = 0;
    
    int nullint = 0;
    int valint = 0;
    
    vector<MSimIA> IAs;
    vector<MSimHT> HTs;
    
    MVector Vec;
    
    int MaxTrackId = 0;
    bool TruncatedAdded = false;
   
    MGUIProgressBar* Progress = 0;
    if (gClient != 0 && gClient->GetRoot() != 0 && m_UseGui == true) {
      Progress = new MGUIProgressBar("Progress", "Progress of conversion");
      Progress->SetMinMax(0, NRowsInit);
    }

    while (RowsInit <= NRowsInit /*&& RowsAct <= NRowsAct*/) {

      //if (RowsInit > 10) break;
      if (m_Interrupt == true) break;
      if (m_EventID >= m_MaxEventID) break;
      
      Fitsio.Start();
      
      IAs.clear();
      HTs.clear();
      TruncatedAdded = false;
      MaxTrackId = 0;
      
      const int NoId = numeric_limits<int>::max();
      EventId = NoId;
      
      if (Progress != 0) {
        Progress->SetValue(RowsInit);
      }
      
      // Get the IA information:
      for (; RowsInit <= NRowsInit; ++RowsInit) {
      
        //cout<<"Init read..."<<RowsInit<<"!"<<NRowsInit<<endl;
        //val = value;
        if (fits_read_col_int(InitFile, InitColNames["SCTIME"], RowsInit, 1, 1, nullint, &valint, &anynul, &status) ) {
          cerr<<"Column read failed!"<<endl;
          break;
        }
        //cout<<"Idinit: "<<value<<endl;
        if (EventId == NoId) {
          EventId = valint;
        }
      
        if (EventId < valint) break;

        MSimIA ia;
        ia.SetDetectorType(-1);
        ia.SetProcess("INIT");
        ia.SetOriginID(-1);
  
        if (fits_read_col_int(InitFile, InitColNames["ITRA"], RowsInit, 1, 1, nullint, &valint, &anynul, &status) ) {
          cerr<<"Column read failed!"<<endl;
          break;
        }
        ia.SetID(valint);
        if (valint > MaxTrackId) MaxTrackId = valint;
  
        if (InitColNames["NTBEAM"] != 0) {
          if (fits_read_col_int(InitFile, InitColNames["NTBEAM"], RowsInit, 1, 1, nullint, &valint, &anynul, &status) ) {
            cerr<<"Column read failed!"<<endl;
            break;
          }
          ia.SetOriginID(valint);
        }

        if (fits_read_col_int(InitFile, InitColNames["IPART"], RowsInit, 1, 1, nullint, &valint, &anynul, &status) ) {
          cerr<<"Column read failed!"<<endl;
          break;
        }
        ia.SetSecondaryParticleID(valint);
  
        if (fits_read_col_dbl(InitFile, InitColNames["E0"], RowsInit, 1, 1, nulldbl, &valdbl, &anynul, &status) ) {
          cerr<<"Column read failed!"<<endl;
          break;
        }
        ia.SetSecondaryEnergy(valdbl);
  
        if (fits_read_col_dbl(InitFile, InitColNames["X"], RowsInit, 1, 1, nulldbl, &valdbl, &anynul, &status) ) {
          cerr<<"Column read failed!"<<endl;
          break;
        }
        Vec.SetX(valdbl);
  
        if (fits_read_col_dbl(InitFile, InitColNames["Y"], RowsInit, 1, 1, nulldbl, &valdbl, &anynul, &status) ) {
          cerr<<"Column read failed!"<<endl;
          break;
        }
        Vec.SetY(valdbl);
  
        if (fits_read_col_dbl(InitFile, InitColNames["Z"], RowsInit, 1, 1, nulldbl, &valdbl, &anynul, &status) ) {
          cerr<<"Column read failed!"<<endl;
          break;
        }
        Vec.SetZ(valdbl);
        ia.SetPosition(Vec);

        if (fits_read_col_dbl(InitFile, InitColNames["PX"], RowsInit, 1, 1, nulldbl, &valdbl, &anynul, &status) ) {
          cerr<<"Column read failed!"<<endl;
          break;
        }
        Vec.SetX(valdbl);

        if (fits_read_col_dbl(InitFile, InitColNames["PY"], RowsInit, 1, 1, nulldbl, &valdbl, &anynul, &status) ) {
          cerr<<"Column read failed!"<<endl;
          break;
        }
        Vec.SetY(valdbl);
  
        if (fits_read_col_dbl(InitFile, InitColNames["PZ"], RowsInit, 1, 1, nulldbl, &valdbl, &anynul, &status) ) {
          cerr<<"Column read failed!"<<endl;
          break;
        }
        Vec.SetZ(valdbl);
        ia.SetSecondaryDirection(Vec);

        if (InitColNames["PolX"] != 0) {
          if (fits_read_col_dbl(InitFile, InitColNames["PolX"], RowsInit, 1, 1, nulldbl, &valdbl, &anynul, &status) ) {
            cerr<<"Column read failed!"<<endl;
            break;
          }
          Vec.SetX(valdbl);
          
          if (fits_read_col_dbl(InitFile, InitColNames["PolY"], RowsInit, 1, 1, nulldbl, &valdbl, &anynul, &status) ) {
            cerr<<"Column read failed!"<<endl;
            break;
          }
          Vec.SetY(valdbl);
          
          if (fits_read_col_dbl(InitFile, InitColNames["PolZ"], RowsInit, 1, 1, nulldbl, &valdbl, &anynul, &status) ) {
            cerr<<"Column read failed!"<<endl;
            break;
          }
          Vec.SetZ(valdbl);
          ia.SetMotherPolarisation(Vec);
        }

        if (fits_read_col_int(InitFile, InitColNames["INTERACTION"], RowsInit, 1, 1, nullint, &valint, &anynul, &status) ) {
          cerr<<"Column read failed!"<<endl;
          break;
        }
        ia.SetProcess(GetProcessName(valint));
      
        if (fits_read_col_int(InitFile, InitColNames["PARENT"], RowsInit, 1, 1, nullint, &valint, &anynul, &status) ) {
          cerr<<"Column read failed!"<<endl;
          break;
        }
        ia.SetParentNucleus(valint);
  
        IAs.push_back(ia);
      }
      
      unsigned int NHits = 0;
      // Now the Act information:
      int TempRowsAct = 0;
      for (TempRowsAct = RowsAct; TempRowsAct <= NRowsAct; ++TempRowsAct) {
        //cout<<"Act read..."<<TempRowsAct<<"!"<<NRowsAct<<endl;
        // Check if the event number is ok
        if (fits_read_col_int(ActFile, ActColNames["SCTIME"], TempRowsAct, 1, 1, nullint, &valint, &anynul, &status) ) {
          cerr<<"Column read failed!"<<endl;
          break;
        }
        NHits++;
        if (EventId > valint) {
          continue;
        }
        if (EventId < valint) {
          break;
        }
      }

      if (NHits > m_MaxNHits) {
        m_NExceedMaxNHits++;
        RowsAct = TempRowsAct;
        m_TimeFitsio += Fitsio.ElapsedTime();
        ostringstream out;
        out<<"SE"<<endl;
        out<<"ID "<<++m_EventID<<"  "<<EventId<<endl;
        out<<"EX "<<endl;
        m_SimFile->AddText(out.str().c_str());
      } else { 
        // Now the Act information:
        for (; RowsAct <= NRowsAct; ++RowsAct) {
          //cout<<"Act read..."<<RowsAct<<"!"<<NRowsAct<<endl;
          // Check if the event number is ok
          if (fits_read_col_int(ActFile, ActColNames["SCTIME"], RowsAct, 1, 1, nullint, &valint, &anynul, &status) ) {
            cerr<<"Column read failed: act: 1"<<endl;
            break;
          }
          //cout<<"Idact: "<<value<<endl;
          if (EventId > valint) {
            continue;
          }
          if (EventId < valint) {
            break;
          }
        
          MSimHT ht(&m_Geometry);
          ht.SetDetectorType(-1);
        
          if (fits_read_col_dbl(ActFile, ActColNames["X"], RowsAct, 1, 1, nulldbl, &valdbl, &anynul, &status) ) {
            cerr<<"Column read failed: act: 5"<<endl;
            break;
          }
          Vec.SetX(valdbl);
          if (fits_read_col_dbl(ActFile, ActColNames["Y"], RowsAct, 1, 1, nulldbl, &valdbl, &anynul, &status) ) {
            cerr<<"Column read failed: act: 6"<<endl;
            break;
          }
          Vec.SetY(valdbl);
          if (fits_read_col_dbl(ActFile, ActColNames["Z"], RowsAct, 1, 1, nulldbl, &valdbl, &anynul, &status) ) {
            cerr<<"Column read failed: act: 7"<<endl;
            break;
          }
          Vec.SetZ(valdbl);
          ht.SetPosition(Vec);
          if (fits_read_col_dbl(ActFile, ActColNames["DESTEP"], RowsAct, 1, 1, nulldbl, &valdbl, &anynul, &status) ) {
            cerr<<"Column read failed: act: 4"<<endl;
            break;
          }
          ht.SetEnergy(valdbl);
          if (fits_read_col_dbl(ActFile, ActColNames["TOFG"], RowsAct, 1, 1, nulldbl, &valdbl, &anynul, &status) ) {
            cerr<<"Column read failed: act: 8"<<endl;
            break;
          }
          ht.SetTime(valdbl);
          if (TypeAct == 2) {
//             if (fits_read_col_int(ActFile, 9, RowsAct, 1, 1, nullint, &valint, &anynul, &status) ) {
//               cerr<<"Column read failed: act: 9"<<endl;
//               break;
//             }
//             if (valint < 0) {
//               if (TruncatedAdded == false) {
//                 MaxTrackId++;
//                 MSimIA ia;
//                 ia.SetDetectorType(0);
//                 ia.SetProcess("TRNC");
//                 ia.SetOriginID(0);
//                 ia.SetID(MaxTrackId);
//                 IAs.push_back(ia);
//                 TruncatedAdded = true;
//               }
//               valint = MaxTrackId;
//             }
//             ht.AddOrigin(valint);

            if (fits_read_col_int(ActFile, ActColNames["ITRA"], RowsAct, 1, 1, nullint, &valint, &anynul, &status) ) {
              cerr<<"Column read failed: act: 10"<<endl;
              break;
            }
            if (valint < 0) {
              if (TruncatedAdded == false) {
                MaxTrackId++;
                MSimIA ia;
                ia.SetDetectorType(0);
                ia.SetProcess("TRNC");
                ia.SetOriginID(0);
                ia.SetID(MaxTrackId);
                IAs.push_back(ia);
                TruncatedAdded = true;
              }
              valint = MaxTrackId;
            }
            ht.AddOrigin(valint);

            if (fits_read_col_dbl(ActFile, ActColNames["PROCESSES"], RowsAct, 1, 1, nulldbl, &valdbl, &anynul, &status) ) {
              cerr<<"Column read failed: act: 11"<<endl;
              break;
            }
            for (int i = 0; i < 4; ++i) {
              if (valdbl - 1000*floor(valdbl/1000) != 0) {
                //ht.Processes.push_front(int(valdbl - 1000*floor(valdbl/1000)));
              }
              valdbl = floor(valdbl/1000);
            }
          }
          HTs.push_back(ht);
        }
      
        m_TimeFitsio += Fitsio.ElapsedTime();

        Discretization.Start();
        Discretize(EventId, IAs, HTs);
        m_TimeDiscretization += Discretization.ElapsedTime();
      }

      if (Progress != 0 && Progress->TestCancel() == true) {
        m_Interrupt = true;
        delete Progress;
        Progress = 0;
        break;
      }
    } // RowsInit loop
    delete Progress;
 
    fits_close_file(ActFile, &status);
    fits_close_file(InitFile, &status);
  } // next sim file...

  m_SimFile->CloseEventList();

  ostringstream out;
  out<<endl;
  out<<"Some statistical information for file "<<m_SimFileName<<":"<<endl;
  out<<endl;
  out<<"Number of simulated events:                                                 "<<EventId<<endl;
  out<<"Number of written events:                                                   "<<m_EventID<<endl;
  out<<endl;
  out<<"Number of good (not vetoed, ignored, outside energy window) events:        "<<m_NStoredEvents<<endl;
  out<<"Number of vetoed events:                                                    "<<m_NVetoedEvents<<endl;
  if (m_IgnoreSingles == true) {
    out<<"Number of ignored singles:                                                  "<<m_NIgnoredSingles<<endl;
  }
  out<<"Number of events outside energy window:                                     "<<m_NIgnoredEnergyWindow<<endl;
 
  out<<"Number of events, which are rejected, because they have too many hits:      "<<m_NExceedMaxNHits<<endl;  
  out<<endl;
  out<<"Time spent in Fitsio:                                                       "<<m_TimeFitsio<<endl;
  out<<"Time spent in Discretization:                                               "<<m_TimeDiscretization<<endl;
  m_SimFile->AddFooter(out.str().c_str());

  mout<<out.str()<<endl;

  return true;
}


/******************************************************************************
 * Do all the discretization:
 */
bool ConvertMGGPOD::Discretize(int StartedEventID, 
                               vector<MSimIA>& IAs, vector<MSimHT>& HTs)
{
  //cout<<"Discretizing event: "<<m_EventID<<"/"<<StartedEventID<<" with "<<HTs.size()<<" steps"<<endl;

  if (m_UseNoHitsEvents == false && HTs.size() == 0) return false;
  m_EventID++;

  // Sanity checks:
  if (IAs.size() > 0) {
    if (IAs[0].GetProcess() != "INIT" && 
        IAs[0].GetProcess() != "BET-" && 
        IAs[0].GetProcess() != "BET+" && 
        IAs[0].GetProcess() != "ANNI" && 
        IAs[0].GetProcess() != "ITRN" && 
        IAs[0].GetProcess() != "ALPH" && 
        IAs[0].GetProcess() != "ECAP" && 
        IAs[0].GetProcess() != "ISML") {
      mout<<"Event "<<m_EventID<<": Type of first IA is no valid initial type (such as INIT): >"<<IAs[0].GetProcess()<<"<"<<endl;
      mout<<"THIS IS A SEVERE ERROR AND SOMETHING IS BADLY WRONG WITH YOUR SIMULATION/FITS FILE!!!"<<endl;
      mout<<"Ignoring this event!"<<endl;
      return false;
    }
  }
  

  // Check energy window:
  double HTEnergy = 0;
  for (unsigned int i = 0; i < HTs.size(); ++i) {
    HTEnergy += HTs[i].GetEnergy();
  }
  if (HTEnergy < m_EnergyMin || HTEnergy > m_EnergyMax) {
    ostringstream out;
    out<<"SE"<<endl;
    out<<"ID "<<m_EventID<<"  "<<StartedEventID<<endl;
    out<<"EW "<<HTEnergy<<endl;
    m_SimFile->AddText(out.str().c_str());    
    m_NIgnoredEnergyWindow++;
    return true;
  }

  // Assimilate the data:
  MSimEvent Event; 
  Event.SetGeometry(&m_Geometry);
  Event.SetID(m_EventID);
  Event.SetStartedEventNumber(StartedEventID);
  for (unsigned int i = 0; i < IAs.size(); ++i) {
    Event.AddIA(IAs[i]);
  }

  for (unsigned int i = 0; i < HTs.size(); ++i) {
    //cout<<HTs[i].ToString()<<endl;
    // First check if we have a detector at the Hit position
    MDVolumeSequence* VS = m_Geometry.GetVolumeSequencePointer(HTs[i].GetPosition(), true, true);
    if (VS->GetDetector() == 0) {
      mout<<"Event "<<m_EventID<<": We have a hit outside a sensitive volume!"<<endl;
      mout<<"I will IGNORE this hit! "<<HTs[i].GetEnergy()<<" keV are lost for this event!"<<endl;
    } else {
      Event.AddHT(HTs[i]);
    }
    delete VS;
  }

 
  unsigned int HitsLimit = 500;
  if (HTs.size() > HitsLimit) {
    cout<<"Event "<<StartedEventID<<": Starting discretization of "<<HTs.size()<<" hits ..."<<endl;
  }

  // Upgrade detector information in IAs:
  for (unsigned int i = 0; i < Event.GetNIAs(); ++i) {
    MVector Pos = Event.GetIAAt(i)->GetPosition();
    MDVolumeSequence* S = m_Geometry.GetVolumeSequencePointer(Pos);
    if (S->GetDetector() != 0 && S->GetDeepestVolume()->IsSensitive() == true) {
      Event.GetIAAt(i)->SetDetectorType(S->GetDetector()->GetType());
    } else {
      Event.GetIAAt(i)->SetDetectorType(0);
    }
    delete S;
  }    
  
  // Upgrade the information about the detector:
  for (unsigned int i = 0; i < Event.GetNHTs(); ++i) {
    MDVolumeSequence* S = Event.GetHTAt(i)->GetVolumeSequence();
    if (S->GetDetector() != 0) {    
      Event.GetHTAt(i)->SetDetectorType(S->GetDetector()->GetType());
    } else {
      Event.GetHTAt(i)->SetDetectorType(0);
    }
  }

  if (HTs.size() > m_MaxNHits) {
    vector<unsigned int> NHitsPerDetector(MDDetector::c_MaxDetector, 0);

    for (unsigned int i = 0; i < Event.GetNHTs(); ++i) {
      if (Event.GetHTAt(i)->GetDetector() >= MDDetector::c_MinDetector &&
          Event.GetHTAt(i)->GetDetector() <= MDDetector::c_MaxDetector) {
        NHitsPerDetector[Event.GetHTAt(i)->GetDetector()-1]++;
      }
    }
    
    bool Exceed = false;
    for (int d = MDDetector::c_MinDetector; d <= MDDetector::c_MaxDetector; ++d) {
      if (NHitsPerDetector[d-1] > m_MaxNHits) {
        mout<<"Rejecting event "<<StartedEventID<<", because all detectors of type "<<d<<" have more than "
            <<m_MaxNHits<<" hits BEFORE discretization ("<<NHitsPerDetector[d-1]<<")"<<endl; 
        Exceed = true;
      }
    }

    if (Exceed == true) {
      ostringstream out;
      out<<"SE"<<endl;
      out<<"ID "<<m_EventID<<"  "<<StartedEventID<<endl;
      out<<"EX"<<endl;
      m_SimFile->AddText(out.str().c_str());    
      m_NExceedMaxNHits++;
      return true;
    }
  }  


  // Upgrade mother energy information:
  for (unsigned int i = 1; i < Event.GetNIAs(); ++i) {
    if (Event.GetIAAt(i)->GetProcess() == "COMP" || 
        Event.GetIAAt(i)->GetProcess() == "PHOT") {
      bool Found = false;
      for (unsigned int h = 0; h < Event.GetNHTs(); ++h) {
        massert(Event.GetHTAt(h)->GetNOrigins() == 1); // Critical!
        
        if (Event.GetHTAt(h)->GetPosition().AreEqual(Event.GetIAAt(i)->GetPosition(), 0.000001) &&
            Event.GetHTAt(h)->IsOrigin(1)) {
          double Energy;
          MSimIA* Top = 0;
          bool Predecessor;
          if (Event.GetIAAt(i-1)->GetOriginID() == Event.GetIAAt(i)->GetOriginID()) {
            Predecessor = true;
            Top = Event.GetIAAt(i-1);
          } else {
            Predecessor = false;
            Top = Event.GetIAAt(Event.GetIAAt(i)->GetOriginID()-1);
          }
          if (Predecessor == false) {
            Energy = Top->GetSecondaryEnergy() - 
              Event.GetIAAt(i)->GetSecondaryEnergy();
            //             cout<<Top->GetSecondaryEnergy()<<"-"
            //                 <<Event.GetIAAt(i)->GetSecondaryEnergy()<<"+"
            //                 <<Event.GetHTAt(h)->GetEnergy()<<endl;
          } else {
            Energy = Top->GetMotherEnergy() - 
              Event.GetIAAt(i)->GetSecondaryEnergy();
            //             cout<<Top->GetMotherEnergy()<<"-"
            //                 <<Event.GetIAAt(i)->GetSecondaryEnergy()<<"+"<<Event.GetHTAt(h)->GetEnergy()<<endl;            
          }
          Energy -= Event.GetHTAt(h)->GetEnergy();
          if (Energy < 0) Energy = 0;
          Event.GetIAAt(i)->SetMotherEnergy(Energy);
          Found = true;
        }
      }
      if (Found == false) {
        double Energy;
        MSimIA* Top = 0;
        bool Predecessor;
        if (Event.GetIAAt(i-1)->GetOriginID() == Event.GetIAAt(i)->GetOriginID()) { // ???
          Predecessor = true;
          Top = Event.GetIAAt(i-1);
        } else {
          Predecessor = false;
          Top = Event.GetIAAt(Event.GetIAAt(i)->GetOriginID()-1);
        }
        
        if (Predecessor == false) {
          Energy = Top->GetSecondaryEnergy() - 
            Event.GetIAAt(i)->GetSecondaryEnergy();
          //           cout<<Top->GetSecondaryEnergy()<<"-"
          //               <<Event.GetIAAt(i)->GetSecondaryEnergy()<<endl;
        } else {
          Energy = Top->GetMotherEnergy() - 
            Event.GetIAAt(i)->GetSecondaryEnergy();
          //           cout<<Top->GetMotherEnergy()<<"-"
          //               <<Event.GetIAAt(i)->GetSecondaryEnergy()<<endl;            
        }
        
        Event.GetIAAt(i)->SetMotherEnergy(Energy);
      } 
    } // COMP, PHOT loop
  } // IA loop

  // Discretize
  Event.Discretize(MDDetector::c_ACS);

  // Check for Veto
  bool Veto = false;
  double VetoEnergy = 0;
  if (m_IgnoreVeto == false) {
    mimp<<"Really stupid ACS veto activated..."<<show;
    for (unsigned int h = 0; h < Event.GetNHTs(); ++h) {
      if (Event.GetHTAt(h)->GetDetector() == MDDetector::c_ACS) {
        MDVolumeSequence* V = Event.GetHTAt(h)->GetVolumeSequence();
        massert(V->GetDetector() != 0);
        MVector Pos = V->GetPositionInDetector();
        Veto = V->GetDetector()->IsVeto(Pos, Event.GetHTAt(h)->GetEnergy());
        if (Veto == true) {
          VetoEnergy = Event.GetHTAt(h)->GetEnergy();
          break;
        }
      }
    }
  }
    
  if (m_DoDiscretization == true) {
    if (Veto == false) {
      for (int d = MDDetector::c_MinDetector; d <= MDDetector::c_MaxDetector; ++d) {
        if (d != MDDetector::c_ACS) {
          Event.Discretize(d);
        }
      }
    }
  }


  // Upgrade the information about energy deposits in passive material:
  // This includes guard rings!
//   if (Event.GetNIAs() > 0) {
//     double Energy = 0;
//     for (unsigned int i = 0; i < Event.GetNHTs(); ++i) {
//       Energy += Event.GetHTAt(i)->GetEnergy();
//     }
//     // Check all hits with origin 0:
//     double InitEnergy = 0;
//     for (unsigned int i = 0; i < Event.GetNIAs(); ++i) {
//       if (Event.GetIAAt(i)->GetOriginID() == 0) {
//         InitEnergy += Event.GetIAAt(i)->GetSecondaryEnergy();
//         if (Event.GetIAAt(i)->GetProcess() == "BET+" && 
//             Event.GetIAAt(i)->GetSecondaryParticleID() == 2) {
//           InitEnergy += 2*c_E0;
//         }
//       }
//     }
//     Energy = InitEnergy-Energy;
//     Energy -= Event.GetGuardringEnergy();
//     if (Energy < 0.001) Energy = 0;
    
//     Event.SetEnergyDepositNotSensitiveMaterial(Energy);
//   }

  // Store:
  if (Veto == true) {
    ostringstream out;
    out<<"SE"<<endl;
    out<<"ID "<<m_EventID<<"  "<<StartedEventID<<endl;
    out<<"VT"<<endl;
    m_SimFile->AddText(out.str().c_str());
    mdebug<<"Event "<<StartedEventID<<" has been vetoed with "
          <<VetoEnergy<<" keV deposit in one ACS detector"<<endl;
    m_NVetoedEvents++;
  } else if (m_IgnoreSingles == true && Event.GetNHTs() <= 1) {
    ostringstream out;
    out<<"SE"<<endl;
    out<<"ID "<<m_EventID<<"  "<<StartedEventID<<endl;
    out<<"SI"<<endl;
    m_SimFile->AddText(out.str().c_str());    
    m_NIgnoredSingles++;
  } else {
    MString Text = Event.ToSimString(MSimEvent::c_StoreSimulationInfoAll, 0);
    m_SimFile->AddText(Text);
    m_NStoredEvents++;
  }

  if (HTs.size() > HitsLimit) {
    cout<<"Finished discretization..."<<endl;
  }

  return true;
}

/******************************************************************************
 * Does the same as UHTOC in Geant3
 */
MString ConvertMGGPOD::GetProcessName(int Number)
{
  // Number = c1+(c2*256)+(c3*256^2)+(c3*256^3)

  int c;
  MString Case;

  for (int i = 3; i >= 0; --i) {
    c = Number % 256;
    Number -= c;
    Number /= 256;
    Case += char(c);
  }
 
  return Case;
}


/******************************************************************************
 * Do whatever analysis is necessary
 */
bool ConvertMGGPOD::Display()
{
  fitsfile* fptr = 0; // FITS file pointer, defined in fitsio.h
  char* val = 0;
  char value[1000];
  char nullstr[]="*";
  char keyword[FLEN_KEYWORD];
  char keyname[FLEN_KEYWORD];
  char colname[FLEN_VALUE];
  char coltype[FLEN_VALUE];
  char card[FLEN_CARD];   /* Standard string lengths defined in fitsio.h */
  int status = 0;   /*  CFITSIO status value MUST be initialized to zero!  */
  int hdupos, hdutype, nkeys, ncols, ii, anynul, bitpix, naxis, dispwidth[1000];
  int firstcol, lastcol = 0, linewidth;
  long naxes[10], jj, nrows;
  
  for (unsigned int f = 0; f < m_FileName.size(); ++f) {

    cout<<endl;
    cout<<endl;
    cout<<"Displaying content of fits file \""<<m_FileName[f]<<"\""<<endl;

    if (!fits_open_file(&fptr, m_FileName[f], READONLY, &status)) {

      // Get the current HDU position
      fits_get_hdu_num(fptr, &hdupos);

      // Main loop through each extension */
      for (; !status; hdupos++) {
      
        cout<<endl;
        cout<<endl;
        cout<<"Data of HDU "<<hdupos<<endl;

        fits_get_hdrspace(fptr, &nkeys, NULL, &status); // get # of keywords
        if (nkeys >= 1) {
          cout<<"Header:"<<endl;
          for (ii = 1; ii <= nkeys; ++ii) { /* Read and print each keywords */
            if (fits_read_record(fptr, ii, card, &status))break;
            cout<<"   "<<card<<endl;
          }
          cout<<"- end header"<<endl<<endl;
        } else {
          cout<<"No header found!"<<endl;
        }


        fits_get_hdu_type(fptr, &hdutype, &status);  /* Get the HDU type */
        if (hdutype == IMAGE_HDU) {   /* primary array or image HDU */
        
          cout<<"Structure:"<<endl;
          cout<<"   HDU type: image"<<endl;
          fits_get_img_param(fptr, 10, &bitpix, &naxis, naxes, &status);

          printf("   Array:  NAXIS = %d,  BITPIX = %d\n", naxis, bitpix);
          for (ii = 0; ii < naxis; ii++) {
            printf("      NAXIS%d = %ld\n",ii+1, naxes[ii]);
          }  
          cout<<"- end structure"<<endl;

          cout<<endl;
          cout<<endl;
          cout<<"Data:"<<endl;
          cout<<"   Error: images are not supported up to now!"<<endl;
          cout<<"- end data"<<endl;

        } else  if (hdutype == ASCII_TBL || hdutype == BINARY_TBL) { 

          cout<<"Structure:"<<endl;
          fits_get_num_rows(fptr, &nrows, &status);
          fits_get_num_cols(fptr, &ncols, &status);
        
          if (hdutype == ASCII_TBL) {
            printf("   HDU type: ASCII Table:  ");
          } else {
            printf("   HDU type: Binary Table:  ");
          }
        
          printf("   %d columns x %ld rows\n", ncols, nrows);
          printf("    COL NAME             FORMAT\n");
        
          for (ii = 1; ii <= ncols; ii++) {
            fits_make_keyn((char*) "TTYPE", ii, keyname, &status); /* make keyword */
            fits_read_key(fptr, TSTRING, keyname, colname, NULL, &status);
            fits_make_keyn((char*) "TFORM", ii, keyname, &status); /* make keyword */
            fits_read_key(fptr, TSTRING, keyname, coltype, NULL, &status);
          
            printf("    %3d %-16s %-16s\n", ii, colname, coltype);
          }
          cout<<"- end structure"<<endl;

          cout<<endl;
          cout<<endl;
          cout<<"Data:"<<endl;
          // find the number of columns that will fit within m_MaxLineWidth characters
          lastcol = 0;
          while(lastcol < ncols) {
            linewidth = 0;
            firstcol = lastcol+1;
            for (lastcol = firstcol; lastcol <= ncols; lastcol++) {
              fits_get_col_display_width
                (fptr, lastcol, &dispwidth[lastcol], &status);
              linewidth += dispwidth[lastcol] + 1;
              if (linewidth > m_MaxLineWidth) break;  
            }
            if (lastcol > firstcol)lastcol--;  // the last col didn't fit
          
            // print column names as column headers
            printf("\n    ");
            for (ii = firstcol; ii <= lastcol; ii++) {
              fits_make_keyn((char*) "TTYPE", ii, keyword, &status);
              fits_read_key(fptr, TSTRING, keyword, colname, NULL, &status);
              colname[dispwidth[ii]] = '\0';  // truncate long names
              printf("%*s ",dispwidth[ii], colname); 
            }
            printf("\n");  // terminate header line
          
            // print each column, row by row (there are faster ways to do this)
            val = value; 
            for (jj = 1; jj <= nrows && !status; jj++) {
              printf("%4d ", int(jj));
              for (ii = firstcol; ii <= lastcol; ii++) {
                // read value as a string, regardless of intrinsic datatype
                if (fits_read_col_str (fptr,ii,jj, 1, 1, nullstr,
                                       &val, &anynul, &status) ) {
                  break;  // jump out of loop on error 
                }
                printf("%-*s ",dispwidth[ii], value);
              }
              printf("\n");
            }
          }
          cout<<"- end data"<<endl;
        } else {
          cout<<"Error: Unsupported HDU type!"<<endl;
        }


        // try to move to next HDU
        fits_movrel_hdu(fptr, 1, NULL, &status);
      }
    
      // Reset after normal error
      if (status == END_OF_FILE)  status = 0;
      fits_close_file(fptr, &status);
    } else {
      cout<<"Unable to open fits file \""<<m_FileName[f]<<"\""<<endl;
      break;
    }
  
    if (status) {
      fits_report_error(stderr, status); // print any error message
      return false;
    }
  }

  return true;
}


/******************************************************************************/

ConvertMGGPOD* g_Prg = 0;

/******************************************************************************/


/******************************************************************************
 * Called when an interrupt signal is flagged
 * All catched signals lead to a well defined exit of the program
 */
void CatchSignal(int a)
{
  const int Max = 3;
  static int MaxNHits = 0;
  MaxNHits++;
  
  if (g_Prg != 0 && MaxNHits < Max) {
    cout<<"Catched signal Ctrl-C: Hit "<<Max-MaxNHits<<" times more for real abort!"<<endl;
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

  g_Prg = new ConvertMGGPOD();

  if (g_Prg->ParseCommandLine(argc, argv) == false) {
    cerr<<"Error during parsing of command line!"<<endl;
    return -1;
  }
  //  TApplication* ConvertMGGPODApp = 0;
  //  if (g_Prg->UseGui() == true) {
  //    ConvertMGGPODApp = new TApplication("ConvertMGGPODApp", 0, 0);
  //  }

  if (g_Prg->Analyze() == false) {
    cerr<<"Error during analysis or analysis interrupted!"<<endl;
    return -2;
  } 

  cout<<"Program exited normally!"<<endl;

  // delete ConvertMGGPODApp;

  return 0;
}

/*
 * Cosima: the end...
 ******************************************************************************/
