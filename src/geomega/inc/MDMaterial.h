/*
 * MDMaterial.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MDMaterial__
#define __MDMaterial__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include "TGeoMedium.h"

// MEGAlib libs:
#include "MGlobal.h"
#include "MString.h"
#include "MDMaterialComponent.h"
#include "MResponseMatrixO1.h"
#include "MResponseMatrixO3.h"

// Standard libs::
#include <vector>
using namespace std;

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MDMaterial
{
  // public interface:
 public:
  MDMaterial();
  MDMaterial(MString Name, MString ShortName, MString MGeantShortName);
  virtual ~MDMaterial();

  static MDMaterial* GetDummyMaterial();

  static int ConvertZToNumber(MString Name);
  static MString ConvertZToString(int Z);

  void SetName(MString Name, MString ShortName, MString MGeantShortName);

  virtual MString GetName() const;
  MString GetShortName() const;
  MString GetOriginalMGeantShortName() const;
  MString GetMGeantShortName() const;

  void SetDensity(double Density);
  double GetDensity() const { return m_Density; }

  void SetRadiationLength(double RadiationLength);
  double GetRadiationLength() const { return m_RadiationLength; }

  void SetComponent(MDMaterialComponent* Component);
  void SetComponent(double A, double Z, double Weigth, int Type);
  bool SetComponent(MString Name, double Weigth, int Type);
  MDMaterialComponent* GetComponentAt(unsigned int i) const;
  unsigned int GetNComponents() const;

  void SetSensitivity(int Sensitivity);
  double GetSensitivity() const { return m_Sensitive; }

  void SetCrossSectionFileDirectory(MString Directory);
  MString GetCrossSectionFileDirectory() const;

  bool AreCrossSectionsPresent() const { return m_CrossSectionsPresent; }

  //! Return the total absorption coefficient (= macroscopic cross section)
  double GetAbsorptionCoefficient(double Energy) const;
  //! Return the photo absorption coefficient (= macroscopic cross section)
  double GetPhotoAbsorptionCoefficient(double Energy) const;
  //! Return the Compton scattering coefficient (= macroscopic cross section)
  double GetComptonAbsorptionCoefficient(double Energy) const;
  //! Return the pair absoption coefficient (= macroscopic cross section)
  double GetPairAbsorptionCoefficient(double Energy) const;
  //! Return the rayleigh scattering coefficient (= macroscopic cross section)
  double GetRayleighAbsorptionCoefficient(double Energy) const;

  //! Return the hash of this object --- it is only availbale after a call to validate!
  unsigned long GetHash() const { return m_Hash; }

  int GetID() const;

  int GetZMainComponent() const;

  MString GetGeant3DIM() const;
  MString GetGeant3DATA() const;
  MString GetGeant3() const;
  MString GetMGeant() const;
  MString GetMGeantTmed(int Sensitivity = -1) const;
  
  //! Return in Geomega compatible format
  MString GetGeomega() const;

  MString ToString() const; 

  static void ResetIDs();

  //! Validate the data
  bool Validate();
  //! Load the cross-sections
  bool LoadCrossSections(bool Verbose = false);

  // Clone interface... identical to MDVolume, MDDetector...
  void AddClone(MDMaterial* Clone);
  unsigned int GetNClones() const;
  MDMaterial* GetCloneAt(unsigned int i) const;
  unsigned int GetCloneId(MDMaterial* Clone) const;
  bool IsClone() const;

  void SetCloneTemplate(MDMaterial* Template);
  MDMaterial* GetCloneTemplate() const;

  bool CopyDataToClones();

  //! Return this class as ROOT material
  TGeoMedium* GetRootMedium();

  // protected methods:
 protected:

  // private methods:
 private:


  // protected members:
 protected:


  // private members:
 private:
  MString m_Name;
  MString m_ShortName;
  MString m_OriginalMGeantShortName;
  MString m_MGeantShortName;

  int m_ID;
  
  //! The material in ROOT notation
  TGeoMedium* m_GeoMedium;

  //! Hash value to cross check if cross sections file belongs to this class
  unsigned long m_Hash;

  double m_Density;
  int m_Sensitive;
  vector<MDMaterialComponent*> m_Components;
  double m_RadiationLength;

  static int m_IDCounter;

  MString m_CrossSectionFileDirectory;

  //! True if all macroscopic cross section have been read in correctly
  bool m_CrossSectionsPresent;
  //! The total macroscopic cross section
  MResponseMatrixO1 m_TotalCrossSection;
  //! The photo effect macroscopic cross section
  MResponseMatrixO1 m_PhotoCrossSection;
  //! The Compton scattering macroscopic cross section
  MResponseMatrixO1 m_ComptonCrossSection;
  //! The pair creation macroscopic cross section
  MResponseMatrixO1 m_PairCrossSection;
  //! The Rayleigh scattering macroscopic cross section
  MResponseMatrixO1 m_RayleighCrossSection;

  static MDMaterial* s_DummyMaterial;

  //! True if the global Doppler Limit file has already been loaded
  static bool s_DopplerLimitAllElementsLoaded;
  //! The global Doppler limit file:
  static MResponseMatrixO3 s_DopplerLimitAllElements;

  // Clone interface:
  //! List of all clones of this volume
  vector<MDMaterial*> m_Clones; 
  //!  Pointer to the clone template, 0 otherwise
  MDMaterial* m_CloneTemplate; 


#ifdef ___CINT___
 public:
  ClassDef(MDMaterial, 0) // material of a detector
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
