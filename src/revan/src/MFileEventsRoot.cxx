/*
 * MFileEventsRoot.cxx
 *
 *
 *
 */


////////////////////////////////////////////////////////////////////////////////
//
// MFileEventsRoot
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MFileEventsRoot.h"

// Standard libs:

// ROOT libs:
#include "TFile.h"
#include "TTree.h"

// MEGAlib libs:
#include "MGlobal.h"
#include "MFile.h"
#include "MPhysicalEvent.h"


////////////////////////////////////////////////////////////////////////////////


//#ifdef ___CLING___
//ClassImp(MFileEventsTra)
//#endif


////////////////////////////////////////////////////////////////////////////////

MFileEventsRoot::MFileEventsRoot(TString FileName, unsigned int Way) //: MFileEvents()
{
  m_Way = MFile::c_Create;
  m_FileName = FileName+".root";
  if(m_Way == MFile::c_Read) {
    m_file = TFile::Open(FileName);
    m_header = (TTree*)m_file->Get("Header");
    setHeaderAddress();
    m_footer = (TTree*)m_file->Get("Footer");
    setFooterAddress();
    m_merit = (TTree*)m_file->Get("Merit");
    setMeritAddress();
  } else {
    m_file = TFile::Open(m_FileName, "recreate");
    m_header = new TTree("Header", "Header");
    createHeader();
    m_footer = new TTree("Footer", "Footer");
    createFooter();
    m_merit = new TTree("Merit", "Merit");
    createMerit();
  }
}


MFileEventsRoot::~MFileEventsRoot()
{
  Close();
}

/*bool MFileEventsRoot::Open(TString FileName, unsigned int Way) {
  if(m_file) {
    mout<<"Error while opening file "<<FileName<<".root: "<<endl;
    mout<<"A file is already opened!"<<endl;
    return false;
  }
  m_Way = Way;
  m_FileName = FileName+".root";
  if(m_Way == MFile::c_Read) {
    m_file = TFile::Open(FileName);
    m_header = (TTree*)m_file->Get("Header");
    m_footer = (TTree*)m_file->Get("Footer");
    m_merit = (TTree*)m_file->Get("Merit");
  } else {
    m_file = TFile::Open(m_FileName, "recreate");
    //m_file->Close();
    m_header = new TTree("Header", "Header");//memory resident ? For test for now
    createHeader();
    m_footer = new TTree("Footer", "Footer");
    createFooter();
    //m_merit = new TTree("Merit", "Merit");
    //createMerit();
  }
  return true;
}*/

bool MFileEventsRoot::Close() {
  if(m_file) {
    if (m_Way != MFile::c_Read) {
      m_file->cd();
      if (m_header) { m_header->Write(); }
      if (m_footer) { m_footer->Write(); }
      if (m_merit) {  m_merit->Write();  }
      m_file->Write();
    }
    m_file->Close();
  }
  return true;
}

bool MFileEventsRoot::AddEvent(MPhysicalEvent* P) {
  // Get information from P
  MVector oipos, oidir, oipol, evtpos, evtdir;
  oipos = P->GetOIPosition();
  oidir = P->GetOIDirection();
  oipol = P->GetOIPolarization();
  evtpos = P->GetPosition();
  evtdir = P->GetOrigin();
  if (oipos != g_VectorNotDefined && oidir != g_VectorNotDefined && oipol != g_VectorNotDefined) {
    StartX = oipos.GetX();
    StartY = oipos.GetY();
    StartZ = oipos.GetZ();
    StartDirX = oidir.GetX();
    StartDirY = oidir.GetY();
    StartDirZ = oidir.GetZ();
    StartPolX = oidir.GetX();
    StartPolY = oidir.GetY();
    StartPolZ = oidir.GetZ();
    StartEnergy = P->GetOIEnergy();
  }
  EvtId = P->GetId();
  EvtType = P->GetType();
  EvtNHits = P->GetNHits();
  EvtEnergy = P->GetEnergy();
  EvtTimeWalk = P->GetTimeWalk();
  EvtPosX = evtpos.GetX();
  EvtPosY = evtpos.GetY();
  EvtPosZ = evtpos.GetZ();
  EvtDirX = evtdir.GetX();
  EvtDirY = evtdir.GetY();
  EvtDirZ = evtdir.GetZ();
  // Fill merit with one event
  m_merit->Fill();
  return true;
}

