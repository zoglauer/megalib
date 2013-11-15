/*
 * MInterfaceMimrec.cxx
 *
 *
 * Copyright (C) by Andreas Zoglauer, Mark Bandstra.
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
// MInterfaceMimrec
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MInterfaceMimrec.h"

// Standard libs:
#include <sstream>
#include <iomanip>
#include <limits>
using namespace std;

// ROOT libs:
#include "TROOT.h"
#include "TMath.h"
#include "TGMsgBox.h"
#include "TSystem.h"
#include "TLine.h"
#include "TH1.h"
#include "TH2.h"
#include "TH3.h"
#include "TVirtualFitter.h"
#include "TFitResult.h"

// MEGAlib libs:
#include "MGlobal.h"
#include "MImage2D.h"
#include "MAssert.h"
#include "MVector.h"
#include "MStreams.h"
#include "MGUIMimrecMain.h"
#include "MBPDataImage.h"
#include "MImage.h"
#include "MImageGalactic.h"
#include "MStreams.h"
#include "MEventSelector.h"
#include "MPrelude.h"
#include "MDDetector.h"
#include "MDVolumeSequence.h"
#include "MFitFunctions.h"
#include "MLMLAlgorithms.h"
#include "MProjection.h"
#include "MSpectralAnalyzer.h"
#include "MPeak.h"
#include "MIsotope.h"
#include "MStandardAnalysis.h"
#include "MResponse.h"
#include "MResponseGaussian.h"
#include "MResponseGaussianByUncertainties.h"
#include "MResponsePRM.h"
#include "MResponseEnergyLeakage.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MInterfaceMimrec)
#endif


////////////////////////////////////////////////////////////////////////////////


MInterfaceMimrec::MInterfaceMimrec() : MInterface()
{
  // standard constructor

  m_Imager = 0;
  m_Data = new MSettingsMimrec();
  m_BasicGuiData = dynamic_cast<MSettings*>(m_Data);

  m_EventFile = 0;
  m_Selector = new MEventSelector();

  m_SingleBackprojection = new double *[4];
}


////////////////////////////////////////////////////////////////////////////////


MInterfaceMimrec::~MInterfaceMimrec()
{
  // standard destructor

  delete m_Imager;
  // more missing!
}


////////////////////////////////////////////////////////////////////////////////


bool MInterfaceMimrec::ParseCommandLine(int argc, char** argv)
{
  ostringstream Usage;
  Usage<<endl;
  Usage<<"  Usage: mimrec <options>"<<endl;
  Usage<<endl;
  Usage<<"    Basic options:"<<endl;
  Usage<<"      -g --geometry <filename>.geo.setup:"<<endl;
  Usage<<"             Use this file as geometry-file"<<endl;
  Usage<<"      -f --filename <filename>.tra:"<<endl;
  Usage<<"             This is the file which is going to be analyzed"<<endl;
  Usage<<"      -c --configuration <filename>.cfg:"<<endl;
  Usage<<"             Use this file as parameter file (uses files from -f and -g)"<<endl;
  Usage<<"             If no configuration file is give ~/.mimrec.cfg is used"<<endl;
  Usage<<"      -d --debug:"<<endl;
  Usage<<"             Use debug mode"<<endl;
  Usage<<"      -h --help:"<<endl;
  Usage<<"             You know the answer..."<<endl;
  Usage<<endl;
  Usage<<"    High level functions:"<<endl;
  Usage<<"      -i --image:"<<endl;
  Usage<<"             Create an image"<<endl;
  Usage<<"      -s --spectrum:"<<endl;
  Usage<<"             Create a spectrum"<<endl;
  Usage<<"      -a --arm-gamma:"<<endl;
  Usage<<"             Create an arm"<<endl;
  Usage<<"      -e --event-selections:"<<endl;
  Usage<<"             Dump event selections"<<endl;
  Usage<<"         --standard-analysis-spherical <energy [keV]> <theta [deg]> <phi [deg]>"<<endl;
  Usage<<"             Do a standard analysis (Spectra, ARM, Counts) and dump the results to a *.sta file"<<endl;
  Usage<<endl;
  Usage<<"    Additional options for high level functions:"<<endl;
  Usage<<"      -n --no-gui:"<<endl;
  Usage<<"             Do not use a graphical user interface"<<endl;
  Usage<<"      -k --keep-alive:"<<endl;
  Usage<<"             Do not quit after executing a batch run, if we do have a gui"<<endl;
  //Usage<<"         --special:"<<endl;
  //Usage<<"             Activate special mode"<<endl;
  //Usage<<"      -  --:"<<endl;
  //Usage<<"             "<<endl;
  Usage<<endl;

  // Store some options temporarily:
  m_UseGui = true;
  bool KeepAlive = false;

  MString Option;

  // Check for help
  for (int i = 1; i < argc; i++) {
    Option = argv[i];
    if (Option == "-h" || Option == "--help" || Option == "?" || Option == "-?") {
      cout<<Usage.str()<<endl;
      return false;
    }
  }

  // First check if all options are ok:
  for (int i = 1; i < argc; i++) {
    Option = argv[i];

    // Single argument
    if (Option == "-g" || Option == "--geometry" || 
        Option == "-c" || Option == "--configuration" ||
        Option == "-f" || Option == "--filename") {
      if (!((argc > i+1) && argv[i+1][0] != '-')){
        cout<<"Error: Option "<<argv[i]<<" needs a second argument!"<<endl;
        cout<<Usage.str()<<endl;
        return false;
      }
    }
    // Tripple argument
    else if (Option == "--standard-analysis-spherical") {
      if (!((argc > i+3) && argv[i+1][0] != '-' && argv[i+2][0] != '-' && argv[i+3][0] != '-')){
        cout<<"Error: Option "<<argv[i]<<" needs 3 arguments!"<<endl;
        cout<<Usage.str()<<endl;
        return false;
      }
    }
  }
  
  // Now parse all first level options
  for (int i = 1; i < argc; i++) {
    Option = argv[i];
    if (Option == "--no-gui" || Option == "-n") {
      cout<<"Command-line parser: Do not use the gui"<<endl;
      m_UseGui = false;
      gROOT->SetBatch(true);
    } else if (Option == "--keep-alive" || Option == "-k") {
      cout<<"Command-line parser: Do not quit after batch run"<<endl;
      KeepAlive = true;
    } else if (Option == "--debug" || Option == "-d") {
      if (g_DebugLevel < 2) g_DebugLevel = 2;
      cout<<"Command-line parser: Use debug mode"<<endl;
    } else if (Option == "--configuration" || Option == "-c") {
      m_Data->Read(argv[++i]);
      cout<<"Command-line parser: Use configuration file "<<m_Data->GetSettingsFileName()<<endl;
    }
  }

  // Do we have a X up and running?
  if (gClient == 0 || gClient->GetRoot() == 0 || gROOT->IsBatch() == true) {
    cout<<"Command-line parser: No X-client or ROOT in batch mode: Using batch mode"<<endl;
    cout<<"                     If you didn't give a batch command, the program will exit."<<endl;
    m_UseGui = false;
    gROOT->SetBatch(true);
  }

  if (m_UseGui == false) KeepAlive = false;

  // Now parse all second level options
  for (int i = 1; i < argc; i++) {
    Option = argv[i];
    if (Option == "--geometry" || Option == "-g") {
      if (m_Data->SetGeometryFileName(argv[++i]) == false) {
        cout<<"Command-line parser: The geometry file could not be opened correctly: "<<argv[i]<<endl;
        return false;
      }
      cout<<"Command-line parser: Use geometry file "<<m_Data->GetGeometryFileName()<<endl;
    } else if (Option == "--filename" || Option == "-f") {
      if (m_Data->SetCurrentFileName(argv[++i]) == false) {
        cout<<"Command-line parser: The file could not be opened correctly: "<<argv[i]<<endl;
        return false;
      }
      cout<<"Command-line parser: Use file "<<m_Data->GetCurrentFileName()<<endl;
    } else if (Option == "--special" || Option == "--development") {
      m_Data->SetSpecialMode(true);
      cout<<"Command-line parser: Activating development extras mode - hope, you know what you are doing..."<<endl;
    }
  }

  // Load the geometry
  if (SetGeometry(m_Data->GetGeometryFileName(), false) == false) {
    cout<<"Command-line parser: "<<m_Data->GetGeometryFileName()<<" is no aceptable geometry file!"<<endl;
    cout<<"Command-line parser: Please give a correct geometry file via the -g option."<<endl;
    if (m_UseGui == true) {
      cout<<"Command-line parser: Trying to start with a dummy geometry..."<<endl;
      m_Data->SetGeometryFileName(g_MEGAlibPath + "/resource/examples/geomega/special/Dummy.geo.setup");
      if (SetGeometry(m_Data->GetGeometryFileName(), false) == false) {
        cout<<"Command-line parser: Hmmm, even reading of dummy geometry failed... Bye."<<endl;
        return false;
      }
    } else {
      return false;
    }
  }

  // Now parse all high level options
  for (int i = 1; i < argc; i++) {
    Option = argv[i];
    if (Option == "--spectrum" || Option == "-s") {
      cout<<"Command-line parser: Generating spectrum..."<<endl;  
      // m_Data->SetStoreImages(true);
      EnergySpectra();
      return KeepAlive;
    } else if (Option == "--arm-gamma" || Option == "-a") {
      cout<<"Command-line parser: Generating ARM gamma..."<<endl;  
      // m_Data->SetStoreImages(true);
      ARMGamma();
      return KeepAlive;
    } else if (Option == "--standard-analysis-spherical") {
      double Energy = atof(argv[++i]);
      double Theta = atof(argv[++i])*c_Rad;
      double Phi = atof(argv[++i])*c_Rad;
      MVector Position;
      Position.SetMagThetaPhi(c_FarAway, Theta, Phi);
      cout<<"Command-line parser: Performing standard analysis..."<<endl;  
      StandardAnalysis(Energy, Position);
      return KeepAlive;
    } else if (Option == "--event-selections" || Option == "-e") {
      cout<<"Command-line parser: Dumping event selections..."<<endl;  
      // m_Data->SetStoreImages(true);
      ShowEventSelections();
      return KeepAlive;
    } else if (Option == "--image" || Option == "-i") {
      cout<<"Command-line parser: Generating image..."<<endl;  
      // m_Data->SetStoreImages(true);
      Reconstruct();
      return KeepAlive;
    }
  }

   
  // Execute some low level commands
  if (m_UseGui == true) {
    m_Gui = new MGUIMimrecMain(this, m_Data);
    m_Gui->Create();
  } else {
    return false;
  }

  // Show change log / license if changed:
  MPrelude P;
  if (P.Play() == false) return false; // license was not accepted
  
  return true;
}


// ////////////////////////////////////////////////////////////////////////////////


// void MInterfaceMimrec::SetGuiData(MGUIData* BasicGuiData = 0)
// {
//   if (BasicGuiData == 0) {
//     m_Data = new MSettingsMimrec(); // == Load default
//     m_BasicGuiData = dynamic_cast<MGUIData*>(m_Data);
//   } else {
//     m_BasicGuiData = BasicGuiData;
//     m_Data = dynamic_cast<MSettingsMimrec*>(BasicGuiData);
//   }
// }


////////////////////////////////////////////////////////////////////////////////


bool MInterfaceMimrec::LoadConfiguration(MString FileName)
{  
  // Load the configuration file

  if (m_Data == 0) {
    m_Data = new MSettingsMimrec();
    m_BasicGuiData = dynamic_cast<MSettings*>(m_Data);
    if (m_UseGui == true) {
      m_Gui->SetConfiguration(m_Data);
    }
  }
  
  m_Data->Read(FileName);

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MInterfaceMimrec::SaveConfiguration(MString FileName)
{
  // Save the configuration file

  massert(m_Data != 0);

  m_Data->Write(FileName);

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MInterfaceMimrec::SetGeometry(MString FileName, bool UpdateGui)
{
  if (m_Geometry != 0) {
    delete m_Geometry;
    m_Geometry = 0;
  }

  if (FileName.EndsWith(g_StringNotDefined) == true) return false;

  // Check if the geometry exists:
  if (MFile::FileExists(FileName) == false) {
    mgui<<"The geometry file \""<<FileName<<"\" does not exist!!"<<error;
    m_BasicGuiData->SetGeometryFileName(g_StringNotDefined);
    return false;
  }

  MFile::ExpandFileName(FileName);

  m_Geometry = new MDGeometryQuest();
  if (m_Geometry->ScanSetupFile(FileName, false) == true) {
    m_BasicGuiData->SetGeometryFileName(FileName);
    mout<<"Geometry "<<m_Geometry->GetName()<<" loaded!"<<endl;
  } else {
    mgui<<"Loading of geometry \""<<FileName<<"\" failed!"<<endl;
    mgui<<"Please check the output for geometry errors and correct them!"<<error;
    delete m_Geometry;
    m_Geometry = 0;
    m_BasicGuiData->SetGeometryFileName(g_StringNotDefined);
  } 

  if (m_UseGui == true && UpdateGui == true) {
    m_Gui->UpdateConfiguration();
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


MVector MInterfaceMimrec::GetTestPosition()
{
  // Determine the test position of ARM, etc. cuts in various coordinate system

  MVector Test(0.0, 0.0, 1.0); 

  // Get the data of the ARM-"Test"-Position
  if (m_Data->GetCoordinateSystem() == MProjection::c_Spheric) {
    Test.SetMagThetaPhi(c_FarAway, m_Data->GetTPTheta()*c_Rad, m_Data->GetTPPhi()*c_Rad);
  } else if (m_Data->GetCoordinateSystem() == MProjection::c_Galactic) {
    Test.SetMagThetaPhi(c_FarAway, (m_Data->GetTPGalLatitude()+90)*c_Rad, m_Data->GetTPGalLongitude()*c_Rad);
  } else if (m_Data->GetCoordinateSystem() == MProjection::c_Cartesian2D ||
             m_Data->GetCoordinateSystem() == MProjection::c_Cartesian3D) {
    Test.SetXYZ(m_Data->GetTPX(), m_Data->GetTPY(), m_Data->GetTPZ());
  } else {
    merr<<"Unknown coordinate system ID: "<<m_Data->GetCoordinateSystem()<<fatal;
  }

  return Test;
}


////////////////////////////////////////////////////////////////////////////////


double MInterfaceMimrec::GetTotalEnergyMin()
{
  // Determine the test position of ARM, etc. cuts in various coordinate system

  double Min = numeric_limits<double>::max(); 

  if (m_Data->GetFirstEnergyRangeMax() > 0) {
    if (m_Data->GetFirstEnergyRangeMin() < Min) Min = m_Data->GetFirstEnergyRangeMin();
  }
  if (m_Data->GetSecondEnergyRangeMax() > 0) {
    if (m_Data->GetSecondEnergyRangeMin() < Min) Min = m_Data->GetSecondEnergyRangeMin();
  }
  if (m_Data->GetThirdEnergyRangeMax() > 0) {
    if (m_Data->GetThirdEnergyRangeMin() < Min) Min = m_Data->GetThirdEnergyRangeMin();
  }
  if (m_Data->GetFourthEnergyRangeMax() > 0) {
    if (m_Data->GetFourthEnergyRangeMin() < Min) Min = m_Data->GetFourthEnergyRangeMin();
  }

  return Min;
}


////////////////////////////////////////////////////////////////////////////////


double MInterfaceMimrec::GetTotalEnergyMax()
{
  // Determine the test position of ARM, etc. cuts in various coordinate system

  double Max = 0; 

  if (m_Data->GetFirstEnergyRangeMax() > 0) {
    if (m_Data->GetFirstEnergyRangeMax() > Max) Max = m_Data->GetFirstEnergyRangeMax();
  }
  if (m_Data->GetSecondEnergyRangeMax() > 0) {
    if (m_Data->GetSecondEnergyRangeMax() > Max) Max = m_Data->GetSecondEnergyRangeMax();
  }
  if (m_Data->GetThirdEnergyRangeMax() > 0) {
    if (m_Data->GetThirdEnergyRangeMax() > Max) Max = m_Data->GetThirdEnergyRangeMax();
  }
  if (m_Data->GetFourthEnergyRangeMax() > 0) {
    if (m_Data->GetFourthEnergyRangeMax() > Max) Max = m_Data->GetFourthEnergyRangeMax();
  }

  return Max;
}


////////////////////////////////////////////////////////////////////////////////


void MInterfaceMimrec::InterruptReconstruction()
{
  m_Interrupt = true;
}


////////////////////////////////////////////////////////////////////////////////


void MInterfaceMimrec::Reconstruct(bool Animate)
{
  // Main reconstruction routine

  bool JustDeconvolve = false;
  bool JustShowImage = false;

  if (Animate == false && // In case of animation we always perform backprojections
      m_Data->IsEventSelectionModified() == false &&
      m_Data->IsBackprojectionModified() == false) {
    if (m_Data->IsLikelihoodModified() == true) {
      int Return = 0;
      new TGMsgBox(gClient->GetRoot(), gClient->GetRoot(), "Info", 
                   "Only data concerning the deconvolution has been modified.\nDo you wish to just perform deconvolution?\nOtherwise also the response is redetermined, too.", 
                   kMBIconQuestion, kMBYes | kMBNo, &Return);
      if (Return == 1) {
        JustDeconvolve = true;
      }
    } else {
      int Return = 0;
      new TGMsgBox(gClient->GetRoot(), gClient->GetRoot(), "Info", 
                   "No data has been modified.\nDo you just want to see the last image again?\nIf not, the image will be recomputed.", 
                   kMBIconQuestion, kMBYes | kMBNo | kMBCancel, &Return);
      if (Return == 1) {
        JustShowImage = true;
      } else if (Return == 64) {
        return;
      }
    }
  }

  if (JustDeconvolve == false && JustShowImage == false) {
    if (m_Imager != 0) {
      delete m_Imager;
    }
    
    // Initialize:
    m_Imager = new MImager(m_Data->GetCoordinateSystem(),
                           m_Data->GetNThreads());
    m_Imager->SetGeometry(m_Geometry);
    
    // Maths:
    m_Imager->SetApproximatedMaths(m_Data->GetApproximatedMaths());
    
    // Set the dimensions of the image
    if (m_Data->GetCoordinateSystem() == MProjection::c_Spheric) {
      m_Imager->SetViewport(m_Data->GetPhiMin()*c_Rad, 
                            m_Data->GetPhiMax()*c_Rad, 
                            m_Data->GetBinsPhi(),
                            m_Data->GetThetaMin()*c_Rad,
                            m_Data->GetThetaMax()*c_Rad,
                            m_Data->GetBinsTheta(),
                            c_FarAway/10, 
                            c_FarAway, 
                            1, 
                            m_Data->GetImageRotationXAxis(), 
                            m_Data->GetImageRotationZAxis());
    } else if (m_Data->GetCoordinateSystem() == MProjection::c_Galactic) {
      m_Imager->SetViewport(m_Data->GetGalLongitudeMin()*c_Rad, 
                            m_Data->GetGalLongitudeMax()*c_Rad, 
                            m_Data->GetBinsGalLongitude(),
                            (m_Data->GetGalLatitudeMin()+90)*c_Rad,
                            (m_Data->GetGalLatitudeMax()+90)*c_Rad,
                            m_Data->GetBinsGalLatitude(),
                            c_FarAway/10, 
                            c_FarAway, 
                            1);
    } else if (m_Data->GetCoordinateSystem() == MProjection::c_Cartesian2D ||
               m_Data->GetCoordinateSystem() == MProjection::c_Cartesian3D){
      m_Imager->SetViewport(m_Data->GetXMin(), 
                            m_Data->GetXMax(), 
                            m_Data->GetBinsX(),
                            m_Data->GetYMin(), 
                            m_Data->GetYMax(), 
                            m_Data->GetBinsY(),
                            m_Data->GetZMin(), 
                            m_Data->GetZMax(), 
                            m_Data->GetBinsZ());
    } else {
      merr<<"Unknown coordinate system ID: "<<m_Data->GetCoordinateSystem()<<fatal;
    }
    
    // Set the draw modes
    m_Imager->SetDrawMode(m_Data->GetImageDrawMode());
    m_Imager->SetPalette(m_Data->GetImagePalette());
    m_Imager->SetSourceCatalog(m_Data->GetImageSourceCatalog());
    
    if (Animate == true) {
      m_Imager->SetAnimationMode(m_Data->GetAnimationMode());
      m_Imager->SetAnimationFrameTime(m_Data->GetAnimationFrameTime());
      m_Imager->SetAnimationFileName(m_Data->GetAnimationFileName());
    } else {
     m_Imager->SetAnimationMode(MImager::c_AnimateNothing); 
    }
    
    // Set the response type:
    if (m_Data->GetResponseType() == 0) {
      m_Imager->SetResponseGaussian(m_Data->GetFitParameterComptonTransSphere(), 
                                    m_Data->GetFitParameterComptonLongSphere(),
                                    m_Data->GetFitParameterPair(),
                                    m_Data->GetGauss1DCutOff(),
                                    m_Data->GetUseAbsorptions());
    } else if (m_Data->GetResponseType() == 1) {
      m_Imager->SetResponseGaussianByUncertainties();
    } else if (m_Data->GetResponseType() == 2) {
      m_Imager->SetResponseEnergyLeakage(m_Data->GetFitParameterComptonTransSphere(), 
                                         m_Data->GetFitParameterComptonLongSphere());
      
    } else if (m_Data->GetResponseType() == 3) {
      if (m_Imager->SetResponsePRM(m_Data->GetImagingResponseComptonTransversalFileName(),
                                   m_Data->GetImagingResponseComptonLongitudinalFileName(),
                                   m_Data->GetImagingResponsePairRadialFileName()) == false) {
        mgui<<"Cannot set PRM response! Aborting imaging!"<<error;
        return;
      }
    } else {
      merr<<"Unknown response type: "<<m_Data->GetResponseType()<<show;
      return;
    }
    m_Imager->UseAbsorptions(m_Data->GetUseAbsorptions());
  
    // The tra file name
    if (m_Imager->SetFileName(m_Data->GetCurrentFileName(), m_Data->GetFastFileParsing()) == false) {
      return;
    }

    // A new event selector:
    MEventSelector S;
    S.SetSettings(m_Data);
    S.SetGeometry(m_Geometry);
    m_Imager->SetEventSelector(S);

    // Memory management... needs clean up...
    m_Imager->SetMemoryManagment(m_Data->GetRAM(),
                                 m_Data->GetSwap(),
                                 m_Data->GetMemoryExhausted(),
                                 m_Data->GetBytes());
  }

  if (JustShowImage == false) {
    if (m_Data->GetLHAlgorithm() == MLMLAlgorithms::c_ClassicEM) {
      m_Imager->SetDeconvolutionAlgorithmClassicEM();
    } else if (m_Data->GetLHAlgorithm() == MLMLAlgorithms::c_OSEM) {
      m_Imager->SetDeconvolutionAlgorithmOSEM(m_Data->GetOSEMSubSets());
    } else {
      merr<<"Unknown deconvolution algorithm. Using classic EM."<<error;
      m_Imager->SetDeconvolutionAlgorithmClassicEM();
    }

    if (m_Data->GetLHStopCriteria() == 0) {
      m_Imager->SetStopCriterionByIterations(m_Data->GetNIterations());
    } else {
      merr<<"Unknown stop criterion. Stopping after 0 iterations."<<error;
      m_Imager->SetStopCriterionByIterations(0);
    }

    // Analyze... do the reconstruction
    m_Imager->Analyze(!JustDeconvolve);

    //   for (unsigned int i = 0; i < m_Imager->GetNImages(); ++i) {
    //     m_Imager->GetImage(i)->Display();
    //   }
  } else {
    if (m_Imager->GetNImages() > 0) {
      m_Imager->GetImage(m_Imager->GetNImages() - 1)->Display();
    }
  }


  // Reset the modification flags in the GUI data:
  m_Data->ResetEventSelectionModified();
  m_Data->ResetBackprojectionModified();
  m_Data->ResetLikelihoodModified();

  return;
}

////////////////////////////////////////////////////////////////////////////////

  
void MInterfaceMimrec::SpectralAnalyzer()
{
  //! The spectral analyzer

  MSpectralAnalyzer S;
  
  
  // Fill the initial histogram:
  S.SetGeometry(m_Geometry);
  S.SetSpectrum(1000, GetTotalEnergyMin(), GetTotalEnergyMax());

  // A new event selector:
  MEventSelector Sel;
  Sel.SetSettings(m_Data);
  Sel.SetGeometry(m_Geometry);

  // ... loop over all events and save a count in the belonging bin ...
  if (InitializeEventloader() == false) return;

  MPhysicalEvent* Event = 0;
  while ((Event = m_EventFile->GetNextEvent()) != 0) {
    if (m_Selector->IsQualifiedEvent(Event, true) == true) {
      S.FillSpectrum(Event->GetEnergy());
    }
		
    delete Event;
  } 
  
  
  // Set the GUI options
 
	// peak search

	S.SetSignaltoNoiseRatio(m_Data->GetSpectralSignaltoNoiseRatio());
	S.SetPoissonLimit(m_Data->GetSpectralPoissonLimit());
	
  // Isotope Selection
  S.SetIsotopeFileName(m_Data->GetSpectralIsotopeFileName());
  S.SetEnergyRange(m_Data->GetSpectralEnergyRange());
  
  // Do the analysis
  if (S.FindIsotopes() == true) {
//    cout<<"Found "<<S.GetNIsotopes()<<" isotopes."<<endl;
  }
}


////////////////////////////////////////////////////////////////////////////////

  
bool MInterfaceMimrec::InitializeEventloader(MString File)
{
  // Start the event loader...

  if (File == "") File = m_Data->GetCurrentFileName();

  if (m_EventFile != 0) delete m_EventFile;
  m_EventFile = new MFileEventsTra();
  if (m_EventFile->Open(File) == false) return false;
  m_EventFile->ShowProgress(m_UseGui);
  m_EventFile->SetFastFileParsing(m_Data->GetFastFileParsing());
  if (m_Data->GetNThreads() > 1) {
    m_EventFile->StartThread();
  }
  

  m_Selector->Reset();
  m_Selector->SetGeometry(m_Geometry);
  m_Selector->SetSettings(m_Data);

  return true;
}


////////////////////////////////////////////////////////////////////////////////

  
void MInterfaceMimrec::ShowEventSelections()
{
  // Show how many events pass the event selections
	
  int NEvents = 0;
  int NGoodEvents = 0;
	
  // ... loop over all events and save a count in the belonging bin ...
  if (InitializeEventloader() == false) return;

  MPhysicalEvent* Event = 0;
  while ((Event = m_EventFile->GetNextEvent()) != 0) {
    NEvents++;
    //cout<<Event->GetEventType()<<": "<<Event->ToString()<<endl;
    if (m_Selector->IsQualifiedEvent(Event, true) == true) {
      NGoodEvents++;
    }
		
    delete Event;
  } 
	
  cout<<endl;
  cout<<endl;
  cout<<"Event selections:"<<endl;
  cout<<endl;
  cout<<"All events  .................... "<<NEvents<<endl;
  cout<<"Not rejected events  ........... "<<NGoodEvents<<endl;
  cout<<endl;
	
  cout<<m_Selector->ToString()<<endl;
	
  m_EventFile->Close();
}


////////////////////////////////////////////////////////////////////////////////

  
void MInterfaceMimrec::ShowEventSelectionsStepwise()
{
  // Show how many events pass the event selections
	
  // ... loop over all events and save a count in the belonging bin ...
  if (InitializeEventloader() == false) return;

  MEventSelector AllOpen;
  int NAllOpen = 0;
  m_Selector->ApplyGeometry(AllOpen);

  MEventSelector RestrictDetectors = AllOpen;
  int NRestrictDetectors = 0;
  m_Selector->ApplyExcludedDetectors(RestrictDetectors);

  MEventSelector RestrictEventTypes = RestrictDetectors;
  int NRestrictEventTypes = 0;
  m_Selector->ApplyUseComptons(RestrictEventTypes);
  m_Selector->ApplyUseTrackedComptons(RestrictEventTypes);
  m_Selector->ApplyUseNotTrackedComptons(RestrictEventTypes);
  m_Selector->ApplyUsePairs(RestrictEventTypes);
  m_Selector->ApplyUsePhotos(RestrictEventTypes);
  m_Selector->ApplyUseUnidentifiables(RestrictEventTypes);
  m_Selector->ApplyUseDecays(RestrictEventTypes);

  MEventSelector RestrictBeamRadius = RestrictEventTypes;
  int NRestrictBeamRadius = 0;
  m_Selector->ApplyBeam(RestrictBeamRadius);
  m_Selector->ApplyBeamRadius(RestrictBeamRadius);

  MEventSelector RestrictBeamDepth = RestrictBeamRadius;
  int NRestrictBeamDepth = 0;
  m_Selector->ApplyBeamDepth(RestrictBeamDepth);

  MEventSelector RestrictEnergies = RestrictBeamDepth;
  int NRestrictEnergies = 0;
  m_Selector->ApplyFirstTotalEnergy(RestrictEnergies);
  m_Selector->ApplySecondTotalEnergy(RestrictEnergies);
  m_Selector->ApplyElectronEnergy(RestrictEnergies);
  m_Selector->ApplyGammaEnergy(RestrictEnergies);

  MEventSelector RestrictPhi = RestrictEnergies;
  int NRestrictPhi = 0;
  m_Selector->ApplyComptonAngle(RestrictPhi);

  MEventSelector RestrictTheta = RestrictPhi;
  int NRestrictTheta = 0;
  m_Selector->ApplyThetaDeviationMax(RestrictTheta);

  MEventSelector RestrictEHC = RestrictTheta;
  int NRestrictEHC = 0;
  m_Selector->ApplyEarthHorizonCut(RestrictEHC);

  MEventSelector RestrictQFC = RestrictEHC;
  int NRestrictQFC = 0;
  m_Selector->ApplyComptonQualityFactor(RestrictQFC);

  MEventSelector RestrictQFT = RestrictQFC;
  int NRestrictQFT = 0;
  m_Selector->ApplyTrackQualityFactor(RestrictQFT);

  MEventSelector RestrictDistance = RestrictQFT;
  int NRestrictDistance = 0;
  m_Selector->ApplyFirstDistance(RestrictDistance);
  m_Selector->ApplyDistance(RestrictDistance);

  MEventSelector RestrictARM = RestrictDistance;
  int NRestrictARM = 0;
  m_Selector->ApplySourceWindow(RestrictARM);
  m_Selector->ApplySourceARM(RestrictARM);

  MEventSelector RestrictSPD = RestrictARM;
  int NRestrictSPD = 0;
  m_Selector->ApplySourceWindow(RestrictSPD);
  m_Selector->ApplySourceSPD(RestrictSPD);

  int NRestrictAll= 0;


  MPhysicalEvent* Event = 0;
  while ((Event = m_EventFile->GetNextEvent()) != 0) {
    if (AllOpen.IsQualifiedEvent(Event, false) == true) {
      NAllOpen++;
    }
    if (RestrictDetectors.IsQualifiedEvent(Event, false) == true) {
      NRestrictDetectors++;
    }
    if (RestrictEventTypes.IsQualifiedEvent(Event, false) == true) {
      NRestrictEventTypes++;
    }
    if (RestrictBeamRadius.IsQualifiedEvent(Event, false) == true) {
      NRestrictBeamRadius++;
    }
    if (RestrictBeamDepth.IsQualifiedEvent(Event, false) == true) {
      NRestrictBeamDepth++;
    }
    if (RestrictPhi.IsQualifiedEvent(Event, false) == true) {
      NRestrictPhi++;
    }
    if (RestrictTheta.IsQualifiedEvent(Event, false) == true) {
      NRestrictTheta++;
    }
    if (RestrictEHC.IsQualifiedEvent(Event, false) == true) {
      NRestrictEHC++;
    }
    if (RestrictQFC.IsQualifiedEvent(Event, false) == true) {
      NRestrictQFC++;
    }
    if (RestrictQFT.IsQualifiedEvent(Event, false) == true) {
      NRestrictQFT++;
    }
    if (m_Selector->IsQualifiedEvent(Event, false) == true) {
      NRestrictAll++;
    }
    if (RestrictEnergies.IsQualifiedEvent(Event, false) == true) {
      NRestrictEnergies++;
    }
    if (RestrictDistance.IsQualifiedEvent(Event, false) == true) {
      NRestrictDistance++;
    }
    if (RestrictARM.IsQualifiedEvent(Event, false) == true) {
      NRestrictARM++;
    }
    if (RestrictSPD.IsQualifiedEvent(Event, false) == true) {
      NRestrictSPD++;
    }
    delete Event;
  } 
		
  m_EventFile->Close();

  cout<<endl;
  cout<<endl;
  cout<<"Event selections:                                "<<endl;
  cout<<endl;
  cout<<"No restrictions  ............................... "<<NAllOpen<<endl;
  cout<<"Apply detector restrictions  ................... "<<NRestrictDetectors<<endl;
  cout<<"Apply event type restrictions  ................. "<<NRestrictEventTypes<<endl;
  cout<<"Apply beam restrictions radius ................. "<<NRestrictBeamRadius<<endl;
  cout<<"Apply beam restrictions depth .................. "<<NRestrictBeamDepth<<endl;
  cout<<"Apply Energies restrictions  ................... "<<NRestrictEnergies<<endl;
  cout<<"Apply Phi restrictions  ........................ "<<NRestrictPhi<<endl;
  cout<<"Apply Theta deviation restrictions ............. "<<NRestrictTheta<<endl;
  cout<<"Apply EHC restrictions  ........................ "<<NRestrictEHC<<endl;
  cout<<"Apply Compton Quality factors restrictions  .... "<<NRestrictQFC<<endl;
  cout<<"Apply Track Quality factors restrictions  ...... "<<NRestrictQFT<<endl;
  cout<<"Apply Distance restrictions  ................... "<<NRestrictDistance<<endl;
  cout<<"Apply ARM restrictions  ........................ "<<NRestrictARM<<endl;
  cout<<"Apply SPD restrictions  ........................ "<<NRestrictSPD<<endl;
  cout<<"Apply all other restrictions  .................. "<<NRestrictAll<<endl;
  cout<<endl;
}


////////////////////////////////////////////////////////////////////////////////

  
void MInterfaceMimrec::ExtractEvents()
{
  // Show how many events pass the event selections
		
  // ... loop over all events and save a count in the belonging bin ...
  if (InitializeEventloader() == false) return;

  // Create a new tra file to which we can write the extracted events
  MString FileName = m_EventFile->GetFileName();
  FileName = FileName.Remove(FileName.Length()-4, 4); // remove final tra
  FileName += ".extracted.tra";
  MFileEventsTra* OutFile = new MFileEventsTra();
  OutFile->Open(FileName, MFile::c_Write);
  if (OutFile->IsOpen() == false) {
    mgui<<"Unable to open file: "<<FileName<<endl;
    mgui<<"Aborting event extraction"<<show;
    return;
  }
  

  MPhysicalEvent* Event = 0;
  while ((Event = m_EventFile->GetNextEvent()) != 0) {
    if (m_Selector->IsQualifiedEvent(Event) == true) {
      OutFile->AddEvent(Event);
    }
		
    delete Event;
  }
  m_EventFile->Close();
  OutFile->Close();

  mout<<"Extraction file created: "<<FileName<<endl;
}


////////////////////////////////////////////////////////////////////////////////

  
void MInterfaceMimrec::ThetaOriginDistribution()
{
  // Show how many events pass the event selections
	

  TH1D* Hist = new TH1D("ThetaOriginDistribution", 
                        "Theta Origin Distribution", 90, 0, 180);
  Hist->SetBit(kCanDelete);
  Hist->SetDirectory(0);
  Hist->SetXTitle("Zenit angle [#circ]");
  Hist->SetYTitle("counts");
  Hist->SetStats(false);
  Hist->SetFillColor(8);

	
  // ... loop over all events and save a count in the belonging bin ...
  if (InitializeEventloader() == false) return;

  MPhysicalEvent* Event = 0;
  while ((Event = m_EventFile->GetNextEvent()) != 0) {
    
    // Only accept Comptons within the selected ranges...
    if (m_Selector->IsQualifiedEvent(Event) == true) {
      if (Event->GetOrigin() != g_VectorNotDefined) {
        if (Event->GetType() == MPhysicalEvent::c_Compton) {
          Hist->Fill(dynamic_cast<MComptonEvent*>(Event)->Di().Theta()*c_Deg);          
        } else {
          Hist->Fill(Event->GetOrigin().Theta()*c_Deg);
        }
      }
    }

    delete Event;
  } 
  m_EventFile->Close();

  TCanvas* Canvas = new TCanvas("ThetaOriginDistributionCanvas", 
                                "Theta Origin Distribution Canvas", 800, 600);
  Canvas->SetFillColor(0);
  Canvas->SetFrameBorderSize(0);
  Canvas->SetFrameBorderMode(0);
  Canvas->SetBorderSize(0);
  Canvas->SetBorderMode(0);
  Canvas->cd();
  Hist->Draw();
  Canvas->Update();

  return;
}


////////////////////////////////////////////////////////////////////////////////

  
void MInterfaceMimrec::ARMGamma()
{
  // Display the angular resolution measurement for the gamma-ray
  // The ARM value for the scattered gamma-ray is the minimum angle between 
  // the gamma-cone-surface and the line connecting the cone-apex with the 
  // (Test-)position

  int NEvents = 0;
  double Value = 0;
  int NAverages = 0;
  double Average = 0;
  int Inside = 0;

  int NBins = m_Data->GetHistBinsARMGamma();
  double Disk = m_Data->GetTPDistanceTrans();
  MVector TestPosition = GetTestPosition();
  
  // Initalize the image size (x-axis)
  TH1D* Hist = new TH1D("ARMComptonCone", "ARM (Compton cone)", NBins, -Disk, Disk);
  Hist->SetBit(kCanDelete);
  Hist->SetDirectory(0);
  Hist->SetXTitle("ARM - Compton cone [#circ]");
  //Hist->SetYTitle("counts/degree");
  Hist->SetYTitle("counts");
  Hist->SetTitleOffset(1.3f, "Y");
  Hist->SetStats(false);
  Hist->SetLineColor(kBlack);
  //Hist->SetFillColor(8);
  Hist->SetMinimum(0);

  // Now restart the event-loader:
  if (InitializeEventloader() == false) return;

  MPhysicalEvent* Event = 0;
  MComptonEvent* ComptonEvent = 0; 
//   MPairEvent* PairEvent = 0; 
  // ... loop over all events and save a count in the belonging bin ...
  while ((Event = m_EventFile->GetNextEvent()) != 0) {

    // Only accept Comptons within the selected ranges...
    if (m_Selector->IsQualifiedEvent(Event) == true) {
      if (Event->GetEventType() == MPhysicalEvent::c_Compton) {
        ComptonEvent = dynamic_cast<MComptonEvent*>(Event);

        Value = ComptonEvent->GetARMGamma(TestPosition);
        if (Value*c_Deg > -Disk && Value*c_Deg < Disk) {
          Inside++;
        }

        Hist->Fill(Value*c_Deg);
        Average += ComptonEvent->Phi()*c_Deg;
        NEvents++;
        NAverages++;
//       } else if (Event->GetEventType() == MPhysicalEvent::c_Pair) {
//         PairEvent = dynamic_cast<MPairEvent*>(Event);

//         Value = PairEvent->GetARMGamma(TestPosition);
//         if (Value*c_Deg > -Disk && Value*c_Deg < Disk) Inside++;
//         Hist->Fill(Value*c_Deg);
//         NEvents++;
      }
    }

    delete Event;
  } 

  m_EventFile->Close();

  if (Hist->GetMaximum() == 0) {
    Error("ARMGamma()", "No events passed the event selections or file is empty!");
    return;
  }

//   // Normalize to counts/degree:
//   for (int b = 1; b <= Hist->GetNbinsX(); ++b) {
//     Hist->SetBinContent(b, Hist->GetBinContent(b)/Hist->GetBinWidth(b));
//     Hist->SetBinError(b, sqrt(Hist->GetBinContent(b)));
//   }

  // Determine 1 sigma, 2 sigma and 3 sigma radius:
  double Sigma0 = 0.5;
  bool Sigma0Found = false;
  double Sigma1 = 0.6826;
  bool Sigma1Found = false;
  double Sigma2 = 0.9546;
  bool Sigma2Found = false;
  double Sigma3 = 0.9973;
  bool Sigma3Found = false;

  int CentralBin = Hist->FindBin(0);
  double All = Hist->Integral(1, NBins);
  double Content = 0.0;
  for (int b = 0; b + CentralBin <= Hist->GetNbinsX(); ++b) {
    if (b == 0) {
      Content += Hist->GetBinContent(CentralBin);
    } else {
      Content += Hist->GetBinContent(CentralBin + b) + Hist->GetBinContent(CentralBin - b);
    }
    if (Sigma0Found == false && Content >= Sigma0*All) {
      mout<<100*Sigma0<<"% containment (radius): "<<Hist->GetBinCenter(CentralBin + b)<<endl;
      Sigma0Found = true;
    }
    if (Sigma1Found == false && Content >= Sigma1*All) {
      mout<<100*Sigma1<<"% containment (radius): "<<Hist->GetBinCenter(CentralBin + b)<<endl;
      Sigma1Found = true;
    }
    if (Sigma2Found == false && Content >= Sigma2*All) {
      mout<<100*Sigma2<<"% containment (radius): "<<Hist->GetBinCenter(CentralBin + b)<<endl;
      Sigma2Found = true;
    }
    if (Sigma3Found == false && Content >= Sigma3*All) {
      mout<<100*Sigma3<<"% containment (radius): "<<Hist->GetBinCenter(CentralBin + b)<<endl;
      Sigma3Found = true;
    }
    
  }


  TCanvas* Canvas = new TCanvas("CanvasARMGamma", "Canvas ARM Gamma", 800, 600);
  Canvas->SetFillColor(0);
  Canvas->SetFrameBorderSize(0);
  Canvas->SetFrameBorderMode(0);
  Canvas->SetBorderSize(0);
  Canvas->SetBorderMode(0);
  Canvas->cd();
  Hist->Draw();
  Canvas->Modified();
  Canvas->Update();

  TF1* Fit = 0;
  Fit = new TF1("DoubleLorentzAsymGausArm", DoubleLorentzAsymGausArm, 
                -Disk*0.99, Disk*0.99, 9);
  Fit->SetBit(kCanDelete);
  Fit->SetParNames("Offset", "Mean", 
                   "Lorentz Width 1", "Lorentz Height 1",
                   "Lorentz Width 2", "Lorentz Height 2",
                   "Gaus Height", "Gaus Sigma 1", "Gaus Sigma 2");
  Fit->SetParameters(0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1);
  Fit->SetParLimits(1, -Disk*0.99, Disk*0.99);
  Fit->FixParameter(0, 0);
  
  Canvas->cd();
  TFitResultPtr FitResult;
  TH1D* Confidence = 0;
  if (Fit != 0) {
    FitResult = Hist->Fit(Fit, "RQI SE");
    if (FitResult->IsValid() == true) {
      Confidence = new TH1D(*Hist);
      Confidence->SetName("ConfidenceHistogramARM");
      Confidence->SetTitle("Confidence Histogram ARM");
      Confidence->SetLineColor(kBlue-9);
      Confidence->SetFillColor(kBlue-9);
      (TVirtualFitter::GetFitter())->GetConfidenceIntervals(Confidence);
      Hist->SetTitle("ARM (Compton cone) with confidence intervals");
    }
  }
  Hist->Draw("HIST");
  if (Fit != 0) {
    if (FitResult->IsValid() == true) {
      Confidence->Draw("E5 SAME");
    } else {
      Fit->Draw("SAME");      
    }
  }
  Hist->Draw("HIST SAME");
  Canvas->Modified();
  Canvas->Update();
  
  cout<<endl;
  cout<<"Statistics of ARM histogram and fit: "<<endl;
  cout<<endl;
  cout<<"Analyzed Compton and pair events:        "<<NEvents<<endl;
  cout<<"Compton and pair events in histogram:    "<<Inside<<" ("<<((NEvents > 0) ? 100.0*Inside/NEvents : 0.0)<<"%)"<<endl;
  cout<<endl;
  cout<<"RMS:                                     "<<Hist->GetRMS()<<" deg"<<endl;
  cout<<endl;
  if (Fit != 0) {
    cout<<"Total FWHM of fit:                       "<<GetFWHM(Fit, -180, 180)<<" deg"<<endl;
    //cout<<"Maximum of fit (x position):             "<<Fit->GetMaximumX(-Disk, +Disk)<<" deg with "<<Fit->GetMaximum(-Disk, +Disk)<<endl;
    cout<<"Maximum of fit (x position):             "<<FitResult->Parameter(1)<<" ["<<FitResult->LowerError(1)+FitResult->Parameter(1)<<"..."<<FitResult->UpperError(1)+FitResult->Parameter(1)<<"] deg with maximum "<<Fit->Eval(FitResult->Parameter(1))<<" cts"<<endl;

    if (FitResult->IsValid() == false) {
      cout<<endl;
      cout<<"The fit to the data was not successful!"<<endl;
      cout<<"Try again with a different window or different bin size or more statistics."<<endl;
    }
  }
  cout<<endl;
  // delete Fit; // automatically deleted, when Canvas is deleted?

//   // Calculate RMS:
//   double rms = 0.0;
//   double rmssum = 0.0;
//   for (int i = 1; i <= Hist->GetXaxis()->GetNbins(); ++i) {
//     rmssum += Hist->GetBinContent(i);
//     rms += Hist->GetBinContent(i)*(Hist->GetXaxis()->GetBinCenter(i)*Hist->GetXaxis()->GetBinCenter(i));
//   }
//   rms = sqrt(rms/rmssum);
//   cout<<"RMS:                                     "<<rms<<endl;
//   cout<<"RMS:                                     "<<rmssum<<endl;


//   if (m_Data->GetStoreImages() == true) {
//     // Get the base file name of the tra file:
//     MString Name = m_Data->GetCurrentFileName();
//     Name.Remove(Name.Length()-4, 4);

//     Canvas->SaveAs(Name + ".ARMGamma.gif");
//     Canvas->SaveAs(Name + ".ARMGamma.eps");
//     Canvas->SaveAs(Name + ".ARMGamma.root");
//   }

  return;
}


////////////////////////////////////////////////////////////////////////////////

  
void MInterfaceMimrec::DualARM()
{
  // Display the angular resolution measurement for the gamma-ray
  // The ARM value for the scattered gamma-ray is the minimum angle between 
  // the gamma-cone-surface and the line connecting the cone-apex with the 
  // (Test-)position

  int NEvents = 0;
  int NEventsInside = 0;
  
  MVector TestPosition = GetTestPosition();
  
  double x1Max = m_Data->GetTPDistanceTrans();
  int x1Bins = m_Data->GetHistBinsARMGamma();

  double x2Max = m_Data->GetTPDistanceLong();
  int x2Bins = m_Data->GetHistBinsARMElectron();

  // Initalize the image size (x-axis)
  TH2D* Hist = new TH2D("DualARM", "Dual ARM", x1Bins, -x1Max, x1Max, x2Bins, -x2Max, x2Max);
  Hist->SetBit(kCanDelete);
  Hist->SetDirectory(0);
  Hist->SetXTitle("ARM - Compton cone [#circ]");
  Hist->SetYTitle("ARM - Electron cone [#circ]");
  Hist->SetZTitle("counts/degree^{2}");
  Hist->SetStats(false);
  Hist->SetFillColor(8);
  Hist->SetContour(50);

  // Now restart the event-loader:
  if (InitializeEventloader() == false) return;

  MPhysicalEvent* Event = 0;
  MComptonEvent* ComptonEvent = 0; 
  double xValue = 0;
  double yValue = 0;
  // ... loop over all events and save a count in the belonging bin ...
  while ((Event = m_EventFile->GetNextEvent()) != 0) {

    // Only accept Comptons within the selected ranges...
    if (m_Selector->IsQualifiedEvent(Event) == true) {
      if (Event->GetEventType() == MPhysicalEvent::c_Compton) {
        ComptonEvent = dynamic_cast<MComptonEvent*>(Event);

        xValue = ComptonEvent->GetARMGamma(TestPosition);
        yValue = ComptonEvent->GetSPDElectron(TestPosition);
        if (xValue*c_Deg > -x1Max && xValue*c_Deg < x1Max &&
            yValue*c_Deg > -x2Max && yValue*c_Deg < x2Max) {
          NEventsInside++;
        }
        Hist->Fill(xValue*c_Deg, yValue*c_Deg, 1);
        NEvents++;
      } 
    }

    delete Event;
  } 

  m_EventFile->Close();

  if (Hist->GetMaximum() == 0) {
    Error("ARMGamma()", "No events passed the event selections or file is empty!");
    return;
  }

  // Normalize to counts/degree:
  for (int b = 1; b <= Hist->GetNbinsX(); ++b) {
    Hist->SetBinContent(b, Hist->GetBinContent(b)/Hist->GetBinWidth(b));
  }

  TCanvas* Canvas = new TCanvas("CanvasDualARMGamma", "Dual ARM Gamma", 800, 600);
  Canvas->SetFillColor(0);
  Canvas->SetFrameBorderSize(0);
  Canvas->SetFrameBorderMode(0);
  Canvas->SetBorderSize(0);
  Canvas->SetBorderMode(0);

  Canvas->cd();
  Hist->Draw("colz");
  Canvas->Update();

  mout<<endl;
  mout<<"Some statistics: "<<endl;
  mout<<endl;
  mout<<"Analyzed Compton and pair events:        "<<NEvents<<endl;
  mout<<"Compton and pair events in histogram:    "<<NEventsInside
      <<" ("<<((NEvents > 0) ? 100.0*NEventsInside/NEvents : 0.0)<<"%)"<<endl;
 
  return;
}

////////////////////////////////////////////////////////////////////////////////

  
void MInterfaceMimrec::ARMResponseComparison()
{
  // Compare the ARM with the imaging response
   
  MVector TestPosition = GetTestPosition();
  
  double ARMMax = m_Data->GetTPDistanceTrans();
  int ARMBins = m_Data->GetHistBinsARMGamma();

  MResponse* Response = 0;
  if (m_Data->GetResponseType() == 0) {
    Response = new MResponseGaussian(m_Data->GetFitParameterComptonTransSphere(), 
                                     m_Data->GetFitParameterComptonLongSphere(),
                                     m_Data->GetFitParameterPair());
  } else if (m_Data->GetResponseType() == 1) {
    Response = new MResponseEnergyLeakage(m_Data->GetFitParameterComptonTransSphere(), 
                                       m_Data->GetFitParameterComptonLongSphere());
  } else if (m_Data->GetResponseType() == 2) {
    Response = new MResponseGaussianByUncertainties();
  } else if (m_Data->GetResponseType() == 3) {
    Response = new MResponsePRM();
    if (dynamic_cast<MResponsePRM*>(Response)->LoadResponseFiles(m_Data->GetImagingResponseComptonTransversalFileName(),
                                   m_Data->GetImagingResponseComptonLongitudinalFileName(),
                                   m_Data->GetImagingResponsePairRadialFileName()) == false) {
      mgui<<"Unable to load responsefiles!"<<endl;
      delete Response;
      return;
    }
  } else {
    merr<<"Unknown response type: "<<m_Data->GetResponseType()<<show;
    return;
  }
  
  // Initalize the ARM histogram
  TH1D* ARMHist = new TH1D("ARMHist", "ARM (coarsly binned) vs. Response (smoothly binned)", ARMBins, -ARMMax, ARMMax);
  ARMHist->SetBit(kCanDelete);
  ARMHist->SetDirectory(0);
  ARMHist->SetXTitle("ARM - Compton cone [#circ]");
  ARMHist->SetYTitle("normalized counts (I=1)");
  ARMHist->SetStats(false);
  ARMHist->SetContour(50);

  // Initalize the Response histogram
  TH1D* ResponseHist = new TH1D("ResponseHist", "ResponseHist", 10*ARMBins, -ARMMax, ARMMax);
  ResponseHist->SetBit(kCanDelete);
  ResponseHist->SetDirectory(0);
  ResponseHist->SetXTitle("ARM - Compton cone [#circ]");
  ResponseHist->SetYTitle("normalized counts");
  ResponseHist->SetStats(false);
  ResponseHist->SetContour(50);
  
  // Now restart the event-loader:
  if (InitializeEventloader() == false) return;

  MPhysicalEvent* Event = 0;
  MComptonEvent* ComptonEvent = 0; 
  // ... loop over all events and save a count in the belonging bin ...
  while ((Event = m_EventFile->GetNextEvent()) != 0) {

    // Only accept Comptons within the selected ranges...
    if (m_Selector->IsQualifiedEvent(Event) == true) {
      if (Event->GetEventType() == MPhysicalEvent::c_Compton) {
        ComptonEvent = dynamic_cast<MComptonEvent*>(Event);
        double ARM = ComptonEvent->GetARMGamma(TestPosition)*c_Deg;
        ARMHist->Fill(ARM, 1);
        Response->AnalyzeEvent(ComptonEvent);
        for (int b = 1; b <= ResponseHist->GetNbinsX(); ++b) {
          ResponseHist->SetBinContent(b, ResponseHist->GetBinContent(b) + Response->GetComptonResponse((ResponseHist->GetBinCenter(b) - ARM)*c_Rad)); 
        }
      } 
    }

    delete Event;
  } 

  m_EventFile->Close();

  if (ARMHist->GetMaximum() == 0) {
    Error("ARMGamma()", "No events passed the event selections or file is empty!");
    return;
  }

  // Normalize the response to the ARM histogram:
  ResponseHist->Scale(10.0/ResponseHist->Integral());
  ARMHist->Scale(1.0/ARMHist->Integral());

  TCanvas* Canvas = new TCanvas("ARMResponseComparison", "ARM Response Comparison", 800, 600);
  Canvas->cd();
  ARMHist->Draw();
  ResponseHist->Draw("SAME");
  Canvas->Update();
}

  
////////////////////////////////////////////////////////////////////////////////

  
void MInterfaceMimrec::AngularResolutionPairs()
{
  // Display the angular resolution measurement for the gamma-ray
  // The ARM value for the scattered gamma-ray is the minimum angle between 
  // the gamma-cone-surface and the line connecting the cone-apex with the 
  // (Test-)position

  int NEvents = 0;
  double Value = 0;
  int Inside = 0;

  int NBins = m_Data->GetHistBinsARMGamma();
  double Disk = m_Data->GetTPDistanceTrans();
  MVector TestPosition = GetTestPosition();
  
  // Initalize the image size (x-axis)
  //BinWidth = 2*Disk/NBins;
  TH1D* Hist = new TH1D("AngularResolutionPairs", "Angular resolution pairs", NBins, 0, Disk);
  Hist->SetBit(kCanDelete);
  Hist->SetDirectory(0);
  Hist->SetXTitle("Angular resolution [#circ]");
  Hist->SetYTitle("counts");
  Hist->SetStats(false);
  Hist->SetFillColor(8);
  Hist->SetMinimum(0);
  //double BinWidth = 2*Disk/NBins;

  TH2D* Hist2 = new TH2D("AngularResolutionPairsVsOpeningAngle", "Angular resolution pairs Vs Opening angle", NBins, 0, Disk,
                        9, 0, 90);
  Hist2->SetBit(kCanDelete);
  Hist2->SetXTitle("Angular resolution [#circ]");
  Hist2->SetYTitle("Opening angle [#circ]");
  Hist2->SetZTitle("counts");
  Hist2->SetStats(false);
  Hist2->SetFillColor(8);

  // Now restart the event-loader:
  if (InitializeEventloader() == false) return;

  MPhysicalEvent* Event = 0;
  MPairEvent* PairEvent = 0; 
  // ... loop over all events and save a count in the belonging bin ...
  while ((Event = m_EventFile->GetNextEvent()) != 0) {

    // Only accept Comptons within the selected ranges...
    if (m_Selector->IsQualifiedEvent(Event) == true) {
      if (Event->GetEventType() == MPhysicalEvent::c_Pair) {
        PairEvent = dynamic_cast<MPairEvent*>(Event);

        Value = PairEvent->GetARMGamma(TestPosition);
        if (Value*c_Deg > -Disk && Value*c_Deg < Disk) Inside++;
        Hist->Fill(Value*c_Deg);
        Hist2->Fill(Value*c_Deg, PairEvent->GetOpeningAngle()*c_Deg);
        NEvents++;
      }
    }

    delete Event;
  } 

  m_EventFile->Close();

  if (Hist->GetMaximum() == 0) {
    mgui<<"No events passed the event selections or file is empty!"<<endl;
    return;
  }

    
  TCanvas *Canvas = new TCanvas("Canvas angular resolution pairs", "Canvas angular resolution pairs", 800, 600);
  Canvas->SetFillColor(0);
  Canvas->SetFrameBorderSize(0);
  Canvas->SetFrameBorderMode(0);
  Canvas->SetBorderSize(0);
  Canvas->SetBorderMode(0);

//   TF1 *L = new TF1("LorentzAsymGausArm", LorentzAsymGausArm, -Disk*0.99, Disk*0.99, 8);
//   L->SetParNames("Lorentz Height", "Lorentz Width", "Lorentz Mean", "Lorentz Offset",
//                  "Gaus Height", "Gaus Mean", "Gaus Sigma 1", "Gaus Sigma 2");
//   L->SetParameters(1, 1, 0, 0, 1, 1, 1, 1);
//   L->SetParLimits(0, 0, 99999999);
//   //L->SetParLimits(1, 0, 99999999);
//   //L->SetParLimits(2, -9999999, 9999999);
//   //L->FixParameter(2, 0);
//   //L->FixParameter(3, 0);
//   //L->SetParLimits(3, 0, 99999);
//   //L->SetParLimits(5, 0, 100);
//   //L->SetParLimits(6, 0, 10000);
//   //L->SetParLimits(7, 0, 10000);
//   Hist->Fit("LorentzAsymGausArm", "RQw");

  //   TF1* AsymGausFunction = new TF1("AGF", AsymGaus, -Disk*0.99, Disk*0.99, 5);
  //   AsymGausFunction->SetParNames("Offset", "Gaus Height", "Gaus Mean", "Gaus Sigma 1", "Gaus Sigma 2");
  //   AsymGausFunction->SetParameters(L->GetParameter(3), L->GetParameter(4), 
  //                                   L->GetParameter(5), L->GetParameter(6), L->GetParameter(7));
  

  Canvas->cd();
  Hist->Draw();
  //L->Draw("CSAME");
  //AsymGausFunction->Draw("CSAME");
  Canvas->Update();

  cout<<endl;
  cout<<"Some statistics: "<<endl;
  cout<<endl;
  cout<<"Analyzed pair events:        "<<NEvents<<endl;
  cout<<"Pair events in histogram:    "<<Inside<<" ("<<((NEvents > 0) ? 100.0*Inside/NEvents : 0.0)<<"%)"<<endl;
//   cout<<"Total FWHM of fit:                       "<<GetFWHM(L, -180, 180)<<" deg"<<endl;
//   cout<<"Maximum of fit (x position):             "<<L->GetMaximumX(-Disk, +Disk)<<" deg"<<endl;
  cout<<endl;
//   delete L;


  // Calculate 68% containment:
  double Containment = 0.68;
  double All = Hist->Integral(1, NBins);
  double Content = 0;
  for (int b = 1; b <= NBins; b++) {
    Content += Hist->GetBinContent(b);
    if (Content >= Containment*All) {
      cout<<Containment<<"% containment: "<<Hist->GetBinCenter(b)<<endl;
      break;
    }
  }
    
  TCanvas* Canvas2 = new TCanvas("CanvasAngularResolutionPairsVsOpeningAngle", "Canvas angular resolution pairs vs. opening angle", 800, 600);
  Canvas2->SetFillColor(0);
  Canvas2->SetFrameBorderSize(0);
  Canvas2->SetFrameBorderMode(0);
  Canvas2->SetBorderSize(0);
  Canvas2->SetBorderMode(0);

  Canvas2->cd();
  Hist2->Draw("colz");
  Canvas2->Update();

  return;
}


////////////////////////////////////////////////////////////////////////////////

  
void MInterfaceMimrec::ARMGammaVsCompton()
{
  // Display the angular resolution measurement for the gamma-ray as function of
  // the Compton scatter angle
  // The ARM value for the scattered gamma-ray is the minimum angle between 
  // the gamma-cone-surface and the line connecting the cone-apex with the 
  // (Test-)position

  int NBins = m_Data->GetHistBinsARMGamma();
  double Disk = m_Data->GetTPDistanceTrans();
  MVector TestPosition = GetTestPosition();
  
  // Initalize the image size (x-axis)
  //BinWidth = 2*Disk/NBins;
  int NBinsArm = NBins;
  int NBinsAngle = NBins;
  TH2D* Hist = new TH2D("ARM vs. Compton Angle", "ARM vs. Compton Angle (normalized)", 
                        NBinsArm, -Disk, Disk, NBinsAngle, 
                        m_Data->GetComptonAngleRangeMin(), 
                        m_Data->GetComptonAngleRangeMax());
  Hist->SetBit(kCanDelete);
  Hist->SetXTitle("ARM [#circ]");
  Hist->SetYTitle("Compton scatter angle [#circ]");

  // Now restart the event-loader:
  if (InitializeEventloader() == false) return;

  MPhysicalEvent* Event = 0;
  MComptonEvent* ComptonEvent = 0; 
  // ... loop over all events and save a count in the belonging bin ...
  while ((Event = m_EventFile->GetNextEvent()) != 0) {

    // Only accept Comptons within the selected ranges...
    if (m_Selector->IsQualifiedEvent(Event) == true) {
      if (Event->GetEventType() == MPhysicalEvent::c_Compton) {
        ComptonEvent = dynamic_cast<MComptonEvent*>(Event);

        Hist->Fill(ComptonEvent->GetARMGamma(TestPosition)*c_Deg, 
                   ComptonEvent->Phi()*c_Deg);
      } 
    }

    delete Event;
  } 

  m_EventFile->Close();

  if (Hist->GetMaximum() == 0) {
    Error("ARMGamma()", "No events passed the event selections or file is empty!");
    return;
  }

  // Normalize:
  for (int by = 1; by <= Hist->GetNbinsY(); ++by) {
    double Sum = 0;
    for (int bx = 1; bx <= Hist->GetNbinsX(); ++bx) {
      Sum += Hist->GetBinContent(bx, by);
    }
    if (Sum > 0) {
      for (int bx = 1; bx <= Hist->GetNbinsX(); ++bx) {
        Hist->SetBinContent(bx, by, Hist->GetBinContent(bx, by)/Sum);
      }
    }
  }
    
  TCanvas* ARMvsComptonCanvas = 
    new TCanvas("Canvas ARM vs Compton", 
                "Canvas ARM vs Compton", 800, 600);
  ARMvsComptonCanvas->cd();
  //Nicen(Hist, ARMvsComptonCanvas);
  Hist->Draw("COLZ");
  ARMvsComptonCanvas->Update();

  return;
}


////////////////////////////////////////////////////////////////////////////////

  
void MInterfaceMimrec::ARMGammaVsDistance()
{
  // Display the angular resolution measurement for the gamma-ray as function of
  // the Compton scatter angle
  // The ARM value for the scattered gamma-ray is the minimum angle between 
  // the gamma-cone-surface and the line connecting the cone-apex with the 
  // (Test-)position

  int NBins = m_Data->GetHistBinsARMGamma();
  double Disk = m_Data->GetTPDistanceTrans();
  MVector TestPosition = GetTestPosition();
  
  // Initalize the image size (x-axis)
  //BinWidth = 2*Disk/NBins;
  int NBinsArm = NBins;
  int NBinsDistance = 20;
  TH2D* Hist = new TH2D("ARMVsDistance", "ARM vs. distance", 
                        NBinsArm, -Disk, Disk, NBinsDistance, 
                        m_Data->GetFirstDistanceRangeMin(), 
                        m_Data->GetFirstDistanceRangeMax());
  Hist->SetBit(kCanDelete);
  Hist->SetXTitle("ARM [#circ]");
  Hist->SetYTitle("Distance [cm]");

  cout<<"Dist: "<<m_Data->GetFirstDistanceRangeMin()<<":"<<m_Data->GetFirstDistanceRangeMax()<<endl;

  // Now restart the event-loader:
  if (InitializeEventloader() == false) return;

  MPhysicalEvent* Event = 0;
  MComptonEvent* ComptonEvent = 0; 
  // ... loop over all events and save a count in the belonging bin ...
  while ((Event = m_EventFile->GetNextEvent()) != 0) {

    // Only accept Comptons within the selected ranges...
    if (m_Selector->IsQualifiedEvent(Event) == true) {
      if (Event->GetEventType() == MPhysicalEvent::c_Compton) {
        ComptonEvent = dynamic_cast<MComptonEvent*>(Event);

        Hist->Fill(ComptonEvent->GetARMGamma(TestPosition)*c_Deg, 
                   (ComptonEvent->C2() - ComptonEvent->C1()).Mag());
      } 
    }

    delete Event;
  } 

  m_EventFile->Close();

  if (Hist->GetMaximum() == 0) {
    Error("ARMGamma()", "No events passed the event selections or file is empty!");
    return;
  }

//   // Normalize:
//   for (int by = 1; by <= Hist->GetNbinsY(); ++by) {
//     double Sum = 0;
//     for (int bx = 1; bx <= Hist->GetNbinsX(); ++bx) {
//       Sum += Hist->GetBinContent(bx, by);
//     }
//     if (Sum > 0) {
//       for (int bx = 1; bx <= Hist->GetNbinsX(); ++bx) {
//         Hist->SetBinContent(bx, by, Hist->GetBinContent(bx, by)/Sum);
//       }
//     }
//   }
    
  TCanvas* ARMvsComptonCanvas = 
    new TCanvas("CanvasARMVSDistance", 
                "Canvas ARM vs Distance", 800, 600);
  ARMvsComptonCanvas->cd();
  //Nicen(Hist, ARMvsComptonCanvas);
  Hist->Draw("COLZ");
  ARMvsComptonCanvas->Update();

  return;
}


////////////////////////////////////////////////////////////////////////////////

  
void MInterfaceMimrec::SignificanceMap()
{
  // Display the angular resolution measurement for the gamma-ray as function of
  // the Compton scatter angle
  // The ARM value for the scattered gamma-ray is the minimum angle between 
  // the gamma-cone-surface and the line connecting the cone-apex with the 
  // (Test-)position

  double XMin = m_Data->GetGalLongitudeMin();
  double XMax = m_Data->GetGalLongitudeMax();
  int NBinsXAngle = m_Data->GetBinsGalLongitude();
  double YMin = m_Data->GetGalLatitudeMin();
  double YMax = m_Data->GetGalLatitudeMax();
  int NBinsYAngle = m_Data->GetBinsGalLatitude();
  double Radius = m_Data->GetSignificanceMapRadius();
  double Distance = m_Data->GetSignificanceMapDistance();

  double dX = (XMax-XMin)/((double)NBinsXAngle);
  double dY = (YMax-YMin)/((double)NBinsYAngle);
  
  // Initalize the image size (x-axis)
  TH2D* Hist_CenterCounts = new TH2D("SignificanceMap_C", "Counts at each point within ARM cut", 
			NBinsXAngle, XMin, XMax,
			NBinsYAngle, YMin, YMax);
  Hist_CenterCounts->SetBit(kCanDelete);
  Hist_CenterCounts->SetXTitle("Galactic Longitude [#circ]");
  Hist_CenterCounts->SetYTitle("Galactic Latitude [#circ]");
  // Other histograms for counting
  // Center point and K test points
  TH2D* Hist_Center_0 = new TH2D("SignificanceMap_C_0", "Counts at center and 0 test points", 
				 NBinsXAngle, XMin, XMax,
				 NBinsYAngle, YMin, YMax);
  TH2D* Hist_Center_1 = new TH2D("SignificanceMap_C_1", "Counts at center and 1 test point", 
				 NBinsXAngle, XMin, XMax,
				 NBinsYAngle, YMin, YMax);
  TH2D* Hist_Center_2 = new TH2D("SignificanceMap_C_2", "Counts at center and 2 test points", 
				 NBinsXAngle, XMin, XMax,
				 NBinsYAngle, YMin, YMax);
  TH2D* Hist_Center_3 = new TH2D("SignificanceMap_C_3", "Counts at center and 3 test points", 
				 NBinsXAngle, XMin, XMax,
				 NBinsYAngle, YMin, YMax);
  TH2D* Hist_Center_4 = new TH2D("SignificanceMap_C_4", "Counts at center and 4 test points", 
				 NBinsXAngle, XMin, XMax,
				 NBinsYAngle, YMin, YMax);
  TH2D* Hist_Center_5 = new TH2D("SignificanceMap_C_5", "Counts at center and 5 test points", 
				 NBinsXAngle, XMin, XMax,
				 NBinsYAngle, YMin, YMax);
  TH2D* Hist_Center_6 = new TH2D("SignificanceMap_C_6", "Counts at center and 6 test points", 
				 NBinsXAngle, XMin, XMax,
				 NBinsYAngle, YMin, YMax);
  TH2D* Hist_Center_7 = new TH2D("SignificanceMap_C_7", "Counts at center and 7 test points", 
				 NBinsXAngle, XMin, XMax,
				 NBinsYAngle, YMin, YMax);
  TH2D* Hist_Center_8 = new TH2D("SignificanceMap_C_8", "Counts at center and 8 test points", 
				 NBinsXAngle, XMin, XMax,
				 NBinsYAngle, YMin, YMax);
  // No center point, but at K test points
  TH2D* Hist_NoCenter_1 = new TH2D("SignificanceMap_NC_1", "Counts at 1 test point, not at center", 
				   NBinsXAngle, XMin, XMax,
				   NBinsYAngle, YMin, YMax);
  TH2D* Hist_NoCenter_2 = new TH2D("SignificanceMap_NC_2", "Counts at 2 test points, not at center", 
				   NBinsXAngle, XMin, XMax,
				   NBinsYAngle, YMin, YMax);
  TH2D* Hist_NoCenter_3 = new TH2D("SignificanceMap_NC_3", "Counts at 3 test points, not at center", 
				   NBinsXAngle, XMin, XMax,
				   NBinsYAngle, YMin, YMax);
  TH2D* Hist_NoCenter_4 = new TH2D("SignificanceMap_NC_4", "Counts at 4 test points, not at center", 
				   NBinsXAngle, XMin, XMax,
				   NBinsYAngle, YMin, YMax);
  TH2D* Hist_NoCenter_5 = new TH2D("SignificanceMap_NC_5", "Counts at 5 test points, not at center", 
				   NBinsXAngle, XMin, XMax,
				   NBinsYAngle, YMin, YMax);
  TH2D* Hist_NoCenter_6 = new TH2D("SignificanceMap_NC_6", "Counts at 6 test points, not at center", 
				   NBinsXAngle, XMin, XMax,
				   NBinsYAngle, YMin, YMax);
  TH2D* Hist_NoCenter_7 = new TH2D("SignificanceMap_NC_7", "Counts at 7 test points, not at center", 
				   NBinsXAngle, XMin, XMax,
				   NBinsYAngle, YMin, YMax);
  TH2D* Hist_NoCenter_8 = new TH2D("SignificanceMap_NC_8", "Counts at 8 test points, not at center", 
				   NBinsXAngle, XMin, XMax,
				   NBinsYAngle, YMin, YMax);
  // Counts at each test point
  TH2D* Hist_Counts1 = new TH2D("SignificanceMap_Counts1", "Counts at test point 1", 
				   NBinsXAngle, XMin, XMax,
				   NBinsYAngle, YMin, YMax);
  TH2D* Hist_Counts2 = new TH2D("SignificanceMap_Counts2", "Counts at test point 2", 
				   NBinsXAngle, XMin, XMax,
				   NBinsYAngle, YMin, YMax);
  TH2D* Hist_Counts3 = new TH2D("SignificanceMap_Counts3", "Counts at test point 3", 
				   NBinsXAngle, XMin, XMax,
				   NBinsYAngle, YMin, YMax);
  TH2D* Hist_Counts4 = new TH2D("SignificanceMap_Counts4", "Counts at test point 4", 
				   NBinsXAngle, XMin, XMax,
				   NBinsYAngle, YMin, YMax);
  TH2D* Hist_Counts5 = new TH2D("SignificanceMap_Counts5", "Counts at test point 5", 
				   NBinsXAngle, XMin, XMax,
				   NBinsYAngle, YMin, YMax);
  TH2D* Hist_Counts6 = new TH2D("SignificanceMap_Counts6", "Counts at test point 6", 
				   NBinsXAngle, XMin, XMax,
				   NBinsYAngle, YMin, YMax);
  TH2D* Hist_Counts7 = new TH2D("SignificanceMap_Counts7", "Counts at test point 7", 
				   NBinsXAngle, XMin, XMax,
				   NBinsYAngle, YMin, YMax);
  TH2D* Hist_Counts8 = new TH2D("SignificanceMap_Counts8", "Counts at test point 8", 
				   NBinsXAngle, XMin, XMax,
				   NBinsYAngle, YMin, YMax);
  // Histograms for SNR calculation
  TH2D* Hist_Average = new TH2D("SignificanceMap_Average", "Average", 
				   NBinsXAngle, XMin, XMax,
				   NBinsYAngle, YMin, YMax);
  TH2D* Hist_Excess = new TH2D("SignificanceMap_Excess", "Excess", 
				   NBinsXAngle, XMin, XMax,
				   NBinsYAngle, YMin, YMax);
  TH2D* Hist_Sigma = new TH2D("SignificanceMap_Sigma", "Sigma", 
				   NBinsXAngle, XMin, XMax,
				   NBinsYAngle, YMin, YMax);
  TH2D* Hist_SNR = new TH2D("SignificanceMap_SNR", "Signal-to-noise ratio", 
			    NBinsXAngle, XMin, XMax,
			    NBinsYAngle, YMin, YMax);

  MVector BinCenter, TestPoint1, TestPoint2, TestPoint3, TestPoint4,
    TestPoint5, TestPoint6, TestPoint7, TestPoint8;
  int n_testpoints;
  double DistDiag = Distance/sqrt(2.);
  mout << "  Distance: " << Distance << endl;
  mout << "  Radius:   " << Radius << endl;

  // Now restart the event-loader:
  if (InitializeEventloader() == false) return;

  MPhysicalEvent* Event = 0;
  MComptonEvent* ComptonEvent = 0; 
  // ... loop over all events and save a count in the belonging bin ...
  while ((Event = m_EventFile->GetNextEvent()) != 0) {

    // Only accept Comptons within the selected ranges...
    if (m_Selector->IsQualifiedEvent(Event) == true) {
      if (Event->GetEventType() == MPhysicalEvent::c_Compton) {
        ComptonEvent = dynamic_cast<MComptonEvent*>(Event);
	// Go through each bin and record the counts
	for (int i_X=0; i_X<NBinsXAngle; i_X++) {
	  double L = XMin + dX*(double)i_X + dX/2.;
	  for (int i_Y=0; i_Y<NBinsYAngle; i_Y++) {
	    double B = YMin + dY*(double)i_Y + dY/2.;
	    // add Compton events that fall within the Radius of the center of each bin
	    BinCenter.SetMagThetaPhi(c_FarAway, (B+90)*c_Rad, L*c_Rad);
	    // Fix these test points... just using simple formulae for now, should use rotations
	    TestPoint1.SetMagThetaPhi(c_FarAway, (90+B+Distance)*c_Rad, (L         )*c_Rad);
	    TestPoint2.SetMagThetaPhi(c_FarAway, (90+B+DistDiag)*c_Rad, (L-DistDiag)*c_Rad);
	    TestPoint3.SetMagThetaPhi(c_FarAway, (90+B         )*c_Rad, (L-Distance)*c_Rad);
	    TestPoint4.SetMagThetaPhi(c_FarAway, (90+B-DistDiag)*c_Rad, (L-DistDiag)*c_Rad);
	    TestPoint5.SetMagThetaPhi(c_FarAway, (90+B-Distance)*c_Rad, (L         )*c_Rad);
	    TestPoint6.SetMagThetaPhi(c_FarAway, (90+B-DistDiag)*c_Rad, (L+DistDiag)*c_Rad);
	    TestPoint7.SetMagThetaPhi(c_FarAway, (90+B         )*c_Rad, (L+Distance)*c_Rad);
	    TestPoint8.SetMagThetaPhi(c_FarAway, (90+B+DistDiag)*c_Rad, (L+DistDiag)*c_Rad);
	    bool at_center = (fabs(ComptonEvent->GetARMGamma(BinCenter)*c_Deg)<=Radius);
	    bool at_TP1 = (fabs(ComptonEvent->GetARMGamma(TestPoint1)*c_Deg)<=Radius);
	    bool at_TP2 = (fabs(ComptonEvent->GetARMGamma(TestPoint2)*c_Deg)<=Radius);
	    bool at_TP3 = (fabs(ComptonEvent->GetARMGamma(TestPoint3)*c_Deg)<=Radius);
	    bool at_TP4 = (fabs(ComptonEvent->GetARMGamma(TestPoint4)*c_Deg)<=Radius);
	    bool at_TP5 = (fabs(ComptonEvent->GetARMGamma(TestPoint5)*c_Deg)<=Radius);
	    bool at_TP6 = (fabs(ComptonEvent->GetARMGamma(TestPoint6)*c_Deg)<=Radius);
	    bool at_TP7 = (fabs(ComptonEvent->GetARMGamma(TestPoint7)*c_Deg)<=Radius);
	    bool at_TP8 = (fabs(ComptonEvent->GetARMGamma(TestPoint8)*c_Deg)<=Radius);
	    n_testpoints = 0;
	    if (at_TP1) n_testpoints++;
	    if (at_TP2) n_testpoints++;
	    if (at_TP3) n_testpoints++;
	    if (at_TP4) n_testpoints++;
	    if (at_TP5) n_testpoints++;
	    if (at_TP6) n_testpoints++;
	    if (at_TP7) n_testpoints++;
	    if (at_TP8) n_testpoints++;
	    // Counts at center and test point
	    if (at_center) {
	      Hist_CenterCounts->Fill(L,B);
	    }
	    if (at_TP1) {
	      Hist_Counts1->Fill(L,B);
	    }
	    if (at_TP2) {
	      Hist_Counts2->Fill(L,B);
	    }
	    if (at_TP3) {
	      Hist_Counts3->Fill(L,B);
	    }
	    if (at_TP4) {
	      Hist_Counts4->Fill(L,B);
	    }
	    if (at_TP5) {
	      Hist_Counts5->Fill(L,B);
	    }
	    if (at_TP6) {
	      Hist_Counts6->Fill(L,B);
	    }
	    if (at_TP7) {
	      Hist_Counts7->Fill(L,B);
	    }
	    if (at_TP8) {
	      Hist_Counts8->Fill(L,B);
	    }
	    // Counts at center and K test points
	    if (at_center && (n_testpoints==0)) {
	      Hist_Center_0->Fill(L,B);
	    }
	    if (at_center && (n_testpoints==1)) {
	      Hist_Center_1->Fill(L,B);
	    }
	    if (at_center && (n_testpoints==2)) {
	      Hist_Center_2->Fill(L,B);
	    }
	    if (at_center && (n_testpoints==3)) {
	      Hist_Center_3->Fill(L,B);
	    }
	    if (at_center && (n_testpoints==4)) {
	      Hist_Center_4->Fill(L,B);
	    }
	    if (at_center && (n_testpoints==5)) {
	      Hist_Center_5->Fill(L,B);
	    }
	    if (at_center && (n_testpoints==6)) {
	      Hist_Center_6->Fill(L,B);
	    }
	    if (at_center && (n_testpoints==7)) {
	      Hist_Center_7->Fill(L,B);
	    }
	    if (at_center && (n_testpoints==8)) {
	      Hist_Center_8->Fill(L,B);
	    }
	    // No counts at center, but at K test points
	    if (!at_center && (n_testpoints==1)) {
	      Hist_NoCenter_1->Fill(L,B);
	    }
	    if (!at_center && (n_testpoints==2)) {
	      Hist_NoCenter_2->Fill(L,B);
	    }
	    if (!at_center && (n_testpoints==3)) {
	      Hist_NoCenter_3->Fill(L,B);
	    }
	    if (!at_center && (n_testpoints==4)) {
	      Hist_NoCenter_4->Fill(L,B);
	    }
	    if (!at_center && (n_testpoints==5)) {
	      Hist_NoCenter_5->Fill(L,B);
	    }
	    if (!at_center && (n_testpoints==6)) {
	      Hist_NoCenter_6->Fill(L,B);
	    }
	    if (!at_center && (n_testpoints==7)) {
	      Hist_NoCenter_7->Fill(L,B);
	    }
	    if (!at_center && (n_testpoints==8)) {
	      Hist_NoCenter_8->Fill(L,B);
	    }
	  }
	}
      } 
    }

    delete Event;
  } 

  m_EventFile->Close();

  if (Hist_CenterCounts->GetMaximum() == 0) {
    mgui<<"No events passed the event selections or file is empty!"<<error;
    return;
  }

  // Calculate average, excess, and SNR
  Hist_Average->Fill(-90.,-100.);
  for (int i_X=0; i_X<=NBinsXAngle; i_X++) {
    for (int i_Y=0; i_Y<=NBinsYAngle; i_Y++) {
      // calculate average of test points (average background counts)
      double avg = (1./8.)*( (double)Hist_Counts1->GetBinContent(i_X,i_Y)
			    +(double)Hist_Counts2->GetBinContent(i_X,i_Y)
			    +(double)Hist_Counts3->GetBinContent(i_X,i_Y)
			    +(double)Hist_Counts4->GetBinContent(i_X,i_Y)
			    +(double)Hist_Counts5->GetBinContent(i_X,i_Y)
			    +(double)Hist_Counts6->GetBinContent(i_X,i_Y)
			    +(double)Hist_Counts7->GetBinContent(i_X,i_Y)
			    +(double)Hist_Counts8->GetBinContent(i_X,i_Y) );
      Hist_Average->SetBinContent(i_X,i_Y,avg);
      // calculate excess counts
      double excess = Hist_CenterCounts->GetBinContent(i_X,i_Y)-avg;
      Hist_Excess->SetBinContent(i_X,i_Y,excess);
      // simple estimate of poisson noise
      double sigma1 = 0.;
      sigma1 = sqrt(avg);
      // more accurate estimate of poisson noise
      double N_C_0 = (double)Hist_Center_0->GetBinContent(i_X,i_Y);
      double N_C_1 = (double)Hist_Center_1->GetBinContent(i_X,i_Y);
      double N_C_2 = (double)Hist_Center_2->GetBinContent(i_X,i_Y);
      double N_C_3 = (double)Hist_Center_3->GetBinContent(i_X,i_Y);
      double N_C_4 = (double)Hist_Center_4->GetBinContent(i_X,i_Y);
      double N_C_5 = (double)Hist_Center_5->GetBinContent(i_X,i_Y);
      double N_C_6 = (double)Hist_Center_6->GetBinContent(i_X,i_Y);
      double N_C_7 = (double)Hist_Center_7->GetBinContent(i_X,i_Y);
      double N_C_8 = (double)Hist_Center_8->GetBinContent(i_X,i_Y);
      double N_notC_1 = (double)Hist_NoCenter_1->GetBinContent(i_X,i_Y);
      double N_notC_2 = (double)Hist_NoCenter_2->GetBinContent(i_X,i_Y);
      double N_notC_3 = (double)Hist_NoCenter_3->GetBinContent(i_X,i_Y);
      double N_notC_4 = (double)Hist_NoCenter_4->GetBinContent(i_X,i_Y);
      double N_notC_5 = (double)Hist_NoCenter_5->GetBinContent(i_X,i_Y);
      double N_notC_6 = (double)Hist_NoCenter_6->GetBinContent(i_X,i_Y);
      double N_notC_7 = (double)Hist_NoCenter_7->GetBinContent(i_X,i_Y);
      double N_notC_8 = (double)Hist_NoCenter_8->GetBinContent(i_X,i_Y);
      double sigma2 = N_C_0
	+ pow(1.-(1./8.),2)*N_C_1
	+ pow(1.-(2./8.),2)*N_C_2
	+ pow(1.-(3./8.),2)*N_C_3
	+ pow(1.-(4./8.),2)*N_C_4
	+ pow(1.-(5./8.),2)*N_C_5
	+ pow(1.-(6./8.),2)*N_C_6
	+ pow(1.-(7./8.),2)*N_C_7
	+ pow(1.-(8./8.),2)*N_C_8
	+ pow(1./8.,2)*N_notC_1
	+ pow(2./8.,2)*N_notC_2
	+ pow(3./8.,2)*N_notC_3
	+ pow(4./8.,2)*N_notC_4
	+ pow(5./8.,2)*N_notC_5
	+ pow(6./8.,2)*N_notC_6
	+ pow(7./8.,2)*N_notC_7
	+ pow(8./8.,2)*N_notC_8;
      sigma2 = sqrt(sigma2);
      mout << "Sigmas: 1= " << sigma1 << "  2= " << sigma2 << endl;
      // Choose one of the noise estimates
      double sigma = sigma2;
      Hist_Sigma->SetBinContent(i_X,i_Y,sigma);
      // calculate SNR!
      if (sigma != 0) {
        Hist_SNR->SetBinContent(i_X,i_Y,excess/sigma);
      }
    }
  }

  // Find min and max of histograms
  double hist_min = 1000000.;
  double hist_max = -1.;
  if (Hist_CenterCounts->GetMaximum()>hist_max) hist_max = Hist_CenterCounts->GetMaximum();
  if (Hist_CenterCounts->GetMinimum()<hist_min) hist_min = Hist_CenterCounts->GetMinimum();
  if (Hist_Counts1->GetMaximum()>hist_max) hist_max = Hist_Counts1->GetMaximum();
  if (Hist_Counts2->GetMaximum()>hist_max) hist_max = Hist_Counts2->GetMaximum();
  if (Hist_Counts3->GetMaximum()>hist_max) hist_max = Hist_Counts3->GetMaximum();
  if (Hist_Counts4->GetMaximum()>hist_max) hist_max = Hist_Counts4->GetMaximum();
  if (Hist_Counts5->GetMaximum()>hist_max) hist_max = Hist_Counts5->GetMaximum();
  if (Hist_Counts6->GetMaximum()>hist_max) hist_max = Hist_Counts6->GetMaximum();
  if (Hist_Counts7->GetMaximum()>hist_max) hist_max = Hist_Counts7->GetMaximum();
  if (Hist_Counts8->GetMaximum()>hist_max) hist_max = Hist_Counts8->GetMaximum();
  if (Hist_Counts1->GetMinimum()<hist_min) hist_min = Hist_Counts1->GetMinimum();
  if (Hist_Counts2->GetMinimum()<hist_min) hist_min = Hist_Counts2->GetMinimum();
  if (Hist_Counts3->GetMinimum()<hist_min) hist_min = Hist_Counts3->GetMinimum();
  if (Hist_Counts4->GetMinimum()<hist_min) hist_min = Hist_Counts4->GetMinimum();
  if (Hist_Counts5->GetMinimum()<hist_min) hist_min = Hist_Counts5->GetMinimum();
  if (Hist_Counts6->GetMinimum()<hist_min) hist_min = Hist_Counts6->GetMinimum();
  if (Hist_Counts7->GetMinimum()<hist_min) hist_min = Hist_Counts7->GetMinimum();
  if (Hist_Counts8->GetMinimum()<hist_min) hist_min = Hist_Counts8->GetMinimum();

  /*
  // Counts consistent with central bin
  TCanvas* Sig_CenterCounts_Canvas = 
    new TCanvas("SignificanceMap", 
                "Counts at each point within ARM cut", 800, 600);
  Sig_CenterCounts_Canvas->cd();
  // do a trick to reverse the x axis
  Sig_CenterCounts_Canvas->SetTheta(-90.001);
  Sig_CenterCounts_Canvas->SetPhi(180.001);
  Hist_CenterCounts->SetMinimum(hist_min);
  Hist_CenterCounts->SetMaximum(hist_max);
  Hist_CenterCounts->Draw("SURF2Z");
  //Hist_CenterCounts->Draw("COLZ");
  Sig_CenterCounts_Canvas->Update();

  // Counts consistent with test point 1
  TCanvas* Sig_Counts1_Canvas = 
    new TCanvas("SignificanceMap_TestPoint1", 
                "Counts at test point 1 within ARM cut", 800, 600);
  Sig_Counts1_Canvas->cd();
  // do a trick to reverse the x axis
  Sig_Counts1_Canvas->SetTheta(-90.001);
  Sig_Counts1_Canvas->SetPhi(180.001);
  Hist_Counts1->SetMinimum(hist_min);
  Hist_Counts1->SetMaximum(hist_max);
  Hist_Counts1->Draw("SURF2Z");
  //Hist_Counts1->Draw("COLZ");
  Sig_Counts1_Canvas->Update();

  // Counts consistent with test point 2
  TCanvas* Sig_Counts2_Canvas = 
    new TCanvas("SignificanceMap_TestPoint2", 
                "Counts at test point 2 within ARM cut", 800, 600);
  Sig_Counts2_Canvas->cd();
  // do a trick to reverse the x axis
  Sig_Counts2_Canvas->SetTheta(-90.001);
  Sig_Counts2_Canvas->SetPhi(180.001);
  Hist_Counts2->SetMinimum(hist_min);
  Hist_Counts2->SetMaximum(hist_max);
  Hist_Counts2->Draw("SURF2Z");
  //Hist_Counts2->Draw("COLZ");
  Sig_Counts2_Canvas->Update();

  // Counts consistent with test point 3
  TCanvas* Sig_Counts3_Canvas = 
    new TCanvas("SignificanceMap_TestPoint3", 
                "Counts at test point 3 within ARM cut", 800, 600);
  Sig_Counts3_Canvas->cd();
  // do a trick to reverse the x axis
  Sig_Counts3_Canvas->SetTheta(-90.001);
  Sig_Counts3_Canvas->SetPhi(180.001);
  Hist_Counts3->SetMinimum(hist_min);
  Hist_Counts3->SetMaximum(hist_max);
  Hist_Counts3->Draw("SURF2Z");
  //Hist_Counts3->Draw("COLZ");
  Sig_Counts3_Canvas->Update();

  // Counts consistent with test point 4
  TCanvas* Sig_Counts4_Canvas = 
    new TCanvas("SignificanceMap_TestPoint4", 
                "Counts at test point 4 within ARM cut", 800, 600);
  Sig_Counts4_Canvas->cd();
  // do a trick to reverse the x axis
  Sig_Counts4_Canvas->SetTheta(-90.001);
  Sig_Counts4_Canvas->SetPhi(180.001);
  Hist_Counts4->SetMinimum(hist_min);
  Hist_Counts4->SetMaximum(hist_max);
  Hist_Counts4->Draw("SURF2Z");
  //Hist_Counts4->Draw("COLZ");
  Sig_Counts4_Canvas->Update();

  // Counts consistent with test point 5
  TCanvas* Sig_Counts5_Canvas = 
    new TCanvas("SignificanceMap_TestPoint5", 
                "Counts at test point 5 within ARM cut", 800, 600);
  Sig_Counts5_Canvas->cd();
  // do a trick to reverse the x axis
  Sig_Counts5_Canvas->SetTheta(-90.001);
  Sig_Counts5_Canvas->SetPhi(180.001);
  Hist_Counts5->SetMinimum(hist_min);
  Hist_Counts5->SetMaximum(hist_max);
  Hist_Counts5->Draw("SURF2Z");
  //Hist_Counts5->Draw("COLZ");
  Sig_Counts5_Canvas->Update();

  // Counts consistent with test point 6
  TCanvas* Sig_Counts6_Canvas = 
    new TCanvas("SignificanceMap_TestPoint6", 
                "Counts at test point 6 within ARM cut", 800, 600);
  Sig_Counts6_Canvas->cd();
  // do a trick to reverse the x axis
  Sig_Counts6_Canvas->SetTheta(-90.001);
  Sig_Counts6_Canvas->SetPhi(180.001);
  Hist_Counts6->SetMinimum(hist_min);
  Hist_Counts6->SetMaximum(hist_max);
  Hist_Counts6->Draw("SURF2Z");
  //Hist_Counts6->Draw("COLZ");
  Sig_Counts6_Canvas->Update();

  // Counts consistent with test point 7
  TCanvas* Sig_Counts7_Canvas = 
    new TCanvas("SignificanceMap_TestPoint7", 
                "Counts at test point 7 within ARM cut", 800, 600);
  Sig_Counts7_Canvas->cd();
  // do a trick to reverse the x axis
  Sig_Counts7_Canvas->SetTheta(-90.001);
  Sig_Counts7_Canvas->SetPhi(180.001);
  Hist_Counts7->SetMinimum(hist_min);
  Hist_Counts7->SetMaximum(hist_max);
  Hist_Counts7->Draw("SURF2Z");
  //Hist_Counts7->Draw("COLZ");
  Sig_Counts7_Canvas->Update();

  // Counts consistent with test point 8
  TCanvas* Sig_Counts8_Canvas = 
    new TCanvas("SignificanceMap_TestPoint8", 
                "Counts at test point 8 within ARM cut", 800, 600);
  Sig_Counts8_Canvas->cd();
  // do a trick to reverse the x axis
  Sig_Counts8_Canvas->SetTheta(-90.001);
  Sig_Counts8_Canvas->SetPhi(180.001);
  Hist_Counts8->SetMinimum(hist_min);
  Hist_Counts8->SetMaximum(hist_max);
  Hist_Counts8->Draw("SURF2Z");
  //Hist_Counts8->Draw("COLZ");
  Sig_Counts8_Canvas->Update();
  mout << "Min of 8:             " << Hist_Counts8->GetMinimum() << endl;
  mout << "Max of 8:             " << Hist_Counts8->GetMaximum() << endl;

  // Average counts
  TCanvas* Sig_Average_Canvas = 
    new TCanvas("SignificanceMap_Average", 
                "Background estimate (average of four test points)", 800, 600);
  Sig_Average_Canvas->cd();
  // do a trick to reverse the x axis
  Sig_Average_Canvas->SetTheta(-90.001);
  Sig_Average_Canvas->SetPhi(180.001);
  Hist_Average->SetMinimum(hist_min);
  Hist_Average->SetMaximum(hist_max);
  Hist_Average->Draw("SURF2Z");
  mout << "Min of avg:             " << Hist_Average->GetMinimum() << endl;
  mout << "Max of avg:             " << Hist_Average->GetMaximum() << endl;
  //Hist_Average->Draw("COLZ");
  Sig_Average_Canvas->Update();

  // Excess counts
  TCanvas* Sig_Excess_Canvas = 
    new TCanvas("SignificanceMap_Excess", 
                "Excess counts above background", 800, 600);
  Sig_Excess_Canvas->cd();
  // do a trick to reverse the x axis
  Sig_Excess_Canvas->SetTheta(-90.001);
  Sig_Excess_Canvas->SetPhi(180.001);
  Hist_Excess->Draw("SURF2Z");
  //Hist_Excess->Draw("COLZ");
  Sig_Excess_Canvas->Update();
  
  // SNR
  TCanvas* Sig_SNR_Canvas = 
    new TCanvas("SignificanceMap_SNR", 
                "Signal-to-noise ratio", 800, 600);
  Sig_SNR_Canvas->cd();
  // do a trick to reverse the x axis
  Sig_SNR_Canvas->SetTheta(-90.001);
  Sig_SNR_Canvas->SetPhi(180.001);
  Hist_SNR->Draw("SURF2Z");
  //Hist_SNR->Draw("COLZ");
  Sig_SNR_Canvas->Update();
  */
  
  // Convert to MImage*
  double* Array = new double[NBinsXAngle*NBinsYAngle];
  for (int x = 0; x < NBinsXAngle; ++x) {
    for (int y = 0; y < NBinsYAngle; ++y) {
      Array[x+y*NBinsXAngle] = Hist_SNR->GetBinContent(x+1, y+1);
    }
  }

  MImageGalactic* Image = new MImageGalactic("Significance map", 
                               Array, 
                               "Longitude [deg]", 
                               XMin,
                               XMax, 
                               NBinsXAngle,
                               "Latitude [deg]", 
                               YMin, 
                               YMax, 
                               NBinsYAngle, 
                               m_Data->GetImagePalette(), 
                               m_Data->GetImageDrawMode(),
                               m_Data->GetImageSourceCatalog());
  Image->Display();
  
  delete [] Array;
  // Image: memory leak, but who cares...
  
  return;
}


