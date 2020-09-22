/* 
 * TraFitsConverter.cxx
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
#include <csignal>
#include <map>
#include <vector>
using namespace std;

// ROOT
#include <TApplication.h>
#include <TStyle.h>
#include <TH1.h>
#include <TCanvas.h>
#include <MString.h>

// HEAsoft
#include "fitsio.h"

// MEGAlib
#include "MGlobal.h"
#include "MStreams.h"
#include "MDGeometryQuest.h"
#include "MDDetector.h"
#include "MFileEventsSim.h"
#include "MDVolumeSequence.h"
#include "MPhysicalEvent.h"
#include "MComptonEvent.h"
#include "MPhotoEvent.h"
#include "MFileEventsTra.h"

/******************************************************************************/

class TraFitsConverter
{
public:
  //! Default constructor
  TraFitsConverter();
  //! Default destructor
  ~TraFitsConverter();
  
  //! Parse the command line
  bool ParseCommandLine(int argc, char** argv);
  //! Analyze whatever needs to be analyzed...
  bool ToFits();
  //! Analyze whatever needs to be analyzed...
  bool ToTra();
  //! Interrupt the analysis
  void Interrupt() { m_Interrupt = true; }

private:
  //! True, if the analysis needs to be interrupted
  bool m_Interrupt;

  //! Tra file name
  MString m_FileName;
  //! Geometry file name
  MString m_GeometryFileName;
  //! Observation object
  MString m_Object;
  
  
};

/******************************************************************************/


/******************************************************************************
 * Default constructor
 */
TraFitsConverter::TraFitsConverter() : m_Interrupt(false)
{
  // Intentionally left blank
}


/******************************************************************************
 * Default destructor
 */
TraFitsConverter::~TraFitsConverter()
{
  // Intentionally left blank
}


/******************************************************************************
 * Parse the command line
 */
bool TraFitsConverter::ParseCommandLine(int argc, char** argv)
{
  ostringstream Usage;
  Usage<<endl;
  Usage<<"  Usage: TraFitsConverter <options>"<<endl;
  Usage<<"    General options:"<<endl;
  Usage<<"         -f:   tra file name"<<endl;
  Usage<<"         -g:   geometry file name"<<endl;
  Usage<<"         -o:   object"<<endl;
  Usage<<"         -h:   print this help"<<endl;
  Usage<<endl;

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
    if (Option == "-f" || Option == "-o") {
      if (!((argc > i+1) && argv[i+1][0] != '-')){
        cout<<"Error: Option "<<argv[i][1]<<" needs a second argument!"<<endl;
        cout<<Usage.str()<<endl;
        return false;
      }
    } 
    // Multiple arguments_
    //else if (Option == "-??") {
    //  if (!((argc > i+2) && argv[i+1][0] != '-' && argv[i+2][0] != '-')){
    //    cout<<"Error: Option "<<argv[i][1]<<" needs two arguments!"<<endl;
    //    cout<<Usage.str()<<endl;
    //    return false;
    //  }
    //}

    // Then fulfill the options:
    if (Option == "-f") {
      m_FileName = argv[++i];
      cout<<"Accepting file name: "<<m_FileName<<endl;
    } else if (Option == "-g") {
      m_GeometryFileName = argv[++i];
      cout<<"Accepting file name: "<<m_GeometryFileName<<endl;
    } else if (Option == "-o") {
      m_Object = argv[++i];
      cout<<"Accepting object: "<<m_Object<<endl;
    } else {
      cout<<"Error: Unknown option \""<<Option<<"\"!"<<endl;
      cout<<Usage.str()<<endl;
      return false;
    }
  }

  if (m_FileName == "") {
    cout<<"Error: Need a tra file name!"<<endl;
    cout<<Usage.str()<<endl;
    return false;
  }

  if (m_GeometryFileName == "") {
    cout<<"Error: Need a geometry file name!"<<endl;
    cout<<Usage.str()<<endl;
    return false;
  }

  if (m_FileName.EndsWith(".tra") == true ||  m_FileName.EndsWith(".tra.gz") == true) {
    ToFits();
  } else if (m_FileName.EndsWith(".fits") == true ||  m_FileName.EndsWith(".fits.gz") == true) {
    ToTra();
  } else {
    cout<<"Error: Need a tra or fits file name, not a "<<m_FileName<<" file "<<endl;
    cout<<Usage.str()<<endl;
    return false;  }
  
  return true;
}


/******************************************************************************
 * Do whatever analysis is necessary
 */
