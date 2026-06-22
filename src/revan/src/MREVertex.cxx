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

MREVertex::MREVertex(MREVertex* vertex):
      MRESE(vertex), // copy base class members
      m_electron_dir(vertex->m_electron_dir),
      m_positron_dir(vertex->m_positron_dir),
      m_gamma_dir(vertex->m_gamma_dir),
      m_electron_energy(vertex->m_electron_energy),
      m_positron_energy(vertex->m_positron_energy),
      m_vertex_type(vertex->m_vertex_type),
      m_EventID(vertex->m_EventID)
      //m_AllRESEs(vertex->m_AllRESEs),
      //m_rese(vertex->m_rese)
{
  m_SubElementType = MRESE::c_Vertex;
  // m_RESEList elements copied during MRESE call; re-linking m_AllRESE
  for (MRESE* r : vertex->m_AllRESEs) {
    for(int i=0; i<m_RESEList->GetNRESEs(); i++) {
      MRESE* c = m_RESEList->GetRESEAt(i);
      if(c->GetID() == r->GetID()) m_AllRESEs.push_back(c);
    }
  }
  // Re-link m_rese
  for(int i=0; i<m_RESEList->GetNRESEs(); i++) {
    MRESE* c = m_RESEList->GetRESEAt(i);
    if (c->GetID() == vertex->m_rese->GetID()) m_rese = c;
  }
  // Re-link m_TrackHits
  for (size_t k=0; k<vertex->m_TrackHits.size(); k++) {
    CombinedHit c;
    c.m_energy = vertex->m_TrackHits[k].m_energy;
    c.m_position = vertex->m_TrackHits[k].m_position;
    for (MRESE* r : vertex->m_TrackHits[k].m_reses) {
      for(int i=0; i<m_RESEList->GetNRESEs(); i++) {
        MRESE* t = m_RESEList->GetRESEAt(i);
        if(t->GetID() == r->GetID())
          c.m_reses.push_back(t);
      }
    }
    m_TrackHits.push_back(c);
  }
}


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