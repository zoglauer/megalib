/*
 * MNeuron.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MNeuron__
#define __MNeuron__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MFile.h"

// Forward declarations:
class MSynapse;


////////////////////////////////////////////////////////////////////////////////


class MNeuron
{
  // public interface:
public:
  //! Standard constructor
  MNeuron();
  //! Standard constructor
  MNeuron(int NValues, int NErrors);
  //! Copy constructor
  MNeuron(const MNeuron& N);
  //! Standard destructor
  virtual ~MNeuron();
  
  //! Set the basic value
  virtual void Set(int NValues = 1, int NErrors = 1);
  
  virtual double GetMomentum();
  
  //! Set the current value
  virtual void SetValue(double Value, int i = 0);
  //! Return the current value
  virtual double GetValue(int i = 0);
  
  //! Set the current Error
  virtual void SetError(double Error, int i = 0);
  //! Return the current Error
  virtual double GetError(int i = 0);
  
  //! Start the action
  virtual void Run(int mode = 0) ;
  
  //! Start learning process
  virtual void Learn(int mode = 0);
  
  // Read/Write capability
  virtual bool Stream(MFile& S, const int Version, const bool Read);
  //! Parse one line of code
  virtual int ParseLine(MString Line);
  
  
  // Return the ID
  virtual unsigned int GetID() const;
  
  //! Set the input synapse
  virtual void SetInputSynapse(MSynapse* S);
  //! Set the output synapse
  virtual void SetOutputSynapse(MSynapse* S);
  
  //! The IDs are only after copy construction or loading up to date!
  virtual unsigned int GetNInputSynapseIDs() { return m_InputSynapsesIDs.size(); }
  virtual unsigned int GetNOutputSynapseIDs() { return m_InputSynapsesIDs.size(); }
  virtual unsigned int GetInputSynapseIDs(unsigned int i) { return m_InputSynapsesIDs[i]; }
  virtual unsigned int GetOutputSynapseIDs(unsigned int i) { return m_InputSynapsesIDs[i]; }
  
  //! Dump the content into a string 
  virtual MString ToString() const;
  
  
  // protected methods:
protected:
  
  // private methods:
private:
  
  
  
  // protected members:
protected:
  //! Identification number
  unsigned int m_ID;
  //! List of input Synapses
  vector<MSynapse*> m_InputSynapses;
  //! List of input Synapses IDs
  vector<unsigned int> m_InputSynapsesIDs;
  //! List of output Synapses
  vector<MSynapse*> m_OutputSynapses;
  //! List of output Synapses IDs
  vector<unsigned int> m_OutputSynapsesIDs;
  
  //! Array of values stored in this Neuron
  vector<double> m_Values;
  //! Array of error values associates with the values
  vector<double> m_Errors;
  
  //! Momentum
  double m_Momentum;
  
  
  // private members:
private:
  //! ID counter
  static unsigned int s_IDCounter;
  
  
  #ifdef ___CLING___
public:
  ClassDef(MNeuron, 0) // no description
  #endif
  
};

#endif


////////////////////////////////////////////////////////////////////////////////


