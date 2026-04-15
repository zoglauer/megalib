// MERTrackFirstTwoLayers.cxx
#include "MERTrackFirstTwoLayers.h"
#include "MRETrack.h" // included for MERTrack instantiation

#include <iostream>
#include <vector>
#include <cmath>
#include <utility>
#include <algorithm>
#include <stdexcept>
#include <string>

using namespace std;

#ifdef ___CLING___
ClassImp(MERTrackFirstTwoLayers)
#endif

// normalization helper functions -- don't see this anywhere in MVector??
static MVector Normalize(const MVector& v)
{
    double mag = v.Mag();
    if (mag == 0.0) return {0,0,0};
    return v / mag;
}

//////////////////////////////////////////
// OUTER CLASS - MERTrackFirstTwoLayers 
//////////////////////////////////////////

MERTrackFirstTwoLayers::MERTrackFirstTwoLayers(MGeometryRevan* geom)
    : MERTrack(), m_Geometry(geom), m_Theta(0.0), m_Phi(0.0)
{
    mout << "Running new geometric reconstruction algorithm!" << endl;
}

//////////////////////////////////////////

MERTrackFirstTwoLayers::~MERTrackFirstTwoLayers() {}

//////////////////////////////////////////
// VERTEX CLASS
//////////////////////////////////////////

MERTrackFirstTwoLayers::Vertex::Vertex(MRESE* rese_in, MGeometryRevan* geom, std::vector<MRESE*> allRESEs, MVector* position): 
      rese(rese_in), Geometry(geom), AllRESEs(allRESEs),
      electron_dir({0,0,0}), positron_dir({0,0,0}), gamma_dir({0,0,0}),
      electron_energy(0.0), positron_energy(0.0),
      vertex_type("unknown"), EventID(-1)
{
    if (position != nullptr) {
        x = position->X();
        y = position->Y();
        z = position->Z();
        id = (rese != nullptr) ? rese->GetID() : -1;
    } else {
        MVector pos = rese->GetPosition();
        x = pos.X();
        y = pos.Y();
        z = pos.Z();
        id = rese->GetID();
    }
    energy = (rese != nullptr) ? rese->GetEnergy() : 0.0;
}

//////////////////////////////////////////

MVector   MERTrackFirstTwoLayers::Vertex::GetPosition()    const { return {x, y, z}; }
double MERTrackFirstTwoLayers::Vertex::GetXPosition()   const { return x; }
double MERTrackFirstTwoLayers::Vertex::GetYPosition()   const { return y; }
double MERTrackFirstTwoLayers::Vertex::GetZPosition()   const { return z; }
int    MERTrackFirstTwoLayers::Vertex::GetID()          const { return id; }
double MERTrackFirstTwoLayers::Vertex::GetHitEnergy()   const { return energy; }

//////////////////////////////////////////

MVector MERTrackFirstTwoLayers::Vertex::ComputeGammaDirection()
{
    if (electron_energy == 0.0 && positron_energy == 0.0)
        throw std::runtime_error("Track directions/energies are zero.");

    MVector d1 = Normalize(electron_dir);
    MVector d2 = Normalize(positron_dir);

    MVector weighted_sum = d1 * electron_energy + d2 * positron_energy;
    gamma_dir = (weighted_sum / (electron_energy + positron_energy)) * (-1.0);
    gamma_dir = Normalize(gamma_dir);

    return gamma_dir;
}

//////////////////////////////////////////
// STANDALONE FUNCTIONS
//////////////////////////////////////////

MVector MERTrackFirstTwoLayers::InitialVector(double theta_deg, double phi_deg)
{
    double theta = theta_deg * M_PI / 180.0;
    double phi   = phi_deg   * M_PI / 180.0;
    MVector v = { std::sin(theta) * std::cos(phi),
               std::sin(theta) * std::sin(phi),
               std::cos(theta) };
    return Normalize(v);
}

//////////////////////////////////////////