bool MFileEventsRoot::AddHeader() {
  // Fill Header
  m_header->Fill();
  return true;
}

bool MFileEventsRoot::AddFooter() {
  // Fill footer
  m_footer->Fill();
  return true;
}

void MFileEventsRoot::createMerit() {
  m_merit->Branch("StartX", &StartX, "StartX/D");
  m_merit->Branch("StartY", &StartY, "StartY/D");
  m_merit->Branch("StartZ", &StartZ, "StartZ/D");
  m_merit->Branch("StartDirX", &StartDirX, "StartDirX/D");
  m_merit->Branch("StartDirY", &StartDirY, "StartDirY/D");
  m_merit->Branch("StartDirZ", &StartDirZ, "StartDirZ/D");
  m_merit->Branch("StartPolX", &StartPolX, "StartPolX/D");
  m_merit->Branch("StartPolY", &StartPolY, "StartPolY/D");
  m_merit->Branch("StartPolZ", &StartPolZ, "StartPolZ/D");
  m_merit->Branch("StartEnergy", &StartEnergy, "StartEnergy/D");
  m_merit->Branch("EvtId", &EvtId, "EvtId/L");
  m_merit->Branch("EvtType", &EvtType, "EvtType/I");
  m_merit->Branch("EvtNHits", &EvtNHits, "EvtNHits/i");
  m_merit->Branch("EvtEnergy", &EvtEnergy, "EvtEnergy/D");
  m_merit->Branch("EvtTimeWalk", &EvtTimeWalk, "EvtTimeWalk/I");
  m_merit->Branch("EvtPosX", &EvtPosX, "EvtPosX/D");
  m_merit->Branch("EvtPosY", &EvtPosY, "EvtPosY/D");
  m_merit->Branch("EvtPosZ", &EvtPosZ, "EvtPosZ/D");
  m_merit->Branch("EvtDirX", &EvtDirX, "EvtDirX/D");
  m_merit->Branch("EvtDirY", &EvtDirY, "EvtDirY/D");
  m_merit->Branch("EvtDirZ", &EvtDirZ, "EvtDirZ/D");
}
void MFileEventsRoot::setMeritAddress() {
  m_merit->SetBranchAddress("StartX", &StartX);
  m_merit->SetBranchAddress("StartY", &StartY);
  m_merit->SetBranchAddress("StartZ", &StartZ);
  m_merit->SetBranchAddress("StartDirX", &StartDirX);
  m_merit->SetBranchAddress("StartDirY", &StartDirY);
  m_merit->SetBranchAddress("StartDirZ", &StartDirZ);
  m_merit->SetBranchAddress("StartPolX", &StartPolX);
  m_merit->SetBranchAddress("StartPolY", &StartPolY);
  m_merit->SetBranchAddress("StartPolZ", &StartPolZ);
  m_merit->SetBranchAddress("StartEnergy", &StartEnergy);
  m_merit->SetBranchAddress("EvtId", &EvtId);
  m_merit->SetBranchAddress("EvtType", &EvtType);
  m_merit->SetBranchAddress("EvtNHits", &EvtNHits);
  m_merit->SetBranchAddress("EvtEnergy", &EvtEnergy);
  m_merit->SetBranchAddress("EvtTimeWalk", &EvtTimeWalk);
  m_merit->SetBranchAddress("EvtPosX", &EvtPosX);
  m_merit->SetBranchAddress("EvtPosY", &EvtPosY);
  m_merit->SetBranchAddress("EvtPosZ", &EvtPosZ);
  m_merit->SetBranchAddress("EvtDirX", &EvtDirX);
  m_merit->SetBranchAddress("EvtDirY", &EvtDirY);
  m_merit->SetBranchAddress("EvtDirZ", &EvtDirZ);
}


