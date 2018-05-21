/*
 * MDShapePCON.cxx
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
// MDShapePCON
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MDShapePCON.h"

// Standard libs:
#include <sstream>
using namespace std;

// ROOT libs:
#include <TGeoPcon.h>

// MEGAlib libs:
#include "MStreams.h"
#include "MAssert.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MDShapePCON)
#endif


////////////////////////////////////////////////////////////////////////////////


MDShapePCON::MDShapePCON(const MString& Name) : MDShape(Name)
{
  // Standard constructor

  m_Phi = 0;
  m_DPhi = 0;
  m_NSections = 0;

  m_Type = "PCON";
}


////////////////////////////////////////////////////////////////////////////////


MDShapePCON::~MDShapePCON()
{
  // Default destructor
}

////////////////////////////////////////////////////////////////////////////////


bool MDShapePCON::Set(double Phi, double DPhi, unsigned int NSections)
{
  // Correctly initialize this shape

  if (Phi < 0 || Phi >= 360) {
    mout<<"   ***  Error  ***  in shape PCON "<<endl;
    mout<<"Phi ("<<Phi<<") needs to be within [0;360["<<endl;
    return false;            
  }
  if (DPhi <= 0 || DPhi > 360) {
    mout<<"   ***  Error  ***  in shape PCON "<<endl;
    mout<<"Phimax ("<<DPhi<<") of shape PCON needs to be within ]0;360]"<<endl;
    return false;            
  }
  
  if (NSections < 2) {
    mout<<"   ***  Error  ***  in shape BRIK "<<endl;
    mout<<"PCON needs at least two segments"<<endl;
    return false;            
  }

  m_Phi = Phi;
  m_DPhi = DPhi;
  m_NSections = NSections;

  m_Z.resize(NSections);
  m_Rmin.resize(NSections);
  m_Rmax.resize(NSections);

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MDShapePCON::AddSection(unsigned int Section, double Z, double Rmin, double Rmax)
{
  // Add a section 

  if (Rmin < 0) {
    mout<<"   ***  Error  ***  in shape PCON section "<<Section<<endl;
    mout<<"rmin ("<<Rmin<<") needs to be larger or equal 0"<<endl;
    return false;            
  }
  if (Rmax < 0) {
    mout<<"   ***  Error  ***  in shape PCON section "<<Section<<endl;
    mout<<"rmax ("<<Rmax<<") needs to be larger or equal than 0"<<endl;
    return false;            
  }
  if (Rmin >= Rmax) {
    mout<<"   ***  Error  ***  in shape PCON section "<<Section<<endl;
    mout<<"rmax ("<<Rmax<<") needs to be larger than rmin ("<<Rmin<<")"<<endl;
    return false;            
  }

  m_Z[Section] = Z;
  m_Rmin[Section] = Rmin;
  m_Rmax[Section] = Rmax;

  return true;
}
 

////////////////////////////////////////////////////////////////////////////////


bool MDShapePCON::Validate()
{
  delete m_Geo;
  m_Geo = new TGeoPcon(m_Phi, m_DPhi, m_NSections);
  
  for (unsigned int i = 0; i < m_NSections; ++i) {
    dynamic_cast<TGeoPcon*>(m_Geo)->DefineSection(i, m_Z[i], m_Rmin[i], m_Rmax[i]);
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MDShapePCON::Parse(const MTokenizer& Tokenizer, const MDDebugInfo& Info) 
{ 
  // Parse some tokenized text
  
  if (Tokenizer.IsTokenAt(1, "Parameters") == true || Tokenizer.IsTokenAt(1, "Shape") == true) {
    unsigned int Offset = 0;
    if (Tokenizer.IsTokenAt(1, "Shape") == true) Offset = 1;
    
    if (Tokenizer.GetNTokens() < 2+Offset+3 + 3) {
      Info.Error("This PGON must contain at least 6 elements!");
      return false;
    }
      
    unsigned int n = Tokenizer.GetTokenAtAsUnsignedInt(4+Offset);
    if (Tokenizer.GetNTokens() != 2+Offset+3 + 3*n) {
      Info.Error("A PCON must contain 3 + 3*n parameters!");
      return false;
    }
      
    if (Set(Tokenizer.GetTokenAtAsDouble(2+Offset),
            Tokenizer.GetTokenAtAsDouble(3+Offset), 
            Tokenizer.GetTokenAtAsInt(4+Offset)) == false) {
      Info.Error("The basic parameters for the shape PCON are not OK.");
      return false;
    } 

    // Check for ordering:
    bool Increasing = true;
    bool Ordered = false;
    for (unsigned int i = 0; i < n-1; ++i) {
      if (Tokenizer.GetTokenAtAsDouble((2+Offset+3) + 3*(i+1)) > Tokenizer.GetTokenAtAsDouble((2+Offset+3) + 3*i)) {
        if (Ordered == true) {
          if (Increasing == false) {
            Info.Error("z of shape PCON needs to be ordered");
            return false;                          
          }
        } else {
          Ordered = true;
          Increasing = true;
        }
      } else if (Tokenizer.GetTokenAtAsDouble((2+Offset+3) + 3*(i+1)) < Tokenizer.GetTokenAtAsDouble(2+Offset+3 + 3*i)){
        if (Ordered == true) {
          if (Increasing == true) {
            Info.Error("z of shape PCON needs to be ordered");
            return false;                          
          }
        } else {
          Ordered = true;
          Increasing = false;
        }
      }
    }

    unsigned int j = 0;
    for (unsigned int i = 0; i < n; ++i) {
      if (Increasing == false) {
        j = n-i-1;
      } else {
        j = i;
      }

      double z =    Tokenizer.GetTokenAtAsDouble((2+Offset+3) + 0 + 3*j);
      double rmin = Tokenizer.GetTokenAtAsDouble((2+Offset+3) + 1 + 3*j);
      double rmax = Tokenizer.GetTokenAtAsDouble((2+Offset+3) + 2 + 3*j);

      if (AddSection(i, z, rmin, rmax) == false) {
        Info.Error("The segment parameters for the shape PCON are not OK.");
        return false;
      }
    }
  } else {
    Info.Error("Unhandled descriptor in shape PCON!");
    return false;
  }
 
  return true; 
}


////////////////////////////////////////////////////////////////////////////////


MString MDShapePCON::GetGeomega() const
{
  // Return the Geomega representation 

  ostringstream out;

  out<<"PCON "<<m_Phi<<" "<<m_DPhi<<" "<<m_NSections;
  for (unsigned int i = 0; i < m_NSections; ++i) {
    out<<" "<<m_Z[i]<<" "<<m_Rmin[i]<<" "<<m_Rmax[i];
  }

  return out.str().c_str();
}


////////////////////////////////////////////////////////////////////////////////


MVector MDShapePCON::GetSize()
{
  // Return the size of a surrounding box

  double z = 0;
  double max = 0;
  for (unsigned int i = 0; i < m_NSections; ++i) {
    if (m_Rmax[i] > max) max = m_Rmax[i];
    z += m_Z[i];
  }
  return MVector(max, max, z);
}


////////////////////////////////////////////////////////////////////////////////


double MDShapePCON::GetPhi()
{
  return m_Phi;
}


////////////////////////////////////////////////////////////////////////////////


double MDShapePCON::GetDPhi()
{
  return m_DPhi;
}


////////////////////////////////////////////////////////////////////////////////


unsigned int MDShapePCON::GetNSections()
{
  return m_NSections;
}


////////////////////////////////////////////////////////////////////////////////


double MDShapePCON::GetZ(unsigned int Section)
{
  if (Section >= m_NSections) {
    massert(false);
  }

  return m_Z[Section];
}


////////////////////////////////////////////////////////////////////////////////


double MDShapePCON::GetRmin(unsigned int Section)
{
  if (Section >= m_NSections) {
    massert(false);
  }

  return m_Rmin[Section];
}


////////////////////////////////////////////////////////////////////////////////


double MDShapePCON::GetRmax(unsigned int Section)
{
  if (Section >= m_NSections) {
    massert(false);
  }

  return m_Rmax[Section];
}


////////////////////////////////////////////////////////////////////////////////


double MDShapePCON::GetVolume()
{
  // Calculate the volume of this object...

  double Volume = 0;
  
  for (unsigned int i = 1; i < m_NSections; ++i) {
    Volume += 1.0/3.0*c_Pi*(m_Z[i]-m_Z[i-1])*(m_Rmax[i]*m_Rmax[i] + m_Rmax[i]*m_Rmax[i-1] + m_Rmax[i-1]*m_Rmax[i-1]);
    Volume -= 1.0/3.0*c_Pi*(m_Z[i]-m_Z[i-1])*(m_Rmin[i]*m_Rmin[i] + m_Rmin[i]*m_Rmin[i-1] + m_Rmin[i-1]*m_Rmin[i-1]);
  }

  Volume *= m_DPhi/360;
  return Volume;
}


////////////////////////////////////////////////////////////////////////////////


void MDShapePCON::Scale(const double Factor)
{
  // Scale this shape by Factor

  for (unsigned int i = 0; i < m_NSections; ++i) {
    m_Rmin[i] *= Factor;
    m_Rmax[i] *= Factor;
    m_Z[i] *= Factor;  
  }
  
  Validate();
}


////////////////////////////////////////////////////////////////////////////////


MVector MDShapePCON::GetUniquePosition() const
{
  // Return a unique position within this detectors volume

  double R = 0.25*(m_Rmax[1]+m_Rmin[1]+m_Rmax[0]+m_Rmin[0]);
  double Angle = (m_Phi+0.5*m_DPhi)*c_Rad; 

  return MVector(R*cos(Angle), R*sin(Angle), (m_Z[1]+m_Z[0])/2.0);
}


////////////////////////////////////////////////////////////////////////////////


MString MDShapePCON::ToString()
{
  // 

  ostringstream out;

  out<<"      PCON ("<<m_Phi<<","<<m_DPhi<<","<<m_NSections;
  for (unsigned int i = 0; i < m_NSections; ++i) {
    out<<","<<m_Z[i]<<","<<m_Rmin[i]<<","<<m_Rmax[i];
  }
  out<<")"<<endl;

  return out.str().c_str();
}



// MDShapePCON.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