////////////////////////////////////////////////////////////////////////////////

  
void MInterfaceMimrec::SPDElectronVsCompton()
{
  // Display the angular resolution measurement for the gamma-ray as function of
  // the Compton scatter angle
  // The ARM value for the scattered gamma-ray is the minimum angle between 
  // the gamma-cone-surface and the line connecting the cone-apex with the 
  // (Test-)position

  int NBins = m_Data->GetHistBinsARMElectron();
  double Disk = m_Data->GetTPDistanceLong();
  MVector TestPosition = GetTestPosition();
  
  // Initalize the image size (x-axis)
  //BinWidth = 2*Disk/NBins;
  int NBinsArm = NBins;
  int NBinsAngle = NBins/3;
  TH2D* Hist = new TH2D("SPD vs. Compton Scatter Angle", "SPD vs. Compton Scatter Angle", 
                        NBinsArm, 0, Disk, NBinsAngle, 
                        m_Data->GetComptonAngleRangeMin(), 
                        m_Data->GetComptonAngleRangeMax());
  Hist->SetBit(kCanDelete);
  Hist->SetXTitle("SPD [#circ]");
  Hist->SetYTitle("Compton scatter angle [#circ]");

  // Now restart the event-loader:
  if (InitializeEventloader() == false) return;

  MPhysicalEvent* Event = 0;
  MComptonEvent* ComptonEvent = 0; 
  // ... loop over all events and save a count in the belonging bin ...
  while ((Event = m_EventFile->GetNextEvent()) != 0) {

    // Only accept Comptons within the selected ranges...
    if (m_Selector->IsQualifiedEvent(Event) == true) {
      if (Event->GetEventType() == MPhysicalEvent::c_Compton) {
        ComptonEvent = dynamic_cast<MComptonEvent*>(Event);

        Hist->Fill(ComptonEvent->GetSPDElectron(TestPosition)*c_Deg, 
                   ComptonEvent->Phi()*c_Deg);
      } 
    }

    delete Event;
  } 

  m_EventFile->Close();

  if (Hist->GetMaximum() == 0) {
    Error("ARMGamma()", "No events passed the event selections or file is empty!");
    return;
  }

  


  // Normalize:
  for (int by = 1; by <= Hist->GetNbinsY(); ++by) {
    double Sum = 0;
    for (int bx = 1; bx <= Hist->GetNbinsX(); ++bx) {
      Sum += Hist->GetBinContent(bx, by);
    }
    if (Sum > 0) {
      for (int bx = 1; bx <= Hist->GetNbinsX(); ++bx) {
        Hist->SetBinContent(bx, by, Hist->GetBinContent(bx, by)/Sum);
      }
    }
  }
    
  // Fill empty bins with something extremely small:
  mimp<<"HACK!"<<endl;
  for (int by = 1; by <= Hist->GetNbinsY(); ++by) {
    for (int bx = 1; bx <= Hist->GetNbinsX(); ++bx) {
      if (Hist->GetBinContent(bx, by) == 0) {
        Hist->SetBinContent(bx, by, 1e-10);
      }
    }
  }

  TCanvas* ARMvsComptonCanvas = 
    new TCanvas("Canvas ARM vs Compton", 
                "Canvas ARM vs Compton", 800, 600);
  ARMvsComptonCanvas->cd();
  //Nicen(Hist, ARMvsComptonCanvas);
  Hist->Draw("COLZ");
  ARMvsComptonCanvas->Update();

  // 68% containment:
  vector<double> X(Hist->GetNbinsY());
  for (int by = 1; by <= Hist->GetNbinsY(); ++by) {
    double Sum = 0;
    double OldSum = 0;
    for (int bx = 1; bx <= Hist->GetNbinsX(); ++bx) {
      Sum += Hist->GetBinContent(bx, by);
      if (Sum > 0.68) {
        X[by-1] = Hist->GetXaxis()->GetBinLowEdge(bx) + Hist->GetXaxis()->GetBinWidth(bx)/(Sum-OldSum)*(0.68-OldSum);
        break;
      }
      OldSum = Sum;
    }
  }

  for (unsigned int i = 1; i < X.size(); ++i) {
    if (X[i-1] != 0 && X[i] != 0) {
      TLine* line = new TLine(X[i-1], Hist->GetYaxis()->GetBinCenter(i), 
                              X[i], Hist->GetYaxis()->GetBinCenter(i+1));
      line->SetLineWidth(3);
      line->Draw();
    }
  }
  ARMvsComptonCanvas->Update();

  return;
}


