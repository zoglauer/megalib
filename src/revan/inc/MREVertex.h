#ifndef __MREVertex__
#define __MREVertex__

// ROOT libs:
#include <TROOT.h>

// MEGAlib libs:
#include "MVector.h"
#include "MRESE.h"

// Standard libs:
#include <vector>
#include <stdlib.h>
using namespace std;

// Forward declarations:


// Some really small helpers:

// ── CombinedHit ──────────────────────────────────────────────────────────────
// Carries a (possibly merged) hit's position and energy alongside the
// original MRESE* constituents.  Used by CombiningHitsByAngle and stored
// in Vertex::TrackHits so TrackPairs can call AddRESE on real objects.
typedef struct CombinedHit {
    MVector             m_position;   // midpoint if merged, original pos otherwise
    double              m_energy;     // summed energy of all constituents
    std::vector<MRESE*> m_reses;      // original MRESE* objects (1 or more)
} CombinedHit;




////////////////////////////////////////////////////////////////////////////////



class MREVertex: public MRESE
{
 public:
  MREVertex(MRESE* rese, std::vector<MRESE*> allRESEs = {}, MVector* position = nullptr);
  MREVertex(MREVertex* vertex);
  ~MREVertex() {}

  void ComputeGammaDirection();
  //virtual MVector GetPosition() const override { return m_Position; }
  double  GetHitEnergy() const { return m_energy; }
  int     GetID()        const { return m_id; }
  MRESE*  GetRESE()      const { return m_rese; }

  MVector m_electron_dir;
  MVector m_positron_dir;
  MVector m_gamma_dir;
  double  m_electron_energy;
  double  m_positron_energy;
  std::string m_vertex_type;
  int m_EventID;

  // Flat list of constituent MRESE* objects (used directly for 2ht2ht).
  std::vector<MRESE*> m_AllRESEs;

  // Per-track CombinedHit objects (used for 1ht2ht and 1ht1ht).
  // TrackHits[0] = electron side, TrackHits[1] = positron side.
  // Each proxy's .reses field holds the original MRESE* constituents
  // that should be passed to MRETrack::AddRESE.
  std::vector<CombinedHit> m_TrackHits;

 private:
  MRESE* m_rese;
  //MVector m_position;
  double m_energy;
  int m_id;

#ifdef ___CLING___
 public:
  ClassDef(MREVertex, 0) // no description
#endif

};

#endif