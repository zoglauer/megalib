// MERTrackFirstTwoLayers.cxx
#include "MERTrackFirstTwoLayers.h"
#include "MRETrack.h"
#include "MREVertex.h"

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


//////////////////////////////////////////
// OUTER CLASS - MERTrackFirstTwoLayers
//////////////////////////////////////////

MERTrackFirstTwoLayers::MERTrackFirstTwoLayers(MGeometryRevan* geom)
 : MERTrack(), m_Geometry(geom)//, m_InterlayerDistance(0.0)
{
  mout << "Running new geometric reconstruction algorithm!" << endl;
  m_NLayersForVertexSearch = 3;
  m_FractionOfLayersWithTwoHits = 0.;
  m_RangeForVertexSearch = 30;
  m_NEmptyLayersAboveVertex = 1;
  m_NEmptyLayersBelowVertex = 1;
  m_N2htLayersStartVertex = 1;

  // Get interlayer distance from the first detector in the list
  //m_InterlayerDistance = m_DetectorList[0]->GetStructuralPitch().Z(); // defined in .det file
  //mout << "alaviron: Interlayer distance: " << m_InterlayerDistance << " cm" << endl;
  //if (m_InterlayerDistance == 0.0) {
 //throw std::runtime_error("Tracking detector has zero z-pitch specified in geometry — cannot determine interlayer distance.");
  //}
}


MERTrackFirstTwoLayers::~MERTrackFirstTwoLayers() {}



//////////////////////////////////////////
// STANDALONE FUNCTIONS
//////////////////////////////////////////