////////////////////////////////////////////////////////////////////////////////


void MInterfaceMimrec::ComptonProbabilityWithARMSelection()
{
  // Display the angular resolution measurement for the gamma-ray as function of
  // the Compton scatter angle
  // The ARM value for the scattered gamma-ray is the minimum angle between 
  // the gamma-cone-surface and the line connecting the cone-apex with the 
  // (Test-)position
  
  double Disk = m_Data->GetTPDistanceTrans();
  MVector TestPosition = GetTestPosition();

  // Initialize the image size
  int xNBins = m_Data->GetHistBinsARMGamma();
  double* xBins = CreateAxisBins(m_Data->GetComptonQualityFactorRangeMin(), m_Data->GetComptonQualityFactorRangeMax(), xNBins, true);



  TH1D* HistGood = new TH1D("Compton Quality Factor in and outside ARM Selection", 
                            "Compton Quality Factor in and outside ARM Selection", 
                            xNBins, xBins);
  HistGood->SetBit(kCanDelete);
  HistGood->SetXTitle("Compton Quality Factor");
  HistGood->SetYTitle("counts");
  HistGood->SetFillColor(8);
  //HistGood->SetMinimum(0);

  TH1D* HistBad = new TH1D("Compton Quality Factor inside (green) and outside (red) ARM Selection", 
                           "Compton Quality Factor inside (green) and outside (red) ARM Selection", 
                           xNBins, xBins);
  HistBad->SetBit(kCanDelete);
  HistBad->SetXTitle("Compton Quality Factor");
  HistBad->SetYTitle("counts");
  HistBad->SetFillColor(2);
  //HistBad->SetMinimum(0);

  // Now restart the event-loader:
  if (InitializeEventloader() == false) return;

  double ArmValue;
  MPhysicalEvent* Event = 0;
  MComptonEvent* ComptonEvent = 0; 
  // ... loop over all events and save a count in the belonging bin ...
  while ((Event = m_EventFile->GetNextEvent()) != 0) {

    // Only accept Comptons within the selected ranges...
    if (m_Selector->IsQualifiedEvent(Event) == true) {
      if (Event->GetEventType() == MPhysicalEvent::c_Compton) {
        ComptonEvent = dynamic_cast<MComptonEvent*>(Event);

        ArmValue = ComptonEvent->GetARMGamma(TestPosition)*c_Deg;
        if (fabs(ArmValue) < Disk) {
          HistGood->Fill(ComptonEvent->ComptonQualityFactor1());
        } else {
          HistBad->Fill(ComptonEvent->ComptonQualityFactor1());
        }
      } 
    }

    delete Event;
  } 

  m_EventFile->Close();

  if (HistGood->GetMaximum() == 0 && HistBad->GetMaximum() == 0) {
    Error("ARMGamma()", "No events passed the event selections or file is empty!");
    return;
  }

    
  TCanvas* CanvasHistGood = 
    new TCanvas("Compton Quality Factor in and outside ARM Selection",
                "Compton Quality Factor in and outside ARM Selection", 800, 600);
  CanvasHistGood->cd();
  HistBad->Draw();
  HistGood->Draw("SAME");
  CanvasHistGood->SetLogx();  
  CanvasHistGood->Update();
    
  delete [] xBins;

  return;
}


