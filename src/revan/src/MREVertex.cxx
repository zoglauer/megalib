/*
 * MREVertex.cxx
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
// MREVertex
//
// Base class for event reconstruction tasks, e.g. find clusters, tracks,
// Compton sequence
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MREVertex.h"

// Standard libs:
#include <list>
#include <algorithm>
#include <functional>
#include <limits>
#include <cfloat>
using namespace std;

// ROOT libs:

// MEGAlib libs:

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MREVertex)
#endif



//////////////////////////////////////////
// VERTEX CLASS
//////////////////////////////////////////
MREVertex::MREVertex(MRESE* rese_in, std::vector<MRESE*> allRESEs, MVector* position):
      m_electron_dir({0,0,0}),
      m_positron_dir({0,0,0}),
      m_gamma_dir({0,0,0}),
      m_electron_energy(0.0),
      m_positron_energy(0.0),
      m_vertex_type("unknown"),
      m_EventID(-1),
      m_AllRESEs(allRESEs),
      m_rese(rese_in)
{
  m_SubElementType = MRESE::c_Vertex;
  if (position != nullptr) {
    m_Position = *position;
  } else {
    MVector pos = m_rese->GetPosition();
    m_Position = pos;
  }
  m_energy = (m_rese != nullptr) ? m_rese->GetEnergy() : 0.0;
}

//////////////////////////////////////////

void MREVertex::ComputeGammaDirection()
{
  if (m_electron_energy == 0.0 || m_positron_energy == 0.0) throw std::runtime_error("Track energies are zero.");

  MVector d1 = m_electron_dir.Unit();
  MVector d2 = m_positron_dir.Unit();

  // Using an energy-weighted reconstruction
  MVector weighted_sum = d1 * m_electron_energy + d2 * m_positron_energy;
  m_gamma_dir = (weighted_sum / (m_electron_energy + m_positron_energy)) * (-1.0);
  m_gamma_dir = m_gamma_dir.Unit();

  // return m_gamma_dir;
}