void MFileEventsRoot::createHeader() {
  m_header->Branch("Version", &m_version, "&m_version/I");
  m_header->Branch("Geometry", m_geometry, "m_geometry[1024]/C");
}
void MFileEventsRoot::setHeaderAddress() {
  m_header->SetBranchAddress("Version", &m_version);
  m_header->SetBranchAddress("Geometry", m_geometry);
}


void MFileEventsRoot::createFooter() {
  m_footer->Branch("Date", Date, "Date[1024]/C");
  m_footer->Branch("OriginalFile", OriginalFile, "OriginalFile[1024]/C");
  m_footer->Branch("GeometryFile", GeometryFile, "GeometryFile[1024]/C");
  m_footer->Branch("HitClusterizer", HitClusterizer, "HitClusterizer[1024]/C");
  m_footer->Branch("Tracker", Tracker, "Tracker[1024]/C");
  m_footer->Branch("CSR", CSR, "CSR[1024]/C");
  m_footer->Branch("Decay", Decay, "Decay[1024]/C");
  m_footer->Branch("ERNoising", ERNoising, "ERNoising[1024]/C");
  m_footer->Branch("NEvents", &NEvents, "NEvents/I");
  m_footer->Branch("NPassedEventSelection", &NPassedEventSelection, "NPassedEventSelection/I");
  m_footer->Branch("NGoodEvents", &NGoodEvents, "NGoodEvents/I");
  m_footer->Branch("NPhotoEvents", &NPhotoEvents, "NPhotoEvents/I");
  m_footer->Branch("NComptonEvents", &NComptonEvents, "NComptonEvents/I");
  m_footer->Branch("NDecayEvents", &NDecayEvents, "NDecayEvents/I");
  m_footer->Branch("NPairEvents", &NPairEvents, "NPairEvents/I");
  m_footer->Branch("NMuonEvents", &NMuonEvents, "NMuonEvents/I");
  m_footer->Branch("NPETEvents", &NPETEvents, "NPETEvents/I");
  m_footer->Branch("NMultiEvents", &NMultiEvents, "NMultiEvents/I");
  m_footer->Branch("RejectedEvents", &RejectedEvents, "RejectedEvents/I");
}
void MFileEventsRoot::setFooterAddress() {
  m_footer->SetBranchAddress("Date", Date);
  m_footer->SetBranchAddress("OriginalFile", OriginalFile);
  m_footer->SetBranchAddress("GeometryFile", GeometryFile);
  m_footer->SetBranchAddress("HitClusterizer", HitClusterizer);
  m_footer->SetBranchAddress("Tracker", Tracker);
  m_footer->SetBranchAddress("CSR", CSR);
  m_footer->SetBranchAddress("Decay", Decay);
  m_footer->SetBranchAddress("ERNoising", ERNoising);
  m_footer->SetBranchAddress("NEvents", &NEvents);
  m_footer->SetBranchAddress("NPassedEventSelection", &NPassedEventSelection);
  m_footer->SetBranchAddress("NGoodEvents", &NGoodEvents);
  m_footer->SetBranchAddress("NPhotoEvents", &NPhotoEvents);
  m_footer->SetBranchAddress("NComptonEvents", &NComptonEvents);
  m_footer->SetBranchAddress("NDecayEvents", &NDecayEvents);
  m_footer->SetBranchAddress("NPairEvents", &NPairEvents);
  m_footer->SetBranchAddress("NMuonEvents", &NMuonEvents);
  m_footer->SetBranchAddress("NPETEvents", &NPETEvents);
  m_footer->SetBranchAddress("NMultiEvents", &NMultiEvents);
  m_footer->SetBranchAddress("RejectedEvents", &RejectedEvents);
}





