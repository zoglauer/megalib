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
#include "Math/MinimizerOptions.h"

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
#include "MCoordinateSystem.h"
#include "MSpectralAnalyzer.h"
#include "MPeak.h"
#include "MIsotope.h"
#include "MStandardAnalysis.h"
#include "MResponse.h"
#include "MResponseGaussian.h"
#include "MResponseGaussianByUncertainties.h"
#include "MResponseConeShapes.h"
#include "MResponsePRM.h"
#include "MResponseEnergyLeakage.h"
#include "MBinnerBayesianBlocks.h"
#include "MARMFitter.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MInterfaceMimrec)
#endif


////////////////////////////////////////////////////////////////////////////////


MInterfaceMimrec::MInterfaceMimrec() : MInterface()
{
  // standard constructor

  m_Imager = nullptr;
  m_Settings = new MSettingsMimrec();
  m_BasicGuiData = dynamic_cast<MSettings*>(m_Settings);

  m_EventFile = nullptr;
  m_Selector = new MEventSelector();

 // m_SingleBackprojection = new double *[4];
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
  Usage<<"      -C --change-configuration <pattern>:"<<endl;
  Usage<<"             Replace any value in the configuration file (-C can be used multiple times)"<<endl;
  Usage<<"             E.g. to replace the standard ARM cut range with 10 degrees, one would set pattern to:"<<endl;
  Usage<<"             -C TestPositions.DistanceTrans=10"<<endl;
  Usage<<"      -d --debug:"<<endl;
  Usage<<"             Use debug mode"<<endl;
  Usage<<"      -h --help:"<<endl;
  Usage<<"             You know the answer..."<<endl;
  Usage<<endl;
  Usage<<"    High level functions:"<<endl;
  Usage<<"      -o --output:"<<endl;
  Usage<<"             For --image, --spectrum, --light-curve, --distances, --scatter-angles, --sequence-length, and --arm-gamma: Save the generated histogram."<<endl;
  Usage<<"             For -x: Save the extracted events"<<endl;
  Usage<<"             If multiple histograms are generated, additional modifiers will be added to the file name"<<endl; 
  Usage<<"      -i --image:"<<endl;
  Usage<<"             Create an image. If the -o option is given then the image is saved to this file."<<endl;
  Usage<<"      -s --spectrum:"<<endl;
  Usage<<"             Create a spectrum. If the -o option is given then the image is saved to this file."<<endl;
  Usage<<"      -a --arm-gamma:"<<endl;
  Usage<<"             Create an ARM plot. If the -o option is given then the image is saved to this file."<<endl;
  Usage<<"         --spd-electron:"<<endl;
  Usage<<"             Create a SPD plot. If the -o option is given then the image is saved to this file."<<endl;
  Usage<<"      -l --light-curve:"<<endl;
  Usage<<"             Create a light curve. If the -o option is given then the image is saved to this file."<<endl;
  Usage<<"      -p --polarization:"<<endl;
  Usage<<"             Perform polarization analysis. If the -o option is given then the image is saved to this file."<<endl;
  Usage<<"         --interaction-distance:"<<endl;
  Usage<<"             Create interaction distance plots. If the -o option is given then the image is saved to this file."<<endl;
  Usage<<"         --save-cfg:"<<endl;
  Usage<<"             Safe the configuration. If the -o option is given then the configuration is saved to this file."<<endl;
  Usage<<"         --scatter-angles:"<<endl;
  Usage<<"             Create the scatter-angle distributions. If the -o option is given then the image is saved to this file."<<endl;
  Usage<<"         --sequence-length:"<<endl;
  Usage<<"             Create the sequence length plots. If the -o option is given then the image is saved to this file."<<endl;
  Usage<<"      -x --extract:"<<endl;
  Usage<<"             Extract events using the given event selection to the file given by -o"<<endl;
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
        Option == "-o" || Option == "--output" ||
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
      if (g_Verbosity < 2) g_Verbosity = 2;
      cout<<"Command-line parser: Use debug mode"<<endl;
    } else if (Option == "--configuration" || Option == "-c") {
      MString FileName = argv[++i];
      if (MFile::Exists(FileName) == false) {
        cout<<"Command-line parser: The configuration file does not exists: "<<FileName<<endl;
        return false;
      }
      m_Settings->Read(FileName);
      cout<<"Command-line parser: Use configuration file "<<m_Settings->GetSettingsFileName()<<endl;
    } else if (Option == "--output" || Option == "-o") {
      m_OutputFileName = argv[++i];
      if (m_OutputFileName.Last('.') == string::npos) {
        m_OutputFileName += ".png";
        cout<<"Command-line parser: Output file name has no file type. Using .png"<<endl;
      }
      cout<<"Command-line parser: Use this output file name "<<m_OutputFileName<<endl;
    }
  }
  
  // Look if we need to change the configuration
  for (int i = 1; i < argc; i++) {
    Option = argv[i];
    if (Option == "--change-configuration" || Option == "-C") {
      if (m_Settings->Change(argv[++i]) == false) {
        cout<<"ERROR: Command-line parser: Unable to change this configuration value: "<<argv[i]<<endl;        
      } else {
        cout<<"Command-line parser: Changing this configuration value: "<<argv[i]<<endl;
      }
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
      MString FileName = argv[++i];
      if (MFile::Exists(FileName) == false) {
        cout<<"Command-line parser: The geometry file does not exists: "<<FileName<<endl;
        return false;
      }
      if (m_Settings->SetGeometryFileName(FileName) == false) {
        cout<<"Command-line parser: The geometry file could not be opened correctly: "<<FileName<<endl;
        return false;
      }
      cout<<"Command-line parser: Use geometry file "<<m_Settings->GetGeometryFileName()<<endl;
    } else if (Option == "--filename" || Option == "-f") {
      MString FileName = argv[++i];
      if (MFile::Exists(FileName) == false) {
        cout<<"Command-line parser: The data file does not exists: "<<FileName<<endl;
        return false;
      }
      if (m_Settings->SetCurrentFileName(FileName) == false) {
        cout<<"Command-line parser: The data file could not be opened correctly: "<<FileName<<endl;
        return false;
      }
      cout<<"Command-line parser: Use file "<<m_Settings->GetCurrentFileName()<<endl;
    } else if (Option == "--special" || Option == "--development") {
      m_Settings->SetSpecialMode(true);
      cout<<"Command-line parser: Activating development extras mode - hope, you know what you are doing..."<<endl;
    }
  }

  // Load the geometry
  if (SetGeometry(m_Settings->GetGeometryFileName(), false) == false) {
    cout<<"Command-line parser: "<<m_Settings->GetGeometryFileName()<<" is no aceptable geometry file!"<<endl;
    cout<<"Command-line parser: Please give a correct geometry file via the -g option."<<endl;
    cout<<"Command-line parser: A universally working geometry for mimrec would be: ${MEGALIB}/resource/examples/geomega/special/Dummy.geo.setup"<<endl;
    return false;
    /*
    if (m_UseGui == true) {
      cout<<"Command-line parser: Trying to start with a dummy geometry..."<<endl;
      m_Settings->SetGeometryFileName(g_MEGAlibPath + "/resource/examples/geomega/special/Dummy.geo.setup");
      if (SetGeometry(m_Settings->GetGeometryFileName(), false) == false) {
        cout<<"Command-line parser: Hmmm, even reading of dummy geometry failed... Bye."<<endl;
        return false;
      }
    } else {
      return false;
    }
    */
  }

  // Now parse all high level options
  for (int i = 1; i < argc; i++) {
    Option = argv[i];
    if (Option == "--spectrum" || Option == "-s") {
      cout<<"Command-line parser: Generating spectrum..."<<endl;  
      EnergySpectra();
      return KeepAlive;
    } else if (Option == "--arm-gamma" || Option == "-a") {
      cout<<"Command-line parser: Generating ARM gamma..."<<endl;
      ARMGamma();
      return KeepAlive;
    } else if (Option == "--spd-electron") {
      cout<<"Command-line parser: Generating SPD gamma..."<<endl;
      SPDElectron();
      return KeepAlive;
    } else if (Option == "--light-curve" || Option == "-l") {
      cout<<"Command-line parser: Generating light curve..."<<endl;  
      LightCurve();
      return KeepAlive;
    } else if (Option == "--polarization" || Option == "-p") {
      cout<<"Command-line parser: Performing polarization analysis..."<<endl;  
      Polarization();
      return KeepAlive;
    } else if (Option == "--scatter-angles") {
      cout<<"Command-line parser: Generating scatter-angles plot..."<<endl;
      ScatterAnglesDistribution();
      return KeepAlive;
    } else if (Option == "--save-cfg") {
      cout<<"Command-line parser: Save the configuration..."<<endl;
      SaveConfiguration(m_OutputFileName != "" ? m_OutputFileName : "NewConfiguration.cfg");
      return KeepAlive;
    } else if (Option == "--interaction-distance") {
      cout<<"Command-line parser: Generating interaction-distance plot..."<<endl;  
      DistanceDistribution();
      return KeepAlive;
    } else if (Option == "--sequence-length") {
      cout<<"Command-line parser: Generating Compton-sequence length plot..."<<endl;  
      SequenceLengths();
      return KeepAlive;
    } else if (Option == "--extract" || Option == "-x") {
      cout<<"Command-line parser: Extracting events..."<<endl;  
      ExtractEvents();
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
      // m_Settings->SetStoreImages(true);
      ShowEventSelections();
      return KeepAlive;
    } else if (Option == "--image" || Option == "-i") {
      cout<<"Command-line parser: Generating image..."<<endl;  
      // m_Settings->SetStoreImages(true);
      Reconstruct();
      return KeepAlive;
    }
  }

   
  // Execute some low level commands
  if (m_UseGui == true) {
    m_Gui = new MGUIMimrecMain(this, m_Settings);
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
//     m_Settings = new MSettingsMimrec(); // == Load default
//     m_BasicGuiData = dynamic_cast<MGUIData*>(m_Settings);
//   } else {
//     m_BasicGuiData = BasicGuiData;
//     m_Settings = dynamic_cast<MSettingsMimrec*>(BasicGuiData);
//   }
// }


////////////////////////////////////////////////////////////////////////////////


bool MInterfaceMimrec::LoadConfiguration(MString FileName)
{  
  // Load the configuration file

  if (m_Settings == 0) {
    m_Settings = new MSettingsMimrec();
    m_BasicGuiData = dynamic_cast<MSettings*>(m_Settings);
    if (m_UseGui == true) {
      m_Gui->SetConfiguration(m_Settings);
    }
  }
  
  m_Settings->Read(FileName);

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MInterfaceMimrec::SaveConfiguration(MString FileName)
{
  // Save the configuration file

  massert(m_Settings != 0);

  m_Settings->Write(FileName);

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
  if (m_Settings->GetCoordinateSystem() == MCoordinateSystem::c_Spheric) {
    Test.SetMagThetaPhi(c_FarAway, m_Settings->GetTPTheta()*c_Rad, m_Settings->GetTPPhi()*c_Rad);
  } else if (m_Settings->GetCoordinateSystem() == MCoordinateSystem::c_Galactic) {
    Test.SetMagThetaPhi(c_FarAway, (m_Settings->GetTPGalLatitude()+90)*c_Rad, m_Settings->GetTPGalLongitude()*c_Rad);
  } else if (m_Settings->GetCoordinateSystem() == MCoordinateSystem::c_Cartesian2D ||
             m_Settings->GetCoordinateSystem() == MCoordinateSystem::c_Cartesian3D) {
    Test.SetXYZ(m_Settings->GetTPX(), m_Settings->GetTPY(), m_Settings->GetTPZ());
  } else {
    merr<<"Unknown coordinate system ID: "<<m_Settings->GetCoordinateSystem()<<fatal;
  }

  return Test;
}


////////////////////////////////////////////////////////////////////////////////


double MInterfaceMimrec::GetTotalEnergyMin()
{
  // Determine the test position of ARM, etc. cuts in various coordinate system

  double Min = numeric_limits<double>::max(); 

  if (m_Settings->GetFirstEnergyRangeMax() > 0) {
    if (m_Settings->GetFirstEnergyRangeMin() < Min) Min = m_Settings->GetFirstEnergyRangeMin();
  }
  if (m_Settings->GetSecondEnergyRangeMax() > 0) {
    if (m_Settings->GetSecondEnergyRangeMin() < Min) Min = m_Settings->GetSecondEnergyRangeMin();
  }
  if (m_Settings->GetThirdEnergyRangeMax() > 0) {
    if (m_Settings->GetThirdEnergyRangeMin() < Min) Min = m_Settings->GetThirdEnergyRangeMin();
  }
  if (m_Settings->GetFourthEnergyRangeMax() > 0) {
    if (m_Settings->GetFourthEnergyRangeMin() < Min) Min = m_Settings->GetFourthEnergyRangeMin();
  }

  return Min;
}


////////////////////////////////////////////////////////////////////////////////


double MInterfaceMimrec::GetTotalEnergyMax()
{
  // Determine the test position of ARM, etc. cuts in various coordinate system

  double Max = 0; 

  if (m_Settings->GetFirstEnergyRangeMax() > 0) {
    if (m_Settings->GetFirstEnergyRangeMax() > Max) Max = m_Settings->GetFirstEnergyRangeMax();
  }
  if (m_Settings->GetSecondEnergyRangeMax() > 0) {
    if (m_Settings->GetSecondEnergyRangeMax() > Max) Max = m_Settings->GetSecondEnergyRangeMax();
  }
  if (m_Settings->GetThirdEnergyRangeMax() > 0) {
    if (m_Settings->GetThirdEnergyRangeMax() > Max) Max = m_Settings->GetThirdEnergyRangeMax();
  }
  if (m_Settings->GetFourthEnergyRangeMax() > 0) {
    if (m_Settings->GetFourthEnergyRangeMax() > Max) Max = m_Settings->GetFourthEnergyRangeMax();
  }

  return Max;
}


////////////////////////////////////////////////////////////////////////////////


void MInterfaceMimrec::Reconstruct(bool Animate)
{
  // Main reconstruction routine

  bool JustDeconvolve = false;
  bool JustShowImage = false;

  if (Animate == false && // In case of animation we always perform backprojections
      m_Settings->IsEventSelectionModified() == false &&
      m_Settings->IsBackprojectionModified() == false) {
    if (m_Settings->IsLikelihoodModified() == true) {
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
    m_Imager = new MImager(m_Settings->GetCoordinateSystem());
    m_Imager->SetGeometry(m_Geometry);
    
    if (m_Imager->SetImagingSettings(m_Settings) == false) {
      mgui<<"Unable to set all imaging settings"<<error;
      return;
    }
 
    // Animations are OFF by default when set via SetImageSettings
    if (Animate == true) {
      m_Imager->SetAnimationMode(m_Settings->GetAnimationMode());
      m_Imager->SetAnimationFrameTime(m_Settings->GetAnimationFrameTime());
      m_Imager->SetAnimationFileName(m_Settings->GetAnimationFileName());
    }

    // The tra file name
    if (m_Imager->SetFileName(m_Settings->GetCurrentFileName(), m_Settings->GetFastFileParsing()) == false) {
      return;
    }

    // A new event selector:
    MEventSelector S;
    S.SetSettings(m_Settings);
    S.SetGeometry(m_Geometry);
    m_Imager->SetEventSelector(S);
  }

  if (JustShowImage == false) {

    if (m_Imager->SetDeconvolutionSettings(m_Settings) == false) {
      mgui<<"Unable to set all deconvolution settings"<<error;
      return;
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

  if (m_OutputFileName.IsEmpty() == false) {
    if (m_Imager->GetNImages() > 0) {
      m_Imager->GetImage(m_Imager->GetNImages() - 1)->Display();
      m_Imager->GetImage(m_Imager->GetNImages() - 1)->SaveAs(m_OutputFileName);
    }
  }


  // Reset the modification flags in the GUI data:
  m_Settings->ResetEventSelectionModified();
  m_Settings->ResetBackprojectionModified();
  m_Settings->ResetLikelihoodModified();

  return;
}

////////////////////////////////////////////////////////////////////////////////

  
void MInterfaceMimrec::SpectralAnalyzer()
{
  //! The spectral analyzer

  // Start with the event file loader first (just in case something goes wrong here)
  if (InitializeEventLoader() == false) return;


  MSpectralAnalyzer S;
  
  
  // Fill the initial histogram:
  S.SetGeometry(m_Geometry);
  S.SetSpectrum(1000, GetTotalEnergyMin(), GetTotalEnergyMax());

  // A new event selector:
  MEventSelector Sel;
  Sel.SetSettings(m_Settings);
  Sel.SetGeometry(m_Geometry);

  MPhysicalEvent* Event = nullptr;
  while ((Event = GetNextEvent()) != 0) {
    if (m_Selector->IsQualifiedEventFast(Event) == true) {
      S.FillSpectrum(Event->GetEnergy());
    }
    
    delete Event;
  } 
  
  // Close the event loader
  FinalizeEventLoader();
  
  // Set the GUI options
 
  // peak search

  S.SetSignaltoNoiseRatio(m_Settings->GetSpectralSignaltoNoiseRatio());
  S.SetPoissonLimit(m_Settings->GetSpectralPoissonLimit());
  
  // Isotope Selection
  S.SetIsotopeFileName(m_Settings->GetSpectralIsotopeFileName());
  S.SetEnergyRange(m_Settings->GetSpectralEnergyRange());
  
  // Do the analysis
  if (S.FindIsotopes() == true) {
//    cout<<"Found "<<S.GetNIsotopes()<<" isotopes."<<endl;
  }
}


////////////////////////////////////////////////////////////////////////////////

  
bool MInterfaceMimrec::InitializeEventLoader(MString File)
{
  // Start the event loader...

  if (File.IsEmpty() == true) {
    File = m_Settings->GetCurrentFileName();
  }
  
  if (m_EventFile != nullptr) delete m_EventFile;
  m_EventFile = new MFileEventsTra();
  
  m_EventFile->SetFastFileParsing(m_Settings->GetFastFileParsing());
  if (m_EventFile->Open(File) == false) return false;
  m_EventFile->ShowProgress(m_UseGui);
  if (m_Settings->GetNThreads() > 1) {
    m_EventFile->StartThread();
  }
  
  m_Selector->Reset();
  m_Selector->SetGeometry(m_Geometry);
  m_Selector->SetSettings(m_Settings);

  return true;
}

////////////////////////////////////////////////////////////////////////////////

    
MPhysicalEvent* MInterfaceMimrec::GetNextEvent(bool Checks)
{
  // Get the next event

  if (Checks == true) {
    if (m_EventFile == nullptr) return nullptr;
    if (m_EventFile->IsOpen() == false) return nullptr;
  }
  
  return m_EventFile->GetNextEvent();
}


////////////////////////////////////////////////////////////////////////////////

    
void MInterfaceMimrec::FinalizeEventLoader()
{
  // Close the event loader

  if (m_EventFile != nullptr) {
    if (m_EventFile->IsOpen() == true) {
      m_EventFile->Close();
    }
    delete m_EventFile;
    m_EventFile = nullptr;
  }
}


////////////////////////////////////////////////////////////////////////////////

  
void MInterfaceMimrec::ShowEventSelections()
{
  // Show how many events pass the event selections

  // Start with the event file loader first (just in case something goes wrong here)
  if (InitializeEventLoader() == false) return;
  
  int NEvents = 0;
  int NGoodEvents = 0;

  MPhysicalEvent* Event = nullptr;
  while ((Event = GetNextEvent()) != 0) {
    NEvents++;
    //cout<<Event->GetType()<<": "<<Event->ToString()<<endl;
    if (m_Selector->IsQualifiedEvent(Event, true) == true) {
      NGoodEvents++;
    }
    
    delete Event;
  } 
  
  // Close the event loader
  FinalizeEventLoader();
  
  cout<<endl;
  cout<<endl;
  cout<<"Event selections:"<<endl;
  cout<<endl;
  cout<<"All events  .................... "<<NEvents<<endl;
  cout<<"Not rejected events  ........... "<<NGoodEvents<<endl;
  cout<<endl;
  
  cout<<m_Selector->ToString()<<endl;
}


////////////////////////////////////////////////////////////////////////////////

  
void MInterfaceMimrec::ShowEventSelectionsStepwise()
{
  // Show how many events pass the event selections
  
  // Start with the event file loader first (just in case something goes wrong here)
  if (InitializeEventLoader() == false) return;

  MEventSelector AllOpen;
  int NAllOpen = 0;
  m_Selector->ApplyGeometry(AllOpen);

  MEventSelector RestrictFirstIADetectors = AllOpen;
  int NRestrictFirstIADetectors = 0;
  m_Selector->ApplyExcludedFirstIADetectors(RestrictFirstIADetectors);
  
  MEventSelector RestrictSecondIADetectors = RestrictFirstIADetectors;
  int NRestrictSecondIADetectors = 0;
  m_Selector->ApplyExcludedSecondIADetectors(RestrictSecondIADetectors);
  
  MEventSelector RestrictEventTypes = RestrictSecondIADetectors;
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

  MPhysicalEvent* Event = nullptr;
  while ((Event = GetNextEvent()) != 0) {
    if (AllOpen.IsQualifiedEvent(Event, false) == true) {
      NAllOpen++;
    }
    if (RestrictFirstIADetectors.IsQualifiedEvent(Event, false) == true) {
      NRestrictFirstIADetectors++;
    }
    if (RestrictSecondIADetectors.IsQualifiedEvent(Event, false) == true) {
      NRestrictSecondIADetectors++;
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
  
  // Close the event loader
  FinalizeEventLoader();

  cout<<endl;
  cout<<endl;
  cout<<"Event selections:                                "<<endl;
  cout<<endl;
  cout<<"No restrictions  ............................... "<<NAllOpen<<endl;
  cout<<"Apply 1st interaction detector restrictions  ... "<<NRestrictFirstIADetectors<<endl;
  cout<<"Apply 2nd interaction detector restrictions  ... "<<NRestrictSecondIADetectors<<endl;
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
    
  // Start with the event file loader first (just in case something goes wrong here)
  if (InitializeEventLoader() == false) return;

  MString FileName = m_OutputFileName;
  if (m_OutputFileName.IsEmpty() == true) {
    // Create a new tra file to which we can write the extracted events
    FileName = m_EventFile->GetFileName();
    if (FileName.EndsWith(".tra")) {
      FileName = FileName.Remove(FileName.Length()-4, 4); // remove final tra
    } else if (FileName.EndsWith(".tra.gz")) {
      FileName = FileName.Remove(FileName.Length()-7, 7); // remove final tra
    }
    FileName += ".extracted.tra";
  } 
  
  MFileEventsTra* OutFile = new MFileEventsTra();
  OutFile->Open(FileName, MFile::c_Write);
  if (OutFile->IsOpen() == false) {
    mgui<<"Unable to open file: "<<FileName<<endl;
    mgui<<"Aborting event extraction"<<show;
    return;
  }
  

  MPhysicalEvent* Event = nullptr;
  while ((Event = GetNextEvent()) != 0) {
    if (m_Selector->IsQualifiedEventFast(Event) == true) {
      OutFile->AddEvent(Event);
    }
    
    delete Event;
  }
  
  // Close the event loader
  FinalizeEventLoader();

  OutFile->Close();

  mout<<"Extraction file created: "<<FileName<<endl;
}


////////////////////////////////////////////////////////////////////////////////

  
void MInterfaceMimrec::ThetaOriginDistribution()
{
  // Show how many events pass the event selections
  
  // Start with the event file loader first (just in case something goes wrong here)
  if (InitializeEventLoader() == false) return;

  TH1D* Hist = new TH1D("ThetaOriginDistribution", 
                        "Theta Origin Distribution", 90, 0, 180);
  Hist->SetBit(kCanDelete);
  Hist->SetDirectory(0);
  Hist->SetXTitle("Zenit angle [#circ]");
  Hist->SetYTitle("counts");
  Hist->SetStats(false);
  Hist->SetFillColor(8);


  MPhysicalEvent* Event = nullptr;
  while ((Event = GetNextEvent()) != 0) {
    
    // Only accept Comptons within the selected ranges...
    if (m_Selector->IsQualifiedEventFast(Event) == true) {
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
  
  // Close the event loader
  FinalizeEventLoader();

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

  // Start with the event file loader first (just in case something goes wrong here)
  if (InitializeEventLoader() == false) return;

  MVector TestPosition = GetTestPosition();

  MARMFitter Fitter;
  Fitter.SetNumberOfBins(m_Settings->GetHistBinsARMGamma());
  Fitter.SetMaximumARMValue(m_Settings->GetTPDistanceTrans());
  //Fitter.SetFitFunction(MARMFitFunctionID::c_GeneralizedNormal);
  //Fitter.SetFitFunction(MARMFitFunctionID::c_AsymmetricGeneralizedNormalGeneralizedNormal);
  //Fitter.SetFitFunction(MARMFitFunctionID::c_AsymmetricGeneralizedNormalGeneralizedNormalGeneralizedNormal);
  Fitter.SetFitFunction(m_Settings->GetFitFunctionIDARMGamma());
  Fitter.UseBinnedFitting(!m_Settings->GetUseUnbinnedFittingARMGamma());
  Fitter.UseOptimizedBinning(m_Settings->GetOptimizeBinningARMGamma());

  bool FoundEvents = false;
  MPhysicalEvent* Event = nullptr;
  MComptonEvent* ComptonEvent = nullptr;
  // ... loop over all events and save a count in the belonging bin ...
  while ((Event = GetNextEvent()) != nullptr) {

    // Only accept Comptons within the selected ranges...
    if (m_Selector->IsQualifiedEventFast(Event) == true) {
      if (Event->GetType() == MPhysicalEvent::c_Compton) {
        ComptonEvent = dynamic_cast<MComptonEvent*>(Event);
        Fitter.AddARMValue(ComptonEvent->GetARMGamma(TestPosition, m_Settings->GetCoordinateSystem())*c_Deg);
        FoundEvents = true;
      }
    }

    delete Event;
  } 
  
  //Fitter.SaveARMValues("Data.dat");

  // Close the event loader
  FinalizeEventLoader();

  if (FoundEvents == false) {
    mgui<<"No events passed the event selections."<<error;
  }

  Fitter.Fit(m_Settings->GetNumberOfFitsARMGamma());
  //Fitter.FitAll();

  if (Fitter.WasFittingSuccessful() == true) {
    // Draw the histogram
    TCanvas* FitterCanvas = new TCanvas();
    FitterCanvas->SetTitle("ARM of Compton cone");
    FitterCanvas->cd();
    Fitter.Draw();
    FitterCanvas->Modified();
    FitterCanvas->Update();
    if (m_OutputFileName.IsEmpty() == false) {
      FitterCanvas->SaveAs(m_OutputFileName);
    }

    // Dump the results
    mout<<Fitter.ToString()<<endl;
  }

  return;
}


////////////////////////////////////////////////////////////////////////////////


void MInterfaceMimrec::ARMGammaClassic()
{
  // Display the angular resolution measurement for the gamma-ray
  // The ARM value for the scattered gamma-ray is the minimum angle between
  // the gamma-cone-surface and the line connecting the cone-apex with the
  // (Test-)position

  // Start with the event file loader first (just in case something goes wrong here)
  if (InitializeEventLoader() == false) return;


  //double ConfidenceLevel = 0.9; // 90%
  MString ConfidenceLevelString = "90%";

  int NEvents = 0;
  double Value = 0;
  int NAverages = 0;
  double Average = 0;
  int Inside = 0;

  int NBins = m_Settings->GetHistBinsARMGamma();
  double Disk = m_Settings->GetTPDistanceTrans();
  MVector TestPosition = GetTestPosition();
  double BinWidth = 2.0*Disk/NBins;

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

  MPhysicalEvent* Event = nullptr;
  MComptonEvent* ComptonEvent = nullptr;
//   MPairEvent* PairEvent = 0;
  // ... loop over all events and save a count in the belonging bin ...
  while ((Event = GetNextEvent()) != 0) {

    // Only accept Comptons within the selected ranges...
    if (m_Selector->IsQualifiedEventFast(Event) == true) {
      if (Event->GetType() == MPhysicalEvent::c_Compton) {
        ComptonEvent = dynamic_cast<MComptonEvent*>(Event);

        Value = ComptonEvent->GetARMGamma(TestPosition, m_Settings->GetCoordinateSystem());
        if (Value*c_Deg > -Disk && Value*c_Deg < Disk) {
          Inside++;
        }

        Hist->Fill(Value*c_Deg);
        Average += ComptonEvent->Phi()*c_Deg;
        NEvents++;
        NAverages++;
//       } else if (Event->GetType() == MPhysicalEvent::c_Pair) {
//         PairEvent = dynamic_cast<MPairEvent*>(Event);

//         Value = PairEvent->GetARMGamma(TestPosition, m_Settings->GetCoordinateSystem());
//         if (Value*c_Deg > -Disk && Value*c_Deg < Disk) Inside++;
//         Hist->Fill(Value*c_Deg);
//         NEvents++;
      }
    }

    delete Event;
  }

  // Close the event loader
  FinalizeEventLoader();

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
  double SigmaGuess = 1.0;
  mout<<endl<<endl<<"Containment within the histogram (not automatically -180 to 180 degrees):"<<endl;
  for (int b = 0; b + CentralBin <= Hist->GetNbinsX(); ++b) {
    if (b == 0) {
      Content += Hist->GetBinContent(CentralBin);
    } else {
      Content += Hist->GetBinContent(CentralBin + b) + Hist->GetBinContent(CentralBin - b);
    }
    if (Sigma0Found == false && Content >= Sigma0*All) {
      mout<<"  "<<100*Sigma0<<"% containment (radius): "<<Hist->GetBinCenter(CentralBin + b)<<endl;
      Sigma0Found = true;
    }
    if (Sigma1Found == false && Content >= Sigma1*All) {
      mout<<"  "<<100*Sigma1<<"% containment (radius): "<<Hist->GetBinCenter(CentralBin + b)<<endl;
      SigmaGuess = Hist->GetBinCenter(CentralBin + b);
      Sigma1Found = true;
    }
    if (Sigma2Found == false && Content >= Sigma2*All) {
      mout<<"  "<<100*Sigma2<<"% containment (radius): "<<Hist->GetBinCenter(CentralBin + b)<<endl;
      Sigma2Found = true;
    }
    if (Sigma3Found == false && Content >= Sigma3*All) {
      mout<<"  "<<100*Sigma3<<"% containment (radius): "<<Hist->GetBinCenter(CentralBin + b)<<endl;
      Sigma3Found = true;
    }
  }


  TCanvas* Canvas = new TCanvas();
  Canvas->SetTitle("ARM of Compton cone");
  Canvas->cd();
  Hist->Draw();
  Canvas->Modified();
  Canvas->Update();

  TF1* Fit = 0;
  ROOT::Math::MinimizerOptions::SetDefaultMaxFunctionCalls(20000);
  Fit = new TF1("DoubleLorentzAsymGausArm", DoubleLorentzAsymGausArm,
                -Disk*0.99, Disk*0.99, 9);
  Fit->SetBit(kCanDelete);
  Fit->SetParNames("Offset", "Mean",
                   "Lorentz Width 1", "Lorentz Height 1",
                   "Lorentz Width 2", "Lorentz Height 2",
                   "Gaus Height", "Gaus Sigma 1", "Gaus Sigma 2");
  Fit->SetParameters(0, 1, SigmaGuess, 0.5*Hist->GetMaximum(), SigmaGuess, 0.5*Hist->GetMaximum(), 1, SigmaGuess, SigmaGuess);
  Fit->FixParameter(0, 0);
  Fit->SetParLimits(1, -180, +180);
  Fit->SetParLimits(2, 0.5*BinWidth, 180);
  Fit->SetParLimits(3, 0, 2*Hist->GetMaximum());
  Fit->SetParLimits(4, 0.5*BinWidth, 180);
  Fit->SetParLimits(5, 0, 2*Hist->GetMaximum());
  Fit->SetParLimits(6, 0, 2*Hist->GetMaximum());
  Fit->SetParLimits(7, 0.5*SigmaGuess, 180);
  Fit->SetParLimits(8, 0.5*SigmaGuess, 180);

  Canvas->cd();
  TFitResultPtr FitResult;
  TH1D* Confidence = 0;
  if (Fit != 0) {
    cout<<endl<<endl;
    FitResult = Hist->Fit(Fit, "RIMQ SE");
    /*
    if (FitResult->IsValid() == true) {
      Confidence = new TH1D(*Hist);
      Confidence->SetName("ConfidenceHistogramARM");
      Confidence->SetTitle("Confidence Histogram ARM");
      Confidence->SetLineColor(kAzure+10);
      Confidence->SetFillColor(kAzure+10);
      if (TVirtualFitter::GetFitter() != nullptr) {
        (TVirtualFitter::GetFitter())->GetConfidenceIntervals(Confidence, ConfidenceLevel);
      } else {
        merr<<"Virtual fitter is nullptr -- confidence intervals are wrong!"<<endl;
      }
      Hist->SetTitle(MString("ARM (Compton cone) with ") + ConfidenceLevelString + MString(" confidence intervals"));
    }
    */
  }
  Hist->Draw("HIST");
  if (Fit != 0) {
    Fit->SetLineColor(kBlue+2);
    /*
    if (FitResult->IsValid() == true) {
      Confidence->Draw("E5 SAME");
    }
    */
    Fit->Draw("SAME");
  }
  Hist->Draw("HIST SAME");
  Canvas->Modified();
  Canvas->Update();
  if (m_OutputFileName.IsEmpty() == false) {
    Canvas->SaveAs(m_OutputFileName);
  }

  // Calculate FWHM and its uncertainty using the confidence intervals
  double FWHM = GetFWHM(Fit, -180, 180);
  double MinFWHM = -1;
  double MaxFWHM = -1;

  bool FWHMConfidenceGood = false;
  if (Confidence != 0) {
    FWHMConfidenceGood = true;

    // Sub-step a1: Find maximum of upper error curve
    int MaxBin = 0;
    double MaxContent = 0;
    for (int b = 1; b <= Confidence->GetNbinsX(); ++b) {
      if (Confidence->GetBinContent(b) > MaxContent) {
        MaxContent = Confidence->GetBinContent(b);
        MaxBin = b;
      }
    }
    MaxContent = Fit->GetMaximum(); // Use the fit itstself since it is more accurate (and the Confidence is anyway derived from it)

    // Sub-step a2: Find left half value
    int LeftBelow = 0;
    for (int b = MaxBin; b >= 1; --b) {
      if (Confidence->GetBinContent(b) + Confidence->GetBinError(b) < 0.5*MaxContent) {
        LeftBelow = b;
        break;
      }
    }
    if (LeftBelow == 0) {
      cout<<"Error: You zoomed in too much into your ARM peak! Use a larger window around the peak!"<<endl;
      FWHMConfidenceGood = false;
    }
    if (MaxBin - LeftBelow < 5) {
      cout<<"Error: You zoomed out too much from your ARM peak or don't have enough bins! Use a smaller window around the peak or more bins!"<<endl;
      FWHMConfidenceGood = false;
    }
    // y = m*x+t --> x_LeftFWHM = (0.5*MaxContent - t) / m
    double x1 = Confidence->GetBinCenter(LeftBelow);
    double x2 = Confidence->GetBinCenter(LeftBelow+1);
    double y1 = Confidence->GetBinContent(LeftBelow) + Confidence->GetBinError(LeftBelow);
    double y2 = Confidence->GetBinContent(LeftBelow+1) + Confidence->GetBinError(LeftBelow+1);
    double m = (y2-y1) / (x2-x1);
    double t = y2 - m*x2;
    double LeftFWHMBoarder = (0.5*MaxContent - t) / m;

    // Sub-step a3: Find right half value
    int RightBelow = Confidence->GetNbinsX()+1;
    for (int b = MaxBin; b <= Confidence->GetNbinsX(); ++b) {
      if (Confidence->GetBinContent(b) + Confidence->GetBinError(b) < 0.5*MaxContent) {
        RightBelow = b;
        break;
      }
    }
    if (RightBelow == Confidence->GetNbinsX()+1) {
      cout<<"Error: You zoomed in too much into your ARM peak! Use a larger window around the peak!"<<endl;
      FWHMConfidenceGood = false;
    }
    if (RightBelow - MaxBin < 5) {
      cout<<"Error: You zoomed out too much from your ARM peak or don't have enough bins! Use a smaller window around the peak or more bins!"<<endl;
      FWHMConfidenceGood = false;
    }
    // y = m*x+t --> x_LeftFWHM = (0.5*MaxContent - t) / m
    x1 = Confidence->GetBinCenter(RightBelow-1);
    x2 = Confidence->GetBinCenter(RightBelow);
    y1 = Confidence->GetBinContent(RightBelow-1) + Confidence->GetBinError(RightBelow-1);
    y2 = Confidence->GetBinContent(RightBelow) + Confidence->GetBinError(RightBelow);
    m = (y2-y1) / (x2-x1);
    t = y2 - m*x2;
    double RightFWHMBoarder = (0.5*MaxContent - t) / m;

    MaxFWHM = RightFWHMBoarder - LeftFWHMBoarder;
    //cout<<"Boarders: "<<LeftFWHMBoarder<<":"<<RightFWHMBoarder<<endl;


    // Sub-step b1: Find maximum of lower error curve
    // --> no need to redo

    // Sub-step b2: Find left half value
    LeftBelow = 0;
    for (int b = MaxBin; b >= 1; --b) {
      if (Confidence->GetBinContent(b) - Confidence->GetBinError(b) < 0.5*MaxContent) {
        LeftBelow = b;
        break;
      }
    }
    if (LeftBelow == 0) {
      cout<<"Error: You zoomed in too much into your ARM peak! Use a larger window around the peak!"<<endl;
      FWHMConfidenceGood = false;
    }
    if (MaxBin - LeftBelow < 5) {
      cout<<"Error: You zoomed out too much from your ARM peak or don't have enough bins! Use a smaller window around the peak or more bins!"<<endl;
      FWHMConfidenceGood = false;
    }
    // y = m*x+t --> x_LeftFWHM = (0.5*MaxContent - t) / m
    x1 = Confidence->GetBinCenter(LeftBelow);
    x2 = Confidence->GetBinCenter(LeftBelow+1);
    y1 = Confidence->GetBinContent(LeftBelow) - Confidence->GetBinError(LeftBelow);
    y2 = Confidence->GetBinContent(LeftBelow+1) - Confidence->GetBinError(LeftBelow+1);
    m = (y2-y1) / (x2-x1);
    t = y2 - m*x2;
    LeftFWHMBoarder = (0.5*MaxContent - t) / m;

    // Sub-step b3: Find right half value
    RightBelow = Confidence->GetNbinsX()+1;
    for (int b = MaxBin; b <= Confidence->GetNbinsX(); ++b) {
      if (Confidence->GetBinContent(b) - Confidence->GetBinError(b) < 0.5*MaxContent) {
        RightBelow = b;
        break;
      }
    }
    if (RightBelow == Confidence->GetNbinsX()+1) {
      cout<<"Error: You zoomed in too much into your ARM peak! Use a larger window around the peak!"<<endl;
      FWHMConfidenceGood = false;
    }
    if (RightBelow - MaxBin < 5) {
      cout<<"Error: You zoomed out too much from your ARM peak or don't have enough bins! Use a smaller window around the peak or more bins!"<<endl;
      FWHMConfidenceGood = false;
    }
    // y = m*x+t --> x_LeftFWHM = (0.5*MaxContent - t) / m
    x1 = Confidence->GetBinCenter(RightBelow-1);
    x2 = Confidence->GetBinCenter(RightBelow);
    y1 = Confidence->GetBinContent(RightBelow-1) - Confidence->GetBinError(RightBelow-1);
    y2 = Confidence->GetBinContent(RightBelow) - Confidence->GetBinError(RightBelow);
    m = (y2-y1) / (x2-x1);
    t = y2 - m*x2;
    RightFWHMBoarder = (0.5*MaxContent - t) / m;

    MinFWHM = RightFWHMBoarder - LeftFWHMBoarder;
    //cout<<"Boarders: "<<LeftFWHMBoarder<<":"<<RightFWHMBoarder<<endl;
  }


  // Dump all the information

  cout<<endl;
  cout<<endl;
  cout<<"Statistics of ARM histogram and fit"<<endl;
  cout<<"***********************************"<<endl;
  cout<<endl;
  cout<<"Analyzed Compton and pair events:        "<<NEvents<<endl;
  cout<<"Compton and pair events in histogram:    "<<Inside<<" ("<<((NEvents > 0) ? 100.0*Inside/NEvents : 0.0)<<"%)"<<endl;
  cout<<endl;
  cout<<"RMS:                                     "<<Hist->GetRMS()<<" deg"<<endl;
  cout<<endl;
  if (Fit != 0) {
    cout<<"Total FWHM of fit (not of data!):        "<<FWHM<<" deg";
    if (FWHMConfidenceGood == true) {
      cout<<" ("<<ConfidenceLevelString<<" confidence interval: "<<MinFWHM<<" deg ... "<<MaxFWHM<<" deg)";
    }
    if (FitResult->Parameter(2) < 0.5*BinWidth ||
        FitResult->Parameter(4) < 0.5*BinWidth ||
        FitResult->Parameter(7) < 0.5*BinWidth ||
        FitResult->Parameter(8) < 0.5*BinWidth) {
      cout<<" --- WARNING: One of the widths is smaller than one bin --- fit result my be inaccurate!"<<endl;
    } else {
      cout<<endl;
    }
    cout<<"Maximum of fit (x position):             "<<FitResult->Parameter(1)<<" deg";
    if (FitResult->IsValid() == true) {
      cout<<" (1-sigma uncertainty: "<<FitResult->LowerError(1)+FitResult->Parameter(1)<<" deg ... "<<FitResult->UpperError(1)+FitResult->Parameter(1)<<" deg)";
    }
    cout<<" with maximum "<<Fit->Eval(FitResult->Parameter(1))<<" cts"<<endl;

    if (FitResult->IsValid() == false) {
      cout<<endl;
      cout<<"The fit to the data or the subsequent error calculation was not successful!"<<endl;
      cout<<"Try again with a different range (min/max x-values) or different bin size or more statistics."<<endl;
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


//   if (m_Settings->GetStoreImages() == true) {
//     // Get the base file name of the tra file:
//     MString Name = m_Settings->GetCurrentFileName();
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

  // Start with the event file loader first (just in case something goes wrong here)
  if (InitializeEventLoader() == false) return;

  int NEvents = 0;
  int NEventsInside = 0;
  
  MVector TestPosition = GetTestPosition();
  
  double x1Max = m_Settings->GetTPDistanceTrans();
  int x1Bins = m_Settings->GetHistBinsARMGamma();

  double x2Max = m_Settings->GetTPDistanceLong();
  int x2Bins = m_Settings->GetHistBinsARMElectron();

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

  MPhysicalEvent* Event = nullptr;
  MComptonEvent* ComptonEvent = nullptr; 
  double xValue = 0;
  double yValue = 0;
  // ... loop over all events and save a count in the belonging bin ...
  while ((Event = GetNextEvent()) != 0) {

    // Only accept Comptons within the selected ranges...
    if (m_Selector->IsQualifiedEventFast(Event) == true) {
      if (Event->GetType() == MPhysicalEvent::c_Compton) {
        ComptonEvent = dynamic_cast<MComptonEvent*>(Event);

        xValue = ComptonEvent->GetARMGamma(TestPosition, m_Settings->GetCoordinateSystem());
        yValue = ComptonEvent->GetSPDElectron(TestPosition, m_Settings->GetCoordinateSystem());
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
  
  // Close the event loader
  FinalizeEventLoader();

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

  // Start with the event file loader first (just in case something goes wrong here)
  if (InitializeEventLoader() == false) return;

  
  MVector TestPosition = GetTestPosition();
  
  double ARMMax = m_Settings->GetTPDistanceTrans();
  int ARMBins = m_Settings->GetHistBinsARMGamma();
  
  MResponse* Response = 0;
  if (m_Settings->GetResponseType() == MResponseType::Gauss1D) {
    Response = new MResponseGaussian(m_Settings->GetFitParameterComptonTransSphere(), 
                                     m_Settings->GetFitParameterComptonLongSphere(),
                                     m_Settings->GetFitParameterPair(),
                                     m_Settings->GetFitParameterPET());
  } else if (m_Settings->GetResponseType() == MResponseType::GaussByEnergyLeakage) {
    Response = new MResponseEnergyLeakage(m_Settings->GetFitParameterComptonTransSphere(), 
                                       m_Settings->GetFitParameterComptonLongSphere());
  } else if (m_Settings->GetResponseType() == MResponseType::GaussByUncertainties) {
    Response = new MResponseGaussianByUncertainties();
  } else if (m_Settings->GetResponseType() == MResponseType::PRM) {
    Response = new MResponseConeShapes();
    if (dynamic_cast<MResponseConeShapes*>(Response)->LoadResponseFile(m_Settings->GetImagingResponseConeShapesFileName()) == false) {
      mgui<<"Unable to load response files!"<<endl;
      delete Response;
      return;
    }
  } else if (m_Settings->GetResponseType() == MResponseType::PRM) {
    Response = new MResponsePRM();
    if (dynamic_cast<MResponsePRM*>(Response)->LoadResponseFiles(m_Settings->GetImagingResponseComptonTransversalFileName(),
                                   m_Settings->GetImagingResponseComptonLongitudinalFileName(),
                                   m_Settings->GetImagingResponsePairRadialFileName()) == false) {
      mgui<<"Unable to load response files!"<<endl;
      delete Response;
      return;
    }
  } else {
    merr<<"Unknown response type: "<<m_Settings->GetResponseType()<<show;
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
  

  MPhysicalEvent* Event = nullptr;
  MComptonEvent* ComptonEvent = nullptr; 
  // ... loop over all events and save a count in the belonging bin ...
  while ((Event = GetNextEvent()) != 0) {

    // Only accept Comptons within the selected ranges...
    if (m_Selector->IsQualifiedEventFast(Event) == true) {
      if (Event->GetType() == MPhysicalEvent::c_Compton) {
        ComptonEvent = dynamic_cast<MComptonEvent*>(Event);
        double ARM = ComptonEvent->GetARMGamma(TestPosition, m_Settings->GetCoordinateSystem())*c_Deg;
        ARMHist->Fill(ARM, 1);
        Response->AnalyzeEvent(ComptonEvent);
        for (int b = 1; b <= ResponseHist->GetNbinsX(); ++b) {
          ResponseHist->SetBinContent(b, ResponseHist->GetBinContent(b) + Response->GetComptonResponse((ResponseHist->GetBinCenter(b) - ARM)*c_Rad)); 
        }
      } 
    }

    delete Event;
  } 
  
  // Close the event loader
  FinalizeEventLoader();

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

  // Start with the event file loader first (just in case something goes wrong here)
  if (InitializeEventLoader() == false) return;

  
  int NEvents = 0;
  double Value = 0;
  int Inside = 0;

  int NBins = m_Settings->GetHistBinsARMGamma();
  double Disk = m_Settings->GetTPDistanceTrans();
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


  MPhysicalEvent* Event = nullptr;
  MPairEvent* PairEvent = 0; 
  // ... loop over all events and save a count in the belonging bin ...
  while ((Event = GetNextEvent()) != 0) {

    // Only accept Comptons within the selected ranges...
    if (m_Selector->IsQualifiedEventFast(Event) == true) {
      if (Event->GetType() == MPhysicalEvent::c_Pair) {
        PairEvent = dynamic_cast<MPairEvent*>(Event);

        Value = PairEvent->GetARMGamma(TestPosition, m_Settings->GetCoordinateSystem());
        if (Value*c_Deg > -Disk && Value*c_Deg < Disk) Inside++;
        Hist->Fill(Value*c_Deg);
        Hist2->Fill(Value*c_Deg, PairEvent->GetOpeningAngle()*c_Deg);
        NEvents++;
      }
    }

    delete Event;
  } 
  
  // Close the event loader
  FinalizeEventLoader();

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
  Canvas2->cd();
  Hist2->Draw("colz");
  Canvas2->Update();

  return;
}


////////////////////////////////////////////////////////////////////////////////


//! The resolution measure for PET events
void MInterfaceMimrec::ResolutionMeasurePET()
{
  // Display the angular resolution measurement for PET events
  
  // Start with the event file loader first (just in case something goes wrong here)
  if (InitializeEventLoader() == false) return;
  
  
  int NEvents = 0;
  double Value = 0;
  int Inside = 0;
  
  int NBins = m_Settings->GetHistBinsARMGamma();
  double Disk = m_Settings->GetTPDistanceTrans();
  MVector TestPosition = GetTestPosition();
  
  // Initalize the image size (x-axis)
  //BinWidth = 2*Disk/NBins;
  TH1D* Hist = new TH1D("ResolutionMeasurePET", "Resolution measure PET", NBins, 0, Disk);
  Hist->SetBit(kCanDelete);
  Hist->SetDirectory(0);
  Hist->SetXTitle("Resolution [cm]");
  Hist->SetYTitle("counts");
  Hist->SetStats(false);
  Hist->SetFillColor(8);
  Hist->SetMinimum(0);
  
  
  MPhysicalEvent* Event = nullptr;
  MPETEvent* PETEvent = nullptr; 
  // ... loop over all events and save a count in the belonging bin ...
  while ((Event = GetNextEvent()) != 0) {
    
    // Only accept Comptons within the selected ranges...
    if (m_Selector->IsQualifiedEventFast(Event) == true) {
      if (Event->GetType() == MPhysicalEvent::c_PET) {
        PETEvent = dynamic_cast<MPETEvent*>(Event);
        
        Value = PETEvent->GetResolutionMeasure(TestPosition, m_Settings->GetCoordinateSystem());
        if (Value > -Disk && Value < Disk) Inside++;
        Hist->Fill(Value);
        NEvents++;
      }
    }
    
    delete Event;
  } 
  
  // Close the event loader
  FinalizeEventLoader();
  
  if (Hist->GetMaximum() == 0) {
    mgui<<"No events passed the event selections or file is empty!"<<endl;
    return;
  }
  
  
  TCanvas *Canvas = new TCanvas("Canvas angular resolution PET events", "Canvas angular resolution PET events", 800, 600);
  Canvas->SetFillColor(0);
  Canvas->SetFrameBorderSize(0);
  Canvas->SetFrameBorderMode(0);
  Canvas->SetBorderSize(0);
  Canvas->SetBorderMode(0);
  
  /*
  TF1 *L = new TF1("LorentzAsymGausArm", LorentzAsymGausArm, -Disk*0.99, Disk*0.99, 8);
  L->SetParNames("Lorentz Height", "Lorentz Width", "Lorentz Mean", "Lorentz Offset",
                 "Gaus Height", "Gaus Mean", "Gaus Sigma 1", "Gaus Sigma 2");
  L->SetParameters(1, 1, 0, 0, 1, 1, 1, 1);
  L->SetParLimits(0, 0, 99999999);
  //L->SetParLimits(1, 0, 99999999);
  //L->SetParLimits(2, -9999999, 9999999);
  //L->FixParameter(2, 0);
  //L->FixParameter(3, 0);
  //L->SetParLimits(3, 0, 99999);
  //L->SetParLimits(5, 0, 100);
  //L->SetParLimits(6, 0, 10000);
  //L->SetParLimits(7, 0, 10000);
  Hist->Fit("LorentzAsymGausArm", "RQw");
  */
  
  Canvas->cd();
  Hist->Draw();
  //L->Draw("CSAME");
  Canvas->Update();
  
  cout<<endl;
  cout<<"Some statistics: "<<endl;
  cout<<endl;
  cout<<"Analyzed pair events:        "<<NEvents<<endl;
  cout<<"Pair events in histogram:    "<<Inside<<" ("<<((NEvents > 0) ? 100.0*Inside/NEvents : 0.0)<<"%)"<<endl;
  //cout<<"Total FWHM of fit:                       "<<GetFWHM(L, -180, 180)<<" deg"<<endl;
  //cout<<"Maximum of fit (x position):             "<<L->GetMaximumX(-Disk, +Disk)<<" deg"<<endl;
  cout<<endl;
  //delete L;
  
  return;
}


////////////////////////////////////////////////////////////////////////////////


void MInterfaceMimrec::PhiKinVsPhiGeo()
{
  // Display a 2D histogram of the Compton scatter angle Phi calculated via kinematics and via geometry
  
  // Start with the event file loader first (just in case something goes wrong here)
  if (InitializeEventLoader() == false) return;
  
  
  int NBins = m_Settings->GetHistBinsARMGamma();
  double Disk = m_Settings->GetTPDistanceTrans();
  MVector TestPosition = GetTestPosition();
  
  // Initalize the image size (x-axis)
  TH2D* Hist = new TH2D("PhiKinVsPhiGeo", "Compton scatter angle: Kienmatics vs. geometry", 
                        NBins, 0, Disk, NBins, 0, Disk);
  Hist->SetBit(kCanDelete);
  Hist->SetXTitle("Compton scatter angle via geometry [deg]");
  Hist->SetYTitle("Compton scatter angle via kinematics [deg]");
  
  
  MPhysicalEvent* Event = nullptr;
  MComptonEvent* ComptonEvent = nullptr; 
  // ... loop over all events and save a count in the belonging bin ...
  while ((Event = GetNextEvent()) != 0) {
    
    // Only accept Comptons within the selected ranges...
    if (m_Selector->IsQualifiedEventFast(Event) == true) {
      if (Event->GetType() == MPhysicalEvent::c_Compton) {
        ComptonEvent = dynamic_cast<MComptonEvent*>(Event);
        
        // Comtpon scatter angle via geometry - take care of coordinate systems:
        MVector RotPosition = TestPosition;
        if (ComptonEvent->HasDetectorRotation() == true) RotPosition = ComptonEvent->GetDetectorInverseRotationMatrix()*RotPosition;
        if (m_Settings->GetCoordinateSystem() == MCoordinateSystem::c_Galactic && ComptonEvent->HasGalacticPointing() == true) RotPosition = ComptonEvent->GetGalacticPointingInverseRotationMatrix()*RotPosition;
        
        if ((ComptonEvent->C1() - ComptonEvent->C2()).Angle(RotPosition - ComptonEvent->C1())*c_Deg > 89 && (ComptonEvent->C1() - ComptonEvent->C2()).Angle(RotPosition - ComptonEvent->C1())*c_Deg < 91) {
          cout<<ComptonEvent->GetId()<<": "<<(ComptonEvent->C1() - ComptonEvent->C2()).Angle(RotPosition - ComptonEvent->C1())*c_Deg*c_Deg<<endl;
        }
        
        Hist->Fill((ComptonEvent->C1() - ComptonEvent->C2()).Angle(RotPosition - ComptonEvent->C1())*c_Deg, 
                   ComptonEvent->Phi()*c_Deg);
      } 
    }
    
    delete Event;
  } 
  
  // Close the event loader
  FinalizeEventLoader();
  
  if (Hist->GetMaximum() == 0) {
    Error("ARMGamma()", "No events passed the event selections or file is empty!");
    return;
  }
  
  TCanvas* Canvas = new TCanvas();
  Canvas->SetTitle("Compton scatter angle: Kienmatics vs. geometry");
  Canvas->cd();
  Hist->Draw("COLZ");
  Canvas->Update();
  
  return;
}


////////////////////////////////////////////////////////////////////////////////

  
void MInterfaceMimrec::ARMGammaVsComptonScatterAngle()
{
  // Display the angular resolution measurement for the gamma-ray as function of
  // the Compton scatter angle
  // The ARM value for the scattered gamma-ray is the minimum angle between 
  // the gamma-cone-surface and the line connecting the cone-apex with the 
  // (Test-)position

  // Start with the event file loader first (just in case something goes wrong here)
  if (InitializeEventLoader() == false) return;

  
  int NBins = m_Settings->GetHistBinsARMGamma();
  double Disk = m_Settings->GetTPDistanceTrans();
  MVector TestPosition = GetTestPosition();
  
  // Initalize the image size (x-axis)
  //BinWidth = 2*Disk/NBins;
  int NBinsArm = NBins;
  int NBinsAngle = NBins;
  TH2D* Hist = new TH2D("ARMvsComptonScatterAngle", "ARM vs. Compton scatter angle", 
                        NBinsArm, -Disk, Disk, NBinsAngle, 
                        m_Settings->GetComptonAngleRangeMin(), 
                        m_Settings->GetComptonAngleRangeMax());
  Hist->SetBit(kCanDelete);
  Hist->SetXTitle("ARM [deg]");
  Hist->SetYTitle("Compton scatter angle [deg]");


  MPhysicalEvent* Event = nullptr;
  MComptonEvent* ComptonEvent = nullptr; 
  // ... loop over all events and save a count in the belonging bin ...
  while ((Event = GetNextEvent()) != 0) {

    // Only accept Comptons within the selected ranges...
    if (m_Selector->IsQualifiedEventFast(Event) == true) {
      if (Event->GetType() == MPhysicalEvent::c_Compton) {
        ComptonEvent = dynamic_cast<MComptonEvent*>(Event);

        Hist->Fill(ComptonEvent->GetARMGamma(TestPosition, m_Settings->GetCoordinateSystem())*c_Deg, 
                   ComptonEvent->Phi()*c_Deg);
      } 
    }

    delete Event;
  } 
  
  // Close the event loader
  FinalizeEventLoader();

  if (Hist->GetMaximum() == 0) {
    Error("ARMGamma()", "No events passed the event selections or file is empty!");
    return;
  }

  // Normalize:
  /*
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
  */
    
  TCanvas* Canvas = new TCanvas();
  Canvas->SetTitle("ARM vs. Compton scatter angle");
  Canvas->cd();
  Hist->Draw("COLZ");
  Canvas->Update();

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

  // Start with the event file loader first (just in case something goes wrong here)
  if (InitializeEventLoader() == false) return;

  
  int NBins = m_Settings->GetHistBinsARMGamma();
  double Disk = m_Settings->GetTPDistanceTrans();
  MVector TestPosition = GetTestPosition();
  
  // Initalize the image size (x-axis)
  //BinWidth = 2*Disk/NBins;
  int NBinsArm = NBins;
  int NBinsDistance = 20;
  TH2D* Hist = new TH2D("ARMVsDistance", "ARM vs. distance", 
                        NBinsArm, -Disk, Disk, NBinsDistance, 
                        m_Settings->GetFirstDistanceRangeMin(), 
                        m_Settings->GetFirstDistanceRangeMax());
  Hist->SetBit(kCanDelete);
  Hist->SetXTitle("ARM [#circ]");
  Hist->SetYTitle("Distance [cm]");

  cout<<"Dist: "<<m_Settings->GetFirstDistanceRangeMin()<<":"<<m_Settings->GetFirstDistanceRangeMax()<<endl;


  MPhysicalEvent* Event = nullptr;
  MComptonEvent* ComptonEvent = nullptr; 
  // ... loop over all events and save a count in the belonging bin ...
  while ((Event = GetNextEvent()) != 0) {

    // Only accept Comptons within the selected ranges...
    if (m_Selector->IsQualifiedEventFast(Event) == true) {
      if (Event->GetType() == MPhysicalEvent::c_Compton) {
        ComptonEvent = dynamic_cast<MComptonEvent*>(Event);

        Hist->Fill(ComptonEvent->GetARMGamma(TestPosition, m_Settings->GetCoordinateSystem())*c_Deg, 
                   (ComptonEvent->C2() - ComptonEvent->C1()).Mag());
      } 
    }

    delete Event;
  } 
  
  // Close the event loader
  FinalizeEventLoader();

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

  // Start with the event file loader first (just in case something goes wrong here)
  if (InitializeEventLoader() == false) return;

  
  double XMin = m_Settings->GetGalLongitudeMin();
  double XMax = m_Settings->GetGalLongitudeMax();
  int NBinsXAngle = m_Settings->GetBinsGalLongitude();
  double YMin = m_Settings->GetGalLatitudeMin();
  double YMax = m_Settings->GetGalLatitudeMax();
  int NBinsYAngle = m_Settings->GetBinsGalLatitude();
  double Radius = m_Settings->GetSignificanceMapRadius();
  double Distance = m_Settings->GetSignificanceMapDistance();

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

  MPhysicalEvent* Event = nullptr;
  MComptonEvent* ComptonEvent = nullptr; 
  // ... loop over all events and save a count in the belonging bin ...
  while ((Event = GetNextEvent()) != 0) {
    
    // Only accept Comptons within the selected ranges...
    if (m_Selector->IsQualifiedEventFast(Event) == true) {
      if (Event->GetType() == MPhysicalEvent::c_Compton) {
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
            bool at_center = (fabs(ComptonEvent->GetARMGamma(BinCenter, m_Settings->GetCoordinateSystem())*c_Deg)<=Radius);
            bool at_TP1 = (fabs(ComptonEvent->GetARMGamma(TestPoint1, m_Settings->GetCoordinateSystem())*c_Deg)<=Radius);
            bool at_TP2 = (fabs(ComptonEvent->GetARMGamma(TestPoint2, m_Settings->GetCoordinateSystem())*c_Deg)<=Radius);
            bool at_TP3 = (fabs(ComptonEvent->GetARMGamma(TestPoint3, m_Settings->GetCoordinateSystem())*c_Deg)<=Radius);
            bool at_TP4 = (fabs(ComptonEvent->GetARMGamma(TestPoint4, m_Settings->GetCoordinateSystem())*c_Deg)<=Radius);
            bool at_TP5 = (fabs(ComptonEvent->GetARMGamma(TestPoint5, m_Settings->GetCoordinateSystem())*c_Deg)<=Radius);
            bool at_TP6 = (fabs(ComptonEvent->GetARMGamma(TestPoint6, m_Settings->GetCoordinateSystem())*c_Deg)<=Radius);
            bool at_TP7 = (fabs(ComptonEvent->GetARMGamma(TestPoint7, m_Settings->GetCoordinateSystem())*c_Deg)<=Radius);
            bool at_TP8 = (fabs(ComptonEvent->GetARMGamma(TestPoint8, m_Settings->GetCoordinateSystem())*c_Deg)<=Radius);
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
  
  // Close the event loader
  FinalizeEventLoader();

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
                               "Significance",
                               m_Settings->GetImagePalette(), 
                               m_Settings->GetImageDrawMode(),
                               m_Settings->GetImageSourceCatalog());
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

  // Start with the event file loader first (just in case something goes wrong here)
  if (InitializeEventLoader() == false) return;

  
  int NBins = m_Settings->GetHistBinsARMElectron();
  double Disk = m_Settings->GetTPDistanceLong();
  MVector TestPosition = GetTestPosition();
  
  // Initalize the image size (x-axis)
  //BinWidth = 2*Disk/NBins;
  int NBinsArm = NBins;
  int NBinsAngle = NBins/3;
  TH2D* Hist = new TH2D("SPD vs. Compton Scatter Angle", "SPD vs. Compton Scatter Angle", 
                        NBinsArm, 0, Disk, NBinsAngle, 
                        m_Settings->GetComptonAngleRangeMin(), 
                        m_Settings->GetComptonAngleRangeMax());
  Hist->SetBit(kCanDelete);
  Hist->SetXTitle("SPD [#circ]");
  Hist->SetYTitle("Compton scatter angle [#circ]");


  MPhysicalEvent* Event = nullptr;
  MComptonEvent* ComptonEvent = nullptr; 
  // ... loop over all events and save a count in the belonging bin ...
  while ((Event = GetNextEvent()) != 0) {

    // Only accept Comptons within the selected ranges...
    if (m_Selector->IsQualifiedEventFast(Event) == true) {
      if (Event->GetType() == MPhysicalEvent::c_Compton) {
        ComptonEvent = dynamic_cast<MComptonEvent*>(Event);

        Hist->Fill(ComptonEvent->GetSPDElectron(TestPosition)*c_Deg, 
                   ComptonEvent->Phi()*c_Deg);
      } 
    }

    delete Event;
  } 
  
  // Close the event loader
  FinalizeEventLoader();

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

  // Start with the event file loader first (just in case something goes wrong here)
  if (InitializeEventLoader() == false) return;

    
  double Disk = m_Settings->GetTPDistanceTrans();
  MVector TestPosition = GetTestPosition();

  // Initialize the image size
  int xNBins = m_Settings->GetHistBinsARMGamma();
  double* xBins = CreateAxisBins(m_Settings->GetComptonQualityFactorRangeMin(), m_Settings->GetComptonQualityFactorRangeMax(), xNBins, true);



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


  double ArmValue;
  MPhysicalEvent* Event = nullptr;
  MComptonEvent* ComptonEvent = nullptr; 
  // ... loop over all events and save a count in the belonging bin ...
  while ((Event = GetNextEvent()) != 0) {

    // Only accept Comptons within the selected ranges...
    if (m_Selector->IsQualifiedEventFast(Event) == true) {
      if (Event->GetType() == MPhysicalEvent::c_Compton) {
        ComptonEvent = dynamic_cast<MComptonEvent*>(Event);

        ArmValue = ComptonEvent->GetARMGamma(TestPosition, m_Settings->GetCoordinateSystem())*c_Deg;
        if (fabs(ArmValue) < Disk) {
          HistGood->Fill(ComptonEvent->ComptonQualityFactor1());
        } else {
          HistBad->Fill(ComptonEvent->ComptonQualityFactor1());
        }
      } 
    }

    delete Event;
  } 
  
  // Close the event loader
  FinalizeEventLoader();

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

  // Start with the event file loader first (just in case something goes wrong here)
  if (InitializeEventLoader() == false) return;

  
  int NBins = m_Settings->GetHistBinsARMGamma();
  double Disk = m_Settings->GetTPDistanceTrans();
  MVector TestPosition = GetTestPosition();


  MPhysicalEvent* Event = nullptr;
  MComptonEvent* ComptonEvent = nullptr; 
  // ... loop over all events and save a count in the belonging bin ...
  vector<double> ARMValues;
  vector<double> CQFs;
  while ((Event = GetNextEvent()) != 0) {

    // Only accept Comptons within the selected ranges...
    if (m_Selector->IsQualifiedEventFast(Event) == true) {
      if (Event->GetType() == MPhysicalEvent::c_Compton) {
        ComptonEvent = dynamic_cast<MComptonEvent*>(Event);

        ARMValues.push_back(ComptonEvent->GetARMGamma(TestPosition, m_Settings->GetCoordinateSystem())*c_Deg);
        CQFs.push_back(ComptonEvent->ComptonQualityFactor1());
      } 
    }

    delete Event;
  } 
  
  // Close the event loader
  FinalizeEventLoader();

  if (ARMValues.size() == 0) {
    merr<<"No events passed the event selections or file is empty!"<<endl;
    return; 
  }
  
  // Initialize the image size (x-axis)
  double CQFMax = *max_element(CQFs.begin(), CQFs.end());
  double CQFMin = *min_element(CQFs.begin(), CQFs.end());
  
  if (CQFMax > m_Settings->GetComptonQualityFactorRangeMax()) {
    CQFMax = m_Settings->GetComptonQualityFactorRangeMax();
  }
  if (CQFMin < m_Settings->GetComptonQualityFactorRangeMin()) {
    CQFMin = m_Settings->GetComptonQualityFactorRangeMin();
  }
  
  TH2D* Hist = new TH2D("ARM vs. Compton Quality Factor", "ARM vs. Compton Quality Factor", 
                        NBins, -Disk, +Disk, NBins, CQFMin, CQFMax);
  Hist->SetBit(kCanDelete);
  Hist->SetXTitle("ARM [#circ]");
  Hist->SetYTitle("Compton quality factor");  
  
  for (unsigned int i = 0; i < ARMValues.size(); ++i) {
    Hist->Fill(ARMValues[i], CQFs[i]);
  }

    
  TCanvas* ARMvsComptonCanvas = 
    new TCanvas("Canvas ARM vs Compton quality factor",
                "Canvas ARM vs Compton quality factor", 800, 600);
  ARMvsComptonCanvas->cd();
  Hist->Draw("COLZ");
  ARMvsComptonCanvas->Update();

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

  // Start with the event file loader first (just in case something goes wrong here)
  if (InitializeEventLoader() == false) return;

  
  int NBins = m_Settings->GetHistBinsARMGamma();
  double Disk = m_Settings->GetTPDistanceTrans();
  MVector TestPosition = GetTestPosition();

  // Initialize the image size (x-axis)
  int x1NBins = NBins;
  double* x1Bins = CreateAxisBins(-Disk, +Disk, x1NBins, false);

  bool logx2 = true;
  int x2NBins = NBins;
  double* x2Bins = CreateAxisBins(m_Settings->GetClusteringQualityFactorRangeMin(), m_Settings->GetClusteringQualityFactorRangeMax(), x2NBins, logx2);


  TH2D* Hist = new TH2D("ARMVsClusteringQualityFactor", "ARM vs. Clustering Quality Factor", 
                        x1NBins, x1Bins, x2NBins, x2Bins);
  Hist->SetBit(kCanDelete);
  Hist->SetXTitle("ARM [#circ]");
  Hist->SetYTitle("Clustering quality factor");


  MPhysicalEvent* Event = nullptr;
  MComptonEvent* ComptonEvent = nullptr; 
  // ... loop over all events and save a count in the belonging bin ...
  while ((Event = GetNextEvent()) != 0) {

    // Only accept Comptons within the selected ranges...
    if (m_Selector->IsQualifiedEventFast(Event) == true) {
      if (Event->GetType() == MPhysicalEvent::c_Compton) {
        ComptonEvent = dynamic_cast<MComptonEvent*>(Event);

        Hist->Fill(ComptonEvent->GetARMGamma(TestPosition, m_Settings->GetCoordinateSystem())*c_Deg, 
                   ComptonEvent->ClusteringQualityFactor());
      } 
    }

    delete Event;
  } 
  
  // Close the event loader
  FinalizeEventLoader();

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

  // Start with the event file loader first (just in case something goes wrong here)
  if (InitializeEventLoader() == false) return;

  
  int NBins = m_Settings->GetHistBinsARMElectron();
  double Disk = m_Settings->GetTPDistanceLong();
  MVector TestPosition = GetTestPosition();
  
  // Initalize the image size (x-axis)
  int xNBins = NBins;
  double* xBins = CreateAxisBins(0, +Disk, xNBins, false);

  bool yLog;
  if (m_Settings->GetTrackQualityFactorRangeMin() > 0 && m_Settings->GetTrackQualityFactorRangeMin() < 0.01) {
    yLog = true;
  } else {
    yLog = false;
  }
  int yNBins = NBins;
  double* yBins = CreateAxisBins(m_Settings->GetTrackQualityFactorRangeMin(), m_Settings->GetTrackQualityFactorRangeMax(), yNBins, yLog);


  // Create the histogram
  TH2D* Hist = new TH2D("SPD vs. Track Quality Factor", "SPD vs. Track Quality Factor", 
                        xNBins, xBins, yNBins, yBins);
  Hist->SetBit(kCanDelete);
  Hist->SetXTitle("SPD [#circ]");
  Hist->SetYTitle("Track quality factor");
  Hist->SetContour(50);


  int NQFZero = 0;

  MPhysicalEvent* Event = nullptr;
  MComptonEvent* ComptonEvent = nullptr; 
  // ... loop over all events and save a count in the belonging bin ...
  while ((Event = GetNextEvent()) != 0) {

    // Only accept Comptons within the selected ranges...
    if (m_Selector->IsQualifiedEventFast(Event) == true) {
      if (Event->GetType() == MPhysicalEvent::c_Compton) {
        ComptonEvent = dynamic_cast<MComptonEvent*>(Event);

        Hist->Fill(ComptonEvent->GetSPDElectron(TestPosition)*c_Deg, 
                   ComptonEvent->TrackQualityFactor1());
        if (ComptonEvent->TrackQualityFactor1() == 0) NQFZero++;
      } 
    }

    delete Event;
  } 
  
  // Close the event loader
  FinalizeEventLoader();

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

  // Start with the event file loader first (just in case something goes wrong here)
  if (InitializeEventLoader() == false) return;

  
  int NBins = m_Settings->GetHistBinsARMElectron();
  double Disk = m_Settings->GetTPDistanceLong();
  MVector TestPosition = GetTestPosition();
  

  TH2D* Hist = new TH2D("SPD vs. Total Scatter Angle Deviation", 
                        "SPD vs. Total Scatter Angle Deviation", 
                        NBins, 0, Disk, NBins, 0, m_Settings->GetThetaDeviationMax());
  Hist->SetBit(kCanDelete);
  Hist->SetXTitle("SPD [#circ]");
  Hist->SetYTitle("Theta deviation [#circ]");
  Hist->SetContour(50);


  MPhysicalEvent* Event = nullptr;
  MComptonEvent* ComptonEvent = nullptr; 
  // ... loop over all events and save a count in the belonging bin ...
  while ((Event = GetNextEvent()) != 0) {

    // Only accept Comptons within the selected ranges...
    if (m_Selector->IsQualifiedEventFast(Event) == true) {
      if (Event->GetType() == MPhysicalEvent::c_Compton) {
        ComptonEvent = dynamic_cast<MComptonEvent*>(Event);

        Hist->Fill(ComptonEvent->GetSPDElectron(TestPosition)*c_Deg, 
                   ComptonEvent->DeltaTheta()*c_Deg);
      } 
    }

    delete Event;
  } 
  
  // Close the event loader
  FinalizeEventLoader();

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

  // Start with the event file loader first (just in case something goes wrong here)
  if (InitializeEventLoader() == false) return;

  
  double x, y, z = 10000000.0;
  
  // Get the data of the ARM-"Test"-Position
  if (m_Settings->GetCoordinateSystem() == MCoordinateSystem::c_Spheric) {  // spheric 
    x = m_Settings->GetTPTheta();
    y = m_Settings->GetTPPhi();

    // All the external data stuff is in galactic coodinates, but the library uses
    // spherical, so transform it
    //MMath::GalacticToSpheric(x, y);
    MMath::SphericToCartesean(x, y, z);
  } else if (m_Settings->GetCoordinateSystem() == MCoordinateSystem::c_Cartesian2D ||
             m_Settings->GetCoordinateSystem() == MCoordinateSystem::c_Cartesian3D) {
    x = m_Settings->GetTPX();
    y = m_Settings->GetTPY();
    z = m_Settings->GetTPZ();
  } else {
    merr<<"Unknown coordinate system ID: "<<m_Settings->GetCoordinateSystem()<<fatal;
  }

  
  // Initialize the histogram size
  int x1NBins = 50;
  double* x1Bins = CreateAxisBins(GetTotalEnergyMin(), GetTotalEnergyMax(), x1NBins, false);

  int x2NBins = 20;
  double* x2Bins = CreateAxisBins(m_Settings->GetComptonQualityFactorRangeMin(), m_Settings->GetComptonQualityFactorRangeMax(), x2NBins, true);


  // Create the histogram
  TH2D* Hist = new TH2D("Energy vs. Compton Quality Factor", "Energy vs. Compton Quality Factor", 
                        x1NBins, x1Bins, x2NBins, x2Bins);
  Hist->SetBit(kCanDelete);
  Hist->SetXTitle("Energy [keV]");
  Hist->SetYTitle("Compton Quality Factor");


  MPhysicalEvent* Event = nullptr;
  MComptonEvent* ComptonEvent = nullptr; 
  // ... loop over all events and save a count in the belonging bin ...
  while ((Event = GetNextEvent()) != 0) {

    // Only accept Comptons within the selected ranges...
    if (m_Selector->IsQualifiedEventFast(Event) == true) {
      if (Event->GetType() == MPhysicalEvent::c_Compton) {
        ComptonEvent = dynamic_cast<MComptonEvent*>(Event);

        Hist->Fill(ComptonEvent->Ei(), 
                   ComptonEvent->ComptonQualityFactor1());
      } 
    }

    delete Event;
  } 
  
  // Close the event loader
  FinalizeEventLoader();

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

  // Start with the event file loader first (just in case something goes wrong here)
  if (InitializeEventLoader() == false) return;

  
  unsigned int MaxSequenceLength = 3;
  
  // Initialize the image size
  int xNBins = 100;
  double* xBins = CreateAxisBins(0.5, 100.5, xNBins, false);

  int yNBins = 100;
  double* yBins = CreateAxisBins(m_Settings->GetComptonQualityFactorRangeMin(), m_Settings->GetComptonQualityFactorRangeMax(), yNBins, true);


  TH2D* Hist = new TH2D("ComptonSequenceLengthVsComptonQualityFactor", 
                        "Compton Sequence Length vs. Compton Quality Factor", 
                        xNBins, xBins, yNBins, yBins);
  Hist->SetBit(kCanDelete);
  Hist->SetContour(50);
  Hist->SetXTitle("Compton Sequence Length");
  Hist->SetYTitle("Compton Quality Factor");


  MPhysicalEvent* Event = nullptr;
  MComptonEvent* Compton = 0; 
  // ... loop over all events and save a count in the belonging bin ...
  while ((Event = GetNextEvent()) != 0) {

    // Only accept Comptons within the selected ranges...
    if (m_Selector->IsQualifiedEventFast(Event) == true) {
      if (Event->GetType() == MPhysicalEvent::c_Compton) {
        Compton = dynamic_cast<MComptonEvent*>(Event);

        if (Compton->SequenceLength() > MaxSequenceLength) MaxSequenceLength = Compton->SequenceLength();
        Hist->Fill(Compton->SequenceLength(), Compton->ComptonQualityFactor1());
      } 
    }

    delete Event;
  } 
  
  // Close the event loader
  FinalizeEventLoader();

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
  
  // Start with the event file loader first (just in case something goes wrong here)
  if (InitializeEventLoader() == false) return;

  
  int NEvents = 0;
  double Value = 0;
  int NAverages = 0;
  double Average = 0;
  int Inside = 0;
  int FromTop = 0;
  int FromBottom = 0;

  double SizeARMGamma = 180;


  int NBins = m_Settings->GetHistBinsARMElectron();
  double Disk = m_Settings->GetTPDistanceLong();
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


  // And fill the ARM-vector:

  MPhysicalEvent* Event = nullptr;
  MComptonEvent* ComptonEvent = nullptr; 
  // ... loop over all events and save a count in the belonging bin ...
  while ((Event = GetNextEvent()) != 0) {

    // Only accept Comptons within the selected ranges...
    if (m_Selector->IsQualifiedEventFast(Event) == false) {
      delete Event;
      continue;
    }
    if (Event->GetType() != MPhysicalEvent::c_Compton) {
      delete Event;
      continue;
    }

    ComptonEvent = dynamic_cast<MComptonEvent*>(Event);

    if (ComptonEvent->HasTrack() == false) {
      delete Event;
      continue;
    }

    Value = ComptonEvent->GetARMGamma(TestPosition, m_Settings->GetCoordinateSystem());
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
  
  // Close the event loader
  FinalizeEventLoader();

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
  if (m_OutputFileName.IsEmpty() == false) {
    Canvas->SaveAs(m_OutputFileName);
  }


  cout<<endl;
  cout<<"SPD - Characteristics:"<<endl;
  cout<<endl;
  cout<<"FWHM of fitted function:  "<<GetFWHM(L, -Disk, Disk)<<""<<endl;
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

  // Start with the event file loader first (just in case something goes wrong here)
  if (InitializeEventLoader() == false) return;

  
  int NEvents = 0;
  double Value = 0;
  int NAverages = 0;
  double Average = 0;
  int Inside = 0;
  int FromTop = 0;
  int FromBottom = 0;


  int NBins = m_Settings->GetHistBinsARMElectron();
  double Disk = m_Settings->GetTPDistanceLong();
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


  // And fill the ARM-vector:

  MPhysicalEvent* Event = nullptr;
  MComptonEvent* ComptonEvent = nullptr; 
  // ... loop over all events and save a count in the belonging bin ...
  while ((Event = GetNextEvent()) != 0) {

    // Only accept Comptons within the selected ranges...
    if (m_Selector->IsQualifiedEventFast(Event) == false) {
      delete Event;
      continue;
    }
    if (Event->GetType() != MPhysicalEvent::c_Compton) {
      delete Event;
      continue;
    }

    ComptonEvent = dynamic_cast<MComptonEvent*>(Event);

    if (ComptonEvent->HasTrack() == false) {
      delete Event;
      continue;
    }

    Value = ComptonEvent->GetARMElectron(TestPosition, m_Settings->GetCoordinateSystem());

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
  
  // Close the event loader
  FinalizeEventLoader();

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

  // Start with the event file loader first (just in case something goes wrong here)
  if (InitializeEventLoader() == false) return;

  
  bool xLog = m_Settings->GetLogBinningSpectrum();
  double xMin = GetTotalEnergyMin();
  double xMax = GetTotalEnergyMax();

  if (xLog == true) {
    if (xMin <= 0) xMin = 1; 
  }
  
  if (m_Settings->GetSecondEnergyRangeMax() > 0) {
    if (m_Settings->GetSecondEnergyRangeMax() > xMax) xMax = m_Settings->GetSecondEnergyRangeMax();
    if (m_Settings->GetSecondEnergyRangeMin() < xMin) xMin = m_Settings->GetSecondEnergyRangeMin();
  }
  if (m_Settings->GetThirdEnergyRangeMax() > 0) {
    if (m_Settings->GetThirdEnergyRangeMax() > xMax) xMax = m_Settings->GetThirdEnergyRangeMax();
    if (m_Settings->GetThirdEnergyRangeMin() < xMin) xMin = m_Settings->GetThirdEnergyRangeMin();
  }
  if (m_Settings->GetFourthEnergyRangeMax() > 0) {
    if (m_Settings->GetFourthEnergyRangeMax() > xMax) xMax = m_Settings->GetFourthEnergyRangeMax();
    if (m_Settings->GetFourthEnergyRangeMin() < xMin) xMin = m_Settings->GetFourthEnergyRangeMin();
  }
  

  int NBins = m_Settings->GetHistBinsSpectrum();
  double Disk = m_Settings->GetTPDistanceTrans();
  MVector TestPosition = GetTestPosition();
  bool UseTestPosition = m_Settings->GetTPUse();

  int InsideWindow = 0;
  int OutsideWindow = 0;

  if (xLog == true) {
    if (xMin <= 0) xMin = 1;
  }
  double* xBins = CreateAxisBins(xMin, xMax, NBins, xLog);

  TH1D* Hist = new TH1D("EnergySpectrum", "Energy spectrum", NBins, xBins);
  delete [] xBins;

  Hist->SetDirectory(0);
  Hist->SetBit(kCanDelete);
  Hist->SetXTitle("Energy [keV]");
  Hist->SetYTitle("counts / keV");
  Hist->SetStats(false);
  Hist->SetFillColor(8);
  Hist->SetMinimum(0);

  double EMeas = 0.0;

  MBinnerBayesianBlocks Bayes;
  Bayes.SetMinMax(xMin, xMax);
  Bayes.SetMinimumBinWidth((xMax-xMin)/NBins);
  Bayes.SetPrior(3); 

 
  // ... loop over all events and save a count in the belonging bin ...
  MPhysicalEvent* Event = nullptr;
  while ((Event = GetNextEvent()) != 0) {

    if (m_Selector->IsQualifiedEventFast(Event) == false) {
      delete Event;
      continue;
    }

    if (UseTestPosition == true) {
      if (Event->GetType() == MPhysicalEvent::c_Compton) {
        if (fabs(((MComptonEvent*) Event)->GetARMGamma(TestPosition, m_Settings->GetCoordinateSystem()))*c_Deg < Disk) {
          InsideWindow++;
          Hist->Fill(Event->GetEnergy());
          Bayes.Add(Event->GetEnergy(), 1);
          EMeas += Event->GetEnergy();
        } else {
          OutsideWindow++;
        }
      } else if (Event->GetType() == MPhysicalEvent::c_Pair) {
        if (fabs(((MPairEvent*) Event)->GetARMGamma(TestPosition, m_Settings->GetCoordinateSystem()))*c_Deg < Disk) {
          InsideWindow++;
          Hist->Fill(Event->GetEnergy());
          Bayes.Add(Event->GetEnergy(), 1);
          EMeas += Event->GetEnergy();
        } else {
          OutsideWindow++;
        }
      }
    } else {
      Hist->Fill(Event->GetEnergy());
      Bayes.Add(Event->GetEnergy(), 1);
      EMeas += Event->GetEnergy();
      InsideWindow++;
    }

    delete Event;
  } 
  
  // Close the event loader
  FinalizeEventLoader();
  
//   // %%%%%%%%%%%%%%%%%%%%%%%% Write Spectra as ASCII %%%%%%%%%%

//   // Get the base file name of the tra file:
//   MString AsciiSpectrumNameFull(m_Settings->GetCurrentFileName());
//   MString AsciiSpectrumName(gSystem->BaseName(AsciiSpectrumNameFull.Data()));


//   AsciiSpectrumName.Remove(AsciiSpectrumName.Length()-4, 4);
//   AsciiSpectrumName.Append(".ASCIIspectrum.dat");
  
//   std::ofstream ASCIIout(AsciiSpectrumName, ios::out);
  
//   for (int b = 1; b <= (Hist->GetNbinsX() + 1); ++b)
//     {
//       ASCIIout << (Hist->GetBinLowEdge(b)) << " \t" ;
//       if (b <= Hist->GetNbinsX()) 
//  {
//    ASCIIout << (Hist->GetBinContent(b)) << endl;
//  } 
//       else 
//  {
//    ASCIIout << "0.0 \n";
//  }
      
//     }
  
//   ASCIIout.close();
  
//   cout << "Wrote ASCII spectrum to " <<  AsciiSpectrumName << endl;

//   // %%%%%%%%% End Write spectra as ASCII %%%%%%%%%%%%%%%%%%%%%


//   // Normalize to counts/keV
//   for (int b = 1; b <= Hist->GetNbinsX(); ++b) {
//     Hist->SetBinContent(b, Hist->GetBinContent(b)/Hist->GetBinWidth(b));
//   }
  
  /*
  TCanvas* Canvas2 = new TCanvas();
  Canvas2->cd();
  TH1D* HistBayes = Bayes.GetNormalizedHistogram("count rate", "Energy", "[keV]");
  HistBayes->Draw();
  Canvas2->Update();
  */
  
  // Normalize
  for (int b = 1; b <= Hist->GetNbinsX(); ++b) {
    Hist->SetBinContent(b, Hist->GetBinContent(b)/Hist->GetBinWidth(b));
  }

  TCanvas* Canvas = new TCanvas();
  Canvas->SetTitle("Spectrum canvas");
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
  
  //Hist->SetStats(false);
  //Canvas->cd();
  //Hist->SetMinimum(0);
  //Hist->Draw();
  Canvas->Update(); 
  if (m_OutputFileName.IsEmpty() == false) {
    Canvas->SaveAs(m_OutputFileName);
  }

  cout<<endl;
  cout<<"Energy spectrum - some additional statistics:"<<endl;
  cout<<"Number of events:     "<<InsideWindow+OutsideWindow<<" (inside="<<InsideWindow<<", outside="<<OutsideWindow<<")"<<endl;
  if (InsideWindow > 0) {
    cout<<"Avg. measured energy: "<<EMeas/InsideWindow<<" keV"<<endl;
  }
  cout<<endl;

  if (InsideWindow+OutsideWindow == 0) {
    mout<<"No events passed event selections. Use \"Show event selections\" in the mimrec UI to diagnose"<<show;
  }

//   if (m_Settings->GetStoreImages() == true) {
//     // Get the base file name of the tra file:
//     MString Name = m_Settings->GetCurrentFileName();
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

  // Start with the event file loader first (just in case something goes wrong here)
  if (InitializeEventLoader() == false) return;

  
  double EnergyMin = m_Settings->GetInitialEnergyDepositPairMin();
  double EnergyMax = m_Settings->GetInitialEnergyDepositPairMax();

  // Data histogram:
  TH1D* Hist = new TH1D("InitialEnergyDeposit", "Energy deposit in first interaction layer", 100, EnergyMin, EnergyMax);
  Hist->SetBit(kCanDelete);
  Hist->SetXTitle("Energy [keV]");
  Hist->SetYTitle("counts");
  Hist->SetStats(false);
  Hist->SetFillColor(8);
  Hist->SetMinimum(0);

  // ... loop over all events and save a count in the belonging bin ...
  MPhysicalEvent* Event = nullptr;
  while ((Event = GetNextEvent()) != 0) {

    // Only accept Comptons within the selected ranges... 
    if (m_Selector->IsQualifiedEventFast(Event) == false) {
      delete Event;
      continue;
    }

    if (Event->GetType() == MPhysicalEvent::c_Pair) {
      Hist->Fill(((MPairEvent*) Event)->GetInitialEnergyDeposit());
    } 

    delete Event;
  } 
  
  // Close the event loader
  FinalizeEventLoader();

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

  // Start with the event file loader first (just in case something goes wrong here)
  if (InitializeEventLoader() == false) return;

  bool xLog = m_Settings->GetLogBinningSpectrum();
  double xMin = 0;
  double xMax = GetTotalEnergyMax();
  
  if (xLog == true) {
    if (xMin <= 0) xMin = 1; 
  }
  
  if (m_Settings->GetSecondEnergyRangeMax() > 0) {
    if (m_Settings->GetSecondEnergyRangeMax() > xMax) xMax = m_Settings->GetSecondEnergyRangeMax();
    if (m_Settings->GetSecondEnergyRangeMin() < xMin) xMin = m_Settings->GetSecondEnergyRangeMin();
  }
  if (m_Settings->GetThirdEnergyRangeMax() > 0) {
    if (m_Settings->GetThirdEnergyRangeMax() > xMax) xMax = m_Settings->GetThirdEnergyRangeMax();
    if (m_Settings->GetThirdEnergyRangeMin() < xMin) xMin = m_Settings->GetThirdEnergyRangeMin();
  }
  if (m_Settings->GetFourthEnergyRangeMax() > 0) {
    if (m_Settings->GetFourthEnergyRangeMax() > xMax) xMax = m_Settings->GetFourthEnergyRangeMax();
    if (m_Settings->GetFourthEnergyRangeMin() < xMin) xMin = m_Settings->GetFourthEnergyRangeMin();
  }
  
  
  int NBins = m_Settings->GetHistBinsSpectrum();
  double Disk = m_Settings->GetTPDistanceTrans();
  MVector TestPosition = GetTestPosition();
  bool UseTestPosition = m_Settings->GetTPUse();
  
  if (xLog == true) {
    if (xMin <= 0) xMin = 1;
  }
  double* xBins = CreateAxisBins(xMin, xMax, NBins, xLog);
  
  TH2D* ScatterPlot = new TH2D("Energy Distribution", "Energy Distribution", NBins, xBins, NBins, xBins);
  delete [] xBins;
  
  
  ScatterPlot->SetBit(kCanDelete);
  ScatterPlot->SetXTitle("Energy scattered gamma ray [keV]");
  ScatterPlot->SetYTitle("Energy recoil electron [keV]");
  ScatterPlot->SetStats(false);


  // ... loop over all events and save a count in the belonging bin ...
  MComptonEvent* Compton = nullptr;
  MPhysicalEvent* Event = nullptr;
  while ((Event = GetNextEvent()) != 0) {

    // Only accept Comptons within the selected ranges... 
    if (m_Selector->IsQualifiedEventFast(Event) == true) {
      if (Event->GetType() == MPhysicalEvent::c_Compton) {
        Compton = dynamic_cast<MComptonEvent*>(Event);
        if (UseTestPosition == true) {
          if (fabs(Compton->GetARMGamma(TestPosition, m_Settings->GetCoordinateSystem()))*c_Deg < Disk) {
            ScatterPlot->Fill(Compton->Eg(), Compton->Ee(), 1);
          }
        } else {
          ScatterPlot->Fill(Compton->Eg(), Compton->Ee(), 1);
        }
      }
    }

    delete Event;
  } 
  
  // Close the event loader
  FinalizeEventLoader();

  TCanvas* Canvas = new TCanvas("Scatter Plot", " Scatter Plot", 800, 800);
  Canvas->cd();
  ScatterPlot->Draw("COLZ");
  Canvas->Update();

  return;
}


////////////////////////////////////////////////////////////////////////////////


void MInterfaceMimrec::TimeWalkDistribution()
{
  // Display a time walk distribution

  // Start with the event file loader first (just in case something goes wrong here)
  if (InitializeEventLoader() == false) return;

  
  TH1D* TWHist = new TH1D("Time walk Distribution", "Time walk Distribution", 
                          100, m_Settings->GetTimeWalkRangeMin(), m_Settings->GetTimeWalkRangeMax());
  TWHist->SetBit(kCanDelete);
  TWHist->SetXTitle("Time walk [ns]");
  TWHist->SetYTitle("#");
  TWHist->SetStats(false);
  TWHist->SetFillColor(8);

  
  // ... loop over all events and save a count in the belonging bin ...
  MPhysicalEvent* Event = nullptr;
  while ((Event = GetNextEvent()) != 0) {

    // Only accept Comptons within the selected ranges... 
    if (m_Selector->IsQualifiedEventFast(Event) == true) {
      TWHist->Fill(Event->GetTimeWalk());
    }

    delete Event;
  } 
  
  // Close the event loader
  FinalizeEventLoader();

  TCanvas* Canvas = new TCanvas("Time walk Distribution", "Time walk Distribution", 800, 600);
  Canvas->cd();
  TWHist->Draw();
  Canvas->Update();

  return;
}


////////////////////////////////////////////////////////////////////////////////


void MInterfaceMimrec::TimeWalkArmDistribution()
{
  // Display a time walk distribution

  // Start with the event file loader first (just in case something goes wrong here)
  if (InitializeEventLoader() == false) return;

  
  MImage Image;
  Image.SetSpectrum(MImage::c_WhiteBlack);

  MVector TestPosition = GetTestPosition();

  TH2D* TWHist = new TH2D("Time walk ARM Distribution", "Time walk ARM Distribution", 
                          int(m_Settings->GetTimeWalkRangeMax() - m_Settings->GetTimeWalkRangeMin())/100, m_Settings->GetTimeWalkRangeMin(), m_Settings->GetTimeWalkRangeMax(), 
                          100, m_Settings->GetComptonAngleRangeMin(), m_Settings->GetComptonAngleRangeMax());
  TWHist->SetBit(kCanDelete);
  TWHist->SetXTitle("Time walk [ns]");
  TWHist->SetYTitle("ARM [deg]");
  TWHist->SetStats(false);


  // ... loop over all events and save a count in the belonging bin ...
  MPhysicalEvent* Event = nullptr;
  MComptonEvent* ComptonEvent = nullptr; 
  MPairEvent* PairEvent = 0; 
  while ((Event = GetNextEvent()) != 0) {

    // Only accept Comptons within the selected ranges... 
    if (m_Selector->IsQualifiedEventFast(Event) == true) {
      if (Event->GetType() == MPhysicalEvent::c_Compton) {
        ComptonEvent = dynamic_cast<MComptonEvent*>(Event);
        TWHist->Fill(Event->GetTimeWalk(), fabs(ComptonEvent->GetARMGamma(TestPosition, m_Settings->GetCoordinateSystem()))*c_Deg);
      } else if (Event->GetType() == MPhysicalEvent::c_Pair) {
        PairEvent = dynamic_cast<MPairEvent*>(Event);
        TWHist->Fill(Event->GetTimeWalk(), fabs(PairEvent->GetARMGamma(TestPosition, m_Settings->GetCoordinateSystem()))*c_Deg);
      }
    }

    delete Event;
  } 
  
  // Close the event loader
  FinalizeEventLoader();

  TCanvas* Canvas = new TCanvas("Time walk Distribution", "Time walk Distribution", 800, 600);
  Canvas->cd();
  TWHist->Draw("COLZ");
  Canvas->Update();

  return;
}


////////////////////////////////////////////////////////////////////////////////


void MInterfaceMimrec::ScatterAnglesDistribution()
{
  // Distribution of the gamma-scatter-angle

  // Start with the event file loader first (just in case something goes wrong here)
  if (InitializeEventLoader() == false) return;

  
  int NBins = 90;

  //double Phi;
  int NPhi = 0;
  double AvgPhi = 0;
  double AvgSinPhi = 0;
  double PhiMin = m_Settings->GetComptonAngleRangeMin();
  double PhiMax = m_Settings->GetComptonAngleRangeMax();
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
  


  MPhysicalEvent* Event = nullptr;

  bool FoundTrack = false;
  MComptonEvent* ComptonEvent = nullptr; 
  // ... loop over all events and save a count in the belonging bin ...
  while ((Event = GetNextEvent()) != 0) {

    // Only accept Comptons within the selected ranges... 
    if (m_Selector->IsQualifiedEventFast(Event) == false) {
      delete Event;
      continue;
    }
    if (Event->GetType() != MPhysicalEvent::c_Compton) {
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
  
  // Close the event loader
  FinalizeEventLoader();

  if (NPhi > 0) {
    mout<<endl;
    mout<<"Average scatter angles: "<<endl;
    mout<<endl;
    mout<<"  Compton scatter angle (phi):              "<<AvgPhi/NPhi<<" degree (avg sine: "<<AvgSinPhi/NPhi<<")"<<endl;
    if (NTheta > 0 && NEpsilon > 0) {
      mout<<"  Recoil-electron scatter angle (theta):    "<<AvgTheta/NTheta<<" degree"<<endl;
      mout<<"  Total scatter angle (epsilon):            "<<AvgEpsilon/NEpsilon<<" degree"<<endl;
    }
    mout<<endl;

    TCanvas* PhiCanvas = new TCanvas("CanvasPhi", "Canvas of Compton scatter angle distribution", 800, 600);
    PhiHist->SetStats(false);
    PhiCanvas->cd();
    PhiHist->Draw();
    if (m_OutputFileName.IsEmpty() == false) {
      MString Name = m_OutputFileName;
      if (Name.Last('.') != string::npos) {
        MString Suffix = Name.GetSubString(Name.Last('.'));
        Name.RemoveInPlace(Name.Last('.'));
        Name += MString(".phi") + Suffix;
      }
      PhiCanvas->SaveAs(Name);
    }

    if (NTheta > 0 && NEpsilon > 0) {
      TCanvas* EpsilonCanvas = new TCanvas("CanvasEpsilon", "Canvas of electron scatter angle distribution", 800, 600);
      EpsilonHist->SetStats(false);
      EpsilonCanvas->cd();
      EpsilonHist->Draw();
      if (m_OutputFileName.IsEmpty() == false) {
        MString Name = m_OutputFileName;
        if (Name.Last('.') != string::npos) {
          MString Suffix = Name.GetSubString(Name.Last('.'));
          Name.RemoveInPlace(Name.Last('.'));
          Name += MString(".epsilon") + Suffix;
        }
        EpsilonCanvas->SaveAs(Name);
      }
    
      TCanvas* ThetaCanvas = new TCanvas("CanvasThetaViaEnergy", "Canvas of total scatter angle distribution (kin)", 800, 600);
      ThetaHist->SetStats(false);
      ThetaCanvas->cd();
      ThetaHist->Draw();
      if (m_OutputFileName.IsEmpty() == false) {
        MString Name = m_OutputFileName;
        if (Name.Last('.') != string::npos) {
          MString Suffix = Name.GetSubString(Name.Last('.'));
          Name.RemoveInPlace(Name.Last('.'));
          Name += MString(".theta") + Suffix;
        }
        ThetaCanvas->SaveAs(Name);
      }
      
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

  // Start with the event file loader first (just in case something goes wrong here)
  if (InitializeEventLoader() == false) return;

  
  bool xLog;
  if (m_Settings->GetClusteringQualityFactorRangeMin() > 0 && m_Settings->GetClusteringQualityFactorRangeMin() < 0.01) {
    xLog = true;
  } else {
    xLog = false;
  }
  int xNBins = 50;
  double* xBins = CreateAxisBins(m_Settings->GetClusteringQualityFactorRangeMin(), m_Settings->GetClusteringQualityFactorRangeMax(), xNBins, xLog);


  TH1D* Hist = new TH1D("ClusteringQualityFactor", "Clustering Quality Factor", 
                        xNBins, xBins);
  Hist->SetBit(kCanDelete);
  Hist->SetXTitle("Clustering Quality Factor");
  Hist->SetYTitle("counts");
  Hist->SetStats(false);
  Hist->SetFillColor(8);
  Hist->SetMinimum(0);


  MPhysicalEvent* Event = nullptr;
  MComptonEvent* ComptonEvent = nullptr; 
  // ... loop over all events and save a count in the belonging bin ...
  while ((Event = GetNextEvent()) != 0) {

    // Only accept Comptons within the selected ranges... 
    if (m_Selector->IsQualifiedEventFast(Event) == false) {
      delete Event;
      continue;
    }
    if (Event->GetType() != MPhysicalEvent::c_Compton) {
      delete Event;
      continue;
    }
    ComptonEvent = dynamic_cast<MComptonEvent*>(Event);

    Hist->Fill(ComptonEvent->ClusteringQualityFactor());
    
    delete Event;
  }
  
  // Close the event loader
  FinalizeEventLoader();

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

  // Start with the event file loader first (just in case something goes wrong here)
  if (InitializeEventLoader() == false) return;

  
  bool xLog;
  if (m_Settings->GetComptonQualityFactorRangeMin() > 0 && m_Settings->GetComptonQualityFactorRangeMin() < 0.01) {
    xLog = true;
  } else {
    xLog = false;
  }
  int xNBins = 50;
  double* xBins = CreateAxisBins(m_Settings->GetComptonQualityFactorRangeMin(), m_Settings->GetComptonQualityFactorRangeMax(), xNBins, xLog);


  TH1D* Hist = new TH1D("Compton Quality Factor", "Compton Quality Factor", 
                        xNBins, xBins);
  Hist->SetBit(kCanDelete);
  Hist->SetXTitle("Compton Quality Factor");
  Hist->SetYTitle("counts");
  Hist->SetStats(false);
  Hist->SetFillColor(8);
  Hist->SetMinimum(0);


  MPhysicalEvent* Event = nullptr;
  MComptonEvent* ComptonEvent = nullptr; 
  // ... loop over all events and save a count in the belonging bin ...
  while ((Event = GetNextEvent()) != 0) {

    // Only accept Comptons within the selected ranges... 
    if (m_Selector->IsQualifiedEventFast(Event) == false) {
      delete Event;
      continue;
    }
    if (Event->GetType() != MPhysicalEvent::c_Compton) {
      delete Event;
      continue;
    }
    ComptonEvent = dynamic_cast<MComptonEvent*>(Event);

    Hist->Fill(ComptonEvent->ComptonQualityFactor1());
    
    delete Event;
  }
  
  // Close the event loader
  FinalizeEventLoader();

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

  // Start with the event file loader first (just in case something goes wrong here)
  if (InitializeEventLoader() == false) return;

  
  bool xLog;
  if (m_Settings->GetTrackQualityFactorRangeMin() > 0 && m_Settings->GetTrackQualityFactorRangeMin() < 0.01) {
    xLog = true;
  } else {
    xLog = false;
  }
  int xNBins = 50;
  double* xBins = CreateAxisBins(m_Settings->GetTrackQualityFactorRangeMin(), m_Settings->GetTrackQualityFactorRangeMax(), xNBins, xLog);


  TH1D* Hist = new TH1D("Track Quality Factor", "Track Quality Factor", 
                        xNBins, xBins);
  Hist->SetBit(kCanDelete);
  Hist->SetXTitle("Track Quality Factor");
  Hist->SetYTitle("counts");
  Hist->SetStats(false);
  Hist->SetFillColor(8);
  Hist->SetMinimum(0);


  MPhysicalEvent* Event = nullptr;
  MComptonEvent* ComptonEvent = nullptr; 
  // ... loop over all events and save a count in the belonging bin ...
  while ((Event = GetNextEvent()) != 0) {

    // Only accept Comptons within the selected ranges... 
    if (m_Selector->IsQualifiedEventFast(Event) == false) {
      delete Event;
      continue;
    }
    if (Event->GetType() != MPhysicalEvent::c_Compton) {
      delete Event;
      continue;
    }
    ComptonEvent = dynamic_cast<MComptonEvent*>(Event);

    Hist->Fill(ComptonEvent->TrackQualityFactor1());
    
    delete Event;
  }
  
  // Close the event loader
  FinalizeEventLoader();

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

  // Start with the event file loader first (just in case something goes wrong here)
  if (InitializeEventLoader() == false) return;

  
  mimp<<"Fixed earth position at (0, 0, -1e20) = 180 deg"<<endl;

  MVector Position(0, 0, -c_FarAway);

  MComptonEvent* Compton = 0;
  MPhysicalEvent* Event = nullptr;

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
  while ((Event = GetNextEvent()) != 0) {

    // Only accept Comptons within the selected ranges... 
    if (m_Selector->IsQualifiedEventFast(Event) == true &&
        Event->GetType() == MPhysicalEvent::c_Compton) {
      Compton = (MComptonEvent*) Event;
      Hist->Fill(Compton->GetARMGamma(Position, m_Settings->GetCoordinateSystem())*c_Deg);
      NEvents++;
    }    
    delete Event;
  }
  
  // Close the event loader
  FinalizeEventLoader();

  cout<<"NEvents: "<<NEvents<<endl;

  if (Hist->GetEntries() == 0) {
    mgui<<"No events passed the event selections"<<info;
    return;
  }

  TCanvas* Canvas = new TCanvas("EarthCenterDistance", "Earth Center Distance", 800, 600);
  Canvas->cd();
  Hist->Draw();
  Canvas->Update();

  return;
}


////////////////////////////////////////////////////////////////////////////////


void MInterfaceMimrec::DistanceDistribution()
{
  // Distribution of the distance between the first and the second intercation
  // as well as the minimum distance between any interaction

  // Start with the event file loader first (just in case something goes wrong here)
  if (InitializeEventLoader() == false) return;

  
  int NBins = 100;

  MComptonEvent* Compton = 0;
  MPhysicalEvent* Event = nullptr;

  TH1D* FirstHist = 
    new TH1D("FirstDistance", 
             "Distance between first and second hit", NBins, 
             m_Settings->GetFirstDistanceRangeMin(), m_Settings->GetFirstDistanceRangeMax());
  FirstHist->SetBit(kCanDelete);
  FirstHist->SetXTitle("distance [cm]");
  FirstHist->SetYTitle("counts");
  FirstHist->SetStats(false);
  FirstHist->SetFillColor(8);
  FirstHist->SetMinimum(0);

  TH1D* AnyHist = new TH1D("AnyDistance", "Minimum distance between any hit", NBins, 
                           m_Settings->GetDistanceRangeMin(), m_Settings->GetDistanceRangeMax());
  AnyHist->SetBit(kCanDelete);
  AnyHist->SetXTitle("distance [cm]");
  AnyHist->SetYTitle("counts");
  AnyHist->SetStats(false);
  AnyHist->SetFillColor(8);
  AnyHist->SetMinimum(0);

  int NEvents = 0;


  // First check on the size of the histogram:
  while ((Event = GetNextEvent()) != 0) {

    // Only accept Comptons within the selected ranges... 
    if (m_Selector->IsQualifiedEventFast(Event) == true &&
        Event->GetType() == MPhysicalEvent::c_Compton) {
      Compton = (MComptonEvent*) Event;
      FirstHist->Fill((Compton->C2()-Compton->C1()).Mag());
      AnyHist->Fill(Compton->LeverArm());
      NEvents++;
    }    
    delete Event;
  }
  
  // Close the event loader
  FinalizeEventLoader();

  cout<<"NEvents: "<<NEvents<<endl;

  if (FirstHist->GetEntries() == 0 && AnyHist->GetEntries() == 0) {
    mgui<<"No events passed the event selections"<<info;
    return;
  }

  TCanvas* FirstCanvas = new TCanvas("FirstDistanceCanvas", "FirstDistanceCanvas", 800, 600);
  FirstCanvas->cd();
  FirstHist->Draw();
  FirstCanvas->Update();
  if (m_OutputFileName.IsEmpty() == false) {
    MString Name = m_OutputFileName;
    if (Name.Last('.') != string::npos) {
      MString Suffix = Name.GetSubString(Name.Last('.'));
      Name.RemoveInPlace(Name.Last('.'));
      Name += MString(".firstiadistance") + Suffix;
    }
    FirstCanvas->SaveAs(Name);
  }

  TCanvas* AnyCanvas = new TCanvas("AnyDistanceCanvas", "AnyDistanceCanvas", 800, 600);
  AnyCanvas->cd();
  AnyHist->Draw();
  AnyCanvas->Update();
  if (m_OutputFileName.IsEmpty() == false) {
    MString Name = m_OutputFileName;
    if (Name.Last('.') != string::npos) {
      MString Suffix = Name.GetSubString(Name.Last('.'));
      Name.RemoveInPlace(Name.Last('.'));
      Name += MString(".anyiadistance") + Suffix;
    }
    AnyCanvas->SaveAs(Name);
  }

  mout<<endl;
  mout<<"Mean distance between first and second hit: "<<FirstHist->GetMean()<<" cm"<<endl;
  mout<<"Mean distance between any hit:              "<<AnyHist->GetMean()<<" cm"<<endl;
  mout<<endl;

  return;
}


////////////////////////////////////////////////////////////////////////////////


void MInterfaceMimrec::SequenceLengths()
{
  // Distribution of the distance between the first and the second intercation
  // as well as the minimum distance between any interaction

  // Start with the event file loader first (just in case something goes wrong here)
  if (InitializeEventLoader() == false) return;

  
  unsigned int MaxTrackLength = 3;
  unsigned int MaxSequenceLength = 3;
  int NBins = 100;

  MComptonEvent* Compton = 0;
  MPhysicalEvent* Event = nullptr;

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
  while ((Event = GetNextEvent()) != 0) {

    // Only accept Comptons within the selected ranges... 
    if (m_Selector->IsQualifiedEventFast(Event) == true &&
        Event->GetType() == MPhysicalEvent::c_Compton) {
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
  
  // Close the event loader
  FinalizeEventLoader();


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
  if (m_OutputFileName.IsEmpty() == false) {
    MString Name = m_OutputFileName;
    if (Name.Last('.') != string::npos) {
      MString Suffix = Name.GetSubString(Name.Last('.'));
      Name.RemoveInPlace(Name.Last('.'));
      Name += MString(".firsttrack") + Suffix;
    }
    TrackCanvas->SaveAs(Name);
  } 

  TCanvas* ComptonCanvas = 
    new TCanvas("CanvasLengthComptonSequence", 
                "Canvas of length of reconstructed Compton sequence", 800, 600);
  ComptonCanvas->cd();
  ComptonHist->SetAxisRange(0, MaxSequenceLength);
  ComptonHist->Draw("bar1");
  ComptonCanvas->Update();
  if (m_OutputFileName.IsEmpty() == false) {
    MString Name = m_OutputFileName;
    if (Name.Last('.') != string::npos) {
      MString Suffix = Name.GetSubString(Name.Last('.'));
       Name.RemoveInPlace(Name.Last('.'));
      Name += MString(".compton") + Suffix;
    }
    ComptonCanvas->SaveAs(Name);
  }
  
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


void MInterfaceMimrec::LightCurve()
{
  // Time distribution in the data-set

  // Start with the event file loader first (just in case something goes wrong here)
  if (InitializeEventLoader() == false) return;

  
  unsigned int NBins = 1000;

  MPhysicalEvent* Event = nullptr;

  double MinTime = numeric_limits<double>::max();
  double MaxTime = 0;

  vector<double> TimeList;
  MEventSelector NoTimeWindowSelector = *m_Selector;
  NoTimeWindowSelector.SetTime(0, numeric_limits<double>::max());

  
  // First check on the size of the histogram:
  unsigned long NEvents = 0;
  while ((Event = GetNextEvent()) != 0) {

    // Only accept Comptons within the selected ranges... 
    if (m_Selector->IsQualifiedEventFast(Event) == true) {
      if (Event->GetTime().GetAsDouble() == 0) {
        mout<<"Info - light curve: Ignoring all events with time zero"<<endl; 
      } else {
        if (Event->GetTime().GetAsDouble() < MinTime) MinTime = Event->GetTime().GetAsDouble();
        if (Event->GetTime().GetAsDouble() > MaxTime) MaxTime = Event->GetTime().GetAsDouble();
        if (NEvents > 0) {
          if (TimeList.back() > Event->GetTime().GetAsDouble()) {
            cout<<"Backwards time jump detected: "<<setprecision(20)<<TimeList.back()<<" --> "<<Event->GetTime().GetAsDouble()<<endl; 
          }
        }
        TimeList.push_back(Event->GetTime().GetAsDouble());
        NEvents++;
      }
    }    

    delete Event;
  }
  
  // Close the event loader
  FinalizeEventLoader();

  if (TimeList.empty() == true) {
    mgui<<"Light curve: No events passed the event selections!"<<show; 
    return;
  }
  
  mout<<"Minimum time: "<<setprecision(20)<<MinTime<<endl;
  mout<<"Maximum time: "<<setprecision(20)<<MaxTime<<setprecision(6)<<endl;

  if (NEvents/10 < NBins) NBins = NEvents/10;
  if (NBins < 10) NBins = 10;
  
  //NBins = (int) (MaxTime - MinTime);
  
  // Find the common time between min and max
  long min = (long) MinTime;
  long max = (long) MaxTime;
  
  int Counter = 0;
  do {
    min /= 10;
    max /= 10;
    Counter++;
  } while (min != max);
  
  int Subtract = min * pow(10, Counter);
  
  TH1D* HistOptimized = new TH1D("TimeOptimized", "Light curve", NBins, MinTime - Subtract, MaxTime - Subtract);
  HistOptimized->SetBit(kCanDelete);
  HistOptimized->SetXTitle(MString("Time [s] + ") + MString(Subtract) + " seconds");
  HistOptimized->SetYTitle("counts/sec");
  HistOptimized->SetStats(false);
  HistOptimized->SetLineColor(kBlack);
  HistOptimized->SetFillColor(8);
  HistOptimized->SetMinimum(0);
  HistOptimized->GetXaxis()->SetNdivisions(509);
  
  for (unsigned int i = 0; i < TimeList.size(); ++i) {
    HistOptimized->Fill(TimeList[i] - Subtract);
  }
  HistOptimized->Scale(1.0/HistOptimized->GetBinWidth(1));

  TCanvas* CanvasOptimized = new TCanvas("TimeOptimized", "Light curve", 800, 600);
  CanvasOptimized->cd();
  HistOptimized->Draw("HIST");
  CanvasOptimized->Update();

  /*
  MBinnerBayesianBlocks Bayes;
  Bayes.SetMinMax(MinTime - Subtract, MaxTime - Subtract);
  Bayes.SetMinimumBinWidth((int) (std::max(1.0, 0.5*HistOptimized->GetBinWidth(1)) + 0.5));
  Bayes.SetPrior(5); 
    
  for (unsigned int i = 0; i < TimeList.size(); ++i) {
    Bayes.Add(TimeList[i] - Subtract, 1);
  }

  TH1D* HistBayes = Bayes.GetNormalizedHistogram("Light curve (Bayesian blocks)", MString("Time + ") + MString(Subtract) + MString("[s]"), "counts/sec");
  HistBayes->SetStats(false);
  HistBayes->SetLineColor(kRed);
  HistBayes->SetLineWidth(4.0);
  HistBayes->SetMinimum(0);
  HistBayes->GetXaxis()->SetNdivisions(509);
  HistBayes->Draw("SAME");
  CanvasOptimized->Update();
  */
  
  if (m_OutputFileName.IsEmpty() == false) {
    CanvasOptimized->SaveAs(m_OutputFileName);
  }
  
  
  return; 
}


////////////////////////////////////////////////////////////////////////////////


void MInterfaceMimrec::CoincidenceWindowDistribution()
{
  // Time distribution in the data-set

  // Start with the event file loader first (just in case something goes wrong here)
  if (InitializeEventLoader() == false) return;

  
  int NBins = 100;

  MPhysicalEvent* Event = nullptr;

  TH1D* Hist = 
    new TH1D("CoincidenceWindow", "Coincidence window", NBins, 
             m_Settings->GetCoincidenceWindowRangeMin(), m_Settings->GetCoincidenceWindowRangeMax());
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
  while ((Event = GetNextEvent()) != 0) {
    if (Event->GetType() == MPhysicalEvent::c_Compton) {
      ComptonEvent = dynamic_cast<MComptonEvent*>(Event);

      if (ComptonEvent->CoincidenceWindow().GetAsDouble() < MinTime) MinTime = ComptonEvent->CoincidenceWindow().GetAsDouble();
      if (ComptonEvent->CoincidenceWindow().GetAsDouble() > MaxTime) MaxTime = ComptonEvent->CoincidenceWindow().GetAsDouble();

      // Only accept Comptons within the selected ranges... 
      if (m_Selector->IsQualifiedEventFast(ComptonEvent) == true) {
        Hist->Fill(ComptonEvent->CoincidenceWindow().GetAsDouble());
      }    
      // Only accept Comptons within the selected ranges... 
      if (NoTimeWindowSelector.IsQualifiedEventFast(ComptonEvent) == true) {
        TimeList.push_back(ComptonEvent->CoincidenceWindow().GetAsDouble());
      }
    }
    delete Event;
  }
  
  // Close the event loader
  FinalizeEventLoader();

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

  // Start with the event file loader first (just in case something goes wrong here)
  if (InitializeEventLoader() == false) return;

  
  int x, y;
  int x1NBins = 100;
  int x2NBins = 100;
  double x1Min = -0.7, x1Max = 0.7;
  double x2Min = -0.7, x2Max = 0.7;
  double x1BinWidth = (x1Max-x1Min)/x1NBins;
  double x2BinWidth = (x2Max-x2Min)/x2NBins;

  double* Array = new double[x1NBins*x2NBins];
  for (x = 0; x < x1NBins*x2NBins; x++) Array[x] = 0.0;
  
  // ... loop over all events and save a count in the belonging bin ...
  MPhysicalEvent* Event;
  MComptonEvent Compton;
  while ((Event = GetNextEvent()) != 0) {

    if (m_Selector->IsQualifiedEventFast(Event) == false) {
      delete Event;
      continue;
    }

    if (Event->GetType() == MPhysicalEvent::c_Compton) Compton.Assimilate(dynamic_cast<MComptonEvent*>(Event));

    //cout<<Compton.GetSecondComptonIA().X()<<"!"<<Compton.GetSecondComptonIA().Y()<<endl;
    x = (int) ((Compton.C2().X() - x1Min)/x1BinWidth);
    y = (int) ((Compton.C2().Y() - x2Min)/x2BinWidth);

    //cout<<x<<"!"<<y<<endl;
    if (x >= 0 && x < x1NBins && y >= 0 && y < x2NBins) {
      Array[x + y*x1NBins] += 1;
    }

    delete Event;
  } 
  
  // Close the event loader
  FinalizeEventLoader();


  MImage2D* Image = 
    new MImage2D("Location of Second Interaction", Array,
                 "x [m]", x1Min, x1Max, x1NBins,
                 "y [m]", x2Min, x2Max, x2NBins, "counts");

  Image->Display();

  delete [] Array;

  return;
}


////////////////////////////////////////////////////////////////////////////////


void MInterfaceMimrec::Polarization()
{
  // Checks for Polarization...

  // Initalize the image size (x-axis)
  int NBins = m_Settings->GetHistBinsPolarization();

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
  Corrected->SetFillColor(0);
  
  MPhysicalEvent* Event = nullptr;
  MComptonEvent* ComptonEvent = nullptr; 
  MPairEvent* PairEvent = nullptr; 

  // Test position for ARM:
  MVector TestPosition = GetTestPosition();

  // Arm cut:
  double ArmCut = m_Settings->GetPolarizationArmCut();

  // Some statistics:
  int InsideArmCutSource = 0;
  int OutsideArmCutSource = 0;
  int InsideArmCutBackground = 0;
  int OutsideArmCutBackground = 0;

  // Now restart the event-loader with the polarization file:
  if (InitializeEventLoader() == false) return;

  // ... loop over all events and save a count in the belonging bin ...
  while ((Event = GetNextEvent()) != nullptr) {

    // Only accept Comptons within the selected ranges...
    if (m_Selector->IsQualifiedEventFast(Event) == true) {
      if (Event->GetType() == MPhysicalEvent::c_Compton) {
        ComptonEvent = dynamic_cast<MComptonEvent*>(Event);


        if (fabs(ComptonEvent->GetARMGamma(TestPosition, m_Settings->GetCoordinateSystem()))*c_Deg < ArmCut) {
          /*
          MVector Plain = ComptonEvent->Dg();
          Plain.RotateZ(-Phi);
          Plain.RotateY(-Theta);
          double Angle = Plain.Phi();
          Polarization->Fill(Angle*c_Deg);
          */

          double Angle = ComptonEvent->GetAzimuthalScatterAngle(TestPosition, m_Settings->GetCoordinateSystem())*c_Deg;
          Polarization->Fill(Angle);

          InsideArmCutSource++;
        } else {
          OutsideArmCutSource++;
        }
      } else if (Event->GetType() == MPhysicalEvent::c_Pair) {
        PairEvent = dynamic_cast<MPairEvent*>(Event);

        if (fabs(PairEvent->GetARMGamma(TestPosition, m_Settings->GetCoordinateSystem()))*c_Deg < ArmCut) {
          /*
          MVector Plain = PairEvent->GetElectronDirection() + PairEvent->GetPositronDirection();
          Plain.RotateZ(-Phi);
          Plain.RotateY(-Theta);

          double Angle = Plain.Phi();
          Polarization->Fill(Angle*c_Deg);
          */

          double Angle = PairEvent->GetAzimuthalScatterAngle(TestPosition, m_Settings->GetCoordinateSystem())*c_Deg;
          Polarization->Fill(Angle);

          InsideArmCutSource++;
        } else {
          OutsideArmCutSource++;
        }
      }
    }

    delete Event;
  }   
  
  // Close the event loader
  FinalizeEventLoader();
 
  if (Polarization->Integral() == 0) {
    mgui<<"No events passed the event selections for the polarized data file"<<show;
    return;
  }

  for (int b = 1; b <= Polarization->GetNbinsX(); ++b) {
    if (Polarization->GetBinContent(b) == 0) {
      mgui<<"You don't have enough statistics in your source measurement: Some bins are zero, e.g. bin "<<b<<show;
      return;
    }
  }


  // Now restart the event-loader with the background file:
  if (InitializeEventLoader(m_Settings->GetPolarizationBackgroundFileName()) == false) return;

  
  MEventSelector SecondSelector(*m_Selector);
  SecondSelector.SetTimeMode(0); // Since the background file is likely simulation, we do not use a time cut -- this is noted in the GUI
  
  // ... loop over all events and save a count in the belonging bin ...
  while ((Event = GetNextEvent()) != nullptr) {

    // Only accept Comptons within the selected ranges...
    if (SecondSelector.IsQualifiedEventFast(Event) == true) {
      if (Event->GetType() == MPhysicalEvent::c_Compton) {
        ComptonEvent = dynamic_cast<MComptonEvent*>(Event);

        if (fabs(ComptonEvent->GetARMGamma(TestPosition, m_Settings->GetCoordinateSystem()))*c_Deg < ArmCut) {
          /*
          MVector Plain = ComptonEvent->Dg();
          Plain.RotateZ(-Phi);
          Plain.RotateY(-Theta);

          double Angle = Plain.Phi();
          Background->Fill(Angle*c_Deg);
          */
          double Angle = ComptonEvent->GetAzimuthalScatterAngle(TestPosition, m_Settings->GetCoordinateSystem())*c_Deg;
          Background->Fill(Angle);

          InsideArmCutBackground++;
        } else {
          OutsideArmCutBackground++;
        }
      } else if (Event->GetType() == MPhysicalEvent::c_Pair) {
        PairEvent = dynamic_cast<MPairEvent*>(Event);

        if (fabs(PairEvent->GetARMGamma(TestPosition, m_Settings->GetCoordinateSystem()))*c_Deg < ArmCut) {
          /*
          MVector Plain = PairEvent->GetElectronDirection() + PairEvent->GetPositronDirection();
          Plain.RotateZ(-Phi);
          Plain.RotateY(-Theta);
          
          double Angle = Plain.Phi();
          Background->Fill(Angle*c_Deg);
          */

          double Angle = PairEvent->GetAzimuthalScatterAngle(TestPosition, m_Settings->GetCoordinateSystem())*c_Deg;
          Background->Fill(Angle);

          InsideArmCutBackground++;
        } else {
          OutsideArmCutBackground++;
        }
      }
    }

    delete Event;
  }   
  
  // Close the event loader
  FinalizeEventLoader();

  if (Background->Integral() == 0) {
    mgui<<"No events passed the event selections for the background data file"<<show;
    return;
  }

  // Sanity check
  for (int b = 1; b <= Background->GetNbinsX(); ++b) {
    if (Background->GetBinContent(b) == 0) {
      mgui<<"You don't have enough statistics in your background measurement: Some bins are zero, e.g. bin "<<b<<endl;
      return;
    }
  }


  // Correct the modulation signature with the zero modulation
  
  // The correction is
  // C[i] = P[i]/B[i] * SUM_j( B[j] ) / NBins 
  
  // Why do we do this?
  // The measurement counts is basically the perfect polarization signature multiplied with the probability that we detect something in this direction:
  // Now to get the polarization signature we have to divide by this probability.
  // We don't need anything absolute since we are only interested in the polarization angle and modulation.
  // Thus a proxy for the polarization is the normalized background, i.e. background divided by its mean.
  
  
  // Its uncertainty is 
  // dC[i] = 
  //  
  

  // Since we correcting with background, we have to multiply with its mean
  double Mean = Background->Integral()/NBins;

  for (int i = 1; i <= NBins; ++i) {
    Corrected->SetBinContent(i, Polarization->GetBinContent(i)/Background->GetBinContent(i)*Mean);
    double Uncertainty = 0.0;
    
    // Mean part
    for (int j = 1; j <= NBins; ++j) {
      Uncertainty += pow(Polarization->GetBinContent(i)/Background->GetBinContent(i)/NBins * sqrt(Background->GetBinContent(j)), 2);
    }
    
    // Background part:
    Uncertainty += pow(Polarization->GetBinContent(i)/Background->GetBinContent(i)/Background->GetBinContent(i) * Mean * sqrt(Background->GetBinContent(i)), 2);
    // REMARK:
    // In theory the above case would never the uncertainty equation, since when i look at the mean
    // part: P[i]/B[i] * B[i] cancels out
    // However, since significantly underestimates the uncertainty, thus cannot be right...
    
    
    // Polarization part:
    Uncertainty += pow(Mean/Background->GetBinContent(i) * sqrt(Polarization->GetBinContent(i)), 2);

    Corrected->SetBinError(i, sqrt(Uncertainty));
  }
    
  // Normalize before we do anything else  
  
  for (int b = 1; b <= NBins; ++b) {
    Polarization->SetBinContent(b, Polarization->GetBinContent(b)/Polarization->GetBinWidth(b));
  }

  for (int b = 1; b <= NBins; ++b) {
    Background->SetBinContent(b, Background->GetBinContent(b)/Background->GetBinWidth(b));
  }    
    
  for (int b = 1; b <= NBins; ++b) {
    Corrected->SetBinContent(b, Corrected->GetBinContent(b)/Corrected->GetBinWidth(b));
    Corrected->SetBinError(b, Corrected->GetBinError(b)/Corrected->GetBinWidth(b));
  }
  
    
  // Try to fit a cosine
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

  
  
  // Now normalize to counts/deg & draw everything on screen:

  TCanvas* PolarizationCanvas = new TCanvas("PolarizationCanvas", "Polarized source", 800, 600);
  PolarizationCanvas->cd();
  Polarization->Draw();
  PolarizationCanvas->Update(); 
  

  TCanvas* BackgroundCanvas = new TCanvas("BackgroundCanvas", "Not polarized source", 800, 600);
  BackgroundCanvas->cd();
  Background->Draw();
  BackgroundCanvas->Update();

  
  TCanvas* CorrectedCanvas = new TCanvas("CorrectedCanvas", "Background corrected polarization signature", 800, 600);
  CorrectedCanvas->cd();
  Corrected->Draw(); //"EHIST");
  Mod->Draw("SAME");
  CorrectedCanvas->Update();

  
  // Safe
  if (m_OutputFileName.IsEmpty() == false) {
    MString Name;

    Name = m_OutputFileName;
    if (Name.Last('.') != string::npos) {
      MString Suffix = Name.GetSubString(Name.Last('.'));
      Name.RemoveInPlace(Name.Last('.'));
      Name += MString(".polarization.rawsource") + Suffix;
    }
    PolarizationCanvas->SaveAs(Name);

    Name = m_OutputFileName;
    if (Name.Last('.') != string::npos) {
      MString Suffix = Name.GetSubString(Name.Last('.'));
      Name.RemoveInPlace(Name.Last('.'));
      Name += MString(".polarization.rawbackground") + Suffix;
    }
    BackgroundCanvas->SaveAs(Name);

    Name = m_OutputFileName;
    if (Name.Last('.') != string::npos) {
      MString Suffix = Name.GetSubString(Name.Last('.'));
      Name.RemoveInPlace(Name.Last('.'));
      Name += MString(".polarization.corrected") + Suffix;
    }
    CorrectedCanvas->SaveAs(Name);
  }
  
  delete Lin;
  delete Mod;

  return;
}


////////////////////////////////////////////////////////////////////////////////


void MInterfaceMimrec::AzimuthalComptonScatterAngle()
{

  // Start with the event file loader first (just in case something goes wrong here)
  if (InitializeEventLoader() == false) return;

  
  // Initalize the image size (x-axis)
  int NBins = m_Settings->GetHistBinsARMGamma();
  double Disk = m_Settings->GetTPDistanceTrans();
  MVector TestPosition = GetTestPosition();

  TH1D* Hist = new TH1D("AzimuthalComptonScatterAngle", "Azimuthal Compton Scatter Angle", NBins, -180, 180);
  Hist->SetBit(kCanDelete);
  Hist->SetXTitle("[degree]");
  Hist->SetYTitle("counts/degree");
  Hist->SetFillColor(8);
  Hist->SetStats(false);
  Hist->SetMinimum(0.0);
  Hist->SetNdivisions(-508, "X");

  MPhysicalEvent* Event = nullptr;
  MComptonEvent* ComptonEvent = nullptr; 

  // Some statistics:
  int InsideArmCutSource = 0;
  int OutsideArmCutSource = 0;

  // ... loop over all events and save a count in the belonging bin ...
  while ((Event = GetNextEvent()) != 0) {

    // Only accept Comptons within the selected ranges...
    if (m_Selector->IsQualifiedEventFast(Event) == true) {
      if (Event->GetType() == MPhysicalEvent::c_Compton) {
        ComptonEvent = dynamic_cast<MComptonEvent*>(Event);
        if (fabs(ComptonEvent->GetARMGamma(TestPosition, m_Settings->GetCoordinateSystem()))*c_Deg < Disk) {

          double Angle = ComptonEvent->GetAzimuthalScatterAngle(TestPosition, m_Settings->GetCoordinateSystem())*c_Deg;

          Hist->Fill(Angle);
          
          InsideArmCutSource++;
        } else {
          OutsideArmCutSource++;
        }
      }
    }

    delete Event;
  }   

  // Close the event loader
  FinalizeEventLoader();

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

  // Start with the event file loader first (just in case something goes wrong here)
  if (InitializeEventLoader() == false) return;

  
  // Initalize the image size (x-axis)
  int NBins = m_Settings->GetHistBinsARMElectron();

  TH1D* Hist = new TH1D("AzimuthalElectronScatterAngle", "Azimuthal Electron Scatter Angle", NBins, -180, 180);
  Hist->SetBit(kCanDelete);
  Hist->SetXTitle("[degree]");
  Hist->SetYTitle("counts/degree");
  Hist->SetFillColor(8);
  Hist->SetStats(false);
  Hist->SetMinimum(0.0);
  Hist->SetNdivisions(-508, "X");


  MPhysicalEvent* Event = nullptr;
  MComptonEvent* ComptonEvent = nullptr; 

  // Origin in spherical coordinates:
  double Theta = m_Settings->GetTPTheta()*c_Rad;
  double Phi = m_Settings->GetTPPhi()*c_Rad;

  // Origin in Cartesian Corrdinates:
  MVector Origin;
  Origin.SetMagThetaPhi(c_FarAway, Theta, Phi);

  double ArmCut = 180;

  // Some statistics:
  int InsideArmCutSource = 0;
  int OutsideArmCutSource = 0;

  // ... loop over all events and save a count in the belonging bin ...
  while ((Event = GetNextEvent()) != 0) {

    // Only accept Comptons within the selected ranges...
    if (m_Selector->IsQualifiedEventFast(Event) == true) {
      if (Event->GetType() == MPhysicalEvent::c_Compton) {
        ComptonEvent = dynamic_cast<MComptonEvent*>(Event);
        if (ComptonEvent->HasTrack() == true) {
          if (fabs(ComptonEvent->GetARMGamma(Origin, m_Settings->GetCoordinateSystem()))*c_Deg < ArmCut) {
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
  
  // Close the event loader
  FinalizeEventLoader();

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

  // Start with the event file loader first (just in case something goes wrong here)
  if (InitializeEventLoader() == false) return;

  
  // Initalize the image size (x-axis)
  int NBins = 100;
  TH1D* Hist = new TH1D("OpeningAnglePair", "Opening Angle Pair", NBins, 
                        m_Settings->GetOpeningAnglePairMin(), m_Settings->GetOpeningAnglePairMax());
  Hist->SetBit(kCanDelete);
  Hist->SetXTitle("Opening angle [#circ]");
  Hist->SetYTitle("counts");
  Hist->SetStats(false);
  Hist->SetFillColor(8);


  MPhysicalEvent* Event = nullptr;
  MPairEvent* PairEvent = 0; 
  // ... loop over all events and save a count in the belonging bin ...
  while ((Event = GetNextEvent()) != 0) {

    // Only accept Comptons within the selected ranges...
    if (m_Selector->IsQualifiedEventFast(Event) == true) {
      if (Event->GetType() == MPhysicalEvent::c_Pair) {
        PairEvent = dynamic_cast<MPairEvent*>(Event);
        Hist->Fill(PairEvent->GetOpeningAngle()*c_Deg);
      }
    }

    delete Event;
  }   
  
  // Close the event loader
  FinalizeEventLoader();

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

  // Start with the event file loader first (just in case something goes wrong here)
  if (InitializeEventLoader() == false) return;

  
  int NEvents = 0;
  double Value = 0;
  int Inside = 0;
  
  int NBins = m_Settings->GetHistBinsARMGamma();
  double Disk = m_Settings->GetTPDistanceTrans();
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


  MPhysicalEvent* Event = nullptr;
  MPairEvent* PairEvent = 0; 
  // ... loop over all events and save a count in the belonging bin ...
  while ((Event = GetNextEvent()) != 0) {

    // Only accept Comptons within the selected ranges...
    if (m_Selector->IsQualifiedEventFast(Event) == true) {
      if (Event->GetType() == MPhysicalEvent::c_Pair) {
        PairEvent = dynamic_cast<MPairEvent*>(Event);

        Value = PairEvent->GetARMGamma(TestPosition, m_Settings->GetCoordinateSystem());
        if (Value*c_Deg > -Disk && Value*c_Deg < Disk) Inside++;
        Hist->Fill(Value*c_Deg, PairEvent->GetTrackQualityFactor());
        NEvents++;
      }
    }

    delete Event;
  } 
  
  // Close the event loader
  FinalizeEventLoader();

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


void MInterfaceMimrec::DirectionScatteredGammaRay()
{
  
  // Start with the event file loader first (just in case something goes wrong here)
  if (InitializeEventLoader() == false) return;
  
  
  
  // Initalize the image size (x-axis)
  //BinWidth = 2*Disk/NBins;
  TH2D* Hist = new TH2D("Direction scattered gamma ray", "Direction scattered gamma ray", 90, -180, 180, 45, 0, 180);
  Hist->SetBit(kCanDelete);
  Hist->SetDirectory(0);
  Hist->SetXTitle("Phi in instrument coordiantes [#circ]");
  Hist->SetYTitle("Theta in instrument coordiantes [#circ]");
  Hist->SetStats(false);
  Hist->SetFillColor(8);
  //double BinWidth = 2*Disk/NBins;
  
  
  MPhysicalEvent* Event = nullptr;
  MComptonEvent* ComptonEvent = 0; 
  // ... loop over all events and save a count in the belonging bin ...
  while ((Event = GetNextEvent()) != 0) {
    
    // Only accept Comptons within the selected ranges...
    if (m_Selector->IsQualifiedEventFast(Event) == true) {
      if (Event->GetType() == MPhysicalEvent::c_Compton) {
        ComptonEvent = dynamic_cast<MComptonEvent*>(Event);
        
        Hist->Fill(ComptonEvent->Dg().Phi()*c_Deg, ComptonEvent->Dg().Theta()*c_Deg);
      }
    }
    
    delete Event;
  } 
  
  // Close the event loader
  FinalizeEventLoader();
  
  if (Hist->GetMaximum() == 0) {
    mgui<<"No events passed the event selections or file is empty!"<<endl;
    return;
  }
  
  
  TCanvas *Canvas = new TCanvas("Canvas direction scattered gamma ray", "Canvas direction scattered gamma ray", 800, 600);
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

  // Start with the event file loader first (just in case something goes wrong here)
  if (InitializeEventLoader() == false) return;

  
  MString Name = m_EventFile->GetFileName();
  Name.Replace(Name.Length()-4, 4, ".ids");

  ofstream fout;
  fout.open(Name);
  if (fout.is_open() == false) {
    mgui<<"Unable to open file \""<<Name<<"\""<<endl;
    return;
  }

  // ... loop over all events
  MPhysicalEvent* Event = nullptr;
  while ((Event = GetNextEvent()) != 0) {

    if (m_Selector->IsQualifiedEventFast(Event) == true) {
      fout<<"ID "<<Event->GetId()<<endl;
    }

    delete Event;
  }   
  
  // Close the event loader
  FinalizeEventLoader();

  fout.close();
}


////////////////////////////////////////////////////////////////////////////////


void MInterfaceMimrec::InteractionDepth()
{
  // Calculates the average interaction depth in tracker and calorimeter:

  // Start with the event file loader first (just in case something goes wrong here)
  if (InitializeEventLoader() == false) return;

  
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


  MPhysicalEvent* Event = nullptr;
  MComptonEvent* ComptonEvent = nullptr; 
  // ... loop over all events and save a count in the belonging bin ...
  while ((Event = GetNextEvent()) != 0) {

    // Only accept Comptons within the selected ranges...
    if (m_Selector->IsQualifiedEventFast(Event) == true) {
      if (Event->GetType() == MPhysicalEvent::c_Compton) {
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
  
  // Close the event loader
  FinalizeEventLoader();

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
}


////////////////////////////////////////////////////////////////////////////////


void MInterfaceMimrec::LocationOfInitialInteraction()
{
  if (m_Geometry == 0) {
    mgui<<"You need a loaded geometry for this function"<<error;
    return;
  }

  // Start with the event file loader first (just in case something goes wrong here)
  if (InitializeEventLoader() == false) return;

  
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

  long NMeanDepths = 0;
  double MeanDepth = 0.0;
  
  
  // Step 1: Accumulate many, many hits:

  MVector Pos;
  MPhysicalEvent* Event = nullptr;
  while ((Event = GetNextEvent()) != 0) {

    if (m_Selector->IsQualifiedEventFast(Event) == true) {
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
    
    NMeanDepths++;
    MeanDepth += Pos[2];
  }


  // Step 4: Continue filling from file:

  while ((Event = GetNextEvent()) != 0) {

    if (m_Selector->IsQualifiedEventFast(Event) == true) {
      if (Event->GetPosition() != g_VectorNotDefined) {
        Pos = Event->GetPosition();
        MDVolumeSequence V = m_Geometry->GetVolumeSequence(Pos);
        if (V.GetDetector() != 0) {
          DetectorOccupation[V.GetDetector()->GetName()]++;

          xyzHist->Fill(Pos[0], Pos[1], Pos[2]);
          xHist->Fill(Pos[0]);
          yHist->Fill(Pos[1]);
          zHist->Fill(Pos[2]);
          
          NMeanDepths++;
          MeanDepth += Pos[2];
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

  if (NMeanDepths > 0) {
    mout<<"Mean depth: "<<MeanDepth/NMeanDepths<<" cm"<<endl;
    mout<<endl;
  }
  
  // Close the event loader
  FinalizeEventLoader();
}


////////////////////////////////////////////////////////////////////////////////


void MInterfaceMimrec::CreateExposureMap()
{
  // Create an exposure map
  
  if (m_Settings->GetExposureMode() == MExposureMode::Flat) {
    mgui<<"ERROR: You need to set an exposure mode other than \"flat\" in order to create a useful exposure"<<show;
    return;    
  }
  
  // Check if the exposure efficiency file is there:
  if (MFile::Exists(m_Settings->GetExposureEfficiencyFile()) == false) {
    mgui<<"ERROR: Unable to find exposure efficiency file: \""<<m_Settings->GetExposureEfficiencyFile()<<"\""<<show;
    return;
  }
  
  // Start with the event file loader first (just in case something goes wrong here)
  if (InitializeEventLoader() == false) return;
  
  MExposure Exposure;
  if (Exposure.SetEfficiencyFile(m_Settings->GetExposureEfficiencyFile()) == false) {
    mgui<<"ERROR: Unable to load exposure efficiency file: \""<<m_Settings->GetExposureEfficiencyFile()<<"\""<<show;
    return;    
  }
  Exposure.SetDimensions(m_Settings->GetGalLongitudeMin()*c_Rad, 
                        m_Settings->GetGalLongitudeMax()*c_Rad, 
                        m_Settings->GetBinsGalLongitude(),
                        (m_Settings->GetGalLatitudeMin()+90)*c_Rad,
                        (m_Settings->GetGalLatitudeMax()+90)*c_Rad,
                        m_Settings->GetBinsGalLatitude(),
                        c_FarAway/10, 
                        c_FarAway, 
                        1);
  
  
  MPhysicalEvent* Event;
  while ((Event = GetNextEvent()) != 0) { 
    
    if (m_Selector->IsQualifiedEventFast(Event) == true) {
      Exposure.Expose(Event);
    }
    delete Event;
  }
  
  // Close the event loader
  FinalizeEventLoader();
  
  double* ExposureImage = Exposure.GetExposure();
  
  MImageGalactic* Galactic = 
    new MImageGalactic("Exposure in Galactic coordinates", ExposureImage, 
                       "Longitude [deg]", m_Settings->GetGalLongitudeMin(), m_Settings->GetGalLongitudeMax(), m_Settings->GetBinsGalLongitude(),
                       "Latitude [deg]", m_Settings->GetGalLatitudeMin(), m_Settings->GetGalLatitudeMax(), m_Settings->GetBinsGalLatitude(), "Exposure [cm^{2} s]");
  Galactic->Display(nullptr);

  delete [] ExposureImage;
  
  return;
}
  
  
////////////////////////////////////////////////////////////////////////////////


void MInterfaceMimrec::PointingInGalacticCoordinates()
{
  // Show the pointing of the instrument in galactic coordinates

  // Start with the event file loader first (just in case something goes wrong here)
  if (InitializeEventLoader() == false) return;

    
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


  MPhysicalEvent* Event = nullptr;
  // ... loop over all events and save a count in the belonging bin ...
  while ((Event = GetNextEvent()) != 0) {

    // Only accept some events...
    if (m_Selector->IsQualifiedEventFast(Event) == true) {
      int LongBin = int((Event->GetGalacticPointingZAxisLongitude()*c_Deg - LongMin)/LongBinSize);
      int LatBin = int((Event->GetGalacticPointingZAxisLatitude()*c_Deg + 90 - LatMin)/LatBinSize);
      Array[LongBin + LatBin*LongBins] += 1.0;
    }
    
    delete Event;
  }
  
  // Close the event loader
  FinalizeEventLoader();

  // Prepare an MImageGalactic class:
  MImageGalactic* Galactic = 
    new MImageGalactic("Pointing in galactic coordinates", Array,
                       "Longitude", LongMin, LongMax, LongBins, 
                       "Latitude", LatMin-90, LatMax-90, LatBins,
                       "propotional to time spent [a.u.]",
                       m_Settings->GetImagePalette(), m_Settings->GetImageDrawMode());
  Galactic->Normalize(true);
  Galactic->SetImageArray(Array);
  Galactic->Display();


  delete [] Array;
  // memory leak: Galactic is never deleted!
}


////////////////////////////////////////////////////////////////////////////////


void MInterfaceMimrec::HorizonInSphericalDetectorCoordinates()
{
  // Show the horizon z axis in spherical detector coordinates

  // Start with the event file loader first (just in case something goes wrong here)
  if (InitializeEventLoader() == false) return;

    
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


  MPhysicalEvent* Event = nullptr;
  // ... loop over all events and save a count in the belonging bin ...
  while ((Event = GetNextEvent()) != 0) {

    // Only accept some events...
    if (m_Selector->IsQualifiedEventFast(Event) == true) {
      MVector Z = Event->GetHorizonPointingZAxis();
      double phi = Z.Phi()*c_Deg;
      while (phi < 0) phi += 360; 
      while (phi > 360) phi -= 360; 
      Spherical->Fill(phi, Z.Theta()*c_Deg);
    }
    
    delete Event;
  }
  
  // Close the event loader
  FinalizeEventLoader();

  TCanvas* Canvas = new TCanvas("ZenithCanvas", "ZenithCanvas");

  Canvas->cd();
  Spherical->Draw("colz");
  Canvas->Update();
}


////////////////////////////////////////////////////////////////////////////////


// Create an orientation file for cosima from an existing observation
void MInterfaceMimrec::CreateCosimaOrientationFile()
{

  // Start with the event file loader first (just in case something goes wrong here)
  if (InitializeEventLoader() == false) return;

  MString FileName = m_Settings->GetCurrentFileName();
  FileName.ReplaceAtEnd(".tra.gz", ".ori");
  FileName.ReplaceAtEnd(".tra", ".ori");
  
  ofstream out;
  out.open(FileName);
  out<<endl;
  out<<"Type OrientationsGalactic"<<endl;
  out<<endl;
  out.setf(ios::fixed);
  out.precision(6);
  // First check on the size of the histogram:
  
  FileName = m_Settings->GetCurrentFileName();
  FileName.ReplaceAtEnd(".tra.gz", ".onoff");
  FileName.ReplaceAtEnd(".tra", ".onoff");
  
  ofstream lout;
  lout.open(FileName);
  lout<<endl;
  lout<<"Type OnOff"<<endl;
  lout<<endl;
  lout.setf(ios::fixed);
  lout.precision(6);
  // First check on the size of the histogram:
  
  double LastXAxisLongitude = -1000;
  double LastXAxisLatitude = -1000;
  double LastZAxisLongitude = -1000;
  double LastZAxisLatitude = -1000;
  
  bool First = true;
  MTime LastTime(0);
  MTime Gap(5.0); 
  
  bool IsOn = false;
  MPhysicalEvent* Event = nullptr;
  while ((Event = GetNextEvent()) != 0) {
    if (Event->HasGalacticPointing() == true && m_Selector->IsQualifiedEventFast(Event) == true) {
      if (LastXAxisLongitude != Event->GetGalacticPointingXAxisLongitude() ||
        LastXAxisLatitude != Event->GetGalacticPointingXAxisLatitude() ||
        LastZAxisLongitude != Event->GetGalacticPointingZAxisLongitude() ||
        LastZAxisLatitude != Event->GetGalacticPointingZAxisLatitude()) {
        out<<"OG "<<Event->GetTime()
          <<" "<<Event->GetGalacticPointingXAxisLatitude()*c_Deg
          <<" "<<Event->GetGalacticPointingXAxisLongitude()*c_Deg
          <<" "<<Event->GetGalacticPointingZAxisLatitude()*c_Deg
          <<" "<<Event->GetGalacticPointingZAxisLongitude()*c_Deg<<endl;
         
        LastXAxisLongitude = Event->GetGalacticPointingXAxisLongitude();
        LastXAxisLatitude = Event->GetGalacticPointingXAxisLatitude();
        LastZAxisLongitude = Event->GetGalacticPointingZAxisLongitude();
        LastZAxisLatitude = Event->GetGalacticPointingZAxisLatitude();
      }
      if (First == false) {
        if (Event->GetTime() < LastTime) {
          if (IsOn == true) {
            cout<<"Backwards JUMP! Off: "<<Event->GetTime()<<":"<<LastTime<<endl;
            lout<<"DP "<<LastTime<<" off"<<endl;
            IsOn = false;
          }
        } else {  
          if (Event->GetTime() - LastTime > Gap) {
            if (IsOn == true) {
              cout<<"Forward Jump: Off: "<<Event->GetTime()<<":"<<LastTime<<endl;
              lout<<"DP "<<LastTime<<" off"<<endl;
              IsOn = false;
            }
          } else {
            if (IsOn == false) {
              //cout<<"On: "<<Event->GetTime()<<":"<<LastTime<<endl;
              lout<<"DP "<<LastTime<<" on"<<endl;
              IsOn = true;
            }
          }
        }
      } else {
        //cout<<"On: "<<Event->GetTime()<<":"<<LastTime<<endl;
        lout<<"DP "<<Event->GetTime()<<" on"<<endl;
        First = false;
        IsOn = true;
      }
      if (Event->GetTime() > LastTime) {
        LastTime = Event->GetTime();
      }
    }

    delete Event;
  }
  if (IsOn == true) {
    //cout<<"Off: "<<LastTime<<endl;
    lout<<"DP "<<LastTime<<" off"<<endl;
  }
  
  // Close the event loader
  FinalizeEventLoader();
  
  out<<"EN"<<endl;
  out<<endl;
  out.close();
  
  lout<<"EN"<<endl;
  lout<<endl;
  lout.close();
  
  return;
}


////////////////////////////////////////////////////////////////////////////////


void MInterfaceMimrec::StandardAnalysis(double Energy, MVector Position)
{
  cout<<"Standard analysis started"<<endl; 

  MStandardAnalysis A;
  
  A.SetEventSelector(*m_Selector);
  A.SetGeometry(m_Geometry);
  A.SetFileName(m_Settings->GetCurrentFileName());
   
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