MVector MERTrackFirstTwoLayers::ProjectToLayer(const MVector& pos, const MVector& dir, double z_target)
{
    double t = (z_target - pos.Z()) / dir.Z();
    return { pos.X() + t * dir.X(),
             pos.Y() + t * dir.Y(),
             z_target };
}

//////////////////////////////////////////

double MERTrackFirstTwoLayers::DistanceToVirtual(const MVector& pos, const MVector& virtual_pt)
{
    return (pos - virtual_pt).Mag();
}

//////////////////////////////////////////

bool MERTrackFirstTwoLayers::SelectTwoClosestHits(const std::vector<MRESE*>& hits,
                                                   const MVector& projected_pt,
                                                   MRESE*& hit1_out, MRESE*& hit2_out,
                                                   double cutoff_cm)
{
    std::vector<std::pair<MRESE*, double>> candidates;
    for (MRESE* h : hits) {
        MVector p = h->GetPosition();
        MVector pos = { p.X(), p.Y(), p.Z() };
        double d = DistanceToVirtual(pos, projected_pt);
        if (d <= cutoff_cm) candidates.push_back({h, d});
    }

    if (candidates.size() < 2) {
        hit1_out = nullptr;
        hit2_out = nullptr;
        return false;
    }

    std::sort(candidates.begin(), candidates.end(),
              [](const auto& a, const auto& b){ return a.second < b.second; });

    hit1_out = candidates[0].first;
    hit2_out = candidates[1].first;
    return true;
}

//////////////////////////////////////////

std::vector<MRESE*> MERTrackFirstTwoLayers::GetHitsInLayerBelow(
    const std::vector<MRESE*>& allHits,
    double vertex_z,
    double layer_thickness)
{
    double target_z  = vertex_z - layer_thickness;
    double tolerance = 1e-4;

    std::vector<MRESE*> layer_hits;
    for (MRESE* h : allHits) {
        if (std::fabs(h->GetPosition().Z() - target_z) < tolerance)
            layer_hits.push_back(h);
    }
    return layer_hits;
}

//////////////////////////////////////////
// VERTEXFINDER CLASS
//////////////////////////////////////////

bool MERTrackFirstTwoLayers::VertexFinder::IsInTracker(MRESE* rese)
{
    if (rese->GetType() == MRESE::c_Track) return true;

    MDVolumeSequence* VS = rese->GetVolumeSequence();
    if (VS == nullptr) return false;

    MDDetector* Detector = VS->GetDetector();
    if (Detector != nullptr && dynamic_cast<MDStrip2D*>(Detector) != nullptr) return true;

    return false;
}

//////////////////////////////////////////

std::pair<std::pair<MVector, MVector>, std::pair<MVector, MVector>>
MERTrackFirstTwoLayers::VertexFinder::BestHitPairing(
    const MVector& A1, const MVector& A2,
    const MVector& B1, const MVector& B2)
{
    auto dist = [](const MVector& a, const MVector& b) {
        return std::sqrt(std::pow(a.X()-b.X(),2) + std::pow(a.Y()-b.Y(),2) + std::pow(a.Z()-b.Z(),2));
    };

    double d11 = dist(A1, B1), d12 = dist(A1, B2);
    double d21 = dist(A2, B1), d22 = dist(A2, B2);

    if ((d11 + d22) <= (d12 + d21)) return {{A1, B1}, {A2, B2}};
    else                             return {{A1, B2}, {A2, B1}};
}

//////////////////////////////////////////

MVector MERTrackFirstTwoLayers::VertexFinder::CalculatingVertexPosition(
    const MVector& p1, const MVector& v1_in,
    const MVector& p2, const MVector& v2_in)
{
    MVector v1 = Normalize(v1_in);
    MVector v2 = Normalize(v2_in);

    MVector w0 = { p1.X() - p2.X(), p1.Y() - p2.Y(), p1.Z() - p2.Z() };

    MVector obj;
    double a = v1*v1;
    double b = v1*v2;
    double c = v2*v2;
    double d = v1*w0;
    double e = v2*w0;

    double denom = a*c - b*b;

    if (std::fabs(denom) < 1e-6) return {0, 0, 0}; // parallel tracks 

    double t = (b*e - c*d) / denom;
    double s = (a*e - b*d) / denom;

    MVector pca1 = p1 + v1 * t;
    MVector pca2 = p2 + v2 * s;

    // Midpoint of closest approach
    return (pca1 + pca2) * 0.5;
}

