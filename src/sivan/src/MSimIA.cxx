/*
 * MSimIA.cxx
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
// MSimIA
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MSimIA.h"

// Standard libs:
#include <sstream>
#include <iomanip>
#include <iostream>
using namespace std;

// MEGAlib libs:
#include "MAssert.h"
#include "MStreams.h"

#ifdef ___CINT___
ClassImp(MSimIA)
#endif


////////////////////////////////////////////////////////////////////////////////


MSimIA::MSimIA() : m_Process("UNKN"), m_DetectorType(0), m_ID(-1), m_OriginID(-1),
                   m_Time(0), m_Position(0, 0, 0), 
                   m_MotherParticleID(-1), m_MotherParticleDirection(0, 0, 0), 
                   m_MotherParticlePolarisation(0, 0, 0),  m_MotherParticleEnergy(0), 
                   m_SecondaryParticleID(0), m_SecondaryParticleDirection(0, 0, 0),
                   m_SecondaryParticlePolarisation(0, 0, 0), m_SecondaryParticleEnergy(0), 
                   m_ParentNucleus(0)
{
  // standard constructor

//   m_DetectorType = 0;
//   m_Process = "UNKN";
//   m_ID = -1;
//   m_OriginID = -1;

//   m_MotherParticleID = -1;
//   m_MotherParticleEnergy = 0;

//   m_SecondaryParticleID = -1;
//   m_SecondaryParticleEnergy = -1;
}


////////////////////////////////////////////////////////////////////////////////


MSimIA::MSimIA(const MSimIA& IA)
{
  // copy constructor

  m_DetectorType = IA.m_DetectorType;
  m_Process = IA.m_Process;
  m_ID = IA.m_ID;
  m_OriginID = IA.m_OriginID;
  m_Position = IA.m_Position;
  m_Time = IA.m_Time;

  m_MotherParticleID = IA.m_MotherParticleID;
  m_MotherParticleDirection = IA.m_MotherParticleDirection;
  m_MotherParticlePolarisation = IA.m_MotherParticlePolarisation;
  m_MotherParticleEnergy = IA.m_MotherParticleEnergy;

  m_SecondaryParticleID = IA.m_SecondaryParticleID;
  m_SecondaryParticleDirection = IA.m_SecondaryParticleDirection;
  m_SecondaryParticlePolarisation = IA.m_SecondaryParticlePolarisation;
  m_SecondaryParticleEnergy = IA.m_SecondaryParticleEnergy;

  m_ParentNucleus = IA.m_ParentNucleus;
}


////////////////////////////////////////////////////////////////////////////////


MSimIA::~MSimIA()
{
  // standard destructor
}


////////////////////////////////////////////////////////////////////////////////


bool MSimIA::AddRawInput(MString LineBuffer, int Version)
{
  // Analyze one line of text input...

  bool Ret = true;
  char Type[10] = "????\0";

  if (Version == 15) {
    if (sscanf(LineBuffer.Data(), "IA %4s %d;%d;%d;%lf;%lf;%lf;%d;%lf;%lf;%lf;%lf",
               Type, 
               &m_ID, 
               &m_OriginID, 
               &m_DetectorType, 
               &m_Position[0], 
               &m_Position[1], 
               &m_Position[2], 
               &m_MotherParticleID, 
               &m_MotherParticleDirection[0], 
               &m_MotherParticleDirection[1], 
               &m_MotherParticleDirection[2], 
               &m_MotherParticleEnergy) != 12) {
      Ret = false;
    } else {
      m_Process = MString(Type);
    }
  } else if (Version == 20 || Version == 21) {
    if (sscanf(LineBuffer.Data(), "IA %4s %d;%d;%d;%lf;%lf;%lf;%d;%lf;%lf;%lf;%lf;%lf;%lf;%lf;%d;%lf;%lf;%lf;%lf;%lf;%lf;%lf",
               Type, 
               &m_ID, 
               &m_OriginID, 
               &m_DetectorType, 
               &m_Position[0], 
               &m_Position[1], 
               &m_Position[2], 
               &m_MotherParticleID, 
               &m_MotherParticleDirection[0], 
               &m_MotherParticleDirection[1], 
               &m_MotherParticleDirection[2], 
               &m_MotherParticlePolarisation[0], 
               &m_MotherParticlePolarisation[1], 
               &m_MotherParticlePolarisation[2], 
               &m_MotherParticleEnergy,
               &m_SecondaryParticleID, 
               &m_SecondaryParticleDirection[0], 
               &m_SecondaryParticleDirection[1], 
               &m_SecondaryParticleDirection[2], 
               &m_SecondaryParticlePolarisation[0], 
               &m_SecondaryParticlePolarisation[1], 
               &m_SecondaryParticlePolarisation[2], 
               &m_SecondaryParticleEnergy) != 23) {
      Ret = false;
    } else {
      m_Process = MString(Type);
    }
  } else if (Version == 22) {
    if (sscanf(LineBuffer.Data(), "IA %4s %d;%d;%d;%lf;%lf;%lf;%d;%lf;%lf;%lf;%lf;%lf;%lf;%lf;%d;%lf;%lf;%lf;%lf;%lf;%lf;%lf;%lf",
               Type, 
               &m_ID, 
               &m_OriginID, 
               &m_DetectorType, 
               &m_Position[0], 
               &m_Position[1], 
               &m_Position[2], 
               &m_MotherParticleID, 
               &m_MotherParticleDirection[0], 
               &m_MotherParticleDirection[1], 
               &m_MotherParticleDirection[2], 
               &m_MotherParticlePolarisation[0], 
               &m_MotherParticlePolarisation[1], 
               &m_MotherParticlePolarisation[2], 
               &m_MotherParticleEnergy,
               &m_SecondaryParticleID, 
               &m_SecondaryParticleDirection[0], 
               &m_SecondaryParticleDirection[1], 
               &m_SecondaryParticleDirection[2], 
               &m_SecondaryParticlePolarisation[0], 
               &m_SecondaryParticlePolarisation[1], 
               &m_SecondaryParticlePolarisation[2], 
               &m_Time,
               &m_SecondaryParticleEnergy) != 24) {
      Ret = false;
    } else {
      m_Process = MString(Type);
    }
  } else if (Version == 23 || Version == 25) {
    if (sscanf(LineBuffer.Data(), "IA %4s %d;%d;%d;%lf;%lf;%lf;%lf;%d;%lf;%lf;%lf;%lf;%lf;%lf;%lf;%d;%lf;%lf;%lf;%lf;%lf;%lf;%lf",
               Type, 
               &m_ID, 
               &m_OriginID, 
               &m_DetectorType, 
               &m_Time,
               &m_Position[0], 
               &m_Position[1], 
               &m_Position[2], 
               &m_MotherParticleID, 
               &m_MotherParticleDirection[0], 
               &m_MotherParticleDirection[1], 
               &m_MotherParticleDirection[2], 
               &m_MotherParticlePolarisation[0], 
               &m_MotherParticlePolarisation[1], 
               &m_MotherParticlePolarisation[2], 
               &m_MotherParticleEnergy,
               &m_SecondaryParticleID, 
               &m_SecondaryParticleDirection[0], 
               &m_SecondaryParticleDirection[1], 
               &m_SecondaryParticleDirection[2], 
               &m_SecondaryParticlePolarisation[0], 
               &m_SecondaryParticlePolarisation[1], 
               &m_SecondaryParticlePolarisation[2], 
               &m_SecondaryParticleEnergy) != 24) {
      Ret = false;
    } else {
      m_Process = MString(Type);
    }
  } else if (Version == 24) {
    if (sscanf(LineBuffer.Data(), "IA %4s %d;%d;%d;%lf;%lf;%lf;%lf;%d;%lf;%lf;%lf;%lf;%d;%lf;%lf;%lf;%lf",
               Type, 
               &m_ID, 
               &m_OriginID, 
               &m_DetectorType, 
               &m_Time,
               &m_Position[0], 
               &m_Position[1], 
               &m_Position[2], 
               &m_MotherParticleID, 
               &m_MotherParticleDirection[0], 
               &m_MotherParticleDirection[1], 
               &m_MotherParticleDirection[2], 
               &m_MotherParticleEnergy,
               &m_SecondaryParticleID, 
               &m_SecondaryParticleDirection[0], 
               &m_SecondaryParticleDirection[1], 
               &m_SecondaryParticleDirection[2], 
               &m_SecondaryParticleEnergy) != 18) {
      Ret = false;
    } else {
      m_Process = MString(Type);
    }
  } else if (Version == 26) {
    if (sscanf(LineBuffer.Data(), "IA %4s %d;%d;%d;%lf;%lf;%lf;%lf;%d;%lf;%lf;%lf;%lf;%d;%lf;%lf;%lf;%lf;%d",
               Type, 
               &m_ID, 
               &m_OriginID, 
               &m_DetectorType, 
               &m_Time,
               &m_Position[0], 
               &m_Position[1], 
               &m_Position[2], 
               &m_MotherParticleID, 
               &m_MotherParticleDirection[0], 
               &m_MotherParticleDirection[1], 
               &m_MotherParticleDirection[2], 
               &m_MotherParticleEnergy,
               &m_SecondaryParticleID, 
               &m_SecondaryParticleDirection[0], 
               &m_SecondaryParticleDirection[1], 
               &m_SecondaryParticleDirection[2], 
               &m_SecondaryParticleEnergy,
               &m_ParentNucleus) != 19) {
      Ret = false;
    } else {
      m_Process = MString(Type);
    }
  } else {
    if (sscanf(LineBuffer.Data(), "IA%d;%4s;%d;%d;%d;%lf;%lf;%lf;%lf;%lf;%lf;%lf",
               &m_DetectorType, 
               Type, 
               &m_SecondaryParticleID, 
               &m_ID, 
               &m_OriginID, 
               &m_Position[0], 
               &m_Position[1], 
               &m_Position[2], 
               &m_SecondaryParticleDirection[0], 
               &m_SecondaryParticleDirection[1], 
               &m_SecondaryParticleDirection[2], 
               &m_SecondaryParticleEnergy) != 12) {
      Ret = false;
    } else {
      if (Version <= 2 && (m_SecondaryParticleID == 2 || m_SecondaryParticleID == 3)) {
        m_SecondaryParticleEnergy -= 511.004;
      }
      m_Process = MString(Type);
    }
  }

  if (Ret == false) {
    mout<<"Unable to scan Version "<<Version<<" IA!"<<endl;
  }

  return Ret;
}


////////////////////////////////////////////////////////////////////////////////


MString MSimIA::ToString() const
{
  ostringstream out;

  out<<"Detector number:         "<<m_DetectorType<<endl;
  out<<"Ineraction type:         "<<m_Process<<endl;
  out<<"Interaction number:      "<<m_ID<<endl;
  out<<"Particle origin:         "<<m_OriginID<<endl;
  out<<"Interaction coordinates: "<<m_Position[0]<<", "<<m_Position[1]<<", "<<m_Position[2]<<endl;
  out<<"Interaction time:        "<<m_Time<<endl;

  out<<"Particle number:         "<<m_SecondaryParticleID<<endl;
  out<<"Particle direction:      "<<m_SecondaryParticleDirection[0]<<", "<<m_SecondaryParticleDirection[1]<<", "<<m_SecondaryParticleDirection[2]<<endl;
  out<<"Particle Energy:         "<<m_SecondaryParticleEnergy<<endl;

  out<<"MotherParticle number:         "<<m_MotherParticleID<<endl;
  out<<"MotherParticle direction:      "<<m_MotherParticleDirection[0]<<", "<<m_MotherParticleDirection[1]<<", "<<m_MotherParticleDirection[2]<<endl;
  out<<"MotherParticle Energy:         "<<m_MotherParticleEnergy<<endl;

  out<<"ParentNucleus:                 "<<m_ParentNucleus<<endl;

  return out;
}


////////////////////////////////////////////////////////////////////////////////


MString MSimIA::ToSimString(const int WhatToStore, const int ScientificPrecision, const int Version) const
{
  // Convert this SimEvent to the original *.sim file format...

  // If there is a significant change, make sure you make new sim version,
  // and update all reading functions

  int WidthPos;
  int PrecisionPos;
  int WidthDir;
  int PrecisionDir;
  int WidthEnergy;
  int PrecisionEnergy;
  int WidthTime;
  int PrecisionTime;

  if (ScientificPrecision > 0) {
    WidthPos        = ScientificPrecision+7;
    PrecisionPos    = ScientificPrecision;
    WidthDir        = ScientificPrecision+7;
    PrecisionDir    = ScientificPrecision;
    WidthEnergy     = ScientificPrecision+7;
    PrecisionEnergy = ScientificPrecision;
    WidthTime       = ScientificPrecision+11;
    PrecisionTime   = ScientificPrecision+7;
  } else {
    WidthPos        = 10;
    PrecisionPos    = 5;
    WidthDir        = 8;
    PrecisionDir    = 5;
    WidthEnergy     = 10;
    PrecisionEnergy = 3;
    WidthTime       = 16;
    PrecisionTime   = 12;
  }

  ostringstream Text;
  Text.setf(ios_base::fixed, ios_base::floatfield);
  Text.precision(4);

  // The compact version
  if (Version == 15) {
    Text<<"IA "<<m_Process<<" "
        <<setw(2)<<m_ID<<";"
        <<setw(2)<<m_OriginID<<";"
        <<setw(1)<<m_DetectorType<<";"
        <<((ScientificPrecision == 0) ? fixed : scientific)
        <<setprecision(PrecisionPos)
        <<setw(WidthPos)<<m_Position[0]<<";"
        <<setw(WidthPos)<<m_Position[1]<<";"
        <<setw(WidthPos)<<m_Position[2]<<";"
        <<setw(1)<<m_MotherParticleID<<";"
        <<setprecision(PrecisionDir)
        <<setw(WidthDir)<<m_MotherParticleDirection[0]<<";"
        <<setw(WidthDir)<<m_MotherParticleDirection[1]<<";"
        <<setw(WidthDir)<<m_MotherParticleDirection[2]<<";"
        <<setprecision(PrecisionEnergy)
        <<setw(WidthEnergy)<<m_MotherParticleEnergy;  
   } 
  // The standard version given in MSimEvent::g_OutputVersion
  else {
    Text<<"IA "<<m_Process<<" "
        <<setw(2)<<m_ID<<";"
        <<setw(2)<<m_OriginID<<";"
        <<setw(1)<<m_DetectorType<<";"
        <<scientific<<setprecision(PrecisionTime)<<setw(WidthTime)<<m_Time<<";"
        <<((ScientificPrecision == 0) ? fixed : scientific)
        <<setprecision(PrecisionPos)
        <<setw(WidthPos)<<m_Position[0]<<";"
        <<setw(WidthPos)<<m_Position[1]<<";"
        <<setw(WidthPos)<<m_Position[2]<<";"
        <<setw(1)<<m_MotherParticleID<<";"
        <<setprecision(PrecisionDir)
        <<setw(WidthDir)<<m_MotherParticleDirection[0]<<";"
        <<setw(WidthDir)<<m_MotherParticleDirection[1]<<";"
        <<setw(WidthDir)<<m_MotherParticleDirection[2]<<";"
        <<setw(WidthDir)<<m_MotherParticlePolarisation[0]<<";"
        <<setw(WidthDir)<<m_MotherParticlePolarisation[1]<<";"
        <<setw(WidthDir)<<m_MotherParticlePolarisation[2]<<";"
        <<setprecision(PrecisionEnergy)
        <<setw(WidthEnergy)<<m_MotherParticleEnergy<<";"
        <<setw(1)<<m_SecondaryParticleID<<";"
        <<setprecision(PrecisionDir)
        <<setw(WidthDir)<<m_SecondaryParticleDirection[0]<<";"
        <<setw(WidthDir)<<m_SecondaryParticleDirection[1]<<";"
        <<setw(WidthDir)<<m_SecondaryParticleDirection[2]<<";"
        <<setw(WidthDir)<<m_SecondaryParticlePolarisation[0]<<";"
        <<setw(WidthDir)<<m_SecondaryParticlePolarisation[1]<<";"
        <<setw(WidthDir)<<m_SecondaryParticlePolarisation[2]<<";"
        <<setprecision(PrecisionEnergy)
        <<setw(WidthEnergy)<<m_SecondaryParticleEnergy;
  }

  return Text;
}


////////////////////////////////////////////////////////////////////////////////


void MSimIA::OffsetOrigins(int Offset)
{
  m_ID += Offset;
  m_OriginID += Offset;
}

// MSimIA: the end...
////////////////////////////////////////////////////////////////////////////////
