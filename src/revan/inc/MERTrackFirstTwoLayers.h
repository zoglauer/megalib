#ifndef __MERTrackFirstTwoLayers__
#define __MERTrackFirstTwoLayers__

#include "MGlobal.h"
#include "MGeometryRevan.h"
#include "MERTrack.h"
#include "MDVolumeSequence.h"
#include "MDDetector.h"
#include "MDStrip2D.h"
#include "MRESE.h"
#include "MREVertex.h"
#include "MRERawEvent.h"
#include "MVector.h"
#include <vector>
#include <string>
#include <stdexcept>
#include <cmath>
#include <utility>
#include <algorithm>

//////////////////////////////////////////
// Instantiating the MERTrack class

class MERTrackFirstTwoLayers : public MERTrack
{
public:
  MERTrackFirstTwoLayers(MGeometryRevan* geom);
  virtual ~MERTrackFirstTwoLayers();
  //virtual bool Analyze(MRawEventIncarnations* REList) override;

protected:

  //virtual MRawEventIncarnations* CheckForPair(MRERawEvent* RE) override;
  virtual void TrackPairs(MRERawEvent* RE) override;
  /*static MVector ProjectToLayer(const MVector& pos, const MVector& dir, double z_target);
  static double  DistanceToVirtual(const MVector& pos, const MVector& virtual_pt);
  static bool    SelectTwoClosestHits(const std::vector<MRESE*>& hits,
                                        const MVector& projected_pt,
                                        MRESE*& hit1_out, MRESE*& hit2_out,
                                        double cutoff_cm = 5.0);*/

  // Combine hits in a layer by smallest opening angle from the vertex.
  // Merges the closest-angle pair iteratively until exactly 2 remain.
  // Returns an empty vector if the minimum merge angle exceeds max_angle_deg.
  static std::vector<CombinedHit> CombiningHitsByAngle(
      const std::vector<MRESE*>& hits,
      const MVector& vertex_pos,
      double max_angle_deg = 11.0);
  
  // ── Vertex class ──────────────────────────────────────────────────────────
  /*class Vertex {
  public:
      Vertex(MRESE* rese, std::vector<MRESE*> allRESEs, MVector* position = nullptr);

      MVector ComputeGammaDirection();
      MVector GetPosition()  const { return m_position; }
      int     GetID()        const { return m_id; }
      double  GetHitEnergy() const { return m_energy; }
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
      MVector m_position;
      double m_energy;
      int m_id;
  };*/
  std::pair<std::pair<MVector, MVector>, std::pair<MVector, MVector>>
  BestHitPairing(const MVector& A1, const MVector& A2,
              const MVector& B1, const MVector& B2);
  MVector CalculatingVertexPosition(const MVector& p1, const MVector& v1,
                                      const MVector& p2, const MVector& v2);

  //std::vector<MREVertex*> FindVertices(MRERawEvent* RE);
  virtual MRawEventIncarnations* CheckForPair(MRERawEvent* RE) override;

  MREVertex* TopVertex(const std::vector<MREVertex*>& vertex_list);

  // ── VertexFinder class ────────────────────────────────────────────────────
/*    class VertexFinder {
  public:
      VertexFinder(MGeometryRevan* geom, const std::vector<MDDetector*>& detList)
      : Geometry(geom), m_DetectorList(detList) {}

      //bool IsInTracker(MRESE* rese);




      VertexFinder(MGeometryRevan* geom);
  private:
      MGeometryRevan* Geometry;
      std::vector<MDDetector*> m_DetectorList;
      double m_InterlayerDistance;

  };*/

private:
  MGeometryRevan* m_Geometry;
  //double m_InterlayerDistance;


#ifdef ___CINT___
 public:
  ClassDef(MERTrackFirstTwoLayers, 0)
#endif
};

#endif 

// __MERTrackFirstTwoLayers__