bool TraFitsConverter::ToTra()
{
  // Load geometry:
  MDGeometryQuest* Geometry = new MDGeometryQuest();
  if (Geometry->ScanSetupFile(m_GeometryFileName) == true) {
    cout<<"Geometry "<<Geometry->GetName()<<" loaded!"<<endl;
  } else {
    cout<<"Unable to load geometry "<<Geometry->GetName()<<" - Aborting!"<<endl;
    return false;
  }  
 
  MString FileName = m_FileName;
  if (MFile::Exists(FileName) == false) {
    FileName += ".gz";
    if (MFile::Exists(FileName) == false) {
      cout<<"The file \""<<FileName<<"\" does not exists!"<<endl;
      return false;
    }
  }
 
  cout<<"Reading .tra.fits from file "<<FileName<<endl;
  
  // Section 1:
  // initialize the files
  int status = 0;
  char value[1000];
  char templt[100];
  
  int anynul;
  
  double nulldbl = 0.0;
  double valdbl = 0;
  
  unsigned char nullbyte = 0.0;
  unsigned char valbyte = 0;
  
  long nulllong = 0;
  long vallong = 0;
  
  short nullshort = 0;
  short valshort = 0;
  
  int nullint = 0;
  int valint = 0;
  
  fitsfile* EventFile = 0;
  if (fits_open_file(&EventFile, FileName.Data(), READONLY, &status)) {
    fits_get_errstatus(status, value);
    cout<<"Unable to open Attitude file \""<<FileName<<"\": "<<status<<":"<<value<<endl;
    return false;
  }
  //cout<<"Opened "<<FileName<<endl;
  
  // Move to the second hdu
  fits_movabs_hdu(EventFile, 2, NULL, &status);
  
  // Get the number of columns
  int NColumns = 0;
  map<TString, int> Columns;
  fits_get_num_cols(EventFile, &NColumns, &status);
  for (int c = 1; c <= NColumns; ++c) {
    int col;
    snprintf(templt, sizeof(templt), "%d", c);
    fits_get_colname(EventFile, CASEINSEN, templt, value, &col, &status);
    Columns[value] = c;
    //cout<<value<<endl;
  }
  //cout<<"N columns: "<<NColumns<<endl;
  
  // Get the number of rows:
  long NRows = 0;
  fits_get_num_rows(EventFile, &NRows, &status);
  //cout<<"Rows: "<<NRows<<endl;
  
  // Open a tra file for writting:
  MFileEventsTra Out;
  Out.Open(FileName + ".tra", MFile::c_Write);
  Out.SetGeometryFileName(m_GeometryFileName);
  Out.WriteHeader();
  
  // Loop over the events and all columns and generate MComptonEvent's
  for (long r = 1; r <= NRows; ++r) {
    
    if (fits_read_col_byt(EventFile, Columns["TYPE"], r, 1, 1, nullbyte, &valbyte, &anynul, &status) ) {
      cout<<"Column read (TYPE) failed!"<<endl;
      break;
    }
    int Type = valbyte;
    
    if (Type == 1) {
      MPhotoEvent* C = new MPhotoEvent(); 
      
      double Energy = 0;
      MString Name = MString("ENERGY1");
      if (fits_read_col_dbl(EventFile, Columns[Name.Data()], r, 1, 1, nulldbl, &Energy, &anynul, &status) ) {
        cout<<"Column read ("<<Name<<") failed!"<<endl;
        break;
      }
      
      double PosX = 0;
      Name = MString("POSX1");
      if (fits_read_col_dbl(EventFile, Columns[Name.Data()], r, 1, 1, nulldbl, &PosX, &anynul, &status) ) {
        cout<<"Column read ("<<Name<<") failed!"<<endl;
        break;
      }
      
      double PosY = 0;
      Name = MString("POSY1");
      if (fits_read_col_dbl(EventFile, Columns[Name.Data()], r, 1, 1, nulldbl, &PosY, &anynul, &status) ) {
        cout<<"Column read ("<<Name<<") failed!"<<endl;
        break;
      }
      
      double PosZ = 0;
      Name = MString("POSZ1");
      if (fits_read_col_dbl(EventFile, Columns[Name.Data()], r, 1, 1, nulldbl, &PosZ, &anynul, &status) ) {
        cout<<"Column read ("<<Name<<") failed!"<<endl;
        break;
      }
      
      MPhysicalEventHit H;
      H.Set(MVector(PosX, PosY, PosZ), MVector(0, 0, 0), Energy, 0, 0, 0);
      
      C->Assimilate(MVector(PosX, PosY, PosZ), Energy, 0);
      C->AddHit(H);
      
      
      
      // Then read the rest of the data
      if (fits_read_col_lng(EventFile, Columns["ID"], r, 1, 1, nulllong, &vallong, &anynul, &status) ) {
        cout<<"Column read (ID) failed!"<<endl;
        break;
      }
      C->SetId(vallong);
      
      if (fits_read_col_lng(EventFile, Columns["TIME_S"], r, 1, 1, nulllong, &vallong, &anynul, &status) ) {
        cout<<"Column read (TIME_S) failed!"<<endl;
        break;
      }
      long Seconds = vallong;
      if (fits_read_col_int(EventFile, Columns["TIME_NS"], r, 1, 1, nullint, &valint, &anynul, &status) ) {
        cout<<"Column read (TIME_NS) failed!"<<endl;
        break;
      }
      int NanoSeconds = valint;
      C->SetTime(MTime(Seconds, (long) NanoSeconds));
      
      if (fits_read_col_dbl(EventFile, Columns["TIME"], r, 1, 1, nulldbl, &valdbl, &anynul, &status) ) {
        cout<<"Column read (TIME) failed!"<<endl;
        break;
      }
      //C->SetTime(MTime(valdbl));
      
      double XLat = 0;
      double XLong = 0;
      double ZLat = 0;
      double ZLong = 0;
      if (fits_read_col_dbl(EventFile, Columns["GX_LAT"], r, 1, 1, nulldbl, &XLat, &anynul, &status) ) {
        cout<<"Column read (GX_LAT) failed!"<<endl;
        break;
      }
      if (fits_read_col_dbl(EventFile, Columns["GX_LONG"], r, 1, 1, nulldbl, &XLong, &anynul, &status) ) {
        cout<<"Column read (GX_LONG) failed!"<<endl;
        break;
      }
      
      if (fits_read_col_dbl(EventFile, Columns["GZ_LAT"], r, 1, 1, nulldbl, &ZLat, &anynul, &status) ) {
        cout<<"Column read (GZ_LAT) failed!"<<endl;
        break;
      }
      if (fits_read_col_dbl(EventFile, Columns["GZ_LONG"], r, 1, 1, nulldbl, &ZLong, &anynul, &status) ) {
        cout<<"Column read (GZ_LONG) failed!"<<endl;
        break;
      }
      if (XLat != 0.0 || XLong != 0 || ZLat != 0 || ZLong != 0) {
        C->SetGalacticPointingXAxis(XLong, XLat);
        C->SetGalacticPointingZAxis(ZLong, ZLat);
      }
      
      if (fits_read_col_dbl(EventFile, Columns["HX_LAT"], r, 1, 1, nulldbl, &XLat, &anynul, &status) ) {
        cout<<"Column read (HX_LAT) failed!"<<endl;
        break;
      }
      if (fits_read_col_dbl(EventFile, Columns["HX_LONG"], r, 1, 1, nulldbl, &XLong, &anynul, &status) ) {
        cout<<"Column read (HX_LONG) failed!"<<endl;
        break;
      }
      
      if (fits_read_col_dbl(EventFile, Columns["HZ_LAT"], r, 1, 1, nulldbl, &ZLat, &anynul, &status) ) {
        cout<<"Column read (HZ_LAT) failed!"<<endl;
        break;
      }
      if (fits_read_col_dbl(EventFile, Columns["HZ_LONG"], r, 1, 1, nulldbl, &ZLong, &anynul, &status) ) {
        cout<<"Column read (HZ_LONG) failed!"<<endl;
        break;
      }
      if (XLat != 0.0 || XLong != 0 || ZLat != 0 || ZLong != 0) {
        C->SetHorizonPointingXAxis(XLong, XLat);
        C->SetHorizonPointingZAxis(ZLong, ZLat);
      }

      
      Out.AddEvent(C);
      
      
    } else if (Type == 2) {
      MComptonEvent* C = new MComptonEvent(); 


      vector<MPhysicalEventHit> Hits;
      
      long ID;
      if (fits_read_col_lng(EventFile, Columns["ID"], r, 1, 1, nulllong, &ID, &anynul, &status) ) {
        cout<<"Column read (ID) failed!"<<endl;
        break;
      }
      //cout<<"ID: "<<ID<<endl;
      
      
      if (fits_read_col_sht(EventFile, Columns["NIA"], r, 1, 1, nullshort, &valshort, &anynul, &status) ) {
        cout<<"Column read (NIA) failed!"<<endl;
        break;
      }
      unsigned int NIAs = valshort;
      
      MString Name;
      for (unsigned int i = 1; i <= NIAs; ++i) {
        double Energy = 0;
        Name = MString("ENERGY") + i;
        if (fits_read_col_dbl(EventFile, Columns[Name.Data()], r, 1, 1, nulldbl, &Energy, &anynul, &status) ) {
          cout<<"Column read ("<<Name<<") failed!"<<endl;
          break;
        }
        
        double PosX = 0;
        Name = MString("POSX") + i;
        if (fits_read_col_dbl(EventFile, Columns[Name.Data()], r, 1, 1, nulldbl, &PosX, &anynul, &status) ) {
          cout<<"Column read ("<<Name<<") failed!"<<endl;
          break;
        }
        
        double PosY = 0;
        Name = MString("POSY") + i;
        if (fits_read_col_dbl(EventFile, Columns[Name.Data()], r, 1, 1, nulldbl, &PosY, &anynul, &status) ) {
          cout<<"Column read ("<<Name<<") failed!"<<endl;
          break;
        }
        
        double PosZ = 0;
        Name = MString("POSZ") + i;
        if (fits_read_col_dbl(EventFile, Columns[Name.Data()], r, 1, 1, nulldbl, &PosZ, &anynul, &status) ) {
          cout<<"Column read ("<<Name<<") failed!"<<endl;
          break;
        }
        
        double dEnergy = 0;
        Name = MString("DENERGY") + i;
        if (fits_read_col_dbl(EventFile, Columns[Name.Data()], r, 1, 1, nulldbl, &dEnergy, &anynul, &status) ) {
          cout<<"Column read ("<<Name<<") failed!"<<endl;
          break;
        }
        
        double dPosX = 0;
        Name = MString("DPOSX") + i;
        if (fits_read_col_dbl(EventFile, Columns[Name.Data()], r, 1, 1, nulldbl, &dPosX, &anynul, &status) ) {
          cout<<"Column read ("<<Name<<") failed!"<<endl;
          break;
        }
        
        double dPosY = 0;
        Name = MString("DPOSY") + i;
        if (fits_read_col_dbl(EventFile, Columns[Name.Data()], r, 1, 1, nulldbl, &dPosY, &anynul, &status) ) {
          cout<<"Column read ("<<Name<<") failed!"<<endl;
          break;
        }
        
        double dPosZ = 0;
        Name = MString("DPOSZ") + i;
        if (fits_read_col_dbl(EventFile, Columns[Name.Data()], r, 1, 1, nulldbl, &dPosZ, &anynul, &status) ) {
          cout<<"Column read ("<<Name<<") failed!"<<endl;
          break;
        }
       
        MPhysicalEventHit H;
        H.Set(MVector(PosX, PosY, PosZ), MVector(dPosX, dPosY, dPosZ), Energy, dEnergy, 0, 1E15);

        Hits.push_back(H);      
        //cout<<"Added: "<<Hits.size()<<endl;
      }
      
      // Asssimilate the hits
      //cout<<"Final: "<<Hits.size()<<endl;
      C->Assimilate(Hits);
      
      
      // Then read the rest of the data
      if (fits_read_col_lng(EventFile, Columns["ID"], r, 1, 1, nulllong, &vallong, &anynul, &status) ) {
        cout<<"Column read (ID) failed!"<<endl;
        break;
      }
      C->SetId(vallong);
      
      
      if (fits_read_col_lng(EventFile, Columns["TIME_S"], r, 1, 1, nulllong, &vallong, &anynul, &status) ) {
        cout<<"Column read (TIME_S) failed!"<<endl;
        break;
      }
      long Seconds = vallong;
      if (fits_read_col_int(EventFile, Columns["TIME_NS"], r, 1, 1, nullint, &valint, &anynul, &status) ) {
        cout<<"Column read (TIME_NS) failed!"<<endl;
        break;
      }
      int NanoSeconds = valint;
      C->SetTime(MTime(Seconds, (long) NanoSeconds));
      
      //if (fits_read_col_dbl(EventFile, Columns["TIME"], r, 1, 1, nulldbl, &valdbl, &anynul, &status) ) {
      //  cout<<"Column read (TIME) failed!"<<endl;
      //  break;
      //}
      //C->SetTime(MTime(valdbl));
      
      double XLat = 0;
      double XLong = 0;
      double ZLat = 0;
      double ZLong = 0;
      if (fits_read_col_dbl(EventFile, Columns["GX_LAT"], r, 1, 1, nulldbl, &XLat, &anynul, &status) ) {
        cout<<"Column read (GX_LAT) failed!"<<endl;
        break;
      }
      if (fits_read_col_dbl(EventFile, Columns["GX_LONG"], r, 1, 1, nulldbl, &XLong, &anynul, &status) ) {
        cout<<"Column read (GX_LONG) failed!"<<endl;
        break;
      }
      
      if (fits_read_col_dbl(EventFile, Columns["GZ_LAT"], r, 1, 1, nulldbl, &ZLat, &anynul, &status) ) {
        cout<<"Column read (GZ_LAT) failed!"<<endl;
        break;
      }
      if (fits_read_col_dbl(EventFile, Columns["GZ_LONG"], r, 1, 1, nulldbl, &ZLong, &anynul, &status) ) {
        cout<<"Column read (GZ_LONG) failed!"<<endl;
        break;
      }
      if (XLat != 0.0 || XLong != 0 || ZLat != 0 || ZLong != 0) {
        C->SetGalacticPointingXAxis(XLong, XLat);
        C->SetGalacticPointingZAxis(ZLong, ZLat);
      }
      
      if (fits_read_col_dbl(EventFile, Columns["HX_LAT"], r, 1, 1, nulldbl, &XLat, &anynul, &status) ) {
        cout<<"Column read (HX_LAT) failed!"<<endl;
        break;
      }
      if (fits_read_col_dbl(EventFile, Columns["HX_LONG"], r, 1, 1, nulldbl, &XLong, &anynul, &status) ) {
        cout<<"Column read (HX_LONG) failed!"<<endl;
        break;
      }
      
      if (fits_read_col_dbl(EventFile, Columns["HZ_LAT"], r, 1, 1, nulldbl, &ZLat, &anynul, &status) ) {
        cout<<"Column read (HZ_LAT) failed!"<<endl;
        break;
      }
      if (fits_read_col_dbl(EventFile, Columns["HZ_LONG"], r, 1, 1, nulldbl, &ZLong, &anynul, &status) ) {
        cout<<"Column read (HZ_LONG) failed!"<<endl;
        break;
      }
      if (XLat != 0.0 || XLong != 0 || ZLat != 0 || ZLong != 0) {
        C->SetHorizonPointingXAxis(XLong, XLat);
        C->SetHorizonPointingZAxis(ZLong, ZLat);
      }
      
      
      
      double Quality1 = 0;
      if (fits_read_col_dbl(EventFile, Columns["QUAL1"], r, 1, 1, nulldbl, &Quality1, &anynul, &status) ) {
        cout<<"Column read ("<<Name<<") failed!"<<endl;
        break;
      }
      C->SetComptonQualityFactor1(Quality1);
      
      double Quality2 = 0;
      if (fits_read_col_dbl(EventFile, Columns["QUAL2"], r, 1, 1, nulldbl, &Quality2, &anynul, &status) ) {
        cout<<"Column read ("<<Name<<") failed!"<<endl;
        break;
      }
      C->SetComptonQualityFactor2(Quality2);
      
      Out.AddEvent(C);
    }
  }
 
  Out.Close();
 
  return true;
}



/******************************************************************************
 * Do whatever analysis is necessary
 */
