#ifndef MGUICADConverter_H
#define MGUICADConverter_H

#include <TGFrame.h>
#include <TString.h>
#include <string>
#include <vector>

class TGTextButton;
class TGCheckButton;
class TGLabel;
class TGNumberEntry;
class TGTextEntry;
class TGTextView;
class TTimer;
class MGUIGeomegaMain;

//! Run the standalone STEP converter without blocking ROOT's GUI event loop.
class MGUICADConverter : public TGTransientFrame
{
public:
  MGUICADConverter(const TGWindow* Parent, MGUIGeomegaMain* Main,
                   UInt_t Width = 720, UInt_t Height = 620);
  ~MGUICADConverter() override;

  // Also usable from ROOT scripts (the same validation as the file dialogs).
  bool SetInputFile(const TString& FileName);
  bool SetOutputDirectory(const TString& Directory);
  bool IsConverting() const { return fChildPid > 0; }
  const TString& GetOutputFilePath() const { return fOutputFilePath; }
  const TString& GetLogFilePath() const { return fLogFilePath; }

  void OnSelectFile();
  void OnSelectDest();
  void OnConvert();
  void OnCancel();
  void OnViewGeomega();
  void CloseWindow() override;
  Bool_t HandleTimer(TTimer* Timer) override;
  Bool_t ProcessMessage(Long_t Message, Long_t Parameter1, Long_t Parameter2) override;

private:
  void SetStatus(const TString& Status);
  void SetBusy(bool Busy);
  void ReadLog(bool Finished = false);
  TString FindConverter() const;

  MGUIGeomegaMain* fMain; //!
  TGTextButton* fBtnSelectFile;
  TGTextButton* fBtnDestBrowse;
  TGTextButton* fBtnConvert;
  TGTextButton* fBtnCancel;
  TGTextButton* fBtnViewGeomega;
  TGCheckButton* fCheckGeo;
  TGCheckButton* fCheckObj;
  TGCheckButton* fCheckStl;
  TGNumberEntry* fNumAngDef;
  TGNumberEntry* fNumMaxDev;
  TGNumberEntry* fNumMinVol;
  TGTextEntry* fSourcePath;
  TGTextEntry* fDestPath;
  TGLabel* fStatusLabel;
  TGTextView* fLogView;
  TTimer* fTimer; //!

  TString fInputFilePath;
  TString fOutputDir;
  TString fOutputFilePath;
  TString fLogFilePath;
  std::vector<std::string> fExpectedOutputs;
  Long64_t fLogOffset = 0;
  std::string fPendingLine;
  int fChildPid = -1;
  int fCancelTicks = 0;
  bool fCloseWhenFinished = false;
  bool fHadWarnings = false;

#ifdef ___CLING___
  ClassDefOverride(MGUICADConverter, 0)
#endif
};

#endif
