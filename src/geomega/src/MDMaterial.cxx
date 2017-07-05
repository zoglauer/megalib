/*
 * MDMaterial.cxx
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
// MDMaterial
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MDMaterial.h"
#include "MDMaterialComponent.h"

// Standard libs:
#include <iomanip>
#include <cmath>
#include <limits>
#include <sstream>
using namespace std;

// ROOT libs:
#include "TGeoElement.h"
#include "TGeoMaterial.h"

// MEGAlib libs:
#include "MAssert.h"
#include "MStreams.h"
#include "MFile.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MDMaterial)
#endif


////////////////////////////////////////////////////////////////////////////////


int MDMaterial::m_IDCounter = 1;
MDMaterial* MDMaterial::s_DummyMaterial = 0;
bool MDMaterial::s_DopplerLimitAllElementsLoaded = false;


////////////////////////////////////////////////////////////////////////////////


MDMaterial::MDMaterial()
{
  // default constructor

  // ID of this material:
  if (m_IDCounter == numeric_limits<int>::max()) {
    m_IDCounter = 0;
  } else {
    m_ID = m_IDCounter++;
  }

  m_Name = g_StringNotDefined;

  m_Density = g_DoubleNotDefined;
  m_Sensitive = g_IntNotDefined;
  m_RadiationLength = g_DoubleNotDefined;

  m_CrossSectionsPresent = false;
  m_CrossSectionFileDirectory = g_StringNotDefined;

  m_Hash = 0;
  
  m_GeoMedium = 0;
}

////////////////////////////////////////////////////////////////////////////////


MDMaterial::MDMaterial(MString Name)
{
  // default constructor

  // ID of this material:
  if (m_IDCounter == numeric_limits<int>::max()) {
    m_IDCounter = 0;
  } else {
    m_ID = m_IDCounter++;
  }

  SetName(Name);

  m_Density = g_DoubleNotDefined;
  m_Sensitive = g_IntNotDefined;
  m_RadiationLength = g_DoubleNotDefined;

  m_CrossSectionsPresent = false;
  m_CrossSectionFileDirectory = g_StringNotDefined;

  m_Hash = 0;
  
  m_GeoMedium = 0;
}


////////////////////////////////////////////////////////////////////////////////


MDMaterial::~MDMaterial()
{
  // default destructor

  for (unsigned int c = 0; c < m_Components.size(); ++c) {
    delete m_Components[c];
  }
  m_Components.clear();
  
  // delete m_GeoMedium; // ROOT deletes the ones it uses, but we do not know which ones here...
}

 
////////////////////////////////////////////////////////////////////////////////


MDMaterial* MDMaterial::GetDummyMaterial()
{
  // Return a dummy material, which currently is vacuum - 
  // but this might be changed in the future!

  if (s_DummyMaterial == 0) {
    // We do not have to care about the name, because it can not be used from
    // within the setup file
    s_DummyMaterial = new MDMaterial("GeomegaDummy");
    s_DummyMaterial->SetDensity(1E-12);
    s_DummyMaterial->SetRadiationLength(1E+12);
    s_DummyMaterial->SetComponentByAtomicWeighting("H", 1); 
    s_DummyMaterial->Validate();
  }

  return s_DummyMaterial;
}


////////////////////////////////////////////////////////////////////////////////


int MDMaterial::ConvertNameToAtomicNumber(MString Name)
{
  static const int MaxElements = 111;
  static const char* Elements[] = {"H" ,"He","Li","Be","B" ,"C" ,"N" ,"O" ,"F" ,"Ne","Na","Mg",
                                   "Al","Si","P" ,"S" ,"Cl","Ar","K" ,"Ca","Sc","Ti","V" ,"Cr",
                                   "Mn","Fe","Co","Ni","Cu","Zn","Ga","Ge","As","Se","Br","Kr",
                                   "Rb","Sr","Y" ,"Zr","Nb","Mo","Tc","Ru","Rh","Pd","Ag","Cd",
                                   "In","Sn","Sb","Te","I" ,"Xe","Cs","Ba","La","Ce","Pr","Nd",
                                   "Pm","Sm","Eu","Gd","Tb","Dy","Ho","Er","Tm","Yb","Lu","Hf",
                                   "Ta","W" ,"Re","Os","Ir","Pt","Au","Hg","Tl","Pb","Bi","Po",
                                   "At","Rn","Fr","Ra","Ac","Th","Pa","U" ,"Np","Pu","Am","Cm",
                                   "Bk","Cf","Es","Fm","Md","No","Lr","Rf","Db","Sg","Bh","Hs",
                                   "Mt","Ds","Rg"};
  for (int i = 0; i < MaxElements; ++i) {
    if (Name == Elements[i]) return i+1;
  }
  
  merr<<"Name not found:" <<Name<<endl;
  return 0;
}


////////////////////////////////////////////////////////////////////////////////


MString MDMaterial::ConvertAtomicNumberToName(int Z)
{
  static const int MaxElements = 111;
  static const char* Elements[] = {"H" ,"He","Li","Be","B" ,"C" ,"N" ,"O" ,"F" ,"Ne","Na","Mg",
                                   "Al","Si","P" ,"S" ,"Cl","Ar","K" ,"Ca","Sc","Ti","V" ,"Cr",
                                   "Mn","Fe","Co","Ni","Cu","Zn","Ga","Ge","As","Se","Br","Kr",
                                   "Rb","Sr","Y" ,"Zr","Nb","Mo","Tc","Ru","Rh","Pd","Ag","Cd",
                                   "In","Sn","Sb","Te","I" ,"Xe","Cs","Ba","La","Ce","Pr","Nd",
                                   "Pm","Sm","Eu","Gd","Tb","Dy","Ho","Er","Tm","Yb","Lu","Hf",
                                   "Ta","W" ,"Re","Os","Ir","Pt","Au","Hg","Tl","Pb","Bi","Po",
                                   "At","Rn","Fr","Ra","Ac","Th","Pa","U" ,"Np","Pu","Am","Cm",
                                   "Bk","Cf","Es","Fm","Md","No","Lr","Rf","Db","Sg","Bh","Hs",
                                   "Mt","Ds","Rg"};

  if (Z > 0 && Z <= MaxElements) {
    return Elements[Z-1];
  }
  
  merr<<"Element not found:" <<Z<<endl;
  return "";
}


////////////////////////////////////////////////////////////////////////////////


bool MDMaterial::SetDensity(double Density)
{
  // Set the density of this material

  if (Density <= 0.0) {
    mout<<"   ***  Error  ***  in material "<<m_Name<<endl;
    mout<<"The density must be positive: "<<Density<<endl;
    return false;    
  }
  
  m_Density = Density;
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MDMaterial::SetRadiationLength(double RadiationLength)
{
  // Set the radiation length of this material

  if (RadiationLength <= 0.0) {
    mout<<"   ***  Error  ***  in material "<<m_Name<<endl;
    mout<<"The radiation length must be positive: "<<RadiationLength<<endl;
    return false;    
  }
  
  m_RadiationLength = RadiationLength;
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MDMaterial::SetComponent(MDMaterialComponent* Component) 
{
  if (Component == nullptr) {
    mout<<"   ***  Error  ***  in material "<<m_Name<<endl;
    mout<<"The component is not allowed to be a null pointer."<<endl;
    return false;    
  }
  
  m_Components.push_back(Component);
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Set a component by atomic weighting
bool MDMaterial::SetComponentByAtomicWeighting(double AtomicWeight, int AtomicNumber, int Weighting)
{
  MDMaterialComponent* Component = new MDMaterialComponent();
  if (Component->SetByAtomicWeighting(AtomicWeight, AtomicNumber, Weighting) == false) {
    mout<<"   ***  Error  ***  in material "<<m_Name<<endl;
    mout<<"Unable to create the component."<<endl;    
    delete Component;
    return false;
  }
  
  m_Components.push_back(Component);
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Set a component by atomic weighting with natural composition
bool MDMaterial::SetComponentByAtomicWeighting(MString Name, int Weighting)
{
  MDMaterialComponent* Component = new MDMaterialComponent();
  if (Component->SetByAtomicWeighting(Name, Weighting) == false) {
    mout<<"   ***  Error  ***  in material "<<m_Name<<endl;
    mout<<"Unable to create the component."<<endl;    
    delete Component;
    return false;
  }
  
  m_Components.push_back(Component);
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Set a component by mass weighting
bool MDMaterial::SetComponentByMassWeighting(double AtomicWeight, int AtomicNumber, double Weighting)
{
  MDMaterialComponent* Component = new MDMaterialComponent();
  if (Component->SetByMassWeighting(AtomicWeight, AtomicNumber, Weighting) == false) {
    mout<<"   ***  Error  ***  in material "<<m_Name<<endl;
    mout<<"Unable to create the component."<<endl;    
    delete Component;
    return false;
  }
  
  m_Components.push_back(Component);
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Set a component by mass weighting with natural composition
bool MDMaterial::SetComponentByMassWeighting(MString Name, double Weighting)
{
  MDMaterialComponent* Component = new MDMaterialComponent();
  if (Component->SetByMassWeighting(Name, Weighting) == false) {
    mout<<"   ***  Error  ***  in material "<<m_Name<<endl;
    mout<<"Unable to create the component."<<endl;    
    delete Component;
    return false;
  }
  
  m_Components.push_back(Component);
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MDMaterial::SetCrossSectionFileDirectory(MString Directory)
{
  m_CrossSectionFileDirectory = Directory;
}


////////////////////////////////////////////////////////////////////////////////


MString MDMaterial::GetCrossSectionFileDirectory() const
{
  return m_CrossSectionFileDirectory;
}


////////////////////////////////////////////////////////////////////////////////


double MDMaterial::GetAbsorptionCoefficient(double Energy) const
{
  if (m_CrossSectionsPresent == false) {
    mout<<"   ***  Error  ***  in material "<<m_Name<<endl;
    mout<<"No cross section present."<<endl;    
    return 0;
  }
  
  return m_TotalCrossSection.GetInterpolated(Energy);
}


////////////////////////////////////////////////////////////////////////////////


double MDMaterial::GetComptonAbsorptionCoefficient(double Energy) const
{
  if (m_CrossSectionsPresent == false) {
    mout<<"   ***  Error  ***  in material "<<m_Name<<endl;
    mout<<"No cross section present."<<endl;    
    return 0;
  }
  
  return m_ComptonCrossSection.GetInterpolated(Energy);
}


////////////////////////////////////////////////////////////////////////////////


double MDMaterial::GetPhotoAbsorptionCoefficient(double Energy) const
{
  if (m_CrossSectionsPresent == false) {
    mout<<"   ***  Error  ***  in material "<<m_Name<<endl;
    mout<<"No cross section present."<<endl;    
    return 0;
  }
  
  return m_PhotoCrossSection.GetInterpolated(Energy);
}


////////////////////////////////////////////////////////////////////////////////


double MDMaterial::GetPairAbsorptionCoefficient(double Energy) const
{
  if (m_CrossSectionsPresent == false) {
    mout<<"   ***  Error  ***  in material "<<m_Name<<endl;
    mout<<"No cross section present."<<endl;    
    return 0;
  }
  
  return m_PairCrossSection.GetInterpolated(Energy);
}


////////////////////////////////////////////////////////////////////////////////


double MDMaterial::GetRayleighAbsorptionCoefficient(double Energy) const
{
  if (m_CrossSectionsPresent == false) {
    mout<<"   ***  Error  ***  in material "<<m_Name<<endl;
    mout<<"No cross section present."<<endl;    
    return 0;
  }
  
  return m_RayleighCrossSection.GetInterpolated(Energy);
}


////////////////////////////////////////////////////////////////////////////////


MString MDMaterial::GetGeomega() const
{
  // Return the content in Geomega format:

  ostringstream out;

  out<<"Material "<<m_Name<<endl;
  out<<m_Name<<".Density "<<m_Density<<endl;
  for (auto C : m_Components) {
    out<<m_Name<<C->GetGeomega()<<endl; 
  }
  out<<endl;

  return out.str().c_str();
}


////////////////////////////////////////////////////////////////////////////////


MDMaterialComponent* MDMaterial::GetComponentAt(unsigned int i) const
{
  // Return the ith Component of this material

  if (i < GetNComponents()) {
    return m_Components[i];
  } else {
    mout<<"MDComponent* MDMaterial::GetComponentAt(int i)"<<endl;
    mout<<"Index ("<<i<<") out of bounds (0, "<<GetNComponents()-1<<")"<<endl;
  }

  return nullptr;
}



////////////////////////////////////////////////////////////////////////////////


int MDMaterial::GetAtomicNumberMainComponent() const
{
  // Determine the main component of this material and return its atomic number:

  int AtomicNumber = 0;
  double Weight = 0;
  for (unsigned int i = 0; i < m_Components.size(); ++i) {
    if (m_Components[i]->GetWeightingByMass() > Weight) {
      AtomicNumber = m_Components[i]->GetAtomicNumber();
      Weight = m_Components[i]->GetWeightingByMass();
    } else if (m_Components[i]->GetWeightingByMass() == Weight) {
      if (m_Components[i]->GetAtomicNumber() > AtomicNumber) {
        AtomicNumber = m_Components[i]->GetAtomicNumber();
        Weight = m_Components[i]->GetWeightingByMass();
      }
    }
  }

  return AtomicNumber;
}


////////////////////////////////////////////////////////////////////////////////


MString MDMaterial::ToString() const
{
  // 

  ostringstream out;
  out<<"Material: "<<m_Name<<" - D:"<<m_Density<<", R:"<<m_RadiationLength<<", S:"<<m_Sensitive<<", C's:";
  for (unsigned int i = 0; i < m_Components.size(); i++) {
    out<<" ("<<i<<") "<<m_Components[i]->ToString();
  }
  out<<endl;

  return out.str().c_str();
}


////////////////////////////////////////////////////////////////////////////////


void MDMaterial::ResetIDs()
{
  // Reset the ID counter

  MDMaterial::m_IDCounter = 1;
}


////////////////////////////////////////////////////////////////////////////////


bool MDMaterial::Validate()
{
  // Do some sanity checks:

  if (m_Name == g_StringNotDefined) {
    mout<<"   ***  Error  ***  in material "<<m_Name<<endl;
    mout<<"Material has no name!!"<<endl;
    return false;    
  }

  if (GetNComponents() == 0) {
    mout<<"   ***  Error  ***  in material "<<m_Name<<endl;
    mout<<"Material has no components"<<endl;
    return false;
  }

  if (m_Density == g_DoubleNotDefined) {
    mout<<"   ***  Error  ***  in material "<<m_Name<<endl;
    mout<<"Material has no density"<<endl;
    return false;
  }

  if (m_Density <= 0) {
    mout<<"   ***  Error  ***  in material "<<m_Name<<endl;
    mout<<"Material has a non-positive  density"<<endl;
    return false;
  }

  for (unsigned int i = 0; i < m_Components.size(); ++i) {
    if (m_Components[i]->Validate() == false) {
      mout<<"   ***  Error  ***  in material "<<m_Name<<endl;
      mout<<"Unable to validate a component"<<endl;
      return false;      
    }
  }

  if (m_CrossSectionFileDirectory == g_StringNotDefined) {
    m_CrossSectionFileDirectory = g_MEGAlibPath + "/resource/geometries/materials";
  }


  if (GetNComponents() > 1) {
    // Check if all components have the same type
    MDMaterialComponentWeightingType Type = m_Components[0]->GetWeightingType();
    for (unsigned int i = 1; i < m_Components.size(); ++i) {
      if (Type != m_Components[i]->GetWeightingType()) {
        mout<<"   ***  Error  ***  in material "<<m_Name<<endl;
        mout<<"  All components have to be given either by"
            <<" fractional mass or by number of atoms"<<endl;
        return false;        
      }
    }
    
    if (Type == MDMaterialComponentWeightingType::c_ByMass) {
      /// Make sure the fractional mass sums up to one
      double Sum = 0;
      for (unsigned int i = 0; i < m_Components.size(); ++i) {
        Sum += m_Components[i]->GetWeightingByMass();
      }
      if (Sum != 1) {
        for (unsigned int i = 0; i < m_Components.size(); ++i) {
          m_Components[i]->SetWeightingByMass(m_Components[i]->GetWeightingByMass()/Sum);
        }     
      }
    }
  }

  if (m_RadiationLength == g_DoubleNotDefined) {
    m_RadiationLength = GetRootMedium()->GetMaterial()->GetRadLen();
  }

  // Now create a hopefully unique hash of this object
  vector<double> Array;
  Array.push_back(m_Density);
  Array.push_back(m_RadiationLength);
  for (unsigned int m = 0; m < m_Components.size(); ++m) {
    Array.push_back(m_Components[m]->GetAtomicWeight());
    Array.push_back(m_Components[m]->GetAtomicNumber());
    Array.push_back(m_Components[m]->GetWeightingByMass());
    Array.push_back(int(m_Components[m]->GetWeightingType()));
  }
  double* a = new double[Array.size()];
  copy(Array.begin(), Array.end(), a);
  m_Hash = TString::Hash(a, Array.size()*sizeof(double));
  delete [] a;


  // Load the absorptions
  // An error here does not result in an overall error, 
  // since the main validation in the geometry will check if the cross sections are present
  // and if not create them...
  LoadCrossSections();

//   // Load Doppler broadening info:
//   if (s_DopplerLimitAllElementsLoaded == false) {
//     MString DopplerFileName(g_MEGAlibPath + "/resource/geometries/material/DopplerLimit.MeanAbsoluteDeviation.AllElements.rsp");
//     if (MFile::FileExists(DopplerFileName) == false) {
//       mout<<"   ***  Error  ***  in material "<<m_Name<<endl;
//       mout<<"Can't find Doppler broadening info "<<DopplerFileName<<endl;
//       return false;
//     } else if (m_Absorption.Read(DopplerFileName) == false) {
//       mout<<"   ***  Error  ***  in material "<<m_Name<<endl;
//       mout<<"Can't open Doppler broadening info file "<<DopplerFileName<<endl;
//       return false;
//     }
//   }

  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MDMaterial::LoadCrossSections(bool Verbose)
{
  // Load absorption file:

  m_CrossSectionsPresent = true;

  MString TotalAbs = m_CrossSectionFileDirectory + "/Xsection.Total." + m_Name + ".rsp";
  MString PhotoAbs = m_CrossSectionFileDirectory + "/Xsection.Photo." + m_Name + ".rsp";
  MString ComptonAbs = m_CrossSectionFileDirectory + "/Xsection.Compton." + m_Name + ".rsp";
  MString PairAbs = m_CrossSectionFileDirectory + "/Xsection.Pair." + m_Name + ".rsp";
  MString RayleighAbs = m_CrossSectionFileDirectory + "/Xsection.Rayleigh." + m_Name + ".rsp";

  if (MFile::FileExists(TotalAbs) == false) {
    if (Verbose == true) {
      mout<<"   ***  Warning  ***  in material "<<m_Name<<endl;
      mout<<"Can't find cross section file "<<TotalAbs<<endl;
      mout<<"This is only a relevant problem if you want to calculate "
          <<"absorption probabilities with geomega."<<endl;
      mout<<"See the documentation for a step to step guide to solve this problem."<<endl;
    }
    m_CrossSectionsPresent = false;
  } else {
    if (m_TotalCrossSection.Read(TotalAbs) == false) {
      if (Verbose == true) {
        mout<<"   ***  Error  ***  in material "<<m_Name<<endl;
        mout<<"Can't open cross section file "<<TotalAbs<<endl;
      }
      m_CrossSectionsPresent = false;
      return false;
    } else {
      if (m_TotalCrossSection.GetHash() != m_Hash) {
        if (Verbose == true) {
          mout<<"   ***  Warning  ***  in material "<<m_Name<<endl;
          mout<<"Material has changed since last calculation of cross sections (Hashs: "<<m_TotalCrossSection.GetHash()<<" vs. "<<m_Hash<<")"<<endl;
          mout<<"This is only a relevant problem if you want to calculate "
              <<"absorption probabilities with geomega."<<endl;
          mout<<"See the documentation for a step to step guide to solve this problem."<<endl;
        }
        m_CrossSectionsPresent = false;
        m_TotalCrossSection.Init();
      }
    }
  }

  if (MFile::FileExists(PhotoAbs) == false) {
    if (Verbose == true) {
      mout<<"   ***  Warning  ***  in material "<<m_Name<<endl;
      mout<<"Can't find cross section file "<<PhotoAbs<<endl;
      mout<<"This is only a relevant problem if you want to calculate "
          <<"absorption probabilities with geomega."<<endl;
      mout<<"See the documentation for a step to step guide to solve this problem."<<endl;
    }
    m_CrossSectionsPresent = false;
  } else {
    if (m_PhotoCrossSection.Read(PhotoAbs) == false) {
      if (Verbose == true) {
        mout<<"   ***  Error  ***  in material "<<m_Name<<endl;
        mout<<"Can't open cross section file "<<PhotoAbs<<endl;
      }
      m_CrossSectionsPresent = false;
      return false;
    } else {
      if (m_PhotoCrossSection.GetHash() != m_Hash) {
        if (Verbose == true) {
          mout<<"   ***  Warning  ***  in material "<<m_Name<<endl;
          mout<<"Material has changed since last calculation of cross sections (Hashs: "<<m_PhotoCrossSection.GetHash()<<" vs. "<<m_Hash<<")"<<endl;
          mout<<"This is only a relevant problem if you want to calculate "
              <<"absorption probabilities with geomega."<<endl;
          mout<<"See the documentation for a step to step guide to solve this problem."<<endl;
        }
        m_CrossSectionsPresent = false;
        m_PhotoCrossSection.Init();
      }
    }
  }

  if (MFile::FileExists(ComptonAbs) == false) {
    if (Verbose == true) {
      mout<<"   ***  Warning  ***  in material "<<m_Name<<endl;
      mout<<"Can't find cross section file "<<ComptonAbs<<endl;
      mout<<"This is only a relevant problem if you want to calculate "
          <<"absorption probabilities with geomega."<<endl;
      mout<<"See the documentation for a step to step guide to solve this problem."<<endl;
    }
    m_CrossSectionsPresent = false;
  } else {
    if (m_ComptonCrossSection.Read(ComptonAbs) == false) {
      if (Verbose == true) {
        mout<<"   ***  Error  ***  in material "<<m_Name<<endl;
        mout<<"Can't open cross section file "<<ComptonAbs<<endl;
      }
      m_CrossSectionsPresent = false;
      return false;
    } else {
      if (m_ComptonCrossSection.GetHash() != m_Hash) {
        if (Verbose == true) {
          mout<<"   ***  Warning  ***  in material "<<m_Name<<endl;
          mout<<"Material has changed since last calculation of cross sections (Hashs: "<<m_ComptonCrossSection.GetHash()<<" vs. "<<m_Hash<<")"<<endl;
          mout<<"This is only a relevant problem if you want to calculate "
              <<"absorption probabilities with geomega."<<endl;
          mout<<"See the documentation for a step to step guide to solve this problem."<<endl;
        }
        m_CrossSectionsPresent = false;
        m_ComptonCrossSection.Init();
      }
    }
  }

  if (MFile::FileExists(PairAbs) == false) {
    if (Verbose == true) {
      mout<<"   ***  Warning  ***  in material "<<m_Name<<endl;
      mout<<"Can't find cross section file "<<PairAbs<<endl;
      mout<<"This is only a relevant problem if you want to calculate "
          <<"absorption probabilities with geomega."<<endl;
      mout<<"See the documentation for a step to step guide to solve this problem."<<endl;
    }
    m_CrossSectionsPresent = false;
  } else {
    if (m_PairCrossSection.Read(PairAbs) == false) {
      if (Verbose == true) {
        mout<<"   ***  Error  ***  in material "<<m_Name<<endl;
        mout<<"Can't open cross section file "<<PairAbs<<endl;
      }
      m_CrossSectionsPresent = false;
      return false;
    } else {
      if (m_PairCrossSection.GetHash() != m_Hash) {
        if (Verbose == true) {
          mout<<"   ***  Warning  ***  in material "<<m_Name<<endl;
          mout<<"Material has changed since last calculation of cross sections (Hashs: "<<m_PairCrossSection.GetHash()<<" vs. "<<m_Hash<<")"<<endl;
          mout<<"This is only a relevant problem if you want to calculate "
              <<"absorption probabilities with geomega."<<endl;
          mout<<"See the documentation for a step to step guide to solve this problem."<<endl;
        }
        m_CrossSectionsPresent = false;
        m_PairCrossSection.Init();
      }
    }
  }

  if (MFile::FileExists(RayleighAbs) == false) {
    if (Verbose == true) {
      mout<<"   ***  Warning  ***  in material "<<m_Name<<endl;
      mout<<"Can't find cross section file "<<RayleighAbs<<endl;
      mout<<"This is only a relevant problem if you want to calculate "
          <<"absorption probabilities with geomega."<<endl;
      mout<<"See the documentation for a step to step guide to solve this problem."<<endl;
    }
    m_CrossSectionsPresent = false;
  } else {
    if (m_RayleighCrossSection.Read(RayleighAbs) == false) {
      if (Verbose == true) {
        mout<<"   ***  Error  ***  in material "<<m_Name<<endl;
        mout<<"Can't open cross section file "<<RayleighAbs<<endl;
      }
      m_CrossSectionsPresent = false;
      return false;
    } else {
      if (m_RayleighCrossSection.GetHash() != m_Hash) {
        if (Verbose == true) {
          mout<<"   ***  Warning  ***  in material "<<m_Name<<endl;
          mout<<"Material has changed since last calculation of cross sections (Hashs: "<<m_RayleighCrossSection.GetHash()<<" vs. "<<m_Hash<<")"<<endl;
          mout<<"This is only a relevant problem if you want to calculate "
              <<"absorption probabilities with geomega."<<endl;
          mout<<"See the documentation for a step to step guide to solve this problem."<<endl;
        }
        m_CrossSectionsPresent = false;
        m_RayleighCrossSection.Init();
      }
    }
  }
    
  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MDMaterial::AddClone(MDMaterial* Clone)
{
  // Add the pointer to a volume which is a clone of this volume 

  m_Clones.push_back(Clone);

  Clone->SetCloneTemplate(this);
}


////////////////////////////////////////////////////////////////////////////////


unsigned int MDMaterial::GetNClones() const
{
  // return the number clones of this volume

  return m_Clones.size();
}

////////////////////////////////////////////////////////////////////////////////


MDMaterial* MDMaterial::GetCloneAt(unsigned int i) const
{
  // Return the clone at position i

  if (i < GetNClones()) {
    return m_Clones[i];
  } else {
    Fatal("MDMaterial* MDMaterial::GetCloneAt(int i)",
          "Index (%d) out of bounds (%d, %d)", i, 0, GetNClones()-1);
  }

  return 0;
}


////////////////////////////////////////////////////////////////////////////////


unsigned int MDMaterial::GetCloneId(MDMaterial* Clone) const
{
  // Get the ID of the clone

  for (unsigned int i = 0; i < m_Clones.size(); ++i) {
    if (Clone == m_Clones[i]) return i;
  }
  
  return numeric_limits<unsigned int>::max();
}


////////////////////////////////////////////////////////////////////////////////


void MDMaterial::SetCloneTemplate(MDMaterial* CloneTemplate)
{
  // This volume is a clone of volume CloneTemplate

  m_CloneTemplate = CloneTemplate;
}


////////////////////////////////////////////////////////////////////////////////


bool MDMaterial::CopyDataToClones()
{
  // Copy all data to clone material, whose data is on default values

  MDMaterial* Clone = 0;
  for (unsigned i = 0; i < m_Clones.size(); ++i) {
    Clone = m_Clones[i];
    
    if (Clone->GetDensity() == g_DoubleNotDefined) {
      Clone->SetDensity(m_Density);
    }
    if (Clone->GetRadiationLength() == g_DoubleNotDefined) {
      Clone->SetRadiationLength(m_RadiationLength);
    }
    if (Clone->GetNComponents() == 0) {
      for (unsigned int i = 0; i < m_Components.size(); ++i) {
        MDMaterialComponent* C = new MDMaterialComponent(*m_Components[i]);
        Clone->SetComponent(C);
      }
    }
    if (Clone->GetCrossSectionFileDirectory() == g_StringNotDefined) {
      Clone->SetCrossSectionFileDirectory(m_CrossSectionFileDirectory);
    }
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


TGeoMedium* MDMaterial::GetRootMedium() 
{
  //! Return this material as ROOT medium
  
  if (m_GeoMedium == 0) {
    TGeoMixture* Material = new TGeoMixture(m_Name, m_Components.size(), m_Density);
    for (unsigned int i = 0; i < m_Components.size(); ++i) {
      MDMaterialComponent* C = m_Components[i];
      TGeoElement* Element = new TGeoElement("E", "E", C->GetAtomicNumber(), C->GetAtomicWeight());
      if (C->GetWeightingType() == MDMaterialComponentWeightingType::c_ByMass) {
        Material->AddElement(Element, C->GetWeightingByMass());
      } else {
        Material->AddElement(Element, C->GetWeightingByAtoms());      
      }
    }
    m_GeoMedium = new TGeoMedium(m_Name, m_IDCounter, Material);
  }
  
  return m_GeoMedium;
}


// MDMaterial.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
