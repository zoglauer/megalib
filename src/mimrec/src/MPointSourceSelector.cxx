/*
 * MPointSourceSelector.cxx
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
// MPointSourceSelector
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MPointSourceSelector.h"

// Standard libs:
#include <iostream>
#include <fstream>
using namespace std;

// ROOT libs:
#include <TSystem.h>

// MEGAlib libs:
#include "MAssert.h"
#include "MStreams.h"
#include "MFile.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MPointSourceSelector)
#endif


////////////////////////////////////////////////////////////////////////////////


MPointSourceSelector::MPointSourceSelector() : MParser(' ', false)
{
  // default constructor
}


////////////////////////////////////////////////////////////////////////////////


MPointSourceSelector::~MPointSourceSelector()
{
  // default destructor
}


////////////////////////////////////////////////////////////////////////////////


bool MPointSourceSelector::Open(MString FileName, unsigned int)
{
  // Open the file and do the parsing


  if (MFile::Open(FileName, c_Read) == false) {
		mlog<<"MPointSourceSelector::Open: Unable to open file "<<FileName<<"."<<endl;
    return false;
  }

  // Read header information:
  MTokenizer T;
    
  // Check the first 100 lines for Version and type: 
  int Lines = 100;
  while (TokenizeLine(T) == true) {
    Lines--;
    if (T.GetNTokens() < 2) continue;
    if (T.GetTokenAt(0) == "Version") {
      m_Version = T.GetTokenAtAsInt(1);
    } else if (T.GetTokenAt(0) == "Type") {
      m_FileType = T.GetTokenAtAsString(1);
    }
    if (Lines == 0) break;
    if (m_Version != c_VersionUnknown && 
        m_FileType != c_TypeUnknown) { 
      break;
    }
  }
  if (m_Version == c_VersionUnknown) {
    mout<<"Warning: No version info found in the response file \""<<m_FileName<<"\"!!"<<endl;
  }
  if (m_FileType == c_TypeUnknown) {
    mout<<"Error: No file type info found in the response file \""<<m_FileName<<"\"!!"<<endl;
    Close();
    return false;
  }
  
  Rewind();

  // Now read the rest:
  MString Line;
  while (m_File.good() == true) {
    Line.ReadLine(m_File);
    if (Line.Length() < 2) continue;

    if ((Line[0] == 'P' && Line[1] == 'S') ||
        (Line[0] == 'D' && Line[1] == 'S')) {
      MPointSource PointSource;
      if (PointSource.ParseLine(Line, m_Version) == true) {
        m_List.push_back(PointSource);
      } else {
        return false;
      }
    } else if (Line[0] == 'E' && Line[1] == 'N') {
      break;
    }
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MPointSourceSelector::LoadList(MString FileName)
{
  // Load a PSList from a File...

  MFile::ExpandFileName(FileName);

  fstream FileStream(FileName, ios_base::in);

  if (FileStream.is_open() == false) {
    merr<<"Can't open file "<<FileName<<"!"<<endl;
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////


bool MPointSourceSelector::TokenizeLine(MTokenizer& T)
{
  // Tokenize one line of the file
  // Return false if an error occurs

  if (m_Way != c_Read) {
    merr<<"Only valid if file is in read-mode!"<<endl;
    massert(m_Way == c_Read);
    return false;
  }

  if (m_File.good() == false) return false;

  MString Line;
  Line.ReadLine(m_File);
  T.Analyse(Line);

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MPointSourceSelector::AddPointSource(const MPointSource& PointSource)
{
  // Set a point source

  m_List.push_back(PointSource);
}


////////////////////////////////////////////////////////////////////////////////


MPointSource MPointSourceSelector::GetPointSourceAt(unsigned int i) const
{
  // Return a point source

  massert(i < m_List.size());
  
  return m_List[i];
}


////////////////////////////////////////////////////////////////////////////////


MPointSource& MPointSourceSelector::operator[] (unsigned i) 
{ 
  // Return a point source

  massert(i < m_List.size());
  
  return m_List[i];
}


////////////////////////////////////////////////////////////////////////////////


unsigned int MPointSourceSelector::GetNPointSources() const
{
  // Return the number of point sources

  return m_List.size();
}


////////////////////////////////////////////////////////////////////////////////


ostream& operator<<(ostream& os, const MPointSourceSelector& P)
{
  os<<"List of sources: "<<endl;
  for (unsigned int i = 0; i < P.m_List.size(); ++i) {
    cout<<P.m_List[i]<<endl;
  }
  
  return os;
}


// ////////////////////////////////////////////////////////////////////////////////


// int MPointSourceSelector::SignOf(double a) 
// { 
//   return (a == 0) ? 0 : (a < 0 ? -1 : 1); 
// }


// ////////////////////////////////////////////////////////////////////////////////


// bool MPointSourceSelector::ConvertEquatorialToGalactic(MPointSource& P) 
// {
//   // Go from equatorial to ecliptic:

//   double IncSat = 0.0*TMath::DegToRad();
//   double IncEarth = 23.439168*TMath::DegToRad(); 
//   double InvIncSol = 27.12825*TMath::DegToRad();

//   double theta = P.GetLatitude();
//   double phi = P.GetLongitude();

//   double asc = atan(tan(phi)*cos(IncEarth) - tan(theta)*sin(IncEarth)/cos(phi));
//   double dec = asin(sin(theta)*cos(IncEarth) + cos(theta)*sin(IncEarth)*sin(phi));

//   if (signof(cos(asc)) == signof(cos(phi)) && signof(sin(asc)) == signof(sin(phi))) {
//     asc = asc;
//   } else {
//     asc += TMath::Pi();
//   }
//   while (asc > 2*TMath::Pi()) asc -= 2*TMath::Pi();
//   while (asc < 0) asc += 2*TMath::Pi();

//   // Go from ecliptic to galactic
//   theta = Galactic->GetBinCenter(by) + TMath::Pi()/2.0;
//   phi = Galactic->GetBinCenter(bx);

//   double sinlat  = cos(theta)*cos(InvIncSol)*sin(phi) + sin(theta)*sin(InvIncSol);
//   double sinlong = cos(theta)*cos(phi);
//   double coslong = sin(theta)*cos(InvIncSol) - cos(theta)*sin(phi)*sin(InvIncSol);

//   double lat = asin(sinlat);
//   double lng = atan2(sinlong, coslong);
  
//   while (lng > 2*TMath::Pi()) lng -= 2*TMath::Pi();
//   while (lng < 0) lng += 2*TMath::Pi();

//   P.SetLatitude(Lat);
//   P.SetLongitude(Long);
// }


// MPointSourceSelector.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
