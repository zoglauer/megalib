/*
* MERTrackKalman2D.cxx
*
*
* Copyright (C) by Paolo Cumani & Andreas Zoglauer.
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
// MERTrackKalman2D
//
// Base class for event reconstruction tasks, e.g. find clusters, tracks,
// Compton sequence
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MERTrackKalman2D.h"

// Standard libs:
#include <list>
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <functional>
using namespace std;

// ROOT libs:
#include "TMath.h"

// MEGAlib libs:
#include "MAssert.h"
#include "MStreams.h"
#include "MRESE.h"
#include "MRESEIterator.h"
#include "MRETrack.h"
#include "MREHit.h"
#include "MMuonEvent.h"
#include "MTimer.h"
#include "MFile.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MERTrackKalman2D)
#endif


////////////////////////////////////////////////////////////////////////////////


MERTrackKalman2D::MERTrackKalman2D() : MERTrack()
{
  // Construct an instance of MERTrackKalman2D
  w=pow(tan(60*(TMath::Pi())/180.),2);
  sfg_factor=24;
  start_nrg_fit=20;
  n_fit = 7;

}


////////////////////////////////////////////////////////////////////////////////


bool MERTrackKalman2D::SetSpecialParameters(double SigmaHitPos, unsigned int NLayersForVertexSearch)
{
  m_NLayersForVertexSearch = NLayersForVertexSearch;
  m_sigma = SigmaHitPos;

  if (m_NLayersForVertexSearch < 4) {
    merr<<"Error: Revan (tracking): NLayersForVertexSearch (="<<m_NLayersForVertexSearch<<") must be >= 4. Setting it to 4."<<show;
    m_NLayersForVertexSearch = 4;
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


MERTrackKalman2D::~MERTrackKalman2D()
{
  // Delete this instance of MERTrackKalman2D
}


////////////////////////////////////////////////////////////////////////////////


/*bool MERTrackKalman2D::EvaluateTracks(MRERawEvent* RE)
{
  // Evaluate the given tracks...

  if (RE->GetVertex() != 0) {
    return EvaluatePairs(RE);
  }

  // Mark: Add you evaluations here:
  mout<<"MRawEventIncarnationList* MERTrackKalman2D::EvaluateTracks(MRERawEvent* ER): nyi!"<<endl;
  //return MERTrack::EvaluateTracks(RE);

  // Store your results:
  RE->SetTrackQualityFactor(MRERawEvent::c_NoQualityFactor);
  RE->SetPairQualityFactor(MRERawEvent::c_NoQualityFactor);

  return true;
}*/


////////////////////////////////////////////////////////////////////////////////


MString MERTrackKalman2D::ToString(bool CoreOnly) const
{
  // Dump an options string gor the tra file:

  ostringstream out;

  if (CoreOnly == false) {
    out<<"# Tracking - Kalman options:"<<endl;
    out<<"# "<<endl;
  }
  out<<MERTrack::ToString(true);
  if (CoreOnly == false) {
    out<<"# "<<endl;
  }

  return out.str().c_str();
}

////////////////////////////////////////////////////////////////////////////////