////////////////////////////////////////////////////////////////////////////////

  
void MInterfaceMimrec::ARMGammaVsComptonProbability()
{
  // Display the angular resolution measurement for the gamma-ray as function of
  // the Compton scatter angle
  // The ARM value for the scattered gamma-ray is the minimum angle between 
  // the gamma-cone-surface and the line connecting the cone-apex with the 
  // (Test-)position
  

  int NBins = m_Data->GetHistBinsARMGamma();
  double Disk = m_Data->GetTPDistanceTrans();
  MVector TestPosition = GetTestPosition();

  // Initialize the image size (x-axis)
  int x1NBins = NBins;
  double* x1Bins = CreateAxisBins(-Disk, +Disk, x1NBins, false);

  bool logx2 = true;
  int x2NBins = NBins;
  double* x2Bins = CreateAxisBins(m_Data->GetComptonQualityFactorRangeMin(), m_Data->GetComptonQualityFactorRangeMax(), x2NBins, logx2);



  TH2D* Hist = new TH2D("ARM vs. Compton Quality Factor", "ARM vs. Compton Quality Factor", 
                        x1NBins, x1Bins, x2NBins, x2Bins);
  Hist->SetBit(kCanDelete);
  Hist->SetXTitle("ARM [#circ]");
  Hist->SetYTitle("Compton quality factor");

  // Now restart the event-loader:
  if (InitializeEventloader() == false) return;

  MPhysicalEvent* Event = 0;
  MComptonEvent* ComptonEvent = 0; 
  // ... loop over all events and save a count in the belonging bin ...
  while ((Event = m_EventFile->GetNextEvent()) != 0) {

    // Only accept Comptons within the selected ranges...
    if (m_Selector->IsQualifiedEvent(Event) == true) {
      if (Event->GetEventType() == MPhysicalEvent::c_Compton) {
        ComptonEvent = dynamic_cast<MComptonEvent*>(Event);

        Hist->Fill(ComptonEvent->GetARMGamma(TestPosition)*c_Deg, 
                   ComptonEvent->ComptonQualityFactor1());
      } 
    }

    delete Event;
  } 

  m_EventFile->Close();

  if (Hist->GetMaximum() == 0) {
    Error("ARMGamma()", "No events passed the event selections or file is empty!");
    return;
  }

    
  TCanvas* ARMvsComptonCanvas = 
    new TCanvas("Canvas ARM vs Compton Probability",
                "Canvas ARM vs Compton Probability", 800, 600);
  ARMvsComptonCanvas->cd();
  if (logx2 == true) {
    ARMvsComptonCanvas->SetLogy();
  }  
  Hist->Draw("COLZ");
  ARMvsComptonCanvas->Update();

  delete [] x1Bins;
  delete [] x2Bins;

  return;
}


