// MERTrackFirstTwoLayers.cxx
#include "MERTrackFirstTwoLayers.h"
#include "MRETrack.h" 

#include <iostream>
#include <vector>
#include <cmath>
#include <utility>
#include <algorithm>
#include <stdexcept>
#include <string>
#include <limits>
#include "MAssert.h"

using namespace std;

#ifdef ___CLING___
ClassImp(MERTrackFirstTwoLayers)
#endif

// Normalization function
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
    : MERTrack(), m_Geometry(geom)
{
    mout << "Running new geometric reconstruction algorithm!" << endl;
}


MERTrackFirstTwoLayers::~MERTrackFirstTwoLayers() {}

//////////////////////////////////////////
// VERTEX CLASS
//////////////////////////////////////////
MERTrackFirstTwoLayers::Vertex::Vertex(MRESE* rese_in, std::vector<MRESE*> allRESEs, MVector* position):
      electron_dir({0,0,0}),
      positron_dir({0,0,0}),
      gamma_dir({0,0,0}),
      electron_energy(0.0),
      positron_energy(0.0),
      vertex_type("unknown"),
      EventID(-1),
      AllRESEs(allRESEs),
      rese(rese_in)
{
    if (position != nullptr) {
        x = position->X();
        y = position->Y();
        z = position->Z();
        id = (rese != nullptr) ? rese->GetID() : -1; // placeholder ID -> can update
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
    if (electron_energy == 0.0 || positron_energy == 0.0)
        throw std::runtime_error("Track energies are zero.");

    MVector d1 = Normalize(electron_dir);
    MVector d2 = Normalize(positron_dir);

    // Using an energy-weighted reconstruction
    MVector weighted_sum = d1 * electron_energy + d2 * positron_energy;
    gamma_dir = (weighted_sum / (electron_energy + positron_energy)) * (-1.0);
    gamma_dir = Normalize(gamma_dir);

    return gamma_dir;
}

//////////////////////////////////////////
// STANDALONE FUNCTIONS
//////////////////////////////////////////

std::vector<MERTrackFirstTwoLayers::ClusteredHit>
MERTrackFirstTwoLayers::ClusteringHitsByAngle(
    const std::vector<MRESE*>& hits,
    const MVector& vertex_pos,
    double max_angle_deg)
{
    // Build initial list from the RESEs
    std::vector<ClusteredHit> current;
    current.reserve(hits.size());
    for (MRESE* r : hits) {
        ClusteredHit hp;
        hp.position = r->GetPosition();
        hp.energy   = r->GetEnergy();
        hp.reses    = { r };
        current.push_back(std::move(hp)); // adding to list
    }

    if (current.size() < 2) return {};

    // Iteratively merge the pair with the smallest opening angle as seen from the vertex until exactly two hits remain
    while (current.size() > 2) {
        double min_angle = std::numeric_limits<double>::infinity(); // start at infinity in order to get smaller angles
        int index_i = -1, index_j = -1;

        for (int i = 0; i < (int)current.size(); ++i) {
            MVector dir_i = Normalize(current[i].position - vertex_pos);
            for (int j = i + 1; j < (int)current.size(); ++j) {
                MVector dir_j = Normalize(current[j].position - vertex_pos);
                double cosine = std::max(-1.0, std::min(1.0, dir_i * dir_j));
                double angle  = std::acos(cosine) * 180.0 / M_PI;
                if (angle < min_angle) {
                    min_angle = angle;
                    index_i   = i;
                    index_j   = j;
                }
            }
        }

        // Reject event if no valid close pair exists
        if (min_angle > max_angle_deg) return {};   

        // Merge the closest angle pair: midpoint position, summed energy -> CAN CHANGE SUMMED ENERGY THIS IS TEMPORARY
        ClusteredHit merged;
        merged.position = (current[index_i].position + current[index_j].position) * 0.5;
        merged.energy   = current[index_i].energy + current[index_j].energy;
        merged.reses    = current[index_i].reses;
        for (MRESE* r : current[index_j].reses)
            merged.reses.push_back(r);

        std::vector<ClusteredHit> next;
        next.reserve(current.size() - 1);
        for (int k = 0; k < (int)current.size(); ++k)
            if (k != index_i && k != index_j) next.push_back(current[k]);
        next.push_back(std::move(merged));
        current = std::move(next);
    }

    return current;
}

//////////////////////////////////////////
// VERTEXFINDER CLASS
//////////////////////////////////////////
MERTrackFirstTwoLayers::VertexFinder::VertexFinder(MGeometryRevan* geom)
    : Geometry(geom), m_InterlayerDistance(0.0)
{
    // Build detector list from all MDStrip2D detectors in the geometry
    for (unsigned int i = 0; i < Geometry->GetNDetectors(); ++i) {
        MDDetector* det = Geometry->GetDetectorAt(i);
        if (det != nullptr && dynamic_cast<MDStrip2D*>(det) != nullptr) {
            m_DetectorList.push_back(det);
        }
    }

    if (m_DetectorList.empty()) {
        throw std::runtime_error("No MDStrip2D detectors found in geometry.");
    }

    // Get interlayer distance from the first detector in the list
    m_InterlayerDistance = m_DetectorList[0]->GetStructuralPitch().Z(); // defined in .det file

    if (m_InterlayerDistance == 0.0) {
        throw std::runtime_error("MDStrip2D detector has zero z-pitch — cannot determine interlayer distance.");
    }
}

bool MERTrackFirstTwoLayers::VertexFinder::IsInTracker(MRESE* rese)
{
    if (rese->GetType() == MRESE::c_Track) return true;

    MDVolumeSequence* VS = rese->GetVolumeSequence();
    if (VS == nullptr) return false;

    MDDetector* det = VS->GetDetector();
    if (det == nullptr) return false;

    for (MDDetector* listed : m_DetectorList) {
        if (det == listed) return true;
    }

    return false;
}

//////////////////////////////////////////

std::pair<std::pair<MVector, MVector>, std::pair<MVector, MVector>>
MERTrackFirstTwoLayers::VertexFinder::BestHitPairing(
    const MVector& A1, const MVector& A2,
    const MVector& B1, const MVector& B2)
{
    auto dist = [](const MVector& a, const MVector& b) {
        return std::sqrt(std::pow(a.X()-b.X(),2) + std::pow(a.Y()-b.Y(),2) + std::pow(a.Z()-b.Z(), 2));
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

    double a = v1*v1;
    double b = v1*v2;
    double c = v2*v2;
    double d = v1*w0;
    double e = v2*w0;

    double denom = a*c - b*b;

    if (std::fabs(denom) < 1e-6) return {std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN()};
    
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
    // Returns the vertex with the largest z (shallowest layer)
    return *std::max_element(vertex_list.begin(), vertex_list.end(),
        [](const Vertex& a, const Vertex& b){ return a.GetZPosition() < b.GetZPosition(); });
}

//////////////////////////////////////////

std::vector<MERTrackFirstTwoLayers::Vertex>
MERTrackFirstTwoLayers::VertexFinder::FindVertices(MRERawEvent* RE)
{
    std::vector<Vertex> Vertices;

    const int    LayerRequirement    = 4; // matches the revan requirement
    const int    SearchRange         = 30;
    const int    SearchLayersFor2Hit = 5;

    // Filter RESEs to those in the tracker with energy > 0
    std::vector<MRESE*> RESEs;
    for (int i = 0; i < RE->GetNRESEs(); i++) {
        MRESE* r = RE->GetRESEAt(i);
        if (IsInTracker(r) && r->GetEnergy() > 0)
            RESEs.push_back(r);
    }

    // Sort shallowest -> deepest (largest z first)
    std::sort(RESEs.begin(), RESEs.end(), [](MRESE* a, MRESE* b){
        return a->GetPosition().Z() > b->GetPosition().Z();
    });

    bool vertex_created_for_event = false;
    
    // counters to print debugging statements
    int nRejectedNotOnlyHit       = 0;
    int nRejectedHitAbove         = 0;
    int nRejectedLayerRequirement = 0;
    int nRejectedNo2HitLayer      = 0;
    int nRejectedClustering       = 0;

    // 1ht-2ht and 1ht-1ht event processing
    for (MRESE* candidate : RESEs) {

        // Require candidate to be the only hit in its layer
        bool OnlyHitInLayer = true;
        for (MRESE* rese : RESEs) {
            if (rese == candidate) continue;
            if (Geometry->AreInSameLayer(candidate, rese)) { OnlyHitInLayer = false; break; }
        }
        if (!OnlyHitInLayer) { nRejectedNotOnlyHit++;  continue;}

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
        if (NAbove[1] != 0) { nRejectedHitAbove++; continue;}

        // Count layers with 2+ hits below
        int StartIndex = 0, StopIndex = 0;
        int LayersWithAtLeastTwoHits = 0;

        for (int dist = 1; dist < SearchRange - 1; ++dist) {
            if (NBelow[dist] == 0 && NBelow[dist+1] == 0) break;
            StopIndex = dist;
            if (StartIndex == 0 && NBelow[dist] > 1)  StartIndex = dist;
            if (StartIndex != 0 && NBelow[dist] >= 2) LayersWithAtLeastTwoHits++;
        }

            if (LayersWithAtLeastTwoHits < LayerRequirement) {nRejectedLayerRequirement++; continue;}

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
                selected_distance   = dist;
                break;
            }   
        }

        if (selected_distance == -1) {nRejectedNo2HitLayer++; continue;}

        // Select hit1 and hit2
        MVector cpos     = candidate->GetPosition();
        MVector vtx_cand = { cpos.X(), cpos.Y(), cpos.Z() };

        // Use as ClusteredHit so that clustering can return merged positions/energies
        ClusteredHit hp1, hp2;
        bool hits_valid = false;

        if (selected_distance == 1) {
            // 1ht-2ht: cluster by opening angle from the candidate position
            auto clustered = MERTrackFirstTwoLayers::ClusteringHitsByAngle(
                selected_layer_hits, vtx_cand, 30.0); // using 30 deg as max angle based on previous analysis

            if ((int)clustered.size() >= 2) {
                hp1 = clustered[0];
                hp2 = clustered[1];
                hits_valid = true;
            }
        } else {
            // 1ht-1ht: multiple single-hit layers before the two-hit layer--just take the first two hits in that layer directly
            std::vector<MRESE*> layer_hits_below;
            for (MRESE* rese : RESEs) {
                if (Geometry->GetLayerDistance(candidate, rese) == -selected_distance)
                    layer_hits_below.push_back(rese);
            }
            if ((int)layer_hits_below.size() >= 2) {
                hp1 = { layer_hits_below[0]->GetPosition(), layer_hits_below[0]->GetEnergy(), { layer_hits_below[0] } };
                hp2 = { layer_hits_below[1]->GetPosition(), layer_hits_below[1]->GetEnergy(), { layer_hits_below[1] } };
                hits_valid = true;
            }
        }

        if (!hits_valid) {nRejectedClustering++; continue;}

        // Build vertex 
        std::vector<MRESE*> allRESEs;
        for (MRESE* r : hp1.reses) allRESEs.push_back(r);
        for (MRESE* r : hp2.reses) allRESEs.push_back(r);

        Vertex vtx(candidate, allRESEs);
        vtx.EventID = RE->GetEventID();

        vtx.TrackHits = { hp1, hp2 };

        // Assign track directions and energies 
        MVector vtx_pos = vtx.GetPosition();

        if (selected_distance == 1) {
            // 1ht-2ht
            vtx.electron_dir    = Normalize(hp1.position - vtx_pos);
            vtx.positron_dir    = Normalize(hp2.position - vtx_pos);
            vtx.electron_energy = hp1.energy;
            vtx.positron_energy = hp2.energy;
            vtx.vertex_type     = "type_1ht2ht";

        } else {
            // 1ht-1ht: draw a line between the first two recorded hits and the vertex
            if ((int)RESEs.size() >= 2) {
                MRESE* first_hit  = RESEs[0];
                MRESE* second_hit = RESEs[1];

                MVector fp = first_hit->GetPosition();
                MVector sp = second_hit->GetPosition();

                MVector first_pos  = { fp.X(), fp.Y(), fp.Z() };
                MVector second_pos = { sp.X(), sp.Y(), sp.Z() };

                MVector best_track      = Normalize(vtx_pos - first_pos);
                MVector alternate_track = Normalize(second_pos - vtx_pos);

                auto allclose = [](const MVector& a, const MVector& b) {
                    return std::fabs(a.X()-b.X()) < 1e-6 &&
                           std::fabs(a.Y()-b.Y()) < 1e-6 &&
                           std::fabs(a.Z()-b.Z()) < 1e-6;
                };

                MVector track;
                if (allclose(first_pos, vtx_pos))       track = alternate_track;
                else if (allclose(second_pos, vtx_pos)) track = best_track;
                else                                    track = best_track;

                vtx.electron_dir    = track;
                vtx.positron_dir    = track;
                vtx.electron_energy = first_hit->GetEnergy();
                vtx.positron_energy = second_hit->GetEnergy();
            }
            vtx.vertex_type = "type_1ht1ht";
        }

        vtx.gamma_dir = vtx.ComputeGammaDirection();
        Vertices.push_back(vtx);
        vertex_created_for_event = true;
    }

    // 2ht-2ht: only attempt if no vertex was found above 
    if (!vertex_created_for_event) {

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

        MVector p1 = track1.first,  q1 = track1.second;
        MVector p2 = track2.first,  q2 = track2.second;

        MVector v1 = q1 - p1;
        MVector v2 = q2 - p2;

        // Point of closest approach -> vertex position
        MVector vertex_point = CalculatingVertexPosition(p1, v1, p2, v2);

        // Parallel-track check
        if (std::isnan(vertex_point.X()) || std::isnan(vertex_point.Y()) || std::isnan(vertex_point.Z()))
            return Vertices;

        Vertex vtx(nullptr,
                   {hit1lay1, hit2lay1, hit1lay2, hit2lay2}, &vertex_point);
        vtx.EventID = RE->GetEventID();

        vtx.electron_dir    = Normalize(v1);
        vtx.positron_dir    = Normalize(v2);
        vtx.electron_energy = hit1lay1->GetEnergy() + hit1lay2->GetEnergy();
        vtx.positron_energy = hit2lay1->GetEnergy() + hit2lay2->GetEnergy();
        vtx.vertex_type     = "type_2ht2ht";

        vtx.gamma_dir = vtx.ComputeGammaDirection();
        Vertices.push_back(vtx);
    }

    return Vertices;
}

