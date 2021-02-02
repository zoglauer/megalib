/*
 * MERTrackKalman2D.h
 *
 * Copyright (C) by Paolo Cumani & Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MERTrackKalman2D__
#define __MERTrackKalman2D__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MGeometryRevan.h"
#include "MERTrack.h"


// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MERTrackKalman2D : public MERTrack
{
  // public interface:
 public:
  MERTrackKalman2D();
  virtual ~MERTrackKalman2D();

  //! Set all special parameters - this function should not rely on a previous
  //! call to SetParameters()
  virtual bool SetSpecialParameters(double SigmaHitPos, unsigned int NLayersForVertexSearch);

  virtual MString ToString(bool CoreOnly = false) const;

  // protected methods:
 protected:

  //bool EvaluateTracks(MRERawEvent* ER);
  Float_t Kalman(MRERawEvent* RE, Int_t n, Float_t en, MRESEList Previous, vector < TMatrix > &trk, Int_t view, MRESEList &Chosen);
  tuple<vector < TMatrix >, Float_t, Float_t, vector < TMatrix >, Float_t, Float_t, MRESEList, MRESEList> SearchTracks(MRERawEvent* RE, Int_t view);
  Float_t MultipleScattering2D(vector < TMatrix > trk);
  Float_t MultipleScattering3D(MRESEList trkx, MRESEList trky);
  void TrackPairs(MRERawEvent* RE);

  // private methods:
 private:

  // protected members:
 protected:

  Float_t m_sigma;
  Float_t w;
  Float_t m_heightX0;
  Float_t sfg_factor;

  Float_t start_nrg_fit;        //Energy used in Kalman matrix (keV)
  Int_t n_fit;

  // private members:
 private:


#ifdef ___CINT___
 public:
  ClassDef(MERTrackKalman2D, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
