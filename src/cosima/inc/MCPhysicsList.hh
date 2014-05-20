/*
 * MCPhysicsList.hh
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */



/******************************************************************************
 *
 * Implementation of G4UserPhysicsList:
 * Cares about the initialization of particles, processes and cuts
 *
 */

#ifndef ___MCPhysicsList___
#define ___MCPhysicsList___

// Geant4:
#include "globals.hh"
#include "G4VModularPhysicsList.hh"

// Cosima:
#include "MCParameterFile.hh"
#include "MCRegion.hh"

// Standard lib:
#include <vector>
using namespace std;


/******************************************************************************/

class MCPhysicsList: public G4VModularPhysicsList
{
  // public interface:
 public:
  /// Default constructor
  explicit MCPhysicsList(MCParameterFile& RunParameters);
  /// Default destructor
  virtual ~MCPhysicsList();
  /// Construct the processes - overwritten to post-modify some processes
  virtual void ConstructProcess();
  
  
  /// Id representing using no EM package
  static const int c_EMNone; 
  /// Id representing standard EM package
  static const int c_EMStandard; 
  /// Id representing low energy Livermore package
  static const int c_EMLivermore; 
  /// Id representing low energy Livermore package with polarized Compton effects
  static const int c_EMLivermorePolarized; 
  /// Id representing low energy Livermore package with Mark Kippen's extension
  static const int c_EMLivermoreG4LECS; 
  /// Id representing low energy EM Penelope package
  static const int c_EMPenelope; 
  /// Id representing minimum EM package ID
  static const int c_EMMin; 
  /// Id representing maximum EM package ID
  static const int c_EMMax; 

  /// Id representing using no hadron package
  static const int c_HDNone; 
  /// Id representing QGSP_BIC_HP hadron package
  static const int c_HDQGSP_BIC_HP; 
  /// Id representing QGSP_BERT_HP hadron package
  static const int c_HDQGSP_BERT_HP; 
  /// Id representing QGSP_BERT_HP hadron package
  static const int c_HDFTFP_BERT_HP; 
  /// Id representing minimum hadron package ID
  static const int c_HDMin; 
  /// Id representing maximum hadron package ID
  static const int c_HDMax; 

  /// Set all energy cuts
  void SetCuts();

  // protected methods:
 protected:
  /// Register the chosen physics processes
  void Register();

  // protected members:
 protected:


  // private members:
 private:
  /// Choosen simulation package EM physics
  int m_PhysicsListEM;

  /// Choosen simulation package for hadron physics
  int m_PhysicsListHD;

  /// The regions with their individual cuts:
  const vector<MCRegion>& m_Regions; 

};

#endif


/*
 * MCPhysicsList.hh: the end...
 ******************************************************************************/