Float_t MERTrackKalman2D::Kalman(MRERawEvent* RE, Int_t n, Float_t en, MRESEList Previous, vector < TMatrix > &trk, Int_t view, MRESEList &Chosen)
{
  vector < TMatrix > Ck;
  vector < TMatrix > Csmoothk;
  vector < TMatrix > Fk;
  vector < TMatrix > FTk;
  vector < TMatrix > Cprojk;
  vector < TMatrix > CprojInvk;
  vector < TMatrix > Qk;
  vector < TMatrix > xk;
  vector < TMatrix > xprojk;
  vector < TMatrix > xsmoothk;

  Float_t _planedistance = 0.;
  Float_t m = 1000.;

  TMatrix C(2,2);
  TMatrix C_proj(2,2);
  Cprojk.push_back(C_proj);
  CprojInvk.push_back(C_proj);
  TMatrix F(2,2);
  TMatrix Q(2,2);

  Chosen.RemoveAllRESEs();
  Chosen.Compress();

  Float_t chi2 = 0.;
  MRESE vertex = RE->GetVertex();
  Chosen.AddRESE(RE->GetVertex());

  Float_t height = 2*vertex.GetVolumeSequence()->GetDetector()->GetStructuralSize().GetZ();
  m_heightX0 = height/vertex.GetVolumeSequence()->GetDetector()->GetSensitiveVolume(0)->GetMaterial()->GetRadiationLength();

  //Filtering
  Float_t t; // tan of the RMS of the scattering angle

  Float_t V = (m_sigma*m_sigma);  //Weight Matrix

  Float_t G = 1./(m_sigma*m_sigma); //(Weight Matrix)^-1

  C(0,0)=m_sigma*m_sigma;
  C(0,1)=0.;
  C(1,0)=0.;
  C(1,1)=w;

  Ck.push_back(C);

  vector < Float_t > mk;

  TMatrix x(2,1);
  if(view==0)  x(0,0)=vertex.GetPositionX();
  else if(view==1)  x(0,0)=vertex.GetPositionY();
  x(1,0)=0;

  xk.push_back(x);

  mk.push_back(x(0,0));

  TMatrix x_proj(2,1);
  xprojk.push_back(x_proj);

  Int_t i=0;

  MRESEList List;
  List.RemoveAllRESEs();
  List.Compress();

  // Create a list of RESEs sorted by depth in tracker
  vector<MRESE*> ReseList;
  for (int h = 0; h < RE->GetNRESEs(); h++) {
    if (m_Geometry->IsAbove(RE->GetVertex(), RE->GetRESEAt(h)) == false) {
      ReseList.push_back(RE->GetRESEAt(h));
    }
  }
  sort(ReseList.begin(), ReseList.end(), CompareRESEByZ());

  vector<MRESE*>::iterator Iterator1;
  for (Iterator1 = ReseList.begin(); Iterator1 != ReseList.end(); Iterator1++) {
    if (IsInTracker(*Iterator1) == false) continue;
    List.AddRESE(*Iterator1);
  }
  ReseList.clear();

  Float_t PlaneZ = vertex.GetPosition().Z();
  mdebug<<"Vertex : "<<vertex.GetPosition().X()<<" "<<vertex.GetPosition().Y()<<" "<<vertex.GetPosition().Z()<<endl;
  Float_t LayerDistance = vertex.GetVolumeSequence()->GetDetector()->GetStructuralPitch().GetZ();

  for (int r = 0; r < List.GetNRESEs() && i<n-1; r++) {

    if (PlaneZ <= (List.GetRESEAt(r)->GetPosition().Z() + 0.0001)) continue;
    PlaneZ = List.GetRESEAt(r)->GetPosition().Z();

    for (int k = 0; k < r; k++) {
      if(List.GetRESEAt(k)->GetPosition().Z()-PlaneZ < 0.0001) break;
      _planedistance = List.GetRESEAt(k)->GetPosition().Z()-PlaneZ;
    }

    if(_planedistance>1.5*LayerDistance) break; // we accept only hits on subsequent planes

    t=(13.6/en)*sqrt(m_heightX0)*(1+0.038*log(m_heightX0));

    F(0,0)=1.;
    F(0,1)=_planedistance;
    F(1,0)=0.;
    F(1,1)=1.;

    Fk.push_back(F);

    Q(0,0)=(t*t*height*height)/3.;
    Q(0,1)=t*t*height/2.;
    Q(1,0)=t*t*height/2.;
    Q(1,1)=t*t;
    Qk.push_back(Q);

    TMatrix H(2,1);
    H(0,0)=1;
    H(1,0)=0;

    x_proj = (F*x);

    xprojk.push_back(x_proj);

    TMatrix FT = F;
    FT.T();
    FTk.push_back(FT);

    C_proj = (F*C)*FT + Q;

    Cprojk.push_back(C_proj);

    TMatrix C_projInv=C_proj;

    C_projInv.Invert();

    CprojInvk.push_back(C_projInv);

    C = C_projInv;
    C(0,0)+=G;
    C.Invert();

    Ck.push_back(C);
    //Fk.push_back(F);
    Qk.push_back(Q);

    vector < Float_t > weight;

    for (int k = 0; k < List.GetNRESEs(); k++) {

      if(abs(List.GetRESEAt(k)->GetPosition().Z()-PlaneZ) > 0.0001) continue;

      Bool_t previous = false;

      for (int j = 0; j < Previous.GetNRESEs(); j++) {
        if(abs(Previous.GetRESEAt(j)->GetPosition().Z()-PlaneZ)  > 0.0001) continue;
        if(view==0){
          if(Previous.GetRESEAt(j)->GetPosition().X()-List.GetRESEAt(k)->GetPosition().X() ==0) previous = true;
        } else if(view==1){
          if(Previous.GetRESEAt(j)->GetPosition().Y()-List.GetRESEAt(k)->GetPosition().Y() ==0) previous = true;
        }
      }

      if(view==0){
        if (previous) weight.push_back(sfg_factor+abs(List.GetRESEAt(k)->GetPosition().X()-x_proj(0,0))/(sqrt(C_proj(0,0))));
        else weight.push_back(abs(List.GetRESEAt(k)->GetPosition().X()-x_proj(0,0))/(sqrt(C_proj(0,0))));
      } else if(view==1){
        if (previous) weight.push_back(sfg_factor+abs(List.GetRESEAt(k)->GetPosition().Y()-x_proj(0,0))/(sqrt(C_proj(0,0))));
        else weight.push_back(abs(List.GetRESEAt(k)->GetPosition().Y()-x_proj(0,0))/(sqrt(C_proj(0,0))));
      }

    }
    Int_t kmin=0;
    for(int kk=0; kk<(int) weight.size(); kk++){
      if(weight[kk]==*min_element(weight.begin(), weight.end())) kmin=kk;
    }

    Int_t kind=0;
    for (int kk = 0; kk < List.GetNRESEs(); kk++) {
      if(abs(List.GetRESEAt(kk)->GetPosition().Z()-PlaneZ) > 0.0001) continue;
      else {
        if (kind==kmin){
          if(view==0){
            m=List.GetRESEAt(kk)->GetPosition().X();
          } else if(view==1){
            m=List.GetRESEAt(kk)->GetPosition().Y();
          }

          Chosen.AddRESE(List.GetRESEAt(kk));
          break;
        }
        kind ++;
      }
    }

    mk.push_back(m);
    x = C * (C_projInv*x_proj+(H*(G*m)));
    xk.push_back(x);
    i++;
  }

  if ((Int_t) xk.size()<n) n = xk.size();
  if (n<2) return chi2;

  //Smoothing

  TMatrix xsmooth(2,1);

  xsmoothk.resize(xk.size(), xsmooth);
  xsmoothk[xsmoothk.size()-1] = xk[xk.size()-1];
  trk.push_back(xsmoothk[xsmoothk.size()-1]);

  TMatrix Csmooth(2,2);
  Csmoothk.resize(Ck.size(), Csmooth);
  Csmoothk[Ck.size()-1] = C;

  for (Int_t i = n-2; i >= 0; i--) {
    TMatrix A(2,2);

    A = Ck[i]*FTk[i]*CprojInvk[i+1];

    TMatrix AT = A;
    AT.T();

    xsmoothk[i] = xk[i] + A * (xsmoothk[i+1] - xprojk[i+1]);

    trk.push_back(xsmoothk[i]);

    Csmooth = Ck[i] + A*(Csmoothk[i+1] - Cprojk[i+1])*AT;
    Csmoothk[i] = Csmooth;
    Float_t rk = mk[i] - xsmoothk[i](0,0);
    Float_t Rk = V -  Csmoothk[i](0,0);

    chi2 += rk * rk / Rk;
  }
  reverse(trk.begin(),trk.end());
  return chi2;
}

