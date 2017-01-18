/*
 * MSynapse.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MSynapse__
#define __MSynapse__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MFile.h"

// Forward declarations:
class MNeuron;


////////////////////////////////////////////////////////////////////////////////


class MSynapse
{
  // public interface:
public:
  //! Standard constructor
  MSynapse(int NValues = 0);
  //! Copy constructor
  MSynapse(const MSynapse& S);
  //! Standard destructor
  virtual ~MSynapse();
  
  //!
  virtual unsigned int GetID() const { return m_ID; }
  
  //!
  virtual void SetWeight(double Weight) ;
  virtual double GetWeight(); 
  
  //! 
  virtual double GetInValue(int mode = 0);
  virtual double GetOutValue(int mode = 0); 
  
  //! 
  virtual double GetWeightedInValue(int mode = 0);
  virtual double GetWeightedOutValue(int mode = 0);
  
  //! 
  virtual double GetInError(int mode = 0);
  virtual double GetOutError(int mode = 0);
  
  //! 
  virtual double GetWeightedInError(int mode = 0);
  virtual double GetWeightedOutError(int mode = 0);
  
  //! Connect two neurons
  virtual void Connect(MNeuron* A, MNeuron* B);
  
  // Read/Write capability
  virtual bool Stream(MFile& S, const int Version, const bool Read);
  virtual int ParseLine(MString Line);
  
  //! Set the incoming Neuron
  void SetInNeuron(MNeuron* S); 
  
  //! Set the outgoing Neuron
  void SetOutNeuron(MNeuron* S); 
  
  //! Return the incoming Neuron
  MNeuron* GetInNeuron(); 
  
  //! Return the outgoing Neuron
  MNeuron* GetOutNeuron();
  
  //! Return the incoming Neuron
  unsigned int GetInNeuronID() { return m_InNeuronID; }
  
  //! Return the outgoing Neuron
  unsigned int GetOutNeuronID() { return m_OutNeuronID; }
  
  //! Dump the content into a string 
  virtual MString ToString() const;
  
  
  // protected methods:
protected:
  //MDummy() {};
  //MDummy(const MDummy& Dummy) {};
  
  // private methods:
private:
  
  
  
  // protected members:
protected:
  //! Identification number
  unsigned int m_ID;
  //! Incoming Neuron
  MNeuron* m_InNeuron;
  //!
  unsigned int m_InNeuronID;
  //! Outgoing Neuron
  MNeuron* m_OutNeuron;
  //!
  unsigned int m_OutNeuronID;
  
  //! The weighting value
  double m_Weight;
  
  //! Array of stored values
  vector<double> m_Values;
  
  
  // private members:
private:
  //! ID counter
  static unsigned int s_IDCounter;
  
  #ifdef ___CINT___
public:
  ClassDef(MSynapse, 0) // no description
  #endif
  
};

#endif


////////////////////////////////////////////////////////////////////////////////