std::vector<CombinedHit> MERTrackFirstTwoLayers::CombiningHitsByAngle(
 const std::vector<MRESE*>& hits,
 const MVector& vertex_pos,
 double max_angle_deg)
{
  // Build initial list from the RESEs
  std::vector<CombinedHit> current;
  current.reserve(hits.size());
  for (MRESE* r : hits) {
 CombinedHit hp;
 hp.m_position = r->GetPosition();
 hp.m_energy   = r->GetEnergy();
 hp.m_reses    = { r };
 current.push_back(std::move(hp)); // adding to list
  }

  if (current.size() < 2) return {};

  // Iteratively merge the pair with the smallest opening angle as seen from the vertex until exactly two hits remain
  while (current.size() > 2) {
 double min_angle = std::numeric_limits<double>::infinity(); // start at infinity in order to get smaller angles
 int index_i = -1, index_j = -1;

 for (int i = 0; i < (int)current.size(); ++i) {
   MVector dir_i = (current[i].m_position - vertex_pos).Unit();
   for (int j = i + 1; j < (int)current.size(); ++j) {
     MVector dir_j = (current[j].m_position - vertex_pos).Unit();
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
 CombinedHit merged;
 merged.m_position = (current[index_i].m_position + current[index_j].m_position) * 0.5;
 merged.m_energy   = current[index_i].m_energy + current[index_j].m_energy;
 merged.m_reses    = current[index_i].m_reses;
 for (MRESE* r : current[index_j].m_reses) merged.m_reses.push_back(r);

 std::vector<CombinedHit> next;
 next.reserve(current.size() - 1);
 for (int k = 0; k < (int)current.size(); ++k) {
   if (k != index_i && k != index_j) next.push_back(current[k]);
 }
 next.push_back(std::move(merged));
 current = std::move(next);
  }

  return current;
}

//////////////////////////////////////////
// VERTEXFINDER CLASS
//////////////////////////////////////////
/*MERTrackFirstTwoLayers::VertexFinder::VertexFinder(MGeometryRevan* geom)
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
     throw std::runtime_error("No tracking detectors selected in the electron tracking options!");
 }

 // Get interlayer distance from the first detector in the list
 m_InterlayerDistance = m_DetectorList[0]->GetStructuralPitch().Z(); // defined in .det file

 if (m_InterlayerDistance == 0.0) {
     throw std::runtime_error("Tracking detector has zero z-pitch specified in geometry — cannot determine interlayer distance.");
 }
}*/

/*bool MERTrackFirstTwoLayers::VertexFinder::IsInTracker(MRESE* rese)
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
}*/

//////////////////////////////////////////

std::pair<std::pair<MVector, MVector>, std::pair<MVector, MVector>>
MERTrackFirstTwoLayers::BestHitPairing(const MVector& A1, const MVector& A2, const MVector& B1, const MVector& B2)
{
  auto dist = [](const MVector& a, const MVector& b) {
 return std::sqrt(std::pow(a.X()-b.X(),2) + std::pow(a.Y()-b.Y(),2) + std::pow(a.Z()-b.Z(), 2));
  };

  double d11 = dist(A1, B1), d12 = dist(A1, B2);
  double d21 = dist(A2, B1), d22 = dist(A2, B2);

  if ((d11 + d22) <= (d12 + d21)) return {{A1, B1}, {A2, B2}};
  else                            return {{A1, B2}, {A2, B1}};
}

//////////////////////////////////////////

MVector MERTrackFirstTwoLayers::CalculatingVertexPosition(const MVector& p1, const MVector& v1_in, const MVector& p2, const MVector& v2_in)
{
  MVector v1 = v1_in.Unit();
  MVector v2 = v2_in.Unit();

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

//MERTrackFirstTwoLayers::Vertex
MREVertex* MERTrackFirstTwoLayers::TopVertex(const std::vector<MREVertex*>& vertex_list)
{
  // Returns the vertex with the largest z (shallowest layer)
  //return std::max_element(vertex_list.begin(), vertex_list.end(), [](const MREVertex& a, const MREVertex& b){ return a.GetPositionZ() < b.GetPositionZ(); });
  const MREVertex* tmp = *(std::max_element(vertex_list.begin(), vertex_list.end(),
      [](const MREVertex* a, const MREVertex* b){ return a->GetPositionZ() < b->GetPositionZ(); }));
  //mout << "alaviron: Previous known good:\n  " << tmp->ToString() << " to now:\n  " << std::min_element(vertex_list.begin(), vertex_list.end(), CompareRESEByZ())->ToString() << endl;
  //return *std::min_element(vertex_list.begin(), vertex_list.end(), CompareRESEByZ());
  return const_cast<MREVertex*>(tmp);
}

//////////////////////////////////////////

std::vector<MREVertex*> MERTrackFirstTwoLayers::FindVertices(MRERawEvent* RE)
{
  std::vector<MREVertex*> Vertices;
  mout << "alaviron: Starting vertex finding for event " << RE->GetEventID() << " with " << RE->GetNRESEs() << " RESEs." << endl;

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
  int nRejectedCombining       = 0;

  // 1ht-2ht and 1ht-1ht event processing
  for (MRESE* candidate : RESEs) {

    // Require candidate to be the only hit in its layer
    bool OnlyHitInLayer = true;
    for (MRESE* rese : RESEs) {
    if (rese == candidate) continue;
    if (m_Geometry->AreInSameLayer(candidate, rese)) { OnlyHitInLayer = false; break; }
    }
    if (!OnlyHitInLayer) { nRejectedNotOnlyHit++;  continue;}

    // Count hits above and below by layer distance
    std::vector<int> NBelow(SearchRange, 0);
    std::vector<int> NAbove(SearchRange, 0);

    for (MRESE* rese : RESEs) {
    if (rese == candidate) continue;
    int Distance = m_Geometry->GetLayerDistance(candidate, rese);
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
        if (m_Geometry->GetLayerDistance(candidate, rese) == -dist)
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

    // Use as CombinedHit so that combining can return merged positions/energies
    CombinedHit hp1, hp2;
    bool hits_valid = false;

    if (selected_distance == 1) {
      // 1ht-2ht: combine by opening angle from the candidate position
      auto combined = MERTrackFirstTwoLayers::CombiningHitsByAngle(selected_layer_hits, vtx_cand, 30.0); // using 30 deg as max angle based on previous analysis

      if ((int)combined.size() >= 2) {
        hp1 = combined[0];
        hp2 = combined[1];
        hits_valid = true;
      }
    } else {
      // 1ht-1ht: multiple single-hit layers before the two-hit layer--just take the first two hits in that layer directly
      std::vector<MRESE*> layer_hits_below;
      for (MRESE* rese : RESEs) {
        if (m_Geometry->GetLayerDistance(candidate, rese) == -selected_distance) layer_hits_below.push_back(rese);
      }
      if ((int)layer_hits_below.size() >= 2) {
        hp1 = { layer_hits_below[0]->GetPosition(), layer_hits_below[0]->GetEnergy(), { layer_hits_below[0] } };
        hp2 = { layer_hits_below[1]->GetPosition(), layer_hits_below[1]->GetEnergy(), { layer_hits_below[1] } };
        hits_valid = true;
      }
    }

    if (!hits_valid) {nRejectedCombining++; continue;}

    // Build vertex
    MREVertex* vtx = new MREVertex(candidate);
    vtx->SetPosition(candidate->GetPosition());
    vtx->SetEnergy(candidate->GetEnergy());
    vtx->SetTime(candidate->GetTime());
    vtx->SetDetector(candidate->GetDetector());

/*        std::vector<MRESE*> allRESEs;
     for (MRESE* r : hp1.m_reses) allRESEs.push_back(r);
     for (MRESE* r : hp2.m_reses) allRESEs.push_back(r);

     Vertex vtx(candidate, allRESEs);
     vtx.m_EventID = RE->GetEventID();

     vtx.m_TrackHits = { hp1, hp2 };*/

     // Assign track directions and energies 
    MVector vtx_pos = vtx->GetPosition();

    if (selected_distance == 1) {
      // 1ht-2ht
      vtx->m_electron_dir    = (hp1.m_position - vtx_pos).Unit();
      vtx->m_positron_dir    = (hp2.m_position - vtx_pos).Unit();
      vtx->m_electron_energy = hp1.m_energy;
      vtx->m_positron_energy = hp2.m_energy;
      vtx->m_vertex_type     = "type_1ht2ht";

    } else {
      // 1ht-1ht: draw a line between the first two recorded hits and the vertex
      if ((int)RESEs.size() >= 2) {
        MRESE* first_hit  = RESEs[0];
        MRESE* second_hit = RESEs[1];

        MVector fp = first_hit->GetPosition();
        MVector sp = second_hit->GetPosition();

        MVector first_pos  = { fp.X(), fp.Y(), fp.Z() };
        MVector second_pos = { sp.X(), sp.Y(), sp.Z() };

        MVector best_track      = (vtx_pos - first_pos).Unit();
        MVector alternate_track = (second_pos - vtx_pos).Unit();

        auto allclose = [](const MVector& a, const MVector& b) {
          return std::fabs(a.X()-b.X()) < 1e-6 &&
                std::fabs(a.Y()-b.Y()) < 1e-6 &&
                std::fabs(a.Z()-b.Z()) < 1e-6;
        };

        MVector track;
        if (allclose(first_pos, vtx_pos))       track = alternate_track;
        else if (allclose(second_pos, vtx_pos)) track = best_track;
        else                                    track = best_track;

        vtx->m_electron_dir    = track;
        vtx->m_positron_dir    = track;
        vtx->m_electron_energy = first_hit->GetEnergy();
        vtx->m_positron_energy = second_hit->GetEnergy();
      }
      vtx->m_vertex_type = "type_1ht1ht";
    }

    vtx->ComputeGammaDirection();
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
    if (m_Geometry->GetLayerDistance(rese, top_hit) == 0)
      hits_in_first_layer.push_back(rese);
    }

    if ((int)hits_in_first_layer.size() != 2) return Vertices;

    // Hits in the layer immediately below
    std::vector<MRESE*> hits_in_layer_below;
    for (MRESE* rese : RESEs) {
      if (m_Geometry->GetLayerDistance(hits_in_first_layer[0], rese) == -1)
        hits_in_layer_below.push_back(rese);
    }

    if ((int)hits_in_layer_below.size() != 2) return Vertices;

    // Apply LayerRequirement check on the 2ht topology
    std::vector<int> NBelow(SearchRange, 0);
    for (MRESE* rese : RESEs) {
      int dist = m_Geometry->GetLayerDistance(top_hit, rese);
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

    /*auto toMVector = [](MRESE* r) -> MVector {
      MVector p = r->GetPosition();
      return { p.X(), p.Y(), p.Z() };
    };

    MVector A1 = toMVector(hit1lay1), A2 = toMVector(hit2lay1);
    MVector B1 = toMVector(hit1lay2), B2 = toMVector(hit2lay2);*/
    MVector A1 = hit1lay1->GetPosition();
    MVector A2 = hit2lay1->GetPosition();
    MVector B1 = hit1lay2->GetPosition();
    MVector B2 = hit2lay2->GetPosition();

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

    MREVertex* vtx = new MREVertex(nullptr,
        {hit1lay1, hit2lay1, hit1lay2, hit2lay2}, &vertex_point);
    vtx->m_EventID = RE->GetEventID();

    vtx->m_electron_dir    = v1.Unit();
    vtx->m_positron_dir    = v2.Unit();
    vtx->m_electron_energy = hit1lay1->GetEnergy() + hit1lay2->GetEnergy();
    vtx->m_positron_energy = hit2lay1->GetEnergy() + hit2lay2->GetEnergy();
    vtx->m_vertex_type     = "type_2ht2ht";

    vtx->ComputeGammaDirection();
    Vertices.push_back(vtx);
  }
  mout << "alaviron: Finished vertex finding for event " << RE->GetEventID() << ". Found " << Vertices.size() << " vertices." << endl;
  return Vertices;
}

//////////////////////////////////////////
// ANALYZE — routes the reconstruction for only pairs to the TrackPairs reconstruction
//////////////////////////////////////////

bool MERTrackFirstTwoLayers::Analyze(MRawEventIncarnations* REList)
{
  MERConstruction::Analyze(REList);
  mout << "alaviron: Starting first two layers tracking::Analyze of event " << m_List->GetRawEventAt(0)->GetEventID() << endl;
  for (int e = 0; e < m_List->GetNRawEvents(); e++) {
    MRERawEvent* RE = m_List->GetRawEventAt(e);
    if (RE->GetEventType() == c_PairEvent) {
      mout << "alaviron: 0 " << RE << endl;
      TrackPairs(RE);
      mout << "alaviron: 4 " << RE << endl;
      RE->SetEventReconstructed(true);
      mout << "alaviron: 5 " << RE << endl;
    }
  }
  return true;
}
/*
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
}*/

//////////////////////////////////////////
// TRACKPAIRS — actually performs the reconstruction
//////////////////////////////////////////

void MERTrackFirstTwoLayers::TrackPairs(MRERawEvent* RE)
{
  mout << "alaviron: Starting TrackPairs for event " << RE->GetEventID() << endl;
  if (RE->GetNRESEs() == 0) {
    // mout << "No RESEs, skipping." << endl;
    RE->SetRejectionReason(MRERawEvent::c_RejectionNotEnoughHitsInTracker);
    return;
  }

 //VertexFinder finder(m_Geometry, m_DetectorList);
  std::vector<MREVertex*> vertices = FindVertices(RE);
  mout << "alaviron - left blank" << endl;
  mout << "alaviron: Found " << vertices.size() << " vertices for event " << RE->GetEventID() << endl;
  for (auto& v:vertices) {
    mout << "alaviron: vertex " << v->GetID() << " of type " << v->GetType();
  }
  mout << endl;

  if (vertices.empty()) {
    mout << "No vertex found for event " << RE->GetEventID() << endl;
    RE->SetRejectionReason(MRERawEvent::c_RejectionPairEventNoVertex);
    return;
  }

  MREVertex* best = TopVertex(vertices);
  mout << "alaviron: Selected vertex " << best->GetID() << " of type " << best->GetType() << " with " << best->m_AllRESEs.size() << " contributing hits" << endl;
  for (auto& r : best->m_AllRESEs) {
    mout << "  contributing hit: " << r->ToString() << endl;
  }

  // Set vertex on raw event for MEGAlib bookkeeping
  if (best->GetRESE() != nullptr) {
    RE->SetVertex(best->GetRESE());
  } else if (best->m_AllRESEs.size() > 0) {
    RE->SetVertex(best->m_AllRESEs[0]);
  }
  RE->SetVertexDirection(-1);

  // Build electron and positron tracks 
  MRETrack* Electron = new MRETrack();
  MRETrack* Positron = new MRETrack();

  if (best->GetRESE() != nullptr) {
    Electron->AddRESE(best->GetRESE());
    Electron->SetStartPoint(best->GetRESE());
    Positron->AddRESE(best->GetRESE());
    Positron->SetStartPoint(best->GetRESE());
  }

  if (best->m_vertex_type == "type_1ht2ht" || best->m_vertex_type == "type_1ht1ht") {
    if (best->m_TrackHits.size() >= 2) {
      for (MRESE* r : best->m_TrackHits[0].m_reses) Electron->AddRESE(r);
      for (MRESE* r : best->m_TrackHits[1].m_reses) Positron->AddRESE(r);
    }
  } else if (best->m_vertex_type == "type_2ht2ht") {
    // AllRESEs = [hit1lay1, hit2lay1, hit1lay2, hit2lay2]
    if (best->m_AllRESEs.size() >= 4) {
      Electron->AddRESE(best->m_AllRESEs[0]); // layer 1, track 1
      Electron->AddRESE(best->m_AllRESEs[2]); // layer 2, track 1
      Positron->AddRESE(best->m_AllRESEs[1]); // layer 1, track 2
      Positron->AddRESE(best->m_AllRESEs[3]); // layer 2, track 2
    }
  }

  // Set directions 
  MVector eDir(best->m_electron_dir.X(), best->m_electron_dir.Y(), best->m_electron_dir.Z());
  MVector pDir(best->m_positron_dir.X(), best->m_positron_dir.Y(), best->m_positron_dir.Z());

  Electron->SetFixedDirection(eDir);
  Positron->SetFixedDirection(pDir);

  // Set energies
  Electron->SetEnergy(best->m_electron_energy);
  Positron->SetEnergy(best->m_positron_energy);

  mout << "alaviron: 1 " << RE << endl;
  // Write back to the raw event 
  RE->SetElectronTrack(Electron);
  RE->SetPositronTrack(Positron);
  // PairQualityFactor to keep track of vertex type
  if (best->m_vertex_type == "type_1ht2ht") RE->SetPairQualityFactor(1.0);
  else if (best->m_vertex_type == "type_1ht1ht") RE->SetPairQualityFactor(2.0);
  else RE->SetPairQualityFactor(3.0);
  // Set event as reconstructed
  RE->SetGoodEvent(true);
  RE->SetEventReconstructed(true);
  mout << "alaviron: 2 " << RE << endl;

  mout << "Event " << RE->GetEventID()
    << " | type: "    << best->m_vertex_type
    << " | vertex z: " << best->GetPosition().Z()
    << " | gamma dir: ("
    << best->m_gamma_dir.X() << ", "
    << best->m_gamma_dir.Y() << ", "
    << best->m_gamma_dir.Z() << ")" << endl;
  mout << "alaviron: 3 " << RE << endl;
  return;
}
