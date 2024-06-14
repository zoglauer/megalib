/*
 * MFileEventsRoot.h
 *
 * Write data in root files for debugging/diagnostic purposes
 *
 */

#ifndef __MFileEventsRoot__
#define __MFileEventsRoot__

////////////////////////////////////////////////////////////////////////////////


// Standard libs:
//#include <list>
//using namespace std;

// ROOT libs:
#include "TFile.h"
#include "TTree.h"

// MEGAlib libs:
#include "MGlobal.h"
#include "MFile.h"
#include "MPhysicalEvent.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////

class MFileEventsRoot //: public MFileEvents <- Probably not a good idea to inherit from clearly text-oriented file classes
// So far only supports mono-threaded revan execution.
{
  // public interface:
 public:
  MFileEventsRoot(TString FileName, unsigned int Way = MFile::c_Create);
  virtual ~MFileEventsRoot();

  //! The Open method has to be derived to initialize the include file:
//  virtual bool Open(TString FileName, unsigned int Way = MFile::c_Create);

  //! Close the file and kill the thread (if it (still) exists)
  virtual bool Close();

  bool AddEvent(MPhysicalEvent* P);
  bool AddHeader();
  bool AddFooter();

  // protected methods:
 protected:
  void createMerit();
  void setMeritAddress();

  void createHeader();
  void setHeaderAddress();

  void createFooter();
  void setFooterAddress();

  // private methods:
 private:

  // public members:
 public:
  // Free access to root trees ?
  TTree* m_header;
  TTree* m_footer;
  TTree* m_merit;
  // Root tree members - header
  int m_version;
  char m_geometry[1024];
  // Root tree members - footer
  char Date[1024];
  char OriginalFile[1024];
  char GeometryFile[1024];
  char HitClusterizer[1024];
  char Tracker[1024];
  char CSR[1024];
  char Decay[1024];
  char ERNoising[1024];
  int NEvents, NPassedEventSelection, NGoodEvents, NPhotoEvents, NComptonEvents, NDecayEvents, NPairEvents, NMuonEvents, NPETEvents, NMultiEvents, RejectedEvents;
  // Root tree members - merit
  double StartX, StartY, StartZ, StartDirX, StartDirY, StartDirZ, StartPolX, StartPolY, StartPolZ, StartEnergy;
  long EvtId;
  int EvtType;
  unsigned int EvtNHits;
  double EvtEnergy;
  int EvtTimeWalk;
  double EvtPosX, EvtPosY, EvtPosZ, EvtDirX, EvtDirY, EvtDirZ;

  // protected members:
 protected:
  TString m_FileName;
  TFile* m_file;
  unsigned int m_Way;

  // private members:
 private:

};


#endif