////////////////////////////////////////////////////////////////////////////////


void MInterfaceMimrec::ARMGammaVsClusteringProbability()
{
  // Display the angular resolution measurement for the gamma-ray as function of
  // the Compton scatter angle
  // The ARM value for the scattered gamma-ray is the minimum angle between 
  // the gamma-cone-surface and the line connecting the cone-apex with the 
  // (Test-)position

  int NBins = m_Data->GetHistBinsARMGamma();
  double Disk = m_Data->GetTPDistanceTrans();
  MVector TestPosition = GetTestPosition();

  // Initialize the image size (x-axis)
  int x1NBins = NBins;
  double* x1Bins = CreateAxisBins(-Disk, +Disk, x1NBins, false);

  bool logx2 = true;
  int x2NBins = NBins;
  double* x2Bins = CreateAxisBins(m_Data->GetClusteringQualityFactorRangeMin(), m_Data->GetClusteringQualityFactorRangeMax(), x2NBins, logx2);


  TH2D* Hist = new TH2D("ARMVsClusteringQualityFactor", "ARM vs. Clustering Quality Factor", 
                        x1NBins, x1Bins, x2NBins, x2Bins);
  Hist->SetBit(kCanDelete);
  Hist->SetXTitle("ARM [#circ]");
  Hist->SetYTitle("Clustering quality factor");

  // Now restart the event-loader:
  if (InitializeEventloader() == false) return;

  MPhysicalEvent* Event = 0;
  MComptonEvent* ComptonEvent = 0; 
  // ... loop over all events and save a count in the belonging bin ...
  while ((Event = m_EventFile->GetNextEvent()) != 0) {

    // Only accept Comptons within the selected ranges...
    if (m_Selector->IsQualifiedEvent(Event) == true) {
      if (Event->GetEventType() == MPhysicalEvent::c_Compton) {
        ComptonEvent = dynamic_cast<MComptonEvent*>(Event);

        Hist->Fill(ComptonEvent->GetARMGamma(TestPosition)*c_Deg, 
                   ComptonEvent->ClusteringQualityFactor());
      } 
    }

    delete Event;
  } 

  m_EventFile->Close();

  if (Hist->GetMaximum() == 0) {
    Error("ARMGamma()", "No events passed the event selections or file is empty!");
    return;
  }

    
  TCanvas* ARMvsClusteringCanvas = 
    new TCanvas("CanvasARMVsClusteringProbability",
                "Canvas ARM vs Clustering Probability", 800, 600);
  ARMvsClusteringCanvas->cd();
  ARMvsClusteringCanvas->SetLogy();  
  //Nicen(Hist, ARMvsClusteringCanvas);
  Hist->Draw("COLZ");
  ARMvsClusteringCanvas->Update();

  delete [] x1Bins;
  delete [] x2Bins;

  return;
}


////////////////////////////////////////////////////////////////////////////////

  
void MInterfaceMimrec::SPDVsTrackQualityFactor()
{
  // Display the angular resolution measurement for the gamma-ray as function of
  // the Compton scatter angle
  // The ARM value for the scattered gamma-ray is the minimum angle between 
  // the gamma-cone-surface and the line connecting the cone-apex with the 
  // (Test-)position

  int NBins = m_Data->GetHistBinsARMElectron();
  double Disk = m_Data->GetTPDistanceLong();
  MVector TestPosition = GetTestPosition();
  
  // Initalize the image size (x-axis)
  int xNBins = NBins;
  double* xBins = CreateAxisBins(0, +Disk, xNBins, false);

  bool yLog;
  if (m_Data->GetTrackQualityFactorRangeMin() > 0 && m_Data->GetTrackQualityFactorRangeMin() < 0.01) {
    yLog = true;
  } else {
    yLog = false;
  }
  int yNBins = NBins;
  double* yBins = CreateAxisBins(m_Data->GetTrackQualityFactorRangeMin(), m_Data->GetTrackQualityFactorRangeMax(), yNBins, yLog);


  // Create the histogram
  TH2D* Hist = new TH2D("SPD vs. Track Quality Factor", "SPD vs. Track Quality Factor", 
                        xNBins, xBins, yNBins, yBins);
  Hist->SetBit(kCanDelete);
  Hist->SetXTitle("SPD [#circ]");
  Hist->SetYTitle("Track quality factor");
  Hist->SetContour(50);

  // Now restart the event-loader:
  if (InitializeEventloader() == false) return;

  int NQFZero = 0;

  MPhysicalEvent* Event = 0;
  MComptonEvent* ComptonEvent = 0; 
  // ... loop over all events and save a count in the belonging bin ...
  while ((Event = m_EventFile->GetNextEvent()) != 0) {

    // Only accept Comptons within the selected ranges...
    if (m_Selector->IsQualifiedEvent(Event) == true) {
      if (Event->GetEventType() == MPhysicalEvent::c_Compton) {
        ComptonEvent = dynamic_cast<MComptonEvent*>(Event);

        Hist->Fill(ComptonEvent->GetSPDElectron(TestPosition)*c_Deg, 
                   ComptonEvent->TrackQualityFactor1());
        if (ComptonEvent->TrackQualityFactor1() == 0) NQFZero++;
      } 
    }

    delete Event;
  } 

  m_EventFile->Close();

  if (Hist->GetMaximum() == 0) {
    mgui<<"No events passed the event selections or file is empty!"<<info;
    mout<<"Number of events with a quality factor of zero: "<<NQFZero<<endl;
    return;
  }

    
  TCanvas* Canvas = 
    new TCanvas("SPDVsTrackQualityFactorCanvas",
                "SPD vs Track Quality Factor Canvas", 800, 600);
  Canvas->cd();
  if (yLog == true) {
    Canvas->SetLogy(); 
  } 
  Hist->Draw("COLZ");
  Canvas->Update();

  delete [] xBins;
  delete [] yBins;

  return;
}


////////////////////////////////////////////////////////////////////////////////

  
void MInterfaceMimrec::SPDVsTotalScatterAngleDeviation()
{
  // Display the angular resolution measurement for the gamma-ray as function of
  // the Compton scatter angle
  // The ARM value for the scattered gamma-ray is the minimum angle between 
  // the gamma-cone-surface and the line connecting the cone-apex with the 
  // (Test-)position


  int NBins = m_Data->GetHistBinsARMElectron();
  double Disk = m_Data->GetTPDistanceLong();
  MVector TestPosition = GetTestPosition();
  

  TH2D* Hist = new TH2D("SPD vs. Total Scatter Angle Deviation", 
                        "SPD vs. Total Scatter Angle Deviation", 
                        NBins, 0, Disk, NBins, 0, m_Data->GetThetaDeviationMax());
  Hist->SetBit(kCanDelete);
  Hist->SetXTitle("SPD [#circ]");
  Hist->SetYTitle("Theta deviation [#circ]");
  Hist->SetContour(50);

  // Now restart the event-loader:
  if (InitializeEventloader() == false) return;

  MPhysicalEvent* Event = 0;
  MComptonEvent* ComptonEvent = 0; 
  // ... loop over all events and save a count in the belonging bin ...
  while ((Event = m_EventFile->GetNextEvent()) != 0) {

    // Only accept Comptons within the selected ranges...
    if (m_Selector->IsQualifiedEvent(Event) == true) {
      if (Event->GetEventType() == MPhysicalEvent::c_Compton) {
        ComptonEvent = dynamic_cast<MComptonEvent*>(Event);

        Hist->Fill(ComptonEvent->GetSPDElectron(TestPosition)*c_Deg, 
                   ComptonEvent->DeltaTheta()*c_Deg);
      } 
    }

    delete Event;
  } 

  m_EventFile->Close();

  if (Hist->GetMaximum() == 0) {
    mgui<<"No events passed the event selections or file is empty!"<<info;
    return;
  }

    
  TCanvas* Canvas = 
    new TCanvas("SPDVsTotalScatterAngleDeviationCanvas",
                "SPD vs Total Scatter Angle Deviation Canvas", 800, 600);
  Canvas->cd();
  Hist->Draw("COLZ");
  Canvas->Update();

  return;
}


////////////////////////////////////////////////////////////////////////////////

  
void MInterfaceMimrec::EnergyVsComptonProbability()
{
  // Display the angular resolution measurement for the gamma-ray as function of
  // the Compton scatter angle
  // The ARM value for the scattered gamma-ray is the minimum angle between 
  // the gamma-cone-surface and the line connecting the cone-apex with the 
  // (Test-)position

  double x, y, z = 10000000.0;
  
  // Get the data of the ARM-"Test"-Position
  if (m_Data->GetCoordinateSystem() == MProjection::c_Spheric) {  // spheric 
    x = m_Data->GetTPTheta();
    y = m_Data->GetTPPhi();

    // All the external data stuff is in galactic coodinates, but the library uses
    // spherical, so transform it
    //MMath::GalacticToSpheric(x, y);
    MMath::SphericToCartesean(x, y, z);
  } else if (m_Data->GetCoordinateSystem() == MProjection::c_Cartesian2D ||
             m_Data->GetCoordinateSystem() == MProjection::c_Cartesian3D) {
    x = m_Data->GetTPX();
    y = m_Data->GetTPY();
    z = m_Data->GetTPZ();
  } else {
    merr<<"Unknown coordinate system ID: "<<m_Data->GetCoordinateSystem()<<fatal;
  }

  
  // Initialize the histogram size
  int x1NBins = 50;
  double* x1Bins = CreateAxisBins(GetTotalEnergyMin(), GetTotalEnergyMax(), x1NBins, false);

  int x2NBins = 20;
  double* x2Bins = CreateAxisBins(m_Data->GetComptonQualityFactorRangeMin(), m_Data->GetComptonQualityFactorRangeMax(), x2NBins, true);


  // Create the histogram
  TH2D* Hist = new TH2D("Energy vs. Compton Quality Factor", "Energy vs. Compton Quality Factor", 
                        x1NBins, x1Bins, x2NBins, x2Bins);
  Hist->SetBit(kCanDelete);
  Hist->SetXTitle("Energy [keV]");
  Hist->SetYTitle("Compton Quality Factor");

  // Now restart the event-loader:
  if (InitializeEventloader() == false) return;

  MPhysicalEvent* Event = 0;
  MComptonEvent* ComptonEvent = 0; 
  // ... loop over all events and save a count in the belonging bin ...
  while ((Event = m_EventFile->GetNextEvent()) != 0) {

    // Only accept Comptons within the selected ranges...
    if (m_Selector->IsQualifiedEvent(Event) == true) {
      if (Event->GetEventType() == MPhysicalEvent::c_Compton) {
        ComptonEvent = dynamic_cast<MComptonEvent*>(Event);

        Hist->Fill(ComptonEvent->Ei(), 
                   ComptonEvent->ComptonQualityFactor1());
      } 
    }

    delete Event;
  } 

  m_EventFile->Close();

  if (Hist->GetMaximum() == 0) {
    mgui<<"No events passed the event selections or file is empty!"<<endl;
    return;
  }

    
  TCanvas* EnergyvsComptonCanvas = 
    new TCanvas("Canvas Energy vs Compton Probability",
                "Canvas Energy vs Compton Probability", 800, 600);
  EnergyvsComptonCanvas->cd();
  EnergyvsComptonCanvas->SetLogy();  
  //Nicen(Hist, EnergyvsComptonCanvas);
  Hist->Draw("COLZ");
  EnergyvsComptonCanvas->Update();

  delete [] x1Bins;
  delete [] x2Bins;

  return;
}


////////////////////////////////////////////////////////////////////////////////

  
void MInterfaceMimrec::ComptonSequenceLengthVsComptonProbability()
{
  // Compton Sequence Length vs. Compton Quality Factor

  unsigned int MaxSequenceLength = 3;
  
  // Initialize the image size
  int xNBins = 100;
  double* xBins = CreateAxisBins(0.5, 100.5, xNBins, false);

  int yNBins = 100;
  double* yBins = CreateAxisBins(m_Data->GetComptonQualityFactorRangeMin(), m_Data->GetComptonQualityFactorRangeMax(), yNBins, true);


  TH2D* Hist = new TH2D("ComptonSequenceLengthVsComptonQualityFactor", 
                        "Compton Sequence Length vs. Compton Quality Factor", 
                        xNBins, xBins, yNBins, yBins);
  Hist->SetBit(kCanDelete);
  Hist->SetContour(50);
  Hist->SetXTitle("Compton Sequence Length");
  Hist->SetYTitle("Compton Quality Factor");

  // Now restart the event-loader:
  if (InitializeEventloader() == false) return;

  MPhysicalEvent* Event = 0;
  MComptonEvent* Compton = 0; 
  // ... loop over all events and save a count in the belonging bin ...
  while ((Event = m_EventFile->GetNextEvent()) != 0) {

    // Only accept Comptons within the selected ranges...
    if (m_Selector->IsQualifiedEvent(Event) == true) {
      if (Event->GetEventType() == MPhysicalEvent::c_Compton) {
        Compton = dynamic_cast<MComptonEvent*>(Event);

        if (Compton->SequenceLength() > MaxSequenceLength) MaxSequenceLength = Compton->SequenceLength();
        Hist->Fill(Compton->SequenceLength(), Compton->ComptonQualityFactor1());
      } 
    }

    delete Event;
  } 

  m_EventFile->Close();

  if (Hist->GetMaximum() == 0) {
    mgui<<"No events passed the event selections or file is empty!"<<show;
    return;
  }

    
  TCanvas* Canvas = 
    new TCanvas("CanvasComptonSequenceLengthVsComptonQualityFactor",
                "Canvas Compton Sequence Length vs. Compton Quality Factor", 800, 600);
  Canvas->cd();
  Canvas->SetLogy();  
  Hist->SetAxisRange(2, MaxSequenceLength);
  Hist->Draw("COLZ");
  Canvas->Update();

  delete [] xBins;
  delete [] yBins;

  return;
}


////////////////////////////////////////////////////////////////////////////////

  
void MInterfaceMimrec::SPDElectron()
{
  // Display the angular resolution measurement for the recoil electron
  // The ARM value for the recoil electron is the minimum angle between 
  // the electron-cone-surface and the line connecting the cone-apex with the 
  // (Test-) position
  
  // Display the angular resolution

  int NEvents = 0;
  double Value = 0;
  int NAverages = 0;
  double Average = 0;
  int Inside = 0;
  int FromTop = 0;
  int FromBottom = 0;

  double SizeARMGamma = 180;


  int NBins = m_Data->GetHistBinsARMElectron();
  double Disk = m_Data->GetTPDistanceLong();
  MVector TestPosition = GetTestPosition();

  
  // Initalize the image size (x-axis)
  //BinWidth = 2*Disk/NBins;
  TH1D* Hist = new TH1D("Scatter Plane Deviation",
                        "Scatter Plane Deviation", NBins, 0, Disk);
  Hist->SetXTitle("[deg]");
  Hist->SetYTitle("counts");
  Hist->SetBit(kCanDelete);
  Hist->SetStats(false);
  Hist->SetFillColor(8);
  Hist->SetMinimum(0);

  // Now restart the event-loader:
  if (InitializeEventloader() == false) return;

  // And fill the ARM-vector:

  MPhysicalEvent *Event;
  MComptonEvent *ComptonEvent; 
  // ... loop over all events and save a count in the belonging bin ...
  while ((Event = m_EventFile->GetNextEvent()) != 0) {

    // Only accept Comptons within the selected ranges...
    if (m_Selector->IsQualifiedEvent(Event) == false) {
      delete Event;
      continue;
    }
    if (Event->GetEventType() != MPhysicalEvent::c_Compton) {
      delete Event;
      continue;
    }

    ComptonEvent = dynamic_cast<MComptonEvent*>(Event);

    if (ComptonEvent->HasTrack() == false) {
      delete Event;
      continue;
    }

    Value = ComptonEvent->GetARMGamma(TestPosition);
    if (Value*c_Deg < -SizeARMGamma || Value*c_Deg > SizeARMGamma) {
      cout<<"Out of ARMGamma! Ag="<<Value*c_Deg
          <<" As="<<ComptonEvent->GetSPDElectron(TestPosition)*c_Deg<<endl;
      delete Event;
      continue;
    }

    Value = ComptonEvent->GetSPDElectron(TestPosition);

    if (Value*c_Deg < 90) {
      FromTop++;
    } else {
      FromBottom++;
    }

    if (Value*c_Deg > -Disk && Value*c_Deg < Disk) Inside++;

    //cout<<ComptonEvent->GetId()<<": "<<Value*c_Deg<<endl;
    Hist->Fill(Value*c_Deg);
    Average += ComptonEvent->Theta()*c_Deg;
    NAverages++;
    NEvents++;

    delete Event;
  } 

  m_EventFile->Close();

  if (Hist->GetMaximum() == 0) {
    Error("", "No events passed the event selections or file is empty!");
    return;
  }

    
  TCanvas *Canvas = new TCanvas("Canvas SPD Gamma", "Canvas SPD Gamma", 800, 600);
  Canvas->SetFillColor(0);
  Canvas->SetFrameBorderSize(0);
  Canvas->SetFrameBorderMode(0);
  Canvas->SetBorderSize(0);
  Canvas->SetBorderMode(0);

//   TF1 *L = new TF1("LorentzARM", LorentzARM, 0.01, Disk*0.99, 4);
//   L->SetParameters(1, 1, 1, 1);
//   //L->SetParLimits(0, 0, 99999);
//   //L->SetParLimits(1, -99999, 99999);
//   //L->SetParLimits(2, 0, 99999);
//   L->SetParLimits(3, 0, 99999);
//   L->FixParameter(2, 0);
//   L->SetParNames("Height", "Mean", "Sigma", "Offset");
//   Hist->Fit("LorentzARM", "Rw");

  TF1 *L = new TF1("LorentzGaussSPD", LorentzGaussSPD, 0.01, Disk*0.99, 5);
  L->SetParameters(1, 1, 1, 1, 1);
  //L->SetParLimits(0, 0, 99999);
  //L->SetParLimits(1, -99999, 99999);
  //L->SetParLimits(2, 0, 99999);
  L->SetParLimits(0, 0, 99999);
  //L->FixParameter(1, 0);
  L->SetParNames("Offset", "Sigma1", "Height1", 
                 "Sigma2", "Height2");
  Hist->Fit("LorentzGaussSPD", "Rw");

  Hist->SetStats(false);
  Canvas->cd();
  Hist->Draw();
  Canvas->Update();
  
  cout<<endl;
  cout<<"SPD - Characteristics:"<<endl;
  cout<<endl;
  cout<<"FWHM of fittet function:  "<<GetFWHM(L, -Disk, Disk)<<""<<endl;
  cout<<endl;
  delete L;
  cout<<"Inside: "<<Inside<<endl;
  cout<<"From top: "<<FromTop<<" ("<<100.0*FromTop/(FromTop+ FromBottom)<<"%)"<<endl;
  cout<<endl;

  // Calculate 68% containment:
  double Containment = 0.68;
  double All = Hist->Integral(1, NBins);
  double Content = 0;
  for (int b = 1; b <= NBins; b++) {
    Content += Hist->GetBinContent(b);
    if (Content >= Containment*All) {
      cout<<Containment<<"% containment: "<<Hist->GetBinCenter(b)<<endl;
      break;
    }
  }

  return;
}


////////////////////////////////////////////////////////////////////////////////

  
void MInterfaceMimrec::ARMElectron()
{
  // Display the angular resolution measurement for the recoil electron
  // The ARM value for the recoil electron is the minimum angle between 
  // the electron-cone-surface and the line connecting the cone-apex with the 
  // (Test-) position
  
  // Display the angular resolution

  int NEvents = 0;
  double Value = 0;
  int NAverages = 0;
  double Average = 0;
  int Inside = 0;
  int FromTop = 0;
  int FromBottom = 0;


  int NBins = m_Data->GetHistBinsARMElectron();
  double Disk = m_Data->GetTPDistanceLong();
  MVector TestPosition = GetTestPosition();

  // Initalize the image size (x-axis)
  //BinWidth = 2*Disk/NBins;
  TH1D* Hist = new TH1D("ARM - electron cone", 
                        "ARM - electron cone", NBins, -Disk, Disk);
  Hist->SetXTitle("ARM - electron cone [#circ]");
  Hist->SetYTitle("counts/degree");
  Hist->SetBit(kCanDelete);
  Hist->SetStats(false);
  Hist->SetFillColor(8);
  Hist->SetMinimum(0);

  // Now restart the event-loader:
  if (InitializeEventloader() == false) return;

  // And fill the ARM-vector:

  MPhysicalEvent *Event;
  MComptonEvent *ComptonEvent; 
  // ... loop over all events and save a count in the belonging bin ...
  while ((Event = m_EventFile->GetNextEvent()) != 0) {

    // Only accept Comptons within the selected ranges...
    if (m_Selector->IsQualifiedEvent(Event) == false) {
      delete Event;
      continue;
    }
    if (Event->GetEventType() != MPhysicalEvent::c_Compton) {
      delete Event;
      continue;
    }

    ComptonEvent = dynamic_cast<MComptonEvent*>(Event);

    if (ComptonEvent->HasTrack() == false) {
      delete Event;
      continue;
    }

    Value = ComptonEvent->GetARMElectron(TestPosition);

    if (Value*c_Deg < 70) {
      FromTop++;
    } else {
      FromBottom++;
    }

    if (Value*c_Deg > -Disk && Value*c_Deg < Disk) Inside++;
//     if (Value*c_Deg < 90) {
//       cout<<ComptonEvent->ToString()<<endl;
//     }

    Hist->Fill(Value*c_Deg);
    Average += ComptonEvent->Theta()*c_Deg;
    NAverages++;
    NEvents++;

    delete Event;
  } 

  m_EventFile->Close();

  if (Hist->GetMaximum() == 0) {
    Error("", "No events passed the event selections or file is empty!");
    return;
  }


  // Normalize to counts/degree:
  for (int b = 1; b <= Hist->GetNbinsX(); ++b) {
    Hist->SetBinContent(b, Hist->GetBinContent(b)/Hist->GetBinWidth(b));
    Hist->SetBinError(b, sqrt(Hist->GetBinContent(b)));
  }

  // Determine 1 sigma, 2 sigma and 3 sigma radius:
  double Sigma1 = 0.6826;
  bool Sigma1Found = false;
  double Sigma2 = 0.9546;
  bool Sigma2Found = false;
  double Sigma3 = 0.9973;
  bool Sigma3Found = false;

  int CentralBin = Hist->FindBin(0);
  double All = Hist->Integral(1, NBins);
  double Content = 0.0;
  for (int b = 0; b + CentralBin <= Hist->GetNbinsX(); ++b) {
    if (b == 0) {
      Content += Hist->GetBinContent(CentralBin);
    } else {
      Content += Hist->GetBinContent(CentralBin + b) + Hist->GetBinContent(CentralBin - b);
    }
    if (Sigma1Found == false && Content >= Sigma1*All) {
      cout<<100*Sigma1<<"% containment: "<<Hist->GetBinCenter(CentralBin + b)<<endl;
      Sigma1Found = true;
    }
    if (Sigma2Found == false && Content >= Sigma2*All) {
      cout<<100*Sigma2<<"% containment: "<<Hist->GetBinCenter(CentralBin + b)<<endl;
      Sigma2Found = true;
    }
    if (Sigma3Found == false && Content >= Sigma3*All) {
      cout<<100*Sigma3<<"% containment: "<<Hist->GetBinCenter(CentralBin + b)<<endl;
      Sigma3Found = true;
    }
    
  }

    
  TCanvas *Canvas = new TCanvas("Canvas ARM Electron", "Canvas ARM Electron", 800, 600);
  Canvas->SetFillColor(0);
  Canvas->SetFrameBorderSize(0);
  Canvas->SetFrameBorderMode(0);
  Canvas->SetBorderSize(0);
  Canvas->SetBorderMode(0);


  TF1* Fit = 0;
  Fit = new TF1("DoubleLorentzAsymGausArm", DoubleLorentzAsymGausArm, 
                -Disk*0.99, TMath::Min(40.0, Disk*0.99), 9);
  Fit->SetBit(kCanDelete);
  Fit->SetParNames("Offset", "Mean", 
                   "Lorentz Width1", "Lorentz Height1",
                   "Lorentz Width2", "Lorentz Height2",
                   "Gaus Height", "Gaus Sigma 1", "Gaus Sigma 2");
  Fit->SetParameters(0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1);
  Fit->SetParLimits(1, -Disk*0.99, Disk*0.99);


  
  Canvas->cd();
  Hist->Fit(Fit, "RQI");
  Hist->Draw("HIST");
  Fit->Draw("SAME");
  Canvas->Modified();
  Canvas->Update();
  
  cout<<endl;
  cout<<"ARM - Characteristics:"<<endl;
  cout<<endl;
  cout<<"Total FWHM of fit:                       "<<GetFWHM(Fit, -180, 180)<<" deg"<<endl;
  cout<<"Maximum of fit (x position):             "<<Fit->GetMaximumX(-Disk, +Disk)<<" deg"<<endl;
  cout<<endl;
  cout<<"Inside: "<<Inside<<" of "<<NEvents<<endl;
  cout<<"From top (< 70): "<<FromTop<<" ("<<100.0*FromTop/(FromTop+ FromBottom)<<"%)"<<endl;

  // delete Fit;

  return;
}


////////////////////////////////////////////////////////////////////////////////


