/*
 * MERTrackKalman3D.h
 *
 * Copyright (C) by Paolo Cumani & Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MERTrackKalman3D__
#define __MERTrackKalman3D__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MGeometryRevan.h"
#include "MERTrack.h"

#include "TTree.h"
// Forward declarations:

#include <vector>

////////////////////////////////////////////////////////////////////////////////


class MERTrackKalman3D : public MERTrack
{

  // Utility fucnciton
  static Float_t accumulate(vector<Float_t>::const_iterator start,
			    vector<Float_t>::const_iterator stop,
			    Float_t init_value);
  
  // public interface:
 public:
  MERTrackKalman3D();
  virtual ~MERTrackKalman3D();

  //! Set all special parameters - this function should not rely on a previous
  //! call to SetParameters()
  virtual bool SetSpecialParameters(double SigmaHitPos, unsigned int NLayersForVertexSearch);

  virtual MString ToString(bool CoreOnly = false) const;

  // protected methods:
 protected:

  //bool EvaluateTracks(MRERawEvent* ER);
  Float_t Kalman(MRERawEvent* RE, Int_t n, Float_t en, MRESEList Previous, vector < TMatrix > &trk, MRESEList &Chosen);
  tuple<vector < TMatrix >, Float_t, Float_t, vector < TMatrix >, Float_t, Float_t, MRESEList, MRESEList> SearchTracks(MRERawEvent* RE);
  Float_t MultipleScattering(vector < TMatrix > trk);
  void TrackPairs(MRERawEvent* RE);
  MRawEventIncarnations* CheckForPair(MRERawEvent* RE);

  //These functions are not used, they are kept for record-keeping
  Float_t EnergyEst3D(vector < TMatrix > trk);
  Float_t MultipleScattering3D(vector < TMatrix > trk, Double_t e0);

  // private methods:
 private:

  // protected members:
 protected:

  Float_t m_sigma;
  Float_t w;
  Float_t m_heightX0;
  Float_t sfg_factor;
  Float_t max_offset;

  Float_t start_nrg_fit;        //Energy used in Kalman matrix (keV)
  Int_t n_fit;

  // private members:
 private:


#ifdef ___CINT___
 public:
  ClassDef(MERTrackKalman3D, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