bool TraFitsConverter::ToFits()
{
    // Load geometry:
  MDGeometryQuest* Geometry = new MDGeometryQuest();
  if (Geometry->ScanSetupFile(m_GeometryFileName) == true) {
    cout<<"Geometry "<<Geometry->GetName()<<" loaded!"<<endl;
  } else {
    cout<<"Unable to load geometry "<<Geometry->GetName()<<" - Aborting!"<<endl;
    return false;
  }  
    
  MFileEventsTra* Reader = new MFileEventsTra();
  if (Reader->Open(m_FileName) == false) {
    mout<<"Unable to open file "<<m_FileName<<". Aborting!"<<endl;
    return false;
  }
  Reader->ShowProgress();

  vector<long> ID;
  vector<unsigned char> TYPE;
  
  vector<double> TIME;
  vector<long> TIME_S;
  vector<unsigned int> TIME_NS;
  
  vector<double> GX_LAT;
  vector<double> GX_LONG;
  vector<double> GZ_LAT;
  vector<double> GZ_LONG;
  vector<double> HX_LAT;
  vector<double> HX_LONG;
  vector<double> HZ_LAT;
  vector<double> HZ_LONG;

  vector<short> NIA;   
  
  vector<double> ENERGY1;
  vector<double> POSX1;
  vector<double> POSY1;
  vector<double> POSZ1;
  vector<double> DENERGY1;
  vector<double> DPOSX1;
  vector<double> DPOSY1;
  vector<double> DPOSZ1;
  vector<double> ENERGY2;
  vector<double> POSX2;
  vector<double> POSY2;
  vector<double> POSZ2;
  vector<double> DENERGY2;
  vector<double> DPOSX2;
  vector<double> DPOSY2;
  vector<double> DPOSZ2;
  vector<double> ENERGY3;
  vector<double> POSX3;
  vector<double> POSY3;
  vector<double> POSZ3;
  vector<double> DENERGY3;
  vector<double> DPOSX3;
  vector<double> DPOSY3;
  vector<double> DPOSZ3;
  vector<double> ENERGY4;
  vector<double> POSX4;
  vector<double> POSY4;
  vector<double> POSZ4;
  vector<double> DENERGY4;
  vector<double> DPOSX4;
  vector<double> DPOSY4;
  vector<double> DPOSZ4;
  vector<double> ENERGY5;
  vector<double> POSX5;
  vector<double> POSY5;
  vector<double> POSZ5;
  vector<double> DENERGY5;
  vector<double> DPOSX5;
  vector<double> DPOSY5;
  vector<double> DPOSZ5;
  vector<double> ENERGY6;
  vector<double> POSX6;
  vector<double> POSY6;
  vector<double> POSZ6;
  vector<double> DENERGY6;
  vector<double> DPOSX6;
  vector<double> DPOSY6;
  vector<double> DPOSZ6;
  vector<double> ENERGY7;
  vector<double> POSX7;
  vector<double> POSY7;
  vector<double> POSZ7;
  vector<double> DENERGY7;
  vector<double> DPOSX7;
  vector<double> DPOSY7;
  vector<double> DPOSZ7;
  
  vector<double> QUAL1;
  vector<double> QUAL2;
  
  MPhysicalEvent* Event;
  MComptonEvent* C = nullptr;
  MPhotoEvent* S = nullptr;
  
  bool IsFirst = true;
  MTime Start;
  MTime Stop;
  
  while ((Event = Reader->GetNextEvent()) != 0) { 
    // Hitting Ctrl-C raises this flag
    if (m_Interrupt == true) return false;
    
    if (IsFirst == true) {
      Start = Event->GetTime();
      IsFirst = false;
    } else {
      Stop = Event->GetTime();
    }
    
    
    if (Event->GetType() == MPhysicalEvent::c_Compton) {
      C = dynamic_cast<MComptonEvent*>(Event);
      ID.push_back(C->GetId());
      TYPE.push_back(2);
      
      TIME.push_back(C->GetTime().GetAsSeconds());
      TIME_S.push_back((long) C->GetTime().GetAsSeconds());
      TIME_NS.push_back(C->GetTime().GetNanoSeconds());
      
      if (C->HasGalacticPointing() == true) {
        GX_LAT.push_back(C->GetGalacticPointingXAxisLatitude());
        GX_LONG.push_back(C->GetGalacticPointingXAxisLongitude()); 
        GZ_LAT.push_back(C->GetGalacticPointingZAxisLatitude());
        GZ_LONG.push_back(C->GetGalacticPointingZAxisLongitude());
      } else {
        GX_LAT.push_back(0);
        GX_LONG.push_back(0); 
        GZ_LAT.push_back(0);
        GZ_LONG.push_back(0);
      }
      
      if (C->HasHorizonPointing() == true) {
        HX_LAT.push_back(C->GetHorizonPointingXAxisLatitude());
        HX_LONG.push_back(C->GetHorizonPointingXAxisLongitude()); 
        HZ_LAT.push_back(C->GetHorizonPointingZAxisLatitude());
        HZ_LONG.push_back(C->GetHorizonPointingZAxisLongitude());
      } else {
        HX_LAT.push_back(0);
        HX_LONG.push_back(0); 
        HZ_LAT.push_back(0);
        HZ_LONG.push_back(0);
      }
      
      NIA.push_back(C->GetNHits());
      
      if (C->GetNHits() >= 1) {
        const MPhysicalEventHit& Hit = C->GetHit(0);
        
        ENERGY1.push_back(Hit.GetEnergy());
        POSX1.push_back(Hit.GetPosition().X());
        POSY1.push_back(Hit.GetPosition().Y());
        POSZ1.push_back(Hit.GetPosition().Z());
        
        DENERGY1.push_back(Hit.GetEnergyUncertainty());
        DPOSX1.push_back(Hit.GetPositionUncertainty().X());
        DPOSY1.push_back(Hit.GetPositionUncertainty().Y());
        DPOSZ1.push_back(Hit.GetPositionUncertainty().Z());
      } else {
        ENERGY1.push_back(0);
        POSX1.push_back(0);
        POSY1.push_back(0);
        POSZ1.push_back(0);
        
        DENERGY1.push_back(0);
        DPOSX1.push_back(0);
        DPOSY1.push_back(0);
        DPOSZ1.push_back(0);
      }
      
      if (C->GetNHits() >= 2) {
        const MPhysicalEventHit& Hit = C->GetHit(1);
        
        ENERGY2.push_back(Hit.GetEnergy());
        POSX2.push_back(Hit.GetPosition().X());
        POSY2.push_back(Hit.GetPosition().Y());
        POSZ2.push_back(Hit.GetPosition().Z());
        
        DENERGY2.push_back(Hit.GetEnergyUncertainty());
        DPOSX2.push_back(Hit.GetPositionUncertainty().X());
        DPOSY2.push_back(Hit.GetPositionUncertainty().Y());
        DPOSZ2.push_back(Hit.GetPositionUncertainty().Z());
      } else {
        ENERGY2.push_back(0);
        POSX2.push_back(0);
        POSY2.push_back(0);
        POSZ2.push_back(0);
        
        DENERGY2.push_back(0);
        DPOSX2.push_back(0);
        DPOSY2.push_back(0);
        DPOSZ2.push_back(0);
      }
      
      if (C->GetNHits() >= 3) {
        const MPhysicalEventHit& Hit = C->GetHit(2);
        
        ENERGY3.push_back(Hit.GetEnergy());
        POSX3.push_back(Hit.GetPosition().X());
        POSY3.push_back(Hit.GetPosition().Y());
        POSZ3.push_back(Hit.GetPosition().Z());
        
        DENERGY3.push_back(Hit.GetEnergyUncertainty());
        DPOSX3.push_back(Hit.GetPositionUncertainty().X());
        DPOSY3.push_back(Hit.GetPositionUncertainty().Y());
        DPOSZ3.push_back(Hit.GetPositionUncertainty().Z());
      } else {
        ENERGY3.push_back(0);
        POSX3.push_back(0);
        POSY3.push_back(0);
        POSZ3.push_back(0);
        
        DENERGY3.push_back(0);
        DPOSX3.push_back(0);
        DPOSY3.push_back(0);
        DPOSZ3.push_back(0);
      }
      
      if (C->GetNHits() >= 4) {
        const MPhysicalEventHit& Hit = C->GetHit(3);
        
        ENERGY4.push_back(Hit.GetEnergy());
        POSX4.push_back(Hit.GetPosition().X());
        POSY4.push_back(Hit.GetPosition().Y());
        POSZ4.push_back(Hit.GetPosition().Z());
        
        DENERGY4.push_back(Hit.GetEnergyUncertainty());
        DPOSX4.push_back(Hit.GetPositionUncertainty().X());
        DPOSY4.push_back(Hit.GetPositionUncertainty().Y());
        DPOSZ4.push_back(Hit.GetPositionUncertainty().Z());
      } else {
        ENERGY4.push_back(0);
        POSX4.push_back(0);
        POSY4.push_back(0);
        POSZ4.push_back(0);
        
        DENERGY4.push_back(0);
        DPOSX4.push_back(0);
        DPOSY4.push_back(0);
        DPOSZ4.push_back(0);
      }
      
      if (C->GetNHits() >= 5) {
        const MPhysicalEventHit& Hit = C->GetHit(4);
        
        ENERGY5.push_back(Hit.GetEnergy());
        POSX5.push_back(Hit.GetPosition().X());
        POSY5.push_back(Hit.GetPosition().Y());
        POSZ5.push_back(Hit.GetPosition().Z());
        
        DENERGY5.push_back(Hit.GetEnergyUncertainty());
        DPOSX5.push_back(Hit.GetPositionUncertainty().X());
        DPOSY5.push_back(Hit.GetPositionUncertainty().Y());
        DPOSZ5.push_back(Hit.GetPositionUncertainty().Z());
      } else {
        ENERGY5.push_back(0);
        POSX5.push_back(0);
        POSY5.push_back(0);
        POSZ5.push_back(0);
        
        DENERGY5.push_back(0);
        DPOSX5.push_back(0);
        DPOSY5.push_back(0);
        DPOSZ5.push_back(0);
      }
      
      if (C->GetNHits() >= 6) {
        const MPhysicalEventHit& Hit = C->GetHit(5);
        
        ENERGY6.push_back(Hit.GetEnergy());
        POSX6.push_back(Hit.GetPosition().X());
        POSY6.push_back(Hit.GetPosition().Y());
        POSZ6.push_back(Hit.GetPosition().Z());
        
        DENERGY6.push_back(Hit.GetEnergyUncertainty());
        DPOSX6.push_back(Hit.GetPositionUncertainty().X());
        DPOSY6.push_back(Hit.GetPositionUncertainty().Y());
        DPOSZ6.push_back(Hit.GetPositionUncertainty().Z());
      } else {
        ENERGY6.push_back(0);
        POSX6.push_back(0);
        POSY6.push_back(0);
        POSZ6.push_back(0);
        
        DENERGY6.push_back(0);
        DPOSX6.push_back(0);
        DPOSY6.push_back(0);
        DPOSZ6.push_back(0);
      }
      
      if (C->GetNHits() >= 7) {
        const MPhysicalEventHit& Hit = C->GetHit(6);
        
        ENERGY7.push_back(Hit.GetEnergy());
        POSX7.push_back(Hit.GetPosition().X());
        POSY7.push_back(Hit.GetPosition().Y());
        POSZ7.push_back(Hit.GetPosition().Z());
        
        DENERGY7.push_back(Hit.GetEnergyUncertainty());
        DPOSX7.push_back(Hit.GetPositionUncertainty().X());
        DPOSY7.push_back(Hit.GetPositionUncertainty().Y());
        DPOSZ7.push_back(Hit.GetPositionUncertainty().Z());
      } else {
        ENERGY7.push_back(0);
        POSX7.push_back(0);
        POSY7.push_back(0);
        POSZ7.push_back(0);
        
        DENERGY7.push_back(0);
        DPOSX7.push_back(0);
        DPOSY7.push_back(0);
        DPOSZ7.push_back(0);
      }
      
      QUAL1.push_back(C->ComptonQualityFactor1());
      QUAL2.push_back(C->ComptonQualityFactor2());
      
      
    } else if (Event->GetType() == MPhysicalEvent::c_Photo) {
      S = dynamic_cast<MPhotoEvent*>(Event);
      ID.push_back(S->GetId());
      TYPE.push_back(1);
      
      TIME.push_back(S->GetTime().GetAsSeconds());
      TIME_S.push_back((long) S->GetTime().GetAsSeconds());
      TIME_NS.push_back(S->GetTime().GetNanoSeconds());
      
      if (S->HasGalacticPointing() == true) {
        GX_LAT.push_back(S->GetGalacticPointingXAxisLatitude());
        GX_LONG.push_back(S->GetGalacticPointingXAxisLongitude()); 
        GZ_LAT.push_back(S->GetGalacticPointingZAxisLatitude());
        GZ_LONG.push_back(S->GetGalacticPointingZAxisLongitude());
      } else {
        GX_LAT.push_back(0);
        GX_LONG.push_back(0); 
        GZ_LAT.push_back(0);
        GZ_LONG.push_back(0);
      }
      
      if (S->HasHorizonPointing() == true) {
        HX_LAT.push_back(S->GetHorizonPointingXAxisLatitude());
        HX_LONG.push_back(S->GetHorizonPointingXAxisLongitude()); 
        HZ_LAT.push_back(S->GetHorizonPointingZAxisLatitude());
        HZ_LONG.push_back(S->GetHorizonPointingZAxisLongitude());
      } else {
        HX_LAT.push_back(0);
        HX_LONG.push_back(0); 
        HZ_LAT.push_back(0);
        HZ_LONG.push_back(0);
      }
      
      NIA.push_back(1);      

      ENERGY1.push_back(S->GetEnergy());
      POSX1.push_back(S->GetPosition().X());
      POSY1.push_back(S->GetPosition().Y());
      POSZ1.push_back(S->GetPosition().Z());
        
      DENERGY1.push_back(0);
      DPOSX1.push_back(0);
      DPOSY1.push_back(0);
      DPOSZ1.push_back(0);
      
      ENERGY2.push_back(0);
      POSX2.push_back(0);
      POSY2.push_back(0);
      POSZ2.push_back(0);
      
      DENERGY2.push_back(0);
      DPOSX2.push_back(0);
      DPOSY2.push_back(0);
      DPOSZ2.push_back(0);

      ENERGY3.push_back(0);
      POSX3.push_back(0);
      POSY3.push_back(0);
      POSZ3.push_back(0);
      
      DENERGY3.push_back(0);
      DPOSX3.push_back(0);
      DPOSY3.push_back(0);
      DPOSZ3.push_back(0);

      ENERGY4.push_back(0);
      POSX4.push_back(0);
      POSY4.push_back(0);
      POSZ4.push_back(0);
      
      DENERGY4.push_back(0);
      DPOSX4.push_back(0);
      DPOSY4.push_back(0);
      DPOSZ4.push_back(0);

      ENERGY5.push_back(0);
      POSX5.push_back(0);
      POSY5.push_back(0);
      POSZ5.push_back(0);
      
      DENERGY5.push_back(0);
      DPOSX5.push_back(0);
      DPOSY5.push_back(0);
      DPOSZ5.push_back(0);

      ENERGY6.push_back(0);
      POSX6.push_back(0);
      POSY6.push_back(0);
      POSZ6.push_back(0);
      
      DENERGY6.push_back(0);
      DPOSX6.push_back(0);
      DPOSY6.push_back(0);
      DPOSZ6.push_back(0);

      ENERGY7.push_back(0);
      POSX7.push_back(0);
      POSY7.push_back(0);
      POSZ7.push_back(0);
      
      DENERGY7.push_back(0);
      DPOSX7.push_back(0);
      DPOSY7.push_back(0);
      DPOSZ7.push_back(0);
      
      QUAL1.push_back(0);
      QUAL2.push_back(0);
    }  

    delete Event;
  }
  
  
  MString FitsFileName = "!" + m_FileName + ".fits"; 
  int FileVersion = 1;
  
  int HDUType = 0;
  int Status = 0;
  fitsfile* m_File = nullptr;
  
  fits_create_file(&m_File, FitsFileName, &Status);
  if (Status != 0) {
    cout<<"Error in creating file: "<<FitsFileName<<show;
    m_File = nullptr;
    return false;
  }
  
  //! create binary table extension
  char ExtensionName[] = "EVENTS";
  long nrow = 0;
  int tfield = 3 + 8 + 7*8 + 2;
  /*
  char* ttype[] = {"ID","TYPE","TIME_S","TIME_NS","GX_LAT","GX_LONG","GZ_LAT","GZ_LONG","HX_LAT","HX_LONG","HZ_LAT","HZ_LONG","ENERGY1","POSX1","POSY1","POSZ1","DENERGY1","DPOSX1","DPOSY1","DPOSZ1","ENERGY2","POSX2","POSY2","POSZ2","DENERGY2","DPOSX2","DPOSY2","DPOSZ2","ENERGY3","POSX3","POSY3","POSZ3","DENERGY3","DPOSX3","DPOSY3","DPOSZ3","ENERGY4","POSX4","POSY4","POSZ4","DENERGY4","DPOSX4","DPOSY4","DPOSZ4","ENERGY5","POSX5","POSY5","POSZ5","DENERGY5","DPOSX5","DPOSY5","DPOSZ5","ENERGY6","POSX6","POSY6","POSZ6","DENERGY6","DPOSX6","DPOSY6","DPOSZ6","ENERGY7","POSX7","POSY7","POSZ7","DENERGY7","DPOSX7","DPOSY7","DPOSZ7","QUAL1","QUAL2"};
  char* tform[] = {"1k","1b"  ,"1v"    ,"1v"     ,"1d"    ,"1d"     ,"1d"    ,"1d"     ,"1d"    ,"1d"     ,"1d"    ,"1d"     ,"1d"     ,"1d"   ,"1d"   ,"1d"   ,"1d"      ,"1d"    ,"1d"    ,"1d"    ,"1d"     ,"1d"   ,"1d"   ,"1d"   ,"1d"      ,"1d"    ,"1d"    ,"1d"    ,"1d"     ,"1d"   ,"1d"   ,"1d"   ,"1d"      ,"1d"    ,"1d"    ,"1d"    ,"1d"     ,"1d"   ,"1d"   ,"1d"   ,"1d"      ,"1d"    ,"1d"    ,"1d"    ,"1d"     ,"1d"   ,"1d"   ,"1d"   ,"1d"      ,"1d"    ,"1d"    ,"1d"    ,"1d"     ,"1d"   ,"1d"   ,"1d"   ,"1d"      ,"1d"    ,"1d"    ,"1d"    ,"1d"     ,"1d"   ,"1d"   ,"1d"   ,"1d"      ,"1d"    ,"1d"    ,"1d"    ,"1d"   ,"1d"};
  char* tunit[] = {"NA","NA"  ,"s"     ,"ns"     ,"deg"   ,"deg"    ,"deg"   ,"deg"    ,"deg"   ,"deg"    ,"deg"   ,"deg"    ,"keV"    ,"cm"   ,"cm"   ,"cm"   ,"keV"     ,"cm"    ,"cm"    ,"cm"    ,"keV"    ,"cm"   ,"cm"   ,"cm"   ,"keV"     ,"cm"    ,"cm"    ,"cm"    ,"keV"    ,"cm"   ,"cm"   ,"cm"   ,"keV"     ,"cm"    ,"cm"    ,"cm"    ,"keV"    ,"cm"   ,"cm"   ,"cm"   ,"keV"     ,"cm"    ,"cm"    ,"cm"    ,"keV"    ,"cm"   ,"cm"   ,"cm"   ,"keV"     ,"cm"    ,"cm"    ,"cm"    ,"keV"    ,"cm"   ,"cm"   ,"cm"   ,"keV"     ,"cm"    ,"cm"    ,"cm"    ,"keV"    ,"cm"   ,"cm"   ,"cm"   ,"keV"     ,"cm"    ,"cm"    ,"cm"    ,"NA"   ,"NA"};
  char* comments[] = {
    "ID of the event",
    "Type: 1: single, 2: Compton, 3: Pair"
    
  }
  */
  vector<const char*> cttype;
  vector<const char*> ctform;
  vector<const char*> ctunit;
  vector<const char*> ctcomment;
  
  cttype.push_back("ID");       ctform.push_back("1k");   ctunit.push_back("NA");   ctcomment.push_back("ID of the event");
  cttype.push_back("TYPE");     ctform.push_back("1b");   ctunit.push_back("NA");   ctcomment.push_back("Type: 1: single, 2: Compton, 3: Pair");
  cttype.push_back("TIME");     ctform.push_back("1d");   ctunit.push_back("s");    ctcomment.push_back("Time the event was measured - seconds");
  cttype.push_back("TIME_S");   ctform.push_back("1k");   ctunit.push_back("s");    ctcomment.push_back("Time the event was measured - seconds");
  cttype.push_back("TIME_NS");  ctform.push_back("1v");   ctunit.push_back("ns");   ctcomment.push_back("Time the event was measured - nano-seconds");
  cttype.push_back("GX_LAT");   ctform.push_back("1d");   ctunit.push_back("deg");  ctcomment.push_back("Pointing of telescope X axis in Galactic coordinate system -latitude");
  cttype.push_back("GX_LONG");  ctform.push_back("1d");   ctunit.push_back("deg");  ctcomment.push_back("Pointing of telescope X axis in Galactic coordinate system -longitude");
  cttype.push_back("GZ_LAT");   ctform.push_back("1d");   ctunit.push_back("deg");  ctcomment.push_back("Pointing of telescope Z axis in Galactic coordinate system - latitude");
  cttype.push_back("GZ_LONG");  ctform.push_back("1d");   ctunit.push_back("deg");  ctcomment.push_back("Pointing of telescope Z axis in Galactic coordinate system - longitude");
  cttype.push_back("HX_LAT");   ctform.push_back("1d");   ctunit.push_back("deg");  ctcomment.push_back("Pointing of telescope X axis in Horizontal coordinate system - latitude");
  cttype.push_back("HX_LONG");  ctform.push_back("1d");   ctunit.push_back("deg");  ctcomment.push_back("Pointing of telescope X axis in Horizontal coordinate system - longitude");
  cttype.push_back("HZ_LAT");   ctform.push_back("1d");   ctunit.push_back("deg");  ctcomment.push_back("Pointing of telescope Z axis in Horizontal coordinate system - latitude");
  cttype.push_back("HZ_LONG");  ctform.push_back("1d");   ctunit.push_back("deg");  ctcomment.push_back("Pointing of telescope Z axis in Horizontal coordinate system - longitude");
  
  cttype.push_back("NIA");      ctform.push_back("1i");   ctunit.push_back("keV");  ctcomment.push_back("Number of interactions");
  
  cttype.push_back("ENERGY1");  ctform.push_back("1d");   ctunit.push_back("keV");  ctcomment.push_back("Energy deposited in interaction 1");
  cttype.push_back("POSX1");    ctform.push_back("1d");   ctunit.push_back("cm");   ctcomment.push_back("X position of interaction 1");
  cttype.push_back("POSY1");    ctform.push_back("1d");   ctunit.push_back("cm");   ctcomment.push_back("Y position of interaction 1");
  cttype.push_back("POSZ1");    ctform.push_back("1d");   ctunit.push_back("cm");   ctcomment.push_back("Z position of interaction 1");
  cttype.push_back("DENERGY1"); ctform.push_back("1d");   ctunit.push_back("keV");  ctcomment.push_back("Uncertainty of energy deposited in interaction 1");
  cttype.push_back("DPOSX1");   ctform.push_back("1d");   ctunit.push_back("cm");   ctcomment.push_back("Uncertainty of X position of interaction 1");
  cttype.push_back("DPOSY1");   ctform.push_back("1d");   ctunit.push_back("cm");   ctcomment.push_back("Uncertainty of Y position of interaction 1");
  cttype.push_back("DPOSZ1");   ctform.push_back("1d");   ctunit.push_back("cm");   ctcomment.push_back("Uncertainty of Z position of interaction 1");
  
  cttype.push_back("ENERGY2");  ctform.push_back("1d");   ctunit.push_back("keV");  ctcomment.push_back("Energy deposited in interaction 2");
  cttype.push_back("POSX2");    ctform.push_back("1d");   ctunit.push_back("cm");   ctcomment.push_back("X position of interaction 2");
  cttype.push_back("POSY2");    ctform.push_back("1d");   ctunit.push_back("cm");   ctcomment.push_back("Y position of interaction 2");
  cttype.push_back("POSZ2");    ctform.push_back("1d");   ctunit.push_back("cm");   ctcomment.push_back("Z position of interaction 2");
  cttype.push_back("DENERGY2"); ctform.push_back("1d");   ctunit.push_back("keV");  ctcomment.push_back("Uncertainty of energy deposited in interaction 2");
  cttype.push_back("DPOSX2");   ctform.push_back("1d");   ctunit.push_back("cm");   ctcomment.push_back("Uncertainty of X position of interaction 2");
  cttype.push_back("DPOSY2");   ctform.push_back("1d");   ctunit.push_back("cm");   ctcomment.push_back("Uncertainty of Y position of interaction 2");
  cttype.push_back("DPOSZ2");   ctform.push_back("1d");   ctunit.push_back("cm");   ctcomment.push_back("Uncertainty of Z position of interaction 2");
  
  cttype.push_back("ENERGY3");  ctform.push_back("1d");   ctunit.push_back("keV");  ctcomment.push_back("Energy deposited in interaction 3");
  cttype.push_back("POSX3");    ctform.push_back("1d");   ctunit.push_back("cm");   ctcomment.push_back("X position of interaction 3");
  cttype.push_back("POSY3");    ctform.push_back("1d");   ctunit.push_back("cm");   ctcomment.push_back("Y position of interaction 3");
  cttype.push_back("POSZ3");    ctform.push_back("1d");   ctunit.push_back("cm");   ctcomment.push_back("Z position of interaction 3");
  cttype.push_back("DENERGY3"); ctform.push_back("1d");   ctunit.push_back("keV");  ctcomment.push_back("Uncertainty of energy deposited in interaction 3");
  cttype.push_back("DPOSX3");   ctform.push_back("1d");   ctunit.push_back("cm");   ctcomment.push_back("Uncertainty of X position of interaction 3");
  cttype.push_back("DPOSY3");   ctform.push_back("1d");   ctunit.push_back("cm");   ctcomment.push_back("Uncertainty of Y position of interaction 3");
  cttype.push_back("DPOSZ3");   ctform.push_back("1d");   ctunit.push_back("cm");   ctcomment.push_back("Uncertainty of Z position of interaction 3");
  
  cttype.push_back("ENERGY4");  ctform.push_back("1d");   ctunit.push_back("keV");  ctcomment.push_back("Energy deposited in interaction 4");
  cttype.push_back("POSX4");    ctform.push_back("1d");   ctunit.push_back("cm");   ctcomment.push_back("X position of interaction 4");
  cttype.push_back("POSY4");    ctform.push_back("1d");   ctunit.push_back("cm");   ctcomment.push_back("Y position of interaction 4");
  cttype.push_back("POSZ4");    ctform.push_back("1d");   ctunit.push_back("cm");   ctcomment.push_back("Z position of interaction 4");
  cttype.push_back("DENERGY4"); ctform.push_back("1d");   ctunit.push_back("keV");  ctcomment.push_back("Uncertainty of energy deposited in interaction 4");
  cttype.push_back("DPOSX4");   ctform.push_back("1d");   ctunit.push_back("cm");   ctcomment.push_back("Uncertainty of X position of interaction 4");
  cttype.push_back("DPOSY4");   ctform.push_back("1d");   ctunit.push_back("cm");   ctcomment.push_back("Uncertainty of Y position of interaction 4");
  cttype.push_back("DPOSZ4");   ctform.push_back("1d");   ctunit.push_back("cm");   ctcomment.push_back("Uncertainty of Z position of interaction 4");
  
  cttype.push_back("ENERGY5");  ctform.push_back("1d");   ctunit.push_back("keV");  ctcomment.push_back("Energy deposited in interaction 5");
  cttype.push_back("POSX5");    ctform.push_back("1d");   ctunit.push_back("cm");   ctcomment.push_back("X position of interaction 5");
  cttype.push_back("POSY5");    ctform.push_back("1d");   ctunit.push_back("cm");   ctcomment.push_back("Y position of interaction 5");
  cttype.push_back("POSZ5");    ctform.push_back("1d");   ctunit.push_back("cm");   ctcomment.push_back("Z position of interaction 5");
  cttype.push_back("DENERGY5"); ctform.push_back("1d");   ctunit.push_back("keV");  ctcomment.push_back("Uncertainty of energy deposited in interaction 5");
  cttype.push_back("DPOSX5");   ctform.push_back("1d");   ctunit.push_back("cm");   ctcomment.push_back("Uncertainty of X position of interaction 5");
  cttype.push_back("DPOSY5");   ctform.push_back("1d");   ctunit.push_back("cm");   ctcomment.push_back("Uncertainty of Y position of interaction 5");
  cttype.push_back("DPOSZ5");   ctform.push_back("1d");   ctunit.push_back("cm");   ctcomment.push_back("Uncertainty of Z position of interaction 5");
  
  cttype.push_back("ENERGY6");  ctform.push_back("1d");   ctunit.push_back("keV");  ctcomment.push_back("Energy deposited in interaction 6");
  cttype.push_back("POSX6");    ctform.push_back("1d");   ctunit.push_back("cm");   ctcomment.push_back("X position of interaction 6");
  cttype.push_back("POSY6");    ctform.push_back("1d");   ctunit.push_back("cm");   ctcomment.push_back("Y position of interaction 6");
  cttype.push_back("POSZ6");    ctform.push_back("1d");   ctunit.push_back("cm");   ctcomment.push_back("Z position of interaction 6");
  cttype.push_back("DENERGY6"); ctform.push_back("1d");   ctunit.push_back("keV");  ctcomment.push_back("Uncertainty of energy deposited in interaction 6");
  cttype.push_back("DPOSX6");   ctform.push_back("1d");   ctunit.push_back("cm");   ctcomment.push_back("Uncertainty of X position of interaction 6");
  cttype.push_back("DPOSY6");   ctform.push_back("1d");   ctunit.push_back("cm");   ctcomment.push_back("Uncertainty of Y position of interaction 6");
  cttype.push_back("DPOSZ6");   ctform.push_back("1d");   ctunit.push_back("cm");   ctcomment.push_back("Uncertainty of Z position of interaction 6");
  
  cttype.push_back("ENERGY7");  ctform.push_back("1d");   ctunit.push_back("keV");  ctcomment.push_back("Energy deposited in interaction 7");
  cttype.push_back("POSX7");    ctform.push_back("1d");   ctunit.push_back("cm");   ctcomment.push_back("X position of interaction 7");
  cttype.push_back("POSY7");    ctform.push_back("1d");   ctunit.push_back("cm");   ctcomment.push_back("Y position of interaction 7");
  cttype.push_back("POSZ7");    ctform.push_back("1d");   ctunit.push_back("cm");   ctcomment.push_back("Z position of interaction 7");
  cttype.push_back("DENERGY7"); ctform.push_back("1d");   ctunit.push_back("keV");  ctcomment.push_back("Uncertainty of energy deposited in interaction 7");
  cttype.push_back("DPOSX7");   ctform.push_back("1d");   ctunit.push_back("cm");   ctcomment.push_back("Uncertainty of X position of interaction 7");
  cttype.push_back("DPOSY7");   ctform.push_back("1d");   ctunit.push_back("cm");   ctcomment.push_back("Uncertainty of Y position of interaction 7");
  cttype.push_back("DPOSZ7");   ctform.push_back("1d");   ctunit.push_back("cm");   ctcomment.push_back("Uncertainty of Z position of interaction 7");
  
  cttype.push_back("QUAL1"); ctform.push_back("1d"); ctunit.push_back("NA"); ctcomment.push_back("Primary quality factor of this event");
  cttype.push_back("QUAL2"); ctform.push_back("1d"); ctunit.push_back("NA"); ctcomment.push_back("Secondary quality factor of this event");
  
  
  
  vector<char*> ttype;
  for (auto l: cttype) ttype.push_back(const_cast<char*>(l));
  vector<char*> tform;
  for (auto l: ctform) tform.push_back(const_cast<char*>(l));
  vector<char*> tunit;
  for (auto l: ctunit) tunit.push_back(const_cast<char*>(l));
  vector<char*> tcomment;
  for (auto l: ctcomment) tcomment.push_back(const_cast<char*>(l));
                                       
  tfield = ttype.size();
  
  fits_create_tbl(m_File, BINARY_TBL, nrow, tfield, &ttype[0], &tform[0], &tunit[0], ExtensionName, &Status); 
  if (Status != 0) {
    mgui<<"Error in creating extension: "<<ExtensionName<<" Error: "<<Status<<endl;
    m_File = 0;
    return false;
  }
  
  fits_movabs_hdu(m_File, 2, &HDUType, &Status);
  if (Status != 0) {
    mgui<<"Error in moving to HDU 2. Error: "<<Status<<endl;
    m_File = 0;
    return false;
  }
  
  for (unsigned int i = 0; i < tcomment.size(); ++i) {
    MString T("TTYPE");
    T += i+1;
    fits_modify_comment(m_File, T.Data(), tcomment[i], &Status);
    if (Status != 0) {
      mgui<<"Error in modifying comment for "<<T.Data()<<". Error: "<<Status<<endl;
      m_File = 0;
      return false;
    }
  }
  
  fits_write_key(m_File, TSTRING, "HDUCLASS",  const_cast<char*>("OGIP"), " ", &Status);  
  fits_write_key(m_File, TSTRING, "HDUCLAS1",  const_cast<char*>("EVENTS"), " ", &Status);  
  fits_write_key(m_File, TSTRING, "HDUVERS",  const_cast<char*>("1.2.1"), " ", &Status);    
  
  fits_write_key(m_File, TSTRING, "TELESCOP", const_cast<char*>("COSI-2016"), "Compton Spectrometer and Imager - 2016 version", &Status);
  fits_write_key(m_File, TSTRING, "INSTRUME", const_cast<char*>("GeD"), "COSI Germanium detectors", &Status);
  fits_write_key(m_File, TSTRING, "CREATOR",  const_cast<char*>("MEGAlib"), "This file was created with MEGAlib", &Status);  
  fits_write_key(m_File, TSTRING, "CREATVER", const_cast<char*>(Reader->GetMEGAlibVersion().Data()), "MEHGAlib version", &Status); 
  
  fits_write_key(m_File, TSTRING, "DATTYPE",  const_cast<char*>("TRA"), "Data corresponds to MEGAlib tra file", &Status);  
  fits_write_key(m_File, TSTRING, "DATLEVEL",  const_cast<char*>("3"), "Data is COSI LEVEL 3 data", &Status);  
  fits_write_key(m_File, TINT, "LEVELVER",  &FileVersion, "Version of this Level 3 fits data format", &Status);  
  
  MTime Now;
  fits_write_key(m_File, TSTRING, "DATE",  const_cast<char*>(Now.GetFitsDateString().Data()), "Date that FITS file was created", &Status);  

  fits_write_key(m_File, TSTRING, "DATE-OBS",  const_cast<char*>(Start.GetFitsDateString().Data()), "Start date of the observation (dd/mm/yy)", &Status);  
  fits_write_key(m_File, TSTRING, "TIME-OBS",  const_cast<char*>(Start.GetFitsTimeString().Data()), "Start time of the observation (hh:mm:ss)", &Status);  
  
  fits_write_key(m_File, TSTRING, "DATE-END",  const_cast<char*>(Stop.GetFitsDateString().Data()), "End date of the observation (dd/mm/yy)", &Status);  
  fits_write_key(m_File, TSTRING, "TIME-END",  const_cast<char*>(Stop.GetFitsTimeString().Data()), "End time of the observation (hh:mm:ss)", &Status);  
  
  fits_write_key(m_File, TSTRING, "OBSMODE",  const_cast<char*>("SCAN"), "COSI was in zenith pointing scanning mode", &Status);  
  if (m_Object != "") {
    fits_write_key(m_File, TSTRING, "OBJECT",  const_cast<char*>(m_Object.Data()), "Name of the observed object", &Status);  
  } else {
    fits_write_key(m_File, TSTRING, "OBJECT",  const_cast<char*>("N/A"), "General all-sky observation", &Status);  
  }
  
  MTime Diff = Stop;
  Diff -= Start;
  fits_write_key(m_File, TSTRING, "TSTART",  const_cast<char*>(Start.GetLongIntsString().Data()), "mission time of the start of the observation ", &Status); 
  fits_write_key(m_File, TSTRING, "TSTOP",  const_cast<char*>(Stop.GetLongIntsString().Data()), "mission time of the end of the observation", &Status); 
  //fits_write_key(m_File, TSTRING, "MJDREF",  const_cast<char*>(), "MJD corresponding to SC clock start (1994.0)", &Status); 
  fits_write_key(m_File, TSTRING, "TIMEUNIT",  const_cast<char*>("s"), "units for the time related keywords", &Status); 
  fits_write_key(m_File, TSTRING, "TIMESYS",  const_cast<char*>("UTC"), "type of time system that is used ", &Status); 
  fits_write_key(m_File, TSTRING, "TIMEREF",  const_cast<char*>("LOCAL"), "reference frame used for times", &Status); 
  fits_write_key(m_File, TSTRING, "TASSIGN",  const_cast<char*>("COSI"), "Location where time assignment performed", &Status); 
  fits_write_key(m_File, TSTRING, "TELAPSE",  const_cast<char*>(Diff.GetLongIntsString().Data()), "otal elapsed time of the observation", &Status); 
  fits_write_key(m_File, TSTRING, "ONTIME",  const_cast<char*>(Diff.GetLongIntsString().Data()), "active integration time", &Status); 
  fits_write_key(m_File, TSTRING, "LIVETIME",  const_cast<char*>(Diff.GetLongIntsString().Data()), "ONTIME corrected for deadtime effects", &Status); 
  fits_write_key(m_File, TSTRING, "EXPOSURE",  const_cast<char*>(Diff.GetLongIntsString().Data()), "Total time used to calc the correct countrate", &Status); 
  
  //fits_write_key(m_File, TSTRING, "GEOMETRY",  const_cast<char*>(Reader->GetGeometryFileName().Data()), "The geometry used to create this file", &Status); 
  
  
  
  
  char Words[30];
  
  int Column = 0;
  

  fits_write_col(m_File, TLONG, ++Column, 1, 1, ID.size(), &ID[0], &Status);
  if (Status != 0) {
    fits_get_errstatus(Status, Words);
    cerr << "Error : fits_write_col('ID') failed (" << Words << ")" << endl;
    fits_close_file(m_File, &Status);
    m_File = 0;
    return false;
  }
  fits_write_col(m_File, TBYTE, ++Column, 1, 1, TYPE.size(), &TYPE[0], &Status);
  if (Status != 0) {
    fits_get_errstatus(Status, Words);
    cerr << "Error : fits_write_col('TYPE') failed (" << Words << ")" << endl;
    fits_close_file(m_File, &Status);
    m_File = 0;
    return false;
  }
  fits_write_col(m_File, TDOUBLE, ++Column, 1, 1, TIME.size(), &TIME[0], &Status);
  if (Status != 0) {
    fits_get_errstatus(Status, Words);
    cerr << "Error : fits_write_col('TIME') failed (" << Words << ")" << endl;
    fits_close_file(m_File, &Status);
    m_File = 0;
    return false;
  }
  fits_write_col(m_File, TLONG, ++Column, 1, 1, TIME_S.size(), &TIME_S[0], &Status);
  if (Status != 0) {
    fits_get_errstatus(Status, Words);
    cerr << "Error : fits_write_col('TIME_S') failed (" << Words << ")" << endl;
    fits_close_file(m_File, &Status);
    m_File = 0;
    return false;
  }
  fits_write_col(m_File, TINT, ++Column, 1, 1, TIME_NS.size(), &TIME_NS[0], &Status);
  if (Status != 0) {
    fits_get_errstatus(Status, Words);
    cerr << "Error : fits_write_col('TIME_NS') failed (" << Words << ")" << endl;
    fits_close_file(m_File, &Status);
    m_File = 0;
    return false;
  }
  fits_write_col(m_File, TDOUBLE, ++Column, 1, 1, GX_LAT.size(), &GX_LAT[0], &Status);
  if (Status != 0) {
    fits_get_errstatus(Status, Words);
    cerr << "Error : fits_write_col('GX_LAT') failed (" << Words << ")" << endl;
    fits_close_file(m_File, &Status);
    m_File = 0;
    return false;
  }
  fits_write_col(m_File, TDOUBLE, ++Column, 1, 1, GX_LONG.size(), &GX_LONG[0], &Status);
  if (Status != 0) {
    fits_get_errstatus(Status, Words);
    cerr << "Error : fits_write_col('GX_LONG') failed (" << Words << ")" << endl;
    fits_close_file(m_File, &Status);
    m_File = 0;
    return false;
  }
  fits_write_col(m_File, TDOUBLE, ++Column, 1, 1, GZ_LAT.size(), &GZ_LAT[0], &Status);
  if (Status != 0) {
    fits_get_errstatus(Status, Words);
    cerr << "Error : fits_write_col('GZ_LAT') failed (" << Words << ")" << endl;
    fits_close_file(m_File, &Status);
    m_File = 0;
    return false;
  }
  fits_write_col(m_File, TDOUBLE, ++Column, 1, 1, GZ_LONG.size(), &GZ_LONG[0], &Status);
  if (Status != 0) {
    fits_get_errstatus(Status, Words);
    cerr << "Error : fits_write_col('GZ_LONG') failed (" << Words << ")" << endl;
    fits_close_file(m_File, &Status);
    m_File = 0;
    return false;
  }
  
  fits_write_col(m_File, TDOUBLE, ++Column, 1, 1, HX_LAT.size(), &HX_LAT[0], &Status);
  if (Status != 0) {
    fits_get_errstatus(Status, Words);
    cerr << "Error : fits_write_col('HX_LAT') failed (" << Words << ")" << endl;
    fits_close_file(m_File, &Status);
    m_File = 0;
    return false;
  }
  fits_write_col(m_File, TDOUBLE, ++Column, 1, 1, HX_LONG.size(), &HX_LONG[0], &Status);
  if (Status != 0) {
    fits_get_errstatus(Status, Words);
    cerr << "Error : fits_write_col('HX_LONG') failed (" << Words << ")" << endl;
    fits_close_file(m_File, &Status);
    m_File = 0;
    return false;
  }
  fits_write_col(m_File, TDOUBLE, ++Column, 1, 1, HZ_LAT.size(), &HZ_LAT[0], &Status);
  if (Status != 0) {
    fits_get_errstatus(Status, Words);
    cerr << "Error : fits_write_col('HZ_LAT') failed (" << Words << ")" << endl;
    fits_close_file(m_File, &Status);
    m_File = 0;
    return false;
  }
  fits_write_col(m_File, TDOUBLE, ++Column, 1, 1, HZ_LONG.size(), &HZ_LONG[0], &Status);
  if (Status != 0) {
    fits_get_errstatus(Status, Words);
    cerr << "Error : fits_write_col('HZ_LONG') failed (" << Words << ")" << endl;
    fits_close_file(m_File, &Status);
    m_File = 0;
    return false;
  }
  
  
  fits_write_col(m_File, TSHORT, ++Column, 1, 1, NIA.size(), &NIA[0], &Status);
  if (Status != 0) {
    fits_get_errstatus(Status, Words);
    cerr << "Error : fits_write_col('NIA') failed (" << Words << ")" << endl;
    fits_close_file(m_File, &Status);
    m_File = 0;
    return false;
  }
  
  
  fits_write_col(m_File, TDOUBLE, ++Column, 1, 1, ENERGY1.size(), &ENERGY1[0], &Status);
  if (Status != 0) {
    fits_get_errstatus(Status, Words);
    cerr << "Error : fits_write_col('ENERGY1') failed (" << Words << ")" << endl;
    fits_close_file(m_File, &Status);
    m_File = 0;
    return false;
  }
  fits_write_col(m_File, TDOUBLE, ++Column, 1, 1, POSX1.size(), &POSX1[0], &Status);
  if (Status != 0) {
    fits_get_errstatus(Status, Words);
    cerr << "Error : fits_write_col('POSX1') failed (" << Words << ")" << endl;
    fits_close_file(m_File, &Status);
    m_File = 0;
    return false;
  }
  fits_write_col(m_File, TDOUBLE, ++Column, 1, 1, POSY1.size(), &POSY1[0], &Status);
  if (Status != 0) {
    fits_get_errstatus(Status, Words);
    cerr << "Error : fits_write_col('POSY1') failed (" << Words << ")" << endl;
    fits_close_file(m_File, &Status);
    m_File = 0;
    return false;
  }
  fits_write_col(m_File, TDOUBLE, ++Column, 1, 1, POSZ1.size(), &POSZ1[0], &Status);
  if (Status != 0) {
    fits_get_errstatus(Status, Words);
    cerr << "Error : fits_write_col('POSZ1') failed (" << Words << ")" << endl;
    fits_close_file(m_File, &Status);
    m_File = 0;
    return false;
  }
  fits_write_col(m_File, TDOUBLE, ++Column, 1, 1, DENERGY1.size(), &DENERGY1[0], &Status);
  if (Status != 0) {
    fits_get_errstatus(Status, Words);
    cerr << "Error : fits_write_col('DENERGY1') failed (" << Words << ")" << endl;
    fits_close_file(m_File, &Status);
    m_File = 0;
    return false;
  }
  fits_write_col(m_File, TDOUBLE, ++Column, 1, 1, DPOSX1.size(), &DPOSX1[0], &Status);
  if (Status != 0) {
    fits_get_errstatus(Status, Words);
    cerr << "Error : fits_write_col('DPOSX1') failed (" << Words << ")" << endl;
    fits_close_file(m_File, &Status);
    m_File = 0;
    return false;
  }
  fits_write_col(m_File, TDOUBLE, ++Column, 1, 1, DPOSY1.size(), &DPOSY1[0], &Status);
  if (Status != 0) {
    fits_get_errstatus(Status, Words);
    cerr << "Error : fits_write_col('DPOSY1') failed (" << Words << ")" << endl;
    fits_close_file(m_File, &Status);
    m_File = 0;
    return false;
  }
  fits_write_col(m_File, TDOUBLE, ++Column, 1, 1, DPOSZ1.size(), &DPOSZ1[0], &Status);
  if (Status != 0) {
    fits_get_errstatus(Status, Words);
    cerr << "Error : fits_write_col('DPOSZ1') failed (" << Words << ")" << endl;
    fits_close_file(m_File, &Status);
    m_File = 0;
    return false;
  }
  
  
  fits_write_col(m_File, TDOUBLE, ++Column, 1, 1, ENERGY2.size(), &ENERGY2[0], &Status);
  if (Status != 0) {
    fits_get_errstatus(Status, Words);
    cerr << "Error : fits_write_col('ENERGY2') failed (" << Words << ")" << endl;
    fits_close_file(m_File, &Status);
    m_File = 0;
    return false;
  }
  fits_write_col(m_File, TDOUBLE, ++Column, 1, 1, POSX2.size(), &POSX2[0], &Status);
  if (Status != 0) {
    fits_get_errstatus(Status, Words);
    cerr << "Error : fits_write_col('POSX2') failed (" << Words << ")" << endl;
    fits_close_file(m_File, &Status);
    m_File = 0;
    return false;
  }
  fits_write_col(m_File, TDOUBLE, ++Column, 1, 1, POSY2.size(), &POSY2[0], &Status);
  if (Status != 0) {
    fits_get_errstatus(Status, Words);
    cerr << "Error : fits_write_col('POSY2') failed (" << Words << ")" << endl;
    fits_close_file(m_File, &Status);
    m_File = 0;
    return false;
  }
  fits_write_col(m_File, TDOUBLE, ++Column, 1, 1, POSZ2.size(), &POSZ2[0], &Status);
  if (Status != 0) {
    fits_get_errstatus(Status, Words);
    cerr << "Error : fits_write_col('POSZ2') failed (" << Words << ")" << endl;
    fits_close_file(m_File, &Status);
    m_File = 0;
    return false;
  }
  fits_write_col(m_File, TDOUBLE, ++Column, 1, 1, DENERGY2.size(), &DENERGY2[0], &Status);
  if (Status != 0) {
    fits_get_errstatus(Status, Words);
    cerr << "Error : fits_write_col('DENERGY2') failed (" << Words << ")" << endl;
    fits_close_file(m_File, &Status);
    m_File = 0;
    return false;
  }
  fits_write_col(m_File, TDOUBLE, ++Column, 1, 1, DPOSX2.size(), &DPOSX2[0], &Status);
  if (Status != 0) {
    fits_get_errstatus(Status, Words);
    cerr << "Error : fits_write_col('DPOSX2') failed (" << Words << ")" << endl;
    fits_close_file(m_File, &Status);
    m_File = 0;
    return false;
  }
  fits_write_col(m_File, TDOUBLE, ++Column, 1, 1, DPOSY2.size(), &DPOSY2[0], &Status);
  if (Status != 0) {
    fits_get_errstatus(Status, Words);
    cerr << "Error : fits_write_col('DPOSY2') failed (" << Words << ")" << endl;
    fits_close_file(m_File, &Status);
    m_File = 0;
    return false;
  }
  fits_write_col(m_File, TDOUBLE, ++Column, 1, 1, DPOSZ2.size(), &DPOSZ2[0], &Status);
  if (Status != 0) {
    fits_get_errstatus(Status, Words);
    cerr << "Error : fits_write_col('DPOSZ2') failed (" << Words << ")" << endl;
    fits_close_file(m_File, &Status);
    m_File = 0;
    return false;
  }
  
  
  fits_write_col(m_File, TDOUBLE, ++Column, 1, 1, ENERGY3.size(), &ENERGY3[0], &Status);
  if (Status != 0) {
    fits_get_errstatus(Status, Words);
    cerr << "Error : fits_write_col('ENERGY3') failed (" << Words << ")" << endl;
    fits_close_file(m_File, &Status);
    m_File = 0;
    return false;
  }
  fits_write_col(m_File, TDOUBLE, ++Column, 1, 1, POSX3.size(), &POSX3[0], &Status);
  if (Status != 0) {
    fits_get_errstatus(Status, Words);
    cerr << "Error : fits_write_col('POSX3') failed (" << Words << ")" << endl;
    fits_close_file(m_File, &Status);
    m_File = 0;
    return false;
  }
  fits_write_col(m_File, TDOUBLE, ++Column, 1, 1, POSY3.size(), &POSY3[0], &Status);
  if (Status != 0) {
    fits_get_errstatus(Status, Words);
    cerr << "Error : fits_write_col('POSY3') failed (" << Words << ")" << endl;
    fits_close_file(m_File, &Status);
    m_File = 0;
    return false;
  }
  fits_write_col(m_File, TDOUBLE, ++Column, 1, 1, POSZ3.size(), &POSZ3[0], &Status);
  if (Status != 0) {
    fits_get_errstatus(Status, Words);
    cerr << "Error : fits_write_col('POSZ3') failed (" << Words << ")" << endl;
    fits_close_file(m_File, &Status);
    m_File = 0;
    return false;
  }
  fits_write_col(m_File, TDOUBLE, ++Column, 1, 1, DENERGY3.size(), &DENERGY3[0], &Status);
  if (Status != 0) {
    fits_get_errstatus(Status, Words);
    cerr << "Error : fits_write_col('DENERGY3') failed (" << Words << ")" << endl;
    fits_close_file(m_File, &Status);
    m_File = 0;
    return false;
  }
  fits_write_col(m_File, TDOUBLE, ++Column, 1, 1, DPOSX3.size(), &DPOSX3[0], &Status);
  if (Status != 0) {
    fits_get_errstatus(Status, Words);
    cerr << "Error : fits_write_col('DPOSX3') failed (" << Words << ")" << endl;
    fits_close_file(m_File, &Status);
    m_File = 0;
    return false;
  }
  fits_write_col(m_File, TDOUBLE, ++Column, 1, 1, DPOSY3.size(), &DPOSY3[0], &Status);
  if (Status != 0) {
    fits_get_errstatus(Status, Words);
    cerr << "Error : fits_write_col('DPOSY3') failed (" << Words << ")" << endl;
    fits_close_file(m_File, &Status);
    m_File = 0;
    return false;
  }
  fits_write_col(m_File, TDOUBLE, ++Column, 1, 1, DPOSZ3.size(), &DPOSZ3[0], &Status);
  if (Status != 0) {
    fits_get_errstatus(Status, Words);
    cerr << "Error : fits_write_col('DPOSZ3') failed (" << Words << ")" << endl;
    fits_close_file(m_File, &Status);
    m_File = 0;
    return false;
  }
  
  
  fits_write_col(m_File, TDOUBLE, ++Column, 1, 1, ENERGY4.size(), &ENERGY4[0], &Status);
  if (Status != 0) {
    fits_get_errstatus(Status, Words);
    cerr << "Error : fits_write_col('ENERGY4') failed (" << Words << ")" << endl;
    fits_close_file(m_File, &Status);
    m_File = 0;
    return false;
  }
  fits_write_col(m_File, TDOUBLE, ++Column, 1, 1, POSX4.size(), &POSX4[0], &Status);
  if (Status != 0) {
    fits_get_errstatus(Status, Words);
    cerr << "Error : fits_write_col('POSX4') failed (" << Words << ")" << endl;
    fits_close_file(m_File, &Status);
    m_File = 0;
    return false;
  }
  fits_write_col(m_File, TDOUBLE, ++Column, 1, 1, POSY4.size(), &POSY4[0], &Status);
  if (Status != 0) {
    fits_get_errstatus(Status, Words);
    cerr << "Error : fits_write_col('POSY4') failed (" << Words << ")" << endl;
    fits_close_file(m_File, &Status);
    m_File = 0;
    return false;
  }
  fits_write_col(m_File, TDOUBLE, ++Column, 1, 1, POSZ4.size(), &POSZ4[0], &Status);
  if (Status != 0) {
    fits_get_errstatus(Status, Words);
    cerr << "Error : fits_write_col('POSZ4') failed (" << Words << ")" << endl;
    fits_close_file(m_File, &Status);
    m_File = 0;
    return false;
  }
  fits_write_col(m_File, TDOUBLE, ++Column, 1, 1, DENERGY4.size(), &DENERGY4[0], &Status);
  if (Status != 0) {
    fits_get_errstatus(Status, Words);
    cerr << "Error : fits_write_col('DENERGY4') failed (" << Words << ")" << endl;
    fits_close_file(m_File, &Status);
    m_File = 0;
    return false;
  }
  fits_write_col(m_File, TDOUBLE, ++Column, 1, 1, DPOSX4.size(), &DPOSX4[0], &Status);
  if (Status != 0) {
    fits_get_errstatus(Status, Words);
    cerr << "Error : fits_write_col('DPOSX4') failed (" << Words << ")" << endl;
    fits_close_file(m_File, &Status);
    m_File = 0;
    return false;
  }
  fits_write_col(m_File, TDOUBLE, ++Column, 1, 1, DPOSY4.size(), &DPOSY4[0], &Status);
  if (Status != 0) {
    fits_get_errstatus(Status, Words);
    cerr << "Error : fits_write_col('DPOSY4') failed (" << Words << ")" << endl;
    fits_close_file(m_File, &Status);
    m_File = 0;
    return false;
  }
  fits_write_col(m_File, TDOUBLE, ++Column, 1, 1, DPOSZ4.size(), &DPOSZ4[0], &Status);
  if (Status != 0) {
    fits_get_errstatus(Status, Words);
    cerr << "Error : fits_write_col('DPOSZ4') failed (" << Words << ")" << endl;
    fits_close_file(m_File, &Status);
    m_File = 0;
    return false;
  }
  
  
  fits_write_col(m_File, TDOUBLE, ++Column, 1, 1, ENERGY5.size(), &ENERGY5[0], &Status);
  if (Status != 0) {
    fits_get_errstatus(Status, Words);
    cerr << "Error : fits_write_col('ENERGY5') failed (" << Words << ")" << endl;
    fits_close_file(m_File, &Status);
    m_File = 0;
    return false;
  }
  fits_write_col(m_File, TDOUBLE, ++Column, 1, 1, POSX5.size(), &POSX5[0], &Status);
  if (Status != 0) {
    fits_get_errstatus(Status, Words);
    cerr << "Error : fits_write_col('POSX5') failed (" << Words << ")" << endl;
    fits_close_file(m_File, &Status);
    m_File = 0;
    return false;
  }
  fits_write_col(m_File, TDOUBLE, ++Column, 1, 1, POSY5.size(), &POSY5[0], &Status);
  if (Status != 0) {
    fits_get_errstatus(Status, Words);
    cerr << "Error : fits_write_col('POSY5') failed (" << Words << ")" << endl;
    fits_close_file(m_File, &Status);
    m_File = 0;
    return false;
  }
  fits_write_col(m_File, TDOUBLE, ++Column, 1, 1, POSZ5.size(), &POSZ5[0], &Status);
  if (Status != 0) {
    fits_get_errstatus(Status, Words);
    cerr << "Error : fits_write_col('POSZ5') failed (" << Words << ")" << endl;
    fits_close_file(m_File, &Status);
    m_File = 0;
    return false;
  }
  fits_write_col(m_File, TDOUBLE, ++Column, 1, 1, DENERGY5.size(), &DENERGY5[0], &Status);
  if (Status != 0) {
    fits_get_errstatus(Status, Words);
    cerr << "Error : fits_write_col('DENERGY5') failed (" << Words << ")" << endl;
    fits_close_file(m_File, &Status);
    m_File = 0;
    return false;
  }
  fits_write_col(m_File, TDOUBLE, ++Column, 1, 1, DPOSX5.size(), &DPOSX5[0], &Status);
  if (Status != 0) {
    fits_get_errstatus(Status, Words);
    cerr << "Error : fits_write_col('DPOSX5') failed (" << Words << ")" << endl;
    fits_close_file(m_File, &Status);
    m_File = 0;
    return false;
  }
  fits_write_col(m_File, TDOUBLE, ++Column, 1, 1, DPOSY5.size(), &DPOSY5[0], &Status);
  if (Status != 0) {
    fits_get_errstatus(Status, Words);
    cerr << "Error : fits_write_col('DPOSY5') failed (" << Words << ")" << endl;
    fits_close_file(m_File, &Status);
    m_File = 0;
    return false;
  }
  fits_write_col(m_File, TDOUBLE, ++Column, 1, 1, DPOSZ5.size(), &DPOSZ5[0], &Status);
  if (Status != 0) {
    fits_get_errstatus(Status, Words);
    cerr << "Error : fits_write_col('DPOSZ5') failed (" << Words << ")" << endl;
    fits_close_file(m_File, &Status);
    m_File = 0;
    return false;
  }
  
  
  fits_write_col(m_File, TDOUBLE, ++Column, 1, 1, ENERGY6.size(), &ENERGY6[0], &Status);
  if (Status != 0) {
    fits_get_errstatus(Status, Words);
    cerr << "Error : fits_write_col('ENERGY6') failed (" << Words << ")" << endl;
    fits_close_file(m_File, &Status);
    m_File = 0;
    return false;
  }
  fits_write_col(m_File, TDOUBLE, ++Column, 1, 1, POSX6.size(), &POSX6[0], &Status);
  if (Status != 0) {
    fits_get_errstatus(Status, Words);
    cerr << "Error : fits_write_col('POSX6') failed (" << Words << ")" << endl;
    fits_close_file(m_File, &Status);
    m_File = 0;
    return false;
  }
  fits_write_col(m_File, TDOUBLE, ++Column, 1, 1, POSY6.size(), &POSY6[0], &Status);
  if (Status != 0) {
    fits_get_errstatus(Status, Words);
    cerr << "Error : fits_write_col('POSY6') failed (" << Words << ")" << endl;
    fits_close_file(m_File, &Status);
    m_File = 0;
    return false;
  }
  fits_write_col(m_File, TDOUBLE, ++Column, 1, 1, POSZ6.size(), &POSZ6[0], &Status);
  if (Status != 0) {
    fits_get_errstatus(Status, Words);
    cerr << "Error : fits_write_col('POSZ6') failed (" << Words << ")" << endl;
    fits_close_file(m_File, &Status);
    m_File = 0;
    return false;
  }
  fits_write_col(m_File, TDOUBLE, ++Column, 1, 1, DENERGY6.size(), &DENERGY6[0], &Status);
  if (Status != 0) {
    fits_get_errstatus(Status, Words);
    cerr << "Error : fits_write_col('DENERGY6') failed (" << Words << ")" << endl;
    fits_close_file(m_File, &Status);
    m_File = 0;
    return false;
  }
  fits_write_col(m_File, TDOUBLE, ++Column, 1, 1, DPOSX6.size(), &DPOSX6[0], &Status);
  if (Status != 0) {
    fits_get_errstatus(Status, Words);
    cerr << "Error : fits_write_col('DPOSX6') failed (" << Words << ")" << endl;
    fits_close_file(m_File, &Status);
    m_File = 0;
    return false;
  }
  fits_write_col(m_File, TDOUBLE, ++Column, 1, 1, DPOSY6.size(), &DPOSY6[0], &Status);
  if (Status != 0) {
    fits_get_errstatus(Status, Words);
    cerr << "Error : fits_write_col('DPOSY6') failed (" << Words << ")" << endl;
    fits_close_file(m_File, &Status);
    m_File = 0;
    return false;
  }
  fits_write_col(m_File, TDOUBLE, ++Column, 1, 1, DPOSZ6.size(), &DPOSZ6[0], &Status);
  if (Status != 0) {
    fits_get_errstatus(Status, Words);
    cerr << "Error : fits_write_col('DPOSZ6') failed (" << Words << ")" << endl;
    fits_close_file(m_File, &Status);
    m_File = 0;
    return false;
  }
  
  
  fits_write_col(m_File, TDOUBLE, ++Column, 1, 1, ENERGY7.size(), &ENERGY7[0], &Status);
  if (Status != 0) {
    fits_get_errstatus(Status, Words);
    cerr << "Error : fits_write_col('ENERGY7') failed (" << Words << ")" << endl;
    fits_close_file(m_File, &Status);
    m_File = 0;
    return false;
  }
  fits_write_col(m_File, TDOUBLE, ++Column, 1, 1, POSX7.size(), &POSX7[0], &Status);
  if (Status != 0) {
    fits_get_errstatus(Status, Words);
    cerr << "Error : fits_write_col('POSX7') failed (" << Words << ")" << endl;
    fits_close_file(m_File, &Status);
    m_File = 0;
    return false;
  }
  fits_write_col(m_File, TDOUBLE, ++Column, 1, 1, POSY7.size(), &POSY7[0], &Status);
  if (Status != 0) {
    fits_get_errstatus(Status, Words);
    cerr << "Error : fits_write_col('POSY7') failed (" << Words << ")" << endl;
    fits_close_file(m_File, &Status);
    m_File = 0;
    return false;
  }
  fits_write_col(m_File, TDOUBLE, ++Column, 1, 1, POSZ7.size(), &POSZ7[0], &Status);
  if (Status != 0) {
    fits_get_errstatus(Status, Words);
    cerr << "Error : fits_write_col('POSZ7') failed (" << Words << ")" << endl;
    fits_close_file(m_File, &Status);
    m_File = 0;
    return false;
  }
  fits_write_col(m_File, TDOUBLE, ++Column, 1, 1, DENERGY7.size(), &DENERGY7[0], &Status);
  if (Status != 0) {
    fits_get_errstatus(Status, Words);
    cerr << "Error : fits_write_col('DENERGY7') failed (" << Words << ")" << endl;
    fits_close_file(m_File, &Status);
    m_File = 0;
    return false;
  }
  fits_write_col(m_File, TDOUBLE, ++Column, 1, 1, DPOSX7.size(), &DPOSX7[0], &Status);
  if (Status != 0) {
    fits_get_errstatus(Status, Words);
    cerr << "Error : fits_write_col('DPOSX7') failed (" << Words << ")" << endl;
    fits_close_file(m_File, &Status);
    m_File = 0;
    return false;
  }
  fits_write_col(m_File, TDOUBLE, ++Column, 1, 1, DPOSY7.size(), &DPOSY7[0], &Status);
  if (Status != 0) {
    fits_get_errstatus(Status, Words);
    cerr << "Error : fits_write_col('DPOSY7') failed (" << Words << ")" << endl;
    fits_close_file(m_File, &Status);
    m_File = 0;
    return false;
  }
  fits_write_col(m_File, TDOUBLE, ++Column, 1, 1, DPOSZ7.size(), &DPOSZ7[0], &Status);
  if (Status != 0) {
    fits_get_errstatus(Status, Words);
    cerr << "Error : fits_write_col('DPOSZ7') failed (" << Words << ")" << endl;
    fits_close_file(m_File, &Status);
    m_File = 0;
    return false;
  }
  
  
  fits_write_col(m_File, TDOUBLE, ++Column, 1, 1, QUAL1.size(), &QUAL1[0], &Status);
  if (Status != 0) {
    fits_get_errstatus(Status, Words);
    cerr << "Error : fits_write_col('QUAL1') failed (" << Words << ")" << endl;
    fits_close_file(m_File, &Status);
    m_File = 0;
    return false;
  }
  fits_write_col(m_File, TDOUBLE, ++Column, 1, 1, QUAL2.size(), &QUAL2[0], &Status);
  if (Status != 0) {
    fits_get_errstatus(Status, Words);
    cerr << "Error : fits_write_col('QUAL2') failed (" << Words << ")" << endl;
    fits_close_file(m_File, &Status);
    m_File = 0;
    return false;
  }
  
  
  fits_close_file(m_File, &Status);
  
  

  // Some cleanup
  delete Reader;
  delete Geometry;

  
  return true;
}