tuple<vector < TMatrix >, Float_t, Float_t, vector < TMatrix >, Float_t, Float_t, MRESEList, MRESEList> MERTrackKalman2D::SearchTracks(MRERawEvent* RE, Int_t view)
{
  MRESEList PrevChoice;
  Float_t en = start_nrg_fit;
  Float_t nrg_est1;
  Float_t nrg_est2;
  Float_t chi1=0.;
  Float_t chi2=0.;
  Int_t loop1=0;
  Bool_t stop=false;
  Int_t n = n_fit;

  MRESEList Chosen1;
  MRESEList Chosen2;

  vector < TMatrix > trk1;
  vector < TMatrix > trk2;
  mdebug<<"First track "<<endl;
  PrevChoice.RemoveAllRESEs();
  PrevChoice.Compress();

  //track 1
  while(!stop){

    chi1 = Kalman(RE, n, en, PrevChoice, trk1, view, Chosen1);

    if (trk1.size()<2){
      return std::make_tuple(trk1, 0, 0, trk2, 0, 0, Chosen1, Chosen2);
    }

    nrg_est1=0.;

    //Energy Estimation
    nrg_est1 = MultipleScattering2D(trk1);

    if (nrg_est1> 500000.) nrg_est1=500000.;
    if ((nrg_est1/en)<1/3){
      //loop1++;
      en=en/3.;
      trk1.clear();
    } else if ((nrg_est1/en)>3. && loop1<9){
      loop1++;
      en=en*3.;
      trk1.clear();
    } else stop=true;
  }

  for (size_t i = 0; i < trk1.size(); i++) {
    mdebug<<"Track1 after Kalman "<<trk1.size()<<" "<<trk1[i](0,0)<<" "<<trk1[i](1,0)<<endl;
  }

  stop=false;
  en = start_nrg_fit;
  Int_t loop2=0;
  mdebug<<"Second track "<<endl;
  //track 2
  while(!stop){
    Chosen2.RemoveAllRESEs();
    Chosen2.Compress();

    chi2 = Kalman(RE, n, en, Chosen1, trk2, view, Chosen2);

    if (trk2.size()<2){
      return std::make_tuple(trk1, 0, 0, trk2, 0, 0, Chosen1, Chosen2);
    }

    nrg_est2=0;

    //Energy Estimation
    nrg_est2 = MultipleScattering2D(trk2);

    if (nrg_est2> 500000.) nrg_est2=500000.;
    if ((nrg_est2/en)<1/3){
      //loop1++;
      en=en/3.;
      trk2.clear();
    } else if ((nrg_est2/en)>3. && loop2<9){
      loop2++;
      en=en*3.;
      trk2.clear();
    } else stop=true;
  }
  for (size_t i = 0; i < trk2.size(); i++) {
    mdebug<<"Track2 after Kalman "<<trk2.size()<<" "<<trk2[i](0,0)<<" "<<trk2[i](1,0)<<endl;
  }
  return std::make_tuple(trk1, nrg_est1, chi1, trk2, nrg_est2, chi2, Chosen1, Chosen2);
}