//////////////////////////////////////////

MERTrackFirstTwoLayers::Vertex
MERTrackFirstTwoLayers::VertexFinder::TopVertex(const std::vector<Vertex>& vertex_list)
{
    // Returns the vertex with the "smallest" (shallowest) z position
    return *std::min_element(vertex_list.begin(), vertex_list.end(),
        [](const Vertex& a, const Vertex& b){ return a.GetZPosition() < b.GetZPosition(); });
}

//////////////////////////////////////////

std::vector<MERTrackFirstTwoLayers::Vertex>
MERTrackFirstTwoLayers::VertexFinder::FindVertices(MRERawEvent* RE, double theta, double phi)
{
    std::vector<Vertex> Vertices;

    const int LayerRequirement     = 5;
    const int SearchRange          = 30;
    const int SearchLayersFor2Hit  = 5;
    const double interlayerdistance = 1.5; // cm -- this is specific to AMEGO-X!!!

    // filtering RESEs
    std::vector<MRESE*> RESEs;
    for (int i = 0; i < RE->GetNRESEs(); ++i) {
        MRESE* r = RE->GetRESEAt(i);
        if (IsInTracker(r) && r->GetEnergy() > 0)
            RESEs.push_back(r);
    }

    // sort from shallowest to deepest
    std::sort(RESEs.begin(), RESEs.end(), [](MRESE* a, MRESE* b){
        return a->GetPosition().Z() > b->GetPosition().Z();
    });

    bool vertex_created_for_event = false;

    // 1ht 2ht and 1ht 1ht event processing
    for (MRESE* candidate : RESEs) {

        // Require candidate to be the only hit in its layer
        bool OnlyHitInLayer = true;
        for (MRESE* rese : RESEs) {
            if (rese == candidate) continue;
            if (Geometry->AreInSameLayer(candidate, rese)) { OnlyHitInLayer = false; break; }
        }
        if (!OnlyHitInLayer) continue;

        // Count hits above and below by layer distance
        std::vector<int> NBelow(SearchRange, 0);
        std::vector<int> NAbove(SearchRange, 0);

        for (MRESE* rese : RESEs) {
            if (rese == candidate) continue;
            int Distance = Geometry->GetLayerDistance(candidate, rese);
            if (Distance > 0 && Distance < SearchRange)       NAbove[Distance]++;
            else if (Distance < 0 && -Distance < SearchRange) NBelow[-Distance]++;
        }

        // Reject if anything is directly above the candidate
        if (NAbove[1] != 0) continue;

        // Count layers with 2+ hits below
        int StartIndex = 0, StopIndex = 0;
        int LayersWithAtLeastTwoHits = 0;

        for (int dist = 1; dist < SearchRange - 1; ++dist) {
            if (NBelow[dist] == 0 && NBelow[dist+1] == 0) break;
            StopIndex = dist;
            if (StartIndex == 0 && NBelow[dist] > 1)  StartIndex = dist;
            if (StartIndex != 0 && NBelow[dist] >= 2) LayersWithAtLeastTwoHits++;
        }

        // Trim StopIndex back to last solid 2-hit layer
        for (int dist = StopIndex; dist > 2; --dist) {
            if (NBelow[dist-1] >= 2 && NBelow[dist-2] >= 2) break;
            StopIndex = dist;
        }

        if (LayersWithAtLeastTwoHits < LayerRequirement) continue;

        // Find the first layer below the candidate with 2+ hits
        std::vector<MRESE*> selected_layer_hits;
        int selected_distance = -1;

        for (int dist = 1; dist <= SearchLayersFor2Hit; ++dist) {
            std::vector<MRESE*> layer_hits;
            for (MRESE* rese : RESEs) {
                if (Geometry->GetLayerDistance(candidate, rese) == -dist)
                    layer_hits.push_back(rese);
            }
            if ((int)layer_hits.size() >= 2) {
                selected_layer_hits = layer_hits;
                selected_distance = dist;
                break;
            }
        }

        if (selected_distance == -1) continue;

        // ── Project virtual point to the selected layer ───────────────────────
        MVector cpos = candidate->GetPosition();
        MVector init_pos = { cpos.X(), cpos.Y(), cpos.Z() };
        MVector init_dir = MERTrackFirstTwoLayers::InitialVector(theta, phi);
        double z_target = cpos.Z() - selected_distance * interlayerdistance;
        MVector projected_point = MERTrackFirstTwoLayers::ProjectToLayer(init_pos, init_dir, z_target);

        // ── Select hit1 and hit2 ──────────────────────────────────────────────
        MRESE* hit1 = nullptr;
        MRESE* hit2 = nullptr;

        if (selected_distance == 1) {
            // 1ht-2ht: pick the two closest hits to the projected point
            MERTrackFirstTwoLayers::SelectTwoClosestHits(selected_layer_hits, projected_point, hit1, hit2, 5.0);
        } else {
            // 1ht-1ht: multiple single hits before two-hit layer; just take first two
            std::vector<MRESE*> layer_hits_below;
            for (MRESE* rese : RESEs) {
                if (Geometry->GetLayerDistance(candidate, rese) == -selected_distance)
                    layer_hits_below.push_back(rese);
            }
            if ((int)layer_hits_below.size() >= 2) {
                hit1 = layer_hits_below[0];
                hit2 = layer_hits_below[1];
            }
        }

        if (hit1 == nullptr || hit2 == nullptr) continue;

        // ── Build vertex ──────────────────────────────────────────────────────
        Vertex vtx(candidate, Geometry, {hit1, hit2});
        vtx.EventID = RE->GetEventID();

        // ── Assign track directions and energies ──────────────────────────────
        MVector vtx_pos = vtx.GetPosition();

        if (selected_distance == 1) {
            // Typical 1ht-2ht:
            MVector p1 = hit1->GetPosition();
            MVector p2 = hit2->GetPosition();
            vtx.electron_dir = Normalize(MVector{p1.X(), p1.Y(), p1.Z()} - vtx_pos);
            vtx.positron_dir = Normalize(MVector{p2.X(), p2.Y(), p2.Z()} - vtx_pos);
            vtx.electron_energy = hit1->GetEnergy();
            vtx.positron_energy = hit2->GetEnergy();
            vtx.vertex_type = "type_1ht2ht";

        } else {
            // 1ht-1ht:
            if ((int)RESEs.size() >= 2) {
                MRESE* first_hit  = RESEs[0];
                MRESE* second_hit = RESEs[1];

                MVector fp = first_hit->GetPosition();
                MVector sp = second_hit->GetPosition();

                MVector first_pos  = { fp.X(), fp.Y(), fp.Z() };
                MVector second_pos = { sp.X(), sp.Y(), sp.Z() };

                MVector best_track      = Normalize(vtx_pos - first_pos);
                MVector alternate_track = Normalize(second_pos - vtx_pos);

                // Handle degenerate cases where a hit coincides with the vertex
                auto allclose = [](const MVector& a, const MVector& b) {
                    return std::fabs(a.X()-b.X()) < 1e-6 &&
                           std::fabs(a.Y()-b.Y()) < 1e-6 &&
                           std::fabs(a.Z()-b.Z()) < 1e-6;
                };

                MVector track;
                if (allclose(first_pos, vtx_pos))        track = alternate_track;
                else if (allclose(second_pos, vtx_pos))  track = best_track;
                else                                     track = best_track;

                vtx.electron_dir = track;
                vtx.positron_dir = track;
                vtx.electron_energy = first_hit->GetEnergy();
                vtx.positron_energy = first_hit->GetEnergy();
            }
            vtx.vertex_type = "type_1ht1ht";
        }

        vtx.gamma_dir = vtx.ComputeGammaDirection();
        Vertices.push_back(vtx);
        vertex_created_for_event = true;
    }

    // 2 ht 2ht
        if (RESEs.empty()) return Vertices;

        MRESE* top_hit = RESEs[0];

        // Hits in the topmost layer
        std::vector<MRESE*> hits_in_first_layer;
        for (MRESE* rese : RESEs) {
            if (Geometry->GetLayerDistance(rese, top_hit) == 0)
                hits_in_first_layer.push_back(rese);
        }

        if ((int)hits_in_first_layer.size() != 2) return Vertices;

        // Hits in the layer immediately below
        std::vector<MRESE*> hits_in_layer_below;
        for (MRESE* rese : RESEs) {
            if (Geometry->GetLayerDistance(hits_in_first_layer[0], rese) == -1)
                hits_in_layer_below.push_back(rese);
        }

        if ((int)hits_in_layer_below.size() != 2) return Vertices;

        // Apply LayerRequirement check on the 2ht topology
        std::vector<int> NBelow(SearchRange, 0);
        for (MRESE* rese : RESEs) {
            int dist = Geometry->GetLayerDistance(top_hit, rese);
            if (dist < 0 && -dist < SearchRange) NBelow[-dist]++;
        }

        int StartIndex = 0, LayersWithAtLeastTwoHits = 0;
        for (int dist = 1; dist < SearchRange - 1; ++dist) {
            if (NBelow[dist] == 0 && NBelow[dist+1] == 0) break;
            if (StartIndex == 0 && NBelow[dist] > 1)  StartIndex = dist;
            if (StartIndex != 0 && NBelow[dist] >= 2) LayersWithAtLeastTwoHits++;
        }

        if (LayersWithAtLeastTwoHits < LayerRequirement) return Vertices;

        // Extract the four hit positions
        MRESE* hit1lay1 = hits_in_first_layer[0];
        MRESE* hit2lay1 = hits_in_first_layer[1];
        MRESE* hit1lay2 = hits_in_layer_below[0];
        MRESE* hit2lay2 = hits_in_layer_below[1];

        auto toMVector = [](MRESE* r) -> MVector {
            MVector p = r->GetPosition();
            return { p.X(), p.Y(), p.Z() };
        };

        MVector A1 = toMVector(hit1lay1), A2 = toMVector(hit2lay1);
        MVector B1 = toMVector(hit1lay2), B2 = toMVector(hit2lay2);

        // Best pairing of hits across the two layers
        auto [track1, track2] = BestHitPairing(A1, A2, B1, B2);

        MVector p1 = track1.first,  q1 = track1.second; // layer1 hit, layer2 hit for track 1
        MVector p2 = track2.first,  q2 = track2.second; // layer1 hit, layer2 hit for track 2

        MVector v1 = q1 - p1;
        MVector v2 = q2 - p2;

        // Point of closest approach is declared to be the vertex position
        MVector vertex_point = CalculatingVertexPosition(p1, v1, p2, v2);

        // Check for parallel tracks (CalculatingVertexPosition returns {0,0,0} sentinel)
        if (vertex_point.X() == 0.0 && vertex_point.Y() == 0.0 && vertex_point.Z() == 0.0) return Vertices;

        Vertex vtx(nullptr, Geometry, {hit1lay1, hit2lay1, hit1lay2, hit2lay2}, &vertex_point);
        vtx.EventID = RE->GetEventID();

        vtx.electron_dir = Normalize(v1);
        vtx.positron_dir = Normalize(v2);
        vtx.electron_energy = hit1lay1->GetEnergy() + hit1lay2->GetEnergy();
        vtx.positron_energy = hit2lay1->GetEnergy() + hit2lay2->GetEnergy();
        vtx.vertex_type = "type_2ht2ht";

        vtx.gamma_dir = vtx.ComputeGammaDirection();
        Vertices.push_back(vtx);

    return Vertices;
}

