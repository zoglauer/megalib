/*
 * MXmlNode.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MXmlNode__
#define __MXmlNode__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <cstdlib>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MXmlData.h"
#include "MXmlAttribute.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! An XML node used by the lightweight MEGAlib XML writer/reader
class MXmlNode : public MXmlData
{
  // public interface:
 public:
  //! Default constructor
  MXmlNode();
  //! Constructor without a value
  MXmlNode(MXmlNode* MotherNode, const MString& Name);
  //! Constructor with a string value
  MXmlNode(MXmlNode* MotherNode, const MString& Name, const MString& Value);
  //! Constructor with an integer value
  MXmlNode(MXmlNode* MotherNode, const MString& Name, int Value);
  //! Constructor with a long value
  MXmlNode(MXmlNode* MotherNode, const MString& Name, long Value);
  //! Constructor with an unsigned integer value
  MXmlNode(MXmlNode* MotherNode, const MString& Name, unsigned int Value);
  //! Constructor with an unsigned long value
  MXmlNode(MXmlNode* MotherNode, const MString& Name, unsigned long Value);
  //! Constructor with a double value
  MXmlNode(MXmlNode* MotherNode, const MString& Name, double Value);
  //! Constructor with a vector value
  MXmlNode(MXmlNode* MotherNode, const MString& Name, MVector Value);
  //! Constructor with a time value
  MXmlNode(MXmlNode* MotherNode, const MString& Name, const MTime& Value);
  //! Constructor with a boolean value
  MXmlNode(MXmlNode* MotherNode, const MString& Name, bool Value);
  //! Constructor with integer min/max values
  MXmlNode(MXmlNode* MotherNode, const MString& Name, int ValueMin, int ValueMax);
  //! Constructor with long min/max values
  MXmlNode(MXmlNode* MotherNode, const MString& Name, long ValueMin, long ValueMax);
  //! Constructor with double min/max values
  MXmlNode(MXmlNode* MotherNode, const MString& Name, double ValueMin, double ValueMax);
  //! Constructor with time min/max values
  MXmlNode(MXmlNode* MotherNode, const MString& Name, const MTime& ValueMin, const MTime& ValueMax);
  //! Default destructor
  virtual ~MXmlNode();

  //! Reset the node
  virtual void Clear();

  //! Return the value of the node as vector
  MVector GetValueAsVector() const;
  //! Return the minimum value of the node as int
  int GetMinValueAsInt() const;
  //! Return the maximum value of the node as int
  int GetMaxValueAsInt() const;
  //! Return the minimum value of the node as long
  long GetMinValueAsLong() const;
  //! Return the maximum value of the node as long
  long GetMaxValueAsLong() const;
  //! Return the minimum value of the node as double
  double GetMinValueAsDouble() const;
  //! Return the maximum value of the node as double
  double GetMaxValueAsDouble() const;
  //! Return the minimum value of the node as time
  MTime GetMinValueAsTime() const;
  //! Return the maximum value of the node as time
  MTime GetMaxValueAsTime() const;


  //! Return the number of nodes
  unsigned int GetNNodes() { return m_Nodes.size(); }
  //! Return a given node
  MXmlNode* GetNode(unsigned int i);
  //! Return a given node
  MXmlNode* GetNode(const MString& Name);
  //! Add a node
  void AddNode(MXmlNode* Node);


  //! Return the number of attributes
  unsigned int GetNAttributes() { return m_Attributes.size(); }
  //! Return a given attribute
  MXmlAttribute* GetAttribute(unsigned int i);
  //! Return a given attribute
  MXmlAttribute* GetAttribute(const MString& Name);
  //! Add an attribute
  void AddAttribute(MXmlAttribute* Attribute);


  //! Return the XML text
  virtual MString ToString(unsigned int Indent = 0);


  // protected methods:
 protected:
  //! Parse text into this node
  virtual bool Parse(MString Text);
  //! Return true if the next tag is closed
  virtual bool IsClosed(MString Text);
  
  // private methods:
 private:



  // protected members:
 protected:
  //! All sub nodes
  vector<MXmlNode*> m_Nodes;
  //! All attributes
  vector<MXmlAttribute*> m_Attributes;


  // private members:
 private:


#ifdef ___CLING___
 public:
  ClassDef(MXmlNode, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
