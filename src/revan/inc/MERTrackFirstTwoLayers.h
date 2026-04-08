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
    virtual void TrackPairs(MRERawEvent* RE) override;

    void SetIncidentAngles(double theta_deg, double phi_deg) {
        m_Theta = theta_deg;
        m_Phi   = phi_deg;
    }

private:
    MGeometryRevan* m_Geometry;
    double m_Theta;
    double m_Phi;

    // using MVector instead of defining a struct for vectors
    static MVector InitialVector(double theta_deg, double phi_deg);
    static MVector ProjectToLayer(const MVector& pos, const MVector& dir, double z_target);
    static double  DistanceToVirtual(const MVector& pos, const MVector& virtual_pt);
    static bool    SelectTwoClosestHits(const std::vector<MRESE*>& hits,
                                         const MVector& projected_pt,
                                         MRESE*& hit1_out, MRESE*& hit2_out,
                                         double cutoff_cm = 5.0);
    static std::vector<MRESE*> GetHitsInLayerBelow(const std::vector<MRESE*>& allHits,
                                                     double vertex_z,
                                                     double layer_thickness = 1.5);

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
        std::vector<MRESE*> AllRESEs;

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
        VertexFinder(MGeometryRevan* geom) : Geometry(geom) {}

        bool IsInTracker(MRESE* rese);

        std::pair<std::pair<MVector, MVector>, std::pair<MVector, MVector>>
        BestHitPairing(const MVector& A1, const MVector& A2,
                       const MVector& B1, const MVector& B2);

        MVector CalculatingVertexPosition(const MVector& p1, const MVector& v1,
                                           const MVector& p2, const MVector& v2);

        std::vector<Vertex> FindVertices(MRERawEvent* RE, double theta, double phi);

        Vertex TopVertex(const std::vector<Vertex>& vertex_list);

    private:
        MGeometryRevan* Geometry;
    };

#ifdef ___CINT___
public:
    ClassDef(MERTrackFirstTwoLayers, 0)
#endif
};

#endif // __MERTrackFirstTwoLayers__