/******************************************************************************/

TraFitsConverter* g_Prg = 0;
int g_NInterrupts = 2;

/******************************************************************************/


/******************************************************************************
 * Called when an interrupt signal is flagged
 * All catched signals lead to a well defined exit of the program
 */
void CatchSignal(int a)
{
  cout<<"Catched signal Ctrl-C:"<<endl;
  
  --g_NInterrupts;
  if (g_NInterrupts <= 0) {
    cout<<"Aborting..."<<endl;
    abort();
  } else {
    cout<<"Trying to cancel the analysis..."<<endl;
    if (g_Prg != 0) {
      g_Prg->Interrupt();
    }
    cout<<"If you hit "<<g_NInterrupts<<" more times, then I will abort immediately!"<<endl;
  }
}


/******************************************************************************
 * Main program
 */
int main(int argc, char** argv)
{
  // Set a default error handler and catch some signals...
  signal(SIGINT, CatchSignal);

  // Initialize global MEGAlib variables, especially mgui, etc.
  MGlobal::Initialize();

  TApplication TraFitsConverterApp("TraFitsConverterApp", 0, 0);

  g_Prg = new TraFitsConverter();

  if (g_Prg->ParseCommandLine(argc, argv) == false) {
    cerr<<"Error during parsing of command line!"<<endl;
    return -1;
  } 


  cout<<"Program exited normally!"<<endl;

  return 0;
}

/*
 * Cosima: the end...
 ******************************************************************************/