//////////////////////////////////////////
// ANALYZE — routes the reconstruction for only pairs to the TrackPairs reconstruction
//////////////////////////////////////////

bool MERTrackFirstTwoLayers::Analyze(MRawEventIncarnations* REList)
{
    MERConstruction::Analyze(REList);

    for (int e = 0; e < m_List->GetNRawEvents(); e++) {
        MRERawEvent* RE = m_List->GetRawEventAt(e);
        if (RE->GetEventType() == c_PairEvent) {
            TrackPairs(RE);
            RE->SetEventReconstructed(true);
        }
    }
    return true;
}

//////////////////////////////////////////
// Implementing modified version of CheckForPair function in the base class
//////////////////////////////////////////

MRawEventIncarnations* MERTrackFirstTwoLayers::CheckForPair(MRERawEvent* RE)
{
  mdebug<<"Searching for a pair vertex"<<endl;
  mout << "CheckForPair called for event " << RE->GetEventID() << endl;

  MRawEventIncarnations *List = 0;
  bool OnlyHitInLayer = false;
  unsigned int SearchRange = 30;

  // Create a list of RESEs sorted by depth in tracker
  vector<MRESE*> ReseList;
  for (int h = 0; h < RE->GetNRESEs(); h++) {
    if (IsInTracker(RE->GetRESEAt(h)) == false) continue;

    ReseList.push_back(RE->GetRESEAt(h));
  }
  sort(ReseList.begin(), ReseList.end(), CompareRESEByZ());

  mdebug<<"RESE's sorted by depth: "<<endl;
  vector<MRESE*>::iterator Iterator1;
  vector<MRESE*>::iterator Iterator2;
  for (Iterator1 = ReseList.begin(); Iterator1 != ReseList.end(); Iterator1++) {
    mdebug<<(*Iterator1)->GetID()<<": "<<(*Iterator1)->GetPosition().Z()<<endl;
  }

  // For each of the RESEs in the list check if it could be the first of the vertex
  for (Iterator1 = ReseList.begin(); Iterator1 != ReseList.end(); Iterator1++) {

    // If it is a single hit, and if it is the only one in its layer:
    OnlyHitInLayer = true;
    for (Iterator2 = ReseList.begin(); Iterator2 != ReseList.end(); Iterator2++) {
      if ((*Iterator1) == (*Iterator2)) continue;
      if (m_Geometry->AreInSameLayer((*Iterator1), (*Iterator2)) == true) {
        OnlyHitInLayer = false;
        break;
      }
    }
    if (OnlyHitInLayer == false) continue;
    mdebug<<"Search vertex: Only hit in layer:"<<endl;
    mdebug<<(*Iterator1)->ToString()<<endl;

    // We only have one hit:
    vector<int> NBelow(SearchRange, 0);
    vector<int> NAbove(SearchRange, 0);

    int Distance;
    for (Iterator2 = ReseList.begin(); Iterator2 != ReseList.end(); Iterator2++) {
      if ((*Iterator1) == (*Iterator2)) continue;

      Distance = m_Geometry->GetLayerDistance((*Iterator1), (*Iterator2));
      if (Distance > 0 && Distance < int(SearchRange)) NAbove[Distance]++;
      if (Distance < 0 && abs(Distance) < int(SearchRange)) NBelow[abs(Distance)]++;
      massert(Distance != 0);
    }

    // Pair starting from top
    MRESE* Vertex = 0;
    int VertexDirection = 0;

    // Check for vertex below
    if (NAbove[1] == 0) {
      int StartIndex = 0; // We start when we have 2 hits for the first time
      int StopIndex = 0; // We stop when we skip 2 layers for the first time
      int LayersWithAtLeastTwoHitsBetweenStartAndStop = 0;


      for (unsigned int d = 1; d < SearchRange-1; ++d) {
        if (NBelow[d] == 0 && NBelow[d+1] == 0) break;
        StopIndex = d;
        if (StartIndex == 0 && NBelow[d] > 1) StartIndex = d;

        if (StartIndex != 0) {
          if (NBelow[d] >= 2) ++LayersWithAtLeastTwoHitsBetweenStartAndStop;
        }
      }

      // Since we can have just a single track at the end, move upward until we have at least two hits in a row
      for (unsigned int d = StopIndex; d > 2; --d) {
        if (NBelow[d-1] >= 2 && NBelow[d-2] >= 2) break;
        StopIndex = d;
      }

      mdebug<<"Search vertex ("<<(*Iterator1)->GetPosition().Z()<<"): Above: ";
      for (int i: NAbove) mdebug<<i<<" ";
      mdebug<<endl;
      mdebug<<"Search vertex ("<<(*Iterator1)->GetPosition().Z()<<"): Below: ";
      for (int i: NBelow) mdebug<<i<<" ";
      mdebug<<endl;

      mdebug<<"Vertex statistics (max: "<<SearchRange<<"): layers used: "<<StopIndex<<", start of 2+ hits: "<<StartIndex<<"  layers with 2+ hits between start and stop: "<<LayersWithAtLeastTwoHitsBetweenStartAndStop<<" ("<<((StopIndex-StartIndex > 0) ? 100.0*LayersWithAtLeastTwoHitsBetweenStartAndStop/(StopIndex-StartIndex) : 0)<<"%)"<<endl;

      if (LayersWithAtLeastTwoHitsBetweenStartAndStop > 3) {
        // REMOVED THE ORIGINAL 4+ LAYER REQ AND >50% W/ PAIRS
        Vertex = (*Iterator1);
        VertexDirection = -1;
      }
    }

    if (Vertex != 0) {
      if (List == 0) List = new MRawEventIncarnations();
      RE->SetVertex(Vertex);
      RE->SetVertexDirection(VertexDirection);
      MRERawEvent *New = RE->Duplicate();
      RE->SetVertex(0);
      List->AddRawEvent(New);
      mdebug<<"Search vertex: Found vertex: "<<Vertex->GetID()<<endl;
      break; 
    }
  }

  return List;
}