void MInterfaceMimrec::EnergySpectra()
{
  // Display the energy-spectrum of all acceptable events 

  bool xLog = false;
  double xMin = GetTotalEnergyMin();
  double xMax = GetTotalEnergyMax();

  if (m_Data->GetSecondEnergyRangeMax() > 0) {
    if (m_Data->GetSecondEnergyRangeMax() > xMax) xMax = m_Data->GetSecondEnergyRangeMax();
    if (m_Data->GetSecondEnergyRangeMin() < xMin) xMin = m_Data->GetSecondEnergyRangeMin();
  }
  if (m_Data->GetThirdEnergyRangeMax() > 0) {
    if (m_Data->GetThirdEnergyRangeMax() > xMax) xMax = m_Data->GetThirdEnergyRangeMax();
    if (m_Data->GetThirdEnergyRangeMin() < xMin) xMin = m_Data->GetThirdEnergyRangeMin();
  }
  if (m_Data->GetFourthEnergyRangeMax() > 0) {
    if (m_Data->GetFourthEnergyRangeMax() > xMax) xMax = m_Data->GetFourthEnergyRangeMax();
    if (m_Data->GetFourthEnergyRangeMin() < xMin) xMin = m_Data->GetFourthEnergyRangeMin();
  }
  
//   double  xMin = 350.0;
//   double  xMax = 10000.0;

  MPhysicalEvent* Event;
  if (InitializeEventloader() == false) return;

  int NBins = m_Data->GetHistBinsSpectrum();
  double Disk = m_Data->GetTPDistanceTrans();
  MVector TestPosition = GetTestPosition();

  int InsideWindow = 0;
  int OutsideWindow = 0;

  if (xLog == true) {
    if (xMin <= 0) xMin = 1;
  }
  double* xBins = CreateAxisBins(xMin, xMax, NBins, xLog);

  TH1D* Hist = new TH1D("EnergySpectrum", "Energy spectrum", NBins, xBins);
  delete [] xBins;

  Hist->SetBit(kCanDelete);
  Hist->SetXTitle("Energy [keV]");
  Hist->SetYTitle("counts");
  //Hist->SetYTitle("counts");
  Hist->SetStats(false);
  Hist->SetFillColor(8);
  Hist->SetMinimum(0);

  double EMeas = 0.0;

  // ... loop over all events and save a count in the belonging bin ...
  while ((Event = m_EventFile->GetNextEvent()) != 0) {

    // Only accept Comptons within the selected ranges... 
    if (m_Selector->IsQualifiedEvent(Event) == false) {
      delete Event;
      continue;
    }

    if (Disk > 0) {
      if (Event->GetEventType() == MPhysicalEvent::c_Compton) {
        if (fabs(((MComptonEvent*) Event)->GetARMGamma(TestPosition))*c_Deg < Disk) {
          InsideWindow++;
          Hist->Fill(Event->GetEnergy());
          EMeas += Event->GetEnergy();
        } else {
          OutsideWindow++;
        }
      } else if (Event->GetEventType() == MPhysicalEvent::c_Pair) {
        if (fabs(((MPairEvent*) Event)->GetARMGamma(TestPosition))*c_Deg < Disk) {
          InsideWindow++;
          Hist->Fill(Event->GetEnergy());
          EMeas += Event->GetEnergy();
        } else {
          OutsideWindow++;
        }
      }
    } else {
      Hist->Fill(Event->GetEnergy());
      EMeas += Event->GetEnergy();
      InsideWindow++;
    }

    delete Event;
  } 
  m_EventFile->Close();
	
//   // %%%%%%%%%%%%%%%%%%%%%%%% Write Spectra as ASCII %%%%%%%%%%

//   // Get the base file name of the tra file:
//   MString AsciiSpectrumNameFull(m_Data->GetCurrentFileName());
//   MString AsciiSpectrumName(gSystem->BaseName(AsciiSpectrumNameFull.Data()));


//   AsciiSpectrumName.Remove(AsciiSpectrumName.Length()-4, 4);
//   AsciiSpectrumName.Append(".ASCIIspectrum.dat");
  
//   std::ofstream ASCIIout(AsciiSpectrumName, ios::out);
  
//   for (int b = 1; b <= (Hist->GetNbinsX() + 1); ++b)
//     {
//       ASCIIout << (Hist->GetBinLowEdge(b)) << " \t" ;
//       if (b <= Hist->GetNbinsX()) 
// 	{
// 	  ASCIIout << (Hist->GetBinContent(b)) << endl;
// 	} 
//       else 
// 	{
// 	  ASCIIout << "0.0 \n";
// 	}
      
//     }
  
//   ASCIIout.close();
  
//   cout << "Wrote ASCII spectrum to " <<  AsciiSpectrumName << endl;

//   // %%%%%%%%% End Write spectra as ASCII %%%%%%%%%%%%%%%%%%%%%


//   // Normalize to counts/keV
//   for (int b = 1; b <= Hist->GetNbinsX(); ++b) {
//     Hist->SetBinContent(b, Hist->GetBinContent(b)/Hist->GetBinWidth(b));
//   }

  TCanvas* Canvas = new TCanvas("SpectrumCanvas", "Spectrum canvas", 800, 600);
  Canvas->cd();
  if (xLog == true) {
    Canvas->SetLogx();
  }
  Hist->Draw();
  Canvas->Update();

 
  //TF1 *L = new TF1("TrippleGauss", TrippleGauss, 0.01*xMin, 0.99*xMax, 10);
//   TF1 *L = new TF1("TrippleGauss", TrippleGauss, 300, 900, 10);

//   L->SetParameters(1, 1, 1, 1, 1, 1, 1, 1, 1, 1);

//   L->SetParNames("Offset", "Sigma1", "Mean1", "Height1", 
//                  "Sigma2", "Mean2", "Height2",
//                  "Sigma3", "Mean3", "Height3");
//   Hist->Fit("TrippleGauss", "Rw");
  Hist->SetStats(false);
  Canvas->cd();
  Hist->SetMinimum(0);
  Hist->Draw();
  Canvas->Update(); 

  cout<<endl;
  cout<<"Energy spectrum - some additional statistics:"<<endl;
  cout<<"Number of events:     "<<InsideWindow+OutsideWindow<<" (inside="<<InsideWindow<<", outside="<<OutsideWindow<<")"<<endl;
  cout<<"Avg. measured energy: "<<EMeas/InsideWindow<<" keV"<<endl;
  cout<<endl;

  if (InsideWindow+OutsideWindow == 0) {
    cout<<m_Selector->ToString()<<endl;
  }

//   if (m_Data->GetStoreImages() == true) {
//     // Get the base file name of the tra file:
//     MString Name = m_Data->GetCurrentFileName();
//     Name.Remove(Name.Length()-4, 4);

//     Canvas->SaveAs(Name + ".Spectrum.gif");
//     Canvas->SaveAs(Name + ".Spectrum.eps");
//     Canvas->SaveAs(Name + ".Spectrum.root");
//   }

  return;
}


////////////////////////////////////////////////////////////////////////////////


void MInterfaceMimrec::InitialEnergyDeposit()
{
  // Display the energy-spectrum of all acceptable events 
  // (remember: not all of them will be reconstructable)

  double EnergyMin = m_Data->GetInitialEnergyDepositPairMin();
  double EnergyMax = m_Data->GetInitialEnergyDepositPairMax();

  MPhysicalEvent *Event;
  if (InitializeEventloader() == false) return;
  m_EventFile->IsOpen();


  // Data histogram:
  TH1D* Hist = new TH1D("InitialEnergyDeposit", "Energy deposit in first interaction layer", 100, EnergyMin, EnergyMax);
  Hist->SetBit(kCanDelete);
  Hist->SetXTitle("Energy [keV]");
  Hist->SetYTitle("counts");
  Hist->SetStats(false);
  Hist->SetFillColor(8);
  Hist->SetMinimum(0);

  // ... loop over all events and save a count in the belonging bin ...
  while ((Event = m_EventFile->GetNextEvent()) != 0) {

    // Only accept Comptons within the selected ranges... 
    if (m_Selector->IsQualifiedEvent(Event) == false) {
      delete Event;
      continue;
    }

    if (Event->GetEventType() == MPhysicalEvent::c_Pair) {
      Hist->Fill(((MPairEvent*) Event)->GetInitialEnergyDeposit());
    } 

    delete Event;
  } 
  m_EventFile->Close();

  TCanvas* SpectrumCanvas = new TCanvas("InitialEnergyDepositCanvas", "Initial energy deposit of pairs", 800, 600);
  SpectrumCanvas->cd();
  Hist->Draw();
  SpectrumCanvas->Update();

  return;
}


////////////////////////////////////////////////////////////////////////////////


void MInterfaceMimrec::EnergyDistributionElectronPhoton()
{
  // Display the energy-spectrum of all acceptable events 
  // (remember: not all of them will be reconstructable)

  if (InitializeEventloader() == false) return;

  double EnergyMax = GetTotalEnergyMax();

  TH2D* ScatterPlot = new TH2D("Energy Distribution", "Energy Distribution", 
                               50, 0, EnergyMax, 
                               50, 0, EnergyMax);
  ScatterPlot->SetBit(kCanDelete);
  ScatterPlot->SetXTitle("Energy D2 [keV]");
  ScatterPlot->SetYTitle("Energy D1 [keV]");
  ScatterPlot->SetStats(false);

  // ... loop over all events and save a count in the belonging bin ...
  MPhysicalEvent* Event = 0;
  while ((Event = m_EventFile->GetNextEvent()) != 0) {

    // Only accept Comptons within the selected ranges... 
    if (m_Selector->IsQualifiedEvent(Event) == true) {
      if (Event->GetEventType() == MPhysicalEvent::c_Compton) {
        ScatterPlot->Fill(((MComptonEvent*) Event)->Eg(), ((MComptonEvent*) Event)->Ee(), 1);
      }
    }

    delete Event;
  } 

  TCanvas* Canvas = new TCanvas("Scatter Plot", " Scatter Plot", 800, 600);
  Canvas->cd();
  ScatterPlot->Draw("COLZ");
  Canvas->Update();

  m_EventFile->Close();

  return;
}


////////////////////////////////////////////////////////////////////////////////


void MInterfaceMimrec::TimeWalkDistribution()
{
  // Display a time walk distribution

  if (InitializeEventloader() == false) return;

  TH1D* TWHist = new TH1D("Time walk Distribution", "Time walk Distribution", 
                          100, m_Data->GetTimeWalkRangeMin(), m_Data->GetTimeWalkRangeMax());
  TWHist->SetBit(kCanDelete);
  TWHist->SetXTitle("Time walk [ns]");
  TWHist->SetYTitle("#");
  TWHist->SetStats(false);
  TWHist->SetFillColor(8);

  // ... loop over all events and save a count in the belonging bin ...
  MPhysicalEvent* Event = 0;
  while ((Event = m_EventFile->GetNextEvent()) != 0) {

    // Only accept Comptons within the selected ranges... 
    if (m_Selector->IsQualifiedEvent(Event) == true) {
      TWHist->Fill(Event->GetTimeWalk());
    }

    delete Event;
  } 

  TCanvas* Canvas = new TCanvas("Time walk Distribution", "Time walk Distribution", 800, 600);
  Canvas->cd();
  TWHist->Draw();
  Canvas->Update();

  m_EventFile->Close();

  return;
}


////////////////////////////////////////////////////////////////////////////////


void MInterfaceMimrec::TimeWalkArmDistribution()
{
  // Display a time walk distribution

  MImage Image;
  Image.SetSpectrum(MImage::c_WhiteBlack);

  MVector TestPosition = GetTestPosition();

  if (InitializeEventloader() == false) return;

  TH2D* TWHist = new TH2D("Time walk ARM Distribution", "Time walk ARM Distribution", 
                          int(m_Data->GetTimeWalkRangeMax() - m_Data->GetTimeWalkRangeMin())/100, m_Data->GetTimeWalkRangeMin(), m_Data->GetTimeWalkRangeMax(), 
                          100, m_Data->GetComptonAngleRangeMin(), m_Data->GetComptonAngleRangeMax());
  TWHist->SetBit(kCanDelete);
  TWHist->SetXTitle("Time walk [ns]");
  TWHist->SetYTitle("ARM [deg]");
  TWHist->SetStats(false);

  // ... loop over all events and save a count in the belonging bin ...
  MPhysicalEvent* Event = 0;
  MComptonEvent* ComptonEvent = 0; 
  MPairEvent* PairEvent = 0; 
  while ((Event = m_EventFile->GetNextEvent()) != 0) {

    // Only accept Comptons within the selected ranges... 
    if (m_Selector->IsQualifiedEvent(Event) == true) {
      if (Event->GetEventType() == MPhysicalEvent::c_Compton) {
        ComptonEvent = dynamic_cast<MComptonEvent*>(Event);
        TWHist->Fill(Event->GetTimeWalk(), fabs(ComptonEvent->GetARMGamma(TestPosition))*c_Deg);
      } else if (Event->GetEventType() == MPhysicalEvent::c_Pair) {
        PairEvent = dynamic_cast<MPairEvent*>(Event);
        TWHist->Fill(Event->GetTimeWalk(), fabs(PairEvent->GetARMGamma(TestPosition))*c_Deg);
      }
    }

    delete Event;
  } 

  TCanvas* Canvas = new TCanvas("Time walk Distribution", "Time walk Distribution", 800, 600);
  Canvas->cd();
  TWHist->Draw("COLZ");
  Canvas->Update();

  m_EventFile->Close();

  return;
}


////////////////////////////////////////////////////////////////////////////////


void MInterfaceMimrec::ScatterAnglesDistribution()
{
  // Distribution of the gamma-scatter-angle

  int NBins = 90;

  //double Phi;
  int NPhi = 0;
  double AvgPhi = 0;
  double AvgSinPhi = 0;
  double PhiMin = m_Data->GetComptonAngleRangeMin();
  double PhiMax = m_Data->GetComptonAngleRangeMax();
  TH1D* PhiHist = new TH1D("PhiDistribution", "Compton scatter angle (phi) distribution", 
                           NBins, PhiMin, PhiMax);
  PhiHist->SetBit(kCanDelete);
  PhiHist->SetStats(false);
  PhiHist->SetFillColor(8);
  PhiHist->SetXTitle("[degree]");
  PhiHist->SetYTitle("#");
  PhiHist->SetMinimum(0);

  //double Epsilon;
  int NEpsilon = 0;
  double AvgEpsilon = 0;
  double EpsilonMin = 0;
  double EpsilonMax = 90;
  TH1D* EpsilonHist = new TH1D("EpsilonDistribution", "Electron scatter angle (epsilon) distribution", 
                               NBins, EpsilonMin, EpsilonMax);
  EpsilonHist->SetBit(kCanDelete);
  EpsilonHist->SetStats(false);
  EpsilonHist->SetFillColor(8);
  EpsilonHist->SetXTitle("[degree]");
  EpsilonHist->SetYTitle("#");
  EpsilonHist->SetMinimum(0);

  //double Theta;
  int NTheta = 0;
  double AvgTheta = 0;
  double ThetaMin = 0;
  double ThetaMax = 180;
  TH1D* ThetaHist = new TH1D("ThetaDistribution", "Total scatter angle (theta) distribution (calculated by energy)", NBins, ThetaMin, ThetaMax);
  ThetaHist->SetBit(kCanDelete);
  ThetaHist->SetStats(false);
  ThetaHist->SetFillColor(8);
  ThetaHist->SetXTitle("[degree]");
  ThetaHist->SetYTitle("#");
  ThetaHist->SetMinimum(0);

  double AvgThetaGeo = 0;
  TH1D* ThetaGeoHist = new TH1D("ThetaGeoDistribution", "Total scatter angle (theta) distribution (calculated by geometry)", NBins, ThetaMin, ThetaMax);
  ThetaGeoHist->SetBit(kCanDelete);
  ThetaGeoHist->SetStats(false);
  ThetaGeoHist->SetFillColor(8);
  ThetaGeoHist->SetXTitle("[degree]");
  ThetaGeoHist->SetYTitle("#");
  
  TH1D* ThetaDiffHist = new TH1D("ThetaDiffDistribution", "Total scatter angle (theta): difference between geometric and kinetic angle", NBins, -180, 180);
  ThetaDiffHist->SetBit(kCanDelete);
  ThetaDiffHist->SetStats(false);
  ThetaDiffHist->SetFillColor(8);
  ThetaDiffHist->SetXTitle("#theta_{kin} - #theta_{geo} [degree]");
  ThetaDiffHist->SetYTitle("#");
  


  MPhysicalEvent *Event;
  // Now restart the event-loader:
  if (InitializeEventloader() == false) return;

  bool FoundTrack = false;
  MComptonEvent *ComptonEvent; 
  // ... loop over all events and save a count in the belonging bin ...
  while ((Event = m_EventFile->GetNextEvent()) != 0) {

    // Only accept Comptons within the selected ranges... 
    if (m_Selector->IsQualifiedEvent(Event) == false) {
      delete Event;
      continue;
    }
    if (Event->GetEventType() != MPhysicalEvent::c_Compton) {
      delete Event;
      continue;
    }
    ComptonEvent = dynamic_cast<MComptonEvent*>(Event);
    FoundTrack = ComptonEvent->HasTrack();

    NPhi++;
    AvgPhi += ComptonEvent->Phi()*c_Deg;
    AvgSinPhi += sin(ComptonEvent->Phi());
    PhiHist->Fill(ComptonEvent->Phi()*c_Deg);

    if (FoundTrack == true) {
      NTheta++;
      AvgTheta += ComptonEvent->Theta()*c_Deg;
      ThetaHist->Fill(ComptonEvent->Theta()*c_Deg);

      AvgThetaGeo += ComptonEvent->CalculateThetaViaAngles()*c_Deg;
      ThetaGeoHist->Fill(ComptonEvent->CalculateThetaViaAngles()*c_Deg);

      ThetaDiffHist->Fill(ComptonEvent->Theta()*c_Deg - ComptonEvent->CalculateThetaViaAngles()*c_Deg);

      NEpsilon++;
      AvgEpsilon += ComptonEvent->Epsilon()*c_Deg;
      EpsilonHist->Fill(ComptonEvent->Epsilon()*c_Deg);
    }
    //cout<<"Sanity check: phi="<<ComptonEvent->Phi()<<" + eps="<<ComptonEvent->Epsilon()
    //    <<" = "<<ComptonEvent->Phi()+ComptonEvent->Epsilon()<<" === "<<ComptonEvent->Theta()<<endl;

    delete Event;
  }
  m_EventFile->Close();

  if (NPhi > 0) {
    mout<<"Average angles: "<<endl;
    mout<<endl;
    mout<<"Phi:          "<<AvgPhi/NPhi<<"  (avg sine: "<<AvgSinPhi/NPhi<<")"<<endl;
    mout<<"Theta:        "<<AvgTheta/NTheta<<endl;
    mout<<"Epsilon:      "<<AvgEpsilon/NEpsilon<<endl;
  

    TCanvas* PhiCanvas = new TCanvas("CanvasPhi", "Canvas of Compton scatter angle distribution", 800, 600);
    PhiHist->SetStats(false);
    PhiCanvas->cd();
    PhiHist->Draw();
    
    if (NTheta > 0) {
      TCanvas* EpsilonCanvas = new TCanvas("CanvasEpsilon", "Canvas of electron scatter angle distribution", 800, 600);
      EpsilonHist->SetStats(false);
      EpsilonCanvas->cd();
      EpsilonHist->Draw();
      
      TCanvas* ThetaCanvas = new TCanvas("CanvasThetaViaEnergy", "Canvas of total scatter angle distribution (kin)", 800, 600);
      ThetaHist->SetStats(false);
      ThetaCanvas->cd();
      ThetaHist->Draw();
      
      TCanvas* ThetaGeoCanvas = new TCanvas("CanvasThetaViaGeo", "Canvas of total scatter angle distribution (geo)", 800, 600);
      ThetaGeoHist->SetStats(false);
      ThetaGeoCanvas->cd();
      ThetaGeoHist->Draw();
      
      TCanvas* ThetaDiffCanvas = new TCanvas("CanvasThetaDiff", "Canvas of total scatter angle distribution: kin - geo", 800, 600);
      ThetaDiffHist->SetStats(false);
      ThetaDiffCanvas->cd();
      ThetaDiffHist->Draw();
    } else {
      delete EpsilonHist;
      delete ThetaHist;
      delete ThetaGeoHist;
      delete ThetaDiffHist;
      // other histaograms are deleted with Canvas
    }
  } else {
    mgui<<"No events passed the event selection"<<warn;
  }

  return;
}


////////////////////////////////////////////////////////////////////////////////


void MInterfaceMimrec::ClusteringQualityFactor()
{
  // Distribution of the Clustering quality factor

  bool xLog;
  if (m_Data->GetClusteringQualityFactorRangeMin() > 0 && m_Data->GetClusteringQualityFactorRangeMin() < 0.01) {
    xLog = true;
  } else {
    xLog = false;
  }
  int xNBins = 50;
  double* xBins = CreateAxisBins(m_Data->GetClusteringQualityFactorRangeMin(), m_Data->GetClusteringQualityFactorRangeMax(), xNBins, xLog);


  TH1D* Hist = new TH1D("ClusteringQualityFactor", "Clustering Quality Factor", 
                        xNBins, xBins);
  Hist->SetBit(kCanDelete);
  Hist->SetXTitle("Clustering Quality Factor");
  Hist->SetYTitle("counts");
  Hist->SetStats(false);
  Hist->SetFillColor(8);
  Hist->SetMinimum(0);


  MPhysicalEvent *Event;
  // Now restart the event-loader:
  if (InitializeEventloader() == false) return;

  MComptonEvent *ComptonEvent; 
  // ... loop over all events and save a count in the belonging bin ...
  while ((Event = m_EventFile->GetNextEvent()) != 0) {

    // Only accept Comptons within the selected ranges... 
    if (m_Selector->IsQualifiedEvent(Event) == false) {
      delete Event;
      continue;
    }
    if (Event->GetEventType() != MPhysicalEvent::c_Compton) {
      delete Event;
      continue;
    }
    ComptonEvent = dynamic_cast<MComptonEvent*>(Event);

    Hist->Fill(ComptonEvent->ClusteringQualityFactor());
    
    delete Event;
  }
  m_EventFile->Close();

  TCanvas* Canvas = new TCanvas("CanvasClusteringQF", "Canvas Clustering Quality Factor", 800, 600);
  Canvas->cd();
  if (xLog == true) {
    Canvas->SetLogx();
  }
  Hist->Draw();
  Canvas->Update();

  delete [] xBins;

  return;
}


////////////////////////////////////////////////////////////////////////////////


void MInterfaceMimrec::ComptonQualityFactor()
{
  // Distribution of the Compton quality factor

  bool xLog;
  if (m_Data->GetComptonQualityFactorRangeMin() > 0 && m_Data->GetComptonQualityFactorRangeMin() < 0.01) {
    xLog = true;
  } else {
    xLog = false;
  }
  int xNBins = 50;
  double* xBins = CreateAxisBins(m_Data->GetComptonQualityFactorRangeMin(), m_Data->GetComptonQualityFactorRangeMax(), xNBins, xLog);


  TH1D* Hist = new TH1D("Compton Quality Factor", "Compton Quality Factor", 
                        xNBins, xBins);
  Hist->SetBit(kCanDelete);
  Hist->SetXTitle("Compton Quality Factor");
  Hist->SetYTitle("counts");
  Hist->SetStats(false);
  Hist->SetFillColor(8);
  Hist->SetMinimum(0);


  MPhysicalEvent *Event;
  // Now restart the event-loader:
  if (InitializeEventloader() == false) return;

  MComptonEvent *ComptonEvent; 
  // ... loop over all events and save a count in the belonging bin ...
  while ((Event = m_EventFile->GetNextEvent()) != 0) {

    // Only accept Comptons within the selected ranges... 
    if (m_Selector->IsQualifiedEvent(Event) == false) {
      delete Event;
      continue;
    }
    if (Event->GetEventType() != MPhysicalEvent::c_Compton) {
      delete Event;
      continue;
    }
    ComptonEvent = dynamic_cast<MComptonEvent*>(Event);

    Hist->Fill(ComptonEvent->ComptonQualityFactor1());
    
    delete Event;
  }
  m_EventFile->Close();

  TCanvas* Canvas = new TCanvas("Canvas CQF", "Canvas CQF", 800, 600);
  Canvas->cd();
  if (xLog == true) {
    Canvas->SetLogx();
  }
  Hist->Draw();
  Canvas->Update();

  delete [] xBins;

  return;
}


////////////////////////////////////////////////////////////////////////////////


void MInterfaceMimrec::TrackQualityFactor()
{
  // Distribution of the Track quality factor

  bool xLog;
  if (m_Data->GetTrackQualityFactorRangeMin() > 0 && m_Data->GetTrackQualityFactorRangeMin() < 0.01) {
    xLog = true;
  } else {
    xLog = false;
  }
  int xNBins = 50;
  double* xBins = CreateAxisBins(m_Data->GetTrackQualityFactorRangeMin(), m_Data->GetTrackQualityFactorRangeMax(), xNBins, xLog);


  TH1D* Hist = new TH1D("Track Quality Factor", "Track Quality Factor", 
                        xNBins, xBins);
  Hist->SetBit(kCanDelete);
  Hist->SetXTitle("Track Quality Factor");
  Hist->SetYTitle("counts");
  Hist->SetStats(false);
  Hist->SetFillColor(8);
  Hist->SetMinimum(0);


  MPhysicalEvent* Event;
  // Now restart the event-loader:
  if (InitializeEventloader() == false) return;

  MComptonEvent* ComptonEvent; 
  // ... loop over all events and save a count in the belonging bin ...
  while ((Event = m_EventFile->GetNextEvent()) != 0) {

    // Only accept Comptons within the selected ranges... 
    if (m_Selector->IsQualifiedEvent(Event) == false) {
      delete Event;
      continue;
    }
    if (Event->GetEventType() != MPhysicalEvent::c_Compton) {
      delete Event;
      continue;
    }
    ComptonEvent = dynamic_cast<MComptonEvent*>(Event);

    Hist->Fill(ComptonEvent->TrackQualityFactor1());
    
    delete Event;
  }
  m_EventFile->Close();

  TCanvas* Canvas = new TCanvas("CanvasTQF", "TCQF Canvas", 800, 600);
  Canvas->cd();
  if (xLog == true) {
    Canvas->SetLogx();
  }
  Hist->Draw();
  Canvas->Update();

  delete [] xBins;

  return;
}


////////////////////////////////////////////////////////////////////////////////


void MInterfaceMimrec::EarthCenterDistance()
{

  mimp<<"Fixed earth position at (0, 0, -1e20) = 180 deg"<<endl;

  MVector Position(0, 0, -c_FarAway);

  MComptonEvent* Compton = 0;
  MPhysicalEvent* Event = 0;
  if (InitializeEventloader() == false) return;

  int NEvents = 0;
  int NBins = 100;
  
  TH1D* Hist = 
    new TH1D("EarthCenterDistance", 
             "Distance between cone circle and earth center", NBins, 
             0.0, 180.0);
  Hist->SetBit(kCanDelete);
  Hist->SetXTitle("Distance between cone circle and earth center [deg]");
  Hist->SetYTitle("counts");
  Hist->SetStats(false);
  Hist->SetFillColor(8);
  Hist->SetMinimum(0);

 // First check on the size of the histogram:
  while ((Event = m_EventFile->GetNextEvent()) != 0) {

    // Only accept Comptons within the selected ranges... 
    if (m_Selector->IsQualifiedEvent(Event) == true &&
        Event->GetEventType() == MPhysicalEvent::c_Compton) {
      Compton = (MComptonEvent*) Event;
      Hist->Fill(Compton->GetARMGamma(Position)*c_Deg);
      NEvents++;
    }    
    delete Event;
  }
  m_EventFile->Close();

  cout<<"NEvents: "<<NEvents<<endl;

  if (Hist->GetEntries() == 0) {
    mgui<<"No events passed the event selections"<<info;
    return;
  }

  TCanvas* Canvas = new TCanvas("EarthCenterDistance", "Earth Center Distance", 800, 600);
  Canvas->cd();
  Hist->Draw();
  Canvas->Update();


}


