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
  m_ShortName = g_StringNotDefined;
  m_OriginalMGeantShortName = g_StringNotDefined;
  m_MGeantShortName = g_StringNotDefined;

  m_Density = g_DoubleNotDefined;
  m_Sensitive = g_IntNotDefined;
  m_RadiationLength = g_DoubleNotDefined;

  m_CrossSectionsPresent = false;
  m_CrossSectionFileDirectory = g_StringNotDefined;

  m_Hash = 0;
  
  m_GeoMedium = 0;
}

////////////////////////////////////////////////////////////////////////////////


MDMaterial::MDMaterial(MString Name, MString ShortName, MString MGeantShortName)
{
  // default constructor

  // ID of this material:
  if (m_IDCounter == numeric_limits<int>::max()) {
    m_IDCounter = 0;
  } else {
    m_ID = m_IDCounter++;
  }

  SetName(Name, ShortName, MGeantShortName);

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
    s_DummyMaterial = new MDMaterial("GeomegaDummy", "GDUM", "GeomegaDummy");
    s_DummyMaterial->SetDensity(1E-12);
    s_DummyMaterial->SetRadiationLength(1E+12);
    s_DummyMaterial->SetComponent(1.0, 1.0, 1.0, MDMaterialComponent::c_ByAtoms);
    s_DummyMaterial->SetSensitivity(1);
    s_DummyMaterial->Validate();
  }

  return s_DummyMaterial;
}


////////////////////////////////////////////////////////////////////////////////


int MDMaterial::ConvertZToNumber(MString Name)
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


MString MDMaterial::ConvertZToString(int Z)
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


void MDMaterial::SetName(MString Name, MString ShortName, MString MGeantShortName)
{
  // Set all names...

  m_Name = Name;
  m_ShortName = ShortName;
  m_OriginalMGeantShortName = MGeantShortName;
  m_MGeantShortName = "M" + MGeantShortName;
  m_MGeantShortName.ToLower();
}


////////////////////////////////////////////////////////////////////////////////


MString MDMaterial::GetName() const
{
  // Return the name of this material
  
  return m_Name;
}


////////////////////////////////////////////////////////////////////////////////


MString MDMaterial::GetShortName() const
{
  // Return the name of this material
  
  return m_ShortName;
}


////////////////////////////////////////////////////////////////////////////////


MString MDMaterial::GetMGeantShortName() const
{
  // Return the name of this material
  
  return m_MGeantShortName;
}


////////////////////////////////////////////////////////////////////////////////


MString MDMaterial::GetOriginalMGeantShortName() const
{
  // Return the short name created by MDGeometry::CreateShortName()
  // Only used in ShortNameExists!
  
  return m_OriginalMGeantShortName;
}


////////////////////////////////////////////////////////////////////////////////


void MDMaterial::SetDensity(double Density)
{
  // Set the density of this material

  m_Density = Density;
}


////////////////////////////////////////////////////////////////////////////////


void MDMaterial::SetRadiationLength(double RadiationLength)
{
  // Set the radiation length of this material

  massert(RadiationLength > 0);

  m_RadiationLength = RadiationLength;
}


////////////////////////////////////////////////////////////////////////////////


void MDMaterial::SetComponent(MDMaterialComponent* C) 
{
  m_Components.push_back(C); 
}


////////////////////////////////////////////////////////////////////////////////


void MDMaterial::SetComponent(double A, double Z, 
                              double Weight, int Type)
{
  // The the component data, atomic mass, number of protons number of atoms per molecule

  m_Components.push_back(new MDMaterialComponent(A, Z, Weight, Type));
}


////////////////////////////////////////////////////////////////////////////////