Float_t MERTrackKalman2D::MultipleScattering2D(vector < TMatrix > trk)
{
  vector < Float_t > angles;
  UInt_t i;

  for (i = 0; i < trk.size(); i++) {
    angles.push_back(TMath::ATan( trk[i](1,0)));
  }

  vector < Float_t > dtheta1;
  vector < Float_t > absdtheta1;
  Float_t dt1;
  for (i = 1; i < angles.size(); i++) {
    dtheta1.push_back((Float_t) (angles[i] - angles[i - 1]));
  }

  dtheta1.erase(dtheta1.end() - 1, dtheta1.end());

  Float_t   heightX0tot = 0;
  vector < Float_t > planes;
  Float_t meanX0 = 0;

  for(i=0; i< dtheta1.size(); i++){
    heightX0tot +=  m_heightX0;
    dtheta1[i]/= exp(m_heightX0/cos(atan2(trk[1](1,0),1)));    //Energy correction
  }

  bool stop = false;
  meanX0 = heightX0tot/dtheta1.size();

  while (!stop) {
    Float_t maxdt =0.;
    dt1 = 0.;
    absdtheta1.clear();

    for (i = 0; i < dtheta1.size(); i++) {
      dt1 += dtheta1[i] * dtheta1[i];
      absdtheta1.push_back(TMath::Abs( dtheta1[i]));
      if(TMath::Abs( dtheta1[i])>maxdt) maxdt=TMath::Abs( dtheta1[i]);
    }
    dt1 = sqrt((dt1 / dtheta1.size()));

    if (maxdt / dt1 > 3.) {
      for (i = 0; i < absdtheta1.size(); i++) {

        if (absdtheta1[i] == maxdt) {
          absdtheta1.erase(absdtheta1.begin() + i);
          dtheta1.erase(dtheta1.begin() + i);
          i--;
        }
      }
    } else {
      stop = true;
    }
  }

  Float_t energy=13.6*sqrt(meanX0)*(1.+0.038*log(meanX0))/(dt1*sqrt(cos(atan2(trk[1](1,0), 1.))));
  dtheta1.clear();
  absdtheta1.clear();
  angles.clear();
  return energy;

}   //MultipleScattering2D