////////////////////////////////////////////////////////////////////////////////


void MInterfaceMimrec::DistanceDistribution()
{
  // Distribution of the distance between the first and the second intercation
  // as well as the minimum distance between any interaction

  int NBins = 100;

  MComptonEvent* Compton = 0;
  MPhysicalEvent* Event = 0;
  if (InitializeEventloader() == false) return;

  TH1D* FirstHist = 
    new TH1D("FirstDistance", 
             "Distance between first and second hit", NBins, 
             m_Data->GetFirstDistanceRangeMin(), m_Data->GetFirstDistanceRangeMax());
  FirstHist->SetBit(kCanDelete);
  FirstHist->SetXTitle("distance [cm]");
  FirstHist->SetYTitle("counts");
  FirstHist->SetStats(false);
  FirstHist->SetFillColor(8);
  FirstHist->SetMinimum(0);

  TH1D* AnyHist = new TH1D("AnyDistance", "Minimum distance between any hit", NBins, 
                           m_Data->GetDistanceRangeMin(), m_Data->GetDistanceRangeMax());
  AnyHist->SetBit(kCanDelete);
  AnyHist->SetXTitle("distance [cm]");
  AnyHist->SetYTitle("counts");
  AnyHist->SetStats(false);
  AnyHist->SetFillColor(8);
  AnyHist->SetMinimum(0);

  int NEvents = 0;

  // First check on the size of the histogram:
  while ((Event = m_EventFile->GetNextEvent()) != 0) {

    // Only accept Comptons within the selected ranges... 
    if (m_Selector->IsQualifiedEvent(Event) == true &&
        Event->GetEventType() == MPhysicalEvent::c_Compton) {
      Compton = (MComptonEvent*) Event;
      FirstHist->Fill((Compton->C2()-Compton->C1()).Mag());
      AnyHist->Fill(Compton->LeverArm());
      NEvents++;
    }    
    delete Event;
  }
  m_EventFile->Close();

  cout<<"NEvents: "<<NEvents<<endl;

  if (FirstHist->GetEntries() == 0 && AnyHist->GetEntries() == 0) {
    mgui<<"No events passed the event selections"<<info;
    return;
  }

  TCanvas* FirstCanvas = new TCanvas("FirstDistanceCanvas", "FirstDistanceCanvas", 800, 600);
  FirstCanvas->cd();
  FirstHist->Draw();
  FirstCanvas->Update();

  TCanvas* AnyCanvas = new TCanvas("AnyDistanceCanvas", "AnyDistanceCanvas", 800, 600);
  AnyCanvas->cd();
  AnyHist->Draw();
  AnyCanvas->Update();

  mout<<"Mean distance between first and second hit: "<<FirstHist->GetMean()<<endl;
  mout<<"Mean distance between any hit: "<<AnyHist->GetMean()<<endl;


  return;
}


////////////////////////////////////////////////////////////////////////////////


void MInterfaceMimrec::SequenceLengths()
{
  // Distribution of the distance between the first and the second intercation
  // as well as the minimum distance between any interaction

  unsigned int MaxTrackLength = 3;
  unsigned int MaxSequenceLength = 3;
  int NBins = 100;

  MComptonEvent* Compton = 0;
  MPhysicalEvent* Event = 0;
  if (InitializeEventloader() == false) return;

  TH1D* TrackHist = 
    new TH1D("LengthFirstTrack", 
             "Length of first Compton electron track", NBins, 
             0.5, NBins+0.5);
  TrackHist->SetBit(kCanDelete);
  TrackHist->SetXTitle("Reconstructed electron track length");
  TrackHist->SetYTitle("counts");
  TrackHist->SetStats(false);
  TrackHist->SetFillColor(8);
  TrackHist->SetMinimum(0);

  TH1D* ComptonHist = new TH1D("LengthComptonSequence", 
                               "Length of reconstructed Compton sequence", NBins, 
                           0.5, NBins+0.5);
  ComptonHist->SetBit(kCanDelete);
  ComptonHist->SetXTitle("Reconstructed sequence length");
  ComptonHist->SetYTitle("counts");
  ComptonHist->SetStats(false);
  ComptonHist->SetFillColor(8);
  ComptonHist->SetMinimum(0);

  int NEvents = 0;

  double TracksGt2 = 0.0;
  int NTracksGt2 = 0;

  // First check on the size of the histogram:
  while ((Event = m_EventFile->GetNextEvent()) != 0) {

    // Only accept Comptons within the selected ranges... 
    if (m_Selector->IsQualifiedEvent(Event) == true &&
        Event->GetEventType() == MPhysicalEvent::c_Compton) {
      Compton = (MComptonEvent*) Event;
      if (Compton->TrackLength() > MaxTrackLength) MaxTrackLength = Compton->TrackLength();
      TrackHist->Fill(Compton->TrackLength());
      if (Compton->TrackLength() >= 2) {
        NTracksGt2++;
        TracksGt2 += Compton->TrackLength();
      }
      if (Compton->SequenceLength() > MaxSequenceLength) MaxSequenceLength = Compton->SequenceLength();
      ComptonHist->Fill(Compton->SequenceLength());
      NEvents++;
    }    
    delete Event;
  }
  m_EventFile->Close();


  if (TrackHist->GetEntries() == 0 && ComptonHist->GetEntries() == 0) {
    mgui<<"No events passed the event selections"<<info;
    return;
  }

  TCanvas* TrackCanvas = 
    new TCanvas("CanvasLengthFirstTrack", 
                "Canvas of length of first Compton electron track", 800, 600);
  TrackCanvas->cd();
  TrackHist->SetAxisRange(0, MaxTrackLength);
  TrackHist->Draw("bar1");
  TrackCanvas->Update();

  TCanvas* ComptonCanvas = 
    new TCanvas("CanvasLengthComptonSequence", 
                "Canvas of length of reconstructed Compton sequence", 800, 600);
  ComptonCanvas->cd();
  ComptonHist->SetAxisRange(0, MaxSequenceLength);
  ComptonHist->Draw("bar1");
  ComptonCanvas->Update();

  mout<<"Track lengths:"<<endl;
  int All = 0;
  for (int b = 1; b <= TrackHist->GetNbinsX() && b <= 15; ++b) {
    All += int(TrackHist->GetBinContent(b));
    mout<<"TL "<<b<<":"<<TrackHist->GetBinContent(b)<<endl;
  }
  mout<<"Tracked: "<<All - TrackHist->GetBinContent(1)<<endl;
  mout<<"Not tracked: "<<TrackHist->GetBinContent(1)<<endl;

  mout<<"Sequence lengths:"<<endl;
  for (int b = 1; b <= ComptonHist->GetNbinsX() && b <= 10; ++b) {
    mout<<"CL "<<b<<":"<<ComptonHist->GetBinContent(b)<<endl;
  }

  mout<<"Average track length:            "<<TrackHist->GetMean()<<endl;
  if (NTracksGt2 > 0) {
    mout<<"Average track length of tracks:  "<<TracksGt2/NTracksGt2<<endl;
  }
  mout<<"Average sequence length:         "<<ComptonHist->GetMean()<<endl;


  return;
}


////////////////////////////////////////////////////////////////////////////////


// void MInterfaceMimrec::EnergyDistributionD2()
// {
//   // Display the energy-spectrum of all acceptable events 
//   // (remember: not all of them will be reconstructable)

//   const int NBins = 50;
//   int Bin, NEvents = 0;
//   double EnergyMin;
//   EnergyMin = GetTotalEnergyMin();
//   double EnergyMax;
//   EnergyMax = GetTotalEnergyMax();
  
//   double *EnergyArray = new double[NBins];
//   for (Bin = 0; Bin < NBins; Bin++) {
//     EnergyArray[Bin] = 0.0;
//   }
//   double BinWidth = 
//     (EnergyMax - EnergyMin)/NBins;

//   MPhysicalEvent* Event;
//   MComptonEvent Compton;

//   // Start the event loader...
//   if (InitializeEventloader() == false) return;

//   // ... loop over all events and save a count in the belonging bin ...
//   while ((Event = m_EventFile->GetNextEvent()) != 0) {

//     if (m_Selector->IsQualifiedEvent(Event) == false) {
//       delete Event;
//       continue;
//     }

//     if (Event->GetEventType() == MPhysicalEvent::c_Compton) {
//       Compton.Assimilate(dynamic_cast<MComptonEvent*>(Event));
//     }

//     Bin = int ((Compton.Eg() - EnergyMin)/BinWidth);
//     if (Bin >= 0 && Bin < NBins) {
//       EnergyArray[Bin] += 1.0;
//       NEvents++;
//     }

//     delete Event;
//   }
	
//   // ... and display it.
//   m_Display.DisplayHistogram("Measured energy distribution D2", "Energy [keV]", "Number of counts", 
//                              EnergyArray, EnergyMin, EnergyMax, NBins);

//   cout<<"NEvents: "<<NEvents<<endl;

//   delete [] EnergyArray;
// }


////////////////////////////////////////////////////////////////////////////////


void MInterfaceMimrec::TimeDistribution()
{
  // Time distribution in the data-set

  int NBins = 1000;

  MPhysicalEvent* Event = 0;
  if (InitializeEventloader() == false) return;

  TH1D* Hist = 
    new TH1D("Time", "Time", NBins, 
             m_Data->GetTimeRangeMin(), m_Data->GetTimeRangeMax());
  Hist->SetBit(kCanDelete);
  Hist->SetXTitle("time [s]");
  Hist->SetYTitle("counts");
  Hist->SetStats(false);
  Hist->SetFillColor(8);
  Hist->SetMinimum(0);

  double MinTime = numeric_limits<double>::max();
  double MaxTime = 0;

  vector<double> TimeList;
  MEventSelector NoTimeWindowSelector = *m_Selector;
  NoTimeWindowSelector.SetTime(0, numeric_limits<double>::max());

  // First check on the size of the histogram:
  while ((Event = m_EventFile->GetNextEvent()) != 0) {

    if (Event->GetTime().GetAsDouble() < MinTime) MinTime = Event->GetTime().GetAsDouble();
    if (Event->GetTime().GetAsDouble() > MaxTime) MaxTime = Event->GetTime().GetAsDouble();

    // Only accept Comptons within the selected ranges... 
    if (m_Selector->IsQualifiedEvent(Event) == true) {
      Hist->Fill(Event->GetTime().GetAsDouble());
    }    
    // Only accept Comptons within the selected ranges... 
    if (NoTimeWindowSelector.IsQualifiedEvent(Event) == true) {
      TimeList.push_back(Event->GetTime().GetAsDouble());
    }    
    delete Event;
  }
  m_EventFile->Close();

  mout<<"Minimum time: "<<setprecision(20)<<MinTime<<endl;
  mout<<"Maximum time: "<<setprecision(20)<<MaxTime<<setprecision(6)<<endl;

  if (Hist->GetIntegral() == 0) {
    mgui<<"No events passed the event selections"<<info;
    return;
  }

  TCanvas* Canvas = new TCanvas("Time", "Time", 800, 600);
  Canvas->cd();
  Hist->Draw();
  Canvas->Update();


  TH1D* HistOptimized = 
    new TH1D("TimeOptimized", "Time (optimized time window without the cuts in the event selector)", NBins, 
             MinTime, MaxTime);
  HistOptimized->SetBit(kCanDelete);
  HistOptimized->SetXTitle("time [s]");
  HistOptimized->SetYTitle("counts");
  HistOptimized->SetStats(false);
  HistOptimized->SetFillColor(8);
  HistOptimized->SetMinimum(0);

  for (unsigned int i = 0; i < TimeList.size(); ++i) {
    HistOptimized->Fill(TimeList[i]);
  }

  TCanvas* CanvasOptimized = new TCanvas("TimeOptimized", "Time (optimized time window)", 800, 600);
  CanvasOptimized->cd();
  HistOptimized->Draw();
  CanvasOptimized->Update();


  return; 
}


////////////////////////////////////////////////////////////////////////////////


void MInterfaceMimrec::CoincidenceWindowDistribution()
{
  // Time distribution in the data-set

  int NBins = 100;

  MPhysicalEvent* Event = 0;
  if (InitializeEventloader() == false) return;

  TH1D* Hist = 
    new TH1D("CoincidenceWindow", "Coincidence window", NBins, 
             m_Data->GetCoincidenceWindowRangeMin(), m_Data->GetCoincidenceWindowRangeMax());
  Hist->SetBit(kCanDelete);
  Hist->SetXTitle("time [s]");
  Hist->SetYTitle("counts");
  Hist->SetStats(false);
  Hist->SetFillColor(8);
  Hist->SetMinimum(0);

  double MinTime = numeric_limits<double>::max();
  double MaxTime = 0;

  vector<double> TimeList;
  MEventSelector NoTimeWindowSelector = *m_Selector;
  NoTimeWindowSelector.SetTime(0, numeric_limits<double>::max());

  // First check on the size of the histogram:
  MComptonEvent* ComptonEvent;
  while ((Event = m_EventFile->GetNextEvent()) != 0) {
    if (Event->GetEventType() == MPhysicalEvent::c_Compton) {
      ComptonEvent = dynamic_cast<MComptonEvent*>(Event);

      if (ComptonEvent->CoincidenceWindow().GetAsDouble() < MinTime) MinTime = ComptonEvent->CoincidenceWindow().GetAsDouble();
      if (ComptonEvent->CoincidenceWindow().GetAsDouble() > MaxTime) MaxTime = ComptonEvent->CoincidenceWindow().GetAsDouble();

      // Only accept Comptons within the selected ranges... 
      if (m_Selector->IsQualifiedEvent(ComptonEvent) == true) {
        Hist->Fill(ComptonEvent->CoincidenceWindow().GetAsDouble());
      }    
      // Only accept Comptons within the selected ranges... 
      if (NoTimeWindowSelector.IsQualifiedEvent(ComptonEvent) == true) {
        TimeList.push_back(ComptonEvent->CoincidenceWindow().GetAsDouble());
      }
    }
    delete Event;
  }
  m_EventFile->Close();

  mout<<"Minimum coincidence window: "<<setprecision(20)<<MinTime<<endl;
  mout<<"Maximum coincidence window: "<<setprecision(20)<<MaxTime<<setprecision(6)<<endl;

  if (Hist->GetIntegral() == 0) {
    mgui<<"No events passed the event selections"<<info;
    return;
  }

  TCanvas* Canvas = new TCanvas("CoincidenceWindow", "Coincidence window", 800, 600);
  Canvas->cd();
  Hist->Draw();
  Canvas->Update();


  TH1D* HistOptimized = 
    new TH1D("OptimizedCoincidenceWindow", "Coincidence window (optimized window without the cuts in the event selector)", NBins, MinTime, MaxTime);
  HistOptimized->SetBit(kCanDelete);
  HistOptimized->SetXTitle("time [s]");
  HistOptimized->SetYTitle("counts");
  HistOptimized->SetStats(false);
  HistOptimized->SetFillColor(8);
  HistOptimized->SetMinimum(0);

  for (unsigned int i = 0; i < TimeList.size(); ++i) {
    HistOptimized->Fill(TimeList[i]);
  }

  TCanvas* CanvasOptimized = new TCanvas("OptimizedCoincidenceWindow", "Coincidence window (optimized)", 800, 600);
  CanvasOptimized->cd();
  HistOptimized->Draw();
  CanvasOptimized->Update();


  return; 
}


////////////////////////////////////////////////////////////////////////////////


void MInterfaceMimrec::LocationOfFirstIA()
{
  // Display the location of the first compton interaction

  int x, y;
  int x1NBins = 100;
  int x2NBins = 100;
  double x1Min = -0.7, x1Max = 0.7;
  double x2Min = -0.7, x2Max = 0.7;
  double x1BinWidth = (x1Max-x1Min)/x1NBins;
  double x2BinWidth = (x2Max-x2Min)/x2NBins;

  double* Array = new double[x1NBins*x2NBins];
  for (x = 0; x < x1NBins*x2NBins; x++) Array[x] = 0.0;

  // Start the Event loader
  if (InitializeEventloader() == false) return;

  // ... loop over all events and save a count in the belonging bin ...
  MPhysicalEvent* Event;
  MComptonEvent Compton;
  while ((Event = m_EventFile->GetNextEvent()) != 0) {

    if (m_Selector->IsQualifiedEvent(Event) == false) {
      delete Event;
      continue;
    }

    if (Event->GetEventType() == MPhysicalEvent::c_Compton) Compton.Assimilate(dynamic_cast<MComptonEvent*>(Event));

    //cout<<Compton.GetSecondComptonIA().X()<<"!"<<Compton.GetSecondComptonIA().Y()<<endl;
    x = (int) ((Compton.C2().X() - x1Min)/x1BinWidth);
    y = (int) ((Compton.C2().Y() - x2Min)/x2BinWidth);

    //cout<<x<<"!"<<y<<endl;
    if (x >= 0 && x < x1NBins && y >= 0 && y < x2NBins) {
      Array[x + y*x1NBins] += 1;
    }

    delete Event;
  } 


  MImage2D* Image = 
    new MImage2D("Location of Second Interaction", Array,
                 "x [m]", x1Min, x1Max, x1NBins,
                 "y [m]", x2Min, x2Max, x2NBins);

  Image->Display();

  delete [] Array;

  return;
}


// ////////////////////////////////////////////////////////////////////////////////


// void MInterfaceMimrec::DOM()
// {
//   int Bin, NBins = 50;
//   int NEvents = 0;
//   double *ARM = new double[NBins];
//   for (Bin = 0; Bin < NBins; Bin++) ARM[Bin] = 0.0;
//   double BinWidth;

//   double x, y, z = 10000000.0, Disk;
  
//   // Get the data of the ARM-"Test"-Position
//   if (m_Data->GetCoordinateSystem() == MProjection::c_Spheric) {  // spheric 
//     x = m_Data->GetTPTheta();
//     y = m_Data->GetTPPhi();
//     Disk = 180;

//     // All the external data stuff is in galactic coodinates, but the library uses
//     // spherical, so transform it
//     MMath::GalacticToSpheric(x, y);
//     MMath::SphericToCartesean(x, y, z);
//   } else if (m_Data->GetCoordinateSystem() == MProjection::c_Cartesian2D ||
//              m_Data->GetCoordinateSystem() == MProjection::c_Cartesian3D) {
//     x = m_Data->GetTPX();
//     y = m_Data->GetTPY();
//     z = m_Data->GetTPZ();
//     Disk = 180; //
//   } else {
//     merr<<"Unknown coordinate system ID: "<<m_Data->GetCoordinateSystem()<<fatal;
//   }
  
//   // Initalize the image size (x-axis)
//   BinWidth = 2*Disk/NBins;

//   // Now restart the event-loader:
//   if (InitializeEventloader() == false) return;

//   // And fill the ARM-vector:

//   MPhysicalEvent* Event;
//   MComptonEvent ComptonEvent;
//   MVector Dir;
//   // ... loop over all events and save a count in the belonging bin ...
//   while ((Event = m_EventFile->GetNextEvent()) != 0) {

//     if (m_Selector->IsQualifiedEvent(Event) == false) {
//       delete Event;
//       continue;
//     }

//     if (Event->GetEventType() == MPhysicalEvent::c_Compton) {
//       ComptonEvent.Assimilate(Event);

//       if (ComptonEvent.GetARMGamma(MVector(x, y, z))*c_Deg < 5) {
//         Dir = ComptonEvent.De();
//         Bin = int ((Dir.Theta()*c_Deg + Disk)/BinWidth);
//       }
//     } else {
//       delete Event;
//       continue;
//     }
//     if (Bin >= 0 && Bin < NBins) {
//       ARM[Bin] += 1;
//       NEvents++;
//     }
//     delete Event;
//   } 

//   m_Display.DisplayHistogram("Zenith angle of electron direction", 
//                              "Zenith angle [deg]", "Number of events", 
//                              ARM, -Disk, Disk, NBins);

//   cout<<"ARM: Inside disk = "<<NEvents<<endl;

//   return;
// }


////////////////////////////////////////////////////////////////////////////////


void MInterfaceMimrec::Polarization()
{
  // Checks for Polarization...

  // Initalize the image size (x-axis)
  int NBins = m_Data->GetHistBinsPolarization();

  double Min = -180.000001;
  double Max = +180.000001;
  
  TH1D* Background = new TH1D("Background", "Not polarized source", NBins, Min, Max);
  Background->SetBit(kCanDelete);
  Background->SetXTitle("[degree]");
  Background->SetYTitle("counts/degree");
  Background->SetFillColor(8);
  Background->SetStats(false);
  Background->SetMinimum(0.0);
  Background->SetNdivisions(-508, "X");

  TH1D* Polarization = new TH1D("Polarization", "Polarized source", NBins, Min, Max);
  Polarization->SetBit(kCanDelete);
  Polarization->SetXTitle("[degree]");
  Polarization->SetYTitle("counts/degree");
  Polarization->SetFillColor(8);
  Polarization->SetStats(false);
  Polarization->SetMinimum(0.0);
  Polarization->SetNdivisions(-508, "X");

  TH1D* Corrected = new TH1D("Corrected", "Geometry corrected polarization signature", NBins, Min, Max);
  Corrected->SetBit(kCanDelete);
  Corrected->SetXTitle("[degree]");
  Corrected->SetYTitle("corrected counts/degree");
  Corrected->SetFillColor(8);
  Corrected->SetStats(false);
  Corrected->SetMinimum(0.0);
  Corrected->SetNdivisions(-508, "X");

  MPhysicalEvent* Event = 0;
  MComptonEvent* ComptonEvent = 0; 
  MPairEvent* PairEvent = 0; 

  // Origin in spherical coordinates:
  double Theta = m_Data->GetTPTheta()*c_Rad;
  double Phi = m_Data->GetTPPhi()*c_Rad;

  // Origin in Cartesian Corrdinates:
  MVector Origin;
  Origin.SetMagThetaPhi(c_FarAway, Theta, Phi);

  // Arm cut:
  double ArmCut = m_Data->GetPolarizationArmCut();

  // Some statistics:
  int InsideArmCutSource = 0;
  int OutsideArmCutSource = 0;
  int InsideArmCutBackground = 0;
  int OutsideArmCutBackground = 0;

  // Now restart the event-loader with the polarization file:
  if (InitializeEventloader() == false) return;

  // ... loop over all events and save a count in the belonging bin ...
  while ((Event = m_EventFile->GetNextEvent()) != 0) {

    // Only accept Comptons within the selected ranges...
    if (m_Selector->IsQualifiedEvent(Event) == true) {
      if (Event->GetEventType() == MPhysicalEvent::c_Compton) {
        ComptonEvent = dynamic_cast<MComptonEvent*>(Event);

        if (fabs(ComptonEvent->GetARMGamma(Origin))*c_Deg < ArmCut) {
          MVector Plain = ComptonEvent->Dg();
          Plain.RotateZ(-Phi);
          Plain.RotateY(-Theta);

          double Angle = Plain.Phi();
          Polarization->Fill(Angle*c_Deg);

          InsideArmCutSource++;
        } else {
          OutsideArmCutSource++;
        }
      } else if (Event->GetEventType() == MPhysicalEvent::c_Pair) {
        PairEvent = dynamic_cast<MPairEvent*>(Event);

        if (fabs(PairEvent->GetARMGamma(Origin))*c_Deg < ArmCut) {
          MVector Plain = PairEvent->GetElectronDirection() + 
            PairEvent->GetPositronDirection();
          Plain.RotateZ(-Phi);
          Plain.RotateY(-Theta);
          
          double Angle = Plain.Phi();
          Polarization->Fill(Angle*c_Deg);
          InsideArmCutSource++;
        } else {
          OutsideArmCutSource++;
        }
      }
    }

    delete Event;
  }   
  m_EventFile->Close();

  // Normalize to counts/deg
  for (int b = 1; b <= Polarization->GetNbinsX(); ++b) {
    Polarization->SetBinContent(b, Polarization->GetBinContent(b)/Polarization->GetBinWidth(b));
  }

  
  if (Polarization->Integral() == 0) {
    mgui<<"No events passed the event selections for the polarized data file"<<show;
    return;
  }

  TCanvas* PolarizationCanvas = new TCanvas("PolarizationCanvas", "Polarized source", 800, 600);
  PolarizationCanvas->cd();
  Polarization->Draw();
  PolarizationCanvas->Update();

  for (int b = 1; b <= Polarization->GetNbinsX(); ++b) {
    if (Polarization->GetBinContent(b) == 0) {
      mgui<<"You don't have enough statistics in your source measurement: Some bins are zero, e.g. bin "<<b<<show;
      return;
    }
  }


  // Now restart the event-loader with the background file:
  if (InitializeEventloader(m_Data->GetPolarizationBackgroundFileName()) == false) return;

  // ... loop over all events and save a count in the belonging bin ...
  while ((Event = m_EventFile->GetNextEvent()) != 0) {

    // Only accept Comptons within the selected ranges...
    if (m_Selector->IsQualifiedEvent(Event) == true) {
      if (Event->GetEventType() == MPhysicalEvent::c_Compton) {
        ComptonEvent = dynamic_cast<MComptonEvent*>(Event);

        if (fabs(ComptonEvent->GetARMGamma(Origin))*c_Deg < ArmCut) {
          MVector Plain = ComptonEvent->Dg();
          Plain.RotateZ(-Phi);
          Plain.RotateY(-Theta);

          double Angle = Plain.Phi();
          Background->Fill(Angle*c_Deg);
          InsideArmCutBackground++;
        } else {
          OutsideArmCutBackground++;
        }
      } else if (Event->GetEventType() == MPhysicalEvent::c_Pair) {
        PairEvent = dynamic_cast<MPairEvent*>(Event);

        if (fabs(PairEvent->GetARMGamma(Origin))*c_Deg < ArmCut) {
          MVector Plain = PairEvent->GetElectronDirection() + 
            PairEvent->GetPositronDirection();
          Plain.RotateZ(-Phi);
          Plain.RotateY(-Theta);
          
          double Angle = Plain.Phi();
          Background->Fill(Angle*c_Deg);
          InsideArmCutBackground++;
        } else {
          OutsideArmCutBackground++;
        }
      }
    }

    delete Event;
  }   
  m_EventFile->Close();

  // Normalize to counts/deg
  for (int b = 1; b <= Background->GetNbinsX(); ++b) {
    if (Background->GetBinContent(b) == 0) {
      mgui<<"You don't have enough statistics in your background measurement: Some bins are zero!"<<endl;
      return;
    }
    Background->SetBinContent(b, Background->GetBinContent(b)/Background->GetBinWidth(b));
  }
  
  if (Background->Integral() == 0) {
    mgui<<"No events passed the event selections for the background data file"<<show;
    return;
  }

  TCanvas* BackgroundCanvas = new TCanvas("BackgroundCanvas", "Not polarized source", 800, 600);
  BackgroundCanvas->cd();
  Background->Draw();
  BackgroundCanvas->Update();

  // Normalize to counts/deg
  for (int b = 1; b <= Background->GetNbinsX(); ++b) {
    if (Background->GetBinContent(b) == 0) {
      mgui<<"You don't have enough statistics in your background measurement: Some bins are zero, e.g. bin "<<b<<endl;
      return;
    }
  }

  // Correct the image:
  double Mean = Polarization->Integral()/Corrected->GetNbinsX();

  // The scaling is necessary, since we cannot assume that pol and background have been measured for exactly the same time...
  Mean *= Background->Integral()/Polarization->Integral();

  for (int b = 1; b <= Corrected->GetNbinsX(); ++b) {
    Corrected->SetBinContent(b, Polarization->GetBinContent(b)/Background->GetBinContent(b)*Mean);
  }

  // Try to fit a cosinus
  TCanvas* CorrectedCanvas = new TCanvas("CorrectedCanvas", "Background corrected polarization signature", 800, 600);
  CorrectedCanvas->cd();

  TF1* Lin = new TF1("LinearModulation", "pol0", -180*0.99, 180*0.99);
  Corrected->Fit(Lin, "RQFI");
  double LinChisquare = Lin->GetChisquare();
  int LinNDF = Lin->GetNDF();

  TF1* Mod = new TF1("PolarizationModulation", PolarizationModulation, -180*0.99, 180*0.99, 3);
  Mod->SetParNames("Offset (#)", "Scale (#)", "Shift (deg)");
  Mod->SetParameters(Lin->GetParameter(0), 0.5, 0);
  Mod->SetParLimits(1, 0, 10000000);
  Mod->SetParLimits(2, -400, 400);
  Corrected->Fit(Mod, "RQ");

  Corrected->Draw();
  CorrectedCanvas->Update();

  double Modulation = fabs(Mod->GetParameter(1)/Mod->GetParameter(0));
  double ModulationError = sqrt((Mod->GetParError(1)*Mod->GetParError(1))/(Mod->GetParameter(0)*Mod->GetParameter(0)) + 
                        (Mod->GetParError(0)*Mod->GetParError(0)*Mod->GetParameter(1)*Mod->GetParameter(1))/
                        (Mod->GetParameter(0)*Mod->GetParameter(0)*Mod->GetParameter(0)*Mod->GetParameter(0)));

  double PolarizationAngle = Mod->GetParameter(2);
  double PolarizationAngleError = Mod->GetParError(2);
  while (PolarizationAngle < 0) PolarizationAngle += 180;
  while (PolarizationAngle > 180) PolarizationAngle -= 180;

  double ModChisquare = Mod->GetChisquare();
  int ModNDF = Mod->GetNDF();

  mout<<endl;
  mout<<"Polarization analysis:"<<endl;
  mout<<endl;
  mout<<"Modulation:          "<<Modulation<<"+-"<<ModulationError<<endl;
  mout<<"Polarization angle: ("<<PolarizationAngle<<"+-"<<PolarizationAngleError<<") deg"<<endl;
  mout<<endl;
  mout<<"Sanity checks: "<<endl;
  mout<<"  Cosine modulation fit: Chi-Square="
      <<ModChisquare<<" NDF="
      <<ModNDF<<" -> p="
      <<TMath::Prob(ModChisquare, ModNDF)<<endl;
  mout<<"  Linear fit: Chi-Square="
      <<LinChisquare<<"  NDF="
      <<LinNDF<<" -> p="
      <<TMath::Prob(LinChisquare, LinNDF)<<endl;
  mout<<endl;
  mout<<"  Inside ARM cut (source): "<<InsideArmCutSource<<endl;
  mout<<"  Outside ARM cut (source): "<<OutsideArmCutSource<<endl;
  mout<<"  Inside ARM cut (bkg): "<<InsideArmCutBackground<<endl;
  mout<<"  Outside ARM cut (bkg): "<<OutsideArmCutBackground<<endl;
  mout<<endl;

  delete Lin;
  delete Mod;

  return;
}


