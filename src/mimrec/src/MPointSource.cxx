/*
 * MPointsource.cxx
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
// MPointsource
//
// This class stores all the values necessary to identify a pointsource:
// 
// its x-'n'y-values, ie galactic latitude and longitude
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MPointSource.h"

// Standard libs:
#include <iostream>
#include <cstdlib>
#include <cstdio>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MAssert.h"
#include "MStreams.h"
#include "MTokenizer.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MPointSource)
#endif


////////////////////////////////////////////////////////////////////////////////


const unsigned int MPointSource::c_ExtensionPointLike = 1;
const unsigned int MPointSource::c_ExtensionDiskLike  = 2;

const unsigned int MPointSource::c_SpectrumUnknown               = 0;
const unsigned int MPointSource::c_SpectrumMono                  = 1;
const unsigned int MPointSource::c_SpectrumPowerLaw              = 2;
const unsigned int MPointSource::c_SpectrumPowerLawExpCutoff     = 3;
const unsigned int MPointSource::c_SpectrumBrokenPowerLaw        = 4;
const unsigned int MPointSource::c_SpectrumDoubleBrokenPowerLaw  = 5;

const unsigned int MPointSource::c_ObjectUnknown = 0;
const unsigned int MPointSource::c_ObjectAGN     = 1;
const unsigned int MPointSource::c_ObjectBlazar  = 2;
const unsigned int MPointSource::c_ObjectBLLac   = 3;
const unsigned int MPointSource::c_ObjectFSRQ    = 4;
const unsigned int MPointSource::c_ObjectPulsar  = 5;
const unsigned int MPointSource::c_ObjectSNR     = 6;
const unsigned int MPointSource::c_ObjectCloud   = 7;
const unsigned int MPointSource::c_ObjectHMXB    = 8;
const unsigned int MPointSource::c_ObjectLMXB    = 9;


const double MPointSource::c_RadiusOfPointSource = 2.777e-4; // one arcsec

////////////////////////////////////////////////////////////////////////////////


MPointSource::MPointSource()
{
  // standard constructor

  m_ExtensionType = c_ExtensionPointLike;
  m_SpectralType = c_SpectrumMono;
  m_ObjectType = c_ObjectUnknown;
  m_Name = g_StringNotDefined;

  m_RightAscension = 0;
  m_Declination = 0;
  m_Latitude = 0;
  m_Longitude = 0;
  m_Flux = 0;
  m_MinimumEnergy = 0;
  m_MaximumEnergy = 0;

  m_Gamma1 = 0;
  m_Gamma2 = 0;
  m_Gamma3 = 0;

  m_Break1 = 0;
  m_Break2 = 0;
  
  m_Constant1 = 0;
  m_Constant2 = 0;
  m_Constant3 = 0;

  m_Radius = c_RadiusOfPointSource;
}


////////////////////////////////////////////////////////////////////////////////


MPointSource::MPointSource(MString Name, double Latitude, double Longitude, 
                           double Flux)
{
  // extended constructor

  m_ExtensionType = c_ExtensionPointLike;
  m_SpectralType = c_SpectrumMono;
  m_ObjectType = c_ObjectUnknown;
  m_Radius = c_RadiusOfPointSource;

  m_RightAscension = 0;
  m_Declination = 0;
  m_Latitude = 0;
  m_Longitude = 0;
  m_Flux = 0;
  m_MinimumEnergy = 0;
  m_MaximumEnergy = 0;

  m_Gamma1 = 0;
  m_Gamma2 = 0;
  m_Gamma3 = 0;

  m_Break1 = 0;
  m_Break2 = 0;
  
  m_Constant1 = 0;
  m_Constant2 = 0;
  m_Constant3 = 0;

  m_Name = Name;
  m_Latitude = Latitude;
  m_Longitude = Longitude;
  m_Flux = Flux;
}


////////////////////////////////////////////////////////////////////////////////


MPointSource::MPointSource(MString Name, double Latitude, double Longitude, 
                           double Radius, double Flux)
{
  // extended constructor

  m_ExtensionType = c_ExtensionDiskLike;
  m_SpectralType = c_SpectrumMono;
  m_ObjectType = c_ObjectUnknown;
  m_Radius = Radius;

  m_RightAscension = 0;
  m_Declination = 0;
  m_Latitude = 0;
  m_Longitude = 0;
  m_Flux = 0;
  m_MinimumEnergy = 0;
  m_MaximumEnergy = 0;

  m_Gamma1 = 0;
  m_Gamma2 = 0;
  m_Gamma3 = 0;

  m_Break1 = 0;
  m_Break2 = 0;
  
  m_Constant1 = 0;
  m_Constant2 = 0;
  m_Constant3 = 0;

  m_Name = Name;
  m_Latitude = Latitude;
  m_Longitude = Longitude;
  m_Flux = Flux;
}


////////////////////////////////////////////////////////////////////////////////


bool MPointSource::ParseLine(MString DataScat, unsigned int Version)
{
  // Parse from file

  MTokenizer Tokenizer;
  Tokenizer.Analyse(DataScat);

  if (Tokenizer.GetNTokens() == 0) return false;

  if (Tokenizer.IsTokenAt(0, "PS") == true) {
    if (Tokenizer.GetNTokens() < 5) {
      mout<<" *** ERROR ***"<<endl;
      mout<<"  PS keyword must for example look like this: PS MO <gal lat [deg]> <gal long [deg]> <flux [ph/cm2/s]> <name>"<<endl;
      return false;
    }

    if (Tokenizer.GetTokenAtAsString(1) == "MO") {
      if (Tokenizer.GetNTokens() >= 6) {
        m_ExtensionType = c_ExtensionPointLike;
        m_SpectralType = c_SpectrumMono;
        m_Radius = c_RadiusOfPointSource;
        
        m_Latitude = Tokenizer.GetTokenAtAsDouble(2);
        m_Longitude = Tokenizer.GetTokenAtAsDouble(3);
        m_Flux = Tokenizer.GetTokenAtAsDouble(4); 
        m_Name = Tokenizer.GetTokenAfterAsString(5);
      } else {
        mout<<" *** ERROR ***"<<endl;
        mout<<"  PS keyword must look like this: PS MO <gal lat [deg]> <gal long [deg]> <flux [ph/cm2/s]> <name>"<<endl;
        return false;
      }
    } else if (Tokenizer.GetTokenAtAsString(1) == "PL") {
      if (Tokenizer.GetNTokens() >= 10) {
        m_ExtensionType = c_ExtensionPointLike;
        m_SpectralType = c_SpectrumPowerLaw;
        m_Radius = c_RadiusOfPointSource;

        SetObjectType(Tokenizer.GetTokenAtAsString(2));
        SetCoordinates(Tokenizer.GetTokenAtAsString(3), Tokenizer.GetTokenAtAsDouble(4), Tokenizer.GetTokenAtAsDouble(5));

        m_Flux = Tokenizer.GetTokenAtAsDouble(6); 
        m_MinimumEnergy = Tokenizer.GetTokenAtAsDouble(7); 
        m_MaximumEnergy = Tokenizer.GetTokenAtAsDouble(8); 

        m_Gamma1 = Tokenizer.GetTokenAtAsDouble(9); 
        m_Name = Tokenizer.GetTokenAfterAsString(10);
      } else {
        mout<<" *** ERROR ***"<<endl;
        mout<<"  PS keyword must look like this: PS PL <gal lat [deg]> <gal long [deg]> <flux [ph/cm2/s]>  <min energy [keV]> < max energy [keV] <power law index> <name>"<<endl;
        mout<<Tokenizer.ToString()<<endl;
        return false;
      }
    } else if (Tokenizer.GetTokenAtAsString(1) == "BPL") {
      if (Tokenizer.GetNTokens() >= 12) {
        m_ExtensionType = c_ExtensionPointLike;
        m_SpectralType = c_SpectrumBrokenPowerLaw;
        m_Radius = c_RadiusOfPointSource;

        SetObjectType(Tokenizer.GetTokenAtAsString(2));
        SetCoordinates(Tokenizer.GetTokenAtAsString(3), Tokenizer.GetTokenAtAsDouble(4), Tokenizer.GetTokenAtAsDouble(5));

        m_Flux = Tokenizer.GetTokenAtAsDouble(6); 
        m_MinimumEnergy = Tokenizer.GetTokenAtAsDouble(7); 
        m_MaximumEnergy = Tokenizer.GetTokenAtAsDouble(8); 

        m_Gamma1 = Tokenizer.GetTokenAtAsDouble(9); 
        m_Gamma2 = Tokenizer.GetTokenAtAsDouble(10); 
        m_Break1 = Tokenizer.GetTokenAtAsDouble(11); 
        m_Name = Tokenizer.GetTokenAfterAsString(12);
      } else {
        mout<<" *** ERROR ***"<<endl;
        mout<<"  PS keyword must look like this: PS PL <gal lat [deg]> <gal long [deg]> <flux [ph/cm2/s]>  <power law index> <name>"<<endl;
        mout<<Tokenizer.ToString()<<endl;
        return false;
      }
    } else if (Tokenizer.GetTokenAtAsString(1) == "PLEC") {
      if (Tokenizer.GetNTokens() >= 12) {
        m_ExtensionType = c_ExtensionPointLike;
        m_SpectralType = c_SpectrumPowerLawExpCutoff;
        m_Radius = c_RadiusOfPointSource;

        SetObjectType(Tokenizer.GetTokenAtAsString(2));
        SetCoordinates(Tokenizer.GetTokenAtAsString(3), Tokenizer.GetTokenAtAsDouble(4), Tokenizer.GetTokenAtAsDouble(5));

        m_Flux = Tokenizer.GetTokenAtAsDouble(6); 
        m_MinimumEnergy = Tokenizer.GetTokenAtAsDouble(7); 
        m_MaximumEnergy = Tokenizer.GetTokenAtAsDouble(8); 

        m_Gamma1 = Tokenizer.GetTokenAtAsDouble(9); 
        m_Gamma2 = Tokenizer.GetTokenAtAsDouble(10); 
        m_Break1 = Tokenizer.GetTokenAtAsDouble(11); 
        m_Name = Tokenizer.GetTokenAfterAsString(12);
      } else {
        mout<<" *** ERROR ***"<<endl;
        mout<<"  PS keyword must look like this: PS PL <Type> GA <gal lat [deg]> <gal long [deg]> <flux [ph/cm2/s]>  <power law index> <name>"<<endl;
        mout<<Tokenizer.ToString()<<endl;
        return false;
      }
    } else if (Tokenizer.GetTokenAtAsString(1) == "U") {
      if (Tokenizer.GetNTokens() >= 7) {
        m_ExtensionType = c_ExtensionPointLike;
        m_SpectralType = c_SpectrumUnknown;
        m_Radius = c_RadiusOfPointSource;

        SetObjectType(Tokenizer.GetTokenAtAsString(2));
        SetCoordinates(Tokenizer.GetTokenAtAsString(3), Tokenizer.GetTokenAtAsDouble(4), Tokenizer.GetTokenAtAsDouble(5));

        m_Name = Tokenizer.GetTokenAfterAsString(6);
      } else {
        mout<<" *** ERROR ***"<<endl;
        mout<<"  PS keyword must look like this: PS U <Type> GA <gal lat [deg]> <gal long [deg]> <name>"<<endl;
        mout<<Tokenizer.ToString()<<endl;
        return false;
      }
    } else {
      mout<<" *** ERROR ***"<<endl;
      mout<<"  PS unknown sub type: "<<Tokenizer.GetTokenAtAsString(1)<<endl;
      return false;
    }
  } else if (Tokenizer.IsTokenAt(0, "DS") == true) {
    if (Tokenizer.GetNTokens() < 6 ) {
      mout<<" *** ERROR ***"<<endl;
      mout<<"  DS keyword must look like this: DS <gal lat [deg]> <gal long [deg]> <radius [deg]> <flux [ph/cm2/s]> <name>"<<endl;
      return false;
    }
    m_ExtensionType = c_ExtensionDiskLike;

    m_Latitude = Tokenizer.GetTokenAtAsDouble(1);
    m_Longitude = Tokenizer.GetTokenAtAsDouble(2);
    m_Radius = Tokenizer.GetTokenAtAsDouble(3); 
    m_Flux = Tokenizer.GetTokenAtAsDouble(4); 
    m_Name = Tokenizer.GetTokenAfterAsString(5);
  } else {
    mout<<" *** ERROR ***"<<endl;
    mout<<"  Unknown keyword "<<Tokenizer.GetTokenAt(0)<<endl;
    return false;
  }
 
  //cout<<"Initial verify: "<<endl;
  //Verify();

  return true;
}


////////////////////////////////////////////////////////////////////////////////


MPointSource::~MPointSource()
{
  //standard destructor
}


////////////////////////////////////////////////////////////////////////////////


void MPointSource::SetObjectType(MString Object)
{
  //

  Object.ToLower();
  if (Object == "pulsar") {
    m_ObjectType = c_ObjectPulsar;
  } else if (Object == "agn") {
    m_ObjectType = c_ObjectAGN;
  } else if (Object == "blazar") {
    m_ObjectType = c_ObjectBlazar;
  } else if (Object == "bllac") {
    m_ObjectType = c_ObjectBLLac;
  } else if (Object == "fsrq") {
    m_ObjectType = c_ObjectFSRQ;
  } else if (Object == "snr") {
    m_ObjectType = c_ObjectSNR;
  } else if (Object == "cloud") {
    m_ObjectType = c_ObjectCloud;
  } else if (Object == "hmxb") {
    m_ObjectType = c_ObjectHMXB;
  } else if (Object == "lmxb") {
    m_ObjectType = c_ObjectLMXB;
  } else {
    m_ObjectType = c_ObjectUnknown;
  }

}

////////////////////////////////////////////////////////////////////////////////


void MPointSource::SetCoordinates(MString Type, double theta, double phi)
{
  if (Type == "GA") {
    m_Latitude = theta;
    m_Longitude = phi;
  } else if (Type == "EQ") {
    m_RightAscension = theta;
    m_Declination = phi;
    ConvertEquatorialGalactic();
  }
}


////////////////////////////////////////////////////////////////////////////////


void MPointSource::ConvertEquatorialGalactic()
{
  m_Latitude = asin(cos(m_Declination*c_Rad)*cos(27.4*c_Rad)*cos((m_RightAscension-192.25)*c_Rad) + sin(m_Declination*c_Rad)*sin(27.4*c_Rad))*c_Deg;

  double Zaehler = sin(m_Declination*c_Rad) - sin(m_Latitude*c_Rad)*sin(27.4*c_Rad);
  double Nenner = cos(m_Declination*c_Rad)*sin((m_RightAscension-192.25)*c_Rad)*cos(27.4*c_Rad);

  m_Longitude = atan2(Zaehler, Nenner)*c_Deg + 33.0; 
  while (m_Longitude < 0) m_Longitude += 360.0; 
  while (m_Longitude > 360.0) m_Longitude -= 360.0; 

  return;
}


////////////////////////////////////////////////////////////////////////////////


void MPointSource::SetFlux(const double& Flux, const double& MinimumEnergy, const double& MaximumEnergy)
{
  m_Flux = Flux;
  m_MinimumEnergy = MinimumEnergy;
  m_MaximumEnergy = MaximumEnergy;

  //cout<<"Set new flux verify: "<<endl;
  Verify();
}


////////////////////////////////////////////////////////////////////////////////


bool MPointSource::Verify()
{
  // Verify the data and calculate the constants

  mimp<<"Crunch-mode implementation: Not complete and not error checked!"<<show;

  // There is a "stetig fortsetzbare" something at gamma = 1.0;
  if (m_Gamma1 == 1.0) m_Gamma1 = 0.999999;
  if (m_Gamma2 == 1.0) m_Gamma2 = 0.999999;
  if (m_Gamma3 == 1.0) m_Gamma3 = 0.999999;


  if (m_SpectralType == c_SpectrumPowerLaw) {
    m_Constant1 = (m_Flux*(m_Gamma1-1)*pow(m_MinimumEnergy, m_Gamma1)*pow(m_MaximumEnergy, m_Gamma1))/
      (m_MinimumEnergy*pow(m_MaximumEnergy, m_Gamma1)-m_MaximumEnergy*pow(m_MinimumEnergy, m_Gamma1));
  } else if (m_SpectralType == c_SpectrumBrokenPowerLaw) {
    // Calculate m_Constant1 and m_Constant2
    if (m_MaximumEnergy < m_Break1) {
      // Energy interval is before the break ...
      m_Constant1 = (m_Flux*(m_Gamma1-1)*pow(m_MinimumEnergy, m_Gamma1)*pow(m_MaximumEnergy, m_Gamma1))/
        (m_MinimumEnergy*pow(m_MaximumEnergy, m_Gamma1)-m_MaximumEnergy*pow(m_MinimumEnergy, m_Gamma1));
      m_Constant2 = m_Constant1*pow(m_Break1, -m_Gamma1)/pow(m_Break1, -m_Gamma2);
    } else if (m_MinimumEnergy > m_Break1) {
      // Energy interval after before the break ...
      m_Constant2 = (m_Flux*(m_Gamma2-1)*pow(m_MinimumEnergy, m_Gamma2)*pow(m_MaximumEnergy, m_Gamma2))/
        (m_MinimumEnergy*pow(m_MaximumEnergy, m_Gamma2)-m_MaximumEnergy*pow(m_MinimumEnergy, m_Gamma2));
      m_Constant1 = m_Constant2*pow(m_Break1, -m_Gamma2)/pow(m_Break1, -m_Gamma1);
    } else {
      // Break is between min and max ...
      m_Constant1 = -m_Flux*pow(m_Break1, m_Gamma1)*pow(m_MinimumEnergy,m_Gamma1)*pow(m_MaximumEnergy,m_Gamma2)*(m_Gamma1*m_Gamma2-m_Gamma1-m_Gamma2+1)/
        (m_Break1*pow(m_MinimumEnergy,m_Gamma1)*pow(m_MaximumEnergy,m_Gamma2)*m_Gamma2 - 
         m_MinimumEnergy*pow(m_Break1,m_Gamma1)*pow(m_MaximumEnergy,m_Gamma2)*m_Gamma2 + 
         m_MinimumEnergy*pow(m_Break1,m_Gamma1)*pow(m_MaximumEnergy,m_Gamma2) + 
         m_MaximumEnergy*pow(m_Break1,m_Gamma2)*pow(m_MinimumEnergy,m_Gamma1)*m_Gamma1 - 
         m_MaximumEnergy*pow(m_Break1,m_Gamma2)*pow(m_MinimumEnergy,m_Gamma1) - 
         m_Break1*pow(m_MinimumEnergy,m_Gamma1)*pow(m_MaximumEnergy,m_Gamma2)*m_Gamma1);
      m_Constant2 = m_Constant1*pow(m_Break1, -m_Gamma1)/pow(m_Break1, -m_Gamma2);
    }
  } else if (m_SpectralType == c_SpectrumDoubleBrokenPowerLaw) {
    // Calculate m_Constant1 and m_Constant2 and m_Constant3
    if (m_MaximumEnergy <= m_Break1) {
      // Energy interval is before the first break ...
      m_Constant1 = (m_Flux*(m_Gamma1-1)*pow(m_MinimumEnergy, m_Gamma1)*pow(m_MaximumEnergy, m_Gamma1))/
        (m_MinimumEnergy*pow(m_MaximumEnergy, m_Gamma1)-m_MaximumEnergy*pow(m_MinimumEnergy, m_Gamma1));
      m_Constant2 = m_Constant1*pow(m_Break1, -m_Gamma1)/pow(m_Break1, -m_Gamma2);
      m_Constant3 = m_Constant2*pow(m_Break2, -m_Gamma2)/pow(m_Break2, -m_Gamma3);
    } else if (m_MinimumEnergy >= m_Break1 && m_MaximumEnergy <= m_Break2) {
      // Energy interval is between the first and second break ...
      m_Constant2 = (m_Flux*(m_Gamma2-1)*pow(m_MinimumEnergy, m_Gamma2)*pow(m_MaximumEnergy, m_Gamma2))/
        (m_MinimumEnergy*pow(m_MaximumEnergy, m_Gamma2)-m_MaximumEnergy*pow(m_MinimumEnergy, m_Gamma2));
      m_Constant1 = m_Constant2*pow(m_Break1, -m_Gamma2)/pow(m_Break1, -m_Gamma1);
      m_Constant3 = m_Constant2*pow(m_Break2, -m_Gamma2)/pow(m_Break2, -m_Gamma3);    
    } else if (m_MinimumEnergy >= m_Break2) {
      // Energy interval after before the second break ...
      m_Constant3 = (m_Flux*(m_Gamma3-1)*pow(m_MinimumEnergy, m_Gamma3)*pow(m_MaximumEnergy, m_Gamma3))/
        (m_MinimumEnergy*pow(m_MaximumEnergy, m_Gamma3)-m_MaximumEnergy*pow(m_MinimumEnergy, m_Gamma3));
      m_Constant2 = m_Constant3*pow(m_Break2, -m_Gamma3)/pow(m_Break2, -m_Gamma2);
      m_Constant1 = m_Constant2*pow(m_Break1, -m_Gamma2)/pow(m_Break1, -m_Gamma1);
    } else if (m_MinimumEnergy < m_Break1 && m_MaximumEnergy > m_Break1 && m_MaximumEnergy < m_Break2) {
      // Break is around first break ...
      m_Constant1 = -m_Flux*pow(m_Break1, m_Gamma1)*pow(m_MinimumEnergy,m_Gamma1)*pow(m_MaximumEnergy,m_Gamma2)*(m_Gamma1*m_Gamma2-m_Gamma1-m_Gamma2+1)/
        (m_Break1*pow(m_MinimumEnergy,m_Gamma1)*pow(m_MaximumEnergy,m_Gamma2)*m_Gamma2 - 
         m_MinimumEnergy*pow(m_Break1,m_Gamma1)*pow(m_MaximumEnergy,m_Gamma2)*m_Gamma2 + 
         m_MinimumEnergy*pow(m_Break1,m_Gamma1)*pow(m_MaximumEnergy,m_Gamma2) + 
         m_MaximumEnergy*pow(m_Break1,m_Gamma2)*pow(m_MinimumEnergy,m_Gamma1)*m_Gamma1 - 
         m_MaximumEnergy*pow(m_Break1,m_Gamma2)*pow(m_MinimumEnergy,m_Gamma1) - 
         m_Break1*pow(m_MinimumEnergy,m_Gamma1)*pow(m_MaximumEnergy,m_Gamma2)*m_Gamma1);
      m_Constant2 = m_Constant1*pow(m_Break1, -m_Gamma1)/pow(m_Break1, -m_Gamma2);
      m_Constant3 = m_Constant2*pow(m_Break2, -m_Gamma2)/pow(m_Break2, -m_Gamma3);
    } else if (m_MinimumEnergy > m_Break1 && m_MinimumEnergy < m_Break2 && m_MaximumEnergy > m_Break2) {
      // Energy interval is around second break ...
      m_Constant2 = -m_Flux*pow(m_Break2, m_Gamma2)*pow(m_MinimumEnergy,m_Gamma2)*pow(m_MaximumEnergy,m_Gamma3)*(m_Gamma2*m_Gamma3-m_Gamma2-m_Gamma3+1)/
        (m_Break2*pow(m_MinimumEnergy,m_Gamma2)*pow(m_MaximumEnergy,m_Gamma3)*m_Gamma3 - 
         m_MinimumEnergy*pow(m_Break2,m_Gamma2)*pow(m_MaximumEnergy,m_Gamma3)*m_Gamma3 + 
         m_MinimumEnergy*pow(m_Break2,m_Gamma2)*pow(m_MaximumEnergy,m_Gamma3) + 
         m_MaximumEnergy*pow(m_Break2,m_Gamma3)*pow(m_MinimumEnergy,m_Gamma2)*m_Gamma2 - 
         m_MaximumEnergy*pow(m_Break2,m_Gamma3)*pow(m_MinimumEnergy,m_Gamma2) - 
         m_Break2*pow(m_MinimumEnergy,m_Gamma2)*pow(m_MaximumEnergy,m_Gamma3)*m_Gamma2);
      m_Constant1 = m_Constant2*pow(m_Break1, -m_Gamma2)/pow(m_Break1, -m_Gamma1);
      m_Constant3 = m_Constant2*pow(m_Break2, -m_Gamma2)/pow(m_Break2, -m_Gamma3);    
    } else if (m_MinimumEnergy < m_Break1 && m_MaximumEnergy > m_Break2) {
      // Break is around first AND second break ...
      m_Constant2 = -m_Flux*pow(m_Break1,m_Gamma2)*pow(m_MinimumEnergy,m_Gamma1)*pow(m_MaximumEnergy,m_Gamma3)*pow(m_Break2,m_Gamma2)*(m_Gamma1*m_Gamma2*m_Gamma3-m_Gamma1*m_Gamma2-m_Gamma1*m_Gamma3+m_Gamma1-m_Gamma2*m_Gamma3+m_Gamma2+m_Gamma3-1)/
        (-m_MinimumEnergy*pow(m_Break1,m_Gamma1)*pow(m_MaximumEnergy,m_Gamma3)*pow(m_Break2,m_Gamma2)+
         m_MinimumEnergy*pow(m_Break1,m_Gamma1)*pow(m_MaximumEnergy,m_Gamma3)*pow(m_Break2,m_Gamma2)*m_Gamma3+
         m_MaximumEnergy*pow(m_Break2,m_Gamma3)*pow(m_Break1,m_Gamma2)*pow(m_MinimumEnergy,m_Gamma1)-
         m_MaximumEnergy*pow(m_Break2,m_Gamma3)*pow(m_Break1,m_Gamma2)*pow(m_MinimumEnergy,m_Gamma1)*m_Gamma1-
         m_Break1*pow(m_MinimumEnergy,m_Gamma1)*pow(m_MaximumEnergy,m_Gamma3)*pow(m_Break2,m_Gamma2)*m_Gamma2-
         m_Break1*pow(m_MinimumEnergy,m_Gamma1)*pow(m_MaximumEnergy,m_Gamma3)*pow(m_Break2,m_Gamma2)*m_Gamma1*m_Gamma3+
         m_Break1*pow(m_MinimumEnergy,m_Gamma1)*pow(m_MaximumEnergy,m_Gamma3)*pow(m_Break2,m_Gamma2)*m_Gamma1+
         m_Break1*pow(m_MinimumEnergy,m_Gamma1)*pow(m_MaximumEnergy,m_Gamma3)*pow(m_Break2,m_Gamma2)*m_Gamma2*m_Gamma3-
         m_MaximumEnergy*pow(m_Break2,m_Gamma3)*pow(m_Break1,m_Gamma2)*pow(m_MinimumEnergy,m_Gamma1)*m_Gamma2+
         m_MaximumEnergy*pow(m_Break2,m_Gamma3)*pow(m_Break1,m_Gamma2)*pow(m_MinimumEnergy,m_Gamma1)*m_Gamma1*m_Gamma2-
         m_Break2*pow(m_Break1,m_Gamma2)*pow(m_MinimumEnergy,m_Gamma1)*pow(m_MaximumEnergy,m_Gamma3)*m_Gamma1*m_Gamma2+
         m_Break2*pow(m_Break1,m_Gamma2)*pow(m_MinimumEnergy,m_Gamma1)*pow(m_MaximumEnergy,m_Gamma3)*m_Gamma2+
         m_Break2*pow(m_Break1,m_Gamma2)*pow(m_MinimumEnergy,m_Gamma1)*pow(m_MaximumEnergy,m_Gamma3)*m_Gamma1*m_Gamma3-
         m_Break2*pow(m_Break1,m_Gamma2)*pow(m_MinimumEnergy,m_Gamma1)*pow(m_MaximumEnergy,m_Gamma3)*m_Gamma3-
         m_MinimumEnergy*pow(m_Break1,m_Gamma1)*pow(m_MaximumEnergy,m_Gamma3)*pow(m_Break2,m_Gamma2)*m_Gamma2*m_Gamma3+
         m_MinimumEnergy*pow(m_Break1,m_Gamma1)*pow(m_MaximumEnergy,m_Gamma3)*pow(m_Break2,m_Gamma2)*m_Gamma2);
      m_Constant1 = m_Constant2*pow(m_Break1, -m_Gamma2)/pow(m_Break1, -m_Gamma1);
      m_Constant3 = m_Constant2*pow(m_Break2, -m_Gamma2)/pow(m_Break2, -m_Gamma3);    
    } else {
      merr<<"Better rethink your >, >=, <, <=!!"<<show;
      return false;
    }
  } else if (m_SpectralType == c_SpectrumPowerLawExpCutoff) {
    // Problem: the is no closed form solution to determine m_Constant1 ...
    // At some time we have to do a numerical integration --- but not today...
    merr<<"Ignoring the exponential cutoff during integration"<<show;
    m_Constant1 = (m_Flux*(m_Gamma1-1)*pow(m_MinimumEnergy, m_Gamma1)*pow(m_MaximumEnergy, m_Gamma1))/
      (m_MinimumEnergy*pow(m_MaximumEnergy, m_Gamma1)-m_MaximumEnergy*pow(m_MinimumEnergy, m_Gamma1));
  } else {
    merr<<"Spectrum not supported"<<show;
    return false;
  }

  //cout<<"Verify: "<<m_Flux<<":"<<GetFlux(m_MinimumEnergy, m_MaximumEnergy)<<": C1="<<m_Constant1<<" - C2="<<m_Constant2<<" - C3="<<m_Constant3<<" - Emin="<<m_MinimumEnergy<<endl;

  return true;
}


////////////////////////////////////////////////////////////////////////////////


double MPointSource::GetFlux(double Emin, double Emax) 
{
  // Determine flux in ph/cm2/s in the specific band

  mimp<<"Crunch-mode implementation: This is largely untested"<<show;

  double Flux = 0.0;

  if (m_SpectralType == c_SpectrumPowerLaw) {
    Flux = (Emin*m_Constant1)/(pow(Emin, m_Gamma1)*(m_Gamma1-1)) - (Emax*m_Constant1)/(pow(Emax, m_Gamma1)*(m_Gamma1-1));
  } else if (m_SpectralType == c_SpectrumBrokenPowerLaw) {
    if (Emax < m_Break1) {
      // Energy interval is before the break ...
      Flux = (Emin*m_Constant1)/(pow(Emin, m_Gamma1)*(m_Gamma1-1)) - (Emax*m_Constant1)/(pow(Emax, m_Gamma1)*(m_Gamma1-1));
    } else if (Emin > m_Break1) {
      // Energy interval after before the break ...
      Flux = (Emin*m_Constant2)/(pow(Emin, m_Gamma2)*(m_Gamma2-1)) - (Emax*m_Constant2)/(pow(Emax, m_Gamma2)*(m_Gamma2-1));
    } else {
      // Break is between min and max ...
      Flux = (Emin*m_Constant1)/(pow(Emin, m_Gamma1)*(m_Gamma1-1)) - (m_Break1*m_Constant1)/(pow(m_Break1, m_Gamma1)*(m_Gamma1-1)) +
        (m_Break1*m_Constant2)/(pow(m_Break1, m_Gamma2)*(m_Gamma2-1)) - (Emax*m_Constant2)/(pow(Emax, m_Gamma2)*(m_Gamma2-1));
    }
  } else if (m_SpectralType == c_SpectrumDoubleBrokenPowerLaw) {
    if (Emax <= m_Break1) {
      // Energy interval is before the first break ...
      Flux = (Emin*m_Constant1)/(pow(Emin, m_Gamma1)*(m_Gamma1-1)) - (Emax*m_Constant1)/(pow(Emax, m_Gamma1)*(m_Gamma1-1));
    } else if (Emin >= m_Break1 && Emax <= m_Break2) {
      // Energy interval is between the first and second break ...
      Flux = (Emin*m_Constant2)/(pow(Emin, m_Gamma2)*(m_Gamma2-1)) - (Emax*m_Constant2)/(pow(Emax, m_Gamma2)*(m_Gamma2-1));
    } else if (Emin >= m_Break2) {
      // Energy interval after before the second break ...
      Flux = (Emin*m_Constant3)/(pow(Emin, m_Gamma3)*(m_Gamma3-1)) - (Emax*m_Constant3)/(pow(Emax, m_Gamma3)*(m_Gamma3-1));
    } else if (Emin < m_Break1 && Emax > m_Break1 && Emax < m_Break2) {
      // Break is around first break ...
      Flux = (Emin*m_Constant1)/(pow(Emin, m_Gamma1)*(m_Gamma1-1)) - (m_Break1*m_Constant1)/(pow(m_Break1, m_Gamma1)*(m_Gamma1-1)) +
        (m_Break1*m_Constant2)/(pow(m_Break1, m_Gamma2)*(m_Gamma2-1)) - (Emax*m_Constant2)/(pow(Emax, m_Gamma2)*(m_Gamma2-1));
    } else if (Emin > m_Break1 && Emin < m_Break2 && Emax > m_Break2) {
      // Break is around first break ...
      Flux = (Emin*m_Constant2)/(pow(Emin, m_Gamma2)*(m_Gamma2-1)) - (m_Break2*m_Constant2)/(pow(m_Break2, m_Gamma2)*(m_Gamma2-1)) +
        (m_Break2*m_Constant3)/(pow(m_Break2, m_Gamma3)*(m_Gamma3-1)) - (Emax*m_Constant3)/(pow(Emax, m_Gamma3)*(m_Gamma3-1));
    } else if (Emin < m_Break1 && Emax > m_Break2) {
      // Break is around first AND second break ...
      Flux = (Emin*m_Constant1)/(pow(Emin, m_Gamma1)*(m_Gamma1-1)) - (m_Break1*m_Constant1)/(pow(m_Break1, m_Gamma1)*(m_Gamma1-1)) +
        (m_Break1*m_Constant2)/(pow(m_Break1, m_Gamma2)*(m_Gamma2-1)) - (m_Break2*m_Constant2)/(pow(m_Break2, m_Gamma2)*(m_Gamma2-1)) +
        (m_Break2*m_Constant3)/(pow(m_Break2, m_Gamma3)*(m_Gamma3-1)) - (Emax*m_Constant3)/(pow(Emax, m_Gamma3)*(m_Gamma3-1));
    }
  } else if (m_SpectralType == c_SpectrumPowerLawExpCutoff) {
    Flux = (Emin*m_Constant1)/(pow(Emin, m_Gamma1)*(m_Gamma1-1)) - (Emax*m_Constant1)/(pow(Emax, m_Gamma1)*(m_Gamma1-1));
  } else {
    merr<<"Spectrum not supported"<<show;
  }

  return Flux;
}


////////////////////////////////////////////////////////////////////////////////


bool MPointSource::AddLowPowerLawBreak(double Gamma1, double Break1)
{
  // Add a break to power-law and broken-power-law spectra: New break is FIRST break

  if (m_MinimumEnergy < Break1) {
    merr<<"Break1 must be below the current minimum energy: "<<Break1<<" ?>? "<<m_MinimumEnergy<<endl;
    merr<<"Otherwise the flux would be inconsistent!"<<show;
    return false;
  }

  if (m_SpectralType == c_SpectrumPowerLaw) {
    m_SpectralType = c_SpectrumBrokenPowerLaw;
    m_Break1 = Break1;
    m_Gamma2 = m_Gamma1;
    m_Gamma1 = Gamma1;
    Verify();
  } else if (m_SpectralType == c_SpectrumBrokenPowerLaw) {
    m_SpectralType = c_SpectrumDoubleBrokenPowerLaw;
    m_Break2 = m_Break1;
    m_Break1 = Break1;
    m_Gamma3 = m_Gamma2;
    m_Gamma2 = m_Gamma1;
    m_Gamma1 = Gamma1;
    cout<<"Add break verify:"<<endl;
    Verify();
  } else {
    merr<<"Wrong spectral type!"<<endl;
    return false;
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


ostream& operator<<(ostream& os, const MPointSource& P)
{
  if (P.m_ExtensionType == MPointSource::c_ExtensionPointLike) {
    os<<"Point source \""<<P.m_Name<<"\" at ("<<P.m_Latitude<<", "<<P.m_Longitude<<"). ";
    if (P.m_SpectralType == MPointSource::c_SpectrumMono) {
      os<<"Spectrum: monoenergetic with the flux "<<P.m_Flux<<" ";
    } else if (P.m_SpectralType == MPointSource::c_SpectrumPowerLaw) {
      os<<"Spectrum: power-law with the flux "<<P.m_Flux<<" and spectral index "<<P.m_Gamma1<<" ";
    } else if (P.m_SpectralType == MPointSource::c_SpectrumBrokenPowerLaw) {
      os<<"Spectrum: broken power-law with the flux "<<P.m_Flux<<" and: E-break: "<<P.m_Break1<<", spectral indices "<<P.m_Gamma1<<", "<<P.m_Gamma2<<" ";
    } else if (P.m_SpectralType == MPointSource::c_SpectrumDoubleBrokenPowerLaw) {
      os<<"Spectrum: double broken power-law with the flux "<<P.m_Flux<<" and: E-breaks: "<<P.m_Break1<<" & "<<P.m_Break2<<", spectral indices "<<P.m_Gamma1<<", "<<P.m_Gamma2<<", "<<P.m_Gamma3<<" ";
    } else if (P.m_SpectralType == MPointSource::c_SpectrumPowerLawExpCutoff) {
      os<<"Spectrum: power-law with exponential cutoff the flux "<<P.m_Flux<<" and spectral index "<<P.m_Gamma1<<" ";
    } else {
      os<<"Spectrum: unknown"<<" ";
    }
  } else if (P.m_ExtensionType == MPointSource::c_ExtensionDiskLike) {
    os<<"Disk source "<<P.m_Name<<" at ("<<P.m_Latitude<<", "<<P.m_Longitude<<") with radius "<<P.m_Radius<<" deg and with the flux "<<P.m_Flux<<" ";
  } else {
    os<<"Unknown source extension"<<endl;
  }
  os<<endl;

  return os;
}


// MPointSource: the end...
////////////////////////////////////////////////////////////////////////////////