Float_t MERTrackKalman2D::MultipleScattering3D(MRESEList trkx, MRESEList trky)
{
  Int_t n = min((trkx.GetNRESEs()),(trky.GetNRESEs()));
  Float_t vi[3];
  TVector3 vi0;
  TVector3 vi1;

  vector < Float_t > theta(n);

  Float_t E = 0;

  for (Int_t i = 0; i < n-1; i++) {

    vi[0] = trkx.GetRESEAt(i+1)->GetPosition().X() - trkx.GetRESEAt(i)->GetPosition().X();
    vi[1] = trky.GetRESEAt(i+1)->GetPosition().Y() - trky.GetRESEAt(i)->GetPosition().Y();
    vi[2]= trkx.GetRESEAt(i+1)->GetPosition().Z() - trkx.GetRESEAt(i)->GetPosition().Z();
    vi0.SetXYZ(vi[0], vi[1], vi[2]);
    vi0.SetMag(1.);

    theta[i] = vi0.Theta();

    if (i != 0) {
      Float_t angle = vi0.Angle(vi1);
      E = (13.6/angle)*sqrt(m_heightX0/cos(TMath::Pi()-vi0.Theta()))*(1+0.038*log(m_heightX0/cos(TMath::Pi()-vi0.Theta())));

      break;
    }
    vi1=vi0;
  }
  return E;
}