////////////////////////////////////////////////////////////////////////////////


void MInterfaceMimrec::AzimuthalComptonScatterAngle()
{
  // Initalize the image size (x-axis)
  int NBins = m_Data->GetHistBinsARMGamma();

  TH1D* Hist = new TH1D("AzimuthalComptonScatterAngle", "Azimuthal Compton Scatter Angle", NBins, -180, 180);
  Hist->SetBit(kCanDelete);
  Hist->SetXTitle("[degree]");
  Hist->SetYTitle("counts/degree");
  Hist->SetFillColor(8);
  Hist->SetStats(false);
  Hist->SetMinimum(0.0);
  Hist->SetNdivisions(-508, "X");


  MPhysicalEvent* Event = 0;
  MComptonEvent* ComptonEvent = 0; 

  // Origin in spherical coordinates:
  double Theta = m_Data->GetTPTheta()*c_Rad;
  double Phi = m_Data->GetTPPhi()*c_Rad;

  // Origin in Cartesian Corrdinates:
  MVector Origin;
  Origin.SetMagThetaPhi(c_FarAway, Theta, Phi);

  double ArmCut = 180;

  // Some statistics:
  int InsideArmCutSource = 0;
  int OutsideArmCutSource = 0;

  // Now restart the event-loader with the polarization file:
  if (InitializeEventloader() == false) return;

  // ... loop over all events and save a count in the belonging bin ...
  while ((Event = m_EventFile->GetNextEvent()) != 0) {

    // Only accept Comptons within the selected ranges...
    if (m_Selector->IsQualifiedEvent(Event) == true) {
      if (Event->GetEventType() == MPhysicalEvent::c_Compton) {
        ComptonEvent = dynamic_cast<MComptonEvent*>(Event);
        if (fabs(ComptonEvent->GetARMGamma(Origin))*c_Deg < ArmCut) {
          MVector Plain = ComptonEvent->Dg();
          Plain.RotateZ(-Phi);
          Plain.RotateY(-Theta);
          
          double Angle = Plain.Phi();
          Hist->Fill(Angle*c_Deg);
          
          InsideArmCutSource++;
          } else {
          OutsideArmCutSource++;
        }
      }
    }

    delete Event;
  }   
  m_EventFile->Close();

  // Normalize to counts/deg
  for (int b = 1; b <= Hist->GetNbinsX(); ++b) {
    Hist->SetBinContent(b, Hist->GetBinContent(b)/Hist->GetBinWidth(b));
  }

  if (Hist->Integral() == 0) {
    mgui<<"No events passed the event selections!"<<show;
    return;
  }

  TCanvas* Canvas = 
    new TCanvas("AzimuthalComptonScatterAngleCanvas", 
                "Azimuthal Compton Scatter Angle Canvas", 800, 600);
  Canvas->cd();
  Hist->Draw();
  Canvas->Update();
}

////////////////////////////////////////////////////////////////////////////////


void MInterfaceMimrec::AzimuthalElectronScatterAngle()
{
  // Initalize the image size (x-axis)
  int NBins = m_Data->GetHistBinsARMElectron();

  TH1D* Hist = new TH1D("AzimuthalElectronScatterAngle", "Azimuthal Electron Scatter Angle", NBins, -180, 180);
  Hist->SetBit(kCanDelete);
  Hist->SetXTitle("[degree]");
  Hist->SetYTitle("counts/degree");
  Hist->SetFillColor(8);
  Hist->SetStats(false);
  Hist->SetMinimum(0.0);
  Hist->SetNdivisions(-508, "X");


  MPhysicalEvent* Event = 0;
  MComptonEvent* ComptonEvent = 0; 

  // Origin in spherical coordinates:
  double Theta = m_Data->GetTPTheta()*c_Rad;
  double Phi = m_Data->GetTPPhi()*c_Rad;

  // Origin in Cartesian Corrdinates:
  MVector Origin;
  Origin.SetMagThetaPhi(c_FarAway, Theta, Phi);

  double ArmCut = 180;

  // Some statistics:
  int InsideArmCutSource = 0;
  int OutsideArmCutSource = 0;

  // Now restart the event-loader with the polarization file:
  if (InitializeEventloader() == false) return;

  // ... loop over all events and save a count in the belonging bin ...
  while ((Event = m_EventFile->GetNextEvent()) != 0) {

    // Only accept Comptons within the selected ranges...
    if (m_Selector->IsQualifiedEvent(Event) == true) {
      if (Event->GetEventType() == MPhysicalEvent::c_Compton) {
        ComptonEvent = dynamic_cast<MComptonEvent*>(Event);
        if (ComptonEvent->HasTrack() == true) {
          if (fabs(ComptonEvent->GetARMGamma(Origin))*c_Deg < ArmCut) {
            MVector Plain = ComptonEvent->De();
            Plain.RotateZ(-Phi);
            Plain.RotateY(-Theta);
          
            double Angle = Plain.Phi();
            Hist->Fill(Angle*c_Deg);
            
            InsideArmCutSource++;
          } else {
            OutsideArmCutSource++;
          }
        }
      }
    }

    delete Event;
  }   
  m_EventFile->Close();

  // Normalize to counts/deg
  for (int b = 1; b <= Hist->GetNbinsX(); ++b) {
    Hist->SetBinContent(b, Hist->GetBinContent(b)/Hist->GetBinWidth(b));
  }

  if (Hist->Integral() == 0) {
    mgui<<"No events passed the event selections!"<<show;
    return;
  }

  TCanvas* Canvas = 
    new TCanvas("AzimuthalElectronScatterAngleCanvas", 
                "Azimuthal Electron Scatter Angle Canvas", 800, 600);
  Canvas->cd();
  Hist->Draw();
  Canvas->Update();
}


////////////////////////////////////////////////////////////////////////////////


void MInterfaceMimrec::OpeningAnglePair()
{
  //

  // Initalize the image size (x-axis)
  int NBins = 100;
  TH1D* Hist = new TH1D("OpeningAnglePair", "Opening Angle Pair", NBins, 
                        m_Data->GetOpeningAnglePairMin(), m_Data->GetOpeningAnglePairMax());
  Hist->SetBit(kCanDelete);
  Hist->SetXTitle("Opening angle [#circ]");
  Hist->SetYTitle("counts");
  Hist->SetStats(false);
  Hist->SetFillColor(8);


  // Now restart the event-loader:
  if (InitializeEventloader() == false) return;

  MPhysicalEvent* Event = 0;
  MPairEvent* PairEvent = 0; 
  // ... loop over all events and save a count in the belonging bin ...
  while ((Event = m_EventFile->GetNextEvent()) != 0) {

    // Only accept Comptons within the selected ranges...
    if (m_Selector->IsQualifiedEvent(Event) == true) {
      if (Event->GetEventType() == MPhysicalEvent::c_Pair) {
        PairEvent = dynamic_cast<MPairEvent*>(Event);
        Hist->Fill(PairEvent->GetOpeningAngle()*c_Deg);
      }
    }

    delete Event;
  }   
  m_EventFile->Close();

  TCanvas* Canvas = new TCanvas("OpeningAnglePair", "Opening Angle Pair", 800, 600);
  Canvas->cd();
  Hist->Draw();
  Canvas->Update();
}


////////////////////////////////////////////////////////////////////////////////


void MInterfaceMimrec::AngularResolutionVsQualityFactorPair()
{
  // Display the angular resolution measurement for the gamma-ray
  // The ARM value for the scattered gamma-ray is the minimum angle between 
  // the gamma-cone-surface and the line connecting the cone-apex with the 
  // (Test-)position

  int NEvents = 0;
  double Value = 0;
  int Inside = 0;
  
  int NBins = m_Data->GetHistBinsARMGamma();
  double Disk = m_Data->GetTPDistanceTrans();
  MVector TestPosition = GetTestPosition();

  // Initalize the image size (x-axis)
  //BinWidth = 2*Disk/NBins;
  TH2D* Hist = new TH2D("Angular resolution pairs", "Angular resolution pairs", NBins, 0, Disk, 100, 0, 1);
  Hist->SetBit(kCanDelete);
  Hist->SetDirectory(0);
  Hist->SetXTitle("Angular resolution pairs [#circ]");
  Hist->SetYTitle("Quality factor");
  Hist->SetStats(false);
  Hist->SetFillColor(8);
  //double BinWidth = 2*Disk/NBins;

  // Now restart the event-loader:
  if (InitializeEventloader() == false) return;

  MPhysicalEvent* Event = 0;
  MPairEvent* PairEvent = 0; 
  // ... loop over all events and save a count in the belonging bin ...
  while ((Event = m_EventFile->GetNextEvent()) != 0) {

    // Only accept Comptons within the selected ranges...
    if (m_Selector->IsQualifiedEvent(Event) == true) {
      if (Event->GetEventType() == MPhysicalEvent::c_Pair) {
        PairEvent = dynamic_cast<MPairEvent*>(Event);

        Value = PairEvent->GetARMGamma(TestPosition);
        if (Value*c_Deg > -Disk && Value*c_Deg < Disk) Inside++;
        Hist->Fill(Value*c_Deg, PairEvent->GetTrackQualityFactor());
        NEvents++;
      }
    }

    delete Event;
  } 

  m_EventFile->Close();

  if (Hist->GetMaximum() == 0) {
    mgui<<"No events passed the event selections or file is empty!"<<endl;
    return;
  }

    
  TCanvas *Canvas = new TCanvas("Canvas angular resolution pairs vs quality factor", "Canvas angular resolution pairs vs quality factor", 800, 600);
  Canvas->SetFillColor(0);
  Canvas->SetFrameBorderSize(0);
  Canvas->SetFrameBorderMode(0);
  Canvas->SetBorderSize(0);
  Canvas->SetBorderMode(0);

  Canvas->cd();
  Hist->Draw("colz");
  Canvas->Update();

  return;
}




////////////////////////////////////////////////////////////////////////////////


void MInterfaceMimrec::SelectIds()
{
  //

  // Now restart the event-loader:
  if (InitializeEventloader() == false) return;

  MString Name = m_EventFile->GetFileName();
  Name.Replace(Name.Length()-4, 4, ".ids");

  ofstream fout;
  fout.open(Name);
  if (fout.is_open() == false) {
    mgui<<"Unable to open file \""<<Name<<"\""<<endl;
    return;
  }

  // ... loop over all events
  MPhysicalEvent* Event = 0;
  while ((Event = m_EventFile->GetNextEvent()) != 0) {

    if (m_Selector->IsQualifiedEvent(Event) == true) {
      fout<<"ID "<<Event->GetId()<<endl;
    }

    delete Event;
  }   
  m_EventFile->Close();

  fout.close();
}


////////////////////////////////////////////////////////////////////////////////


void MInterfaceMimrec::InteractionDepth()
{
  // Calculates the average interaction depth in tracker and calorimeter:

  TH1D* D1 = new TH1D("D1", "D1", 50, 15, 26);
  D1->SetBit(kCanDelete);
  D1->SetXTitle("[cm]");
  D1->SetYTitle("#");
  //D1->SetStats(false);
  D1->SetFillColor(8);


  int ND2 = 0;
  double D2avg = 0;
  double D2min = 15;
  double D2max = 0;
  TH1D* D2 = new TH1D("D2", "D2", 50, D2max, D2min);
  D2->SetBit(kCanDelete);
  D2->SetXTitle("[cm]");
  D2->SetYTitle("#");
  //D2->SetStats(false);
  D2->SetFillColor(8);


  // Now restart the event-loader:
  if (InitializeEventloader() == false) return;

  MPhysicalEvent* Event = 0;
  MComptonEvent* ComptonEvent = 0; 
  // ... loop over all events and save a count in the belonging bin ...
  while ((Event = m_EventFile->GetNextEvent()) != 0) {

    // Only accept Comptons within the selected ranges...
    if (m_Selector->IsQualifiedEvent(Event) == true) {
      if (Event->GetEventType() == MPhysicalEvent::c_Compton) {
        ComptonEvent = dynamic_cast<MComptonEvent*>(Event);
        D1->Fill(ComptonEvent->C1()[2]);
        D2->Fill(ComptonEvent->C2()[2]);
        D2avg += ComptonEvent->C2()[2];
        ND2++;
        if (ComptonEvent->C2()[2] < D2min) D2min = ComptonEvent->C2()[2];
        if (ComptonEvent->C2()[2] > D2max) D2max = ComptonEvent->C2()[2];
      }
    }

    delete Event;
  }   

  TCanvas* D1C = new TCanvas();
  D1C->cd();
  D1->Draw();
  D1C->Update();

  TCanvas* D2C = new TCanvas();
  D2C->cd();
  D2->Draw();
  D2C->Update();

  cout<<"D2min = "<<D2min<<endl;
  cout<<"D2max = "<<D2max<<endl;
  cout<<"D2avg = "<<((ND2 > 0) ? D2avg/ND2-D2min : 0)<<endl;


  m_EventFile->Close();
}


////////////////////////////////////////////////////////////////////////////////


void MInterfaceMimrec::LocationOfInitialInteraction()
{
  if (m_Geometry == 0) {
    mgui<<"You need a loaded geometry for this function"<<error;
    return;
  }
 
  if (InitializeEventloader() == false) return;


  double xMin = -100;
  double xMax = +100;
  double yMin = -100;
  double yMax = +100;
  double zMin = -100;
  double zMax = +100;

  int MaxNBins = 200;

  unsigned int MaxNPositions = 10000000;
  vector<MVector> Positions;

  map<MString, int> DetectorOccupation;


  // Step 1: Accumulate many, many hits:

  MVector Pos;
  MPhysicalEvent* Event = 0;
  while ((Event = m_EventFile->GetNextEvent()) != 0) {

    if (m_Selector->IsQualifiedEvent(Event) == true) {
      if (Event->GetPosition() != g_VectorNotDefined) { // ???? !!!!
        Pos = Event->GetPosition();
        MDVolumeSequence V = m_Geometry->GetVolumeSequence(Pos);
        if (V.GetDetector() != 0) {
          DetectorOccupation[V.GetDetector()->GetName()]++;
          Positions.push_back(Pos);
        }
      } else {
        merr<<"We have a event without a valid position. ID = "<<Event->GetId()<<". Ignoring it..."<<endl;
      }
    }
    
    delete Event;

    if (Positions.size() > MaxNPositions) {
      break;
    }
  }


  // Step 2: Create the histograms

  DetermineAxis(xMin, xMax, yMin, yMax, zMin, zMax, Positions);

  TH3D* xyzHist = new TH3D("SpacialHitDistributionXYZ", 
                        "Spacial hit distribution xyz", 
                        MaxNBins, xMin, +xMax,
                        MaxNBins, yMin, +yMax, 
                        MaxNBins, zMin, +zMax);
  xyzHist->SetBit(kCanDelete);
  xyzHist->GetXaxis()->SetTitle("x [cm]");
  xyzHist->GetYaxis()->SetTitle("y [cm]");
  xyzHist->GetZaxis()->SetTitle("z [cm]");

  TH1D* xHist = new TH1D("SpacialHitDistributionX", 
			 "Spacial hit distribution x", 
			 MaxNBins, xMin, +xMax);
  xHist->SetBit(kCanDelete);
  xHist->GetXaxis()->SetTitle("x [cm]");
  xHist->GetYaxis()->SetTitle("counts");

  TH1D* yHist = new TH1D("SpacialHitDistributionY", 
			 "Spacial hit distribution y", 
			 MaxNBins, yMin, +yMax);
  yHist->SetBit(kCanDelete);
  yHist->GetXaxis()->SetTitle("y [cm]");
  yHist->GetYaxis()->SetTitle("counts");

  TH1D* zHist = new TH1D("SpacialHitDistributionZ", 
			 "Spacial hit distribution z", 
			 MaxNBins, zMin, +zMax);
  zHist->SetBit(kCanDelete);
  zHist->GetXaxis()->SetTitle("z [cm]");
  zHist->GetYaxis()->SetTitle("counts");


  // Step 3: Fill the current events:

  for (unsigned int p = 0; p < Positions.size(); ++p) {
    Pos = Positions[p];

    xyzHist->Fill(Pos[0], Pos[1], Pos[2]);
    xHist->Fill(Pos[0]);
    yHist->Fill(Pos[1]);
    zHist->Fill(Pos[2]);
  }


  // Step 4: Continue filling from file:

  while ((Event = m_EventFile->GetNextEvent()) != 0) {

    if (m_Selector->IsQualifiedEvent(Event) == true) {
      if (Event->GetPosition() != g_VectorNotDefined) {
        Pos = Event->GetPosition();
        MDVolumeSequence V = m_Geometry->GetVolumeSequence(Pos);
        if (V.GetDetector() != 0) {
          DetectorOccupation[V.GetDetector()->GetName()]++;

          xyzHist->Fill(Pos[0], Pos[1], Pos[2]);
          xHist->Fill(Pos[0]);
          yHist->Fill(Pos[1]);
          zHist->Fill(Pos[2]);        
        }
      }
    } 

    delete Event;
  }


  // Step 5: Show the histograms

  TCanvas* xyzCanvas = new TCanvas();
  xyzCanvas->cd();
  xyzHist->Draw();
  xyzCanvas->Update();

  TCanvas* xCanvas = new TCanvas();
  xCanvas->cd();
  xHist->Draw();
  xCanvas->Update();

  TCanvas* yCanvas = new TCanvas();
  yCanvas->cd();
  yHist->Draw();
  yCanvas->Update();

  TCanvas* zCanvas = new TCanvas();
  zCanvas->cd();
  zHist->Draw();
  zCanvas->Update();

  // Dump the info:
  map<MString, int>::iterator Iter;
  mout<<endl;
  mout<<"Detector of the first interaction: "<<endl;
  for (Iter = DetectorOccupation.begin(); Iter != DetectorOccupation.end(); ++Iter) {
    mout<<(*Iter).first<<": "<<(*Iter).second<<endl;
  }
  mout<<endl;

  m_EventFile->Close();
}


////////////////////////////////////////////////////////////////////////////////


void MInterfaceMimrec::PointingInGalacticCoordinates()
{
  // Show the pointing of the instrument in galactic coordinates
  
  double LatMin = 0;   // -90 
  double LatMax = 180; // -90
  unsigned int LatBins = 360;
  double LatBinSize = (LatMax - LatMin)/LatBins;

  double LongMin = -180;
  double LongMax = 180;
  unsigned int LongBins = 720;
  double LongBinSize = (LongMax - LongMin)/LongBins;

  double* Array = new double[LatBins*LongBins];
  for (unsigned int i = 0; i < LatBins*LongBins; ++i) Array[i] = 0;
  
  // Now restart the event-loader:
  if (InitializeEventloader() == false) return;

  MPhysicalEvent* Event = 0;
  // ... loop over all events and save a count in the belonging bin ...
  while ((Event = m_EventFile->GetNextEvent()) != 0) {

    // Only accept some events...
    if (m_Selector->IsQualifiedEvent(Event) == true) {
      int LongBin = int((Event->GetGalacticPointingZAxisLongitude()*c_Deg - LongMin)/LongBinSize);
      int LatBin = int((Event->GetGalacticPointingZAxisLatitude()*c_Deg + 90 - LatMin)/LatBinSize);
      Array[LongBin + LatBin*LongBins] += 1.0;
    }
    
    delete Event;
  }
  m_EventFile->Close();

  // Prepare an MImageGalactic class:
  MImageGalactic* Galactic = 
    new MImageGalactic("Pointing in galactic coordinates", Array,
                       "Longitude", LongMin, LongMax, LongBins, 
                       "Latitude", LatMin-90, LatMax-90, LatBins,
                       m_Data->GetImagePalette(), m_Data->GetImageDrawMode());
  Galactic->SetImageArray(Array);
  Galactic->Display();


  delete [] Array;
  // memory leak: Galactic is never deleted!
}


////////////////////////////////////////////////////////////////////////////////


void MInterfaceMimrec::HorizonInSphericalDetectorCoordinates()
{
  // Show the horizon z axis in spherical detector coordinates
  
  double PhiMin = 0;   // -90 
  double PhiMax = 360; // -90
  unsigned int PhiBins = 360;

  double ThetaMin = 0;
  double ThetaMax = 180;
  unsigned int ThetaBins = 180;

  TH2D* Spherical = new TH2D("Zenith", "Zenith of horizon system in spherical coordinates of detector", PhiBins, PhiMin, PhiMax, ThetaBins, ThetaMin, ThetaMax);
  Spherical->SetXTitle("phi [deg]");
  Spherical->SetYTitle("theta [deg]");
  Spherical->SetBit(kCanDelete);
  Spherical->SetStats(false);
  Spherical->SetFillColor(8);
  Spherical->SetMinimum(0);

  
  // Now restart the event-loader:
  if (InitializeEventloader() == false) return;

  MPhysicalEvent* Event = 0;
  // ... loop over all events and save a count in the belonging bin ...
  while ((Event = m_EventFile->GetNextEvent()) != 0) {

    // Only accept some events...
    if (m_Selector->IsQualifiedEvent(Event) == true) {
      MVector Z = Event->GetHorizonPointingZAxis();
      double phi = Z.Phi()*c_Deg;
      while (phi < 0) phi += 360; 
      while (phi > 360) phi -= 360; 
      Spherical->Fill(phi, Z.Theta()*c_Deg);
    }
    
    delete Event;
  }
  m_EventFile->Close();

  TCanvas* Canvas = new TCanvas("ZenithCanvas", "ZenithCanvas");

  Canvas->cd();
  Spherical->Draw("colz");
  Canvas->Update();
}


////////////////////////////////////////////////////////////////////////////////


void MInterfaceMimrec::StandardAnalysis(double Energy, MVector Position)
{
  cout<<"Standard analysis started"<<endl; 

  MStandardAnalysis A;
  
  A.SetEventSelector(*m_Selector);
  A.SetGeometry(m_Geometry);
  A.SetFileName(m_Data->GetCurrentFileName());
   
  A.SetPosition(Position);
  A.SetEnergy(Energy);
  
  if (A.Analyze() == false) {
    cout<<"Analysis failed"<<endl;
  }
}


////////////////////////////////////////////////////////////////////////////////


void MInterfaceMimrec::TestOnly()
{
  mimp<<"This is doing nothing...!"<<show;

  return;
}


// MInterfaceMimrec: the end...
////////////////////////////////////////////////////////////////////////////////