bool MDMaterial::SetComponent(MString Name, double Weight, int Type)
{
  // The the component data, atomic mass, number of protons number of atoms per molecule

  MDMaterialComponent* C = new MDMaterialComponent();
  if (C->SetElement(Name) == false) {
    delete C;
    return false;
  }
  C->SetWeight(Weight);
  C->SetType(Type);
  
  m_Components.push_back(C);
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MDMaterial::SetSensitivity(int Sensitive)
{
  // 1 if this is a sensitive material

  massert(Sensitive >= 0);

  m_Sensitive = Sensitive;
}


////////////////////////////////////////////////////////////////////////////////


int MDMaterial::GetID() const
{
  //

  return m_ID;
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
  if (m_CrossSectionsPresent == false) return 0;
  return m_TotalCrossSection.GetInterpolated(Energy);
}


////////////////////////////////////////////////////////////////////////////////


double MDMaterial::GetComptonAbsorptionCoefficient(double Energy) const
{
  if (m_CrossSectionsPresent == false) return 0;
  return m_ComptonCrossSection.GetInterpolated(Energy);
}


////////////////////////////////////////////////////////////////////////////////


double MDMaterial::GetPhotoAbsorptionCoefficient(double Energy) const
{
  if (m_CrossSectionsPresent == false) return 0;
  return m_PhotoCrossSection.GetInterpolated(Energy);
}


////////////////////////////////////////////////////////////////////////////////


double MDMaterial::GetPairAbsorptionCoefficient(double Energy) const
{
  if (m_CrossSectionsPresent == false) return 0;
  return m_PairCrossSection.GetInterpolated(Energy);
}


////////////////////////////////////////////////////////////////////////////////


double MDMaterial::GetRayleighAbsorptionCoefficient(double Energy) const
{
  if (m_CrossSectionsPresent == false) return 0;
  return m_RayleighCrossSection.GetInterpolated(Energy);
}


////////////////////////////////////////////////////////////////////////////////


MString MDMaterial::GetGeant3DIM() const
{
  // Return the Geant3 dimensions

  if (m_Components.size() == 1) return MString("");

  ostringstream out;

  //  out<<"******     Material: "<<m_Name<<endl;
  out<<"      REAL A"<<m_ShortName<<",Z"<<m_ShortName<<",W"<<m_ShortName<<endl;
  out<<"      DIMENSION A"<<m_ShortName<<"("<<m_Components.size()<<"),Z"
     <<m_ShortName<<"("<<m_Components.size()<<"),W"
     <<m_ShortName<<"("<<m_Components.size()<<")"<<endl<<endl;  

  return out.str().c_str();
}


////////////////////////////////////////////////////////////////////////////////


MString MDMaterial::GetGeant3DATA() const
{
  // Return the DATA.string for geant3

  ostringstream out;

  //if (m_Components->GetLast() == 0) return MString("");
  if (m_Components.size() == 1) return MString("");
  out.setf(ios::fixed, ios::floatfield);
  //out.precision(2);
  out<<"      DATA A"<<m_ShortName<<"/"<<m_Components[0]->GetA();
  for (unsigned int i = 1; i < m_Components.size(); i++) {
    out<<","<<m_Components[i]->GetA();
  }
  out<<"/"<<endl;
      
  out<<"      DATA Z"<<m_ShortName<<"/"<<m_Components[0]->GetZ();
  for (unsigned int i = 1; i < m_Components.size(); i++) {
    out<<","<<m_Components[i]->GetZ();
  }
  out<<"/"<<endl;

  out<<"      DATA W"<<m_ShortName<<"/"<<m_Components[0]->GetWeight();
  for (unsigned int i = 1; i < m_Components.size(); i++) {
    out<<","<<m_Components[i]->GetWeight();
  }
  out<<"/"<<endl;

  return out.str().c_str();
}


////////////////////////////////////////////////////////////////////////////////


MString MDMaterial::GetGeant3() const
{
  // Return the GSMATE/GSMIXT tables

  ostringstream out;

  out.setf(ios::scientific, ios::floatfield);
  //out.precision(3);

  out<<endl<<"***** Material: "<<m_Name<<endl;
  if (m_Components.size() == 1) {
    out<<"      CALL GSMATE("<<m_ID<<",'M"<<m_ShortName<<"$',"
       <<m_Components[0]->GetA()<<","<<m_Components[0]->GetZ()
       <<","<<m_Density<<","<<m_RadiationLength<<",0.0,UBUF,0)"<<endl;
  } else {
    if (m_Components[0]->GetType() == MDMaterialComponent::c_ByAtoms) {
      out<<"      CALL GSMIXT("<<m_ID<<",'M"<<m_ShortName<<"$',A"
         <<m_ShortName<<",Z"<<m_ShortName<<","<<m_Density
         <<",-"<<m_Components.size()<<",W"<<m_ShortName<<")"<<endl;
    } else {
      out<<"      CALL GSMIXT("<<m_ID<<",'M"<<m_ShortName<<"$',A"
         <<m_ShortName<<",Z"<<m_ShortName<<","<<m_Density
         <<",+"<<m_Components.size()<<",W"<<m_ShortName<<")"<<endl;
    }
  }

  out<<"      CALL GSTMED("<<m_ID<<",'M"<<m_ShortName<<"$',"
     <<m_ID<<","<<m_Sensitive<<",0,0.,0.,-1.E-4,-1.E-6,1.E-3,-0.,UBUF,0)"<<endl;
  out<<"      ZMNAME("<<m_ID<<") = \""<<m_Name<<"\""<<endl;

  return out.str().c_str();
}


////////////////////////////////////////////////////////////////////////////////


MString MDMaterial::GetMGeant() const
{
  // Fetch material data in MGEANT/MGGPOD format
  // Guest author RMK Jan-2004

  ostringstream out;
  //out.precision(3);

  // Z <= 100 is enforced by MDMaterialComponent!
  const char* elem[] = {"H ","He","Li","Be","B ","C ","N ","O ","F ","Ne","Na","Mg",
      "Al","Si","P ","S ","Cl","Ar","K ","Ca","Sc","Ti","V ","Cr",
      "Mn","Fe","Co","Ni","Cu","Zn","Ga","Ge","As","Se","Br","Kr",
      "Rb","Sr","Y ","Zr","Nb","Mo","Tc","Ru","Rh","Pd","Ag","Cd",
      "In","Sn","Sb","Te","I ","Xe","Cs","Ba","La","Ce","Pr","Nd",
      "Pm","Sm","Eu","Gd","Tb","Dy","Ho","Er","Tm","Yb","Lu","Hf",
      "Ta","W ","Re","Os","Ir","Pt","Au","Hg","Tl","Pb","Bi","Po",
      "At","Rn","Fr","Ra","Ac","Th","Pa","U ","Np","Pu","Am","Cm",
      "Bk","Cf","Es","Fm","Md","No","Lr"};

  out<<endl<<"! "<<m_Name<<endl;

  MString ShortName = m_MGeantShortName;
  ShortName.ToUpper();

  // For a single-component material (MGEANT user materials start at number 17)
  if (GetNComponents() == 1) {


    // using geant MATE only for vacuum:
    if (GetComponentAt(0)->GetZ() == 0 || GetComponentAt(0)->GetA() == 0) {
      out<<"mate "<<setw(3)<<m_ID+16<<"  "<<ShortName<<"  ";
      out<<m_Components[0]->GetA()<<"  ";
      out<<m_Components[0]->GetZ()<<"  ";
      out.setf(ios::scientific, ios::floatfield);
      out<<m_Density<<"  "<<m_RadiationLength<<" "<<" 1.0  0"<<endl;
      out.setf(ios::fmtflags(0), ios::floatfield);
    } 
    // using MGEANT umix to get natural isotopic abundance
    else {
      // This has been enforced elsewhere, but...
      int Element = 1;
      if (int(GetComponentAt(0)->GetZ()) < 1) {
        mout<<"   ***  Severe warning  ***  in material "<<m_Name<<endl;
        mout<<"Element has Z < 1:"<<GetComponentAt(0)->GetZ()<<endl;
        Element = 1;
      }  else if (int(GetComponentAt(0)->GetZ()) > 100) {
        mout<<"   ***  Severe warning  ***  in material "<<m_Name<<endl;
        mout<<"Element has Z > 100:"<<GetComponentAt(0)->GetZ()<<endl;
        Element = 100;        
      } else {
        Element = int(GetComponentAt(0)->GetZ());
      }

      out<<"umix "<<setw(3)<<m_ID+16<<"  "<<ShortName<<"  "
         <<GetNComponents()<<"  ";
      out.setf(ios::scientific, ios::floatfield);
      out<<m_Density<<endl;
      out.setf(ios::fmtflags(0), ios::floatfield);
      out<<"               "
         <<"0.0  "<<elem[Element-1]
         <<"  1"<<endl;
    }


  // For a multi-component mixture (MGEANT user materials start at number 17)
  // Note that GeoMega only supports mixtures through specification of 
  // the number of atoms per component (negative number of components), 
  // not fraction by weight
  } else {     

    // using the standard MGEANT MIXT (one isotope per component)
    //     out << "mixt " << m_ID+16 << "  MAT" << m_Name.Data() << "    "
    //  << "-" << GetNComponents() << "  " << m_Density << endl;
    //     for (unsigned int i = 0; i < GetNComponents(); i++) {
    //       out << "                    "
    //    << GetComponentAt(i)->GetA() << " "
    //    << GetComponentAt(i)->GetZ() << " "
    //    << GetComponentAt(i)->GetNAtoms() << endl;
    //     }   
    //     out << endl;

    // using the MGEANT UMIX (natural isotopic abmudance)
    if (m_Components[0]->GetType() == MDMaterialComponent::c_ByMass) {
      out<<"umix "<<setw(3)<<m_ID+16<<"  ";
      out<<ShortName<<"  ";
      out<<GetNComponents()<<"  ";
      out.setf(ios::scientific, ios::floatfield);
      out<<m_Density<<endl;
      out.setf(ios::fmtflags(0), ios::floatfield);
    } else {
      out<<"umix "<<setw(3)<<m_ID+16<<"  ";
      out<<ShortName<<"  ";
      out<<"-"<<GetNComponents()<<"  ";
      out.setf(ios::scientific, ios::floatfield);
      out<<m_Density<<endl;
      out.setf(ios::fmtflags(0), ios::floatfield);
    }
    for (unsigned int i = 0; i < GetNComponents(); i++) {
      int Element = 1;
      if (int(GetComponentAt(i)->GetZ()) < 1) {
        mout<<"   ***  Severe warning  ***  in material "<<m_Name<<endl;
        mout<<"Element has Z < 1:"<<GetComponentAt(i)->GetZ()<<endl;
        Element = 1;
      }  else if (int(GetComponentAt(i)->GetZ()) > 100) {
        mout<<"   ***  Severe warning  ***  in material "<<m_Name<<endl;
        mout<<"Element has Z > 100:"<<GetComponentAt(i)->GetZ()<<endl;
        Element = 100;        
      } else {
        Element = int(GetComponentAt(i)->GetZ());
      }

      out<<"               "
          <<"0.0  "<<elem[Element-1]<<"  "
          <<GetComponentAt(i)->GetWeight()<<endl;
    }   
  }

  return out.str().c_str();
}


////////////////////////////////////////////////////////////////////////////////


MString MDMaterial::GetMGeantTmed(int Sensitivity) const
{
  // Fetch tracking media data in MGEANT/MGGPOD format
  // Guest author RMK Jan-2004

  ostringstream out;

  if (Sensitivity < 0) Sensitivity = m_Sensitive;

  MString Name = m_MGeantShortName;
  Name.ToUpper();

  // Set sensitivity keyword based on sensitivity flag
  if (Sensitivity == 1) {
    out<<"tmed "<<setw(3)<<m_ID<<"  "<<Name<<" "<<Name<<" "
       <<"dete"<<" 0  0.0  -1.0  -1.0  -1.0  1.0e-4  -1.0  0";
  } else if (Sensitivity == 2) {
    out<<"tmed "<<setw(3)<<m_ID<<"  "<<Name<<" "<<Name<<" "
       <<"dete"<<" 0  0.0  -1.0  -1.0  -1.0  1.0e-4  -1.0  0";
  } else {
    out<<"tmed "<<setw(3)<<m_ID<<"  "<<Name<<" "<<Name<<" "
       <<"pass"<<" 0  0.0  -1.0  -1.0  -1.0  1.0e-4  -1.0  0";
  }

  out << endl;

  return out.str().c_str();
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

  return 0;
}


////////////////////////////////////////////////////////////////////////////////


unsigned int MDMaterial::GetNComponents() const
{
  // Return the number of components of this material

  return m_Components.size();
}


////////////////////////////////////////////////////////////////////////////////


int MDMaterial::GetZMainComponent() const
{
  // Determine the main component of this material and return its Z:

  double Z = 0;
  double Weight = 0;
  for (unsigned int i = 0; i < m_Components.size(); ++i) {
    if (m_Components[i]->GetWeight() > Weight) {
      Z = m_Components[i]->GetZ();
      Weight = m_Components[i]->GetWeight();
    } else if (m_Components[i]->GetWeight() == Weight) {
      if (m_Components[i]->GetZ() > Z) {
        Z = m_Components[i]->GetZ();
        Weight = m_Components[i]->GetWeight();
      }
    }
  }

  return int(Z+0.5);
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

  // It is not good to require (Geant3/MGEANT/MGGPOD) short names...

  if (m_ShortName == g_StringNotDefined) {
    mout<<"   ***  Warning  ***  in material "<<m_Name<<endl;
    mout<<"Material has no short name (only relevant for Geant3)!!"<<endl;
  }

  if (m_MGeantShortName == g_StringNotDefined) {
    mout<<"   ***  Warning  ***  in material "<<m_Name<<endl;
    mout<<"Material has no MGeant/MGGPOD short name (only relevant for MGeant/MGGPOD)!!"<<endl;
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


  if (GetNComponents() == 1) {
    // Calculate radiation length:
    // According to: Physical Review D: Particles and fields

    const double F = 1/137.036; // Fine structure constant
    const double r = 2.81794092E-13; // cm: Classical electron radius
    const double Na = 6.0221367E+23; // /mol: Avogadro's number

    double d = m_Density;
    double A = GetComponentAt(0)->GetA(); //
    double Z = GetComponentAt(0)->GetZ(); //

    double FZ2 = F*F*Z*Z;
    double Fc = // Coulomb correction function
      FZ2*(1.0/(1+FZ2) + 0.20206 - 0.0369*FZ2 + 0.0083*FZ2*FZ2 - 0.0020*FZ2*FZ2*FZ2);

    double Lrad;
    double Lradl;
    if (Z == 1) {
      Lrad = 5.31;
      Lradl = 6.144;
    } else if (Z == 2) { 
      Lrad = 4.79;
      Lradl = 5.621;
    } else if (Z == 3) { 
      Lrad = 4.74;
      Lradl = 5.805;
    } else if (Z == 4) { 
      Lrad = 4.71;
      Lradl = 5.924;
    } else { 
      Lrad = log(184.15/pow(Z,1.0/3.0));
      Lradl = log(1194/pow(Z,2.0/3.0));
    }

    double Rad = 4.0*F*r*r*Na*1/A*(Z*Z*(Lrad-Fc)+Z*Lradl);
    Rad = 1.0/(d*Rad);

    if (Rad < 1000) {
      if (m_RadiationLength == g_DoubleNotDefined) {
        //mout<<"   ***  Warning  ***  in material "<<m_Name<<endl;
        //mout<<"  No radiation length given!"<<endl;
        //mout<<"  Using calculated one: "<<Rad<<endl;
        m_RadiationLength = Rad;
      } else if (fabs(1 - m_RadiationLength/Rad) > 0.05) {
        mout<<"   ***  Error  ***  in material "<<m_Name<<endl;
        mout<<"  Given radiation length ("<<m_RadiationLength
            <<") and calculated one ("<<Rad<<") differ by more than 5%!"<<endl;
        return false;
      } 
    }
  } else {
    // Check if all components have the same type
    int Type = m_Components[0]->GetType();
    for (unsigned int i = 1; i < m_Components.size(); ++i) {
      if (Type != m_Components[i]->GetType()) {
        mout<<"   ***  Error  ***  in material "<<m_Name<<endl;
        mout<<"  All components have to be given either by"
            <<" fractional mass or by number of atoms"<<endl;
        return false;        
      }
    }
    
    if (Type == MDMaterialComponent::c_ByMass) {
      /// Make sure the fractional mass sums up to one
      double Sum = 0;
      for (unsigned int i = 0; i < m_Components.size(); ++i) {
        Sum += m_Components[i]->GetWeight();
      }
      if (Sum != 1) {
        for (unsigned int i = 0; i < m_Components.size(); ++i) {
          m_Components[i]->SetWeight(m_Components[i]->GetWeight()/Sum);
        }     
      }
    } else {
      // Make sure we have integers!
      double Tolerance = 1E-8;
      for (unsigned int i = 0; i < m_Components.size(); ++i) {
        if (fabs(int(m_Components[i]->GetWeight() + Tolerance) - m_Components[i]->GetWeight()) > Tolerance) {
          mout<<"   ***  Error  ***  in material "<<m_Name<<endl;
          mout<<"  All components of a \"by atoms\" material"
              <<" have to be integers!"<<endl;
          return false;                 
        }
      }      
    }
  }


  // Now create a hopefully unique hash of this object
  vector<double> Array;
  Array.push_back(m_Density);
  Array.push_back(m_RadiationLength);
  for (unsigned int m = 0; m < m_Components.size(); ++m) {
    Array.push_back(m_Components[m]->GetA());
    Array.push_back(m_Components[m]->GetZ());
    Array.push_back(m_Components[m]->GetWeight());
    Array.push_back(m_Components[m]->GetType());
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
        MDMaterialComponent* C = m_Components[i];
        Clone->SetComponent(C->GetA(), C->GetZ(), C->GetWeight(), C->GetType());
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
      TGeoElement* Element = new TGeoElement("E", "E", C->GetZ(), C->GetA());
      if (C->GetType() == MDMaterialComponent::c_ByMass) {
        Material->AddElement(Element, double(C->GetWeight()));
      } else {
        Material->AddElement(Element, int(C->GetWeight()));      
      }
    }
    m_GeoMedium = new TGeoMedium(m_Name, m_IDCounter, Material);
  }
  
  return m_GeoMedium;
}


// MDMaterial.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
