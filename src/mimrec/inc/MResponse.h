/*
 * MResponse.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MResponse__
#define __MResponse__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include "TObject.h"

// MEGAlib libs:
#include "MGlobal.h"
#include "MPhysicalEvent.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MResponse
{
  // public interface:
 public:
  MResponse();
  virtual ~MResponse();

  //! Choose if you want to use maths approximations
  void SetApproximatedMaths(bool Approximated = true) { m_ApproximatedMaths = Approximated; }

  virtual void SetThreshold(const double Threshold);
  virtual double GetThreshold() const;

  virtual bool AnalyzeEvent(MPhysicalEvent* Event); 

  //! Compton response for untracked events
  virtual double GetComptonResponse(const double t);
  //! Compton response for tracked events
  virtual double GetComptonResponse(const double t, const double l);
  //! Maximum of the response
  virtual double GetComptonMaximum();
  //! Integral of the response
  virtual double GetComptonIntegral(const double Radius) const;

  virtual double GetComptonTransversalMax();
  virtual double GetComptonTransversalMin();

  //! Get pair response
  virtual double GetPairResponse(const double t);
  //! Integral of the response
  virtual double GetPairIntegral() const;
 


  // protected methods:
 protected:


  // private methods:
 private:



  // protected members:
 protected:
  //! The threshold out to which the response should be calculated
  double m_Threshold;
  //! Flag indiaction whether or not approxiamte4d maths should be used:
  bool m_ApproximatedMaths;


  // private members:
 private:



#ifdef ___CINT___
 public:
  ClassDef(MResponse, 0) // basic response class
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
