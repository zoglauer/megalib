#ifndef __MERTrackFirstTwoLayers__
#define __MERTrackFirstTwoLayers__

#include "MGlobal.h"
#include "MGeometryRevan.h"
#include "MERTrack.h"
#include "MDVolumeSequence.h"
#include "MDDetector.h"
#include "MDStrip2D.h"
#include "MRESE.h"
#include "MRERawEvent.h"
#include "MVector.h"
#include <vector>
#include <string>
#include <stdexcept>
#include <cmath>
#include <utility>
#include <algorithm>

//////////////////////////////////////////

class MERTrackFirstTwoLayers : public MERTrack
{
public:
    MERTrackFirstTwoLayers(MGeometryRevan* geom);
    virtual ~MERTrackFirstTwoLayers();
    virtual bool Analyze(MRawEventIncarnations* REList) override;
    virtual void TrackPairs(MRERawEvent* RE) override;

    // ── ClusteredHit ──────────────────────────────────────────────────────────────
    // Carries a (possibly merged) hit's position and energy alongside the
    // original MRESE* constituents.  Used by ClusteringHitsByAngle and stored
    // in Vertex::TrackHits so TrackPairs can call AddRESE on real objects.
    struct ClusteredHit {
        MVector             position;   // midpoint if merged, original pos otherwise
        double              energy;     // summed energy of all constituents
        std::vector<MRESE*> reses;      // original MRESE* objects (1 or more)
    };

private:
    MGeometryRevan* m_Geometry;

    static MVector ProjectToLayer(const MVector& pos, const MVector& dir, double z_target);
    static double  DistanceToVirtual(const MVector& pos, const MVector& virtual_pt);
    static bool    SelectTwoClosestHits(const std::vector<MRESE*>& hits,
                                         const MVector& projected_pt,
                                         MRESE*& hit1_out, MRESE*& hit2_out,
                                         double cutoff_cm = 5.0);
    static std::vector<MRESE*> GetHitsInLayerBelow(const std::vector<MRESE*>& allHits,
                                                     double vertex_z,
                                                     double layer_thickness = 1.5);

    // Cluster hits in a layer by smallest opening angle from the vertex.
    // Merges the closest-angle pair iteratively until exactly 2 remain.
    // Returns an empty vector if the minimum merge angle exceeds max_angle_deg.
    static std::vector<ClusteredHit> ClusteringHitsByAngle(
        const std::vector<MRESE*>& hits,
        const MVector& vertex_pos,
        double max_angle_deg = 11.0);
    
    virtual MRawEventIncarnations* CheckForPair(MRERawEvent* RE) override;

    // ── Vertex class ──────────────────────────────────────────────────────────
    class Vertex {
    public:
        Vertex(MRESE* rese, MGeometryRevan* geom,
               std::vector<MRESE*> allRESEs, MVector* position = nullptr);

        MVector ComputeGammaDirection();
        MVector GetPosition()  const;
        double  GetXPosition() const;
        double  GetYPosition() const;
        double  GetZPosition() const;
        int     GetID()        const;
        double  GetHitEnergy() const;
        MRESE*  GetRESE()      const { return rese; }

        MVector electron_dir;
        MVector positron_dir;
        MVector gamma_dir;
        double  electron_energy;
        double  positron_energy;
        std::string vertex_type;
        int EventID;

        // Flat list of constituent MRESE* objects (used directly for 2ht2ht).
        std::vector<MRESE*> AllRESEs;

        // Per-track ClusteredHit objects (used for 1ht2ht and 1ht1ht).
        // TrackHits[0] = electron side, TrackHits[1] = positron side.
        // Each proxy's .reses field holds the original MRESE* constituents
        // that should be passed to MRETrack::AddRESE.
        std::vector<ClusteredHit> TrackHits;

    private:
        MRESE* rese;
        MGeometryRevan* Geometry;
        double x, y, z;
        double energy;
        int id;
    };

    // ── VertexFinder class ────────────────────────────────────────────────────
    class VertexFinder {
    public:
        VertexFinder(MGeometryRevan* geom, const std::vector<MDDetector*>& detList)
        : Geometry(geom), m_DetectorList(detList) {}

        bool IsInTracker(MRESE* rese);

        std::pair<std::pair<MVector, MVector>, std::pair<MVector, MVector>>
        BestHitPairing(const MVector& A1, const MVector& A2,
                       const MVector& B1, const MVector& B2);

        MVector CalculatingVertexPosition(const MVector& p1, const MVector& v1,
                                           const MVector& p2, const MVector& v2);

        std::vector<Vertex> FindVertices(MRERawEvent* RE);

        Vertex TopVertex(const std::vector<Vertex>& vertex_list);
        VertexFinder(MGeometryRevan* geom);
    private:
        MGeometryRevan* Geometry;
        std::vector<MDDetector*> m_DetectorList;
        double m_InterlayerDistance;

    };

#ifdef ___CINT___
public:
    ClassDef(MERTrackFirstTwoLayers, 0)
#endif
};

#endif 

// __MERTrackFirstTwoLayers__