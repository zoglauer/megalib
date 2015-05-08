/*
 * MModuleTemplate.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MModuleTemplate__
#define __MModuleTemplate__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MModule.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MModuleTemplate : public MModule
{
  // public interface:
 public:
  //! Default constructor
  MModuleTemplate();
  //! Default destructor
  virtual ~MModuleTemplate();
  
  //! Create a new object of this class 
  virtual MModuleTemplate* Clone() { return new MModuleTemplate(); }
  
  //! Initialize the module
  virtual bool Initialize();

  //! Main data analysis routine, which updates the event to a new level 
  virtual bool AnalyzeEvent(MReadOutAssembly* Event);

  //! Show the options GUI
  virtual void ShowOptionsGUI();

  //! Read the configuration data from an XML node
  virtual bool ReadXmlConfiguration(MXmlNode* Node);
  //! Create an XML node tree from the configuration
  virtual MXmlNode* CreateXmlConfiguration();


  // protected methods:
 protected:

  // private methods:
 private:
  //! No Copy constructor
  MModuleTemplate(const MModuleTemplate&) = delete;
  //! No copying whatsoever
  MModuleTemplate& operator=(const MModuleTemplate&) = delete;


  // protected members:
 protected:


  // private members:
 private:




#ifdef ___CINT___
 public:
  ClassDef(MModuleTemplate, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