//////////////////////////////////////////
// TRACKPAIRS — actually performs the reconstruction
//////////////////////////////////////////

void MERTrackFirstTwoLayers::TrackPairs(MRERawEvent* RE)
{
    mout << "Event " << RE->GetEventID() << endl;

    if (RE->GetNRESEs() == 0) {
        mout << "No RESEs, skipping." << endl;
        RE->SetRejectionReason(MRERawEvent::c_RejectionNotEnoughHitsInTracker);
        return;
    }

    VertexFinder finder(m_Geometry);
    std::vector<Vertex> vertices = finder.FindVertices(RE, m_Theta, m_Phi);

    if (vertices.empty()) {
        mout << "No vertex found for event " << RE->GetEventID() << endl;
        RE->SetRejectionReason(MRERawEvent::c_RejectionNoVertexFound);
        return;
    }

    Vertex best = finder.TopVertex(vertices);

    // ── Build electron and positron tracks ────────────────────────────────────
    MRETrack* Electron = new MRETrack();
    MRETrack* Positron = new MRETrack();

    // Attach the vertex RESE as the start point of both tracks
    // (only valid for 1ht morphologies where vertex is a real RESE)
    if (best.GetRESE() != nullptr) {
        Electron->AddRESE(best.GetRESE());
        Electron->SetStartPoint(best.GetRESE());
        Positron->AddRESE(best.GetRESE());
        Positron->SetStartPoint(best.GetRESE());
    }

    // Add the downstream hits to each track
    // AllRESEs holds [hit1, hit2] for 1ht types, [h1l1, h2l1, h1l2, h2l2] for 2ht2ht
    if (best.vertex_type == "type_1ht2ht" || best.vertex_type == "type_1ht1ht") {
        if (best.AllRESEs.size() >= 2) {
            Electron->AddRESE(best.AllRESEs[0]);
            Positron->AddRESE(best.AllRESEs[1]);
        }
    } else if (best.vertex_type == "type_2ht2ht") {
        // AllRESEs = [hit1lay1, hit2lay1, hit1lay2, hit2lay2]
        if (best.AllRESEs.size() >= 4) {
            Electron->AddRESE(best.AllRESEs[0]); // layer 1, track 1
            Electron->AddRESE(best.AllRESEs[2]); // layer 2, track 1
            Positron->AddRESE(best.AllRESEs[1]); // layer 1, track 2
            Positron->AddRESE(best.AllRESEs[3]); // layer 2, track 2
        }
    }

    // ── Set directions (MEGAlib uses MVector, negate z convention matches Kalman) ──
    MVector eDir(best.electron_dir.X(), best.electron_dir.Y(), best.electron_dir.Z());
    MVector pDir(best.positron_dir.X(), best.positron_dir.Y(), best.positron_dir.Z());

    Electron->SetFixedDirection(eDir);
    Positron->SetFixedDirection(pDir);

    // ── Set energies (MEGAlib works in keV, GetEnergy() already returns keV) ──
    Electron->SetEnergy(best.electron_energy);
    Positron->SetEnergy(best.positron_energy);

    // quality factor... what value should be put here? 
    //Electron->SetQualityFactor(1.0);
    //Positron->SetQualityFactor(1.0);

    // ── Write back to the raw event ───────────────────────────────────────────
    RE->SetElectronTrack(Electron);
    RE->SetPositronTrack(Positron);
    RE->SetPairQualityFactor(1.0);
    RE->SetGoodEvent(true);
    RE->SetEventReconstructed(true);

    mout << "Event " << RE->GetEventID()
         << " | type: "    << best.vertex_type
         << " | vertex z: " << best.GetZPosition()
         << " | gamma dir: ("
         << best.gamma_dir.X() << ", "
         << best.gamma_dir.Y() << ", "
         << best.gamma_dir.Z() << ")" << endl;
}