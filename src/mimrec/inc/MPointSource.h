/*
 * MPointSource.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MPointSource__
#define __MPointSource__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include "MString.h"

// MEGAlib libs:
#include "MGlobal.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MPointSource
{
  // Public Interface:
 public:
  MPointSource();
  MPointSource(MString Name, double Latitude, double Longitude, double Flux);
  MPointSource(MString Name, double Latitude, double Longitude, double Radius, double Flux);
  virtual ~MPointSource();

  bool ParseLine(MString Line, unsigned int Version);

  void SetExtensionType(const unsigned int& ExtensionType) { m_ExtensionType = ExtensionType; }
  unsigned int GetExtensionType() const { return m_ExtensionType; }

  void SetObjectType(MString ObjectType);
  void SetObjectType(const unsigned int& ObjectType) { m_ObjectType = ObjectType; }
  unsigned int GetObjectType() const { return m_ObjectType; }

  void SetSpectralType(const unsigned int& SpectralType) { m_SpectralType = SpectralType; }
  unsigned int GetSpectralType() const { return m_SpectralType; }

  void SetName(const MString& Name) { m_Name = Name; }
  MString GetName() const { return m_Name; };

  void SetCoordinates(MString Type, double theta, double phi);

  void SetLatitude(const double& Latitude) { m_Latitude = Latitude; }
  double GetLatitude() const { return m_Latitude; }

  void SetLongitude(const double& Longitude) { m_Longitude = Longitude; }
  double GetLongitude() const { return m_Longitude; }

  void SetFlux(const double& Flux, const double& MinimumEnergy, const double& MaximumEnergy);
  double GetFlux() const { return m_Flux; }
  double GetFlux(double Emin, double Emax);
  double GetMinimumEnergy() const { return m_MinimumEnergy; }
  double GetMaximumEnergy() const { return m_MaximumEnergy; }
 

  void SetGamma1(const double& Gamma1) { m_Gamma1 = Gamma1; }
  double GetGamma1() const { return m_Gamma1; }

  void SetGamma2(const double& Gamma2) { m_Gamma2 = Gamma2; }
  double GetGamma2() const { return m_Gamma2; }

  void SetGamma3(const double& Gamma3) { m_Gamma3 = Gamma3; }
  double GetGamma3() const { return m_Gamma3; }

  void SetBreak1(const double& Break1) { m_Break1 = Break1; }
  double GetBreak1() const { return m_Break1; }

  void SetBreak2(const double& Break2) { m_Break2 = Break2; }
  double GetBreak2() const { return m_Break2; }

  void SetRadius(const double& Radius) { m_Radius = Radius; }
  double GetRadius() { return m_Radius; }

  //! Add a break to power-law and broken-power-law spectra: New break is FIRST break
  bool AddLowPowerLawBreak(double Gamma1, double Break1);



  static const unsigned int c_ExtensionPointLike;
  static const unsigned int c_ExtensionDiskLike;

  static const unsigned int c_SpectrumUnknown;
  static const unsigned int c_SpectrumMono;
  static const unsigned int c_SpectrumPowerLaw;
  static const unsigned int c_SpectrumPowerLawExpCutoff;
  static const unsigned int c_SpectrumBrokenPowerLaw;
  static const unsigned int c_SpectrumDoubleBrokenPowerLaw;

  static const double c_RadiusOfPointSource;

  static const unsigned int c_ObjectUnknown;
  static const unsigned int c_ObjectAGN;
  static const unsigned int c_ObjectBlazar;
  static const unsigned int c_ObjectBLLac;
  static const unsigned int c_ObjectPulsar;
  static const unsigned int c_ObjectFSRQ;
  static const unsigned int c_ObjectSNR;
  static const unsigned int c_ObjectCloud;
  static const unsigned int c_ObjectHMXB;
  static const unsigned int c_ObjectLMXB;

  // protected methods:
 protected:
  void ConvertEquatorialGalactic();
  
  //! Verify the data and calculate the constants
  bool Verify();

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  //! Type of the source (point-like, disk-like)
  unsigned int m_ExtensionType;
  //! Type of the source (mono, power law, broken power law, etc.)
  unsigned int m_SpectralType;
  //! Type of the source (pulsar, agn, etc.)
  unsigned int m_ObjectType;

  //! Name of the point source
  MString m_Name; 

  //! Right Ascencion
  double m_RightAscension;
  //! Declination
  double m_Declination;
  //! Galactic Latitude
  double m_Latitude;
  //! Galactic Longitude
  double m_Longitude;

  //! Flux in ph/cm2/s
  double m_Flux;
  //! Minimum integration energy for flux
  double m_MinimumEnergy;
  //! Maximum integration energy for flux
  double m_MaximumEnergy;

  //! Gamma as in E^{-gamma1}
  double m_Gamma1;
  //! Gamma as in E^{-gamma2}
  double m_Gamma2;
  //! Gamma as in E^{-gamma2}
  double m_Gamma3;
  //! Break energy for broken power law
  double m_Break1;
  //! Break energy for double broken power law
  double m_Break2;

  //! First constant of the spectral distribution (e.g. Constant1*E^(-Gamma1) )
  double m_Constant1;
  //! Second constant of the spectral distribution
  double m_Constant2;
  //! Third constant of the spectral distribution
  double m_Constant3;

  //! Radius
  double m_Radius;

  friend ostream& operator<<(ostream& os, const MPointSource& P);

#ifdef ___CLING___
 public:
  ClassDef(MPointSource, 0)
#endif

};

ostream& operator<<(ostream& os, const MPointSource& P);

#endif


////////////////////////////////////////////////////////////////////////////////