void MERTrackKalman2D::TrackPairs(MRERawEvent* RE)
{
  if (RE->GetVertex() == 0) {
    Fatal("void MRERawEvent::TrackPairs()",
    "No vertex found!");
  }

  Float_t E1x=0.;
  Float_t E2x=0.;
  Float_t ChiT1x=0.;
  Float_t ChiT2x=0.;

  vector < TMatrix > Track1x;
  vector < TMatrix > Track2x;

  Float_t E1y=0.;
  Float_t E2y=0.;
  Float_t ChiT1y=0.;
  Float_t ChiT2y=0.;

  Float_t E1=0.;
  Float_t E2=0.;
  Float_t Egamma=0.;

  vector < TMatrix > Track1y;
  vector < TMatrix > Track2y;

  MRESEList Chosen1x;
  MRESEList Chosen2x;

  MRESEList Chosen1y;
  MRESEList Chosen2y;

  MRETrack* Electron = new MRETrack();
  MRETrack* Positron = new MRETrack();

  // Now create the vertex:
  Electron->AddRESE(RE->GetVertex());
  Electron->SetStartPoint(RE->GetVertex());
  Positron->AddRESE(RE->GetVertex());
  Positron->SetStartPoint(RE->GetVertex());

  // View X
  tie(Track1x, E1x, ChiT1x, Track2x, E2x, ChiT2x, Chosen1x, Chosen2x) = SearchTracks(RE, 0);
  if (ChiT1x==0. && ChiT2x==0.) return;

  //View Y
  tie(Track1y, E1y, ChiT1y, Track2y, E2y, ChiT2y, Chosen1y, Chosen2y) = SearchTracks(RE, 1);

  if (ChiT1y==0. && ChiT2y==0.) return;

  if((E1x-E2x)*(E1y-E2y)<0.){
    Float_t E1ytemp=E1y;
    E1y=E2y;
    E2y=E1ytemp;

    Float_t ChiT1ytemp=ChiT1y;
    ChiT1y=ChiT2y;
    ChiT2y=ChiT1ytemp;

    Track1y.swap(Track2y);

    MRESEList Chosen1ytemp(Chosen1y);
    Chosen1y=Chosen2y;
    Chosen2y=Chosen1ytemp;
  }

  if (Chosen1x.GetNRESEs()>2 && Chosen1y.GetNRESEs()>2){
    E1= MultipleScattering3D(Chosen1x, Chosen1y);
    if (Chosen2x.GetNRESEs()>2 && Chosen2y.GetNRESEs()>2){
      E2= MultipleScattering3D(Chosen2x, Chosen2y);
    } else E2= 0.;
    Egamma= E1+E2;

    for (int r = 0; r < Chosen1x.GetNRESEs(); r++) {
      Electron->AddRESE(Chosen1x.GetRESEAt(r));
    }

    for (int r = 0; r < Chosen1y.GetNRESEs(); r++) {
      Electron->AddRESE(Chosen1y.GetRESEAt(r));
    }

    for (int r = 0; r < Chosen2x.GetNRESEs(); r++) {
      Positron->AddRESE(Chosen2x.GetRESEAt(r));
    }

    for (int r = 0; r < Chosen2y.GetNRESEs(); r++) {
      Positron->AddRESE(Chosen2y.GetRESEAt(r));
    }

    Float_t thetax1=Track1x[0](1,0);
    Float_t thetay1=Track1y[0](1,0);

    Float_t thetax2=Track2x[0](1,0);
    Float_t thetay2=Track2y[0](1,0);

    mdebug<<"Theta of the four tracks: "<<c_Deg*atan(thetax1)<<" "<<c_Deg*atan(thetay1)<<" "<<c_Deg*atan(thetax2)<<" "<<c_Deg*atan(thetay2)<<" "<<endl;

    Float_t x1= thetax1/sqrt(pow(thetax1,2.)+pow(thetay1,2.)+1);
    Float_t y1= thetay1/sqrt(pow(thetax1,2.)+pow(thetay1,2.)+1);
    Float_t z1= 1./sqrt(pow(thetax1,2.)+pow(thetay1,2.)+1);

    Float_t x2= thetax2/sqrt(pow(thetax2,2.)+pow(thetay2,2.)+1);
    Float_t y2= thetay2/sqrt(pow(thetax2,2.)+pow(thetay2,2.)+1);
    Float_t z2= 1./sqrt(pow(thetax2,2.)+pow(thetay2,2.)+1);

    Float_t x= x1*E1+x2*E2/Egamma;
    Float_t y= y1*E1+y2*E2/Egamma;
    Float_t z= z1*E1+z2*E2/Egamma;

    Float_t mag=sqrt(x*x+y*y+z*z);
    Float_t mag1=sqrt(x1*x1+y1*y1+z1*z1);
    Float_t mag2=sqrt(x2*x2+y2*y2+z2*z2);


    MVector direction(x/mag,y/mag,-z/mag);
    MVector direction1(x1/mag1,y1/mag1,-z1/mag1);
    MVector direction2(x2/mag2,y2/mag2,-z2/mag2);

    Electron->SetFixedDirection(direction1);
    Positron->SetFixedDirection(direction2);

    Electron->SetEnergy(E1*1000); //So far the energy was calculated in MeV, MEGAlib works in keV
    Positron->SetEnergy(E2*1000);

    Electron->SetQualityFactor(ChiT1x+ChiT2x);
    Positron->SetQualityFactor(ChiT1y+ChiT2y);

    mdebug<<"Energy of the two particles: "<<E1<<" "<<E2<<endl;
    mdebug<<"Reconstructed theta, phi and energy of the gamma: "<<c_Deg*direction.Theta()<<" "<<c_Deg*direction.Phi()<<" "<<Egamma<<endl;
    RE->SetElectronTrack(Electron);
    RE->SetPositronTrack(Positron);

    //RE->SetTrackQualityFactor((ChiT1y+ChiT2y)/100);
    RE->SetPairQualityFactor((ChiT1y+ChiT2y));
    RE->SetGoodEvent(true);
    //RE->SetEventType(c_PairEvent);
    RE->SetEventReconstructed(true);

  } else{
    Egamma= -1.;
    E1= 1.;
    E2= 1.;
  }

}
// MERTrackKalman2D.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