//////////////////////////////////////////
// TRACKPAIRS — actually performs the reconstruction
//////////////////////////////////////////

void MERTrackFirstTwoLayers::TrackPairs(MRERawEvent* RE)
{
    if (RE->GetNRESEs() == 0) {
        // mout << "No RESEs, skipping." << endl;
        RE->SetRejectionReason(MRERawEvent::c_RejectionNotEnoughHitsInTracker);
        return;
    }

    VertexFinder finder(m_Geometry, m_DetectorList);
    std::vector<Vertex> vertices = finder.FindVertices(RE);

    if (vertices.empty()) {
        // mout << "No vertex found for event " << RE->GetEventID() << endl;
        RE->SetRejectionReason(MRERawEvent::c_RejectionPairEventNoVertex);
        return;
    }

    Vertex best = finder.TopVertex(vertices);

        // Set vertex on raw event for MEGAlib bookkeeping
        if (best.GetRESE() != nullptr) {
            RE->SetVertex(best.GetRESE());
        } else if (best.AllRESEs.size() > 0) {
            RE->SetVertex(best.AllRESEs[0]);
        }
        RE->SetVertexDirection(-1);

    // Build electron and positron tracks 
    MRETrack* Electron = new MRETrack();
    MRETrack* Positron = new MRETrack();

    if (best.GetRESE() != nullptr) {
        Electron->AddRESE(best.GetRESE());
        Electron->SetStartPoint(best.GetRESE());
        Positron->AddRESE(best.GetRESE());
        Positron->SetStartPoint(best.GetRESE());
    }

    if (best.vertex_type == "type_1ht2ht" || best.vertex_type == "type_1ht1ht") {
        if (best.TrackHits.size() >= 2) {
            for (MRESE* r : best.TrackHits[0].reses) Electron->AddRESE(r);
            for (MRESE* r : best.TrackHits[1].reses) Positron->AddRESE(r);
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

    // Set directions 
    MVector eDir(best.electron_dir.X(), best.electron_dir.Y(), best.electron_dir.Z());
    MVector pDir(best.positron_dir.X(), best.positron_dir.Y(), best.positron_dir.Z());

    Electron->SetFixedDirection(eDir);
    Positron->SetFixedDirection(pDir);

    // Set energies
    Electron->SetEnergy(best.electron_energy);
    Positron->SetEnergy(best.positron_energy);


    // Write back to the raw event 
    RE->SetElectronTrack(Electron);
    RE->SetPositronTrack(Positron);
    //RE->SetPairQualityFactor(1.0); needed?
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
