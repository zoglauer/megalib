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


//! Class representing a geomega material
class MDMaterial
{
  // public interface:
 public:
  //! Standard constructor
  MDMaterial();
  //! Default constructor with name
  MDMaterial(MString Name);
  //! Standard destructor
  virtual ~MDMaterial();

  //! Return a dummy material
  static MDMaterial* GetDummyMaterial();

  //!
  static int ConvertNameToAtomicNumber(MString Name);
  static MString ConvertAtomicNumberToName(int Z);

  //! Set the name
  void SetName(MString Name) { m_Name = Name; }
  //! Get the name
  MString GetName() const { return m_Name; }

  //! Set the density of the material
  bool SetDensity(double Density);
  //! Return the density of the material
  double GetDensity() const { return m_Density; }

  //! Set the radiation length of the material
  bool SetRadiationLength(double RadiationLength);
  //! Return the radiation length of the material
  double GetRadiationLength() const { return m_RadiationLength; }

  //! Set the component
  bool SetComponent(MDMaterialComponent* Component);
  //! Set a component by atomic weighting
  bool SetComponentByAtomicWeighting(double AtomicWeight, int AtomicNumber, int Weighting);
  //! Set a component by atomic weighting with natural composition
  bool SetComponentByAtomicWeighting(MString Name, int Weighting);
  //! Set a component by mass weighting
  bool SetComponentByMassWeighting(double AtomicWeight, int AtomicNumber, double Weighting);
  //! Set a component by mass weighting with natural composition
  bool SetComponentByMassWeighting(MString Name, double Weighting);
  //! Get the component by index
  //! Return nullptr if the index is out of bound
  MDMaterialComponent* GetComponentAt(unsigned int i) const;
  //! Get the number of component 
  unsigned int GetNComponents() const { return m_Components.size(); }

  //! Set the directory where the cross sections are stored
  void SetCrossSectionFileDirectory(MString Directory);
  //! Return the directory where the cross sections are stored
  MString GetCrossSectionFileDirectory() const;

  //! return true if we have all cross sections ready
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

  //! Return the unique ID of the material
  int GetID() const { return m_ID; }

  // Determine the main component of this material and return its atomic number:
  int GetAtomicNumberMainComponent() const;

  
  //! Return in Geomega compatible format
  MString GetGeomega() const;
  //! Return a description of the material
  MString ToString() const; 

  //! Reset the ID counter
  static void ResetIDs();

  //! Validate the data
  bool Validate();
  //! Load the cross-sections
  bool LoadCrossSections(bool Verbose = false);

  // Clone interface... identical to MDVolume, MDDetector...
  
  //! Add a material clone
  void AddClone(MDMaterial* Clone);
  //! Return the number of clones
  unsigned int GetNClones() const;
  //! Get a specific clone
  MDMaterial* GetCloneAt(unsigned int i) const;
  //! Get the ID from a specific clone
  unsigned int GetCloneId(MDMaterial* Clone) const;
  //! Return true if this is a clone
  bool IsClone() const;

  //! Set the volume which is the template for this clone
  void SetCloneTemplate(MDMaterial* Template);
  //! Get the volumes which is the template for this clone
  MDMaterial* GetCloneTemplate() const;

  //! Copy all the common data to the clones
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
  //! The name of the material
  MString m_Name;

  //! The unique ID of the material
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


#ifdef ___CLING___
 public:
  ClassDef(MDMaterial, 0) // material of a detector
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
